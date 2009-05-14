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
#ifndef PVMF_STREAMING_MANAGER_NODE_H_INCLUDED
#include "pvmf_streaming_manager_node.h"
#endif

#ifndef PV_INTERFACE_H
#include "pv_interface.h"
#endif

#ifndef PVMF_SM_FSP_REGISTRY_H
#include "pvmf_sm_fsp_registry.h"
#endif

#ifndef PVMF_BASIC_ERRORINFOMESSAGE_H_INCLUDED
#include "pvmf_basic_errorinfomessage.h"
#endif

#ifndef PVMF_SM_FSP_REGISTRY_H
#include "pvmf_sm_fsp_registry.h"
#endif

#ifndef OSCL_DLL_H_INCLUDED
#include "oscl_dll.h"
#endif

//////////////////////////////////////////////////////////////////////
//To make this module as DLL we need to define entry point for the DLL
//////////////////////////////////////////////////////////////////////
OSCL_DLL_ENTRY_POINT_DEFAULT()


//////////////////////////////////////////////////
// Node Constructor & Destructor
//////////////////////////////////////////////////


PVMFStreamingManagerNode* PVMFStreamingManagerNode::New(int32 aPriority)
{
    PVMFStreamingManagerNode* smNode = NULL;
    smNode = OSCL_NEW(PVMFStreamingManagerNode, (aPriority));
    if (smNode)
    {
        smNode->Construct();
    }
    return smNode;
}

OSCL_EXPORT_REF PVMFStreamingManagerNode::PVMFStreamingManagerNode(int32 aPriority):
        OsclActiveObject(aPriority, "StreamingManagerNode"),
        iSMFSPRegistry(NULL),
        iSMFSPlugin(NULL),
        iLogger(NULL)
{
    iFSPDataSourceInitializationIntf = NULL;
}

void PVMFStreamingManagerNode::Construct()
{
    int32 err;
    OSCL_TRY(err,
             /*
              * Create the input command queue.  Use a reserve to avoid lots of
              * dynamic memory allocation.
              */
             iInputCommands.Construct(PVMF_STREAMING_MANAGER_NODE_COMMAND_ID_START,
                                      PVMF_STREAMING_MANAGER_VECTOR_RESERVE);
             iSMFSPRegistry = OSCL_NEW(PVMFSMFSPRegistry, ());
            );

    if (err != OsclErrNone)
    {
        OSCL_LEAVE(err);
    }
}

OSCL_EXPORT_REF PVMFStreamingManagerNode::~PVMFStreamingManagerNode()
{
    PVMF_SM_LOGSTACKTRACE((0, "PVMFStreamingManagerNode::~PVMFStreamingManagerNode - In"));

    if (iSMFSPlugin)
    {
        if (iSMFSPRegistry)
        {
            if (iFSPDataSourceInitializationIntf)
            {
                iFSPDataSourceInitializationIntf->removeRef();
                iFSPDataSourceInitializationIntf = NULL;
            }

            bool retval = iSMFSPRegistry->ReleaseSMFSP(iFSPUuid, iSMFSPlugin);
            if (false == retval)
            {
                PVMF_SM_LOGERROR((0, "PVMFStreamingManagerNode::~PVMFStreamingManagerNode - Error FSP could not be deleted "));
            }
            iSMFSPlugin = NULL;
        }
    }
    if (iSMFSPRegistry)
    {
        OSCL_DELETE(iSMFSPRegistry);
        iSMFSPRegistry = NULL;
    }

    iLogger = NULL;

    PVMF_SM_LOGSTACKTRACE((0, "PVMFStreamingManagerNode::~PVMFStreamingManagerNode - Out"));
}

/* PVMFNodeInterface's virtual functions implemenation*/
OSCL_EXPORT_REF PVMFStatus PVMFStreamingManagerNode::ThreadLogon()
{
    PVMF_SM_LOGSTACKTRACE((0, "PVMFStreamingManagerNode::ThreadLogon - In"));

    PVMFStatus status = PVMFSuccess;
    switch (iInterfaceState)
    {
        case EPVMFNodeCreated:
        {
            if (!IsAdded())
                AddToScheduler();
            iLogger = PVLogger::GetLoggerObject("PVMFStreamingManagerNode");
            SetState(EPVMFNodeIdle);
        }
        break;
        default:
            status = PVMFErrInvalidState;
            break;
    }

    if (PVMFSuccess == status && iSMFSPlugin)
    {
        status = iSMFSPlugin->ThreadLogon();
    }

    PVMF_SM_LOGSTACKTRACE((0, "PVMFStreamingManagerNode::ThreadLogon - Out status[%d]", status));
    return (status);
}

OSCL_EXPORT_REF PVMFStatus PVMFStreamingManagerNode::ThreadLogoff()
{
    PVMF_SM_LOGSTACKTRACE((0, "PVMFStreamingManagerNode::ThreadLogoff - In"));


    PVMFStatus status = PVMFSuccess;

    if (iSMFSPlugin)
    {
        status = iSMFSPlugin->ThreadLogoff();
        OSCL_ASSERT(PVMFFailure != status);
        if (PVMFSuccess != status)
        {
            return status;
        }
    }

    switch (iInterfaceState)
    {
        case EPVMFNodeIdle:
        {
            iLogger = NULL;
            if (IsAdded())
                RemoveFromScheduler();
            SetState(EPVMFNodeCreated);
        }
        break;

        case EPVMFNodeCreated:
            status = PVMFSuccess;
            break;

        default:
            status = PVMFErrInvalidState;
            break;
    }

    PVMF_SM_LOGSTACKTRACE((0, "PVMFStreamingManagerNode::ThreadLogoff - Out status[%d]", status));
    return status;
}

PVMFSessionId PVMFStreamingManagerNode::Connect(const PVMFNodeSessionInfo &aSession)
{
    iUpstreamSession.iId = PVMFNodeInterface::Connect(aSession);
    iUpstreamSession.iInfo =  aSession;
    if (iSMFSPlugin)
    {
        iSMFSPlugin->Connect(iUpstreamSession);
    }
    return iUpstreamSession.iId;
}

PVMFStatus PVMFStreamingManagerNode::Disconnect(PVMFSessionId aSessionId)
{
    if (iSMFSPlugin)
        iSMFSPlugin->Disconnect(aSessionId);
    return PVMFNodeInterface::Disconnect(aSessionId);
}

OSCL_EXPORT_REF PVMFStatus PVMFStreamingManagerNode::GetCapability(PVMFNodeCapability& aNodeCapability)
{
    PVMF_SM_LOGSTACKTRACE((0, "PVMFStreamingManagerNode::GetCapability - In"));

    PVMFStatus status = PVMFSuccess;
    if (iSMFSPlugin)
    {
        status = iSMFSPlugin->GetCapability(aNodeCapability);
    }
    else
    {
        PVMF_SM_LOGERROR((0, "PVMFStreamingManagerNode::GetCapability - Error iSMFSPlugin 0x%x, ", iSMFSPlugin));
        status = PVMFFailure;
    }

    PVMF_SM_LOGSTACKTRACE((0, "PVMFStreamingManagerNode::GetCapability - Out status[%d]", status));
    return status;
}

OSCL_EXPORT_REF PVMFPortIter* PVMFStreamingManagerNode::GetPorts(const PVMFPortFilter* aFilter)
{
    PVMF_SM_LOGSTACKTRACE((0, "PVMFStreamingManagerNode::GetPorts - In"));

    PVMFPortIter* portIter = NULL;
    if (iSMFSPlugin)
    {
        portIter = iSMFSPlugin->GetPorts(aFilter);
    }
    else
    {
        PVMF_SM_LOGERROR((0, "PVMFStreamingManagerNode::GetPorts - Error iSMFSPlugin 0x%x", iSMFSPlugin));
    }

    PVMF_SM_LOGSTACKTRACE((0, "PVMFStreamingManagerNode::GetPorts - Out portIter[0x%x]", portIter));
    return portIter;
}

OSCL_EXPORT_REF PVMFCommandId PVMFStreamingManagerNode::QueryUUID(PVMFSessionId aSessId,
        const PvmfMimeString& aMimeType,
        Oscl_Vector< PVUuid, OsclMemAllocator >& aUuids,
        bool aExactUuidsOnly,
        const OsclAny* aContext)
{
    PVMF_SM_LOGERROR((0, "PVMFStreamingManagerNode::QueryUUID - Error Deprecated Interface - Call Not Expected"));
    OSCL_UNUSED_ARG(aSessId);
    OSCL_UNUSED_ARG(aMimeType);
    OSCL_UNUSED_ARG(aUuids);
    OSCL_UNUSED_ARG(aExactUuidsOnly);
    OSCL_UNUSED_ARG(aContext);
    //We dont expect the user of streaming manager node to call QueryUUID, this API is deprecated for SM node.
    OSCL_LEAVE(OsclErrNotSupported);
    return 0;
}

OSCL_EXPORT_REF PVMFCommandId PVMFStreamingManagerNode::QueryInterface(PVMFSessionId aSessId, const PVUuid& aUuid,
        PVInterface*& aInterfacePtr,
        const OsclAny* aContext)
{
    PVMF_SM_LOGSTACKTRACE((0, "PVMFStreamingManagerNode::QueryInterface - In"));

    //Check if we can handle this API in this node itself
    PVInterface* ifptr = NULL;
    if (queryInterface(aUuid, ifptr) == false)
    {
        if (!iSMFSPlugin)
            OSCL_LEAVE(OsclErrInvalidState);
        return 	iSMFSPlugin->QueryInterface(aSessId, aUuid, aInterfacePtr, aContext);
    }
    else
    {
        PVMFStreamingManagerNodeCommand cmd;
        cmd.Construct(aSessId, PVMF_STREAMING_MANAGER_NODE_QUERYINTERFACE, aUuid, aInterfacePtr, aContext);
        PVMF_SM_LOGSTACKTRACE((0, "PVMFStreamingManagerNode::QueryInterface - Out"));
        return QueueCommandL(cmd);
    }
}

OSCL_EXPORT_REF PVMFCommandId PVMFStreamingManagerNode::RequestPort(PVMFSessionId aSessId,
        int32 aPortTag,
        const PvmfMimeString* aPortConfig,
        const OsclAny* aContext)
{
    PVMF_SM_LOGSTACKTRACE((0, "PVMFStreamingManagerNode::RequestPort - In"));

    if (!iSMFSPlugin)
        OSCL_LEAVE(OsclErrInvalidState);

    PVMFCommandId cmdId = 0;
    cmdId = iSMFSPlugin->RequestPort(aSessId, aPortTag, aPortConfig, aContext);

    PVMF_SM_LOGSTACKTRACE((0, "PVMFStreamingManagerNode::RequestPort - Out cmdId[%d]", cmdId));
    return cmdId;
}

OSCL_EXPORT_REF PVMFCommandId PVMFStreamingManagerNode::ReleasePort(PVMFSessionId aSessId,
        PVMFPortInterface& aPort,
        const OsclAny* aContext)
{
    PVMF_SM_LOGSTACKTRACE((0, "PVMFStreamingManagerNode::ReleasePort - In"));

    if (!iSMFSPlugin)
        OSCL_LEAVE(OsclErrInvalidState);

    PVMFCommandId cmdId = 0;
    cmdId = iSMFSPlugin->ReleasePort(aSessId, aPort, aContext);

    PVMF_SM_LOGSTACKTRACE((0, "PVMFStreamingManagerNode::ReleasePort - Out cmdId[%d]", cmdId));
    return cmdId;
}

OSCL_EXPORT_REF PVMFCommandId PVMFStreamingManagerNode::Init(PVMFSessionId aSessId,
        const OsclAny* aContext)
{
    PVMF_SM_LOGSTACKTRACE((0, "PVMFStreamingManagerNode::Init - In"));

    if (!iSMFSPlugin)
        OSCL_LEAVE(OsclErrInvalidState);

    PVMFCommandId cmdId = 0;
    cmdId = iSMFSPlugin->Init(aSessId, aContext);

    PVMF_SM_LOGSTACKTRACE((0, "PVMFStreamingManagerNode::Init - Out cmdId[%d]", cmdId));
    return cmdId;
}

OSCL_EXPORT_REF PVMFCommandId PVMFStreamingManagerNode::Prepare(PVMFSessionId aSessId,
        const OsclAny* aContext)
{
    PVMF_SM_LOGSTACKTRACE((0, "PVMFStreamingManagerNode::Prepare - In"));

    if (!iSMFSPlugin)
        OSCL_LEAVE(OsclErrInvalidState);

    PVMFCommandId cmdId = 0;
    cmdId = iSMFSPlugin->Prepare(aSessId, aContext);

    PVMF_SM_LOGSTACKTRACE((0, "PVMFStreamingManagerNode::Prepare - Out cmdId[%d]", cmdId));
    return cmdId;
}

OSCL_EXPORT_REF PVMFCommandId PVMFStreamingManagerNode::Start(PVMFSessionId aSessId,
        const OsclAny* aContext)
{
    PVMF_SM_LOGSTACKTRACE((0, "PVMFStreamingManagerNode::Start - In"));

    if (!iSMFSPlugin)
        OSCL_LEAVE(OsclErrInvalidState);

    PVMFCommandId cmdId = 0;
    cmdId = iSMFSPlugin->Start(aSessId, aContext);

    PVMF_SM_LOGSTACKTRACE((0, "PVMFStreamingManagerNode::Start - Out cmdId[%d]", cmdId));
    return cmdId;
}

OSCL_EXPORT_REF PVMFCommandId PVMFStreamingManagerNode::Stop(PVMFSessionId aSessId,
        const OsclAny* aContext)
{
    PVMF_SM_LOGSTACKTRACE((0, "PVMFStreamingManagerNode::Stop - In"));

    if (!iSMFSPlugin)
        OSCL_LEAVE(OsclErrInvalidState);

    PVMFCommandId cmdId = 0;
    cmdId = iSMFSPlugin->Stop(aSessId, aContext);

    PVMF_SM_LOGSTACKTRACE((0, "PVMFStreamingManagerNode::Stop - Out cmdId[%d]", cmdId));
    return cmdId;
}

OSCL_EXPORT_REF PVMFCommandId PVMFStreamingManagerNode::Flush(PVMFSessionId aSessId,
        const OsclAny* aContext)
{
    PVMF_SM_LOGSTACKTRACE((0, "PVMFStreamingManagerNode::Flush - In"));

    if (!iSMFSPlugin)
        OSCL_LEAVE(OsclErrInvalidState);

    PVMFCommandId cmdId = 0;
    cmdId = iSMFSPlugin->Flush(aSessId, aContext);

    PVMF_SM_LOGSTACKTRACE((0, "PVMFStreamingManagerNode::Flush - Out cmdId[%d]", cmdId));
    return cmdId;
}

OSCL_EXPORT_REF PVMFCommandId PVMFStreamingManagerNode::Pause(PVMFSessionId aSessId,
        const OsclAny* aContext)
{
    PVMF_SM_LOGSTACKTRACE((0, "PVMFStreamingManagerNode::Pause - In"));

    if (!iSMFSPlugin)
    {
        PVMF_SM_LOGSTACKTRACE((0, "PVMFStreamingManagerNode::Pause - FSP was not existing"));
        OSCL_LEAVE(OsclErrInvalidState);
    }

    PVMFCommandId cmdId = 0;
    cmdId = iSMFSPlugin->Pause(aSessId, aContext);

    PVMF_SM_LOGSTACKTRACE((0, "PVMFStreamingManagerNode::Pause - Out cmdId[%d]", cmdId));
    return cmdId;
}

OSCL_EXPORT_REF PVMFCommandId PVMFStreamingManagerNode::Reset(PVMFSessionId aSessId,
        const OsclAny* aContext)
{
    PVMF_SM_LOGSTACKTRACE((0, "PVMFStreamingManagerNode::Reset - In"));

    PVMFCommandId cmdId = 0;
    if (iSMFSPlugin)
    {
        cmdId = iSMFSPlugin->Reset(aSessId, aContext);
    }
    else
    {
        PVMFStreamingManagerNodeCommand cmd;
        cmd.Construct(aSessId, PVMF_STREAMING_MANAGER_NODE_RESET, aContext);
        cmdId = QueueCommandL(cmd);
    }

    PVMF_SM_LOGSTACKTRACE((0, "PVMFStreamingManagerNode::Reset - Out cmdId[%d]", cmdId));
    return cmdId;
}

OSCL_EXPORT_REF PVMFCommandId PVMFStreamingManagerNode::CancelAllCommands(PVMFSessionId aSessId,
        const OsclAny* aContextData)
{
    PVMF_SM_LOGSTACKTRACE((0, "StreamingManagerNode:CancelAllCommands"));

    PVMFCommandId cmdId = 0;
    if (iSMFSPlugin)
    {
        cmdId = iSMFSPlugin->CancelAllCommands(aSessId, aContextData);
    }
    else
    {
        PVMFStreamingManagerNodeCommand cmd;
        cmd.Construct(aSessId, PVMF_STREAMING_MANAGER_NODE_CANCELALLCOMMANDS, aContextData);
        cmdId = QueueCommandL(cmd);
    }

    PVMF_SM_LOGSTACKTRACE((0, "PVMFStreamingManagerNode::CancelAllCommands() - Out cmdId[%d]", cmdId));
    return cmdId;
}

OSCL_EXPORT_REF PVMFCommandId PVMFStreamingManagerNode::CancelCommand(PVMFSessionId aSessId,
        PVMFCommandId aCmdId,
        const OsclAny* aContextData)
{
    PVMF_SM_LOGSTACKTRACE((0, "StreamingManagerNode:CancelCommand with id %d", aCmdId));

    PVMFCommandId cmdId = 0;
    if (iSMFSPlugin)
    {
        cmdId = iSMFSPlugin->CancelCommand(aSessId, aCmdId, aContextData);
    }
    else
    {
        PVMFStreamingManagerNodeCommand cmd;
        cmd.Construct(aSessId, PVMF_STREAMING_MANAGER_NODE_CANCELCOMMAND, aContextData);
        cmdId = QueueCommandL(cmd);
    }

    PVMF_SM_LOGSTACKTRACE((0, "PVMFStreamingManagerNode::CancelCommand() - Out cmdId[%d]", cmdId));
    return cmdId;
}

/* PVMFPortActivityHandler's virtual functions implemenation */
void PVMFStreamingManagerNode::HandlePortActivity(const PVMFPortActivity& aActivity)
{
    //this node just acts as a proxy and does'nt has any port of its own, So, this func will never be called
    OSCL_UNUSED_ARG(aActivity);
    OSCL_ASSERT(false);
}

/* PVMFDataSourceInitializationExtensionInterface's virtual functions implemenation */
PVMFStatus PVMFStreamingManagerNode::SetSourceInitializationData(OSCL_wString& aSourceURL,
        PVMFFormatType& aSourceFormat,
        OsclAny* aSourceData)
{
    PVMF_SM_LOGSTACKTRACE((0, "PVMFStreamingManagerNode::SetSourceInitializationData() In"));
    PVMFStatus status = PVMFFailure;
    //we may have the sequence like
    //Reset->queryinterface(PVMFDataSourceInitializationExtensionInterface)->PVMFDataSourceInitializationExtensionInterface::SetSourceInitializationData
    //Hence, we cannot rule out the possibility of FSP plugin already being created or multiple calls to
    //PVMFDataSourceInitializationExtensionInterface::SetSourceInitializationData
    if (iSMFSPlugin)
    {
        if (iSMFSPRegistry)
        {
            if (iFSPDataSourceInitializationIntf)
            {
                iFSPDataSourceInitializationIntf->removeRef();
                iFSPDataSourceInitializationIntf = NULL;
            }
            iSMFSPRegistry->ReleaseSMFSP(iFSPUuid, iSMFSPlugin);
            iSMFSPlugin = NULL;
        }
    }

    PVMFFormatType fspSrcFormat = aSourceFormat;
    //Check if tunelling plugin is to be instantiated?
    if (aSourceFormat == PVMF_MIME_DATA_SOURCE_RTSP_URL)//check for SDP files too???
    {
        OSCL_wStackString<8> rtsptScheme(_STRLIT_WCHAR("rtspt"));
        if (oscl_strncmp(rtsptScheme.get_cstr(), aSourceURL.get_cstr(), 5) == 0)
        {
            fspSrcFormat = PVMF_MIME_DATA_SOURCE_RTSP_TUNNELLING;
        }
    }

    Oscl_Vector<PVUuid, OsclMemAllocator> srcNodeUuidVec;
    if (PVMFSuccess == iSMFSPRegistry->QueryRegistry(fspSrcFormat, srcNodeUuidVec))
    {
        iFSPUuid = srcNodeUuidVec.front();
        iSMFSPlugin = iSMFSPRegistry->CreateSMFSP(iFSPUuid);
        //(ii)threadlogon to FSP
        iSMFSPlugin->ThreadLogon();
        //(iii)connect to it, make sure datasource initialization interface is present
        PVMFSessionId sessID = iSMFSPlugin->Connect(iUpstreamSession);
        PVInterface* temp = NULL;
        PVMFStatus statusQ = PVMFFailure;

        statusQ = iSMFSPlugin->queryInterface(PVMF_DATA_SOURCE_INIT_INTERFACE_UUID, temp);
        iFSPDataSourceInitializationIntf = OSCL_STATIC_CAST(PVMFDataSourceInitializationExtensionInterface*, temp);

        if ((statusQ == PVMFSuccess) && (iFSPDataSourceInitializationIntf != NULL))
        {
            status = iFSPDataSourceInitializationIntf->SetSourceInitializationData(aSourceURL, aSourceFormat, aSourceData);
        }
        else
        {
            iSMFSPlugin->Disconnect(sessID);
            iSMFSPlugin->ThreadLogoff();
            iSMFSPRegistry->ReleaseSMFSP(iFSPUuid, iSMFSPlugin);
            iSMFSPlugin = NULL;
            PVMF_SM_LOGSTACKTRACE((0, "PVMFStreamingManagerNode::SetSourceInitializationData() - Destructed FSP "));
        }
    }

    return status;
}

PVMFStatus PVMFStreamingManagerNode::SetClientPlayBackClock(PVMFMediaClock* aClientClock)
{
    PVMF_SM_LOGSTACKTRACE((0, "PVMFStreamingManagerNode::SetClientPlayBackClock - In"));

    PVMFStatus status = PVMFSuccess;
    if (iFSPDataSourceInitializationIntf)
    {
        status = iFSPDataSourceInitializationIntf->SetClientPlayBackClock(aClientClock);
    }
    else
    {
        status = PVMFFailure;
    }

    PVMF_SM_LOGSTACKTRACE((0, "PVMFStreamingManagerNode::SetClientPlayBackClock - Out status [%d]", status));
    return status;
}

PVMFStatus PVMFStreamingManagerNode::SetEstimatedServerClock(PVMFMediaClock* aEstimatedServerClock)
{
    PVMF_SM_LOGSTACKTRACE((0, "PVMFStreamingManagerNode::SetEstimatedServerClock - In"));

    PVMFStatus status = PVMFSuccess;
    if (iFSPDataSourceInitializationIntf)
    {
        status = iFSPDataSourceInitializationIntf->SetEstimatedServerClock(aEstimatedServerClock);
    }
    else
    {
        status = PVMFFailure;
    }

    PVMF_SM_LOGSTACKTRACE((0, "PVMFStreamingManagerNode::SetEstimatedServerClock - Out status [%d]", status));
    return status;
}

/**
 * This routine is called by various command APIs to queue an
 * asynchronous command for processing by the command handler AO.
 * This function may leave if the command can't be queued due to
 * memory allocation failure.
 */
PVMFCommandId PVMFStreamingManagerNode::QueueCommandL(PVMFStreamingManagerNodeCommand& aCmd)
{
    PVMFCommandId id;

    id = iInputCommands.AddL(aCmd);

    if (IsAdded())
    {
        //wakeup the AO
        RunIfNotReady();
    }
    return id;
}

/* From OsclActiveObject */
void PVMFStreamingManagerNode::Run()
{
    PVMF_SM_LOGSTACKTRACE((0, "PVMFStreamingManagerNode::Run - In"));
    /* Process commands */
    if (!iInputCommands.empty())
    {
        if (ProcessCommand(iInputCommands.front()))
        {
            /*
             * re-schedule if more commands to do
             * and node isn't reset.
             */
            if (!iInputCommands.empty() && iInterfaceState != EPVMFNodeCreated)
            {
                if (IsAdded())
                {
                    RunIfNotReady();
                }
            }
            return;
        }
    }

    PVMF_SM_LOGSTACKTRACE((0, "PVMFStreamingManagerNode::Run - Out"));
}

void PVMFStreamingManagerNode::DoCancel()
{
    PVMF_SM_LOGSTACKTRACE((0, "PVMFStreamingManagerNode::DoCancel - In"));
    /* the base class cancel operation is sufficient */
    OsclActiveObject::DoCancel();
    PVMF_SM_LOGSTACKTRACE((0, "PVMFStreamingManagerNode::DoCancel - Out"));
}

bool PVMFStreamingManagerNode::ProcessCommand(PVMFStreamingManagerNodeCommand& aCmd)
{
    PVMF_SM_LOGSTACKTRACE((0, "PVMFStreamingManagerNode::ProcessCommand - In"));
    //Because all the commands are processed in one AO cycle. So, there's no need for current command Q/Cancel command Q.

    switch (aCmd.iCmd)
    {
        case PVMF_STREAMING_MANAGER_NODE_QUERYINTERFACE:
            DoQueryInterface(aCmd);
            break;

        case PVMF_STREAMING_MANAGER_NODE_RESET:
            ThreadLogoff();
            CommandComplete(iInputCommands, aCmd, PVMFSuccess);
            break;

        case PVMF_STREAMING_MANAGER_NODE_CANCELALLCOMMANDS:
            DoCancelAllCommands(aCmd);
            break;

        case PVMF_STREAMING_MANAGER_NODE_CANCELCOMMAND:
            CommandComplete(iInputCommands, aCmd, PVMFErrNotSupported);
            break;

        default:
            /* unknown command type */
            CommandComplete(iInputCommands, aCmd, PVMFErrNotSupported);
            break;
    }
    return true;
}

bool PVMFStreamingManagerNode::DoCancelAllCommands(PVMFStreamingManagerNodeCommand& aCmd)
{
    const int32 cancelCmdId = iInputCommands.front().iId;
    const int32 inputCmndsSz = iInputCommands.size();

    for (int ii = inputCmndsSz - 1; ii > 0 ; ii--) //we dont want to process element at index 0 (being cancel command)
    {
        if (iInputCommands[ii].iId < cancelCmdId)
        {
            CommandComplete(iInputCommands, iInputCommands[ii], PVMFErrCancelled);
        }
    }
    CommandComplete(iInputCommands, aCmd, PVMFSuccess); //complete the cancel all command

    PVMF_SM_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::DoCancelAllCommands Out"));
    return true;
}

void PVMFStreamingManagerNode::CommandComplete(PVMFStreamingManagerNodeCmdQ& aCmdQ,
        PVMFStreamingManagerNodeCommand& aCmd,
        PVMFStatus aStatus,
        OsclAny* aEventData,
        PVUuid* aEventUUID,
        int32* aEventCode)
{
    PVMF_SM_LOGSTACKTRACE((0, "StreamingManagerNode:CommandComplete Id %d Cmd %d Status %d Context %d Data %d"
                           , aCmd.iId, aCmd.iCmd, aStatus, aCmd.iContext, aEventData));

    PVInterface* extif = NULL;
    PVMFBasicErrorInfoMessage* errormsg = NULL;
    if (aEventUUID && aEventCode)
    {
        errormsg = OSCL_NEW(PVMFBasicErrorInfoMessage, (*aEventCode, *aEventUUID, NULL));
        extif = OSCL_STATIC_CAST(PVInterface*, errormsg);
    }

    /* create response */
    PVMFCmdResp resp(aCmd.iId, aCmd.iContext, aStatus, extif, aEventData);
    PVMFSessionId session = aCmd.iSession;

    /* Erase the command from the queue */
    aCmdQ.Erase(&aCmd);

    /* Report completion to the session observer */
    ReportCmdCompleteEvent(session, resp);

    if (errormsg)
    {
        errormsg->removeRef();
    }

    /* Reschedule AO if input command queue is not empty */
    if (!iInputCommands.empty() && IsAdded())
    {
        if (IsAdded())
        {
            RunIfNotReady();
        }
    }
}

void PVMFStreamingManagerNode::DoQueryInterface(PVMFStreamingManagerNodeCommand& aCmd)
{
    PVMF_SM_LOGSTACKTRACE((0, "PVMFStreamingManagerNode::DoQueryInterface - In"));

    PVUuid* uuid;
    PVInterface** ifptr;
    aCmd.Parse(uuid, ifptr);
    if (ifptr == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFStreamingManagerNode::DoQueryInterface() Passed in parameter invalid."));
        CommandComplete(iInputCommands, aCmd, PVMFErrArgument);
        return;
    }

    if (queryInterface(*uuid, *ifptr) == false)
    {
        CommandComplete(iInputCommands, aCmd, PVMFFailure);
    }
    else
    {
        (*ifptr)->addRef();
        CommandComplete(iInputCommands, aCmd, PVMFSuccess);
    }

    PVMF_SM_LOGSTACKTRACE((0, "PVMFStreamingManagerNode::DoQueryInterface - Out"));
}

bool PVMFStreamingManagerNode::queryInterface(const PVUuid& uuid, PVInterface*& iface)
{
    PVMF_SM_LOGSTACKTRACE((0, "PVMFStreamingManagerNode::queryInterface - In"));

    if (uuid == PVMF_DATA_SOURCE_INIT_INTERFACE_UUID)
    {
        iface = OSCL_STATIC_CAST(PVInterface*, this);
    }
    else
    {
        PVMF_SM_LOGERROR((0, "PVMFStreamingManagerNode::queryInterface() Unsupported interface UUID."));
        return false;
    }

    PVMF_SM_LOGSTACKTRACE((0, "PVMFStreamingManagerNode::queryInterface - Out"));
    return true;
}

//Streaming Source node implements all the interfaces in the Streaming Manager
//Node or FSP, So they are created and destroyed with the creation/destruction
//of StreamingManager Node/FSP. Hence addref and removeref doesn't really do
//any thing..
void PVMFStreamingManagerNode::addRef()
{

}

void PVMFStreamingManagerNode::removeRef()
{

}
