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
///////////////////////////////////////////////////////////////////////////////
//
// sequence_gen.cpp
//
//
//
//
///////////////////////////////////////////////////////////////////////////////

#include "oscl_assert.h"
#include "sequence_gen.h"


SequenceGenerator::SequenceGenerator(int initialSeqNum,
                                     uint32 timestampReposDelta)
{
    SequenceGenerator::initialSeqNum = initialSeqNum;
    SequenceGenerator::timestampReposDelta = timestampReposDelta;
}

SequenceGenerator::~SequenceGenerator()
{

}

bool SequenceGenerator::registerNextObjectFrag(uint stream,
        uint objNum,
        uint objOffset,
        uint objLen,
        uint fragLen,
        bool* inCompleteFlag,
        bool ibRepositionFlag)
{
    StreamSequenceInfo* pstream = getStream(stream);

    // check if previously unprovisioned stream
    if (pstream->streamId == -1)
    {
        // initialize
        pstream->streamId = stream;
        pstream->currSeq = initialSeqNum;
        pstream->currObjectNum = objNum;
        pstream->currObjectLen = objLen;
        pstream->currObjectOffset = objOffset + fragLen;
    }


    else if (pstream->currObjectNum == objNum)
    {
        // adding data to current object

        // check if data is intended for the current offset
        if (objOffset != pstream->currObjectOffset)
        {
            // lost fragments!

            // skip a sequence number
            pstream->currSeq++;

            // correct the position
            pstream->currObjectOffset = objOffset;
        }

        // account for the new fragment
        pstream->currObjectOffset += fragLen;
    }
    else
    {
        // starting a new object

        pstream->currObjectOffset += fragLen;

        // check if new object is expected
        if (! pstream->objComplete && !ibRepositionFlag)
        {
            // no - previous object is incomplete
            pstream->currSeq++;
        }

        // does the new object start at the beginning?
        if (objOffset != 0)
        {
            // first fragment(s) of object is missing - skip a seq num
            if (!ibRepositionFlag)
                pstream->currSeq++;
            *inCompleteFlag = true;
        }

        pstream->currObjectNum = objNum;
        pstream->currObjectLen = objLen;
        pstream->currObjectOffset = objOffset + fragLen;
    }

    // check if the current object is complete
    pstream->objComplete =
        pstream->currObjectOffset ==
        pstream->currObjectLen;


    return pstream->objComplete;
}

uint32 SequenceGenerator::generateSequenceNum(uint stream)
{
    OSCL_ASSERT(stream < vStreamInfo.size());
    return vStreamInfo[stream].currSeq++;
}

uint32 SequenceGenerator::generateTimestamp(uint stream, uint32 timestamp, bool reposition)
{
    StreamSequenceInfo* pInfo = getStream(stream);

    if (reposition)
    {
//       pInfo->timestampBase = pInfo->currTimestamp - timestamp + timestampReposDelta;
    }

    //OSCL_ASSERT(timestamp + pInfo->timestampBase >= pInfo->currTimestamp);

    return (pInfo->currTimestamp = timestamp + pInfo->timestampBase);
}

SequenceGenerator::StreamSequenceInfo* SequenceGenerator::getStream(uint stream)
{
    // check if the stream array is big enough - if not,
    // grow it to sufficient size
    if (stream >= vStreamInfo.size())
    {
        StreamSequenceInfo empty;

        for (uint i = vStreamInfo.size(); i < stream + 1; i++)
        {
            vStreamInfo.push_back(empty);
        }
    }

    return &vStreamInfo[stream];
}

uint32 SequenceGenerator::getMinTimestamp()
{
    if (vStreamInfo.size() == 0)
    {
        return 0;
    }

    uint32 min = 0x7FFFFFFF;
    Oscl_Vector<StreamSequenceInfo, OsclMemAllocator>::iterator it;

    for (it = vStreamInfo.begin(); it != vStreamInfo.end(); it++)
    {
        if (it->streamId == -1)
        {
            continue;
        }
        if (it->currTimestamp < min)
        {
            min = it->currTimestamp;
        }
    }
    return min;
}

// set next seqnum for stream
void   SequenceGenerator::setSeqnum(const uint stream, const uint32 seqnum)
{
    Oscl_Vector<StreamSequenceInfo, OsclMemAllocator>::iterator it;
    for (it = vStreamInfo.begin(); it != vStreamInfo.end(); it++)
    {
        if (it->streamId == (int)stream)
        {
            // since we are interrupting sequence here, also reset object ID to avoid gaps
            it->currSeq = seqnum;
            it->currObjectOffset = 0;
            it->objComplete = true;
            break;
        }
    }
}
