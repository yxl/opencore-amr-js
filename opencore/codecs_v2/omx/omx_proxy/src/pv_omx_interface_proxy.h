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


#ifndef PV_OMX_INTERFACE_PROXY_H_INCLUDED
#define PV_OMX_INTERFACE_PROXY_H_INCLUDED


#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif
#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif
#ifndef PV_OMX_PROXIED_INTERFACE_H_INCLUDED
#include "pv_omx_proxied_interface.h"
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
#ifndef OMX_PROXY_VECTOR_H_INCLUDED
#include "omx_proxy_vector.h"
#endif

#ifndef PV_OMXDEFS_H_INCLUDED
#include "pv_omxdefs.h"
#endif

#if PROXY_INTERFACE

class CPVInterfaceProxyHandler_OMX;
class CPVInterfaceProxyNotifier_OMX;
class OsclExecScheduler;
class PVLogger;
class Oscl_DefAlloc;
/**
//list element for the proxy message queues.
*/

//Memory allocator class (to replace OsclMemAllocator
class oscl_allocator: public Oscl_DefAlloc
{
    public:
        OsclAny* allocate(const uint32 size)
        {
            OsclAny* tmp = iDefAlloc.allocate(size);


            return tmp;
        }

        OsclAny deallocate(OsclAny* p)
        {

            iDefAlloc.deallocate(p);
        }
        virtual ~oscl_allocator() {};

    private:
        _OsclBasicAllocator iDefAlloc;

};


class CPVProxyMsg_OMX
{
    public:
        CPVProxyMsg_OMX(TPVProxyId id, TPVProxyMsgId MsgId, TPVCommandId cmdid, OsclAny *msg)
                : iProxyId(id), iMsgId(MsgId), iCmdId(cmdid), iMsg(msg)
        {}
        ~CPVProxyMsg_OMX()
        {}
        TPVProxyId iProxyId;
        TPVProxyMsgId iMsgId;
        TPVCommandId  iCmdId;
        OsclAny *iMsg;
};

/**
//list element for the proxy interface list
*/
class CPVProxyInterface_OMX
{
    public:
        CPVProxyInterface_OMX(TPVProxyId a, PVProxiedInterfaceServer_OMX* b, PVProxiedInterfaceClient_OMX* c): iProxyId(a), iServer(b), iClient(c)
        {}
        ~CPVProxyInterface_OMX()
        {}
        TPVProxyId iProxyId;
        PVProxiedInterfaceServer_OMX *iServer;
        PVProxiedInterfaceClient_OMX *iClient;
};


const int32 PVPROXY_DEFAULT_STACK_SIZE = 0x2000;

/**
** Implementation of PVMainProxy
*/
class CPVInterfaceProxy_OMX: public PVMainProxy_OMX
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
        OSCL_IMPORT_REF static CPVInterfaceProxy_OMX * NewL(
            PVProxiedEngine_OMX& app
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
            PVProxiedInterfaceServer_OMX& server_proxy,
            PVProxiedInterfaceClient_OMX& client_proxy) ;
        OSCL_IMPORT_REF void UnregisterProxiedInterface(TPVProxyId aProxyId) ;
        OSCL_IMPORT_REF TPVProxyMsgId SendCommand(TPVProxyId aProxyId, TPVCommandId cmdid, OsclAny *aCmd);
        OSCL_IMPORT_REF void CancelCommand(TPVProxyId aProxyId, TPVProxyMsgId aMsgId);
        OSCL_IMPORT_REF void CancelAllCommands(TPVProxyId aProxyId);
        OSCL_IMPORT_REF TPVProxyMsgId SendNotification(TPVProxyId aProxyId, OsclAny *aResp);
        OSCL_IMPORT_REF void CancelNotification(TPVProxyId aProxyId, TPVProxyMsgId aMsgId);
        OSCL_IMPORT_REF void CancelAllNotifications(TPVProxyId aProxyId);
        OSCL_IMPORT_REF void StopPVThread();
        OSCL_IMPORT_REF bool StartPVThread();
        OSCL_IMPORT_REF void DeliverNotifications(int32 aCount, int32& aNoticesPending);

    protected:
        CPVInterfaceProxy_OMX(PVProxiedEngine_OMX&, Oscl_DefAlloc*, int32);
        //Changed return type to use a different fail mechanism
        bool ConstructL(uint32, uint32, int32, int32);
        virtual ~CPVInterfaceProxy_OMX();

        //PV Thread stuff..
        PVThreadContext iPVThreadContext;
        int32 iStacksize;
        OsclExecScheduler *iPVScheduler;
        PVLogger *iLogger;
        OsclSemaphore iThreadCreatedSem;
        bool iOMXThreadCreated;
        void InThread();
        OsclThread iPVThread;

        //The handler and notfier AOs.
        CPVInterfaceProxyHandler_OMX *iHandler;
        CPVInterfaceProxyNotifier_OMX *iNotifier;

        OsclSemaphore iInitSem;
        bool iStopped;
        OsclSemaphore iExitedSem;

        //proxy Id counter
        int32 iProxyIdCounter;

        //command Id counter
        OsclMutex iCounterCrit;
        int32 iCommandIdCounter;

        //the proxy list
        OsclMutex iProxyListCrit;
        //Wrote a new vector class & template class
        Oscl_Vector_New<CPVProxyInterface_OMX, Oscl_Vector_Allocator> iProxyList;
        CPVProxyInterface_OMX* FindInterface(TPVProxyId aId, bool locked = false);
        void CleanupInterfaceMessages(CPVProxyInterface_OMX *aExt);

        //Note: The message queues need to be thread safe and therefore
        //must use a basic allocator that does not use the PV memory management.

        //The message queues-- handler side
        OsclMutex iHandlerQueueCrit;

        //Wrote a new oscl_vector class & its memory template class
        Oscl_Vector_New<CPVProxyMsg_OMX, Oscl_Vector_Allocator> iCommandQueue;

        //The message queues-- notifier side
        OsclMutex iNotifierQueueCrit;
        Oscl_Vector<CPVProxyMsg_OMX, OsclMemAllocator> iNotificationQueue;

        void CleanupAppThreadQueues();
        void CleanupPVThreadQueues();
        void CleanupNotifications(CPVProxyInterface_OMX *aExt, bool aAll, TPVProxyMsgId aMsgId = 0);
        void CleanupCommands(CPVProxyInterface_OMX *aExt, bool aAll, TPVProxyMsgId aMsgId = 0);

        //OsclMemAllocator iDefAlloc;
        oscl_allocator iDefAlloc;
        Oscl_DefAlloc *iAlloc;

        PVProxiedEngine_OMX &iPVApp;

        friend class CPVInterfaceProxyHandler_OMX;
        friend class CPVInterfaceProxyNotifier_OMX;
        friend TOsclThreadFuncRet OSCL_THREAD_DECL pvproxythreadmain_omx(TOsclThreadFuncArg *aPtr);
};
#endif // PROXY_INTERFACE
#endif //


