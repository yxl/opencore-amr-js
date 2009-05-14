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
**   File:   rtcp_encoder.cpp
**
**   Description:
**      This module implements the RTCP class. This class is used to encode and
**      decode RTCP packets. Please refer to the RTCP design document for
**      details.
*/

/*
** Includes
*/

#include "oscl_time.h"
#include "oscl_mem.h"
#include "rtcp_encoder.h"
#include "rtcp_constants.h"


/*
** Constants
*/

/*
** Methods
*/

OSCL_EXPORT_REF RTCP_Encoder::RTCP_Encoder(const uint8 *cname, const int32 cname_len,
        uint32 ssrc,
        const uint8 version)
        : RTCP_Base(version), SSRC(ssrc)
{

    setCName(cname, cname_len);
}


void RTCP_Encoder::setCName(const uint8 * cname,
                            int32 cname_len)
{
    if (cname && cname_len)
    {
        cName_length = (cname_len < RTCP_ENCODER_MAX_CNAME_SIZE) ? cname_len :
                       RTCP_ENCODER_MAX_CNAME_SIZE;
        oscl_memcpy(cName, cname, cName_length);
    }
    else
    {
        oscl_memcpy(cName, "PVSS", 4);
        cName_length = 4;
    }
}

RTCP_Encoder::Error_t
RTCP_Encoder::EncodeReportBlock(OsclBinOStreamBigEndian & outStream,/* Input
                                                                  * stream
                                                                  * reference
                                                                  */
                                const RTCP_ReportBlock* report
                               )
{
    if (! report)
    {
        return FAIL;
    }

    outStream << report->sourceSSRC;
    const int32 MAX_CUMULATIVE_LOST = 0x7FFFFF;

    uint32 tempint32 = report->cumulativeNumberOfPacketsLost;

    if (report->cumulativeNumberOfPacketsLost > MAX_CUMULATIVE_LOST)
    {
        tempint32 = MAX_CUMULATIVE_LOST;
    }

    tempint32 &= FRACTION_LOST_MASK;

    tempint32 |= (report->fractionLost) << FRACTION_LOST_POSITION;

    outStream << tempint32;

    outStream << report->highestSequenceNumberReceived;
    outStream << report->interarrivalJitter;

    outStream << report->lastSR;
    outStream << report->delaySinceLastSR;

    if (outStream.fail())
    {
        return FAIL;
    }
    else
    {
        return RTCP_SUCCESS;
    }
}


RTCP_Encoder::Error_t
RTCP_Encoder::output_rtcp_header(uint8 packet_type,
                                 uint8 count_field,
                                 uint16 size,
                                 OsclBinOStreamBigEndian& outStream,
                                 bool pad_bit)
{

    uint8 tempChar = rtcpVersion << RTPRTCP_VERSION_BIT_POSITION;
    if (pad_bit)
    {
        tempChar |= RTCP_PAD_BIT_MASK;
    }
    // transform size into number of 32 bit words - 1
    size = (size / 4) - 1;

    tempChar |= count_field & RECORD_COUNT_MASK;
    outStream << tempChar;
    outStream << packet_type;
    outStream << size;
    if (outStream.fail())
    {
        return FAIL;
    }
    return RTCP_SUCCESS;
}

int32 RTCP_Encoder::GetEncodedSize(const RTCP_RR& rr_packet)
{
    uint num_report_blocks = rr_packet.get_num_report_blocks();
    int32 size = (num_report_blocks * RTCP_REPORT_BLOCK_SIZE) +
                 RTCP_HEADER_SIZE + RTCP_RR_SENDER_INFO_SIZE;

    return size;
}

int32 RTCP_Encoder::GetEncodedSize(const RTCP_SR& sr_packet)
{
    uint num_report_blocks = sr_packet.get_num_report_blocks();
    int32 size = (num_report_blocks * RTCP_REPORT_BLOCK_SIZE) +
                 RTCP_HEADER_SIZE + RTCP_SR_SENDER_INFO_SIZE;

    return size;
}

int32 RTCP_Encoder::GetEncodedSize(const RTCP_SDES& sdes_packet)
{
    uint num_chunks = sdes_packet.get_num_chunks();
    // now go through the chunks and get the sizes

    const SDES_chunk* chunk_ptr;
    int32 size = 0;
    for (uint ii = 0; ii < num_chunks; ++ii)
    {
        if ((chunk_ptr = sdes_packet.read_chunk(ii)) != NULL)
        {
            int32 chunk_size = chunk_ptr->get_chunk_size();
            // add enough for the SSRC, the NULL item, and any padding
            chunk_size += 8 - (chunk_size & 0x3);
            size += chunk_size;
        }

    }

    size += RTCP_HEADER_SIZE;

    return size;
}

int32 RTCP_Encoder::GetEncodedSize(const RTCP_APP& app_packet)
{
    int32 size;

    if (oscl_memcmp(app_packet.type, PVSS_APP_RTCP_NAME, 4))
    {
        // this is a non PVSS APP packet
        int32 app_data_size = sizeof(RTCP_PSS0_APP);
        if (app_data_size % 4)
        {
            app_data_size += 4 - (app_data_size & 0x3);
        }
        size = RTCP_HEADER_SIZE + RTCP_APP_HEADER_INFO_SIZE +
               app_data_size;
    }
    else
    {
        size = RTCP_HEADER_SIZE + RTCP_APP_HEADER_INFO_SIZE +
               RTCP_PVSS_APP_COMMON_SIZE;

        switch (app_packet.pvss_app_data.subtype)
        {
            case DRC_REPORT:
            {
                size += RTCP_PVSS_APP_DRC_SIZE;
                break;
            }

            case LOW_BUF_WARNING:
            {
                size += RTCP_PVSS_APP_BUFLOW_SIZE;
                break;
            }

            case HIGH_BUF_WARNING:
            {
                size += RTCP_PVSS_APP_BUFHIGH_SIZE;
                break;
            }
        }
    }

    return size;
}



RTCP_Encoder::Error_t RTCP_Encoder::EncodeRR(const RTCP_RR& rr_packet,
        OsclMemoryFragment& output_buffer,
        uint8 pad_length)
{
    OsclBinOStreamBigEndian outStream;

    outStream.Attach(1, &output_buffer);

    // figure out how many bytes will be needed
    uint num_report_blocks = rr_packet.get_num_report_blocks();
    uint size = (num_report_blocks * RTCP_REPORT_BLOCK_SIZE) +
                RTCP_HEADER_SIZE + RTCP_RR_SENDER_INFO_SIZE + pad_length;

    if ((size & 0x3) != 0)
    {
        // improper pad length -- the packet length must be multiple of 4
        return INVALID_PAD_LENGTH;
    }

    if (output_buffer.len < size)
    {
        output_buffer.ptr = 0;
        output_buffer.len = size;
        return OUTPUT_TRUNCATED;
    }

    Error_t status;
    if ((status = output_rtcp_header(RR_PACKET_TYPE,
                                     num_report_blocks,
                                     size, outStream,
                                     (pad_length != 0))) !=
            RTCP_SUCCESS)
    {
        return status;
    }

    // output the ssrc
    outStream << rr_packet.senderSSRC;
    if (outStream.fail())
    {
        return FAIL;
    }

    // output the report blocks
    for (uint ii = 0; ii < num_report_blocks; ++ii)
    {
        if ((status = EncodeReportBlock(outStream, rr_packet.read_report_block(ii))) != RTCP_SUCCESS)
        {
            return status;
        }

    }

    // output any pad bits
    if (pad_length)
    {
        uint8 pad_value = 0;
        for (int32 ii = 0; ii < pad_length - 1; ++ii)
        {
            outStream << pad_value;
        }
        outStream << pad_length;
        if (outStream.fail())
        {
            return FAIL;
        }
    }

    output_buffer.len = outStream.tellg();

    return RTCP_SUCCESS;
}


RTCP_Encoder::Error_t RTCP_Encoder::EncodeSR(const RTCP_SR& sr_packet,
        OsclMemoryFragment& output_buffer,
        uint8 pad_length)
{
    OsclBinOStreamBigEndian outStream;

    outStream.Attach(1, &output_buffer);

    // figure out how many bytes will be needed
    uint num_report_blocks = sr_packet.get_num_report_blocks();
    uint size = (num_report_blocks * RTCP_REPORT_BLOCK_SIZE) +
                RTCP_HEADER_SIZE + RTCP_SR_SENDER_INFO_SIZE + pad_length;

    if ((size & 0x3) != 0)
    {
        // improper pad length -- the packet length must be multiple of 4
        return INVALID_PAD_LENGTH;
    }

    if (output_buffer.len < size)
    {
        output_buffer.ptr = 0;
        output_buffer.len = size;
        return OUTPUT_TRUNCATED;
    }

    Error_t status;
    if ((status = output_rtcp_header(SR_PACKET_TYPE, num_report_blocks,
                                     size, outStream,
                                     (pad_length != 0))) !=
            RTCP_SUCCESS)
    {
        return status;
    }

    // output the sender information
    outStream << sr_packet.senderSSRC;
    outStream << sr_packet.NTP_timestamp_high;
    outStream << sr_packet.NTP_timestamp_low;
    outStream << sr_packet.RTP_timestamp;
    outStream << sr_packet.packet_count;
    outStream << sr_packet.octet_count;

    if (outStream.fail())
    {
        return FAIL;
    }

    // output the report blocks
    for (uint ii = 0; ii < num_report_blocks; ++ii)
    {
        if ((status = EncodeReportBlock(outStream, sr_packet.read_report_block(ii))) != RTCP_SUCCESS)
        {
            return status;
        }

    }

    // output any pad bits
    if (pad_length)
    {
        uint8 pad_value = 0;
        for (int32 ii = 0; ii < pad_length - 1; ++ii)
        {
            outStream << pad_value;
        }
        outStream << pad_length;
        if (outStream.fail())
        {
            return FAIL;
        }
    }

    output_buffer.len = outStream.tellg();

    return RTCP_SUCCESS;
}



RTCP_Encoder::Error_t RTCP_Encoder::EncodeBYE(const RTCP_BYE& bye_packet,
        OsclMemoryFragment& output_buffer,
        uint8 pad_length)
{
    OsclBinOStreamBigEndian outStream;

    outStream.Attach(1, &output_buffer);

    // figure out how many bytes will be needed
    int32 ssrc_count = bye_packet.src_count & RECORD_COUNT_MASK;
    uint size = (ssrc_count * sizeof(bye_packet.ssrc_array[0])) +
                RTCP_HEADER_SIZE + pad_length;

    uint8 reason_len = (uint8)(bye_packet.reason_string.len & 0xFF);
    uint8 reason_len_pad = 0;
    if (bye_packet.reason_string.ptr && reason_len)
    {
        // figure out how many bytes in the reason string including any padding
        reason_len_pad = reason_len + 1;
        if (reason_len_pad & 0x3)
        {
            reason_len_pad += (4 - (reason_len_pad & 0x3));
            size += reason_len + reason_len_pad;
        }
    }

    if ((size & 0x3) != 0)
    {
        // improper pad length -- the packet length must be multiple of 4
        return INVALID_PAD_LENGTH;
    }

    if (output_buffer.len < size)
    {
        output_buffer.ptr = 0;
        output_buffer.len = size;
        return OUTPUT_TRUNCATED;
    }

    Error_t status;
    if ((status = output_rtcp_header(RR_PACKET_TYPE, ssrc_count,
                                     size, outStream,
                                     (pad_length != 0))) !=
            RTCP_SUCCESS)
    {
        return status;
    }

    for (int32 ii = 0; ii < ssrc_count; ++ii)
    {
        outStream << bye_packet.ssrc_array[ii];
    }

    if (outStream.fail())
    {
        return FAIL;
    }

    // output the reason string if any
    if (reason_len)
    {
        outStream << reason_len;
        outStream.write((int8*)bye_packet.reason_string.ptr,
                        reason_len);
        uint8 tmpchar = 0;
        for (int32 jj = 0; jj < reason_len_pad; ++jj)
        {
            outStream << tmpchar;
        }
        if (outStream.fail())
        {
            return FAIL;
        }

    }


    // output any pad bits
    if (pad_length)
    {
        uint8 pad_value = 0;
        for (int32 ii = 0; ii < pad_length - 1; ++ii)
        {
            outStream << pad_value;
        }
        outStream << pad_length;
        if (outStream.fail())
        {
            return FAIL;
        }
    }

    output_buffer.len = outStream.tellg();

    return RTCP_SUCCESS;
}


RTCP_Encoder::Error_t
RTCP_Encoder::EncodeSDESItem(OsclBinOStreamBigEndian& outStream,
                             const SDES_item* item_ptr)
{
    if (! item_ptr)
    {
        return FAIL;
    }

    outStream << item_ptr->type;

    uint8 len = (uint8)(item_ptr->content.len & 0xFF);  // limit to 255 characters

    outStream << len;

    outStream.write((int8*)item_ptr->content.ptr,
                    len);

    if (outStream.fail())
    {
        return FAIL;
    }

    return RTCP_SUCCESS;

}



RTCP_Encoder::Error_t
RTCP_Encoder::EncodeSDESChunk(OsclBinOStreamBigEndian& outStream,
                              const SDES_chunk* chunk_ptr)

{
    if (! chunk_ptr)
    {
        return FAIL;
    }

    outStream << chunk_ptr->ssrc;
    if (outStream.fail())
    {
        return FAIL;
    }

    // get the number of items
    uint num_items = chunk_ptr->get_num_items();

    // record the position
    uint32 pos = outStream.tellg();

    Error_t status;
    for (uint ii = 0; ii < num_items; ++ii)
    {
        if ((status = EncodeSDESItem(outStream, chunk_ptr->read_item(ii))) !=
                RTCP_SUCCESS)
        {
            return status;
        }
    }

    uint32 len = outStream.tellg() - pos;
    // Add the null item and any padding to make it to the next 32-bit boundary
    len = 4 - (len & 0x3);

    uint8 pad[4] = {0, 0, 0, 0};
    outStream.write((int8*)pad, len);

    if (outStream.fail())
    {
        return FAIL;
    }

    return RTCP_SUCCESS;

}



RTCP_Encoder::Error_t RTCP_Encoder::EncodeSDES(const RTCP_SDES& sdes_packet,
        OsclMemoryFragment& output_buffer,
        uint8 pad_length)
{
    OsclBinOStreamBigEndian outStream;

    outStream.Attach(1, &output_buffer);

    // figure out how many bytes will be needed
    uint num_chunks = sdes_packet.get_num_chunks();
    int32 size = GetEncodedSize(sdes_packet) + pad_length;

    if ((size & 0x3) != 0)
    {
        // improper pad length -- the packet length must be multiple of 4
        return INVALID_PAD_LENGTH;
    }

    Error_t status;
    if ((status = output_rtcp_header(SDES_PACKET_TYPE, num_chunks,
                                     size, outStream,
                                     (pad_length != 0))) !=
            RTCP_SUCCESS)
    {
        return status;
    }


    for (uint ii = 0; ii < num_chunks; ++ii)
    {
        if ((status = EncodeSDESChunk(outStream, sdes_packet.read_chunk(ii))) !=
                RTCP_SUCCESS)
        {
            return status;
        }
    }


    // output any pad bits
    if (pad_length)
    {
        uint8 pad_value = 0;
        for (int32 ii = 0; ii < pad_length - 1; ++ii)
        {
            outStream << pad_value;
        }
        outStream << pad_length;
        if (outStream.fail())
        {
            return FAIL;
        }
    }

    output_buffer.len = outStream.tellg();

    return RTCP_SUCCESS;

}




RTCP_Encoder::Error_t RTCP_Encoder::EncodeAPP(const RTCP_APP& app_packet,
        OsclMemoryFragment& output_buffer,
        uint8 pad_length)
{
    OsclBinOStreamBigEndian outStream;

    outStream.Attach(1, &output_buffer);

    // figure out how many bytes will be needed
    uint size = GetEncodedSize(app_packet) + pad_length;

    if ((size & 0x3) != 0)
    {
        // improper pad length -- the packet length must be multiple of 4
        return INVALID_PAD_LENGTH;
    }

    if (output_buffer.len < size)
    {
        output_buffer.ptr = 0;
        output_buffer.len = size;
        return OUTPUT_TRUNCATED;
    }

    Error_t status;
    if ((status = output_rtcp_header(APP_PACKET_TYPE, app_packet.subtype,
                                     size, outStream,
                                     (pad_length != 0))) !=
            RTCP_SUCCESS)
    {
        return status;
    }

    // output the ssrc and type
    outStream << app_packet.ssrc;
    outStream.write((int8*)app_packet.type, 4);


    if (oscl_memcmp(app_packet.type, PSS0_APP_RTCP_NAME, 4) == 0)
    {
        outStream << app_packet.pss0_app_data.sourcessrc;
        outStream << app_packet.pss0_app_data.playoutdelayinms;
        outStream << app_packet.pss0_app_data.nsn;
        outStream << app_packet.pss0_app_data.nun;
        outStream << app_packet.pss0_app_data.freebufferspace; //in 64 byte blocks
    }
    else if (oscl_memcmp(app_packet.type, PVSS_APP_RTCP_NAME, 4) == 0)
    {
        // output the PVSS common data
        outStream << app_packet.pvss_app_data.common.sendTime;
        outStream << app_packet.pvss_app_data.common.recvRate;   // bits/sec
        outStream << app_packet.pvss_app_data.common.recvRateInterval;
        outStream << app_packet.pvss_app_data.common.playbackBufDepth;
        outStream << app_packet.pvss_app_data.common.highestCtrlMediaSeqNum;
        outStream << app_packet.pvss_app_data.common.cumulativeBytes;

        switch (app_packet.pvss_app_data.subtype)
        {
            case DRC_REPORT:
            {
                outStream << app_packet.pvss_app_data.extraDRC.rebufCount;
                outStream << app_packet.pvss_app_data.extraDRC.missingPackets;
                outStream << app_packet.pvss_app_data.extraDRC.cumulativePacketsReceived;
                outStream << app_packet.pvss_app_data.extraDRC.totalProcessedFrames;
                outStream << app_packet.pvss_app_data.extraDRC.totalSkippedFrames;
                outStream << app_packet.pvss_app_data.extraDRC.cumulativePacketsLost;

                break;
            }

            case LOW_BUF_WARNING:
            {
                outStream << app_packet.pvss_app_data.extraBufLow.depletionRateInteger;
                outStream << app_packet.pvss_app_data.extraBufLow.depletionRateFraction;
                break;
            }

            case HIGH_BUF_WARNING:
            {
                outStream << app_packet.pvss_app_data.extraBufHigh.fillRateInteger;
                outStream << app_packet.pvss_app_data.extraBufHigh.fillRateFraction;

                break;
            }

        }
    }
    else
    {
        // this is not a PVSS APP packet or PSS0 APP packet
        outStream.write((int8*)app_packet.app_data.ptr,
                        app_packet.app_data.len);

        if (app_packet.app_data.len & 0x3)
        {
            // pad the data to the next 32-bit boundary
            int8 pad[4] = {0, 0, 0, 0};
            outStream.write(pad, 4 - (app_packet.app_data.len & 0x3));
        }
    }

    if (outStream.fail())
    {
        return FAIL;
    }

    // output any pad bits
    if (pad_length)
    {
        uint8 pad_value = 0;
        for (int32 ii = 0; ii < pad_length - 1; ++ii)
        {
            outStream << pad_value;
        }
        outStream << pad_length;
        if (outStream.fail())
        {
            return FAIL;
        }
    }

    output_buffer.len = outStream.tellg();

    return RTCP_SUCCESS;

}


OSCL_EXPORT_REF RTCP_Encoder::Error_t
RTCP_Encoder::EncodeCompoundRR(const RTCP_RR& rr_packet,
                               OsclMemoryFragment& output_buffer,
                               const RTCP_APP* app_packet,
                               uint8 pad_length)
{
    Error_t status;
    // encode the RR packet
    OsclMemoryFragment working_buffer = output_buffer;
    uint remaining_len = working_buffer.len;
    if ((status = EncodeRR(rr_packet, working_buffer)) != RTCP_SUCCESS)
    {
        return status;
    }

    working_buffer.ptr = (uint8*) working_buffer.ptr + working_buffer.len;
    remaining_len -= working_buffer.len;
    working_buffer.len = remaining_len;

    // encode the SDES packet
    SDES_item cname_item;
    cname_item.type = CNAME_RTCP_SDES;
    cname_item.content.ptr = cName;
    cname_item.content.len = cName_length;
    SDES_chunk chunk(SSRC);
    chunk.add_item(cname_item);
    RTCP_SDES sdes_packet;
    sdes_packet.add_chunk(chunk);
    uint8 sdes_pad = (app_packet) ? 0 : pad_length;
    if ((status = EncodeSDES(sdes_packet, working_buffer, sdes_pad)) != RTCP_SUCCESS)
    {
        return status;
    }

    working_buffer.ptr = (uint8*) working_buffer.ptr + working_buffer.len;
    remaining_len -= working_buffer.len;
    working_buffer.len = remaining_len;
    if (app_packet)
    {
        // encode the app
        if ((status = EncodeAPP(*app_packet, working_buffer, sdes_pad)) != RTCP_SUCCESS)
        {
            return status;
        }

        remaining_len -= working_buffer.len;
    }



    // update the length field
    output_buffer.len -= remaining_len;


    return RTCP_SUCCESS;
}


RTCP_Encoder::Error_t
RTCP_Encoder::EncodeCompoundSR(const RTCP_SR& sr_packet,
                               OsclMemoryFragment& output_buffer,
                               uint8 pad_length)
{
    Error_t status;
    // encode the SR packet
    OsclMemoryFragment working_buffer = output_buffer;
    uint remaining_len = working_buffer.len;
    if ((status = EncodeSR(sr_packet, working_buffer)) != RTCP_SUCCESS)
    {
        return status;
    }

    working_buffer.ptr = (uint8*) working_buffer.ptr + working_buffer.len;
    remaining_len -= working_buffer.len;
    working_buffer.len = remaining_len;

    // encode the SDES packet
    SDES_item cname_item;
    cname_item.type = CNAME_RTCP_SDES;
    cname_item.content.ptr = cName;
    cname_item.content.len = cName_length;
    SDES_chunk chunk(SSRC);
    chunk.add_item(cname_item);
    RTCP_SDES sdes_packet;
    sdes_packet.add_chunk(chunk);
    if ((status = EncodeSDES(sdes_packet, working_buffer, pad_length)) != RTCP_SUCCESS)
    {
        return status;
    }

    remaining_len -= working_buffer.len;

    // update the length field
    output_buffer.len -= remaining_len;

    return RTCP_SUCCESS;
}
