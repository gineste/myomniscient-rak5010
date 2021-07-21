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
   Description:   Main ino file-

*/

/****************************************************************************************
   Include Files
 ****************************************************************************************/
#include <SoftwareSerial.h>
#include <Arduino.h>
#include <math.h>
#include <Arduino_nRF5x_lowPower.h> // LowPower Library for nRF5x

#include "sensors.h"
#include "bg96.h"
#include "config.h"
#include "timeout.h"

/****************************************************************************************
   Defines
 ****************************************************************************************/

/****************************************************************************************
   Private type declarations
 ****************************************************************************************/

/****************************************************************************************
   Private function declarations
 ****************************************************************************************/
static void nrf_io3_it_cb(void);        // trailer full interrupt CB
static void nrf_io4_it_cb(void);        // trailer empty interrupt CB
static void vUpdateTiming(void);        // update timings
static void vStatem_ContextSetup(void); // setup context

/****************************************************************************************
   Variable declarations
 ****************************************************************************************/
uint8_t g_u8FlagFull = 0;     // trailer full flag
uint8_t g_u8FlagEmpty = 0;    // trailer empty flag


static sStatemContext_t g_sStatemContext = {
    .u64lastTimeUpdateMs = 0,     // in ms => last time we updated the timings
    .u32lastStatusS = 0,          // time in S since last send over network
};
/****************************************************************************************
   Public functions
 ****************************************************************************************/

/**************************************************************************/
/*!
    @brief  The setup function runs once when reset the board
*/
/**************************************************************************/
void setup()
{
#ifdef DEBUG
  Serial.begin (SERIAL_BAUDRATE);
  while ( !Serial ) delay(10);   // for nrf52840 with native usb
  Serial.printf("%s\n", "********************EXOTIC SYSTEMS 2021************************");
#endif

  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(NRF_IO3, INPUT_PULLUP);
  pinMode(NRF_IO4, INPUT_PULLUP);

  digitalWrite(LED_GREEN_PIN, LOW);
  
  /*attachInterrupt(digitalPinToInterrupt(NRF_IO3), nrf_io3_it_cb, FALLING);
  attachInterrupt(digitalPinToInterrupt(NRF_IO4), nrf_io4_it_cb, FALLING);*/
  
 // nRF5x_lowPower.enableWakeupByInterrupt(NRF_IO3, FALLING);
 // nRF5x_lowPower.enableWakeupByInterrupt(NRF_IO4, FALLING);

  vStatem_ContextSetup();

  bg96_init();
  Serial1.begin(SERIAL_BAUDRATE);
  while ( !Serial1 ) delay(10);   // for bg96 with uart1, softserial is limited in baudrate
  delay(5000);                    // necessary for BG96 boot on ext battery

  // blink led once
  digitalWrite(LED_GREEN_PIN, HIGH);
  delay(250);
  digitalWrite(LED_GREEN_PIN, LOW);
  delay(250);
  
  //eBG96_TurnOn();
  //eBG96_SendCommand("ATE0", GSM_CMD_RSP_OK_RF, CMD_TIMEOUT);        // turn off the echo mode
  //eBG96_SendCommand("AT+CFUN=4", GSM_CMD_RSP_OK_RF, CMD_TIMEOUT); // airplane mode
  //eBG96_SendCommand("AT+QGPSCFG=\"gpsnmeatype\",1", GSM_CMD_RSP_OK_RF, CMD_TIMEOUT);
  //eBG96_SendCommand("AT+QGPS=1", GSM_CMD_RSP_OK_RF, CMD_TIMEOUT);  
  //eBG96_TurnOff();
  
  //bg96_at("AT+CGATT=1");          //Connect to network
  //delay(3000);
  //eBG96_SendCommand("AT+CGATT=1", GSM_CMD_RSP_OK_RF, APN_TIMEOUT);  
  //eBG96_SetRATSearchSeq("01");       /* GSM */
  //bg96_at("AT+QCFG=1"); //GSM mode
  //delay(2000);
  //eBG96_SendCommand("AT+QICSGP=1,1,\"nxt17.net\",\"\",\"\",1", GSM_CMD_RSP_OK_RF, CMD_TIMEOUT);
  //eBG96_SendCommand("AT+QIACT=1", GSM_CMD_RSP_OK_RF, APN_TIMEOUT);
  
  //connect(0u);

  if (eBG96_TurnOff() != BG96_SUCCESS)
  {
    eBG96_TurnOff();
  }
  vBG96_GNSS_TurnOff();
  Serial1.end();
}

/**************************************************************************/
/*!
    @brief  The loop function runs over and over again forever
*/
/**************************************************************************/
void loop()
{
  uint64_t l_u64Timestamp = u32Time_getMs();
#ifdef DEBUG
  Serial.printf("[%d] LOOP\r\n", (uint32_t) l_u64Timestamp);
#endif
  vUpdateTiming();

  if ((STATUS_SEND_DUTY > 0) && (g_sStatemContext.u32lastStatusS >= STATUS_SEND_DUTY)) 
  {
    g_sStatemContext.u32lastStatusS = 0;
  #ifdef DEBUG
    Serial.printf("send status\r\n");
  #endif
  
    // blink led twice
    digitalWrite(LED_GREEN_PIN, HIGH);
    delay(250);
    digitalWrite(LED_GREEN_PIN, LOW);
    delay(250);
    digitalWrite(LED_GREEN_PIN, HIGH);
    delay(250);
    digitalWrite(LED_GREEN_PIN, LOW);
    delay(250);
    
    Serial1.begin(SERIAL_BAUDRATE);
    while ( !Serial1 ) delay(10);   // for bg96 with uart1, softserial is limited in baudrate
    delay(5000);                    // necessary for BG96 boot on ext battery
    eBG96_TurnOn();
    connect(0u);
    if (eBG96_TurnOff() != BG96_SUCCESS)
    {
      eBG96_TurnOff();
    }
    Serial1.end();

    // blink led twice
    digitalWrite(LED_GREEN_PIN, HIGH);
    delay(250);
    digitalWrite(LED_GREEN_PIN, LOW);
    delay(250);
    digitalWrite(LED_GREEN_PIN, HIGH);
    delay(250);
    digitalWrite(LED_GREEN_PIN, LOW);
    delay(250);
  }

 /* // trailer is full
  if (g_u8FlagFull == 1u)
  {
    g_u8FlagFull = 0u;
    if (digitalRead(NRF_IO4) == HIGH)
    {
      digitalWrite(LED_GREEN_PIN, HIGH);   
      delay(100);
      connect(1u);
    }
  }

  // trailer is empty
  if (g_u8FlagEmpty == 1u)
  {
    g_u8FlagEmpty = 0u;
    if (digitalRead(NRF_IO3) == HIGH)
    {
      digitalWrite(LED_GREEN_PIN, LOW);   
      delay(100);
      connect(0u);
    }
  }*/
  
  delay(1000);
}

/****************************************************************************************
   Private functions
 ****************************************************************************************/
static void nrf_io3_it_cb() {
  g_u8FlagFull = 1;
}

static void nrf_io4_it_cb() {
  g_u8FlagEmpty = 1;
}

/**
 * On every machine cycle, call the timing update;
 * The precision is 1S so the scheduler period should not be lower than 1s
 */
static void vUpdateTiming()
{
  uint64_t now = u32Time_getMs();
  uint32_t durationS =(uint32_t)((now - g_sStatemContext.u64lastTimeUpdateMs)/1000);

  g_sStatemContext.u64lastTimeUpdateMs += (durationS*1000);
  g_sStatemContext.u32lastStatusS += durationS;

}

/**
 * @brief Setup Statem context
 * @param None
 * @retval None
 */
static void vStatem_ContextSetup(void)
{
  /* context default values */
  g_sStatemContext.u64lastTimeUpdateMs = 0;
  g_sStatemContext.u32lastStatusS = 0;
}
/****************************************************************************************
   End Of File
 ****************************************************************************************/
