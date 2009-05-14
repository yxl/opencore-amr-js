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
/*********************************************************************************/
/*     -------------------------------------------------------------------       */
/*                           MPEG-4 SampleEntry Class                            */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This SampleEntry Class is the base class for the HinstSampleEntry,
    VisualSampleEntry, AudioSampleEntry. and MpegSampleEntry classes.
*/


#define IMPLEMENT_SampleEntry
#include "decoderspecificinfo.h"
#include "esdescriptor.h"
#include "sampleentry.h"
#include "atomutils.h"

// Stream-in Constructor
SampleEntry::SampleEntry(MP4_FF_FILE *fp, uint32 size, uint32 type)
        : Atom(fp, size, type)
{
    if (_success)
    {

        if (!AtomUtils::read8read8(fp, _reserved[0], _reserved[1]))
            _success = false;
        if (!AtomUtils::read8read8(fp, _reserved[2], _reserved[3]))
            _success = false;
        if (!AtomUtils::read8read8(fp, _reserved[4], _reserved[5]))
            _success = false;

        if (!AtomUtils::read16(fp, _dataReferenceIndex))
            _success = false;

        if (!_success)
            _mp4ErrorCode = READ_SAMPLE_ENTRY_FAILED;
    }
    else
    {
        _mp4ErrorCode = READ_SAMPLE_ENTRY_FAILED;
    }
}

// Destructor
SampleEntry::~SampleEntry()
{
    // Empty
}

DecoderSpecificInfo * SampleEntry::getDecoderSpecificInfo() const
{
    return NULL;
}
uint8 SampleEntry::getObjectTypeIndication() const
{
    return 0;
}

uint32 SampleEntry::getMaxBufferSizeDB() const
{
    return 0;
}
uint32 SampleEntry::getAverageBitrate() const
{
    return 0;
}
uint32 SampleEntry::getMaxBitrate() const
{
    return 0;
}
uint16 SampleEntry::getWidth() const
{
    return 0;
}
uint16 SampleEntry::getHeight() const
{
    return 0;
}
