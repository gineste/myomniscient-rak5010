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
#include <stdint.h>

#include "sensors.h"
#include "BG96.h"
#include "GNSS.h"
#include "cellular.h"
#include "config.h"
#include "timeout.h"

/****************************************************************************************
   Defines
 ****************************************************************************************/
#define DEBOUNCE_DELAY_MS  (100)

/****************************************************************************************
   Private type declarations
 ****************************************************************************************/

/****************************************************************************************
   Private function declarations
 ****************************************************************************************/
static void vUpdateTiming(void);        // update timings
static void vStatem_ContextSetup(void); // setup context

static void nrf_io2_it_cb();
static void nrf_io4_it_cb();

/****************************************************************************************
   Variable declarations
 ****************************************************************************************/
static sStatemContext_t g_sStatemContext = {
  .u32lastTimeUpdateS = 0,      // in s => last time we updated the timings
  .u32lastStatusS = 0,          // time in S since last send over network
};

static uint8_t g_u8SwitchEventReady = 0u;
static uint32_t g_u32LastDebounceTime = 0u;
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
  pinMode(NRF_IO2, INPUT_PULLUP);
  pinMode(NRF_IO4, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(NRF_IO2), nrf_io2_it_cb, FALLING);
  attachInterrupt(digitalPinToInterrupt(NRF_IO4), nrf_io4_it_cb, FALLING);

  // blink led once
  digitalWrite(LED_GREEN_PIN, HIGH);
  delay(250);
  digitalWrite(LED_GREEN_PIN, LOW);
  delay(250);

#if (WDG_ENABLE == 1u) 
  // Configure WDT.
  NRF_WDT->CONFIG         = 0x01;     // Configure WDT to run when CPU is asleep
  NRF_WDT->CRV            = (TIMEOUT_WDG_S * 32768) - 1;  // Timeout set to 120 seconds, timeout[s] = (CRV-1)/32768
  NRF_WDT->RREN           = 0x01;     // Enable the RR[0] reload register
  NRF_WDT->TASKS_START    = 1;        // Start WDT    
#endif

  // setup context
  vStatem_ContextSetup();

  // Init sensors
  vSensorMngr_Init();

  // Read TORs state
  vSensorMngr_TORStateSet(0, (digitalRead(NRF_IO2) == LOW));
  vSensorMngr_TORStateSet(1, (digitalRead(NRF_IO4) == LOW));

  // Init BG96
  vBG96_Init();

  // send status at boot and turn off
#if (SEND_STATUS_AT_BOOT == 1u)
#ifdef DEBUG
  Serial.printf("Send status at boot..\r\n");
  delay(100);
#endif

  Serial1.begin(SERIAL_BAUDRATE);
  while ( !Serial1 ) delay(10);   // for bg96 with uart1, softserial is limited in baudrate

  eBG96_TurnOn();

  eGNSS_TurnOn();
  eGNSS_UpdatePosition(TIME_TO_FIX_MAX);
  eGNSS_TurnOff();

  vCellular_SendData(CELLULAR_MSG_HB);
#endif

  if (eBG96_TurnOff() != BG96_SUCCESS)
  {
    eBG96_TurnOff();
  }

#if (SEND_STATUS_AT_BOOT == 1u)
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
  uint32_t l_u32Timestamp = u32Time_getMs();
  uint8_t l_u8State = 0u;
#ifdef DEBUG
  Serial.printf("[%d] LOOP\r\n", l_u32Timestamp);
#endif
  vUpdateTiming();

#if (WDG_ENABLE == 1u) 
  // Reload the WDTs RR[0] reload register
  NRF_WDT->RR[0] = WDT_RR_RR_Reload; 
#endif

  if ((u32Time_getMs() - g_u32LastDebounceTime) > DEBOUNCE_DELAY_MS)
  {
    l_u8State = (digitalRead(NRF_IO2) == LOW);    // pull up

    if (l_u8State != u8SensorMngr_TORStateGet(SENSOR_MNGR_TOR2))
    {
      g_u8SwitchEventReady = 1u;
      vSensorMngr_TORStateSet(SENSOR_MNGR_TOR2, (digitalRead(NRF_IO2) == LOW));
    }

    l_u8State = (digitalRead(NRF_IO4) == LOW);    // pull up

    if (l_u8State != u8SensorMngr_TORStateGet(SENSOR_MNGR_TOR1))
    {
      g_u8SwitchEventReady = 1u;
      vSensorMngr_TORStateSet(SENSOR_MNGR_TOR1, (digitalRead(NRF_IO4) == LOW));
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
    eBG96_TurnOn();

    eGNSS_TurnOn();
    eGNSS_UpdatePosition(TIME_TO_FIX_MAX);
    eGNSS_TurnOff();

    vCellular_SendData(CELLULAR_MSG_EVENT);

    if (eBG96_TurnOff() != BG96_SUCCESS)
    {
      eBG96_TurnOff();
    }
    Serial1.end();
  }

  // check HB msg send alarm
  if ((STATUS_SEND_DUTY > 0) && (g_sStatemContext.u32lastStatusS >= STATUS_SEND_DUTY))
  {
    g_sStatemContext.u32lastStatusS = 0;
#ifdef DEBUG
    Serial.printf("send HB\r\n");
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
    eBG96_TurnOn();

    eGNSS_TurnOn();
    eGNSS_UpdatePosition(TIME_TO_FIX_MAX);
    eGNSS_TurnOff();

    vCellular_SendData(CELLULAR_MSG_HB);

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
   On every machine cycle, call the timing update;
   The precision is 1S so the scheduler period should not be lower than 1s
*/
static void vUpdateTiming()
{
  uint32_t now = (u32Time_getMs() / 1000u);
  uint32_t durationS = 0u;

  if (now < g_sStatemContext.u32lastTimeUpdateS){
    durationS = UINT32_MAX - g_sStatemContext.u32lastTimeUpdateS + now;
  }else{
    durationS = now - g_sStatemContext.u32lastTimeUpdateS; 
  }

  g_sStatemContext.u32lastTimeUpdateS += durationS;
  g_sStatemContext.u32lastStatusS += durationS;

}

/**
   @brief Setup Statem context
   @param None
   @retval None
*/
static void vStatem_ContextSetup(void)
{
  /* context default values */
  g_sStatemContext.u32lastTimeUpdateS = 0;
  g_sStatemContext.u32lastStatusS = 0;
}

static void nrf_io2_it_cb() {
  g_u32LastDebounceTime = u32Time_getMs();
}

static void nrf_io4_it_cb() {
  g_u32LastDebounceTime = u32Time_getMs();
}

/****************************************************************************************
   End Of File
 ****************************************************************************************/
