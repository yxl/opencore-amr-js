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
    This PVA_FF_TimeToSampleAtom Class contains a compact version of a table that allows
    indexing from decoding to sample number.
*/


#define IMPLEMENT_TimeToSampleAtom

#include "timetosampleatom.h"
#include "atomutils.h"
#include "a_atomdefs.h"

typedef Oscl_Vector<uint32, OsclMemAllocator> uint32VecType;
typedef Oscl_Vector<int32, OsclMemAllocator> int32VecType;
// Constructor
PVA_FF_TimeToSampleAtom::PVA_FF_TimeToSampleAtom(uint32 mediaType)
        : PVA_FF_FullAtom(TIME_TO_SAMPLE_ATOM, (uint8)0, (uint32)0),
        _mediaType(mediaType)
{
    // Initializing members and vectors
    _firstEntry = true;
    _entryCount = 0;
    _lastTSUpdated = false;		// used in movie fragment mode not to update table before rendering

    PV_MP4_FF_NEW(fp->auditCB, uint32VecType, (), _psampleCountVec);
    PV_MP4_FF_NEW(fp->auditCB, int32VecType, (), _psampleDeltaVec);

    recomputeSize();
}

// Destructor
PVA_FF_TimeToSampleAtom::~PVA_FF_TimeToSampleAtom()
{
    // DO CLEANUP OF VECTORS!!!
    PV_MP4_FF_TEMPLATED_DELETE(NULL, uint32VecType, Oscl_Vector, _psampleCountVec);
    PV_MP4_FF_TEMPLATED_DELETE(NULL, int32VecType, Oscl_Vector, _psampleDeltaVec);
}

void
PVA_FF_TimeToSampleAtom::nextSample(uint32 ts)
{
    switch (_mediaType)
    {
        case MEDIA_TYPE_AUDIO: // sample fp an IMediaSample
        case MEDIA_TYPE_VISUAL: // sample fp an IMediaSample
        case MEDIA_TYPE_TEXT:	// sample fp an IMediatextSample for timed text
        {
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

                // Add entry to table
                addDelta(delta);
            }
        }
        break;

        case MEDIA_TYPE_UNKNOWN:
        default:
            break;
    }
}

// in movie fragment mode set the actual duration of
// last sample
void
PVA_FF_TimeToSampleAtom::updateLastTSEntry(uint32 ts)
{
    if (((uint32) _mediaType == MEDIA_TYPE_AUDIO) ||
            ((uint32) _mediaType == MEDIA_TYPE_VISUAL))
    {
        int32 delta = ts - _currentTimestamp;
        addDelta(delta);
    }

    _lastTSUpdated = true;
}

// Add delta to the table - logic contained within if shoudl just update table entries
// or if should add new entries
void
PVA_FF_TimeToSampleAtom::addDelta(int32 delta)
{
    // Entries are calculated as difference between current ts and previous ts.  Therefore
    // the first entry to the table fp made when the second sample fp received
    if (_entryCount == 0)
    {
        // Add first delta entry
        addEntry(1, delta);
    }
    else
    {
        int32 lastDelta = (*_psampleDeltaVec)[_entryCount - 1];
        if (delta == lastDelta)
        {
            // Only need to replace count entry (increment it)
            uint32 count = (*_psampleCountVec)[_entryCount - 1];
            _psampleCountVec->pop_back();
            _psampleCountVec->push_back(count + 1); // incrementing count
        }
        else
        {
            // deltas differ - add new entry
            addEntry(1, delta);
        }
    }
}

// Add entry to the vector
void
PVA_FF_TimeToSampleAtom::addEntry(uint32 count, int32 delta)
{
    _psampleDeltaVec->push_back(delta);
    _psampleCountVec->push_back(count);
    _entryCount++;
    recomputeSize();
}


// Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
bool
PVA_FF_TimeToSampleAtom::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
{
    int32 rendered = 0;

    if (!renderAtomBaseMembers(fp))
    {
        return false;
    }
    rendered += getDefaultSize();

    // This is required to comply with w3850 Sec 13.2.3.16
    // "Note that the time to sample atoms must give durations for all
    // samples including the last one"
    if (_lastTSUpdated == false)
    {
        if (_entryCount > 0)
        {
            (*_psampleCountVec)[_entryCount - 1] += 1;
        }
    }

    if (!PVA_FF_AtomUtils::render32(fp, getEntryCount()))
    {
        return false;
    }
    rendered += 4;

    // Render the vectors of counts and deltas
    if ((_psampleCountVec->size() < _entryCount) ||
            (_psampleDeltaVec->size() < _entryCount))
    {
        return false;
    }
    for (uint32 i = 0; i < _entryCount; i++)
    {
        if (!PVA_FF_AtomUtils::render32(fp, (*_psampleCountVec)[i]))
        {
            return false;
        }
        if (!PVA_FF_AtomUtils::render32(fp, (*_psampleDeltaVec)[i]))
        {
            return false;
        }
        rendered += 8;
    }

    return true;
}


void
PVA_FF_TimeToSampleAtom::recomputeSize()
{
    // Include size of all base atom members
    int32 size = getDefaultSize();

    size += 4; // For entryCount

    // Inlclude size of entries in vectors
    for (uint32 i = 0; i < _entryCount; i++)
    {
        size += 8;
    }

    _size = size;

    // Update the size of the parent atom
    if (_pparent != NULL)
    {
        _pparent->recomputeSize();
    }
}
