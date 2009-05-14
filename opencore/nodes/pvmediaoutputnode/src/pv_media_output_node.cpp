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
 * @file pvmi_io_interface_node.cpp
 * @brief
 */

#include "oscl_base.h"
#include "pv_media_output_node_factory.h"
#include "pv_media_output_node.h"
#include "pv_media_output_node_inport.h"
#include "oscl_dll.h"
#include "pvmf_basic_errorinfomessage.h"
#include "pv_media_output_node_events.h"

// Define entry point for this DLL
OSCL_DLL_ENTRY_POINT_DEFAULT()

/**
//Macros for calling PVLogger
*/
#define LOGREPOS(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG,iReposLogger,PVLOGMSG_INFO,m);
#define LOGERROR(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iLogger,PVLOGMSG_ERR,m);
#define LOGINFOHI(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG,iLogger,PVLOGMSG_INFO,m);
#define LOGINFOMED(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG,iLogger,PVLOGMSG_INFO,m);
#define LOGINFOLOW(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG,iLogger,PVLOGMSG_INFO,m);
#define LOGINFO(m) LOGINFOMED(m)
#define LOGDIAGNOSTICS(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF,iDiagnosticsLogger,PVLOGMSG_INFO,m);

//this should always be 1. set this to zero if
//you want to bypass avsync (typically used in
//case one wants to decode and render ASAP)
#define PVMF_MEDIA_OUTPUT_NODE_ENABLE_AV_SYNC 1

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFNodeInterface* PVMediaOutputNodeFactory::CreateMediaOutputNode(
    PvmiMIOControl* aMIOControl)
{
    return PVMediaOutputNode::Create(aMIOControl);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PVMediaOutputNodeFactory::DeleteMediaOutputNode(PVMFNodeInterface* aNode)
{
    PVMediaOutputNode::Release(aNode);
}

////////////////////////////////////////////////////////////////////////////
PVMFNodeInterface* PVMediaOutputNode::Create(PvmiMIOControl* aIOInterfacePtr)
{
    PVMediaOutputNode* node = OSCL_NEW(PVMediaOutputNode, ());
    if (node)
    {
        OSCL_TRAPSTACK_PUSH(node);
        node->ConstructL(aIOInterfacePtr);
        OSCL_TRAPSTACK_POP();
    }
    return (PVMFNodeInterface*)node;
}

////////////////////////////////////////////////////////////////////////////
void PVMediaOutputNode::Release(PVMFNodeInterface* aNode)
{
    OSCL_DELETE(((PVMediaOutputNode*)aNode));
}

////////////////////////////////////////////////////////////////////////////
PVMediaOutputNode::~PVMediaOutputNode()
{
    LogDiagnostics();

    Cancel();
    if (IsAdded())
        RemoveFromScheduler();

    iLogger = NULL;

    if (iMIOControl)
    {
        //call disconnect to make sure that there are no
        //callback once the object has been destroyed
        iMIOControl->disconnect(iMIOSession);
        //ignore any returned errors.
        iMIOControl->ThreadLogoff();
    }

    //if any MIO commands are outstanding, there will be
    //a crash when they callback-- so panic here instead.
    if (!iCancelCommand.empty()
            || iMediaIORequest != ENone)
        OSCL_ASSERT(0);//OsclError::Panic("PVMOUT",PVMoutPanic_OutstandingMIO_Command);

    //Cleanup allocated ports
    while (!iInPortVector.empty())
        iInPortVector.Erase(&iInPortVector.front());

    //Cleanup commands
    //The command queues are self-deleting, but we want to
    //notify the observer of unprocessed commands.
    while (!iCurrentCommand.empty())
    {
        CommandComplete(iCurrentCommand, iCurrentCommand.front(), PVMFFailure);
    }
    while (!iInputCommands.empty())
    {
        CommandComplete(iInputCommands, iInputCommands.front(), PVMFFailure);
    }

}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMediaOutputNode::ThreadLogon()
{
    if (iInterfaceState != EPVMFNodeCreated)
        return PVMFErrInvalidState;

    iLogger = PVLogger::GetLoggerObject("PVMediaOutputNode");
    iReposLogger = PVLogger::GetLoggerObject("pvplayerrepos.mionode");
    iDiagnosticsLogger = PVLogger::GetLoggerObject("pvplayerdiagnostics.mionode");

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMediaOutputNode::ThreadLogon"));

    if (!IsAdded())
        AddToScheduler();

    if (iMIOControl)
    {
        iMIOControl->ThreadLogon();
        iMediaIOState = STATE_LOGGED_ON;
    }

    if (iMIOControl->connect(iMIOSession, this) != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVMediaOutputNode::ThreadLogon: Error - iMIOControl->connect failed"));
        return PVMFFailure;
    }

    SetState(EPVMFNodeIdle);
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMediaOutputNode::ThreadLogoff()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMediaOutputNode::ThreadLogoff"));

    if (iInterfaceState != EPVMFNodeIdle)
        return PVMFErrInvalidState;

    if (IsAdded())
        RemoveFromScheduler();

    iLogger = NULL;

    if (iMIOControl)
    {
        // Currently we do not distinguish between these states
        // in how we drive the MIO. In the future, we will be
        // able to independently reset/disconnect MIOs.
        //
        // The MIO reset is called here instead of the internal
        // reset because there is currently no processing there.
        // This is to reduce risk to existing MIOs.
        //
        // It can be moved to the internal node reset in the future.
        PVMFStatus status = PVMFFailure;
        status = iMIOControl->disconnect(iMIOSession);
        //ignore any returned errors.
        iMIOControl->ThreadLogoff();
        iMediaIOState = STATE_IDLE;
    }

    SetState(EPVMFNodeCreated);
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMediaOutputNode::GetCapability(PVMFNodeCapability& aNodeCapability)
{
    if (!iMIOConfig)
        return PVMFFailure;

    aNodeCapability.iCanSupportMultipleInputPorts = false;
    aNodeCapability.iCanSupportMultipleOutputPorts = false;
    aNodeCapability.iHasMaxNumberOfPorts = true;
    aNodeCapability.iMaxNumberOfPorts = 1;

    PvmiKvp* kvp ;
    int numParams ;
    int32 err ;
    int32 i ;
    PVMFStatus status;

    // Get input formats capability from media IO
    kvp = NULL;
    numParams = 0;
    status = iMIOConfig->getParametersSync(NULL, (char*)INPUT_FORMATS_CAP_QUERY, kvp, numParams, NULL);
    if (status == PVMFSuccess)
    {
        OSCL_TRY(err,
                 for (i = 0; i < numParams; i++)
                 aNodeCapability.iInputFormatCapability.push_back(PVMFFormatType(kvp[i].value.pChar_value));
                );
        if (kvp)
            iMIOConfig->releaseParameters(0, kvp, numParams);
    }
    //else ignore errors.

    // Get output formats capability from media IO
    kvp = NULL;
    numParams = 0;
    status = iMIOConfig->getParametersSync(NULL, (char*)OUTPUT_FORMATS_CAP_QUERY, kvp, numParams, NULL);
    if (status == PVMFSuccess)
    {
        OSCL_TRY(err,
                 for (i = 0; i < numParams; i++)
                 aNodeCapability.iOutputFormatCapability.push_back(PVMFFormatType(kvp[i].value.pChar_value));
                );
        if (kvp)
            iMIOConfig->releaseParameters(0, kvp, numParams);
    }
    //else ignore errors.

    if (aNodeCapability.iInputFormatCapability.empty() && aNodeCapability.iOutputFormatCapability.empty())
        return PVMFFailure;

    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFPortIter* PVMediaOutputNode::GetPorts(const PVMFPortFilter* aFilter)
{
    OSCL_UNUSED_ARG(aFilter);//port filter is not implemented.
    iInPortVector.Reset();
    return &iInPortVector;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMediaOutputNode::QueryUUID(PVMFSessionId s, const PvmfMimeString& aMimeType,
        Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids,
        bool aExactUuidsOnly,
        const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMediaOutputNode::QueryUUID() called"));
    PVMediaOutputNodeCmd cmd;
    cmd.PVMediaOutputNodeCmdBase::Construct(s, PVMF_GENERIC_NODE_QUERYUUID, aMimeType, aUuids, aExactUuidsOnly, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMediaOutputNode::QueryInterface(PVMFSessionId s, const PVUuid& aUuid,
        PVInterface*& aInterfacePtr,
        const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMediaOutputNode::QueryInterface() called"));
    PVMediaOutputNodeCmd cmd;
    cmd.PVMediaOutputNodeCmdBase::Construct(s, PVMF_GENERIC_NODE_QUERYINTERFACE, aUuid, aInterfacePtr, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMediaOutputNode::RequestPort(PVMFSessionId s, int32 aPortTag, const PvmfMimeString* aPortConfig, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMediaOutputNode::RequestPort() called"));
    PVMediaOutputNodeCmd cmd;
    cmd.PVMediaOutputNodeCmdBase::Construct(s, PVMF_GENERIC_NODE_REQUESTPORT, aPortTag, aPortConfig, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMediaOutputNode::ReleasePort(PVMFSessionId s, PVMFPortInterface& aPort, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMediaOutputNode::ReleasePort() called"));
    PVMediaOutputNodeCmd cmd;
    cmd.PVMediaOutputNodeCmdBase::Construct(s, PVMF_GENERIC_NODE_RELEASEPORT, aPort, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMediaOutputNode::Init(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMediaOutputNode::Init() called"));
    PVMediaOutputNodeCmd cmd;
    cmd.PVMediaOutputNodeCmdBase::Construct(s, PVMF_GENERIC_NODE_INIT, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMediaOutputNode::Prepare(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMediaOutputNode::Prepare() called"));
    PVMediaOutputNodeCmd cmd;
    cmd.PVMediaOutputNodeCmdBase::Construct(s, PVMF_GENERIC_NODE_PREPARE, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMediaOutputNode::Start(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMediaOutputNode::Start() called"));
    PVMediaOutputNodeCmd cmd;
    cmd.PVMediaOutputNodeCmdBase::Construct(s, PVMF_GENERIC_NODE_START, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMediaOutputNode::Stop(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMediaOutputNode::Stop() called"));
    PVMediaOutputNodeCmd cmd;
    cmd.PVMediaOutputNodeCmdBase::Construct(s, PVMF_GENERIC_NODE_STOP, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMediaOutputNode::Flush(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMediaOutputNode::Flush() called"));
    PVMediaOutputNodeCmd cmd;
    cmd.PVMediaOutputNodeCmdBase::Construct(s, PVMF_GENERIC_NODE_FLUSH, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMediaOutputNode::Pause(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMediaOutputNode::Pause() called"));
    PVMediaOutputNodeCmd cmd;
    cmd.PVMediaOutputNodeCmdBase::Construct(s, PVMF_GENERIC_NODE_PAUSE, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMediaOutputNode::Reset(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMediaOutputNode::Reset() called"));
    PVMediaOutputNodeCmd cmd;
    cmd.PVMediaOutputNodeCmdBase::Construct(s, PVMF_GENERIC_NODE_RESET, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMediaOutputNode::CancelAllCommands(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMediaOutputNode::CancelAllCommands() called"));
    PVMediaOutputNodeCmd cmd;
    cmd.PVMediaOutputNodeCmdBase::Construct(s, PVMF_GENERIC_NODE_CANCELALLCOMMANDS, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMediaOutputNode::CancelCommand(PVMFSessionId s, PVMFCommandId aCmdId, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMediaOutputNode::CancelCommands() called cmdId=%d", aCmdId));
    PVMediaOutputNodeCmd cmd;
    cmd.PVMediaOutputNodeCmdBase::Construct(s, PVMF_GENERIC_NODE_CANCELCOMMAND, aCmdId, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PVMediaOutputNode::addRef()
{
    ++iExtensionRefCount;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PVMediaOutputNode::removeRef()
{
    if (iExtensionRefCount > 0)
        --iExtensionRefCount;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF bool PVMediaOutputNode::queryInterface(const PVUuid& uuid, PVInterface*& iface)
{
    if (uuid == PvmfNodesSyncControlUuid)
    {
        PvmfNodesSyncControlInterface* myInterface = OSCL_STATIC_CAST(PvmfNodesSyncControlInterface*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
        ++iExtensionRefCount;
    }
    else if (uuid == PVMI_CAPABILITY_AND_CONFIG_PVUUID)
    {
        PvmiCapabilityAndConfig* myInterface = OSCL_STATIC_CAST(PvmiCapabilityAndConfig*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
        ++iExtensionRefCount;
    }
    else
    {
        iface = NULL;
        return false;
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMediaOutputNode::SetClock(PVMFMediaClock* aClock)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMediaOutputNode::SetClock: aClock=0x%x", aClock));

    //remove any old clock
    if (iClock)
    {
        if (iMIOClockExtension)
            iMIOClockExtension->SetClock(NULL);
        for (uint32 i = 0; i < iInPortVector.size(); i++)
            iInPortVector[i]->SetClock(NULL);
    }

    iClock = aClock;
    for (uint32 i = 0; i < iInPortVector.size(); i++)
    {
        iInPortVector[i]->SetClock(aClock);
        iInPortVector[i]->ChangeClockRate(iClockRate);
    }

    //pass the clock to the optional MIO clock interface
    if (iMIOClockExtension)
    {
        iMIOClockExtension->SetClock(aClock);
    }
    else
    {
#if (PVMF_MEDIA_OUTPUT_NODE_ENABLE_AV_SYNC)
        //turn on sync params
        for (uint32 i = 0; i < iInPortVector.size(); i++)
        {
            iInPortVector[i]->EnableMediaSync();
            iInPortVector[i]->SetMargins(iEarlyMargin, iLateMargin);
        }
#endif
    }
    return PVMFSuccess;
}

///////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMediaOutputNode::ChangeClockRate(int32 aRate)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMediaOutputNode::ChangeClockRate: aRate=%d", aRate));

    iClockRate = aRate;
    for (uint32 i = 0; i < iInPortVector.size() ; i++)
        iInPortVector[i]->ChangeClockRate(aRate);

    // For now support all rates.
    // In future, need to check with underlying media IO component

    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMediaOutputNode::SetMargins(int32 aEarlyMargin, int32 aLateMargin)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMediaOutputNode::SetMargins: aEarlyMargin=%d, aLateMargin=%d", aEarlyMargin, aLateMargin));

    //save the margins
    iEarlyMargin = aEarlyMargin;
    iLateMargin = aLateMargin;

    //pass the margins to the ports
    for (uint32 i = 0; i < iInPortVector.size() ; i++)
        iInPortVector[i]->SetMargins(aEarlyMargin, aLateMargin);

    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PVMediaOutputNode::ClockStarted()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMediaOutputNode::ClockStarted"));

    //notify the ports
    for (uint32 i = 0; i < iInPortVector.size() ; i++)
        iInPortVector[i]->ClockStarted();

}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PVMediaOutputNode::ClockStopped()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMediaOutputNode::ClockStopped"));

    //notify the ports
    for (uint32 i = 0; i < iInPortVector.size() ; i++)
        iInPortVector[i]->ClockStopped();
}


////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMediaOutputNode::SkipMediaData(PVMFSessionId s,
        PVMFTimestamp aResumeTimestamp,
        uint32 aStreamID,
        bool aPlayBackPositionContinuous,
        OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMediaOutputNode::SkipMediaData() called "));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iReposLogger, PVLOGMSG_INFO,
                    (0, "PVMediaOutputNode::SkipMediaData() called - Mime=%s", iSinkFormatString.get_str()));
    PVMediaOutputNodeCmd cmd;
    cmd.Construct(s,
                  PVMF_MEDIAOUTPUTNODE_SKIPMEDIADATA,
                  aResumeTimestamp,
                  aStreamID,
                  aPlayBackPositionContinuous,
                  aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PVMediaOutputNode::RequestCompleted(const PVMFCmdResp& aResponse)
//callback from the MIO module.
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMediaOutputNode::RequestCompleted: Cmd ID=%d", aResponse.GetCmdId()));

    //look for cancel completion.
    if (iMediaIOCancelPending
            && aResponse.GetCmdId() == iMediaIOCancelCmdId)
    {
        iMediaIOCancelPending = false;

        OSCL_ASSERT(!iCancelCommand.empty());

        //Current cancel command is now complete.
        CommandComplete(iCancelCommand, iCancelCommand.front(), PVMFSuccess);
    }
    //look for non-cancel completion
    else if (iMediaIORequest != ENone
             && aResponse.GetCmdId() == iMediaIOCmdId)
    {
        OSCL_ASSERT(!iCurrentCommand.empty());
        PVMediaOutputNodeCmd& cmd = iCurrentCommand.front();

        switch (iMediaIORequest)
        {
            case EQueryCapability:
                iMIOConfig = OSCL_STATIC_CAST(PvmiCapabilityAndConfig*, iMIOConfigPVI);
                iMIOConfigPVI = NULL;
                if (aResponse.GetCmdStatus() != PVMFSuccess)
                    cmd.iEventCode = PVMFMoutNodeErr_MediaIOQueryCapConfigInterface;
                CommandComplete(iCurrentCommand, iCurrentCommand.front(), aResponse.GetCmdStatus());
                break;

            case EQueryClockExtension:
                //ignore any error from this query since the interface is optional.
                iMediaIORequest = ENone;
                iMIOClockExtension = OSCL_STATIC_CAST(PvmiClockExtensionInterface*, iMIOClockExtensionPVI);
                iMIOClockExtensionPVI = NULL;
                //re-do the clock setting call since iMIOClockExtension may have changed.
                if (aResponse.GetCmdStatus() == PVMFSuccess
                        && iMIOClockExtension)
                {
                    SetClock(iClock);
                }
                //To continue the Node Init, query for the
                //capability & config interface
                {
                    PVMFStatus status = SendMioRequest(iCurrentCommand[0], EQueryCapability);
                    if (status == PVMFPending)
                        return;//wait on response
                    else
                        CommandComplete(iCurrentCommand, iCurrentCommand.front(), status);
                }
                break;

            case EInit:
                if (aResponse.GetCmdStatus() != PVMFSuccess)
                    cmd.iEventCode = PVMFMoutNodeErr_MediaIOInit;
                else
                    iMediaIOState = STATE_INITIALIZED;
                CommandComplete(iCurrentCommand, iCurrentCommand.front(), aResponse.GetCmdStatus());
                break;

            case EStart:
                if (aResponse.GetCmdStatus() != PVMFSuccess)
                    cmd.iEventCode = PVMFMoutNodeErr_MediaIOStart;
                else
                    iMediaIOState = STATE_STARTED;
                CommandComplete(iCurrentCommand, iCurrentCommand.front(), aResponse.GetCmdStatus());
                break;

            case EPause:
                if (aResponse.GetCmdStatus() != PVMFSuccess)
                    cmd.iEventCode = PVMFMoutNodeErr_MediaIOPause;
                else
                    iMediaIOState = STATE_PAUSED;
                CommandComplete(iCurrentCommand, iCurrentCommand.front(), aResponse.GetCmdStatus());
                break;

            case EStop:
                if (aResponse.GetCmdStatus() != PVMFSuccess)
                    cmd.iEventCode = PVMFMoutNodeErr_MediaIOStop;
                else
                    iMediaIOState = STATE_INITIALIZED;
                CommandComplete(iCurrentCommand, iCurrentCommand.front(), aResponse.GetCmdStatus());
                break;

            case EDiscard:
            {
                if (aResponse.GetCmdStatus() != PVMFSuccess)
                {
                    cmd.iEventCode = PVMFMoutNodeErr_MediaIODiscardData;
                    CommandComplete(iCurrentCommand, iCurrentCommand.front(), aResponse.GetCmdStatus());
                }
                else
                {
                    iMediaIORequest = ENone;
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iReposLogger, PVLOGMSG_INFO,
                                    (0, "PVMediaOutputNode::ResuestCompleted - EDiscard success - Mime=%s",
                                     iSinkFormatString.get_str()));
                    //attempt to complete skip media data
                    CompleteSkipMediaData();
                }
            }
            break;

            case EReset:
            {
                if (aResponse.GetCmdStatus() != PVMFSuccess)
                {
                    cmd.iEventCode = PVMFMoutNodeErr_MediaIOReset;
                }
                else
                {
                    iMediaIOState = STATE_LOGGED_ON;
                }
                CommandComplete(iCurrentCommand, iCurrentCommand.front(), aResponse.GetCmdStatus());
            }
            break;

            default:
                OSCL_ASSERT(false);
                CommandComplete(iCurrentCommand, iCurrentCommand.front(), PVMFFailure);
                break;
        }

    }
    else
    {
        //unexpected response.
        LOGERROR((0, "PVMediaOutputNode:RequestComplete Warning! Unexpected command ID %d"
                  , aResponse.GetCmdId()));
    }
}

OSCL_EXPORT_REF void PVMediaOutputNode::ReportErrorEvent(PVMFEventType aEventType, PVInterface* aExtMsg)
{
    OSCL_UNUSED_ARG(aEventType);
    OSCL_UNUSED_ARG(aExtMsg);
}
OSCL_EXPORT_REF void PVMediaOutputNode::ReportInfoEvent(PVMFEventType aEventType, PVInterface* aExtMsg)
{
    OSCL_UNUSED_ARG(aExtMsg);
    if (PVMFMIOConfigurationComplete == aEventType)
    {
        LOGINFO((0, "PVMediaOutputNode::ReportInfoEvent PVMFMIOConfigurationComplete received"));
        for (uint32 i = 0; i < iInPortVector.size(); i++)
        {
            iInPortVector[i]->SetMIOComponentConfigStatus(true);
        }
    }
}


////////////////////////////////////////////////////////////////////////////
PVMediaOutputNode::PVMediaOutputNode()
        : OsclActiveObject(OsclActiveObject::EPriorityNominal, "PVMediaOutputNode")
        , iEventUuid(PVMFMediaOutputNodeEventTypesUUID)
        , iMIOControl(NULL)
        , iMIOSession(NULL)
        , iMIOConfig(NULL)
        , iMIOConfigPVI(NULL)
        , iMediaIOState(STATE_IDLE)
        , iClock(NULL)
        , iEarlyMargin(DEFAULT_EARLY_MARGIN)
        , iLateMargin(DEFAULT_LATE_MARGIN)
        , iDiagnosticsLogger(NULL)
        , iDiagnosticsLogged(false)
        , iExtensionRefCount(0)
        , iLogger(NULL)
        , iReposLogger(NULL)
        , iRecentBOSStreamID(0)
{
}

////////////////////////////////////////////////////////////////////////////
void PVMediaOutputNode::ConstructL(PvmiMIOControl* aIOInterfacePtr)
{
    iLogger = NULL;
    iMIOControl = aIOInterfacePtr;
    iInputCommands.Construct(1, 10);//reserve 10
    iCurrentCommand.Construct(1, 1);//reserve 1.
    iCancelCommand.Construct(1, 1);//reserve 1.
    iInPortVector.Construct(0);//reserve zero
    iMediaIORequest = ENone;
    iMediaIOCancelPending = false;
    iMIOClockExtension = NULL;
    iMIOClockExtensionPVI = NULL;
    iClockRate = 100000;
}

////////////////////////////////////////////////////////////////////////////
/**
//This routine is called by various command APIs to queue an
//asynchronous command for processing by the command handler AO.
//This function may leave if the command can't be queued due to
//memory allocation failure.
*/
PVMFCommandId PVMediaOutputNode::QueueCommandL(PVMediaOutputNodeCmd& aCmd)
{
    PVMFCommandId id;

    id = iInputCommands.AddL(aCmd);

    //wakeup the AO
    RunIfNotReady();

    return id;
}

/**
//The various command handlers call this when a command is complete.
*/
void PVMediaOutputNode::CommandComplete(PVMediaOutputNodeCmdQ& aCmdQ, PVMediaOutputNodeCmd& aCmd, PVMFStatus aStatus, OsclAny*aEventData)
{
    if (aStatus == PVMFSuccess || aCmd.iCmd == PVMF_GENERIC_NODE_QUERYINTERFACE) //(mg) treat QueryIF failures as info, not errors
    {
        LOGINFO((0, "PVMediaOutputNode:CommandComplete Id %d Cmd %d Status %d Context %d EVData %d EVCode %d"
                 , aCmd.iId, aCmd.iCmd, aStatus, aCmd.iContext, aEventData, aCmd.iEventCode));
    }
    else
    {
        LOGERROR((0, "PVMediaOutputNode:CommandComplete Error! Id %d Cmd %d Status %d Context %d EVData %d EVCode %d"
                  , aCmd.iId, aCmd.iCmd, aStatus, aCmd.iContext, aEventData, aCmd.iEventCode));
    }

    //do state transitions and any final command completion.
    if (aStatus == PVMFSuccess)
    {
        switch (aCmd.iCmd)
        {
            case PVMF_GENERIC_NODE_INIT:
                SetState(EPVMFNodeInitialized);
                break;
            case PVMF_GENERIC_NODE_PREPARE:
                SetState(EPVMFNodePrepared);
                break;
            case PVMF_GENERIC_NODE_START:
                SetState(EPVMFNodeStarted);
                {
                    for (uint32 i = 0; i < iInPortVector.size(); i++)
                        iInPortVector[i]->NodeStarted();
                }
                break;
            case PVMF_GENERIC_NODE_PAUSE:
                SetState(EPVMFNodePaused);
                break;
            case PVMF_GENERIC_NODE_STOP:
                SetState(EPVMFNodePrepared);
                break;
            case PVMF_GENERIC_NODE_FLUSH:
                SetState(EPVMFNodePrepared);
                //resume port input so the ports can be re-started.
                {
                    for (uint32 i = 0;i < iInPortVector.size();i++)
                        iInPortVector[i]->ResumeInput();
                }
                break;
            case PVMF_GENERIC_NODE_RESET:
                SetState(EPVMFNodeIdle);
                break;
            default:
                break;
        }
    }

    //Reset the media I/O request
    iMediaIORequest = ENone;
    {
        //Extract parameters needed for command response.
        PVMFCommandId cmdId = aCmd.iId;
        const OsclAny* cmdContext = aCmd.iContext;
        PVMFSessionId cmdSess = aCmd.iSession;
        PVMFStatus eventCode = aCmd.iEventCode;

        //Erase the command from the queue.
        aCmdQ.Erase(&aCmd);

        if (eventCode != PVMFMoutNodeErr_First)
        {
            //create extended response.

            PVMFBasicErrorInfoMessage*eventmsg = OSCL_NEW(PVMFBasicErrorInfoMessage, (eventCode, iEventUuid, NULL));

            PVMFCmdResp resp(cmdId
                             , cmdContext
                             , aStatus
                             , OSCL_STATIC_CAST(PVInterface*, eventmsg)
                             , aEventData);

            //report to the session observers.
            PVMFNodeInterface::ReportCmdCompleteEvent(cmdSess, resp);

            //remove the ref to the extended response
            if (eventmsg)
                eventmsg->removeRef();
        }
        else
        {
            //create basic response
            PVMFCmdResp resp(cmdId, cmdContext, aStatus, aEventData);

            //report to the session observers.
            PVMFNodeInterface::ReportCmdCompleteEvent(cmdSess, resp);
        }
    }
    //re-schedule if there are more commands and node isn't logged off
    if (!iInputCommands.empty()
            && IsAdded())
        RunIfNotReady();
}


////////////////////////////////////////////////////////////////////////////
void PVMediaOutputNode::Run()
{
    //Process async node commands.
    if (!iInputCommands.empty())
    {
        ProcessCommand();
    }

    //Check for completion of a flush command...
    if (iCurrentCommand.size() > 0
            && iCurrentCommand.front().iCmd == PVMF_GENERIC_NODE_FLUSH
            && PortQueuesEmpty())
    {
        //Flush is complete.
        CommandComplete(iCurrentCommand, iCurrentCommand.front(), PVMFSuccess);
    }
}

bool PVMediaOutputNode::PortQueuesEmpty()
{
    uint32 i;
    for (i = 0;i < iInPortVector.size();i++)
    {
        if (iInPortVector[i]->IncomingMsgQueueSize() > 0 ||
                iInPortVector[i]->OutgoingMsgQueueSize())
        {
            return false;
        }
    }
    return true;
}

/**
//Called by the command handler AO to process a command from
//the input queue.
*/
void PVMediaOutputNode::ProcessCommand()
{
    //Can't do anything when an asynchronous cancel is in progress-- just
    //need to wait on completion.
    if (!iCancelCommand.empty())
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMediaOutputNode::ProcessCommand Cancel pending so return"));
        return ; //keep waiting.
    }

    //If a command is in progress, only certain commands can interrupt it.
    if (!iCurrentCommand.empty()
            && !iInputCommands.front().caninterrupt())
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMediaOutputNode::ProcessCommand Command pending so return"));
        return ; //keep waiting
    }

    //The newest or highest pri command is in the front of the queue.
    OSCL_ASSERT(!iInputCommands.empty());
    PVMediaOutputNodeCmd& aCmd = iInputCommands.front();

    PVMFStatus cmdstatus;
    OsclAny* aEventData = NULL;
    if (aCmd.caninterrupt())
    {
        //save input command in cancel command
        int32 err;
        OSCL_TRY(err, iCancelCommand.StoreL(aCmd););
        if (err != OsclErrNone)
        {
            cmdstatus = PVMFErrNoMemory;
        }
        else
        {
            //Process the interrupt commands.
            switch (aCmd.iCmd)
            {
                case PVMF_GENERIC_NODE_CANCELALLCOMMANDS:
                    cmdstatus = DoCancelAllCommands(aCmd);
                    break;

                case PVMF_GENERIC_NODE_CANCELCOMMAND:
                    cmdstatus = DoCancelCommand(aCmd);
                    break;

                default:
                    OSCL_ASSERT(false);
                    cmdstatus = PVMFFailure;
                    break;
            }
        }

        //erase the input command.
        if (cmdstatus != PVMFPending)
        {
            //Request already failed or completed successfully -- erase from Cancel Command.
            //Node command remains in Input Commands to be completed at end of this function
            if (iCancelCommand.size() > 0)
                iCancelCommand.Erase(&iCancelCommand.front());
        }
        else
        {
            //Cancel is still pending.
            //Node command is now stored in Cancel Command, so erase from Input Commands and wait
            iInputCommands.Erase(&aCmd);
        }
    }
    else
    {
        //save input command in current command
        int32 err;
        OSCL_TRY(err, iCurrentCommand.StoreL(aCmd););
        if (err != OsclErrNone)
        {
            cmdstatus = PVMFErrNoMemory;
        }
        else
        {
            //Process the normal pri commands.
            switch (aCmd.iCmd)
            {
                case PVMF_GENERIC_NODE_QUERYUUID:
                    cmdstatus = DoQueryUuid(aCmd);
                    break;

                case PVMF_GENERIC_NODE_QUERYINTERFACE:
                    cmdstatus = DoQueryInterface(aCmd);
                    break;

                case PVMF_GENERIC_NODE_REQUESTPORT:
                    cmdstatus = DoRequestPort(aCmd, aEventData);
                    break;

                case PVMF_GENERIC_NODE_RELEASEPORT:
                    cmdstatus = DoReleasePort(aCmd);
                    break;

                case PVMF_GENERIC_NODE_INIT:
                    cmdstatus = DoInit(aCmd);
                    break;

                case PVMF_GENERIC_NODE_PREPARE:
                    cmdstatus = DoPrepare(aCmd);
                    break;

                case PVMF_GENERIC_NODE_START:
                    cmdstatus = DoStart(aCmd);
                    break;

                case PVMF_GENERIC_NODE_STOP:
                    cmdstatus = DoStop(aCmd);
                    break;

                case PVMF_GENERIC_NODE_FLUSH:
                    cmdstatus = DoFlush(aCmd);
                    break;

                case PVMF_GENERIC_NODE_PAUSE:
                    cmdstatus = DoPause(aCmd);
                    break;

                case PVMF_GENERIC_NODE_RESET:
                    cmdstatus = DoReset(aCmd);
                    break;

                case PVMF_MEDIAOUTPUTNODE_SKIPMEDIADATA:
                    cmdstatus = DoSkipMediaData(aCmd);
                    break;

                default://unknown command type
                    OSCL_ASSERT(false);
                    cmdstatus = PVMFFailure;
                    break;
            }
        }

        //erase the input command.
        if (cmdstatus != PVMFPending)
        {
            //Request already failed-- erase from Current Command.
            //Node command remains in Input Commands.
            iCurrentCommand.Erase(&iCurrentCommand.front());
        }
        else
        {
            //Node command is now stored in Current Command, so erase from Input Commands.
            iInputCommands.Erase(&aCmd);
        }
    }

    if (cmdstatus != PVMFPending)
    {
        CommandComplete(iInputCommands, aCmd, cmdstatus, aEventData);
    }
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMediaOutputNode::DoQueryUuid(PVMediaOutputNodeCmd& aCmd)
{
    //This node supports Query UUID from any state

    OSCL_String* mimetype;
    Oscl_Vector<PVUuid, OsclMemAllocator> *uuidvec;
    bool exactmatch;
    aCmd.PVMediaOutputNodeCmdBase::Parse(mimetype, uuidvec, exactmatch);

    uuidvec->push_back(PvmfNodesSyncControlUuid);

    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMediaOutputNode::DoQueryInterface(PVMediaOutputNodeCmd& aCmd)
{
    PVUuid* uuid;
    PVInterface** ptr;
    aCmd.PVMediaOutputNodeCmdBase::Parse(uuid, ptr);
    if (uuid && ptr)
    {
        if (queryInterface(*uuid, *ptr))
            return PVMFSuccess;
    }
    return PVMFFailure;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMediaOutputNode::DoRequestPort(PVMediaOutputNodeCmd& aCmd, OsclAny*&aEventData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMediaOutputNode::DoRequestPort"));
    //This node supports port request from any state

    //retrieve port tag & mimetype
    int32 tag;
    OSCL_String* mimetype;
    aCmd.PVMediaOutputNodeCmdBase::Parse(tag, mimetype);

    switch (tag)
    {
        case PVMF_MEDIAIO_NODE_INPUT_PORT_TAG:
        {
            //Allocate a new port
            OsclAny *ptr = NULL;
            int32 err;
            OSCL_TRY(err, ptr = iInPortVector.Allocate(););
            if (err != OsclErrNone || !ptr)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMediaOutputNode::DoRequestPort: Error - iInPortVector Out of memory"));
                return PVMFErrNoMemory;
            }
            PVMediaOutputNodePort *port = OSCL_PLACEMENT_NEW(ptr, PVMediaOutputNodePort(this));

            //Add the port to the port vector.
            OSCL_TRY(err, iInPortVector.AddL(port););
            if (err != OsclErrNone)
            {
                iInPortVector.DestructAndDealloc(port);
                return PVMFErrNoMemory;
            }

            //set the format from the mimestring, if provided
            if (mimetype)
            {
                PVMFStatus status = port->Configure(*mimetype);
                if (status != PVMFSuccess)
                {
                    //bad format!
                    iInPortVector.Erase(&port);
                    iInPortVector.DestructAndDealloc(port);
                    return PVMFErrArgument;
                }
            }

            iSinkFormatString = *mimetype;

            //pass the current clock settings to the port.
            SetClock(iClock);

            //cast the port to the generic interface before returning.
            PVMFPortInterface*retval = port;
            aEventData = (OsclAny*)retval;
            return PVMFSuccess;
        }
        break;

        default:
        {
            //bad port tag
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMediaOutputNode::DoRequestPort: Error - Invalid port tag"));
            return PVMFFailure;
        }
        break;
    }
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMediaOutputNode::DoInit(PVMediaOutputNodeCmd& aCmd)
{
    OSCL_UNUSED_ARG(aCmd);

    if (iInterfaceState != EPVMFNodeIdle)
        return PVMFErrInvalidState;

    if (!iMIOControl)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVMediaOutputNode::DoInit: Error - iMIOControl is NULL"));
        aCmd.iEventCode = PVMFMoutNodeErr_MediaIONotExist;
        return PVMFFailure;
    }

    //Query for MIO interfaces.
    return SendMioRequest(aCmd, EQueryClockExtension);
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMediaOutputNode::DoPrepare(PVMediaOutputNodeCmd& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMediaOutputNode::DoPrepare"));

    if (iInterfaceState != EPVMFNodeInitialized)
        return PVMFErrInvalidState;

    return SendMioRequest(aCmd, EInit);

}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMediaOutputNode::DoStart(PVMediaOutputNodeCmd& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMediaOutputNode::DoStart"));

    if (iInterfaceState != EPVMFNodePrepared
            && iInterfaceState != EPVMFNodePaused)
        return PVMFErrInvalidState;

    iDiagnosticsLogged = false;
    iInPortVector[0]->iFramesDropped = 0;
    iInPortVector[0]->iTotalFrames = 0;

    //Start the MIO

    return SendMioRequest(aCmd, EStart);
}


////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMediaOutputNode::DoStop(PVMediaOutputNodeCmd& aCmd)
{
    //clear the message queues of any unprocessed data now.
    uint32 i;
    for (i = 0; i < iInPortVector.size(); i++)
    {
        iInPortVector[i]->ClearMsgQueues();
    }
    if (iInterfaceState != EPVMFNodeStarted
            && iInterfaceState != EPVMFNodePaused)
        return PVMFErrInvalidState;

    LogDiagnostics();

    if (iMediaIOState == STATE_STARTED || iMediaIOState == STATE_PAUSED)
    {
        //Stop the MIO component only if MIOs are in Paused or Started states.
        return SendMioRequest(aCmd, EStop);
    }
    else
    {
        // no stop needed if MIOs are not in started or paused states. return success.
        return PVMFSuccess;
    }
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMediaOutputNode::DoFlush(PVMediaOutputNodeCmd& aCmd)
{
    OSCL_UNUSED_ARG(aCmd);

    if (iInterfaceState != EPVMFNodeStarted
            && iInterfaceState != EPVMFNodePaused)
        return PVMFErrInvalidState;

    //Disable the input.
    if (iInPortVector.size() > 0)
    {
        for (uint32 i = 0;i < iInPortVector.size();i++)
            iInPortVector[i]->SuspendInput();
    }

    //wait for all data to be consumed
    return PVMFPending;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMediaOutputNode::DoPause(PVMediaOutputNodeCmd& aCmd)
{

    if (iInterfaceState != EPVMFNodeStarted)
        return PVMFErrInvalidState;

    return SendMioRequest(aCmd, EPause);
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMediaOutputNode::DoSkipMediaData(PVMediaOutputNodeCmd& aCmd)
{
    //Here is what we do during skip:
    //1) We pass the skiptimestamp to ports. This is to make sure that
    //they can start tossing out data ASAP
    //2) We send DiscardData to media output components. Failure to
    //queue discarddata on mediaoutput comps is considered fatal failure
    //and will result in skip failure on media output node
    //3) Then we wait for all discard data to complete and all ports to
    //report BOS. Ports call "ReportBOS" when they recv BOS.

    PVMFTimestamp resumeTimestamp;
    bool playbackpositioncontinuous;
    uint32 streamID;
    aCmd.Parse(resumeTimestamp,
               playbackpositioncontinuous,
               streamID);
    iRecentBOSStreamID = streamID;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iReposLogger, PVLOGMSG_INFO,
                    (0, "PVMediaOutputNode::DoSkipMediaData - Mime=%s, SkipTS=%d, StreamID=%d, SFR=%d",
                     iSinkFormatString.get_str(),
                     resumeTimestamp,
                     iRecentBOSStreamID,
                     playbackpositioncontinuous));

    //although we treat inport as a vector, we still
    //assume just one media output comp per node
    if (iInPortVector.size() > 0)
    {
        iInPortVector[0]->SetSkipTimeStamp(resumeTimestamp, iRecentBOSStreamID);
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iReposLogger, PVLOGMSG_ERR,
                        (0, "PVMediaOutputNode::DoSkipMediaData - No Input Ports - Mime=%s", iSinkFormatString.get_str()));
        return PVMFErrInvalidState;
    }

    PVMFStatus status = PVMFFailure;
    if (playbackpositioncontinuous == true)
    {
        //source node has not be involved in determining this skip
        //boundaries. in this case if an EOS has been sent to the mio
        //component, then do not call discard data
        //assume that there is only one port per mio node and one
        //mio comp per node.
        //also do NOT ever call discard data in case of compressed mio
        if (iInPortVector[0]->isUnCompressedMIO == true)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iReposLogger, PVLOGMSG_INFO,
                            (0, "PVMediaOutputNode::DoSkipMediaData - SFR - Calling DiscardData - Mime=%s", iSinkFormatString.get_str()));
            status = SendMioRequest(aCmd, EDiscard);
            if (status != PVMFPending)
            {
                iMediaIORequest = ENone;
                return status;
            }
        }
        else
        {
            // For Compressed MIO, we need not call discard data and should report Skip Media Complete
            // right away.
            return PVMFSuccess;
        }
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iReposLogger, PVLOGMSG_INFO,
                        (0, "PVMediaOutputNode::DoSkipMediaData - Calling DiscardData - Mime=%s", iSinkFormatString.get_str()));
        status = SendMioRequest(aCmd, EDiscard);
        if (status != PVMFPending)
        {
            iMediaIORequest = ENone;
            return status;
        }
    }
    //wait on discard
    return status;
}

void PVMediaOutputNode::CompleteSkipMediaData()
{
    if ((!iCurrentCommand.empty()) &&
            ((iCurrentCommand.front().iCmd == PVMF_MEDIAOUTPUTNODE_SKIPMEDIADATA)))
    {
        if (iMediaIORequest == ENone)
        {
            //implies that discard is complete
            if (CheckForBOS() == PVMFSuccess)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iReposLogger, PVLOGMSG_INFO,
                                (0, "PVMediaOutputNode::CompleteSkipMediaData - SkipComplete - Mime=%s", iSinkFormatString.get_str()));
                CommandComplete(iCurrentCommand, iCurrentCommand[0], PVMFSuccess);
                for (uint32 i = 0; i < iInPortVector.size(); i++)
                {
                    //clear out the bos stream id vec on all ports so that they is no confusion later on
                    iInPortVector[i]->ClearPreviousBOSStreamIDs(iRecentBOSStreamID);
                }
                return;
            }
        }
        //else continue waiting on the discard to complete.
    }
    else
    {
        //this means that either skipmediadata cmd is yet to be issued or processed
        //this should be a problem since ports record the BOS stream ids which would
        //be checked later
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iReposLogger, PVLOGMSG_ERR,
                        (0, "PVMediaOutputNode::CompleteSkipMediaData - Waiting On SkipCmd To Be Issued/Processed - Mime=%s", iSinkFormatString.get_str()));
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVMediaOutputNode::CompleteSkipMediaData - Waiting On SkipCmd To Be Issued/Processed - Mime=%s", iSinkFormatString.get_str()));
    }
}


////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMediaOutputNode::DoCancelAllCommands(PVMediaOutputNodeCmd& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMediaOutputNode::DoCancelAllCommands In"));

    if (!iCurrentCommand.empty())
    {
        if (iCurrentCommand.front().iCmd == PVMF_MEDIAOUTPUTNODE_SKIPMEDIADATA)
        {
            for (uint32 i = 0;i < iInPortVector.size();i++)
            {
                iInPortVector[i]->CancelSkip();
            }
        }
    }

    //First cancel any MIO commmand in progress.
    bool cancelmiopending = false;
    if (iMediaIORequest != ENone)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMediaOutputNode::DoCancelAllCommands Cancelling any pending MIO request"));
        if (CancelMioRequest(aCmd) == PVMFPending)
        {
            cancelmiopending = true;
        }
    }

    //Then cancel the current command if any
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMediaOutputNode::DoCancelAllCommands Cancelling current command"));
        while (!iCurrentCommand.empty())
        {
            CommandComplete(iCurrentCommand, iCurrentCommand[0], PVMFErrCancelled);
        }
    }

    //next cancel all queued commands
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMediaOutputNode::DoCancelAllCommands Cancelling all queued commands"));
        //start at element 1 since this cancel command is element 0.
        while (iInputCommands.size() > 1)
        {
            CommandComplete(iInputCommands, iInputCommands[1], PVMFErrCancelled);
        }
    }

    if (cancelmiopending)
    {
        // Need to wait for MIO cancel to complete before completing
        // the cancelall command
        return PVMFPending;
    }
    else
    {
        //Cancel is complete
        return PVMFSuccess;
    }
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMediaOutputNode::DoCancelCommand(PVMediaOutputNodeCmd& aCmd)
{
    //extract the command ID from the parameters.
    PVMFCommandId id;
    aCmd.PVMediaOutputNodeCmdBase::Parse(id);

    //first check "current" command if any
    {
        PVMediaOutputNodeCmd* cmd = iCurrentCommand.FindById(id);
        if (cmd)
        {
            if (cmd->iCmd == PVMF_MEDIAOUTPUTNODE_SKIPMEDIADATA)
            {
                //cancel any SkipMediaData in progress on the ports.
                for (uint32 i = 0;i < iInPortVector.size();i++)
                {
                    iInPortVector[i]->CancelSkip();
                }
            }

            //Check if MIO request needs to be cancelled
            bool pendingmiocancel = false;
            if (iMediaIORequest != ENone)
            {
                if (CancelMioRequest(aCmd) == PVMFPending)
                {
                    pendingmiocancel = true;
                }
            }

            //Cancel the queued command
            CommandComplete(iCurrentCommand, *cmd, PVMFErrCancelled);

            if (pendingmiocancel)
            {
                // Wait for MIO cancel to complete
                // before completing CancelCommand
                return PVMFPending;
            }
            else
            {
                //report cancel success
                return PVMFSuccess;
            }
        }
    }

    //next check input queue.
    {
        //start at element 1 since this cancel command is element 0.
        PVMediaOutputNodeCmd* cmd = iInputCommands.FindById(id, 1);
        if (cmd)
        {
            //cancel the queued command
            CommandComplete(iInputCommands, *cmd, PVMFErrCancelled);
            //report cancel success
            return PVMFSuccess;
        }
    }
    //if we get here the command isn't queued so the cancel fails.
    aCmd.iEventCode = PVMFMoutNodeErr_CmdNotQueued;
    return PVMFFailure;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMediaOutputNode::DoReleasePort(PVMediaOutputNodeCmd& aCmd)
{
    //This node supports release port from any state
    PVMFPortInterface* p;
    aCmd.PVMediaOutputNodeCmdBase::Parse(p);
    //search the input port vector
    {
        PVMediaOutputNodePort* port = (PVMediaOutputNodePort*)p;
        PVMediaOutputNodePort** portPtr = iInPortVector.FindByValue(port);
        if (portPtr)
        {
            (*portPtr)->Disconnect();
            iInPortVector.Erase(portPtr);
            return PVMFSuccess;
        }
    }
    aCmd.iEventCode = PVMFMoutNodeErr_PortNotExist;
    return PVMFFailure;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMediaOutputNode::DoReset(PVMediaOutputNodeCmd& aCmd)
{
    OSCL_UNUSED_ARG(aCmd);

    LogDiagnostics();

    // delete all ports and notify observer.
    while (!iInPortVector.empty())
    {
        iInPortVector.front()->Disconnect();
        iInPortVector.Erase(&iInPortVector.front());
    }

    // restore original port vector reserve.
    iInPortVector.Reconstruct();

    if ((iInterfaceState == EPVMFNodeIdle) ||
            (iInterfaceState == EPVMFNodeCreated))
    {
        // node is either in Created or Idle state, no need to call Reset on MIO. MIO has not been
        // connected yet so no need to send a asynchronous command to MIO.
        return PVMFSuccess;
    }
    else
    {
        return SendMioRequest(aCmd, EReset);
    }
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMediaOutputNode::CancelMioRequest(PVMediaOutputNodeCmd& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMediaOutputNode::CancelMioRequest In"));

    OSCL_ASSERT(iMediaIORequest != ENone);

    OSCL_ASSERT(!iMediaIOCancelPending);

    //Issue the cancel to the MIO.
    iMediaIOCancelPending = true;
    int32 err;
    OSCL_TRY(err, iMediaIOCancelCmdId = iMIOControl->CancelCommand(iMediaIOCmdId););
    if (err != OsclErrNone)
    {
        aCmd.iEventCode = PVMFMoutNodeErr_MediaIOCancelCommand;
        iMediaIOCancelPending = false;
        LOGINFOHI((0, "PVMediaOutputNode::CancelMioRequest: CancelCommand on MIO failed"));
        return PVMFFailure;
    }
    LOGINFOHI((0, "PVMediaOutputNode::CancelMioRequest: Cancel Command Issued to MIO component, waiting on response..."));
    return PVMFPending;//wait on request to cancel.
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMediaOutputNode::SendMioRequest(PVMediaOutputNodeCmd& aCmd, EMioRequest aRequest)
{
    //Make an asynchronous request to MIO component
    //and save the request in iMediaIORequest.


    //there should not be a MIO command in progress..
    OSCL_ASSERT(iMediaIORequest == ENone);


    //save media io request.
    iMediaIORequest = aRequest;

    PVMFStatus status;

    //Issue the command to the MIO.
    switch (aRequest)
    {
        case EQueryCapability:
        {
            int32 err ;
            iMIOConfigPVI = NULL;
            OSCL_TRY(err,
                     iMediaIOCmdId = iMIOControl->QueryInterface(PVMI_CAPABILITY_AND_CONFIG_PVUUID,
                                     iMIOConfigPVI, NULL);
                    );

            if (err != OsclErrNone)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMediaOutputNode::SendMioRequest: Error - iMIOControl->QueryInterface(cap & config) failed"));
                aCmd.iEventCode = PVMFMoutNodeErr_MediaIOQueryCapConfigInterface;
                status = PVMFFailure;
            }
            else
            {
                status = PVMFPending;
            }
        }
        break;

        case EQueryClockExtension:
        {
            int32 err ;
            iMIOClockExtensionPVI = NULL;
            OSCL_TRY(err,
                     iMediaIOCmdId = iMIOControl->QueryInterface(PvmiClockExtensionInterfaceUuid,
                                     iMIOClockExtensionPVI, NULL);
                    );

            if (err != OsclErrNone)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMediaOutputNode::SendMioRequest: Error iMIOControl->QueryInterface(clock ext) failed"));
                //this interface is optional so ignore the error
                status = PVMFSuccess;
            }
            else
            {
                status = PVMFPending;
            }
        }
        break;

        case EInit:
        {
            int32 err = OsclErrNone;
            PvmiMediaTransfer* mediaTransfer = NULL;
            if (iInPortVector.size() > 0)
            {
                mediaTransfer = iInPortVector[0]->getMediaTransfer();
            }

            OSCL_TRY(err, iMediaIOCmdId = iMIOControl->Init(););
            if ((err != OsclErrNone))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMediaOutputNode::SendMioRequest: Error - iMIOControl->Init failed"));
                aCmd.iEventCode = PVMFMoutNodeErr_MediaIOInit;
                status = PVMFFailure;
            }
            else
            {
                status = PVMFPending;
            }
        }
        break;

        case EStart:
        {

            int32 err = OsclErrNone;
            PvmiMediaTransfer* mediaTransfer = NULL;
            if (iInPortVector.size() > 0)
            {
                mediaTransfer = iInPortVector[0]->getMediaTransfer();
            }
            if (mediaTransfer != NULL)
            {
                OSCL_TRY(err, iMediaIOCmdId = iMIOControl->Start(););
            }
            if ((err != OsclErrNone) || (mediaTransfer == NULL))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMediaOutputNode::SendMioRequest: Error - iMIOControl->Start failed"));
                aCmd.iEventCode = PVMFMoutNodeErr_MediaIOStart;
                status = PVMFFailure;
            }
            else
            {
                status = PVMFPending;
            }
        }
        break;

        case EPause:
        {
            int32 err = OsclErrNone;
            PvmiMediaTransfer* mediaTransfer = NULL;
            if (iInPortVector.size() > 0)
            {
                mediaTransfer = iInPortVector[0]->getMediaTransfer();
            }
            if (mediaTransfer != NULL)
            {
                OSCL_TRY(err, iMediaIOCmdId = iMIOControl->Pause(););
            }
            if ((err != OsclErrNone) || (mediaTransfer == NULL))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMediaOutputNode::SendMioRequest: Error - iMIOControl->Pause failed"));
                aCmd.iEventCode = PVMFMoutNodeErr_MediaIOPause;
                status = PVMFFailure;
            }
            else
            {
                status = PVMFPending;
            }
        }
        break;

        case EStop:
        {

            int32 err = OsclErrNone;
            PvmiMediaTransfer* mediaTransfer = NULL;
            if (iInPortVector.size() > 0)
            {
                mediaTransfer = iInPortVector[0]->getMediaTransfer();
            }
            else
            {
                /*There can be cases where stop is called after ports have been released and in such cases
                we succeed stop as no-op assuming a subsequent reset will be called which would
                guarantee proper reset of mio comp. */
                return PVMFSuccess;
            }
            if (mediaTransfer != NULL)
            {
                OSCL_TRY(err, iMediaIOCmdId = iMIOControl->Stop(););
            }
            if ((err != OsclErrNone) || (mediaTransfer == NULL))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMediaOutputNode::SendMioRequest: Error - iMIOControl->Stop failed"));
                aCmd.iEventCode = PVMFMoutNodeErr_MediaIOStop;
                status = PVMFFailure;
            }
            else
            {
                status = PVMFPending;
            }
        }
        break;

        case EDiscard:
        {
            int32 err = OsclErrNone;
            PvmiMediaTransfer* mediaTransfer = NULL;
            if (iInPortVector.size() > 0)
            {
                mediaTransfer = iInPortVector[0]->getMediaTransfer();
            }
            if (mediaTransfer != NULL)
            {
                PVMFTimestamp resumeTimestamp;
                bool playbackpositioncontinuous;
                uint32 streamId;
                aCmd.Parse(resumeTimestamp, playbackpositioncontinuous, streamId);
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iReposLogger, PVLOGMSG_INFO,
                                (0, "PVMediaOutputNode::SendMioRequest(EDiscard): skipTimestamp=%d", resumeTimestamp));
                OSCL_TRY(err, iMediaIOCmdId = iMIOControl->DiscardData(resumeTimestamp););
            }
            if ((err != OsclErrNone) || (mediaTransfer == NULL))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMediaOutputNode::SendMioRequest: Error - iMIOControl->DiscardData failed"));
                aCmd.iEventCode = PVMFMoutNodeErr_MediaIODiscardData;
                status = PVMFFailure;
            }
            else
            {
                status = PVMFPending;
            }
        }
        break;

        case EReset:
        {
            int32 err ;
            OSCL_TRY(err, iMediaIOCmdId = iMIOControl->Reset(););
            if (err != OsclErrNone)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMediaOutputNode::SendMioRequest: Error - iMIOControl->Reset failed"));
                aCmd.iEventCode = PVMFMoutNodeErr_MediaIOReset;
                status = PVMFFailure;
            }
            else
            {
                status = PVMFPending;
            }
        }
        break;

        default:
            OSCL_ASSERT(false);//unrecognized command.
            status = PVMFFailure;
            break;
    }

    if (status == PVMFPending)
    {
        LOGINFOHI((0, "PVMediaOutputNode:SendMIORequest: Command Issued to MIO component, waiting on response..."));
    }

    return status;
}




/////////////////////////////////////////////////////
// Event reporting routines.
/////////////////////////////////////////////////////
void PVMediaOutputNode::SetState(TPVMFNodeInterfaceState s)
{
    LOGINFO((0, "PVMediaOutputNode:SetState %d", s));
    PVMFNodeInterface::SetState(s);
}

void PVMediaOutputNode::ReportErrorEvent(PVMFEventType aEventType, OsclAny* aEventData, PVMFStatus aEventCode)
{
    LOGERROR((0, "PVMediaOutputNode:NodeErrorEvent Type %d EVData %d EVCode %d"
              , aEventType, aEventData, aEventCode));

    //create the extension message if any.
    if (aEventCode != PVMFMoutNodeErr_First)
    {
        PVMFBasicErrorInfoMessage* eventmsg = OSCL_NEW(PVMFBasicErrorInfoMessage, (aEventCode, iEventUuid, NULL));
        PVMFAsyncEvent asyncevent(PVMFErrorEvent, aEventType, NULL, OSCL_STATIC_CAST(PVInterface*, eventmsg), aEventData, NULL, 0);
        PVMFNodeInterface::ReportErrorEvent(asyncevent);
        eventmsg->removeRef();
    }
    else
        PVMFNodeInterface::ReportErrorEvent(aEventType, aEventData);
}

void PVMediaOutputNode::ReportInfoEvent(PVMFEventType aEventType, OsclAny* aEventData, PVMFStatus aEventCode)
{
    LOGINFO((0, "PVMediaOutputNode:NodeInfoEvent Type %d EVData %d EVCode %d"
             , aEventType, aEventData, aEventCode));

    //create the extension message if any.
    if (aEventCode != PVMFMoutNodeErr_First)
    {
        PVMFBasicErrorInfoMessage* eventmsg = OSCL_NEW(PVMFBasicErrorInfoMessage, (aEventCode, iEventUuid, NULL));
        PVMFAsyncEvent asyncevent(PVMFErrorEvent, aEventType, NULL, OSCL_STATIC_CAST(PVInterface*, eventmsg), aEventData, NULL, 0);
        PVMFNodeInterface::ReportInfoEvent(asyncevent);
        eventmsg->removeRef();
    }
    else
        PVMFNodeInterface::ReportInfoEvent(aEventType, aEventData);
}

OSCL_EXPORT_REF PVMFStatus PVMediaOutputNode::verifyParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters, int num_elements)
{
    OSCL_UNUSED_ARG(aSession);
    if (iMIOConfig)
    {
        return (iMIOConfig->verifyParametersSync(iMIOSession, aParameters, num_elements));
    }
    else
    {
        return PVMFFailure;
    }
}

OSCL_EXPORT_REF PVMFStatus PVMediaOutputNode::getParametersSync(PvmiMIOSession aSession,
        PvmiKeyType aIdentifier,
        PvmiKvp*& aParameters,
        int& aNumParamElements,
        PvmiCapabilityContext aContext)
{
    OSCL_UNUSED_ARG(aSession);
    return (iMIOConfig->getParametersSync(iMIOSession, aIdentifier, aParameters, aNumParamElements, aContext));
}

OSCL_EXPORT_REF PVMFStatus PVMediaOutputNode::releaseParameters(PvmiMIOSession aSession, PvmiKvp* aParameters, int num_elements)
{
    OSCL_UNUSED_ARG(aSession);
    return (iMIOConfig->releaseParameters(iMIOSession, aParameters, num_elements));
}

void PVMediaOutputNode::ReportBOS()
{
    CompleteSkipMediaData();
}

PVMFStatus PVMediaOutputNode::CheckForBOS()
{
    Oscl_Vector<uint32, OsclMemAllocator>::iterator it;
    for (it = iInPortVector[0]->iBOSStreamIDVec.begin();
            it != iInPortVector[0]->iBOSStreamIDVec.end();
            it++)
    {
        if (*it == iRecentBOSStreamID)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iReposLogger, PVLOGMSG_INFO,
                            (0, "PVMediaOutputNode::CheckForBOS - BOS Found - Mime=%s, BOSStreamID=%d",
                             iSinkFormatString.get_str(), iRecentBOSStreamID));
            //we have recvd BOS
            return PVMFSuccess;
        }
    }
    return PVMFPending;
}

void PVMediaOutputNode::LogDiagnostics()
{
    if (iDiagnosticsLogged == false)
    {
        iDiagnosticsLogged = true;

        if (!iInPortVector.empty())
        {
            LOGDIAGNOSTICS((0, "PVMediaOutputNode:LogDiagnostics Mime %s, FramesDropped/TotalFrames %d/%d"
                            , iSinkFormatString.get_str(), iInPortVector[0]->iFramesDropped, iInPortVector[0]->iTotalFrames));
        }
    }
}












