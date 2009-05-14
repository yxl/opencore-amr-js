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
#ifndef PVMF_MEDIA_CMD_H_INCLUDED
#include "pvmf_media_cmd.h"
#endif
#ifndef PVMF_MEDIA_MSG_HEADER_H_INCLUDED
#include "pvmf_media_msg_header.h"
#endif
#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif


OSCL_EXPORT_REF PVMFTimestamp PVMFMediaCmd::getTimestamp()
{
    return hdr_ptr->timestamp;
}

OSCL_EXPORT_REF uint32 PVMFMediaCmd::getDuration()
{
    return hdr_ptr->duration;
}

OSCL_EXPORT_REF uint32 PVMFMediaCmd::getStreamID()
{
    return hdr_ptr->stream_id;
}

OSCL_EXPORT_REF uint32 PVMFMediaCmd::getSeqNum()
{
    return hdr_ptr->seqnum;
}

OSCL_EXPORT_REF PVUid32 PVMFMediaCmd::getFormatID()
{
    return hdr_ptr->format_id;
}

OSCL_EXPORT_REF bool PVMFMediaCmd::queryInterface(const PVUuid& uuid, PVInterface*& iface)
{
    OSCL_UNUSED_ARG(uuid);
    iface = NULL;
    return false;
}

OSCL_EXPORT_REF bool PVMFMediaCmd::getFormatSpecificInfo(OsclRefCounterMemFrag& memfrag)
{
    memfrag = hdr_ptr->format_spec_info;
    return true;
}

OSCL_EXPORT_REF void PVMFMediaCmd::setFormatSpecificInfo(OsclRefCounterMemFrag& memfrag)
{
    hdr_ptr->format_spec_info = memfrag;
}

OSCL_EXPORT_REF void PVMFMediaCmd::setTimestamp(const PVMFTimestamp& ts)
{
    hdr_ptr->timestamp = ts;
}

OSCL_EXPORT_REF void PVMFMediaCmd::setDuration(const uint32& duration)
{
    hdr_ptr->duration = duration;
}

OSCL_EXPORT_REF void PVMFMediaCmd::setSeqNum(uint32 seq)
{
    hdr_ptr->seqnum = seq;
}

OSCL_EXPORT_REF void PVMFMediaCmd::setStreamID(uint32 id)
{
    hdr_ptr->stream_id = id;
}

OSCL_EXPORT_REF void PVMFMediaCmd::setFormatID(PVUid32 id)
{
    hdr_ptr->format_id = id;
}

class MediaCmdCleanupSA : public OsclDestructDealloc
{
    public:
        virtual ~MediaCmdCleanupSA() {};
        virtual void destruct_and_dealloc(OsclAny* ptr)
        {
            uint8* tmp_ptr = (uint8*) ptr;
            uint aligned_refcnt_size =
                oscl_mem_aligned_size(sizeof(OsclRefCounterSA<MediaCmdCleanupSA>));
            tmp_ptr += aligned_refcnt_size;
            PVMFMediaCmd* mcmd_ptr = reinterpret_cast<PVMFMediaCmd*>(tmp_ptr);
            mcmd_ptr->~PVMFMediaCmd();
            OsclMemAllocator alloc;
            alloc.deallocate(ptr);
        }
};

class MediaCmdCleanupDA : public OsclDestructDealloc
{
    public:
        MediaCmdCleanupDA(Oscl_DefAlloc* in_gen_alloc) : gen_alloc(in_gen_alloc) {};
        virtual ~MediaCmdCleanupDA() {};

        virtual void destruct_and_dealloc(OsclAny* ptr)
        {
            uint8* tmp_ptr = (uint8*) ptr;
            uint aligned_size =
                oscl_mem_aligned_size(sizeof(OsclRefCounterDA));
            // skip the refcounter
            tmp_ptr += aligned_size;
            // skip the MediaData Cleanup
            aligned_size = oscl_mem_aligned_size(sizeof(MediaCmdCleanupDA));
            tmp_ptr += aligned_size;

            PVMFMediaCmd* mcmd_ptr = reinterpret_cast<PVMFMediaCmd*>(tmp_ptr);
            mcmd_ptr->~PVMFMediaCmd();
            gen_alloc->deallocate(ptr);
        }

    private:
        Oscl_DefAlloc* gen_alloc;
};


OSCL_EXPORT_REF OsclSharedPtr<PVMFMediaCmd>
PVMFMediaCmd::createMediaCmd(Oscl_DefAlloc* gen_alloc)
{
    // allocate enough room
    uint8* my_ptr;
    OsclRefCounter* my_refcnt;

    uint aligned_media_cmd_size =
        oscl_mem_aligned_size(sizeof(PVMFMediaCmd));

    // must compute the aligned size for PVMFMediaCmd.
    if (gen_alloc)
    {
        uint aligned_refcnt_size =
            oscl_mem_aligned_size(sizeof(OsclRefCounterDA));
        uint aligned_cleanup_size =
            oscl_mem_aligned_size(sizeof(MediaCmdCleanupDA));
        my_ptr = (uint8*) gen_alloc->ALLOCATE(aligned_refcnt_size +
                                              aligned_cleanup_size +
                                              aligned_media_cmd_size +
                                              sizeof(PVMFMediaMsgHeader));

        MediaCmdCleanupDA *my_cleanup =
            OSCL_PLACEMENT_NEW(my_ptr + aligned_refcnt_size, MediaCmdCleanupDA(gen_alloc));
        my_refcnt = OSCL_PLACEMENT_NEW(my_ptr, OsclRefCounterDA(my_ptr, my_cleanup));
        my_ptr += aligned_refcnt_size + aligned_cleanup_size;
    }
    else
    {
        OsclMemAllocator my_alloc;
        uint aligned_refcnt_size =
            oscl_mem_aligned_size(sizeof(OsclRefCounterSA<MediaCmdCleanupSA>));
        my_ptr = (uint8*) my_alloc.ALLOCATE(aligned_refcnt_size +
                                            aligned_media_cmd_size +
                                            sizeof(PVMFMediaMsgHeader));
        my_refcnt = OSCL_PLACEMENT_NEW(my_ptr, OsclRefCounterSA<MediaCmdCleanupSA>(my_ptr));
        my_ptr += aligned_refcnt_size;
    }

    PVMFMediaCmd *media_cmd_ptr = OSCL_PLACEMENT_NEW(my_ptr, PVMFMediaCmd());

    media_cmd_ptr->hdr_ptr =
        OSCL_PLACEMENT_NEW(my_ptr + aligned_media_cmd_size, PVMFMediaMsgHeader());

    OsclSharedPtr<PVMFMediaCmd> shared_media_cmd(media_cmd_ptr, my_refcnt);

    return shared_media_cmd;
}

OSCL_EXPORT_REF OsclSharedPtr<PVMFMediaCmd>
PVMFMediaCmd::createMediaCmd(const PVMFMediaMsgHeader* msgHeader,
                             Oscl_DefAlloc* gen_alloc)
{
    // allocate enough room
    uint8* my_ptr;
    OsclRefCounter* my_refcnt;

    uint aligned_media_data_size =
        oscl_mem_aligned_size(sizeof(PVMFMediaCmd));

    // must compute the aligned size for PVMFMediaCmd.
    if (gen_alloc)
    {
        uint aligned_refcnt_size =
            oscl_mem_aligned_size(sizeof(OsclRefCounterDA));
        uint aligned_cleanup_size =
            oscl_mem_aligned_size(sizeof(MediaCmdCleanupDA));

        my_ptr = (uint8*) gen_alloc->ALLOCATE(aligned_refcnt_size +
                                              aligned_cleanup_size +
                                              aligned_media_data_size +
                                              sizeof(PVMFMediaMsgHeader));

        MediaCmdCleanupDA *my_cleanup =
            OSCL_PLACEMENT_NEW(my_ptr + aligned_refcnt_size, MediaCmdCleanupDA(gen_alloc));
        my_refcnt = OSCL_PLACEMENT_NEW(my_ptr, OsclRefCounterDA(my_ptr, my_cleanup));
        my_ptr += aligned_refcnt_size + aligned_cleanup_size;

    }
    else
    {
        OsclMemAllocator my_alloc;
        uint aligned_refcnt_size =
            oscl_mem_aligned_size(sizeof(OsclRefCounterSA<MediaCmdCleanupSA>));
        my_ptr = (uint8*) my_alloc.ALLOCATE(aligned_refcnt_size +
                                            aligned_media_data_size +
                                            sizeof(PVMFMediaMsgHeader));
        my_refcnt = OSCL_PLACEMENT_NEW(my_ptr, OsclRefCounterSA<MediaCmdCleanupSA>(my_ptr));
        my_ptr += aligned_refcnt_size;
    }

    PVMFMediaCmd *media_cmd_ptr = OSCL_PLACEMENT_NEW(my_ptr, PVMFMediaCmd());

    media_cmd_ptr->hdr_ptr =
        OSCL_PLACEMENT_NEW(my_ptr + aligned_media_data_size, PVMFMediaMsgHeader());

    media_cmd_ptr->hdr_ptr->timestamp = msgHeader->timestamp;
    media_cmd_ptr->hdr_ptr->duration = msgHeader->duration;
    media_cmd_ptr->hdr_ptr->stream_id = msgHeader->stream_id;
    media_cmd_ptr->hdr_ptr->seqnum    = msgHeader->seqnum;
    media_cmd_ptr->hdr_ptr->format_id = msgHeader->format_id;
    media_cmd_ptr->hdr_ptr->format_spec_info = msgHeader->format_spec_info;

    OsclSharedPtr<PVMFMediaCmd> shared_media_data(media_cmd_ptr, my_refcnt);

    return shared_media_data;
}


OSCL_EXPORT_REF PVMFMediaCmd::~PVMFMediaCmd()
{
    // call the destructors
    hdr_ptr->~PVMFMediaMsgHeader();
}

OSCL_EXPORT_REF void convertToPVMFMediaCmdMsg(PVMFSharedMediaMsgPtr& dest,
        PVMFSharedMediaCmdPtr& source)
{
    OsclRefCounter* refcnt = source.GetRefCounter();
    /*
     * explicitly increment refcnt since Bind doesn't do it when
     * refcnt and pointer are added separately
     */
    refcnt->addRef();
    dest.Bind(source.GetRep(), refcnt);
}

OSCL_EXPORT_REF void convertToPVMFMediaCmd(PVMFSharedMediaCmdPtr& dest,
        PVMFSharedMediaMsgPtr& source)
{
    OsclRefCounter* refcnt = source.GetRefCounter();
    /*
     * explicitly increment refcnt since Bind doesn't do it when
     * refcnt and pointer are added separately
     */
    refcnt->addRef();
    dest.Bind(reinterpret_cast<PVMFMediaCmd*>(source.GetRep()), refcnt);
}


