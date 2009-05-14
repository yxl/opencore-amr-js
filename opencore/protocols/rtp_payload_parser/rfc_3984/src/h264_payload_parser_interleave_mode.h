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
#ifndef H264_PAYLOAD_PARSER_INTERLEAVE_MODE_H
#define H264_PAYLOAD_PARSER_INTERLEAVE_MODE_H


#ifndef OSCL_PRIQUEUE_H_INCLUDED
#include "oscl_priqueue.h"
#endif

#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif

#ifndef PAYLOAD_PARSER_H_INCLUDED
#include "payload_parser.h"
#endif

#ifndef H264_PAYLOAD_PARSER_MACROS_H_INCLUDED
#include "h264_payload_parser_macros.h"
#endif

/////////////////////////////////////////////////////////////////////////////////
////////////// priority queue related data structures ///////////////////////////
/////////////////////////////////////////////////////////////////////////////////
class PVH264PayLoadOutputMediaData
{
    public:
        PVH264PayLoadOutputMediaData() : don(0), fuType(0) {};

        PVH264PayLoadOutputMediaData(const PVH264PayLoadOutputMediaData& x)
        {
            data  = x.data;
            don  = x.don;
            fuType = x.fuType;
        }

        PVH264PayLoadOutputMediaData(IPayloadParser::Payload adata, uint32 aDon, uint32 aFuType)
        {
            data = adata;
            don = aDon;
            fuType = aFuType;
        }

        //required for compile of OsclPriorityQueue but not currently used & not implemented.
        //would be needed if code called "remove".
        bool operator==(const PVH264PayLoadOutputMediaData& x)const
        {
            OSCL_UNUSED_ARG(&x);
            OSCL_ASSERT(0);
            return false;
        }

    public:
        IPayloadParser::Payload data;
        uint32	don;
        uint32  fuType;	// First byte: current fuType
        // 0 complete NAL
        // 1 starting FU
        // 2 intermediate FU
        // 3 ending FU
        // Second byte: previous fuType
        // Third-Fourth bytes: counter for intermediate FUs for the following compare function

};

// priority comparison class
class PVH264PayLoadOutputMediaDataCompareLess
{
    public:
        /**
        * The algorithm used in OsclPriorityQueue needs a compare function
        * that returns true when a's priority is less than b's. In the
        * current context, when a's don is larger than b's don, return true
        * Basically when a's don is smaller than b's don, then a's priority
        * is higher than b's.
        * @return true if a's priority is less than b's, else false
        */
        int compare(PVH264PayLoadOutputMediaData& a, PVH264PayLoadOutputMediaData& b) const
        {
            int32 diffDon = PVH264PayLoadOutputMediaDataCompareLess::don_diff(a.don, b.don);
            if (diffDon == 0)
            {
                // use fuType
                diffDon = PVH264PayLoadOutputMediaDataCompareLess::fuType_diff(a.fuType, b.fuType);
            }
            return (diffDon < 0);
        }

    private:

        static int32 don_diff(uint32 m_don, uint32 n_don)
        {
            if (m_don == n_don) return 0;

            if (m_don < n_don && (n_don - m_don < 32768))
                return ((int32)n_don - (int32)m_don);

            if (m_don < n_don && (n_don - m_don >= 32768))
                return ((int32)n_don - (int32)m_don - 65536);

            if (m_don > n_don && (m_don - n_don < 32768))
                return -((int32)m_don - (int32)n_don);

            if (m_don > n_don && (m_don - n_don >= 32768))
                return (65536 - m_don + n_don);

            return 0;
        }

        static int32 fuType_diff(uint32 m_fuType, uint32 n_fuType)
        {
            int32 diff_fuType = (int32)(n_fuType & 0x03) - (int32)(m_fuType & 0x03);

            // for intermediate FUs
            if (diff_fuType == 0)
            {
                diff_fuType = (int32)(n_fuType >> 16) - (int32)(m_fuType >> 16);
            }

            return diff_fuType;
        }
};

//memory allocator type for this node.
typedef OsclMemAllocator PVH264PayloadParserAllocator;


class InterleaveModeProcessing
{

    public:

        // constructor
        InterleaveModeProcessing() : iNALUnitsCounter(0),
                iDon(0),
                iDonBase(0),
                iTimestampOffset(0),
                iPrevNALType(0),
                iRtpPayloadPtr(NULL)
        {
            ;
        }

        // destructor
        ~InterleaveModeProcessing()
        {

            // for iOutputMediaDataQueue, remove all the elements from the output data priority queue
            while (!iOutputMediaDataQueue.empty())
            {
                iOutputMediaDataQueue.pop();
            }

            iNALUnitsCounter = iDon = iDonBase =
                                          iTimestampOffset = iPrevNALType = 0;
            iRtpPayloadPtr = NULL;
        }

        // initialize the iOutputMediaDataQueue when in the interleaved mode
        bool initialize(uint32 aInterleaveDepth)
        {
            if (aInterleaveDepth > 0)
            {
                int32 err = 0;
                OSCL_TRY(err, iOutputMediaDataQueue.reserve((aInterleaveDepth + 1) << 2));
                if (err) return false;
            }
            return true;
        }

        // push data to priority queue
        bool queue(const IPayloadParser::Payload& aMediaData, const uint8 nal_type, const uint32 fragmentedBitMask)
        {
            uint32 prev_fuType = iPrevNALType & 0x03;
            uint32 fuType = fragmentedBitMask & 0x03;
            uint32 fuCounter = 0;
            if (fuType == 2)  // for the above fuType_diff() in priority comparsion
            {
                fuCounter = (fragmentedBitMask >> BIT_SHIFT_FOR_FU_COUNTER) << 16;
            }
            PVH264PayLoadOutputMediaData out_data(aMediaData, iDon, (fuCounter | (prev_fuType << 8) | fuType));

            // push into the prority queue
            int32 err;
            OSCL_TRY(err, iOutputMediaDataQueue.push(out_data));
            if (err || iOutputMediaDataQueue.empty()) return false;

            // update internal variables
            updateWhenQueuingData(fuType, nal_type, fragmentedBitMask);
            return true;
        }

        // pop data from priority queue
        void dequeue(IPayloadParser::Payload& accessUnit)
        {
            PVH264PayLoadOutputMediaData out_data(iOutputMediaDataQueue.top());
            iOutputMediaDataQueue.pop();
            accessUnit = out_data.data;

            // update internal variables
            updateWhenDequeuingData(out_data.fuType);
        }

        // priority queue is empty
        bool isQueueEmpty()
        {
            return iOutputMediaDataQueue.empty();
        }

    private:
        // update variables when the prority queue is queuing data
        void updateWhenQueuingData(const uint32 fuType, const uint8 nal_type, const uint32 fragmentedBitMask)
        {
            if (fuType == 0 || fuType == 3)  // complete NAL or ending FU
            {
                iNALUnitsCounter++;
            }

            // update iPrevNALType, only for complete NALs and FU-B
            if (nal_type != H264_RTP_PAYLOAD_FU_A)
            {
                iPrevNALType = fragmentedBitMask & 0x07;
            }
            else   // FU-A
            {
                iPrevNALType >>= 2;
                iPrevNALType <<= 2;		// clear bit 1 and bit 0
                iPrevNALType |= fuType;	// keep bit 2 unchanged for FU-A, for isCurrRTPPacketIgnored()
            }
        }

        // update variables when the prority queue is de-queuing data
        void updateWhenDequeuingData(const uint32 fuType)
        {
            uint32 prev_fuType = (fuType >> 8) & 0x03;
            uint32 curr_fuType = fuType & 0x03;

            if (curr_fuType == 0)  // complete NAL
            {
                iNALUnitsCounter--;
                if (prev_fuType == 1 || prev_fuType == 2)  // current one is complete NAL, but the previous one is fragmented NAL in case that the ending fragmented NAL is lost
                {
                    iNALUnitsCounter--;
                }
            }
            else if (curr_fuType == 3)  // ending fragemented NAL
            {
                iNALUnitsCounter--;
            }
        }


    public:
        // counter of complete NAL units, which could be smaller than the actual number of elements in iOutputMediaDataQueue
        uint32 iNALUnitsCounter;

        // hold the current DON, for FU-As following the first FU-B
        uint32 iDon;

        // hold the DONB, for MTAPs, or serves as the counter for FUs
        uint32 iDonBase;

        // for MTAPs only, save the timestamp offset
        uint32 iTimestampOffset;

        // for the NAL type of the previous media message sent out, should be equal to "iIsFragmentedBitMask&0x03" of class H264PayloadParser
        uint32 iPrevNALType;

        // save the rtp payload pointer to check if the current input rtp packet is the same as the previous one
        uint8 *iRtpPayloadPtr;

        // Priority Queue of output media data
        OsclPriorityQueue < PVH264PayLoadOutputMediaData,
        PVH264PayloadParserAllocator,
        Oscl_Vector<PVH264PayLoadOutputMediaData, PVH264PayloadParserAllocator>,
        PVH264PayLoadOutputMediaDataCompareLess > iOutputMediaDataQueue;

};

#endif // H264_PAYLOAD_PARSER_INTERLEAVE_MODE_H

