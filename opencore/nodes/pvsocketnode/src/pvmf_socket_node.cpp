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

#include "pvmf_socket_node.h"
#include "oscl_string_utils.h"
#include "pvmf_basic_errorinfomessage.h"
#include "pvmf_errorinfomessage_extension.h"
#include "pvmf_socket_node_events.h"
#include "oscl_byte_order.h"
#include "pvmf_protocol_engine_command_format_ids.h"
#include "pvmf_media_cmd.h"
#include "pvmf_media_msg_format_ids.h"
#include "oscl_tickcount.h"
#include "oscl_rand.h"
#include "oscl_time.h"
#include "pvmf_socket_node_tunables.h"
#include "oscl_bin_stream.h"
// Use default DLL entry point for Symbian
#include "oscl_dll.h"

//////////////////////////////////////////////////
// PVMFSocketNodeMemPool
//////////////////////////////////////////////////

PVMFSocketNodeMemPool::PVMFSocketNodeMemPool(uint32 aMemPoolNumBufs)
        : iMediaDataMemPool(NULL)
{
    iMediaDataMemPool = OSCL_NEW(OsclMemPoolFixedChunkAllocator, (aMemPoolNumBufs, MEDIA_DATA_CLASS_SIZE));
    iInternalAlloc = NULL;
    iSharedBufferAlloc = NULL;
    iMediaMsgAllocator = NULL;
    iMediaFragGroupAlloc = NULL;
    iMediaFragGroupAllocMempool = NULL;
}

void PVMFSocketNodeMemPool::CreateAllocators(const OSCL_HeapString<OsclMemAllocator>& iMime, uint32 aSize, uint32 aExpectedNumberOfBlocksPerBuffer, uint32 aResizeSize, uint32 aMaxNumResizes)
{
    if (!iSharedBufferAlloc)
    {
        iSharedDataBufferInfo.Init(aSize, aExpectedNumberOfBlocksPerBuffer, aResizeSize, aMaxNumResizes);
        const uint32 maxAllocatorNameLen = 255;
        char allocatorName[maxAllocatorNameLen];
        const uint32 strLenForMimeName  = maxAllocatorNameLen - oscl_strlen("InternalSocketBuffer");
        oscl_strncpy(allocatorName, iMime.get_cstr(), strLenForMimeName);
        allocatorName[strLenForMimeName] = '\0';
        oscl_strcat(allocatorName, "InternalSocketBuffer");
        CreateDefaultDataBufferAllocator(allocatorName);
    }
#if SNODE_ENABLE_UDP_MULTI_PACKET
    CreateUDPMultipleRecvAllocator();
#endif
}

OsclMemPoolResizableAllocator* PVMFSocketNodeMemPool::CreateResizableDataBufferAllocator(const char* allocatorName)
{
    OSCL_UNUSED_ARG(allocatorName);
    if (iMediaMsgAllocator)
    {
        /*
         * It may be possible that the allocator was binded to iInternalAlloc/iSharedBufferAlloc
         * So destruct it.
         * Deallocation of iInternalAlloc/iSharedBufferAlloc will be done as part of Reset/Destruction of node
         */
        OSCL_DELETE(iMediaMsgAllocator);
        iMediaMsgAllocator = NULL;
    }

    OSCL_ASSERT(iSharedBufferAlloc == NULL);
    if (iSharedBufferAlloc)
        return NULL;


    iSharedBufferAlloc = OSCL_NEW(OsclMemPoolResizableAllocator, (iSharedDataBufferInfo.iSize, (1 + iSharedDataBufferInfo.iMaxNumResizes),
                                  iSharedDataBufferInfo.iExpectedNumberOfBlocksPerBuffer));
    iSharedBufferAlloc->setMaxSzForNewMemPoolBuffer(iSharedDataBufferInfo.iResizeSize);
    iMediaMsgAllocator = OSCL_NEW(PVMFResizableSimpleMediaMsgAlloc, (iSharedBufferAlloc));
    return iSharedBufferAlloc;
}

void PVMFSocketNodeMemPool::CreateDefaultDataBufferAllocator(const char* allocatorName)
{
    OSCL_UNUSED_ARG(allocatorName);
    if (iMediaMsgAllocator)
    {
        /*
         * It may be possible that the allocator was binded to iInternalAlloc/iSharedBufferAlloc
         * So destruct it.
         * Deallocation of iInternalAlloc/iSharedBufferAlloc will be done as part of Reset/Destruction of node
         */
        OSCL_DELETE(iMediaMsgAllocator);
        iMediaMsgAllocator = NULL;
    }

    OSCL_ASSERT(iInternalAlloc == NULL);
    if (iInternalAlloc)
        return;

    iInternalAlloc = OSCL_NEW(OsclMemPoolResizableAllocator, (iSharedDataBufferInfo.iSize, (1 + iSharedDataBufferInfo.iMaxNumResizes),
                              iSharedDataBufferInfo.iExpectedNumberOfBlocksPerBuffer));
    iInternalAlloc->setMaxSzForNewMemPoolBuffer(iSharedDataBufferInfo.iResizeSize);
    iMediaMsgAllocator = OSCL_NEW(PVMFResizableSimpleMediaMsgAlloc, (iInternalAlloc));
}

void PVMFSocketNodeMemPool::CreateUDPMultipleRecvAllocator()
{
#if SNODE_ENABLE_UDP_MULTI_PACKET
    //Create the multiple-receive allocator
    iMediaFragGroupAllocMempool
    = OSCL_NEW(OsclMemPoolFixedChunkAllocator
               , (SNODE_UDP_MULTI_FRAG_ALLOCATOR_MAX_MSGS)
              );
    iMediaFragGroupAlloc
    = OSCL_NEW(PVMFMediaFragGroupCombinedAlloc<PVMFSocketNodeAllocator>
               , (SNODE_UDP_MULTI_FRAG_ALLOCATOR_MAX_MSGS
                  , SNODE_UDP_MULTI_FRAG_ALLOCATOR_MAX_FRAGS_PER_MSG
                  , iMediaFragGroupAllocMempool)
              );
    iMediaFragGroupAlloc->create();
#endif
}

uint32 PVMFSocketNodeMemPool::GetMaxSizeMediaMsgLen()
{
    OsclMemPoolResizableAllocator* resizableAllocator = NULL;

    const uint32 wrappingOverhead = iMediaMsgAllocator->GetMediaMsgAllocationOverheadBytes();

    if (iSharedBufferAlloc)
    {
        resizableAllocator = OSCL_STATIC_CAST(OsclMemPoolResizableAllocator*, iSharedBufferAlloc);
    }
    else if (iInternalAlloc)
    {
        resizableAllocator = OSCL_STATIC_CAST(OsclMemPoolResizableAllocator*, iInternalAlloc);
    }

    if (resizableAllocator)
    {
        return (resizableAllocator->getLargestContiguousFreeBlockSize() - wrappingOverhead);
    }

    return 0;
}

void PVMFSocketNodeMemPool::DestroyAllocators()
{
    if (iInternalAlloc != NULL)
    {
        iInternalAlloc->CancelFreeChunkAvailableCallback();
        iInternalAlloc->removeRef();
    }
    if (iSharedBufferAlloc != NULL)
    {
        iSharedBufferAlloc->CancelFreeChunkAvailableCallback();
        iSharedBufferAlloc->removeRef();
    }
    if (iMediaMsgAllocator)
    {
        OSCL_DELETE(iMediaMsgAllocator);
        iMediaMsgAllocator = NULL;
    }
#if SNODE_ENABLE_UDP_MULTI_PACKET
    DestroyUDPMultipleRecvAllocator();
#endif
}

void PVMFSocketNodeMemPool::DestroyUDPMultipleRecvAllocator()
{
#if SNODE_ENABLE_UDP_MULTI_PACKET
    if (iMediaFragGroupAlloc)
    {
        iMediaFragGroupAlloc->removeRef();
        iMediaFragGroupAlloc = NULL;
    }
    if (iMediaFragGroupAllocMempool)
    {
        iMediaFragGroupAllocMempool->removeRef();
        iMediaFragGroupAllocMempool = NULL;
    }
#endif
}

//////////////////////////////////////////////////
// SocketNodeStats
//////////////////////////////////////////////////

#if(ENABLE_SOCKET_NODE_STATS)
void SocketNodeStats::Log(PVMFPortVector<PVMFSocketPort, PVMFSocketNodeAllocator>& aPortVec)
{
    if (iLogged)
        return;//prevent multiple logging.
    if (!iLogger)
        return;//prevent logging after thread logoff.
    iLogged = true;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iLogger, PVLOGMSG_ERR,
                    (0, "SocketNodeStats:@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iLogger, PVLOGMSG_ERR,
                    (0, "SocketNodeStats: %8d Num Run", iNumRun));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iLogger, PVLOGMSG_ERR,
                    (0, "SocketNodeStats: %8d Num QueueNodeCmd", iNumQueueNodeCmd));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iLogger, PVLOGMSG_ERR,
                    (0, "SocketNodeStats: %8d Num Bind", iNumBind));

    for (uint32 i = 0;i < aPortVec.size();i++)
    {
        if (aPortVec[i]->iConfig)
            aPortVec[i]->iConfig->iPortStats.Log(iLogger, aPortVec[i]->iConfig->iMime);
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iLogger, PVLOGMSG_ERR,
                    (0, "SocketNodeStats:@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"));
}
#endif //ENABLE_SOCKET_NODE_STATS

//////////////////////////////////////////////////
// SocketPortConfig
//////////////////////////////////////////////////

void SocketPortConfig::CreateAllocators(uint32 aSize, uint32 aExpectedNumberOfBlocksPerBuffer, uint32 aResizeSize, uint32 aMaxNumResizes)
{
    iMemPool->CreateAllocators(iMime, aSize, aExpectedNumberOfBlocksPerBuffer, aResizeSize, aMaxNumResizes);
}

void SocketPortConfig::CleanupMemPools(Oscl_DefAlloc& aAlloc)
{
    if (iMemPool)
    {
        iMemPool->DestroyAllocators();
        iMemPool->~PVMFSocketNodeMemPool();
        aAlloc.deallocate((OsclAny*)(iMemPool));
        iMemPool = NULL;
    }
}

//////////////////////////////////////////////////
// End SocketPortConfig
//////////////////////////////////////////////////

//////////////////////////////////////////////////
// Standard Node APIs
//////////////////////////////////////////////////

OSCL_EXPORT_REF PVMFSocketNode::PVMFSocketNode(int32 aPriority)
        : OsclActiveObject(aPriority, "PVMFSocketNode")
        , TIMEOUT_CONNECT(30000)
        , TIMEOUT_SEND(3000)
        , TIMEOUT_SENDTO(3000)
        , TIMEOUT_RECV(-1)
        , TIMEOUT_RECVFROM(-1)
        , TIMEOUT_SHUTDOWN(10000)
        , UDP_PORT_RANGE(2000)
        , MAX_UDP_PACKET_SIZE(MAX_SOCKET_BUFFER_SIZE)
        , MIN_UDP_PACKET_SIZE(MIN_SOCKET_BUFFER_SIZE)
{
    iLogger = NULL;
    iDataPathLogger = NULL;
    iDataPathLoggerRTP = NULL;
    iDataPathLoggerRTCP = NULL;
    iOsclErrorTrapImp = NULL;
    iSockServ = NULL;
    iMaxTcpRecvBufferSize = SNODE_DEFAULT_SOCKET_TCP_BUFFER_SIZE;
    iMaxTcpRecvBufferCount = SNODE_DEFAULT_SOCKET_TCP_BUFFER_COUNT;
    iSocketID = 0;
    iCommandErrorCode = PVMFSocketNodeErrorEventStart;
    iErrorEventErrorCode = PVMFSocketNodeErrorEventStart;
    iExtensionInterface = NULL;
    iInSocketCallback = false;
    iNumStopPortActivityPending = (-1);//inactive.

    int32 err;
    OSCL_TRY(err,

             //Create the input command queue.  Use a reserve to avoid lots of
             //dynamic memory allocation.
             iPendingCmdQueue.Construct(PVMF_SOCKET_NODE_COMMAND_ID_START,
                                        PVMF_SOCKET_NODE_COMMAND_VECTOR_RESERVE);

             //Create the "current command" queue.  It will only contain one
             //command at a time, so use a reserve of 1.
             iCurrentCmdQueue.Construct(0, 1);
             iCancelCmdQueue.Construct(0, 1);

             //Create the port vector.
             iPortVector.Construct(PVMF_SOCKET_NODE_PORT_VECTOR_RESERVE);

             //Set the node capability data.
             //This node can support an unlimited number of ports.
             iCapability.iCanSupportMultipleInputPorts = true;
             iCapability.iCanSupportMultipleOutputPorts = true;
             iCapability.iHasMaxNumberOfPorts = false;
             iCapability.iMaxNumberOfPorts = 0;//no maximum

             iCapability.iInputFormatCapability.push_back(PVMF_MIME_INET_UDP);
             iCapability.iInputFormatCapability.push_back(PVMF_MIME_INET_TCP);

             iCapability.iOutputFormatCapability.push_back(PVMF_MIME_INET_TCP);
             iCapability.iOutputFormatCapability.push_back(PVMF_MIME_INET_UDP);

             iDnsCache.NewL();

            );

    if (err != OsclErrNone)
    {
        //if a leave happened, cleanup and re-throw the error
        iPendingCmdQueue.clear();
        iCurrentCmdQueue.clear();
        iCancelCmdQueue.clear();
        iPortVector.clear();
        iCapability.iInputFormatCapability.clear();
        iCapability.iOutputFormatCapability.clear();
        OSCL_CLEANUP_BASE_CLASS(PVMFNodeInterface);
        OSCL_CLEANUP_BASE_CLASS(OsclActiveObject);
        OSCL_LEAVE(err);
    }
}


OSCL_EXPORT_REF PVMFSocketNode::~PVMFSocketNode()
{
#if(ENABLE_SOCKET_NODE_STATS)
    iSocketNodeStats.Log(iPortVector);
#endif

    Cancel();

    //thread logoff
    if (IsAdded())
        RemoveFromScheduler();

    if (iExtensionInterface)
    {
        iExtensionInterface->removeRef();
    }

    /* Cleanup allocated ports */
    CleanupPorts();
    iPortVector.clear();
    CleanupClosedTCPSockets();
    CleanupClosedUDPSockets();
    CleanupClosedDNS();
    //Cleanup commands
    //The command queues are self-deleting, but we want to
    //notify the observer of unprocessed commands.
    while (!iCurrentCmdQueue.empty())
    {
        CommandComplete(iCurrentCmdQueue, iCurrentCmdQueue.front(), PVMFFailure);
    }
    while (!iPendingCmdQueue.empty())
    {
        CommandComplete(iPendingCmdQueue, iPendingCmdQueue.front(), PVMFFailure);
    }
    while (!iCancelCmdQueue.empty())
    {
        CommandComplete(iCancelCmdQueue, iCancelCmdQueue.front(), PVMFFailure);
    }
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "Goin to delete Sock Serv"));
    if (iSockServ)
    {
        iSockServ->Close();
        iSockServ->~OsclSocketServ();
        iAlloc.deallocate(iSockServ);
        iSockServ = NULL;
    }
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFSocketNode:~PVMFSocketNode out"));
}


//Do thread-specific node creation and go to "Idle" state.
OSCL_EXPORT_REF PVMFStatus PVMFSocketNode::ThreadLogon()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFSocketNode:ThreadLogon"));

    switch (iInterfaceState)
    {
        case EPVMFNodeCreated:
            if (!IsAdded())
                AddToScheduler();
            iLogger = PVLogger::GetLoggerObject("PVMFSocketNode");
            iDataPathLogger = PVLogger::GetLoggerObject("datapath.socketnode");
            iDataPathLoggerRTP = PVLogger::GetLoggerObject("datapath.socketnode.rtp");
            iDataPathLoggerRTCP = PVLogger::GetLoggerObject("datapath.socketnode.rtcp");
            iOsclErrorTrapImp = OsclErrorTrap::GetErrorTrapImp();
            SetState(EPVMFNodeIdle);
            return PVMFSuccess;
        default:
            return PVMFErrInvalidState;
    }
}

//Do thread-specific node cleanup and go to "Created" state.
OSCL_EXPORT_REF PVMFStatus PVMFSocketNode::ThreadLogoff()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFSocketNode:ThreadLogoff"));

    switch (iInterfaceState)
    {
        case EPVMFNodeIdle:
        {
            if (IsAdded())
                RemoveFromScheduler();
            iLogger = NULL;
            iDataPathLogger = NULL;
            iDataPathLoggerRTP = NULL;
            iDataPathLoggerRTCP = NULL;
            iOsclErrorTrapImp = NULL;
            SetState(EPVMFNodeCreated);
#if(ENABLE_SOCKET_NODE_STATS)
            iSocketNodeStats.Logoff();
#endif
            return PVMFSuccess;
        }
        // break;	This break statement was removed to avoid compiler warning for Unreachable Code

        default:
            return PVMFErrInvalidState;
            // break;	This break statement was removed to avoid compiler warning for Unreachable Code
    }
}

//retrieve node capabilities.
OSCL_EXPORT_REF PVMFStatus PVMFSocketNode::GetCapability(PVMFNodeCapability& aNodeCapability)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFSocketNode:GetCapability"));
    aNodeCapability = iCapability;
    return PVMFSuccess;
}

//retrieve a port iterator.
OSCL_EXPORT_REF PVMFPortIter* PVMFSocketNode::GetPorts(const PVMFPortFilter* aFilter)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFSocketNode:GetPorts"));

    OSCL_UNUSED_ARG(aFilter);//port filter is not implemented.
    iPortVector.Reset();
    return &iPortVector;
}

OSCL_EXPORT_REF PVMFCommandId PVMFSocketNode::QueryUUID(PVMFSessionId s, const PvmfMimeString& aMimeType,
        Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids,
        bool aExactUuidsOnly,
        const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFSocketNode:QueryUUID"));

    PVMFSocketNodeCommand cmd;
    cmd.PVMFSocketNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_QUERYUUID, aMimeType, aUuids, aExactUuidsOnly, aContext);
    return QueueCommandL(cmd);
}

OSCL_EXPORT_REF PVMFCommandId PVMFSocketNode::QueryInterface(PVMFSessionId s, const PVUuid& aUuid,
        PVInterface*& aInterfacePtr,
        const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFSocketNode:QueryInterface"));

    PVMFSocketNodeCommand cmd;
    cmd.PVMFSocketNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_QUERYINTERFACE, aUuid, aInterfacePtr, aContext);
    return QueueCommandL(cmd);
}

OSCL_EXPORT_REF PVMFCommandId PVMFSocketNode::RequestPort(PVMFSessionId s, int32 aPortTag, const PvmfMimeString* aPortConfig, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFSocketNode:RequestPort"));
    PVMFSocketNodeCommand cmd;
    cmd.PVMFSocketNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_REQUESTPORT, aPortTag, aPortConfig, aContext);
    return QueueCommandL(cmd);
}

OSCL_EXPORT_REF PVMFCommandId PVMFSocketNode::ReleasePort(PVMFSessionId s, PVMFPortInterface& aPort, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFSocketNode:ReleasePort"));
    PVMFSocketNodeCommand cmd;
    cmd.PVMFSocketNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_RELEASEPORT, aPort, aContext);
    return QueueCommandL(cmd);
}

OSCL_EXPORT_REF PVMFCommandId PVMFSocketNode::Init(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFSocketNode:Init"));
    PVMFSocketNodeCommand cmd;
    cmd.PVMFSocketNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_INIT, aContext);
    return QueueCommandL(cmd);
}

OSCL_EXPORT_REF PVMFCommandId PVMFSocketNode::Prepare(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFSocketNode:Init"));
    PVMFSocketNodeCommand cmd;
    cmd.PVMFSocketNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_PREPARE, aContext);
    return QueueCommandL(cmd);
}

OSCL_EXPORT_REF PVMFCommandId PVMFSocketNode::Start(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFSocketNode:Start"));
    PVMFSocketNodeCommand cmd;
    cmd.PVMFSocketNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_START, aContext);
    return QueueCommandL(cmd);
}

OSCL_EXPORT_REF PVMFCommandId PVMFSocketNode::Stop(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFSocketNode:Stop"));
    PVMFSocketNodeCommand cmd;
    cmd.PVMFSocketNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_STOP, aContext);
    return QueueCommandL(cmd);
}

OSCL_EXPORT_REF PVMFCommandId PVMFSocketNode::Flush(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFSocketNode:Flush"));
    PVMFSocketNodeCommand cmd;
    cmd.PVMFSocketNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_FLUSH, aContext);
    return QueueCommandL(cmd);
}

OSCL_EXPORT_REF PVMFCommandId PVMFSocketNode::Pause(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFSocketNode:Pause"));
    PVMFSocketNodeCommand cmd;
    cmd.PVMFSocketNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_PAUSE, aContext);
    return QueueCommandL(cmd);
}

OSCL_EXPORT_REF PVMFCommandId PVMFSocketNode::Reset(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFSocketNode:Reset"));
    PVMFSocketNodeCommand cmd;
    cmd.PVMFSocketNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_RESET, aContext);
    return QueueCommandL(cmd);
}

OSCL_EXPORT_REF PVMFCommandId PVMFSocketNode::CancelAllCommands(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFSocketNode:CancelAllCommands"));
    PVMFSocketNodeCommand cmd;
    cmd.PVMFSocketNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_CANCELALLCOMMANDS, aContext);
    return QueueCommandL(cmd);
}

OSCL_EXPORT_REF PVMFCommandId PVMFSocketNode::CancelCommand(PVMFSessionId s, PVMFCommandId aCmdId, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFSocketNode:CancelCommand"));
    PVMFSocketNodeCommand cmd;
    cmd.PVMFSocketNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_CANCELCOMMAND, aCmdId, aContext);
    return QueueCommandL(cmd);
}

//Port activity handler
void PVMFSocketNode::HandlePortActivity(const PVMFPortActivity &aActivity)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "0x%x PVMFSocketNode::HandlePortActivity IN: port=0x%x, type=%d",
                     this, aActivity.iPort, aActivity.iType));

    switch (aActivity.iType)
    {

        case PVMF_PORT_ACTIVITY_CONNECT:
        {
            PVMFSocketPort* sockPort = OSCL_STATIC_CAST(PVMFSocketPort*, aActivity.iPort);
            OSCL_ASSERT(sockPort && sockPort->iConfig);
            SocketPortConfig& aSockConfig = *sockPort->iConfig;
            aSockConfig.CreateAllocators(iMaxTcpRecvBufferSize * (iMaxTcpRecvBufferCount - 1), iMaxTcpRecvBufferCount - 1, iMaxTcpRecvBufferSize, 1);
#if(ENABLE_SOCKET_NODE_STATS)
            aSockConfig.iPortStats.iNumPortEventConnect++;
#endif

            //Receives may have been blocked waiting on the port to be connected, so check here.
            //Note: This is a known use case for HTTP streaming, where the port request and connect
            //can happen after the node start
            if (CanReceive(aSockConfig))
                StartRecvOperation(aSockConfig);
        }
        break;

        case PVMF_PORT_ACTIVITY_INCOMING_MSG:
        {
            PVMFSocketPort* sockPort = (PVMFSocketPort*)(aActivity.iPort);
            OSCL_ASSERT(sockPort && sockPort->iConfig);
            SocketPortConfig& aSockConfig = *sockPort->iConfig;

#if(ENABLE_SOCKET_NODE_STATS)
            aSockConfig.iPortStats.iNumPortEventIncomingMsg++;
#endif

            //Try to process this message now.
            if (CanProcessIncomingMsg(aSockConfig))
                ProcessIncomingMsg(aSockConfig);
            //Otherwise, ignore this event now.  Other code will check
            //the input queue as needed.
        }
        break;

        case PVMF_PORT_ACTIVITY_CONNECTED_PORT_READY:
        {
            PVMFSocketPort* sockPort = (PVMFSocketPort*)(aActivity.iPort);
            OSCL_ASSERT(sockPort && sockPort->iConfig);
            SocketPortConfig& aSockConfig = *sockPort->iConfig;

#if(ENABLE_SOCKET_NODE_STATS)
            aSockConfig.iPortStats.iNumPortEventConnectedPortReady++;
#endif

            //See if the port was waiting on this event, and if so, continue processing.

            if (aSockConfig.iState.iConnectOperation == EPVSocketPortConnectOperation_WaitOnConnectedPort)
            {
#if(ENABLE_SOCKET_NODE_STATS)
                aSockConfig.iPortStats.EndConnectTime(aSockConfig.iState.iConnectOperation);
#endif
                ConnectOperationComplete(aSockConfig, PVMFSuccess, NULL);
            }

            if (aSockConfig.iState.iRecvOperation == EPVSocketPortRecvOperation_WaitOnConnectedPort)
            {
#if(ENABLE_SOCKET_NODE_STATS)
                aSockConfig.iPortStats.EndRecvTime(aSockConfig.iState.iRecvOperation);
#endif
                RecvOperationComplete(aSockConfig, PVMFSuccess, NULL);
            }

            //Otherwise ignore this event now.  Other code will check connected
            //port status as needed.
        }
        break;

        default:
            //all other events can be ignored.
            break;
    }
}

//////////////////////////////////////////////////
// End Standard Node APIs
//////////////////////////////////////////////////

//////////////////////////////////////////////////
// Additional Public APIs unique to Socket Node
//////////////////////////////////////////////////

OSCL_EXPORT_REF bool PVMFSocketNode::GetPortConfig(PVMFPortInterface &aPort, OsclNetworkAddress &aLocalAdd, OsclNetworkAddress &aRemoteAdd)
{
    PVMF_SOCKETNODE_LOGSTACKTRACE((0, "PVMFSocketNode::GetPortConfig IN"));
    aLocalAdd.ipAddr.Set("0.0.0.0");
    aLocalAdd.port = 0;

    aRemoteAdd.ipAddr.Set("");
    aRemoteAdd.port = 0;

    for (uint32 i = 0; i < iPortVector.size(); i++)
    {
        if (iPortVector[i]->iConfig
                && iPortVector[i]->iConfig->iPVMFPort == &aPort)
        {
            aLocalAdd = iPortVector[i]->iConfig->iAddr.iLocalAdd;
            aRemoteAdd = iPortVector[i]->iConfig->iAddr.iRemoteAdd;
            return true;
        }
    }
    return false;
}

OSCL_EXPORT_REF bool PVMFSocketNode::SetPortConfig(PVMFPortInterface &aPort, OsclNetworkAddress aLocalAdd, OsclNetworkAddress aRemoteAdd)
{
    PVMF_SOCKETNODE_LOGSTACKTRACE((0, "PVMFSocketNode::SetPortConfig IN"));
    for (uint32 i = 0; i < iPortVector.size(); i++)
    {
        if (iPortVector[i]->iConfig
                && iPortVector[i]->iConfig->iPVMFPort == &aPort)
        {
            iPortVector[i]->iConfig->iAddr.iLocalAdd = aLocalAdd;
            iPortVector[i]->iConfig->iAddr.iRemoteAdd = aRemoteAdd;
            return true;
        }
    }
    return false;
}

//////////////////////////////////////////////////
// End Additional Public APIs unique to Socket Node
//////////////////////////////////////////////////


//Create UDP sockets on consecutive ports.
//This is needed because 3GPP streaming servers require consecutive ports.
//If successfully created, the sockets are saved in the "allocated port" vector.
//The created sockets will be associated with PVMF ports later during RequestPort
//commands.
PVMFStatus PVMFSocketNode::AllocateConsecutivePorts(PvmfMimeString* aPortConfig,
        uint32& aLowerPortNum,
        uint32& aHigherPortNum, uint32& aStartPortNum)
{
    PVMF_SOCKETNODE_LOGSTACKTRACE((0, "PVMFSocketNode::AllocateConsecutivePorts IN"));

    //used in case of 3GPP streaming to allocate consecutive udp ports
    //for rtp and rtcp.
    //lower port number = rtp => PVMF_SOCKET_NODE_PORT_TYPE_SOURCE
    //higher port number = rtcp => PVMF_SOCKET_NODE_PORT_TYPE_SINK
    aLowerPortNum = 0;
    aHigherPortNum = 0;

    uint32 startSockID = iSocketID;

    PVMFStatus status = PVMFFailure;

    SocketPortConfig* lower_sock_config = OSCL_NEW(SocketPortConfig, ());
    SocketPortConfig* higher_sock_config = OSCL_NEW(SocketPortConfig, ());

    for (int maxNumOfBind = UDP_PORT_RANGE;maxNumOfBind >= 0;maxNumOfBind--)
    {
        OSCL_HeapString<OsclMemAllocator> rtpportConfigWithMime;
        rtpportConfigWithMime += aPortConfig->get_cstr();
        rtpportConfigWithMime += _STRLIT_CHAR("/rtp");
        ParseTransportConfig(&rtpportConfigWithMime,
                             lower_sock_config->iAddr,
                             lower_sock_config->iMime);
        //this should only be UDP
        if (lower_sock_config->iAddr.iProtocol != INET_UDP)
        {
            status = PVMFErrArgument;
            goto Exit;
        }
        lower_sock_config->iContainer = this;
        lower_sock_config->iAddr.iLocalAdd.port = aStartPortNum;
        lower_sock_config->iSockId = startSockID++;
        lower_sock_config->iRTP = true;

        if (NULL == (lower_sock_config->iUDPSocket = (OsclUDPSocket*)CreateOsclSocketAndBind(lower_sock_config->iAddr, lower_sock_config->iSockId)))
        {
            status = PVMFFailure;
            goto Exit;
        }

        OSCL_HeapString<OsclMemAllocator> rtcpportConfigWithMime;
        rtcpportConfigWithMime += aPortConfig->get_cstr();
        rtcpportConfigWithMime += _STRLIT_CHAR("/rtcp");

        ParseTransportConfig(&rtcpportConfigWithMime,
                             higher_sock_config->iAddr,
                             higher_sock_config->iMime);
        //this should only be UDP
        if (higher_sock_config->iAddr.iProtocol != INET_UDP)
        {
            status = PVMFErrArgument;
            goto Exit;
        }
        higher_sock_config->iContainer = this;
        higher_sock_config->iAddr.iLocalAdd.port =
            lower_sock_config->iAddr.iLocalAdd.port + 1;
        higher_sock_config->iSockId = startSockID++;
        higher_sock_config->iRTCP = true;

        if (NULL == (higher_sock_config->iUDPSocket = (OsclUDPSocket*)CreateOsclSocketAndBind(higher_sock_config->iAddr, higher_sock_config->iSockId)))
        {
            status = PVMFFailure;
            goto Exit;
        }

        //We were able to create and bind, now see whether we got consecutive ports or not.
        if (higher_sock_config->iAddr.iLocalAdd.port ==
                (lower_sock_config->iAddr.iLocalAdd.port + 1))
        {
            aLowerPortNum = lower_sock_config->iAddr.iLocalAdd.port;
            lower_sock_config->iPVMFPort = NULL;
            lower_sock_config->iTag = PVMF_SOCKET_NODE_PORT_TYPE_SOURCE;
            iAllocatedPortVector.push_back(lower_sock_config);

            aHigherPortNum = higher_sock_config->iAddr.iLocalAdd.port;
            higher_sock_config->iPVMFPort = NULL;
            higher_sock_config->iTag = PVMF_SOCKET_NODE_PORT_TYPE_SINK;
            iAllocatedPortVector.push_back(higher_sock_config);

            iSocketID = startSockID;

            aStartPortNum = higher_sock_config->iAddr.iLocalAdd.port + 1;

            status = PVMFSuccess;
            goto Exit;
        }
        else
        {
            //Delete and try again
            OsclUDPSocket* udpSocket1 = lower_sock_config->iUDPSocket;
            udpSocket1->~OsclUDPSocket();
            iAlloc.deallocate(udpSocket1);
            lower_sock_config->iUDPSocket = NULL;

            OsclUDPSocket* udpSocket2 = higher_sock_config->iUDPSocket;
            udpSocket2->~OsclUDPSocket();
            iAlloc.deallocate(udpSocket2);
            higher_sock_config->iUDPSocket = NULL;

            aStartPortNum = higher_sock_config->iAddr.iLocalAdd.port + 1;
            startSockID = iSocketID;
        }
    }//for max port bind attempts


    status = PVMFFailure;

Exit:
    if (status != PVMFSuccess)
    {
        OSCL_DELETE(lower_sock_config);
        OSCL_DELETE(higher_sock_config);
    }
    return status;
}

OSCL_EXPORT_REF PVMFStatus PVMFSocketNode::SetMaxTCPRecvBufferSize(uint32 aBufferSize)
{
    if ((aBufferSize > 0) && (aBufferSize < SNODE_DEFAULT_MAX_TCP_RECV_BUFFER_SIZE))
    {
        iMaxTcpRecvBufferSize = aBufferSize;
        return PVMFSuccess;
    }
    return PVMFErrArgument;
}

OSCL_EXPORT_REF PVMFStatus PVMFSocketNode::GetMaxTCPRecvBufferSize(uint32& aSize)
{
    aSize = iMaxTcpRecvBufferSize;
    return PVMFSuccess;
}

OSCL_EXPORT_REF PVMFStatus PVMFSocketNode::SetMaxTCPRecvBufferCount(uint32 aCount)
{
    iMaxTcpRecvBufferCount = aCount;
    return PVMFSuccess;
}

OSCL_EXPORT_REF PVMFStatus PVMFSocketNode::GetMaxTCPRecvBufferCount(uint32& aCount)
{
    aCount = iMaxTcpRecvBufferCount;
    return PVMFSuccess;
}

OsclMemPoolResizableAllocator* PVMFSocketNode::CreateSharedBuffer(const PVMFPortInterface* aPort , uint32 aBufferSize, uint32 aExpectedNumberOfBlocksPerBuffer, uint32 aResizeSize, uint32 aMaxNumResizes)
{
    //validate input params if required..
    OSCL_ASSERT(aPort);
    if (!aPort)
        return NULL;

    //If data buffer for the port is already created and is existing, then destroy the prev buffer and create new one (with requested attributes).
    //CreateResizableDataBufferAllocator func will take care of this.
    SocketPortConfig* portConfig = OSCL_STATIC_CAST(PVMFSocketPort*, aPort)->iConfig;
    PVMFSocketNodeMemPool * portMemPool = portConfig->iMemPool;
    portMemPool->iSharedDataBufferInfo.Init(aBufferSize, aExpectedNumberOfBlocksPerBuffer, aResizeSize, aMaxNumResizes);
    return portMemPool->CreateResizableDataBufferAllocator(portConfig->iMime.get_cstr());
}
///////////////////////////
// End Socket Extension Interface
///////////////////////////


//////////////////////////////////////
// Incoming Message Handlers
//////////////////////////////////////

//Tell whether it's possible to process an incoming message on a port.
bool PVMFSocketNode::CanProcessIncomingMsg(SocketPortConfig& aSockConfig)
{
    return
        //node is started
        iInterfaceState == EPVMFNodeStarted
        //port has input messages
        && aSockConfig.iPVMFPort && aSockConfig.iPVMFPort->IncomingMsgQueueSize() > 0
        //port is not busy with any sequence
        && aSockConfig.iState.iSequence == EPVSocketPortSequence_None;
}

//Process an incoming message on this port.
//Assume caller already verified "can process incoming msg".
void PVMFSocketNode::ProcessIncomingMsg(SocketPortConfig& aSockConfig)
{
    PVMF_SOCKETNODE_LOGSTACKTRACE((0, "PVMFSocketNode::ProcessIncomingMsg: aPort=0x%x", aSockConfig.iPVMFPort));

    //Dequeue the incoming message
    PVMFSharedMediaMsgPtr msg;
    PVMFStatus status = aSockConfig.iPVMFPort->DequeueIncomingMsg(msg);
    if (status != PVMFSuccess)
    {
        PVMF_SOCKETNODE_LOGERROR((0, "PVMFSocketNode::ProcessIncomingDataMsg: Error - DequeueIncomingMsg failed"));
        ReportErrorEvent(PVMFErrPortProcessing);
        return ;
    }

    //calling logic should prevent processing a message when some other sequence
    //is active on the port
    OSCL_ASSERT(aSockConfig.iState.iSequence == EPVSocketPortSequence_None);

    //Handle 3 types of messages
    if (msg->getFormatID() == PVMF_MEDIA_CMD_SOCKET_CONNECT_FORMAT_ID)
    {
#if(ENABLE_SOCKET_NODE_STATS)
        aSockConfig.iPortStats.iNumDequeueIncomingConnectMsg++;
#endif
        //Connect message
        PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFSocketNode::ProcessIncomingMsg - CONNECT SockId=%d, Mime=%s", aSockConfig.iSockId, aSockConfig.iMime.get_cstr()));

        //Parse the address
        OsclRefCounterMemFrag newSockAddr;
        msg->getFormatSpecificInfo(newSockAddr);
        if (newSockAddr.getMemFragPtr())
        {
            if (!ParseTransportConfig((char*)newSockAddr.getMemFragPtr(),
                                      newSockAddr.getMemFragSize(),
                                      aSockConfig.iAddr,
                                      aSockConfig.iMime))
            {
                PVMF_SOCKETNODE_LOGERROR((0, "PVMFSocketNode::ProcessIncomingMsg: ERROR - Invalid port config"));
                ReportSocketNodeError(PVMFErrPortProcessing, PVMFSocketNodeErrorInvalidPortTag);
                return ;
            }
        }

        //Start the sequence
        StartSequence(aSockConfig, EPVSocketPortSequence_InputConnectMsg);
    }
    else if (msg->getFormatID() == PVMF_MEDIA_CMD_SOCKET_DISCONNECT_FORMAT_ID)
    {
#if(ENABLE_SOCKET_NODE_STATS)
        aSockConfig.iPortStats.iNumDequeueIncomingDisconnectMsg++;
#endif
        //Disconnect message
        PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFSocketNode::ProcessIncomingMsg - DISCONNECT msg - SockId=%d, Mime=%s", aSockConfig.iSockId, aSockConfig.iMime.get_cstr()));

        //Start the sequence
        StartSequence(aSockConfig, EPVSocketPortSequence_InputDisconnectMsg);
    }
    else if (msg->getFormatID() == PVMF_MEDIA_MSG_DATA_FORMAT_ID)
    {
#if(ENABLE_SOCKET_NODE_STATS)
        aSockConfig.iPortStats.iNumDequeueIncomingDataMsg++;
#endif
        //Media Data message
        PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFSocketNode::ProcessIncomingMsg - MEDIA DATA msg - SockId=%d, Mime=%s", aSockConfig.iSockId, aSockConfig.iMime.get_cstr()));

        //calling logic should prevent any simultaneous send/connect
        OSCL_ASSERT(aSockConfig.iState.iConnectOperation == EPVSocketPortConnectOperation_None);

        //Start the sequence
        StartSequence(aSockConfig, EPVSocketPortSequence_InputDataMsg, (OsclAny*)&msg);
    }
    else
    {
        //unexpected message type
        ReportErrorEvent(PVMFErrPortProcessing);
    }
}


//////////////////////////////////////
// End Incoming Message Handlers
//////////////////////////////////////

//////////////////////////////////////
// Connect Operation Handlers
//////////////////////////////////////


//Start the given "connect" operation on the port.
//If the connect fails, this will call "connect operation complete" and continue to
//the next operation.
PVMFStatus PVMFSocketNode::StartConnectOperation(SocketPortConfig& aSockConfig, TPVSocketPortConnectOperation aOperation)
{
    PVMF_SOCKETNODE_LOGSTACKTRACE((0, "PVMFSocketNode::StartConnectOperation() In Operation %d", aOperation));

    //connect operation should be idle now.
    OSCL_ASSERT(aSockConfig.iState.iConnectOperation == EPVSocketPortConnectOperation_None);

    //Update the state
    aSockConfig.iState.iConnectOperation = aOperation;

    PVMFStatus status = PVMFFailure;

    switch (aOperation)
    {
        case EPVSocketPortConnectOperation_GetHostByName:
        {
            //This is the beginning of TCP socket create/connect.

            //this should be TCP.
            OSCL_ASSERT(aSockConfig.iAddr.iProtocol == INET_TCP);

            //Any old socket should be cleaned up or else queued for cleanup at this point.
            OSCL_ASSERT(!aSockConfig.iTCPSocket);

            //Create the socket
            PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFSocketNode::StartConnectOperation GetHostByName, Creating socket - SockId=%d, Mime=%s", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
            aSockConfig.iTCPSocket = (OsclTCPSocket*)CreateOsclSocketAndBind(aSockConfig.iAddr, aSockConfig.iSockId);
            if (!aSockConfig.iTCPSocket)
            {
                PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFSocketNode::StartConnectOperation GetHostByName, Creating socket FAILED - SockId=%d, Mime=%s", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
                PVMF_SOCKETNODE_LOGERROR((0, "PVMFSocketNode::StartConnectOperation: ERROR - CreateSocket()"));
                if (aSockConfig.iState.iSequence == EPVSocketPortSequence_RequestPort)
                {
                    iCommandErrorCode = PVMFSocketNodeErrorSocketFailure;
                }
                else
                {
                    ReportSocketNodeError(PVMFErrResource, PVMFSocketNodeErrorSocketFailure);
                }
                status = PVMFErrResource;
                break;
            }

            //see if DNS is needed
            if (OsclValidInetAddr(aSockConfig.iAddr.iRemoteHost.get_cstr()))
            {
                PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFSocketNode::StartConnectOperation GetHostByName, DNS is not needed - SockId=%d, Mime=%s", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
                aSockConfig.iAddr.iRemoteAdd.ipAddr.Set(aSockConfig.iAddr.iRemoteHost.get_cstr());
                status = PVMFSuccess;//no lookup needed-- continue to next operation.
                break;
            }

            //DNS lookup needed.
            //see if we can use the DNS cache instead of an actual lookup
            aSockConfig.iAddr.iRemoteAdd.ipAddr.Set("");
            if (iDnsCache.QueryGetHostByName(aSockConfig.iAddr.iRemoteHost.get_str(), (char*)aSockConfig.iAddr.iRemoteAdd.ipAddr.Str(), aSockConfig.iAddr.iRemoteAdd.ipAddr.MaxLen()))
            {
                //DNS cache lookup success
                PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFSocketNode::StartConnectOperation GetHostByName, DNS cache lookup success - SockId=%d, Mime=%s, Host=%s", aSockConfig.iSockId, aSockConfig.iMime.get_str(), aSockConfig.iAddr.iRemoteHost.get_str()));
                status = PVMFSuccess;//no lookup needed-- continue to next operation.
                break;
            }

            //Create DNS object
            if (!aSockConfig.iDNS)
            {
                PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFSocketNode::StartConnectOperation GetHostByName, Creating DNS object - SockId=%d, Mime=%s", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
                aSockConfig.iDNS = OsclDNS::NewL(iAlloc, *iSockServ, *this, aSockConfig.iSockId);
                if (!aSockConfig.iDNS)
                {
                    PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFSocketNode::StartConnectOperation GetHostByName, Creating DNS FAILED - SockId=%d, Mime=%s", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
                    PVMF_SOCKETNODE_LOGERROR((0, "PVMFSocketNode::StartConnectOperation: ERROR - Create DNS()"));
                    if (aSockConfig.iState.iSequence == EPVSocketPortSequence_RequestPort)
                    {
                        iCommandErrorCode = PVMFSocketNodeError_DNSLookup;
                    }
                    else
                    {
                        ReportSocketNodeError(PVMFErrResource, PVMFSocketNodeError_DNSLookup);
                    }
                    status = PVMFErrResource;
                    break;
                }
            }

            //start the asynchronous DNS operation.

#if(ENABLE_SOCKET_NODE_STATS)
            aSockConfig.iPortStats.iNumGetHostByName++;
            aSockConfig.iPortStats.StartConnectTime(aSockConfig.iState.iConnectOperation);
#endif

            TPVDNSEvent aEvent = aSockConfig.iDNS->GetHostByName(aSockConfig.iAddr.iRemoteHost.get_str(),
                                 aSockConfig.iAddr.iRemoteAdd, -1);
            if (EPVDNSPending != aEvent)
            {
                aSockConfig.iState.iConnectOperation = EPVSocketPortConnectOperation_None;
                PVMF_SOCKETNODE_LOGDATATRAFFIC_E((0, "PVMFSocketNode::StartConnectOperation: GetHostByName Failed - SockId=%d, Mime=%s, Host=%s",
                                                  aSockConfig.iSockId, aSockConfig.iMime.get_str(),
                                                  aSockConfig.iAddr.iRemoteHost.get_str()));
                if (aSockConfig.iState.iSequence == EPVSocketPortSequence_RequestPort)
                {
                    iCommandErrorCode = PVMFSocketNodeError_DNSLookup;
                }
                else
                {
                    ReportSocketNodeError(PVMFErrResource, PVMFSocketNodeError_DNSLookup);
                }
                status = PVMFErrResource;
                break;
            }

            PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFSocketNode::StartConnectOperation: Waiting On DNS - SockId=%d, Mime=%s, Host=%s",
                                              aSockConfig.iSockId, aSockConfig.iMime.get_str(),
                                              aSockConfig.iAddr.iRemoteHost.get_str()));
#if (ENABLE_SOCKET_NODE_DEBUG_INFO_EVENT)
            ReportInfoEvent(PVMFSocketNodeInfoEventRequestedDNSResolution);
#endif
            status = PVMFPending;
            //wait on the DNS lookup to complete in HandleDNSEvent callback
        }
        break;

        case EPVSocketPortConnectOperation_Connect:
            //Initiate a socket connect, after DNS lookup was successful or address is already IP.
            //Will return "pending" or an error.
        {
            if (!aSockConfig.iTCPSocket)
            {
                PVMF_SOCKETNODE_LOGDATATRAFFIC_E((0, "PVMFSocketNode::StartConnectOperation: ERROR socket already exists - SockId=%d, Mime=%s",
                                                  aSockConfig.iSockId, aSockConfig.iMime.get_str()));
                status = PVMFFailure;//unexpected
                break;
            }

            //start the asynchronous connect operation.

#if(ENABLE_SOCKET_NODE_STATS)
            aSockConfig.iPortStats.iNumConnect++;
            aSockConfig.iPortStats.StartConnectTime(aSockConfig.iState.iConnectOperation);
#endif

            TPVSocketEvent ret = aSockConfig.iTCPSocket->Connect(aSockConfig.iAddr.iRemoteAdd, TIMEOUT_CONNECT);
            if (ret != EPVSocketPending)
            {
                //failure
                PVMF_SOCKETNODE_LOGDATATRAFFIC_E((0, "PVMFSocketNode::StartConnectOperation: Starting Connect FAILED - SockId=%d, Mime=%s, RemoteIP=%s",
                                                  aSockConfig.iSockId, aSockConfig.iMime.get_str(),
                                                  (char*)(aSockConfig.iAddr.iRemoteAdd.ipAddr.Str())));
                if (aSockConfig.iState.iSequence == EPVSocketPortSequence_RequestPort)
                {
                    iCommandErrorCode = PVMFSocketNodeError_TCPSocketConnect;
                }
                else
                {
                    ReportSocketNodeError(PVMFErrResource, PVMFSocketNodeError_TCPSocketConnect);
                }
                status = PVMFErrResource;
                break;
            }
            PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFSocketNode::StartConnectOperation - Socket Connect Pending...SockId=%d, Mime=%s, RemoteIP=%s"
                                              , aSockConfig.iSockId, aSockConfig.iMime.get_str(),
                                              (char*)(aSockConfig.iAddr.iRemoteAdd.ipAddr.Str())));
#if (ENABLE_SOCKET_NODE_DEBUG_INFO_EVENT)
            ReportInfoEvent(PVMFSocketNodeInfoEventConnectRequestPending);
#endif
            status = PVMFPending;
            //wait on the socket connect to complete in HandleSocketEvent callback
        }
        break;

        case EPVSocketPortConnectOperation_Shutdown:
        {
            if (!aSockConfig.iTCPSocket)
            {
                PVMF_SOCKETNODE_LOGDATATRAFFIC_E((0, "PVMFSocketNode::StartConnectOperation: ERROR socket already exists - SockId=%d, Mime=%s",
                                                  aSockConfig.iSockId, aSockConfig.iMime.get_str()));
                status = PVMFFailure;//unexpected
                break;
            }

            //Initiate a socket shutdown.
#if(ENABLE_SOCKET_NODE_STATS)
            aSockConfig.iPortStats.iNumShutdown++;
            aSockConfig.iPortStats.StartConnectTime(aSockConfig.iState.iConnectOperation);
#endif
            TPVSocketEvent ret = aSockConfig.iTCPSocket->Shutdown(EPVSocketBothShutdown, TIMEOUT_SHUTDOWN);
            if (ret == EPVSocketPending)
            {
                PVMF_SOCKETNODE_LOGDATATRAFFIC_E((0, "PVMFSocketNode::StartConnectOperation - Shutdown Pending...SockId=%d, Mime=%s", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
                status = PVMFPending;
                //wait on the socket shutdown to complete in HandleSocketEvent callback
            }
            else
            {
                status = PVMFFailure;
                PVMF_SOCKETNODE_LOGDATATRAFFIC_E((0, "PVMFSocketNode::StartConnectOperation - Shutdown FAILED - SockId=%d, Mime=%s", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
            }
        }
        break;

        case EPVSocketPortConnectOperation_WaitOnConnectedPort:
            if (aSockConfig.iPVMFPort
                    && aSockConfig.iPVMFPort->IsOutgoingQueueBusy())
            {
#if(ENABLE_SOCKET_NODE_STATS)
                aSockConfig.iPortStats.iNumWaitOnConnectedPort++;
                aSockConfig.iPortStats.StartConnectTime(aSockConfig.iState.iConnectOperation);
#endif
                status = PVMFPending;
                //wait for CONNECTED_PORT_READY event in HandlePortActivity
            }
            else
            {
                status = PVMFSuccess;//nothing needed.
            }
            break;

        case EPVSocketPortConnectOperation_WaitOnSendRecvIdle:
            if (aSockConfig.iState.iRecvOperation == EPVSocketPortRecvOperation_None
                    && aSockConfig.iState.iSendOperation == EPVSocketPortSendOperation_None)
            {
                //send/recv are already idle.
                status = PVMFSuccess;
            }
            else
            {
#if(ENABLE_SOCKET_NODE_STATS)
                aSockConfig.iPortStats.iNumWaitOnSendRecvIdle++;
                aSockConfig.iPortStats.StartConnectTime(aSockConfig.iState.iConnectOperation);
#endif
                status = PVMFPending;
                //wait for send/recv completion (in RecvOperationComplete or SendOperationComplete)
            }
            break;

        default:
            //add code to handle this case
            status = PVMFFailure;
            OSCL_ASSERT(0);
            break;
    }

    //Handle synchronous completion or failure.
    if (status != PVMFPending)
        status = ConnectOperationComplete(aSockConfig, status, NULL);

    return status;
}

//The current connect operation is complete.
//Continue with the next operation, or finish the sequence.
//The status is in "aStatus".  If there was a socket callback, the result is in "aSocketActivity".
PVMFStatus PVMFSocketNode::ConnectOperationComplete(SocketPortConfig& aSockConfig, PVMFStatus aStatus, PVMFSocketActivity* aSocketActivity)
{
    PVMF_SOCKETNODE_LOGSTACKTRACE((0, "PVMFSocketNode::ConnectOperationComplete() in"));

    //status should never be "pending" here
    OSCL_ASSERT(aStatus != PVMFPending);

    PVMFStatus status = aStatus;

    TPVSocketPortConnectOperation curOp = aSockConfig.iState.iConnectOperation;
    aSockConfig.iState.iConnectOperation = EPVSocketPortConnectOperation_None;
    aSockConfig.iState.iConnectOperationStatus = aStatus;
    aSockConfig.iState.iConnectOperationCanceled = false;


    switch (curOp)
    {
        case EPVSocketPortConnectOperation_None:
            //unexpected.
            break;

        case EPVSocketPortConnectOperation_GetHostByName:
            //TCP Get Host By Name is complete

            //catch completion of connect cancel during the cleanup sequence
            if (aSockConfig.iState.iSequence == EPVSocketPortSequence_SocketCleanup)
            {
                //Continue the shutdown sequence
                status = StartConnectOperation(aSockConfig, EPVSocketPortConnectOperation_Shutdown);
                break;
            }
            //else normal connect sequence.

            if (aStatus == PVMFSuccess)
            {
                PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFSocketNode::ConnectOperationComplete GetHostByName Success - SockId=%d, Mime=%s Host= %s", aSockConfig.iSockId, aSockConfig.iMime.get_str()
                                                  , aSockConfig.iAddr.iRemoteHost.get_str()));
                //Save lookup results in the DNS cache
                if (aSocketActivity)
                {
                    iDnsCache.UpdateCacheRecord(aSockConfig.iAddr.iRemoteHost.get_str(), (char*)aSockConfig.iAddr.iRemoteAdd.ipAddr.Str());
                }

                //DNS Lookup is always followed by a Connect
                status = StartConnectOperation(aSockConfig, EPVSocketPortConnectOperation_Connect);
            }
            else
            {
                PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFSocketNode::ConnectOperationComplete GetHostByName FAILED - SockId=%d, Mime=%s ", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
                //If DNS lookup failed, then fail the current sequence with
                //an error.
                switch (aSockConfig.iState.iSequence)
                {
                    case EPVSocketPortSequence_RequestPort:
                        //report the error in the command complete
                        iCommandErrorCode = PVMFSocketNodeError_DNSLookup;
                        break;
                    default:
                        //report the error in an error message
                        ReportSocketNodeError(PVMFErrResource, PVMFSocketNodeError_DNSLookup);
                        break;
                }
                //Fail the sequence.
                SequenceComplete(aSockConfig, aStatus);
            }
            break;

        case EPVSocketPortConnectOperation_Connect:
            //TCP Connect is complete

            //catch completion of connect cancel during the cleanup sequence
            if (aSockConfig.iState.iSequence == EPVSocketPortSequence_SocketCleanup)
            {
                //Continue the shutdown sequence
                status = StartConnectOperation(aSockConfig, EPVSocketPortConnectOperation_Shutdown);
                break;
            }
            //else normal connect sequence.

            if (aStatus == PVMFSuccess)
            {
                PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFSocketNode::ConnectOperationComplete Connect Success - SockId=%d, Mime=%s ", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
            }
            else
            {
                PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFSocketNode::ConnectOperationComplete Connect FAILED - SockId=%d, Mime=%s ", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
                //Report connect errors.
                switch (aSockConfig.iState.iSequence)
                {
                    case EPVSocketPortSequence_RequestPort:
                        //for a node command, we report the error in the node command completion message
                        iCommandErrorCode = PVMFSocketNodeError_TCPSocketConnect;
                        break;
                    case EPVSocketPortSequence_InputConnectMsg:
                    default:
                        //for other sequences, we report the error in an error message.
                        ReportSocketNodeError(PVMFErrResource, PVMFSocketNodeError_TCPSocketConnect);
                        break;
                }
            }
            //Connect complete is always the end of a sequence.
            SequenceComplete(aSockConfig, aStatus);
            break;

        case EPVSocketPortConnectOperation_Shutdown:
            //TCP Shutdown is complete

            if (aStatus == PVMFSuccess)
            {
                PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFSocketNode::ConnectOperationComplete Shutdown Success - SockId=%d, Mime=%s ", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
            }
            else
            {
                PVMF_SOCKETNODE_LOGDATATRAFFIC_E((0, "PVMFSocketNode::ConnectOperationComplete Shutdown FAILED - SockId=%d, Mime=%s ", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
                if (aSocketActivity)
                {
                    PVMF_SOCKETNODE_LOGERROR((0, "PVMFSocketNode::ConnectOperationComplete: ERROR aEvent=%d, Ln %d", aSocketActivity->iEvent, __LINE__));
                }
            }

            //Note: ignore shutdown failures and just continue onto the
            //next operation.

            //Shutdown is followed by waiting for send/recv to go idle.

            //On most OS the shutdown command is sufficient to cause send/recv to complete,
            //but on some it doesn't, so cancel any outstanding send/recv op here.
            CancelSendOperation(aSockConfig);
            CancelRecvOperation(aSockConfig);

            status = StartConnectOperation(aSockConfig, EPVSocketPortConnectOperation_WaitOnSendRecvIdle);
            break;

        case EPVSocketPortConnectOperation_WaitOnConnectedPort:
            //TCP Wait on connected port is complete.

            //Proceed to send the EOS and close the socket
            PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFSocketNode::ConnectOperationComplete WaitOnConnectedPort Calling CleanupTCP - SockId=%d, Mime=%s ", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
            CleanupTCP(aSockConfig);

            //Check the sequence...
            switch (aSockConfig.iState.iSequence)
            {
                case EPVSocketPortSequence_InputConnectMsg:
                    //a disconnect is followed by a host lookup + connect
                    //for an input connect message
                    status = StartConnectOperation(aSockConfig, EPVSocketPortConnectOperation_GetHostByName);
                    break;

                default:
                    //for all other sequences, this is the end
                    SequenceComplete(aSockConfig, PVMFSuccess);
                    status = PVMFSuccess;
                    break;
            }
            break;

        case EPVSocketPortConnectOperation_WaitOnSendRecvIdle:
            //UDP or TCP "Wait on Send/Recv Idle" is complete.

            //If TCP, wait on connected port to send EOS
            if (aSockConfig.iTCPSocket)
            {
                status = StartConnectOperation(aSockConfig, EPVSocketPortConnectOperation_WaitOnConnectedPort);
            }
            //If UDP, proceed to close & delete
            else if (aSockConfig.iUDPSocket)
            {
                CleanupUDP(aSockConfig);
                //This is the end of the sequence.
                SequenceComplete(aSockConfig, PVMFSuccess);
                status = PVMFSuccess;
            }
            else
            {
                //unexpected, but end the sequence anyway.
                SequenceComplete(aSockConfig, PVMFSuccess);
                status = PVMFSuccess;
            }
            break;


        default:
            OSCL_ASSERT(0);//unexpected, add code for this case.
            break;
    }
    return status;
}

PVMFStatus PVMFSocketNode::CancelConnectOperation(SocketPortConfig& aSockConfig)
{
    PVMF_SOCKETNODE_LOGSTACKTRACE((0, "PVMFSocketNode::CancelConnectOperation() In"));

    PVMFStatus status = PVMFSuccess;

    switch (aSockConfig.iState.iConnectOperation)
    {
        case EPVSocketPortConnectOperation_None:
            break;

        case EPVSocketPortConnectOperation_GetHostByName:
            if (aSockConfig.iDNS)
            {
                if (!aSockConfig.iState.iConnectOperationCanceled)
                {
                    aSockConfig.iState.iConnectOperationCanceled = true;
                    PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFSocketNode::CancelConnectOperation Calling CancelGetHostByName - SockId=%d, Mime=%s ", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
                    aSockConfig.iDNS->CancelGetHostByName();
                }
                status = PVMFPending;
                //wait for host lookup to complete in HandleDNSEvent
            }
            break;

        case EPVSocketPortConnectOperation_Connect:
            if (aSockConfig.iTCPSocket)
            {
                if (!aSockConfig.iState.iConnectOperationCanceled)
                {
                    aSockConfig.iState.iConnectOperationCanceled = true;
                    PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFSocketNode::CancelConnectOperation Calling CancelConnect - SockId=%d, Mime=%s ", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
                    aSockConfig.iTCPSocket->CancelConnect();
                }
                status = PVMFPending;
                //wait for connect to complete in HandleSocketEvent
            }
            break;

        case EPVSocketPortConnectOperation_Shutdown:
            if (aSockConfig.iTCPSocket)
            {
                if (!aSockConfig.iState.iConnectOperationCanceled)
                {
                    aSockConfig.iState.iConnectOperationCanceled = true;
                    PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFSocketNode::CancelConnectOperation Calling CancelShutdown - SockId=%d, Mime=%s ", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
                    aSockConfig.iTCPSocket->CancelShutdown();
                }
                status = PVMFPending;
                //wait for shutdown to complete in HandleSocketEvent
            }
            break;

        case EPVSocketPortConnectOperation_WaitOnConnectedPort:
            //just clear the state
            PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFSocketNode::CancelConnectOperation Cancelling WaitOnConnectedPort - SockId=%d, Mime=%s ", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
            aSockConfig.iState.iConnectOperation = EPVSocketPortConnectOperation_None;
            break;

        case EPVSocketPortConnectOperation_WaitOnSendRecvIdle:
            //just clear the state
            PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFSocketNode::CancelConnectOperation Cancelling WaitOnSendRecvIdle - SockId=%d, Mime=%s ", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
            aSockConfig.iState.iConnectOperation = EPVSocketPortConnectOperation_None;
            break;

        default:
            OSCL_ASSERT(0);//add code for this case
            status = PVMFFailure;
            break;
    }
    return status;
}

//////////////////////////////////////
// End Connect Operation Handlers
//////////////////////////////////////

//////////////////////////////////
// Receive Operation Handlers
//////////////////////////////////

//Tell if it's possible to initiate a recv or recvfrom operation
//on this port.
bool PVMFSocketNode::CanReceive(SocketPortConfig& aSockConfig)
{
    return
        //node is started
        iInterfaceState == EPVMFNodeStarted
        //port is connected
        && aSockConfig.iPVMFPort && aSockConfig.iPVMFPort->IsConnected()
        //socket exists (gets created during request port or connect sequence)
        && (aSockConfig.iUDPSocket || aSockConfig.iTCPSocket)
        //port is not busy with any sequence other than sending data
        && (aSockConfig.iState.iSequence == EPVSocketPortSequence_None
            || aSockConfig.iState.iSequence == EPVSocketPortSequence_InputDataMsg)
        //port is not busy with any receive operation.
        && aSockConfig.iState.iRecvOperation == EPVSocketPortRecvOperation_None
        //there's no node stop going on.
        && iNumStopPortActivityPending < 0;
}

//Enter the "wait on memory" state
void PVMFSocketNode::StartRecvWaitOnMemory(SocketPortConfig& aSockConfig, int32 aSize)
{
    PVMF_SOCKETNODE_LOGSTACKTRACE((0, "PVMFSocketNode::StartRecvWaitOnMemory() In"));

    aSockConfig.iState.iRecvOperation = EPVSocketPortRecvOperation_WaitOnMemory;

#if(ENABLE_SOCKET_NODE_STATS)
    aSockConfig.iPortStats.iNumWaitOnMemory++;
    aSockConfig.iPortStats.StartRecvTime(aSockConfig.iState.iRecvOperation);
#endif

    if (aSize)
    {//wait on data buffer
        PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFSocketNode::StartRecvWaitOnMemory, wait on data buffer - SockId=%d, Mime=%s ", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
        if (aSockConfig.iMemPool->iSharedBufferAlloc)
        {
            aSockConfig.iMemPool->iSharedBufferAlloc->notifyfreeblockavailable(aSockConfig, aSize, NULL);
        }
        else
        {
            if (aSockConfig.iMemPool->iInternalAlloc)
                aSockConfig.iMemPool->iInternalAlloc->notifyfreeblockavailable(aSockConfig, aSize, NULL);
        }
    }
    else
    {//wait on media data wrapper
        PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFSocketNode::StartRecvWaitOnMemory, wait on data wrapper - SockId=%d, Mime=%s ", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
        aSockConfig.iMemPool->iMediaDataMemPool->notifyfreechunkavailable(aSockConfig, NULL);
    }
}

//Enter the "wait on connected port" state
void PVMFSocketNode::StartRecvWaitOnConnectedPort(SocketPortConfig& aSockConfig, PVMFSocketActivity& aSocketActivity)
{
    PVMF_SOCKETNODE_LOGSTACKTRACE((0, "PVMFSocketNode::StartRecvWaitOnConnectedPort() In"));

    //outgoing queue is busy-- must queue this event for later processing

    PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFSocketNode::StartRecvWaitOnConnectedPort, wait for connected port - SockId=%d, Mime=%s ", aSockConfig.iSockId, aSockConfig.iMime.get_str()));

    switch (aSocketActivity.iFxn)
    {
        case EPVSocketRecv:
        case EPVSocketRecvFrom:
#if(ENABLE_SOCKET_NODE_STATS)
            aSockConfig.iPortStats.iNumQueueSocketRecv++;
#endif
            aSockConfig.iSocketRecvActivity.Set(aSocketActivity.iStatus
                                                , aSocketActivity.iId
                                                , aSocketActivity.iFxn
                                                , aSocketActivity.iEvent
                                                , aSocketActivity.iError);
            break;
        default:
            OSCL_ASSERT(false);//invalid input arg.
            break;
    }

    //current state shoudl be idle
    OSCL_ASSERT(aSockConfig.iState.iRecvOperation == EPVSocketPortRecvOperation_None);

    aSockConfig.iState.iRecvOperation = EPVSocketPortRecvOperation_WaitOnConnectedPort;

#if(ENABLE_SOCKET_NODE_STATS)
    aSockConfig.iPortStats.iNumWaitOnConnectedPort++;
    aSockConfig.iPortStats.StartRecvTime(aSockConfig.iState.iRecvOperation);
#endif

}

//Start a receive operation on the port's socket.
//Assume the caller already verified that the port can receive now.
//If there is a failure, this routine will call the "recv operation complete" routine to
//do error handling or continue to the next state.
PVMFStatus PVMFSocketNode::StartRecvOperation(SocketPortConfig& aSockConfig)
{
    PVMF_SOCKETNODE_LOGSTACKTRACE((0, "PVMFSocketNode::StartRecvOperation() In"));

    //caller should have verified we can receive data right now.
    OSCL_ASSERT(aSockConfig.iState.iRecvOperation == EPVSocketPortRecvOperation_None);

    //there should be either a UDP or TCP socket on this port and a memory pool.
    if (!aSockConfig.iTCPSocket && !aSockConfig.iUDPSocket)
    {
        PVMF_SOCKETNODE_LOGDATATRAFFIC_E((0, "PVMFSocketNode::StartRecvOperation ERROR no socket - SockId=%d, Mime=%s ", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
        return PVMFFailure;
    }
    if (!aSockConfig.iMemPool)
    {
        PVMF_SOCKETNODE_LOGDATATRAFFIC_E((0, "PVMFSocketNode::StartRecvOperation ERROR no mem pool - SockId=%d, Mime=%s ", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
        return PVMFFailure;
    }

    PVMFStatus status = PVMFFailure;

    if (aSockConfig.iUDPSocket)
    {
        aSockConfig.iState.iRecvOperation = EPVSocketPortRecvOperation_RecvFrom;

        //Create the memory buffer

        int32 memSize;
        int32 multiRecvLimitBytes = 0;
        switch (aSockConfig.iMemPool->iPortTag)
        {
            case PVMF_SOCKET_NODE_PORT_TYPE_SOURCE:
                //source tag is used for RTP
#if SNODE_ENABLE_UDP_MULTI_PACKET
                //must allocate enough space for the max bytes per receive plus
                //one more max size packet, since we can't peek ahead to see
                //how much data is on the socket.
                memSize = SNODE_UDP_MULTI_MAX_BYTES_PER_RECV + MAX_UDP_PACKET_SIZE;
                //this is the amt of "free space" required to continue receiving
                //additional packets. Use the max packet size to avoid truncated
                //packets.
                multiRecvLimitBytes = MAX_UDP_PACKET_SIZE;
#else
                //for single packet recvs.
                memSize = MAX_UDP_PACKET_SIZE;
#endif
                break;
            case PVMF_SOCKET_NODE_PORT_TYPE_SINK:
                //sink tag is used for RTCP
            default:
                memSize = MAX_FEEDBACK_SOCKET_BUFFER_SIZE;
                break;
        }

        //Socket node provides the buffer to receive the data on the socket
        //Socket node gets this from the mem pool.
        //It may be possible that socket node may not be able to get the
        //buffer of required size from the mempool
        //If we are not able to get the packets from the OS, then the packets
        //received from server will eventually be lost.
        //In general, streaming server will be sending the UDP packets of small size
        //(say 2048 bytes.. = MIN_UDP_PACKET_SIZE)
        //So its better to attempt to allocate the memory of size
        //SNODE_UDP_MULTI_MIN_BYTES_PER_RECV + MIN_UDP_PACKET_SIZE/MIN_UDP_PACKET_SIZE
        //in such cases.

        const int32 largestMediaMsgSz = aSockConfig.iMemPool->GetMaxSizeMediaMsgLen();

        PVMF_SOCKETNODE_LOGINFO((0, "PVMFSocketNode::StartRecvOperation - MaxSizeMediMsg  %d memSize %d", largestMediaMsgSz, memSize));

        if (memSize > largestMediaMsgSz)
        {
#if SNODE_ENABLE_UDP_MULTI_PACKET
            multiRecvLimitBytes = MIN_UDP_PACKET_SIZE;

            const int32 minBufferToContinueRecv = (SNODE_UDP_MULTI_MIN_BYTES_PER_RECV + MIN_UDP_PACKET_SIZE);

            if (largestMediaMsgSz >= minBufferToContinueRecv)
            {
                memSize = largestMediaMsgSz;
            }
            else
            {
                memSize = minBufferToContinueRecv;
            }
#else
            if (largestMediaMsgSz >= MIN_UDP_PACKET_SIZE)
            {
                memSize = largestMediaMsgSz;
            }
            else
            {
                memSize = MIN_UDP_PACKET_SIZE;
            }
#endif
            PVMF_SOCKETNODE_LOGINFO((0, "PVMFSocketNode::StartRecvOperation - Resizing Mem Request Attributes## NewSz %d recvLimitBytes %d", memSize, multiRecvLimitBytes));
        }

        //Create the media data impl.
        int32 err;
        OsclSharedPtr<PVMFMediaDataImpl> mediaDataImpl;
        err = GetMediaDataImpl(aSockConfig, mediaDataImpl, memSize);
        if (err != OsclErrNone)
        {
            StartRecvWaitOnMemory(aSockConfig, memSize);
            status = PVMFPending;
            //wait on memory pool callback "notifyfreechunkavailable"
            goto Exit;
        }

        //Create the media data buffer.
        err = CreateMediaData(aSockConfig, mediaDataImpl);
        if (err != OsclErrNone)
        {
            StartRecvWaitOnMemory(aSockConfig);
            status = PVMFPending;
            //wait on memory pool callback "notifyfreechunkavailable"
            goto Exit;
        }

        // Retrieve memory fragment to write to
        OsclRefCounterMemFrag refCtrMemFragOut;
        aSockConfig.iPendingRecvMediaData->getMediaFragment(0, refCtrMemFragOut);

        //Issue the Oscl socket request

#if(ENABLE_SOCKET_NODE_STATS)
        aSockConfig.iPortStats.iNumRecvFrom++;
        aSockConfig.iPortStats.StartRecvTime(EPVSocketPortRecvOperation_RecvFrom);
#endif

        TPVSocketEvent retVal = aSockConfig.iUDPSocket->RecvFrom((uint8*)refCtrMemFragOut.getMemFragPtr()
                                , refCtrMemFragOut.getCapacity()
                                , aSockConfig.iSourceAddr
                                , TIMEOUT_RECVFROM
                                , multiRecvLimitBytes
                                , &aSockConfig.iRecvFromPacketLen);

        if (EPVSocketPending != retVal)
        {
            PVMF_SOCKETNODE_LOGERROR((0, "PVMFSocketNode::RecvFrom() failed retVal=%d Ln %d", retVal, __LINE__));
            PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFSocketNode::StartRecvOperation RecvFrom FAILED - SockId=%d, Mime=%s ", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
            status = PVMFFailure;
            goto Exit;
        }
        PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFSocketNode::StartRecvOperation RecvFrom Pending - SockId=%d, Mime=%s ", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
        status = PVMFPending;
        //wait on the socket callback HandleSocketEvent
    }

    else if (aSockConfig.iTCPSocket)
    {
        aSockConfig.iState.iRecvOperation = EPVSocketPortRecvOperation_Recv;

        //Allocate memory
        int32 err;
        OsclSharedPtr<PVMFMediaDataImpl> mediaDataImpl;
        mediaDataImpl = aSockConfig.iMemPool->getMediaDataImpl(iMaxTcpRecvBufferSize);
        if (NULL == mediaDataImpl.GetRep())
        {
            StartRecvWaitOnMemory(aSockConfig, iMaxTcpRecvBufferSize);
            status = PVMFPending;
            //wait on memory pool callback "notifyfreechunkavailable"
            goto Exit;
        }

        OSCL_TRY(err,
                 aSockConfig.iPendingRecvMediaData = PVMFMediaData::createMediaData(mediaDataImpl, aSockConfig.iMemPool->iMediaDataMemPool););
        if (err != OsclErrNone)
        {
            StartRecvWaitOnMemory(aSockConfig);
            status = PVMFPending;
            //wait on memory pool callback "notifyfreechunkavailable"
            goto Exit;
        }

        // Retrieve memory fragment to write to
        OsclRefCounterMemFrag refCtrMemFragOut;
        aSockConfig.iPendingRecvMediaData->getMediaFragment(0, refCtrMemFragOut);

        //Issue the Oscl socket request.

#if(ENABLE_SOCKET_NODE_STATS)
        aSockConfig.iPortStats.iNumRecv++;
        aSockConfig.iPortStats.StartRecvTime(EPVSocketPortRecvOperation_Recv);
#endif

        TPVSocketEvent retVal = aSockConfig.iTCPSocket->Recv((uint8*)refCtrMemFragOut.getMemFragPtr()
                                , refCtrMemFragOut.getCapacity()
                                , TIMEOUT_RECV);
        if (EPVSocketPending != retVal)
        {
            PVMF_SOCKETNODE_LOGERROR((0, "PVMFSocketNode::Recv() failed retVal=%d Ln %d", retVal, __LINE__));
            PVMF_SOCKETNODE_LOGDATATRAFFIC_E((0, "PVMFSocketNode::StartRecvOperation Recv FAILED - SockId=%d, Mime=%s ", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
            status = PVMFFailure;
            goto Exit;
        }
        PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFSocketNode::StartRecvOperation Recv Pending - SockId=%d, Mime=%s ", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
        status = PVMFPending;
        //wait on HandleSocketEvent callback.
    }
    else
    {
        PVMF_SOCKETNODE_LOGERROR((0, "PVMFSocketNode::StartRecvOperation() Unexpected error, no socket"));
    }

Exit:
    //Handle synchronous completion or failure.

    if (status != PVMFPending
            && status != PVMFFailure)
    {
        PVMF_SOCKETNODE_LOGERROR((0, "PVMFSocketNode::StartRecvOperation: Error. status=%d", status));
    }

    if (status != PVMFPending)
        status = RecvOperationComplete(aSockConfig, status, NULL);

    return status;
}

//The current recv operation is complete.  Process the result and continue with the next operation.
//The status is in "aStatus".
//If there was a socket callback, the result is in "aSocketActivity".
PVMFStatus PVMFSocketNode::RecvOperationComplete(SocketPortConfig& aSockConfig, PVMFStatus aStatus, PVMFSocketActivity* aSocketActivity)
{
    PVMF_SOCKETNODE_LOGSTACKTRACE((0, "PVMFSocketNode::RecvOperationComplete() In"));

    //status should never be pending here
    OSCL_ASSERT(aStatus != PVMFPending);

    PVMFStatus status = aStatus;
    bool recvOperationCanceled = aSockConfig.iState.iRecvOperationCanceled;

    //Update the state
    TPVSocketPortRecvOperation curOp = aSockConfig.iState.iRecvOperation;
    aSockConfig.iState.iRecvOperation = EPVSocketPortRecvOperation_None;
    aSockConfig.iState.iRecvOperationStatus = aStatus;
    aSockConfig.iState.iRecvOperationCanceled = false;

    switch (curOp)
    {
        case EPVSocketPortRecvOperation_WaitOnMemory:
            //a memory wait is complete.
            PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFSocketNode::RecvOperationComplete WaitOnMemory - SockId=%d, Mime=%s ", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
            break;

        case EPVSocketPortRecvOperation_WaitOnConnectedPort:
            //a port wait is complete
            PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFSocketNode::RecvOperationComplete WaitOnConnectedPort - SockId=%d, Mime=%s ", aSockConfig.iSockId, aSockConfig.iMime.get_str()));

            //there may be a prior recv or recvfrom result that was waiting on this
            if (aSockConfig.iSocketRecvActivity.iValid)
            {
                aSockConfig.iSocketRecvActivity.iValid = false;

                switch (aSockConfig.iSocketRecvActivity.iFxn)
                {
                    case EPVSocketRecv:
                        HandleRecvComplete(aSockConfig
                                           , aSockConfig.iSocketRecvActivity.iStatus
                                           , &aSockConfig.iSocketRecvActivity, recvOperationCanceled);
                        break;
                    case EPVSocketRecvFrom:
                        HandleRecvFromComplete(aSockConfig
                                               , aSockConfig.iSocketRecvActivity.iStatus
                                               , &aSockConfig.iSocketRecvActivity, recvOperationCanceled);
                        break;
                    default:
                        OSCL_ASSERT(0);//invalid arg
                        break;
                }
            }
            break;

        case EPVSocketPortRecvOperation_Recv:
            //TCP receive is complete
            PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFSocketNode::RecvOperationComplete Recv - SockId=%d, Mime=%s ", aSockConfig.iSockId, aSockConfig.iMime.get_str()));

            HandleRecvComplete(aSockConfig, aStatus, aSocketActivity, recvOperationCanceled);
            break;

        case EPVSocketPortRecvOperation_RecvFrom:
            //UDP receive from is complete
            PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFSocketNode::RecvOperationComplete RecvFrom - SockId=%d, Mime=%s ", aSockConfig.iSockId, aSockConfig.iMime.get_str()));

            HandleRecvFromComplete(aSockConfig, aStatus, aSocketActivity, recvOperationCanceled);
            break;

        default:
            OSCL_ASSERT(0);//add code for this case
            break;
    }

    //Receive may have been blocked waiting on this operation to complete,
    //so start/resume receives now if possible
    if (CanReceive(aSockConfig))
        status = StartRecvOperation(aSockConfig);

    //Detect connect "Wait on send/recv idle" complete
    if (aSockConfig.iState.iConnectOperation == EPVSocketPortConnectOperation_WaitOnSendRecvIdle
            && aSockConfig.iState.iRecvOperation == EPVSocketPortRecvOperation_None
            && aSockConfig.iState.iSendOperation == EPVSocketPortSendOperation_None)
    {
        ConnectOperationComplete(aSockConfig, PVMFSuccess, NULL);
    }

    return status;
}

//Cancel any current receive operation.
//Return Success if the operation is canceled or Pending if we need to wait.
PVMFStatus PVMFSocketNode::CancelRecvOperation(SocketPortConfig& aSockConfig)
{
    PVMF_SOCKETNODE_LOGSTACKTRACE((0, "PVMFSocketNode::CancelRecvOperation() In"));

    PVMFStatus status = PVMFSuccess;

    switch (aSockConfig.iState.iRecvOperation)
    {
        case EPVSocketPortRecvOperation_None:
            break;

        case EPVSocketPortRecvOperation_Recv:
            if (aSockConfig.iTCPSocket)
            {
                if (!aSockConfig.iState.iRecvOperationCanceled)
                {
                    aSockConfig.iState.iRecvOperationCanceled = true;
                    PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFSocketNode::CancelRecvOperation Calling CancelRecv - SockId=%d, Mime=%s ", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
                    aSockConfig.iTCPSocket->CancelRecv();
                }
                status = PVMFPending;
                //wait on recv to complete in HandleSocketEvent
            }
            break;

        case EPVSocketPortRecvOperation_RecvFrom:
            if (aSockConfig.iUDPSocket)
            {
                if (!aSockConfig.iState.iRecvOperationCanceled)
                {
                    aSockConfig.iState.iRecvOperationCanceled = true;
                    PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFSocketNode::CancelRecvOperation Calling CancelRecvFrom - SockId=%d, Mime=%s ", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
                    aSockConfig.iUDPSocket->CancelRecvFrom();
                }
                status = PVMFPending;
                //wait on recvfrom to complete in HandleSocketEvent
            }
            break;

        case EPVSocketPortRecvOperation_WaitOnConnectedPort:
            //just clear the state
            PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFSocketNode::CancelRecvOperation Cancelling WaitOnConnectedPort - SockId=%d, Mime=%s ", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
            aSockConfig.iState.iRecvOperation = EPVSocketPortRecvOperation_None;
            //also discard the received data and the associated recv activity
            if (aSockConfig.iSocketRecvActivity.iValid)
                aSockConfig.iSocketRecvActivity.iValid = false;
            if (aSockConfig.iPendingRecvMediaData.GetRep())
                aSockConfig.iPendingRecvMediaData.Unbind();
            break;

        case EPVSocketPortRecvOperation_WaitOnMemory:
            PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFSocketNode::CancelRecvOperation Cancelling WaitOnMemory - SockId=%d, Mime=%s ", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
            if (aSockConfig.iMemPool != NULL)
            {
                if (aSockConfig.iMemPool->iInternalAlloc)
                    aSockConfig.iMemPool->iInternalAlloc->CancelFreeChunkAvailableCallback();
                if (aSockConfig.iMemPool->iSharedBufferAlloc)
                    aSockConfig.iMemPool->iSharedBufferAlloc->CancelFreeChunkAvailableCallback();
            }
            //clear the state
            aSockConfig.iState.iRecvOperation = EPVSocketPortRecvOperation_None;
            break;

        default:
            OSCL_ASSERT(0);//add code for this case
            status = PVMFFailure;
            break;
    }
    return status;
}

//a memory pool callback
void SocketPortConfig::freechunkavailable(OsclAny* aContextData)
{
#if(ENABLE_SOCKET_NODE_STATS)
    iPortStats.iNumFreeChunkCallback++;
#endif
    OSCL_UNUSED_ARG(aContextData);

    //complete the "wait on memory" state
    if (iState.iRecvOperation == EPVSocketPortRecvOperation_WaitOnMemory)
    {
#if(ENABLE_SOCKET_NODE_STATS)
        iPortStats.EndRecvTime(iState.iRecvOperation);
#endif
        iContainer->RecvOperationComplete(*this, PVMFSuccess, NULL);
    }
}

void SocketPortConfig::freeblockavailable(OsclAny* aContextData)
{
    freechunkavailable(aContextData);
}

//Handler for a "recv" complete.  This handles two input cases:
// - Operation failed to initiate.  Status will be in aStatus and aSocketActivity will be NULL.
// - Operation completed asynchronously via HandleSocketEvent callback. Status will be in aStatus
//   and socket callback result will be in aSocketActivity.
//
//The end result of processing will be one of the following:
// - Received data sent to connected port
// - Result queued on socket activity queue for later processing.
// - Error or cancellation processed.
void PVMFSocketNode::HandleRecvComplete(SocketPortConfig& aSockConfig, PVMFStatus aStatus, PVMFSocketActivity* aSocketActivity, bool aRecvOperationCanceled)
{
    PVMF_SOCKETNODE_LOGSTACKTRACE((0, "PVMFSocketNode::HandleRecvComplete() In"));

    //operation should be complete when this is called.
    OSCL_ASSERT(aStatus != PVMFPending);


    int32 sockActivityEvent = aSocketActivity->iEvent;
    PVMFStatus status = aStatus;

    int32 tmplen;
    aSockConfig.iTCPSocket->GetRecvData(&tmplen);

    // If data length is <= zero and aSocketActivity->iEvent is EPVSocketSuccess, it should be treated as failure.
    // On some platforms, its possible to get data length as zero and aSocketActivity->iEvent as EPVSocketSuccess.
    if (EPVSocketSuccess == sockActivityEvent && tmplen <= 0)
    {
        PVMF_SOCKETNODE_LOGINFO((0, "PVMFSocketNode::HandleRecvFromComplete() Sucessful Recv With Zero Length. Treating as failure."));

        sockActivityEvent = EPVSocketFailure;
        status = PVMFFailure;
    }

    //If there's no socket activity input, then this must be a failure in initiating
    //a Recv operation.
    if (!aSocketActivity)
    {
        OSCL_ASSERT(status != PVMFSuccess);
        PVMF_SOCKETNODE_LOGDATATRAFFIC_E((0, "PVMFSocketNode::HandleRecvComplete: Request Failed - SockId=%d, Mime=%s", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
        ReportSocketNodeError(PVMFErrResource, PVMFSocketNodeErrorSocketFailure);
        //release media data
        if (aSockConfig.iPendingRecvMediaData.GetRep())
            aSockConfig.iPendingRecvMediaData.Unbind();
        return;
    }

    //Otherwise this is a result from Oscl Sockets of a Recv call.

    //See whether we can send this data to the connected port
    if (sockActivityEvent == EPVSocketSuccess)
    {
        if (!aSockConfig.iPVMFPort
                || !aSockConfig.iPVMFPort->IsConnected())
        {
            //port already disconnected-- just discard the data
            if (aSockConfig.iPendingRecvMediaData.GetRep())
                aSockConfig.iPendingRecvMediaData.Unbind();
            return;
        }
        if (aSockConfig.iPVMFPort->IsOutgoingQueueBusy())
        {
            //wait on port so we can send recv data.
            StartRecvWaitOnConnectedPort(aSockConfig, *aSocketActivity);
            return;
        }
    }

    //If we get here then it's time to process the recv result.

    //Release media data on failure
    if (status != PVMFSuccess)
    {
        if (aSockConfig.iPendingRecvMediaData.GetRep())
            aSockConfig.iPendingRecvMediaData.Unbind();
    }

    switch (sockActivityEvent)
    {
        case EPVSocketSuccess:
        {
            //Get data length and set media buffer size
            int32 len;
            aSockConfig.iTCPSocket->GetRecvData(&len);
            aSockConfig.iPendingRecvMediaData->setMediaFragFilledLen(0, len);

            // Resize the buffer
            if (aSockConfig.iMemPool)
            {
                OsclSharedPtr<PVMFMediaDataImpl> mediaMsgImpl;
                aSockConfig.iPendingRecvMediaData->getMediaDataImpl(mediaMsgImpl);
                aSockConfig.iMemPool->resizeSocketDataBuffer(mediaMsgImpl);
            }
            else
            {
                PVMF_SOCKETNODE_LOGERROR((0, "PVMFSocketNode::HandleRecvComplete() ERROR:mempool not found"));
                OSCL_ASSERT(0);
                return ;
            }

            // set system time as timestamp
            TimeValue currentTime;
            currentTime.set_to_current_time();
            int32 currentMilliSec = currentTime.to_msec();
            aSockConfig.iPendingRecvMediaData->setTimestamp((PVMFTimestamp)currentMilliSec);

            //queue to next port
            PVMFSharedMediaMsgPtr aMediaMsgPtr;
            convertToPVMFMediaMsg(aMediaMsgPtr, aSockConfig.iPendingRecvMediaData);

#if(ENABLE_SOCKET_NODE_STATS)
            aSockConfig.iPortStats.iNumQueueOutgoingTCPMsg++;
#endif
            PVMFStatus status = aSockConfig.iPVMFPort->QueueOutgoingMsg(aMediaMsgPtr);
            if (status != PVMFSuccess)
            {
                PVMF_SOCKETNODE_LOGDATATRAFFIC_E((0, "PVMFSocketNode::HandleRecvComplete: QueueOutgoingMsg Failed - SockId=%d, Mime=%s", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
                PVMF_SOCKETNODE_LOGERROR((0, "PVMFSocketNode::HandleRecvComplete() ERROR:%d, Outgoing queue size=%d. Data discarded!", status, aSockConfig.iPVMFPort->OutgoingMsgQueueSize()));
                ReportErrorEvent(PVMFErrPortProcessing);
                return ;
            }

            PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFSocketNode::HandleRecvComplete: Request Succcess  - SockId=%d, Mime=%s, Len=%d", aSockConfig.iSockId, aSockConfig.iMime.get_str(), len));
        }
        break;

        case EPVSocketTimeout:
        {
            PVMF_SOCKETNODE_LOGDATATRAFFIC_E((0, "PVMFSocketNode::HandleRecvComplete: Request TimedOut - SockId=%d, Mime=%s", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
            PVMF_SOCKETNODE_LOGERROR((0, "PVMFSocketNode::HandleRecvComplete() ERROR EPVSocketTimeout Ln %d", __LINE__));
            ReportSocketNodeError(PVMFErrTimeout, PVMFSocketNodeErrorSocketTimeOut);
        }
        break;

        case EPVSocketCancel:
            //On some OS we may get EPVSocketCancel for recv failures, so check whether this
            //was response to a cancel command or not.
            if (aRecvOperationCanceled)
            {
                PVMF_SOCKETNODE_LOGDATATRAFFIC_E((0, "PVMFSocketNode::HandleRecvComplete: Request Cancelled - SockId=%d, Mime=%s", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
                PVMF_SOCKETNODE_LOGINFO((0, "PVMFSocketNode::HandleRecvComplete() EPVSocketCancel"));
                break;
            }
            //else fallthrough to the failure processing...
        case EPVSocketFailure:
        {
            //After a receive failure, we may need to do a TCP shutdown.
            //Check what else is currently happening on the port.
            switch (aSockConfig.iState.iSequence)
            {
                case EPVSocketPortSequence_RequestPort:
                case EPVSocketPortSequence_InputConnectMsg:
                case EPVSocketPortSequence_InputDisconnectMsg:
                case EPVSocketPortSequence_SocketCleanup:
                    //some of these conditions may be impossible, but in any case,
                    //it doesn't make sense to start a shutdown sequence during
                    //any of these.  Ignore the error.
                    PVMF_SOCKETNODE_LOGDATATRAFFIC_E((0, "PVMFSocketNode::HandleRecvComplete: Request Failed, ignoring - SockId=%d, Mime=%s", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
                    break;

                case EPVSocketPortSequence_None:
                case EPVSocketPortSequence_InputDataMsg:
                    //for these cases, start a shutdown sequence
                    PVMF_SOCKETNODE_LOGDATATRAFFIC_E((0, "PVMFSocketNode::HandleRecvComplete: Request Failed, starting shutdown sequence - SockId=%d, Mime=%s", aSockConfig.iSockId, aSockConfig.iMime.get_str()));

                    //start the sequence
                    StartSequence(aSockConfig, EPVSocketPortSequence_SocketCleanup);
                    break;

                default:
                    //need code to handle this case.
                    OSCL_ASSERT(0);
                    break;
            }
        }
        break;

        default:
            OSCL_ASSERT(0);
            break;
    }
}

//Handler for a "recv from" complete.  This handles two input cases:
// - Operation failed to initiate.  Status will be in aStatus and aSocketActivity will be NULL.
// - Operation completed asynchronously via HandleSocketEvent callback. Status will be in aStatus
//   and socket callback result will be in aSocketActivity.
//
//The end result of processing will be one of the following:
// - Received data sent to connected port
// - Result queued on socket activity queue for later processing.
// - Error or cancellation processed.
void PVMFSocketNode::HandleRecvFromComplete(SocketPortConfig& aSockConfig, PVMFStatus aStatus, PVMFSocketActivity* aSocketActivity, bool aRecvOperationCanceled)
{
    OSCL_UNUSED_ARG(aStatus);
    PVMF_SOCKETNODE_LOGSTACKTRACE((0, "PVMFSocketNode::HandleRecvFromComplete() In"));

    //operation should be complete when this is called.
    OSCL_ASSERT(aStatus != PVMFPending);

    //If there's no socket activity input, then this must be a failure in initiating
    //a RecvFrom operation.
    if (!aSocketActivity)
    {
        OSCL_ASSERT(aStatus != PVMFSuccess);
        PVMF_SOCKETNODE_LOGDATATRAFFIC_E((0, "PVMFSocketNode::HandleRecvFromComplete: Request Failed - SockId=%d, Mime=%s", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
        ReportSocketNodeError(PVMFErrResource, PVMFSocketNodeErrorSocketFailure);
        //release media data
        if (aSockConfig.iPendingRecvMediaData.GetRep())
            aSockConfig.iPendingRecvMediaData.Unbind();
        return;
    }

    //Otherwise this is a result from Oscl Sockets of a RecvFrom call.

    //Handle "truncated packet" error, in which case we still have data even though
    //the result is EPVSocketFailure.
    int32 dataLen = 0;
    uint8* dataPtr = NULL;
    if (aSocketActivity->iEvent == EPVSocketFailure
            || aSocketActivity->iEvent == EPVSocketSuccess)
    {
        if (aSockConfig.iUDPSocket)
            dataPtr = aSockConfig.iUDPSocket->GetRecvData(&dataLen);
#if(ENABLE_SOCKET_NODE_STATS)
        aSockConfig.iPortStats.iNumRecvFromPackets += aSockConfig.iRecvFromPacketLen.size();
        if (aSockConfig.iRecvFromPacketLen.size() > aSockConfig.iPortStats.iMaxRecvFromPackets)
            aSockConfig.iPortStats.iMaxRecvFromPackets = aSockConfig.iRecvFromPacketLen.size();
#endif
    }

    //Handle cases where we got some data
    if (dataLen > 0)
    {
        //See if we can pass this data along to the connected port.  If not, then queue the
        //result for later processing.
        if (aSockConfig.iPVMFPort->IsOutgoingQueueBusy())
        {
            //outgoing queue is busy-- must queue this for later.
            StartRecvWaitOnConnectedPort(aSockConfig, *aSocketActivity);
        }
        else
        {
            //report truncated packet.
            if (aSocketActivity->iEvent == EPVSocketFailure)
            {
                PVUuid eventuuid = PVMFSocketNodeEventTypeUUID;
                int32 infocode = PVMFSocketNodeInfoEventPacketTruncated;
                ReportInfoEvent(PVMFErrCorrupt, NULL, &eventuuid, &infocode);
            }

            //set the media buffer size.
            aSockConfig.iPendingRecvMediaData->setMediaFragFilledLen(0, dataLen);

            //resize the buffer.
            if (aSockConfig.iMemPool)
            {
                OsclSharedPtr<PVMFMediaDataImpl> mediaMsgImpl;
                aSockConfig.iPendingRecvMediaData->getMediaDataImpl(mediaMsgImpl);
                aSockConfig.iMemPool->resizeSocketDataBuffer(mediaMsgImpl);
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFSocketNode::HandleRecvFromComplete() ERROR:mempool not found"));
                PVMF_SOCKETNODE_LOGDATATRAFFIC_E((0, "PVMFSocketNode::HandleRecvFromComplete: ERROR mempool not found - SockId=%d, Mime=%s", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
                return;//unexpected, cleanup
            }

#if SNODE_ENABLE_UDP_MULTI_PACKET
            //The media buffer may contain multiple UDP packets.  We need to
            //re-package these into a media frag group.

            //Allocate a new media frag group.
            int32 err;
            OsclSharedPtr< PVMFMediaDataImpl > mediaFragGroup;
            err = Allocate(aSockConfig, mediaFragGroup);
            if (err != OsclErrNone)
            {
                //Unexpected error.  With current usage by JB node, this pool should
                //never run out.
                PVMF_SOCKETNODE_LOGDATATRAFFIC_E((0, "PVMFSocketNode::HandleRecvFromComplete: Unexpected mempool error - SockId=%d, Mime=%s", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
                ReportSocketNodeError(PVMFErrResource, PVMFSocketNodeErrorSocketFailure);
                //release media data
                if (aSockConfig.iPendingRecvMediaData.GetRep())
                    aSockConfig.iPendingRecvMediaData.Unbind();
                return;
            }

            //get the single frag data buffer pointer.
            OsclRefCounterMemFrag singleFrag;
            aSockConfig.iPendingRecvMediaData->getMediaFragment(0, singleFrag);

            for (uint32 i = 0;i < aSockConfig.iRecvFromPacketLen.size();dataPtr += aSockConfig.iRecvFromPacketLen[i++])
            {
                PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFSocketNode::HandleRecvFromComplete: Fragment %d Len %d - SockId=%d, Mime=%s"
                                                  , i, aSockConfig.iRecvFromPacketLen[i], aSockConfig.iSockId, aSockConfig.iMime.get_str()));
                //Create a new fragment with the existing refcount
                //and the proper packet boundaries.
                OsclRefCounterMemFrag multiFrag(singleFrag);
                //now adjust the single frag to the actual packet boundary.
                multiFrag.getMemFrag().ptr = dataPtr;
                multiFrag.getMemFrag().len = aSockConfig.iRecvFromPacketLen[i];
                mediaFragGroup->appendMediaFragment(multiFrag);
#if (PVLOGGER_INST_LEVEL > PVLOGMSG_INST_LLDBG)
                if (aSockConfig.iRTP)
                {
                    LogRTPHeaderFields(aSockConfig, multiFrag);
                }
                else if (aSockConfig.iRTCP)
                {
                    LogRTCPHeaderFields(aSockConfig, multiFrag);
                }
#endif
            }

            //Replace the original media data message with the new one.
            //since we've linked the new frags to the original refcounter, we
            //can just unbind the current message then re-allocate it with the
            //new data.
            aSockConfig.iPendingRecvMediaData.Unbind();
            err = CreateMediaData(aSockConfig, mediaFragGroup);
            if (err != OsclErrNone)
            {
                //unexpected since we just freed one message
                PVMF_SOCKETNODE_LOGDATATRAFFIC_E((0, "PVMFSocketNode::HandleRecvFromComplete: Unexpected mempool error - SockId=%d, Mime=%s", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
                ReportSocketNodeError(PVMFErrResource, PVMFSocketNodeErrorSocketFailure);
                return;
            }

#else //SNODE_ENABLE_UDP_MULTI_PACKET

#if (PVLOGGER_INST_LEVEL > PVLOGMSG_INST_LLDBG)
            if (aSockConfig.iRTP)
            {
                OsclRefCounterMemFrag memfrag;
                aSockConfig.iPendingRecvMediaData->getMediaFragment(0, memfrag);
                LogRTPHeaderFields(aSockConfig, memfrag);
            }
            else if (aSockConfig.iRTCP)
            {
                OsclRefCounterMemFrag memfrag;
                aSockConfig.iPendingRecvMediaData->getMediaFragment(0, memfrag);
                LogRTCPHeaderFields(aSockConfig, memfrag);
            }
#endif

#endif //SNODE_ENABLE_UDP_MULTI_PACKET

            //push the received data to the connected port
            PVMFSharedMediaMsgPtr mediaMsgPtr;
            convertToPVMFMediaMsg(mediaMsgPtr, aSockConfig.iPendingRecvMediaData);

#if(ENABLE_SOCKET_NODE_STATS)
            aSockConfig.iPortStats.iNumQueueOutgoingUDPMsg++;
#endif
            PVMFStatus status = aSockConfig.iPVMFPort->QueueOutgoingMsg(mediaMsgPtr);
            if (status != PVMFSuccess)
            {
                //should never get here because we already checked outgoign queue earlier.
                PVMF_SOCKETNODE_LOGERROR((0, "PVMFSocketNode::HandleRecvFromComplete() ERROR:%d, Outgoing queue size=%d. Data discarded!", status, aSockConfig.iPVMFPort->OutgoingMsgQueueSize()));
                ReportErrorEvent(PVMFInfoOverflow);
            }

            PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFSocketNode::HandleRecvFromComplete: Request Succcess - SockId=%d, Mime=%s, Len=%d", aSockConfig.iSockId, aSockConfig.iMime.get_str(), dataLen));
        }
    }
    else
    {
        //handle error, cancel, and timeout cases.

        //release media data.
        if (aSockConfig.iPendingRecvMediaData.GetRep())
            aSockConfig.iPendingRecvMediaData.Unbind();

        //report error events.
        switch (aSocketActivity->iEvent)
        {
            case EPVSocketCancel:
                //On some OS we may get EPVSocketCancel for recv failures, so check whether this
                //was response to a cancel command or not.
                if (aRecvOperationCanceled)
                {
                    PVMF_SOCKETNODE_LOGDATATRAFFIC_E((0, "PVMFSocketNode::HandleRecvFromComplete: Request Cancelled - SockId=%d, Mime=%s", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
                    break;
                }
                //else fallthrough to failure processing...
            case EPVSocketFailure:
                //report RTP failures.
                //Note: do not report RTCP errors, because some servers send zero-byte packets
                //and we want to be tolerant of that case.
                if (aSockConfig.iTag == PVMF_SOCKET_NODE_PORT_TYPE_SOURCE)
                {
                    PVMF_SOCKETNODE_LOGDATATRAFFIC_E((0, "PVMFSocketNode::HandleRecvFromComplete: Request Failed - SockId=%d, Mime=%s", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
                    ReportSocketNodeError(PVMFErrResource, PVMFSocketNodeErrorSocketFailure);
                }
                break;

            case EPVSocketTimeout:
            {
                PVMF_SOCKETNODE_LOGDATATRAFFIC_E((0, "PVMFSocketNode::HandleRecvFromComplete: Request TimedOut - SockId=%d, Mime=%s", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
                PVMF_SOCKETNODE_LOGERROR((0, "PVMFSocketNode::HandleRecvFromComplete() ERROR EPVSocketTimeout Ln %d", __LINE__));
                ReportSocketNodeError(PVMFErrTimeout, PVMFSocketNodeErrorSocketTimeOut);
            }
            break;

            default:
                break;
        }
    }
}

//////////////////////////////////
// End Receive Operation Handlers
//////////////////////////////////


//////////////////////////////////
// Send Operation Handlers
//////////////////////////////////


//Start a send operation on the port's socket.
//Assume the caller already verified that the port can send now.
//If there is a failure, this routine will call the "send operation complete" routine to
//do error handling or continue to the next state.
PVMFStatus PVMFSocketNode::StartSendOperation(SocketPortConfig& aSockConfig, PVMFSharedMediaMsgPtr& aMsg)
{
    PVMF_SOCKETNODE_LOGSTACKTRACE((0, "PVMFSocketNode::StartSendOperation() In"));

    //caller should have verified we can send now.
    OSCL_ASSERT(aSockConfig.iState.iSendOperation == EPVSocketPortSendOperation_None);

    //caller should provide a media data message as input
    OSCL_ASSERT(aMsg->getFormatID() == PVMF_MEDIA_MSG_DATA_FORMAT_ID);

    //there should be either a UDP or TCP socket on this port
    if (!aSockConfig.iTCPSocket && !aSockConfig.iUDPSocket)
    {
        PVMF_SOCKETNODE_LOGDATATRAFFIC_E((0, "PVMFSocketNode::StartSendOperation ERROR no socket - SockId=%d, Mime=%s", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
        return PVMFFailure;
    }

    aSockConfig.iPVMFPort->iNumFramesConsumed++;

    // Retrieve memory fragment to write to
    if (aSockConfig.iPendingSendMediaData.GetRep())
        aSockConfig.iPendingSendMediaData.Unbind();

    convertToPVMFMediaData(aSockConfig.iPendingSendMediaData, aMsg);
    OsclRefCounterMemFrag refCtrMemFragOut;
    aSockConfig.iPendingSendMediaData->getMediaFragment(0, refCtrMemFragOut);
    PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFSocketNode::StartSendOperation - SockId=%d, Mime=%s, Len=%d", aSockConfig.iSockId, aSockConfig.iMime.get_str(), refCtrMemFragOut.getMemFragSize()));

    PVMFStatus status = PVMFFailure;

    if (aSockConfig.iUDPSocket)
    {
        aSockConfig.iState.iSendOperation = EPVSocketPortSendOperation_SendTo;

#if(ENABLE_SOCKET_NODE_STATS)
        aSockConfig.iPortStats.iNumSendTo++;
        aSockConfig.iPortStats.StartSendTime(aSockConfig.iState.iSendOperation);
#endif
        TPVSocketEvent retVal = aSockConfig.iUDPSocket->SendTo((uint8*)refCtrMemFragOut.getMemFragPtr()
                                , refCtrMemFragOut.getMemFragSize()
                                , aSockConfig.iAddr.iRemoteAdd
                                , TIMEOUT_SENDTO);

        if (retVal == EPVSocketPending)
            status = PVMFPending;//wait on HandleSocketEvent callback
        else
            status = PVMFFailure;

        if (PVMFPending != status)
        {
            //error
            PVMF_SOCKETNODE_LOGERROR((0, "0x%x PVMFSocketNode::SendTo() failed retVal=%d Ln %d", this, retVal, __LINE__));
            PVMF_SOCKETNODE_LOGERROR((0, "PVMFSocketNode::StartSendOperation: UDP - Error. status=%d", status));
            PVMF_SOCKETNODE_LOGDATATRAFFIC_E((0, "PVMFSocketNode::StartSendOperation SendTo FAILED - SockId=%d, Mime=%s", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
        }
        else
        {
            PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFSocketNode::StartSendOperation SendTo Pending - SockId=%d, Mime=%s", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
        }
    }

    else if (aSockConfig.iTCPSocket)
    {
        aSockConfig.iState.iSendOperation = EPVSocketPortSendOperation_Send;

#if(ENABLE_SOCKET_NODE_STATS)
        aSockConfig.iPortStats.iNumSend++;
        aSockConfig.iPortStats.StartSendTime(aSockConfig.iState.iSendOperation);
#endif
        TPVSocketEvent retVal = aSockConfig.iTCPSocket->Send((uint8*)refCtrMemFragOut.getMemFragPtr(),
                                refCtrMemFragOut.getMemFragSize(),
                                TIMEOUT_SEND);

        if (retVal == EPVSocketPending)
            status = PVMFPending;//wait on HandleSocketEvent callback
        else
            status = PVMFFailure;

        if (PVMFPending != status)
        {
            PVMF_SOCKETNODE_LOGERROR((0, "PVMFSocketNode::StartSendOperation: TCP - Error. status=%d", status));
            PVMF_SOCKETNODE_LOGDATATRAFFIC_E((0, "PVMFSocketNode::StartSendOperation Calling Send FAILED - SockId=%d, Mime=%s", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
        }
        else
        {
            PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFSocketNode::StartSendOperation Send Pending - SockId=%d, Mime=%s", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
        }
    }

    //Handle synchronous completion or failures
    if (status != PVMFPending)
        status = SendOperationComplete(aSockConfig, status, NULL);

    return status;
}

//The current send operation is complete and status is in "aStatus".
//If there was a socket callback, the result is in "aSocketActivity".
//Process the result and continue with the next operation if needed.
PVMFStatus PVMFSocketNode::SendOperationComplete(SocketPortConfig& aSockConfig, PVMFStatus aStatus, PVMFSocketActivity* aSocketActivity)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFSocketNode::SendOperationComplete()"));

    OSCL_ASSERT(aStatus != PVMFPending);

    PVMFStatus status = aStatus;

    //Update the send state
    TPVSocketPortSendOperation curOp = aSockConfig.iState.iSendOperation;
    aSockConfig.iState.iSendOperation = EPVSocketPortSendOperation_None;
    aSockConfig.iState.iSendOperationStatus = aStatus;
    aSockConfig.iState.iSendOperationCanceled = false;

    //Release the media buffer after each send operation, regardless of success/fail.
    if (aSockConfig.iPendingSendMediaData.GetRep() != NULL)
        aSockConfig.iPendingSendMediaData.Unbind();

    if (aSocketActivity)
    {
        //Datapath logging
        switch (aSocketActivity->iEvent)
        {
            case EPVSocketSuccess:
                PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFSocketNode::SendOperationComplete - Success - SockId=%d, Mime=%s", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
                break;
            case EPVSocketTimeout:
                PVMF_SOCKETNODE_LOGDATATRAFFIC_E((0, "PVMFSocketNode::SendOperationComplete - TimeOut - SockId=%d, Mime=%s", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
                break;
            case EPVSocketFailure:
                PVMF_SOCKETNODE_LOGDATATRAFFIC_E((0, "PVMFSocketNode::SendOperationComplete - Failed - SockId=%d, Mime=%s", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
                break;
            case EPVSocketCancel:
                PVMF_SOCKETNODE_LOGDATATRAFFIC_E((0, "PVMFSocketNode::SendOperationComplete - Cancelled - SockId=%d, Mime=%s", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
                break;
            default:
                OSCL_ASSERT(0);
                break;
        }
    }

    //report TCP errors.
    if (aStatus != PVMFSuccess
            && curOp == EPVSocketPortSendOperation_Send)
    {
        PVMF_SOCKETNODE_LOGDATATRAFFIC_E((0, "PVMFSocketNode::SendOperationComplete - Send Failed - "
                                          "SockId=%d, Mime=%s, Operaton=%d",
                                          aSockConfig.iSockId, aSockConfig.iMime.get_str(), aSockConfig.iState.iSendOperation));
        PVMF_SOCKETNODE_LOGERROR((0, "PVMFSocketNode::SendOperationComplete: Send/SendTo() ERROR status=%d, Ln %d", status, __LINE__));
        ReportSocketNodeError(PVMFErrResource, PVMFSocketNodeErrorTCPSocketSendError);
    }

    //This completes an input data message sequence
    if (aSockConfig.iState.iSequence == EPVSocketPortSequence_InputDataMsg)
        SequenceComplete(aSockConfig, aStatus);

    //Detect connect "Wait on send/recv idle" complete
    if (aSockConfig.iState.iConnectOperation == EPVSocketPortConnectOperation_WaitOnSendRecvIdle
            && aSockConfig.iState.iRecvOperation == EPVSocketPortRecvOperation_None
            && aSockConfig.iState.iSendOperation == EPVSocketPortSendOperation_None)
    {
        ConnectOperationComplete(aSockConfig, PVMFSuccess, NULL);
    }

    return status;
}

//Cancel the current send operation.
//Return Success if the operation is canceled, or Pending if we need to wait.
PVMFStatus PVMFSocketNode::CancelSendOperation(SocketPortConfig& aSockConfig)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFSocketNode::CancelSendOperation()"));

    PVMFStatus status = PVMFSuccess;

    switch (aSockConfig.iState.iSendOperation)
    {
        case EPVSocketPortSendOperation_None:
            break;

        case EPVSocketPortSendOperation_Send:
            if (aSockConfig.iTCPSocket)
            {
                if (!aSockConfig.iState.iSendOperationCanceled)
                {
                    aSockConfig.iState.iSendOperationCanceled = true;
                    PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFSocketNode::CancelSendOperation Calling CancelSend - SockId=%d, Mime=%s", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
                    aSockConfig.iTCPSocket->CancelSend();
                }
                status = PVMFPending;
                //wait on send to complete in HandleSocketEvent
            }
            break;

        case EPVSocketPortSendOperation_SendTo:
            if (aSockConfig.iUDPSocket)
            {
                if (!aSockConfig.iState.iSendOperationCanceled)
                {
                    aSockConfig.iState.iSendOperationCanceled = true;
                    PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFSocketNode::CancelSendOperation Calling CancelSendTo - SockId=%d, Mime=%s", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
                    aSockConfig.iUDPSocket->CancelSendTo();
                }
                status = PVMFPending;
                //wait on sendto to complete in HandleSocketEvent
            }
            break;

        default:
            OSCL_ASSERT(0);
            status = PVMFFailure;
            break;
    }
    return status;
}

//////////////////////////////////
// End Send Operation Handlers
//////////////////////////////////

//////////////////////////////////////
// Sequence Handler
//////////////////////////////////////
static const char* const TPVSocketPortSequenceStr[] =
{
    "EPVSocketPortSequence_None"
    , "EPVSocketPortSequence_RequestPort"
    , "EPVSocketPortSequence_InputConnectMsg"
    , "EPVSocketPortSequence_InputDataMsg"
    , "EPVSocketPortSequence_InputDisconnectMsg"
    , "EPVSocketPortSequence_SocketCleanup"
    , "EPVSocketPortSequence_Last"
} ;

//Start a sequence on a port.
//This routine will complete all steps possible unless some operation has
//asynchronous completion.
//This call may actually complete the sequence.
//This routine documents the series of steps for each sequence.
PVMFStatus PVMFSocketNode::StartSequence(SocketPortConfig& aSockConfig, TPVSocketPortSequence aSequence, OsclAny* aParam)
{
    PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFSocketNode::StartSequence, '%s' - SockId=%d, Mime=%s"
                                      , TPVSocketPortSequenceStr[aSequence]
                                      , aSockConfig.iSockId, aSockConfig.iMime.get_cstr()));

    //Set the sequence.
    aSockConfig.iState.iSequence = aSequence;

    switch (aSequence)
    {
        case EPVSocketPortSequence_RequestPort:
            //Sequence:
            // 1. GetHostByName,
            // 2. Connect
            // 3. Command complete.

            return StartConnectOperation(aSockConfig, EPVSocketPortConnectOperation_GetHostByName);

        case EPVSocketPortSequence_InputConnectMsg:
            //Sequence:
            // If already connected,
            // 1. Shutdown
            // 2. Cancel send/recv operation if needed.
            // 3. Wait for send/recv idle if needed.
            // 4. Wait for connected port if needed.
            // 5. Close socket + send EOS to connected port + delete socket
            //      or add to cleanup queue & trigger AO.
            // 6. Create
            // 7. GetHostByName
            // 8. Connect
            // Else if not already connected,
            // 1. GetHostByName
            // 2. Connect

            if (aSockConfig.iTCPSocket)
            {
                PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFSocketNode::StartSequence - Disconnecting before Connect SockId=%d, Mime=%s", aSockConfig.iSockId, aSockConfig.iMime.get_cstr()));
                return StartConnectOperation(aSockConfig, EPVSocketPortConnectOperation_Shutdown);
            }
            else
            {
                return StartConnectOperation(aSockConfig, EPVSocketPortConnectOperation_GetHostByName);
            }
            break;

        case EPVSocketPortSequence_InputDataMsg:
            //Sequence:
            // 1. Send

            OSCL_ASSERT(aParam);
            return StartSendOperation(aSockConfig, *((PVMFSharedMediaMsgPtr*)aParam));

        case EPVSocketPortSequence_InputDisconnectMsg:
            //Sequence:
            // 1. Shutdown
            // 2. Cancel send/recv operation if needed.
            // 3. Wait for send/recv idle if needed.
            // 4. Wait for connected port if needed.
            // 5. Close socket + send EOS to connected port + delete socket
            //      or add to cleanup queue & trigger AO.

            return StartConnectOperation(aSockConfig, EPVSocketPortConnectOperation_Shutdown);

        case EPVSocketPortSequence_SocketCleanup:
            if (aSockConfig.iTCPSocket)
            {
                //TCP Shutdown Sequence:
                // 1. Cancel current connect operation if needed.
                // 2. Cancel send/recv operation if needed.
                // 3. Shutdown
                // 4. Wait for send/recv idle if needed.
                // 5. Wait for connected port if needed.
                // 6. Close socket + send EOS to connected port + delete socket
                //      or add to cleanup queue & trigger AO.

                if (aSockConfig.iState.iConnectOperation == EPVSocketPortConnectOperation_Shutdown)
                {
                    return PVMFPending;//wait on completion in HandleSocketEvent
                }
                if (CancelConnectOperation(aSockConfig) == PVMFPending)
                {
                    return PVMFPending;//wait on connect operation to cancel
                    //(in HandleSocketEvent or HandleDNSEvent)
                }
                else
                {
                    //Go ahead and start any send/recv cancel since they can happen simultaneously
                    //with the Shutdown operation.
                    CancelRecvOperation(aSockConfig);
                    CancelSendOperation(aSockConfig);

                    return StartConnectOperation(aSockConfig, EPVSocketPortConnectOperation_Shutdown);
                }
            }
            else if (aSockConfig.iUDPSocket)
            {
                //UDP Shutdown Sequence:
                // 1. Cancel all current operations
                // 2. Wait for send/recv idle if needed.
                // 3. Close socket + delete socket or add to cleanup queue & trigger AO.

                switch (aSockConfig.iState.iConnectOperation)
                {
                    case EPVSocketPortConnectOperation_WaitOnSendRecvIdle:
                        //keep waiting for completion (in RecvOperationComplete or SendOperationComplete)
                        return PVMFPending;

                    case EPVSocketPortConnectOperation_None:

                        CancelRecvOperation(aSockConfig);
                        CancelSendOperation(aSockConfig);

                        return StartConnectOperation(aSockConfig, EPVSocketPortConnectOperation_WaitOnSendRecvIdle);

                    default:
                        //unexpected
                        OSCL_ASSERT(0);
                        return PVMFFailure;

                }
            }
            //else
            {
                //nothing needed.
                SequenceComplete(aSockConfig, PVMFSuccess);
                return PVMFSuccess;
            }
            break;

        default:
            //add code to handle this case
            OSCL_ASSERT(0);
            return PVMFFailure;
    }
}

//The current multi-operation sequence is complete.
//Process the result and continue with the next operation if needed.
void PVMFSocketNode::SequenceComplete(SocketPortConfig& aSockConfig, PVMFStatus aStatus)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFSocketNode::SequenceComplete() Sequence %d Status %d ", aSockConfig.iState.iSequence, aStatus));

    PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFSocketNode::SequenceComplete '%s' - SockId=%d, Mime=%s"
                                      , TPVSocketPortSequenceStr[aSockConfig.iState.iSequence]
                                      , aSockConfig.iSockId, aSockConfig.iMime.get_str()));

    //Update the state
    TPVSocketPortSequence curSequence = aSockConfig.iState.iSequence;
    aSockConfig.iState.iSequence = EPVSocketPortSequence_None;
    aSockConfig.iState.iSequenceStatus = aStatus;

    switch (curSequence)
    {
        case EPVSocketPortSequence_RequestPort:
            //may need to complete the node command
            if (iCurrentCmdQueue.size()
                    && iCurrentCmdQueue.front().iCmd == PVMF_GENERIC_NODE_REQUESTPORT)
            {
                //@TODO would be nice to cleanup the allocated port in case of a failure.
                CommandComplete(iCurrentCmdQueue, iCurrentCmdQueue.front(), aStatus, iRequestedPort);
            }
            break;

        case EPVSocketPortSequence_InputConnectMsg:
        case EPVSocketPortSequence_InputDisconnectMsg:
        case EPVSocketPortSequence_InputDataMsg:
            //If we've just completed an input port message, this may complete a node Flush command
            if (!iCurrentCmdQueue.empty()
                    && iCurrentCmdQueue.front().iCmd == PVMF_GENERIC_NODE_FLUSH)
            {
                //Flush is done when all input ports are empty.
                for (uint32 i = 0;i < iPortVector.size();i++)
                {
                    if (iPortVector[i]->IncomingMsgQueueSize() > 0)
                        return;//keep waiting
                }
                //All ports empty-- complete the Flush.
                CommandComplete(iCurrentCmdQueue, iCurrentCmdQueue.front(), PVMFSuccess);
            }
            break;


        case EPVSocketPortSequence_SocketCleanup:
            //When socket cleanup is complete, decrement the "stop node activity"
            //counter and look for completion of all ports.

            //This port is done-- decrement the counter
            iNumStopPortActivityPending--;

            //When counter reaches zero, all ports are done and the sequence is complete.
            if (iNumStopPortActivityPending == 0)
            {
                //Reset the counter to the "idle" value.
                iNumStopPortActivityPending = (-1);

                //There may be a Reset or CancelAll command waiting on
                //this to complete.
                if (!iCurrentCmdQueue.empty()
                        && iCurrentCmdQueue.front().iCmd == PVMF_GENERIC_NODE_RESET)
                {
                    CommandComplete(iCurrentCmdQueue, iCurrentCmdQueue.front(), PVMFSuccess);
                }
                else if (!iCancelCmdQueue.empty()
                         && iCancelCmdQueue.front().iCmd == PVMF_GENERIC_NODE_CANCELALLCOMMANDS)
                {
                    CommandComplete(iCancelCmdQueue, iCancelCmdQueue.front(), PVMFSuccess);
                }
                //ports may be cleaned up now, so exit this routine now to
                //avoid a crash from further port processing.
                return;
            }
            break;

        default:
            break;
    }


    //All sequences other than input data message will
    //block receive operations, so may need to start or resume receives now.
    if (curSequence != EPVSocketPortSequence_InputDataMsg
            && (aStatus == PVMFSuccess)
            && CanReceive(aSockConfig))
    {
        StartRecvOperation(aSockConfig);
    }

    //Input message processing may have been blocked waiting on
    //this sequence to complete, so resume now.
    if (CanProcessIncomingMsg(aSockConfig)
            && (aStatus == PVMFSuccess))
    {
        ProcessIncomingMsg(aSockConfig);
    }
}

//////////////////////////////////////
// End Sequence Handler
//////////////////////////////////////


//////////////////////////////////////
// Command Handlers
//////////////////////////////////////

//See if we can process a new input command right now.
bool PVMFSocketNode::CanProcessCommand()
{
    //We may need to wait on
    //1) a new command to arrive
    //2) some prior command to complete
    //Note: this allows a Cancel to interrupt the current command, but it does
    //not allow a Cancel to interrupt another cancel.
    return (!iPendingCmdQueue.empty()
            && (iCurrentCmdQueue.empty()
                || (iPendingCmdQueue.front().hipri() && iCancelCmdQueue.empty())));
}

//Process an input command.
void PVMFSocketNode::ProcessCommand(PVMFSocketNodeCmdQ& aCmdQ, PVMFSocketNodeCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFSocketNodeCommand::ProcessCommand() in"));

    PVMFStatus status = PVMFFailure;
    iCommandErrorCode = PVMFSocketNodeErrorEventStart; //no error

    if (aCmd.hipri())
    {
        //calling logic should prevent multiple cancels in progress.
        OSCL_ASSERT(iCancelCmdQueue.empty());

        switch (aCmd.iCmd)
        {
            case PVMF_GENERIC_NODE_CANCELALLCOMMANDS:
                status = DoCancelAllCommands(aCmd);
                break;

            case PVMF_GENERIC_NODE_CANCELCOMMAND:
                status = DoCancelCommand(aCmd);
                break;

            default://unknown command type
                status = PVMFFailure;
                break;
        }

        //If the command was not finished in this call, then move it to the
        //"cancel cmd" queue where it will remain until it is completed.
        if (status == PVMFPending)
        {
            //move the new cmd to the "cancel cmd" queue where it will
            //remain until complete.
            int32 err;
            OSCL_TRY(err, iCancelCmdQueue.StoreL(aCmd););

            //we reserved space in the CancelCmdQueue already, so
            //it should not be possible to fail here.
            OSCL_ASSERT(err == OsclErrNone);

            //erase the original command
            aCmdQ.Erase(&aCmd);
        }
        else
        {
            CommandComplete(aCmdQ, aCmd, status, NULL);
        }
    }
    else
    {
        //calling logic should prevent multiple commands in progress.
        OSCL_ASSERT(iCurrentCmdQueue.empty());

        OsclAny* eventData = NULL;

        switch (aCmd.iCmd)
        {
            case PVMF_GENERIC_NODE_QUERYUUID:
                status = DoQueryUuid(aCmd);
                break;

            case PVMF_GENERIC_NODE_QUERYINTERFACE:
                status = DoQueryInterface(aCmd);
                break;

            case PVMF_GENERIC_NODE_REQUESTPORT:
            {
                iRequestedPort = NULL;
                status = DoRequestPort(aCmd, iRequestedPort);
                eventData = iRequestedPort;
                break;
            }

            case PVMF_GENERIC_NODE_RELEASEPORT:
                status = DoReleasePort(aCmd);
                break;

            case PVMF_GENERIC_NODE_INIT:
                status = DoInit(aCmd);
                break;

            case PVMF_GENERIC_NODE_PREPARE:
                status = DoPrepare(aCmd);
                if (status == PVMFSuccess)
                {
                    ChangeExternalState(EPVMFNodePrepared);
                }
                break;

            case PVMF_GENERIC_NODE_START:
                status = DoStart(aCmd);
                break;

            case PVMF_GENERIC_NODE_STOP:
                status = DoStop(aCmd);
                break;

            case PVMF_GENERIC_NODE_FLUSH:
                status = DoFlush(aCmd);
                break;

            case PVMF_GENERIC_NODE_PAUSE:
                status = DoPause(aCmd);
                break;

            case PVMF_GENERIC_NODE_RESET:
                status = DoReset(aCmd);
                break;

            default://unknown command type
                status = PVMFFailure;
                break;
        }

        //If the command was not finished in this call, then move it to the
        //"current cmd" queue where it will remain until it is completed.
        if (status == PVMFPending)
        {
            //move the new cmd to the "current cmd" queue where it will
            //remain until complete.
            int32 err;
            OSCL_TRY(err, iCurrentCmdQueue.StoreL(aCmd););

            //we reserved space in the CurrentCmdQueue already, so
            //it should not be possible to fail here.
            OSCL_ASSERT(err == OsclErrNone);

            //erase the original command
            aCmdQ.Erase(&aCmd);
        }
        else
        {
            CommandComplete(aCmdQ, aCmd, status, eventData);
        }
    }
}

//Called to complete a node command.
void PVMFSocketNode::CommandComplete(PVMFSocketNodeCmdQ& aCmdQ,
                                     PVMFSocketNodeCommand& aCmd,
                                     PVMFStatus aStatus,
                                     OsclAny* aEventData,
                                     PVUuid* aEventUUID,
                                     int32* aEventCode)

{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFSocketNode:CommandComplete Id %d Cmd %d Status %d Context %d Data %d"
                    , aCmd.iId, aCmd.iCmd, aStatus, aCmd.iContext, aEventData));

    //See whether command processing was blocked on entry to this routine, so we can
    //decide later whether it's necessary to trigger the AO to resume processsing.
    bool canProcess = CanProcessCommand();

    PVMFStatus status = aStatus;

    //do standard state changes & other cmd completion.
    if (aStatus == PVMFSuccess)
    {
        switch (aCmd.iCmd)
        {
            case PVMF_GENERIC_NODE_INIT:
                ChangeExternalState(EPVMFNodeInitialized);
                break;

            case PVMF_GENERIC_NODE_PREPARE:
                ChangeExternalState(EPVMFNodePrepared);
                break;

            case PVMF_GENERIC_NODE_START:
                ChangeExternalState(EPVMFNodeStarted);
                break;

            case PVMF_GENERIC_NODE_STOP:
                ChangeExternalState(EPVMFNodePrepared);

#if(ENABLE_SOCKET_NODE_STATS)
                //dump the stats.
                iSocketNodeStats.Log(iPortVector);
#endif
                break;

            case PVMF_GENERIC_NODE_PAUSE:
                ChangeExternalState(EPVMFNodePaused);
                break;

            case PVMF_GENERIC_NODE_RESET:
                ChangeExternalState(EPVMFNodeCreated);

                //Complete the reset command.
                {
#if(ENABLE_SOCKET_NODE_STATS)
                    iSocketNodeStats.Log(iPortVector);
#endif
                    //cleanup all ports.
                    CleanupPorts();
                    //go back to Idle state
                    iPortVector.Reconstruct();
                    SetState(EPVMFNodeIdle);
                }
                break;

            case PVMF_GENERIC_NODE_CANCELALLCOMMANDS:
                //Complete the reset command.
            {
#if(ENABLE_SOCKET_NODE_STATS)
                iSocketNodeStats.Log(iPortVector);
#endif
                //Since "cancel all" is effectively used as a Reset,
                //go ahead and cleanup all ports.
                CleanupPorts();
            }
            break;
            default:
                break;
        }
    }

    //create extended error message if an explicit error was provided in args
    PVInterface* extif = NULL;
    PVMFBasicErrorInfoMessage* errormsg = NULL;
    if (aEventUUID && aEventCode)
    {
        errormsg = OSCL_NEW(PVMFBasicErrorInfoMessage, (*aEventCode, *aEventUUID, NULL));
        extif = OSCL_STATIC_CAST(PVInterface*, errormsg);
    }
    //else if no error input, see if "command error code" was set.
    else if (iCommandErrorCode != PVMFSocketNodeErrorEventStart)
    {
        PVUuid eventuuid = PVMFSocketNodeEventTypeUUID;
        errormsg = OSCL_NEW(PVMFBasicErrorInfoMessage, (iCommandErrorCode, eventuuid, NULL));
        extif = OSCL_STATIC_CAST(PVInterface*, errormsg);
    }

    //create response
    PVMFCmdResp resp(aCmd.iId, aCmd.iContext, status, extif, aEventData);
    PVMFSessionId session = aCmd.iSession;

    //Erase the command from the queue.
    aCmdQ.Erase(&aCmd);

    //Report completion to the session observer.
    ReportCmdCompleteEvent(session, resp);

    if (errormsg)
    {
        errormsg->removeRef();
    }
    iCommandErrorCode = PVMFSocketNodeErrorEventStart;//reset.

    //See if there was a pending cancel waiting on the current command to
    //complete.
    if (!iCancelCmdQueue.empty()
            && iCancelCmdQueue.front().iCmd == PVMF_GENERIC_NODE_CANCELCOMMAND)
    {
        CommandComplete(iCancelCmdQueue, iCancelCmdQueue.front(), PVMFSuccess);
    }

    //May need to resume command handling if the AO was blocked
    //waiting on asynchronous command completion, but it's unblocked now.
    if (!canProcess
            && CanProcessCommand()
            && IsAdded())
    {
        RunIfNotReady();
    }
}

void PVMFSocketNode::ReportErrorEvent(PVMFEventType aEventType,
                                      OsclAny* aEventData,
                                      PVUuid* aEventUUID,
                                      int32* aEventCode)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_ERR,
                    (0, "PVMFSocketNode:NodeErrorEvent Type %d Data %d"
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

void PVMFSocketNode::ReportInfoEvent(PVMFEventType aEventType,
                                     OsclAny* aEventData,
                                     PVUuid* aEventUUID,
                                     int32* aEventCode)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFSocketNode:NodeInfoEvent Type %d Data %d"
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

PVMFCommandId PVMFSocketNode::QueueCommandL(PVMFSocketNodeCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFSocketNode::QueueCommandL()"));

#if(ENABLE_SOCKET_NODE_STATS)
    iSocketNodeStats.iNumQueueNodeCmd++;
#endif

    PVMFCommandId id;

    id = iPendingCmdQueue.AddL(aCmd);

    //This may be a processing trigger.
    //Wakeup the AO if needed.
    if (IsAdded()
            && CanProcessCommand())
    {
        RunIfNotReady();
    }

    return id;
}

PVMFStatus PVMFSocketNode::DoReset(PVMFSocketNodeCommand& aCmd)
{
    OSCL_UNUSED_ARG(aCmd);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFSocketNode::DoReset()"));

    /* This node allows a reset from any state */

    return DoStopNodeActivity();
}

PVMFStatus PVMFSocketNode::DoQueryUuid(PVMFSocketNodeCommand& aCmd)
{
    //This node supports Query UUID from any state

    OSCL_String* mimetype;
    Oscl_Vector<PVUuid, OsclMemAllocator> *uuidvec;
    bool exactmatch;
    aCmd.PVMFSocketNodeCommandBase::Parse(mimetype, uuidvec, exactmatch);

    //Try to match the input mimetype against any of
    //the custom interfaces for this node

    //Match against custom interface1...
    if (*mimetype == PVMF_SOCKET_NODE_EXTENSION_INTERFACE_MIMETYPE
            //also match against base mimetypes for custom interface1,
            //unless exactmatch is set.
            || (!exactmatch && *mimetype == PVMF_SOCKET_NODE_MIMETYPE)
            || (!exactmatch && *mimetype == PVMF_SOCKET_NODE_BASEMIMETYPE))
    {

        PVUuid uuid(PVMF_SOCKET_NODE_EXTENSION_INTERFACE_UUID);
        uuidvec->push_back(uuid);
    }
    return PVMFSuccess;
}

bool PVMFSocketNode::queryInterface(const PVUuid& uuid, PVInterface*& iface)
{
    iface = NULL;
    if (uuid == PVUuid(PVMF_SOCKET_NODE_EXTENSION_INTERFACE_UUID))
    {
        if (!iExtensionInterface)
        {
            iExtensionInterface = OSCL_NEW(PVMFSocketNodeExtensionInterfaceImpl, (this));
        }
        if (iExtensionInterface)
        {
            return (iExtensionInterface->queryInterface(uuid, iface));
        }
        else
        {
            PVMF_SOCKETNODE_LOGERROR((0, "PVMFSocketNode::queryInterface: Error - Out of memory"));
            OSCL_LEAVE(OsclErrNoMemory);
            return false;
        }
    }
    else
    {
        return false;
    }
}

PVMFStatus PVMFSocketNode::DoQueryInterface(PVMFSocketNodeCommand&  aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFSocketNode::DoQueryInterface"));

    PVUuid* uuid;
    PVInterface** ptr;
    aCmd.PVMFSocketNodeCommandBase::Parse(uuid, ptr);
    if (*uuid == PVUuid(PVMF_SOCKET_NODE_EXTENSION_INTERFACE_UUID))
    {
        if (!iExtensionInterface)
        {
            iExtensionInterface = OSCL_NEW(PVMFSocketNodeExtensionInterfaceImpl, (this));
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

PVMFStatus PVMFSocketNode::DoRequestPort(PVMFSocketNodeCommand& aCmd, PVMFSocketPort* &aPort)
{
    PVMF_SOCKETNODE_LOGSTACKTRACE((0, "PVMFSocketNode::DoRequestPort() In"));

    //retrieve port tag.
    int32 tag;
    OSCL_String* portconfig;
    aCmd.PVMFSocketNodeCommandBase::Parse(tag, portconfig);
    //validate the tag...
    switch (tag)
    {
        case PVMF_SOCKET_NODE_PORT_TYPE_SOURCE:
        case PVMF_SOCKET_NODE_PORT_TYPE_SINK:
        case PVMF_SOCKET_NODE_PORT_TYPE_PASSTHRU:
            break;
        default:
            //bad port tag
            PVMF_SOCKETNODE_LOGERROR((0, "PVMFSocketNode::DoRequestPort: Error - Invalid port tag"));
            iCommandErrorCode = PVMFSocketNodeErrorInvalidPortTag;
            return PVMFErrArgument;
    }

    SOCKET_ADDR sock_add;
    sock_add.iTag = tag;
    OSCL_HeapString<OsclMemAllocator> mime;
    if (!ParseTransportConfig(portconfig, sock_add, mime))
    {
        PVMF_SOCKETNODE_LOGERROR((0, "PVMFSocketNode::DoRequestPort: ERROR - Invalid port config"));
        iCommandErrorCode = PVMFSocketNodeErrorInvalidPortTag;
        return PVMFErrArgument;
    }

    PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFSocketNode::DoRequestPort tag %d config %s",
                                      tag, portconfig->get_cstr()));

    switch (sock_add.iProtocol)
    {
        case INET_TCP:
        {
            //Create a new port with no socket.

            PVMFStatus status = AddPort(tag, aPort);
            if (status != PVMFSuccess)
                return status;

            //create the socket config

            SocketPortConfig* sock_config = OSCL_NEW(SocketPortConfig, ());
            aPort->iConfig = sock_config;
            sock_config->iPVMFPort = aPort;

            sock_config->iMime = mime;
            sock_config->iContainer = this;
            sock_config->iSockId = iSocketID++;
            sock_config->iTag = tag;
            sock_config->iAddr = sock_add;

            //create the mem pool
            PVMFSocketNodeMemPool* memPool;
            status = AllocatePortMemPool(tag, memPool);
            if (status == PVMFSuccess)
                sock_config->iMemPool = memPool;
            if (status != PVMFSuccess)
                return status;

            //optionally create the socket and launch the connect sequence.
            if (oscl_strstr(portconfig->get_cstr(), ";no-connect-on-create") != NULL)
            {
                //if "no create on connect" was selected then port creation is complete now.
                status = PVMFSuccess;
            }
            else
            {
                //else create the socket and launch the connect sequence.
                OSCL_ASSERT(sock_config->iState.iSequence == EPVSocketPortSequence_None);
                status = StartSequence(*sock_config, EPVSocketPortSequence_RequestPort);
                //may need to wait on connect completion here, or else there could
                //be an error.
            }
            return status;
        }

        case INET_UDP:
        {
            //We should have already created this UDP socket in a call to AllocateConsecutivePorts.
            SocketPortConfig* sockConfig = FindSocketPortConfig(sock_add);
            if (!sockConfig)
            {
                PVMF_SOCKETNODE_LOGERROR((0, "PVMFSocketNode::DoRequestPort: ERROR - Invalid protocol for port request"));
                return PVMFFailure;
            }

            //The udp socket was already created using AllocateConsecutivePorts(),
            //the but PVMFPort and memory pool were not yet created.

            //Allocate a new PVMF port

            PVMFStatus status = AddPort(tag, aPort);
            if (status == PVMFSuccess)
            {
                PVMFSocketNodeMemPool* memPool = NULL;
                status = AllocatePortMemPool(tag, memPool);
                if (status == PVMFSuccess)
                    sockConfig->iMemPool = memPool;
            }

            if (status != PVMFSuccess)
            {
                //put the port config back in the allocated port vector
                iAllocatedPortVector.push_back(sockConfig);
                return status;
            }

            //move the sock config to the PVMF port vector and link the config
            //back to the port.
            aPort->iConfig = sockConfig;
            sockConfig->iPVMFPort = aPort;

            return status;
        }
        break;
        default:
            PVMF_SOCKETNODE_LOGERROR((0, "PVMFSocketNode::DoRequestPort: ERROR - Invalid protocol for port request"));
            return PVMFFailure;
    }
    PVMF_SOCKETNODE_LOGERROR((0, "PVMFSocketNode::DoRequestPort: ERROR - Invalid protocol for port request"));
    return PVMFFailure;
}

//Release ports is a do-nothing for this node.
PVMFStatus PVMFSocketNode::DoReleasePort(PVMFSocketNodeCommand& aCmd)
{
    OSCL_UNUSED_ARG(aCmd);
    return PVMFSuccess;
}

PVMFStatus PVMFSocketNode::DoInit(PVMFSocketNodeCommand& aCmd)
{
    OSCL_UNUSED_ARG(aCmd);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFSocketNode::DoInit() In"));

    if (iInterfaceState != EPVMFNodeIdle)
    {
        return PVMFErrInvalidState;
    }

#if(ENABLE_SOCKET_NODE_STATS)
    iSocketNodeStats.Init();
#endif

    PVMFStatus status = PVMFSuccess;

    //Create socket server session
    if (NULL == iSockServ)
    {
        int32 err;
        OSCL_TRY(err, iSockServ = OsclSocketServ::NewL(iAlloc););
        if (err || (iSockServ ==  NULL))
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFSocketNode::DoInit: ERROR. OsclSocketServ::NewL() fail Ln %d", __LINE__));

            iCommandErrorCode = PVMFSocketNodeErrorSocketServerCreateError;
            status =  PVMFErrNoResources;
        }
        else
        {
            if (iSockServ->Connect() == OsclErrNone)
            {
                status = PVMFSuccess;
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMFSocketNode::DoInit: ERROR. OsclSocketServ::Connect() fail Ln %d", __LINE__));

                iCommandErrorCode = PVMFSocketNodeErrorSocketServConnectError;
                status = PVMFErrResource;
            }
        }
    }
    return status;
}

//Prepare is a do-nothing for this node.
PVMFStatus PVMFSocketNode::DoPrepare(PVMFSocketNodeCommand& aCmd)
{
    OSCL_UNUSED_ARG(aCmd);
    PVMF_SOCKETNODE_LOGSTACKTRACE((0, "PVMFSocketNode::DoPrepare() In"));

    if (iInterfaceState != EPVMFNodeInitialized)
    {
        return PVMFErrInvalidState;
    }
    return PVMFSuccess;
}

PVMFStatus PVMFSocketNode::DoStart(PVMFSocketNodeCommand& aCmd)
{
    OSCL_UNUSED_ARG(aCmd);

    PVMF_SOCKETNODE_LOGSTACKTRACE((0, "PVMFSocketNode::DoStart() In"));
    /*
     * If the node is already started just return success - multiple starts can happen with
     * flow control (auto-pause / auto-resume) scenarios
     */
    if (iInterfaceState == EPVMFNodeStarted)
    {
        return PVMFSuccess;//already started.
    }

    if (iInterfaceState != EPVMFNodePrepared &&
            iInterfaceState != EPVMFNodePaused)
    {
        return PVMFErrInvalidState;
    }

    //We need to update the node state before
    //we can get things going.
    TPVMFNodeInterfaceState curState = iInterfaceState;
    ChangeExternalState(EPVMFNodeStarted);

    PVMFStatus status = PVMFSuccess;
    for (uint32 i = 0;i < iPortVector.size();i++)
    {
        SocketPortConfig* sockConfig = iPortVector[i]->iConfig;
        if (sockConfig)
        {
            //Start handling incoming messages
            if (CanProcessIncomingMsg(*sockConfig))
                ProcessIncomingMsg(*sockConfig);

            //Start the receives.
            if (CanReceive(*sockConfig))
            {
                status = StartRecvOperation(*sockConfig);
                //receive may be pending, but we don't block
                //the start command for it.
                if (status == PVMFPending)
                {
                    status = PVMFSuccess;
                }
                else if (status != PVMFSuccess)
                {
                    //fail the command if any individual startups fail.
                    //restore the old node state.
                    ChangeExternalState(curState);
                    break;//out of for loop
                }
            }
        }
    }
    return status;
}

PVMFStatus PVMFSocketNode::DoStop(PVMFSocketNodeCommand& aCmd)
{
    OSCL_UNUSED_ARG(aCmd);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFSocketNode::DoStop() In"));

    if ((iInterfaceState != EPVMFNodeStarted) && (iInterfaceState != EPVMFNodePaused))
    {
        return PVMFErrInvalidState;
    }

    //Stop will essentially just stop the receive operations and input port processing,
    //since those handlers check the node state.  Current operations will not be
    //interrupted.
    return PVMFSuccess;
}

//Stop all current socket/DNS activity and clear port messages.
//Return PVMFSuccess if everything is stopped, or else PVMFPending if some
//ports still have operations pending.
//
//Also update the global "iNumStopNodeActivityPending"
//with the number of ports that still have operations pending
PVMFStatus PVMFSocketNode::DoStopNodeActivity()
{
    //note that "iNumStopNodeActivityPending" uses the value (-1) to indicate idle state,
    //or N...0 to countdown how many ports we're waiting on.

    //Note: in case there's already a "stop node activity" going on, just
    //return the status.
    if (iNumStopPortActivityPending > 0)
        return PVMFPending; //keep waiting on completion (in SequenceComplete)

    //Notify all PVMF ports to suspend their input, and discard any current messages.
    for (uint32 i = 0;i < iPortVector.size();i++)
    {
        iPortVector[i]->SuspendInput();
        iPortVector[i]->ClearMsgQueues();
    }

    //Stop socket activity on all ports.
    {
        uint32 nPortsPending = 0;
        for (uint32 i = 0;i < iPortVector.size();i++)
        {
            SocketPortConfig* portConfig = iPortVector[i]->iConfig;
            if (portConfig)
            {
                // discard any saved socket activity events
                portConfig->iSocketRecvActivity.iValid = false;

                // start a shutdown sequence on the port
                // look at what else might be happening currently.

                //if a request port is going on, be sure to complete the command,
                //although we will interrupt the current processing.
                if (portConfig->iState.iSequence == EPVSocketPortSequence_RequestPort
                        && iCurrentCmdQueue.size()
                        && iCurrentCmdQueue.front().iCmd == PVMF_GENERIC_NODE_REQUESTPORT)
                {
                    //@TODO would be nice to cleanup the allocated port in case of a failure.
                    CommandComplete(iCurrentCmdQueue, iCurrentCmdQueue.front(), PVMFErrCancelled, NULL);
                }
                //for any other sequence, we will just interrupt it with the shutdown.

                //if a cleanup is already underway, just keep waiting for it to complete,
                //else start a new sequence
                if (portConfig->iState.iSequence == EPVSocketPortSequence_SocketCleanup)
                {
                    nPortsPending++;
                }
                else if (StartSequence(*portConfig, EPVSocketPortSequence_SocketCleanup) == PVMFPending)
                {
                    nPortsPending++;
                }
            }
        }

        if (nPortsPending > 0)
            iNumStopPortActivityPending = nPortsPending;
    }

    if (iNumStopPortActivityPending > 0)
        return PVMFPending; //wait on completion in SequenceComplete.

    return PVMFSuccess;
}

//Flush is implemented for this node, but hasn't been tested.
PVMFStatus PVMFSocketNode::DoFlush(PVMFSocketNodeCommand& aCmd)
{
    OSCL_UNUSED_ARG(aCmd);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFSocketNode::DoFlush() In"));

    if ((iInterfaceState != EPVMFNodeStarted) && (iInterfaceState != EPVMFNodePaused))
    {
        return PVMFErrInvalidState;
    }

    //Notify all ports to suspend their input, then
    //wait on completion of processing of current input.
    PVMFStatus status = PVMFSuccess;
    {
        for (uint32 i = 0;i < iPortVector.size();i++)
        {
            iPortVector[i]->SuspendInput();
            if (status != PVMFPending
                    && iPortVector[i]->IncomingMsgQueueSize() > 0)
            {
                status = PVMFPending;//Wait on this queue to empty.
                //Completion is detected in SequenceComplete.
            }
        }
    }

    return status;
}

//Pause is a do-nothing for this node.
PVMFStatus PVMFSocketNode::DoPause(PVMFSocketNodeCommand& aCmd)
{
    OSCL_UNUSED_ARG(aCmd);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFSocketNode::DoPause() In"));

    /*
     * If the node is already paused just return success - multiple pause can happen with
     * flow control (auto-pause / auto-resume) scenarios
     */
    if (iInterfaceState == EPVMFNodePaused)
    {
        return PVMFSuccess;
    }

    if (iInterfaceState != EPVMFNodeStarted)
    {
        return PVMFErrInvalidState;
    }

    return PVMFSuccess;
}

PVMFStatus PVMFSocketNode::DoCancelCommand(PVMFSocketNodeCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFSocketNode::DoCancelCommand() Called"));

    // extract the command ID from the parameters.
    PVMFCommandId cmdId;
    aCmd.PVMFSocketNodeCommandBase::Parse(cmdId);

    // first check "current" command if any
    // it's safe to assume the "current cmd queue" is only 1-deep.
    if (!iCurrentCmdQueue.empty()
            && iCurrentCmdQueue.front().iId == cmdId)
    {
        return DoCancelCurrentCommand(iCurrentCmdQueue, iCurrentCmdQueue.front());
        //wait on current command to complete.  The cancel command
        //will ultimately be completed in the "CommandComplete" for the current command.
    }

    // next see if the command is still queued in the "pending command queue"
    {
        // start at element 1 since this cancel command is element 0
        PVMFSocketNodeCommand* cmd = iPendingCmdQueue.FindById(cmdId, 1);
        if (cmd)
        {
            // cancel the queued command.  Note this will complete out-of-order.
            CommandComplete(iPendingCmdQueue, *cmd, PVMFErrCancelled);
            // no further action is required.
            return PVMFSuccess;
        }
    }

    //this command fails if the given command is not queued or in progress.
    return PVMFErrArgument;
}

PVMFStatus PVMFSocketNode::DoCancelAllCommands(PVMFSocketNodeCommand& aCmd)
{
    OSCL_UNUSED_ARG(aCmd);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFSocketNode::DoCancelCommand() IN"));

    //This is not a conventional "cancel all" implementation.
    //Streaming manager currently expects this call to shutdown all activity.
    //It would really be more correct to use node Reset for this.

    PVMFStatus status = DoStopNodeActivity();

    //Since node activity is suspended, just go ahead and forcibly cancel any other
    //pending commands now.  There could be a Reset, a Flush, or a RequestPort
    //happening.
    if (!iCurrentCmdQueue.empty())
        CommandComplete(iCurrentCmdQueue, iCurrentCmdQueue.front(), PVMFErrCancelled);

    //Cancel all other pending commands, except for this one which is
    //element 0 in the queue
    for (uint32 i = 1;i < iPendingCmdQueue.size();i++)
        CommandComplete(iPendingCmdQueue, iPendingCmdQueue[i], PVMFErrCancelled);

    //May need to wait on completion of StopNodeActivity.
    return status;
}

//This routine must cancel the currently executing node command.
//It only needs to handle those commands that may have asynchronous completion.
PVMFStatus PVMFSocketNode::DoCancelCurrentCommand(PVMFSocketNodeCmdQ& aCmdQ, PVMFSocketNodeCommand& aCmd)
{
    OSCL_UNUSED_ARG(aCmdQ);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFSocketNode::DoCancelCurrentCommand()"));
    switch (aCmd.iCmd)
    {

        case PVMF_GENERIC_NODE_REQUESTPORT:
            //there may be a connect operation pending-- cancel it.
        {
            if (iRequestedPort)
            {
                SocketPortConfig* sockConfig = iRequestedPort->iConfig;
                if (sockConfig
                        && sockConfig->iState.iConnectOperation != EPVSocketPortConnectOperation_None)
                {
                    CancelConnectOperation(*sockConfig);
                    return PVMFPending;//wait on the operation to complete
                    //in HandleSocketEvent or HandleDNSEvent
                }
            }
            //shouldn't get here...
            return PVMFFailure;
        }
        break;

        case PVMF_GENERIC_NODE_RESET:
            //it's too complicated to cancel a reset, so just wait on completion
            return PVMFPending;

        case PVMF_GENERIC_NODE_FLUSH:
            //to cancel a flush, just discard all remaining port messages
            //and keep waiting on completion of current message.
        {
            for (uint32 i = 0;i < iPortVector.size();i++)
                iPortVector[i]->ClearMsgQueues();
        }
        return PVMFPending;//keep waiting on flush completion in SequenceComplete.

        case PVMF_GENERIC_NODE_CANCELCOMMAND:
        case PVMF_GENERIC_NODE_CANCELALLCOMMANDS:
            //these have asynchronous completion, but the command handling prevents
            //processing a cancel during a cancel, so we shouldn't get here.
            OSCL_ASSERT(0);
            return PVMFFailure;

        default:
            //no other node commands have asynchronous completion, so this is unexpected.
            OSCL_ASSERT(0);
            return PVMFFailure;
    }
}

//////////////////////////////////////
// End Command Handlers
//////////////////////////////////////


//////////////////////////////////////
// DNS & Socket Callbacks
//////////////////////////////////////

OSCL_EXPORT_REF void PVMFSocketNode::HandleDNSEvent(int32 aId, TPVDNSFxn aFxn, TPVDNSEvent aEvent, int32 aError)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFSocketNode::HandleDNSEvent() In aId=%d, aFxn=%d, aEvent=%d, aError=%d", aId, aFxn, aEvent, aError));

    iInSocketCallback = true;

    if (aEvent != EPVDNSSuccess)
    {
        PVMF_SOCKETNODE_LOGERROR((0, "PVMFSocketNode::HandleDNSEvent: Error - Failure"));
    }

    //Note: since the DNS object ID is identical to the socket ID, we can lookup the
    //socket port config using the DNS ID.
    SocketPortConfig* sockConfigPtr = FindSocketPortConfig((uint32)aId);
    if (!sockConfigPtr)
    {
        //it's possible the DNS object was already delete-- so just fail gracefully.
        PVMF_SOCKETNODE_LOGERROR((0, "PVMFSocketNode::HandleDNSEvent: Error - Invalid Socket ID"));
    }
    else
    {

#if(ENABLE_SOCKET_NODE_STATS)
        sockConfigPtr->iPortStats.iNumDNSCallback++;
#endif

        //create socket activity
        PVMFSocketActivity activity((aEvent == EPVDNSSuccess) ? PVMFSuccess : PVMFFailure, aId, aFxn, aEvent, aError);

        //Call the appropriate handler
        switch (aFxn)
        {
            case EPVDNSGetHostByName:
                OSCL_ASSERT(sockConfigPtr->iState.iConnectOperation == EPVSocketPortConnectOperation_GetHostByName);
#if(ENABLE_SOCKET_NODE_STATS)
                if (aEvent == EPVDNSSuccess)
                    sockConfigPtr->iPortStats.EndConnectTime(sockConfigPtr->iState.iConnectOperation);
#endif
                ConnectOperationComplete(*sockConfigPtr, activity.iStatus, &activity);
                break;

            default:
                OSCL_ASSERT(0);//unexpected
                break;
        }
    }

    iInSocketCallback = false;
}


//This is the callback from Oscl Sockets for socket operation completion.
OSCL_EXPORT_REF  void PVMFSocketNode::HandleSocketEvent(int32 aId, TPVSocketFxn aFxn, TPVSocketEvent aEvent, int32 aError)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFSocketNode::HandleSocketEvent() In aId=%d, aFxn=%d, aEvent=%d, aError=%d", aId, aFxn, aEvent, aError));

    iInSocketCallback = true;

    SocketPortConfig* sockConfigPtr = FindSocketPortConfig((uint32)aId);
    if (!sockConfigPtr)
    {
        //it's possible the socket was already deleted-- so just fail gracefully.
        PVMF_SOCKETNODE_LOGERROR((0, "PVMFSocketNode::HandleSocketEvent: Error - Invalid Socket ID"));
    }
    else
    {
        if (aEvent != EPVSocketSuccess)
        {
            PVMF_SOCKETNODE_LOGERROR((0, "PVMFSocketNode::HandleSocketEvent: Error - Event=%d, SockId=%d, Mime=%s",
                                      aEvent, aId, sockConfigPtr->iMime.get_cstr()));
        }
#if(ENABLE_SOCKET_NODE_STATS)
        sockConfigPtr->iPortStats.iNumSocketCallback++;
#endif
        //create socket activity
        PVMFSocketActivity activity((aEvent == EPVSocketSuccess) ? PVMFSuccess : PVMFFailure, aId, aFxn, aEvent, aError);

        //Call the appropriate handler
        switch (aFxn)
        {
            case EPVSocketRecvFrom:
            case EPVSocketRecv:
                OSCL_ASSERT(sockConfigPtr->iState.iRecvOperation == EPVSocketPortRecvOperation_Recv
                            || sockConfigPtr->iState.iRecvOperation == EPVSocketPortRecvOperation_RecvFrom);
#if(ENABLE_SOCKET_NODE_STATS)
                if (aEvent == EPVSocketSuccess)
                    sockConfigPtr->iPortStats.EndRecvTime(sockConfigPtr->iState.iRecvOperation);
#endif
                RecvOperationComplete(*sockConfigPtr, activity.iStatus, &activity);
                break;

            case EPVSocketSendTo:
            case EPVSocketSend:
                OSCL_ASSERT(sockConfigPtr->iState.iSendOperation == EPVSocketPortSendOperation_Send
                            || sockConfigPtr->iState.iSendOperation == EPVSocketPortSendOperation_SendTo);
#if(ENABLE_SOCKET_NODE_STATS)
                if (aEvent == EPVSocketSuccess)
                    sockConfigPtr->iPortStats.EndSendTime(sockConfigPtr->iState.iSendOperation);
#endif
                SendOperationComplete(*sockConfigPtr, activity.iStatus, &activity);
                break;

            case EPVSocketConnect:
                OSCL_ASSERT(sockConfigPtr->iState.iConnectOperation == EPVSocketPortConnectOperation_Connect);
#if(ENABLE_SOCKET_NODE_STATS)
                if (aEvent == EPVSocketSuccess)
                    sockConfigPtr->iPortStats.EndConnectTime(sockConfigPtr->iState.iConnectOperation);
#endif
                ConnectOperationComplete(*sockConfigPtr, activity.iStatus, &activity);
                break;

            case EPVSocketShutdown:
                OSCL_ASSERT(sockConfigPtr->iState.iConnectOperation == EPVSocketPortConnectOperation_Shutdown);
#if(ENABLE_SOCKET_NODE_STATS)
                if (aEvent == EPVSocketSuccess)
                    sockConfigPtr->iPortStats.EndConnectTime(sockConfigPtr->iState.iConnectOperation);
#endif
                ConnectOperationComplete(*sockConfigPtr, activity.iStatus, &activity);
                break;

            default:
                OSCL_ASSERT(0);//unexpected
                break;
        }
    }

    iInSocketCallback = false;
}

//////////////////////////////////////
// End DNS & Socket Callbacks
//////////////////////////////////////

///////////////////////
// Low-level port processing
///////////////////////

//Allocate a node port and add it to the port vector.
PVMFStatus PVMFSocketNode::AddPort(int32 tag, PVMFSocketPort* &port)
{
    PVMF_SOCKETNODE_LOGSTACKTRACE((0, "PVMFSocketNode::AddPort() In"));
    int32 err;
    OsclAny *ptr = NULL;
    OSCL_TRY(err, ptr = iPortVector.Allocate(););
    if (err != OsclErrNone || !ptr)
    {
        PVMF_SOCKETNODE_LOGERROR((0, "PVMFSocketNode::AddPort: ERROR - err=%d Ln %d", err, __LINE__));
        return PVMFErrNoMemory;
    }

    port = new(ptr) PVMFSocketPort((int32)tag,
                                   this,
                                   DEFAULT_DATA_QUEUE_CAPACITY,
                                   DEFAULT_DATA_QUEUE_CAPACITY,
                                   DEFAULT_READY_TO_RECEIVE_THRESHOLD_PERCENT,
                                   //the output queue is empty because
                                   //this node pushes data directly to the
                                   //connecte port's input.
                                   0, 0, 0);

    //Add the port to the port vector.
    OSCL_TRY(err, iPortVector.AddL(port););
    if (err != OsclErrNone)
    {
        iPortVector.DestructAndDealloc(port);
        return PVMFErrNoMemory;
    }
    return PVMFSuccess;
}

//Allocate a memory pool for the input port config.
//When finished, the port config is updated with the
//mempool and the port tag.
PVMFStatus PVMFSocketNode::AllocatePortMemPool(int32 tag, PVMFSocketNodeMemPool* & aMemPool)
{
    PVMF_SOCKETNODE_LOGSTACKTRACE((0, "PVMFSocketNode::AllocatePortMemPool() In"));
    aMemPool = NULL;
    OsclAny *MemPtr = NULL;
    int32 err;
    OSCL_TRY(err, MemPtr = iAlloc.ALLOCATE(sizeof(PVMFSocketNodeMemPool)););
    if ((err != OsclErrNone) || (MemPtr == NULL))
    {
        PVMF_SOCKETNODE_LOGERROR((0, "PVMFSocketNode::DoRequestPort: PVMFSocketNodeMemPool Allocate Failed Ln %d", err, __LINE__));
        return PVMFErrNoMemory;
    }

    switch (tag)
    {
        case PVMF_SOCKET_NODE_PORT_TYPE_SOURCE:
            //source tag is used for RTP
        {
            int32 errcode = 0;
            errcode = SocketPlacementNew(aMemPool, MemPtr, DEFAULT_NUM_MEDIA_MSGS_IN_JITTER_BUFFER);
            if (errcode != OsclErrNone)
            {
                PVMF_SOCKETNODE_LOGERROR((0, "PVMFSocketNode::DoRequestPort: PVMFSocketNodeMemPool Construct Failed Ln %d", err, __LINE__));
                iAlloc.deallocate(MemPtr);
                return PVMFErrNoMemory;
            }
        }
        break;
        case PVMF_SOCKET_NODE_PORT_TYPE_SINK:
        case PVMF_SOCKET_NODE_PORT_TYPE_PASSTHRU:
            //sink tag is used for RTCP
        {
            int32 errcode = 0;
            errcode = SocketPlacementNew(aMemPool, MemPtr, SNODE_DEFAULT_NUMBER_MEDIADATA_IN_MEMPOOL);
            if (errcode != OsclErrNone)
            {
                PVMF_SOCKETNODE_LOGERROR((0, "PVMFSocketNode::DoRequestPort: PVMFSocketNodeMemPool Construct Failed Ln %d", err, __LINE__));
                iAlloc.deallocate(MemPtr);
                return PVMFErrNoMemory;
            }
        }
        break;

        default://unexpected-- caller already validated the tag.
            iAlloc.deallocate(MemPtr);
            return PVMFFailure;
            break;
    }
    aMemPool->iPortTag = tag;
    return PVMFSuccess;
}


//Lookup the socket port config entry, given a socket address.
//If the entry is in the "allocated port" vector, it will be erased, so
//it's up to the caller to make sure it gets deleted or moved.
SocketPortConfig* PVMFSocketNode::FindSocketPortConfig(SOCKET_ADDR& aSockAddr)
{
    uint32 i;
    //search the allocated port vector
    for (i = 0;i < iAllocatedPortVector.size();i++)
    {
        if (iAllocatedPortVector[i]
                && MatchSocketAddr(aSockAddr, *iAllocatedPortVector[i]))
        {
            //erase it from the vector before returning to the caller.
            SocketPortConfig* elem = iAllocatedPortVector[i];
            iAllocatedPortVector.erase(&iAllocatedPortVector[i]);
            return elem;
        }
    }
    //search the port vector
    for (i = 0;i < iPortVector.size();i++)
    {
        if (iPortVector[i]->iConfig
                && MatchSocketAddr(aSockAddr, *iPortVector[i]->iConfig))
        {
            return iPortVector[i]->iConfig;
        }
    }
    return NULL;
}

//return "true" if input socket address matches port config
bool PVMFSocketNode::MatchSocketAddr(SOCKET_ADDR& aSockAddr, SocketPortConfig& aSockConfig)
{
    return (aSockConfig.iAddr.iProtocol == aSockAddr.iProtocol
            && aSockConfig.iAddr.iRemoteAdd.port == aSockAddr.iRemoteAdd.port
            && aSockConfig.iAddr.iRemoteHost == aSockAddr.iRemoteHost
            && aSockConfig.iTag == aSockAddr.iTag
            //local address may or may not be specified-- ignore if not.
            && (!aSockAddr.iLocalAdd.port
                || (aSockConfig.iAddr.iLocalAdd.port == aSockAddr.iLocalAdd.port)));
}

//Lookup the socket port config entry, given a socket ID.
SocketPortConfig* PVMFSocketNode::FindSocketPortConfig(uint32 aId)
{
    for (uint32 i = 0;i < iPortVector.size();i++)
    {
        if (iPortVector[i]->iConfig
                && iPortVector[i]->iConfig->iSockId == aId)
        {
            return iPortVector[i]->iConfig;
        }
    }
    return NULL;
}

//Create a socket using the given socket ID and protocol.
//For UDP, this will also attempt to bind, incrementing port numbers until
//either success or maximum attempts is reached.  This has the side effect
//of updating the port number in the local address.
//It returns a pointer to the socket cast as OsclAny*.
OsclAny* PVMFSocketNode::CreateOsclSocketAndBind(SOCKET_ADDR &aSockAdd, uint32 aSockId)
{
    PVMF_SOCKETNODE_LOGSTACKTRACE((0, "PVMFSocketNode::CreateOsclSocketAndBind() In"));
    //create the socket server session if it doesn't exist yet.
    if (iSockServ == NULL)
    {
        int32 err;
        OSCL_TRY(err, iSockServ = OsclSocketServ::NewL(iAlloc););
        if (err || (iSockServ ==  NULL))
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFSocketNode::CreateSocket() ERROR- iSockServ=0x%x, err=%d, Ln %d", iSockServ, err, __LINE__));
            return NULL;
        }
        if (iSockServ->Connect() != OsclErrNone)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFSocketNode::CreateSocket() ERROR Connect(), Ln %d", __LINE__));
            return NULL;
        }
    }

    switch (aSockAdd.iProtocol)
    {
        case INET_UDP:
        {
            int32 err;
            OsclUDPSocket *UDPSocket = NULL;
            OSCL_TRY(err, UDPSocket = OsclUDPSocket::NewL(iAlloc, *iSockServ, this, aSockId););
            if (err || (UDPSocket ==  NULL))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFSocketNode::CreateSocket() ERROR- UDPSocket=0x%x, err=%d, Ln %d", UDPSocket, err, __LINE__));
                return NULL;
            }

            //Bind this socket to the address
            bool bBindOK = false;
            int maxNumOfBind = UDP_PORT_RANGE;
            while (maxNumOfBind-- > 0)
            {
#if(ENABLE_SOCKET_NODE_STATS)
                iSocketNodeStats.iNumBind++;
#endif
                if (UDPSocket->Bind(aSockAdd.iLocalAdd) == EPVSocketSuccess)
                {
                    bBindOK = true;
                    break;
                }
                aSockAdd.iLocalAdd.port++;
            };

            //If bind failed, delete the socket.
            if (!bBindOK)
            {
                UDPSocket->~OsclUDPSocket();
                iAlloc.deallocate(UDPSocket);

                UDPSocket = NULL;
            }
            return UDPSocket;
        }

        case INET_TCP:
        {

            int32 err;
            OsclTCPSocket *TCPSocket = NULL;

            OSCL_TRY(err, TCPSocket = OsclTCPSocket::NewL(iAlloc, *iSockServ, this, aSockId););
            if (err || (TCPSocket ==  NULL))
            {
                return NULL;
            }
            return TCPSocket;
        }

        default:
            return NULL;
    }
}

//This finishes up a TCP socket cleanup, after the socket shutdown
//is complete.
void PVMFSocketNode::CleanupTCP(SocketPortConfig& aSockConfig)
{
    if (aSockConfig.iTCPSocket)
    {
        //Close the socket.  This will free the OS-level resources.  This
        //socket object cannot be re-used.
        aSockConfig.iTCPSocket->Close();
        PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFSocketNode::CleanupTCP - Socket Closed - SockId=%d, Mime=%s, Port=0x%x", aSockConfig.iSockId, aSockConfig.iMime.get_str(), aSockConfig.iPVMFPort));

        //Send EOS.  To avoid multiple EOS we only do this when the
        //socket is still open.
        if (aSockConfig.iPVMFPort
                && aSockConfig.iPVMFPort->IsConnected())
        {
            PVMFSharedMediaCmdPtr sharedMediaCmdPtr = PVMFMediaCmd::createMediaCmd();
            sharedMediaCmdPtr->setFormatID(PVMF_MEDIA_CMD_EOS_FORMAT_ID);
            PVMFSharedMediaMsgPtr mediaMsgOut;
            convertToPVMFMediaCmdMsg(mediaMsgOut, sharedMediaCmdPtr);
#if(ENABLE_SOCKET_NODE_STATS)
            aSockConfig.iPortStats.iNumQueueOutgoingEOSMsg++;
#endif
            PVMFStatus status = aSockConfig.iPVMFPort->QueueOutgoingMsg(mediaMsgOut);
            //There's no recovery from a queue busy condition here.
            //We assume the caller already waited on the connected port before
            //calling this routine.
            if (status != PVMFSuccess)
            {
                PVMF_SOCKETNODE_LOGERROR((0, "PVMFSocketNode::SendPVMFEOS() ERROR:%d, Outgoing queue size=%d. Data discarded!", status, aSockConfig.iPVMFPort->OutgoingMsgQueueSize()));
                ReportErrorEvent(PVMFErrPortProcessing);
                PVMF_SOCKETNODE_LOGDATATRAFFIC_E((0, "PVMFSocketNode::CleanupTCP - EOS Send FAILED - SockId=%d, Mime=%s, Port=0x%x", aSockConfig.iSockId, aSockConfig.iMime.get_str(), aSockConfig.iPVMFPort));
            }
            else
            {
                PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFSocketNode::CleanupTCP - EOS Sent - SockId=%d, Mime=%s, Port=0x%x", aSockConfig.iSockId, aSockConfig.iMime.get_str(), aSockConfig.iPVMFPort));
            }
        }

        if (iInSocketCallback)
        {
            //prevent deleting an object from within its own Run.
            PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFSocketNode::CleanupTCP - queueing old socket for deletion - SockId=%d, Mime=%s", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
            iClosedTCPSocketVector.push_back(aSockConfig.iTCPSocket);
            aSockConfig.iTCPSocket = NULL;
            RunIfNotReady();//schedule the AO to delete the socket.
        }
        else
        {
            PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFSocketNode::CleanupTCP - Deleting socket - SockId=%d, Mime=%s, Port=0x%x", aSockConfig.iSockId, aSockConfig.iMime.get_str(), aSockConfig.iPVMFPort));
            aSockConfig.iTCPSocket->~OsclTCPSocket();
            iAlloc.deallocate(aSockConfig.iTCPSocket);
            aSockConfig.iTCPSocket = NULL;
        }
    }

    //cleanup media messages
    //(this must happen *after* the optional EOS)
    if (aSockConfig.iPendingRecvMediaData.GetRep() != NULL)
        aSockConfig.iPendingRecvMediaData.Unbind();
    if (aSockConfig.iPendingSendMediaData.GetRep() != NULL)
        aSockConfig.iPendingSendMediaData.Unbind();
}

//Cancel and/or delete UDP socket
void PVMFSocketNode::CleanupUDP(SocketPortConfig& aSockConfig)
{
    //delete
    if (aSockConfig.iUDPSocket)
    {
        if (iInSocketCallback)
        {
            //prevent deleting an object from within its own Run.
            PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFSocketNode::CleanupTCP - queueing old socket for deletion - SockId=%d, Mime=%s", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
            iClosedUDPSocketVector.push_back(aSockConfig.iUDPSocket);
            aSockConfig.iUDPSocket = NULL;
            RunIfNotReady();//schedule the AO to delete the socket.
        }
        else
        {
            aSockConfig.iUDPSocket->~OsclUDPSocket();
            iAlloc.deallocate(aSockConfig.iUDPSocket);
            aSockConfig.iUDPSocket = NULL;
        }
    }

    //cleanup media messages when socket is deleted
    if (aSockConfig.iPendingRecvMediaData.GetRep() != NULL)
        aSockConfig.iPendingRecvMediaData.Unbind();
    if (aSockConfig.iPendingSendMediaData.GetRep() != NULL)
        aSockConfig.iPendingSendMediaData.Unbind();
}

void PVMFSocketNode::CleanupDNS(SocketPortConfig& aSockConfig)
{
    if (aSockConfig.iDNS)
    {
        if (iInSocketCallback)
        {
            //prevent deleting an object from within its own Run.
            PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFSocketNode::CleanupDNS - queueing old DNS for deletion - SockId=%d, Mime=%s", aSockConfig.iSockId, aSockConfig.iMime.get_str()));
            iClosedDNSVector.push_back(aSockConfig.iDNS);
            aSockConfig.iDNS = NULL;
            RunIfNotReady();//schedule the AO to delete the object.
        }
        else
        {
            aSockConfig.iDNS->~OsclDNS();
            iAlloc.deallocate(aSockConfig.iDNS);
            aSockConfig.iDNS = NULL;
        }
    }
}

//A synchronous cleanup routine for all ports.
//It is assumed that all sockets are idle when this is called.
//This is used in node destructor, and in completion of
//Reset and Cancel All commands.
void PVMFSocketNode::CleanupPorts()
{
    // Cleanup port vector
    while (!iPortVector.empty())
    {
        SocketPortConfig* it = iPortVector.front()->iConfig;
        if (it)
        {
            //unlink the PVMFPort so we won't try to send any EOS msg
            //during the TCP cleanup.
            it->iPVMFPort = NULL;
            CleanupTCP(*it);
            CleanupUDP(*it);
            CleanupDNS(*it);
            it->CleanupMemPools(iAlloc);
            OSCL_DELETE(it);
            iPortVector.front()->iConfig = NULL;
        }
        iPortVector.Erase(&iPortVector.front());
    }
    // Cleanup allocated ports (these may have bound UDP sockets
    // but don't have any PVMF ports).
    while (!iAllocatedPortVector.empty())
    {
        SocketPortConfig* it = iAllocatedPortVector.front();
        CleanupTCP(*it);
        CleanupUDP(*it);
        CleanupDNS(*it);
        it->CleanupMemPools(iAlloc);
        OSCL_DELETE(it);
        iAllocatedPortVector.erase(&iAllocatedPortVector.front());
    }
}

void PVMFSocketNode::CleanupClosedTCPSockets()
{
    while (!iClosedTCPSocketVector.empty())
    {
        OsclTCPSocket* obj = iClosedTCPSocketVector.front();
        obj->~OsclTCPSocket();
        iAlloc.deallocate(obj);
        iClosedTCPSocketVector.erase(&iClosedTCPSocketVector.front());
    }
}

void PVMFSocketNode::CleanupClosedUDPSockets()
{
    while (!iClosedUDPSocketVector.empty())
    {
        OsclUDPSocket* obj = iClosedUDPSocketVector.front();
        obj->~OsclUDPSocket();
        iAlloc.deallocate(obj);
        iClosedUDPSocketVector.erase(&iClosedUDPSocketVector.front());
    }
}

void PVMFSocketNode::CleanupClosedDNS()
{
    while (!iClosedDNSVector.empty())
    {
        OsclDNS* obj = iClosedDNSVector.front();
        obj->~OsclDNS();
        iAlloc.deallocate(obj);
        iClosedDNSVector.erase(&iClosedDNSVector.front());
    }
}

bool PVMFSocketNode::ParseTransportConfig(OSCL_String *aPortConfig,
        SOCKET_ADDR &aSockConfig,
        OSCL_String& aMime)
{
    PVMF_SOCKETNODE_LOGSTACKTRACE((0, "PVMFSocketNode::ParseTransportConfig() In"));
    char* head = aPortConfig->get_str();
    int32 len = aPortConfig->get_size();
    return ParseTransportConfig(head, len, aSockConfig, aMime);
}

bool PVMFSocketNode::ParseTransportConfig(char *aPortConfig,
        int32 aLen,
        SOCKET_ADDR &aSockConfig,
        OSCL_String& aMime)
{
    PVMF_SOCKETNODE_LOGSTACKTRACE((0, "PVMFSocketNode::ParseTransportConfig() In"));
    char* head = aPortConfig;
    int32 len = aLen;

    char *tail = head + len;

    if (oscl_strstr(head, "TCP"))
        aSockConfig.iProtocol = INET_TCP;
    else if (oscl_strstr(head, "UDP"))
        aSockConfig.iProtocol = INET_UDP;
    else
        return false;//invalid protocol.

    //for future extension
    aSockConfig.iLocalAdd.ipAddr.Set("0.0.0.0");	//INADDR_ANY

    aSockConfig.iRemoteAdd.ipAddr.Set("");

    OSCL_StackString<128> address("remote_address=");

    char *tmpHead = OSCL_CONST_CAST(char*, oscl_strstr(head, address.get_cstr()));
    if (tmpHead)
    {
        tmpHead += address.get_size();
        char *tmpTail = OSCL_CONST_CAST(char*, oscl_strstr(tmpHead, ";"));
        if (tmpTail == NULL)
        {
            tmpTail = tail;
        }
        char tmpChar = *tmpTail;
        *tmpTail = '\0';
        aSockConfig.iRemoteAdd.ipAddr.Set(tmpHead);

        OSCL_HeapString<PVMFSocketNodeAllocator> tmpServerName(tmpHead, (tmpTail - tmpHead));
        aSockConfig.iRemoteHost = tmpServerName;

        *tmpTail = tmpChar;
    }

    {
        TimeValue current_time;
        current_time.set_to_current_time();
        uint32 my_seed = current_time.get_sec();

        OsclRand random_num;
        random_num.Seed(my_seed);
        int32 first = random_num.Rand();
        uint32 myport = (first & 0x1FFF) + 0x2000;	//start from 8192
        aSockConfig.iLocalAdd.port = (myport >> 1) << 1;	//start from even;
    }
    OSCL_StackString<128> client_port("client_port=");
    tmpHead = OSCL_CONST_CAST(char*, oscl_strstr(head, client_port.get_cstr()));
    if (tmpHead)
    {
        tmpHead += client_port.get_size();
        char *tmpTail = OSCL_CONST_CAST(char*, oscl_strstr(tmpHead, ";"));
        if (tmpTail == NULL)
        {
            tmpTail = tail;
        }
        char tmpChar = *tmpTail;
        *tmpTail = '\0';

        uint32 atoi_tmp;
        if (PV_atoi(tmpHead, 'd', atoi_tmp))
        {
            aSockConfig.iLocalAdd.port = atoi_tmp;
        }
        *tmpTail = tmpChar;
    }

    aSockConfig.iRemoteAdd.port = 0;
    OSCL_StackString<128> server_port("remote_port=");
    tmpHead = OSCL_CONST_CAST(char*, oscl_strstr(head, server_port.get_cstr()));
    if (tmpHead)
    {
        tmpHead += server_port.get_size();
        char *tmpTail = OSCL_CONST_CAST(char*, oscl_strstr(tmpHead, ";"));
        if (tmpTail == NULL)
        {
            tmpTail = tail;
        }

        char tmpChar = *tmpTail;
        *tmpTail = '\0';

        uint32 atoi_tmp;
        if (PV_atoi(tmpHead, 'd', atoi_tmp))
        {
            aSockConfig.iRemoteAdd.port = atoi_tmp;
        }
        *tmpTail = tmpChar;
    }

    OSCL_StackString<128> mime("mime=");
    tmpHead = OSCL_CONST_CAST(char*, oscl_strstr(head, mime.get_cstr()));
    if (tmpHead)
    {
        tmpHead += mime.get_size();
        char *tmpTail = OSCL_CONST_CAST(char*, oscl_strstr(tmpHead, ";"));
        if (tmpTail == NULL)
        {
            tmpTail = tail;
        }
        aMime += tmpHead;
    }
    return true;
}

///////////////////////
// End Low-level port processing
///////////////////////


//////////////////////////////
// Logging & Diagnostics
//////////////////////////////

void PVMFSocketNode::LogRTPHeaderFields(SocketPortConfig& aSockConfig,
                                        OsclRefCounterMemFrag& memFragIn)
{
    OSCL_UNUSED_ARG(aSockConfig);
    uint8* rtpHeader = NULL;
    uint32 rtpPacketLen = 0;

    /* Get start of RTP packet */
    rtpHeader    = (uint8*)(memFragIn.getMemFrag().ptr);
    rtpPacketLen = memFragIn.getMemFrag().len;

    if ((rtpHeader != NULL) && (rtpPacketLen > 0))
    {
        /* is this a legal data packet? */
        if (rtpPacketLen <= 12)
        {
            PVMF_SOCKETNODE_LOGDATATRAFFIC_E((0, "PVMFSocketNode - illegal RTP pkt size"));
        }

        /* Parse RTP version */
        uint8  rtpVersion = (((*rtpHeader) & 0xC0) >> 6);
        if (rtpVersion != 2)
        {
            PVMF_SOCKETNODE_LOGDATATRAFFIC_E((0, "PVMFSocketNode illegal rtp version"));
        }

        /* Check for padding */
        uint8 pbit = (((*rtpHeader) & 0x20) >> 5);
        uint8 numPaddingOctets = 0;
        if (pbit)
        {
            numPaddingOctets = *(rtpHeader + (rtpPacketLen - 1));
        }

        // skip over xbit, csrc and mbit
        rtpHeader++;

        rtpHeader++;

        /* Parse sequence number */
        uint16 seqNum16 = 0;
        oscl_memcpy((char *)&seqNum16, rtpHeader, sizeof(seqNum16));
        big_endian_to_host((char *)&seqNum16, sizeof(seqNum16));
        rtpHeader += 2;

        /* Parse rtp time stamp */
        uint32 ts32 = 0;
        oscl_memcpy((char *)&ts32, rtpHeader, sizeof(ts32));
        big_endian_to_host((char *)&ts32, sizeof(ts32));
        rtpHeader += 4;

        /* Parse SSRC */
        uint32 ssrc32 = 0;
        oscl_memcpy((char *)&ssrc32, rtpHeader, sizeof(ssrc32));
        big_endian_to_host((char *)&ssrc32, sizeof(ssrc32));
        rtpHeader += 4;

        PVMF_SOCKETNODE_LOGDATATRAFFIC_RTP((0, "SN-RTP - Mime=%s, ssrc=%d, seq=%d, ts=%d, len=%d",
                                            aSockConfig.iMime.get_cstr(), ssrc32, (uint32)seqNum16, (PVMFTimestamp)ts32, rtpPacketLen));
    }
}

void PVMFSocketNode::LogRTCPHeaderFields(SocketPortConfig& aSockConfig,
        OsclRefCounterMemFrag& memFragIn)
{
    OSCL_UNUSED_ARG(aSockConfig);
    const uint8 SR_PACKET_TYPE = 200;
    const uint8 RR_PACKET_TYPE = 201;
    const uint8 SDES_PACKET_TYPE = 202;
    const uint8 BYE_PACKET_TYPE = 203;
    const uint8 APP_PACKET_TYPE = 204;
    uint8* rtcpHeader = NULL;
    int32 rtcpPacketLen = 0;

    /* Get start of RTP packet */
    rtcpHeader    = (uint8*)(memFragIn.getMemFrag().ptr);
    rtcpPacketLen = (int32)(memFragIn.getMemFrag().len);

    if ((rtcpHeader != NULL) && (rtcpPacketLen > 0))
    {
        while (rtcpPacketLen > 0)
        {
            OsclBinIStreamBigEndian inStream;
            inStream.Attach(rtcpHeader, rtcpPacketLen);
            uint8 tempChar;
            inStream >> tempChar;
            if (inStream.fail())
            {
                PVMF_SOCKETNODE_LOGDATATRAFFIC_RTCP((0, "SN-RTCP - Mime=%s, Invalid RTCP Packet",
                                                     aSockConfig.iMime.get_cstr()));
                return;
            }
            // read the type
            uint8 payloadType;
            uint16 rtcpLength;
            inStream >> payloadType;
            inStream >> rtcpLength;
            if (inStream.fail())
            {
                PVMF_SOCKETNODE_LOGDATATRAFFIC_RTCP((0, "SN-RTCP - Mime=%s, Invalid RTCP Packet Headers",
                                                     aSockConfig.iMime.get_cstr()));
                return;
            }
            switch (payloadType)
            {
                case SR_PACKET_TYPE:
                {
                    PVMF_SOCKETNODE_LOGDATATRAFFIC_RTCP((0, "SN-RTCP - Mime=%s, RTCP_SR RECVD",
                                                         aSockConfig.iMime.get_cstr()));
                }
                break;

                case RR_PACKET_TYPE:
                {
                    PVMF_SOCKETNODE_LOGDATATRAFFIC_RTCP((0, "SN-RTCP - Mime=%s, RTCP_RR RECVD",
                                                         aSockConfig.iMime.get_cstr()));
                }
                break;

                case SDES_PACKET_TYPE:
                {
                    PVMF_SOCKETNODE_LOGDATATRAFFIC_RTCP((0, "SN-RTCP - Mime=%s, RTCP_SDES_PKT RECVD",
                                                         aSockConfig.iMime.get_cstr()));
                }
                break;

                case BYE_PACKET_TYPE:
                {
                    PVMF_SOCKETNODE_LOGDATATRAFFIC_RTCP((0, "SN-RTCP - Mime=%s, RTCP_BYE RECVD",
                                                         aSockConfig.iMime.get_cstr()));
                }
                break;

                case APP_PACKET_TYPE:
                {
                    PVMF_SOCKETNODE_LOGDATATRAFFIC_RTCP((0, "SN-RTCP - Mime=%s, RTCP_APP_PKT RECVD",
                                                         aSockConfig.iMime.get_cstr()));
                }
                break;

                default:
                    break;
            }
            rtcpPacketLen -= (rtcpLength + 1) * 4;
            rtcpHeader += (rtcpLength + 1) * 4;
            if (rtcpLength)
            {
                inStream.seekFromCurrentPosition((rtcpLength*4));
                if (inStream.fail())
                {
                    PVMF_SOCKETNODE_LOGDATATRAFFIC_RTCP((0, "SN-RTCP - Mime=%s, RTCP Packet Read Failed",
                                                         aSockConfig.iMime.get_cstr()));
                    return;
                }
            }
        } //end of while loop
    }
}

//////////////////////////////
// End Logging & Diagnostics
//////////////////////////////


///////////////////////////////////
// AO
///////////////////////////////////

void PVMFSocketNode::Run()
{
    PVMF_SOCKETNODE_LOGSTACKTRACE((0, "PVMFSocketNode::Run() In"));

#if(ENABLE_SOCKET_NODE_STATS)
    iSocketNodeStats.iNumRun++;
#endif

    if (!iClosedTCPSocketVector.empty())
        CleanupClosedTCPSockets();
    if (!iClosedUDPSocketVector.empty())
        CleanupClosedUDPSockets();
    if (!iClosedDNSVector.empty())
        CleanupClosedDNS();

    //Process node commands
    while (CanProcessCommand())
        ProcessCommand(iPendingCmdQueue, iPendingCmdQueue.front());
}

int32 PVMFSocketNode::SocketPlacementNew(PVMFSocketNodeMemPool *& aMemPool, OsclAny *aMemPtr, int32 aMemPoolNumBufs)
{
    int32 errcode = 0;
    OSCL_TRY(errcode, aMemPool = OSCL_PLACEMENT_NEW(aMemPtr, PVMFSocketNodeMemPool(aMemPoolNumBufs);));
    return errcode;
}

int32 PVMFSocketNode::CreateMediaData(SocketPortConfig& aSockConfig, OsclSharedPtr< PVMFMediaDataImpl > &aMediaptr)
{
    int32 err = 0;
    OSCL_TRY_NO_TLS(iOsclErrorTrapImp, err,
                    aSockConfig.iPendingRecvMediaData = PVMFMediaData::createMediaData(aMediaptr, aSockConfig.iMemPool->iMediaDataMemPool););
    return err;
}

int32 PVMFSocketNode::Allocate(SocketPortConfig &aSockConfig, OsclSharedPtr< PVMFMediaDataImpl > &aMediaFragGroup)
{
#if SNODE_ENABLE_UDP_MULTI_PACKET
    int32 err = 0;
    OSCL_TRY_NO_TLS(iOsclErrorTrapImp, err,
                    aMediaFragGroup = aSockConfig.iMemPool->iMediaFragGroupAlloc->allocate(););
    return err;
#else
    return OsclErrNotSupported;
#endif
}

int32 PVMFSocketNode::GetMediaDataImpl(SocketPortConfig& aSockConfig, OsclSharedPtr<PVMFMediaDataImpl> &aMediaDataImpl, int32 aMemSize)
{
    int32 err = 0;
    aMediaDataImpl = aSockConfig.iMemPool->getMediaDataImpl(aMemSize);
    if (NULL == aMediaDataImpl.GetRep())
    {
        err = OsclErrNoMemory;
    }
    return err;
}

