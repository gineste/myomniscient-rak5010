/*
      ____  _  _   __   ____   __    ___    ____  _  _  ____  ____  ____  _  _  ____
     (  __)( \/ ) /  \ (_  _) (  )  / __)  / ___)( \/ )/ ___)(_  _)(  __)( \/ )/ ___)
      ) _)  )  ( (  O )  )(    )(  ( (__   \___ \ )  / \___ \  )(   ) _) / \/ \\___ \
     (____)(_/\_) \__/  (__)  (__)  \___)  (____/(__/  (____/ (__) (____)\_)(_/(____/

   Copyright (c) 2021 EXOTIC SYSTEMS. All Rights Reserved.

   Licensees are granted free, non-transferable use of the information. NO WARRANTY
   of ANY KIND is provided. This heading must NOT be removed from the file.

   Date:          12/07/2021
   Author:        Martin C.
   Description:   cellular app

*/

/****************************************************************************************
   Include Files
 ****************************************************************************************/
#include <Arduino.h>

#include "BG96.h"
#include "config.h"
#include "sensors.h"
#include "BG96_LTE.h"

#include "cellular.h"

/****************************************************************************************
   Defines
 ****************************************************************************************/
#define IP_MAX_SIZE           (16u)
#define MAX_JSON_LEN          (1024u)

typedef volatile uint32_t REG32;
#define pREG32 (REG32 *)

#define MAC_ADDRESS_HIGH  (*(pREG32 (0x100000a8)))
#define MAC_ADDRESS_LOW   (*(pREG32 (0x100000a4)))

#define MAX_CHECK_NETWORK_RETRIES  (120u)
#define MAX_CHECK_CONTEXT_RETRIES (3u)

#define MAX_STR_LEN            (8u)
#define MAX_STR_NETWORK_LEN     (32u)

/****************************************************************************************
   Private type declarations
 ****************************************************************************************/

/****************************************************************************************
   Private function declarations
 ****************************************************************************************/
static void vCellular_PostHttp(String json);

/****************************************************************************************
   Variable declarations
 ****************************************************************************************/
uint32_t addr_high = ((MAC_ADDRESS_HIGH) & 0x0000ffff) | 0x0000c000;
uint32_t addr_low  = MAC_ADDRESS_LOW;

static eNetworkTech_t g_eNetworkTech = NET_TECH_GSM;
static eCellularBand_t g_eBandTech = CELLULAR_BAND_LTE;
static char g_achNetworkName[MAX_OPERATOR_NAME_LEN + 1u] = {0u};
static char g_achAccessTech[MAX_STR_LEN] = {0};
static char g_achOperatorId[MAX_STR_LEN] = {0};
static char g_achNetworkBand[MAX_STR_NETWORK_LEN] = {0};
static char g_achChannel[MAX_STR_LEN] = {0};
  
/****************************************************************************************
   Public functions
 ****************************************************************************************/
void vCellular_SendData(void) {
  char l_achCmd[MAX_CMD_LEN] = {0};
  char l_achJson[MAX_JSON_LEN] = {0};
  
  eBG96ErrorCode_t l_eBg96Code = BG96_SUCCESS;
  eNetCtxStat_t l_eCtxState = NET_CTX_DEACTIVATE;
  char l_achIp[IP_MAX_SIZE] = {0};
  static uint16_t l_u16FrameCnt = 0u;
  uint8_t l_u8Retry = 0u;

  uint8_t l_u8ChargeStatus, l_u8ChargeLevel = 0u;
  uint16_t l_u16BattMv = 0u;
  
  sSptkNetInfo_t l_sNetInfo = {0};      /* Status msg network info */
  eNetworkStat_t l_eNetworkState = NET_STAT_DETACHED;
  eNetworkMode_t l_eNetworkMode = NET_MODE_DEREGISTER;
  eNetworkRegisterState_t l_eNetworkRegisterState = NET_REG_SEARCHING;

  s_SensorMngrData_t * l_psSensorsData = psSensorMngr_GetSensorData();
  
  if (BG96_SUCCESS != eBG96_SendCommand("AT+CGATT=1", GSM_CMD_RSP_OK_RF, APN_TIMEOUT))
  {
    while(l_u8Retry < 200u)
    {
      if (BG96_SUCCESS != eBG96_SendCommand("AT+CGATT=1", GSM_CMD_RSP_OK_RF, APN_TIMEOUT))
      {
        l_u8Retry++;
        delay(100); 
      }else{
        break;
      }
    }
  }
  
  //eBG96_SendCommand("AT+QCFG=\"nwscanseq\",01,1", GSM_CMD_RSP_OK_RF, CMD_TIMEOUT);
  //eBG96_SetRATSearchSeq("01");  // GSM
  eBG96_SendCommand("AT+QCFG=1", GSM_CMD_RSP_OK_RF, CMD_TIMEOUT); // needed otherwise QIACT stuck...
  
  eBG96_SendCommand("AT+QICSGP=1,1,\"nxt17.net\",\"\",\"\",1", GSM_CMD_RSP_OK_RF, CMD_TIMEOUT);

  eBG96_SendCommand("AT+QIACT=1", GSM_CMD_RSP_OK_RF, CMD_TIMEOUT); 

  eBG96_SendCommand("AT+QHTTPCFG=\"contextid\",1", GSM_CMD_RSP_OK_RF, CMD_TIMEOUT);
  eBG96_SendCommand("AT+QHTTPCFG=\"responseheader\",1", GSM_CMD_RSP_OK_RF, CMD_TIMEOUT);
  
  eBG96_SendCommand("AT+QHTTPCFG=\"requestheader\",1", GSM_CMD_RSP_OK_RF, CMD_TIMEOUT);

  /* Get some network information */
  memset(g_achNetworkName, 0, sizeof(g_achNetworkName));
  eBG96_GetNetwork(&l_eNetworkMode, g_achNetworkName, &g_eNetworkTech);
  
  memset(g_achAccessTech, 0, sizeof(g_achAccessTech));
  memset(g_achNetworkBand, 0, sizeof(g_achNetworkBand));
  memset(g_achOperatorId, 0, sizeof(g_achOperatorId));
  memset(g_achChannel, 0, sizeof(g_achChannel));

  l_eBg96Code = eBG96_GetNetworkInfo(g_achAccessTech, g_achNetworkBand, g_achOperatorId, g_achChannel);
  if(BG96_SUCCESS == l_eBg96Code)
  {
  #ifdef DEBUG
    Serial.printf("Connect to %s, in %s on %s (ch%s)\r\n", g_achNetworkName, g_achAccessTech, g_achNetworkBand, g_achChannel);
  #endif
  }

  /* Get received signal strength */
  l_eBg96Code = eBG96_GetRSSI(&(l_sNetInfo.s16Rssi));
  if(BG96_SUCCESS == l_eBg96Code)
  {
  #ifdef DEBUG
    Serial.printf("RSSI is %d dBm\r\n", l_sNetInfo.s16Rssi);
  #endif
  }

  // Get battery voltage and charge level
  if(BG96_SUCCESS == eBG96_GetBattInfos(&l_u8ChargeStatus, &l_u8ChargeLevel, &l_u16BattMv))
  {
  #ifdef DEBUG
    Serial.printf("Batt infos : %hu%% ; %hu mV\r\n", l_u8ChargeLevel, l_u16BattMv);
  #endif
  }
  
  memset(l_achJson, 0, MAX_JSON_LEN);
  snprintf(l_achJson, MAX_JSON_LEN, "{\"location\": {\"accuracy\": %.1f,\"altitude\": %.1f,\"accuracyType\": \"High\",\"position\": {\"lat\": %f,\"lon\": %f},"
                    "\"lastPositionUpdate\": \"12332141244\"},\"manufacturer\": \"Rak\",\"manufacturerId\": \"%02X%02X%02X%02X%02X%02X\",\"lagTagUpdate\": \"123123123123123\","
                    "\"technology\": \"GPS\",\"metadataTag\": {TOR_state: {\"TOR1_current_state\": %d,\"TOR1_previous_state\": %d,\"TOR2_current_state\": %d,\"TOR2_previous_state\": %d},"
                    "\"messageType\": \"POSITION_MESSAGE\",\"sequenceCounter\": %d,\"eventType\": \"1\",\"profile\": {},\"voltage_int\": %d,\"batt_level\": %d,"
                    "\"network\": {\"RSSI\": %d,\"Operator\": \"%s\",\"Tech\": \"%s\",\"Band\": \"%s\"}}}", 
                    l_psSensorsData->sPosition.f32Hdop, l_psSensorsData->sPosition.f32Altitude, l_psSensorsData->sPosition.f32Latitude, l_psSensorsData->sPosition.f32Longitude,  
                    (addr_high >> 8) & 0xFF, (addr_high) & 0xFF, (addr_low >> 24) & 0xFF,(addr_low >> 16) & 0xFF, (addr_low >> 8) & 0xFF, (addr_low) & 0xFF,
                    l_psSensorsData->au8TORs[SENSOR_MNGR_TOR1], l_psSensorsData->au8TORsPrevious[SENSOR_MNGR_TOR1], l_psSensorsData->au8TORs[SENSOR_MNGR_TOR2], 
                  l_psSensorsData->au8TORsPrevious[SENSOR_MNGR_TOR2], l_u16FrameCnt, l_u16BattMv, l_u8ChargeLevel, l_sNetInfo.s16Rssi, g_achNetworkName, g_achAccessTech, g_achNetworkBand);

  vCellular_PostHttp(String(l_achJson));

  eBG96_SendCommand("AT+CGATT=0", GSM_CMD_RSP_OK_RF, DEACT_TIMEOUT); 
  
  l_u16FrameCnt++;
}

static void vCellular_PostHttp(String json) {
  String payload = "";
  String host = "";
  String url = "";
  char l_achCmd[MAX_CMD_LEN] = {0};
  String urn = "";
  uint16_t l_u16PostLen = 0u;
  
  host = "prod-31.francecentral.logic.azure.com";
  urn = "/workflows/06e296db4bc1404f816444502827ae80/triggers/manual/paths/invoke/exotic/v1?api-version=2016-06-01&sp=%2Ftriggers%2Fmanual%2Frun&sv=1.0&sig=ZActYj_9ziMvqfhtcc4JWmjgVEdOTO8mMygYEB0I4FA";
  url = "https://" + host + urn;

  // configure URL
  memset(l_achCmd, 0, MAX_CMD_LEN);
  sprintf(l_achCmd, "AT+QHTTPURL=%d,80",url.length());
  eBG96_SendCommand(l_achCmd, GSM_CONNECT_STR, CMD_TIMEOUT);
  eBG96_SendCommand((char *)(url.c_str()), GSM_CMD_RSP_OK_RF, CMD_TIMEOUT);

  // build full payload
  payload = "POST " +  url +  " HTTP/1.1\r\n";
  payload += "Host: " + host + "\r\n";
  payload += "Accept: */*\r\n";
  payload += "User-Agent: QUECTEL_MODULE\r\n";
  payload += "Connection: Keep-Alive\r\n";
  payload += "X-proxy-auth: 88kmrhn6CSt9GparhdNXE8aSRrjeDCx\r\n";
  payload += "Content-Type: Application/json\r\n";
  payload += "Content-Length: " + String(json.length());
  payload += "\r\n\r\n";
  payload += json;

#ifdef DEBUG
  Serial.print("Payload : "); Serial.println(payload);
#endif

  // post http with payload
  l_u16PostLen = payload.length();
  memset(l_achCmd, 0, MAX_CMD_LEN);
  sprintf(l_achCmd, "AT+QHTTPPOST=%d,80,80",l_u16PostLen);
  if(BG96_SUCCESS == eBG96_SendCommand(l_achCmd, GSM_CONNECT_STR, CONN_TIMEOUT))
  {
    Serial1.print(payload);
    delay(3000);
  }
}


/****************************************************************************************
   Private functions
 ****************************************************************************************/
 
/****************************************************************************************
   End Of File
 ****************************************************************************************/
