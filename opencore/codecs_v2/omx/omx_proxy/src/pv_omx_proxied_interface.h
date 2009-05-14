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

#ifndef PV_OMX_PROXIED_INTERFACE_H_INCLUDED
#define PV_OMX_PROXIED_INTERFACE_H_INCLUDED


#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef PV_UUID_H_INCLUDED
#include "pv_uuid.h"
#endif
#ifndef PV_INTERFACE_H_INCLUDED
#include "pv_interface.h"
#endif
#ifndef PV_OMXDEFS_H_INCLUDED
#include "pv_omxdefs.h"
#endif

#define PVUidProxiedInterface  PVUuid(0xf7076653,0x6088,0x47c6,0x88,0xc1,0xb7,0xed,0x28,0xe7,0x2b,0xeb)
#if PROXY_INTERFACE

class PVProxiedInterface_OMX;
class PVProxiedInterfaceClient_OMX;
class PVProxiedInterfaceServer_OMX;
class PVProxiedEngine_OMX;

typedef int32 TPVProxyId;
typedef int32 TPVProxyMsgId;
typedef PVUuid TPVProxyUUID;

//Added command id to identify the openmax commands
typedef int32 TPVCommandId;

/**
** Main proxy class.  This class defines the public interface
** to the PV proxy.
*/
class PVMainProxy_OMX
{
    public:

        /**
        ** This call registers a proxied interface with the main
        ** proxy and assigns a unique interface ID.
        ** Interfaces may be registered at any time during
        ** the life of the main proxy.
        **
        ** @return aProxyId: the ID of this interface pair.
        ** @param aServer: server side (PV Thread) implementation.
        ** @param aClient: client side (app thread) implementation.
        ** This function will leave if memory allocation fails.
        */
        virtual TPVProxyId RegisterProxiedInterface(
            PVProxiedInterfaceServer_OMX& aServer,
            PVProxiedInterfaceClient_OMX& aClient) = 0;

        /**
        ** This call un-registers a proxied interface with the main
        ** proxy.  During this call, the main proxy will de-queue all
        ** undelivered messages belonging to this interface and will
        ** call their memory cleanup routine.
        ** If the interface is not currently registered, this call
        ** will have no effect.
        ** Interfaces may be un-registered at any time during the
        ** life of the main proxy.
        **
        ** @param aProxyId: the ID of this interface pair.
        ** @return : result code
        **/
        virtual void UnregisterProxiedInterface(TPVProxyId aProxyId) = 0;

        /**
        ** This API adds message to the command queue.  This would
        ** typically be called from a PVProxiedInterfaceClient
        ** module.  The message will be delivered asynchronously under
        ** the PV thread.
        **
        ** @param aProxyId: the ID of this interface pair.
        ** @param aCmd: the command.
        ** @return: a unique message ID, assigned by the proxy.
        ** This function will leave if memory allocation fails.
        **/
        virtual TPVProxyMsgId SendCommand(TPVProxyId aProxyId, TPVCommandId cmdid, OsclAny* aCmd) = 0;

        /**
        ** This API will cancel a command that was previously sent
        ** and may still be queued.  This is a synchronous operation.
        ** The proxy will de-queue the message and call its memory cleanup
        ** routine.  If the message is no longer queued, this call will
        ** have no effect.
        **
        ** @param aProxyId: the ID of this interface pair.
        ** @param aMsgId: the command's ID, previously returned by SendCommand.
        ** @return : result code
        */
        virtual void CancelCommand(TPVProxyId aProxyId, TPVProxyMsgId aMsgId) = 0;

        /**
        ** This API will de-queue and cleanup all commands that were
        ** previously sent for an interface and may still be queued.
        ** The cancel operation is synchronous.
        ** If no commands are queued for the interface, this call has
        ** no effect.
        **
        ** @param aProxyId: the ID of this interface pair.
        ** @return : result code
        */
        virtual void CancelAllCommands(TPVProxyId aProxyId) = 0;

        /**
        ** This API adds a message to the notification queue.
        ** This would typically be called by a PVProxiedInterfaceServer
        ** module.  The message will be delivered asynchronously under
        ** the app thread.
        **
        ** @param aProxyId: the ID of this interface pair.
        ** @param aResp: the notification.
        ** @return: a unique message ID, assigned by the proxy.
        ** This function will leave if memory allocation fails.
        **/
        virtual TPVProxyMsgId SendNotification(TPVProxyId aProxyId, OsclAny* aResp) = 0;

        /**
        ** This API will cancel a notification that was previously sent
        ** and may still be queued.  This is a synchronous operation.
        ** The proxy will de-queue the message and call its memory cleanup
        ** routine.  If the message is no longer queued, this call will
        ** have no effect.
        **
        ** @param aProxyId: the ID of this interface pair.
        ** @param aMsgId: the message ID, previously returned by SendNotification.
        ** @return : result code
        */
        virtual void CancelNotification(TPVProxyId aProxyId, TPVProxyMsgId aMsgId) = 0;

        /**
        ** This API will de-queue and cleanup all notifications that were
        ** previously sent for an interface and may still be queued.
        ** The cancel operation is synchronous.
        ** If no notifications are queued for the interface, this call has
        ** no effect.
        **
        ** @param aProxyId: the ID of this interface pair.
        ** @return : result code
        */
        virtual void CancelAllNotifications(TPVProxyId aProxyId) = 0;

        /**
        ** This API will start the PV thread.  The call will block
        ** until the thread starts up and the engine thread logon
        ** completes.
        **
        **  @returns: true if thread creation succeeded-- false otherwise.
        */
        virtual bool StartPVThread() = 0;

        /**
        ** This API will stop the PV thread's scheduler and block
        ** until the thread cleanup is complete.  During the thread
        ** cleanup, the scheduler is stopped, all un-sent messages
        ** are automatically cleaned up, and the engine Thread Logoff
        ** routine is called.
        **
        ** Calling this API under the PV thread context is an error
        ** and will leave.
        */
        virtual void StopPVThread() = 0;

        /**
        ** This API may be used to run the client-side of the proxy
        ** in a non-blocking mode, for cases where there is no Oscl
        ** scheduler or native scheduler running in the application
        ** thread.
        ** The proxy client will run until all pending notifications
        ** have been sent, or else the max count of notifications has
        ** been reached.  This API also returns the count of notifications
        ** still pending after the processing is complete.
        ** Any call to this API from within the PV thread context is
        ** an error and will leave.
        **
        ** @param aMaxCount: (input param) the maximum of pending
        **   notifications to process during the call.
        ** @param aPendingCount: (output param) number of pending
        **   notifications still remaining on the queue.
        */
        virtual void DeliverNotifications(int32 aMaxCount, int32& aPendingCount) = 0;
};

/**
** A proxied engine must implement this class
*/
class PVProxiedEngine_OMX
{
    public:

        /**
        ** Create PV logger appenders for the PV thread.
        ** Creating appenders in this call is optional and
        ** allows logging by the PV thread related to thread
        ** and scheduler initialization that occurs before
        ** the thread logon.
        **/
        virtual void CreateLoggerAppenders() = 0;

        /**
        ** Perform all thread-specific engine creation and
        ** initialization.
        ** This call is made by the main proxy from the PV
        ** Thread after Oscl has been initialized
        ** in the thread, but before starting the scheduler.
        **
        ** @param proxy: reference to the caller.  The app
        **   may save this pointer and use it to make calls
        **   to the main proxy.
        */
        virtual void PVThreadLogon(PVMainProxy_OMX &proxy) = 0;

        /**
        ** Perform all thread-specific engine cleanup.
        ** This call is made by the main proxy from the PV thread
        ** after the scheduler has exited, but before Oscl has been
        ** cleaned up.
        **
        ** @param proxy: reference to the caller.  The app
        **   may save this pointer and use it to make calls
        **   to the main proxy.
        */
        virtual void PVThreadLogoff(PVMainProxy_OMX &proxy) = 0;
};

/**
** Proxied interface modules may implement this class
** in order to provide a common interface query mechanism.
*/
class PVProxiedInterface_OMX : public PVInterface
{
    public:
        /**
        ** To query for supported (proxied) interfaces.
        **
        ** @param aUuid (input): the requested UUID
        ** @param aInterfacePtr (output): a pointer to
        **    the interface implementation, or NULL if not available.
        **
        ** The holder of the interface pointer must call
        ** "removeRef" when it is done with the pointer.
        */
        virtual void QueryProxiedInterface(const TPVProxyUUID& aUuid, PVInterface*& aInterfacePtr) = 0;
        /**
        ** May be used to pass the main proxy pointer to
        ** the implementation.
        */
        virtual void SetMainProxy(PVMainProxy_OMX*) = 0;
};


/**
** Proxied interface modules must implement this class on the
** PV thread side.
*/
class PVProxiedInterfaceServer_OMX
{
    public:
        /**
        ** PVMainProxy calls this under the PV thread to process a
        ** command off the queue.
        **
        ** @param aId: the message ID assigned by the SendCommand call.
        ** @param aMsg: the command data.
        */
        //Added one more argument to identify the openmax command
        virtual void HandleCommand(TPVProxyMsgId aMsgId, TPVCommandId, OsclAny* aMsg) = 0;

        /**
        ** PVMainProxy calls this to cleanup an un-sent or canceled
        ** notification.  The server module should clean up any
        ** allocated memory.  The cleanup operation must be synchronous
        ** and thread-safe.
        **
        ** @param aId: the message ID assigned by the SendNotification call.
        ** @param aMsg: the notification data.
        */
        virtual void CleanupNotification(TPVProxyMsgId aId, OsclAny* aMsg) = 0;
};

/**
** Proxied interface pairs must implement this class on the
** app thread side.
*/
class PVProxiedInterfaceClient_OMX
{
    public:
        /**
        ** PVMainProxy calls this to process a notification off the
        ** queue.
        ** @param aId: the message ID assigned by the SendNotification call.
        ** @param aMsg: the notification data.
        */
        virtual void HandleNotification(TPVProxyMsgId aId, OsclAny* aMsg) = 0;

        /**
        ** PVMainProxy calls this to cleanup an un-sent or canceled
        ** command.  The client module should clean up any allocated
        ** memory.  The cleanup operation must be synchronous
        ** and thread-safe.
        **
        ** @param aId: the message ID assigned by the SendCommand call.
        ** @param aMsg: the command data.
        */
        virtual void CleanupCommand(TPVProxyMsgId aId, OsclAny* aMsg) = 0;
};

/**
//A basic implemention of PVProxiedInterface.
//Interface implementations
//can derive from this.
*/
template<class Alloc>
class PVProxiedInterfaceImpl_OMX : public PVProxiedInterface_OMX
{
    public:
        PVProxiedInterfaceImpl_OMX(const PVUuid& uuid)
                : iRefCounter(1)
                , iUuid(uuid)
                , iMainProxy(NULL)
        {}
        void SetUuid(const PVUuid& uuid)
        {
            iUuid = uuid;
        }
        bool TestUuid(const PVUuid& uuid)
        {
            return iUuid == uuid;
        }
        virtual ~PVProxiedInterfaceImpl_OMX()
        {}
        void removeRef()
        {
            --iRefCounter;
            if (iRefCounter <= 0)
            {
                this->~PVProxiedInterfaceImpl_OMX();
                Alloc alloc;
                alloc.deallocate(this);
            }
        }
        void addRef()
        {
            iRefCounter++;
        }
        bool queryInterface(const PVUuid&, PVInterface*&)
        {
            return false;
        }

        void SetMainProxy(PVMainProxy_OMX *p)
        {
            iMainProxy = p;
        }
        void QueryProxiedInterface(const TPVProxyUUID& aUuid, PVInterface*& aInterfacePtr)
        {
            if (aUuid == iUuid)
                aInterfacePtr = (PVInterface*)this;
            else
                aInterfacePtr = NULL;
        }
    protected:
        int32 iRefCounter;
        PVUuid iUuid;
        PVMainProxy_OMX *iMainProxy;
};

#endif // PROXY_INTERFACE
#endif


