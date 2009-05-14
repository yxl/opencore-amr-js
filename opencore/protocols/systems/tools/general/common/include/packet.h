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
#ifndef PACKET_H
#define PACKET_H

#include "oscl_media_data.h"
#include "oscl_media_status.h"

#include "media_fragment.h"

const int DEF_PACKET_MAX_FRAGMENTS = 128;
const int DEF_PACKET_IMMEDIATE_DATA = 128;
const int DEF_MAX_PACKET_SIZE = 1024;

/* A packet represents a message.  It contains a list of MediaFragments.
   Packets can be chained. */
class Packet : public MediaData<Packet, DEF_PACKET_MAX_FRAGMENTS, DEF_PACKET_IMMEDIATE_DATA>
{
    public:
        Packet(): flat_ptr(NULL)
        {
        }

        virtual ~Packet()
        {
            if (flat_ptr) OSCL_DEFAULT_FREE(flat_ptr);
            flat_ptr = NULL;
            Clear();
        }

        BufFragStatusClass::status_t AddBufferFragment(BufferFragment& frag, BufferState* state)
        {
            return AddFragment(frag, state);
        }

        virtual uint8* AddHeader(MediaFragment* mfrag)
        {
            AddFragment(*mfrag->GetFragment(), mfrag->GetState(), 0);
            return mfrag->GetPtr();
        }

        virtual uint8* AddTrailer(MediaFragment* mfrag)
        {
            AddFragment(*mfrag->GetFragment(), mfrag->GetState(), APPEND_MEDIA_AT_END);
            return mfrag->GetPtr();
        }

        /*  virtual BufFragStatus AddFragment(MediaFragment* fragment, int32 location_offset = APPEND_MEDIA_AT_END) = 0;
          virtual void Clear() = 0;
          virtual BufferFragment* GetFragments() = 0;
          virtual MediaFragment* GetFragment(int32 idx, MediaFragment& fragment) = 0;
          virtual int GetAvailableBufferSize() = 0;
          virtual int32 GetNumFrags() const = 0;
          virtual int32 GetMaxFrags() const = 0;
          virtual uint32 GetLength() const = 0;
          virtual int GetMediaSize() const = 0;
          virtual void AppendNext(Packet *next_ptr) = 0;
          virtual Packet*  GetNext() const = 0;*/
        /* constructs a flat buffer containing the media data.  It gets deleted when the Packet is destroyed
           Shall be removed in later versions */

        virtual uint8* GetDataPtr()
        {
            BufferFragment* frag = NULL;
            if (GetMediaSize() <= 0) return NULL;
            frag = GetFragment(0);
            return ((uint8 *) frag->ptr);
        }

        virtual uint8* GetMediaPtr()
        {
            uint8* buf = NULL;
            BufferFragment* frag = NULL;

            if (GetMediaSize() <= 0) return NULL;
            if (flat_ptr) return flat_ptr;
            buf = flat_ptr = (uint8*)OSCL_DEFAULT_MALLOC(GetMediaSize());
            for (int frag_num = 0; frag_num < GetNumFrags(); frag_num++)
            {
                frag = GetFragment(frag_num);
                oscl_memcpy(buf, frag->ptr, frag->len);
                buf += frag->len;
            }
            return flat_ptr;
        }

        virtual void ClearMediaPtr()
        {
            if (flat_ptr) OSCL_DEFAULT_FREE(flat_ptr);
            flat_ptr = NULL;
        }
    private:
        uint8* flat_ptr;
};


typedef enum PacketTypes
{
    EPacketDefault
}EPacketType;

#endif
