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
#define IMPLEMENT_PVA_FF_AVCSampleEntry

#include "avcsampleentry.h"
#include "atomutils.h"
#include "a_atomdefs.h"

// Constructor
PVA_FF_AVCSampleEntry::PVA_FF_AVCSampleEntry(uint8 profile, uint8 profileComp, uint8 level)
        : PVA_FF_SampleEntry(AVC_SAMPLE_ENTRY)
{

    init(profile, profileComp, level);
    recomputeSize();


}

// Destructor
PVA_FF_AVCSampleEntry::~PVA_FF_AVCSampleEntry()
{
    PV_MP4_FF_DELETE(NULL, PVA_FF_AVCConfigurationAtom, _pAVCConfigurationAtom);
}

void
PVA_FF_AVCSampleEntry::init(uint8 profile, uint8 profileComp, uint8 level)
{
    for (int i = 0;i < 6;i++)
    {
        _reserved[i] = 0;
    }
    _dataReferenceIndex = 1;
    _preDefined1 = 0;
    _reserved1 = 0;

    for (int j = 0;j < 3;j++)
    {
        _predefined2[j] = 0;
    }
    _width = 176;
    _height = 144;

    _horizResolution = 0x00480000;
    _vertResolution = 0x00480000;
    _reserved2 = 0;
    _predefined3 = 1;
    for (int k = 0;k < 32;k++)
    {
        _compressorName[k] = 0;
    }

    _depth = 0x0018;
    _predefined4 = -1;

    PV_MP4_FF_NEW(fp->auditCB, PVA_FF_AVCConfigurationAtom, (profile, profileComp, level), _pAVCConfigurationAtom);

}

void PVA_FF_AVCSampleEntry::setVideoParam(int16 width, int16 height)
{
    _width = width;
    _height = height;

}

void PVA_FF_AVCSampleEntry::addDecoderSpecificInfo(PVA_FF_DecoderSpecificInfo *pinfo)
{


    uint8* sample = pinfo->getInfo();
    uint32 size = pinfo->getInfoSize();

    setSample(sample, size);
}


// Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
bool
PVA_FF_AVCSampleEntry::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
{

    int32 rendered = 0;

    if (!renderAtomBaseMembers(fp))
    {
        return false;
    }
    rendered += getDefaultSize();

    // From PVA_FF_SampleEntry base class
    for (int k = 0; k < 6; k++)
    {
        if (!PVA_FF_AtomUtils::render8(fp, _reserved[k]))
        {
            return false;
        }
        rendered += 1;
    }


    if (!PVA_FF_AtomUtils::render16(fp, _dataReferenceIndex))
    {
        return false;
    }
    rendered  += 2;
    if (!PVA_FF_AtomUtils::render16(fp, _preDefined1))
    {
        return false;
    }
    rendered  += 2;

    if (!PVA_FF_AtomUtils::render16(fp, _reserved1))
    {
        return false;
    }
    rendered  += 2;
    for (int j = 0; j < 3;j++)
    {
        if (!PVA_FF_AtomUtils::render32(fp, _predefined2[j]))
        {
            return false;
        }
        rendered += 4;
    }
    if (!PVA_FF_AtomUtils::render16(fp, _width))
    {
        return false;
    }
    rendered  += 2;

    if (!PVA_FF_AtomUtils::render16(fp, _height))
    {
        return false;
    }
    rendered  += 2;


    if (!PVA_FF_AtomUtils::render32(fp, _horizResolution))
    {
        return false;
    }
    rendered  += 4;

    if (!PVA_FF_AtomUtils::render32(fp, _vertResolution))
    {
        return false;
    }
    rendered  += 4;

    if (!PVA_FF_AtomUtils::render32(fp, _reserved2))
    {
        return false;
    }
    rendered  += 4;

    if (!PVA_FF_AtomUtils::render16(fp, _predefined3))
    {
        return false;
    }
    rendered  += 2;

    if (!PVA_FF_AtomUtils::renderByteData(fp, 32, _compressorName))
    {
        return false;
    }
    rendered  += 32;


    if (!PVA_FF_AtomUtils::render16(fp, _depth))
    {
        return false;
    }
    rendered  += 2;

    if (!PVA_FF_AtomUtils::render16(fp, _predefined4))
    {
        return false;
    }
    rendered  += 2;

    if (!_pAVCConfigurationAtom->renderToFileStream(fp))
    {
        return false;
    }
    rendered += _pAVCConfigurationAtom->getSize();

    return true;
}

void
PVA_FF_AVCSampleEntry::recomputeSize()
{
    int32 size = getDefaultSize();

// From PVA_FF_SampleEntry base class
    for (int k = 0; k < 6; k++)
    {
        size += sizeof(_reserved[k]);
    }

    size += sizeof(_dataReferenceIndex);
    size  += sizeof(_preDefined1);
    size  += sizeof(_reserved1);
    for (int j = 0; j < 3;j++)
    {
        size += sizeof(_predefined2[j]);
    }
    size  += sizeof(_width);
    size  += sizeof(_height);
    size  += sizeof(_horizResolution);
    size  += sizeof(_vertResolution);
    size  += sizeof(_reserved2);
    size  += sizeof(_predefined3);
    size  += 32;
    size  += sizeof(_depth);
    size  += sizeof(_predefined4);
    size  += _pAVCConfigurationAtom->getSize();

    _size = size;
    // Update size of parent
    if (_pparent != NULL)
    {
        _pparent->recomputeSize();
    }

}
