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
#ifndef BG96_H_
#define BG96_H_

/****************************************************************************************
 * Include Files
 ****************************************************************************************/
#include "sensors.h"

/****************************************************************************************
 * Defines
 ****************************************************************************************/
#define  GSM_RXBUF_MAXSIZE           1600
#define  GSM_GENER_CMD_LEN           (128)
#define  MAX_CMD_LEN                 (256u)

#define RDY_TIMEOUT       (10000u) /* ms */
#define CMD_TIMEOUT       (3000u) /* ms */
#define PWDN_TIMEOUT      (65000u) /* ms */
#define APN_TIMEOUT       (150000u) /* ms */
#define DEACT_TIMEOUT     (40000u) /* ms */

#define  GSM_OPENSOCKET_OK_STR          "CONNECT OK\r\n"
#define  GSM_OPENSOCKET_FAIL_STR        "CONNECT FAIL\r\n"
#define  GSM_READY_RF                    "RDY\r\n"
#define  GSM_CMD_CRLF                   "\r\n"
#define  GSM_CMD_RSP_OK_RF              "OK\r\n"
#define  GSM_CMD_RSP_OK                 "OK"
#define  GSM_CHECKSIM_RSP_OK            "+CPIN: READY"

/****************************************************************************************
 * Type definitions
 ****************************************************************************************/
 typedef enum _BG96_ERR_CODE_{
  BG96_SUCCESS,
  BG96_ERROR_PARAM,
  BG96_ERROR_FAILED,
  BG96_ERROR_TRANSMIT,
  BG96_ERROR_TIMEOUT,
  BG96_ERROR_REFUSED,

  BG96_ERROR_MAXID
} eBG96ErrorCode_t;

typedef enum _GNSS_CODES_ {
   GNSS_C_SUCCESS,
  GNSS_ERROR_PARAM,
  GNSS_ERROR_FAILED,
  GNSS_ERROR_NO_POSITION,
  GNSS_ERROR_TIMEOUT
} eGnssCodes_t;

typedef enum _BG96_STATUS_ {
  BG96_STATUS_INACTIVE   = 0u,
  BG96_STATUS_ACTIVE    = 1u,
} eBG96Status_t;

/****************************************************************************************
 * Public function declarations
 ****************************************************************************************/
 eBG96ErrorCode_t eBG96_TurnOn(void);
eBG96ErrorCode_t eBG96_TurnOff(void);

void bg96_init();         //bg96 power up
eBG96ErrorCode_t eBG96_SendCommand(char *at, const char * p_pchExpectedRsp, uint32_t p_u32Timeout);
void bg96_at(char *at);   //this function is suitable for most AT commands of bg96. e.g. bg96_at("ATI")
eGnssCodes_t eGNSS_GetPosition(sPosition_t * p_psPosition) ;
void connect(uint8_t p_u8Flag);
eBG96ErrorCode_t  eBG96_SetApnContext(char * p_pchApn, char * p_pchUser, char * p_pchPassword);
eBG96ErrorCode_t  eBG96_ActiveContext(void);
eBG96ErrorCode_t  eBG96_WaitResponse(char *rsp_value, uint32_t timeout_ms, const char * p_pchExpectedRsp);
void vBG96_GNSS_TurnOn(void);
void vBG96_GNSS_TurnOff(void);
eBG96ErrorCode_t eBG96_SetRATSearchSeq(char * p_pchSearchSeq);

#endif /* BG96_H_ */
