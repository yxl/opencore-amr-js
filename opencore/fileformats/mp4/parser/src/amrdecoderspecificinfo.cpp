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
/*********************************************************************************/
/*     -------------------------------------------------------------------       */
/*                          MPEG-4 Util: AudioHintSample                         */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This DecoderSpecificInfo Class that holds the Mpeg4 VOL header for the
	video stream
*/

#define __IMPLEMENT_AMRDecoderSpecificInfo__

#include "amrdecoderspecificinfo.h"

#include "atom.h"
#include "atomutils.h"

// Default constructor
AMRDecoderSpecificInfo::AMRDecoderSpecificInfo(MP4_FF_FILE *fp, bool o3GPPTrack)
        : DecoderSpecificInfo(fp, o3GPPTrack, false)
{
    _VendorCode = 0;
    _codec_version = 0;
    _band_mode = 0;
    _frame_type = 0xFF;
    _mode_set = 0;
    _mode_change_period = 0;
    _frames_per_sample = 0;
    _mode_change_neighbour = 0;

    if (o3GPPTrack)
    {
    }
    else
    {
        AtomUtils::read32(fp, _VendorCode);

        AtomUtils::read8(fp, _codec_version);

        AtomUtils::read8(fp, _frame_type);

        AtomUtils::read16(fp, _mode_set);

        uint8 data = 0;

        AtomUtils::read8(fp, data);

        if (data & 0x01)
        {
            _mode_change_neighbour = true;
        }

        _mode_change_period = (uint8)((data >> 1) & 0x7F);

        AtomUtils::read8(fp, data);
        AtomUtils::read8(fp, data);
        AtomUtils::read8(fp, data);
    }
}


