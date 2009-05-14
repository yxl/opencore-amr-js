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
/*********************************************************************************/
/*     -------------------------------------------------------------------       */
/*                         MPEG-4 MovieHeaderAtom Class                          */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This MovieHeaderAtom Class defines the overall media-independent information
    relevant to the MPEG-4 presentation as a whole.
*/


#define IMPLEMENT_MovieHeaderAtom

#ifndef OSCL_SNPRINTF_H_INCLUDED
#include "oscl_snprintf.h"
#endif
#ifndef OSCL_UTF8CONV_H
#include "oscl_utf8conv.h"
#endif
#include "movieheaderatom.h"
#include "atomutils.h"
#include "atomdefs.h"

#define ADJUST_DAY_COUNT_USER_PERSPECTIVE_FOR_FIRST_DAY_OF_YEAR 1
// Stream-in Constructor
MovieHeaderAtom::MovieHeaderAtom(MP4_FF_FILE *fp, uint32 size, uint32 type)
        : FullAtom(fp, size, type)
{
    if (_success)
    {

        _pparent = NULL;

        if (getVersion() == 1)
        {
            if (!AtomUtils::read64(fp, _creationTime64))
            {
                _success = false;
            }
            if (!AtomUtils::read64(fp, _modificationTime64))
            {
                _success = false;
            }
            if (!AtomUtils::read32(fp, _timeScale))
            {
                _success = false;
            }
            if (!AtomUtils::read64(fp, _duration64))
            {
                _success = false;
            }
        }
        else
        {
            if (!AtomUtils::read32(fp, _creationTime))
            {
                _success = false;
            }
            if (!AtomUtils::read32(fp, _modificationTime))
            {
                _success = false;
            }
            if (!AtomUtils::read32(fp, _timeScale))
            {
                _success = false;
            }
            if (!AtomUtils::read32(fp, _duration))
            {
                _success = false;
            }

        }

        // Read all defaulted reserved members - don't care about values
        uint32 junk;
        for (int32 i = 0; i < 19; i++)
        {
            if (!AtomUtils::read32(fp, junk))
            {
                _success = false;
            }
        }

        if (!AtomUtils::read32(fp, _nextTrackID))
            _success = false;

        if (!_success)
            _mp4ErrorCode = READ_MOVIE_HEADER_ATOM_FAILED;
    }
    else
    {
        if (_mp4ErrorCode != ATOM_VERSION_NOT_SUPPORTED)
            _mp4ErrorCode = READ_MOVIE_HEADER_ATOM_FAILED;
    }

}

// Destructor
MovieHeaderAtom::~MovieHeaderAtom()
{
    // Empty
}

OSCL_wHeapString<OsclMemAllocator> MovieHeaderAtom::convertTimeToDate(uint32 time)
{
    OSCL_HeapString<OsclMemAllocator> date;

    char buf[64];

    int32 numDaysInMonth[12] =
        {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    int32 numDaysInMonthLeap[12] =
        {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    int32 refYear = 1904;

    // (365*4 + 1) * 24 * 3600
    int32 numSecsInABlkofFourYears = 126230400;

    // 365*24*3600
    int32 numSecInYear = 31536000;

    int32 numBlksOfFour = (time / numSecsInABlkofFourYears);

    int32 leftOverSecs = (time - (numBlksOfFour * numSecsInABlkofFourYears));

    int32 leftOverYears = 0;
    int32 year    = 0;
    int32 numDays = 0;
    int32 numHrs  = 0;
    int32 numMins = 0;
    int32 month   = 0;
    int32 milliSecs = 0;

    if (leftOverSecs > numSecInYear)
    {
        /*To take in to consideration at least 1 year among the number of years
         as leap among leftOverYear*/

        leftOverSecs -= (24 * 3600);

        leftOverYears = (leftOverSecs / numSecInYear);

        leftOverSecs -= (leftOverYears * numSecInYear);


        numDays = (uint16)(leftOverSecs / (24 * 3600));

        leftOverSecs -= (numDays * 24 * 3600);


        for (uint32 i = 0; i < 12; i++)
        {
            if (numDays >= numDaysInMonth[i])
            {
                numDays = (numDays - numDaysInMonth[i]);
            }
            else
            {
                month = (i + 1);
                break;
            }
        }


    }
    else
    {
        numDays = (leftOverSecs / (24 * 3600));

        leftOverSecs -= (numDays * 24 * 3600);

        for (int32 i = 0; i < 12; i++)
        {
            if (numDays >= numDaysInMonthLeap[i])
            {
                numDays = (numDays - numDaysInMonthLeap[i]);
            }
            else
            {
                month = (i + 1);
                break;
            }
        }

    }

    // Considering a situation where by the date is 1st Jan of any year
    // the leftOverSecs would be less than the sec available in a day resulting numDays
    // as 0, however that is 1st Jan from user perspective. So a day adjustment factor
    // is added into numDays.


    numDays += ADJUST_DAY_COUNT_USER_PERSPECTIVE_FOR_FIRST_DAY_OF_YEAR;



    numHrs = (leftOverSecs / 3600);

    leftOverSecs -= (numHrs * 3600);

    numMins = (leftOverSecs / 60);

    leftOverSecs -= (numMins * 60);

    year = (refYear + (numBlksOfFour * 4) + leftOverYears);

    oscl_snprintf(buf,
                  256,
                  "%04d%02d%02dT%02d%02d%02d.%03dZ",
                  year, month, numDays, numHrs,
                  numMins, leftOverSecs, milliSecs);

    date += buf;

    /* convert to unicode */
    uint32 wDateBufLen = 64 * sizeof(oscl_wchar);
    oscl_wchar wDate[64*sizeof(oscl_wchar)];
    oscl_memset((OsclAny*)wDate, 0, wDateBufLen);
    oscl_UTF8ToUnicode(date.get_cstr(),
                       (int32)(date.get_size()),
                       wDate,
                       (int32)(wDateBufLen));

    OSCL_wHeapString<OsclMemAllocator> wDateStr;
    wDateStr += wDate;

    return (wDateStr);
}
