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

#include "threadsafe_callback_ao.h"
#include "pvlogger.h"
#include "oscl_procstatus.h"

#include "oscl_dll.h"

OSCL_DLL_ENTRY_POINT_DEFAULT()


/////////////////////////////////////////////////////////////////////////////
// Add AO to the scheduler
/////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void ThreadSafeCallbackAO::ThreadLogon()
{
    if (!IsAdded())
    {
        AddToScheduler();
    }

    iLogger = PVLogger::GetLoggerObject(iLoggerString);
}
////////////////////////////////////////////////////////////////////////////
// Remove AO from the scheduler
////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void ThreadSafeCallbackAO::ThreadLogoff()
{
    //thread logoff
    if (IsAdded())
    {
        RemoveFromScheduler();
    }

    iLogger = NULL;
}
///////////////////////////////////////////////////////////////////////////
// CONSTRUCTOR
///////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF ThreadSafeCallbackAO::ThreadSafeCallbackAO(void *aObserver, uint32 aDepth, const char *aAOname, int32 aPriority)
        : OsclActiveObject(aPriority, aAOname),
        iLogger(NULL)
{


    OsclReturnCode queue_init_status = OsclSuccess;
    OsclProcStatus::eOsclProcError mutex_init_status = OsclProcStatus::SUCCESS_ERROR;
    OsclProcStatus::eOsclProcError sema_init_status = OsclProcStatus::SUCCESS_ERROR;
    int32 err = 0;

    iLoggerString = aAOname;
    iObserver = aObserver;
    Q = NULL;

    OSCL_TRY(err,
             queue_init_status = QueueInit(aDepth);	//create the Q
             mutex_init_status = Mutex.Create(); // Create Mutex
             sema_init_status  = RemoteThreadCtrlSema.Create(aDepth);

             ThreadLogon(); // add to scheduler
            );

    if ((err != 0) ||
            (queue_init_status != OsclSuccess) ||
            (mutex_init_status != OsclProcStatus::SUCCESS_ERROR) ||
            (sema_init_status  != OsclProcStatus::SUCCESS_ERROR)
       )
    {
        OSCL_LEAVE(OsclFailure);
    }


    PendForExec(); // make sure to "prime" the callback AO for the first event that arrives

}

/////////////////////////////////////////////////////////////////////////////
// Destructor
/////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF ThreadSafeCallbackAO::~ThreadSafeCallbackAO()
{
    OsclReturnCode queue_deinit_status = OsclSuccess;
    OsclProcStatus::eOsclProcError mutex_close_status = OsclProcStatus::SUCCESS_ERROR;
    OsclProcStatus::eOsclProcError sema_close_status = OsclProcStatus::SUCCESS_ERROR;

    int32 err = 0;

    OSCL_TRY(err,
             queue_deinit_status = QueueDeInit(); // Destroy the queue
             mutex_close_status =  Mutex.Close(); // close the mutex
             sema_close_status  =  RemoteThreadCtrlSema.Close(); // close the semaphore
             ThreadLogoff();
            );

    if ((err != 0) ||
            (queue_deinit_status != OsclSuccess) ||
            (mutex_close_status != OsclProcStatus::SUCCESS_ERROR) ||
            (sema_close_status != OsclProcStatus::SUCCESS_ERROR))
    {
        OSCL_LEAVE(OsclFailure);
    }

    iObserver = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// Run Routine
/////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void ThreadSafeCallbackAO::Run()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "ThreadSafeCallbackAO::Run() In"));


    OsclAny *P; // parameter to dequeue
    OsclReturnCode status;


    // get the event parameters and status of de-queuing
    P = DeQueue(status);


    if (status == OsclSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "ThreadSafeCallbackAO::Run() - Calling Process Event"));
        ProcessEvent(P);
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "ThreadSafeCallbackAO::Run() - could not dequeue event data"));
    }



    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "ThreadSafeCallbackAO::Run() Out"));
}


/////////////////////////////////////////////////////////////////////////////////////
//////// ReceiveEvent
/////////////////////////////////////////////////////////////////////////////////////
/////// NOTE: THIS METHOD IS EXECUTED IN THE REMOTE THREAD CONTEXT
//////////////AS PART OF THE CALLBACK. IT QUEUES THE EVENT PARAMETERS !!!!!!!!!!!!
/////////////////////////////////////////////////////////////////////////////////////
///////////// GENERIC API requires only one parameter (osclany pointer to "event data")
///////////////
////////////// BEFORE CALLING THE GENERIC API, YOU MUST SETUP (COPY IF NECESSARY)
///////////////// CALLBACK EVENT PARAMETERS AND PROVIDE An OSCLANY POINTER TO THEM
/////////////////////////////////////////////////////////////////////////////////////
///////////////YOU MUST NOT USE LOGGER IN THE CALLBACK (IN THE REMOTE THREAD CONTEXT)
///////////////
//////////////// EXAMPLE CODE (For CALLING THE GENERIC API):

// typedef struct event_data{
//		int d1;
//		char *d2;
//		uint32 d3;
//		} event_data;

//int ActualCallbackAPI(int p1, char * p2, uint32 p3)
//{
//		// pack all parameters
//		event_data *pED = malloc(sizeof(event_data));
//		pED->d1 = p1;
//		pED->d2 = p2;
//		pED->d3 = p3;
//		OsclAny *P = (OsclAny *) pED:
//			// call the generic callback API:
//		PVMFStatus stat = ReceiveEvent(P);
//		// check the status
//		if(stat == PVMFSuccess)
//			return OK;
//		else
//			return ERROR;
//}
//



OSCL_EXPORT_REF OsclReturnCode ThreadSafeCallbackAO::ReceiveEvent(OsclAny *EventData)
{

    OsclReturnCode status;

    status = Queue(EventData);

    return status;
}

OSCL_EXPORT_REF OsclReturnCode ThreadSafeCallbackAO::ProcessEvent(OsclAny *EventData)
{
    OSCL_UNUSED_ARG(EventData);
// DO NOTHING. OVERLOAD THIS METHOD IN THE DERIVED CLASS TO DO SOMETHING MEANINGFUL
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "ThreadSafeCallbackAO::ProcessEvent() In and Out"));
    return OsclSuccess;
}
///////////////////////////////////////////////////////////////////////////////
//// QUEUE OPERATIONS
///////////////////////////////////////////////////////////////////////////////


/////CREATE QUEUE AND INIT ///////////////////

OSCL_EXPORT_REF OsclReturnCode ThreadSafeCallbackAO::QueueInit(uint32 aMaxQueueDepth)
{

    if (aMaxQueueDepth < 1)
    {
        Q = NULL;
        return OsclErrArgument;
    }

    // create Q as simple array
    Q = (QueueT *) oscl_malloc(sizeof(QueueT));
    if (Q == NULL)
    {
        return OsclErrNoMemory;
    }

    Q->MaxNumElements = aMaxQueueDepth;
    Q->NumElem = 0;
    Q->index_in = 0;
    Q->index_out = 0;
    Q->pFirst = NULL;
    // create the Q elements
    Q->pFirst = (QElement *) oscl_malloc((Q->MaxNumElements) * oscl_mem_aligned_size(sizeof(QElement)));
    if (Q->pFirst == NULL)
    {
        Q = NULL;
        return OsclErrNoMemory;
    }

    // zero out the Q
    oscl_memset(Q->pFirst, 0, (Q->MaxNumElements) * oscl_mem_aligned_size(sizeof(QElement)));


    return OsclSuccess;
}

///////////// DESTROY QUEUE ///////////////////////
OSCL_EXPORT_REF OsclReturnCode ThreadSafeCallbackAO::QueueDeInit()
{
    OsclReturnCode status = OsclSuccess;

    // free the Q elements
    if (Q->pFirst != NULL)
        oscl_free(Q->pFirst);

    Q->pFirst = NULL;
    // free the Q structure
    if (Q != NULL)
        oscl_free(Q);

    Q = NULL;

    return status;
}

/////////////QUEUE ONE ELEMENT //////////////////////
OSCL_EXPORT_REF OsclReturnCode ThreadSafeCallbackAO::Queue(OsclAny *pData)
{
    OsclProcStatus::eOsclProcError sema_status;

    // Wait on the remote thread control semaphore. If the queue is full, must block and wait
    // for the AO to dequeue some previous event. If the queue is not full, proceed
    sema_status = RemoteThreadCtrlSema.Wait();
    if (sema_status != OsclProcStatus::SUCCESS_ERROR)
        return OsclFailure;


    // protect queue access
    Mutex.Lock();

    if (Q->NumElem >= Q->MaxNumElements)
    {
        Mutex.Unlock();
        RemoteThreadCtrlSema.Signal(); // signal the sema to maintain sema count consistency in case
        // of error (inability to queue)
        return OsclFailure;
    }

    (Q->pFirst[Q->index_in]).pData = pData;

    Q->index_in++;
    // roll-over the index to 0 if it reaches the end.
    if (Q->index_in == Q->MaxNumElements)
        Q->index_in = 0;

    Q->NumElem++;

    // check if AO needs to be scheduled (i.e. check if this is the first event the queue after the queue was empty)
    if (GetQueueNumElem() == 1)
    {
        PendComplete(OSCL_REQUEST_ERR_NONE);
    }

    Mutex.Unlock();

    return OsclSuccess;
}

///////////DE-QUEUE ONE ELEMENT /////////////////////
OSCL_EXPORT_REF OsclAny* ThreadSafeCallbackAO::DeQueue(OsclReturnCode &stat)
{
    OsclAny *pData;
    OsclProcStatus::eOsclProcError sema_status;

    stat = OsclSuccess;

    // Protect the queue while accessing it:
    Mutex.Lock();

    if (Q->NumElem == 0)
    {
        // nothing to de-queue
        stat = OsclFailure;
        Mutex.Unlock();

        return NULL;
    }

    pData = (Q->pFirst[Q->index_out]).pData;

    Q->index_out++;
    // roll-over the index
    if (Q->index_out == Q->MaxNumElements)
        Q->index_out = 0;

    Q->NumElem--;

    // check if there is need to reschedule
    if ((Q->NumElem) > 0)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "ThreadSafeCallbackAO::Run() - More events, call RunIfNotReady()"));
        RunIfNotReady();
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "ThreadSafeCallbackAO::Run() - No more events, call PendForExec()"));
        PendForExec();
    }

    //release queue access
    Mutex.Unlock();

    // Signal the semaphore that controls the remote thread.
    // The remote thread might be blocked and waiting for an event to be processed in case the event queue is full
    sema_status = RemoteThreadCtrlSema.Signal();
    if (sema_status != OsclProcStatus::SUCCESS_ERROR)
    {
        stat = OsclFailure;
        return NULL;
    }

    return pData;
}

////////// GET CURRENT QUEUE SIZE//////////////
OSCL_EXPORT_REF uint32 ThreadSafeCallbackAO::GetQueueNumElem()
{
    return Q->NumElem;
}

