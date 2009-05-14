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

#ifndef PVMF_PROTOCOLENGINE_NODE_COMMON_H_INCLUDED
#define PVMF_PROTOCOLENGINE_NODE_COMMON_H_INCLUDED

#ifndef OSCL_MEM_MEMPOOL_H_INCLUDED
#include "oscl_mem_mempool.h"
#endif
#ifndef PVMF_MEDIA_DATA_H_INCLUDED
#include "pvmf_media_data.h"
#endif
#ifndef PVMF_SIMPLE_MEDIA_BUFFER_H_INCLUDED
#include "pvmf_simple_media_buffer.h"
#endif
#ifndef PVMF_META_DATA_TYPES_H_INCLUDED
#include "pvmf_meta_data_types.h"
#endif
#ifndef PVMF_MEDIA_MSG_FORMAT_IDS_H_INCLUDED
#include "pvmf_media_msg_format_ids.h"
#endif
#ifndef PVMI_DATASTREAMUSER_INTERFACE_H_INCLUDED
#include "pvmi_datastreamuser_interface.h"
#endif
#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif
#ifndef PVLOGGER_FILE_APPENDER_H_INCLUDED
#include "pvlogger_file_appender.h"
#endif
#ifndef PVMF_DOWNLOAD_DATA_SOURCE_H_INCLUDED
#include "pvmf_download_data_source.h"
#endif
#ifndef PVMF_SOURCE_CONTEXT_DATA_H_INCLUDED
#include "pvmf_source_context_data.h"
#endif
#ifndef OSCL_TIMER_H_INCLUDED
#include "oscl_timer.h"
#endif
#ifndef PVMF_PROTOCOL_ENGINE_COMMON_H_INCLUDED
#include "pvmf_protocol_engine_common.h"
#endif
#ifndef PVMF_NODE_UTILS_H_INCLUDED
#include "pvmf_node_utils.h"
#endif
#ifndef PVMF_PROTOCOL_ENGINE_NODE_EVENTS_H_INCLUDED
#include "pvmf_protocol_engine_node_events.h"
#endif
#ifndef PVMI_DATA_STREAM_INTERFACE_H_INCLUDED
#include "pvmi_data_stream_interface.h"
#endif
#ifndef PVMF_EVENT_HANDLING_H_INCLUDED
#include "pvmf_event_handling.h"
#endif
#ifndef PVMF_MEDIA_PRESENTATION_INFO_H_INCLUDED
#include "pvmf_media_presentation_info.h"
#endif
#ifndef PVMF_MEMPOOL_H_INCLUDED
#include "pvmf_mempool.h"
#endif
#ifndef PVMF_MEDIA_CLOCK_H_INCLUDED
#include "pvmf_media_clock.h"
#endif
#ifndef PVMI_KVP_H_INCLUDED
#include "pvmi_kvp.h"
#endif


//#define PVMF_PROTOCOL_ENGINE_LOG_MS_STREAMING_OUTPUT



//memory allocator type for this node.
typedef OsclMemAllocator PVMFProtocolEngineNodeAllocator;

// Structure to hold the key string info for
// AAC decnode's capability-and-config
struct PVProtocolEngineNodeKeyStringData
{
    char iString[64];
    PvmiKvpType iType;
    PvmiKvpValueType iValueType;
};

// The number of characters to allocate for the key string
#define PVPROTOCOLENGINENODECONFIG_KEYSTRING_SIZE 128

///////////////////////////////////////////////////////
// For Command implementation
///////////////////////////////////////////////////////

//Default vector reserve size
#define PVMF_PROTOCOLENGINE_NODE_COMMAND_VECTOR_RESERVE 16

//Starting value for command IDs
#define PVMF_PROTOCOLENGINE_NODE_COMMAND_ID_START 6000

enum PVMFProtocolEngineNodeCmdType
{
    PVPROTOCOLENGINE_NODE_CMD_SEEK = PVMF_GENERIC_NODE_COMMAND_LAST,
    PVPROTOCOLENGINE_NODE_CMD_BITSTREAM_SWITCH,
    PVPROTOCOLENGINE_NODE_CMD_DATASTREAM_REQUEST_REPOSITION
};

enum PVMFProtocolEngineNodeState
{
    // this is special node internal state for processing start command
    // we set node state as this state before doing the actual work for start
    // this is to differentiate the node state between pause and start without completing start command
    // this differentiation aims to ignore logging/keep-alive response when start command gets called,
    // but hasn't got completed.
    // This internal state should be gone when start command gets completed, then node state will change
    // to EPVMFNodeStarted
    PVMFProtocolEngineNodeState_BeingStarted = EPVMFNodeLastState,
};


//Node command type.
#define PVMFProtocolEngineNodeCommandBase PVMFGenericNodeCommand<PVMFProtocolEngineNodeAllocator> // to remove warning on symbian build
class PVMFProtocolEngineNodeCommand: public PVMFProtocolEngineNodeCommandBase
{
    public:
        //constructor for Custom2 command
        void Construct(PVMFSessionId s, int32 cmd, int32 arg1, int32 arg2, int32& arg3, const OsclAny*aContext)
        {
            PVMFProtocolEngineNodeCommandBase::Construct(s, cmd, aContext);
            iParam1 = (OsclAny*)arg1;
            iParam2 = (OsclAny*)arg2;
            iParam3 = (OsclAny*) & arg3;
        }
        void Parse(int32&arg1, int32&arg2, int32*&arg3)
        {
            arg1 = (int32)iParam1;
            arg2 = (int32)iParam2;
            arg3 = (int32*)iParam3;
        }

        // Constructor and parser for seek and bitstreamSwitch
        void Construct(PVMFSessionId s, int32 cmd,
                       uint64 aNPTInMS,
                       uint32& aFirstSeqNumAfterChange,
                       OsclAny* aContext)
        {
            PVMFProtocolEngineNodeCommandBase::Construct(s, cmd, aContext);
            iNPTInMS = aNPTInMS;
            iParam2 = (OsclAny*) & aFirstSeqNumAfterChange;
        }

        void Parse(uint64& aNPTInMS, uint32*& aFirstSeqNumAfterChange)
        {
            aNPTInMS = iNPTInMS;
            aFirstSeqNumAfterChange = (uint32*)iParam2;
        }

        // constructor and parser for data stream request, especially reposition request
        void Construct(PVMFSessionId s, int32 cmd,
                       PvmiDataStreamSession aSessionID,
                       PvmiDataStreamRequest aRequestID,
                       OsclAny* aRequestData,
                       PvmiDataStreamCommandId aDataStreamCmdId,
                       OsclAny* aContext)
        {
            PVMFProtocolEngineNodeCommandBase::Construct(s, cmd, aContext);
            iParam1 = (OsclAny*)aSessionID;
            iParam2 = (OsclAny*)((uint32)aRequestID);
            iParam3 = aRequestData;
            iParam4 = (OsclAny*)aDataStreamCmdId;
        }

        void Parse(PvmiDataStreamSession& aSessionID, PvmiDataStreamRequest& aRequestID,
                   OsclAny*& aRequestData, PvmiDataStreamCommandId &aDataStreamCmdId)
        {
            aSessionID   = (PvmiDataStreamSession)iParam1;
            aRequestData = iParam3;
            aDataStreamCmdId = (PvmiDataStreamCommandId)iParam4;
            uint32 requestIDNum = (uint32)iParam2;
            aRequestID   = (PvmiDataStreamRequest)requestIDNum;
        }

        void Parse(OsclAny*& aRequestData)
        {
            aRequestData = iParam3;
        }

        void Parse(OsclAny*& aRequestData, PvmiDataStreamCommandId &aDataStreamCmdId)
        {
            aRequestData = iParam3;
            aDataStreamCmdId = (PvmiDataStreamCommandId)iParam4;
        }

    private:
        uint64 iNPTInMS;
};


//Command queue type
typedef PVMFNodeCommandQueue<PVMFProtocolEngineNodeCommand, PVMFProtocolEngineNodeAllocator> PVMFProtocolEngineNodeCmdQ;

typedef Oscl_Vector<PVMFSharedMediaMsgPtr, PVMFProtocolEngineNodeAllocator> INPUT_DATA_QUEUE;
typedef Oscl_Vector<OsclRefCounterMemFrag, PVMFProtocolEngineNodeAllocator> OUTPUT_DATA_QUEUE;
typedef Oscl_Vector<OsclRefCounterMemFrag*, PVMFProtocolEngineNodeAllocator> PENDING_OUTPUT_DATA_QUEUE;

// two macros used in the array member and function parameter below
#define PVHTTPDOWNLOADOUTPUT_CONTENTDATA_CHUNKSIZE 8000
#define EVENT_HANDLER_TOTAL 10

enum NetworkTimerType
{
    SERVER_RESPONSE_TIMER_ID = 0,
    SERVER_INACTIVITY_TIMER_ID,
    SERVER_KEEPALIVE_TIMER_ID,
    SERVER_RESPONSE_TIMER_ID_FOR_STOPEOS_LOGGING,
    // handle data processing in case of no data input (PE node will become idle) for download/progressive streaming,
    // so there should be a way to activate PE node to continue data processing if needed
    WALL_CLOCK_TIMER_ID,
    // a timer to report buffer status periodically, say at every up to 2sec, at least buffer status has to be reported
    // which tells our system is running
    BUFFER_STATUS_TIMER_ID
};

enum PVHttpProtocol
{
    PVHTTPPROTOCOL_PROGRESSIVE_DOWNLOAD = 0,
    PVHTTPPROTOCOL_PROGRESSIVE_STREAMING,
    PVHTTPPROTOCOL_SHOUTCAST,
    PVHTTPPROTOCOL_FASTTRACK_DOWNLOAD,
    PVHTTPPROTOCOL_MS_HTTP_STREAMING,
    PVHTTPPROTOCOL_UNKNOWN
};

// Forward declarations
class PVMFProtocolEngineNode;
class PVMFProtocolEnginePort;
class PVMIDataStreamSyncInterface;
class PVLogger;
class PVMFLightMediaFragGroupAlloc;
class PVMFMediaPresentationInfo;
class HttpBasedProtocol;
class PVMFProtocolEngineNodeOutput;
class DownloadControlInterface;
class DownloadProgressInterface;
class EventReporter;
class PVDlCfgFile;
class PVDlCfgFileContainer;
class PVMFDownloadDataSourceContainer;
class PVMFProtocolEngineNodeTimer;
class InterfacingObjectContainer;
class UserAgentField;
struct PVProtocolEngineNodeInternalEvent;
class PVProtocolEngineNodeInternalEventHandler;
class PVMFProtocolEnginePort;
class OsclSharedLibrary;


enum NodeObjectType
{
    NodeObjectType_InputPortForData = 0,
    NodeObjectType_InputPortForLogging,
    NodeObjectType_OutPort,
    NodeObjectType_InternalEventQueue,

    NodeObjectType_Protocol,
    NodeObjectType_Output,
    NodeObjectType_DownloadControl,
    NodeObjectType_DownloadProgress,
    NodeObjectType_EventReport,
    NodeObjectType_DlCfgFileContainer,
    NodeObjectType_DataSourceContainer,
    NodeObjectType_Timer,
    NodeObjectType_InterfacingObjectContainer,
    NodeObjectType_UseAgentField
};

class ProtocolContainerObserver
{
    public:
        virtual uint32 GetObserverState() = 0;
        virtual void SetObserverState(const uint32 aState) = 0;
        virtual bool DispatchEvent(PVProtocolEngineNodeInternalEvent *aEvent) = 0;
        virtual bool SendMediaCommand(PVMFProtocolEnginePort *aPort, PVUid32 aCmdId, const bool isForLogging = false) = 0;
        virtual void ClearRest(const bool aNeedDelete = false) = 0;
        virtual void RecheduleDataFlow() = 0;
        virtual void SendManualResumeNotificationEvent() = 0;
        virtual bool IsRepositionCmdPending() = 0;
        virtual PVMFProtocolEngineNodeCommand* FindPendingCmd(int32 aCmdId) = 0;
        virtual void CompletePendingCmd(int32 status) = 0;
        virtual void CompleteInputCmd(PVMFProtocolEngineNodeCommand& aCmd, int32 status) = 0;
        virtual void ErasePendingCmd(PVMFProtocolEngineNodeCommand *aCmd) = 0;
        virtual void ReportEvent(PVMFEventType aEventType, OsclAny* aEventData = NULL, const int32 aEventCode = 0, OsclAny* aEventLocalBuffer = NULL, const uint32 aEventLocalBufferSize = 0) = 0;
};



// This class handles protocol initialization for multiple http based protocols,
// and it serves as a friend class of PVMFProtocolEngineNode, and thus helps do
// protocol initialization.
class ProtocolContainer
{
    public:
        // constructor
        OSCL_IMPORT_REF ProtocolContainer(PVMFProtocolEngineNode *aNode = NULL);
        virtual ~ProtocolContainer()
        {
            iDataPathLogger = NULL;
            clear();
        }

        void setObserver(ProtocolContainerObserver *aObserver)
        {
            iObserver = aObserver;
        }
        OSCL_IMPORT_REF virtual bool createProtocolObjects();
        OSCL_IMPORT_REF virtual void deleteProtocolObjects();
        OSCL_IMPORT_REF virtual bool isObjectsReady(); // centralize the info-checking
        OSCL_IMPORT_REF virtual void setSupportObject(OsclAny* aSupportObject, const uint32 aType);
        virtual PVMFStatus doInit()
        {
            return PVMFSuccess;    // used in PVMFProtocolEngineNode::doInit
        }
        OSCL_IMPORT_REF virtual PVMFStatus doPrepare();							// used in PVMFProtocolEngineNode::doPrepare, the default implementation is for both 3gpp and fasttrack download
        virtual bool doProPrepare()
        {
            return true;    // used only for fasttrack, invoke the call to generate SDP info.
        }
        virtual int32 doPreStart()
        {
            return PROCESS_SUCCESS;
        }
        virtual bool doPause()
        {
            return true;
        }
        OSCL_IMPORT_REF virtual PVMFStatus doStop();
        OSCL_IMPORT_REF virtual bool doEOS(const bool isTrueEOS = true);
        virtual bool doInfoUpdate(const uint32 downloadStatus)
        {
            OSCL_UNUSED_ARG(downloadStatus);    // for now, used for download only, report event and update download control
            return true;
        }
        virtual PVMFStatus doSeek(PVMFProtocolEngineNodeCommand& aCmd)
        {
            OSCL_UNUSED_ARG(aCmd);
            return PVMFSuccess;
        }
        virtual PVMFStatus doBitstreamSwitch(PVMFProtocolEngineNodeCommand& aCmd)
        {
            OSCL_UNUSED_ARG(aCmd);
            return PVMFSuccess;
        }
        OSCL_IMPORT_REF virtual bool reconnectSocket(const bool aForceSocketReconnect = true);	// used for progressive download and http streaming
        virtual bool needSocketReconnect()
        {
            return true;
        }
        OSCL_IMPORT_REF virtual void startDataFlowByCommand(const bool needDoSocketReconnect = true);

        OSCL_IMPORT_REF virtual void doClear(const bool aNeedDelete = false);
        OSCL_IMPORT_REF virtual void doStopClear();
        OSCL_IMPORT_REF virtual void doCancelClear();
        virtual bool addSourceData(OsclAny* aSourceData)
        {
            OSCL_UNUSED_ARG(aSourceData);
            return true;
        }
        virtual bool createCfgFile(OSCL_String& aUri)
        {
            OSCL_UNUSED_ARG(aUri);
            return true;
        }
        virtual bool getProxy(OSCL_String& aProxyName, uint32 &aProxyPort) = 0;
        virtual void setHttpVersion(const uint32 aHttpVersion)
        {
            OSCL_UNUSED_ARG(aHttpVersion);
        }
        virtual bool handleContentRangeUnmatch()
        {
            return true;
        }
        virtual bool downloadUpdateForHttpHeaderAvailable()
        {
            return true;
        }
        virtual void setHttpExtensionHeaderField(OSCL_String &aFieldKey, OSCL_String &aFieldValue, const HttpMethod aMethod = HTTP_GET, const bool aPurgeOnRedirect = false)
        {
            OSCL_UNUSED_ARG(aFieldKey);
            OSCL_UNUSED_ARG(aFieldValue);
            OSCL_UNUSED_ARG(aMethod);
            OSCL_UNUSED_ARG(aPurgeOnRedirect);
        }

        OSCL_IMPORT_REF virtual void handleTimeout(const int32 timerID);
        OSCL_IMPORT_REF virtual bool handleProtocolStateComplete(PVProtocolEngineNodeInternalEvent &aEvent, PVProtocolEngineNodeInternalEventHandler *aEventHandler);

        // for fasttrack only
        virtual PVMFStatus getMediaPresentationInfo(PVMFMediaPresentationInfo& aInfo)
        {
            OSCL_UNUSED_ARG(aInfo);
            return PVMFSuccess;
        }
        virtual PVMFStatus selectTracks(PVMFMediaPresentationInfo& aInfo)
        {
            OSCL_UNUSED_ARG(aInfo);
            return PVMFSuccess;
        }

        //Http status code 409 means low disk space
        virtual bool isHTTP409ForLowDiskSpace(const int32 errorCode)
        {
            OSCL_UNUSED_ARG(errorCode);
            return false;
        }

        // for ms http streaming only
        virtual void setLoggingStartInPause(const bool aLoggingStartInPause = true)
        {
            OSCL_UNUSED_ARG(aLoggingStartInPause);
        }
        virtual bool isLoggingStartInPause()
        {
            return false;
        }
        virtual bool needSendEOSDuetoError(const int32 aErrorCode)
        {
            OSCL_UNUSED_ARG(aErrorCode);
            return false;
        }

        // for progressive download only
        virtual bool needCheckExtraDataComeIn()
        {
            return false;
        }
        virtual bool needCheckEOSAfterDisconnectSocket()
        {
            return false;
        }

        // for progressive playback (special case of progressive download) only
        virtual bool isStreamingPlayback()
        {
            return false;
        }
        virtual bool completeRepositionRequest()
        {
            return false;
        }
        virtual void checkSendResumeNotification()
        {
            ;
        }
        virtual void enableInfoUpdate(const bool aEnabled = true)
        {
            OSCL_UNUSED_ARG(aEnabled);
        }

        OSCL_IMPORT_REF virtual OsclAny* getObject(const NodeObjectType aObjectType);

        virtual void SetSharedLibraryPtr(OsclSharedLibrary* aPtr)
        {
            iOsclSharedLibrary = aPtr;
        }

        /**
         * Retrieves shared library pointer
         * @returns Pointer to the shared library.
         **/
        virtual OsclSharedLibrary* GetSharedLibraryPtr()
        {
            return iOsclSharedLibrary;
        }

    protected:
        OSCL_IMPORT_REF virtual PVMFStatus initImpl();
        virtual int32 initNodeOutput() = 0;
        OSCL_IMPORT_REF bool initProtocol();
        virtual bool initProtocol_SetConfigInfo() = 0;
        virtual void initDownloadControl()
        {
            ;
        }
        OSCL_IMPORT_REF uint32 getBitMaskForHTTPMethod(const HttpMethod aMethod = HTTP_GET);
        // called by handleTimeout()
        virtual bool handleTimeoutInPause(const int32 timerID)
        {
            OSCL_UNUSED_ARG(timerID);
            return false;
        }
        virtual bool handleTimeoutInDownloadStreamingDone(const int32 timerID)
        {
            OSCL_UNUSED_ARG(timerID);
            return false;
        }
        // called by doStop()
        OSCL_IMPORT_REF virtual void sendSocketDisconnectCmd();
        // called by handleTimeout()
        OSCL_IMPORT_REF virtual bool ignoreThisTimeout(const int32 timerID);
        OSCL_IMPORT_REF virtual void clear();

    private:
        //called by createProtocolObjects()
        bool createNetworkTimer();

        // called by handleTimeout()
        bool handleTimeoutErr(const int32 timerID);

        // called by startDataFlowByCommand()
        void checkEOSMsgFromInputPort();
        // called by doClear or doCancelClear()
        void clearInternalEventQueue();

    protected:
        PVMFProtocolEngineNode *iNode;
        ProtocolContainerObserver *iObserver;
        PVLogger *iDataPathLogger;

        // hold references for the following node objects
        HttpBasedProtocol *iProtocol;
        PVMFProtocolEngineNodeOutput *iNodeOutput;
        DownloadControlInterface *iDownloadControl;
        DownloadProgressInterface *iDownloadProgess;
        EventReporter *iEventReport;
        PVDlCfgFileContainer *iCfgFileContainer;
        PVMFDownloadDataSourceContainer *iDownloadSource;
        PVMFProtocolEngineNodeTimer *iNodeTimer;
        InterfacingObjectContainer *iInterfacingObjectContainer;
        UserAgentField *iUserAgentField;

        // pass-in node objects
        PVMFProtocolEnginePort *iPortInForData, *iPortInForLogging, *iPortOut;
        Oscl_Vector<PVProtocolEngineNodeInternalEvent, PVMFProtocolEngineNodeAllocator> *iInternalEventQueue;

        OsclSharedLibrary* iOsclSharedLibrary;
};

////////////////////////////////////////////////////////////////////////////////////

// Using state to handle multiple use cases
enum PVProtocolEngineNodePrcoessingState
{
    ProcessingState_Idle = 0,
    ProcessingState_NormalDataflow,				// normal data flow
    // other states are changed to using event
};

static const TPVMFNodeInterfaceState SetStateByCommand[11] =
{
    EPVMFNodeInitialized,	// PVMF_GENERIC_NODE_INIT=4
    EPVMFNodePrepared,		// PVMF_GENERIC_NODE_PREPARE=5
    EPVMFNodeStarted,		// PVMF_GENERIC_NODE_START=6
    EPVMFNodePrepared,		// PVMF_GENERIC_NODE_STOP=7
    EPVMFNodeStarted,		// PVMF_GENERIC_NODE_FLUSH=8
    EPVMFNodePaused,		// PVMF_GENERIC_NODE_PAUSE=9
    EPVMFNodeCreated,		// PVMF_GENERIC_NODE_RESET=10
    EPVMFNodeLastState,		// PVMF_GENERIC_NODE_CANCELALLCOMMANDS=11
    EPVMFNodeLastState,		// PVMF_GENERIC_NODE_CANCELCOMMAND=12
    EPVMFNodeStarted,		// PVPROTOCOLENGINE_NODE_CMD_SEEK=13,
    EPVMFNodeStarted		// PVPROTOCOLENGINE_NODE_CMD_BITSTREAM_SWITCH=14
};


////////////////////////////////////////////////////////////////////////////////////
//////	PVProtocolEngineNodeInternalEvent definition
////////////////////////////////////////////////////////////////////////////////////

// This structure defines the internal event(constrast to node event) struture, which serves as the basis of
// event-driven handling inside the node
enum PVProtocolEngineNodeInternalEventType
{
    // This group of events comes from the callback/feedback from protocol engine
    PVProtocolEngineNodeInternalEventType_HttpHeaderAvailable = 0,
    PVProtocolEngineNodeInternalEventType_FirstPacketAvailable,
    PVProtocolEngineNodeInternalEventType_NormalDataAvailable,
    PVProtocolEngineNodeInternalEventType_ProtocolStateComplete,

    // This group of events comes from node itself
    PVProtocolEngineNodeInternalEventType_EndOfProcessing,
    PVProtocolEngineNodeInternalEventType_ServerResponseError_Bypassing,
    PVProtocolEngineNodeInternalEventType_ProtocolStateError,
    PVProtocolEngineNodeInternalEventType_CheckResumeNotificationMaually,
    PVProtocolEngineNodeInternalEventType_OutgoingMsgQueuedAndSentSuccessfully,

    // data flow event
    PVProtocolEngineNodeInternalEventType_IncomingMessageReady = 9,
    PVProtocolEngineNodeInternalEventType_HasExtraInputData,
    PVProtocolEngineNodeInternalEventType_OutputDataReady,
    PVProtocolEngineNodeInternalEventType_StartDataflowByCommand,
    PVProtocolEngineNodeInternalEventType_StartDataflowByBufferAvailability,
    PVProtocolEngineNodeInternalEventType_StartDataflowBySendRequestAction,
    PVProtocolEngineNodeInternalEventType_StartDataflowByPortOutgoingQueueReady
};

struct PVProtocolEngineNodeInternalEvent
{
    PVProtocolEngineNodeInternalEventType iEventId;
    OsclAny *iEventInfo; // any other side info except the actual data, such as error code, sequence number(http streaming), seek offset(fasttrack)
    OsclAny *iEventData; // actual data for the event

    // default constructor
    PVProtocolEngineNodeInternalEvent() : iEventId(PVProtocolEngineNodeInternalEventType_HttpHeaderAvailable), iEventInfo(NULL), iEventData(NULL)
    {
        ;
    }

    // constructor with parameters
    PVProtocolEngineNodeInternalEvent(PVProtocolEngineNodeInternalEventType aEventId, OsclAny *aEventInfo, OsclAny *aEventData = NULL)
    {
        iEventId   = aEventId;
        iEventInfo = aEventInfo;
        iEventData = aEventData;
    }

    PVProtocolEngineNodeInternalEvent(const ProtocolEngineOutputDataSideInfo &aSideInfo, const OsclAny *aData = NULL)
    {
        ProtocolEngineOutputDataSideInfo sideInfo = (ProtocolEngineOutputDataSideInfo&) aSideInfo;
        iEventId   = (PVProtocolEngineNodeInternalEventType)((uint32)sideInfo.iDataType);
        iEventInfo = (OsclAny *)sideInfo.iData;
        iEventData = (OsclAny *)aData;
    }
    PVProtocolEngineNodeInternalEvent(PVProtocolEngineNodeInternalEventType aEventId, int32 aInfoCode = 0)
    {
        iEventId   = aEventId;
        iEventInfo = (OsclAny *)aInfoCode;
        iEventData = NULL;
    }

    // copy constructor
    PVProtocolEngineNodeInternalEvent(const PVProtocolEngineNodeInternalEvent &x)
    {
        iEventId   = x.iEventId;
        iEventInfo = x.iEventInfo;
        iEventData = x.iEventData;
    }

    // operator "="
    PVProtocolEngineNodeInternalEvent &operator=(const PVProtocolEngineNodeInternalEvent& x)
    {
        iEventId   = x.iEventId;
        iEventInfo = x.iEventInfo;
        iEventData = x.iEventData;
        return *this;
    }
};

// this structure defines information needed for EndOfDataProcessingHandler, will be as iEventInfo
struct EndOfDataProcessingInfo
{
    bool iSendResumeNotification;
    bool iExtraDataComeIn;
    bool iSendServerDisconnectEvent;
    bool iStreamingDone;
    bool iForceStop;

    // constructor
    EndOfDataProcessingInfo() : iSendResumeNotification(false),
            iExtraDataComeIn(false),
            iSendServerDisconnectEvent(false),
            iStreamingDone(false),
            iForceStop(false) {}

    bool isValid() const
    {
        return (iSendResumeNotification		||
                iExtraDataComeIn			||
                iSendServerDisconnectEvent	||
                iStreamingDone				||
                iForceStop);
    }

    void clear()
    {
        iSendResumeNotification = false;
        iExtraDataComeIn = false;
        iSendServerDisconnectEvent = false;
        iStreamingDone = false;
        iForceStop = false;
    }
};

// this structure defines information needed for ProtocolStateErrorHandler, will be as iEventInfo
struct ProtocolStateErrorInfo
{
    int32 iErrorCode;
    // true means using the current iErrorCode, false means using the previous iErrorCode
    bool iUseInputErrorCode;

    // constructor
    ProtocolStateErrorInfo() : iErrorCode(0), iUseInputErrorCode(true)
    {
        ;
    }
    ProtocolStateErrorInfo(const int32 aErrorCode, const bool aUseInputErrorCode = true) :
            iErrorCode(aErrorCode),
            iUseInputErrorCode(aUseInputErrorCode)
    {
        ;
    }
};

// this structure defines infomation needed for OutgoingMsgSentSuccessHandler, will be as iEventInfo
struct OutgoingMsgSentSuccessInfo
{
    PVMFProtocolEnginePort *iPort;
    PVMFSharedMediaMsgPtr iMsg;

    // constructor
    OutgoingMsgSentSuccessInfo(): iPort(NULL)
    {
        ;
    }
    OutgoingMsgSentSuccessInfo(PVMFProtocolEnginePort *aPort, PVMFSharedMediaMsgPtr &aMsg) :
            iPort(aPort), iMsg(aMsg)
    {
        ;
    }

    OutgoingMsgSentSuccessInfo &operator=(const OutgoingMsgSentSuccessInfo& x)
    {
        iPort = x.iPort;
        iMsg  = x.iMsg;
        return *this;
    }
};

// use polymophism to handle variant events
class PVProtocolEngineNodeInternalEventHandler
{
    public:
        virtual ~PVProtocolEngineNodeInternalEventHandler() {}

        virtual bool handle(PVProtocolEngineNodeInternalEvent &aEvent) = 0;
        virtual bool completePendingCommand(PVProtocolEngineNodeInternalEvent &aEvent);

        // contructor
        PVProtocolEngineNodeInternalEventHandler(PVMFProtocolEngineNode *aNode);

    protected:
        bool isBeingStopped(const int32 aStatus = PROCESS_SUCCESS_END_OF_MESSAGE); // common routine

    private:
        inline bool isCurrEventMatchCurrPendingCommand(uint32 aCurrEventId);
        bool completePendingCommandWithError(PVProtocolEngineNodeInternalEvent &aEvent);
        int32 getBasePVMFErrorReturnCode(const int32 errorCode, const bool isForCommandComplete = true);
        void handleErrResponse(int32 &aBaseCode, int32 &aErrCode, char* &aEventData, uint32 &aEventDataLen);
        void handleAuthenErrResponse(int32 &aErrCode, char* &aEventData, uint32 &aEventDataLen);
        void handleRedirectErrResponse(char* &aEventData, uint32 &aEventDataLen);
        inline bool isStopCmdPending(); // called by isBeingStopped
        inline bool isProtocolStateComplete(const int32 aStatus);

    protected:
        PVMFProtocolEngineNode *iNode;
        PVLogger* iDataPathLogger;

    private:
        OSCL_HeapString<OsclMemAllocator> iAuthenInfoRealm;
};

// ProtocolStateErrorHandling becomes a little bit more complicated due to the new requirement:
// PE node needs to send EOS to downstream node once error happens during streaming
// Previously, PE node just error out without doing that much stuff. Now for this new requirement,
// PE node needs two rounds error handling in this case, at the first round, probably does nothing but
// storing error code and preparing sending EOS, and then at the second round, does the real error handling
// But for other cases (PDL or streaming doesn't really start), just do error handling
class ProtocolStateErrorHandler : public PVProtocolEngineNodeInternalEventHandler
{
    public:
        bool handle(PVProtocolEngineNodeInternalEvent &aEvent);

        // constructor
        ProtocolStateErrorHandler(PVMFProtocolEngineNode *aNode) :
                PVProtocolEngineNodeInternalEventHandler(aNode), iErrorCode(0)
        {
            ;
        }
    private:
        int32 parseServerResponseCode(const int32 aErrorCode, bool &isInfoEvent);
        // return value: 0 means caller needs to return immediately, not 0 means error
        int32 checkRedirectHandling(const int32 aErrorCode);
        bool handleRedirect();
        bool NeedHandleContentRangeUnmatch(const int32 aErrorCode);
        bool handleContentRangeUnmatch();
        bool needCompletePendingCommandAtThisRound(PVProtocolEngineNodeInternalEvent &aEvent);

    private:
        int32 iErrorCode;
};

class HttpHeaderAvailableHandler : public PVProtocolEngineNodeInternalEventHandler
{
    public:
        bool handle(PVProtocolEngineNodeInternalEvent &aEvent);

        // constructor
        HttpHeaderAvailableHandler(PVMFProtocolEngineNode *aNode) :
                PVProtocolEngineNodeInternalEventHandler(aNode)
        {
            ;
        }
};

class FirstPacketAvailableHandler : public PVProtocolEngineNodeInternalEventHandler
{
    public:
        bool handle(PVProtocolEngineNodeInternalEvent &aEvent);

        // constructor
        FirstPacketAvailableHandler(PVMFProtocolEngineNode *aNode) :
                PVProtocolEngineNodeInternalEventHandler(aNode)
        {
            ;
        }
};


class NormalDataAvailableHandler : public PVProtocolEngineNodeInternalEventHandler
{
    public:
        bool handle(PVProtocolEngineNodeInternalEvent &aEvent);

        // constructor
        NormalDataAvailableHandler(PVMFProtocolEngineNode *aNode) :
                PVProtocolEngineNodeInternalEventHandler(aNode)
        {
            ;
        }
};

class ProtocolStateCompleteHandler : public PVProtocolEngineNodeInternalEventHandler
{
    public:
        bool handle(PVProtocolEngineNodeInternalEvent &aEvent);

        // constructor
        ProtocolStateCompleteHandler(PVMFProtocolEngineNode *aNode) :
                PVProtocolEngineNodeInternalEventHandler(aNode)
        {
            ;
        }
};

class NormalDataFlowHandler : public PVProtocolEngineNodeInternalEventHandler
{
    public:
        bool handle(PVProtocolEngineNodeInternalEvent &aEvent);

        // constructor
        NormalDataFlowHandler(PVMFProtocolEngineNode *aNode) : PVProtocolEngineNodeInternalEventHandler(aNode), iSendSocketReconnect(false)
        {
            ;
        }

    private:
        bool flushDataPostProcessing(const int32 aStatusFlushData);
        bool handleEOSLogging();
        bool handleEOS(const int32 aStatus);
        bool handleEndOfProcessing(const int32 aStatus);
        bool dataFlowContinue(const int32 aStatus);
        inline bool isReadyGotoNextState(const int32 aStatus); // called by dataFlowContinueOrStop

    private:
        bool iSendSocketReconnect;
};

class EndOfDataProcessingHandler : public PVProtocolEngineNodeInternalEventHandler
{
    public:
        bool handle(PVProtocolEngineNodeInternalEvent &aEvent);

        // constructor
        EndOfDataProcessingHandler(PVMFProtocolEngineNode *aNode) :
                PVProtocolEngineNodeInternalEventHandler(aNode)
        {
            ;
        }

    private:
        void cleanupForStop(PVProtocolEngineNodeInternalEvent &aEvent);
};


class ServerResponseErrorBypassingHandler : public PVProtocolEngineNodeInternalEventHandler
{
    public:
        bool handle(PVProtocolEngineNodeInternalEvent &aEvent);

        // constructor
        ServerResponseErrorBypassingHandler(PVMFProtocolEngineNode *aNode) :
                PVProtocolEngineNodeInternalEventHandler(aNode)
        {
            ;
        }
};


// This handler is for PVProtocolEngineNodeInternalEventType_CheckResumeNotificationMaually only. This happens in progressive streaming,
// when MBDS is quickly filled up,and parser node hasn't parsed any data. In this special case, PE node will go idle, but parser node
// may send an internal RequestResumeNotification before parsing any data, then deadlock happens.
class CheckResumeNotificationHandler : public PVProtocolEngineNodeInternalEventHandler
{
    public:
        bool handle(PVProtocolEngineNodeInternalEvent &aEvent);

        // constructor
        CheckResumeNotificationHandler(PVMFProtocolEngineNode *aNode) :
                PVProtocolEngineNodeInternalEventHandler(aNode)
        {
            ;
        }
};

class OutgoingMsgSentSuccessHandler : public PVProtocolEngineNodeInternalEventHandler
{
    public:
        bool handle(PVProtocolEngineNodeInternalEvent &aEvent);

        // constructor
        OutgoingMsgSentSuccessHandler(PVMFProtocolEngineNode *aNode) :
                PVProtocolEngineNodeInternalEventHandler(aNode)
        {
            ;
        }
};




////////////////////////////////////////////////////////////////////////////////////
//////	PVMFProtocolEngineNodeOutput
////////////////////////////////////////////////////////////////////////////////////

// Observer class for pvHttpStreamingOutput to notify the node when the output buffer is available since
// the memory pool is created inside pvHttpStreamingOutput
class PVMFProtocolEngineNodeOutputObserver
{
    public:
        virtual ~PVMFProtocolEngineNodeOutputObserver() {}

        // notify the node that the output buffer pool is fool, so hold off any data processing
        virtual void OutputBufferPoolFull() = 0;
        // notify the node that the new output buffer inside the pool is back to available.
        virtual void OutputBufferAvailable() = 0;
        // notify the node that the new data is already written the file and then download control and status
        // should be updated responsively
        virtual void ReadyToUpdateDownloadControl() = 0;
        // notify the node that a media message has been queued in outgoing message queue successfully
        virtual bool QueueOutgoingMsgSentComplete(PVMFProtocolEnginePort *aPort, PVMFSharedMediaMsgPtr &aMsg, const PVMFStatus aStatus) = 0;
};

enum NodeOutputType
{
    NodeOutputType_InputPortForData = 0,
    NodeOutputType_InputPortForLogging,
    NodeOutputType_OutPort,
    NodeOutputType_DataStreamFactory
};

// This base class encapsulates port objects (port and data stream)
class PVMFProtocolEngineNodeOutput
{
    public:
        // set output object such as, input port, output port and data stream factory
        OSCL_IMPORT_REF virtual void setOutputObject(OsclAny* aOutputObject, const uint32 aObjectType = NodeOutputType_InputPortForData);
        virtual void setConfigInfo(OsclAny* aConfig)
        {
            OSCL_UNUSED_ARG(aConfig);
        }
        OSCL_IMPORT_REF virtual bool passDownNewOutputData(OUTPUT_DATA_QUEUE &aOutputQueue, OsclAny* aSideInfo = NULL);
        OSCL_IMPORT_REF virtual int32 flushData(const uint32 aOutputType = NodeOutputType_InputPortForData) = 0;
        virtual int32 initialize(OsclAny* aInitInfo = NULL) = 0;
        virtual int32 reCreateMemPool(uint32 aNumPool)
        {
            OSCL_UNUSED_ARG(aNumPool);
            return PROCESS_SUCCESS;
        }
        virtual uint32 getNumBuffersInMediaDataPool()
        {
            return 0;
        }
        OSCL_IMPORT_REF bool getBuffer(PVMFSharedMediaDataPtr &aMediaData, uint32 aRequestSize = PVHTTPDOWNLOADOUTPUT_CONTENTDATA_CHUNKSIZE);
        OSCL_IMPORT_REF virtual void discardData(const bool aNeedReopen = false);
        OSCL_IMPORT_REF virtual bool isPortBusy();

        // PPB (progressive streaming/playback)
        virtual bool releaseMemFrag(OsclRefCounterMemFrag* aFrag)
        {
            OSCL_UNUSED_ARG(aFrag);
            return false;
        };
        virtual void setContentLength(uint32 aLength)
        {
            OSCL_UNUSED_ARG(aLength);
        };
        virtual void dataStreamCommandCompleted(const PVMFCmdResp& aResponse)
        {
            OSCL_UNUSED_ARG(aResponse);
        };
        virtual void setDataStreamSourceRequestObserver(PvmiDataStreamRequestObserver* aObserver)
        {
            OSCL_UNUSED_ARG(aObserver);
        };
        virtual void flushDataStream()
        {
            ;
        }
        virtual bool seekDataStream(const uint32 aSeekOffset)
        {
            OSCL_UNUSED_ARG(aSeekOffset);
            return true;
        };

        // get info from output object to serve as the basis for status update
        uint32 getCurrentOutputSize()
        {
            return iCurrTotalOutputSize;
        }
        void setCurrentOutputSize(const uint32 aCurrentSize)
        {
            iCurrTotalOutputSize = aCurrentSize;    // used in resume download
        }
        virtual uint32 getCurrentPlaybackTime()
        {
            return 0;
        }
        // in case of progressive streaming, the following two sizes mean available cache size and maximum cache size
        virtual uint32 getAvailableOutputSize()
        {
            return 0xFFFFFFFF;
        }
        virtual uint32 getMaxAvailableOutputSize()
        {
            return 0;
        }

        // constructor and destructor
        OSCL_IMPORT_REF PVMFProtocolEngineNodeOutput(PVMFProtocolEngineNodeOutputObserver *aObserver = NULL);
        OSCL_IMPORT_REF virtual ~PVMFProtocolEngineNodeOutput();

    protected:
        // for media data
        OSCL_IMPORT_REF PVMFStatus createMemPool();
        OSCL_IMPORT_REF void deleteMemPool();
        // create media data for composing http request
        OSCL_IMPORT_REF bool createMediaData(PVMFSharedMediaDataPtr &aMediaData, uint32 aRequestSize);
        OSCL_IMPORT_REF bool sendToDestPort(PVMFSharedMediaDataPtr &aMediaData, PVMFProtocolEnginePort *aPort);

        // send the input media data to the port
        OSCL_IMPORT_REF virtual bool sendToPort(PVMFSharedMediaDataPtr &aMediaData, const uint32 aPortType = NodeOutputType_InputPortForData);

        // reset
        OSCL_IMPORT_REF virtual void reset();

    protected:
        PVMFProtocolEnginePort *iPortIn;  // input port connecting to socket node, both for download and http streaming

        // Output buffer memory pool
        OsclMemPoolFixedChunkAllocator *iContentDataMemPool;

        // Allocator for simple media data buffer
        PVMFSimpleMediaBufferCombinedAlloc *iMediaDataAlloc;

        // Memory pool for simple media data
        PVMFMemPoolFixedChunkAllocator iMediaDataMemPool;

        PVMFSharedMediaDataPtr iMediaData;
        Oscl_Vector<OUTPUT_DATA_QUEUE, PVMFProtocolEngineNodeAllocator> iOutputFramesQueue;
        // Mem frags in this queue are being used by the data stream
        PENDING_OUTPUT_DATA_QUEUE iPendingOutputDataQueue;

        // observer to notify the node when output buffer is available
        PVMFProtocolEngineNodeOutputObserver *iObserver;

        // current total output size, serves as the basis for status update
        uint32 iCurrTotalOutputSize;

        PVLogger* iLogger;
        PVLogger* iDataPathLogger;
        PVLogger* iClockLogger;
};


////////////////////////////////////////////////////////////////////////////////////
//////	DownloadControlInterface
////////////////////////////////////////////////////////////////////////////////////
enum DownloadControlSupportObjectType
{
    DownloadControlSupportObjectType_SupportInterface = 0,
    DownloadControlSupportObjectType_ProgressInterface,
    DownloadControlSupportObjectType_EnginePlaybackClock,
    DownloadControlSupportObjectType_ProtocolEngine,
    DownloadControlSupportObjectType_ConfigFileContainer,
    DownloadControlSupportObjectType_SDPInfoContainer,
    DownloadControlSupportObjectType_DownloadProgress,
    DownloadControlSupportObjectType_OutputObject
};

// The intent of introducing this download ocntrol interface is to make streaming counterpart as a NULL object,
// which remove any error-prone check
class DownloadControlInterface
{
    public:
        // set download control supporting objects:
        virtual void setSupportObject(OsclAny *aDLSupportObject, DownloadControlSupportObjectType aType) = 0;
        // From PVMFDownloadProgressInterface API pass down
        virtual void requestResumeNotification(const uint32 currentNPTReadPosition, bool& aDownloadComplete, bool& aNeedSendUnderflowEvent) = 0;
        // check whether to make resume notification; if needed, then make resume notification
        virtual void cancelResumeNotification() = 0;
        // cancel request of resume notification.
        virtual int32 checkResumeNotification(const bool aDownloadComplete = true) = 0;
        // return true for the new download progress
        // From PVMFDownloadProgressInterface API
        virtual void getDownloadClock(OsclSharedPtr<PVMFMediaClock> &aClock) = 0;
        // From PVMFDownloadProgressInterface API
        virtual void setClipDuration(const uint32 aClipDurationMsec) = 0;
        // for auto-resume control for resume download
        virtual void setPrevDownloadSize(uint32 aPrevDownloadSize = 0) = 0;
        virtual void clear() = 0;

        // for progressive playback repositioning
        virtual void clearPerRequest() = 0;
        virtual bool isDownloadCompletedPerRequest() = 0;

        // destructor
        virtual ~DownloadControlInterface()
        {
            ;
        }
};

////////////////////////////////////////////////////////////////////////////////////
//////	DownloadProgressInterface
////////////////////////////////////////////////////////////////////////////////////

// This class encapsulates the download progress update based on different download progress modes
class DownloadProgressInterface
{
    public:
        virtual void setSupportObject(OsclAny *aDLSupportObject, DownloadControlSupportObjectType aType) = 0;

        // updata download clock and download progress
        virtual bool update(const bool aDownloadComplete = false) = 0;

        // return true for the new download progress
        virtual bool getNewProgressPercent(uint32 &aProgressPercent) = 0;

        // return duration for auto-resume decision
        virtual void setClipDuration(const uint32 aClipDurationMsec) = 0;

        virtual void setDownloadProgressMode(DownloadProgressMode aMode = DownloadProgressMode_TimeBased) = 0;

        virtual ~DownloadProgressInterface()
        {
            ;
        }
};

////////////////////////////////////////////////////////////////////////////////////
//////	EventReporter
////////////////////////////////////////////////////////////////////////////////////
// This class wraps up sending node info&error event.
enum EventReporterSupportObjectType
{
    EventReporterSupportObjectType_DownloadProgress = 0,
    EventReporterSupportObjectType_ProtocolEngine,
    EventReporterSupportObjectType_ConfigFileContainer,
    EventReporterSupportObjectType_NodeInterfacingObject,
    EventReporterSupportObjectType_TimerObject,
    EventReporterSupportObjectType_OutputObject
};

class EventReporterObserver
{
    public:
        virtual void ReportEvent(PVMFEventType aEventType, OsclAny* aEventData = NULL, const int32 aEventCode = 0, OsclAny* aEventLocalBuffer = NULL, const uint32 aEventLocalBufferSize = 0) = 0;
        virtual void NotifyContentTooLarge() = 0;
        virtual uint32 GetObserverState() = 0;
};

class EventReporter
{
    public:
        virtual ~EventReporter() {}

        // constructor
        OSCL_IMPORT_REF EventReporter(EventReporterObserver *aObserver);

        virtual void setSupportObject(OsclAny *aSupportObject, EventReporterSupportObjectType aType)
        {
            OSCL_UNUSED_ARG(aSupportObject);
            OSCL_UNUSED_ARG(aType);
        }

        // major function to walk throught all the node info & error event whether to be sent out
        virtual bool checkReportEvent(const uint32 downloadStatus)
        {
            OSCL_UNUSED_ARG(downloadStatus);
            return true;
        }
        // the content-length event and content too large event, can't fit into the above checkReportEvent() completely
        virtual bool checkContentInfoEvent(const uint32 downloadStatus)
        {
            OSCL_UNUSED_ARG(downloadStatus);
            return true;
        }

        // enable some specific events
        virtual void sendDataReadyEvent()
        {
            ;
        }
        virtual void enableBufferingCompleteEvent()
        {
            ;
        }
        virtual void sendBufferStatusEvent()
        {
            ;
        }

        OSCL_IMPORT_REF virtual void clear();

        // node is running in start state to kick off the normal downloading and streaming
        OSCL_IMPORT_REF void startRealDataflow();

    protected:
        bool iStarted;
        EventReporterObserver *iObserver;
        PVLogger *iDataPathLogger;
};


// This class wraps up user agent setting, differentiated in progessive download, fastrack and ms http streaming.
// Any this kind of variation should be wrapped up into an object
class UserAgentField
{
    public:
        // constructor
        UserAgentField() : iOverwritable(false)
        {
            ;
        }
        OSCL_IMPORT_REF UserAgentField(OSCL_wString &aUserAgent, const bool isOverwritable = false);
        OSCL_IMPORT_REF UserAgentField(OSCL_String &aUserAgent, const bool isOverwritable = false);
        virtual ~UserAgentField()
        {
            ;
        }

        // set user agent
        OSCL_IMPORT_REF bool setUserAgent(OSCL_wString &aUserAgent, const bool isOverwritable = false);
        OSCL_IMPORT_REF bool setUserAgent(OSCL_String &aUserAgent, const bool isOverwritable = false);

        // get the actual user agent (not wide string version) based on overwrite mode or replace mode (attach the input user agent to the default one)
        OSCL_IMPORT_REF bool getUserAgent(OSCL_String &aUserAgent);

    protected:
        virtual void getDefaultUserAgent(OSCL_String &aUserAgent) = 0;

    protected:
        OSCL_HeapString<OsclMemAllocator> iInputUserAgent;
        OSCL_HeapString<OsclMemAllocator> iActualUserAgent;
        bool iOverwritable;
};

typedef Oscl_Vector<OutgoingMsgSentSuccessInfo, PVMFProtocolEngineNodeAllocator> OutgoingMsgSentSuccessInfoVec;

// This class interfaces between the node and node user, which is in fact a data holder and holds the data set by the node public APIs
// and some output data return to node user
class InterfacingObjectContainer
{
    public:
        // set and get download format
        void setDownloadFormat(PVMFFormatType &aDownloadFormat)
        {
            iDownloadFormat = (PVMFFormatType)aDownloadFormat;
        }
        PVMFFormatType getDownloadFormat() const
        {
            return iDownloadFormat;
        }

        // set and get url
        void setURI(OSCL_String &aUri, const bool aRedirectUri = false)
        {
            iDownloadURI.setURI(aUri, aRedirectUri);
        }
        void setURI(OSCL_wString &aUri, const bool aRedirectUri = false)
        {
            iDownloadURI.setURI(aUri, aRedirectUri);
        }
        INetURI &getURIObject()
        {
            return iDownloadURI;
        }

        void setLoggingURI(OSCL_String &aUri)
        {
            iLoggingURI.setURI(aUri);
        }
        void setLoggingURI(OSCL_wString &aUri)
        {
            iLoggingURI.setURI(aUri);
        }
        INetURI &getLoggingURIObject()
        {
            return iLoggingURI;
        }

        // set and get data stream factory
        void setDataStreamFactory(const PVMFDataStreamFactory *aDataStreamFactory)
        {
            iDataStreamFactory = (PVMFDataStreamFactory *)aDataStreamFactory;
        }
        PVMFDataStreamFactory *getDataStreamFactory()
        {
            return iDataStreamFactory;
        }

        // set and get stream parameters in http streaming
        void setStreamParams(PVMFProtocolEngineNodeMSHTTPStreamingParams &aStreamParams)
        {
            iStreamParams = aStreamParams;
        }
        PVProtocolEngineMSHttpStreamingParams *getStreamParams()
        {
            return &iStreamParams;
        }

        //set fast cache setting
        void SetAccelBitrate(uint32 aAccelBitrate)
        {
            iStreamParams.iAccelBitrate = aAccelBitrate;
        }
        void SetAccelDuration(uint32 aAccelDuration)
        {
            iStreamParams.iAccelDuration = aAccelDuration;
        }

        //set max streaming size
        void SetMaxHttpStreamingSize(uint32 aMaxHttpStreamingSize)
        {
            iStreamParams.iMaxHttpStreamingSize = aMaxHttpStreamingSize;
        }

        // set and get number of buffers in media message allocator in http streaming
        void setMediaMsgAllocatorNumBuffers(const uint32 aNumBuffersInAllocator)
        {
            iNumBuffersInAllocator = aNumBuffersInAllocator;
        }
        uint32 getMediaMsgAllocatorNumBuffers() const
        {
            return iNumBuffersInAllocator;
        }

        // set and get number of redirect trials
        void setNumRedirectTrials(const uint32 aNumRedirectTrials)
        {
            iNumRedirectTrials = aNumRedirectTrials;
        }
        void setCurrNumRedirectTrials(const uint32 aCurrNumRedirectTrials)
        {
            iCurrRedirectTrials = aCurrNumRedirectTrials;
        }
        uint32 getNumRedirectTrials() const
        {
            return iNumRedirectTrials;
        }
        uint32 getCurrNumRedirectTrials() const
        {
            return iCurrRedirectTrials;
        }

        // set and get http header
        // return the actual http header length, 0 means no header
        OSCL_IMPORT_REF uint32 setHttpHeader(OUTPUT_DATA_QUEUE &aHttpHeader);
        void getHTTPHeader(uint8*& aHeader, uint32& aHeaderLen)
        {
            aHeader = (uint8*)iHttpHeaderBuffer;
            aHeaderLen = iHttpHeaderLength;
        }

        // set and get file size
        void setFileSize(const uint32 aFileSize)
        {
            iFileSize = aFileSize;
        }
        uint32 getFileSize() const
        {
            return iFileSize;
        }

        // socket connect flags
        void updateSocketConnectFlags(const bool isEOS)
        {
            if (!iCurrSocketConnection && !isEOS) iSocketReconnectCmdSent = false; // when connnection becomes from down (EOS) to up, clear the flag of sending socket reconnect command
            iPrevSocketConnection = iCurrSocketConnection;
            iCurrSocketConnection = !isEOS;
        }
        bool isSocketConnectionUp() const
        {
            return iCurrSocketConnection;
        }
        bool isPrevSocketConnectionUp() const
        {
            return iPrevSocketConnection;
        }
        bool isSocketReconnectCmdSent() const
        {
            return iSocketReconnectCmdSent;
        }
        void setSocketReconnectCmdSent(const bool aSocketReconnectCmdSent = true)
        {
            iSocketReconnectCmdSent = aSocketReconnectCmdSent;
        }
        bool ignoreCurrentInputData() const
        {
            return isCurrentInputDataUnwanted;
        }
        void setInputDataUnwanted(const bool aInputDataUnwanted = true)
        {
            isCurrentInputDataUnwanted = aInputDataUnwanted;
        }

        // KeepAlive timeout
        void setKeepAliveTimeout(const uint32 aTimeout)
        {
            iKeepAliveTimeout = aTimeout;
        }
        uint32 getKeepAliveTimeout() const
        {
            return iKeepAliveTimeout;
        }

        // Streaming proxy
        OSCL_IMPORT_REF bool setStreamingProxy(OSCL_wString& aProxyName, const uint32 aProxyPort = DEFAULT_HTTP_PORT_NUMBER);
        void getStreamingProxy(OSCL_String& aProxyName, uint32 &aProxyPort)
        {
            aProxyName = iProxyName;
            aProxyPort = iProxyPort;
        }

        // flag of disabling HTTP HEAD request
        void setHttpHeadRequestDisabled(const bool aDisableHeadRequest = true)
        {
            iDisableHeadRequest = aDisableHeadRequest;
        }
        bool getHttpHeadRequestDisabled() const
        {
            return iDisableHeadRequest;
        }

        // maximum ASF header size
        void setMaxASFHeaderSize(const uint32 aMaxASFHeaderSize)
        {
            iMaxASFHeaderSize = aMaxASFHeaderSize;
        }
        uint32 getMaxASFHeaderSize() const
        {
            return iMaxASFHeaderSize;
        }

        // latest packet number just sent to downstream node successfully
        void setLatestPacketNumSent(const uint32 aPacketNum)
        {
            iLatestDataPacketNumSent = aPacketNum;
        }
        uint32 getLatestPacketNumSent() const
        {
            return iLatestDataPacketNumSent;
        }

        // user-id and password for HTTP authentication
        void setUserAuthInfo(OSCL_String &aUserID, OSCL_String &aPasswd)
        {
            if (aUserID.get_size() > 0)
            {
                iStreamParams.iUserID = OSCL_HeapString<OsclMemAllocator> (aUserID.get_cstr(), aUserID.get_size());
            }
            if (aPasswd.get_size() > 0)
            {
                iStreamParams.iUserPasswd = OSCL_HeapString<OsclMemAllocator> (aPasswd.get_cstr(), aPasswd.get_size());
            }
        }

        OSCL_IMPORT_REF void setNumBuffersInMediaDataPoolSMCalc(uint32 aVal);
        uint32 getNumBuffersInMediaDataPoolSMCalc() const
        {
            return iNumBuffersInMediaDataPoolSMCalc;
        }
        // iOutputPortConnected
        void setOutputPortConnect(const bool aConnected = true)
        {
            iOutputPortConnected = aConnected;
        }
        bool getOutputPortConnect() const
        {
            return iOutputPortConnected;
        }

        // iCancelCmdHappened
        void setCancelCmdHappened(const bool aCancelCmdHappened = true)
        {
            iCancelCmdHappened = aCancelCmdHappened;
        }
        bool getCancelCmdHappened() const
        {
            return iCancelCmdHappened;
        }

        // iEOPInfo
        EndOfDataProcessingInfo *getEOPInfo()
        {
            return &iEOPInfo;
        }

        // protocol state complete info
        void setProtocolStateCompleteInfo(const ProtocolStateCompleteInfo &aInfo, const bool aForceSet = false)
        {
            if (!aForceSet)
            {
                if (aInfo.isDownloadStreamingDone) iProtocolStateCompleteInfo.isDownloadStreamingDone = true;
                if (aInfo.isWholeSessionDone)	  iProtocolStateCompleteInfo.isWholeSessionDone = true;
                if (aInfo.isEOSAchieved)			  iProtocolStateCompleteInfo.isEOSAchieved = true;
            }
            else
            {
                iProtocolStateCompleteInfo.isDownloadStreamingDone = aInfo.isDownloadStreamingDone;
                iProtocolStateCompleteInfo.isWholeSessionDone	   = aInfo.isWholeSessionDone;
                iProtocolStateCompleteInfo.isEOSAchieved		   = aInfo.isEOSAchieved;
            }
        }
        ProtocolStateCompleteInfo *getProtocolStateCompleteInfo()
        {
            return &iProtocolStateCompleteInfo;
        }

        OutgoingMsgSentSuccessInfoVec *getOutgoingMsgSentSuccessInfoVec()
        {
            return &iOutgoingMsgSentSuccessInfoVec;
        }

        bool isDownloadStreamingDone()
        {
            return iProtocolStateCompleteInfo.isDownloadStreamingDone;
        }
        bool isWholeSessionDone()
        {
            return iProtocolStateCompleteInfo.isWholeSessionDone;
        }
        bool isEOSAchieved()
        {
            return iProtocolStateCompleteInfo.isEOSAchieved;
        }

        void setTruncatedForLimitSize(const bool aTruncatedForLimitSize = false)
        {
            iTruncatedForLimitSize = aTruncatedForLimitSize;
        }
        bool getTruncatedForLimitSize() const
        {
            return iTruncatedForLimitSize;
        }

        // constructor
        InterfacingObjectContainer();
        ~InterfacingObjectContainer()
        {
            clear();
            iOutgoingMsgSentSuccessInfoVec.clear();
        }

        // clear
        void clear()
        {
            iHttpHeaderLength			= 0;
            iFileSize					= 0;
            iSocketReconnectCmdSent		= false;
            iCurrRedirectTrials			= 0;
            isCurrentInputDataUnwanted	= true; // when clear(), treat all the input data unwanted (that needs to be ignored), let command and event to enable it
            iProcessingDone				= false;
            iKeepAliveTimeout			= 0;
            iDisableHeadRequest			= true; // changed on the request of Japan
            iMaxASFHeaderSize			= 0;
            iCancelCmdHappened			= false;
            iTruncatedForLimitSize		= false;
            iProtocolStateCompleteInfo.clear();
        }

    private:
        PVMFFormatType	iDownloadFormat;
        // set by SetSourceInitializationData()
        INetURI iDownloadURI;
        // set by SetLoggingURL
        INetURI iLoggingURI;
        // set by PassDatastreamFactory()
        PVMFDataStreamFactory *iDataStreamFactory;
        // set by SetStreamParams()
        PVProtocolEngineMSHttpStreamingParams iStreamParams;
        // set by SetMediaMsgAllocatorNumBuffers()
        uint32 iNumBuffersInAllocator;
        // set by SetNumRedirectTrials
        uint32 iNumRedirectTrials;
        uint32 iCurrRedirectTrials;

        uint32 iNumBuffersInMediaDataPoolSMCalc;
        // get from GetHTTPHeader()
        char iHttpHeaderBuffer[PVHTTPDOWNLOADOUTPUT_CONTENTDATA_CHUNKSIZE+1]; // to hold http header
        uint32 iHttpHeaderLength;
        // get from GetFileSize()
        uint32 iFileSize;

        // socket connect flags
        bool iCurrSocketConnection; // true means the connection is up and on ; false means the connection is downn
        bool iPrevSocketConnection; // the status of previous socket connection
        bool iSocketReconnectCmdSent; // flag to record whether the socket reconnect command is sent

        // this flag is introduced to ignore unwanted incoming messages in certain scenarios,
        // such as stop/reset/cancel, protocol state complete/error, while init/prepare/start command would disable
        // this flag
        bool isCurrentInputDataUnwanted;

        bool iProcessingDone; // work as a global variable

        uint32 iKeepAliveTimeout;

        // streaming proxy and port
        OSCL_HeapString<OsclMemAllocator> iProxyName;
        uint32 iProxyPort;

        bool iDisableHeadRequest;
        uint32 iMaxASFHeaderSize;
        uint32 iLatestDataPacketNumSent;

        // the connection status between PE node output port and downstream(JB) node input port
        // this flag will help sending EOS or not
        bool iOutputPortConnected;

        bool iCancelCmdHappened;

        // work as a global variable
        EndOfDataProcessingInfo iEOPInfo;
        ProtocolStateCompleteInfo iProtocolStateCompleteInfo;
        OutgoingMsgSentSuccessInfoVec iOutgoingMsgSentSuccessInfoVec;

        // This flag mean data-size reach limitation or not
        // true : data reached limitation
        // false: data don't reach limitation
        bool iTruncatedForLimitSize;
};


////////////////////////////////////////////////////////////////////////////////////
//////	PVMFProtocolEngineNodeTimer
////////////////////////////////////////////////////////////////////////////////////

// This class wraps OsclTimer<allocator> to hide some details and make call more expressive
struct TimerUnit
{
    uint32 iTimerID;
    uint32 iTimeout;

    // constructor
    TimerUnit() : iTimerID(0), iTimeout(0)
    {
        ;
    }
    TimerUnit(const uint32 aTimerID, const uint32 aTimeout) : iTimerID(aTimerID), iTimeout(aTimeout)
    {
        ;
    }
};

class PVMFProtocolEngineNodeTimer
{
    public:
        // factory and destructor
        OSCL_IMPORT_REF static PVMFProtocolEngineNodeTimer* create(OsclTimerObserver *aObserver);
        OSCL_IMPORT_REF ~PVMFProtocolEngineNodeTimer();

        // register and set timer id and timeout value
        // note that if aTimeout=0, that means using the internal default one
        OSCL_IMPORT_REF void set(const uint32 aTimerID, const int32 aTimeout = 0);

        // get the timeout value for specific timer id. If not set, return default value
        // if input timer id is not registered, will return 0xffffffff
        OSCL_IMPORT_REF uint32 getTimeout(const uint32 aTimerID);

        // start the timer with optional new timeout value, aTimeout=0 means using the existing one
        // if the timer id doesn't exist, it will return failure, which means set() has to be called before start()
        OSCL_IMPORT_REF bool start(const uint32 aTimerID, const int32 aTimeout = 0);

        OSCL_IMPORT_REF void cancel(const uint32 aTimerID);
        OSCL_IMPORT_REF void clear();

        // clear all the timers except the timer with the given timer ID
        OSCL_IMPORT_REF void clearExcept(const uint32 aTimerID);

    private:
        // constructor
        PVMFProtocolEngineNodeTimer() : iWatchdogTimer(NULL)
        {
            ;
        }
        bool construct(OsclTimerObserver *aObserver);

        uint32 getDefaultTimeout(const uint32 aTimerID);
        uint32 getTimerVectorIndex(const uint32 aTimerID);

    private:
        OsclTimer<PVMFProtocolEngineNodeAllocator>	*iWatchdogTimer;
        Oscl_Vector<TimerUnit, PVMFProtocolEngineNodeAllocator> iTimerVec;
};

#endif

