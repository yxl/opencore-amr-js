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

#define __IMPLEMENT_AMRDecoderSpecificInfo3GPP__

#include "3gpp_amrdecoderspecificinfo.h"

#include "atom.h"
#include "atomutils.h"

const int16 AMRModeSetMask[8] =
{
    0x0001, 0x0002, 0x0004, 0x0008,
    0x0010, 0x0020, 0x0040, 0x0080,
};

const int32 AMRBitRates[8] =
{
    4750, 5150,  5900,  6700,
    7400, 7950, 10200, 12200,
};

// Default constructor
AMRSpecificAtom::AMRSpecificAtom(MP4_FF_FILE *fp, uint32 size, uint32 type)
        : Atom(fp, size, type)
{
    _bitRate = 0;

    if (_success)
    {
        AtomUtils::read32(fp, _VendorCode);

        AtomUtils::read8(fp, _decoder_version);

        AtomUtils::read16(fp, _mode_set);

        AtomUtils::read8(fp, _mode_change_period);

        AtomUtils::read8(fp, _frames_per_sample);

        for (uint32 i = 0; i < 8; i++)
        {
            if (_mode_set & AMRModeSetMask[i])
            {
                _bitRate = AMRBitRates[i];
            }
        }

    }
}


