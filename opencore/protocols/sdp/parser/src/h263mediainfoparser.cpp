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
#include "h263_media_info_parser.h"
#include "oscl_string_utils.h"
#include "oscl_string_containers.h"

SDP_ERROR_CODE
SDPH263MediaInfoParser::parseMediaInfo(const char *buff, const int index, SDPInfo *sdp, payloadVector payload_vec, bool isSipSdp, int alt_id, bool alt_def_id)
{

    const char *current_start = buff; //Pointer to the beginning of the media text
    const char *end = buff + index;   //Pointer to the end of the media text
    const char *line_start_ptr, *line_end_ptr;
    bool framesize_found_in_fmtp = false;
    bool framesize_found = false;
    int fmtp_cnt = 0;

    bool altMedia = false;
    if (!alt_id || (alt_def_id == true))
        altMedia = false;
    else
        altMedia = true;

    void *memory = sdp->alloc(sizeof(h263_mediaInfo), altMedia);
    if (NULL == memory)
    {
        PVMF_SDP_PARSER_LOGERROR((0, "SDPH263MediaInfoParser::parseMediaInfo - Memory allocation failure"));
        return SDP_NO_MEMORY;
    }
    else
    {
        h263_mediaInfo *h263V = OSCL_PLACEMENT_NEW(memory, h263_mediaInfo());

        h263V->setMediaInfoID(sdp->getMediaObjectIndex());

        // Allocate memory to the payload specific objects
        for (uint32 ii = 0; ii < payload_vec.size(); ii++)
        {
            void* mem = h263V->alloc(sizeof(H263PayloadSpecificInfoType));
            if (mem == NULL)
            {
                PVMF_SDP_PARSER_LOGERROR((0, "SDPH263MediaInfoParser::parseMediaInfo - Memory allocation failure"));
                return SDP_NO_MEMORY;
            }
            else
            {
                H263PayloadSpecificInfoType* h263Payload = OSCL_PLACEMENT_NEW(mem, H263PayloadSpecificInfoType(payload_vec[ii]));
                (void) h263Payload;
            }
        }


        if (alt_id && !alt_def_id)
        {
            sdp->copyFmDefMedia(h263V);
            //empty alternate & default track ID vectors.
            h263V->resetAlternateTrackId();
            h263V->resetDependentTrackId();

        }

        SDP_ERROR_CODE status = baseMediaInfoParser(buff, h263V, index, alt_id, alt_def_id, isSipSdp);
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
                    if ((!oscl_strncmp(line_start_ptr, "a=alt:", oscl_strlen("a=alt:"))) && (alt_def_id == false))
                    {
                        line_start_ptr += oscl_strlen("a=alt:");
                        for (; *line_start_ptr != ':'; line_start_ptr++);
                        line_start_ptr = line_start_ptr + 1;
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
                        if (PV_atoi(tmp_start_line, 'd', tmp_end_line - tmp_start_line,  payloadNumber) == false)
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPH263MediaInfoParser::parseMediaInfo - Bad a=fmtp line format - Bad payload number"));
                            return SDP_BAD_MEDIA_FMTP;
                        }
                        else
                        {
                            int p;
                            if (!h263V->lookupPayloadNumber(payloadNumber, p))
                            {
                                fmtp_cnt--;
                                break;
                            }
                        }

                        // payloadNumber is present in the mediaInfo. get the payload
                        // Specific pointer corresponding to this payload
                        H263PayloadSpecificInfoType* payloadPtr =
                            (H263PayloadSpecificInfoType*)h263V->getPayloadSpecificInfoTypePtr(payloadNumber);
                        if (payloadPtr == NULL)
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPH263MediaInfoParser::parseMediaInfo - Unable to find payload pointer for payload"));
                            return SDP_PAYLOAD_MISMATCH;
                        }

                        PVMF_SDP_PARSER_LOGINFO((0, "SDPH263MediaInfoParser::parseMediaInfo - processing payload number : %d", payloadNumber));

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
                                if (!oscl_strncmp(temp, "profile=", oscl_strlen("profile=")))
                                {
                                    temp += oscl_strlen("profile=");
                                    temp = skip_whitespace(temp, tmp_end_line);
                                    if (temp > tmp_end_line)
                                    {
                                        PVMF_SDP_PARSER_LOGERROR((0, "SDPH263MediaInfoParser::parseMediaInfo - Bad a=fmtp line format - Bad profile= field"));
                                        return SDP_BAD_MEDIA_FMTP;
                                    }
                                    uint32 cp;
                                    if (PV_atoi(temp, 'd', (tmp_end_line - temp), cp) == true)
                                        payloadPtr->setCodecProfile(cp);
                                }
                                if (!oscl_strncmp(temp, "level=", oscl_strlen("level=")))
                                {
                                    temp += oscl_strlen("level=");
                                    temp = skip_whitespace(temp, tmp_end_line);
                                    if (temp > tmp_end_line)
                                    {
                                        PVMF_SDP_PARSER_LOGERROR((0, "SDPH263MediaInfoParser::parseMediaInfo - Bad a=fmtp line format - Bad level= field"));
                                        return SDP_BAD_MEDIA_FMTP;
                                    }
                                    uint32 cl;
                                    if (PV_atoi(temp, 'd', (tmp_end_line - temp), cl) == true)
                                        payloadPtr->setCodecLevel(cl);
                                }
                                if (!oscl_strncmp(temp, "framesize=", oscl_strlen("framesize=")))
                                {
                                    framesize_found_in_fmtp = true;
                                    temp += oscl_strlen("framesize=");
                                    temp = skip_whitespace(temp, tmp_end_line);
                                    if (temp > tmp_end_line)
                                    {
                                        PVMF_SDP_PARSER_LOGERROR((0, "SDPH263MediaInfoParser::parseMediaInfo - Bad a=fmtp line format - Bad framesize= field"));
                                        return SDP_BAD_MEDIA_FMTP;
                                    }
                                    const char *end = NULL;
                                    int idx = 0;
                                    for (idx = 0; idx < (tmp_end_line - temp); idx++)
                                    {
                                        if (temp[idx] == '-')
                                        {
                                            end = temp + idx;
                                        }
                                    }
                                    if (end == NULL)
                                    {
                                        PVMF_SDP_PARSER_LOGERROR((0, "SDPH263MediaInfoParser::parseMediaInfo - Bad a=fmtp line format - frame width missing"));
                                        return SDP_MISSING_MEDIA_DESCRIPTION;
                                    }
                                    uint32 width;
                                    if (PV_atoi(temp, 'd', (end - temp), width) == true)
                                    {
                                        payloadPtr->setFrameWidth(width);
                                    }
                                    temp = end + 1;
                                    temp = skip_whitespace(temp, tmp_end_line);
                                    if (temp > tmp_end_line)
                                    {
                                        PVMF_SDP_PARSER_LOGERROR((0, "SDPH263MediaInfoParser::parseMediaInfo - Bad a=fmtp line format - frame height missing"));
                                        return SDP_BAD_MEDIA_FMTP;
                                    }
                                    uint32 height;
                                    if (PV_atoi(temp, 'd', (tmp_end_line - temp), height) == true)
                                    {
                                        payloadPtr->setFrameHeight(height);
                                    }
                                }
                                if (!oscl_strncmp(temp, "decode_buf=", oscl_strlen("decode_buf=")))
                                {
                                    temp += oscl_strlen("decode_buf=");
                                    temp = skip_whitespace(temp, tmp_end_line);
                                    if (temp > tmp_end_line)
                                    {
                                        PVMF_SDP_PARSER_LOGERROR((0, "SDPH263MediaInfoParser::parseMediaInfo - Bad a=fmtp line format - Bad decode_buf= field"));
                                        return SDP_BAD_MEDIA_FMTP;
                                    }
                                    uint32 decode_buf;
                                    if (PV_atoi(temp, 'd', (tmp_end_line - temp),  decode_buf) == true)
                                        payloadPtr->setMaxBufferSize(decode_buf);
                                }
                                if (tmp_end_line != line_end_ptr) temp = tmp_end_line + 1;
                                temp = skip_whitespace(temp, line_end_ptr);
                                if (temp >= line_end_ptr)
                                {
                                    PVMF_SDP_PARSER_LOGERROR((0, "SDPH263MediaInfoParser::parseMediaInfo - Bad a=fmtp line format"));
                                    return SDP_BAD_MEDIA_FMTP;
                                }
                            }
                        }
                    }  // end a=fmtp
                    StrPtrLen fmsize("a=framesize:");
                    if (!oscl_strncmp(line_start_ptr, fmsize.c_str(), fmsize.length()))
                    {
                        uint32 width, height;
                        const char *sptr = line_start_ptr + fmsize.length();
                        const char *eptr = skip_to_whitespace(sptr, line_end_ptr);

                        if (sptr > eptr)
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPH263MediaInfoParser::parseMediaInfo - Bad a=framesize line format"));
                            return SDP_BAD_MEDIA_FRAMESIZE;
                        }
                        uint32 payloadNo;
                        if (PV_atoi(sptr, 'd', (eptr - sptr), payloadNo))
                        {
                            int p;
                            if (!h263V->lookupPayloadNumber(payloadNo, p))
                                break;
                        }
                        else
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPH263MediaInfoParser::parseMediaInfo - Bad a=framesize line format - bad payload number"));
                            return SDP_BAD_MEDIA_FRAMESIZE;
                        }

                        framesize_found = true;

                        // payloadNumber is present in the mediaInfo. get the payload
                        // Specific pointer corresponding to this payload
                        H263PayloadSpecificInfoType* payloadPtr2 =
                            (H263PayloadSpecificInfoType*)h263V->getPayloadSpecificInfoTypePtr(payloadNo);
                        if (payloadPtr2 == NULL)
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPH263MediaInfoParser::parseMediaInfo - Bad a=framesize - unable to find payload pointer for the payload"));
                            return SDP_PAYLOAD_MISMATCH;
                        }

                        PVMF_SDP_PARSER_LOGINFO((0, "SDPH263MediaInfoParser::parseMediaInfo - processing payload number : %d", payloadNo));

                        sptr = eptr;
                        sptr = skip_whitespace(sptr , line_end_ptr);

                        for (;*eptr != '-' ; ++eptr);

                        if (!PV_atoi(sptr, 'd', eptr - sptr, width))
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPH263MediaInfoParser::parseMediaInfo - Bad a=framesize line format - bad width factor"));
                            return SDP_BAD_MEDIA_FRAMESIZE ;
                        }

                        eptr = eptr + 1;
                        sptr = eptr;
                        if (sptr > line_end_ptr)
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPH263MediaInfoParser::parseMediaInfo - Bad a=framesize line format"));
                            return SDP_BAD_MEDIA_FRAMESIZE;
                        }
                        eptr = skip_to_whitespace(sptr, line_end_ptr);
                        if (!PV_atoi(sptr, 'd', eptr - sptr, height))
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPH263MediaInfoParser::parseMediaInfo - Bad a=framesize line format - Bad height factor"));
                            return SDP_BAD_MEDIA_FRAMESIZE;
                        }
                        if (framesize_found_in_fmtp)
                        {
                            if ((int)width != payloadPtr2->getFrameWidth() || (int)height != payloadPtr2->getFrameHeight())
                            {
                                PVMF_SDP_PARSER_LOGERROR((0, "SDPH263MediaInfoParser::parseMediaInfo - Bad a=framesize line format - frame width & height mis match"));
                                return SDP_BAD_MEDIA_FRAMESIZE;
                            }

                        }
                        else
                        {
                            payloadPtr2->setFrameWidth(width);
                            payloadPtr2->setFrameHeight(height);
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
            status = setDependentMediaId(altGroupBW, length, h263V, alt_id);
            if (status != SDP_SUCCESS)
                return SDP_BAD_MEDIA_ALT_ID;
        }

        const char *altGroupLANG = session->getAltGroupLANG();
        length = session->getAltGroupLANGLength();

        if (length > 0)
        {
            status = setDependentMediaId(altGroupLANG, length, h263V, alt_id);
            if (status != SDP_SUCCESS)
                return SDP_BAD_MEDIA_ALT_ID;
        }

        if ((h263V->getCFieldStatus() || session->getCFieldStatus()))
        {
            //if sample rate is zero override with defaults
            Oscl_Vector<PayloadSpecificInfoTypeBase*, SDPParserAlloc> payloadSpecificInfoVector =
                h263V->getPayloadSpecificInfoVector();
            for (int ii = 0; ii < (int)payloadSpecificInfoVector.size();ii++)
            {
                if (payloadSpecificInfoVector[ii]->getSampleRate() == 0)
                {
                    payloadSpecificInfoVector[ii]->sampleRate =
                        PVMF_SDP_DEFAULT_H263_SAMPLE_RATE;
                }
            }
            return SDP_SUCCESS;
        }
        else
        {
            PVMF_SDP_PARSER_LOGERROR((0, "SDPH263MediaInfoParser::parseMediaInfo - no c field"));
            return SDP_FAILURE_NO_C_FIELD;
        }
    }


}
