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
 *  @file pvmf_media_msg_format_ids.h
 *  @brief This file defines the PV Multimedia Framework (PVMF) media message
 *  format ids which is used for in-band control/command messages sent over
 *  the normal media data path. All media data msgs have an id of zero and each
 *  PVMF Media Command has an unique ID
 */

#ifndef PVMF_MEDIA_MSG_FORMAT_IDS_H_INCLUDED
#define PVMF_MEDIA_MSG_FORMAT_IDS_H_INCLUDED

#ifndef PV_UUID_H_INCLUDED
#include "pv_uuid.h"
#endif

const PVUid32 PVMF_MEDIA_MSG_FORMAT_UNDEFINED = 0xFFFFFFFF;

/* zero through 255 is reserved for media data */
const PVUid32 PVMF_MEDIA_MSG_DATA_FORMAT_ID = 0x00000000;

/* common commands start from 256 and go upto 511 */
const PVUid32 PVMF_MEDIA_CMD_FORMAT_IDS_START = 0x00000100;

const PVUid32 PVMF_MEDIA_CMD_EOS_FORMAT_ID  = 0x00000101;

const PVUid32 PVMF_MEDIA_CMD_RE_CONFIG_FORMAT_ID = 0x00000102;

/* add new command ids here */
const PVUid32 PVMF_MEDIA_CMD_BOS_FORMAT_ID  = 0x00000103;

/* format id for no-op messages */
const PVUid32 PVMF_MEDIA_CMD_NOOP_FORMAT_ID  = 0x00000104;

const PVUid32 PVMF_MEDIA_CMD_FORMAT_IDS_END = 0x00000200;

/* command ids above 512 are reserved for node specific commands */
#endif

