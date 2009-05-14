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
    This PVA_FF_SampleEntry Class is the base class for the HinstSampleEntry,
    PVA_FF_VisualSampleEntry, PVA_FF_AudioSampleEntry. and PVA_FF_MpegSampleEntry classes.
*/


#define IMPLEMENT_SampleEntry

#include "sampleentry.h"


// Constructor
PVA_FF_SampleEntry::PVA_FF_SampleEntry(uint32 format)
        : PVA_FF_Atom(format)
{

    _reserved[0] = 0;
    _reserved[1] = 0;
    _reserved[2] = 0;
    _reserved[3] = 0;
    _reserved[4] = 0;
    _reserved[5] = 0;
    _dataReferenceIndex = 1;
}

// Destructor
PVA_FF_SampleEntry::~PVA_FF_SampleEntry()
{
    // Empty
}
