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

#ifndef SDP_RETURN_CODE_H
#define SDP_RETURN_CODE_H

typedef enum
{
    SDP_SUCCESS,
    SDP_VALUE_OUT_OF_RANGE,
    SDP_MISSING_ORIGIN_FIELD,
    SDP_MISSING_SESSION_NAME_FIELD,
    SDP_MISSING_AUTHOR_VERSION_FIELD,
    SDP_MISSING_AUTHOR_FIELD,
    SDP_MISSING_COPYRIGHT_FIELD,
    SDP_MISSING_RATING_FIELD,
    SDP_MISSING_CREATION_DATE_FIELD,
    SDP_MISSING_RANGE_FIELD,
    SDP_MISSING_WMF_VERSION_FIELD,
    SDP_MISSING_CONTROL_URL_FIELD,
    SDP_INVALID_MEDIA_PARAMETER,
    SDP_MISSING_MEDIA_DESCRIPTION,
    SDP_MISSING_RTP_MAP,
    SDP_FAILURE
} SDPReturnCode;
#endif
