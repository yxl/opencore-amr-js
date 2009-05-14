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
// h264_payload_parser.h
//
// H.264 payload parser.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef H264_PAYLOAD_PARSER_INCLUDED_H
#define H264_PAYLOAD_PARSER_INCLUDED_H

#include "payload_parser.h"

#ifndef H264_PAYLOAD_PARSER_MACROS_H_INCLUDED
#include "h264_payload_parser_macros.h"
#endif

// Forward declaration
class H264PayloadParserUtility;
class InterleaveModeProcessing;

///////////////////////////////////////////////////////////////////////////////
//
// H.264 PayloadParser Class
//
///////////////////////////////////////////////////////////////////////////////

class H264PayloadParser : public IPayloadParser
{
    public:
        OSCL_IMPORT_REF H264PayloadParser();
        OSCL_IMPORT_REF virtual ~H264PayloadParser();

        OSCL_IMPORT_REF bool Init(mediaInfo* config);
        OSCL_IMPORT_REF PayloadParserStatus Parse(const Payload& inputPacket,
                Oscl_Vector<Payload, OsclMemAllocator>& vParsedPayloads);

        // access functions
        uint32 getTimestampForFU() const
        {
            return iTimestampForFU;
        }
        void setTimestampForFU(const uint32 aTimestampForFU)
        {
            iTimestampForFU = aTimestampForFU;
        }

        uint32 getFragmentedBitMask() const
        {
            return iIsFragmentedBitMask;
        }
        void setFragmentedBitMask(const uint32 aFragmentedBitMask)
        {
            iIsFragmentedBitMask = aFragmentedBitMask;
        }

        OSCL_IMPORT_REF virtual void Reposition(const bool adjustSequence = false, const uint32 stream = 0, const uint32 seqnum = 0);
        OSCL_IMPORT_REF virtual uint32 GetMinCurrTimestamp();


    private:

        /**
         * Parse RTP payload in pass-through mode, basically treat all the input rtp packet as NALU packet
         * without extra parsing.
         * @param aRtpPacket,	input rtp packet
         * @param accessUnit,	output media data
         * @return RTP_PAYLOAD_PARSER_RET_CODE code
         **/
        /**
          * Parse RTP payload for non-interleaved mode(including single NAL unit mode)
          *
          * @param aRtpPacket,	input rtp packet
           * @param nal_type,		input nal type to avoid uncessary parsing operation
          * @param accessUnit,	output media data
          * @return RTP_PAYLOAD_PARSER_RET_CODE code
          **/
        PayloadParserStatus parseRTPPayload_For_Non_InterleavedMode(const Payload& inputPacket,
                const uint8 nal_type,
                Oscl_Vector<Payload, OsclMemAllocator>& vParsedPayloads);

        /**
         * Parse RTP payload for interleaved mode. To implement the interleaved mode is to re-order NAL units from
         * the transmission order to the NAL unit decoder order, by using 16-bit DON(Decoding Order Number),
         * within the deinterleaving buffer. The deinterleaving buffer is designed to use priority queue (e.g.
         * OsclPriorityQueue). The priority comparison is to compare the DONs of any two media message, by using
         * the definition of don_diff(m,n), as specified in section 5.5. Basically don_diff(m,n) > 0 means m is ahead
         * of n in terms of decoding order, and thus indicates m's priority is higher than n's.
         *
         * After the priority queue queues up to srop-interleaving-depth + 1 NAL units during the initial buffering,
         * then priority queue will pop media message out(based on the highest prority, i.e. the lowest DON) for each
         * input RTP packet. And the number of ouput media messages should be same as the number of generated media
         * message for the input RTP packet, and thus make sure the priority queue always buffers the same number of
         * NAL units after the initial buffering.
         *
         * @param aRtpPacket,	input rtp packet
          * @param nal_type,		input nal type to avoid uncessary parsing operation
         * @param accessUnit,	output media data
         * @return RTP_PAYLOAD_PARSER_RET_CODE code
         **/
        PayloadParserStatus parseRTPPayload_For_InterleavedMode(const Payload& inputPacket, uint8 nal_type,
                Oscl_Vector<Payload, OsclMemAllocator>& vParsedPayloads);

        /**
         * Parse RTP payload to generate single output media message synchronously, which is used for both non-interleaved mode
         * and interleaved mode.
         *
         * @param aRtpPacket,	input rtp packet
          * @param nal_type,		input nal type to avoid uncessary parsing operation
         * @param accessUnit,	output media data
         * @param rtp_payload_ptr_offset,	the offset of the rtp payload parser pointer, especially used for handling MTAP packet
         * @return RTP_PAYLOAD_PARSER_RET_CODE code
         **/
        PayloadParserStatus parseRTPPayload_For_SingleMessageOutput(const Payload& inputPacket, uint8 nal_type,
                Oscl_Vector<Payload, OsclMemAllocator>& vParsedPayloads,
                uint32 &rtp_payload_ptr_offset);

        /**
         * Check if data queue flush is needed
         *
         * @param aRtpPacket,	input rtp packet
         * @return true => data queue flush is needed
         **/
        inline bool isFlushNeeded(const Payload& rtpPacket);

        /**
        * Flush the priority queue
        * @param accessUnit, output media data
        * @return PayloadParserStatus code
        **/
        PayloadParserStatus flush(Payload& accessUnit);


        /**
         * Check if the current packetization mode is interleaved mode. Right now use interleaving depth, later we may have a better
         * way. That's why I keep this as a function
         *
           * @param nal_type,	input nal type to avoid uncessary parsing operation
         * @return true => interleave mode ; false => non-interleaved mode
         **/
        bool isInterleaveMode()
        {
            return (iInterleaveDepth != 0);
        }

        /**
         * For interleave mode, there are the required nal types (STAP-B, MTAPs, FU-B/FU-A),
         * @param nal_type,		input nal type to avoid uncessary parsing operation
         * @return true => required nal type using in interleave mode ;
         *         false => not required nal type which is probably not usde in interleave mode
         **/
        bool isRequiredTypeForInterleaveMode(const uint8 nal_type)
        {
            return (nal_type == H264_RTP_PAYLOAD_STAP_B ||	// STAP-B(Single-Time Aggregation Packet with DON(Decoding Order Number)
                    nal_type == H264_RTP_PAYLOAD_MTAP16 ||	// MTAP16(Multi-Time Aggregation Packet with 16-bit time stamp offset
                    nal_type == H264_RTP_PAYLOAD_MTAP24 ||	// MTAP24(Multi-Time Aggregation Packet with 24-bit time stamp offset
                    nal_type == H264_RTP_PAYLOAD_FU_B   ||	// FU-B(Fragmentation Unit with DON(Decoding Order Number))
                    (nal_type == H264_RTP_PAYLOAD_FU_A && isInterleaveMode()));  // FU-A(Fragmentation Unit without DON follows the first FU-B
        }

        /**
         * For interleave mode, besides the required nal types (STAP-B, MTAPs, FU-B/FU-A),
         * there are exceptional nal types that can be used for interleave mode, e.g. SPS and PPS
         * @param nal_type,		input nal type to avoid uncessary parsing operation
         * @return true => exceptional nal type using in interleave mode ;
         *         false => not exceptional nal type which is prohibited in interleave mode
         **/
        bool isExceptionTypeForInterleaveMode(const uint8 nal_type)
        {
            return (nal_type == H264_RTP_PAYLOAD_SPS || nal_type == H264_RTP_PAYLOAD_PPS);
        }

        // check if the input nal type is MTAP type(MTAP16 or MTAP24)
        bool isMTAPType(const uint8 nal_type)
        {
            return (nal_type == H264_RTP_PAYLOAD_MTAP16 || nal_type == H264_RTP_PAYLOAD_MTAP24);
        }


        /**
         * Check if the current input packet is the same as the previous input for the return code: RTP_PAYLOAD_PARSER_INPUT_NOT_EXHAUSTED
         *
         * @param aRtpPacket,	input rtp packet
          * @param nal_type,		input nal type to avoid uncessary parsing operation
         * @return true => ignore the current packet ; false => will use this packet
         **/
        inline bool isCurrRTPPacketIgnored(const Payload& inputPacket, const uint8 nal_type);

        /**
         * Get the input (media data imp object "aMediaDataIn" and memory fragment "aMemFragIn") setup. And check the memory fragment is already
         * used up, then we need to exit. In addition, get the actual utility object based on the input nal type. This utility object is used to
         * get marker info, timestamp and memory fragment for different nal types. Here I used polymorphism to address the multiple-type based handling.
         *
         * @param aRtpPacket,	input rtp packet
          * @param nal_type,		input nal type to avoid uncessary parsing operation
         * @param aMediaDataIn,	output media data imp object
         * @param aMemFragIn,	output memory fragment to hold the actual payload
         * @param rtp_payload_ptr_offset,	the offset of the rtp payload parser pointer, used to check if the rtp payload is parsed all
         * @return true => ignore the current packet ; false => will use this packet
         **/
        inline bool getInputSetup(const Payload& inputPacket, const uint8 nal_type,
                                  uint32 rtp_payload_ptr_offset);
        /**
         * Create utility table, iUtilityTable. In order to remove run-time overhead of dynamic creation, we create the utility objects (4) all upfront
         * Basically, iUtilityTable[0] => STAP packet handling
         *			  iUtilityTable[1] => FU packet handling
         *			  iUtilityTable[2] => MTAP packet handling
         *			  iUtilityTable[3] => NALU(or undefined nal type) packet handling
         *
         * @return true => success ; false => failure
         **/
        bool createParserUtilityTable();

        // Delete utility table, iUtilityTable
        void deleteParserUtilityTable();

        // Create and initialize interleaving mode processing object iIMP
        bool createIMPObject();

        // get the current nal type
        inline bool getNALType(const Payload& inputPacket, uint8 &nal_type);

    private:
        H264PayloadParserUtility **iUtilityTable;
        H264PayloadParserUtility *iUtility; // save the current utility in the table

        uint32 iInterleaveDepth;
        InterleaveModeProcessing *iIMP; // defined in h264_payload_parser_internal.h

        // These two variables are "global" for all utility objects to remove static variables defined in h264_payload_parser_utility.h
        uint32 iTimestampForFU;
        uint32 iIsFragmentedBitMask; // bit 1-0:	0 completed NAL
        //				1 starting FU
        //				2 intermediate FU
        //				3 ending FU
        // bit 2:	0 = FU-A 1 = FU-B
        // bit 10-3:FU header: S E R Type
        // >=bit11: counter for intermediate FUs
};

#endif //H264_PAYLOAD_PARSER_INCLUDED_H

