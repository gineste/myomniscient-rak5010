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
   Description:   BG96 drivers

*/

/****************************************************************************************
   Include Files
 ****************************************************************************************/
#include <Arduino.h>
#include <SoftwareSerial.h>

#include "bg96.h"

/****************************************************************************************
   Defines
 ****************************************************************************************/
//Pin define
#define bg96_W_DISABLE 29
#define bg96_RESET 28
#define bg96_PWRKEY 2
#define bg96_GPS_EN 39

/****************************************************************************************
   Private type declarations
 ****************************************************************************************/

/****************************************************************************************
   Private function declarations
 ****************************************************************************************/

/****************************************************************************************
   Variable declarations
 ****************************************************************************************/
String bg96_rsp = "";
char GSM_RSP[1600] = {0};

static uint16_t rxReadIndex  = 0;
static uint16_t rxWriteIndex = 0;
static uint16_t rxCount      = 0;
static uint8_t Gsm_RxBuf[GSM_RXBUF_MAXSIZE];

/****************************************************************************************
   Public functions
 ****************************************************************************************/
//bg96 power up
void bg96_init()
{
  pinMode(bg96_RESET, OUTPUT);
  pinMode(bg96_PWRKEY, OUTPUT);
  pinMode(bg96_GPS_EN, OUTPUT);
  pinMode(bg96_W_DISABLE, OUTPUT);

  digitalWrite(bg96_RESET, 0);
  digitalWrite(bg96_PWRKEY, 1);
  digitalWrite(bg96_W_DISABLE, 1);
  delay(2000);
  digitalWrite(bg96_PWRKEY, 0);
  digitalWrite(bg96_GPS_EN, 1);
  delay(2000);
}

//this function is suitable for most AT commands of bg96. e.g. bg96_at("ATI")
void bg96_at_wait_rsp(char *at)
{
  char tmp[MAX_CMD_LEN] = {0};
  int ret = -1;
  int len = strlen(at);

  if ((at != NULL) && (len <= MAX_CMD_LEN))
  {
    strncpy(tmp, at, len);
    tmp[len] = '\r';
    Serial1.write(tmp);
    delay(10);
    memset(GSM_RSP, 0, 1600);
    ret = Gsm_WaitRspOK(GSM_RSP, GSM_GENER_CMD_TIMEOUT * 8, true);
    Serial.printf("ret %d ; %s\r\n", ret, GSM_RSP);
  } else {
    Serial.printf("AT cmd too long\r\n");
  }
}

//this function is suitable for most AT commands of bg96. e.g. bg96_at("ATI")
void bg96_at(char *at)
{
  char tmp[256] = {0};
  int len = strlen(at);
  strncpy(tmp, at, len);
  tmp[len] = '\r';
  Serial1.write(tmp);
  delay(10);
  while (Serial1.available()) {
    bg96_rsp += char(Serial1.read());
    delay(2);
  }
  Serial.println(bg96_rsp);
  bg96_rsp = "";
}

//gps data
eGnssCodes_t eGNSS_GetPosition(sPosition_t * p_psPosition)
{
  int ret = -1;
  eGnssCodes_t l_eCode = GNSS_C_SUCCESS;
  int32_t l_s32scanResult = -1;
  int32_t l_s32Time = 0;
  int32_t l_s32DecimalTime = 0;
  int32_t l_s32Date = 0;
  char l_sLatitude[15u], l_sLongitude[15u], l_sHdop[5u], l_sAltitude[10u], l_sCourseOverGround[10u], l_sSpeedKph[10u], l_sSpeedKnots[10u] = {0};
  
  /* send GPS command to BG96 */
  memset(GSM_RSP, 0, GSM_GENER_CMD_LEN);
  Serial1.write("AT+QGPSLOC=2\r");
  ret = Gsm_WaitRspOK(GSM_RSP, GSM_GENER_CMD_TIMEOUT * 4, true);
#ifdef DEBUG
  Serial.printf("AT+QGPSLOC %d\r\n", ret);
#endif

  if ((ret == 0) && (p_psPosition != NULL))
  {
      /* init buffers */
      memset(l_sLatitude, 0, 15);
      memset(l_sLongitude, 0, 15);
      memset(l_sHdop, 0, 5);
      memset(l_sAltitude, 0, 10);
      memset(l_sCourseOverGround, 0, 10);
      memset(l_sSpeedKph, 0, 10);
      memset(l_sSpeedKnots, 0, 10);

      /* parse response */
      l_s32scanResult  = sscanf(strstr(GSM_RSP, "+QGPSLOC:"), "+QGPSLOC: %d.%d,%[0-9.],%[0-9.],%[0-9.],%[0-9.],%c,%[0-9.],%[0-9.],%[0-9.],%d,%hu",
                            (int*) & (l_s32Time),
                            (int*) & (l_s32DecimalTime),
                            l_sLatitude,
                            l_sLongitude,
                            l_sHdop,
                            l_sAltitude,
                            &(p_psPosition->u8FixType),
                            l_sCourseOverGround,
                            l_sSpeedKph,
                            l_sSpeedKnots,
                            (int*) & (l_s32Date),
                            &(p_psPosition->u16Satellites));

       /* convert to float */
       p_psPosition->f32Latitude = atof(l_sLatitude);
       p_psPosition->f32Longitude = atof(l_sLongitude);
       p_psPosition->f32Hdop = atof(l_sHdop);
       p_psPosition->f32Altitude = atof(l_sAltitude);
       p_psPosition->f32CourseOverGround = atof(l_sCourseOverGround);
       p_psPosition->f32Speedkph = atof(l_sSpeedKph);
       p_psPosition->f32Speedknots = atof(l_sSpeedKnots);

    if (l_s32scanResult >= 11)
    {
      if (p_psPosition->u8FixType > 0x30)
      {
        p_psPosition->u8FixType -= 0x30;
      }

      /* split time from hhmmss to hh mm ss */
      p_psPosition->u8Seconds = l_s32Time % 100;
      l_s32Time -= p_psPosition->u8Seconds;
      l_s32Time /= 100;

      p_psPosition->u8Minutes = l_s32Time % 100;
      l_s32Time -= p_psPosition->u8Minutes;
      l_s32Time /= 100;

      p_psPosition->u8Hours = l_s32Time % 100;

      /* split date from ddmmyy to dd mm yy */
      p_psPosition->u8Year = l_s32Date % 100;
      l_s32Date -= p_psPosition->u8Year;
      l_s32Date /= 100;

      p_psPosition->u8Month = l_s32Date % 100;
      l_s32Date -= p_psPosition->u8Month;
      l_s32Date /= 100;

      p_psPosition->u8Day = l_s32Date % 100;

#ifdef DEBUG
      Serial.println("Parse GPS OK");

      Serial.printf("Date = %u/%u/%u %u:%u:%u\r\n", p_psPosition->u8Day, p_psPosition->u8Month, 
                      p_psPosition->u8Year, p_psPosition->u8Hours, p_psPosition->u8Minutes, p_psPosition->u8Seconds);
                      
      Serial.printf("Lat = %f\r\n", p_psPosition->f32Latitude);
      Serial.printf("Long = %f\r\n", p_psPosition->f32Longitude);
      Serial.printf("Hdop = %f\r\n", p_psPosition->f32Hdop);
      Serial.printf("altitude = %f\r\n", p_psPosition->f32Altitude);
      Serial.printf("Fix type = %u\r\n", p_psPosition->u8FixType);
      Serial.printf("course over ground = %f\r\n", p_psPosition->f32CourseOverGround);
      Serial.printf("speed kph = %f\r\n", p_psPosition->f32Speedkph);
      Serial.printf("speed knots = %f\r\n", p_psPosition->f32Speedknots);
      Serial.printf("satellites = %hu\r\n", p_psPosition->u16Satellites);
#endif

      /* Fix with position */
      l_eCode = GNSS_C_SUCCESS;
    } else {
      /* Parsing error, consider no position (?) */
      l_eCode = GNSS_ERROR_NO_POSITION;
    #ifdef DEBUG
      Serial.printf("Parse GPS FAIL ; scan = %d\r\n", l_s32scanResult);
    #endif
    }
  }
  else
  {
  #ifdef DEBUG
    Serial.println("GPS FAILED");
  #endif
  }

  return l_eCode;
}

//network connect
void connect(uint8_t p_u8Flag) {

  //Serial.println("get network info...");
  //bg96_at("AT+QNWINFO");
  //delay(2000);

  //Serial.println("QIACT...");
  bg96_at_wait_rsp("AT+QIACT=1");
  //delay(2000);

  //bg96_at("AT+QIACT=?");
  //delay(2000);

  //Serial.println("get time");
  //bg96_at("AT+QLTS=1"); //query GMT time from network
  //delay(2000);

  bg96_at_wait_rsp("AT+QHTTPCFG=\"contextid\",1");
  //delay(2000);

  bg96_at_wait_rsp("AT+QHTTPCFG=\"responseheader\",1");
  //delay(2000);

  //GET request
  //bg96_at("AT+QHTTPURL=17,80");
  //delay(3000);
  //Serial1.write("http://google.fr/\r");
  //delay(2000);
  //bg96_at("AT+QHTTPGET=80");
  //delay(2000);

  //POST request
  bg96_at("AT+QHTTPURL=57,80"); //57 is length of the url
  delay(3000);
  Serial1.write("https://webhook.site/b80027c3-ec69-4694-b32d-b640549c6213\r");
  delay(3000);
  bg96_at("AT+QHTTPPOST=58,80,80");//48 is length of the post data
  delay(3000);
  //Serial1.write("{location:{position:{lat:49.12345,lon:0.12345}}}\r");

  if (p_u8Flag == 0u)
  {
    // empty
    Serial1.write("{TOR_state: {TOR1_current_state: 1,TOR2_current_state: 0}}\r");
  }
  else
  {
    // full
    Serial1.write("{TOR_state: {TOR1_current_state: 0,TOR2_current_state: 1}}\r");
  }
  //delay(3000);

  //bg96_at("AT+QHTTPREAD=80");
  //delay(3000);
  /*bg96_rsp="";
    while (Serial1.available()) {
    bg96_rsp += char(Serial1.read());
    delay(2);
    }
    Serial.print(bg96_rsp); */
}

/**@brief Set Apn information
   @param p_pchApn         Apn operator address
   @param p_pchUser        Apn access username
   @param p_pchPassword    Apn access password

   @info Always use IPv4 context

   @retval BG96_SUCCESS
   @retval BG96_ERROR_FAILED
   @retval BG96_ERROR_PARAM
*/
eBG96ErrorCode_t eBG96_SetApnContext(char * p_pchApn, char * p_pchUser, char * p_pchPassword)
{
  eBG96ErrorCode_t l_eCode = BG96_SUCCESS;
  char l_achCmd[MAX_CMD_LEN] = {0};

  /* By default always use IPv4 context */
  if ((p_pchApn != NULL) && (p_pchUser != NULL) && (p_pchPassword != NULL))
  {
    snprintf(l_achCmd, MAX_CMD_LEN, "AT+QICSGP=1,1,\"%s\",\"%s\",\"%s\",1", p_pchApn, p_pchUser, p_pchPassword);
    bg96_at(l_achCmd);
    l_eCode = BG96_SUCCESS;
  } else {
    l_eCode = BG96_ERROR_PARAM;
  }

  return l_eCode;

}

/**@brief Active TCP/IP context
   @param p_pchIp         Allocated IP returned by server
   @retval BG96_SUCCESS
   @retval BG96_ERROR_FAILED
   @retval BG96_ERROR_PARAM
*/
eBG96ErrorCode_t eBG96_ActiveContext(void)
{
  bg96_at("AT+QIACT=1");
  return BG96_SUCCESS;
}

int Gsm_RxByte(void)
{
  int c = -1;

  //__disable_irq();
  //if (rxCount > 0)
  {
    //c = Gsm_RxBuf[rxReadIndex];
    if (Serial1.available() > 0)
    {
      c = char(Serial1.read());
    }

    rxReadIndex++;
    if (rxReadIndex == GSM_RXBUF_MAXSIZE)
    {
      rxReadIndex = 0;
    }
    //rxCount--;
  }
  //__enable_irq();

  return c;
}

int Gsm_WaitRspOK(char *rsp_value, uint16_t timeout_ms, uint8_t is_rf)
{
  int ret = -1, wait_len = 0;
  char len[10] = {0};
  uint16_t time_count = timeout_ms;
  uint32_t i = 0;
  int       c;
  char *cmp_p = NULL;

  wait_len = is_rf ? strlen(GSM_CMD_RSP_OK_RF) : strlen(GSM_CMD_RSP_OK);

  /*if(g_type == GSM_TYPE_FILE)
    {
      do
      {
          c = Gsm_RxByte();
          if(c < 0)
          {
              time_count--;
              delay_ms(1);
              continue;
          }

          rsp_value[i++] = (char)c;
          //NRF_LOG_INFO("%02X", rsp_value[i - 1]);
          time_count--;
      }
      while(time_count > 0);
    }
    else*/
  {
    memset(GSM_RSP, 0, 1600);
    do
    {
      int c;
      c = Gsm_RxByte();
      if (c < 0)
      {
        time_count--;
        delay(1);
        continue;
      }

      GSM_RSP[i++] = (char)c;

      if (i >= 0 && rsp_value != NULL)
      {
        if (is_rf)
          cmp_p = strstr(GSM_RSP, GSM_CMD_RSP_OK_RF);
        else
          cmp_p = strstr(GSM_RSP, GSM_CMD_RSP_OK);
        if (cmp_p)
        {
          if (i > wait_len && rsp_value != NULL)
          {
            //SEGGER_RTT_printf(0,"--%s  len=%d\r\n", rsp_value, i);
            memcpy(rsp_value, GSM_RSP, i);
          }
          ret = 0;
          break;
        }
      }
    }
    while (time_count > 0);
  }

  return ret;
}

/****************************************************************************************
   Private functions
 ****************************************************************************************/

/****************************************************************************************
   End Of File
 ****************************************************************************************/
