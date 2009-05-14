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
 *
 * @file pvmf_loopback_node.cpp
 * @brief Simple file output node. Writes incoming data to specified
 * file without any media type specific file format
 *
 */
#include "pvlogger.h"
#include "pvmf_loopback_ioport.h"
#include "pvmf_loopback_node.h"

#define LOGERROR(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iLogger,PVLOGMSG_ERR,m);
#define LOGINFOHI(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG,iLogger,PVLOGMSG_INFO,m);
#define LOGINFOMED(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG,iLogger,PVLOGMSG_INFO,m);
#define LOGINFOLOW(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG,iLogger,PVLOGMSG_INFO,m);
#define LOGINFO(m) LOGINFOMED(m)

// Use default DLL entry point for Symbian
#include "oscl_dll.h"

OSCL_DLL_ENTRY_POINT_DEFAULT()

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFNodeInterface *PVMFLoopbackNode::Create()
{
    PVMFLoopbackNode *self = OSCL_NEW(PVMFLoopbackNode, (OsclActiveObject::EPriorityNominal));
    return self;
}

////////////////////////////////////////////////////////////////////////////
PVMFLoopbackNode::PVMFLoopbackNode(int32 aPriority) : OsclActiveObject(aPriority, "LoopbackNode")
        , iCmdIdCounter(0)
        , iIOPort(NULL)
{
    iLogger = NULL;
    int32 err;
    OSCL_TRY(err,

             //Create the input command queue.  Use a reserve to avoid lots of
             //dynamic memory allocation.
             iInputCommands.Construct(PVMF_LOOPBACK_NODE_COMMAND_ID_START, PVMF_LOOPBACK_NODE_COMMAND_VECTOR_RESERVE);

             //Create the "current command" queue.  It will only contain one
             //command at a time, so use a reserve of 1.
             iCurrentCommand.Construct(0, 1);

             //Set the node capability data.
             //This node can support an unlimited number of ports.
             iCapability.iCanSupportMultipleInputPorts = true;
             iCapability.iCanSupportMultipleOutputPorts = true;
             iCapability.iHasMaxNumberOfPorts = false;
             iCapability.iMaxNumberOfPorts = 0;//no maximum
             iCapability.iInputFormatCapability.push_back(PVMF_MIME_PCM16);
             iCapability.iInputFormatCapability.push_back(PVMF_MIME_RGB24);
             iCapability.iInputFormatCapability.push_back(PVMF_MIME_MP3);
             iCapability.iOutputFormatCapability.push_back(PVMF_MIME_PCM16);
             iCapability.iOutputFormatCapability.push_back(PVMF_MIME_RGB24);
             iCapability.iOutputFormatCapability.push_back(PVMF_MIME_MP3);

            );

    if (err != OsclErrNone)
    {
        //if a leave happened, cleanup and re-throw the error
        iInputCommands.clear();
        iCurrentCommand.clear();
        iCapability.iInputFormatCapability.clear();
        iCapability.iOutputFormatCapability.clear();
        OSCL_CLEANUP_BASE_CLASS(PVMFNodeInterface);
        OSCL_CLEANUP_BASE_CLASS(OsclActiveObject);
        OSCL_LEAVE(err);
    }
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFLoopbackNode::~PVMFLoopbackNode()
{

    if (IsAdded())
        RemoveFromScheduler();
    if (iIOPort)
    {
        OSCL_DELETE(iIOPort);
        iIOPort = NULL;
    }

    while (!iCurrentCommand.empty())
    {
        CommandComplete(iCurrentCommand, iCurrentCommand.front(), PVMFFailure);
        iCurrentCommand.Erase(&iCurrentCommand.front());
    }

    while (!iInputCommands.empty())
    {
        CommandComplete(iInputCommands, iInputCommands.front(), PVMFFailure);
        iInputCommands.Erase(&iInputCommands.front());
    }
}

////////////////////////////////////////////////////////////////////////////
void PVMFLoopbackNode::DoCancel()
{
    OsclActiveObject::DoCancel();
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMFLoopbackNode::ThreadLogon()
{

    LOGINFO((0, "loopbackNode:ThreadLogon"));
    switch (iInterfaceState)
    {
        case EPVMFNodeCreated:
            if (!IsAdded())
                AddToScheduler();
            iLogger = PVLogger::GetLoggerObject("PVMFLoopbackNode");
            SetState(EPVMFNodeIdle);
            return PVMFSuccess;
            break;
        default:
            return PVMFErrInvalidState;
            break;
    }
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMFLoopbackNode::ThreadLogoff()
{

    LOGINFO((0, "PVMFLoopbackNode:ThreadLogoff"));
    switch (iInterfaceState)
    {
        case EPVMFNodeIdle:
            if (IsAdded())
                RemoveFromScheduler();
            iLogger = NULL;
            SetState(EPVMFNodeCreated);
            return PVMFSuccess;
            break;

        default:
            return PVMFErrInvalidState;
            break;
    }
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMFLoopbackNode::GetCapability(PVMFNodeCapability& aNodeCapability)
{
    LOGINFO((0, "PVMFLoopbackNode:GetCapability"));
    aNodeCapability = iCapability;
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
/**
//Queue an asynchronous node command
*/
OSCL_EXPORT_REF PVMFCommandId PVMFLoopbackNode::QueryUUID(PVMFSessionId s, const PvmfMimeString& aMimeType,
        Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids,
        bool aExactUuidsOnly,
        const OsclAny* aContext)
{
    LOGINFO((0, "PVMFLoopbackNode:QueryUUID"));
    OSCL_UNUSED_ARG(aMimeType);
    OSCL_UNUSED_ARG(aUuids);
    OSCL_UNUSED_ARG(aExactUuidsOnly);
    OSCL_UNUSED_ARG(aContext);
    OSCL_UNUSED_ARG(s);
    OSCL_LEAVE(PVMFErrNotSupported);
    return -1;
}

/**
//Queue an asynchronous node command
*/
OSCL_EXPORT_REF PVMFCommandId PVMFLoopbackNode::QueryInterface(PVMFSessionId s, const PVUuid& aUuid,
        PVInterface*& aInterfacePtr,
        const OsclAny* aContext)
{
    LOGINFO((0, "PVMFLoopbackNode:QueryInterface"));
    OSCL_UNUSED_ARG(aUuid);
    OSCL_UNUSED_ARG(aInterfacePtr);
    OSCL_UNUSED_ARG(aContext);
    OSCL_UNUSED_ARG(s);
    OSCL_LEAVE(PVMFErrNotSupported);
    return -1;
}

////////////////////////////////////////////////////////////////////////////

OSCL_EXPORT_REF PVMFCommandId PVMFLoopbackNode::RequestPort(PVMFSessionId s, int32 aPortTag, const PvmfMimeString* aPortConfig, const OsclAny* aContext)
{
    LOGINFO((0, "PVMFLoopbackNode:RequestPort"));
    PVMFLoopbackNodeCmd cmd;
    cmd.PVMFLoopbackNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_REQUESTPORT, aPortTag, aPortConfig, aContext);

    return QueueCommandL(cmd);
}

/**
//Queue an asynchronous node command
*/
OSCL_EXPORT_REF PVMFCommandId PVMFLoopbackNode::ReleasePort(PVMFSessionId s, PVMFPortInterface& aPort, const OsclAny* aContext)
{
    LOGINFO((0, "PVMFLoopbackNode:ReleasePort"));
    PVMFLoopbackNodeCmd cmd;
    cmd.PVMFLoopbackNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_RELEASEPORT, aPort, aContext);
    return QueueCommandL(cmd);
}

/**
//Queue an asynchronous node command
*/

OSCL_EXPORT_REF PVMFCommandId PVMFLoopbackNode::Prepare(PVMFSessionId s, const OsclAny* aContext)
{
    LOGINFO((0, "SampleNode:Prepare"));
    PVMFLoopbackNodeCmd cmd;
    cmd.PVMFLoopbackNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_PREPARE, aContext);
    return QueueCommandL(cmd);
}
/**
//Queue an asynchronous node command
*/
OSCL_EXPORT_REF PVMFCommandId PVMFLoopbackNode::Init(PVMFSessionId s, const OsclAny* aContext)
{
    LOGINFO((0, "PVMFLoopbackNode:Init"));
    PVMFLoopbackNodeCmd cmd;
    cmd.PVMFLoopbackNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_INIT, aContext);
    return QueueCommandL(cmd);
}


/**
//Queue an asynchronous node command
*/
OSCL_EXPORT_REF PVMFCommandId PVMFLoopbackNode::Start(PVMFSessionId s, const OsclAny* aContext)
{
    LOGINFO((0, "PVMFLoopbackNode:Start"));
    PVMFLoopbackNodeCmd cmd;
    cmd.PVMFLoopbackNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_START, aContext);
    return QueueCommandL(cmd);
}

/**
//Queue an asynchronous node command
*/
OSCL_EXPORT_REF PVMFCommandId PVMFLoopbackNode::Stop(PVMFSessionId s, const OsclAny* aContext)
{
    LOGINFO((0, "PVMFLoopbackNode:Stop"));
    PVMFLoopbackNodeCmd cmd;
    cmd.PVMFLoopbackNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_STOP, aContext);
    return QueueCommandL(cmd);
}

/**
//Queue an asynchronous node command
*/
OSCL_EXPORT_REF PVMFCommandId PVMFLoopbackNode::Flush(PVMFSessionId s, const OsclAny* aContext)
{
    LOGINFO((0, "PVMFLoopbackNode:Flush"));
    PVMFLoopbackNodeCmd cmd;
    cmd.PVMFLoopbackNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_FLUSH, aContext);
    return QueueCommandL(cmd);
}

/**
//Queue an asynchronous node command
*/
OSCL_EXPORT_REF PVMFCommandId PVMFLoopbackNode::Pause(PVMFSessionId s, const OsclAny* aContext)
{
    LOGINFO((0, "PVMFLoopbackNode:Pause"));
    PVMFLoopbackNodeCmd cmd;
    cmd.PVMFLoopbackNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_PAUSE, aContext);
    return QueueCommandL(cmd);
}

/**
//Queue an asynchronous node command
*/
OSCL_EXPORT_REF PVMFCommandId PVMFLoopbackNode::Reset(PVMFSessionId s, const OsclAny* aContext)
{
    LOGINFO((0, "PVMFLoopbackNode:Reset"));
    PVMFLoopbackNodeCmd cmd;
    cmd.PVMFLoopbackNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_RESET, aContext);
    return QueueCommandL(cmd);
}

/**
//Queue an asynchronous node command
*/
OSCL_EXPORT_REF PVMFCommandId PVMFLoopbackNode::CancelAllCommands(PVMFSessionId s, const OsclAny* aContext)
{
    OSCL_UNUSED_ARG(aContext);
    OSCL_UNUSED_ARG(s);
    OSCL_LEAVE(PVMFErrNotSupported);
    return -1;
}

/**
//Queue an asynchronous node command
*/
OSCL_EXPORT_REF PVMFCommandId PVMFLoopbackNode::CancelCommand(PVMFSessionId s, PVMFCommandId aCmdId, const OsclAny* aContext)
{
    OSCL_UNUSED_ARG(aCmdId);
    OSCL_UNUSED_ARG(aContext);
    OSCL_UNUSED_ARG(s);
    OSCL_LEAVE(PVMFErrNotSupported);
    return -1;
}


////////////////////////////////////////////////////////////////////////////

PVMFCommandId PVMFLoopbackNode::QueueCommandL(PVMFLoopbackNodeCmd& aCmd)
{
    PVMFCommandId id;

    id = iInputCommands.AddL(aCmd);

    //wakeup the AO
    RunIfNotReady();

    return id;
}
////////////////////////////////////////////////////////////////////////////
void PVMFLoopbackNode::Run()
{
    //Process commands.
    if (!iInputCommands.empty())
    {
        if (ProcessCommand(iInputCommands.front()))
        {
            //re-schedule if more commands to do
            //and node isn't reset.
            if (!iInputCommands.empty()
                    && iInterfaceState != EPVMFNodeCreated)
            {
                RunIfNotReady();
            }
        }
    }

    //this AO needs to monitor for node flush completion.
    if (FlushPending())
    {
        //uint32 i;
        if (iIOPort->IncomingMsgQueueSize() > 0
                || iIOPort->OutgoingMsgQueueSize() > 0)
        {
            //need to wait on completion...
            RunIfNotReady();
            return;
        }
        //Flush is complete.  Go to prepared state.
        SetState(EPVMFNodePrepared);
        //resume port input so the ports can be re-started.
        iIOPort->ResumeInput();
        CommandComplete(iCurrentCommand, iCurrentCommand.front(), PVMFSuccess);
    }
}




///////////////////////////////////////////////////////////////////////////
bool PVMFLoopbackNode::ProcessCommand(PVMFLoopbackNodeCmd& aCmd)
{
    //normally this node will not start processing one command
    //until the prior one is finished.  However, a hi priority
    //command such as Cancel must be able to interrupt a command
    //in progress.
    if (!iCurrentCommand.empty() && !aCmd.hipri())
        return false;

    switch (aCmd.iCmd)
    {


        case PVMF_GENERIC_NODE_REQUESTPORT:
            DoRequestPort(aCmd);
            break;

        case PVMF_GENERIC_NODE_RELEASEPORT:
            DoReleasePort(aCmd);
            break;

        case PVMF_GENERIC_NODE_INIT:
            DoInit(aCmd);
            break;

        case PVMF_GENERIC_NODE_PREPARE:
            DoPrepare(aCmd);
            break;

        case PVMF_GENERIC_NODE_START:
            DoStart(aCmd);
            break;

        case PVMF_GENERIC_NODE_STOP:
            DoStop(aCmd);
            break;

        case PVMF_GENERIC_NODE_FLUSH:
            DoFlush(aCmd);
            break;

        case PVMF_GENERIC_NODE_PAUSE:
            DoPause(aCmd);
            break;

        case PVMF_GENERIC_NODE_RESET:
            DoReset(aCmd);
            break;

        default://unknown command type
            CommandComplete(iInputCommands, aCmd, PVMFFailure);
            break;
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////
void PVMFLoopbackNode::CommandComplete(PVMFLoopbackNodeCmdQ& aCmdQ, PVMFLoopbackNodeCmd& aCmd, PVMFStatus aStatus, OsclAny* aEventData)
{
    LOGINFO((0, "SampleNode:CommandComplete Id %d Cmd %d Status %d Context %d Data %d"
             , aCmd.iId, aCmd.iCmd, aStatus, aCmd.iContext, aEventData));

    //create response
    PVMFCmdResp resp(aCmd.iId, aCmd.iContext, aStatus, aEventData);
    PVMFSessionId session = aCmd.iSession;

    //Erase the command from the queue.
    aCmdQ.Erase(&aCmd);

    //Report completion to the session observer.
    ReportCmdCompleteEvent(session, resp);
}



//////////////////////////////////////////////////////////////////


/**
//Called by the command handler AO to do the node Init
*/
void PVMFLoopbackNode::DoInit(PVMFLoopbackNodeCmd& aCmd)
{
    switch (iInterfaceState)
    {
        case EPVMFNodeIdle:
            //this node doesn't need to do anything
            SetState(EPVMFNodeInitialized);
            CommandComplete(iInputCommands, aCmd, PVMFSuccess);
            break;

        default:
            CommandComplete(iInputCommands, aCmd, PVMFErrInvalidState);
            break;
    }
}
////////////////////////////////////////////////
void PVMFLoopbackNode::DoStart(PVMFLoopbackNodeCmd& aCmd)
{
    PVMFStatus status = PVMFSuccess;
    switch (iInterfaceState)
    {
        case EPVMFNodePrepared:
        case EPVMFNodePaused:
            //transition to Started
            SetState(EPVMFNodeStarted);
            //wakeup the port
            iIOPort->RunIfNotReady();

            break;

        default:
            status = PVMFErrInvalidState;
            break;
    }

    CommandComplete(iInputCommands, aCmd, status);
}


/**
//Called by the command handler AO to do the node Prepare
*/
void PVMFLoopbackNode::DoPrepare(PVMFLoopbackNodeCmd& aCmd)
{
    switch (iInterfaceState)
    {
        case EPVMFNodeInitialized:
            //this node doesn't need to do anything to get ready
            //to start.
            SetState(EPVMFNodePrepared);
            CommandComplete(iInputCommands, aCmd, PVMFSuccess);
            break;

        default:
            CommandComplete(iInputCommands, aCmd, PVMFErrInvalidState);
            break;
    }
}

/**
//Called by the command handler AO to do the node Stop
*/
void PVMFLoopbackNode::DoStop(PVMFLoopbackNodeCmd& aCmd)
{
    switch (iInterfaceState)
    {
        case EPVMFNodeStarted:
        case EPVMFNodePaused:

            //Resetting the IO Port
            if (iIOPort)
            {
                iIOPort->Reset();
            }
            //transition to Prepared state
            SetState(EPVMFNodePrepared);

            CommandComplete(iInputCommands, aCmd, PVMFSuccess);
            break;

        default:
            CommandComplete(iInputCommands, aCmd, PVMFErrInvalidState);
            break;
    }
}

/**
//Called by the command handler AO to do the node Pause
*/
void PVMFLoopbackNode::DoPause(PVMFLoopbackNodeCmd& aCmd)
{
    switch (iInterfaceState)
    {
        case EPVMFNodeStarted:
            // Pause data source
            SetState(EPVMFNodePaused);
            CommandComplete(iInputCommands, aCmd, PVMFSuccess);
            break;
        default:
            CommandComplete(iInputCommands, aCmd, PVMFErrInvalidState);
            break;
    }
}

/**
//Called by the command handler AO to do the node Flush
*/
void PVMFLoopbackNode::DoFlush(PVMFLoopbackNodeCmd& aCmd)
{
    switch (iInterfaceState)
    {
        case EPVMFNodeStarted:
        case EPVMFNodePaused:
            //the flush is asynchronous.  move the command from
            //the input command queue to the current command, where
            //it will remain until the flush completes.
            int32 err;
            OSCL_TRY(err, iCurrentCommand.StoreL(aCmd););
            if (err != OsclErrNone)
            {
                CommandComplete(iInputCommands, aCmd, PVMFErrNoMemory);
                return;
            }
            iInputCommands.Erase(&aCmd);

            iIOPort->SuspendInput();

            //if node isn't active wakeup the ports
            //so they'll flush.
            if (iInterfaceState != EPVMFNodeStarted)
            {
                iIOPort->RunIfNotReady();
            }

            break;

        default:
            CommandComplete(iInputCommands, aCmd, PVMFErrInvalidState);
            break;
    }
}

/**
//A routine to tell if a flush operation is in progress.
*/
bool PVMFLoopbackNode::FlushPending()
{
    return (iCurrentCommand.size() > 0
            && iCurrentCommand.front().iCmd == PVMFLOOPBACK_NODE_CMD_FLUSH);
}


void PVMFLoopbackNode::SetState(TPVMFNodeInterfaceState s)
{
    LOGINFO((0, "LoopbackNode:SetState %d", s));
    PVMFNodeInterface::SetState(s);
}
/**
//Called by the command handler AO to do the node Reset.
*/
void PVMFLoopbackNode::DoReset(PVMFLoopbackNodeCmd& aCmd)
{
    //This example node allows a reset from any idle state.
    switch (iInterfaceState)
    {
        case EPVMFNodeCreated:
        case EPVMFNodeIdle:
        case EPVMFNodeInitialized:
        case EPVMFNodePrepared:
        {
            if (iIOPort)
            {
                //delete ports and notify observer.
                OSCL_DELETE(iIOPort);
                iIOPort = NULL;
            }
            //logoff & go back to Created state.
            SetState(EPVMFNodeIdle);
            CommandComplete(iInputCommands, aCmd, PVMFSuccess);
        }
        break;

        default:
            CommandComplete(iInputCommands, aCmd, PVMFErrInvalidState);
            break;
    }


}


/**
// called by the command handler AO to do Request Port
*/
void PVMFLoopbackNode::DoRequestPort(PVMFLoopbackNodeCmd& aCmd)
{
    int32 tag;
    OSCL_String* mimetype;
    aCmd.PVMFLoopbackNodeCommandBase::Parse(tag, mimetype);

    switch (tag)
    {
        case PVMF_LOOPBACKNODE_PORT_TYPE_LOOPBACK:
            break;

        default:
        {
            //bad port tag
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFLoopbackNode::DoRequestPort: Error - Invalid port tag"));
            CommandComplete(iInputCommands, aCmd, PVMFFailure);
            return;
        }
        break;
    }

    iIOPort = OSCL_NEW(PVMFLoopbackIOPort, (tag, this));

    if (!iIOPort)
    {
        CommandComplete(iInputCommands, aCmd, PVMFErrNoMemory);
        return;
    }

    if (mimetype)
    {
        PVMFFormatType fmt = mimetype->get_str();
        if (fmt != PVMF_MIME_FORMAT_UNKNOWN && iIOPort->IsFormatSupported(fmt))
        {
            iIOPort->iFormat = fmt;
            iIOPort->FormatUpdated();
        }
    }

    CommandComplete(iInputCommands, aCmd, PVMFSuccess, (OsclAny*)iIOPort);
}


/**
// called by the command handler AO to do Request Port
*/
void PVMFLoopbackNode::DoReleasePort(PVMFLoopbackNodeCmd& aCmd)

{
    if (iIOPort)
    {
        OSCL_DELETE(iIOPort);
        iIOPort = NULL;
        CommandComplete(iInputCommands, aCmd, PVMFSuccess);
    }
    else
        CommandComplete(iInputCommands, aCmd, PVMFFailure);
}


void PVMFLoopbackNode::ReportInfoEvent(PVMFEventType aEventType, OsclAny* aEventData)
{
    LOGINFO((0, "LoopbackNode:NodeInfoEvent Type %d Data %d"
             , aEventType, aEventData));

    PVMFNodeInterface::ReportInfoEvent(aEventType, aEventData);
}

void PVMFLoopbackNode::ReportErrorEvent(PVMFEventType aEventType, OsclAny* aEventData)
{
    LOGINFO((0, "LoopbackNode:NodeErrorEvent Type %d Data %d"
             , aEventType, aEventData));

    PVMFNodeInterface::ReportErrorEvent(aEventType, aEventData);
}

