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
    This PVA_FF_ESDescriptor Class contains information on the Elementary Stream
*/


#define IMPLEMENT_ESDescriptor

#include "esdescriptor.h"
#include "atomutils.h"
#include "a_atomdefs.h"

// Constructor - used for a NEW stream (i.e. a new track)
PVA_FF_ESDescriptor::PVA_FF_ESDescriptor(int32 streamType, int32 codecType)
        : PVA_FF_BaseDescriptor(0x03)
{
    PV_MP4_FF_NEW(fp->auditCB, PVA_FF_DecoderConfigDescriptor, (streamType, codecType), _pdcd);
    PV_MP4_FF_NEW(fp->auditCB, PVA_FF_SLConfigDescriptor, (), _pslcd);

    _ESID = 0; // Initialize the ESID, will be set later
    init();

    recomputeSize();

    _pdcd->setParent(this);
    _pslcd->setParent(this);
}

// Destructor
PVA_FF_ESDescriptor::~PVA_FF_ESDescriptor()
{
    // Cleanup descriptors
    PV_MP4_FF_DELETE(NULL, PVA_FF_DecoderConfigDescriptor, _pdcd);
    PV_MP4_FF_DELETE(NULL, PVA_FF_SLConfigDescriptor, _pslcd);
}

void
PVA_FF_ESDescriptor::init()
{
    _streamDependenceFlag = false;
    _urlFlag = false;
    _reserved = 1;
    _streamPriority = 0;
    _dependsOnESID = 0;
    _urlLength = 0;
}

void
PVA_FF_ESDescriptor::recomputeSize()
{
    _urlLength = (uint8)(_urlString.get_size());

    int32 contents = 0;

    contents += 2; //(16 bits for ESID)
    contents += 1; //(8 bits for packed SDF, UF, reserved, and stream Priority)
    if (_streamDependenceFlag)  // If this stream depends on another ES
    {
        contents += 2; // depends on ESID
    }
    if (_urlFlag)  // URL only present fp flag set
    {
        contents += 1; // URL length stored in 1 byte
        contents += _urlLength; // length of actual URL string
    }
    contents += _pdcd->getSizeOfDescriptorObject();
    contents += _pslcd->getSizeOfDescriptorObject();

    // Setting actual size of this descriptor class
    _sizeOfClass = contents;
    _sizeOfSizeField = PVA_FF_AtomUtils::getNumberOfBytesUsedToStoreSizeOfClass(contents);

    // Have the parent descriptor recompute its size based on this update
    if (_pparent != NULL)
    {
        _pparent->recomputeSize();
    }
}


// Rendering the Descriptor in proper format (bitlengths, etc.) to an ostream
bool
PVA_FF_ESDescriptor::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
{
    int32 rendered = 0; // Keep track of number of bytes rendered

    // Render attributes of the PVA_FF_BaseDescriptor class
    int32 numBytes = renderBaseDescriptorMembers(fp);

    if (numBytes == 0)
    {
        return false;
    }
    rendered += numBytes; // (1 + variable for _sizeOfClass)

    if (!PVA_FF_AtomUtils::render16(fp, 0))
    {
        return false;
    }
    rendered += 2;

    // Pack and render SDF, UF, reserved, and stream Priority

    uint8 data = 0x00; // OCRstreamFlag set to 0

    if (_streamDependenceFlag)
    {
        // If this stream depends on another ES
        data |= 0x80; //data = 1000 0000
    }
    if (_urlFlag)
    {
        // If URL present in this descriptor
        data |= 0x40; //data = 1100 0000
    }

    // OCRStreamFlag defaults to ZERO - Bit 3 from MSB

    data |= (_streamPriority & 0x1f); // LS 5 bits for stream priority
    if (!PVA_FF_AtomUtils::render8(fp, data))
    {
        return false;
    }
    rendered += 1;

    if (_streamDependenceFlag)
    {
        // If this stream depends on another ES
        if (!PVA_FF_AtomUtils::render16(fp, _dependsOnESID))
        {
            return false;
        }
        rendered += 2;
    }
    if (_urlFlag)
    {
        //If URL present in this descriptor
        if (!PVA_FF_AtomUtils::render8(fp, _urlLength))
        {
            return false;
        }
        if (!PVA_FF_AtomUtils::renderString(fp, _urlString))
        {
            return false;
        }
        rendered += _urlLength + 1;
    }

    // Render PVA_FF_DecoderConfigDescriptor
    if (!_pdcd->renderToFileStream(fp))
    {
        return false;
    }
    rendered += _pdcd->getSizeOfDescriptorObject();

    // Render PVA_FF_SLConfigDescriptor
    if (!_pslcd->renderToFileStream(fp))
    {
        return false;
    }
    rendered += _pslcd->getSizeOfDescriptorObject();

    return true;
}

