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
#ifndef GPP_H263DECODERSPECIFICINFO_H_INCLUDED
#define GPP_H263DECODERSPECIFICINFO_H_INCLUDED

#ifndef OSCL_STRING_CONTAINERS_H_INCLUDED
#include "oscl_string_containers.h"
#endif

#ifndef ATOM_H_INCLUDED
#include "atom.h"
#endif
#ifndef ATOMDEFS_H_INCLUDED
#include "atomdefs.h"
#endif


class H263SpecficAtom : public Atom
{

    public:
        H263SpecficAtom(MP4_FF_FILE *fp, uint32 size, uint32 type); // Default constructor
        virtual ~H263SpecficAtom() {};  // Destructor

        uint32 getVendorCode()
        {
            return _VendorCode;
        }
        int8   getDecoderVersion()
        {
            return _decoder_version;
        }
        int8   getCodecLevel()
        {
            return _codec_level;
        }
        int8   getCodecProfile()
        {
            return _codec_profile;
        }
        uint32 getAverageBitRate()
        {
            return _averageBitRate;
        }
        uint32 getMaxBitRate()
        {
            return _maxBitRate;
        }



    private:
        uint32       _VendorCode;
        uint8        _decoder_version;
        uint8        _codec_profile;
        uint8        _codec_level;
        uint32       _averageBitRate;
        uint32       _maxBitRate;

};

#endif // GPP_H263DECODERSPECIFICINFO_H_INCLUDED
