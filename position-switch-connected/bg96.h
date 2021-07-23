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

#define  MAX_OPERATOR_NAME_LEN       (64u)
#define  GSM_RXBUF_MAXSIZE           (1600)
#define  GSM_GENER_CMD_LEN           (128)
#define  MAX_CMD_LEN                 (1024u)

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

typedef enum _NETWORK_TECH_ {
   NET_TECH_GSM                 = '0',
   NET_TECH_LTE_M1              = '8',
   NET_TECH_LTE_NB1             = '9'
} eNetworkTech_t;

typedef enum _NETWORK_STAT_ {
   NET_STAT_DETACHED             = '0',
   NET_STAT_ATTACHED             = '1',
} eNetworkStat_t;

typedef enum _NETWORK_MODE_ {
   NET_MODE_AUTOMATIC           = '0',
   NET_MODE_MANUAL_SELECTED     = '1',
   NET_MODE_DEREGISTER          = '2',
   NET_MODE_RESERVED1           = '3',
   NET_MODE_MANUAL_AUTOMATIC    = '4'
} eNetworkMode_t;

typedef enum _NETWORK_REG_ {
    NET_REG_STOPPED            = '0', /* Not registered. MT is not currently searching an operator to register to. */
    NET_REG_REGISTERED         = '1', /* Registered, home network. */
    NET_REG_SEARCHING          = '2', /* Not registered, but MT is currently trying to attach or searching an operator to */
    NET_REG_DENIED             = '3', /* Registration denied */
    NET_REG_UNKNOWN            = '4', /* Unknown */
    NET_REG_ROAMING            = '5', /* Registered, roaming */
} eNetworkRegisterState_t;

typedef enum _CELLULAR_BAND_{
  CELLULAR_BAND_GSM_800,
  CELLULAR_BAND_GSM_900,
  CELLULAR_BAND_GSM_1800,
  CELLULAR_BAND_GSM_1900,
  CELLULAR_BAND_LTE,
  CELLULAR_BAND_COUNT
}eCellularBand_t;

/****************************************************************************************
 * Public function declarations
 ****************************************************************************************/
 eBG96ErrorCode_t eBG96_TurnOn(void);
eBG96ErrorCode_t  eBG96_TurnOff(void);

void              bg96_init();         //bg96 power up
eBG96ErrorCode_t  eBG96_SendCommand(char *at, const char * p_pchExpectedRsp, uint32_t p_u32Timeout);
eBG96ErrorCode_t eBG96_SendCommandExpected(char *at,  const char * p_pchSearchStr, const char * p_pchExpectedRsp, uint32_t p_u32Timeout);
eBG96ErrorCode_t eBG96_UpdateResponseStr(const char * p_pchSearchStr);
void              bg96_at(char *at);   //this function is suitable for most AT commands of bg96. e.g. bg96_at("ATI")
eBG96ErrorCode_t  eBG96_WaitResponse(char *rsp_value, uint32_t timeout_ms, const char * p_pchExpectedRsp);
eBG96ErrorCode_t  eBG96_GetContextState(eNetCtxStat_t * p_peIpState, char * p_pchIp);


 eBG96ErrorCode_t   eBG96_SetApnContext(char * p_pchApn, char * p_pchUser, char * p_pchPassword);
eBG96ErrorCode_t    eBG96_ActiveContext(void);
eBG96ErrorCode_t    eBG96_SetRATSearchSeq(char * p_pchSearchSeq);
eBG96ErrorCode_t    eBG96_GetRSSI(int16_t * p_ps16Rssi);
eBG96ErrorCode_t    eBG96_GetNetworkInfo(char * p_pchAccessTech, char * p_pchBand, char * p_pchOperatorId, char * p_pchChannelId);
eBG96ErrorCode_t    eBG96_GetNetwork(eNetworkMode_t *p_eNetworkMode, char * p_pchNetworkName, eNetworkTech_t *p_peNetworkTech);
eBG96ErrorCode_t    eBG96_GetBattInfos(uint8_t * p_pu8ChargeStatus, uint8_t * p_pu8ChargeLevel, uint16_t * p_pu16BattMv);

#endif /* BG96_H_ */
