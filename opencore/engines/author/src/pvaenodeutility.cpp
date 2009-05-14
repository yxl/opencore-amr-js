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
/**
 * @file pvaenodeutility.cpp
 * Utility class to perform node operations
 */

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif
#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif
#ifndef PVAE_NODE_UTILITY_H_INCLUDED
#include "pvaenodeutility.h"
#endif
#ifndef PVAE_TUNEABLES_H_INCLUDED
#include "pvae_tuneables.h"
#endif

////////////////////////////////////////////////////////////////////////////
PVAuthorEngineNodeUtility::PVAuthorEngineNodeUtility() :
        OsclTimerObject(OsclActiveObject::EPriorityNominal, "PVAuthorEngineNodeUtility"),
        iObserver(NULL),
        iCmdQueue(PVAE_CMD_VECTOR_RESERVE)
{
    iLogger = PVLogger::GetLoggerObject("PVAuthorEngineNodeUtility");
    AddToScheduler();
}

////////////////////////////////////////////////////////////////////////////
PVAuthorEngineNodeUtility::~PVAuthorEngineNodeUtility()
{
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVAuthorEngineNodeUtility::Connect(PVAENodeContainer* aMasterNode, int32 aTag1,
        PVAENodeContainer* aSlaveNode, int32 aTag2,
        const PvmfMimeString& aMimeString,
        OsclAny* aContext)
{
    LOG_STACK_TRACE((0, "PVAuthorEngineNodeUtility::Connect: &aMasterNode=0x%x, aTag1=%d, &aSlaveNode=0x%x, aTag2=%d",
                     &aMasterNode, aTag1, &aSlaveNode, aTag2));

    PVAENodeUtilCmd cmd;
    PVMFStatus status = cmd.ConstructConnect(aMasterNode, aTag1, aSlaveNode, aTag2, aMimeString, aContext);
    if (status != PVMFSuccess)
    {
        LOG_ERR((0, "PVAuthorEngineNodeUtility::Connect: Error - cmd.ConstructConnect failed. status=%d", status));
        return status;
    }

    return AddCmdToQueue(cmd);
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVAuthorEngineNodeUtility::Disconnect(PVAENodeContainer* aNodeContainer, OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVAuthorEngineNodeUtility::Disconnect: &aNodeContainer=0x%x, aContext=0x%x",
                     &aNodeContainer, aContext));

    PVAENodeUtilCmd cmd;
    PVMFStatus status = cmd.ConstructDisconnect(aNodeContainer, aContext);
    if (status != PVMFSuccess)
    {
        LOG_ERR((0, "PVAuthorEngineNodeUtility::Disconnect: Error - cmd.ConstructDisconnect failed. status=%d", status));
        return status;
    }

    return AddCmdToQueue(cmd);
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVAuthorEngineNodeUtility::QueryUUID(PVAENodeContainer* aNodeContainer,
        const PvmfMimeString& aMimeType,
        Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids,
        bool aExactUuidsOnly,
        OsclAny* aContext)
{
    LOG_STACK_TRACE((0, "PVAuthorEngineNodeUtility::QueryUUID: &aNodeContainer=0x%x, aMimeType=%s, &aUuids=0x%x, aExactUuidsOnly=%d, aContext=0x%x",
                     &aNodeContainer, aMimeType.get_cstr(), &aUuids, aExactUuidsOnly, aContext));

    PVAENodeUtilCmd cmd;
    PVMFStatus status = cmd.ConstructQueryUUID(aNodeContainer, aMimeType, aUuids, aExactUuidsOnly, aContext);
    if (status != PVMFSuccess)
    {
        LOG_ERR((0, "PVAuthorEngineNodeUtility::QueryUUID: Error - cmd.ConstructQueryUUID failed. status=%d", status));
        return status;
    }

    return AddCmdToQueue(cmd);
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVAuthorEngineNodeUtility::QueryInterface(PVAENodeContainer* aNodeContainer,
        const PVUuid& aUuid,
        PVInterface*& aInterfacePtr,
        OsclAny* aContext)
{
    LOG_STACK_TRACE((0, "PVAuthorEngineNodeUtility::QueryInterface: &aNodeContainer=0x%x, &aUuid=0x%x, aContext=0x%x",
                     &aNodeContainer, &aUuid, aContext));

    PVAENodeUtilCmd cmd;
    PVMFStatus status = cmd.ConstructQueryInterface(aNodeContainer, aUuid, aInterfacePtr, aContext);
    if (status != PVMFSuccess)
    {
        LOG_ERR((0, "PVAuthorEngineNodeUtility::QueryInterface: Error - cmd.ConstructQueryInterface failed. status=%d", status));
        return status;
    }

    return AddCmdToQueue(cmd);
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVAuthorEngineNodeUtility::Init(const PVAENodeContainer* aNode, OsclAny* aContext)
{
    LOG_STACK_TRACE((0, "PVAuthorEngineNodeUtility::Init: aNode=0x%x, aContext=0x%x", aNode, aContext));

    PVAENodeUtilCmd cmd;
    PVMFStatus status = cmd.ConstructInit((PVAENodeContainer*)aNode, aContext);
    if (status != PVMFSuccess)
    {
        LOG_ERR((0, "PVAuthorEngineNodeUtility::Init: Error - cmd.Construct failed. status=%d", status));
        return status;
    }

    return AddCmdToQueue(cmd);
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVAuthorEngineNodeUtility::Init(const PVAENodeContainerVector& aNodes, OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVAuthorEngineNodeUtility::Init: &aNodes=0x%x, aContext=0x%x", &aNodes, aContext));


    PVAENodeUtilCmd cmd;
    PVMFStatus status = cmd.Construct(PVAENU_CMD_INIT, aNodes, aContext);
    if (status != PVMFSuccess)
    {
        LOG_ERR((0, "PVAuthorEngineNodeUtility::Init: Error - cmd.Construct failed. status=%d", status));
        return status;
    }

    return AddCmdToQueue(cmd);
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVAuthorEngineNodeUtility::Prepare(const PVAENodeContainerVector& aNodes, OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVAuthorEngineNodeUtility::Prepare: &aNodes=0x%x, aContext=0x%x", &aNodes, aContext));

    PVAENodeUtilCmd cmd;
    PVMFStatus status = cmd.Construct(PVAENU_CMD_PREPARE, aNodes, aContext);
    if (status != PVMFSuccess)
    {
        LOG_ERR((0, "PVAuthorEngineNodeUtility::Prepare: Error - cmd.Construct failed. status=%d", status));
        return status;
    }

    return AddCmdToQueue(cmd);
}


////////////////////////////////////////////////////////////////////////////
PVMFStatus PVAuthorEngineNodeUtility::Start(const PVAENodeContainerVector& aNodes, OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVAuthorEngineNodeUtility::Start: &aNodes=0x%x, aContext=0x%x", &aNodes, aContext));

    PVAENodeUtilCmd cmd;
    PVMFStatus status = cmd.Construct(PVAENU_CMD_START, aNodes, aContext);
    if (status != PVMFSuccess)
    {
        LOG_ERR((0, "PVAuthorEngineNodeUtility::Start: Error - cmd.Construct failed. status=%d", status));
        return status;
    }

    return AddCmdToQueue(cmd);
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVAuthorEngineNodeUtility::Stop(const PVAENodeContainerVector& aNodes, OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVAuthorEngineNodeUtility::Stop: &aNodes=0x%x, aContext=0x%x", &aNodes, aContext));

    PVAENodeUtilCmd cmd;
    PVMFStatus status = cmd.Construct(PVAENU_CMD_STOP, aNodes, aContext);
    if (status != PVMFSuccess)
    {
        LOG_ERR((0, "PVAuthorEngineNodeUtility::Stop: Error - cmd.Construct failed. status=%d", status));
        return status;
    }

    return AddCmdToQueue(cmd);
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVAuthorEngineNodeUtility::Pause(const PVAENodeContainerVector& aNodes, OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVAuthorEngineNodeUtility::Pause: &aNodes=0x%x, aContext=0x%x", &aNodes, aContext));

    PVAENodeUtilCmd cmd;
    PVMFStatus status = cmd.Construct(PVAENU_CMD_PAUSE, aNodes, aContext);
    if (status != PVMFSuccess)
    {
        LOG_ERR((0, "PVAuthorEngineNodeUtility::Pause: Error - cmd.Construct failed. status=%d", status));
        return status;
    }

    return AddCmdToQueue(cmd);
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVAuthorEngineNodeUtility::Flush(const PVAENodeContainerVector& aNodes, OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVAuthorEngineNodeUtility::Flush: &aNodes=0x%x, aContext=0x%x", &aNodes, aContext));

    PVAENodeUtilCmd cmd;
    PVMFStatus status = cmd.Construct(PVAENU_CMD_FLUSH, aNodes, aContext);
    if (status != PVMFSuccess)
    {
        LOG_ERR((0, "PVAuthorEngineNodeUtility::Flush: Error - cmd.Construct failed. status=%d", status));
        return status;
    }

    return AddCmdToQueue(cmd);
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVAuthorEngineNodeUtility::Reset(const PVAENodeContainerVector& aNodes, OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVAuthorEngineNodeUtility::Reset: &aNodes=0x%x, aContext=0x%x", &aNodes, aContext));

    PVAENodeUtilCmd cmd;
    PVMFStatus status = cmd.Construct(PVAENU_CMD_RESET, aNodes, aContext);
    if (status != PVMFSuccess)
    {
        LOG_ERR((0, "PVAuthorEngineNodeUtility::Reset: Error - cmd.Construct failed. status=%d", status));
        return status;
    }

    return AddCmdToQueue(cmd);
}

////////////////////////////////////////////////////////////////////////////
uint32 PVAuthorEngineNodeUtility::GetCommandQueueSize()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVAuthorEngineNodeUtility::GetCommandQueueSize: size=%d", iCmdQueue.size()));
    return iCmdQueue.size();
}

////////////////////////////////////////////////////////////////////////////
void PVAuthorEngineNodeUtility::NodeCommandCompleted(const PVMFCmdResp& aResponse)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVAuthorEngineNodeUtility::NodeCommandCompleted"));

    if (aResponse.GetCmdStatus() != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                        (0, "PVAuthorEngineNodeUtility::NodeCommandCompleted: Command failed - context=0x%x, status=0x%x",
                         aResponse.GetContext(), aResponse.GetCmdStatus()));
        CompleteUtilityCmd(iCmdQueue[0], aResponse.GetCmdStatus());
        return;
    }

    if (iCmdQueue.empty())
    {
        LOG_ERR((0, "PVAuthorEngineNodeUtility::NodeCommandCompleted: Error - Empty command queue"));
        PVMFAsyncEvent event(PVMFErrorEvent, PVMFFailure, NULL, NULL);
        iObserver->NodeUtilErrorEvent(event);
        return;
    }

    PVMFStatus status = PVMFSuccess;
    PVAENodeUtilCmd cmd = iCmdQueue[0];
    switch (cmd.iType)
    {
        case PVAENU_CMD_CONNECT:
            status = CompleteConnect(cmd, aResponse);
            break;
        case PVAENU_CMD_DISCONNECT:
            status = DoDisconnect(cmd);
            break;
        case PVAENU_CMD_QUERY_UUID:
            status = PVMFSuccess;
            break;
        case PVAENU_CMD_QUERY_INTERFACE:
            status = CompleteQueryInterface(cmd);
            break;
        case PVAENU_CMD_INIT:
            status = CompleteStateTransition(cmd, EPVMFNodeInitialized);
            break;
        case PVAENU_CMD_PREPARE:
            status = CompleteStateTransition(cmd, EPVMFNodePrepared);
            break;
        case PVAENU_CMD_START:
            status = CompleteStateTransition(cmd, EPVMFNodeStarted);
            break;
        case PVAENU_CMD_PAUSE:
            status = CompleteStateTransition(cmd, EPVMFNodePaused);
            break;
        case PVAENU_CMD_STOP:
        case PVAENU_CMD_FLUSH:
            status = CompleteStateTransition(cmd, EPVMFNodePrepared);
            break;
        case PVAENU_CMD_RESET:
            status = CompleteStateTransition(cmd, EPVMFNodeIdle);
            break;
        default:
            status = PVMFFailure;
            break;
    }

    if (status != PVMFPending)
        CompleteUtilityCmd(cmd, status);
}

////////////////////////////////////////////////////////////////////////////
void PVAuthorEngineNodeUtility::Run()
{
    LOG_STACK_TRACE((0, "PVAuthorEngineNodeUtility::Run: Enter"));

    if (iCmdQueue.empty())
        return;

    PVAENodeUtilCmd cmd = iCmdQueue[0];
    if (cmd.iNodes.empty())
    {
        LOG_ERR((0, "PVAuthorEngineNodeUtility::Run: Error - cmd.iNodes is empty"));
        CompleteUtilityCmd(cmd, PVMFFailure);
        return;
    }

    PVMFStatus status = PVMFFailure;
    LOG_STACK_TRACE((0, "PVAuthorEngineNodeUtility::Run: cmd.iType=%d", cmd.iType));
    switch (cmd.iType)
    {
        case PVAENU_CMD_CONNECT:
            status = DoConnect(cmd);
            break;
        case PVAENU_CMD_DISCONNECT:
            status = DoDisconnect(cmd);
            break;
        case PVAENU_CMD_QUERY_UUID:
            status = DoQueryUuid(cmd);
            break;
        case PVAENU_CMD_QUERY_INTERFACE:
            status = DoQueryInterface(cmd);
            break;
        case PVAENU_CMD_INIT:
            status = DoInit(cmd);
            break;
        case PVAENU_CMD_PREPARE:
            status = DoPrepare(cmd);
            break;
        case PVAENU_CMD_START:
            status = DoStart(cmd);
            break;
        case PVAENU_CMD_PAUSE:
            status = DoPause(cmd);
            break;
        case PVAENU_CMD_STOP:
            status = DoStop(cmd);
            break;
        case PVAENU_CMD_FLUSH:
            status = DoFlush(cmd);
            break;
        case PVAENU_CMD_RESET:
            status = DoReset(cmd);
            break;
        default:
            status = PVMFFailure;
            break;
    }

    if (status != PVMFPending)
        CompleteUtilityCmd(cmd, status);

    LOG_STACK_TRACE((0, "PVAuthorEngineNodeUtility::Run: Exit"));
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVAuthorEngineNodeUtility::AddCmdToQueue(PVAENodeUtilCmd& aCmd)
{
    LOG_STACK_TRACE((0, "PVAuthorEngineNodeUtility::AddCmdToQueue"));

    int32 err = 0;
    OSCL_TRY(err, iCmdQueue.push_back(aCmd););
    OSCL_FIRST_CATCH_ANY(err,
                         LOG_ERR((0, "PVAuthorEngineNodeUtility::AddCmdToQueue: Error - iCmdQueue.push_back() failed"));
                         return PVMFErrNoMemory;
                        );

    if (iCmdQueue.size() == 1)
    {
        // Call RunIfNotReady() only if there are no other commands
        // in the queue. Otherwise, wait for the previous one(s)
        // to complete. RunIfNotReady() will be called in
        // CompleteUtilityCmd()
        RunIfNotReady();
    }

    return PVMFPending;
}

////////////////////////////////////////////////////////////////////////////
void PVAuthorEngineNodeUtility::CompleteUtilityCmd(const PVAENodeUtilCmd& aCmd, PVMFStatus aStatus)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVAuthorEngineNodeUtility::CompleteUtilityCmd: aCmd.iType=%d,  aStatus=0x%x", aCmd.iType, aStatus));

    if (!iObserver)
    {
        LOG_ERR((0, "PVAuthorEngineNodeUtility::CompleteUtilityCmd: Error - Observer not set"));
        OSCL_LEAVE(OsclErrGeneral);
        // return;	This statement was removed to avoid compiler warning for Unreachable Code
    }

    if (iCmdQueue.empty() || aCmd.iType != iCmdQueue[0].iType)
    {
        LOG_ERR((0, "PVAuthorEngineNodeUtility::CompleteUtilityCmd: Error - Empty command queue or mismatched command"));
        PVMFAsyncEvent event(PVMFErrorEvent, PVMFFailure, NULL, NULL);
        iObserver->NodeUtilErrorEvent(event);
        return;
    }

    // Remove command from queue
    iCmdQueue.erase(iCmdQueue.begin());

    // Callback to engine
    PVMFCmdResp resp(0, aCmd.iContext, aStatus);
    iObserver->NodeUtilCommandCompleted(resp);

    // Run next command when needed
    if (!iCmdQueue.empty())
        RunIfNotReady();
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVAuthorEngineNodeUtility::DoConnect(const PVAENodeUtilCmd& aCmd)
{
    LOG_STACK_TRACE((0, "PVAuthorEngineNodeUtility::DoConnect"));
    int32 err = 0;
    OSCL_TRY(err,
             int32 tag1 = 0;
             int32 tag2 = 0;
             OSCL_HeapString<OsclMemAllocator> mimeType;
             PVMFStatus status = ((PVAENodeUtilCmd)aCmd).ParseConnect(tag1, tag2, mimeType);
             if (status != PVMFSuccess)
{
    LOG_ERR((0, "PVAuthorEngineNodeUtility::DoConnect: Error - cmd.ParseConnect failed. status=%d", status));
        return status;
    }

    if (mimeType.get_size() > 0)
{
    // Request a port from the master node
    aCmd.iNodes[0]->iNode->RequestPort(aCmd.iNodes[0]->iSessionId, tag1,
                                           &mimeType, (OsclAny*)aCmd.iNodes[0]);
    }
    else
    {
        // Request a port from the master node
        aCmd.iNodes[0]->iNode->RequestPort(aCmd.iNodes[0]->iSessionId, tag1,
                                           NULL, (OsclAny*)aCmd.iNodes[0]);
    }
            );

    OSCL_FIRST_CATCH_ANY(err,
                         LOG_ERR((0, "PVAuthorEngineNodeUtility::DoConnect: Error - RequestPort failed. node=0x%x",
                                  aCmd.iNodes[0]->iNode));
                         return PVMFFailure;
                        );

    return PVMFPending;
}


// This is a work-around function to eliminate the compiler warning on certain platforms:
// "Variable might be clobbered by 'longjmp' or vfork'
static int32 requestport(PVMFNodeInterface* n, int32 id, int32 t2, OSCL_String& mt, OsclAny* p)
{
    int32 err;
    OSCL_TRY(err,
             if (mt.get_size() > 0)
{
    n->RequestPort(id, t2, &mt, p);
    }
    else
    {
        n->RequestPort(id, t2, NULL, p);
    }
            );
    return err;
}


////////////////////////////////////////////////////////////////////////////
PVMFStatus PVAuthorEngineNodeUtility::CompleteConnect(const PVAENodeUtilCmd& aCmd, const PVMFCmdResp& aResponse)
{
    LOG_STACK_TRACE((0, "PVAuthorEngineNodeUtility::CompleteConnect"));

    PVAENodeContainer* nodeContainer = OSCL_REINTERPRET_CAST(PVAENodeContainer*, aResponse.GetContext());
    PVMFPortInterface* port = OSCL_REINTERPRET_CAST(PVMFPortInterface*, aResponse.GetEventData());
    if (!port || !nodeContainer)
    {
        LOG_ERR((0, "PVAuthorEngineNodeUtility::CompleteConnect: Error - Invalid port or node container"));
        return PVMFFailure;
    }

    int32 tag1 = 0;
    int32 tag2 = 0;
    OSCL_HeapString<OsclMemAllocator> mimeType;
    PVMFStatus status = ((PVAENodeUtilCmd)aCmd).ParseConnect(tag1, tag2, mimeType);

    if (status != PVMFSuccess)
    {
        LOG_ERR((0, "PVAuthorEngineNodeUtility::CompleteConnect: Error - cmd.ParseConnect failed. status=%d", status));
        return status;
    }

    int32 err = 0;
    // Add port to port vector of node container
    switch (tag1)
    {
        case PVAE_NODE_INPUT_PORT_TAG:
            nodeContainer->iInputPorts.push_back(port);
            break;
        case PVAE_NODE_OUTPUT_PORT_TAG:
            nodeContainer->iOutputPorts.push_back(port);
            break;
        default:
            LOG_ERR((0, "PVAuthorEngineNodeUtility::CompleteConnect: Error - Unsupported port tag"));
            OSCL_LEAVE(OsclErrNotSupported);
            break;
    }

    if (nodeContainer == aCmd.iNodes[0])
    {
        err = requestport(aCmd.iNodes[1]->iNode, aCmd.iNodes[1]->iSessionId, tag2, mimeType, (OsclAny*)aCmd.iNodes[1]);

        OSCL_FIRST_CATCH_ANY(err,
                             LOG_ERR((0, "PVAuthorEngineNodeUtility::CompleteConnect: Error - RequestPort failed. node=0x%x",
                                      aCmd.iNodes[1]->iNode));
                             return PVMFFailure;
                            );

        status = PVMFPending;
    }
    else if (nodeContainer == aCmd.iNodes[1])
    {
        // Connect the ports
        PVMFPortInterface* masterPort = NULL;
        switch (tag1)
        {
            case PVAE_NODE_INPUT_PORT_TAG:
                masterPort = aCmd.iNodes[0]->iInputPorts.back();
                break;
            case PVAE_NODE_OUTPUT_PORT_TAG:
                masterPort = aCmd.iNodes[0]->iOutputPorts.back();
                break;
            default:
                LOG_ERR((0, "PVAuthorEngineNodeUtility::CompleteConnect: Error - Unsupported port tag"));
                return PVMFErrNotSupported;
        }

        if (!masterPort)
        {
            LOG_ERR((0, "PVAuthorEngineNodeUtility::CompleteConnect: Error - Master port unavailable"));
            return PVMFFailure;
        }
        status = masterPort->Connect(port);
    }
    else
    {
        LOG_ERR((0, "PVAuthorEngineNodeUtility::CompleteConnect: Error - Invalid port"));
        status = PVMFFailure;
    }

    return status;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVAuthorEngineNodeUtility::DoDisconnect(const PVAENodeUtilCmd& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVAuthorEngineNodeUtility::DoDisconnect"));

    PVMFPortInterface* port = NULL;
    PVAENodeContainer* container = aCmd.iNodes[0];

    if (!container->iInputPorts.empty())
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_DEBUG,
                        (0, "PVAuthorEngineNodeUtility::DoDisconnect: Release input port=0x%x", port));
        port = container->iInputPorts[0];
        container->iInputPorts.erase(container->iInputPorts.begin());
    }
    else if (!container->iOutputPorts.empty())
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_DEBUG,
                        (0, "PVAuthorEngineNodeUtility::DoDisconnect: Release output port=0x%x", port));
        port = container->iOutputPorts[0];
        container->iOutputPorts.erase(container->iOutputPorts.begin());
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_DEBUG,
                        (0, "PVAuthorEngineNodeUtility::DoDisconnect: All ports are released"));
        return PVMFSuccess;
    }

    if (PVMFFailure == ReleasePort(container, port))
        return PVMFFailure;

    return PVMFPending;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVAuthorEngineNodeUtility::DoQueryUuid(const PVAENodeUtilCmd& aCmd)
{
    LOG_STACK_TRACE((0, "PVAuthorEngineNodeUtility::DoQueryUuid"));

    Oscl_Vector<PVUuid, OsclMemAllocator>* uuids = NULL;
    bool exactUuidsOnly = false;
    PVMFStatus status = ((PVAENodeUtilCmd)aCmd).ParseQueryUUID(uuids, exactUuidsOnly);
    if (status != PVMFSuccess || !uuids)
    {
        LOG_ERR((0, "PVAuthorEngineNodeUtility::DoQueryUuid: Error - aCmd.ParseQueryUUID failed. status=%d", status));
        return status;
    }

    int32 err = 0;
    OSCL_TRY(err,
             aCmd.iNodes[0]->iNode->QueryUUID(aCmd.iNodes[0]->iSessionId, aCmd.iMimeType,
                                              *uuids, exactUuidsOnly, aCmd.iContext);
            );

    OSCL_FIRST_CATCH_ANY(err,
                         LOG_ERR((0, "PVAuthorEngineNodeUtility::DoQueryUuid: Error - QueryUUID failed. node=0x%x",
                                  aCmd.iNodes[0]->iNode));
                         return PVMFFailure;
                        );

    return PVMFPending;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVAuthorEngineNodeUtility::DoQueryInterface(const PVAENodeUtilCmd& aCmd)
{
    LOG_STACK_TRACE((0, "PVAuthorEngineNodeUtility::DoQueryInterface"));

    int32 err = 0;
    OSCL_TRY(err,
             PVInterface** interfacePtr = NULL;
             PVMFStatus status = ((PVAENodeUtilCmd)aCmd).ParseQueryInterface(interfacePtr);
             if (status != PVMFSuccess || !interfacePtr)
{
    LOG_ERR((0, "PVAuthorEngineNodeUtility::DoQueryInterface: Error - aCmd.ParseQueryInterface failed"));
        return PVMFFailure;
    }

    aCmd.iNodes[0]->iNode->QueryInterface(aCmd.iNodes[0]->iSessionId, aCmd.iUuid,
                                          (PVInterface*&)*interfacePtr, aCmd.iContext);
            );

    OSCL_FIRST_CATCH_ANY(err,
                         LOG_ERR((0, "PVAuthorEngineNodeUtility::DoQueryInterface: Error - QueryInterface failed. node=0x%x",
                                  aCmd.iNodes[0]->iNode));
                         return PVMFFailure;
                        );

    return PVMFPending;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVAuthorEngineNodeUtility::CompleteQueryInterface(const PVAENodeUtilCmd& aCmd)
{
    LOG_STACK_TRACE((0, "PVAuthorEngineNodeUtility::CompleteQueryInterface"));

    int32 err = 0;
    OSCL_TRY(err,
             PVInterface** interfacePtr = NULL;
             PVMFStatus status = ((PVAENodeUtilCmd)aCmd).ParseQueryInterface(interfacePtr);
             if (status != PVMFSuccess || !interfacePtr)
{
    LOG_ERR((0, "PVAuthorEngineNodeUtility::CompleteQueryInterface: Error - aCmd.ParseQueryInterface failed"));
        return PVMFFailure;
    }
    aCmd.iNodes[0]->iExtensionUuids.push_back(aCmd.iUuid);
    (*interfacePtr)->addRef();
    aCmd.iNodes[0]->iExtensions.push_back(*interfacePtr);
            );

    OSCL_FIRST_CATCH_ANY(err,
                         LOG_ERR((0, "PVAuthorEngineNodeUtility::CompleteQueryInterface: Error - Adding extension to node container failed."));
                         return PVMFFailure;
                        );

    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVAuthorEngineNodeUtility::DoInit(const PVAENodeUtilCmd& aCmd)
{
    LOG_STACK_TRACE((0, "PVAuthorEngineNodeUtility::DoInit"));

    int32 err = 0;
    PVAENodeContainer* nodeContainer = NULL;
    OSCL_TRY(err,
             for (uint32 i = 0; i < aCmd.iNodes.size(); i++)
{
    nodeContainer = aCmd.iNodes[i];
        nodeContainer->iNode->Init(nodeContainer->iSessionId, aCmd.iContext);
    }
            );

    OSCL_FIRST_CATCH_ANY(err,
                         LOG_ERR((0, "PVAuthorEngineNodeUtility::DoInit: Error - Init failed. node=0x%x",
                                  nodeContainer->iNode));
                         return PVMFFailure;
                        );

    return PVMFPending;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVAuthorEngineNodeUtility::DoPrepare(const PVAENodeUtilCmd& aCmd)
{
    LOG_STACK_TRACE((0, "PVAuthorEngineNodeUtility::DoPrepare"));

    int32 err = 0;
    PVAENodeContainer* nodeContainer = NULL;
    OSCL_TRY(err,
             for (uint32 i = 0; i < aCmd.iNodes.size(); i++)
{
    nodeContainer = aCmd.iNodes[i];
        nodeContainer->iNode->Prepare(nodeContainer->iSessionId, aCmd.iContext);
    }
            );

    OSCL_FIRST_CATCH_ANY(err,
                         LOG_ERR((0, "PVAuthorEngineNodeUtility::DoPrepare: Error - Prepare failed. node=0x%x",
                                  nodeContainer->iNode));
                         return PVMFFailure;
                        );

    return PVMFPending;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVAuthorEngineNodeUtility::DoStart(const PVAENodeUtilCmd& aCmd)
{
    LOG_STACK_TRACE((0, "PVAuthorEngineNodeUtility::DoStart"));

    int32 err = 0;
    PVAENodeContainer* nodeContainer = NULL;
    OSCL_TRY(err,
             for (uint32 i = 0; i < aCmd.iNodes.size(); i++)
{
    nodeContainer = aCmd.iNodes[i];
        nodeContainer->iNode->Start(nodeContainer->iSessionId, aCmd.iContext);
    }
            );

    OSCL_FIRST_CATCH_ANY(err,
                         LOG_ERR((0, "PVAuthorEngineNodeUtility::DoStart: Error - Start failed. node=0x%x",
                                  nodeContainer->iNode));
                         return PVMFFailure;
                        );

    return PVMFPending;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVAuthorEngineNodeUtility::DoPause(const PVAENodeUtilCmd& aCmd)
{
    LOG_STACK_TRACE((0, "PVAuthorEngineNodeUtility::DoPause"));

    int32 err = 0;
    PVAENodeContainer* nodeContainer = NULL;
    OSCL_TRY(err,
             for (uint32 i = 0; i < aCmd.iNodes.size(); i++)
{
    nodeContainer = aCmd.iNodes[i];
        nodeContainer->iNode->Pause(nodeContainer->iSessionId, aCmd.iContext);
    }
            );

    OSCL_FIRST_CATCH_ANY(err,
                         LOG_ERR((0, "PVAuthorEngineNodeUtility::DoPause: Error - Pause failed. node=0x%x",
                                  nodeContainer->iNode));
                         return PVMFFailure;
                        );

    return PVMFPending;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVAuthorEngineNodeUtility::DoStop(const PVAENodeUtilCmd& aCmd)
{
    LOG_STACK_TRACE((0, "PVAuthorEngineNodeUtility::DoStop"));

    int32 err = 0;
    PVAENodeContainer* nodeContainer = NULL;
    OSCL_TRY(err,
             for (uint32 i = 0; i < aCmd.iNodes.size(); i++)
{
    nodeContainer = aCmd.iNodes[i];
        nodeContainer->iNode->Stop(nodeContainer->iSessionId, aCmd.iContext);
    }
            );

    OSCL_FIRST_CATCH_ANY(err,
                         LOG_ERR((0, "PVAuthorEngineNodeUtility::DoStop: Error - Stop failed. node=0x%x",
                                  nodeContainer->iNode));
                         return PVMFFailure;
                        );

    return PVMFPending;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVAuthorEngineNodeUtility::DoFlush(const PVAENodeUtilCmd& aCmd)
{
    LOG_STACK_TRACE((0, "PVAuthorEngineNodeUtility::DoFlush"));

    int32 err = 0;
    PVAENodeContainer* nodeContainer = NULL;
    OSCL_TRY(err,
             for (uint32 i = 0; i < aCmd.iNodes.size(); i++)
{
    nodeContainer = aCmd.iNodes[i];
        nodeContainer->iNode->Flush(nodeContainer->iSessionId, aCmd.iContext);
    }
            );

    OSCL_FIRST_CATCH_ANY(err,
                         LOG_ERR((0, "PVAuthorEngineNodeUtility::DoFlush: Error - Flush failed. node=0x%x",
                                  nodeContainer->iNode));
                         return PVMFFailure;
                        );

    return PVMFPending;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVAuthorEngineNodeUtility::DoReset(const PVAENodeUtilCmd& aCmd)
{
    LOG_STACK_TRACE((0, "PVAuthorEngineNodeUtility::DoReset"));

    int32 err = 0;
    PVAENodeContainer* nodeContainer = NULL;
    OSCL_TRY(err,
             for (uint32 i = 0; i < aCmd.iNodes.size(); i++)
{
    nodeContainer = aCmd.iNodes[i];
        for (uint32 j = 0; j < nodeContainer->iExtensions.size(); j++)
            nodeContainer->iExtensions[j]->removeRef();
        nodeContainer->iExtensions.clear();
        nodeContainer->iExtensionUuids.clear();
        nodeContainer->iNode->Reset(nodeContainer->iSessionId, aCmd.iContext);
    }
            );

    OSCL_FIRST_CATCH_ANY(err,
                         LOG_ERR((0, "PVAuthorEngineNodeUtility::DoReset: Error - Reset failed. node=0x%x",
                                  nodeContainer->iNode));
                         return PVMFFailure;
                        );

    return PVMFPending;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVAuthorEngineNodeUtility::CompleteStateTransition(const PVAENodeUtilCmd& aCmd,
        TPVMFNodeInterfaceState aState)
{
    LOG_STACK_TRACE((0, "PVAuthorEngineNodeUtility::CompleteStateTransition: aState=%d", aState));

    //Handle reset differently since we need to thread logoff here
    //if a node state is EPVMFNodeIdle (which is what aState would be if aCmd.iType is PVAENU_CMD_RESET)
    //do threadlogoff and take the node back to created
    if (aCmd.iType == PVAENU_CMD_RESET)
    {
        for (uint32 i = 0; i < aCmd.iNodes.size(); i++)
        {
            if (aCmd.iNodes[i]->iNode->GetState() == aState)
            {
                // reset completed, perform threadlogoff on the node.
                PVMFStatus status = aCmd.iNodes[i]->iNode->ThreadLogoff();
                if (PVMFSuccess != status)
                {
                    LOG_ERR((0, "PVAuthorEngineNodeUtility::CompleteStateTransition: Error - Node ThreadLogoff failed for node=0x%x", aCmd.iNodes[i]->iNode));
                    OSCL_ASSERT(false);
                }
            }
        }
        //now that we have taken the nodes in idle back to created,
        //override aState to EPVMFNodeCreated
        aState = EPVMFNodeCreated;
    }
    for (uint32 i = 0; i < aCmd.iNodes.size(); i++)
    {
        if (aCmd.iNodes[i]->iNode->GetState() != aState)
        {
            // Some nodes have not completed this command. Continue to wait
            return PVMFPending;
        }
    }
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVAuthorEngineNodeUtility::ReleasePort(PVAENodeContainer*& aContainer, PVMFPortInterface*& aPort)
{
    int32 err = 0;
    OSCL_TRY(err,
             aPort->Disconnect();
             aContainer->iNode->ReleasePort(aContainer->iSessionId, *aPort, (OsclAny*)aContainer);
            );
    OSCL_FIRST_CATCH_ANY(err,
                         LOG_ERR((0, "PVAuthorEngineNodeUtility::DoDisconnect: Error - ReleasePort failed"));
                         return PVMFFailure;
                        );
    return PVMFSuccess;
}
