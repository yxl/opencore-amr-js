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
    This PVA_FF_HintMediaHeaderAtom Class contains general presentation information,
    independent of coding, about the audio media within the stream.
*/


#define IMPLEMENT_HintMediaHeaderAtom

#include "hintmediaheaderatom.h"
#include "atomutils.h"
#include "a_atomdefs.h"

// Constructor
PVA_FF_HintMediaHeaderAtom::PVA_FF_HintMediaHeaderAtom()
        : PVA_FF_MediaInformationHeaderAtom(HINT_MEDIA_HEADER_ATOM, (uint8)0, (uint32) 0) // version = 0!
{
    init();
    recomputeSize();
}


// Destructor
PVA_FF_HintMediaHeaderAtom::~PVA_FF_HintMediaHeaderAtom()
{
    // Empty
}

void
PVA_FF_HintMediaHeaderAtom::init()
{
    _maxPDUSize = 0;
    _avgPDUSize = 0;
    _maxBitrate = 0;
    _avgBitrate = 0;
    _slidingAvgBitrate = 0;
}

uint32
PVA_FF_HintMediaHeaderAtom::getMediaInformationHeaderType() const
{
    return (uint32)MEDIA_INFORMATION_HEADER_TYPE_HINT;
}


// Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
bool
PVA_FF_HintMediaHeaderAtom::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
{
    int32 rendered = 0;

    if (!renderAtomBaseMembers(fp))
    {
        return false;
    }
    rendered += getDefaultSize();

    // Render member variables
    if (!PVA_FF_AtomUtils::render16(fp, getMaxPDUSize()))
    {
        return false;
    }
    if (!PVA_FF_AtomUtils::render16(fp, getAvgPDUSize()))
    {
        return false;
    }
    rendered += 4;

    if (!PVA_FF_AtomUtils::render32(fp, getMaxBitrate()))
    {
        return false;
    }
    if (!PVA_FF_AtomUtils::render32(fp, getAvgBitrate()))
    {
        return false;
    }
    if (!PVA_FF_AtomUtils::render32(fp, getSlidingAvgBitrate()))
    {
        return false;
    }
    rendered += 12;

    return true;
}



void
PVA_FF_HintMediaHeaderAtom::recomputeSize()
{
    int32 size = getDefaultSize();
    size += 4; // for max and avf PDU sizes
    size += 12; // for max, avg, and sliding bitrates

    _size = size;
}

