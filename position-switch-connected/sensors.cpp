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
 * Description:   Sensors mngt
 *
 */

/****************************************************************************************
 * Include Files
 ****************************************************************************************/
#include <SoftwareSerial.h>
#include <Wire.h>
#include <Arduino.h>

#include "BG96.h"
#include "GNSS.h"
#include "timeout.h"
#include "config.h"

#include "sensors.h"

/****************************************************************************************
 * Defines
 ****************************************************************************************/
#define lis3dh_addr   0x19
#define opt3001_addr  0x44
#define shtc3_addr  0x70
#define lps22hb_addr  0x5c

/****************************************************************************************
 * Private type declarations
 ****************************************************************************************/

/****************************************************************************************
 * Private function declarations
 ****************************************************************************************/

/****************************************************************************************
 * Variable declarations
 ****************************************************************************************/
TwoWire *wi = &Wire;
static s_SensorMngrData_t g_sSensorsData = { 0 };

/****************************************************************************************
 * Public functions
 ****************************************************************************************/ 
 
 /**@brief   Function to get all sensors data.
 * @return  pointer on sensors data structure.
 */
s_SensorMngrData_t *psSensorMngr_GetSensorData(void)
{
 return &g_sSensorsData;
}

/**@brief   Function to perform a gps position.
 * @param p_u32TimeoutInSeconds Desired timeout for position acquisition
 * @return  Error code.
 */
e_SensorMngr_ErrorCode_t eSensorMngr_UpdatePosition(uint32_t p_u32TimeoutInSeconds)
{
  e_SensorMngr_ErrorCode_t l_eSensorMngrErrorCode = SENSOR_MNGR_SUCCESS;
  eGnssCodes_t l_eGNSSCode = GNSS_ERROR_FAILED;
  uint32_t l_u32TimeStart = u32Time_getMs();
  uint32_t l_u32TimeOut = 0u;
  sPosition_t l_sPosition = {0};

  if(p_u32TimeoutInSeconds > TIME_TO_FIX_MAX)
  {
    l_u32TimeOut = u32Time_getMs() + SECOND_TO_MS(TIME_TO_FIX_MAX);
  }else{
    l_u32TimeOut = u32Time_getMs() + SECOND_TO_MS(p_u32TimeoutInSeconds);
  }

  /* Save current Time at start */
  l_u32TimeStart = u32Time_getMs();

  /* Wait for a valid GPS position */
  while((l_eGNSSCode != GNSS_ERROR_TIMEOUT) &&
      ((l_sPosition.f32Hdop <= 0.0f) || (l_sPosition.f32Hdop > 4.0f)) &&
      (u32Time_getMs() < l_u32TimeOut))
  {
    /* Request for a position */
    l_eGNSSCode = eGNSS_GetPosition(&l_sPosition);
   #ifdef DEBUG
    Serial.printf("[%u] Wait Position [%u s]? %s\r\n",
             (unsigned int)u32Time_getMs(),
             (unsigned int) MS_TO_SECOND(l_u32TimeOut - (unsigned int)u32Time_getMs()),
             ((l_eGNSSCode == GNSS_C_SUCCESS) ? "FIX" : "NO_POSITION"));
   #endif

    /* Retry each second, less is not necessary */
    vTime_WaitMs(1000);

    /* Check if HDOP is acceptable, or the fix timeout is reached */

  };

  /* calculate time to fix */
  l_sPosition.u8TimeToFix = (u32Time_getMs() - l_u32TimeStart) / 1000u;
#ifdef DEBUG
  Serial.printf("[%u] GNSSFIX (%u ms)\r\n", (unsigned int)u32Time_getMs(), (unsigned int)(u32Time_getMs() - l_u32TimeStart));
#endif

  /* If fix gps failed then set default values */
  if(l_sPosition.f32Hdop == 0.0f)
  {
    l_sPosition.u8Day = 1u;
    l_sPosition.u8Month = 1u;
    l_sPosition.u8Year = 20u;
  }

  /* Copy Position in global variable */
  memcpy(&(g_sSensorsData.sPosition), &l_sPosition, sizeof(sPosition_t));

  if(GNSS_C_SUCCESS != l_eGNSSCode)
  {
    l_eSensorMngrErrorCode = SENSOR_MNGR_ERROR;
  }else{
    l_eSensorMngrErrorCode = SENSOR_MNGR_SUCCESS;
  }

  return l_eSensorMngrErrorCode;
}

 //sensor init
void sensor_init()
{
  wi->beginTransmission(lis3dh_addr);
  wi->write(byte(0x20));
  wi->write(byte(0x57));  
  wi->endTransmission();
  delay(5);
  wi->beginTransmission(lis3dh_addr);
  wi->write(byte(0x23));
  wi->write(byte(0x08));  
  wi->endTransmission();
  delay(5);  

  wi->beginTransmission(shtc3_addr);
  wi->write(byte(0x35));
  wi->write(byte(0x17));  
  wi->endTransmission();
  delay(5);  
  
  wi->beginTransmission(shtc3_addr);
  wi->write(byte(0xEF));
  wi->write(byte(0xC8));  
  wi->endTransmission();
  delay(5);


  wi->beginTransmission(opt3001_addr);
  wi->write(byte(0x01));
  wi->write(byte(0xCC));
  wi->write(byte(0x10));  
  wi->endTransmission();
  delay(5);
  
  wi->beginTransmission(lps22hb_addr);
  wi->write(byte(0x10));
  wi->write(byte(0x50)); 
  wi->endTransmission();
}

//acc data
void acc_data_show()
{
  byte acc_h;
  byte acc_l;
  int x = 0;
  int y = 0;
  int z = 0;
  float accx;
  float accy;
  float accz;
  wi->beginTransmission(lis3dh_addr);
  wi->write(byte(0x28));
  wi->endTransmission();
  wi->requestFrom(lis3dh_addr, 1);
  while(wi->available()){
    acc_l = wi->read();
  } 
  wi->beginTransmission(lis3dh_addr);
  wi->write(byte(0x29));
  wi->endTransmission();
  wi->requestFrom(lis3dh_addr, 1);
  while(wi->available()){
    acc_h = wi->read();
  }
  x = (acc_h << 8) | acc_l;
  if(x<0x8000){x=x;}else{x=x-0x10000;}
  accx = x*4000/65536.0;
  
  wi->beginTransmission(lis3dh_addr);
  wi->write(byte(0x2a));
  wi->endTransmission();
  wi->requestFrom(lis3dh_addr, 1);
  while(wi->available()){
    acc_l = wi->read();
  } 
  wi->beginTransmission(lis3dh_addr);
  wi->write(byte(0x2b));
  wi->endTransmission();
  wi->requestFrom(lis3dh_addr, 1);
  while(wi->available()){
    acc_h = wi->read();
  }
  y = (acc_h << 8) | acc_l;
  if(y<0x8000){y=y;}else{y=y-0x10000;}
  accy = y*4000/65536.0;
  
  wi->beginTransmission(lis3dh_addr);
  wi->write(byte(0x2c));
  wi->endTransmission();
  wi->requestFrom(lis3dh_addr, 1);
  while(wi->available()){
    acc_l = wi->read();
  } 
  wi->beginTransmission(lis3dh_addr);
  wi->write(byte(0x2d));
  wi->endTransmission();
  wi->requestFrom(lis3dh_addr, 1);
  while(wi->available()){
    acc_h = wi->read();
  }
  z = (acc_h << 8) | acc_l;
  if(z<0x8000){z=z;}else{z=z-0x10000;}
  accz = z*4000/65536.0;
  Serial.print("ACC(mg): x=");
  Serial.print(accx);
  Serial.print(" y=");
  Serial.print(accy);
  Serial.print(" z=");
  Serial.println(accz);  
}

//light data
void light_show()
{
  byte tmp[2];
  int i=0;
  int m;
  int e;
  double h;
  float light;
  wi->beginTransmission(opt3001_addr);
  wi->write(byte(0x00));
  wi->endTransmission();
  wi->requestFrom(opt3001_addr, 2);
  while(wi->available()){
    tmp[i++] = wi->read();
  }
  m=((tmp[0]<<8) | tmp[1]) & 0x0FFF;
  e=(((tmp[0]<<8) | tmp[1]) & 0xF000) >> 12;
  h= pow(2,e);
  light = m*(0.01*h);
  Serial.print("Light=");
  Serial.println(light); 
}

//pressure data
void pressure_data_show()
{
  byte xl;
  byte l;
  byte h;
  int pre;
  float p;
  wi->beginTransmission(lps22hb_addr);
  wi->write(byte(0x28));
  wi->endTransmission();
  wi->requestFrom(lps22hb_addr, 1);
  while(wi->available()){
    xl = wi->read();
  } 
  wi->beginTransmission(lps22hb_addr);
  wi->write(byte(0x29));
  wi->endTransmission();
  wi->requestFrom(lps22hb_addr, 1);
  while(wi->available()){
    l = wi->read();
  }
  
  wi->beginTransmission(lps22hb_addr);
  wi->write(byte(0x2a));
  wi->endTransmission();
  wi->requestFrom(lps22hb_addr, 1);
  while(wi->available()){
    h = wi->read();
  } 
  pre = (h<<16) | (l<<8) | xl;
  if(pre & 0x00800000){
    pre |= 0xFF000000;
  }
  p= pre/4096.0;
  Serial.print("Pressure(HPa) =");
  Serial.println(p); 
}

//temperature & humidity
void environment_data_show()
{
  byte t[6];
  int i=0;
  float _temperature;
  float _humidity;
  wi->beginTransmission(shtc3_addr);
  wi->write(byte(0x7C));
  wi->write(byte(0xA2));  
  wi->endTransmission();
  
  wi->beginTransmission(shtc3_addr);
  wi->endTransmission();
  wi->requestFrom(shtc3_addr, 6);
  while(wi->available()){
    t[i++] = wi->read();
  } 
  _temperature= (t[1]|(t[0]<<8))*175/65536.0 -45.0;
  _humidity=(t[4]|(t[3]<<8))*100/65536.0;
  Serial.print("Temperature =");
  Serial.print(_temperature); 
  Serial.print(" humidity =");
  Serial.println(_humidity);  
  Serial.println("");
}

/****************************************************************************************
 * Private functions
 ****************************************************************************************/

/****************************************************************************************
 * End Of File
 ****************************************************************************************/
