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
#ifndef OSCL_BYTE_ORDER_H_INCLUDED
#include "oscl_byte_order.h"
#endif

#ifndef OSCL_SOCKET_H_INCLUDED
#include "oscl_socket.h"
#endif

#ifndef OSCL_SOCKET_TYPES_H_INCLUDED
#include "oscl_socket_types.h"
#endif

#ifndef OSCL_STRING_UTILS_H_INCLUDED
#include "oscl_string_utils.h"
#endif

#ifndef OSCL_SNPRINTF_H_INCLUDED
#include "oscl_snprintf.h"
#endif

#ifndef PVRTSP_CLIENT_ENGINE_NODE_H
#include "pvrtsp_client_engine_node.h"
#endif

#ifndef PVRTSP_CLIENT_ENGINE_UTILS_H
#include "pvrtsp_client_engine_utils.h"
#endif

#ifndef PVMF_SIMPLE_MEDIA_BUFFER_H_INCLUDED
#include "pvmf_simple_media_buffer.h"
#endif

#ifndef PVMF_BASIC_ERRORINFOMESSAGE_H_INCLUDED
#include "pvmf_basic_errorinfomessage.h"
#endif

#ifndef PVMF_ERRORINFOMESSAGE_EXTENSION_H_INCLUDED
#include "pvmf_errorinfomessage_extension.h"
#endif

#ifndef PVMF_SM_NODE_EVENTS_H_INCLUDED
#include "pvmf_sm_node_events.h"
#endif

#ifndef PVMF_MEDIA_MSG_HEADER_H_INCLUDED
#include "pvmf_media_msg_header.h"
#endif

#ifndef PVMF_SM_TUNABLES_H_INCLUDED
#include "pvmf_sm_tunables.h"
#endif

#ifndef BASE64_CODEC_H_INCLUDED
#include "base64_codec.h"
#endif

#ifndef PV_STRING_URI_H_INCLUDE
#include "pv_string_uri.h"
#endif

#ifndef PVRTSP_ENGINE_NODE_EXTENSION_INTERFACE_IMPL_H_INCLUDED
#include "pvrtspenginenodeextensioninterface_impl.h"
#endif

/*
#ifndef PV_PLAYER_SDKINFO_H_INCLUDED	//\engines\player\src\pv_player_sdkinfo.h
#include "pv_player_sdkinfo.h"
#endif
*/
////////////////////////////////////////////////////////////////////////////////

const int PVRTSPEngineNode::REQ_SEND_SOCKET_ID = 1;
const int PVRTSPEngineNode::REQ_RECV_SOCKET_ID = 2;


OSCL_EXPORT_REF PVRTSPEngineNode::PVRTSPEngineNode(int32 aPriority) :
        OsclTimerObject(aPriority, "PVRTSPEngineNode"),
        iState(PVRTSP_ENGINE_NODE_STATE_IDLE),
        iCurrentCmdId(0),
        iSockServ(NULL),
        iSocketCleanupState(ESocketCleanup_Idle),
        iRTSPParser(NULL),
        iRTSPParserState(RTSPParser::WAITING_FOR_DATA),
        iOutgoingSeq(0),
        bNoRecvPending(false),
        bNoSendPending(false),
        iTheBusyPort(NULL),
        iLogger(NULL),
        iExtensionRefCount(0),
        iNumRedirectTrials(PVRTSPENGINENODE_DEFAULT_NUMBER_OF_REDIRECT_TRIALS),
        iNumHostCallback(0),
        iNumConnectCallback(0),
        iNumSendCallback(0),
        iNumRecvCallback(0),
        BASE_REQUEST_ID(0),
        REQ_TIMER_WATCHDOG_ID(0),
        REQ_TIMER_KEEPALIVE_ID(0),
        REQ_DNS_LOOKUP_ID(0),
        DEFAULT_RTSP_PORT(554),
        DEFAULT_HTTP_PORT(80),
        TIMEOUT_CONNECT_AND_DNS_LOOKUP(30000),
        TIMEOUT_SEND(3000),
        TIMEOUT_RECV(-1),
        TIMEOUT_SHUTDOWN(30000),
        TIMEOUT_WATCHDOG(20),
        TIMEOUT_WATCHDOG_TEARDOWN(2),
        TIMEOUT_KEEPALIVE(PVRTSPENGINENODE_DEFAULT_KEEP_ALIVE_INTERVAL),
        RECOMMENDED_RTP_BLOCK_SIZE(1400),
        setupTrackIndex(0),
        bRepositioning(false),// \todo reset the reqplayrange to invalid after get the PLAY resp
        iSrvResponse(NULL),
        bSrvRespPending(false),
        iWatchdogTimer(NULL),
        iCurrentErrorCode(PVMFRTSPClientEngineNodeErrorEventStart),
        iEventUUID(PVMFRTSPClientEngineNodeEventTypeUUID),
        bKeepAliveInPlay(false),
        iKeepAliveMethod(METHOD_OPTIONS),
        bAddXStrHeader(false),
        iErrorRecoveryAttempt(0),
        iGetPostCorrelationObject(NULL),
        ibIsRealRDT(false),
        ipRealChallengeGen(NULL),
        ipRdtParser(NULL),
        ipFragGroupAllocator(NULL),
        ipFragGroupMemPool(NULL),
        ibBlockedOnFragGroups(false),
        iExtensionInterface(NULL)
{
    int32 err;
    OSCL_TRY(err,
             //Create the input command queue.  Use a reserve to avoid lots of
             //dynamic memory allocation.
             iPendingCmdQueue.Construct(PVMF_RTSP_ENGINE_NODE_COMMAND_ID_START, PVMF_RTSP_ENGINE_NODE_COMMAND_VECTOR_RESERVE);

             //Create the "current command" queue.  It will only contain one
             //command at a time, so use a reserve of 1.
             iRunningCmdQueue.Construct(0, 1);

             //Create the port vector.
             iPortVector.Construct(PVMF_RTSP_NODE_PORT_VECTOR_RESERVE);
             iPortActivityQueue.reserve(PVMF_RTSP_ENGINE_NODE_COMMAND_VECTOR_RESERVE);

             //Set the node capability data.
             //This node can support one duplex port.
             iCapability.iCanSupportMultipleInputPorts = false;
             iCapability.iCanSupportMultipleOutputPorts = false;
             iCapability.iHasMaxNumberOfPorts = true;
             iCapability.iMaxNumberOfPorts = 1;

             iEntityMemFrag.len = 0;
             iEntityMemFrag.ptr = NULL;

             iRTSPEngTmpBuf.len = 0;
             iRTSPEngTmpBuf.ptr = OSCL_MALLOC(RTSP_MAX_FULL_REQUEST_SIZE);
             OsclError::LeaveIfNull(iRTSPEngTmpBuf.ptr);
             iRTSPEngTmpBuf.len = RTSP_MAX_FULL_REQUEST_SIZE;

             iWatchdogTimer = OSCL_NEW(OsclTimer<PVRTSPEngineNodeAllocator>, ("PVRTSPEngineNodeWatchDog"));
             OsclError::LeaveIfNull(iWatchdogTimer);

             //TBD
             //iMediaDataResizableAlloc =OSCL_NEW(OsclMemPoolResizableAllocator, (RECOMMENDED_RTP_BLOCK_SIZE, 0, 0, &iAlloc));;
             iMediaDataResizableAlloc = OSCL_NEW(OsclMemPoolResizableAllocator, (RECOMMENDED_RTP_BLOCK_SIZE));
             OsclError::LeaveIfNull(iMediaDataResizableAlloc);


             iMediaDataImplAlloc = OSCL_NEW(PVMFSimpleMediaBufferCombinedAlloc, (iMediaDataResizableAlloc));;
             OsclError::LeaveIfNull(iMediaDataImplAlloc);

            );

    if (err != OsclErrNone)
    {
        //if a leave happened, cleanup and re-throw the error
        iPendingCmdQueue.clear();
        iRunningCmdQueue.clear();
        iPortVector.clear();
        iCapability.iInputFormatCapability.clear();
        iCapability.iOutputFormatCapability.clear();
        OSCL_CLEANUP_BASE_CLASS(PVMFNodeInterface);
        OSCL_CLEANUP_BASE_CLASS(OsclTimerObject);
        OSCL_LEAVE(err);
    }
    iWatchdogTimer->SetObserver(this);
    iWatchdogTimer->SetFrequency(1);
    iInterfaceState = EPVMFNodeCreated;
}

OSCL_EXPORT_REF PVRTSPEngineNode::~PVRTSPEngineNode()
{
    Cancel();

    if (iExtensionInterface)
    {
        iExtensionInterface->removeRef();
    }

    if (iWatchdogTimer)
    {
        OSCL_DELETE(iWatchdogTimer);
        iWatchdogTimer = NULL;
    }

    if (iRTSPEngTmpBuf.len > 0)
    {
        OSCL_FREE(iRTSPEngTmpBuf.ptr);
        iRTSPEngTmpBuf.len = 0;
        iRTSPEngTmpBuf.ptr = NULL;
    }

    if (iEntityMemFrag.len > 0)
    {
        OSCL_FREE(iEntityMemFrag.ptr);
        iEntityMemFrag.len = 0;
        iEntityMemFrag.ptr = NULL;
    }

    if (iSrvResponse)
    {
        OSCL_DELETE(iSrvResponse);
        iSrvResponse = NULL;
    }

    if (iRTSPParser)
    {
        OSCL_DELETE(iRTSPParser);
        iRTSPParser = NULL;
    }

    if (iMediaDataImplAlloc != NULL)
    {
        OSCL_DELETE(iMediaDataImplAlloc);
    }

    if (iMediaDataResizableAlloc != NULL)
    {
        iMediaDataResizableAlloc->removeRef();
    }

    clearOutgoingMsgQueue();

    resetSocket(true);

    if (iDNS.iDns)
    {
        iDNS.iDns->~OsclDNS();
        iAlloc.deallocate(iDNS.iDns);
        iDNS.iDns = NULL;
    }

    if (iSockServ)
    {
        iSockServ->Close();
        iSockServ->~OsclSocketServ();
        iAlloc.deallocate(iSockServ);
        iSockServ = NULL;
    }

    if (ipFragGroupAllocator != NULL)
        OSCL_DELETE(ipFragGroupAllocator);
    if (ipFragGroupMemPool != NULL)
        OSCL_DELETE(ipFragGroupMemPool);

    if (iGetPostCorrelationObject != NULL)
    {
        OSCL_DELETE(iGetPostCorrelationObject);
        iGetPostCorrelationObject = NULL;
    }

}

OSCL_EXPORT_REF PVMFStatus PVRTSPEngineNode::ThreadLogon()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::ThreadLogon() called"));
    switch (iInterfaceState)
    {
        case EPVMFNodeCreated:
            if (!IsAdded())
                AddToScheduler();
            iLogger = PVLogger::GetLoggerObject("PVRTSPEngineNode");
            SetState(EPVMFNodeIdle);
            return PVMFSuccess;
            // break;	This break statement was removed to avoid compiler warning for Unreachable Code
        default:
            return PVMFErrInvalidState;
            // break;	This break statement was removed to avoid compiler warning for Unreachable Code
    }
}

OSCL_EXPORT_REF PVMFStatus PVRTSPEngineNode::ThreadLogoff()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::ThreadLogoff() called"));
    switch (iInterfaceState)
    {
        case EPVMFNodeIdle:
            if (IsAdded())
                RemoveFromScheduler();
            iLogger = NULL;
            SetState(EPVMFNodeCreated);
            return PVMFSuccess;
            // break;	This break statement was removed to avoid compiler warning for Unreachable Code

        default:
            return PVMFErrInvalidState;
            // break;	This break statement was removed to avoid compiler warning for Unreachable Code
    }
}

OSCL_EXPORT_REF PVMFStatus PVRTSPEngineNode::GetCapability(PVMFNodeCapability& aNodeCapability)
{
    OSCL_UNUSED_ARG(aNodeCapability);
    return PVMFFailure;
}

OSCL_EXPORT_REF PVMFPortIter* PVRTSPEngineNode::GetPorts(const PVMFPortFilter* aFilter)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::GetPorts() called"));

    OSCL_UNUSED_ARG(aFilter);
    // TODO: Return the currently available ports
    return NULL;
}

OSCL_EXPORT_REF PVMFCommandId PVRTSPEngineNode::QueryUUID(PVMFSessionId aSession
        , const PvmfMimeString& aMimeType
        , Oscl_Vector<PVUuid, PVRTSPEngineNodeAllocator>& aUuids
        , bool aExactUuidsOnly
        , const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::QueryUUID() called"));

    PVRTSPEngineCommand cmd;
    cmd.PVRTSPEngineCommandBase::Construct(aSession, PVMF_GENERIC_NODE_QUERYUUID, aMimeType, \
                                           aUuids, aExactUuidsOnly, aContext);

    return AddCmdToQueue(cmd);
}

OSCL_EXPORT_REF void PVRTSPEngineNode::addRef()
{

}

OSCL_EXPORT_REF void PVRTSPEngineNode::removeRef()
{

}

OSCL_EXPORT_REF bool PVRTSPEngineNode::queryInterface(const PVUuid& uuid, PVInterface*& iface)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::QueryInterface() In iExtensionInterface %x", iExtensionInterface));
    iface = NULL;
    if (uuid == KPVRTSPEngineNodeExtensionUuid)
    {
        if (!iExtensionInterface)
        {
            iExtensionInterface = OSCL_NEW(PVRTSPEngineNodeExtensionInterfaceImpl, (this));
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::QueryInterface() iExtensionInterface %x", iExtensionInterface));
        }
        if (iExtensionInterface)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::QueryInterface() Interface existing iExtensionInterface %x", iExtensionInterface));
            return (iExtensionInterface->queryInterface(uuid, iface));
        }
        else
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVRTSPEngineNode::queryInterface()- ERROR No memory"));
            OSCL_LEAVE(OsclErrNoMemory);
            return false;
        }
    }
    else
    {
        return false;
    }
}

OSCL_EXPORT_REF PVMFCommandId PVRTSPEngineNode::QueryInterface(PVMFSessionId aSession
        , const PVUuid& aUuid
        , PVInterface*& aInterfacePtr
        , const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::QueryInterface() called"));

    PVRTSPEngineCommand cmd;
    cmd.PVRTSPEngineCommandBase::Construct(aSession, PVMF_GENERIC_NODE_QUERYINTERFACE, aUuid, aInterfacePtr, aContext);
    return AddCmdToQueue(cmd);
}

OSCL_EXPORT_REF   PVMFCommandId PVRTSPEngineNode::RequestPort(PVMFSessionId aSession
        , int32 aPortTag
        , const PvmfMimeString* aPortConfig
        , const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::RequestPort() called"));

    PVRTSPEngineCommand cmd;
    cmd.PVRTSPEngineCommandBase::Construct(aSession, PVMF_GENERIC_NODE_REQUESTPORT, aPortTag, aPortConfig, aContext);
    return AddCmdToQueue(cmd);
}

OSCL_EXPORT_REF   PVMFCommandId PVRTSPEngineNode::ReleasePort(PVMFSessionId aSession
        , PVMFPortInterface& aPort
        , const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::ReleasePort() called"));

    PVRTSPEngineCommand cmd;
    cmd.PVRTSPEngineCommandBase::Construct(aSession, PVMF_GENERIC_NODE_RELEASEPORT, aPort, aContext);
    return AddCmdToQueue(cmd);
}

OSCL_EXPORT_REF   PVMFCommandId PVRTSPEngineNode::Init(PVMFSessionId aSession
        , const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::Init() called"));

    PVRTSPEngineCommand cmd;
    cmd.PVRTSPEngineCommandBase::Construct(aSession, PVMF_GENERIC_NODE_INIT, aContext);
    return AddCmdToQueue(cmd);
}

OSCL_EXPORT_REF   PVMFCommandId PVRTSPEngineNode::Prepare(PVMFSessionId aSession
        , const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::Init() called"));

    PVRTSPEngineCommand cmd;
    cmd.PVRTSPEngineCommandBase::Construct(aSession, PVMF_GENERIC_NODE_PREPARE, aContext);
    return AddCmdToQueue(cmd);
}

OSCL_EXPORT_REF   PVMFCommandId PVRTSPEngineNode::Start(PVMFSessionId aSession
        , const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::Start() called"));

    PVRTSPEngineCommand cmd;
    cmd.PVRTSPEngineCommandBase::Construct(aSession, PVMF_GENERIC_NODE_START, aContext);
    return AddCmdToQueue(cmd);
}

OSCL_EXPORT_REF   PVMFCommandId PVRTSPEngineNode::Pause(PVMFSessionId aSession
        , const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::Pause() called"));

    PVRTSPEngineCommand cmd;
    cmd.PVRTSPEngineCommandBase::Construct(aSession, PVMF_GENERIC_NODE_PAUSE, aContext);
    return AddCmdToQueue(cmd);
}

OSCL_EXPORT_REF   PVMFCommandId PVRTSPEngineNode::Stop(PVMFSessionId aSession
        , const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::Stop() called"));

    PVRTSPEngineCommand cmd;
    cmd.PVRTSPEngineCommandBase::Construct(aSession, PVMF_GENERIC_NODE_STOP, aContext);
    return AddCmdToQueue(cmd);
}

OSCL_EXPORT_REF   PVMFCommandId PVRTSPEngineNode::Reset(PVMFSessionId aSession
        , const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::Reset() called"));

    PVRTSPEngineCommand cmd;
    cmd.PVRTSPEngineCommandBase::Construct(aSession, PVMF_GENERIC_NODE_RESET, aContext);
    return AddCmdToQueue(cmd);
}

OSCL_EXPORT_REF PVMFCommandId PVRTSPEngineNode::Flush(PVMFSessionId aSession, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::Flush() called"));

    PVRTSPEngineCommand cmd;
    cmd.PVRTSPEngineCommandBase::Construct(aSession, PVMF_GENERIC_NODE_FLUSH, aContext);
    return AddCmdToQueue(cmd);
}

OSCL_EXPORT_REF   PVMFCommandId PVRTSPEngineNode::CancelCommand(PVMFSessionId aSession
        , PVMFCommandId aCmdId
        , const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::CancelCommand() called"));
    //OSCL_LEAVE(OsclErrNotSupported);
    PVRTSPEngineCommand cmd;
    cmd.PVRTSPEngineCommandBase::Construct(aSession, PVMF_GENERIC_NODE_CANCELCOMMAND, aCmdId, aContextData);
    return AddCmdToQueue(cmd);
}

OSCL_EXPORT_REF   PVMFCommandId PVRTSPEngineNode::CancelAllCommands(PVMFSessionId aSession
        , const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::CancelAllCommands() called"));

    //OSCL_LEAVE(OsclErrNotSupported);
    PVRTSPEngineCommand cmd;
    cmd.PVRTSPEngineCommandBase::Construct(aSession, PVMF_GENERIC_NODE_CANCELALLCOMMANDS, aContextData);
    return AddCmdToQueue(cmd);
}
//************ end PVMFNodeInterface

//************ begin PVRTSPEngineNodeExtensionInterface
OSCL_EXPORT_REF	PVMFStatus PVRTSPEngineNode::SetStreamingType(PVRTSPStreamingType aType)
{
    iSessionInfo.iStreamingType = aType;
    return PVMFSuccess;
}

OSCL_EXPORT_REF  PVMFStatus PVRTSPEngineNode::SetSessionURL(OSCL_wString& aURL)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::SetSessionURL() called"));
    if (iInterfaceState == EPVMFNodeIdle)
    {
        if (parseURL(aURL))
        {
            iSessionInfo.bExternalSDP = false;
            return PVMFSuccess;
        }
    }
    iSessionInfo.iSessionURL = "";
    return PVMFFailure;
}

OSCL_EXPORT_REF PVMFStatus PVRTSPEngineNode::SetRtspProxy(OSCL_String& aRtspProxyName, uint32 aRtspProxyPort)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::SetRtspProxy() aRtspProxy %s %d", aRtspProxyName.get_cstr(), aRtspProxyPort));

    //If proxy is in use, both the name and the port have to be set.
    if ((0 == aRtspProxyName.get_size())
            || (0 == aRtspProxyPort)
            || (iInterfaceState != EPVMFNodeIdle))
    {
        return PVMFFailure;
    }

    {
        iSessionInfo.iProxyName = aRtspProxyName;
        iSessionInfo.iProxyPort = aRtspProxyPort;

        return PVMFSuccess;
    }
}
OSCL_EXPORT_REF  PVMFStatus PVRTSPEngineNode::GetRtspProxy(OSCL_String& aRtspProxyName, uint32& aRtspProxyPort)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::GetRtspProxy() RtspProxy %s %d", iSessionInfo.iProxyName.get_cstr(), iSessionInfo.iProxyPort));
    aRtspProxyName = iSessionInfo.iProxyName;
    aRtspProxyPort = iSessionInfo.iProxyPort;
    return PVMFSuccess;
}

OSCL_EXPORT_REF PVMFStatus PVRTSPEngineNode::SetSDPInfo(OsclSharedPtr<SDPInfo>& aSDPinfo, Oscl_Vector<StreamInfo, PVRTSPEngineNodeAllocator> &aSelectedStream)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::SetSDPInfo() called"));

    if ((iInterfaceState == EPVMFNodePrepared) ||
            (iInterfaceState == EPVMFNodeInitialized) ||
            (iInterfaceState == EPVMFNodeIdle))
    {
        if (iState == PVRTSP_ENGINE_NODE_STATE_DESCRIBE_DONE)
        {
            iSessionInfo.bExternalSDP = false;
        }
        else if (iState == PVRTSP_ENGINE_NODE_STATE_IDLE)
        {
            iSessionInfo.bExternalSDP = true;
        }
        else
        {
            return PVMFErrInvalidState;
        }

        iSessionInfo.iSDPinfo = aSDPinfo;
        iSessionInfo.iSelectedStream = aSelectedStream;

        if (iSessionInfo.bExternalSDP)
        {
            //set the server address
            const char *servURL = (aSDPinfo->getSessionInfo())->getControlURL();
            uint32 servURLLen = oscl_strlen(servURL);
            if (servURLLen >= iRTSPEngTmpBuf.len)
            {
                //we do not support URLs larger than RTSP_MAX_FULL_REQUEST_SIZE
                //iRTSPEngTmpBuf.len is initialized to RTSP_MAX_FULL_REQUEST_SIZE
                return PVMFFailure;
            }
            oscl_memset(iRTSPEngTmpBuf.ptr, 0, iRTSPEngTmpBuf.len);
            oscl_strncpy((mbchar*)iRTSPEngTmpBuf.ptr, servURL, servURLLen);
            if (!parseURL((mbchar*)iRTSPEngTmpBuf.ptr))
            {
                return PVMFFailure;
            }
        }
        return PVMFSuccess;
    }
    return PVMFErrInvalidState;
}

OSCL_EXPORT_REF PVMFStatus PVRTSPEngineNode::GetSDP(OsclRefCounterMemFrag& aSDPMemFrag)
{
    aSDPMemFrag = iSessionInfo.pSDPBuf;
    return PVMFSuccess;
}

OSCL_EXPORT_REF PVMFStatus PVRTSPEngineNode::GetServerInfo(PVRTSPEngineNodeServerInfo& aServerInfo)
{
    aServerInfo.iServerName = iSessionInfo.iServerName;
    aServerInfo.iIsPVServer = iSessionInfo.pvServerIsSetFlag;
    aServerInfo.iRoundTripDelayInMS = iSessionInfo.roundTripDelay;
    aServerInfo.iServerVersionNumber = iSessionInfo.iServerVersionNumber;
    return PVMFSuccess;
}

OSCL_EXPORT_REF PVMFStatus PVRTSPEngineNode::GetStreamInfo(Oscl_Vector<StreamInfo, PVRTSPEngineNodeAllocator> &aSelectedStream)
{
    aSelectedStream = iSessionInfo.iSelectedStream;
    return PVMFSuccess;
}

OSCL_EXPORT_REF PVMFStatus PVRTSPEngineNode::GetUserAgent(OSCL_wString& aUserAgent)
{
    uint32 tmpSize = iSessionInfo.iUserAgent.get_size();
    tmpSize += 8;
    int32 err;
    oscl_wchar *tmpBuf = NULL;
    OSCL_TRY(err, tmpBuf = OSCL_ARRAY_NEW(oscl_wchar, (tmpSize)););
    if ((err != OsclErrNone) || (tmpBuf == NULL))
    {
        return PVMFFailure;
    }

    if (0 == oscl_UTF8ToUnicode(iSessionInfo.iUserAgent.get_cstr(), iSessionInfo.iUserAgent.get_size(), (oscl_wchar*)tmpBuf, (tmpSize*sizeof(oscl_wchar))))
    {
        OSCL_ARRAY_DELETE(tmpBuf);
        return PVMFFailure;
    }

    aUserAgent = tmpBuf;

    OSCL_ARRAY_DELETE(tmpBuf);
    return PVMFSuccess;
}

OSCL_EXPORT_REF  PVMFStatus PVRTSPEngineNode::SetClientParameters(OSCL_wString& aUserAgent,
        OSCL_wString&  aUserNetwork,
        OSCL_wString&  aDeviceInfo)
{
    uint32 tmpSize = aUserAgent.get_size();
    if (tmpSize < aUserNetwork.get_size())
    {
        tmpSize = aUserNetwork.get_size();
    }
    if (tmpSize < aDeviceInfo.get_size())
    {
        tmpSize = aDeviceInfo.get_size();
    }
    tmpSize += 8;

    int32 err;
    uint8 *tmpBuf = NULL;
    OSCL_TRY(err, tmpBuf = OSCL_ARRAY_NEW(uint8, (tmpSize)););
    if ((err != OsclErrNone) || (tmpBuf == NULL))
    {
        return PVMFFailure;
    }

    if (aUserAgent.get_size() > 0)
    {
        if (0 == oscl_UnicodeToUTF8(aUserAgent.get_cstr(), aUserAgent.get_size(), (char*)tmpBuf, tmpSize))
        {
            OSCL_ARRAY_DELETE(tmpBuf);
            return PVMFFailure;
        }
        //\engines\player\src\pv_player_sdkinfo.h
        //#define PVPLAYER_ENGINE_SDKINFO_LABEL "PVPLAYER 04.07.00.01"
        //iSessionInfo.iUserAgent = PVPLAYER_ENGINE_SDKINFO_LABEL;
        //iSessionInfo.iUserAgent += (char*)tmpBuf;
        iSessionInfo.iUserAgent = (char*)tmpBuf;
    }

    if (aUserNetwork.get_size() > 0)
    {
        if (0 == oscl_UnicodeToUTF8(aUserNetwork.get_cstr(), aUserNetwork.get_size(), (char*)tmpBuf, tmpSize))
        {
            OSCL_ARRAY_DELETE(tmpBuf);
            return PVMFFailure;
        }
        iSessionInfo.iUserNetwork = (char*)tmpBuf;
    }

    if (aDeviceInfo.get_size() > 0)
    {
        if (0 == oscl_UnicodeToUTF8(aDeviceInfo.get_cstr(), aDeviceInfo.get_size(), (char*)tmpBuf, tmpSize))
        {
            OSCL_ARRAY_DELETE(tmpBuf);
            return PVMFFailure;
        }
        iSessionInfo.iDeviceInfo = (char*)tmpBuf;
    }

    OSCL_ARRAY_DELETE(tmpBuf);
    return PVMFSuccess;
}

OSCL_EXPORT_REF bool PVRTSPEngineNode::IsRdtTransport()
{
    return ibIsRealRDT;
}


OSCL_EXPORT_REF void PVRTSPEngineNode::SetPortRdtStreamId(PVMFPortInterface* pPort,
        int iRdtStreamId)
{
    ((PVMFRTSPPort*)pPort)->iRdtStreamId = iRdtStreamId;
}

OSCL_EXPORT_REF void PVRTSPEngineNode::SetRealChallengeCalculator(IRealChallengeGen* pChallengeCalc)
{
    ipRealChallengeGen = pChallengeCalc;
}

OSCL_EXPORT_REF void PVRTSPEngineNode::SetRdtParser(IPayloadParser* pRdtParser)
{
    ipRdtParser = pRdtParser;
}

OSCL_EXPORT_REF  PVMFStatus PVRTSPEngineNode::SetAuthenticationParameters(OSCL_wString& aUserID,
        OSCL_wString& aAuthentication,
        OSCL_wString& aExpiration,
        OSCL_wString& aApplicationSpecificString,
        OSCL_wString& aVerification,
        OSCL_wString& aSignature)
{
    uint32 tmpSize = aUserID.get_size();
    if (tmpSize < aAuthentication.get_size())
    {
        tmpSize = aAuthentication.get_size();
    }
    if (tmpSize < aExpiration.get_size())
    {
        tmpSize = aExpiration.get_size();
    }
    if (tmpSize < aApplicationSpecificString.get_size())
    {
        tmpSize = aApplicationSpecificString.get_size();
    }
    if (tmpSize < aVerification.get_size())
    {
        tmpSize = aVerification.get_size();
    }
    if (tmpSize < aSignature.get_size())
    {
        tmpSize = aSignature.get_size();
    }
    tmpSize += 8;

    int32 err;
    uint8 *tmpBuf = NULL;
    OSCL_TRY(err, tmpBuf = OSCL_ARRAY_NEW(uint8, (tmpSize)););
    if ((err != OsclErrNone) || (tmpBuf == NULL))
    {
        return PVMFFailure;
    }

    if (0 == oscl_UnicodeToUTF8(aUserID.get_cstr(), aUserID.get_size(), (char*)tmpBuf, tmpSize))
    {
        OSCL_ARRAY_DELETE(tmpBuf);
        return PVMFFailure;
    }
    iSessionInfo.iUserID = (char*)tmpBuf;

    if (0 == oscl_UnicodeToUTF8(aAuthentication.get_cstr(), aAuthentication.get_size(), (char*)tmpBuf, tmpSize))
    {
        OSCL_ARRAY_DELETE(tmpBuf);
        return PVMFFailure;
    }
    iSessionInfo.iAuthentication = (char*)tmpBuf;

    if (0 == oscl_UnicodeToUTF8(aExpiration.get_cstr(), aExpiration.get_size(), (char*)tmpBuf, tmpSize))
    {
        OSCL_ARRAY_DELETE(tmpBuf);
        return PVMFFailure;
    }
    iSessionInfo.iExpiration = (char*)tmpBuf;

    if (0 == oscl_UnicodeToUTF8(aApplicationSpecificString.get_cstr(), aApplicationSpecificString.get_size(), (char*)tmpBuf, tmpSize))
    {
        OSCL_ARRAY_DELETE(tmpBuf);
        return PVMFFailure;
    }
    iSessionInfo.iApplicationSpecificString = (char*)tmpBuf;

    if (0 == oscl_UnicodeToUTF8(aVerification.get_cstr(), aVerification.get_size(), (char*)tmpBuf, tmpSize))
    {
        OSCL_ARRAY_DELETE(tmpBuf);
        return PVMFFailure;
    }
    iSessionInfo.iVerification = (char*)tmpBuf;

    if (0 == oscl_UnicodeToUTF8(aSignature.get_cstr(), aSignature.get_size(), (char*)tmpBuf, tmpSize))
    {
        OSCL_ARRAY_DELETE(tmpBuf);
        return PVMFFailure;
    }
    iSessionInfo.iSignature = (char*)tmpBuf;

    OSCL_ARRAY_DELETE(tmpBuf);

    return PVMFSuccess;
}

OSCL_EXPORT_REF PVMFStatus PVRTSPEngineNode::SetRequestPlayRange(const RtspRangeType& aRange)
{
    if (aRange.format == RtspRangeType::NPT_RANGE)
    {//only accept npt for now.
        iSessionInfo.iReqPlayRange = aRange;
        iSessionInfo.iActPlayRange.format = RtspRangeType::INVALID_RANGE;
        if (PVRTSP_ENGINE_NODE_STATE_PAUSE_DONE == iState)
        {
            bRepositioning =  true;
        }
        return PVMFSuccess;
    }
    return PVMFFailure;
}

OSCL_EXPORT_REF PVMFStatus PVRTSPEngineNode::GetActualPlayRange(RtspRangeType& aRange)
{
    aRange = iSessionInfo.iActPlayRange;
    if (iSessionInfo.iActPlayRange.format == RtspRangeType::INVALID_RANGE)
    {
        return PVMFFailure;
    }
    return PVMFSuccess;
}

//************ end PVRTSPEngineNodeExtensionInterface

//************ begin OsclSocketObserver
OSCL_EXPORT_REF  void PVRTSPEngineNode::HandleSocketEvent(int32 aId, TPVSocketFxn aFxn, TPVSocketEvent aEvent, int32 aError)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::HandleSocketEvent() In aId=%d, aFxn=%d, aEvent=%d, aError=%d", aId, aFxn, aEvent, aError));

    //update socket container state.
    //note we only update iRecvSocket container when it's a unique socket.
    SocketContainer* container;
    switch (aId)
    {
        case REQ_RECV_SOCKET_ID:
            container = &iRecvSocket;
            break;
        case REQ_SEND_SOCKET_ID:
            container = &iSendSocket;
            break;
        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVRTSPEngineNode::HandleSocketEvent() ERROR invalid aId=%d", aId));
            return;
    }
    //clear the appropriate cmd pending & canceled flags.
    switch (aFxn)
    {
        case EPVSocketConnect:
            container->iConnectState.Reset();
            OSCL_ASSERT(iNumConnectCallback > 0);
            iNumConnectCallback--;
            break;
        case EPVSocketRecv:
            container->iRecvState.Reset();
            OSCL_ASSERT(iNumRecvCallback > 0);
            iNumRecvCallback--;
            break;
        case EPVSocketSend:
            container->iSendState.Reset();
            OSCL_ASSERT(iNumSendCallback > 0);
            iNumSendCallback--;
            break;
        case EPVSocketShutdown:
            container->iShutdownState.Reset();
            break;
        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVRTSPEngineNode::HandleSocketEvent() ERROR invalid aFxn=%d", aFxn));
            return;
    }

    if (!IsAdded())
    {//prevent the leave 49. should never get here
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVRTSPEngineNode::HandleSocketEvent() ERROR line %d", __LINE__));
        return;
    }

    //For socket cleanup sequence including Stop & Reset command
    if (iSocketCleanupState != ESocketCleanup_Idle)
    {
        RunIfNotReady();
        return;
    }

    //save info
    SocketEvent tmpSockEvent;
    tmpSockEvent.iSockId	= aId;
    tmpSockEvent.iSockFxn = aFxn;
    tmpSockEvent.iSockEvent = aEvent;
    tmpSockEvent.iSockError = aError;

    if (aFxn == EPVSocketRecv)
    {
        bNoRecvPending = true;
        if (EPVSocketSuccess == aEvent)
        {
            int32 incomingMessageLen;
            uint8* recvData = iRecvSocket.iSocket->GetRecvData(&incomingMessageLen);
            OSCL_UNUSED_ARG(recvData);

#ifdef MY_RTSP_DEBUG
            {
                const uint32 dbgBufSize = 256;
                uint8* dbgBuf = OSCL_ARRAY_NEW(uint8, dbgBufSize);
                if (dbgBuf)	oscl_memcpy(dbgBuf, recvData, dbgBufSize - 1);
                dbgBuf[dbgBufSize-1] = '\0';
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "C <--- S\n%s", dbgBuf));
                OSCL_ARRAY_DELETE(dbgBuf);
            }
#endif

            if (incomingMessageLen > 0)
            {
                if (!iRTSPParser->registerDataBufferWritten(incomingMessageLen))
                {//parser some kind of error on Engine's part, or Parser's internal inconsistency
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVRTSPEngineNode::HandleSocketEvent() registerDataBufferWritten() error"));
                    iRTSPParser->flush();
                }
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO, (0, "PVRTSPEngineNode::HandleSocketEvent() In incomingMessageLen=%d", incomingMessageLen));
            }
            RunIfNotReady();
            return; // for recv success, process is done
        }
    }
    else if (aFxn == EPVSocketSend)
    {
        if (aId == REQ_RECV_SOCKET_ID)
        {//clear POST msg
            iRecvChannelMsg = "";
        }

        //TBD using send Q
        if ((bSrvRespPending) && (EPVSocketSuccess == aEvent))
        {//there is one resp waiting on queue because there was a send() pending
            bSrvRespPending = false;

            if (PVMFSuccess != sendSocketOutgoingMsg(iSendSocket, *iSrvResponse))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVRTSPEngineNode::HandleSocketEvent() sendSocketOutgoingMsg() error"));
            }
        }
        else
        {
            bNoSendPending = true;
            if (iSrvResponse)
            {
                OSCL_DELETE(iSrvResponse);
                iSrvResponse = NULL;
            }
            if (iState == PVRTSP_ENGINE_NODE_STATE_DESCRIBE_DONE)
            {//wait first SETUP response before sending the remaining SETUPs
                bNoSendPending = false;
                return;//handling of this socketevent is done.
            }
        }
    }

    iSocketEventQueue.push_back(tmpSockEvent);
    RunIfNotReady();
}
//************ end OsclSocketObserver

//************ begin OsclDNSObserver
OSCL_EXPORT_REF void PVRTSPEngineNode::HandleDNSEvent(int32 aId, TPVDNSFxn aFxn, TPVDNSEvent aEvent, int32 aError)
{
    OSCL_UNUSED_ARG(aEvent);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::HandleDNSEvent() In aId=%d, aFxn=%d, aEvent=%d, aError=%d", aId, aFxn, aEvent, aError));

    //clear the cmd Pending and Canceled flags
    iDNS.iState.Reset();

    if (aFxn == EPVDNSGetHostByName)
    {
        OSCL_ASSERT(iNumHostCallback > 0);
        iNumHostCallback--;
    }
    if (!IsAdded())
    {//prevent the leave 49. should never get here
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVRTSPEngineNode::HandleDNSEvent() ERROR line %d", __LINE__));
        return;
    }

    //For socket cleanup sequence including Stop & Reset command
    if (iSocketCleanupState != ESocketCleanup_Idle)
    {
        RunIfNotReady();
        return;
    }

    if ((aId == REQ_DNS_LOOKUP_ID) && (aFxn == EPVDNSGetHostByName))
    {
        {//wrap the DNS event as an socket event
            SocketEvent tmpSockEvent;
            {//TBD type mismatch
                tmpSockEvent.iSockId	= aId;
                tmpSockEvent.iSockFxn = EPVSocketRecv; //aFxn;

                tmpSockEvent.iSockEvent = EPVSocketSuccess;	//aEvent;
                if (oscl_strlen((const char*)iSessionInfo.iSrvAdd.ipAddr.Str()) == 0)
                {
                    iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorDNSLookUpError;
                    tmpSockEvent.iSockEvent = EPVSocketFailure;	//aEvent;
                }
                tmpSockEvent.iSockError = aError;
            }

            iSocketEventQueue.push_back(tmpSockEvent);
        }

        ChangeInternalState(PVRTSP_ENGINE_NODE_STATE_CONNECT);
        RunIfNotReady();
        return;
    }
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::HandleDNSEvent() unsolicited event"));
}
//************ end OsclDNSObserver

void PVRTSPEngineNode::Run()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::Run() In"));

    //Drive the reset sequence
    if (iSocketCleanupState != ESocketCleanup_Idle)
    {
        if (resetSocket() == PVMFPending)
            return;//keep waiting on callbacks.
    }

    //Process commands.
    if (iPendingCmdQueue.size() > 0)
    {
        if (ProcessCommand(iPendingCmdQueue.front()))
        {
            if (IsAdded())
                RunIfNotReady();
            return;
        }
    }

    if (iRunningCmdQueue.size() > 0)
    {
        DispatchCommand(iRunningCmdQueue.front());
        if ((iPendingCmdQueue.size() > 0) && IsAdded())
        {
            RunIfNotReady();
        }
    }
    else
    {
        if (RTSPParser::REQUEST_IS_READY == iRTSPParserState)
        {
            PVMFStatus iRet = processIncomingMessage(iIncomingMsg);
            if ((iRet != PVMFPending) && (iRet != PVMFSuccess))
            {//TBD error handling.
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO, (0, "PVRTSPEngineNode::Run() ERROR processIncomingMessage(). Ln %d", __LINE__));
                //ChangeInternalState(PVRTSP_ENGINE_NODE_STATE_INVALID);
            }
        }
        else if (RTSPParser::ENTITY_BODY_IS_READY == iRTSPParserState)
        {//Incoming message also has an entity body
            PVMFStatus iRet = processEntityBody(iIncomingMsg, iEntityMemFrag);
            if ((iRet != PVMFPending) && (iRet != PVMFSuccess))
            {//TBD error handling.
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO, (0, "PVRTSPEngineNode::Run() ERROR processIncomingMessage(). Ln %d", __LINE__));
                //ChangeInternalState(PVRTSP_ENGINE_NODE_STATE_INVALID);
            }
        }
        else if (!clearEventQueue())
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVRTSPEngineNode::Run() ERROR Ln %d", __LINE__));

            iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorRTSPSocketConnectError;
            //TBD PVMFFailure;
            if (iErrorRecoveryAttempt-- <= 0)
            {
                ChangeExternalState(EPVMFNodeError);
                ReportInfoEvent(PVMFInfoStateChanged);
            }
            else
            {
                int32 err;
                PVRTSPErrorContext* errorContext = NULL;
                OSCL_TRY(err, errorContext = OSCL_NEW(PVRTSPErrorContext, ()));
                if (err || (errorContext ==  NULL))
                {
                    ChangeExternalState(EPVMFNodeError);
                }
                else
                {//send error info
                    errorContext->iErrState = iState;

                    ReportInfoEvent(PVMFInfoErrorHandlingStart);
                    partialResetSessionInfo();
                    clearOutgoingMsgQueue();
                    PVMFStatus status = resetSocket();

                    PVRTSPEngineCommand cmd;
                    //const OsclAny* aContext = OSCL_STATIC_CAST(OsclAny*, errorContext);
                    //cmd.PVRTSPEngineCommandBase::Construct(aCmd.iSession,PVMF_RTSP_NODE_ERROR_RECOVERY,aContext);
                    cmd.PVRTSPEngineCommandBase::Construct(0, PVMF_RTSP_NODE_ERROR_RECOVERY, NULL);
                    cmd.iParam1 = OSCL_STATIC_CAST(OsclAny*, errorContext);

                    iRunningCmdQueue.AddL(cmd);
                    if (status != PVMFPending)
                        RunIfNotReady();
                }
            }
        }
    }

    if (iInterfaceState == EPVMFNodeStarted || FlushPending())
    {
        while (!iPortActivityQueue.empty())
        {
            if (!ProcessPortActivity())
            {//error
                break;
            }
        }
    }
    if (FlushPending() && iPortActivityQueue.empty())
    {
        //If we get here we did not process any ports or commands.
        //Check for completion of a flush command...
        SetState(EPVMFNodePrepared);
        //resume port input so the ports can be re-started.
        for (uint32 i = 0;i < iPortVector.size();i++)
            iPortVector[i]->ResumeInput();
        CommandComplete(iRunningCmdQueue, iRunningCmdQueue.front(), PVMFSuccess);
        RunIfNotReady();
    }

    if (rtspParserLoop())
    {
        RunIfNotReady();
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::Run() Out"));
}


/**
//A routine to tell if a flush operation is in progress.
*/
bool PVRTSPEngineNode::FlushPending()
{
    return (iRunningCmdQueue.size() > 0
            && iRunningCmdQueue.front().iCmd == PVMF_GENERIC_NODE_FLUSH);
}

bool PVRTSPEngineNode::ProcessPortActivity()
{//called by the AO to process a port activity message
    //Pop the queue...
    PVMFPortActivity activity(iPortActivityQueue.front());
    iPortActivityQueue.erase(&iPortActivityQueue.front());

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "0x%x PVRTSPEngineNode::ProcessPortActivity: port=0x%x, type=%d",
                     this, activity.iPort, activity.iType));

    PVMFStatus status = PVMFSuccess;
    switch (activity.iType)
    {
        case PVMF_PORT_ACTIVITY_OUTGOING_MSG:
            if (NULL == iTheBusyPort)
            {
                if (activity.iPort->OutgoingMsgQueueSize() > 0)
                {
                    status = ProcessOutgoingMsg(activity.iPort);
                    //if there is still data, queue another port activity event.
                    if (status == PVMFSuccess
                            && activity.iPort->OutgoingMsgQueueSize() > 0)
                    {
                        QueuePortActivity(activity);
                    }
                }
            }
            break;
        case PVMF_PORT_ACTIVITY_INCOMING_MSG:
            break;

        default:
            break;
    }

    //report a failure in port processing...
    if (status != PVMFErrBusy && status != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "0x%x PVRTSPEngineNode::ProcessPortActivity() Error - ProcessPortActivity failed. port=0x%x, type=%d",
                         this, activity.iPort, activity.iType));
        ReportErrorEvent(PVMFErrPortProcessing);
        return false;
    }
    //return true if we processed an activity...
    //return (status!=PVMFErrBusy);
    return true;
}

/////////////////////////////////////////////////////
PVMFStatus PVRTSPEngineNode::ProcessOutgoingMsg(PVMFPortInterface* aPort)
{
    //Called by the AO to process one message off the outgoing
    //message queue for the given port.  This routine will
    //try to send the data to the connected port.

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "0x%x PVRTSPEngineNode::ProcessOutgoingMsg: aPort=0x%x", this, aPort));

    PVMFStatus status = aPort->Send();
    if (status == PVMFErrBusy)
    {
        //iTheBusyPort = true;
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "0x%x PVRTSPEngineNode::ProcessOutgoingMsg: Connected port busy", this));
    }

    return status;
}

/**
//Called by the command handler AO to process a command from
//the input queue.
//Return true if a command was processed, false if the command
//processor is busy and can't process another command now.
*/
bool PVRTSPEngineNode::ProcessCommand(PVRTSPEngineCommand& aInCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::ProcessCommand() in"));

    //don't interrupt a cancel command
    if (!iCancelCmdQueue.empty())
        return false;
    if (!iRunningCmdQueue.empty()
            && iRunningCmdQueue.front().iCmd == PVMF_RTSP_NODE_CANCELALLRESET)
        return false;

    //don't interrupt a running command unless this is hi-priority command
    //such as a cancel.
    if (iRunningCmdQueue.size() > 0 && !aInCmd.hipri())
        return false;

    {
        //move the command from the pending command queue to the
        //running command queue, where it will remain until it completes.
        int32 err;
        OSCL_TRY(err, iRunningCmdQueue.StoreL(aInCmd););

        if (err != OsclErrNone)
        {
            CommandComplete(iPendingCmdQueue, aInCmd, PVMFErrNoMemory);
            return PVMFErrNoMemory;
        }
        iPendingCmdQueue.Erase(&aInCmd);
    }

    DispatchCommand(iRunningCmdQueue.front());
    return true;
}

PVMFStatus PVRTSPEngineNode::DispatchCommand(PVRTSPEngineCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::DispatchCommand() in"));

    bool bRevertToPreviousStateImpossible = true;
    bool bErrorRecoveryImpossible = true;
    PVMFStatus iRet = PVMFFailure;
    switch (aCmd.iCmd)
    {
        case PVMF_GENERIC_NODE_QUERYUUID:
            iRet = DoQueryUuid(aCmd);
            break;

        case PVMF_GENERIC_NODE_INIT:
            iRet = DoInitNode(aCmd);
            if (iRet == PVMFSuccess)
            {
                ChangeExternalState(EPVMFNodeInitialized);
            }
            bErrorRecoveryImpossible = false;
            break;

        case PVMF_GENERIC_NODE_PREPARE:
            iRet = DoPrepareNode(aCmd);
            if (iRet == PVMFSuccess)
            {
                ChangeExternalState(EPVMFNodePrepared);
            }
            bErrorRecoveryImpossible = false;
            break;

        case PVMF_GENERIC_NODE_START:
            iRet = DoStartNode(aCmd);
            if (iRet == PVMFSuccess)
            {
                ChangeExternalState(EPVMFNodeStarted);
                if (bKeepAliveInPlay)
                {
                    //setup the timer for sending keep-alive
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO, (0, "PVRTSPEngineNode::DispatchCommand() start keep-alive timer %d. Ln %d", TIMEOUT_KEEPALIVE, __LINE__));
                    iWatchdogTimer->Request(REQ_TIMER_KEEPALIVE_ID, 0, TIMEOUT_KEEPALIVE);
                }
            }
            bErrorRecoveryImpossible = false;
            break;

        case PVMF_GENERIC_NODE_STOP:
            iRet = DoStopNode(aCmd);
            if (iRet == PVMFSuccess)
            {
                ChangeExternalState(EPVMFNodePrepared);
            }
            bRevertToPreviousStateImpossible = false;
            bErrorRecoveryImpossible = false;
            break;

        case PVMF_GENERIC_NODE_PAUSE:
            iRet = DoPauseNode(aCmd);
            if (iRet == PVMFSuccess)
            {
                //setup the timer for sending keep-alive
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO, (0, "PVRTSPEngineNode::DispatchCommand() start keep-alive timer %d. Ln %d", TIMEOUT_KEEPALIVE, __LINE__));
                iWatchdogTimer->Request(REQ_TIMER_KEEPALIVE_ID, 0, TIMEOUT_KEEPALIVE);

                ChangeExternalState(EPVMFNodePaused);
            }
            bRevertToPreviousStateImpossible = false;
            bErrorRecoveryImpossible = false;
            break;

        case PVMF_GENERIC_NODE_RESET:
        case PVMF_RTSP_NODE_CANCELALLRESET:
            iRet = DoResetNode(aCmd);
            if (iRet != PVMFPending)
            {
                OSCL_DELETE(iRTSPParser);
                iRTSPParser = NULL;

                partialResetSessionInfo();
                ResetSessionInfo();
                clearOutgoingMsgQueue();
                iRet = resetSocket();
            }
            break;

        case PVMF_GENERIC_NODE_QUERYINTERFACE:
            iRet = DoQueryInterface(aCmd);
            break;

        case PVMF_GENERIC_NODE_CANCELALLCOMMANDS:
            iRet = DoCancelAllCommands(aCmd);
            break;


        case PVMF_GENERIC_NODE_FLUSH:
            iRet = DoFlush(aCmd);
            break;

        case PVMF_GENERIC_NODE_REQUESTPORT:
        {
            PVMFRTSPPort* aPort = NULL;
            iRet = DoRequestPort(aCmd, aPort);
            if (iRet == PVMFSuccess)
            {
                //Return the port pointer to the caller.
                CommandComplete(iRunningCmdQueue, aCmd, iRet, (OsclAny*)aPort);
                return iRet;
            }
        }
        break;

        case PVMF_GENERIC_NODE_RELEASEPORT:
            iRet = DoReleasePort(aCmd);
            break;

        case PVMF_RTSP_NODE_ERROR_RECOVERY:
            iRet = DoErrorRecovery(aCmd);
            if (iRet != PVMFPending)
            {
                if ((iRet != PVMFSuccess) && (iErrorRecoveryAttempt-- > 0))
                {//retry
                    partialResetSessionInfo();
                    clearOutgoingMsgQueue();
                    iRet = resetSocket();
                    if (iRet != PVMFPending)
                        RunIfNotReady();
                    return PVMFPending;
                }

                if ((iRet == PVMFSuccess) && (iState == PVRTSP_ENGINE_NODE_STATE_PAUSE_DONE))
                {
                    //setup the timer for sending keep-alive
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO, (0, "PVRTSPEngineNode::DispatchCommand() start keep-alive timer %d. Ln %d", TIMEOUT_KEEPALIVE, __LINE__));
                    iWatchdogTimer->Request(REQ_TIMER_KEEPALIVE_ID, 0, TIMEOUT_KEEPALIVE);

                    ChangeExternalState(EPVMFNodePaused);
                    ReportInfoEvent(PVMFInfoStateChanged);
                }

                PVRTSPErrorContext* errorContext = OSCL_STATIC_CAST(PVRTSPErrorContext*, aCmd.iParam1);
                if (errorContext)
                {
                    OSCL_DELETE(errorContext);
                }
                //Erase the cmd from iRunningCmdQueue
                iRunningCmdQueue.Erase(&aCmd);


                if (iRunningCmdQueue.size() > 0)
                {//error happened while servicing an reqeust, resume service
                    //RunIfNotReady();
                    aCmd = iRunningCmdQueue.front();
                    if (iRet == PVMFSuccess)
                    {
                        iRet = PVMFPending;
                        //TBD
                        //this RunIfNotReady() is only needed when no event pending
                        // like Prepare() fails in PVRTSP_ENGINE_NODE_STATE_DESCRIBE_DONE state
                        RunIfNotReady();
                    }
                }
                else
                {//do report event
                    if (iRet != PVMFSuccess)
                    {
                        ChangeExternalState(EPVMFNodeError);
                        ReportInfoEvent(PVMFInfoStateChanged);
                    }
                    iRet = PVMFPending; ////internal cmd, no CommandComplete needed
                }
                ReportInfoEvent(PVMFInfoErrorHandlingComplete);
            }
            break;

        default://unknown command type
            iRet = PVMFFailure;
            break;
    }

    if (iRet != PVMFPending)
    {
        if (iRet != PVMFSuccess)
        {
            if (bRevertToPreviousStateImpossible)
            {
                ///////////////////////////////////////////////////////////////////////////
                //
                // Added 5/9/2006 to disable error recovery. Error recovery needs to become
                // configurable; until then, we'll disable it
                //
                //bErrorRecoveryImpossible = true;
                ///////////////////////////////////////////////////////////////////////////

                if ((bErrorRecoveryImpossible) || (iErrorRecoveryAttempt-- <= 0))
                {
                    ChangeExternalState(EPVMFNodeError);
                    ReportInfoEvent(PVMFInfoStateChanged);
                }
                else
                {
                    int32 err;
                    PVRTSPErrorContext* errorContext = NULL;
                    OSCL_TRY(err, errorContext = OSCL_NEW(PVRTSPErrorContext, ()));
                    if (err || (errorContext ==  NULL))
                    {
                        iRet = PVMFFailure; // reinitialized since it may be clobbered by OSCL_TRY()
                        ChangeExternalState(EPVMFNodeError);
                    }
                    else
                    {//send error info
                        errorContext->iErrState = iState;

                        ReportInfoEvent(PVMFInfoErrorHandlingStart);
                        partialResetSessionInfo();
                        clearOutgoingMsgQueue();
                        PVMFStatus status = resetSocket();

                        iState = PVRTSP_ENGINE_NODE_STATE_IDLE;

                        PVRTSPEngineCommand cmd;
                        //const OsclAny* aContext = OSCL_STATIC_CAST(OsclAny*, errorContext);
                        //cmd.PVRTSPEngineCommandBase::Construct(aCmd.iSession,PVMF_RTSP_NODE_ERROR_RECOVERY,aContext);
                        cmd.PVRTSPEngineCommandBase::Construct(aCmd.iSession, PVMF_RTSP_NODE_ERROR_RECOVERY, NULL);
                        cmd.iParam1 = OSCL_STATIC_CAST(OsclAny*, errorContext);

                        iRunningCmdQueue.AddL(cmd);

                        if (status != PVMFPending)
                            RunIfNotReady();
                        return PVMFPending;
                    }
                }
            }
            if (iCurrentErrorCode != PVMFRTSPClientEngineNodeErrorEventStart)
            {
                CommandComplete(iRunningCmdQueue, aCmd, iRet, NULL, &iEventUUID, &iCurrentErrorCode);
                /* Reset error code */
                iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorEventStart;
                return iRet;
            }
        }
        CommandComplete(iRunningCmdQueue, aCmd, iRet);
    }
    return iRet;
}


/**
//The various command handlers call this when a command is complete.
*/
void PVRTSPEngineNode::CommandComplete(PVRTSPEngineNodeCmdQ& aCmdQ,
                                       PVRTSPEngineCommand& aCmd,
                                       PVMFStatus aStatus,
                                       OsclAny* aEventData,
                                       PVUuid* aEventUUID,
                                       int32* aEventCode)

{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode:CommandComplete Id %d Cmd %d Status %d Context %d Data %d"
                    , aCmd.iId, aCmd.iCmd, aStatus, aCmd.iContext, aEventData));

    //Do special handling of some commands.
    switch (aCmd.iCmd)
    {

        case PVMF_RTSP_NODE_CANCELALLRESET:
            //restore command ID
            aCmd.iCmd = PVMF_GENERIC_NODE_CANCELALLCOMMANDS;
            break;

        case PVMF_GENERIC_NODE_RESET:
            if (aStatus == PVMFSuccess)
                ChangeExternalState(EPVMFNodeIdle);
            ThreadLogoff();
            break;

        case PVMF_GENERIC_NODE_CANCELALLCOMMANDS:
            //Add a reset sequence to the end of "Cancel all" processing, in order to
            //satisfy the expectation of streaming manager node.
        {
            //change the command type to "cancelallreset"
            aCmd.iCmd = PVMF_RTSP_NODE_CANCELALLRESET;
            //move command from cancel command queue to running command queue
            //if necessary.  we do this because this node is only setup to
            //continue processing commands in the running queue.
            if (&aCmdQ == &iCancelCmdQueue)
            {
                iRunningCmdQueue.StoreL(aCmd);
                aCmdQ.Erase(&aCmd);
            }
            RunIfNotReady();
            return;
        }
        default:
            break;
    }

    PVInterface* extif = NULL;
    PVMFBasicErrorInfoMessage* errormsg = NULL;
    if (aEventUUID && aEventCode)
    {
        errormsg = OSCL_NEW(PVMFBasicErrorInfoMessage, (*aEventCode, *aEventUUID, NULL));
        extif = OSCL_STATIC_CAST(PVInterface*, errormsg);
    }

    //create response
    PVMFCmdResp resp(aCmd.iId, aCmd.iContext, aStatus, extif, aEventData);
    PVMFSessionId session = aCmd.iSession;

    //Erase the command from the queue.
    aCmdQ.Erase(&aCmd);

    //Report completion to the session observer.
    ReportCmdCompleteEvent(session, resp);

    if (errormsg)
    {
        errormsg->removeRef();
    }

    //There may be a cancel command that was just waiting on the running command to finish.
    //If so, complete the cancel command now.
    if (&aCmdQ == &iRunningCmdQueue
            && !iCancelCmdQueue.empty())
    {
        CommandComplete(iCancelCmdQueue, iCancelCmdQueue.front(), PVMFSuccess);
    }
}

// Handle command and data events
PVMFCommandId PVRTSPEngineNode::AddCmdToQueue(PVRTSPEngineCommand& aCmd)
{
    PVMFCommandId id;

    id = iPendingCmdQueue.AddL(aCmd);

    //wakeup the AO
    RunIfNotReady();

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::AddCmdToQueue() Cmd Id %d", id));
    return id;
}

void PVRTSPEngineNode::ChangeExternalState(TPVMFNodeInterfaceState aNewState)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::ChangeExternalState() Old %d New %d", iInterfaceState, aNewState));
    iInterfaceState = aNewState;
}

void PVRTSPEngineNode::ChangeInternalState(PVRTSPEngineState aNewState)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::ChangeInternalState() Old %d New %d", iState, aNewState));
    iState = aNewState;
}

PVMFStatus PVRTSPEngineNode::DoInitNode(PVRTSPEngineCommand &aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::DoInitNode() In"));
    OSCL_UNUSED_ARG(aCmd);

    if (iInterfaceState != EPVMFNodeIdle)
    {
        return PVMFErrInvalidState;
    }

    if (bAddXStrHeader)
    {
        // create iGetPostCorrelationObject
        if (!iGetPostCorrelationObject)
        {
            if ((iGetPostCorrelationObject = GetPostCorrelationObject::create()) == NULL) return PVMFFailure;
        }
    }

    return SendRtspDescribe(aCmd);
}

PVMFStatus PVRTSPEngineNode::SendRtspDescribe(PVRTSPEngineCommand &aCmd)
{
    OSCL_UNUSED_ARG(aCmd);

    PVMFStatus iRet = PVMFPending;
    switch (iState)
    {
        case PVRTSP_ENGINE_NODE_STATE_IDLE:
        {
            if (iSockServ == NULL)
            {
                int32 err;
                OSCL_TRY(err, iSockServ = OsclSocketServ::NewL(iAlloc););
                if (err || (iSockServ ==  NULL))
                {
                    iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorSocketServerError;
                    iRet =  PVMFFailure;
                    break;
                }
                if (iSockServ->Connect() != OsclErrNone)
                {
                    iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorSocketServerError;
                    iRet =  PVMFFailure;
                    break;
                }
            }

            if (!iRTSPParser)
            {
                iRTSPParser = OSCL_NEW(RTSPParser, ());
                if (NULL == iRTSPParser)
                {
                    iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorRTSPParserError;
                    iRet =  PVMFFailure;
                    break;
                }
            }
            iRTSPParser->flush();

            // 1. Do DNS look up if needed.
            OSCL_HeapString<PVRTSPEngineNodeAllocator> endPointName = iSessionInfo.iServerName;
            if (iSessionInfo.iProxyName.get_size())
            {
                iSessionInfo.iSrvAdd.port = iSessionInfo.iProxyPort;
                endPointName = iSessionInfo.iProxyName;
            }

            if (OsclValidInetAddr(endPointName.get_cstr()))
            {//ip address
                iSessionInfo.iSrvAdd.ipAddr.Set(endPointName.get_cstr());
                ChangeInternalState(PVRTSP_ENGINE_NODE_STATE_CONNECT);
                RunIfNotReady();
            }
            else
            {//dns lookup
                if (NULL == iDNS.iDns)
                {
                    REQ_DNS_LOOKUP_ID =  ++BASE_REQUEST_ID;
                    iDNS.iDns = OsclDNS::NewL(iAlloc, *iSockServ, *this, REQ_DNS_LOOKUP_ID);
                }
                if (iDNS.iDns == NULL)
                {
                    iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorDNSLookUpError;
                    iRet =  PVMFFailure;
                    break;
                }
                iDNS.iState.Reset();

                iSessionInfo.iSrvAdd.ipAddr.Set("");
                if (EPVDNSPending != iDNS.iDns->GetHostByName(endPointName.get_str(), iSessionInfo.iSrvAdd, TIMEOUT_CONNECT_AND_DNS_LOOKUP))
                {
                    iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorDNSLookUpError;
                    iRet =  PVMFFailure;
                    break;
                }
                iDNS.iState.iPending = true;
                iNumHostCallback++;
                ChangeInternalState(PVRTSP_ENGINE_NODE_STATE_DNS_RESOLVING);
            }
            break;
        }

        case PVRTSP_ENGINE_NODE_STATE_DNS_RESOLVING:
        {
            break;
        }

        case PVRTSP_ENGINE_NODE_STATE_CONNECT:
        {
            if (!clearEventQueue())
            {
                //cancell the watchdog
                iWatchdogTimer->Cancel(REQ_TIMER_WATCHDOG_ID);

                iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorDNSLookUpError;
                iRet =  PVMFFailure;
                break;
            }
            {
                //Allocate 1 TCP socket and set both iSendSocket and iRecvSocket to that socket.
                //Note: in this case we only track the status in the "send" container since
                //it's really only one socket.
                int32 err;
                OsclTCPSocket *sock = NULL;
                OSCL_TRY(err, sock = OsclTCPSocket::NewL(iAlloc, *iSockServ, this, REQ_SEND_SOCKET_ID););
                if (err || (sock ==  NULL))
                {
                    iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorRTSPSocketCreateError;
                    iRet =  PVMFFailure;
                    break;
                }
                iRecvSocket.Reset(sock);
                iSendSocket.Reset(sock);

                //proxy support
                //OSCL_StackString<64> tmpServerName = _STRLIT_CHAR("172.16.2.145");
                //iSessionInfo.iSrvAdd.ipAddr.Set( tmpServerName.get_cstr() );

                TPVSocketEvent sendConnect = iSendSocket.iSocket->Connect(iSessionInfo.iSrvAdd, TIMEOUT_CONNECT_AND_DNS_LOOKUP);
                if (sendConnect == EPVSocketPending)
                    iSendSocket.iConnectState.iPending = true;
                if (sendConnect != EPVSocketPending)
                {
                    iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorRTSPSocketConnectError;
                    iRet =  PVMFFailure;
                    break;
                }
                iNumConnectCallback++;
            }

            ChangeInternalState(PVRTSP_ENGINE_NODE_STATE_CONNECTING);
            break;
        }
        case PVRTSP_ENGINE_NODE_STATE_CONNECTING:
        {
            uint32 numSockEvents = (PVRTSP_RM_HTTP == iSessionInfo.iStreamingType) ? 2 : 1;
            if (iSocketEventQueue.size() < numSockEvents)
            {
                break;
            }

            do
            {
                SocketEvent tmpSockEvent(iSocketEventQueue.front());
                iSocketEventQueue.erase(&iSocketEventQueue.front());

                if (tmpSockEvent.iSockEvent != EPVSocketSuccess)
                {
                    iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorRTSPSocketConnectError;
                    iRet =  PVMFFailure;
                    break;
                }
                if (tmpSockEvent.iSockFxn != EPVSocketConnect)
                {	//unsolicited socket event
                    //break;
                    continue;
                }
                if (tmpSockEvent.iSockId == REQ_RECV_SOCKET_ID)
                {
                    bNoRecvPending = true;
                }
                else if (tmpSockEvent.iSockId == REQ_SEND_SOCKET_ID)
                {
                    bNoSendPending = true;
                }

                if (PVRTSP_RM_HTTP != iSessionInfo.iStreamingType)
                {
                    bNoSendPending = bNoRecvPending = true;
                }
            }
            while (!iSocketEventQueue.empty());

            if (!(bNoSendPending && bNoRecvPending))
            {
                break;
            }

            REQ_TIMER_WATCHDOG_ID =  ++BASE_REQUEST_ID;
            REQ_TIMER_KEEPALIVE_ID =  ++BASE_REQUEST_ID;
            if (iSessionInfo.iSDPinfo.GetRep() != NULL)
            {//if sdp is available
                ChangeInternalState(PVRTSP_ENGINE_NODE_STATE_DESCRIBE_DONE);
                iRet =  PVMFSuccess;
                break;
            }
            else
            {//if sdp is not available
                ChangeInternalState(PVRTSP_ENGINE_NODE_STATE_SEND_OPTIONS);
            }
            //DO NOT break, continue to send DESCRIBE
            RunIfNotReady();
            break;
        }
        case PVRTSP_ENGINE_NODE_STATE_HTTP_CLOAKING_SETUP:
        {//send the GET and POST requests.
            if (!clearEventQueue())
            {
                //cancell the watchdog
                iWatchdogTimer->Cancel(REQ_TIMER_WATCHDOG_ID);

                iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorRTSPSocketConnectError;
                iRet =  PVMFFailure;
                break;
            }

            if (RTSPParser::REQUEST_IS_READY != iRTSPParserState)
                break;

            {
                iRet = processIncomingMessage(iIncomingMsg);
                if (iRet != PVMFPending)
                {
                    //cancell the watchdog
                    iWatchdogTimer->Cancel(REQ_TIMER_WATCHDOG_ID);
                }
                if (iRet == PVMFSuccess)
                {
                    iRet = PVMFPending;
                }
                else
                {//either pending or error
                    break;
                }
            }

            if (iSessionInfo.iSDPinfo.GetRep() != NULL)
            {//if sdp is available
                ChangeInternalState(PVRTSP_ENGINE_NODE_STATE_DESCRIBE_DONE);
                iRet =  PVMFSuccess;
                break;
            }
            else
            {//if sdp is not available
                ChangeInternalState(PVRTSP_ENGINE_NODE_STATE_SEND_OPTIONS);
                //go to next case directly
                //RunIfNotReady(); break;
            }
        }

        case PVRTSP_ENGINE_NODE_STATE_SEND_OPTIONS:
        {
            if (bNoSendPending)
            {
                // send options
                RTSPOutgoingMessage *tmpOutgoingMsg = OSCL_NEW(RTSPOutgoingMessage, ());
                if (tmpOutgoingMsg == NULL)
                {
                    iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorOutOfMemory;
                    iRet =  PVMFFailure;
                    break;
                }

                if (PVMFSuccess != composeOptionsRequest(*tmpOutgoingMsg))
                {
                    iCurrentErrorCode =
                        PVMFRTSPClientEngineNodeErrorRTSPComposeOptionsRequestError;
                    OSCL_DELETE(tmpOutgoingMsg);
                    iRet =  PVMFFailure;
                    break;
                }

                if (PVMFSuccess != sendSocketOutgoingMsg(iSendSocket, *tmpOutgoingMsg))
                {
                    iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorSocketSendError;
                    OSCL_DELETE(tmpOutgoingMsg);
                    iRet =  PVMFFailure;
                    break;
                }

                bNoSendPending = false;
                iOutgoingMsgQueue.push(tmpOutgoingMsg);
                ChangeInternalState(PVRTSP_ENGINE_NODE_STATE_SEND_DESCRIBE);

                //setup the watch dog for server response
                iWatchdogTimer->Request(REQ_TIMER_WATCHDOG_ID, 0, TIMEOUT_WATCHDOG);
            }
            break;
        }

        case PVRTSP_ENGINE_NODE_STATE_SEND_DESCRIBE:
        {
            if (RTSPParser::REQUEST_IS_READY == iRTSPParserState)
            {
                iRet = processIncomingMessage(iIncomingMsg);
                if (iRet == PVMFSuccess)
                {
                    iRet = PVMFPending;
                }
                else
                {//either pending or error
                    if (iRet != PVMFPending)
                    {
                        //cancell the watchdog
                        iWatchdogTimer->Cancel(REQ_TIMER_WATCHDOG_ID);
                    }
                    if ((RTSPResponseMsg != iIncomingMsg.msgType) || (iRet != PVMFPending))
                    {//processIncomingMessage() returns pending if there are unacknowledged
                        //rtsp msgs
                        break;
                    }
                }

                iRealChallenge1 = "";
                const StrPtrLen *tmpRealChallenge = iIncomingMsg.queryField("RealChallenge1");
                if (tmpRealChallenge)
                {
                    iRealChallenge1 = OSCL_HeapString<OsclMemAllocator>(tmpRealChallenge->c_str());
                }

                //ChangeInternalState(PVRTSP_ENGINE_NODE_STATE_DESCRIBE_WAITING);
            }

            if (bNoSendPending)
            {
                // send describe
                RTSPOutgoingMessage *tmpOutgoingMsg =  OSCL_NEW(RTSPOutgoingMessage, ());
                if (tmpOutgoingMsg == NULL)
                {
                    iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorOutOfMemory;
                    iRet =  PVMFFailure;
                    break;
                }
                if (PVMFSuccess != composeDescribeRequest(*tmpOutgoingMsg))
                {
                    iCurrentErrorCode =
                        PVMFRTSPClientEngineNodeErrorRTSPComposeDescribeRequestError;
                    OSCL_DELETE(tmpOutgoingMsg);
                    iRet =  PVMFFailure;

                    //cancell the watchdog
                    iWatchdogTimer->Cancel(REQ_TIMER_WATCHDOG_ID);
                    break;
                }

                if (PVMFSuccess != sendSocketOutgoingMsg(iSendSocket, *tmpOutgoingMsg))
                {
                    iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorSocketSendError;
                    OSCL_DELETE(tmpOutgoingMsg);
                    iRet =  PVMFFailure;
                    //cancell the watchdog
                    iWatchdogTimer->Cancel(REQ_TIMER_WATCHDOG_ID);
                    break;
                }

                bNoSendPending = false;
                iOutgoingMsgQueue.push(tmpOutgoingMsg);

                ChangeInternalState(PVRTSP_ENGINE_NODE_STATE_OPTIONS_WAITING);
                //RunIfNotReady();
            }
            break;
        }
        case PVRTSP_ENGINE_NODE_STATE_OPTIONS_WAITING:
        {
            if (RTSPParser::REQUEST_IS_READY == iRTSPParserState)
            {
                iRet = processIncomingMessage(iIncomingMsg);
                if (iRet == PVMFSuccess)
                {
                    iRet = PVMFPending;
                }
                else
                {//either pending or error
                    if (iRet != PVMFPending)
                    {
                        //cancell the watchdog
                        iWatchdogTimer->Cancel(REQ_TIMER_WATCHDOG_ID);
                    }

                    if ((RTSPResponseMsg != iIncomingMsg.msgType) || (iRet != PVMFPending))
                    {//processIncomingMessage() returns pending if there are unacknowledged
                        //rtsp msgs
                        break;
                    }
                }

                iRealChallenge1 = "";
                const StrPtrLen *tmpRealChallenge = iIncomingMsg.queryField("RealChallenge1");
                if (tmpRealChallenge)
                {
                    iRealChallenge1 = OSCL_HeapString<OsclMemAllocator>(tmpRealChallenge->c_str());
                }

                ChangeInternalState(PVRTSP_ENGINE_NODE_STATE_DESCRIBE_WAITING);
            }
            else if (!clearEventQueue())
            {//sth failed, could be Send, Recv, or server closes
                iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorSocketError;
                iRet =  PVMFFailure;
                //cancell the watchdog
                iWatchdogTimer->Cancel(REQ_TIMER_WATCHDOG_ID);
            }
            break;
        }
        case PVRTSP_ENGINE_NODE_STATE_DESCRIBE_WAITING:
        {
            if (RTSPParser::REQUEST_IS_READY == iRTSPParserState)
            {
                iRet = processIncomingMessage(iIncomingMsg);
                if (iRet == PVMFSuccess)
                {//Init is not done until we get SDP
                    iRet = PVMFPending;
                }
                else
                {//either pending or error
                    if (iRet != PVMFPending)
                    {
                        //cancell the watchdog
                        iWatchdogTimer->Cancel(REQ_TIMER_WATCHDOG_ID);
                    }
                    break;
                }
            }
            else if (RTSPParser::ENTITY_BODY_IS_READY == iRTSPParserState)
            {//got sdp
                {
                    OsclRefCounter* my_refcnt = new OsclRefCounterSA< RTSPNodeMemDestructDealloc >(iEntityMemFrag.ptr);
                    if (my_refcnt == NULL)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::SendRtspDescribe() Unable to Allocate Memory"));
                        return PVMFErrNoMemory;
                    }

                    iSessionInfo.pSDPBuf = OsclRefCounterMemFrag(iEntityMemFrag, my_refcnt, iEntityMemFrag.len);
                    {//done with the entity body, change the ownership of the mem
                        iEntityMemFrag.len = 0;
                        iEntityMemFrag.ptr = NULL;
                    }
                }

                ChangeInternalState(PVRTSP_ENGINE_NODE_STATE_DESCRIBE_DONE);

                //cancell the watchdog
                iWatchdogTimer->Cancel(REQ_TIMER_WATCHDOG_ID);
                //iWatchdogTimer->Clear();

                iRet =  PVMFSuccess;
            }
            else if (!clearEventQueue())
            {
                //cancell the watchdog
                iWatchdogTimer->Cancel(REQ_TIMER_WATCHDOG_ID);

                iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorSocketError;
                iRet =  PVMFFailure;
                break;
            }
            break;
        }
        default:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVRTSPEngineNode::SendRtspDescribe() In"));
            iRet = PVMFErrInvalidState;
            break;
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::SendRtspDescribe() Out"));
    return iRet;
}

PVMFStatus PVRTSPEngineNode::DoPrepareNode(PVRTSPEngineCommand &aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::DoPrepareNode() In"));
    OSCL_UNUSED_ARG(aCmd);

    if (iInterfaceState != EPVMFNodeInitialized)
    {
        return PVMFErrInvalidState;
    }

    return SendRtspSetup(aCmd);
}

PVMFStatus PVRTSPEngineNode::SendRtspSetup(PVRTSPEngineCommand &aCmd)
{
    OSCL_UNUSED_ARG(aCmd);

    PVMFStatus iRet = PVMFPending;
    switch (iState)
    {
        case PVRTSP_ENGINE_NODE_STATE_DESCRIBE_DONE:
        case PVRTSP_ENGINE_NODE_STATE_PROCESS_REST_SETUP:
        {
            /*
            before this, the track selection is done.
            Also, the Bind for RTP and RTCP for each channel are done

              compose one RTSPOutgoingMessage for each SETUP
              push each RTSPOutgoingMessage in the iOutgoingMsgQueue
              lump all the SETUPs in one Send
            	*/

            if (RTSPParser::REQUEST_IS_READY == iRTSPParserState)
            {
                iRet = processIncomingMessage(iIncomingMsg);
                if (iRet == PVMFSuccess)
                {//The Prepare() not done until all the SETUPs are sent
                    iRet = PVMFPending;
                }
                else
                {//either pending or error
                    if (iRet != PVMFPending)
                    {
                        //cancell the watchdog
                        iWatchdogTimer->Cancel(REQ_TIMER_WATCHDOG_ID);
                    }
                    break;
                }

                if (iState == PVRTSP_ENGINE_NODE_STATE_DESCRIBE_DONE)
                {//ok, got the resp of 1st SETUP, send the reset SETUPs
                    bNoSendPending = true;
                    ChangeInternalState(PVRTSP_ENGINE_NODE_STATE_PROCESS_REST_SETUP);
                }

                //idx = iSessionInfo.trackSelectionList->getNumTracks();
                //if(all SETUPs resp are back)
                if ((uint32)setupTrackIndex  == iSessionInfo.iSelectedStream.size())
                {
                    if (!iOutgoingMsgQueue.empty())
                    {
                        RTSPOutgoingMessage* tmpOutgoingMsg = iOutgoingMsgQueue.top();
                        if (tmpOutgoingMsg->method == METHOD_SETUP)
                        {//still got some SETUPs of which server has not responded
                            break;
                        }
                    }

                    //cancell the watchdog
                    iWatchdogTimer->Cancel(REQ_TIMER_WATCHDOG_ID);
                    //iWatchdogTimer->Clear();

                    if (ibIsRealRDT)
                    {
                        // create frag group allocator
                        ipFragGroupMemPool = OSCL_NEW(OsclMemPoolFixedChunkAllocator, (DEFAULT_NUM_MEDIA_MSGS_IN_JITTER_BUFFER));
                        if (ipFragGroupMemPool == NULL)
                        {
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVRTSPEngineNode::DoInitNode() Error - Unable to allocate mempool"));
                            return PVMFErrNoMemory;
                        }
                        ipFragGroupAllocator = OSCL_NEW(PVMFMediaFragGroupCombinedAlloc<OsclMemAllocator>, (
                                                            DEFAULT_NUM_MEDIA_MSGS_IN_JITTER_BUFFER,
                                                            1, ipFragGroupMemPool));
                        if (ipFragGroupAllocator == NULL)
                        {
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVRTSPEngineNode::DoInitNode() Error - Unable to create frag group allocator"));
                            return PVMFErrNoMemory;
                        }
                        ipFragGroupAllocator->create();
                    }

                    ChangeInternalState(PVRTSP_ENGINE_NODE_STATE_SETUP_DONE);
                    iRet =  PVMFSuccess;
                    break;
                    //break;	//send PLAY back-to-back
                }
            }
            else if (RTSPParser::ENTITY_BODY_IS_READY == iRTSPParserState)
            {//got still image
                ChangeInternalState(PVRTSP_ENGINE_NODE_STATE_SETUP_DONE);
                iRet =  PVMFSuccess;
                break;
            }
            else if (!iSocketEventQueue.empty())
            {//TBD			if(!clearEventQueue())
                SocketEvent tmpSockEvent(iSocketEventQueue.front());
                iSocketEventQueue.erase(&iSocketEventQueue.front());

                if (tmpSockEvent.iSockEvent != EPVSocketSuccess)
                {//sth failed, could be Send, Recv, or server closes
                    iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorSocketError;
                    iRet =  PVMFFailure;
                    break;
                }

                if ((iState == PVRTSP_ENGINE_NODE_STATE_DESCRIBE_DONE)
                        && (tmpSockEvent.iSockFxn == EPVSocketSend))
                {//pretend there is a send pending so it waits for the first
                    //SETUP resp to come back and then sends the rest SETUPs
                    bNoSendPending = false;
                    break;
                }
            }

            //The trackID is the index to the SDP media info array.
            //Get the first track's index
            //int trackID = iSessionInfo.trackSelectionList->getTrackIndex(setupIndex);

            //compose and send SETUP
            //if( (bNoSendPending) && (NOT all the SETUPs are sent out ) )
            if ((bNoSendPending) && ((uint32)setupTrackIndex  < iSessionInfo.iSelectedStream.size()))
            {
                RTSPOutgoingMessage *tmpOutgoingMsg =  OSCL_NEW(RTSPOutgoingMessage, ());
                if (tmpOutgoingMsg == NULL)
                {
                    iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorOutOfMemory;
                    return  PVMFFailure;
                }
                //optimize here: use copy/modify instead of build from scratch
                //idx = iSessionInfo.iSDPinfo.getNumMediaObjects();
                //idx = iSessionInfo.trackSelectionList->getNumTracks();
                //if( PVMFSuccess != composeSetupRequest(*tmpOutgoingMsg, idx))
                if (PVMFSuccess != composeSetupRequest(*tmpOutgoingMsg, iSessionInfo.iSelectedStream[setupTrackIndex ]))
                {
                    iCurrentErrorCode =
                        PVMFRTSPClientEngineNodeErrorRTSPComposeSetupRequestError;
                    OSCL_DELETE(tmpOutgoingMsg);
                    return  PVMFFailure;
                }
                setupTrackIndex ++;

                if (PVMFSuccess != sendSocketOutgoingMsg(iSendSocket, *tmpOutgoingMsg))
                {
                    /* need to pop the msg based on cseq, NOT necessarily the early ones,
                    although YES in this case.
                    //for(int idx=0; idx < iSessionInfo.trackSelectionList->getNumTracks(); idx++ )
                    for(int idx=0; idx < iSessionInfo.iSDPinfo->getNumMediaObjects(); idx++ )
                    {
                    //iOutgoingMsgQueue.pop();
                    }
                    	*/
                    iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorSocketSendError;
                    OSCL_DELETE(tmpOutgoingMsg);
                    iRet =  PVMFFailure;
                    break;
                }

                bNoSendPending = false;
                iOutgoingMsgQueue.push(tmpOutgoingMsg);
                //ChangeInternalState(PVRTSP_ENGINE_NODE_STATE_WAIT_FIRST_SETUP);

                //setup the watch dog for server response
                if (setupTrackIndex == 1)
                {//only setup watchdog for the first SETUP, but it monitors all
                    iWatchdogTimer->Request(REQ_TIMER_WATCHDOG_ID, 0, TIMEOUT_WATCHDOG);
                }
            }
            break;
            //RunIfNotReady();
        }

        default:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVRTSPEngineNode::SendRtspSetup() iState=%d Line %d", iState, __LINE__));
            iRet = PVMFErrInvalidState;
            break;
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::SendRtspSetup() Out"));
    return iRet;
}


bool PVRTSPEngineNode::parseURL(const OSCL_wString& aURL)
//(wchar_t *url)
{
    if (0 == oscl_UnicodeToUTF8(aURL.get_cstr(), aURL.get_size(), ((mbchar*)iRTSPEngTmpBuf.ptr), iRTSPEngTmpBuf.len))
    {
        return false;
    }

    return parseURL((mbchar*)iRTSPEngTmpBuf.ptr);
}

bool PVRTSPEngineNode::parseURL(const char *aUrl)
{
    /* Input: absolute URI
     * Output: iSessionInfo.iSessionURL, iSessionInfo.iServerName, and iSessionInfo.iSrvAdd.port
     * Connection end point is always iSrvAdd
     * if no proxy is used, iSrvAdd.ipAddr is ip of iServerName and iSrvAdd.port is the server port.
     * Both derived from an absolute url.
     * if proxy is used, iSrvAdd is iProxyName:iProxyPort.
     */
    if (aUrl == NULL)
    {
        return false;
    }

    uint32 aURLMaxOutLength;
    PVStringUri::PersentageToEscapedEncoding((mbchar*) aUrl, aURLMaxOutLength);
    PVStringUri::IllegalCharactersToEscapedEncoding((mbchar*) aUrl, aURLMaxOutLength);

    iSessionInfo.iSessionURL = ((mbchar*)aUrl);
    OSCL_HeapString<PVRTSPEngineNodeAllocator> tmpURL = ((mbchar*)aUrl);

    mbchar *server_ip_ptr = OSCL_CONST_CAST(mbchar*, oscl_strstr(((mbchar*)tmpURL.get_cstr()), "//"));
    if (server_ip_ptr == NULL)
    {
        return false;
    }

    server_ip_ptr += 2;

    /* Locate the server name. */
    mbchar *server_port_ptr = OSCL_CONST_CAST(mbchar*, oscl_strstr(server_ip_ptr, ":"));
    mbchar *clip_name = OSCL_CONST_CAST(mbchar*, oscl_strstr(server_ip_ptr, "/"));
    if (clip_name != NULL)
    {
        *clip_name++ = '\0';
    }

    /* Locate the port number if provided. */
    iSessionInfo.iSrvAdd.port = (iSessionInfo.iStreamingType == PVRTSP_RM_HTTP) ? DEFAULT_HTTP_PORT : DEFAULT_RTSP_PORT;
    if ((server_port_ptr != NULL)  && (*(server_port_ptr + 1) != '/'))
    {
        *(server_port_ptr++) = '\0';
        uint32 atoi_tmp;
        if (PV_atoi(server_port_ptr, 'd', atoi_tmp))
        {
            iSessionInfo.iSrvAdd.port = atoi_tmp;
        }
    }

    OSCL_HeapString<PVRTSPEngineNodeAllocator> tmpServerName(server_ip_ptr, oscl_strlen(server_ip_ptr));
    iSessionInfo.iServerName = tmpServerName;

//iSessionInfo.iSrvAdd.port = 20080;
//iSessionInfo.iServerName = "172.16.2.42";

    return true;
}

PVMFStatus
PVRTSPEngineNode::composeOptionsRequest(RTSPOutgoingMessage &iMsg)
{
    iMsg.reset();

    iMsg.numOfTransportEntries = 0;
    iMsg.msgType = RTSPRequestMsg;
    iMsg.method = METHOD_OPTIONS;
    iMsg.originalURI.setPtrLen(iSessionInfo.iSessionURL.get_cstr(), iSessionInfo.iSessionURL.get_size());
    iMsg.cseq = iOutgoingSeq++;
    iMsg.cseqIsSet = true;
    iMsg.acceptIsSet = false;
    iMsg.userAgent = iSessionInfo.iUserAgent.get_cstr();
    iMsg.userAgentIsSet = true;

    {
        // setup parameters for the options command. this is necessary
        // for real rdt support.
        StrCSumPtrLen ClientChallenge = _STRLIT_CHAR("ClientChallenge");
        OSCL_HeapString<PVRTSPEngineNodeAllocator> ClientChallenge_Val("9e26d33f2984236010ef6253fb1887f7");
        iMsg.addField(&ClientChallenge, ClientChallenge_Val.get_cstr());

        StrCSumPtrLen PlayerStarttime = _STRLIT_CHAR("PlayerStarttime");
        OSCL_HeapString<PVRTSPEngineNodeAllocator> PlayerStarttime_Val("[28/03/2003:22:50:23 00:00]");
        iMsg.addField(&PlayerStarttime, PlayerStarttime_Val.get_cstr());

        StrCSumPtrLen CompanyID = _STRLIT_CHAR("CompanyID");
        OSCL_HeapString<PVRTSPEngineNodeAllocator> CompanyID_Val("KnKV4M4I/B2FjJ1TToLycw==");
        iMsg.addField(&CompanyID, CompanyID_Val.get_cstr());

        StrCSumPtrLen playerGuid = _STRLIT_CHAR("GUID");
        OSCL_StackString<64> playerGuidVal = _STRLIT_CHAR("00000000-0000-0000-0000-000000000000");
        iMsg.addField(&playerGuid, playerGuidVal.get_cstr());
    }

    if (iMsg.compose() == false)
    {
        return PVMFFailure;
    }
    else
    {
        return PVMFSuccess;
    }
}

/*
* Function : int composeDescribeRequest()
* Date	    : 09/13/2002
* Purpose  : Composes RTSP DESCRIBE request
* In/out   :
* Return   :
* Modified :
*/
PVMFStatus
PVRTSPEngineNode::composeDescribeRequest(RTSPOutgoingMessage &iMsg)
{
    iMsg.reset();
    iMsg.numOfTransportEntries = 0;
    iMsg.msgType = RTSPRequestMsg;
    iMsg.method = METHOD_DESCRIBE;
    iMsg.originalURI.setPtrLen(iSessionInfo.iSessionURL.get_cstr(), iSessionInfo.iSessionURL.get_size());
    iMsg.cseq = iOutgoingSeq++;
    iMsg.cseqIsSet = true;
    iMsg.accept = "application/sdp";
    iMsg.acceptIsSet = true;
    iMsg.userAgent = iSessionInfo.iUserAgent.get_cstr();
    iMsg.userAgentIsSet = true;

    if (oscl_strlen(iSessionInfo.iUserNetwork.get_cstr()))
    {
        StrCSumPtrLen UserNetwork = _STRLIT_CHAR("User-Network");
        iMsg.addField(&UserNetwork, iSessionInfo.iUserNetwork.get_cstr());
    }
    if (oscl_strlen(iSessionInfo.iDeviceInfo.get_cstr()))
    {
        StrCSumPtrLen DeviceInfo = _STRLIT_CHAR("DeviceInfo");
        iMsg.addField(&DeviceInfo, iSessionInfo.iDeviceInfo.get_cstr());
    }
    if (oscl_strlen(iSessionInfo.iUserID.get_cstr()) && oscl_strlen(iSessionInfo.iAuthentication.get_cstr()))
    {
        OSCL_HeapString<PVRTSPEngineNodeAllocator> myBuf("user=");
        myBuf += iSessionInfo.iUserID.get_cstr();
        myBuf += ";authentication=";
        myBuf += iSessionInfo.iAuthentication.get_cstr();

        StrCSumPtrLen User_id = _STRLIT_CHAR("ID");
        iMsg.addField(&User_id, myBuf.get_cstr());
    }
    if (oscl_strlen(iSessionInfo.iExpiration.get_cstr()))
    {
        StrCSumPtrLen Expiration = _STRLIT_CHAR("Expiration");
        iMsg.addField(&Expiration, iSessionInfo.iExpiration.get_cstr());
    }
    if (oscl_strlen(iSessionInfo.iApplicationSpecificString.get_cstr()))
    {
        StrCSumPtrLen Application_specific_string = _STRLIT_CHAR("Application-Specific-String");
        iMsg.addField(&Application_specific_string, iSessionInfo.iApplicationSpecificString.get_cstr());
    }

    if (iSessionInfo.iVerification.get_size() && iSessionInfo.iSignature.get_size())
    {
        OSCL_HeapString<PVRTSPEngineNodeAllocator> myBuf("filler=");
        myBuf += iSessionInfo.iVerification.get_cstr();
        myBuf += ";signature=";
        myBuf += iSessionInfo.iSignature.get_cstr();

        StrCSumPtrLen Verification = _STRLIT_CHAR("Verification");
        iMsg.addField(&Verification, myBuf.get_cstr());
    }

    {//If the Accept-Encoding field-value is empty, then only the "identity"
        // encoding is acceptable.
        StrCSumPtrLen AcceptEncoding = _STRLIT_CHAR("Accept-Encoding");
        iMsg.addField(&AcceptEncoding, "");
    }

    if (iMsg.compose() == false)
    {
        return PVMFFailure;
    }

    iSessionInfo.clientServerDelay = 0;
    uint32 clock = 0;
    bool overflowFlag = false;
    iRoundTripClockTimeBase.GetCurrentTime32(clock, overflowFlag, PVMF_MEDIA_CLOCK_MSEC);
    iSessionInfo.clientServerDelay = clock;

    //iSessionInfo.composedMessage = iMsg.retrieveComposedBuffer();
    return PVMFSuccess;
}

PVMFStatus PVRTSPEngineNode::processServerRequest(RTSPIncomingMessage &aMsg)
{//input: server request in aMsg;
    //just send the response. bingo.
    //all S->C are optional, including ANNOUNCE, GET_PARAMETER, SET_PARAMETER, OPTIONS
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::processServerRequest() In"));

    if (iSrvResponse == NULL)
    {
        iSrvResponse =  OSCL_NEW(RTSPOutgoingMessage, ());
        if (iSrvResponse == NULL)
        {
            iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorOutOfMemory;
            return  PVMFFailure;
        }
    }

    iSrvResponse->reset();
    iSrvResponse->msgType = RTSPResponseMsg;
    iSrvResponse->numOfTransportEntries = 0;

    if (aMsg.method == METHOD_END_OF_STREAM)
    {//
        iSrvResponse->statusCode =	CodeOK;
        iSrvResponse->reasonString = "OK";
        ReportInfoEvent(PVMFInfoEndOfData);
    }
    else if (aMsg.method == METHOD_SET_PARAMETER)
    {//
        iSrvResponse->statusCode =	CodeOK;
        iSrvResponse->reasonString = "OK";
    }
    else
    {
        iSrvResponse->statusCode =	CodeNotImplemented;
        iSrvResponse->reasonString = "Not Implemented";
    }
    //iSrvResponse->statusCode = CodeParameterNotUnderstood;
    //iSrvResponse->reasonString = "Parameter Not Understood";
    iSrvResponse->cseq = aMsg.cseq;
    iSrvResponse->cseqIsSet = true;

    if (iSessionInfo.iSID.get_size())
    {
        iSrvResponse->sessionId.setPtrLen(iSessionInfo.iSID.get_cstr(), iSessionInfo.iSID.get_size());
        iSrvResponse->sessionIdIsSet = true;
    }
    if (iSrvResponse->compose() == false)
    {
        iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorRTSPCompose501ResponseError;
        OSCL_DELETE(iSrvResponse);
        iSrvResponse = NULL;
        return PVMFFailure;
    }

    if (bNoSendPending)// bSrvRespPending
    {
        if (PVMFSuccess != sendSocketOutgoingMsg(iSendSocket, *iSrvResponse))
        {
            /* need to pop the msg based on cseq, NOT necessarily the early ones,
            although YES in this case.
            //for(int idx=0; idx < iSessionInfo.trackSelectionList->getNumTracks(); idx++ )
            for(int idx=0; idx < iSessionInfo.iSDPinfo->getNumMediaObjects(); idx++ )
            {
            //iOutgoingMsgQueue.pop();
            }
            	*/
            iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorSocketSendError;
            OSCL_DELETE(iSrvResponse);
            iSrvResponse = NULL;
            return  PVMFFailure;
        }

        bNoSendPending = false;
    }
    else
    {
        bSrvRespPending = true;
    }

    return PVMFSuccess;
}

/**
* This API processes server requests with entity bodies.
*
* @param aMsg The server request.
* @param aEntityMemFrag The oscl memory fragment which holds the entity body.
* @returns PVMF status.
*/
PVMFStatus PVRTSPEngineNode::processEntityBody(RTSPIncomingMessage &aMsg, OsclMemoryFragment &aEntityMemFrag)
{	//all S->C are optional, including ANNOUNCE, GET_PARAMETER, SET_PARAMETER, OPTIONS
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::processEntityBody() In"));

    if (iEntityMemFrag.ptr == NULL)
    {//the entity body hasn't come yet.
        //return PVMFFailure;
        return PVMFPending;
    }

    if (iSrvResponse == NULL)
    {
        iSrvResponse =  OSCL_NEW(RTSPOutgoingMessage, ());
        if (iSrvResponse == NULL)
        {
            iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorOutOfMemory;
            return  PVMFFailure;
        }
    }

    iSrvResponse->reset();
    iSrvResponse->msgType = RTSPResponseMsg;
    iSrvResponse->numOfTransportEntries = 0;

    if (aMsg.method == METHOD_SET_PARAMETER)
    {//
        iSrvResponse->statusCode =	CodeOK;
        iSrvResponse->reasonString = "OK";
    }
    else
    {
        iSrvResponse->statusCode =	CodeNotImplemented;
        iSrvResponse->reasonString = "Not Implemented";
    }
    //iSrvResponse->statusCode = CodeParameterNotUnderstood;
    //iSrvResponse->reasonString = "Parameter Not Understood";
    iSrvResponse->cseq = aMsg.cseq;
    iSrvResponse->cseqIsSet = true;

    if (iSessionInfo.iSID.get_size())
    {
        iSrvResponse->sessionId.setPtrLen(iSessionInfo.iSID.get_cstr(), iSessionInfo.iSID.get_size());
        iSrvResponse->sessionIdIsSet = true;
    }
    if (iSrvResponse->compose() == false)
    {
        iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorRTSPCompose501ResponseError;
        OSCL_DELETE(iSrvResponse);
        iSrvResponse = NULL;
        return PVMFFailure;
    }

    if (bNoSendPending)// bSrvRespPending
    {
        if (PVMFSuccess != sendSocketOutgoingMsg(iSendSocket, *iSrvResponse))
        {
            /* need to pop the msg based on cseq, NOT necessarily the early ones,
            although YES in this case.
            //for(int idx=0; idx < iSessionInfo.trackSelectionList->getNumTracks(); idx++ )
            for(int idx=0; idx < iSessionInfo.iSDPinfo->getNumMediaObjects(); idx++ )
            {
            //iOutgoingMsgQueue.pop();
            }
            	*/
            iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorSocketSendError;
            OSCL_DELETE(iSrvResponse);
            iSrvResponse = NULL;
            return  PVMFFailure;
        }

        bNoSendPending = false;
    }
    else
    {
        bSrvRespPending = true;
    }

    PVMFStatus tmpRet = PVMFSuccess;
    OSCL_UNUSED_ARG(aEntityMemFrag);
    return tmpRet;
}


PVMFStatus PVRTSPEngineNode::DoStartNode(PVRTSPEngineCommand &aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::DoStartNode() In"));
    OSCL_UNUSED_ARG(aCmd);

    //If session is completed, then do not send the play command to the server..
    if (IsSessionCompleted() && !bRepositioning)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::DoStartNode() Skipping sending play 'cos of session expiry"));
        ChangeInternalState(PVRTSP_ENGINE_NODE_STATE_PLAY_DONE);
        return PVMFSuccess;
    }

    if (iInterfaceState != EPVMFNodePrepared &&
            iInterfaceState != EPVMFNodePaused)
    {
        return PVMFErrInvalidState;
    }

    return 	SendRtspPlay(aCmd);
}

PVMFStatus PVRTSPEngineNode::SendRtspPlay(PVRTSPEngineCommand &aCmd)
{
    OSCL_UNUSED_ARG(aCmd);

    PVMFStatus iRet = PVMFPending;
    switch (iState)
    {
        case PVRTSP_ENGINE_NODE_STATE_SETUP_DONE:
        case PVRTSP_ENGINE_NODE_STATE_PAUSE_DONE:
        {
            if (!bNoSendPending)
            {
                break;
            }
            //compose and send PLAY
            //if ASF streaming, the SET_PARAMETER should be pipelined as well
            RTSPOutgoingMessage *tmpOutgoingMsg =  OSCL_NEW(RTSPOutgoingMessage, ());
            if (tmpOutgoingMsg == NULL)
            {
                iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorOutOfMemory;
                iRet =  PVMFFailure;
                break;
            }
            if (PVMFSuccess != composePlayRequest(*tmpOutgoingMsg))
            {
                iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorRTSPComposePlayRequestError;
                OSCL_DELETE(tmpOutgoingMsg);
                iRet =  PVMFFailure;
                break;
            }

            if (PVMFSuccess != sendSocketOutgoingMsg(iSendSocket, *tmpOutgoingMsg))
            {
                iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorSocketSendError;
                OSCL_DELETE(tmpOutgoingMsg);
                iRet =  PVMFFailure;
                break;
            }

            bNoSendPending = false;
            iOutgoingMsgQueue.push(tmpOutgoingMsg);

            ChangeInternalState(PVRTSP_ENGINE_NODE_STATE_WAIT_PLAY);
            //setup the watch dog for server response
            iWatchdogTimer->Request(REQ_TIMER_WATCHDOG_ID, 0, TIMEOUT_WATCHDOG);
            RunIfNotReady();
            break;
        }
        case PVRTSP_ENGINE_NODE_STATE_WAIT_PLAY:
        {
            if (RTSPParser::REQUEST_IS_READY == iRTSPParserState)
            {
                iRet = processIncomingMessage(iIncomingMsg);
                if (iRet != PVMFPending)
                {
                    //cancell the watchdog
                    iWatchdogTimer->Cancel(REQ_TIMER_WATCHDOG_ID);
                    if (iRet == PVMFSuccess)
                    {
                        ChangeInternalState(PVRTSP_ENGINE_NODE_STATE_PLAY_DONE);
                    }
                }
            }
            else if (RTSPParser::ENTITY_BODY_IS_READY == iRTSPParserState)
            {//got MS TCP RTP packets
                //processSDP(REINTERPRET_CAST(mbchar*, pSDPBuf.ptr), pSDPBuf.len-1);
                //processSDP((mbchar*)pSDPBuf.ptr, pSDPBuf.len-1);
                //ChangeInternalState(PVRTSP_ENGINE_NODE_STATE_WAIT_DESCRIBE_DONE);
                iRet = processIncomingMessage(iIncomingMsg);
                if (iRet != PVMFPending)
                {
                    //cancell the watchdog
                    iWatchdogTimer->Cancel(REQ_TIMER_WATCHDOG_ID);
                    if (iRet == PVMFSuccess)
                    {
                        ChangeInternalState(PVRTSP_ENGINE_NODE_STATE_PLAY_DONE);
                    }
                }
            }
            else if (!clearEventQueue())
            {
                //cancell the watchdog
                iWatchdogTimer->Cancel(REQ_TIMER_WATCHDOG_ID);

                iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorSocketError;
                iRet =  PVMFFailure;
            }
            break;
        }

        default:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVRTSPEngineNode::SendRtspPlay() iState=%d Line %d", iState, __LINE__));
            iRet = PVMFErrInvalidState;
            break;
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::SendRtspPlay() Out"));
    return iRet;
}

PVMFStatus PVRTSPEngineNode::composeSetupRequest(RTSPOutgoingMessage &iMsg, StreamInfo &aSelected)
{
    //Reset the data structure
    iMsg.reset();

    if (iSessionInfo.iSDPinfo.GetRep() == NULL) return PVMFFailure;

    //Here we decide if the selected track is a still image or not
    mediaInfo *tmpMediaInfo = iSessionInfo.iSDPinfo->getMediaInfoBasedOnID(aSelected.iSDPStreamId);
    if (NULL == tmpMediaInfo)
    {
        return PVMFFailure;
    }
    StrCSumPtrLen still_image = "X-MP4V-IMAGE";
    if (!oscl_strncmp(tmpMediaInfo->getMIMEType(), still_image.c_str(), still_image.length()))
    {
        StrPtrLen contentType = "text/parameters";
        StrCSumPtrLen image = "Image\r\n";

        iMsg.contentType = contentType;
        iMsg.contentTypeIsSet = true;
        iMsg.contentLength = image.length();
        iMsg.contentLengthIsSet = true;
        iMsg.accept = "X-MP4V-IMAGE";
        iMsg.acceptIsSet = true;
        iMsg.method = METHOD_GET_PARAMETER;
        iMsg.numOfTransportEntries = 0;

        /*
        mbchar mediaURL[RTSP_MAX_FULL_REQUEST_SIZE];
        mediaURL[0] = '\0';

          oscl_strncpy( mediaURL,
          ( iSessionInfo.contentBaseFlag )? iSessionInfo.contentBaseURL:requestURL,
          oscl_strlen(( iSessionInfo.contentBaseFlag )? iSessionInfo.contentBaseURL:requestURL) );
          mediaURL[oscl_strlen(( iSessionInfo.contentBaseFlag )? iSessionInfo.contentBaseURL:requestURL)] = '\0';
          iMsg.originalURI = mediaURL;
        */
    }
    else
    {
        //Set standard fields
        iMsg.method = METHOD_SETUP;
        iMsg.userAgent = iSessionInfo.iUserAgent.get_cstr();
        iMsg.userAgentIsSet = true;

        {
            iMsg.numOfTransportEntries = 1;
            iMsg.transport[0].protocol = RtspTransport::RTP_PROTOCOL;
            iMsg.transport[0].protocolIsSet = true;
            iMsg.transport[0].profile = RtspTransport::AVP_PROFILE;
            iMsg.transport[0].profileIsSet = true;
            iMsg.transport[0].delivery = RtspTransport::UNICAST_DELIVERY;
            iMsg.transport[0].deliveryIsSet = true;
            if ((iSessionInfo.iStreamingType == PVRTSP_3GPP_UDP)
                    || (iSessionInfo.iStreamingType == PVRTSP_MS_UDP))
            {
                iMsg.transport[0].transportType = RtspTransport::UDP_TRANSPORT;
                iMsg.transport[0].transportTypeIsSet = true;

                iMsg.transport[0].channelIsSet = false;

                iMsg.transport[0].client_portIsSet = true;
                iMsg.transport[0].client_port1 = OSCL_STATIC_CAST(uint16, aSelected.iCliRTPPort);	//dataPort;
                iMsg.transport[0].client_port2 = OSCL_STATIC_CAST(uint16, aSelected.iCliRTCPPort);	//feedbackPort;

                if (iSessionInfo.iStreamingType == PVRTSP_MS_UDP)
                {//check here!
                    iMsg.transport[0].client_port2 = 0;
                }

                /* Send recommended block size of RTP packet to the server.  CJ 09/10/2001 */
                StrCSumPtrLen blockSize = "Blocksize";
                oscl_snprintf(((mbchar*)iRTSPEngTmpBuf.ptr), iRTSPEngTmpBuf.len, "%d", RECOMMENDED_RTP_BLOCK_SIZE);
                iMsg.addField(&blockSize, ((mbchar*)iRTSPEngTmpBuf.ptr));
            }
            else
            {
                iMsg.transport[0].transportType = RtspTransport::TCP_TRANSPORT;
                iMsg.transport[0].transportTypeIsSet = true;

                iMsg.transport[0].client_portIsSet = false;

                iMsg.transport[0].channelIsSet = false;
                //iMsg.transport[0].channel1 = 8888;//aSelected.iSerRTPPort;
                //iMsg.transport[0].channel2 = 9999;//aSelected.iSerRTCPPort;
            }

            iMsg.transport[0].appendIsSet = false;
            iMsg.transport[0].layersIsSet = false;
            iMsg.transport[0].modeIsSet = false;
            iMsg.transport[0].portIsSet = false;
            iMsg.transport[0].server_portIsSet = false;
            iMsg.transport[0].ttlIsSet = false;
            iMsg.transport[0].ssrcIsSet = false;
        }

        {/*
			StrCSumPtrLen mytransport = "Transport";
			iMsg.addField(&mytransport, "x-real-rdt/udp;client_port=6970");
			iMsg.numOfTransportEntries = 0;
			*/
        }
        //Compose etag
        if (oscl_strlen((iSessionInfo.iSDPinfo->getSessionInfo())->getETag()) != 0)
        {
            if (oscl_snprintf(((mbchar*)iRTSPEngTmpBuf.ptr), iRTSPEngTmpBuf.len, "\"%s\"", (iSessionInfo.iSDPinfo->getSessionInfo())->getETag()) != 1)
            {
                StrCSumPtrLen etag = "If-Match";
                iMsg.addField(&etag, ((mbchar*)iRTSPEngTmpBuf.ptr));
            }
        }

        if (oscl_strlen(iSessionInfo.iUserNetwork.get_cstr()))
        {
            StrCSumPtrLen UserNetwork = _STRLIT_CHAR("User-Network");
            iMsg.addField(&UserNetwork, iSessionInfo.iUserNetwork.get_cstr());
        }
        if (oscl_strlen(iSessionInfo.iDeviceInfo.get_cstr()))
        {
            StrCSumPtrLen DeviceInfo = _STRLIT_CHAR("DeviceInfo");
            iMsg.addField(&DeviceInfo, iSessionInfo.iDeviceInfo.get_cstr());
        }
        if (oscl_strlen(iSessionInfo.iUserID.get_cstr()) && oscl_strlen(iSessionInfo.iAuthentication.get_cstr()))
        {
            OSCL_HeapString<PVRTSPEngineNodeAllocator> myBuf("user=");
            myBuf += iSessionInfo.iUserID.get_cstr();
            myBuf += ";authentication=";
            myBuf += iSessionInfo.iAuthentication.get_cstr();

            StrCSumPtrLen User_id = _STRLIT_CHAR("ID");
            iMsg.addField(&User_id, myBuf.get_cstr());
        }
        if (oscl_strlen(iSessionInfo.iExpiration.get_cstr()))
        {
            StrCSumPtrLen Expiration = _STRLIT_CHAR("Expiration");
            iMsg.addField(&Expiration, iSessionInfo.iExpiration.get_cstr());
        }
        if (oscl_strlen(iSessionInfo.iApplicationSpecificString.get_cstr()))
        {
            StrCSumPtrLen Application_specific_string = _STRLIT_CHAR("Application-Specific-String");
            iMsg.addField(&Application_specific_string, iSessionInfo.iApplicationSpecificString.get_cstr());
        }
        if (iSessionInfo.iVerification.get_size() && iSessionInfo.iSignature.get_size())
        {
            OSCL_HeapString<PVRTSPEngineNodeAllocator> myBuf("filler=");
            myBuf += iSessionInfo.iVerification.get_cstr();
            myBuf += ";signature=";
            myBuf += iSessionInfo.iSignature.get_cstr();

            StrCSumPtrLen Verification = _STRLIT_CHAR("Verification");
            iMsg.addField(&Verification, myBuf.get_cstr());
        }

        //Compose the media level URL for use in a RTSP request message.
        if (composeMediaURL(aSelected.iSDPStreamId, iMsg.originalURI) != PVMFSuccess)
        {
            return PVMFFailure;
        }
        aSelected.iMediaURI = iMsg.originalURI.c_str();

        if (aSelected.b3gppAdaptationIsSet)
        {// 3GPP release-6 rate adaptation
            mbchar buffer[256];

            OSCL_HeapString<PVRTSPEngineNodeAllocator> myBuf("url=\"");
            myBuf += iMsg.originalURI.c_str();
            myBuf += "\";size=";
            oscl_snprintf(buffer, 256, "%d", aSelected.iBufSize);
            myBuf += buffer;
            myBuf += ";target-time=";
            oscl_snprintf(buffer, 256, "%d", aSelected.iTargetTime);
            myBuf += buffer;

            StrCSumPtrLen _3GPPadaptation = "3GPP-Adaptation";
            iMsg.addField(&_3GPPadaptation, myBuf.get_str());
        }
    }

    iMsg.msgType = RTSPRequestMsg;
    iMsg.cseq = iOutgoingSeq++;
    iMsg.cseqIsSet = true;

    //OSCL_StackString<32> tmpSID = _STRLIT_CHAR("d5ac74e39ac8d75f");
    //iSessionInfo.iSID.set(tmpSID.get_cstr(), tmpSID.get_size());

    if (iSessionInfo.iSID.get_size())
    {
        iMsg.sessionId.setPtrLen(iSessionInfo.iSID.get_cstr(), iSessionInfo.iSID.get_size());
        iMsg.sessionIdIsSet = true;
    }

    if (iMsg.compose() == false)
    {
        return PVMFFailure;
    }

    iSessionInfo.clientServerDelay = 0;
    uint32 clock = 0;
    bool overflowFlag = false;
    iRoundTripClockTimeBase.GetCurrentTime32(clock, overflowFlag, PVMF_MEDIA_CLOCK_MSEC);
    iSessionInfo.clientServerDelay = clock;

    return PVMFSuccess;
}

PVMFStatus PVRTSPEngineNode::composePlayRequest(RTSPOutgoingMessage &aMsg)
{
    aMsg.reset();
    aMsg.numOfTransportEntries = 0;
    aMsg.msgType = RTSPRequestMsg;
    aMsg.method = METHOD_PLAY;
    aMsg.cseq = iOutgoingSeq++;
    aMsg.cseqIsSet = true;

    if (iSessionInfo.iSID.get_size())
    {
        aMsg.sessionId.setPtrLen(iSessionInfo.iSID.get_cstr(), iSessionInfo.iSID.get_size());
        aMsg.sessionIdIsSet = true;
    }

    //Add range field only if it is a PLAY request
    //if( (iState != EResumeSession) && (!upstreamClient) )
    if ((iState != PVRTSP_ENGINE_NODE_STATE_PAUSE_DONE)
            || (bRepositioning)
            || (iInterfaceState == EPVMFNodePrepared))
        //if( iState == PVRTSP_ENGINE_NODE_STATE_SETUP_DONE )
    {
        bRepositioning = false;
        OSCL_StackString<8> npt = _STRLIT_CHAR("npt=");
        oscl_strncpy(((mbchar*)iRTSPEngTmpBuf.ptr), npt.get_cstr(), npt.get_size());
        ((mbchar*)iRTSPEngTmpBuf.ptr)[npt.get_size()] = '\0';

        if (iSessionInfo.iReqPlayRange.format == RtspRangeType::NPT_RANGE)
        {
            if (iSessionInfo.iReqPlayRange.start_is_set == true)
            {
                if (iSessionInfo.iReqPlayRange.npt_start.npt_format == NptTimeFormat::NPT_SEC)
                {
                    oscl_snprintf(((mbchar*)iRTSPEngTmpBuf.ptr) + oscl_strlen(((mbchar*)iRTSPEngTmpBuf.ptr)), \
                                  (64 - oscl_strlen(((mbchar*)iRTSPEngTmpBuf.ptr))), "%d.%03d-", \
                                  iSessionInfo.iReqPlayRange.npt_start.npt_sec.sec, iSessionInfo.iReqPlayRange.npt_start.npt_sec.milli_sec);
                }
                else if (iSessionInfo.iReqPlayRange.npt_start.npt_format == NptTimeFormat::NOW)
                {
                    oscl_snprintf(((mbchar*)iRTSPEngTmpBuf.ptr) + oscl_strlen(((mbchar*)iRTSPEngTmpBuf.ptr)), \
                                  (64 - oscl_strlen(((mbchar*)iRTSPEngTmpBuf.ptr))), "now-");
                }
                else
                {
                    return PVMFFailure;
                }
            }

            if (iSessionInfo.iReqPlayRange.end_is_set == true)
            {
                if (iSessionInfo.iReqPlayRange.npt_end.npt_format == NptTimeFormat::NPT_SEC)
                {
                    if ((iSessionInfo.iReqPlayRange.npt_end.npt_sec.sec != 0)
                            || (iSessionInfo.iReqPlayRange.npt_end.npt_sec.milli_sec != 0))
                    {
                        oscl_snprintf(((mbchar*)iRTSPEngTmpBuf.ptr) + oscl_strlen(((mbchar*)iRTSPEngTmpBuf.ptr)), \
                                      (64 - oscl_strlen(((mbchar*)iRTSPEngTmpBuf.ptr))), "%d.%03d", \
                                      iSessionInfo.iReqPlayRange.npt_end.npt_sec.sec, iSessionInfo.iReqPlayRange.npt_end.npt_sec.milli_sec);
                    }
                }
                else
                {
                    return PVMFFailure;
                }
            }

            StrCSumPtrLen Range = _STRLIT_CHAR("Range");
            aMsg.addField(&Range, ((mbchar*)iRTSPEngTmpBuf.ptr));
        }
    }

    {//put user agent in SETUP an PLAY for testing for Real. IOT testing Jun 02, 05
        aMsg.userAgent = iSessionInfo.iUserAgent.get_cstr();
        aMsg.userAgentIsSet = true;
    }

    if (composeSessionURL(aMsg) != PVMFSuccess)
    {
        return PVMFFailure;
    }

    if (aMsg.compose() == false)
    {
        return PVMFFailure;
    }

    //iSessionInfo.composedMessage = aMsg.retrieveComposedBuffer();

    iSessionInfo.clientServerDelay = 0;
    uint32 clock = 0;
    bool overflowFlag = false;
    iRoundTripClockTimeBase.GetCurrentTime32(clock, overflowFlag, PVMF_MEDIA_CLOCK_MSEC);
    iSessionInfo.clientServerDelay = clock;

    return PVMFSuccess;
}

PVMFStatus PVRTSPEngineNode::composePauseRequest(RTSPOutgoingMessage &aMsg)
{
    aMsg.reset();
    aMsg.numOfTransportEntries = 0;
    aMsg.msgType = RTSPRequestMsg;
    aMsg.method = METHOD_PAUSE;
    aMsg.cseq = iOutgoingSeq++;
    aMsg.cseqIsSet = true;

    if (iSessionInfo.iSID.get_size())
    {
        aMsg.sessionId.setPtrLen(iSessionInfo.iSID.get_cstr(), iSessionInfo.iSID.get_size());
        aMsg.sessionIdIsSet = true;
    }

    {//put user agent in SETUP an PLAY for testing for Real. IOT testing Jun 02, 05
        aMsg.userAgent = iSessionInfo.iUserAgent.get_cstr();
        aMsg.userAgentIsSet = true;
    }

    if (composeSessionURL(aMsg) != PVMFSuccess)
    {
        return PVMFFailure;
    }

    if (aMsg.compose() == false)
    {
        return PVMFFailure;
    }

    iSessionInfo.clientServerDelay = 0;
    uint32 clock = 0;
    bool overflowFlag = false;
    iRoundTripClockTimeBase.GetCurrentTime32(clock, overflowFlag, PVMF_MEDIA_CLOCK_MSEC);
    iSessionInfo.clientServerDelay = clock;

    return PVMFSuccess;
}


PVMFStatus PVRTSPEngineNode::composeStopRequest(RTSPOutgoingMessage &aMsg)
{
    aMsg.reset();
    aMsg.numOfTransportEntries = 0;
    aMsg.msgType = RTSPRequestMsg;
    aMsg.method = METHOD_TEARDOWN;
    aMsg.cseq = iOutgoingSeq++;
    aMsg.cseqIsSet = true;

    aMsg.userAgent = iSessionInfo.iUserAgent.get_cstr();
    aMsg.userAgentIsSet = true;

    if (iSessionInfo.iSID.get_size())
    {
        aMsg.sessionId.setPtrLen(iSessionInfo.iSID.get_cstr(), iSessionInfo.iSID.get_size());
        aMsg.sessionIdIsSet = true;
    }

    if (composeSessionURL(aMsg) != PVMFSuccess)
    {
        return PVMFFailure;
    }

    StrCSumPtrLen connection = "Connection";
    aMsg.addField(&connection, "close");

    if (aMsg.compose() == false)
    {
        return PVMFFailure;
    }

    iSessionInfo.clientServerDelay = 0;
    uint32 clock = 0;
    bool overflowFlag = false;
    iRoundTripClockTimeBase.GetCurrentTime32(clock, overflowFlag, PVMF_MEDIA_CLOCK_MSEC);
    iSessionInfo.clientServerDelay = clock;

    return PVMFSuccess;
}

/*
* Function : PVMFStatus composeSessionURL()
* Date     : 10/30/2002
* Purpose  : Composing a session level URL for use in a RTSP request message.
* In/out   :
* Return   : PVMFSuccess upon succeessful composition. PVMFFailure otherwise.
* Modified :
*/
PVMFStatus PVRTSPEngineNode::composeSessionURL(RTSPOutgoingMessage &aMsg)
{
    OSCL_StackString<16> rtsp_str = _STRLIT_CHAR("rtsp");
    const char *sdpSessionURL = (iSessionInfo.iSDPinfo->getSessionInfo())->getControlURL();
    if (sdpSessionURL == NULL)
    {
        return PVMFFailure;
    }

    //1. SDP from DESCRIBE response
    if (iSessionInfo.bExternalSDP)
    {
        if (!oscl_strncmp(sdpSessionURL, rtsp_str.get_cstr(), rtsp_str.get_size()))
        {
            aMsg.originalURI = sdpSessionURL;
            return PVMFSuccess;
        }
        return PVMFFailure;
    }
    else
    {
        char *baseURL;
        if (iSessionInfo.iContentBaseURL.get_size())
        {
            baseURL = iSessionInfo.iContentBaseURL.get_str();
        }
        else
        {
            baseURL = iSessionInfo.iSessionURL.get_str();
        }

        //Case where control url is *
        ((mbchar*)iRTSPEngTmpBuf.ptr)[0] = '\0';
        uint tmpLen = iRTSPEngTmpBuf.len;
        mbchar asterisk[] = {"*"};
        if (!oscl_strncmp(sdpSessionURL, asterisk, oscl_strlen(asterisk)))
        {
            oscl_strncpy(((mbchar*)iRTSPEngTmpBuf.ptr), baseURL, oscl_strlen(baseURL));
            ((mbchar*)iRTSPEngTmpBuf.ptr)[oscl_strlen(baseURL)] = '\0';

            if (((mbchar*)iRTSPEngTmpBuf.ptr)[oscl_strlen(((mbchar*)iRTSPEngTmpBuf.ptr)) - 1] == '/')
            {
                ((mbchar*)iRTSPEngTmpBuf.ptr)[oscl_strlen(((mbchar*)iRTSPEngTmpBuf.ptr)) - 1] = '\0';
            }
            aMsg.originalURI = ((mbchar*)iRTSPEngTmpBuf.ptr);
        }
        //Case where control url is absolute
        else if (!oscl_strncmp(sdpSessionURL, rtsp_str.get_cstr(), rtsp_str.get_size()))
        {
            aMsg.originalURI = sdpSessionURL;
        }
        //other cases
        else if (composeURL((const char *)baseURL, sdpSessionURL, \
                            ((mbchar*)iRTSPEngTmpBuf.ptr), tmpLen) == true)
        {
            aMsg.originalURI = ((mbchar*)iRTSPEngTmpBuf.ptr);
        }
        else
        {
            return PVMFFailure;
        }
    }
    return PVMFSuccess;
}

PVMFStatus PVRTSPEngineNode::composeMediaURL(int aTrackID, StrPtrLen &aMediaURI)
{
    OSCL_StackString<16> rtsp_str = _STRLIT_CHAR("rtsp");

    const char *sdpMediaURL = (iSessionInfo.iSDPinfo->getMediaInfoBasedOnID(aTrackID))->getControlURL();
    if (sdpMediaURL == NULL)
    {
        return PVMFFailure;
    }

    if (!oscl_strncmp(sdpMediaURL, rtsp_str.get_cstr(), rtsp_str.get_size()))
    {
        {
            aMediaURI = sdpMediaURL;
        }

    }
    else
    {
        const char *sdpSessionURL = (iSessionInfo.iSDPinfo->getSessionInfo())->getControlURL();

        if (!oscl_strncmp(sdpSessionURL, rtsp_str.get_cstr(), rtsp_str.get_size()))
        {
            ((mbchar*)iRTSPEngTmpBuf.ptr)[0] = '\0';
            uint tmpLen = iRTSPEngTmpBuf.len;

            if (composeURL(sdpSessionURL, sdpMediaURL,
                           ((mbchar*)iRTSPEngTmpBuf.ptr), tmpLen) != true)
            {
                return PVMFFailure;
            }

            aMediaURI = ((mbchar*)iRTSPEngTmpBuf.ptr);
        }
        //Compose absolute URL
        else
        {
            char *baseURL;
            if (iSessionInfo.iContentBaseURL.get_size())
            {
                baseURL = iSessionInfo.iContentBaseURL.get_str();
            }
            else
            {
                baseURL = iSessionInfo.iSessionURL.get_str();
            }

            {
                uint tmpLen = iRTSPEngTmpBuf.len;
                if (composeURL((const char *)baseURL,
                               sdpMediaURL,
                               ((mbchar*)iRTSPEngTmpBuf.ptr), tmpLen) != true)
                {
                    return PVMFFailure;
                }
            }
            aMediaURI = ((mbchar*)iRTSPEngTmpBuf.ptr);
        }
    }
    return PVMFSuccess;
}

PVMFStatus PVRTSPEngineNode::processCommonResponse(RTSPIncomingMessage &aMsg)
{
    /*Parse content base - this is needed to compose URLs for future RTSP requests*/

    if (iSessionInfo.iContentBaseURL.get_size() == 0)
    {
        if (aMsg.contentBase.length())
        {
            OSCL_HeapString<PVRTSPEngineNodeAllocator> tmpBaseURL(aMsg.contentBase.c_str(), aMsg.contentBase.length());
            iSessionInfo.iContentBaseURL = tmpBaseURL;
        }
        else
        {
            const StrPtrLen *tmpBaseURLLoc = aMsg.queryField("Content-Location");
            if (tmpBaseURLLoc)
            {
                OSCL_HeapString<PVRTSPEngineNodeAllocator> tmpBaseURL(tmpBaseURLLoc->c_str(), tmpBaseURLLoc->length());
                iSessionInfo.iContentBaseURL = tmpBaseURL;
            }
        }
    }

    /*Extract session id from the server response. This is done only once per session and the following condition ensures it. */
    //might need to check the SID match
    if ((aMsg.sessionIdIsSet) && (iSessionInfo.iSID.get_size() == 0))
    {
        //because the RTSP parser just gives "d2ecb87b0816b4a;timeout=60"
        char *timeout_location = OSCL_CONST_CAST(char*, oscl_strstr(aMsg.sessionId.c_str(), ";timeout"));
        if (NULL != timeout_location)
        {
            OSCL_HeapString<PVRTSPEngineNodeAllocator> tmpSID(aMsg.sessionId.c_str(), timeout_location - aMsg.sessionId.c_str());
            iSessionInfo.iSID = tmpSID;

            //should be timeout-RTT.
            int32 tmpTimeout = (aMsg.timeout - 5);
            if ((TIMEOUT_KEEPALIVE > tmpTimeout) && (tmpTimeout > 0))
            {
                TIMEOUT_KEEPALIVE = tmpTimeout;
            }
        }
        else
        {
            OSCL_HeapString<PVRTSPEngineNodeAllocator> tmpSID(aMsg.sessionId.c_str(), aMsg.sessionId.length());
            iSessionInfo.iSID = tmpSID;
        }
        iSessionInfo.tSIDIsSetFlag = true;
    }

    /*Parse preroll duration from server response (if available)*/
    iSessionInfo.prerollDuration = 0;

    const StrPtrLen *tmpBufSize = aMsg.queryField("Buffersize");

    if (tmpBufSize != NULL)
    {
        //        pSessionInfo->prerollDuration = atoi( aMsg.queryField(buffer)->getPtr() );
        uint32 atoi_tmp;
        PV_atoi(tmpBufSize->c_str(), 'd', atoi_tmp);
        iSessionInfo.prerollDuration = atoi_tmp;
    }

    /*
     * Check for PVServer - needed to perform firewall port remapping
     */
    const StrPtrLen *serverTag = aMsg.queryField("Server");
    OSCL_StackString<8> pvServerTag(_STRLIT_CHAR("PVSS"));
    if (serverTag != NULL)
    {
        uint32 minLen = OSCL_MIN((pvServerTag.get_size()), ((uint32)(serverTag->size())));
        if (!oscl_strncmp(serverTag->c_str(), pvServerTag.get_cstr(), minLen))
        {
            iSessionInfo.pvServerIsSetFlag = true;
        }
        else
        {
            iSessionInfo.pvServerIsSetFlag = false;
        }
    }
    /*
     * Check for PVServer version number
     */
    if (NULL != serverTag)
    {
        if (iSessionInfo.pvServerIsSetFlag)
        {
            OSCL_StackString<8> pvServerVersionNumberLocator(_STRLIT_CHAR("/"));
            const char *versionNumberLocation = oscl_strstr(serverTag->c_str(), pvServerVersionNumberLocator.get_cstr());
            if (NULL != versionNumberLocation)
            {
                uint32 versionNumber = 0;
                if (PV_atoi(versionNumberLocation + 1, 'd', 1, versionNumber))
                {
                    iSessionInfo.iServerVersionNumber = versionNumber;
                }
            }
        }
    }

    return PVMFSuccess;
}

PVMFStatus PVRTSPEngineNode::processIncomingMessage(RTSPIncomingMessage &iIncomingMsg)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::processIncomingMessage() in"));

    if (RTSPOk != iIncomingMsg.isMalformed())
    {
        iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorMalformedRTSPMessage;
        return PVMFFailure;
    }
    if (RTSPRequestMsg == iIncomingMsg.msgType)
    {
        if (METHOD_BINARY_DATA == iIncomingMsg.method)
        {//need to check the rtsp parcom
            /*
            iIncomingMsg.bufferSize;
            iIncomingMsg.contentLength;
            iIncomingMsg.fullRequestBuffer;
            iIncomingMsg.fullRequestBufferSizeUsed;
            */
            //iIncomingMsg.reset();
            return PVMFPending;
        }
        //PVMFStatus tmpRet = ( (iIncomingMsg.contentLengthIsSet) && (iIncomingMsg.contentLength > 0) )
        PVMFStatus tmpRet = ((iIncomingMsg.contentLengthIsSet))
                            ? processEntityBody(iIncomingMsg, iEntityMemFrag)
                            : processServerRequest(iIncomingMsg);
        return (tmpRet == PVMFSuccess) ? PVMFPending : tmpRet;
    }

    if (RTSPResponseMsg != iIncomingMsg.msgType)
    {
        iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorIncorrectRTSPMessageType;
        return PVMFFailure;
    }

    if (iOutgoingMsgQueue.empty())
    {
        //we don't expect a response at this moment.
        iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorUnknownRTSPMessage;
        return PVMFFailure;
    }

    //pop the outgoing msg queue and see the match
    RTSPOutgoingMessage* tmpOutgoingMsg = iOutgoingMsgQueue.top();
    if (tmpOutgoingMsg->cseqIsSet)
    {
        //we always send seq. But just to be sure
        if (!iIncomingMsg.cseqIsSet)
        {
            //server should reply with seq
            iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorMissingSeqNumInServerResponse;
            return PVMFFailure;
        }
        if (tmpOutgoingMsg->cseq != iIncomingMsg.cseq)
        {
            //FIFO server messed up
            iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorRTSPRequestResponseMismatch;
            return PVMFFailure;
        }
        iOutgoingMsgQueue.pop();
    }
    else if (tmpOutgoingMsg->method == METHOD_GET)
    {//TBD varify this is the http GET response
        iOutgoingMsgQueue.pop();
    }

    //check session ID as well
    if (200 == iIncomingMsg.statusCode)
    {
        //OK
        processCommonResponse(iIncomingMsg);
        //ChangeInternalState(PVRTSP_ENGINE_NODE_STATE_SETUP_DONE);
        //break;

        if (tmpOutgoingMsg->method == METHOD_SETUP)
        {
            for (uint32 i = 0; i < iSessionInfo.iSelectedStream.size(); i++)
            {
                if (!oscl_strncmp(tmpOutgoingMsg->originalURI.c_str(), iSessionInfo.iSelectedStream[i].iMediaURI.get_cstr(), tmpOutgoingMsg->originalURI.length()))
                {
                    if (iIncomingMsg.numOfTransportEntries)
                    {
                        {//For transport options, we only let server choose
                            //between "RTP/AVP/UDP" or "x-pn-tng/tcp"; and for "x-pn-tng/tcp",
                            //we only do http cloaking. 06/03/21
                            ibIsRealRDT = false;
                                if ((iIncomingMsg.transport[0].protocol != RtspTransport::RTP_PROTOCOL)
                                        || (iIncomingMsg.transport[0].profile != RtspTransport::AVP_PROFILE))
                                {//PVMFRTSPClientEngineNodeErrorUnsupportedTransport
                                    iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorMalformedRTSPMessage;
                                    return PVMFFailure;
                                }
                        }

                        iSessionInfo.iSelectedStream[i].ssrcIsSet = iIncomingMsg.transport[0].ssrcIsSet;
                        if (iIncomingMsg.transport[0].ssrcIsSet)
                        {
                            iSessionInfo.iSelectedStream[i].iSSRC = iIncomingMsg.transport[0].ssrc;
                        }
                        if (!(iIncomingMsg.transport[0].server_portIsSet || iIncomingMsg.transport[0].channelIsSet))
                        {
                            iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorMalformedRTSPMessage;
                            return PVMFFailure;
                        }
                        if (iIncomingMsg.transport[0].server_portIsSet)
                        {
                            iSessionInfo.iSelectedStream[i].iSerRTPPort	= iIncomingMsg.transport[0].server_port1;	//RTP
                            iSessionInfo.iSelectedStream[i].iSerRTCPPort = iIncomingMsg.transport[0].server_port2;	//RTCP
                        }
                        if (iIncomingMsg.transport[0].channelIsSet)
                        {
                            iSessionInfo.iSelectedStream[i].iSerRTPPort	= iIncomingMsg.transport[0].channel1;	//RTP
                            iSessionInfo.iSelectedStream[i].iSerRTCPPort = iIncomingMsg.transport[0].channel2;	//RTCP

                            //since we're here, let's set the channel id for the port as well
                            for (int32 j = iPortVector.size() - 1; j >= 0; j--)
                            {
                                if (((PVMFRTSPPort*)(iPortVector[j]))->iSdpTrackID == iSessionInfo.iSelectedStream[i].iSDPStreamId)
                                {
                                    PVMFRTSPPort *pvPort = (PVMFRTSPPort*)(iPortVector[j]);
                                    pvPort->iChannelID = (pvPort->bIsMedia)
                                                         ? iSessionInfo.iSelectedStream[i].iSerRTPPort
                                                         : iSessionInfo.iSelectedStream[i].iSerRTCPPort;
                                    pvPort->bIsChannelIDSet = true;
                                }
                            }
                        }
                        iSessionInfo.iSelectedStream[i].iSerIpAddr.Set(iSessionInfo.iSrvAdd.ipAddr.Str());
                    }
                    else
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVRTSPEngineNode::processIncomingMessage() ERROR iIncomingMsg.numOfTransportEntries = %d Ln %d", iIncomingMsg.numOfTransportEntries, __LINE__));
                        iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorMalformedRTSPMessage;
                        return PVMFFailure;
                    }
                    break;
                }
            }

            if (!iSessionInfo.tSIDIsSetFlag)
            {
                //server does not send the session ID
                iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorMissingSessionIdInServerResponse;
                return PVMFFailure;
            }
            uint32 clock = 0;
            bool overflowFlag = false;
            iRoundTripClockTimeBase.GetCurrentTime32(clock, overflowFlag, PVMF_MEDIA_CLOCK_MSEC);
            clock -= (uint32)iSessionInfo.clientServerDelay;
            iSessionInfo.clientServerDelay = clock;
            iSessionInfo.roundTripDelay =
                Oscl_Int64_Utils::get_uint64_lower32(iSessionInfo.clientServerDelay);
        }
        else if (tmpOutgoingMsg->method == METHOD_PLAY)
        {
            if (iSessionInfo.iActPlayRange.format == RtspRangeType::INVALID_RANGE)
            {//play
                iSessionInfo.iActPlayRange	= iSessionInfo.iReqPlayRange;
            }

            if (iIncomingMsg.rangeIsSet)
            {
                if ((iSessionInfo.iActPlayRange.format == iIncomingMsg.range.format)
                        && (iIncomingMsg.range.format == RtspRangeType::NPT_RANGE))
                {
                    if (iIncomingMsg.range.start_is_set)
                    {
                        iSessionInfo.iActPlayRange.start_is_set = true;
                        iSessionInfo.iActPlayRange.npt_start	= iIncomingMsg.range.npt_start;
                    }
                    if (iIncomingMsg.range.end_is_set)
                    {
                        iSessionInfo.iActPlayRange.end_is_set = true;
                        iSessionInfo.iActPlayRange.npt_end	= iIncomingMsg.range.npt_end;
                    }
                }
                else
                {//the server actually changes the range format in one session
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVRTSPEngineNode::processIncomingMessage() ERROR iIncomingMsg.range.format = %d Ln %d", iIncomingMsg.range.format, __LINE__));
                }
            }

            for (uint32 idx = 0; idx < iIncomingMsg.numOfRtpInfoEntries; idx++)
            {
                RTSPRTPInfo *rtpinfo = iIncomingMsg.rtpInfo + idx;
                if (!rtpinfo->urlIsSet)
                {//error
                    //break;
                }
                char *url_temp = (char *) rtpinfo->url.c_str();
                //int url_len = rtpinfo->url.length() + 1;//Added to prevent erroneous track selection.

                for (uint32 i = 0; i < iSessionInfo.iSelectedStream.size(); i++)
                {
                    int sdp_track_id = iSessionInfo.iSelectedStream[i].iSDPStreamId;
                    //simplified check here.
                    if (NULL == oscl_strstr((iSessionInfo.iSDPinfo->getMediaInfoBasedOnID(sdp_track_id))->getControlURL(), url_temp))
                    {
                        if (NULL == oscl_strstr(url_temp, (iSessionInfo.iSDPinfo->getMediaInfoBasedOnID(sdp_track_id))->getControlURL()))
                        {
                            continue;
                        }
                    }
                    iSessionInfo.iSelectedStream[i].seqIsSet = rtpinfo->seqIsSet;
                    iSessionInfo.iSelectedStream[i].seq = rtpinfo->seq;
                    iSessionInfo.iSelectedStream[i].rtptimeIsSet = rtpinfo->rtptimeIsSet;
                    iSessionInfo.iSelectedStream[i].rtptime = rtpinfo->rtptime;
                    iSessionInfo.iSelectedStream[i].iSerIpAddr.Set(iSessionInfo.iSrvAdd.ipAddr.Str());
                }
            }
            uint32 clock = 0;
            bool overflowFlag = false;
            iRoundTripClockTimeBase.GetCurrentTime32(clock, overflowFlag, PVMF_MEDIA_CLOCK_MSEC);
            clock -= (uint32)iSessionInfo.clientServerDelay;
            iSessionInfo.clientServerDelay = clock;
            iSessionInfo.roundTripDelay =
                Oscl_Int64_Utils::get_uint64_lower32(iSessionInfo.clientServerDelay);
        }
        else if (tmpOutgoingMsg->method == iKeepAliveMethod)
        {
            if ((iState == PVRTSP_ENGINE_NODE_STATE_PAUSE_DONE) ||
                    ((bKeepAliveInPlay) && (iState == PVRTSP_ENGINE_NODE_STATE_PLAY_DONE)))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO, (0, "PVRTSPEngineNode::processIncomingMessage() start keep-alive timer %d. Ln %d", TIMEOUT_KEEPALIVE, __LINE__));
                iWatchdogTimer->Request(REQ_TIMER_KEEPALIVE_ID, 0, TIMEOUT_KEEPALIVE);
            }
        }

        if (tmpOutgoingMsg)
            OSCL_DELETE(tmpOutgoingMsg);
    }
    else if ((iIncomingMsg.statusCode >= 300) && (iIncomingMsg.statusCode < 400))
    {
        //redirect
        int32 infocode;
        MapRTSPCodeToEventCode(iIncomingMsg.statusCode, infocode);
        ReportInfoEvent(PVMFInfoRemoteSourceNotification, NULL, &iEventUUID, &infocode);
        OSCL_DELETE(tmpOutgoingMsg);

        const StrPtrLen *tmpBaseURLLoc = iIncomingMsg.queryField("Location");
        //const StrPtrLen *tmpBaseURLLoc = iIncomingMsg.queryField("Content-Location");
        if (tmpBaseURLLoc)
        {
            if (!parseURL(tmpBaseURLLoc->c_str()))
            {
                return PVMFFailure;
            }
        }
        else
        {
            return PVMFFailure;   //If Location is not present in response
        }
        iErrorRecoveryAttempt = iNumRedirectTrials-- ? 1 : 0;
        if (iErrorRecoveryAttempt == 0)
        {
            iCurrentErrorCode = infocode; // Send error to application
            return PVMFFailure;
        }
        return PVMFInfoRemoteSourceNotification;
    }
    else
    {
        if (tmpOutgoingMsg)
        {
            if (tmpOutgoingMsg->method == METHOD_OPTIONS)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVRTSPEngineNode::processIncomingMessage() OPTIONS ERROR %d Line %d", iIncomingMsg.statusCode, __LINE__));
                OSCL_DELETE(tmpOutgoingMsg);
                return iOutgoingMsgQueue.empty() ? PVMFSuccess : PVMFPending;
            }
        }

        //error
        MapRTSPCodeToEventCode(iIncomingMsg.statusCode, iCurrentErrorCode);
        OSCL_DELETE(tmpOutgoingMsg);
        return PVMFFailure;
    }

    return iOutgoingMsgQueue.empty() ? PVMFSuccess : PVMFPending;
    //return PVMFSuccess;
}

void PVRTSPEngineNode::MapRTSPCodeToEventCode(RTSPStatusCode aStatusCode,
        int32& aEventCode)
{
    switch (aStatusCode)
    {
        case 300:
            //"300"      ; Multiple Choices
            aEventCode = PVMRTSPClientEngineInfoRTSPRedirectCode300;
            break;
        case 301:
            //"301"      ; Moved Permanently
            aEventCode = PVMRTSPClientEngineInfoRTSPRedirectCode301;
            break;
        case 302:
            //"302"      ; Moved Temporarily
            aEventCode = PVMRTSPClientEngineInfoRTSPRedirectCode302;
            break;
        case 303:
            //"303"      ; See Other
            aEventCode = PVMRTSPClientEngineInfoRTSPRedirectCode303;
            break;
        case 304:
            //"304"      ; Not Modified
            aEventCode = PVMRTSPClientEngineInfoRTSPRedirectCode304;
            break;
        case 305:
            //"305"      ; Use Proxy
            aEventCode = PVMRTSPClientEngineInfoRTSPRedirectCode305;
            break;
        case 400:
            //"400"      ; Bad Request
            aEventCode = PVMFRTSPClientEngineNodeErrorRTSPErrorCode400;
            break;
        case 401:
            //"401"      ; Unauthorized
            aEventCode = PVMFRTSPClientEngineNodeErrorRTSPErrorCode401;
            break;
        case 402:
            //"402"      ; Payment Required
            aEventCode = PVMFRTSPClientEngineNodeErrorRTSPErrorCode402;
            break;
        case 403:
            //"403"      ; Forbidden
            aEventCode = PVMFRTSPClientEngineNodeErrorRTSPErrorCode403;
            break;
        case 404:
            //"404"      ; Not Found
            aEventCode = PVMFRTSPClientEngineNodeErrorRTSPErrorCode404;
            break;
        case 405:
            //"405"      ; Method Not Allowed
            aEventCode = PVMFRTSPClientEngineNodeErrorRTSPErrorCode405;
            break;
        case 406:
            //"406"      ; Not Acceptable
            aEventCode = PVMFRTSPClientEngineNodeErrorRTSPErrorCode406;
            break;
        case 407:
            //"407"      ; Proxy Authentication Required
            aEventCode = PVMFRTSPClientEngineNodeErrorRTSPErrorCode407;
            break;
        case 408:
            //"408"      ; Request Time-out
            aEventCode = PVMFRTSPClientEngineNodeErrorRTSPErrorCode408;
            break;
        case 410:
            //"410"      ; Gone
            aEventCode = PVMFRTSPClientEngineNodeErrorRTSPErrorCode410;
            break;
        case 411:
            //"411"      ; Length Required
            aEventCode = PVMFRTSPClientEngineNodeErrorRTSPErrorCode411;
            break;
        case 412:
            //"412"      ; Precondition Failed
            aEventCode = PVMFRTSPClientEngineNodeErrorRTSPErrorCode412;
            break;
        case 413:
            //"413"      ; Request Entity Too Large
            aEventCode = PVMFRTSPClientEngineNodeErrorRTSPErrorCode413;
            break;
        case 414:
            //"414"      ; Request-URI Too Large
            aEventCode = PVMFRTSPClientEngineNodeErrorRTSPErrorCode414;
            break;
        case 415:
            //"415"      ; Unsupported Media Type
            aEventCode = PVMFRTSPClientEngineNodeErrorRTSPErrorCode415;
            break;
        case 451:
            //"451"      ; Parameter Not Understood
            aEventCode = PVMFRTSPClientEngineNodeErrorRTSPErrorCode451;
            break;
        case 452:
            //"452"      ; Conference Not Found
            aEventCode = PVMFRTSPClientEngineNodeErrorRTSPErrorCode452;
            break;
        case 453:
            //"453"      ; Not Enough Bandwidth
            aEventCode = PVMFRTSPClientEngineNodeErrorRTSPErrorCode453;
            break;
        case 454:
            //"454"      ; Session Not Found
            aEventCode = PVMFRTSPClientEngineNodeErrorRTSPErrorCode454;
            break;
        case 455:
            //"455"      ; Method Not Valid in This State
            aEventCode = PVMFRTSPClientEngineNodeErrorRTSPErrorCode455;
            break;
        case 456:
            //"456"      ; Header Field Not Valid for Resource
            aEventCode = PVMFRTSPClientEngineNodeErrorRTSPErrorCode456;
            break;
        case 457:
            //"457"      ; Invalid Range
            aEventCode = PVMFRTSPClientEngineNodeErrorRTSPErrorCode457;
            break;
        case 458:
            //"458"      ; Parameter Is Read-Only
            aEventCode = PVMFRTSPClientEngineNodeErrorRTSPErrorCode458;
            break;
        case 459:
            //"459"      ; Aggregate operation not allowed
            aEventCode = PVMFRTSPClientEngineNodeErrorRTSPErrorCode459;
            break;
        case 460:
            //"460"      ; Only aggregate operation allowed
            aEventCode = PVMFRTSPClientEngineNodeErrorRTSPErrorCode460;
            break;
        case 461:
            //"461"      ; Unsupported transport
            aEventCode = PVMFRTSPClientEngineNodeErrorRTSPErrorCode461;
            break;
        case 462:
            //"462"      ; Destination unreachable
            aEventCode = PVMFRTSPClientEngineNodeErrorRTSPErrorCode462;
            break;
        case 500:
            //"500"      ; Internal Server Error
            aEventCode = PVMFRTSPClientEngineNodeErrorRTSPErrorCode500;
            break;
        case 501:
            //"501"      ; Not Implemented
            aEventCode = PVMFRTSPClientEngineNodeErrorRTSPErrorCode501;
            break;
        case 502:
            //"502"      ; Bad Gateway
            aEventCode = PVMFRTSPClientEngineNodeErrorRTSPErrorCode502;
            break;
        case 503:
            //"503"      ; Service Unavailable
            aEventCode = PVMFRTSPClientEngineNodeErrorRTSPErrorCode503;
            break;
        case 504:
            //"504"      ; Gateway Time-out
            aEventCode = PVMFRTSPClientEngineNodeErrorRTSPErrorCode504;
            break;
        case 505:
            //"505"      ; RTSP Version not supported
            aEventCode = PVMFRTSPClientEngineNodeErrorRTSPErrorCode505;
            break;
        case 551:
            //"551"      ; Option not supported
            aEventCode = PVMFRTSPClientEngineNodeErrorRTSPErrorCode551;
            break;
        default:
            // Unknown
            aEventCode = PVMFRTSPClientEngineNodeErrorRTSPCodeUnknown;
            break;
    }
}

PVMFStatus PVRTSPEngineNode::DoPauseNode(PVRTSPEngineCommand &aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::DoPauseNode() In"));

    //If session is completed, then do not send the pause command to the server..
    if (IsSessionCompleted())
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::DoPauseNode() Skipping sending pause 'cos of session expiry"));
        ChangeInternalState(PVRTSP_ENGINE_NODE_STATE_PAUSE_DONE);
        return PVMFSuccess;
    }

    if (iInterfaceState != EPVMFNodeStarted)
    {
        return PVMFErrInvalidState;
    }
    return SendRtspPause(aCmd);
}

PVMFStatus PVRTSPEngineNode::DoStopNode(PVRTSPEngineCommand &aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::DoStopNode() In"));

    if ((iInterfaceState != EPVMFNodeStarted) && (iInterfaceState != EPVMFNodePaused))
    {
        if (iInterfaceState == EPVMFNodeError)
        {
            return PVMFSuccess;
        }
        return PVMFErrInvalidState;
    }
    return SendRtspTeardown(aCmd);
}

PVMFStatus PVRTSPEngineNode::SendRtspPause(PVRTSPEngineCommand &aCmd)
{
    OSCL_UNUSED_ARG(aCmd);

    PVMFStatus iRet = PVMFPending;
    switch (iState)
    {
        case PVRTSP_ENGINE_NODE_STATE_PAUSE_DONE:
        {
            iRet = PVMFSuccess;
            break;
        }
        case PVRTSP_ENGINE_NODE_STATE_PLAY_DONE:
        {
            if (bNoSendPending)
            {
                RTSPOutgoingMessage *tmpOutgoingMsg =  OSCL_NEW(RTSPOutgoingMessage, ());
                if (tmpOutgoingMsg == NULL)
                {
                    iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorOutOfMemory;
                    return  PVMFFailure;
                }

                if (PVMFSuccess != composePauseRequest(*tmpOutgoingMsg))
                {
                    iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorRTSPComposePauseRequestError;
                    OSCL_DELETE(tmpOutgoingMsg);
                    return  PVMFFailure;
                }

                if (PVMFSuccess != sendSocketOutgoingMsg(iSendSocket, *tmpOutgoingMsg))
                {
                    /* need to pop the msg based on cseq, NOT necessarily the early ones,
                    although YES in this case.
                    //for(int idx=0; idx < iSessionInfo.trackSelectionList->getNumTracks(); idx++ )
                    for(int idx=0; idx < iSessionInfo.iSDPinfo->getNumMediaObjects(); idx++ )
                    {
                    //iOutgoingMsgQueue.pop();
                    }
                    	*/
                    iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorSocketSendError;
                    OSCL_DELETE(tmpOutgoingMsg);
                    iRet =  PVMFFailure;
                    break;
                }

                bNoSendPending = false;
                iOutgoingMsgQueue.push(tmpOutgoingMsg);
                ChangeInternalState(PVRTSP_ENGINE_NODE_STATE_WAIT_PAUSE);

                //setup the watch dog for server response
                iWatchdogTimer->Request(REQ_TIMER_WATCHDOG_ID, 0, TIMEOUT_WATCHDOG);
            }
            break;
        }
        case PVRTSP_ENGINE_NODE_STATE_WAIT_PAUSE:
        {
            if (RTSPParser::REQUEST_IS_READY == iRTSPParserState)
            {
                iRet = processIncomingMessage(iIncomingMsg);
                if (iRet != PVMFPending)
                {
                    //cancell the watchdog
                    iWatchdogTimer->Cancel(REQ_TIMER_WATCHDOG_ID);
                    if (iRet == PVMFSuccess)
                    {
                        ChangeInternalState(PVRTSP_ENGINE_NODE_STATE_PAUSE_DONE);
                    }
                    else if ((iRet == PVMFFailure) && (iCurrentErrorCode == PVMFRTSPClientEngineNodeErrorRTSPErrorCode455))
                    {//"455"      ; Method Not Valid in This State
                        iRet = PVMFSuccess;
                        ChangeInternalState(PVRTSP_ENGINE_NODE_STATE_PAUSE_DONE);
                    }
                    else
                    {	//revert to previous state,
                        //which means don't change to EPVMFNodeError state
                        ChangeInternalState(PVRTSP_ENGINE_NODE_STATE_PLAY_DONE);
                    }
                }
            }
            else if (RTSPParser::ENTITY_BODY_IS_READY == iRTSPParserState)
            {//got MS TCP RTP packets
                //processSDP(REINTERPRET_CAST(mbchar*, pSDPBuf.ptr), pSDPBuf.len-1);
                //processSDP((mbchar*)pSDPBuf.ptr, pSDPBuf.len-1);
                //ChangeInternalState(PVRTSP_ENGINE_NODE_STATE_WAIT_DESCRIBE_DONE);
                iRet =  PVMFSuccess;
            }
            else if (!clearEventQueue())
            {
                //cancell the watchdog
                iWatchdogTimer->Cancel(REQ_TIMER_WATCHDOG_ID);

                iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorSocketError;
                iRet =  PVMFFailure;
            }
            break;
        }
        default:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVRTSPEngineNode::SendRtspPause() iState=%d Line %d", iState, __LINE__));
            iRet = PVMFErrInvalidState;
            break;
        }
    }

    return iRet;
}

PVMFStatus PVRTSPEngineNode::DoResetNode(PVRTSPEngineCommand &aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::DoResetNode() In"));
    if (iGetPostCorrelationObject != NULL)
    {
        OSCL_DELETE(iGetPostCorrelationObject);
        iGetPostCorrelationObject = NULL;
    }
    return SendRtspTeardown(aCmd);
}


PVMFStatus PVRTSPEngineNode::SendRtspTeardown(PVRTSPEngineCommand &aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::SendRtspTeardown() In"));
    OSCL_UNUSED_ARG(aCmd);
    /* Allow reset from any state */
    PVMFStatus iRet = PVMFPending;
    switch (iState)
    {
        case PVRTSP_ENGINE_NODE_STATE_SETUP_DONE:
        case PVRTSP_ENGINE_NODE_STATE_PAUSE_DONE:
        case PVRTSP_ENGINE_NODE_STATE_PLAY_DONE:
        {
            if (bNoSendPending)
            {
                RTSPOutgoingMessage *tmpOutgoingMsg =  OSCL_NEW(RTSPOutgoingMessage, ());
                if (tmpOutgoingMsg == NULL)
                {
                    iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorOutOfMemory;
                    return  PVMFFailure;
                }

                if (PVMFSuccess != composeStopRequest(*tmpOutgoingMsg))
                {
                    iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorRTSPComposeStopRequestError;
                    OSCL_DELETE(tmpOutgoingMsg);
                    return  PVMFFailure;
                }
                if (PVMFSuccess != sendSocketOutgoingMsg(iSendSocket, *tmpOutgoingMsg))
                {
                    iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorSocketSendError;
                    OSCL_DELETE(tmpOutgoingMsg);
                    //maybe server closed the connection or some other errors
                    bNoRecvPending = false; //prevent doing recv() after the TEARDOWN
                    iRet = PVMFSuccess;
                    ChangeInternalState(PVRTSP_ENGINE_NODE_STATE_CONNECT);
                    break;
                }

                //setup the watchdog for server response
                iWatchdogTimer->Request(REQ_TIMER_WATCHDOG_ID, 0, TIMEOUT_WATCHDOG_TEARDOWN);

                bNoSendPending = false;
                iOutgoingMsgQueue.push(tmpOutgoingMsg);
                ChangeInternalState(PVRTSP_ENGINE_NODE_STATE_WAIT_STOP);
            }
            break;
        }

        case PVRTSP_ENGINE_NODE_STATE_WAIT_CALLBACK:
            if ((iNumHostCallback + iNumConnectCallback + iNumSendCallback + iNumRecvCallback) == 0
                    && resetSocket() != PVMFPending)
            {
                iRet = PVMFSuccess;
                ChangeInternalState(PVRTSP_ENGINE_NODE_STATE_CONNECT);
            }
            break;

        case PVRTSP_ENGINE_NODE_STATE_WAIT_STOP:
            if (clearEventQueue() && (RTSPParser::REQUEST_IS_READY != iRTSPParserState))
            {//no error and didn't get sth, TEARDOWN response or sth else, doesn't matter
                break;
            }
            //just go to default to cleanup
        default:
        {
            bNoRecvPending = false; //prevent doing recv() after the TEARDOWN
            bNoSendPending = false; //prevent doing send() after the TEARDOWN

            clearOutgoingMsgQueue();
            iSocketEventQueue.clear();

            PVMFStatus status = resetSocket();

            iWatchdogTimer->Cancel(REQ_TIMER_WATCHDOG_ID);
            iWatchdogTimer->Cancel(REQ_TIMER_KEEPALIVE_ID);
            REQ_TIMER_WATCHDOG_ID = REQ_TIMER_KEEPALIVE_ID = 0;

            if ((iNumHostCallback + iNumConnectCallback + iNumSendCallback + iNumRecvCallback) == 0
                    && status != PVMFPending)
            {
                iRet = PVMFSuccess;
                ChangeInternalState(PVRTSP_ENGINE_NODE_STATE_CONNECT);
            }
            else
            {
                ChangeInternalState(PVRTSP_ENGINE_NODE_STATE_WAIT_CALLBACK);
            }
            break;
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::SendRtspTeardown() Out"));

    return iRet;
}

PVMFStatus PVRTSPEngineNode::DoQueryUuid(PVRTSPEngineCommand &aCmd)
{
    //This node supports Query UUID from any state

    OSCL_String* mimetype;
    Oscl_Vector<PVUuid, PVRTSPEngineNodeAllocator> *uuidvec;
    bool exactmatch;
    aCmd.PVRTSPEngineCommandBase::Parse(mimetype, uuidvec, exactmatch);

    //Try to match the input mimetype against any of
    //the custom interfaces for this node
    //Match against custom interface1...
    if (*mimetype == PVMF_RTSPENGINENODE_CUSTOM1_MIMETYPE
            //also match against base mimetypes for custom interface1,
            //unless exactmatch is set.
            || (!exactmatch && *mimetype == PVMF_RTSPENGINENODE_MIMETYPE)
            || (!exactmatch && *mimetype == PVMF_RTSPENGINENODE_BASEMIMETYPE))
    {
        uuidvec->push_back(KPVRTSPEngineNodeExtensionUuid);
    }

    return PVMFSuccess;
}

PVMFStatus PVRTSPEngineNode::DoQueryInterface(PVRTSPEngineCommand &aCmd)
{
    //This node supports Query Interface from any state

    PVUuid* uuid;
    PVInterface** ptr;
    aCmd.PVRTSPEngineCommandBase::Parse(uuid, ptr);

    if (*uuid == KPVRTSPEngineNodeExtensionUuid)
    {
        if (!iExtensionInterface)
        {
            iExtensionInterface = OSCL_NEW(PVRTSPEngineNodeExtensionInterfaceImpl, (this));
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVRTSPEngineNode:DoQueryInterface iExtensionInterface %x "
                             , iExtensionInterface));
        }
        if (iExtensionInterface)
        {
            if (iExtensionInterface->queryInterface(*uuid, *ptr))
            {
                return PVMFSuccess;
            }
            else
            {
                return PVMFErrNotSupported;
            }
        }
        else
        {
            return PVMFErrNoMemory;
        }
    }
    else
    {//not supported
        *ptr = NULL;
        return PVMFErrNotSupported;
    }
}

void PVRTSPEngineNode::ReportErrorEvent(PVMFEventType aEventType,
                                        OsclAny* aEventData,
                                        PVUuid* aEventUUID,
                                        int32* aEventCode)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                    (0, "PVRTSPEngineNode:NodeErrorEvent Type %d Data %d"
                     , aEventType, aEventData));

    if (aEventUUID && aEventCode)
    {
        PVMFBasicErrorInfoMessage* eventmsg =
            OSCL_NEW(PVMFBasicErrorInfoMessage, (*aEventCode, *aEventUUID, NULL));
        PVMFAsyncEvent asyncevent(PVMFErrorEvent,
                                  aEventType,
                                  NULL,
                                  OSCL_STATIC_CAST(PVInterface*, eventmsg),
                                  aEventData,
                                  NULL,
                                  0);
        PVMFNodeInterface::ReportErrorEvent(asyncevent);
        eventmsg->removeRef();
    }
    else
    {
        PVMFNodeInterface::ReportErrorEvent(aEventType, aEventData);
    }
}

void PVRTSPEngineNode::ReportInfoEvent(PVMFEventType aEventType,
                                       OsclAny* aEventData,
                                       PVUuid* aEventUUID,
                                       int32* aEventCode)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVRTSPEngineNode:NodeInfoEvent Type %d Data %d"
                     , aEventType, aEventData));

    if (aEventUUID && aEventCode)
    {
        PVMFBasicErrorInfoMessage* eventmsg =
            OSCL_NEW(PVMFBasicErrorInfoMessage, (*aEventCode, *aEventUUID, NULL));
        PVMFAsyncEvent asyncevent(PVMFInfoEvent,
                                  aEventType,
                                  NULL,
                                  OSCL_STATIC_CAST(PVInterface*, eventmsg),
                                  aEventData,
                                  NULL,
                                  0);
        PVMFNodeInterface::ReportInfoEvent(asyncevent);
        eventmsg->removeRef();
    }
    else
    {
        PVMFNodeInterface::ReportInfoEvent(aEventType, aEventData);
    }
}


/////////////////////////////////////////////////////
// Port Processing routines
/////////////////////////////////////////////////////
void PVRTSPEngineNode::HandlePortActivity(const PVMFPortActivity &aActivity)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "0x%x PVRTSPEngineNode::HandlePortActivity: port=0x%x, type=%d IQ=%d OQ=%d",
                     this, aActivity.iPort, aActivity.iType, aActivity.iPort->IncomingMsgQueueSize(), aActivity.iPort->OutgoingMsgQueueSize()));
    switch (aActivity.iType)
    {
        case PVMF_PORT_ACTIVITY_OUTGOING_MSG:
            //An outgoing message was queued on this port.
            //We only need to queue a port activity event on the
            //first message.  Additional events will be queued during
            //the port processing as needed.
            if (aActivity.iPort->OutgoingMsgQueueSize() == 1)
                QueuePortActivity(aActivity);
            break;
        case PVMF_PORT_ACTIVITY_CONNECTED_PORT_BUSY:
            iTheBusyPort = aActivity.iPort;
            break;

        case PVMF_PORT_ACTIVITY_OUTGOING_QUEUE_BUSY:
        case PVMF_PORT_ACTIVITY_OUTGOING_QUEUE_READY:
            //iTheBusyPort = iTheBusyPort;
            break;

        case PVMF_PORT_ACTIVITY_CONNECTED_PORT_READY:
            if (iTheBusyPort == aActivity.iPort)
            {
                for (int32 i = iPortVector.size() - 1; i >= 0; i--)
                {
                    if (((PVMFRTSPPort*)(iPortVector[i]))->OutgoingMsgQueueSize() > 0)
                    {
                        PVMFPortActivity activity(aActivity.iPort, PVMF_PORT_ACTIVITY_OUTGOING_MSG);
                        QueuePortActivity(activity);
                    }
                }

                if (iRTSPParserState == RTSPParser::EMBEDDED_DATA_IS_READY)
                {
                    if (!ibBlockedOnFragGroups)
                    {
                        DispatchEmbeddedData(iIncomingMsg.channelID);
                    }
                }
                iTheBusyPort = NULL;
            }
            break;

        case PVMF_PORT_ACTIVITY_CONNECT:
        default:
            break;
    }
}

/////////////////////////////////////////////////////
// Port Processing routines
/////////////////////////////////////////////////////

void PVRTSPEngineNode::QueuePortActivity(const PVMFPortActivity &aActivity)
{
    //queue a new port activity event
    int32 err;
    OSCL_TRY(err, iPortActivityQueue.push_back(aActivity););
    if (err != OsclErrNone)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "0x%x PVRTSPEngineNode::QueuePortActivity: Error - iPortActivityQueue.push_back() failed", this));
        ReportErrorEvent(PVMFErrPortProcessing, (OsclAny*)(aActivity.iPort));
    }
    else
    {
        //wake up the AO to process the port activity event.
        RunIfNotReady();
    }
}

void PVRTSPEngineNode::TimeoutOccurred(int32 timerID, int32 timeoutInfo)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::TimeoutOccurred() in timerID=%d", timerID));
    OSCL_UNUSED_ARG(timeoutInfo);

    if (!IsAdded())
    {//prevent the leave 49. should never get here
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVRTSPEngineNode::TimeoutOccurred() ERROR line %d", __LINE__));
        return;
    }
    if ((timerID != REQ_TIMER_WATCHDOG_ID) && (timerID != REQ_TIMER_KEEPALIVE_ID) && (PVRTSP_ENGINE_NODE_STATE_WAIT_CALLBACK == iState))
    {//waiting for the callback to finish the Stop() or Reset()
        RunIfNotReady();
        return;
    }

    if (timerID == REQ_TIMER_WATCHDOG_ID)
    {//watchdog
        SocketEvent tmpSockEvent;
        tmpSockEvent.iSockId	= timerID;
        tmpSockEvent.iSockFxn = EPVSocketRecv;
        tmpSockEvent.iSockEvent = EPVSocketTimeout;
        tmpSockEvent.iSockError = 0;

        iSocketEventQueue.push_back(tmpSockEvent);
        RunIfNotReady();
    }
    else if (timerID == REQ_TIMER_KEEPALIVE_ID)
    {//keep-alive
        if ((bNoSendPending) &&
                ((iState == PVRTSP_ENGINE_NODE_STATE_PAUSE_DONE)
                 || ((bKeepAliveInPlay) && (iState == PVRTSP_ENGINE_NODE_STATE_PLAY_DONE))))
        {
            RTSPOutgoingMessage *tmpOutgoingMsg =  OSCL_NEW(RTSPOutgoingMessage, ());
            if (tmpOutgoingMsg == NULL)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVRTSPEngineNode::TimeoutOccurred ERROR out-of-memory. Ln %d", __LINE__));
                //iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorOutOfMemory;
                return;
            }

            if (PVMFSuccess != composeKeepAliveRequest(*tmpOutgoingMsg))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVRTSPEngineNode::TimeoutOccurred ERROR composeKeepAliveRequest fail. Ln %d", __LINE__));
                OSCL_DELETE(tmpOutgoingMsg);
                //iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorRTSPComposeStopRequestError;
                return;
            }
            if (PVMFSuccess != sendSocketOutgoingMsg(iSendSocket, *tmpOutgoingMsg))
            {
                iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorSocketSendError;
                OSCL_DELETE(tmpOutgoingMsg);
                return;
                //iRet =  PVMFFailure;
                //break;
            }

            bNoSendPending = false;
            iOutgoingMsgQueue.push(tmpOutgoingMsg);
        }
    }
}

PVMFStatus PVRTSPEngineNode::composeKeepAliveRequest(RTSPOutgoingMessage &aMsg)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::composeKeepAliveRequest() in"));

    aMsg.reset();
    aMsg.numOfTransportEntries = 0;
    aMsg.msgType = RTSPRequestMsg;
    aMsg.method = iKeepAliveMethod;
    aMsg.cseq = iOutgoingSeq++;
    aMsg.cseqIsSet = true;
    aMsg.userAgent = iSessionInfo.iUserAgent.get_cstr();
    aMsg.userAgentIsSet = true;

    if (iSessionInfo.iSID.get_size())
    {
        aMsg.sessionId.setPtrLen(iSessionInfo.iSID.get_cstr(), iSessionInfo.iSID.get_size());
        aMsg.sessionIdIsSet = true;
    }

    if (composeSessionURL(aMsg) != PVMFSuccess)
    {
        return PVMFFailure;
    }

    if (aMsg.compose() == false)
    {
        return PVMFFailure;
    }
    return PVMFSuccess;
}

OSCL_EXPORT_REF PVMFStatus  PVRTSPEngineNode::SetKeepAliveMethod_timeout(int32 aTimeout)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::SetKeepAliveMethod_timeout() in aTimeout=%d", aTimeout));

    //user server timeout if aTimeout == 0
    aTimeout = aTimeout / 1000;	//sec
    if (aTimeout)
    {
        if (aTimeout > 0)//if(aTimeout > 3000)
        {
            TIMEOUT_KEEPALIVE = aTimeout;
        }
        else
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::SetKeepAliveMethod() ERROR aTimeout=%d. Ln %d", aTimeout, __LINE__));
            return PVMFFailure;
        }
    }
    else
    {
        TIMEOUT_KEEPALIVE = PVRTSPENGINENODE_DEFAULT_KEEP_ALIVE_INTERVAL;
    }

    return PVMFSuccess;
}

OSCL_EXPORT_REF PVMFStatus PVRTSPEngineNode::SetKeepAliveMethod_use_SET_PARAMETER(bool aUseSetParameter)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::SetKeepAliveMethod_timeout() in aUseSetParameter=%d", aUseSetParameter));
    iKeepAliveMethod = aUseSetParameter ? METHOD_SET_PARAMETER : METHOD_OPTIONS;

    return PVMFSuccess;
}

OSCL_EXPORT_REF PVMFStatus PVRTSPEngineNode::SetKeepAliveMethod_keep_alive_in_play(bool aKeepAliveInPlay)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::SetKeepAliveMethod_timeout() in aKeepAliveInPlay=%d", aKeepAliveInPlay));
    bKeepAliveInPlay = aKeepAliveInPlay;

    return PVMFSuccess;
}


OSCL_EXPORT_REF PVMFStatus  PVRTSPEngineNode::GetKeepAliveMethod(int32 &aTimeout, bool &aUseSetParameter, bool &aKeepAliveInPlay)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::GetKeepAliveMethod() in"));

    aTimeout = TIMEOUT_KEEPALIVE * 1000;
    aUseSetParameter = (METHOD_SET_PARAMETER == iKeepAliveMethod);
    aKeepAliveInPlay = bKeepAliveInPlay;

    return PVMFSuccess;
}

OSCL_EXPORT_REF PVMFStatus PVRTSPEngineNode::GetRTSPTimeOut(int32 &aTimeout)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::GetRTSPTimeOut() In"));

    aTimeout = TIMEOUT_WATCHDOG;
    return PVMFSuccess;
}

OSCL_EXPORT_REF PVMFStatus PVRTSPEngineNode::SetRTSPTimeOut(int32 aTimeout)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::SetRTSPTimeOut() In"));

    TIMEOUT_WATCHDOG = aTimeout;
    return PVMFSuccess;
}

PVMFStatus PVRTSPEngineNode::DoRequestPort(PVRTSPEngineCommand &aCmd, PVMFRTSPPort* &aPort)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::DoRequestPort() In"));

    aPort = NULL;

    //retrieve port tag.
    int32 tag;
    OSCL_String* portconfig;
    aCmd.PVRTSPEngineCommandBase::Parse(tag, portconfig);

    OSCL_HeapString<PVRTSPEngineNodeAllocator> tmpPortConfig = portconfig->get_cstr();
    char *head = tmpPortConfig.get_str();
    if ((!head) ||
            ((tag != PVMF_RTSP_NODE_PORT_TYPE_OUTPUT) && (tag != PVMF_RTSP_NODE_PORT_TYPE_INPUT_OUTPUT)))
    {
        return PVMFErrArgument;//invalide portconfig.
    }

    //portconfig should be "sdpTrackIndex=5/media" or "sdpTrackIndex=5/feedback"


    bool bIsMedia = true;
    OSCL_StackString<128> IsMedia("/media");
    char *tmpCh = OSCL_CONST_CAST(char*, oscl_strstr(head, IsMedia.get_cstr()));
    if (!tmpCh)
    {
        OSCL_StackString<128> IsFeedback("/feedback");
        tmpCh = OSCL_CONST_CAST(char*, oscl_strstr(head, IsFeedback.get_cstr()));
        if (!tmpCh)
        {
            return PVMFErrArgument;
        }
        bIsMedia = false;
    }
    *tmpCh = '\0';	//set the delimiter for atoi
    OSCL_StackString<128> sdpTrackIndex("sdpTrackIndex=");
    tmpCh = OSCL_CONST_CAST(char*, oscl_strstr(head, sdpTrackIndex.get_cstr()));
    if (!tmpCh)
    {
        return PVMFErrArgument;
    }

    tmpCh += sdpTrackIndex.get_size();
    uint32 atoi_tmp;
    int32 tmpId = -1; //invalide sdp track id
    if (PV_atoi(tmpCh, 'd', atoi_tmp))
    {
        tmpId = atoi_tmp;
    }

    if (tmpId < 0)
    {
        return PVMFErrArgument;//invalide portconfig.
    }

    // statements were moved to sep. function to  remove compiler warnings caused by OSCL_TRY()
    return DoAddPort(tmpId, bIsMedia, tag, aPort);
}


PVMFStatus PVRTSPEngineNode::DoAddPort(int32 id, bool isMedia, int32 tag, PVMFRTSPPort* &aPort)
{
    int32 leavecode;

    OSCL_TRY(leavecode,
             aPort = OSCL_STATIC_CAST(PVMFRTSPPort*, OSCL_NEW(PVMFRTSPPort, (id, isMedia, tag, this)));
             iPortVector.AddL(aPort);
            );

    if (leavecode != OsclErrNone)
    {
        if (NULL == aPort)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVRTSPEngineNode::DoAddPort() new port fail ERROR leavecode=%d Ln %d", leavecode, __LINE__));
        }

        return PVMFErrNoMemory;
    }

    return PVMFSuccess;
}

PVMFStatus PVRTSPEngineNode::DoReleasePort(PVRTSPEngineCommand &aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::DoReleasePort() In"));

    //Find the port in the port vector
    PVMFPortInterface* port;
    aCmd.PVRTSPEngineCommandBase::Parse(port);

    return PVMFSuccess;
}

/* allocate aReqBufSize memory for iEmbeddedData
return true if memory is successfully allocated
*/
bool PVRTSPEngineNode::PrepareEmbeddedDataMemory(uint32 aReqBufSize, OsclMemoryFragment &aMemFrag)
{
    //PVMFSimpleMediaBufferCombinedAlloc media_data_alloc;
    //OsclSharedPtr<PVMFMediaDataImpl> media_data_imp = media_data_alloc.allocate(aReqBufSize);
    // Allocator for simple media data buffer impl

    OsclSharedPtr<PVMFMediaDataImpl> media_data_imp;
    int32 errcode;
    OSCL_TRY(errcode, media_data_imp = iMediaDataImplAlloc->allocate(aReqBufSize));
    if (errcode != 0)
    {
        OSCL_ASSERT(false);//there is no limit for the allocator num of buffers for now

        ReportErrorEvent(PVMFErrArgument, NULL);
        return false;
    }

    //OsclSharedPtr<PVMFMediaDataImpl> media_data_imp = myAllocate(aReqBufSize);

    // create a media data buffer
    iEmbeddedDataPtr = PVMFMediaData::createMediaData(media_data_imp,   &iAlloc);

    iEmbeddedDataPtr->setMediaFragFilledLen(0, aReqBufSize);

    OsclRefCounterMemFrag refCtrMemFragOut;
    iEmbeddedDataPtr->getMediaFragment(0, refCtrMemFragOut);

    aMemFrag = refCtrMemFragOut.getMemFrag();

    return (aMemFrag.ptr == NULL) ? false : true;
}

/* dispatch the embedded data in iEmbeddedData to different ports according to channel id
return true if send success
*/
bool PVRTSPEngineNode::DispatchEmbeddedData(uint32 aChannelID)
{
    if (iTheBusyPort)
    {
        return false;
    }

    if (ibIsRealRDT)
    {
        return DispatchEmbeddedRdtData();
    }

    PVMFRTSPPort* pvPort = NULL;
    for (int32 i = iPortVector.size() - 1; i >= 0; i--)
    {
        if ((((PVMFRTSPPort*)(iPortVector[i]))->iChannelID == aChannelID)
                && ((PVMFRTSPPort*)(iPortVector[i]))->bIsChannelIDSet)
        {
            pvPort = (PVMFRTSPPort*)(iPortVector[i]);
            break;
        }
    }
    if (pvPort == NULL)
    {//no pvMF port is setup to get this channel, discard the data
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVRTSPEngineNode::DispatchEmbeddedData() channel:%d data discarded.", aChannelID));
        iEmbeddedDataPtr.Unbind();
        return true;
    }

    PVMFSharedMediaMsgPtr aMediaMsgPtr;
    convertToPVMFMediaMsg(aMediaMsgPtr, iEmbeddedDataPtr);
    PVMFStatus status = pvPort->QueueOutgoingMsg(aMediaMsgPtr);
    if (status != PVMFSuccess)
    {
        if (status == PVMFErrBusy)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "PVRTSPEngineNode::DispatchEmbeddedData() BUSY:%d, Outgoing queue size=%d.", status, pvPort->OutgoingMsgQueueSize()));
        }
        else
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVRTSPEngineNode::DispatchEmbeddedData() ERROR:%d, Outgoing queue size=%d. Data discarded!", status, pvPort->OutgoingMsgQueueSize()));
            ReportErrorEvent(PVMFErrPortProcessing);
        }
        return false;
    }
    return true;
}

bool PVRTSPEngineNode::DispatchEmbeddedRdtData()
{
    IPayloadParser::Payload incomingFrag;
    Oscl_Vector<IPayloadParser::Payload, OsclMemAllocator> vRdtPackets;

    // retrieve the incoming RDT packet mem fragment
    OsclRefCounterMemFrag frag;
    iEmbeddedDataPtr->getMediaFragment(0, frag);
    incomingFrag.vfragments.push_back(frag);

    // parse the RDT headers and retrieve the payloads
    if (ipRdtParser->Parse(incomingFrag, vRdtPackets) != PayloadParserStatus_Success)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVRTSPEngineNode::DispatchEmbeddedRdtData() Unable to parse RDT packet!"));
        iEmbeddedDataPtr.Unbind();
        return true;
    }

    uint32 i = 0;
    for (i = 0; i < vRdtPackets.size(); i++)
    {
        if (!ipFragGroupAllocator->IsMsgAvailable())
        {
            // drop this packet
            iEmbeddedDataPtr.Unbind();
            vRdtPackets.clear();
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVRTSPEngineNode::DispatchEmbeddedRdtData() Out of memory! Stopping processing and dropping packet"));

            // block processing until we get a callback
            ibBlockedOnFragGroups = true;
            ipFragGroupAllocator->notifyfreechunkavailable(*this, NULL);
        }

        OsclSharedPtr<PVMFMediaDataImpl> mediaDataImplOut;
        int32 err;
        mediaDataImplOut = AllocateMediaData(err);
        OSCL_ASSERT(err == OsclErrNone); // we just checked that a message is available
        if (err != OsclErrNone)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVRTSPEngineNode::DispatchEmbeddedRdtData() Unable to allocate media data impl!"));
            return true;
        }

        IPayloadParser::Payload rdtOut = vRdtPackets[i];
        mediaDataImplOut->setMarkerInfo(rdtOut.marker);
        mediaDataImplOut->appendMediaFragment(rdtOut.vfragments[0]);

        PVMFSharedMediaDataPtr mediaDataOut = PVMFMediaData::createMediaData(mediaDataImplOut);
        mediaDataOut->setSeqNum(rdtOut.sequence);
        mediaDataOut->setStreamID(rdtOut.stream);
        mediaDataOut->setTimestamp(rdtOut.timestamp);

        // which port?
        PVMFRTSPPort* pvPort = NULL;
        for (int p = iPortVector.size() - 1; p >= 0; p--)
        {
            if ((uint)((PVMFRTSPPort*)(iPortVector[p]))->iRdtStreamId == rdtOut.stream)
            {

                pvPort = (PVMFRTSPPort*)(iPortVector[p]);
                break;
            }
        }

        if (pvPort == NULL)
        {//no pvMF port is setup to get this channel, discard the data
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVRTSPEngineNode::DispatchEmbeddedRdtData() channel:%d data discarded.", rdtOut.stream));
            iEmbeddedDataPtr.Unbind();
            return true;
        }

        PVMFSharedMediaMsgPtr mediaMsgPtr;
        convertToPVMFMediaMsg(mediaMsgPtr, mediaDataOut);

        PVMFStatus status = pvPort->QueueOutgoingMsg(mediaMsgPtr);
        if (status != PVMFSuccess)
        {
            if (status == PVMFErrBusy)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "PVRTSPEngineNode::DispatchEmbeddedRdtData() BUSY:%d, Outgoing queue size=%d.", status, pvPort->OutgoingMsgQueueSize()));
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVRTSPEngineNode::DispatchEmbeddedRdtData() ERROR:%d, Outgoing queue size=%d. Data discarded!", status, pvPort->OutgoingMsgQueueSize()));
                ReportErrorEvent(PVMFErrPortProcessing);
            }

            return false;
        }
    }

    return true;
}

PVMFStatus PVRTSPEngineNode::DoFlush(PVRTSPEngineCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::DoFlush() In"));
    OSCL_UNUSED_ARG(aCmd);

    if ((iInterfaceState != EPVMFNodeStarted) && (iInterfaceState != EPVMFNodePaused))
    {
        return PVMFErrInvalidState;
    }

    //Notify all ports to suspend their input
    {
        for (uint32 i = 0;i < iPortVector.size();i++)
            iPortVector[i]->SuspendInput();
    }

    //the flush is asynchronous.  Completion is detected in the Run.
    //Make sure the AO is active to finish the flush..
    RunIfNotReady();
    return PVMFPending;
}

PVMFStatus PVRTSPEngineNode::sendSocketOutgoingMsg(SocketContainer &aSock, RTSPOutgoingMessage &aMsg)
{
    StrPtrLen *tmpStrPtrLen = aMsg.retrieveComposedBuffer();
    if (NULL == tmpStrPtrLen)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::sendSocketOutgoingMsg() retrieveComposedBuffer() ERROR, line %d", __LINE__));
        iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorRTSPCompose501ResponseError;
        return  PVMFFailure;
    }
    //PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0,"C ---> S\n%s", tmpStrPtrLen->c_str()));
    if (aSock.iSocket->Send((const uint8*)tmpStrPtrLen->c_str(), tmpStrPtrLen->length(), TIMEOUT_SEND) != EPVSocketPending)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::sendSocketOutgoingMsg() Send() ERROR, line %d", __LINE__));
        iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorSocketSendError;
        return PVMFFailure;
    }
    SetSendPending(aSock);
    iNumSendCallback++;
    return PVMFSuccess;
}

PVMFStatus PVRTSPEngineNode::sendSocketOutgoingMsg(SocketContainer &aSock, const uint8* aSendBuf, uint32 aSendLen)
{
    if (aSock.iSocket->Send(aSendBuf, aSendLen, TIMEOUT_SEND) != EPVSocketPending)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::sendSocketOutgoingMsg() Send() ERROR, line %d", __LINE__));
        iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorSocketSendError;
        return PVMFFailure;
    }
    SetSendPending(aSock);
    iNumSendCallback++;
    return PVMFSuccess;
}

//drive the parser, return true if there is a pending event
bool PVRTSPEngineNode::rtspParserLoop(void)
{
    if ((iRTSPParser == NULL) || (NULL != iTheBusyPort)
            || (iRecvSocket.iSocket == NULL) || (EPVMFNodeError == iInterfaceState))
    {
        return false;
    }

    bool bLoop = true, ret = false;
    while (bLoop)
    {
        iRTSPParserState = iRTSPParser->getState();
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "PVRTSPEngineNode::rtspParserLoop() RTSPParser state %d", iRTSPParserState));

        switch (iRTSPParserState)
        {
            case RTSPParser::WAITING_FOR_DATA:
            {
                if (bNoRecvPending)
                {
                    TPVSocketEvent tmpEvent = EPVSocketFailure;
                    const StrPtrLen *tmpbuf = iRTSPParser->getDataBufferSpec();
                    if (tmpbuf)	// unlikely to fail, err rtn from getDataBufferSpec
                    {
                        tmpEvent = iRecvSocket.iSocket->Recv((uint8*)tmpbuf->c_str(), tmpbuf->length(), TIMEOUT_RECV);
                    }
                    if (tmpEvent != EPVSocketPending)
                    {
                        int32 errcode = PVMFRTSPClientEngineNodeErrorSocketRecvError;
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVRTSPEngineNode::rtspParserLoop() Recv failed"));
                        ReportErrorEvent(PVMFErrProcessing, NULL, &iEventUUID, &errcode);
                        ChangeExternalState(EPVMFNodeError);
                    }
                    else
                    {
                        SetRecvPending(iRecvSocket);
                        iNumRecvCallback++;
                    }
                    bNoRecvPending = false;
                }
                bLoop = false;
                break;
            }
            case RTSPParser::WAITING_FOR_REQUEST_MEMORY:
            {
                iIncomingMsg.reset();
                if (!iRTSPParser->registerNewRequestStruct(&iIncomingMsg))
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVRTSPEngineNode::rtspParserLoop() registerNewRequestStruct failed"));
                    ChangeExternalState(EPVMFNodeError);
                }
                break;
            }
            case RTSPParser::ENTITY_BODY_IS_READY:
                ((uint8*)iEntityMemFrag.ptr)[iEntityMemFrag.len-1] = '\0';
                //let the Doxxx() to process the entity body, like sdp, still img, and server msg
            case RTSPParser::REQUEST_IS_READY:
            {
                bLoop = false;
                ret = true;
                break;
            }
            case RTSPParser::WAITING_FOR_ENTITY_BODY_MEMORY:
            {/*
OsclSharedPtr<PVMFMediaDataImpl> media_data_imp;
int32 errcode;
OSCL_TRY(errcode, media_data_imp = iMediaDataImplAlloc->allocate(iIncomingMsg.contentLength+1));
if (errcode != OsclErrNone)
{
	PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0,"PVRTSPEngineNode::rtspParserLoop() registerEntityBody failed"));

	OSCL_ASSERT(false);//there is no limit for the allocator num of buffers for now
	ChangeExternalState(EPVMFNodeError);
	iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorOutOfMemory;
	ReportErrorEvent(PVMFErrProcessing, NULL, &iEventUUID, &iCurrentErrorCode);

	bLoop = false;
	ret = true;
	break;
}

media_data_imp->getMediaFragment(0, iEntityMemFrag);
(iEntityMemFrag.getMemFrag()).len = iEntityMemFrag.getCapacity();
*/
                if ((iEntityMemFrag.len > 0) || (iEntityMemFrag.ptr != NULL))
                {
                    OSCL_FREE(iEntityMemFrag.ptr);
                    iEntityMemFrag.len = 0;
                    iEntityMemFrag.ptr = NULL;
                }

                iEntityMemFrag.len = 0;
                iEntityMemFrag.ptr = OSCL_MALLOC(iIncomingMsg.contentLength + 1);
                //PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0,"PVRTSPEngineNode::rtspParserLoop() registerEntityBody failed"));

                OsclError::LeaveIfNull(iEntityMemFrag.ptr);
                iEntityMemFrag.len = (iIncomingMsg.contentLength + 1);

                if (!iRTSPParser->registerEntityBody(&iEntityMemFrag))
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVRTSPEngineNode::rtspParserLoop() registerEntityBody failed"));

                    //iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorOutOfMemory;
                    ChangeExternalState(EPVMFNodeError);
                    iCurrentErrorCode = PVMFRTSPClientEngineNodeErrorRTSPParserError;
                    ReportErrorEvent(PVMFErrProcessing, NULL, &iEventUUID, &iCurrentErrorCode);
                    bLoop = false;
                    ret = true;
                }

                break;
            }

            case RTSPParser::WAITING_FOR_EMBEDDED_DATA_MEMORY:
            {//TCP streaming
                RTSPEntityBody iEmbeddedData;
                if (!PrepareEmbeddedDataMemory(iIncomingMsg.contentLength, iEmbeddedData))
                {//no memory available TBD
                    bLoop = false;
                    break;
                }

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "PVRTSPEngineNode::rtspParserLoop() RTP pkts channelID=%d, len=%d", iIncomingMsg.channelID, iIncomingMsg.contentLength));
                iRTSPParser->registerEmbeddedDataMemory(&iEmbeddedData);
                break;
            }
            case RTSPParser::EMBEDDED_DATA_IS_READY:
            {//process the data
                if (!ibBlockedOnFragGroups)
                {
                    if (! DispatchEmbeddedData(iIncomingMsg.channelID)) //, iEmbeddedData))
                    {//send busy or fails
                        bLoop = false;
                    }
                }
                break;
            }
            case RTSPParser::ERROR_REQUEST_TOO_BIG:
            {
                int32 errcode = PVMFRTSPClientEngineNodeErrorRTSPRequestTooBig;
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVRTSPEngineNode::rtspParserLoop() registerEntityBody failed"));
                ChangeExternalState(EPVMFNodeError);
                ReportErrorEvent(PVMFErrProcessing, NULL, &iEventUUID, &errcode);
                bLoop = false;
                break;
            }
            case RTSPParser::INTERNAL_ERROR:
            default:
            {
                int32 errcode = PVMFRTSPClientEngineNodeErrorRTSPParserError;
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVRTSPEngineNode::rtspParserLoop() registerEntityBody failed"));
                ChangeExternalState(EPVMFNodeError);
                ReportErrorEvent(PVMFErrProcessing, NULL, &iEventUUID, &errcode);
                bLoop = false;
                break;
            }
        }
    }
    return ret;
}



void PVRTSPEngineNode::freechunkavailable(OsclAny*)
{
    ibBlockedOnFragGroups = false;
}

PVMFStatus PVRTSPEngineNode::DoErrorRecovery(PVRTSPEngineCommand &aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::DoReleasePort() In"));

    //get error context
    PVRTSPErrorContext* errorContext = OSCL_STATIC_CAST(PVRTSPErrorContext*, aCmd.iParam1);
    if (errorContext == NULL)
    {
        return PVMFFailure;
    }

    PVMFStatus myRet = PVMFPending;

    {
        if (PVRTSP_ENGINE_NODE_STATE_DESCRIBE_DONE > iState)
        {
            myRet = SendRtspDescribe(aCmd);
        }
        else if (PVRTSP_ENGINE_NODE_STATE_SETUP_DONE > iState)
        {
            myRet = SendRtspSetup(aCmd);
            if (myRet == PVMFSuccess)
            {//send OPTIONS and keep the connection alive
                //and wait for new Start() request
                ChangeInternalState(PVRTSP_ENGINE_NODE_STATE_PAUSE_DONE);
            }
        }
    }

    if ((PVMFSuccess != myRet) && (PVMFPending != myRet))
    {/*error during error handling
	 1. decrease the recovery count counter
	 2. reset the socket
	 3. change the iState to restart again
		*/
        if (iErrorRecoveryAttempt-- <= 0)
        {
            return PVMFFailure;
        }
        //TBD reset the socket
        ChangeInternalState(PVRTSP_ENGINE_NODE_STATE_IDLE);
        RunIfNotReady();
    }
    else if (iState >= errorContext->iErrState)
    {
        return PVMFSuccess;
    }

    if (PVMFSuccess == myRet)
    {
        RunIfNotReady();
    }

    return PVMFPending;
}


//This routine is called repeatedly to drive the socket cleanup sequence.
// Step 1: Cancel DNS & socket operations & wait on completion
// Step 2: Shutdown sockets & wait on completion
// Step 3: Delete sockets
//
// If "Immediate" is set we just delete sockets without shutdown sequence.
PVMFStatus PVRTSPEngineNode::resetSocket(bool aImmediate)
{
    //Make sure things aren't already cleaned up.
    if (!iDNS.IsBusy()
            && !iSendSocket.iSocket
            && !iRecvSocket.iSocket)
    {
        //Nothing to do!
        if (iLogger)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::resetSocket() Nothing to do!"));
        }
        return PVMFSuccess;
    }

    if (aImmediate)
    {
        //force immediate cleanup, for use in destructor where we can't wait
        //on any async ops.

        //see if we have one socket or two.
        bool oneSocket = (iSendSocket.iSocket == iRecvSocket.iSocket);
        if (iSendSocket.iSocket)
        {
            iSendSocket.iSocket->~OsclTCPSocket();
            iAlloc.deallocate(iSendSocket.iSocket);
            iSendSocket.iSocket = NULL;
        }
        if (iRecvSocket.iSocket)
        {
            //guard against duplicate destroy/free
            if (!oneSocket)
            {
                iRecvSocket.iSocket->~OsclTCPSocket();
                iAlloc.deallocate(iRecvSocket.iSocket);
            }
            iRecvSocket.iSocket = NULL;
        }
        return PVMFSuccess;
    }

    bool waitOnAsyncOp = false;
    while (!waitOnAsyncOp)
    {
        switch (iSocketCleanupState)
        {
            case ESocketCleanup_Idle:
                //Start a new sequence.
                iSocketCleanupState = ESocketCleanup_CancelCurrentOp;
                break;

            case ESocketCleanup_CancelCurrentOp:
                //Step 1: Cancel current operations

                //Cancel ops on DNS
                if (iDNS.iDns)
                {
                    if (iDNS.iState.iPending
                            && !iDNS.iState.iCanceled)
                    {
                        iDNS.iDns->CancelGetHostByName();
                        iDNS.iState.iCanceled = true;
                        if (iLogger)
                        {
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::resetSocket() Cancel GetHostByName"));
                        }
                    }
                }

                //Cancel ops on send socket.
                if (iSendSocket.iSocket)
                {
                    if (iSendSocket.iConnectState.iPending
                            && !iSendSocket.iConnectState.iCanceled)
                    {
                        iSendSocket.iSocket->CancelConnect();
                        iSendSocket.iConnectState.iCanceled = true;
                        if (iLogger)
                        {
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::resetSocket() Cancel Connect"));
                        }
                    }
                    if (iSendSocket.iSendState.iPending
                            && !iSendSocket.iSendState.iCanceled)
                    {
                        iSendSocket.iSocket->CancelSend();
                        iSendSocket.iSendState.iCanceled = true;
                        if (iLogger)
                        {
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::resetSocket() Cancel Send"));
                        }
                    }
                    if (iSendSocket.iRecvState.iPending
                            && !iSendSocket.iRecvState.iCanceled)
                    {
                        iSendSocket.iSocket->CancelRecv();
                        iSendSocket.iRecvState.iCanceled = true;
                        if (iLogger)
                        {
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::resetSocket() Cancel Recv"));
                        }
                    }
                    OSCL_ASSERT(!iSendSocket.iShutdownState.iPending);
                }

                //Cancel ops on recv socket only when it's
                //a unique socket.
                if (iRecvSocket.iSocket
                        && iRecvSocket.iSocket != iSendSocket.iSocket)
                {
                    if (iRecvSocket.iConnectState.iPending
                            && !iRecvSocket.iConnectState.iCanceled)
                    {
                        iRecvSocket.iSocket->CancelConnect();
                        iRecvSocket.iConnectState.iCanceled = true;
                        if (iLogger)
                        {
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::resetSocket() Cancel Connect (recv sock)"));
                        }
                    }
                    if (iRecvSocket.iSendState.iPending
                            && !iRecvSocket.iSendState.iCanceled)
                    {
                        iRecvSocket.iSocket->CancelSend();
                        iRecvSocket.iSendState.iCanceled = true;
                        if (iLogger)
                        {
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::resetSocket() Cancel Send (recv sock)"));
                        }
                    }
                    if (iRecvSocket.iRecvState.iPending
                            && !iRecvSocket.iRecvState.iCanceled)
                    {
                        iRecvSocket.iSocket->CancelRecv();
                        iRecvSocket.iRecvState.iCanceled = true;
                        if (iLogger)
                        {
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::resetSocket() Cancel Recv (recv sock)"));
                        }
                    }
                    OSCL_ASSERT(!iRecvSocket.iShutdownState.iPending);
                }

                if (iDNS.IsBusy()
                        || iSendSocket.IsBusy()
                        || iRecvSocket.IsBusy())
                {
                    waitOnAsyncOp = true;
                }

                //Either go to wait state or continue to Step 2.
                if (waitOnAsyncOp)
                    iSocketCleanupState = ESocketCleanup_WaitOnCancel;
                else
                    iSocketCleanupState = ESocketCleanup_Shutdown;
                break;

            case ESocketCleanup_WaitOnCancel:
                //Wait on cancel completion for all.

                if (iDNS.IsBusy()
                        || iSendSocket.IsBusy()
                        || iRecvSocket.IsBusy())
                {
                    waitOnAsyncOp = true;
                }

                if (!waitOnAsyncOp)
                    iSocketCleanupState = ESocketCleanup_Shutdown;
                break;

            case ESocketCleanup_Shutdown:
                //Step 2: shutdown both sockets

                if (iDNS.iDns)
                {
                    OSCL_ASSERT(!iDNS.IsBusy());
                }

                if (iSendSocket.iSocket)
                {
                    OSCL_ASSERT(!iSendSocket.IsBusy());

                    if (iSendSocket.iSocket->Shutdown(EPVSocketBothShutdown, TIMEOUT_SHUTDOWN) == EPVSocketPending)
                    {
                        iSendSocket.iShutdownState.iPending = true;
                        waitOnAsyncOp = true;
                        if (iLogger)
                        {
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::resetSocket() Shutdown"));
                        }
                    }
                    //else shutdown failed, ignore & continue
                    else
                    {
                        if (iLogger)
                        {
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::resetSocket() Shutdown Failed"));
                        }
                    }
                }

                //shutown recv socket only when it's a unique socket.
                if (iRecvSocket.iSocket
                        && iRecvSocket.iSocket != iSendSocket.iSocket)
                {
                    OSCL_ASSERT(!iRecvSocket.IsBusy());

                    if (iRecvSocket.iSocket->Shutdown(EPVSocketBothShutdown, TIMEOUT_SHUTDOWN) == EPVSocketPending)
                    {
                        iRecvSocket.iShutdownState.iPending = true;
                        waitOnAsyncOp = true;
                        if (iLogger)
                        {
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::resetSocket() Shutdown (recv sock)"));
                        }
                    }
                    //else shutdown failed, ignore & continue
                    else
                    {
                        if (iLogger)
                        {
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::resetSocket() Shutdown Failed (recv sock)"));
                        }
                    }
                }

                //Either go to wait state or continue to Step 3.
                if (waitOnAsyncOp)
                    iSocketCleanupState = ESocketCleanup_WaitOnShutdown;
                else
                    iSocketCleanupState = ESocketCleanup_Delete;
                break;

            case ESocketCleanup_WaitOnShutdown:
                //Wait on shutdown completion for both sockets.

                if (iSendSocket.IsBusy()
                        || iRecvSocket.IsBusy())
                {
                    waitOnAsyncOp = true;
                }

                if (!waitOnAsyncOp)
                    iSocketCleanupState = ESocketCleanup_Delete;
                break;

            case ESocketCleanup_Delete:
                // Step 3: Delete sockets

                // Note: we assume this calling context is not the socket callback, so
                // it's safe to delete the OsclSocket object here.

            {
                //see if we have one socket or two.
                bool oneSocket = (iSendSocket.iSocket == iRecvSocket.iSocket);
                if (iSendSocket.iSocket)
                {
                    OSCL_ASSERT(!iSendSocket.IsBusy());
                    if (iLogger)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::resetSocket() Delete TCP"));
                    }
                    iSendSocket.iSocket->~OsclTCPSocket();
                    iAlloc.deallocate(iSendSocket.iSocket);
                    iSendSocket.iSocket = NULL;
                }
                if (iRecvSocket.iSocket)
                {
                    OSCL_ASSERT(!iRecvSocket.IsBusy());
                    //guard against duplicate destroy/free
                    if (!oneSocket)
                    {
                        if (iLogger)
                        {
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::resetSocket() Delete TCP (recv sock)"));
                        }
                        iRecvSocket.iSocket->~OsclTCPSocket();
                        iAlloc.deallocate(iRecvSocket.iSocket);
                    }
                    iRecvSocket.iSocket = NULL;
                }

                bNoSendPending = bNoRecvPending = false;

                ChangeInternalState(PVRTSP_ENGINE_NODE_STATE_IDLE);
            }
            iSocketCleanupState = ESocketCleanup_Idle;
            if (iLogger)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::resetSocket() Done!"));
            }
            return PVMFSuccess; //Done!

            default:
                OSCL_ASSERT(0);
                break;
        }
    }

    return (waitOnAsyncOp) ? PVMFPending : PVMFSuccess;
}

void PVRTSPEngineNode::SetSendPending(SocketContainer& aSock)
{
    //Set "send pending" condition on a socket container.
    //update recv socket container only when we have a unique recv socket,
    //otherwise update send socket container.
    if (aSock.iSocket == iRecvSocket.iSocket
            && iRecvSocket.iSocket != iSendSocket.iSocket)
        iRecvSocket.iSendState.iPending = true;
    else
    {
        iSendSocket.iSendState.iPending = true;
        OSCL_ASSERT(aSock.iSocket == iSendSocket.iSocket);
    }
}

void PVRTSPEngineNode::SetRecvPending(SocketContainer& aSock)
{
    //Set "recv pending" condition on a socket container.
    //update recv socket container only when we have a unique recv socket,
    //otherwise update send socket container.
    if (aSock.iSocket == iRecvSocket.iSocket
            && iRecvSocket.iSocket != iSendSocket.iSocket)
        iRecvSocket.iRecvState.iPending = true;
    else
    {
        iSendSocket.iRecvState.iPending = true;
        OSCL_ASSERT(aSock.iSocket == iSendSocket.iSocket);
    }
}

OsclLeaveCode PVRTSPEngineNode::RunError(OsclLeaveCode aError)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVRTSPEngineNode::RunError() aError=%d", aError));


    //return OsclErrNone;
    return aError;
}

void PVRTSPEngineNode::clearOutgoingMsgQueue()
{
    while (!iOutgoingMsgQueue.empty())
    {
        //pop the outgoing msg queue and see the match
        RTSPOutgoingMessage* tmpOutgoingMsg = iOutgoingMsgQueue.top();
        iOutgoingMsgQueue.pop();
        if (tmpOutgoingMsg)
            OSCL_DELETE(tmpOutgoingMsg);
    }
}

void PVRTSPEngineNode::partialResetSessionInfo()
{
    iOutgoingSeq = 0;
    setupTrackIndex = 0;
    iSessionInfo.iSID = "";

    iSessionInfo.iReqPlayRange.format = RtspRangeType::INVALID_RANGE;
    iSessionInfo.bExternalSDP = false;
    iSessionInfo.pvServerIsSetFlag = false;
    iSessionInfo.roundTripDelay = 0;
    iSessionInfo.iSDPinfo.Unbind();
}

void PVRTSPEngineNode::ResetSessionInfo()
{
    iOutgoingSeq = 0;
    setupTrackIndex = 0;
    iSessionInfo.iSID = "";

    iSessionInfo.iReqPlayRange.format = RtspRangeType::INVALID_RANGE;
    iSessionInfo.bExternalSDP = false;
    iSessionInfo.pvServerIsSetFlag = false;
    iSessionInfo.roundTripDelay = 0;
    iSessionInfo.iSDPinfo.Unbind();
}

//clear the internal event queue "iSocketEventQueue"
//return false if any of the event indicate failure
bool PVRTSPEngineNode::clearEventQueue(void)
{
    bool myRet = true;
    while (!iSocketEventQueue.empty())
    {
        SocketEvent tmpSockEvent(iSocketEventQueue.front());
        iSocketEventQueue.erase(&iSocketEventQueue.front());
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO, (0, "PVRTSPEngineNode::clearEventQueue() In iFxn=%d iSockEvent=%d iSockError=%d ", tmpSockEvent.iSockFxn, tmpSockEvent.iSockEvent, tmpSockEvent.iSockError));

        if (tmpSockEvent.iSockEvent != EPVSocketSuccess)
        {
            myRet = false;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVRTSPEngineNode::clearEventQueue() Socket Error"));
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO, (0, "PVRTSPEngineNode::clearEventQueue() Out myRet=%d ", myRet));
    return myRet;
}

////////////////////////////////////////////////////////////////////////////////
/// GetPostCorrelationObject Implementation
////////////////////////////////////////////////////////////////////////////////
#define KGetPostCorrelationFile	_STRLIT("get_post_correlation_number.dat")

GetPostCorrelationObject* GetPostCorrelationObject::create(OSCL_TCHAR* aFileName)
{
    GetPostCorrelationObject *object = OSCL_NEW(GetPostCorrelationObject, ());
    if (!object) return NULL;
    if (!object->construct(aFileName))
    {
        OSCL_DELETE(object);
        return NULL;
    }
    return object;
}

bool GetPostCorrelationObject::construct(OSCL_TCHAR* aFileName)
{
    iGetPostCorrelation = 1;
    iFileCreated = false;
    iFs.Connect();

    OSCL_TCHAR* aLocalFileName = aFileName;
    if (!aLocalFileName) aLocalFileName = (OSCL_TCHAR*)KGetPostCorrelationFile;
    if (iGetPostCorrelationFile.Open(aLocalFileName, Oscl_File::MODE_READ | Oscl_File::MODE_BINARY, iFs) == 0)
    {
        // get the current iGetPostCorrelation
        iGetPostCorrelationFile.Read(&iGetPostCorrelation, sizeof(iGetPostCorrelation), 1);
        iGetPostCorrelationFile.Close();
    }

    if (iGetPostCorrelationFile.Open(aLocalFileName, Oscl_File::MODE_READWRITE | Oscl_File::MODE_BINARY, iFs))
    {
        iFs.Close();
        return false;
    }

    iFileCreated = true;
    return true;
}

GetPostCorrelationObject::~GetPostCorrelationObject()
{
    if (iFileCreated) closeFile();
    iFileCreated = false;
}

void GetPostCorrelationObject::closeFile()
{
    writeToFile();
    iGetPostCorrelationFile.Close();
    iFs.Close();
}


bool GetPostCorrelationObject::writeToFile()
{
    // always write at the beginning
    if (iGetPostCorrelationFile.Seek(0, Oscl_File::SEEKSET)) return false;
    if (iGetPostCorrelationFile.Write(&iGetPostCorrelation, sizeof(iGetPostCorrelation), 1) != 1)
    {
        return false;
    }

    // Flush the data to file
    iGetPostCorrelationFile.Flush();
    return true;
}

bool GetPostCorrelationObject::update()
{
    if (!iFileCreated) return false;

    // increase iGetPostCorrelation by 1 within [1, 255]
    if (iGetPostCorrelation == 255) iGetPostCorrelation = 1;
    else ++iGetPostCorrelation;

    return writeToFile();
}




/**
 * Called by the command handler AO to do the Cancel All
 */
PVMFStatus PVRTSPEngineNode::DoCancelAllCommands(PVRTSPEngineCommand& aCmd)
{
    /* cancel all queued commands */
    while (!iPendingCmdQueue.empty())
        CommandComplete(iPendingCmdQueue, iPendingCmdQueue[1], PVMFErrCancelled);

    if (iRunningCmdQueue.size() > 1)
    {
        MoveCmdToCancelQueue(aCmd);
        return PVMFPending;
    }

    return PVMFSuccess;
}

void PVRTSPEngineNode::MoveCmdToCancelQueue(PVRTSPEngineCommand& aCmd)
{
    /*
     * note: the StoreL cannot fail since the queue is never more than 1 deep
     * and we reserved space.
     */
    iCancelCmdQueue.StoreL(aCmd);
    iRunningCmdQueue.Erase(&aCmd);
}


OsclSharedPtr<PVMFMediaDataImpl> PVRTSPEngineNode::AllocateMediaData(int32& errCode)
{
    OsclSharedPtr<PVMFMediaDataImpl> mediaDataImplOut;
    OSCL_TRY(errCode, mediaDataImplOut = ipFragGroupAllocator->allocate());
    return mediaDataImplOut;
}
