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

#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif

#ifndef PVLOGGER_FILE_APPENDER_H_INCLUDED
#include "pvlogger_file_appender.h"
#endif

#ifndef __UNIT_TEST_TEST_ARGS__
#include "unit_test_args.h"
#endif

#ifndef OSCL_UTF8CONV_H
#include "oscl_utf8conv.h"
#endif

#ifndef OSCL_STRING_UTILS_H_INCLUDED
#include "oscl_string_utils.h"
#endif

#include "alloc_dealloc_test.h"

void alloc_dealloc_test::test()
{
    fprintf(fileoutput, "Start alloc dealloc test, proxy %d.\n", iUseProxy);
    int error = 0;

    scheduler = OsclExecScheduler::Current();

    this->AddToScheduler();

    CPV2WayInterface *temp = NULL;

    if (iUseProxy)
    {

        OSCL_TRY(error, temp = CPV2WayProxyFactory::CreateTerminal(iSIP ? PV_SIP : PV_324M,
                               (PVCommandStatusObserver *) this,
                               (PVInformationalEventObserver *) this,
                               (PVErrorEventObserver *) this));

    }
    else
    {
        OSCL_TRY(error, temp = CPV2WayEngineFactory::CreateTerminal(iSIP ? PV_SIP : PV_324M,
                               (PVCommandStatusObserver *) this,
                               (PVInformationalEventObserver *) this,
                               (PVErrorEventObserver *) this));
    }

    if (error)
    {
        test_is_true(false);
        this->RemoveFromScheduler();
        return;
    }

    test_is_true(true);

    if (iUseProxy)
    {
        CPV2WayProxyFactory::DeleteTerminal(temp);
    }
    else
    {
        CPV2WayEngineFactory::DeleteTerminal(temp);
    }

    this->RemoveFromScheduler();
    return;
}


void alloc_dealloc_test::Run()
{
}

void alloc_dealloc_test::DoCancel()
{
}

void alloc_dealloc_test::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    OSCL_UNUSED_ARG(aEvent);
}

void alloc_dealloc_test::CommandCompleted(const PVCmdResponse& aResponse)
{
    OSCL_UNUSED_ARG(aResponse);
}











