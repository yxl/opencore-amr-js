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
#ifndef PVMF_STREAMING_MANAGER_INTERNAL_H_INCLUDED
#define PVMF_STREAMING_MANAGER_INTERNAL_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_STRING_CONTAINERS_H_INCLUDED
#include "oscl_string_containers.h"
#endif
#ifndef PVMF_NODE_UTILS_H_INCLUDED
#include "pvmf_node_utils.h"
#endif
#ifndef OSCL_MEM_MEMPOOL_H_INCLUDED
#include "oscl_mem_mempool.h"
#endif
#ifndef PVMF_MEDIA_DATA_H_INCLUDED
#include "pvmf_media_data.h"
#endif
#ifndef PVMF_DATA_SOURCE_INIT_EXTENSION_H_INCLUDED
#include "pvmf_data_source_init_extension.h"
#endif
#ifndef PVMF_TRACK_SELECTION_EXTENSION_H_INCLUDED
#include "pvmf_track_selection_extension.h"
#endif
#ifndef PVMF_DATA_SOURCE_PLAYBACK_CONTROL_H_INCLUDED
#include "pvmf_data_source_playback_control.h"
#endif
#ifndef PVMF_STREAMING_MANAGER_EXT_INTERFACE_H_INCLUDED
#include "pvmf_streaming_manager_ext_interface.h"
#endif
#ifndef PVMF_META_DATA_H_INCLUDED
#include "pvmf_meta_data_types.h"
#endif
#ifndef PVMF_META_DATA_EXTENSION_H_INCLUDED
#include "pvmf_meta_data_extension.h"
#endif
#ifndef SDP_INFO_H_INCLUDED
#include "sdp_info.h"
#endif
#ifndef PVMF_SM_TUNABLES_H_INCLUDED
#include "pvmf_sm_tunables.h"
#endif
#ifndef PVMF_STREAMING_ASF_INTERFACES_INCLUDED
#include "pvmf_streaming_asf_interfaces.h"
#endif
#ifndef PVMF_CPMPLUGIN_LICENSE_INTERFACE_H_INCLUDED
#include "pvmf_cpmplugin_license_interface.h"
#endif

/**
 * Macros for calling PVLogger
 */
#define PVMF_SM_LOGERROR(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iLogger,PVLOGMSG_ERR,m);
#define PVMF_SM_LOGINFOHI(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG,iLogger,PVLOGMSG_NOTICE,m);
#define PVMF_SM_LOGINFOMED(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG,iLogger,PVLOGMSG_INFO,m);
#define PVMF_SM_LOGSTACKTRACE(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG,iLogger,PVLOGMSG_STACK_TRACE,m);
#define PVMF_SM_LOGINFOLOW(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG,iLogger,PVLOGMSG_DEBUG,m);
#define PVMF_SM_LOGINFO(m) PVMF_SM_LOGINFOMED(m)
#define PVMF_SM_LOG_COMMAND_SEQ(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG,iCmdSeqLogger,PVLOGMSG_DEBUG,m);
#define PVMF_SM_LOG_COMMAND_REPOS(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG,iReposLogger,PVLOGMSG_DEBUG,m);

#define PVMF_STREAMING_MANAGER_NEW(auditCB,T,params,ptr)\
{\
	ptr = OSCL_NEW(T,params);\
}

#define PVMF_STREAMING_MANAGER_DELETE(auditCB,T,ptr)\
{\
	OSCL_DELETE(ptr);\
}

#define PVMF_STREAMING_MANAGER_TEMPLATED_DELETE(auditCB, T, Tsimple, ptr)\
{\
	OSCL_DELETE(ptr);\
}

#define PV_STREAMING_MANAGER_ARRAY_NEW(auditCB, T, count, ptr)\
{\
	ptr = OSCL_ARRAY_NEW(T, count);\
}

#define PV_STREAMING_MANAGER_ARRAY_DELETE(auditCB, ptr)\
{\
	OSCL_ARRAY_DELETE(ptr);\
}

template <class T> class PVMFSMSharedPtrAlloc: public OsclDestructDealloc
{
    public:
        T* allocate()
        {
            PVMFStreamingManagerNodeAllocator alloc;
            T* ptr = OSCL_PLACEMENT_NEW(alloc.allocate(sizeof(T)), T());
            return ptr;
        }

        virtual void destruct_and_dealloc(OsclAny* ptr)
        {
            T* tptr ;
            tptr = reinterpret_cast<T*>(ptr);
            tptr->~T();
            PVMFStreamingManagerNodeAllocator alloc;
            alloc.deallocate(ptr);
        }
};

/* Default vector reserve size */
#define PVMF_STREAMING_MANAGER_VECTOR_RESERVE 10
/* Starting value for command IDs  */
#define PVMF_STREAMING_MANAGER_NODE_COMMAND_ID_START 9000

///////////////////////////////////////////////
//IDs for all of the asynchronous node commands.
///////////////////////////////////////////////
enum TPVMFStreamingManagerNodeCommand
{
    PVMF_STREAMING_MANAGER_NODE_QUERYUUID,
    PVMF_STREAMING_MANAGER_NODE_QUERYINTERFACE,
    PVMF_STREAMING_MANAGER_NODE_REQUESTPORT,
    PVMF_STREAMING_MANAGER_NODE_RELEASEPORT,
    PVMF_STREAMING_MANAGER_NODE_INIT,
    PVMF_STREAMING_MANAGER_NODE_PREPARE,
    PVMF_STREAMING_MANAGER_NODE_START,
    PVMF_STREAMING_MANAGER_NODE_STOP,
    PVMF_STREAMING_MANAGER_NODE_FLUSH,
    PVMF_STREAMING_MANAGER_NODE_PAUSE,
    PVMF_STREAMING_MANAGER_NODE_RESET,
    PVMF_STREAMING_MANAGER_NODE_CANCELALLCOMMANDS,
    PVMF_STREAMING_MANAGER_NODE_CANCELCOMMAND,
    /* add streaming manager node specific commands here */
    PVMF_STREAMING_MANAGER_NODE_CONSTRUCT_SESSION,
    PVMF_STREAMING_MANAGER_NODE_SET_DATASOURCE_POSITION,
    PVMF_STREAMING_MANAGER_NODE_QUERY_DATASOURCE_POSITION,
    PVMF_STREAMING_MANAGER_NODE_SET_DATASOURCE_RATE,
    PVMF_STREAMING_MANAGER_NODE_GETNODEMETADATAKEYS,
    PVMF_STREAMING_MANAGER_NODE_GETNODEMETADATAVALUES,
    PVMF_STREAMING_MANAGER_NODE_AUTO_PAUSE,
    PVMF_STREAMING_MANAGER_NODE_AUTO_RESUME,
    PVMF_STREAMING_MANAGER_NODE_THIN_STREAM,
    PVMF_STREAMING_MANAGER_NODE_GET_LICENSE_W,
    PVMF_STREAMING_MANAGER_NODE_GET_LICENSE,
    PVMF_STREAMING_MANAGER_NODE_CANCEL_GET_LICENSE,
    PVMF_STREAMING_MANAGER_NODE_CAPCONFIG_SETPARAMS,
    PVMF_STREAMING_MANAGER_NODE_COMMAND_LAST
};

enum TPVMFStreamingManagerChildNodeTag
{
    PVMF_STREAMING_MANAGER_NODE_UNKNOWN = 0,
    PVMF_STREAMING_MANAGER_SOCKET_NODE = 1,
    PVMF_STREAMING_MANAGER_RTSP_SESSION_CONTROLLER_NODE = 2,
    PVMF_STREAMING_MANAGER_JITTER_BUFFER_NODE = 3,
    PVMF_STREAMING_MANAGER_MEDIA_LAYER_NODE = 4,
    PVMF_STREAMING_MANAGER_HTTP_SESSION_CONTROLLER_NODE = 5,
    PVMF_STREAMING_MANAGER_RTPPACKETSOURCE_NODE = 6,
    PVMF_STREAMING_MANAGER_PVR_NODE = 7,
    PVMF_STREAMING_MANAGER_LAST_CHILD_NODE
};


#define PVMF_STREAMING_MANAGER_SOCKET_NODE_COMMAND_START              1000
#define PVMF_STREAMING_MANAGER_RTSP_SESSION_CONTROLLER_COMMAND_START  2000
#define PVMF_STREAMING_MANAGER_JITTER_BUFFER_CONTROLLER_COMMAND_START 3000
#define PVMF_STREAMING_MANAGER_MEDIA_LAYER_COMMAND_START              4000
#define PVMF_STREAMING_MANAGER_HTTP_SESSION_CONTROLLER_COMMAND_START  5000
#define PVMF_STREAMING_MANAGER_RTPPACKETSOURCE_NODE_COMMAND_START     6000
#define PVMF_STREAMING_MANAGER_PVR_NODE_COMMAND_START     7000

#define PVMF_STREAMING_MANAGER_NODE_INTERNAL_QUERY_UUID_CMD_OFFSET       0
#define PVMF_STREAMING_MANAGER_NODE_INTERNAL_QUERY_INTERFACE_CMD_OFFSET  1
#define PVMF_STREAMING_MANAGER_NODE_INTERNAL_INIT_CMD_OFFSET             2
#define PVMF_STREAMING_MANAGER_NODE_INTERNAL_PREPARE_CMD_OFFSET          3
#define PVMF_STREAMING_MANAGER_NODE_INTERNAL_START_CMD_OFFSET            4
#define PVMF_STREAMING_MANAGER_NODE_INTERNAL_STOP_CMD_OFFSET             5
#define PVMF_STREAMING_MANAGER_NODE_INTERNAL_FLUSH_CMD_OFFSET            6
#define PVMF_STREAMING_MANAGER_NODE_INTERNAL_PAUSE_CMD_OFFSET            7
#define PVMF_STREAMING_MANAGER_NODE_INTERNAL_RESET_CMD_OFFSET            8
#define PVMF_STREAMING_MANAGER_NODE_INTERNAL_REQUEST_PORT_OFFSET         9
#define PVMF_STREAMING_MANAGER_NODE_INTERNAL_RELEASE_PORT_OFFSET         10
#define PVMF_STREAMING_MANAGER_NODE_INTERNAL_CANCEL_ALL_OFFSET           11
#define PVMF_STREAMING_MANAGER_NODE_INTERNAL_SEEK_OFFSET                 12
#define PVMF_STREAMING_MANAGER_NODE_INTERNAL_STREAM_SWITCH_CMD_OFFSET    13
#define PVMF_STREAMING_MANAGER_NODE_INTERNAL_PLAYLIST_PLAY_CMD_OFFSET    14

enum TPVMFStreamingManagerInternalCommands
{
    PVMF_STREAMING_MANAGER_INTERNAL_COMMAND_NONE = 0,

    PVMF_STREAMING_MANAGER_SOCKET_NODE_QUERY_UUID = 1000,
    PVMF_STREAMING_MANAGER_SOCKET_NODE_QUERY_INTERFACE = 1001,
    PVMF_STREAMING_MANAGER_SOCKET_NODE_INIT = 1002,
    PVMF_STREAMING_MANAGER_SOCKET_NODE_PREPARE = 1003,
    PVMF_STREAMING_MANAGER_SOCKET_NODE_START = 1004,
    PVMF_STREAMING_MANAGER_SOCKET_NODE_STOP = 1005,
    PVMF_STREAMING_MANAGER_SOCKET_NODE_FLUSH = 1006,
    PVMF_STREAMING_MANAGER_SOCKET_NODE_PAUSE = 1007,
    PVMF_STREAMING_MANAGER_SOCKET_NODE_RESET = 1008,
    PVMF_STREAMING_MANAGER_SOCKET_NODE_REQUEST_PORT = 1009,
    PVMF_STREAMING_MANAGER_SOCKET_NODE_RELEASE_PORT = 1010,
    PVMF_STREAMING_MANAGER_SOCKET_NODE_CANCEL_ALL_COMMANDS = 1011,

    PVMF_STREAMING_MANAGER_RTSP_SESSION_CONTROLLER_QUERY_UUID = 2000,
    PVMF_STREAMING_MANAGER_RTSP_SESSION_CONTROLLER_QUERY_INTERFACE = 2001,
    PVMF_STREAMING_MANAGER_RTSP_SESSION_CONTROLLER_INIT = 2002,
    PVMF_STREAMING_MANAGER_RTSP_SESSION_CONTROLLER_PREPARE = 2003,
    PVMF_STREAMING_MANAGER_RTSP_SESSION_CONTROLLER_START = 2004,
    PVMF_STREAMING_MANAGER_RTSP_SESSION_CONTROLLER_STOP = 2005,
    PVMF_STREAMING_MANAGER_RTSP_SESSION_CONTROLLER_FLUSH = 2006,
    PVMF_STREAMING_MANAGER_RTSP_SESSION_CONTROLLER_PAUSE = 2007,
    PVMF_STREAMING_MANAGER_RTSP_SESSION_CONTROLLER_RESET = 2008,
    PVMF_STREAMING_MANAGER_RTSP_SESSION_CONTROLLER_REQUEST_PORT = 2009,
    PVMF_STREAMING_MANAGER_RTSP_SESSION_CONTROLLER_RELEASE_PORT = 2010,
    PVMF_STREAMING_MANAGER_RTSP_SESSION_CONTROLLER_CANCEL_ALL_COMMANDS = 2011,
    PVMF_STREAMING_MANAGER_RTSP_SESSION_CONTROLLER_PLAYLIST_PLAY = 2014,

    PVMF_STREAMING_MANAGER_JITTER_BUFFER_QUERY_UUID = 3000,
    PVMF_STREAMING_MANAGER_JITTER_BUFFER_QUERY_INTERFACE = 3001,
    PVMF_STREAMING_MANAGER_JITTER_BUFFER_INIT = 3002,
    PVMF_STREAMING_MANAGER_JITTER_BUFFER_PREPARE = 3003,
    PVMF_STREAMING_MANAGER_JITTER_BUFFER_START = 3004,
    PVMF_STREAMING_MANAGER_JITTER_BUFFER_STOP = 3005,
    PVMF_STREAMING_MANAGER_JITTER_BUFFER_FLUSH = 3006,
    PVMF_STREAMING_MANAGER_JITTER_BUFFER_PAUSE = 3007,
    PVMF_STREAMING_MANAGER_JITTER_BUFFER_RESET = 3008,
    PVMF_STREAMING_MANAGER_JITTER_BUFFER_REQUEST_PORT = 3009,
    PVMF_STREAMING_MANAGER_JITTER_BUFFER_RELEASE_PORT = 3010,
    PVMF_STREAMING_MANAGER_JITTER_BUFFER_CANCEL_ALL_COMMANDS = 3011,

    PVMF_STREAMING_MANAGER_MEDIA_LAYER_QUERY_UUID = 4000,
    PVMF_STREAMING_MANAGER_MEDIA_LAYER_QUERY_INTERFACE = 4001,
    PVMF_STREAMING_MANAGER_MEDIA_LAYER_INIT = 4002,
    PVMF_STREAMING_MANAGER_MEDIA_LAYER_PREPARE = 4003,
    PVMF_STREAMING_MANAGER_MEDIA_LAYER_START = 4004,
    PVMF_STREAMING_MANAGER_MEDIA_LAYER_STOP = 4005,
    PVMF_STREAMING_MANAGER_MEDIA_LAYER_FLUSH = 4006,
    PVMF_STREAMING_MANAGER_MEDIA_LAYER_PAUSE = 4007,
    PVMF_STREAMING_MANAGER_MEDIA_LAYER_RESET = 4008,
    PVMF_STREAMING_MANAGER_MEDIA_LAYER_REQUEST_PORT = 4009,
    PVMF_STREAMING_MANAGER_MEDIA_LAYER_RELEASE_PORT = 4010,
    PVMF_STREAMING_MANAGER_MEDIA_LAYER_CANCEL_ALL_COMMANDS = 4011,

    PVMF_STREAMING_MANAGER_HTTP_SESSION_CONTROLLER_QUERY_UUID = 5000,
    PVMF_STREAMING_MANAGER_HTTP_SESSION_CONTROLLER_QUERY_INTERFACE = 5001,
    PVMF_STREAMING_MANAGER_HTTP_SESSION_CONTROLLER_INIT = 5002,
    PVMF_STREAMING_MANAGER_HTTP_SESSION_CONTROLLER_PREPARE = 5003,
    PVMF_STREAMING_MANAGER_HTTP_SESSION_CONTROLLER_START = 5004,
    PVMF_STREAMING_MANAGER_HTTP_SESSION_CONTROLLER_STOP = 5005,
    PVMF_STREAMING_MANAGER_HTTP_SESSION_CONTROLLER_FLUSH = 5006,
    PVMF_STREAMING_MANAGER_HTTP_SESSION_CONTROLLER_PAUSE = 5007,
    PVMF_STREAMING_MANAGER_HTTP_SESSION_CONTROLLER_RESET = 5008,
    PVMF_STREAMING_MANAGER_HTTP_SESSION_CONTROLLER_REQUEST_PORT = 5009,
    PVMF_STREAMING_MANAGER_HTTP_SESSION_CONTROLLER_RELEASE_PORT = 5010,
    PVMF_STREAMING_MANAGER_HTTP_SESSION_CONTROLLER_CANCEL_ALL_COMMANDS = 5011,
    PVMF_STREAMING_MANAGER_HTTP_SESSION_CONTROLLER_SEEK = 5012,
    PVMF_STREAMING_MANAGER_HTTP_SESSION_CONTROLLER_STREAM_SWITCH = 5013,

    PVMF_STREAMING_MANAGER_RTP_PACKET_SOURCE_NODE_QUERY_UUID = 6000,
    PVMF_STREAMING_MANAGER_RTP_PACKET_SOURCE_NODE_QUERY_INTERFACE = 6001,
    PVMF_STREAMING_MANAGER_RTP_PACKET_SOURCE_NODE_INIT = 6002,
    PVMF_STREAMING_MANAGER_RTP_PACKET_SOURCE_NODE_PREPARE = 6003,
    PVMF_STREAMING_MANAGER_RTP_PACKET_SOURCE_NODE_START = 6004,
    PVMF_STREAMING_MANAGER_RTP_PACKET_SOURCE_NODE_STOP = 6005,
    PVMF_STREAMING_MANAGER_RTP_PACKET_SOURCE_NODE_FLUSH = 6006,
    PVMF_STREAMING_MANAGER_RTP_PACKET_SOURCE_NODE_PAUSE = 6007,
    PVMF_STREAMING_MANAGER_RTP_PACKET_SOURCE_NODE_RESET = 6008,
    PVMF_STREAMING_MANAGER_RTP_PACKET_SOURCE_NODE_REQUEST_PORT = 6009,
    PVMF_STREAMING_MANAGER_RTP_PACKET_SOURCE_NODE_RELEASE_PORT = 6010,
    PVMF_STREAMING_MANAGER_RTP_PACKET_SOURCE_NODE_CANCEL_ALL_COMMANDS = 6011,

    PVMF_STREAMING_MANAGER_PVR_NODE_QUERY_UUID = 7000,
    PVMF_STREAMING_MANAGER_PVR_NODE_QUERY_INTERFACE = 7001,
    PVMF_STREAMING_MANAGER_PVR_NODE_INIT = 7002,
    PVMF_STREAMING_MANAGER_PVR_NODE_PREPARE = 7003,
    PVMF_STREAMING_MANAGER_PVR_NODE_START = 7004,
    PVMF_STREAMING_MANAGER_PVR_NODE_STOP = 7005,
    PVMF_STREAMING_MANAGER_PVR_NODE_FLUSH = 7006,
    PVMF_STREAMING_MANAGER_PVR_NODE_PAUSE = 7007,
    PVMF_STREAMING_MANAGER_PVR_NODE_RESET = 7008,
    PVMF_STREAMING_MANAGER_PVR_NODE_REQUEST_PORT = 7009,
    PVMF_STREAMING_MANAGER_PVR_NODE_RELEASE_PORT = 7010,
    PVMF_STREAMING_MANAGER_PVR_NODE_CANCEL_ALL_COMMANDS = 7011,

    PVMF_STREAMING_MANAGER_INTERNAL_COMMAND_LAST = 7999

};

static const char PVMFSTREAMINGMGRNODE_JANUS_AUTH_DATA_V2_HEADER[] = "V2 ASF header";
static const char PVMFSTREAMINGMGRNODE_JANUS_AUTH_DATA_V1_HEADER[] = "V1 ASF header";
#define PVMF_STREAMING_MANAGER_NODE_MAX_CPM_METADATA_KEYS 256

class PVMFSMASFHeaderInfo
{
    public:
        PVMFSMASFHeaderInfo()
        {
            iASFHeaderObject    = NULL;
            iASFFileSize        = 0;
            iASFFileDuration    = 0;
            iASFPreRollDuration = 0;
            iASFBroadCastFlag   = 0;
            iASFStreamRate      = PVMF_STREAMING_MAANGER_NODE_DEFAULT_STREAM_SPEED;
            iASFDRMProtected	= false;
        };

        virtual ~PVMFSMASFHeaderInfo()
        {
            if (iASFHeaderObject != NULL)
            {
                PVMF_STREAMING_MANAGER_DELETE(NULL,
                                              StreamAsfHeader,
                                              iASFHeaderObject);
                iASFHeaderObject = NULL;
            }
        };

        StreamAsfHeader*      iASFHeaderObject;
        uint64        iASFFileSize;
        uint64        iASFFileDuration;
        uint64        iASFPreRollDuration;
        uint8         iASFBroadCastFlag;
        uint32        iASFStreamRate;
        bool		  iASFDRMProtected;
};

class PVMFSMSessionSourceInfo
{
    public:
        PVMFSMSessionSourceInfo()
        {
            _sessionType = PVMF_FORMAT_UNKNOWN;
            _sourceData = NULL;
            iRTSPTunnelling = false;
        }

        uint32 _sessionType;
        OSCL_wHeapString<PVMFStreamingManagerNodeAllocator>    _sessionURL;
        OsclSharedPtr<SDPInfo> _sdpInfo;
        OsclAny*    _sourceData;
        bool iRTSPTunnelling;
        PVMFSMASFHeaderInfo iASFHeaderInfo;
};

//implementation class for extension interface
class PVMFStreamingManagerNode;

class PVMFStreamingManagerExtensionInterfaceImpl :
            public PVInterfaceImpl<PVMFStreamingManagerNodeAllocator>,
            public PVMFDataSourceInitializationExtensionInterface,
            public PVMFTrackSelectionExtensionInterface,
            public PVMFStreamingManagerExtensionInterface,
            public PvmfDataSourcePlaybackControlInterface,
            public PVMFMetadataExtensionInterface,
            public PvmiCapabilityAndConfig,
            public PVMFCPMPluginLicenseInterface
{
    public:
        PVMFStreamingManagerExtensionInterfaceImpl(PVMFStreamingManagerNode*,
                PVMFSessionId);
        ~PVMFStreamingManagerExtensionInterfaceImpl();

        /*
         * From PVMFDataSourceInitializationExtensionInterface
         */
        OSCL_IMPORT_REF PVMFStatus SetSourceInitializationData(OSCL_wString& aSourceURL,
                PVMFFormatType& aSourceFormat,
                OsclAny* aSourceData);
        OSCL_IMPORT_REF PVMFStatus SetClientPlayBackClock(PVMFMediaClock* aClientClock);
        OSCL_IMPORT_REF PVMFStatus SetEstimatedServerClock(PVMFMediaClock* aClientClock);

        /*
         * From PVMFTrackSelectionExtensionInterface
         */
        OSCL_IMPORT_REF PVMFStatus GetMediaPresentationInfo(PVMFMediaPresentationInfo& aInfo);
        OSCL_IMPORT_REF PVMFStatus SelectTracks(PVMFMediaPresentationInfo& aInfo);

        /*
         * from PVMFStreamingManagerExtensionInterface
         */
        OSCL_IMPORT_REF PVMFStatus setClientParameters(PVMFSMClientParams* clientParams);
        OSCL_IMPORT_REF PVMFStatus setTimeParams(PVMFSMTimeParams* timeParams);
        OSCL_IMPORT_REF PVMFStatus setAuthenticationParameters(PVMFSMAuthenticationParmas* authenticationParams);
        OSCL_IMPORT_REF PVMFStatus setJitterBufferParams(PVMFSMJitterBufferParmas* jitterBufferParams);
        OSCL_IMPORT_REF PVMFStatus resetJitterBuffer();
        OSCL_IMPORT_REF PVMFStatus setPayloadParserRegistry(PayloadParserRegistry*);
        OSCL_IMPORT_REF PVMFStatus setDataPortLogging(bool logEnable, OSCL_String* logPath = NULL);
        OSCL_IMPORT_REF PVMFStatus switchStreams(uint32 aSrcStreamID, uint32 aDestStreamID);

        /*
         * From PvmfDataSourcePlaybackControlInterface
         */
        OSCL_IMPORT_REF PVMFCommandId SetDataSourcePosition(PVMFSessionId aSessionId,
                PVMFTimestamp aTargetNPT,
                PVMFTimestamp& aActualNPT,
                PVMFTimestamp& aActualMediaDataTS,
                bool aSeekToSyncPoint = true,
                uint32 aStreamID = 0,
                OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId SetDataSourcePosition(PVMFSessionId aSessionId,
                PVMFDataSourcePositionParams& aPVMFDataSourcePositionParams,
                OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId QueryDataSourcePosition(PVMFSessionId aSessionId,
                PVMFTimestamp aTargetNPT,
                PVMFTimestamp& aActualNPT,
                bool aSeekToSyncPoint = true,
                OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId QueryDataSourcePosition(PVMFSessionId aSessionId,
                PVMFTimestamp aTargetNPT,
                PVMFTimestamp& aSeekPointBeforeTargetNPT,
                PVMFTimestamp& aSeekPointAfterTargetNPT,
                OsclAny* aContext = NULL,
                bool aSeekToSyncPoint = true);
        OSCL_IMPORT_REF PVMFCommandId SetDataSourceRate(PVMFSessionId aSessionId,
                int32 aRate,
                PVMFTimebase* aTimebase = NULL,
                OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFStatus ComputeSkipTimeStamp(PVMFTimestamp aTargetNPT,
                PVMFTimestamp aActualNPT,
                PVMFTimestamp aActualMediaDataTS,
                PVMFTimestamp& aSkipTimeStamp,
                PVMFTimestamp& aStartNPT);

        /*
         * From PVMFMetadataExtensionInterface
         */
        OSCL_IMPORT_REF uint32 GetNumMetadataKeys(char* aQueryKeyString = 0);
        OSCL_IMPORT_REF uint32 GetNumMetadataValues(PVMFMetadataList& aKeyList);
        OSCL_IMPORT_REF PVMFCommandId GetNodeMetadataKeys(PVMFSessionId aSessionId,
                PVMFMetadataList& aKeyList,
                uint32 aStartingKeyIndex,
                int32 aMaxKeyEntries,
                char* aQueryKeyString = NULL,
                const OsclAny* aContextData = NULL);
        OSCL_IMPORT_REF PVMFCommandId GetNodeMetadataValues(PVMFSessionId aSessionId,
                PVMFMetadataList& aKeyList,
                Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList,
                uint32 aStartingValueIndex,
                int32 aMaxValueEntries,
                const OsclAny* aContextData = NULL);
        OSCL_IMPORT_REF PVMFStatus ReleaseNodeMetadataKeys(PVMFMetadataList& aKeyList,
                uint32 aStartingKeyIndex,
                uint32 aEndKeyIndex);
        OSCL_IMPORT_REF PVMFStatus ReleaseNodeMetadataValues(Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList,
                uint32 aStartingValueIndex,
                uint32 aEndValueIndex);

        //
        // PvmiCapabilityAndConfig
        //
        void setObserver(PvmiConfigAndCapabilityCmdObserver* aObserver)
        {
            iContainer->setObserver(aObserver);
        };

        PVMFStatus getParametersSync(PvmiMIOSession aSession, PvmiKeyType aIdentifier,
                                     PvmiKvp*& aParameters, int& num_parameter_elements,
                                     PvmiCapabilityContext aContext)
        {
            return iContainer->getParametersSync(aSession, aIdentifier, aParameters, num_parameter_elements, aContext);
        };

        PVMFStatus releaseParameters(PvmiMIOSession aSession, PvmiKvp* aParameters,
                                     int num_elements)
        {
            return iContainer->releaseParameters(aSession, aParameters, num_elements);
        };

        void createContext(PvmiMIOSession aSession, PvmiCapabilityContext& aContext)
        {
            iContainer->createContext(aSession, aContext);
        };

        void setContextParameters(PvmiMIOSession aSession, PvmiCapabilityContext& aContext,
                                  PvmiKvp* aParameters, int num_parameter_elements)
        {
            iContainer->setContextParameters(aSession, aContext, aParameters, num_parameter_elements);
        };

        void DeleteContext(PvmiMIOSession aSession, PvmiCapabilityContext& aContext)
        {
            iContainer->DeleteContext(aSession, aContext);
        };

        void setParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters,
                               int num_elements, PvmiKvp * & aRet_kvp)
        {
            iContainer->setParametersSync(aSession, aParameters, num_elements, aRet_kvp);
        };

        PVMFCommandId setParametersAsync(PvmiMIOSession aSession, PvmiKvp* aParameters,
                                         int num_elements, PvmiKvp*& aRet_kvp,
                                         OsclAny* context = NULL)
        {
            return iContainer->setParametersAsync(aSession, aParameters, num_elements, aRet_kvp, context);
        };

        uint32 getCapabilityMetric(PvmiMIOSession aSession)
        {
            return iContainer->getCapabilityMetric(aSession);
        };

        PVMFStatus verifyParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters, int num_elements)
        {
            return iContainer->verifyParametersSync(aSession, aParameters, num_elements);
        };

        //
        // From PVMFCPMPluginLicenseInterface
        //
        OSCL_IMPORT_REF PVMFCommandId GetLicense(PVMFSessionId aSessionId,
                OSCL_wString& aContentName,
                OsclAny* aData,
                uint32 aDataSize,
                int32 aTimeoutMsec,
                OsclAny* aContextData) ;

        OSCL_IMPORT_REF PVMFCommandId GetLicense(PVMFSessionId aSessionId,
                OSCL_String&  aContentName,
                OsclAny* aData,
                uint32 aDataSize,
                int32 aTimeoutMsec,
                OsclAny* aContextData);

        OSCL_IMPORT_REF PVMFCommandId CancelGetLicense(PVMFSessionId aSessionId
                , PVMFCommandId aCmdId
                , OsclAny* aContextData = NULL);

        PVMFStatus GetLicenseURL(PVMFSessionId aSessionId,
                                 OSCL_wString& aContentName,
                                 OSCL_wString& aLicenseURL)
        {
            OSCL_UNUSED_ARG(aSessionId);
            OSCL_UNUSED_ARG(aContentName);
            OSCL_UNUSED_ARG(aLicenseURL);
            //must use Async method.
            return PVMFErrNotSupported;
        }

        PVMFStatus GetLicenseURL(PVMFSessionId aSessionId,
                                 OSCL_String&  aContentName,
                                 OSCL_String&  aLicenseURL)
        {
            OSCL_UNUSED_ARG(aSessionId);
            OSCL_UNUSED_ARG(aContentName);
            OSCL_UNUSED_ARG(aLicenseURL);
            //must use Async method.
            return PVMFErrNotSupported;
        }
        PVMFStatus GetLicenseStatus(
            PVMFCPMLicenseStatus& aStatus) ;

        OSCL_IMPORT_REF void addRef();
        OSCL_IMPORT_REF void removeRef();
        OSCL_IMPORT_REF bool queryInterface(const PVUuid& uuid, PVInterface*& iface);

    private:
        PVMFSessionId iSessionId;
        PVMFStreamingManagerNode *iContainer;
        friend class PVMFStreamingManagerNode;
};

#endif


