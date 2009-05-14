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
**   File:   rtcp.h
**
**   Description:
**      This module defines the RTCP class. This class is used to encode and
**      decode RTCP packets. Please refer to the RTCP design document for
**      details.
*/

#ifndef RTCP_H
#define RTCP_H

/*
** Includes
*/
#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif
#ifndef OSCL_MEDIA_DATA_H
#include "oscl_media_data.h"
#endif
#ifndef RTPRTCP_H
#include "rtprtcp.h"
#endif

/*
** Constants
*/

const int32 RTCP_BYE_SSRC_ARRAY_SIZE = 31;
const int32 RTCP_MAX_REPORT_BLOCKS = 31;

// This number determines how many RTCP report blocks are
// preallocated and how many are allocated dynamically
const uint NUM_PREALLOCATED_RTCP_RR_REPORT_BLOCKS = 1;
const uint NUM_PREALLOCATED_RTCP_SR_REPORT_BLOCKS = 1;
const uint NUM_PREALLOCATED_RTCP_CHUNK_ITEMS = 1;
const uint NUM_PREALLOCATED_RTCP_CHUNKS = 1;


// SDES item types
const uint8 NULL_RTCP_SDES = 0;
const uint8 CNAME_RTCP_SDES = 1;
const uint8 USERNAME_RTCP_SDES = 2;
const uint8 EMAIL_RTCP_SDES = 3;
const uint8 PHONE_RTCP_SDES = 4;
const uint8 LOC_RTCP_SDES = 5;
const uint8 TOOL_RTCP_SDES = 6;
const uint8 NOTE_RTCP_SDES = 7;
const uint8 PRIV_RTCP_SDES = 8;

// PVSS APP Packet Subtypes
const char PVSS_APP_RTCP_NAME[] = "PVSS";
const char PSS0_APP_RTCP_NAME[] = "PSS0";
const uint8 RTCP_NADU_APP_SUBTYPE = 0;
const uint16 RTCP_NADU_APP_DEFAULT_PLAYOUT_DELAY = 0xFFFF;
const uint16 RTCP_NADU_APP_DEFAULT_NUN = 0;
const uint8 DRC_REPORT = 0;
const uint8 LOW_BUF_WARNING = 1;
const uint8 HIGH_BUF_WARNING = 2;
const int16 RTCP_PVSS_APP_MAX_SUPPORTED_SUBTYPE = 2; /* this should always
                                                      * equal the last value
                                                      * in the list above.
                                                      */

/*
 * The report block is the basic component structure of
 * the RTCP SR and RR
 *
 */
struct RTCP_ReportBlock
{
    uint32 sourceSSRC;
    uint8  fractionLost;
    int32  cumulativeNumberOfPacketsLost;
    uint32 highestSequenceNumberReceived;
    uint32 interarrivalJitter;
    uint32 lastSR;
    uint32 delaySinceLastSR;
};

enum RTCPPacketType
{
    SR_RTCP_PACKET,
    RR_RTCP_PACKET,
    SDES_RTCP_PACKET,
    PVSS_APP_RTCP_PACKET,
    APP_RTCP_PACKET,
    BYE_RTCP_PACKET,
    UNKNOWN_RTCP_PACKET
};



/*
 * This structure is used to hold the receiver report (RR) information
 *
 */
class RTCP_RR
{

    public:
        RTCP_RR(uint in_max_report_blocks = RTCP_MAX_REPORT_BLOCKS) : senderSSRC(0), num_report_blocks(0),
                max_report_blocks(in_max_report_blocks)
        {
            additional_reports = NULL;
        };


        ~RTCP_RR()
        {
            if (additional_reports)
            {
                delete[] additional_reports;
            }
        };

        OSCL_IMPORT_REF RTCP_ReportBlock* get_report_block(uint index);
        const RTCP_ReportBlock* read_report_block(uint index) const;

        bool set_max_report_blocks(uint new_max_report_blocks);

        bool set_report_block(uint index, const RTCP_ReportBlock& report_block);

        uint get_num_report_blocks() const
        {
            return num_report_blocks;
        };

        uint32 senderSSRC;

    private:
        uint num_report_blocks;
        uint max_report_blocks;
        RTCP_ReportBlock preallocated_reports[NUM_PREALLOCATED_RTCP_RR_REPORT_BLOCKS];
        RTCP_ReportBlock *additional_reports;

};


class RTCP_SR
{

    public:
        RTCP_SR(uint in_max_report_blocks = RTCP_MAX_REPORT_BLOCKS) : senderSSRC(0), num_report_blocks(0),
                max_report_blocks(in_max_report_blocks)
        {
            additional_reports = NULL;
        };


        ~RTCP_SR()
        {
            if (additional_reports)
            {
                delete[] additional_reports;
            }
        };

        RTCP_ReportBlock* get_report_block(uint index);
        const RTCP_ReportBlock* read_report_block(uint index) const;

        bool set_max_report_blocks(uint new_max_report_blocks);

        bool set_report_block(uint index, const RTCP_ReportBlock& report_block);

        uint get_num_report_blocks() const
        {
            return num_report_blocks;
        };

        uint32 senderSSRC;
        uint32 NTP_timestamp_high;
        uint32 NTP_timestamp_low;
        uint32 RTP_timestamp;
        uint32 packet_count;
        uint32 octet_count;

    private:
        uint num_report_blocks;
        uint max_report_blocks;
        RTCP_ReportBlock preallocated_reports[NUM_PREALLOCATED_RTCP_SR_REPORT_BLOCKS];
        RTCP_ReportBlock *additional_reports;

};



struct SDES_item
{
    uint8  type;
    OsclMemoryFragment content;

    SDES_item()
    {
        type = 0;
        content.ptr = NULL;
        content.len = 0;
    }

    ~SDES_item() {};
};

class SDES_chunk
{

    public:
        SDES_chunk(uint32 in_ssrc = 0): ssrc(in_ssrc),
                max_sdes_items(NUM_PREALLOCATED_RTCP_CHUNK_ITEMS), num_sdes_items(0),
                chunk_size(0)
        {
            additional_items = NULL;
        };
        ~SDES_chunk()
        {
            delete[] additional_items;
        };
        uint32 ssrc;

        void set_max_items(uint num_items);

        SDES_item* get_item(uint item_index);

        const SDES_item* read_item(uint item_index) const;

        uint get_num_items() const
        {
            return num_sdes_items;
        };

        bool add_item(const SDES_item& item);
        uint get_chunk_size() const
        {
            return chunk_size;
        };

    private:
        uint max_sdes_items;
        uint num_sdes_items;
        uint chunk_size;
        SDES_item chunk_items[NUM_PREALLOCATED_RTCP_CHUNK_ITEMS];
        SDES_item *additional_items;

};

const int32 RTCP_SDES_MAX_NUM_CHUNKS = 32;
class RTCP_SDES
{

    public:
        RTCP_SDES(): chunk_count(0),
                max_chunks(NUM_PREALLOCATED_RTCP_CHUNKS)
        {
            additional_chunks = NULL;
        };

        ~RTCP_SDES()
        {
            delete[] additional_chunks;
        };
        void set_max_chunks(uint new_max_chunks);

        SDES_chunk* get_chunk(uint item_index);

        const SDES_chunk* read_chunk(uint item_index) const;

        uint get_num_chunks() const
        {
            return chunk_count;
        };

        bool add_chunk(const SDES_chunk& chunk);


    private:
        uint chunk_count;
        uint max_chunks;

        SDES_chunk chunk_array[NUM_PREALLOCATED_RTCP_CHUNKS];
        SDES_chunk* additional_chunks;

};

struct RTCP_BYE
{
    uint8  src_count;
    uint32 ssrc_array[RTCP_BYE_SSRC_ARRAY_SIZE];
    OsclMemoryFragment reason_string;
};

struct APP_Common
{
    uint32 sendTime;
    uint32 recvRate;   // bits/sec
    uint32 recvRateInterval;
    uint32 playbackBufDepth;
    uint32 highestCtrlMediaSeqNum;
    uint32 cumulativeBytes;
};

struct APP_Extra_DRC
{
    uint16 rebufCount;
    uint16 missingPackets;
    uint32 cumulativePacketsReceived;
    uint32 totalProcessedFrames;
    uint32 totalSkippedFrames;
    uint32 cumulativePacketsLost;
};

struct APP_Extra_BufLow
{
    uint16 depletionRateInteger;
    uint16 depletionRateFraction;
};

struct APP_Extra_BufHigh
{
    uint16 fillRateInteger;
    uint16 fillRateFraction;
};


struct RTCP_PVSS_APP
{
    uint8 subtype;
    APP_Common common;
    union
    {
        APP_Extra_DRC extraDRC;
        APP_Extra_BufLow extraBufLow;
        APP_Extra_BufHigh extraBufHigh;
    };
};

struct RTCP_PSS0_APP
{
    uint32 sourcessrc;
    uint16 playoutdelayinms;
    uint16 nsn;
    uint16 nun;
    uint16 freebufferspace;
};

struct RTCP_APP
{
    uint8 subtype;
    uint32 ssrc;
    char type[4];
    union
    {
        RTCP_PVSS_APP pvss_app_data;
        RTCP_PSS0_APP pss0_app_data;
        OsclMemoryFragment app_data; // this for APPs other than "PVSS" & "PSS0"
    };
};



/*
** Classes
*/
class RTCP_Base
{
    public:
        RTCP_Base(const uint8 version);
        OSCL_IMPORT_REF virtual ~RTCP_Base();

    protected:
        uint8 rtcpVersion;
};

#endif
