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

#include "bg96.h"

/****************************************************************************************
   Defines
 ****************************************************************************************/
#define ON_TIMEOUT_MS    (5500u) /* ms */      // Doc BG96: wait at least 4.8s for waiting status pin outputting level
#define OFF_TIMEOUT_MS    (3000u) /* ms */
#define OFF_TIME        (1000u)

#define PWRKEY_PULSE_ON_MS  (600u) /* ms */
#define PWRKEY_PULSE_OFF_MS (700u) /* ms */

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
 //bg96 power up
void bg96_init()
{
  pinMode(bg96_RESET, OUTPUT);
  pinMode(bg96_PWRKEY, OUTPUT);
  pinMode(bg96_GPS_EN, OUTPUT);
  pinMode(bg96_W_DISABLE, OUTPUT);
  pinMode(bg96_STATUS, INPUT);

  digitalWrite(bg96_RESET, 0);
  digitalWrite(bg96_W_DISABLE, 1);
  //digitalWrite(bg96_GPS_EN, 1);
  digitalWrite(bg96_GPS_EN, 1);
}

//this function is suitable for most AT commands of bg96. e.g. bg96_at("ATI")
eBG96ErrorCode_t eBG96_SendCommand(char *at, const char * p_pchExpectedRsp, uint32_t p_u32Timeout)
{
  char tmp[MAX_CMD_LEN] = {0};
  eBG96ErrorCode_t l_eCode = BG96_SUCCESS;
  int len = strlen(at);

  Serial.printf("Send %s\r\n", at);
  
  if ((at != NULL) && (len <= MAX_CMD_LEN))
  {
    strncpy(tmp, at, len);
    tmp[len] = '\r';
    Serial1.write(tmp);
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

//this function is suitable for most AT commands of bg96. e.g. bg96_at("ATI")
void bg96_at(char *at)
{
  char tmp[256] = {0};
  int len = strlen(at);
  strncpy(tmp, at, len);
  tmp[len] = '\r';
  Serial1.write(tmp);
  delay(10);
  while (Serial1.available()) {
    bg96_rsp += char(Serial1.read());
    delay(2);
  }
  Serial.println(bg96_rsp);
  bg96_rsp = "";
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

  //__disable_irq();
  //if (rxCount > 0)
  {
    //c = Gsm_RxBuf[rxReadIndex];
    if (Serial1.available() > 0)
    {
      c = char(Serial1.read());
    }

    rxReadIndex++;
    if (rxReadIndex == GSM_RXBUF_MAXSIZE)
    {
      rxReadIndex = 0;
    }
    //rxCount--;
  }
  //__enable_irq();

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

  wait_len = strlen(p_pchExpectedRsp);
 
  memset(GSM_RSP, 0, 1600);
  do
  {
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
      if (cmp_p)
      {
        if (i > wait_len && rsp_value != NULL)
        {
          memcpy(rsp_value, GSM_RSP, i);
        }
        l_eErrCode = BG96_SUCCESS;
        break;
      }else{
        l_eErrCode = BG96_ERROR_FAILED;
      }
    }else{
        l_eErrCode = BG96_ERROR_PARAM;
    }
  }
  while (time_count > 0);

  return l_eErrCode;
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
