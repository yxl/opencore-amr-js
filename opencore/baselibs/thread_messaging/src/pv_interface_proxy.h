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
/** \file pv_interface_proxy.h
    \brief Main include file for PV Proxy Implementation.
*/


#ifndef PV_INTERFACE_PROXY_H_INCLUDED
#define PV_INTERFACE_PROXY_H_INCLUDED


#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif
#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif
#ifndef PV_PROXIED_INTERFACE_H_INCLUDED
#include "pv_proxied_interface.h"
#endif
#ifndef OSCL_SCHEDULER_H_INCLUDED
#include "oscl_scheduler.h"
#endif
#ifndef OSCL_MUTEX_H_INCLUDED
#include "oscl_mutex.h"
#endif
#ifndef OSCL_SEMAPHORE_H_INCLUDED
#include "oscl_semaphore.h"
#endif

class CPVInterfaceProxyHandler;
class CPVInterfaceProxyNotifier;
class OsclExecScheduler;
class PVLogger;
class Oscl_DefAlloc;


/**
//list element for the proxy message queues.
*/
class CPVProxyMsg
{
    public:
        CPVProxyMsg(TPVProxyId id, TPVProxyMsgId MsgId, OsclAny *msg)
                : iProxyId(id), iMsgId(MsgId), iMsg(msg)
        {}
        ~CPVProxyMsg()
        {}
        TPVProxyId iProxyId;
        TPVProxyMsgId iMsgId;
        OsclAny *iMsg;
};

/**
//list element for the proxy interface list
*/
class CPVProxyInterface
{
    public:
        CPVProxyInterface(TPVProxyId a, PVProxiedInterfaceServer* b, PVProxiedInterfaceClient* c): iProxyId(a), iServer(b), iClient(c)
        {}
        ~CPVProxyInterface()
        {}
        TPVProxyId iProxyId;
        PVProxiedInterfaceServer *iServer;
        PVProxiedInterfaceClient *iClient;
};

const int32 PVPROXY_DEFAULT_STACK_SIZE = 0x2000;


/**
** Implementation of PVMainProxy
*/
class CPVInterfaceProxy: public PVMainProxy
{
    public:
        /**
        ** Create the proxy.
        ** @param app: the engine to create under the PV thread.
        ** @param alloc: optional memory allocator.  If null, a default
        **    allocator will be used.
        ** @param stacksize: optional stacksize for the PV thread.
        ** @param nreserveInterfaceList: optional reserve size for
        **    the interface list-- to prevent dynamic allocation
        **    in calls to RegisterProxiedInterfaceL.
        ** @param nreserveMessageLists: optional reserve sizes for
        **    the command and notification queues-- to prevent
        **    dynamic allocation by SendCommandL and SendNotificationL
        ** @param handlerPri: priority of the command handler active
        **    object (in the PV thread).
        ** @param notifierPri: priority of the notifier handler active
        **    object in the app thread-- if there is one.
        */
        OSCL_IMPORT_REF static CPVInterfaceProxy * NewL(
            PVProxiedEngine& app
            , Oscl_DefAlloc *alloc = NULL
                                     , int32 stacksize = PVPROXY_DEFAULT_STACK_SIZE
                                                         , uint32 nreserveInterfaceList = 20
                                                                                          , uint32 nreserveMessageLists = 20
                                                                                                                          , int32 handlerPri = OsclActiveObject::EPriorityNominal + 1
                                                                                                                                               , int32 notifierPri = OsclActiveObject::EPriorityNominal + 1
        );

        /**
        ** Cleanup API.  This call will stop the PV thread if needed
        ** and then delete the object.  This call differs from the
        ** destructor only in that it uses the chosen memory allocator
        ** to delete the memory.
        */
        OSCL_IMPORT_REF void Delete();

        //From PVMainProxy
        OSCL_IMPORT_REF TPVProxyId RegisterProxiedInterface(
            PVProxiedInterfaceServer& server_proxy,
            PVProxiedInterfaceClient& client_proxy) ;
        OSCL_IMPORT_REF void UnregisterProxiedInterface(TPVProxyId aProxyId) ;
        OSCL_IMPORT_REF TPVProxyMsgId SendCommand(TPVProxyId aProxyId, OsclAny *aCmd);
        OSCL_IMPORT_REF void CancelCommand(TPVProxyId aProxyId, TPVProxyMsgId aMsgId);
        OSCL_IMPORT_REF void CancelAllCommands(TPVProxyId aProxyId);
        OSCL_IMPORT_REF TPVProxyMsgId SendNotification(TPVProxyId aProxyId, OsclAny *aResp);
        OSCL_IMPORT_REF void CancelNotification(TPVProxyId aProxyId, TPVProxyMsgId aMsgId);
        OSCL_IMPORT_REF void CancelAllNotifications(TPVProxyId aProxyId);
        OSCL_IMPORT_REF void StopPVThread();
        OSCL_IMPORT_REF bool StartPVThread();
        OSCL_IMPORT_REF void DeliverNotifications(int32 aCount, int32& aNoticesPending);

    protected:
        int32 AppThreadLogon();
        int32 AppThreadLogoff();
        int32 AppStartScheduler();
        int32 CPVIConstructL(uint32&, uint32&, int32&, int32&);
        OSCL_IMPORT_REF CPVInterfaceProxy(PVProxiedEngine&, Oscl_DefAlloc*, int32);
        OSCL_IMPORT_REF void ConstructL(uint32, uint32, int32, int32);
        OSCL_IMPORT_REF virtual ~CPVInterfaceProxy();

        //PV Thread stuff..
        PVThreadContext iPVThreadContext;
        int32 iStacksize;
        OsclExecScheduler *iPVScheduler;
        PVLogger *iLogger;
        void InThread();
        OsclThread iPVThread;

        //The handler and notfier AOs.
        CPVInterfaceProxyHandler *iHandler;
        CPVInterfaceProxyNotifier *iNotifier;

        OsclSemaphore iInitSem;
        bool iStopped;
        OsclSemaphore iExitedSem;

        //proxy Id counter
        int32 iProxyIdCounter;

        //command Id counter
        OsclNoYieldMutex iCounterCrit;
        int32 iCommandIdCounter;

        //the proxy list
        OsclNoYieldMutex iProxyListCrit;
        Oscl_Vector<CPVProxyInterface, OsclMemAllocator> iProxyList;
        CPVProxyInterface* FindInterface(TPVProxyId aId, bool locked = false);
        void CleanupInterfaceMessages(CPVProxyInterface *aExt);

        //Note: The message queues need to be thread safe and therefore
        //must use a basic allocator that does not use the PV memory management.

        //The message queues-- handler side
        OsclNoYieldMutex iHandlerQueueCrit;
        Oscl_Vector<CPVProxyMsg, OsclMemAllocator> iCommandQueue;

        //The message queues-- notifier side
        OsclNoYieldMutex iNotifierQueueCrit;
        Oscl_Vector<CPVProxyMsg, OsclMemAllocator> iNotificationQueue;

        void CleanupAppThreadQueues();
        void CleanupPVThreadQueues();
        void CleanupNotifications(CPVProxyInterface *aExt, bool aAll, TPVProxyMsgId aMsgId = 0);
        void CleanupCommands(CPVProxyInterface *aExt, bool aAll, TPVProxyMsgId aMsgId = 0);

        OsclMemAllocator iDefAlloc;
        Oscl_DefAlloc *iAlloc;

        PVProxiedEngine &iPVApp;

        friend class CPVInterfaceProxyHandler;
        friend class CPVInterfaceProxyNotifier;
        friend TOsclThreadFuncRet OSCL_THREAD_DECL pvproxythreadmain(TOsclThreadFuncArg *aPtr);
};

#endif //


