/* ------------------------------------------------------------------
 * Copyright (C) 2009 PacketVideo
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

#ifndef ANDROID_LOGGER_CONFIG_H
#define ANDROID_LOGGER_CONFIG_H

#include "author/authordriver.h"
#include "android_log_appender.h"

#define MAX_STRING_LEN  255

FILE *file;

class PVLoggerConfigFile{
/*  
    To configure logging at runtime, a file pvlogger.txt must be located in the sdcard.
    The format for log level and logger tag in the file should be: "level,node".  Note that there should be no space between log level and logger tag.
    For example, pvlogger.txt can look like:
	1,PVPlayerEngine
	8,PVSocketNode
    Above example means log the message level PVLOGMSG_ALERT for PVPlayerEngine and PVLOGMSG_DEBUG for PVSocketNode.  See pvlogger.h for log level values.
*/
public:
	PVLoggerConfigFile():iLogFileRead(false)
	{
		iFileServer.Connect();
        iLogFileName[MAX_STRING_LEN] = '\0';
		oscl_strncpy(iLogFileName,"/sdcard/pvlogger.txt", MAX_STRING_LEN);
		oscl_memset(ibuffer, 0, sizeof(ibuffer));
	}

	~PVLoggerConfigFile()
	{
		iFileServer.Close();
	}

	bool IsLoggerConfigFilePresent()
	{
		if(-1 != ReadAndParseLoggerConfigFile())
			return true;
		return false;
	}

	//Read and parse the config file
	//retval = -1 if the config file doesnt exist
	int8 ReadAndParseLoggerConfigFile()
	{
		int8 retval = 1;

		if(0 != iLogFile.Open(iLogFileName,Oscl_File::MODE_READ, iFileServer))
		{
			retval = -1;
		}
		else
		{
			if(!iLogFileRead)
			{
				int32 nCharRead = iLogFile.Read(ibuffer,1, sizeof(ibuffer));
				//Parse the buffer for \n chars
				Oscl_Vector<char*,OsclMemAllocator> LogConfigStrings;

				//Get the logger strings
				const char* const lnFd = "\n";
				const int8 lnFdLen = oscl_strlen(lnFd);
				int16 offset = 0;
				char* lastValidBffrAddr = ibuffer + oscl_strlen(ibuffer);
				const char* lnFdIndx = oscl_strstr(ibuffer, lnFd);
				while(lnFdIndx!=NULL && lnFdIndx < lastValidBffrAddr)
				{
					oscl_memset((char*)lnFdIndx, '\0', lnFdLen);
					LogConfigStrings.push_back(ibuffer + offset);
					offset = (lnFdIndx + lnFdLen) - ibuffer;
					lnFdIndx = oscl_strstr(ibuffer + offset, lnFd);
				}
				if(NULL == lnFdIndx && ((ibuffer + offset) < lastValidBffrAddr))
				{
					LogConfigStrings.push_back(ibuffer + offset);
				}

				//Populate the  LoggerConfigElements vector
				{
					if(!LogConfigStrings.empty())
					{
                        Oscl_Vector<char*,OsclMemAllocator>::iterator it;
						if(LogConfigStrings.size() > 0)
						{
							for (it = LogConfigStrings.begin(); it!= LogConfigStrings.end(); it++)
							{
                                char* hashIndex = OSCL_CONST_CAST(char*, oscl_strstr(*it, "#"));
                                char* CommaIndex = OSCL_CONST_CAST(char*, oscl_strstr(*it, ","));
                                // Skip lines with hash
                                if (hashIndex == NULL)
                                {
								    if(CommaIndex != NULL)
								    {
									    *CommaIndex = '\0';
									    LoggerConfigElement obj;
									    uint32 logLevel;
									    PV_atoi(*it,'d',oscl_strlen(*it),logLevel);
									    obj.iLogLevel = logLevel;
									    obj.iLoggerString= CommaIndex +1;
									    iLoggerConfigElements.push_back(obj);
								    }
                                    else
					                {
						                //Add the config element for logging all the modules with specified log level
                                        LoggerConfigElement obj;
                                        uint32 logLevel;
									    PV_atoi(*it,'d',oscl_strlen(*it),logLevel);
						                obj.iLoggerString = "";
						                obj.iLogLevel = logLevel;
						                iLoggerConfigElements.push_back(obj);
					                }
                                }
							}
						}
					}
                    else
					{
						//Add the config element for complete logging for all the modules
						LoggerConfigElement obj;
						obj.iLoggerString = "";
						obj.iLogLevel = 8;
						iLoggerConfigElements.push_back(obj);
					}
				}
				iLogFile.Close();
				iLogFileRead = true;
			}
		}
		return retval;
	}

	void SetLoggerSettings()
	{
		Oscl_Vector<LoggerConfigElement, OsclMemAllocator>::iterator it;

		PVLoggerAppender *appender=NULL;
		OsclRefCounter *refCounter=NULL;
		if(iLoggerConfigElements.empty())
		{
			return;
		}
		
        appender = new AndroidLogAppender<TimeAndIdLayout,1024>();
        OsclRefCounterSA<LogAppenderDestructDealloc<AndroidLogAppender<TimeAndIdLayout,1024> > > *appenderRefCounter =
            new OsclRefCounterSA<LogAppenderDestructDealloc<AndroidLogAppender<TimeAndIdLayout,1024> > >(appender);
        refCounter = appenderRefCounter;

		OsclSharedPtr<PVLoggerAppender> appenderPtr(appender,refCounter);

		for (it = iLoggerConfigElements.begin(); it!= iLoggerConfigElements.end(); it++)
		{
			PVLogger *node = NULL;
			node = PVLogger::GetLoggerObject(it->iLoggerString);
			node->AddAppender(appenderPtr);
			node->SetLogLevel(it->iLogLevel);
		}
	}

private:
	class LoggerConfigElement{
		public:
		LoggerConfigElement()
		{
			iLoggerString = NULL;
			iLogLevel = 8;
		}
		char *iLoggerString;
		int8 iLogLevel;
	};
	bool iLogFileRead;
	Oscl_File iLogFile;
	Oscl_FileServer iFileServer;
	char iLogFileName[MAX_STRING_LEN+1];
	char ibuffer[1024];
	Oscl_Vector<LoggerConfigElement, OsclMemAllocator> iLoggerConfigElements;
};

#endif

