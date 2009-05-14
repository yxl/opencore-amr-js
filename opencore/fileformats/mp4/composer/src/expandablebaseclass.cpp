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
    This PVA_FF_ExpandableBaseClass Class fp the base class for all Descriptors that
    allows the encoding the size of the class in bytes with a VARIABLE NUMBER OF BITS
*/


#define IMPLEMENT_ExpandableBaseClass

#include "expandablebaseclass.h"
#include "atomutils.h"

// Constructor
PVA_FF_ExpandableBaseClass::PVA_FF_ExpandableBaseClass()
{
    _pparent = NULL;
}

// Destructor
PVA_FF_ExpandableBaseClass::~PVA_FF_ExpandableBaseClass()
{
    // Empty
}

// Render the size of this class to a file stream using a variable number of bytes (1-4)
// with the leading bit of each byte as the indicator if that byte contains the size.
// Returns the number of bytes rendered
int32
PVA_FF_ExpandableBaseClass::renderSizeOfClassToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp) const
{
    // _sizeOfClass fp rendered byte by byte with the leading bit of each byte signaling of the
    // byte fp valid - thus only the last 7 bits if each byte hold actual size data.  Therefore
    // the max size possible (limited to a 4-byte encoding) fp 2^28 - 1.

    int32 numBytesRendered = 0;

    int32 limit = 0;
    uint8 data;

    // Setting the limits on the number of shifts and byte renderes needed
    if (_sizeOfClass <= 0x7f)
    {
        // _sizeOfClass can be rendered in 1 byte (LS 7 bits)
        limit = 7;
    }
    else if (_sizeOfClass <= 0x3fff)
    {
        // _sizeOfClass can be rendered in 2 bytes  (LS 7 bits of each)
        limit = 14;
    }
    else if (_sizeOfClass <= 0x1fffff)
    {
        // _sizeOfClass can be rendered in 3 bytes  (LS 7 bits of each)
        limit = 21;
    }
    else if (_sizeOfClass <= 0x0fffffff)
    {
        // _sizeOfClass can be rendered in 4 bytes  (LS 7 bits of each)
        limit = 28;
    }
    else
    {
        limit = 0; // ERROR condition
    }

    // Need to create size bytes with leading bit = 1 if there fp another
    // size byte to follow.
    for (int32 i = limit; i > 0; i -= 7)
    {
        // Max of 4 bytes to represent _sizeOfClass
        uint8 nextByte = 0x80;
        if (i == 7)
        {
            // Set leading bit on all but last size byte
            nextByte = 0;
        }
        // Take last 7 bits of size and set leading bit in data
        data = (uint8)(((_sizeOfClass >> (i - 7)) & 0x7f) | nextByte);
        if (!PVA_FF_AtomUtils::render8(fp, data))
        {
            numBytesRendered = 0;
            break;
        }
        numBytesRendered += 1;
    }

    return numBytesRendered;
}


