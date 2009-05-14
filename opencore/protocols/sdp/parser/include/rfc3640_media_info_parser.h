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

#ifndef RFC3640_MEDIAINFO_PARSER_H_INCLUDED
#define RFC3640_MEDIAINFO_PARSER_H_INCLUDED

#ifndef BASE_MEDIAINFO_PARSER_H
#include "base_media_info_parser.h"
#endif
#ifndef RFC3640_MEDIAINFO_H_INCLUDED
#include "rfc3640_media_info.h"
#endif

SDP_ERROR_CODE rfc3640MediaInfoParser(const char *, const int , SDPInfo *);

class SDPRFC3640MediaInfoParser : public SDPBaseMediaInfoParser
{
    public:
        SDPRFC3640MediaInfoParser() {};
        virtual ~SDPRFC3640MediaInfoParser() {};
        SDP_ERROR_CODE parseMediaInfo(const char *buff, const int index, SDPInfo *sdp,
                                      payloadVector payload_vec, bool isSipSdp, int alt_id = 0, bool alt_def_id = false);
};

#endif //M4V_MEDIAINFO_PARSER_H
