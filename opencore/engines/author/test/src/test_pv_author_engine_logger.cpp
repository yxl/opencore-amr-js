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

// test_pv_author_engine_logger.cpp: implementation of the PVAELogger class.
//
//////////////////////////////////////////////////////////////////////
#ifndef PVLOGGER_FILE_APPENDER_H_INCLUDED
#include "pvlogger_file_appender.h"
#endif
#ifndef PVLOGGER_STDERR_APPENDER_H_INCLUDED
#include "pvlogger_stderr_appender.h"
#endif
#ifndef PVLOGGER_MEM_APPENDER_H_INCLUDED
#include "pvlogger_mem_appender.h"
#endif
#ifndef PVLOGGER_TIME_AND_ID_LAYOUT_H_INCLUDED
#include "pvlogger_time_and_id_layout.h"
#endif
#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif
#ifndef OSCL_STRING_CONTAINERS_H_INCLUDED
#include "oscl_string_containers.h"
#endif
#ifndef TEST_PV_AUTHOR_ENGINE_LOGGER_H_INCLUDED
#include "test_pv_author_engine_logger.h"
#endif

////////////////////////////////////////////////////////////////////////////////
// Macro redefinition for typing convenience : define an 8-bit char* or 16-bit wchar
#define _CSTR(x) _STRLIT_CHAR(x)
#define _W(x) _STRLIT_WCHAR(x)

#define KLogFile ((OSCL_TCHAR*) _STRLIT("pvauthorlog.txt"))

typedef OSCL_HeapString<OsclMemAllocator> HeapString;
typedef OSCL_wHeapString<OsclMemAllocator> wHeapString;

// Open pwszConfigFile to read nodes (and levels) to log
// Returns true on success
OSCL_EXPORT_REF bool PVAELogger::ParseConfigFile(const oscl_wchar *pwszConfigFile)
{
    OSCL_ASSERT(pwszConfigFile);
    if (!pwszConfigFile)
        return false;

    int32 iStatus = 0;
    PVLogger		 *loggernode = NULL;

    bool bFileExists = false;
    Oscl_FileServer fs;
    Oscl_File		filehandle;

    // read configuration file pvaelogger.txt in working directory
    fs.Connect();
    if (filehandle.Open(pwszConfigFile, Oscl_File::MODE_READ, fs) == 0)
        bFileExists = true;

    //If the file "pvaelogger.txt" does not exist in the working dir or
    //is empty, set default: log "AuthorEngine", debug level to StdErr
    if (!bFileExists || 0 == filehandle.Size())
    {
        if (bFileExists)
            filehandle.Close();
        fs.Close();
        return false;
    }

    int32 MAXLENGTH = filehandle.Size();
    uint32 lastlineterminator = 0, firstline = 0;

    PVLoggerAppender *appender = NULL;
    OsclRefCounter	 *refCounter = NULL;
    OsclSharedPtr<PVLoggerAppender> sharedAppenderPtr;

    bool bEof = false;
    while (!bEof)
    {
        char *buf = (char *)OSCL_MALLOC(MAXLENGTH + 1);
        filehandle.Seek(lastlineterminator, Oscl_File::SEEKSET);
        if (filehandle.Read((void *)buf, 1, MAXLENGTH))
        {
            //delibrately remove const-ness char* output from oscl_strstr to a modifiable char* for further operation
            char *ptr = OSCL_CONST_CAST(char*, oscl_strstr(buf, _CSTR("\n")));
            if (ptr)
            {
                if (*(ptr - 1) == '\r')
                {
                    *(ptr - 1) = '\0';
                }
                else
                {
                    *ptr = '\0';
                }

                lastlineterminator += ptr - buf + 1;

            }
            else
            {
                bEof = true;
            }

            // Check if this is a commented line
            if (*buf == '#')
            {
                firstline = lastlineterminator;
                if (buf)
                {
                    OSCL_FREE(buf);
                    buf = NULL;
                }
                continue;
            }

            //If the first uncommented line in the file is NO_LOGGING all logging will be disabled
            //If the first uncommented line in the file is LOGTOFILE, a file appender shall be created
            //If the first uncommented line in the file is LOGTOMEM, a memory appender shall be created
            //otherwise all logs shall be dumped on the cmd wnd

            if (appender == NULL &&
                    ((lastlineterminator == 0) || (firstline == lastlineterminator - (ptr - buf + 1))))

            {
                if (oscl_strstr(buf, _CSTR("NO_LOGGING")))
                {
                    filehandle.Close();
                    fs.Close();
                    if (buf)
                    {
                        OSCL_FREE(buf);
                        buf = NULL;
                    }
                    return true;
                }

                iStatus = PVAELogger::CreateTestAppender(buf, appender, refCounter, sharedAppenderPtr);

                if (iStatus != OsclErrNone)
                {
                    OSCL_ASSERT(!_CSTR("Author Engine Unittest: unable to create log appender"));
                    sharedAppenderPtr.Unbind();
                    if (refCounter)
                        delete refCounter;
                    if (appender)
                        delete appender;
                    if (buf)
                    {
                        OSCL_FREE(buf);
                        buf = NULL;
                    }
                    return false;
                }
            }

            //Extract the node
            char *node = (char *)OSCL_MALLOC(MAXLENGTH + 1);
            if (!node)
            {
                if (NULL != buf)
                {
                    OSCL_FREE(buf);
                    buf = NULL;
                }
                return false;
            }

            const char *marker = oscl_strstr(buf, _CSTR(";"));

            if (!marker)
            {
                OSCL_FREE(node);
                if (buf)
                {
                    OSCL_FREE(buf);
                    buf = NULL;
                }
                continue;
            }

            uint32 node_size = marker - buf;

            oscl_strncpy(node, buf, node_size);
            *(node + node_size) = '\0';

            //Extract the log level
            HeapString *loglevel = OSCL_NEW(HeapString, (oscl_strstr(buf, _CSTR(";")) + 1));
            if (!loglevel)
            {
                OSCL_FREE(node);
                if (buf)
                {
                    OSCL_FREE(buf);
                    buf = NULL;
                }
                return false;
            }

            char *tmp = loglevel->get_str();
            bool logallnodes = false;

            //If node = ALLNODES; enable logging for all
            //ALLNODES should be written in the file before any other logger node
            if (oscl_strstr(node, _CSTR("ALLNODES")))
            {
                loggernode = PVLogger::GetLoggerObject("");
                logallnodes = true;
            }
            else
            {
                loggernode = PVLogger::GetLoggerObject(node);
            }

            loggernode->AddAppender(sharedAppenderPtr);

            //Set log level
            if (oscl_strstr(tmp, _CSTR("PVLOGMSG_EMERG")))
                loggernode->SetLogLevel(PVLOGMSG_EMERG);
            else if (oscl_strstr(tmp, _CSTR("PVLOGMSG_ALERT")))
                loggernode->SetLogLevel(PVLOGMSG_ALERT);
            else if (oscl_strstr(tmp, _CSTR("PVLOGMSG_CRIT")))
                loggernode->SetLogLevel(PVLOGMSG_CRIT);
            else if (oscl_strstr(tmp, _CSTR("PVLOGMSG_ERR")))
                loggernode->SetLogLevel(PVLOGMSG_ERR);
            else if (oscl_strstr(tmp, _CSTR("PVLOGMSG_WARNING")))
                loggernode->SetLogLevel(PVLOGMSG_WARNING);
            else if (oscl_strstr(tmp, _CSTR("PVLOGMSG_NOTICE")))
                loggernode->SetLogLevel(PVLOGMSG_NOTICE);
            else if (oscl_strstr(tmp, _CSTR("PVLOGMSG_INFO")))
                loggernode->SetLogLevel(PVLOGMSG_INFO);
            else if (oscl_strstr(tmp, _CSTR("PVLOGMSG_STACK_TRACE")))
                loggernode->SetLogLevel(PVLOGMSG_STACK_TRACE);
            else if (oscl_strstr(tmp, _CSTR("PVLOGMSG_DEBUG")))
                loggernode->SetLogLevel(PVLOGMSG_DEBUG);

            if (logallnodes == true)
                bEof = true;

            if (node)
                OSCL_FREE(node);
            if (loglevel)
                OSCL_DELETE(loglevel);
            if (buf)
            {
                OSCL_FREE(buf);
                buf = NULL;
            }

        }
        else //file read returns 0
        {
            //We have reached the end of the file
            bEof = true;
        }

        if (buf)
        {
            OSCL_FREE(buf);
            buf = NULL;
        }

    }//end of while loop

    filehandle.Close();
    fs.Close();
    return true;
}

// Attach a StdErrAppender to "PVAuthorEngine" at level PVLOGMSG_DEBUG
// Returns true on success

OSCL_EXPORT_REF bool PVAELogger::MakeStdErrAppender()
{
    int32 iStatus;
    PVLoggerAppender *appender   = NULL;
    OsclRefCounter	 *refCounter = NULL;
    PVLogger		 *loggernode = NULL;

    typedef StdErrAppender<TimeAndIdLayout, 1024> typeStdErrAppender;
    OSCL_TRY(iStatus,
             appender = new typeStdErrAppender() ;
             OsclRefCounterSA<LogAppenderDestructDealloc<typeStdErrAppender > > *appenderRefCounter =
                 new OsclRefCounterSA<LogAppenderDestructDealloc<typeStdErrAppender > >(appender);
             refCounter = appenderRefCounter;
            );

    if (iStatus != OsclErrNone)
    {
        delete refCounter;
        delete appender;
        return false;
    }

    OsclSharedPtr<PVLoggerAppender> appenderPtr(appender, refCounter);
    loggernode = PVLogger::GetLoggerObject(_CSTR("PVAuthorEngine"));
    loggernode->AddAppender(appenderPtr);
    loggernode->SetLogLevel(PVLOGMSG_DEBUG);

    return true;
}

OSCL_EXPORT_REF int32 PVAELogger::CreateTestAppender(char* aBuf, PVLoggerAppender*& aAppender, OsclRefCounter*& aRefCounter, OsclSharedPtr<PVLoggerAppender> &aSharedAppenderPtr)
{
    int32 err;
    if (oscl_strstr(aBuf, _CSTR("LOGTOFILE")))
    {
        typedef TextFileAppender<TimeAndIdLayout, 1024> typeAppender;

        OSCL_TRY(err,
                 aAppender = (PVLoggerAppender*)typeAppender::CreateAppender(KLogFile);
                 aRefCounter = new OsclRefCounterSA<LogAppenderDestructDealloc<typeAppender> >(aAppender);
                 aSharedAppenderPtr.Bind(aAppender, aRefCounter);
                );

    }

    else if (oscl_strstr(aBuf, _CSTR("LOGTOMEM")))
    {
        typedef MemAppender<TimeAndIdLayout, 1024> typeAppender;
        OSCL_TRY(err,
                 aAppender = (PVLoggerAppender*)typeAppender::CreateAppender(KLogFile);
                 aRefCounter = new OsclRefCounterSA<LogAppenderDestructDealloc<typeAppender> >(aAppender);
                 aSharedAppenderPtr.Bind(aAppender, aRefCounter);
                );
    }
    else
    {
        typedef StdErrAppender<TimeAndIdLayout, 1024> typeAppender;
        OSCL_TRY(err,
                 aAppender = new typeAppender() ;
                 aRefCounter = new OsclRefCounterSA<LogAppenderDestructDealloc<typeAppender> >(aAppender);
                 aSharedAppenderPtr.Bind(aAppender, aRefCounter);
                );
    }

    return err;
}
