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
#define WCHAR_SIZE_UTILS_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif

/**
 * Converts the 4 byte wchar data to 2 byte data.
 *
 * @param dest Destination buffer pointer to receive 16 bit data
 * @param src Source buffer pointer holding 4 byte wchar data
 * @param nChars Length of chars need to be converted including null char
 * @return none
 */
OSCL_IMPORT_REF void PackWChar4BytesTo2Bytes(uint8* dest, oscl_wchar* src, int nChars);

/**
 * Converts the 2 byte wchar data to 4 byte data
 *
 * @param dest Destination buffer pointer to receive 4 byte data
 * @param src Source buffer pointer holding 16 bit wchar data
 * @param nChars Length of chars need to be converted including null char
 * @return none
 */
OSCL_IMPORT_REF void ExpandWChar2BytesTo4Bytes(oscl_wchar* dest, uint16* src, int nChars);

#endif // WCHAR_SIZE_UTILS_H_INCLUDED
