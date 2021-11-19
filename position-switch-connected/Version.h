/* 
 *  ____  _  _   __   ____   __    ___    ____  _  _  ____  ____  ____  _  _  ____
 * (  __)( \/ ) /  \ (_  _) (  )  / __)  / ___)( \/ )/ ___)(_  _)(  __)( \/ )/ ___)
 *  ) _)  )  ( (  O )  )(    )(  ( (__   \___ \ )  / \___ \  )(   ) _) / \/ \\___ \
 * (____)(_/\_) \__/  (__)  (__)  \___)  (____/(__/  (____/ (__) (____)\_)(_/(____/
 *
 * Copyright (c) 2021 EXOTIC SYSTEMS. All Rights Reserved.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */
#ifndef VERSION_H
#define VERSION_H

/************************************************************************
 * Include Files
 ************************************************************************/
 
/************************************************************************
 * Defines
 ************************************************************************/
#define BUILD_DATE   "0000-00-00"
#define BUILD_TIME   "00:00:00"

#define FW_VERSION   "1.2.1"
#define FW_VERSION_MAJOR	    (0x1)
#define FW_VERSION_MINOR	    (0x2)
#define FW_VERSION_REVISION   (0x1)

#define HW_VERSION              "0.A"
#define HW_VERSION_PCB          (0)
#define HW_VERSION_BOM          ('A')

#ifdef ITSDK_USER_VERSION
#undef ITSDK_USER_VERSION
#endif

#ifdef ITSDK_USER_VERSION_BYTE
#undef ITSDK_USER_VERSION_BYTE
#endif

#define ITSDK_USER_VERSION				FW_VERSION_MAJOR
#define ITSDK_USER_VERSION_BYTE		(FW_VERSION_MAJOR << 4) | FW_VERSION_MINOR

/* COMMIT HASH */
#define COMMIT_NUMBER    "0000000"

/************************************************************************
 * Type definitions
 ************************************************************************/
 
/************************************************************************
 * Public function declarations
 ************************************************************************/

 
#endif /* VERSION_H */
