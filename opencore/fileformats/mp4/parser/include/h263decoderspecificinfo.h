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
    H263DecoderSpecificInfo
*/

#ifndef H263DECODERSPECIFICINFO_H_INCLUDED
#define H263DECODERSPECIFICINFO_H_INCLUDED

#ifndef ATOMDEFS_H_INCLUDED
#include "atomdefs.h"
#endif

#ifndef DECODERSPECIFICINFO_H_INCLUDED
#include "decoderspecificinfo.h"
#endif


class H263DecoderSpecificInfo : public DecoderSpecificInfo
{

    public:
        H263DecoderSpecificInfo(MP4_FF_FILE *fp, bool o3GPPTrack = false); // Default constructor
        virtual ~H263DecoderSpecificInfo() {}; // Destructor

        uint32 getVendorCode()
        {
            return _VendorCode;
        }
        int8  getEncoderVersion()
        {
            return _codec_version;
        }
        int8  getCodecProfile()
        {
            return _codec_profile;
        }
        int8  getCodecLevel()
        {
            return _codec_level;
        }
        int16 getMaxWidth()
        {
            return _max_width;
        }
        int16 getMaxHeight()
        {
            return _max_height;
        }


        uint32       _VendorCode;
        uint8        _codec_version;
        uint8        _codec_profile;
        uint8        _codec_level;
        uint16       _max_width;
        uint16       _max_height;
};

#endif  // H263DECODERSPECIFICINFO_H_INCLUDED


