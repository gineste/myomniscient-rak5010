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

#define DEBUG
#define SERIAL_BAUDRATE     (115200)

#define SEND_STATUS_AT_BOOT (1u)             // send status at boot if def
#define ENABLE_GPS          (1u)
#define STATUS_SEND_DUTY    (1800u)          // period in seconds to send status  msg

#define SERVER_URL          "https://webhook.site/15cc74bf-54b7-4b93-8746-c023eee63d32"
#define APN_URL             "nxt17.net" //"sl2sfr"
#define APN_USERNAME        ""
#define APN_PASSWORD        ""

#define BG96_RX_PIN  (6)
#define BG96_TX_PIN  (8)

#define LED_GREEN_PIN (12)

#define NRF_IO3       (34)
#define NRF_IO4       (33)

/****************************************************************************************
   Type definitions
 ****************************************************************************************/
typedef struct _STATEM_CTX_ {
  uint64_t  u64lastTimeUpdateMs;    /* in ms => last time we updated the timings */
  uint32_t  u32lastStatusS;         /* time in S since last send over network */
} sStatemContext_t;

/****************************************************************************************
   Public function declarations
 ****************************************************************************************/

#endif /* CONFIG_H_ */
