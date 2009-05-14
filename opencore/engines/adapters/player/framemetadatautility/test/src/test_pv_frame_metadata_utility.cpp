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

#include "test_pv_frame_metadata_utility.h"

#include "test_pv_frame_metadata_utility_testset1.h"
#if RUN_JANUSCPM_TESTCASES
#include "test_pv_frame_metadata_utility_testset_januscpm.h"
#endif

#include "oscl_mem.h"
#include "oscl_mem_audit.h"
#include "oscl_error.h"
#include "oscl_scheduler.h"
#include "pvlogger.h"
#include "pvlogger_file_appender.h"
#include "pvlogger_mem_appender.h"
#include "unit_test_args.h"
#include "oscl_utf8conv.h"
#include "oscl_string_utils.h"

#include "OMX_Core.h"

#ifndef DEFAULTSOURCEFILENAME
#error // The default source file needs to be defined in config file
#endif

#ifndef DEFAULTSOURCEFORMATTYPE
#error // The format type for default source file needs to be defined in config file
#endif

FILE* file;

#define MAX_LEN 100

class PVLoggerConfigFile
{
        /*  To change the logging settings without the need to compile the test application
        	Let us read the logging settings from the file instead of hard coding them over here
        	The name of the config file is pvlogger.ini
        	The format of entries in it is like
        	First entry will decide if the file appender has to be used or error appender will be used.
        	0 -> ErrAppender will be used
        	1 -> File Appender will be used
        	2 -> Mem Appender will be used
        	Entries after this will decide the module whose logging has to be taken.For example, contents of one sample config file could be
        	1
        	1,PVPlayerEngine
        	8,PVSocketNode
        	(pls note that no space is allowed between loglevel and logger tag)
        	This means, we intend to have logging of level 1 for the module PVPlayerEngine
        	and of level 8 for the PVSocketNode on file.
        */
    public:

        PVLoggerConfigFile(): iLogFileRead(false)
        {
            iFileServer.Connect();
            // Full path of pvlogger.ini is: SOURCENAME_PREPEND_STRING + pvlogger.ini
            oscl_strncpy(iLogFileName, SOURCENAME_PREPEND_STRING,
                         oscl_strlen(SOURCENAME_PREPEND_STRING) + 1);
            oscl_strcat(iLogFileName, "pvlogger.ini");
            oscl_memset(ibuffer, 0, sizeof(ibuffer));
            iAppenderType = 0;

        }

        ~PVLoggerConfigFile()
        {
            iFileServer.Close();
        }

        bool get_next_line(const char *start_ptr, const char * end_ptr,
                           const char *& line_start,
                           const char *& line_end)
        {
            // Finds the boundaries of the next non-empty line within start
            // and end ptrs

            // This initializes line_start to the first non-whitespace character
            line_start = skip_whitespace_and_line_term(start_ptr, end_ptr);

            line_end = skip_to_line_term(line_start, end_ptr);

            return (line_start < end_ptr);

        }


        bool IsLoggerConfigFilePresent()
        {
            if (-1 != ReadAndParseLoggerConfigFile())
                return true;
            return false;
        }

        //Read and parse the config file
        //retval = -1 if the config file doesnt exist
        int8 ReadAndParseLoggerConfigFile()
        {
            int8 retval = 1;

            if (0 != iLogFile.Open(iLogFileName, Oscl_File::MODE_READ, iFileServer))
            {
                retval = -1;
            }
            else
            {
                if (!iLogFileRead)
                {
                    int32 nCharRead = iLogFile.Read(ibuffer, 1, sizeof(ibuffer));
                    //Parse the buffer for \n chars
                    Oscl_Vector<char*, OsclMemAllocator> LogConfigStrings;

                    const char *end_ptr = ibuffer + oscl_strlen(ibuffer) ; // Point just beyond the end
                    const char *section_start_ptr;
                    const char *line_start_ptr, *line_end_ptr;
                    char* end_temp_ptr;
                    int16 offset = 0;

                    section_start_ptr = skip_whitespace_and_line_term(ibuffer, end_ptr);

                    while (section_start_ptr < end_ptr)
                    {
                        if (!get_next_line(section_start_ptr, end_ptr,
                                           line_start_ptr, line_end_ptr))
                        {
                            break;
                        }


                        section_start_ptr = line_end_ptr + 1;

                        end_temp_ptr = (char*)line_end_ptr;
                        *end_temp_ptr = '\0';

                        LogConfigStrings.push_back((char*)line_start_ptr);

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
                                    char* CommaIndex = (char*)oscl_strstr(*it, ",");
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
                                obj.iLoggerString = "";
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

        void SetLoggerSettings()
        {
            Oscl_Vector<LoggerConfigElement, OsclMemAllocator>::iterator it;

            PVLoggerAppender *appender = NULL;
            OsclRefCounter *refCounter = NULL;
            if (iLoggerConfigElements.empty())
            {
                return;
            }

            if (iAppenderType == 0)
            {
                appender = new StdErrAppender<TimeAndIdLayout, 1024>();
                OsclRefCounterSA<LogAppenderDestructDealloc<StdErrAppender<TimeAndIdLayout, 1024> > > *appenderRefCounter =
                    new OsclRefCounterSA<LogAppenderDestructDealloc<StdErrAppender<TimeAndIdLayout, 1024> > >(appender);
                refCounter = appenderRefCounter;
            }
            else if (iAppenderType == 1)
            {
                OSCL_wHeapString<OsclMemAllocator> logfilename(OUTPUTNAME_PREPEND_WSTRING);
                logfilename += _STRLIT_WCHAR("player.log");
                appender = (PVLoggerAppender*)TextFileAppender<TimeAndIdLayout, 1024>::CreateAppender(logfilename.get_str());
                OsclRefCounterSA<LogAppenderDestructDealloc<TextFileAppender<TimeAndIdLayout, 1024> > > *appenderRefCounter =
                    new OsclRefCounterSA<LogAppenderDestructDealloc<TextFileAppender<TimeAndIdLayout, 1024> > >(appender);
                refCounter = appenderRefCounter;
            }
            else
            {
                OSCL_wHeapString<OsclMemAllocator> logfilename(OUTPUTNAME_PREPEND_WSTRING);
                logfilename += _STRLIT_WCHAR("player.log");
                appender = (PVLoggerAppender*)MemAppender<TimeAndIdLayout, 1024>::CreateAppender(logfilename.get_str());
                OsclRefCounterSA<LogAppenderDestructDealloc<MemAppender<TimeAndIdLayout, 1024> > > *appenderRefCounter =
                    new OsclRefCounterSA<LogAppenderDestructDealloc<MemAppender<TimeAndIdLayout, 1024> > >(appender);
                refCounter = appenderRefCounter;
            }

            OsclSharedPtr<PVLoggerAppender> appenderPtr(appender, refCounter);

            for (it = iLoggerConfigElements.begin(); it != iLoggerConfigElements.end(); it++)
            {
                PVLogger *node = NULL;
                node = PVLogger::GetLoggerObject(it->iLoggerString);
                node->AddAppender(appenderPtr);
                node->SetLogLevel(it->iLogLevel);
            }
        }

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
        char iLogFileName[255];
        char ibuffer[1024];
        Oscl_Vector<LoggerConfigElement, OsclMemAllocator> iLoggerConfigElements;
};


// Pull out source file name from arguments
//  -source sometestfile.mp4
//
//
void FindSourceFile(cmd_line* command_line,	OSCL_HeapString<OsclMemAllocator>& aFileNameInfo, PVMFFormatType& aInputFileFormatType, FILE* aFile)
{
    aFileNameInfo = SOURCENAME_PREPEND_STRING;
    aFileNameInfo += DEFAULTSOURCEFILENAME;
    aInputFileFormatType = DEFAULTSOURCEFORMATTYPE;

    int iFileArgument = 0;
    bool iFileFound = false;
    bool cmdline_iswchar = command_line->is_wchar();

    int count = command_line->get_count();

    // Search for the "-source" argument
    // Go through each argument
    for (int iFileSearch = 0; iFileSearch < count; iFileSearch++)
    {
        char argstr[128];
        // Convert to UTF8 if necessary
        if (cmdline_iswchar)
        {
            oscl_wchar* argwstr = NULL;
            command_line->get_arg(iFileSearch, argwstr);
            oscl_UnicodeToUTF8(argwstr, oscl_strlen(argwstr), argstr, 128);
            argstr[127] = NULL;
        }
        else
        {
            char* tmpstr = NULL;
            command_line->get_arg(iFileSearch, tmpstr);
            int32 tmpstrlen = oscl_strlen(tmpstr) + 1;
            if (tmpstrlen > 128)
            {
                tmpstrlen = 128;
            }
            oscl_strncpy(argstr, tmpstr, tmpstrlen);
            argstr[tmpstrlen-1] = NULL;
        }

        // Do the string compare
        if (oscl_strcmp(argstr, "-help") == NULL)
        {
            fprintf(aFile, "Source specification option. Default is 'test.mp4':\n");
            fprintf(aFile, "  -source sourcename\n");
            fprintf(aFile, "   Specify the source filename or URL to use for test cases which\n");
            fprintf(aFile, "   allow user-specified source name. The unit test determines the\n");
            fprintf(aFile, "   source format type using extension or URL header.\n\n");
        }
        else if (oscl_strcmp(argstr, "-source") == NULL)
        {
            iFileFound = true;
            iFileArgument = ++iFileSearch;
            break;
        }
    }

    if (iFileFound)
    {
        // Convert to UTF8 if necessary
        if (cmdline_iswchar)
        {
            oscl_wchar* cmd;
            command_line->get_arg(iFileArgument, cmd);
            char tmpstr[256];
            oscl_UnicodeToUTF8(cmd, oscl_strlen(cmd), tmpstr, 256);
            tmpstr[255] = NULL;
            aFileNameInfo = tmpstr;
        }
        else
        {
            char* cmdlinefilename = NULL;
            command_line->get_arg(iFileArgument, cmdlinefilename);
            aFileNameInfo = cmdlinefilename;
        }

        // Check the file extension to determine format type
        // AAC file
        if (oscl_strstr(aFileNameInfo.get_cstr(), ".aac") != NULL || oscl_strstr(aFileNameInfo.get_cstr(), ".AAC") != NULL)
        {
            aInputFileFormatType = PVMF_MIME_AACFF;
        }
        // MP3 file
        else  if (oscl_strstr(aFileNameInfo.get_cstr(), ".mp3") != NULL || oscl_strstr(aFileNameInfo.get_cstr(), ".MP3") != NULL)
        {
            aInputFileFormatType = PVMF_MIME_MP3FF;
        }
        // AMR file (IETF and IF2)
        else  if (oscl_strstr(aFileNameInfo.get_cstr(), ".amr") != NULL || oscl_strstr(aFileNameInfo.get_cstr(), ".AMR") != NULL ||
                  oscl_strstr(aFileNameInfo.get_cstr(), ".cod") != NULL || oscl_strstr(aFileNameInfo.get_cstr(), ".COD") != NULL)
        {
            aInputFileFormatType = PVMF_MIME_AMRFF;
        }
        // RTSP URL
        else  if ((!oscl_strncmp("rtsp", aFileNameInfo.get_cstr(), 4)) ||
                  (!oscl_strncmp("RTSP", aFileNameInfo.get_cstr(), 4)))
        {
            aInputFileFormatType = PVMF_MIME_DATA_SOURCE_RTSP_URL;
        }
        // HTTP URL
        else  if (oscl_strstr(aFileNameInfo.get_cstr(), "http:") != NULL || oscl_strstr(aFileNameInfo.get_cstr(), "HTTP:") != NULL)
        {
            aInputFileFormatType = PVMF_MIME_DATA_SOURCE_HTTP_URL;
        }
        // MP4/3GP file
        else  if (oscl_strstr(aFileNameInfo.get_cstr(), ".mp4") != NULL || oscl_strstr(aFileNameInfo.get_cstr(), ".MP4") != NULL ||
                  oscl_strstr(aFileNameInfo.get_cstr(), ".3gp") != NULL || oscl_strstr(aFileNameInfo.get_cstr(), ".3GP") != NULL)
        {
            aInputFileFormatType = PVMF_MIME_MPEG4FF;
        }
        // ASF file
        else  if (oscl_strstr(aFileNameInfo.get_cstr(), ".asf") != NULL || oscl_strstr(aFileNameInfo.get_cstr(), ".ASF") != NULL ||
                  oscl_strstr(aFileNameInfo.get_cstr(), ".wma") != NULL || oscl_strstr(aFileNameInfo.get_cstr(), ".WMA") != NULL ||
                  oscl_strstr(aFileNameInfo.get_cstr(), ".wmv") != NULL || oscl_strstr(aFileNameInfo.get_cstr(), ".WMV") != NULL)
        {
            aInputFileFormatType = PVMF_MIME_ASFFF;
        }
        // SDP file
        else  if (oscl_strstr(aFileNameInfo.get_cstr(), ".sdp") != NULL || oscl_strstr(aFileNameInfo.get_cstr(), ".SDP") != NULL)
        {
            aInputFileFormatType = PVMF_MIME_DATA_SOURCE_SDP_FILE;
        }
        // PVX file
        else  if (oscl_strstr(aFileNameInfo.get_cstr(), ".pvx") != NULL || oscl_strstr(aFileNameInfo.get_cstr(), ".PVX") != NULL)
        {
            aInputFileFormatType = PVMF_MIME_DATA_SOURCE_PVX_FILE;
        }
        // WAV file
        else  if (oscl_strstr(aFileNameInfo.get_cstr(), ".wav") != NULL || oscl_strstr(aFileNameInfo.get_cstr(), ".WAV") != NULL)
        {
            aInputFileFormatType = PVMF_MIME_WAVFF;
        }
        // Unknown so set to unknown and let the player engine determine the format type
        else
        {
            fprintf(file, "Source type unknown so setting to unknown and have the utility recognize it\n");
            aInputFileFormatType = PVMF_MIME_FORMAT_UNKNOWN;
        }
    }
}

//Find test range args:
//To run a range of tests by enum ID:
//  -test 17 29
void FindTestRange(cmd_line* command_line,	int32& iFirstTest, int32 &iLastTest, FILE* aFile)
{
    //default is to run all tests.
    iFirstTest = 0;
    iLastTest = 999;

    int iTestArgument = 0;
    char *iTestArgStr1 = NULL;
    char *iTestArgStr2 = NULL;
    bool iTestFound = false;
    bool cmdline_iswchar = command_line->is_wchar();

    int count = command_line->get_count();

    // Search for the "-test" argument
    char *iSourceFind = NULL;
    if (cmdline_iswchar)
    {
        iSourceFind = new char[256];
    }

    // Go through each argument
    for (int iTestSearch = 0; iTestSearch < count; iTestSearch++)
    {
        // Convert to UTF8 if necessary
        if (cmdline_iswchar)
        {
            OSCL_TCHAR* cmd = NULL;
            command_line->get_arg(iTestSearch, cmd);
            oscl_UnicodeToUTF8(cmd, oscl_strlen(cmd), iSourceFind, 256);
        }
        else
        {
            iSourceFind = NULL;
            command_line->get_arg(iTestSearch, iSourceFind);
        }

        // Do the string compare
        if (oscl_strcmp(iSourceFind, "-help") == NULL)
        {
            fprintf(aFile, "Test cases to run option. Default is ALL:\n");
            fprintf(aFile, "  -test x y\n");
            fprintf(aFile, "   Specify a range of test cases to run. To run one test case, use the\n");
            fprintf(aFile, "   same index for x and y.\n\n");
        }
        else if (oscl_strcmp(iSourceFind, "-test") == NULL)
        {
            iTestFound = true;
            iTestArgument = ++iTestSearch;
            break;
        }
    }

    if (cmdline_iswchar)
    {
        delete[] iSourceFind;
        iSourceFind = NULL;
    }

    if (iTestFound)
    {
        // Convert to UTF8 if necessary
        if (cmdline_iswchar)
        {
            iTestArgStr1 = new char[256];
            OSCL_TCHAR* cmd;
            command_line->get_arg(iTestArgument, cmd);
            if (cmd)
            {
                oscl_UnicodeToUTF8(cmd, oscl_strlen(cmd), iTestArgStr1, 256);
            }

            iTestArgStr2 = new char[256];
            command_line->get_arg(iTestArgument + 1, cmd);
            if (cmd)
            {
                oscl_UnicodeToUTF8(cmd, oscl_strlen(cmd), iTestArgStr2, 256);
            }
        }
        else
        {
            command_line->get_arg(iTestArgument, iTestArgStr1);
            command_line->get_arg(iTestArgument + 1, iTestArgStr2);
        }

        //Pull out 2 integers...
        if (iTestArgStr1
                && '0' <= iTestArgStr1[0] && iTestArgStr1[0] <= '9'
                && iTestArgStr2
                && '0' <= iTestArgStr2[0] && iTestArgStr2[0] <= '9')
        {
            int len = oscl_strlen(iTestArgStr1);
            switch (len)
            {
                case 3:
                    iFirstTest = 0;
                    if ('0' <= iTestArgStr1[0] && iTestArgStr1[0] <= '9')
                    {
                        iFirstTest = iFirstTest + 100 * (iTestArgStr1[0] - '0');
                    }

                    if ('0' <= iTestArgStr1[1] && iTestArgStr1[1] <= '9')
                    {
                        iFirstTest = iFirstTest + 10 * (iTestArgStr1[1] - '0');
                    }

                    if ('0' <= iTestArgStr1[2] && iTestArgStr1[2] <= '9')
                    {
                        iFirstTest = iFirstTest + 1 * (iTestArgStr1[2] - '0');
                    }
                    break;

                case 2:
                    iFirstTest = 0;
                    if ('0' <= iTestArgStr1[0] && iTestArgStr1[0] <= '9')
                    {
                        iFirstTest = iFirstTest + 10 * (iTestArgStr1[0] - '0');
                    }

                    if ('0' <= iTestArgStr1[1] && iTestArgStr1[1] <= '9')
                    {
                        iFirstTest = iFirstTest + 1 * (iTestArgStr1[1] - '0');
                    }
                    break;

                case 1:
                    iFirstTest = 0;
                    if ('0' <= iTestArgStr1[0] && iTestArgStr1[0] <= '9')
                    {
                        iFirstTest = iFirstTest + 1 * (iTestArgStr1[0] - '0');
                    }
                    break;

                default:
                    break;
            }

            len = oscl_strlen(iTestArgStr2);
            switch (len)
            {
                case 3:
                    iLastTest = 0;
                    if ('0' <= iTestArgStr2[0] && iTestArgStr2[0] <= '9')
                    {
                        iLastTest = iLastTest + 100 * (iTestArgStr2[0] - '0');
                    }

                    if ('0' <= iTestArgStr2[1] && iTestArgStr2[1] <= '9')
                    {
                        iLastTest = iLastTest + 10 * (iTestArgStr2[1] - '0');
                    }

                    if ('0' <= iTestArgStr2[2] && iTestArgStr2[2] <= '9')
                    {
                        iLastTest = iLastTest + 1 * (iTestArgStr2[2] - '0');
                    }
                    break;

                case 2:
                    iLastTest = 0;
                    if ('0' <= iTestArgStr2[0] && iTestArgStr2[0] <= '9')
                    {
                        iLastTest = iLastTest + 10 * (iTestArgStr2[0] - '0');
                    }

                    if ('0' <= iTestArgStr2[1] && iTestArgStr2[1] <= '9')
                    {
                        iLastTest = iLastTest + 1 * (iTestArgStr2[1] - '0');
                    }
                    break;

                case 1:
                    iLastTest = 0;
                    if ('0' <= iTestArgStr2[0] && iTestArgStr2[0] <= '9')
                    {
                        iLastTest = iLastTest + 1 * (iTestArgStr2[0] - '0');
                    }
                    break;

                default:
                    break;
            }
        }
    }

    if (cmdline_iswchar)
    {
        if (iTestArgStr1)
        {
            delete[] iTestArgStr1;
            iTestArgStr1 = NULL;
        }

        if (iTestArgStr2)
        {
            delete[] iTestArgStr2;
            iTestArgStr2 = NULL;
        }

        if (iSourceFind)
        {
            delete[] iSourceFind;
            iSourceFind = NULL;
        }
    }
}


void FindMemMgmtRelatedCmdLineParams(cmd_line* command_line, bool& aPrintDetailedMemLeakInfo, FILE* aFile)
{
    aPrintDetailedMemLeakInfo = false;

    bool cmdline_iswchar = command_line->is_wchar();

    int count = command_line->get_count();

    // Search for the "-logerr"/"-logwarn" argument
    char *iSourceFind = NULL;
    if (cmdline_iswchar)
    {
        iSourceFind = new char[256];
    }

    // Go through each argument
    for (int iTestSearch = 0; iTestSearch < count; iTestSearch++)
    {
        // Convert to UTF8 if necessary
        if (cmdline_iswchar)
        {
            OSCL_TCHAR* cmd = NULL;
            command_line->get_arg(iTestSearch, cmd);
            oscl_UnicodeToUTF8(cmd, oscl_strlen(cmd), iSourceFind, 256);
        }
        else
        {
            iSourceFind = NULL;
            command_line->get_arg(iTestSearch, iSourceFind);
        }

        // Do the string compare
        if (oscl_strcmp(iSourceFind, "-help") == NULL)
        {
            fprintf(aFile, "Printing leak info option. Default is OFF:\n");
            fprintf(aFile, "  -leakinfo\n");
            fprintf(aFile, "   If there is a memory leak, prints out the memory leak information\n");
            fprintf(aFile, "   after all specified test cases have finished running.\n\n");
        }
        else if (oscl_strcmp(iSourceFind, "-leakinfo") == NULL)
        {
            aPrintDetailedMemLeakInfo = true;
        }
    }

    if (cmdline_iswchar)
    {
        delete[] iSourceFind;
        iSourceFind = NULL;
    }
}


void FindLogLevel(cmd_line* command_line, int32& loglevel, FILE* aFile)
{
    //default is verbose
    loglevel = PVLOGMSG_DEBUG;

    bool cmdline_iswchar = command_line->is_wchar();

    int count = command_line->get_count();

    // Search for the "-logerr"/"-logwarn" argument
    char *iSourceFind = NULL;
    if (cmdline_iswchar)
    {
        iSourceFind = new char[256];
    }

    // Go through each argument
    for (int iTestSearch = 0; iTestSearch < count; iTestSearch++)
    {
        // Convert to UTF8 if necessary
        if (cmdline_iswchar)
        {
            OSCL_TCHAR* cmd = NULL;
            command_line->get_arg(iTestSearch, cmd);
            oscl_UnicodeToUTF8(cmd, oscl_strlen(cmd), iSourceFind, 256);
        }
        else
        {
            iSourceFind = NULL;
            command_line->get_arg(iTestSearch, iSourceFind);
        }

        // Do the string compare
        if (oscl_strcmp(iSourceFind, "-help") == NULL)
        {
            fprintf(aFile, "Log level options. Default is debug level:\n");
            fprintf(aFile, "  -logerr\n");
            fprintf(aFile, "   Log at error level\n");
            fprintf(aFile, "  -logwarn\n");
            fprintf(aFile, "   Log at warning level\n\n");
        }
        else if (oscl_strcmp(iSourceFind, "-logerr") == NULL)
        {
            loglevel = PVLOGMSG_ERR;
        }
        else if (oscl_strcmp(iSourceFind, "-logwarn") == NULL)
        {
            loglevel = PVLOGMSG_WARNING;
        }
    }

    if (cmdline_iswchar)
    {
        delete[] iSourceFind;
        iSourceFind = NULL;
    }
}

int _local_main(FILE* filehandle, cmd_line* command_line);

// Entry point for the unit test program
int local_main(FILE* filehandle, cmd_line* command_line)
{
    //Init Oscl
    OsclBase::Init();
    OsclErrorTrap::Init();
    OsclMem::Init();
    OMX_Init();

    const int numArgs = 10; //change as per the number of args below
    char *argv[numArgs];
    char arr[numArgs][MAX_LEN];
    FILE *InputFile = NULL;
    int argc = 0;

    fprintf(filehandle, "Test Program for pvFrameMetadata utility class.\n");

    InputFile = fopen("input.txt", "r+");
    if (NULL != InputFile)
    {
        int ii = 0;
        int len = 0;
        fseek(InputFile , 0 , SEEK_SET);
        while (!feof(InputFile))
        {
            arr[ii][0] = '\0';
            fgets(arr[ii], 127, InputFile);
            len = strlen(arr[ii]);
            if (arr[ii][len-1] == '\n')
            {
                arr[ii][len-1] = '\0';
            }
            else
            {
                arr[ii][len-1] = '\0';
            }
            argv[ii] = arr[ii];

            ii++;
        }
        fclose(InputFile);

        argc = ii - 1;
    }

    int n = 0;

    command_line->setup(argc - n, &argv[n]);

    bool oPrintDetailedMemLeakInfo = false;
    FindMemMgmtRelatedCmdLineParams(command_line, oPrintDetailedMemLeakInfo, filehandle);

    //Run the test under a trap
    int result = 0;
    int32 err = 0;

    OSCL_TRY(err, result = _local_main(filehandle, command_line););

    //Show any exception.
    if (err != 0)
    {
        fprintf(file, "Error!  Leave %d\n", err);
    }

    //Cleanup
    OMX_Deinit();
#if !(OSCL_BYPASS_MEMMGT)
    //Check for memory leaks before cleaning up OsclMem.
    OsclAuditCB auditCB;
    OsclMemInit(auditCB);
    if (auditCB.pAudit)
    {
        MM_Stats_t* stats = auditCB.pAudit->MM_GetStats("");
        if (stats)
        {
            fprintf(file, "\nMemory Stats:\n");
            fprintf(file, "  peakNumAllocs %d\n", stats->peakNumAllocs);
            fprintf(file, "  peakNumBytes %d\n", stats->peakNumBytes);
            fprintf(file, "  totalNumAllocs %d\n", stats->totalNumAllocs);
            fprintf(file, "  totalNumBytes %d\n", stats->totalNumBytes);
            fprintf(file, "  numAllocFails %d\n", stats->numAllocFails);
            if (stats->numAllocs)
            {
                fprintf(file, "  ERROR: Memory Leaks! numAllocs %d, numBytes %d\n", stats->numAllocs, stats->numBytes);
            }
        }
        uint32 leaks = auditCB.pAudit->MM_GetNumAllocNodes();
        if (leaks != 0)
        {
            if (oPrintDetailedMemLeakInfo)
            {
                fprintf(file, "ERROR: %d Memory leaks detected!\n", leaks);
                MM_AllocQueryInfo*info = auditCB.pAudit->MM_CreateAllocNodeInfo(leaks);
                uint32 leakinfo = auditCB.pAudit->MM_GetAllocNodeInfo(info, leaks, 0);
                if (leakinfo != leaks)
                {
                    fprintf(file, "ERROR: Leak info is incomplete.\n");
                }
                for (uint32 i = 0;i < leakinfo;i++)
                {
                    fprintf(file, "Leak Info:\n");
                    fprintf(file, "  allocNum %d\n", info[i].allocNum);
                    fprintf(file, "  fileName %s\n", info[i].fileName);
                    fprintf(file, "  lineNo %d\n", info[i].lineNo);
                    fprintf(file, "  size %d\n", info[i].size);
                    fprintf(file, "  pMemBlock 0x%x\n", info[i].pMemBlock);
                    fprintf(file, "  tag %s\n", info[i].tag);
                }
                auditCB.pAudit->MM_ReleaseAllocNodeInfo(info);
            }
        }
    }
#endif

    OsclMem::Cleanup();
    OsclErrorTrap::Cleanup();
    OsclBase::Cleanup();
    return result;
}

int _local_main(FILE* filehandle, cmd_line* command_line)
{
    file = filehandle;

    // Print out the extension for help if no argument
    if (command_line->get_count() == 0)
    {
        fprintf(file, "  Specify '-help' first to get help information on options\n\n");
    }

    OSCL_HeapString<OsclMemAllocator> filenameinfo;
    PVMFFormatType inputformattype ;
    FindSourceFile(command_line, filenameinfo, inputformattype, file);

    int32 firsttest, lasttest;
    FindTestRange(command_line, firsttest, lasttest, file);

    int32 loglevel;
    FindLogLevel(command_line, loglevel, file);

    fprintf(file, "  Input file name '%s'\n", filenameinfo.get_cstr());
    fprintf(file, "  Test case range %d to %d\n", firsttest, lasttest);
    fprintf(file, "  Log level %d\n", loglevel);

    pvframemetadata_utility_test_suite* util_tests = NULL;
    util_tests = new pvframemetadata_utility_test_suite(filenameinfo.get_str(), inputformattype, firsttest, lasttest, loglevel);
    if (util_tests)
    {
        // Run the utility test
        util_tests->run_test();

        // Print out the results
        text_test_interpreter interp;
        _STRING rs = interp.interpretation(util_tests->last_result());
        fprintf(file, rs.c_str());

        const test_result the_result = util_tests->last_result();
        delete util_tests;
        util_tests = NULL;

        return (the_result.success_count() != the_result.total_test_count());
    }
    else
    {
        fprintf(file, "ERROR! pvframemetadata_utility_test_suite could not be instantiated.\n");
        return 1;
    }
}


pvframemetadata_utility_test_suite::pvframemetadata_utility_test_suite(char *aFileName, PVMFFormatType aFileType, int32 aFirstTest, int32 aLastTest, int32 aLogLevel)
        : test_case()
{
    adopt_test_case(new pvframemetadata_utility_test(aFileName, aFileType, aFirstTest, aLastTest, aLogLevel));
}



pvframemetadata_utility_test::pvframemetadata_utility_test(char *aFileName, PVMFFormatType aFileType, int32 aFirstTest, int32 aLastTest, int32 aLogLevel)
{
    iFileName = aFileName;
    iFileType = aFileType;
    iCurrentTestNumber = 0;
    iCurrentTest = NULL;
    iFirstTest = aFirstTest;
    iLastTest = aLastTest;
    iLogLevel = aLogLevel;
    iTotalAlloc = 0;
    iTotalBytes = 0;
    iAllocFails = 0;
    iNumAllocs = 0;
}


pvframemetadata_utility_test::~pvframemetadata_utility_test()
{
}


void pvframemetadata_utility_test::TestCompleted(test_case &tc)
{
    // Print out the result for this test case
    const test_result the_result = tc.last_result();
    fprintf(file, "  Successes %d, Failures %d\n"
            , the_result.success_count() - iTotalSuccess, the_result.failures().size() - iTotalFail);
    iTotalSuccess = the_result.success_count();
    iTotalFail = the_result.failures().size();
    iTotalError = the_result.errors().size();

    // Go to next test
    ++iCurrentTestNumber;

    // Stop the scheduler
    OsclExecScheduler *sched = OsclExecScheduler::Current();
    if (sched)
    {
        sched->StopScheduler();
    }
}

void pvframemetadata_utility_test::test()
{
    // Specify the starting test case
    iCurrentTestNumber = iFirstTest;
    iTotalSuccess = iTotalFail = iTotalError = 0;

    while (iCurrentTestNumber <= iLastTest || iCurrentTestNumber < BeyondLastTest)
    {
        if (iCurrentTest)
        {
            delete iCurrentTest;
            iCurrentTest = NULL;

            // Shutdown PVLogger and scheduler before checking mem stats
            OsclScheduler::Cleanup();
            PVLogger::Cleanup();
#if !(OSCL_BYPASS_MEMMGT)
            // Print out the memory usage results for this test case
            OsclAuditCB auditCB;
            OsclMemInit(auditCB);
            if (auditCB.pAudit)
            {
                MM_Stats_t* stats = auditCB.pAudit->MM_GetStats("");
                if (stats)
                {
                    fprintf(file, "  Mem stats: TotalAllocs(%d), TotalBytes(%d),\n             AllocFailures(%d), AllocLeak(%d)\n",
                            stats->totalNumAllocs - iTotalAlloc, stats->totalNumBytes - iTotalBytes, stats->numAllocFails - iAllocFails, stats->numAllocs - iNumAllocs);
                }
                else
                {
                    fprintf(file, "Retrieving memory statistics after running test case failed! Memory statistics result is not available.\n");
                }
            }
            else
            {
                fprintf(file, "Memory audit not available! Memory statistics result is not available.\n");
            }
#endif
        }

#if !(OSCL_BYPASS_MEMMGT)
        // Obtain the current mem stats before running the test case
        OsclAuditCB auditCB;
        OsclMemInit(auditCB);
        if (auditCB.pAudit)
        {
            MM_Stats_t* stats = auditCB.pAudit->MM_GetStats("");
            if (stats)
            {
                iTotalAlloc = stats->totalNumAllocs;
                iTotalBytes = stats->totalNumBytes;
                iAllocFails = stats->numAllocFails;
                iNumAllocs = stats->numAllocs;
            }
            else
            {
                fprintf(file, "Retrieving memory statistics before running test case failed! Memory statistics result would be invalid.\n");
            }
        }
        else
        {
            fprintf(file, "Memory audit not available! Memory statistics result would be invalid.\n");
        }
#endif

        // Stop at last test of selected range.
        if (iCurrentTestNumber > iLastTest || iCurrentTestNumber == LastTest)
        {
            iCurrentTestNumber = BeyondLastTest;
        }
        else
        {
            fprintf(file, "\nStarting Test %d: ", iCurrentTestNumber);
            SetupLoggerScheduler();
        }

        // Setup the standard test case parameters based on current unit test settings
        PVFrameMetadataAsyncTestParam testparam;
        testparam.iObserver = this;
        testparam.iTestCase = this;
        testparam.iTestMsgOutputFile = file;
        testparam.iFileName = iFileName;
        testparam.iFileType = iFileType;
        testparam.iOutputFrameType = PVMF_MIME_YUV420;

        switch (iCurrentTestNumber)
        {
            case NewDeleteTest:
                iCurrentTest = new pvframemetadata_async_test_newdelete(testparam);
                break;

            case GetSourceMetadataTest:
                iCurrentTest = new pvframemetadata_async_test_getmetadata(testparam, 1);
                break;

            case GetSourceMetadataandFrameTest:
                iCurrentTest = new pvframemetadata_async_test_getmetadata(testparam, 2);
                break;

            case GetSourceMetadataandBestFrameTest:
                iCurrentTest = new pvframemetadata_async_test_getmetadata(testparam, 2, true);
                break;

            case GetAllMetadataTest:
                iCurrentTest = new pvframemetadata_async_test_getmetadata(testparam, 3);
                break;

            case GetFirstFrameYUV420AndMetadataTest:
#if RUN_YUV420_TESTCASES
                testparam.iOutputFrameType = PVMF_MIME_YUV420;
                iCurrentTest = new pvframemetadata_async_test_getfirstframemetadata(testparam);
                fprintf(file, "YUV 4:2:0 ");
#else
                fprintf(file, "YUV420 test cases disabled\n");
#endif
                break;

            case GetFirstFrameYUV420UtilityBufferTest:
#if RUN_YUV420_TESTCASES
                testparam.iOutputFrameType = PVMF_MIME_YUV420;
                iCurrentTest = new pvframemetadata_async_test_getfirstframeutilitybuffer(testparam);
                fprintf(file, "YUV 4:2:0 ");
#else
                fprintf(file, "YVU420 test cases disabled\n");
#endif
                break;

            case GetFirstFrameRGB12AndMetadataTest:
#if RUN_RGB12_TESTCASES
                testparam.iOutputFrameType = PVMF_RGB12;
                iCurrentTest = new pvframemetadata_async_test_getfirstframemetadata(testparam);
                fprintf(file, "RGB 12bpp ");
#else
                fprintf(file, "RGB12 test cases disabled\n");
#endif
                break;

            case GetFirstFrameRGB12UtilityBufferTest:
#if RUN_RGB12_TESTCASES
                testparam.iOutputFrameType = PVMF_RGB12;
                iCurrentTest = new pvframemetadata_async_test_getfirstframeutilitybuffer(testparam);
                fprintf(file, "RGB 12bpp ");
#else
                fprintf(file, "RGB12 test cases disabled\n");
#endif
                break;

            case GetFirstFrameRGB16AndMetadataTest:
#if RUN_RGB16_TESTCASES
                testparam.iOutputFrameType = PVMF_MIME_RGB16;
                iCurrentTest = new pvframemetadata_async_test_getfirstframemetadata(testparam);
                fprintf(file, "RGB 16bpp ");
#else
                fprintf(file, "RGB16 test cases disabled\n");
#endif
                break;

            case GetFirstFrameRGB16UtilityBufferTest:
#if RUN_RGB16_TESTCASES
                testparam.iOutputFrameType = PVMF_MIME_RGB16;
                iCurrentTest = new pvframemetadata_async_test_getfirstframeutilitybuffer(testparam);
                fprintf(file, "RGB 16bpp ");
#else
                fprintf(file, "RGB16 test cases disabled\n");
#endif
                break;

            case GetFirstFrameRGB24AndMetadataTest:
#if RUN_RGB24_TESTCASES
                testparam.iOutputFrameType = PVMF_RGB24;
                iCurrentTest = new pvframemetadata_async_test_getfirstframemetadata(testparam);
                fprintf(file, "RGB 24bpp ");
#else
                fprintf(file, "RGB24 test cases disabled\n");
#endif
                break;

            case GetFirstFrameRGB24UtilityBufferTest:
#if RUN_RGB24_TESTCASES
                testparam.iOutputFrameType = PVMF_RGB24;
                iCurrentTest = new pvframemetadata_async_test_getfirstframeutilitybuffer(testparam);
                fprintf(file, "RGB 24bpp ");
#else
                fprintf(file, "RGB24 test cases disabled\n");
#endif
                break;

            case Get30thFrameTest:
                iCurrentTest = new pvframemetadata_async_test_get30thframe(testparam);
                break;

            case Get10secFrameTest:
                iCurrentTest = new pvframemetadata_async_test_get10secframe(testparam);
                break;

            case CancelCommandTest:
                iCurrentTest = new pvframemetadata_async_test_cancelcommand(testparam);
                break;

            case MultipleGetFirstFrameAndMetadataTest:
                iCurrentTest = new pvframemetadata_async_test_multigetfirstframemetadata(testparam);
                break;

            case MultipleGetFramesYUVTest:
#if RUN_YUV420_TESTCASES
                testparam.iOutputFrameType = PVMF_MIME_YUV420;
                iCurrentTest = new pvframemetadata_async_test_multigetframe(testparam);
                fprintf(file, "YUV 4:2:0 ");
#else
                fprintf(file, "YUV420 test cases disabled\n");
#endif
                break;

            case MultipleGetFramesRGB24Test:
#if RUN_RGB24_TESTCASES
                testparam.iOutputFrameType = PVMF_RGB24;
                iCurrentTest = new pvframemetadata_async_test_multigetframe(testparam);
                fprintf(file, "RGB 24bpp ");
#else
                fprintf(file, "RGB24 test cases disabled\n");
#endif
                break;

            case InvalidSourceFileTest:
                iCurrentTest = new pvframemetadata_async_test_invalidsourcefile(testparam);
                break;

            case NoGetFrameTest:
                iCurrentTest = new pvframemetadata_async_test_nogetframe(testparam);
                break;

            case NoVideoTrackTest:
                testparam.iFileName = NOVIDEOSOURCEFILENAME;
                testparam.iFileType = NOVIDEOSOURCEFORMATTYPE;
                iCurrentTest = new pvframemetadata_async_test_novideotrack(testparam);
                break;

            case ProtectedMetadataTest:
#if RUN_JANUSCPM_TESTCASES
                iCurrentTest = new pvframemetadata_async_test_protectedmetadata(testparam);
#else
                fprintf(file, "Janus CPM test cases disabled\n");
#endif
                break;
            case SetTimeoutAndGetFrameTest:
                iCurrentTest = new pvframemetadata_async_test_settimeout_getframe(testparam, 2, false);
                break;

            case SetPlayerKeyTest:
                iCurrentTest = new pvframemetadata_async_test_set_player_key(testparam);
                break;

            case BeyondLastTest:
            default:
                iCurrentTestNumber = BeyondLastTest;
                break;
        }

        if (iCurrentTest)
        {
            OsclExecScheduler *sched = OsclExecScheduler::Current();
            if (sched)
            {
                // Print out the test name
                fprintf(file, "%s\n", iCurrentTest->iTestCaseName.get_cstr());
                // Start the test
                iCurrentTest->StartTest();
                // Start the scheduler so the test case would run
#if USE_NATIVE_SCHEDULER
                // Have PV scheduler use the scheduler native to the system
                sched->StartNativeScheduler();
#else
                // Have PV scheduler use its own implementation of the scheduler
                sched->StartScheduler();
#endif
            }
            else
            {
                fprintf(file, "ERROR! Scheduler is not available. Test case could not run.");
                ++iCurrentTestNumber;
            }
        }
        else
        {
            ++iCurrentTestNumber;
            if (iCurrentTestNumber < BeyondLastTest)
            {
                // Shutdown PVLogger and scheduler before continuing on
                OsclScheduler::Cleanup();
                PVLogger::Cleanup();
            }
        }
    }
}

void pvframemetadata_utility_test::SetupLoggerScheduler()
{
    // Enable the following code for logging (on Symbian, RDebug)
    PVLogger::Init();
    PVLoggerConfigFile obj;
    if (obj.IsLoggerConfigFilePresent())
    {
        obj.SetLoggerSettings();
    }
    // Construct and install the active scheduler
    OsclScheduler::Init("PVFrameMetadataUtilityTestScheduler");
}




