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

#include "oscl_time.h"
#include "oscl_assert.h"
#include "oscl_stdstring.h"

#if (OSCL_DISABLE_INLINES)
#include "oscl_time.inl"
#endif


//---OSCL_HAS_UNIX_TIME_FUNCS---//

const uint32 origin_offset = 2208988800U;
const uint64 freq_conv = 281474977;

int NTPTime::set_to_current_time()
{
    struct timeval tv;
    uint64 tmp;

    // get the current time
    // This gives back time since 0h Jan 1, 1970
    gettimeofday(&tv, 0);

    timevalue = origin_offset;
    // now convert to proper time origin
    timevalue += tv.tv_sec;

    timevalue <<= 32;

    // now convert the frequency of the fractional part
    // from usec to 2^32 ticks/sec.
    if (tv.tv_usec > 0)
    {
        tmp = (tv.tv_usec * freq_conv) >> 16;
        timevalue += tmp;
    }

    return 0;
}


NTPTime::NTPTime(const TimeValue& t)
{
    const timeval *pTimeVal = t.getBasicTimeStruct();

    timevalue = origin_offset;
    // now convert to proper time origin
    timevalue += pTimeVal->tv_sec;

    timevalue <<= 32;


    // now convert the frequency of the fractional part
    // from usec to 2^32 ticks/sec.
    uint64 tmp = (pTimeVal->tv_usec * freq_conv) >> 16;


    timevalue += tmp;
}


int DayIndexFromDate(int year, int month, int day)
{
    // return index (0=Sunday, 1=Monday... etc

    int days_so_far[] = {   0  , 31 , 59 , 90 , 120, 151,
                            181, 212, 243, 273, 304, 334
                        };
    if (month > 12 || month < 0 || year < 1 || day < 1 || day > 31)
        return 0; // bad value(s)

    int day_index = ((year - 1) * 365 +
                     (year - 1) / 4 +
                     days_so_far[month-1] +
                     day +
                     (((month > 2) && !(year % 4)) ? 1 : 0)
                    ) % 7;
    return (day_index == 0) ? 6 : (day_index - 1);
}

static const char* const months[] = { "Jan", "Feb", "Mar", "Apr", "May",
                                      "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
                                      ""
                                    };
static const char* const days[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

OSCL_EXPORT_REF void PV8601ToRFC822(PV8601timeStrBuf pv8601_buffer, CtimeStrBuf ctime_buffer)
{
    // Convert from PV8601 Format (20010805T185430.000Z)
    // to RFC822 Format (Sun Aug  5 18:54:30 2001)
    // No <time.h> functions will be used because this code must run on
    // all platforms.
    //---OSCL_HAS_ANSI_STRING_SUPPORT---//
    if ((PV8601TIME_BUFFER_SIZE - 1) != strlen(pv8601_buffer))
    {
        ctime_buffer[0] = 0;
        return;
    }

    char buf[5];

    int year, mon, day, hour, min, sec;

    strncpy(buf, pv8601_buffer, 4);
    buf[4] = 0;
    year = atoi(buf);

    buf[2] = 0;
    strncpy(buf, pv8601_buffer + 4, 2);
    mon = atoi(buf);
    if (mon <= 0 || mon > 13)
        mon = 13;
    strncpy(buf, pv8601_buffer + 6, 2);
    day = atoi(buf);
    strncpy(buf, pv8601_buffer + 9, 2);
    hour = atoi(buf);
    strncpy(buf, pv8601_buffer + 11, 2);
    min = atoi(buf);
    strncpy(buf, pv8601_buffer + 13, 2);
    sec = atoi(buf);

    sprintf(ctime_buffer,
            "%s %s %2d %02d:%02d:%02d %04d",
            days[DayIndexFromDate(year,mon,day)],
            months[mon-1],
            day, hour, min, sec, year);

}

OSCL_EXPORT_REF void RFC822ToPV8601(CtimeStrBuf ctime_buffer, PV8601timeStrBuf pv8601_buffer)
{
    // Convert from RFC822 Format (Sun Aug  5 18:54:30 2001)
    // to PV8601 Format (20010805T185430.000Z)
    // No <time.h> functions will be used because this code must run on
    // all platforms.

    //---OSCL_HAS_ANSI_STRING_SUPPORT--- or ..//
    const char *ptr = ctime_buffer;

    int mon = 0, day, hour, min, sec;

    for (int i = 0; *months[i]; i++)
    {
        if (!strncmp(ptr + 4, months[i], 3))
        {
            mon = i + 1;
            break;
        }
    }

    day = atoi(*(ptr + 8) == ' ' ? (ptr + 9) : (ptr + 8));
    hour = atoi(ptr + 11);
    min = atoi(ptr + 14);
    sec = atoi(ptr + 17);

    sprintf(pv8601_buffer,
            "%04d%02d%02dT%02d%02d%02d.000Z",
            atoi(ptr + 20), // year
            mon, day, hour, min, sec);

}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//! get_rfc822_gmtime_str not supported on all platforms
/*!
   Notes: Symbian implementation feasible but not done.
*/

OSCL_EXPORT_REF char *TimeValue::get_rfc822_gmtime_str(int max_time_strlen,
        char *time_str)
{
    //---OSCL_HAS_UNIX_TIME_FUNCS---//
    struct tm *timeptr;
    struct tm buffer;
    timeptr = gmtime_r(&ts.tv_sec, &buffer);

    int length = snprintf(time_str, max_time_strlen,
                          "%s, %02d %s %04d %02d:%02d:%02d GMT",
                          days[timeptr->tm_wday], timeptr->tm_mday,
                          months[timeptr->tm_mon], timeptr->tm_year + 1900,
                          timeptr->tm_hour, timeptr->tm_min, timeptr->tm_sec);
    if (length < 0 || length > max_time_strlen)
    {
        time_str[0] = NULL_TERM_CHAR;
    }
    return time_str;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//! get_str_ctime not supported on all platforms
/*!
   Notes: Symbian implementation of obsolete function get_str_time
   modified to fit format returned by ctime
*/
OSCL_EXPORT_REF char *TimeValue::get_str_ctime(CtimeStrBuf ctime_buffer)
{
    //---OSCL_HAS_UNIX_TIME_FUNCS---//
    // Uses timeval basic time structure
    char *ptr = ctime_r(&ts.tv_sec, ctime_buffer);

    char *ptr_end = strchr(ptr, '\n');
    if (ptr_end) *ptr_end = '\0';
    return ptr;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
OSCL_EXPORT_REF int TimeValue::get_pv8601_str_time(PV8601timeStrBuf time_strbuf)
{
    //---OSCL_HAS_UNIX_TIME_FUNCS---//
    int num_chars = 0;
    struct tm *timeptr;
    struct tm buffer;

    timeptr = gmtime_r(&ts.tv_sec, &buffer);

    if ((num_chars = strftime(time_strbuf, sizeof(PV8601timeStrBuf),
                              "%Y%m%dT%H%M%S", timeptr)) == 0)
        time_strbuf[0] = NULL_TERM_CHAR;
    else
        num_chars += snprintf(time_strbuf + strlen(time_strbuf),
                              sizeof(PV8601timeStrBuf) - num_chars,
                              ".%03dZ", (int)(ts.tv_usec / 1000));

    return num_chars;

}



