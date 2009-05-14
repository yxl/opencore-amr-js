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
    This PVA_FF_TrackHeaderAtom Class specifies the characteristics of a single MPEG-4
    track.
*/


#define IMPLEMENT_TrackHeaderAtom

#include "trackheaderatom.h"
#include "atomutils.h"
#include "a_atomdefs.h"

#define TIMED_TEXT_WIDTH 176
#define TIMED_TEXT_HEIGHT 177
// Constructor
PVA_FF_TrackHeaderAtom::PVA_FF_TrackHeaderAtom(int32 type, uint32 trackID, uint8 version, uint32 flags, uint32 fileAuthoringFlags)
        : PVA_FF_FullAtom(TRACK_HEADER_ATOM, version, flags)
{
    OSCL_UNUSED_ARG(fileAuthoringFlags);

    _trackID = trackID;
    _mediaType = type;
    _width = 176;
    _height = 144;
    init(type);
}



// Destructor
PVA_FF_TrackHeaderAtom::~PVA_FF_TrackHeaderAtom()
{
    // Empty
}

void
PVA_FF_TrackHeaderAtom::init(int32 type)
{
    PVA_FF_AtomUtils::setTime(_creationTime);
    PVA_FF_AtomUtils::setTime(_modificationTime);
    setTimeScale(0);

    _duration = 0;

    // Initialize the reserved words based on type
    _reserved1 = 0;
    _reserved2[0] = 0;
    _reserved2[1] = 0;
    _reserved2[2] = 0;
    _reserved3 = 0;
    _reserved4 = 0;
    _reserved5[0] = 0x00010000;
    _reserved5[1] = 0;
    _reserved5[2] = 0;
    _reserved5[3] = 0;
    _reserved5[4] = 0x00010000;
    _reserved5[5] = 0;
    _reserved5[6] = 0;
    _reserved5[7] = 0;
    _reserved5[8] = 0x40000000;
    _reserved6 = 0;
    _reserved7 = 0;

    if ((uint32) type == MEDIA_TYPE_AUDIO)  // Set flags for audio track
    {
        _reserved3 = 0x0100;
    }
    else if ((uint32) type == MEDIA_TYPE_VISUAL)  // Set flags for visual track
    {
        _reserved6 = (_width << 16); ;
        _reserved7 = (_height << 16);;
    }
    else if ((uint32) type == MEDIA_TYPE_TEXT)//added for timed text track height and width
    {
        _reserved6 = (TIMED_TEXT_WIDTH << 16);
        _reserved7 = (TIMED_TEXT_HEIGHT << 16);

    }
    else
    {
        // No other special flags
    }

    /*
     * Variables added to ensure that the total track duration includes the duration of the
     * last sample as well, which in our case fp same as the last but one.
     */
    _prevTS = 0;
    _deltaTS = 0;

    recomputeSize();
}

void PVA_FF_TrackHeaderAtom::setVideoWidthHeight(int16 width, int16 height)
{
    _width = width;
    _height = height;
    _reserved6 = (_width << 16); ;
    _reserved7 = (_height << 16);;
}

void
PVA_FF_TrackHeaderAtom::addSample(uint32 ts)
{
    // Need to verify that this fp in the movie timescale
    // Currenly ts for the media samples fp in terms of milliseconds
    // For rounding
    uint32 duration = (uint32)((getTimeScale() / 1000.0f) * ts + 0.5f);

    _deltaTS = duration - _prevTS;

    setDuration(duration);

    _prevTS = duration;
}

// in movie fragment mode set the actual duration of
// last sample
void
PVA_FF_TrackHeaderAtom::updateLastTSEntry(uint32 ts)
{
    uint32 duration = (uint32)((getTimeScale() / 1000.0f) * ts);
    setDuration(duration);
    _deltaTS = 0;		// this will not increase the total duration
    // in movie fragmement mode as done before rendering
}

void
PVA_FF_TrackHeaderAtom::recomputeSize()
{
    int32 size = getDefaultSize(); // From base class
    size += 4; // _creationTime;
    size += 4; // _modificationTime
    size += 4; // _trackID;
    size += 4; // _reserved1
    size += 4; // _duration;
    size += 60; // rest of reserved words

    _size = size;

    // Update the size of the parent atom since this child atom may have changed
    if (_pparent != NULL)
    {
        _pparent->recomputeSize();
    }
}

// Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream.
bool
PVA_FF_TrackHeaderAtom::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
{
    int32 rendered = 0; // Keep track of number of bytes rendered
    uint32 trackID = 0;
    // Render PVA_FF_Atom type and size
    if (!renderAtomBaseMembers(fp))
    {
        return false;
    }
    rendered += getDefaultSize();

    if (!PVA_FF_AtomUtils::render32(fp, getCreationTime()))
    {
        return false;
    }
    if (!PVA_FF_AtomUtils::render32(fp, getModificationTime()))
    {
        return false;
    }

    trackID = getTrackID();


    if (!PVA_FF_AtomUtils::render32(fp, trackID))
    {
        return false;
    }
    rendered += 12;

    if (!PVA_FF_AtomUtils::render32(fp, _reserved1))
    {
        return false;
    }
    rendered += 4;

    /*
     * To ensure that the total track duration includes the duration of the
     * last sample as well, which in our case is same as the last but one.
     */

    uint32 totalDuration = getDuration();
    if (!PVA_FF_AtomUtils::render32(fp, totalDuration))
    {
        return false;
    }
    rendered += 4;

    if (!PVA_FF_AtomUtils::render32(fp, _reserved2[0]))
    {
        return false;
    }
    if (!PVA_FF_AtomUtils::render32(fp, _reserved2[1]))
    {
        return false;
    }
    if (!PVA_FF_AtomUtils::render32(fp, _reserved2[2]))
    {
        return false;
    }
    if (!PVA_FF_AtomUtils::render16(fp, _reserved3))
    {
        return false;
    }
    if (!PVA_FF_AtomUtils::render16(fp, _reserved4))
    {
        return false;
    }
    if (!PVA_FF_AtomUtils::render32(fp, _reserved5[0]))
    {
        return false;
    }
    if (!PVA_FF_AtomUtils::render32(fp, _reserved5[1]))
    {
        return false;
    }
    if (!PVA_FF_AtomUtils::render32(fp, _reserved5[2]))
    {
        return false;
    }
    if (!PVA_FF_AtomUtils::render32(fp, _reserved5[3]))
    {
        return false;
    }
    if (!PVA_FF_AtomUtils::render32(fp, _reserved5[4]))
    {
        return false;
    }
    if (!PVA_FF_AtomUtils::render32(fp, _reserved5[5]))
    {
        return false;
    }
    if (!PVA_FF_AtomUtils::render32(fp, _reserved5[6]))
    {
        return false;
    }
    if (!PVA_FF_AtomUtils::render32(fp, _reserved5[7]))
    {
        return false;
    }
    if (!PVA_FF_AtomUtils::render32(fp, _reserved5[8]))
    {
        return false;
    }
    if (!PVA_FF_AtomUtils::render32(fp, _reserved6))
    {
        return false;
    }
    if (!PVA_FF_AtomUtils::render32(fp, _reserved7))
    {
        return false;
    }
    rendered += 60;

    return true;
}

