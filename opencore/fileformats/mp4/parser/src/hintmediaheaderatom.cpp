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
/*                       MPEG-4 HintMediaHeaderAtom Class                        */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This HintMediaHeaderAtom Class contains general presentation information,
    independent of coding, about the audio media within the stream.
*/


#define IMPLEMENT_HintMediaHeaderAtom

#include "hintmediaheaderatom.h"
#include "atomutils.h"
#include "atomdefs.h"

// Stream-in ctor
HintMediaHeaderAtom::HintMediaHeaderAtom(MP4_FF_FILE *fp, uint32 size, uint32 type)
        : MediaInformationHeaderAtom(fp, size, type)
{
    if (_success)
    {

        _pparent = NULL;

        if (!AtomUtils::read16(fp, _maxPDUSize))
            _success = false;
        if (!AtomUtils::read16(fp, _avgPDUSize))
            _success = false;
        if (!AtomUtils::read32(fp, _maxBitrate))
            _success = false;
        if (!AtomUtils::read32(fp, _avgBitrate))
            _success = false;
        if (!AtomUtils::read32(fp, _slidingAvgBitrate))
            _success = false;

        if (!_success)
            _mp4ErrorCode = READ_HINT_MEDIA_HEADER_ATOM_FAILED;
    }
    else
    {
        if (_mp4ErrorCode != ATOM_VERSION_NOT_SUPPORTED)
            _mp4ErrorCode = READ_HINT_MEDIA_HEADER_ATOM_FAILED;
    }

}

// Destructor
HintMediaHeaderAtom::~HintMediaHeaderAtom()
{
    // Empty
}

uint32
HintMediaHeaderAtom::getMediaInformationHeaderType() const
{
    return (uint32)MEDIA_INFORMATION_HEADER_TYPE_HINT;
}


