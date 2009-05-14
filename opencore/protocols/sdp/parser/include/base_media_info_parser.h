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

#ifndef BASE_MEDIAINFO_PARSER_H
#define BASE_MEDIAINFO_PARSER_H
#include "oscl_mem.h"
#include "sdp_info.h"
#include "sdp_error.h"
#include "media_info.h"
#include "sdp_parsing_utils.h"
#include "oscl_mem.h"
#include "pvlogger.h"

#define PVMF_SDP_PARSER_LOGERROR(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iLogger,PVLOGMSG_ERR,m);
#define PVMF_SDP_PARSER_LOGWARNING(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iLogger,PVLOGMSG_WARNING,m);
#define PVMF_SDP_PARSER_LOGINFOHI(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG,iLogger,PVLOGMSG_INFO,m);
#define PVMF_SDP_PARSER_LOGINFOMED(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG,iLogger,PVLOGMSG_INFO,m);
#define PVMF_SDP_PARSER_LOGINFOLOW(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG,iLogger,PVLOGMSG_INFO,m);
#define PVMF_SDP_PARSER_LOGINFO(m) PVMF_SDP_PARSER_LOGINFOMED(m)

typedef Oscl_Vector<int, SDPParserAlloc> payloadVector;
class SDPBaseMediaInfoParser
{
    public:
        SDPBaseMediaInfoParser()
        {
            iLogger = PVLogger::GetLoggerObject("SDPBaseMediaInfoParser");
        };
        virtual ~SDPBaseMediaInfoParser() {};
        virtual SDP_ERROR_CODE parseMediaInfo(const char *buff, const int index, SDPInfo *sdp,
                                              payloadVector payload_vec, bool isSipSdp, int alt_id = 0, bool alt_def_id = false) = 0;
        SDP_ERROR_CODE baseMediaInfoParser(const char *, mediaInfo *, const int, const int, bool, bool isSipSdp);
        SDP_ERROR_CODE getAltDefaultId(const char* start, const char *end, uint32 &defaultId);
        SDP_ERROR_CODE setDependentMediaId(const char *start, int length, mediaInfo *mediaPtr, int mediaId);
        bool lookForMediaId(const char *startPtr, const char* endPtr, int mediaId);
    protected:
        PVLogger* iLogger;
};

#endif//BASE_MEDIAINFO_PARSER_H
