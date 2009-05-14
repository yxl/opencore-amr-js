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
#define IMPLEMENT_TrackFragmentHeaderAtom

#include "trackfragmentheaderatom.h"


// constructor
PVA_FF_TrackFragmentHeaderAtom::PVA_FF_TrackFragmentHeaderAtom(uint32 trackId)
        : PVA_FF_FullAtom(TRACK_FRAGMENT_HEADER_ATOM, (uint8)0, (uint32)FLAGS_TFHD_ATOM)
{
    _trackId = trackId;
    _baseDataOffset = 0;
    _sampleDescriptionIndex = 0;
    _defaultSampleDuration = 0;
    _defaultSampleSize = 0;
    _defaultSampleFlags = 0;

    recomputeSize();
}

// destructor
PVA_FF_TrackFragmentHeaderAtom::~PVA_FF_TrackFragmentHeaderAtom()
{
    // do nothing
}


// set header data
void
PVA_FF_TrackFragmentHeaderAtom::setHeaderData()
{
    // parameters to sample default parameters will be introduced later
}



// set base data offset
void
PVA_FF_TrackFragmentHeaderAtom::setBaseDataOffset(uint64 offset)
{
    _baseDataOffset = offset;
}



uint32
PVA_FF_TrackFragmentHeaderAtom::getBaseDataOffset()
{
    return (uint32)_baseDataOffset;
}

uint32
PVA_FF_TrackFragmentHeaderAtom::getTrackId()
{
    return _trackId;
}



// recompute size of atom
void
PVA_FF_TrackFragmentHeaderAtom::recomputeSize()
{
    int32 size = getDefaultSize();

    // add size various fields
    size += 4;	// track Id

    uint32 flags = getFlags();

    if (flags & 0x000001)
        size += 8;	// base data offset

    if (flags & 0x000002)
        size += 4;	// description index

    if (flags & 0x000008)
        size += 4;	// sample duration

    if (flags & 0x000010)
        size += 4;	// sample size

    if (flags & 0x000020)
        size += 4;	// sample flags

    _size = size;

    // Update the parent atom size
    if (_pparent != NULL)
    {
        _pparent->recomputeSize();
    }
}


// write atom to target file
bool
PVA_FF_TrackFragmentHeaderAtom::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP* fp)
{
    uint32 rendered = 0;

    if (!renderAtomBaseMembers(fp))
    {
        return false;
    }
    rendered += getDefaultSize();

    if (!PVA_FF_AtomUtils::render32(fp, _trackId))
    {
        return false;
    }
    rendered += 4;

    uint32 flags = getFlags();

    if (flags & 0x000001)
    {
        if (!PVA_FF_AtomUtils::render64(fp, _baseDataOffset))
        {
            return false;
        }
        rendered += 8;
    }

    if (flags & 0x000002)
    {
        if (!PVA_FF_AtomUtils::render32(fp, _sampleDescriptionIndex))
        {
            return false;
        }
        rendered += 4;
    }


    if (flags & 0x000008)
    {
        if (!PVA_FF_AtomUtils::render32(fp, _defaultSampleDuration))
        {
            return false;
        }
        rendered += 4;
    }

    if (flags & 0x000010)
    {
        if (!PVA_FF_AtomUtils::render32(fp, _defaultSampleSize))
        {
            return false;
        }
        rendered += 4;
    }

    if (flags & 0x000020)
    {
        if (!PVA_FF_AtomUtils::render32(fp, _defaultSampleFlags))
        {
            return false;
        }
        rendered += 4;
    }

    return true;
}

