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
    This ESID_Ref Class contains information on the Elementary Stream that will
	be replaced when streaming to a client.  The file format replaces the actual
	ES_Descriptor with this object that references the actual descriptor.  This
	fp used in the OD Stream!
*/

#define IMPLEMENT_ES_ID_Ref

#include "es_id_ref.h"
#include "atomutils.h"
#include "a_atomdefs.h"

// Alternate constructor
PVA_FF_ES_ID_Ref::PVA_FF_ES_ID_Ref(uint32 trackindex)
        : PVA_FF_BaseDescriptor(ES_ID_REF_DESCR_TAG),
        _trackIndex((uint16)trackindex)
{
    recomputeSize();
}

// Destructor
PVA_FF_ES_ID_Ref::~PVA_FF_ES_ID_Ref()
{
    // Empty
}


void
PVA_FF_ES_ID_Ref::recomputeSize()
{
    _sizeOfClass = 2;
    _sizeOfSizeField = 1;
}

// Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
bool
PVA_FF_ES_ID_Ref::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
{
    int rendered = 0; // Keep track of number of bytes rendered

    // Render attributes of the PVA_FF_BaseDescriptor class
    int32 numBytes = renderBaseDescriptorMembers(fp);

    if (numBytes == 0)
    {
        return false;
    }
    rendered += numBytes; // (1 + 1(variable) for _sizeOfClass)

    // Render the ESID
    if (!PVA_FF_AtomUtils::render16(fp, getTrackIndex()))
    {
        return false;
    }
    rendered += 2;

    return true;
}


