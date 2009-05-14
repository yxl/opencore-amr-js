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
#include "base_media_info_parser.h"
#include "oscl_string_utils.h"
#include "oscl_string_containers.h"
#include "rtsp_range_utils.h"


/* Function to allocate temporary buffer, OSCL_TRY() put here to avoid     */
/* compiler warnings                                                       */
static void newTmpBuf(uint32 len, char** buf)
{
    int32 err;
    *buf = NULL;

    OSCL_TRY(err, *buf = OSCL_ARRAY_NEW(char, len));

    if (err != OsclErrNone)
    {
        *buf = NULL;
    }
}

SDP_ERROR_CODE
SDPBaseMediaInfoParser::baseMediaInfoParser(const char* buff,
        mediaInfo* mediaStr,
        const int index,
        const int alt_id,
        bool alt_def_id,
        bool isSipSdp)
{
    const char *current_start = buff; //Pointer to the beginning of the media text
    const char *end = buff + index;   //Pointer to the end of the media text
    const char *line_start_ptr, *line_end_ptr;


    bool a_range_found = false;

    bool a_rtpmap_found = false;
    bool a_control_found = false;
    bool a_control_set = false;

    OsclMemoryFragment memFrag;

    while (get_next_line(current_start, end,
                         line_start_ptr, line_end_ptr))
    {
        if ((!alt_def_id && !alt_id) || (alt_def_id) ||
                ((!alt_def_id) && !oscl_strncmp(line_start_ptr, "a=alt:", oscl_strlen("a=alt:"))))
        {
            if (!alt_def_id && !oscl_strncmp(line_start_ptr, "a=alt:", oscl_strlen("a=alt:")))
            {
                line_start_ptr += oscl_strlen("a=alt:");
                const char *end1 = line_start_ptr;
                for (; *end1 != ':'; end1++);
                uint32 id;
                if (!PV_atoi(line_start_ptr, 'd' , end1 - line_start_ptr, id))
                    return SDP_BAD_MEDIA_ALT_ID;
                if ((int)id != alt_id)
                {
                    //check if id is already present
                    Oscl_Vector<int, SDPParserAlloc> alt_track = mediaStr->getalternateTrackId();
                    bool found = false;

                    for (int ss = 0; ss < (int)alt_track.size();ss++)
                    {
                        if (alt_track[ss] == (int)id)
                            found = true;
                    }

                    if (!found)
                        mediaStr->setalternateTrackId(id);

                    current_start = line_end_ptr;
                    continue;
                }

                line_start_ptr = end1 + 1;
                line_start_ptr = skip_whitespace(line_start_ptr, line_end_ptr);

            }

            switch (*line_start_ptr)
            {
                case 'm':
                {
                    if (*(line_start_ptr + 1) != '=')
                    {
                        PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad m= line format"));
                        return SDP_BAD_MEDIA_FORMAT;
                    }

                    // parse through each field
                    const char *sptr, *eptr;

                    //line_start_ptr+2 since we need to start looking beyond the '=' sign

                    //get the media type (audio, video, application)
                    sptr = skip_whitespace(line_start_ptr + 2, line_end_ptr);
                    if (sptr >= line_end_ptr)
                    {
                        PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad m= line format for media type"));
                        return SDP_BAD_MEDIA_FORMAT;
                    }

                    eptr = skip_to_whitespace(sptr, line_end_ptr);
                    if (eptr <= sptr)
                    {
                        PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad m= line format"));
                        return SDP_BAD_MEDIA_FORMAT;
                    }

                    memFrag.ptr = (void*)sptr;
                    memFrag.len = (eptr - sptr);

                    mediaStr->setType(memFrag);

                    //get the suggested port number
                    sptr = skip_whitespace(eptr, line_end_ptr);
                    if (sptr >= line_end_ptr)
                    {
                        PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad m= line format for suggested port"));
                        return SDP_BAD_MEDIA_FORMAT;
                    }

                    eptr = skip_to_whitespace(sptr, line_end_ptr);
                    if (eptr <= sptr)
                    {
                        PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad m= line format"));
                        return SDP_BAD_MEDIA_FORMAT;
                    }

                    uint32 suggestedPort;

                    OSCL_HeapString<SDPParserAlloc> restOfLine(sptr, eptr - sptr);
                    const char *slash = oscl_strstr(restOfLine.get_cstr(), "/");
                    if (slash)
                    {
                        if (PV_atoi(restOfLine.get_cstr(), 'd', (slash - restOfLine.get_cstr()), suggestedPort) == true)
                        {
                            mediaStr->setSuggestedPort(suggestedPort);
                            // There must be number of ports info after the slash
                            uint32 numOfPorts;
                            const char *ports = oscl_strstr(sptr, "/");
                            ports++;
                            if (ports == NULL)
                            {
                                PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad m= line format for ports info"));
                                return SDP_BAD_MEDIA_FORMAT;
                            }
                            if (PV_atoi(ports, 'd', (eptr - ports), numOfPorts) == true)
                            {
                                mediaStr->setNumOfPorts(numOfPorts);
                            }
                            else
                            {
                                PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad m= line format for ports info"));
                                return SDP_BAD_MEDIA_FORMAT;
                            }
                        }
                        else
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad m= line format for ports info"));
                            return SDP_BAD_MEDIA_FORMAT;
                        }
                    }
                    else
                    {
                        if (PV_atoi(sptr, 'd', (eptr - sptr), suggestedPort) == true)
                        {
                            mediaStr->setSuggestedPort(suggestedPort);
                        }
                        else
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad m= line format for ports info"));
                            return SDP_BAD_MEDIA_FORMAT;
                        }
                    }
                    //get the transport profile
                    sptr = skip_whitespace(eptr, line_end_ptr);
                    if (sptr >= line_end_ptr)
                    {
                        PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad m= line format for tranport profile"));
                        return SDP_BAD_MEDIA_FORMAT;
                    }

                    eptr = skip_to_whitespace(sptr, line_end_ptr);
                    if (eptr <= sptr)
                    {
                        PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad m= line format for tranport profile"));
                        return SDP_BAD_MEDIA_FORMAT;
                    }

                    memFrag.ptr = (void*)sptr;
                    memFrag.len = (eptr - sptr);

                    if (oscl_strncmp(sptr, "RTP/AVP", (eptr - sptr)) && oscl_strncmp(sptr, "RTP/AVPF", (eptr - sptr)) && oscl_strncmp(sptr, "RTP/SAVP", (eptr - sptr)))
                    {
                        PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad m= line format for tranport profile"));
                        return SDP_BAD_MEDIA_FORMAT;
                    }
                    else if ((suggestedPort % 2)) // port number should be even
                    {
                        PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad m= line format - port number is not even"));
                        return SDP_BAD_MEDIA_FORMAT;
                    }

                    mediaStr->setTransportProfile(memFrag);

                    //get the payload number
                    sptr = skip_whitespace(eptr, line_end_ptr);
                    if (sptr >= line_end_ptr)
                    {
                        PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad m= line format for payload number"));
                        return SDP_BAD_MEDIA_FORMAT;
                    }

                    while (eptr < line_end_ptr)
                    {
                        eptr = skip_to_whitespace(sptr, line_end_ptr);
                        if (eptr <= sptr)
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad m= line format for payload number"));
                            return SDP_BAD_MEDIA_FORMAT;
                        }

                        uint32 payloadNumber; ;
                        if (PV_atoi(sptr, 'd', (eptr - sptr), payloadNumber) == true)
                        {
                            // Parse the payload number info only and see if there
                            // is any payload number in static range if yes rtpmap
                            // field may not be present for this
                            for (uint32 ii = 0; ii < mediaStr->getPayloadSpecificInfoVector().size(); ii++)
                            {
                                if (payloadNumber == mediaStr->getPayloadSpecificInfoVector()[ii]->getPayloadNumber())
                                {
                                    // check if (FIRST_STATIC_PAYLOAD <= payloadNumber <= LAST_STATIC_PAYLOAD)
                                    // since payloadNumber is unsigned and FIRST_STATIC_PAYLOAD == 0, only the upper
                                    // boundary needs to be checked. Adding the lower boundary causes compiler warning.
                                    if (payloadNumber <= LAST_STATIC_PAYLOAD)
                                        a_rtpmap_found = true;
                                }
                            }
                        }
                        else
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad m= line format for payload number"));
                            return SDP_BAD_MEDIA_FORMAT;
                        }

                        sptr = skip_whitespace(eptr, line_end_ptr);
                        eptr = sptr;
                    }
                    // No rtpmap will come if port is 0 in sip sdp
                    if (isSipSdp && suggestedPort == 0)
                    {
                        a_rtpmap_found = true;
                    }

                }
                break;
                case 'a':
                {
                    if (*(line_start_ptr + 1) != '=')
                    {
                        PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad a= line format - '=' missing"));
                        return SDP_BAD_MEDIA_FORMAT;
                    }

                    // parse through each field
                    const char *sptr1, *eptr1;
                    if (!oscl_strncmp(line_start_ptr, "a=rtpmap:", oscl_strlen("a=rtpmap:")))
                    {
                        //get the payload number
                        sptr1 = line_start_ptr + oscl_strlen("a=rtpmap:");
                        sptr1 = skip_whitespace(sptr1, line_end_ptr);

                        a_rtpmap_found = true;

                        if (sptr1 >= line_end_ptr)
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad a=rtpmap line format"));
                            return SDP_BAD_MEDIA_RTP_MAP;
                        }
                        eptr1 = skip_to_whitespace(sptr1, line_end_ptr);
                        if (eptr1 <= sptr1)
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad a=rtpmap line format"));
                            return SDP_BAD_MEDIA_RTP_MAP;
                        }
                        uint32 payloadNumber;
                        if (PV_atoi(sptr1, 'd', (eptr1 - sptr1), payloadNumber) == true)
                        {
                            int p;
                            if (!mediaStr->lookupPayloadNumber(payloadNumber, p))
                            {
                                break;
                            }
                        }
                        else
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad a=rtpmap line format for payload number"));
                            return SDP_BAD_MEDIA_RTP_MAP;
                        }

                        // payloadNumber is present in the mediaInfo. get the payload
                        // Specific pointer corresponding to this payload
                        PayloadSpecificInfoTypeBase* payloadPtr =
                            mediaStr->getPayloadSpecificInfoTypePtr(payloadNumber);
                        if (payloadPtr == NULL)
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Unable to get payload pointer for the payload"));
                            return SDP_PAYLOAD_MISMATCH;
                        }
                        PVMF_SDP_PARSER_LOGINFO((0, "SDPBaseMediaInfoParser::parseMediaInfo - processing payload number : %d", payloadNumber));

                        //get the MIME type and sample rate
                        sptr1 = skip_whitespace(eptr1, line_end_ptr);
                        if (sptr1 >= line_end_ptr)
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad a=rtpmap line format for MIME Type & Sample rate"));
                            return SDP_BAD_MEDIA_RTP_MAP;
                        }

                        eptr1 = skip_to_whitespace(sptr1, line_end_ptr);
                        if (eptr1 <= sptr1)
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad a=rtpmap line format"));
                            return SDP_BAD_MEDIA_RTP_MAP;
                        }
                        //int ii = 0;
                        const char *tmp_end_ptr = NULL;
                        /*
                        for( ii = 0; ii < (eptr1-sptr1); ii++ )
                        {
                        if(sptr1[ii] == '/')
                        {
                        tmp_end_ptr = sptr1 + ii;
                        break;
                        }
                        }
                        */
                        const char SDP_FWD_SLASH[] = "/";
                        tmp_end_ptr = oscl_strstr(sptr1, SDP_FWD_SLASH);
                        if (tmp_end_ptr == NULL)
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad a=rtpmap line format - nothing after '/' "));
                            return SDP_BAD_MEDIA_RTP_MAP;
                        }
                        //The below mentioned code converts the non standard MIME type to standard MIME type.
                        //For eg. earlier we have MIME type "AMR" and according to standard it should be
                        //like "audio/AMR". so tho whole logic implements the same.
                        uint32  tempBufLen = 0;
                        char *tmpBuf = NULL;
                        const char SDP_NULL[] = "\0";

                        tempBufLen = oscl_strlen(mediaStr->getType()) + (tmp_end_ptr - sptr1) + 2;

                        // "OSCL_TRY(err, OSCL_ARRAY_NEW(char, tempBufLen)" is in separate function to avoid warnings
                        newTmpBuf(tempBufLen, &tmpBuf);
                        if (NULL == tmpBuf)
                        {
                            return SDP_NO_MEMORY;
                        }

                        oscl_strncpy(tmpBuf, mediaStr->getType(), (oscl_strlen(mediaStr->getType()) + 1));
                        oscl_strcat(tmpBuf, SDP_FWD_SLASH);
                        oscl_strncat(tmpBuf, sptr1, (tmp_end_ptr - sptr1));
                        oscl_strcat(tmpBuf, SDP_NULL);


                        memFrag.ptr = (void*)tmpBuf;
                        memFrag.len = oscl_strlen(tmpBuf);

                        mediaStr->setMIMEType(memFrag);
                        OSCL_ARRAY_DELETE(tmpBuf);
                        tmpBuf = NULL;
                        //Till here
                        tmp_end_ptr++;
                        if (tmp_end_ptr >= eptr1)
                        {
                            return SDP_BAD_MEDIA_RTP_MAP;
                        }
                        tmp_end_ptr = skip_whitespace(tmp_end_ptr, eptr1);
                        if (tmp_end_ptr >= eptr1)
                        {
                            return SDP_BAD_MEDIA_RTP_MAP;
                        }

                        OSCL_HeapString<SDPParserAlloc> restOfLine(tmp_end_ptr, eptr1 - tmp_end_ptr);
                        const char *another_slash = oscl_strstr(restOfLine.get_cstr(), SDP_FWD_SLASH);

                        uint32 sampleRate;
                        if (another_slash)
                        {
                            if (PV_atoi(restOfLine.get_cstr(), 'd', (another_slash - restOfLine.get_cstr()), sampleRate) == true)
                            {
                                payloadPtr->setSampleRate(sampleRate);
                                // There must be channel numbers after the 2nd forward slash
                                uint32 channels;
                                tmp_end_ptr = oscl_strstr(tmp_end_ptr, SDP_FWD_SLASH);
                                if (tmp_end_ptr == NULL)
                                {
                                    PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad a=rtpmap line format for channel info"));
                                    return SDP_BAD_MEDIA_RTP_MAP;
                                }
                                tmp_end_ptr++;
                                if (PV_atoi(tmp_end_ptr, 'd', (eptr1 - tmp_end_ptr), channels) == true)
                                {
                                    payloadPtr->setNoOfChannels(channels);
                                }
                                else
                                {
                                    PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad a=rtpmap line format for channel info"));
                                    return SDP_BAD_MEDIA_RTP_MAP;
                                }
                            }
                            else
                            {
                                PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad a=rtpmap line format for channel info"));
                                return SDP_BAD_MEDIA_RTP_MAP;
                            }
                        }
                        else
                        {
                            if (PV_atoi(tmp_end_ptr, 'd', (eptr1 - tmp_end_ptr), sampleRate) == true)
                            {
                                payloadPtr->setSampleRate(sampleRate);
                            }
                            else
                            {
                                return SDP_BAD_MEDIA_RTP_MAP;
                            }
                        }
                    }
                    if (!oscl_strncmp(line_start_ptr, "a=control:", oscl_strlen("a=control:")))
                    {
                        sptr1 = line_start_ptr + oscl_strlen("a=control:");
                        sptr1 = skip_whitespace(sptr1, line_end_ptr);
                        a_control_found = true;
                        if (sptr1 >= line_end_ptr)
                        {
                            return SDP_BAD_MEDIA_CONTROL_FIELD;
                        }

                        memFrag.ptr = (void*)sptr1;
                        memFrag.len = (line_end_ptr - sptr1);
                        mediaStr->setControlURL(memFrag);

                        for (int ii = 0; ii < (line_end_ptr - sptr1); ii++)
                        {
                            if (sptr1[ii] == '=')
                            {
                                uint32 trackID;
                                sptr1 = skip_whitespace((sptr1 + ii + 1), line_end_ptr);
                                if (sptr1 >= line_end_ptr)
                                {
                                    break;
                                }

                                if ((PV_atoi(sptr1, 'd', 1, trackID) == true))
                                {
                                    mediaStr->setControlTrackID(trackID);
                                }
                                break;
                            }
                        }
                    }
                    if (!oscl_strncmp(line_start_ptr, "a=range:", oscl_strlen("a=range:")))
                    {
                        sptr1 = line_start_ptr + oscl_strlen("a=range:");
                        sptr1 = skip_whitespace(sptr1, line_end_ptr);

                        a_range_found = true;

                        if (sptr1 >= line_end_ptr)
                        {
                            return SDP_BAD_MEDIA_RANGE_FIELD;
                        }
                        parseRtspRange(sptr1, line_end_ptr - sptr1, *(mediaStr->getRtspRange()));
                    }
                    if (!oscl_strncmp(line_start_ptr, "a=depends_on:", oscl_strlen("a=depends_on:")))
                    {
                        sptr1 = line_start_ptr + oscl_strlen("a=depends_on:");
                        memFrag.ptr = (void*)sptr1;
                        memFrag.len = (line_end_ptr - sptr1);
                        mediaStr->setDependsonURL(memFrag);

                        for (int ii = 0; ii < (line_end_ptr - sptr1); ii++)
                        {
                            if (sptr1[ii] == '=')
                            {
                                uint32 trackID;
                                sptr1 = skip_whitespace((sptr1 + ii + 1), line_end_ptr);
                                if (sptr1 >= line_end_ptr)
                                {
                                    break;
                                }

                                if ((PV_atoi(sptr1, 'd', 1, trackID) == true))
                                {
                                    mediaStr->setDependsOnTrackID(trackID);
                                }
                                break;
                            }
                        }
                    }

                    //Random access denied added for 3rd party content random positioning - 01/08/02
                    StrPtrLen random_access("a=random_access_denied");
                    if (!oscl_strncmp(line_start_ptr, random_access.c_str(), random_access.length()))
                    {
                        mediaStr->setRandomAccessDenied(true);
                    }

                    StrPtrLen qoe_metrics("a=3GPP-QoE-Metrics:");
                    if (!oscl_strncmp(line_start_ptr, qoe_metrics.c_str(), qoe_metrics.length()))
                    {
                        const char *sptr;
                        sptr = line_start_ptr + qoe_metrics.length();
                        QoEMetricsType qMetrics;
                        oscl_memset(qMetrics.name, 0, 7);
                        qMetrics.rateFmt = QoEMetricsType::VAL;
                        qMetrics.rateVal = 0;
                        qMetrics.paramFmt = QoEMetricsType::IDIGIT;
                        qMetrics.paramExtIdigit = 0;

                        if (!parseQoEMetrics(sptr, line_end_ptr, qMetrics))
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad a=3GPP-QoE-Metrics: line format"));
                            return SDP_BAD_MEDIA_FORMAT;
                        }
                        mediaStr->setQoEMetrics(qMetrics);


                    }
                    StrPtrLen predec("a=X-predecbufsize:");
                    if (!oscl_strncmp(line_start_ptr, predec.c_str(), predec.length()))
                    {
                        const char *sptr;
                        sptr = line_start_ptr + predec.length();
                        uint32 size;
                        if (!PV_atoi(sptr, 'd', (line_end_ptr - sptr), size))
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad a=X-predecbufsize: line format"));
                            return SDP_BAD_MEDIA_FORMAT;
                        }
                        mediaStr->setPreDecBuffSize(size);
                    }

                    StrPtrLen initpredec("a=X-initpredecbufperiod:");
                    if (!oscl_strncmp(line_start_ptr, initpredec.c_str(), initpredec.length()))
                    {
                        const char *sptr;
                        sptr = line_start_ptr + initpredec.length();
                        uint32 period;
                        if (!PV_atoi(sptr, 'd', (line_end_ptr - sptr), period))
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad a=X-initpredecbufperiod: line format"));
                            return SDP_BAD_MEDIA_FORMAT;
                        }
                        mediaStr->setInitPreDecBuffPeriod(period);
                    }

                    StrPtrLen initpostdec("a=X-initpostdecbufperiod:");
                    if (!oscl_strncmp(line_start_ptr, initpostdec.c_str(), initpostdec.length()))
                    {
                        const char *sptr;
                        sptr = line_start_ptr + initpostdec.length();
                        uint32 period;
                        if (!PV_atoi(sptr, 'd', (line_end_ptr - sptr), period))
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad a=X-initpostdecbufperiod: line format"));
                            return SDP_BAD_MEDIA_FORMAT;
                        }
                        mediaStr->setInitPostDecBuffPeriod(period);
                    }

                    StrPtrLen decbyterate("a=X-decbyterate:");
                    if (!oscl_strncmp(line_start_ptr, decbyterate.c_str(),
                                      decbyterate.length()))
                    {
                        const char *sptr;
                        sptr = line_start_ptr + decbyterate.length();
                        uint32 rate;
                        if (!PV_atoi(sptr, 'd', (line_end_ptr - sptr), rate))
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad a=X-decbyterate: line format"));
                            return SDP_BAD_MEDIA_FORMAT;
                        }
                        mediaStr->setDecByteRate(rate);
                    }

                    StrPtrLen adapt_supp("a=3GPP-Adaptation-Support:");
                    if (!oscl_strncmp(line_start_ptr, adapt_supp.c_str(),
                                      adapt_supp.length()))
                    {
                        const char *sptr = line_start_ptr + adapt_supp.length();
                        sptr = skip_whitespace_and_line_term(sptr, line_end_ptr);
                        uint32 frequency;
                        if (!PV_atoi(sptr, 'd', line_end_ptr - sptr, frequency))
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad a=3GPP-Adaptation-Support: line format - frequency not correct"));
                            return SDP_BAD_MEDIA_FORMAT;
                        }
                        mediaStr->setReportFrequency(frequency);
                    }

                    StrPtrLen asset_info("a=3GPP-Asset-Information:");
                    if (!oscl_strncmp(line_start_ptr, asset_info.c_str(),
                                      asset_info.length()))
                    {
                        const char *sptr = line_start_ptr + asset_info.length();
                        AssetInfoType assetInfo;
                        if (!parseAssetInfo(sptr, line_end_ptr, assetInfo))
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad a=3GPP-Asset-Information: line format"));
                            return SDP_BAD_SESSION_FORMAT;
                        }

                        mediaStr->setAssetInfo(assetInfo);

                    }
                    StrPtrLen srtp("a=3GPP-SRTP-Config:");
                    if (!oscl_strncmp(line_start_ptr, srtp.c_str(),
                                      srtp.length()))
                    {
                        const char *sptr = line_start_ptr + srtp.length();
                        const char *eptr;
                        sptr = skip_whitespace(sptr, line_end_ptr);
                        eptr = skip_to_whitespace(sptr, line_end_ptr);
                        memFrag.ptr = (void *)sptr;
                        memFrag.len = eptr - sptr;

                        mediaStr->setSRTPintg_nonce(memFrag);

                        eptr = eptr + 1;
                        sptr = eptr;
                        if (sptr >= line_end_ptr)
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad a=3GPP-SRTP-Config: line format"));
                            return SDP_BAD_MEDIA_FORMAT;
                        }
                        eptr = skip_to_whitespace(eptr, line_end_ptr);
                        if (eptr >= line_end_ptr)
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad a=3GPP-SRTP-Config: line format"));
                            return SDP_BAD_MEDIA_FORMAT;
                        }

                        memFrag.ptr = (void *)sptr;
                        memFrag.len = eptr - sptr;
                        mediaStr->setSRTPkey_salt(memFrag);

                        eptr = eptr + 1;
                        sptr = eptr;

                        if (!oscl_strncmp(sptr, "auth-tag-len=", oscl_strlen("auth-tag-len=")))
                        {
                            sptr = sptr + oscl_strlen("auth-tag-len=");
                            uint32 length;
                            if (!PV_atoi(sptr, 'd', 2, length))
                            {
                                PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad a=3GPP-SRTP-Config: line format for auth-tag-len= field"));
                                return SDP_BAD_MEDIA_FORMAT;
                            }
                            if ((length != 32) && (length != 80))
                            {
                                PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad a=3GPP-SRTP-Config: line format for auth-tag-len= field"));
                                return SDP_BAD_MEDIA_FORMAT;
                            }
                            else
                                mediaStr->setSRTPauth_tag_len(length);

                        }
                        else
                        {
                            memFrag.ptr = (void *)sptr;
                            memFrag.len = line_end_ptr - sptr;

                            mediaStr->setSRTPparam_ext(memFrag);


                        }


                    }
                    StrPtrLen rtcp_fb("a=rtcp-fb:");
                    if (!oscl_strncmp(line_start_ptr, rtcp_fb.c_str(), rtcp_fb.length()))
                    {
                        const char *sptr = line_start_ptr + rtcp_fb.length();
                        const char *eptr = skip_to_whitespace(sptr, line_end_ptr);

                        if (eptr >= line_end_ptr)
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad a=rtcp-fb: line format"));
                            return SDP_BAD_MEDIA_FORMAT;
                        }

                        memFrag.ptr = (void *)sptr;
                        memFrag.len = eptr - sptr;
                        mediaStr->setrtcp_fb_pt(memFrag);

                        sptr = skip_whitespace(eptr, line_end_ptr);
                        eptr = skip_to_whitespace(sptr, line_end_ptr);

                        memFrag.ptr = (void *)sptr;
                        memFrag.len = eptr - sptr;
                        mediaStr->setrtcp_fb_val(memFrag);

                        if (eptr >= line_end_ptr)
                            break;

                        if (!oscl_strncmp(sptr, "trr-int", eptr - sptr))
                        {
                            sptr = skip_whitespace(eptr, line_end_ptr);
                            eptr = skip_to_line_term(sptr, line_end_ptr);
                            uint32 trr;
                            if (!PV_atoi(sptr, 'd', eptr - sptr, trr))
                            {
                                PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad a=rtcp-fb: line format for trr-int field"));
                                return SDP_BAD_MEDIA_FORMAT;
                            }

                            mediaStr->setrtcp_fb_trr_val(trr);
                        }
                        else
                        {
                            sptr = skip_whitespace(eptr, line_end_ptr);
                            eptr = skip_to_line_term(sptr, line_end_ptr);
                            memFrag.ptr = (void *)sptr;
                            memFrag.len = eptr - sptr;
                            mediaStr->setrtcp_fb_val_param(memFrag);

                        }


                    }
                    if (!oscl_strncmp(line_start_ptr, "a=alt:", oscl_strlen("a=alt:")))
                    {
                        line_start_ptr += oscl_strlen("a=alt:");
                        const char *end1 = line_start_ptr;
                        for (; *end1 != ':'; end1++);
                        uint32 id;
                        if (!PV_atoi(line_start_ptr, 'd' , end1 - line_start_ptr, id))
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad ID in a=alt: line format"));
                            return SDP_BAD_MEDIA_ALT_ID;
                        }

                        //check if id is already present
                        Oscl_Vector<int, SDPParserAlloc> alt_track = mediaStr->getalternateTrackId();
                        bool found = false;
                        for (int ss = 0; ss < (int)alt_track.size();ss++)
                        {
                            if (alt_track[ss] == (int)id)
                                found = true;
                        }
                        if (!found)
                            mediaStr->setalternateTrackId(id);
                    }
                    if (!oscl_strncmp(line_start_ptr, "a=maxprate:", oscl_strlen("a=maxprate:")))
                    {
                        line_start_ptr += oscl_strlen("a=maxprate:");
                        OsclFloat rate;
                        if (!PV_atof(line_start_ptr, line_end_ptr - line_start_ptr, rate))
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad a=maxprate: line format for rate field"));
                            return SDP_BAD_MEDIA_FORMAT;
                        }
                        mediaStr->setMaxprate(rate);
                    }
                    if (!oscl_strncmp(line_start_ptr, "a=X-allowrecord", oscl_strlen("a=X-allowrecord")))
                    {
                        mediaStr->setAllowRecord(true);
                    }

                }
                break;
                case 'b':
                {
                    if (!oscl_strncmp(line_start_ptr, "b=AS:", oscl_strlen("b=AS:")))
                    {
                        const char *sptr;
                        sptr = line_start_ptr + oscl_strlen("b=AS:");
                        sptr = skip_whitespace(sptr, line_end_ptr);
                        if (sptr >= line_end_ptr)
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad b=AS: line format"));
                            return SDP_BAD_MEDIA_FORMAT;
                        }

                        uint32 bitRate;
                        if (PV_atoi(sptr, 'd', (line_end_ptr - sptr),  bitRate) == true)
                        {
                            mediaStr->setBitrate(1000*bitRate);
                        }
                        else
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad b=AS: line format - bitrate incorrect"));
                            return SDP_BAD_MEDIA_FORMAT;
                        }
                    }
                    else if (!oscl_strncmp(line_start_ptr, "b=RS:", oscl_strlen("b=RS:")))
                    {
                        const char *sptr;
                        sptr = line_start_ptr + oscl_strlen("b=AS:");
                        sptr = skip_whitespace(sptr, line_end_ptr);
                        if (sptr >= line_end_ptr)
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad b=RS: line format"));
                            return SDP_BAD_MEDIA_FORMAT;
                        }

                        uint32 rtcpBWSender;
                        if (PV_atoi(sptr, 'd', (line_end_ptr - sptr),  rtcpBWSender) == true)
                        {
                            mediaStr->setRTCPSenderBitRate(rtcpBWSender);
                        }
                        else
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad b=RS: line format - Sender Bitrate incorrect"));
                            return SDP_BAD_MEDIA_FORMAT;
                        }
                    }
                    else if (!oscl_strncmp(line_start_ptr, "b=RR:", oscl_strlen("b=RR:")))
                    {
                        const char *sptr;
                        sptr = line_start_ptr + oscl_strlen("b=AS:");
                        sptr = skip_whitespace(sptr, line_end_ptr);
                        if (sptr >= line_end_ptr)
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad b=RR: line format"));
                            return SDP_BAD_MEDIA_FORMAT;
                        }

                        uint32 rtcpBWReceiver;
                        if (PV_atoi(sptr, 'd', (line_end_ptr - sptr),  rtcpBWReceiver) == true)
                        {
                            mediaStr->setRTCPReceiverBitRate(rtcpBWReceiver);
                        }
                        else
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad b=RR: line format - Receiver Bit rate incorrect"));
                            return SDP_BAD_MEDIA_FORMAT;
                        }
                    }
                    else if (!oscl_strncmp(line_start_ptr, "b=TIAS:", oscl_strlen("b=TIAS:")))
                    {
                        const char *sptr;
                        sptr = line_start_ptr + oscl_strlen("b=TIAS:");
                        sptr = skip_whitespace(sptr, line_end_ptr);
                        if (sptr >= line_end_ptr)
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad b=T1AS: line format"));
                            return SDP_BAD_MEDIA_FORMAT;
                        }

                        uint32 bMod;
                        if (PV_atoi(sptr, 'd', (line_end_ptr - sptr),  bMod) == true)
                        {
                            mediaStr->setBWtias(1000 * bMod);
                        }
                        else
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad b=T1AS: line format - bMod incorrect"));
                            return SDP_BAD_MEDIA_FORMAT;
                        }
                    }
                }
                break;
                case 'u':
                {
                    PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - u field not supported"));
                    return SDP_BAD_MEDIA_FORMAT;
                }
                case 'c':
                {
                    if (*(line_start_ptr + 1) != '=')
                    {
                        PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad c= line format - '=' missing after c"));
                        return SDP_BAD_SESSION_FORMAT;
                    }

                    mediaStr->setCFieldStatus(true);

                    // parse through each field
                    const char *sptr, *eptr;

                    // get the connection network type
                    sptr = skip_whitespace(line_start_ptr + 2, line_end_ptr);
                    if (sptr >= line_end_ptr)
                    {
                        PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad c= line format - connection network type missing"));
                        return SDP_BAD_SESSION_FORMAT;
                    }

                    eptr = skip_to_whitespace(sptr, line_end_ptr);
                    if (eptr <= sptr)
                    {
                        PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad c= line format - part after connection network type missing"));
                        return SDP_BAD_SESSION_FORMAT;
                    }

                    memFrag.ptr = (void*)sptr;
                    memFrag.len = (eptr - sptr);
                    mediaStr->setCNetworkType(memFrag);

                    // get the address type
                    sptr = skip_whitespace(eptr, line_end_ptr);
                    if (sptr >= line_end_ptr)
                    {
                        PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad c= line format - address type missing"));
                        return SDP_BAD_SESSION_FORMAT;
                    }

                    eptr = skip_to_whitespace(sptr, line_end_ptr);
                    if (eptr <= sptr)
                    {
                        PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad c= line format - part after address type missing"));
                        return SDP_BAD_SESSION_FORMAT;
                    }
                    memFrag.ptr = (void*)sptr;
                    memFrag.len = (eptr - sptr);
                    mediaStr->setCAddressType(memFrag);

                    // get the address
                    sptr = skip_whitespace(eptr, line_end_ptr);
                    if (sptr >= line_end_ptr)
                    {
                        PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad c= line format - address missing"));
                        return SDP_BAD_SESSION_FORMAT;
                    }

                    eptr = skip_to_whitespace(sptr, line_end_ptr);
                    if (eptr < sptr)
                    {
                        PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad c= line format - part after address missing"));
                        return SDP_BAD_SESSION_FORMAT;
                    }
                    memFrag.ptr = (void*)sptr;
                    memFrag.len = (eptr - sptr);
                    mediaStr->setCAddress(memFrag);
                    uint32 len = OSCL_MIN((uint32)(eptr - sptr), oscl_strlen("IP4"));
                    if (oscl_strncmp(sptr, "IP4", len) == 0)
                    {
                        uint32 address;
                        const char *addrend = sptr;
                        for (;*addrend != '.';++addrend);

                        if (!PV_atoi(sptr, 'd', addrend - sptr, address))
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad c= line format"));
                            return SDP_BAD_SESSION_FORMAT;
                        }

                        if (address >= 224 && address <= 239) //multicast address look for TTL
                        {
                            for (; (*sptr != '/') && (sptr < eptr); ++sptr);
                            if (sptr == eptr)
                            {
                                PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad c= line format"));
                                return SDP_BAD_SESSION_FORMAT; // no TTL found in multicast address.
                            }
                            else
                            {
                                uint32 ttl;
                                sptr = sptr + 1;
                                if (!PV_atoi(sptr, 'd', eptr - sptr, ttl))
                                {
                                    PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad c= line format"));
                                    return SDP_BAD_SESSION_FORMAT;
                                }
                                if (!(ttl <= 255))
                                {
                                    PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad c= line format"));
                                    return SDP_BAD_SESSION_FORMAT; // ttl out of range.
                                }

                            }

                        }
                        else  // unicast address
                        {
                            for (; (*sptr != '/') && (sptr < eptr); ++sptr);
                            if (!oscl_strncmp(sptr, "/", 1))
                            {
                                PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad c= line format"));
                                return SDP_BAD_SESSION_FORMAT; //unicast address can not have TTL.
                            }
                        }

                        if (eptr < line_end_ptr)
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad c= line format"));
                            return SDP_BAD_SESSION_FORMAT;
                        }
                    }
                    //use "len" here since "IP4" and "IP6" have same lengths
                    else if (oscl_strncmp(sptr, "IP6", len) == 0)
                    {
                        //TBD
                    }
                    break;
                }

                default:
                {
                    //skip a line we don't understand
                }
                break;
            }
        }
        current_start = line_end_ptr;
    }

    mediaStr->setmediaTrackId(alt_id);

    if (!alt_def_id && alt_id)
    {
        uint32 defaultId;
        getAltDefaultId(buff, buff + index, defaultId);
        if (defaultId != 0)
            mediaStr->setalternateTrackId(defaultId);
        else
            return SDP_BAD_MEDIA_ALT_ID;
    }

    if (!a_control_found)
    {
        uint32 addr;
        connectionInfo ci;
        mediaStr->getConnectionInformation(&ci);
        PV_atoi(ci.connectionAddress.get_cstr(), 'd', addr);
        //224.0.0.0 through 239.255.255.255 represent class D network addresses
        //reserved for multicasting, which indicate a DVB connection
        if (addr >= 224 && addr <= 239)
        {
            uint32 id = mediaStr->getMediaInfoID();
            mediaStr->setControlTrackID(id);
            a_control_set = true;
        }
    }

    /*
     * cannot assume that range is always going to be set at media level
     */
    if ((isSipSdp && a_rtpmap_found) || (!alt_def_id && alt_id))
        return SDP_SUCCESS;
    else if ((a_rtpmap_found && a_control_found) || (!alt_def_id && alt_id) || (a_control_set))
        return SDP_SUCCESS;
    else
    {
        PVMF_SDP_PARSER_LOGERROR((0, "SDPBaseMediaInfoParser::parseMediaInfo - Bad Media - no rtpmap and control present"));
        return SDP_BAD_MEDIA_FORMAT;
    }
}


SDP_ERROR_CODE SDPBaseMediaInfoParser::getAltDefaultId(const char* start, const char *end, uint32 &defaultId)
{
    const char *current_start = start;
    const char *line_start_ptr, *line_end_ptr;
    defaultId = 0;
    while (get_next_line(current_start, end,
                         line_start_ptr, line_end_ptr))
    {
        switch (*line_start_ptr)
        {
            case 'a':
            {
                if (!oscl_strncmp(line_start_ptr, "a=alt-default-id:", oscl_strlen("a=alt-default-id:")))
                {
                    line_start_ptr += oscl_strlen("a=alt-default-id:");

                    if (!PV_atoi(line_start_ptr, 'd', line_end_ptr - line_start_ptr, defaultId))
                        return SDP_BAD_MEDIA_ALT_ID;
                    else
                        return SDP_SUCCESS;

                }
            }
            break;
            default:
                break;

        }

        current_start = line_end_ptr;
    }

    return SDP_SUCCESS;
}

SDP_ERROR_CODE SDPBaseMediaInfoParser::setDependentMediaId(const char *start, int length, mediaInfo *mediaPtr, int mediaId)
{
    const char *startPtr = start;
    const char *endLine = start + length;

    while (startPtr < endLine)
    {
        for (; *startPtr != '='; ++startPtr);
        startPtr = startPtr + 1;
        if (startPtr > endLine)
            return SDP_BAD_MEDIA_ALT_ID;
        const char *endPtr = startPtr;
        for (; (*endPtr != ';') && (endPtr != endLine); ++endPtr);

        if (endPtr > endLine)
            return SDP_BAD_MEDIA_ALT_ID;

        if (lookForMediaId(startPtr, endPtr, mediaId))
        {
            while (startPtr < endPtr)
            {
                const char *end = startPtr;
                for (; (*end != ',') && (end < endPtr) ; ++end);
                uint32 id;
                if (!PV_atoi(startPtr, 'd', end - startPtr, id))
                    return SDP_BAD_MEDIA_ALT_ID;
                if ((int)id != mediaId)
                    mediaPtr->setdependentTrackId(id);
                startPtr = end + 1;
            }
        }
        else
            startPtr = endPtr + 1;
    }

    return SDP_SUCCESS;

}

bool SDPBaseMediaInfoParser::lookForMediaId(const char *startPtr, const char* endPtr, int mediaId)
{
    const char *end = startPtr;

    while (startPtr < endPtr)
    {
        for (; (*end != ',') && (end < endPtr); ++end);
        uint32 id;
        PV_atoi(startPtr, 'd' , end - startPtr, id);
        if ((int)id == mediaId)
            return true;
        end = end + 1;
        startPtr = end;
    }

    return false;
}

