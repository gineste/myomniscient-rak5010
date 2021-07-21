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
   Description:   BG96 LTE / GSM

*/

/****************************************************************************************
   Include Files
 ****************************************************************************************/
#include <stdint.h>
#include <Arduino.h>

#include "BG96.h"

#include "BG96_LTE.h"

/****************************************************************************************
   Defines
 ****************************************************************************************/
#define LTE_CMD_LEN        (256u)

/****************************************************************************************
   Private type declarations
 ****************************************************************************************/

/****************************************************************************************
   Private function declarations
 ****************************************************************************************/

/****************************************************************************************
   Variable declarations
 ****************************************************************************************/

/****************************************************************************************
   Public functions
 ****************************************************************************************/

/**@brief Set Apn information
   @param p_pchApn         Apn operator address
   @param p_pchUser        Apn access username
   @param p_pchPassword    Apn access password

   @info Always use IPv4 context

   @retval BG96_SUCCESS
   @retval BG96_ERROR_FAILED
   @retval BG96_ERROR_PARAM
*/
eBG96ErrorCode_t eBG96_SetApnContext(char * p_pchApn, char * p_pchUser, char * p_pchPassword)
{
  eBG96ErrorCode_t l_eCode = BG96_SUCCESS;
  char l_achCmd[MAX_CMD_LEN] = {0};

  /* By default always use IPv4 context */
  if ((p_pchApn != NULL) && (p_pchUser != NULL) && (p_pchPassword != NULL))
  {
    snprintf(l_achCmd, MAX_CMD_LEN, "AT+QICSGP=1,1,\"%s\",\"%s\",\"%s\",1", p_pchApn, p_pchUser, p_pchPassword);
    bg96_at(l_achCmd);
    l_eCode = BG96_SUCCESS;
  } else {
    l_eCode = BG96_ERROR_PARAM;
  }

  return l_eCode;

}

/**@brief Active TCP/IP context
   @param p_pchIp         Allocated IP returned by server
   @retval BG96_SUCCESS
   @retval BG96_ERROR_FAILED
   @retval BG96_ERROR_PARAM
*/
eBG96ErrorCode_t eBG96_ActiveContext(void)
{
  bg96_at("AT+QIACT=1");
  return BG96_SUCCESS;
}

/**@brief Set the searching sequence of RATs
 * @param p_pchSearchSeq      numbers corresponding to searching sequence of RATs
 * @retval BG96_SUCCESS
 * @retval BG96_ERROR_FAILED
 * @retval BG96_ERROR_PARAM
 */
eBG96ErrorCode_t eBG96_SetRATSearchSeq(char * p_pchSearchSeq)
{
  eBG96ErrorCode_t l_eCode = BG96_ERROR_PARAM;
  char l_achCmd[LTE_CMD_LEN] = {0};

  if (p_pchSearchSeq != NULL)
  {
    snprintf(l_achCmd, LTE_CMD_LEN, "AT+QCFG=\"nwscanseq\",%s,1", p_pchSearchSeq);
    l_eCode = eBG96_SendCommand(l_achCmd, GSM_CMD_RSP_OK_RF, CMD_TIMEOUT);
  }

  return l_eCode;
}

/****************************************************************************************
   Private functions
 ****************************************************************************************/

/****************************************************************************************
   End Of File
 ****************************************************************************************/
