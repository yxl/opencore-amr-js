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
/**
 *  @file pvmf_protocol_engine_command_format_ids.h
 *  @brief This file defines command ids which protocol engine node uses to pass the command to socket node, especially for
 *  the command of closing the current socket and opening another socket or reopening the current socket. And this command
 *  is specifically for the protocol engine node and socket node. Other nodes might not understand this one.
 */

#ifndef PVMF_PROTOCOL_ENGINE_COMMAND_FORMAT_IDS_H_INCLUDED
#define PVMF_PROTOCOL_ENGINE_COMMAND_FORMAT_IDS_H_INCLUDED

#ifndef PV_UUID_H_INCLUDED
#include "pv_uuid.h"
#endif

/* add new command ids from 512=0x00000200 */
const PVUid32 PVMF_MEDIA_CMD_SOCKET_CONNECT_FORMAT_ID    = 0x00000200;
const PVUid32 PVMF_MEDIA_CMD_SOCKET_DISCONNECT_FORMAT_ID = 0x00000201;

#endif
