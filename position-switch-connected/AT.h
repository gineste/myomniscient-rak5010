/*
 * AT.h
 *
 *  Created on: 17 févr. 2020
 *      Author: Martin
 */

/*
 *    ____  _  _   __   ____   __    ___    ____  _  _  ____  ____  ____  _  _  ____
 *   (  __)( \/ ) /  \ (_  _) (  )  / __)  / ___)( \/ )/ ___)(_  _)(  __)( \/ )/ ___)
 *    ) _)  )  ( (  O )  )(    )(  ( (__   \___ \ )  / \___ \  )(   ) _) / \/ \\___ \
 *   (____)(_/\_) \__/  (__)  (__)  \___)  (____/(__/  (____/ (__) (____)\_)(_/(____/
 *
 * Copyright (c) 2017 EXOTIC SYSTEMS. All Rights Reserved.
 *
 * Licensees are granted free, non-transferable use of the information. NO WARRANTY
 * of ANY KIND is provided. This heading must NOT be removed from the file.
 *
 */

#ifndef INC_SUPER_TRACKER_AT_H_
#define INC_SUPER_TRACKER_AT_H_

/****************************************************************************************
 * Include Files
 ****************************************************************************************/
#include <stdint.h>

/****************************************************************************************
 * Defines
 ****************************************************************************************/
#define AT_MAX_CMD_LEN		(256u)
#define AT_MAX_RESP_LEN		(512u)
#define AT_NO_RETRY			(0u)
#define AT_CARRIAGE_RETURN	('\r')
#define AT_CTRL_Z				(26u)
#define AT_NO_END_CHAR		(0xFFu)

/****************************************************************************************
 * Type definitions
 ****************************************************************************************/
typedef enum _AT_RETURN_VAL_ {
   AT_RET_OK,
   AT_RET_ERROR,
   AT_RET_TIMEOUT,
   AT_RET_END,
   AT_RET_SUCCESS,
   AT_RET_FAILED
} eATCode_t;

typedef enum _AT_STATUS_ {
	AT_OK,
	AT_NOK
} eAtStatus_t;

typedef eAtStatus_t (*fp_eUART_Transmit_t) (char * p_pchMsg, uint32_t p_u32Len);
typedef uint32_t (*fp_u32Timer_Get_ms_t)(void);
typedef void (*fp_vSetRxState_t)(uint8_t p_u8Active);
typedef void (*fp_vCriticalAccess_t)(uint8_t p_u8Active);

typedef struct _AT_CONTEXT_ {
	fp_eUART_Transmit_t 	fp_eTransmit;
	fp_u32Timer_Get_ms_t	fp_u32Get_ms;
	fp_vSetRxState_t	fp_vSetRxState;
	fp_vCriticalAccess_t	fp_vCriticalAccess;
} sATContext_t ;

/****************************************************************************************
 * Public function declarations
 ****************************************************************************************/
eATCode_t eAT_ContextSet(sATContext_t p_sContext);
void vAT_Init(void);
void vAT_Deinit(void);

eATCode_t eAT_Send(char * p_pchMsg, char p_chEndCharacter);
eATCode_t eAT_RawSend(uint8_t * p_pu8Data, uint32_t p_u32DataLen);


eATCode_t eAT_WaitResponse(const char * p_pchExpectedResponse, char * p_pchResponseBuffer, uint32_t p_u32Timeout);
void vAT_UpdateFrame(const uint8_t p_u8Data);

#endif /* INC_SUPER_TRACKER_AT_H_ */


