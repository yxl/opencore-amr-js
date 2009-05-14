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
    This PVA_FF_SampleSizeAtom Class contains the sample count and a table giving the
    size of each sample.
*/


#define IMPLEMENT_SampleToChunkAtom_H__

#include "sampletochunkatom.h"
#include "atomutils.h"
#include "a_atomdefs.h"


const int32 PVA_FF_SampleToChunkAtom::DEFAULT_MAX_NUM_SAMPLES_PER_CHUNK = 20;
const int32 PVA_FF_SampleToChunkAtom::DEFAULT_MAX_CHUNK_DATA_SIZE = 10240; // 10KB

typedef Oscl_Vector<uint32, OsclMemAllocator> uint32VecType;

// Constructor
PVA_FF_SampleToChunkAtom::PVA_FF_SampleToChunkAtom(uint32 mediaType,
        uint32 fileAuthoringFlags)
        : PVA_FF_FullAtom(SAMPLE_TO_CHUNK_ATOM, (uint8)0, (uint32)0),
        _mediaType(mediaType)
{
    _oInterLeaveMode = false;
    if (fileAuthoringFlags & PVMP4FF_SET_MEDIA_INTERLEAVE_MODE)
    {
        _oInterLeaveMode = true;
    }

    _currIndex = 0;
    _entryCount = 0;

    PV_MP4_FF_NEW(fp->auditCB, uint32VecType, (), _pfirstChunkVec);

    PV_MP4_FF_NEW(fp->auditCB, uint32VecType, (), _psamplesPerChunkVec);

    PV_MP4_FF_NEW(fp->auditCB, uint32VecType, (), _psampleDescriptionIndexVec);


    _currentChunkNumber = 0;
    _currentChunkDataSize = 0;
    _currentChunkNumSamples = 0;
    _maxNumSamplesPerChunk = DEFAULT_MAX_NUM_SAMPLES_PER_CHUNK;
    _maxChunkDataSize = DEFAULT_MAX_CHUNK_DATA_SIZE;

    recomputeSize();
}

PVA_FF_SampleToChunkAtom::~PVA_FF_SampleToChunkAtom()
{
    // Cleanup vectors
    PV_MP4_FF_TEMPLATED_DELETE(NULL, uint32VecType, Oscl_Vector, _pfirstChunkVec);
    PV_MP4_FF_TEMPLATED_DELETE(NULL, uint32VecType, Oscl_Vector, _psamplesPerChunkVec);
    PV_MP4_FF_TEMPLATED_DELETE(NULL, uint32VecType, Oscl_Vector, _psampleDescriptionIndexVec);
}
bool
PVA_FF_SampleToChunkAtom::nextSample(int32 index,
                                     uint32 size,
                                     bool oChunkStart)
{
    // Check to see if this sample fp the start of a new chunk
    bool newChunk;

    if (_oInterLeaveMode)
    {
        if (_pfirstChunkVec->size() == 0)
        {
            newChunk = true;
        }
        else
        {
            newChunk = oChunkStart;
        }
    }
    else
    {
        newChunk = isNewChunk(size, index);
    }

    if (newChunk)
    {
        if (_pfirstChunkVec->size() <= 1)
        {
            // Only zero or one run of chunks -
            // add new entry in table
            addChunkEntry(getNextChunkNumber(), (uint32)1, (uint32)index);
            // first sample in this chunk
            // Assuming ONLY 1 PVA_FF_SampleEntry FOR NOW!
        }
        else
        {
            // Check if samples per chunk for last entry is the same as the previous
            // entry - if so, last entry can be merged with previous one - then add
            // new entry (i.e. replace last entry with new entry)
            if ((((*_psamplesPerChunkVec)[uint32(_pfirstChunkVec->size()-1)]) ==
                    ((*_psamplesPerChunkVec)[uint32(_pfirstChunkVec->size()-2)])) &&
                    (((*_psampleDescriptionIndexVec)[uint32(_pfirstChunkVec->size()-1)]) ==
                     ((*_psampleDescriptionIndexVec)[uint32(_pfirstChunkVec->size()-2)])))

            {
                // SamplesPerChunk entries are the same - last chunk entry can be included
                // with the previous entry - Remove the last entry and add the entry for
                // the new chunk
                replaceLastChunkEntry(getNextChunkNumber(), (uint32)1, (uint32)index);
            }
            else
            {
                // Samples per chunk values differ - just add new entry
                addChunkEntry(getNextChunkNumber(), (uint32)1, (uint32)index);
            }
        }
    }
    else
    {
        // Just another sample in this chunk - update entry
        uint32 chunkNumber = _pfirstChunkVec->back();
        uint32 samples = _psamplesPerChunkVec->back();
        uint32 index = _psampleDescriptionIndexVec->back();

        samples += 1; // Increment the sample count for thic chunk

        // Update the entry for this chunk
        replaceLastChunkEntry(chunkNumber, samples, index);
    }

    return newChunk;

}

bool
PVA_FF_SampleToChunkAtom::isNewChunk(uint32 size, int32 index)
{
    if (index != _currIndex)
    {
        _currIndex = index;
        return true;
    }

    // This method uses three heuristics to determine when a sample fp the start of
    // a new chunk:
    // 1. Every I-frame fp the start of a new chunk
    // 2. A new chunk fp started when the current chunk size reaches a limit (in bytes)
    // 3. A new chunk fp started when the current number of samples in a chunk reaches
    //    a predefined limit (in number of samples)
    // Note that number 1. fp for VIDEO samples ONLY!
    // Also note that the first sample received fp also the start of a new chunk (degenerate case)

    // If this fp the first sample received - it fp the start of a new chunk
    // This corresponds to the vectors of size zero
    if (_pfirstChunkVec->size() == 0)
    {
        // First sample - new chunk
        _currentChunkNumSamples = 1;
        _currentChunkDataSize = size;
        return true;
    }

    // 3. Check for max num samples per chunk - if at max return true so this
    // sample fp the start of a new chunk
    if (_currentChunkNumSamples == _maxNumSamplesPerChunk)
    {
        // This sample starts a new chunk - thus 1 sample
        // Update size of chunk to be size of this sample
        _currentChunkNumSamples = 1;
        _currentChunkDataSize = size;
        return true;
    }
    else
    {
        // Not a new chunk so simply increment value
        _currentChunkNumSamples += 1;
    }

    // 2. Check for the actual size of each chunk - if at max return true so this
    // sample fp the start of a new chunk
    switch (_mediaType)
    {
        case MEDIA_TYPE_TEXT://added for timed text support
        case MEDIA_TYPE_AUDIO:
        case MEDIA_TYPE_VISUAL:
        {
            // Checking actual data size - if current chunk size plus this sample
            // fp greater than the max allowed chunk size, this sample fp the start
            // of a new chunk
            if (_currentChunkDataSize + size > _maxChunkDataSize)
            {
                // This sample starts a new chunk - thus 1 sample
                // Update size of chunk to be size of this sample
                _currentChunkNumSamples = 1;
                _currentChunkDataSize = size;
                return true;
            }
            else
            {
                _currentChunkDataSize += size;
            }
        }
        break;
        case MEDIA_TYPE_UNKNOWN:
        default:
            break;
    }

    // Default case - start a new chunk
    return false;
}


// Adding to and getting first chunk, samples per chunk, and sample
// description index values
void
PVA_FF_SampleToChunkAtom::addChunkEntry(uint32 chunk, uint32 samples, uint32 index)
{
    _pfirstChunkVec->push_back(chunk);
    _psamplesPerChunkVec->push_back(samples);
    _psampleDescriptionIndexVec->push_back(index);
    _entryCount += 1;

    recomputeSize();
}

// Updates the last chunk entry by incrementing the samples
void
PVA_FF_SampleToChunkAtom::replaceLastChunkEntry(uint32 chunk, uint32 samples, uint32 index)
{
    _pfirstChunkVec->pop_back();
    _psamplesPerChunkVec->pop_back();
    _psampleDescriptionIndexVec->pop_back();

    _pfirstChunkVec->push_back(chunk);
    _psamplesPerChunkVec->push_back(samples);
    _psampleDescriptionIndexVec->push_back(index);
}


// Returns the samples description index for the samples in all the chunks in run[index]
int32
PVA_FF_SampleToChunkAtom::getSampleDescriptionIndexAt(uint32 index)
{
    if (index < _psampleDescriptionIndexVec->size())
    {
        return (*_psampleDescriptionIndexVec)[index];
    }
    else
    {
        return PV_ERROR;
    }
}


// Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
bool
PVA_FF_SampleToChunkAtom::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
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
    if ((_pfirstChunkVec->size() < _entryCount) ||
            (_psamplesPerChunkVec->size() < _entryCount) ||
            (_psampleDescriptionIndexVec->size() < _entryCount))
    {
        return false;
    }
    for (uint32 i = 0; i < _entryCount; i++)
    {
        if (!PVA_FF_AtomUtils::render32(fp, (*_pfirstChunkVec)[i] + ARRAY_OFFSET_1_BASE))
        {
            return false;
        }
        if (!PVA_FF_AtomUtils::render32(fp, (*_psamplesPerChunkVec)[i]))
        {
            return false;
        }
        if (!PVA_FF_AtomUtils::render32(fp, (*_psampleDescriptionIndexVec)[i]))
        {
            return false;
        }
    }
    rendered += 12 * _entryCount;

    return true;
}

void
PVA_FF_SampleToChunkAtom::recomputeSize()
{
    int32 size = getDefaultSize();
    size += 4; // For entryCount

    size += 12 * _entryCount; // For each of the 3 4-byte entries

    _size = size;

    // Update size of parent atom
    if (_pparent != NULL)
    {
        _pparent->recomputeSize();
    }
}
