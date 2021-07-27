/*
 *    ____  _  _   __   ____   __    ___    ____  _  _  ____  ____  ____  _  _  ____
 *   (  __)( \/ ) /  \ (_  _) (  )  / __)  / ___)( \/ )/ ___)(_  _)(  __)( \/ )/ ___)
 *    ) _)  )  ( (  O )  )(    )(  ( (__   \___ \ )  / \___ \  )(   ) _) / \/ \\___ \
 *   (____)(_/\_) \__/  (__)  (__)  \___)  (____/(__/  (____/ (__) (____)\_)(_/(____/
 *
 * Copyright (c) 2017 EXOTIC SYSTEMS. All Rights Reserved.
 *
 * Licensees are granted free, non-transferable use of the information. NO WARRANTY
 * of ANY KIND is provided. This heading must NOT be removed from the file.
 *
 * Date:          17/02/2020 (dd MM YYYY)
 * Author:        Martin CORNU
 * Description:   Timer
 *
 */

/****************************************************************************************
 * Include Files
 ****************************************************************************************/
#include <stdint.h>
#include <stdio.h>
#include <Arduino.h>

#include "config.h"

#include "timeout.h"

/****************************************************************************************
 * Defines
 ****************************************************************************************/

/****************************************************************************************
 * Private type declarations
 ****************************************************************************************/

/****************************************************************************************
 * Private function declarations
 ****************************************************************************************/

/****************************************************************************************
 * Variable declarations
 ****************************************************************************************/
/****************************************************************************************
 * Public functions
 ****************************************************************************************/
 uint32_t u32Time_getMs(void)
 {
    return (uint32_t) millis();
 }
 
 void vTime_WaitMs(uint32_t p_u32Timeout)
 {
   uint64_t l_u64TimeMs = u32Time_getMs();
    uint32_t l_u32TimerAlarm = p_u32Timeout + l_u64TimeMs;
    
    if(p_u32Timeout > 0)
    {
       do
       {
       #if (WDG_ENABLE == 1u) 
         // Reload the WDTs RR[0] reload register
         NRF_WDT->RR[0] = WDT_RR_RR_Reload; 
       #endif
          /* Wait */
         l_u64TimeMs = u32Time_getMs();
       }while(l_u32TimerAlarm > l_u64TimeMs);
    }
 }
    
/****************************************************************************************
 * Private functions
 ****************************************************************************************/


/****************************************************************************************
 * End Of File
 ****************************************************************************************/
