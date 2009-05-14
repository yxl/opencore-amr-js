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
#include "aac_media_info_parser.h"
#include "oscl_string_utils.h"
#include "oscl_string_containers.h"

SDP_ERROR_CODE
SDPAACMediaInfoParser::parseMediaInfo(const char *buff,
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
    int decLength = 0;
    int fmtp_cnt = 0;
    SDPAllocDestructDealloc<uint8> SDP_alloc;

    while (get_next_line(current_start, end,
                         line_start_ptr, line_end_ptr))
    {
        if ((!oscl_strncmp(line_start_ptr, "a=alt:", oscl_strlen("a=alt:"))) && (alt_def_id == false))
        {
            line_start_ptr += oscl_strlen("a=alt:");
            for (; *line_start_ptr != ':'; line_start_ptr++);
            line_start_ptr = line_start_ptr + 1;
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
            tmp_start_line = tmp_end_line + 1;
            tmp_start_line = skip_whitespace(tmp_start_line, line_end_ptr);
            if (tmp_start_line >= line_end_ptr)
            {
                break;
            }
            int ii = 0;
            const char *temp = tmp_start_line;
            for (ii = 0; ii < (line_end_ptr - tmp_start_line) ; ii++)
            {
                if ((tmp_start_line[ii] == ';') || (ii == (line_end_ptr - tmp_start_line - 1)))
                {
                    tmp_end_line = tmp_start_line + ii;
                    if ((line_end_ptr - tmp_start_line - 1) == ii)
                    {
                        tmp_end_line++;
                    }

                    if (!oscl_strncmp(temp, "config=", oscl_strlen("config=")))
                    {
                        uint32 currentLength;
                        temp += oscl_strlen("config=");
                        temp = skip_whitespace(temp, line_end_ptr);
                        if (temp >= line_end_ptr)
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPAACMediaInfoParser::parseMediaInfo - no data in config= field"));
                            return SDP_BAD_MEDIA_FMTP;
                        }
                        currentLength = (int)(tmp_end_line - temp) / 2;

                        if (decLength < (int) currentLength)

                            decLength = currentLength;
                    }
                    if (tmp_end_line != line_end_ptr) temp = tmp_end_line + 1;
                    temp = skip_whitespace(temp, line_end_ptr);
                    if (temp >= line_end_ptr)
                    {
                        PVMF_SDP_PARSER_LOGERROR((0, "SDPAACMediaInfoParser::parseMediaInfo - Format in a=fmtp line is incorrect"));
                        return SDP_BAD_MEDIA_FMTP;
                    }
                }
            }
        }
        current_start = line_end_ptr + 1;
    }

    if (fmtp_cnt == 0)  // a=fmtp field not found
    {
        PVMF_SDP_PARSER_LOGERROR((0, "SDPAACMediaInfoParser::parseMediaInfo - a=fmtp field not found"));
        return SDP_FAILURE_NO_FMTP_FIELD;
    }

    bool altMedia = false;
    if (!alt_id || (alt_def_id == true))
        altMedia = false;
    else
        altMedia = true;

    //Allocate media info class here
    void *memory = sdp->alloc(sizeof(aac_mediaInfo), altMedia);
    if (NULL == memory)
    {
        PVMF_SDP_PARSER_LOGERROR((0, "SDPAACMediaInfoParser::parseMediaInfo - unable to allocate memory to media parser"));
        return SDP_NO_MEMORY;
    }
    else
    {
        aac_mediaInfo *aacA = OSCL_PLACEMENT_NEW(memory, aac_mediaInfo());

        aacA->setMediaInfoID(sdp->getMediaObjectIndex());

        // Allocate memory to the payload specific objects
        for (uint32 ii = 0; ii < payload_vec.size(); ii++)
        {
            void* mem = aacA->alloc(sizeof(AacPayloadSpecificInfoType));
            if (mem == NULL)
            {
                return SDP_NO_MEMORY;
            }
            else
            {
                OSCL_PLACEMENT_NEW(mem, AacPayloadSpecificInfoType(payload_vec[ii]));
            }
        }


        if (alt_id && !alt_def_id)
        {
            sdp->copyFmDefMedia(aacA);
            aacA->resetAlternateTrackId();
            aacA->resetDependentTrackId();
        }

        SDP_ERROR_CODE status = baseMediaInfoParser(buff, aacA, index, alt_id, alt_def_id, isSipSdp);
        if (status != SDP_SUCCESS)
        {
            return status;
        }


        current_start = buff;

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
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPAACMediaInfoParser::parseMediaInfo - a=lang field incorrect"));
                            return SDP_BAD_MEDIA_LANG_FIELD;
                        }
                        OsclMemoryFragment memFrag;
                        memFrag.ptr = (void*)sptr;
                        memFrag.len = (line_end_ptr - sptr);

                        ((aac_mediaInfo*)aacA)->setLang(memFrag);
                    }
                    if (!oscl_strncmp(line_start_ptr, "a=fmtp:", oscl_strlen("a=fmtp:")))
                    {
                        const char *tmp_start_line, *tmp_end_line;
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
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPAACMediaInfoParser::parseMediaInfo - a=fmtp field format is incorrect"));
                            return SDP_BAD_MEDIA_FMTP;
                        }
                        else
                        {
                            int p;
                            if (!aacA->lookupPayloadNumber(payloadNumber, p))
                            {
                                fmtp_cnt--;
                                break;
                            }
                        }
                        // payloadNumber is present in the mediaInfo. get the payload
                        // Specific pointer corresponding to this payload
                        AacPayloadSpecificInfoType* payloadPtr =
                            (AacPayloadSpecificInfoType*)aacA->getPayloadSpecificInfoTypePtr(payloadNumber);

                        PVMF_SDP_PARSER_LOGINFO((0, "SDPAacMediaInfoParser::parseMediaInfo - processing payload number : %d", payloadNumber));

                        tmp_start_line = tmp_end_line + 1;
                        tmp_start_line = skip_whitespace(tmp_start_line, line_end_ptr);
                        if (tmp_start_line >= line_end_ptr)
                        {
                            break;
                        }
                        int ii = 0;
                        const char *temp = tmp_start_line;
                        for (ii = 0; ii < (line_end_ptr - tmp_start_line) ; ii++)
                        {
                            if ((tmp_start_line[ii] == ';') || (ii == (line_end_ptr - tmp_start_line - 1)))
                            {
                                tmp_end_line = tmp_start_line + ii;
                                if (ii == (line_end_ptr - tmp_start_line - 1))
                                {
                                    tmp_end_line += 1;
                                }

                                if (!oscl_strncmp(temp, "config=", oscl_strlen("config=")))
                                {

                                    uint8 *mptr = SDP_alloc.allocate(decLength);
                                    OsclRefCounterSA< SDPAllocDestructDealloc<uint8> > *refcnt = new OsclRefCounterSA< SDPAllocDestructDealloc<uint8> >(mptr);
                                    OsclSharedPtr<uint8> decInfo(mptr, refcnt);

                                    temp += oscl_strlen("config=");
                                    temp = skip_whitespace(temp, line_end_ptr);
                                    if (temp >= line_end_ptr)
                                    {
                                        PVMF_SDP_PARSER_LOGERROR((0, "SDPAACMediaInfoParser::parseMediaInfo - a=fmtp field format is incorrect for config= field"));
                                        return SDP_BAD_MEDIA_FMTP;
                                    }
                                    decLength = (int)(tmp_end_line - temp) / 2;
                                    int idx = 0;
                                    for (idx = 0; idx < decLength; idx++)
                                    {
                                        uint32 val;
                                        if (PV_atoi((temp + 2*idx), 'x', 2, val) == false)
                                        {
                                            PVMF_SDP_PARSER_LOGERROR((0, "SDPAACMediaInfoParser::parseMediaInfo - a=fmtp field format is incorrect for config= field"));
                                            return SDP_BAD_MEDIA_FMTP;
                                        }

                                        *(decInfo + idx) = (uint8) val;
                                    }

                                    if (payloadPtr)
                                    {
                                        payloadPtr->setDecoderSpecificInfo(decInfo);
                                        payloadPtr->setDecoderSpecificInfoSize(decLength);
                                    }
                                    else
                                    {
                                        PVMF_SDP_PARSER_LOGERROR((0, "SDPAACMediaInfoParser::parseMediaInfo - a=fmtp field format is incorrect for payload"));
                                        return SDP_PAYLOAD_MISMATCH;
                                    }
                                }

                                // add code for cpresent flag
                                if (!oscl_strncmp(temp, "cpresent=", oscl_strlen("cpresent=")))
                                {
                                    temp += oscl_strlen("cpresent=");
                                    temp = skip_whitespace(temp, line_end_ptr);
                                    if (temp > line_end_ptr)
                                    {
                                        PVMF_SDP_PARSER_LOGERROR((0, "SDPAACMediaInfoParser::parseMediaInfo - a=fmtp format is incorrect for cpresent= field"));
                                        return SDP_BAD_MEDIA_FMTP;
                                    }
                                    uint32 cpresent; //Assuming that the possible values are either 0 or 1
                                    if (PV_atoi(temp, 'd', tmp_end_line - temp, cpresent) == true)
                                    {
                                        if (payloadPtr)
                                            payloadPtr->setcpresent(cpresent ? true : false);
                                        else
                                            return SDP_PAYLOAD_MISMATCH;
                                    }
                                }
                                if (!oscl_strncmp(temp, "SBR-enabled=", oscl_strlen("SBR-enabled=")))
                                {
                                    temp += oscl_strlen("SBR-enabled=");
                                    temp = skip_whitespace(temp, line_end_ptr);
                                    if (temp > line_end_ptr)
                                    {
                                        PVMF_SDP_PARSER_LOGERROR((0, "SDPAACMediaInfoParser::parseMediaInfo - a=fmtp format is incorrect for SBR-enabled= field"));
                                        return SDP_BAD_MEDIA_FORMAT;
                                    }
                                    uint32 sbrEn; //Assuming that the possible values are either 0 or 1
                                    if (PV_atoi(temp, 'd', tmp_end_line - temp, sbrEn) == true)
                                    {
                                        if (payloadPtr)
                                            payloadPtr->setAACplusSBRenabled(sbrEn ? true : false);
                                        else
                                            return SDP_PAYLOAD_MISMATCH;
                                    }
                                }


                                if (!oscl_strncmp(temp, "decode_buf=", oscl_strlen("decode_buf=")))
                                {
                                    temp += oscl_strlen("decode_buf=");
                                    temp = skip_whitespace(temp, tmp_end_line);
                                    if (temp > tmp_end_line)
                                    {
                                        PVMF_SDP_PARSER_LOGERROR((0, "SDPAACMediaInfoParser::parseMediaInfo - a=fmtp format is incorrect for decode_buf= field"));
                                        return SDP_BAD_MEDIA_FORMAT;
                                    }
                                    uint32 dec;
                                    if (PV_atoi(temp, 'd', tmp_end_line - temp, dec) == true)
                                    {
                                        if (payloadPtr)
                                            payloadPtr->setMaxBufferSize(dec);
                                        else
                                            return SDP_PAYLOAD_MISMATCH;
                                    }
                                }

                                if (tmp_end_line != line_end_ptr) temp = tmp_end_line + 1;
                                temp = skip_whitespace(temp, line_end_ptr);
                                if (temp >= line_end_ptr)
                                {
                                    PVMF_SDP_PARSER_LOGERROR((0, "SDPAACMediaInfoParser::parseMediaInfo - a=fmtp format is bad"));
                                    return SDP_BAD_MEDIA_FORMAT;
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

        const char *altGroupBW = session->getAltGroupBW();
        int length = session->getAltGroupBWLength();

        if (length > 0)
        {
            status = setDependentMediaId(altGroupBW, length, aacA, alt_id);
            if (status != SDP_SUCCESS)
                return SDP_BAD_MEDIA_ALT_ID;
        }

        const char *altGroupLANG = session->getAltGroupLANG();
        length = session->getAltGroupLANGLength();

        if (length > 0)
        {
            status = setDependentMediaId(altGroupLANG, length, aacA, alt_id);
            if (status != SDP_SUCCESS)
                return SDP_BAD_MEDIA_ALT_ID;
        }

        if (aacA->getCFieldStatus()	|| session->getCFieldStatus())
        {
            //if sample rate is zero override with defaults
            Oscl_Vector<PayloadSpecificInfoTypeBase*, SDPParserAlloc> payloadSpecificInfoVector =
                aacA->getPayloadSpecificInfoVector();
            for (int ii = 0; ii < (int)payloadSpecificInfoVector.size();ii++)
            {
                if (payloadSpecificInfoVector[ii]->getSampleRate() == 0)
                {
                    payloadSpecificInfoVector[ii]->sampleRate =
                        PVMF_SDP_DEFAULT_LATM_SAMPLE_RATE;
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

