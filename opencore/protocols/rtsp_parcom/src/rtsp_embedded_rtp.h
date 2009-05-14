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

#ifndef RTSP_EMBEDDED_RTP_H_
#define RTSP_EMBEDDED_RTP_H_

//#include "oscl_types.h"
#include "oscl_base.h"
#include "rtp_packet.h"

#ifdef RTSP_EMB_RTP_DOLLAR_SEQUENCE_BUFFER_SIZE
#error Hey, somebody is using RTSP_EMB_RTP_DOLLAR_SEQUENCE_BUFFER_SIZE
#else
#define RTSP_EMB_RTP_DOLLAR_SEQUENCE_BUFFER_SIZE 4
#endif

#ifdef RTSP_EMB_RTP_MAX_NUM_BUFFER_FRAGMENTS
#error Hey, somebody is using RTSP_EMB_RTP_MAX_NUM_BUFFER_FRAGMENTS
#else
#define RTSP_EMB_RTP_MAX_NUM_BUFFER_FRAGMENTS 32
#endif

class RtspEmbeddedRtpPacket
{
    public:

        typedef enum
        {
            BOUND,
            UNBOUND,
            SEEK_ERROR,
            NOT_ENOUGH_BUFFER_FRAGS,
            INVALID_RTP_PACKET
        } Status;

    protected:

        Status  status;

    public:

        inline Status
        getStatus()
        {
            return  status;
        }

        inline bool
        isOk()
        {
            return ((UNBOUND == status) || (BOUND == status));
        }

    public:

        typedef uint8   ChannelIdType;
        typedef uint16  LengthType;

    private:

        RtspEmbeddedRtpPacket(const RtspEmbeddedRtpPacket &);
        RtspEmbeddedRtpPacket & operator= (const RtspEmbeddedRtpPacket &);

    protected:

//    RTPPacket *       rtpPacket;
//    ChannelIdType     channelId;
//    LengthType        length;

        uint8             dollarSequenceBuffer[RTSP_EMB_RTP_DOLLAR_SEQUENCE_BUFFER_SIZE];
        BufferFragment    fragments[ RTSP_EMB_RTP_MAX_NUM_BUFFER_FRAGMENTS ];

//    LengthType        sizeRemaining;
        int               currentBufferFragIdx;
        int               totalNumFragments;

        bool              boundaryReached;

    public:

        RtspEmbeddedRtpPacket()
                : status(UNBOUND)
        {}

    protected:

        inline bool checkSanity(RTPPacket *     newRtpPacket,
                                ChannelIdType   newChannelId,
                                LengthType      newLength
                               );
        inline void buildDollarSequenceBuffer(ChannelIdType   newChannelId,
                                              LengthType      newLength
                                             );
        inline void fillOutBufferFragmentGroupFromPacket(RTPPacket * newRtpPacket);
        inline void startAccounting();

    public:

        bool        bind(RTPPacket *     newRtpPacket,
                         ChannelIdType   newChannelId,
                         LengthType      newLength
                        );

        void        unbind();

        bool        getRemainingFrags(BufferFragment * &,
                                      uint32 &
                                     );

        bool        registerBytesWritten(uint32 bytesWritten);
};

#endif  // RTSP_EMBEDDED_RTP_H_
