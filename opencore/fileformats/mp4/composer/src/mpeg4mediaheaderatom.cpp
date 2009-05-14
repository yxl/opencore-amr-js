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
    This PVA_FF_Mpeg4MediaHeaderAtom Class fp for streams other than visual and audio.
*/

#define IMPLEMENT_Mpeg4MediaHeaderAtom

#include "mpeg4mediaheaderatom.h"
#include "a_atomdefs.h"
#include "atomutils.h"

// Constructor
PVA_FF_Mpeg4MediaHeaderAtom::PVA_FF_Mpeg4MediaHeaderAtom(uint32 mediaType)
        : PVA_FF_MediaInformationHeaderAtom(MPEG4_MEDIA_HEADER_ATOM, (uint8)0, (uint32)0)
{
    _mediaType = mediaType;
    // Currently UNsupported types include:
    // odhd - ObjectDescriptorStream
    // crhd - ClockReferenceStream
    // sdhd - SceneDescriptionStream
    // m7hd - Mpeg7Stream
    // ochd - ObjectContentInfoStream
    // iphd - IPMPStream
    // mjhd - Mpeg-JStream

    recomputeSize();
}

// Destructor
PVA_FF_Mpeg4MediaHeaderAtom::~PVA_FF_Mpeg4MediaHeaderAtom()
{
    // Empty
}

uint32
PVA_FF_Mpeg4MediaHeaderAtom::getMediaInformationHeaderType() const
{
    return _mediaType;
}

void
PVA_FF_Mpeg4MediaHeaderAtom::recomputeSize()
{
    _size = getDefaultSize();
}

// Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
bool
PVA_FF_Mpeg4MediaHeaderAtom::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
{
    int32 rendered = 0;

    if (!renderAtomBaseMembers(fp))
    {
        return false;
    }
    rendered += getDefaultSize();

    return true;
}


