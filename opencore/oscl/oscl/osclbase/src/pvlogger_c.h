/* ------------------------------------------------------------------
 * Copyright (C) 1998-2009 PacketVideo
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 * -------------------------------------------------------------------
 */

/*! \file pvlogger_c.h
    \brief This file contains basic logger interfaces for common use across platforms.
	   C-callable version

    This is the main entry point header file for the logger library.  It should be
    the only one users directly include.
*/

#ifndef PVLOGGER_C_H_INCLUDED
#define PVLOGGER_C_H_INCLUDED

#include "osclconfig.h"

#ifndef OSCL_IMPORT_REF
#define OSCL_IMPORT_REF
#endif

#ifdef __cplusplus
extern "C"
{
#endif

//C-callable logging routines.
    OSCL_IMPORT_REF void* pvLogger_GetLoggerObject(const char* tag);
    OSCL_IMPORT_REF int pvLogger_IsActive(void* logger, int log_level);
    OSCL_IMPORT_REF void pvLogger_LogMsgString(void* logger, int msgID, const char * fmt, ...);

#ifdef __cplusplus
}
#endif


//Logging instrumentation level default.  To change this for a project, add a definition of
//PVLOGGER_C_INST_LEVEL to the osclconfig.h file.  This default sets level to none for release
//mode, full logging for debug build.

#ifndef PVLOGGER_C_INST_LEVEL
#if defined(NDEBUG)
#define PVLOGGER_C_INST_LEVEL 0
#else
#define PVLOGGER_C_INST_LEVEL 5
#endif
#endif

//Instrumentation levels.
#define PVLOGMSG_C_INST_REL   0
#define PVLOGMSG_C_INST_PROF  1
#define PVLOGMSG_C_INST_HLDBG 2
#define PVLOGMSG_C_INST_MLDBG 3
#define PVLOGMSG_C_INST_LLDBG 4

//Logging levels
#define PVLOGMSG_C_EMERG 0
#define PVLOGMSG_C_ALERT 1
#define PVLOGMSG_C_CRIT 2
#define PVLOGMSG_C_ERR 3
#define PVLOGMSG_C_WARNING 4
#define PVLOGMSG_C_NOTICE 5
#define PVLOGMSG_C_INFO 6
#define PVLOGMSG_C_STACK_TRACE 7
#define PVLOGMSG_C_STACK_DEBUG 8

/*
//Example Usage:

#if (PVLOGGER_C_INST_LEVEL > PVLOGMSG_C_INST_LLDBG)
			if(pvLogger_IsActive(logger ,PVLOGMSG_C_ERR))
				pvLogger_LogMsgString( logger ,  0 ,"Some message, value %d", intvalue );

#endif
*/



#endif // PVLOGGER_C_H_INCLUDED
