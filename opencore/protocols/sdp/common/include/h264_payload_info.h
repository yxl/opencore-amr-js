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

#ifndef H264_PAYLOAD_INFO_H
#define H264_PAYLOAD_INFO_H

#include "payload_info.h"

#define PVMF_SDP_DEFAULT_H264_SAMPLE_RATE 90000

class H264PayloadSpecificInfoType : public PayloadSpecificInfoTypeBase
{
    public:
        H264PayloadSpecificInfoType(int payload)
        {
            payloadNumber = payload;

            max_mbps = -1;
            max_fs = -1;
            max_cpb = -1;
            max_dpb = -1;
            max_br = -1;
            redundant_pic_cap = -1;
            packetization_mode = -1;
            sprop_interleaving_depth = 0;
            deint_buf_cap = -1;
            sprop_deint_buf_req = -1;
            sprop_init_buf_time = -1;
            sprop_max_don_diff = -1;
            max_rcmd_nalu_size = -1;
        };

        inline void setMaxMbps(int val)
        {
            max_mbps = (int)val;
        };

        inline int getMaxMbps()
        {
            return max_mbps;
        };

        inline void setMaxFs(int val)
        {
            max_fs = (int)val;
        };

        inline int getMaxFs()
        {
            return max_fs;
        };

        inline void setMaxCpb(int val)
        {
            max_cpb = (int)val;
        };

        inline int getMaxCpb()
        {
            return max_cpb;
        };

        inline void setMaxDpb(int val)
        {
            max_dpb = (int)val;
        };

        inline int getMaxDpb()
        {
            return max_dpb;
        };

        inline void setMaxBr(int val)
        {
            max_br = (int)val;
        };

        inline int getMaxBr()
        {
            return max_br;
        };

        inline void setRedundantPicCap(int val)
        {
            redundant_pic_cap = (int)val;
        };

        inline int getRedundantPicCap()
        {
            return redundant_pic_cap;
        };

        inline void setPacketizationMode(int val)
        {
            packetization_mode = (int)val;
        };

        inline int getPacketizationMode()
        {
            return packetization_mode;
        };

        inline void setSpropInterleavingDepth(int val)
        {
            sprop_interleaving_depth = (int)val;
        };

        inline int getSpropInterleavingDepth()
        {
            return sprop_interleaving_depth;
        };

        inline void setDeintBufCap(int val)
        {
            deint_buf_cap = (int)val;
        };

        inline int getDeintBufCap()
        {
            return deint_buf_cap;
        };

        inline void setSpropDeintBufReq(int val)
        {
            sprop_deint_buf_req = (int)val;
        };

        inline int getSpropDeintBufReq()
        {
            return sprop_deint_buf_req;
        };

        inline void setSpropInitBufTime(int val)
        {
            sprop_init_buf_time = (int)val;
        };

        inline int getSpropInitBufTime()
        {
            return sprop_init_buf_time;
        };

        inline void setSpropMaxDonDiff(int val)
        {
            sprop_max_don_diff = (int)val;
        };

        inline int getSpropMaxDonDiff()
        {
            return sprop_max_don_diff;
        };

        inline void setMaxRcmdNaluSize(int val)
        {
            max_rcmd_nalu_size = (int)val;
        };

        inline int getMaxRcmdNaluSize()
        {
            return max_rcmd_nalu_size;
        };

    private:
        int max_mbps;
        int max_fs;
        int max_cpb;
        int max_dpb;
        int max_br;
        int redundant_pic_cap;
        int packetization_mode;
        int sprop_interleaving_depth;
        int deint_buf_cap;
        int sprop_deint_buf_req;
        int sprop_init_buf_time;
        int sprop_max_don_diff;
        int max_rcmd_nalu_size;
};

#endif
