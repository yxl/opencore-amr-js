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
#include "evrc_media_info_parser.h"
#include "oscl_string_utils.h"
#include "oscl_string_containers.h"

SDP_ERROR_CODE
SDPEVRCMediaInfoParser::parseMediaInfo(const char *buff, const int index, SDPInfo *sdp, payloadVector payload_vec, bool isSipSdp, int alt_id, bool alt_def_id)
{

    const char *current_start = buff; //Pointer to the beginning of the media text
    const char *end = buff + index;   //Pointer to the end of the media text
    const char *line_start_ptr, *line_end_ptr;
    int fmtp_cnt = 0;

    bool altMedia = false;
    if (!alt_id || (alt_def_id == true))
        altMedia = false;
    else
        altMedia = true;

    void *memory = sdp->alloc(sizeof(evrc_mediaInfo), altMedia);
    if (NULL == memory)
    {
        PVMF_SDP_PARSER_LOGERROR((0, "SDPEVRCMediaInfoParser::parseMediaInfo - Memory allocation failure"));
        return SDP_NO_MEMORY;
    }
    else
    {
        evrc_mediaInfo *evrcA = OSCL_PLACEMENT_NEW(memory, evrc_mediaInfo());

        evrcA->setMediaInfoID(sdp->getMediaObjectIndex());

        // Allocate memory to the payload specific objects
        for (uint32 ii = 0; ii < payload_vec.size(); ii++)
        {
            void* mem = evrcA->alloc(sizeof(EvrcPayloadSpecificInfoType));
            if (mem == NULL)
            {
                PVMF_SDP_PARSER_LOGERROR((0, "SDPEVRCMediaInfoParser::parseMediaInfo - Memory allocation failure"));
                return SDP_NO_MEMORY;
            }
            else
            {
                EvrcPayloadSpecificInfoType* evrcPayload = OSCL_PLACEMENT_NEW(mem, EvrcPayloadSpecificInfoType(payload_vec[ii]));
                (void) evrcPayload;
            }
        }


        if (alt_id && !alt_def_id)
        {
            sdp->copyFmDefMedia(evrcA);
            //empty alternate & default track ID vectors.
            evrcA->resetAlternateTrackId();
            evrcA->resetDependentTrackId();
        }

        SDP_ERROR_CODE status = baseMediaInfoParser(buff, evrcA, index, alt_id, alt_def_id, isSipSdp);
        if (status != SDP_SUCCESS)
        {
            return status;
        }


        while (get_next_line(current_start, end,
                             line_start_ptr, line_end_ptr))
        {
            switch (*line_start_ptr)
            {
                case 'a':
                {
                    const char *sptr;
                    if ((!oscl_strncmp(line_start_ptr, "a=alt:", oscl_strlen("a=alt:"))) && (alt_def_id == false))
                    {
                        line_start_ptr += oscl_strlen("a=alt:");
                        for (; *line_start_ptr != ':'; line_start_ptr++);
                        line_start_ptr = line_start_ptr + 1;
                    }
                    if (!oscl_strncmp(line_start_ptr, "a=lang:", oscl_strlen("a=lang:")))
                    {
                        sptr = line_start_ptr + oscl_strlen("a=lang:");
                        sptr = skip_whitespace(sptr, line_end_ptr);
                        if (sptr >= line_end_ptr)
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPEVRCMediaInfoParser::parseMediaInfo - Bad a=lang line format"));
                            return SDP_BAD_MEDIA_LANG_FIELD;
                        }
                        OsclMemoryFragment memFrag;
                        memFrag.ptr = (void*)sptr;
                        memFrag.len = (line_end_ptr - sptr);
                        ((evrc_mediaInfo*)evrcA)->setLang(memFrag);
                    }
                    if (!oscl_strncmp(line_start_ptr, "a=fmtp:", oscl_strlen("a=fmtp:")))
                    {
                        const char *tmp_start_line, *tmp_end_line;
                        fmtp_cnt++ ;

                        tmp_start_line = line_start_ptr + oscl_strlen("a=fmtp:");
                        tmp_start_line = skip_whitespace(tmp_start_line, line_end_ptr);
                        if (tmp_start_line >= line_end_ptr)
                        {
                            break;
                        }
                        tmp_end_line = skip_to_whitespace(tmp_start_line, line_end_ptr);
                        if (tmp_end_line < tmp_start_line)
                        {
                            break;
                        }
                        uint32 payloadNumber;
                        if (PV_atoi(tmp_start_line, 'd', tmp_end_line - tmp_start_line, payloadNumber) == false)
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPEVRCMediaInfoParser::parseMediaInfo - Bad a=fmtp line format - payload number incorrect"));
                            return SDP_BAD_MEDIA_FMTP;
                        }
                        else
                        {
                            int p;
                            if (!evrcA->lookupPayloadNumber(payloadNumber, p))
                            {
                                fmtp_cnt--;
                                break;
                            }
                        }

                        // payloadNumber is present in the mediaInfo. get the payload
                        // Specific pointer corresponding to this payload
                        EvrcPayloadSpecificInfoType* payloadPtr =
                            (EvrcPayloadSpecificInfoType*)evrcA->getPayloadSpecificInfoTypePtr(payloadNumber);
                        if (payloadPtr == NULL)
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPEVRCMediaInfoParser::parseMediaInfo - payload pointer not found for payload"));
                            return SDP_PAYLOAD_MISMATCH;
                        }

                        PVMF_SDP_PARSER_LOGINFO((0, "SDPEvrcMediaInfoParser::parseMediaInfo - processing payload number : %d", payloadNumber));

                        tmp_start_line = tmp_end_line + 1;
                        tmp_start_line = skip_whitespace(tmp_start_line, line_end_ptr);
                        if (tmp_start_line >= line_end_ptr)
                        {
                            break;
                        }
                        const char *temp = tmp_start_line;
                        int ii = 0;
                        for (ii = 0; ii < (line_end_ptr - tmp_start_line); ii++)
                        {
                            if ((tmp_start_line[ii] == ';') || (ii == (line_end_ptr - tmp_start_line - 1)))
                            {
                                tmp_end_line = tmp_start_line + ii;
                                if (ii == (line_end_ptr - tmp_start_line - 1))
                                {
                                    tmp_end_line += 1;
                                }
                                if (!oscl_strncmp(temp, "maxframes=", oscl_strlen("maxframes=")))
                                {
                                    temp += oscl_strlen("maxframes=");
                                    temp = skip_whitespace(temp, tmp_end_line);
                                    if (temp >= tmp_end_line)
                                    {
                                        PVMF_SDP_PARSER_LOGERROR((0, "SDPEVRCMediaInfoParser::parseMediaInfo - Bad a=fmtp line format - maxframes= field incorrect"));
                                        return SDP_BAD_MEDIA_FMTP;
                                    }
                                    uint32 maxFrames;
                                    if (PV_atoi(temp, 'd', (tmp_end_line - temp), maxFrames) == true)
                                        payloadPtr->setMaximumFrames(maxFrames);
                                }
                                if (!oscl_strncmp(temp, "maxbundles=", oscl_strlen("maxbundles=")))
                                {
                                    temp += oscl_strlen("maxbundles=");
                                    temp = skip_whitespace(temp, tmp_end_line);
                                    if (temp >= tmp_end_line)
                                    {
                                        PVMF_SDP_PARSER_LOGERROR((0, "SDPEVRCMediaInfoParser::parseMediaInfo - Bad a=fmtp line format - maxbundles= field incorrect"));
                                        return SDP_BAD_MEDIA_FMTP;
                                    }
                                    uint32 maxBundles;
                                    if (PV_atoi(temp, 'd', (tmp_end_line - temp), maxBundles) == true)
                                        payloadPtr->setMaximumBundle(maxBundles);
                                }
                                if (!oscl_strncmp(temp, "ptime=", oscl_strlen("ptime=")))
                                {
                                    temp += oscl_strlen("ptime=");
                                    temp = skip_whitespace(temp, tmp_end_line);
                                    if (temp >= tmp_end_line)
                                    {
                                        PVMF_SDP_PARSER_LOGERROR((0, "SDPEVRCMediaInfoParser::parseMediaInfo - Bad a=fmtp line format - Bad ptime= field"));
                                        return SDP_BAD_MEDIA_FMTP;
                                    }
                                    uint32 ptime;
                                    if (PV_atoi(temp, 'd', (tmp_end_line - temp),  ptime) == true)
                                        payloadPtr->setPacketTime(ptime);
                                }
                                if (!oscl_strncmp(temp, "decode_buf=", oscl_strlen("decode_buf=")))
                                {
                                    temp += oscl_strlen("decode_buf=");
                                    temp = skip_whitespace(temp, tmp_end_line);
                                    if (temp >= tmp_end_line)
                                    {
                                        PVMF_SDP_PARSER_LOGERROR((0, "SDPEVRCMediaInfoParser::parseMediaInfo - Bad a=fmtp line format - Bad decode_buf= field"));
                                        return SDP_BAD_MEDIA_FMTP;
                                    }
                                    uint32 decode_buf;
                                    if (PV_atoi(temp, 'd', (tmp_end_line - temp), decode_buf) == true)
                                        payloadPtr->setMaxBufferSize(decode_buf);
                                }
                                if (tmp_end_line != line_end_ptr) temp = tmp_end_line + 1;
                                temp = skip_whitespace(temp, line_end_ptr);
                                if (temp >= line_end_ptr)
                                {
                                    PVMF_SDP_PARSER_LOGERROR((0, "SDPEVRCMediaInfoParser::parseMediaInfo - Bad a=fmtp line format"));
                                    return SDP_BAD_MEDIA_FMTP;
                                }
                            }
                        }
                    }
                }
                break;
                default:
                {
                }
                break;
            }
            current_start = line_end_ptr;
        }


        sessionDescription *session = sdp->getSessionInfo();

        const char *altGroupBW = session->getAltGroupBW();
        int length = session->getAltGroupBWLength();

        if (length > 0)
        {
            status = setDependentMediaId(altGroupBW, length, evrcA, alt_id);
            if (status != SDP_SUCCESS)
                return SDP_BAD_MEDIA_ALT_ID;
        }

        const char *altGroupLANG = session->getAltGroupLANG();
        length = session->getAltGroupLANGLength();

        if (length > 0)
        {
            status = setDependentMediaId(altGroupLANG, length, evrcA, alt_id);
            if (status != SDP_SUCCESS)
                return SDP_BAD_MEDIA_ALT_ID;
        }

        if (fmtp_cnt != evrcA->getMediaPayloadNumberCount())
        {
            PVMF_SDP_PARSER_LOGERROR((0, "SDPEVRCMediaInfoParser::parseMediaInfo - Number of payloads present in m= segment does not match number of a=ftmp field count"));
            return SDP_PAYLOAD_MISMATCH;
        }

        if (evrcA->getCFieldStatus()	|| session->getCFieldStatus())
        {
            //if sample rate is zero override with defaults
            Oscl_Vector<PayloadSpecificInfoTypeBase*, SDPParserAlloc> payloadSpecificInfoVector =
                evrcA->getPayloadSpecificInfoVector();
            for (int ii = 0; ii < (int)payloadSpecificInfoVector.size();ii++)
            {
                if (payloadSpecificInfoVector[ii]->getSampleRate() == 0)
                {
                    payloadSpecificInfoVector[ii]->sampleRate =
                        PVMF_SDP_DEFAULT_EVRC_SAMPLE_RATE;
                }
            }
            return SDP_SUCCESS;
        }
        else
        {
            PVMF_SDP_PARSER_LOGERROR((0, "SDPEVRCMediaInfoParser::parseMediaInfo - c field not present"));
            return SDP_FAILURE_NO_C_FIELD;
        }

    }


}
