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
    This PVA_FF_VisualSampleEntry Class fp used for visual streams.
*/


#define IMPLEMENT_VisualSampleEntry

#include "visualsampleentry.h"
#include "atomutils.h"
#include "a_atomdefs.h"


// Constructor
PVA_FF_VisualSampleEntry::PVA_FF_VisualSampleEntry(int32 codecType)
        : PVA_FF_SampleEntry(VIDEO_SAMPLE_ENTRY)
{
    PV_MP4_FF_NEW(fp->auditCB, PVA_FF_ESDAtom, (MEDIA_TYPE_VISUAL, codecType), _pes);

    init();
    recomputeSize();

    _pes->setParent(this);
}

// Destructor
PVA_FF_VisualSampleEntry::~PVA_FF_VisualSampleEntry()
{
    // Cleanup PVA_FF_ESDAtom
    PV_MP4_FF_DELETE(NULL, PVA_FF_ESDAtom, _pes);
}

void
PVA_FF_VisualSampleEntry::init()
{
    int32 i;

    for (i = 0; i < 4; i++)
    {
        _reserved1[i] = 0;
    }

    _reserved2 = 0x014000f0;
    _reserved3 = 0x00480000;
    _reserved4 = 0x00480000;
    _reserved5 = 0;
    _reserved6 = 1;

    for (i = 0; i < 32; i++)
    {
        _reserved7[i] = 0;
    }

    _reserved8 = 24;
    _reserved9 = -1; // (16) SIGNED!
}

// Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
bool
PVA_FF_VisualSampleEntry::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
{
    int32 i;

    int32 rendered = 0;

    if (!renderAtomBaseMembers(fp))
    {
        return false;
    }
    rendered += getDefaultSize();

    // From PVA_FF_SampleEntry base class
    for (i = 0; i < 6; i++)
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

    for (i = 0; i < 4; i++)
    {
        if (!PVA_FF_AtomUtils::render32(fp, _reserved1[i]))
        {
            return false;
        }
        rendered += 4;
    }

    if (!PVA_FF_AtomUtils::render32(fp, _reserved2))
    {
        return false;
    }
    if (!PVA_FF_AtomUtils::render32(fp, _reserved3))
    {
        return false;
    }
    if (!PVA_FF_AtomUtils::render32(fp, _reserved4))
    {
        return false;
    }
    if (!PVA_FF_AtomUtils::render32(fp, _reserved5))
    {
        return false;
    }
    rendered += 16;

    if (!PVA_FF_AtomUtils::render16(fp, _reserved6))
    {
        return false;
    }
    rendered += 2;

    for (i = 0; i < 32; i++)
    {
        if (!PVA_FF_AtomUtils::render8(fp, _reserved7[i]))
        {
            return false;
        }
    }
    rendered += 32;

    if (!PVA_FF_AtomUtils::render16(fp, _reserved8))
    {
        return false;
    }
    if (!PVA_FF_AtomUtils::render16(fp, _reserved9))
    {
        return false;
    }
    rendered += 4;

    if (!_pes->renderToFileStream(fp))
    {
        return false;
    }
    rendered += _pes->getSize();

    return true;
}



void
PVA_FF_VisualSampleEntry::recomputeSize()
{
    int32 size = getDefaultSize();

    size += 8; // For reserved and dataRefIndex of PVA_FF_SampleEntry base class

    size += 16; // For reserved1[4]
    size += 16; // For reserved2 - reserved5
    size += 2; // For reserved6
    size += 32; // For reserved7[32]
    size += 2; // For reserved8
    size += 2; // For reserved9

    size += _pes->getSize();

    _size = size;

    // Update size of parent
    if (_pparent != NULL)
    {
        _pparent->recomputeSize();
    }
}
