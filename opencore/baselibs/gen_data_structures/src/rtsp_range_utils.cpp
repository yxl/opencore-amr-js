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
#include "oscl_mem_basic_functions.h"
#include "oscl_snprintf.h"
#include "oscl_string_utils.h"
#include "oscl_str_ptr_len.h"
#include "oscl_string.h"
#include "rtsp_time_formats.h"
#include "rtsp_range_utils.h"


#include "oscl_dll.h"
OSCL_DLL_ENTRY_POINT_DEFAULT()


// don't make the max int size less than 6 -- it is
// used to size certain arrays below that need at least 6 characters
static const int  MAX_RANGE_INT_SIZE = 10;

// keep the next two constants in sync.  The max range float
// size should be the number of digits plus the decimal point
static const int  MAX_RANGE_FLOAT_SIZE = 7;
static const char RANGE_FLOAT_FORMAT[] = "%0.6f";

OSCL_EXPORT_REF const char * parse_range_integer(const char *start, const char *end,
        int max_digits, char *sep, uint32& value)
{
    const char *sptr, *eptr;
    const char *endpoint;

    if (max_digits > 0 && ((end - start) > max_digits))
    {
        endpoint = start + max_digits;
    }
    else
    {
        endpoint = end;
    }

    sptr = start;

    if (sep)
    {
        for (eptr = sptr; eptr < endpoint &&
                *eptr != *sep ; ++eptr);
    }
    else
    {
        eptr = endpoint;
    }

    // get the hours
    uint32 tmp;
    if (PV_atoi(sptr, 'd',  eptr - sptr, tmp) == false)
    {
        return NULL;
    }

    value = tmp;
    return eptr;

}

OSCL_EXPORT_REF bool parse_smpte_format(const char* start_ptr, const char *end_ptr,
                                        RtspRangeType::RtspRangeFormat format,
                                        SmpteTimeFormat& smpte_range)
{

    // get required HH:MM:SS values
    const char *sptr;

    sptr = start_ptr;
    uint32 tmp;
    char sep;

    sep = ':';
    sptr = parse_range_integer(sptr, end_ptr,
                               2, &sep, tmp);

    if (!sptr || *sptr != ':')
    {
        return false;
    }

    smpte_range.hours = (uint8)tmp;

    // get the minutes
    sptr = parse_range_integer(sptr + 1, end_ptr,
                               2, &sep, tmp);

    if (!sptr || *sptr != ':')
    {
        return false;
    }

    if (tmp > 59)
    {
        return false;
    }

    smpte_range.minutes = (uint8)tmp;

    // get the seconds
    sptr = parse_range_integer(sptr + 1, end_ptr,
                               2, &sep, tmp);

    if (!sptr)
    {
        return false;
    }

    if (tmp > 59)
    {
        return false;
    }

    smpte_range.seconds = (uint8)tmp;

    // now see if there there are any optional fields
    if (*sptr == ':')
    {
        // get the number of frames
        sep = '.';
        sptr = parse_range_integer(sptr + 1, end_ptr,
                                   2, &sep, tmp);

        if (!sptr)
        {
            return false;
        }

        if (format == RtspRangeType::SMPTE_25_RANGE &&
                tmp > 25)
        {
            return false;
        }
        else if (tmp > 30)
        {
            return false;
        }

        smpte_range.frames = (uint8)tmp;

        // now check for optional subframes
        if (*sptr == '.')
        {
            // get the number of subframes
            sptr = parse_range_integer(sptr + 1, end_ptr,
                                       2, NULL, tmp);

            if (!sptr)
            {
                return false;
            }

            if (tmp > 99)
            {
                return false;
            }

            smpte_range.subframes = (uint8)tmp;
        }
        else
        {
            smpte_range.subframes = 0;
        }

    }
    else
    {
        smpte_range.frames = 0;
        smpte_range.subframes = 0;
    }

    return true;
}

OSCL_EXPORT_REF bool parse_npt_format(const char* start_ptr, const char *end_ptr,
                                      NptTimeFormat& npt_range)
{

    // get required HH:MM:SS values
    const char *sptr, *eptr;
    uint32 tmp;

    sptr = start_ptr;

    StrPtrLen now_str("now");
    if (!oscl_strncmp(sptr, now_str.c_str(), now_str.length()))
    {
        // this is the "now" keyword
        npt_range.npt_format = NptTimeFormat::NOW;
        return true;
    }

    // see if the format contains a ':' separator character
    for (eptr = sptr; eptr < end_ptr && *eptr != ':'; ++eptr);

    if (*eptr == ':')
    {
        // this is the npt-hhmmss format

        char sep = ':';
        // get the number of hours
        sptr = parse_range_integer(sptr, eptr,
                                   0, &sep, tmp);

        if (!sptr)
        {
            return false;
        }

        npt_range.npt_format = NptTimeFormat::NPT_HHMMSS;
        npt_range.npt_hhmmss.hours = tmp;


        // get the number of minutes
        sptr = parse_range_integer(sptr + 1, end_ptr,
                                   2, &sep, tmp);

        if (!sptr || *sptr != ':')
        {
            return false;
        }

        if (tmp > 59)
        {
            return false;
        }

        npt_range.npt_hhmmss.min = (uint8)tmp;


        sep = '.';
        // get the number of seconds
        sptr = parse_range_integer(sptr + 1, end_ptr,
                                   2, &sep, tmp);

        if (!sptr)
        {
            return false;
        }

        if (tmp > 59)
        {
            return false;
        }

        npt_range.npt_hhmmss.sec = (uint8)tmp;

        npt_range.npt_hhmmss.frac_sec = 0;
        // determine if the fractional seconds exists
        if (*sptr == '.')
        {
            // get the fractional seconds
            const int MAX_TMP_BUFSIZE = 12;
            char tmpbuf[MAX_TMP_BUFSIZE];
            int copy_size;

            eptr = skip_to_whitespace(sptr, end_ptr);

            copy_size = eptr - sptr;
            if (copy_size > MAX_TMP_BUFSIZE - 1)
            {
                copy_size = MAX_TMP_BUFSIZE - 1;
            }

            oscl_strncpy(tmpbuf, sptr, copy_size);

            tmpbuf[copy_size] = '\0';

            if (!PV_atof(tmpbuf, npt_range.npt_hhmmss.frac_sec))
                return false;

        }

    } // end if the format is NPT_HHMMSS

    else
    {

        char sep = '.';
        // this is the NPT_SEC format
        npt_range.npt_format = NptTimeFormat::NPT_SEC;

        // get the number of seconds
        sptr = parse_range_integer(sptr, eptr,
                                   0, &sep, tmp);

        if (!sptr)
        {
            return false;
        }

        npt_range.npt_sec.sec = tmp;

        npt_range.npt_sec.milli_sec = 0;
        if (*sptr == '.')
        {
            // there is an optional fractional seconds field

            // get the fractional seconds
            const int MAX_TMP_BUFSIZE = 12;
            char tmpbuf[MAX_TMP_BUFSIZE];
            int copy_size;

            eptr = skip_to_whitespace(sptr, end_ptr);

            copy_size = eptr - sptr;

            if (copy_size > MAX_TMP_BUFSIZE - 1)
            {
                copy_size = MAX_TMP_BUFSIZE - 1;
            }

            oscl_strncpy(tmpbuf, sptr, copy_size);


            tmpbuf[copy_size] = '\0';

            OsclFloat tmp_fnum;
            if (!PV_atof(tmpbuf, tmp_fnum))
                return false;
            npt_range.npt_sec.milli_sec = (uint32)(1000.0 * tmp_fnum + 0.5);
        }

    } // end if the format is NPT_SEC

    return true;
}


OSCL_EXPORT_REF bool parse_abs_format(const char* start_ptr, const char *end_ptr,
                                      AbsTimeFormat& abs_range)
{

    // get format YYYYMMDDT
    const char *sptr, *eptr;
    char sep;
    uint32 tmp;

    sptr = start_ptr;

    // check if the ninth character is a "T"
    eptr = sptr + 8;

    sep = 'T';
    if (eptr >= end_ptr || *eptr != sep)
    {
        return false;
    }

    sptr = parse_range_integer(sptr, eptr,
                               4, &sep, tmp);

    if (! sptr || tmp > 9999)
    {
        return false;
    }

    abs_range.year = (uint16)tmp;

    // get the month
    sptr = parse_range_integer(sptr, eptr,
                               2, &sep, tmp);

    if (! sptr || tmp == 0 || tmp > 12)
    {
        return false;
    }

    abs_range.month = (uint8)tmp;

    // get the day
    sptr = parse_range_integer(sptr, eptr,
                               2, &sep, tmp);

    if (! sptr || tmp == 0 || tmp > 31)
    {
        return false;
    }

    abs_range.day = (uint8)tmp;


    sptr = eptr + 1;
    eptr = sptr + 6;

    if (eptr >= end_ptr || (*eptr != 'Z' && *eptr != '.'))
    {
        return false;
    }

    // get the hours
    sptr = parse_range_integer(sptr, eptr,
                               2, NULL, tmp);

    if (! sptr || tmp > 23)
    {
        return false;
    }

    abs_range.hours = (uint8)tmp;

    // get the minutes
    sptr = parse_range_integer(sptr, eptr,
                               2, NULL, tmp);

    if (! sptr || tmp > 59)
    {
        return false;
    }

    abs_range.min = (uint8)tmp;

    // get the number of seconds
    sptr = parse_range_integer(sptr, eptr,
                               2, NULL, tmp);

    if (! sptr || tmp > 59)
    {
        return false;
    }

    abs_range.sec = (uint8)tmp;

    abs_range.frac_sec = 0;

    if (*eptr == '.')
    {
        // get the fractional seconds
        // get the fractional seconds
        const int MAX_TMP_BUFSIZE = 12;
        char tmpbuf[MAX_TMP_BUFSIZE];
        int copy_size;

        sptr = eptr;

        for (eptr = sptr; eptr < end_ptr && *eptr != 'Z';
                ++eptr);

        if (*eptr != 'Z')
        {
            return false;
        }

        copy_size = eptr - sptr;
        if (copy_size > MAX_TMP_BUFSIZE - 1)
        {
            copy_size = MAX_TMP_BUFSIZE - 1;
        }

        oscl_strncpy(tmpbuf, sptr, copy_size);

        tmpbuf[copy_size] = '\0';

        if (!PV_atof(tmpbuf, abs_range.frac_sec))
            return false;

    }

    return true;
}


OSCL_EXPORT_REF bool parseRtspRange(const char *rangeString, int length, RtspRangeType& range)
{
    const char *end = rangeString + length;

    const char* sptr, *eptr;

    // initialize range to invalid format
    range.format = RtspRangeType::INVALID_RANGE;

    // find the first word before the "="
    sptr = skip_whitespace_and_line_term(rangeString, end);
    if (sptr >= end)
    {
        return false;
    }

    for (eptr = sptr; eptr < end &&
            (*eptr != '=' && *eptr != ':' && *eptr != ' ' && *eptr != '\t');
            ++eptr);


    StrPtrLen smpte_type("smpte");
    StrPtrLen smpte25_type("smpte-25");
    StrPtrLen smpte30_type("smpte-30-drop");
    StrPtrLen npt_type("npt");
    StrPtrLen abs_type("clock");
//#ifdef RTSP_PLAYLIST_SUPPORT
    StrPtrLen playlist_play_time_type("playlist_play_time");
//#endif //#ifdef RTSP_PLAYLIST_SUPPORT

    if (!oscl_strncmp(sptr, smpte_type.c_str(), eptr - sptr) ||
            !oscl_strncmp(sptr, smpte25_type.c_str(), eptr - sptr) ||
            !oscl_strncmp(sptr, smpte30_type.c_str(), eptr - sptr))
    {
        // Parsing one of the SMPTE time formats

        // save the exact format temporarily
        RtspRangeType::RtspRangeFormat tmp_format;
        if (!oscl_strncmp(sptr, smpte30_type.c_str(), smpte30_type.length()))
        {
            tmp_format = RtspRangeType::SMPTE_30_RANGE;
        }
        else if (!oscl_strncmp(sptr, smpte25_type.c_str(), smpte25_type.length()))
        {
            tmp_format = RtspRangeType::SMPTE_25_RANGE;
        }
        else
        {
            tmp_format = RtspRangeType::SMPTE_RANGE;
        }

        // skip ahead to beyond the "="
        if (*eptr != '=')
        {
            for (; eptr < end && *eptr != '='; ++eptr);
        }

        sptr = skip_whitespace(eptr + 1, end);
        if (sptr >= end)
        {
            return false;
        }

        // find the start/end separator
        for (eptr = sptr; eptr < end &&
                (*eptr != '-'); ++eptr);

        if (*eptr != '-')
        {
            return false;
        }

        range.start_is_set = false;
        if (eptr > sptr)
        {
            // there is a start time

            if (parse_smpte_format(sptr, eptr, tmp_format, range.smpte_start) == false)
            {
                return false;
            }

            // now set the appropriate flags
            range.start_is_set = true;
        }

        // see if there is a stop time
        sptr = skip_whitespace_and_line_term(eptr + 1, end);
        range.end_is_set = false;
        if (sptr < end)
        {
            // there is a stop time specification
            eptr = skip_to_whitespace(sptr, end);

            if (parse_smpte_format(sptr, eptr, tmp_format, range.smpte_end)
                    == false)
            {
                return false;
            }

            // now set the appropriate flags
            range.end_is_set = true;
        }

        // now set the appropriate range format
        range.format = tmp_format;

    } // end if this is an SMPTE time format

    else if (!oscl_strncmp(sptr, npt_type.c_str(), eptr - sptr))
    {

        // skip ahead to beyond the "=" or ":"
        if (*eptr != '=')
        {
            for (; eptr < end && *eptr != '=' && *eptr != ':'; ++eptr);
        }

        sptr = skip_whitespace(eptr + 1, end);
        if (sptr >= end)
        {
            return false;
        }

        // find the start/end separator
        for (eptr = sptr; eptr < end &&
                (*eptr != '-'); ++eptr);

        if (*eptr != '-')
        {
            return false;
        }

        range.start_is_set = false;
        if (eptr > sptr)
        {
            // there is a start time
            if (parse_npt_format(sptr, eptr, range.npt_start) == false)
            {
                return false;
            }

            // now set the appropriate flags
            range.start_is_set = true;

        }

        // see if there is a stop time
        range.end_is_set = false;
        sptr = skip_whitespace_and_line_term(eptr + 1, end);
        if (sptr < end)
        {
            // there is a stop time specification
            eptr = skip_to_whitespace(sptr, end);

            if (parse_npt_format(sptr, eptr, range.npt_end)
                    == false)
            {
                return false;
            }

            // now set the appropriate flags
            range.end_is_set = true;
        }

        // now set the appropriate range format
        range.format = RtspRangeType::NPT_RANGE;

    } // end if this is an NPT time format

    else if (!oscl_strncmp(sptr, abs_type.c_str(), eptr - sptr))
    {


        // skip ahead to beyond the "="
        if (*eptr != '=')
        {
            for (; eptr < end && *eptr != '='; ++eptr);
        }

        sptr = skip_whitespace(eptr + 1, end);
        if (sptr >= end)
        {
            return false;
        }

        // find the start/end separator
        for (eptr = sptr; eptr < end &&
                (*eptr != '-'); ++eptr);

        if (*eptr != '-')
        {
            return false;
        }

        range.start_is_set = false;
        if (eptr > sptr)
        {
            // there is a start time
            if (parse_abs_format(sptr, eptr, range.abs_start) == false)
            {
                return false;
            }

            // now set the appropriate flags
            range.start_is_set = true;

        }

        // see if there is a stop time
        sptr = skip_whitespace_and_line_term(eptr + 1, end);
        range.end_is_set = true;
        if (sptr < end)
        {
            // there is a stop time specification
            eptr = skip_to_whitespace(sptr, end);

            if (parse_abs_format(sptr, eptr, range.abs_end)
                    == false)
            {
                return false;
            }

            // now set the appropriate flags
            range.end_is_set = true;
        }

        // now set the appropriate range format
        range.format = RtspRangeType::ABS_RANGE;

    } // end if this is an ABS time format
//#ifdef RTSP_PLAYLIST_SUPPORT
    // for Range:playlist_play_time=<URN,clipIndex,clipOffset>
    //playlist_play_time=</public/playlist/va_playlists/test.ply,3,0.0>;npt=194.81542
    else if (!oscl_strncmp(sptr, playlist_play_time_type.c_str(), eptr - sptr))
    {
        // store the whole string since we may not need the parsed version of things
        //oscl_memcpy(range.iPlaylistPlayStr,rangeString,length);
        //range.iPlaylistPlayStr[length] = '\0';

        range.format = RtspRangeType::PLAYLIST_TIME_RANGE;
        range.start_is_set = range.end_is_set = false;

        // now set the appropriate flags
        range.start_is_set = true;

        // skip ahead to beyond the "="
        if (*eptr != '=')
        {
            for (; eptr < end && *eptr != '='; ++eptr);
        }

        sptr = skip_whitespace(eptr + 1, end);
        if (sptr >= end)
        {
            return false;
        }

        // next should be the opening "<"
        // skip ahead to beyond the "<"
        if (*eptr != '<')
        {
            for (; eptr < end && *eptr != '<'; ++eptr);
        }

        sptr = skip_whitespace(eptr + 1, end);
        if (sptr >= end)
        {
            return false;
        }

        // find the comma separator
        for (eptr = sptr; eptr < end &&
                (*eptr != ','); ++eptr);

        if (*eptr != ',')
        {
            return false;
        }

        // first the urn
        if (eptr > sptr)
        {
            // there is a urn
            if (oscl_memcpy(range.iPlaylistUrl, sptr, eptr - sptr) == false)
            {
                return false;
            }
            //range.iUrn[(eptr-sptr)+1] = '\0';
            range.iPlaylistUrl[eptr-sptr] = '\0';
        }

        // now the clipIndex
        sptr = skip_whitespace(eptr + 1, end);
        if (sptr >= end)
        {
            return false;
        }

        // find the next comma separator
        for (eptr = sptr; eptr < end &&
                (*eptr != ','); ++eptr);

        if (*eptr != ',')
        {
            return false;
        }

        // now the clipIndex
        if (eptr > sptr)
        {
            // there is a clipIndex
            uint32 tmp;
            if (PV_atoi(sptr, 'd', eptr - sptr, tmp) == false)
            {
                return false;
            }
            range.playlist_start.iClipIndex = (int32)tmp;
        }

        // now the clipOffset
        sptr = skip_whitespace(eptr + 1, end);
        if (sptr >= end)
        {
            return false;
        }

        // find the final '>' separator or the final possible '.' in offset
        //<sec>.<frac>
        for (eptr = sptr; eptr < end &&
                (*eptr != '>') && (*eptr != '.'); ++eptr);

        if (eptr >= end)
        {
            return false;
        }

        // @todo ignore the factional part for now
        // now the clipOffset
        if (eptr > sptr)
        {
            // there is a clipOffset
            uint32 tmp;
            if (PV_atoi(sptr, 'd', eptr - sptr, tmp) == false)
            {
                return false;
            }
            range.playlist_start.sec = (int32)tmp;

            {
                range.playlist_start.milli_sec = 0;
                if (*eptr == '.')
                {
                    // there is an optional fractional seconds field

                    // get the fractional seconds
                    const int MAX_TMP_BUFSIZE = 12;
                    char tmpbuf[MAX_TMP_BUFSIZE];
                    int copy_size;

                    eptr = skip_to_whitespace(sptr, end);

                    copy_size = eptr - sptr;

                    if (copy_size > MAX_TMP_BUFSIZE - 1)
                    {
                        copy_size = MAX_TMP_BUFSIZE - 1;
                    }

                    oscl_strncpy(tmpbuf, sptr, copy_size);


                    tmpbuf[copy_size] = '\0';

                    OsclFloat tmp_fnum;
                    if (!PV_atof(tmpbuf, tmp_fnum))
                        return false;
                    range.playlist_start.milli_sec = (uint32)(1000.0 * tmp_fnum + 0.5);
                }
            }
        }
    }  // end if this is a playlist_play_time format, for response to playlist_play commands
//#endif //#ifdef RTSP_PLAYLIST_SUPPORT
    else
    {
        /*Unsupported time format*/
        range.format = RtspRangeType::UNKNOWN_RANGE;
        range.start_is_set = false;
        range.end_is_set = false;
        return false;
    }

    return true;
}

OSCL_EXPORT_REF int estimate_SDP_string_len(const NptTimeFormat& npt_range)
{
    int total_len = 0;


    switch (npt_range.npt_format)
    {
        case NptTimeFormat::NOW:
            total_len += 3; // enough for "now"
            break;

        case NptTimeFormat::NPT_SEC:
            // compute the size for the sec field
            if (npt_range.npt_sec.sec < 1000)
            {
                total_len += 3; // enough for at most 3 digits
            }
            else if (npt_range.npt_sec.sec < 1000000)
            {
                total_len += 6; // enough for 6 digits
            }
            else
            {
                total_len += 10; // enough for MAX_INT of uint32
            }

            if (npt_range.npt_sec.milli_sec > 0.0)
            {
                total_len += MAX_RANGE_FLOAT_SIZE; // includes "." and all digits
            }
            break;

        case NptTimeFormat::NPT_HHMMSS:
            if (npt_range.npt_hhmmss.hours < 1000)
            {
                total_len += 3; // enough for at most 3 digits
            }
            else if (npt_range.npt_hhmmss.hours < 1000000)
            {
                total_len += 6; // enough for 6 digits
            }
            else
            {
                total_len += 10; // enough for MAX_INT of uint32
            }

            total_len += 6; // enough for ":MM:SS"

            if (npt_range.npt_hhmmss.frac_sec > 0.0)
            {
                total_len += MAX_RANGE_FLOAT_SIZE; // includes "." and all digits
            }
            break;
    }

    return total_len;
}


OSCL_EXPORT_REF int estimate_SDP_string_len(const RtspRangeType& range)
{
    // this function computes the string size necessary for
    // holding the SDP string representation of the range

    int total_len = 0;

    total_len += 8 ; // this the length of the "a=range:" string

    switch (range.format)
    {

        case RtspRangeType::NPT_RANGE:
            total_len += 4; // this is the length of the "npt=" string

            if (range.start_is_set)
            {
                total_len += estimate_SDP_string_len(range.npt_start);
            }
            else if (!range.end_is_set)
            {
                // return 0 in this case
                return 0;
            }

            total_len += 1; // enough for "-"

            if (range.end_is_set)
            {
                total_len += estimate_SDP_string_len(range.npt_end);
            }

            total_len += 2; // enough for CRLF at the end of the line

            break;


        case RtspRangeType::SMPTE_RANGE:
        case RtspRangeType::SMPTE_25_RANGE:
        case RtspRangeType::SMPTE_30_RANGE:
            if (range.format == RtspRangeType::SMPTE_30_RANGE)
            {
                total_len += 14; // this is the length of the "smpte-30-drop=" string
            }
            else if (range.format == RtspRangeType::SMPTE_25_RANGE)
            {
                total_len += 9; // this is the length of the "smpte-25=" string
            }
            else
            {
                total_len += 6; // this is enough for "smpte="
            }

            if (range.start_is_set)
            {
                total_len += 8; // enough for "XX:YY:ZZ"
                if (range.smpte_start.frames || range.smpte_start.subframes)
                {
                    total_len += 3; // enough for ":FF"
                    if (range.smpte_start.subframes)
                    {
                        total_len += 3; // enough for ".SS"
                    }
                }
            }
            else if (!range.end_is_set)
            {
                // return 0 in this case
                return 0;
            }

            total_len += 1; // enough for "-"

            if (range.end_is_set)
            {
                total_len += 8; // enough for "XX:YY:ZZ"
                if (range.smpte_end.frames || range.smpte_end.subframes)
                {
                    total_len += 3; // enough for ":FF"
                    if (range.smpte_end.subframes)
                    {
                        total_len += 3; // enough for ".SS"
                    }
                }
            }

            total_len += 2; // enough for CRLF at the end of the line

            break;

        case RtspRangeType::ABS_RANGE:

            total_len += 6; // this is the length of the "clock=" string

            if (range.start_is_set)
            {
                total_len += 15; // enough for "YYYYMMDDTHHMMSS"
                if (range.abs_start.frac_sec > 0.0)
                {
                    total_len += MAX_RANGE_FLOAT_SIZE; // includes "." and all digits
                }
                total_len += 1; // enough for the "Z"
            }
            else if (!range.end_is_set)
            {
                // return 0 in this case
                return 0;
            }

            total_len += 1; // enough for "-"

            if (range.end_is_set)
            {
                total_len += 15; // enough for "YYYYMMDDTHHMMSS"
                if (range.abs_end.frac_sec > 0.0)
                {
                    total_len += MAX_RANGE_FLOAT_SIZE; // includes "." and all digits
                }
                total_len += 1; // enough for the "Z"
            }

            total_len += 2; // enough for CRLF at the end of the line

            break;


        default:
            return 0;
            break;
    }

    return total_len;
}



OSCL_EXPORT_REF bool compose_range_string(char *str, unsigned int max_len,
        const NptTimeFormat& npt_range,
        int& len_used)
{
    len_used = 0;
    int length;


    switch (npt_range.npt_format)
    {
        case NptTimeFormat::NOW:
        {
            StrPtrLen now_str("now");
            if ((int)max_len < now_str.length())
            {
                return false;
            }
            oscl_memcpy(str, now_str.c_str(), now_str.length());
            str += now_str.length();
            len_used += now_str.length();
            max_len -= now_str.length();
            break;
        }

        case NptTimeFormat::NPT_SEC:
        {
            char tmpstr[MAX_RANGE_INT_SIZE + 1];
            length = oscl_snprintf(tmpstr, MAX_RANGE_INT_SIZE + 1, "%d",
                                   npt_range.npt_sec.sec);
            if (length < 0 || length > MAX_RANGE_INT_SIZE)
            {
                return false;
            }
            if (length > (int) max_len)
            {
                return false;
            }
            oscl_memcpy(str, tmpstr, length);
            str += length;
            len_used += length;
            max_len -= length;
            if (npt_range.npt_sec.milli_sec > 0.0)
            {
                if (npt_range.npt_sec.milli_sec >= 1.0)
                {
                    return false;
                }
                char tmp[MAX_RANGE_FLOAT_SIZE + 2];
                length = oscl_snprintf(tmp, MAX_RANGE_FLOAT_SIZE + 2, RANGE_FLOAT_FORMAT,
                                       npt_range.npt_sec.milli_sec);
                if (length < 0 || length > MAX_RANGE_FLOAT_SIZE + 1)
                {
                    return false;
                }
                if (length > (int)max_len || tmp[1] != '.')
                {
                    return false;
                }

                oscl_memcpy(str, tmp + 1, length - 1);
                str += length - 1;
                len_used += length - 1;
                max_len -= (length - 1);
            }

            break;
        }

        case NptTimeFormat::NPT_HHMMSS:
        {
            char tmpstr[MAX_RANGE_INT_SIZE + 1];
            length = oscl_snprintf(tmpstr, MAX_RANGE_INT_SIZE + 1, "%d",
                                   npt_range.npt_hhmmss.hours);
            if (length < 0 || length >= MAX_RANGE_INT_SIZE)
            {
                return false;
            }
            if (length > (int) max_len)
            {
                return false;
            }
            oscl_memcpy(str, tmpstr, length);
            str += length;
            len_used += length;
            max_len -= length;

            if (max_len < 6)
            {
                return false;
            }

            length = oscl_snprintf(tmpstr, MAX_RANGE_INT_SIZE + 1, ":%02d:%02d",
                                   npt_range.npt_hhmmss.min, npt_range.npt_hhmmss.sec);
            if (length != 6)
            {
                return false;
            }

            oscl_memcpy(str, tmpstr, length);
            str += length;
            len_used += length;
            max_len -= length;


            if (npt_range.npt_hhmmss.frac_sec > 0.0)
            {
                if (npt_range.npt_hhmmss.frac_sec >= 1.0)
                {
                    return false;
                }
                char tmp[MAX_RANGE_FLOAT_SIZE + 2];
                length = oscl_snprintf(tmp, MAX_RANGE_FLOAT_SIZE + 2, RANGE_FLOAT_FORMAT,
                                       npt_range.npt_hhmmss.frac_sec);
                if (length < 0 || length > MAX_RANGE_FLOAT_SIZE + 1)
                {
                    return false;
                }
                if (length > (int) max_len || tmp[1] != '.')
                {
                    return false;
                }

                oscl_memcpy(str, tmp + 1, length - 1);
                str += length - 1;
                len_used += length - 1;
                max_len -= (length - 1);
            }

            break;
        }

    }

    return true;

}


OSCL_EXPORT_REF bool compose_range_string(char *str, unsigned int max_len,
        const SmpteTimeFormat& smpte_range,
        int& len_used)
{
    len_used = 0;
    int length;


    char tmp[10];

    if (max_len < 8)
    {
        return false;
    }

    length = oscl_snprintf(tmp, 9, "%02d:%02d:%02d", smpte_range.hours,
                           smpte_range.minutes, smpte_range.seconds);

    if (length != 8)
    {
        return false;
    }

    oscl_memcpy(str, tmp , length);
    str += length;
    len_used += length;
    max_len -= length;

    if (smpte_range.frames)
    {
        if (max_len < 3)
        {
            return 0;
        }
        length = oscl_snprintf(tmp, 4, ":%02d", smpte_range.frames);
        if (length != 3)
        {
            return false;
        }

        oscl_memcpy(str, tmp , length);
        str += length;
        len_used += length;
        max_len -= length;
    }

    if (smpte_range.subframes)
    {
        if (max_len < 3)
        {
            return 0;
        }
        length = oscl_snprintf(tmp, 4, ".%02d", smpte_range.subframes);
        if (length != 3)
        {
            return false;
        }

        oscl_memcpy(str, tmp , length);
        str += length;
        len_used += length;
        max_len -= length;
    }

    return true;

}


OSCL_EXPORT_REF bool compose_range_string(char *str, unsigned int max_len,
        const AbsTimeFormat& abs_range,
        int& len_used)
{
    len_used = 0;
    int length;


    char tmp[17];

    if (max_len < 16)
    {
        return false;
    }

    length = oscl_snprintf(tmp, 17, "%04d%02d%02dT%02d%02d%02d",
                           abs_range.year, abs_range.month, abs_range.day,
                           abs_range.hours, abs_range.min, abs_range.sec);

    if (length != 15)
    {
        return false;
    }

    oscl_memcpy(str, tmp , length);
    str += length;
    len_used += length;
    max_len -= length;

    if (abs_range.frac_sec > 0.0)
    {
        if (abs_range.frac_sec >= 1.0)
        {
            return false;
        }
        char tmpstr[MAX_RANGE_FLOAT_SIZE + 2];
        length = oscl_snprintf(tmpstr, MAX_RANGE_FLOAT_SIZE + 2, RANGE_FLOAT_FORMAT,
                               abs_range.frac_sec);
        if (length < 0 || length > MAX_RANGE_FLOAT_SIZE + 1)
        {
            return false;
        }
        if (length > (int)max_len || tmp[1] != '.')
        {
            return false;
        }

        oscl_memcpy(str, tmp + 1, length - 1);
        str += length - 1;
        len_used += length - 1;
        max_len -= (length - 1);
    }

    if (max_len < 1)
    {
        return false;
    }

    *str = 'Z';
    len_used++;

    return true;

}


OSCL_EXPORT_REF bool compose_range_string(char *str, unsigned int max_len, const RtspRangeType& range,
        int& len_used)
{
    len_used = 0;
    int length;

    if (!range.start_is_set && !range.end_is_set)
    {
        return false;
    }

    switch (range.format)
    {
        case RtspRangeType::NPT_RANGE:
        {
            StrPtrLen npt_str("npt=");

            if (max_len < (unsigned int)npt_str.length())
            {
                return false;
            }

            oscl_memcpy(str, npt_str.c_str(), npt_str.length());

            str += npt_str.length();
            max_len -= npt_str.length();
            len_used += npt_str.length();

            if (range.start_is_set)
            {
                if (compose_range_string(str, max_len, range.npt_start,
                                         length) != true)
                {
                    return false;
                }

                len_used += length;
                max_len -= length;
                str += length;
            }

            if (max_len < 1)
            {
                return false;
            }

            *str++ = '-';
            --max_len;
            ++len_used;

            if (range.end_is_set)
            {
                if (compose_range_string(str, max_len, range.npt_end,
                                         length) != true)
                {
                    return false;
                }

                len_used += length;
                max_len -= length;
                str += length;
            }

            break;
        }

        case RtspRangeType::SMPTE_RANGE:
        case RtspRangeType::SMPTE_25_RANGE:
        case RtspRangeType::SMPTE_30_RANGE:
        {
            StrPtrLen smpte30_str("smpte-30-drop=");
            StrPtrLen smpte25_str("smpte-25=");
            StrPtrLen smpte_str("smpte=");

            if (range.format == RtspRangeType::SMPTE_30_RANGE)
            {
                if (max_len < (unsigned int)smpte30_str.length())
                {
                    return false;
                }
                oscl_memcpy(str, smpte30_str.c_str(), smpte30_str.length());

                str += smpte30_str.length();
                max_len -= smpte30_str.length();
                len_used += smpte30_str.length();

            }
            else if (range.format == RtspRangeType::SMPTE_25_RANGE)
            {
                if (max_len < (unsigned int)smpte25_str.length())
                {
                    return false;
                }

                oscl_memcpy(str, smpte25_str.c_str(), smpte25_str.length());

                str += smpte25_str.length();
                max_len -= smpte25_str.length();
                len_used += smpte25_str.length();
            }
            else
            {
                if (max_len < (unsigned int)smpte_str.length())
                {
                    return false;
                }

                oscl_memcpy(str, smpte_str.c_str(), smpte_str.length());
                str += smpte_str.length();
                max_len -= smpte_str.length();
                len_used += smpte_str.length();

            }

            if (range.start_is_set)
            {
                if (compose_range_string(str, max_len, range.smpte_start,
                                         length) != true)
                {
                    return false;
                }

                len_used += length;
                max_len -= length;
                str += length;
            }

            if (max_len < 1)
            {
                return false;
            }

            *str++ = '-';
            --max_len;
            ++len_used;


            if (range.end_is_set)
            {
                if (compose_range_string(str, max_len, range.smpte_end,
                                         length) != true)
                {
                    return false;
                }

                len_used += length;
                max_len -= length;
                str += length;
            }

            break;
        }

        case RtspRangeType::ABS_RANGE:
        {
            StrPtrLen abs_str("clock=");

            if (max_len < (unsigned int)abs_str.length())
            {
                return false;
            }

            oscl_memcpy(str, abs_str.c_str(), abs_str.length());

            str += abs_str.length();
            max_len -= abs_str.length();
            len_used += abs_str.length();

            if (range.start_is_set)
            {
                if (compose_range_string(str, max_len, range.abs_start,
                                         length) != true)
                {
                    return false;
                }

                len_used += length;
                max_len -= length;
                str += length;
            }

            if (max_len < 1)
            {
                return false;
            }

            *str++ = '-';
            --max_len;
            ++len_used;


            if (range.end_is_set)
            {
                if (compose_range_string(str, max_len, range.abs_end,
                                         length) != true)
                {
                    return false;
                }

                len_used += length;
                max_len -= length;
                str += length;
            }

            break;
        }

        default:
            return false;
    }

    if (max_len < 2)
    {
        // can't add line terminator
        return false;
    }

    // add the line terminator
    *str++ = '\r';
    *str++ = '\n';
    len_used += 2;


    return true;

}




OSCL_EXPORT_REF bool compose_SDP_string(char *str, unsigned int max_len,
                                        const RtspRangeType& range,
                                        int& len_used)
{
    len_used = 0;

    // first verify that there is something to do
    if (range.format == RtspRangeType::UNKNOWN_RANGE ||
            range.format == RtspRangeType::INVALID_RANGE)
    {
        // nothing to output but it is not an error
        return true;
    }

    const char *range_str = "a=range:";
    int length = oscl_strlen(range_str);
    // output the "a=range:" string
    if ((int)max_len < length)
    {
        return false;
    }

    oscl_memcpy(str, range_str, length);

    str += length;
    len_used += length;
    max_len -= length;

    if (compose_range_string(str, max_len, range, length) != true)
    {
        len_used += length;
        return false;
    }

    len_used += length;

    return true;
}



OSCL_EXPORT_REF bool compose_RTSP_string(char *str, unsigned int max_len,
        const RtspRangeType& range,
        int& len_used)
{
    len_used = 0;

    // first verify that there is something to do
    if (range.format == RtspRangeType::UNKNOWN_RANGE ||
            range.format == RtspRangeType::INVALID_RANGE)
    {
        // nothing to output but it is not an error
        return true;
    }

    const char *range_str = "Range: ";
    int length = oscl_strlen(range_str);
    // output the "Range: " string
    if ((int)max_len < length)
    {
        return false;
    }

    oscl_memcpy(str, range_str, length);

    str += length;
    len_used += length;
    max_len -= length;

    if (compose_range_string(str, max_len, range, length) != true)
    {
        len_used += length;
        return false;
    }

    len_used += length;

    return true;
}




