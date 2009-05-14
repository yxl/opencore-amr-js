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

#include "rtsp_embedded_rtp.h"
#include "buf_frag_group.h"


inline bool
RtspEmbeddedRtpPacket::checkSanity(RTPPacket *   newRtpPacket,
                                   ChannelIdType newChannelId,
                                   LengthType    newLength
                                  )
{
    if (NULL == newRtpPacket)
    {
        status = INVALID_RTP_PACKET;
        return false;
    }

    if (newRtpPacket->GetNumFrags() > RTSP_EMB_RTP_MAX_NUM_BUFFER_FRAGMENTS - 1)
    {
        status = NOT_ENOUGH_BUFFER_FRAGS;
        return false;
    }

    return true;
}

inline void
RtspEmbeddedRtpPacket::buildDollarSequenceBuffer(ChannelIdType   newChannelId,
        LengthType      newLength
                                                )
{
    dollarSequenceBuffer[0] = '$';
    dollarSequenceBuffer[1] = newChannelId;
    dollarSequenceBuffer[2] = uint8((newLength & 0xFF00) >> 8);
    dollarSequenceBuffer[3] = uint8((newLength & 0xFF));

    fragments[0].ptr = dollarSequenceBuffer;
    fragments[0].len = RTSP_EMB_RTP_DOLLAR_SEQUENCE_BUFFER_SIZE;

    ++totalNumFragments;
}

inline void
RtspEmbeddedRtpPacket::fillOutBufferFragmentGroupFromPacket(RTPPacket * newRtpPacket)
{
    BufferFragment * rtpFragments = newRtpPacket->GetFragments();
    uint32  numRtpPacketFrags     = newRtpPacket->GetNumFrags();

    for (uint32 ii = 0; ii < numRtpPacketFrags; ++ii, ++totalNumFragments)
    {
        fragments[totalNumFragments].ptr = rtpFragments[ii].ptr;
        fragments[totalNumFragments].len = rtpFragments[ii].len;
    }

    // done
}

inline void
RtspEmbeddedRtpPacket::startAccounting()
{
    currentBufferFragIdx = 0;
}


bool
RtspEmbeddedRtpPacket::bind(RTPPacket *     newRtpPacket,
                            ChannelIdType   newChannelId,
                            LengthType      newLength
                           )
{
    // check args
    //

    if (! checkSanity(newRtpPacket, newChannelId, newLength))
    {
        return false;
    }

    // args are fine

    totalNumFragments = 0;
    boundaryReached = false;
    status = BOUND;

    buildDollarSequenceBuffer(newChannelId, newLength);
    fillOutBufferFragmentGroupFromPacket(newRtpPacket);
    startAccounting();

    return true;
}

void
RtspEmbeddedRtpPacket::unbind()
{
    status = UNBOUND;
}

bool
RtspEmbeddedRtpPacket::getRemainingFrags(BufferFragment * & bufArrayPointerRef,
        uint32 &           numFragmentsRef
                                        )
{
    if (BOUND != status)
    {
        return false;
    }

    if (boundaryReached)
    {
        bufArrayPointerRef = NULL;
        numFragmentsRef = 0;

        return false;
    }

    bufArrayPointerRef = & fragments[ currentBufferFragIdx ];
    numFragmentsRef = totalNumFragments - currentBufferFragIdx;

    return true;
}

bool
RtspEmbeddedRtpPacket::registerBytesWritten(uint32 bytesWritten)
{
    if (BOUND != status)
    {
        return false;
    }


    int offset = 0;
    uint8 * ptr;

    bool seekResult = seekBufFragGroup(fragments,
                                       totalNumFragments,
                                       currentBufferFragIdx,
                                       offset,
                                       ptr,
                                       boundaryReached,
                                       bytesWritten,
                                       0
                                      );
    if (false == seekResult)
    {
        status = SEEK_ERROR;

        return false;
    }

    // hooray

    fragments[ currentBufferFragIdx ].ptr =
        static_cast<int8*>(fragments[currentBufferFragIdx].ptr) + offset;
    fragments[ currentBufferFragIdx ].len -= offset;

    if (! boundaryReached)
    {
        if (1 == totalNumFragments - currentBufferFragIdx
                &&  0 == fragments[ currentBufferFragIdx ].len
           )
        {
            boundaryReached = true;
        }
    }

    return true;
}
