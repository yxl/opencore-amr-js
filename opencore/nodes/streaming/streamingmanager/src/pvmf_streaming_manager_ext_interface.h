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
#ifndef PVMF_STREAMING_MANAGER_EXT_INTERFACE_H_INCLUDED
#define PVMF_STREAMING_MANAGER_EXT_INTERFACE_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef PVMF_MEDIA_CLOCK_H_INCLUDED
#include "pvmf_media_clock.h"
#endif
#ifndef OSCL_STRING_CONTAINERS_H_INCLUDED
#include "oscl_string_containers.h"
#endif
#ifndef PV_UUID_H_INCLUDED
#include "pv_uuid.h"
#endif
#ifndef PV_INTERFACE_H
#include "pv_interface.h"
#endif
#ifndef PVMF_STREAMING_MANAGER_NODE_H_INCLUDED
#include "pvmf_streaming_manager_node.h"
#endif
#ifndef PAYLOADPARSER_REGISTRY_H
#include "payload_parser_registry.h"
#endif
#ifndef SDP_INFO_H_INCLUDED
#include "sdp_info.h"
#endif

typedef struct tagPVMFSMClientParams
{
    OSCL_wHeapString<PVMFStreamingManagerNodeAllocator> _userAgent;
    OSCL_wHeapString<PVMFStreamingManagerNodeAllocator> _userNetwork;
    OSCL_wHeapString<PVMFStreamingManagerNodeAllocator> _deviceInfo;
} PVMFSMClientParams;

typedef struct tagPVMFSMTimeParams
{
    uint32 _connectTimeOut;
    uint32 _sendTimeOut;
    uint32 _recvTimeOut;
    uint32 _rtpTimeOut;
    uint32 _keepAliveInterval;
} PVMFSMTimeParams;

typedef struct tagPVMFSMAuthenticationParmas
{
    OSCL_wHeapString<PVMFStreamingManagerNodeAllocator> _userID;
    OSCL_wHeapString<PVMFStreamingManagerNodeAllocator> _authenticationInfo;
    OSCL_wHeapString<PVMFStreamingManagerNodeAllocator> _expirationInfo;
    OSCL_wHeapString<PVMFStreamingManagerNodeAllocator> _applicationSpecificString;
    OSCL_wHeapString<PVMFStreamingManagerNodeAllocator> _verificationInfo;
    OSCL_wHeapString<PVMFStreamingManagerNodeAllocator> _signatureInfo;
} PVMFSMAuthenticationParmas;

typedef struct tagPVMFSMJitterBufferParmas
{
    uint32 _bufferDurationInMilliSeconds;
    uint32 _playBackThresholdInMilliSeconds;
} PVMFSMJitterBufferParmas;

class PVMFStreamingManagerExtensionInterface : public PVInterface
{
    public:
        // Streaming manager config APIs
        OSCL_IMPORT_REF virtual PVMFStatus setClientParameters(PVMFSMClientParams* clientParams) = 0;
        OSCL_IMPORT_REF virtual PVMFStatus setTimeParams(PVMFSMTimeParams* timeParams) = 0;
        OSCL_IMPORT_REF virtual PVMFStatus setAuthenticationParameters(PVMFSMAuthenticationParmas* authenticationParams) = 0;
        OSCL_IMPORT_REF virtual PVMFStatus setJitterBufferParams(PVMFSMJitterBufferParmas* jitterBufferParams) = 0;
        OSCL_IMPORT_REF virtual PVMFStatus resetJitterBuffer() = 0;
        OSCL_IMPORT_REF virtual PVMFStatus setPayloadParserRegistry(PayloadParserRegistry*) = 0;

        OSCL_IMPORT_REF virtual PVMFStatus setDataPortLogging(bool logEnable, OSCL_String* logPath = NULL) = 0;
        OSCL_IMPORT_REF virtual PVMFStatus switchStreams(uint32 aSrcStreamID, uint32 aDestStreamID) = 0;

        OSCL_IMPORT_REF virtual void addRef() = 0;
        OSCL_IMPORT_REF virtual void removeRef() = 0;
        OSCL_IMPORT_REF virtual bool queryInterface(const PVUuid& uuid, PVInterface*& iface) = 0;

};

//Mimetype and Uuid for the extension interface
#define PVMF_STREAMINGMANAGER_CUSTOMINTERFACE_MIMETYPE "pvxxx/PVMFStreamingManagerNode/CustomInterface"
#define PVMF_STREAMINGMANAGER_MIMETYPE "pvxxx/PVMFStreamingManagerNode"
#define PVMF_STREAMINGMANAGER_BASEMIMETYPE "pvxxx"
#define PVMF_STREAMINGMANAGERNODE_EXTENSIONINTERFACE_UUID PVUuid(0x0156f5d6,0x6cc7,0x45b3,0x88,0x26,0xf6,0x80,0x1b,0x9c,0x13,0xa7)

#endif


