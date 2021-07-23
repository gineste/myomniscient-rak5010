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
#include "ExoTime.h"
#include "timeout.h"
#include "GNSS.h"

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

#define BG96_TIME_CMD_LEN    (32u) /* Date time AT command response length */

#define TIME_SYNC_TRY_MAX      (uint8_t)20u

/****************************************************************************************
   Private type declarations
 ****************************************************************************************/

/****************************************************************************************
   Private function declarations
 ****************************************************************************************/
static void vCellular_PostHttp(String json);
static eCellularErrorCode_t eTxSyncTime(uint32_t * p_pu32TxTs);

/****************************************************************************************
   Variable declarations
 ****************************************************************************************/
static uint32_t addr_high = ((MAC_ADDRESS_HIGH) & 0x0000ffff) | 0x0000c000;
static uint32_t addr_low  = MAC_ADDRESS_LOW;

static int8_t g_s8GMT = 0;    /* timezone offset in quarters of hours */
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
void vCellular_SendData(uint8_t p_eMsgType) {
  char l_achCmd[MAX_CMD_LEN] = {0};
  char l_achJson[MAX_JSON_LEN] = {0};
  
  eBG96ErrorCode_t l_eBg96Code = BG96_SUCCESS;
  eNetCtxStat_t l_eCtxState = NET_CTX_DEACTIVATE;
  char l_achIp[IP_MAX_SIZE] = {0};
  static uint16_t l_u16FrameCnt = 0u;
  uint8_t l_u8Retry = 0u;

  uint8_t l_u8ChargeStatus, l_u8ChargeLevel = 0u;
  uint16_t l_u16BattMv = 0u;
  uint32_t l_u32TxTs = 0u;    // Tx timestamp UTC
  
  sSptkNetInfo_t l_sNetInfo = {0};      /* Status msg network info */
  eNetworkStat_t l_eNetworkState = NET_STAT_DETACHED;
  eNetworkMode_t l_eNetworkMode = NET_MODE_DEREGISTER;
  eNetworkRegisterState_t l_eNetworkRegisterState = NET_REG_SEARCHING;

  s_SensorMngrData_t * l_psSensorsData = psSensorMngr_GetSensorData();

  // Connect to network
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

  // Sync TX timestamp UTC
  eTxSyncTime(&l_u32TxTs);
  
  memset(l_achJson, 0, MAX_JSON_LEN);

  if (CELLULAR_MSG_POSITION == p_eMsgType)
  {
    snprintf(l_achJson, MAX_JSON_LEN, "{\"location\": {\"accuracy\": %.1f,\"altitude\": %.1f,\"position\": {\"lat\": %f,\"lon\": %f},"
                      "\"lastPositionUpdate\": %d},\"manufacturer\": \"Rak\",\"manufacturerId\": \"%02X%02X%02X%02X%02X%02X\",\"lagTagUpdate\": %d,"
                      "\"technology\": \"GPS\",\"metadataTag\": {TOR_state: {\"TOR1_current_state\": %d,\"TOR1_previous_state\": %d,\"TOR2_current_state\": %d,\"TOR2_previous_state\": %d},"
                      "\"messageType\": \"HB\",\"sequenceCounter\": %d,\"eventType\": \"1\",\"profile\": {},\"voltage_int\": %d,\"batt_level\": %d,"
                      "\"network\": {\"RSSI\": %d,\"Operator\": \"%s\",\"Tech\": \"%s\",\"Band\": \"%s\"}}}", 
                      l_psSensorsData->sPosition.f32Hdop, l_psSensorsData->sPosition.f32Altitude, l_psSensorsData->sPosition.f32Latitude, l_psSensorsData->sPosition.f32Longitude, l_psSensorsData->u32TsPosition,  
                      (addr_high >> 8) & 0xFF, (addr_high) & 0xFF, (addr_low >> 24) & 0xFF,(addr_low >> 16) & 0xFF, (addr_low >> 8) & 0xFF, (addr_low) & 0xFF, l_u32TxTs,
                      l_psSensorsData->au8TORs[SENSOR_MNGR_TOR1], l_psSensorsData->au8TORsPrevious[SENSOR_MNGR_TOR1], l_psSensorsData->au8TORs[SENSOR_MNGR_TOR2], 
                      l_psSensorsData->au8TORsPrevious[SENSOR_MNGR_TOR2], l_u16FrameCnt, l_u16BattMv, l_u8ChargeLevel, l_sNetInfo.s16Rssi, g_achNetworkName, g_achAccessTech, g_achNetworkBand);
  }else if (CELLULAR_MSG_EVENT == p_eMsgType){
    
    snprintf(l_achJson, MAX_JSON_LEN, "{\"manufacturer\": \"Rak\",\"manufacturerId\": \"%02X%02X%02X%02X%02X%02X\",\"lagTagUpdate\": %d,"
                  "\"technology\": \"GPS\",\"metadataTag\": {TOR_state: {\"TOR1_current_state\": %d,\"TOR1_previous_state\": %d,\"TOR2_current_state\": %d,\"TOR2_previous_state\": %d},"
                  "\"messageType\": \"EVENT\",\"sequenceCounter\": %d,\"eventType\": \"1\",\"profile\": {},\"voltage_int\": %d,\"batt_level\": %d,"
                  "\"network\": {\"RSSI\": %d,\"Operator\": \"%s\",\"Tech\": \"%s\",\"Band\": \"%s\"}}}", 
                  (addr_high >> 8) & 0xFF, (addr_high) & 0xFF, (addr_low >> 24) & 0xFF,(addr_low >> 16) & 0xFF, (addr_low >> 8) & 0xFF, (addr_low) & 0xFF, l_u32TxTs,
                  l_psSensorsData->au8TORs[SENSOR_MNGR_TOR1], l_psSensorsData->au8TORsPrevious[SENSOR_MNGR_TOR1], l_psSensorsData->au8TORs[SENSOR_MNGR_TOR2], 
                  l_psSensorsData->au8TORsPrevious[SENSOR_MNGR_TOR2], l_u16FrameCnt, l_u16BattMv, l_u8ChargeLevel, l_sNetInfo.s16Rssi, g_achNetworkName, g_achAccessTech, g_achNetworkBand);
  }

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

/**@brief Get the UTC date from the BG96 in s_ExoTime_t format.
 * It also gives the time zone (indicates the difference, expressed in
 * quarters of an hour, between the local time and GMT; range: -48...+56).
 * @param p_psExoTime:  pointer to s_ExoTime_t date
 * @param p_ps8GMT:   pointer to time zone
 * @retval Error code
 */
eCellularErrorCode_t eCellular_GetDate(s_ExoTime_t * p_psExoTime, int8_t * p_ps8GMT)
{
  eCellularErrorCode_t l_eCode = CELLULAR_ERROR_FAILED;
  char l_achTimeStr[BG96_TIME_CMD_LEN] = "";  /* AT command response */
  char * l_pchTimeStrPtr = l_achTimeStr;      /* Pointer to move in the l_achTimeStr array */
  char * l_pchEndPtr;                 /* End pointer updates by strtol func */

  if((NULL != p_psExoTime) && (NULL != p_ps8GMT))
  {
    /* Send AT command */
    if(BG96_SUCCESS == eBG96_GetTime(l_achTimeStr))
    {
      /* Store each date "numbers" from the response in s_ExoTime_t struct */
      l_pchTimeStrPtr++;
      p_psExoTime->u16Year = strtol(l_pchTimeStrPtr, &l_pchEndPtr, 10 );

      /* Check new string validity */
      if(l_pchEndPtr != l_pchTimeStrPtr)
      {
        /* Add current millennium */
        p_psExoTime->u16Year += 2000u;

        l_pchTimeStrPtr = l_pchEndPtr;
        l_pchTimeStrPtr++;
        p_psExoTime->u8Month = strtol(l_pchTimeStrPtr, &l_pchEndPtr, 10 );
        if(l_pchEndPtr != l_pchTimeStrPtr)
        {
          l_pchTimeStrPtr = l_pchEndPtr;
          l_pchTimeStrPtr++;
          p_psExoTime->u8DayOfMonth = strtol(l_pchTimeStrPtr, &l_pchEndPtr, 10 );
          if(l_pchEndPtr != l_pchTimeStrPtr)
          {
            l_pchTimeStrPtr = l_pchEndPtr;
            l_pchTimeStrPtr++;
            p_psExoTime->u8Hour = strtol(l_pchTimeStrPtr, &l_pchEndPtr, 10 );
            if(l_pchEndPtr != l_pchTimeStrPtr)
            {
              l_pchTimeStrPtr = l_pchEndPtr;
              l_pchTimeStrPtr++;
              p_psExoTime->u8Minute = strtol(l_pchTimeStrPtr, &l_pchEndPtr, 10 );
              if(l_pchEndPtr != l_pchTimeStrPtr)
              {
                l_pchTimeStrPtr = l_pchEndPtr;
                l_pchTimeStrPtr++;
                p_psExoTime->u8Second = strtol(l_pchTimeStrPtr, &l_pchEndPtr, 10 );
                if(l_pchEndPtr != l_pchTimeStrPtr)
                {
                  l_pchTimeStrPtr = l_pchEndPtr;
                  l_pchTimeStrPtr++;
                  *p_ps8GMT = strtol(l_pchTimeStrPtr, &l_pchEndPtr, 10 );
                  if(l_pchEndPtr != l_pchTimeStrPtr)
                  {
                    l_eCode = CELLULAR_SUCCESS;
                  #ifdef DEBUG
                    Serial.printf("TX DATE IS %d/%d/%d,%d:%d:%d GMT%d\r\n",
                             p_psExoTime->u16Year,
                             p_psExoTime->u8Month,
                             p_psExoTime->u8DayOfMonth,
                             p_psExoTime->u8Hour,
                            p_psExoTime->u8Minute,
                            p_psExoTime->u8Second,
                            *p_ps8GMT);
                   #endif
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  else
  {
    l_eCode = CELLULAR_ERROR_PARAM;
  }

  return l_eCode;
}


/****************************************************************************************
   Private functions
 ****************************************************************************************/
 
 /**
 * @brief SYnchronize device timestamp with the date retreived from Network
 * @param p_pu32TxTs : Timestamp UTC
 * @return Error code
 */
static eCellularErrorCode_t eTxSyncTime(uint32_t * p_pu32TxTs)
{
   eCellularErrorCode_t l_eCode = CELLULAR_ERROR_FAILED;
   s_ExoTime_t l_sExoTime = {0};
   uint32_t l_u32BuildTimeStamp = 0u;
   uint8_t l_u8NbTry = 0u;

  if (p_pu32TxTs != NULL)
  {
    /* Try to sync BG96 time with network */
    do
    {
      vTime_WaitMs(1000u);
      l_u8NbTry++;
  
      l_eCode = eCellular_GetDate(&l_sExoTime, &g_s8GMT);
    #ifdef DEBUG
      Serial.printf("[%u] Wait sync time [%u/%u]\r\n",
           (unsigned int)u32Time_getMs(),
           (unsigned int)l_u8NbTry,
           TIME_SYNC_TRY_MAX);
    #endif
  
    }while( (CELLULAR_SUCCESS == l_eCode)
        && (l_u8NbTry < TIME_SYNC_TRY_MAX)
        && ((l_sExoTime.u16Year < EXOTIME_MIN_YEAR_VALUE) || (l_sExoTime.u16Year > EXOTIME_MAX_YEAR_VALUE)));
  
    /* Init unix EPOC timestamp with BG96 cellular date */
    if((CELLULAR_SUCCESS == l_eCode) && (l_u8NbTry < TIME_SYNC_TRY_MAX))
    {
       if(EXOERRORS_NO == eDateToTimestamp(&l_sExoTime,&l_u32BuildTimeStamp))
       {
          *p_pu32TxTs = l_u32BuildTimeStamp;
          //itsdk_time_sync_EPOC_s(l_u32BuildTimeStamp);
        #ifdef DEBUG
          Serial.printf("Epoc date sync successfully\r\n");
        #endif
       }
    }else{
        #ifdef DEBUG
          Serial.printf("Fail to sync TX UTC time\r\n");
        #endif
    }
  }else{
    l_eCode = CELLULAR_ERROR_PARAM;
  }

  return l_eCode;
}
 
/****************************************************************************************
   End Of File
 ****************************************************************************************/
