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
#ifndef PV_THREADSAFE_CALLBACKAO_H_INCLUDED
#define PV_THREADSAFE_CALLBACKAO_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef OSCL_SCHEDULER_AO_H_INCLUDED
#include "oscl_scheduler_ao.h"
#endif

#ifndef OSCL_ERROR_CODES_H_INCLUDED
#include "oscl_error_codes.h"
#endif

#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif

#ifndef OSCL_SEMAPHORE_H_INCLUDED
#include "oscl_semaphore.h"
#endif
// definition of queue type and queue element

#define DEFAULT_QUEUE_DEPTH 3
typedef struct QElement QElement;
typedef struct QueueT QueueT;



const char thisAOname[] = "threadsafecallbackAO";

class ThreadSafeCallbackAO: public OsclActiveObject
{
    public:

        struct QElement
        {
            OsclAny *pData;
        };

        struct QueueT
        {
            QElement* pFirst; // queue head

            uint32 index_in;			// index of element to be queued next
            uint32 index_out;		// index of element to be de-queued next
            uint32 NumElem;         // Number of elements currently in the queue
            uint32 MaxNumElements;  // Maximum number of queue elements (i.e, queue depth)
        };
        // constructor
        OSCL_IMPORT_REF ThreadSafeCallbackAO(void *aObserver = NULL, uint32 aDepth = DEFAULT_QUEUE_DEPTH, const char *aAOname = thisAOname, int32 aPriority = OsclActiveObject::EPriorityNominal);

        // desctructor
        OSCL_IMPORT_REF virtual ~ThreadSafeCallbackAO();


        OSCL_IMPORT_REF void ThreadLogon();
        OSCL_IMPORT_REF void ThreadLogoff();

        OSCL_IMPORT_REF OsclReturnCode ReceiveEvent(OsclAny *EventData); // Generic callback API
        OSCL_IMPORT_REF virtual OsclReturnCode ProcessEvent(OsclAny *EventData); // Process data

    protected:

        OSCL_IMPORT_REF virtual void Run();

        PVLogger* iLogger;
        const char *iLoggerString;

        void *iObserver;
        OsclSemaphore RemoteThreadCtrlSema; // Semaphore that blocks the remote thread when the event queue is full
        OsclMutex Mutex; // Mutex that controls access to the event queue
        QueueT *Q; // queue of events

        // QUEUE OPERATIONS:
        OSCL_IMPORT_REF OsclReturnCode QueueInit(uint32 aMaxQueueDepth);
        OSCL_IMPORT_REF OsclReturnCode QueueDeInit();
        OSCL_IMPORT_REF OsclReturnCode Queue(OsclAny *); // queue an element
        OSCL_IMPORT_REF virtual OsclAny *DeQueue(OsclReturnCode &status); // dequeue element and return a pointer to the element. return the status
        OSCL_IMPORT_REF uint32 GetQueueNumElem(); // number of elements in the queue

};

#endif
