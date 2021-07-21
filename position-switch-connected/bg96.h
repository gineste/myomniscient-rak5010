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
#include <stdint.h>

/****************************************************************************************
 * Defines
 ****************************************************************************************/
//Pin define
#define bg96_W_DISABLE  29
#define bg96_RESET      28
#define bg96_PWRKEY     2
#define bg96_GPS_EN     39
#define bg96_STATUS     31

#define  GSM_RXBUF_MAXSIZE           1600
#define  GSM_GENER_CMD_LEN           (128)
#define  MAX_CMD_LEN                 (256u)

#define RDY_TIMEOUT       (10000u) /* ms */
#define CMD_TIMEOUT       (3000u) /* ms */
#define PWDN_TIMEOUT      (65000u) /* ms */
#define APN_TIMEOUT       (150000u) /* ms */
#define CONN_TIMEOUT      (130000u) /* ms */
#define DEACT_TIMEOUT     (40000u) /* ms */

#define  GSM_CONNECT_STR                "CONNECT\r\n"
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

typedef enum _BG96_STATUS_ {
  BG96_STATUS_INACTIVE   = 0u,
  BG96_STATUS_ACTIVE    = 1u,
} eBG96Status_t;

typedef enum _NETWORK_IP_STAT_ {
   NET_CTX_DEACTIVATE        = '0',
   NET_CTX_ACTIVATE          = '1',
} eNetCtxStat_t;

/****************************************************************************************
 * Public function declarations
 ****************************************************************************************/
 eBG96ErrorCode_t eBG96_TurnOn(void);
eBG96ErrorCode_t  eBG96_TurnOff(void);

void              bg96_init();         //bg96 power up
eBG96ErrorCode_t  eBG96_SendCommand(char *at, const char * p_pchExpectedRsp, uint32_t p_u32Timeout);
void              bg96_at(char *at);   //this function is suitable for most AT commands of bg96. e.g. bg96_at("ATI")
eBG96ErrorCode_t  eBG96_WaitResponse(char *rsp_value, uint32_t timeout_ms, const char * p_pchExpectedRsp);
eBG96ErrorCode_t  eBG96_GetContextState(eNetCtxStat_t * p_peIpState, char * p_pchIp);

#endif /* BG96_H_ */
