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
    This PVA_FF_SyncSampleAtom Class provides a compact marking of the random access
    points within the stream.
*/


#define IMPLEMENT_SyncSampleAtom

#include "syncsampleatom.h"
#include "atomutils.h"
#include "a_atomdefs.h"

typedef Oscl_Vector<uint32, OsclMemAllocator> uint32VecType;

// Constructor
PVA_FF_SyncSampleAtom::PVA_FF_SyncSampleAtom()
        : PVA_FF_FullAtom(SYNC_SAMPLE_ATOM, (uint8)0, (uint32)0)
{
    _entryCount = 0;
    PV_MP4_FF_NEW(fp->auditCB, uint32VecType, (), _psampleNumbers);
    _nextSampleNumber = 0;

    recomputeSize();
}


// Destructor
PVA_FF_SyncSampleAtom::~PVA_FF_SyncSampleAtom()
{
    // Cleanup vector
    PV_MP4_FF_TEMPLATED_DELETE(NULL, uint32VecType, Oscl_Vector, _psampleNumbers);
}

// This atom maintains a vector of sample numbers that are random access point samples
// i.e. I-frames.  This information if passes in through a single byte flag that fp parsed
// to get the vop_coding_type
void
PVA_FF_SyncSampleAtom::nextSample(uint8 flags)
{
    // Flags Format:
    // | mtb (1) | layer_id (3) | coding_type (2) | ref_select_code (2) |

    uint8 codingType = (uint8)((flags >> 2) & 0x03);

    if (codingType == CODING_TYPE_I)  // Need to check if sample fp an I-frame, then add entry to vector
    {
        addSampleNumber(_nextSampleNumber);
    }
    _nextSampleNumber++; // Increment sample nuber regardless of whether
    // or not you add it to the vector
}

// Adding to and getting the sample number values
void
PVA_FF_SyncSampleAtom::addSampleNumber(uint32 sample)
{
    _psampleNumbers->push_back(sample);
    _entryCount += 1;
    recomputeSize();
}

// Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
bool
PVA_FF_SyncSampleAtom::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
{
    int32 rendered = 0;

    if (!renderAtomBaseMembers(fp))
    {
        return false;
    }
    rendered += getDefaultSize();

    if (!PVA_FF_AtomUtils::render32(fp, getEntryCount()))
    {
        return false;
    }
    rendered += 4;

    // Need to render in a 1-based index vector instead of a 0-based index array
    int32 ARRAY_OFFSET_1_BASE = 1;
    if (_psampleNumbers != NULL)
    {
        if (_psampleNumbers->size() < _entryCount)
        {
            return false;
        }
        for (uint32 i = 0; i < _entryCount; i++)
        {
            if (!PVA_FF_AtomUtils::render32(fp, (*_psampleNumbers)[i] + ARRAY_OFFSET_1_BASE))
            {
                return false;
            }
        }
    }
    rendered += 4 * _entryCount;

    return true;
}




void
PVA_FF_SyncSampleAtom::recomputeSize()
{
    int32 size = getDefaultSize();

    size += 4; // For entryCount
    size += 4 * _entryCount;

    _size = size;

    // Update size of parent atom
    if (_pparent != NULL)
    {
        _pparent->recomputeSize();
    }
}

