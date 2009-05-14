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
// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

//               R T S P _ T I M E _ F O R M A T S
//     ( T I M E / R A N G E   R E P R E S E N T A T I O N S )

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
#ifndef RTSP_TIME_FORMAT_H
#define RTSP_TIME_FORMAT_H

// - - Inclusion - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef OSCL_INT64_UTILS_H_INCLUDED
#include "oscl_int64_utils.h"
#endif
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#ifndef OSCL_MEM_BASIC_FUNCTIONS_H
#include "oscl_mem_basic_functions.h"
#endif



// SMPTE time format allows two decimal digits for each field of the format
// The basic format is  HH:MM:SS where HH = hours, MM = minutes, SS = seconds
// from the beginning of the presentation.  It can optionally contain frame-level
// with the format HH:MM:SS:FF.ff where FF is the frame number and ff is the
// fractional part of the frame (i.e., subframe).

struct SmpteTimeFormat
{
    uint8 hours;
    uint8 minutes;
    uint8 seconds;
    uint8 frames;
    uint8 subframes;
};

// NPT (normal play time) format is represents the time offset from the beginning
// of the presentation either in seconds (including fractional seconds) or
// hours+minutes+secs (including fractional seconds).  Unlike the SMPTE time format,
// the number of digits in the hours representation is arbitrary.  When in the seconds-only
// format, the number of digits for that field is also arbitrary.  We will allow
// 32 bits to hold these arbitrary length fields.  The format is one of
//                "now" | sec+[.frac_sec*] | H+:MM:SS[.frac_sec*]
// where "+" means one or more digits, "*" means zero or more digits, and "[ ]"  means the
// field is optional and may not appear.

struct NptSecFormat
{
    uint32 sec;
    uint32 milli_sec;	//added as a sub for floating counterpart.
//	float frac_sec;
};

struct NptHHMMSSFormat
{
    uint32 hours;
    uint8 min;
    uint8 sec;
    float frac_sec;
};

struct NptTimeFormat
{
    enum NptFormatType { NOW, NPT_SEC, NPT_HHMMSS};

    NptFormatType npt_format;
    union
    {
        NptSecFormat npt_sec;
        NptHHMMSSFormat npt_hhmmss;
    };

};

struct PVNSPlaylistTimeFormat
{
//	mbchar iPlaylistUrl[1024];
    uint32 iClipIndex;
    uint32 sec;
    uint32 milli_sec;
};

// Abs time format represents the absolute time value.  The this represented as
// YYYYMMDD"T"HHMMSS[.frac_sec*]"Z" where YYYY is the four digit year, MM is the two digit
// month, DD is the two-digit day, "T" is just the T character, HH is the two-digit hour,
// MM is the two-digit minute, SS is the two-digit second, and finally "Z" is simply the Z character.
// The time value is based on the UTC (GMT) time coordinates.

struct AbsTimeFormat
{
    uint16 year;
    uint8 month;
    uint8 day;
    uint8 hours;
    uint8 min;
    uint8 sec;
    float frac_sec;
};

struct RtspRangeType
{
    enum
    {
        PlaylistUrlLen = 1024
    };

    RtspRangeType(): format(UNKNOWN_RANGE), start_is_set(false), end_is_set(false)
    {
        oscl_memset(iPlaylistUrl, 0, (PlaylistUrlLen * sizeof(mbchar)));
    }
    enum RtspRangeFormat { NPT_RANGE, SMPTE_RANGE, SMPTE_25_RANGE,
                           SMPTE_30_RANGE,
                           ABS_RANGE,
//#ifdef RTSP_PLAYLIST_SUPPORT
                           PLAYLIST_TIME_RANGE,
//#endif //#ifdef RTSP_PLAYLIST_SUPPORT
                           UNKNOWN_RANGE, INVALID_RANGE
                         };
    RtspRangeFormat format;
    bool start_is_set;
    union
    {
        SmpteTimeFormat smpte_start;
        NptTimeFormat npt_start;
        AbsTimeFormat abs_start;
        PVNSPlaylistTimeFormat	playlist_start;
    };

    bool end_is_set;
    union
    {
        SmpteTimeFormat smpte_end;
        NptTimeFormat npt_end;
        AbsTimeFormat abs_end;
        PVNSPlaylistTimeFormat	playlist_end;
    };

//#ifdef RTSP_PLAYLIST_SUPPORT
    mbchar iPlaylistUrl[PlaylistUrlLen];
//#endif //#ifdef RTSP_PLAYLIST_SUPPORT
    /***********************************************************************************/
    /*Function : int32 convertToMillisec(int32 &startTime, int32 &stopTime)                  */
    /*                                                                                 */
    /*                                                                                 */
    /*Parameters : int32 &startTime - Output                                             */
    /*             int32 &stopTime - Output                                              */
    /*                                                                                 */
    /*Return values : 0 - if conversion was successful.                                */
    /*                -1 - if conversion failed.                                       */
    /*                                                                                 */
    /*Description : This utility function converts time in different formats to time   */
    /*in milliseconds. In case of formats that don't fit in 32 bits after conversion,  */
    /*the function returns an error code. The caller of the function can use the return*/
    /*code to decide whether to call the "calculateDelta" utility function.            */
    /***********************************************************************************/
    int32 convertToMilliSec(int32 &startTime, int32 &stopTime)
    {
        const int32 MILLISEC_IN_HOUR = 3600000;
        const int32 MILLISEC_IN_MIN = 60000;
        const int32 MILLISEC_IN_SEC = 1000;
        switch (format)
        {
            case RtspRangeType::NPT_RANGE:
            {
                if (start_is_set)
                {
                    switch (npt_start.npt_format)
                    {
                        case NptTimeFormat::NOW:
                        {
                            startTime = 0;
                        }
                        break;
                        case NptTimeFormat::NPT_SEC:
                        {
                            startTime = (int32)(MILLISEC_IN_SEC * (npt_start.npt_sec.sec) +
                                                npt_start.npt_sec.milli_sec);
                        }
                        break;
                        case NptTimeFormat::NPT_HHMMSS:
                        {
                            startTime = MILLISEC_IN_HOUR * npt_start.npt_hhmmss.hours +
                                        MILLISEC_IN_MIN * npt_start.npt_hhmmss.min +
                                        MILLISEC_IN_SEC * npt_start.npt_hhmmss.sec +
                                        (int32)(MILLISEC_IN_SEC * npt_start.npt_hhmmss.frac_sec);
                        }
                        break;
                    }
                }
                else
                {
                    startTime = 0;
                }
                if (end_is_set)
                {
                    switch (npt_end.npt_format)
                    {
                        case NptTimeFormat::NOW:
                        {
                            stopTime = 0;
                        }
                        break;
                        case NptTimeFormat::NPT_SEC:
                        {
                            stopTime = (int32)(MILLISEC_IN_SEC * (npt_end.npt_sec.sec) +
                                               npt_end.npt_sec.milli_sec);
                        }
                        break;
                        case NptTimeFormat::NPT_HHMMSS:
                        {
                            stopTime = MILLISEC_IN_HOUR * npt_end.npt_hhmmss.hours +
                                       MILLISEC_IN_MIN * npt_end.npt_hhmmss.min +
                                       MILLISEC_IN_SEC * npt_end.npt_hhmmss.sec +
                                       (int32)(MILLISEC_IN_SEC * npt_end.npt_hhmmss.frac_sec);
                        }
                        break;
                    }
                }
                else
                {
                    stopTime = 0;
                }
            }
            break;
            case RtspRangeType::SMPTE_RANGE:
            case RtspRangeType::SMPTE_25_RANGE:
            case RtspRangeType::SMPTE_30_RANGE:
            {
                if (start_is_set)
                {
                    startTime = MILLISEC_IN_HOUR * smpte_start.hours +
                                MILLISEC_IN_MIN * smpte_start.minutes +
                                MILLISEC_IN_SEC * smpte_start.seconds;
                }
                else
                {
                    startTime = 0;
                }
                if (end_is_set)
                {
                    stopTime = MILLISEC_IN_HOUR * smpte_end.hours +
                               MILLISEC_IN_MIN * smpte_end.minutes +
                               MILLISEC_IN_SEC * smpte_end.seconds;
                }
                else
                {
                    stopTime = 0;
                }
            }
            break;
            case RtspRangeType::ABS_RANGE:
            {
                if (start_is_set && end_is_set)
                {
                    startTime = 0;
                    uint64 delta = calculateDelta();
                    /*
                     *We return valid start and stop times(in millisec) if we see that the delta
                     *between the start and stop times fits in 32 bits.
                     */
                    if (delta < (uint64)0xffffffff)
                    {
                        startTime = 0;

                        stopTime = Oscl_Int64_Utils::get_uint64_lower32(delta);

                    }
                    else
                    {
                        startTime = 0;
                        stopTime = 0;
                        return -1;
                    }
                }
                else
                {
                    return -1;
                }
            }
            break;
            case RtspRangeType::UNKNOWN_RANGE:
            case RtspRangeType::INVALID_RANGE:
            {
                startTime = 0;
                stopTime = 0;
                return -1;
            }

            default:
            {
                startTime = 0;
                stopTime = 0;
                return -1;
            }
        }
        return 0;
    }

    uint64 calculateDelta()
    {
        const int32 DAYS_IN_YEAR = 365;
        const int32 DAYS_IN_LEAP_YEAR = 366;
        const int32 HOURS_IN_DAY = 24;
        const int32 MILLISEC_IN_HOUR = 3600000;
        const int32 MILLISEC_IN_MIN = 60000;
        const int32 MILLISEC_IN_SEC = 1000;

        int32 daysOfYear[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
        int32 daysOfLeapYear[] = {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335};
        uint64 delta = 0;
        uint64 startTime = 0;
        uint64 stopTime = 0;

        /*
         *Let's calculate the start time first. We need to find the number of days in the
         *year first.
         */
        if (start_is_set && end_is_set)
        {
            int32 numberOfDays = 0;
            bool leap = leapYear(abs_start.year);
            if (!leap)
            {
                numberOfDays = daysOfYear[abs_start.month-1];
            }
            else
            {
                numberOfDays = daysOfLeapYear[abs_start.month-1];
            }
            numberOfDays += abs_start.day - 1;
            startTime = numberOfDays * HOURS_IN_DAY;
            startTime += (uint32)abs_start.hours;
            startTime *= MILLISEC_IN_HOUR;
            startTime += abs_start.min * MILLISEC_IN_MIN +
                         abs_start.sec * MILLISEC_IN_SEC +
                         (int32)(abs_start.frac_sec * MILLISEC_IN_SEC);
            /*
             *Now, let's calculate the stop time. We use the start time's year as the reference
             *year.
             */
            numberOfDays = 0;
            for (int32 ii = abs_start.year; ii < abs_end.year; ii++)
            {
                if (leapYear(ii))
                    numberOfDays += DAYS_IN_LEAP_YEAR;
                else
                    numberOfDays += DAYS_IN_YEAR;
            }
            leap = leapYear(abs_end.year);
            if (!leap)
            {
                numberOfDays = daysOfYear[abs_end.month-1];
            }
            else
            {
                numberOfDays = daysOfLeapYear[abs_end.month-1];
            }

            numberOfDays += abs_end.day - 1;
            stopTime = numberOfDays * HOURS_IN_DAY;
            stopTime += (uint32)abs_end.hours;
            stopTime *= MILLISEC_IN_HOUR;
            stopTime += abs_end.min * MILLISEC_IN_MIN +
                        abs_end.sec * MILLISEC_IN_SEC +
                        (int32)(abs_end.frac_sec * MILLISEC_IN_SEC);

            delta = stopTime - startTime;
        }
        else
        {
            return delta;
        }
        return delta;
    }

    bool leapYear(int32 year)
    {
        if (year % 4 != 0)
            return false;
        else if (year % 400 == 0)
            return true;
        else if (year % 100 == 0)
            return false;
        else
            return true;
    }
};
#endif
