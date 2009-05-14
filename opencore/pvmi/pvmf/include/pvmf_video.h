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
/**
 *  @file pvmf_video.h
 *  @brief This file defines structures/utilities specific to video
 *
 */

#ifndef PVMF_VIDEO_H_INCLUDED
#define PVMF_VIDEO_H_INCLUDED

#ifndef PV_UUID_H_INCLUDED
#include "pv_uuid.h"
#endif

#ifndef PVMF_FORMAT_TYPE_H_INCLUDED
#include "pvmf_format_type.h"
#endif

#define PVMF_VIDEO_INPUT_WIDTH 176
#define PVMF_VIDEO_INPUT_HEIGHT 144
#define PVMF_VIDEO_INPUT_FRAME_RATE 15
//#define VIDEO_INPUT_FORMAT PVMFVEN_INPUT_YUV420

const PVUid32 PVMFYuvFormatSpecificInfo0_UID = 0x1;
const PVUid32 PVMFEOSFormatSpecificInfo_UID = 0x2;
class PVMFYuvFormatSpecificInfo0
{
    public:
        PVMFYuvFormatSpecificInfo0()
        {
            uid = PVMFYuvFormatSpecificInfo0_UID;
            video_format = PVMF_MIME_FORMAT_UNKNOWN;
            display_width = 0;
            display_height = 0;
            width = 0;
            height = 0;
            num_buffers = 0;
            buffer_size = 0;
        };

        virtual ~PVMFYuvFormatSpecificInfo0() {};

        PVUid32 uid;
        PVMFFormatType video_format;
        uint32 display_width;
        uint32 display_height;
        uint32 width;
        uint32 height;

        uint32 num_buffers;
        uint32 buffer_size;
};

class PVMFVideoResolution
{
    public:
        PVMFVideoResolution(uint16 w, uint16 h) : width(w), height(h) {}
        uint16 width;
        uint16 height;
};

#define PVMF_RESOLUTION_NULL PVMFVideoResolution(0,0)
#define PVMF_RESOLUTION_SQCIF PVMFVideoResolution(128,96)
#define PVMF_RESOLUTION_QCIF PVMFVideoResolution(176,144)
#define PVMF_RESOLUTION_CIF PVMFVideoResolution(352,288)
#define PVMF_RESOLUTION_4CIF PVMFVideoResolution(704,576)
#define PVMF_RESOLUTION_16CIF PVMFVideoResolution(1408,1152)

class PVMFVideoResolutionRange
{
    public:
        PVMFVideoResolutionRange(PVMFVideoResolution first, PVMFVideoResolution last)
                : iFirst(first), iLast(last) {}
        bool isFit(PVMFVideoResolution res)
        {
            return (res.width >= iFirst.width && res.height >= iFirst.height && res.width <= iLast.width && res.height <= iLast.height);
        }
        PVMFVideoResolution iFirst;
        PVMFVideoResolution iLast;
};
#endif
