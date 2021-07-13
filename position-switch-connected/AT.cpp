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
 * Date:          17/02/2020 (dd MM YYYY)
 * Author:        Martin CORNU
 * Description:   AT Interface to Send command.
 *
 */

/****************************************************************************************
 * Include Files
 ****************************************************************************************/
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <Arduino.h>
#include <Wire.h>

/* self include */
#include "AT.h"

/****************************************************************************************
 * Defines
 ****************************************************************************************/
#define AT_TIMEOUT_MIN     (uint32_t)10000u
#define AT_TIMEOUT_MAX     (uint32_t)60000u

#define AT_EOF_SIZE        (uint8_t)2u

#define TIMEOUT_MSG			"Timeout response"

#define AT_SEND_MAX_RETRIES	(5u)

#define USE_AT_TRACE				(1u)

/****************************************************************************************
 * Private type declarations
 ****************************************************************************************/

/****************************************************************************************
 * Private function declarations
 ****************************************************************************************/

/****************************************************************************************
 * Variable declarations
 ****************************************************************************************/
volatile uint8_t g_u8MsgReceived = 0u;
volatile uint8_t g_u8Timeout = 0u;

static char g_achAtReceivedStr[AT_MAX_RESP_LEN] = { 0u };
volatile uint16_t g_u16RxIdx = 0u;

static sATContext_t g_sATCtx;
static uint8_t g_u8IsContextSet = 0u;

/****************************************************************************************
 * Public functions
 ****************************************************************************************/
/**@brief Function to set context of AT .
 * @param[in]  p_sContext : All context of AT.
 * @return Error Code.
 */
eATCode_t eAT_ContextSet(sATContext_t p_sContext)
{
	eATCode_t l_eErrCode = AT_RET_ERROR;

	if((p_sContext.fp_eTransmit != NULL)
			&& (p_sContext.fp_u32Get_ms != NULL )
			&& (p_sContext.fp_vSetRxState != NULL )
			&& (p_sContext.fp_vCriticalAccess != NULL ))
	{
		g_sATCtx = p_sContext;
		g_u8IsContextSet = 1u;
		l_eErrCode = AT_RET_SUCCESS;
	}
	else
	{  // Could not communicate with sensor
		g_u8IsContextSet = 0u;
		l_eErrCode = AT_RET_SUCCESS;
	}

	return l_eErrCode;
}

/**@brief Initialize AT module.
 * @retval None
 */
void vAT_Init(void)
{
	g_u8Timeout = 0u;
	g_u8MsgReceived = 0u;
	g_u16RxIdx = 0u;

	if(g_u8IsContextSet == 1u)
	{
		g_sATCtx.fp_vSetRxState(1);
	}
}

/**@brief Deinitialize AT module.
 * @retval None
 */
void vAT_Deinit(void)
{
	g_u8Timeout = 0u;
	g_u8MsgReceived = 0u;
	g_u16RxIdx = 0u;

	if(g_u8IsContextSet == 1u)
	{
		g_sATCtx.fp_vSetRxState(0);
	}
}

/**@brief Direct send AT command.
 * @param [in]  p_pu8Msg         : AT message to send
 * @param [in]  p_chEndCharacter : End character to send after data (can be AT_NO_END_CHAR)
 * @retval AT_RET_SUCCESS
 * @retval AT_RET_FAILED
 * @retval AT_RET_ERROR
 */
eATCode_t eAT_Send(char * p_pchMsg, char p_chEndCharacter)
{
	eATCode_t l_eRetCode = AT_RET_SUCCESS;
	uint32_t l_u32Size = strlen(p_pchMsg);

	if(g_u8IsContextSet == 1u)
	{
		g_sATCtx.fp_vCriticalAccess(1);
		g_u16RxIdx = 0u;
		g_u8MsgReceived = 0u;
		g_sATCtx.fp_vCriticalAccess(0);

		/* Send on UART */
		if(AT_OK == g_sATCtx.fp_eTransmit(p_pchMsg, l_u32Size))
		{

#if (USE_AT_TRACE == (1u))
			Serial.printf("Cmd : %s\r\n", p_pchMsg);
#endif
			/* Send End character */
			if(p_chEndCharacter != AT_NO_END_CHAR)
			{
				g_sATCtx.fp_eTransmit(&p_chEndCharacter, 1u);
			}
			l_eRetCode = AT_RET_SUCCESS;
		}else{
			l_eRetCode = AT_RET_FAILED;
		}
	}else{
		l_eRetCode = AT_RET_ERROR;
	}

	return l_eRetCode;
}

/**@brief Direct send RAW Data.
 * @param [in]  p_pu8Data         : Raw data to send
 * @param [in]  p_u32DataLen      : Length of data to send
 * @retval AT_RET_SUCCESS
 * @retval AT_RET_FAILED
 * @retval AT_RET_ERROR
 */
eATCode_t eAT_RawSend(uint8_t * p_pu8Data, uint32_t p_u32DataLen)
{
	eATCode_t l_eRetCode = AT_RET_SUCCESS;

	if(g_u8IsContextSet == 1u)
	{
		/* Send on UART */
		if(AT_OK == g_sATCtx.fp_eTransmit((char*)p_pu8Data, p_u32DataLen))
		{
			l_eRetCode = AT_RET_SUCCESS;
		}else{
			l_eRetCode = AT_RET_FAILED;
		}
	}else{
		l_eRetCode = AT_RET_ERROR;
	}

	return l_eRetCode;
}

/**@brief Wait response on UART
 * @param [in]  p_pchExpectedResponse        : String to wait
 * @param [out] p_pchResponseBuffer     		: Output response buffer
 * @param [in]  p_u32Timeout     				: Timeout
 * @retval AT_RET_SUCCESS
 * @retval AT_RET_FAILED
 * @retval AT_RET_ERROR
 */
eATCode_t eAT_WaitResponse(const char * p_pchExpectedResponse, char * p_pchResponseBuffer, uint32_t p_u32Timeout)
{
	eATCode_t l_eRetCode = AT_RET_SUCCESS;

	char l_achUartRxBytes[AT_MAX_RESP_LEN] = { 0u };
	uint8_t l_u8EOT = 0u;
	uint32_t l_u32RspTimeout = 0u;
	uint8_t l_u8MsgAvailable = g_u8MsgReceived;

	/* Set timeout alarm */
	l_u32RspTimeout = g_sATCtx.fp_u32Get_ms() + p_u32Timeout;


	do{
	/*#if (ITSDK_WITH_WDG == __WDG_IWDG)
		wdg_refresh();
	#endif*/
		if(l_u32RspTimeout < g_sATCtx.fp_u32Get_ms())
		{
			/* Timeout */
			l_eRetCode = AT_RET_TIMEOUT;
			l_u8EOT = 1u;
		}else{
			l_u8MsgAvailable = g_u8MsgReceived;
			if(l_u8MsgAvailable > 0)
			{
				g_sATCtx.fp_vCriticalAccess(1);
				memcpy(l_achUartRxBytes, g_achAtReceivedStr, g_u16RxIdx);
				g_u8MsgReceived = 0;
				g_sATCtx.fp_vCriticalAccess(0);

				if(strstr(l_achUartRxBytes, p_pchExpectedResponse) != NULL)
				{
					if(NULL != p_pchResponseBuffer)
					{
						memcpy(p_pchResponseBuffer, l_achUartRxBytes, g_u16RxIdx);
					}

					l_u8EOT = 1u;
				}else{
					if((strstr(l_achUartRxBytes, "ERROR\r\n") != NULL) ||
							(strstr(l_achUartRxBytes, "+CME ERROR:") != NULL))
					{
						/* Error */
						l_eRetCode = AT_RET_ERROR;
						l_u8EOT = 1u;
					}
				}

			}
		}
	}while(l_u8EOT == 0u);

#if (USE_AT_TRACE == (1u))
	Serial.printf("Rsp : %s\r\n", l_achUartRxBytes);
#endif

	return l_eRetCode;
}

/**@brief Callback function handler for AT command.
 * @param[in] p_u8Data : Input Char.
 * @retval None
 */
void vAT_UpdateFrame(const uint8_t p_u8Data)
{
	/* save in buffer */
	if(p_u8Data == '\0')
	{
		return; /* Avoid \0 */
	}else{
		g_achAtReceivedStr[g_u16RxIdx] = p_u8Data;
		++g_u16RxIdx;
	}

	/* Manage Buffer overflow */
	if(g_u16RxIdx > AT_MAX_RESP_LEN)
	{
		g_u16RxIdx = 0u;
	}else{
		g_achAtReceivedStr[g_u16RxIdx] = '\0';
	}

	if((g_u16RxIdx > 2u) &&
			(
					((g_achAtReceivedStr[g_u16RxIdx - 2] == '\r') && (g_achAtReceivedStr[g_u16RxIdx-1] == '\n'))
					||
					((g_achAtReceivedStr[g_u16RxIdx - 2] == '>') && (g_achAtReceivedStr[g_u16RxIdx-1] == ' '))
			)
	)
	{
		++g_u8MsgReceived;
	}
}

/****************************************************************************************
 * Private functions
 ****************************************************************************************/



/****************************************************************************************
 * End Of File
 ****************************************************************************************/
