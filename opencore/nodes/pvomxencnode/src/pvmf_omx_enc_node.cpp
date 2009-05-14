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
#include "pvmf_omx_enc_node.h"
#include "pvlogger.h"
#include "oscl_error_codes.h"
#include "pvmf_omx_enc_port.h"
#include "pv_mime_string_utils.h"
#include "oscl_snprintf.h"
#include "pvmf_media_cmd.h"
#include "pvmf_media_msg_format_ids.h"
#include "pvmi_kvp_util.h"

#ifdef _DEBUG
#include <stdio.h>
#endif

#include "OMX_Core.h"
#include "pvmf_omx_enc_callbacks.h"     //used for thin AO in encoder's callbacks
#include "pv_omxcore.h"

#define CONFIG_SIZE_AND_VERSION(param) \
	    param.nSize=sizeof(param); \
	    param.nVersion.s.nVersionMajor = SPECVERSIONMAJOR; \
	    param.nVersion.s.nVersionMinor = SPECVERSIONMINOR; \
	    param.nVersion.s.nRevision = SPECREVISION; \
	    param.nVersion.s.nStep = SPECSTEP;


#define CHECK_OMX_ERR_AND_RETURN(Err, str) \
		if (Err != OMX_ErrorNone) \
		{	\
			PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,	(0, str));	\
		}


#define PVOMXENC_EXTRA_YUVBUFFER_POOLNUM 3
#define PVOMXENC_MEDIADATA_POOLNUM (PVOMXENCMAXNUMDPBFRAMESPLUS1 + PVOMXENC_EXTRA_YUVBUFFER_POOLNUM)
#define PVOMXENC_MEDIADATA_CHUNKSIZE 128


const uint32 DEFAULT_VOL_HEADER_LENGTH = 28;
const uint8 DEFAULT_VOL_HEADER[DEFAULT_VOL_HEADER_LENGTH] =
{
    0x00, 0x00, 0x01, 0xB0, 0x08, 0x00, 0x00, 0x01,
    0xB5, 0x09,	0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
    0x01, 0x20, 0x00, 0x84, 0x40, 0xFA, 0x28, 0x2C,
    0x20, 0x90, 0xA2, 0x1F
};


#ifdef _TEST_AE_ERROR_HANDLING
const uint32 FAIL_NODE_CMD_START = 2;
const uint32 FAIL_NODE_CMD_STOP = 3;
const uint32 FAIL_NODE_CMD_FLUSH = 4;
const uint32 FAIL_NODE_CMD_PAUSE = 5;
const uint32 FAIL_NODE_CMD_RELEASE_PORT = 7;
#endif

#define PVMF_OMXENC_NUM_METADATA_VALUES 6

// Constant character strings for metadata keys
static const char PVOMXENCMETADATA_CODECINFO_VIDEO_FORMAT_KEY[] = "codec-info/video/format";
static const char PVOMXENCMETADATA_CODECINFO_VIDEO_WIDTH_KEY[] = "codec-info/video/width";
static const char PVOMXENCMETADATA_CODECINFO_VIDEO_HEIGHT_KEY[] = "codec-info/video/height";
static const char PVOMXENCMETADATA_CODECINFO_VIDEO_PROFILE_KEY[] = "codec-info/video/profile";
static const char PVOMXENCMETADATA_CODECINFO_VIDEO_LEVEL_KEY[] = "codec-info/video/level";
static const char PVOMXENCMETADATA_CODECINFO_VIDEO_AVGBITRATE_KEY[] = "codec-info/video/avgbitrate";//(bits per sec)


static const char PVOMXENCMETADATA_SEMICOLON[] = ";";

static const char LOG_ID_AUDIO_AMRNB[]  = "Audio_AMRNB";
static const char LOG_ID_AUDIO_AMRWB[]  = "Audio_AMRWB";
static const char LOG_ID_AUDIO_AAC[]  = "Audio_AAC";
static const char LOG_ID_VIDEO_H263[] = "Video_H263";
static const char LOG_ID_VIDEO_M4V[] =  "Video_M4V";
static const char LOG_ID_VIDEO_AVC[] =  "Video_AVC";
static const char LOG_ID_UNKNOWN[] = "TypeNotSetYet";

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
OMX_ERRORTYPE CallbackEventHandlerEnc(OMX_OUT OMX_HANDLETYPE aComponent,
                                      OMX_OUT OMX_PTR aAppData,
                                      OMX_OUT OMX_EVENTTYPE aEvent,
                                      OMX_OUT OMX_U32 aData1,
                                      OMX_OUT OMX_U32 aData2,
                                      OMX_OUT OMX_PTR aEventData)
{

    PVMFOMXEncNode *Node = (PVMFOMXEncNode *) aAppData;

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
OMX_ERRORTYPE CallbackEmptyBufferDoneEnc(OMX_OUT OMX_HANDLETYPE aComponent,
        OMX_OUT OMX_PTR aAppData,
        OMX_OUT OMX_BUFFERHEADERTYPE* aBuffer)
{

    PVMFOMXEncNode *Node = (PVMFOMXEncNode *) aAppData;
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
OMX_ERRORTYPE CallbackFillBufferDoneEnc(OMX_OUT OMX_HANDLETYPE aComponent,
                                        OMX_OUT OMX_PTR aAppData,
                                        OMX_OUT OMX_BUFFERHEADERTYPE* aBuffer)
{
    PVMFOMXEncNode *Node = (PVMFOMXEncNode *) aAppData;
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
OsclReturnCode PVMFOMXEncNode::ProcessCallbackEventHandler_MultiThreaded(OsclAny* P)
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
OsclReturnCode PVMFOMXEncNode::ProcessCallbackEmptyBufferDone_MultiThreaded(OsclAny* P)
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
OsclReturnCode PVMFOMXEncNode::ProcessCallbackFillBufferDone_MultiThreaded(OsclAny* P)
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
PVMFOMXEncNode::~PVMFOMXEncNode()
{
    LogDiagnostics();

    //Clearup encoder
    DeleteOMXEncoder();

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

    //Cleanup allocated interfaces

    //Cleanup allocated ports
    ReleaseAllPorts();

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

    if (iNALSizeArray != NULL)
    {
        oscl_free(iNALSizeArray);
    }

    if (iNALPtrArray != NULL)
    {
        oscl_free(iNALPtrArray);
    }

    //Release Input buffer
    iDataIn.Unbind();

}

/////////////////////////////////////////////////////////////////////////////
// Add AO to the scheduler
/////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFOMXEncNode::ThreadLogon()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFOMXEncNode:ThreadLogon"));

    switch (iInterfaceState)
    {
        case EPVMFNodeCreated:
            if (!IsAdded())
            {
                AddToScheduler();
                iIsAdded = true;
            }

            SetState(EPVMFNodeIdle);
            return PVMFSuccess;
            // break;	This break statement was removed to avoid compiler warning for Unreachable Code
        default:
            return PVMFErrInvalidState;
            // break;	This break statement was removed to avoid compiler warning for Unreachable Code
    }
}

/////////////////////////////////////////////////////////////////////////////
// Remove AO from the scheduler
/////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFOMXEncNode::ThreadLogoff()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFOMXEncNode-%s::ThreadLogoff", iNodeTypeId));

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
PVMFStatus PVMFOMXEncNode::GetCapability(PVMFNodeCapability& aNodeCapability)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::GetCapability() called", iNodeTypeId));

    aNodeCapability = iCapability;
    return PVMFSuccess;
}

/////////////////////////////////////////////////////////////////////////////
PVMFPortIter* PVMFOMXEncNode::GetPorts(const PVMFPortFilter* aFilter)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::GetPorts() %s called", iNodeTypeId));

    OSCL_UNUSED_ARG(aFilter);

    return NULL;
}

/////////////////////////////////////////////////////////////////////////////
PVMFCommandId PVMFOMXEncNode::QueueCommandL(PVMFOMXEncNodeCommand& aCmd)
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
PVMFCommandId PVMFOMXEncNode::QueryUUID(PVMFSessionId s, const PvmfMimeString& aMimeType,
                                        Oscl_Vector<PVUuid, PVMFOMXEncNodeAllocator>& aUuids,
                                        bool aExactUuidsOnly,
                                        const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::QueryUUID() called", iNodeTypeId));
    PVMFOMXEncNodeCommand cmd;
    cmd.PVMFOMXEncNodeCommandBase::Construct(s, PVMFOMXEncNodeCommand::PVOMXENC_NODE_CMD_QUERYUUID, aMimeType, aUuids, aExactUuidsOnly, aContext);
    return QueueCommandL(cmd);
}

/////////////////////////////////////////////////////////////////////////////
PVMFCommandId PVMFOMXEncNode::QueryInterface(PVMFSessionId s, const PVUuid& aUuid,
        PVInterface*& aInterfacePtr,
        const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::QueryInterface() called", iNodeTypeId));
    PVMFOMXEncNodeCommand cmd;
    cmd.PVMFOMXEncNodeCommandBase::Construct(s, PVMFOMXEncNodeCommand::PVOMXENC_NODE_CMD_QUERYINTERFACE, aUuid, aInterfacePtr, aContext);
    return QueueCommandL(cmd);
}

/////////////////////////////////////////////////////////////////////////////
PVMFCommandId PVMFOMXEncNode::RequestPort(PVMFSessionId s, int32 aPortTag, const PvmfMimeString* aPortConfig, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::RequestPort() called", iNodeTypeId));
    PVMFOMXEncNodeCommand cmd;

    // IMPORTANT NOTE - ENGINE IS SENDING THE MIME TYPE FOR THE ENCODER INPUT/OUTPUT FORMAT THRU THE PORT PARAMETER
    cmd.PVMFOMXEncNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_REQUESTPORT, aPortTag, aPortConfig, aContext);
    return QueueCommandL(cmd);

}

/////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFOMXEncNode::ReleasePort(PVMFSessionId s, PVMFPortInterface& aPort, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::ReleasePort() called", iNodeTypeId));
    PVMFOMXEncNodeCommand cmd;
    cmd.PVMFOMXEncNodeCommandBase::Construct(s, PVMFOMXEncNodeCommand::PVOMXENC_NODE_CMD_RELEASEPORT, aPort, aContext);
    return QueueCommandL(cmd);
}

/////////////////////////////////////////////////////////////////////////////
PVMFCommandId PVMFOMXEncNode::Init(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::Init() called", iNodeTypeId));
    PVMFOMXEncNodeCommand cmd;
    cmd.PVMFOMXEncNodeCommandBase::Construct(s, PVMFOMXEncNodeCommand::PVOMXENC_NODE_CMD_INIT, aContext);
    return QueueCommandL(cmd);
}

/////////////////////////////////////////////////////////////////////////////
PVMFCommandId PVMFOMXEncNode::Prepare(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::Prepare() called", iNodeTypeId));
    PVMFOMXEncNodeCommand cmd;
    cmd.PVMFOMXEncNodeCommandBase::Construct(s, PVMFOMXEncNodeCommand::PVOMXENC_NODE_CMD_PREPARE, aContext);
    return QueueCommandL(cmd);
}

/////////////////////////////////////////////////////////////////////////////
PVMFCommandId PVMFOMXEncNode::Start(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::Start() called", iNodeTypeId));
    PVMFOMXEncNodeCommand cmd;
    cmd.PVMFOMXEncNodeCommandBase::Construct(s, PVMFOMXEncNodeCommand::PVOMXENC_NODE_CMD_START, aContext);
    return QueueCommandL(cmd);
}

/////////////////////////////////////////////////////////////////////////////
PVMFCommandId PVMFOMXEncNode::Stop(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::Stop() called", iNodeTypeId));
    PVMFOMXEncNodeCommand cmd;
    cmd.PVMFOMXEncNodeCommandBase::Construct(s, PVMFOMXEncNodeCommand::PVOMXENC_NODE_CMD_STOP, aContext);
    return QueueCommandL(cmd);
}

/////////////////////////////////////////////////////////////////////////////
PVMFCommandId PVMFOMXEncNode::Flush(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::Flush() called", iNodeTypeId));
    PVMFOMXEncNodeCommand cmd;
    cmd.PVMFOMXEncNodeCommandBase::Construct(s, PVMFOMXEncNodeCommand::PVOMXENC_NODE_CMD_FLUSH, aContext);
    return QueueCommandL(cmd);
}

/////////////////////////////////////////////////////////////////////////////
PVMFCommandId PVMFOMXEncNode::Pause(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::Pause() called", iNodeTypeId));
    PVMFOMXEncNodeCommand cmd;
    cmd.PVMFOMXEncNodeCommandBase::Construct(s, PVMFOMXEncNodeCommand::PVOMXENC_NODE_CMD_PAUSE, aContext);
    return QueueCommandL(cmd);
}

/////////////////////////////////////////////////////////////////////////////
PVMFCommandId PVMFOMXEncNode::Reset(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::Reset() called", iNodeTypeId));
    PVMFOMXEncNodeCommand cmd;
    cmd.PVMFOMXEncNodeCommandBase::Construct(s, PVMFOMXEncNodeCommand::PVOMXENC_NODE_CMD_RESET, aContext);
    return QueueCommandL(cmd);
}

/////////////////////////////////////////////////////////////////////////////
PVMFCommandId PVMFOMXEncNode::CancelAllCommands(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::CancelAllCommands() called", iNodeTypeId));
    PVMFOMXEncNodeCommand cmd;
    cmd.PVMFOMXEncNodeCommandBase::Construct(s, PVMFOMXEncNodeCommand::PVOMXENC_NODE_CMD_CANCELALL, aContext);
    return QueueCommandL(cmd);
}

/////////////////////////////////////////////////////////////////////////////
PVMFCommandId PVMFOMXEncNode::CancelCommand(PVMFSessionId s, PVMFCommandId aCmdId, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::CancelCommand() called", iNodeTypeId));
    PVMFOMXEncNodeCommand cmd;
    cmd.PVMFOMXEncNodeCommandBase::Construct(s, PVMFOMXEncNodeCommand::PVOMXENC_NODE_CMD_CANCELCMD, aCmdId, aContext);
    return QueueCommandL(cmd);
}





/////////////////////
// Private Section //
/////////////////////

/////////////////////////////////////////////////////////////////////////////
// Class Constructor
/////////////////////////////////////////////////////////////////////////////
PVMFOMXEncNode::PVMFOMXEncNode(int32 aPriority) :
        OsclActiveObject(aPriority, "PVMFOMXEncNode"),
        iInPort(NULL),
        iOutPort(NULL),
        iOutBufMemoryPool(NULL),
        iMediaDataMemPool(NULL),
        iOMXComponentOutputBufferSize(0),
        iOutputAllocSize(0),
        iProcessingState(EPVMFOMXEncNodeProcessingState_Idle),
        iOMXEncoder(NULL),
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
        iStopInResetMsgSent(false)
{
    iInterfaceState = EPVMFNodeCreated;


    // Allocate memory for VOL header
    uint refCounterSize = oscl_mem_aligned_size(sizeof(OsclRefCounterDA));
    uint VolHdrSize = refCounterSize + DEFAULT_VOL_HEADER_LENGTH;
    uint8 *memBufferVOLHeader = NULL;

    uint ParamSetSize = refCounterSize + DEFAULT_PARAMS_SET_LENGTH;
    uint8 *memBufferParamSet = NULL;


    int32 err;
    OSCL_TRY(err,

             //Create the input command queue.  Use a reserve to avoid lots of
             //dynamic memory allocation.
             iInputCommands.Construct(PVMF_OMXENC_NODE_COMMAND_ID_START, PVMF_OMXENC_NODE_COMMAND_VECTOR_RESERVE);

             //Create the "current command" queue.  It will only contain one
             //command at a time, so use a reserve of 1.
             iCurrentCommand.Construct(0, 1);

             //Set the node capability data.
             //This node can support an unlimited number of ports.
             iCapability.iCanSupportMultipleInputPorts = false;
             iCapability.iCanSupportMultipleOutputPorts = false;
             iCapability.iHasMaxNumberOfPorts = true;
             iCapability.iMaxNumberOfPorts = 2;

             // video output
             iCapability.iOutputFormatCapability.push_back(PVMF_MIME_H264_VIDEO_MP4);
             iCapability.iOutputFormatCapability.push_back(PVMF_MIME_H264_VIDEO_RAW);
             //iCapability.iOutputFormatCapability.push_back(PVMF_MIME_H264_VIDEO);
             iCapability.iOutputFormatCapability.push_back(PVMF_MIME_M4V);
             iCapability.iOutputFormatCapability.push_back(PVMF_MIME_H2631998);
             iCapability.iOutputFormatCapability.push_back(PVMF_MIME_H2632000);
             //iCapability.iOutputFormatCapability.push_back(PVMF_MIME_WMV);
             // audio output
             iCapability.iOutputFormatCapability.push_back(PVMF_MIME_AMR_IETF);
             iCapability.iOutputFormatCapability.push_back(PVMF_MIME_AMRWB_IETF);
             iCapability.iOutputFormatCapability.push_back(PVMF_MIME_AMR_IF2);
             iCapability.iOutputFormatCapability.push_back(PVMF_MIME_ADTS);
             iCapability.iOutputFormatCapability.push_back(PVMF_MIME_ADIF);
             iCapability.iOutputFormatCapability.push_back(PVMF_MIME_MPEG4_AUDIO);

             // video input
             iCapability.iInputFormatCapability.push_back(PVMF_MIME_YUV420);
             iCapability.iInputFormatCapability.push_back(PVMF_MIME_YUV422);
             iCapability.iInputFormatCapability.push_back(PVMF_MIME_RGB24);
             iCapability.iInputFormatCapability.push_back(PVMF_MIME_RGB12);

             // audio input
             iCapability.iInputFormatCapability.push_back(PVMF_MIME_PCM16);



             iAvailableMetadataKeys.reserve(PVMF_OMXENC_NUM_METADATA_VALUES);
             iAvailableMetadataKeys.clear();

             // for VOL header
             memBufferVOLHeader = (uint8*)iAlloc.allocate(VolHdrSize);
             if (!memBufferVOLHeader)
{
    OSCL_LEAVE(PVMFErrNoMemory);
    }

    memBufferParamSet = (uint8*) iAlloc.allocate(ParamSetSize);
    if (!memBufferParamSet)
{
    OSCL_LEAVE(PVMFErrNoMemory);
    }

            );

    // Save default VOL header
    oscl_memset(memBufferVOLHeader, 0, DEFAULT_VOL_HEADER_LENGTH);
    OsclMemoryFragment volHeader;
    OsclRefCounter* refCounterVOLHeader = new(memBufferVOLHeader) OsclRefCounterDA(memBufferVOLHeader,
            (OsclDestructDealloc*)&iAlloc);
    memBufferVOLHeader += refCounterSize;
    volHeader.ptr = memBufferVOLHeader;
    oscl_memcpy(volHeader.ptr, (OsclAny*)DEFAULT_VOL_HEADER, DEFAULT_VOL_HEADER_LENGTH);
    volHeader.len = DEFAULT_VOL_HEADER_LENGTH;
    iVolHeader = OsclRefCounterMemFrag(volHeader, refCounterVOLHeader, DEFAULT_VOL_HEADER_LENGTH);


    // construct SPS&PPS placeholder
    oscl_memset(memBufferParamSet, 0, DEFAULT_PARAMS_SET_LENGTH);
    OsclMemoryFragment paramSet;
    OsclRefCounter* refCounterParamSet = new(memBufferParamSet) OsclRefCounterDA(memBufferParamSet,
            (OsclDestructDealloc*)&iAlloc);
    memBufferParamSet += refCounterSize;
    paramSet.ptr = memBufferParamSet;
    paramSet.len = DEFAULT_PARAMS_SET_LENGTH;

    iParamSet = OsclRefCounterMemFrag(paramSet, refCounterParamSet, DEFAULT_PARAMS_SET_LENGTH);

    // initialize length and number of sps/ppss
    iParamSet.getMemFrag().len = 0;
    iNumPPSs = 0;
    iNumSPSs = 0;
    iSpsPpsSequenceOver = false;
    iFirstNAL = false; //set this to false so that mp4 can proceed without a problem.
    // in case of AVC, this flag will be set after spspps

    iNALSizeArray = NULL;
    iNALPtrArray = NULL;
    iNALSizeArrayMaxElems = 0;
    iNumNALs = 0;
    iFirstNALStartCodeSize = 0;

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

    // EOS flag init
    iIsEOSSentToComponent = false;
    iIsEOSReceivedFromComponent = false;

    // init state of component
    iCurrentEncoderState = OMX_StateInvalid;

    iTimeStampOut = 0;
    iTimeStampPrevious = 0;
    iBufferLenOut = 0;
    iBufferLenPrevious = 0;
    iEndOfFrameFlagPrevious = 0;
    iKeyFrameFlagPrevious = 0;
    iEndOfNALFlagPrevious = 0;


    iEndOfFrameFlagOut = 0;
    iKeyFrameFlagOut = 0;
    iEndOfNALFlagOut = 0;

    // counts output frames (for logging)
    iFrameCounter = 0;
    iInFormat = PVMF_MIME_FORMAT_UNKNOWN;
    iOutFormat = PVMF_MIME_FORMAT_UNKNOWN;

    // zero out encoder param structure

    oscl_memset(&iVideoInputFormat, 0, sizeof(iVideoInputFormat));

    // set default values
    iVideoInputFormat.iVideoFormat = EI_YUV420;
    iVideoInputFormat.iFrameWidth = DEFAULT_FRAME_WIDTH;
    iVideoInputFormat.iFrameHeight = DEFAULT_FRAME_HEIGHT;
    iVideoInputFormat.iFrameRate = (float)DEFAULT_FRAME_RATE;
    iVideoInputFormat.iFrameOrientation = 0;

    oscl_memset(&iVideoEncodeParam, 0, sizeof(iVideoEncodeParam));

    iVideoEncodeParam.iEncodeID = 0;
    iVideoEncodeParam.iNumLayer = 1;
    iVideoEncodeParam.iFrameWidth[0] = DEFAULT_FRAME_WIDTH;
    iVideoEncodeParam.iFrameHeight[0] = DEFAULT_FRAME_HEIGHT;
    iVideoEncodeParam.iBitRate[0] = DEFAULT_BITRATE;
    iVideoEncodeParam.iFrameRate[0] = (float)DEFAULT_FRAME_RATE;
    iVideoEncodeParam.iFrameQuality = 10;
    iVideoEncodeParam.iSceneDetection = false;
    iVideoEncodeParam.iRVLCEnable = false;
    iVideoEncodeParam.iIFrameInterval = DEFAULT_FRAME_RATE * 10;
    iVideoEncodeParam.iBufferDelay = (float)0.2;
    iVideoEncodeParam.iContentType = EI_H263;
    iVideoEncodeParam.iRateControlType = ECBR_1;
    iVideoEncodeParam.iIquant[0] = 15;
    iVideoEncodeParam.iPquant[0] = 12;
    iVideoEncodeParam.iBquant[0] = 12;
    iVideoEncodeParam.iSearchRange = 16;
    iVideoEncodeParam.iMV8x8 = false;
    iVideoEncodeParam.iMVHalfPel = true;
    iVideoEncodeParam.iPacketSize = 256;
    iVideoEncodeParam.iNoCurrentSkip = false;
    iVideoEncodeParam.iNoFrameSkip = false;
    iVideoEncodeParam.iClipDuration = 0;
    iVideoEncodeParam.iProfileLevel = EI_CORE_LEVEL2;
    /////////////////AVC SPECIFIC///////////////////////////
    iVideoEncodeParam.iEncMode = EI_ENCMODE_RECORDER;
    iVideoEncodeParam.iAVCProfile = EI_PROFILE_BASELINE;
    iVideoEncodeParam.iAVCLevel = EI_LEVEL_11;


    oscl_memset(&iAudioInputFormat, 0, sizeof(iAudioInputFormat));
    // Currently, set according to AMR values
    iAudioInputFormat.iInputInterleaveMode = EINTERLEAVE_LR;
    iAudioInputFormat.iInputBitsPerSample = 16;
    iAudioInputFormat.iInputNumChannels = 1;
    iAudioInputFormat.iInputSamplingRate = 8000;

    oscl_memset(&iAudioEncodeParam, 0, sizeof(iAudioEncodeParam));

    iAudioEncodeParam.iMaxNumOutputFramesPerBuffer = MAX_NUM_AMR_FRAMES_PER_BUFFER;
    iAudioEncodeParam.iAMRBitrate = GSM_AMR_12_2;
    iAudioEncodeParam.iOutputBitrate = 24000;
    iAudioEncodeParam.iOutputNumChannels = iAudioInputFormat.iInputNumChannels;
    iAudioEncodeParam.iOutputSamplingRate = iAudioInputFormat.iInputSamplingRate;


#ifdef _TEST_AE_ERROR_HANDLING
    iErrorHandlingInit = false;
    iErrorHandlingEncodeCount = 0;
    iCountFrames = 0;
    iErrorDataPathStall = 0;
    iErrorNodeCmd = 0;
    iErrorConfigHeader = false;
    iErrorEncodeFlag = 0;
#endif

    sendYuvFsi = true;

    iNodeTypeId = LOG_ID_UNKNOWN;
    iLogger = PVLogger::GetLoggerObject("PVMFOMXEncNode");
    iRunlLogger = PVLogger::GetLoggerObject("Run.PVMFOMXEncNode");
    iDataPathLogger = PVLogger::GetLoggerObject("datapath");
    iClockLogger = PVLogger::GetLoggerObject("clock");
    iDiagnosticsLogger = PVLogger::GetLoggerObject("pvplayerdiagnostics.encnode.OMXEncnode");
}

/////////////////////////////////////////////////////////////////////////////
// Local Run Routine
/////////////////////////////////////////////////////////////////////////////
void PVMFOMXEncNode::Run()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::Run() In", iNodeTypeId));

    // if reset is in progress, call DoReset again until Reset Msg is sent
    if ((iResetInProgress == true) &&
            (iResetMsgSent == false) &&
            (iCurrentCommand.size() > 0) &&
            (iCurrentCommand.front().iCmd == PVMFOMXEncNodeCommand::PVOMXENC_NODE_CMD_RESET)
       )
    {
        DoReset(iCurrentCommand.front());
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "PVMFOMXEncNode-%s::Run() - Calling DoReset", iNodeTypeId));
        return; // don't do anything else
    }
    //Check for NODE commands...
    if (!iInputCommands.empty())
    {
        if (ProcessCommand(iInputCommands.front()))
        {
            if (iInterfaceState != EPVMFNodeCreated
                    && (!iInputCommands.empty() || (iInPort && (iInPort->IncomingMsgQueueSize() > 0)) ||
                        (iDataIn.GetRep() != NULL)))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "PVMFOMXEncNode-%s::Run() - rescheduling after process command", iNodeTypeId));
                RunIfNotReady();
            }
            return;
        }

        if (!iInputCommands.empty())
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "PVMFOMXEncNode-%s::Run() - rescheduling to process more commands", iNodeTypeId));
            RunIfNotReady();
        }
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "PVMFOMXEncNode-%s::Run() - Input commands empty", iNodeTypeId));
    }

    if (((iCurrentCommand.size() == 0) && (iInterfaceState != EPVMFNodeStarted)) ||
            ((iCurrentCommand.size() > 0) && (iCurrentCommand.front().iCmd == PVMFOMXEncNodeCommand::PVOMXENC_NODE_CMD_START) && (iInterfaceState != EPVMFNodeStarted)))
    {
        // rescheduling because of input data will be handled in Command Processing Part
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "PVMFOMXEncNode-%s::Run() - Node not in Started state yet", iNodeTypeId));
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
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "PVMFOMXEncNode-%s::Run() - Outgoing Port Busy, cannot send more msgs", iNodeTypeId));
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
        if (iInPort && (iInPort->IncomingMsgQueueSize() > 0) && (iDataIn.GetRep() == NULL) && !iEndOfDataReached)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "PVMFOMXEncNode-%s::Run() - Getting more input", iNodeTypeId));
            if (!ProcessIncomingMsg(iInPort))
            {
                //Re-schedule to come back.
                RunIfNotReady();
                return;
            }
        }

        if (iSendBOS)
        {
            SendBeginOfMediaStreamCommand();
        }

        // If in init or ready to encode state, process data in the input port if there is input available and input buffers are present
        // (note: at EOS, iDataIn will not be available)
        if ((iDataIn.GetRep() != NULL) ||
                ((iNumOutstandingOutputBuffers < iNumOutputBuffers) &&
                 (iProcessingState == EPVMFOMXEncNodeProcessingState_ReadyToEncode) &&
                 (iResetMsgSent == false)) ||
                ((iDynamicReconfigInProgress == true) && (iResetMsgSent == false))
           )
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "PVMFOMXEncNode-%s::Run() - Calling HandleProcessingState", iNodeTypeId));

            // input data is available, that means there is data to be encoded
            if (HandleProcessingState() != PVMFSuccess)
            {
                // If HandleProcessingState does not return Success, we must wait for an event
                // no point in  rescheduling
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "PVMFOMXEncNode-%s::Run() - HandleProcessingState did not return Success", iNodeTypeId));
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
    timeinloop  = (endtime - starttime);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iRunlLogger, PVLOGMSG_INFO,
                    (0, "PVMFOMXEncNode-%s::Run() - LoopCount = %d, Time spent in loop(in ms) = %d, iNumOutstandingInputBuffers = %d, iNumOutstandingOutputBuffers = %d ", iNodeTypeId,
                     loopCount, timeinloop, iNumOutstandingInputBuffers, iNumOutstandingOutputBuffers));
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
                            (0, "PVMFOMXEncNode-%s::Run() - Sending EOS marked buffer To Component ", iNodeTypeId));

            iIsEOSSentToComponent = true;


            // if the component is not yet initialized or if it's in the middle of port reconfig,
            // don't send EOS buffer to component. It does not care. Just set the flag as if we received
            // EOS from the component to enable sending EOS downstream
            if (iProcessingState != EPVMFOMXEncNodeProcessingState_ReadyToEncode)
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
                            (0, "PVMFOMXEncNode-%s::Run() - Received EOS from component, Sending EOS msg downstream ", iNodeTypeId));

            if (iOutPort && iOutPort->IsOutgoingQueueBusy())
            {
                // note: we already tried to empty the outgoing q. If it's still busy,
                // it means that output port is busy. Just return and wait for the port to become free.
                // this will wake up the node and it will send out a msg from the q etc.
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "PVMFOMXEncNode-%s::Run() - - EOS cannot be sent downstream, outgoing queue busy - wait", iNodeTypeId));
                return;
            }

            if (SendEndOfTrackCommand()) // this will only q the EOS
            {
                // EOS send downstream OK, so reset the flag
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "PVMFOMXEncNode-%s::Run() - EOS was queued to be sent downstream", iNodeTypeId));

                iEndOfDataReached = false; // to resume normal processing, reset the flags
                iIsEOSSentToComponent = false;
                iIsEOSReceivedFromComponent = false;

                RunIfNotReady(); // Run again to send out the EOS msg from the outgoing q, and resume
                // normal processing

                // END OF STREAM EVENT reported by the downstream node.
                //ReportInfoEvent(PVMFInfoEndOfData);
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
            (iCurrentCommand.front().iCmd == PVMFOMXEncNodeCommand::PVOMXENC_NODE_CMD_FLUSH) &&
            (iInPort->IncomingMsgQueueSize() == 0) &&
            (iOutPort->OutgoingMsgQueueSize() == 0) &&
            (iDataIn.GetRep() == NULL) &&
            (iNumOutstandingOutputBuffers == iNumOutputBuffers) && // all output buffers are with the component or outstanding
            (iNumOutstandingInputBuffers == 0) // all input buffers were processed and returned. These 2 conditions mean the component is idle
       )
    {
        //flush command is almost completed
        //Debug check-- all the port queues should be empty at this point.
        OMX_ERRORTYPE err = OMX_ErrorNone;
        OMX_STATETYPE sState = OMX_StateInvalid;

        OSCL_ASSERT(iInPort->IncomingMsgQueueSize() == 0 && iOutPort->OutgoingMsgQueueSize() == 0);

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "PVMFOMXEncNode-%s::Run() - Flush pending", iNodeTypeId));

        // now, drive the state of the omx component to IDLE - wait for the callback from the component and send cmd complete from there
        iDataIn.Unbind();
        iPreviousMediaData.Unbind(); // make sure nothing is holding this data
        if ((iOutFormat == PVMF_MIME_H264_VIDEO_MP4) || (iOutFormat == PVMF_MIME_H264_VIDEO_RAW))
        {
            // prepare for next start
            iFirstNAL = true;
        }

        // Clear the data flags

        iEndOfDataReached = false;
        iIsEOSSentToComponent = false;
        iIsEOSReceivedFromComponent = false;


        iDoNotSendOutputBuffersDownstreamFlag = true; // stop sending output buffers downstream
        iDoNotSaveInputBuffersFlag = true;


        //Get state of OpenMAX encoder
        err = OMX_GetState(iOMXEncoder, &sState);
        if (err != OMX_ErrorNone)
        {
            //Error condition report
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFOMXEncNode-%s::Run(): Flush pending - Can't get State of encoder!", iNodeTypeId));

            sState = OMX_StateInvalid;
        }

        if ((sState == OMX_StateExecuting) || (sState == OMX_StatePause))
        {
            /* Change state to OMX_StateIdle from OMX_StateExecuting or OMX_StatePause. */

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXEncNode-%s::Run(): Flush pending:  Changing Component State Executing->Idle or Pause->Idle", iNodeTypeId));

            err = OMX_SendCommand(iOMXEncoder, OMX_CommandStateSet, OMX_StateIdle, NULL);
            if (err != OMX_ErrorNone)
            {
                //Error condition report
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMFOMXEncNode-%s::Run(): Flush pending : Can't send StateSet command to encoder!", iNodeTypeId));

                CommandComplete(iCurrentCommand, iCurrentCommand.front(), PVMFErrInvalidState);
                return;
            }

            // prevent the node from sending more buffers etc.
            // if port reconfiguration is in process, let the state remain one of the port config states
            //	if there is a start command, we can do it seemlessly (by continuing the port reconfig)
            if (iProcessingState == EPVMFOMXEncNodeProcessingState_ReadyToEncode)
                iProcessingState = EPVMFOMXEncNodeProcessingState_Stopping;

        }
        else
        {
            //Error condition report
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFOMXEncNode-%s::Run(): Flush pending : Encoder is not in the Executing or Pause state!", iNodeTypeId));

            CommandComplete(iCurrentCommand, iCurrentCommand.front(), PVMFErrInvalidState);
            return;
        }


    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::Run() Out", iNodeTypeId));
}

/////////////////////////////////////////////////////////////////////////////
// This routine will dispatch recived commands
/////////////////////////////////////////////////////////////////////////////
bool PVMFOMXEncNode::ProcessCommand(PVMFOMXEncNodeCommand& aCmd)
{
    //normally this node will not start processing one command
    //until the prior one is finished.  However, a hi priority
    //command such as Cancel must be able to interrupt a command
    //in progress.
    if (!iCurrentCommand.empty() && !aCmd.hipri())
        return false;

    switch (aCmd.iCmd)
    {
        case PVMFOMXEncNodeCommand::PVOMXENC_NODE_CMD_QUERYUUID:
            DoQueryUuid(aCmd);
            break;

        case PVMFOMXEncNodeCommand::PVOMXENC_NODE_CMD_QUERYINTERFACE:
            DoQueryInterface(aCmd);
            break;

        case PVMFOMXEncNodeCommand::PVOMXENC_NODE_CMD_REQUESTPORT:
            DoRequestPort(aCmd);
            break;

        case PVMFOMXEncNodeCommand::PVOMXENC_NODE_CMD_RELEASEPORT:
            DoReleasePort(aCmd);
            break;

        case PVMFOMXEncNodeCommand::PVOMXENC_NODE_CMD_INIT:
            DoInit(aCmd);
            break;

        case PVMFOMXEncNodeCommand::PVOMXENC_NODE_CMD_PREPARE:
            DoPrepare(aCmd);
            break;

        case PVMFOMXEncNodeCommand::PVOMXENC_NODE_CMD_START:
            DoStart(aCmd);
            break;

        case PVMFOMXEncNodeCommand::PVOMXENC_NODE_CMD_STOP:
            DoStop(aCmd);
            break;

        case PVMFOMXEncNodeCommand::PVOMXENC_NODE_CMD_FLUSH:
            DoFlush(aCmd);
            break;

        case PVMFOMXEncNodeCommand::PVOMXENC_NODE_CMD_PAUSE:
            DoPause(aCmd);
            break;

        case PVMFOMXEncNodeCommand::PVOMXENC_NODE_CMD_RESET:
            DoReset(aCmd);
            break;

        case PVMFOMXEncNodeCommand::PVOMXENC_NODE_CMD_CANCELCMD:
            DoCancelCommand(aCmd);
            break;

        case PVMFOMXEncNodeCommand::PVOMXENC_NODE_CMD_CANCELALL:
            DoCancelAllCommands(aCmd);
            break;

        case PVMFOMXEncNodeCommand::PVOMXENC_NODE_CMD_GETNODEMETADATAKEY:
        {
            PVMFStatus retval = DoGetNodeMetadataKey(aCmd);
            CommandComplete(iInputCommands, aCmd, retval);
        }
        break;

        case PVMFOMXEncNodeCommand::PVOMXENC_NODE_CMD_GETNODEMETADATAVALUE:
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
bool PVMFOMXEncNode::ProcessIncomingMsg(PVMFPortInterface* aPort)
{
    //Called by the AO to process one buffer off the port's
    //incoming data queue.  This routine will dequeue and
    //dispatch the data.

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "0x%x PVMFOMXEncNode-%s::ProcessIncomingMsg: aPort=0x%x", this, iNodeTypeId, aPort));

    PVMFStatus status = PVMFFailure;


//#define SIMULATE_BOS
#ifdef SIMULATE_BOS

    if (((PVMFOMXEncPort*)aPort)->iNumFramesConsumed == 6))
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

#ifdef _DEBUG
        printf("PVMFOMXEncNode-%s::ProcessIncomingMsg() SIMULATED BOS\n", iNodeTypeId);
#endif
        ((PVMFOMXEncPort*)aPort)->iNumFramesConsumed++;
        return true;

    }
#endif
//#define SIMULATE_PREMATURE_EOS
#ifdef SIMULATE_PREMATURE_EOS
    if (((PVMFOMXEncPort*)aPort)->iNumFramesConsumed == 5)
    {
        PVMFSharedMediaCmdPtr EOSCmdPtr = PVMFMediaCmd::createMediaCmd();

        // Set the format ID to EOS
        EOSCmdPtr->setFormatID(PVMF_MEDIA_CMD_EOS_FORMAT_ID);

        // Set the timestamp
        EOSCmdPtr->setTimestamp(200);

        // Convert to media message and send it out
        PVMFSharedMediaMsgPtr mediaMsgOut;
        convertToPVMFMediaCmdMsg(mediaMsgOut, EOSCmdPtr);

        ((PVMFOMXEncPort*)aPort)->iNumFramesConsumed++;

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::ProcessIncomingMsg: SIMULATED EOS", iNodeTypeId));
#ifdef _DEBUG
        printf("PVMFOMXEncNode-%s::ProcessIncomingMsg() SIMULATED EOS\n", iNodeTypeId);
#endif
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
                    (0, "0x%x PVMFOMXEncNode-%s::ProcessIncomingMsg: Error - DequeueIncomingMsg failed", this, iNodeTypeId));
        return false;
    }

    if (msg->getFormatID() == PVMF_MEDIA_CMD_BOS_FORMAT_ID)
    {
        //store the stream id and time stamp of bos message
        iStreamID = msg->getStreamID();
        iBOSTimestamp = msg->getTimestamp();
        iSendBOS = true;

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::ProcessIncomingMsg: Received BOS stream %d, timestamp %d", iNodeTypeId, iStreamID, iBOSTimestamp));
        ((PVMFOMXEncPort*)aPort)->iNumFramesConsumed++;
        return true;
    }
    else if (msg->getFormatID() == PVMF_MEDIA_CMD_EOS_FORMAT_ID)
    {
        // Set EOS flag
        iEndOfDataReached = true;
        // Save the timestamp for the EOS cmd
        iEndOfDataTimestamp = msg->getTimestamp();

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::ProcessIncomingMsg: Received EOS", iNodeTypeId));

        ((PVMFOMXEncPort*)aPort)->iNumFramesConsumed++;
        return true; // do not do conversion into media data, just set the flag and leave
    }

    convertToPVMFMediaData(iDataIn, msg);


    iCurrFragNum = 0; // for new message, reset the fragment counter


    ((PVMFOMXEncPort*)aPort)->iNumFramesConsumed++;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::ProcessIncomingMsg() Received %d frames", iNodeTypeId, ((PVMFOMXEncPort*)aPort)->iNumFramesConsumed));

    //return true if we processed an activity...
    return true;
}

/////////////////////////////////////////////////////////////////////////////
// This routine will process outgoing message by sending it into output the port
/////////////////////////////////////////////////////////////////////////////
bool PVMFOMXEncNode::ProcessOutgoingMsg(PVMFPortInterface* aPort)
{
    //Called by the AO to process one message off the outgoing
    //message queue for the given port.  This routine will
    //try to send the data to the connected port.

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "0x%x PVMFOMXEncNode-%s::ProcessOutgoingMsg: aPort=0x%x", this, iNodeTypeId, aPort));

    PVMFStatus status = aPort->Send();
    if (status == PVMFErrBusy)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "0x%x PVMFOMXEncNode-%s::ProcessOutgoingMsg: Connected port goes into busy state", this, iNodeTypeId));
    }

    //Report any unexpected failure in port processing...
    //(the InvalidState error happens when port input is suspended,
    //so don't report it.)
    if (status != PVMFErrBusy
            && status != PVMFSuccess
            && status != PVMFErrInvalidState)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "0x%x PVMFOMXEncNode-%s::ProcessOutgoingMsg: Error - ProcessPortActivity failed. port=0x%x, type=%d",
                         this, iNodeTypeId, iOutPort, PVMF_PORT_ACTIVITY_OUTGOING_MSG));
        ReportErrorEvent(PVMFErrPortProcessing);
    }

    //return true if we processed an activity...
    return (status != PVMFErrBusy);
}

/////////////////////////////////////////////////////////////////////////////
// This routine will process received data usign State Machine
/////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFOMXEncNode::HandleProcessingState()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::HandleProcessingState() In", iNodeTypeId));

    PVMFStatus status = PVMFSuccess;

    switch (iProcessingState)
    {

            // The FOLLOWING 4 states handle Dynamic Port Reconfiguration
        case EPVMFOMXEncNodeProcessingState_PortReconfig:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXEncNode-%s::HandleProcessingState() Port Reconfiguration -> Sending Port Disable Command", iNodeTypeId));

            // port reconfiguration is required. Only one port at a time is disabled and then re-enabled after buffer resizing
            OMX_SendCommand(iOMXEncoder, OMX_CommandPortDisable, iPortIndexForDynamicReconfig, NULL);
            // the port will now start returning outstanding buffers
            // set the flag to prevent output from going downstream (in case of output port being reconfigd)
            // set the flag to prevent input from being saved and returned to component (in case of input port being reconfigd)
            // set the state to wait for port saying it is disabled
            if (iPortIndexForDynamicReconfig == iOutputPortIndex)
            {
                iDoNotSendOutputBuffersDownstreamFlag = true;
            }
            else if (iPortIndexForDynamicReconfig == iInputPortIndex)
            {
                iDoNotSaveInputBuffersFlag = true;

            }
            iProcessingState = EPVMFOMXEncNodeProcessingState_WaitForBufferReturn;


            // fall through to the next case to check if all buffers are already back
        }

        case EPVMFOMXEncNodeProcessingState_WaitForBufferReturn:
        {
            // as buffers are coming back, Run may be called, wait until all buffers are back, then Free them all

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXEncNode-%s::HandleProcessingState() Port Reconfiguration -> WaitForBufferReturn ", iNodeTypeId));
            // check if it's output port being reconfigured
            if (iPortIndexForDynamicReconfig == iOutputPortIndex)
            {
                // if all buffers have returned, free them
                if (iNumOutstandingOutputBuffers == 0)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                    (0, "PVMFOMXEncNode-%s::HandleProcessingState() Port Reconfiguration -> all output buffers are back, free them", iNodeTypeId));
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
                                            (0, "PVMFOMXEncNode-%s::HandleProcessingState() Port Reconfiguration -> Cannot free output buffers ", iNodeTypeId));

                            SetState(EPVMFNodeError);
                            ReportErrorEvent(PVMFErrNoMemory);
                            return PVMFErrNoMemory;
                        }
                    }
                    // if the callback (that port is disabled) has not arrived yet, wait for it
                    // if it has arrived, it will set the state to PortReEnable
                    if (iProcessingState != EPVMFOMXEncNodeProcessingState_PortReEnable)
                        iProcessingState = EPVMFOMXEncNodeProcessingState_WaitForPortDisable;

                    status = PVMFSuccess; // allow rescheduling of the node potentially
                }
                else
                    status = PVMFErrNoMemory; // must wait for buffers to come back. No point in automatic rescheduling
                // but each buffer will reschedule the node when it comes in
            }
            else
            {
                // this is input port

                // if all buffers have returned, free them
                if (iNumOutstandingInputBuffers == 0)
                {

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                    (0, "PVMFOMXEncNode-%s::HandleProcessingState() Port Reconfiguration -> all input buffers are back, free them", iNodeTypeId));
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
                                            (0, "PVMFOMXEncNode-%s::HandleProcessingState() Port Reconfiguration -> Cannot free input buffers ", iNodeTypeId));

                            SetState(EPVMFNodeError);
                            ReportErrorEvent(PVMFErrNoMemory);
                            return PVMFErrNoMemory;

                        }
                    }
                    // if the callback (that port is disabled) has not arrived yet, wait for it
                    // if it has arrived, it will set the state to PortReEnable
                    if (iProcessingState != EPVMFOMXEncNodeProcessingState_PortReEnable)
                        iProcessingState = EPVMFOMXEncNodeProcessingState_WaitForPortDisable;

                    status = PVMFSuccess; // allow rescheduling of the node
                }
                else
                    status = PVMFErrNoMemory; // must wait for buffers to come back. No point in automatic
                // rescheduling. Each buffer will reschedule the node
                // when it comes in
            }


            // the state will be changed to PortReEnable once we get confirmation that Port was actually disabled
            break;
        }

        case EPVMFOMXEncNodeProcessingState_WaitForPortDisable:
        {

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXEncNode-%s::HandleProcessingState() Port Reconfiguration -> wait for port disable callback", iNodeTypeId));
            // do nothing. Just wait for the port to become disabled (we'll get event from component, which will
            // transition the state to PortReEnable
            status = PVMFErrNoMemory; // prevent Rescheduling the node
            break;
        }

        case EPVMFOMXEncNodeProcessingState_PortReEnable:
        {

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXEncNode-%s::HandleProcessingState() Port Reconfiguration -> Sending reenable port command", iNodeTypeId));
            // set the port index so that we get parameters for the proper port
            iParamPort.nPortIndex = iPortIndexForDynamicReconfig;
            // iParamPort.nVersion = OMX_VERSION;

            // get new parameters of the port
            OMX_GetParameter(iOMXEncoder, OMX_IndexParamPortDefinition, &iParamPort);

            // send command for port re-enabling (for this to happen, we must first recreate the buffers)
            OMX_SendCommand(iOMXEncoder, OMX_CommandPortEnable, iPortIndexForDynamicReconfig, NULL);

            // is this output port?
            if (iPortIndexForDynamicReconfig == iOutputPortIndex)
            {
                iOMXComponentOutputBufferSize = ((iParamPort.format.video.nFrameWidth + 15) & (~15)) * ((iParamPort.format.video.nFrameHeight + 15) & (~15)) * 3 / 2;

                // check the new buffer size
                if (iInPort)
                {
                    if (((PVMFOMXEncPort*)iInPort)->iFormat == PVMF_MIME_H264_VIDEO ||
                            ((PVMFOMXEncPort*)iInPort)->iFormat == PVMF_MIME_H264_VIDEO_MP4 ||
                            ((PVMFOMXEncPort*)iInPort)->iFormat == PVMF_MIME_H264_VIDEO_RAW ||
                            ((PVMFOMXEncPort*)iInPort)->iFormat == PVMF_MIME_M4V ||
                            ((PVMFOMXEncPort*)iInPort)->iFormat == PVMF_MIME_H2631998 ||
                            ((PVMFOMXEncPort*)iInPort)->iFormat == PVMF_MIME_H2632000)
                    {
                        iOMXComponentOutputBufferSize = ((iParamPort.format.video.nFrameWidth + 15) & (~15)) * ((iParamPort.format.video.nFrameHeight + 15) & (~15)) * 3 / 2;
                    }
                    else if (((PVMFOMXEncPort*)iInPort)->iFormat == PVMF_MIME_WMV)
                    {
                        // This is a requirement for the WMV encoder that we have currently
                        iOMXComponentOutputBufferSize = ((iParamPort.format.video.nFrameWidth + 3) & (~3)) * (iParamPort.format.video.nFrameHeight) * 3 / 2;
                    }
                    else
                    {
                        OSCL_ASSERT(false);
                    }
                }
                // set the new width / height
                iYUVWidth =  iParamPort.format.video.nFrameWidth;
                iYUVHeight = iParamPort.format.video.nFrameHeight;

                if (iOMXComponentOutputBufferSize < iParamPort.nBufferSize)
                    iOMXComponentOutputBufferSize = iParamPort.nBufferSize;

                // do we need to increase the number of buffers?
                if (iNumOutputBuffers < iParamPort.nBufferCountMin)
                    iNumOutputBuffers = iParamPort.nBufferCountMin;

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "PVMFOMXEncNode-%s::HandleProcessingState() new output buffers %d, size %d", iNodeTypeId, iNumOutputBuffers, iOMXComponentOutputBufferSize));

                /* Allocate output buffers */
                if (!CreateOutMemPool(iNumOutputBuffers))
                {

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "PVMFOMXEncNode-%s::HandleProcessingState() Port Reconfiguration -> Cannot allocate output buffers ", iNodeTypeId));

                    SetState(EPVMFNodeError);
                    ReportErrorEvent(PVMFErrNoMemory);
                    return PVMFErrNoMemory;
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
                                    (0, "PVMFOMXEncNode-%s::HandleProcessingState() Port Reconfiguration -> Cannot provide output buffers to component", iNodeTypeId));

                    SetState(EPVMFNodeError);
                    ReportErrorEvent(PVMFErrNoMemory);
                    return PVMFErrNoMemory;

                }

                // do not drop output any more, i.e. enable output to be sent downstream
                iDoNotSendOutputBuffersDownstreamFlag = false;


            }
            else
            {
                // this is input port

                iOMXComponentInputBufferSize = iParamPort.nBufferSize;
                // do we need to increase the number of buffers?
                if (iNumInputBuffers < iParamPort.nBufferCountMin)
                    iNumInputBuffers = iParamPort.nBufferCountMin;

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "PVMFOMXEncNode-%s::HandleProcessingState() new buffers %d, size %d", iNodeTypeId, iNumInputBuffers, iOMXComponentInputBufferSize));

                /* Allocate input buffers */
                if (!CreateInputMemPool(iNumInputBuffers))
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "PVMFOMXEncNode-%s::HandleProcessingState() Port Reconfiguration -> Cannot allocate new input buffers to component", iNodeTypeId));

                    SetState(EPVMFNodeError);
                    ReportErrorEvent(PVMFErrNoMemory);
                    return PVMFErrNoMemory;
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
                                    (0, "PVMFOMXEncNode-%s::HandleProcessingState() Port Reconfiguration -> Cannot provide new input buffers to component", iNodeTypeId));

                    SetState(EPVMFNodeError);
                    ReportErrorEvent(PVMFErrNoMemory);
                    return PVMFErrNoMemory;

                }
                // do not drop partially consumed input
                iDoNotSaveInputBuffersFlag = false;


            }

            // if the callback that the port was re-enabled has not arrived yet, wait for it
            // if it has arrived, it will set the state to either PortReconfig or to ReadyToEncode
            if (iProcessingState != EPVMFOMXEncNodeProcessingState_PortReconfig &&
                    iProcessingState != EPVMFOMXEncNodeProcessingState_ReadyToEncode)
                iProcessingState = EPVMFOMXEncNodeProcessingState_WaitForPortEnable;

            status = PVMFSuccess; // allow rescheduling of the node
            break;
        }

        case EPVMFOMXEncNodeProcessingState_WaitForPortEnable:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXEncNode-%s::HandleProcessingState() Port Reconfiguration -> wait for port enable callback", iNodeTypeId));
            // do nothing. Just wait for the port to become enabled (we'll get event from component, which will
            // transition the state to ReadyToEncode
            status = PVMFErrNoMemory; // prevent ReScheduling
            break;
        }

        // NORMAL DATA FLOW STATE:
        case EPVMFOMXEncNodeProcessingState_ReadyToEncode:
        {

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXEncNode-%s::HandleProcessingState() Ready To Encode start", iNodeTypeId));
            // In normal data flow and decoding state
            // Send all available output buffers to the encoder

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
                                (0, "PVMFOMXEncNode-%s::HandleProcessingState() Sending previous - partially consumed input back to the OMX component", iNodeTypeId));

                OMX_EmptyThisBuffer(iOMXEncoder, iInputBufferToResendToComponent);
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
        case EPVMFOMXEncNodeProcessingState_Stopping:
        {

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXEncNode-%s::HandleProcessingState() Stopping -> wait for Component to move from Executing->Idle", iNodeTypeId));


            status = PVMFErrNoMemory; // prevent rescheduling
            break;
        }

        case EPVMFOMXEncNodeProcessingState_WaitForOutgoingQueue:
            status = PVMFErrNoMemory;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::HandleProcessingState() Do nothing since waiting for output port queue to become available", iNodeTypeId));
            break;

        default:
            break;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::HandleProcessingState() Out", iNodeTypeId));

    return status;

}
/////////////////////////////////////////////////////////////////////////////
bool PVMFOMXEncNode::SendOutputBufferToOMXComponent()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXEncNode-%s::SendOutputBufferToOMXComponent() In", iNodeTypeId));


    OutputBufCtrlStruct *output_buf = NULL;
    int32  errcode = 0;
    uint32 ii;

    // try to get output buffer header
    OSCL_TRY(errcode, output_buf = (OutputBufCtrlStruct *) iOutBufMemoryPool->allocate(iOutputAllocSize));
    if (errcode != 0)
    {
        if (errcode == OsclErrNoResources)
        {

            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger,
                            PVLOGMSG_DEBUG, (0, "PVMFOMXEncNode-%s::SendOutputBufferToOMXComponent() No more output buffers in the mempool", iNodeTypeId));

            iOutBufMemoryPool->notifyfreechunkavailable(*this, (OsclAny *) iOutBufMemoryPool); // To signal when next deallocate() is called on mempool

            return false;
        }
        else
        {
            // Memory allocation for the pool failed
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFOMXEncNode-%s::SendOutputBufferToOMXComponent() Output mempool error", iNodeTypeId));


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
    OMX_FillThisBuffer(iOMXEncoder, output_buf->pBufHdr);



    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXEncNode-%s::SendOutputBufferToOMXComponent() Out", iNodeTypeId));

    return true;
}
////////////////////////////////////////////////////////////////////////////////
bool PVMFOMXEncNode::NegotiateVideoComponentParameters()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXEncNode-%s::NegotiateVideoParameters() In", iNodeTypeId));

    OMX_ERRORTYPE Err;
    OMX_CONFIG_ROTATIONTYPE InputRotationType;

    // first get the number of ports and port indices
    OMX_PORT_PARAM_TYPE VideoPortParameters;
    uint32 NumPorts;
    uint32 ii;

    // get starting number
    CONFIG_SIZE_AND_VERSION(VideoPortParameters);

    Err = OMX_GetParameter(iOMXEncoder, OMX_IndexParamVideoInit, &VideoPortParameters);
    NumPorts = VideoPortParameters.nPorts; // must be at least 2 of them (in&out)

    if (Err != OMX_ErrorNone || NumPorts < 2)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::NegotiateVideoComponentParameters() There is insuffucient (%d) ports", iNodeTypeId, NumPorts));
        return false;
    }


    // loop through video ports starting from the starting index to find index of the first input port
    for (ii = VideoPortParameters.nStartPortNumber ;ii < VideoPortParameters.nStartPortNumber + NumPorts; ii++)
    {
        // get port parameters, and determine if it is input or output
        // if there are more than 2 ports, the first one we encounter that has input direction is picked


        CONFIG_SIZE_AND_VERSION(iParamPort);
        //port
        iParamPort.nPortIndex = ii; // iInputPortIndex; //OMF_MC_H264D_PORT_INDEX_OF_STREAM;
        Err = OMX_GetParameter(iOMXEncoder, OMX_IndexParamPortDefinition, &iParamPort);

        if (Err != OMX_ErrorNone)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXEncNode-%s::NegotiateVideoComponentParameters() Problem negotiating with port %d ", iNodeTypeId, ii));

            return false;
        }

        if (iParamPort.eDir == OMX_DirInput)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXEncNode-%s::NegotiateVideoComponentParameters() Found Input port index %d ", iNodeTypeId, ii));

            iInputPortIndex = ii;
            break;
        }
    }
    if (ii == VideoPortParameters.nStartPortNumber + NumPorts)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::NegotiateVideoComponentParameters() Cannot find any input port ", iNodeTypeId));
        return false;
    }


    // loop through video ports starting from the starting index to find index of the first output port
    for (ii = VideoPortParameters.nStartPortNumber ;ii < VideoPortParameters.nStartPortNumber + NumPorts; ii++)
    {
        // get port parameters, and determine if it is input or output
        // if there are more than 2 ports, the first one we encounter that has output direction is picked


        CONFIG_SIZE_AND_VERSION(iParamPort);
        //port
        iParamPort.nPortIndex = ii;
        Err = OMX_GetParameter(iOMXEncoder, OMX_IndexParamPortDefinition, &iParamPort);

        if (Err != OMX_ErrorNone)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXEncNode-%s::NegotiateVideoComponentParameters() Problem negotiating with port %d ", iNodeTypeId, ii));

            return false;
        }

        if (iParamPort.eDir == OMX_DirOutput)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXEncNode-%s::NegotiateVideoComponentParameters() Found Output port index %d ", iNodeTypeId, ii));

            iOutputPortIndex = ii;
            break;
        }
    }
    if (ii == VideoPortParameters.nStartPortNumber + NumPorts)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::NegotiateVideoComponentParameters() Cannot find any output port ", iNodeTypeId));
        return false;
    }



    // now get input parameters


    //INPUT PORT

    // first check if encode parameters have been set correctly
    if ((0 == iVideoEncodeParam.iFrameWidth[0]) ||
            (0 == iVideoEncodeParam.iFrameHeight[0]) ||
            (0 == iVideoEncodeParam.iFrameRate[0])   ||
            (0 == iVideoEncodeParam.iBitRate[0]) ||
            (0 == iVideoInputFormat.iFrameWidth) ||
            (0 == iVideoInputFormat.iFrameHeight) ||
            (0 == iVideoInputFormat.iFrameRate)
       )
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::NegotiateVideoComponentParameters() Encode parameters not set correctly", iNodeTypeId));
        return false;
    }

    // first of all, check if the port supports the adequate port format
    OMX_VIDEO_PARAM_PORTFORMATTYPE Video_port_format;

    OMX_COLOR_FORMATTYPE DesiredPortColorFormat = OMX_COLOR_FormatYUV420Planar;

    CONFIG_SIZE_AND_VERSION(Video_port_format);

    Video_port_format.nPortIndex = iInputPortIndex; // set input port as target

    // loop over supported formats until we hit the one we want
    // or until the component has no more supported formats (in which case it returns OMX_ErrorNoMore
    Err = OMX_ErrorNone;
    Video_port_format.nIndex = 0; //init the format counter
    while (OMX_ErrorNone == Err)
    {

        Err = OMX_GetParameter(iOMXEncoder, OMX_IndexParamVideoPortFormat, &Video_port_format);
        if ((OMX_ErrorNone != Err))
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXEncNode-%s::NegotiateVideoComponentParameters() Problem getting port format for input port %d or no desired port format found", iNodeTypeId, iInputPortIndex));
            return false;
        }

        if ((Video_port_format.eColorFormat == DesiredPortColorFormat))
            break;

        Video_port_format.nIndex ++;
    }

    // OK, we've found the desired format, set it as the one used
    Err = OMX_SetParameter(iOMXEncoder, OMX_IndexParamVideoPortFormat, &Video_port_format);
    if ((OMX_ErrorNone != Err))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::NegotiateVideoComponentParameters() Problem setting port format for input port %d ", iNodeTypeId, iInputPortIndex));
        return false;
    }



    CONFIG_SIZE_AND_VERSION(iParamPort);
    iParamPort.nPortIndex = iInputPortIndex;
    Err = OMX_GetParameter(iOMXEncoder, OMX_IndexParamPortDefinition, &iParamPort);
    if (Err != OMX_ErrorNone)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::NegotiateVideoComponentParameters() Problem negotiating with input port %d ", iNodeTypeId, iInputPortIndex));
        return false;
    }

    if (iInFormat == PVMF_MIME_RGB24)
    {
        iOMXComponentInputBufferSize = (iVideoInputFormat.iFrameWidth * iVideoInputFormat.iFrameHeight * 3);
        iParamPort.format.video.eColorFormat = OMX_COLOR_Format24bitRGB888;
    }
    else if (iInFormat == PVMF_MIME_RGB12)
    {
        iOMXComponentInputBufferSize = (iVideoInputFormat.iFrameWidth * iVideoInputFormat.iFrameHeight * 2);
        iParamPort.format.video.eColorFormat = OMX_COLOR_Format12bitRGB444;
    }
    else if (iInFormat == PVMF_MIME_YUV420)
    {
        iOMXComponentInputBufferSize = (iVideoInputFormat.iFrameWidth * iVideoInputFormat.iFrameHeight * 3) >> 1;
        iParamPort.format.video.eColorFormat = OMX_COLOR_FormatYUV420Planar;
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::NegotiateVideoComponentParameters() Problem with input port %d color format", iNodeTypeId, iInputPortIndex));
        return false;
    }

    //Set the input buffer size to the encoder component
    iParamPort.nBufferSize = iOMXComponentInputBufferSize;

    // set the width and height of video frame and input framerate

    iParamPort.format.video.nFrameWidth = iVideoInputFormat.iFrameWidth;
    iParamPort.format.video.nFrameHeight = iVideoInputFormat.iFrameHeight;
    // This is Q16 value, so shift by 16 first and cast to preserve accuracy
    iParamPort.format.video.xFramerate = (uint32)(iVideoInputFormat.iFrameRate * (1 << 16));

    // indicate that input is uncompressed so that color format is valid
    iParamPort.format.video.eCompressionFormat = OMX_VIDEO_CodingUnused;


    // let the component decide about the number of input buffers
    iNumInputBuffers = iParamPort.nBufferCountActual;

    // do we need to increase the number of buffers?
    if (iNumInputBuffers < iParamPort.nBufferCountMin)
        iNumInputBuffers = iParamPort.nBufferCountMin;


    // if component allows us to allocate buffers, we'll decide how many to allocate
    if (iOMXComponentSupportsExternalInputBufferAlloc && (iParamPort.nBufferCountMin < NUMBER_INPUT_BUFFER))
    {
        // preset the number of input buffers
        iNumInputBuffers = NUMBER_INPUT_BUFFER;
    }

    // set the number of input buffer
    iParamPort.nBufferCountActual = iNumInputBuffers;

    // set the number of actual input buffers
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXEncNode-%s::NegotiateVideoComponentParameters() Inport buffers %d,size %d", iNodeTypeId, iNumInputBuffers, iOMXComponentInputBufferSize));

    // lock in the input port parameters
    CONFIG_SIZE_AND_VERSION(iParamPort);
    Err = OMX_SetParameter(iOMXEncoder, OMX_IndexParamPortDefinition, &iParamPort);
    if (Err != OMX_ErrorNone)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::NegotiateVideoComponentParameters() Problem setting parameters in input port %d ", iNodeTypeId, iInputPortIndex));
        return false;
    }


//////////////////// OUTPUT PORT //////////////////////////////////////////////
    CONFIG_SIZE_AND_VERSION(Video_port_format);

    Video_port_format.nPortIndex = iOutputPortIndex; // set output port as target
    OMX_VIDEO_CODINGTYPE DesiredPortFormat = OMX_VIDEO_CodingUnused;
    if (iOutFormat == PVMF_MIME_M4V)
    {
        DesiredPortFormat = OMX_VIDEO_CodingMPEG4;
    }
    else if (iOutFormat == PVMF_MIME_H2631998 ||
             iOutFormat == PVMF_MIME_H2632000)
    {
        DesiredPortFormat = OMX_VIDEO_CodingH263;
    }
    else if (iOutFormat == PVMF_MIME_H264_VIDEO_RAW ||
             iOutFormat == PVMF_MIME_H264_VIDEO_MP4)
    {
        DesiredPortFormat = OMX_VIDEO_CodingAVC;
    }
    else
    {
        DesiredPortFormat = OMX_VIDEO_CodingUnused;
    }

    if (DesiredPortFormat == OMX_VIDEO_CodingUnused)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::NegotiateVideoComponentParameters() Problem with output port %d format", iNodeTypeId, iOutputPortIndex));
        return false;
    }
    // loop over supported formats until we hit the one we want
    // or until the component has no more supported formats (in which case it returns OMX_ErrorNoMore
    Err = OMX_ErrorNone;
    Video_port_format.nIndex = 0; //init the format counter
    while (OMX_ErrorNone == Err)
    {

        Err = OMX_GetParameter(iOMXEncoder, OMX_IndexParamVideoPortFormat, &Video_port_format);
        if ((OMX_ErrorNone != Err))
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXEncNode-%s::NegotiateVideoComponentParameters() Problem getting port format for output port %d or no desired port format found", iNodeTypeId, iOutputPortIndex));
            return false;
        }

        if ((Video_port_format.eCompressionFormat == DesiredPortFormat))
            break;

        Video_port_format.nIndex ++;
    }

    // OK, we've found the desired format, set it as the one used
    Err = OMX_SetParameter(iOMXEncoder, OMX_IndexParamVideoPortFormat, &Video_port_format);
    if ((OMX_ErrorNone != Err))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::NegotiateVideoComponentParameters() Problem setting port format for output port %d ", iNodeTypeId, iOutputPortIndex));
        return false;
    }




    //Port 1 for output port
    CONFIG_SIZE_AND_VERSION(iParamPort);
    iParamPort.nPortIndex = iOutputPortIndex;
    Err = OMX_GetParameter(iOMXEncoder, OMX_IndexParamPortDefinition, &iParamPort);
    if (Err != OMX_ErrorNone)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::NegotiateVideoComponentParameters() Problem negotiating with output port %d ", iNodeTypeId, iOutputPortIndex));
        return false;
    }

    // let the component decide output buffer size
    iOMXComponentOutputBufferSize = iParamPort.nBufferSize;

    // let the component decide num output buffers
    iNumOutputBuffers = iParamPort.nBufferCountActual;


    //check the number
    if (iNumOutputBuffers < iParamPort.nBufferCountMin)
        iNumOutputBuffers = iParamPort.nBufferCountMin;


    // set the number ourselves
    if (iOMXComponentSupportsExternalOutputBufferAlloc && (iParamPort.nBufferCountMin < NUMBER_OUTPUT_BUFFER))
    {
        iNumOutputBuffers = NUMBER_OUTPUT_BUFFER;
    }

    iParamPort.nBufferCountActual = iNumOutputBuffers;


    // set the output (target) bitrate, framerate, width/height etc.
    iParamPort.format.video.nFrameWidth = iVideoEncodeParam.iFrameWidth[0];
    iParamPort.format.video.nFrameHeight = iVideoEncodeParam.iFrameHeight[0];
    // Q16 value, cast after the shift to preserve the accuracy.
    iParamPort.format.video.xFramerate = (uint32)(iVideoEncodeParam.iFrameRate[0] * (1 << 16));

    iParamPort.format.video.nBitrate = iVideoEncodeParam.iBitRate[0];
    iParamPort.format.video.eColorFormat = OMX_COLOR_FormatUnused;

    if (iOutFormat == PVMF_MIME_M4V)
    {
        iParamPort.format.video.eCompressionFormat = OMX_VIDEO_CodingMPEG4;
    }
    else if (iOutFormat == PVMF_MIME_H2631998 ||
             iOutFormat == PVMF_MIME_H2632000)
    {
        iParamPort.format.video.eCompressionFormat = OMX_VIDEO_CodingH263;
    }
    else if (iOutFormat == PVMF_MIME_H264_VIDEO_RAW ||
             iOutFormat == PVMF_MIME_H264_VIDEO_MP4)
    {
        iParamPort.format.video.eCompressionFormat = OMX_VIDEO_CodingAVC;
    }
    else
    {
        iParamPort.format.video.eCompressionFormat = OMX_VIDEO_CodingAutoDetect;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXEncNode-%s::NegotiateVideoComponentParameters() Outport buffers %d,size %d", iNodeTypeId, iNumOutputBuffers, iOMXComponentOutputBufferSize));
    CONFIG_SIZE_AND_VERSION(iParamPort);
    Err = OMX_SetParameter(iOMXEncoder, OMX_IndexParamPortDefinition, &iParamPort);
    if (Err != OMX_ErrorNone)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::NegotiateVideoComponentParameters() Problem setting parameters in output port %d ", iNodeTypeId, iOutputPortIndex));
        return false;
    }

    InputRotationType.nPortIndex = iInputPortIndex;
    Err = OMX_GetParameter(iOMXEncoder, OMX_IndexConfigCommonRotate, &InputRotationType);
    if (Err != OMX_ErrorNone)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::NegotiateVideoComponentParameters() Problem getting OMX_IndexConfigCommonRotate param ", iNodeTypeId));
        return false;
    }

    //Set the OMX_CONFIG_ROTATIONTYPE parameters
    InputRotationType.nRotation = ((iVideoInputFormat.iFrameOrientation == 1) ? 180 : 0);
    Err = OMX_SetParameter(iOMXEncoder, OMX_IndexConfigCommonRotate, &InputRotationType);
    if (Err != OMX_ErrorNone)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::NegotiateVideoComponentParameters() Problem setting OMX_IndexConfigCommonRotate param ", iNodeTypeId));
        return false;
    }


    // now call codec specific parameter setting
    bool status = true;
    if (iOutFormat == PVMF_MIME_M4V)
    {
        status = SetMP4EncoderParameters();
    }
    else if (iOutFormat == PVMF_MIME_H2631998 ||
             iOutFormat == PVMF_MIME_H2632000)
    {
        status = SetH263EncoderParameters();
    }
    else if (iOutFormat == PVMF_MIME_H264_VIDEO_RAW ||
             iOutFormat == PVMF_MIME_H264_VIDEO_MP4)
    {
        status = SetH264EncoderParameters();
    }

    return status;
}
bool PVMFOMXEncNode::SetMP4EncoderParameters()
{
    OMX_ERRORTYPE Err = OMX_ErrorNone;

    OMX_VIDEO_PARAM_MPEG4TYPE Mpeg4Type;
    OMX_VIDEO_PARAM_BITRATETYPE BitRateType;
    OMX_VIDEO_PARAM_QUANTIZATIONTYPE QuantParam;
    OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE ErrCorrType;

    // DV: VALUES HERE ARE FOR THE MOST PART HARDCODED BASED ON PV DEFAULTS
    OMX_VIDEO_PARAM_MOTIONVECTORTYPE MotionVector;
    OMX_VIDEO_PARAM_INTRAREFRESHTYPE RefreshParam;


    CONFIG_SIZE_AND_VERSION(Mpeg4Type);
    Mpeg4Type.nPortIndex = iOutputPortIndex;

    Err = OMX_GetParameter(iOMXEncoder, OMX_IndexParamVideoMpeg4, &Mpeg4Type);
    if (OMX_ErrorNone != Err)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::SetMP4EncoderParameters Parameter Invalid", iNodeTypeId));
    }


    //Set the OMX_VIDEO_PARAM_MPEG4TYPE parameters

    Mpeg4Type.nPortIndex = iOutputPortIndex;
    // extra parameters - hardcoded
    Mpeg4Type.nSliceHeaderSpacing = 0;
    Mpeg4Type.bSVH = OMX_FALSE; //((iEncoderParam.iContentType == EI_H263)? true: false);
    Mpeg4Type.bGov = OMX_FALSE; // disable or enable GOV header
    // extra parameters - hardcoded
    Mpeg4Type.nAllowedPictureTypes = OMX_VIDEO_PictureTypeI | OMX_VIDEO_PictureTypeP;


    // params based on iFrame interval
    if (iVideoEncodeParam.iIFrameInterval == -1) // encode only one frame
    {
        Mpeg4Type.nPFrames = 0xFFFFFFFF;
    }
    else if (iVideoEncodeParam.iIFrameInterval == 0) // no P frames
    {
        Mpeg4Type.nPFrames = 0;
        Mpeg4Type.nAllowedPictureTypes = OMX_VIDEO_PictureTypeI; // maps to only supporting I-frames
    }
    else
    {
        Mpeg4Type.nPFrames = (OMX_U32)(iVideoEncodeParam.iIFrameInterval * iVideoEncodeParam.iFrameRate[0] - 1);
    }

    // extra parameters - hardcoded
    Mpeg4Type.nBFrames = 0;
    Mpeg4Type.nIDCVLCThreshold = 0;
    Mpeg4Type.bACPred = OMX_TRUE;
    Mpeg4Type.nMaxPacketSize = iVideoEncodeParam.iPacketSize;
    Mpeg4Type.nTimeIncRes = 1000; // (in relation to (should be higher than) frame rate )
    Mpeg4Type.nHeaderExtension = 0;
    Mpeg4Type.bReversibleVLC = ((iVideoEncodeParam.iRVLCEnable == true) ? OMX_TRUE : OMX_FALSE);

    switch (iVideoEncodeParam.iProfileLevel)
    {

        case EI_SIMPLE_LEVEL0:
            Mpeg4Type.eProfile = OMX_VIDEO_MPEG4ProfileSimple;
            Mpeg4Type.eLevel = OMX_VIDEO_MPEG4Level0;
            break;

        case EI_SIMPLE_LEVEL1:
            Mpeg4Type.eProfile = OMX_VIDEO_MPEG4ProfileSimple;
            Mpeg4Type.eLevel = OMX_VIDEO_MPEG4Level1;
            break;

        case EI_SIMPLE_LEVEL2:
            Mpeg4Type.eProfile = OMX_VIDEO_MPEG4ProfileSimple;
            Mpeg4Type.eLevel = OMX_VIDEO_MPEG4Level2;
            break;

        case EI_SIMPLE_LEVEL3:
            Mpeg4Type.eProfile = OMX_VIDEO_MPEG4ProfileSimple;
            Mpeg4Type.eLevel = OMX_VIDEO_MPEG4Level3;
            break;

        case EI_CORE_LEVEL1:
            Mpeg4Type.eProfile = OMX_VIDEO_MPEG4ProfileCore;
            Mpeg4Type.eLevel = OMX_VIDEO_MPEG4Level1;
            break;

        case EI_CORE_LEVEL2:
            Mpeg4Type.eProfile = OMX_VIDEO_MPEG4ProfileCore;
            Mpeg4Type.eLevel = OMX_VIDEO_MPEG4Level2;
            break;

        case EI_SIMPLE_SCALABLE_LEVEL0:
            Mpeg4Type.eProfile = OMX_VIDEO_MPEG4ProfileSimpleScalable;
            Mpeg4Type.eLevel = OMX_VIDEO_MPEG4Level0;
            break;

        case EI_SIMPLE_SCALABLE_LEVEL1:
            Mpeg4Type.eProfile = OMX_VIDEO_MPEG4ProfileSimpleScalable;
            Mpeg4Type.eLevel = OMX_VIDEO_MPEG4Level1;
            break;

        case EI_SIMPLE_SCALABLE_LEVEL2:
            Mpeg4Type.eProfile = OMX_VIDEO_MPEG4ProfileSimpleScalable;
            Mpeg4Type.eLevel = OMX_VIDEO_MPEG4Level2;
            break;

        case EI_CORE_SCALABLE_LEVEL1:
            Mpeg4Type.eProfile = OMX_VIDEO_MPEG4ProfileCoreScalable;
            Mpeg4Type.eLevel = OMX_VIDEO_MPEG4Level1;
            break;

        case EI_CORE_SCALABLE_LEVEL2:
            Mpeg4Type.eProfile = OMX_VIDEO_MPEG4ProfileCoreScalable;
            Mpeg4Type.eLevel = OMX_VIDEO_MPEG4Level2;
            break;

        case EI_CORE_SCALABLE_LEVEL3:
            Mpeg4Type.eProfile = OMX_VIDEO_MPEG4ProfileCoreScalable;
            Mpeg4Type.eLevel = OMX_VIDEO_MPEG4Level3;
            break;

    }

    Err = OMX_SetParameter(iOMXEncoder, OMX_IndexParamVideoMpeg4, &Mpeg4Type);
    if (OMX_ErrorNone != Err)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::SetMP4EncoderParameters Parameter Invalid", iNodeTypeId));
    }

    //OMX_VIDEO_PARAM_BITRATETYPE Settings
    CONFIG_SIZE_AND_VERSION(BitRateType);

    BitRateType.nPortIndex = iOutputPortIndex;
    Err = OMX_GetParameter(iOMXEncoder, OMX_IndexParamVideoBitrate, &BitRateType);

    if (OMX_ErrorNone != Err)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::SetMP4EncoderParameters Parameter Invalid", iNodeTypeId));
    }

    //Set the parameters now
    BitRateType.nPortIndex = iOutputPortIndex;
    switch (iVideoEncodeParam.iRateControlType)
    {
        case ECONSTANT_Q:
            BitRateType.eControlRate = OMX_Video_ControlRateDisable;
            break;
        case ECBR_1:
            BitRateType.eControlRate = OMX_Video_ControlRateConstant;
            break;
        case EVBR_1:
            BitRateType.eControlRate = OMX_Video_ControlRateVariable;
            break;
        default:
            BitRateType.eControlRate = OMX_Video_ControlRateDisable;
            break;
    }

    BitRateType.nTargetBitrate = iVideoEncodeParam.iBitRate[0];
    Err = OMX_SetParameter(iOMXEncoder, OMX_IndexParamVideoBitrate, &BitRateType);

    if (OMX_ErrorNone != Err)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::SetMP4EncoderParameters Parameter Invalid", iNodeTypeId));
    }

    //OMX_VIDEO_PARAM_QUANTIZATIONTYPE Settings
    if (BitRateType.eControlRate == OMX_Video_ControlRateDisable)
    {
        CONFIG_SIZE_AND_VERSION(QuantParam);
        QuantParam.nPortIndex = iOutputPortIndex;

        Err = OMX_GetParameter(iOMXEncoder, OMX_IndexParamVideoQuantization, &QuantParam);
        if (OMX_ErrorNone != Err)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXEncNode-%s::SetMP4EncoderParameters Parameter Invalid", iNodeTypeId));
        }

        //Set the parameters now
        QuantParam.nPortIndex = iOutputPortIndex;
        QuantParam.nQpI = DEFAULT_OMX_MP4ENC_QPI;
        QuantParam.nQpP = DEFAULT_OMX_MP4ENC_QPP;
        QuantParam.nQpB = DEFAULT_OMX_MP4ENC_QPB;
        Err = OMX_SetParameter(iOMXEncoder, OMX_IndexParamVideoQuantization, &QuantParam);
        if (OMX_ErrorNone != Err)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXEncNode-%s::SetMP4EncoderParameters Parameter Invalid", iNodeTypeId));
        }
    }



    //OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE Settings (For streaming/2-way)

    CONFIG_SIZE_AND_VERSION(ErrCorrType);
    ErrCorrType.nPortIndex = iOutputPortIndex;
    Err = OMX_GetParameter(iOMXEncoder, OMX_IndexParamVideoErrorCorrection, &ErrCorrType);
    if (OMX_ErrorNone != Err)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::SetMP4EncoderParameters Parameter Invalid", iNodeTypeId));
    }

    //Set the parameters now
    ErrCorrType.nPortIndex = iOutputPortIndex;
    if (iVideoEncodeParam.iContentType == EI_M4V_STREAMING)
    {
        ErrCorrType.bEnableDataPartitioning = OMX_TRUE;
        ErrCorrType.bEnableResync = OMX_TRUE;
    }
    else
    {
        ErrCorrType.bEnableDataPartitioning = OMX_FALSE;
        ErrCorrType.bEnableResync = OMX_FALSE;
    }

    // extra parameters - hardcoded
    ErrCorrType.bEnableHEC = OMX_FALSE;
    ErrCorrType.nResynchMarkerSpacing = iVideoEncodeParam.iPacketSize;
    ErrCorrType.bEnableRVLC = ((iVideoEncodeParam.iRVLCEnable == true) ? OMX_TRUE : OMX_FALSE); // corresponds to encode param rvlcEnable

    Err = OMX_SetParameter(iOMXEncoder, OMX_IndexParamVideoErrorCorrection, &ErrCorrType);
    if (OMX_ErrorNone != Err)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::SetMP4EncoderParameters Parameter Invalid", iNodeTypeId));
    }


    //OMX_VIDEO_PARAM_MOTIONVECTORTYPE Settings
    CONFIG_SIZE_AND_VERSION(MotionVector);
    MotionVector.nPortIndex = iOutputPortIndex;

    Err = OMX_GetParameter(iOMXEncoder, OMX_IndexParamVideoMotionVector, &MotionVector);
    if (OMX_ErrorNone != Err)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::SetMP4EncoderParameters Parameter Invalid", iNodeTypeId));
    }

    MotionVector.nPortIndex = iOutputPortIndex;

    // extra parameters - hardcoded
    MotionVector.sXSearchRange = iVideoEncodeParam.iSearchRange;
    MotionVector.sYSearchRange = iVideoEncodeParam.iSearchRange;
    MotionVector.bFourMV = ((iVideoEncodeParam.iMV8x8 == true) ? OMX_TRUE : OMX_FALSE);
    MotionVector.eAccuracy = ((iVideoEncodeParam.iMVHalfPel == true) ? OMX_Video_MotionVectorHalfPel : OMX_Video_MotionVectorPixel);
    MotionVector.bUnrestrictedMVs = OMX_TRUE;

    Err = OMX_SetParameter(iOMXEncoder, OMX_IndexParamVideoMotionVector, &MotionVector);
    if (OMX_ErrorNone != Err)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::SetMP4EncoderParameters Parameter Invalid", iNodeTypeId));
    }


    //OMX_VIDEO_PARAM_INTRAREFRESHTYPE Settings
    CONFIG_SIZE_AND_VERSION(RefreshParam);
    RefreshParam.nPortIndex = iOutputPortIndex;

    Err = OMX_GetParameter(iOMXEncoder, OMX_IndexParamVideoIntraRefresh, &RefreshParam);
    if (OMX_ErrorNone != Err)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::SetMP4EncoderParameters Parameter Invalid", iNodeTypeId));
    }

    // extra parameters - hardcoded based on PV defaults
    RefreshParam.nPortIndex = iOutputPortIndex;
    RefreshParam.eRefreshMode = OMX_VIDEO_IntraRefreshBoth;
    RefreshParam.nCirMBs = iVideoEncodeParam.iNumIntraMBRefresh;

    Err = OMX_SetParameter(iOMXEncoder, OMX_IndexParamVideoIntraRefresh, &RefreshParam);
    if (OMX_ErrorNone != Err)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::SetMP4EncoderParameters Parameter Invalid", iNodeTypeId));
    }

    return true;
}

bool PVMFOMXEncNode::SetH263EncoderParameters()
{

    OMX_ERRORTYPE Err = OMX_ErrorNone;

    OMX_VIDEO_PARAM_H263TYPE H263Type;
    OMX_VIDEO_PARAM_BITRATETYPE BitRateType;
    OMX_VIDEO_PARAM_QUANTIZATIONTYPE QuantParam;
    OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE ErrCorrType;

    // DV: VALUES HERE ARE FOR THE MOST PART HARDCODED BASED ON PV DEFAULTS
    OMX_VIDEO_PARAM_MOTIONVECTORTYPE MotionVector;
    OMX_VIDEO_PARAM_INTRAREFRESHTYPE RefreshParam;


    CONFIG_SIZE_AND_VERSION(H263Type);
    H263Type.nPortIndex = iOutputPortIndex;

    Err = OMX_GetParameter(iOMXEncoder, OMX_IndexParamVideoH263, &H263Type);
    if (OMX_ErrorNone != Err)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::SetH263EncoderParameters Parameter Invalid", iNodeTypeId));
    }


    //Set the OMX_VIDEO_PARAM_H263TYPE parameters

    //DV: Here, we only set the nPFrames and AllowedFrameTypes, i.e. iIFrameInterval related variables

    H263Type.nPortIndex = iOutputPortIndex;

    H263Type.nAllowedPictureTypes = OMX_VIDEO_PictureTypeI | OMX_VIDEO_PictureTypeP;
    if (iVideoEncodeParam.iIFrameInterval == -1) // encode only one I frame followed by P frames
    {
        H263Type.nPFrames = 0xFFFFFFFF;
    }
    else if (iVideoEncodeParam.iIFrameInterval == 0) // no P frames
    {
        H263Type.nPFrames = 0;
        H263Type.nAllowedPictureTypes = OMX_VIDEO_PictureTypeI; // maps to only supporting I-frames
    }
    else
    {
        H263Type.nPFrames = (OMX_U32)(iVideoEncodeParam.iIFrameInterval * iVideoEncodeParam.iFrameRate[0] - 1);
    }

    // extra parameters - hardcoded
    H263Type.nBFrames = 0;
    H263Type.eProfile = OMX_VIDEO_H263ProfileBaseline;
    H263Type.eLevel = OMX_VIDEO_H263Level45;
    H263Type.bPLUSPTYPEAllowed = OMX_FALSE;
    H263Type.bForceRoundingTypeToZero = OMX_FALSE;
    H263Type.nPictureHeaderRepetition = 0;
    H263Type.nGOBHeaderInterval = 2;

    Err = OMX_SetParameter(iOMXEncoder, OMX_IndexParamVideoH263, &H263Type);
    if (OMX_ErrorNone != Err)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::SetH263EncoderParameters Parameter Invalid", iNodeTypeId));
    }

    //OMX_VIDEO_PARAM_BITRATETYPE Settings
    CONFIG_SIZE_AND_VERSION(BitRateType);

    BitRateType.nPortIndex = iOutputPortIndex;
    Err = OMX_GetParameter(iOMXEncoder, OMX_IndexParamVideoBitrate, &BitRateType);
    if (OMX_ErrorNone != Err)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::SetH263EncoderParameters Parameter Invalid", iNodeTypeId));
    }

    //Set the parameters now
    BitRateType.nPortIndex = iOutputPortIndex;
    switch (iVideoEncodeParam.iRateControlType)
    {
        case ECONSTANT_Q:
            BitRateType.eControlRate = OMX_Video_ControlRateDisable;
            break;
        case ECBR_1:
            BitRateType.eControlRate = OMX_Video_ControlRateConstant;
            break;
        case EVBR_1:
            BitRateType.eControlRate = OMX_Video_ControlRateVariable;
            break;
        default:
            BitRateType.eControlRate = OMX_Video_ControlRateDisable;
            break;
    }

    BitRateType.nTargetBitrate = iVideoEncodeParam.iBitRate[0];
    Err = OMX_SetParameter(iOMXEncoder, OMX_IndexParamVideoBitrate, &BitRateType);
    if (OMX_ErrorNone != Err)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::SetH263EncoderParameters Parameter Invalid", iNodeTypeId));
    }


    //OMX_VIDEO_PARAM_QUANTIZATIONTYPE Settings
    if (BitRateType.eControlRate == OMX_Video_ControlRateDisable)
    {
        CONFIG_SIZE_AND_VERSION(QuantParam);
        QuantParam.nPortIndex = iOutputPortIndex;

        Err = OMX_GetParameter(iOMXEncoder, OMX_IndexParamVideoQuantization, &QuantParam);
        if (OMX_ErrorNone != Err)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXEncNode-%s::SetH263EncoderParameters Parameter Invalid", iNodeTypeId));
        }

        //Set the parameters now
        QuantParam.nPortIndex = iOutputPortIndex;
        QuantParam.nQpI = DEFAULT_OMX_MP4ENC_QPI;
        QuantParam.nQpP = DEFAULT_OMX_MP4ENC_QPP;
        QuantParam.nQpB = DEFAULT_OMX_MP4ENC_QPB;
        Err = OMX_SetParameter(iOMXEncoder, OMX_IndexParamVideoQuantization, &QuantParam);
        if (OMX_ErrorNone != Err)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXEncNode-%s::SetH263EncoderParameters Parameter Invalid", iNodeTypeId));
        }
    }


    //OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE Settings (For streaming/2-way)

    CONFIG_SIZE_AND_VERSION(ErrCorrType);
    ErrCorrType.nPortIndex = iOutputPortIndex;
    Err = OMX_GetParameter(iOMXEncoder, OMX_IndexParamVideoErrorCorrection, &ErrCorrType);
    if (OMX_ErrorNone != Err)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::SetH263EncoderParameters Parameter Invalid", iNodeTypeId));
    }

    //Set the parameters now
    ErrCorrType.nPortIndex = iOutputPortIndex;
    //if (iVideoEncodeParam.iContentType == EI_M4V_STREAMING)
    //{
    //    ErrCorrType.bEnableDataPartitioning = OMX_TRUE;
    //}
    //else
    //{
    //    ErrCorrType.bEnableDataPartitioning = OMX_FALSE;
    //}
    ErrCorrType.bEnableHEC = OMX_FALSE;
    ErrCorrType.bEnableResync = OMX_FALSE;
    ErrCorrType.nResynchMarkerSpacing = 0;
    ErrCorrType.bEnableRVLC = OMX_FALSE;		// corresponds to encode param rvlcEnable
    Err = OMX_SetParameter(iOMXEncoder, OMX_IndexParamVideoErrorCorrection, &ErrCorrType);
    if (OMX_ErrorNone != Err)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::SetH263EncoderParameters Parameter Invalid", iNodeTypeId));
    }



    //OMX_VIDEO_PARAM_MOTIONVECTORTYPE Settings
    CONFIG_SIZE_AND_VERSION(MotionVector);
    MotionVector.nPortIndex = iOutputPortIndex;

    Err = OMX_GetParameter(iOMXEncoder, OMX_IndexParamVideoMotionVector, &MotionVector);
    if (OMX_ErrorNone != Err)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::SetH263EncoderParameters Parameter Invalid", iNodeTypeId));
    }

    // extra parameters - hardcoded
    MotionVector.sXSearchRange = iVideoEncodeParam.iSearchRange;
    MotionVector.sYSearchRange = iVideoEncodeParam.iSearchRange;
    MotionVector.bFourMV =  OMX_FALSE;
    MotionVector.eAccuracy = ((iVideoEncodeParam.iMVHalfPel == true) ? OMX_Video_MotionVectorHalfPel : OMX_Video_MotionVectorPixel);
    MotionVector.bUnrestrictedMVs = OMX_FALSE;


    Err = OMX_SetParameter(iOMXEncoder, OMX_IndexParamVideoMotionVector, &MotionVector);
    if (OMX_ErrorNone != Err)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::SetH263EncoderParameters Parameter Invalid", iNodeTypeId));
    }


    //OMX_VIDEO_PARAM_INTRAREFRESHTYPE Settings

    CONFIG_SIZE_AND_VERSION(RefreshParam);
    RefreshParam.nPortIndex = iOutputPortIndex;

    Err = OMX_GetParameter(iOMXEncoder, OMX_IndexParamVideoIntraRefresh, &RefreshParam);
    if (OMX_ErrorNone != Err)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::SetH263EncoderParameters Parameter Invalid", iNodeTypeId));
    }

    // extra parameters - hardcoded based on PV defaults
    RefreshParam.nPortIndex = iOutputPortIndex;
    RefreshParam.eRefreshMode = OMX_VIDEO_IntraRefreshBoth;
    RefreshParam.nCirMBs = iVideoEncodeParam.iNumIntraMBRefresh;


    Err = OMX_SetParameter(iOMXEncoder, OMX_IndexParamVideoIntraRefresh, &RefreshParam);
    if (OMX_ErrorNone != Err)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::SetH263EncoderParameters Parameter Invalid", iNodeTypeId));
    }

    return true;
}

bool PVMFOMXEncNode::SetH264EncoderParameters()
{
    OMX_ERRORTYPE Err = OMX_ErrorNone;

    OMX_VIDEO_PARAM_AVCTYPE H264Type;
    OMX_VIDEO_PARAM_BITRATETYPE BitRateType;
    OMX_VIDEO_PARAM_QUANTIZATIONTYPE QuantParam;

    // to be refined
    OMX_VIDEO_PARAM_MOTIONVECTORTYPE MotionVector;
    OMX_VIDEO_PARAM_INTRAREFRESHTYPE RefreshParam;
    OMX_VIDEO_PARAM_VBSMCTYPE VbsmcType;


    CONFIG_SIZE_AND_VERSION(H264Type);
    H264Type.nPortIndex = iOutputPortIndex;

    Err = OMX_GetParameter(iOMXEncoder, OMX_IndexParamVideoAvc, &H264Type);
    if (OMX_ErrorNone != Err)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::SetH264EncoderParameters Parameter Invalid", iNodeTypeId));
    }


    H264Type.nPortIndex = iOutputPortIndex;

    H264Type.nAllowedPictureTypes  = OMX_VIDEO_PictureTypeI | OMX_VIDEO_PictureTypeP;
    if (iVideoEncodeParam.iIFrameInterval == -1) // encode only one I frame followed by P frames
    {
        H264Type.nPFrames = 0xFFFFFFFF;
    }
    else if (iVideoEncodeParam.iIFrameInterval == 0) // no P frames
    {
        H264Type.nPFrames = 0;
        H264Type.nAllowedPictureTypes = OMX_VIDEO_PictureTypeI; // maps to only supporting I-frames
    }
    else
    {
        H264Type.nPFrames = (OMX_U32)(iVideoEncodeParam.iIFrameInterval * iVideoEncodeParam.iFrameRate[0] - 1);
    }

    // extra parameters -hardcoded
    H264Type.nSliceHeaderSpacing = 0;
    H264Type.nBFrames = 0;
    H264Type.bUseHadamard = OMX_TRUE;
    H264Type.nRefFrames = 1;
    H264Type.nRefIdx10ActiveMinus1 = 0;
    H264Type.nRefIdx11ActiveMinus1 = 0;
    H264Type.bEnableUEP = OMX_FALSE;
    H264Type.bEnableFMO = OMX_FALSE;
    H264Type.bEnableASO = OMX_FALSE;
    H264Type.bEnableRS = OMX_FALSE;
    H264Type.eProfile = OMX_VIDEO_AVCProfileBaseline;
    H264Type.eLevel = OMX_VIDEO_AVCLevel1b;
    H264Type.bFrameMBsOnly = OMX_TRUE;
    H264Type.bMBAFF = OMX_FALSE;
    H264Type.bEntropyCodingCABAC = OMX_FALSE;
    H264Type.bWeightedPPrediction = OMX_FALSE;
    H264Type.bconstIpred = OMX_FALSE;
    H264Type.bDirect8x8Inference = OMX_FALSE;
    H264Type.bDirectSpatialTemporal = OMX_FALSE;
    H264Type.nCabacInitIdc = 0;
    H264Type.eLoopFilterMode = OMX_VIDEO_AVCLoopFilterEnable;


    Err = OMX_SetParameter(iOMXEncoder, OMX_IndexParamVideoAvc, &H264Type);
    if (OMX_ErrorNone != Err)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::SetH264EncoderParameters Parameter Invalid", iNodeTypeId));
    }


    //OMX_VIDEO_PARAM_BITRATETYPE Settings
    CONFIG_SIZE_AND_VERSION(BitRateType);

    BitRateType.nPortIndex = iOutputPortIndex;
    Err = OMX_GetParameter(iOMXEncoder, OMX_IndexParamVideoBitrate, &BitRateType);
    if (OMX_ErrorNone != Err)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::SetH264EncoderParameters Parameter Invalid", iNodeTypeId));
    }

    //Set the parameters now
    BitRateType.nPortIndex = iOutputPortIndex;
    switch (iVideoEncodeParam.iRateControlType)
    {
        case ECONSTANT_Q:
            BitRateType.eControlRate = OMX_Video_ControlRateDisable;
            break;
        case ECBR_1:
            BitRateType.eControlRate = OMX_Video_ControlRateConstant;
            break;
        case EVBR_1:
            BitRateType.eControlRate = OMX_Video_ControlRateVariable;
            break;
        default:
            BitRateType.eControlRate = OMX_Video_ControlRateDisable;
            break;
    }

    BitRateType.nTargetBitrate = iVideoEncodeParam.iBitRate[0];
    Err = OMX_SetParameter(iOMXEncoder, OMX_IndexParamVideoBitrate, &BitRateType);
    if (OMX_ErrorNone != Err)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::SetH264EncoderParameters Parameter Invalid", iNodeTypeId));
    }


    //OMX_VIDEO_PARAM_QUANTIZATIONTYPE Settings
    if (BitRateType.eControlRate == OMX_Video_ControlRateDisable)
    {
        CONFIG_SIZE_AND_VERSION(QuantParam);
        QuantParam.nPortIndex = iOutputPortIndex;

        Err = OMX_GetParameter(iOMXEncoder, OMX_IndexParamVideoQuantization, &QuantParam);
        if (OMX_ErrorNone != Err)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXEncNode-%s::SetH264EncoderParameters Parameter Invalid", iNodeTypeId));
        }

        //Set the parameters now
        QuantParam.nPortIndex = iOutputPortIndex;
        QuantParam.nQpI = DEFAULT_OMX_AVCENC_QPI;
        QuantParam.nQpP = DEFAULT_OMX_AVCENC_QPP;
        QuantParam.nQpB = DEFAULT_OMX_AVCENC_QPB;
        Err = OMX_SetParameter(iOMXEncoder, OMX_IndexParamVideoQuantization, &QuantParam);
        if (OMX_ErrorNone != Err)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXEncNode-%s::SetH264EncoderParameters Parameter Invalid", iNodeTypeId));
        }
    }




    //OMX_VIDEO_PARAM_MOTIONVECTORTYPE Settings
    CONFIG_SIZE_AND_VERSION(MotionVector);
    MotionVector.nPortIndex = iOutputPortIndex;

    Err = OMX_GetParameter(iOMXEncoder, OMX_IndexParamVideoMotionVector, &MotionVector);
    if (OMX_ErrorNone != Err)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::SetH264EncoderParameters Parameter Invalid", iNodeTypeId));
    }

    // extra parameters - hardcoded
    MotionVector.sXSearchRange = iVideoEncodeParam.iSearchRange;
    MotionVector.sYSearchRange = iVideoEncodeParam.iSearchRange;
    MotionVector.bFourMV =  OMX_FALSE;
    MotionVector.eAccuracy = OMX_Video_MotionVectorQuarterPel; // hardcoded
    MotionVector.bUnrestrictedMVs = OMX_TRUE;


    Err = OMX_SetParameter(iOMXEncoder, OMX_IndexParamVideoMotionVector, &MotionVector);
    if (OMX_ErrorNone != Err)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::SetH264EncoderParameters Parameter Invalid", iNodeTypeId));
    }


    //OMX_VIDEO_PARAM_INTRAREFRESHTYPE Settings

    CONFIG_SIZE_AND_VERSION(RefreshParam);
    RefreshParam.nPortIndex = iOutputPortIndex;

    Err = OMX_GetParameter(iOMXEncoder, OMX_IndexParamVideoIntraRefresh, &RefreshParam);
    if (OMX_ErrorNone != Err)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::SetH264EncoderParameters Parameter Invalid", iNodeTypeId));
    }

    // extra parameters - hardcoded based on PV defaults
    RefreshParam.nPortIndex = iOutputPortIndex;
    RefreshParam.eRefreshMode = OMX_VIDEO_IntraRefreshBoth;
    RefreshParam.nCirMBs = iVideoEncodeParam.iNumIntraMBRefresh;


    Err = OMX_SetParameter(iOMXEncoder, OMX_IndexParamVideoIntraRefresh, &RefreshParam);
    if (OMX_ErrorNone != Err)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::SetH264EncoderParameters Parameter Invalid", iNodeTypeId));
    }

    CONFIG_SIZE_AND_VERSION(VbsmcType);
    VbsmcType.nPortIndex = iOutputPortIndex;

    Err = OMX_GetParameter(iOMXEncoder, OMX_IndexParamVideoVBSMC, &VbsmcType);
    if (OMX_ErrorNone != Err)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::SetH264EncoderParameters Parameter Invalid", iNodeTypeId));
    }

    VbsmcType.b16x16 = OMX_TRUE;
    VbsmcType.b16x8 = VbsmcType.b8x16 = VbsmcType.b8x8 = VbsmcType.b8x4 = VbsmcType.b4x8 = VbsmcType.b4x4 = OMX_FALSE;

    Err = OMX_SetParameter(iOMXEncoder, OMX_IndexParamVideoVBSMC, &VbsmcType);
    if (OMX_ErrorNone != Err)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::SetH264EncoderParameters Parameter Invalid", iNodeTypeId));
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////
bool PVMFOMXEncNode::NegotiateAudioComponentParameters()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXEncNode-%s::NegotiateAudioParameters() In", iNodeTypeId));

    OMX_ERRORTYPE Err;

    // first get the number of ports and port indices
    OMX_PORT_PARAM_TYPE AudioPortParameters;
    uint32 NumPorts;
    uint32 ii;

    // get starting number
    CONFIG_SIZE_AND_VERSION(AudioPortParameters);

    Err = OMX_GetParameter(iOMXEncoder, OMX_IndexParamAudioInit, &AudioPortParameters);
    NumPorts = AudioPortParameters.nPorts; // must be at least 2 of them (in&out)

    if (Err != OMX_ErrorNone || NumPorts < 2)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::NegotiateAudioComponentParameters() There is insuffucient (%d) ports", iNodeTypeId, NumPorts));
        return false;
    }


    // loop through ports starting from the starting index to find index of the first input port
    for (ii = AudioPortParameters.nStartPortNumber ;ii < AudioPortParameters.nStartPortNumber + NumPorts; ii++)
    {
        // get port parameters, and determine if it is input or output
        // if there are more than 2 ports, the first one we encounter that has input direction is picked


        CONFIG_SIZE_AND_VERSION(iParamPort);
        //port
        iParamPort.nPortIndex = ii;
        Err = OMX_GetParameter(iOMXEncoder, OMX_IndexParamPortDefinition, &iParamPort);

        if (Err != OMX_ErrorNone)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXEncNode-%s::NegotiateAudioComponentParameters() Problem negotiating with port %d ", iNodeTypeId, ii));

            return false;
        }

        if (iParamPort.eDir == OMX_DirInput)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXEncNode-%s::NegotiateAudioComponentParameters() Found Input port index %d ", iNodeTypeId, ii));

            iInputPortIndex = ii;
            break;
        }
    }
    if (ii == AudioPortParameters.nStartPortNumber + NumPorts)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::NegotiateAudioComponentParameters() Cannot find any input port ", iNodeTypeId));
        return false;
    }


    // loop through ports starting from the starting index to find index of the first output port
    for (ii = AudioPortParameters.nStartPortNumber ;ii < AudioPortParameters.nStartPortNumber + NumPorts; ii++)
    {
        // get port parameters, and determine if it is input or output
        // if there are more than 2 ports, the first one we encounter that has output direction is picked


        CONFIG_SIZE_AND_VERSION(iParamPort);
        //port
        iParamPort.nPortIndex = ii;
        Err = OMX_GetParameter(iOMXEncoder, OMX_IndexParamPortDefinition, &iParamPort);

        if (Err != OMX_ErrorNone)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXEncNode-%s::NegotiateAudioComponentParameters() Problem negotiating with port %d ", iNodeTypeId, ii));

            return false;
        }

        if (iParamPort.eDir == OMX_DirOutput)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXEncNode-%s::NegotiateAudioComponentParameters() Found Output port index %d ", iNodeTypeId, ii));

            iOutputPortIndex = ii;
            break;
        }
    }
    if (ii == AudioPortParameters.nStartPortNumber + NumPorts)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::NegotiateAudioComponentParameters() Cannot find any output port ", iNodeTypeId));
        return false;
    }



    // now get input parameters


    //INPUT PORT

    // first basic check if encode parameters have been set correctly
    if ((0 == iAudioEncodeParam.iMaxNumOutputFramesPerBuffer) ||
            (0 == iAudioEncodeParam.iAMRBitrate) ||
            (0 == iAudioEncodeParam.iOutputNumChannels) ||
            (2 < iAudioEncodeParam.iOutputNumChannels) ||
            (0 == iAudioEncodeParam.iOutputSamplingRate) ||
            (0 == iAudioInputFormat.iInputBitsPerSample) ||
            (0 == iAudioInputFormat.iInputNumChannels) ||
            (2 < iAudioInputFormat.iInputNumChannels) ||
            (0 == iAudioInputFormat.iInputSamplingRate)
       )
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::NegotiateAudioComponentParameters() Encode parameters not set correctly", iNodeTypeId));
        return false;
    }

    // first of all, check if the port supports the adequate port format
    OMX_AUDIO_PARAM_PORTFORMATTYPE Audio_port_format;
    OMX_AUDIO_CODINGTYPE DesiredPortFormat = OMX_AUDIO_CodingPCM;
    CONFIG_SIZE_AND_VERSION(Audio_port_format);

    Audio_port_format.nPortIndex = iInputPortIndex; // set input port as target

    // loop over supported formats until we hit the one we want
    // or until the component has no more supported formats (in which case it returns OMX_ErrorNoMore
    Err = OMX_ErrorNone;
    Audio_port_format.nIndex = 0; //init the format counter
    while (OMX_ErrorNone == Err)
    {

        Err = OMX_GetParameter(iOMXEncoder, OMX_IndexParamAudioPortFormat, &Audio_port_format);
        if ((OMX_ErrorNone != Err))
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXEncNode-%s::NegotiateAudioComponentParameters() Problem getting port format for input port %d or no desired port format found", iNodeTypeId, iInputPortIndex));
            return false;
        }

        if (Audio_port_format.eEncoding == DesiredPortFormat)
            break;

        Audio_port_format.nIndex ++;
    }

    // OK, we've found the desired format, set it as the one used
    Err = OMX_SetParameter(iOMXEncoder, OMX_IndexParamAudioPortFormat, &Audio_port_format);
    if ((OMX_ErrorNone != Err))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::NegotiateAudioComponentParameters() Problem setting port format for input port %d ", iNodeTypeId, iInputPortIndex));
        return false;
    }




    CONFIG_SIZE_AND_VERSION(iParamPort);
    iParamPort.nPortIndex = iInputPortIndex;
    Err = OMX_GetParameter(iOMXEncoder, OMX_IndexParamPortDefinition, &iParamPort);
    if (Err != OMX_ErrorNone)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::NegotiateAudioComponentParameters() Problem negotiating with input port %d ", iNodeTypeId, iInputPortIndex));
        return false;
    }

    // TODO: FIX THIS - JUST READ THE BUFFER SIZE FROM COMPONENT PORT
    if (iInFormat == PVMF_MIME_PCM16)
    {
        iOMXComponentInputBufferSize = MAX_NUM_AMR_FRAMES_PER_BUFFER * (PVMF_AMRENC_DEFAULT_FRAME_DURATION * PVMF_AMRENC_DEFAULT_SAMPLING_RATE * PVMF_AMRENC_DEFAULT_BITSPERSAMPLE) / (1000 * 8);

    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::NegotiateAudioComponentParameters() Problem with input port %d color format", iNodeTypeId, iInputPortIndex));
        return false;
    }

    //Set the input buffer size to the encoder component
    iParamPort.nBufferSize = iOMXComponentInputBufferSize;

    // set Encoding type

    //iParamPort.format.audio.bFlagErrorConcealment = OMX_TRUE;

    // indicate that input is uncompressed i.e. PCM
    iParamPort.format.audio.eEncoding = OMX_AUDIO_CodingPCM;

    // let the component decide about the number of input buffers
    iNumInputBuffers = iParamPort.nBufferCountActual;

    // do we need to increase the number of buffers?
    if (iNumInputBuffers < iParamPort.nBufferCountMin)
        iNumInputBuffers = iParamPort.nBufferCountMin;


    // if component allows us to allocate buffers, we'll decide how many to allocate
    if (iOMXComponentSupportsExternalInputBufferAlloc && (iParamPort.nBufferCountMin < NUMBER_INPUT_BUFFER))
    {
        // preset the number of input buffers
        iNumInputBuffers = NUMBER_INPUT_BUFFER;
    }

    // set the number of input buffer
    iParamPort.nBufferCountActual = iNumInputBuffers;

    // set the number of actual input buffers
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXEncNode-%s::NegotiateAudioComponentParameters() Inport buffers %d,size %d", iNodeTypeId, iNumInputBuffers, iOMXComponentInputBufferSize));

    // lock in the input port parameters
    CONFIG_SIZE_AND_VERSION(iParamPort);
    Err = OMX_SetParameter(iOMXEncoder, OMX_IndexParamPortDefinition, &iParamPort);
    if (Err != OMX_ErrorNone)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::NegotiateAudioComponentParameters() Problem setting parameters in input port %d ", iNodeTypeId, iInputPortIndex));
        return false;
    }

    // For INPUT, we also need to set the PCM parameters, such as sampling rate, etc.
    // GET the output buffer params and sizes
    OMX_AUDIO_PARAM_PCMMODETYPE Audio_Pcm_Param;
    Audio_Pcm_Param.nPortIndex = iInputPortIndex;

    CONFIG_SIZE_AND_VERSION(Audio_Pcm_Param);


    Err = OMX_GetParameter(iOMXEncoder, OMX_IndexParamAudioPcm, &Audio_Pcm_Param);
    if (Err != OMX_ErrorNone)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::NegotiateAudioComponentParameters() Problem getting PCM parameters with input port %d ", iNodeTypeId, iInputPortIndex));
        return false;
    }


    Audio_Pcm_Param.nChannels		=	(OMX_U32) iAudioInputFormat.iInputNumChannels;
    Audio_Pcm_Param.eNumData		=	OMX_NumericalDataSigned; // signed
    Audio_Pcm_Param.eEndian			=	OMX_EndianLittle; // little-endian
    Audio_Pcm_Param.bInterleaved	=	((EINTERLEAVE_LR == iAudioInputFormat.iInputInterleaveMode) ? OMX_TRUE : OMX_FALSE);
    Audio_Pcm_Param.nBitPerSample	=	(OMX_U32) iAudioInputFormat.iInputBitsPerSample;
    Audio_Pcm_Param.nSamplingRate	=	(OMX_U32) iAudioInputFormat.iInputSamplingRate;
    Audio_Pcm_Param.ePCMMode		=	OMX_AUDIO_PCMModeLinear;
    // don't set - let use default Audio_Pcm_Param.eChannelMapping


    CONFIG_SIZE_AND_VERSION(Audio_Pcm_Param);

    Err = OMX_SetParameter(iOMXEncoder, OMX_IndexParamAudioPcm, &Audio_Pcm_Param);
    if (Err != OMX_ErrorNone)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::NegotiateAudioComponentParameters() Problem setting PCM parameters with input port %d ", iNodeTypeId, iInputPortIndex));
        return false;
    }

    //////////////////////// OUTPUT PORT////////////////////////////////////////////////

    // first of all, check if the port supports the adequate port format
    if ((iOutFormat == PVMF_MIME_AMR_IETF) ||
            (iOutFormat == PVMF_MIME_AMRWB_IETF) ||
            (iOutFormat == PVMF_MIME_AMR_IF2))
    {
        DesiredPortFormat = OMX_AUDIO_CodingAMR;
    }
    else if ((iOutFormat == PVMF_MIME_ADIF) ||
             (iOutFormat == PVMF_MIME_ADTS) ||
             (iOutFormat == PVMF_MIME_MPEG4_AUDIO))
    {
        DesiredPortFormat = OMX_AUDIO_CodingAAC;
    }
    else
    {
        DesiredPortFormat = OMX_AUDIO_CodingUnused;
    }

    if (DesiredPortFormat == OMX_AUDIO_CodingUnused)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::NegotiateAudioComponentParameters() Problem with output port %d format", iNodeTypeId, iOutputPortIndex));
        return false;
    }

    CONFIG_SIZE_AND_VERSION(Audio_port_format);

    Audio_port_format.nPortIndex = iOutputPortIndex; // set output port as target

    // loop over supported formats until we hit the one we want
    // or until the component has no more supported formats (in which case it returns OMX_ErrorNoMore
    Err = OMX_ErrorNone;
    Audio_port_format.nIndex = 0; //init the format counter
    while (OMX_ErrorNone == Err)
    {

        Err = OMX_GetParameter(iOMXEncoder, OMX_IndexParamAudioPortFormat, &Audio_port_format);
        if ((OMX_ErrorNone != Err))
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXEncNode-%s::NegotiateAudioComponentParameters() Problem getting port format for output port %d or no desired port format found", iNodeTypeId, iOutputPortIndex));
            return false;
        }

        if (Audio_port_format.eEncoding == DesiredPortFormat)
            break;

        Audio_port_format.nIndex ++;
    }

    // OK, we've found the desired format, set it as the one used
    Err = OMX_SetParameter(iOMXEncoder, OMX_IndexParamAudioPortFormat, &Audio_port_format);
    if ((OMX_ErrorNone != Err))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::NegotiateAudioComponentParameters() Problem setting port format for output port %d ", iNodeTypeId, iOutputPortIndex));
        return false;
    }


    //Port 1 for output port
    CONFIG_SIZE_AND_VERSION(iParamPort);
    iParamPort.nPortIndex = iOutputPortIndex;
    Err = OMX_GetParameter(iOMXEncoder, OMX_IndexParamPortDefinition, &iParamPort);
    if (Err != OMX_ErrorNone)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::NegotiateAudioComponentParameters() Problem negotiating with output port %d ", iNodeTypeId, iOutputPortIndex));
        return false;
    }

    // let the component decide output buffer size
    iOMXComponentOutputBufferSize = iParamPort.nBufferSize;

    // let the component decide num output buffers
    iNumOutputBuffers = iParamPort.nBufferCountActual;

    //check the number
    if (iNumOutputBuffers < iParamPort.nBufferCountMin)
        iNumOutputBuffers = iParamPort.nBufferCountMin;


    // set the number ourselves
    if (iOMXComponentSupportsExternalOutputBufferAlloc && (iParamPort.nBufferCountMin < NUMBER_OUTPUT_BUFFER))
    {
        iNumOutputBuffers = NUMBER_OUTPUT_BUFFER;
    }

    iParamPort.nBufferCountActual = iNumOutputBuffers;


    // set the output (target) format, etc.
    iParamPort.format.audio.bFlagErrorConcealment = OMX_TRUE;



    if ((iOutFormat == PVMF_MIME_AMR_IETF) ||
            (iOutFormat == PVMF_MIME_AMRWB_IETF) ||
            (iOutFormat == PVMF_MIME_AMR_IF2))
    {
        iParamPort.format.audio.eEncoding = OMX_AUDIO_CodingAMR;
    }
    else if ((iOutFormat == PVMF_MIME_ADTS) ||
             (iOutFormat == PVMF_MIME_ADIF) ||
             (iOutFormat == PVMF_MIME_MPEG4_AUDIO))
    {
        iParamPort.format.audio.eEncoding = OMX_AUDIO_CodingAAC;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXEncNode-%s::NegotiateAudioComponentParameters() Outport buffers %d,size %d", iNodeTypeId, iNumOutputBuffers, iOMXComponentOutputBufferSize));


    CONFIG_SIZE_AND_VERSION(iParamPort);
    Err = OMX_SetParameter(iOMXEncoder, OMX_IndexParamPortDefinition, &iParamPort);
    if (Err != OMX_ErrorNone)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::NegotiateAudioComponentParameters() Problem setting parameters in output port %d ", iNodeTypeId, iOutputPortIndex));
        return false;

    }

    // now call codec specific parameter setting
    bool status = true;
    if ((iOutFormat == PVMF_MIME_AMR_IETF) ||
            (iOutFormat == PVMF_MIME_AMRWB_IETF) ||
            (iOutFormat == PVMF_MIME_AMR_IF2))
    {

        status = SetAMREncoderParameters();
    }
    else if ((iOutFormat == PVMF_MIME_ADTS) ||
             (iOutFormat == PVMF_MIME_ADIF) ||
             (iOutFormat == PVMF_MIME_MPEG4_AUDIO))
    {
        status = SetAACEncoderParameters();
    }


    return status;
}

bool PVMFOMXEncNode::SetAMREncoderParameters()
{

    OMX_ERRORTYPE Err = OMX_ErrorNone;
    OMX_AUDIO_PARAM_AMRTYPE AmrType;

    CONFIG_SIZE_AND_VERSION(AmrType);
    AmrType.nPortIndex = iOutputPortIndex;

    Err = OMX_GetParameter(iOMXEncoder, OMX_IndexParamAudioAmr, &AmrType);
    if (Err != OMX_ErrorNone)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::SetAMREncoderParameters - Problem getting AMR parameters in output port %d ", iNodeTypeId, iOutputPortIndex));
        return false;
    }

    AmrType.nChannels = iAudioEncodeParam.iOutputNumChannels; // must be 1

    switch (iAudioEncodeParam.iAMRBitrate)
    {
        case GSM_AMR_4_75:  // AMR NB bitrates
            AmrType.eAMRBandMode = OMX_AUDIO_AMRBandModeNB0;
            break;
        case GSM_AMR_5_15:
            AmrType.eAMRBandMode = OMX_AUDIO_AMRBandModeNB1;
            break;
        case GSM_AMR_5_90:
            AmrType.eAMRBandMode = OMX_AUDIO_AMRBandModeNB2;
            break;
        case GSM_AMR_6_70:
            AmrType.eAMRBandMode = OMX_AUDIO_AMRBandModeNB3;
            break;
        case GSM_AMR_7_40:
            AmrType.eAMRBandMode = OMX_AUDIO_AMRBandModeNB4;
            break;
        case GSM_AMR_7_95:
            AmrType.eAMRBandMode = OMX_AUDIO_AMRBandModeNB5;
            break;
        case GSM_AMR_10_2:
            AmrType.eAMRBandMode = OMX_AUDIO_AMRBandModeNB6;
            break;
        case GSM_AMR_12_2:
            AmrType.eAMRBandMode = OMX_AUDIO_AMRBandModeNB7;
            break;
        case GSM_AMR_6_60: // AMR WB bitrates start here
            AmrType.eAMRBandMode = OMX_AUDIO_AMRBandModeWB0;
            break;
        case GSM_AMR_8_85:
            AmrType.eAMRBandMode = OMX_AUDIO_AMRBandModeWB1;
            break;
        case GSM_AMR_12_65:
            AmrType.eAMRBandMode = OMX_AUDIO_AMRBandModeWB2;
            break;
        case GSM_AMR_14_25:
            AmrType.eAMRBandMode = OMX_AUDIO_AMRBandModeWB3;
            break;
        case GSM_AMR_15_85:
            AmrType.eAMRBandMode = OMX_AUDIO_AMRBandModeWB4;
            break;
        case GSM_AMR_18_25:
            AmrType.eAMRBandMode = OMX_AUDIO_AMRBandModeWB5;
            break;
        case GSM_AMR_19_85:
            AmrType.eAMRBandMode = OMX_AUDIO_AMRBandModeWB6;
            break;
        case GSM_AMR_23_05:
            AmrType.eAMRBandMode = OMX_AUDIO_AMRBandModeWB7;
            break;
        case GSM_AMR_23_85:
            AmrType.eAMRBandMode = OMX_AUDIO_AMRBandModeWB8;
            break;
        default:
            return false;
    }

    AmrType.eAMRDTXMode = OMX_AUDIO_AMRDTXModeOnAuto;

    if ((iOutFormat == PVMF_MIME_AMR_IETF) || (iOutFormat == PVMF_MIME_AMRWB_IETF))
    {
        AmrType.eAMRFrameFormat = OMX_AUDIO_AMRFrameFormatFSF;
    }
    else if (iOutFormat == PVMF_MIME_AMR_IF2)
    {
        AmrType.eAMRFrameFormat = OMX_AUDIO_AMRFrameFormatIF2;
    }

    CONFIG_SIZE_AND_VERSION(AmrType);
    AmrType.nPortIndex = iOutputPortIndex;

    Err = OMX_SetParameter(iOMXEncoder, OMX_IndexParamAudioAmr, &AmrType);
    if (Err != OMX_ErrorNone)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::SetAMREncoderParameters - Problem setting AMR parameters in output port %d ", iNodeTypeId, iOutputPortIndex));
        return false;
    }

    return true;
}

bool PVMFOMXEncNode::SetAACEncoderParameters()
{

    OMX_ERRORTYPE Err = OMX_ErrorNone;
    OMX_AUDIO_PARAM_AACPROFILETYPE AacType;


    CONFIG_SIZE_AND_VERSION(AacType);
    AacType.nPortIndex = iOutputPortIndex;

    Err = OMX_GetParameter(iOMXEncoder, OMX_IndexParamAudioAac, &AacType);
    if (Err != OMX_ErrorNone)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::SetAACEncoderParameters - Problem getting AAC parameters in output port %d ", iNodeTypeId, iOutputPortIndex));
        return false;
    }

    AacType.nChannels = iAudioEncodeParam.iOutputNumChannels;
    // The following parameter could be set to input sampling rate or 0 (i.e. unknown)
    AacType.nSampleRate = iAudioEncodeParam.iOutputSamplingRate;
    // The following parameter could be set to 0 (i.e. unknown)
    AacType.nBitRate = iAudioEncodeParam.iOutputBitrate;

    // Let encoder decide the following parameters - i.e. set to 0
    AacType.nAudioBandWidth =   0;
    AacType.nFrameLength    =   0;
    AacType.nAACtools =         OMX_AUDIO_AACToolAll; // this means all tools are allowed - let encoder decide
    AacType.nAACERtools =       OMX_AUDIO_AACERNone;   // error resilience tools are not allowed

    // Set the AAC profile to use LC
    AacType.eAACProfile = OMX_AUDIO_AACObjectLC;

    // Do set the stream format
    if (iOutFormat == PVMF_MIME_ADTS)
    {
        AacType.eAACStreamFormat = OMX_AUDIO_AACStreamFormatMP4ADTS;
    }
    else if (iOutFormat == PVMF_MIME_ADIF)
    {
        AacType.eAACStreamFormat = OMX_AUDIO_AACStreamFormatADIF;
    }
    else if (iOutFormat == PVMF_MIME_MPEG4_AUDIO)
    {
        AacType.eAACStreamFormat = OMX_AUDIO_AACStreamFormatMP4FF;
    }

    // Set the AAC channel mode - (stereo, or mono)
    if (iAudioEncodeParam.iOutputNumChannels == 1)
    {
        AacType.eChannelMode = OMX_AUDIO_ChannelModeMono;
    }
    else if (iAudioEncodeParam.iOutputNumChannels == 2)
    {
        AacType.eChannelMode = OMX_AUDIO_ChannelModeStereo;
    }

    CONFIG_SIZE_AND_VERSION(AacType);
    AacType.nPortIndex = iOutputPortIndex;

    Err = OMX_SetParameter(iOMXEncoder, OMX_IndexParamAudioAac, &AacType);
    if (Err != OMX_ErrorNone)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::SetAACEncoderParameters - Problem setting AAC parameters in output port %d ", iNodeTypeId, iOutputPortIndex));
        return false;
    }

    return true;
}

bool PVMFOMXEncNode::SetDefaultCapabilityFlags()
{

    iIsOMXComponentMultiThreaded = true;

    iOMXComponentSupportsExternalOutputBufferAlloc = false;
    iOMXComponentSupportsExternalInputBufferAlloc = false;
    iOMXComponentSupportsMovableInputBuffers = false;

    iOMXComponentUsesNALStartCodes = true;
    iOMXComponentSupportsPartialFrames = false;
    iOMXComponentCanHandleIncompleteFrames = false;
    iOMXComponentUsesFullAVCFrames = false;

    return true;
}



bool PVMFOMXEncNode::SendEOSBufferToOMXComponent()
{

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXEncNode-%s::SendEOSBufferToOMXComponent() In", iNodeTypeId));


    // first of all, check if the component is running. EOS could be sent prior to component/encoder
    // even being initialized

    // returning false will ensure that the EOS will be sent downstream anyway without waiting for the
    // Component to respond
    if (iCurrentEncoderState != OMX_StateExecuting)
        return false;

    // get an input buffer. Without a buffer, no point in proceeding
    InputBufCtrlStruct *input_buf = NULL;
    int32 errcode = 0;

    // we already checked that the number of buffers is OK, so we don't expect problems
    // try to get input buffer header
    OSCL_TRY(errcode, input_buf = (InputBufCtrlStruct *) iInBufMemoryPool->allocate(iInputAllocSize));
    if (errcode != 0)
    {
        if (errcode == OsclErrNoResources)
        {

            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger,
                            PVLOGMSG_DEBUG, (0, "PVMFOMXEncNode-%s::SendEOSBufferToOMXComponent() No more buffers in the mempool - unexpected", iNodeTypeId));

            iInBufMemoryPool->notifyfreechunkavailable(*this, (OsclAny*) iInBufMemoryPool); // To signal when next deallocate() is called on mempool

            return false;
        }
        else
        {
            // Memory allocation for the pool failed
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFOMXEncNode-%s::SendEOSBufferToOMXComponent() Input mempool error", iNodeTypeId));


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
    OMX_EmptyThisBuffer(iOMXEncoder, input_buf->pBufHdr);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXEncNode-%s::SendEOSBufferToOMXComponent() Out", iNodeTypeId));

    return true;

}


bool PVMFOMXEncNode::SendInputBufferToOMXComponent()
{

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXEncNode-%s::SendInputBufferToOMXComponent() In", iNodeTypeId));

    // first of all , get an input buffer. Without a buffer, no point in proceeding
    InputBufCtrlStruct *input_buf = NULL;
    int32 errcode = 0;
    uint32 ii;

    do
    {
        // do loop to loop over all fragments

        // try to get input buffer header
        InputBufCtrlStruct *temp = NULL;
        OSCL_TRY(errcode, temp = (InputBufCtrlStruct *) iInBufMemoryPool->allocate(iInputAllocSize));
        input_buf = temp;
        if (errcode != 0)
        {
            if (errcode == OsclErrNoResources)
            {

                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger,
                                PVLOGMSG_DEBUG, (0, "PVMFOMXEncNode-%s::SendInputBufferToOMXComponent() No more buffers in the mempool", iNodeTypeId));

                iInBufMemoryPool->notifyfreechunkavailable(*this, (OsclAny*) iInBufMemoryPool); // To signal when next deallocate() is called on mempool

                return false;
            }
            else
            {
                // Memory allocation for the pool failed
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMFOMXEncNode-%s::SendInputBufferToOMXComponent() Input mempool error", iNodeTypeId));


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



            iCurrentMsgMarkerBit = iDataIn->getMarkerInfo() & PVMF_MEDIA_DATA_MARKER_INFO_M_BIT;


            // logging info:
            if (iDataIn->getNumFragments() > 1)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "PVMFOMXEncNode-%s::SendInputBufferToOMXComponent() - New msg has MULTI-FRAGMENTS", iNodeTypeId));
            }

            if (!(iDataIn->getMarkerInfo() & PVMF_MEDIA_DATA_MARKER_INFO_M_BIT))
            {

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "PVMFOMXEncNode-%s::SendInputBufferToOMXComponent() - New msg has NO MARKER BIT", iNodeTypeId));
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
                            (0, "PVMFOMXEncNode-%s::SendInputBufferToOMXComponent() - Buffer 0x%x of size %d, %d frag out of tot. %d, TS=%d", iNodeTypeId, input_buf->pBufHdr->pBuffer, frag.getMemFragSize(), iCurrFragNum + 1, iDataIn->getNumFragments(), iInTimestamp));

            iCurrFragNum++; // increment fragment number and move on to the next


        }
        else
        {

            // in this case, no need to use input msg refcounter, each buffer fragment is copied over and treated separately
            (input_buf->pMediaData).Unbind();


            // init variables
            iCopyPosition = 0;
            iFragmentSizeRemainingToCopy  = frag.getMemFragSize();


            // can the remaining fragment fit into the buffer?
            if (iFragmentSizeRemainingToCopy <= (input_buf->pBufHdr->nAllocLen))
            {

                oscl_memcpy(input_buf->pBufHdr->pBuffer,
                            (void *)((uint8 *)frag.getMemFragPtr() + iCopyPosition),
                            iFragmentSizeRemainingToCopy);

                input_buf->pBufHdr->nFilledLen = iFragmentSizeRemainingToCopy;

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "PVMFOMXEncNode-%s::SendInputBufferToOMXComponent() - Copied %d bytes of fragment %d out of %d into buffer 0x%x of size %d, TS=%d ", iNodeTypeId, iFragmentSizeRemainingToCopy, iCurrFragNum + 1, iDataIn->getNumFragments(), input_buf->pBufHdr->pBuffer, input_buf->pBufHdr->nFilledLen, iInTimestamp));

                iCopyPosition += iFragmentSizeRemainingToCopy;
                iFragmentSizeRemainingToCopy = 0;



            }
            else
            {
                // copy as much as you can of the current fragment into the current buffer
                oscl_memcpy(input_buf->pBufHdr->pBuffer,
                            (void *)((uint8 *)frag.getMemFragPtr() + iCopyPosition),
                            input_buf->pBufHdr->nAllocLen);

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "PVMFOMXEncNode-%s::SendInputBufferToOMXComponent() - Frame cannot fit into input buffer ! Copied %d bytes of fragment %d out of %d into buffer 0x%x of size %d, TS=%d", iNodeTypeId, input_buf->pBufHdr->nAllocLen, iCurrFragNum + 1, iDataIn->getNumFragments(), input_buf->pBufHdr->pBuffer, input_buf->pBufHdr->nFilledLen, iInTimestamp));

                input_buf->pBufHdr->nFilledLen = input_buf->pBufHdr->nAllocLen;
                iCopyPosition += input_buf->pBufHdr->nAllocLen; // move current position within fragment forward
                iFragmentSizeRemainingToCopy -= input_buf->pBufHdr->nAllocLen;

            }

            // proceed to the next fragment regardless of input buffer size
            iCurrFragNum++;
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


        // set the key frame flag if necessary (mark every fragment that belongs to it)
        if (iDataIn->getMarkerInfo() & PVMF_MEDIA_DATA_MARKER_INFO_RANDOM_ACCESS_POINT_BIT)
            input_buf->pBufHdr->nFlags |= OMX_BUFFERFLAG_SYNCFRAME;


        // in case of encoder, all input frames should be marked
        input_buf->pBufHdr->nFlags |= OMX_BUFFERFLAG_ENDOFFRAME;
        OMX_EmptyThisBuffer(iOMXEncoder, input_buf->pBufHdr);


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
                    (0, "PVMFOMXEncNode-%s::SendInputBufferToOMXComponent() Out", iNodeTypeId));

    return true;

}



/////////////////////////////////////////////////////////////////////////////
bool PVMFOMXEncNode::CreateOutMemPool(uint32 num_buffers)
{

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXEncNode-%s::CreateOutMemPool() start", iNodeTypeId));
    // In the case OMX component wants to allocate its own buffers,
    // mempool only contains OutputBufCtrlStructures (i.e. ptrs to buffer headers)
    // In case OMX component uses pre-allocated buffers (here),
    // mempool allocates OutputBufCtrlStructure (i.e. ptrs to buffer hdrs), followed by actual buffers

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXEncNode-%s::CreateOutMemPool() Allocating output buffer header pointers", iNodeTypeId));

    iOutputAllocSize = oscl_mem_aligned_size((uint32)sizeof(OutputBufCtrlStruct));

    if (iOMXComponentSupportsExternalOutputBufferAlloc)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::CreateOutMemPool() Allocating output buffers of size %d as well", iNodeTypeId, iOMXComponentOutputBufferSize));

        //pre-negotiated output buffer size
        iOutputAllocSize += iOMXComponentOutputBufferSize;
    }

    // ENCODER SPECIFIC FOR AVC RAW (BYTESTREAM) FORMAT
    if ((iOutFormat == PVMF_MIME_H264_VIDEO_RAW) &&
            (!iOMXComponentUsesFullAVCFrames) &&
            (!iOMXComponentUsesNALStartCodes))
    {
        iOutputAllocSize += 4; // NAL SYNC WORD SIZE
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

    int32 leavecode = 0;
    OSCL_TRY(leavecode, iOutBufMemoryPool = OSCL_NEW(OsclMemPoolFixedChunkAllocator, (num_buffers)););
    if (leavecode || iOutBufMemoryPool == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger,
                        PVLOGMSG_ERR, (0, "PVMFOMXEncNode-%s::CreateOutMemPool() Memory pool structure for output buffers failed to allocate", iNodeTypeId));
        return false;
    }



    // allocate a dummy buffer to actually create the mempool
    OsclAny *dummy_alloc = NULL; // this dummy buffer will be released at end of scope
    leavecode = 0;
    OSCL_TRY(leavecode, dummy_alloc = iOutBufMemoryPool->allocate(iOutputAllocSize));
    if (leavecode || dummy_alloc == NULL)
    {

        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger,
                        PVLOGMSG_ERR, (0, "PVMFOMXEncNode-%s::CreateOutMemPool() Memory pool for output buffers failed to allocate", iNodeTypeId));
        return false;
    }
    iOutBufMemoryPool->deallocate(dummy_alloc);
    // init the counter
    iNumOutstandingOutputBuffers = 0;

    // allocate mempool for media data message wrapper
    leavecode = 0;
    OSCL_TRY(leavecode, iMediaDataMemPool = OSCL_NEW(OsclMemPoolFixedChunkAllocator, (num_buffers, PVOMXENC_MEDIADATA_CHUNKSIZE)));
    if (leavecode || iMediaDataMemPool == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncNode-%s::CreateOutMemPool() Media Data Buffer pool for output buffers failed to allocate", iNodeTypeId));
        return false;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::CreateOutMemPool() done", iNodeTypeId));
    return true;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// Creates memory pool for input buffer management ///////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool PVMFOMXEncNode::CreateInputMemPool(uint32 num_buffers)
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
                    (0, "PVMFOMXEncNode-%s::CreateInputMemPool() start ", iNodeTypeId));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXEncNode-%s::CreateInputMemPool() allocating buffer header pointers and shared media data ptrs ", iNodeTypeId));



    iInputAllocSize = oscl_mem_aligned_size((uint32) sizeof(InputBufCtrlStruct)); //aligned_size_buffer_header_ptr+aligned_size_media_data_ptr;

    // Need to allocate buffers in the node either if component supports external buffers buffers
    // but they are not movable

    if ((iOMXComponentSupportsExternalInputBufferAlloc && !iOMXComponentSupportsMovableInputBuffers))
    {
        //pre-negotiated input buffer size
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::CreateOutMemPool() Allocating input buffers of size %d as well", iNodeTypeId, iOMXComponentInputBufferSize));

        iInputAllocSize += iOMXComponentInputBufferSize;
    }

    if (iInBufMemoryPool)
    {
        iInBufMemoryPool->removeRef();
        iInBufMemoryPool = NULL;
    }

    int32 leavecode = 0;
    OSCL_TRY(leavecode, iInBufMemoryPool = OSCL_NEW(OsclMemPoolFixedChunkAllocator, (num_buffers)););
    if (leavecode || iInBufMemoryPool == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger,
                        PVLOGMSG_ERR, (0, "PVMFOMXEncNode-%s::CreateInputMemPool() Memory pool structure for input buffers failed to allocate", iNodeTypeId));
        return false;
    }
    // try to allocate a dummy buffer to actually create the mempool and allocate the needed memory
    // allocate a dummy buffer to actually create the mempool, this dummy buffer will be released at end of scope of this method
    OsclAny *dummy_alloc = NULL;
    leavecode = 0;
    OSCL_TRY(leavecode, dummy_alloc = iInBufMemoryPool->allocate(iInputAllocSize));
    if (leavecode || dummy_alloc == NULL)
    {

        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger,
                        PVLOGMSG_ERR, (0, "PVMFOMXEncNode-%s::CreateInputMemPool() Memory pool for input buffers failed to allocate", iNodeTypeId));
        return false;
    }

    // init the counter
    iNumOutstandingInputBuffers = 0;


    iInputBufferToResendToComponent = NULL; // nothing to resend yet
    iInBufMemoryPool->deallocate(dummy_alloc);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::CreateInputMemPool() done", iNodeTypeId));
    return true;
}
////////////////////////////////////////////////////////////////////////////
bool PVMFOMXEncNode::ProvideBuffersToComponent(OsclMemPoolFixedChunkAllocator *aMemPool, // allocator
        uint32 aAllocSize,	 // size to allocate from pool (hdr only or hdr+ buffer)
        uint32 aNumBuffers,    // number of buffers
        uint32 aActualBufferSize, // aactual buffer size
        uint32 aPortIndex,      // port idx
        bool aUseBufferOK,		// can component use OMX_UseBuffer or should it use OMX_AllocateBuffer
        bool	aIsThisInputBuffer		// is this input or output
                                              )
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::ProvideBuffersToComponent() enter", iNodeTypeId));

    uint32 ii = 0;
    OMX_ERRORTYPE err = OMX_ErrorNone;
    OsclAny **ctrl_struct_ptr = NULL;	// temporary array to keep the addresses of buffer ctrl structures and buffers

    ctrl_struct_ptr = (OsclAny **) oscl_malloc(aNumBuffers * sizeof(OsclAny *));
    if (ctrl_struct_ptr == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::ProvideBuffersToComponent ctrl_struct_ptr == NULL", iNodeTypeId));
        return false;
    }


    // Now, go through all buffers and tell component to
    // either use a buffer, or to allocate its own buffer
    for (ii = 0; ii < aNumBuffers; ii++)
    {

        int32 errcode = 0;
        // get the address where the buf hdr ptr will be stored
        errcode = MemAllocate(ctrl_struct_ptr[ii], aMemPool, aAllocSize);
        if ((errcode != OsclErrNone) || (ctrl_struct_ptr[ii] == NULL))
        {
            if (errcode == OsclErrNoResources)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "PVMFOMXEncNode-%s::ProvideBuffersToComponent ->allocate() failed for no mempool chunk available", iNodeTypeId));
            }
            else
            {
                // General error
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMFOMXEncNode-%s::ProvideBuffersToComponent ->allocate() failed due to some general error", iNodeTypeId));

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

                err = OMX_UseBuffer(iOMXEncoder,	// hComponent
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
                OutputBufCtrlStruct *temp = (OutputBufCtrlStruct *) ctrl_struct_ptr[ii];
                // advance buffer ptr to skip the structure
                pB += oscl_mem_aligned_size(sizeof(OutputBufCtrlStruct));

                if ((iOutFormat == PVMF_MIME_H264_VIDEO_RAW) &&
                        !iOMXComponentUsesFullAVCFrames &&
                        !iOMXComponentUsesNALStartCodes)
                {
                    // write out NAL sync word at the beginning of the buffer
                    pB[0] = 0;
                    pB[1] = 0;
                    pB[2] = 0;
                    pB[3] = 1;
                    pB += 4;

                    // THe buffer that the component knows is always the same
                    // The node will move the ptr -4 when it needs teh sync word
                }

                err = OMX_UseBuffer(iOMXEncoder,	// hComponent
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
        else
        {
            // the component must allocate its own buffers.
            if (aIsThisInputBuffer)
            {

                InputBufCtrlStruct *temp = (InputBufCtrlStruct *) ctrl_struct_ptr[ii];
                err = OMX_AllocateBuffer(iOMXEncoder,
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
                err = OMX_AllocateBuffer(iOMXEncoder,
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
                            (0, "PVMFOMXEncNode-%s::ProvideBuffersToComponent() Problem using/allocating a buffer", iNodeTypeId));


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

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::ProvideBuffersToComponent() done", iNodeTypeId));
    return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool PVMFOMXEncNode::FreeBuffersFromComponent(OsclMemPoolFixedChunkAllocator *aMemPool, // allocator
        uint32 aAllocSize,	 // size to allocate from pool (hdr only or hdr+ buffer)
        uint32 aNumBuffers,    // number of buffers
        uint32 aPortIndex,      // port idx
        bool	aIsThisInputBuffer		// is this input or output
                                             )
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::FreeBuffersToComponent() enter", iNodeTypeId));

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

        int32 errcode = 0;
        // get the address where the buf hdr ptr will be stored

        errcode = MemAllocate(ctrl_struct_ptr[ii], aMemPool, aAllocSize);
        if ((errcode != OsclErrNone) || (ctrl_struct_ptr[ii] == NULL))
        {
            if (errcode == OsclErrNoResources)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "PVMFOMXEncNode-%s::FreeBuffersFromComponent ->allocate() failed for no mempool chunk available", iNodeTypeId));
            }
            else
            {
                // General error
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMFOMXEncNode-%s::FreeBuffersFromComponent ->allocate() failed due to some general error", iNodeTypeId));

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
            err = OMX_FreeBuffer(iOMXEncoder,
                                 aPortIndex,
                                 temp->pBufHdr);

        }
        else
        {
            iNumOutstandingOutputBuffers++;
            OutputBufCtrlStruct *temp = (OutputBufCtrlStruct *) ctrl_struct_ptr[ii];
            err = OMX_FreeBuffer(iOMXEncoder,
                                 aPortIndex,
                                 temp->pBufHdr);

        }

        if (err != OMX_ErrorNone)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFOMXEncNode-%s::FreeBuffersFromComponent() Problem freeing a buffer", iNodeTypeId));

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
    }
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::FreeBuffersFromComponent() done", iNodeTypeId));
    return true;
}



/////////////////////////////////////////////////////////////////////////////
////////////////////// CALLBACK PROCESSING FOR EVENT HANDLER
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE PVMFOMXEncNode::EventHandlerProcessing(OMX_OUT OMX_HANDLETYPE aComponent,
        OMX_OUT OMX_PTR aAppData,
        OMX_OUT OMX_EVENTTYPE aEvent,
        OMX_OUT OMX_U32 aData1,
        OMX_OUT OMX_U32 aData2,
        OMX_OUT OMX_PTR aEventData)
{
    OSCL_UNUSED_ARG(aComponent);
    OSCL_UNUSED_ARG(aAppData);
    OSCL_UNUSED_ARG(aEventData);

    switch (aEvent)
    {
        case OMX_EventCmdComplete:
        {

            switch (aData1)
            {
                case OMX_CommandStateSet:
                {
                    HandleComponentStateChange(aData2);
                    break;
                }
                case OMX_CommandFlush:            //not supported
                    // nothing to do here yet
                    break;

                case OMX_CommandPortDisable:
                {
                    // if port disable command is done, we can re-allocate the buffers and re-enable the port

                    iProcessingState = EPVMFOMXEncNodeProcessingState_PortReEnable;
                    iPortIndexForDynamicReconfig =  aData2;

                    RunIfNotReady();
                    break;
                }
                case OMX_CommandPortEnable:
                    // port enable command is done. Check if the other port also reported change.
                    // If not, we can start data flow. Otherwise, must start dynamic reconfig procedure for
                    // the other port as well.
                {
                    if (iSecondPortReportedChange)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                        (0, "PVMFOMXEncNode-%s::EventHandlerProcessing: OMX_CommandPortEnable - completed on port %d, dynamic reconfiguration needed on port %d", iNodeTypeId, aData2, iSecondPortToReconfig));

                        iProcessingState = EPVMFOMXEncNodeProcessingState_PortReconfig;
                        iPortIndexForDynamicReconfig = iSecondPortToReconfig;
                        iSecondPortReportedChange = false;
                    }
                    else
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                        (0, "PVMFOMXEncNode-%s::EventHandlerProcessing: OMX_CommandPortEnable - completed on port %d, resuming normal data flow", iNodeTypeId, aData2));
                        iProcessingState = EPVMFOMXEncNodeProcessingState_ReadyToEncode;
                        iDynamicReconfigInProgress = false;
                    }
                    RunIfNotReady();
                    break;
                }

                case OMX_CommandMarkBuffer:
                    // nothing to do here yet;
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "PVMFOMXEncNode-%s::EventHandlerProcessing: OMX_CommandMarkBuffer - completed - no action taken", iNodeTypeId));

                    break;

                default:
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "PVMFOMXEncNode-%s::EventHandlerProcessing: Unsupported event", iNodeTypeId));
                    break;
                }
            }//end of switch (aData1)

            break;
        }//end of case OMX_EventCmdComplete

        case OMX_EventError:
        {

            if (aData1 == (OMX_U32) OMX_ErrorStreamCorrupt)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMFOMXEncNode-%s::EventHandlerProcessing: OMX_EventError - Bitstream corrupt error", iNodeTypeId));
                // Errors from corrupt bitstream are reported as info events
                ReportInfoEvent(PVMFInfoProcessingFailure, NULL);

            }
            else
            {

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMFOMXEncNode-%s::EventHandlerProcessing: OMX_EventError", iNodeTypeId));
                // for now, any error from the component will be reported as error
                ReportErrorEvent(PVMFErrorEvent, NULL, NULL);
                SetState(EPVMFNodeError);
            }
            break;

        }

        case OMX_EventBufferFlag:
        {
            // the component is reporting it encountered end of stream flag
            // we'll send eos when we get the actual last buffer with marked eos

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXEncNode-%s::EventHandlerProcessing: OMX_EventBufferFlag (EOS) flag returned from OMX component", iNodeTypeId));

            RunIfNotReady();
            break;
        }//end of case OMX_EventBufferFlag

        case OMX_EventMark:
        {

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXEncNode-%s::EventHandlerProcessing: OMX_EventMark returned from OMX component - no action taken", iNodeTypeId));

            RunIfNotReady();
            break;
        }//end of case OMX_EventMark

        case OMX_EventPortSettingsChanged:
        {

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXEncNode-%s::EventHandlerProcessing: OMX_EventPortSettingsChanged returned from OMX component", iNodeTypeId));

            // first check if dynamic reconfiguration is already in progress,
            // if so, wait until this is completed, and then initiate the 2nd reconfiguration
            if (iDynamicReconfigInProgress)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "PVMFOMXEncNode-%s::EventHandlerProcessing: OMX_EventPortSettingsChanged returned for port %d, dynamic reconfig already in progress", iNodeTypeId, aData1));

                iSecondPortToReconfig = aData1;
                iSecondPortReportedChange = true;
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "PVMFOMXEncNode-%s::EventHandlerProcessing: OMX_EventPortSettingsChanged returned for port %d", iNodeTypeId, aData1));

                iProcessingState = EPVMFOMXEncNodeProcessingState_PortReconfig;
                iPortIndexForDynamicReconfig = aData1;
                iDynamicReconfigInProgress = true;
            }

            RunIfNotReady();
            break;
        }//end of case OMX_PortSettingsChanged

        case OMX_EventResourcesAcquired:        //not supported
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXEncNode-%s::EventHandlerProcessing: OMX_EventResourcesAcquired returned from OMX component - no action taken", iNodeTypeId));

            RunIfNotReady();

            break;
        }

        default:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXEncNode-%s::EventHandlerProcessing:  Unknown Event returned from OMX component - no action taken", iNodeTypeId));

            break;
        }

    }//end of switch (eEvent)



    return OMX_ErrorNone;
}




/////////////////////////////////////////////////////////////////////////////
// This function handles the event of OMX component state change
/////////////////////////////////////////////////////////////////////////////
void PVMFOMXEncNode::HandleComponentStateChange(OMX_U32 encoder_state)
{
    switch (encoder_state)
    {
        case OMX_StateIdle:
        {
            iCurrentEncoderState = OMX_StateIdle;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXEncNode-%s::HandleComponentStateChange: OMX_StateIdle reached", iNodeTypeId));

            //  this state can be reached either going from OMX_Loaded->OMX_Idle (preparing)
            //	or going from OMX_Executing->OMX_Idle (stopping)


            if ((iCurrentCommand.size() > 0) &&
                    (iCurrentCommand.front().iCmd == PVMFOMXEncNodeCommand::PVOMXENC_NODE_CMD_PREPARE))
            {
                iProcessingState = EPVMFOMXEncNodeProcessingState_ReadyToEncode;
                SetState(EPVMFNodePrepared);
                CommandComplete(iCurrentCommand, iCurrentCommand.front(), PVMFSuccess);
                RunIfNotReady();
            }
            else if ((iCurrentCommand.size() > 0) &&
                     (iCurrentCommand.front().iCmd == PVMFOMXEncNodeCommand::PVOMXENC_NODE_CMD_STOP))
            {
                // if we are stopped, we won't start until the node gets DoStart command.
                //	in this case, we are ready to start sending buffers
                if (iProcessingState == EPVMFOMXEncNodeProcessingState_Stopping)
                    iProcessingState = EPVMFOMXEncNodeProcessingState_ReadyToEncode;
                // if the processing state was not stopping, leave the state as it was (continue port reconfiguration)
                SetState(EPVMFNodePrepared);
                CommandComplete(iCurrentCommand, iCurrentCommand.front(), PVMFSuccess);

                RunIfNotReady();
            }
            else if ((iCurrentCommand.size() > 0) &&
                     (iCurrentCommand.front().iCmd == PVMFOMXEncNodeCommand::PVOMXENC_NODE_CMD_FLUSH))
            {
                // if there is a flush, similar to the case of stop, we won't start until the node gets
                // DoStart command. In this case, we'll be ready
                if (iProcessingState == EPVMFOMXEncNodeProcessingState_Stopping)
                    iProcessingState = EPVMFOMXEncNodeProcessingState_ReadyToEncode;



                //Flush is complete.  Go to prepared state.
                SetState(EPVMFNodePrepared);
                //resume port input (if possible) so the ports can be re-started.
                if (iInPort)
                {
                    iInPort->ResumeInput();
                }
                if (iOutPort)
                {
                    iOutPort->ResumeInput();
                }
                CommandComplete(iCurrentCommand, iCurrentCommand.front(), PVMFSuccess);
                RunIfNotReady();

            }
            else if ((iCurrentCommand.size() > 0) &&
                     (iCurrentCommand.front().iCmd == PVMFOMXEncNodeCommand::PVOMXENC_NODE_CMD_RESET))
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
            iCurrentEncoderState = OMX_StateExecuting;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXEncNode-%s::HandleComponentStateChange: OMX_StateExecuting reached", iNodeTypeId));

            // this state can be reached going from OMX_Idle -> OMX_Executing (preparing)
            //	or going from OMX_Pause -> OMX_Executing (coming from pause)
            //	either way, this is a response to "DoStart" command

            if ((iCurrentCommand.size() > 0) &&
                    (iCurrentCommand.front().iCmd == PVMFOMXEncNodeCommand::PVOMXENC_NODE_CMD_START))
            {
                SetState(EPVMFNodeStarted);
                CommandComplete(iCurrentCommand, iCurrentCommand.front(), PVMFSuccess);

                RunIfNotReady();
            }

            break;
        }//end of case OMX_StateExecuting

        case OMX_StatePause:
        {
            iCurrentEncoderState = OMX_StatePause;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXEncNode-%s::HandleComponentStateChange: OMX_StatePause reached", iNodeTypeId));


            //	This state can be reached going from OMX_Executing-> OMX_Pause
            if ((iCurrentCommand.size() > 0) &&
                    (iCurrentCommand.front().iCmd == PVMFOMXEncNodeCommand::PVOMXENC_NODE_CMD_PAUSE))
            {
                SetState(EPVMFNodePaused);
                CommandComplete(iCurrentCommand, iCurrentCommand.front(), PVMFSuccess);
                RunIfNotReady();
            }

            break;
        }//end of case OMX_StatePause

        case OMX_StateLoaded:
        {
            iCurrentEncoderState = OMX_StateLoaded;

            //  this state can be reached only going from OMX_Idle ->OMX_Loaded (stopped to reset)
            //

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXEncNode-%s::HandleComponentStateChange: OMX_StateLoaded reached", iNodeTypeId));
            //Check if command's responce is pending
            if ((iCurrentCommand.size() > 0) &&
                    (iCurrentCommand.front().iCmd == PVMFOMXEncNodeCommand::PVOMXENC_NODE_CMD_RESET))
            {

                // move this here
                if (iInPort)
                {
                    OSCL_DELETE(((PVMFOMXEncPort*)iInPort));
                    iInPort = NULL;
                }

                if (iOutPort)
                {
                    OSCL_DELETE(((PVMFOMXEncPort*)iOutPort));
                    iOutPort = NULL;
                }

                iDataIn.Unbind();

                // Reset the metadata key list
                iAvailableMetadataKeys.clear();


                iProcessingState = EPVMFOMXEncNodeProcessingState_Idle;
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
            iCurrentEncoderState = OMX_StateInvalid;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFOMXEncNode-%s::HandleComponentStateChange: OMX_StateInvalid reached", iNodeTypeId));

            break;
        }//end of case OMX_StateInvalid

    }//end of switch(encoder_state)

}






/////////////////////////////////////////////////////////////////////////////
////////////////////// CALLBACK PROCESSING FOR EMPTY BUFFER DONE - input buffer was consumed
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE PVMFOMXEncNode::EmptyBufferDoneProcessing(OMX_OUT OMX_HANDLETYPE aComponent,
        OMX_OUT OMX_PTR aAppData,
        OMX_OUT OMX_BUFFERHEADERTYPE* aBuffer)
{

    OSCL_UNUSED_ARG(aComponent);
    OSCL_UNUSED_ARG(aAppData);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXEncNode-%s::EmptyBufferDoneProcessing: In", iNodeTypeId));

    OSCL_ASSERT((void*) aComponent == (void*) iOMXEncoder); // component should match the component
    OSCL_ASSERT(aAppData == (OMX_PTR)(this));		// AppData should represent this node ptr

    // first, get the buffer "context", i.e. pointer to application private data that contains the
    // address of the mempool buffer (so that it can be released)
    InputBufCtrlStruct *pContext = (InputBufCtrlStruct *)(aBuffer->pAppPrivate);



    // if a buffer is not empty, log a msg, but release anyway
    if ((aBuffer->nFilledLen > 0) && (iDoNotSaveInputBuffersFlag == false))
        // if dynamic port reconfig is in progress for input port, don't keep the buffer
    {

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::EmptyBufferDoneProcessing: Input buffer returned non-empty with %d bytes still in it", iNodeTypeId, aBuffer->nFilledLen));


    }

    iInputBufferToResendToComponent = NULL;


    // input buffer is to be released,
    // refcount needs to be decremented (possibly - the input msg associated with the buffer will be unbound)
    // NOTE: in case of "moveable" input buffers (passed into component without copying), unbinding decrements a refcount which eventually results
    //			in input message being released back to upstream mempool once all its fragments are returned
    //		in case of input buffers passed into component by copying, unbinding has no effect
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXEncNode-%s::EmptyBufferDoneProcessing: Release input buffer (with %d refcount remaining of input message)", iNodeTypeId, (pContext->pMediaData).get_count() - 1));


    (pContext->pMediaData).Unbind();


    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXEncNode-%s::EmptyBufferDoneProcessing: Release input buffer %x back to mempool", iNodeTypeId, pContext));

    iInBufMemoryPool->deallocate((OsclAny *) pContext);


    // the OMX spec says that no error is to be returned
    return OMX_ErrorNone;

}



/////////////////////////////////////////////////////////////////////////////
////////////////////// CALLBACK PROCESSING FOR FILL BUFFER DONE - output buffer is ready
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE PVMFOMXEncNode::FillBufferDoneProcessing(OMX_OUT OMX_HANDLETYPE aComponent,
        OMX_OUT OMX_PTR aAppData,
        OMX_OUT OMX_BUFFERHEADERTYPE* aBuffer)
{

    OSCL_UNUSED_ARG(aComponent);
    OSCL_UNUSED_ARG(aAppData);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXEncNode-%s::FillBufferDoneProcessing: In", iNodeTypeId));

    OSCL_ASSERT((void*) aComponent == (void*) iOMXEncoder); // component should match the component
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
                        (0, "PVMFOMXEncNode-%s::FillBufferDoneProcessing: Output buffer has EOS set", iNodeTypeId));

    }

    if (iOMXComponentUsesNALStartCodes)
    {
        // find size of first start code, since it can be either 3 or 4 bytes
        uint8* pData = (uint8*)aBuffer->pBuffer + aBuffer->nOffset;
        uint8* pTemp;
        uint32 size = aBuffer->nFilledLen;
        if (AVCAnnexBGetNALUnit(pData, &pTemp, (int32*)&size, true))
        {
            iFirstNALStartCodeSize = (uint32)pTemp - (uint32)pData;
        }
        else
        {
            iFirstNALStartCodeSize = 0;
        }
    }

    /* the case in which a buffer simply containing a start code is sent */
    if (aBuffer->nFilledLen <= iFirstNALStartCodeSize && iOMXComponentUsesNALStartCodes)
    {
        aBuffer->nFilledLen = 0;
    }

    // if a buffer is empty, or if it should not be sent downstream (say, due to state change)
    // release the buffer back to the pool
    if ((aBuffer->nFilledLen == 0) || (iDoNotSendOutputBuffersDownstreamFlag == true))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::FillBufferDoneProcessing: Release output buffer %x back to mempool - buffer empty or not to be sent downstream", iNodeTypeId, pContext));

        iOutBufMemoryPool->deallocate(pContext);

    }
    else
    {

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::FillBufferDoneProcessing: Output frame %d received", iNodeTypeId, iFrameCounter));

        iFrameCounter++;
        // get pointer to actual buffer data
        uint8 *pBufdata = ((uint8*) aBuffer->pBuffer);
        // move the data pointer based on offset info
        pBufdata += aBuffer->nOffset;

        uint32 bufLen = (uint32) aBuffer->nFilledLen;

        // in case of mp4 streaming and the very 1st buffer, save vol header separately
        if ((iOutFormat == PVMF_MIME_M4V) && (iVideoEncodeParam.iContentType == EI_M4V_STREAMING)
                && (iFrameCounter == 1))
        {

            // save the first buffer since this is the VOL header

            uint refCounterSize = oscl_mem_aligned_size(sizeof(OsclRefCounterDA));
            OsclMemoryFragment volHeader;
            volHeader.ptr = NULL;
            volHeader.len = aBuffer->nFilledLen; // vol header size should be (28)
            uint8* memBuffer = (uint8*)iAlloc.allocate(refCounterSize + volHeader.len);
            oscl_memset(memBuffer, 0, refCounterSize + volHeader.len);
            OsclRefCounter* refCounter = OSCL_PLACEMENT_NEW(memBuffer, OsclRefCounterDA(memBuffer, (OsclDestructDealloc*) & iAlloc));
            memBuffer += refCounterSize;
            volHeader.ptr = (OsclAny*)memBuffer;

            // copy the vol header from OMX buffer
            oscl_memcpy(volHeader.ptr, pBufdata, volHeader.len);

            // save in class variable
            iVolHeader = OsclRefCounterMemFrag(volHeader, refCounter, volHeader.len);

            // release the OMX buffer
            iOutBufMemoryPool->deallocate(pContext);
            return OMX_ErrorNone;
        }

        if (iFrameCounter == 1)
        {
            if ((iOutFormat == PVMF_MIME_ADTS)
                    || (iOutFormat == PVMF_MIME_ADIF)
                    || (iOutFormat == PVMF_MIME_MPEG4_AUDIO)
                    || (iOutFormat == PVMF_MIME_WMA)
                    || (iOutFormat == PVMF_MIME_WMV)
               )
            {

                // save the first buffer since this is the config header and needs to be sent separately
                uint refCounterSize = oscl_mem_aligned_size(sizeof(OsclRefCounterDA));
                OsclMemoryFragment configHeader;
                configHeader.ptr = NULL;
                configHeader.len = aBuffer->nFilledLen;
                uint8* memBuffer = (uint8*)iAlloc.allocate(refCounterSize + configHeader.len);
                oscl_memset(memBuffer, 0, refCounterSize + configHeader.len);
                OsclRefCounter* refCounter = OSCL_PLACEMENT_NEW(memBuffer, OsclRefCounterDA(memBuffer, (OsclDestructDealloc*) & iAlloc));
                memBuffer += refCounterSize;
                configHeader.ptr = (OsclAny*)memBuffer;

                // copy the vol header from OMX buffer
                oscl_memcpy(configHeader.ptr, pBufdata, configHeader.len);

                // save in class variable
                iConfigHeader = OsclRefCounterMemFrag(configHeader, refCounter, configHeader.len);

                // release the OMX buffer
                iOutBufMemoryPool->deallocate(pContext);
                return OMX_ErrorNone;
            }
        }

        // in case of avc mp4, need to save sps/pps sequences
        if ((iOutFormat == PVMF_MIME_H264_VIDEO_MP4) && (!iSpsPpsSequenceOver))
        {

            if (iOMXComponentUsesNALStartCodes)
            {
                // remove the start code from these single NAL frames
                pBufdata += iFirstNALStartCodeSize;
                bufLen -= iFirstNALStartCodeSize;
            }
            // detect nal type
            uint8 *bitstream = pBufdata;
            int nal_type;

            // adjust the pointers in the iParamSet memfragment
            uint8 *destptr = (uint8*) iParamSet.getMemFragPtr();
            uint32 length = iParamSet.getMemFrag().len;
            uint32 capacity = iParamSet.getCapacity();

            destptr += length;


            nal_type = bitstream[0] & 0x1F;

            if (nal_type == 0x07) // SPS type NAL
            {
                // can the SPS fit into the buffer
                if (aBuffer->nFilledLen <= (capacity - length))
                {
                    iSPSs[iNumSPSs].ptr = destptr;
                    iSPSs[iNumSPSs++].len = aBuffer->nFilledLen;

                    oscl_memcpy(destptr, pBufdata, aBuffer->nFilledLen); // copy SPS into iParamSet memfragment
                    length += aBuffer->nFilledLen;
                    iParamSet.getMemFrag().len = length; // update length
                }


                // release the OMX buffer
                iOutBufMemoryPool->deallocate(pContext);
                return OMX_ErrorNone;
            }
            else if (nal_type == 0x08) // PPS type NAL
            {

                // can the PPS fit into the buffer?
                if (aBuffer->nFilledLen <= (capacity - length))
                {

                    iPPSs[iNumPPSs].ptr = destptr;
                    iPPSs[iNumPPSs++].len = aBuffer->nFilledLen;

                    oscl_memcpy(destptr, pBufdata, aBuffer->nFilledLen); // copy PPS into iParamSet memfragment
                    length += aBuffer->nFilledLen;
                    iParamSet.getMemFrag().len = length; // update length


                }

                // release the OMX buffer
                iOutBufMemoryPool->deallocate(pContext);
                return OMX_ErrorNone;
            }
            else
            {
                if (iOMXComponentUsesNALStartCodes)
                {
                    // bring start code back since it's not a SPS or PPS
                    pBufdata -= iFirstNALStartCodeSize;
                    bufLen += iFirstNALStartCodeSize;
                }

                // this is neither SPS nor PPS
                // stop recording SPS/PPS
                iSpsPpsSequenceOver = true;
                iFirstNAL = true; // set indicator of first NAL
                // send out SPS/PPS recorded so far
                if (((PVMFOMXEncPort*)iOutPort))
                {
                    ((PVMFOMXEncPort*)iOutPort)->SendSPS_PPS(iSPSs, iNumSPSs, iPPSs, iNumPPSs);
                }
            }

        }

        if (iOMXComponentUsesFullAVCFrames && iOutFormat == PVMF_MIME_H264_VIDEO_MP4)
        {
            // do not parse fo PVMF_MIME_H264_VIDEO_RAW, since this data is just written raw, and knowing
            // the NAL boundaries isn't necessary

            if (!ParseFullAVCFramesIntoNALs(aBuffer))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "PVMFOMXEncNode-%s::FillBufferDoneProcessing: Problem parsing NALs in buffer %x of size %d - releasing the buffer", iNodeTypeId, pBufdata, aBuffer->nFilledLen));

                iOutBufMemoryPool->deallocate(pContext);
            }
        }

        // adjust ptr to data to expose the NAL sync word if necessary
        if ((iOutFormat == PVMF_MIME_H264_VIDEO_RAW) &&
                (iOMXComponentSupportsExternalOutputBufferAlloc) &&
                (!iOMXComponentUsesFullAVCFrames) &&
                (!iOMXComponentUsesNALStartCodes) &&
                ((iFirstNAL == true) || (iEndOfNALFlagPrevious != 0))
           )
        {
            pBufdata -= 4;
            bufLen += 4;
        }

        // otherwise, queue output buffer
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::FillBufferDoneProcessing: Wrapping buffer %x of size %d", iNodeTypeId, pBufdata, aBuffer->nFilledLen));
        // wrap the buffer into the MediaDataImpl wrapper, and queue it for sending downstream
        // wrapping will create a refcounter. When refcounter goes to 0 i.e. when media data
        // is released in downstream components, the custom deallocator will automatically release the buffer back to the
        //	mempool. To do that, the deallocator needs to have info about Context
        // NOTE: we had to wait until now to wrap the buffer data because we only know
        //			now where the actual data is located (based on buffer offset)
        OsclSharedPtr<PVMFMediaDataImpl> MediaDataCurr = WrapOutputBuffer(pBufdata, bufLen, pContext);

        // if you can't get the MediaDataCurr, release the buffer back to the pool
        if (MediaDataCurr.GetRep() == NULL)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXEncNode-%s::FillBufferDoneProcessing: Problem wrapping buffer %x of size %d - releasing the buffer", iNodeTypeId, pBufdata, aBuffer->nFilledLen));

            iOutBufMemoryPool->deallocate(pContext);
        }
        else
        {
            OsclSharedPtr<PVMFMediaDataImpl> MediaDataOut;

            //DV - NOTE:
            //In case of AVC, OMX_EndOfFrameFlag is used to mark the end of NAL
            //				  End of frame is determined by comparing timestamps
            //				As a consequence, there is a lag of one buffer
            //				i.e. we send/queue the previous buffer and keep the current one so that
            //				When the next buffer arrives, we can compare timestamps etc.

            // In case of MP4, OMX_EndOFFrameFlag is used to mark end of frame and there is no lag

            if ((iOutFormat == PVMF_MIME_H264_VIDEO_MP4) || (iOutFormat == PVMF_MIME_H264_VIDEO_RAW))
            {

                MediaDataOut = iPreviousMediaData; // send out previous media data

                // copy data to be attached with outgoing media data from previous buffer
                iTimeStampOut = iTimeStampPrevious;
                iKeyFrameFlagOut = iKeyFrameFlagPrevious;
                iEndOfNALFlagOut = iEndOfNALFlagPrevious;
                // figure out if we need to set end of frame based on TS
                if (aBuffer->nTimeStamp != iTimeStampPrevious)
                    iEndOfFrameFlagOut = 1;
                else
                    iEndOfFrameFlagOut = 0;
                iBufferLenOut = iBufferLenPrevious;

                // now read the info for the current data
                iPreviousMediaData = MediaDataCurr;
                // record timestamp, flags etc.
                iTimeStampPrevious = aBuffer->nTimeStamp;
                // check for Key Frame
                iKeyFrameFlagPrevious = (aBuffer->nFlags & OMX_BUFFERFLAG_SYNCFRAME);
                // for avc, ENDOFFRAMEFLAG is used to delimit NALs
                iEndOfNALFlagPrevious = (aBuffer->nFlags & OMX_BUFFERFLAG_ENDOFFRAME);
                iBufferLenPrevious = bufLen;


            }
            else // MP4 case
            {

                MediaDataOut = MediaDataCurr;
                // record timestamp, flags etc.
                iTimeStampOut = aBuffer->nTimeStamp;
                // check for Key Frame
                iKeyFrameFlagOut = (aBuffer->nFlags & OMX_BUFFERFLAG_SYNCFRAME);
                iEndOfNALFlagOut = 0; // set this to 0 for mp4
                //check for END OF FRAME FLAG
                iEndOfFrameFlagOut = (aBuffer->nFlags & OMX_BUFFERFLAG_ENDOFFRAME);
                iBufferLenOut = aBuffer->nFilledLen;


            }
            // if there's a problem queuing output buffer, MediaDataCurr will expire at end of scope and
            // release buffer back to the pool, (this should not be the case)

            // queue only if not the first NAL
            if (!iFirstNAL)
            {
                if (QueueOutputBuffer(MediaDataOut, iBufferLenOut))
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                    (0, "PVMFOMXEncNode-%s::FillBufferDoneProcessing: Buffer %x of size %d queued - reschedule the node to send out", iNodeTypeId, pBufdata, aBuffer->nFilledLen));

                    // if queing went OK,
                    // re-schedule the node so that outgoing queue can be emptied (unless the outgoing port is busy)
                    if ((iOutPort) && !(iOutPort->IsConnectedPortBusy()))
                        RunIfNotReady();
                }
                else
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                    (0, "PVMFOMXEncNode-%s::FillBufferDoneProcessing: Problem queing buffer %x of size %d - releasing the buffer", iNodeTypeId, pBufdata, aBuffer->nFilledLen));
                }
            }
            else
            {
                iFirstNAL = false;
            }

            // if EOS and AVC (with a lag), queue also the last buffer
            if ((aBuffer->nFlags & OMX_BUFFERFLAG_EOS) &&
                    ((iOutFormat == PVMF_MIME_H264_VIDEO_MP4) || (iOutFormat == PVMF_MIME_H264_VIDEO_RAW))
               )
            {

                MediaDataOut = iPreviousMediaData; // send out previous media data

                // copy data to be attached with outgoing media data from previous buffer
                iTimeStampOut = iTimeStampPrevious;
                iKeyFrameFlagOut = iKeyFrameFlagPrevious;
                iEndOfNALFlagOut =   1;
                iEndOfFrameFlagOut = 1;

                iBufferLenOut = iBufferLenPrevious;

                if (QueueOutputBuffer(MediaDataOut, iBufferLenOut))
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                    (0, "PVMFOMXEncNode-%s::FillBufferDoneProcessing: Buffer %x of size %d queued - reschedule the node to send out", iNodeTypeId, pBufdata, aBuffer->nFilledLen));

                    // if queing went OK,
                    // re-schedule the node so that outgoing queue can be emptied (unless the outgoing port is busy)
                    if ((iOutPort) && !(iOutPort->IsConnectedPortBusy()))
                        RunIfNotReady();
                }
                else
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                    (0, "PVMFOMXEncNode-%s::FillBufferDoneProcessing: Problem queing buffer %x of size %d - releasing the buffer", iNodeTypeId, pBufdata, aBuffer->nFilledLen));
                }

            }
        }

    }
    // the OMX spec says that no error is to be returned
    return OMX_ErrorNone;

}
////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// Put output buffer in outgoing queue //////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
bool PVMFOMXEncNode::QueueOutputBuffer(OsclSharedPtr<PVMFMediaDataImpl> &mediadataimplout, uint32 aDataLen)
{

    bool status = true;
    PVMFSharedMediaDataPtr mediaDataOut;
    int32 leavecode = 0;

    // NOTE: ASSUMPTION IS THAT OUTGOING QUEUE IS BIG ENOUGH TO QUEUE ALL THE OUTPUT BUFFERS
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXEncNode-%s::QueueOutputFrame: In", iNodeTypeId));

    // First check if we can put outgoing msg. into the queue
    if (iOutPort->IsOutgoingQueueBusy())
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "PVMFOMXEncNode-%s::QueueOutputFrame() OutgoingQueue is busy", iNodeTypeId));
        return false;
    }

    OSCL_TRY(leavecode,
             mediaDataOut = PVMFMediaData::createMediaData(mediadataimplout, iMediaDataMemPool););
    if (leavecode == 0)
    {

        uint32 marker_bits = mediaDataOut->getMarkerInfo();

        if (iEndOfFrameFlagOut != 0)
        {
            marker_bits |= PVMF_MEDIA_DATA_MARKER_INFO_M_BIT;
        }

        if (iKeyFrameFlagOut != 0)
        {
            marker_bits |= PVMF_MEDIA_DATA_MARKER_INFO_RANDOM_ACCESS_POINT_BIT;
        }

        if (iEndOfNALFlagOut != 0)
        {
            marker_bits |= PVMF_MEDIA_DATA_MARKER_INFO_END_OF_NAL_BIT;
        }

        // attach marker bit
        mediaDataOut->setMarkerInfo(marker_bits);


        // Update the filled length of the fragment
        mediaDataOut->setMediaFragFilledLen(0, aDataLen);

        // Set timestamp
        mediaDataOut->setTimestamp(iTimeStampOut);

        // Set Streamid
        mediaDataOut->setStreamID(iStreamID);

        // Set sequence number
        mediaDataOut->setSeqNum(iSeqNum++);

        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iDataPathLogger, PVLOGMSG_INFO, (0, ":PVMFOMXEncNode-%s::QueueOutputFrame(): - SeqNum=%d, TS=%d", iNodeTypeId, iSeqNum, iTimeStampOut));


        // Check if Fsi needs to be sent (VOL header)
        if (sendYuvFsi)
        {
            if (iVideoEncodeParam.iContentType == EI_M4V_STREAMING)
            {
                mediaDataOut->setFormatSpecificInfo(iVolHeader);

            }
            else if ((iOutFormat == PVMF_MIME_ADTS)
                     || (iOutFormat == PVMF_MIME_ADIF)
                     || (iOutFormat == PVMF_MIME_MPEG4_AUDIO)
                     || (iOutFormat == PVMF_MIME_WMA)
                     || (iOutFormat == PVMF_MIME_WMV))
            {

                mediaDataOut->setFormatSpecificInfo(iConfigHeader);

            }

            sendYuvFsi = false;
        }

        // Send frame to downstream node
        PVMFSharedMediaMsgPtr mediaMsgOut;
        convertToPVMFMediaMsg(mediaMsgOut, mediaDataOut);

        if (iOutPort && (iOutPort->QueueOutgoingMsg(mediaMsgOut) == PVMFSuccess))
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                            (0, "PVMFOMXEncNode-%s::QueueOutputFrame(): Queued frame OK ", iNodeTypeId));

        }
        else
        {
            // we should not get here because we always check for whether queue is busy or not
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFOMXEncNode-%s::QueueOutputFrame(): Send frame failed", iNodeTypeId));
            return false;
        }




    }//end of if (leavecode==0)
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVMFOMXEncNode-%s::QueueOutputFrame() call PVMFMediaData::createMediaData is failed", iNodeTypeId));
        return false;
    }

    return status;

}

//////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// Attach a MediaDataImpl wrapper (refcount, deallocator etc.)
/////////////////////////////// to the output buffer /////////////////////////////////////////
OsclSharedPtr<PVMFMediaDataImpl> PVMFOMXEncNode::WrapOutputBuffer(uint8 *pData, uint32 aDataLen, OsclAny *pContext)
{
    // wrap output buffer into a mediadataimpl
    uint32 aligned_cleanup_size = oscl_mem_aligned_size(sizeof(PVOMXEncBufferSharedPtrWrapperCombinedCleanupDA));
    uint32 aligned_refcnt_size = oscl_mem_aligned_size(sizeof(OsclRefCounterDA));
    uint8 *my_ptr = (uint8*) oscl_malloc(aligned_cleanup_size + aligned_refcnt_size);

    if (my_ptr == NULL)
    {
        OsclSharedPtr<PVMFMediaDataImpl> null_buff(NULL, NULL);
        return null_buff;
    }

    // create a deallocator and pass the buffer_allocator to it as well as pointer to data that needs to be returned to the mempool
    PVOMXEncBufferSharedPtrWrapperCombinedCleanupDA *cleanup_ptr =
        OSCL_PLACEMENT_NEW(my_ptr + aligned_refcnt_size, PVOMXEncBufferSharedPtrWrapperCombinedCleanupDA(iOutBufMemoryPool, pContext));

    // create the ref counter after the cleanup object (refcount is set to 1 at creation)
    OsclRefCounterDA *my_refcnt;
    PVMFMediaDataImpl* media_data_ptr;

    if (iOMXComponentUsesFullAVCFrames && iNumNALs > 0)
    {
        uint32 ii;
        media_data_ptr = OSCL_NEW(PVMFMediaFragGroup<OsclMemAllocator>, (iNumNALs));
        my_refcnt = OSCL_PLACEMENT_NEW(my_ptr, OsclRefCounterDA(media_data_ptr, cleanup_ptr));

        // loop through and assign a create a media fragment for each NAL
        for (ii = 0; ii < iNumNALs; ii++)
        {
            OsclMemoryFragment memFrag;

            if (iOMXComponentUsesNALStartCodes)
            {
                // need to get NAL ptr from stored array, since start codes can be 3 or 4 bytes,
                // and the only way to know is by parsing the buffer (which we should have done by now and stored in the array)
                memFrag.ptr = iNALPtrArray[ii];
            }
            else
            {
                memFrag.ptr = pData;
                pData += iNALSizeArray[ii];
            }

            memFrag.len = iNALSizeArray[ii];

            OsclRefCounterMemFrag refCountMemFragOut(memFrag, my_refcnt, memFrag.len);
            media_data_ptr->appendMediaFragment(refCountMemFragOut);
        }

        oscl_memset((void *) iNALSizeArray, 0, sizeof(uint32) * iNumNALs);
        iNumNALs = 0;
    }
    else
    {
        media_data_ptr = OSCL_NEW(PVMFMediaFragGroup<OsclMemAllocator>, (1));
        my_refcnt = OSCL_PLACEMENT_NEW(my_ptr, OsclRefCounterDA(media_data_ptr, cleanup_ptr));

        OsclMemoryFragment memFrag;

        if (iOMXComponentUsesNALStartCodes && (iOutFormat != PVMF_MIME_H264_VIDEO_RAW))
        {
            // skip start code
            pData += iFirstNALStartCodeSize;
        }

        memFrag.ptr = pData;
        memFrag.len = aDataLen;

        OsclRefCounterMemFrag refCountMemFragOut(memFrag, my_refcnt, memFrag.len);
        media_data_ptr->appendMediaFragment(refCountMemFragOut);
    }

    OsclSharedPtr<PVMFMediaDataImpl> MediaDataImplOut(media_data_ptr, my_refcnt);

    return MediaDataImplOut;
}
//////////////////////////////////////////////////////////////////////////////
bool PVMFOMXEncNode::SendBeginOfMediaStreamCommand()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXEncNode-%s::SendBeginOfMediaStreamCommand() In", iNodeTypeId));

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
                        (0, "PVMFOMXEncNode-%s::SendBeginOfMediaStreamCommand() Outgoing queue busy", iNodeTypeId));
        return false;
    }

    iSendBOS = false;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXEncNode-%s::SendBeginOfMediaStreamCommand() BOS Sent StreamID %d", iNodeTypeId, iStreamID));
    return true;
}
////////////////////////////////////
bool PVMFOMXEncNode::SendEndOfTrackCommand(void)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXEncNode-%s::SendEndOfTrackCommand() In", iNodeTypeId));

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
                    (0, "PVMFOMXEncNode-%s::SendEndOfTrackCommand() Out", iNodeTypeId));
    return true;
}

/////////////////////////////////////////////////////////////////////////////
//The various command handlers call this routine when a command is complete.
/////////////////////////////////////////////////////////////////////////////
void PVMFOMXEncNode::CommandComplete(PVMFOMXEncNodeCmdQ& aCmdQ, PVMFOMXEncNodeCommand& aCmd, PVMFStatus aStatus, OsclAny* aEventData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::CommandComplete Id %d Cmd %d Status %d Context %d Data %d", iNodeTypeId
                    , aCmd.iId, aCmd.iCmd, aStatus, aCmd.iContext, aEventData));

    //create response
    PVMFCmdResp resp(aCmd.iId, aCmd.iContext, aStatus, aEventData);
    PVMFSessionId session = aCmd.iSession;

    //Erase the command from the queue.
    aCmdQ.Erase(&aCmd);

    //Report completion to the session observer.
    ReportCmdCompleteEvent(session, resp);
}

/////////////////////////////////////////////////////////////////////////////
void PVMFOMXEncNode::DoInit(PVMFOMXEncNodeCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::DoInit() In", iNodeTypeId));
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
void PVMFOMXEncNode::DoPrepare(PVMFOMXEncNodeCommand& aCmd)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    OMX_STRING Role = NULL;

    switch (iInterfaceState)
    {
        case EPVMFNodeInitialized:
        {
            if (NULL == iInPort)
            {
                CommandComplete(iInputCommands, aCmd, PVMFFailure);
            }


            // by now the encode parameters have been set

            if (iOutFormat == PVMF_MIME_H2631998 ||
                    iOutFormat == PVMF_MIME_H2632000)
            {
                Role = (OMX_STRING)"video_encoder.h263";
                iNodeTypeId = LOG_ID_VIDEO_H263;
            }
            else if (iOutFormat == PVMF_MIME_M4V)
            {
                Role = (OMX_STRING)"video_encoder.mpeg4";
                iNodeTypeId = LOG_ID_VIDEO_M4V;
            }
            else if (iOutFormat == PVMF_MIME_H264_VIDEO_RAW ||
                     iOutFormat == PVMF_MIME_H264_VIDEO_MP4)
            {
                Role = (OMX_STRING)"video_encoder.avc";
                iNodeTypeId = LOG_ID_VIDEO_AVC;
                iFirstNAL = true; // set this flag to prevent node from queueing the first
                // buffer
            }
            else if ((iOutFormat == PVMF_MIME_AMR_IETF) ||
                     (iOutFormat == PVMF_MIME_AMR_IF2))
            {
                Role = (OMX_STRING)"audio_encoder.amrnb";
                iNodeTypeId = LOG_ID_AUDIO_AMRNB;
            }
            else if (iOutFormat == PVMF_MIME_AMRWB_IETF)
            {
                Role = (OMX_STRING)"audio_encoder.amrwb";
                iNodeTypeId = LOG_ID_AUDIO_AMRWB;
            }
            else if (iOutFormat == PVMF_MIME_ADTS ||
                     iOutFormat == PVMF_MIME_ADIF ||
                     iOutFormat == PVMF_MIME_MPEG4_AUDIO)
            {
                Role = (OMX_STRING)"audio_encoder.aac";
                iNodeTypeId = LOG_ID_AUDIO_AAC;
            }
            else
            {
                // Illegal codec specified.
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncNode-%s::DoPrepare() Input port format other then codec type", iNodeTypeId));
                CommandComplete(iInputCommands, aCmd, PVMFErrArgument);
                return;
            }


            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "PVMFOMXEncNode-%s::Initializing OMX component and encoder for role %s", iNodeTypeId, Role));

            /* Set callback structure */
            iCallbacks.EventHandler    = CallbackEventHandlerEnc; //event_handler;
            iCallbacks.EmptyBufferDone = CallbackEmptyBufferDoneEnc; //empty_buffer_done;
            iCallbacks.FillBufferDone  = CallbackFillBufferDoneEnc; //fill_buffer_done;


            // determine components which can fit the role
            // then, create the component. If multiple components fit the role,
            // the first one registered will be selected. If that one fails to
            // be created, the second one in the list is selected etc.
            OMX_U32 num_comps = 0;
            OMX_STRING *CompOfRole;
            OMX_S8 CompName[PV_OMX_MAX_COMPONENT_NAME_LENGTH];
            // call once to find out the number of components that can fit the role
            OMX_GetComponentsOfRole(Role, &num_comps, NULL);
            uint32 ii;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "PVMFOMXEncNode-%s::DoPrepare(): There are %d components of role %s ", iNodeTypeId, num_comps, Role));

            if (num_comps > 0)
            {
                CompOfRole = (OMX_STRING *)oscl_malloc(num_comps * sizeof(OMX_STRING));

                for (ii = 0; ii < num_comps; ii++)
                    CompOfRole[ii] = (OMX_STRING) oscl_malloc(PV_OMX_MAX_COMPONENT_NAME_LENGTH * sizeof(OMX_U8));

                // call 2nd time to get the component names
                OMX_GetComponentsOfRole(Role, &num_comps, (OMX_U8 **)CompOfRole);

                for (ii = 0; ii < num_comps; ii++)
                {
                    // try to create component
                    err = OMX_GetHandle(&iOMXEncoder, (OMX_STRING) CompOfRole[ii], (OMX_PTR) this, (OMX_CALLBACKTYPE *) & iCallbacks);
                    // if successful, no need to continue
                    if ((err == OMX_ErrorNone) && (iOMXEncoder != NULL))
                    {
                        oscl_strncpy((OMX_STRING)CompName, (OMX_STRING) CompOfRole[ii], PV_OMX_MAX_COMPONENT_NAME_LENGTH);

                        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                        (0, "PVMFOMXEncNode-%s::DoPrepare(): Got Component %s handle ", iNodeTypeId, CompOfRole[ii]));

                        break;
                    }
                    else
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                        (0, "PVMFOMXEncNode-%s::DoPrepare(): Cannot get component %s handle, try another component if available", iNodeTypeId, CompOfRole[ii]));
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
                if ((err != OMX_ErrorNone) || (iOMXEncoder == NULL))
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "PVMFOMXEncNode-%s::Can't get handle for encoder!", iNodeTypeId));
                    iOMXEncoder = NULL;
                    CommandComplete(iInputCommands, aCmd, PVMFErrResource);
                    return;
                }
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMFOMXEncNode-%s::No component can handle role %s !", iNodeTypeId, Role));
                iOMXEncoder = NULL;
                CommandComplete(iInputCommands, aCmd, PVMFErrResource);
                return;
            }



            if (!iOMXEncoder)
            {
                CommandComplete(iInputCommands, aCmd, PVMFErrNoResources);
                return;
            }


            // find out how many roles the component supports
            OMX_U32 NumRoles;
            err = OMX_GetRolesOfComponent((OMX_STRING)CompName, &NumRoles, NULL);
            if (err != OMX_ErrorNone)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMFOMXEncNode-%s::DoPrepare() Problem getting component roles", iNodeTypeId));

                CommandComplete(iInputCommands, aCmd, PVMFErrResource);
                return;
            }

            // if the component supports multiple roles, call OMX_SetParameter
            if (NumRoles > 1)
            {
                OMX_PARAM_COMPONENTROLETYPE RoleParam;
                CONFIG_SIZE_AND_VERSION(RoleParam);
                oscl_strncpy((OMX_STRING)RoleParam.cRole, (OMX_STRING)Role, OMX_MAX_STRINGNAME_SIZE);
                err = OMX_SetParameter(iOMXEncoder, OMX_IndexParamStandardComponentRole, &RoleParam);
                if (err != OMX_ErrorNone)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "PVMFOMXEncNode-%s::DoPrepare() Problem setting component role", iNodeTypeId));

                    CommandComplete(iInputCommands, aCmd, PVMFErrResource);
                    return;
                }
            }


            // GET CAPABILITY FLAGS FROM PV COMPONENT, IF this fails, use defaults
            PV_OMXComponentCapabilityFlagsType Cap_flags;
            err = OMX_GetParameter(iOMXEncoder, (OMX_INDEXTYPE) PV_OMX_COMPONENT_CAPABILITY_TYPE_INDEX, &Cap_flags);
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

            /*	iOMXComponentUsesNALStartCodes:				The component inserts start codes before NALs

             	iOMXComponentUsesFullAVCFrames
             		&& !iOMXComponentUsesNALStartCodes:		The component outputs full frames, and stores NAL start codes using the
             												OMX ExtraData structure in the output buffer
             	iOMXComponentUsesFullAVCFrames
             		&& iOMXComponentUsesNALStartCodes:		The component outputs full frames, and delimits NALs by their start codes

             	iOutFormat == PVMF_MIME_H264_VIDEO_RAW
             		&& !iOMXComponentUsesNALStartCodes:		The node inserts the start codes and hides them / exposes them when needed

             	iOutFormat == PVMF_MIME_H264_VIDEO_RAW
             		&& !iOMXComponentUsesNALStartCodes
             		&& iOMXComponentUsesFullAVCFrames:		This is an invalid combination.  If the node wants raw output, and the component
             												uses full frames, and no start codes, then there is no way to detect the
             												NAL boundaries.

             */

            if (iOutFormat == PVMF_MIME_H264_VIDEO_RAW &&
                    iOMXComponentUsesFullAVCFrames && !iOMXComponentUsesNALStartCodes)
            {
                // This is an invalid combination (see above). Therefore, return an error.


                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMFOMXEncNode-%s::DoPrepare() Component cannot support %s format", iNodeTypeId, PVMF_MIME_H264_VIDEO_RAW));

                CommandComplete(iInputCommands, aCmd, PVMFErrNotSupported);
                return;
            }


            // find out about parameters
            if ((iOutFormat == PVMF_MIME_AMR_IETF) || (iOutFormat == PVMF_MIME_AMRWB_IETF) || (iOutFormat == PVMF_MIME_AMR_IF2) ||
                    (iOutFormat == PVMF_MIME_ADIF) || (iOutFormat == PVMF_MIME_ADTS) || (iOutFormat == PVMF_MIME_MPEG4_AUDIO))
            {
                if (!NegotiateAudioComponentParameters())
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "PVMFOMXEncNode-%s::DoPrepare() Cannot get component parameters", iNodeTypeId));

                    CommandComplete(iInputCommands, aCmd, PVMFErrNoResources);
                    return;
                }
            }
            else
            {

                if (!NegotiateVideoComponentParameters())
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "PVMFOMXEncNode-%s::DoPrepare() Cannot get component parameters", iNodeTypeId));

                    CommandComplete(iInputCommands, aCmd, PVMFErrNoResources);
                    return;
                }
            }

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
            // Get the priority of the encoder node, and set the threadsafe callback AO priority to 1 higher

            iThreadSafeHandlerEventHandler = OSCL_NEW(EventHandlerThreadSafeCallbackAOEnc, (this, 10, "EventHandlerAO", Priority() + 2));

            if (iThreadSafeHandlerEmptyBufferDone)
            {
                OSCL_DELETE(iThreadSafeHandlerEmptyBufferDone);
                iThreadSafeHandlerEmptyBufferDone = NULL;
            }
            // use queue depth of iNumInputBuffers to prevent deadlock
            iThreadSafeHandlerEmptyBufferDone = OSCL_NEW(EmptyBufferDoneThreadSafeCallbackAOEnc, (this, iNumInputBuffers, "EmptyBufferDoneAO", Priority() + 1));

            if (iThreadSafeHandlerFillBufferDone)
            {
                OSCL_DELETE(iThreadSafeHandlerFillBufferDone);
                iThreadSafeHandlerFillBufferDone = NULL;
            }
            // use queue depth of iNumOutputBuffers to prevent deadlock
            iThreadSafeHandlerFillBufferDone = OSCL_NEW(FillBufferDoneThreadSafeCallbackAOEnc, (this, iNumOutputBuffers, "FillBufferDoneAO", Priority() + 1));

            if ((iThreadSafeHandlerEventHandler == NULL) ||
                    (iThreadSafeHandlerEmptyBufferDone == NULL) ||
                    (iThreadSafeHandlerFillBufferDone == NULL)
               )
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMFOMXEncNode-%s::Can't get threadsafe callbacks for encoder!", iNodeTypeId));
                iOMXEncoder = NULL;
            }


            // Init Encoder
            iCurrentEncoderState = OMX_StateLoaded;

            /* Change state to OMX_StateIdle from OMX_StateLoaded. */
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "PVMFOMXEncNode-%s::DoPrepare(): Changing Component state Loaded -> Idle ", iNodeTypeId));

            err = OMX_SendCommand(iOMXEncoder, OMX_CommandStateSet, OMX_StateIdle, NULL);
            if (err != OMX_ErrorNone)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMFOMXEncNode-%s::DoPrepare() Can't send StateSet command!", iNodeTypeId));

                CommandComplete(iInputCommands, aCmd, PVMFErrNoResources);
                return;
            }


            /* Allocate input buffers */
            if (!CreateInputMemPool(iNumInputBuffers))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMFOMXEncNode-%s::DoPrepare() Can't allocate mempool for input buffers!", iNodeTypeId));

                CommandComplete(iInputCommands, aCmd, PVMFErrNoResources);
                return;
            }

            in_ctrl_struct_ptr = NULL;
            in_buff_hdr_ptr = NULL;

            in_ctrl_struct_ptr = (OsclAny **) oscl_malloc(iNumInputBuffers * sizeof(OsclAny *));

            if (in_ctrl_struct_ptr == NULL)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "PVMFOMXEncNode-%s::DoPreapare in_ctrl_struct_ptr == NULL", iNodeTypeId));

                CommandComplete(iInputCommands, aCmd, PVMFErrNoResources);
                return ;
            }

            in_buff_hdr_ptr = (OsclAny **) oscl_malloc(iNumInputBuffers * sizeof(OsclAny *));

            if (in_buff_hdr_ptr == NULL)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "PVMFOMXEncNode-%s::DoPreapare in_buff_hdr_ptr == NULL", iNodeTypeId));

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
                                (0, "PVMFOMXEncNode-%s::DoPrepare() Component can't use input buffers!", iNodeTypeId));

                CommandComplete(iInputCommands, aCmd, PVMFErrNoResources);
                return;
            }


            /* Allocate output buffers */
            if (!CreateOutMemPool(iNumOutputBuffers))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMFOMXEncNode-%s::DoPrepare() Can't allocate mempool for output buffers!", iNodeTypeId));

                CommandComplete(iInputCommands, aCmd, PVMFErrNoResources);
                return;
            }

            out_ctrl_struct_ptr = NULL;
            out_buff_hdr_ptr = NULL;

            out_ctrl_struct_ptr = (OsclAny **) oscl_malloc(iNumOutputBuffers * sizeof(OsclAny *));

            if (out_ctrl_struct_ptr == NULL)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "PVMFOMXEncNode-%s::DoPreapare out_ctrl_struct_ptr == NULL", iNodeTypeId));

                CommandComplete(iInputCommands, aCmd, PVMFErrNoResources);
                return ;
            }

            out_buff_hdr_ptr = (OsclAny **) oscl_malloc(iNumOutputBuffers * sizeof(OsclAny *));

            if (out_buff_hdr_ptr == NULL)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "PVMFOMXEncNode-%s::DoPreapare out_buff_hdr_ptr == NULL", iNodeTypeId));

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
                                (0, "PVMFOMXEncNode-%s::DoPrepare() Component can't use output buffers!", iNodeTypeId));

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
void PVMFOMXEncNode::DoStart(PVMFOMXEncNodeCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::DoStart() In", iNodeTypeId));

    iDiagnosticsLogged = false;

    PVMFStatus status = PVMFSuccess;

    OMX_ERRORTYPE  err;
    OMX_STATETYPE sState;

#ifdef _TEST_AE_ERROR_HANDLING
    if (FAIL_NODE_CMD_START == iErrorNodeCmd)
    {
        iInterfaceState = EPVMFNodeError;
    }
#endif



    switch (iInterfaceState)
    {
        case EPVMFNodePrepared:
        case EPVMFNodePaused:
        {
            //Get state of OpenMAX encoder
            err = OMX_GetState(iOMXEncoder, &sState);
            if (err != OMX_ErrorNone)
            {
                //Error condition report
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMFOMXEncNode-%s::DoStart(): Can't get State of encoder!", iNodeTypeId));

                sState = OMX_StateInvalid;
            }
#ifdef _TEST_AE_ERROR_HANDLING
            if (iErrorHandlingInit)
            {
                //some random state ,we want Init to fail

                sState = OMX_StateInvalid;
            }
#endif
            if ((sState == OMX_StateIdle) || (sState == OMX_StatePause))
            {
                /* Change state to OMX_StateExecuting form OMX_StateIdle. */
                // init the flag
                iDoNotSendOutputBuffersDownstreamFlag = false; // or if output was not being sent downstream due to state changes
                // re-anable sending output

                iDoNotSaveInputBuffersFlag = false;


                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "PVMFOMXEncNode-%s::DoStart() Changing Component state Idle->Executing", iNodeTypeId));

                err = OMX_SendCommand(iOMXEncoder, OMX_CommandStateSet, OMX_StateExecuting, NULL);
                if (err != OMX_ErrorNone)
                {
                    //Error condition report
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "PVMFOMXEncNode-%s::DoStart(): Can't send StateSet command to encoder!", iNodeTypeId));

                    status = PVMFErrInvalidState;
                }

            }
            else
            {
                //Error condition report
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMFOMXEncNode-%s::DoStart(): Encoder is not in the Idle or Pause state!", iNodeTypeId));

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
void PVMFOMXEncNode::DoStop(PVMFOMXEncNodeCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::DoStop() In", iNodeTypeId));

    LogDiagnostics();

    OMX_ERRORTYPE  err;
    OMX_STATETYPE sState;
#ifdef _TEST_AE_ERROR_HANDLING
    if (FAIL_NODE_CMD_STOP == iErrorNodeCmd)
    {
        iInterfaceState = EPVMFNodeError;
    }
#endif
    switch (iInterfaceState)
    {
        case EPVMFNodeStarted:
        case EPVMFNodePaused:
        case EPVMFNodePrepared:
            // Stop data source
            // This will also prevent execution of HandleProcessingState

            iDataIn.Unbind();
            iPreviousMediaData.Unbind();
            if ((iOutFormat == PVMF_MIME_H264_VIDEO_MP4) || (iOutFormat == PVMF_MIME_H264_VIDEO_RAW))
            {
                // prepare for next start (if it comes)
                iFirstNAL = true;
            }
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


            //Get state of OpenMAX encoder
            err = OMX_GetState(iOMXEncoder, &sState);
            if (err != OMX_ErrorNone)
            {
                //Error condition report
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMFOMXEncNode-%s::DoStop(): Can't get State of encoder!", iNodeTypeId));

                sState = OMX_StateInvalid;
            }

            if ((sState == OMX_StateExecuting) || (sState == OMX_StatePause))
            {
                /* Change state to OMX_StateIdle from OMX_StateExecuting or OMX_StatePause. */

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "PVMFOMXEncNode-%s::DoStop() Changing Component State Executing->Idle or Pause->Idle", iNodeTypeId));

                err = OMX_SendCommand(iOMXEncoder, OMX_CommandStateSet, OMX_StateIdle, NULL);
                if (err != OMX_ErrorNone)
                {
                    //Error condition report
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "PVMFOMXEncNode-%s::DoStop(): Can't send StateSet command to encoder!", iNodeTypeId));

                    CommandComplete(iInputCommands, aCmd, PVMFErrInvalidState);
                    break;
                }

                // prevent the node from sending more buffers etc.
                // if port reconfiguration is in process, let the state remain one of the port config states
                //	if there is a start command, we can do it seemlessly (by continuing the port reconfig)
                if (iProcessingState == EPVMFOMXEncNodeProcessingState_ReadyToEncode)
                    iProcessingState = EPVMFOMXEncNodeProcessingState_Stopping;

            }
            else
            {
                //Error condition report
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMFOMXEncNode-%s::DoStop(): Encoder is not in the Executing or Pause state!", iNodeTypeId));

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
void PVMFOMXEncNode::DoFlush(PVMFOMXEncNodeCommand& aCmd)
{
#ifdef _TEST_AE_ERROR_HANDLING
    if (FAIL_NODE_CMD_FLUSH == iErrorNodeCmd)
    {
        iInterfaceState = EPVMFNodeError;
    }
#endif
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
            RunIfNotReady();
            if (iOutPort)
            {
                iOutPort->SuspendInput();
            }
            // Stop data source

            // DV: Sending "OMX_CommandFlush" to the encoder;
            // Not used because PV Flush command expects data to be processed before getting out
            iDoNotSendOutputBuffersDownstreamFlag = true; // collect output buffers

            break;

        default:
            CommandComplete(iInputCommands, aCmd, PVMFErrInvalidState);
            break;
    }
}

/////////////////////////////////////////////////////////////////////////////
void PVMFOMXEncNode::DoPause(PVMFOMXEncNodeCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::DoPause() In", iNodeTypeId));

    OMX_ERRORTYPE  err;
    OMX_STATETYPE sState;
#ifdef _TEST_AE_ERROR_HANDLING
    if (FAIL_NODE_CMD_PAUSE == iErrorNodeCmd)
    {
        iInterfaceState = EPVMFNodeError;
    }
#endif

    switch (iInterfaceState)
    {
        case EPVMFNodeStarted:


            //Get state of OpenMAX encoder
            err = OMX_GetState(iOMXEncoder, &sState);
            if (err != OMX_ErrorNone)
            {
                //Error condition report
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMFOMXEncNode-%s::DoPause(): Can't get State of encoder!", iNodeTypeId));

                sState = OMX_StateInvalid;
            }

            if (sState == OMX_StateExecuting)
            {
                /* Change state to OMX_StatePause from OMX_StateExecuting. */
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "PVMFOMXEncNode-%s::DoPause() Changing Component State Executing->Idle", iNodeTypeId));

                err = OMX_SendCommand(iOMXEncoder, OMX_CommandStateSet, OMX_StatePause, NULL);
                if (err != OMX_ErrorNone)
                {
                    //Error condition report
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "PVMFOMXEncNode-%s::DoPause(): Can't send StateSet command to encoder!", iNodeTypeId));

                    CommandComplete(iInputCommands, aCmd, PVMFErrInvalidState);
                    break;
                }

            }
            else
            {
                //Error condition report
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMFOMXEncNode-%s::DoPause(): Encoder is not in the Executing state!", iNodeTypeId));
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
void PVMFOMXEncNode::DoReset(PVMFOMXEncNodeCommand& aCmd)
{

    OMX_ERRORTYPE  err;
    OMX_STATETYPE sState;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXEncNode-%s::DoReset() In", iNodeTypeId));

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
            //Check if encoder is initilized
            if (iOMXEncoder != NULL)
            {
                //Get state of OpenMAX encoder
                err = OMX_GetState(iOMXEncoder, &sState);
                if (err != OMX_ErrorNone)
                {
                    //Error condition report
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "PVMFOMXEncNode-%s::DoReset(): Can't get State of encoder!", iNodeTypeId));
                    if (iResetInProgress)
                    {
                        // cmd is in current q
                        iResetInProgress = false;
                        if ((iCurrentCommand.size() > 0) &&
                                (iCurrentCommand.front().iCmd == PVMFOMXEncNodeCommand::PVOMXENC_NODE_CMD_RESET)
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
                                        (0, "PVMFOMXEncNode-%s::DoReset() OMX comp is in loaded state. Wait for official callback to change variables etc.", iNodeTypeId));
                        return;
                    }
                    else
                    {
                        //delete all ports and notify observer.
                        if (iInPort)
                        {
                            OSCL_DELETE(((PVMFOMXEncPort*)iInPort));
                            iInPort = NULL;
                        }

                        if (iOutPort)
                        {
                            OSCL_DELETE(((PVMFOMXEncPort*)iOutPort));
                            iOutPort = NULL;
                        }

                        iDataIn.Unbind();


                        // Reset the metadata key list
                        iAvailableMetadataKeys.clear();

                        iEndOfDataReached = false;
                        iIsEOSSentToComponent = false;
                        iIsEOSReceivedFromComponent = false;


                        iProcessingState = EPVMFOMXEncNodeProcessingState_Idle;
                        //logoff & go back to Created state.
                        SetState(EPVMFNodeIdle);

                        CommandComplete(iInputCommands, aCmd, PVMFSuccess);

                        //CommandComplete(iInputCommands, aCmd, PVMFErrResource);
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
                                        (0, "PVMFOMXEncNode-%s::DoReset() Waiting for %d input and-or %d output buffers", iNodeTypeId, iNumOutstandingInputBuffers, iNumOutstandingOutputBuffers));

                        return;
                    }

                    if (!iResetMsgSent)
                    {
                        // We can come here only if all buffers are already back
                        // Don't repeat any of this twice.
                        /* Change state to OMX_StateLoaded form OMX_StateIdle. */
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                        (0, "PVMFOMXEncNode-%s::DoReset() Changing Component State Idle->Loaded", iNodeTypeId));

                        err = OMX_SendCommand(iOMXEncoder, OMX_CommandStateSet, OMX_StateLoaded, NULL);
                        if (err != OMX_ErrorNone)
                        {
                            //Error condition report
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                            (0, "PVMFOMXEncNode-%s::DoReset(): Can't send StateSet command to encoder!", iNodeTypeId));
                        }

                        iResetMsgSent = true;


                        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                        (0, "PVMFOMXEncNode-%s::DoReset() freeing output buffers", iNodeTypeId));

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
                                                (0, "PVMFOMXEncNode-%s::DoReset() Cannot free output buffers ", iNodeTypeId));

                                if (iResetInProgress)
                                {
                                    iResetInProgress = false;
                                    if ((iCurrentCommand.size() > 0) &&
                                            (iCurrentCommand.front().iCmd == PVMFOMXEncNodeCommand::PVOMXENC_NODE_CMD_RESET)
                                       )
                                    {
                                        CommandComplete(iCurrentCommand, iCurrentCommand.front() , PVMFErrResource);
                                    }
                                }

                            }

                        }
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                        (0, "PVMFOMXEncNode-%s::DoReset() freeing input buffers ", iNodeTypeId));
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
                                                (0, "PVMFOMXEncNode-%s::DoReset() Cannot free input buffers ", iNodeTypeId));

                                if (iResetInProgress)
                                {
                                    iResetInProgress = false;
                                    if ((iCurrentCommand.size() > 0) &&
                                            (iCurrentCommand.front().iCmd == PVMFOMXEncNodeCommand::PVOMXENC_NODE_CMD_RESET)
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

                        // don't send twice in a row

                        iDataIn.Unbind();
                        iPreviousMediaData.Unbind();
                        if ((iOutFormat == PVMF_MIME_H264_VIDEO_MP4) || (iOutFormat == PVMF_MIME_H264_VIDEO_RAW))
                        {
                            // prepare for next start (if it comes)
                            iFirstNAL = true;
                        }
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
                                        (0, "PVMFOMXEncNode-%s::DoReset() Changing Component State Executing->Idle or Pause->Idle", iNodeTypeId));

                        err = OMX_SendCommand(iOMXEncoder, OMX_CommandStateSet, OMX_StateIdle, NULL);
                        if (err != OMX_ErrorNone)
                        {
                            //Error condition report
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                            (0, "PVMFOMXEncNode-%s::DoReset(): Can't send StateSet command to Encoder!", iNodeTypeId));

                            CommandComplete(iInputCommands, aCmd, PVMFErrInvalidState);
                            break;
                        }

                        iStopInResetMsgSent = true;

                        // prevent the node from sending more buffers etc.
                        // if port reconfiguration is in process, let the state remain one of the port config states
                        //	if there is a start command, we can do it seemlessly (by continuing the port reconfig)
                        if (iProcessingState == EPVMFOMXEncNodeProcessingState_ReadyToEncode)
                            iProcessingState = EPVMFOMXEncNodeProcessingState_Stopping;
                    }
                    return;

                }
                else
                {
                    //Error condition report
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "PVMFOMXEncNode-%s::DoReset(): Encoder is not in the Idle state!", iNodeTypeId));
                    if (iResetInProgress)
                    {
                        iResetInProgress = false;
                        if ((iCurrentCommand.size() > 0) &&
                                (iCurrentCommand.front().iCmd == PVMFOMXEncNodeCommand::PVOMXENC_NODE_CMD_RESET)
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
            }//end of if (iOMXEncoder != NULL)

            //delete all ports and notify observer.
            if (iInPort)
            {
                OSCL_DELETE(((PVMFOMXEncPort*)iInPort));
                iInPort = NULL;
            }

            if (iOutPort)
            {
                OSCL_DELETE(((PVMFOMXEncPort*)iOutPort));
                iOutPort = NULL;
            }

            iDataIn.Unbind();


            // Reset the metadata key list
            iAvailableMetadataKeys.clear();

            iEndOfDataReached = false;
            iIsEOSSentToComponent = false;
            iIsEOSReceivedFromComponent = false;


            iProcessingState = EPVMFOMXEncNodeProcessingState_Idle;
            //logoff & go back to Created state.
            SetState(EPVMFNodeIdle);


            if (iResetInProgress)
            {
                iResetInProgress = false;
                if ((iCurrentCommand.size() > 0) &&
                        (iCurrentCommand.front().iCmd == PVMFOMXEncNodeCommand::PVOMXENC_NODE_CMD_RESET)
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
void PVMFOMXEncNode::DoRequestPort(PVMFOMXEncNodeCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXEncNode-%s::DoRequestPort() In", iNodeTypeId));
    //This node supports port request from any state

    //retrieve port tag.
    int32 tag;
    OSCL_String* portconfig;

    aCmd.PVMFOMXEncNodeCommandBase::Parse(tag, portconfig);

    PVMFPortInterface* port = NULL;


    int32 leavecode = 0;
    //validate the tag...

    switch (tag)
    {
        case PVMF_OMX_ENC_NODE_PORT_TYPE_INPUT:
            if (iInPort)
            {
                CommandComplete(iInputCommands, aCmd, PVMFFailure);
                break;
            }
            OSCL_TRY(leavecode, iInPort = OSCL_NEW(PVMFOMXEncPort, ((int32)tag, this, "OMXEncIn")););
            if (leavecode || iInPort == NULL)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMFOMXEncNode-%s::DoRequestPort: Error - Input port instantiation failed", iNodeTypeId));
                CommandComplete(iInputCommands, aCmd, PVMFErrArgument);
                return;
            }

            // if format was provided in mimestring, set it now.
            if (portconfig)
            {
                PVMFFormatType format = portconfig->get_str();
                if (((PVMFOMXEncPort*)iInPort)->IsFormatSupported(format))
                {
                    ((PVMFOMXEncPort*)iInPort)->iFormat = format;
                }
                else
                {

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "PVMFOMXEncNode-%s::DoRequestPort: Error - Input port does not support format %s", iNodeTypeId, format.getMIMEStrPtr()));
                    OSCL_DELETE(((PVMFOMXEncPort*)iInPort));
                    iInPort = NULL;
                    CommandComplete(iInputCommands, aCmd, PVMFErrArgument);
                    return;
                }

            }


            port = iInPort;
            break;

        case PVMF_OMX_ENC_NODE_PORT_TYPE_OUTPUT:
            if (iOutPort)
            {
                CommandComplete(iInputCommands, aCmd, PVMFFailure);
                break;
            }
            OSCL_TRY(leavecode, iOutPort = OSCL_NEW(PVMFOMXEncPort, ((int32)tag, this, "OMXEncOut")));
            if (leavecode || iOutPort == NULL)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMFOMXEncNode-%s::DoRequestPort: Error - Output port instantiation failed", iNodeTypeId));
                CommandComplete(iInputCommands, aCmd, PVMFErrArgument);
                return;
            }

            // if format was provided in mimestring, set it now.
            if (portconfig)
            {
                PVMFFormatType format = portconfig->get_str();
                if (((PVMFOMXEncPort*)iOutPort)->IsFormatSupported(format) && (SetCodecType(format) == PVMFSuccess))
                {
                    ((PVMFOMXEncPort*)iOutPort)->iFormat = format;
                }
                else
                {

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "PVMFOMXEncNode-%s::DoRequestPort: Error - Output port does not support format %s", iNodeTypeId, format.getMIMEStrPtr()));
                    OSCL_DELETE(((PVMFOMXEncPort*)iOutPort));
                    iOutPort = NULL;
                    CommandComplete(iInputCommands, aCmd, PVMFErrArgument);
                    return;
                }


            }

            port = iOutPort;
            break;

        default:
            //bad port tag
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFOMXEncNode-%s::DoRequestPort: Error - Invalid port tag", iNodeTypeId));
            CommandComplete(iInputCommands, aCmd, PVMFErrArgument);
            return;
    }

    //Return the port pointer to the caller.
    CommandComplete(iInputCommands, aCmd, PVMFSuccess, (OsclAny*)port);
}

/////////////////////////////////////////////////////////////////////////////
void PVMFOMXEncNode::DoReleasePort(PVMFOMXEncNodeCommand& aCmd)
{
    PVMFPortInterface* temp;
    aCmd.PVMFOMXEncNodeCommandBase::Parse(temp);
    PVMFOMXEncPort* port = (PVMFOMXEncPort*) temp;
#ifdef _TEST_AE_ERROR_HANDLING
    if (FAIL_NODE_CMD_RELEASE_PORT == iErrorNodeCmd)
    {
        port = NULL;
    }
#endif

    if (port != NULL && (port == iInPort || port == iOutPort))
    {
        if (port == iInPort)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXEncNode-%s::DoReleasePort Input port released", iNodeTypeId));

            OSCL_DELETE(((PVMFOMXEncPort*)iInPort));
            iInPort = NULL;
        }
        else
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXEncNode-%s::DoReleasePort Output port released", iNodeTypeId));

            OSCL_DELETE(((PVMFOMXEncPort*)iOutPort));
            iOutPort = NULL;
        }
        //delete the port.
        CommandComplete(iInputCommands, aCmd, PVMFSuccess);
    }
    else
    {
        //port not found.
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVMFOMXEncNode-%s::DoReleasePort ERROR unknown port cannot be released", iNodeTypeId));

        CommandComplete(iInputCommands, aCmd, PVMFFailure);
    }
}

/////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFOMXEncNode::DoGetNodeMetadataKey(PVMFOMXEncNodeCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXEncNode-%s::DoGetNodeMetadataKey() In", iNodeTypeId));

    PVMFMetadataList* keylistptr = NULL;
    uint32 starting_index;
    int32 max_entries;
    char* query_key;

    aCmd.PVMFOMXEncNodeCommand::Parse(keylistptr, starting_index, max_entries, query_key);

    // Check parameters
    if (keylistptr == NULL)
    {
        // The list pointer is invalid
        return PVMFErrArgument;
    }

    // Update the available metadata keys
    iAvailableMetadataKeys.clear();
    int32 leavecode = 0;
    leavecode = Push_Back_MetadataKeys(PVOMXENCMETADATA_CODECINFO_VIDEO_FORMAT_KEY);

    if (iYUVWidth > 0 && iYUVHeight > 0)
    {
        leavecode = 0;
        leavecode = Push_Back_MetadataKeys(PVOMXENCMETADATA_CODECINFO_VIDEO_WIDTH_KEY);
        if (0 == leavecode)
            leavecode = Push_Back_MetadataKeys(PVOMXENCMETADATA_CODECINFO_VIDEO_HEIGHT_KEY);
    }
    // add the profile, level and avgbitrate
    PVMF_MPEGVideoProfileType aProfile;
    PVMF_MPEGVideoLevelType aLevel;
    if (GetProfileAndLevel(aProfile, aLevel) == PVMFSuccess)
    {
        // For H263 this metadata will be available only after first frame decoding
        leavecode = 0;
        leavecode = Push_Back_MetadataKeys(PVOMXENCMETADATA_CODECINFO_VIDEO_PROFILE_KEY);
        if (0 == leavecode)
            leavecode = Push_Back_MetadataKeys(PVOMXENCMETADATA_CODECINFO_VIDEO_LEVEL_KEY);
    }
    if (0 == leavecode)
        leavecode = Push_Back_MetadataKeys(PVOMXENCMETADATA_CODECINFO_VIDEO_AVGBITRATE_KEY);

    if ((starting_index > (iAvailableMetadataKeys.size() - 1)) || max_entries == 0)
    {
        // Invalid starting index and/or max entries
        return PVMFErrArgument;
    }

    // Copy the requested keys
    uint32 num_entries = 0;
    int32 num_added = 0;
    for (uint32 lcv = 0; lcv < iAvailableMetadataKeys.size(); lcv++)
    {
        if (query_key == NULL)
        {
            // No query key so this key is counted
            ++num_entries;
            if (num_entries > starting_index)
            {
                // Past the starting index so copy the key
                leavecode = 0;
                leavecode = Push_Back_MetadataKeys(keylistptr, lcv);
                OSCL_FIRST_CATCH_ANY(leavecode,
                                     PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                                     (0, "PVMFOMXEncNode-%s::DoGetNodeMetadataKey() Memory allocation failure when copying metadata key", iNodeTypeId));
                                     return PVMFErrNoMemory);
                num_added++;
            }
        }
        else
        {
            // Check if the key matche the query key
            if (pv_mime_strcmp(iAvailableMetadataKeys[lcv].get_cstr(), query_key) >= 0)
            {
                // This key is counted
                ++num_entries;
                if (num_entries > starting_index)
                {
                    // Past the starting index so copy the key
                    leavecode = 0;
                    leavecode = Push_Back_MetadataKeys(keylistptr, lcv);
                    OSCL_FIRST_CATCH_ANY(leavecode,
                                         PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncNode-%s::DoGetNodeMetadataKey() Memory allocation failure when copying metadata key", iNodeTypeId));
                                         return PVMFErrNoMemory);
                    num_added++;
                }
            }
        }

        // Check if max number of entries have been copied
        if (max_entries > 0 && num_added >= max_entries)
        {
            break;
        }
    }

    return PVMFSuccess;
}

/////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFOMXEncNode::DoGetNodeMetadataValue(PVMFOMXEncNodeCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::DoGetNodeMetadataValue() In", iNodeTypeId));

    PVMFMetadataList* keylistptr = NULL;
    Oscl_Vector<PvmiKvp, OsclMemAllocator>* valuelistptr = NULL;
    uint32 starting_index;
    int32 max_entries;

    aCmd.PVMFOMXEncNodeCommand::Parse(keylistptr, valuelistptr, starting_index, max_entries);

    // Check the parameters
    if (keylistptr == NULL || valuelistptr == NULL)
    {
        return PVMFErrArgument;
    }

    uint32 numkeys = keylistptr->size();

    if (starting_index > (numkeys - 1) || numkeys <= 0 || max_entries == 0)
    {
        // Don't do anything
        return PVMFErrArgument;
    }

    uint32 numvalentries = 0;
    int32 numentriesadded = 0;
    for (uint32 lcv = 0; lcv < numkeys; lcv++)
    {
        int32 leavecode = 0;
        PvmiKvp KeyVal;
        KeyVal.key = NULL;
        uint32 KeyLen = 0;

        if ((oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVOMXENCMETADATA_CODECINFO_VIDEO_WIDTH_KEY) == 0) &&
                iYUVWidth > 0)
        {
            // Video width
            // Increment the counter for the number of values found so far
            ++numvalentries;

            // Create a value entry if past the starting index
            if (numvalentries > starting_index)
            {
                KeyLen = oscl_strlen(PVOMXENCMETADATA_CODECINFO_VIDEO_WIDTH_KEY) + 1; // for "codec-info/video/width;"
                KeyLen += oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR); // for "valtype="
                KeyLen += oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR) + 1; // for "uint32" and NULL terminator

                // Allocate memory for the string
                leavecode = 0;
                leavecode = CreateNewArray(KeyVal.key, KeyLen);



                if (leavecode == 0)
                {
                    // Copy the key string
                    oscl_strncpy(KeyVal.key, PVOMXENCMETADATA_CODECINFO_VIDEO_WIDTH_KEY, oscl_strlen(PVOMXENCMETADATA_CODECINFO_VIDEO_WIDTH_KEY) + 1);
                    oscl_strncat(KeyVal.key, PVOMXENCMETADATA_SEMICOLON, oscl_strlen(PVOMXENCMETADATA_SEMICOLON));
                    oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR));
                    oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR));
                    KeyVal.key[KeyLen-1] = NULL_TERM_CHAR;
                    // Copy the value
                    KeyVal.value.uint32_value = iYUVWidth;
                    // Set the length and capacity
                    KeyVal.length = 1;
                    KeyVal.capacity = 1;
                }
                else
                {
                    // Memory allocation failed
                    KeyVal.key = NULL;
                    break;
                }
            }
        }
        else if ((oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVOMXENCMETADATA_CODECINFO_VIDEO_HEIGHT_KEY) == 0) &&
                 iYUVHeight > 0)
        {
            // Video height
            // Increment the counter for the number of values found so far
            ++numvalentries;

            // Create a value entry if past the starting index
            if (numvalentries > starting_index)
            {
                KeyLen = oscl_strlen(PVOMXENCMETADATA_CODECINFO_VIDEO_HEIGHT_KEY) + 1; // for "codec-info/video/height;"
                KeyLen += oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR); // for "valtype="
                KeyLen += oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR) + 1; // for "uint32" and NULL terminator

                // Allocate memory for the string
                leavecode = 0;
                leavecode = CreateNewArray(KeyVal.key, KeyLen);



                if (leavecode == 0)
                {
                    // Copy the key string
                    oscl_strncpy(KeyVal.key, PVOMXENCMETADATA_CODECINFO_VIDEO_HEIGHT_KEY, oscl_strlen(PVOMXENCMETADATA_CODECINFO_VIDEO_HEIGHT_KEY) + 1);
                    oscl_strncat(KeyVal.key, PVOMXENCMETADATA_SEMICOLON, oscl_strlen(PVOMXENCMETADATA_SEMICOLON));
                    oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR));
                    oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR));
                    KeyVal.key[KeyLen-1] = NULL_TERM_CHAR;
                    // Copy the value
                    KeyVal.value.uint32_value = iYUVHeight;
                    // Set the length and capacity
                    KeyVal.length = 1;
                    KeyVal.capacity = 1;
                }
                else
                {
                    // Memory allocation failed
                    KeyVal.key = NULL;
                    break;
                }
            }
        }
        else if ((oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVOMXENCMETADATA_CODECINFO_VIDEO_PROFILE_KEY) == 0))
        {
            // Video profile
            // Increment the counter for the number of values found so far
            ++numvalentries;

            // Create a value entry if past the starting index
            if (numvalentries > starting_index)
            {
                KeyLen = oscl_strlen(PVOMXENCMETADATA_CODECINFO_VIDEO_PROFILE_KEY) + 1; // for "codec-info/video/profile;"
                KeyLen += oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR); // for "valtype="
                KeyLen += oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR) + 1; // for "uint32" and NULL terminator

                // Allocate memory for the string
                leavecode = 0;
                leavecode = CreateNewArray(KeyVal.key, KeyLen);



                if (leavecode == 0)
                {
                    PVMF_MPEGVideoProfileType aProfile;
                    PVMF_MPEGVideoLevelType aLevel;
                    if (GetProfileAndLevel(aProfile, aLevel) == PVMFSuccess)
                    {
                        // Copy the key string
                        oscl_strncpy(KeyVal.key, PVOMXENCMETADATA_CODECINFO_VIDEO_PROFILE_KEY, oscl_strlen(PVOMXENCMETADATA_CODECINFO_VIDEO_PROFILE_KEY) + 1);
                        oscl_strncat(KeyVal.key, PVOMXENCMETADATA_SEMICOLON, oscl_strlen(PVOMXENCMETADATA_SEMICOLON));
                        oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR));
                        oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR));
                        KeyVal.key[KeyLen-1] = NULL_TERM_CHAR;
                        // Copy the value
                        KeyVal.value.uint32_value = (uint32)aProfile; // This is to be decided, who will interpret these value
                        // Set the length and capacity
                        KeyVal.length = 1;
                        KeyVal.capacity = 1;
                    }
                    else
                    {
                        // Memory allocation failed
                        KeyVal.key = NULL;
                        break;
                    }
                }
                else
                {
                    // Memory allocation failed
                    KeyVal.key = NULL;
                    break;
                }
            }
        }
        else if ((oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVOMXENCMETADATA_CODECINFO_VIDEO_LEVEL_KEY) == 0))
        {
            // Video level
            // Increment the counter for the number of values found so far
            ++numvalentries;

            // Create a value entry if past the starting index
            if (numvalentries > starting_index)
            {
                KeyLen = oscl_strlen(PVOMXENCMETADATA_CODECINFO_VIDEO_LEVEL_KEY) + 1; // for "codec-info/video/level;"
                KeyLen += oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR); // for "valtype="
                KeyLen += oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR) + 1; // for "uint32" and NULL terminator

                // Allocate memory for the string
                leavecode = 0;
                leavecode = CreateNewArray(KeyVal.key, KeyLen);



                if (leavecode == 0)
                {
                    PVMF_MPEGVideoProfileType aProfile;
                    PVMF_MPEGVideoLevelType aLevel;
                    if (GetProfileAndLevel(aProfile, aLevel) == PVMFSuccess)
                    {
                        // Copy the key string
                        oscl_strncpy(KeyVal.key, PVOMXENCMETADATA_CODECINFO_VIDEO_LEVEL_KEY, oscl_strlen(PVOMXENCMETADATA_CODECINFO_VIDEO_LEVEL_KEY) + 1);
                        oscl_strncat(KeyVal.key, PVOMXENCMETADATA_SEMICOLON, oscl_strlen(PVOMXENCMETADATA_SEMICOLON));
                        oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR));
                        oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR));
                        KeyVal.key[KeyLen-1] = NULL_TERM_CHAR;
                        // Copy the value
                        KeyVal.value.uint32_value = (uint32)aLevel; // This is to be decided, who will interpret these value
                        // Set the length and capacity
                        KeyVal.length = 1;
                        KeyVal.capacity = 1;
                    }
                    else
                    {
                        // Memory allocation failed
                        KeyVal.key = NULL;
                        break;
                    }
                }
                else
                {
                    // Memory allocation failed
                    KeyVal.key = NULL;
                    break;
                }
            }
        }
        else if ((oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVOMXENCMETADATA_CODECINFO_VIDEO_AVGBITRATE_KEY) == 0) &&
                 (iAvgBitrateValue > 0))
        {
            // Video average bitrate
            // Increment the counter for the number of values found so far
            ++numvalentries;

            // Create a value entry if past the starting index
            if (numvalentries > starting_index)
            {
                KeyLen = oscl_strlen(PVOMXENCMETADATA_CODECINFO_VIDEO_AVGBITRATE_KEY) + 1; // for "codec-info/video/avgbitrate;"
                KeyLen += oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR); // for "valtype="
                KeyLen += oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR) + 1; // for "uint32" and NULL terminator

                // Allocate memory for the string
                leavecode = 0;
                leavecode = CreateNewArray(KeyVal.key, KeyLen);



                if (leavecode == 0)
                {
                    // Copy the key string
                    oscl_strncpy(KeyVal.key, PVOMXENCMETADATA_CODECINFO_VIDEO_AVGBITRATE_KEY, oscl_strlen(PVOMXENCMETADATA_CODECINFO_VIDEO_AVGBITRATE_KEY) + 1);
                    oscl_strncat(KeyVal.key, PVOMXENCMETADATA_SEMICOLON, oscl_strlen(PVOMXENCMETADATA_SEMICOLON));
                    oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR));
                    oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR));
                    KeyVal.key[KeyLen-1] = NULL_TERM_CHAR;
                    // Copy the value
                    KeyVal.value.uint32_value = iAvgBitrateValue;
                    // Set the length and capacity
                    KeyVal.length = 1;
                    KeyVal.capacity = 1;

                }
                else
                {
                    // Memory allocation failed
                    KeyVal.key = NULL;
                    break;
                }
            }
        }
        else if ((oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVOMXENCMETADATA_CODECINFO_VIDEO_FORMAT_KEY) == 0) &&
                 (((PVMFOMXEncPort*)iInPort)->iFormat == PVMF_MIME_H2631998 || ((PVMFOMXEncPort*)iInPort)->iFormat == PVMF_MIME_H2632000 || ((PVMFOMXEncPort*)iInPort)->iFormat == PVMF_MIME_M4V ||
                  ((PVMFOMXEncPort*)iInPort)->iFormat == PVMF_MIME_H264_VIDEO || ((PVMFOMXEncPort*)iInPort)->iFormat == PVMF_MIME_H264_VIDEO_MP4 ||
                  ((PVMFOMXEncPort*)iInPort)->iFormat == PVMF_MIME_H264_VIDEO_RAW  || ((PVMFOMXEncPort*)iInPort)->iFormat == PVMF_MIME_WMV))
        {
            // Format
            // Increment the counter for the number of values found so far
            ++numvalentries;

            // Create a value entry if past the starting index
            if (numvalentries > starting_index)
            {
                KeyLen = oscl_strlen(PVOMXENCMETADATA_CODECINFO_VIDEO_FORMAT_KEY) + 1; // for "codec-info/video/format;"
                KeyLen += oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR); // for "valtype="
                KeyLen += oscl_strlen(PVMI_KVPVALTYPE_CHARPTR_STRING_CONSTCHAR) + 1; // for "char*" and NULL terminator

                uint32 valuelen = 0;
                if (((PVMFOMXEncPort*)iInPort)->iFormat == PVMF_MIME_H264_VIDEO)
                {
                    valuelen = oscl_strlen(_STRLIT_CHAR(PVMF_MIME_H264_VIDEO)) + 1; // Value string plus one for NULL terminator
                }
                else if (((PVMFOMXEncPort*)iInPort)->iFormat == PVMF_MIME_H264_VIDEO_MP4)
                {
                    valuelen = oscl_strlen(_STRLIT_CHAR(PVMF_MIME_H264_VIDEO_MP4)) + 1; // Value string plus one for NULL terminator
                }
                else if (((PVMFOMXEncPort*)iInPort)->iFormat == PVMF_MIME_H264_VIDEO_RAW)
                {
                    valuelen = oscl_strlen(_STRLIT_CHAR(PVMF_MIME_H264_VIDEO_RAW)) + 1; // Value string plus one for NULL terminator
                }
                else if (((PVMFOMXEncPort*)iInPort)->iFormat == PVMF_MIME_M4V)
                {
                    valuelen = oscl_strlen(_STRLIT_CHAR(PVMF_MIME_M4V)) + 1; // Value string plus one for NULL terminator
                }
                else if (((PVMFOMXEncPort*)iInPort)->iFormat == PVMF_MIME_H2631998)
                {
                    valuelen = oscl_strlen(_STRLIT_CHAR(PVMF_MIME_H2631998)) + 1; // Value string plus one for NULL terminator
                }
                else if (((PVMFOMXEncPort*)iInPort)->iFormat == PVMF_MIME_H2632000)
                {
                    oscl_strncpy(KeyVal.value.pChar_value, _STRLIT_CHAR(PVMF_MIME_H2632000), valuelen);
                }
                else if (((PVMFOMXEncPort*)iInPort)->iFormat == PVMF_MIME_WMV)
                {
                    valuelen = oscl_strlen(_STRLIT_CHAR(PVMF_MIME_WMV)) + 1; // Value string plus one for NULL terminator
                }
                else
                {
                    // Should not enter here
                    OSCL_ASSERT(false);
                    valuelen = 1;
                }

                // Allocate memory for the strings
                leavecode = 0;
                leavecode = CreateNewArray(KeyVal.key, KeyLen);
                if (0 == leavecode)
                {
                    leavecode = CreateNewArray(KeyVal.value.pChar_value , valuelen);
                }

                if (leavecode == 0)
                {
                    // Copy the key string
                    oscl_strncpy(KeyVal.key, PVOMXENCMETADATA_CODECINFO_VIDEO_FORMAT_KEY, oscl_strlen(PVOMXENCMETADATA_CODECINFO_VIDEO_FORMAT_KEY) + 1);
                    oscl_strncat(KeyVal.key, PVOMXENCMETADATA_SEMICOLON, oscl_strlen(PVOMXENCMETADATA_SEMICOLON));
                    oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR));
                    oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_CHARPTR_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_CHARPTR_STRING_CONSTCHAR));
                    KeyVal.key[KeyLen-1] = NULL_TERM_CHAR;
                    // Copy the value
                    if (((PVMFOMXEncPort*)iInPort)->iFormat == PVMF_MIME_H264_VIDEO)
                    {
                        oscl_strncpy(KeyVal.value.pChar_value, _STRLIT_CHAR(PVMF_MIME_H264_VIDEO), valuelen);
                    }
                    else if (((PVMFOMXEncPort*)iInPort)->iFormat == PVMF_MIME_H264_VIDEO_MP4)
                    {
                        oscl_strncpy(KeyVal.value.pChar_value, _STRLIT_CHAR(PVMF_MIME_H264_VIDEO_MP4), valuelen);
                    }
                    else if (((PVMFOMXEncPort*)iInPort)->iFormat == PVMF_MIME_H264_VIDEO_RAW)
                    {
                        oscl_strncpy(KeyVal.value.pChar_value, _STRLIT_CHAR(PVMF_MIME_H264_VIDEO_RAW), valuelen);
                    }
                    else if (((PVMFOMXEncPort*)iInPort)->iFormat == PVMF_MIME_M4V)
                    {
                        oscl_strncpy(KeyVal.value.pChar_value, _STRLIT_CHAR(PVMF_MIME_M4V), valuelen);
                    }
                    else if (((PVMFOMXEncPort*)iInPort)->iFormat == PVMF_MIME_H2631998)
                    {
                        oscl_strncpy(KeyVal.value.pChar_value, _STRLIT_CHAR(PVMF_MIME_H2631998), valuelen);
                    }
                    else if (((PVMFOMXEncPort*)iInPort)->iFormat == PVMF_MIME_H2632000)
                    {
                        oscl_strncpy(KeyVal.value.pChar_value, _STRLIT_CHAR(PVMF_MIME_H2632000), valuelen);
                    }
                    else if (((PVMFOMXEncPort*)iInPort)->iFormat == PVMF_MIME_WMV)
                    {
                        oscl_strncpy(KeyVal.value.pChar_value, _STRLIT_CHAR(PVMF_MIME_WMV), valuelen);
                    }
                    else
                    {
                        // Should not enter here
                        OSCL_ASSERT(false);
                        valuelen = 1;
                    }

                    KeyVal.value.pChar_value[valuelen-1] = NULL_TERM_CHAR;
                    // Set the length and capacity
                    KeyVal.length = valuelen;
                    KeyVal.capacity = valuelen;
                }
                else
                {
                    // Memory allocation failed so clean up
                    if (KeyVal.key)
                    {
                        OSCL_ARRAY_DELETE(KeyVal.key);
                        KeyVal.key = NULL;
                    }
                    if (KeyVal.value.pChar_value)
                    {
                        OSCL_ARRAY_DELETE(KeyVal.value.pChar_value);
                    }
                    break;
                }
            }
        }

        if (KeyVal.key != NULL)
        {
            leavecode = 0;
            leavecode = PushBackKeyVal(valuelistptr, KeyVal);
            if (leavecode != 0)
            {
                switch (GetValTypeFromKeyString(KeyVal.key))
                {
                    case PVMI_KVPVALTYPE_CHARPTR:
                        if (KeyVal.value.pChar_value != NULL)
                        {
                            OSCL_ARRAY_DELETE(KeyVal.value.pChar_value);
                            KeyVal.value.pChar_value = NULL;
                        }
                        break;

                    default:
                        // Add more case statements if other value types are returned
                        break;
                }

                OSCL_ARRAY_DELETE(KeyVal.key);
                KeyVal.key = NULL;
            }
            else
            {
                // Increment the counter for number of value entries added to the list
                ++numentriesadded;
            }

            // Check if the max number of value entries were added
            if (max_entries > 0 && numentriesadded >= max_entries)
            {
                break;
            }
        }
    }

    return PVMFSuccess;
}

/////////////////////////////////////////////////////////////////////////////
bool PVMFOMXEncNode::ReleaseAllPorts()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::ReleaseAllPorts() In", iNodeTypeId));

    if (iInPort)
    {
        iInPort->ClearMsgQueues();
        iInPort->Disconnect();
        OSCL_DELETE(((PVMFOMXEncPort*)iInPort));
        iInPort = NULL;
    }

    if (iOutPort)
    {
        iOutPort->ClearMsgQueues();
        iOutPort->Disconnect();
        OSCL_DELETE(((PVMFOMXEncPort*)iOutPort));
        iOutPort = NULL;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// Clean Up Encoder
/////////////////////////////////////////////////////////////////////////////
bool PVMFOMXEncNode::DeleteOMXEncoder()
{
    OMX_ERRORTYPE  err;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXEncNode-%s::DeleteOMXEncoder() In", iNodeTypeId));

    if (iOMXEncoder != NULL)
    {
        /* Free Component handle. */
        err = OMX_FreeHandle(iOMXEncoder);
        if (err != OMX_ErrorNone)
        {
            //Error condition report
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFOMXEncNode-%s::DeleteOMXEncoder(): Can't free encoder's handle!", iNodeTypeId));
        }
        iOMXEncoder = NULL;

    }//end of if (iOMXEncoder != NULL)


    return true;
}

/////////////////////////////////////////////////////////////////////////////
void PVMFOMXEncNode::ChangeNodeState(TPVMFNodeInterfaceState aNewState)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::ChangeNodeState() Changing state from %d to %d", iNodeTypeId, iInterfaceState, aNewState));
    iInterfaceState = aNewState;
}

/////////////////////////////////////////////////////////////////////////////
void PVMFOMXEncNode::freechunkavailable(OsclAny *aContext)
{

    // check context to see whether input or output buffer was returned to the mempool
    if (aContext == (OsclAny *) iInBufMemoryPool)
    {

        iNumOutstandingInputBuffers--;

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::freechunkavailable() Memory chunk in INPUT mempool was deallocated, %d out of %d now available", iNodeTypeId, iNumInputBuffers - iNumOutstandingInputBuffers, iNumInputBuffers));

        // notification only works once.
        // If there are multiple buffers coming back in a row, make sure to set the notification
        // flag in the mempool again, so that next buffer also causes notification
        iInBufMemoryPool->notifyfreechunkavailable(*this, aContext);

    }
    else if (aContext == (OsclAny *) iOutBufMemoryPool)
    {

        iNumOutstandingOutputBuffers--;
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::freechunkavailable() Memory chunk in OUTPUT mempool was deallocated, %d out of %d now available", iNodeTypeId, iNumOutputBuffers - iNumOutstandingOutputBuffers, iNumOutputBuffers));

        // notification only works once.
        // If there are multiple buffers coming back in a row, make sure to set the notification
        // flag in the mempool again, so that next buffer also causes notification
        iOutBufMemoryPool->notifyfreechunkavailable(*this, aContext);

    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::freechunkavailable() UNKNOWN mempool ", iNodeTypeId));

    }

    // reschedule
    if (IsAdded())
        RunIfNotReady();


}

/////////////////////////////////////////////////////////////////////////////
void PVMFOMXEncNode::HandlePortActivity(const PVMFPortActivity &aActivity)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "0x%x PVMFOMXEncNode-%s::PortActivity: port=0x%x, type=%d",
                     this, iNodeTypeId, aActivity.iPort, aActivity.iType));

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
                            (0, "PVMFOMXEncNode-%s::PortActivity: IncomingMsgQueueSize=%d", iNodeTypeId, aActivity.iPort->IncomingMsgQueueSize()));
            if (aActivity.iPort->IncomingMsgQueueSize() == 1)
            {
                //wake up the AO to process the port activity event.
                RunIfNotReady();
            }
            break;

        case PVMF_PORT_ACTIVITY_OUTGOING_QUEUE_READY:
            if (iProcessingState == EPVMFOMXEncNodeProcessingState_WaitForOutgoingQueue)
            {
                iProcessingState = EPVMFOMXEncNodeProcessingState_ReadyToEncode;
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

            //iProcessingState should transition from WaitForOutputPort to ReadyToEncode
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "0x%x PVMFOMXEncNode-%s::PortActivity: Connected port is now ready", this, iNodeTypeId));
            RunIfNotReady();
            break;

        default:
            break;
    }
}

/////////////////////////////////////////////////////////////////////////////
void PVMFOMXEncNode::DoCancelAllCommands(PVMFOMXEncNodeCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXEncNode-%s::DoCancelAllCommands", iNodeTypeId));

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
void PVMFOMXEncNode::DoCancelCommand(PVMFOMXEncNodeCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXEncNode-%s::DoCancelCommand", iNodeTypeId));

    //extract the command ID from the parameters.
    PVMFCommandId id;
    aCmd.PVMFOMXEncNodeCommandBase::Parse(id);

    //first check "current" command if any
    {
        PVMFOMXEncNodeCommand* cmd = iCurrentCommand.FindById(id);
        if (cmd)
        {

            // if reset is being canceled:
            if (cmd->iCmd == PVMFOMXEncNodeCommand::PVOMXENC_NODE_CMD_RESET)
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
        PVMFOMXEncNodeCommand* cmd = iInputCommands.FindById(id, 1);
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
void PVMFOMXEncNode::DoQueryUuid(PVMFOMXEncNodeCommand& aCmd)
{
    //This node supports Query UUID from any state

    OSCL_String* mimetype;
    Oscl_Vector<PVUuid, OsclMemAllocator> *uuidvec;
    bool exactmatch;
    aCmd.PVMFOMXEncNodeCommandBase::Parse(mimetype, uuidvec, exactmatch);

    //Try to match the input mimetype against any of
    //the custom interfaces for this node

    //Match against custom interface1...
    if (*mimetype == PVMF_OMX_ENC_NODE_CUSTOM1_MIMETYPE
            //also match against base mimetypes for custom interface1,
            //unless exactmatch is set.
            || (!exactmatch && *mimetype == PVMF_OMX_ENC_NODE_MIMETYPE)
            || (!exactmatch && *mimetype == PVMF_BASEMIMETYPE))
    {

        PVUuid uuid(PVMF_OMX_ENC_NODE_CUSTOM1_UUID);
        uuidvec->push_back(uuid);
    }
    CommandComplete(iInputCommands, aCmd, PVMFSuccess);
}

/////////////////////////////////////////////////////////////////////////////
void PVMFOMXEncNode::DoQueryInterface(PVMFOMXEncNodeCommand&  aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXEncNode-%s::DoQueryInterface", iNodeTypeId));
    PVUuid* uuid;
    PVInterface** ptr;
    aCmd.PVMFOMXEncNodeCommandBase::Parse(uuid, ptr);

    if (*uuid == PVUuid(PVMF_OMX_ENC_NODE_CUSTOM1_UUID))
    {
        addRef();
        *ptr = (PVMFOMXEncNodeExtensionInterface*)this;
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
    else if (*uuid == PVMp4H263EncExtensionUUID)
    {
        addRef();
        *ptr = OSCL_STATIC_CAST(PVMp4H263EncExtensionInterface*, this);
        CommandComplete(iInputCommands, aCmd, PVMFSuccess);
        //iface = OSCL_STATIC_CAST(PVInterface*, myInterface);

    }
    else if (*uuid == PVAudioEncExtensionUUID)
    {
        addRef();
        *ptr = OSCL_STATIC_CAST(PVAudioEncExtensionInterface*, this);
        //iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
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
void PVMFOMXEncNode::addRef()
{
    ++iExtensionRefCount;
}

/////////////////////////////////////////////////////////////////////////////
void PVMFOMXEncNode::removeRef()
{
    --iExtensionRefCount;
}

/////////////////////////////////////////////////////////////////////////////
bool PVMFOMXEncNode::queryInterface(const PVUuid& uuid, PVInterface*& iface)
{
    PVUuid my_uuid(PVMF_OMX_ENC_NODE_CUSTOM1_UUID);
    if (uuid == my_uuid)
    {
        PVMFOMXEncNodeExtensionInterface* myInterface = OSCL_STATIC_CAST(PVMFOMXEncNodeExtensionInterface*, this);
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

/////////////////////////////////////////////////////////////////////////////
uint32 PVMFOMXEncNode::GetNumMetadataKeys(char* aQueryKeyString)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::GetNumMetadataKeys() called", iNodeTypeId));

    // Update the available metadata keys
    iAvailableMetadataKeys.clear();
    int32 errcode = 0;
    OSCL_TRY(errcode, iAvailableMetadataKeys.push_back(PVOMXENCMETADATA_CODECINFO_VIDEO_FORMAT_KEY));

    if (iYUVWidth > 0 && iYUVHeight > 0)
    {
        errcode = 0;
        OSCL_TRY(errcode,
                 iAvailableMetadataKeys.push_back(PVOMXENCMETADATA_CODECINFO_VIDEO_WIDTH_KEY);
                 iAvailableMetadataKeys.push_back(PVOMXENCMETADATA_CODECINFO_VIDEO_HEIGHT_KEY));
    }
    // add the profile, level and avgbitrate
    PVMF_MPEGVideoProfileType aProfile;
    PVMF_MPEGVideoLevelType aLevel;
    if (GetProfileAndLevel(aProfile, aLevel) == PVMFSuccess)
    {
        // For H263 this metadata will be available only after first frame decoding
        errcode = 0;
        OSCL_TRY(errcode, iAvailableMetadataKeys.push_back(PVOMXENCMETADATA_CODECINFO_VIDEO_PROFILE_KEY));
        errcode = 0;
        OSCL_TRY(errcode, iAvailableMetadataKeys.push_back(PVOMXENCMETADATA_CODECINFO_VIDEO_LEVEL_KEY));
    }
    errcode = 0;
    OSCL_TRY(errcode, iAvailableMetadataKeys.push_back(PVOMXENCMETADATA_CODECINFO_VIDEO_AVGBITRATE_KEY));



    uint32 num_entries = 0;

    if (aQueryKeyString == NULL)
    {
        num_entries = iAvailableMetadataKeys.size();
    }
    else
    {
        for (uint32 i = 0; i < iAvailableMetadataKeys.size(); i++)
        {
            if (pv_mime_strcmp(iAvailableMetadataKeys[i].get_cstr(), aQueryKeyString) >= 0)
            {
                num_entries++;
            }
        }
    }
    return num_entries; // Number of elements
}

///////////////////////////////////////////////////////////////////////////////////////
void PVMFOMXEncNode::setObserver(PvmiConfigAndCapabilityCmdObserver* aObserver)
{
    OSCL_UNUSED_ARG(aObserver);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::setObserver()", iNodeTypeId));
    // This method is not supported so leave
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncNode-%s::setObserver() is not supported!", iNodeTypeId));
    OSCL_LEAVE(PVMFErrNotSupported);
}


PVMFStatus PVMFOMXEncNode::getParametersSync(PvmiMIOSession aSession, PvmiKeyType aIdentifier, PvmiKvp*& aParameters, int& aNumParamElements, PvmiCapabilityContext aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::getParametersSync()", iNodeTypeId));
    OSCL_UNUSED_ARG(aSession);

    return DoCapConfigGetParametersSync(aIdentifier, aParameters, aNumParamElements, aContext);
}


PVMFStatus PVMFOMXEncNode::releaseParameters(PvmiMIOSession aSession, PvmiKvp* aParameters, int aNumElements)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::releaseParameters()", iNodeTypeId));
    OSCL_UNUSED_ARG(aSession);

    return DoCapConfigReleaseParameters(aParameters, aNumElements);
}


void PVMFOMXEncNode::createContext(PvmiMIOSession aSession, PvmiCapabilityContext& aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::createContext()", iNodeTypeId));
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aContext);
    // This method is not supported so leave
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncNode-%s::createContext() is not supported!", iNodeTypeId));
    OSCL_LEAVE(PVMFErrNotSupported);
}


void PVMFOMXEncNode::setContextParameters(PvmiMIOSession aSession, PvmiCapabilityContext& aContext, PvmiKvp* aParameters, int aNumParamElements)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::setContextParameters()", iNodeTypeId));
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aContext);
    OSCL_UNUSED_ARG(aParameters);
    OSCL_UNUSED_ARG(aNumParamElements);
    // This method is not supported so leave
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncNode-%s::setContextParameters() is not supported!", iNodeTypeId));
    OSCL_LEAVE(PVMFErrNotSupported);
}


void PVMFOMXEncNode::DeleteContext(PvmiMIOSession aSession, PvmiCapabilityContext& aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::DeleteContext()", iNodeTypeId));
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aContext);
    // This method is not supported so leave
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncNode-%s::DeleteContext() is not supported!", iNodeTypeId));
    OSCL_LEAVE(PVMFErrNotSupported);
}


void PVMFOMXEncNode::setParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters, int aNumElements, PvmiKvp* &aRetKVP)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::setParametersSync()", iNodeTypeId));
    OSCL_UNUSED_ARG(aSession);

    // Complete the request synchronously
    DoCapConfigSetParameters(aParameters, aNumElements, aRetKVP);
}


PVMFCommandId PVMFOMXEncNode::setParametersAsync(PvmiMIOSession aSession, PvmiKvp* aParameters, int aNumElements, PvmiKvp*& aRetKVP, OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::setParametersAsync()", iNodeTypeId));
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aParameters);
    OSCL_UNUSED_ARG(aNumElements);
    OSCL_UNUSED_ARG(aRetKVP);
    OSCL_UNUSED_ARG(aContext);

    // This method is not supported so leave
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncNode-%s::setParametersAsync() is not supported!", iNodeTypeId));
    OSCL_LEAVE(PVMFErrNotSupported);
    return 0;
}


uint32 PVMFOMXEncNode::getCapabilityMetric(PvmiMIOSession aSession)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::getCapabilityMetric()", iNodeTypeId));
    OSCL_UNUSED_ARG(aSession);
    // Not supported so return 0
    return 0;
}


PVMFStatus PVMFOMXEncNode::verifyParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters, int aNumElements)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::verifyParametersSync()", iNodeTypeId));
    OSCL_UNUSED_ARG(aSession);

    return DoCapConfigVerifyParameters(aParameters, aNumElements);
}


/////////////////////////////////////////////////////////////////////////////
uint32 PVMFOMXEncNode::GetNumMetadataValues(PVMFMetadataList& aKeyList)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::GetNumMetadataValues() called", iNodeTypeId));

    uint32 numkeys = aKeyList.size();

    if (numkeys <= 0)
    {
        // Don't do anything
        return 0;
    }

    // Count the number of value entries for the provided key list
    uint32 numvalentries = 0;
    PVMF_MPEGVideoProfileType aProfile;
    PVMF_MPEGVideoLevelType aLevel;
    for (uint32 lcv = 0; lcv < numkeys; lcv++)
    {
        if ((oscl_strcmp(aKeyList[lcv].get_cstr(), PVOMXENCMETADATA_CODECINFO_VIDEO_WIDTH_KEY) == 0) &&
                iYUVWidth > 0)
        {
            // Video width
            ++numvalentries;
        }
        else if ((oscl_strcmp(aKeyList[lcv].get_cstr(), PVOMXENCMETADATA_CODECINFO_VIDEO_HEIGHT_KEY) == 0) &&
                 iYUVHeight > 0)
        {
            // Video height
            ++numvalentries;
        }
        else if ((oscl_strcmp(aKeyList[lcv].get_cstr(), PVOMXENCMETADATA_CODECINFO_VIDEO_PROFILE_KEY) == 0) &&
                 (GetProfileAndLevel(aProfile, aLevel) == PVMFSuccess))

        {
            // Video profile
            ++numvalentries;
        }
        else if ((oscl_strcmp(aKeyList[lcv].get_cstr(), PVOMXENCMETADATA_CODECINFO_VIDEO_LEVEL_KEY) == 0) &&
                 (GetProfileAndLevel(aProfile, aLevel) == PVMFSuccess))
        {
            // Video level
            ++numvalentries;
        }
        else if ((oscl_strcmp(aKeyList[lcv].get_cstr(), PVOMXENCMETADATA_CODECINFO_VIDEO_AVGBITRATE_KEY) == 0) &&
                 (iAvgBitrateValue > 0))

        {
            // Video average bitrate
            if (iAvgBitrateValue > 0)
                ++numvalentries;
        }
        else if ((oscl_strcmp(aKeyList[lcv].get_cstr(), PVOMXENCMETADATA_CODECINFO_VIDEO_FORMAT_KEY) == 0) &&
                 (((PVMFOMXEncPort*)iInPort)->iFormat == PVMF_MIME_WMV || ((PVMFOMXEncPort*)iInPort)->iFormat == PVMF_MIME_M4V || ((PVMFOMXEncPort*)iInPort)->iFormat == PVMF_MIME_H2631998 || ((PVMFOMXEncPort*)iInPort)->iFormat == PVMF_MIME_H2632000 || ((PVMFOMXEncPort*)iInPort)->iFormat == PVMF_MIME_H264_VIDEO || ((PVMFOMXEncPort*)iInPort)->iFormat == PVMF_MIME_H264_VIDEO_MP4 || ((PVMFOMXEncPort*)iInPort)->iFormat == PVMF_MIME_H264_VIDEO_RAW))
        {
            // Format
            ++numvalentries;
        }
    }

    return numvalentries;
}

/////////////////////////////////////////////////////////////////////////////
PVMFCommandId PVMFOMXEncNode::GetNodeMetadataKeys(PVMFSessionId aSessionId, PVMFMetadataList& aKeyList, uint32 starting_index, int32 max_entries, char* query_key, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNodeCommand::GetNodeMetadataKeys() called", iNodeTypeId));

    PVMFOMXEncNodeCommand cmd;
    cmd.PVMFOMXEncNodeCommand::Construct(aSessionId, PVMFOMXEncNodeCommand::PVOMXENC_NODE_CMD_GETNODEMETADATAKEY, &aKeyList, starting_index, max_entries, query_key, aContext);
    return QueueCommandL(cmd);
}

/////////////////////////////////////////////////////////////////////////////
PVMFCommandId PVMFOMXEncNode::GetNodeMetadataValues(PVMFSessionId aSessionId, PVMFMetadataList& aKeyList, Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList, uint32 starting_index, int32 max_entries, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNodeCommand::GetNodeMetadataValue() called", iNodeTypeId));

    PVMFOMXEncNodeCommand cmd;
    cmd.PVMFOMXEncNodeCommand::Construct(aSessionId, PVMFOMXEncNodeCommand::PVOMXENC_NODE_CMD_GETNODEMETADATAVALUE, &aKeyList, &aValueList, starting_index, max_entries, aContext);
    return QueueCommandL(cmd);
}

// From PVMFMetadataExtensionInterface
/////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFOMXEncNode::ReleaseNodeMetadataKeys(PVMFMetadataList& , uint32 , uint32)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::ReleaseNodeMetadataKeys() called", iNodeTypeId));
    //nothing needed-- there's no dynamic allocation in this node's key list
    return PVMFSuccess;
}

// From PVMFMetadataExtensionInterface
/////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFOMXEncNode::ReleaseNodeMetadataValues(Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList, uint32 start, uint32 end)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::ReleaseNodeMetadataValues() called", iNodeTypeId));

    if (aValueList.size() == 0 || start > end)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncNode-%s::ReleaseNodeMetadataValues() Invalid start/end index", iNodeTypeId));
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






PVMFStatus PVMFOMXEncNode::GetProfileAndLevel(PVMF_MPEGVideoProfileType& aProfile, PVMF_MPEGVideoLevelType& aLevel)
{

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::GetProfileAndLevel() In", iNodeTypeId));

    if (NULL == iOMXEncoder)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncNode-%s::GetProfileAndLevel() iEncoder is Null", iNodeTypeId));
        aProfile = PV_MPEG_VIDEO_RESERVED_PROFILE;
        aLevel	= PV_MPEG_VIDEO_LEVEL_UNKNOWN;
        return PVMFFailure;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncNode-%s::GetProfileAndLevel() iEncoder is Null", iNodeTypeId));
    aProfile = PV_MPEG_VIDEO_RESERVED_PROFILE;
    aLevel	= PV_MPEG_VIDEO_LEVEL_UNKNOWN;
    // DV: FOR NOW, JUST RETURN FAILURE, WE DON'T SUPPORT THIS FEATURE YET
    return PVMFFailure;


}


/////////////////////////////////////////////////////////////////////////////
void PVMFOMXEncNode::LogDiagnostics()
{
    if (iDiagnosticsLogged == false)
    {
        iDiagnosticsLogged = true;
        PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iDiagnosticsLogger, PVLOGMSG_INFO, (0, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"));
        PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iDiagnosticsLogger, PVLOGMSG_INFO, (0, "PVMFOMXEncNode-%s - Number of Media Msgs Sent = %d", iNodeTypeId, iSeqNum));
        PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iDiagnosticsLogger, PVLOGMSG_INFO, (0, "PVMFOMXEncNode-%s - TS of last encoded msg = %d", iNodeTypeId, iTimeStampOut));
    }
}



//////////////////////////////////
/////EXTENSION INTERFACE
////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF bool PVMFOMXEncNode::SetNumLayers(uint32 aNumLayers)
{
    switch (iInterfaceState)
    {
        case EPVMFNodeStarted:
        case EPVMFNodePaused:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFOMXEncNode-%s::SetNumLayers: Error iInterfaceState=%d", iNodeTypeId, iInterfaceState));
            return false;

        default:
            break;
    }

    if (aNumLayers > MAX_LAYER)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVMFOMXEncNode-%s::SetNumLayers: Error Max num layers is %d", iNodeTypeId, MAX_LAYER));
        return false;
    }

    iVideoEncodeParam.iNumLayer = aNumLayers;
    return true;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF bool PVMFOMXEncNode::SetOutputBitRate(uint32 aLayer, uint32 aBitRate)
{
    switch (iInterfaceState)
    {
        case EPVMFNodeStarted:
        case EPVMFNodePaused:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncNode-%s::SetOutputBitRate: Error iInterfaceState=%d", iInterfaceState));
            return false;

        default:
            break;
    }

    if ((int32)aLayer >= iVideoEncodeParam.iNumLayer)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVMFOMXEncNode-%s::SetOutputBitRate: Error Invalid layer number", iNodeTypeId));
        return false;
    }

    iVideoEncodeParam.iBitRate[aLayer] = aBitRate;
    return true;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF bool PVMFOMXEncNode::SetOutputFrameSize(uint32 aLayer, uint32 aWidth, uint32 aHeight)
{
    switch (iInterfaceState)
    {
        case EPVMFNodeStarted:
        case EPVMFNodePaused:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFOMXEncNode-%s::SetOutputFrameSize: Error iInterfaceState=%d", iNodeTypeId, iInterfaceState));
            return false;

        default:
            break;
    }

    if ((int32)aLayer >= iVideoEncodeParam.iNumLayer)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVMFOMXEncNode-%s::SetOutputFrameSize: Error Invalid layer number", iNodeTypeId));
        return false;
    }

    iVideoEncodeParam.iFrameWidth[aLayer] = aWidth;
    iVideoEncodeParam.iFrameHeight[aLayer] = aHeight;
    return true;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF bool PVMFOMXEncNode::SetOutputFrameRate(uint32 aLayer, OsclFloat aFrameRate)
{
    switch (iInterfaceState)
    {
        case EPVMFNodeStarted:
        case EPVMFNodePaused:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFOMXEncNode-%s::SetOutputFrameRate: Error iInterfaceState=%d", iInterfaceState));
            return false;

        default:
            break;
    }

    if ((int32)aLayer >= iVideoEncodeParam.iNumLayer)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVMFOMXEncNode-%s::SetOutputFrameRate: Error Invalid layer number", iNodeTypeId));
        return false;
    }

    iVideoEncodeParam.iFrameRate[aLayer] = OSCL_STATIC_CAST(float, aFrameRate);
    return true;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF bool PVMFOMXEncNode::SetSegmentTargetSize(uint32 aLayer, uint32 aSizeBytes)
{
    OSCL_UNUSED_ARG(aLayer);

    switch (iInterfaceState)
    {
        case EPVMFNodeStarted:
        case EPVMFNodePaused:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFOMXEncNode-%s::SetSegmentTargetSize: Error iInterfaceState=%d", iNodeTypeId, iInterfaceState));
            return false;

        default:
            break;
    }

    iVideoEncodeParam.iPacketSize = aSizeBytes;
    return true;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF bool PVMFOMXEncNode::SetRateControlType(uint32 aLayer, PVMFVENRateControlType aRateControl)
{
    OSCL_UNUSED_ARG(aLayer);

    switch (iInterfaceState)
    {
        case EPVMFNodeStarted:
        case EPVMFNodePaused:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFOMXEncNode-%s::SetRateControlType: Error iInterfaceState=%d", iNodeTypeId, iInterfaceState));
            return false;

        default:
            break;
    }

    switch (aRateControl)
    {
        case PVMFVEN_RATE_CONTROL_CONSTANT_Q:
            iVideoEncodeParam.iRateControlType = ECONSTANT_Q;
            break;
        case PVMFVEN_RATE_CONTROL_CBR:
            iVideoEncodeParam.iRateControlType = ECBR_1;
            break;
        case PVMFVEN_RATE_CONTROL_VBR:
            iVideoEncodeParam.iRateControlType = EVBR_1;
            break;
        default:
            return false;
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF bool PVMFOMXEncNode::SetDataPartitioning(bool aDataPartitioning)
{
    switch (iInterfaceState)
    {
        case EPVMFNodeStarted:
        case EPVMFNodePaused:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFOMXEncNode-%s::SetDataPartitioning: Error iInterfaceState=%d", iNodeTypeId, iInterfaceState));
            return false;

        default:
            break;
    }

    if (iVideoEncodeParam.iContentType == EI_H263 || iVideoEncodeParam.iContentType == EI_H264)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVMFOMXEncNode-%s::SetDataPartitioning: Error data partitioning not supported for H263 or H264", iNodeTypeId));

        // ignore the error
        return true;
        //return false;
    }

    if (aDataPartitioning)
        iVideoEncodeParam.iContentType = EI_M4V_STREAMING;
    else
        iVideoEncodeParam.iContentType = EI_M4V_DOWNLOAD;

    return true;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF bool PVMFOMXEncNode::SetIFrameInterval(uint32 aIFrameInterval)
{
    switch (iInterfaceState)
    {
        case EPVMFNodeStarted:
        case EPVMFNodePaused:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFOMXEncNode-%s::SetIFrameInterval: Error iInterfaceState=%d", iNodeTypeId, iInterfaceState));
            return false;

        default:
            break;
    }

    iVideoEncodeParam.iIFrameInterval = aIFrameInterval;
    return true;
}
////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF bool PVMFOMXEncNode::SetSceneDetection(bool aSCD)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXEncNode-%s::SetSceneDetection to %d", iNodeTypeId, aSCD));


    switch (iInterfaceState)
    {
        case EPVMFNodeStarted:
        case EPVMFNodePaused:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFOMXEncEncNode-%s::SetIFrameInterval: Error iInterfaceState=%d", iNodeTypeId, iInterfaceState));
            return false;

        default:
            break;
    }

    iVideoEncodeParam.iSceneDetection = aSCD;

    return true;
}
////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF bool PVMFOMXEncNode::SetRVLC(bool aRVLC)
{

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXEncNode-%s::SetRVLC", iNodeTypeId));

    switch (iInterfaceState)
    {
        case EPVMFNodeStarted:
        case EPVMFNodePaused:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFOMXEncEncNode-%s::SetRVLC: Error iInterfaceState=%d", iNodeTypeId, iInterfaceState));
            return false;

        default:
            break;
    }

    if (iVideoEncodeParam.iContentType == EI_H263 || iVideoEncodeParam.iContentType == EI_H264)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVMFOMXEncNode-%s::SetRVLC : RVLC not supported for H263 or H264", iNodeTypeId));

        // ignore the error
        return true;
    }

    iVideoEncodeParam.iRVLCEnable = aRVLC;
    return true;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF bool PVMFOMXEncNode::GetVolHeader(OsclRefCounterMemFrag& aVolHeader)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXEncNode-%s::GetVolHeader", iNodeTypeId));

#ifdef _TEST_AE_ERROR_HANDLING
    if (iErrorConfigHeader)
    {
        iInterfaceState = EPVMFNodeError;
    }
#endif
    switch (iInterfaceState)
    {
        case EPVMFNodeInitialized:
        case EPVMFNodePrepared:
        case EPVMFNodeStarted:
        case EPVMFNodePaused:
            break;

        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFOMXEncNode-%s::GetVolHeader: Error - Wrong state", iNodeTypeId));
            return false;
    }

    if ((iVideoEncodeParam.iContentType == EI_H263) || (iVideoEncodeParam.iContentType == EI_H264))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVMFOMXEncNode-%s::GetVolHeader: Error - VOL header only for M4V encode", iNodeTypeId));
        return false;
    }

    uint8 *ptr = (uint8 *) iVolHeader.getMemFragPtr();
    //If data partioning mode
    if (iVideoEncodeParam.iContentType == EI_M4V_STREAMING)
    {
        ptr[iVolHeader.getMemFragSize() - 1] = 0x8F;
    }
    //else combined mode
    else if (iVideoEncodeParam.iContentType == EI_M4V_DOWNLOAD)
    {
        ptr[iVolHeader.getMemFragSize() - 1] = 0x1F;
    }

    aVolHeader = iVolHeader;

    return true;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF bool PVMFOMXEncNode::RequestIFrame()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXEncNode-%s::RequestIFrame", iNodeTypeId));

    switch (iInterfaceState)
    {
        case EPVMFNodeStarted:
        case EPVMFNodePaused:
            break;
        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFOMXEncNode-%s::RequestIFrame: Error - Wrong state", iNodeTypeId));
            return false;
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF bool PVMFOMXEncNode::SetCodec(PVMFFormatType aCodec)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXEncNode-%s::SetCodec %s", iNodeTypeId, aCodec.getMIMEStrPtr()));

    if (SetCodecType(aCodec) == PVMFSuccess)
    {
        return true;
    }
    else
    {
        return false;
    }

}



PVMFStatus PVMFOMXEncNode::SetCodecType(PVMFFormatType aCodec)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXEncNode-%s::SetCodecType: aCodec=%s", iNodeTypeId, aCodec.getMIMEStrPtr()));

    switch (iInterfaceState)
    {
        case EPVMFNodeStarted:
        case EPVMFNodePaused:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFOMXEncNode-%s::SetCodecType: Error iInterfaceState=%d", iNodeTypeId, iInterfaceState));

            return PVMFErrInvalidState;
        default:
            break;
    }

    if (aCodec == PVMF_MIME_H2631998)
    {
        iVideoEncodeParam.iContentType = EI_H263;
        iOutFormat = PVMF_MIME_H2631998;
    }
    else if (aCodec == PVMF_MIME_H2632000)
    {
        iVideoEncodeParam.iContentType = EI_H263;
        iOutFormat = PVMF_MIME_H2632000;
    }
    else if (aCodec == PVMF_MIME_M4V)
    {
        iVideoEncodeParam.iContentType = EI_M4V_STREAMING;
        iOutFormat = PVMF_MIME_M4V;
    }
    else if (aCodec == PVMF_MIME_H264_VIDEO_RAW ||
             aCodec == PVMF_MIME_H264_VIDEO_MP4)
    {
        iVideoEncodeParam.iContentType = EI_H264;
        iOutFormat = aCodec;
    }
    else if (aCodec == PVMF_MIME_AMR_IETF ||
             aCodec == PVMF_MIME_AMR_IF2 ||
             aCodec == PVMF_MIME_AMRWB_IETF)
    {
        iOutFormat = aCodec;
    }
    else if (aCodec == PVMF_MIME_ADTS ||
             aCodec == PVMF_MIME_ADIF ||
             aCodec == PVMF_MIME_MPEG4_AUDIO)
    {
        iOutFormat = aCodec;
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXEncNode-%s::SetCodecType: ERROR Unsupported format aCodec=%s", iNodeTypeId, aCodec.getMIMEStrPtr()));

        return PVMFErrNotSupported;
    }

    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF bool PVMFOMXEncNode::SetFSIParam(uint8* aFSIBuff, int aFSIBuffLength)
{


    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                    (0, "PVMFOMXEncNode-%s::SetFSIParam: Error- NOT IMPLEMENTED", iNodeTypeId));
    OSCL_UNUSED_ARG(aFSIBuff);
    OSCL_UNUSED_ARG(aFSIBuffLength);

    return true;
}



// The input format methods are called from the port
////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFOMXEncNode::SetInputFormat(PVMFFormatType aFormat)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXEncNode-%s::SetInputFormat: aFormat=%s", iNodeTypeId, aFormat.getMIMEStrPtr()));

    switch (iInterfaceState)
    {
        case EPVMFNodeStarted:
        case EPVMFNodePaused:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFOMXEncNode-%s::SetInputFormat: Error - iInterfaceState=%d", iNodeTypeId, iInterfaceState));
            return PVMFErrInvalidState;
        default:
            break;
    }

    iInFormat = aFormat;

    if (aFormat == PVMF_MIME_YUV420)
    {
        iVideoInputFormat.iVideoFormat = EI_YUV420;
    }
    else if (aFormat == PVMF_MIME_YUV422)
    {
        iVideoInputFormat.iVideoFormat = EI_UYVY;
    }
    else if (aFormat == PVMF_MIME_RGB24)
    {
        iVideoInputFormat.iVideoFormat = EI_RGB24;
    }
    else if (aFormat == PVMF_MIME_RGB12)
    {
        iVideoInputFormat.iVideoFormat = EI_RGB12;
    }
    else if (aFormat == PVMF_MIME_PCM16)
    {
        // nothing to do here, but don't fail
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVMFOMXEncNode-%s::SetInputFormat: Error - Unsupported format", iNodeTypeId));
        return PVMFFailure;
    }

    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFOMXEncNode::SetInputFrameSize(uint32 aWidth, uint32 aHeight, uint8 aFrmOrient)
{
    switch (iInterfaceState)
    {
        case EPVMFNodeStarted:
        case EPVMFNodePaused:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFOMXEncNode-%s::SetInputFrameSize: Error iInterfaceState=%d", iNodeTypeId, iInterfaceState));
            return false;
        default:
            break;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXEncNode-%s::SetInputFrameSize: aWidth=%d, aHeight=%d, aFrmOrient=%d", iNodeTypeId, aWidth, aHeight, aFrmOrient));

    iVideoInputFormat.iFrameWidth = aWidth;
    iVideoInputFormat.iFrameHeight = aHeight;
    iVideoInputFormat.iFrameOrientation = aFrmOrient;
    return true;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFOMXEncNode::SetInputFrameRate(OsclFloat aFrameRate)
{
    switch (iInterfaceState)
    {
        case EPVMFNodeStarted:
        case EPVMFNodePaused:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFOMXEncNode-%s::SetInputFrameRate: Error iInterfaceState=%d", iNodeTypeId, iInterfaceState));
            return false;
        default:
            break;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXEncNode-%s::SetInputFrameRate: aFrameRate=%d", iNodeTypeId, aFrameRate));

    iVideoInputFormat.iFrameRate = OSCL_STATIC_CAST(float, aFrameRate);
    iVideoEncodeParam.iNoFrameSkip = iVideoEncodeParam.iNoCurrentSkip = false;
    return true;
}

////////////////////////////////////////////////////////////////////////////
PVMFFormatType PVMFOMXEncNode::GetCodecType()
{
    return iOutFormat;
}

////////////////////////////////////////////////////////////////////////////
// DV: Note - for video - there is an uint32 arg
uint32 PVMFOMXEncNode::GetOutputBitRate(uint32 aLayer)
{
    if ((int32)aLayer >= iVideoEncodeParam.iNumLayer)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVMFOMXEncNode-%s::GetOutputBitRate: Error - Invalid layer number", iNodeTypeId));
        return 0;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXEncNode-%s::GetOutputBitRate: =%d", iNodeTypeId, iVideoEncodeParam.iBitRate[aLayer]));

    return iVideoEncodeParam.iBitRate[aLayer];
}

////////////////////////////////////////////////////////////////////////////
OsclFloat PVMFOMXEncNode::GetOutputFrameRate(uint32 aLayer)
{
    if ((int32)aLayer >= iVideoEncodeParam.iNumLayer)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVMFOMXEncNode-%s::GetOutputFrameRate: Error Invalid layer number", iNodeTypeId));
        return 0;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXEncNode-%s::GetOutputFrameRate: =%f", iNodeTypeId, (OsclFloat) iVideoEncodeParam.iFrameRate[aLayer]));

    return (OsclFloat)iVideoEncodeParam.iFrameRate[aLayer];
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFOMXEncNode::GetOutputFrameSize(uint32 aLayer, uint32& aWidth, uint32& aHeight)
{
    if ((int32)aLayer >= iVideoEncodeParam.iNumLayer)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVMFOMXEncNode-%s::GetOutputFrameSize: Error Invalid layer number", iNodeTypeId));
        return PVMFFailure;
    }

    aWidth = iVideoEncodeParam.iFrameWidth[aLayer];
    aHeight = iVideoEncodeParam.iFrameHeight[aLayer];
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
uint32 PVMFOMXEncNode::GetIFrameInterval()
{
    return iVideoEncodeParam.iIFrameInterval;
}
/////////////////////////////////////////////////////////////////////////////

uint32 PVMFOMXEncNode::GetOutputSamplingRate()
{


    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXEncNode-%s::GetOutputSamplingRate: =%d", iNodeTypeId, iAudioEncodeParam.iOutputSamplingRate));

    return (uint32) iAudioEncodeParam.iOutputSamplingRate;
}
/////////////////////////////////////////////////////////////////////////////

uint32 PVMFOMXEncNode::GetOutputNumChannels()
{


    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXEncNode-%s::GetOutputNumChannels: =%d", iNodeTypeId, iAudioEncodeParam.iOutputNumChannels));

    return (uint32) iAudioEncodeParam.iOutputNumChannels;
}

/////////////////////////AMRENCInterfaceExtension //////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMFOMXEncNode::SetOutputBitRate(PVMF_GSMAMR_Rate aBitRate)
{
    // this particular API is used only for AMR (NB or WB)
    // do some error checking - make sure that NB (i.e. WB) bitrates correspond to NB (i.e. WB) codec
    if ((iOutFormat == PVMF_MIME_AMR_IF2) ||
            (iOutFormat == PVMF_MIME_AMR_IETF)
       )
    {

        switch (aBitRate)
        {
            case GSM_AMR_4_75:
            case GSM_AMR_5_15:
            case GSM_AMR_5_90:
            case GSM_AMR_6_70:
            case GSM_AMR_7_40:
            case GSM_AMR_7_95:
            case GSM_AMR_10_2:
            case GSM_AMR_12_2:

                iAudioEncodeParam.iAMRBitrate = aBitRate;

                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "PVMFOMXEncNode-%s::SetOutputBitRate() OK - %d", iNodeTypeId, aBitRate));

                return PVMFSuccess;

            case GSM_AMR_6_60: // AMR WB bitrates start here
            case GSM_AMR_8_85:
            case GSM_AMR_12_65:
            case GSM_AMR_14_25:
            case GSM_AMR_15_85:
            case GSM_AMR_18_25:
            case GSM_AMR_19_85:
            case GSM_AMR_23_05:
            case GSM_AMR_23_85:

                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMFOMXEncNode-%s::SetOutputBitRate() failed - %d", iNodeTypeId, aBitRate));

                return PVMFFailure;

            default:

                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMFOMXEncNode-%s::SetOutputBitRate() failed - %d", iNodeTypeId, aBitRate));

                return PVMFFailure;
        }
    }

    if (iOutFormat == PVMF_MIME_AMRWB_IETF)
    {
        switch (aBitRate)
        {
            case GSM_AMR_4_75:
            case GSM_AMR_5_15:
            case GSM_AMR_5_90:
            case GSM_AMR_6_70:
            case GSM_AMR_7_40:
            case GSM_AMR_7_95:
            case GSM_AMR_10_2:
            case GSM_AMR_12_2:

                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMFOMXEncNode-%s::SetOutputBitRate() failed - %d", iNodeTypeId, aBitRate));

                return PVMFFailure;

            case GSM_AMR_6_60: // AMR WB bitrates start here
            case GSM_AMR_8_85:
            case GSM_AMR_12_65:
            case GSM_AMR_14_25:
            case GSM_AMR_15_85:
            case GSM_AMR_18_25:
            case GSM_AMR_19_85:
            case GSM_AMR_23_05:
            case GSM_AMR_23_85:

                iAudioEncodeParam.iAMRBitrate = aBitRate;
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "PVMFOMXEncNode-%s::SetOutputBitRate() OK - %d", iNodeTypeId, aBitRate));

                return PVMFSuccess;

            default:

                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMFOMXEncNode-%s::SetOutputBitRate() failed - %d", iNodeTypeId, aBitRate));

                return PVMFFailure;
        }
    }

    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMFOMXEncNode::SetMaxNumOutputFramesPerBuffer(uint32 aNumOutputFrames)
{
    iAudioEncodeParam.iMaxNumOutputFramesPerBuffer = aNumOutputFrames;
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMFOMXEncNode::SetOutputBitRate(uint32 aBitRate)
{
    // this API is used for Non-AMR codecs
    iAudioEncodeParam.iOutputBitrate = aBitRate;
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMFOMXEncNode::SetOutputNumChannel(uint32 aNumChannels)
{

    iAudioEncodeParam.iOutputNumChannels = aNumChannels;
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMFOMXEncNode::SetOutputSamplingRate(uint32 aSamplingRate)
{
    iAudioEncodeParam.iOutputSamplingRate = aSamplingRate;
    return PVMFSuccess;
}


////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFOMXEncNode::SetInputSamplingRate(uint32 aSamplingRate)
{
    // do some error checking - make sure the input sampling rate is 8khz (i.e. 16khz) for AMRNB (i.e. WB)
    if (((iOutFormat == PVMF_MIME_AMR_IF2) ||
            (iOutFormat == PVMF_MIME_AMR_IETF)) &&
            (aSamplingRate != 8000)
       )
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVMFOMXEncNode-%s::SetInputBitsSamplingRate() failed - %d", iNodeTypeId, aSamplingRate));

        return PVMFFailure;
    }

    if ((iOutFormat == PVMF_MIME_AMRWB_IETF) && (aSamplingRate != 16000))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVMFOMXEncNode-%s::SetInputBitsSamplingRate() failed - %d", iNodeTypeId, aSamplingRate));

        return PVMFFailure;
    }

    iAudioInputFormat.iInputSamplingRate = aSamplingRate;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXEncNode-%s::SetInputBitsSamplingRate() OK setting OutputSamplingRate as well - %d", iNodeTypeId, aSamplingRate));

    // set output as well
    iAudioEncodeParam.iOutputSamplingRate = aSamplingRate;
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFOMXEncNode::SetInputBitsPerSample(uint32 aBitsPerSample)
{
    if (aBitsPerSample != 16)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVMFOMXEncNode-%s::SetInputBitsPerSample() failed - %d", iNodeTypeId, aBitsPerSample));

        return PVMFErrNotSupported;
    }

    iAudioInputFormat.iInputBitsPerSample = aBitsPerSample;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXEncNode-%s::SetInputBitsPerSample() OK - %d", iNodeTypeId, aBitsPerSample));

    return PVMFSuccess;
}


////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFOMXEncNode::SetInputNumChannels(uint32 aNumChannels)
{
    // do some error checking - make sure the number of INPUT channels is 1 for AMR (NB or WB)

    if (((iOutFormat == PVMF_MIME_AMR_IF2) ||
            (iOutFormat == PVMF_MIME_AMR_IETF) ||
            (iOutFormat == PVMF_MIME_AMRWB_IETF)) &&
            (aNumChannels > 1)
       )
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVMFOMXEncNode-%s::SetInputNumChannels() failed - %d", iNodeTypeId, aNumChannels));
        return PVMFFailure;
    }

    iAudioInputFormat.iInputNumChannels = aNumChannels;

    //set output as well
    iAudioEncodeParam.iOutputNumChannels = aNumChannels;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXEncNode-%s::SetInputNumChannels() OK - %d", iNodeTypeId, aNumChannels));

    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
uint32 PVMFOMXEncNode::GetOutputBitRate()
{
    if ((iOutFormat == PVMF_MIME_AMR_IF2) ||
            (iOutFormat == PVMF_MIME_AMR_IETF) ||
            (iOutFormat == PVMF_MIME_AMRWB_IETF)
       )
    {

        switch (iAudioEncodeParam.iAMRBitrate)
        {
            case GSM_AMR_4_75:
                return 4750;
            case GSM_AMR_5_15:
                return 5150;
            case GSM_AMR_5_90:
                return 5900;
            case GSM_AMR_6_70:
                return 6700;
            case GSM_AMR_7_40:
                return 7400;
            case GSM_AMR_7_95:
                return 7950;
            case GSM_AMR_10_2:
                return 10200;
            case GSM_AMR_12_2:
                return 12200;
            case GSM_AMR_6_60: // AMR WB bitrates start here
                return 6600;
            case GSM_AMR_8_85:
                return 8850;
            case GSM_AMR_12_65:
                return 12650;
            case GSM_AMR_14_25:
                return 14250;
            case GSM_AMR_15_85:
                return 15850;
            case GSM_AMR_18_25:
                return 18250;
            case GSM_AMR_19_85:
                return 19850;
            case GSM_AMR_23_05:
                return 23050;
            case GSM_AMR_23_85:
                return 23850;
            default:
                return 0;
        }
    }
    else
    {
        return iAudioEncodeParam.iOutputBitrate;
    }
}


///////////////////////// FROM CAP CONFIG IN VIDEO ENC NODE ////////////////////
PVMFStatus PVMFOMXEncNode::GetConfigParameter(PvmiKvp*& aParameters, int& aNumParamElements, int32 aIndex, PvmiKvpAttr aReqattr)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::GetConfigParameter() In", iNodeTypeId));

    aNumParamElements = 0;

    // Allocate memory for the KVP
    aParameters = (PvmiKvp*)oscl_malloc(sizeof(PvmiKvp));
    if (NULL == aParameters)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncNode-%s::GetConfigParameter() Memory allocation for KVP failed", iNodeTypeId));
        return PVMFErrNoMemory;
    }
    oscl_memset(aParameters, 0, sizeof(PvmiKvp));
    // Allocate memory for the key string in KVP
    PvmiKeyType memblock = (PvmiKeyType)oscl_malloc(PVOMXENCNODECONFIG_KEYSTRING_SIZE * sizeof(char));
    if (NULL == memblock)
    {
        oscl_free(aParameters);
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncNode-%s::GetConfigParameter() Memory allocation for key string failed", iNodeTypeId));
        return PVMFErrNoMemory;
    }
    oscl_strset(memblock, 0, PVOMXENCNODECONFIG_KEYSTRING_SIZE * sizeof(char));
    // Assign the key string buffer to KVP
    aParameters[0].key = memblock;

    // Copy the key string
    if (iInFormat == PVMF_MIME_PCM16)
    {
        // Copy the key string
        oscl_strncat(aParameters[0].key, _STRLIT_CHAR("x-pvmf/encoder/audio/"), 21);
        oscl_strncat(aParameters[0].key, PVOMXEncNodeConfigBaseKeys[aIndex].iString, oscl_strlen(PVOMXEncNodeConfigBaseKeys[aIndex].iString));
        oscl_strncat(aParameters[0].key, _STRLIT_CHAR(";type=value;valtype="), 20);
    }
    else
    {
        oscl_strncat(aParameters[0].key, _STRLIT_CHAR("x-pvmf/encoder/video/"), 21);
        oscl_strncat(aParameters[0].key, PVOMXEncNodeConfigBaseKeys[aIndex].iString, oscl_strlen(PVOMXEncNodeConfigBaseKeys[aIndex].iString));
        oscl_strncat(aParameters[0].key, _STRLIT_CHAR(";type=value;valtype="), 20);
    }
    switch (PVOMXEncNodeConfigBaseKeys[aIndex].iValueType)
    {
        case PVMI_KVPVALTYPE_BITARRAY32:
            oscl_strncat(aParameters[0].key, _STRLIT_CHAR(PVMI_KVPVALTYPE_BITARRAY32_STRING), oscl_strlen(PVMI_KVPVALTYPE_BITARRAY32_STRING));
            break;

        case PVMI_KVPVALTYPE_KSV:
            oscl_strncat(aParameters[0].key, _STRLIT_CHAR(PVMI_KVPVALTYPE_KSV_STRING), oscl_strlen(PVMI_KVPVALTYPE_KSV_STRING));
            break;

        case PVMI_KVPVALTYPE_BOOL:
            oscl_strncat(aParameters[0].key, _STRLIT_CHAR(PVMI_KVPVALTYPE_BOOL_STRING), oscl_strlen(PVMI_KVPVALTYPE_BOOL_STRING));
            break;

        case PVMI_KVPVALTYPE_INT32:
            if (PVMI_KVPATTR_CUR == aReqattr)
            {
                oscl_strncat(aParameters[0].key, _STRLIT_CHAR(PVMI_KVPVALTYPE_INT32_STRING), oscl_strlen(PVMI_KVPVALTYPE_RANGE_UINT32_STRING));
            }
            break;
        case PVMI_KVPVALTYPE_UINT32:
        default:
            if (PVMI_KVPATTR_CAP == aReqattr)
            {
                oscl_strncat(aParameters[0].key, _STRLIT_CHAR(PVMI_KVPVALTYPE_RANGE_UINT32_STRING), oscl_strlen(PVMI_KVPVALTYPE_RANGE_UINT32_STRING));
            }
            else
            {
                oscl_strncat(aParameters[0].key, _STRLIT_CHAR(PVMI_KVPVALTYPE_UINT32_STRING), oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING));
            }
            break;
    }
    aParameters[0].key[PVOMXENCNODECONFIG_KEYSTRING_SIZE-1] = 0;

    // Copy the requested info
    switch (aIndex)
    {

        case SAMPLING_RATE:	// "sampling_rate"
            if (PVMI_KVPATTR_CUR == aReqattr)
            {
                // get the parameter here
                aParameters[0].value.uint32_value = iAudioInputFormat.iInputSamplingRate;

            }
            else if (PVMI_KVPATTR_DEF == aReqattr)
            {
                // Return default
                aParameters[0].value.uint32_value = PVMF_AMRENC_DEFAULT_SAMPLING_RATE;

            }
            else
            {
                // Return capability
            }
            break;

        case CHANNELS:	// "channels"

            if (PVMI_KVPATTR_CUR == aReqattr)
            {
                // get the par
                aParameters[0].value.uint32_value = iAudioInputFormat.iInputNumChannels;
            }
            else if (PVMI_KVPATTR_DEF == aReqattr)
            {
                // Return default
                aParameters[0].value.uint32_value = PVMF_AMRENC_DEFAULT_NUM_CHANNELS;
            }
            else
            {
                // Return capability
            }
            break;

        case ENCODING_MODE:	// "encoding_mode"
            if (PVMI_KVPATTR_CUR == aReqattr)
            {
                // Return current value
                aParameters[0].value.uint32_value = iVideoEncodeParam.iEncMode;
            }
            else if (PVMI_KVPATTR_DEF == aReqattr)
            {
                // Return default
                aParameters[0].value.uint32_value = EI_ENCMODE_RECORDER;//default setting
            }
            else
            {
                // Return capability
            }
            break;

#ifdef _TEST_AE_ERROR_HANDLING
        case ERROR_START_INIT://error_start_init
            if (PVMI_KVPATTR_CUR == aReqattr)
            {
                // Return current value
                aParameters[0].value.bool_value = iErrorHandlingInit;
            }
            else if (PVMI_KVPATTR_DEF == aReqattr)
            {
                // Return default
                aParameters[0].value.bool_value = true;
            }
            else
            {
                // Return capability
            }
            break;
        case ERROR_ENCODE://error_encode
            if (PVMI_KVPATTR_CUR == aReqattr)
            {
                // Return current value
                aParameters[0].value.uint32_value = iErrorHandlingEncodeCount;
            }
            else if (PVMI_KVPATTR_DEF == aReqattr)
            {
                // Return default
                aParameters[0].value.uint32_value = 1;
            }
            else
            {
                // Return capability
            }
            break;

#endif

        default:
            // Invalid index
            oscl_free(aParameters[0].key);
            oscl_free(aParameters);
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncNode-%s::GetConfigParameter() Invalid index to video enc node parameter", iNodeTypeId));
            return PVMFErrNotSupported;
    }

    aNumParamElements = 1;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::GetConfigParameter() Out", iNodeTypeId));
    return PVMFSuccess;
}

PVMFStatus PVMFOMXEncNode::VerifyAndSetConfigParameter(PvmiKvp& aParameter, bool aSetParam)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::VerifyAndSetConfigParameter() In", iNodeTypeId));

    // Determine the valtype
    PvmiKvpValueType keyvaltype = GetValTypeFromKeyString(aParameter.key);
    if (PVMI_KVPVALTYPE_UNKNOWN == keyvaltype)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncNode-%s::VerifyAndSetConfigParameter() Valtype in key string unknown", iNodeTypeId));
        return PVMFErrNotSupported;
    }
    // Retrieve the fourth component from the key string
    char* compstr = NULL;
    pv_mime_string_extract_type(3, aParameter.key, compstr);

    int32 enccomp4ind;
    for (enccomp4ind = 0; enccomp4ind < PVOMXENCNODECONFIG_BASE_NUMKEYS; ++enccomp4ind)
    {
        // Go through each component string at 4th level
        if (pv_mime_strcmp(compstr, (char*)(PVOMXEncNodeConfigBaseKeys[enccomp4ind].iString)) >= 0)
        {
            // Break out of the for loop
            break;
        }
    }

    if (PVOMXENCNODECONFIG_BASE_NUMKEYS <= enccomp4ind)
    {
        // Match couldn't be found or non-leaf node specified
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncNode-%s::VerifyAndSetConfigParameter() Unsupported key or non-leaf node", iNodeTypeId));
        return PVMFErrNotSupported;
    }

    // Verify the valtype
    if (keyvaltype != PVOMXEncNodeConfigBaseKeys[enccomp4ind].iValueType)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncNode-%s::VerifyAndSetConfigParameter() Valtype does not match for key", iNodeTypeId));
        return PVMFErrNotSupported;
    }

    switch (enccomp4ind)
    {


        case SAMPLING_RATE: // "sampling_rate"
            // Change the parameter
            if (aSetParam)
            {
                // set the parameter here
                iAudioInputFormat.iInputSamplingRate = aParameter.value.uint32_value;
                iAudioEncodeParam.iOutputSamplingRate = aParameter.value.uint32_value;
            }
            break;

        case CHANNELS:  // "channels"
            // change the parameter
            if (aSetParam)
            {
                // set the parameter here
                iAudioInputFormat.iInputNumChannels = aParameter.value.uint32_value;
                iAudioEncodeParam.iOutputNumChannels = aParameter.value.uint32_value;
            }
            break;
        case ENCODING_MODE: // "encoding_mode"
            // change the parameter
            if (aSetParam)
            {
                iVideoEncodeParam.iEncMode = (EncEncodingMode)aParameter.value.uint32_value;
            }
            break;



#ifdef _TEST_AE_ERROR_HANDLING
        case ERROR_START_INIT: // "error_start_init"
            // change the parameter
            if (aSetParam)
            {
                iErrorHandlingInit = aParameter.value.bool_value;
            }
            break;
        case ERROR_ENCODE: // "error_avcencode"
            // change the parameter
            if (aSetParam)
            {

                char* paramstr = NULL;
                OSCL_HeapString<OsclMemAllocator> mode1 = "mode=duration";
                OSCL_HeapString<OsclMemAllocator> mode2 = "mode=frames";

                if (pv_mime_string_parse_param(aParameter.key, mode1.get_str(), paramstr) > 0)
                {
                    iErrorEncodeFlag = 1;
                    iErrorHandlingEncodeCount = aParameter.value.uint32_value;

                }

                else if (pv_mime_string_parse_param(aParameter.key, mode2.get_str(), paramstr) > 0)
                {
                    iErrorEncodeFlag = 2;
                    iErrorHandlingEncodeCount = aParameter.value.uint32_value;

                }
            }
            break;
        case ERROR_NODE_CMD: //"error-node-cmd"
            if (aSetParam)
            {
                iErrorNodeCmd = aParameter.value.uint32_value;
            }
            break;
        case ERROR_CONFIG_HEADER:
            if (aSetParam)
            {
                iErrorConfigHeader = aParameter.value.bool_value;
            }
            break;
        case ERROR_DATAPATH_STALL:
            if (aSetParam)
            {
                iErrorDataPathStall = aParameter.value.uint32_value;
            }
            break;
#endif

        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncNode-%s::VerifyAndSetConfigParameter() Invalid index for video enc node parameter", iNodeTypeId));
            return PVMFErrNotSupported;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::VerifyAndSetConfigParameter() Out", iNodeTypeId));
    return PVMFSuccess;
}

//void PVMFOMXEncNode::setParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters, int aNumElements, PvmiKvp*& aRetKVP)
void PVMFOMXEncNode::DoCapConfigSetParameters(PvmiKvp* aParameters, int aNumElements, PvmiKvp*& aRetKVP)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::DoCapConfigSetParameters()", iNodeTypeId));
    //OSCL_UNUSED_ARG(aSession);

    if (NULL == aParameters || aNumElements < 1)
    {
        if (aParameters)
        {
            aRetKVP = aParameters;
        }
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncNode-%s::DoCapConfigSetParameters() Passed in parameter invalid", iNodeTypeId));
        return;
    }

    // Go through each parameter
    for (int32 paramind = 0; paramind < aNumElements; ++paramind)
    {
        // Count the number of components and parameters in the key
        int compcount = pv_mime_string_compcnt(aParameters[paramind].key);
        // Retrieve the first component from the key string
        char* compstr = NULL;
        pv_mime_string_extract_type(0, aParameters[paramind].key, compstr);


        if ((
                    (pv_mime_strcmp(compstr, _STRLIT_CHAR("x-pvmf/encoder/video")) < 0) &&
                    (pv_mime_strcmp(compstr, _STRLIT_CHAR("x-pvmf/encoder/audio")) < 0)
                ) || (compcount < 4))
        {
            // First 3 components should be "x-pvmf/encoder/video" or "x-pvmf/encoder/audio" and there must
            // be at least 4 components
            aRetKVP = &aParameters[paramind];
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncNode-%s::DoCapConfigSetParameters() Unsupported key", iNodeTypeId));
            return;
        }

        // check if audio parameters are asked from video enc instance or vice versa
        if (((pv_mime_strcmp(compstr, _STRLIT_CHAR("x-pvmf/encoder/video")) > 0) && (iInFormat == PVMF_MIME_PCM16)) ||
                ((pv_mime_strcmp(compstr, _STRLIT_CHAR("x-pvmf/encoder/audio")) > 0) && (iInFormat != PVMF_MIME_PCM16))
           )
        {

            aRetKVP = &aParameters[paramind];
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncNode-%s::DoCapConfigSetParameters() Unsupported key", iNodeTypeId));
            return;
        }

        if (4 == compcount)
        {
            // Verify and set the passed-in video enc node setting
            PVMFStatus retval = VerifyAndSetConfigParameter(aParameters[paramind], true);
            if (PVMFSuccess != retval)
            {
                aRetKVP = &aParameters[paramind];
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncNode-%s::DoCapConfigSetParameters() Setting parameter %d failed", iNodeTypeId, paramind));
                return;
            }
        }

        else
        {
            // Do not support more than 4 components right now
            aRetKVP = &aParameters[paramind];
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncNode-%s::DoCapConfigSetParameters() Unsupported key", iNodeTypeId));
            return;
        }
    }
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::DoCapConfigSetParameters() Out", iNodeTypeId));
}


//PVMFStatus PVMFOMXEncNode::getParametersSync(PvmiMIOSession aSession, PvmiKeyType aIdentifier, PvmiKvp*& aParameters, int& aNumParamElements, PvmiCapabilityContext aContext)
PVMFStatus PVMFOMXEncNode::DoCapConfigGetParametersSync(PvmiKeyType aIdentifier, PvmiKvp*& aParameters, int& aNumParamElements, PvmiCapabilityContext aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::DoCapConfigGetParametersSync()", iNodeTypeId));
    //OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aContext);

    // Initialize the output parameters
    aNumParamElements = 0;
    aParameters = NULL;

    // Count the number of components and parameters in the key
    int compcount = pv_mime_string_compcnt(aIdentifier);
    // Retrieve the first component from the key string
    char* compstr = NULL;
    pv_mime_string_extract_type(0, aIdentifier, compstr);

    if ((
                (pv_mime_strcmp(compstr, _STRLIT_CHAR("x-pvmf/encoder/video")) < 0) &&
                (pv_mime_strcmp(compstr, _STRLIT_CHAR("x-pvmf/encoder/audio")) < 0)
            ) || (compcount < 4))
    {
        // First 3 components should be "x-pvmf/encoder/video" or  "x-pvmf/encoder/audio" and there must
        // be at least 4 components
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncNode-%s::DoCapConfigGetParametersSync() Invalid key string", iNodeTypeId));
        return PVMFErrNotSupported;
    }

    // check if audio parameters are asked from video enc instance or vice versa
    if (((pv_mime_strcmp(compstr, _STRLIT_CHAR("x-pvmf/encoder/video")) > 0) && (iInFormat == PVMF_MIME_PCM16)) ||
            ((pv_mime_strcmp(compstr, _STRLIT_CHAR("x-pvmf/encoder/audio")) > 0) && (iInFormat != PVMF_MIME_PCM16))
       )
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncNode-%s::DoCapConfigGetParameters() Unsupported key", iNodeTypeId));
        return PVMFErrNotSupported;
    }

    // Retrieve the fourth component from the key string
    pv_mime_string_extract_type(3, aIdentifier, compstr);

    for (int32 enccomp4ind = 0; enccomp4ind < PVOMXENCNODECONFIG_BASE_NUMKEYS; ++enccomp4ind)
    {
        // Go through each video enc component string at 4th level
        if (pv_mime_strcmp(compstr, (char*)(PVOMXEncNodeConfigBaseKeys[enccomp4ind].iString)) >= 0)
        {
            if (4 == compcount)
            {
                // Determine what is requested
                PvmiKvpAttr reqattr = GetAttrTypeFromKeyString(aIdentifier);
                if (PVMI_KVPATTR_UNKNOWN == reqattr)
                {
                    reqattr = PVMI_KVPATTR_CUR;
                }

                // Return the requested info
                PVMFStatus retval = GetConfigParameter(aParameters, aNumParamElements, enccomp4ind, reqattr);
                if (PVMFSuccess != retval)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncNode-%s::DoCapConfigGetParametersSync() Retrieving video enc node parameter failed", iNodeTypeId));
                    return retval;
                }
            }
            else
            {
                // Right now videoenc node doesn't support more than 4 components
                // for this sub-key string so error out
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncNode-%s::DoCapConfigGetParametersSync() Unsupported key", iNodeTypeId));
                return PVMFErrNotSupported;
            }
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::DoCapConfigGetParametersSync() Out", iNodeTypeId));
    if (aNumParamElements == 0)
    {
        // If no one could get the parameter, return error
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncNode-%s::DoCapConfigGetParametersSync() Unsupported key", iNodeTypeId));
        return PVMFFailure;
    }
    else
    {
        return PVMFSuccess;
    }
}

//PVMFStatus PVMFOMXEncNode::releaseParameters(PvmiMIOSession aSession, PvmiKvp* aParameters, int aNumElements)
PVMFStatus PVMFOMXEncNode::DoCapConfigReleaseParameters(PvmiKvp* aParameters, int aNumElements)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::releaseParameters()", iNodeTypeId));
    //OSCL_UNUSED_ARG(aSession);

    if (aParameters == NULL || aNumElements < 1)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncNode-%s::DoCapConfigReleaseParameters() KVP list is NULL or number of elements is 0", iNodeTypeId));
        return PVMFErrArgument;
    }

    // Count the number of components and parameters in the key
    int compcount = pv_mime_string_compcnt(aParameters[0].key);
    // Retrieve the first component from the key string
    char* compstr = NULL;
    pv_mime_string_extract_type(0, aParameters[0].key, compstr);

    if ((
                (pv_mime_strcmp(compstr, _STRLIT_CHAR("x-pvmf/encoder/video")) < 0) &&
                (pv_mime_strcmp(compstr, _STRLIT_CHAR("x-pvmf/encoder/audio")) < 0)
            ) || (compcount < 3))
    {
        // First 3 component should be "x-pvmf/encoder/video" or "x-pvmf/encoder/audio" and there must
        // be at least three components
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncNode-%s::DoCapConfigReleaseParameters() Unsupported key", iNodeTypeId));
        return PVMFErrNotSupported;
    }

    // check if audio parameters are asked from video enc instance or vice versa
    if (((pv_mime_strcmp(compstr, _STRLIT_CHAR("x-pvmf/encoder/video")) > 0) && (iInFormat == PVMF_MIME_PCM16)) ||
            ((pv_mime_strcmp(compstr, _STRLIT_CHAR("x-pvmf/encoder/audio")) > 0) && (iInFormat != PVMF_MIME_PCM16))
       )
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncNode-%s::DoCapConfigReleaseParameters() Unsupported key", iNodeTypeId));
        return PVMFErrNotSupported;
    }

    // Retrieve the third component from the key string
    pv_mime_string_extract_type(2, aParameters[0].key, compstr);

    // Go through each KVP and release memory for value if allocated from heap
    for (int32 ii = 0; ii < aNumElements; ++ii)
    {
        // Next check if it is a value type that allocated memory
        PvmiKvpType kvptype = GetTypeFromKeyString(aParameters[ii].key);
        if (PVMI_KVPTYPE_VALUE == kvptype || PVMI_KVPTYPE_UNKNOWN == kvptype)
        {
            PvmiKvpValueType keyvaltype = GetValTypeFromKeyString(aParameters[ii].key);
            if (PVMI_KVPVALTYPE_UNKNOWN == keyvaltype)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncNode-%s::DoCapConfigReleaseParameters() Valtype not specified in key string", iNodeTypeId));
                return PVMFErrNotSupported;
            }

            if (PVMI_KVPVALTYPE_CHARPTR == keyvaltype && NULL != aParameters[ii].value.pChar_value)
            {
                oscl_free(aParameters[ii].value.pChar_value);
                aParameters[ii].value.pChar_value = NULL;
            }
            else if (keyvaltype == PVMI_KVPVALTYPE_KSV && NULL != aParameters[ii].value.key_specific_value)
            {
                oscl_free(aParameters[ii].value.key_specific_value);
                aParameters[ii].value.key_specific_value = NULL;
            }
            else if (PVMI_KVPVALTYPE_RANGE_UINT32 == keyvaltype && NULL != aParameters[ii].value.key_specific_value)
            {
                range_uint32* rui32 = (range_uint32*)aParameters[ii].value.key_specific_value;
                aParameters[ii].value.key_specific_value = NULL;
                oscl_free(rui32);
            }
            // @TODO Add more types if video enc node starts returning more types
        }
    }

    // Video enc node allocated its key strings in one chunk so just free the first key string ptr
    oscl_free(aParameters[0].key);

    // Free memory for the parameter list
    oscl_free(aParameters);
    aParameters = NULL;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::DoCapConfigReleaseParameters() Out", iNodeTypeId));
    return PVMFSuccess;
}


//PVMFStatus PVMFOMXEncNode::verifyParametersSync (PvmiMIOSession aSession, PvmiKvp* aParameters, int aNumElements)
PVMFStatus PVMFOMXEncNode::DoCapConfigVerifyParameters(PvmiKvp* aParameters, int aNumElements)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::DoCapConfigVerifyParameters()", iNodeTypeId));
    //OSCL_UNUSED_ARG(aSession);

    if (NULL == aParameters || aNumElements < 1)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncNode-%s::DoCapConfigVerifyParameters() Passed in parameter invalid", iNodeTypeId));
        return PVMFErrArgument;
    }

    // Go through each parameter
    for (int32 paramind = 0; paramind < aNumElements; ++paramind)
    {
        // Count the number of components and parameters in the key
        int compcount = pv_mime_string_compcnt(aParameters[paramind].key);
        // Retrieve the first component from the key string
        char* compstr = NULL;
        pv_mime_string_extract_type(0, aParameters[paramind].key, compstr);

        if (
            ((pv_mime_strcmp(compstr, _STRLIT_CHAR("x-pvmf/encoder/video")) < 0) &&
             (pv_mime_strcmp(compstr, _STRLIT_CHAR("x-pvmf/encoder/audio")) < 0)
            ) || compcount < 3)
        {
            // First 3 components should be "x-pvmf/encoder/video" or
            // "x-pvmf/enoder/audio" and there must
            // be at least 3 components

            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncNode-%s::DoCapConfigVerifyParameters() Unsupported key", iNodeTypeId));
            return PVMFErrNotSupported;
        }

        // check if audio parameters are asked from video enc instance or vice versa
        if (((pv_mime_strcmp(compstr, _STRLIT_CHAR("x-pvmf/encoder/video")) > 0) && (iInFormat == PVMF_MIME_PCM16)) ||
                ((pv_mime_strcmp(compstr, _STRLIT_CHAR("x-pvmf/encoder/audio")) > 0) && (iInFormat != PVMF_MIME_PCM16))
           )
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncNode-%s::DoCapConfigVerifyParameters() Unsupported key", iNodeTypeId));
            return PVMFErrNotSupported;
        }

        if (4 == compcount)
        {
            // Verify and set the passed-in video enc node setting
            PVMFStatus retval = VerifyAndSetConfigParameter(aParameters[paramind], false);
            if (retval != PVMFSuccess)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncNode-%s::DoCapConfigVerifyParameters() Setting parameter %d failed", iNodeTypeId, paramind));
                return retval;
            }
        }
        else
        {
            // Do not support more than 4 components right now
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncNode-%s::DoCapConfigVerifyParameters() Unsupported key", iNodeTypeId));
            return PVMFErrNotSupported;
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncNode-%s::DoCapConfigVerifyParameters() Out", iNodeTypeId));
    return PVMFSuccess;

}

int32 PVMFOMXEncNode::PushBackKeyVal(Oscl_Vector<PvmiKvp, OsclMemAllocator>*& aValueListPtr, PvmiKvp &aKeyVal)
{
    int32 leavecode = 0;
    OSCL_TRY(leavecode, (*aValueListPtr).push_back(aKeyVal));
    return leavecode;
}

int32 PVMFOMXEncNode::Push_Back_MetadataKeys(const char* aMetadataKey)
{
    int32 leavecode = 0;
    OSCL_TRY(leavecode, iAvailableMetadataKeys.push_back(aMetadataKey));
    return leavecode;
}

int32 PVMFOMXEncNode::Push_Back_MetadataKeys(PVMFMetadataList *&aKeylistptr, uint32 aLcv)
{
    int32 leavecode = 0;
    OSCL_TRY(leavecode, aKeylistptr->push_back(iAvailableMetadataKeys[aLcv]));
    return leavecode;
}

int32 PVMFOMXEncNode::CreateNewArray(char*& aPtr, int32 aLen)
{
    int32 leavecode = 0;
    OSCL_TRY(leavecode,
             aPtr = OSCL_ARRAY_NEW(char, aLen););
    return leavecode;
}

int32 PVMFOMXEncNode::MemAllocate(OsclAny *&aPtr, OsclMemPoolFixedChunkAllocator *aMemPool, uint32 aAllocSize)
{
    uint32 errcode = 0;
    OSCL_TRY(errcode, aPtr = (OsclAny *) aMemPool->allocate(aAllocSize));
    return errcode;
}

bool PVMFOMXEncNode::ParseFullAVCFramesIntoNALs(OMX_BUFFERHEADERTYPE* aOutputBuffer)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "PVMFOMXEncNode-%s : ParseFullAVCFramesIntoNALs IN", iNodeTypeId));

    if (iOMXComponentUsesNALStartCodes && !(aOutputBuffer->nFlags & OMX_BUFFERFLAG_EXTRADATA))
    {
        OMX_U32 offset = aOutputBuffer->nOffset;
        OMX_U32 length = aOutputBuffer->nFilledLen;
        OMX_U8* pBuffer = aOutputBuffer->pBuffer + offset;
        OMX_U8* pCurrNAL;
        OMX_U32 bytesConsumed;
        int32 nalSize;

        iNumNALs = 0;

        while (length > 0)
        {
            nalSize = length;

            if (false == AVCAnnexBGetNALUnit(pBuffer, &pCurrNAL, &nalSize, false))
            {
                break;
            }

            bytesConsumed = nalSize + (int32)(pCurrNAL - pBuffer);
            length -= bytesConsumed;
            pBuffer += bytesConsumed;

            if ((iNALSizeArrayMaxElems > iNumNALs) && (iNALSizeArray != NULL))
            {
                iNALSizeArray[iNumNALs] = nalSize;
                iNALPtrArray[iNumNALs] = (uint8*)pCurrNAL; /* need store NAL ptrs since start code can be either 4 bytes or 3 bytes */
                iNumNALs++;
            }
            else
            {
                iNumNALs++;

                // count the number of NALs in the buffer
                while (length > 0)
                {
                    nalSize = length;

                    if (false == AVCAnnexBGetNALUnit(pBuffer, &pCurrNAL, &nalSize, false))
                    {
                        break;
                    }

                    bytesConsumed = nalSize + (int32)(pCurrNAL - pBuffer);
                    length -= bytesConsumed;
                    pBuffer += bytesConsumed;

                    iNumNALs++;
                }

                // reassign alloc size to new max.
                iNALSizeArrayMaxElems = iNumNALs;

                // free memory and then reallocate
                if (iNALSizeArray != NULL)
                {
                    oscl_free(iNALSizeArray);
                }
                iNALSizeArray = (uint32*) oscl_malloc(sizeof(uint32) * iNALSizeArrayMaxElems);

                if (iNALPtrArray != NULL)
                {
                    oscl_free(iNALPtrArray);
                }
                iNALPtrArray = (uint8**) oscl_malloc(sizeof(uint8*) * iNALSizeArrayMaxElems);

                // reset parameters and start over
                iNumNALs = 0;
                length = aOutputBuffer->nFilledLen;
                pBuffer = aOutputBuffer->pBuffer + offset;

                if (iNALSizeArray == NULL || iNALPtrArray == NULL)
                {
                    iNALSizeArrayMaxElems = 0;

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "PVMFOMXEncNode-%s : ParseFullAVCFramesIntoNALs ERROR - Out of Memory", iNodeTypeId));
                    return false;
                }
            }
        }

        if (iNumNALs <= 0)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "PVMFOMXEncNode-%s : ParseFullAVCFramesIntoNALs ERROR", iNodeTypeId));
            return false;
        }
    }
    else if (aOutputBuffer->nFlags & OMX_BUFFERFLAG_EXTRADATA)
    {
        // get extra data from end of buffer
        OMX_OTHER_EXTRADATATYPE *pExtra;
        OMX_U32 offset = aOutputBuffer->nOffset + aOutputBuffer->nFilledLen;
        OMX_U32 allocLen = aOutputBuffer->nAllocLen;
        OMX_U8* pTemp = aOutputBuffer->pBuffer + offset;

        // align
        pExtra = (OMX_OTHER_EXTRADATATYPE *)(((OMX_U32) pTemp + 3) & ~3);
        offset += (OMX_U32) pExtra - (OMX_U32) pTemp;

        while (pExtra->eType != OMX_ExtraDataNone)
        {
            if (pExtra->eType == OMX_ExtraDataNALSizeArray)
            {
                iNumNALs = pExtra->nDataSize >> 2;
                if ((iNALSizeArrayMaxElems > iNumNALs) && (iNALSizeArray != NULL))
                {
                    oscl_memcpy(iNALSizeArray, ((OMX_U8*)pExtra + 20), pExtra->nDataSize); // 20 is the size of the extra data struct (minus data hint)
                }
                else
                {
                    // reassign alloc size to new max.
                    iNALSizeArrayMaxElems = iNumNALs;

                    // free memory and then reallocate
                    if (iNALSizeArray != NULL)
                    {
                        oscl_free(iNALSizeArray);
                    }

                    iNALSizeArray = (uint32*) oscl_malloc(sizeof(uint32) * iNALSizeArrayMaxElems);
                    if (iNALSizeArray == NULL)
                    {
                        iNALSizeArrayMaxElems = 0;

                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "PVMFOMXEncNode-%s : ParseFullAVCFramesIntoNALs ERROR - Out of Memory", iNodeTypeId));
                        return false;
                    }

                    oscl_memcpy(iNALSizeArray, ((OMX_U8*)pExtra + 20), pExtra->nDataSize); // 20 is the size of the extra data struct (minus data hint)
                }

                break;
            }

            offset += pExtra->nSize;
            /* 20 is size of extra data struct (minus data hint),
             * so if there isn't enough room for there to be a full struct, and we haven't reached an OMX_ExtraDataNone
             * the data is corrupt
             */
            if (offset > (allocLen - 20))
            {
                // corrupt data
                break;
            }
            else
            {
                pExtra = (OMX_OTHER_EXTRADATATYPE *)((OMX_U8*)pExtra + pExtra->nSize);
            }
        }

        if (pExtra->eType != OMX_ExtraDataNALSizeArray)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "PVMFOMXEncNode-%s : ParseFullAVCFramesIntoNALs ERROR", iNodeTypeId));
            return false;
        }
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "PVMFOMXEncNode-%s : ParseFullAVCFramesIntoNALs ERROR", iNodeTypeId));
        return false;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "PVMFOMXEncNode-%s : ParseFullAVCFramesIntoNALs OUT", iNodeTypeId));
    return true;
}

/* utility function copied from the AVC Decoder interface */
bool PVMFOMXEncNode::AVCAnnexBGetNALUnit(uint8 *bitstream, uint8 **nal_unit, int32 *size, bool getPtrOnly)
{
    int32 i, j, FoundStartCode = 0;
    int32 end;

    i = 0;
    while (bitstream[i] == 0 && i < *size)
    {
        i++;
    }
    if (i >= *size)
    {
        *nal_unit = bitstream;
        return false; /* cannot find any start_code_prefix. */
    }
    else if (bitstream[i] != 0x1)
    {
        i = -1;  /* start_code_prefix is not at the beginning, continue */
    }

    i++;
    *nal_unit = bitstream + i; /* point to the beginning of the NAL unit */

    if (getPtrOnly)
    {
        // size not needed, just return with ptr
        return true;
    }

    j = end = i;
    while (!FoundStartCode)
    {
        while ((j + 1 < *size) && (bitstream[j] != 0 || bitstream[j+1] != 0))  /* see 2 consecutive zero bytes */
        {
            j++;
        }
        end = j;   /* stop and check for start code */
        while (j + 2 < *size && bitstream[j+2] == 0) /* keep reading for zero byte */
        {
            j++;
        }
        if (j + 2 >= *size)
        {
            *size -= i;
            return true;  /* cannot find the second start_code_prefix */
        }
        if (bitstream[j+2] == 0x1)
        {
            FoundStartCode = 1;
        }
        else
        {
            /* could be emulation code 0x3 */
            j += 2; /* continue the search */
        }
    }

    *size = end - i;

    return true;
}

