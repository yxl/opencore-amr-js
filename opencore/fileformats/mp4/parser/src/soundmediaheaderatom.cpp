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
/*                      MPEG-4 SoundMediaHeaderAtom Class                        */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This SoundMediaHeaderAtom Class contains general presentation information,
    independent of coding, about the audio media within the stream.
*/


#define IMPLEMENT_SoundMediaHeaderAtom

#include "soundmediaheaderatom.h"
#include "atomutils.h"
#include "atomdefs.h"

// Stream-in ctor
SoundMediaHeaderAtom::SoundMediaHeaderAtom(MP4_FF_FILE *fp, uint32 size, uint32 type)
        : MediaInformationHeaderAtom(fp, size, type)
{
    if (_success)
    {

        // Read the 4 bytes of reserved - defaulted in init()
        if (!AtomUtils::read32(fp, _reserved))
        {
            _success = false;
            _mp4ErrorCode = READ_SOUND_MEDIA_HEADER_ATOM_FAILED;
        }

    }
    else
    {
        if (_mp4ErrorCode != ATOM_VERSION_NOT_SUPPORTED)
            _mp4ErrorCode = READ_SOUND_MEDIA_HEADER_ATOM_FAILED;
    }
}

// Destructor
SoundMediaHeaderAtom::~SoundMediaHeaderAtom()
{
    // Empty
}

uint32
SoundMediaHeaderAtom::getMediaInformationHeaderType() const
{
    return (uint32)MEDIA_INFORMATION_HEADER_TYPE_AUDIO;
}





