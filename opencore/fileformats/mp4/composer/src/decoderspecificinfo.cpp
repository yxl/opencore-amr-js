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
    This PVA_FF_DecoderSpecificInfo Class that holds the Mpeg4 VOL header for the
	video stream
*/

#define __IMPLEMENT_DecoderSpecificInfo__

#include "decoderspecificinfo.h"

#include "atomutils.h"

// Constructor
PVA_FF_DecoderSpecificInfo::PVA_FF_DecoderSpecificInfo()
        : PVA_FF_BaseDescriptor(0x05)
{
    _infoSize = 0;
    _pinfo = NULL;
}


// Constructor
PVA_FF_DecoderSpecificInfo::PVA_FF_DecoderSpecificInfo(uint8 *pdata, uint32 size)
        : PVA_FF_BaseDescriptor(0x05)
{
    _infoSize = size;
    _pinfo = (uint8 *)OSCL_MALLOC(_infoSize);

    oscl_memcpy(_pinfo, pdata, _infoSize);
    recomputeSize();
}

// Constructor
PVA_FF_DecoderSpecificInfo::PVA_FF_DecoderSpecificInfo(PVA_FF_TextSampleDescInfo *pdata, uint32 size)
        : PVA_FF_BaseDescriptor(0x05)
{
    _infoSize = size;
    _pinfo = (uint8 *)OSCL_MALLOC(_infoSize);

    oscl_memcpy(_pinfo, pdata, _infoSize);
    recomputeSize();
}


// Destructor
PVA_FF_DecoderSpecificInfo::~PVA_FF_DecoderSpecificInfo()
{

    OSCL_FREE(_pinfo);
    _pinfo  = NULL;
}

void
PVA_FF_DecoderSpecificInfo::addInfo(uint8 *info, uint32 size)
{
    _infoSize = size;
    _pinfo = (uint8 *)oscl_malloc(_infoSize);

    oscl_memcpy(_pinfo, info, _infoSize);
}


bool
PVA_FF_DecoderSpecificInfo::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
{
    // Render base descriptor packed size and tag
    if (!renderBaseDescriptorMembers(fp))
    {
        return false;
    }

    // Render decoder specific info payload
    if (!PVA_FF_AtomUtils::renderByteData(fp, _infoSize, _pinfo))
    {
        return false;
    }

    return true;
}

void
PVA_FF_DecoderSpecificInfo::recomputeSize()
{
    int32 contents = _infoSize; // Size of decoder specific info payload

    _sizeOfClass = contents;
    _sizeOfSizeField = PVA_FF_AtomUtils::getNumberOfBytesUsedToStoreSizeOfClass(contents);

    // Have the parent descriptor recompute its size based on this update
    if (_pparent != NULL)
    {
        _pparent->recomputeSize();
    }
}


