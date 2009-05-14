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
    This PVA_FF_SoundMediaHeaderAtom Class contains general presentation information,
    independent of coding, about the audio media within the stream.
*/


#define IMPLEMENT_SoundMediaHeaderAtom

#include "soundmediaheaderatom.h"
#include "atomutils.h"
#include "a_atomdefs.h"

// Constructor
PVA_FF_SoundMediaHeaderAtom::PVA_FF_SoundMediaHeaderAtom()
        : PVA_FF_MediaInformationHeaderAtom(SOUND_MEDIA_HEADER_ATOM, (uint8)0, (uint32)0) // version = 0!
{
    _reserved = 0;
    recomputeSize();
}

// Destructor
PVA_FF_SoundMediaHeaderAtom::~PVA_FF_SoundMediaHeaderAtom()
{
    // Empty
}

uint32
PVA_FF_SoundMediaHeaderAtom::getMediaInformationHeaderType() const
{
    return (uint32)MEDIA_INFORMATION_HEADER_TYPE_AUDIO;
}


// Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
bool
PVA_FF_SoundMediaHeaderAtom::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
{
    int32 rendered = 0;

    if (!renderAtomBaseMembers(fp))
    {
        return false;
    }
    rendered += getDefaultSize();

    // Render 32 bits of 0
    if (!PVA_FF_AtomUtils::render32(fp, _reserved))
    {
        return false;
    }
    rendered += 4;

    return true;
}



void
PVA_FF_SoundMediaHeaderAtom::recomputeSize()
{
    int32 size = getDefaultSize();
    size += 4; // for 32 bit reserved

    _size = size;
}

