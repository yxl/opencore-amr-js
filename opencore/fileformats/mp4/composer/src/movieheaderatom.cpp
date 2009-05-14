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
    This PVA_FF_MovieHeaderAtom Class defines the overall media-independent information
    relevant to the MPEG-4 presentation as a whole.
*/


#define IMPLEMENT_MovieHeaderAtom

#include "movieheaderatom.h"
#include "atomutils.h"
#include "a_atomdefs.h"


// Constructor
PVA_FF_MovieHeaderAtom::PVA_FF_MovieHeaderAtom(uint8 version, uint32 flags, uint32 fileAuthoringFlags)
        : PVA_FF_FullAtom(MOVIE_HEADER_ATOM, version, flags)
{
    OSCL_UNUSED_ARG(fileAuthoringFlags);

    PVA_FF_AtomUtils::setTime(_creationTime); // Setting creating time (since 1/1/1904)
    PVA_FF_AtomUtils::setTime(_modificationTime); // Setting modification time
    _timeScale = DEFAULT_PRESENTATION_TIMESCALE;
    _duration = 0;
    _nextTrackID = INITIAL_TRACK_ID;

    /*
     * Variables added to ensure that the total track duration includes the duration of the
     * last sample as well, which in our case fp same as the last but one.
     */
    _prevTS = 0;
    _deltaTS = 0;

    recomputeSize();
}

// Desctuctor
PVA_FF_MovieHeaderAtom::~PVA_FF_MovieHeaderAtom()
{
    // Empty
}

// Recompute size of atom based on member variables, etc.
void
PVA_FF_MovieHeaderAtom::recomputeSize()
{
    int32 size = getDefaultSize(); // Default size of PVA_FF_FullAtom class
    size += sizeof(_creationTime); // Sizes of member variables
    size += sizeof(_modificationTime);
    size += sizeof(_timeScale);
    size += sizeof(_duration);

    size += 76; // Size of combined reserved words

    size += sizeof(_nextTrackID);

    _size = size;
}

// Get the next available track ID that can be added to this container
uint32
PVA_FF_MovieHeaderAtom::getNextTrackID() const
{
    return _nextTrackID;
}

// Find the next available trackID
uint32
PVA_FF_MovieHeaderAtom::findNextTrackID()
{
    return _nextTrackID++; // For now until need more sophisticated search for next
    // available track ID
}

void
PVA_FF_MovieHeaderAtom::addSample(uint32 ts)
{
    // Need to verify that this fp in the movie timescale
    // Currenly ts for the media samples fp in terms of milliseconds
    uint32 duration =
        (uint32)((getTimeScale() / 1000.0f) * ts + 0.5f); // For rounding

    if (duration > _duration)
    {
        _deltaTS = duration - _prevTS;

        setDuration(duration);

        _prevTS = duration;
    }
}

// Render atom to a file stream
bool
PVA_FF_MovieHeaderAtom::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
{
    int32 rendered = 0; // Keep track of number of bytes rendered

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
    if (!PVA_FF_AtomUtils::render32(fp, getTimeScale()))
    {
        return false;
    }

    /*
     * To ensure that the total track duration includes the duration of the
     * last sample as well, which in our case fp same as the last but one.
     */
    //uint32 totalDuration = getDuration() + _deltaTS;
    uint32 totalDuration = getDuration();
    if (!PVA_FF_AtomUtils::render32(fp, totalDuration))
    {
        return false;
    }
    rendered += 16;

    uint32 reserved = 0x00010000;
    if (!PVA_FF_AtomUtils::render32(fp, reserved))
    {
        return false;
    }
    rendered += 4;

    reserved = 0x0100;
    if (!PVA_FF_AtomUtils::render16(fp, (uint16)(reserved)))
    {
        return false;
    }
    rendered += 2;

    // const bit(32)[2]
    reserved = 0;
    if (!PVA_FF_AtomUtils::render16(fp, (uint16)(reserved)))
    {
        return false;
    }
    if (!PVA_FF_AtomUtils::render32(fp, reserved))
    {
        return false;
    }
    if (!PVA_FF_AtomUtils::render32(fp, reserved))
    {
        return false;
    }
    rendered += 10;

    // const bit(32)[9]
    reserved = 0x00010000;
    if (!PVA_FF_AtomUtils::render32(fp, reserved))
    {
        return false;
    }
    rendered += 4;

    reserved = 0;
    if (!PVA_FF_AtomUtils::render32(fp, reserved))
    {
        return false;
    }
    if (!PVA_FF_AtomUtils::render32(fp, reserved))
    {
        return false;
    }
    if (!PVA_FF_AtomUtils::render32(fp, reserved))
    {
        return false;
    }
    rendered += 12;

    reserved = 0x00010000;
    if (!PVA_FF_AtomUtils::render32(fp, reserved))
    {
        return false;
    }
    rendered += 4;

    reserved = 0;
    if (!PVA_FF_AtomUtils::render32(fp, reserved))
    {
        return false;
    }
    if (!PVA_FF_AtomUtils::render32(fp, reserved))
    {
        return false;
    }
    if (!PVA_FF_AtomUtils::render32(fp, reserved))
    {
        return false;
    }
    rendered += 12;

    reserved = 0x40000000;
    if (!PVA_FF_AtomUtils::render32(fp, reserved))
    {
        return false;
    }
    rendered += 4;

    // const bit(32)[6] reserved = 0
    reserved = 0;
    if (!PVA_FF_AtomUtils::render32(fp, reserved))
    {
        return false;
    }
    if (!PVA_FF_AtomUtils::render32(fp, reserved))
    {
        return false;
    }
    if (!PVA_FF_AtomUtils::render32(fp, reserved))
    {
        return false;
    }
    if (!PVA_FF_AtomUtils::render32(fp, reserved))
    {
        return false;
    }
    if (!PVA_FF_AtomUtils::render32(fp, reserved))
    {
        return false;
    }
    if (!PVA_FF_AtomUtils::render32(fp, reserved))
    {
        return false;
    }
    rendered += 24;

    // _nextTrackID
    if (!PVA_FF_AtomUtils::render32(fp, _nextTrackID))
    {
        return false;
    }
    rendered += 4;

    return true;
}

