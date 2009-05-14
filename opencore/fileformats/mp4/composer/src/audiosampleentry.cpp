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
    This PVA_FF_AudioSampleEntry Class is used for visual streams.
*/

#define IMPLEMENT_AudioSampleEntry

#include "audiosampleentry.h"
#include "atomutils.h"
#include "a_atomdefs.h"


// Constructor
PVA_FF_AudioSampleEntry::PVA_FF_AudioSampleEntry(int32 codecType)
        : PVA_FF_SampleEntry(FourCharConstToUint32('m', 'p', '4', 'a'))
{
    PV_MP4_FF_NEW(fp->auditCB, PVA_FF_ESDAtom, (MEDIA_TYPE_AUDIO, codecType), _pes);

    init();
    recomputeSize();

    _pes->setParent(this);
}

// Destructor
PVA_FF_AudioSampleEntry::~PVA_FF_AudioSampleEntry()
{
    // Cleanup PVA_FF_ESDAtom
    PV_MP4_FF_DELETE(NULL, PVA_FF_ESDAtom, _pes);
}

void
PVA_FF_AudioSampleEntry::init()
{
    _reserved1[0] = 0;
    _reserved1[1] = 0;

    _reserved2 = 2;
    _reserved3 = 16;
    _reserved4 = 0;

    _timeScale = 0;

    _reserved5 = 0;
}

// Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
bool
PVA_FF_AudioSampleEntry::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
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
        if (! PVA_FF_AtomUtils::render8(fp, _reserved[i]))
        {
            return false;
        }
        rendered += 1;
    }
    if (!PVA_FF_AtomUtils::render16(fp, _dataReferenceIndex))
    {
        return false;
    }

    if (!PVA_FF_AtomUtils::render32(fp, _reserved1[0]))
    {
        return false;
    }
    if (!PVA_FF_AtomUtils::render32(fp, _reserved1[1]))
    {
        return false;
    }
    rendered += 8;

    if (!PVA_FF_AtomUtils::render16(fp, _reserved2))
    {
        return false;
    }
    if (!PVA_FF_AtomUtils::render16(fp, _reserved3))
    {
        return false;
    }
    if (!PVA_FF_AtomUtils::render32(fp, _reserved4))
    {
        return false;
    }
    rendered += 8;

    if (!PVA_FF_AtomUtils::render16(fp, _timeScale))
    {
        return false;
    }
    rendered += 2;

    if (!PVA_FF_AtomUtils::render16(fp, _reserved5))
    {
        return false;
    }
    rendered += 2;

    if (!_pes->renderToFileStream(fp))
    {
        return false;
    }
    rendered += getESDAtom().getSize();

    return true;
}


void
PVA_FF_AudioSampleEntry::recomputeSize()
{
    int32 size = getDefaultSize();

    size += 8; // For reserved and dataRefIndex of PVA_FF_SampleEntry base class

    size += 16; // For reserved1 - reserved4
    size += 2; // For timescale
    size += 2; // For reserved5
    size += getESDAtom().getSize();

    _size = size;

    if (_pparent != NULL)
    {
        _pparent->recomputeSize();
    }
}
