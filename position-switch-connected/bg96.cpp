/*
 *    ____  _  _   __   ____   __    ___    ____  _  _  ____  ____  ____  _  _  ____
 *   (  __)( \/ ) /  \ (_  _) (  )  / __)  / ___)( \/ )/ ___)(_  _)(  __)( \/ )/ ___)
 *    ) _)  )  ( (  O )  )(    )(  ( (__   \___ \ )  / \___ \  )(   ) _) / \/ \\___ \     
 *   (____)(_/\_) \__/  (__)  (__)  \___)  (____/(__/  (____/ (__) (____)\_)(_/(____/
 *
 * Copyright (c) 2021 EXOTIC SYSTEMS. All Rights Reserved.
 *
 * Licensees are granted free, non-transferable use of the information. NO WARRANTY 
 * of ANY KIND is provided. This heading must NOT be removed from the file.
 *
 * Date:          12/07/2021
 * Author:        Martin C.
 * Description:   BG96 drivers
 *
 */

/****************************************************************************************
 * Include Files
 ****************************************************************************************/
#include <Arduino.h>

#include "bg96.h"

/****************************************************************************************
 * Defines
 ****************************************************************************************/
//Pin define
#define bg96_W_DISABLE 29
#define bg96_RESET 28
#define bg96_PWRKEY 2
#define bg96_GPS_EN 39

#define MAX_CMD_LEN    (256u)

/****************************************************************************************
 * Private type declarations
 ****************************************************************************************/

/****************************************************************************************
 * Private function declarations
 ****************************************************************************************/

/****************************************************************************************
 * Variable declarations
 ****************************************************************************************/
 String bg96_rsp = "";

/****************************************************************************************
 * Public functions
 ****************************************************************************************/ 
//bg96 power up
void bg96_init()
{
     pinMode(bg96_RESET, OUTPUT);
     pinMode(bg96_PWRKEY, OUTPUT);
     pinMode(bg96_GPS_EN, OUTPUT);
     pinMode(bg96_W_DISABLE, OUTPUT);

     digitalWrite(bg96_RESET,0);
     digitalWrite(bg96_PWRKEY,1);
     digitalWrite(bg96_W_DISABLE,1);
     delay(2000);
     digitalWrite(bg96_PWRKEY,0);
     digitalWrite(bg96_GPS_EN,1);
     delay(2000);
}
//this function is suitable for most AT commands of bg96. e.g. bg96_at("ATI")
void bg96_at(char *at)
{
  char tmp[256] = {0};
  int len = strlen(at);
  strncpy(tmp,at,len);
  tmp[len]='\r';
  Serial1.write(tmp);
  delay(10);
  while(Serial1.available()){
      bg96_rsp += char(Serial1.read());
      delay(2);
  }
  Serial.println(bg96_rsp);
  bg96_rsp="";
}

//gps data
void gps_show()
{
 
  bg96_rsp="";
  Serial1.write("AT+QGPSGNMEA=\"GGA\"\r");
  delay(10);
  while(Serial1.available()){
      bg96_rsp += char(Serial1.read());
      delay(2);
  }
  Serial.println(bg96_rsp);
  bg96_rsp="";
}

/**@brief Set Apn information
 * @param p_pchApn         Apn operator address
 * @param p_pchUser        Apn access username
 * @param p_pchPassword    Apn access password
 *
 * @info Always use IPv4 context
 *
 * @retval BG96_SUCCESS
 * @retval BG96_ERROR_FAILED
 * @retval BG96_ERROR_PARAM
 */
eBG96ErrorCode_t eBG96_SetApnContext(char * p_pchApn, char * p_pchUser, char * p_pchPassword)
{
  eBG96ErrorCode_t l_eCode = BG96_SUCCESS;
  char l_achCmd[MAX_CMD_LEN] = {0};

  /* By default always use IPv4 context */
  if((p_pchApn != NULL) && (p_pchUser != NULL) && (p_pchPassword != NULL))
  {
    snprintf(l_achCmd, MAX_CMD_LEN, "AT+QICSGP=1,1,\"%s\",\"%s\",\"%s\",1", p_pchApn, p_pchUser, p_pchPassword);
    bg96_at(l_achCmd);
    l_eCode = BG96_SUCCESS;
  }else{
    l_eCode = BG96_ERROR_PARAM;
  }

  return l_eCode;

}

/**@brief Active TCP/IP context
 * @param p_pchIp         Allocated IP returned by server
 * @retval BG96_SUCCESS
 * @retval BG96_ERROR_FAILED
 * @retval BG96_ERROR_PARAM
 */
eBG96ErrorCode_t eBG96_ActiveContext(void)
{
  bg96_at("AT+QIACT=1");
  return BG96_SUCCESS;
}

/****************************************************************************************
 * Private functions
 ****************************************************************************************/

/****************************************************************************************
 * End Of File
 ****************************************************************************************/
