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

#ifndef SDP_PARSER_H
#define SDP_PARSER_H

#include "oscl_types.h"
#include "oscl_mem.h"
#include "sdp_mediaparser_registry.h"
#include "sdp_mediaparser_factory.h"
#include "session_info_parser.h"
#include "base_media_info_parser.h"
#include "sdp_parsing_utils.h"
#include "sdp_memory.h"
#include "pvlogger.h"

const int MAX_PAYLOAD = 10;			/*Defined arbitrarily*/
const int MAX_STRING_LEN = 256;

typedef struct _SDPTrackInfo
{
    mbchar codec_type[MAX_STRING_LEN];	/* MIME type of the data */
    int bitrate;				                /* data rate in kbps     */
    int trackID;
    uint8 majorVersion;
    uint8 minorVersion;
} SDPTrackInfo;

typedef struct _playTime
{
    int startTime;
    int stopTime;
}playTime;

typedef struct _movieInfo
{
    SDPTrackInfo TrackArray[MAX_PAYLOAD];
    int trackCount;
    mbchar movieName[MAX_STRING_LEN];
    mbchar creationDate[MAX_STRING_LEN];
    playTime duration;
}movieInfo;

class SDP_Parser
{
    private:
        PVLogger* iLogger;
        SDPMediaParserRegistry *& _pSDPMediaParserRegistry;
        int mediaArrayIndex;
        SDP_ERROR_CODE(*sessionParser)(const char *sdp_text, int length, SDPInfo& sdp);	/*Used to parse session information*/
        bool applicationFlag;

        bool parse_rtpmap(const char *start, const char *end, int& rtp_payload,
                          OsclMemoryFragment& encoding_name);


        int validate_media_line(const char *start, const char *end, Oscl_Vector<int, SDPParserAlloc>& payload_type, uint32& portNumber);

        //  void print_memfrag(MemoryFragment& memfrag);
        int convertToMilliSec(RtspRangeType range , int &startTime, int &stopTime);
        bool isSipSdp;
    public:

        OSCL_IMPORT_REF
        SDP_Parser(SDPMediaParserRegistry*& regTable, bool sipSdp = false);

        OSCL_IMPORT_REF
        ~SDP_Parser();

        OSCL_IMPORT_REF
        SDP_ERROR_CODE parseSDP(const char *, int length , SDPInfo *sdp);

        OSCL_IMPORT_REF
        SDP_ERROR_CODE parseSDPDownload(const char *, int length, SDPInfo *sdp, movieInfo *mv);

        OSCL_IMPORT_REF
        int getNumberOfTracks();

        OSCL_IMPORT_REF
        int setNumberOfTracks(int tracks);

        OSCL_IMPORT_REF
        bool allocateRegistrar();                       //Used to allocate new parser.

};
#endif
