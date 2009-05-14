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

#ifndef PV_GAU_H_
#define PV_GAU_H_
#include "oscl_media_data.h"
#include "oscl_mem.h"

#define MAX_NUM_FRAGMENTS  10
const uint32 MAX_GAU_BUNDLE = 40;

struct SimpleBufferFragGroup
{
    int32 num_fragments;  //number of buffer fragments in this buffer frag group
    BufferFragment fragments[MAX_NUM_FRAGMENTS];  //buffer fragments within this group
    BufferState * buf_states[MAX_NUM_FRAGMENTS];  //buffer state for each buffer fragment

    SimpleBufferFragGroup()
            : num_fragments(0)
    {
    }
};

struct MediaMetaInfo
{
    uint32  len;  //size of the frame
    uint32  ts;   //time stamp
    uint32  ts_delta; // timestamp delta from the previous timestamp
    uint32  sample_info;  //4 uint8 information together
    bool  dropFlag;
    uint8 layer;

    //overload < operator for sorting later
    bool operator < (const struct MediaMetaInfo b) const
    {
        if (ts < b.ts)
            return true;
        else
            return false;
    }

    bool  operator  <= (const struct  MediaMetaInfo  b) const
    {
        if (ts <= b.ts)
            return true;
        else
            return false;
    }
};



struct GAU
{

    uint32  numMediaSamples;  //number of media samples within this GAU
    SimpleBufferFragGroup buf;
    MediaMetaInfo    info[MAX_GAU_BUNDLE];
    bool free_buffer_states_when_done;
    uint64 SampleOffset;
    OSCL_IMPORT_REF uint32  getSamplesTotalSize(uint32 number, MediaMetaInfo *metaInfo);  //return the sum of total sample size

    OSCL_IMPORT_REF GAU();

    OSCL_IMPORT_REF GAU(GAU & gauElement);

    OSCL_IMPORT_REF ~GAU();

    void SetFreeBufferStatesWhenDone()
    {
        free_buffer_states_when_done = true;
    }

    /*
      Return info pointer
    */
    MediaMetaInfo *getInfo(uint32 i)
    {
        if (i < MAX_GAU_BUNDLE)
            return info + i;
        else
            return NULL;

    }


    int32 AddBufFrag(uint8 *ptr, uint32 len, BufferFreeFuncPtr freeFunc)
    {
        if (buf.num_fragments >= MAX_NUM_FRAGMENTS) return 0;

        buf.fragments[buf.num_fragments].ptr = ptr;
        buf.fragments[buf.num_fragments].len = len;
        if (buf.buf_states[buf.num_fragments]) OSCL_DELETE(buf.buf_states[buf.num_fragments]);
        buf.buf_states[buf.num_fragments] = OSCL_NEW(BufferState, ());
        buf.buf_states[buf.num_fragments]->bind(ptr, freeFunc);
        buf.num_fragments ++;
        return 1;
    }
};


#endif  // PV_GAU_H_
