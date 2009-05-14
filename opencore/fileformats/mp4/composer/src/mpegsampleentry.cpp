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
    This PVA_FF_MpegSampleEntry fp used for hint tracks.
*/


#define IMPLEMENT_MpegSampleEntry

#include "mpegsampleentry.h"
#include "a_atomdefs.h"
#include "atomutils.h"


// Constructor
PVA_FF_MpegSampleEntry::PVA_FF_MpegSampleEntry(int32 mediaType)
        : PVA_FF_SampleEntry(MPEG_SAMPLE_ENTRY)
{
    // codecType fp hard coded to ZERO intentionally as PVA_FF_MpegSampleEntry fp called for
    // non-media samples ONLY.
    PV_MP4_FF_NEW(fp->auditCB, PVA_FF_ESDAtom, (mediaType, 0), _pes);
    init();
    recomputeSize();

    _pes->setParent(this);
}

// Destructor
PVA_FF_MpegSampleEntry::~PVA_FF_MpegSampleEntry()
{
    // Cleanup PVA_FF_ESDAtom
    PV_MP4_FF_DELETE(NULL, PVA_FF_ESDAtom, _pes);
}


void
PVA_FF_MpegSampleEntry::init()
{
    //
}

// Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
bool
PVA_FF_MpegSampleEntry::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
{
    int32 rendered = 0;

    if (!renderAtomBaseMembers(fp))
    {
        return false;
    }
    rendered += getDefaultSize();

    // From PVA_FF_SampleEntry base class
    for (int32 i = 0; i < 6; i++)
    {
        if (!PVA_FF_AtomUtils::render8(fp, _reserved[i]))
        {
            return false;
        }
        rendered += 1;
    }

    if (!PVA_FF_AtomUtils::render16(fp, _dataReferenceIndex))
    {
        return false;
    }

    if (!_pes->renderToFileStream(fp))
    {
        return false;
    }
    rendered += _pes->getSize();

    return true;
}



void
PVA_FF_MpegSampleEntry::recomputeSize()
{
    int32 size = getDefaultSize();
    size += 8; // For reserved and dataRefIndex of PVA_FF_SampleEntry base class

    size += _pes->getSize();

    _size = size;

    // Update size of parnet atom
    if (_pparent != NULL)
    {
        _pparent->recomputeSize();
    }
}
