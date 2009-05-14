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
 *  @file pvmf_media_msg_header.h
 *  @brief This file defines the PV Multimedia Framework (PVMF) media message
 *  header class which is used to hold the basic information such as timestamp,
 *  sequence number, etc for every media message.
 *
 */

#ifndef PVMF_MEDIA_MSG_HEADER_H_INCLUDED
#define PVMF_MEDIA_MSG_HEADER_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef OSCL_REFCOUNTER_MEMFRAG_H_INCLUDED
#include "oscl_refcounter_memfrag.h"
#endif

#ifndef PV_UUID_H_INCLUDED
#include "pv_uuid.h"
#endif

struct PVMFMediaMsgHeader
{
    PVMFMediaMsgHeader(): timestamp(0),
            duration(0),
            seqnum(0),
            stream_id(0),
            format_id(0)
    {};

    PVMFTimestamp timestamp;
    uint32 duration;
    uint32 seqnum;
    uint32 stream_id;
    PVUid32 format_id;
    OsclRefCounterMemFrag format_spec_info;

};


#endif
