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
#ifndef OSCL_STRING_UTILS_H_INCLUDED
#include "oscl_string_utils.h"
#endif

#include "oscl_stdstring.h"

/* ======================================================================== */
/*	Function : PV_atoi(char *buf,char new_format)							*/
/*	Date     : 02/22/2001													*/
/*	Purpose  : Extracts an integer from the input string.					*/
/*	In/out   :																*/
/*	Return   :																*/
/*	Modified :																*/
/* ======================================================================== */
OSCL_EXPORT_REF bool PV_atoi(const char *buf, const char new_format, uint32& value)
{
    return PV_atoi(buf, new_format, oscl_strlen(buf), value);
}


OSCL_EXPORT_REF bool PV_atoi(const char *buf, const char new_format, int length, uint32& value)
{
    uint32 old, temp;
    const char *ptr = buf;
    value = 0;

    if (buf == NULL) return false;

    switch (new_format)
    {
        case 'x':
        {
            while (ptr - buf < length)
            {
                if ((*ptr == 'a') || (*ptr == 'A'))
                    temp = 10;
                else if ((*ptr == 'b') || (*ptr == 'B'))
                    temp = 11;
                else if ((*ptr == 'c') || (*ptr == 'C'))
                    temp = 12;
                else if ((*ptr == 'd') || (*ptr == 'D'))
                    temp = 13;
                else if ((*ptr == 'e') || (*ptr == 'E'))
                    temp = 14;
                else if ((*ptr == 'f') || (*ptr == 'F'))
                    temp = 15;
                else if ((*ptr >= 48) && (*ptr <= 57))
                    temp = (int)(*ptr - 48);
                else return false;
                ++ptr;
                old = value;
                value = value * 16 + temp;
                if (old > value)
                {
                    // overflow
                    return false;
                }
            }
        }
        break;
        case 'd':
        {
            while (ptr - buf < length)
            {
                if ((*ptr >= 48) && (*ptr <= 57))
                {
                    temp = (int)(*ptr - 48);
                    ++ptr;
                    old = value;
                    value = value * 10 + temp;
                    if (old > value)
                    {
                        // overflow
                        return false;
                    }
                }
                else
                {
                    return false;
                }

            }
        }
        break;
        default :
        {
            while (ptr - buf < length)
            {
                if ((*ptr >= 48) && (*ptr <= 57))
                {
                    temp = (int)(*ptr - 48);
                    ++ptr;
                    old = value;
                    value = value * 10 + temp;
                    if (old > value)
                    {
                        // overflow
                        return false;
                    }
                }
                else
                {
                    return false;
                }
            }
        }
        break;
    }

    return true;
}

OSCL_EXPORT_REF bool PV_atoi(const char *buf, const char new_format, int length, uint64& value)
{
    uint64 old, temp;
    const char *ptr = buf;
    value = 0;

    if (buf == NULL) return false;

    switch (new_format)
    {
        case 'x':
        {
            while (ptr - buf < length)
            {
                if ((*ptr == 'a') || (*ptr == 'A'))
                    temp = 10;
                else if ((*ptr == 'b') || (*ptr == 'B'))
                    temp = 11;
                else if ((*ptr == 'c') || (*ptr == 'C'))
                    temp = 12;
                else if ((*ptr == 'd') || (*ptr == 'D'))
                    temp = 13;
                else if ((*ptr == 'e') || (*ptr == 'E'))
                    temp = 14;
                else if ((*ptr == 'f') || (*ptr == 'F'))
                    temp = 15;
                else if ((*ptr >= 48) && (*ptr <= 57))
                    temp = (int)(*ptr - 48);
                else return false;
                ++ptr;
                old = value;
                value = value * (uint64)16 + temp;
                if (old > value)
                {
                    // overflow
                    return false;
                }
            }
        }
        break;
        case 'd':
        {
            while (ptr - buf < length)
            {
                if ((*ptr >= 48) && (*ptr <= 57))
                {
                    temp = (int)(*ptr - 48);
                    ++ptr;
                    old = value;
                    value = value * (uint64)10 + temp;
                    if (old > value)
                    {
                        // overflow
                        return false;
                    }
                }
                else
                {
                    return false;
                }

            }
        }
        break;
        default :
        {
            while (ptr - buf < length)
            {
                if ((*ptr >= 48) && (*ptr <= 57))
                {
                    temp = (int)(*ptr - 48);
                    ++ptr;
                    old = value;
                    value = value * (uint64)10 + temp;
                    if (old > value)
                    {
                        // overflow
                        return false;
                    }
                }
                else
                {
                    return false;
                }
            }
        }
        break;
    }

    return true;
}




/* ======================================================================== */
/*	Function : skip_whitespace                                          */
/*	Date     : 1/1/2000				                    */
/*	Purpose  : Skips over any leading whitespace (i.e., a space or      */
/*                 horizontal tab character) in the input string and        */
/*                 returns the pointer to the first non-whitespace          */
/*                 character.                                               */
/*	In/out   :						            */
/*	Return   : pointer to first non-whitespace character                */
/*	Modified :							    */
/* ======================================================================== */

OSCL_EXPORT_REF const char* skip_whitespace(const char *ptr)
{
    while (ptr && *ptr)
    {
        if (*ptr != ' ' && *ptr != '\t')
        {
            break;
        }

        ++ptr;
    }

    return ptr;
}

OSCL_EXPORT_REF char * skip_whitespace(char * ptr)
{
    while (ptr && *ptr)
    {
        if (*ptr != ' ' && *ptr != '\t')
        {
            break;
        }

        ++ptr;
    }

    return ptr;
}



/* ======================================================================== */
/*	Function : skip_whitespace                                          */
/*	Date     : 1/1/2000				                    */
/*	Purpose  : Skips over any leading whitespace (i.e., a space or      */
/*                 horizontal tab character) in the input string and        */
/*                 returns the pointer to the first non-whitespace          */
/*                 character.  The input string is represented by           */
/*                 starting and ending pointers and does not need to be     */
/*                 NULL terminated.                                         */
/*	In/out   :						            */
/*	Return   : pointer to first non-whitespace character                */
/*	Modified :							    */
/* ======================================================================== */

OSCL_EXPORT_REF const char* skip_whitespace(const char *start, const char *end)
{
    while (start && (start < end))
    {
        if (*start != ' ' && *start != '\t')
        {
            break;
        }

        ++start;
    }

    return start;
}


/* ======================================================================== */
/*	Function : skip_to_whitespace                                       */
/*	Date     : 1/1/2001				                    */
/*	Purpose  : Skips to the first whitespace character (i.e., a space   */
/*                 or horizontal tab character or line terminator) in the   */
/*                 input string and returns the pointer to the first        */
/*                 non-whitespace character.                                */
/*                 The input string is represented by                       */
/*                 starting and ending pointers and does not need to be     */
/*                 NULL terminated.                                         */
/*	In/out   :						            */
/*	Return   : pointer to first non-whitespace character                */
/*	Modified :							    */
/* ======================================================================== */

OSCL_EXPORT_REF const char* skip_to_whitespace(const char *start, const char *end)
{
    while (start && (start < end))
    {
        if (*start == ' ' || *start == '\t' ||
                *start == '\r' || *start == '\n')
        {
            break;
        }

        ++start;
    }

    return start;
}

/* ======================================================================== */
/*	Function : skip_to_line_term                                        */
/*	Date     : 9/15/2001				                    */
/*	Purpose  : Skips over any characters to the next line terminator    */
/*                 (i.e., \r and \n) and                                    */
/*                 returns the pointer to the line term character.          */
/*                 The input string is represented by                       */
/*                 starting and ending pointers and does not need to be     */
/*                 NULL terminated.                                         */
/*	In/out   :						            */
/*	Return   : pointer to line terminator character                     */
/*	Modified :							    */
/* ======================================================================== */

OSCL_EXPORT_REF const char * skip_to_line_term(const char *start, const char *end)
{
    while (start && (start < end))
    {
        if (*start == '\r' || *start == '\n')
        {
            return start;
        }
        ++start;
    }

    return start;
}


/* ======================================================================== */
/*	Function : skip_whitespace_and_line_term                            */
/*	Date     : 9/15/2001				                    */
/*	Purpose  : Skips over any leading whitespace (i.e., a space or      */
/*                 horizontal tab character) or line terminator (i.e., \r   */
/*                 and \n) and                                              */
/*                 returns the pointer to the first non-whitespace          */
/*                 character.  The input string is represented by           */
/*                 starting and ending pointers and does not need to be     */
/*                 NULL terminated.                                         */
/*	In/out   :						            */
/*	Return   : pointer to first non-whitespace character                */
/*	Modified :							    */
/* ======================================================================== */

OSCL_EXPORT_REF const char* skip_whitespace_and_line_term(const char *start, const char *end)
{
    while (start && (start < end))
    {
        if (*start != ' ' && *start != '\t' &&
                *start != '\r' && *start != '\n')
        {
            break;
        }

        ++start;
    }

    return start;
}




/* ======================================================================== */
/*	Function : extract_string                                           */
/*	Date     : 1/1/2000				                    */
/*	Purpose  : Extracts string of a maximum size after skipping any     */
/*                 leading whitespace.  The input string is represented by  */
/*                 starting and ending pointers and does not need to be     */
/*                 NULL terminated.                                         */
/*	In/out   :						            */
/*	Return   : length of the extracted string        		    */
/*	Modified :							    */
/* ======================================================================== */


OSCL_EXPORT_REF int extract_string(const char * start, const char *end, char *outstring, int maxsize)
{
    int len = 0;

    if (! outstring)
    {
        return 0;
    }

    start = skip_whitespace(start, end);

    for (; start && (start < end) ; ++start)
    {

        if (*start == ' ' || *start == '\t' || *start == '\n' || *start == '\r')
        {
            // whitespace so stop copying
            break;
        }

        if (len < maxsize)
        {
            *outstring++ = *start;
        }
        else if (len == maxsize)
        {
            // too long so just terminate the string
            *(outstring - 1) = '\0';
        }
        ++len;

    }

    if (len < maxsize)
    {
        // terminate the string
        *outstring = '\0';
    }

    return len;
}

/* ======================================================================== */
/*	Function : extract_string                                           */
/*	Date     : 1/1/2000				                    */
/*	Purpose  : Extracts string of a maximum size after skipping any     */
/*                 leading whitespace.                                      */
/*	In/out   :						            */
/*	Return   : length of the extracted string        		    */
/*	Modified :							    */
/* ======================================================================== */

OSCL_EXPORT_REF int extract_string(const char * in_ptr, char *outstring, int maxsize)
{
    int len = 0;

    if (! outstring)
    {
        return 0;
    }

    in_ptr = skip_whitespace(in_ptr, in_ptr + oscl_strlen(in_ptr));

    for (; in_ptr && *in_ptr ; ++in_ptr)
    {

        if (*in_ptr == ' ' || *in_ptr == '\t' || *in_ptr == '\n' || *in_ptr == '\r')
        {
            // whitespace so stop copying
            break;
        }

        if (len < maxsize)
        {
            *outstring++ = *in_ptr;
        }
        else if (len == maxsize)
        {
            // too long so just terminate the string
            *(outstring - 1) = '\0';
        }
        ++len;

    }

    if (len < maxsize)
    {
        // terminate the string
        *outstring = '\0';
    }

    return len;
}

/* ======================================================================== */
/*	Function : PV_atof(char *buf, OsclFloat& value)							*/
/*	Purpose  : Converts a character string to a float. The string			*/
/*             argument has the following form:                             */
/*             [whitespace] [sign] [digits] [.digits]                       */
/*	In/out   :																*/
/*	Return   :																*/
/*	Modified :																*/
/* ======================================================================== */
OSCL_EXPORT_REF bool PV_atof(const char *buf, OsclFloat& value)
{
    return PV_atof(buf, oscl_strlen(buf), value);
}

OSCL_EXPORT_REF bool PV_atof(const char *buf, int length, OsclFloat& value)
{
    int i, decimal_point_index = -1;
    OsclFloat old;
    bool minus_sign = false;
    bool digit_found = false;
    value = 0.0;

    for (i = 0; i < length; i++)
    {
        if (buf[i] == ' ' || buf[i] == '\t')
        {
            if (digit_found)
                break; // stop the conversion if the numeric value is ended with whitespace or tab (ie, "1.23  ")
            else
                continue; // skip leading whitespace or tab
        }
        else if (buf[i] == '-')
        {
            if (digit_found)
                break; // stop the conversion if the numeric value is ended with a minus sign (ie, "1.23-")
            else
            {
                minus_sign = true;
                continue;
            }
        }
        else if (buf[i] < '0' || buf[i] > '9')
        {
            if (buf[i] == '.')
            {
                if (decimal_point_index > -1)
                    break; // found another decimal point so stopping
                else // continue the conversion if this is the first decimal point
                {
                    decimal_point_index = i;
                    continue;
                }
            }
            else
                return false;	// Non-numeric char so stopping
        }
        else
        {
            old = value;
            value = value * 10 + buf[i] - '0';
            if (old > value)
            {
                // overflow
                return false;
            }
            if (!digit_found)
                digit_found = true;
        }
    }

    if (value > 0.0)
    {
        if (decimal_point_index != -1)
        {
            i = i - 1 - decimal_point_index;
            if (i > 0)
            {
                while (i--)
                    value /= 10;
            }
        }
        if (minus_sign)
            value = (OsclFloat)(0.0 - value);
    }
    return true;
}

OSCL_EXPORT_REF int oscl_abs(int aVal)
{
    return(aVal < 0 ? -aVal : aVal);
}

