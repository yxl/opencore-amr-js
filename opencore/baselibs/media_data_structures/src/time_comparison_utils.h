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

/**
 *  @file time_comparison_utils.h
 *  @brief Provides time comparison utilities
 */

#ifndef TIME_COMPARISON_UTILS_H_INCLUDED
#define TIME_COMPARISON_UTILS_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef PVMF_TIMESTAMP_H_INCLUDED
#include "pvmf_timestamp.h"
#endif


class PVTimeComparisonUtils
{
    public:
        typedef enum
        {
            MEDIA_EARLY_OUTSIDE_WINDOW,
            MEDIA_EARLY_WITHIN_WINDOW,
            MEDIA_ONTIME_WITHIN_WINDOW,
            MEDIA_LATE_WITHIN_WINDOW,
            MEDIA_LATE_OUTSIDE_WINDOW
        } MediaTimeStatus;


        /*!*********************************************************************
        **
        ** Function:    IsEarlier
        **
        ** Synopsis: This function returns true if TimeA is considered EARLIER than TimeB.
        **
        ** Arguments :
        ** @param  [aTimeA] -- Time stamp to be compared.
        ** @param  [aTimeB] -- Time stamp to be compared.
        ** @param  [delta]  -- delta is an output parameter and is always
        **                     a small positive value representing TimeB-TimeA if TimeA is earlier
        **                     than TimeB or TimeA-TimeB if TimeB is earlier than TimeA.

        ** Returns:
        ** @return  bool  -- returns true if TimeA is considered EARLIER than TimeB.
        **
        **
        ** Notes:
        **
        **********************************************************************/

        OSCL_IMPORT_REF static bool IsEarlier(
            /*IN*/ uint32 aTimeA,
            /*IN*/ uint32 aTimeB,
            /*OUT*/ uint32& delta);

        /*!*********************************************************************
        **
        ** Function:    CheckTimeWindow
        **
        ** Synopsis: This function would return one of MEDIA_EARLY_OUTSIDE_WINDOW, MEDIA_EARLY_WITHIN_WINDOW,
        **           MEDIA_LATE_WITHIN_WINDOW or MEDIA_LATE_OUTSIDE_WINDOW depending on where the TimeStamp
        **           is within the window specified by (Clock-LateMargin) to (Clock+EarlyMargin).
        **
        ** Arguments :
        ** @param  [aTimeStamp]    -- Time stamp to be checked.
        ** @param  [aClock]        -- present clock value.
        ** @param  [aEarlyMargin]  -- early margin of the window
        ** @param  [aLateMargin]   -- late margin of the window
        ** @param  [aDelta]        -- aDelta is an output parameter and would contain the difference
        **                            in timestamp and clock in positive form.

        ** Returns:
        ** @return  MediaTimeStatus  -- returns one of the values from MediaTimeStatus enum.
        **
        **
        ** Notes:
        **
        **********************************************************************/

        OSCL_IMPORT_REF static MediaTimeStatus CheckTimeWindow(
            /*IN*/ PVMFTimestamp aTimeStamp,
            /*IN*/ uint32 aClock,
            /*IN*/ uint32 aEarlyMargin,
            /*IN*/ uint32 aLateMargin,
            /*OUT*/ uint32 &aDelta);

};
#endif
