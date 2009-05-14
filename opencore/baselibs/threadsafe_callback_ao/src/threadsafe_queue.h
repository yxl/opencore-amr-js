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
#ifndef THREADSAFE_QUEUE_H_INCLUDED
#define THREADSAFE_QUEUE_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef OSCL_BASE_ALLOC_H_INCLUDED
#include "oscl_base_alloc.h"
#endif

#ifndef OSCL_SCHEDULER_AO_H_INCLUDED
#include "oscl_scheduler_ao.h"
#endif

#ifndef OSCL_SEMAPHORE_H_INCLUDED
#include "oscl_semaphore.h"
#endif

#ifndef OSCL_MUTEX_H_INCLUDED
#include "oscl_mutex.h"
#endif

#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif

class ThreadSafeQueue;
class ThreadSafeQueueObserver
{
    public:
        /*
        ** Observer receives this callback when data has been added to the queue.
        ** There may be multiple data available when this is called.  There will
        ** not necessarily be one notice per item, so the observer should process
        ** the entire queue when it receives this notice.
        **
        ** param (in) aQueue: originating queue, in case multiple queues share an observer.
        */
        OSCL_IMPORT_REF virtual void ThreadSafeQueueDataAvailable(ThreadSafeQueue* aQueue) = 0;
};

/*
** A thread-safe queue.
** This queue resides in a thread that has a scheduler.
** Any thread(s) can add data.
** The queue will call ThreadSafeQueueDataAvailable from the owner thread
** when data has been added to the queue.
*/

typedef uint32 ThreadSafeQueueId;

class ThreadSafeQueue: public OsclActiveObject
{
    public:

        OSCL_IMPORT_REF ThreadSafeQueue();
        OSCL_IMPORT_REF virtual ~ThreadSafeQueue();

        /*
        ** Configure
        ** @param (in) aObs: observer
        ** @param (in) aReserve: queue reserve
        ** @param (in) aId: initial value for the assigned data IDs.
        */
        OSCL_IMPORT_REF void Configure(ThreadSafeQueueObserver* aObs, uint32 aReserve = 1, uint32 aId = 0);

        /*
        ** Add data to the queue from any thread.
        ** @param (in) aData: item to queue
        ** @param (in) aId: optional command ID.  If none is input, then one will be
        **   generated from the internal counter.
        ** @return: a unique ID for the data.
        */
        OSCL_IMPORT_REF ThreadSafeQueueId AddToQueue(OsclAny *aData, ThreadSafeQueueId* aId = NULL);

        /*
        ** DeQueue data from any thread.  Data is returned in FIFO order.
        ** @param (out) aId: the ID that was returned by AddToQueue.
        ** @param (out) aData: the queued item.
        ** @return: number of items de-queued (either 0 or 1)
        */
        OSCL_IMPORT_REF uint32 DeQueue(ThreadSafeQueueId& aId, OsclAny*& aData);

        /*
        ** Checks whether calling context is the same as the queue thread context.
        ** @return true if same thread context.
        */
        OSCL_IMPORT_REF bool IsInThread();

    protected:

        class ThreadSafeQueueElement
        {
            public:
                ThreadSafeQueueElement(ThreadSafeQueueId aId, OsclAny* aData)
                        : iId(aId)
                        , iData(aData)
                {}
                ThreadSafeQueueElement(const ThreadSafeQueueElement& aElem)
                        : iId(aElem.iId)
                        , iData(aElem.iData)
                {}
                ThreadSafeQueueId iId;
                OsclAny *iData;
        };

        void Run();

        ThreadSafeQueueObserver *iObserver;

        OsclSemaphore iQueueReadySem;//this sem is signaled when it's OK to signal the AO.
        OsclNoYieldMutex iQueueMut; //data lock

        Oscl_Vector<ThreadSafeQueueElement, _OsclBasicAllocator> iQueue;

        uint32 iCounter;//data ID counter

        TOsclThreadId iThreadId;

        void Lock();
        void Unlock();
};

#endif
