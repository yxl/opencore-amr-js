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
#include "media_clock_converter.h"

// Use default DLL entry point for Symbian
#include "oscl_dll.h"
OSCL_DLL_ENTRY_POINT_DEFAULT()

// All the rounding in this utility is using method rounding up
// For example, normal calculation
//   A / B
// is replaced as
//   (A + (B - 1))/ B
// In an average roudning up is same as rounding down, but it resolve some issue

//////////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void MediaClockConverter::set_value(const MediaClockConverter& src)
{
    // Timescale value cannot be zero
    OSCL_ASSERT(src.get_timescale() != 0);
    if (src.get_timescale() == 0)
    {
        OSCL_LEAVE(OsclErrCorrupt);
    }

    OSCL_ASSERT(timescale != 0);
    if (0 == timescale)
    {
        OSCL_LEAVE(OsclErrCorrupt);
    }

    uint64 value = (uint64(src.get_wrap_count())) << 32;

    value += src.get_current_timestamp();

    // rounding up
    value = (uint64(value) * timescale + uint64(src.get_timescale() - 1)) / src.get_timescale();

    wrap_count = ((uint32)(value >> 32)) % timescale;

    current_ts = (uint32)(value & 0xFFFFFFFF);
}

//////////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void MediaClockConverter::set_timescale(uint32 new_timescale)
{
    // Timescale value cannot be zero
    OSCL_ASSERT(new_timescale != 0);
    if (0 == new_timescale)
    {
        OSCL_LEAVE(OsclErrArgument);
    }

    OSCL_ASSERT(timescale != 0);
    if (0 == timescale)
    {
        OSCL_LEAVE(OsclErrCorrupt);
    }

    uint64 value = ((uint64)wrap_count) << 32;
    value += current_ts;

    // rounding up
    value = (value * new_timescale + uint64(timescale - 1)) / timescale;

    timescale = new_timescale;

    wrap_count = ((uint32)(value >> 32)) % timescale;

    current_ts = (uint32)(value & 0xFFFFFFFF);
}

//////////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void MediaClockConverter::set_clock_other_timescale(uint32 value,
        uint32 in_timescale)
{
    // Timescale value cannot be zero
    OSCL_ASSERT(in_timescale != 0);
    if (0 == in_timescale)
    {
        OSCL_LEAVE(OsclErrArgument);
    }

    OSCL_ASSERT(timescale != 0);
    if (0 == timescale)
    {
        OSCL_LEAVE(OsclErrCorrupt);
    }

    uint64 new_value = (uint64)value * timescale;
    uint64 in_timescale64Comp = (uint64)(in_timescale - 1);

    // rounding up
    new_value = new_value + in_timescale64Comp ;
    new_value /= in_timescale;

    wrap_count = ((uint32)(new_value >> 32)) % timescale;

    current_ts = (uint32)(new_value & 0xFFFFFFFF);
}

//////////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF uint32 MediaClockConverter::get_timediff_and_update_clock(uint32 value,
        uint32 in_timescale,
        uint32 output_timescale)
{
    // Timescale value cannot be zero
    OSCL_ASSERT(in_timescale != 0);
    if (0 == in_timescale)
    {
        OSCL_LEAVE(OsclErrArgument);
    }

    // convert to native timescale
    // rounding up
    uint64 new_value = (uint64)value * timescale + uint64(in_timescale - 1);
    new_value /= in_timescale;

    uint32 new_timevalue = ((uint32)(new_value & 0xFFFFFFFF));

    return get_timediff_and_update_clock(new_timevalue, output_timescale);

}

//////////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF uint32 MediaClockConverter::get_timediff_and_update_clock(uint32 value,
        uint32 output_timescale)
{
    // Timescale value cannot be zero
    OSCL_ASSERT(timescale != 0);
    if (0 == timescale)
    {
        OSCL_LEAVE(OsclErrCorrupt);
    }

    uint32 diff = value - current_ts;

    // convert to output timescale
    // rounding up
    uint64 new_value = (uint64)diff * output_timescale + uint64(timescale - 1);
    new_value /= timescale;

    diff = ((uint32)(new_value & 0xFFFFFFFF));

    if (update_clock(value))
    {
        return diff;
    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF bool MediaClockConverter::update_clock(uint32 new_ts)
{
    uint32 diff = new_ts - current_ts;
    if (new_ts < current_ts)
    {
        if (diff < WRAP_THRESHOLD)
        {
            if (++wrap_count >= timescale)
            {
                wrap_count = 0;
            }
            current_ts = new_ts;
            return true;
        }
        // otherwise this an earlier value so ignore it.
    }
    else
    {
        if (diff < MISORDER_THRESHOLD)
        {
            current_ts = new_ts;
            return true;
        }
        // otherwise this an earlier value so ignore it.
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF uint32 MediaClockConverter::get_converted_ts(uint32 new_timescale) const
{
    // Timescale value cannot be zero
    OSCL_ASSERT(timescale != 0);
    if (0 == timescale)
    {
        OSCL_LEAVE(OsclErrCorrupt);
    }

    uint64 value = ((uint64)wrap_count) << 32;
    // rounding up
    value = ((value + uint64(current_ts)) * uint64(new_timescale) + uint64(timescale - 1)) / uint64(timescale);

    return ((uint32) value);

}
