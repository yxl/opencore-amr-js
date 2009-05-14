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
#ifndef BIT_UTIL_H
#define BIT_UTIL_H

#ifndef OSCL_TYPES_H_INCLUDED
#include "oscl_types.h"
#endif
#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

class BitUtil
{
    public:
        static uint32 bitValue(octet* buff, uint32 offset, uint32 len);
        static bool bitCopy(octet* src, uint32 src_offset, uint32 bitLen, octet** output, uint32& byteLenOut);

};



#endif //BIT_UTIL_H


