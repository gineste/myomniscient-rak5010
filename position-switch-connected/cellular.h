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
#ifndef CELLULAR_H_
#define CELLULAR_H_

/****************************************************************************************
 * Include Files
 ****************************************************************************************/

/****************************************************************************************
 * Defines
 ****************************************************************************************/

/****************************************************************************************
 * Type definitions
 ****************************************************************************************/
typedef struct _SPTK_NET_INFO_ {
  char * pchOperator;
  char * pchNetTech;
  char * pchBandTech;
  float f32Rate;
  int16_t s16Rssi;
} sSptkNetInfo_t;

typedef enum _CELLULAR_ERR_CODE_{
  CELLULAR_SUCCESS,
   CELLULAR_ERROR_PARAM,
  CELLULAR_ERROR_FAILED,
  CELLULAR_ERROR_CONFIG,
  CELLULAR_ERROR_APN,
  CELLULAR_ERROR_NOT_REGISTERED,
  CELLULAR_ERROR_TIMEOUT
}eCellularErrorCode_t;

/****************************************************************************************
 * Public function declarations
 ****************************************************************************************/
 void vCellular_SendData(void);

#endif /* CELLULAR_H_ */
