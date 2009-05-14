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
#ifndef PVMF_SM_NODE_EVENTS_H_INCLUDED
#define PVMF_SM_NODE_EVENTS_H_INCLUDED

#define PVMFStreamingManagerNodeEventTypeUUID PVUuid(0xf91ab192,0xdcac,0x4717,0x82,0x3b,0x4d,0x62,0x6e,0x71,0x0b,0x15)
/**
 * An enumeration of error codes from PVMFStreamingManagerNode
 **/
typedef enum
{
    PVMFStreamingManagerNodeErrorEventStart = 1024,
    PVMFStreamingManagerNodeErrorInvalidRequestPortTag,
    PVMFStreamingManagerNodeErrorInvalidPort,
    PVMFStreamingManagerNodeErrorParseSDPFailed,
    PVMFStreamingManagerNodeGraphConnectFailed,
    PVMFStreamingManagerNodeGraphDisconnectFailed,
    PVMFStreamingManagerNodeGetPortConfigFromHTTPNodeFailed,
    PVMFStreamingManagerNodeErrorEventEnd
} PVMFStreamingManagerNodeErrorEventType;

/**
 * An enumeration of info codes from PVMFStreamingManagerNode
 **/
typedef enum
{
    PVMFStreamingManagerNodeInfoEventStart = 8192,
    PVMFStreamingManagerNodeInfoLaunchURL,
    PVMFStreamingManagerNodeInfoEventEnd
} PVMFStreamingManagerNodeInfoEventType;

#define PVMFJitterBufferNodeEventTypeUUID PVUuid(0xf2efcd33,0x6a77,0x4860,0x8b,0x9f,0xb6,0x24,0xbd,0x90,0x5f,0x2e)
/**
 * An enumeration of error codes from PVMFJitterBufferNode
 **/
typedef enum
{
    PVMFJitterBufferNodeErrorEventStart = 1024,
    PVMFJitterBufferNodeRemoteInactivityTimerExpired,
    PVMFJitterBufferNodeRTCPRRGenerationFailed,
    PVMFJitterBufferNodeUnableToRegisterIncomingPacket,
    PVMFJitterBufferNodeInputDataPacketHeaderParserError,
    PVMFJitterBufferNodeRTCPSRProcFailed,
    PVMFJitterBufferNodeFirewallPacketGenerationFailed,
    PVMFJitterBufferNodeCorruptRTPPacket,
    PVMFJitterBufferNodeFirewallPacketExchangeFailed,
    PVMFJitterBufferNodeErrorEventEnd
} PVMFJitterBufferNodeErrorEventType;

#define PVMFMediaLayerNodeEventTypeUUID PVUuid(0xaf589e87,0xa76b,0x406d,0xac,0xf7,0x9d,0x79,0xda,0x4e,0x78,0xde)
/**
 * An enumeration of error codes from PVMFMediaLayerNode
 **/
typedef enum
{
    PVMFMediaLayerNodeErrorEventStart = 1024,
    PVMFMediaLayerNodePayloadParserError,
    PVMFMediaLayerNodeErrorEventEnd
} PVMFMediaLayerNodeErrorEventType;

/**
 * An enumeration of info codes from PVMFMediaLayerNode
 **/
typedef enum
{
    PVMFMediaLayerNodeInfoEventStart = 8192,
    PVMFMediaLayerNodeExcercisingPortFlowControl,
    PVMFMediaLayerNodeWaitingForBuffersToParserPayload,
    PVMFMediaLayerNodeInfoEventEnd
} PVMFMediaLayerNodeInfoEventType;

#define PVMFRTSPClientEngineNodeEventTypeUUID PVUuid(0x56b93e98,0x08fb,0x48de,0x8d,0x76,0xf8,0xde,0xc7,0xf8,0xb1,0xb1)

/**
 * An enumeration of error codes from PVMFRTSPClientEngineNode
 **/
typedef enum
{
    PVMFRTSPClientEngineNodeErrorEventStart = 1024,
    PVMFRTSPClientEngineNodeErrorSocketServerError,
    PVMFRTSPClientEngineNodeErrorDNSLookUpError,
    PVMFRTSPClientEngineNodeErrorRTSPSocketCreateError,
    PVMFRTSPClientEngineNodeErrorRTSPSocketConnectError,
    PVMFRTSPClientEngineNodeErrorSocketSendError,
    PVMFRTSPClientEngineNodeErrorSocketRecvError,
    PVMFRTSPClientEngineNodeErrorSocketError,
    PVMFRTSPClientEngineNodeErrorOutOfMemory,
    PVMFRTSPClientEngineNodeErrorRTSPRequestTooBig,
    PVMFRTSPClientEngineNodeErrorRTSPParserError,
    PVMFRTSPClientEngineNodeErrorUnknownRTSPMessage,
    PVMFRTSPClientEngineNodeErrorIncorrectRTSPMessageType,
    PVMFRTSPClientEngineNodeErrorMalformedRTSPMessage,
    PVMFRTSPClientEngineNodeErrorMissingSeqNumInServerResponse,
    PVMFRTSPClientEngineNodeErrorMissingSessionIdInServerResponse,
    PVMFRTSPClientEngineNodeErrorRTSPRequestResponseMismatch,
    PVMFRTSPClientEngineNodeErrorRTSPComposeOptionsRequestError,
    PVMFRTSPClientEngineNodeErrorRTSPComposeDescribeRequestError,
    PVMFRTSPClientEngineNodeErrorRTSPComposeSetupRequestError,
    PVMFRTSPClientEngineNodeErrorRTSPComposePlayRequestError,
    PVMFRTSPClientEngineNodeErrorRTSPComposePauseRequestError,
    PVMFRTSPClientEngineNodeErrorRTSPComposeStopRequestError,
    PVMFRTSPClientEngineNodeErrorRTSPCompose501ResponseError,
    //"400"      ; Bad Request
    PVMFRTSPClientEngineNodeErrorRTSPErrorCode400,
    //"401"      ; Unauthorized
    PVMFRTSPClientEngineNodeErrorRTSPErrorCode401,
    //"402"      ; Payment Required
    PVMFRTSPClientEngineNodeErrorRTSPErrorCode402,
    //"403"      ; Forbidden
    PVMFRTSPClientEngineNodeErrorRTSPErrorCode403,
    //"404"      ; Not Found
    PVMFRTSPClientEngineNodeErrorRTSPErrorCode404,
    //"405"      ; Method Not Allowed
    PVMFRTSPClientEngineNodeErrorRTSPErrorCode405,
    //"406"      ; Not Acceptable
    PVMFRTSPClientEngineNodeErrorRTSPErrorCode406,
    //"407"      ; Proxy Authentication Required
    PVMFRTSPClientEngineNodeErrorRTSPErrorCode407,
    //"408"      ; Request Time-out
    PVMFRTSPClientEngineNodeErrorRTSPErrorCode408,
    //"410"      ; Gone
    PVMFRTSPClientEngineNodeErrorRTSPErrorCode410,
    //"411"      ; Length Required
    PVMFRTSPClientEngineNodeErrorRTSPErrorCode411,
    //"412"      ; Precondition Failed
    PVMFRTSPClientEngineNodeErrorRTSPErrorCode412,
    //"413"      ; Request Entity Too Large
    PVMFRTSPClientEngineNodeErrorRTSPErrorCode413,
    //"414"      ; Request-URI Too Large
    PVMFRTSPClientEngineNodeErrorRTSPErrorCode414,
    //"415"      ; Unsupported Media Type
    PVMFRTSPClientEngineNodeErrorRTSPErrorCode415,
    //"451"      ; Parameter Not Understood
    PVMFRTSPClientEngineNodeErrorRTSPErrorCode451,
    //"452"      ; Conference Not Found
    PVMFRTSPClientEngineNodeErrorRTSPErrorCode452,
    //"453"      ; Not Enough Bandwidth
    PVMFRTSPClientEngineNodeErrorRTSPErrorCode453,
    //"454"      ; Session Not Found
    PVMFRTSPClientEngineNodeErrorRTSPErrorCode454,
    //"455"      ; Method Not Valid in This State
    PVMFRTSPClientEngineNodeErrorRTSPErrorCode455,
    //"456"      ; Header Field Not Valid for Resource
    PVMFRTSPClientEngineNodeErrorRTSPErrorCode456,
    //"457"      ; Invalid Range
    PVMFRTSPClientEngineNodeErrorRTSPErrorCode457,
    //"458"      ; Parameter Is Read-Only
    PVMFRTSPClientEngineNodeErrorRTSPErrorCode458,
    //"459"      ; Aggregate operation not allowed
    PVMFRTSPClientEngineNodeErrorRTSPErrorCode459,
    //"460"      ; Only aggregate operation allowed
    PVMFRTSPClientEngineNodeErrorRTSPErrorCode460,
    //"461"      ; Unsupported transport
    PVMFRTSPClientEngineNodeErrorRTSPErrorCode461,
    //"462"      ; Destination unreachable
    PVMFRTSPClientEngineNodeErrorRTSPErrorCode462,
    //"500"      ; Internal Server Error
    PVMFRTSPClientEngineNodeErrorRTSPErrorCode500,
    //"501"      ; Not Implemented
    PVMFRTSPClientEngineNodeErrorRTSPErrorCode501,
    //"502"      ; Bad Gateway
    PVMFRTSPClientEngineNodeErrorRTSPErrorCode502,
    //"503"      ; Service Unavailable
    PVMFRTSPClientEngineNodeErrorRTSPErrorCode503,
    //"504"      ; Gateway Time-out
    PVMFRTSPClientEngineNodeErrorRTSPErrorCode504,
    //"505"      ; RTSP Version not supported
    PVMFRTSPClientEngineNodeErrorRTSPErrorCode505,
    //"551"      ; Option not supported
    PVMFRTSPClientEngineNodeErrorRTSPErrorCode551,
    // Unknown
    PVMFRTSPClientEngineNodeErrorRTSPCodeUnknown,
    PVMFRTSPClientEngineNodeErrorEventEnd
} PVMFRTSPClientEngineNodeErrorEventType;

/**
 * An enumeration of info codes from PVMFRTSPClientEngineNode
 **/
typedef enum
{
    PVMFRTSPClientEngineNodeInfoEventStart = 8192,
    PVMFRTSPClientEngineNodeInfoRedirect,
    //"300"      ; Multiple Choices
    PVMRTSPClientEngineInfoRTSPRedirectCode300,
    //"301"      ; Moved Permanently
    PVMRTSPClientEngineInfoRTSPRedirectCode301,
    //"302"      ; Moved Temporarily
    PVMRTSPClientEngineInfoRTSPRedirectCode302,
    //"303"      ; See Other
    PVMRTSPClientEngineInfoRTSPRedirectCode303,
    //"304"      ; Not Modified
    PVMRTSPClientEngineInfoRTSPRedirectCode304,
    //"305"      ; Use Proxy
    PVMRTSPClientEngineInfoRTSPRedirectCode305,
    PVMFRTSPClientEngineNodeInfoEOS,
    PVMFRTSPClientEngineNodeInfoEventEnd
} PVMFRTSPClientEngineNodeInfoEventType;

typedef struct
{
    uint32 iPlaylistUrlLen;
    char *iPlaylistUrlPtr;
    uint32 iPlaylistIndex;
    uint32 iPlaylistOffsetSec;
    uint32 iPlaylistOffsetMillsec;

    uint32 iPlaylistNPTSec;
    uint32 iPlaylistNPTMillsec;

    //max 256
    uint32	iPlaylistMediaNameLen;
    //char	*iPlaylistMediaNamePtr;
    char iPlaylistMediaNamePtr[256];

    //max 512
    uint32	iPlaylistUserDataLen;
    //char	*iPlaylistUserDataPtr;
    char iPlaylistUserDataPtr[512];
}PVMFRTSPClientEngineNodePlaylistInfoType;
#endif


