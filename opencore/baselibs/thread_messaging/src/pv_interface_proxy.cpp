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


#ifndef PV_INTERFACE_PROXY_H_INCLUDED
#include "pv_interface_proxy.h"
#endif

#ifndef PV_INTERFACE_PROXY_HANDLER_H_INCLUDED
#include "pv_interface_proxy_handler.h"
#endif

#ifndef PV_INTERFACE_PROXY_NOTIFIER_H_INCLUDED
#include "pv_interface_proxy_notifier.h"
#endif

#ifndef OSCL_DLL_H_INCLUDED
#include "oscl_dll.h"
#endif

#include "oscl_mem.h"

#include "pvlogger.h"

#include "oscl_scheduler.h"
#include "oscl_error.h"
#include "oscl_error_trapcleanup.h"
#include "oscl_tls.h"


OSCL_DLL_ENTRY_POINT_DEFAULT()


//
//CPVInterfaceProxy
//

OSCL_EXPORT_REF CPVInterfaceProxy * CPVInterfaceProxy::NewL(
    PVProxiedEngine& app
    , Oscl_DefAlloc *alloc
    , int32 stacksize
    , uint32 nreserve1
    , uint32 nreserve2
    , int32 handlerPri
    , int32 notifierPri)
//called under app thread context
{
    OsclMemAllocator defallocL;
    OsclAny *ptr = NULL;
    if (alloc)
    {
        ptr = alloc->ALLOCATE(sizeof(CPVInterfaceProxy));
        OsclError::LeaveIfNull(ptr);
    }
    else
    {
        ptr = defallocL.ALLOCATE(sizeof(CPVInterfaceProxy));
    }
    CPVInterfaceProxy *self = OSCL_PLACEMENT_NEW(ptr, CPVInterfaceProxy(app, alloc, stacksize));
    int32 err;
    err = self->CPVIConstructL(nreserve1, nreserve2, handlerPri, notifierPri);
    if (err != OSCL_ERR_NONE)
    {
        self->Delete();
        return NULL;
    }
    return self;
}

OSCL_EXPORT_REF CPVInterfaceProxy::CPVInterfaceProxy(PVProxiedEngine& app, Oscl_DefAlloc*alloc, int32 stacksize)
        : iPVApp(app)
//called under app thread context
{
    iCommandIdCounter = 0;
    iProxyIdCounter = 0;
    iHandler = NULL;
    iPVScheduler = NULL;
    iNotifier = NULL;
    iStacksize = stacksize;
    iStopped = true;
    iAlloc = (alloc) ? alloc : &iDefAlloc;
    iLogger = NULL;
}

OSCL_EXPORT_REF void CPVInterfaceProxy::ConstructL(uint32 nreserve1, uint32 nreserve2, int32 handlerPri, int32 notifierPri)
{
    // Create the semaphores and critical sections
    if (iInitSem.Create() != OsclProcStatus::SUCCESS_ERROR
            || iExitedSem.Create() != OsclProcStatus::SUCCESS_ERROR
            || iCounterCrit.Create() != OsclProcStatus::SUCCESS_ERROR
            || iHandlerQueueCrit.Create() != OsclProcStatus::SUCCESS_ERROR
            || iNotifierQueueCrit.Create() != OsclProcStatus::SUCCESS_ERROR
            || iProxyListCrit.Create() != OsclProcStatus::SUCCESS_ERROR)
    {
        OsclError::Leave(OsclErrGeneral);
    }
    //reserve space in vectors...
    if (nreserve1 > 0)
        iProxyList.reserve(nreserve1);
    if (nreserve2 > 0)
    {
        iCommandQueue.reserve(nreserve2);
//		iNotificationQueue.reserve(nreserve2);
    }

    //create handler
    OsclAny *ptr = iAlloc->ALLOCATE(sizeof(CPVInterfaceProxyHandler));
    OsclError::LeaveIfNull(ptr);
    iHandler = OSCL_PLACEMENT_NEW(ptr, CPVInterfaceProxyHandler(this, handlerPri));

    //create notifier
    ptr = iAlloc->ALLOCATE(sizeof(CPVInterfaceProxyNotifier));
    OsclError::LeaveIfNull(ptr);
    iNotifier = OSCL_PLACEMENT_NEW(ptr, CPVInterfaceProxyNotifier(this, notifierPri));
}

OSCL_EXPORT_REF void CPVInterfaceProxy::Delete()
//called under app thread context
{
    this->~CPVInterfaceProxy();
    iAlloc->deallocate(this);
}

OSCL_EXPORT_REF CPVInterfaceProxy::~CPVInterfaceProxy()
//called under app thread context
{
    //make sure thread was stopped.
    StopPVThread();

    CleanupAppThreadQueues();

    //delete handler and notifier
    if (iHandler)
    {
        iHandler->~CPVInterfaceProxyHandler();
        iAlloc->deallocate(iHandler);
    }
    iHandler = NULL;
    if (iNotifier)
    {
        iNotifier->~CPVInterfaceProxyNotifier();
        iAlloc->deallocate(iNotifier);
    }
    iNotifier = NULL;

    iCounterCrit.Close();
    iHandlerQueueCrit.Close();
    iNotifierQueueCrit.Close();
    iProxyListCrit.Close();
    iInitSem.Close();
    iExitedSem.Close();
}

//forward...
TOsclThreadFuncRet OSCL_THREAD_DECL pvproxythreadmain(TOsclThreadFuncArg *aPtr);

OSCL_EXPORT_REF bool CPVInterfaceProxy::StartPVThread()
//called under app thread context
{
    if (!iStopped)
        return false;//thread already active

    //add notifier AO to app thread scheduler, if it has one.
    if (PVThreadContext::ThreadHasScheduler())
    {
        //Add notification active object to API thread.
        iNotifier->AddToScheduler();
        iNotifier->PendForExec();
    }

    // Create the PV thread.
    OsclProcStatus::eOsclProcError err;
    err = iPVThread.Create((TOsclThreadFuncPtr)pvproxythreadmain,
                           iStacksize,
                           (TOsclThreadFuncArg)this);
    if (err == OSCL_ERR_NONE)
    {
        iStopped = false;
        //Wait for PV thread to initialize its scheduler.
        if (iInitSem.Wait() != OsclProcStatus::SUCCESS_ERROR)
            OsclError::Leave(OsclErrSystemCallFailed);//unexpected sem error
        return true;
    }
    else
    {
        //error cleanup
        iNotifier->RemoveFromScheduler();
        return false;
    }
}

OSCL_EXPORT_REF void CPVInterfaceProxy::StopPVThread()
//called under app thread context.
{
    //if called under the PV thread, we'll get deadlock..
    //so don't allow it.
    if (iPVThreadContext.IsSameThreadContext())
        OsclError::Leave(OsclErrThreadContextIncorrect);

    if (iStopped)
        return ;

    //deque notifier AO
    iNotifierQueueCrit.Lock();
    if (iNotifier && iNotifier->IsAdded())
        iNotifier->RemoveFromScheduler();
    iNotifierQueueCrit.Unlock();

    //Stop the scheduler loop.
    if (iPVScheduler)
        iPVScheduler->StopScheduler();

    //Wait for PV thread to finish up, then it's safe
    //to delete the remaining stuff.
    if (iExitedSem.Wait() != OsclProcStatus::SUCCESS_ERROR)
        OsclError::Leave(OsclErrSystemCallFailed);//unexpected sem error

    //let the destructor know it's exited.
    iStopped = true;

    //The thread will exit on its own, but go ahead and
    //forcibly terminate it now to make sure it's cleaned
    //up by the time this call returns.
    iPVThread.Terminate(0);
}

OSCL_EXPORT_REF void CPVInterfaceProxy::DeliverNotifications(int32 aTargetCount, int32& aNoticesPending)
//deliver notifications off the queue, from the app thread size
{
    //make sure this isn't called under PV thread...
    if (iPVThreadContext.IsSameThreadContext())
        OsclError::Leave(OsclErrThreadContextIncorrect);

    for (int32 count = 0;count < aTargetCount;)
    {
        //get next notification or cleanup message.
        iNotifierQueueCrit.Lock();
        CPVProxyMsg notice(0, 0, NULL);
        if (iNotificationQueue.size() > 0)
        {
            notice = iNotificationQueue[0];
            iNotificationQueue.erase(&iNotificationQueue[0]);
        }
        iNotifierQueueCrit.Unlock();

        if (notice.iMsg)
        {
            count++;
            CPVProxyInterface *ext = FindInterface(notice.iProxyId);
            if (ext)
                ext->iClient->HandleNotification(notice.iMsgId, notice.iMsg);
            else
            {	//since messages are cleaned up when interfaces
                //get unregistered, we should not get here.
                OSCL_ASSERT(0);//debug error.
            }
        }
        else
            break;//no more messages.
    }
    //return number of notices left after trying to process
    //the desired number.
    iNotifierQueueCrit.Lock();
    aNoticesPending = iNotificationQueue.size();
    iNotifierQueueCrit.Unlock();
}

void CPVInterfaceProxy::CleanupAppThreadQueues()
//cleanup memory allocated in App thread.
{
    //un-sent commands...
    iHandlerQueueCrit.Lock();
    while (!iCommandQueue.empty())
    {
        CPVProxyMsg *msg = &iCommandQueue[0];
        CPVProxyInterface *proxy = FindInterface(msg->iProxyId);
        if (proxy)
            proxy->iClient->CleanupCommand(msg->iMsgId, msg->iMsg);
        iCommandQueue.erase(msg);
    }
    iCommandQueue.clear();
    iCommandQueue.destroy();
    iHandlerQueueCrit.Unlock();

    //proxy list...
    iProxyListCrit.Lock();
    iProxyList.clear();
    iProxyList.destroy();
    iProxyListCrit.Unlock();
}

void CPVInterfaceProxy::CleanupPVThreadQueues()
//cleanup memory allocated in PV thread.
{
    //un-sent notifications
    iNotifierQueueCrit.Lock();
    while (!iNotificationQueue.empty())
    {
        CPVProxyMsg *msg = &iNotificationQueue[0];
        CPVProxyInterface *proxy = FindInterface(msg->iProxyId);
        if (proxy)
            proxy->iServer->CleanupNotification(msg->iMsgId, msg->iMsg);
        iNotificationQueue.erase(msg);
    }
    iNotificationQueue.clear();
    iNotificationQueue.destroy();
    iNotifierQueueCrit.Unlock();
}

OSCL_EXPORT_REF void CPVInterfaceProxy::CancelCommand(TPVProxyId aProxyId, TPVProxyMsgId aMsgId)
{
    CleanupCommands(FindInterface(aProxyId), false, aMsgId);
}

OSCL_EXPORT_REF void CPVInterfaceProxy::CancelAllCommands(TPVProxyId aProxyId)
{
    CleanupCommands(FindInterface(aProxyId), true);
}

void CPVInterfaceProxy::CleanupCommands(CPVProxyInterface *aExt, bool aAll, TPVProxyMsgId aMsgId)
{
    if (!aExt)
        return ;
    iHandlerQueueCrit.Lock();
    for (uint32 i = 0;i < iCommandQueue.size();i++)
    {
        CPVProxyMsg *msg = &iCommandQueue[i];
        if (msg->iProxyId == aExt->iProxyId
                && (aAll || msg->iMsgId == aMsgId))
        {
            aExt->iClient->CleanupCommand(msg->iMsgId, msg->iMsg);
            iCommandQueue.erase(msg);
            i--;//move back one since vecter gets scrunched.
            if (!aAll)
            {
                iHandlerQueueCrit.Unlock();
                return ;
            }
        }
    }
    iHandlerQueueCrit.Unlock();
}

OSCL_EXPORT_REF void CPVInterfaceProxy::CancelNotification(TPVProxyId aProxyId, TPVProxyMsgId aMsgId)
{
    CleanupNotifications(FindInterface(aProxyId), false, aMsgId);
}

OSCL_EXPORT_REF void CPVInterfaceProxy::CancelAllNotifications(TPVProxyId aProxyId)
{
    CleanupNotifications(FindInterface(aProxyId), true);
}

void CPVInterfaceProxy::CleanupNotifications(CPVProxyInterface *aExt, bool aAll, TPVProxyMsgId aMsgId)
{
    if (!aExt)
        return ;
    iNotifierQueueCrit.Lock();
    for (uint i = 0;i < iNotificationQueue.size();i++)
    {
        CPVProxyMsg *msg = &iNotificationQueue[i];
        if (msg->iProxyId == aExt->iProxyId
                && (aAll || msg->iMsgId == aMsgId))
        {
            aExt->iServer->CleanupNotification(msg->iMsgId, msg->iMsg);
            iNotificationQueue.erase(msg);
            i--;//move back one since vector gets scrunched.
            if (!aAll)
            {
                iNotifierQueueCrit.Unlock();
                return ;
            }
        }
    }
    iNotifierQueueCrit.Unlock();
}

void CPVInterfaceProxy::CleanupInterfaceMessages(CPVProxyInterface *aExt)
//cleanup all extension messages for a particular interface.
{
    CleanupCommands(aExt, true);
    CleanupNotifications(aExt, true);
}

OSCL_EXPORT_REF TPVProxyId CPVInterfaceProxy::RegisterProxiedInterface(
    PVProxiedInterfaceServer& server_proxy,
    PVProxiedInterfaceClient& client_proxy)
//Proxy extensions call this to register themselves.
{
    TPVProxyId id = ++iProxyIdCounter;
    iProxyListCrit.Lock();
    CPVProxyInterface proxy(id, &server_proxy, &client_proxy);
    int32 err;
    OSCL_TRY(err, iProxyList.push_back(proxy););
    iProxyListCrit.Unlock();
    OsclError::LeaveIfError(err);
    return id;
}

OSCL_EXPORT_REF void CPVInterfaceProxy::UnregisterProxiedInterface(TPVProxyId aProxyId)
//Proxy extensions call this to unregister themselves.
{
    iProxyListCrit.Lock();
    CPVProxyInterface *ext = FindInterface(aProxyId, true);
    if (ext)
    {
        //cleanup unprocessed messages and remove.
        CleanupInterfaceMessages(ext);
        iProxyList.erase(ext);
    }
    iProxyListCrit.Unlock();
}

OSCL_EXPORT_REF TPVProxyMsgId CPVInterfaceProxy::SendCommand(TPVProxyId aProxyId, OsclAny *aCmd)
//Proxy extensions call this to send commands from app side
//to PV side.
{
    int32 err = OSCL_ERR_NONE;
    iCounterCrit.Lock();
    TPVProxyMsgId id = ++iCommandIdCounter;
    iCounterCrit.Unlock();
    iHandlerQueueCrit.Lock();
    CPVProxyMsg msg(aProxyId, id, aCmd);
    OSCL_TRY(err, iCommandQueue.push_back(msg););
    //if the queue was empty, signal the AO.
    //note: when the queue is empty it is safe to assume the handler AO has a request
    //pending.
    if (iCommandQueue.size() == 1)
        iHandler->PendComplete(OSCL_REQUEST_ERR_NONE);
    iHandlerQueueCrit.Unlock();
    //propagate any allocation failure...
    OsclError::LeaveIfError(err);
    return id;
}

OSCL_EXPORT_REF TPVProxyMsgId CPVInterfaceProxy::SendNotification(TPVProxyId aProxyId, OsclAny *aResp)
//Proxy extensions call this to send notifications from PV
//side to app side.
{
    int32 err = OSCL_ERR_NONE;
    iCounterCrit.Lock();
    TPVProxyMsgId id = ++iCommandIdCounter;
    iCounterCrit.Unlock();
    iNotifierQueueCrit.Lock();
    CPVProxyMsg msg(aProxyId, id, aResp);
    OSCL_TRY(err, iNotificationQueue.push_back(msg););
    //if the queue was empty and the notifier is scheduled,
    //signal it.
    //note: when the queue is empty it is safe to assume the notifier AO has a request
    //pending.
    if (iNotifier
            && iNotifier->IsAdded()
            && iNotificationQueue.size() == 1)
        iNotifier->PendComplete(OSCL_REQUEST_ERR_NONE);
    iNotifierQueueCrit.Unlock();
    //propagate any allocation failure...
    OsclError::LeaveIfError(err);
    return id;
}

CPVProxyInterface * CPVInterfaceProxy::FindInterface(TPVProxyId aId, bool locked)
//lookup a registered proxy interface
{
    if (!locked)
        iProxyListCrit.Lock();
    for (uint32 i = 0;i < iProxyList.size();i++)
    {
        if (iProxyList[i].iProxyId == aId)
        {
            if (!locked)
                iProxyListCrit.Unlock();
            return &iProxyList[i];
        }
    }
    if (!locked)
        iProxyListCrit.Unlock();
    return NULL;
}

void CPVInterfaceProxy::InThread()
//this is the guts of the proxy thread routine.
//it's a separate routine since it needs to be
//run under a trap handler in order to avoid any
//Cbase 66 panic from the cleanup stack.
{
    int32 errTerm(OsclErrNone);
    int32 errSched(OsclErrNone);

    //create & install scheduler
    OsclScheduler::Init("PVProxy");
    iPVScheduler = OsclExecScheduler::Current();

    iPVThreadContext.EnterThreadContext();

    //add handler to scheduler
    iHandler->AddToScheduler();
    iHandler->PendForExec();

    //App thread logon...
    int32 err;
    err = AppThreadLogon();
    if (err != OsclErrNone)
        errTerm = err;

    //Start scheduler.  This call blocks until scheduler is
    //either stopped or exits due to an error.
    err = AppStartScheduler();
    if (err != OsclErrNone)
    {
        errSched = err;
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "appstartscheduler error %d...", errSched));
    }


    //Cleanup un-processed data.
    CleanupPVThreadQueues();

    //App thread logoff...
    err = AppThreadLogoff();
    if (err != OSCL_ERR_NONE)
        errTerm = err;

    //Deque the handler AO
    iHandlerQueueCrit.Lock();
    iHandler->RemoveFromScheduler();
    iHandlerQueueCrit.Unlock();

    iPVThreadContext.ExitThreadContext();

    //Uninstall scheduler
    OsclScheduler::Cleanup();
    iPVScheduler = NULL;

    //Generate panics if any leaves happened.
    OSCL_ASSERT(errTerm == OsclErrNone);//EPVProxyPanicEngineLeave
    OSCL_ASSERT(errSched == OsclErrNone);//EPVProxyPanicSchedulerLeave
}

////////////////////////////////
// OS-specific Thread routines
////////////////////////////////


#include "oscl_mem_audit.h"

TOsclThreadFuncRet OSCL_THREAD_DECL pvproxythreadmain(TOsclThreadFuncArg *aPtr)
//PV Thread main routine
{

    CPVInterfaceProxy *proxy = (CPVInterfaceProxy *) aPtr;

    //Init OSCL and create logger.
    OsclBase::Init();
    OsclErrorTrap::Init();
    OsclMem::Init();
    PVLogger::Init();

    //Call the proxied app routine to create its logger appenders.
    proxy->iPVApp.CreateLoggerAppenders();
    proxy->iLogger = PVLogger::GetLoggerObject("pvproxy");

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, proxy->iLogger, PVLOGMSG_NOTICE, (0, "PVPROXY:Proxy Thread 0x%x Entry...", OsclExecScheduler::GetId()));


    int32 leave;
    OSCL_TRY(leave, proxy->InThread(););

    if (leave != OsclErrNone)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, proxy->iLogger, PVLOGMSG_ERR, (0, "PVPROXY:Proxy Thread 0x%x Exit: Leave Reason %d", OsclExecScheduler::GetId(), leave));
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, proxy->iLogger, PVLOGMSG_NOTICE, (0, "PVPROXY:Proxy Thread 0x%x Exit: Normal", OsclExecScheduler::GetId()));
    }

    //Cleanup logger.
    PVLogger::Cleanup();

    OsclMem::Cleanup();
    proxy->iLogger = NULL;
    OsclErrorTrap::Cleanup();
    OsclBase::Cleanup();

    //Signal & Exit
    proxy->iExitedSem.Signal();

    return 0;
}

int32 CPVInterfaceProxy::CPVIConstructL(uint32 &aNreserve1, uint32 &aNreserve2, int32 &aHandlerPri, int32 &aNotifierPri)
{
    int32 err;
    OSCL_TRY(err, ConstructL(aNreserve1, aNreserve2, aHandlerPri, aNotifierPri););
    return err;
}


int32 CPVInterfaceProxy::AppThreadLogon()
{
    int32 err;
    OSCL_TRY(err, iPVApp.PVThreadLogon(*this););
    return err;
}

int32 CPVInterfaceProxy::AppThreadLogoff()
{
    int32 err;
    OSCL_TRY(err, iPVApp.PVThreadLogoff(*this););
    return err;
}

int32 CPVInterfaceProxy::AppStartScheduler()
{
    int32 err;
    OSCL_TRY(err, iPVScheduler->StartScheduler(&iInitSem););
    return err;
}

