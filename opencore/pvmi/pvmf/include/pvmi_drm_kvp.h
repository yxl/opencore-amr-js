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

#ifndef PVMI_DRM_KVP_H_INCLUDED
#define PVMI_DRM_KVP_H_INCLUDED

#define PVMF_DRM_INFO_IS_PROTECTED_QUERY "drm/is-protected"
#define PVMF_DRM_INFO_IS_PROTECTED_VALUE "drm/is-protected;valtype=bool"

#define PVMF_DRM_INFO_IS_LICENSE_AVAILABLE_QUERY "drm/is-license-available"
#define PVMF_DRM_INFO_IS_LICENSE_AVAILABLE_VALUE "drm/is-license-available;valtype=bool"

#define PVMF_DRM_INFO_IS_FORWARD_LOCKED_QUERY "drm/is-forward-locked"
#define PVMF_DRM_INFO_IS_FORWARD_LOCKED_VALUE "drm/is-forward-locked;valtype=bool"

#define PVMF_DRM_INFO_CAN_USE_AS_RINGTONE_QUERY "drm/can-use-as-ringtone"
#define PVMF_DRM_INFO_CAN_USE_AS_RINGTONE_VALUE "drm/can-use-as-ringtone;valtype=bool"

/*
 * License types fall into following categories:
 *	- time based (has an start and end time)
 *  - duration based (a certain amount of time since first use)
 *  - count based
 *  - or a combination of both
 *  Value returned by this query would be a string that will take any of
 *  the following forms:
 * "unlimited", "time", "duration", "count", "time-count", "duration-count"
 */
#define PVMF_DRM_INFO_LICENSE_TYPE_QUERY "drm/license-type"
#define PVMF_DRM_INFO_LICENSE_TYPE_VALUE "drm/license-type;valtype=char*"

/*
 * Value returned is uint32
 */
#define PVMF_DRM_INFO_LICENSE_COUNT_QUERY "drm/num-counts"
#define PVMF_DRM_INFO_LICENSE_COUNT_VALUE "drm/num-counts;valtype=uint32"

/*
 * All start and end times are in ISO 8601 Timeformat
 * The format is as follows. Exactly the components shown here must be present,
 * with exactly this punctuation. Note that the "T" appears literally in the string.
 * to indicate the beginning of the time element, as specified in ISO 8601.
 *		Year:
 *			YYYY (eg 1997)
 *		Year and month:
 *			YYYY-MM (eg 1997-07)
 *		Complete date:
 *			YYYY-MM-DD (eg 1997-07-16)
 *		Complete date plus hours and minutes:
 *			YYYY-MM-DDThh:mmTZD (eg 1997-07-16T19:20+01:00)
 *		Complete date plus hours, minutes and seconds:
 *			YYYY-MM-DDThh:mm:ssTZD (eg 1997-07-16T19:20:30+01:00)
 *		Complete date plus hours, minutes, seconds and a decimal fraction of a
 *		second
 *			YYYY-MM-DDThh:mm:ss.sTZD (eg 1997-07-16T19:20:30.45+01:00)
 *	where:
 *  YYYY = four-digit year
 *  MM   = two-digit month (01=January, etc.)
 *  DD   = two-digit day of month (01 through 31)
 *  hh   = two digits of hour (00 through 23) (am/pm NOT allowed)
 *  mm   = two digits of minute (00 through 59)
 *  ss   = two digits of second (00 through 59)
 *  s    = one or more digits representing a decimal fraction of a second
 *  TZD  = time zone designator (Z or +hh:mm or -hh:mm)
 *
 * This profile defines two ways of handling time zone offsets:
 *		- Times are expressed in UTC (Coordinated Universal Time),
 *      with a special UTC designator ("Z").
 *
 *		- Times are expressed in local time, together with a time zone offset
 *      in hours and minutes. A time zone offset of "+hh:mm" indicates that
 *      the date/time uses a local time zone which is "hh" hours and "mm"
 *      minutes ahead of UTC. A time zone offset of "-hh:mm" indicates that
 *      the date/time uses a local time zone which is "hh" hours and "mm"
 *      minutes behind UTC.
 *
 * For example:
 * 1994-11-05T08:15:30-05:00 corresponds to November 5, 1994, 8:15:30 am,
 * US Eastern Standard Time.
 * 1994-11-05T13:15:30Z corresponds to the same instant.
 */

/*
 * StartTime in ISO 8601 format. If the start time is not set then the value
 * would be "now"
 */
#define PVMF_DRM_INFO_LICENSE_START_TIME_QUERY "drm/license-start"
#define PVMF_DRM_INFO_LICENSE_START_TIME_VALUE "drm/license-start;valtype=char*"

/*
 * Expiration Time in ISO 8601 format. If no end time then this query would not be
 * answered.
 */
#define PVMF_DRM_INFO_LICENSE_EXPIRATION_TIME_QUERY "drm/license-expiry"
#define PVMF_DRM_INFO_LICENSE_EXPIRATION_TIME_VALUE "drm/license-expiry;valtype=char*"

/*
 * StartTime in Microsoft's FILETIME format (number of 100s of nanoseconds since
 * Jan 1, 1601)
 */
#define PVMF_DRM_INFO_LICENSE_START_TIME_VALUE_FORMAT_MS_FILETIME "drm/license-start;valtype=pUnit64_value;format=ms-filetime"

/*
 * Expiration Time in Microsoft's FILETIME format (number of 100s of nanoseconds since
 * Jan 1, 1601)
 */
#define PVMF_DRM_INFO_LICENSE_EXPIRATION_TIME_VALUE_FORMAT_MS_FILETIME "drm/license-expiry;valtype=pUnit64_value;format=ms-filetime"

/*
 * License duration since first use. Duration is specified in number of seconds.
 */
#define PVMF_DRM_INFO_LICENSE_DURATION_SINCE_FIRST_USE_QUERY "drm/duration"
#define PVMF_DRM_INFO_LICENSE_DURATION_SINCE_FIRST_USE_VALUE "drm/duration;valtype=uint32"

/*
 * License issuer
 */
#define PVMF_DRM_INFO_LICENSE_ISSUER_QUERY "dla/license-issuer"
#define PVMF_DRM_INFO_LICENSE_ISSUER_VALUE "dla/license-issuer;valtype=char*"

/*
 * Redirect times
 */
#define PVMF_DRM_INFO_REDIRECT_TIMES_QUERY "dla/num-redirect"
#define PVMF_DRM_INFO_REDIRECT_TIMES_VALUE "dla/num-redirect;valtype=uint32"

/*
 * Opaque data for enveloped content
 */
#define PVMF_DRM_INFO_ENVELOPE_DATA_SIZE_QUERY "drm/envelope-data-size"
#define PVMF_DRM_INFO_ENVELOPE_DATA_SIZE_VALUE "drm/envelope-data-size;valtype=uint32"
#define PVMF_DRM_INFO_ENVELOPE_DATA_QUERY "drm/envelope-data"
#define PVMF_DRM_INFO_ENVELOPE_DATA_VALUE "drm/envelope-data;valtype=uint8*"

#endif // PVMI_DRM_KVP_H_INCLUDED




