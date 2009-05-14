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

//               O S C L  URI S T R I N G  FUNCTIONS

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

// - - Inclusion - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include "oscl_string_uri.h"
#include "oscl_string_utils.h"
#include "oscl_exception.h"
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/* ========================================================================
 *	Function : oscl_str_unescape_uri
 *	Date     : 11/04/2002
 *  Purpose  : see oscl_string_uri.h
 *	Modified :
 * ========================================================================
 */
OSCL_EXPORT_REF bool  oscl_str_unescape_uri(const char *str_buf_in, char *str_buf_out, uint32 max_out_buf_bytes, uint32 max_bytes, uint32& out_buf_len)
{
    const char *src = str_buf_in;
    char  *dest = str_buf_out;
    uint32 srcStrLen = max_bytes, destStrLen = max_out_buf_bytes ;
    uint32 val = 0;
    int32  i = 0;
    bool   result = false;

    out_buf_len = 0;
    if ((srcStrLen <= 0) || (src == NULL))
        return (false);

    while (srcStrLen > 0)
    {
        if (src[i] == '%')
        {
            i++;
            srcStrLen--;

            if (srcStrLen >= 2)
            {
                if (PV_atoi(&src[i], 'x', 2, val) == false)
                {
                    if (destStrLen > 0)
                        *dest = (char)'\0';
                    return false;
                }
                i += 2;
                if (destStrLen > 0)
                {
                    *dest++ = (char)val;
                    destStrLen--;
                }
                out_buf_len += 1;
                srcStrLen -= 2;
            }
            else
            {
                if (destStrLen > 0)
                    *dest = (char)'\0';
                return false;
            }
        }
        else
        {
            if (src[i] == (char)'\0')
                break;

            if (destStrLen > 0)
            {
                *dest++ = src[i];
                destStrLen--;
            }
            out_buf_len += 1;
            i++;
            srcStrLen--;
        }
    }

    if (destStrLen > 0)
    {
        *dest = (char)'\0';
        result = true;
    }
    else
        out_buf_len += 1; //needed size

    return result;
}

/* ========================================================================
 *	Function : oscl_str_unescape_uri
 *	Date     : 11/04/2002
 *  Purpose  : see oscl_string_uri.h
 *	Modified :
 * ========================================================================
 */
OSCL_EXPORT_REF bool  oscl_str_unescape_uri(const OSCL_String& srcString, OSCL_String& destString, uint32& out_buf_len)
{
    const char *src = srcString.get_cstr();
    int srcStrLen = srcString.get_size();

    destString = "";
    out_buf_len = 0;

    if ((srcStrLen <= 0) || (src == NULL))
    {
        return false;
    }

    char buf[2];
    buf[1] = '\0';

    for (int32 i = 0;srcStrLen > 0;)
    {
        if (src[i] == '%')
        {
            i++;
            srcStrLen--;

            if ((srcStrLen >= 2))
            {
                uint32 val;
                if (PV_atoi(&src[i], 'x', 2, val) == false)
                {
                    return false;
                }

                //check capacity before appending to avoid any leave.
                if (destString.get_size() == destString.get_maxsize())
                    return false;
                buf[0] = (char)val;
                destString += buf;
                out_buf_len++;

                i += 2;
                srcStrLen -= 2;
            }
            else
            {
                return false;
            }
        }
        else
        {
            if (src[i] == (char)'\0')
                break;

            //check capacity before appending to avoid any leave.
            if (destString.get_size() == destString.get_maxsize())
                return false;
            buf[0] = src[i];
            destString += buf;
            out_buf_len++;

            i++;
            srcStrLen--;
        }
    }

    return true;
}

