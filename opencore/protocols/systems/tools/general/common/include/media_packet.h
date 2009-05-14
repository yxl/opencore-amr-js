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
// -*- c++ -*-
#ifndef MEDIA_PACKET_H
#define MEDIA_PACKET_H

#include "oscl_media_data.h"
#include "oscl_media_status.h"
#include "oscl_time.h"

#include "media_fragment.h"
#include "packet.h"
#ifndef PVMF_MEDIA_DATA_H_INCLUDED
#include "pvmf_media_data.h"
#endif
#ifndef PVMF_SIMPLE_MEDIA_BUFFER_H_INCLUDED
#include "pvmf_simple_media_buffer.h"
#endif
// Constants controlling the size of the RTP packet container

class MediaPacket : public Packet
{
    public:
        virtual ~MediaPacket() {};

        OSCL_IMPORT_REF BufFragStatusClass::status_t AddMediaFragment(MediaFragment* fragment, int32 location_offset = APPEND_MEDIA_AT_END);

        OSCL_IMPORT_REF MediaFragment* GetMediaFragment(int32 idx, MediaFragment& fragment);

        OSCL_IMPORT_REF void Reset();
        OSCL_IMPORT_REF void SetTimeval(TimeValue& val)
        {
            time_val = val;
        }
        OSCL_IMPORT_REF TimeValue* GetTimeval()
        {
            return &time_val;
        }

        uint8* cur_pos;
        uint8* end_pos;
        int cur_frag_num;

    protected:
        MediaPacket() : cur_pos(NULL), end_pos(NULL), cur_frag_num(0)/*, next(NULL)*/ {};
    private:
//	MediaPacket *next;
        TimeValue time_val;
        friend class MediaPacketAllocator;
        friend class MediaPacketPoolAllocator;
};

class MediaPacketAllocator
{
    public:
        MediaPacketAllocator() {};

        virtual ~MediaPacketAllocator()
        {
            //printf("here\n");
        }
        /* This should change to allocate memory from a memory pool */
        OSCL_IMPORT_REF virtual MediaPacket* allocate_packet(EPacketType type = EPacketDefault);
        /* Change to return to pool */
        OSCL_IMPORT_REF virtual void deallocate_packet(MediaPacket* pack);
};

class MediaPacketPoolAllocator : public MediaPacketAllocator
{
    public:
        OSCL_IMPORT_REF MediaPacketPoolAllocator(int packets);
        virtual ~MediaPacketPoolAllocator();

        OSCL_IMPORT_REF MediaPacket *allocate_packet(EPacketType type = EPacketDefault);

        OSCL_IMPORT_REF void deallocate_packet(MediaPacket *pack);

    private:
        int num_packets;
        MediaPacket *free_list;
        MediaPacket *entire_list;
};

OSCL_IMPORT_REF PVMFSharedMediaMsgPtr MediaPacketToPVMFMediaMsg(MediaPacket* packet, PVMFSimpleMediaBufferCombinedAlloc& alloc);
OSCL_IMPORT_REF MediaPacket* PVMFMediaMsgToMediaPacket(PVMFSharedMediaMsgPtr aMsg,
        MediaPacketAllocator& aMediaPacketAlloc, FragmentAllocator& aFragmentAlloc);
#endif
