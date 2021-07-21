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
#include "BG96_LTE.h"
#include "sensors.h"

#include "cellular.h"

/****************************************************************************************
   Defines
 ****************************************************************************************/
#define JSON_STR_MAX_LEN          (1024u)
#define JSON_DATA_LEN             (60u)
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
//network connect
void connect(void) {
  char l_achJson[JSON_DATA_LEN] = {0};
  
  s_SensorMngrData_t l_sSensorsData = *(psSensorMngr_GetSensorData());
  
  eBG96_SetRATSearchSeq("01");  // GSM
  eBG96_SendCommand("AT+QICSGP=1,1,\"nxt17.net\",\"\",\"\",1", GSM_CMD_RSP_OK_RF, CMD_TIMEOUT);
  
  eBG96_SendCommand("AT+QNWINFO", GSM_CMD_RSP_OK_RF, CMD_TIMEOUT);

  eBG96_SendCommand("AT+QIACT=1", GSM_CMD_RSP_OK_RF, CMD_TIMEOUT);

  //Serial.println("get time");
  //bg96_at("AT+QLTS=1"); //query GMT time from network
  //delay(2000);

  eBG96_SendCommand("AT+QHTTPCFG=\"contextid\",1", GSM_CMD_RSP_OK_RF, CMD_TIMEOUT);
  eBG96_SendCommand("AT+QHTTPCFG=\"responseheader\",1", GSM_CMD_RSP_OK_RF, CMD_TIMEOUT);

  //POST request
  bg96_at("AT+QHTTPURL=57,80"); //57 is length of the url
  delay(3000);
  //Serial1.write("https://webhook.site/b80027c3-ec69-4694-b32d-b640549c6213\r");
  Serial1.write("https://webhook.site/15cc74bf-54b7-4b93-8746-c023eee63d32\r");
  delay(3000);

  bg96_at("AT+QHTTPPOST=58,80,80");//48 is length of the post data
  delay(3000);

  memset(l_achJson, 0, JSON_DATA_LEN);
  sprintf(l_achJson, "{TOR_state: {TOR1_current_state: %d,TOR2_current_state: %d}}\r", l_sSensorsData.u8TOR1, l_sSensorsData.u8TOR2);
  Serial1.write(l_achJson);
    
  delay(3000);
}

/****************************************************************************************
   Private functions
 ****************************************************************************************/

/****************************************************************************************
   End Of File
 ****************************************************************************************/
