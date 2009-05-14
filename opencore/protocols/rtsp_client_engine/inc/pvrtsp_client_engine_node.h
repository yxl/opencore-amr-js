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
#ifndef PVRTSP_CLIENT_ENGINE_NODE_H
#define PVRTSP_CLIENT_ENGINE_NODE_H


#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef OSCL_UTF8CONV_H
#include "oscl_utf8conv.h"
#endif

#ifndef OSCL_ERROR_CODES_H_INCLUDED
#include "oscl_error_codes.h"
#endif

#ifndef OSCL_SCHEDULER_AO_H_INCLUDED
#include "oscl_scheduler_ao.h"
#endif

#ifndef OSCL_SOCKET_TYPES_H_INCLUDED
#include "oscl_socket_types.h"
#endif

#ifndef OSCL_SOCKET_H_INCLUDED
#include "oscl_socket.h"
#endif

#ifndef OSCL_DNS_H_INCLUDED
#include "oscl_dns.h"
#endif

#ifndef OSCL_STRING_CONTAINERS_H_INCLUDED
#include "oscl_string_containers.h"
#endif

#ifndef PVMF_MEDIA_CLOCK_H_INCLUDED
#include "pvmf_media_clock.h"
#endif

#ifndef OSCL_TIMER_H_INCLUDED
#include "oscl_timer.h"
#endif

#ifndef PVMF_FORMAT_TYPE_H_INCLUDED
#include "pvmf_format_type.h"
#endif

#ifndef PVMF_NODE_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
#endif

#ifndef PVMF_NODE_UTILS_H_INCLUDED
#include "pvmf_node_utils.h"
#endif

#ifndef OSCL_PRIQUEUE_H_INCLUDED
#include "oscl_priqueue.h"
#endif

#ifndef PVMF_SIMPLE_MEDIA_BUFFER_H_INCLUDED
#include "pvmf_simple_media_buffer.h"
#endif

#ifndef OSCL_MEM_MEMPOOL_H_INCLUDED
#include "oscl_mem_mempool.h"
#endif

#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif

#ifndef PVRTSP_ENGINE_NODE_EXTENSION_INTERFACE_H_INCLUDED
#include "pvrtspenginenodeextensioninterface.h"
#endif

#ifndef RTSP_PAR_COM_MESSAGE_DS_H_
#include "rtsp_par_com_message.h"
#endif

#ifndef RTSP_PARSER_H_
#include "rtsp_parser.h"
#endif

#ifndef SDP_PARSER_H
#include "sdp_parser.h"
#endif

#ifndef PVMF_RTSP_PORT_H_INCLUDED
#include "pvrtsp_client_engine_port.h"
#endif

#ifndef OSCL_FILEIO_H_INCLUDED
#include "oscl_file_io.h"
#endif

#ifndef PVMF_STREAMING_REAL_INTERFACES_INCLUDED
#include "pvmf_streaming_real_interfaces.h"
#endif

#ifndef PAYLOAD_PARSER_H_INCLUDED
#include "payload_parser.h"
#endif

#ifndef PVMF_SM_CONFIG_H_INCLUDED
#include "pvmf_sm_config.h"
#endif

//Default vector reserve size
#define PVMF_RTSP_ENGINE_NODE_COMMAND_VECTOR_RESERVE 10

//Starting value for command IDs
#define PVMF_RTSP_ENGINE_NODE_COMMAND_ID_START 6000

#define MAX_TOTAL_TRACKS 4 /* maximum number of channels per session */

//The RTSP state enumeration
typedef enum
{
    STATE_INIT,
    STATE_READY,
    STATE_PLAYING,
    //STATE_RECORDING
} SessionState;

typedef struct _DataChannelInfo
{
    uint8 *hexSID;

    bool   ssrc_is_set;
    uint32 ssrc;

    bool   seqbase_is_set;
    uint32 seq;

    bool   rtptime_is_set;
    uint32 rtptime;

    int32  offset;
    uint32 timescale;
    uint32 rtcp_interval;
} DataChannelInfo;

//memory allocator type for this node.
typedef OsclMemAllocator PVRTSPEngineNodeAllocator;

class SessionInfo
{
    public:
        //bool sdpFlag;			    //Used to make sure that we read into the mediainfo array after it has been assigned
        //SDP_ERROR_CODE sdpErrorCode;            //Saves the value returned from the sdp parser

        OSCL_HeapString<PVRTSPEngineNodeAllocator> iSessionURL;
        OSCL_HeapString<PVRTSPEngineNodeAllocator> iContentBaseURL;//Per session control URL from content base field

        OSCL_HeapString<PVRTSPEngineNodeAllocator> iServerName;	//could be either DNS or ip address

        OSCL_HeapString<PVRTSPEngineNodeAllocator> iProxyName;	//could be either DNS or ip address
        uint32	iProxyPort;

        OsclNetworkAddress iSrvAdd;

        bool bExternalSDP;			//true if got SDP through external(NOT DESCRIBE) methods

        RtspRangeType	iReqPlayRange;
        RtspRangeType	iActPlayRange;

        OsclRefCounterMemFrag	pSDPBuf;

        OsclSharedPtr<SDPInfo>	iSDPinfo;
        //SDP_Parser *iSDPparser;		//Pointer to the SDP_Parser class object
        Oscl_Vector<StreamInfo, PVRTSPEngineNodeAllocator> iSelectedStream;

        //Form the track to channel association table. This table is used while
        //interacting with the media buffer. This table is needed to convert from an index
        //based on the number of tracks in the SDP to an index into the number of tracks
        //actually selected. For eg, the SDP might have 5 media tracks, but the number of tracks
        //that get selected could be 2. If the first track that gets selected corresponds to the third
        //track in the SDP list, then the mapping would be 2 -> 0. Similarly, if the second track that
        //gets selected is the fifth in the SDP list, the mapping would be 4 -> 1.
        //All indices begin from 0.
        /*
        for(int ii = 0; ii < numberOfChannels; ii++)
        {
            int sdp_track_id = pSessionInfo->trackSelectionList->getTrackIndex(ii);
            pSessionInfo->channel_tbl[sdp_track_id] = ii;
        }
        */
        int number_of_channels;                 //Used for interaction with the media buffer class
        int channel_tbl[MAX_TOTAL_TRACKS];      //Index from SDP track IDs to tracks actually selected

        OSCL_HeapString<PVRTSPEngineNodeAllocator> iSID;	//alphanumeric session ID

        //OsclMemoryFragment pImgBuf;

        bool serverReplyFlag;		    //Used for communication between the PVStream modules (network module and the task scheduler)
        bool getStateFlag;			    //Used to ensure that the PE's main loop has been called
        bool pvServerIsSetFlag; 		    //Used to indicate if we are streaming from PVServer or not
        bool tSIDIsSetFlag;			    //Used to send session id from second SETUP request
        uint32 iServerVersionNumber;    // Version number of PVSS
        int32 prerollDuration;                  //Saves the jitter buffer size

        int32 fwp_counter;			    //Id of the first returned firewall packet
        int32 rtt;                              //Saves the round trip time for firewall exchange
        uint32 roundTripDelay;                   //Saves the round trip delay for a DESCRIBE request
        uint64 clientServerDelay;                //Saves the client server delay during a DESCRIBE request

        bool pipeLineFlag;                      //Used to indicate a pipe lined request

        OSCL_HeapString<PVRTSPEngineNodeAllocator>	iUserAgent;
        OSCL_HeapString<PVRTSPEngineNodeAllocator>	iUserNetwork;
        OSCL_HeapString<PVRTSPEngineNodeAllocator>	iDeviceInfo;

        OSCL_HeapString<PVRTSPEngineNodeAllocator>	iUserID;
        OSCL_HeapString<PVRTSPEngineNodeAllocator>	iAuthentication;
        OSCL_HeapString<PVRTSPEngineNodeAllocator>	iExpiration;
        OSCL_HeapString<PVRTSPEngineNodeAllocator>	iApplicationSpecificString;
        OSCL_HeapString<PVRTSPEngineNodeAllocator>	iVerification;
        OSCL_HeapString<PVRTSPEngineNodeAllocator>	iSignature;

        enum PVRTSPStreamingType iStreamingType;

        bool									   iSessionCompleted;
        void UpdateSessionCompletionStatus(bool aSessionCompleted)
        {
            iSessionCompleted = aSessionCompleted;
        }

    public:
        SessionInfo():
                iProxyPort(0),
                bExternalSDP(false),
                pvServerIsSetFlag(false),
                iServerVersionNumber(0),
                roundTripDelay(0),
                iSessionCompleted(false)
        {
            iUserAgent += _STRLIT_CHAR("PVPlayer4.0");
            iReqPlayRange.format = RtspRangeType::INVALID_RANGE;
        };

} ;

class RTSPNodeMemDestructDealloc : public OsclDestructDealloc
{
    public:
        virtual void destruct_and_dealloc(OsclAny *ptr)
        {
            OSCL_FREE(ptr);
        }
};

typedef PVMFGenericNodeCommand<PVRTSPEngineNodeAllocator> PVRTSPEngineCommandBase;

enum TPVMFRtspNodeCommand //TPVMFGenericNodeCommand
{
    PVMF_RTSP_NODE_ERROR_RECOVERY = PVMF_GENERIC_NODE_COMMAND_LAST + 1
    , PVMF_RTSP_NODE_CANCELALLRESET
};
class PVRTSPEngineCommand: public PVRTSPEngineCommandBase
{
    public:

        void Construct(PVMFSessionId s, int32 cmd, int32 arg1, int32 arg2, int32& arg3, \
                       const OsclAny*aContext)
        {
            PVRTSPEngineCommandBase::Construct(s, cmd, aContext);
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
        virtual bool hipri()
        {
            if (iCmd == PVMF_RTSP_NODE_ERROR_RECOVERY)
                return true;

            return PVRTSPEngineCommandBase::hipri();
        }

        /*
        PVMFSessionId iCmdSid;
        int32 iCmdId;
        TPVMFGenericNodeCommand iCmdType;
        OsclAny* iContext;
        PVMFPortInterface* iPort;
        OsclAny* iData1;
        OsclAny* iData2;
        OsclAny* iData3;
        */
    private:
        //PVRTSPEngineCommand(const PVRTSPEngineCommand& aCmd);
};

//Command queue type
typedef PVMFNodeCommandQueue<PVRTSPEngineCommand, PVRTSPEngineNodeAllocator> PVRTSPEngineNodeCmdQ;

class PVRTSPGenericMessageCompareLess
{
    public:
        /**
        * The algorithm used in OsclPriorityQueue needs a compare function
        * that returns true when A's priority is less than B's
        * @return true if A's priority is less than B's, else false
        	*/
        int compare(RTSPGenericMessage* a, RTSPGenericMessage* b) const
        {
            return (PVRTSPGenericMessageCompareLess::GetPriority(*a) > PVRTSPGenericMessageCompareLess::GetPriority(*b));
        }

        /**
        * Returns the priority of each command
        * @return A 0-based priority number. A lower number indicates lower priority.
        */
        static int GetPriority(RTSPGenericMessage &aCmd)
        {//for cseq numbers, the low numbers come first. so higher priority
            return aCmd.cseq;
        }
};

class GetPostCorrelationObject
{
    public:
        // factory method
        static GetPostCorrelationObject *create(OSCL_TCHAR* aFileName = NULL);
        // destructor
        ~GetPostCorrelationObject();

        // get post correlation value
        uint8 get() const
        {
            return iGetPostCorrelation;
        }
        // increase get post correlation value by 1 within [1, 255]
        bool update();

    private:
        // constructor
        GetPostCorrelationObject()
        {
            ;
        }
        bool construct(OSCL_TCHAR* aFileName);
        void closeFile();
        bool writeToFile();

    private:
        uint8 iGetPostCorrelation;
        bool iFileCreated; // check for the file creation

        // File IO stuff
        Oscl_FileServer iFs;
        Oscl_File iGetPostCorrelationFile;
};

class PVRTSPEngineNode
            : public PVInterface,
            public PVMFNodeInterface,
            public OsclTimerObject,
            public OsclSocketObserver,
            public OsclDNSObserver,
            public OsclTimerObserver,
            public OsclMemPoolFixedChunkAllocatorObserver
{
    public:
        OSCL_IMPORT_REF PVRTSPEngineNode(int32 aPriority);
        OSCL_IMPORT_REF virtual ~PVRTSPEngineNode();

        //************ begin PVMFNodeInterface

        OSCL_IMPORT_REF virtual  PVMFStatus ThreadLogon();
        OSCL_IMPORT_REF virtual  PVMFStatus ThreadLogoff();

        /**
        GetCapability can be invoked only when after a node is initialized
        **/
        OSCL_IMPORT_REF virtual  PVMFStatus GetCapability(PVMFNodeCapability& aNodeCapability);

        /**
         * Returns a list of ports currently available in the node that meet the filter criteria
         * We can add fancier iterators and filters as needed.
         * For now we return all the available ports.  If no ports are present, NULL is returned
         **/
        OSCL_IMPORT_REF virtual  PVMFPortIter* GetPorts(const PVMFPortFilter* aFilter = NULL);

        /**
         * This API is to allow for extensibility of the PVMF Node interface.
         * It allows a caller to ask for all UUIDs associated with a particular MIME type.
         * If interfaces of the requested MIME type are found within the system, they are added
         * to the UUIDs array.
         *
         * Also added to the UUIDs array will be all interfaces which have the requested MIME
         * type as a base MIME type.  This functionality can be turned off.
         *
         * @param aMimeType The MIME type of the desired interfaces
         * @param aUuids A vector to hold the discovered UUIDs
         * @param aExactUuidsOnly Turns on/off the retrival of UUIDs with aMimeType as a base type
         * @param aContext Optional opaque data to be passed back to user with the command response
         * @returns A unique command id for asynchronous completion
         */
        OSCL_IMPORT_REF virtual PVMFCommandId QueryUUID(PVMFSessionId aSession
                , const PvmfMimeString& aMimeType
                , Oscl_Vector<PVUuid, PVRTSPEngineNodeAllocator>& aUuids
                , bool aExactUuidsOnly = false
                                         , const OsclAny* aContext = NULL);

        /**
         * This API is to allow for extensibility of the PVMF Node interface.
         * It allows a caller to ask for an instance of a particular interface object to be returned.
         * The mechanism is analogous to the COM IUnknown method.  The interfaces are identified with
         * an interface ID that is a UUID as in DCE and a pointer to the interface object is
         * returned if it is supported.  Otherwise the returned pointer is NULL.
         *
         * @param aUuid The UUID of the desired interface
         * @param aInterfacePtr The output pointer to the desired interface
         * @param aContext Optional opaque data to be passed back to user with the command response
         * @returns A unique command id for asynchronous completion
         */
        OSCL_IMPORT_REF virtual PVMFCommandId QueryInterface(PVMFSessionId aSession
                , const PVUuid& aUuid
                , PVInterface*& aInterfacePtr
                , const OsclAny* aContext = NULL);

        /**
         * Requests the node to return a port meeting certain criteria for format types and buffering
         * capabilities.  The node may return a reference to an already created unused port or it may dynamically
         * create one if it has the capability to do so.  Since there might be some port specific initializations
         * that might need to be done for ports created on demand, it will be most flexible to have this as an
         * asynchronous API.
         * A reference to the port interface is returned with the the command completion.  It is passed as an auto ptr
         * carrying opaque data that needs to be cast to PVMFPortInterface*
         * @exception PVMFErrNotSupported leaves if this is not supported.
         **/
        OSCL_IMPORT_REF virtual PVMFCommandId RequestPort(PVMFSessionId aSession
                , int32 aPortTag
                , const PvmfMimeString* aPortConfig = NULL
                                                      , const OsclAny* aContext = NULL);

        /**
         * Releases a port back to the owning node.
         * @exception PVMFErrArgument leaves if this node is not the owner.
         **/
        OSCL_IMPORT_REF virtual PVMFCommandId ReleasePort(PVMFSessionId aSession
                , PVMFPortInterface& aPort
                , const OsclAny* aContext = NULL);

        //Describe,
        OSCL_IMPORT_REF virtual PVMFCommandId Init(PVMFSessionId aSession
                , const OsclAny* aContext = NULL);

        OSCL_IMPORT_REF virtual PVMFCommandId Prepare(PVMFSessionId aSession,
                const OsclAny* aContext = NULL);

        //Setup, Firewall pkts exchange, Play
        /**
         * Causes the node to start servicing all connected ports.
         **/
        OSCL_IMPORT_REF virtual PVMFCommandId Start(PVMFSessionId aSession
                , const OsclAny* aContext = NULL);

        //Pause
        /**
         * Causes the node to pause servicing all connected ports without
         * discarding un-processed data.
         **/
        OSCL_IMPORT_REF virtual PVMFCommandId Pause(PVMFSessionId aSession
                , const OsclAny* aContext = NULL);

        //Teardown
        OSCL_IMPORT_REF virtual PVMFCommandId Stop(PVMFSessionId aSession
                , const OsclAny* aContext = NULL);

        /**
         * Resets the node.  The node should relinquish all resources that is has acquired as part of the
         * initialization process and should be ready to be deleted when this completes.
         **/
        OSCL_IMPORT_REF virtual PVMFCommandId Reset(PVMFSessionId aSession
                , const OsclAny* aContext = NULL);

        /**
         * Causes the node to stop servicing all connected ports as
         * soon as current data is processed.
         **/
        OSCL_IMPORT_REF virtual PVMFCommandId Flush(PVMFSessionId aSession
                , const OsclAny* aContext = NULL);


        /**
         * Cancel all pending requests. The current request being processed, if any, will also be aborted.
         *
         * @param aContextData Optional opaque data that will be passed back to the user with the command response
         * @returns A unique command id for asynchronous completion
         */
        OSCL_IMPORT_REF virtual PVMFCommandId CancelAllCommands(PVMFSessionId aSession
                , const OsclAny* aContextData = NULL);

        /**
         * Cancels pending command with the specified ID.
         *
         * @param aCmdId Command Id of the command to be cancelled
         * @param aContextData Optional opaque data that will be passed back to the user with the command response
         * @returns A unique command id for asynchronous completion
         */
        OSCL_IMPORT_REF virtual PVMFCommandId CancelCommand(PVMFSessionId aSession
                , PVMFCommandId aCmdId
                , const OsclAny* aContextData = NULL);

        /**
         * Ports call this API to report activity to the node.
         *
         * @param aActivity Information regarding the activity.
         */
        OSCL_IMPORT_REF void HandlePortActivity(const PVMFPortActivity& aActivity);

        //************ end PVMFNodeInterface

        //************ begin PVRTSPEngineNodeExtensionInterface
        OSCL_IMPORT_REF virtual void addRef();
        OSCL_IMPORT_REF virtual void removeRef(void);
        OSCL_IMPORT_REF virtual bool queryInterface(const PVUuid& uuid, PVInterface*& iface);

        OSCL_IMPORT_REF virtual PVMFStatus SetStreamingType(PVRTSPStreamingType aType = PVRTSP_3GPP_UDP);

        OSCL_IMPORT_REF virtual PVMFStatus SetSessionURL(OSCL_wString& aURL);

        OSCL_IMPORT_REF virtual PVMFStatus SetRtspProxy(OSCL_String& aRtspProxyName, uint32 aRtspProxyPort);
        OSCL_IMPORT_REF virtual PVMFStatus GetRtspProxy(OSCL_String& aRtspProxyName, uint32& aRtspProxyPort);

        OSCL_IMPORT_REF virtual PVMFStatus GetSDP(OsclRefCounterMemFrag& aSDPBuf);
        OSCL_IMPORT_REF virtual PVMFStatus SetSDPInfo(OsclSharedPtr<SDPInfo>& aSDPinfo, Oscl_Vector<StreamInfo, PVRTSPEngineNodeAllocator> &aSelectedStream);
        OSCL_IMPORT_REF virtual PVMFStatus GetServerInfo(PVRTSPEngineNodeServerInfo& aServerInfo);
        OSCL_IMPORT_REF virtual PVMFStatus GetStreamInfo(Oscl_Vector<StreamInfo, PVRTSPEngineNodeAllocator> &aSelectedStream);

        OSCL_IMPORT_REF virtual PVMFStatus GetUserAgent(OSCL_wString& aUserAgent);

        OSCL_IMPORT_REF virtual PVMFStatus SetClientParameters(OSCL_wString& aUserAgent,
                OSCL_wString&  aUserNetwork,
                OSCL_wString&  aDeviceInfo);

        OSCL_IMPORT_REF virtual bool IsRdtTransport();
        OSCL_IMPORT_REF virtual void SetPortRdtStreamId(PVMFPortInterface* pPort,
                int iRdtStreamId);

        OSCL_IMPORT_REF virtual void SetRealChallengeCalculator(IRealChallengeGen* pChallengeCalc);
        OSCL_IMPORT_REF virtual void SetRdtParser(IPayloadParser* pRdtParser);

        OSCL_IMPORT_REF virtual PVMFStatus SetAuthenticationParameters(OSCL_wString& aUserID,
                OSCL_wString& aAuthentication,
                OSCL_wString& aExpiration,
                OSCL_wString& aApplicationSpecificString,
                OSCL_wString& aVerification,
                OSCL_wString& aSignature);

        OSCL_IMPORT_REF virtual PVMFStatus SetRequestPlayRange(const RtspRangeType& aRange);
        OSCL_IMPORT_REF virtual PVMFStatus GetActualPlayRange(RtspRangeType& aRange);

        OSCL_IMPORT_REF virtual PVMFStatus SetKeepAliveMethod_timeout(int32 aTimeout = 0);
        OSCL_IMPORT_REF virtual PVMFStatus SetKeepAliveMethod_use_SET_PARAMETER(bool aUseSetParameter = false);
        OSCL_IMPORT_REF virtual PVMFStatus SetKeepAliveMethod_keep_alive_in_play(bool aKeepAliveInPlay = false);

        OSCL_IMPORT_REF virtual PVMFStatus GetKeepAliveMethod(int32 &aTimeout, bool &aUseSetParameter, bool &aKeepAliveInPlay);


        OSCL_IMPORT_REF virtual PVMFStatus GetRTSPTimeOut(int32 &aTimeout);
        OSCL_IMPORT_REF virtual PVMFStatus SetRTSPTimeOut(int32 aTimeout);

        //************ end PVRTSPEngineNodeExtensionInterface

        //************ begin OsclTimerObserver
        OSCL_IMPORT_REF virtual void TimeoutOccurred(int32 timerID, int32 timeoutInfo);

        //************ begin OsclSocketObserver
        OSCL_IMPORT_REF virtual void HandleSocketEvent(int32 aId, TPVSocketFxn aFxn, TPVSocketEvent aEvent, int32 aError);
        //************ end OsclSocketObserver

        //************ begin OsclDNSObserver
        OSCL_IMPORT_REF virtual void HandleDNSEvent(int32 aId, TPVDNSFxn aFxn, TPVDNSEvent aEvent, int32 aError);
        //************ end OsclDNSObserver

        //************ begin OsclMemPoolFixedChunkAllocatorObserver
        void freechunkavailable(OsclAny*);
        //************ end OsclMemPoolFixedChunkAllocatorObserver

        void UpdateSessionCompletionStatus(bool aSessionCompleted)
        {
            iSessionInfo.iSessionCompleted = aSessionCompleted;
        }

        bool IsSessionCompleted() const
        {
            return iSessionInfo.iSessionCompleted;
        }

        typedef struct _SocketEvent
        {
            int32			iSockId;
            TPVSocketFxn	iSockFxn;
            TPVSocketEvent	iSockEvent;
            int32			iSockError;
        }SocketEvent;

        enum PVRTSPEngineState
        {
            //for async request
            PVRTSP_ENGINE_NODE_STATE_IDLE,
            PVRTSP_ENGINE_NODE_STATE_DNS_RESOLVING,

            PVRTSP_ENGINE_NODE_STATE_CONNECT,
            PVRTSP_ENGINE_NODE_STATE_CONNECTING,

            PVRTSP_ENGINE_NODE_STATE_HTTP_CLOAKING_SETUP,

            PVRTSP_ENGINE_NODE_STATE_SEND_OPTIONS,
            PVRTSP_ENGINE_NODE_STATE_SEND_DESCRIBE,
            PVRTSP_ENGINE_NODE_STATE_OPTIONS_WAITING,
            PVRTSP_ENGINE_NODE_STATE_DESCRIBE_WAITING,

            PVRTSP_ENGINE_NODE_STATE_DESCRIBE_DONE,
            PVRTSP_ENGINE_NODE_STATE_PROCESS_REST_SETUP,

            PVRTSP_ENGINE_NODE_STATE_SETUP_DONE,
            PVRTSP_ENGINE_NODE_STATE_WAIT_PLAY,

            PVRTSP_ENGINE_NODE_STATE_PLAY_DONE,
            PVRTSP_ENGINE_NODE_STATE_WAIT_PAUSE,
            PVRTSP_ENGINE_NODE_STATE_PAUSE_DONE,

            PVRTSP_ENGINE_NODE_STATE_WAIT_STOP,

            PVRTSP_ENGINE_NODE_STATE_WAIT_CALLBACK,

            PVRTSP_ENGINE_NODE_STATE_INVALID
        }iState;

    private:
        OsclSharedPtr<PVMFMediaDataImpl> AllocateMediaData(int32& errCode);
        Oscl_Vector<SocketEvent, PVRTSPEngineNodeAllocator> iSocketEventQueue;

        PVRTSPEngineNodeAllocator iAlloc;

        PVMFCommandId iCurrentCmdId;

        OsclSocketServ	*iSockServ;

//note: this class is for internal use only, but must be public to avoid ADS v1.2 compile error.
    public:
        //To keep track of current socket op
        class SocketState
        {
            public:
                SocketState()
                        : iPending(false)
                        , iCanceled(false)
                {}
                bool iPending;
                bool iCanceled;
                void Reset()
                {
                    iPending = iCanceled = false;
                }
        };

    private:

        //Container for a TCP socket that can connect, send, & recv.
        class SocketContainer
        {
            public:
                SocketContainer(): iSocket(NULL)
                {}
                OsclTCPSocket* iSocket;
                SocketState iConnectState;
                SocketState iSendState;
                SocketState iRecvState;
                SocketState iShutdownState;
                void Reset(OsclTCPSocket* aSock)
                {
                    iSocket = aSock;
                    iConnectState.Reset();
                    iSendState.Reset();
                    iRecvState.Reset();
                    iShutdownState.Reset();
                }
                bool IsBusy()
                {
                    return iSocket
                           && (iConnectState.iPending
                               || iSendState.iPending
                               || iRecvState.iPending
                               || iShutdownState.iPending);
                }
        };
        class DnsContainer
        {
            public:
                DnsContainer(): iDns(NULL)
                {}
                OsclDNS* iDns;
                SocketState iState;
                bool IsBusy()
                {
                    return (iDns
                            && iState.iPending);
                }
        };
        void SetSendPending(SocketContainer&);
        void SetRecvPending(SocketContainer&);

        SocketContainer iSendSocket, iRecvSocket;
        DnsContainer iDNS;

        //To keep track of socket reset sequence.
        enum TSocketCleanupState
        {
            ESocketCleanup_Idle
            , ESocketCleanup_CancelCurrentOp
            , ESocketCleanup_WaitOnCancel
            , ESocketCleanup_Shutdown
            , ESocketCleanup_WaitOnShutdown
            , ESocketCleanup_Delete
        };
        TSocketCleanupState iSocketCleanupState;

        //only for http cloaking, string to store the text to send until send completes
        OSCL_HeapString<PVRTSPEngineNodeAllocator> iRecvChannelMsg, iSendChannelMsg;

        RTSPParser *iRTSPParser;
        RTSPParser::ParserState iRTSPParserState;
        RTSPIncomingMessage iIncomingMsg;
        uint32 iOutgoingSeq;

        bool bNoRecvPending;//an Recv() is pending on RTSP socket
        bool bNoSendPending;//a Send() is pending on RTSP socket

        PVMFPortInterface* iTheBusyPort;
        //OsclMemoryFragment entityBody;		    //Used to register with RTSP parser
        //OsclMemoryFragment embeddedDataMemory;	//Used to save embedded binary data

        SessionInfo iSessionInfo;

        PVLogger* iLogger;

        //uint8 iBufEmbedded[2048];
        //RTSPEntityBody iEmbeddedData;
        PVMFSharedMediaDataPtr	iEmbeddedDataPtr;
        // Reference counter for extension
        uint32 iExtensionRefCount;
        uint32 iNumRedirectTrials;

        //socket server will callback even if Cancel() is called
        //most likely this is the case for OsclDNS as well
        //But this is NOT the case for OsclTimer
        uint32	iNumHostCallback, iNumConnectCallback, iNumSendCallback, iNumRecvCallback;
        int BASE_REQUEST_ID;
        static const int REQ_SEND_SOCKET_ID;
        static const int REQ_RECV_SOCKET_ID;
        int REQ_TIMER_WATCHDOG_ID, REQ_TIMER_KEEPALIVE_ID;
        int REQ_DNS_LOOKUP_ID;


        const int DEFAULT_RTSP_PORT, DEFAULT_HTTP_PORT;

        //these three are in milliseconds
        const int TIMEOUT_CONNECT_AND_DNS_LOOKUP, TIMEOUT_SEND, TIMEOUT_RECV;
        const int TIMEOUT_SHUTDOWN;

        //these two are in seconds
        int TIMEOUT_WATCHDOG;
        const int TIMEOUT_WATCHDOG_TEARDOWN;
        int TIMEOUT_KEEPALIVE;
        const int RECOMMENDED_RTP_BLOCK_SIZE;

        int setupTrackIndex;
        bool bRepositioning;

        class PVRTSPErrorContext
        {
            public:
                SocketEvent		iErrSockEvent;
                PVRTSPEngineState	iErrState;
        };


        //temp string compose buffer for internal use RTSP_MAX_FULL_REQUEST_SIZE
        OsclMemoryFragment	iRTSPEngTmpBuf;
        OsclMemoryFragment	iEntityMemFrag;
        //OsclRefCounterMemFrag	iEntityMemFrag;

        // Queue of commands for cancel
        PVRTSPEngineNodeCmdQ iCancelCmdQueue;
        // Queue of commands user requested
        PVRTSPEngineNodeCmdQ iPendingCmdQueue;

        //Queue for cmds which are running.
        //normally this node will not start processing one command
        //until the prior one is finished.  However, a hi priority
        //command such as Cancel must be able to interrupt a command
        //in progress.
        PVRTSPEngineNodeCmdQ iRunningCmdQueue;
        PVMFPortVector<PVMFRTSPPort, PVRTSPEngineNodeAllocator> iPortVector;
        /**
         * Queue holding port activity. Only incoming and outgoing msg activity are
         * put on the queue.  For each port, there should only be at most one activity
         * of each type on the queue.
         */
        Oscl_Vector<PVMFPortActivity, PVRTSPEngineNodeAllocator> iPortActivityQueue;
        PVMFNodeCapability iCapability;

        //OsclPriorityQueue<PVRTSPEngineCommand,PVRTSPEngineNodeAllocator,Oscl_Vector<PVRTSPEngineCommand,PVRTSPEngineNodeAllocator>,PVRTSPEngineCommandCompareLess> iPendingCmdQueue;
//	Oscl_Vector<PVRTSPEngineAsyncEvent, PVRTSPEngineNodeAllocator> iPendingEvents;

//	OsclPriorityQueue<RTSPIncomingMessage,PVRTSPEngineNodeAllocator,Oscl_Vector<RTSPIncomingMessage,PVRTSPEngineNodeAllocator>,PVRTSPGenericMessageCompareLess> iIncomingMsgQueue;
        OsclPriorityQueue<RTSPOutgoingMessage*, PVRTSPEngineNodeAllocator, Oscl_Vector<RTSPOutgoingMessage*, PVRTSPEngineNodeAllocator>, PVRTSPGenericMessageCompareLess> iOutgoingMsgQueue;
        RTSPOutgoingMessage* iSrvResponse;
        bool	bSrvRespPending;

        OsclTimer<PVRTSPEngineNodeAllocator>	*iWatchdogTimer;

        int32 iCurrentErrorCode;
        PVUuid iEventUUID;

        bool	bKeepAliveInPlay;
        RTSPMethod	iKeepAliveMethod;

        bool	bAddXStrHeader;

        OsclMemPoolResizableAllocator *iMediaDataResizableAlloc;
        PVMFSimpleMediaBufferCombinedAlloc *iMediaDataImplAlloc;

        /* Round trip delay calculation */
        PVMFTimebase_Tickcount iRoundTripClockTimeBase;

        int32	iErrorRecoveryAttempt;

        //uint8	iGetPostCorrelation;
        GetPostCorrelationObject *iGetPostCorrelationObject;
    private:
        PVRTSPEngineNode();
        PVRTSPEngineNode&  operator = (const PVRTSPEngineNode&);
        PVRTSPEngineNode(const PVRTSPEngineNode&);

        //OsclActiveObject
        virtual void Run();
        virtual OsclLeaveCode RunError(OsclLeaveCode aError);

        PVMFStatus sendSocketOutgoingMsg(SocketContainer &aSock, RTSPOutgoingMessage &aMsg);
        PVMFStatus sendSocketOutgoingMsg(SocketContainer &aSock, const uint8* aSendBuf, uint32 aSendLen);

        void ChangeExternalState(TPVMFNodeInterfaceState aNewState);

        // Handle command and data events
        PVMFCommandId AddCmdToQueue(PVRTSPEngineCommand& aCmd);

        bool ProcessCommand(PVRTSPEngineCommand& aCmd);
        bool rtspParserLoop(void);

        PVMFStatus DispatchCommand(PVRTSPEngineCommand& aCmd);
        void MoveCmdToCancelQueue(PVRTSPEngineCommand& aCmd);

        PVMFStatus DoInitNode(PVRTSPEngineCommand &aCmd);
        PVMFStatus DoPrepareNode(PVRTSPEngineCommand &aCmd);
        PVMFStatus DoStartNode(PVRTSPEngineCommand &aCmd);
        PVMFStatus DoPauseNode(PVRTSPEngineCommand &aCmd);
        PVMFStatus DoStopNode(PVRTSPEngineCommand &aCmd);
        PVMFStatus DoResetNode(PVRTSPEngineCommand &aCmd);
        PVMFStatus DoQueryUuid(PVRTSPEngineCommand &aCmd);
        PVMFStatus DoQueryInterface(PVRTSPEngineCommand &aCmd);
        PVMFStatus DoCancelCommand(PVRTSPEngineCommand &aCmd);
        PVMFStatus DoCancelAllCommands(PVRTSPEngineCommand &aCmd);
        PVMFStatus DoFlush(PVRTSPEngineCommand &aCmd);

        PVMFStatus DoErrorRecovery(PVRTSPEngineCommand &aCmd);

        PVMFStatus DoRequestPort(PVRTSPEngineCommand &aCmd, PVMFRTSPPort* &aPort);
        PVMFStatus DoAddPort(int32 id, bool isMedia, int32 tag, PVMFRTSPPort* &aPort);
        PVMFStatus DoReleasePort(PVRTSPEngineCommand &aCmd);

        bool FlushPending();
        bool ProcessPortActivity();
        void QueuePortActivity(const PVMFPortActivity &aActivity);
        PVMFStatus ProcessOutgoingMsg(PVMFPortInterface* aPort);

        PVMFStatus SendRtspDescribe(PVRTSPEngineCommand &aCmd);
        PVMFStatus SendRtspSetup(PVRTSPEngineCommand &aCmd);
        PVMFStatus SendRtspPlay(PVRTSPEngineCommand &aCmd);
        PVMFStatus SendRtspPause(PVRTSPEngineCommand &aCmd);
        PVMFStatus SendRtspTeardown(PVRTSPEngineCommand &aCmd);

        void CommandComplete(PVRTSPEngineNodeCmdQ&,
                             PVRTSPEngineCommand&,
                             PVMFStatus, OsclAny* aData = NULL,
                             PVUuid* aEventUUID = NULL,
                             int32* aEventCode = NULL);

        bool parseURL(const OSCL_wString& aURL);
        bool parseURL(const char* aURL);


        void ChangeInternalState(PVRTSPEngineState aNewTask);
        PVMFStatus composeOptionsRequest(RTSPOutgoingMessage&);
        PVMFStatus composeDescribeRequest(RTSPOutgoingMessage&);
        PVMFStatus composeSetupRequest(RTSPOutgoingMessage &iMsg, StreamInfo &aSelected);
        PVMFStatus composePlayRequest(RTSPOutgoingMessage &iMsg);
        PVMFStatus composeStopRequest(RTSPOutgoingMessage &iMsg);
        PVMFStatus composePauseRequest(RTSPOutgoingMessage &iMsg);
        PVMFStatus composeKeepAliveRequest(RTSPOutgoingMessage &aMsg);

        PVMFStatus composeGetRequest(RTSPOutgoingMessage &iMsg);
        PVMFStatus composePostRequest(RTSPOutgoingMessage &iMsg);

        PVMFStatus processIncomingMessage(RTSPIncomingMessage &iIncomingMsg);
        PVMFStatus processServerRequest(RTSPIncomingMessage &aMsg);
        PVMFStatus processEntityBody(RTSPIncomingMessage &aMsg, OsclMemoryFragment &aEntityMemFrag);

        PVMFStatus processCommonResponse(RTSPIncomingMessage &aMsg);

        //should merge togather and move to jitter buffer
        PVMFStatus composeSessionURL(RTSPOutgoingMessage &aMsg);
        PVMFStatus composeMediaURL(int aTrackID, StrPtrLen &aMediaURI);

        //PVMFStatus processSDP(OsclMemoryFragment &aSDPBuf, SDPInfo &aSDPinfo);

        void ReportErrorEvent(PVMFEventType aEventType,
                              OsclAny* aEventData = NULL,
                              PVUuid* aEventUUID = NULL,
                              int32* aEventCode = NULL);
        void ReportInfoEvent(PVMFEventType aEventType,
                             OsclAny* aEventData = NULL,
                             PVUuid* aEventUUID = NULL,
                             int32* aEventCode = NULL);

        void MapRTSPCodeToEventCode(RTSPStatusCode aStatusCode,
                                    int32& aEventCode);
        //allocate aReqBufSize memory for iEmbeddedData
        bool PrepareEmbeddedDataMemory(uint32 aReqBufSize, OsclMemoryFragment &);
        bool DispatchEmbeddedData(uint32 aChannelID);
        // private members added for real support
        bool ibIsRealRDT;

        // realchallenge1 string returned by OPTIONS request
        OSCL_HeapString<OsclMemAllocator> iRealChallenge1;

        // realchallenge2 string to be sent in SETUP request
        OSCL_HeapString<OsclMemAllocator> iRealChallenge2;

        IRealChallengeGen* ipRealChallengeGen;
        IPayloadParser* ipRdtParser;

        // allocator for outgoing media frag groups
        PVMFMediaFragGroupCombinedAlloc<OsclMemAllocator>* ipFragGroupAllocator;
        OsclMemPoolFixedChunkAllocator* ipFragGroupMemPool;

        /////


        bool DispatchEmbeddedRdtData();
        bool ibBlockedOnFragGroups;
        //bool simpleHttpParser(const uint8 *aBuf, int32 &aLen, bool &aIsStatus200);

        PVMFStatus resetSocket(bool aImmediate = false);
        void clearOutgoingMsgQueue(void);
        void partialResetSessionInfo(void);

        bool clearEventQueue(void);

        void ResetSessionInfo(void);
        PVRTSPEngineNodeExtensionInterface* iExtensionInterface;
};

#endif //PVRTSP_CLIENT_ENGINE_NODE_H
