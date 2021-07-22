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
#include <Arduino.h>

#include "BG96.h"
#include "timeout.h"
#include "config.h"

#include "sensors.h"

/****************************************************************************************
 * Defines
 ****************************************************************************************/

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
static s_SensorMngrData_t g_sSensorsData = { 0 };

/****************************************************************************************
 * Public functions
 ****************************************************************************************/ 
 
 /**@brief   Function to init all sensors data.
 */
void vSensorMngr_Init(void)
{  
  __NOP();
}

 
 /**@brief   Function to get all sensors data.
 * @return  pointer on sensors data structure.
 */
s_SensorMngrData_t *psSensorMngr_GetSensorData(void)
{
 return &g_sSensorsData;
}

 /**@brief   Function to get TOR state.
 */
uint8_t u8SensorMngr_TORStateGet(uint8_t p_u8TORIndex)
{
  return (g_sSensorsData.au8TORs[p_u8TORIndex]);
}

 /**@brief   Function to set TOR state.
 */
void vSensorMngr_TORStateSet(uint8_t p_u8TORIndex, uint8_t p_u8State)
{
  // First save current state in previous
  g_sSensorsData.au8TORsPrevious[p_u8TORIndex] = g_sSensorsData.au8TORs[p_u8TORIndex];
  // Then set state
  g_sSensorsData.au8TORs[p_u8TORIndex] = p_u8State;
}

/**@brief   Function to set gps position.
 * @param   p_sPosition : GPS position
 * @return  None
 */
void vSensorMngr_PositionSet(sPosition_t p_sPosition)
{
  /* Copy Position in global variable */
  memcpy(&(g_sSensorsData.sPosition), &p_sPosition, sizeof(sPosition_t));
}


/****************************************************************************************
 * Private functions
 ****************************************************************************************/

/****************************************************************************************
 * End Of File
 ****************************************************************************************/
