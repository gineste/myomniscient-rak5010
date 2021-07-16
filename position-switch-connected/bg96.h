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
#define  GSM_RXBUF_MAXSIZE           1600
#define  MAX_CMD_LEN                  (256u)

#define  GSM_GENER_CMD_LEN                    (128)
#define  GSM_GENER_CMD_TIMEOUT                (500)  //ms
#define  GSM_OPENSOCKET_CMD_TIMEOUT           (15000)  //ms
#define  GSM_GETDNSIP_CMD_TIMEOUT                (30*1000)  //ms

#define  GSM_OPENSOCKET_OK_STR          "CONNECT OK\r\n"
#define  GSM_OPENSOCKET_FAIL_STR        "CONNECT FAIL\r\n"
#define  FIX_BAUD_URC                    "RDY\r\n"
#define  GSM_CMD_CRLF                   "\r\n"
#define  GSM_CMD_RSP_OK_RF              "OK\r\n"
#define  GSM_CMD_RSP_OK                 "OK"
#define  GSM_CHECKSIM_RSP_OK            "+CPIN: READY"

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

typedef struct _GNSS_POSITION_ {
   uint8_t u8Hours;
   uint8_t u8Minutes;
   uint8_t u8Seconds;
   uint8_t u8Day;
   uint8_t u8Month;
   uint8_t u8Year;

   float f32Latitude;
   float f32Longitude;
   float f32Altitude;

   float f32CourseOverGround;
   float f32Speedkph;
   float f32Speedknots;

   float f32Hdop;
   uint16_t u16Satellites;
   uint8_t u8FixType;

   uint8_t u8TimeToFix;

} sPosition_t;
/****************************************************************************************
 * Public function declarations
 ****************************************************************************************/
void bg96_init();         //bg96 power up
void bg96_at_wait_rsp(char *at);
void bg96_at(char *at);   //this function is suitable for most AT commands of bg96. e.g. bg96_at("ATI")
void gps_get_position(sPosition_t * p_psPosition) ;
void connect(uint8_t p_u8Flag);
eBG96ErrorCode_t  eBG96_SetApnContext(char * p_pchApn, char * p_pchUser, char * p_pchPassword);
eBG96ErrorCode_t  eBG96_ActiveContext(void);
int Gsm_WaitRspOK(char *rsp_value, uint16_t timeout_ms, uint8_t is_rf);

#endif /* BG96_H_ */
