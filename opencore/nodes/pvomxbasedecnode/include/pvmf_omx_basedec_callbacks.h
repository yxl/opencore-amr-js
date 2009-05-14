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
#ifndef PVMF_OMX_BASEDEC_CALLBACKS_H_INCLUDED
#define PVMF_OMX_BASEDEC_CALLBACKS_H_INCLUDED

#ifndef THREADSAFE_CALLBACK_AO_H_INCLUDED
#include "threadsafe_callback_ao.h"
#endif

#ifndef THREADSAFE_MEMPOOL_H_INCLUDED
#include "threadsafe_mempool.h"
#endif

#ifndef OMX_Types_h
#include "OMX_Types.h"
#endif

#ifndef OMX_Core_h
#include "OMX_Core.h"
#endif

#ifndef OSCL_MEM_MEMPOOL_H_INCLUDED
#include "oscl_mem_mempool.h"
#endif
// structure that contains EventHandler callback type parameters:
typedef struct EventHandlerSpecificData
{
    OMX_HANDLETYPE hComponent;
    OMX_PTR pAppData;
    OMX_EVENTTYPE eEvent;
    OMX_U32 nData1;
    OMX_U32 nData2;
    OMX_PTR pEventData;
} EventHandlerSpecificData;


// structure that contains EmptyBufferDone callback type parameters:
typedef struct EmptyBufferDoneSpecificData
{
    OMX_HANDLETYPE hComponent;
    OMX_PTR pAppData;
    OMX_BUFFERHEADERTYPE* pBuffer;
} EmptyBufferDoneSpecificData;


// structure that contains FillBufferDone callback type parameters:
typedef struct FillBufferDoneSpecificData
{
    OMX_HANDLETYPE hComponent;
    OMX_PTR pAppData;
    OMX_BUFFERHEADERTYPE* pBuffer;
} FillBufferDoneSpecificData;


// This class defines the callback AO that handles the callbacks from the remote thread in a thread-safe way.
// The callback events arriving from the remote thread are queued and processed later in PV thread context.
// The class is DERIVED from the "ThreadSafeCallbackAO" class defined in "threadsafe_callback_ao.h/cpp"
// OVERLOAD THE METHOD : "ProcessEvent" so that it does something meaningful


// Test AO receives the remote thread specific API callback and then calls the generic API "ReceiveEvent"
const char EventHandlerAOName[] = "EventHandlerCallbackAO";
const char EmptyBufferDoneAOName[] = "EventHandlerCallbackAO";
const char FillBufferDoneAOName[] = "EventHandlerCallbackAO";


/**************** CLASS FOR EVENT HANDLER *************/
class EventHandlerThreadSafeCallbackAO : public ThreadSafeCallbackAO
{
    public:
        // Constructor
        EventHandlerThreadSafeCallbackAO(
            void* aObserver = NULL,
            uint32 aDepth = DEFAULT_QUEUE_DEPTH,
            const char* aAOname = EventHandlerAOName,
            int32 aPriority = OsclActiveObject::EPriorityNominal);


        // OVERLOADED ProcessEvent
        OsclReturnCode ProcessEvent(OsclAny* EventData);

        // overloaded Run and DeQueue to optimize performance (and process more than 1 event per Run)
        virtual void Run();
        virtual OsclAny* DeQueue(OsclReturnCode &stat);

        virtual ~EventHandlerThreadSafeCallbackAO();
        ThreadSafeMemPoolFixedChunkAllocator *iMemoryPool;

};


/**************** CLASS FOR EVENT HANDLER *************/
class EmptyBufferDoneThreadSafeCallbackAO : public ThreadSafeCallbackAO
{
    public:
        // Constructor
        EmptyBufferDoneThreadSafeCallbackAO(
            void* aObserver = NULL,
            uint32 aDepth = DEFAULT_QUEUE_DEPTH,
            const char* aAOname = EmptyBufferDoneAOName,
            int32 aPriority = OsclActiveObject::EPriorityNominal);

        // OVERLOADED ProcessEvent
        OsclReturnCode ProcessEvent(OsclAny* EventData);

        // overloaded Run and DeQueue to optimize performance (and process more than 1 event per Run)
        virtual void Run();
        virtual OsclAny* DeQueue(OsclReturnCode &stat);

        virtual ~EmptyBufferDoneThreadSafeCallbackAO();
        ThreadSafeMemPoolFixedChunkAllocator *iMemoryPool;
};




/**************** CLASS FOR EVENT HANDLER *************/
class FillBufferDoneThreadSafeCallbackAO : public ThreadSafeCallbackAO
{
    public:
        // Constructor
        FillBufferDoneThreadSafeCallbackAO(void* aObserver = NULL,
                                           uint32 aDepth = DEFAULT_QUEUE_DEPTH,
                                           const char* aAOname = FillBufferDoneAOName,
                                           int32 aPriority = OsclActiveObject::EPriorityNominal);

        // OVERLOADED ProcessEvent
        OsclReturnCode ProcessEvent(OsclAny* EventData);
        // overloaded Run and DeQueue to optimize performance (and process more than 1 event per Run)
        virtual void Run();
        virtual OsclAny* DeQueue(OsclReturnCode &stat);

        virtual ~FillBufferDoneThreadSafeCallbackAO();
        ThreadSafeMemPoolFixedChunkAllocator *iMemoryPool;
};

#endif	//#ifndef PVMF_OMX_BASEDEC_CALLBACKS_H_INLCUDED
