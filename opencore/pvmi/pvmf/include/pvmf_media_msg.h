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
 *  @file pvmf_media_msg.h
 *  @brief This file defines the PV Multimedia Framework (PVMF) media message
 *  class which is used as the main interface for all messages sent over the
 *  normal media data path.
 *
 */

#ifndef PVMF_MEDIA_MSG_H_INCLUDED
#define PVMF_MEDIA_MSG_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef PVMF_TIMESTAMP_H_INCLUDED
#include "pvmf_timestamp.h"
#endif

#ifndef PV_INTERFACE_H
#include "pv_interface.h"
#endif

#ifndef OSCL_SHARED_PTR_H_INCLUDED
#include "oscl_shared_ptr.h"
#endif

#ifndef OSCL_REFCOUNTER_MEMFRAG_H_INCLUDED
#include "oscl_refcounter_memfrag.h"
#endif


class PVMFMediaMsg
{


    public:
        virtual ~PVMFMediaMsg() {}
        virtual PVMFTimestamp getTimestamp() = 0;
        virtual uint32 getDuration() = 0;
        virtual uint32 getStreamID() = 0;
        virtual uint32 getSeqNum() = 0;
        virtual PVUid32 getFormatID() = 0;
        virtual bool queryInterface(const PVUuid& uuid, PVInterface*& iface) = 0;
        virtual bool getFormatSpecificInfo(OsclRefCounterMemFrag& memfrag) = 0;
        virtual void setTimestamp(const PVMFTimestamp& ts) = 0;
        virtual void setDuration(const uint32& duration) = 0;
        virtual void setStreamID(uint32 id) = 0;
        virtual void setSeqNum(uint32 seqnum) = 0;

};

typedef OsclSharedPtr<PVMFMediaMsg> PVMFSharedMediaMsgPtr;

#endif
