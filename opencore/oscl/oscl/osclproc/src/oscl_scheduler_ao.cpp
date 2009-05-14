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


#include "oscl_scheduler_ao.h"
#include "oscl_error.h"
#include "oscl_scheduler.h"
#include "pvlogger.h"
#include "oscl_tickcount.h"

#define OSCL_DISABLE_WARNING_THIS_USED_IN_BASE_CLASS
#define OSCL_DISABLE_WARNING_TYPEDEF_USED_AS_SYNONYM
#include "osclconfig_compiler_warnings.h"

#include "oscl_scheduler_tuneables.h"

/////////////////////
// PVActiveBase
/////////////////////

PVActiveBase::PVActiveBase(const char name[],
                           int32 pri)
{
    iName.Set(name);
#if(PV_SCHED_ENABLE_AO_STATS)
    iPVActiveStats = NULL;
#endif
    iPVReadyQLink.iAOPriority = pri;
    iBusy = false;
    iStatus = OSCL_REQUEST_ERR_NONE;
}

PVActiveBase::~PVActiveBase()
{
    if (iBusy)
        OsclError::Leave(OsclErrInvalidState);//EExecStillReadyOnDestruct
}

#if(PV_SCHED_ENABLE_AO_STATS)
PVActiveStats::PVActiveStats(OsclExecSchedulerCommonBase* aScheduler, const char* aAOName, PVActiveBase* aActiveBase)
{
    OSCL_ASSERT(aScheduler);

    iPVActiveBase = aActiveBase;
    iAOName = aAOName;
    iScheduler = aScheduler;

    iNumRun = 0;
    iNumRunError = 0;
    iMaxTicksInRun = 0;
    iTotalTicksInRun = 0;
    i64Valid = true;//make 64-bit the default.
    i64TotalTicksInRun = 0;
    iPercent = 0.0;
    iLeave = OsclErrNone;
    iNumCancel = 0;
    iNumInstances = 1;
    iPriority = (aActiveBase) ? aActiveBase->iPVReadyQLink.iAOPriority : 0;
}
PVActiveStats::~PVActiveStats()
{
    iPVStatQLink.Remove();

    //destroy the link from the AO to this object
    if (iPVActiveBase)
        iPVActiveBase->iPVActiveStats = NULL;
}
void PVActiveStats::Combine(PVActiveStats& aStats)
{
    iNumRun += aStats.iNumRun;
    iNumRunError += aStats.iNumRunError;
    iTotalTicksInRun += aStats.iTotalTicksInRun;
    OSCL_ASSERT(i64Valid == aStats.i64Valid);
    i64TotalTicksInRun += aStats.i64TotalTicksInRun;
    iNumCancel += aStats.iNumCancel;
    if (aStats.iLeave != OsclErrNone)
        iLeave = aStats.iLeave;
    iNumInstances++;
}
#endif //#if(PV_SCHED_ENABLE_AO_STATS)


void PVActiveBase::AddToScheduler()
{
    iThreadContext.EnterThreadContext();
    if (iThreadContext.iScheduler)
    {
        iAddedNum = iThreadContext.iScheduler->iNumAOAdded++;

#if(PV_SCHED_ENABLE_AO_STATS)
        //add to PV stat Q
        if (!iPVActiveStats)
        {
            OsclAny* ptr = iThreadContext.iScheduler->iAlloc->allocate(sizeof(PVActiveStats));
            OsclError::LeaveIfNull(ptr);
            iPVActiveStats = OSCL_PLACEMENT_NEW(ptr, PVActiveStats(iThreadContext.iScheduler, (char*)iName.Str(), this));
            iThreadContext.iScheduler->iPVStatQ.InsertTail(*iPVActiveStats);
            //note: this memory is cleaned up in CleanupStatQ when the scheduler
            //exits.
        }
#endif
    }
}

void PVActiveBase::RemoveFromScheduler()
{
    if (IsAdded())
    {
        if (iBusy)
            Cancel();
        //no additional de-queueing is needed-- once AOs are
        //canceled they're not in any queues.
    }

    iThreadContext.ExitThreadContext();

#if(PV_SCHED_ENABLE_AO_STATS)
    //destroy the link from the stats object back to this object.
    //this means that a new stats object will be created if this AO
    //is added to this or another scheduler again.
    if (iPVActiveStats)
    {
        iPVActiveStats->iPVActiveBase = NULL;
        //since we've unlinked this to the PVActiveStats object,
        //we must prevent accessing the object again since it could
        //get deleted.
        iPVActiveStats = NULL;
    }
#endif
}

void PVActiveBase::Destroy()
//common AO cleanup.
{
    RemoveFromScheduler();
}

void PVActiveBase::Activate()
//Activate an AO's request.  This only does the portion of the
//activation that is common to both timers and non-timers.
{

    //mimic standard symbian panics.
    if (iBusy)
        OsclError::Leave(OsclErrInvalidState);//EExecAlreadyActive
    if (!iThreadContext.iOpen)
        OsclError::Leave(OsclErrInvalidState);//EExecNotAdded

#if PV_SCHED_ENABLE_THREAD_CONTEXT_CHECKS
    PVThreadContext::LeaveIfWrongThread(iThreadContext);
#endif

    iBusy = true;

    //caller will activate the timer.  For
    //non-timers, nothing else is needed.

}

OSCL_EXPORT_REF bool PVActiveBase::IsAdded() const
{
    return iThreadContext.iOpen;
}


void PVActiveBase::Cancel()
{
    if (iBusy)
    {
#if PV_SCHED_ENABLE_THREAD_CONTEXT_CHECKS
        //require same thread context for cancel calls,
        //since we'll be calling the DoCancel routine.
        PVThreadContext::LeaveIfWrongThread(iThreadContext);
#endif

        //call the cancel handler-- this should
        //complete the request if needed.
        //Note: symbian calls the DoCancel even
        //if the request is already complete, so
        //I do the same in order to get the same
        //behavior.
#if(PV_SCHED_ENABLE_AO_STATS)
        iPVActiveStats->iNumCancel++;
#endif
        DoCancel();

        //wait for request to cancel.
        iThreadContext.iScheduler->RequestCanceled(this);
    }
}

/////////////////////
// OsclActiveObject
/////////////////////

OSCL_EXPORT_REF OsclActiveObject::OsclActiveObject(int32 aPriority, const char name[]):
        PVActiveBase(name, aPriority)
{
    iStatus = OSCL_REQUEST_ERR_NONE;
}

OSCL_EXPORT_REF OsclActiveObject::~OsclActiveObject()
{
    //"typically, a derived class calls Cancel in its
    //destructor"
    Cancel();
    PVActiveBase::Destroy();
}

OSCL_EXPORT_REF void OsclActiveObject::PendComplete(int32 aStatus)
{
    iThreadContext.PendComplete(this, aStatus, EPVThreadContext_Undetermined);
}

OSCL_EXPORT_REF void OsclActiveObject::AddToScheduler()
{
    PVActiveBase::AddToScheduler();
}


OSCL_EXPORT_REF void OsclActiveObject::RemoveFromScheduler()
{
    PVActiveBase::RemoveFromScheduler();
}

OSCL_EXPORT_REF void OsclActiveObject::SetBusy()
//Need this overload to prevent anyone from using
//OsclActiveObject::SetActive directly on systems that have
//that method (eg Symbian)
{
    Activate();
    //nothing else needed.
}

OSCL_EXPORT_REF bool OsclActiveObject::IsBusy() const
//On systems with OsclActiveObj::IsActive, this function
//allows us to convert the return type to bool
{
    return iBusy;
}


OSCL_EXPORT_REF void OsclActiveObject::Cancel()
//Need this overload to prevent anyone from using
//OsclActiveObject::Cancel on systems that have that method (Symbian).
{
    PVActiveBase::Cancel();
}

OSCL_EXPORT_REF int32 OsclActiveObject::Priority() const
{
    return iPVReadyQLink.iAOPriority;
}

OSCL_EXPORT_REF int32 OsclActiveObject::Status() const
//get the AO status value.
{
    return iStatus.Value();
}

OSCL_EXPORT_REF OsclAOStatus& OsclActiveObject::StatusRef()
//get a ref to the AO status object.
{
    return iStatus;
}

OSCL_EXPORT_REF void OsclActiveObject::SetStatus(int32 s)
//set the AO status value.
{
    iStatus = s;
}

OSCL_EXPORT_REF void OsclActiveObject::PendForExec()
//activate the AO request.
{
    SetBusy();
    iStatus = OSCL_REQUEST_PENDING;
}

OSCL_EXPORT_REF void OsclActiveObject::RunIfNotReady()
//If the AO request is not active, activate and complete it.
{
    if (!IsBusy())
    {
        PendForExec();
        PendComplete(OSCL_REQUEST_ERR_NONE);
    }
}

OSCL_EXPORT_REF void OsclActiveObject::DoCancel()
//default request canceler for AOs
{
    if (iStatus == OSCL_REQUEST_PENDING)
        iThreadContext.PendComplete(this, OSCL_REQUEST_ERR_CANCEL, EPVThreadContext_InThread);
}

OSCL_EXPORT_REF int32 OsclActiveObject::RunError(int32 aError)
//default error handler for active objects.
{
    return aError;
}


/////////////////////
// OsclTimerObject
/////////////////////

OSCL_EXPORT_REF OsclTimerObject::OsclTimerObject(int32 aPriority, const char name[]):
        PVActiveBase(name, aPriority)
{
    SetStatus(OSCL_REQUEST_ERR_NONE);
}

OSCL_EXPORT_REF OsclTimerObject::~OsclTimerObject()
{
    //"typically, a derived class calls Cancel in its
    //destructor"
    Cancel();
    PVActiveBase::Destroy();
}

OSCL_EXPORT_REF void OsclTimerObject::AddToScheduler()
{
    iPVReadyQLink.iTimeToRunTicks = 0;

    PVActiveBase::AddToScheduler();
}


OSCL_EXPORT_REF void OsclTimerObject::RemoveFromScheduler()
{
    PVActiveBase::RemoveFromScheduler();

}

OSCL_EXPORT_REF void OsclTimerObject::After(int32 aDelayMicrosec)
//like CTimer::After.
{
    PVActiveBase::Activate();


    //Just put this AO in the scheduler timer queue-- the scheduler
    //will complete the request at the correct time.
    iStatus = OSCL_REQUEST_PENDING;
    iThreadContext.iScheduler->AddToExecTimerQ(this, aDelayMicrosec);

}

OSCL_EXPORT_REF void OsclTimerObject::RunIfNotReady(uint32 aDelayMicrosec)
//If the AO is not ready, start its timeout for completion.
{
    if (!IsBusy())

    {
        if (aDelayMicrosec > 0)
        {
            OsclTimerObject::After(aDelayMicrosec);
        }
        else
        {
            // If delay is 0, make ready and complete pend immediately, to avoid going through the timer queue
            SetBusy();
            SetStatus(OSCL_REQUEST_PENDING);
            if (IsAdded())
            {
                iThreadContext.PendComplete(this, OSCL_REQUEST_ERR_NONE, EPVThreadContext_InThread);
            }
        }
    }
}

OSCL_EXPORT_REF void OsclTimerObject::SetBusy()
//Need this overload to prevent anyone from using
//OsclActiveObject::SetActive on Symbian.
{
    PVActiveBase::Activate();

}

OSCL_EXPORT_REF bool OsclTimerObject::IsBusy() const
//needed to prevent using OsclActiveObject::IsActive, just to
//get correct return type bool instead of TBool.
{
    return iBusy;
}


OSCL_EXPORT_REF void OsclTimerObject::Cancel()
//Need this overload to prevent anyone from using
//OsclActiveObject::Cancel on Symbian.
{
    PVActiveBase::Cancel();
}

OSCL_EXPORT_REF int32 OsclTimerObject::Priority() const
{
    return iPVReadyQLink.iAOPriority;
}

OSCL_EXPORT_REF int32 OsclTimerObject::Status() const
{
    return iStatus.Value();
}

OSCL_EXPORT_REF OsclAOStatus& OsclTimerObject::StatusRef()
{
    return iStatus;
}

OSCL_EXPORT_REF void OsclTimerObject::SetStatus(int32 s)
{
    iStatus = s;
}

OSCL_EXPORT_REF void OsclTimerObject::DoCancel()
//default request canceler for timer objects.
{
    //cancel the pending timeout.
    if (iStatus == OSCL_REQUEST_PENDING)
        iThreadContext.iScheduler->PendComplete(this, OSCL_REQUEST_ERR_CANCEL, EPVThreadContext_InThread);
}

OSCL_EXPORT_REF int32 OsclTimerObject::RunError(int32 aError)
//default error handler for timer objects.
{
    return aError;
}



