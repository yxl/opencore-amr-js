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

#ifndef SESSIONINFO_PARSER_H
#define SESSIONINFO_PARSER_H
//#include "sdp_return_code.h"
#include "sdp_error.h"
#include "sdp_memory.h"
#include "session_info.h"
#include "sdp_info.h"
#include "sdp_parsing_utils.h"

SDP_ERROR_CODE parseSDPSessionInfo(const char *sdp_text, int length, SDPInfo *sdp);

#endif //SESSIONINFO_PARSER_H
