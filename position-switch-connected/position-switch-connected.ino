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
#include <Wire.h>
#include <math.h>
#include <Arduino_nRF5x_lowPower.h> // LowPower Library for nRF5x

#include "sensors.h"
#include "bg96.h"
#include "config.h"
#include "AT.h"

/****************************************************************************************
   Defines
 ****************************************************************************************/

/****************************************************************************************
   Private type declarations
 ****************************************************************************************/

/****************************************************************************************
   Private function declarations
 ****************************************************************************************/

/****************************************************************************************
   Variable declarations
 ****************************************************************************************/
TwoWire *wi_i2c = &Wire;
const int baudrate = 115200;

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

  Serial.begin (baudrate);
  while ( !Serial ) delay(10);   // for nrf52840 with native usb
  Serial.printf("%s\n", "********************EXOTIC SYSTEMS 2021************************");

  //nRF5x_lowPower.powerMode(POWER_MODE_OFF);
}

/**************************************************************************/
/*!
    @brief  The loop function runs over and over again forever
*/
/**************************************************************************/
void loop()
{
  Serial.println("loop");
  delay(4000);
}

/****************************************************************************************
   Private functions
 ****************************************************************************************/

/****************************************************************************************
   End Of File
 ****************************************************************************************/