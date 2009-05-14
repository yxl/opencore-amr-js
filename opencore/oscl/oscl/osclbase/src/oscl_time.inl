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

//               O S C L _ T I M E   ( T I M E   F U N C T I O N S )

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

#include "oscl_int64_utils.h"

//------ NTPTime ------------
OSCL_COND_EXPORT_REF OSCL_INLINE NTPTime::NTPTime()
{
    set_to_current_time();
}

OSCL_COND_EXPORT_REF OSCL_INLINE NTPTime::NTPTime(const NTPTime& src)
{
    timevalue = src.timevalue;
}

OSCL_COND_EXPORT_REF OSCL_INLINE NTPTime::NTPTime(const uint32 seconds)
{
    Oscl_Int64_Utils::set_uint64(timevalue, seconds, 0);
}

OSCL_COND_EXPORT_REF OSCL_INLINE NTPTime::NTPTime(const int32 seconds)
{
    if (seconds < 0) return;
    timevalue = uint64(seconds) << 32;
}

OSCL_COND_EXPORT_REF OSCL_INLINE NTPTime::NTPTime(const uint64 value)
{
    timevalue = value;
}

OSCL_COND_EXPORT_REF OSCL_INLINE NTPTime& NTPTime::operator=(uint32 newval)
{
    Oscl_Int64_Utils::set_uint64(timevalue, newval, 0);
    return *this;
}

OSCL_COND_EXPORT_REF OSCL_INLINE NTPTime& NTPTime::operator=(uint64 newval)
{
    timevalue = newval;
    return *this;
}

OSCL_COND_EXPORT_REF OSCL_INLINE NTPTime& NTPTime::operator+=(uint64 val)
{
    timevalue += val;
    return *this;
}

OSCL_COND_EXPORT_REF OSCL_INLINE NTPTime NTPTime::operator-(const NTPTime &ntpt) const
{
    NTPTime temp;
    temp.timevalue = timevalue - ntpt.timevalue;
    return temp;
}

OSCL_COND_EXPORT_REF OSCL_INLINE void NTPTime::set_from_system_time(const uint32 systemtime)
{
    timevalue = int64(systemtime + 2208988800U) << 32;
}

OSCL_COND_EXPORT_REF OSCL_INLINE uint32 NTPTime::get_middle32() const
{
    return Oscl_Int64_Utils::get_uint64_middle32(timevalue);
}

OSCL_COND_EXPORT_REF OSCL_INLINE uint32 NTPTime::get_upper32() const
{
    return Oscl_Int64_Utils::get_uint64_upper32(timevalue);
}

OSCL_COND_EXPORT_REF OSCL_INLINE uint32 NTPTime::get_lower32() const
{
    return Oscl_Int64_Utils::get_uint64_lower32(timevalue);
}

OSCL_COND_EXPORT_REF OSCL_INLINE int32 NTPTime::to_system_time() const
{
    return (uint32)((timevalue >> 32) - 2208988800U);
}

OSCL_COND_EXPORT_REF OSCL_INLINE uint64 NTPTime::get_value() const
{
    return timevalue;
}

//------ TimeValue ----------
OSCL_COND_EXPORT_REF OSCL_INLINE bool TimeValue::is_zero()
{
    return ((ts.tv_usec == 0) && (ts.tv_sec == 0));
}

OSCL_COND_EXPORT_REF OSCL_INLINE uint32 TimeValue::get_sec() const
{
    return ts.tv_sec;
};


OSCL_COND_EXPORT_REF OSCL_INLINE int32 TimeValue::to_msec() const
{
    return ((ts.tv_usec / 1000) + (ts.tv_sec * 1000));
}

OSCL_COND_EXPORT_REF OSCL_INLINE uint32 TimeValue::get_usec() const
{
    return ts.tv_usec;
}

OSCL_COND_EXPORT_REF OSCL_INLINE TimeValue operator -(const TimeValue& a, const TimeValue& b)
{
    TimeValue c;
    c = a;
    c -= b;
    return c;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

OSCL_COND_EXPORT_REF OSCL_INLINE TimeValue& TimeValue::operator =(const TimeValue & a)
{
    this->ts.tv_usec = a.ts.tv_usec;
    this->ts.tv_sec = a.ts.tv_sec;
    return *this;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

OSCL_COND_EXPORT_REF OSCL_INLINE TimeValue& TimeValue::operator +=(const TimeValue & a)
{
    this->ts.tv_usec += a.ts.tv_usec;
    this->ts.tv_sec += a.ts.tv_sec;

    if (this->ts.tv_usec >= USEC_PER_SEC)
    {
        this->ts.tv_usec -= USEC_PER_SEC;
        ++this->ts.tv_sec;
    }
    return *this;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


OSCL_COND_EXPORT_REF OSCL_INLINE TimeValue& TimeValue::operator -=(const TimeValue & a)
{
    this->ts.tv_usec -= a.ts.tv_usec;
    this->ts.tv_sec -= a.ts.tv_sec;

    if ((this->ts.tv_sec > 0) && (this->ts.tv_usec < 0))
    {
        this->ts.tv_usec += USEC_PER_SEC;
        --this->ts.tv_sec;
    }
    return *this;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


OSCL_COND_EXPORT_REF OSCL_INLINE TimeValue& TimeValue::operator *=(const int scale)
{
    this->ts.tv_usec *= scale;
    this->ts.tv_sec *= scale;
    unsigned long secs = this->ts.tv_usec / USEC_PER_SEC;
    this->ts.tv_sec += secs;
    this->ts.tv_usec -= secs * USEC_PER_SEC;
    return *this;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


OSCL_COND_EXPORT_REF OSCL_INLINE bool operator ==(const TimeValue& a, const TimeValue& b)
{
    return timercmp(&a.ts, &b.ts, ==);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

OSCL_COND_EXPORT_REF OSCL_INLINE bool operator !=(const TimeValue& a, const TimeValue& b)
{
    return timercmp(&a.ts, &b.ts, !=);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

OSCL_COND_EXPORT_REF OSCL_INLINE bool operator <=(const TimeValue& a, const TimeValue& b)
{

    return !timercmp(&a.ts, &b.ts, >);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

OSCL_COND_EXPORT_REF OSCL_INLINE bool operator >=(const TimeValue& a, const TimeValue& b)
{
    return !timercmp(&a.ts, &b.ts, <);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


OSCL_COND_EXPORT_REF OSCL_INLINE bool operator >(const TimeValue& a, const TimeValue& b)
{
    return timercmp(&a.ts, &b.ts, >);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

OSCL_COND_EXPORT_REF OSCL_INLINE bool operator <(const TimeValue& a, const TimeValue& b)
{

    return timercmp(&a.ts, &b.ts, <);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
OSCL_COND_EXPORT_REF OSCL_INLINE void TimeValue::set_to_current_time()
{
    gettimeofday(&ts, NULL);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#include "oscl_mem_basic_functions.h"

OSCL_COND_EXPORT_REF OSCL_INLINE void TimeValue::set_to_zero()
{
    oscl_memset(&ts, 0, sizeof(ts));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

OSCL_COND_EXPORT_REF OSCL_INLINE TimeValue::TimeValue()
{
    gettimeofday(&ts, NULL);
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

OSCL_COND_EXPORT_REF OSCL_INLINE TimeValue::TimeValue(const TimeValue& Tv)
{
    ts.tv_usec = Tv.ts.tv_usec;
    ts.tv_sec  = Tv.ts.tv_sec;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
OSCL_COND_EXPORT_REF OSCL_INLINE TimeValue::TimeValue(const OsclBasicTimeStruct& in_tv)
{
    ts.tv_usec = in_tv.tv_usec;
    ts.tv_sec = in_tv.tv_sec;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
OSCL_COND_EXPORT_REF OSCL_INLINE TimeValue::TimeValue(long num_units, TimeUnits units)
{
    ts.tv_sec = num_units / MapToSeconds[units];
    long diff = num_units - MapToSeconds[units] * ts.tv_sec;
    ts.tv_usec = diff * MapToUSeconds[units];
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
OSCL_COND_EXPORT_REF OSCL_INLINE void TimeValue::set_from_ntp_time(const uint32 ntp_offset_seconds)
{
    ts.tv_sec = ntp_offset_seconds - unix_ntp_offset;
    ts.tv_usec = 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
OSCL_COND_EXPORT_REF OSCL_INLINE TimeValue::TimeValue(OsclBasicDateTimeStruct ints)
{
    ts.tv_sec = mktime(&ints);
    ts.tv_usec = 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

OSCL_COND_EXPORT_REF OSCL_INLINE int32 TimeValue::get_local_time()
{
    // Daylight saving time offset of 1 hour in the summer
    uint dst_offset = 60 * 60;
    int32 GMTime = ts.tv_sec;

    struct tm *timeptr;
    struct tm buffer;
    int dstFlag;
    timeptr = localtime_r(&ts.tv_sec, &buffer);

    GMTime -= timeptr->tm_gmtoff;

    dstFlag = timeptr->tm_isdst;

    if (dstFlag == 1)
    {
        // Daylight saving time. Add an hour's offset to get GMT
        GMTime += dst_offset;
    }

    return GMTime;
}

OSCL_COND_EXPORT_REF OSCL_INLINE OsclBasicTimeStruct * TimeValue::get_timeval_ptr()
{
    return &ts;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

