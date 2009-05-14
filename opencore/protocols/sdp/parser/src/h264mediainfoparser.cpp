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
#include "h264_media_info_parser.h"
#include "oscl_string_utils.h"
#include "oscl_string_containers.h"
#include "sdp_parsing_utils.h"

#define MIN_ENCODED_BYTES 4
#define AVC_NALTYPE_MASK 0x1F
#define AVC_NALTYPE_SPS 7
#define AVC_NALTYPE_PPS 8

SDP_ERROR_CODE
SDPH264MediaInfoParser::parseMediaInfo(const char *buff,
                                       const int index,
                                       SDPInfo *sdp,
                                       payloadVector payload_vec,
                                       bool isSipSdp,
                                       int alt_id,
                                       bool alt_def_id)
{
    //Pointer to the beginning of the media text
    const char *current_start = buff;
    //Pointer to the end of the media text
    const char *end = buff + index;
    const char *line_start_ptr, *line_end_ptr;
    int fmtp_cnt = 0;
    bool altMedia = false;

    if (!alt_id || (alt_def_id == true))
    {
        altMedia = false;
    }
    else
    {
        altMedia = true;
    }
    void *memory = sdp->alloc(sizeof(h264_mediaInfo), altMedia);
    if (NULL == memory)
    {
        PVMF_SDP_PARSER_LOGERROR((0, "SDPH264MediaInfoParser::parseMediaInfo - Memory allocation failure"));
        return SDP_NO_MEMORY;
    }
    else
    {
        h264_mediaInfo *h264V = OSCL_PLACEMENT_NEW(memory, h264_mediaInfo());

        h264V->setMediaInfoID(sdp->getMediaObjectIndex());

        // Allocate memory to the payload specific objects
        for (uint32 ii = 0; ii < payload_vec.size(); ii++)
        {
            void* mem = h264V->alloc(sizeof(H264PayloadSpecificInfoType));
            if (mem == NULL)
            {
                PVMF_SDP_PARSER_LOGERROR((0, "SDPH264MediaInfoParser::parseMediaInfo - Memory allocation failure"));
                return SDP_NO_MEMORY;
            }
            else
            {
                H264PayloadSpecificInfoType* h264Payload = OSCL_PLACEMENT_NEW(mem, H264PayloadSpecificInfoType(payload_vec[ii]));
                (void) h264Payload;
            }
        }


        if (alt_id && !alt_def_id)
        {
            sdp->copyFmDefMedia(h264V);
            //empty alternate & default track ID vectors.
            h264V->resetAlternateTrackId();
            h264V->resetDependentTrackId();
        }

        SDP_ERROR_CODE status =
            baseMediaInfoParser(buff, h264V, index, alt_id, alt_def_id, isSipSdp);
        if (status != SDP_SUCCESS)
        {
            return status;
        }

        while (get_next_line(current_start,
                             end,
                             line_start_ptr,
                             line_end_ptr))
        {
            switch (*line_start_ptr)
            {
                case 'a':
                {
                    if ((!oscl_strncmp(line_start_ptr,
                                       "a=alt:",
                                       oscl_strlen("a=alt:"))) &&
                            (alt_def_id == false))
                    {
                        line_start_ptr += oscl_strlen("a=alt:");
                        for (; *line_start_ptr != ':'; line_start_ptr++);
                        line_start_ptr = line_start_ptr + 1;
                    }
                    if (!oscl_strncmp(line_start_ptr,
                                      "a=fmtp:",
                                      oscl_strlen("a=fmtp:")))
                    {
                        const char *tmp_start_line, *tmp_end_line;
                        fmtp_cnt++ ;

                        tmp_start_line = line_start_ptr + oscl_strlen("a=fmtp:");
                        tmp_start_line =
                            skip_whitespace(tmp_start_line, line_end_ptr);
                        if (tmp_start_line >= line_end_ptr)
                        {
                            break;
                        }
                        tmp_end_line =
                            skip_to_whitespace(tmp_start_line, line_end_ptr);
                        if (tmp_end_line < tmp_start_line)
                        {
                            break;
                        }
                        uint32 payloadNumber;
                        if (PV_atoi(tmp_start_line,
                                    'd',
                                    (tmp_end_line - tmp_start_line),
                                    payloadNumber) == false)
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPH264MediaInfoParser::parseMediaInfo - Bad a=fmtp: line format - Bad payload number"));
                            return SDP_BAD_MEDIA_FMTP;
                        }
                        else
                        {
                            int p;
                            if (!h264V->lookupPayloadNumber(payloadNumber, p))
                            {
                                fmtp_cnt--;
                                break;
                            }
                        }

                        // payloadNumber is present in the mediaInfo. get the payload
                        // Specific pointer corresponding to this payload
                        H264PayloadSpecificInfoType* payloadPtr =
                            (H264PayloadSpecificInfoType*)h264V->getPayloadSpecificInfoTypePtr(payloadNumber);
                        if (payloadPtr == NULL)
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDPH264MediaInfoParser::parseMediaInfo - Bad a=fmtp: Payload pointer not found for payload"));
                            return SDP_PAYLOAD_MISMATCH;
                        }

                        PVMF_SDP_PARSER_LOGINFO((0, "SDPH264MediaInfoParser::parseMediaInfo - processing payload number : %d", payloadNumber));

                        tmp_start_line = tmp_end_line + 1;
                        tmp_start_line =
                            skip_whitespace(tmp_start_line, line_end_ptr);
                        if (tmp_start_line >= line_end_ptr)
                        {
                            break;
                        }
                        int ii = 0;
                        const char *temp = tmp_start_line;
                        for (ii = 0; ii < (line_end_ptr - tmp_start_line); ii++)
                        {
                            if ((tmp_start_line[ii] == ';') ||
                                    (ii == (line_end_ptr - tmp_start_line) - 1))
                            {
                                tmp_end_line = tmp_start_line + ii;
                                if (ii == (line_end_ptr - tmp_start_line) - 1)
                                {
                                    tmp_end_line += 1;
                                }
                                if (!oscl_strncmp(temp,
                                                  "profile-level-id=",
                                                  oscl_strlen("profile-level-id=")))
                                {
                                    temp += oscl_strlen("profile-level-id=");
                                    temp = skip_whitespace(temp, tmp_end_line);
                                    if (temp > tmp_end_line)
                                    {
                                        PVMF_SDP_PARSER_LOGERROR((0, "SDPH264MediaInfoParser::parseMediaInfo - Bad a=fmtp: line format - Bad profile-level-id: field"));
                                        return SDP_BAD_MEDIA_FMTP;
                                    }
                                    uint32 val;
                                    if (PV_atoi(temp,
                                                'x',
                                                (tmp_end_line - temp),
                                                val) == true)
                                    {
                                        payloadPtr->setProfileLevelID(val);
                                    }

                                }
                                if (!oscl_strncmp(temp,
                                                  "max-mbps=",
                                                  oscl_strlen("max-mbps=")))
                                {
                                    temp += oscl_strlen("max-mbps=");
                                    temp = skip_whitespace(temp, tmp_end_line);
                                    if (temp > tmp_end_line)
                                    {
                                        PVMF_SDP_PARSER_LOGERROR((0, "SDPH264MediaInfoParser::parseMediaInfo - Bad a=fmtp: line format - Bad max-mbps: field"));
                                        return SDP_BAD_MEDIA_FMTP;
                                    }
                                    uint32 val;
                                    if (PV_atoi(temp,
                                                'd',
                                                (tmp_end_line - temp),
                                                val) == true)
                                    {
                                        payloadPtr->setMaxMbps(val);
                                    }
                                }
                                if (!oscl_strncmp(temp,
                                                  "max-fs=",
                                                  oscl_strlen("max-fs=")))
                                {
                                    temp += oscl_strlen("max-fs=");
                                    temp = skip_whitespace(temp, tmp_end_line);
                                    if (temp > tmp_end_line)
                                    {
                                        PVMF_SDP_PARSER_LOGERROR((0, "SDPH264MediaInfoParser::parseMediaInfo - Bad a=fmtp: line format - Bad max-fs field"));
                                        return SDP_BAD_MEDIA_FMTP;
                                    }
                                    uint32 val;
                                    if (PV_atoi(temp,
                                                'd',
                                                (tmp_end_line - temp),
                                                val) == true)
                                    {
                                        payloadPtr->setMaxFs(val);
                                    }
                                }
                                if (!oscl_strncmp(temp,
                                                  "max-cpb=",
                                                  oscl_strlen("max-cpb=")))
                                {
                                    temp += oscl_strlen("max-cpb=");
                                    temp = skip_whitespace(temp, tmp_end_line);
                                    if (temp > tmp_end_line)
                                    {
                                        PVMF_SDP_PARSER_LOGERROR((0, "SDPH264MediaInfoParser::parseMediaInfo - Bad a=fmtp: line format - Bad max-cpb: field"));
                                        return SDP_BAD_MEDIA_FMTP;
                                    }
                                    uint32 val;
                                    if (PV_atoi(temp,
                                                'd',
                                                (tmp_end_line - temp),
                                                val) == true)
                                    {
                                        payloadPtr->setMaxCpb(val);
                                    }
                                }
                                if (!oscl_strncmp(temp,
                                                  "max-dpb=",
                                                  oscl_strlen("max-dpb=")))
                                {
                                    temp += oscl_strlen("max-dpb=");
                                    temp = skip_whitespace(temp, tmp_end_line);
                                    if (temp > tmp_end_line)
                                    {
                                        PVMF_SDP_PARSER_LOGERROR((0, "SDPH264MediaInfoParser::parseMediaInfo - Bad a=fmtp: line format - Bad max-dpb field"));
                                        return SDP_BAD_MEDIA_FMTP;
                                    }
                                    uint32 val;
                                    if (PV_atoi(temp,
                                                'd',
                                                (tmp_end_line - temp),
                                                val) == true)
                                    {
                                        payloadPtr->setMaxDpb(val);
                                    }
                                }
                                if (!oscl_strncmp(temp,
                                                  "max-br=",
                                                  oscl_strlen("max-br=")))
                                {
                                    temp += oscl_strlen("max-br=");
                                    temp = skip_whitespace(temp, tmp_end_line);
                                    if (temp > tmp_end_line)
                                    {
                                        PVMF_SDP_PARSER_LOGERROR((0, "SDPH264MediaInfoParser::parseMediaInfo - Bad a=fmtp: line format - Bad max-br field"));
                                        return SDP_BAD_MEDIA_FMTP;
                                    }
                                    uint32 val;
                                    if (PV_atoi(temp,
                                                'd',
                                                (tmp_end_line - temp),
                                                val) == true)
                                    {
                                        payloadPtr->setMaxBr(val);
                                    }
                                }
                                if (!oscl_strncmp(temp,
                                                  "redundant-pic-cap=",
                                                  oscl_strlen("redundant-pic-cap=")))
                                {
                                    temp += oscl_strlen("redundant-pic-cap=");
                                    temp = skip_whitespace(temp, tmp_end_line);
                                    if (temp > tmp_end_line)
                                    {
                                        PVMF_SDP_PARSER_LOGERROR((0, "SDPH264MediaInfoParser::parseMediaInfo - Bad a=fmtp: line format - Bad redundant-pic-cap field"));
                                        return SDP_BAD_MEDIA_FMTP;
                                    }
                                    uint32 val;
                                    if (PV_atoi(temp,
                                                'd',
                                                (tmp_end_line - temp),
                                                val) == true)
                                    {
                                        payloadPtr->setRedundantPicCap(val);
                                    }
                                }
                                if (!oscl_strncmp(temp,
                                                  "sprop-parameter-sets=",
                                                  oscl_strlen("sprop-parameter-sets=")))
                                {
                                    temp += oscl_strlen("sprop-parameter-sets=");
                                    temp = skip_whitespace(temp, line_end_ptr);
                                    if (temp >= line_end_ptr)
                                    {
                                        PVMF_SDP_PARSER_LOGERROR((0, "SDPH264MediaInfoParser::parseMediaInfo - Bad a=fmtp: line format - Bad sprop-parameter-sets field"));
                                        return SDP_BAD_MEDIA_FMTP;
                                    }
                                    uint32 parameterSetLength = (int)(tmp_end_line - temp);
                                    SDP_ERROR_CODE errCode =
                                        parseParameterSets(temp,
                                                           parameterSetLength,
                                                           h264V,
                                                           payloadNumber);

                                    if (errCode != SDP_SUCCESS)
                                    {
                                        return errCode;
                                    }

                                }
                                if (!oscl_strncmp(temp,
                                                  "packetization-mode=",
                                                  oscl_strlen("packetization-mode=")))
                                {
                                    temp += oscl_strlen("packetization-mode=");
                                    temp = skip_whitespace(temp, tmp_end_line);
                                    if (temp > tmp_end_line)
                                    {
                                        PVMF_SDP_PARSER_LOGERROR((0, "SDPH264MediaInfoParser::parseMediaInfo - Bad a=fmtp: line format - Bad packetization-mode field"));
                                        return SDP_BAD_MEDIA_FMTP;
                                    }
                                    uint32 val;
                                    if (PV_atoi(temp,
                                                'd',
                                                (tmp_end_line - temp),
                                                val) == true)
                                    {
                                        payloadPtr->setPacketizationMode(val);
                                    }
                                }
                                if (!oscl_strncmp(temp,
                                                  "sprop-interleaving-depth=",
                                                  oscl_strlen("sprop-interleaving-depth=")))
                                {
                                    temp += oscl_strlen("sprop-interleaving-depth=");
                                    temp = skip_whitespace(temp, tmp_end_line);
                                    if (temp > tmp_end_line)
                                    {
                                        PVMF_SDP_PARSER_LOGERROR((0, "SDPH264MediaInfoParser::parseMediaInfo - Bad a=fmtp: line format - Bad sprop-interleaving-depth field"));
                                        return SDP_BAD_MEDIA_FMTP;
                                    }
                                    uint32 val;
                                    if (PV_atoi(temp,
                                                'd',
                                                (tmp_end_line - temp),
                                                val) == true)
                                    {
                                        payloadPtr->setSpropInterleavingDepth(val);
                                    }
                                }
                                if (!oscl_strncmp(temp,
                                                  "deint-buf-cap=",
                                                  oscl_strlen("deint-buf-cap=")))
                                {
                                    temp += oscl_strlen("deint-buf-cap=");
                                    temp = skip_whitespace(temp, tmp_end_line);
                                    if (temp > tmp_end_line)
                                    {
                                        PVMF_SDP_PARSER_LOGERROR((0, "SDPH264MediaInfoParser::parseMediaInfo - Bad a=fmtp: line format - Bad deint-buf-cap field"));
                                        return SDP_BAD_MEDIA_FMTP;
                                    }
                                    uint32 val;
                                    if (PV_atoi(temp,
                                                'd',
                                                (tmp_end_line - temp),
                                                val) == true)
                                    {
                                        payloadPtr->setDeintBufCap(val);
                                    }
                                }
                                if (!oscl_strncmp(temp,
                                                  "sprop-deint-buf-req=",
                                                  oscl_strlen("sprop-deint-buf-req=")))
                                {
                                    temp += oscl_strlen("sprop-deint-buf-req=");
                                    temp = skip_whitespace(temp, tmp_end_line);
                                    if (temp > tmp_end_line)
                                    {
                                        PVMF_SDP_PARSER_LOGERROR((0, "SDPH264MediaInfoParser::parseMediaInfo - Bad a=fmtp: line format - Bad sprop-deint-buf-req field"));
                                        return SDP_BAD_MEDIA_FMTP;
                                    }
                                    uint32 val;
                                    if (PV_atoi(temp,
                                                'd',
                                                (tmp_end_line - temp),
                                                val) == true)
                                    {
                                        payloadPtr->setSpropDeintBufReq(val);
                                    }
                                }
                                if (!oscl_strncmp(temp,
                                                  "sprop-init-buf-time=",
                                                  oscl_strlen("sprop-init-buf-time=")))
                                {
                                    temp += oscl_strlen("sprop-init-buf-time=");
                                    temp = skip_whitespace(temp, tmp_end_line);
                                    if (temp > tmp_end_line)
                                    {
                                        PVMF_SDP_PARSER_LOGERROR((0, "SDPH264MediaInfoParser::parseMediaInfo - Bad a=fmtp: line format - Bad sprop-init-buf-time field"));
                                        return SDP_BAD_MEDIA_FMTP;
                                    }
                                    uint32 val;
                                    if (PV_atoi(temp,
                                                'd',
                                                (tmp_end_line - temp),
                                                val) == true)
                                    {
                                        payloadPtr->setSpropInitBufTime(val);
                                    }
                                }
                                if (!oscl_strncmp(temp,
                                                  "sprop-max-don-diff=",
                                                  oscl_strlen("sprop-max-don-diff=")))
                                {
                                    temp += oscl_strlen("sprop-max-don-diff=");
                                    temp = skip_whitespace(temp, tmp_end_line);
                                    if (temp > tmp_end_line)
                                    {
                                        PVMF_SDP_PARSER_LOGERROR((0, "SDPH264MediaInfoParser::parseMediaInfo - Bad a=fmtp: line format - Bad sprop-max-don-diff field"));
                                        return SDP_BAD_MEDIA_FMTP;
                                    }
                                    uint32 val;
                                    if (PV_atoi(temp,
                                                'd',
                                                (tmp_end_line - temp),
                                                val) == true)
                                    {
                                        payloadPtr->setSpropMaxDonDiff(val);
                                    }
                                }
                                if (!oscl_strncmp(temp,
                                                  "max-rcmd-nalu-size=",
                                                  oscl_strlen("max-rcmd-nalu-size=")))
                                {
                                    temp += oscl_strlen("max-rcmd-nalu-size=");
                                    temp = skip_whitespace(temp, tmp_end_line);
                                    if (temp > tmp_end_line)
                                    {
                                        PVMF_SDP_PARSER_LOGERROR((0, "SDPH264MediaInfoParser::parseMediaInfo - Bad a=fmtp: line format - Bad max-rcmd-nalu-size field"));
                                        return SDP_BAD_MEDIA_FMTP;
                                    }
                                    uint32 val;
                                    if (PV_atoi(temp,
                                                'd',
                                                (tmp_end_line - temp),
                                                val) == true)
                                    {
                                        payloadPtr->setMaxRcmdNaluSize(val);
                                    }
                                }
                                if (!oscl_strncmp(temp,
                                                  "decode_buf=",
                                                  oscl_strlen("decode_buf=")))
                                {
                                    temp += oscl_strlen("decode_buf=");
                                    temp = skip_whitespace(temp, tmp_end_line);
                                    if (temp > tmp_end_line)
                                    {
                                        PVMF_SDP_PARSER_LOGERROR((0, "SDPH264MediaInfoParser::parseMediaInfo - Bad a=fmtp: line format - Bad decode-buf field"));
                                        return SDP_BAD_MEDIA_FMTP;
                                    }
                                    uint32 decode_buf;
                                    if (PV_atoi(temp,
                                                'd',
                                                (tmp_end_line - temp),
                                                decode_buf) == true)
                                    {
                                        payloadPtr->setMaxBufferSize(decode_buf);
                                    }
                                }
                                if (tmp_end_line != line_end_ptr)
                                {
                                    temp = tmp_end_line + 1;
                                }
                                temp = skip_whitespace(temp, line_end_ptr);
                                if (temp >= line_end_ptr)
                                {
                                    PVMF_SDP_PARSER_LOGERROR((0, "SDPH264MediaInfoParser::parseMediaInfo - Bad a=fmtp: line format"));
                                    return SDP_BAD_MEDIA_FMTP;
                                }
                            }
                        }
                    }// end a=fmtp
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
            status = setDependentMediaId(altGroupBW, length, h264V, alt_id);
            if (status != SDP_SUCCESS)
                return SDP_BAD_MEDIA_ALT_ID;
        }

        const char *altGroupLANG = session->getAltGroupLANG();
        length = session->getAltGroupLANGLength();

        if (length > 0)
        {
            status = setDependentMediaId(altGroupLANG, length, h264V, alt_id);
            if (status != SDP_SUCCESS)
                return SDP_BAD_MEDIA_ALT_ID;
        }

        if ((fmtp_cnt == h264V->getMediaPayloadNumberCount()) &&
                (h264V->getCFieldStatus() || session->getCFieldStatus()))

            return SDP_SUCCESS;
        else
        {
            PVMF_SDP_PARSER_LOGERROR((0, "SDPH264MediaInfoParser::parseMediaInfo - no c field present"));
            return SDP_FAILURE_NO_C_FIELD;
        }

    }


}

SDP_ERROR_CODE SDPH264MediaInfoParser::parseParameterSets(const char* aParamSetBuf,
        int   aParamSetBufLen,
        h264_mediaInfo* aH264MediaInfo,
        uint32 aPayLoadNumber)
{
    /*Determine number of parameter sets */
    const char *tmp_start_line = aParamSetBuf;
    int i;
    uint32 numParamSets = 0;
    Oscl_Vector<uint8*, OsclMemAllocator> paramSetStartBufVec;
    Oscl_Vector<uint32, OsclMemAllocator> paramSetBufSizeVec;
    paramSetStartBufVec.push_back((uint8*)tmp_start_line);
    uint32 prevIndex = 0;
    for (i = 0; i < aParamSetBufLen; i++)
    {
        if (tmp_start_line[i] == ',')
        {
            uint8* ptr = (uint8*)(tmp_start_line + (i + 1));
            paramSetStartBufVec.push_back(ptr);
            uint32 size = (i - prevIndex);
            prevIndex = i + 1;
            paramSetBufSizeVec.push_back(size);
            numParamSets++;
        }
    }
    numParamSets++;
    paramSetBufSizeVec.push_back((aParamSetBufLen - prevIndex));

    if (paramSetBufSizeVec[0] >= MIN_ENCODED_BYTES)
    {
        uint8  decoded[MIN_ENCODED_BYTES];
        uint32 outBufLen = 0;
        //Decode the minimum amount required to extract the NAL type.
        if (sdp_decodebase64(paramSetStartBufVec[0],
                             MIN_ENCODED_BYTES,
                             decoded,
                             outBufLen,
                             sizeof(decoded)
                            ))
        {
            uint8 nal_type = decoded[0] & AVC_NALTYPE_MASK;
            switch (nal_type)
            {
                    //Rather than modify every codec to accept SPS and PPS in any order,
                    //it is best to simply swap the order in the SDP parser so SPS
                    //always comes before PPS.
                    //If PPS comes before SPS, swap the order.
                case AVC_NALTYPE_PPS:
                {
                    uint8* tempBuf = paramSetStartBufVec.back();
                    paramSetStartBufVec.pop_back();
                    paramSetStartBufVec.push_front(tempBuf);
                    uint32 tempSize = paramSetBufSizeVec.back();
                    paramSetBufSizeVec.pop_back();
                    paramSetBufSizeVec.push_front(tempSize);
                }
                break;

                case AVC_NALTYPE_SPS:
                default:
                    break;
            }
        }
    }

    int configBufLength = aParamSetBufLen + numParamSets * (sizeof(uint16));
    SDPAllocDestructDealloc<uint8> SDP_alloc;
    uint8 *configPtr = SDP_alloc.allocate(configBufLength);
    OsclRefCounterSA< SDPAllocDestructDealloc<uint8> > *refcnt =
        new OsclRefCounterSA< SDPAllocDestructDealloc<uint8> >(configPtr);
    OsclSharedPtr<uint8> parameterSetPtr(configPtr, refcnt);

    uint32 offset = 0;
    for (uint32 j = 0; j < paramSetStartBufVec.size(); j++)
    {
        uint8* inBuf = paramSetStartBufVec[j];
        uint32 inBufLen = paramSetBufSizeVec[j];
        uint32 outBufLen = 0;
        uint8* paramSetSizePtr = configPtr + offset;
        offset += 2; // for param set size
        uint8* outPtr = configPtr + offset;
        uint32 maxOutBufLen = configBufLength - offset;
        if (!sdp_decodebase64(inBuf,
                              inBufLen,
                              outPtr,
                              outBufLen,
                              maxOutBufLen))
        {
            PVMF_SDP_PARSER_LOGERROR((0, "SDPH264MediaInfoParser::parseParameterSets : unable to set decodebase"));
            return SDP_BAD_MEDIA_FMTP;
        }
        uint16 len = (uint16)outBufLen;
        oscl_memcpy(paramSetSizePtr, &len, sizeof(uint16));
        offset += outBufLen;
    }

    H264PayloadSpecificInfoType* payloadPtr =
        (H264PayloadSpecificInfoType*)aH264MediaInfo->getPayloadSpecificInfoTypePtr(aPayLoadNumber);
    if (payloadPtr == NULL)
    {
        PVMF_SDP_PARSER_LOGERROR((0, "SDPH264MediaInfoParser::parseParameterSets - Unable to find payload ptr for payload"));
        return SDP_PAYLOAD_MISMATCH;
    }

    PVMF_SDP_PARSER_LOGINFO((0, "SDPH264MediaInfoParser::parseParameterSets - processing payload number : %d", aPayLoadNumber));

    payloadPtr->setDecoderSpecificInfo(parameterSetPtr);
    payloadPtr->setDecoderSpecificInfoSize(offset);

    //if sample rate is zero override with defaults
    Oscl_Vector<PayloadSpecificInfoTypeBase*, SDPParserAlloc> payloadSpecificInfoVector =
        aH264MediaInfo->getPayloadSpecificInfoVector();
    for (int ii = 0; ii < (int)payloadSpecificInfoVector.size();ii++)
    {
        if (payloadSpecificInfoVector[ii]->getSampleRate() == 0)
        {
            payloadSpecificInfoVector[ii]->sampleRate =
                PVMF_SDP_DEFAULT_H264_SAMPLE_RATE;
        }
    }
    return SDP_SUCCESS;
}


