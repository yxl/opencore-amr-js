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
 *  @file pvmf_media_cmd.h
 *  @brief This file defines the PV Multimedia Framework (PVMF) media command
 *  class which is used for in-band control/command messages sent over the
 *  normal media data path. This class is a special case of the PVMFMediaMsg.
 *
 */

#ifndef PVMF_MEDIA_CMD_H_INCLUDED
#define PVMF_MEDIA_CMD_H_INCLUDED


/* include needed for PVMFMediaMsg definition */
#ifndef PVMF_MEDIA_MSG_H_INCLUDED
#include "pvmf_media_msg.h"
#endif

struct PVMFMediaMsgHeader;

class PVMFMediaCmd : public PVMFMediaMsg
{
    public:
        PVMFMediaCmd() : hdr_ptr(NULL) {};

        OSCL_IMPORT_REF virtual ~PVMFMediaCmd();

        OSCL_IMPORT_REF virtual PVMFTimestamp getTimestamp();
        OSCL_IMPORT_REF virtual uint32 getDuration();
        OSCL_IMPORT_REF virtual uint32 getStreamID();
        OSCL_IMPORT_REF virtual uint32 getSeqNum();
        OSCL_IMPORT_REF virtual PVUid32 getFormatID();
        OSCL_IMPORT_REF virtual bool queryInterface(const PVUuid& uuid, PVInterface*& iface);
        OSCL_IMPORT_REF virtual bool getFormatSpecificInfo(OsclRefCounterMemFrag& memfrag);
        OSCL_IMPORT_REF virtual void setTimestamp(const PVMFTimestamp& timestamp);
        OSCL_IMPORT_REF virtual void setDuration(const uint32& duration);
        OSCL_IMPORT_REF virtual void setStreamID(uint32 id);
        OSCL_IMPORT_REF virtual void setSeqNum(uint32 seqnum);

        /* Not from PVMFMediaMsg interface */
        OSCL_IMPORT_REF virtual void setFormatSpecificInfo(OsclRefCounterMemFrag& memfrag);
        OSCL_IMPORT_REF virtual void setFormatID(PVUid32 id);

        OSCL_IMPORT_REF static OsclSharedPtr<PVMFMediaCmd>
        createMediaCmd(Oscl_DefAlloc* gen_alloc = NULL);

        OSCL_IMPORT_REF static OsclSharedPtr<PVMFMediaCmd>
        createMediaCmd(const PVMFMediaMsgHeader* msgHeader,
                       Oscl_DefAlloc* gen_alloc = NULL);

    private:
        PVMFMediaMsgHeader* hdr_ptr;
};

typedef OsclSharedPtr<PVMFMediaCmd> PVMFSharedMediaCmdPtr;

OSCL_IMPORT_REF void convertToPVMFMediaCmdMsg(PVMFSharedMediaMsgPtr& dest,
        PVMFSharedMediaCmdPtr& source);
OSCL_IMPORT_REF void convertToPVMFMediaCmd(PVMFSharedMediaCmdPtr& dest,
        PVMFSharedMediaMsgPtr& source);

#endif
