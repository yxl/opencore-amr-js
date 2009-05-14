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

#ifndef WCHAR_SIZE_UTILS_H_INCLUDED
#include "wchar_size_utils.h"
#endif

/**
 * Converts the 4 byte wchar data to 16 bit data
 */
OSCL_EXPORT_REF void PackWChar4BytesTo2Bytes(uint8* dest, oscl_wchar* src, int nChars)
{
    int count;
    uint8* destPtr = dest;
    oscl_wchar* srcPtr = src;
    for (count = 0; count < (nChars - 1); count++)
    {
        oscl_memcpy(destPtr, srcPtr, 2);
        destPtr += 2;
        srcPtr++;
    }
    *destPtr = 0;
    *(++destPtr) = 0;
}

/**
 * Converts the 2 byte wchar data to 4 byte data
 */
OSCL_EXPORT_REF void ExpandWChar2BytesTo4Bytes(oscl_wchar * dest, uint16* src, int nChars)
{
    int count;
    for (count = 0; count < (nChars - 1); count++)
    {
        dest[count] = src[count];
    }
    dest[count] = 0;
}
