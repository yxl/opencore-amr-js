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
#include "amr_media_info_parser.h"
#include "oscl_string_utils.h"
#include "oscl_string_containers.h"

SDP_ERROR_CODE
SDPAMRMediaInfoParser::parseMediaInfo(const char *buff,
                                      const int index,
                                      SDPInfo *sdp,
                                      payloadVector payload_vec,
                                      bool isSipSdp,
                                      int alt_id,
                                      bool alt_def_id)
{

    const char *current_start = buff; //Pointer to the beginning of the media text
    const char *end = buff + index;   //Pointer to the end of the media text
    const char *line_start_ptr, *line_end_ptr;
    int modes[] = {1, 2, 4, 8, 16, 32, 64, 128};

    int fmtp_cnt = 0;

    bool altMedia = false;
    if (!alt_id || (alt_def_id == true))
        altMedia = false;
    else
        altMedia = true;
    void *memory = NULL;

    memory = sdp->alloc(sizeof(amr_mediaInfo), altMedia);

    if (NULL == memory)
    {
        PVMF_SDP_PARSER_LOGERROR((0, "SDPAMRMediaInfoParser::parseMediaInfo - No Memory allocated"));
        return SDP_NO_MEMORY;
    }
    else
    {
        amr_mediaInfo *amrA = OSCL_PLACEMENT_NEW(memory, amr_mediaInfo());

        amrA->setMediaInfoID(sdp->getMediaObjectIndex());

        // Allocate memory to the payload specific objects
        for (uint32 ii = 0; ii < payload_vec.size(); ii++)
        {
            void* mem = amrA->alloc(sizeof(AmrPayloadSpecificInfoType));
            if (mem == NULL)
            {
                return SDP_NO_MEMORY;
            }
            else
            {
                AmrPayloadSpecificInfoType* amrPayload = OSCL_PLACEMENT_NEW(mem, AmrPayloadSpecificInfoType(payload_vec[ii]));
                (void) amrPayload;
            }
        }

        if (alt_id && !alt_def_id)
        {
            sdp->copyFmDefMedia(amrA);
            //empty alternate & default track ID vectors.
            amrA->resetAlternateTrackId();
            amrA->resetDependentTrackId();
        }

        SDP_ERROR_CODE status = baseMediaInfoParser(buff, amrA, index , alt_id, alt_def_id, isSipSdp);

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
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPAMRMediaInfoParser::parseMediaInfo - a=lang field bad"));
                            return SDP_BAD_MEDIA_LANG_FIELD;
                        }
                        OsclMemoryFragment memFrag;
                        memFrag.ptr = (void*)sptr;
                        memFrag.len = (line_end_ptr - sptr);

                        ((amr_mediaInfo*)amrA)->setLang(memFrag);
                    }

                    if (!oscl_strncmp(line_start_ptr, "a=maxptime:", oscl_strlen("a=maxptime:")))
                    {
                        sptr = line_start_ptr + oscl_strlen("a=maxptime:");
                        sptr = skip_whitespace(sptr, line_end_ptr);
                        if (sptr >= line_end_ptr)
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPAMRMediaInfoParser::parseMediaInfo - a=maxptime field bad"));
                            return SDP_BAD_MEDIA_MAXPTIME;
                        }
                        uint32 maxptime = 0;
                        if (PV_atoi(sptr, 'd', (line_end_ptr - sptr), maxptime) == true)
                        {
                            ((amr_mediaInfo*)amrA)->setMaximumPTime(maxptime);
                        }
                    }
                    if (!oscl_strncmp(line_start_ptr, "a=fmtp:", oscl_strlen("a=fmtp:")))
                    {
                        const char *tmp_start_line, *tmp_end_line;

                        fmtp_cnt++;

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
                        if (PV_atoi(tmp_start_line, 'd', (tmp_end_line - tmp_start_line), payloadNumber) == false)
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPAMRMediaInfoParser::parseMediaInfo - a=fmtp field bad for payload number"));
                            return SDP_BAD_MEDIA_FMTP;
                        }
                        else
                        {
                            int p;
                            if (!amrA->lookupPayloadNumber(payloadNumber, p))
                            {
                                fmtp_cnt--;
                                break;
                            }
                        }

                        // payloadNumber is present in the mediaInfo. get the payload
                        // Specific pointer corresponding to this payload
                        AmrPayloadSpecificInfoType* payloadPtr =
                            (AmrPayloadSpecificInfoType*)amrA->getPayloadSpecificInfoTypePtr(payloadNumber);
                        if (payloadPtr == NULL)
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPAMRMediaInfoParser::parseMediaInfo - unable to find payload ptr for payload number"));
                            return SDP_PAYLOAD_MISMATCH;
                        }

                        PVMF_SDP_PARSER_LOGINFO((0, "SDPAmrMediaInfoParser::parseMediaInfo - processing payload number : %d", payloadNumber));

                        tmp_start_line = tmp_end_line + 1;
                        tmp_start_line = skip_whitespace(tmp_start_line, line_end_ptr);
                        if (tmp_start_line >= line_end_ptr)
                        {
                            break;
                        }
                        int ii = 0;
                        const char *temp = tmp_start_line;
                        for (ii = 0; ii < (line_end_ptr - tmp_start_line); ii++)
                        {
                            if ((tmp_start_line[ii] == ';') || (ii == (line_end_ptr - tmp_start_line) - 1))
                            {
                                tmp_end_line = tmp_start_line + ii;
                                if (ii == (line_end_ptr - tmp_start_line) - 1)
                                {
                                    tmp_end_line += 1;
                                }
                                if (!oscl_strncmp(temp, "maxptime=", oscl_strlen("maxptime=")))
                                {
                                    temp += oscl_strlen("maxptime=");
                                    temp = skip_whitespace(temp, tmp_end_line);
                                    if (temp >= tmp_end_line)
                                    {
                                        PVMF_SDP_PARSER_LOGERROR((0, "SDPAMRMediaInfoParser::parseMediaInfo - a=fmtp field format is bad for maxptime="));
                                        return SDP_BAD_MEDIA_MAXPTIME;
                                    }
                                    uint32 maxPTime;
                                    if (PV_atoi(temp, 'd', (tmp_end_line - temp),  maxPTime) == true)
                                        payloadPtr->setMaximumPTime(maxPTime);
                                }
                                if (!oscl_strncmp(temp, "crc", oscl_strlen("crc")))
                                {
                                    if (!oscl_strncmp(temp, "crc=0", oscl_strlen("crc=0")))
                                        payloadPtr->setCRC(false);
                                    else
                                        payloadPtr->setCRC(true);

                                }
                                if (!oscl_strncmp(temp, "robust-sorting", oscl_strlen("robust-sorting")))
                                {


                                    //We need to make sure that we do not read beyond valid memory
                                    if ((line_end_ptr - temp) >= (int)oscl_strlen("robust-sorting=0"))
                                    {
                                        if (oscl_strncmp(temp, "robust-sorting=0", oscl_strlen("robust-sorting=0")))
                                        {
                                            payloadPtr->setRobustSorting(true);
                                        }
                                    }
                                    else
                                    {
                                        payloadPtr->setRobustSorting(false);
                                    }
                                }
                                if (!oscl_strncmp(temp, "octet-align", oscl_strlen("octet-align")))
                                {

                                    //We need to make sure that we do not read beyond valid memory
                                    if ((line_end_ptr - temp) >= (int) oscl_strlen("octet-align=0"))
                                    {
                                        if (oscl_strncmp(temp, "octet-align=0", oscl_strlen("octet-align=0")))
                                        {
                                            payloadPtr->setOctetAlign(true);
                                        }
                                    }
                                    else
                                    {
                                        payloadPtr->setOctetAlign(false);
                                    }
                                }
                                if (!oscl_strncmp(temp, "interleaving=", oscl_strlen("interleaving=")))
                                {
                                    temp += oscl_strlen("interleaving=");
                                    temp = (char *)skip_whitespace(temp, tmp_end_line);
                                    if (temp > tmp_end_line)
                                    {
                                        PVMF_SDP_PARSER_LOGERROR((0, "SDPAMRMediaInfoParser::parseMediaInfo - a=fmtp field format is bad for interleaving="));
                                        return SDP_BAD_MEDIA_FMTP;
                                    }
                                    uint32 interleave;
                                    if (PV_atoi(temp, 'd', (tmp_end_line - temp),   interleave) == true)
                                        payloadPtr->setInterLeaving(interleave);
                                }
                                if (!oscl_strncmp(temp, "decode_buf=", oscl_strlen("decode_buf=")))
                                {
                                    temp += oscl_strlen("decode_buf=");
                                    temp = skip_whitespace(temp, tmp_end_line);
                                    if (temp > tmp_end_line)
                                    {
                                        PVMF_SDP_PARSER_LOGERROR((0, "SDPAMRMediaInfoParser::parseMediaInfo - a=fmtp field format is bad for decode_buf="));
                                        return SDP_BAD_MEDIA_FMTP;
                                    }
                                    uint32 dec;
                                    if (PV_atoi(temp, 'd', (tmp_end_line - temp),   dec) == true)
                                        payloadPtr->setMaxBufferSize(dec);
                                }
                                if (!oscl_strncmp(temp, "mode-change-period=", oscl_strlen("mode-change-period=")))
                                {
                                    temp += oscl_strlen("mode-change-period=");
                                    temp = skip_whitespace(temp, tmp_end_line);
                                    if (temp > tmp_end_line)
                                    {
                                        PVMF_SDP_PARSER_LOGERROR((0, "SDPAMRMediaInfoParser::parseMediaInfo - a=fmtp field format is bad for mode-change-period="));
                                        return SDP_BAD_MEDIA_FMTP;
                                    }
                                    uint32 mcp;
                                    if (PV_atoi(temp, 'd', (tmp_end_line - temp),  mcp) == true)
                                        payloadPtr->setModeChangePeriod(mcp);
                                }
                                if (!oscl_strncmp(temp, "mode-change-neighbor", oscl_strlen("mode-change-neighbor")))
                                {
                                    //We need to make sure that we do not read beyond valid memory
                                    if ((line_end_ptr - temp) >= (int)oscl_strlen("mode-change-neighbor=0"))
                                    {
                                        if (oscl_strncmp(temp, "mode-change-neighbor=0", oscl_strlen("mode-change-neighbor=0")))
                                        {
                                            payloadPtr->setModeChangeNeighbor(true);
                                        }
                                    }
                                    else
                                    {
                                        payloadPtr->setModeChangeNeighbor(false);
                                    }
                                }
                                if (!oscl_strncmp(temp, "mode-change-neighbor=1", oscl_strlen("mode-change-neighbor=1")))
                                {
                                    payloadPtr->setModeChangeNeighbor(true);
                                }
                                if (!oscl_strncmp(temp, "mode-set=", oscl_strlen("mode-set=")))
                                {
                                    temp += oscl_strlen("mode-set=");
                                    temp = skip_whitespace(temp, tmp_end_line);
                                    if (temp > tmp_end_line)
                                    {
                                        PVMF_SDP_PARSER_LOGERROR((0, "SDPAMRMediaInfoParser::parseMediaInfo - a=fmtp field format is bad for mode-set="));
                                        return SDP_BAD_MEDIA_FMTP;
                                    }
                                    int idx = 0;
                                    int cModeList = 0;
                                    const char *begin = temp, *end;
                                    for (idx = 0; idx < (tmp_end_line - temp); idx++)
                                    {
                                        if ((temp[idx] == ',') || ((tmp_end_line - temp) - 1 == idx))
                                        {
                                            {
                                                end = temp + idx;
                                            }
                                            if ((tmp_end_line - temp) - 1 == idx)
                                            {
                                                end++;
                                            }
                                            uint32 temp_idx;
                                            if (PV_atoi(begin, 'd', (end - begin),  temp_idx) == true) cModeList += modes[temp_idx];
                                            begin = end + 1;
                                        }
                                    }
                                    payloadPtr->setCodecModeList(cModeList);
                                }
                                if (tmp_end_line != line_end_ptr) temp = tmp_end_line + 1;
                                temp = skip_whitespace(temp, line_end_ptr);
                                if (temp >= line_end_ptr)
                                {
                                    PVMF_SDP_PARSER_LOGERROR((0, "SDPAMRMediaInfoParser::parseMediaInfo - a=fmtp field format is bad"));
                                    return SDP_BAD_MEDIA_FMTP;
                                }
                            }
                        }
                    }
                }
                break;
                default:
                    break;
            }
            current_start = line_end_ptr;
        }
        sessionDescription *session = sdp->getSessionInfo();

        if (fmtp_cnt != amrA->getMediaPayloadNumberCount() && isSipSdp == false)
            return SDP_PAYLOAD_MISMATCH;

        const char *altGroupBW = session->getAltGroupBW();
        int length = session->getAltGroupBWLength();

        if (length > 0)
        {
            status = setDependentMediaId(altGroupBW, length, amrA, alt_id);
            if (status != SDP_SUCCESS)
                return SDP_BAD_MEDIA_ALT_ID;
        }

        const char *altGroupLANG = session->getAltGroupLANG();
        length = session->getAltGroupLANGLength();

        if (length > 0)
        {
            status = setDependentMediaId(altGroupLANG, length, amrA, alt_id);
            if (status != SDP_SUCCESS)
                return SDP_BAD_MEDIA_ALT_ID;
        }

        if (amrA->getCFieldStatus()	|| session->getCFieldStatus())
        {
            //if sample rate is zero override with defaults
            Oscl_Vector<PayloadSpecificInfoTypeBase*, SDPParserAlloc> payloadSpecificInfoVector =
                amrA->getPayloadSpecificInfoVector();
            for (int ii = 0; ii < (int)payloadSpecificInfoVector.size();ii++)
            {
                if (payloadSpecificInfoVector[ii]->getSampleRate() == 0)
                {
                    payloadSpecificInfoVector[ii]->sampleRate =
                        PVMF_SDP_DEFAULT_AMR_SAMPLE_RATE;
                }
            }
            return SDP_SUCCESS;
        }
        else
        {
            return SDP_FAILURE_NO_C_FIELD;
        }
    }
}
