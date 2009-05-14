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

#include "pv_2way_proxy_adapter.h"
#include "pvlogger.h"
#include "pvt_common.h"
#include "pv_2way_engine_factory.h"
#include "oscl_error_trapcleanup.h"

#define DEFAULT_2WAY_STACK_SIZE 8192

void PVCmnCmdRespMsg::Set(PVCommandId aId,
                          void *aContext,
                          PVMFStatus aStatus,
                          OsclAny* aEventData,
                          int32 aEventDataSize)
{
    OSCL_UNUSED_ARG(aEventDataSize);
    iId = aId;
    iContext = aContext;
    iStatus = aStatus;
    iEventData = aEventData;
    //iEventDataSize = aEventDataSize;
}

void PVCmnAsyncEventMsg::Set(const PVAsyncInformationalEvent& aEvent, PVEventType aType,
                             PVExclusivePtr aPtr,
                             uint8 *aBuffer,
                             uint32 aBufferSize)
{
    iEventType = aType;
    iEventData = aPtr;
    iEventExtInterface = aEvent.GetEventExtensionInterface();
    if (iEventExtInterface)
    {
        iEventExtInterface->addRef();
    }
    if (aBuffer)
    {
        if (aBufferSize > PV_COMMON_ASYNC_EVENT_LOCAL_BUF_SIZE)
        {
            oscl_memcpy(iLocalBuffer, aBuffer, PV_COMMON_ASYNC_EVENT_LOCAL_BUF_SIZE);
        }
        else
        {
            oscl_memcpy(iLocalBuffer, aBuffer, aBufferSize);
        }
    }
}

void PVCmnAsyncErrorEvent::Set(PVEventType aEventType,
                               PVExclusivePtr aEventData,
                               uint8* aLocalBuffer,
                               int32 aLocalBufferSize)
{
    iEventType = aEventType;
    iEventData = aEventData;
    if (aLocalBuffer)
    {
        if (aLocalBufferSize > PV_COMMON_ASYNC_EVENT_LOCAL_BUF_SIZE)
        {
            oscl_memcpy(iLocalBuffer, aLocalBuffer, PV_COMMON_ASYNC_EVENT_LOCAL_BUF_SIZE);
        }
        else
        {
            oscl_memcpy(iLocalBuffer, aLocalBuffer, aLocalBufferSize);
        }
    }
}

CPV2WayProxyAdapter *CPV2WayProxyAdapter::New(TPVTerminalType aTerminalType,
        PVCommandStatusObserver* aCmdStatusObserver,
        PVInformationalEventObserver *aInfoEventObserver,
        PVErrorEventObserver *aErrorEventObserver)
//called by the factory to create a new proxied 2way interface.
{
    int32 error;
    CPV2WayProxyAdapter* aRet = OSCL_NEW(CPV2WayProxyAdapter, ());
    if (aRet)
    {
        error = Construct(aRet, aTerminalType, aCmdStatusObserver,
                          aInfoEventObserver,
                          aErrorEventObserver);
        if (error)
        {
            OSCL_DELETE(aRet);
            aRet = NULL;
            OSCL_LEAVE(error);
        }
    }
    else
    {
        OSCL_LEAVE(PVMFErrNoMemory);
    }

    return aRet;
}

int32 CPV2WayProxyAdapter::Construct(CPV2WayProxyAdapter*& aRet,
                                     TPVTerminalType aTerminalType,
                                     PVCommandStatusObserver* aCmdStatusObserver,
                                     PVInformationalEventObserver *aInfoEventObserver,
                                     PVErrorEventObserver *aErrorEventObserver)
{
    int32 error;
    OSCL_TRY(error, aRet->ConstructL(aTerminalType,
                                     aCmdStatusObserver,
                                     aInfoEventObserver,
                                     aErrorEventObserver));
    return error;
}

OsclAny CPV2WayProxyAdapter::ConstructL(TPVTerminalType aTerminalType,
                                        PVCommandStatusObserver* aCmdStatusObserver,
                                        PVInformationalEventObserver *aInfoEventObserver,
                                        PVErrorEventObserver *aErrorEventObserver)
{
    OSCL_UNUSED_ARG(aTerminalType);
    iCmdStatusObserver = aCmdStatusObserver;
    iInfoEventObserver = aInfoEventObserver;
    iErrorEventObserver = aErrorEventObserver;

    int32 i;
    for (i = 0; i < MAX_PENDING_2WAY_COMMANDS; i++)
    {
        iFreeCmdMsg.push_back(&iCmdMsg[i]);
    }

    for (i = 0; i < MAX_PENDING_2WAY_EVENTS; i++)
    {
        iFreeEventMsg.push_back(&iEventMsg[i]);
    }

    for (i = 0; i < MAX_PENDING_2WAY_ERRORS; i++)
    {
        iFreeErrorMsg.push_back(&iErrorMsg[i]);
    }

    //Create proxy
    iPVProxy = CPVInterfaceProxy::NewL(*this, NULL, 2 * DEFAULT_2WAY_STACK_SIZE);
    //Register ourself as a proxied interface
    iProxyId = iPVProxy->RegisterProxiedInterface(*this, *this);
    //Start the proxy thread.
    iPVProxy->StartPVThread();
}


CPV2WayProxyAdapter::~CPV2WayProxyAdapter()
//called by the factory to delete 2way interface.
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPV2WayProxyAdapter::~CPV2WayProxyAdapter iterminalEngine(%x)", iterminalEngine));
    if (iPVProxy)
    {
        iPVProxy->StopPVThread();
        iPVProxy->Delete();
    }
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPV2WayProxyAdapter::~CPV2WayProxyAdapter - done"));
}

//
// Pure virtuals from Oscl proxy base class.
//

OSCL_EXPORT_REF void CPV2WayProxyAdapter::DeleteTerminal(PVLogger *aLogger)
//called by proxy base class to delete terminal under the PV thread.
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, aLogger, PVLOGMSG_STACK_TRACE, (0, "CPV2WayProxyAdapter::DeleteTerminal-in"));
    if (iterminalEngine)
        CPV2WayEngineFactory::DeleteTerminal(iterminalEngine);
    iterminalEngine = NULL;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, aLogger, PVLOGMSG_STACK_TRACE, (0, "CPV2WayProxyAdapter::DeleteTerminal-out"));
}

OSCL_EXPORT_REF void CPV2WayProxyAdapter::CreateTerminal(PVLogger *aLogger)
//called by proxy base class to create terminal under the PV thread.
{
    OSCL_UNUSED_ARG(aLogger);
    iterminalEngine =  CPV2WayEngineFactory::CreateTerminal(PV_324M,
                       this,//observers
                       this,
                       this);
}

OSCL_EXPORT_REF void CPV2WayProxyAdapter::CreateLoggerAppenders()
{
}

OSCL_EXPORT_REF void CPV2WayProxyAdapter::PVThreadLogon(PVMainProxy &proxy)
{
    OSCL_UNUSED_ARG(proxy);
    CreateTerminal();
}

OSCL_EXPORT_REF void CPV2WayProxyAdapter::PVThreadLogoff(PVMainProxy &proxy)
{
    OSCL_UNUSED_ARG(proxy);
    DeleteTerminal();
}

OSCL_EXPORT_REF void CPV2WayProxyAdapter::CleanupMessage(CPVCmnInterfaceCmdMessage *cmdMsg, PVLogger *aLogger)
//Cleanup an un-processed command message that was passed to SendAPI.
{
    OSCL_UNUSED_ARG(aLogger);
    if (cmdMsg)
        OSCL_DELETE(cmdMsg);
}

OSCL_EXPORT_REF void CPV2WayProxyAdapter::CleanupNotification(CPVCmnInterfaceObserverMessage *obsMsg, PVLogger *aLogger)
//Cleanup an un-processed notifier message that was passed to NotifyCaller
{
    OSCL_UNUSED_ARG(aLogger);
    OSCL_UNUSED_ARG(obsMsg);
}

OSCL_EXPORT_REF void CPV2WayProxyAdapter::ProcessMessage(CPVCmnInterfaceCmdMessage *aMsg, PVLogger *aLogger)
//called in the PV thread to field a command.
{
    OSCL_UNUSED_ARG(aLogger);
    if (!aMsg)
        return;

    int32 err = ProcessMessageLTry(aMsg);

    //if ProcessMessage did a leave, create
    //a response here...
    if (err)
    {
        int32 tmp_err;
        PVCmnCmdRespMsg *msg = NULL;
        OSCL_TRY(tmp_err, msg = GetCmdMsgL());
        if (tmp_err) return;

        msg->Set(aMsg->GetCommandId() , //id
                 aMsg, //context data
                 err, //status
                 NULL, 0); //response data

        iPVProxy->SendNotification(iProxyId, (OsclAny*)msg);
    }
}

int CPV2WayProxyAdapter::ProcessMessageLTry(CPVCmnInterfaceCmdMessage* aMsg)
{
    // this function exists to get rid of compiler warnings
    int32 err = 0;
    OSCL_TRY(err, ProcessMessageL(aMsg););
    return err;
}

OSCL_EXPORT_REF void CPV2WayProxyAdapter::HandleCommand(TPVProxyMsgId aMsgId, OsclAny* aMsg)
{
    CPVCmnInterfaceCmdMessage *msg = (CPVCmnInterfaceCmdMessage*)aMsg;
    msg->SetId(aMsgId);
    ProcessMessage(msg);
}

OSCL_EXPORT_REF void CPV2WayProxyAdapter::CleanupNotification(TPVProxyMsgId aId,
        OsclAny* aMsg)
{
    OSCL_UNUSED_ARG(aId);
    OSCL_UNUSED_ARG(aMsg);
    //CleanupMessage((CCPVCmnInterfaceCmdMessage*)aMsg);
}

OSCL_EXPORT_REF void CPV2WayProxyAdapter::ProcessNotification(CPVCmnInterfaceObserverMessage *aMsg, PVLogger *aLogger)
//called in the app thread to notify observer.
{
    int32 err = 0;
    OSCL_TRY(err, ProcessNotificationL(aMsg););
    if (err)
    {
        //not really sure what to do with this...
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, aLogger, PVLOGMSG_NONFATAL_ERROR, (0, "PV2WAYPROXY:Error! ProcessNotificationL %d", err));
    }
}

OSCL_EXPORT_REF void CPV2WayProxyAdapter::HandleNotification(TPVProxyMsgId aId, OsclAny* aMsg)
{
    OSCL_UNUSED_ARG(aId);
    ProcessNotification((CPVCmnInterfaceObserverMessage*)aMsg);
}

OSCL_EXPORT_REF void CPV2WayProxyAdapter::CleanupCommand(TPVProxyMsgId aId, OsclAny* aMsg)
{
    OSCL_UNUSED_ARG(aId);
    CleanupNotification((CPVCmnInterfaceObserverMessage*)aMsg);
}

//
// proxied 2way API implementation.
//

PVCommandId CPV2WayProxyAdapter::GetSDKInfo(PVSDKInfo &aSDKInfo, OsclAny* aContextData)
{
    PV2WayMessageGetSDKInfo *msg = OSCL_NEW(PV2WayMessageGetSDKInfo, (aSDKInfo, aContextData));
    if (msg == NULL)
    {
        OSCL_LEAVE(PVMFErrNoMemory);
    }
    return iPVProxy->SendCommand(iProxyId, (OsclAny*)msg);
}

PVCommandId CPV2WayProxyAdapter::GetSDKModuleInfo(PVSDKModuleInfo &aSDKModuleInfo, OsclAny* aContextData)
{
    PV2WayMessageGetSDKModuleInfo *msg = OSCL_NEW(PV2WayMessageGetSDKModuleInfo, (aSDKModuleInfo, aContextData));
    if (msg == NULL)
    {
        OSCL_LEAVE(PVMFErrNoMemory);
    }
    return iPVProxy->SendCommand(iProxyId, (OsclAny*)msg);
}

PVCommandId CPV2WayProxyAdapter::Init(PV2WayInitInfo& aInitInfo, OsclAny* aContextData)
{
    PV2WayMessageInit *msg = OSCL_NEW(PV2WayMessageInit, (aInitInfo, aContextData));
    if (msg == NULL)
    {
        OSCL_LEAVE(PVMFErrNoMemory);
    }
    return iPVProxy->SendCommand(iProxyId, (OsclAny*)msg);
}


PVCommandId CPV2WayProxyAdapter::Reset(OsclAny* aContextData)
{
    PV2WayMessageReset *msg = OSCL_NEW(PV2WayMessageReset, (aContextData));
    if (msg == NULL)
    {
        OSCL_LEAVE(PVMFErrNoMemory);
    }
    return iPVProxy->SendCommand(iProxyId, (OsclAny*)msg);
}

PVCommandId CPV2WayProxyAdapter::AddDataSource(PVTrackId aTrackId,
        PVMFNodeInterface& aDataSource,
        OsclAny* aContextData)
{
    PV2WayMessageAddDataSource *msg = OSCL_NEW(PV2WayMessageAddDataSource, (aTrackId, aDataSource, aContextData));
    if (msg == NULL)
    {
        OSCL_LEAVE(PVMFErrNoMemory);
    }
    return iPVProxy->SendCommand(iProxyId, (OsclAny*)msg);
}

PVCommandId CPV2WayProxyAdapter::RemoveDataSource(PVMFNodeInterface& aDataSource, OsclAny* aContextData)
{
    PV2WayMessageRemoveDataSource *msg = OSCL_NEW(PV2WayMessageRemoveDataSource, (aDataSource, aContextData));
    if (msg == NULL)
    {
        OSCL_LEAVE(PVMFErrNoMemory);
    }
    return iPVProxy->SendCommand(iProxyId, (OsclAny*)msg);
}

PVCommandId CPV2WayProxyAdapter::AddDataSink(PVTrackId aTrackId,
        PVMFNodeInterface& aDataSink,
        OsclAny* aContextData)
{
    PV2WayMessageAddDataSink *msg = OSCL_NEW(PV2WayMessageAddDataSink, (aTrackId, aDataSink, aContextData));
    if (msg == NULL)
    {
        OSCL_LEAVE(PVMFErrNoMemory);
    }
    return iPVProxy->SendCommand(iProxyId, (OsclAny*)msg);
}

PVCommandId CPV2WayProxyAdapter::RemoveDataSink(PVMFNodeInterface& aDataSink, OsclAny* aContextData)
{
    PV2WayMessageRemoveDataSink *msg = OSCL_NEW(PV2WayMessageRemoveDataSink, (aDataSink, aContextData));
    if (msg == NULL)
    {
        OSCL_LEAVE(PVMFErrNoMemory);
    }
    return iPVProxy->SendCommand(iProxyId, (OsclAny*)msg);
}

PVCommandId CPV2WayProxyAdapter::Connect(const PV2WayConnectOptions& aOptions,
        PVMFNodeInterface* aCommServer,
        OsclAny* aContextData)
{
    PV2WayMessageConnect *msg = OSCL_NEW(PV2WayMessageConnect, (aOptions, aCommServer, aContextData));
    if (msg == NULL)
    {
        OSCL_LEAVE(PVMFErrNoMemory);
    }
    return iPVProxy->SendCommand(iProxyId, (OsclAny*)msg);
}

PVCommandId CPV2WayProxyAdapter::Disconnect(OsclAny* aContextData)
{
    PV2WayMessageDisconnect *msg = OSCL_NEW(PV2WayMessageDisconnect, (aContextData));
    if (msg == NULL)
    {
        OSCL_LEAVE(PVMFErrNoMemory);
    }
    return iPVProxy->SendCommand(iProxyId, (OsclAny*)msg);
}

PVCommandId CPV2WayProxyAdapter::GetState(PV2WayState& aState, OsclAny* aContextData)
{
    PV2WayMessageGetPV2WayState *msg = OSCL_NEW(PV2WayMessageGetPV2WayState, (aState, aContextData));
    if (msg == NULL)
    {
        OSCL_LEAVE(PVMFErrNoMemory);
    }
    return iPVProxy->SendCommand(iProxyId, (OsclAny*)msg);
}

PVCommandId CPV2WayProxyAdapter::Pause(PV2WayDirection aDirection,
                                       PVTrackId aTrackId,
                                       OsclAny* aContextData)
{
    PV2WayMessagePause *msg = OSCL_NEW(PV2WayMessagePause, (aDirection, aTrackId, aContextData));
    if (msg == NULL)
    {
        OSCL_LEAVE(PVMFErrNoMemory);
    }
    return iPVProxy->SendCommand(iProxyId, (OsclAny*)msg);
}

PVCommandId CPV2WayProxyAdapter::Resume(PV2WayDirection aDirection,
                                        PVTrackId aTrackId,
                                        OsclAny* aContextData)
{
    PV2WayMessageResume *msg = OSCL_NEW(PV2WayMessageResume, (aDirection, aTrackId, aContextData));
    if (msg == NULL)
    {
        OSCL_LEAVE(PVMFErrNoMemory);
    }
    return iPVProxy->SendCommand(iProxyId, (OsclAny*)msg);
}

PVCommandId CPV2WayProxyAdapter::SetLogAppender(const char* aTag, OsclSharedPtr<PVLoggerAppender>& aAppender, OsclAny* aContextData)
{
    ////// copy tag string //////
    Oscl_TAlloc<uint8, OsclMemAllocator> myAlloc;
    typedef OsclRefCounterSA<Oscl_TAlloc<uint8, OsclMemAllocator> > refcount_type;

    uint32 allocSize = sizeof(refcount_type) + (sizeof(char) * (oscl_strlen(aTag) + 1));
    uint8 * mem = (uint8*)myAlloc.allocate(allocSize);
    OsclError::PushL(mem);

    char* tag = (char*)(mem + sizeof(refcount_type));
    oscl_strncpy(tag, aTag, oscl_strlen(aTag) + 1);

    refcount_type *tagRefCounter = OSCL_PLACEMENT_NEW(mem, refcount_type(mem));

    OsclSharedPtr<char> tagPtr(tag, tagRefCounter);


    PV2WayMessageSetLogAppender *msg = OSCL_NEW(PV2WayMessageSetLogAppender , (tagPtr, aAppender, aContextData));
    if (msg == NULL)
    {
        OSCL_LEAVE(PVMFErrNoMemory);
    }

    OsclError::Pop();

    return iPVProxy->SendCommand(iProxyId, (OsclAny*)msg);
}

PVCommandId CPV2WayProxyAdapter::RemoveLogAppender(const char* aTag, OsclSharedPtr<PVLoggerAppender>& aAppender, OsclAny* aContextData)
{
    ////// copy tag string //////
    Oscl_TAlloc<uint8, OsclMemAllocator> myAlloc;
    typedef OsclRefCounterSA<Oscl_TAlloc<uint8, OsclMemAllocator> > refcount_type;

    uint32 allocSize = sizeof(refcount_type) + (sizeof(char) * (oscl_strlen(aTag) + 1));
    uint8 * mem = (uint8*)myAlloc.allocate(allocSize);
    OsclError::PushL(mem);

    char* tag = (char*)(mem + sizeof(refcount_type));
    oscl_strncpy(tag, aTag, oscl_strlen(aTag) + 1);

    refcount_type *tagRefCounter =  OSCL_PLACEMENT_NEW(mem, refcount_type(mem));

    OsclSharedPtr<char> tagPtr(tag, tagRefCounter);


    PV2WayMessageRemoveLogAppender *msg = OSCL_NEW(PV2WayMessageRemoveLogAppender , (tagPtr, aAppender, aContextData));
    if (msg == NULL)
    {
        OSCL_LEAVE(PVMFErrNoMemory);
    }

    OsclError::Pop();

    return iPVProxy->SendCommand(iProxyId, (OsclAny*)msg);
}

PVCommandId CPV2WayProxyAdapter::SetLogLevel(const char *aTag, int32 aLevel, bool aSetSubtree, OsclAny* aContextData)
{
    // set the log level in this thread
    PVLogger *logger = PVLogger::GetLoggerObject(aTag);
    logger->SetLogLevel(aLevel);

    ////// copy tag string //////
    Oscl_TAlloc<uint8, OsclMemAllocator> myAlloc;
    typedef OsclRefCounterSA<Oscl_TAlloc<uint8, OsclMemAllocator> > refcount_type;

    uint32 allocSize = sizeof(refcount_type) + (sizeof(char) * (oscl_strlen(aTag) + 1));
    uint8 * mem = (uint8*)myAlloc.allocate(allocSize);
    OsclError::PushL(mem);

    char* tag = (char*)(mem + sizeof(refcount_type));
    oscl_strncpy(tag, aTag, oscl_strlen(aTag) + 1);

    refcount_type *tagRefCounter = OSCL_PLACEMENT_NEW(mem, refcount_type(mem));

    OsclSharedPtr<char> tagPtr(tag, tagRefCounter);


    PV2WayMessageSetLogLevel *msg = OSCL_NEW(PV2WayMessageSetLogLevel, (tagPtr, aLevel, aSetSubtree, aContextData));
    if (msg == NULL)
    {
        OSCL_LEAVE(PVMFErrNoMemory);
    }

    OsclError::Pop();

    return iPVProxy->SendCommand(iProxyId, (OsclAny*)msg);
}

PVCommandId CPV2WayProxyAdapter::GetLogLevel(const char *aTag, int32 &aLogLevel, OsclAny* aContextData)
{
    ////// copy tag string //////
    Oscl_TAlloc<uint8, OsclMemAllocator> myAlloc;
    typedef OsclRefCounterSA<Oscl_TAlloc<uint8, OsclMemAllocator> > refcount_type;

    uint32 allocSize = sizeof(refcount_type) + (sizeof(char) * (oscl_strlen(aTag) + 1));
    uint8 * mem = (uint8*)myAlloc.allocate(allocSize);
    OsclError::PushL(mem);

    char* tag = (char*)(mem + sizeof(refcount_type));
    oscl_strncpy(tag, aTag, oscl_strlen(aTag) + 1);

    refcount_type *tagRefCounter =  OSCL_PLACEMENT_NEW(mem, refcount_type(mem));

    OsclSharedPtr<char> tagPtr(tag, tagRefCounter);


    PV2WayMessageGetLogLevel *msg = OSCL_NEW(PV2WayMessageGetLogLevel, (tagPtr, aLogLevel, aContextData));
    if (msg == NULL)
    {
        OSCL_LEAVE(PVMFErrNoMemory);
    }

    OsclError::Pop();

    return iPVProxy->SendCommand(iProxyId, (OsclAny*)msg);
}


PVCommandId CPV2WayProxyAdapter::QueryUUID(const PvmfMimeString& aMimeType, Oscl_Vector<PVUuid, BasicAlloc>& aUuids,
        bool aExactUuidsOnly, OsclAny* aContextData)
{
    PV2WayMessageQueryUUID *msg = OSCL_NEW(PV2WayMessageQueryUUID, (aMimeType, aUuids, aExactUuidsOnly, aContextData));
    if (msg == NULL)
    {
        OSCL_LEAVE(PVMFErrNoMemory);
    }
    return iPVProxy->SendCommand(iProxyId, (OsclAny*)msg);
}

PVCommandId CPV2WayProxyAdapter::QueryInterface(const PVUuid& aUuid, PVInterface*& aInterfacePtr, OsclAny* aContextData)
{
    PV2WayMessageQueryInterface *msg = OSCL_NEW(PV2WayMessageQueryInterface, (aUuid, aInterfacePtr, aContextData));
    if (msg == NULL)
    {
        OSCL_LEAVE(PVMFErrNoMemory);
    }
    return iPVProxy->SendCommand(iProxyId, (OsclAny*)msg);

    // proxiedinterface->QueryProxiedInterface(uuid, iface);

}

PVCommandId CPV2WayProxyAdapter::CancelAllCommands(OsclAny* aContextData)
{
    PV2WayMessageCancelAllCommands *msg = OSCL_NEW(PV2WayMessageCancelAllCommands, (aContextData));
    if (msg == NULL)
    {
        OSCL_LEAVE(PVMFErrNoMemory);
    }
    return iPVProxy->SendCommand(iProxyId, (OsclAny*)msg);
}

//
// 2way engine observer implementation.
//

void CPV2WayProxyAdapter::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    PVCmnAsyncErrorEvent *msg;
    TPVCmnExclusivePtr aExclusivePtr;
    aEvent.GetEventData(aExclusivePtr);

    msg = GetErrorMsgL();

    msg->Set(aEvent.GetEventType(),
             aExclusivePtr,
             aEvent.GetLocalBuffer(),
             PV_COMMON_ASYNC_EVENT_LOCAL_BUF_SIZE);

    iPVProxy->SendNotification(iProxyId, (OsclAny*)msg);
}

void CPV2WayProxyAdapter::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    PVCmnAsyncEventMsg *msg;
    TPVCmnExclusivePtr aExclusivePtr;
    aEvent.GetEventData(aExclusivePtr);
    msg = GetEventMsgL();

    msg->Set(aEvent, aEvent.GetEventType(),
             aExclusivePtr,
             aEvent.GetLocalBuffer(),
             PV_COMMON_ASYNC_EVENT_LOCAL_BUF_SIZE);

    iPVProxy->SendNotification(iProxyId, (OsclAny*)msg);
}

void CPV2WayProxyAdapter::CommandCompleted(const PVCmdResponse& aResponse)
{
    // if command is query interface, we need to get the proxied interface for the response
    CPVCmnInterfaceCmdMessage *iface_msg = (CPVCmnInterfaceCmdMessage *)aResponse.GetContext();
    PVMFStatus status = aResponse.GetCmdStatus();

    if (iface_msg->GetType() == PVT_COMMAND_QUERY_INTERFACE)
    {
        status = PVMFFailure;
        /* Handle query interface*/
        PVUuid uuid = OSCL_STATIC_CAST(PV2WayMessageQueryInterface*, iface_msg)->iUuid;
        PVInterface *iface = OSCL_STATIC_CAST(PV2WayMessageQueryInterface*, iface_msg)->iInterfacePtr;

        if (iface != NULL && aResponse.GetCmdStatus() == PVMFSuccess)
        {
            PVProxiedInterface *proxiedinterface = NULL;
            PVInterface * tempInterface = NULL;

            bool success = iface->queryInterface(PVUidProxiedInterface, tempInterface);
            proxiedinterface = OSCL_STATIC_CAST(PVProxiedInterface*, tempInterface) ;
            iface->removeRef();
            OSCL_STATIC_CAST(PV2WayMessageQueryInterface*, iface_msg)->iInterfacePtr = iface = NULL;

            if (success && proxiedinterface)
            {
                proxiedinterface->SetMainProxy(iPVProxy);
                proxiedinterface->QueryProxiedInterface(uuid, iface);
                proxiedinterface->removeRef();
                OSCL_STATIC_CAST(PV2WayMessageQueryInterface*, iface_msg)->iInterfacePtr = iface;
                if (iface != NULL)
                {
                    status = PVMFSuccess;
                }
            }
        }
    }

    PVCmnCmdRespMsg *msg;
    msg = GetCmdMsgL();

    msg->Set(aResponse.GetCmdId(), //id
             iface_msg, //context
             status,
             aResponse.GetResponseData(),
             aResponse.GetResponseDataSize());//response data

    iPVProxy->SendNotification(iProxyId, (OsclAny*)msg);
}

//
// server side processing
//

void CPV2WayProxyAdapter::ProcessMessageL(CPVCmnInterfaceCmdMessage *aMsg)
//called in the PV thread to field a command.
{
    int32 error = 0;
    //Call the engine, passing the command message pointer as the context data.
    //We will need the command message later in the response processing,
    //so we can restore the original command ID and context data that the
    //app sees.

    switch (aMsg->GetType())
    {
        case PVT_COMMAND_INIT:
        {
            OSCL_TRY(error, iterminalEngine->Init(
                         OSCL_STATIC_CAST(PV2WayMessageInit *, aMsg)->iInitInfo,
                         aMsg));
        }
        break;
        case PVT_COMMAND_GET_SDK_INFO:
        {
            OSCL_TRY(error, iterminalEngine->GetSDKInfo(
                         OSCL_STATIC_CAST(PV2WayMessageGetSDKInfo *, aMsg)->iSDKInfo,
                         aMsg));
        }
        break;

        case PVT_COMMAND_GET_SDK_MODULE_INFO:
        {
            OSCL_TRY(error, iterminalEngine->GetSDKModuleInfo(
                         OSCL_STATIC_CAST(PV2WayMessageGetSDKModuleInfo* , aMsg)->iSDKModuleInfo,
                         aMsg));
        }
        break;

        case PVT_COMMAND_GET_PV2WAY_STATE:
        {
            OSCL_TRY(error, iterminalEngine->GetState(
                         OSCL_STATIC_CAST(PV2WayMessageGetPV2WayState*, aMsg)->iState,
                         aMsg));
        }
        break;

        case PVT_COMMAND_RESET:
            OSCL_TRY(error, iterminalEngine->Reset(aMsg));
            break;

        case PVT_COMMAND_ADD_DATA_SOURCE:
            OSCL_TRY(error, iterminalEngine->AddDataSource(
                         OSCL_STATIC_CAST(PV2WayMessageAddDataSource *, aMsg)->iTrackId,
                         OSCL_STATIC_CAST(PV2WayMessageAddDataSource *, aMsg)->iDataSource,
                         aMsg));
            break;

        case PVT_COMMAND_REMOVE_DATA_SOURCE:
            OSCL_TRY(error, iterminalEngine->RemoveDataSource(
                         OSCL_STATIC_CAST(PV2WayMessageRemoveDataSource*, aMsg)->iDataSource,
                         aMsg));
            break;

        case PVT_COMMAND_ADD_DATA_SINK:
            OSCL_TRY(error, iterminalEngine->AddDataSink(
                         OSCL_STATIC_CAST(PV2WayMessageAddDataSink *, aMsg)->iTrackId,
                         OSCL_STATIC_CAST(PV2WayMessageAddDataSink *, aMsg)->iDataSink,
                         aMsg));
            break;

        case PVT_COMMAND_REMOVE_DATA_SINK:
            OSCL_TRY(error, iterminalEngine->RemoveDataSink(
                         OSCL_STATIC_CAST(PV2WayMessageRemoveDataSink* , aMsg)->iDataSink,
                         aMsg));
            break;

        case PVT_COMMAND_CONNECT:
            OSCL_TRY(error, iterminalEngine->Connect(
                         OSCL_STATIC_CAST(PV2WayMessageConnect *, aMsg)->iConnectOptions,
                         OSCL_STATIC_CAST(PV2WayMessageConnect *, aMsg)->iCommServer,
                         aMsg));
            break;

        case PVT_COMMAND_DISCONNECT:
            OSCL_TRY(error, iterminalEngine->Disconnect(aMsg));
            break;

        case PVT_COMMAND_PAUSE:
            OSCL_TRY(error, iterminalEngine->Pause(
                         OSCL_STATIC_CAST(PV2WayMessagePause* , aMsg)->iDirection,
                         OSCL_STATIC_CAST(PV2WayMessagePause* , aMsg)->iTrackId,
                         aMsg));
            break;

        case PVT_COMMAND_RESUME:
            OSCL_TRY(error, iterminalEngine->Resume(
                         OSCL_STATIC_CAST(PV2WayMessagePause* , aMsg)->iDirection,
                         OSCL_STATIC_CAST(PV2WayMessagePause* , aMsg)->iTrackId,
                         aMsg));
            break;

        case PVT_COMMAND_SET_LOG_APPENDER:
        {
            OSCL_TRY(error, iterminalEngine->SetLogAppender(
                         OSCL_STATIC_CAST(PV2WayMessageSetLogAppender*, aMsg)->iTag,
                         OSCL_STATIC_CAST(PV2WayMessageSetLogAppender*, aMsg)->iAppender,
                         aMsg));
        }
        break;

        case PVT_COMMAND_REMOVE_LOG_APPENDER:
        {
            OSCL_TRY(error, iterminalEngine->RemoveLogAppender(
                         OSCL_STATIC_CAST(PV2WayMessageRemoveLogAppender*, aMsg)->iTag,
                         OSCL_STATIC_CAST(PV2WayMessageRemoveLogAppender*, aMsg)->iAppender,
                         aMsg));
        }
        break;

        case PVT_COMMAND_SET_LOG_LEVEL:
            OSCL_TRY(error, iterminalEngine->SetLogLevel(
                         OSCL_STATIC_CAST(PV2WayMessageSetLogLevel*, aMsg)->iTag,
                         OSCL_STATIC_CAST(PV2WayMessageSetLogLevel*, aMsg)->iLevel,
                         OSCL_STATIC_CAST(PV2WayMessageSetLogLevel*, aMsg)->iSetSubtree,
                         aMsg));
            break;

        case PVT_COMMAND_GET_LOG_LEVEL:
            OSCL_TRY(error, iterminalEngine->GetLogLevel(
                         OSCL_STATIC_CAST(PV2WayMessageGetLogLevel*, aMsg)->iTag,
                         OSCL_STATIC_CAST(PV2WayMessageGetLogLevel*, aMsg)->iLogLevel,
                         aMsg));
            break;

        case PVT_COMMAND_QUERY_UUID:
            OSCL_TRY(error, iterminalEngine->QueryUUID(
                         OSCL_STATIC_CAST(PV2WayMessageQueryUUID*, aMsg)->iMimeType,
                         OSCL_STATIC_CAST(PV2WayMessageQueryUUID*, aMsg)->iUuids,
                         OSCL_STATIC_CAST(PV2WayMessageQueryUUID*, aMsg)->iExactUuidsOnly,
                         aMsg));
            break;

        case PVT_COMMAND_QUERY_INTERFACE:
            OSCL_TRY(error, iterminalEngine->QueryInterface(
                         OSCL_STATIC_CAST(PV2WayMessageQueryInterface*, aMsg)->iUuid,
                         OSCL_STATIC_CAST(PV2WayMessageQueryInterface*, aMsg)->iInterfacePtr,
                         aMsg));
            break;

        case PVT_COMMAND_CANCEL_ALL_COMMANDS:
            OSCL_TRY(error, iterminalEngine->CancelAllCommands(
                         aMsg));
            break;
    }

    if (error)
    {
        //create response to go across proxy.
        PVCmnCmdRespMsg *msg = GetCmdMsgL();

        msg->Set(aMsg->GetCommandId(), //id
                 aMsg, //context
                 error, //status
                 NULL, 0); //response data

        iPVProxy->SendNotification(iProxyId, (OsclAny*)msg);
    }
}

//
// client side processing
//

void CPV2WayProxyAdapter::ProcessNotificationL(CPVCmnInterfaceObserverMessage *aMsg)
//called in the app thread to notify observer.
{
    int32 err = 0;

    if (!aMsg)
        return;

    switch (aMsg->GetResponseType())
    {
        case 0:
        {
            PVCmnCmdRespMsg *resp = (PVCmnCmdRespMsg*)aMsg;

            //Get the command message that prompted this
            //response.  The pointer is in the context data.
            CPVCmnInterfaceCmdMessage *cmd = (CPVCmnInterfaceCmdMessage*)resp->GetContext();

            //Create a new response with the original command ID and
            //original context data.
            resp->SetId(cmd->GetCommandId());
            resp->SetContextData(cmd->GetContextData());
            OSCL_TRY(err, iCmdStatusObserver->CommandCompleted(*resp));
            //ignore any leave from the observer function.

            //discard command message.
            OSCL_DELETE(cmd);

            FreeCmdMsg(resp);
        }
        break;

        case 1:
        {
            PVCmnAsyncEventMsg *event = (PVCmnAsyncEventMsg*) aMsg;

            OSCL_TRY(err, iInfoEventObserver->HandleInformationalEvent(*event););
            //ignore any leave from the observer function.

            FreeEventMsg((PVCmnAsyncEventMsg *) aMsg);

        }
        break;

        case 2:
        {
            PVCmnAsyncErrorEvent *event = (PVCmnAsyncErrorEvent *) aMsg;

            OSCL_TRY(err, iErrorEventObserver->HandleErrorEvent(*event););
            //ignore any leave from the observer function.

            FreeErrorMsg((PVCmnAsyncErrorEvent *) aMsg);
        }
        break;

        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "CPV2WayProxyAdapter::ProcessNotificationL unknown response (%d)", aMsg->GetResponseType()));
            //Assume command message.
            FreeCmdMsg((PVCmnCmdRespMsg *) aMsg);
            break;
    }
}


PVCmnCmdRespMsg *CPV2WayProxyAdapter::GetCmdMsgL()
{
    if (iFreeCmdMsg.empty())
    {
        OSCL_LEAVE(PVMFErrNoMemory);
    }
    else
    {
        PVCmnCmdRespMsg *cmd = (PVCmnCmdRespMsg *)iFreeCmdMsg[0];
        iFreeCmdMsg.erase(iFreeCmdMsg.begin());
        return cmd;
    }

    return NULL;
}

PVCmnAsyncEventMsg *CPV2WayProxyAdapter::GetEventMsgL()
{
    if (iFreeEventMsg.empty())
    {
        OSCL_LEAVE(PVMFErrNoMemory);
    }
    else
    {
        PVCmnAsyncEventMsg *cmd = (PVCmnAsyncEventMsg *)iFreeEventMsg[0];
        iFreeEventMsg.erase(iFreeEventMsg.begin());
        return cmd;
    }

    return NULL;
}

PVCmnAsyncErrorEvent* CPV2WayProxyAdapter::GetErrorMsgL()
{
    if (iFreeErrorMsg.empty())
    {
        OSCL_LEAVE(PVMFErrNoMemory);
    }
    else
    {
        PVCmnAsyncErrorEvent *cmd = (PVCmnAsyncErrorEvent *)iFreeErrorMsg[0];
        iFreeErrorMsg.erase(iFreeErrorMsg.begin());
        return cmd;
    }

    return NULL;
}

