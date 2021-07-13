/* 
 *    ____  _  _   __   ____   __    ___    ____  _  _  ____  ____  ____  _  _  ____
 *   (  __)( \/ ) /  \ (_  _) (  )  / __)  / ___)( \/ )/ ___)(_  _)(  __)( \/ )/ ___)
 *    ) _)  )  ( (  O )  )(    )(  ( (__   \___ \ )  / \___ \  )(   ) _) / \/ \\___ \     
 *   (____)(_/\_) \__/  (__)  (__)  \___)  (____/(__/  (____/ (__) (____)\_)(_/(____/
 *
 * Copyright (c) 2021 EXOTIC SYSTEMS. All Rights Reserved.
 *
 * Licensees are granted free, non-transferable use of the information. NO WARRANTY 
 * of ANY KIND is provided. This heading must NOT be removed from the file.
 *
 */
#ifndef BG96_H_
#define BG96_H_

/****************************************************************************************
 * Include Files
 ****************************************************************************************/

/****************************************************************************************
 * Defines
 ****************************************************************************************/

/****************************************************************************************
 * Type definitions
 ****************************************************************************************/
 typedef enum _BG96_ERR_CODE_{
 BG96_SUCCESS,
  BG96_ERROR_PARAM,
  BG96_ERROR_FAILED,
  BG96_ERROR_TRANSMIT,
  BG96_ERROR_TIMEOUT,
  BG96_ERROR_REFUSED,

  BG96_ERROR_MAXID
} eBG96ErrorCode_t;
/****************************************************************************************
 * Public function declarations
 ****************************************************************************************/
void bg96_init();         //bg96 power up
void bg96_at(char *at);   //this function is suitable for most AT commands of bg96. e.g. bg96_at("ATI")
void gps_show();          //gps data
void connect();
eBG96ErrorCode_t  eBG96_SetApnContext(char * p_pchApn, char * p_pchUser, char * p_pchPassword);
eBG96ErrorCode_t  eBG96_ActiveContext(void);

#endif /* BG96_H_ */
