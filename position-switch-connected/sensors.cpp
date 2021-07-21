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
static void nrf_io3_it_cb(void);        // trailer full interrupt CB
static void nrf_io4_it_cb(void);        // trailer empty interrupt CB

/****************************************************************************************
 * Variable declarations
 ****************************************************************************************/
TwoWire *wi = &Wire;
static s_SensorMngrData_t g_sSensorsData = { 0 };

// switch event flags
uint8_t g_u8FlagFull = 0;     // trailer full flag
uint8_t g_u8FlagEmpty = 0;    // trailer empty flag
uint8_t g_u8SwitchEventReady = 0u;

/****************************************************************************************
 * Public functions
 ****************************************************************************************/ 
 
 /**@brief   Function to init all sensors data.
 */
void vSensorMngr_Init(void)
{
  pinMode(NRF_IO3, INPUT_PULLUP);
  pinMode(NRF_IO4, INPUT_PULLUP);
  
  attachInterrupt(digitalPinToInterrupt(NRF_IO3), nrf_io3_it_cb, FALLING);
  attachInterrupt(digitalPinToInterrupt(NRF_IO4), nrf_io4_it_cb, FALLING);
  
  // read initial switch state
  vSensorMngr_ReadnUpdateSwitch();
}

 
 /**@brief   Function to get all sensors data.
 * @return  pointer on sensors data structure.
 */
s_SensorMngrData_t *psSensorMngr_GetSensorData(void)
{
 return &g_sSensorsData;
}

/**@brief   Function to read and update switch position values
 * @return  Error code.
 */
void vSensorMngr_ReadnUpdateSwitch(void)
{
  if (digitalRead(NRF_IO3) == LOW)    // pullup
  {
    g_sSensorsData.u8TOR1 = 1u;
  }else{
    g_sSensorsData.u8TOR1 = 0u;
  }
  
  if (digitalRead(NRF_IO4) == LOW)    // pullup
  {
    g_sSensorsData.u8TOR2 = 1u;
  }else{
    g_sSensorsData.u8TOR2 = 0u;
  }
}

/**@brief   Function to update switch position values
 * @return  Error code.
 */
e_SensorMngr_ErrorCode_t eSensorMngr_UpdateSwitch(void)
{
  if (g_u8FlagFull == 1u)
  {
    g_u8SwitchEventReady = 1u;
    g_u8FlagFull = 0u;
    g_sSensorsData.u8TOR1 = 1u;
    
    if (digitalRead(NRF_IO4) == HIGH)       // check if other position deactivate (pullup)
    {
      g_sSensorsData.u8TOR2 = 0u;
      
      // blink led once
      digitalWrite(LED_GREEN_PIN, HIGH);
      delay(250);
      digitalWrite(LED_GREEN_PIN, LOW);
      delay(250);

    #ifdef DEBUG
      Serial.printf("TOR1 switched ON\r\n");
    #endif
    }
  }

  // trailer is empty
  if (g_u8FlagEmpty == 1u)
  {
    g_u8SwitchEventReady = 1u;
    g_u8FlagEmpty = 0u;
    g_sSensorsData.u8TOR2 = 1u;
    
    if (digitalRead(NRF_IO3) == HIGH)       // check if other position deactivate (pullup)
    {
      g_sSensorsData.u8TOR1 = 0u;
      
      // blink led once
      digitalWrite(LED_GREEN_PIN, HIGH);
      delay(250);
      digitalWrite(LED_GREEN_PIN, LOW);
      delay(250);

    #ifdef DEBUG
      Serial.printf("TOR2 switched ON\r\n");
    #endif
    }
  }
}

/**@brief   Function to get if switch event is ready to be sent
 * @return  1 if ready, 0 if not.
 */
uint8_t u8SensorMngr_SwitchEventReadyGet(void)
{
  return g_u8SwitchEventReady;
}

/**@brief   Function to set if a switch event is ready to be sent
 * @param   p_u8IsReady : 1 is ready, 0 is not
 */
void vSensorMngr_SwitchEventReadySet(uint8_t p_u8IsReady)
{
  if ((p_u8IsReady == 0u) || (p_u8IsReady == 1u))
  {
    g_u8SwitchEventReady = p_u8IsReady; 
  }
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

/****************************************************************************************
 * Private functions
 ****************************************************************************************/
static void nrf_io3_it_cb() {
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time > 200) 
  {
    g_u8FlagFull = 1;
  }
  last_interrupt_time = interrupt_time;
}

static void nrf_io4_it_cb() {
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time > 200) 
  {
    g_u8FlagEmpty = 1;
  }
  last_interrupt_time = interrupt_time;
}

/****************************************************************************************
 * End Of File
 ****************************************************************************************/
