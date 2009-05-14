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
    This PVA_FF_VideoMediaHeaderAtom Class contains general presentation information,
    independent of coding, about the visual media within the stream.
*/


#define IMPLEMENT_VideoMediaHeaderAtom

#include "videomediaheaderatom.h"
#include "atomutils.h"
#include "a_atomdefs.h"

// Constructor
PVA_FF_VideoMediaHeaderAtom::PVA_FF_VideoMediaHeaderAtom()
        : PVA_FF_MediaInformationHeaderAtom(VIDEO_MEDIA_HEADER_ATOM, (uint8)0, (uint32)1) // version=0 & flags=1!
{
    _reserved = 0;
    recomputeSize();
}

// Destructor
PVA_FF_VideoMediaHeaderAtom::~PVA_FF_VideoMediaHeaderAtom()
{
    // Empty
}

uint32
PVA_FF_VideoMediaHeaderAtom::getMediaInformationHeaderType() const
{
    return (uint32)MEDIA_INFORMATION_HEADER_TYPE_VISUAL;
}


// Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
bool
PVA_FF_VideoMediaHeaderAtom::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
{
    uint32 rendered = 0;

    if (!renderAtomBaseMembers(fp))
    {
        return false;
    }
    rendered += getDefaultSize();

    // Render 64 bits of 0
    if (!PVA_FF_AtomUtils::render32(fp, _reserved))
    {
        return false;
    }
    if (!PVA_FF_AtomUtils::render32(fp, _reserved))
    {
        return false;
    }
    rendered += 8;

    return true;
}




void
PVA_FF_VideoMediaHeaderAtom::recomputeSize()
{
    int32 size = getDefaultSize();
    size += 8; // for 64 bit reserved

    _size = size;
}

