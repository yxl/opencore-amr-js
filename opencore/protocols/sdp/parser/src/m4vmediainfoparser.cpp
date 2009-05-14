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
#include "m4v_media_info_parser.h"
#include "oscl_string_utils.h"
#include "oscl_string_containers.h"

SDP_ERROR_CODE
SDPMPEG4MediaInfoParser::parseMediaInfo(const char *buff, const int index, SDPInfo *sdp, payloadVector payload_vec, bool isSipSdp, int alt_id, bool alt_def_id)
{

    const char *current_start = buff; //Pointer to the beginning of the media text
    const char *end = buff + index;   //Pointer to the end of the media text
    const char *line_start_ptr, *line_end_ptr;
    int VOLLength = 0;
    int fmtp_cnt = 0 ;
    bool framesize_found_in_fmtp = false;
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
            char *tmp_start_line, *tmp_end_line;
            fmtp_cnt++ ;

            tmp_start_line = (char *)line_start_ptr + oscl_strlen("a=fmtp:");
            tmp_start_line = (char *)skip_whitespace(tmp_start_line, line_end_ptr);
            if (tmp_start_line >= line_end_ptr)
            {
                break;
            }
            tmp_end_line = (char *)skip_to_whitespace(tmp_start_line, line_end_ptr);
            if (tmp_end_line < tmp_start_line)
            {
                break;
            }
            tmp_start_line = tmp_end_line + 1;
            tmp_start_line = (char *)skip_whitespace(tmp_start_line, line_end_ptr);
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
                        int currentVOLLength;
                        temp += oscl_strlen("config=");
                        temp = skip_whitespace(temp, line_end_ptr);
                        if (temp >= line_end_ptr)
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPM4VMediaInfoParser::parseMediaInfo - Bad a=fmtp line format - Bad config field"));
                            return SDP_BAD_MEDIA_FMTP;
                        }

                        currentVOLLength = (int)(tmp_end_line - temp) / 2;
                        if (VOLLength < currentVOLLength)
                            VOLLength = currentVOLLength;
                    }
                    if (tmp_end_line != line_end_ptr) temp = tmp_end_line + 1;
                    temp = skip_whitespace(temp, line_end_ptr);
                    if (temp >= line_end_ptr)
                    {
                        PVMF_SDP_PARSER_LOGERROR((0, "SDPM4VMediaInfoParser::parseMediaInfo - Bad a=fmtp line format"));
                        return SDP_BAD_MEDIA_FMTP;
                    }

                }
            }
        }


        current_start = line_end_ptr + 1;
    }

    if (fmtp_cnt == 0 && isSipSdp == false)
    {
        PVMF_SDP_PARSER_LOGERROR((0, "SDPM4VMediaInfoParser::parseMediaInfo - No fmtp line found"));
        return SDP_BAD_MEDIA_FORMAT;
    }

    if (VOLLength < 0)
    {
        VOLLength = 0;
    }

    bool altMedia = false;
    if (!alt_id || (alt_def_id == true))
        altMedia = false;
    else
        altMedia = true;

    void *memory = sdp->alloc(sizeof(m4v_mediaInfo), altMedia);
    if (NULL == memory)
    {
        PVMF_SDP_PARSER_LOGERROR((0, "SDPM4VMediaInfoParser::parseMediaInfo - memory allocation failure"));
        return SDP_NO_MEMORY;
    }
    else
    {
        m4v_mediaInfo *m4Video = OSCL_PLACEMENT_NEW(memory, m4v_mediaInfo());

        m4Video->setMediaInfoID(sdp->getMediaObjectIndex());

        // Allocate memory to the payload specific objects
        for (uint32 ii = 0; ii < payload_vec.size(); ii++)
        {
            void* mem = m4Video->alloc(sizeof(M4vPayloadSpecificInfoType));
            if (mem == NULL)
            {
                PVMF_SDP_PARSER_LOGERROR((0, "SDPM4VMediaInfoParser::parseMediaInfo - Memory allocation failure"));
                return SDP_NO_MEMORY;
            }
            else
            {
                M4vPayloadSpecificInfoType* payload = OSCL_PLACEMENT_NEW(mem, M4vPayloadSpecificInfoType(payload_vec[ii]));
                (void) payload;
            }
        }


        if (alt_id && !alt_def_id)
        {
            sdp->copyFmDefMedia(m4Video);
            //empty alternate & default track ID vectors.
            m4Video->resetAlternateTrackId();
            m4Video->resetDependentTrackId();
        }

        SDP_ERROR_CODE status = baseMediaInfoParser(buff, m4Video, index, alt_id, alt_def_id, isSipSdp);
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
                    if (!oscl_strncmp(line_start_ptr, "a=framerate:", oscl_strlen("a=framerate:")))
                    {
                        sptr = line_start_ptr + oscl_strlen("a=framerate:");
                        sptr = skip_whitespace(sptr, line_end_ptr);
                        if (sptr >= line_end_ptr)
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPM4VMediaInfoParser::parseMediaInfo - Bad a=framerate line format"));
                            return SDP_BAD_MEDIA_FRAME_RATE;
                        }
                        OsclFloat rate;
                        if (!PV_atof(sptr, line_end_ptr - sptr, rate))
                            return SDP_BAD_MEDIA_FORMAT;
                        ((m4v_mediaInfo *)m4Video)->setFrameRate(rate);
                    }
                    if (!oscl_strncmp(line_start_ptr, "a=I_frame_interval:", oscl_strlen("a=I_frame_interval:")))
                    {
                        sptr = line_start_ptr + oscl_strlen("a=I_frame_interval:");
                        sptr = skip_whitespace(sptr, line_end_ptr);
                        if (sptr >= line_end_ptr)
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPM4VMediaInfoParser::parseMediaInfo - Bad a=I_frame_interval line format"));
                            return SDP_BAD_MEDIA_FRAME_INTERVAL;
                        }
                        uint32 ifi;
                        if (PV_atoi(sptr, 'd', (line_end_ptr - sptr), ifi) == true)((m4v_mediaInfo *)m4Video)->setIFrameInterval(ifi);
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
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPM4VMediaInfoParser::parseMediaInfo - Bad a=fmtp line format - Bad payload number"));
                            return SDP_BAD_MEDIA_FMTP;
                        }
                        else
                        {
                            int p;
                            if (!m4Video->lookupPayloadNumber(payloadNumber, p))
                            {
                                fmtp_cnt--;
                                break;
                            }
                        }

                        M4vPayloadSpecificInfoType* payloadPtr =
                            (M4vPayloadSpecificInfoType*)m4Video->getPayloadSpecificInfoTypePtr(payloadNumber);
                        if (payloadPtr == NULL)
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPM4VMediaInfoParser::parseMediaInfo - Bad a=fmtp line format - payload pointer not found for payload"));
                            return SDP_PAYLOAD_MISMATCH;
                        }

                        PVMF_SDP_PARSER_LOGINFO((0, "SDPM4VMediaInfoParser::parseMediaInfo - processing payload number : %d", payloadNumber));

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
                                    uint8 *mptr = SDP_alloc.allocate(VOLLength);
                                    OsclRefCounterSA< SDPAllocDestructDealloc<uint8> > *refcnt = new OsclRefCounterSA< SDPAllocDestructDealloc<uint8> >(mptr);
                                    OsclSharedPtr<uint8> VOLPtr(mptr, refcnt);

                                    temp += oscl_strlen("config=");
                                    temp = skip_whitespace(temp, line_end_ptr);
                                    if (temp >= line_end_ptr)
                                    {
                                        PVMF_SDP_PARSER_LOGERROR((0, "SDPM4VMediaInfoParser::parseMediaInfo - Bad a=fmtp line format - Bad config field"));
                                        return SDP_BAD_MEDIA_FMTP;
                                    }
                                    VOLLength = (int)(tmp_end_line - temp) / 2;
                                    int idx = 0;
                                    for (idx = 0; idx < VOLLength; idx++)
                                    {
                                        uint32 val;
                                        //Set this value in the vol header array
                                        if (PV_atoi((temp + 2*idx), 'x', 2 , val) == false)
                                        {
                                            PVMF_SDP_PARSER_LOGERROR((0, "SDPM4VMediaInfoParser::parseMediaInfo - Bad a=fmtp line format - Bad config field"));
                                            return SDP_BAD_MEDIA_FMTP;
                                        }

                                        *(VOLPtr + idx) = (uint8)val;

                                    }

                                    payloadPtr->setVOLHeader(VOLPtr);
                                    payloadPtr->setVOLHeaderSize(VOLLength);
                                    payloadPtr->setDecoderSpecificInfo(VOLPtr);
                                    payloadPtr->setDecoderSpecificInfoSize(VOLLength);

                                }
                                if (!oscl_strncmp(temp, "profile-level-id=", oscl_strlen("profile-level-id=")))
                                {
                                    temp += oscl_strlen("profile-level-id=");
                                    temp = skip_whitespace(temp, line_end_ptr);
                                    if (temp > line_end_ptr)
                                    {
                                        PVMF_SDP_PARSER_LOGERROR((0, "SDPM4VMediaInfoParser::parseMediaInfo - Bad a=fmtp line format - Bad profile-level-id field"));
                                        return SDP_BAD_MEDIA_FMTP;
                                    }
                                    uint32 pl;
                                    if (PV_atoi(temp, 'd', tmp_end_line - temp ,  pl) == true)
                                        payloadPtr->setProfileLevelID(pl);

                                }
                                if (!oscl_strncmp(temp, "framesize=", oscl_strlen("framesize=")))
                                {
                                    temp += oscl_strlen("framesize=");
                                    temp = skip_whitespace(temp, tmp_end_line);
                                    framesize_found_in_fmtp  = true;
                                    if (temp > tmp_end_line)
                                    {
                                        PVMF_SDP_PARSER_LOGERROR((0, "SDPM4VMediaInfoParser::parseMediaInfo - Bad a=fmtp line format - Bad framesize field"));
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
                                        PVMF_SDP_PARSER_LOGERROR((0, "SDPM4VMediaInfoParser::parseMediaInfo - Bad a=fmtp line format - framesize width info missing"));
                                        return SDP_MISSING_MEDIA_DESCRIPTION;
                                    }
                                    uint32 width;
                                    if (PV_atoi(temp, 'd', (end - temp),  width) == true)
                                        payloadPtr->setFrameWidth(width);
                                    temp = end + 1;
                                    temp = skip_whitespace(temp, tmp_end_line);
                                    if (temp > tmp_end_line)
                                    {
                                        PVMF_SDP_PARSER_LOGERROR((0, "SDPM4VMediaInfoParser::parseMediaInfo - Bad a=fmtp line format - framesize height info missing"));
                                        return SDP_BAD_MEDIA_FMTP;
                                    }
                                    uint32 height;
                                    if (PV_atoi(temp, 'd', tmp_end_line - temp, height) == true)
                                        payloadPtr->setFrameHeight(height);
                                }
                                if (!oscl_strncmp(temp, "decode_buf=", oscl_strlen("decode_buf=")))
                                {
                                    temp += oscl_strlen("decode_buf=");
                                    temp = skip_whitespace(temp, tmp_end_line);
                                    if (temp > tmp_end_line)
                                    {
                                        PVMF_SDP_PARSER_LOGERROR((0, "SDPM4VMediaInfoParser::parseMediaInfo - Bad a=fmtp line format - Bad decode_buf field"));
                                        return SDP_BAD_MEDIA_FMTP;
                                    }
                                    uint32 dec;
                                    if (PV_atoi(temp, 'd', tmp_end_line - temp, dec) == true)
                                        payloadPtr->setMaxBufferSize(dec);
                                }
                                if (tmp_end_line != line_end_ptr) temp = tmp_end_line + 1;
                                temp = skip_whitespace(temp, line_end_ptr);
                                if (temp >= line_end_ptr)
                                {
                                    PVMF_SDP_PARSER_LOGERROR((0, "SDPM4VMediaInfoParser::parseMediaInfo - Bad a=fmtp line format"));
                                    return SDP_BAD_MEDIA_FMTP;
                                }
                            }
                        }
                    }
                    StrPtrLen fmsize("a=framesize:");
                    if (!oscl_strncmp(line_start_ptr, fmsize.c_str(), fmsize.length()))
                    {
                        uint32 width, height;
                        const char *sptr = line_start_ptr + fmsize.length();
                        const char *eptr = skip_to_whitespace(sptr, line_end_ptr);


                        if (sptr > eptr)
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPM4VMediaInfoParser::parseMediaInfo - Bad a=framesize line format"));
                            return SDP_BAD_MEDIA_FRAMESIZE;
                        }
                        uint32 payloadNo;
                        if (PV_atoi(sptr, 'd', (eptr - sptr), payloadNo))
                        {
                            int p;
                            if (!((m4v_mediaInfo *)m4Video)->lookupPayloadNumber(payloadNo, p))
                                break;

                        }
                        else
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPM4VMediaInfoParser::parseMediaInfo - Bad a=framesize line format - Bad payload number"));
                            return SDP_BAD_MEDIA_FRAMESIZE;
                        }

                        M4vPayloadSpecificInfoType* payloadPtr2 =
                            (M4vPayloadSpecificInfoType*)m4Video->getPayloadSpecificInfoTypePtr(payloadNo);
                        if (payloadPtr2 == NULL)
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPM4VMediaInfoParser::parseMediaInfo - Bad a=framesize line format - payload pointer not found for payload"));
                            return SDP_PAYLOAD_MISMATCH;
                        }

                        sptr = eptr;
                        sptr = skip_whitespace(sptr , line_end_ptr);

                        for (;*eptr != '-' ; ++eptr);

                        if (!PV_atoi(sptr, 'd', eptr - sptr, width))
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPM4VMediaInfoParser::parseMediaInfo - Bad a=framesize line format"));
                            return SDP_BAD_MEDIA_FRAMESIZE;
                        }

                        eptr = eptr + 1;
                        sptr = eptr;
                        if (sptr > line_end_ptr)
                            return SDP_BAD_MEDIA_FRAMESIZE;
                        eptr = skip_to_line_term(sptr, line_end_ptr);
                        if (!PV_atoi(sptr, 'd', eptr - sptr, height))
                            return SDP_BAD_MEDIA_FRAMESIZE;

                        if (framesize_found_in_fmtp)
                        {
                            if ((int)width != payloadPtr2->getFrameWidth() || (int)height != payloadPtr2->getFrameHeight())
                            {
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
                    break;
            }
            current_start = line_end_ptr;
        }

        sessionDescription *session = sdp->getSessionInfo();

        const char *altGroupBW = session->getAltGroupBW();
        int length = session->getAltGroupBWLength();

        if (length > 0)
        {
            status = setDependentMediaId(altGroupBW, length, m4Video, alt_id);
            if (status != SDP_SUCCESS)
                return SDP_BAD_MEDIA_ALT_ID;
        }

        const char *altGroupLANG = session->getAltGroupLANG();
        length = session->getAltGroupLANGLength();

        if (length > 0)
        {
            status = setDependentMediaId(altGroupLANG, length, m4Video, alt_id);
            if (status != SDP_SUCCESS)
                return SDP_BAD_MEDIA_ALT_ID;
        }

        if (m4Video->getCFieldStatus()	|| session->getCFieldStatus())
        {
            //if sample rate is zero override with defaults
            Oscl_Vector<PayloadSpecificInfoTypeBase*, SDPParserAlloc> payloadSpecificInfoVector =
                m4Video->getPayloadSpecificInfoVector();
            for (int ii = 0; ii < (int)payloadSpecificInfoVector.size();ii++)
            {
                if (payloadSpecificInfoVector[ii]->getSampleRate() == 0)
                {
                    payloadSpecificInfoVector[ii]->sampleRate =
                        PVMF_SDP_DEFAULT_MPEG4_VIDEO_SAMPLE_RATE;
                }
            }
            return SDP_SUCCESS;
        }
        else
        {
            PVMF_SDP_PARSER_LOGERROR((0, "SDPM4VMediaInfoParser::parseMediaInfo - No C field present"));
            return SDP_FAILURE_NO_C_FIELD;
        }
    }

}

