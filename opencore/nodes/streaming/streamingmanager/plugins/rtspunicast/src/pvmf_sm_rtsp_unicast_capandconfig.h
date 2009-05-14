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
#ifndef PVMF_SM_RTSP_UNICAST_CAPANDCONFIG_H
#define PVMF_SM_RTSP_UNICAST_CAPANDCONFIG_H
///////////////////////////////////////////////////////////////////////////////
//
// Capability and config interface related constants and definitions
//   - based on pv_player_engine.h
//
///////////////////////////////////////////////////////////////////////////////
struct StreamingManagerKeyStringData
{
    char iString[64];
    PvmiKvpType iType;
    PvmiKvpValueType iValueType;
};
// The number of characters to allocate for the key string
#define SMCONFIG_KEYSTRING_SIZE 128

static const StreamingManagerKeyStringData StreamingManagerConfig_BaseKeys[] =
{
    {"delay", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_UINT32},
    {"jitterBufferNumResize", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_UINT32},
    {"jitterBufferResizeSize", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_UINT32},
    {"jitterbuffer-inactivity-duration", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_UINT32},
    {"user-agent", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_WCHARPTR},
    {"keep-alive-interval", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_UINT32},
    {"keep-alive-during-play", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_BOOL},
    {"rtsp-timeout", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_UINT32},
    {"rebuffering-threshold", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_UINT32},
    {"disable-firewall-packets", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_BOOL}
};

static const uint StreamingManagerConfig_NumBaseKeys =
    (sizeof(StreamingManagerConfig_BaseKeys) /
     sizeof(StreamingManagerKeyStringData));

enum BaseKeys_IndexMapType
{
    BASEKEY_DELAY = 0,
    BASEKEY_JITTERBUFFER_NUMRESIZE,
    BASEKEY_JITTERBUFFER_RESIZESIZE,
    BASEKEY_JITTERBUFFER_MAX_INACTIVITY_DURATION,
    BASEKEY_SESSION_CONTROLLER_USER_AGENT,
    BASEKEY_SESSION_CONTROLLER_KEEP_ALIVE_INTERVAL,
    BASEKEY_SESSION_CONTROLLER_KEEP_ALIVE_DURING_PLAY,
    BASEKEY_SESSION_CONTROLLER_RTSP_TIMEOUT,
    BASEKEY_REBUFFERING_THRESHOLD,
    BASEKEY_DISABLE_FIREWALL_PACKETS
};

typedef struct tagPVMFSMClientParams
{
    OSCL_wHeapString<OsclMemAllocator> _userAgent;
    OSCL_wHeapString<OsclMemAllocator> _userNetwork;
    OSCL_wHeapString<OsclMemAllocator> _deviceInfo;
} PVMFSMClientParams;

#endif
