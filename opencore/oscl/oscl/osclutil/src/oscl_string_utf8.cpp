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

//               O S C L  UTF-8 S T R I N G  FUNCTIONS

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

// - - Inclusion - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include "oscl_string_utf8.h"
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#define UTF8_1_BYTE_MASK     0x80     //1000 0000 -> 0xxx xxxx
#define UTF8_1_BYTE_SIG      0x00     //0000 0000 -> 0xxx xxxx

#define UTF8_2_BYTES_MASK_1  0xE0     //1110 0000 -> 110y yyyy
#define UTF8_2_BYTES_MASK_2  0xC0     //1100 0000 -> 10xx xxxx
#define UTF8_2_BYTES_SIG_1   0xC0     //1100 0000 -> 110y yyyy
#define UTF8_2_BYTES_SIG_2   0x80     //1000 0000 -> 10xx xxxx


#define UTF8_3_BYTES_MASK_1  0xF0     //1111 0000 -> 1110 zzzz
#define UTF8_3_BYTES_MASK_2  0xC0     //1100 0000 -> 10yy yyyy
#define UTF8_3_BYTES_MASK_3  0xC0     //1100 0000 -> 10xx xxxx
#define UTF8_3_BYTES_SIG_1   0xE0     //1110 0000 -> 1110 zzzz
#define UTF8_3_BYTES_SIG_2   0x80     //1000 0000 -> 10yy yyyy
#define UTF8_3_BYTES_SIG_3   0x80     //1000 0000 -> 10xx xxxx


#define UTF8_4_BYTES_MASK_1    0xF8     //1111 1000 -> 1111 0uuu
#define UTF8_4_BYTES_MASK_2    0xC0     //1100 0000 -> 10uu zzzz
#define UTF8_4_BYTES_MASK_3    0xC0     //1100 0000 -> 10yy yyyy
#define UTF8_4_BYTES_MASK_4    0xC0     //1100 0000 -> 10xx xxxx
#define UTF8_4_BYTES_SIG_1     0xF0     //1111 0000 -> 1111 0uuu
#define UTF8_4_BYTES_SIG_2     0x80     //1000 0000 -> 10uu zzzz
#define UTF8_4_BYTES_SIG_3     0x80     //1000 0000 -> 10yy yyyy
#define UTF8_4_BYTES_SIG_4     0x80     //1000 0000 -> 10xx xxxx

/* ========================================================================
 *	Function : oscl_str_is_valid_and_truncate_utf8
 *	Date     : 10/25/2002
 *  Purpose  :
 *	Modified :
 * ========================================================================
 */
OSCL_EXPORT_REF bool  oscl_str_is_valid_and_truncate_utf8(const uint8 *str_buf, uint32& num_valid_characters, uint32 max_bytes, uint32& truncate_pos_index,
        uint32 max_char_2_valid = 0)
{
    bool result = true, done = false;
    uint32 bytes_left;
    uint32 total_num_valid_characters;
    const uint8 *ptr;

    ptr = str_buf;
    bytes_left = max_bytes;
    total_num_valid_characters = 0;
    truncate_pos_index = 0;

    while (!done)
    {
        if ((*ptr & UTF8_1_BYTE_MASK) == UTF8_1_BYTE_SIG)
        {
            if ((*ptr == 0x0) && (max_bytes == 0)) //if need to be terminated at first null char
            {
                result = true;
                done = true;
                continue;
            }
            if (bytes_left > 0)
                bytes_left--;
            if (max_bytes && !bytes_left) //done when there is no more data
                done = true;

            ptr++;
            total_num_valid_characters ++;
            truncate_pos_index += 1;
        }
        else if ((*ptr & UTF8_2_BYTES_MASK_1) == UTF8_2_BYTES_SIG_1)
        {
            if ((!max_bytes) || (bytes_left >= 2))
            {
                if (((*(ptr + 1) & UTF8_2_BYTES_MASK_2) != UTF8_2_BYTES_SIG_2)
                        || (*ptr < 0xC2)  //C0, C1 are illegal
                   )
                { //invalid char
                    done = true;
                    result = false;
                    continue;
                }
                if (bytes_left)
                    bytes_left -= 2;

                if (max_bytes && !bytes_left) //done when there is no more data
                    done = true;

                ptr += 2;
                total_num_valid_characters++;
                truncate_pos_index += 2;
            }
            else
            {
                done = true;
                result = false;
            }
        }
        else if ((*ptr & UTF8_3_BYTES_MASK_1) == UTF8_3_BYTES_SIG_1)
        {
            if ((!max_bytes) || (bytes_left >= 3))
            {
                if (((*(ptr + 1) & UTF8_3_BYTES_MASK_2) != UTF8_3_BYTES_SIG_2)
                        || ((*(ptr + 2) & UTF8_3_BYTES_MASK_3) != UTF8_3_BYTES_SIG_3)
                        || ((*ptr == 0xE0) && (*(ptr + 1) < 0xA0)) //if 1st byte =E0, 2n byte has to be greater than 0xA0
                   )
                { //invalid char
                    done = true;
                    result = false;
                    continue;
                }
                if (bytes_left > 0)
                    bytes_left -= 3;

                if (max_bytes && !bytes_left) //done when there is no more data
                    done = true;

                ptr += 3;
                total_num_valid_characters++;
                truncate_pos_index += 3;
            }
            else
            {
                done = true;
                result = false;
            }
        }
        else if ((*ptr & UTF8_4_BYTES_MASK_1) == UTF8_4_BYTES_SIG_1)
        {
            if ((!max_bytes) || (bytes_left >= 4))
            {
                if (((*(ptr + 1) & UTF8_4_BYTES_MASK_2) != UTF8_4_BYTES_SIG_2)
                        || ((*(ptr + 2) & UTF8_4_BYTES_MASK_3) != UTF8_4_BYTES_SIG_3)
                        || ((*(ptr + 3) & UTF8_4_BYTES_MASK_4) != UTF8_4_BYTES_SIG_4)
                        || (*ptr > 0xF4)
                        || ((*ptr == 0xF4) && (*(ptr + 1) > 0x8F))
                   )
                { //invalid char
                    done = true;
                    result = false;
                    continue;
                }
                if (bytes_left > 0)
                    bytes_left -= 4;

                if (max_bytes && !bytes_left) //done when there is no more data
                    done = true;

                ptr += 4;
                total_num_valid_characters++;
                truncate_pos_index += 4;
            }
            else
            {
                done = true;
                result = false;
            }
        }
        else  //found invalid char
        {
            done = true;
            result = false;

            if (bytes_left > 0)
                bytes_left--;
        }
        if (num_valid_characters && (total_num_valid_characters == num_valid_characters))
            done = true;
        if ((!num_valid_characters) && max_char_2_valid && (total_num_valid_characters == max_char_2_valid))
            done = true;

    }
    num_valid_characters = total_num_valid_characters;

    return result;
}

/* ========================================================================
 *	Function : oscl_str_is_valid_utf8
 *	Date     : 10/25/2002
 *  Purpose  : see oscl_string_utf8.h
 *	Modified :
 * ========================================================================
 */
OSCL_EXPORT_REF bool  oscl_str_is_valid_utf8(const uint8 *str_buf, uint32& num_valid_characters, uint32 max_bytes,
        uint32 max_char_2_valid, uint32 *num_byte_4_char)
{
    uint32 truncate_pos_index = 0;
    num_valid_characters = 0;
    bool result;

    if (num_byte_4_char)
        *num_byte_4_char = 0;
    result = oscl_str_is_valid_and_truncate_utf8(str_buf, num_valid_characters, max_bytes, truncate_pos_index, max_char_2_valid);
    if (num_byte_4_char)
        *num_byte_4_char = truncate_pos_index;

    return result;
}
/* ========================================================================
 *	Function : oscl_str_truncate_utf8
 *	Date     : 10/25/2002
 *  Purpose  : see oscl_string_utf8.h
 *	Modified :
 * ========================================================================
 */
OSCL_EXPORT_REF int32  oscl_str_truncate_utf8(uint8 *str_buf, uint32 max_chars, uint32 max_bytes)
{
    uint32 num_valid_characters, truncate_pos_index;

    num_valid_characters = max_chars;
    truncate_pos_index = 0;

    oscl_str_is_valid_and_truncate_utf8(str_buf, num_valid_characters, max_bytes, truncate_pos_index);
    if (max_bytes)
    {
        if (truncate_pos_index <= (max_bytes - 1))
            *(str_buf + truncate_pos_index) = 0x00;
    }
    else
    {
        *(str_buf + truncate_pos_index) = 0x00;
    }

    return num_valid_characters;
}

