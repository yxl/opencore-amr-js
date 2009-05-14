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


#define IMPLEMENT_SampleSizeAtom_H__

#include "samplesizeatom.h"
#include "atomutils.h"
#include "a_atomdefs.h"

typedef Oscl_Vector<uint32, OsclMemAllocator> uint32VecType;

// Constructor
PVA_FF_SampleSizeAtom::PVA_FF_SampleSizeAtom(uint32 mediaType)
        : PVA_FF_FullAtom(SAMPLE_SIZE_ATOM, (uint8)0, (uint32)0),
        _mediaType(mediaType)
{
    _sampleCount = 0;
    _sampleSize = 0;
    PV_MP4_FF_NEW(fp->auditCB, uint32VecType, (), _psampleSizeVec);

    recomputeSize();
}


// Destructor
PVA_FF_SampleSizeAtom::~PVA_FF_SampleSizeAtom()
{
    // CLEAN UP VECTOR
    PV_MP4_FF_TEMPLATED_DELETE(NULL, uint32VecType, Oscl_Vector, _psampleSizeVec);
}


void
PVA_FF_SampleSizeAtom::nextSample(uint32 size)
{
    switch (_mediaType)
    {
        case MEDIA_TYPE_TEXT://added for timed text support
        case MEDIA_TYPE_AUDIO:
        case MEDIA_TYPE_VISUAL:
            addNextSampleSize(size);
            break;
        case MEDIA_TYPE_UNKNOWN:
        default:
            break;
    }
}

void
PVA_FF_SampleSizeAtom::addNextSampleSize(uint32 size)
{
    if (_psampleSizeVec->size() == 0)  // empty vector - either no samples or all samples same size
    {
        if (_sampleSize != size)  // New size different from existing
        {
            if (_sampleCount == 0)  // New sample fp actually FIRST sample
            {
                _sampleSize = size;
                _sampleCount += 1;
            }
            else   // New sample differs from all other sample that were the same size
            {
                // Build vector of existing sample sizes, then add new one
                for (int32 i = 0; i < (int32)_sampleCount; i++)
                {
                    _psampleSizeVec->push_back(_sampleSize);
                }
                _sampleSize = 0; // Flag indicating that there fp a vector of sizes
                _psampleSizeVec->push_back(size);
                _sampleCount += 1; // Increment for new sample
            }
        }
        else   // New size same as all others - simple incrememt count
        {
            _sampleCount += 1; // Increment for new sample
        }
    }
    else   // elements in vector - i.e. differing sample sizes
    {
        // all samples listed in vector - just add to vector
        _psampleSizeVec->push_back(size);
        _sampleCount += 1;
    }

    recomputeSize();
}

// Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
bool
PVA_FF_SampleSizeAtom::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
{
    int32 rendered = 0;

    if (!renderAtomBaseMembers(fp))
    {
        return false;
    }
    rendered += getDefaultSize();

    // Render sampleSize and sampleCount
    if (!PVA_FF_AtomUtils::render32(fp, getDefaultSampleSize()))
    {
        return false;
    }
    if (!PVA_FF_AtomUtils::render32(fp, getSampleCount()))
    {
        return false;
    }
    rendered += 8;

    if (getDefaultSampleSize() == 0)
    {
        if (_psampleSizeVec->size() < getSampleCount())
        {
            return false;
        }
        for (int32 i = 0; i < (int32)(getSampleCount()); i++)
        {
            if (!PVA_FF_AtomUtils::render32(fp, (*_psampleSizeVec)[i]))
            {
                return false;
            }
        }
        rendered += 4 * getSampleCount();
    }

    return true;
}




void
PVA_FF_SampleSizeAtom::recomputeSize()
{
    int32 size = getDefaultSize();

    size += 4; // For sampleSize
    size += 4; // For sampleCount
    if (_sampleSize == 0)
    {
        size += 4 * _sampleCount; // For all elements in vec
    }

    _size = size;

    // Update the parent atom size
    if (_pparent != NULL)
    {
        _pparent->recomputeSize();
    }
}

bool
PVA_FF_SampleSizeAtom::reAuthorFirstSample(uint32 size)
{
    /* Overwrite the first sample size with this value */

    if (_sampleSize == 0)
    {
        /*
         * Indicates that there is a vector of samples
         * So, overwrite the first element's size
         */
        (*_psampleSizeVec)[0] = size;
    }
    else
    {
        /*
         * all samples of the same size
         */
        if (_sampleSize != size)
        {
            return false;
        }
        /* Do nothing */
    }
    return true;
}
