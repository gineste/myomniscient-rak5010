/*
      ____  _  _   __   ____   __    ___    ____  _  _  ____  ____  ____  _  _  ____
     (  __)( \/ ) /  \ (_  _) (  )  / __)  / ___)( \/ )/ ___)(_  _)(  __)( \/ )/ ___)
      ) _)  )  ( (  O )  )(    )(  ( (__   \___ \ )  / \___ \  )(   ) _) / \/ \\___ \
     (____)(_/\_) \__/  (__)  (__)  \___)  (____/(__/  (____/ (__) (____)\_)(_/(____/

   Copyright (c) 2021 EXOTIC SYSTEMS. All Rights Reserved.

   Licensees are granted free, non-transferable use of the information. NO WARRANTY
   of ANY KIND is provided. This heading must NOT be removed from the file.

   Date:          12/07/2021
   Author:        Martin C.
   Description:   BG96 drivers

*/

/****************************************************************************************
   Include Files
 ****************************************************************************************/
#include <Arduino.h>
#include <SoftwareSerial.h>

#include "timeout.h"
#include "sensors.h"
#include "config.h"
#include "str_utils.h"

#include "bg96.h"

/****************************************************************************************
   Defines
 ****************************************************************************************/
#define ON_TIMEOUT_MS     (5500u) /* ms */      // Doc BG96: wait at least 4.8s for waiting status pin outputting level
#define OFF_TIMEOUT_MS    (3000u) /* ms */
#define OFF_TIME          (1000u)

#define PWRKEY_PULSE_ON_MS  (600u) /* ms */
#define PWRKEY_PULSE_OFF_MS (700u) /* ms */

#define LTE_CMD_LEN        (256u)

/****************************************************************************************
   Private type declarations
 ****************************************************************************************/

/****************************************************************************************
   Private function declarations
 ****************************************************************************************/
 static eBG96Status_t eBG96_ReadStatusPin(void);
 static eBG96Status_t eBG96_WaitStatus(uint8_t p_u8ExpectedState, uint32_t p_u32Timeout);
 static eBG96Status_t eBG96_PulsePwrKey(uint32_t p_u32Duration);

/****************************************************************************************
   Variable declarations
 ****************************************************************************************/
String bg96_rsp = "";
char GSM_RSP[1600] = {0};

static uint16_t rxReadIndex  = 0;
static uint16_t rxWriteIndex = 0;
static uint16_t rxCount      = 0;
static uint8_t Gsm_RxBuf[GSM_RXBUF_MAXSIZE];

/****************************************************************************************
   Public functions
 ****************************************************************************************/
 
void vBG96_Init(void)
{
  pinMode(bg96_RESET, OUTPUT);
  pinMode(bg96_PWRKEY, OUTPUT);
  pinMode(bg96_GPS_EN, OUTPUT);
  pinMode(bg96_W_DISABLE, OUTPUT);
  pinMode(bg96_STATUS, INPUT);

  digitalWrite(bg96_RESET, 0);
  digitalWrite(bg96_W_DISABLE, 1);
  digitalWrite(bg96_GPS_EN, 0);
}

eBG96ErrorCode_t eBG96_SendCommand(char *at, const char * p_pchExpectedRsp, uint32_t p_u32Timeout)
{
  eBG96ErrorCode_t l_eCode = BG96_SUCCESS;
  int len = strlen(at);
  
  if ((at != NULL) && (len > 0))
  {
    Serial1.write(at);
    Serial1.write('\r');
    delay(10);
    memset(GSM_RSP, 0, 1600);
    l_eCode = eBG96_WaitResponse(GSM_RSP, p_u32Timeout, p_pchExpectedRsp);
    #ifdef DEBUG
      Serial.printf("%s\r\n", GSM_RSP);
    #endif
  } else {
  #ifdef DEBUG
    Serial.printf("AT cmd too long\r\n");
  #endif
    l_eCode = BG96_ERROR_PARAM;
  }

  return l_eCode;
}

eBG96ErrorCode_t eBG96_SendCommandExpected(char *at,  const char * p_pchSearchStr, const char * p_pchExpectedRsp, uint32_t p_u32Timeout)
{
  eBG96ErrorCode_t l_eCode = BG96_SUCCESS;
  int len = strlen(at);
  
  if ((at != NULL) && (len > 0))
  {
    Serial1.write(at);
    Serial1.write('\r');
    memset(GSM_RSP, 0, 1600);
    l_eCode = eBG96_WaitResponse(GSM_RSP, p_u32Timeout, p_pchExpectedRsp);
    if(BG96_SUCCESS == l_eCode)
    {
    #ifdef DEBUG
      Serial.printf("Update response\r\n");
    #endif
      l_eCode = eBG96_UpdateResponseStr(p_pchSearchStr);
    }
    #ifdef DEBUG
      Serial.printf("%s\r\n", GSM_RSP);
    #endif
  } else {
  #ifdef DEBUG
    Serial.printf("AT cmd too long\r\n");
  #endif
    l_eCode = BG96_ERROR_PARAM;
  }

  return l_eCode;
}

eBG96ErrorCode_t eBG96_UpdateResponseStr(const char * p_pchSearchStr)
{
  eBG96ErrorCode_t l_eCode = BG96_SUCCESS;
  char * l_pchRespPtr = NULL;

  if(p_pchSearchStr != NULL)
  {
    /* Search for interesting data */
    l_pchRespPtr = strstr(GSM_RSP, p_pchSearchStr);
    if(l_pchRespPtr != NULL)
    {
      /* Copy response */
      strcpy(GSM_RSP, l_pchRespPtr);
      l_eCode = BG96_SUCCESS;
    }else{
      /* Response not found, error ! */
      l_eCode = BG96_ERROR_FAILED;
    }
  }else{
    /* No Change */
    l_eCode = BG96_SUCCESS;
  }

  return l_eCode;
}
 
 /**@brief      Turn on BG96.
 * @retval BG96_SUCCESS
 * @retval BG96_ERROR_TIMEOUT
 * @retval BG96_ERROR_FAILED
 */
eBG96ErrorCode_t eBG96_TurnOn(void)
{
 eBG96ErrorCode_t l_eisReady = BG96_ERROR_FAILED;
  eBG96Status_t l_eBG96_Status = BG96_STATUS_INACTIVE;
  uint32_t l_u32TsIn = u32Time_getMs();

#ifdef DEBUG
  Serial.printf("[%u] GETSTATUS (%u ms)\r\n", (unsigned int)u32Time_getMs(), (unsigned int)(u32Time_getMs() - l_u32TsIn));
#endif
    
  /* Get current status */
  l_eBG96_Status = eBG96_ReadStatusPin();

  /* Check if modem is OFF */
  if(l_eBG96_Status == BG96_STATUS_INACTIVE)
  {    
    /* Switch on Vcc */
    //vBG96_EnablePwr();

    /* Make a pulse PWRKEY at 500ms to start the BG96 */
    eBG96_PulsePwrKey(PWRKEY_PULSE_ON_MS);

    /* Wait for STATUS comes High(active) */
    l_eBG96_Status = eBG96_WaitStatus(BG96_STATUS_ACTIVE, ON_TIMEOUT_MS);
    if(BG96_STATUS_ACTIVE == l_eBG96_Status)
    {
      /* Reinit AT layer (restart rx on uart) */
      //vAT_Init();

      /* Wait for UART and internal BG96 state machine */
      //l_eisReady = eBG96_WaitResponse(RDY_STRING, RDY_TIMEOUT);
      memset(GSM_RSP, 0, 1600);
      l_eisReady = eBG96_WaitResponse(GSM_RSP, RDY_TIMEOUT, GSM_READY_RF);

      //g_ePowerState = BG96_PWR_BGON;
    }else{
      /* Not in good state */
      l_eisReady = BG96_ERROR_FAILED;
    #ifdef DEBUG
      Serial.printf("BG96 turn on PWR key failed\r\n");
    #endif
    }
  }else{
    /* Reinit AT layer (restart rx on uart) */
    //vAT_Init();

    /* Already started */
    l_eisReady = BG96_SUCCESS;
  }

#ifdef DEBUG
  Serial.printf("[%u] STATUS=%d, READY=%d (%u ms)\r\n",
           (unsigned int)u32Time_getMs(),
           l_eBG96_Status,
           l_eisReady,
           (unsigned int)(u32Time_getMs() - l_u32TsIn));
#endif

  return l_eisReady;
}

/**@brief      Turn off BG96.
 * @retval BG96_SUCCESS
 * @retval BG96_ERROR_FAILED
 */
eBG96ErrorCode_t eBG96_TurnOff(void)
{
  eBG96ErrorCode_t l_eCode = BG96_ERROR_FAILED;
  uint32_t l_u32TsIn = u32Time_getMs();

  eBG96Status_t l_eBG96_Status = eBG96_ReadStatusPin(); /* Get current status */

  #ifdef DEBUG
  Serial.printf("[%u] GETSTATUS (%u ms)\r\n", (unsigned int)u32Time_getMs(), (unsigned int)(u32Time_getMs() - l_u32TsIn));
  #endif
    
  if (l_eBG96_Status == BG96_STATUS_ACTIVE)
  {
    l_u32TsIn = u32Time_getMs();

    /* Make a pulse PWRKEY to turn off the BG96 */
    eBG96_PulsePwrKey(PWRKEY_PULSE_OFF_MS);

    /* Wait for STATUS comes Low(inactive) */
    if(BG96_STATUS_INACTIVE == eBG96_WaitStatus(BG96_STATUS_INACTIVE, OFF_TIMEOUT_MS))
    {
      l_eCode = BG96_SUCCESS;
      //g_ePowerState = BG96_PWR_BGOFF;
    }else{
      l_eCode = BG96_ERROR_FAILED;
    #ifdef DEBUG
      Serial.printf("BG96 turn off PWR key failed\r\n");
    #endif

      /* Switch off Vcc for 1s */
      //vBG96_DisablePwr();
    }
  #ifdef DEBUG
    Serial.printf("[%u] OFF (%u ms)\r\n", (unsigned int)u32Time_getMs(), (unsigned int)(u32Time_getMs() - l_u32TsIn));
  #endif
  }else{
    /* Already off */
    l_eCode = BG96_SUCCESS;
  }

  return l_eCode;
}

int Gsm_RxByte(void)
{
  int c = -1;
  
  if (Serial1.available() > 0)
  {
    c = char(Serial1.read());
  }

  rxReadIndex++;
  if (rxReadIndex == GSM_RXBUF_MAXSIZE)
  {
    rxReadIndex = 0;
  }
  
  return c;
}

eBG96ErrorCode_t eBG96_WaitResponse(char *rsp_value, uint32_t timeout_ms, const char * p_pchExpectedRsp)
{
  eBG96ErrorCode_t l_eErrCode = BG96_ERROR_PARAM;  
  int wait_len = 0;
  char len[10] = {0};
  uint16_t time_count = timeout_ms;
  uint32_t i = 0;
  int       c;
  char *cmp_p = NULL;
  uint8_t l_u8EOT = 0u;

  wait_len = strlen(p_pchExpectedRsp);
 
  memset(GSM_RSP, 0, 1600);
  do
  {
  #if (WDG_ENABLE == 1u) 
    // Reload the WDTs RR[0] reload register
    NRF_WDT->RR[0] = WDT_RR_RR_Reload; 
  #endif
    
    int c;
    c = Gsm_RxByte();
    if (c < 0)
    {
      time_count--;
      delay(1);
      continue;
    }

    GSM_RSP[i++] = (char)c;

    if (i >= 0 && rsp_value != NULL)
    {
      cmp_p = strstr(GSM_RSP, p_pchExpectedRsp);
      if (cmp_p != NULL)
      {
        if (i > wait_len && rsp_value != NULL)
        {
          memcpy(rsp_value, GSM_RSP, i);
        }
        l_eErrCode = BG96_SUCCESS;
        l_u8EOT = 1u;
      }else{
        l_eErrCode = BG96_ERROR_FAILED;
        if((strstr(GSM_RSP, "ERROR\r\n") != NULL) ||
            (strstr(GSM_RSP, "+CME ERROR:") != NULL))
          {
            /* Error */
            l_u8EOT = 1u;
          }
        }
    }else{
        l_eErrCode = BG96_ERROR_PARAM;
    }
  } while ((time_count > 0) && (l_u8EOT != 1u));

  return l_eErrCode;
}

/**@brief Get current context status
 * @param p_peIpState         ACTIVATED/DEACTIVATED
 * @retval BG96_SUCCESS
 * @retval BG96_ERROR_FAILED
 * @retval BG96_ERROR_PARAM
 */
eBG96ErrorCode_t eBG96_GetContextState(eNetCtxStat_t * p_peIpState, char * p_pchIp)
{
  eBG96ErrorCode_t l_eCode = BG96_SUCCESS;

  uint8_t l_u8CtxId = 0u;
  uint8_t l_u8CtxState = 0u;
  uint8_t l_u8CtxType = 0u;

  if((p_peIpState != NULL) && (p_pchIp != NULL))
  {
    l_eCode = eBG96_SendCommand("AT+QIACT?", GSM_CMD_RSP_OK_RF, APN_TIMEOUT);

    if(l_eCode == BG96_SUCCESS)
    {
      if(sscanf(GSM_RSP, "+QIACT: %c, %c, %c, %s", &l_u8CtxId, &l_u8CtxState, &l_u8CtxType, p_pchIp) > 0)
      {
        *p_peIpState = (eNetCtxStat_t) l_u8CtxState;
      
        #ifdef DEBUG
          Serial.printf("%s\r\n", GSM_RSP);
        #endif
      }else{
      }
    }
  }else{
    l_eCode = BG96_ERROR_PARAM;
  }

  return l_eCode;
}

/**@brief Set the searching sequence of RATs
 * @param p_pchSearchSeq      numbers corresponding to searching sequence of RATs
 * @retval BG96_SUCCESS
 * @retval BG96_ERROR_FAILED
 * @retval BG96_ERROR_PARAM
 */
eBG96ErrorCode_t eBG96_SetRATSearchSeq(char * p_pchSearchSeq)
{
  eBG96ErrorCode_t l_eCode = BG96_ERROR_PARAM;
  char l_achCmd[LTE_CMD_LEN] = {0};

  if (p_pchSearchSeq != NULL)
  {
    snprintf(l_achCmd, LTE_CMD_LEN, "AT+QCFG=\"nwscanseq\",%s,1", p_pchSearchSeq);
    l_eCode = eBG96_SendCommand(l_achCmd, GSM_CMD_RSP_OK_RF, CMD_TIMEOUT);
  }

  return l_eCode;
}

/**@brief Get the received signal strengh (rssi) in dBm
 * Rssi = 99 if not known or not detectable
 * @param p_ps16Rssi : BG96 RSSI response
 * @retval BG96_SUCCESS
 * @retval BG96_ERROR_FAILED
 * @retval BG96_ERROR_PARAM
 */
eBG96ErrorCode_t eBG96_GetRSSI(int16_t * p_ps16Rssi)
{
  eBG96ErrorCode_t l_eCode = BG96_ERROR_PARAM;
  unsigned int l_uiDumb_number = 0;
  unsigned int l_uiRssi = 0;

  if (p_ps16Rssi != NULL)
  {
    l_eCode = eBG96_SendCommandExpected("AT+CSQ", "+CSQ:", GSM_CMD_RSP_OK_RF, CMD_TIMEOUT);
    if(BG96_SUCCESS == l_eCode)
    {
      if(0 < sscanf(GSM_RSP, "+CSQ: %u,%u\r\n", &l_uiRssi,&l_uiDumb_number))
      {
        l_eCode = BG96_SUCCESS;
        if(l_uiRssi != 99)
        {
          *p_ps16Rssi = (int16_t) ((2 * l_uiRssi) - 113);
        }
      }else{
        l_eCode = BG96_ERROR_FAILED;
      }
    }
  }

  return l_eCode;
}

/**@brief Get the information about the current network used
 * @param p_pchAccessTech
 * @param p_pchBand
 * @param p_pchOperatorId
 * @param p_pchChannelId
 * @retval BG96_SUCCESS
 * @retval BG96_ERROR_FAILED
 * @retval BG96_ERROR_PARAM
 */
eBG96ErrorCode_t eBG96_GetNetworkInfo(char * p_pchAccessTech, char * p_pchBand, char * p_pchOperatorId, char * p_pchChannelId)
{
  eBG96ErrorCode_t l_eCode = BG96_ERROR_PARAM;
  char *l_apchArgv[5] = {0};
  uint16_t l_u16Argc = 0u;
  char *l_pchParams;

  if ((p_pchAccessTech != NULL) && (p_pchBand != NULL) && (p_pchOperatorId != NULL) && (p_pchChannelId != NULL))
  {
    l_eCode = eBG96_SendCommandExpected("AT+QNWINFO", "+QNWINFO:", GSM_CMD_RSP_OK_RF, CMD_TIMEOUT);
    if(BG96_SUCCESS == l_eCode)
    {
      l_pchParams = strstr(GSM_RSP, ": ");
      if(l_pchParams != NULL)
      {
        l_pchParams+=2u;
        l_u16Argc = u16SU_GetParamsFromString(l_pchParams, ',', l_apchArgv, 5u);
        
        if(l_u16Argc == 5u)
        {
          strncpy(p_pchAccessTech,  &(l_apchArgv[0][1]), strlen(l_apchArgv[0]) - 2u); /* remove " " */
          strncpy(p_pchOperatorId,  &(l_apchArgv[1][1]), strlen(l_apchArgv[1]) - 2u); /* remove " " */
          strncpy(p_pchBand,      &(l_apchArgv[2][1]), strlen(l_apchArgv[2]) - 2u); /* remove " " */
          strncpy(p_pchChannelId,   l_apchArgv[3], strlen(l_apchArgv[3]));
          l_eCode = BG96_SUCCESS;
        }else{
          l_eCode = BG96_ERROR_FAILED;
        }
      }else{
        l_eCode = BG96_ERROR_FAILED;
      }
    }
  }else{
    l_eCode = BG96_ERROR_PARAM;
  }

  return l_eCode;
}

/**@brief Request information about current operator
 * @param p_eNetworkMode      Network mode used
 * @param p_pchNetworkName    Operator name in ASCII
 * @param p_peNetworkTech      Current network access technology
 * @retval BG96_SUCCESS
 * @retval BG96_ERROR_FAILED
 * @retval BG96_ERROR_PARAM
 */
eBG96ErrorCode_t eBG96_GetNetwork(eNetworkMode_t *p_eNetworkMode, char * p_pchNetworkName, eNetworkTech_t *p_peNetworkTech)
{
  eBG96ErrorCode_t l_eCode = BG96_SUCCESS;
  uint8_t l_u8NetFormat = 0u; /* Field not used */
  char *l_pchOperatorNameStart = NULL;
  char *l_pchOperatorNameEnd = NULL;

  if((p_eNetworkMode != NULL) && (p_pchNetworkName != NULL) && (p_peNetworkTech != NULL))
  {
    l_eCode = eBG96_SendCommandExpected("AT+COPS?", "+COPS:", GSM_CMD_RSP_OK_RF, CMD_TIMEOUT);

    if(l_eCode == BG96_SUCCESS)
    {
      if(sscanf(GSM_RSP, "+COPS: %c,%c,\"%s\",%c", (uint8_t *)p_eNetworkMode, &l_u8NetFormat, p_pchNetworkName, (uint8_t *)p_peNetworkTech) > 0)
      {
        l_pchOperatorNameStart = strstr(GSM_RSP, "\"") + 1; /* skip " */
        l_pchOperatorNameEnd = strstr(l_pchOperatorNameStart, "\"");

        if ((l_pchOperatorNameEnd != NULL) && (l_pchOperatorNameStart != NULL))
        {
          memset(p_pchNetworkName, 0, MAX_OPERATOR_NAME_LEN);
          strncpy(p_pchNetworkName, l_pchOperatorNameStart, l_pchOperatorNameEnd - l_pchOperatorNameStart); 
        }

        *p_peNetworkTech = (eNetworkTech_t) *(l_pchOperatorNameEnd+2);
      }else{
        p_pchNetworkName = "UNKNOWN";
      }
    }
  }else{
    l_eCode = BG96_ERROR_PARAM;
  }

  return l_eCode;
}

/**@brief Get battery information
 * @param p_pu8ChargeStatus : 0 not charging ; 1 charging ; 2 charge finished
 * @param p_pu8ChargeLevel  : batt level in %
 * @param p_pu16BattMv      : batt voltage in mv
 * @retval BG96_SUCCESS
 * @retval BG96_ERROR_FAILED
 * @retval BG96_ERROR_PARAM
 */
eBG96ErrorCode_t eBG96_GetBattInfos(uint8_t * p_pu8ChargeStatus, uint8_t * p_pu8ChargeLevel, uint16_t * p_pu16BattMv)
{
  eBG96ErrorCode_t l_eCode = BG96_ERROR_PARAM;

  if ((p_pu8ChargeStatus != NULL) && (p_pu8ChargeLevel != NULL) && (p_pu16BattMv != NULL))
  {
    l_eCode = eBG96_SendCommandExpected("AT+CBC", "+CBC:", GSM_CMD_RSP_OK_RF, CMD_TIMEOUT);
    if(BG96_SUCCESS == l_eCode)
    {
      if(0 < sscanf(GSM_RSP, "+CBC: %hu,%hu,%hu\r\n", p_pu8ChargeStatus, p_pu8ChargeLevel, p_pu16BattMv))
      {
        l_eCode = BG96_SUCCESS;
      }else{
        l_eCode = BG96_ERROR_FAILED;
      }
    }
  }else{
    l_eCode = BG96_ERROR_PARAM;
  }

  return l_eCode;
}

/**@brief Queries the real time clock (RTC) of the module
 * @param p_pchTimeStr : BG96 Time response
 * @retval BG96_SUCCESS
 * @retval BG96_ERROR_FAILED
 * @retval BG96_ERROR_PARAM
 */
eBG96ErrorCode_t eBG96_GetTime(char * p_pchTimeStr)
{
  eBG96ErrorCode_t l_eCode = BG96_ERROR_PARAM;
  if (p_pchTimeStr != NULL)
  {
    l_eCode = eBG96_SendCommandExpected("AT+CCLK?", "+CCLK:", GSM_CMD_RSP_OK_RF, CMD_TIMEOUT);
    if(BG96_SUCCESS == l_eCode)
    {
      if(0 < sscanf(GSM_RSP, "+CCLK: %s", p_pchTimeStr))
      {
        l_eCode = BG96_SUCCESS;
      }else{
        l_eCode = BG96_ERROR_FAILED;
      }
    }
  }else{
    l_eCode = BG96_ERROR_PARAM;
  }

  return l_eCode;
}

/****************************************************************************************
   Private functions
 ****************************************************************************************/
 static eBG96Status_t eBG96_ReadStatusPin(void)
{
 return (HIGH == digitalRead(bg96_STATUS)) ? BG96_STATUS_ACTIVE : BG96_STATUS_INACTIVE;
}

static eBG96Status_t eBG96_WaitStatus(uint8_t p_u8ExpectedState, uint32_t p_u32Timeout)
{
  uint32_t l_u32Alarm = u32Time_getMs() + p_u32Timeout;
  eBG96Status_t l_eBG96_Status = BG96_STATUS_INACTIVE;

  /* Read status pin until status pin high or timeout */
  do
  {
  #if (WDG_ENABLE == 1u) 
    // Reload the WDTs RR[0] reload register
    NRF_WDT->RR[0] = WDT_RR_RR_Reload; 
  #endif

    l_eBG96_Status = eBG96_ReadStatusPin();
  }
  while ( (l_eBG96_Status != p_u8ExpectedState) && (l_u32Alarm >= u32Time_getMs()) );

  return l_eBG96_Status;
}

static eBG96Status_t eBG96_PulsePwrKey(uint32_t p_u32Duration)
{
  digitalWrite(bg96_PWRKEY, HIGH);
  vTime_WaitMs(p_u32Duration);
  digitalWrite(bg96_PWRKEY, LOW);

  return eBG96_ReadStatusPin();
}

/****************************************************************************************
   End Of File
 ****************************************************************************************/
