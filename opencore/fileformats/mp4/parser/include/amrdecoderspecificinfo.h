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
#ifndef AMRDECODERSPECIFICINFO_H_INCLUDED
#define AMRDECODERSPECIFICINFO_H_INCLUDED

#ifndef OSCL_STRING_CONTAINERS_H_INCLUDED
#include "oscl_string_containers.h"
#endif

#ifndef ATOMDEFS_H_INCLUDED
#include "atomdefs.h"
#endif
#ifndef DECODERSPECIFICINFO_H_INCLUDED
#include "decoderspecificinfo.h"
#endif


class AMRDecoderSpecificInfo : public DecoderSpecificInfo
{

    public:
        AMRDecoderSpecificInfo(MP4_FF_FILE *fp, bool o3GPPTrack = false); // Default constructor
        virtual ~AMRDecoderSpecificInfo() {}; // Destructor

        int32 getVendorcode()
        {
            return _VendorCode;
        }
        int8 getEncoderVersion()
        {
            return _codec_version;
        }
        uint8 getFrameType()
        {
            return (uint8)(_frame_type & 0x0f);
        }
        uint16 getModeSet()
        {
            return _mode_set;
        }
        uint8 getModeChangePeriod()
        {
            return _mode_change_period;
        }
        bool  getModeChangeNeighbour()
        {
            return _mode_change_neighbour;
        }
        uint8 getFramesPerSample()
        {
            return _frames_per_sample;
        }

        uint32        _VendorCode;
        uint8         _codec_version;
        uint8         _band_mode;
        uint8         _frame_type;
        uint16        _mode_set;
        uint8         _mode_change_period;
        bool          _mode_change_neighbour;
        uint8         _frames_per_sample;
};

#endif // AMRDECODERSPECIFICINFO_H_INCLUDED
