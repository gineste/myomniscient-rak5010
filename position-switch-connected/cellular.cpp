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

/****************************************************************************************
   Private type declarations
 ****************************************************************************************/

/****************************************************************************************
   Private function declarations
 ****************************************************************************************/

/****************************************************************************************
   Variable declarations
 ****************************************************************************************/

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
  sSptkNetInfo_t l_sNetInfo = {0};      /* Status msg network info */

  s_SensorMngrData_t * l_psSensorsData = psSensorMngr_GetSensorData();
  
  eBG96_SendCommand("AT+CGATT=1", GSM_CMD_RSP_OK_RF, APN_TIMEOUT);
  
  //eBG96_SendCommand("AT+QCFG=\"nwscanseq\",01,1", GSM_CMD_RSP_OK_RF, CMD_TIMEOUT);
  //eBG96_SetRATSearchSeq("01");  // GSM
  eBG96_SendCommand("AT+QCFG=1", GSM_CMD_RSP_OK_RF, CMD_TIMEOUT); // needed otherwise QIACT stuck...
  
  eBG96_SendCommand("AT+QICSGP=1,1,\"nxt17.net\",\"\",\"\",1", GSM_CMD_RSP_OK_RF, CMD_TIMEOUT);
  
  //eBG96_SendCommand("AT+QNWINFO", GSM_CMD_RSP_OK_RF, CMD_TIMEOUT);

  eBG96_SendCommand("AT+QIACT=1", GSM_CMD_RSP_OK_RF, CMD_TIMEOUT); 
//  bg96_at("AT+QIACT=1");
  //delay(2000);

  //Serial.println("get time");
  //bg96_at("AT+QLTS=1"); //query GMT time from network
  //delay(2000);

  eBG96_SendCommand("AT+QHTTPCFG=\"contextid\",1", GSM_CMD_RSP_OK_RF, CMD_TIMEOUT);
  eBG96_SendCommand("AT+QHTTPCFG=\"responseheader\",1", GSM_CMD_RSP_OK_RF, CMD_TIMEOUT);

  //POST request
  bg96_at("AT+QHTTPURL=57,80"); //57 is length of the url
  delay(3000);
  //Serial1.write("https://webhook.site/b80027c3-ec69-4694-b32d-b640549c6213\r");
  //Serial1.write("https://webhook.site/15cc74bf-54b7-4b93-8746-c023eee63d32\r");
  //delay(3000);
  eBG96_SendCommand(SERVER_URL, GSM_CMD_RSP_OK_RF, CMD_TIMEOUT);
  //memset(l_achCmd, 0, MAX_JSON_LEN);
  //sprintf(l_achCmd, "AT+QHTTPURL=%d,80", strlen(SERVER_URL));
  //eBG96_SendCommand("AT+QHTTPURL=57,80", GSM_CONNECT_STR, CMD_TIMEOUT);
  //eBG96_SendCommand(l_achCmd, GSM_CONNECT_STR, CONN_TIMEOUT);
  //eBG96_SendCommand(SERVER_URL, GSM_CMD_RSP_OK_RF, CONN_TIMEOUT);

  /* Get network information */
  /*l_sNetInfo.f32Rate = f32Cellular_GetRegisterRatio();
  l_sNetInfo.pchOperator = pchCellular_GetLastOperatorName();
  l_sNetInfo.pchNetTech = pchCellular_GetLastNetworkTech();
  l_sNetInfo.pchBandTech = pchCellular_GetLastBandTech();*/
  
  /* Get received signal strength */
  l_eBg96Code = eBG96_GetRSSI(&(l_sNetInfo.s16Rssi));
  if(BG96_SUCCESS == l_eBg96Code)
  {
  #ifdef DEBUG
    Serial.printf("RSSI is %d dBm\r\n", l_sNetInfo.s16Rssi);
  #endif
  }


  memset(l_achJson, 0, MAX_JSON_LEN);
  //sprintf(l_achCmd, "{TOR_state: {TOR1_current_state: %d,TOR2_current_state: %d}}\r", l_psSensorsData.au8TORs[0], l_psSensorsData.au8TORs[1]);
  snprintf(l_achJson, MAX_JSON_LEN, "{\"location\": {\"accuracy\": 10,\"altitude\": 30,\"accuracyType\": \"High\",\"position\": {\"lat\": 49.1235111233,\"lon\": 0.12321414141},"
                    "\"lastPositionUpdate\": \"12332141244\"},\"manufacturer\": \"Rak\",\"manufacturerId\": \"123213123FAZDAD\",\"lagTagUpdate\": \"123123123123123\","
                    "\"technology\": \"GPS\",\"metadataTag\": {TOR_state: {\"TOR1_current_state\": %d,\"TOR1_previous_state\": %d,\"TOR2_current_state\": %d,\"TOR2_previous_state\": %d},"
                    "\"messageType\": \"POSITION_MESSAGE\",\"sequenceCounter\": %d,\"eventType\": \"1\",\"profile\": {},\"voltage_int\": 3,\"network\": {\"RSSI\": %d}}}", 
                    l_psSensorsData->au8TORs[SENSOR_MNGR_TOR1], l_psSensorsData->au8TORsPrevious[SENSOR_MNGR_TOR1], l_psSensorsData->au8TORs[SENSOR_MNGR_TOR2], 
                    l_psSensorsData->au8TORsPrevious[SENSOR_MNGR_TOR2], l_u16FrameCnt,l_sNetInfo.s16Rssi);
                    
  memset(l_achCmd, 0, MAX_CMD_LEN);
  snprintf(l_achCmd, MAX_CMD_LEN, "AT+QHTTPPOST=%d,80,80", strlen(l_achJson));
  eBG96_SendCommand(l_achCmd, GSM_CONNECT_STR, CONN_TIMEOUT);  // 58 is length of json body
  
  eBG96_SendCommand(l_achJson, GSM_CMD_RSP_OK_RF, CONN_TIMEOUT);

  l_u16FrameCnt++;
}


/****************************************************************************************
   Private functions
 ****************************************************************************************/

/****************************************************************************************
   End Of File
 ****************************************************************************************/
