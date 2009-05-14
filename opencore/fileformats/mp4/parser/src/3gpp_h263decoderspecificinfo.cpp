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

#define __IMPLEMENT_H263DecoderSpecificInfo3GPP_

#include "3gpp_h263decoderspecificinfo.h"

#include "atomutils.h"

H263SpecficAtom::H263SpecficAtom(MP4_FF_FILE *fp, uint32 size, uint32 type)
        : Atom(fp, size, type)
{
    _VendorCode = 0;
    _decoder_version = 0;
    _codec_profile = 0;
    _codec_level = 0;
    _averageBitRate = 0;
    _maxBitRate = 0;

    if (_success)
    {
        // Render decoder specific info payload
        if (!AtomUtils::read32(fp, _VendorCode))
        {
            _success = false;
            return;
        }

        if (!AtomUtils::read8(fp, _decoder_version))
        {
            _success = false;
            return;
        }

        if (!AtomUtils::read8(fp, _codec_level))
        {
            _success = false;
            return;
        }

        if (!AtomUtils::read8(fp, _codec_profile))
        {
            _success = false;
            return;
        }

        uint32 isH263BitrAtom = AtomUtils::peekNextNthBytes(fp, 2);

        if (isH263BitrAtom == H263_BITRATE_ATOM)
        {
            uint32 atomType = UNKNOWN_ATOM;
            uint32 atomSize = 0;

            AtomUtils::getNextAtomType(fp, atomSize, atomType);
            if (!AtomUtils::read32(fp, _averageBitRate))
            {
                _success = false;
                return;
            }
            if (!AtomUtils::read32(fp, _maxBitRate))
            {
                _success = false;
                return;
            }
        }
    }
    return;
}



