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
#include "rfc3640_media_info_parser.h"
#include "oscl_string_utils.h"
#include "oscl_string_containers.h"
#include "sdp_error.h"

SDP_ERROR_CODE
SDPRFC3640MediaInfoParser::parseMediaInfo(const char *buff, const int index, SDPInfo *sdp, payloadVector payload_vec, bool isSipSdp, int alt_id, bool alt_def_id)
{

    const char *current_start = buff; //Pointer to the beginning of the media text
    const char *end = buff + index;   //Pointer to the end of the media text
    const char *line_start_ptr, *line_end_ptr;
    int VOLLength = 0;
    int fmtp_cnt = 0 ;
    bool sizelength_found_in_fmtp = false;
    bool indexlength_found_in_fmtp = false;
    bool indexDeltaLength_found_in_fmtp = false;
    SDPAllocDestructDealloc<uint8> SDP_alloc;
    int strmType = 5;
    OsclMemoryFragment modeMemFrag = {NULL, 0};
    int decLength = 0;


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
                        uint32 currentLength;
                        temp += oscl_strlen("config=");
                        temp = skip_whitespace(temp, line_end_ptr);
                        if (temp >= line_end_ptr)
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPRFC3640MediaInfoParser::parseMediaInfo - no data in config= field"));
                            return SDP_BAD_MEDIA_FMTP;
                        }
                        currentLength = (int)(tmp_end_line - temp) / 2;

                        if (decLength < (int) currentLength)

                            decLength = currentLength;
                    }

                    if (!oscl_CIstrncmp(temp, "streamtype=", oscl_strlen("streamtype=")))
                    {
                        temp += oscl_strlen("streamtype=");
                        temp = skip_whitespace(temp, line_end_ptr);
                        if (temp >= line_end_ptr)
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPRFC3640MediaInfoParser::parseMediaInfo - Bad a=fmtp line format - Bad StreamType field"));
                            return SDP_BAD_MEDIA_FMTP;
                        }
                        uint32 type;
                        if (PV_atoi(temp, 'd', (tmp_end_line - temp), type) == false)
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPRFC3640MediaInfoParser::parseMediaInfo - Bad a=fmtp line format - Bad StreamType field"));
                            return SDP_BAD_MEDIA_FMTP;
                        }
                        else
                            strmType = type;
                    }
                    if (!oscl_strncmp(temp, "mode=", oscl_strlen("mode=")))
                    {
                        temp +=  oscl_strlen("mode=");
                        temp = skip_whitespace(temp, line_end_ptr);
                        if (temp >= line_end_ptr)
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPRFC3640MediaInfoParser::parseMediaInfo - Bad a=fmtp line format - Bad mode field"));
                            return SDP_BAD_MEDIA_FMTP;
                        }
                        modeMemFrag.ptr = (void*)temp;
                        modeMemFrag.len = (tmp_end_line - temp);
                    }

                    if (tmp_end_line != line_end_ptr) temp = tmp_end_line + 1;
                    temp = skip_whitespace(temp, line_end_ptr);
                    if (temp >= line_end_ptr)
                    {
                        PVMF_SDP_PARSER_LOGERROR((0, "SDPRFC3640MediaInfoParser::parseMediaInfo - Bad a=fmtp line format"));
                        return SDP_BAD_MEDIA_FMTP;
                    }

                }
            }
        }


        current_start = line_end_ptr + 1;
    }

    if (fmtp_cnt == 0 && isSipSdp == false)
    {
        PVMF_SDP_PARSER_LOGERROR((0, "SDPRFC3640MediaInfoParser::parseMediaInfo - No fmtp line found"));
        return SDP_BAD_MEDIA_FORMAT;
    }

    if (NULL == modeMemFrag.ptr)
    {
        PVMF_SDP_PARSER_LOGERROR((0, "SDPRFC3640MediaInfoParser::parseMediaInfo - Bad a=fmtp line format - No mode field"));
        return SDP_BAD_MEDIA_FMTP;
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

    void *memory = sdp->alloc(sizeof(rfc3640_mediaInfo), altMedia);
    if (NULL == memory)
    {
        PVMF_SDP_PARSER_LOGERROR((0, "SDPRFC3640MediaInfoParser::parseMediaInfo - memory allocation failure"));
        return SDP_NO_MEMORY;
    }
    else
    {
        rfc3640_mediaInfo *m3640media = OSCL_PLACEMENT_NEW(memory, rfc3640_mediaInfo());

        m3640media->setMediaInfoID(sdp->getMediaObjectIndex());
        m3640media->setStreamType(strmType);
        m3640media->setModeType(modeMemFrag);

        // Allocate memory to the payload specific objects
        for (uint32 ii = 0; ii < payload_vec.size(); ii++)
        {
            void* mem = m3640media->alloc(sizeof(RFC3640PayloadSpecificInfoType));
            if (mem == NULL)
            {
                PVMF_SDP_PARSER_LOGERROR((0, "SDPRFC3640MediaInfoParser::parseMediaInfo - Memory allocation failure"));
                return SDP_NO_MEMORY;
            }
            else
            {
                RFC3640PayloadSpecificInfoType* payload = OSCL_PLACEMENT_NEW(mem, RFC3640PayloadSpecificInfoType(payload_vec[ii]));
                (void) payload;
            }
        }


        if (alt_id && !alt_def_id)
        {
            sdp->copyFmDefMedia(m3640media);
            //empty alternate & default track ID vectors.
            m3640media->resetAlternateTrackId();
            m3640media->resetDependentTrackId();
        }

        SDP_ERROR_CODE status = baseMediaInfoParser(buff, m3640media, index, alt_id, alt_def_id, isSipSdp);
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
                    if ((!oscl_strncmp(line_start_ptr, "a=alt:", oscl_strlen("a=alt:"))) && (alt_def_id == false))
                    {
                        line_start_ptr += oscl_strlen("a=alt:");
                        for (; *line_start_ptr != ':'; line_start_ptr++);
                        line_start_ptr = line_start_ptr + 1;
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
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPRFC3640MediaInfoParser::parseMediaInfo - Bad a=fmtp line format - Bad payload number"));
                            return SDP_BAD_MEDIA_FMTP;
                        }
                        else
                        {
                            int p;
                            if (!m3640media->lookupPayloadNumber(payloadNumber, p))
                            {
                                fmtp_cnt--;
                                break;
                            }
                        }

                        RFC3640PayloadSpecificInfoType* payloadPtr =
                            (RFC3640PayloadSpecificInfoType*)m3640media->getPayloadSpecificInfoTypePtr(payloadNumber);
                        if (payloadPtr == NULL)
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPRFC3640MediaInfoParser::parseMediaInfo - Bad a=fmtp line format - payload pointer not found for payload"));
                            return SDP_PAYLOAD_MISMATCH;
                        }

                        PVMF_SDP_PARSER_LOGINFO((0, "SDPRFC3640MediaInfoParser::parseMediaInfo - processing payload number : %d", payloadNumber));

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
                                if (!oscl_CIstrncmp(temp, "profile-level-id=", oscl_strlen("profile-level-id=")))
                                {
                                    temp += oscl_strlen("profile-level-id=");
                                    temp = skip_whitespace(temp, line_end_ptr);
                                    if (temp > line_end_ptr)
                                    {
                                        PVMF_SDP_PARSER_LOGERROR((0, "SDPRFC3640MediaInfoParser::parseMediaInfo - Bad a=fmtp line format - Bad profile-level-id field"));
                                        return SDP_BAD_MEDIA_FMTP;
                                    }
                                    uint32 pl;
                                    if (PV_atoi(temp, 'd', tmp_end_line - temp ,  pl) == true)
                                        payloadPtr->setProfileLevelID(pl);

                                }
                                if (!oscl_CIstrncmp(temp, "sizelength=", oscl_strlen("SizeLength=")))
                                {
                                    temp += oscl_strlen("SizeLength=");
                                    temp = skip_whitespace(temp, tmp_end_line);
                                    sizelength_found_in_fmtp  = true;
                                    if (temp > tmp_end_line)
                                    {
                                        PVMF_SDP_PARSER_LOGERROR((0, "SDPRFC3640MediaInfoParser::parseMediaInfo - Bad a=fmtp line format - Bad sizelength field"));
                                        return SDP_BAD_MEDIA_FMTP;
                                    }
                                    uint32 length;
                                    if (PV_atoi(temp, 'd', (tmp_end_line - temp),  length) == true)
                                        payloadPtr->setSizeLength(length);
                                }
                                if (!oscl_CIstrncmp(temp, "indexlength=", oscl_strlen("IndexLength=")))
                                {
                                    temp += oscl_strlen("IndexLength=");
                                    temp = skip_whitespace(temp, tmp_end_line);
                                    indexlength_found_in_fmtp  = true;
                                    if (temp > tmp_end_line)
                                    {
                                        PVMF_SDP_PARSER_LOGERROR((0, "SDPRFC3640MediaInfoParser::parseMediaInfo - Bad a=fmtp line format - Bad indexlength field"));
                                        return SDP_BAD_MEDIA_FMTP;
                                    }
                                    uint32 length;
                                    if (PV_atoi(temp, 'd', (tmp_end_line - temp),  length) == true)
                                        payloadPtr->setIndexLength(length);
                                }
                                if (!oscl_CIstrncmp(temp, "indexdeltalength=", oscl_strlen("IndexDeltaLength=")))
                                {
                                    temp += oscl_strlen("IndexDeltaLength=");
                                    temp = skip_whitespace(temp, tmp_end_line);
                                    indexDeltaLength_found_in_fmtp  = true;
                                    if (temp > tmp_end_line)
                                    {
                                        PVMF_SDP_PARSER_LOGERROR((0, "SDPRFC3640MediaInfoParser::parseMediaInfo - Bad a=fmtp line format - Bad indexDeltaLength field"));
                                        return SDP_BAD_MEDIA_FMTP;
                                    }
                                    uint32 length;
                                    if (PV_atoi(temp, 'd', (tmp_end_line - temp),  length) == true)
                                        payloadPtr->setIndexDeltaLength(length);
                                }
                                if (!oscl_CIstrncmp(temp, "CTSDeltaLength=", oscl_strlen("CTSDeltaLength=")))
                                {
                                    temp += oscl_strlen("CTSDeltaLength=");
                                    temp = skip_whitespace(temp, tmp_end_line);
                                    indexDeltaLength_found_in_fmtp  = true;
                                    if (temp > tmp_end_line)
                                    {
                                        PVMF_SDP_PARSER_LOGERROR((0, "SDPRFC3640MediaInfoParser::parseMediaInfo - Bad a=fmtp line format - Bad CTSDeltaLength field"));
                                        return SDP_BAD_MEDIA_FMTP;
                                    }
                                    uint32 length;
                                    if (PV_atoi(temp, 'd', (tmp_end_line - temp),  length) == true)
                                        payloadPtr->setCTSDeltaLength(length);
                                }
                                if (!oscl_CIstrncmp(temp, "DTSDeltaLength=", oscl_strlen("DTSDeltaLength=")))
                                {
                                    temp += oscl_strlen("DTSDeltaLength=");
                                    temp = skip_whitespace(temp, tmp_end_line);
                                    indexDeltaLength_found_in_fmtp  = true;
                                    if (temp > tmp_end_line)
                                    {
                                        PVMF_SDP_PARSER_LOGERROR((0, "SDPRFC3640MediaInfoParser::parseMediaInfo - Bad a=fmtp line format - Bad DTSDeltaLength field"));
                                        return SDP_BAD_MEDIA_FMTP;
                                    }
                                    uint32 length;
                                    if (PV_atoi(temp, 'd', (tmp_end_line - temp),  length) == true)
                                        payloadPtr->setDTSDeltaLength(length);
                                }
                                if (!oscl_CIstrncmp(temp, "constantDuration=", oscl_strlen("ConstantDuration=")))
                                {
                                    temp += oscl_strlen("ConstantDuration=");
                                    temp = skip_whitespace(temp, tmp_end_line);
                                    indexDeltaLength_found_in_fmtp  = true;
                                    if (temp > tmp_end_line)
                                    {
                                        PVMF_SDP_PARSER_LOGERROR((0, "SDPRFC3640MediaInfoParser::parseMediaInfo - Bad a=fmtp line format - Bad constantDuration field"));
                                        return SDP_BAD_MEDIA_FMTP;
                                    }
                                    uint32 length;
                                    if (PV_atoi(temp, 'd', (tmp_end_line - temp),  length) == true)
                                        payloadPtr->setConstantDuration(length);
                                }
                                if (!oscl_CIstrncmp(temp, "maxDisplacement=", oscl_strlen("MaxDisplacement=")))
                                {
                                    temp += oscl_strlen("MaxDisplacement=");
                                    temp = skip_whitespace(temp, tmp_end_line);
                                    indexDeltaLength_found_in_fmtp  = true;
                                    if (temp > tmp_end_line)
                                    {
                                        PVMF_SDP_PARSER_LOGERROR((0, "SDPRFC3640MediaInfoParser::parseMediaInfo - Bad a=fmtp line format - Bad maxDisplacement field"));
                                        return SDP_BAD_MEDIA_FMTP;
                                    }
                                    uint32 length;
                                    if (PV_atoi(temp, 'd', (tmp_end_line - temp),  length) == true)
                                        payloadPtr->setMaxDisplacement(length);
                                }
                                if (!oscl_CIstrncmp(temp, "de-interleaveBufferSize=", oscl_strlen("de-interleaveBufferSize=")))
                                {
                                    temp += oscl_strlen("de-interleaveBufferSize=");
                                    temp = skip_whitespace(temp, tmp_end_line);
                                    indexDeltaLength_found_in_fmtp  = true;
                                    if (temp > tmp_end_line)
                                    {
                                        PVMF_SDP_PARSER_LOGERROR((0, "SDPRFC3640MediaInfoParser::parseMediaInfo - Bad a=fmtp line format - Bad de-interleaveBufferSize= field"));
                                        return SDP_BAD_MEDIA_FMTP;
                                    }
                                    uint32 length;
                                    if (PV_atoi(temp, 'd', (tmp_end_line - temp),  length) == true)
                                        payloadPtr->setDeInterleaveBufferSize(length);
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
                                        PVMF_SDP_PARSER_LOGERROR((0, "SDPRFC3640MediaInfoParser::parseMediaInfo - a=fmtp field format is incorrect for config= field"));
                                        return SDP_BAD_MEDIA_FMTP;
                                    }
                                    decLength = (int)(tmp_end_line - temp) / 2;
                                    int idx = 0;
                                    for (idx = 0; idx < decLength; idx++)
                                    {
                                        uint32 val;
                                        if (PV_atoi((temp + 2*idx), 'x', 2, val) == false)
                                        {
                                            PVMF_SDP_PARSER_LOGERROR((0, "SDPRFC3640MediaInfoParser::parseMediaInfo - a=fmtp field format is incorrect for config= field"));
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
                                        PVMF_SDP_PARSER_LOGERROR((0, "SDPRFC3640MediaInfoParser::parseMediaInfo - a=fmtp field format is incorrect for payload"));
                                        return SDP_PAYLOAD_MISMATCH;
                                    }
                                }
                                if (!oscl_strncmp(temp, "decode_buf=", oscl_strlen("decode_buf=")))
                                {
                                    temp += oscl_strlen("decode_buf=");
                                    temp = skip_whitespace(temp, tmp_end_line);
                                    if (temp > tmp_end_line)
                                    {
                                        PVMF_SDP_PARSER_LOGERROR((0, "SDPRFC3640MediaInfoParser::parseMediaInfo - Bad a=fmtp line format - Bad decode_buf field"));
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
                                    PVMF_SDP_PARSER_LOGERROR((0, "SDPRFC3640MediaInfoParser::parseMediaInfo - Bad a=fmtp line format"));
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

        if (!(indexDeltaLength_found_in_fmtp && sizelength_found_in_fmtp && indexlength_found_in_fmtp))
        {
            PVMF_SDP_PARSER_LOGERROR((0, "SDPRFC3640MediaInfoParser::parseMediaInfo - Bad a=fmtp line format - Required info missing"));
            return SDP_MISSING_MEDIA_DESCRIPTION;

        }
        sessionDescription *session = sdp->getSessionInfo();

        const char *altGroupBW = session->getAltGroupBW();
        int length = session->getAltGroupBWLength();

        if (length > 0)
        {
            status = setDependentMediaId(altGroupBW, length, m3640media, alt_id);
            if (status != SDP_SUCCESS)
                return SDP_BAD_MEDIA_ALT_ID;
        }

        const char *altGroupLANG = session->getAltGroupLANG();
        length = session->getAltGroupLANGLength();

        if (length > 0)
        {
            status = setDependentMediaId(altGroupLANG, length, m3640media, alt_id);
            if (status != SDP_SUCCESS)
                return SDP_BAD_MEDIA_ALT_ID;
        }

        return SDP_SUCCESS;
    }

}

