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
    This PVA_FF_SLConfigDescriptor Class
*/


#define IMPLEMENT_SLConfigDescriptor

#include "slconfigdescriptor.h"
#include "atomutils.h"

// Constructor
PVA_FF_SLConfigDescriptor::PVA_FF_SLConfigDescriptor()
        : PVA_FF_BaseDescriptor(0x06)
{
    init();
}

// Destructor
PVA_FF_SLConfigDescriptor::~PVA_FF_SLConfigDescriptor()
{
    // Empty
}

void
PVA_FF_SLConfigDescriptor::init()
{
    _predefined = 0x02; // According to MPEG4 File Format specification
    // - section 13.1.3.2 "Handling of elementary streams"
    // (last paragraph)
    _reserved2 = 0x7f;
    _OCRStreamFlag = false;
    _OCRESID = 0;

    if (_predefined == 0x01)
    {
        _useAccessUnitStartFlag = false;
        _useAccessUnitEndFlag = false;
        _useRandomAccessPointFlag = false;
        _usePaddingFlag = false;
        _useTimeStampsFlag = false;
        _useIdleFlag = false;
        _AULength = 0;
        _degradationPriorityLength = 0;
        _AUSeqNumLength = 0;
        _packetSeqNumLength = 0;
        _reserved1 = 0x3; // 0b11
    }


    recomputeSize();
}

// Rendering the Descriptor in proper format (bitlengths, etc.) to an ostream
bool
PVA_FF_SLConfigDescriptor::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
{
    int32 rendered = 0; // Keep track of number of bytes rendered

    // Render the base class members
    int32 numBytes = renderBaseDescriptorMembers(fp);

    if (numBytes == 0)
    {
        return false;
    }
    rendered += numBytes;

    if (!PVA_FF_AtomUtils::render8(fp, _predefined))
    {
        return false;
    }
    rendered += 1;

    return true;
}

void
PVA_FF_SLConfigDescriptor::recomputeSize()
{
    int32 contents = 0;
    contents += 1; //(8 bits for predefined flag)

    if (_predefined == 0)
    {
        contents += 1; //(8 bits for packet boolean flags)
        contents += 4; // (32 bits for TS resolution)
        contents += 4; // (32 bits for OCR resolution)
        contents += 1; // (8 bits for TS length)
        contents += 1; // (8 bits for OCR length)
        contents += 1; // (8 bits for AU length)
        contents += 1; // (8 bits for instant bitrate length)
        contents += 2; // (16 bits for packed DP, AUSN, pacSN lengths plus reserved)
        contents += 4; // (32 bits for timescale)
        contents += 2; // (16 bits for AU duration)
        contents += 2; // (16 bits for CU duration)

        contents += 4; // (32 bits for start Dec TS)
        contents += 4; // (32 bits for start Comp TS)
    }
    _sizeOfClass = contents;
    _sizeOfSizeField = PVA_FF_AtomUtils::getNumberOfBytesUsedToStoreSizeOfClass(contents);

    // Have the parent descriptor recompute its size based on this update
    if (_pparent != NULL)
    {
        _pparent->recomputeSize();
    }
}
