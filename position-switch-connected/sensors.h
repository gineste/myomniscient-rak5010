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
 */
#ifndef SENSORS_H_
#define SENSORS_H_

/****************************************************************************************
 * Include Files
 ****************************************************************************************/
#include <stdint.h>

/****************************************************************************************
 * Defines
 ****************************************************************************************/
#define TIME_TO_FIX_MAX       (120u)   /* GPS time to fix max in seconds */

#define TOR_NUMBER            (2u)

/****************************************************************************************
 * Type definitions
 ****************************************************************************************/
typedef enum _SENSOR_MNGR_ERROR_ {
  SENSOR_MNGR_SUCCESS,
  SENSOR_MNGR_ERROR_PARAM,
  SENSOR_MNGR_ERROR
} e_SensorMngr_ErrorCode_t;

typedef enum _SENSOR_MNGR_TORS_ {
  SENSOR_MNGR_TOR2,
  SENSOR_MNGR_TOR1
} e_SensorMngr_TORs_t;

typedef struct _GNSS_POSITION_ {
   uint8_t u8Hours;
   uint8_t u8Minutes;
   uint8_t u8Seconds;
   uint8_t u8Day;
   uint8_t u8Month;
   uint8_t u8Year;

   float f32Latitude;
   float f32Longitude;
   float f32Altitude;

   float f32CourseOverGround;
   float f32Speedkph;
   float f32Speedknots;

   float f32Hdop;
   uint16_t u16Satellites;
   uint8_t u8FixType;

   uint8_t u8TimeToFix;

} sPosition_t;

typedef struct __attribute__ ((__packed__)) _SENSOR_MNGR_DATA_  {
   uint8_t      au8TORs[TOR_NUMBER];          /* switch 0 : full ; switch 1 : empty */
   uint8_t      au8TORsPrevious[TOR_NUMBER];  /* previous TORs position */
   sPosition_t  sPosition;                    /* Last gps position */
   uint32_t     u32TsPosition;                /* UTC timestamp of last position */
}s_SensorMngrData_t;

/****************************************************************************************
 * Public function declarations
 ****************************************************************************************/
/**@brief   Function to init all sensors data.
 */
void vSensorMngr_Init(void);

/**@brief   Function to get all sensors data.
 * @return  sensors data structure.
 */
s_SensorMngrData_t *psSensorMngr_GetSensorData(void);

uint8_t u8SensorMngr_TORStateGet(uint8_t p_u8TORIndex);

 /**@brief   Function to set TOR state.
 */
void vSensorMngr_TORStateSet(uint8_t p_u8TORIndex, uint8_t p_u8State);

/**@brief   Function to update switch position values
 * @return  Error code.
 */
e_SensorMngr_ErrorCode_t eSensorMngr_UpdateSwitch(void);

/**@brief   Function to set gps position.
 * @param   p_sPosition : GPS position
 * @param   p_u32TsPosition : Timestamp UTC of position
 * @return  None
 */
void vSensorMngr_PositionSet(sPosition_t p_sPosition, uint32_t p_u32TsPosition);

#endif /* SENSORS_H_ */
