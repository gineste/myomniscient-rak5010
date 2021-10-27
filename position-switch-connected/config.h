/*
      ____  _  _   __   ____   __    ___    ____  _  _  ____  ____  ____  _  _  ____
     (  __)( \/ ) /  \ (_  _) (  )  / __)  / ___)( \/ )/ ___)(_  _)(  __)( \/ )/ ___)
      ) _)  )  ( (  O )  )(    )(  ( (__   \___ \ )  / \___ \  )(   ) _) / \/ \\___ \
     (____)(_/\_) \__/  (__)  (__)  \___)  (____/(__/  (____/ (__) (____)\_)(_/(____/

   Copyright (c) 2021 EXOTIC SYSTEMS. All Rights Reserved.

   Licensees are granted free, non-transferable use of the information. NO WARRANTY
   of ANY KIND is provided. This heading must NOT be removed from the file.

*/
#ifndef CONFIG_H_
#define CONFIG_H_

/****************************************************************************************
   Include Files
 ****************************************************************************************/

/****************************************************************************************
   Defines
 ****************************************************************************************/

// comment to disable debug infos
//#define DEBUG

#define WDG_ENABLE          (1u)
#if (WDG_ENABLE == 1u)
  #define TIMEOUT_WDG_S     (8)
#endif

#define SERIAL_BAUDRATE     (115200)

#define SEND_STATUS_AT_BOOT (1u)             // send HB msg at boot
#define ENABLE_GPS          (1u)
#define STATUS_SEND_DUTY    (86400u)         // period in seconds to send status  msg

// IOs declaration
// NB: DO NOT USE NRF_IO3 otherwise it may crash at boot
// because it is used as Serial gpio with bootloader
#define NRF_IO2       (20)
#define NRF_IO4       (33)

#define BG96_RX_PIN  (6)
#define BG96_TX_PIN  (8)

#define LED_GREEN_PIN (12)

/****************************************************************************************
   Type definitions
 ****************************************************************************************/
typedef struct _STATEM_CTX_ {
  uint32_t  u32lastTimeUpdateS;     /* in s => last time we updated the timings */
  uint32_t  u32lastStatusS;         /* time in S since last send over network */
} sStatemContext_t;

/****************************************************************************************
   Public function declarations
 ****************************************************************************************/

#endif /* CONFIG_H_ */
