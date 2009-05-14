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


#ifndef RTSP_RANGE_UTILS_H
#define RTSP_RANGE_UTILS_H


#include "rtsp_time_formats.h"



OSCL_IMPORT_REF bool parseRtspRange(const char *rangeString, int length,
                                    RtspRangeType& range);


OSCL_IMPORT_REF int estimate_SDP_string_len(const RtspRangeType& range);
OSCL_IMPORT_REF int estimate_SDP_string_len(const NptTimeFormat& npt_range);

OSCL_IMPORT_REF bool compose_SDP_string(char *str, unsigned int max_len,
                                        const RtspRangeType& range,
                                        int& len_used);


OSCL_IMPORT_REF bool compose_RTSP_string(char *str, unsigned int max_len,
        const RtspRangeType& range,
        int& len_used);



#endif
