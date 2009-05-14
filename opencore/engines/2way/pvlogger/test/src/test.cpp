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

#include "stdio.h"
#include "oscl_base.h"
#include "oscl_mem.h"
#include "oscl_mem_audit.h"
#include "oscl_error.h"
#include "osclconfig.h"
#include "oscl_string.h"
#include "oscl_str_ptr_len.h"
#include "oscl_scheduler.h"
#include "oscl_utf8conv.h"
#define OSCL_DISABLE_WARNING_CPLUSPLUS_EXC_HANDLER_USED
#include "osclconfig_compiler_warnings.h"
#include "oscl_file_io.h"
#include "pv_logger_impl.h"
#include "pvlogger.h"

#define CONFIG_FILE_PATH _STRLIT("")

void CleanUp();
void Mod1();
void Mod2();
void Mod3();
void DoLogging();

int main()
{
    OsclBase::Init();
    OsclMem::Init();
    OsclErrorTrap::Init();
    PVLogger::Init();
    PVLoggerConfigFile *obj = new PVLoggerConfigFile();
    obj->SetConfigFilePath(CONFIG_FILE_PATH);
    if (obj->IsLoggerConfigFilePresent())
    {
        obj->SetLoggerSettings(NULL, _STRLIT("pvlog.txt"));
    }
    else
    {
        delete obj;
        CleanUp();
        return 1;
    }

    DoLogging();

    delete obj;
    CleanUp();
    return 0;
}

void CleanUp()
{
    PVLogger::Cleanup();
    OsclErrorTrap::Cleanup();
    OsclMem::Cleanup();
    OsclBase::Cleanup();
}

void DoLogging()
{
    Mod1();
    Mod2();
    Mod3();
}

void Mod1()
{
    PVLogger *logger = PVLogger::GetLoggerObject("mod1");
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, logger, PVLOGMSG_ALERT, (1, "Test Logging of module 1"));
}

void Mod2()
{
    PVLogger *logger = PVLogger::GetLoggerObject("mod2");
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, logger, PVLOGMSG_STACK_TRACE, (1, "Test Logging of module 2"));
}

void Mod3()
{
    PVLogger *logger = PVLogger::GetLoggerObject("mod3");
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, logger, PVLOGMSG_DEBUG, (1, "Test Logging of module 3"));
}
