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
/*                                                                               */
/*********************************************************************************/

/*
**   File:   rtcp_decoder.cpp
**
**   Description:
**      This module implements the RTCP_Decoder class. This class is used to encode and
**      decode RTCP_Decoder packets. Please refer to the RTCP_Decoder design document for
**      details.
*/

/*
** Includes
*/


#define DEBUG_PRINT 0
#if DEBUG_PRINT
#include <stdio.h>
#endif

#ifdef PV_OS_ZREX
//you get a compile error on zrex for the arm target
//unless these includes come first.
#include <stdlib.h>
#include <math.h>
#endif

#include "rtcp_decoder.h"
#include "rtcp_constants.h"
#include "oscl_mem.h"

/*
** Constants
*/

/*
** Methods
*/
OSCL_EXPORT_REF  RTCP_Decoder::RTCP_Decoder(const uint8 version)
        : RTCP_Base(version)
{

}

OSCL_EXPORT_REF RTCP_Decoder::~RTCP_Decoder()
{
}

OSCL_EXPORT_REF RTCP_Decoder::Error_t
RTCP_Decoder::scan_compound_packet(OsclMemoryFragment& input_packet,
                                   int32 max_array_size, int32& filled_size,
                                   RTCPPacketType *array_of_packet_types,
                                   OsclMemoryFragment *array_of_packets)
{

    uint8 *ptr = (uint8 *) input_packet.ptr;
    int32 remaining_len = input_packet.len;

    filled_size = 0;

    if (! ptr || remaining_len <= 0)
    {
        return FAIL;
    }

    OsclBinIStreamBigEndian inStream;
    inStream.Attach(ptr, remaining_len);
    uint8 tempChar;

    while (remaining_len > 0 && filled_size < max_array_size)
    {

        inStream >> tempChar;
        if (inStream.fail())
        {
            return EOS_ON_READ;
        }

        // read the type
        uint8 payloadType;
        uint16 rtcpLength;
        inStream >> payloadType;
        inStream >> rtcpLength;
        if (inStream.fail())
        {
            return EOS_ON_READ;
        }

        // store the length and type
        switch (payloadType)
        {

            case SR_PACKET_TYPE:
                array_of_packet_types[filled_size] = SR_RTCP_PACKET;
                break;

            case RR_PACKET_TYPE:
                array_of_packet_types[filled_size] = RR_RTCP_PACKET;
                break;

            case SDES_PACKET_TYPE:
                array_of_packet_types[filled_size] = SDES_RTCP_PACKET;
                break;

            case BYE_PACKET_TYPE:
                array_of_packet_types[filled_size] = BYE_RTCP_PACKET;
                break;

            case APP_PACKET_TYPE:
            {
                // figure out whether this is a PVSS type
                // ptr is at the beginning of APP packet
                uint8* test_ptr = ptr + 8;
                if (oscl_memcmp(test_ptr, PVSS_APP_RTCP_NAME, 4))
                {
                    array_of_packet_types[filled_size] = APP_RTCP_PACKET;
                }
                else
                {
                    array_of_packet_types[filled_size] = PVSS_APP_RTCP_PACKET;
                }
                break;
            }


            default:
                array_of_packet_types[filled_size] = UNKNOWN_RTCP_PACKET;
                break;
        }

        // record the ptr and length
        array_of_packets[filled_size].ptr = ptr;
        array_of_packets[filled_size++].len = (rtcpLength + 1) * 4;

        remaining_len -= (rtcpLength + 1) * 4;

        ptr += (rtcpLength + 1) * 4;
        if (rtcpLength)
        {
            inStream.seekFromCurrentPosition((rtcpLength*4));
            if (inStream.fail())
            {
                return EOS_ON_READ;
            }
        }

    } // end while loop


    return RTCP_SUCCESS;

}




// Start of new code

/*
** Description:
**      Decode a sender or receiver report block
**
** Returns: SUCCESS if successful, FAIL if not.
** Side effects: None.
*/
RTCP_Decoder::Error_t RTCP_Decoder::DecodeReportBlock(
    OsclBinIStreamBigEndian & inStream,       /* Input stream reference */
    RTCP_ReportBlock* report
)
{
    if (! report)
    {
        return FAIL;
    }

    inStream >> report->sourceSSRC;
    const uint32 SIGN_BIT_MASK = 0x800000;
    const uint32 SIGN_EXTENSION = 0xFF000000;

    uint32 tempint32;
    inStream >> tempint32;
    report->fractionLost = (uint8)(tempint32 >> FRACTION_LOST_POSITION);
    report->cumulativeNumberOfPacketsLost =  tempint32 & FRACTION_LOST_MASK;
    if (report->cumulativeNumberOfPacketsLost & SIGN_BIT_MASK)
    {
        report->cumulativeNumberOfPacketsLost |= SIGN_EXTENSION;
    }

    inStream >> report->highestSequenceNumberReceived;
    inStream >> report->interarrivalJitter;

    inStream >> report->lastSR;
    inStream >> report->delaySinceLastSR;

    if (inStream.fail())
    {
        return FAIL;
    }
    else
    {
        return RTCP_SUCCESS;
    }
}



OSCL_EXPORT_REF RTCP_Decoder::Error_t
RTCP_Decoder::DecodeRR(OsclMemoryFragment& input_packet,
                       RTCP_RR& rr_packet)
{

#if DEBUG_PRINT
    printf("Within RTCP_Decoder::DecodeRR\n");
#endif

    // attach the bin stream
    OsclBinIStreamBigEndian inStream;

    if (input_packet.ptr == NULL ||
            input_packet.len == 0)
    {
        return FAIL;
    }

    inStream.Attach(input_packet.ptr, input_packet.len);
    uint8 tempChar;

    // decode the version, report count, packet type, and length
    inStream >> tempChar;
    if (inStream.eof() || inStream.fail())
    {
        return FAIL;
    }

    uint8 rcvdVersion = tempChar >> RTPRTCP_VERSION_BIT_POSITION;
    uint8 report_count = tempChar & RECORD_COUNT_MASK;
    if (rcvdVersion != rtcpVersion)
    {
#if DEBUG_PRINT
        printf("Wrong RR RTP version\n");
#endif
        return UNSUPPORTED_RTCP_VERSION;
    }

    uint8 payloadType;
    uint16 rtcpLength;
    inStream >> payloadType;
    inStream >> rtcpLength;
    if (inStream.fail())
    {
        return FAIL;
    }

    if (payloadType != RR_PACKET_TYPE)
    {
        return RTCP_PACKET_TYPE_MISMATCH;
    }

    if (rtcpLength < (report_count*6 + 1))
    {
        return RTCP_LENGTH_MISMATCH;
    }

    // set the number of report blocks
    rr_packet.set_max_report_blocks(report_count);

    // decode the sender SSRC
    inStream >> rr_packet.senderSSRC;

    Error_t status;
    // decode each of the report blocks
    for (uint ii = 0; ii < report_count; ++ii)
    {
        if ((status = DecodeReportBlock(inStream, rr_packet.get_report_block(ii))) != RTCP_SUCCESS)
        {
            return status;
        }
    }

    return RTCP_SUCCESS;
}

/*
** Description:
**      Decode a SR (sender report) RTCP_Decoder object.
**
** Returns: SUCCESS if successful, FAIL if not.
** Side effects: None.
*/
OSCL_EXPORT_REF RTCP_Decoder::Error_t RTCP_Decoder::DecodeSR(
    const OsclMemoryFragment& input_packet,
    RTCP_SR& sr_packet)
{

#if DEBUG_PRINT
    printf("Within RTCP_Decoder::DecodeSR\n");
#endif

    // attach the bin stream
    OsclBinIStreamBigEndian inStream;

    if (input_packet.ptr == NULL ||
            input_packet.len == 0)
    {
        return FAIL;
    }

    inStream.Attach(input_packet.ptr, input_packet.len);
    uint8 tempChar;

    // decode the version, report count, packet type, and length
    inStream >> tempChar;
    if (inStream.eof() || inStream.fail())
    {
        return FAIL;
    }

    uint8 rcvdVersion = tempChar >> RTPRTCP_VERSION_BIT_POSITION;
    uint8 report_count = tempChar & RECORD_COUNT_MASK;
    if (rcvdVersion != rtcpVersion)
    {
#if DEBUG_PRINT
        printf("Wrong SR RTCP version\n");
#endif
        return UNSUPPORTED_RTCP_VERSION;
    }

    // set the max report


    uint8 payloadType;
    uint16 rtcpLength;
    inStream >> payloadType;
    inStream >> rtcpLength;
    if (inStream.fail())
    {
        return FAIL;
    }

    if (payloadType != SR_PACKET_TYPE)
    {
        return RTCP_PACKET_TYPE_MISMATCH;
    }

    if (rtcpLength < (report_count*6 + 6))
    {
        return RTCP_LENGTH_MISMATCH;
    }

    // read the sender information
    // decode the sender SSRC
    inStream >> sr_packet.senderSSRC;
    inStream >> sr_packet.NTP_timestamp_high;
    inStream >> sr_packet.NTP_timestamp_low;
    inStream >> sr_packet.RTP_timestamp;
    inStream >> sr_packet.packet_count;
    inStream >> sr_packet.octet_count;

    if (inStream.fail())
    {
        return FAIL;
    }

    // set the number of report blocks
    sr_packet.set_max_report_blocks(report_count);


    Error_t status;
    // decode each of the report blocks
    for (uint ii = 0; ii < report_count; ++ii)
    {
        if ((status = DecodeReportBlock(inStream, sr_packet.get_report_block(ii))) != RTCP_SUCCESS)
        {
            return status;
        }
    }

    return RTCP_SUCCESS;
}


/*
** Description:
**      Decode a SDES RTCP_Decoder object. The only field supported and encoded is CNAME.
**
** Returns: SUCCESS if successful, FAIL if not.
** Side effects: None.
*/
OSCL_EXPORT_REF RTCP_Decoder::Error_t
RTCP_Decoder::DecodeSDES(
    const OsclMemoryFragment& input_packet,
    RTCP_SDES& sdes_packet)
{

    // attach the bin stream
    OsclBinIStreamBigEndian inStream;

    if (input_packet.ptr == NULL ||
            input_packet.len == 0)
    {
        return FAIL;
    }

    inStream.Attach(input_packet.ptr, input_packet.len);
    uint8 tempChar;

    // decode the version, report count, packet type, and length
    inStream >> tempChar;
    if (inStream.eof() || inStream.fail())
    {
        return FAIL;
    }

    uint8 rcvdVersion = tempChar >> RTPRTCP_VERSION_BIT_POSITION;
    uint8 chunk_count = tempChar & RECORD_COUNT_MASK;
    if (rcvdVersion != rtcpVersion)
    {
#if DEBUG_PRINT
        printf("Wrong RR RTP version\n");
#endif
        return UNSUPPORTED_RTCP_VERSION;
    }

    uint8 payloadType;
    uint16 rtcpLength;
    inStream >> payloadType;
    inStream >> rtcpLength;
    if (inStream.fail())
    {
        return FAIL;
    }

    if (payloadType != SDES_PACKET_TYPE)
    {
        return RTCP_PACKET_TYPE_MISMATCH;
    }


    sdes_packet.set_max_chunks(chunk_count);

    // now decode each chunk
    SDES_chunk* chunk_ptr;
    Error_t status;
    OsclMemoryFragment chunk_frag;
    for (uint ii = 0; ii < chunk_count; ++ii)
    {
        if (!(chunk_ptr = sdes_packet.get_chunk(ii)))
        {
            return FAIL;
        }
        chunk_frag.ptr = inStream.tellg() + ((int8*)input_packet.ptr);
        chunk_frag.len = input_packet.len - inStream.tellg();
        if ((status = DecodeSDESChunk(inStream, chunk_ptr, chunk_frag)) != RTCP_SUCCESS)
        {
            return status;
        }
    }

    return RTCP_SUCCESS;

}


RTCP_Decoder::Error_t
RTCP_Decoder::DecodeSDESChunk(OsclBinIStreamBigEndian & inStream,
                              SDES_chunk* sdes_chunk,
                              OsclMemoryFragment& chunk_data)
{
    // create a temporary for up to TMP_SDES_STORAGE sdes items.
    const uint TMP_SDES_STORAGE = 10;

    if (! sdes_chunk)
    {
        return FAIL;
    }

    uint8* ptr = (uint8 *)chunk_data.ptr;
    int32 len = chunk_data.len;
    uint8* end_ptr = ptr + len;

    SDES_item tmp_sdes_items[TMP_SDES_STORAGE];


    // get the ssrc
    inStream >> sdes_chunk->ssrc;
    len -= sizeof(sdes_chunk->ssrc);
    ptr += sizeof(sdes_chunk->ssrc);

    uint num_items = 0;
    // figure out the number of sdes items
    while ((ptr <= end_ptr - 2) && (*ptr != 0))
    {
        if (num_items < TMP_SDES_STORAGE)
        {
            // store the information
            tmp_sdes_items[num_items].type = *ptr++;
            uint8 item_len = *ptr++;
            len -= 2;
            if (item_len > len)
            {
                return EOS_ON_READ;
            }
            tmp_sdes_items[num_items].content.ptr = ptr;
            tmp_sdes_items[num_items].content.len = item_len;
            ptr += item_len;
            len -= item_len;
        }
        else
        {
            // simply skip over this one and count it.
            ++ptr;
            uint8 item_len = *ptr++;
            len -= 2;
            if (item_len > len)
            {
                return EOS_ON_READ;
            }
            ptr += item_len;
            len -= item_len;
        }

        ++num_items;
    }

    if (*ptr != 0)
    {
        return EOS_ON_READ;
    }


    // set the max number of sdes items
    sdes_chunk->set_max_items(num_items);

    // record the tmp SDES items
    uint loop_limit = (num_items <= TMP_SDES_STORAGE) ? num_items : TMP_SDES_STORAGE;

    SDES_item* sdes_item_ptr;
    uint ii;
    for (ii = 0; ii < loop_limit; ++ii)
    {
        if (!(sdes_item_ptr = sdes_chunk->get_item(ii)))
        {
            return FAIL;
        }
        *sdes_item_ptr = tmp_sdes_items[ii];
    }

    // record any remaining items beyond the temp storage size
    if ((loop_limit < num_items) && (ii < TMP_SDES_STORAGE))
    {
        uint8* cp_ptr = (uint8*) tmp_sdes_items[ii].content.ptr;
        uint8 cp_len = (uint8)(tmp_sdes_items[ii].content.len & 0xFF);

        cp_ptr += cp_len;
        len = end_ptr - cp_ptr;
        ii = loop_limit;
        while ((ii < num_items) && (cp_ptr <= end_ptr - 2) && (*cp_ptr != 0))
        {
            if (!(sdes_item_ptr = sdes_chunk->get_item(ii)))
            {
                return FAIL;
            }

            sdes_item_ptr->type = *cp_ptr++;
            uint8 item_len = *cp_ptr++;
            len -= 2;
            if (item_len > len)
            {
                return EOS_ON_READ;
            }
            sdes_item_ptr->content.ptr = cp_ptr;
            sdes_item_ptr->content.len = item_len;
            cp_ptr += item_len;
            len -= item_len;
            ++ii;
        }

    }

    // now skip over the padding
    // subtract off the SSRC length (even though it is
    len = ptr  - sizeof(sdes_chunk->ssrc) - (uint8 *)chunk_data.ptr;

    int32 pad_bytes = 4 - (len & 0x3);

    // move the inStream pos ahead
    inStream.seekFromCurrentPosition(len + pad_bytes);

    if (inStream.fail())
    {
        return FAIL;
    }

    return RTCP_SUCCESS;

}


/*
** Description:
**      Decode a RTCP BYE report.
**
** Returns: SUCCESS if successful, FAIL if not.
** Side effects: None.
*/
OSCL_EXPORT_REF RTCP_Decoder::Error_t
RTCP_Decoder::DecodeBYE(const OsclMemoryFragment& input_packet,
                        RTCP_BYE& bye_packet)
{

#if DEBUG_PRINT
    printf("Within RTCP_Decoder::DecodeBYE\n");
#endif

    // attach the bin stream
    OsclBinIStreamBigEndian inStream;

    if (input_packet.ptr == NULL ||
            input_packet.len == 0)
    {
        return FAIL;
    }

    inStream.Attach(input_packet.ptr, input_packet.len);
    uint8 tempChar;

    // decode the version, report count, packet type, and length
    inStream >> tempChar;
    if (inStream.eof() || inStream.fail())
    {
        return FAIL;
    }

    uint8 rcvdVersion = tempChar >> RTPRTCP_VERSION_BIT_POSITION;
    uint8 sourceCount = tempChar & RECORD_COUNT_MASK;
    if (rcvdVersion != rtcpVersion)
    {
#if DEBUG_PRINT
        printf("Wrong SR RTCP version\n");
#endif
        return UNSUPPORTED_RTCP_VERSION;
    }

    uint8 payloadType;
    uint16 rtcpLength;
    inStream >> payloadType;
    inStream >> rtcpLength;
    if (inStream.fail())
    {
        return FAIL;
    }


    if (payloadType != BYE_PACKET_TYPE)
    {
        return RTCP_PACKET_TYPE_MISMATCH;
    }

    bye_packet.src_count = sourceCount;
    for (uint ii = 0; ii < sourceCount; ++ii)
    {
        inStream >> bye_packet.ssrc_array[ii];
        if (inStream.fail())
        {
            return FAIL;
        }
    }

    // now check the reason string
    int32 len = inStream.PositionInBlock();

    bye_packet.reason_string.ptr = 0;
    bye_packet.reason_string.len = 0;

    if (len < (rtcpLength + 1)*4)
    {
        uint8 tmplen;
        inStream >> tmplen;
        if (inStream.fail())
        {
            return FAIL;
        }

        if (tmplen + len + 1 > (rtcpLength + 1)*4)
        {
            return EOS_ON_READ;
        }

        bye_packet.reason_string.ptr = ((uint8*) input_packet.ptr) + len + 1;
        bye_packet.reason_string.len = tmplen;

    }

    return RTCP_SUCCESS;
}

/*
** Description:
**      Decode a RTCP BYE report.
**
** Returns: SUCCESS if successful, FAIL if not.
** Side effects: None.
*/
OSCL_EXPORT_REF RTCP_Decoder::Error_t RTCP_Decoder::DecodeAPP(const OsclMemoryFragment& input_packet,
        RTCP_APP& app_packet)
{

    // attach the bin stream
    OsclBinIStreamBigEndian inStream;

    if (input_packet.ptr == NULL ||
            input_packet.len == 0)
    {
        return FAIL;
    }

    inStream.Attach(input_packet.ptr, input_packet.len);
    uint8 tempChar;

    // decode the version, report count, packet type, and length
    inStream >> tempChar;
    if (inStream.eof() || inStream.fail())
    {
        return FAIL;
    }

    uint8 rcvdVersion = tempChar >> RTPRTCP_VERSION_BIT_POSITION;
    uint8 padBit = ((tempChar & (1 << RTPRTCP_PAD_FLAG_BIT_POSITION)) != 0);
    uint8 subType = tempChar & RECORD_COUNT_MASK;
    if (rcvdVersion != rtcpVersion)
    {
#if DEBUG_PRINT
        printf("Wrong SR RTCP version\n");
#endif
        return UNSUPPORTED_RTCP_VERSION;
    }

    uint8 payloadType;
    uint16 rtcpLength;
    inStream >> payloadType;
    inStream >> rtcpLength;
    if (inStream.fail())
    {
        return FAIL;
    }


    if (payloadType != APP_PACKET_TYPE)
    {
        return RTCP_PACKET_TYPE_MISMATCH;
    }

    uint8* endPtr = (uint8*)input_packet.ptr + (rtcpLength + 1) * 4;
    if (padBit)
    {
        uint8 pad_size = *(endPtr - 1);
        endPtr -= pad_size;
        // must at least be 12 characters
        if (endPtr < (uint8*)input_packet.ptr + 12)
        {
            return FAIL;
        }
    }


    inStream >> app_packet.ssrc;
    inStream.get((int8 *) app_packet.type, 4);
    if (inStream.fail())
    {
        return FAIL;
    }


    app_packet.subtype = subType;
    if (oscl_memcmp(app_packet.type, PVSS_APP_RTCP_NAME, 4))
    {
        // some other app packet -- just record the memory ptr and length
        app_packet.app_data.ptr = ((uint8*)input_packet.ptr) + inStream.tellg();
        app_packet.app_data.len = endPtr - ((uint8*)app_packet.app_data.ptr);
        return RTCP_SUCCESS;
    }


    // store the subtype in the PVSS APP data structure also.
    app_packet.pvss_app_data.subtype = subType;

    // Check SubType
    if (subType > RTCP_PVSS_APP_MAX_SUPPORTED_SUBTYPE)
    {
#if DEBUG_PRINT
        printf("Unsupported APP SubType\n");
#endif
        inStream.seekFromCurrentPosition((rtcpLength - 2)*4);
        return UNSUPPORTED_RTCP_PVSS_APP;
    }

    int32 curpos = inStream.tellg();
    inStream >> app_packet.pvss_app_data.common.sendTime;
    inStream >> app_packet.pvss_app_data.common.recvRate;
    inStream >> app_packet.pvss_app_data.common.recvRateInterval;
    inStream >> app_packet.pvss_app_data.common.playbackBufDepth;
    inStream >> app_packet.pvss_app_data.common.highestCtrlMediaSeqNum;
    inStream >> app_packet.pvss_app_data.common.cumulativeBytes;
    if (inStream.fail())
    {
        return FAIL;
    }


    switch (subType)
    {
        case 0: // DRC
            inStream >> app_packet.pvss_app_data.extraDRC.rebufCount;
            inStream >> app_packet.pvss_app_data.extraDRC.missingPackets;
            inStream >> app_packet.pvss_app_data.extraDRC.cumulativePacketsReceived;
            inStream >> app_packet.pvss_app_data.extraDRC.totalProcessedFrames;
            inStream >> app_packet.pvss_app_data.extraDRC.totalSkippedFrames;
            inStream >> app_packet.pvss_app_data.extraDRC.cumulativePacketsLost;

            break;
        case 1: // BufLow
            inStream >> app_packet.pvss_app_data.extraBufLow.depletionRateInteger;
            inStream >> app_packet.pvss_app_data.extraBufLow.depletionRateFraction;

            break;
        case 2:  // BufHigh
            inStream >> app_packet.pvss_app_data.extraBufHigh.fillRateInteger;
            inStream >> app_packet.pvss_app_data.extraBufHigh.fillRateFraction;

            break;
        default:
            break;
    }

    if (inStream.fail())
    {
        return FAIL;
    }


    int32 diff = inStream.tellg() - curpos;

    diff = (rtcpLength - 2) * 4 - diff;
    if (diff < 0)
    {
        return FAIL;
    }
    else if (diff > 0)
    {
        inStream.seekFromCurrentPosition(diff);
    }

    return RTCP_SUCCESS;
}
