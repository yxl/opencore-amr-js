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
/*                          MPEG-4 Util: H263DecoderSpecificInfo                 */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This DecoderSpecificInfo Class that holds the Mpeg4 VOL header for the
	video stream
*/

#define __IMPLEMENT_H263DecoderSpecificInfo__

#include "h263decoderspecificinfo.h"

#include "atomutils.h"

H263DecoderSpecificInfo::H263DecoderSpecificInfo(MP4_FF_FILE *fp, bool o3GPPTrack)
        : DecoderSpecificInfo(fp, o3GPPTrack, false)
{
    uint8 tmp;

    if (o3GPPTrack)
    {

    }
    else
    {
        AtomUtils::read32(fp, _VendorCode);

        AtomUtils::read8(fp, _codec_version);

        AtomUtils::read8(fp, _codec_profile);

        AtomUtils::read8(fp, _codec_level);

        AtomUtils::read8(fp, tmp);

        AtomUtils::read16(fp, _max_width);

        AtomUtils::read16(fp, _max_height);
    }

    return;
}


