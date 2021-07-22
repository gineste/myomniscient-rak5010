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
 * str_utils.c
 * Date:          21 avr. 2020
 * Author:        aurel
 * Project:       RAK5010 connected switch
 * Description:   str utils
 *
 */

/****************************************************************************************
 * Include Files
 ****************************************************************************************/ 
#include <stdio.h>
#include <string.h>
#include "str_utils.h"

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
/**
 * @brief Convert a character 0-9,A-F in byte
 * @param p_ch
 * @return unsigned byte
 */
uint8_t u8SU_CharToByte(char p_ch)
{
	if((p_ch >= '0') && (p_ch <= '9'))
		return p_ch - '0';

	if((p_ch >= 'A') && (p_ch <= 'F'))
		return p_ch - 'A' + 10u;

	return 0xFFu;
}

/**
 * @brief Convert a byte to a two characters 0-9,A-F
 * @param p_u8Byte
 * @param p_pchOut
 * @return size of str
 */
uint8_t u8SU_ByteToAsciiHex(uint8_t p_u8Byte, char p_pchOut[])
{
	p_pchOut[0] = (p_u8Byte >> 4) & 0x0Fu;
	if(p_pchOut[0] > 9u)
	{
		p_pchOut[0] += 'A' - 10u;
	}else{
		p_pchOut[0] += '0';
	}

	p_pchOut[1] = (p_u8Byte >> 0) & 0x0Fu;
	if(p_pchOut[1] > 9u)
	{
		p_pchOut[1] += 'A' - 10u;
	}else{
		p_pchOut[1] += '0';
	}

	return 2u;
}

/**
 * @brief Convert a two characters 0-9,A-F in byte
 * @param p_chMsb
 * @param p_chLsb
 * @return uint8_t
 */
uint8_t u8SU_AsciiHexToByte(char p_pchOut[])
{
	uint8_t l_u8Value = 0u;

	l_u8Value = u8SU_CharToByte(p_pchOut[0]);
	l_u8Value <<= 4;
	l_u8Value += u8SU_CharToByte(p_pchOut[1]);

	return l_u8Value;
}

/**
 * @brief Convert an uint32_t in string
 * @param p_u32
 * @param p_pchOutput
 * @return size of string
 */
uint8_t u8SU_U32ToAscii(uint32_t p_u32, char p_pchOutput[])
{
	char l_achTmp[32] = {0};
	uint8_t l_u8ConvertedSize = 0u;
	uint8_t l_u8TmpIdx = 0u;
	uint8_t l_u8TmpValue = 0u;

	if(p_pchOutput != NULL)
	{
		if(p_u32 > 0u)
		{
			while(p_u32 != 0u)
			{
				l_u8TmpValue = p_u32 % 10;
				if((l_u8TmpValue >= 0) && (l_u8TmpValue <= 9))
				{
					l_achTmp[l_u8TmpIdx] = l_u8TmpValue + '0';
					++l_u8TmpIdx;
				}else{
					break;
				}

				p_u32 -= l_u8TmpValue;
				p_u32 /= 10u;
			}

			l_u8ConvertedSize = l_u8TmpIdx;

			for(l_u8TmpIdx = 0u; l_u8TmpIdx < l_u8ConvertedSize; l_u8TmpIdx++)
			{
				p_pchOutput[l_u8TmpIdx] = l_achTmp[l_u8ConvertedSize - l_u8TmpIdx - 1];
			}
			p_pchOutput[l_u8TmpIdx] = '\0';
		}else{
			p_pchOutput[0] = '0';
			p_pchOutput[1] = '\0';
		}
	}

	return l_u8ConvertedSize;
}


/**
 * @brief Convert a string number in int32_t
 * @param p_pchStr
 * @return int32_t
 */
int32_t s32SU_AsciiToS32(char p_pchStr[])
{
	uint32_t l_u32StrLen = 0u;
	uint32_t l_u32StrIdx = 0u;

	int32_t l_s32Value = 0;
	int8_t l_s8Sign = 1;

	if(p_pchStr != NULL)
	{
		l_u32StrLen = strlen(p_pchStr);
		l_u32StrIdx = 0u;

		while(l_u32StrIdx < l_u32StrLen)
		{
			if((p_pchStr[l_u32StrIdx] >= '0') && (p_pchStr[l_u32StrIdx] <= '9'))
			{
				l_s32Value *= 10;
				l_s32Value += p_pchStr[l_u32StrIdx] - '0';
			}else if(p_pchStr[l_u32StrIdx] == '-')
			{
				l_s8Sign = -1;
			}else{
				break;
			}

			++l_u32StrIdx;
		}
	}

	return l_s32Value * l_s8Sign;
}

/**
 * @brief Convert a hex ascii string in bytes array
 * @param p_u32Value			Value to convert
 * @param p_pchOutStr			AsciiHex string
 * @retval 8u	converted
 * @retval 0u	wrong param
 */
uint32_t u32SU_U32ToHexAscii(uint32_t p_u32Value, char p_pchOutStr[])
{
	if(p_pchOutStr != NULL)
	{
		u8SU_ByteToAsciiHex((uint8_t) ((p_u32Value >> 24) & 0xFFu), &(p_pchOutStr[0]));
		u8SU_ByteToAsciiHex((uint8_t) ((p_u32Value >> 16) & 0xFFu), &(p_pchOutStr[2]));
		u8SU_ByteToAsciiHex((uint8_t) ((p_u32Value >> 8 ) & 0xFFu), &(p_pchOutStr[4]));
		u8SU_ByteToAsciiHex((uint8_t) ((p_u32Value >> 0 ) & 0xFFu), &(p_pchOutStr[6]));

		return 8u;
	}else{
		return 0u;
	}
}

uint32_t u32SU_HexAsciiToU32(char p_pchHexStr[], uint16_t p_u16HexStrLen)
{
	uint32_t l_u32StrIdx = 0u;
	uint32_t l_u32Value = 0u;

	for(l_u32StrIdx = 0u; l_u32StrIdx < p_u16HexStrLen; l_u32StrIdx += 2u)
	{
		l_u32Value = ((uint32_t) (l_u32Value << 8u)) & 0xFFFFFFFFu;
		l_u32Value += u8SU_AsciiHexToByte(&(p_pchHexStr[l_u32StrIdx]));
	}

	return l_u32Value;
}


/**
 * @brief Convert a hex ascii string in bytes array
 * @param p_pchHexStr			Hex ascii string
 * @param p_u16HexStrLen		Hex ascii string length
 * @param p_au8Bytes			Output bytes array
 * @param p_u16BytesLenMax		Output bytes array max size
 * @retval 0 wrong function parameters
 * @retval > 0 number of bytes decoded
 */
uint16_t u16SU_AsciiHexStrToBytes(char p_pchHexStr[], uint16_t p_u16HexStrLen, uint8_t p_au8Bytes[], uint16_t p_u16BytesLenMax)
{
	uint16_t l_u16StrIdx = 0u;
	uint16_t l_u16HexBinaryLineLen = 0u;

	if((p_pchHexStr != NULL) && (p_u16HexStrLen > 1u) && (p_au8Bytes != NULL) && (p_u16BytesLenMax >= (p_u16HexStrLen / 2u)))
	{
		while(((p_u16HexStrLen - l_u16StrIdx) > 1u) && (l_u16HexBinaryLineLen < p_u16BytesLenMax))
		{
			p_au8Bytes[l_u16HexBinaryLineLen] = u8SU_AsciiHexToByte(&(p_pchHexStr[l_u16StrIdx])); /* use n and n+1 */
			l_u16HexBinaryLineLen++;
			l_u16StrIdx += 2u;
		}
	}

	return l_u16HexBinaryLineLen;
}

/**
 * @brief Convert a bytes array in hex ascii string
 * @param p_au8Bytes			bytes array
 * @param p_u16BytesLen			bytes array size
 * @param p_pchHexStr			Hex ascii string
 * @param p_u16HexStrLenMax		Hex ascii string max length
 * @retval 0 wrong function parameters
 * @retval > 0 number of bytes used
 */
uint16_t u16SU_BytesToAsciiHexStr(uint8_t p_au8Bytes[], uint16_t p_u16BytesLen, char p_pchHexStr[], uint16_t p_u16HexStrLenMax)
{
	uint16_t l_u16ByteIdx = 0u;
	uint16_t l_u16StrSize = 0u;

	if((p_pchHexStr != NULL) && (p_u16BytesLen > 0u) && (p_au8Bytes != NULL) && (p_u16HexStrLenMax >= (p_u16BytesLen * 2u)))
	{
		for(l_u16ByteIdx = 0u; l_u16ByteIdx < p_u16BytesLen; l_u16ByteIdx++)
		{
			l_u16StrSize += u8SU_ByteToAsciiHex(p_au8Bytes[l_u16ByteIdx], &(p_pchHexStr[l_u16ByteIdx * 2u]));
		}
	}
	p_pchHexStr[l_u16StrSize] = '\0';
	return l_u16StrSize;
}

/**
 * @brief Convert two bytes in uint16_t
 * @param p_u8Msb
 * @param p_u8Lsb
 * @return uint16_t
 */
uint16_t u16SU_BytesToU16(uint8_t p_u8Msb, uint8_t p_u8Lsb)
{
	return (uint16_t) (((uint16_t) p_u8Msb << 8u) + (uint16_t) p_u8Lsb);
}

/**
 * @brief Convert four bytes in uint32_t
 * @param p_u8XMsb
 * @param p_u8Msb
 * @param p_u8Lsb
 * @param p_u8XLsb
 * @return uint32_t
 */
uint32_t u32SU_BytesToU32(uint8_t p_u8XMsb, uint8_t p_u8Msb, uint8_t p_u8Lsb, uint8_t p_u8XLsb)
{
	return (uint32_t) (((uint32_t)p_u8XMsb << 24u) + ((uint32_t)p_u8Msb << 16u) + ((uint32_t)p_u8Lsb << 8u) + (uint32_t)p_u8XLsb);
}
/**
 * @brief Convert a float number to string
 * @param p_f32Value
 * @param p_u8Decimal
 * @param p_achFloatStr
 * @return size of string
 */
uint32_t u32SU_FloatToAscii(float p_f32Value, uint8_t p_u8Decimal, char p_achFloatStr[])
{
	uint32_t l_u32EntPart = 0u;
	uint32_t l_u32DecPart = 0u;
	float l_f32DecPart = 0.0f;
	uint8_t l_u8Sign = 0u;

	if(p_f32Value < 0.0f)
	{
		l_u8Sign = 1u;
		p_f32Value = -p_f32Value;
	}

	l_u32EntPart = (uint32_t) p_f32Value;
	l_f32DecPart = (p_f32Value - l_u32EntPart);
	while(p_u8Decimal > 0)
	{
		l_f32DecPart *= 10.0f;
		p_u8Decimal--;
	}
	l_u32DecPart = (uint32_t) l_f32DecPart;

	if(l_u8Sign == 1u)
	{
		return sprintf(p_achFloatStr, "-%u.%u", (unsigned int)l_u32EntPart, (unsigned int)l_u32DecPart);
	}else{
		return sprintf(p_achFloatStr, "%u.%u", (unsigned int)l_u32EntPart, (unsigned int)l_u32DecPart);
	}
}

/**
 * @brief Convert a string number in float
 * @param p_pchStr
 * @return float value
 */
float f32SU_AsciiToFloat(char p_pchStr[])
{
	uint32_t l_u32StrLen = 0u;
	uint32_t l_u32StrIdx = 0u;

	int32_t l_s32ValueTmp = 0;
	float l_f32Value = 0;
	int8_t l_s8Sign = 1;
	uint8_t l_u8Decimal = 0u;
	uint32_t l_u32Factor = 1u;

	if(p_pchStr != NULL)
	{
		l_u32StrLen = strlen(p_pchStr);
		l_u32StrIdx = 0u;

		while(l_u32StrIdx < l_u32StrLen)
		{
			if((p_pchStr[l_u32StrIdx] >= '0') && (p_pchStr[l_u32StrIdx] <= '9'))
			{
				l_u32Factor *= 10u;
				l_s32ValueTmp *= 10;
				l_s32ValueTmp += p_pchStr[l_u32StrIdx] - '0';
			}else if((p_pchStr[l_u32StrIdx] == '.') && (l_u8Decimal == 0u))
			{
				l_f32Value = l_s32ValueTmp;

				l_u8Decimal = 1u;
				l_u32Factor = 1u;
				l_s32ValueTmp = 0;
			}else if(p_pchStr[l_u32StrIdx] == '-')
			{
				l_s8Sign = -1;
			}else{
				break;
			}

			++l_u32StrIdx;
		}
	}

	if(l_u8Decimal == 1u)
	{
		l_f32Value += (float)l_s32ValueTmp / l_u32Factor;
	}else{
		l_f32Value = l_s32ValueTmp;
	}
	l_f32Value *= l_s8Sign;

	return l_f32Value;
}

/**
 * @brief Search substrin in a string
 * @param p_pchString
 * @param p_pchSubString
 * @retval NULL if not foud
 * @retval > 0 pointer on start of substring in the string
 */
char * pchSU_SearchString(char * p_pchString, const char * p_pchSubString)
{
	return strstr(p_pchString, p_pchSubString);
}

/**
 * @brief Split string in substring with a separator
 * @param p_pchStr
 * @param p_chSeprator
 * @param p_apchArgc
 * @param p_u16ArgcMax
 * @retval 0 wrong function parameters
 * @retval 1 no separator found
 * @retval > 0 number of substring found
 */
uint16_t u16SU_GetParamsFromString(char p_pchStr[], char p_chSeprator, char *p_apchArgc[], uint16_t p_u16ArgcMax)
{
	uint32_t l_u32StrLen = 0u;
	uint32_t l_u32StrIdx = 0u;
	uint16_t l_u16Argv = 0u;
	uint8_t l_u8IgnoreSeparator = 0u;

	if((p_pchStr != NULL) && (p_apchArgc != NULL) && (p_u16ArgcMax > 0))
	{
		l_u32StrLen = strlen(p_pchStr);
		l_u32StrIdx = 0u;
		l_u16Argv = 0u;

		/* Start of p_pchStr is a substring */
		p_apchArgc[l_u16Argv] = &(p_pchStr[l_u32StrIdx]);
		++l_u16Argv;

		while((l_u32StrIdx < l_u32StrLen) && (l_u16Argv < p_u16ArgcMax))
		{
			/* Ignore separator if it is between two \" */
			if(p_pchStr[l_u32StrIdx] == '\"')
			{
				l_u8IgnoreSeparator = !l_u8IgnoreSeparator;
			}

			if((l_u8IgnoreSeparator == 0u) && ((p_pchStr[l_u32StrIdx] == p_chSeprator) || (p_pchStr[l_u32StrIdx] == '\r')))
			{
				p_pchStr[l_u32StrIdx] = '\0';
				p_apchArgc[l_u16Argv] = &(p_pchStr[l_u32StrIdx + 1u]);
				++l_u16Argv;
			}

			++l_u32StrIdx;
		}
	}

	return l_u16Argv;
}

/**
 * @brief Return a substring present between two delimiters
 * @param p_pchStr					Source string
 * @param p_chStartDelimiter		Start delimiter of substring
 * @param p_chEndDelimiter			End delimiter of substring
 * @param p_pchSubString			Substring extracted
 * @param p_u8Size					Size of the substring
 * @retval 0 wrong function parameters or no separator found
 * @retval 1 substring existing
 */
uint8_t u8SU_GetStringBetweenDelimiters(char * p_pchString, char * p_pchStartDelimiter, char * p_pchEndDelimiter, char * p_pchSubString, uint8_t p_u8Size)
{
	uint8_t 	l_u8Code = 0u;
	char 		l_achTmpString[256];	/* Copy of string to avoid modify the original one */
	char * 	l_pchSubString;

	if ((p_pchString != NULL) && (p_pchStartDelimiter != NULL) && (p_pchEndDelimiter != NULL) && (p_pchSubString != NULL))
	{
		strcpy(l_achTmpString, p_pchString);

		l_pchSubString = strtok(l_achTmpString,p_pchStartDelimiter);
		l_pchSubString = strtok(NULL,p_pchEndDelimiter);

		if (l_pchSubString != NULL)
		{
			l_u8Code = 1u;
			strncpy(p_pchSubString,l_pchSubString,p_u8Size);
		}
	}

	return l_u8Code;
}



#ifdef STR_UNIT_TEST

void vStrUtilsTest(void)
{
	char l_pchParamStr[] = "abc,0,\"d,e\",569.87,abcdef";
	char l_pchHexStr[] = "A00AA1B2C3D4E5F70809";
	uint8_t l_au8Bytes[16] = {0};
	char l_achStr[16] = {0};

	char *l_pchParams[16] = {0};

	if(9 != u8SU_CharToByte('9'))
		while(1);

	u8SU_ByteToAsciiHex(0xCDu, l_achStr);
	if(0xCDu != u8SU_AsciiHexToByte(l_achStr))
		while(1);

	u8SU_U32ToAscii(0xABCDEF01u, l_achStr);
	if(0xABCDEF01u != s32SU_AsciiToS32(l_achStr))
		while(1);

	u32SU_U32ToHexAscii(0xABCDEF01u, l_achStr);
	if(0xABCDEF01u != u32SU_HexAsciiToU32(l_achStr, 8u))
		while(1);

	if(10u != u16SU_AsciiHexStrToBytes(l_pchHexStr, strlen(l_pchHexStr), l_au8Bytes, 16u))
		while(1);

	if(0xABCDu != u16SU_BytesToU16(0xABu, 0xCDu))
		while(1);

	if(0xABCD1234u != u32SU_BytesToU32(0xABu, 0xCDu, 0x12u, 0x34u))
		while(1);

	/* cut to 4 decimal to test */
	u32SU_FloatToAscii(3.14157f, 4u, l_achStr);
	if(3.1415f != f32SU_AsciiToFloat(l_achStr))
		while(1);

	if(NULL == pchSU_SearchString(l_pchParamStr, "bcd"))
		while(1);

	if(5u != u16SU_GetParamsFromString(l_pchParamStr, ',', l_pchParams, 16u))
		while(1);




}
#endif

/****************************************************************************************
 * Private functions
 ****************************************************************************************/ 

/****************************************************************************************
 * End Of File
 ****************************************************************************************/
