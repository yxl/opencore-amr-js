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

#include "threadsafe_queue.h"

//Only do sem wait on pre-emptive thread OS.
//The reason is that sem wait is not always available on
//non-pre-emptive OS (such as under Brew applet thread)
#if !(OSCL_HAS_NON_PREEMPTIVE_THREAD_SUPPORT)
#define USE_SEM_WAIT 1
#else
#define USE_SEM_WAIT 0
#endif

OSCL_EXPORT_REF ThreadSafeQueue::ThreadSafeQueue()
        : OsclActiveObject(OsclActiveObject::EPriorityNominal, "ThreadSafeQueue")
{
    iObserver = NULL;
    iCounter = 1;
    if (OsclThread::GetId(iThreadId) != OsclProcStatus::SUCCESS_ERROR)
        OsclError::Leave(OsclErrSystemCallFailed);
#if USE_SEM_WAIT
    iQueueReadySem.Create();
#endif
    iQueueMut.Create();
    AddToScheduler();
    PendForExec();
    iQueueReadySem.Signal();
}

OSCL_EXPORT_REF ThreadSafeQueue::~ThreadSafeQueue()
{
    RemoveFromScheduler();
#if USE_SEM_WAIT
    iQueueReadySem.Close();
#endif
    iQueueMut.Close();
}

OSCL_EXPORT_REF bool ThreadSafeQueue::IsInThread()
{
    TOsclThreadId id;
    if (OsclThread::GetId(id) == OsclProcStatus::SUCCESS_ERROR)
    {
        return OsclThread::CompareId(id, iThreadId);
    }
    return false;
}

OSCL_EXPORT_REF void ThreadSafeQueue::Configure(ThreadSafeQueueObserver* aObs, uint32 aReserve, uint32 aId)
{
    iQueueMut.Lock();
    iObserver = aObs;
    iQueue.reserve(aReserve);
    iCounter = aId;
    iQueueMut.Unlock();
}

OSCL_EXPORT_REF ThreadSafeQueueId ThreadSafeQueue::AddToQueue(OsclAny *EventData, ThreadSafeQueueId* aId)
{
    iQueueMut.Lock();
    uint32 count = (aId) ? *aId : ++iCounter;
    ThreadSafeQueueElement elem(count, EventData);
    iQueue.push_back(elem);
    uint32 size = iQueue.size();
    iQueueMut.Unlock();

    //Signal the AO.  Only signal when the queue was previously empty in order
    // to minimize the amount of blocking in this call.
    if (size == 1)
    {
#if USE_SEM_WAIT
        //Wait on the AO to be ready to be signaled.
        iQueueReadySem.Wait();
        PendComplete(OSCL_REQUEST_ERR_NONE);
#else
        //To avoid problems under brew applet, don't do a sem wait here.
        //instead just check AO status and signal if needed.  It should
        //not be possible to lose data, since the only time AO is *not* ready
        //to be signaled is when a notification is already pending.
        //The reason to not do this in all platforms is that Status() call
        //is not thread-safe, but on non-preemptive OS it's ok here.
        if (Status() == OSCL_REQUEST_PENDING)
            PendComplete(OSCL_REQUEST_ERR_NONE);
#endif
    }

    return count;
}

OSCL_EXPORT_REF uint32 ThreadSafeQueue::DeQueue(ThreadSafeQueueId& aId, OsclAny*& aData)
{
    uint32 num = 0;
    iQueueMut.Lock();
    if (iQueue.size())
    {
        aId = iQueue[0].iId;
        aData = iQueue[0].iData;
        iQueue.erase(&iQueue[0]);
        num++;
    }
    iQueueMut.Unlock();
    return num;
}

void ThreadSafeQueue::Run()
{
    iQueueMut.Lock();
    PendForExec();
#if USE_SEM_WAIT
    iQueueReadySem.Signal();
#endif
    uint32 count = iQueue.size();
    ThreadSafeQueueObserver* obs = iObserver;
    iQueueMut.Unlock();

    //note: don't do the callback under the lock, in order to allow
    //de-queueing the data in the callback.  this creates the possibility
    //that queue size may not equal "count" in the callback.
    if (count && obs)
        obs->ThreadSafeQueueDataAvailable(this);
}




