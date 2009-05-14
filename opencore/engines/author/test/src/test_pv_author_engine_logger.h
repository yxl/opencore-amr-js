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
#ifndef TEST_PV_AUTHOR_ENGINE_LOGGER_H_INCLUDED
#define TEST_PV_AUTHOR_ENGINE_LOGGER_H_INCLUDED

#ifndef OSCL_TYPES_H_INCLUDED
#include "oscl_types.h"
#endif

#ifndef OSCL_DEFALLOC_H_INCLUDED
#include "oscl_defalloc.h"
#endif

//===================================================================
//CLASS:       PVAELOGGER
//===================================================================
// Initialize logging nodes
//===================================================================
// Format of logging configuration file:
// A hash mark (#) is used to indicate a comment line which will not
// be parsed.
// First line can be NO_LOGGING in which case no logs are created
// and the rest of the file is skipped
// By default logging output is directed to stderr (non-Windows) or
// ATL Trace (Win32/WinMobile). If LOGTOFILE is specified on the first
// line, output is redirected into a new file PVAUTHORLOG.TXT using a
// text file appender. If LOGTOMEM is specified on the first line,
// the output is still redirected to the file PVAUTHORLOG.TXT but using
// a memory appender.
// All other lines must be of the format NODENAME;LOGLEVEL where
// nodename is the case-sensitive name of a node such as EngineHandler
// or PVPlayer. Use ALLNODES if you want all nodes to be logged.
// LOGLEVEL is one of :
// PVLOGMSG_EMERG
// PVLOGMSG_ALERT
// PVLOGMSG_CRIT
// PVLOGMSG_ERR
// PVLOGMSG_WARNING
// PVLOGMSG_NOTICE
// PVLOGMSG_INFO
// PVLOGMSG_STACK_TRACE
//===================================================================
// Example:
//
// # This is a sample configuration
// LOGTOFILE
// PVAuthorEngine;PVLOGMSG_DEBUG
// PVAuthorEngine;PVLOGMSG_WARNING

class PVAELogger
{
    public:
        // Open pwszConfigFile to read nodes (and levels) to log
        // Returns true on success
        OSCL_IMPORT_REF static bool ParseConfigFile(const oscl_wchar *pwszConfigFile);

        // Attach a StdErrAppender to "PVAtuhorEngine" at level PVLOGMSG_DEBUG
        // Returns true on success
        OSCL_IMPORT_REF static bool MakeStdErrAppender();

        OSCL_EXPORT_REF static int32 CreateTestAppender(char *aBuf, PVLoggerAppender *&aAppender, OsclRefCounter *&aRefCounter, OsclSharedPtr<PVLoggerAppender> &aSharedAppenderPtr);
};

//===================================================================
//CLASS:       LogAppenderDestructDealloc
//===================================================================
// For smart ptrs call delete when done (used by LogAppender)
template<class DestructClass>
class LogAppenderDestructDealloc : public OsclDestructDealloc
{
    public:
        virtual void destruct_and_dealloc(OsclAny *ptr)
        {
            delete((DestructClass*)ptr);
        }
};


#endif //TEST_PV_AUTHOR_ENGINE_LOGGER_H_INCLUDED
