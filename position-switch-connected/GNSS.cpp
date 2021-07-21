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
   Description:   GNSS app

*/

/****************************************************************************************
   Include Files
 ****************************************************************************************/
#include <Arduino.h>

#include "BG96.h"
#include "timeout.h"

#include "GNSS.h"

/****************************************************************************************
   Defines
 ****************************************************************************************/

/****************************************************************************************
   Private type declarations
 ****************************************************************************************/

/****************************************************************************************
   Private function declarations
 ****************************************************************************************/

/****************************************************************************************
   Variable declarations
 ****************************************************************************************/
char GNSS_RSP[1600] = {0};

/****************************************************************************************
   Public functions
 ****************************************************************************************/
/**@brief Turn on GNSS module on BG96
 * @param None
 * @retval BG96_SUCCESS
 * @retval BG96_ERROR_FAILED
 * @retval BG96_ERROR_PARAM
 */
eGnssCodes_t eGNSS_TurnOn(void)
{
  eGnssCodes_t l_eCode = GNSS_ERROR_FAILED;
  
  digitalWrite(bg96_GPS_EN, HIGH);
  vTime_WaitMs(10);
  
  if(BG96_SUCCESS == eBG96_SendCommand("AT+QGPS=1", GSM_CMD_RSP_OK_RF, CMD_TIMEOUT))
  {
    l_eCode = GNSS_C_SUCCESS;
  }else{
    l_eCode = GNSS_ERROR_FAILED;
  }

  return l_eCode;
}

/**@brief Turn on GNSS module on BG96
 * @param None
 * @retval BG96_SUCCESS
 * @retval BG96_ERROR_FAILED
 * @retval BG96_ERROR_PARAM
 */
eGnssCodes_t eGNSS_TurnOff(void)
{
  eGnssCodes_t l_eCode = GNSS_ERROR_FAILED;

  if (BG96_SUCCESS == eBG96_SendCommand("AT+QGPSEND", GSM_CMD_RSP_OK_RF, CMD_TIMEOUT))
  {
    l_eCode = GNSS_C_SUCCESS;
  }else{
    l_eCode = GNSS_ERROR_FAILED;
  }
  digitalWrite(bg96_GPS_EN, LOW);

  return l_eCode;
}

eGnssCodes_t eGNSS_GetPosition(sPosition_t * p_psPosition)
{
  eBG96ErrorCode_t l_eBG96Code = BG96_SUCCESS;
  eGnssCodes_t l_eCode = GNSS_C_SUCCESS;
  int32_t l_s32scanResult = -1;
  int32_t l_s32Time = 0;
  int32_t l_s32DecimalTime = 0;
  int32_t l_s32Date = 0;
  char l_sLatitude[15u], l_sLongitude[15u], l_sHdop[5u], l_sAltitude[10u], l_sCourseOverGround[10u], l_sSpeedKph[10u], l_sSpeedKnots[10u] = {0};
  
  /* send GPS command to BG96 */
  memset(GNSS_RSP, 0, GSM_GENER_CMD_LEN);
  Serial1.write("AT+QGPSLOC=2\r");
  l_eBG96Code = eBG96_WaitResponse(GNSS_RSP, CMD_TIMEOUT, GSM_CMD_RSP_OK_RF);

  if ((l_eBG96Code == GNSS_C_SUCCESS) && (p_psPosition != NULL))
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
      l_s32scanResult  = sscanf(strstr(GNSS_RSP, "+QGPSLOC:"), "+QGPSLOC: %d.%d,%[0-9.],%[0-9.],%[0-9.],%[0-9.],%c,%[0-9.],%[0-9.],%[0-9.],%d,%hu",
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
      /* Parsing error, consider no position */
      l_eCode = GNSS_ERROR_NO_POSITION;
    #ifdef DEBUG
      Serial.printf("Parse GPS FAIL ; scan = %d\r\n", l_s32scanResult);
    #endif
    }
  }
  else
  {
    /* no position */
    l_eCode = GNSS_ERROR_NO_POSITION;
  #ifdef DEBUG
    Serial.println("GPS FAILED");
  #endif
  }

  return l_eCode;
}
/****************************************************************************************
   Private functions
 ****************************************************************************************/

/****************************************************************************************
   End Of File
 ****************************************************************************************/