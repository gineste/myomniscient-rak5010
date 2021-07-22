/*
 *    ____  _  _   __   ____   __    ___    ____  _  _  ____  ____  ____  _  _  ____
 *   (  __)( \/ ) /  \ (_  _) (  )  / __)  / ___)( \/ )/ ___)(_  _)(  __)( \/ )/ ___)
 *    ) _)  )  ( (  O )  )(    )(  ( (__   \___ \ )  / \___ \  )(   ) _) / \/ \\___ \     
 *   (____)(_/\_) \__/  (__)  (__)  \___)  (____/(__/  (____/ (__) (____)\_)(_/(____/
 *
 * Copyright (c) 2020 EXOTIC SYSTEMS. All Rights Reserved.
 *
 * Licensees are granted free, non-transferable use of the information. NO WARRANTY 
 * of ANY KIND is provided. This heading must NOT be removed from the file.
 *
 * str_utils.h
 * Date:          21 avr. 2020
 * Author:        aurel
 * Project:       RAK5010 connected switch
 * Description:   str utils
 *
 */
#ifndef UTILS_STR_UTILS_H_
#define UTILS_STR_UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************************
 * Include Files
 ****************************************************************************************/ 
#include <stdio.h>

/****************************************************************************************
 * Defines
 ****************************************************************************************/ 
/*#define STR_UNIT_TEST*/

/****************************************************************************************
 * Type declarations
 ****************************************************************************************/

/****************************************************************************************
 * Variable declarations
 ****************************************************************************************/  

/****************************************************************************************
 * Public functions
 ****************************************************************************************/  
uint8_t u8SU_CharToByte(char p_ch);

uint8_t u8SU_ByteToAsciiHex(uint8_t p_u8Byte, char p_pchOut[]);
uint8_t u8SU_AsciiHexToByte(char p_pchOut[]);

uint8_t u8SU_U32ToAscii(uint32_t p_u32, char p_pchOutput[]);
int32_t s32SU_AsciiToS32(char p_pchStr[]);

uint32_t u32SU_U32ToHexAscii(uint32_t p_u32Value, char p_pchOutStr[]);
uint32_t u32SU_HexAsciiToU32(char p_pchHexStr[], uint16_t p_u16HexStrLen);

uint16_t u16SU_AsciiHexStrToBytes(char p_pchHexStr[], uint16_t p_u16HexStrLen, uint8_t p_au8Bytes[], uint16_t p_u16BytesLenMax);
uint16_t u16SU_BytesToAsciiHexStr(uint8_t p_au8Bytes[], uint16_t p_u16BytesLen, char p_pchHexStr[], uint16_t p_u16HexStrLenMax);

uint16_t u16SU_BytesToU16(uint8_t p_u8Msb, uint8_t p_u8Lsb);
uint32_t u32SU_BytesToU32(uint8_t p_u8XMsb, uint8_t p_u8Msb, uint8_t p_u8Lsb, uint8_t p_u8XLsb);

uint32_t u32SU_FloatToAscii(float p_f32Value, uint8_t p_u8Decimal, char p_achFloatStr[]);
float f32SU_AsciiToFloat(char p_pchStr[]);

char * pchSU_SearchString(char * p_pchString, const char * p_pchSubString);

uint16_t u16SU_GetParamsFromString(char p_pchStr[], char p_chSeprator, char *p_apchArgc[], uint16_t p_u16ArgcMax);

uint8_t u8SU_GetStringBetweenDelimiters(char * p_pchString, char * p_pchStartDelimiter, char * p_pchEndDelimiter, char * p_pchSubString, uint8_t p_u8Size);



#ifdef STR_UNIT_TEST
void vStrUtilsTest(void);
#else
#define vStrUtilsTest()
#endif

#ifdef __cplusplus
} // extern "C"
#endif

/****************************************************************************************
 * End Of File
 ****************************************************************************************/
#endif /* UTILS_STR_UTILS_H_ */
