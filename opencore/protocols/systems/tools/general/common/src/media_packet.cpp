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
#ifndef PVMF_RETURN_CODES_H_INCLUDED
#include "pvmf_return_codes.h"
#endif

#include "media_packet.h"

OSCL_EXPORT_REF MediaFragment* MediaPacket::GetMediaFragment(int32 idx, MediaFragment& fragment)
{
    BufferState * state = GetBufferState(idx);
    /*if(!state)
    	return NULL;*/
    fragment.Set(*GetFragment(idx), state);
    return &fragment;
}


OSCL_EXPORT_REF BufFragStatusClass::status_t MediaPacket::AddMediaFragment(MediaFragment* fragment, int32 location_offset)
{
    return AddFragment(*fragment->GetFragment(), fragment->GetState(), location_offset);
}

OSCL_EXPORT_REF void MediaPacket::Reset()
{
    cur_frag_num = 0;
    cur_pos = NULL;
    end_pos = NULL;
    AppendNext(NULL);
}


/* This should change to allocate memory from a memory pool */
OSCL_EXPORT_REF MediaPacket* MediaPacketAllocator::allocate_packet(EPacketType type)
{
    OSCL_UNUSED_ARG(type);
    return OSCL_NEW(MediaPacket, ());
}

/* Change to return to pool */
OSCL_EXPORT_REF void MediaPacketAllocator::deallocate_packet(MediaPacket* pack)
{
    OSCL_DELETE(pack);
}

OSCL_EXPORT_REF MediaPacketPoolAllocator::MediaPacketPoolAllocator(int packets)
{
    num_packets = packets;

    entire_list = OSCL_ARRAY_NEW(MediaPacket, num_packets);
    free_list = entire_list;

    int i;
    for (i = 0; i < num_packets; i++)
    {
        if (i > 0)
        {
            entire_list[i-1].AppendNext(&entire_list[i]);
        }
    }
    entire_list[i-1].AppendNext(NULL);

}

MediaPacketPoolAllocator::~MediaPacketPoolAllocator()
{
    if (entire_list) delete[] entire_list;
}

OSCL_EXPORT_REF MediaPacket *MediaPacketPoolAllocator::allocate_packet(EPacketType type)
{
    OSCL_UNUSED_ARG(type);

    MediaPacket *pkt;

    if (free_list)
    {
        pkt = free_list;
        free_list = (MediaPacket *)free_list->GetNext();

        pkt->AppendNext(NULL);

        return pkt;
    }
    else
    {
        return NULL;
    }
}

OSCL_EXPORT_REF void MediaPacketPoolAllocator::deallocate_packet(MediaPacket* pack)
{
    pack->Clear();
    pack->Reset();

    pack->AppendNext(free_list);
    free_list = pack;

}

OSCL_EXPORT_REF PVMFSharedMediaMsgPtr MediaPacketToPVMFMediaMsg(MediaPacket* packet, PVMFSimpleMediaBufferCombinedAlloc& alloc)
{
    if (packet == NULL || packet->GetMediaSize() == 0)
    {
        OSCL_LEAVE(PVMFErrArgument);
    }
    uint16 packet_size = (uint16)packet->GetMediaSize();

    // Create new media data buffer
    OsclSharedPtr<PVMFMediaDataImpl> mediaDataImpl = alloc.allocate(packet_size);
    PVMFSharedMediaDataPtr mediaData = PVMFMediaData::createMediaData(mediaDataImpl);

    // Retrieve memory fragment to write to
    OsclRefCounterMemFrag refCtrMemFrag;
    mediaData->getMediaFragment(0, refCtrMemFrag);
    uint8* copy_pos = (uint8*)refCtrMemFrag.getMemFragPtr();
    MediaFragment frag;
    for (int frag_num = 0; frag_num < packet->GetNumFrags(); frag_num++)
    {
        packet->GetMediaFragment(frag_num, frag);
        oscl_memcpy(copy_pos, frag.GetPtr(), frag.GetLen());
        copy_pos += frag.GetLen();
    }

    // update the filled length of the fragment
    mediaData->setMediaFragFilledLen(0, packet_size);

    // Send packet to downstream node
    PVMFSharedMediaMsgPtr mediaMsg;
    convertToPVMFMediaMsg(mediaMsg, mediaData);
    return mediaMsg;
}

/* Callers responsibility to delete the packet */
OSCL_EXPORT_REF MediaPacket* PVMFMediaMsgToMediaPacket(PVMFSharedMediaMsgPtr aMsg,
        MediaPacketAllocator& aMediaPacketAlloc, FragmentAllocator& aFragmentAlloc)
{

    PVMFSharedMediaDataPtr mediaData;
    convertToPVMFMediaData(mediaData, aMsg);
    if (mediaData->getFilledSize() == 0)
    {
        return NULL;
    }

    MediaPacket* packet = aMediaPacketAlloc.allocate_packet();
    OSCL_ASSERT(packet->GetNumFrags() == 0);

    MediaFragment *frag = aFragmentAlloc.allocate_fragment(mediaData->getFilledSize());

    OsclRefCounterMemFrag rcmfrag;

    uint8* write_pos = frag->GetPtr();
    for (uint16 frag_num = 0; frag_num < mediaData->getNumFragments(); frag_num++)
    {
        mediaData->getMediaFragment(frag_num, rcmfrag);
        oscl_memcpy(write_pos, rcmfrag.getMemFragPtr(), rcmfrag.getMemFragSize());
        write_pos += rcmfrag.getMemFragSize();
    }

    // add fragment to packet and send
    packet->AddMediaFragment(frag);
    aFragmentAlloc.deallocate_fragment(frag);

    return packet;
}
