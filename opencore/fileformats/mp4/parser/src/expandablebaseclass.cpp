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
/*                       MPEG-4 ExpandableBaseClass Class                        */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This ExpandableBaseClass Class is the base class for all Descriptors that
    allows the encoding the size of the class in bytes with a VARIABLE NUMBER OF BITS
*/


#define IMPLEMENT_ExpandableBaseClass

#include "expandablebaseclass.h"
#include "atomutils.h"

// Stream-in Constructor
ExpandableBaseClass::ExpandableBaseClass(MP4_FF_FILE *fp,
        bool o3GPPTrack)
{
    _success = true;

    if (!o3GPPTrack)
    {
        if (!AtomUtils::read8(fp, _tag))
            _success = false;

        readSizeOfClassFromFileStream(fp);
    }
}

// Destructor
ExpandableBaseClass::~ExpandableBaseClass()
{
    // Empty
}

// Read in the size of this class from a file stream using a variable number of bytes (1-4)
// with the leading bit of each byte as the indicator if that byte contains the size.
// Returns the number of bytes read in
int32
ExpandableBaseClass::readSizeOfClassFromFileStream(MP4_FF_FILE *fp)
{
    // _sizeOfClass is rendered byte by byte with the leading bit of each byte signaling of the
    // byte is valid - thus only the last 7 bits if each byte hold actual size data.  Therefore
    // the max size possible (limited to a 4-byte encoding) is 2^28 - 1.

    _sizeOfClass = 0;
    uint32 size = 0;
    uint8 data;
    int32 numBytesRead = 0;

    if (!AtomUtils::read8(fp, data)) // Read in first byte -
        _success = false;

    if (_success)
    {

        size = data & 0x7f; // Take last 7 bits
        bool nextByte = (data >= 128) ? true : false; // Check leading bit of byte
        numBytesRead = 1;

        while (nextByte)
        {

            if (!AtomUtils::read8(fp, data))  // Read in next byte
            {
                _success = false;
                break;
            }
            nextByte = (data >= 128) ? true : false; // Check leading bit of byte
            size = (size << 7) | (data & 0x7f); // Take last 7 bits
            numBytesRead += 1;
        }
        _sizeOfSizeField = numBytesRead;

    }

    // Size field only size of descriptor data - not including tag and
    // size field itself.  Need to add tag and size field to _sizeOfClass
    // so we can handle the descriptors as we did in 1.0
    _sizeOfClass = size + numBytesRead + 1; // 1 for tag

    return numBytesRead;
}


