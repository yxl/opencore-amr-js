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
#include "pvmf_omx_basedec_node.h"
#include "pvlogger.h"
#include "oscl_error_codes.h"
#include "pvmf_omx_basedec_port.h"
#include "pv_mime_string_utils.h"
#include "oscl_snprintf.h"
#include "pvmf_media_cmd.h"
#include "pvmf_media_msg_format_ids.h"
#include "pvmi_kvp_util.h"

#include "OMX_Core.h"
#include "pvmf_omx_basedec_callbacks.h"     //used for thin AO in Decoder's callbacks
#include "pv_omxcore.h"
#include "pv_omx_config_parser.h"
#define CONFIG_SIZE_AND_VERSION(param) \
	    param.nSize=sizeof(param); \
	    param.nVersion.s.nVersionMajor = SPECVERSIONMAJOR; \
	    param.nVersion.s.nVersionMinor = SPECVERSIONMINOR; \
	    param.nVersion.s.nRevision = SPECREVISION; \
	    param.nVersion.s.nStep = SPECSTEP;

#define PVOMXBASEDEC_MEDIADATA_CHUNKSIZE 128


// OMX CALLBACKS
// 1) AO OMX component running in the same thread as the OMX node
//	In this case, the callbacks can be called directly from the component
//	The callback: OMX Component->CallbackEventHandler->EventHandlerProcessing
//	The callback can perform do RunIfNotReady

// 2) Multithreaded component
//	In this case, the callback is made using the threadsafe callback (TSCB) AO
//	Component thread : OMX Component->CallbackEventHandler->TSCB(ReceiveEvent) => event is queued
//  Node thread		 : dequeue event => TSCB(ProcessEvent)->ProcessCallbackEventHandler->EventHandlerProcessing



// callback for Event Handler - in multithreaded case, event is queued to be processed later
//	in AO case, event is processed immediately by calling EventHandlerProcessing
OMX_ERRORTYPE CallbackEventHandler(OMX_OUT OMX_HANDLETYPE aComponent,
                                   OMX_OUT OMX_PTR aAppData,
                                   OMX_OUT OMX_EVENTTYPE aEvent,
                                   OMX_OUT OMX_U32 aData1,
                                   OMX_OUT OMX_U32 aData2,
                                   OMX_OUT OMX_PTR aEventData)
{

    PVMFOMXBaseDecNode *Node = (PVMFOMXBaseDecNode *) aAppData;

    if (Node->IsComponentMultiThreaded())
    {
        // allocate the memory for the callback event specific data
        //EventHandlerSpecificData* ED = (EventHandlerSpecificData*) oscl_malloc(sizeof (EventHandlerSpecificData));
        EventHandlerSpecificData* ED = (EventHandlerSpecificData*) Node->iThreadSafeHandlerEventHandler->iMemoryPool->allocate(sizeof(EventHandlerSpecificData));

        // pack the relevant data into the structure
        ED->hComponent = aComponent;
        ED->pAppData = aAppData;
        ED->eEvent = aEvent;
        ED->nData1 = aData1;
        ED->nData2 = aData2;
        ED->pEventData = aEventData;

        // convert the pointer into OsclAny ptr
        OsclAny* P = (OsclAny*) ED;


        // CALL the generic callback AO API:
        Node->iThreadSafeHandlerEventHandler->ReceiveEvent(P);

        return OMX_ErrorNone;
    }
    else
    {

        OMX_ERRORTYPE status;
        status = Node->EventHandlerProcessing(aComponent, aAppData, aEvent, aData1, aData2, aEventData);
        return status;
    }

}

// callback for EmptyBufferDone - in multithreaded case, event is queued to be processed later
//	in AO case, event is processed immediately by calling EmptyBufferDoneProcessing
OMX_ERRORTYPE CallbackEmptyBufferDone(OMX_OUT OMX_HANDLETYPE aComponent,
                                      OMX_OUT OMX_PTR aAppData,
                                      OMX_OUT OMX_BUFFERHEADERTYPE* aBuffer)
{

    PVMFOMXBaseDecNode *Node = (PVMFOMXBaseDecNode *) aAppData;
    if (Node->IsComponentMultiThreaded())
    {

        // allocate the memory for the callback event specific data
        //EmptyBufferDoneSpecificData* ED = (EmptyBufferDoneSpecificData*) oscl_malloc(sizeof (EmptyBufferDoneSpecificData));
        EmptyBufferDoneSpecificData* ED = (EmptyBufferDoneSpecificData*) Node->iThreadSafeHandlerEmptyBufferDone->iMemoryPool->allocate(sizeof(EmptyBufferDoneSpecificData));

        // pack the relevant data into the structure
        ED->hComponent = aComponent;
        ED->pAppData = aAppData;
        ED->pBuffer = aBuffer;

        // convert the pointer into OsclAny ptr
        OsclAny* P = (OsclAny*) ED;

        // CALL the generic callback AO API:
        Node->iThreadSafeHandlerEmptyBufferDone->ReceiveEvent(P);

        return OMX_ErrorNone;
    }
    else
    {
        OMX_ERRORTYPE status;
        status = Node->EmptyBufferDoneProcessing(aComponent, aAppData, aBuffer);
        return status;
    }

}

// callback for FillBufferDone - in multithreaded case, event is queued to be processed later
//	in AO case, event is processed immediately by calling FillBufferDoneProcessing
OMX_ERRORTYPE CallbackFillBufferDone(OMX_OUT OMX_HANDLETYPE aComponent,
                                     OMX_OUT OMX_PTR aAppData,
                                     OMX_OUT OMX_BUFFERHEADERTYPE* aBuffer)
{
    PVMFOMXBaseDecNode *Node = (PVMFOMXBaseDecNode *) aAppData;
    if (Node->IsComponentMultiThreaded())
    {

        // allocate the memory for the callback event specific data
        //FillBufferDoneSpecificData* ED = (FillBufferDoneSpecificData*) oscl_malloc(sizeof (FillBufferDoneSpecificData));
        FillBufferDoneSpecificData* ED = (FillBufferDoneSpecificData*) Node->iThreadSafeHandlerFillBufferDone->iMemoryPool->allocate(sizeof(FillBufferDoneSpecificData));

        // pack the relevant data into the structure
        ED->hComponent = aComponent;
        ED->pAppData = aAppData;
        ED->pBuffer = aBuffer;

        // convert the pointer into OsclAny ptr
        OsclAny* P = (OsclAny*) ED;

        // CALL the generic callback AO API:
        Node->iThreadSafeHandlerFillBufferDone->ReceiveEvent(P);

        return OMX_ErrorNone;
    }
    else
    {
        OMX_ERRORTYPE status;
        status = Node->FillBufferDoneProcessing(aComponent, aAppData, aBuffer);
        return status;
    }

}

// Callback processing in multithreaded case - dequeued event - call EventHandlerProcessing
OSCL_EXPORT_REF OsclReturnCode PVMFOMXBaseDecNode::ProcessCallbackEventHandler_MultiThreaded(OsclAny* P)
{

    // re-cast the pointer

    EventHandlerSpecificData* ED = (EventHandlerSpecificData*) P;

    OMX_HANDLETYPE aComponent = ED->hComponent;
    OMX_PTR aAppData = ED->pAppData;
    OMX_EVENTTYPE aEvent = ED->eEvent;
    OMX_U32 aData1 = ED->nData1;
    OMX_U32 aData2 = ED->nData2;
    OMX_PTR aEventData = ED->pEventData;


    EventHandlerProcessing(aComponent, aAppData, aEvent, aData1, aData2, aEventData);


    // release the allocated memory when no longer needed

    iThreadSafeHandlerEventHandler->iMemoryPool->deallocate(ED);
    ED = NULL;

    return OsclSuccess;
}



// Callback processing in multithreaded case - dequeued event - call EmptyBufferDoneProcessing
OSCL_EXPORT_REF OsclReturnCode PVMFOMXBaseDecNode::ProcessCallbackEmptyBufferDone_MultiThreaded(OsclAny* P)
{


    // re-cast the pointer
    EmptyBufferDoneSpecificData* ED = (EmptyBufferDoneSpecificData*) P;

    OMX_HANDLETYPE aComponent = ED->hComponent;
    OMX_PTR aAppData = ED->pAppData;
    OMX_BUFFERHEADERTYPE* aBuffer = ED->pBuffer;

    EmptyBufferDoneProcessing(aComponent, aAppData, aBuffer);

    // release the allocated memory when no longer needed

    iThreadSafeHandlerEmptyBufferDone->iMemoryPool->deallocate(ED);
    ED = NULL;

    return OsclSuccess;
}


// Callback processing in multithreaded case - dequeued event - call FillBufferDoneProcessing
OSCL_EXPORT_REF OsclReturnCode PVMFOMXBaseDecNode::ProcessCallbackFillBufferDone_MultiThreaded(OsclAny* P)
{

    // re-cast the pointer
    FillBufferDoneSpecificData* ED = (FillBufferDoneSpecificData*) P;

    OMX_HANDLETYPE aComponent = ED->hComponent;
    OMX_PTR aAppData = ED->pAppData;
    OMX_BUFFERHEADERTYPE* aBuffer = ED->pBuffer;


    FillBufferDoneProcessing(aComponent, aAppData, aBuffer);


    // release the allocated memory when no longer needed

    iThreadSafeHandlerFillBufferDone->iMemoryPool->deallocate(ED);
    ED = NULL;

    return OsclSuccess;
}

/////////////////////////////////////////////////////////////////////////////
// Class Destructor
/////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFOMXBaseDecNode::~PVMFOMXBaseDecNode()
{
    LogDiagnostics();

    //Clearup decoder
    DeleteOMXBaseDecoder();

    // Cleanup callback AOs and Mempools
    if (iThreadSafeHandlerEventHandler)
    {
        OSCL_DELETE(iThreadSafeHandlerEventHandler);
        iThreadSafeHandlerEventHandler = NULL;
    }
    if (iThreadSafeHandlerEmptyBufferDone)
    {
        OSCL_DELETE(iThreadSafeHandlerEmptyBufferDone);
        iThreadSafeHandlerEmptyBufferDone = NULL;
    }
    if (iThreadSafeHandlerFillBufferDone)
    {
        OSCL_DELETE(iThreadSafeHandlerFillBufferDone);
        iThreadSafeHandlerFillBufferDone = NULL;
    }

    if (iMediaDataMemPool)
    {
        iMediaDataMemPool->removeRef();
        iMediaDataMemPool = NULL;
    }

    if (iOutBufMemoryPool)
    {
        iOutBufMemoryPool->removeRef();
        iOutBufMemoryPool = NULL;
    }
    if (iInBufMemoryPool)
    {
        iInBufMemoryPool->removeRef();
        iInBufMemoryPool = NULL;
    }

    //Thread logoff
    if (IsAdded())
    {
        RemoveFromScheduler();
        iIsAdded = false;
    }

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

    //Release Input buffer
    iDataIn.Unbind();
}

/////////////////////////////////////////////////////////////////////////////
// Remove AO from the scheduler
/////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMFOMXBaseDecNode::ThreadLogoff()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "%s:ThreadLogoff", iName.Str()));

    switch (iInterfaceState)
    {
        case EPVMFNodeIdle:
            if (IsAdded())
            {
                RemoveFromScheduler();
                iIsAdded = false;
            }
            iLogger = NULL;
            SetState(EPVMFNodeCreated);
            return PVMFSuccess;
            // break;	This break statement was removed to avoid compiler warning for Unreachable Code


        default:
            return PVMFErrInvalidState;
            // break;	This break statement was removed to avoid compiler warning for Unreachable Code

    }
}

/////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMFOMXBaseDecNode::GetCapability(PVMFNodeCapability& aNodeCapability)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "%s::GetCapability() called", iName.Str()));

    aNodeCapability = iCapability;
    return PVMFSuccess;
}

/////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFPortIter* PVMFOMXBaseDecNode::GetPorts(const PVMFPortFilter* aFilter)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "%s::GetPorts() called", iName.Str()));

    OSCL_UNUSED_ARG(aFilter);

    return NULL;
}

/////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMFOMXBaseDecNode::QueueCommandL(PVMFOMXBaseDecNodeCommand& aCmd)
{
    PVMFCommandId id;

    id = iInputCommands.AddL(aCmd);

    if (iInputCommands.size() == 1)
    {
        //wakeup the AO all the rest of input commands will reschedule the AO in Run
        RunIfNotReady();
    }
    return id;
}

/////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMFOMXBaseDecNode::QueryUUID(PVMFSessionId s, const PvmfMimeString& aMimeType,
        Oscl_Vector<PVUuid, PVMFOMXBaseDecNodeAllocator>& aUuids,
        bool aExactUuidsOnly,
        const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "%s::QueryUUID() called", iName.Str()));
    PVMFOMXBaseDecNodeCommand cmd;
    cmd.PVMFOMXBaseDecNodeCommandBase::Construct(s, PVMFOMXBaseDecNodeCommand::PVOMXBASEDEC_NODE_CMD_QUERYUUID, aMimeType, aUuids, aExactUuidsOnly, aContext);
    return QueueCommandL(cmd);
}

/////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMFOMXBaseDecNode::QueryInterface(PVMFSessionId s, const PVUuid& aUuid,
        PVInterface*& aInterfacePtr,
        const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "%s::QueryInterface() called", iName.Str()));
    PVMFOMXBaseDecNodeCommand cmd;
    cmd.PVMFOMXBaseDecNodeCommandBase::Construct(s, PVMFOMXBaseDecNodeCommand::PVOMXBASEDEC_NODE_CMD_QUERYINTERFACE, aUuid, aInterfacePtr, aContext);
    return QueueCommandL(cmd);
}

/////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMFOMXBaseDecNode::RequestPort(PVMFSessionId s, int32 aPortTag, const PvmfMimeString* /* aPortConfig */, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "%s::RequestPort() called", iName.Str()));
    PVMFOMXBaseDecNodeCommand cmd;
    cmd.PVMFOMXBaseDecNodeCommandBase::Construct(s, PVMFOMXBaseDecNodeCommand::PVOMXBASEDEC_NODE_CMD_REQUESTPORT, aPortTag, aContext);
    return QueueCommandL(cmd);
}

/////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMFOMXBaseDecNode::ReleasePort(PVMFSessionId s, PVMFPortInterface& aPort, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "%s::ReleasePort() called", iName.Str()));
    PVMFOMXBaseDecNodeCommand cmd;
    cmd.PVMFOMXBaseDecNodeCommandBase::Construct(s, PVMFOMXBaseDecNodeCommand::PVOMXBASEDEC_NODE_CMD_RELEASEPORT, aPort, aContext);
    return QueueCommandL(cmd);
}

/////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMFOMXBaseDecNode::Init(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "%s::Init() called", iName.Str()));
    PVMFOMXBaseDecNodeCommand cmd;
    cmd.PVMFOMXBaseDecNodeCommandBase::Construct(s, PVMFOMXBaseDecNodeCommand::PVOMXBASEDEC_NODE_CMD_INIT, aContext);
    return QueueCommandL(cmd);
}

/////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMFOMXBaseDecNode::Prepare(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "%s::Prepare() called", iName.Str()));
    PVMFOMXBaseDecNodeCommand cmd;
    cmd.PVMFOMXBaseDecNodeCommandBase::Construct(s, PVMFOMXBaseDecNodeCommand::PVOMXBASEDEC_NODE_CMD_PREPARE, aContext);
    return QueueCommandL(cmd);
}

/////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMFOMXBaseDecNode::Start(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "%s::Start() called", iName.Str()));
    PVMFOMXBaseDecNodeCommand cmd;
    cmd.PVMFOMXBaseDecNodeCommandBase::Construct(s, PVMFOMXBaseDecNodeCommand::PVOMXBASEDEC_NODE_CMD_START, aContext);
    return QueueCommandL(cmd);
}

/////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMFOMXBaseDecNode::Stop(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "%s::Stop() called", iName.Str()));
    PVMFOMXBaseDecNodeCommand cmd;
    cmd.PVMFOMXBaseDecNodeCommandBase::Construct(s, PVMFOMXBaseDecNodeCommand::PVOMXBASEDEC_NODE_CMD_STOP, aContext);
    return QueueCommandL(cmd);
}

/////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMFOMXBaseDecNode::Flush(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "%s::Flush() called", iName.Str()));
    PVMFOMXBaseDecNodeCommand cmd;
    cmd.PVMFOMXBaseDecNodeCommandBase::Construct(s, PVMFOMXBaseDecNodeCommand::PVOMXBASEDEC_NODE_CMD_FLUSH, aContext);
    return QueueCommandL(cmd);
}

/////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMFOMXBaseDecNode::Pause(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "%s::Pause() called", iName.Str()));
    PVMFOMXBaseDecNodeCommand cmd;
    cmd.PVMFOMXBaseDecNodeCommandBase::Construct(s, PVMFOMXBaseDecNodeCommand::PVOMXBASEDEC_NODE_CMD_PAUSE, aContext);
    return QueueCommandL(cmd);
}

/////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMFOMXBaseDecNode::Reset(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "%s::Reset() called", iName.Str()));
    PVMFOMXBaseDecNodeCommand cmd;
    cmd.PVMFOMXBaseDecNodeCommandBase::Construct(s, PVMFOMXBaseDecNodeCommand::PVOMXBASEDEC_NODE_CMD_RESET, aContext);
    return QueueCommandL(cmd);
}

/////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMFOMXBaseDecNode::CancelAllCommands(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "%s::CancelAllCommands() called", iName.Str()));
    PVMFOMXBaseDecNodeCommand cmd;
    cmd.PVMFOMXBaseDecNodeCommandBase::Construct(s, PVMFOMXBaseDecNodeCommand::PVOMXBASEDEC_NODE_CMD_CANCELALL, aContext);
    return QueueCommandL(cmd);
}

/////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMFOMXBaseDecNode::CancelCommand(PVMFSessionId s, PVMFCommandId aCmdId, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "%s::CancelCommand() called", iName.Str()));
    PVMFOMXBaseDecNodeCommand cmd;
    cmd.PVMFOMXBaseDecNodeCommandBase::Construct(s, PVMFOMXBaseDecNodeCommand::PVOMXBASEDEC_NODE_CMD_CANCELCMD, aCmdId, aContext);
    return QueueCommandL(cmd);
}

/////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMFOMXBaseDecNode::SetDecoderNodeConfiguration(PVMFOMXBaseDecNodeConfig& aNodeConfig)
{
    iNodeConfig = aNodeConfig;
    return PVMFSuccess;
}


/////////////////////
// Private Section //
/////////////////////

/////////////////////////////////////////////////////////////////////////////
// Class Constructor
/////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFOMXBaseDecNode::PVMFOMXBaseDecNode(int32 aPriority, const char aAOName[]) :
        OsclActiveObject(aPriority, aAOName),
        iInPort(NULL),
        iOutPort(NULL),
        iOutBufMemoryPool(NULL),
        iMediaDataMemPool(NULL),
        iOMXComponentOutputBufferSize(0),
        iOutputAllocSize(0),
        iProcessingState(EPVMFOMXBaseDecNodeProcessingState_Idle),
        iOMXDecoder(NULL),
        iSendBOS(false),
        iStreamID(0),
        iBOSTimestamp(0),
        iSeqNum(0),
        iSeqNum_In(0),
        iIsAdded(true),
        iLogger(NULL),
        iDataPathLogger(NULL),
        iClockLogger(NULL),
        iExtensionRefCount(0),
        iEndOfDataReached(false),
        iEndOfDataTimestamp(0),
        iDiagnosticsLogger(NULL),
        iDiagnosticsLogged(false),
        iAvgBitrateValue(0),
        iResetInProgress(false),
        iResetMsgSent(false),
        iStopInResetMsgSent(false),
        iCompactFSISettingSucceeded(false)
{
    iThreadSafeHandlerEventHandler = NULL;
    iThreadSafeHandlerEmptyBufferDone = NULL;
    iThreadSafeHandlerFillBufferDone = NULL;

    iInBufMemoryPool = NULL;
    iOutBufMemoryPool = NULL;

    // init to some value
    iOMXComponentOutputBufferSize = 0;
    iNumOutputBuffers = 0;
    iOMXComponentInputBufferSize = 0;
    iNumInputBuffers = 0;

    iDoNotSendOutputBuffersDownstreamFlag = false;
    iDoNotSaveInputBuffersFlag = false;

    iOutputBuffersFreed = true;// buffers have not been created yet, so they can be considered freed
    iInputBuffersFreed = true;

    // dynamic port reconfig init vars
    iSecondPortReportedChange = false;
    iDynamicReconfigInProgress = false;
    iPauseCommandWasSentToComponent = false;
    iStopCommandWasSentToComponent = false;

    // EOS flag init
    iIsEOSSentToComponent = false;
    iIsEOSReceivedFromComponent = false;

    // reset repositioning related flags
    iIsRepositioningRequestSentToComponent = false;
    iIsRepositionDoneReceivedFromComponent = false;
    iIsOutputPortFlushed = false;
    iIsInputPortFlushed = false;

    // init state of component
    iCurrentDecoderState = OMX_StateInvalid;

    iOutTimeStamp = 0;

    // counts output frames (for logging)
    iFrameCounter = 0;
    iInputBufferUnderConstruction = NULL; // for partial frame assembly
    iFirstPieceOfPartialFrame = true;
    iObtainNewInputBuffer = true;
    iFirstDataMsgAfterBOS = true;
    iKeepDroppingMsgsUntilMarkerBit = false;

}

/////////////////////////////////////////////////////////////////////////////
// Local Run Routine
/////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PVMFOMXBaseDecNode::Run()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "%s::Run() In", iName.Str()));

    // if reset is in progress, call DoReset again until Reset Msg is sent
    if ((iResetInProgress == true) &&
            (iResetMsgSent == false) &&
            (iCurrentCommand.size() > 0) &&
            (iCurrentCommand.front().iCmd == PVMFOMXBaseDecNodeCommand::PVOMXBASEDEC_NODE_CMD_RESET)
       )
    {
        DoReset(iCurrentCommand.front());
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "%s::Run() - Calling DoReset", iName.Str()));
        return; // don't do anything else
    }
    //Check for NODE commands...
    if (!iInputCommands.empty())
    {
        if (ProcessCommand(iInputCommands.front()))
        {
            if (iInterfaceState != EPVMFNodeCreated
                    && (!iInputCommands.empty() || (iInPort && (iInPort->IncomingMsgQueueSize() > 0)) ||
                        (iDataIn.GetRep() != NULL) || (iDynamicReconfigInProgress == true) ||
                        ((iNumOutstandingOutputBuffers < iNumOutputBuffers) &&
                         (iProcessingState == EPVMFOMXBaseDecNodeProcessingState_ReadyToDecode))
                       ))
            {
                // reschedule if more data is available, or if port reconfig needs to be finished (even if there is no new data)
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "%s::Run() - rescheduling after process command", iName.Str()));
                RunIfNotReady();
            }
            return;
        }

        if (!iInputCommands.empty())
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "%s::Run() - rescheduling to process more commands", iName.Str()));
            RunIfNotReady();
        }
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "%s::Run() - Input commands empty", iName.Str()));
    }

    if (((iCurrentCommand.size() == 0) && (iInterfaceState != EPVMFNodeStarted)) ||
            ((iCurrentCommand.size() > 0) && (iCurrentCommand.front().iCmd == PVMFOMXBaseDecNodeCommand::PVOMXBASEDEC_NODE_CMD_START) && (iInterfaceState != EPVMFNodeStarted)))
    {
        // rescheduling because of input data will be handled in Command Processing Part
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "%s::Run() - Node not in Started state yet", iName.Str()));
        return;
    }


    // Process port activity, push out all outgoing messages
    if (iOutPort)
    {
        while (iOutPort->OutgoingMsgQueueSize())
        {
            // if port is busy it is going to wakeup from port ready event
            if (!ProcessOutgoingMsg(iOutPort))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "%s::Run() - Outgoing Port Busy, cannot send more msgs", iName.Str()));
                break;
            }
        }
    }
    int loopCount = 0;
#if (PVLOGGER_INST_LEVEL >= PVLOGMSG_INST_REL)
    uint32 startticks = OsclTickCount::TickCount();
    uint32 starttime = OsclTickCount::TicksToMsec(startticks);
#endif
    do // Try to consume all the data from the Input port
    {
        // Process port activity if there is no input data that is being processed
        // Do not accept any input if EOS needs to be sent out
        if (iInPort && (iInPort->IncomingMsgQueueSize() > 0) && (iDataIn.GetRep() == NULL) &&
                (!iEndOfDataReached) &&
                (!iDynamicReconfigInProgress) &&
                (!iIsRepositioningRequestSentToComponent)
           )

        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "%s::Run() - Getting more input", iName.Str()));
            if (!ProcessIncomingMsg(iInPort))
            {
                //Re-schedule
                RunIfNotReady();
                return;
            }
        }

        if (iSendBOS)
        {

            // this routine may be re-entered multiple times in multiple Run's before the component goes through cycle execute->idle->execute
            if (!HandleRepositioning())
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "%s::Run() - Repositioning not done yet", iName.Str()));

                return;
            }

            SendBeginOfMediaStreamCommand();


        }
        // If in init or ready to decode state, process data in the input port if there is input available and input buffers are present
        // (note: at EOS, iDataIn will not be available)

        if ((iDataIn.GetRep() != NULL) ||
                ((iNumOutstandingOutputBuffers < iNumOutputBuffers) &&
                 (iProcessingState == EPVMFOMXBaseDecNodeProcessingState_ReadyToDecode) &&
                 (iResetMsgSent == false)) ||
                ((iDynamicReconfigInProgress == true) && (iResetMsgSent == false))

           )
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "%s::Run() - Calling HandleProcessingState", iName.Str()));

            // input data is available, that means there is input data to be decoded
            if (HandleProcessingState() != PVMFSuccess)
            {
                // If HandleProcessingState does not return Success, we must wait for an event
                // no point in  rescheduling
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "%s::Run() - HandleProcessingState did not return Success", iName.Str()));

                return;
            }
        }

        loopCount++;
    }
    while (iInPort &&
            (((iInPort->IncomingMsgQueueSize() > 0) || (iDataIn.GetRep() != NULL)) && (iNumOutstandingInputBuffers < iNumInputBuffers))
            && (!iEndOfDataReached)
            && (iResetMsgSent == false)
          );
#if (PVLOGGER_INST_LEVEL >= PVLOGMSG_INST_REL)
    uint32 endticks = OsclTickCount::TickCount();
    uint32 endtime = OsclTickCount::TicksToMsec(endticks);
    uint32 timeinloop;
    timeinloop = (endtime - starttime);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iRunlLogger, PVLOGMSG_INFO,
                    (0, "%s::Run() - LoopCount = %d, Time spent in loop(in ms) = %d, iNumOutstandingInputBuffers = %d, iNumOutstandingOutputBuffers = %d ",
                     iName.Str(), loopCount, timeinloop, iNumOutstandingInputBuffers, iNumOutstandingOutputBuffers));
#endif

    // EOS processing:
    // first send an empty buffer to OMX component and mark the EOS flag
    // wait for the OMX component to send async event to indicate that it has reached this EOS buffer
    // then, create and send the EOS message downstream

    if (iEndOfDataReached && !iDynamicReconfigInProgress)
    {

        // if EOS was not sent yet and we have an available input buffer, send EOS buffer to component
        if (!iIsEOSSentToComponent && (iNumOutstandingInputBuffers < iNumInputBuffers))
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "%s::Run() - Sending EOS marked buffer To Component ", iName.Str()));

            iIsEOSSentToComponent = true;

            // if the component is not yet initialized or if it's in the middle of port reconfig,
            // don't send EOS buffer to component. It does not care. Just set the flag as if we received
            // EOS from the component to enable sending EOS downstream
            if (iProcessingState != EPVMFOMXBaseDecNodeProcessingState_ReadyToDecode)
            {

                iIsEOSReceivedFromComponent = true;
            }
            else if (!SendEOSBufferToOMXComponent())

            {
                // for some reason, Component can't receive the EOS buffer
                // it could be that it is not initialized yet (because EOS could be the first msg). In this case,
                // send the EOS downstream anyway
                iIsEOSReceivedFromComponent = true;
            }
        }

        // DV: we must wait for event (acknowledgment from component)
        // before sending EOS downstream. This is because OMX Component will send
        // the EOS event only after processing remaining buffers

        if (iIsEOSReceivedFromComponent)
        {

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "%s::Run() - Received EOS from component, Sending EOS msg downstream ", iName.Str()));

            if (iOutPort && iOutPort->IsOutgoingQueueBusy())
            {
                // note: we already tried to empty the outgoing q. If it's still busy,
                // it means that output port is busy. Just return and wait for the port to become free.
                // this will wake up the node and it will send out a msg from the q etc.
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "%s::Run() - - EOS cannot be sent downstream, outgoing queue busy - wait", iName.Str()));
                return;
            }

            if (SendEndOfTrackCommand()) // this will only q the EOS
            {
                // EOS send downstream OK, so reset the flag
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "%s::Run() - EOS was queued to be sent downstream", iName.Str()));

                iEndOfDataReached = false; // to resume normal processing, reset the flags
                iIsEOSSentToComponent = false;
                iIsEOSReceivedFromComponent = false;

                RunIfNotReady(); // Run again to send out the EOS msg from the outgoing q, and resume
                // normal processing
                ReportInfoEvent(PVMFInfoEndOfData);
            }
        }
        else
        {
            // keep sending output buffers, it's possible that the component needs to flush output
            //	data at the end
            while (iNumOutstandingOutputBuffers < iNumOutputBuffers)
            {
                if (!SendOutputBufferToOMXComponent())
                    break;
            }
        }

    }


    //Check for flash command complition...
    if (iInPort && iOutPort && (iCurrentCommand.size() > 0) &&
            (iCurrentCommand.front().iCmd == PVMFOMXBaseDecNodeCommand::PVOMXBASEDEC_NODE_CMD_FLUSH) &&
            (iInPort->IncomingMsgQueueSize() == 0) &&
            (iOutPort->OutgoingMsgQueueSize() == 0) &&
            (iDataIn.GetRep() == NULL))
    {
        //flush command is complited
        //Debug check-- all the port queues should be empty at this point.

        OSCL_ASSERT(iInPort->IncomingMsgQueueSize() == 0 && iOutPort->OutgoingMsgQueueSize() == 0);

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "%s::Run() - Flush pending", iName.Str()));
        iEndOfDataReached = false;
        iIsEOSSentToComponent = false;
        iIsEOSReceivedFromComponent = false;


        //Flush is complete.  Go to initialized state.
        SetState(EPVMFNodePrepared);
        //resume port input so the ports can be re-started.
        iInPort->ResumeInput();
        iOutPort->ResumeInput();
        CommandComplete(iCurrentCommand, iCurrentCommand.front(), PVMFSuccess);
        RunIfNotReady();
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "%s::Run() Out", iName.Str()));
}

/////////////////////////////////////////////////////////////////////////////
// This routine will dispatch recived commands
/////////////////////////////////////////////////////////////////////////////
bool PVMFOMXBaseDecNode::ProcessCommand(PVMFOMXBaseDecNodeCommand& aCmd)
{
    //normally this node will not start processing one command
    //until the prior one is finished.  However, a hi priority
    //command such as Cancel must be able to interrupt a command
    //in progress.
    if (!iCurrentCommand.empty() && !aCmd.hipri())
        return false;

    switch (aCmd.iCmd)
    {
        case PVMFOMXBaseDecNodeCommand::PVOMXBASEDEC_NODE_CMD_QUERYUUID:
            DoQueryUuid(aCmd);
            break;

        case PVMFOMXBaseDecNodeCommand::PVOMXBASEDEC_NODE_CMD_QUERYINTERFACE:
            DoQueryInterface(aCmd);
            break;

        case PVMFOMXBaseDecNodeCommand::PVOMXBASEDEC_NODE_CMD_REQUESTPORT:
            DoRequestPort(aCmd);
            break;

        case PVMFOMXBaseDecNodeCommand::PVOMXBASEDEC_NODE_CMD_RELEASEPORT:
            DoReleasePort(aCmd);
            break;

        case PVMFOMXBaseDecNodeCommand::PVOMXBASEDEC_NODE_CMD_INIT:
            DoInit(aCmd);
            break;

        case PVMFOMXBaseDecNodeCommand::PVOMXBASEDEC_NODE_CMD_PREPARE:
            DoPrepare(aCmd);
            break;

        case PVMFOMXBaseDecNodeCommand::PVOMXBASEDEC_NODE_CMD_START:
            DoStart(aCmd);
            break;

        case PVMFOMXBaseDecNodeCommand::PVOMXBASEDEC_NODE_CMD_STOP:
            DoStop(aCmd);
            break;

        case PVMFOMXBaseDecNodeCommand::PVOMXBASEDEC_NODE_CMD_FLUSH:
            DoFlush(aCmd);
            break;

        case PVMFOMXBaseDecNodeCommand::PVOMXBASEDEC_NODE_CMD_PAUSE:
            DoPause(aCmd);
            break;

        case PVMFOMXBaseDecNodeCommand::PVOMXBASEDEC_NODE_CMD_RESET:
            DoReset(aCmd);
            break;

        case PVMFOMXBaseDecNodeCommand::PVOMXBASEDEC_NODE_CMD_CANCELCMD:
            DoCancelCommand(aCmd);
            break;

        case PVMFOMXBaseDecNodeCommand::PVOMXBASEDEC_NODE_CMD_CANCELALL:
            DoCancelAllCommands(aCmd);
            break;

        case PVMFOMXBaseDecNodeCommand::PVOMXBASEDEC_NODE_CMD_GETNODEMETADATAKEY:
        {
            PVMFStatus retval = DoGetNodeMetadataKey(aCmd);
            CommandComplete(iInputCommands, aCmd, retval);
        }
        break;

        case PVMFOMXBaseDecNodeCommand::PVOMXBASEDEC_NODE_CMD_GETNODEMETADATAVALUE:
        {
            PVMFStatus retval = DoGetNodeMetadataValue(aCmd);
            CommandComplete(iInputCommands, aCmd, retval);
        }
        break;

        default://unknown command type
            CommandComplete(iInputCommands, aCmd, PVMFFailure);
            break;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// This routine will process incomming message from the port
/////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF bool PVMFOMXBaseDecNode::ProcessIncomingMsg(PVMFPortInterface* aPort)
{
    //Called by the AO to process one buffer off the port's
    //incoming data queue.  This routine will dequeue and
    //dispatch the data.

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "0x%x %s::ProcessIncomingMsg: aPort=0x%x", this, iName.Str(), aPort));

    PVMFStatus status = PVMFFailure;


//#define SIMULATE_BOS
#ifdef SIMULATE_BOS

    if (((PVMFOMXDecPort*)aPort)->iNumFramesConsumed == 6))
    {

        PVMFSharedMediaCmdPtr BOSCmdPtr = PVMFMediaCmd::createMediaCmd();

        // Set the format ID to BOS
        BOSCmdPtr->setFormatID(PVMF_MEDIA_CMD_BOS_FORMAT_ID);

        // Set the timestamp
        BOSCmdPtr->setTimestamp(201);
        BOSCmdPtr->setStreamID(0);

        // Convert to media message and send it out
        PVMFSharedMediaMsgPtr mediaMsgOut;
        convertToPVMFMediaCmdMsg(mediaMsgOut, BOSCmdPtr);

        //store the stream id and time stamp of bos message
        iStreamID = mediaMsgOut->getStreamID();
        iBOSTimestamp = mediaMsgOut->getTimestamp();
        iSendBOS = true;

        ((PVMFOMXDecPort*)aPort)->iNumFramesConsumed++;
        return true;

    }
#endif
//#define SIMULATE_PREMATURE_EOS
#ifdef SIMULATE_PREMATURE_EOS
    if (((PVMFOMXDecPort*)aPort)->iNumFramesConsumed == 5)
    {
        PVMFSharedMediaCmdPtr EOSCmdPtr = PVMFMediaCmd::createMediaCmd();

        // Set the format ID to EOS
        EOSCmdPtr->setFormatID(PVMF_MEDIA_CMD_EOS_FORMAT_ID);

        // Set the timestamp
        EOSCmdPtr->setTimestamp(200);

        // Convert to media message and send it out
        PVMFSharedMediaMsgPtr mediaMsgOut;
        convertToPVMFMediaCmdMsg(mediaMsgOut, EOSCmdPtr);

        ((PVMFOMXDecPort*)aPort)->iNumFramesConsumed++;

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "%s::ProcessIncomingMsg: SIMULATED EOS", iName.Str()));

        // Set EOS flag
        iEndOfDataReached = true;
        // Save the timestamp for the EOS cmd
        iEndOfDataTimestamp = mediaMsgOut->getTimestamp();

        return true;
    }

#endif



    PVMFSharedMediaMsgPtr msg;

    status = aPort->DequeueIncomingMsg(msg);
    if (status != PVMFSuccess)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                    (0, "0x%x %s::ProcessIncomingMsg: Error - DequeueIncomingMsg failed", this, iName.Str()));
        return false;
    }

    if (msg->getFormatID() == PVMF_MEDIA_CMD_BOS_FORMAT_ID)
    {
        //store the stream id and time stamp of bos message
        iStreamID = msg->getStreamID();
        iBOSTimestamp = msg->getTimestamp();
        iSendBOS = true;

        // if new BOS arrives, and
        //if we're in the middle of a partial frame assembly
        // abandon it and start fresh
        if (iObtainNewInputBuffer == false)
        {
            if (iInputBufferUnderConstruction != NULL)
            {
                if (iInBufMemoryPool != NULL)
                {
                    iInBufMemoryPool->deallocate((OsclAny *)iInputBufferUnderConstruction);
                }
                iInputBufferUnderConstruction = NULL;
            }
            iObtainNewInputBuffer = true;

        }

        // needed to init the sequence numbers and timestamp for partial frame assembly
        iFirstDataMsgAfterBOS = true;
        iKeepDroppingMsgsUntilMarkerBit = false;

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "%s::ProcessIncomingMsg: Received BOS stream %d, timestamp %d", iName.Str(), iStreamID, iBOSTimestamp));
        ((PVMFOMXDecPort*)aPort)->iNumFramesConsumed++;
        return true;
    }
    else if (msg->getFormatID() == PVMF_MEDIA_CMD_EOS_FORMAT_ID)
    {
        // Set EOS flag
        iEndOfDataReached = true;
        // Save the timestamp for the EOS cmd
        iEndOfDataTimestamp = msg->getTimestamp();

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "%s::ProcessIncomingMsg: Received EOS", iName.Str()));

        ((PVMFOMXDecPort*)aPort)->iNumFramesConsumed++;
        return true; // do not do conversion into media data, just set the flag and leave
    }

    convertToPVMFMediaData(iDataIn, msg);


    iCurrFragNum = 0; // for new message, reset the fragment counter
    iIsNewDataFragment = true;

    ((PVMFOMXDecPort*)aPort)->iNumFramesConsumed++;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "%s::ProcessIncomingMsg() Received %d frames", iName.Str(), ((PVMFOMXDecPort*)aPort)->iNumFramesConsumed));

    //return true if we processed an activity...
    return true;
}

/////////////////////////////////////////////////////////////////////////////
// This routine will process outgoing message by sending it into output the port
/////////////////////////////////////////////////////////////////////////////
bool PVMFOMXBaseDecNode::ProcessOutgoingMsg(PVMFPortInterface* aPort)
{
    //Called by the AO to process one message off the outgoing
    //message queue for the given port.  This routine will
    //try to send the data to the connected port.

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "0x%x %s::ProcessOutgoingMsg: aPort=0x%x", this, iName.Str(), aPort));

    PVMFStatus status = aPort->Send();
    if (status == PVMFErrBusy)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "0x%x %s::ProcessOutgoingMsg: Connected port goes into busy state", this, iName.Str()));
    }

    //Report any unexpected failure in port processing...
    //(the InvalidState error happens when port input is suspended,
    //so don't report it.)
    if (status != PVMFErrBusy
            && status != PVMFSuccess
            && status != PVMFErrInvalidState)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "0x%x %s::Run: Error - ProcessPortActivity failed. port=0x%x, type=%d",
                         this, iName.Str(), iOutPort, PVMF_PORT_ACTIVITY_OUTGOING_MSG));
        ReportErrorEvent(PVMFErrPortProcessing);
    }

    //return true if we processed an activity...
    return (status != PVMFErrBusy);
}

/////////////////////////////////////////////////////////////////////////////
// This routine will process received data usign State Machine
/////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFOMXBaseDecNode::HandleProcessingState()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "%s::HandleProcessingState() In", iName.Str()));

    PVMFStatus status = PVMFSuccess;

    switch (iProcessingState)
    {
        case EPVMFOMXBaseDecNodeProcessingState_InitDecoder:
        {
            // do init only if input data is available
            if (iDataIn.GetRep() != NULL)
            {
                if (!InitDecoder(iDataIn))
                {
                    // Decoder initialization failed. Fatal error
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "%s::HandleProcessingState() Decoder initialization failed", iName.Str()));
                    ReportErrorEvent(PVMFErrResourceConfiguration);
                    ChangeNodeState(EPVMFNodeError);
                    break;
                }

                // if a callback already happened, continue to decoding. If not, wait
                // it is also possible that port settings changed event may occur.
                //DV: temp
                //if(iProcessingState != EPVMFOMXBaseDecNodeProcessingState_ReadyToDecode)
                //	iProcessingState = EPVMFOMXBaseDecNodeProcessingState_WaitForInitCompletion;

                iProcessingState = EPVMFOMXBaseDecNodeProcessingState_ReadyToDecode;
                // spin once to send output buffers
                RunIfNotReady();
                status = PVMFSuccess; // allow rescheduling
            }
            break;
        }

        case EPVMFOMXBaseDecNodeProcessingState_WaitForInitCompletion:
        {

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "%s::HandleProcessingState() WaitForInitCompletion -> wait for config buffer to return", iName.Str()));


            status = PVMFPending; // prevent rescheduling
            break;
        }
        // The FOLLOWING 4 states handle Dynamic Port Reconfiguration
        case EPVMFOMXBaseDecNodeProcessingState_PortReconfig:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "%s::HandleProcessingState() Port Reconfiguration -> Sending Flush Command", iName.Str()));


            // Collect all buffers first (before starting the portDisable command)
            // FIRST send a flush command. This will return all buffers from the component. Any outstanding buffers are in MIO
            // Then wait for all buffers to come back from MIO. If we haven't sent port disable, we'll be able to process
            // other commands in the copmponent (such as pause, stop etc.)
            OMX_ERRORTYPE err = OMX_ErrorNone;
            OMX_STATETYPE sState;

            // first check the state (if executing or paused, continue)
            err = OMX_GetState(iOMXDecoder, &sState);
            if (err != OMX_ErrorNone)
            {
                //Error condition report
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "%s::HandleProcessingState (): PortReconfig Can't get State of decoder - trying to send port flush request!", iName.Str()));

                sState = OMX_StateInvalid;
                ReportErrorEvent(PVMFErrResourceConfiguration);
                ChangeNodeState(EPVMFNodeError);
                status = PVMFFailure;
                break;
            }

            if ((sState != OMX_StateExecuting) && (sState != OMX_StatePause))
            {

                // possibly as a consequence of a previously queued cmd to go to Idle state?
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "%s::HandleProcessingState (): PortReconfig: Component State is not executing or paused, do not proceed with port flush", iName.Str()));


            }
            else
            {

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "%s::HandleProcessingState (): PortReconfig Sending Flush command to component", iName.Str()));


                // the port will now start returning outstanding buffers
                // set the flag to prevent output from going downstream (in case of output port being reconfigd)
                // set the flag to prevent input from being saved and returned to component (in case of input port being reconfigd)
                // set the state to wait for port saying it is disabled
                if (iPortIndexForDynamicReconfig == iOutputPortIndex)
                {
                    iDoNotSendOutputBuffersDownstreamFlag = true;
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                    (0, "%s::HandleProcessingState() Port Reconfiguration -> Output Port", iName.Str()));

                }
                else if (iPortIndexForDynamicReconfig == iInputPortIndex)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                    (0, "%s::HandleProcessingState() Port Reconfiguration -> Input Port", iName.Str()));

                    iDoNotSaveInputBuffersFlag = true;
                }
                else
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                    (0, "%s::HandleProcessingState() Port Reconfiguration -> UNKNOWN PORT", iName.Str()));

                    sState = OMX_StateInvalid;
                    ReportErrorEvent(PVMFErrResourceConfiguration);
                    ChangeNodeState(EPVMFNodeError);
                    status = PVMFFailure;
                    break;

                }

                // send command to flush appropriate port
                err = OMX_SendCommand(iOMXDecoder, OMX_CommandFlush, iPortIndexForDynamicReconfig, NULL);
                if (err != OMX_ErrorNone)
                {
                    //Error condition report
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "%s::HandleProcessingState (): PortReconfig : Can't send flush command !", iName.Str()));

                    sState = OMX_StateInvalid;
                    ReportErrorEvent(PVMFErrResourceConfiguration);
                    ChangeNodeState(EPVMFNodeError);
                    status = PVMFFailure;
                    break;
                }



            }

            // now sit back and wait for buffers to return
            // if there is a pause/stop cmd in the meanwhile, component will process it
            // and the node will end up in pause/stop state (so this internal state does not matter)
            iProcessingState = EPVMFOMXBaseDecNodeProcessingState_WaitForBufferReturn;


            // fall through to the next case to check if all buffers are already back



        }

        case EPVMFOMXBaseDecNodeProcessingState_WaitForBufferReturn:
        {
            // as buffers are coming back, Run may be called, wait until all buffers are back, then Free them all

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "%s::HandleProcessingState() Port Reconfiguration -> WaitForBufferReturn ", iName.Str()));
            // check if it's output port being reconfigured
            if (iPortIndexForDynamicReconfig == iOutputPortIndex)
            {
                // if all buffers have returned, free them
                if (iNumOutstandingOutputBuffers == 0)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                    (0, "%s::HandleProcessingState() Port Reconfiguration -> all output buffers are back, free them", iName.Str()));

                    // port reconfiguration is required. Only one port at a time is disabled and then re-enabled after buffer resizing
                    OMX_SendCommand(iOMXDecoder, OMX_CommandPortDisable, iPortIndexForDynamicReconfig, NULL);


                    if (false == iOutputBuffersFreed)
                    {
                        if (!FreeBuffersFromComponent(iOutBufMemoryPool, // allocator
                                                      iOutputAllocSize,	 // size to allocate from pool (hdr only or hdr+ buffer)
                                                      iNumOutputBuffers, // number of buffers
                                                      iOutputPortIndex, // port idx
                                                      false // this is not input
                                                     ))
                        {
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                            (0, "%s::HandleProcessingState() Port Reconfiguration -> Cannot free output buffers ", iName.Str()));

                            SetState(EPVMFNodeError);
                            ReportErrorEvent(PVMFErrNoMemory);
                            return PVMFErrNoMemory;
                        }
                    }
                    // if the callback (that port is disabled) has not arrived yet, wait for it
                    // if it has arrived, it will set the state to PortReEnable
                    if (iProcessingState != EPVMFOMXBaseDecNodeProcessingState_PortReEnable)
                        iProcessingState = EPVMFOMXBaseDecNodeProcessingState_WaitForPortDisable;

                    status = PVMFSuccess; // allow rescheduling of the node potentially
                }
                else
                    status = PVMFPending; // must wait for buffers to come back. No point in automatic rescheduling
                // but each buffer will reschedule the node when it comes in
            }
            else
            {
                // this is input port

                // if all buffers have returned, free them
                if (iNumOutstandingInputBuffers == 0)
                {

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                    (0, "%s::HandleProcessingState() Port Reconfiguration -> all input buffers are back, free them", iName.Str()));

                    // port reconfiguration is required. Only one port at a time is disabled and then re-enabled after buffer resizing
                    OMX_SendCommand(iOMXDecoder, OMX_CommandPortDisable, iPortIndexForDynamicReconfig, NULL);

                    if (false == iInputBuffersFreed)
                    {
                        if (!FreeBuffersFromComponent(iInBufMemoryPool, // allocator
                                                      iInputAllocSize,	 // size to allocate from pool (hdr only or hdr+ buffer)
                                                      iNumInputBuffers, // number of buffers
                                                      iInputPortIndex, // port idx
                                                      true // this is input
                                                     ))
                        {
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                            (0, "%s::HandleProcessingState() Port Reconfiguration -> Cannot free input buffers ", iName.Str()));

                            SetState(EPVMFNodeError);
                            ReportErrorEvent(PVMFErrNoMemory);
                            return PVMFErrNoMemory;

                        }
                    }
                    // if the callback (that port is disabled) has not arrived yet, wait for it
                    // if it has arrived, it will set the state to PortReEnable
                    if (iProcessingState != EPVMFOMXBaseDecNodeProcessingState_PortReEnable)
                        iProcessingState = EPVMFOMXBaseDecNodeProcessingState_WaitForPortDisable;

                    status = PVMFSuccess; // allow rescheduling of the node
                }
                else
                    status = PVMFPending; // must wait for buffers to come back. No point in automatic
                // rescheduling. Each buffer will reschedule the node
                // when it comes in
            }


            // the state will be changed to PortReEnable once we get confirmation that Port was actually disabled
            break;
        }

        case EPVMFOMXBaseDecNodeProcessingState_WaitForPortDisable:
        {

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "%s::HandleProcessingState() Port Reconfiguration -> wait for port disable callback", iName.Str()));
            // do nothing. Just wait for the port to become disabled (we'll get event from component, which will
            // transition the state to PortReEnable
            status = PVMFPending; // prevent Rescheduling the node
            break;
        }

        case EPVMFOMXBaseDecNodeProcessingState_PortReEnable:
        {

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "%s::HandleProcessingState() Port Reconfiguration -> Sending reenable port command", iName.Str()));

            status = HandlePortReEnable();
            break;
        }

        case EPVMFOMXBaseDecNodeProcessingState_WaitForPortEnable:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "%s::HandleProcessingState() Port Reconfiguration -> wait for port enable callback", iName.Str()));
            // do nothing. Just wait for the port to become enabled (we'll get event from component, which will
            // transition the state to ReadyToDecode
            status = PVMFPending; // prevent ReScheduling
            break;
        }

        // NORMAL DATA FLOW STATE:
        case EPVMFOMXBaseDecNodeProcessingState_ReadyToDecode:
        {

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "%s::HandleProcessingState() Ready To Decode start", iName.Str()));
            // In normal data flow and decoding state
            // Send all available output buffers to the decoder

            while (iNumOutstandingOutputBuffers < iNumOutputBuffers)
            {
                // grab buffer header from the mempool if possible, and send to component
                if (!SendOutputBufferToOMXComponent())

                    break;

            }


            // next, see if partially consumed input buffer needs to be resent back to OMX component
            // NOTE: it is not allowed that the component returns more than 1 partially consumed input buffers
            //		 i.e. if a partially consumed input buffer is returned, it is assumed that the OMX component
            //		 will be waiting to get data

            if (iInputBufferToResendToComponent != NULL)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "%s::HandleProcessingState() Sending previous - partially consumed input back to the OMX component", iName.Str()));

                OMX_EmptyThisBuffer(iOMXDecoder, iInputBufferToResendToComponent);
                iInputBufferToResendToComponent = NULL; // do this only once
            }
            else if ((iNumOutstandingInputBuffers < iNumInputBuffers) && (iDataIn.GetRep() != NULL))
            {
                // try to get an input buffer header
                // and send the input data over to the component
                SendInputBufferToOMXComponent();
            }

            status = PVMFSuccess;
            break;


        }
        case EPVMFOMXBaseDecNodeProcessingState_Stopping:
        {

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "%s::HandleProcessingState() Stopping -> wait for Component to move from Executing->Idle", iName.Str()));


            status = PVMFPending; // prevent rescheduling
            break;
        }

        case EPVMFOMXBaseDecNodeProcessingState_Pausing:
        {

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "%s::HandleProcessingState() Pausing -> wait for Component to move from Executing->Pause", iName.Str()));


            status = PVMFPending; // prevent rescheduling
            break;
        }


        case EPVMFOMXBaseDecNodeProcessingState_WaitForOutgoingQueue:
            status = PVMFPending;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "%s::HandleProcessingState() Do nothing since waiting for output port queue to become available", iName.Str()));
            break;

        default:
            break;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "%s::HandleProcessingState() Out", iName.Str()));

    return status;

}
/////////////////////////////////////////////////////////////////////////////
bool PVMFOMXBaseDecNode::SendOutputBufferToOMXComponent()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "%s::SendOutputBufferToOMXComponent() In", iName.Str()));


    OutputBufCtrlStruct *output_buf = NULL;
    int32 errcode = OsclErrNone;
    uint32 ii;

    // try to get output buffer header
    OSCL_TRY(errcode, output_buf = (OutputBufCtrlStruct *) iOutBufMemoryPool->allocate(iOutputAllocSize));

    if (OsclErrNone != errcode)
    {
        if (OsclErrNoResources == errcode)
        {

            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger,
                            PVLOGMSG_DEBUG, (0, "%s::SendOutputBufferToOMXComponent() No more output buffers in the mempool", iName.Str()));

            iOutBufMemoryPool->notifyfreechunkavailable(*this, (OsclAny *) iOutBufMemoryPool); // To signal when next deallocate() is called on mempool

            return false;
        }
        else
        {
            // Memory allocation for the pool failed
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "%s::SendOutputBufferToOMXComponent() Output mempool error", iName.Str()));


            SetState(EPVMFNodeError);
            ReportErrorEvent(PVMFErrNoMemory);
            return false;
        }

    }


    //for every allocated buffer, make sure you notify when buffer is released. Keep track of allocated buffers
    // use mempool as context to recognize which buffer (input or output) was returned
    iOutBufMemoryPool->notifyfreechunkavailable(*this, (OsclAny *)iOutBufMemoryPool);
    iNumOutstandingOutputBuffers++;

    for (ii = 0;ii < iNumOutputBuffers;ii++)
    {
        if (output_buf == out_ctrl_struct_ptr[ii])
        {
            break;
        }
    }

    if (ii == iNumOutputBuffers)
        return false;

    output_buf->pBufHdr = (OMX_BUFFERHEADERTYPE *)out_buff_hdr_ptr[ii];

    output_buf->pBufHdr->nFilledLen = 0; // make sure you tell OMX component buffer is empty
    output_buf->pBufHdr->nOffset = 0;
    output_buf->pBufHdr->pAppPrivate = output_buf; // set pAppPrivate to be pointer to output_buf
    // (this is context for future release of this buffer to the mempool)
    // this was done during buffer creation, but still repeat just in case

    output_buf->pBufHdr->nFlags = 0; // zero out the flags

    OMX_FillThisBuffer(iOMXDecoder, output_buf->pBufHdr);



    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "%s::SendOutputBufferToOMXComponent() Out", iName.Str()));

    return true;
}
////////////////////////////////////////////////////////////////////////////////
bool PVMFOMXBaseDecNode::SetDefaultCapabilityFlags()
{

    iIsOMXComponentMultiThreaded = true;

    iOMXComponentSupportsExternalOutputBufferAlloc = false;
    iOMXComponentSupportsExternalInputBufferAlloc = false;
    iOMXComponentSupportsMovableInputBuffers = false;
    iOMXComponentUsesNALStartCodes = true;
    iOMXComponentSupportsPartialFrames = false;
    iOMXComponentCanHandleIncompleteFrames = true;
    iOMXComponentUsesFullAVCFrames = false;

    return true;
}



bool PVMFOMXBaseDecNode::SendEOSBufferToOMXComponent()
{

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "%s::SendEOSBufferToOMXComponent() In", iName.Str()));


    // first of all, check if the component is running. EOS could be sent prior to component/decoder
    // even being initialized

    // returning false will ensure that the EOS will be sent downstream anyway without waiting for the
    // Component to respond
    if (iCurrentDecoderState != OMX_StateExecuting)
        return false;

    // get an input buffer. Without a buffer, no point in proceeding
    InputBufCtrlStruct *input_buf = NULL;
    int32 errcode = OsclErrNone;

    // we already checked that the number of buffers is OK, so we don't expect problems
    // try to get input buffer header
    OSCL_TRY(errcode, input_buf = (InputBufCtrlStruct *) iInBufMemoryPool->allocate(iInputAllocSize));
    if (OsclErrNone != errcode)
    {
        if (OsclErrNoResources == errcode)
        {

            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger,
                            PVLOGMSG_DEBUG, (0, "%s::SendEOSBufferToOMXComponent() No more buffers in the mempool - unexpected", iName.Str()));

            iInBufMemoryPool->notifyfreechunkavailable(*this, (OsclAny*) iInBufMemoryPool); // To signal when next deallocate() is called on mempool

            return false;
        }
        else
        {
            // Memory allocation for the pool failed
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "%s::SendEOSBufferToOMXComponent() Input mempool error", iName.Str()));


            SetState(EPVMFNodeError);
            ReportErrorEvent(PVMFErrNoMemory);
            return false;
        }

    }

    // keep track of buffers. When buffer is deallocated/released, the counter will be decremented
    iInBufMemoryPool->notifyfreechunkavailable(*this, (OsclAny*) iInBufMemoryPool);
    iNumOutstandingInputBuffers++;

    // in this case, no need to use input msg refcounter. Make sure its unbound
    (input_buf->pMediaData).Unbind();

    // THIS IS AN EMPTY BUFFER. FLAGS ARE THE ONLY IMPORTANT THING
    input_buf->pBufHdr->nFilledLen = 0;
    input_buf->pBufHdr->nOffset = 0;
    input_buf->pBufHdr->nTimeStamp = iEndOfDataTimestamp;

    // set ptr to input_buf structure for Context (for when the buffer is returned)
    input_buf->pBufHdr->pAppPrivate = (OMX_PTR) input_buf;

    // do not use Mark here (but init to NULL to prevent problems)
    input_buf->pBufHdr->hMarkTargetComponent = NULL;
    input_buf->pBufHdr->pMarkData = NULL;


    // init buffer flags
    input_buf->pBufHdr->nFlags = 0;

    input_buf->pBufHdr->nFlags |= OMX_BUFFERFLAG_ENDOFFRAME;
    // most importantly, set the EOS flag:
    input_buf->pBufHdr->nFlags |= OMX_BUFFERFLAG_EOS;

    // send buffer to component
    OMX_EmptyThisBuffer(iOMXDecoder, input_buf->pBufHdr);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "%s::SendEOSBufferToOMXComponent() Out", iName.Str()));

    return true;

}

// this method is called under certain conditions only if the node is doing partial frame assembly
OSCL_EXPORT_REF void PVMFOMXBaseDecNode::DropCurrentBufferUnderConstruction()
{
    if (iObtainNewInputBuffer == false)
    {
        if (iInputBufferUnderConstruction != NULL)
        {
            if (iInBufMemoryPool != NULL)
            {
                iInBufMemoryPool->deallocate((OsclAny *)iInputBufferUnderConstruction);
            }

            iInputBufferUnderConstruction = NULL;
        }
        iObtainNewInputBuffer = true;

    }
}

// this method is called under certain conditions only if the node is doing partial frame assembly
OSCL_EXPORT_REF void PVMFOMXBaseDecNode::SendIncompleteBufferUnderConstruction()
{

    // this should never be the case, but check anyway
    if (iInputBufferUnderConstruction != NULL)
    {
        // mark as end of frame (the actual end piece is missing)
        iInputBufferUnderConstruction->pBufHdr->nFlags |= OMX_BUFFERFLAG_ENDOFFRAME;

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "%s::SendIncompleteBufferUnderConstruction()  - Sending Incomplete Buffer 0x%x to OMX Component MARKER field set to %x, TS=%d", iName.Str(), iInputBufferUnderConstruction->pBufHdr->pBuffer, iInputBufferUnderConstruction->pBufHdr->nFlags, iInTimestamp));

        OMX_EmptyThisBuffer(iOMXDecoder, iInputBufferUnderConstruction->pBufHdr);


        iInputBufferUnderConstruction = NULL;
        iObtainNewInputBuffer = true;

    }
}

OSCL_EXPORT_REF bool PVMFOMXBaseDecNode::SendInputBufferToOMXComponent()
{

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "%s::SendInputBufferToOMXComponent() In", iName.Str()));


    // first need to take care of  missing packets if node is assembling partial frames.
    // The action depends whether the component (I) can handle incomplete frames/NALs or (II) cannot handle incomplete frames/NALs
    if (!iOMXComponentSupportsPartialFrames)
    {

        // there are 4 cases after receiving a media msg and realizing there were missing packet(s):

        // a) TS remains the same - i.e. missing 1 or more pieces in the middle of the same frame
        //		 I) basically ignore  - keep assembling the same frame  (middle will be missing)
        //		II) drop current buffer, drop msgs until next msg with marker bit arrives


        // b) TS is different than previous frame. Previous frame was sent OK (had marker bit).
        //				New frame assembly has not started yet. one or more pieces are missing from
        //				the beginning of the frame
        //	  	 I) basically ignore - get a new buffer and start assembling new frame (beginning will be missing)
        //		II) no buffer to drop, but keep dropping msgs until next msg with marker bit arrives

        // c) TS is different than previous frame. Frame assembly has started (we were in the middle of a frame)
        //		but only 1 piece is missing => We know that the missing frame must have had the marker bit

        //		 I) send out current buffer (last piece will be missing), get a new buffer and start assembling new frame (which is OK)
        //		II) just drop current buffer. Get a new buffer and start assembling new frame (no need to wait for marker bit)

        // d) TS is different than previous frame. Frame assembly has started ( we were in the middle of a frame)
        //		multiple pieces are missing => The last piece of the frame with the marker bit is missing for sure, but
        //		there could be also other frames missing or the beginning of the next frame is missing etc.

        //		 I) send out current bufer (last piece will be missing). Get a new buffer and start assembling new frame (beginning COULD BE missing as well)
        //		II) drop current buffer. Keep dropping msgs until next msg with marker bit arrives


        // extract info from the media message

        uint32 current_msg_seq_num = iDataIn->getSeqNum();
        uint32 current_msg_ts = iDataIn->getTimestamp();
        uint32 current_msg_marker;
        if (iSetMarkerBitForEveryFrag == true) // PV AVC case
        {
            current_msg_marker = iDataIn->getMarkerInfo() & PVMF_MEDIA_DATA_MARKER_INFO_END_OF_NAL_BIT;
            current_msg_marker |= iDataIn->getMarkerInfo() & PVMF_MEDIA_DATA_MARKER_INFO_M_BIT;
        }
        else
        {
            current_msg_marker = iDataIn->getMarkerInfo() & PVMF_MEDIA_DATA_MARKER_INFO_M_BIT;
        }

        // check if this is the very first data msg
        if (iFirstDataMsgAfterBOS)
        {
            iFirstDataMsgAfterBOS = false;
            //init the sequence number & ts to make sure dropping logic does not kick in
            iInPacketSeqNum = current_msg_seq_num - 1;
            iInTimestamp = current_msg_ts - 10;
        }


        // first check if we need to keep dropping msgs
        if (iKeepDroppingMsgsUntilMarkerBit)
        {
            // drop this message
            iDataIn.Unbind();

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "%s::SendInputBufferToOMXComponent() Dropping input msg with seqnum %d until marker bit", iName.Str(), current_msg_seq_num));

            //if msg has marker bit, stop dropping msgs
            if ((current_msg_marker != 0 && !iOMXComponentUsesFullAVCFrames) || // frame or NAL boundaries
                    ((current_msg_marker & PVMF_MEDIA_DATA_MARKER_INFO_M_BIT) && iOMXComponentUsesFullAVCFrames)) // only frame boundaries
            {
                iKeepDroppingMsgsUntilMarkerBit = false;
                // also remember the sequence number & timestamp so that we have reference
                iInPacketSeqNum = current_msg_seq_num;
                iInTimestamp = current_msg_ts;
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "%s::SendInputBufferToOMXComponent() Input msg with seqnum %d has marker bit set. Stop dropping msgs", iName.Str(), current_msg_seq_num));

            }
            return true;
        }

        // is there something missing?
        // compare current and saved sequence number - difference should be exactly 1
        //	if it is more, there is something missing
        if ((current_msg_seq_num - iInPacketSeqNum) > 1)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "%s::SendInputBufferToOMXComponent() - MISSING PACKET DETECTED. Input msg with seqnum %d, TS=%d. Previous seqnum: %d, Previous TS: %d", iName.Str(), current_msg_seq_num, iInPacketSeqNum, current_msg_ts, iInTimestamp));

            // find out which case it is by comparing TS
            if (current_msg_ts == iInTimestamp)
            {

                // this is CASE a)
                // same ts, i.e. pieces are missing from the middle of the current frame
                if (!iOMXComponentCanHandleIncompleteFrames)
                {
                    // drop current buffer, drop msgs until you hit msg with marker bit
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                    (0, "%s::SendInputBufferToOMXComponent() - Drop current buffer under construction. Keep dropping msgs until marker bit", iName.Str()));

                    DropCurrentBufferUnderConstruction();
                    iKeepDroppingMsgsUntilMarkerBit = true;
                }
                else
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                    (0, "%s::SendInputBufferToOMXComponent() - Continue processing", iName.Str()));

                }
            }
            else // new ts and old ts are different
            {
                //  are we at the beginning of the new frame assembly?
                if (iObtainNewInputBuffer)
                {
                    // CASE b)
                    // i.e. we sent out previous frame, but have not started assembling a new frame. Pieces are missing from the beginning
                    if (!iOMXComponentCanHandleIncompleteFrames)
                    {
                        // there is no current buffer to drop, but drop msgs until you hit msg with marker bit
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                        (0, "%s::SendInputBufferToOMXComponent() - No current buffer under construction. Keep dropping msgs until marker bit", iName.Str()));

                        iKeepDroppingMsgsUntilMarkerBit = true;
                    }
                    else
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                        (0, "%s::SendInputBufferToOMXComponent() - Continue processing", iName.Str()));
                    }
                }
                else	// no, we are in the middle of a frame assembly, but new ts is different
                {
                    // is only 1 msg missing?
                    if ((current_msg_seq_num - iInPacketSeqNum) == 2)
                    {
                        // CASE c)
                        // only the last piece of the previous frame is missing
                        if (iOMXComponentCanHandleIncompleteFrames)
                        {
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                            (0, "%s::SendInputBufferToOMXComponent() - Send incomplete buffer under construction. Start assembling new frame", iName.Str()));

                            SendIncompleteBufferUnderConstruction();
                        }
                        else
                        {
                            // drop current frame only, but no need to wait until next marker bit.
                            // start assembling new frame
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                            (0, "%s::SendInputBufferToOMXComponent() - Drop current buffer under construction. It's OK to start assembling new frame. Only 1 packet is missing", iName.Str()));

                            DropCurrentBufferUnderConstruction();
                        }
                    }
                    else
                    {
                        // CASE d)
                        // (multiple) final piece(s) of the previous frame are missing and possibly pieces at the
                        // beginning of a new frame are also missing
                        if (iOMXComponentCanHandleIncompleteFrames)
                        {
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                            (0, "%s::SendInputBufferToOMXComponent() - Send incomplete buffer under construction. Start assembling new frame (potentially damaged)", iName.Str()));

                            SendIncompleteBufferUnderConstruction();
                        }
                        else
                        {
                            // drop current frame. start assembling new frame, but first keep dropping
                            // until you hit msg with marker bit.
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                            (0, "%s::SendInputBufferToOMXComponent() - Drop current buffer under construction. Keep dropping msgs until marker bit", iName.Str()));

                            DropCurrentBufferUnderConstruction();
                            iKeepDroppingMsgsUntilMarkerBit = true;
                        }
                    }
                }// end of if(obtainNewInputBuffer)/else
            }// end of if(curr_msg_ts == iInTimestamp)
        }//end of if(deltaseqnum>1)/else

        // check if we need to keep dropping msgs
        if (iKeepDroppingMsgsUntilMarkerBit)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "%s::SendInputBufferToOMXComponent() Dropping input msg with seqnum %d until marker bit", iName.Str(), current_msg_seq_num));

            // drop this message
            iDataIn.Unbind();

            //if msg has marker bit, stop dropping msgs
            if ((current_msg_marker != 0 && !iOMXComponentUsesFullAVCFrames) || // frame or NAL boundaries
                    ((current_msg_marker & PVMF_MEDIA_DATA_MARKER_INFO_M_BIT) && iOMXComponentUsesFullAVCFrames)) // only frame boundaries
            {
                iKeepDroppingMsgsUntilMarkerBit = false;
                // also remember the sequence number & timestamp so that we have reference
                iInPacketSeqNum = current_msg_seq_num;
                iInTimestamp = current_msg_ts;
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "%s::SendInputBufferToOMXComponent() Input msg with seqnum %d has marker bit set. Stop dropping msgs", iName.Str(), current_msg_seq_num));

            }
            return true;
        }

    }// end of if/else (iOMXSUpportsPartialFrames)

    InputBufCtrlStruct *input_buf = NULL;
    int32 errcode = OsclErrNone;
    uint32 ii;

// NOTE: a) if NAL start codes must be inserted i.e. iOMXComponentUsesNALStartCodess is TRUE, then iOMXComponentSupportsMovableInputBuffers must be set to FALSE.
//		 b) if iOMXComponentSupportsPartialFrames is FALSE, then iOMXComponentSupportsMovableInputBuffers must be FALSE as well
//		 c) if iOMXCOmponentSupportsPartialFrames is FALSE, and the input frame/NAL size is larger than the buffer size, the frame/NAL is discarded

    do
    {
        // do loop to loop over all fragments
        // first of all , get an input buffer. Without a buffer, no point in proceeding
        if (iObtainNewInputBuffer == true) // if partial frames are being reconstructed, we may be copying data into
            //existing buffer, so we don't need the new buffer
        {
            // try to get input buffer header
            OsclAny* temp;
            errcode = AllocateChunkFromMemPool(temp, iInBufMemoryPool, iInputAllocSize);
            input_buf = (InputBufCtrlStruct*) temp;
            if (OsclErrNone != errcode)
            {
                if (OsclErrNoResources == errcode)
                {

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger,
                                    PVLOGMSG_DEBUG, (0, "%s::SendInputBufferToOMXComponent() No more buffers in the mempool", iName.Str()));

                    iInBufMemoryPool->notifyfreechunkavailable(*this, (OsclAny*) iInBufMemoryPool); // To signal when next deallocate() is called on mempool

                    return false;
                }
                else
                {
                    // Memory allocation for the pool failed
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "%s::SendInputBufferToOMXComponent() Input mempool error", iName.Str()));


                    SetState(EPVMFNodeError);
                    ReportErrorEvent(PVMFErrNoMemory);
                    return false;
                }

            }

            // keep track of buffers. When buffer is deallocated/released, the counter will be decremented
            iInBufMemoryPool->notifyfreechunkavailable(*this, (OsclAny*) iInBufMemoryPool);
            iNumOutstandingInputBuffers++;

            for (ii = 0;ii < iNumInputBuffers;ii++)
            {
                if (input_buf == in_ctrl_struct_ptr[ii])
                {
                    break;
                }
            }

            if (ii == iNumInputBuffers)
                return false;

            input_buf->pBufHdr = (OMX_BUFFERHEADERTYPE *)in_buff_hdr_ptr[ii];

            // Now we have the buffer header (i.e. a buffer) to send to component:
            // Depending on OMX component capabilities, either pass the input msg fragment(s) directly
            //	into OMX component without copying (and update the input msg refcount)
            //	or memcopy the content of input msg memfrag(s) into OMX component allocated buffers
            input_buf->pBufHdr->nFilledLen = 0; // init this for now
            // save this in a class member
            iInputBufferUnderConstruction = input_buf;
            // set flags
            if (iOMXComponentSupportsPartialFrames == true)
            {
                // if partial frames can be sent, then send them
                // but we'll always need the new buffer for the new fragment
                iObtainNewInputBuffer = true;
            }
            else
            {
                // if we need to assemble partial frames, then obtain a new buffer
                // only after assembling the partial frame
                iObtainNewInputBuffer = false;
            }

            if (iOMXComponentUsesFullAVCFrames)
            {
                // reset NAL counters
                oscl_memset(iNALSizeArray, 0, sizeof(uint32) * iNALCount);
                iNALCount = 0;
            }

            iIncompleteFrame = false;
            iFirstPieceOfPartialFrame = true;
        }
        else
        {
            input_buf = iInputBufferUnderConstruction;
        }

        // When copying content, a special case is when the input fragment is larger than the buffer and has to
        //	be fragmented here and broken over 2 or more buffers. Potential problem with available buffers etc.

        // if this is the first fragment in a new message, extract some info:
        if (iCurrFragNum == 0)
        {

            // NOTE: SeqNum differ in Codec and in Node because of the fact that
            // one msg can contain multiple fragments that are sent to the codec as
            // separate buffers. Node tracks msgs and codec tracks even separate fragments

            iCodecSeqNum += (iDataIn->getSeqNum() - iInPacketSeqNum); // increment the codec seq. # by the same
            // amount that the input seq. number increased

            iInPacketSeqNum = iDataIn->getSeqNum(); // remember input sequence number
            iInTimestamp = iDataIn->getTimestamp();
            iInDuration = iDataIn->getDuration();
            iInNumFrags = iDataIn->getNumFragments();

            if (iSetMarkerBitForEveryFrag == true) // PV AVC case
            {
                iCurrentMsgMarkerBit = iDataIn->getMarkerInfo() & PVMF_MEDIA_DATA_MARKER_INFO_END_OF_NAL_BIT;
                iCurrentMsgMarkerBit |= iDataIn->getMarkerInfo() & PVMF_MEDIA_DATA_MARKER_INFO_M_BIT;
            }
            else
            {
                iCurrentMsgMarkerBit = iDataIn->getMarkerInfo() & PVMF_MEDIA_DATA_MARKER_INFO_M_BIT;
            }

            //Force marker bit for AMR streaming formats (marker bit may not be set even though full frames are present)
            if (iInPort && (
                        (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_AMR) ||
                        (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_AMRWB)
                    ))
            {
                iCurrentMsgMarkerBit = PVMF_MEDIA_DATA_MARKER_INFO_M_BIT;
            }


            // logging info:
            if (iDataIn->getNumFragments() > 1)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "%s::SendInputBufferToOMXComponent() - New msg has MULTI-FRAGMENTS", iName.Str()));
            }

            if (!(iDataIn->getMarkerInfo() & PVMF_MEDIA_DATA_MARKER_INFO_M_BIT) && !(iDataIn->getMarkerInfo() & PVMF_MEDIA_DATA_MARKER_INFO_END_OF_NAL_BIT))
            {

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "%s::SendInputBufferToOMXComponent() - New msg has NO MARKER BIT", iName.Str()));
            }
        }


        // get a memfrag from the message
        OsclRefCounterMemFrag frag;
        iDataIn->getMediaFragment(iCurrFragNum, frag);


        if (iOMXComponentSupportsMovableInputBuffers)
        {
            // no copying required

            // increment the RefCounter of the message associated with the mem fragment/buffer
            // when sending this buffer to OMX component. (When getting the buffer back, the refcounter
            // will be decremented. Thus, when the last fragment is returned, the input mssage is finally released

            iDataIn.GetRefCounter()->addRef();

            // associate the buffer ctrl structure with the message ref counter and ptr
            input_buf->pMediaData = PVMFSharedMediaDataPtr(iDataIn.GetRep(), iDataIn.GetRefCounter());


            // set pointer to the data, length, offset
            input_buf->pBufHdr->pBuffer = (uint8 *)frag.getMemFragPtr();
            input_buf->pBufHdr->nFilledLen = frag.getMemFragSize();

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "%s::SendInputBufferToOMXComponent() - Buffer 0x%x of size %d, %d frag out of tot. %d, TS=%d", iName.Str(), input_buf->pBufHdr->pBuffer, frag.getMemFragSize(), iCurrFragNum + 1, iDataIn->getNumFragments(), iInTimestamp));

            iCurrFragNum++; // increment fragment number and move on to the next
            iIsNewDataFragment = true; // update the flag

        }
        else
        {

            // in this case, no need to use input msg refcounter, each buffer fragment is copied over and treated separately
            (input_buf->pMediaData).Unbind();

            if (iOMXComponentUsesNALStartCodes == true && iFirstPieceOfPartialFrame == true)
            {
                oscl_memcpy(input_buf->pBufHdr->pBuffer + input_buf->pBufHdr->nFilledLen,
                            (void *) NAL_START_CODE,
                            NAL_START_CODE_SIZE);
                input_buf->pBufHdr->nFilledLen += NAL_START_CODE_SIZE;
                iFirstPieceOfPartialFrame = false;

            }

            // is this a new data fragment or are we still working on separating the old one?
            if (iIsNewDataFragment == true)
            {
                //  if fragment size is larger than the buffer size,
                //	need to break up the fragment even further into smaller chunks

                // init variables needed for fragment separation
                iCopyPosition = 0;
                iFragmentSizeRemainingToCopy  = frag.getMemFragSize();

            }

            // can the remaining fragment fit into the buffer?
            uint32 bytes_remaining_in_buffer;

            if (iOMXComponentUsesFullAVCFrames && !iOMXComponentUsesNALStartCodes)
            {
                // need to keep to account the extra data appended at the end of the buffer
                int32 temp = (input_buf->pBufHdr->nAllocLen - input_buf->pBufHdr->nFilledLen - (20 + 4 * (iNALCount + 1) + 20 + 6));//(sizeOfExtraDataStruct_NAL + sizeOfExTraData + sizeOfExtraDataStruct_terminator + padding);
                bytes_remaining_in_buffer = (uint32)((temp < 0) ? 0 : temp);
            }
            else
            {
                bytes_remaining_in_buffer = (input_buf->pBufHdr->nAllocLen - input_buf->pBufHdr->nFilledLen);
            }

            if (iFragmentSizeRemainingToCopy <= bytes_remaining_in_buffer)
            {

                oscl_memcpy(input_buf->pBufHdr->pBuffer + input_buf->pBufHdr->nFilledLen,
                            (void *)((uint8 *)frag.getMemFragPtr() + iCopyPosition),
                            iFragmentSizeRemainingToCopy);

                input_buf->pBufHdr->nFilledLen += iFragmentSizeRemainingToCopy;

                if (iOMXComponentUsesFullAVCFrames)
                {
                    iNALSizeArray[iNALCount] += iFragmentSizeRemainingToCopy;

                    if ((iCurrentMsgMarkerBit & PVMF_MEDIA_DATA_MARKER_INFO_END_OF_NAL_BIT) &&
                            (1 == iDataIn->getNumFragments()))
                    {
                        // streaming case (and 1 nal per frame file format case)
                        iNALCount++;
                        // we have a full NAL now, so insert a start code (if it needs it) for the next NAL, the next time through the loop
                        iFirstPieceOfPartialFrame = true;
                    }
                    else if (iDataIn->getNumFragments() > 1)
                    {
                        // multiple nals per frame file format case
                        iNALCount = iCurrFragNum + 1;
                        // we have a full NAL now, so insert a start code for the next NAL, the next time through the loop
                        iFirstPieceOfPartialFrame = true;
                    }
                }

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "%s::SendInputBufferToOMXComponent() - Copied %d bytes of fragment %d out of %d into buffer 0x%x of size %d, TS=%d ", iName.Str(), iFragmentSizeRemainingToCopy, iCurrFragNum + 1, iDataIn->getNumFragments(), input_buf->pBufHdr->pBuffer, input_buf->pBufHdr->nFilledLen, iInTimestamp));

                iCopyPosition += iFragmentSizeRemainingToCopy;
                iFragmentSizeRemainingToCopy = 0;

                iIsNewDataFragment = true; // done with this fragment. Get a new one
                iCurrFragNum++;

            }
            else
            {
                // copy as much as you can of the current fragment into the current buffer
                if (bytes_remaining_in_buffer > 0)
                {
                    oscl_memcpy(input_buf->pBufHdr->pBuffer + input_buf->pBufHdr->nFilledLen,
                                (void *)((uint8 *)frag.getMemFragPtr() + iCopyPosition),
                                bytes_remaining_in_buffer);

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                    (0, "%s::SendInputBufferToOMXComponent() - Copied %d bytes of fragment %d out of %d into buffer 0x%x of size %d, TS=%d", iName.Str(), bytes_remaining_in_buffer, iCurrFragNum + 1, iDataIn->getNumFragments(), input_buf->pBufHdr->pBuffer, input_buf->pBufHdr->nFilledLen, iInTimestamp));
                }

                input_buf->pBufHdr->nFilledLen += bytes_remaining_in_buffer;

                if (iOMXComponentUsesFullAVCFrames && (bytes_remaining_in_buffer > 0))
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                    (0, "%s::SendInputBufferToOMXComponent() - Reconstructing partial frame (iOMXComponentUsesFullAVCFrames) - more data cannot fit in buffer 0x%x, TS=%d.Skipping data.", iName.Str(), input_buf->pBufHdr->pBuffer, iInTimestamp));

                    iNALSizeArray[iNALCount] += bytes_remaining_in_buffer;

                    // increment NAL count regardless if market bit is present or not since it is a fragment
                    iNALCount++;
                }

                iCopyPosition += bytes_remaining_in_buffer; // move current position within fragment forward
                iFragmentSizeRemainingToCopy -= bytes_remaining_in_buffer;
                iIncompleteFrame = true;
                iIsNewDataFragment = false; // set the flag to indicate we're still working on the "old" fragment
                if (!iOMXComponentSupportsPartialFrames)
                {
                    // if partial frames are not supported, and data cannot fit into the buffer, i.e. the buffer is full at this point
                    // simply go through remaining fragments if they exist and "drop" them
                    // i.e. send what data is alrady copied in the buffer and ingore the rest
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                    (0, "%s::SendInputBufferToOMXComponent() - Reconstructing partial frame - more data cannot fit in buffer 0x%x, TS=%d.Skipping data.", iName.Str(), input_buf->pBufHdr->pBuffer, iInTimestamp));

                    iIsNewDataFragment = true; // done with this fragment, get a new one
                    iCurrFragNum++;
                }
            }

        }


        // set buffer fields (this is the same regardless of whether the input is movable or not
        input_buf->pBufHdr->nOffset = 0;
        input_buf->pBufHdr->nTimeStamp = iInTimestamp;

        // set ptr to input_buf structure for Context (for when the buffer is returned)
        input_buf->pBufHdr->pAppPrivate = (OMX_PTR) input_buf;

        // do not use Mark here (but init to NULL to prevent problems)
        input_buf->pBufHdr->hMarkTargetComponent = NULL;
        input_buf->pBufHdr->pMarkData = NULL;


        // init buffer flags
        input_buf->pBufHdr->nFlags = 0;

        // set marker bit on or off
        // Audio:
        // a) AAC - file playback - each fragment is a complete frame (1 msg may contain multiple fragments/frames)
        //    AAC - streaming	- 1 msg may contain a partial frame, but LATM parser will assemble a full frame
        //						(when LATM parser is done, we attach a marker bit to the data it produces)

        // b) AMR - file playback - each msg is N whole frames (marker bit is always set)
        //    AMR - streaming   - each msg is N whole frames (marker bit is missing from incoming msgs -set it here)

        // c) MP3 - file playback - 1 msg is N whole frames
        //
        // Video:
        // a) AVC - file playback - each fragment is a complete NAL (1 or more frags i.e. NALs per msg)
        //    AVC - streaming	- 1 msg contains 1 full NAL or a portion of a NAL
        // NAL may be broken up over multiple msgs. Frags are not allowed in streaming
        // b) M4V - file playback - each msg is 1 frame
        //    M4V - streaming   - 1 frame may be broken up into multiple messages and fragments

        // c) WMV - file playback - 1 frame is 1 msg
        //    WMV - streaming     - 1 frame may be broken up into multiple messages and fragments


        if (iSetMarkerBitForEveryFrag == true)
        {


            if (iIsNewDataFragment)
            {
                if ((iDataIn->getNumFragments() > 1))
                {
                    // if more than 1 fragment in the message and we have not broken it up
                    //(i.e. this is the last piece of a broken up piece), put marker bit on it unconditionally
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                    (0, "%s::SendInputBufferToOMXComponent() - END OF FRAGMENT - Multifragmented msg AVC case, Buffer 0x%x MARKER bit set to 1", iName.Str(), input_buf->pBufHdr->pBuffer));

                    if (!iOMXComponentUsesFullAVCFrames)
                    {
                        // NAL mode, (uses OMX_BUFFERFLAG_ENDOFFRAME flag to mark end of NAL instead of end of frame)
                        // once NAL is complete, make sure you send it and obtain new buffer
                        input_buf->pBufHdr->nFlags |= OMX_BUFFERFLAG_ENDOFFRAME;
                        iObtainNewInputBuffer = true;
                    }
                    else if (iCurrentMsgMarkerBit & PVMF_MEDIA_DATA_MARKER_INFO_M_BIT)
                    {
                        // frame mode (send out full AVC frames)
                        if (iCurrFragNum == iDataIn->getNumFragments())
                        {
                            input_buf->pBufHdr->nFlags |= OMX_BUFFERFLAG_ENDOFFRAME;
                            iObtainNewInputBuffer = true;
                        }
                    }
                }
                else if ((iDataIn->getNumFragments() == 1))
                {
                    // this is (the last piece of broken up by us) single-fragmented message. This can be a piece of a NAL (streaming) or a full NAL (file )
                    // apply marker bit if the message carries one
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                    (0, "%s::SendInputBufferToOMXComponent() - END OF FRAGMENT - Buffer 0x%x MARKER bit set to %d", iName.Str(), input_buf->pBufHdr->pBuffer, iCurrentMsgMarkerBit));

                    // if either PVMF_MEDIA_DATA_MARKER_INFO_END_OF_NAL_BIT or PVMF_MEDIA_DATA_MARKER_INFO_M_BIT
                    // and we're not in "frame" mode, then set OMX_BUFFERFLAG_ENDOFFRAME
                    if (iCurrentMsgMarkerBit && !iOMXComponentUsesFullAVCFrames)
                    {
                        // NAL mode, (uses OMX_BUFFERFLAG_ENDOFFRAME flag to mark end of NAL instead of end of frame)
                        // once NAL is complete, make sure you send it and obtain new buffer
                        input_buf->pBufHdr->nFlags |= OMX_BUFFERFLAG_ENDOFFRAME;
                        iObtainNewInputBuffer = true;
                    }
                    else if (iCurrentMsgMarkerBit & PVMF_MEDIA_DATA_MARKER_INFO_M_BIT)
                    {
                        // frame mode
                        input_buf->pBufHdr->nFlags |= OMX_BUFFERFLAG_ENDOFFRAME;
                        iObtainNewInputBuffer = true;
                    }
                }
            }
            else
            {
                // we are separating fragments that are too big, i.e. bigger than
                // what 1 buffer can hold, this fragment Can NEVER have marker bit
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "%s::SendInputBufferToOMXComponent() - NOT END OF FRAGMENT - Buffer 0x%x MARKER bit set to 0", iName.Str(), input_buf->pBufHdr->pBuffer));

            }
        }
        else
        {
            // "normal" case, i.e. only fragments at ends of msgs may have marker bit set
            //					fragments in the middle of a message never have marker bit set
            // there is also a (slight) possibility we broke up the fragment into more fragments
            //	because they can't fit into input buffer. In this case, make sure you apply
            //	the marker bit (if necessary) only to the very last piece of the very last fragment

            // for all other cases, clear the marker bit flag for the buffer
            if ((iCurrFragNum == iDataIn->getNumFragments()) && iIsNewDataFragment)
            {
                // if all the fragments have been exhausted, and this is the last piece
                // of the (possibly broken up) last fragment

                // use the marker bit from the end of message
                if (iCurrentMsgMarkerBit)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                    (0, "%s::SendInputBufferToOMXComponent() - END OF MESSAGE - Buffer 0x%x MARKER bit set to 1, TS=%d", iName.Str(), input_buf->pBufHdr->pBuffer, iInTimestamp));

                    input_buf->pBufHdr->nFlags |= OMX_BUFFERFLAG_ENDOFFRAME;
                    // once frame is complete, make sure you send it and obtain new buffer

                    iObtainNewInputBuffer = true;
                }
                else
                {

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                    (0, "%s::SendInputBufferToOMXComponent() - END OF MESSAGE - Buffer 0x%x MARKER bit set to 0, TS=%d", iName.Str(), input_buf->pBufHdr->pBuffer, iInTimestamp));
                }
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "%s::SendInputBufferToOMXComponent() - NOT END OF MESSAGE - Buffer 0x%x MARKER bit set to 0, TS=%d", iName.Str(), input_buf->pBufHdr->pBuffer, iInTimestamp));
            }


        }// end of else(setmarkerbitforeveryfrag)


        // set the key frame flag if necessary (mark every fragment that belongs to it)
        if (iDataIn->getMarkerInfo() & PVMF_MEDIA_DATA_MARKER_INFO_RANDOM_ACCESS_POINT_BIT)
            input_buf->pBufHdr->nFlags |= OMX_BUFFERFLAG_SYNCFRAME;

        if (iObtainNewInputBuffer == true)
        {
            // if partial frames are supported, this flag will always be set
            // if partial frames are not supported, this flag will be set only
            // if the partial frame/NAL has been assembled, so we can send it

            // if incomplete frames are not supported then let go of this buffer, and move on
            if (iIncompleteFrame && !iOMXComponentCanHandleIncompleteFrames)
            {
                DropCurrentBufferUnderConstruction();
            }
            else
            {
                // append extra data for "frame" mode
                if (iOMXComponentUsesFullAVCFrames && !iOMXComponentUsesNALStartCodes)
                {
                    if (!AppendExtraDataToBuffer(input_buf, (OMX_EXTRADATATYPE) OMX_ExtraDataNALSizeArray, (uint8*) iNALSizeArray, 4 * iNALCount))
                    {
                        // if AppendExtraDataToBuffer returns false, that means there wasn't enough room to write the data, so drop the buffer
                        DropCurrentBufferUnderConstruction();
                    }
                }

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "%s::SendInputBufferToOMXComponent()  - Sending Buffer 0x%x to OMX Component MARKER field set to %x, TS=%d", iName.Str(), input_buf->pBufHdr->pBuffer, input_buf->pBufHdr->nFlags, iInTimestamp));

                OMX_EmptyThisBuffer(iOMXDecoder, input_buf->pBufHdr);
                iInputBufferUnderConstruction = NULL; // this buffer is gone to OMX component now
            }
        }

        // if we sent all fragments to OMX component, decouple the input message from iDataIn
        // Input message is "decoupled", so that we can get a new message for processing into iDataIn
        //	However, the actual message is released completely to upstream mempool once all of its fragments
        //	are returned by the OMX component

        if (iCurrFragNum == iDataIn->getNumFragments())
        {
            iDataIn.Unbind();

        }


    }
    while (iCurrFragNum < iInNumFrags); //iDataIn->getNumFragments());

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "%s::SendInputBufferToOMXComponent() Out", iName.Str()));

    return true;

}

/////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF bool PVMFOMXBaseDecNode::AppendExtraDataToBuffer(InputBufCtrlStruct* aInputBuffer,
        OMX_EXTRADATATYPE aType,
        uint8* aExtraData,
        uint8 aDataLength)

{
    // This function is used to append AVC NAL info to the buffer using the OMX_EXTRADATA_TYPE structure, when
    // a component requires buffers with full AVC frames rather than just NALs
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "%s::AppendExtraDataToBuffer() In", iName.Str()));


    if ((aType != OMX_ExtraDataNone) && (aExtraData != NULL) && (aInputBuffer->pBufHdr->pBuffer != NULL))
    {
        const uint32 sizeOfExtraDataStruct = 20; // 20 is the number of bytes for the OMX_OTHER_EXTRADATATYPE structure (minus the data hint member)

        OMX_OTHER_EXTRADATATYPE extra;
        OMX_OTHER_EXTRADATATYPE terminator;

        CONFIG_SIZE_AND_VERSION(extra);
        CONFIG_SIZE_AND_VERSION(terminator);

        extra.nPortIndex = iInputPortIndex;
        terminator.nPortIndex = iInputPortIndex;

        extra.eType = aType;
        extra.nSize = (sizeOfExtraDataStruct + aDataLength + 3) & ~3; // size + padding for byte alignment
        extra.nDataSize = aDataLength;

        // fill in fields for terminator
        terminator.eType = OMX_ExtraDataNone;
        terminator.nDataSize = 0;

        // make sure there is enough room in the buffer
        if (aInputBuffer->pBufHdr->nAllocLen < (aInputBuffer->pBufHdr->nFilledLen + sizeOfExtraDataStruct + aDataLength + terminator.nSize + 6))
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "%s::AppendExtraDataToBuffer()  - Error (not enough room in buffer) appending extra data to Buffer 0x%x to OMX Component, TS=%d", iName.Str(), aInputBuffer->pBufHdr->pBuffer, iInTimestamp));

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "%s::AppendExtraDataToBuffer() Out", iName.Str()));

            return false;
        }

        // copy extra data into buffer
        // need to align to 4 bytes
        OMX_U8* buffer = aInputBuffer->pBufHdr->pBuffer + aInputBuffer->pBufHdr->nOffset + aInputBuffer->pBufHdr->nFilledLen;
        buffer = (OMX_U8*)(((OMX_U32) buffer + 3) & ~3);

        oscl_memcpy(buffer, &extra, sizeOfExtraDataStruct);
        oscl_memcpy(buffer + sizeOfExtraDataStruct, aExtraData, aDataLength);
        buffer += extra.nSize;

        oscl_memcpy(buffer, &terminator, terminator.nSize);

        // flag buffer
        aInputBuffer->pBufHdr->nFlags |= OMX_BUFFERFLAG_EXTRADATA;

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "%s::AppendExtraDataToBuffer()  - Appending extra data to Buffer 0x%x to OMX Component, TS=%d", iName.Str(), aInputBuffer->pBufHdr->pBuffer, iInTimestamp));

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "%s::AppendExtraDataToBuffer() Out", iName.Str()));

        return true;
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "%s::AppendExtraDataToBuffer() Out", iName.Str()));

        return false;
    }
}

/////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF bool PVMFOMXBaseDecNode::SendConfigBufferToOMXComponent(uint8 *initbuffer, uint32 initbufsize)

{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "%s::SendConfigBufferToOMXComponent() In", iName.Str()));


    // first of all , get an input buffer. Without a buffer, no point in proceeding
    InputBufCtrlStruct *input_buf = NULL;
    int32 errcode = OsclErrNone;

    // try to get input buffer header
    OSCL_TRY(errcode, input_buf = (InputBufCtrlStruct *) iInBufMemoryPool->allocate(iInputAllocSize));
    if (OsclErrNone != errcode)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "%s::SendConfigBufferToOMXComponent() Input buffer mempool problem -unexpected at init", iName.Str()));

        return false;
    }

    // Got a buffer OK
    // keep track of buffers. When buffer is deallocated/released, the counter will be decremented
    iInBufMemoryPool->notifyfreechunkavailable(*this, (OsclAny*) iInBufMemoryPool);
    iNumOutstandingInputBuffers++;

    input_buf->pBufHdr->nFilledLen = 0; //init this to 0

    // Now we have the buffer header (i.e. a buffer) to send to component:
    // Depending on OMX component capabilities, either pass the input msg fragment(s) directly
    //	into OMX component without copying (and update the input msg refcount)
    //	or memcopy the content of input msg memfrag(s) into OMX component allocated buffers

    // When copying content, a special case is when the input fragment is larger than the buffer and has to
    //	be fragmented here and broken over 2 or more buffers. Potential problem with available buffers etc.

    iCodecSeqNum += (iDataIn->getSeqNum() - iInPacketSeqNum); // increment the codec seq. # by the same
    // amount that the input seq. number increased

    iInPacketSeqNum = iDataIn->getSeqNum(); // remember input sequence number
    iInTimestamp = iDataIn->getTimestamp();
    iInDuration = iDataIn->getDuration();


    if (!(iDataIn->getMarkerInfo() & PVMF_MEDIA_DATA_MARKER_INFO_M_BIT))
    {

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "%s::SendConfigBufferToOMXComponent() - New msg has NO MARKER BIT", iName.Str()));
    }


    if (iOMXComponentSupportsMovableInputBuffers)
    {
        // no copying required

        // increment the RefCounter of the message associated with the mem fragment/buffer
        // when sending this buffer to OMX component. (When getting the buffer back, the refcounter
        // will be decremented. Thus, when the last fragment is returned, the input mssage is finally released

        iDataIn.GetRefCounter()->addRef();

        // associate the buffer ctrl structure with the message ref counter and ptr
        input_buf->pMediaData = PVMFSharedMediaDataPtr(iDataIn.GetRep(), iDataIn.GetRefCounter());


        // set pointer to the data, length, offset
        input_buf->pBufHdr->pBuffer = initbuffer;
        input_buf->pBufHdr->nFilledLen = initbufsize;

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "%s::SendConfigBufferToOMXComponent() - Config Buffer 0x%x of size %d", iName.Str(), initbuffer, initbufsize));

    }
    else
    {

        // in this case, no need to use input msg refcounter, each buffer fragment is copied over and treated separately
        (input_buf->pMediaData).Unbind();

        // we assume the buffer is large enough to fit the config data

        iCopyPosition = 0;
        iFragmentSizeRemainingToCopy  = initbufsize;

        if (iOMXComponentUsesNALStartCodes == true)
        {
            oscl_memcpy(input_buf->pBufHdr->pBuffer,
                        (void *) NAL_START_CODE,
                        NAL_START_CODE_SIZE);
            input_buf->pBufHdr->nFilledLen += NAL_START_CODE_SIZE;

        }

        // can the remaining fragment fit into the buffer?
        uint32 bytes_remaining_in_buffer = (input_buf->pBufHdr->nAllocLen - input_buf->pBufHdr->nFilledLen);

        if (iFragmentSizeRemainingToCopy <= bytes_remaining_in_buffer)
        {

            oscl_memcpy(input_buf->pBufHdr->pBuffer + input_buf->pBufHdr->nFilledLen,
                        (void *)(initbuffer + iCopyPosition),
                        iFragmentSizeRemainingToCopy);

            input_buf->pBufHdr->nFilledLen += iFragmentSizeRemainingToCopy;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "%s::SendConfigBufferToOMXComponent() - Copied %d bytes into buffer 0x%x of size %d", iName.Str(), iFragmentSizeRemainingToCopy, input_buf->pBufHdr->pBuffer, input_buf->pBufHdr->nFilledLen));

            iCopyPosition += iFragmentSizeRemainingToCopy;
            iFragmentSizeRemainingToCopy = 0;


        }
        else
        {

            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "%s::SendConfigBufferToOMXComponent() Config buffer too large problem -unexpected at init", iName.Str()));

            return false;
        }

    }


    // set buffer fields (this is the same regardless of whether the input is movable or not
    input_buf->pBufHdr->nOffset = 0;
    input_buf->pBufHdr->nTimeStamp = iInTimestamp;

    // set ptr to input_buf structure for Context (for when the buffer is returned)
    input_buf->pBufHdr->pAppPrivate = (OMX_PTR) input_buf;

    // do not use Mark here (but init to NULL to prevent problems)
    input_buf->pBufHdr->hMarkTargetComponent = NULL;
    input_buf->pBufHdr->pMarkData = NULL;


    // init buffer flags
    input_buf->pBufHdr->nFlags = 0;

    // set marker bit on

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "%s::SendConfigBufferToOMXComponent() - END OF FRAGMENT - Buffer 0x%x MARKER bit set to 1", iName.Str(), input_buf->pBufHdr->pBuffer));

    input_buf->pBufHdr->nFlags |= OMX_BUFFERFLAG_ENDOFFRAME;

    // set buffer flag indicating buffer contains codec config data
    input_buf->pBufHdr->nFlags |= OMX_BUFFERFLAG_CODECCONFIG;

    OMX_EmptyThisBuffer(iOMXDecoder, input_buf->pBufHdr);

    return true;

}



/////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF bool PVMFOMXBaseDecNode::CreateOutMemPool(uint32 num_buffers)
{

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "%s::CreateOutMemPool() start", iName.Str()));
    // In the case OMX component wants to allocate its own buffers,
    // mempool only contains OutputBufCtrlStructures (i.e. ptrs to buffer headers)
    // In case OMX component uses pre-allocated buffers (here),
    // mempool allocates OutputBufCtrlStructure (i.e. ptrs to buffer hdrs), followed by actual buffers

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "%s::CreateOutMemPool() Allocating output buffer header pointers", iName.Str()));

    iOutputAllocSize = oscl_mem_aligned_size((uint32)sizeof(OutputBufCtrlStruct));

    if (iOMXComponentSupportsExternalOutputBufferAlloc)
    {
        // In case of an external output buffer allocator interface, output buffer memory will be allocated
        // outside the node and hence iOutputAllocSize need not be incremented here

        if (NULL == ipExternalOutputBufferAllocatorInterface)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "%s::CreateOutMemPool() Allocating output buffers of size %d as well", iName.Str(), iOMXComponentOutputBufferSize));
            //pre-negotiated output buffer size
            iOutputAllocSize += iOMXComponentOutputBufferSize;
        }
    }

    // for media data wrapper
    if (iMediaDataMemPool)
    {
        iMediaDataMemPool->removeRef();
        iMediaDataMemPool = NULL;
    }

    if (iOutBufMemoryPool)
    {
        iOutBufMemoryPool->removeRef();
        iOutBufMemoryPool = NULL;
    }

    int32 leavecode = OsclErrNone;
    OSCL_TRY(leavecode, iOutBufMemoryPool = OSCL_NEW(OsclMemPoolFixedChunkAllocator, (num_buffers)););
    if (leavecode || iOutBufMemoryPool == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger,
                        PVLOGMSG_ERR, (0, "%s::CreateOutMemPool() Memory pool structure for output buffers failed to allocate", iName.Str()));
        return false;
    }



    // allocate a dummy buffer to actually create the mempool
    OsclAny *dummy_alloc = NULL; // this dummy buffer will be released at end of scope
    leavecode = OsclErrNone;
    OSCL_TRY(leavecode, dummy_alloc = iOutBufMemoryPool->allocate(iOutputAllocSize));
    if (leavecode || dummy_alloc == NULL)
    {

        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger,
                        PVLOGMSG_ERR, (0, "%s::CreateOutMemPool() Memory pool for output buffers failed to allocate", iName.Str()));
        return false;
    }
    iOutBufMemoryPool->deallocate(dummy_alloc);
    // init the counter
    iNumOutstandingOutputBuffers = 0;

    // allocate mempool for media data message wrapper
    leavecode = OsclErrNone;
    OSCL_TRY(leavecode, iMediaDataMemPool = OSCL_NEW(OsclMemPoolFixedChunkAllocator, (num_buffers, PVOMXBASEDEC_MEDIADATA_CHUNKSIZE)));
    if (leavecode || iMediaDataMemPool == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "%s::CreateOutMemPool() Media Data Buffer pool for output buffers failed to allocate", iName.Str()));
        return false;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "%s::CreateOutMemPool() done", iName.Str()));
    return true;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// Creates memory pool for input buffer management ///////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF bool PVMFOMXBaseDecNode::CreateInputMemPool(uint32 num_buffers)
{
    // 3 cases in order of preference and simplicity

    // Case 1 (buffers allocated upstream - no memcpy needed):
    //	PV OMX Component - We use buffers allocated outside the OMX node (i.e. allocated upstream)
    // Mempool contains InputBufCtrlStructures (ptrs to buffer headers and PMVFMediaData ptrs - to keep track of when to unbind input msgs)

    // NOTE:	in this case, when providing input buffers to OMX component,
    //			OMX_UseBuffer calls will provide some initial pointers and sizes of buffers, but these
    //			are dummy values. Actual buffer pointers and filled sizes will be obtained from the input msg fragments.
    //			The PV OMX component will use the buffers even if the ptrs differ from the ones during initialization
    //			3rd party OMX components can also use this case if they are capable of ignoring the actual buffer pointers in
    //			buffer header field (i.e. if after OMX_UseBuffer(...) call, they allow the ptr to actual buffer data to change at a later time

    // CASE 2 (buffers allocated in the node - memcpy needed)
    //			If 3rd party OMX component can use buffers allocated outside the OMX component, but it cannot
    //			change buffer ptr allocations dynamically (i.e. after initialization with OMX_UseBuffer call is complete)

    //		Mempool contains InputBufCtrlStructures (ptrs to buffer headers, PVMFMediaData ptrs to keep track of when to unbind input msgs) +
    //				actual buffers.
    //			NOTE: Data must be copied from input message into the local buffer before the buffer is given to the OMX component

    // CASE 3 (buffers allocated in the component - memcpy needed)
    //			If 3rd party OMX component must allocate its own buffers
    //			Mempool only contains InputBufCtrlStruct (ptrs to buffer headers + PMVFMediaData ptrs to keep track of when to unbind input msgs)
    //			NOTE: Data must be copied from input message into the local buffer before the buffer is given to the OMX component (like in case 2)

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "%s::CreateInputMemPool() start ", iName.Str()));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "%s::CreateInputMemPool() allocating buffer header pointers and shared media data ptrs ", iName.Str()));



    iInputAllocSize = oscl_mem_aligned_size((uint32) sizeof(InputBufCtrlStruct)); //aligned_size_buffer_header_ptr+aligned_size_media_data_ptr;

    // Need to allocate buffers in the node either if component supports external buffers buffers
    // but they are not movable

    if ((iOMXComponentSupportsExternalInputBufferAlloc && !iOMXComponentSupportsMovableInputBuffers))
    {
        //pre-negotiated input buffer size
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "%s::CreateOutMemPool() Allocating input buffers of size %d as well", iName.Str(), iOMXComponentInputBufferSize));

        iInputAllocSize += iOMXComponentInputBufferSize;
    }

    if (iInBufMemoryPool)
    {
        iInBufMemoryPool->removeRef();
        iInBufMemoryPool = NULL;
    }

    int32 leavecode = OsclErrNone;
    OSCL_TRY(leavecode, iInBufMemoryPool = OSCL_NEW(OsclMemPoolFixedChunkAllocator, (num_buffers)););
    if (leavecode || iInBufMemoryPool == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger,
                        PVLOGMSG_ERR, (0, "%s::CreateInputMemPool() Memory pool structure for input buffers failed to allocate", iName.Str()));
        return false;
    }
    // try to allocate a dummy buffer to actually create the mempool and allocate the needed memory
    // allocate a dummy buffer to actually create the mempool, this dummy buffer will be released at end of scope of this method
    OsclAny *dummy_alloc = NULL;
    leavecode = OsclErrNone;
    OSCL_TRY(leavecode, dummy_alloc = iInBufMemoryPool->allocate(iInputAllocSize));
    if (leavecode || dummy_alloc == NULL)
    {

        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger,
                        PVLOGMSG_ERR, (0, "%s::CreateInputMemPool() Memory pool for input buffers failed to allocate", iName.Str()));
        return false;
    }

    // init the counter
    iNumOutstandingInputBuffers = 0;


    iInputBufferToResendToComponent = NULL; // nothing to resend yet
    iInBufMemoryPool->deallocate(dummy_alloc);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "%s::CreateInputMemPool() done", iName.Str()));
    return true;
}
////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF bool PVMFOMXBaseDecNode::ProvideBuffersToComponent(OsclMemPoolFixedChunkAllocator *aMemPool, // allocator
        uint32 aAllocSize,	 // size to allocate from pool (hdr only or hdr+ buffer)
        uint32 aNumBuffers,    // number of buffers
        uint32 aActualBufferSize, // aactual buffer size
        uint32 aPortIndex,      // port idx
        bool aUseBufferOK,		// can component use OMX_UseBuffer or should it use OMX_AllocateBuffer
        bool	aIsThisInputBuffer		// is this input or output
                                                                  )
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "%s::ProvideBuffersToComponent() enter", iName.Str()));

    uint32 ii = 0;
    OMX_ERRORTYPE err = OMX_ErrorNone;
    OsclAny **ctrl_struct_ptr = NULL;	// temporary array to keep the addresses of buffer ctrl structures and buffers


    ctrl_struct_ptr = (OsclAny **) oscl_malloc(aNumBuffers * sizeof(OsclAny *));
    if (ctrl_struct_ptr == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "%s::ProvideBuffersToComponent ctrl_struct_ptr == NULL", iName.Str()));
        return false;
    }



    // Now, go through all buffers and tell component to
    // either use a buffer, or to allocate its own buffer
    for (ii = 0; ii < aNumBuffers; ii++)
    {

        int32 errcode = OsclErrNone;
        // get the address where the buf hdr ptr will be stored
        errcode = AllocateChunkFromMemPool(ctrl_struct_ptr[ii], aMemPool, aAllocSize);
        if ((OsclErrNone != errcode) || (ctrl_struct_ptr[ii] == NULL))
        {
            if (OsclErrNoResources == errcode)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "%s::ProvideBuffersToComponent ->allocate() failed for no mempool chunk available", iName.Str()));
            }
            else
            {
                // General error
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "%s::ProvideBuffersToComponent ->allocate() failed due to some general error", iName.Str()));

                ReportErrorEvent(PVMFFailure);
                ChangeNodeState(EPVMFNodeError);
            }

            return false;
        }

        if (aUseBufferOK)
        {
            // Buffers are already allocated outside OMX component.
            // In case of output buffers, the buffer itself is located
            // just after the buffer header pointer.

            // In case of input buffers, the buffer header pointer is followed by a MediaDataSharedPtr
            //	which is used to ensure proper unbinding of the input messages. The buffer itself is either:
            //		a) allocated upstream (and the ptr to the buffer
            //			is a dummy pointer to which the component does not pay attention - PV OMX component)
            //		b) located just after the buffer header pointer and MediaDataSharedPtr

            uint8 *pB = ((uint8*) ctrl_struct_ptr[ii]);


            // in case of input buffers, initialize also MediaDataSharedPtr structure
            if (aIsThisInputBuffer)
            {

                InputBufCtrlStruct *temp = (InputBufCtrlStruct *) ctrl_struct_ptr[ii];
                oscl_memset(&(temp->pMediaData), 0, sizeof(PVMFSharedMediaDataPtr));
                temp->pMediaData = PVMFSharedMediaDataPtr(NULL, NULL);

                // advance ptr to skip the structure
                pB += oscl_mem_aligned_size(sizeof(InputBufCtrlStruct));

                err = OMX_UseBuffer(iOMXDecoder,	// hComponent
                                    &(temp->pBufHdr),		// address where ptr to buffer header will be stored
                                    aPortIndex,				// port index (for port for which buffer is provided)
                                    ctrl_struct_ptr[ii],	// App. private data = pointer to beginning of allocated data
                                    //				to have a context when component returns with a callback (i.e. to know
                                    //				what to free etc.
                                    (OMX_U32)aActualBufferSize,		// buffer size
                                    pB);						// buffer data ptr

                in_ctrl_struct_ptr[ii] = ctrl_struct_ptr[ii];
                in_buff_hdr_ptr[ii] = temp->pBufHdr;

            }
            else
            {
                if (ipExternalOutputBufferAllocatorInterface)
                {
                    // Actual buffer memory will be allocated outside the node from
                    // an external output buffer allocator interface

                    uint8 *pB = (uint8*) ipFixedSizeBufferAlloc->allocate();
                    if (NULL == pB)
                    {
                        //  error
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                        (0, "%s::ProvideBuffersToComponent ->allocate() failed due to some general error", iName.Str()));
                        ReportErrorEvent(PVMFFailure);
                        ChangeNodeState(EPVMFNodeError);
                        return false;
                    }

                    OutputBufCtrlStruct *temp = (OutputBufCtrlStruct *)ctrl_struct_ptr[ii];

                    err = OMX_UseBuffer(iOMXDecoder,	// hComponent
                                        &(temp->pBufHdr),		// address where ptr to buffer header will be stored
                                        aPortIndex,				// port index (for port for which buffer is provided)
                                        ctrl_struct_ptr[ii],	// App. private data = pointer to beginning of allocated data
                                        //				to have a context when component returns with a callback (i.e. to know
                                        //				what to free etc.
                                        (OMX_U32)aActualBufferSize,		// buffer size
                                        pB);						// buffer data ptr

                    out_ctrl_struct_ptr[ii] = ctrl_struct_ptr[ii];
                    out_buff_hdr_ptr[ii] = temp->pBufHdr;

                }
                else
                {
                    OutputBufCtrlStruct *temp = (OutputBufCtrlStruct *) ctrl_struct_ptr[ii];
                    // advance buffer ptr to skip the structure
                    pB += oscl_mem_aligned_size(sizeof(OutputBufCtrlStruct));

                    err = OMX_UseBuffer(iOMXDecoder,	// hComponent
                                        &(temp->pBufHdr),		// address where ptr to buffer header will be stored
                                        aPortIndex,				// port index (for port for which buffer is provided)
                                        ctrl_struct_ptr[ii],	// App. private data = pointer to beginning of allocated data
                                        //				to have a context when component returns with a callback (i.e. to know
                                        //				what to free etc.
                                        (OMX_U32)aActualBufferSize,		// buffer size
                                        pB);						// buffer data ptr

                    out_ctrl_struct_ptr[ii] = ctrl_struct_ptr[ii];
                    out_buff_hdr_ptr[ii] = temp->pBufHdr;
                }

            }


        }
        else
        {
            // the component must allocate its own buffers.
            if (aIsThisInputBuffer)
            {

                InputBufCtrlStruct *temp = (InputBufCtrlStruct *) ctrl_struct_ptr[ii];
                // make sure to init all this to NULL
                oscl_memset(&(temp->pMediaData), 0, sizeof(PVMFSharedMediaDataPtr));
                temp->pMediaData = PVMFSharedMediaDataPtr(NULL, NULL);

                err = OMX_AllocateBuffer(iOMXDecoder,
                                         &(temp->pBufHdr),
                                         aPortIndex,
                                         ctrl_struct_ptr[ii],
                                         (OMX_U32)aActualBufferSize);

                in_ctrl_struct_ptr[ii] = ctrl_struct_ptr[ii];
                in_buff_hdr_ptr[ii] = temp->pBufHdr;
            }
            else
            {
                OutputBufCtrlStruct *temp = (OutputBufCtrlStruct *) ctrl_struct_ptr[ii];
                err = OMX_AllocateBuffer(iOMXDecoder,
                                         &(temp->pBufHdr),
                                         aPortIndex,
                                         ctrl_struct_ptr[ii],
                                         (OMX_U32)aActualBufferSize);

                out_ctrl_struct_ptr[ii] = ctrl_struct_ptr[ii];
                out_buff_hdr_ptr[ii] = temp->pBufHdr;
            }

        }

        if (err != OMX_ErrorNone)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "%s::ProvideBuffersToComponent() Problem using/allocating a buffer", iName.Str()));


            return false;
        }

    }

    for (ii = 0; ii < aNumBuffers; ii++)
    {
        // after initializing the buffer hdr ptrs, return them
        // to the mempool
        aMemPool->deallocate((OsclAny*) ctrl_struct_ptr[ii]);
    }

    oscl_free(ctrl_struct_ptr);

    // set the flags
    if (aIsThisInputBuffer)
    {
        iInputBuffersFreed = false;
    }
    else
    {
        iOutputBuffersFreed = false;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "%s::ProvideBuffersToComponent() done", iName.Str()));
    return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool PVMFOMXBaseDecNode::FreeBuffersFromComponent(OsclMemPoolFixedChunkAllocator *aMemPool, // allocator
        uint32 aAllocSize,	 // size to allocate from pool (hdr only or hdr+ buffer)
        uint32 aNumBuffers,    // number of buffers
        uint32 aPortIndex,      // port idx
        bool	aIsThisInputBuffer		// is this input or output
                                                 )
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "%s::FreeBuffersToComponent() enter", iName.Str()));

    uint32 ii = 0;
    OMX_ERRORTYPE err = OMX_ErrorNone;
    OsclAny **ctrl_struct_ptr = NULL;	// temporary array to keep the addresses of buffer ctrl structures and buffers


    ctrl_struct_ptr = (OsclAny **) oscl_malloc(aNumBuffers * sizeof(OsclAny *));
    if (ctrl_struct_ptr == NULL)
    {
        return false;
    }


    // Now, go through all buffers and tell component to free them
    for (ii = 0; ii < aNumBuffers; ii++)
    {

        int32 errcode = OsclErrNone;
        // get the address where the buf hdr ptr will be stored

        errcode = AllocateChunkFromMemPool(ctrl_struct_ptr[ii], aMemPool, aAllocSize);
        if ((OsclErrNone != errcode) || (ctrl_struct_ptr[ii] == NULL))
        {
            if (OsclErrNoResources == errcode)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "%s::FreeBuffersFromComponent ->allocate() failed for no mempool chunk available", iName.Str()));
            }
            else
            {
                // General error
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "%s::FreeBuffersFromComponent ->allocate() failed due to some general error", iName.Str()));

                ReportErrorEvent(PVMFFailure);
                ChangeNodeState(EPVMFNodeError);
            }

            return false;
        }
        // to maintain correct count
        aMemPool->notifyfreechunkavailable((*this), (OsclAny*) aMemPool);

        if (aIsThisInputBuffer)
        {

            iNumOutstandingInputBuffers++;
            // get the buf hdr pointer
            InputBufCtrlStruct *temp = (InputBufCtrlStruct *) ctrl_struct_ptr[ii];
            err = OMX_FreeBuffer(iOMXDecoder,
                                 aPortIndex,
                                 temp->pBufHdr);

        }
        else
        {
            if (ipExternalOutputBufferAllocatorInterface)
            {
                //Deallocate the output buffer memory that was allocated outside the node
                //using an external output buffer allocator interface

                iNumOutstandingOutputBuffers++;
                OutputBufCtrlStruct *temp = (OutputBufCtrlStruct *) ctrl_struct_ptr[ii];
                ipFixedSizeBufferAlloc->deallocate((OsclAny*) temp->pBufHdr->pBuffer);

                err = OMX_FreeBuffer(iOMXDecoder,
                                     aPortIndex,
                                     temp->pBufHdr);
            }
            else
            {
                iNumOutstandingOutputBuffers++;
                OutputBufCtrlStruct *temp = (OutputBufCtrlStruct *) ctrl_struct_ptr[ii];
                err = OMX_FreeBuffer(iOMXDecoder,
                                     aPortIndex,
                                     temp->pBufHdr);
            }
        }

        if (err != OMX_ErrorNone)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "%s::FreeBuffersFromComponent() Problem freeing a buffer", iName.Str()));

            return false;
        }

    }

    for (ii = 0; ii < aNumBuffers; ii++)
    {
        // after freeing the buffer hdr ptrs, return them
        // to the mempool (which will itself then be deleted promptly)
        aMemPool->deallocate((OsclAny*) ctrl_struct_ptr[ii]);
    }

    oscl_free(ctrl_struct_ptr);

    // mark buffers as freed (so as not to do it twice)
    if (aIsThisInputBuffer)
    {
        oscl_free(in_ctrl_struct_ptr);
        oscl_free(in_buff_hdr_ptr);

        in_ctrl_struct_ptr = NULL;
        in_buff_hdr_ptr = NULL;
        iInputBuffersFreed = true;
    }
    else
    {
        oscl_free(out_ctrl_struct_ptr);
        oscl_free(out_buff_hdr_ptr);

        out_ctrl_struct_ptr = NULL;
        out_buff_hdr_ptr = NULL;
        iOutputBuffersFreed = true;

        if (ipExternalOutputBufferAllocatorInterface)
        {
            ipExternalOutputBufferAllocatorInterface->removeRef();
            ipExternalOutputBufferAllocatorInterface = NULL;
        }
    }
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "%s::FreeBuffersFromComponent() done", iName.Str()));
    return true;
}

/////////////////////////////////////////////////////////////////////////////
// This function handles the event of OMX component state change
/////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PVMFOMXBaseDecNode::HandleComponentStateChange(OMX_U32 decoder_state)
{
    switch (decoder_state)
    {
        case OMX_StateIdle:
        {
            iCurrentDecoderState = OMX_StateIdle;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "%s::HandleComponentStateChange: OMX_StateIdle reached", iName.Str()));

            //  this state can be reached either going from OMX_Loaded->OMX_Idle (preparing)
            //	or going from OMX_Executing->OMX_Idle (stopping)


            if ((iCurrentCommand.size() > 0) &&
                    (iCurrentCommand.front().iCmd == PVMFOMXBaseDecNodeCommand::PVOMXBASEDEC_NODE_CMD_PREPARE))
            {
                iProcessingState = EPVMFOMXBaseDecNodeProcessingState_InitDecoder;
                SetState(EPVMFNodePrepared);
                CommandComplete(iCurrentCommand, iCurrentCommand.front(), PVMFSuccess);
                RunIfNotReady();
            }
            else if ((iCurrentCommand.size() > 0) &&
                     (iCurrentCommand.front().iCmd == PVMFOMXBaseDecNodeCommand::PVOMXBASEDEC_NODE_CMD_STOP))
            {
                // if we are stopped, we won't start until the node gets DoStart command.
                //	in this case, we are ready to start sending buffers
                if (iProcessingState == EPVMFOMXBaseDecNodeProcessingState_Stopping)
                    iProcessingState = EPVMFOMXBaseDecNodeProcessingState_ReadyToDecode;
                // if the processing state was not stopping, leave the state as it was (continue port reconfiguration)
                SetState(EPVMFNodePrepared);
                iStopCommandWasSentToComponent = false;
                CommandComplete(iCurrentCommand, iCurrentCommand.front(), PVMFSuccess);

                RunIfNotReady();
            }
            else if ((iCurrentCommand.size() > 0) &&
                     (iCurrentCommand.front().iCmd == PVMFOMXBaseDecNodeCommand::PVOMXBASEDEC_NODE_CMD_RESET))
            {
                // State change to Idle was initiated due to Reset. First need to reach idle, and then loaded
                // Once Idle is reached, we need to initiate idle->loaded transition
                iStopInResetMsgSent = false;
                RunIfNotReady();
            }
            break;
        }//end of case OMX_StateIdle

        case OMX_StateExecuting:
        {
            iCurrentDecoderState = OMX_StateExecuting;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "%s::HandleComponentStateChange: OMX_StateExecuting reached", iName.Str()));

            // this state can be reached going from OMX_Idle -> OMX_Executing (preparing)
            //	or going from OMX_Pause -> OMX_Executing (coming from pause)
            //	either way, this is a response to "DoStart" command

            if ((iCurrentCommand.size() > 0) &&
                    (iCurrentCommand.front().iCmd == PVMFOMXBaseDecNodeCommand::PVOMXBASEDEC_NODE_CMD_START))
            {
                SetState(EPVMFNodeStarted);
                CommandComplete(iCurrentCommand, iCurrentCommand.front(), PVMFSuccess);

                RunIfNotReady();
            }

            break;
        }//end of case OMX_StateExecuting

        case OMX_StatePause:
        {
            iCurrentDecoderState = OMX_StatePause;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "%s::HandleComponentStateChange: OMX_StatePause reached", iName.Str()));

            // if we are paused, we won't start until the node gets DoStart command.
            //	in this case, we are ready to start sending buffers
            if (iProcessingState == EPVMFOMXBaseDecNodeProcessingState_Pausing)
                iProcessingState = EPVMFOMXBaseDecNodeProcessingState_ReadyToDecode;

            //	This state can be reached going from OMX_Executing-> OMX_Pause
            if ((iCurrentCommand.size() > 0) &&
                    (iCurrentCommand.front().iCmd == PVMFOMXBaseDecNodeCommand::PVOMXBASEDEC_NODE_CMD_PAUSE))
            {

                // if we are paused, we won't start until the node gets DoStart command.
                //	in this case, we are ready to start sending buffers
                if (iProcessingState == EPVMFOMXBaseDecNodeProcessingState_Pausing)
                    iProcessingState = EPVMFOMXBaseDecNodeProcessingState_ReadyToDecode;
                // if the processing state was not pausing, leave the state as it was (continue port reconfiguration)


                SetState(EPVMFNodePaused);
                iPauseCommandWasSentToComponent = false;
                CommandComplete(iCurrentCommand, iCurrentCommand.front(), PVMFSuccess);
                RunIfNotReady();
            }

            break;
        }//end of case OMX_StatePause

        case OMX_StateLoaded:
        {
            iCurrentDecoderState = OMX_StateLoaded;

            //  this state can be reached only going from OMX_Idle ->OMX_Loaded (stopped to reset)
            //

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "%s::HandleComponentStateChange: OMX_StateLoaded reached", iName.Str()));
            //Check if command's responce is pending
            if ((iCurrentCommand.size() > 0) &&
                    (iCurrentCommand.front().iCmd == PVMFOMXBaseDecNodeCommand::PVOMXBASEDEC_NODE_CMD_RESET))
            {

                // move this here
                if (iInPort)
                {
                    OSCL_DELETE(((PVMFOMXDecPort*)iInPort));
                    iInPort = NULL;
                }

                if (iOutPort)
                {
                    OSCL_DELETE(((PVMFOMXDecPort*)iOutPort));
                    iOutPort = NULL;
                }

                iDataIn.Unbind();

                // Reset the metadata key list
                iAvailableMetadataKeys.clear();


                iProcessingState = EPVMFOMXBaseDecNodeProcessingState_Idle;
                //logoff & go back to Created state.
                SetState(EPVMFNodeIdle);
                CommandComplete(iCurrentCommand, iCurrentCommand.front(), PVMFSuccess);
                iResetInProgress = false;
                iResetMsgSent = false;
            }

            break;
        }//end of case OMX_StateLoaded

        case OMX_StateInvalid:
        default:
        {
            iCurrentDecoderState = OMX_StateInvalid;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "%s::HandleComponentStateChange: OMX_StateInvalid reached", iName.Str()));

            break;
        }//end of case OMX_StateInvalid

    }//end of switch(decoder_state)

}






/////////////////////////////////////////////////////////////////////////////
////////////////////// CALLBACK PROCESSING FOR EMPTY BUFFER DONE - input buffer was consumed
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE PVMFOMXBaseDecNode::EmptyBufferDoneProcessing(OMX_OUT OMX_HANDLETYPE aComponent,
        OMX_OUT OMX_PTR aAppData,
        OMX_OUT OMX_BUFFERHEADERTYPE* aBuffer)
{
    OSCL_UNUSED_ARG(aComponent);
    OSCL_UNUSED_ARG(aAppData);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "%s::EmptyBufferDoneProcessing: In", iName.Str()));

    OSCL_ASSERT((void*) aComponent == (void*) iOMXDecoder); // component should match the component
    OSCL_ASSERT(aAppData == (OMX_PTR)(this));		// AppData should represent this node ptr

    // first, get the buffer "context", i.e. pointer to application private data that contains the
    // address of the mempool buffer (so that it can be released)
    InputBufCtrlStruct *pContext = (InputBufCtrlStruct *)(aBuffer->pAppPrivate);



    // if a buffer is not empty, log a msg, but release anyway
    if ((aBuffer->nFilledLen > 0) && (iDoNotSaveInputBuffersFlag == false))
        // if dynamic port reconfig is in progress for input port, don't keep the buffer
    {

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "%s::EmptyBufferDoneProcessing: Input buffer returned non-empty with %d bytes still in it", iName.Str(), aBuffer->nFilledLen));


    }


    iInputBufferToResendToComponent = NULL;

    // input buffer is to be released,
    // refcount needs to be decremented (possibly - the input msg associated with the buffer will be unbound)
    // NOTE: in case of "moveable" input buffers (passed into component without copying), unbinding decrements a refcount which eventually results
    //			in input message being released back to upstream mempool once all its fragments are returned
    //		in case of input buffers passed into component by copying, unbinding has no effect
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "%s::EmptyBufferDoneProcessing: Release input buffer with TS=%d (with %d refcount remaining of input message)", iName.Str(), aBuffer->nTimeStamp, (pContext->pMediaData).get_count() - 1));


    (pContext->pMediaData).Unbind();


    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "%s::EmptyBufferDoneProcessing: Release input buffer %x back to mempool", iName.Str(), pContext));

    iInBufMemoryPool->deallocate((OsclAny *) pContext);


    // the OMX spec says that no error is to be returned
    return OMX_ErrorNone;

}



/////////////////////////////////////////////////////////////////////////////
////////////////////// CALLBACK PROCESSING FOR FILL BUFFER DONE - output buffer is ready
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE PVMFOMXBaseDecNode::FillBufferDoneProcessing(OMX_OUT OMX_HANDLETYPE aComponent,
        OMX_OUT OMX_PTR aAppData,
        OMX_OUT OMX_BUFFERHEADERTYPE* aBuffer)
{
    OSCL_UNUSED_ARG(aComponent);
    OSCL_UNUSED_ARG(aAppData);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "%s::FillBufferDoneProcessing: In", iName.Str()));

    OSCL_ASSERT((void*) aComponent == (void*) iOMXDecoder); // component should match the component
    OSCL_ASSERT(aAppData == (OMX_PTR)(this));		// AppData should represent this node ptr

    // first, get the buffer "context", i.e. pointer to application private data that contains the
    // address of the mempool buffer (so that it can be released)
    OsclAny *pContext = (OsclAny*) aBuffer->pAppPrivate;


    // check for EOS flag
    if ((aBuffer->nFlags & OMX_BUFFERFLAG_EOS))
    {
        // EOS received - enable sending EOS msg
        iIsEOSReceivedFromComponent = true;

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "%s::FillBufferDoneProcessing: Output buffer has EOS set", iName.Str()));

    }

    // if a buffer is empty, or if it should not be sent downstream (say, due to state change)
    // release the buffer back to the pool
    if ((aBuffer->nFilledLen == 0) || (iDoNotSendOutputBuffersDownstreamFlag == true))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "%s::FillBufferDoneProcessing: Release output buffer %x back to mempool - buffer empty or not to be sent downstream", iName.Str(), pContext));

        iOutBufMemoryPool->deallocate(pContext);

    }
    else
    {

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "%s::FillBufferDoneProcessing: Output frame %d received", iName.Str(), iFrameCounter++));

        // get pointer to actual buffer data
        uint8 *pBufdata = ((uint8*) aBuffer->pBuffer);
        // move the data pointer based on offset info
        pBufdata += aBuffer->nOffset;

        iOutTimeStamp = aBuffer->nTimeStamp;

        ipPrivateData = (OsclAny *) aBuffer->pPlatformPrivate; // record the pointer

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "%s::FillBufferDoneProcessing: Wrapping buffer %x of size %d", iName.Str(), pBufdata, aBuffer->nFilledLen));
        // wrap the buffer into the MediaDataImpl wrapper, and queue it for sending downstream
        // wrapping will create a refcounter. When refcounter goes to 0 i.e. when media data
        // is released in downstream components, the custom deallocator will automatically release the buffer back to the
        //	mempool. To do that, the deallocator needs to have info about Context
        // NOTE: we had to wait until now to wrap the buffer data because we only know
        //			now where the actual data is located (based on buffer offset)
        OsclSharedPtr<PVMFMediaDataImpl> MediaDataOut = WrapOutputBuffer(pBufdata, (uint32)(aBuffer->nFilledLen), pContext);

        // if you can't get the MediaDataOut, release the buffer back to the pool
        if (MediaDataOut.GetRep() == NULL)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "%s::FillBufferDoneProcessing: Problem wrapping buffer %x of size %d - releasing the buffer", iName.Str(), pBufdata, aBuffer->nFilledLen));

            iOutBufMemoryPool->deallocate(pContext);
        }
        else
        {

            // if there's a problem queuing output buffer, MediaDataOut will expire at end of scope and
            // release buffer back to the pool, (this should not be the case)
            if (QueueOutputBuffer(MediaDataOut, aBuffer->nFilledLen))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "%s::FillBufferDoneProcessing: Buffer %x of size %d queued - reschedule the node to send out", iName.Str(), pBufdata, aBuffer->nFilledLen));

                // if queing went OK,
                // re-schedule the node so that outgoing queue can be emptied (unless the outgoing port is busy)
                if ((iOutPort) && !(iOutPort->IsConnectedPortBusy()))
                    RunIfNotReady();
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "%s::FillBufferDoneProcessing: Problem queing buffer %x of size %d - releasing the buffer", iName.Str(), pBufdata, aBuffer->nFilledLen));
            }


        }

    }
    // the OMX spec says that no error is to be returned
    return OMX_ErrorNone;

}

//////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// Attach a MediaDataImpl wrapper (refcount, deallocator etc.)
/////////////////////////////// to the output buffer /////////////////////////////////////////
OsclSharedPtr<PVMFMediaDataImpl> PVMFOMXBaseDecNode::WrapOutputBuffer(uint8 *pData, uint32 aDataLen, OsclAny *pContext)
{
    // wrap output buffer into a mediadataimpl
    uint32 aligned_class_size = oscl_mem_aligned_size(sizeof(PVMFSimpleMediaBuffer));
    uint32 aligned_cleanup_size = oscl_mem_aligned_size(sizeof(PVOMXDecBufferSharedPtrWrapperCombinedCleanupDA));
    uint32 aligned_refcnt_size = oscl_mem_aligned_size(sizeof(OsclRefCounterDA));
    uint8 *my_ptr = (uint8*) oscl_malloc(aligned_refcnt_size + aligned_cleanup_size + aligned_class_size);

    if (my_ptr == NULL)
    {
        OsclSharedPtr<PVMFMediaDataImpl> null_buff(NULL, NULL);
        return null_buff;
    }
    // create a deallocator and pass the buffer_allocator to it as well as pointer to data that needs to be returned to the mempool
    PVOMXDecBufferSharedPtrWrapperCombinedCleanupDA *cleanup_ptr =
        OSCL_PLACEMENT_NEW(my_ptr + aligned_refcnt_size, PVOMXDecBufferSharedPtrWrapperCombinedCleanupDA(iOutBufMemoryPool, pContext));

    //ModifiedPvciBufferCombinedCleanup* cleanup_ptr = OSCL_PLACEMENT_NEW(my_ptr + aligned_refcnt_size,ModifiedPvciBufferCombinedCleanup(aOutput.GetRefCounter()) );

    // create the ref counter after the cleanup object (refcount is set to 1 at creation)
    OsclRefCounterDA *my_refcnt = OSCL_PLACEMENT_NEW(my_ptr, OsclRefCounterDA(my_ptr, cleanup_ptr));

    my_ptr += aligned_refcnt_size + aligned_cleanup_size;

    PVMFMediaDataImpl* media_data_ptr = OSCL_PLACEMENT_NEW(my_ptr, PVMFSimpleMediaBuffer((void *) pData, // ptr to data
                                        aDataLen, // capacity
                                        my_refcnt));   // ref counter

    OsclSharedPtr<PVMFMediaDataImpl> MediaDataImplOut(media_data_ptr, my_refcnt);

    MediaDataImplOut->setMediaFragFilledLen(0, aDataLen);

    return MediaDataImplOut;

}
//////////////////////////////////////////////////////////////////////////////
bool PVMFOMXBaseDecNode::SendBeginOfMediaStreamCommand()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "%s::SendBeginOfMediaStreamCommand() In", iName.Str()));

    PVMFSharedMediaCmdPtr sharedMediaCmdPtr = PVMFMediaCmd::createMediaCmd();
    // Set the formatID, timestamp, sequenceNumber and streamID for the media message
    sharedMediaCmdPtr->setFormatID(PVMF_MEDIA_CMD_BOS_FORMAT_ID);
    sharedMediaCmdPtr->setTimestamp(iBOSTimestamp);
    //reset the sequence number
    uint32 seqNum = 0;
    sharedMediaCmdPtr->setSeqNum(seqNum);
    sharedMediaCmdPtr->setStreamID(iStreamID);

    PVMFSharedMediaMsgPtr mediaMsgOut;
    convertToPVMFMediaCmdMsg(mediaMsgOut, sharedMediaCmdPtr);
    if (iOutPort->QueueOutgoingMsg(mediaMsgOut) != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "%s::SendBeginOfMediaStreamCommand() Outgoing queue busy", iName.Str()));
        return false;
    }

    iSendBOS = false;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "%s::SendBeginOfMediaStreamCommand() BOS Sent StreamID %d", iName.Str(), iStreamID));
    return true;
}
////////////////////////////////////
bool PVMFOMXBaseDecNode::SendEndOfTrackCommand(void)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "%s::SendEndOfTrackCommand() In", iName.Str()));

    PVMFSharedMediaCmdPtr sharedMediaCmdPtr = PVMFMediaCmd::createMediaCmd();

    sharedMediaCmdPtr->setFormatID(PVMF_MEDIA_CMD_EOS_FORMAT_ID);

    // Set the timestamp
    sharedMediaCmdPtr->setTimestamp(iEndOfDataTimestamp);

    // Set Streamid
    sharedMediaCmdPtr->setStreamID(iStreamID);

    // Set the sequence number
    sharedMediaCmdPtr->setSeqNum(iSeqNum++);

    PVMFSharedMediaMsgPtr mediaMsgOut;
    convertToPVMFMediaCmdMsg(mediaMsgOut, sharedMediaCmdPtr);
    if (iOutPort->QueueOutgoingMsg(mediaMsgOut) != PVMFSuccess)
    {
        // this should not happen because we check for queue busy before calling this function
        return false;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "%s::SendEndOfTrackCommand() Out", iName.Str()));
    return true;
}

/////////////////////////////////////////////////////////////////////////////
//The various command handlers call this routine when a command is complete.
/////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PVMFOMXBaseDecNode::CommandComplete(PVMFOMXBaseDecNodeCmdQ& aCmdQ, PVMFOMXBaseDecNodeCommand& aCmd, PVMFStatus aStatus, OsclAny* aEventData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "%s:CommandComplete Id %d Cmd %d Status %d Context %d Data %d",
                    iName.Str(), aCmd.iId, aCmd.iCmd, aStatus, aCmd.iContext, aEventData));

    //create response
    PVMFCmdResp resp(aCmd.iId, aCmd.iContext, aStatus, aEventData);
    PVMFSessionId session = aCmd.iSession;

    //Erase the command from the queue.
    aCmdQ.Erase(&aCmd);

    //Report completion to the session observer.
    ReportCmdCompleteEvent(session, resp);
}

/////////////////////////////////////////////////////////////////////////////
void PVMFOMXBaseDecNode::DoInit(PVMFOMXBaseDecNodeCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "%s::DoInit() In", iName.Str()));
    switch (iInterfaceState)
    {
        case EPVMFNodeIdle:
        {
            SetState(EPVMFNodeInitialized);
            CommandComplete(iInputCommands, aCmd, PVMFSuccess);
            break;
        }

        default:
            CommandComplete(iInputCommands, aCmd, PVMFErrInvalidState);
            break;
    }
}

/////////////////////////////////////////////////////////////////////////////
void PVMFOMXBaseDecNode::DoPrepare(PVMFOMXBaseDecNodeCommand& aCmd)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    OMXConfigParserInputs aInputParameters;
    aInputParameters.cComponentRole = NULL;
    OMX_PTR aOutputParameters = NULL;

    switch (iInterfaceState)
    {
        case EPVMFNodeInitialized:
        {
            if (NULL == iInPort)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "%s::DoPrepare() Input port not initialized", iName.Str()));
                CommandComplete(iInputCommands, aCmd, PVMFFailure);
                return;
            }

            // Check format of input data
            PVMFFormatType format = ((PVMFOMXDecPort*)iInPort)->iFormat;
            // AAC
            if (format == PVMF_MIME_MPEG4_AUDIO ||
                    format == PVMF_MIME_3640 ||
                    format == PVMF_MIME_LATM ||
                    format == PVMF_MIME_ADIF ||
                    format == PVMF_MIME_ASF_MPEG4_AUDIO ||
                    format == PVMF_MIME_AAC_SIZEHDR)
            {
                aInputParameters.cComponentRole = (OMX_STRING)"audio_decoder.aac";
                aOutputParameters = (AudioOMXConfigParserOutputs *)oscl_malloc(sizeof(AudioOMXConfigParserOutputs));
            }
            // AMR
            else if (format == PVMF_MIME_AMR_IF2 ||
                     format == PVMF_MIME_AMR_IETF ||
                     format == PVMF_MIME_AMR)
            {
                aInputParameters.cComponentRole = (OMX_STRING)"audio_decoder.amrnb";
                aOutputParameters = (AudioOMXConfigParserOutputs *)oscl_malloc(sizeof(AudioOMXConfigParserOutputs));
            }
            else if (format == PVMF_MIME_AMRWB_IETF ||
                     format == PVMF_MIME_AMRWB)
            {
                aInputParameters.cComponentRole = (OMX_STRING)"audio_decoder.amrwb";
                aOutputParameters = (AudioOMXConfigParserOutputs *)oscl_malloc(sizeof(AudioOMXConfigParserOutputs));
            }
            else if (format == PVMF_MIME_MP3)
            {
                aInputParameters.cComponentRole = (OMX_STRING)"audio_decoder.mp3";
                aOutputParameters = (AudioOMXConfigParserOutputs *)oscl_malloc(sizeof(AudioOMXConfigParserOutputs));
            }
            else if (format ==  PVMF_MIME_WMA)
            {
                aInputParameters.cComponentRole = (OMX_STRING)"audio_decoder.wma";
                aOutputParameters = (AudioOMXConfigParserOutputs *)oscl_malloc(sizeof(AudioOMXConfigParserOutputs));
            }
            else if (format ==  PVMF_MIME_H264_VIDEO ||
                     format == PVMF_MIME_H264_VIDEO_MP4 ||
                     format == PVMF_MIME_H264_VIDEO_RAW)
            {
                aInputParameters.cComponentRole = (OMX_STRING)"video_decoder.avc";
                aOutputParameters = (VideoOMXConfigParserOutputs *)oscl_malloc(sizeof(VideoOMXConfigParserOutputs));
            }
            else if (format ==  PVMF_MIME_M4V)
            {
                aInputParameters.cComponentRole = (OMX_STRING)"video_decoder.mpeg4";
                aOutputParameters = (VideoOMXConfigParserOutputs *)oscl_malloc(sizeof(VideoOMXConfigParserOutputs));
            }
            else if (format ==  PVMF_MIME_H2631998 ||
                     format == PVMF_MIME_H2632000)
            {
                aInputParameters.cComponentRole = (OMX_STRING)"video_decoder.h263";
                aOutputParameters = (VideoOMXConfigParserOutputs *)oscl_malloc(sizeof(VideoOMXConfigParserOutputs));
            }
            else if (format ==  PVMF_MIME_WMV)
            {
                aInputParameters.cComponentRole = (OMX_STRING)"video_decoder.wmv";
                aOutputParameters = (VideoOMXConfigParserOutputs *)oscl_malloc(sizeof(VideoOMXConfigParserOutputs));
            }
            else
            {
                // Illegal codec specified.
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "%s::DoPrepare() Input port format other then codec type", iName.Str()));
                CommandComplete(iInputCommands, aCmd, PVMFErrArgument);
                return;
            }
            if (aOutputParameters == NULL)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "%s::Can't allocate memory for OMXConfigParser output!", iName.Str()))
                CommandComplete(iInputCommands, aCmd, PVMFErrResource);
                return;
            }

            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "%s::Initializing OMX component and decoder for role %s", iName.Str(), aInputParameters.cComponentRole));

            /* Set callback structure */
            iCallbacks.EventHandler    = CallbackEventHandler; //event_handler;
            iCallbacks.EmptyBufferDone = CallbackEmptyBufferDone; //empty_buffer_done;
            iCallbacks.FillBufferDone  = CallbackFillBufferDone; //fill_buffer_done;


            // determine components which can fit the role
            // then, create the component. If multiple components fit the role,
            // the first one registered will be selected. If that one fails to
            // be created, the second one in the list is selected etc.
            OMX_BOOL status;
            OMX_U32 num_comps = 0;
            OMX_STRING *CompOfRole;
            OMX_S8 CompName[PV_OMX_MAX_COMPONENT_NAME_LENGTH];

            //AudioOMXConfigParserOutputs aOutputParameters;
            aInputParameters.inPtr = (uint8*)((PVMFOMXDecPort*)iInPort)->iTrackConfig;
            aInputParameters.inBytes = (int32)((PVMFOMXDecPort*)iInPort)->iTrackConfigSize;

            if (aInputParameters.inBytes == 0 || aInputParameters.inPtr == NULL)
            {
                if (format == PVMF_MIME_WMA ||
                        format == PVMF_MIME_MPEG4_AUDIO ||
                        format == PVMF_MIME_3640 ||
                        format == PVMF_MIME_LATM ||
                        format == PVMF_MIME_ADIF ||
                        format == PVMF_MIME_ASF_MPEG4_AUDIO ||
                        format == PVMF_MIME_AAC_SIZEHDR)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "%s::DoPrepare() Cannot get component parameters", iName.Str()));
                    oscl_free(aOutputParameters);
                    CommandComplete(iInputCommands, aCmd, PVMFErrNoResources);
                    return;

                }
            }
            // call once to find out the number of components that can fit the role
            OMX_GetComponentsOfRole(aInputParameters.cComponentRole, &num_comps, NULL);
            uint32 ii;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "%s::DoPrepare(): There are %d components of role %s ", iName.Str(), num_comps, aInputParameters.cComponentRole));

            if (num_comps > 0)
            {
                CompOfRole = (OMX_STRING *)oscl_malloc(num_comps * sizeof(OMX_STRING));

                for (ii = 0; ii < num_comps; ii++)
                    CompOfRole[ii] = (OMX_STRING) oscl_malloc(PV_OMX_MAX_COMPONENT_NAME_LENGTH * sizeof(OMX_U8));

                // call 2nd time to get the component names
                OMX_GetComponentsOfRole(aInputParameters.cComponentRole, &num_comps, (OMX_U8 **)CompOfRole);

                for (ii = 0; ii < num_comps; ii++)
                {
                    aInputParameters.cComponentName = CompOfRole[ii];
                    status = OMXConfigParser(&aInputParameters, aOutputParameters);
                    if (status == OMX_TRUE)
                    {
                        // need to actually copy name since the memory for CompOfRole needs to be freed before a possible return or there will a be a memory leak,
                        // but also needs to valid long enough to use it when getting the number of roles later on
                        oscl_strncpy((OMX_STRING)CompName, (OMX_STRING) CompOfRole[ii], PV_OMX_MAX_COMPONENT_NAME_LENGTH);

                        // try to create component
                        err = OMX_GetHandle(&iOMXDecoder, (OMX_STRING) aInputParameters.cComponentName, (OMX_PTR) this, (OMX_CALLBACKTYPE *) & iCallbacks);
                        // if successful, no need to continue
                        if ((err == OMX_ErrorNone) && (iOMXDecoder != NULL))
                        {
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                            (0, "%s::DoPrepare(): Got Component %s handle ", iName.Str(), aInputParameters.cComponentName));
                            break;
                        }
                        else
                        {
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                            (0, "%s::DoPrepare(): Cannot get component %s handle, try another component if available", iName.Str(), aInputParameters.cComponentName));
                        }
                    }
                    else
                    {
                        status = OMX_FALSE;
                    }


                }
                // whether successful or not, need to free CompOfRoles
                for (ii = 0; ii < num_comps; ii++)
                {
                    oscl_free(CompOfRole[ii]);
                    CompOfRole[ii] = NULL;
                }

                oscl_free(CompOfRole);

                // check if there was a problem
                if ((err != OMX_ErrorNone) || (iOMXDecoder == NULL))
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "%s::Can't get handle for decoder!", iName.Str()));
                    iOMXDecoder = NULL;
                    oscl_free(aOutputParameters);
                    CommandComplete(iInputCommands, aCmd, PVMFErrResource);
                    return;
                }
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "%s::No component can handle role %s !", iName.Str(), aInputParameters.cComponentRole));
                iOMXDecoder = NULL;
                oscl_free(aOutputParameters);
                CommandComplete(iInputCommands, aCmd, PVMFErrResource);
                return;
            }



            if (!iOMXDecoder)
            {
                oscl_free(aOutputParameters);
                CommandComplete(iInputCommands, aCmd, PVMFErrNoResources);
                return;
            }

            // find out how many roles the component supports
            OMX_U32 NumRoles;
            err = OMX_GetRolesOfComponent((OMX_STRING)CompName, &NumRoles, NULL);
            if (err != OMX_ErrorNone)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "%s::DoPrepare() Problem getting component roles", iName.Str()));

                CommandComplete(iInputCommands, aCmd, PVMFErrResource);
                return;
            }

            // if the component supports multiple roles, call OMX_SetParameter
            if (NumRoles > 1)
            {
                OMX_PARAM_COMPONENTROLETYPE RoleParam;
                CONFIG_SIZE_AND_VERSION(RoleParam);
                oscl_strncpy((OMX_STRING)RoleParam.cRole, (OMX_STRING)aInputParameters.cComponentRole, OMX_MAX_STRINGNAME_SIZE);
                err = OMX_SetParameter(iOMXDecoder, OMX_IndexParamStandardComponentRole, &RoleParam);
                if (err != OMX_ErrorNone)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "%s::DoPrepare() Problem setting component role", iName.Str()));

                    CommandComplete(iInputCommands, aCmd, PVMFErrResource);
                    return;
                }
            }

            // GET CAPABILITY FLAGS FROM PV COMPONENT, IF this fails, use defaults
            PV_OMXComponentCapabilityFlagsType Cap_flags;
            err = OMX_GetParameter(iOMXDecoder, (OMX_INDEXTYPE) PV_OMX_COMPONENT_CAPABILITY_TYPE_INDEX, &Cap_flags);
            if (err != OMX_ErrorNone)
            {
                SetDefaultCapabilityFlags();
            }
            else
            {
                iIsOMXComponentMultiThreaded = (OMX_TRUE == Cap_flags.iIsOMXComponentMultiThreaded) ? true : false;
                iOMXComponentSupportsExternalInputBufferAlloc = (OMX_TRUE == Cap_flags.iOMXComponentSupportsExternalInputBufferAlloc) ? true : false;
                iOMXComponentSupportsExternalOutputBufferAlloc = (OMX_TRUE == Cap_flags.iOMXComponentSupportsExternalOutputBufferAlloc) ? true : false;
                iOMXComponentSupportsMovableInputBuffers = (OMX_TRUE == Cap_flags.iOMXComponentSupportsMovableInputBuffers) ? true : false;
                iOMXComponentSupportsPartialFrames = (OMX_TRUE == Cap_flags.iOMXComponentSupportsPartialFrames) ? true : false;
                iOMXComponentUsesNALStartCodes = (OMX_TRUE == Cap_flags.iOMXComponentUsesNALStartCodes) ? true : false;
                iOMXComponentCanHandleIncompleteFrames = (OMX_TRUE == Cap_flags.iOMXComponentCanHandleIncompleteFrames) ? true : false;
                iOMXComponentUsesFullAVCFrames = (OMX_TRUE == Cap_flags.iOMXComponentUsesFullAVCFrames) ? true : false;
            }

            // do some sanity checking

            if ((format != PVMF_MIME_H264_VIDEO) && (format != PVMF_MIME_H264_VIDEO_MP4) && (format != PVMF_MIME_H264_VIDEO_RAW))
            {
                iOMXComponentUsesNALStartCodes = false;
                iOMXComponentUsesFullAVCFrames = false;
            }

            if (iOMXComponentUsesFullAVCFrames)
            {
                iNALCount = 0;
                oscl_memset(iNALSizeArray, 0, sizeof(uint32) * MAX_NAL_PER_FRAME); // 100 is max number of NALs
            }

            // make sure that copying is used where necessary
            if (!iOMXComponentSupportsPartialFrames || iOMXComponentUsesNALStartCodes || iOMXComponentUsesFullAVCFrames)
            {
                iOMXComponentSupportsMovableInputBuffers = false;
            }

            // find out about parameters

            if (!NegotiateComponentParameters(aOutputParameters))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "%s::DoPrepare() Cannot get component parameters", iName.Str()));

                oscl_free(aOutputParameters);
                CommandComplete(iInputCommands, aCmd, PVMFErrNoResources);
                return;
            }
            oscl_free(aOutputParameters);

            // create active objects to handle callbacks in case of multithreaded implementation

            // NOTE: CREATE THE THREADSAFE CALLBACK AOs REGARDLESS OF WHETHER MULTITHREADED COMPONENT OR NOT
            //		If it is not multithreaded, we won't use them
            //		The Flag iIsComponentMultiThreaded decides which mechanism is used for callbacks.
            //		This flag is set by looking at component capabilities (or to true by default)

            if (iThreadSafeHandlerEventHandler)
            {
                OSCL_DELETE(iThreadSafeHandlerEventHandler);
                iThreadSafeHandlerEventHandler = NULL;
            }
            // substitute default parameters: observer(this node),queuedepth(3),nameAO for logging
            // Get the priority of the dec node, and set the threadsafe callback AO priority to 1 higher
            iThreadSafeHandlerEventHandler = OSCL_NEW(EventHandlerThreadSafeCallbackAO, (this, 10, "EventHandlerAO", Priority() + 2));

            if (iThreadSafeHandlerEmptyBufferDone)
            {
                OSCL_DELETE(iThreadSafeHandlerEmptyBufferDone);
                iThreadSafeHandlerEmptyBufferDone = NULL;
            }
            // use queue depth of iNumInputBuffers to prevent deadlock
            iThreadSafeHandlerEmptyBufferDone = OSCL_NEW(EmptyBufferDoneThreadSafeCallbackAO, (this, iNumInputBuffers, "EmptyBufferDoneAO", Priority() + 1));

            if (iThreadSafeHandlerFillBufferDone)
            {
                OSCL_DELETE(iThreadSafeHandlerFillBufferDone);
                iThreadSafeHandlerFillBufferDone = NULL;
            }
            // use queue depth of iNumOutputBuffers to prevent deadlock
            iThreadSafeHandlerFillBufferDone = OSCL_NEW(FillBufferDoneThreadSafeCallbackAO, (this, iNumOutputBuffers, "FillBufferDoneAO", Priority() + 1));

            if ((iThreadSafeHandlerEventHandler == NULL) ||
                    (iThreadSafeHandlerEmptyBufferDone == NULL) ||
                    (iThreadSafeHandlerFillBufferDone == NULL)
               )
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "%s::Can't get threadsafe callbacks for decoder!", iName.Str()));
                iOMXDecoder = NULL;
            }

            // ONLY FOR AVC FILE PLAYBACK WILL 1 FRAGMENT CONTAIN ONE FULL NAL
            if ((format == PVMF_MIME_H264_VIDEO) || (format == PVMF_MIME_H264_VIDEO_MP4))
            {
                // every memory fragment in case of AVC is a full NAL
                iSetMarkerBitForEveryFrag = true;
            }
            else
            {
                iSetMarkerBitForEveryFrag = false;
            }


            // Init Decoder
            iCurrentDecoderState = OMX_StateLoaded;

            /* Change state to OMX_StateIdle from OMX_StateLoaded. */
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "%s::DoPrepare(): Changing Component state Loaded -> Idle ", iName.Str()));

            err = OMX_SendCommand(iOMXDecoder, OMX_CommandStateSet, OMX_StateIdle, NULL);
            if (err != OMX_ErrorNone)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "%s::DoPrepare() Can't send StateSet command!", iName.Str()));

                CommandComplete(iInputCommands, aCmd, PVMFErrNoResources);
                return;
            }


            /* Allocate input buffers */
            if (!CreateInputMemPool(iNumInputBuffers))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "%s::DoPrepare() Can't allocate mempool for input buffers!", iName.Str()));

                CommandComplete(iInputCommands, aCmd, PVMFErrNoResources);
                return;
            }

            in_ctrl_struct_ptr = NULL;
            in_buff_hdr_ptr = NULL;

            in_ctrl_struct_ptr = (OsclAny **) oscl_malloc(iNumInputBuffers * sizeof(OsclAny *));

            if (in_ctrl_struct_ptr == NULL)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "%s::DoPrepare() in_ctrl_struct_ptr == NULL"));

                CommandComplete(iInputCommands, aCmd, PVMFErrNoResources);
                return ;
            }

            in_buff_hdr_ptr = (OsclAny **) oscl_malloc(iNumInputBuffers * sizeof(OsclAny *));

            if (in_buff_hdr_ptr == NULL)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "%s::DoPrepare() in_buff_hdr_ptr == NULL"));

                CommandComplete(iInputCommands, aCmd, PVMFErrNoResources);
                return ;
            }

            if (!ProvideBuffersToComponent(iInBufMemoryPool, // allocator
                                           iInputAllocSize,	 // size to allocate from pool (hdr only or hdr+ buffer)
                                           iNumInputBuffers, // number of buffers
                                           iOMXComponentInputBufferSize, // actual buffer size
                                           iInputPortIndex, // port idx
                                           iOMXComponentSupportsExternalInputBufferAlloc, // can component use OMX_UseBuffer
                                           true // this is input
                                          ))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "%s::DoPrepare() Component can't use input buffers!", iName.Str()));

                CommandComplete(iInputCommands, aCmd, PVMFErrNoResources);
                return;
            }


            /* Allocate output buffers */
            if (!CreateOutMemPool(iNumOutputBuffers))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "%s::DoPrepare() Can't allocate mempool for output buffers!", iName.Str()));

                CommandComplete(iInputCommands, aCmd, PVMFErrNoResources);
                return;
            }

            out_ctrl_struct_ptr = NULL;
            out_buff_hdr_ptr = NULL;

            out_ctrl_struct_ptr = (OsclAny **) oscl_malloc(iNumOutputBuffers * sizeof(OsclAny *));

            if (out_ctrl_struct_ptr == NULL)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "%s::DoPrepare() out_ctrl_struct_ptr == NULL"));

                CommandComplete(iInputCommands, aCmd, PVMFErrNoResources);
                return ;
            }

            out_buff_hdr_ptr = (OsclAny **) oscl_malloc(iNumOutputBuffers * sizeof(OsclAny *));

            if (out_buff_hdr_ptr == NULL)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "%s::DoPrepare()  out_buff_hdr_ptr == NULL"));

                CommandComplete(iInputCommands, aCmd, PVMFErrNoResources);
                return ;
            }


            if (!ProvideBuffersToComponent(iOutBufMemoryPool, // allocator
                                           iOutputAllocSize,	 // size to allocate from pool (hdr only or hdr+ buffer)
                                           iNumOutputBuffers, // number of buffers
                                           iOMXComponentOutputBufferSize, // actual buffer size
                                           iOutputPortIndex, // port idx
                                           iOMXComponentSupportsExternalOutputBufferAlloc, // can component use OMX_UseBuffer
                                           false // this is not input
                                          ))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "%s::DoPrepare() Component can't use output buffers!", iName.Str()));

                CommandComplete(iInputCommands, aCmd, PVMFErrNoResources);
                return;
            }


            //this command is asynchronous.  move the command from
            //the input command queue to the current command, where
            //it will remain until it completes. We have to wait for
            // OMX component state transition to complete

            int32 err;
            OSCL_TRY(err, iCurrentCommand.StoreL(aCmd););
            if (err != OsclErrNone)
            {
                CommandComplete(iInputCommands, aCmd, PVMFErrNoMemory);
                return;
            }
            iInputCommands.Erase(&aCmd);

        }
        break;
        case EPVMFNodePrepared:
            CommandComplete(iInputCommands, aCmd, PVMFSuccess);
            break;
        default:
            CommandComplete(iInputCommands, aCmd, PVMFErrInvalidState);
            break;
    }

}

/////////////////////////////////////////////////////////////////////////////
void PVMFOMXBaseDecNode::DoStart(PVMFOMXBaseDecNodeCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "%s::DoStart() In", iName.Str()));

    iDiagnosticsLogged = false;

    PVMFStatus status = PVMFSuccess;

    OMX_ERRORTYPE  err;
    OMX_STATETYPE sState;

    switch (iInterfaceState)
    {
        case EPVMFNodePrepared:
        case EPVMFNodePaused:
        {
            //Get state of OpenMAX decoder
            err = OMX_GetState(iOMXDecoder, &sState);
            if (err != OMX_ErrorNone)
            {
                //Error condition report
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,

                                (0, "%s::DoStart(): Can't get State of decoder!", iName.Str()));

                sState = OMX_StateInvalid;
            }

            if ((sState == OMX_StateIdle) || (sState == OMX_StatePause))
            {
                /* Change state to OMX_StateExecuting form OMX_StateIdle. */
                // init the flag
                if (!iDynamicReconfigInProgress)
                {

                    iDoNotSendOutputBuffersDownstreamFlag = false; // or if output was not being sent downstream due to state changes
                    // re-anable sending output

                    iDoNotSaveInputBuffersFlag = false;

                }

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "%s::DoStart() Changing Component state Idle->Executing", iName.Str()));

                err = OMX_SendCommand(iOMXDecoder, OMX_CommandStateSet, OMX_StateExecuting, NULL);
                if (err != OMX_ErrorNone)
                {
                    //Error condition report
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "%s::DoStart(): Can't send StateSet command to decoder!", iName.Str()));

                    status = PVMFErrInvalidState;
                }

            }
            else
            {
                //Error condition report
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "%s::DoStart(): Decoder is not in the Idle or Pause state!", iName.Str()));

                status = PVMFErrInvalidState;
            }


        }
        break;

        default:
            status = PVMFErrInvalidState;
            break;
    }

    if (status == PVMFErrInvalidState)
    {
        CommandComplete(iInputCommands, aCmd, status);
    }
    else
    {
        //this command is asynchronous.  move the command from
        //the input command queue to the current command, where
        //it will remain until it completes.
        int32 err;
        OSCL_TRY(err, iCurrentCommand.StoreL(aCmd););
        if (err != OsclErrNone)
        {
            CommandComplete(iInputCommands, aCmd, PVMFErrNoMemory);
        }
        iInputCommands.Erase(&aCmd);
    }
}

/////////////////////////////////////////////////////////////////////////////
void PVMFOMXBaseDecNode::DoStop(PVMFOMXBaseDecNodeCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "%s::DoStop() In", iName.Str()));

    LogDiagnostics();

    OMX_ERRORTYPE  err;
    OMX_STATETYPE sState;

    switch (iInterfaceState)
    {
        case EPVMFNodeStarted:
        case EPVMFNodePaused:
        case EPVMFNodePrepared:
            // Stop data source
            // This will also prevent execution of HandleProcessingState

            iDataIn.Unbind();
            // Clear queued messages in ports
            if (iInPort)
            {
                iInPort->ClearMsgQueues();
            }

            if (iOutPort)
            {
                iOutPort->ClearMsgQueues();
            }

            // Clear the data flags

            iEndOfDataReached = false;
            iIsEOSSentToComponent = false;
            iIsEOSReceivedFromComponent = false;


            iDoNotSendOutputBuffersDownstreamFlag = true; // stop sending output buffers downstream
            iDoNotSaveInputBuffersFlag = true;

            //if we're in the middle of a partial frame assembly
            // abandon it and start fresh
            if (iObtainNewInputBuffer == false)
            {
                if (iInputBufferUnderConstruction != NULL)
                {
                    if (iInBufMemoryPool != NULL)
                    {
                        iInBufMemoryPool->deallocate((OsclAny *)iInputBufferUnderConstruction);
                    }
                    iInputBufferUnderConstruction = NULL;
                }
                iObtainNewInputBuffer = true;

            }

            iFirstDataMsgAfterBOS = true;

            //Get state of OpenMAX decoder
            err = OMX_GetState(iOMXDecoder, &sState);
            if (err != OMX_ErrorNone)
            {
                //Error condition report
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "%s::DoStop(): Can't get State of decoder!", iName.Str()));

                sState = OMX_StateInvalid;
            }

            if ((sState == OMX_StateExecuting) || (sState == OMX_StatePause))
            {
                /* Change state to OMX_StateIdle from OMX_StateExecuting or OMX_StatePause. */

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "%s::DoStop() Changing Component State Executing->Idle or Pause->Idle", iName.Str()));

                err = OMX_SendCommand(iOMXDecoder, OMX_CommandStateSet, OMX_StateIdle, NULL);
                if (err != OMX_ErrorNone)
                {
                    //Error condition report
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "%s::DoStop(): Can't send StateSet command to decoder!", iName.Str()));

                    CommandComplete(iInputCommands, aCmd, PVMFErrInvalidState);
                    break;
                }

                // prevent the node from sending more buffers etc.
                // if port reconfiguration is in process, let the state remain one of the port config states
                //	if there is a start command, we can do it seemlessly (by continuing the port reconfig)
                if (iProcessingState == EPVMFOMXBaseDecNodeProcessingState_ReadyToDecode)
                    iProcessingState = EPVMFOMXBaseDecNodeProcessingState_Stopping;

                // indicate that stop cmd was sent
                if (iDynamicReconfigInProgress)
                {
                    iStopCommandWasSentToComponent = true;
                }


            }
            else
            {
                //Error condition report
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "%s::DoStop(): Decoder is not in the Executing or Pause state!", iName.Str()));

                CommandComplete(iInputCommands, aCmd, PVMFErrInvalidState);
                break;
            }

            //this command is asynchronous.  move the command from
            //the input command queue to the current command, where
            //it will remain until it completes.
            int32 err;
            OSCL_TRY(err, iCurrentCommand.StoreL(aCmd););
            if (err != OsclErrNone)
            {
                CommandComplete(iInputCommands, aCmd, PVMFErrNoMemory);
                return;
            }
            iInputCommands.Erase(&aCmd);

            break;

        default:
            CommandComplete(iInputCommands, aCmd, PVMFErrInvalidState);
            break;
    }
}

/////////////////////////////////////////////////////////////////////////////
void PVMFOMXBaseDecNode::DoFlush(PVMFOMXBaseDecNodeCommand& aCmd)
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
            if (iInPort)
            {
                iInPort->SuspendInput();
            }
            if (iOutPort)
            {
                iOutPort->SuspendInput();
            }
            // Stop data source

            // DV: Sending "OMX_CommandFlush" to the decoder: Not supported yet

            break;

        default:
            CommandComplete(iInputCommands, aCmd, PVMFErrInvalidState);
            break;
    }
}

/////////////////////////////////////////////////////////////////////////////
void PVMFOMXBaseDecNode::DoPause(PVMFOMXBaseDecNodeCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "%s::DoPause() In", iName.Str()));

    OMX_ERRORTYPE  err;
    OMX_STATETYPE sState;

    switch (iInterfaceState)
    {
        case EPVMFNodeStarted:


            //Get state of OpenMAX decoder
            err = OMX_GetState(iOMXDecoder, &sState);
            if (err != OMX_ErrorNone)
            {
                //Error condition report
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "%s::DoPause(): Can't get State of decoder!", iName.Str()));

                sState = OMX_StateInvalid;
            }

            if (sState == OMX_StateExecuting)
            {
                /* Change state to OMX_StatePause from OMX_StateExecuting. */
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "%s::DoPause() Changing Component State Executing->Idle", iName.Str()));


                // prevent the node from sending more buffers etc.
                // if port reconfiguration is in process, let the state remain one of the port config states
                //	if there is a start command, we can do it seemlessly (by continuing the port reconfig)
                if (iProcessingState == EPVMFOMXBaseDecNodeProcessingState_ReadyToDecode)
                    iProcessingState = EPVMFOMXBaseDecNodeProcessingState_Pausing;

                // indicate that pause cmd was sent
                if (iDynamicReconfigInProgress)
                {
                    iPauseCommandWasSentToComponent = true;
                }

                err = OMX_SendCommand(iOMXDecoder, OMX_CommandStateSet, OMX_StatePause, NULL);
                if (err != OMX_ErrorNone)
                {
                    //Error condition report
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "%s::DoPause(): Can't send StateSet command to decoder!", iName.Str()));

                    CommandComplete(iInputCommands, aCmd, PVMFErrInvalidState);
                    break;
                }

            }
            else
            {
                //Error condition report
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "%s::DoPause(): Decoder is not in the Executing state!", iName.Str()));
                CommandComplete(iInputCommands, aCmd, PVMFErrInvalidState);
                break;
            }

            //this command is asynchronous.  move the command from
            //the input command queue to the current command, where
            //it will remain until it completes.
            int32 err;
            OSCL_TRY(err, iCurrentCommand.StoreL(aCmd););
            if (err != OsclErrNone)
            {
                CommandComplete(iInputCommands, aCmd, PVMFErrNoMemory);
                return;
            }
            iInputCommands.Erase(&aCmd);

            break;
        default:
            CommandComplete(iInputCommands, aCmd, PVMFErrInvalidState);
            break;
    }
}

/////////////////////////////////////////////////////////////////////////////
void PVMFOMXBaseDecNode::DoReset(PVMFOMXBaseDecNodeCommand& aCmd)
{

    OMX_ERRORTYPE  err;
    OMX_STATETYPE sState;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "%s::DoReset() In", iName.Str()));

    LogDiagnostics();

    switch (iInterfaceState)
    {
        case EPVMFNodeIdle:
        case EPVMFNodeInitialized:
        case EPVMFNodePrepared:
        case EPVMFNodeStarted:
        case EPVMFNodePaused:
        case EPVMFNodeError:
        {
            //Check if decoder is initilized
            if (iOMXDecoder != NULL)
            {

                //if we're in the middle of a partial frame assembly
                // abandon it and start fresh
                if (iObtainNewInputBuffer == false)
                {
                    if (iInputBufferUnderConstruction != NULL)
                    {
                        if (iInBufMemoryPool != NULL)
                        {
                            iInBufMemoryPool->deallocate((OsclAny *)iInputBufferUnderConstruction);
                        }
                        iInputBufferUnderConstruction = NULL;
                    }
                    iObtainNewInputBuffer = true;

                }

                iFirstDataMsgAfterBOS = true;
                iKeepDroppingMsgsUntilMarkerBit = false;

                //Get state of OpenMAX decoder
                err = OMX_GetState(iOMXDecoder, &sState);
                if (err != OMX_ErrorNone)
                {
                    //Error condition report
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "%s::DoReset(): Can't get State of decoder!", iName.Str()));
                    if (iResetInProgress)
                    {
                        // cmd is in current q
                        iResetInProgress = false;
                        if ((iCurrentCommand.size() > 0) &&
                                (iCurrentCommand.front().iCmd == PVMFOMXBaseDecNodeCommand::PVOMXBASEDEC_NODE_CMD_RESET)
                           )
                        {
                            CommandComplete(iCurrentCommand, iCurrentCommand.front(), PVMFErrResource);
                        }

                    }
                    else
                    {
                        CommandComplete(iInputCommands, aCmd, PVMFErrResource);
                    }
                    return;
                }

                if (sState == OMX_StateLoaded)
                {
                    // this is a value obtained by synchronous call to component. Either the component was
                    // already in this state without node issuing any commands,
                    // or perhaps we started the Reset, but the callback notification has not yet arrived.
                    if (iResetInProgress)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                        (0, "%s::DoReset() OMX comp is in loaded state. Wait for official callback to change variables etc.", iName.Str()));
                        return;
                    }
                    else
                    {

                        //CommandComplete(iInputCommands, aCmd, PVMFErrResource);
                        //delete all ports and notify observer.
                        if (iInPort)
                        {
                            OSCL_DELETE(((PVMFOMXDecPort*)iInPort));
                            iInPort = NULL;
                        }

                        if (iOutPort)
                        {
                            OSCL_DELETE(((PVMFOMXDecPort*)iOutPort));
                            iOutPort = NULL;
                        }

                        iDataIn.Unbind();


                        // Reset the metadata key list
                        iAvailableMetadataKeys.clear();

                        iEndOfDataReached = false;
                        iIsEOSSentToComponent = false;
                        iIsEOSReceivedFromComponent = false;


                        iProcessingState = EPVMFOMXBaseDecNodeProcessingState_Idle;

                        SetState(EPVMFNodeIdle);

                        CommandComplete(iInputCommands, aCmd, PVMFSuccess);

                        return;
                    }
                }

                if (sState == OMX_StateIdle)
                {


                    //this command is asynchronous.  move the command from
                    //the input command queue to the current command, where
                    //it will remain until it is completed.
                    if (!iResetInProgress)
                    {
                        int32 err;
                        OSCL_TRY(err, iCurrentCommand.StoreL(aCmd););
                        if (err != OsclErrNone)
                        {
                            CommandComplete(iInputCommands, aCmd, PVMFErrNoMemory);
                            return;
                        }
                        iInputCommands.Erase(&aCmd);

                        iResetInProgress = true;
                    }

                    // if buffers aren't all back (due to timing issues with different callback AOs
                    //		state change can be reported before all buffers are returned)
                    if (iNumOutstandingInputBuffers > 0 || iNumOutstandingOutputBuffers > 0)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                        (0, "%s::DoReset() Waiting for %d input and-or %d output buffers", iName.Str(), iNumOutstandingInputBuffers, iNumOutstandingOutputBuffers));

                        return;
                    }

                    if (!iResetMsgSent)
                    {
                        // We can come here only if all buffers are already back
                        // Don't repeat any of this twice.
                        /* Change state to OMX_StateLoaded form OMX_StateIdle. */
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                        (0, "%s::DoReset() Changing Component State Idle->Loaded", iName.Str()));

                        err = OMX_SendCommand(iOMXDecoder, OMX_CommandStateSet, OMX_StateLoaded, NULL);
                        if (err != OMX_ErrorNone)
                        {
                            //Error condition report
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                            (0, "%s::DoReset(): Can't send StateSet command to decoder!", iName.Str()));
                        }

                        iResetMsgSent = true;


                        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                        (0, "%s::DoReset() freeing output buffers", iName.Str()));

                        if (false == iOutputBuffersFreed)
                        {
                            if (!FreeBuffersFromComponent(iOutBufMemoryPool, // allocator
                                                          iOutputAllocSize,	 // size to allocate from pool (hdr only or hdr+ buffer)
                                                          iNumOutputBuffers, // number of buffers
                                                          iOutputPortIndex, // port idx
                                                          false // this is not input
                                                         ))
                            {
                                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                                (0, "%s::DoReset() Cannot free output buffers ", iName.Str()));

                                if (iResetInProgress)
                                {
                                    iResetInProgress = false;
                                    if ((iCurrentCommand.size() > 0) &&
                                            (iCurrentCommand.front().iCmd == PVMFOMXBaseDecNodeCommand::PVOMXBASEDEC_NODE_CMD_RESET)
                                       )
                                    {
                                        CommandComplete(iCurrentCommand, iCurrentCommand.front() , PVMFErrResource);
                                    }
                                }

                            }

                        }
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                        (0, "%s::DoReset() freeing input buffers ", iName.Str()));
                        if (false == iInputBuffersFreed)
                        {
                            if (!FreeBuffersFromComponent(iInBufMemoryPool, // allocator
                                                          iInputAllocSize,	 // size to allocate from pool (hdr only or hdr+ buffer)
                                                          iNumInputBuffers, // number of buffers
                                                          iInputPortIndex, // port idx
                                                          true // this is input
                                                         ))
                            {
                                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                                (0, "%s::DoReset() Cannot free input buffers ", iName.Str()));

                                if (iResetInProgress)
                                {
                                    iResetInProgress = false;
                                    if ((iCurrentCommand.size() > 0) &&
                                            (iCurrentCommand.front().iCmd == PVMFOMXBaseDecNodeCommand::PVOMXBASEDEC_NODE_CMD_RESET)
                                       )
                                    {
                                        CommandComplete(iCurrentCommand, iCurrentCommand.front(), PVMFErrResource);
                                    }
                                }


                            }
                        }



                        iEndOfDataReached = false;
                        iIsEOSSentToComponent = false;
                        iIsEOSReceivedFromComponent = false;



                        // also, perform Port deletion when the component replies with the command
                        // complete, not right here
                    } // end of if(iResetMsgSent)


                    return;

                }

                if ((sState == OMX_StateExecuting) || (sState == OMX_StatePause))
                {
                    //this command is asynchronous.  move the command from
                    //the input command queue to the current command, where
                    //it will remain until it is completed.
                    if (!iResetInProgress)
                    {
                        int32 err;
                        OSCL_TRY(err, iCurrentCommand.StoreL(aCmd););
                        if (err != OsclErrNone)
                        {
                            CommandComplete(iInputCommands, aCmd, PVMFErrNoMemory);
                            return;
                        }
                        iInputCommands.Erase(&aCmd);

                        iResetInProgress = true;
                    }

                    /* Change state to OMX_StateIdle from OMX_StateExecuting or OMX_StatePause. */

                    if (!iStopInResetMsgSent)
                    {
                        // replicate behavior of stop cmd
                        iDataIn.Unbind();
                        // Clear queued messages in ports
                        if (iInPort)
                        {
                            iInPort->ClearMsgQueues();
                        }

                        if (iOutPort)
                        {
                            iOutPort->ClearMsgQueues();
                        }

                        // Clear the data flags

                        iEndOfDataReached = false;
                        iIsEOSSentToComponent = false;
                        iIsEOSReceivedFromComponent = false;


                        iDoNotSendOutputBuffersDownstreamFlag = true; // stop sending output buffers downstream
                        iDoNotSaveInputBuffersFlag = true;


                        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                        (0, "%s::DoReset() Changing Component State Executing->Idle or Pause->Idle", iName.Str()));

                        err = OMX_SendCommand(iOMXDecoder, OMX_CommandStateSet, OMX_StateIdle, NULL);
                        if (err != OMX_ErrorNone)
                        {
                            //Error condition report
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                            (0, "%s::DoReset(): Can't send StateSet command to decoder!", iName.Str()));

                            CommandComplete(iInputCommands, aCmd, PVMFErrInvalidState);
                            break;
                        }

                        iStopInResetMsgSent = true;
                        // prevent the node from sending more buffers etc.
                        // if port reconfiguration is in process, let the state remain one of the port config states
                        //	if there is a start command, we can do it seemlessly (by continuing the port reconfig)
                        if (iProcessingState == EPVMFOMXBaseDecNodeProcessingState_ReadyToDecode)
                            iProcessingState = EPVMFOMXBaseDecNodeProcessingState_Stopping;
                    }

                    return;

                }
                else
                {
                    //Error condition report
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "%s::DoReset(): Decoder is not in the Idle state!", iName.Str()));
                    if (iResetInProgress)
                    {
                        iResetInProgress = false;
                        if ((iCurrentCommand.size() > 0) &&
                                (iCurrentCommand.front().iCmd == PVMFOMXBaseDecNodeCommand::PVOMXBASEDEC_NODE_CMD_RESET)
                           )
                        {
                            CommandComplete(iCurrentCommand, iCurrentCommand.front(), PVMFErrInvalidState);
                        }
                    }
                    else
                    {
                        CommandComplete(iInputCommands, aCmd, PVMFErrInvalidState);
                    }
                    break;
                }//end of if (sState == OMX_StateIdle)
            }//end of if (iOMXDecoder != NULL)

            //delete all ports and notify observer.
            if (iInPort)
            {
                OSCL_DELETE(((PVMFOMXDecPort*)iInPort));
                iInPort = NULL;
            }

            if (iOutPort)
            {
                OSCL_DELETE(((PVMFOMXDecPort*)iOutPort));
                iOutPort = NULL;
            }

            iDataIn.Unbind();


            // Reset the metadata key list
            iAvailableMetadataKeys.clear();

            iEndOfDataReached = false;
            iIsEOSSentToComponent = false;
            iIsEOSReceivedFromComponent = false;

            if (iOMXComponentUsesFullAVCFrames)
            {
                iNALCount = 0;
                oscl_memset(iNALSizeArray, 0, sizeof(uint32) * 100); // 100 is max number of NALs
            }

            iProcessingState = EPVMFOMXBaseDecNodeProcessingState_Idle;
            //logoff & go back to Created state.
            SetState(EPVMFNodeIdle);


            if (iResetInProgress)
            {
                iResetInProgress = false;
                if ((iCurrentCommand.size() > 0) &&
                        (iCurrentCommand.front().iCmd == PVMFOMXBaseDecNodeCommand::PVOMXBASEDEC_NODE_CMD_RESET)
                   )
                {
                    CommandComplete(iCurrentCommand, iCurrentCommand.front(), PVMFSuccess);
                }
            }
            else
            {
                CommandComplete(iInputCommands, aCmd, PVMFSuccess);
            }

        }
        break;

        default:
            CommandComplete(iInputCommands, aCmd, PVMFErrInvalidState);
            break;
    }
}

/////////////////////////////////////////////////////////////////////////////
// Clean Up Decoder
/////////////////////////////////////////////////////////////////////////////
bool PVMFOMXBaseDecNode::DeleteOMXBaseDecoder()
{
    OMX_ERRORTYPE  err;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "%s::DeleteOMXBaseDecoder() In", iName.Str()));

    if (iOMXDecoder != NULL)
    {
        /* Free Component handle. */
        err = OMX_FreeHandle(iOMXDecoder);
        if (err != OMX_ErrorNone)
        {
            //Error condition report
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "%s::DeleteOMXBaseDecoder(): Can't free decoder's handle!", iName.Str()));
        }
        iOMXDecoder = NULL;

    }//end of if (iOMXDecoder != NULL)


    return true;
}

/////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PVMFOMXBaseDecNode::ChangeNodeState(TPVMFNodeInterfaceState aNewState)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "%s::ChangeNodeState() Changing state from %d to %d", iName.Str(), iInterfaceState, aNewState));
    iInterfaceState = aNewState;
}

/////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PVMFOMXBaseDecNode::freechunkavailable(OsclAny *aContext)
{

    // check context to see whether input or output buffer was returned to the mempool
    if (aContext == (OsclAny *) iInBufMemoryPool)
    {

        iNumOutstandingInputBuffers--;

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "%s::freechunkavailable() Memory chunk in INPUT mempool was deallocated, %d out of %d now available", iName.Str(), iNumInputBuffers - iNumOutstandingInputBuffers, iNumInputBuffers));

        // notification only works once.
        // If there are multiple buffers coming back in a row, make sure to set the notification
        // flag in the mempool again, so that next buffer also causes notification
        iInBufMemoryPool->notifyfreechunkavailable(*this, aContext);

    }
    else if (aContext == (OsclAny *) iOutBufMemoryPool)
    {

        iNumOutstandingOutputBuffers--;
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "%s::freechunkavailable() Memory chunk in OUTPUT mempool was deallocated, %d out of %d now available", iName.Str(), iNumOutputBuffers - iNumOutstandingOutputBuffers, iNumOutputBuffers));

        // notification only works once.
        // If there are multiple buffers coming back in a row, make sure to set the notification
        // flag in the mempool again, so that next buffer also causes notification
        iOutBufMemoryPool->notifyfreechunkavailable(*this, aContext);

    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "%s::freechunkavailable() UNKNOWN mempool ", iName.Str()));

    }

    // reschedule
    if (IsAdded())
        RunIfNotReady();


}

/////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PVMFOMXBaseDecNode::HandlePortActivity(const PVMFPortActivity &aActivity)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "0x%x PVMFOMXBaseDecNode::PortActivity: port=0x%x, type=%d",
                     this, aActivity.iPort, aActivity.iType));

    switch (aActivity.iType)
    {
        case PVMF_PORT_ACTIVITY_OUTGOING_MSG:
            //An outgoing message was queued on this port.
            //We only need to queue a port activity event on the
            //first message.  Additional events will be queued during
            //the port processing as needed.
            if (aActivity.iPort->OutgoingMsgQueueSize() == 1)
            {
                //wake up the AO to process the port activity event.
                RunIfNotReady();
            }
            break;

        case PVMF_PORT_ACTIVITY_INCOMING_MSG:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "%s::PortActivity: IncomingMsgQueueSize=%d", iName.Str(), aActivity.iPort->IncomingMsgQueueSize()));
            if (aActivity.iPort->IncomingMsgQueueSize() == 1)
            {
                //wake up the AO to process the port activity event.
                RunIfNotReady();
            }
            break;

        case PVMF_PORT_ACTIVITY_OUTGOING_QUEUE_READY:
            if (iProcessingState == EPVMFOMXBaseDecNodeProcessingState_WaitForOutgoingQueue)
            {
                iProcessingState = EPVMFOMXBaseDecNodeProcessingState_ReadyToDecode;
                RunIfNotReady();
            }
            break;

        case PVMF_PORT_ACTIVITY_CONNECT:
            //nothing needed.
            break;

        case PVMF_PORT_ACTIVITY_DISCONNECT:
            //clear the node input data when either port is disconnected.

            iDataIn.Unbind();
            break;

        case PVMF_PORT_ACTIVITY_CONNECTED_PORT_BUSY:
            // The connected port has become busy (its incoming queue is
            // busy).
            // No action is needed here-- the port processing code
            // checks for connected port busy during data processing.
            break;

        case PVMF_PORT_ACTIVITY_CONNECTED_PORT_READY:
            // The connected port has transitioned from Busy to Ready to Receive.
            // It's time to start processing outgoing messages again.

            //iProcessingState should transition from WaitForOutputPort to ReadyToDecode
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "0x%x PVMFOMXBaseDecNode::PortActivity: Connected port is now ready", this));
            RunIfNotReady();
            break;

        default:
            break;
    }
}

/////////////////////////////////////////////////////////////////////////////
void PVMFOMXBaseDecNode::DoCancelAllCommands(PVMFOMXBaseDecNodeCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "%s::DoCancelAllCommands", iName.Str()));

    //first cancel the current command if any
    {
        while (!iCurrentCommand.empty())
        {
            CommandComplete(iCurrentCommand, iCurrentCommand[0], PVMFErrCancelled);
        }

    }

    //next cancel all queued commands
    {
        //start at element 1 since this cancel command is element 0.
        while (iInputCommands.size() > 1)
        {
            CommandComplete(iInputCommands, iInputCommands[1], PVMFErrCancelled);
        }
    }

    if (iResetInProgress && !iResetMsgSent)
    {
        // if reset is started but reset msg has not been sent, we can cancel reset
        // as if nothing happened. Otherwise, the callback will set the flag back to false
        iResetInProgress = false;
    }
    //finally, report cancel complete.
    CommandComplete(iInputCommands, aCmd, PVMFSuccess);
}

/////////////////////////////////////////////////////////////////////////////
void PVMFOMXBaseDecNode::DoCancelCommand(PVMFOMXBaseDecNodeCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "%s::DoCancelCommand", iName.Str()));

    //extract the command ID from the parameters.
    PVMFCommandId id;
    aCmd.PVMFOMXBaseDecNodeCommandBase::Parse(id);

    //first check "current" command if any
    {
        PVMFOMXBaseDecNodeCommand* cmd = iCurrentCommand.FindById(id);
        if (cmd)
        {

            // if reset is being canceled:
            if (cmd->iCmd == PVMFOMXBaseDecNodeCommand::PVOMXBASEDEC_NODE_CMD_RESET)
            {
                if (iResetInProgress && !iResetMsgSent)
                {
                    // if reset is started but reset msg has not been sent, we can cancel reset
                    // as if nothing happened. Otherwise, the callback will set the flag back to false
                    iResetInProgress = false;
                }
            }
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
        PVMFOMXBaseDecNodeCommand* cmd = iInputCommands.FindById(id, 1);
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
    CommandComplete(iInputCommands, aCmd, PVMFErrArgument);
}

/////////////////////////////////////////////////////////////////////////////
void PVMFOMXBaseDecNode::DoQueryInterface(PVMFOMXBaseDecNodeCommand&  aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "%s::DoQueryInterface", iName.Str()));
    PVUuid* uuid;
    PVInterface** ptr;
    aCmd.PVMFOMXBaseDecNodeCommandBase::Parse(uuid, ptr);

    if (*uuid == PVUuid(PVMF_OMX_BASE_DEC_NODE_CUSTOM1_UUID))
    {
        addRef();
        *ptr = (PVMFOMXBaseDecNodeExtensionInterface*)this;
        CommandComplete(iInputCommands, aCmd, PVMFSuccess);
    }
    else if (*uuid == PVUuid(KPVMFMetadataExtensionUuid))
    {
        addRef();
        *ptr = (PVMFMetadataExtensionInterface*)this;
        CommandComplete(iInputCommands, aCmd, PVMFSuccess);
    }
    else if (*uuid == PVUuid(PVMI_CAPABILITY_AND_CONFIG_PVUUID))
    {
        addRef();
        *ptr = (PvmiCapabilityAndConfig*)this;
        CommandComplete(iInputCommands, aCmd, PVMFSuccess);
    }
    else
    {
        //not supported
        *ptr = NULL;
        CommandComplete(iInputCommands, aCmd, PVMFFailure);
    }
}


/////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PVMFOMXBaseDecNode::addRef()
{
    ++iExtensionRefCount;
}

/////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PVMFOMXBaseDecNode::removeRef()
{
    --iExtensionRefCount;
}

/////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF bool PVMFOMXBaseDecNode::queryInterface(const PVUuid& uuid, PVInterface*& iface)
{
    PVUuid my_uuid(PVMF_OMX_BASE_DEC_NODE_CUSTOM1_UUID);
    if (uuid == my_uuid)
    {
        PVMFOMXBaseDecNodeExtensionInterface* myInterface = OSCL_STATIC_CAST(PVMFOMXBaseDecNodeExtensionInterface*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
        ++iExtensionRefCount;
        return true;
    }
    else if (uuid == KPVMFMetadataExtensionUuid)
    {
        PVMFMetadataExtensionInterface* myInterface = OSCL_STATIC_CAST(PVMFMetadataExtensionInterface*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
        ++iExtensionRefCount;
        return true;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////
bool PVMFOMXBaseDecNode::HandleRepositioning()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "%s::HandleRepositioning() IN", iName.Str()));


    // 1) Send Flush command to component for both input and output ports
    // 2) "Wait" until component flushes both ports
    // 3) Resume
    OMX_ERRORTYPE  err = OMX_ErrorNone;
    OMX_STATETYPE sState = OMX_StateInvalid;


    if (!iIsRepositioningRequestSentToComponent)
    {

        // first check the state (if executing or paused, continue)
        err = OMX_GetState(iOMXDecoder, &sState);
        if (err != OMX_ErrorNone)
        {
            //Error condition report
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "%s::HandleRepositioning(): Can't get State of decoder - trying to send reposition request!", iName.Str()));

            sState = OMX_StateInvalid;
            ReportErrorEvent(PVMFErrResourceConfiguration);
            ChangeNodeState(EPVMFNodeError);
            return false;
        }

        if ((sState != OMX_StateExecuting) && (sState != OMX_StatePause))
        {

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "%s::HandleRepositioning() Component State is not executing or paused, do not proceed with repositioning", iName.Str()));

            return true;

        }


        iIsRepositioningRequestSentToComponent = true; // prevent sending requests multiple times
        iIsInputPortFlushed = false;	// flag that will be set to true once component flushes the port
        iIsOutputPortFlushed = false;
        iDoNotSendOutputBuffersDownstreamFlag = true;

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "%s::HandleRepositioning() Sending Flush command to component", iName.Str()));

        // send command to flush all ports (arg is OMX_ALL)
        err = OMX_SendCommand(iOMXDecoder, OMX_CommandFlush, OMX_ALL, NULL);
        if (err != OMX_ErrorNone)
        {
            //Error condition report
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "%s::HandleRepositioning(): Can't send flush command  - trying to send reposition request!", iName.Str()));

            sState = OMX_StateInvalid;
            ReportErrorEvent(PVMFErrResourceConfiguration);
            ChangeNodeState(EPVMFNodeError);
            return false;
        }

    }

    if (iIsRepositionDoneReceivedFromComponent)
    {

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "%s::HandleRepositioning() Component has flushed both ports, and is done repositioning", iName.Str()));

        iIsRepositioningRequestSentToComponent = false; // enable sending requests again
        iIsRepositionDoneReceivedFromComponent = false;
        iIsInputPortFlushed = false;
        iIsOutputPortFlushed = false;

        iDoNotSendOutputBuffersDownstreamFlag = false;
        return true;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "%s::HandleRepositioning() Component is not yet done repositioning ", iName.Str()));

    return false;

}

///////////////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PVMFOMXBaseDecNode::setObserver(PvmiConfigAndCapabilityCmdObserver* aObserver)
{
    OSCL_UNUSED_ARG(aObserver);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "%s::setObserver()", iName.Str()));
    // This method is not supported so leave
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "%s::setObserver() is not supported!", iName.Str()));
    OSCL_LEAVE(PVMFErrNotSupported);
}


OSCL_EXPORT_REF PVMFStatus PVMFOMXBaseDecNode::getParametersSync(PvmiMIOSession aSession, PvmiKeyType aIdentifier, PvmiKvp*& aParameters, int& aNumParamElements, PvmiCapabilityContext aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "%s::getParametersSync()", iName.Str()));
    OSCL_UNUSED_ARG(aSession);

    return DoCapConfigGetParametersSync(aIdentifier, aParameters, aNumParamElements, aContext);
}


OSCL_EXPORT_REF PVMFStatus PVMFOMXBaseDecNode::releaseParameters(PvmiMIOSession aSession, PvmiKvp* aParameters, int aNumElements)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "%s::releaseParameters()", iName.Str()));
    OSCL_UNUSED_ARG(aSession);

    return DoCapConfigReleaseParameters(aParameters, aNumElements);
}


OSCL_EXPORT_REF void PVMFOMXBaseDecNode::createContext(PvmiMIOSession aSession, PvmiCapabilityContext& aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "%s::createContext()", iName.Str()));
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aContext);
    // This method is not supported so leave
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "%s::createContext() is not supported!", iName.Str()));
    OSCL_LEAVE(PVMFErrNotSupported);
}


OSCL_EXPORT_REF void PVMFOMXBaseDecNode::setContextParameters(PvmiMIOSession aSession, PvmiCapabilityContext& aContext, PvmiKvp* aParameters, int aNumParamElements)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "%s::setContextParameters()", iName.Str()));
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aContext);
    OSCL_UNUSED_ARG(aParameters);
    OSCL_UNUSED_ARG(aNumParamElements);
    // This method is not supported so leave
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "%s::setContextParameters() is not supported!", iName.Str()));
    OSCL_LEAVE(PVMFErrNotSupported);
}


OSCL_EXPORT_REF void PVMFOMXBaseDecNode::DeleteContext(PvmiMIOSession aSession, PvmiCapabilityContext& aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "%s::DeleteContext()", iName.Str()));
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aContext);
    // This method is not supported so leave
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "%s::DeleteContext() is not supported!", iName.Str()));
    OSCL_LEAVE(PVMFErrNotSupported);
}


OSCL_EXPORT_REF void PVMFOMXBaseDecNode::setParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters, int aNumElements, PvmiKvp* &aRetKVP)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "%s::setParametersSync()", iName.Str()));
    OSCL_UNUSED_ARG(aSession);

    // Complete the request synchronously
    DoCapConfigSetParameters(aParameters, aNumElements, aRetKVP);
}


OSCL_EXPORT_REF PVMFCommandId PVMFOMXBaseDecNode::setParametersAsync(PvmiMIOSession aSession, PvmiKvp* aParameters, int aNumElements, PvmiKvp*& aRetKVP, OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "%s::setParametersAsync()", iName.Str()));
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aParameters);
    OSCL_UNUSED_ARG(aNumElements);
    OSCL_UNUSED_ARG(aRetKVP);
    OSCL_UNUSED_ARG(aContext);

    // This method is not supported so leave
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "%s::setParametersAsync() is not supported!", iName.Str()));
    OSCL_LEAVE(PVMFErrNotSupported);
    return 0;
}


OSCL_EXPORT_REF uint32 PVMFOMXBaseDecNode::getCapabilityMetric(PvmiMIOSession aSession)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "%s::getCapabilityMetric()", iName.Str()));
    OSCL_UNUSED_ARG(aSession);
    // Not supported so return 0
    return 0;
}


OSCL_EXPORT_REF PVMFStatus PVMFOMXBaseDecNode::verifyParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters, int aNumElements)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "%s::verifyParametersSync()", iName.Str()));
    OSCL_UNUSED_ARG(aSession);

    return DoCapConfigVerifyParameters(aParameters, aNumElements);
}

/////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMFOMXBaseDecNode::GetNodeMetadataKeys(PVMFSessionId aSessionId, PVMFMetadataList& aKeyList, uint32 starting_index, int32 max_entries, char* query_key, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "%sCommand::GetNodeMetadataKeys() called", iName.Str()));

    PVMFOMXBaseDecNodeCommand cmd;
    cmd.PVMFOMXBaseDecNodeCommand::Construct(aSessionId, PVMFOMXBaseDecNodeCommand::PVOMXBASEDEC_NODE_CMD_GETNODEMETADATAKEY, &aKeyList, starting_index, max_entries, query_key, aContext);
    return QueueCommandL(cmd);
}

/////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMFOMXBaseDecNode::GetNodeMetadataValues(PVMFSessionId aSessionId, PVMFMetadataList& aKeyList, Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList, uint32 starting_index, int32 max_entries, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "%sCommand::GetNodeMetadataValue() called", iName.Str()));

    PVMFOMXBaseDecNodeCommand cmd;
    cmd.PVMFOMXBaseDecNodeCommand::Construct(aSessionId, PVMFOMXBaseDecNodeCommand::PVOMXBASEDEC_NODE_CMD_GETNODEMETADATAVALUE, &aKeyList, &aValueList, starting_index, max_entries, aContext);
    return QueueCommandL(cmd);
}

// From PVMFMetadataExtensionInterface
/////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMFOMXBaseDecNode::ReleaseNodeMetadataKeys(PVMFMetadataList& , uint32 , uint32)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "%s::ReleaseNodeMetadataKeys() called", iName.Str()));
    //nothing needed-- there's no dynamic allocation in this node's key list
    return PVMFSuccess;
}

// From PVMFMetadataExtensionInterface
/////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMFOMXBaseDecNode::ReleaseNodeMetadataValues(Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList, uint32 start, uint32 end)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "%s::ReleaseNodeMetadataValues() called", iName.Str()));

    if (aValueList.size() == 0 || start > end)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "%s::ReleaseNodeMetadataValues() Invalid start/end index", iName.Str()));
        return PVMFErrArgument;
    }

    if (end >= aValueList.size())
    {
        end = aValueList.size() - 1;
    }

    for (uint32 i = start; i <= end; i++)
    {
        if (aValueList[i].key != NULL)
        {
            switch (GetValTypeFromKeyString(aValueList[i].key))
            {
                case PVMI_KVPVALTYPE_CHARPTR:
                    if (aValueList[i].value.pChar_value != NULL)
                    {
                        OSCL_ARRAY_DELETE(aValueList[i].value.pChar_value);
                        aValueList[i].value.pChar_value = NULL;
                    }
                    break;

                case PVMI_KVPVALTYPE_UINT32:
                case PVMI_KVPVALTYPE_UINT8:
                    // No memory to free for these valtypes
                    break;

                default:
                    // Should not get a value that wasn't created from here
                    break;
            }

            OSCL_ARRAY_DELETE(aValueList[i].key);
            aValueList[i].key = NULL;
        }
    }

    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
// CAPABILITY CONFIG PRIVATE
OSCL_EXPORT_REF PVMFStatus PVMFOMXBaseDecNode::DoCapConfigGetParametersSync(PvmiKeyType aIdentifier, PvmiKvp*& aParameters, int& aNumParamElements, PvmiCapabilityContext aContext)
{
    OSCL_UNUSED_ARG(aParameters);
    OSCL_UNUSED_ARG(aIdentifier);
    OSCL_UNUSED_ARG(aNumParamElements);
    OSCL_UNUSED_ARG(aContext);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "%s::DoCapConfigGetParametersSync() In", iName.Str()));

    return PVMFFailure;
}


OSCL_EXPORT_REF PVMFStatus PVMFOMXBaseDecNode::DoCapConfigReleaseParameters(PvmiKvp* aParameters, int aNumElements)
{
    OSCL_UNUSED_ARG(aParameters);
    OSCL_UNUSED_ARG(aNumElements);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "%s::DoCapConfigReleaseParameters() Out", iName.Str()));
    return PVMFSuccess;
}


OSCL_EXPORT_REF void PVMFOMXBaseDecNode::DoCapConfigSetParameters(PvmiKvp* aParameters, int aNumElements, PvmiKvp* &aRetKVP)
{
    OSCL_UNUSED_ARG(aParameters);
    OSCL_UNUSED_ARG(aNumElements);
    OSCL_UNUSED_ARG(aRetKVP);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "%s::DoCapConfigSetParameters() Out", iName.Str()));
}


OSCL_EXPORT_REF PVMFStatus PVMFOMXBaseDecNode::DoCapConfigVerifyParameters(PvmiKvp* aParameters, int aNumElements)
{
    OSCL_UNUSED_ARG(aParameters);
    OSCL_UNUSED_ARG(aNumElements);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "%s::DoCapConfigVerifyParameters() In", iName.Str()));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "%s::DoCapConfigVerifyParameters() Out", iName.Str()));
    return PVMFSuccess;
}


/////////////////////////////////////////////////////////////////////////////
void PVMFOMXBaseDecNode::LogDiagnostics()
{
    if (iDiagnosticsLogged == false)
    {
        iDiagnosticsLogged = true;
        PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iDiagnosticsLogger, PVLOGMSG_INFO, (0, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"));
        PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iDiagnosticsLogger, PVLOGMSG_INFO, (0, "%s - Number of Frames Sent = %d", iName.Str(), iSeqNum));
        PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iDiagnosticsLogger, PVLOGMSG_INFO, (0, "%s - TS of last decoded frame = %d", iName.Str(), iOutTimeStamp));
    }
}


OSCL_EXPORT_REF OsclAny* PVMFOMXBaseDecNode::AllocateKVPKeyArray(int32& aLeaveCode, PvmiKvpValueType aValueType, int32 aNumElements)
{
    int32 leaveCode = OsclErrNone;
    OsclAny* aBuffer = NULL;
    switch (aValueType)
    {
        case PVMI_KVPVALTYPE_WCHARPTR:
            OSCL_TRY(leaveCode,
                     aBuffer = (oscl_wchar*) OSCL_ARRAY_NEW(oscl_wchar, aNumElements);
                    );
            break;

        case PVMI_KVPVALTYPE_CHARPTR:
            OSCL_TRY(leaveCode,
                     aBuffer = (char*) OSCL_ARRAY_NEW(char, aNumElements);
                    );
            break;
        case PVMI_KVPVALTYPE_UINT8PTR:
            OSCL_TRY(leaveCode,
                     aBuffer = (uint8*) OSCL_ARRAY_NEW(uint8, aNumElements);
                    );
            break;
        default:
            break;
    }
    aLeaveCode = leaveCode;
    return aBuffer;
}





