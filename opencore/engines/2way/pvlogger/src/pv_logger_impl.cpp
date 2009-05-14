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
#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif

#ifndef PVLOGGER_STDERR_APPENDER_H_INCLUDED
#include "pvlogger_stderr_appender.h"
#endif

#ifndef PVLOGGER_TIME_AND_ID_LAYOUT_H_INCLUDED
#include "pvlogger_time_and_id_layout.h"
#endif

#ifndef PVLOGGER_FILE_APPENDER_H_INCLUDED
#include "pvlogger_file_appender.h"
#endif

#ifndef PV_LOGGER_IMPL_H_INCLUDED
#include "pv_logger_impl.h"
#endif

PVLoggerConfigFile::~PVLoggerConfigFile()
{
    iFileServer.Close();
}

bool PVLoggerConfigFile::IsLoggerConfigFilePresent()
{
    if (ESuccess == ReadAndParseLoggerConfigFile())
    {
        // success
        return true;
    }
    // failure
    return false;
}
void PVLoggerConfigFile::SetConfigFilePath(const oscl_wchar *aConfigFilepath)
{
    oscl_strncpy(iLogFileName, aConfigFilepath, FILENAME_LEN);
    //set log file name
    oscl_strcat(iLogFileName, CONFIG_FILE_NAME);
}

//Read and parse the config file
//retval = -1 if the config file doesnt exist
int8  PVLoggerConfigFile::ReadAndParseLoggerConfigFile()
{
    int8 retval = ESuccess;

    if (ESuccess != iLogFile.Open(iLogFileName, Oscl_File::MODE_READ, iFileServer))
    {
        // error occured in opening logger config file
        retval = EError;
    }
    else
    {
        // able to open logger config file
        if (!iLogFileRead)
        {
            iLogFile.Read(ibuffer, 1, sizeof(ibuffer));
            //Parse the buffer for \n chars
            Oscl_Vector<char*, OsclMemAllocator> LogConfigStrings;

            //Get the logger strings
#if defined(__linux__) || defined(linux)
            const char* const lnFd = "\n";
#endif

            const int8 lnFdLen = oscl_strlen(lnFd);
            int16 offset = 0;
            char* lastValidBffrAddr = ibuffer + oscl_strlen(ibuffer);
            const char* lnFdIndx = oscl_strstr(ibuffer, lnFd);
            while (lnFdIndx != NULL && lnFdIndx < lastValidBffrAddr)
            {
                oscl_memset((char*)lnFdIndx, '\0', lnFdLen);
                LogConfigStrings.push_back(ibuffer + offset);
                offset = (lnFdIndx + lnFdLen) - ibuffer;
                lnFdIndx = OSCL_CONST_CAST(char*, oscl_strstr(ibuffer + offset, lnFd));
            }
            if (NULL == lnFdIndx && ((ibuffer + offset) < lastValidBffrAddr)) //If \r\n is skipped after the last logging str in ini file
            {
                LogConfigStrings.push_back(ibuffer + offset);
            }


            //Populate the  LoggerConfigElements vector
            {
                if (!LogConfigStrings.empty())
                {
                    Oscl_Vector<char*, OsclMemAllocator>::iterator it;
                    it = LogConfigStrings.begin();
                    uint32 appenderType;
                    PV_atoi(*it, 'd', oscl_strlen(*it), appenderType);
                    iAppenderType = appenderType;
                    if (LogConfigStrings.size() > 1)
                    {
                        for (it = LogConfigStrings.begin() + 1; it != LogConfigStrings.end(); it++)
                        {
                            char* CommaIndex = OSCL_CONST_CAST(char*, oscl_strstr(*it, ","));
                            if (CommaIndex != NULL)
                            {
                                *CommaIndex = '\0';
                                LoggerConfigElement obj;
                                uint32 logLevel;
                                PV_atoi(*it, 'd', oscl_strlen(*it), logLevel);
                                obj.iLogLevel = logLevel;
                                obj.iLoggerString = CommaIndex + 1;
                                iLoggerConfigElements.push_back(obj);
                            }
                        }
                    }
                    else
                    {
                        //Add the config element for complete logging fo all the modules
                        LoggerConfigElement obj;
                        obj.iLoggerString = NULL;
                        obj.iLogLevel = 8;
                        iLoggerConfigElements.push_back(obj);
                    }
                }
            }
            iLogFile.Close();
            iLogFileRead = true;
        }
    }
    return retval;
}

void PVLoggerConfigFile::SetAppenderType(int aAppenderType)
{
    iAppenderType = aAppenderType;
}

uint32 PVLoggerConfigFile::SetLoggerSettings(CPV2WayInterface *aTerminal, const oscl_wchar *aLogPath)
{
    Oscl_Vector<LoggerConfigElement, OsclMemAllocator>::iterator it;
    uint32 error = 1;

    PVLoggerAppender *appender = NULL;
    OsclRefCounter *refCounter = NULL;
    if (iLoggerConfigElements.empty())
    {
        return error;
    }

    if (EConsoleLog == iAppenderType)
    {
        // for console
        appender = new StdErrAppender<TimeAndIdLayout, 1024>();
        OsclRefCounterSA<AppenderDestructDealloc<StdErrAppender<TimeAndIdLayout, 1024> > > *appenderRefCounter =
            new OsclRefCounterSA<AppenderDestructDealloc<StdErrAppender<TimeAndIdLayout, 1024> > >(appender);
        refCounter = appenderRefCounter;
    }
    else if (EFileLog == iAppenderType)
    {
        //for file
        OSCL_wHeapString<OsclMemAllocator> logfilename(aLogPath);
        appender = (PVLoggerAppender*)TextFileAppender<TimeAndIdLayout, 1024>::CreateAppender(logfilename.get_str());
        OsclRefCounterSA<AppenderDestructDealloc<TextFileAppender<TimeAndIdLayout, 1024> > > *appenderRefCounter =
            new OsclRefCounterSA<AppenderDestructDealloc<TextFileAppender<TimeAndIdLayout, 1024> > >(appender);
        refCounter = appenderRefCounter;
    }
    else
    {
        // No Valid Logger Appender
        return error;
    }

    OsclSharedPtr<PVLoggerAppender> appenderPtr(appender, refCounter);

    for (it = iLoggerConfigElements.begin(); it != iLoggerConfigElements.end(); it++)
    {
        char* loggerString = OSCL_STATIC_CAST(char*, "");
        if (0 != oscl_strncmp(it->iLoggerString, ENABLE_ALL_LOGS, oscl_strlen(ENABLE_ALL_LOGS)))
        {
            loggerString = it->iLoggerString;
        }
        if (NULL == aTerminal)
        {
            PVLogger *node = NULL;

            node = PVLogger::GetLoggerObject(loggerString);
            if (NULL == node)
            {
                error = 1;
                return error;
            }
            node->AddAppender(appenderPtr);
            node->SetLogLevel(it->iLogLevel);
        }
        else
        {
            aTerminal->SetLogLevel("", it->iLogLevel, true);
            aTerminal->SetLogAppender(loggerString, appenderPtr);
            error = 0;
        }

        if (0 != error)
        {
            return error;
        }
    }

    return error;
}

