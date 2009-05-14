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
#define IMPLEMENT_TrackFragmentRunAtom

#include "trackfragmentrunatom.h"
typedef Oscl_Vector<PVA_FF_TrackRunSample, OsclMemAllocator> PVA_FF_TrackRunSampleVecType;


// constructor
PVA_FF_TrackFragmentRunAtom::PVA_FF_TrackFragmentRunAtom()
        : PVA_FF_FullAtom(TRACK_RUN_ATOM, (uint8)0, (uint32)TRACK_RUN_ATOM_FLAGS)
{

    _currentTimestamp = 0;
    _firstEntry = true;

    _sampleCount = 0;
    _dataOffset = 0;
    _firstSampleFlags = 0;
    _lastTSupdated = false;

    // initialise vectors to store sample parameters
    PV_MP4_FF_NEW(fp->auditCB, PVA_FF_TrackRunSampleVecType, (), _psampleEntriesVec);

    recomputeSize();
}


// destructor
PVA_FF_TrackFragmentRunAtom::~PVA_FF_TrackFragmentRunAtom()
{
    // Delete sample parameter vectors
    PV_MP4_FF_TEMPLATED_DELETE(NULL, PVA_FF_TrackRunSampleVecType, Oscl_Vector, _psampleEntriesVec);

}

// will be used to set tf_flags and denotes sample parameter present
void
PVA_FF_TrackFragmentRunAtom::setFlags(uint32 flags)
{

    // currently not in use. only flags specified by TRACK_RUN_ATOM_FLAGS macro are present
    OSCL_UNUSED_ARG(flags);
}


void
PVA_FF_TrackFragmentRunAtom::setDataOffset(uint32 offset)
{
    _dataOffset = offset;
}


// add new sample entry
void
PVA_FF_TrackFragmentRunAtom::addSample(uint32 size, uint32 ts, uint8 flags)
{
    _sampleCount++;

    PVA_FF_TrackRunSample sampleEntry;
    sampleEntry.sampleDuration = 0;

    if (_firstEntry)
    {
        _currentTimestamp = ts;
        _firstEntry = false;
    }
    else
    {
        // Calculate delta
        int32 delta = ts - _currentTimestamp;
        _currentTimestamp = ts;

        uint32 size = _psampleEntriesVec->size();

        (*_psampleEntriesVec)[size-1].sampleDuration = delta;	// add sample duration for last sample.. same as second last sample
        // last sample duration added while rendering
    }
    //Store meta data params
    sampleEntry.sampleSize = size;

    uint8 codingType = (uint8)((flags >> 2) & 0x03);
    if (codingType == CODING_TYPE_I)
    {
        sampleEntry.sampleFlags = KEY_FRAME_ENTRY;
    }
    else
    {
        sampleEntry.sampleFlags = 0;
    }

    _psampleEntriesVec->push_back(sampleEntry);

    recomputeSize();
}


// recompute size of atom
void
PVA_FF_TrackFragmentRunAtom::recomputeSize()
{
    int32 size = getDefaultSize();

    size += 4;  // sample count;
    size += 4;	// data offset

    if (_psampleEntriesVec->size() != 0)
    {
        for (uint32 ii = 0; ii < _psampleEntriesVec->size(); ii++)
        {
            size += 4; // sample duration
            size += 4; // sample vector
            size += 4; // sample flags
        }
    }
    _size = size;

    // Update the parent atom size
    if (_pparent != NULL)
    {
        _pparent->recomputeSize();
    }
}



// update duration for last entry
void
PVA_FF_TrackFragmentRunAtom::updateLastTSEntry(uint32 ts)
{
    uint32 delta = ts - _currentTimestamp;
    uint32 size = _psampleEntriesVec->size();
    (*_psampleEntriesVec)[size -1].sampleDuration = delta;

    _lastTSupdated = true;
}



// write atom to target file
bool
PVA_FF_TrackFragmentRunAtom::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP* fp)
{
    uint32 rendered = 0;
    uint32 temp = 0;

    if (!renderAtomBaseMembers(fp))
    {
        return false;
    }
    rendered += getDefaultSize();

    if (!PVA_FF_AtomUtils::render32(fp, _sampleCount))
    {
        return false;
    }
    rendered += 4;

    if (!PVA_FF_AtomUtils::render32(fp, _dataOffset))
    {
        return false;
    }
    rendered += 4;

    // update last sample duration (for last MOOF only)
    if (!_lastTSupdated)
    {
        temp = (*_psampleEntriesVec)[ _sampleCount - 2].sampleDuration;
        (*_psampleEntriesVec)[ _sampleCount - 1].sampleDuration = temp;
    }

    if (_psampleEntriesVec->size() < _sampleCount)
    {
        return false;
    }
    for (uint32 ii = 0; ii < _sampleCount; ii++)
    {

        temp = (*_psampleEntriesVec)[ii].sampleDuration;
        if (!PVA_FF_AtomUtils::render32(fp, temp))
        {
            return false;
        }
        rendered += 4;

        temp = (*_psampleEntriesVec)[ii].sampleSize;
        if (!PVA_FF_AtomUtils::render32(fp, temp))
        {
            return false;
        }
        rendered += 4;

        temp = (*_psampleEntriesVec)[ii].sampleFlags;
        if (!PVA_FF_AtomUtils::render32(fp, temp))
        {
            return false;
        }
        rendered += 4;

    }

    return true;
}
