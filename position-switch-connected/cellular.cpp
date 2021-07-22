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
  //eBG96_SendCommand("AT+QICSGP=1,1,\"sl2sfr\",\"\",\"\",1", GSM_CMD_RSP_OK_RF, CMD_TIMEOUT);
  
  //eBG96_SendCommand("AT+QNWINFO", GSM_CMD_RSP_OK_RF, CMD_TIMEOUT);

  eBG96_SendCommand("AT+QIACT=1", GSM_CMD_RSP_OK_RF, CMD_TIMEOUT); 
//  bg96_at("AT+QIACT=1");
  //delay(2000);

  //Serial.println("get time");
  //bg96_at("AT+QLTS=1"); //query GMT time from network
  //delay(2000);

  eBG96_SendCommand("AT+QHTTPCFG=\"contextid\",1", GSM_CMD_RSP_OK_RF, CMD_TIMEOUT);
  eBG96_SendCommand("AT+QHTTPCFG=\"responseheader\",1", GSM_CMD_RSP_OK_RF, CMD_TIMEOUT);
  
  eBG96_SendCommand("AT+QHTTPCFG=\"requestheader\",1", GSM_CMD_RSP_OK_RF, CMD_TIMEOUT);


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
                    "\"lastPositionUpdate\": \"12332141244\"},\"manufacturer\": \"Rak\",\"manufacturerId\": \"%02X%02X%02X%02X%02X%02X\",\"lagTagUpdate\": \"123123123123123\","
                    "\"technology\": \"GPS\",\"metadataTag\": {TOR_state: {\"TOR1_current_state\": %d,\"TOR1_previous_state\": %d,\"TOR2_current_state\": %d,\"TOR2_previous_state\": %d},"
                    "\"messageType\": \"POSITION_MESSAGE\",\"sequenceCounter\": %d,\"eventType\": \"1\",\"profile\": {},\"voltage_int\": 3,\"network\": {\"RSSI\": %d}}}", 
                    (addr_high >> 8) & 0xFF, (addr_high) & 0xFF, (addr_low >> 24) & 0xFF,(addr_low >> 16) & 0xFF, (addr_low >> 8) & 0xFF, (addr_low) & 0xFF,
                    l_psSensorsData->au8TORs[SENSOR_MNGR_TOR1], l_psSensorsData->au8TORsPrevious[SENSOR_MNGR_TOR1], l_psSensorsData->au8TORs[SENSOR_MNGR_TOR2], 
                    l_psSensorsData->au8TORsPrevious[SENSOR_MNGR_TOR2], l_u16FrameCnt,l_sNetInfo.s16Rssi);

  vCellular_PostHttp(String(l_achJson));

#if 0
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
                    "\"lastPositionUpdate\": \"12332141244\"},\"manufacturer\": \"Rak\",\"manufacturerId\": \"%02X%02X%02X%02X%02X%02X\",\"lagTagUpdate\": \"123123123123123\","
                    "\"technology\": \"GPS\",\"metadataTag\": {TOR_state: {\"TOR1_current_state\": %d,\"TOR1_previous_state\": %d,\"TOR2_current_state\": %d,\"TOR2_previous_state\": %d},"
                    "\"messageType\": \"POSITION_MESSAGE\",\"sequenceCounter\": %d,\"eventType\": \"1\",\"profile\": {},\"voltage_int\": 3,\"network\": {\"RSSI\": %d}}}", 
                    (addr_high >> 8) & 0xFF, (addr_high) & 0xFF, (addr_low >> 24) & 0xFF,(addr_low >> 16) & 0xFF, (addr_low >> 8) & 0xFF, (addr_low) & 0xFF,
                    l_psSensorsData->au8TORs[SENSOR_MNGR_TOR1], l_psSensorsData->au8TORsPrevious[SENSOR_MNGR_TOR1], l_psSensorsData->au8TORs[SENSOR_MNGR_TOR2], 
                    l_psSensorsData->au8TORsPrevious[SENSOR_MNGR_TOR2], l_u16FrameCnt,l_sNetInfo.s16Rssi);
                    
  memset(l_achCmd, 0, MAX_CMD_LEN);
  snprintf(l_achCmd, MAX_CMD_LEN, "AT+QHTTPPOST=%d,80,80", strlen(l_achJson));
  eBG96_SendCommand(l_achCmd, GSM_CONNECT_STR, CONN_TIMEOUT);  // 58 is length of json body
  
  eBG96_SendCommand(l_achJson, GSM_CMD_RSP_OK_RF, CONN_TIMEOUT);
#endif

  l_u16FrameCnt++;
}

static void vCellular_PostHttp(String json) {
  #if 0
  String payload = "";
  String host;
  String l_json = json;
  String url;
  char cmd [100];
  String urn;

  Serial.printf("JSON LEN = %d\r\n", l_json.length());
  
  host = "prod-31.francecentral.logic.azure.com";
  urn = "/workflows/06e296db4bc1404f816444502827ae80/triggers/manual/paths/invoke/exotic/v1?api-version=2016-06-01&sp=%2Ftriggers%2Fmanual%2Frun&sv=1.0&sig=ZActYj_9ziMvqfhtcc4JWmjgVEdOTO8mMygYEB0I4FA";
  url = "https://" + host + urn;
  String ms_json = "{location:{position:{lat:49.12345,lon:0.12345}}}";
  Serial.print(url);
  sprintf(cmd, "AT+QHTTPURL=%d,80",url.length());
  bg96_at(cmd);
  delay(1000);
  Serial1.print(url);
  delay(3000);
  payload = "POST " +  url +  " HTTP/1.1\r\n";
  payload += "Host: " + host + "\r\n";
  payload += "Accept: */*\r\n";
  payload += "User-Agent: QUECTEL_MODULE\r\n";
  payload += "Connection: Keep-Alive\r\n";
  payload += "X-proxy-auth: 88kmrhn6CSt9GparhdNXE8aSRrjeDCx\r\n";
  payload += "Content-Type: Application/json\r\n";
  //payload += "Content-Length: 525";// + l_json.length();
  String json_len = String(ms_json.length());
  payload += "Content-Length: " + json_len;
  payload += "\r\n\r\n";
  //payload += l_json;
  payload += ms_json;
  Serial.println("*****PAYLOAD******");
  Serial.println(payload);
  uint32_t post_len = payload.length();
  sprintf(cmd, "AT+QHTTPPOST=%d,80,80",post_len);
  Serial.println(cmd);
  bg96_at(cmd);
  delay(3000);
  Serial1.print(payload);
  #endif

  String payload = "";
String host;
String url;
//String json;
char cmd [100];
String urn;
  host = "prod-31.francecentral.logic.azure.com";
  urn = "/workflows/06e296db4bc1404f816444502827ae80/triggers/manual/paths/invoke/exotic/v1?api-version=2016-06-01&sp=%2Ftriggers%2Fmanual%2Frun&sv=1.0&sig=ZActYj_9ziMvqfhtcc4JWmjgVEdOTO8mMygYEB0I4FA";
  url = "https://" + host + urn;
  //json = "{location:{position:{lat:49.12345,lon:0.12345}}}";
  Serial.print(url);
  memset(cmd, 0, 100);
  sprintf(cmd, "AT+QHTTPURL=%d,80",url.length());
  bg96_at(cmd);
  delay(1000);
  Serial1.print(url);
  delay(3000);
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
  Serial.println("*****PAYLOAD******");
  Serial.println(payload);
  uint32_t post_len = payload.length();
  memset(cmd, 0, 100);
  sprintf(cmd, "AT+QHTTPPOST=%d,80,80",post_len);
  Serial.println(cmd);
  eBG96_SendCommand(cmd, GSM_CONNECT_STR, CONN_TIMEOUT);  // 58 is length of json body
  /*Serial1.print(payload);
  delay(3000);*/
  eBG96_SendCommand((char *)(payload.c_str()), GSM_CMD_RSP_OK_RF, CONN_TIMEOUT);  // 58 is length of json body
}


/****************************************************************************************
   Private functions
 ****************************************************************************************/

/****************************************************************************************
   End Of File
 ****************************************************************************************/
