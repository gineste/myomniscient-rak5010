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
#ifndef GNSS_H_
#define GNSS_H_

/****************************************************************************************
 * Include Files
 ****************************************************************************************/

/****************************************************************************************
 * Defines
 ****************************************************************************************/

/****************************************************************************************
 * Type definitions
 ****************************************************************************************/
typedef enum _GNSS_CODES_ {
   GNSS_C_SUCCESS,
  GNSS_ERROR_PARAM,
  GNSS_ERROR_FAILED,
  GNSS_ERROR_NO_POSITION,
  GNSS_ERROR_TIMEOUT
} eGnssCodes_t;

/****************************************************************************************
 * Public function declarations
 ****************************************************************************************/
 eGnssCodes_t eGNSS_TurnOn(void);
 eGnssCodes_t eGNSS_TurnOff(void);
 eGnssCodes_t eGNSS_UpdatePosition(uint32_t p_u32TimeoutInSeconds);

#endif /* GNSS_H_ */
