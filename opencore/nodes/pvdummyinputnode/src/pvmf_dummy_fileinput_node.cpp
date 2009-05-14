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
#ifndef PVMF_DUMMY_FILEINPUT_NODE_H_INCLUDED
#include "pvmf_dummy_fileinput_node.h"
#endif
#ifndef PVMF_FILEINPUT_NODE_INTERNAL_H_INCLUDED
#include "pvmf_fileinput_node_internal.h"
#endif
#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif
#ifndef PVMF_COMMON_AUDIO_DECNODE_H_INCLUDE
#include "pvmf_common_audio_decnode.h"
#endif
#ifndef PVMF_VIDEO_H_INCLUDED
#include "pvmf_video.h"
#endif
#ifndef OSCL_DLL_H_INCLUDED
#include "oscl_dll.h"
#endif
#ifndef PVMF_VIDEO_H_INCLUDED
#include "pvmf_video.h"
#endif
#ifndef PVMF_DUMMY_FILEINPUT_NODE_FACTORY_H_INCLUDED
#include "pvmf_dummy_fileinput_node_factory.h"
#endif
#include "pvmf_media_cmd.h"
#include "pvmf_media_msg_format_ids.h"

// also defined in the pvmf_fileinput_node.h:
// This test splits each frame into two pieces and sends them in 2 separate messages.
// the first message has no marker bit set, the 2nd message has marker bit set
//#define FRAGMENTATION_TEST

OSCL_DLL_ENTRY_POINT_DEFAULT()

#define GetUnalignedDword( pb, dw ) \
            (dw) = ((uint32) *(pb + 3) << 24) + \
                   ((uint32) *(pb + 2) << 16) + \
                   ((uint16) *(pb + 1) << 8) + *pb;

#define GetUnalignedDwordEx( pb, dw )   GetUnalignedDword( pb, dw ); (pb) += sizeof(uint32);
#define LoadDWORD( dw, p )  GetUnalignedDwordEx( p, dw )

/**
//Macros for calling PVLogger
*/
#define LOGERROR(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iLogger,PVLOGMSG_ERR,m);
#define LOGINFOHI(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG,iLogger,PVLOGMSG_INFO,m);
#define LOGINFOMED(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG,iLogger,PVLOGMSG_INFO,m);
#define LOGINFOLOW(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG,iLogger,PVLOGMSG_INFO,m);
#define LOGINFO(m) LOGINFOMED(m)


#define PVFILEIN_MEDIADATA_POOLNUM 16
#define PVFILEIN_MEDIADATA_CHUNKSIZE 128

//#define SEND_CONFIGINFO_SEPARATELY
const uint32 AMR_FRAME_DELAY = 20; // 20ms


/** Factory class */
OSCL_EXPORT_REF PVMFNodeInterface* PVMFDummyFileInputNodeFactory::CreateDummyFileInputNode(PVMFFileInputSettings* aSettings,
        int32 aPriority)
{
    PVMFDummyFileInputNode* node = NULL;
    int32 err = 0;
    OSCL_TRY(err, node = OSCL_NEW(PVMFDummyFileInputNode, (aSettings, aPriority)););
    OSCL_FIRST_CATCH_ANY(err, return NULL;);
    return node;
}

OSCL_EXPORT_REF bool PVMFDummyFileInputNodeFactory::DeleteDummyFileInputNode(PVMFNodeInterface*& aNode)
{
    if (!aNode)
        return false;
    PVMFDummyFileInputNode* node = (PVMFDummyFileInputNode*)aNode;
    OSCL_DELETE(node);
    aNode = NULL;
    return true;
}

/**
//////////////////////////////////////////////////
// Node Constructor & Destructor
//////////////////////////////////////////////////
*/

OSCL_EXPORT_REF PVMFDummyFileInputNode::PVMFDummyFileInputNode(PVMFFileInputSettings* aSettings, int32 aPriority)
        : OsclTimerObject(aPriority, "PVMFDummyFileInputNode")
        , iMediaBufferMemPool(NULL)
        , iMediaDataAlloc(NULL)
        , iMediaDataMemPool(PVFILEIN_MEDIADATA_POOLNUM, PVFILEIN_MEDIADATA_CHUNKSIZE)
        , iAudioData(NULL)
        , iVideoData(NULL)
        , iLogger(NULL)
        , iFormatSpecificInfoSize(0)
        , iSetFormatSpecificInfo(false)
        , iFileHeaderSize(0)
        , iFsOpen(false)
        , iFileOpen(false)
        , iDataEventCounter(0)
        , iTotalNumFrames(0)
        , iMilliSecondsPerDataEvent(0)
        , iMicroSecondsPerDataEvent(0)
        , iTSForRunIfInactive(0)
        , iTimeStamp(0)
        , iExtensionRefCount(0)
        , iWaitingOnFreeChunk(false)
        , iEndOfFileReached(false)
{
    int32 err;
    OSCL_TRY(err,

             //Create the input command queue.  Use a reserve to avoid lots of
             //dynamic memory allocation.
             iInputCommands.Construct(PVMF_FILEINPUT_NODE_COMMAND_ID_START, PVMF_FILEINPUT_NODE_COMMAND_VECTOR_RESERVE);

             //Create the "current command" queue.  It will only contain one
             //command at a time, so use a reserve of 1.
             iCurrentCommand.Construct(0, 1);

             //Create the port vector.
             iPortVector.Construct(PVMF_FILEINPUT_NODE_PORT_VECTOR_RESERVE);

             //Set the node capability data.
             //This node can support an unlimited number of ports.
             iCapability.iCanSupportMultipleInputPorts = true;
             iCapability.iCanSupportMultipleOutputPorts = true;
             iCapability.iHasMaxNumberOfPorts = false;
             iCapability.iMaxNumberOfPorts = 0;//no maximum
             // chesterc: The node only supports the format of the input file, not
             // all formats that the node could parse.
             iCapability.iOutputFormatCapability.push_back(aSettings->iMediaFormat);

             // construct iSettings
             ConstructInputSettings(aSettings);
            );

    if (err != OsclErrNone)
    {
        //if a leave happened, cleanup and re-throw the error
        iInputCommands.clear();
        iCurrentCommand.clear();
        iPortVector.clear();
        iCapability.iInputFormatCapability.clear();
        iCapability.iOutputFormatCapability.clear();
        OSCL_CLEANUP_BASE_CLASS(PVMFNodeInterface);
        OSCL_CLEANUP_BASE_CLASS(OsclTimerObject);
        OSCL_LEAVE(err);
    }
}

OSCL_EXPORT_REF PVMFDummyFileInputNode::~PVMFDummyFileInputNode()
{
    //thread logoff
    if (IsAdded())
        RemoveFromScheduler();

    //Cleanup allocated interfaces

    //Cleanup allocated ports
    while (!iPortVector.empty())
        iPortVector.Erase(&iPortVector.front());

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

    CloseInputFile();
    iAlloc.deallocate((OsclAny*)iAudioData);
    iAlloc.deallocate((OsclAny*)iVideoData);

    if (iMediaBufferMemPool)
    {
        OSCL_DELETE(iMediaBufferMemPool);
        iMediaBufferMemPool = NULL;
    }

    if (iMediaDataAlloc)
    {
        OSCL_DELETE(iMediaDataAlloc);
        iMediaDataAlloc = NULL;
    }
}

/**
//////////////////////////////////////////////////
// Public Node API implementation
//////////////////////////////////////////////////
*/

/**
//Do thread-specific node creation and go to "Idle" state.
*/
OSCL_EXPORT_REF PVMFStatus PVMFDummyFileInputNode::ThreadLogon()
{
    LOGINFO((0, "FileInputNode:ThreadLogon"));
    switch (iInterfaceState)
    {
        case EPVMFNodeCreated:
            if (!IsAdded())
                AddToScheduler();
            iLogger = PVLogger::GetLoggerObject("PVMFDummyFileInputNode");
            SetState(EPVMFNodeIdle);
            return PVMFSuccess;
            break;
        default:
            return PVMFErrInvalidState;
            break;
    }
}

/**
//Do thread-specific node cleanup and go to "Created" state.
*/
OSCL_EXPORT_REF PVMFStatus PVMFDummyFileInputNode::ThreadLogoff()
{
    LOGINFO((0, "FileInputNode:ThreadLogoff"));
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

/**
//retrieve node capabilities.
*/
OSCL_EXPORT_REF PVMFStatus PVMFDummyFileInputNode::GetCapability(PVMFNodeCapability& aNodeCapability)
{
    LOGINFO((0, "FileInputNode:GetCapability"));
    aNodeCapability = iCapability;
    return PVMFSuccess;
}

/**
//retrive a port iterator.
*/
OSCL_EXPORT_REF PVMFPortIter* PVMFDummyFileInputNode::GetPorts(const PVMFPortFilter* aFilter)
{
    LOGINFO((0, "FileInputNode:GetPorts"));
    OSCL_UNUSED_ARG(aFilter);//port filter is not implemented.
    iPortVector.Reset();
    return &iPortVector;
}

/**
//Queue an asynchronous node command
*/
OSCL_EXPORT_REF PVMFCommandId PVMFDummyFileInputNode::QueryUUID(PVMFSessionId s, const PvmfMimeString& aMimeType,
        Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids,
        bool aExactUuidsOnly,
        const OsclAny* aContext)
{
    LOGINFO((0, "FileInputNode:QueryUUID"));
    PVMFFileInputNodeCommand cmd;
    cmd.PVMFFileInputNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_QUERYUUID, aMimeType, aUuids, aExactUuidsOnly, aContext);
    return QueueCommandL(cmd);
}

/**
//Queue an asynchronous node command
*/
OSCL_EXPORT_REF PVMFCommandId PVMFDummyFileInputNode::QueryInterface(PVMFSessionId s, const PVUuid& aUuid,
        PVInterface*& aInterfacePtr,
        const OsclAny* aContext)
{
    LOGINFO((0, "FileInputNode:QueryInterface"));
    PVMFFileInputNodeCommand cmd;
    cmd.PVMFFileInputNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_QUERYINTERFACE, aUuid, aInterfacePtr, aContext);
    return QueueCommandL(cmd);
}

/**
//Queue an asynchronous node command
*/
OSCL_EXPORT_REF PVMFCommandId PVMFDummyFileInputNode::RequestPort(PVMFSessionId s, int32 aPortTag, const PvmfMimeString* aPortConfig, const OsclAny* aContext)
{
    OSCL_UNUSED_ARG(aPortConfig);
    LOGINFO((0, "FileInputNode:RequestPort"));
    PVMFFileInputNodeCommand cmd;
    cmd.PVMFFileInputNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_REQUESTPORT, aPortTag, aPortConfig, aContext);
    return QueueCommandL(cmd);
}

/**
//Queue an asynchronous node command
*/
OSCL_EXPORT_REF PVMFCommandId PVMFDummyFileInputNode::ReleasePort(PVMFSessionId s, PVMFPortInterface& aPort, const OsclAny* aContext)
{
    LOGINFO((0, "FileInputNode:ReleasePort"));
    PVMFFileInputNodeCommand cmd;
    cmd.PVMFFileInputNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_RELEASEPORT, aPort, aContext);
    return QueueCommandL(cmd);
}

/**
//Queue an asynchronous node command
*/
OSCL_EXPORT_REF PVMFCommandId PVMFDummyFileInputNode::Init(PVMFSessionId s, const OsclAny* aContext)
{
    LOGINFO((0, "FileInputNode:Init"));
    PVMFFileInputNodeCommand cmd;
    cmd.PVMFFileInputNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_INIT, aContext);
    return QueueCommandL(cmd);
}

/**
//Queue an asynchronous node command
*/
OSCL_EXPORT_REF PVMFCommandId PVMFDummyFileInputNode::Prepare(PVMFSessionId s, const OsclAny* aContext)
{
    LOGINFO((0, "FileInputNode:Prepare"));
    PVMFFileInputNodeCommand cmd;
    cmd.PVMFFileInputNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_PREPARE, aContext);
    return QueueCommandL(cmd);
}

/**
//Queue an asynchronous node command
*/
OSCL_EXPORT_REF PVMFCommandId PVMFDummyFileInputNode::Start(PVMFSessionId s, const OsclAny* aContext)
{
    LOGINFO((0, "FileInputNode:Start"));
    PVMFFileInputNodeCommand cmd;
    cmd.PVMFFileInputNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_START, aContext);
    return QueueCommandL(cmd);
}

/**
//Queue an asynchronous node command
*/
OSCL_EXPORT_REF PVMFCommandId PVMFDummyFileInputNode::Stop(PVMFSessionId s, const OsclAny* aContext)
{
    LOGINFO((0, "FileInputNode:Stop"));
    PVMFFileInputNodeCommand cmd;
    cmd.PVMFFileInputNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_STOP, aContext);
    return QueueCommandL(cmd);
}

/**
//Queue an asynchronous node command
*/
OSCL_EXPORT_REF PVMFCommandId PVMFDummyFileInputNode::Flush(PVMFSessionId s, const OsclAny* aContext)
{
    LOGINFO((0, "FileInputNode:Flush"));
    PVMFFileInputNodeCommand cmd;
    cmd.PVMFFileInputNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_FLUSH, aContext);
    return QueueCommandL(cmd);
}

/**
//Queue an asynchronous node command
*/
OSCL_EXPORT_REF PVMFCommandId PVMFDummyFileInputNode::Pause(PVMFSessionId s, const OsclAny* aContext)
{
    LOGINFO((0, "FileInputNode:Pause"));
    PVMFFileInputNodeCommand cmd;
    cmd.PVMFFileInputNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_PAUSE, aContext);
    return QueueCommandL(cmd);
}

/**
//Queue an asynchronous node command
*/
OSCL_EXPORT_REF PVMFCommandId PVMFDummyFileInputNode::Reset(PVMFSessionId s, const OsclAny* aContext)
{
    LOGINFO((0, "FileInputNode:Reset"));
    PVMFFileInputNodeCommand cmd;
    cmd.PVMFFileInputNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_RESET, aContext);
    return QueueCommandL(cmd);
}

/**
//Queue an asynchronous node command
*/
OSCL_EXPORT_REF PVMFCommandId PVMFDummyFileInputNode::CancelAllCommands(PVMFSessionId s, const OsclAny* aContext)
{
    LOGINFO((0, "FileInputNode:CancelAllCommands"));
    PVMFFileInputNodeCommand cmd;
    cmd.PVMFFileInputNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_CANCELALLCOMMANDS, aContext);
    return QueueCommandL(cmd);
}

/**
//Queue an asynchronous node command
*/
OSCL_EXPORT_REF PVMFCommandId PVMFDummyFileInputNode::CancelCommand(PVMFSessionId s, PVMFCommandId aCmdId, const OsclAny* aContext)
{
    LOGINFO((0, "FileInputNode:CancelCommand"));
    PVMFFileInputNodeCommand cmd;
    cmd.PVMFFileInputNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_CANCELCOMMAND, aCmdId, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PVMFDummyFileInputNode::addRef()
{
    ++iExtensionRefCount;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PVMFDummyFileInputNode::removeRef()
{
    --iExtensionRefCount;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF bool PVMFDummyFileInputNode::queryInterface(const PVUuid& uuid, PVInterface*& iface)
{
    if (uuid == KPVMFFileInputNodeExtensionUuid)
    {
        PVMFFileInputNodeExtensionInterface* myInterface = OSCL_STATIC_CAST(PVMFFileInputNodeExtensionInterface*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
        ++iExtensionRefCount;
        return true;
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF bool PVMFDummyFileInputNode::GetBitstreamConfig(OsclRefCounterMemFrag& aConfig)
{
    if (!iFormatSpecificInfo.getMemFragPtr())
        return false;

    aConfig = iFormatSpecificInfo;
    return true;
}


/**
//This routine is called by various command APIs to queue an
//asynchronous command for processing by the command handler AO.
//This function may leave if the command can't be queued due to
//memory allocation failure.
*/
PVMFCommandId PVMFDummyFileInputNode::QueueCommandL(PVMFFileInputNodeCommand& aCmd)
{
    PVMFCommandId id;

    id = iInputCommands.AddL(aCmd);

    //wakeup the AO
    RunIfNotReady();

    return id;
}

/**
/////////////////////////////////////////////////////
// Asynchronous Command processing routines.
// These routines are all called under the AO.
/////////////////////////////////////////////////////
*/

/**
//Called by the command handler AO to process a command from
//the input queue.
//Return true if a command was processed, false if the command
//processor is busy and can't process another command now.
*/
bool PVMFDummyFileInputNode::ProcessCommand(PVMFFileInputNodeCommand& aCmd)
{
    //normally this node will not start processing one command
    //until the prior one is finished.  However, a hi priority
    //command such as Cancel must be able to interrupt a command
    //in progress.
    if (!iCurrentCommand.empty() && !aCmd.hipri())
        return false;

    switch (aCmd.iCmd)
    {
        case PVMF_GENERIC_NODE_QUERYUUID:
            DoQueryUuid(aCmd);
            break;

        case PVMF_GENERIC_NODE_QUERYINTERFACE:
            DoQueryInterface(aCmd);
            break;

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

        case PVMF_GENERIC_NODE_CANCELALLCOMMANDS:
            DoCancelAllCommands(aCmd);
            break;

        case PVMF_GENERIC_NODE_CANCELCOMMAND:
            DoCancelCommand(aCmd);
            break;

        default://unknown command type
            CommandComplete(iInputCommands, aCmd, PVMFFailure);
            break;
    }

    return true;
}

/**
//The various command handlers call this when a command is complete.
*/
void PVMFDummyFileInputNode::CommandComplete(PVMFFileInputNodeCmdQ& aCmdQ, PVMFFileInputNodeCommand& aCmd, PVMFStatus aStatus, OsclAny* aEventData)
{
    LOGINFO((0, "FileInputNode:CommandComplete Id %d Cmd %d Status %d Context %d Data %d"
             , aCmd.iId, aCmd.iCmd, aStatus, aCmd.iContext, aEventData));

    //create response
    PVMFCmdResp resp(aCmd.iId, aCmd.iContext, aStatus, aEventData);
    PVMFSessionId session = aCmd.iSession;

    //Erase the command from the queue.
    aCmdQ.Erase(&aCmd);

    //Report completion to the session observer.
    ReportCmdCompleteEvent(session, resp);
}

/**
//Called by the command handler AO to do the node Reset.
*/
void PVMFDummyFileInputNode::DoReset(PVMFFileInputNodeCommand& aCmd)
{
    //This example node allows a reset from any idle state.
    switch (iInterfaceState)
    {
        case EPVMFNodeCreated:
        case EPVMFNodeIdle:
        case EPVMFNodeInitialized:
        case EPVMFNodePrepared:
        {
            //delete all ports and notify observer.
            while (!iPortVector.empty())
                iPortVector.Erase(&iPortVector.front());

            //restore original port vector reserve.
            iPortVector.Reconstruct();

            // close the input file
            CloseInputFile();

            //logoff & go back to Created state.
            SetState(EPVMFNodeIdle);
            PVMFStatus status = ThreadLogoff();

            CommandComplete(iInputCommands, aCmd, status);
        }
        break;

        default:
            CommandComplete(iInputCommands, aCmd, PVMFErrInvalidState);
            break;
    }
}

/**
//Called by the command handler AO to do the Query UUID
*/
void PVMFDummyFileInputNode::DoQueryUuid(PVMFFileInputNodeCommand& aCmd)
{
    //This node supports Query UUID from any state

    OSCL_String* mimetype;
    Oscl_Vector<PVUuid, OsclMemAllocator> *uuidvec;
    bool exactmatch;
    aCmd.PVMFFileInputNodeCommandBase::Parse(mimetype, uuidvec, exactmatch);

    //Try to match the input mimetype against any of
    //the custom interfaces for this node

    //Match against extension interface...
    if (*mimetype == PVMF_FILEINPUT_NODE_EXTENSION_INTERFACE_MIMETYPE
            //also match against base mimetypes for custom interface1,
            //unless exactmatch is set.
            || (!exactmatch && *mimetype == PVMF_FILEINPUT_NODE_MIMETYPE)
            || (!exactmatch && *mimetype == PVMF_BASEMIMETYPE))
    {

        PVUuid uuid(KPVMFFileInputNodeExtensionUuid);
        uuidvec->push_back(uuid);
    }
    CommandComplete(iInputCommands, aCmd, PVMFSuccess);
}

/**
//Called by the command handler AO to do the Query Interface.
*/
void PVMFDummyFileInputNode::DoQueryInterface(PVMFFileInputNodeCommand& aCmd)
{
    //This node supports Query Interface from any state

    PVUuid* uuid;
    PVInterface** ptr;
    aCmd.PVMFFileInputNodeCommandBase::Parse(uuid, ptr);

    if (*uuid == PVUuid(KPVMFFileInputNodeExtensionUuid))
    {
        queryInterface(*uuid, *ptr);
        CommandComplete(iInputCommands, aCmd, PVMFSuccess);
    }
    else
    {//not supported
        *ptr = NULL;
        CommandComplete(iInputCommands, aCmd, PVMFFailure);
    }
}

/**
//Called by the command handler AO to do the port request
*/
void PVMFDummyFileInputNode::DoRequestPort(PVMFFileInputNodeCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFDummyFileInputNode::DoRequestPort"));

    //This node supports port request from any state

    //this node is limited to one port
    if (iPortVector.size() > 0)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVMFDummyFileInputNode::DoRequestPort: Multiple port request!"));
        CommandComplete(iInputCommands, aCmd, PVMFFailure);
    }

    //retrieve port tag.
    int32 tag;
    OSCL_String* mimetype;
    aCmd.PVMFFileInputNodeCommandBase::Parse(tag, mimetype);

    //(mimetype is not used on this node)

    //validate the tag...
    switch (tag)
    {
        case PVMF_DUMMY_FILEINPUT_NODE_PORT_TYPE_SOURCE:
            break;
        default:
        {
            //bad port tag
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFDummyFileInputNode::DoRequestPort: Error - Invalid port tag"));
            CommandComplete(iInputCommands, aCmd, PVMFFailure);
            return;
        }
        break;
    }

    //Allocate a new port
    OsclAny *ptr = NULL;
    int32 err;
    OSCL_TRY(err, ptr = iPortVector.Allocate(););
    if (err != OsclErrNone || !ptr)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVMFDummyFileInputNode::DoRequestPort: Error - iPortVector Out of memory"));
        CommandComplete(iInputCommands, aCmd, PVMFErrNoMemory);
        return;
    }

    //create base port
    PVMFFileDummyInputPort*port = NULL;
    port = new(ptr) PVMFFileDummyInputPort(tag
                                           , this //the node handles port activity.
                                           , INPORT_CAPACITY, INPORT_RESERVE, INPORT_THRESOLD //no input queue needed.
                                           , OUTPORT_CAPACITY, OUTPORT_RESERVE, OUTPORT_THRESOLD);//output queue has a limit of 10 with no reserve

    //if format was provided in mimestring, set it now.
    if (mimetype)
    {
        PVMFFormatType fmt = mimetype->get_str();
        if (fmt != PVMF_MIME_FORMAT_UNKNOWN
                && port->IsFormatSupported(fmt))
        {
            port->iFormat = fmt;
            port->FormatUpdated();
        }
    }

    //Add the port to the port vector.
    OSCL_TRY(err, iPortVector.AddL(port););
    if (err != OsclErrNone)
    {
        CommandComplete(iInputCommands, aCmd, PVMFErrNoMemory);
        return;
    }

    //Return the port pointer to the caller.
    CommandComplete(iInputCommands, aCmd, PVMFSuccess, (OsclAny*)port);
}

/**
//Called by the command handler AO to do the port release
*/
void PVMFDummyFileInputNode::DoReleasePort(PVMFFileInputNodeCommand& aCmd)
{
    //This node supports release port from any state

    //Find the port in the port vector
    PVMFFileDummyInputPort *port;
    PVMFPortInterface * tempInterface = NULL;

    aCmd.PVMFFileInputNodeCommandBase::Parse(tempInterface);
    port = OSCL_STATIC_CAST(PVMFFileDummyInputPort*, tempInterface);

    PVMFFileDummyInputPort** portPtr = iPortVector.FindByValue(port);
    if (portPtr)
    {
        //delete the port.
        iPortVector.Erase(portPtr);
        CommandComplete(iInputCommands, aCmd, PVMFSuccess);
    }
    else
    {
        //port not found.
        CommandComplete(iInputCommands, aCmd, PVMFFailure);
    }
}

/**
//Called by the command handler AO to do the node Init
*/
void PVMFDummyFileInputNode::DoInit(PVMFFileInputNodeCommand& aCmd)
{
    switch (iInterfaceState)
    {
        case EPVMFNodeIdle:
        {
            //this node doesn't need to do anything
            PVMFStatus status = Initialize();
            if (status == PVMFSuccess)
                SetState(EPVMFNodeInitialized);
            //else
            //	SetState(EPVMFNodeError);

            CommandComplete(iInputCommands, aCmd, status);
        }
        break;

        default:
            CommandComplete(iInputCommands, aCmd, PVMFErrInvalidState);
            break;
    }
}

/**
//Called by the command handler AO to do the node Prepare
*/
void PVMFDummyFileInputNode::DoPrepare(PVMFFileInputNodeCommand& aCmd)
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
//Called by the command handler AO to do the node Start
*/
void PVMFDummyFileInputNode::DoStart(PVMFFileInputNodeCommand& aCmd)
{
    PVMFStatus status = PVMFSuccess;
    switch (iInterfaceState)
    {
        case EPVMFNodePrepared:
            iTimeStamp = 0;
            // Don't break here. Continue to process start command

        case EPVMFNodePaused:
            // If it's a start from stopped or initialized state, iInputFile
            // object will be NULL and output file will be opened and read
            // from the beginning.
            if (!iFileOpen)
            {
                if (iFs.Connect() != 0) return;
                iFsOpen = true;

                if (iInputFile.Open(iSettings.iFileName.get_cstr(), Oscl_File::MODE_READ | Oscl_File::MODE_BINARY, iFs))
                    status = PVMFFailure;

                if (status == PVMFSuccess)
                {
                    iFileOpen = true;
                    if (iInputFile.Seek(iFileHeaderSize + iFormatSpecificInfoSize, Oscl_File::SEEKSET))
                        status = PVMFFailure;
                }
            }

            if (status == PVMFSuccess)
            {
                //transition to Started
                SetState(EPVMFNodeStarted);
            }
            //else
            //	SetState(EPVMFNodeError);

            break;

        default:
            status = PVMFErrInvalidState;
            break;
    }

    CommandComplete(iInputCommands, aCmd, status);
}

/**
//Called by the command handler AO to do the node Stop
*/
void PVMFDummyFileInputNode::DoStop(PVMFFileInputNodeCommand& aCmd)
{
    switch (iInterfaceState)
    {
        case EPVMFNodeStarted:
        case EPVMFNodePaused:

            // Clear queued messages in ports
            uint32 i;
            for (i = 0; i < iPortVector.size(); i++)
                iPortVector[i]->ClearMsgQueues();

            // Close the input file
            CloseInputFile();
            iDataEventCounter = 0;

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
//Called by the command handler AO to do the node Flush
*/
void PVMFDummyFileInputNode::DoFlush(PVMFFileInputNodeCommand& aCmd)
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

            //Notify all ports to suspend their input
            {
                for (uint32 i = 0;i < iPortVector.size();i++)
                    iPortVector[i]->SuspendInput();
            }

            // Close the input file
            CloseInputFile();
            break;

        default:
            CommandComplete(iInputCommands, aCmd, PVMFErrInvalidState);
            break;
    }
}

/**
//A routine to tell if a flush operation is in progress.
*/
bool PVMFDummyFileInputNode::FlushPending()
{
    return (iCurrentCommand.size() > 0
            && iCurrentCommand.front().iCmd == PVMF_GENERIC_NODE_FLUSH);
}


/**
//Called by the command handler AO to do the node Pause
*/
void PVMFDummyFileInputNode::DoPause(PVMFFileInputNodeCommand& aCmd)
{
    switch (iInterfaceState)
    {
        case EPVMFNodeStarted:

            SetState(EPVMFNodePaused);
            CommandComplete(iInputCommands, aCmd, PVMFSuccess);
            break;
        default:
            CommandComplete(iInputCommands, aCmd, PVMFErrInvalidState);
            break;
    }
}

/**
//Called by the command handler AO to do the Cancel All
*/
void PVMFDummyFileInputNode::DoCancelAllCommands(PVMFFileInputNodeCommand& aCmd)
{
    //first cancel the current command if any
    {
        while (!iCurrentCommand.empty())
            CommandComplete(iCurrentCommand, iCurrentCommand[0], PVMFErrCancelled);
    }

    //next cancel all queued commands
    {
        //start at element 1 since this cancel command is element 0.
        while (iInputCommands.size() > 1)
            CommandComplete(iInputCommands, iInputCommands[1], PVMFErrCancelled);
    }

    //finally, report cancel complete.
    CommandComplete(iInputCommands, aCmd, PVMFSuccess);
}

/**
//Called by the command handler AO to do the Cancel single command
*/
void PVMFDummyFileInputNode::DoCancelCommand(PVMFFileInputNodeCommand& aCmd)
{
    //extract the command ID from the parameters.
    PVMFCommandId id;
    aCmd.PVMFFileInputNodeCommandBase::Parse(id);

    //first check "current" command if any
    {
        PVMFFileInputNodeCommand* cmd = iCurrentCommand.FindById(id);
        if (cmd)
        {
            //cancel the queued command
            CommandComplete(iCurrentCommand, *cmd, PVMFErrCancelled);
            //report cancel success
            CommandComplete(iInputCommands, aCmd, PVMFSuccess);
            return;
        }
    }

    //next check input queue.
    {
        //start at element 1 since this cancel command is element 0.
        PVMFFileInputNodeCommand* cmd = iInputCommands.FindById(id, 1);
        if (cmd)
        {
            //cancel the queued command
            CommandComplete(iInputCommands, *cmd, PVMFErrCancelled);
            //report cancel success
            CommandComplete(iInputCommands, aCmd, PVMFSuccess);
            return;
        }
    }
    //if we get here the command isn't queued so the cancel fails.
    CommandComplete(iInputCommands, aCmd, PVMFFailure);
}

/////////////////////////////////////////////////////
// Event reporting routines.
/////////////////////////////////////////////////////
void PVMFDummyFileInputNode::SetState(TPVMFNodeInterfaceState s)
{
    LOGINFO((0, "FileInputNode:SetState %d", s));
    PVMFNodeInterface::SetState(s);
}

void PVMFDummyFileInputNode::ReportErrorEvent(PVMFEventType aEventType, OsclAny* aEventData)
{
    LOGINFO((0, "FileInputNode:NodeErrorEvent Type %d Data %d"
             , aEventType, aEventData));

    PVMFNodeInterface::ReportErrorEvent(aEventType, aEventData);
}

void PVMFDummyFileInputNode::ReportInfoEvent(PVMFEventType aEventType, OsclAny* aEventData)
{
    LOGINFO((0, "FileInputNode:NodeInfoEvent Type %d Data %d"
             , aEventType, aEventData));

    PVMFNodeInterface::ReportInfoEvent(aEventType, aEventData);
}

/////////////////////////////////////////////////////
// Port Processing routines
/////////////////////////////////////////////////////

void PVMFDummyFileInputNode::HandlePortActivity(const PVMFPortActivity &aActivity)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "0x%x PVMFDummyFileInputNode::PortActivity: port=0x%x, type=%d",
                     this, aActivity.iPort, aActivity.iType));

    //A port is reporting some activity or state change.  This code
    //figures out whether we need to queue a processing event
    //for the AO, and/or report a node event to the observer.

    //int32 err = 0;
    switch (aActivity.iType)
    {
        case PVMF_PORT_ACTIVITY_CREATED:
            //Report port created info event to the node.
            PVMFNodeInterface::ReportInfoEvent(PVMFInfoPortCreated
                                               , (OsclAny*)aActivity.iPort);
            break;

        case PVMF_PORT_ACTIVITY_DELETED:
            //Report port deleted info event to the node.
            PVMFNodeInterface::ReportInfoEvent(PVMFInfoPortDeleted
                                               , (OsclAny*)aActivity.iPort);
            break;

        case PVMF_PORT_ACTIVITY_CONNECT:
            break;

        case PVMF_PORT_ACTIVITY_DISCONNECT:
            break;

        case PVMF_PORT_ACTIVITY_OUTGOING_MSG:
            // chesterc: No RunIfNotReady here. It will be called in Run after SendNewMediaMessage.
            // Calling RunIfNotReady here would screw up frame rate simulation
            break;

        case PVMF_PORT_ACTIVITY_INCOMING_MSG:
            break;

        case PVMF_PORT_ACTIVITY_OUTGOING_QUEUE_BUSY:
            break;

        case PVMF_PORT_ACTIVITY_OUTGOING_QUEUE_READY:
            //wakeup to resume queueing outgoing messages.
            RunIfNotReady();
            break;

        case PVMF_PORT_ACTIVITY_CONNECTED_PORT_BUSY:
            break;

        case PVMF_PORT_ACTIVITY_CONNECTED_PORT_READY:
            //wakeup to resume sending outgoing data.
            RunIfNotReady();
            break;

        default:
            break;
    }
}


/**
/////////////////////////////////////////////////////
// Active object implementation
/////////////////////////////////////////////////////
*/

/**
//Example of a single-AO node implementation.  This AO handles
//both API commands and port activity.
//
//The AO will either process one command, send one outgoing message,
//or create one new outgoing message per call.
//It will re-schedule itself and run continuously
//until it runs out of things to do.
*/
void PVMFDummyFileInputNode::Run()
{
    /*
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
    	(0, "0x%x PVMFDummyFileInputNode::Run: ENTER - iInputCommands.size=%d, iPortActivityQueue.size=%d",
    	this, iInputCommands.size(), iPortActivityQueue.size()));
    */

    //Process commands.
    if (!iInputCommands.empty())
    {
        if (ProcessCommand(iInputCommands.front()))
        {
            //note:it's possible the node could be reset so
            //check isAdded here to avoid scheduler panic.
            if (IsAdded())
                RunIfNotReady();
            return;
        }
    }

    //Process ports
    if (iPortVector.size() > 0)
    {
        //Send output data.
        if (iInterfaceState == EPVMFNodeStarted || FlushPending())
        {
            if (iPortVector[0]->OutgoingMsgQueueSize() > 0
                    && !iPortVector[0]->IsConnectedPortBusy())
            {
                PVMFStatus status = iPortVector[0]->Send();
                if (status == PVMFSuccess)
                {
                    //increment timestamp and frame counter
                    IncTimestamp();
                    ++iDataEventCounter;
                    RunIfNotReady();
                    return;
                }
                else if (status == PVMFErrBusy)
                {
                    return;//wait on port ready callback
                }
            }
            while (iPortVector[0]->IncomingMsgQueueSize() > 0)
            {
                // Just purge this data so that we
                // don't have a memory lockup
                PVMFSharedMediaMsgPtr aMsg;
                iPortVector[0]->DequeueIncomingMsg(aMsg);
            }
        }

        // Create new data and send to the output queue.
        // chesterc: In flush state, don't queue new data
        if (iInterfaceState == EPVMFNodeStarted && !FlushPending())
        {
            if (!iPortVector[0]->IsOutgoingQueueBusy())
            {
                PVMFStatus status = SendNewMediaMessage();
                if (status == PVMFSuccess)
                {
                    //Re-schedule if not end of file.
                    RunIfNotReady(iTSForRunIfInactive);
                    return;
                }
            }
        }

        //Monitor completion of flush command.
        if (FlushPending())
        {
            if (iPortVector[0]->IncomingMsgQueueSize() > 0
                    || iPortVector[0]->OutgoingMsgQueueSize() > 0)
            {
                RunIfNotReady();
                return;//wait on queues to empty.
            }

            //Flush is complete.  Go to prepared state.
            SetState(EPVMFNodePrepared);

            //resume port input so the ports can be re-started.
            iPortVector[0]->ResumeInput();

            CommandComplete(iCurrentCommand, iCurrentCommand.front(), PVMFSuccess);
            RunIfNotReady();
        }
    }
}

PVMFStatus PVMFDummyFileInputNode::SendNewMediaMessage()
{
#ifdef FRAGMENTATION_TEST
    PVMFSharedMediaMsgPtr mediaMsg;
    PVMFSharedMediaMsgPtr mediaMsg2;
    PVMFStatus status = GenerateMediaMessage2(mediaMsg, mediaMsg2);
#else
    PVMFSharedMediaMsgPtr mediaMsg;
    PVMFStatus status = GenerateMediaMessage(mediaMsg);
#endif

    if (status == PVMFSuccess)
    {
        status = iPortVector[0]->QueueOutgoingMsg(mediaMsg);

        if (status == PVMFSuccess)
        {
            //keep count of the number of source
            //frames generated on this port.
            iPortVector[0]->iNumFramesGenerated++;
        }
        else
        {
            //just discard the data...
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "PVMFDummyFileInputNode::SendNewMediaMessage: Data lost!. "));
        }
#ifdef FRAGMENTATION_TEST
        if (!iEndOfFileReached)
        {
            // 2nd message exists unless EOF
            status = iPortVector[0]->QueueOutgoingMsg(mediaMsg2);

            if (status == PVMFSuccess)
            {
                //keep count of the number of source
                //frames generated on this port.
                iPortVector[0]->iNumFramesGenerated++;
            }
            else
            {
                //just discard the data...
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "PVMFDummyFileInputNode::SendNewMediaMessage: Data lost!. "));
            }
        }
#endif

    }
    else if (iEndOfFileReached)
    {
        // end of file
    }
    else if (status != PVMFErrNoMemory) // excluding the case that memory pool runs out of memory
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVMFDummyFileInputNode::SendNewMediaMessage: Error - GenerateData failed"));
        PVMFNodeInterface* node = OSCL_STATIC_CAST(PVMFNodeInterface*, this);
        ReportErrorEvent(PVMF_FILEINPUT_NODE_ERROR_DATA_GENERATING_ERROR, (OsclAny*)node);
    }

    return status;
}

void PVMFDummyFileInputNode::DoCancel()
{//the base class cancel operation is sufficient.
    OsclTimerObject::DoCancel();
}


////////////////////////////////////////////////////////////////////////////
void PVMFDummyFileInputNode::ConstructInputSettings(PVMFFileInputSettings* aSettings)
{
    iSettings.iFileName = aSettings->iFileName;
    iSettings.iFrameHeight = aSettings->iFrameHeight;
    iSettings.iFrameRate = aSettings->iFrameRate;
    iSettings.iFrameWidth = aSettings->iFrameWidth;
    iSettings.iLoopInputFile = aSettings->iLoopInputFile;
    iSettings.iMediaFormat = aSettings->iMediaFormat;
    iSettings.iNumChannels = aSettings->iNumChannels;
    iSettings.iSamplingFrequency = aSettings->iSamplingFrequency;
    iSettings.iTimescale = aSettings->iTimescale;
    iSettings.iNum20msFramesPerChunk = aSettings->iNum20msFramesPerChunk;
    iSettings.iFrameRateSimulation = aSettings->iFrameRateSimulation;
    iSettings.iFirstFrameTimestamp = aSettings->iFirstFrameTimestamp;
    iSettings.iBitrate = aSettings->iBitrate;
}

////////////////////////////////////////////////////////////////////////////
void PVMFDummyFileInputNode::CloseInputFile()
{
    if (iFileOpen)
    {
        iInputFile.Close();
        iFileOpen = false;
    }

    if (iFsOpen)
    {
        iFs.Close();
        iFsOpen = false;
    }
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFDummyFileInputNode::Initialize()
{
    if (iFs.Connect() != 0) return 0;

    iFsOpen = true;

    if (0 != iInputFile.Open(iSettings.iFileName.get_cstr(), Oscl_File::MODE_READ | Oscl_File::MODE_BINARY, iFs))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVMFDummyFileInputNode::HandleCmdInit: Error - iInputFile.Open failed"));
        return PVMFFailure;
    }


    iFileOpen = true;
    uint32 maxFrameSize = 0;

    if (iSettings.iMediaFormat == PVMF_MIME_H2632000)
    {
        int32 fileStart = iInputFile.Tell();
        iInputFile.Seek(0, Oscl_File::SEEKEND);
        int32 fileEnd = iInputFile.Tell();
        iInputFile.Seek(fileStart, Oscl_File::SEEKSET);
        int32 fileSize = fileEnd - fileStart;

        // Validate settings
        if (iSettings.iFrameHeight <= 0 || iSettings.iFrameWidth <= 0 ||
                iSettings.iFrameRate <= 0 || iSettings.iTimescale <= 0)
        {
            CloseInputFile();
            return PVMFErrArgument;
        }

        fileStart = iInputFile.Tell();
        iInputFile.Seek(0, Oscl_File::SEEKEND);
        fileEnd = iInputFile.Tell();
        iInputFile.Seek(fileStart, Oscl_File::SEEKSET);
        fileSize = fileEnd - fileStart;
        iVideoData = (uint8*)iAlloc.allocate(fileSize);
        if (!iVideoData)
        {
            CloseInputFile();
            return PVMFErrNoMemory;
        }

        // Read the whole file to data buffer then go back to front
        iInputFile.Read((OsclAny*)iVideoData, sizeof(uint8), fileSize);
        iInputFile.Seek(fileStart, Oscl_File::SEEKSET);
        iMilliSecondsPerDataEvent = (int32)(1000 / iSettings.iFrameRate);
        iMicroSecondsPerDataEvent = iMilliSecondsPerDataEvent * 1000;

    }

    else if (iSettings.iMediaFormat == PVMF_MIME_AMR_IF2)
    {
        int32 fileStart, fileEnd, fileSize;
        fileStart = iInputFile.Tell();
        iInputFile.Seek(0, Oscl_File::SEEKEND);
        fileEnd = iInputFile.Tell();
        iInputFile.Seek(fileStart, Oscl_File::SEEKSET);
        fileSize = fileEnd - fileStart;
        iAudioData = (uint8*)iAlloc.allocate(fileSize);
        if (!iAudioData)
        {
            CloseInputFile();
            return PVMFErrNoMemory;
        }

        // Read the whole file to data buffer then go back to front
        iInputFile.Read((OsclAny*)iAudioData, sizeof(uint8), fileSize);
        iInputFile.Seek(fileStart, Oscl_File::SEEKSET);

    }
    else
        CloseInputFile();
    return PVMFErrArgument;

    iDataEventCounter = 0;
    CloseInputFile();

    // Create memory pool for the media data, using the maximum frame size found earlier
    int32 err = 0;
    OSCL_TRY(err,
             if (iMediaBufferMemPool)
{
    OSCL_DELETE(iMediaBufferMemPool);
        iMediaBufferMemPool = NULL;
    }
    iMediaBufferMemPool = OSCL_NEW(OsclMemPoolFixedChunkAllocator,
                                   (PVFILEIN_MEDIADATA_POOLNUM));
    if (!iMediaBufferMemPool)
    OSCL_LEAVE(OsclErrNoMemory);

    if (iMediaDataAlloc)
    {
        OSCL_DELETE(iMediaDataAlloc);
            iMediaDataAlloc = NULL;
        }
    iMediaDataAlloc = OSCL_NEW(PVMFSimpleMediaBufferCombinedAlloc, (iMediaBufferMemPool));
    if (!iMediaDataAlloc)
    OSCL_LEAVE(OsclErrNoMemory);

    // The first allocate call will set the chunk size of the memory pool. Use the max
    // frame size calculated earlier to set the chunk size.  The allocated data will be
    // deallocated automatically as tmpPtr goes out of scope.
    OsclSharedPtr<PVMFMediaDataImpl> tmpPtr = iMediaDataAlloc->allocate(maxFrameSize);
            );
    if (err != OsclErrNone)
        return PVMFFailure;

    return PVMFSuccess;
}

void PVMFDummyFileInputNode::IncTimestamp()
{
    //Increment running timestamp if needed.
    if (iSettings.iMediaFormat == PVMF_MIME_AMR_IF2)
    {
        iTimeStamp += iMilliSecondsPerDataEvent;
    }
}

void PVMFDummyFileInputNode::freechunkavailable(OsclAny* aContextData)
{
    OSCL_UNUSED_ARG(aContextData);
    if (iWaitingOnFreeChunk)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVMFDummyFileInputNode::freechunkavailable: Resuming processing, free chunk available."));
        iWaitingOnFreeChunk = false;
        if (IsAdded())
        {
            RunIfNotReady();
        }
    }
}


PVMFStatus PVMFDummyFileInputNode::GenerateMediaMessage(PVMFSharedMediaMsgPtr& aMediaMsg)
{

    if (iInterfaceState != EPVMFNodeStarted)
    {
        return PVMFSuccess;
    }

    uint32 bytesToRead = 0;
    uint32 timeStamp = 0;

    //Find the frame...
    if (!((iSettings.iMediaFormat == PVMF_MIME_H2632000) || (iSettings.iMediaFormat == PVMF_MIME_AMR_IF2)))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVMFDummyFileInputNode::HandleEventPortActivity: Error - Unsupported media format"));
        return PVMFFailure;
    }

    if (timeStamp < iSettings.iFirstFrameTimestamp)
    {
        // Search for the first frame greater than the specified start time
        iTSForRunIfInactive = 0;
        return PVMFSuccess;
    }

    // Create new media data buffer

    PVMFSharedMediaDataPtr mediaData;
    int32 err = 0;
    OSCL_TRY(err,
             OsclSharedPtr<PVMFMediaDataImpl> mediaDataImpl = iMediaDataAlloc->allocate(bytesToRead);
             mediaData = PVMFMediaData::createMediaData(mediaDataImpl, &iMediaDataMemPool);
            );
    if (err != OsclErrNone)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVMFDummyFileInputNode::HandleEventPortActivity: Mempool empty-- calling notifyfreechunkavailable"));
        //register to receive notice when memory is available.
        iMediaBufferMemPool->notifyfreechunkavailable(*this);
        iWaitingOnFreeChunk = true;
        return PVMFErrNoMemory;
    }

    // Retrieve memory fragment to write to

    OsclRefCounterMemFrag refCtrMemFrag;
    mediaData->getMediaFragment(0, refCtrMemFrag);
    if (refCtrMemFrag.getCapacity() < bytesToRead)
        return PVMFFailure;


    // For AAC Latm, read the marker info before reading the data

    // Read the frame from file

    //uint32 len = iInputFile.Read(refCtrMemFrag.getMemFragPtr(), sizeof(uint8), bytesToRead );
    if (iSettings.iMediaFormat == PVMF_MIME_H2632000)
    {
        oscl_memcpy(refCtrMemFrag.getMemFragPtr(), iVideoData, sizeof(iVideoData));
        // update the filled length of the fragment
        mediaData->setMediaFragFilledLen(0, sizeof(iVideoData));
    }
    else
    {
        oscl_memcpy(refCtrMemFrag.getMemFragPtr(), iAudioData, sizeof(iAudioData));
        // update the filled length of the fragment
        mediaData->setMediaFragFilledLen(0, sizeof(iAudioData));
    }

    // Set timestamp
    mediaData->setTimestamp(timeStamp);
    // Set sequence number
    mediaData->setSeqNum(iDataEventCounter);

    if (iSettings.iMediaFormat == PVMF_MIME_H2632000)

    {
        // For M4V and H263 data always set the marker bit to 1
        OsclSharedPtr<PVMFMediaDataImpl> media_data_impl;
        mediaData->getMediaDataImpl(media_data_impl);
        media_data_impl->setMarkerInfo(1);
    }


    // Set format specific info in media data message
#ifndef SEND_CONFIGINFO_SEPARATELY
    mediaData->setFormatSpecificInfo(iFormatSpecificInfo);
#else
    if (iSetFormatSpecificInfo)
    {
        // Create new media data buffer
        OsclSharedPtr<PVMFMediaDataImpl> emptyImpl = iMediaDataAlloc.allocate(0);
        PVMFSharedMediaDataPtr volData;
        int errcode = 0;
        OSCL_TRY(errcode, volData = PVMFMediaData::createMediaData(emptyImpl, &iMediaDataMemPool));
        OSCL_FIRST_CATCH_ANY(errcode, return PVMFErrNoMemory);

        // Set format specific info in media data message
        volData->setFormatSpecificInfo(iFormatSpecificInfo);

        // Send VOL header to downstream node
        PVMFSharedMediaMsgPtr volMsg;
        convertToPVMFMediaMsg(volMsg, volData);
        iSetFormatSpecificInfo = false;

        ++iDataEventCounter;
        return PVMFSuccess;
    }
#endif

    // Send frame to downstream node

    convertToPVMFMediaMsg(aMediaMsg, mediaData);

    // Queue the next data event
    if (iSettings.iFrameRateSimulation)
        iTSForRunIfInactive = iMicroSecondsPerDataEvent;
    else
        iTSForRunIfInactive = 0;

    return PVMFSuccess;
}


int32 PVMFDummyFileInputNode::GetIF2FrameSize(uint8 aFrameType)
{
    // Last 4 bits of first byte of the package is frame type
    uint8 frameType = (uint8)(0x0f & aFrameType);

    // Find frame size for each frame type
    switch (frameType)
    {
        case 0: // AMR 4.75 Kbps
            return 13;
        case 1: // AMR 5.15 Kbps
            return 14;
        case 2: // AMR 5.90 Kbps
            return 16;
        case 3: // AMR 6.70 Kbps
            return 18;
        case 4: // AMR 7.40 Kbps
            return 19;
        case 5: // AMR 7.95 Kbps
            return 21;
        case 6: // AMR 10.2 Kbps
            return 26;
        case 7: // AMR 12.2 Kbps
            return 31;
        case 8: // AMR Frame SID
        case 9: // AMR Frame GSM EFR SID
        case 10: // AMR Frame TDMA EFR SID
        case 11: // AMR Frame PDC EFR SID
            return 6;
        case 15: // AMR Frame No Data
            return 1;
        default: // Error - For Future Use
            return -1;
    }
}

int32 PVMFDummyFileInputNode::LocateH263FrameHeader(uint8 *video_buffer, int32 vop_size)
{
    int32 idx;
    uint8 *ptr;

    idx = 1;
    ptr = video_buffer + 1;
    vop_size -= 4;
    do
    {
        ptr--;
        idx--;
        for (;;)
        {
            if (ptr[1])
            {
                ptr += 2;
                idx += 2;
            }
            else if (ptr[0])
            {
                ptr++;
                idx++;
            }
            else
            {
                break;
            }
            if (idx >= vop_size) return -1;
        }
        ptr += 2;
        idx += 2;
    }
    while ((video_buffer[idx] & 0xfc) != 0x80);

    return idx -2;
}
