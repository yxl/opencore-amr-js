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
#ifndef PVRTSP_ENGINE_NODE_EXTENSION_INTERFACE_H_INCLUDED
#define PVRTSP_ENGINE_NODE_EXTENSION_INTERFACE_H_INCLUDED

#ifndef PV_INTERFACE_H
#include "pv_interface.h"
#endif

#ifndef PVMF_RETURN_CODES_H_INCLUDED
#include "pvmf_return_codes.h"
#endif

#ifndef OSCL_STRING_H_INCLUDED
#include "oscl_string.h"
#endif

#ifndef OSCL_SOCKET_H_INCLUDED
#include "oscl_socket.h"
#endif

#ifndef PVMF_STREAMING_REAL_INTERFACES_INCLUDED
#include "pvmf_streaming_real_interfaces.h"
#endif

#ifndef PAYLOAD_PARSER_H_INCLUDED
#include "payload_parser.h"
#endif

#ifndef PVMF_EVENT_HANDLING_H_INCLUDED
#include "pvmf_event_handling.h"
#endif

#ifndef PVMF_SM_CONFIG_H_INCLUDED
#include "pvmf_sm_config.h"
#endif

// UUID for the extension interface
#define KPVRTSPEngineNodeExtensionUuid  PVUuid(0x00000000,0x0000,0x4887,0xB1,0x0B,0x7E,0xE6,0x48,0x47,0xB2,0x05)

//Mimetypes for the custom interface
#define PVMF_RTSPENGINENODE_CUSTOM1_MIMETYPE "pvmf/RTSPEngineNode/Custom1"
#define PVMF_RTSPENGINENODE_MIMETYPE "pvmf/RTSPEngineNode"
#define PVMF_RTSPENGINENODE_BASEMIMETYPE "pvmf"

class SDPInfo;

struct RtspRangeType;

/** Enumerated list of port tags supported by this port */
typedef enum
{
    PVMF_RTSP_NODE_PORT_TYPE_INPUT,
    PVMF_RTSP_NODE_PORT_TYPE_OUTPUT,
    PVMF_RTSP_NODE_PORT_TYPE_INPUT_OUTPUT
} PVMFRTSPNodePortTag;

enum PVRTSPStreamingType
{
    PVRTSP_3GPP_UDP,
    PVRTSP_3GPP_TCP,
    PVRTSP_RM_HTTP,//Real http cloaking
    PVRTSP_MS_UDP,
    PVRTSP_MS_TCP
};


struct StreamInfo
{
    int32	iSDPStreamId;

    bool    ssrcIsSet;
    uint32	iSSRC;

    uint32	iCliRTCPPort;
    uint32	iCliRTPPort;

    //As a default, RTCP packets are sent
    //on the first available channel higher than the RTP channel.
    //for UDP as transport, they are just the UDP port number
    //for TCP as transport, they are the interleaved channel ID
    uint32	iSerRTPPort;
    uint32	iSerRTCPPort;

    bool	rtptimeIsSet;
    uint32	rtptime;

    bool	seqIsSet;
    uint32	seq;

    bool	b3gppAdaptationIsSet;
    uint32	iBufSize;	//bytes
    uint32	iTargetTime;	//ms

    OsclNameString<PVNETWORKADDRESS_LEN>	iSerIpAddr;

    OSCL_HeapString<OsclMemAllocator>	iMediaURI;
};

class PVRTSPEngineNodeServerInfo
{
    public:
        PVRTSPEngineNodeServerInfo()
        {
            iIsPVServer = false;
            iRoundTripDelayInMS = 0;
            iServerVersionNumber = 0;
        }

        virtual ~PVRTSPEngineNodeServerInfo()
        {
        };

        bool iIsPVServer;
        uint32 iRoundTripDelayInMS;
        OSCL_HeapString<OsclMemAllocator> iServerName;
        uint32 	iServerVersionNumber;
};

#define PVRTSPENGINENODE_DEFAULT_KEEP_ALIVE_INTERVAL 55
#define PVRTSPENGINENODE_DEFAULT_RTSP_INACTIVITY_TIMER 15
#define PVRTSPENGINENODE_DEFAULT_NUMBER_OF_REDIRECT_TRIALS 1

//Forward Declarations
class PVMFPortInterface;

class PVRTSPEngineNodeExtensionInterface : public PVInterface
{
    public:
        virtual ~PVRTSPEngineNodeExtensionInterface() {}
        //**********begin PVInterface
        OSCL_IMPORT_REF virtual void addRef() = 0;
        OSCL_IMPORT_REF virtual void removeRef() = 0;
        OSCL_IMPORT_REF virtual bool queryInterface(const PVUuid& uuid, PVInterface*& iface) = 0;
        //**********end PVInterface

        OSCL_IMPORT_REF virtual PVMFStatus SetStreamingType(PVRTSPStreamingType aType = PVRTSP_3GPP_UDP) = 0;

        //Either SetSessionURL() or  SetSDPInfo() must be called before Init()
        OSCL_IMPORT_REF virtual PVMFStatus SetSessionURL(OSCL_wString& aURL) = 0;
        OSCL_IMPORT_REF virtual PVMFStatus GetSDP(OsclRefCounterMemFrag& aSDPBuf) = 0;

        OSCL_IMPORT_REF virtual PVMFStatus SetRtspProxy(OSCL_String& aRtspProxyName, uint32 aRtspProxyPort) = 0;
        OSCL_IMPORT_REF virtual PVMFStatus GetRtspProxy(OSCL_String& aRtspProxyName, uint32& aRtspProxyPort) = 0;

        // to be called before init
        OSCL_IMPORT_REF virtual bool IsRdtTransport() = 0;

        OSCL_IMPORT_REF virtual void SetPortRdtStreamId(PVMFPortInterface* pPort,
                int iRdtStreamId) = 0;

        OSCL_IMPORT_REF virtual PVMFStatus SetSDPInfo(OsclSharedPtr<SDPInfo>& aSDPinfo, Oscl_Vector<StreamInfo, OsclMemAllocator> &aSelectedStream) = 0;
        OSCL_IMPORT_REF virtual PVMFStatus GetServerInfo(PVRTSPEngineNodeServerInfo& aServerInfo) = 0;
        OSCL_IMPORT_REF virtual PVMFStatus GetStreamInfo(Oscl_Vector<StreamInfo, OsclMemAllocator> &aSelectedStream) = 0;


        // API to pass in Real related parameters
        OSCL_IMPORT_REF virtual void SetRealChallengeCalculator(IRealChallengeGen* pChallengeCalc) = 0;
        OSCL_IMPORT_REF virtual void SetRdtParser(IPayloadParser* pRdtParser) = 0;

        //One of these must be called before Start()
        OSCL_IMPORT_REF virtual PVMFStatus SetRequestPlayRange(const RtspRangeType& aRange) = 0;
        OSCL_IMPORT_REF virtual PVMFStatus GetActualPlayRange(RtspRangeType& aRange) = 0;


        OSCL_IMPORT_REF virtual PVMFStatus GetUserAgent(OSCL_wString& aUserAgent) = 0;

        OSCL_IMPORT_REF virtual PVMFStatus SetClientParameters(OSCL_wString& aUserAgent,
                OSCL_wString&  aUserNetwork,
                OSCL_wString&  aDeviceInfo) = 0;

        OSCL_IMPORT_REF virtual PVMFStatus SetAuthenticationParameters(OSCL_wString& aUserID,
                OSCL_wString& aAuthentication,
                OSCL_wString& aExpiration,
                OSCL_wString& aApplicationSpecificString,
                OSCL_wString& aVerification,
                OSCL_wString& aSignature) = 0;
        //OSCL_IMPORT_REF virtual PVMFStatus SetTimeout(uint32 aTimeout)=0;
        //OSCL_IMPORT_REF virtual PVMFStatus GetTimeout(uint32& aTimeout)=0;

        /**
         * This API is to set the keep-alive mechanism for rtsp client.
         *
         * @param aTimeout The interval in milliseconds of sending the RTSP keep-alive commands.
         *	0 to use server defined timeout value.
         * @param aUseSetParameter true to use SET_PARAMETER; false to use OPTIONS
         * @param aKeepAliveInPlay Turns on/off the sending RTSP keep-alive commands during PLAY
         * @returns Completion status
         */
        //OSCL_IMPORT_REF virtual PVMFStatus SetKeepAliveMethod(int32 aTimeout=0, bool aUseSetParameter=false, bool aKeepAliveInPlay=false)=0;
        OSCL_IMPORT_REF virtual PVMFStatus SetKeepAliveMethod_timeout(int32 aTimeout = 0) = 0;
        OSCL_IMPORT_REF virtual PVMFStatus SetKeepAliveMethod_use_SET_PARAMETER(bool aUseSetParameter = false) = 0;
        OSCL_IMPORT_REF virtual PVMFStatus SetKeepAliveMethod_keep_alive_in_play(bool aKeepAliveInPlay = false) = 0;

        OSCL_IMPORT_REF virtual PVMFStatus GetKeepAliveMethod(int32 &aTimeout, bool &aUseSetParameter, bool &aKeepAliveInPlay) = 0;

        /**
        * This method is called to enable/disable X-STR http header to be a part of the HTTP GET or POST methods
        *
        * @param aAddXSTRHeader boolean which indicates whether to set the header to ON or OFF
        *
        */

        OSCL_IMPORT_REF virtual PVMFStatus GetRTSPTimeOut(int32 &aTimeout) = 0;
        OSCL_IMPORT_REF virtual PVMFStatus SetRTSPTimeOut(int32 aTimeout) = 0;
        OSCL_IMPORT_REF virtual void UpdateSessionCompletionStatus(bool aSessionCompleted) = 0;
};

#endif //PVRTSP_ENGINE_NODE_EXTENSION_INTERFACE_H_INCLUDED

