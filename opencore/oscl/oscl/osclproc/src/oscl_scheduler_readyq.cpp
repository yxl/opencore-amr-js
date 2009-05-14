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


#include "oscl_scheduler_readyq.h"


#include "oscl_scheduler_ao.h"
#include "oscl_scheduler.h"
#include "oscl_tickcount.h"
#include "oscl_scheduler_tuneables.h"

////////////////////////////////////////
// Ready Queue & Timer Queue Support
////////////////////////////////////////

//scheduler has an allocator but there's no way to use
//it here since this allocator must be a template argument
//to oscl priority queue.
OsclAny* OsclReadyAlloc::allocate_fl(const uint32 size, const char * file_name, const int line_num)
{
    OsclAny*p = iBasicAlloc.allocate_fl(size, file_name, line_num);
    OsclError::LeaveIfNull(p);
    return p;
}
OsclAny* OsclReadyAlloc::allocate(const uint32 size)
{
    OsclAny*p = iBasicAlloc.ALLOCATE(size);
    OsclError::LeaveIfNull(p);
    return p;
}
void OsclReadyAlloc::deallocate(OsclAny* p)
{
    iBasicAlloc.deallocate(p);
}

//evalute "priority of a is less than priority of b"
int OsclTimerCompare::compare(TOsclReady& a, TOsclReady& b)
{
    //Sort by time to run.  Earlier "time to run" has precedence.
    if (a->iPVReadyQLink.iTimeToRunTicks != b->iPVReadyQLink.iTimeToRunTicks)
    {
        //calculate a>b, taking possible rollover into account.
        uint32 delta = b->iPVReadyQLink.iTimeToRunTicks - a->iPVReadyQLink.iTimeToRunTicks;
        return (delta > OsclExecScheduler::iTimeCompareThreshold);
    }

    //Now sort by priority
    return OsclReadyCompare::compare(a, b);
}

//evalute "priority of a is less than priority of b"
int OsclReadyCompare::compare(TOsclReady& a, TOsclReady& b)
{
    //Sort by AO priority.  Higher priority has precedence.
    if (a->iPVReadyQLink.iAOPriority != b->iPVReadyQLink.iAOPriority)
        return (a->iPVReadyQLink.iAOPriority < b->iPVReadyQLink.iAOPriority);

    //if there was a priority tie, impose a FIFO order.

    //This section allows switching between "fair scheduling" and linear
    //behavior.  We always use fair scheduling, but for testing it can be helpful to
    //swap in the linear behavior.
#if PV_SCHED_FAIR_SCHEDULING
    //Sort by FIFO order, to create fair scheduling.
    //AOs that have been queued the longest have precedence.
    return (a->iPVReadyQLink.iSeqNum >= b->iPVReadyQLink.iSeqNum);
#else
    //Sort by the order when AO was added to scheduler, to simulate
    //Symbian native ActiveScheduler behavior.
    //AOs that were added earlier have precedence.
    return (a->iAddedNum > b->iAddedNum);
#endif

}

////////////////////////////////////////
//OsclReadyQ
////////////////////////////////////////
void OsclReadyQ::Construct(int nreserve)
{
    iSeqNumCounter = 0;
    if (nreserve > 0)
        c.reserve(nreserve);
    iCallback = NULL;
}

void OsclReadyQ::ThreadLogon()
{
    iSem.Create();
    iCrit.Create();
}

void OsclReadyQ::ThreadLogoff()
{
    iSem.Close();
    iCrit.Close();
}

//
//Note: all semaphore errors are fatal, since they can cause
// scheduler to spin or hang.
//

PVActiveBase* OsclReadyQ::WaitAndPopTop()
//block until an AO is ready and pop the highest pri AO.
{
    switch (iSem.Wait())
    {
        case OsclProcStatus::SUCCESS_ERROR:
            return PopTopAfterWait();

        default:
            OsclError::Leave(OsclErrSystemCallFailed);
            return NULL;
    }
}

PVActiveBase* OsclReadyQ::WaitAndPopTop(uint32 aTimeoutVal)
//block until an AO is ready or timeout is reached.
{
    switch (iSem.Wait(aTimeoutVal))
    {
        case OsclProcStatus::WAIT_TIMEOUT_ERROR:
            //timeout reached, no AO ready.
            return NULL;

        case OsclProcStatus::SUCCESS_ERROR:
            //an AO is ready-- pop it.
            return PopTopAfterWait();

        default:
            //no timeout and no AO.
            OsclError::Leave(OsclErrSystemCallFailed);
            return NULL;
    }
}

bool OsclReadyQ::IsIn(TOsclReady b)
//tell if elemement is in this q
{
    iCrit.Lock();
    bool isin = (b->iPVReadyQLink.iIsIn == this);
    iCrit.Unlock();
    return isin;
}

PVActiveBase* OsclReadyQ::PopTop()
//deque and return highest pri element.
{
    iCrit.Lock();

    PVActiveBase*elem = (size() > 0) ? top() : NULL;
    if (elem)
    {
        elem->iPVReadyQLink.iIsIn = NULL;

        pop();

        //this call won't block-- it will just decrement the sem.
        if (iSem.Wait() != OsclProcStatus::SUCCESS_ERROR)
            OsclError::Leave(OsclErrSystemCallFailed);
    }

    iCrit.Unlock();
    return elem;
}

PVActiveBase* OsclReadyQ::PopTopAfterWait()
//deque and return highest pri element.  used when we have
//already done a sem wait prior to this call, so there is no sem
//decrement here.
{
    iCrit.Lock();

    PVActiveBase*elem = (size() > 0) ? top() : NULL;

    if (elem)
    {
        elem->iPVReadyQLink.iIsIn = NULL;
        pop();
    }
    else
    {//there should always be an element available after a sem wait.
        OSCL_ASSERT(0);
    }

    iCrit.Unlock();

    return elem;
}

PVActiveBase* OsclReadyQ::Top()
//return highest pri element without removing.
{
    iCrit.Lock();
    PVActiveBase* elem = (size() > 0) ? top() : NULL;
    iCrit.Unlock();
    return elem;
}

void OsclReadyQ::Remove(TOsclReady a)
//remove the given element
{
    iCrit.Lock();

    a->iPVReadyQLink.iIsIn = NULL;

    int32 nfound = remove(a);

    if (nfound > 0)
    {
        //this call won't block-- it will just decrement the sem.
        if (iSem.Wait() != OsclProcStatus::SUCCESS_ERROR)
            OsclError::Leave(OsclErrSystemCallFailed);
    }

    iCrit.Unlock();
}

int32 OsclReadyQ::PendComplete(PVActiveBase *pvbase, int32 aReason)
//Complete an AO request
{
    iCrit.Lock();

    //make sure this AO is not already queued.
    int32 err;
    if (pvbase->IsInAnyQ())
    {
        err = OsclErrInvalidState;//EExecAlreadyAdded
    }
    //make sure the AO has a request active
    else if (!pvbase->iBusy
             || pvbase->iStatus != OSCL_REQUEST_PENDING)
    {
        err = OsclErrCorrupt;//EExecStrayEvent;
    }
    else
    {
        //Add to pri queue
        pvbase->iPVReadyQLink.iIsIn = this;
        pvbase->iPVReadyQLink.iTimeQueuedTicks = OsclTickCount::TickCount();
        pvbase->iPVReadyQLink.iSeqNum = ++iSeqNumCounter;//for the FIFO sort
        push(pvbase);

        //increment the sem
        if (iSem.Signal() != OsclProcStatus::SUCCESS_ERROR)
        {
            err = OsclErrSystemCallFailed;
        }
        else
        {
            //update the AO status
            pvbase->iStatus = aReason;

            //make scheduler callback if needed.
            //note: this needs to happen under the lock since we're updating
            //the callback pointer.
            if (iCallback)
            {
                iCallback->OsclSchedulerReadyCallback(iCallbackContext);
                iCallback = NULL;
            }
            err = OsclErrNone;
        }
    }

    iCrit.Unlock();
    return err;
}

int32 OsclReadyQ::WaitForRequestComplete(PVActiveBase* pvbase)
//Wait on a particular request to complete
{
    int32 nwait = 0;//count the number of request sem waits.

    for (bool complete = false;!complete;)
    {
        //Wait on any request to complete.
        if (iSem.Wait() != OsclProcStatus::SUCCESS_ERROR)
            return OsclErrSystemCallFailed;

        nwait++;

        //Some request was complete but it might not be the one of interest,
        //so check for completion again.
        complete = IsIn(pvbase);
    }

    //Restore the request semaphore value since we may have decremented it without
    //removing anything from the ReadyQ.
    while (nwait > 0)
    {
        if (iSem.Signal() != OsclProcStatus::SUCCESS_ERROR)
            return OsclErrSystemCallFailed;

        nwait--;
    }

    return OsclErrNone;
}

void OsclReadyQ::RegisterForCallback(OsclSchedulerObserver* aCallback, OsclAny* aCallbackContext)
{
    //Use the Q lock to avoid thread contention over
    //callback pointer.
    iCrit.Lock();

    //Callback right away if ready Q is non-empty.
    if (size() && aCallback)
    {
        iCallback = NULL;
        aCallback->OsclSchedulerReadyCallback(aCallbackContext);
    }
    else
    {
        //save the new pointers.  Callback will happen when timer Q or ready Q is
        //updated.
        iCallback = aCallback;
        iCallbackContext = aCallbackContext;
    }
    iCrit.Unlock();
}

void OsclReadyQ::TimerCallback(uint32 aDelayMicrosec)
//Inform scheduler observer of a change in the shortest timer interval
{
    //lock when updating callback pointer.
    iCrit.Lock();
    OsclSchedulerObserver* callback = iCallback;
    iCallback = NULL;
    iCrit.Unlock();

    //Timer callback happens outside the lock, to allow code
    //to register for additional callback.
    if (callback)
        callback->OsclSchedulerTimerCallback(iCallbackContext, aDelayMicrosec / 1000);
}

////////////////////////////////////////
//OsclTimerQ
////////////////////////////////////////
void OsclTimerQ::Construct(int nreserve)
{
    iSeqNumCounter = 0;
    if (nreserve > 0)
        c.reserve(nreserve);
}

bool OsclTimerQ::IsIn(TOsclReady b)
//tell if element is in this q
{
    return (b->iPVReadyQLink.iIsIn == this);
}

PVActiveBase* OsclTimerQ::PopTop()
//deque and return highest pri element.
{
    PVActiveBase*elem = (size() > 0) ? top() : NULL;
    if (elem)
        Pop(elem);

    return elem;
}

PVActiveBase* OsclTimerQ::Top()
//return highest pri element without removing.
{
    return (size() > 0) ? top() : NULL;
}

void OsclTimerQ::Pop(TOsclReady b)
//remove queue top.
{
    b->iPVReadyQLink.iIsIn = NULL;
    pop();
}

void OsclTimerQ::Remove(TOsclReady a)
{
    a->iPVReadyQLink.iIsIn = NULL;
    remove(a);
}

void OsclTimerQ::Add(TOsclReady b)
{
    b->iPVReadyQLink.iIsIn = this;
    b->iPVReadyQLink.iTimeQueuedTicks = OsclTickCount::TickCount();
    b->iPVReadyQLink.iSeqNum = ++iSeqNumCounter;//for the FIFO sort

    push(b);
}








