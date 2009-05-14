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
#define IMPLEMENT_TrackExtendsAtom

#include "trackextendsatom.h"

// constructor
PVA_FF_TrackExtendsAtom::PVA_FF_TrackExtendsAtom(uint32 mediaType,
        int32 codecType,
        uint32 trackId)
        : PVA_FF_FullAtom(TRACK_EXTENDS_ATOM, (uint8)0, (uint32)0)
{
    _mediaType = mediaType;

    _codecType = codecType;

    _trackId  = trackId;

    _defaultSampleDescriptionIndex = 0;		// all flags are kept 0
    _defaultSampleDuration = 0;
    _defaultSampleSize = 0;
    _defaultSampleFlags = 0;

    recomputeSize();
}

//destructor
PVA_FF_TrackExtendsAtom::~PVA_FF_TrackExtendsAtom()
{
    // do nothing
}

// return trackId
uint32
PVA_FF_TrackExtendsAtom::getTrackId()
{
    return _trackId;
}

//set value of trackId
void
PVA_FF_TrackExtendsAtom::setTrackId(uint32 trackId)
{
    _trackId = trackId;
}


// recompute size of atom
void
PVA_FF_TrackExtendsAtom::recomputeSize()
{
    int32 size = getDefaultSize();

    size += 4; // For trackId
    size += 4; // For sample description
    size += 4; // For sample duration
    size += 4; // For sample size
    size += 4; // For sample flags

    _size = size;

    // Update the parent atom size
    if (_pparent != NULL)
    {
        _pparent->recomputeSize();
    }
}


// write atom in target file
bool
PVA_FF_TrackExtendsAtom::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP* fp)
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

    if (!PVA_FF_AtomUtils::render32(fp, _defaultSampleDescriptionIndex))
    {
        return false;
    }
    rendered += 4;

    if (!PVA_FF_AtomUtils::render32(fp, _defaultSampleDuration))
    {
        return false;
    }
    rendered += 4;

    if (!PVA_FF_AtomUtils::render32(fp, _defaultSampleSize))
    {
        return false;
    }
    rendered += 4;

    if (!PVA_FF_AtomUtils::render32(fp, _defaultSampleFlags))
    {
        return false;
    }
    rendered += 4;

    return true;
}
