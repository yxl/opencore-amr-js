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
/*
    This PVA_FF_DecoderSpecificInfo Class that holds the Mpeg4 VOL header for the
	video stream
*/

#ifndef __H263DecoderSpecificInfo3GPP_H__
#define __H263DecoderSpecificInfo3GPP_H__

#include "atom.h"
#include "atomutils.h"
#include "a_atomdefs.h"

class PVA_FF_H263DecBitrateAtom : public PVA_FF_Atom
{
    public:
        PVA_FF_H263DecBitrateAtom();
        virtual ~PVA_FF_H263DecBitrateAtom() {};
        void setAvgBitrate(uint32 avg_bitrate)
        {
            _avg_bitrate = avg_bitrate;
        }
        void setMaxBitrate(uint32 max_bitrate)
        {
            _max_bitrate = max_bitrate;
        }

        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);
        virtual void recomputeSize();

    private:
        uint32 _avg_bitrate;
        uint32 _max_bitrate;
};


class PVA_FF_H263SpecficAtom : public PVA_FF_Atom
{

    public:
        PVA_FF_H263SpecficAtom(); // Default constructor
        virtual ~PVA_FF_H263SpecficAtom()
        {
            if (_ph263_decbitrateatom != NULL)
            {
                PV_MP4_FF_DELETE(NULL, PVA_FF_H263DecBitrateAtom , _ph263_decbitrateatom);
            }
        }; // Destructor

        void setVendorCode(int32 VendorCode = PACKETVIDEO_FOURCC)
        {
            _VendorCode = VendorCode;
        }
        void setEncoderVersion(uint8 encoder_version = 0)
        {
            _encoder_version = encoder_version;
        }
        void setCodecProfile(uint8 codec_profile = 0)
        {
            _codec_profile = codec_profile;
        }
        void setCodecLevel(uint8 codec_level = 10)
        {
            _codec_level = codec_level;
        }
        void setMaxWidth(uint16 max_width = 176)
        {
            _max_width = max_width;
        }
        void setMaxHeight(uint16 max_height = 144)
        {
            _max_height = max_height;
        }

        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);
        virtual void recomputeSize();

        PVA_FF_H263DecBitrateAtom * _ph263_decbitrateatom;
    private:
        int32   _VendorCode;
        uint8   _encoder_version;
        uint8   _codec_profile;
        uint8   _codec_level;
        uint16  _max_width;
        uint16  _max_height;

};

#endif

