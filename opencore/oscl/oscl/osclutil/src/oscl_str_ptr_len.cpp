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
// -*- c++ -*-
// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

//               O S C L _ S T R _ P T R _ L E N

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

// - - Inclusion - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#ifndef OSCL_STR_PTR_LEN_H_INCLUDED
#include "oscl_str_ptr_len.h"
#endif

#define OSCL_DISABLE_WARNING_CONV_POSSIBLE_LOSS_OF_DATA
#include "osclconfig_compiler_warnings.h"
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// This function calculates the checksum of the string.
// Note that this function should be case insensitive! (i.e. checksums for
// "I love pizza" and "I LoVe PiZzA" should be the same)
OSCL_EXPORT_REF void StrCSumPtrLen::setCheckSum()
{
    checkSum = 0;

    for (int32 i = 0; i < len; ++i)
    {
        //checkSum = (CheckSumType)(checkSum << 3);
        if (isLetter(ptr[i]))
        {
            checkSum = (CheckSumType)(checkSum + (ptr[i] | OSCL_ASCII_CASE_MAGIC_BIT));
        }
        else
        {
            checkSum = (CheckSumType)(checkSum + ptr[i]);
        }
    }
}

