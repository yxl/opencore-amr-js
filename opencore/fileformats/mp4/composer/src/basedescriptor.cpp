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
    This PVA_FF_BaseDescriptor Class
*/

#define IMPLEMENT_BaseDescriptor

#include "basedescriptor.h"
#include "atomutils.h"


// Constructor
PVA_FF_BaseDescriptor::PVA_FF_BaseDescriptor(uint8 tag)
{
    _sizeOfClass = 0;
    _sizeOfSizeField = DEFAULT_DESCRIPTOR_SIZE; // 1
    _tag = tag;
}

// Destructor
PVA_FF_BaseDescriptor::~PVA_FF_BaseDescriptor()
{
    // Empty
}

// Rendering the PVA_FF_BaseDescriptor members in proper format (bitlengths, etc.)
// to an ostream
int
PVA_FF_BaseDescriptor::renderBaseDescriptorMembers(MP4_AUTHOR_FF_FILE_IO_WRAP *fp) const
{
    PVA_FF_AtomUtils::render8(fp, getTag());
    int32 numBytesRendered = 1;

    // Render attributes of the PVA_FF_BaseDescriptor class
    int32 numBytes = renderSizeOfClassToFileStream(fp);

    if (numBytes > 0)
    {
        numBytesRendered += numBytes;
    }
    else
    {
        numBytesRendered = 0;
    }

    return numBytesRendered;
}



