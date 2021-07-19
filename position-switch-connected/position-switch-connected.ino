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
#include "AT.h"

/****************************************************************************************
   Defines
 ****************************************************************************************/
#define SERIAL_BAUDRATE   (115200)
/****************************************************************************************
   Private type declarations
 ****************************************************************************************/

/****************************************************************************************
   Private function declarations
 ****************************************************************************************/
static void nrf_io3_it_cb();  // trailer full interrupt CB
static void nrf_io4_it_cb();  // trailer empty interrupt CB

/****************************************************************************************
   Variable declarations
 ****************************************************************************************/
uint8_t g_u8FlagFull = 0;     // trailer full flag
uint8_t g_u8FlagEmpty = 0;    // trailer empty flag

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
  
  attachInterrupt(digitalPinToInterrupt(NRF_IO3), nrf_io3_it_cb, FALLING);
  attachInterrupt(digitalPinToInterrupt(NRF_IO4), nrf_io4_it_cb, FALLING);

  bg96_init();
  Serial1.begin(SERIAL_BAUDRATE);
  while ( !Serial1 ) delay(10);   // for bg96 with uart1, softserial is limited in baudrate
  delay(5000);
  bg96_at_wait_rsp("ATE1");       //turn off the echo mode  (ATE1  echo on)
  bg96_at_wait_rsp("AT+QGPSCFG=\"gpsnmeatype\",1");
  bg96_at_wait_rsp("AT+QGPS=1, 1, 1, 1, 1");  
  
  bg96_at("AT+CGATT=1");          //Connect to network
  delay(3000);
  bg96_at("AT+QCFG=1"); //GSM mode
  delay(2000);
 
  bg96_at_wait_rsp("AT+QICSGP=1,1,\"nxt17.net\",\"\",\"\",1");
  
  digitalWrite(LED_GREEN_PIN, HIGH);
  delay(500);
  digitalWrite(LED_GREEN_PIN, LOW);
  delay(500);
  
  //nRF5x_lowPower.powerMode(POWER_MODE_OFF);
}

/**************************************************************************/
/*!
    @brief  The loop function runs over and over again forever
*/
/**************************************************************************/
void loop()
{
#ifdef DEBUG
  Serial.println("loop");
  delay(1000);
#endif

  // trailer is full
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
  }

  /*
  bg96_at("ATI");  //display product information
  delay(2000);
  bg96_at("AT+QNWINFO"); 
  delay(2000);*/
 
  sPosition_t l_sPosition = {0};
  eGNSS_GetPosition(&l_sPosition);

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
/****************************************************************************************
   End Of File
 ****************************************************************************************/
