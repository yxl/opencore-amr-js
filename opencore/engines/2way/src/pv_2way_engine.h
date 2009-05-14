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
#ifndef PV_2WAY_ENGINE_H_INCLUDED
#define PV_2WAY_ENGINE_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef OSCL_SCHEDULER_AO_H_INCLUDED
#include "oscl_scheduler_ao.h"
#endif

#ifndef OSCL_TIMER_H_INCLUDED
#include "oscl_timer.h"
#endif

#ifndef PVMF_MEDIA_CLOCK_H_INCLUDED
#include "pvmf_media_clock.h"
#endif

#ifndef OSCL_BYTE_ORDER_H_INCLUDED
#include "oscl_byte_order.h"
#endif

#ifndef PV_2WAY_INTERFACE_H_INCLUDED
#include "pv_2way_interface.h"
#endif

#ifndef PV_2WAY_BASIC_INTERFACES_H_INCLUDE
#include "pv_2way_basic_interfaces.h"
#endif

#ifndef PVMF_NODE_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
#endif

#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif

#ifndef PV_MP4_H263_ENC_EXTENSION_H_INCLUDED
#include "pvmp4h263encextension.h"
#endif

// COMM and Stack related
#ifndef NO_2WAY_324
#ifndef TSCMAIN_H_INCLUDED // Gkl
#include "tscmain.h"
#endif
#endif

#define MAX_TX_AUDIO_CODECS_SUPPORTED 2
#define MAX_RX_AUDIO_CODECS_SUPPORTED 2
#define MAX_TX_VIDEO_CODECS_SUPPORTED 3
#define MAX_RX_VIDEO_CODECS_SUPPORTED 3

#define MAX_LOGICAL_CHANNEL_PARAMS 3
#define MAX_PENDING_2WAY_COMMANDS 20
#define MAX_PENDING_2WAY_EVENTS 20 // Gkl
#define MAX_PENDING_2WAY_ERRORS 20 // Gkl
#define MAX_PENDING_2WAY_NODE_COMMANDS 20
#define MAX_2WAY_SOURCESINK 2
#define MAX_SESSION_CHANNELS 4
#define MAX_SESSION_MEDIA_CAPS MAX_SESSION_CHANNELS*2


#if defined (__WINS__)
#define AUDIO_INPUT_FILENAME _STRLIT("c:\\audio_in.if2")
#define AUDIO_OUTPUT_FILENAME _STRLIT("c:\\audio_out.if2")
#else
#define AUDIO_INPUT_FILENAME _STRLIT("e:\\audio_in.if2")
#define AUDIO_OUTPUT_FILENAME _STRLIT("e:\\audio_out.if2")
#endif

#define MAX_AMR_BITRATE 12200
#define MIN_AMR_BITRATE 12200
#define MAX_AUDIO_BITRATE 12200

#define DEFAULT_MIN_IFRAME_REQ_INT 1
#define IFRAME_REQ_TIMERID 1


#define END_SESSION_TIMER_ID 2
#define END_SESSION_TIMER "end_session_timer"
#define END_SESSION_TIMER_FREQUENCY 5
#define END_SESSION_TIMER_VALUE 3
/* End session interval = END_SESSION_TIMER_VALUE * 1000/END_SESSION_TIMER_FREQUENCY */

#define REMOTE_DISCONNECT_TIMER_ID 3
#define REMOTE_DISCONNECT_TIMER "remote_disconnect_timer"
#define REMOTE_DISCONNECT_TIMER_FREQUENCY 5
#define REMOTE_DISCONNECT_TIMER_VALUE 3



#define MAX_VIDEO_BITRATE 40000

#define VIDEO_ENCODER_WIDTH 176
#define VIDEO_ENCODER_HEIGHT 144
#if defined (__WINS__)
#define VIDEO_ENCODER_FRAME_RATE 2
#else
#define VIDEO_ENCODER_FRAME_RATE 7.5
#endif
// TBD ARI: This should be removed when capability exchange between video encoder and video source is implemented
#define VIDEO_INPUT_FRAME_RATE VIDEO_ENCODER_FRAME_RATE
#define VIDEO_ENCODER_BITRATE 42000
#define VIDEO_ENCODER_RVLC 0
#define VIDEO_ENCODER_DATA_PARTITIONING 1
#define VIDEO_ENCODER_I_FRAME_INTERVAL 5
#define VIDEO_ENCODER_RATE_CONTROL PVMFVEN_RATE_CONTROL_CBR
#define VIDEO_ENCODER_INPUT_FORMAT PVMFVEN_INPUT_YUV420
#define VIDEO_ENCODER_SEGMENT_SIZE 256

//Play defaults
#define DEFAULT_PLAY_FROM_FILE_TYPE PVMF_MIME_MPEG4FF
#define DEFAULT_PLAY_FROM_FILE_AUDIO PVMF_MIME_AMR_IETF
#define DEFAULT_PLAY_FROM_FILE_VIDEO PVMF_MIME_YUV420

//Gkl
#if defined(PV_RECORD_TO_FILE_SUPPORT)
#define RECORDED_FILESIZE_NOTIFICATION_TIMERID 2
#define MIN_RECORDED_FILESIZE_NOTIFICATION_INTERVAL 500
#endif

#if defined(PV_PLAY_FROM_FILE_SUPPORT)
//Play defaults
#define DEFAULT_PLAY_FROM_FILE_TYPE PVMF_MIME_MPEG4FF
#define DEFAULT_PLAY_FROM_FILE_AUDIO PVMF_MIME_AMR_IETF
#define DEFAULT_PLAY_FROM_FILE_VIDEO PVMF_MIME_YUV420
#endif


class PvmfNodesSyncControlInterface;
typedef enum
{
    EPV2WayAudio = 0,
    EPV2WayVideo,
    EPV2WayMuxData
} TPV2WayMediaType;

typedef enum
{
    EPV2WayAudioIn = 0,
    EPV2WayAudioOut,
    EPV2WayVideoIn,
    EPV2WayVideoOut
} TPV2WayPortTagType;

/**
 * TPV2WayCommandType enum
 *
 *  Enumeration of types of commands that can be issued to the pv2way(Used internall by engine).
 *  The comments for each command type specify the possible error codes that can be
 *  returned with the completion of each command.  For details on each command, pls refer the
 *  interface definition
 *
 **/

enum TPV2WayCommandType
{
    /**
     * Indicates completion of the initialization process.   If successful, the terminal
     * should be in the ESetup state.  If failed, the pv2way remains in the
     * Eidle state.  The derived class contains information about which
     * components failed to initialize.
     *
     * The application must wait for this callback after the terminal is instantiated
     * before calling any of the terminal APIs.
     *
     * If it fails, a system-wide error code is returned.
     *
     **/
    PVT_COMMAND_INIT,
    /**
     * Indicates completion of a GetSDKInfoL call. If successful, the reference to the
     * TPVSDKInfo struct passed in will be populated with version information pertaining
     * to the pv2way SDK
     * If it fails, a system-wide error code is returned.
     *
     **/
    PVT_COMMAND_GET_SDK_INFO,
    /**
     * Indicates completion of a GetSDKModuleInfoL call. If successful, the reference to the
     * TPVSDKModuleInfo struct passed in will be populated with version information pertaining
     * to the currently loaded modules
     * If it fails, a system-wide error code is returned.
     *
     **/
    PVT_COMMAND_GET_SDK_MODULE_INFO,
    /**
     * Indicates completion of a GetPV2WayStateL call. If successful, the reference to the
     * TPV2WayState struct passed in will be populated with current state information.
     * If it fails, a system-wide error code is returned.
     *
     **/
    PVT_COMMAND_GET_PV2WAY_STATE,
    /**
     * Indicates that the terminal has released all resources and is in the EIdle
     * state.  It is now ok to de-allocate the terminal.
     * If it fails, a system-wide error code is returned.
     **/
    PVT_COMMAND_RESET,
    /**
     * This indicates the completion status of a previously issued
     * AddDataSourceL command.
     *
     * If successful, the engine will start the source and either start demuxing (COMM source)
     * or, sending media data from the source (mic, camera etc).
     *
     * On failure, a system-wide error code is returned.
     * The application may either re-try AddDataSourceL, or disconnect
     * the call.   The application does not have to call RemoveDataSourceL.
     *
     * Information about the track - codec, bitrate etc.  are provided via the derived
     * class
     **/
    PVT_COMMAND_ADD_DATA_SOURCE,
    /**
     * This  indicates the completion status of a previously issued RemoveMediaSource command.
     *
     * If successful, the track is closed and all resources associated with the track
     * have been released.
     *
     * Once this callback is invoked for a track, the pointer to CPVTrackInfo
     * for the track will be invalid.
     *
     * If it fails, a system-wide error code is returned.
     *
     **/
    PVT_COMMAND_REMOVE_DATA_SOURCE,
    /**
     * This indicates the completion status of a previously issued
     * AddDataSinkL command.  It also provides track information.
     *
     * If successful, the engine will start sending media data to the sink.
     *
     * If it fails, a system-wide error code is returned.
     * The application may either re-try AddDataSinkL, or disconnect
     * the call.   The application does not have to call RemoveMediaSink.
     *
     * Information about the track - codec, bitrate etc.  are provided via the derived
     * class
     **/
    PVT_COMMAND_ADD_DATA_SINK,
    /**
     * Similar to RemoveMediaSource
     **/
    PVT_COMMAND_REMOVE_DATA_SINK,
    /**
     * Indicates completion of a pending Connect command.  If successful,
     * the application can open the media tracks using the AddMediaSource API.
     *
     * If Connnect fails, a system-wide error code is returned.
     * The application can either re-try Connect or de-allocate the terminal.
     * The Phone application does not have to call Disconnect if ConnectComplete is
     * called with a status that indicates failure.
     *
     **/
    PVT_COMMAND_CONNECT,
    /**
     * Indicates completion of a pending Disconnect command.   The terminal
     * transitions from EDisconnecting back to the ESetup state.   The Phone
     * application may now call Connect again or reset and de-allocate the terminal.
     * If it fails, a system-wide error code is returned.
     *
     **/
    PVT_COMMAND_DISCONNECT,

    /**
     * Indicates completion of a pending Pause command.
     *
     **/
    PVT_COMMAND_PAUSE,
    /**
     * Indicates completion of a pending Resume command.
     *
     **/
    PVT_COMMAND_RESUME,
    /**
     * Indicates completion of a pending SetLogAppenderL command.
     *
     **/
    PVT_COMMAND_SET_LOG_APPENDER,
    /**
     * Indicates completion of a pending RemoveLogAppenderL command.
     *
     **/
    PVT_COMMAND_REMOVE_LOG_APPENDER,
    /**
     * Indicates completion of a pending SetLogLevel command.
     *
     **/
    PVT_COMMAND_SET_LOG_LEVEL,
    /**
     * Indicates completion of a pending GetLogLevel command.
     *
     **/
    PVT_COMMAND_GET_LOG_LEVEL,
    /**
     * Indicates completion of QueryUUIDL command
     *
     **/
    PVT_COMMAND_QUERY_UUID,
    /**
     * Indicates completion of QueryInterfaceL command
     *
     **/
    PVT_COMMAND_QUERY_INTERFACE,
    /**
     * Indicates pending Init or Connect commands were cancelled
     *
     **/
    PVT_COMMAND_CANCEL_ALL_COMMANDS,

    PVT_LAST_COMMAND
};

// This function returns a priority index for each format type.
uint32 GetPriorityIndexForPVMFFormatType(PVMFFormatType aFormatType);

//Priority to MP4 over H.263
template<class Alloc> struct PV2WayRegFormatTypeCompare
{
    bool operator()(const PVMFFormatType& x, const PVMFFormatType& y) const
    {
        uint32 x_val = GetPriorityIndexForPVMFFormatType(x);
        uint32 y_val = GetPriorityIndexForPVMFFormatType(y);

        return (x_val < y_val) ? true : false;
    }
};

class TPV2WayNotificationInfo
{
    public:
        enum TPV2WayNotificationInfoType
        {
            EPV2WayCommandType,
            EPV2WayEventType
        };

        TPV2WayNotificationInfo(TPV2WayNotificationInfoType aType) : notificationType(aType) {};
        virtual ~TPV2WayNotificationInfo() {};

        TPV2WayNotificationInfoType notificationType;
};

class TPV2WayCmdInfo : public TPV2WayNotificationInfo
{
    public:
        TPV2WayCmdInfo() : TPV2WayNotificationInfo(EPV2WayCommandType),
                type(PVT_COMMAND_INIT),
                id(0),
                contextData(NULL),
                status(PVMFSuccess),
                responseData(NULL),
                responseDataSize(0),
                iPvtCmdData(0)
        {};

        ~TPV2WayCmdInfo() {};

        void Clear()
        {
            type = PVT_COMMAND_INIT;
            id = 0;
            contextData = NULL;
            status = PVMFSuccess;
            responseData = NULL;
            responseDataSize = 0;
            iPvtCmdData = 0;
        }

        TPV2WayCommandType type;
        PVCommandId id;
        void *contextData;
        TPVCmnCommandStatus status;
        void* responseData;
        int32 responseDataSize;
        uint32 iPvtCmdData;
};

class TPV2WayEventInfo : public TPV2WayNotificationInfo
{
    public:
        TPV2WayEventInfo() : TPV2WayNotificationInfo(EPV2WayEventType),
                type(PVT_INDICATION_INCOMING_TRACK),
                exclusivePtr(NULL),
                localBufferSize(0)

        {
            oscl_memset(localBuffer, 0, PV_COMMON_ASYNC_EVENT_LOCAL_BUF_SIZE);
        };

        ~TPV2WayEventInfo() {};

        void Clear()
        {
            type = PVT_INDICATION_INCOMING_TRACK;
            exclusivePtr = NULL;
            localBufferSize = 0;
            oscl_memset(localBuffer, 0, PV_COMMON_ASYNC_EVENT_LOCAL_BUF_SIZE);
        }

        TPVTIndicationType type;
        TPVCmnExclusivePtr exclusivePtr;
        uint32 localBufferSize;
        uint8 localBuffer[PV_COMMON_ASYNC_EVENT_LOCAL_BUF_SIZE];
};

/**
 * TPV2WayPortStatus Class
 *
 * An enumeration of port status
 **/
typedef enum
{
    ENoPort = 0,
    ERequestPort,
    EHasPort,
    EReleasePort
} TPV2WayPortStatus;

class CPV2WayPort
{
    public:
        CPV2WayPort() : iPort(NULL),
                iPortStatus(ENoPort),
                iPortCmdId(0)
        {};

        ~CPV2WayPort() {};

        PVMFPortInterface *GetPort()
        {
            return iPort;
        }
        TPV2WayPortStatus GetStatus()
        {
            return iPortStatus;
        }
        PVCommandId GetCmdId()
        {
            return iPortCmdId;
        }

        void SetPort(PVMFPortInterface *aPort)
        {
            if (aPort)
            {
                iPortStatus = EHasPort;
            }
            else
            {
                iPortStatus = ENoPort;
            }
            iPort = aPort;
        }
        void SetStatus(TPV2WayPortStatus aStatus)
        {
            iPortStatus = aStatus;
        }
        void SetCmdId(PVCommandId aId)
        {
            iPortCmdId = aId;
        }

    private:
        PVMFPortInterface *iPort;
        TPV2WayPortStatus iPortStatus;
        PVCommandId iPortCmdId;
};


typedef enum
{
    PV2WAY_NODE_CMD_QUERY_INTERFACE,
    PV2WAY_NODE_CMD_INIT,
    PV2WAY_NODE_CMD_REQUESTPORT,
    PV2WAY_NODE_CMD_PREPARE,
    PV2WAY_NODE_CMD_START,
    PV2WAY_NODE_CMD_PAUSE,
    PV2WAY_NODE_CMD_STOP,
    PV2WAY_NODE_CMD_RELEASEPORT,
    PV2WAY_NODE_CMD_RESET,
    PV2WAY_NODE_CMD_CANCELCMD,
    PV2WAY_NODE_CMD_CANCELALL,
    PV2WAY_NODE_CMD_INVALID,
    PV2WAY_NODE_CMD_QUERY_UUID,
    PV2WAY_NODE_CMD_SKIP_MEDIA_DATA
}  PV2WayNodeCmdType;

class TPV2WayNodeQueryInterfaceParams
{
    public:
        PVUuid *iUuid;
        PVInterface **iInterfacePtr;
};

class TPV2WayNodeQueryUuidParams
{
    public:
        Oscl_Vector<PVUuid, OsclMemAllocator> *iUuids;
        OSCL_HeapString<OsclMemAllocator> mimetype;
};



class PV2WayNodeInterface
{
    public:
        typedef enum
        {
            NoInterface,
            QueryInterface,
            HasInterface
        } InterfaceState;

        PV2WayNodeInterface() : iState(NoInterface),
                iInterface(NULL),
                iId(0)
        {};

        ~PV2WayNodeInterface() {};

        void Reset()
        {
            iState = NoInterface;
            iInterface = NULL;
            iId = 0;
        }

        InterfaceState iState;
        PVInterface *iInterface;
        PVMFCommandId iId;
};

class CPV2WayNodeCommandObserver
{
    public:
        virtual void CommandHandler(PV2WayNodeCmdType aType, const PVMFCmdResp& aResponse) = 0;

};

class CPV2WayNodeContextData
{
    public:
        CPV2WayNodeContextData() : iObserver(NULL),
                iNode(NULL),
                iContextData(NULL)
        {};

        void Clear()
        {
            iObserver = NULL;
            iNode = NULL;
            iContextData = NULL;
        }

        CPV2WayNodeCommandObserver *iObserver;
        PVMFNodeInterface	*iNode;
        void *iContextData;
};

class TPV2WayNodeCmdInfo
{
    public:
        TPV2WayNodeCmdInfo() : type(PV2WAY_NODE_CMD_INIT),
                id(0),
                engineCmdInfo(NULL)
        {};

        ~TPV2WayNodeCmdInfo() {};

        void Clear()
        {
            type = PV2WAY_NODE_CMD_INIT;
            id = 0;
            context.Clear();
            engineCmdInfo = NULL;
        }

        PV2WayNodeCmdType type;
        PVMFCommandId id;
        CPV2WayNodeContextData context;
        TPV2WayCmdInfo *engineCmdInfo;
};

class TPV2WayNodeRequestPortParams
{
    public:
        TPV2WayNodeRequestPortParams() : portTag(-1),
                format(PVMF_MIME_FORMAT_UNKNOWN)
        {};

        int32 portTag;
        PVMFFormatType format;
};

class TPV2WayNode
{
    public:
        TPV2WayNode() : iNode(NULL),
                iSessionId(0)
        {};

        TPV2WayNode(PVMFNodeInterface *aNode) : iNode(aNode), iSessionId(0)
        {};


        TPV2WayNode(const TPV2WayNode &aNode) : iNode(aNode.iNode),
                iSessionId(aNode.iSessionId)
        {};

        TPV2WayNode& operator=(const TPV2WayNode& a)
        {
            iNode = a.iNode;
            iSessionId = a.iSessionId;
            return *this;
        }

        void Clear()
        {
            iNode = NULL;
            iSessionId = 0;
        }

        operator PVMFNodeInterface *()
        {
            return iNode;
        }

        PVMFSessionId GetSessionId()
        {
            return iSessionId;
        }

        PVMFNodeInterface *iNode;
        PVMFSessionId iSessionId;
};

class SinkNodeIFList
{
    public:

        SinkNodeIFList() : iSinkNode(NULL)
                , iNodeInterface()

        {};

        ~SinkNodeIFList()
        {};

        SinkNodeIFList(const SinkNodeIFList& aList)
        {
            iSinkNode = aList.iSinkNode;
            iNodeInterface = aList.iNodeInterface;
        };

        void Clear()
        {
            iSinkNode = NULL;
            iNodeInterface.Reset();
        }

        TPV2WayNode* iSinkNode;
        PV2WayNodeInterface iNodeInterface;
};

class PlayFromFileNode;
class CPVDatapathNode;
class CPV2WayDecDataChannelDatapath;
class CPV2WayEncDataChannelDatapath;
class CPV2WayMuxDatapath;
class CPV2WayPreviewDatapath;
class CPV2WayRecDatapath;
class CPV2WayInitDatapath;
class PVCommandStatusObserver;
class PVInformationalEventObserver;
class PVErrorEventObserver;

class CPV2WayNodeConfigurationObserver
{
    public:
        virtual PVMFStatus ConfigureNode(CPVDatapathNode *aNode) = 0;
};

class CPV324m2Way : OsclActiveObject,
            public CPV2WayInterface,
            public PVMFNodeCmdStatusObserver,
            public PVMFNodeInfoEventObserver,
            public PVMFNodeErrorEventObserver,
            public CPV2WayNodeCommandObserver,
            public CPV2WayNodeConfigurationObserver,
#ifndef NO_2WAY_324
            public TSCObserver,
#endif
            public OsclTimerObserver
{
    public:
        OSCL_IMPORT_REF static CPV324m2Way *NewL(PVMFNodeInterface* aTsc,
                TPVTerminalType aTerminalType,
                PVCommandStatusObserver* aCmdStatusObserver,
                PVInformationalEventObserver *aInfoEventObserver,
                PVErrorEventObserver *aErrorEventObserver);

        OSCL_EXPORT_REF static void Delete(CPV324m2Way *aTerminal);

        PVCommandId GetSDKInfo(PVSDKInfo &aSDKInfo, OsclAny* aContextData = NULL);
        PVCommandId GetSDKModuleInfo(PVSDKModuleInfo &aSDKModuleInfo, OsclAny* aContextData = NULL);
        PVCommandId Init(PV2WayInitInfo& aInitInfo, OsclAny* aContextData = NULL);
        PVCommandId Reset(OsclAny* aContextData = NULL);
        PVCommandId AddDataSource(PVTrackId aChannelId, PVMFNodeInterface& aDataSource, OsclAny* aContextData = NULL);
        PVCommandId RemoveDataSource(PVMFNodeInterface& aDataSource, OsclAny* aContextData = NULL);
        PVCommandId AddDataSink(PVTrackId aChannelId, PVMFNodeInterface& aDataSink, OsclAny* aContextData = NULL);
        PVCommandId RemoveDataSink(PVMFNodeInterface& aDataSink, OsclAny* aContextData = NULL);
        PVCommandId Connect(const PV2WayConnectOptions& aOptions, PVMFNodeInterface* aCommServer = NULL, OsclAny* aContextData = NULL);
        PVCommandId Disconnect(OsclAny* aContextData = NULL);
        PVCommandId GetState(PV2WayState& aState, OsclAny* aContextData = NULL);
        PVCommandId SetLatencyQualityTradeoff(PVMFNodeInterface& aTrack, int32 aTradeoff, OsclAny* aContextData = NULL);
        PVCommandId Pause(PV2WayDirection aDirection, PVTrackId aTrackId, OsclAny* aContextData = NULL);
        PVCommandId Resume(PV2WayDirection aDirection, PVTrackId aTrackId, OsclAny* aContextData = NULL);
        PVCommandId SetLogAppender(const char* aTag, OsclSharedPtr<PVLoggerAppender>& aAppender, OsclAny* aContextData = NULL);
        PVCommandId RemoveLogAppender(const char* aTag, OsclSharedPtr<PVLoggerAppender>& aAppender, OsclAny* aContextData = NULL);
        PVCommandId SetLogLevel(const char* aTag, int32 aLevel, bool aSetSubtree = false, OsclAny* aContextData = NULL);
        PVCommandId GetLogLevel(const char* aTag, int32& aLogInfo, OsclAny* aContextData = NULL);
        //PVCommandId SendUserInput(CPVUserInput& user_input, OsclAny* aContextData = NULL);
        //PVCommandId GetCallStatistics(CPVCmn2WayStatistics& aStats, OsclAny* aContextData = NULL);
        PVCommandId QueryInterface(const PVUuid& aUuid, PVInterface*& aInterfacePtr, OsclAny* aContext = NULL);
        PVCommandId QueryUUID(const PvmfMimeString& aMimeType, Oscl_Vector<PVUuid, BasicAlloc>& aUuids,
                              bool aExactUuidsOnly = false, OsclAny* aContextData = NULL);
        PVCommandId CancelAllCommands(OsclAny* aContextData = NULL);

        // from PVMFNodeCmdEventObserver
        void NodeCommandCompleted(const PVMFCmdResp& aResponse);

        // from PVMFNodeInfoEventObserver
        void HandleNodeInformationalEvent(const PVMFAsyncEvent& aEvent);

        // from PVMFNodeErrorEventObserver
        void HandleNodeErrorEvent(const PVMFAsyncEvent& aEvent);

        // from CPV2WayNodeCommandObserver
        void CommandHandler(PV2WayNodeCmdType aType, const PVMFCmdResp& aResponse);

        // from CPV2WayNodeConfigurationObserver
        PVMFStatus ConfigureNode(CPVDatapathNode *aNode);

        //TSC Observer Virtuals
        /* Responses to commands */
        void OutgoingChannelEstablished(TPVChannelId id, PVCodecType_t codec, uint8* fsi, uint32 fsi_len);
        /* Unsolicited indications */
        void ConnectComplete(PVMFStatus status);
        void InternalError();
        /* Requests engine to stop av codecs.  TSC will clean up logical channels by itself */
        void DisconnectRequestReceived();
        TPVStatusCode IncomingChannel(TPVChannelId aId,
                                      PVCodecType_t aCodec,
                                      uint8* aFormatSpecificInfo = NULL, uint32 aFormatSpecificInfoLen = 0);
        void ChannelClosed(TPVDirection direction, TPVChannelId id, PVCodecType_t codec, PVMFStatus status = PVMFSuccess);
        void RequestFrameUpdate(PVMFPortInterface *port);

        // OsclTimerObserver virtuals
        void TimeoutOccurred(int32 timerID, int32 timeoutInfo);

#ifndef NO_2WAY_324
        bool AllChannelsOpened();
#endif
        bool Supports(PVMFNodeCapability &capability, PVMFFormatType aFormat, bool isInput = true);
#ifdef MEM_TRACK
        void MemStats();
#endif
    private:
        CPV324m2Way();
        ~CPV324m2Way();
        void ClearVideoEncNode();
        void ConstructL(PVMFNodeInterface* aTsc,
                        TPVTerminalType aType,
                        PVCommandStatusObserver* aCmdStatusObserver,
                        PVInformationalEventObserver *aInfoEventObserver,
                        PVErrorEventObserver *aErrorEventObserver);
        void SetDefaults();

        void PreInit();
        // OsclActiveObject virtuals
        void DoCancel();
        void Run();
        void Dispatch(TPV2WayCmdInfo* aCmdInfo);
        void Dispatch(TPV2WayEventInfo* aEventInfo);
        void SetState(PV2WayState aState)
        {
            iLastState = iState;
            iState = aState;
        }

        void InitiateDisconnect();
        void InitiateReset();
        void CheckState();
        void CheckInit();
        void CheckConnect();
        void CheckDisconnect();
        void CheckReset();

        void RemoveAudioDecPath();
        void RemoveAudioEncPath();
        void RemoveVideoDecPath();
        void RemoveVideoEncPath();

        void HandleCommNodeCmd(PV2WayNodeCmdType aType, const PVMFCmdResp& aResponse);
        void HandleTscNodeCmd(PV2WayNodeCmdType aType, const PVMFCmdResp& aResponse);
        void HandleVideoDecNodeCmd(PV2WayNodeCmdType aType, const PVMFCmdResp& aResponse);
        void HandleVideoEncNodeCmd(PV2WayNodeCmdType aType, const PVMFCmdResp& aResponse);
        void HandleAudioEncNodeCmd(PV2WayNodeCmdType aType, const PVMFCmdResp& aResponse);
        void HandleSinkNodeCmd(PV2WayNodeCmdType aType, const PVMFCmdResp& aResponse, TPV2WayNode* aNode);
        void GenerateIFrame(PVMFPortInterface *aPort);
        void RequestRemoteIFrame(PVMFPortInterface *aPort);

        TPV2WayCmdInfo *GetCmdInfoL();
        void FreeCmdInfo(TPV2WayCmdInfo *info);

        TPV2WayEventInfo *GetEventInfoL();
        void FreeEventInfo(TPV2WayEventInfo *info);

        PVMFCommandId SendNodeCmdL(PV2WayNodeCmdType aCmd, TPV2WayNode *aNode,
                                   CPV2WayNodeCommandObserver *aObserver,
                                   void *aParam = NULL, TPV2WayCmdInfo *a2WayCmdInfo = NULL);

        TPV2WayNodeCmdInfo *FindPendingNodeCmd(PVMFNodeInterface *aNode, PVMFCommandId aId);
        void RemovePendingNodeCmd(PVMFNodeInterface *aNode, PVMFCommandId aId, bool aAllCmds = false);

        void FillSDKInfo(PVSDKInfo &aSDKInfo);



        int32 GetStackNodePortTag(TPV2WayPortTagType portType);

        bool CheckMandatoryCodecs(const PVMFFormatType *aMandatoryList, uint32 aMandatorySize, Oscl_Vector<PVMFFormatType, OsclMemAllocator> &aCodecList);

        void InitiateSession(TPV2WayNode& aNode);

        void SetPreferredCodecs(PV2WayInitInfo& aInitInfo);

        void SetPreferredCodecs(TPVDirection aDir,
                                Oscl_Vector<const char*, OsclMemAllocator>& aAudioFormats,
                                Oscl_Vector<const char*, OsclMemAllocator>& aVideoFormats);

        const char* FindFormatType(PVMFFormatType aFormatType,
                                   Oscl_Vector<const char*, OsclMemAllocator>& aAudioFormats,
                                   Oscl_Vector<const char*, OsclMemAllocator>& aVideoFormats);

        bool IsSupported(const PVMFFormatType& aInputFmtType, const PVMFFormatType& aOutputFmtType);

        const char* CanConvertFormat(TPVDirection aDir, const PVMFFormatType& aThisFmtType, Oscl_Vector<const char*, OsclMemAllocator>& aThatFormatList);

        void DoSelectFormat(TPVDirection aDir, PVMFFormatType aFormatType, const char* aFormatStr, TPVPriority aPriority, PVMFFormatType aFormatApp = PVMF_MIME_FORMAT_UNKNOWN);

        void DoAddDataSource(TPV2WayNode& aNode, const PVMFCmdResp& aResponse);

        void DoAddDataSink(TPV2WayNode& aNode, const PVMFCmdResp& aResponse);

        PVCommandId DoRemoveDataSourceSink(PVMFNodeInterface& aEndPt, OsclAny* aContextData);

        bool IsNodeInList(Oscl_Vector<TPV2WayNode*, OsclMemAllocator>& aList, PVMFNodeInterface* aNode);

        bool IsSourceNode(PVMFNodeInterface* aNode);

        bool IsSinkNode(PVMFNodeInterface* aNode);


        TPV2WayNode* GetTPV2WayNode(Oscl_Vector<TPV2WayNode*, OsclMemAllocator>& aList, PVMFNodeInterface* aNode);

        TPV2WayNode* RemoveTPV2WayNode(Oscl_Vector<TPV2WayNode*, OsclMemAllocator>& aList, PVMFNodeInterface* aNode);

        bool IsNodeReset(PVMFNodeInterface& aNode);

        void SupportedSinkNodeInterfaces(TPV2WayNode* aNode);
        typedef PV2WayRegFormatTypeCompare<OsclMemAllocator> pvmf_format_type_key_compare_class;


        void ConvertMapToVector(Oscl_Map<PVMFFormatType, FormatCapabilityInfo, OsclMemAllocator, pvmf_format_type_key_compare_class>& aCodecs, Oscl_Vector<FormatCapabilityInfo, OsclMemAllocator>& aFormatCapability);

        void AddVideoEncoderNode();
        void AddAudioEncoderNode();
        void AddVideoDecoderNode(uint8* aFormatSpecificInfo, uint32 aFormatSpecificInfoLen);
        void AddAudioDecoderNode();
        void RegisterMioLatency(const char* aMimeStr, bool aAudio, PVMFFormatType aFmtType);
        uint32 LookupMioLatency(PVMFFormatType aFmtType, bool aAudio);


        bool GetEventInfo(TPV2WayEventInfo*& event);
        static int32 Construct(CPV324m2Way* aRet,
                               PVMFNodeInterface* aTsc,
                               TPVTerminalType aTerminalType,
                               PVCommandStatusObserver* aCmdStatusObserver,
                               PVInformationalEventObserver *aInfoEventObserver,
                               PVErrorEventObserver *aErrorEventObserver);

        void GetStackSupportedFormats();
        PVMFStatus EstablishChannel(TPVDirection aDir,
                                    TPVChannelId aId,
                                    PVCodecType_t aCodec,
                                    uint8* fsi, uint32 fsi_len);
#ifdef PV_USE_DSP_AMR_CODECS
        void InitializeDsp();
        void ReleaseDsp();
        CPVDsp* iDsp; // owned.
#endif // PV_USE_DSP_AMR_CODECS

        // Loopback mode
        TPVLoopbackMode iLoopbackMode;

        // Current state
        PV2WayState iState;
        PV2WayState iLastState;

        // Command status observer
        PVCommandStatusObserver* iCmdStatusObserver;

        // Observer for informational messages
        PVInformationalEventObserver *iInfoEventObserver;

        // Observer for errors
        PVErrorEventObserver *iErrorEventObserver;

        /* For milestone 2, we will not use the proxy framework.  This will be replaced for Milestone 3 */
        Oscl_Vector<TPV2WayNotificationInfo*, BasicAlloc> iPendingNotifications;

        PVCommandId iCommandId;

        PVUuid iCapConfigPVUuid;

        Oscl_Vector<PVUuid, OsclMemAllocator> iSipMgrNodeUuidVec;

        // Video data path
        CPV2WayEncDataChannelDatapath *iVideoEncDatapath;
        CPV2WayDecDataChannelDatapath *iVideoDecDatapath;

        // Audio data path
        CPV2WayEncDataChannelDatapath *iAudioEncDatapath;
        CPV2WayDecDataChannelDatapath *iAudioDecDatapath;

        // COMM Datapath
        bool iIsStackConnected;
        CPV2WayMuxDatapath *iMuxDatapath;
        TPV2WayNode iTscNode;
        TSC* iTscNodePtr;
        TPV2WayNode iCommNode;

        TPV2WayNode iAudioSinkNode;
        TPV2WayNode iAudioSrcNode;

#if defined(PV_PLAY_FROM_FILE_SUPPORT)
        TPV2WayNode iAudioSrcSplitterNode;
        TPV2WayNode iVideoSrcSplitterNode;
#endif

        TPV2WayNode iVideoEncNode;
        PV2WayNodeInterface iVideoEncNodeInterface;

        TPV2WayNode iVideoDecNode;
        TPV2WayNode iVideoParserNode;

#if defined(PV_RECORD_TO_FILE_SUPPORT)
        TPV2WayNode iVideoDecSplitterNode;
#endif

        TPV2WayNode iAudioEncNode;
        TPV2WayNode iAudioDecNode;
        PV2WayNodeInterface iAudioEncNodeInterface;

        PV2WayNodeInterface iClockSyncInterface;

        Oscl_Vector<SinkNodeIFList, OsclMemAllocator> iSinkNodeList;

        Oscl_Vector<TPV2WayNode*, OsclMemAllocator> iSourceNodes;
        Oscl_Vector<TPV2WayNode*, OsclMemAllocator> iSinkNodes;

        PVUuid iSyncControlPVUuid;

        Oscl_Vector<TPV2WayCmdInfo *, BasicAlloc> iFreeCmdInfo;
        TPV2WayCmdInfo iCmdInfo[MAX_PENDING_2WAY_COMMANDS];

        Oscl_Vector<TPV2WayEventInfo *, BasicAlloc> iFreeEventInfo;
        TPV2WayEventInfo iEventInfo[MAX_PENDING_2WAY_EVENTS];

        Oscl_Vector<TPV2WayNodeCmdInfo *, BasicAlloc> iFreeNodeCmdInfo;
        Oscl_Vector<TPV2WayNodeCmdInfo *, BasicAlloc> iPendingNodeCmdInfo;
        TPV2WayNodeCmdInfo iNodeCmdInfo[MAX_PENDING_2WAY_NODE_COMMANDS];

        // pending Init info
        TPV2WayCmdInfo *iInitInfo;

        // pending Connect info
        TPV2WayCmdInfo *iConnectInfo;

        // Is a disconnect pending ?
        TPV2WayCmdInfo *iDisconnectInfo;

        // Is a reset pending ?
        TPV2WayCmdInfo *iResetInfo;

        // Is a cancel pending ?
        TPV2WayCmdInfo *iCancelInfo;

        // Is a get session params call pending ?
        TPV2WayCmdInfo *iSessionParamsInfo;

        PVLogger *iLogger;


        //Minimum time between making successive I frame requests (sec).
        uint32 iMinIFrameRequestInterval;
        OsclTimer<OsclMemAllocator> iIFrameReqTimer;
        OsclTimer<OsclMemAllocator>* iEndSessionTimer;
        OsclTimer<OsclMemAllocator>* iRemoteDisconnectTimer;
        bool isIFrameReqTimerActive;

#ifndef NO_2WAY_324
        Oscl_Vector<H324ChannelParameters, PVMFTscAlloc> iIncomingChannelParams;
        Oscl_Map<PVMFFormatType, FormatCapabilityInfo, OsclMemAllocator, pvmf_format_type_key_compare_class> iIncomingAudioCodecs;
        Oscl_Map<PVMFFormatType, FormatCapabilityInfo, OsclMemAllocator, pvmf_format_type_key_compare_class> iIncomingVideoCodecs;
        Oscl_Map<PVMFFormatType, FormatCapabilityInfo, OsclMemAllocator, pvmf_format_type_key_compare_class> iOutgoingAudioCodecs;
        Oscl_Map<PVMFFormatType, FormatCapabilityInfo, OsclMemAllocator, pvmf_format_type_key_compare_class> iOutgoingVideoCodecs;

        Oscl_Map<char*, uint32, OsclMemAllocator> iAudioLatency;
        Oscl_Map<char*, uint32, OsclMemAllocator> iVideoLatency;

        Oscl_Vector<FormatCapabilityInfo, OsclMemAllocator> iFormatCapability;

        /* A list of formats supported by the stack in a certain order of priority.  This list should ideally come from the stack but
        for now we can generate it internally in the engine and later we will have an API for the stack to return it.
        For the current version, the list would look like this:
        {AMR, Mpeg-4, H.263}
        */
        Oscl_Map<PVMFFormatType, CPvtMediaCapability*, OsclMemAllocator, pvmf_format_type_key_compare_class> iStackSupportedFormats;
        /* App Format is the format of the Node/MIO that the application needs to pass in for an incoming/outgoing datapath.
           These maps track which application formats are expected by the engine for formats(codecs) supported by the engine
           for the current call.
           */
        Oscl_Map<PVMFFormatType, PVMFFormatType, OsclMemAllocator, pvmf_format_type_key_compare_class> iAppFormatForEngineFormatIncoming;
        Oscl_Map<PVMFFormatType, PVMFFormatType, OsclMemAllocator, pvmf_format_type_key_compare_class> iAppFormatForEngineFormatOutgoing;

        Oscl_Vector<FormatCapabilityInfo, OsclMemAllocator> iIncomingUserInputFormats;
        Oscl_Vector<H324ChannelParameters, PVMFTscAlloc> iOutgoingChannelParams;
        int32 iIncomingAudioTrackTag;
        int32 iIncomingVideoTrackTag;
        int32 iOutgoingAudioTrackTag;
        int32 iOutgoingVideoTrackTag;
#endif
        PVUuid iVideoEncPVUuid;
        PVUuid iAudioEncPVUuid;
        PVMFCommandId iVideoEncQueryIntCmdId;

        typedef enum
        {
            File2WayIdle,
            File2WayInitializing,
            File2WayInitialized,
            File2WayResetting
        } File2WayState;


#if defined(PV_RECORD_TO_FILE_SUPPORT)
        void HandleFFComposerNodeCmd(PV2WayNodeCmdType aType, const PVMFCmdResp& aResponse);
        void RemoveAudioRecPath();
        void RemoveVideoRecPath();
        void CheckRecordFileState();
        void CheckRecordFileInit();
        void CheckRecordFileReset();
        void InitiateResetRecordFile();

        OSCL_wHeapString<OsclMemAllocator> iRecFilename;
        File2WayState iRecordFileState;

        // Is a init record file pending ?
        TPV2WayCmdInfo *iInitRecFileInfo;

        // Is a reset record file pending ?
        TPV2WayCmdInfo *iResetRecFileInfo;

        TPV2WayNode iFFComposerNode;
        PV2WayNodeInterface iFFClipConfig;
        PV2WayNodeInterface iFFTrackConfig;
        PV2WayNodeInterface iFFSizeAndDuration;

        PVUuid iFFClipConfigPVUuid;
        PVUuid iFFTrackConfigPVUuid;
        PVUuid iFFSizeAndDurationPVUuid;
        CPV2WayRecDatapath *iAudioRecDatapath;
        CPV2WayRecDatapath *iVideoRecDatapath;

        uint32 iRecFileSizeNotificationInterval; //Interval in msec
        OsclTimer<OsclMemAllocator> iRecFileSizeNotificationTimer;
        bool isRecFileSizeNotificationTimerActive;
#endif

#if defined(PV_PLAY_FROM_FILE_SUPPORT)
        void RemoveAudioPreviewPath();
        void RemoveVideoPreviewPath();

        CPV2WayPreviewDatapath *iAudioPreviewDatapath;
        CPV2WayPreviewDatapath *iVideoPreviewDatapath;

        void HandlePFFNodeCmd(PV2WayNodeCmdType aType, const PVMFCmdResp& aResponse);
        void CheckPlayFileState();
        void CheckPlayFileInit();
        void CheckPlayFileReset();
        void InitiateResetPlayFile();
        void CheckAudioSourceMixingPort();
        void HandleAudioSrcNodeCmd(PV2WayNodeCmdType aType, const PVMFCmdResp& aResponse);

        OSCL_wHeapString<OsclMemAllocator> iPlayFilename;
        File2WayState iPlayFileState;
        bool iUsePlayFileAsSource;

        // Is a init play file pending ?
        TPV2WayCmdInfo *iInitPlayFileInfo;

        // Is a reset play file pending ?
        TPV2WayCmdInfo *iResetPlayFileInfo;

        // Play file cmd ?
        TPV2WayCmdInfo *iPlayFileCmdInfo;

        PlayFromFileNode *iPlayFromFileNode;
        CPV2WayPort iAudioPlayPort;
        CPV2WayPort iVideoPlayPort;
#endif

        /* Common clock to be shared with nodes that support the PvmfNodesSyncControlInterface interface */
        PVMFMediaClock iClock;
        PVMFTimebase_Tickcount iTickCountTimeBase;

        friend class CPV2WayCmdControlDatapath;
        friend class CPV2WayDatapath;
        friend class CPV2WayDataChannelDatapath;
        friend class CPV2WayDecDataChannelDatapath;
        friend class CPV2WayEncDataChannelDatapath;
        friend class CPV2WayMuxDatapath;

#if defined(PV_PLAY_FROM_FILE_SUPPORT)
        friend class CPV2WayPreviewDatapath;
#endif

#if defined(PV_RECORD_TO_FILE_SUPPORT)
        friend class CPV2WayRecDatapath;
#endif

        TSC *iTSCInterface;
        TSC_324m *iTSC324mInterface;

        TPVTerminalType iTerminalType;

        int32 iPendingTscReset;
        int32 iPendingAudioEncReset;
        int32 iPendingVideoEncReset;
        int32 iAudioDatapathLatency;
        int32 iVideoDatapathLatency;

        /* The AddDataSource command for video will be pending untill the extension interface for the encoder is queried and the
            encoder is configured */
        TPV2WayCmdInfo *iAddDataSourceVideoCmd;
};

#endif

