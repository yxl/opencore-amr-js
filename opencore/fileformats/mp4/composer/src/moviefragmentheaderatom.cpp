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
#define IMPLEMENT_MovieFragmentHeaderAtom

#include "moviefragmentheaderatom.h"

// constructor
PVA_FF_MovieFragmentHeaderAtom::PVA_FF_MovieFragmentHeaderAtom(uint32 sequenceNumber)
        : PVA_FF_FullAtom(MOVIE_FRAGMENT_HEADER_ATOM, (uint8)0, (uint32)0)
{
    _sequenceNumber = sequenceNumber;

    recomputeSize();

}

// destructor
PVA_FF_MovieFragmentHeaderAtom::~PVA_FF_MovieFragmentHeaderAtom()
{
    // do nothing
}


// return sequence number
uint32
PVA_FF_MovieFragmentHeaderAtom::getSequenceNumber()
{
    return _sequenceNumber;
}


// recompute size of atom
void
PVA_FF_MovieFragmentHeaderAtom::recomputeSize()
{
    int32 size = getDefaultSize();

    size += 4; // For sequence number

    _size = size;

    // Update the parent atom size
    if (_pparent != NULL)
    {
        _pparent->recomputeSize();
    }
}


// write atom to target file
bool
PVA_FF_MovieFragmentHeaderAtom::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP* fp)
{
    uint32 rendered = 0;

    if (!renderAtomBaseMembers(fp))
    {
        return false;
    }
    rendered += getDefaultSize();

    if (!PVA_FF_AtomUtils::render32(fp, _sequenceNumber))
    {
        return false;
    }
    rendered += 4;

    return true;
}

