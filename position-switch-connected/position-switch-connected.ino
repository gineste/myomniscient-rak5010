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
   Description:   Main ino file

*/

/****************************************************************************************
   Include Files
 ****************************************************************************************/
#include <SoftwareSerial.h>
#include <Arduino.h>
#include <math.h>

#include "sensors.h"
#include "BG96.h"
#include "BG96_LTE.h"
#include "GNSS.h"
#include "cellular.h"
#include "config.h"
#include "timeout.h"

/****************************************************************************************
   Defines
 ****************************************************************************************/
#define DEBOUNCE_DELAY_MS  (200)

/****************************************************************************************
   Private type declarations
 ****************************************************************************************/

/****************************************************************************************
   Private function declarations
 ****************************************************************************************/
static void vUpdateTiming(void);        // update timings
static void vStatem_ContextSetup(void); // setup context

/****************************************************************************************
   Variable declarations
 ****************************************************************************************/
static sStatemContext_t g_sStatemContext = {
    .u64lastTimeUpdateMs = 0,     // in ms => last time we updated the timings
    .u32lastStatusS = 0,          // time in S since last send over network
};

static uint8_t g_u8SwitchEventReady = 0u;
static uint32_t g_u32LastDebounceTimeFull = 0u;
static uint32_t g_u32LastDebounceTimeEmpty = 0u;
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
  
  delay(10);
  digitalWrite(LED_GREEN_PIN, LOW);
  
  attachInterrupt(digitalPinToInterrupt(NRF_IO3), nrf_io3_it_cb, FALLING);
  attachInterrupt(digitalPinToInterrupt(NRF_IO4), nrf_io4_it_cb, FALLING);
  
  vStatem_ContextSetup();

  vSensorMngr_Init();
  u8SensorMngr_TORStateSet(0, (digitalRead(NRF_IO3) == LOW));
  u8SensorMngr_TORStateSet(1, (digitalRead(NRF_IO4) == LOW));

  bg96_init();

  // blink led once
  digitalWrite(LED_GREEN_PIN, HIGH);
  delay(250);
  digitalWrite(LED_GREEN_PIN, LOW);
  delay(250);

  // send status at boot and turn off
#if (SEND_STATUS_AT_BOOT == 1u)
  #ifdef DEBUG
  Serial.printf("Send status at boot..\r\n");
  delay(500);
  #endif
  
  Serial1.begin(SERIAL_BAUDRATE);
  while ( !Serial1 ) delay(10);   // for bg96 with uart1, softserial is limited in baudrate
  delay(5000);                    // necessary for BG96 boot on ext battery
  
  eBG96_TurnOn();
  vCellular_SendData();
  
  if (eBG96_TurnOff() != BG96_SUCCESS)
  {
    eBG96_TurnOff();
  }
  Serial1.end();
#endif
}

/**************************************************************************/
/*!
    @brief  The loop function runs over and over again forever
*/
/**************************************************************************/
void loop()
{
  uint64_t l_u64Timestamp = u32Time_getMs();
  uint8_t l_u8State = 0u;
#ifdef DEBUG
  Serial.printf("[%d] LOOP\r\n", (uint32_t) l_u64Timestamp);
#endif
  vUpdateTiming();

  // trailer is full
  if ((u32Time_getMs() - g_u32LastDebounceTimeFull) > DEBOUNCE_DELAY_MS)
  {  
    l_u8State = (digitalRead(NRF_IO3) == LOW);    // pull up
    
    if (l_u8State != u8SensorMngr_TORStateGet(0))
    {
      g_u8SwitchEventReady = 1u;
      u8SensorMngr_TORStateSet(0, (digitalRead(NRF_IO3) == LOW));
    }
  }

  // trailer is empty
  if ((u32Time_getMs() - g_u32LastDebounceTimeEmpty) > DEBOUNCE_DELAY_MS)
  {   
    l_u8State = (digitalRead(NRF_IO4) == LOW);    // pull up
    
    if (l_u8State != u8SensorMngr_TORStateGet(1))
    {
      g_u8SwitchEventReady = 1u;
      u8SensorMngr_TORStateSet(1, (digitalRead(NRF_IO4) == LOW));
    }
  }

  // check event msg send alarm
  if (1u == g_u8SwitchEventReady)
  {
    g_u8SwitchEventReady = 0u;

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
    vCellular_SendData();
    if (eBG96_TurnOff() != BG96_SUCCESS)
    {
      eBG96_TurnOff();
    }
    Serial1.end();
  }

  // check status msg send alarm
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
    vCellular_SendData();
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
  
  delay(1000);
}

/****************************************************************************************
   Private functions
 ****************************************************************************************/

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

static void nrf_io3_it_cb() {
    g_u32LastDebounceTimeFull = u32Time_getMs();
}

static void nrf_io4_it_cb() {
    g_u32LastDebounceTimeEmpty = u32Time_getMs();
}
/****************************************************************************************
   End Of File
 ****************************************************************************************/
