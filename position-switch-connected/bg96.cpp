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
//Pin define
#define bg96_W_DISABLE  29
#define bg96_RESET      28
#define bg96_PWRKEY     2
#define bg96_GPS_EN     39
#define bg96_STATUS     31

#define LTE_CMD_LEN        (256u)

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

//gps data
eGnssCodes_t eGNSS_GetPosition(sPosition_t * p_psPosition)
{
  eBG96ErrorCode_t l_eBG96Code = BG96_SUCCESS;
  eGnssCodes_t l_eCode = GNSS_C_SUCCESS;
  int32_t l_s32scanResult = -1;
  int32_t l_s32Time = 0;
  int32_t l_s32DecimalTime = 0;
  int32_t l_s32Date = 0;
  char l_sLatitude[15u], l_sLongitude[15u], l_sHdop[5u], l_sAltitude[10u], l_sCourseOverGround[10u], l_sSpeedKph[10u], l_sSpeedKnots[10u] = {0};
  
  /* send GPS command to BG96 */
  memset(GSM_RSP, 0, GSM_GENER_CMD_LEN);
  Serial1.write("AT+QGPSLOC=2\r");
  l_eBG96Code = eBG96_WaitResponse(GSM_RSP, CMD_TIMEOUT, GSM_CMD_RSP_OK_RF);

  if ((l_eBG96Code == GNSS_C_SUCCESS) && (p_psPosition != NULL))
  {
      /* init buffers */
      memset(l_sLatitude, 0, 15);
      memset(l_sLongitude, 0, 15);
      memset(l_sHdop, 0, 5);
      memset(l_sAltitude, 0, 10);
      memset(l_sCourseOverGround, 0, 10);
      memset(l_sSpeedKph, 0, 10);
      memset(l_sSpeedKnots, 0, 10);

      /* parse response */
      l_s32scanResult  = sscanf(strstr(GSM_RSP, "+QGPSLOC:"), "+QGPSLOC: %d.%d,%[0-9.],%[0-9.],%[0-9.],%[0-9.],%c,%[0-9.],%[0-9.],%[0-9.],%d,%hu",
                            (int*) & (l_s32Time),
                            (int*) & (l_s32DecimalTime),
                            l_sLatitude,
                            l_sLongitude,
                            l_sHdop,
                            l_sAltitude,
                            &(p_psPosition->u8FixType),
                            l_sCourseOverGround,
                            l_sSpeedKph,
                            l_sSpeedKnots,
                            (int*) & (l_s32Date),
                            &(p_psPosition->u16Satellites));

       /* convert to float */
       p_psPosition->f32Latitude = atof(l_sLatitude);
       p_psPosition->f32Longitude = atof(l_sLongitude);
       p_psPosition->f32Hdop = atof(l_sHdop);
       p_psPosition->f32Altitude = atof(l_sAltitude);
       p_psPosition->f32CourseOverGround = atof(l_sCourseOverGround);
       p_psPosition->f32Speedkph = atof(l_sSpeedKph);
       p_psPosition->f32Speedknots = atof(l_sSpeedKnots);

    if (l_s32scanResult >= 11)
    {
      if (p_psPosition->u8FixType > 0x30)
      {
        p_psPosition->u8FixType -= 0x30;
      }

      /* split time from hhmmss to hh mm ss */
      p_psPosition->u8Seconds = l_s32Time % 100;
      l_s32Time -= p_psPosition->u8Seconds;
      l_s32Time /= 100;

      p_psPosition->u8Minutes = l_s32Time % 100;
      l_s32Time -= p_psPosition->u8Minutes;
      l_s32Time /= 100;

      p_psPosition->u8Hours = l_s32Time % 100;

      /* split date from ddmmyy to dd mm yy */
      p_psPosition->u8Year = l_s32Date % 100;
      l_s32Date -= p_psPosition->u8Year;
      l_s32Date /= 100;

      p_psPosition->u8Month = l_s32Date % 100;
      l_s32Date -= p_psPosition->u8Month;
      l_s32Date /= 100;

      p_psPosition->u8Day = l_s32Date % 100;

#ifdef DEBUG
      Serial.println("Parse GPS OK");

      Serial.printf("Date = %u/%u/%u %u:%u:%u\r\n", p_psPosition->u8Day, p_psPosition->u8Month, 
                      p_psPosition->u8Year, p_psPosition->u8Hours, p_psPosition->u8Minutes, p_psPosition->u8Seconds);
                      
      Serial.printf("Lat = %f\r\n", p_psPosition->f32Latitude);
      Serial.printf("Long = %f\r\n", p_psPosition->f32Longitude);
      Serial.printf("Hdop = %f\r\n", p_psPosition->f32Hdop);
      Serial.printf("altitude = %f\r\n", p_psPosition->f32Altitude);
      Serial.printf("Fix type = %u\r\n", p_psPosition->u8FixType);
      Serial.printf("course over ground = %f\r\n", p_psPosition->f32CourseOverGround);
      Serial.printf("speed kph = %f\r\n", p_psPosition->f32Speedkph);
      Serial.printf("speed knots = %f\r\n", p_psPosition->f32Speedknots);
      Serial.printf("satellites = %hu\r\n", p_psPosition->u16Satellites);
#endif

      /* Fix with position */
      l_eCode = GNSS_C_SUCCESS;
    } else {
      /* Parsing error, consider no position */
      l_eCode = GNSS_ERROR_NO_POSITION;
    #ifdef DEBUG
      Serial.printf("Parse GPS FAIL ; scan = %d\r\n", l_s32scanResult);
    #endif
    }
  }
  else
  {
    /* no position */
    l_eCode = GNSS_ERROR_NO_POSITION;
  #ifdef DEBUG
    Serial.println("GPS FAILED");
  #endif
  }

  return l_eCode;
}

//network connect
void connect(uint8_t p_u8Flag) {
  eBG96_SetRATSearchSeq("01");  // GSM
  eBG96_SendCommand("AT+QICSGP=1,1,\"nxt17.net\",\"\",\"\",1", GSM_CMD_RSP_OK_RF, CMD_TIMEOUT);
  
  eBG96_SendCommand("AT+QNWINFO", GSM_CMD_RSP_OK_RF, CMD_TIMEOUT);

  eBG96_SendCommand("AT+QIACT=1", GSM_CMD_RSP_OK_RF, CMD_TIMEOUT);

  //Serial.println("get time");
  //bg96_at("AT+QLTS=1"); //query GMT time from network
  //delay(2000);

  eBG96_SendCommand("AT+QHTTPCFG=\"contextid\",1", GSM_CMD_RSP_OK_RF, CMD_TIMEOUT);
  eBG96_SendCommand("AT+QHTTPCFG=\"responseheader\",1", GSM_CMD_RSP_OK_RF, CMD_TIMEOUT);

  //POST request
  bg96_at("AT+QHTTPURL=57,80"); //57 is length of the url
  delay(3000);
  //Serial1.write("https://webhook.site/b80027c3-ec69-4694-b32d-b640549c6213\r");
  Serial1.write("https://webhook.site/15cc74bf-54b7-4b93-8746-c023eee63d32\r");
  delay(3000);
  bg96_at("AT+QHTTPPOST=58,80,80");//48 is length of the post data
  delay(3000);

  if (p_u8Flag == 0u)
  {
    // empty
    Serial1.write("{TOR_state: {TOR1_current_state: 1,TOR2_current_state: 0}}\r");
  }
  else
  {
    // full
    Serial1.write("{TOR_state: {TOR1_current_state: 0,TOR2_current_state: 1}}\r");
  }
  delay(3000);
}

/**@brief Set Apn information
   @param p_pchApn         Apn operator address
   @param p_pchUser        Apn access username
   @param p_pchPassword    Apn access password

   @info Always use IPv4 context

   @retval BG96_SUCCESS
   @retval BG96_ERROR_FAILED
   @retval BG96_ERROR_PARAM
*/
eBG96ErrorCode_t eBG96_SetApnContext(char * p_pchApn, char * p_pchUser, char * p_pchPassword)
{
  eBG96ErrorCode_t l_eCode = BG96_SUCCESS;
  char l_achCmd[MAX_CMD_LEN] = {0};

  /* By default always use IPv4 context */
  if ((p_pchApn != NULL) && (p_pchUser != NULL) && (p_pchPassword != NULL))
  {
    snprintf(l_achCmd, MAX_CMD_LEN, "AT+QICSGP=1,1,\"%s\",\"%s\",\"%s\",1", p_pchApn, p_pchUser, p_pchPassword);
    bg96_at(l_achCmd);
    l_eCode = BG96_SUCCESS;
  } else {
    l_eCode = BG96_ERROR_PARAM;
  }

  return l_eCode;

}

/**@brief Active TCP/IP context
   @param p_pchIp         Allocated IP returned by server
   @retval BG96_SUCCESS
   @retval BG96_ERROR_FAILED
   @retval BG96_ERROR_PARAM
*/
eBG96ErrorCode_t eBG96_ActiveContext(void)
{
  bg96_at("AT+QIACT=1");
  return BG96_SUCCESS;
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

/**@brief Turn on GNSS module on BG96
 * @param None
 * @retval BG96_SUCCESS
 * @retval BG96_ERROR_FAILED
 * @retval BG96_ERROR_PARAM
 */
void vBG96_GNSS_TurnOn(void)
{
  digitalWrite(bg96_GPS_EN, HIGH);
  vTime_WaitMs(10);
  eBG96_SendCommand("AT+QGPS=1", GSM_CMD_RSP_OK_RF, CMD_TIMEOUT);
  //bg96_at("AT+QGPS=1");
}

/**@brief Turn on GNSS module on BG96
 * @param None
 * @retval BG96_SUCCESS
 * @retval BG96_ERROR_FAILED
 * @retval BG96_ERROR_PARAM
 */
void vBG96_GNSS_TurnOff(void)
{
  eBG96_SendCommand("AT+QGPSEND", GSM_CMD_RSP_OK_RF, CMD_TIMEOUT);
  digitalWrite(bg96_GPS_EN, LOW);
  //bg96_at("AT+QGPSEND");
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
