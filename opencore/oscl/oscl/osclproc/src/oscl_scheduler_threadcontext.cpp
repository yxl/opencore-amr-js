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


#include "osclconfig_proc.h"
#include "oscl_scheduler_threadcontext.h"
#include "oscl_scheduler.h"
#include "oscl_error.h"
#include "oscl_error_imp.h"
#include "oscl_thread.h"

OSCL_EXPORT_REF PVThreadContext::PVThreadContext()
{
    iOpen = false;
    iScheduler = NULL;
}

OSCL_EXPORT_REF PVThreadContext::~PVThreadContext()
{
}

void PVThreadContext::LeaveIfWrongThread(PVThreadContext &a)
//static routine to verify thread context.
{
    if (!a.IsSameThreadContext())
        OsclError::Leave(OsclErrThreadContextIncorrect);
}

OSCL_EXPORT_REF bool PVThreadContext::IsSameThreadContext()
{
    if (!iOpen)
        return false;//unknown

    //check calling thread context against
    //this one.
    TOsclThreadId id;
    int32 result = OsclThread::GetId(id);
    if (result != OsclProcStatus::SUCCESS_ERROR)
        OsclError::Leave(OsclErrSystemCallFailed);

    return OsclThread::CompareId(id, iThreadId);
}

OSCL_EXPORT_REF void PVThreadContext::EnterThreadContext()
{
    //Save thread ID.
    int32 result = OsclThread::GetId(iThreadId);
    if (result != OsclProcStatus::SUCCESS_ERROR)
        OsclError::Leave(OsclErrSystemCallFailed);


    //Set current thread scheduler.
    iScheduler = OsclExecScheduler::GetScheduler();

    if (!iScheduler)
    {
        //There must be a pv scheduler.
        OsclError::Leave(OsclErrNotInstalled);
    }

    iOpen = true;
}

OSCL_EXPORT_REF void PVThreadContext::ExitThreadContext()
{
    iScheduler = NULL;
    iOpen = false;
}

void PVThreadContext::PendComplete(PVActiveBase *pvbase, int32 aReason, TPVThreadContext aCallingContext)
{
    //request can be completed from any thread.
    if (!iOpen)
        OsclError::Leave(OsclErrInvalidState);//thread context unknown
    //status can be anything but 'pending'
    if (aReason == OSCL_REQUEST_PENDING)
        OsclError::Leave(OsclErrInvalidState);//bad request status


    //use PV scheduler mechanism to complete requests.

    //first determine the calling context if possible.
    if (aCallingContext == EPVThreadContext_Undetermined && IsSameThreadContext())
        iScheduler->PendComplete(pvbase, aReason, EPVThreadContext_InThread);
    else
        iScheduler->PendComplete(pvbase, aReason, aCallingContext);

}

OSCL_EXPORT_REF uint32 PVThreadContext::Id()
//static routine to get current context.
{
    TOsclThreadId id;
    int32 result = OsclThread::GetId(id);
    if (result != OsclProcStatus::SUCCESS_ERROR)
        OsclError::Leave(OsclErrSystemCallFailed);
    return (uint32)id;
}

/**
// a static utility to tell whether the calling
// thread has any scheduler-- either Oscl scheduler
// or native scheduler.
*/
OSCL_EXPORT_REF bool PVThreadContext::ThreadHasScheduler()
{
    if (OsclExecScheduler::Current() != NULL)
        return true;
    return false;
}


