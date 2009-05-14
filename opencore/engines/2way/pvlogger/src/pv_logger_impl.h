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
#ifndef PV_2WAY_INTERFACE_H_INCLUDED
#include "pv_2way_interface.h"
#endif

#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif

#ifndef OSCL_MEM_AUDIT_H_INCLUDED
#include "oscl_mem_audit.h"
#endif

#ifndef OSCL_ERROR_H_INCLUDED
#include "oscl_error.h"
#endif

#ifndef OSCL_SCHEDULER_H_INCLUDED
#include "oscl_scheduler.h"
#endif

#ifndef OSCL_UTF8CONV_H
#include "oscl_utf8conv.h"
#endif

#include "oscl_file_io.h"

#ifndef FILENAME_LEN
#define FILENAME_LEN 255
#endif

#define MAX_BUFFER_SIZE 1024

// logger config file name
#ifndef CONFIG_FILE_NAME
#define CONFIG_FILE_NAME _STRLIT("pvlogger.ini")
#endif

// logger config file name
#ifndef ENABLE_ALL_LOGS
#define ENABLE_ALL_LOGS "ALL"
#endif

/*
#ifndef DEFAULTSOURCEFORMATTYPE
#error // The format type for default source file needs to be defined in config file
#endif
*/
#ifndef OSCL_STRING_UTILS_H_INCLUDED
#include "oscl_string_utils.h"
#endif

class PVLoggerConfigFile
{
        /*  To change the logging settings without the need to compile the test application
        	Let us read the logging settings from the file instead of hard coding them over here
        	The name of the config file is pvlogger.ini
        	The format of entries in it is like
        	First entry will decide if the file appender has to be used or error appender will be used.
        	0 -> ErrAppender will be used
        	1 -> File Appender will be used

            Entries after this will decide the module whose logging has to be taken.For example, contents of one sample config file could be
        	1
        	1,2wayEngine
            8,3g324m.h245user
        	(pls note that no space is allowed between loglevel and logger tag)
        	This means, we intend to have logging of level 1 for the module 2wayEngine
        	and of level 8 for the 3g324m h245user on file.
        */
    public:

        typedef enum
        {
            EConsoleLog,
            EFileLog
        } LoggingType;

        typedef enum
        {
            ESuccess,
            EError
        } LoggerResult;

        PVLoggerConfigFile(): iLogFileRead(false)
        {
            iFileServer.Connect();
            iAppenderType = 0;
            for (int i = 0; i < MAX_BUFFER_SIZE; i++)
            {
                ibuffer[i] = '0';
            }
        }

        ~PVLoggerConfigFile();

        void SetAppenderType(int aAppenderType);
        bool IsLoggerConfigFilePresent();

        //Set logger config file path
        void SetConfigFilePath(const oscl_wchar *aConfigFilepath);
        //Read and parse the config file
        //retval = -1 if the config file doesnt exist
        int8 ReadAndParseLoggerConfigFile();
        //set logger settings
        uint32 SetLoggerSettings(CPV2WayInterface *aTerminal, const oscl_wchar * aLogPath);
    private:
        class LoggerConfigElement
        {
            public:
                LoggerConfigElement()
                {
                    iLoggerString = NULL;
                    iLogLevel = 8;
                }
                char *iLoggerString;
                int8 iLogLevel;
        };
        int8 iAppenderType; //Type of appender to be used for the logging 0-> Err Appender, 1-> File Appender
        bool iLogFileRead;
        Oscl_File iLogFile;
        Oscl_FileServer iFileServer;
        oscl_wchar iLogFileName[FILENAME_LEN]; // logger config file name
        char ibuffer[MAX_BUFFER_SIZE];
        Oscl_Vector<LoggerConfigElement, OsclMemAllocator> iLoggerConfigElements;
        template<class DestructClass>
        class AppenderDestructDealloc : public OsclDestructDealloc
        {
            public:
                virtual void destruct_and_dealloc(OsclAny *ptr)
                {
                    delete((DestructClass*)ptr);
                }
        };
};
