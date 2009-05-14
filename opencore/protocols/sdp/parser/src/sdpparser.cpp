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
#include "sdp_parser.h"
#include "sdp_mediaparser_registry.h"
#include "oscl_string_utils.h"
#include "oscl_string_containers.h"
#include "oscl_str_ptr_len.h"
#include "base_media_info_parser.h"
#include "aac_media_info_parser.h"
#include "amr_media_info_parser.h"
#include "h263_media_info_parser.h"
#include "m4v_media_info_parser.h"
#include "still_image_media_info_parser.h"
#include "pcma_media_info_parser.h"
#include "pcmu_media_info_parser.h"
#include "oscl_vector.h"
#include "oscl_dll.h"

OSCL_DLL_ENTRY_POINT_DEFAULT()
struct mime_payload_pair
{
    OsclMemoryFragment mime;
    Oscl_Vector<int, SDPParserAlloc> payload_no;
};

OSCL_EXPORT_REF SDP_Parser::SDP_Parser(SDPMediaParserRegistry*& regTable, bool sipSdp):
        iLogger(NULL),
        _pSDPMediaParserRegistry(regTable),
        mediaArrayIndex(0),
        applicationFlag(false),
        isSipSdp(sipSdp)
{
    iLogger = PVLogger::GetLoggerObject("SDP_Parser");
}


OSCL_EXPORT_REF SDP_Parser::~SDP_Parser()
{
}

bool SDP_Parser::parse_rtpmap(const char *start, const char *end, int& rtp_payload,
                              OsclMemoryFragment& encoding_name)
{
    const int len_of_rtpmap = 9;

    // grab the endpoints
    const char *sptr = start + len_of_rtpmap;
    const char *eptr;

    // skip to the first whitespace character
    eptr = skip_to_whitespace(sptr, end);
    if (eptr < sptr)
    {
        return false;
    }
    uint32 rtpPayload;
    if (PV_atoi(sptr, 'd', (eptr - sptr), rtpPayload) == false)
    {
        return false;
    }
    rtp_payload = (int)rtpPayload;

    // now get the encoding name
    sptr = skip_whitespace(eptr, end);
    if (sptr >= end)
    {
        return false;
    }

    // now skip to end of the encoding name
    for (eptr = sptr; eptr < end &&
            (*eptr != ' ' && *eptr != '\t' && *eptr != '/');
            ++eptr);

    if (eptr >= end)
    {
        return false;
    }

    encoding_name.ptr = (void *) sptr;
    encoding_name.len = eptr - sptr;

    return true;
}


int SDP_Parser::validate_media_line(const char *start, const char *end, Oscl_Vector<int, SDPParserAlloc>& payload_type, uint32& portNumber)
{
    int len;
    const char *sptr, *eptr;

    sptr = start + 2;  // start after the "m="
    // skip to end of media type
    eptr = skip_to_whitespace(sptr, end);
    if (eptr >= end)
    {
        return 0;
    }


    len = eptr - sptr;
    // make sure type is supported
    if (!oscl_CIstrncmp(sptr, "audio", len) || !oscl_CIstrncmp(sptr, "video", len) ||
            !oscl_CIstrncmp(sptr, "application", len))
    {
        // the type is supported
        // make sure there is only one payload type in the format list

        // skip to start of port number
        sptr = skip_whitespace(eptr, end);
        if (sptr >= end)
        {
            return 0;
        }

        // skip to end of port number
        eptr = skip_to_whitespace(sptr, end);
        if (eptr <= sptr)
        {
            return 0;
        }

        const char *tmp_end_ptr = sptr;
        const char SDP_FWD_SLASH[] = "/";

        OSCL_HeapString<SDPParserAlloc> restOfLine(tmp_end_ptr, eptr - tmp_end_ptr);
        const char *slash = oscl_strstr(restOfLine.get_cstr(), SDP_FWD_SLASH);

        if (slash == NULL)
        {
            // Get the port number
            if (PV_atoi(sptr, 'd', (eptr - sptr), portNumber) == false)
            {
                return 0;
            }
        }
        else
        {
            // Get the port number
            if (PV_atoi(restOfLine.get_cstr(), 'd', (slash - restOfLine.get_cstr()), portNumber) == false)
            {
                return 0;
            }
        }

        // skip to start of transport
        sptr = skip_whitespace(eptr, end);
        if (sptr >= end)
        {
            return 0;
        }

        // skip to end of transport
        eptr = skip_to_whitespace(sptr, end);
        if (eptr <= sptr)
        {
            return 0;
        }

        // skip to start of format list
        sptr = skip_whitespace(eptr, end);
        if (sptr >= end)
        {
            return 0;
        }

        // skip to end of first payload arg
        eptr = skip_to_whitespace(sptr, end);
        if (eptr <= sptr)
        {
            return 0;
        }

        // record the payload type for non-application m= lines
        if (oscl_strncmp(start + 2, "application", len))
        {
            uint32 payloadType;

            while (sptr < end)
            {
                if (PV_atoi(sptr, 'd', (eptr - sptr), payloadType) == false)
                {
                    return 0;
                }

                payload_type.push_back(payloadType);

                sptr = skip_to_whitespace(sptr, end);
                sptr = skip_whitespace_and_line_term(eptr, end);
                eptr = skip_whitespace_and_line_term(eptr, end);
                eptr = skip_to_whitespace(eptr, end);
            }
        }
        else
        {
            uint32 len = OSCL_MIN((uint32)(eptr - start), oscl_strlen("IMAGE"));
            if (!oscl_strncmp(start, "IMAGE", len))
            {
                applicationFlag = true;
            }
            else	//don't support this media. so skip the section
                return 0;
        }

        if (sptr < end)
        {
            return 0;
        }

        return 1;
    }

    return 0;

}

OSCL_EXPORT_REF
SDP_ERROR_CODE SDP_Parser::parseSDP(const char *sdpText, int text_length, SDPInfo *sdp)
{
    int index = 0, sdpIndex = 0;

    const char *end_ptr = sdpText + text_length ; // Point just beyond the end
    const char *section_start_ptr;
    const char *section_end_ptr;
    const char *line_start_ptr, *line_end_ptr;
    bool session_info_parsed = false;

    /**************************************************************************/

    // The purpose of this outer loop is to partition the SDP into different
    // sections to be passed off to session-level parsers or media-level parsers.
    // We just need to find the boundaries and pass the appropriate sections
    // of code to the subparsers.

    sdpIndex = 0;
    index = 0;

    // these track whether media and session-level sections have already
    // been found
    int media_sections_found = 0;
    int session_section_found = 0;

    // skip any leading whitespace including line terminators
    section_start_ptr = skip_whitespace_and_line_term(sdpText, end_ptr);

    while ((section_start_ptr - sdpText) < text_length)
    {
        if (!get_next_line(section_start_ptr, end_ptr,
                           line_start_ptr, line_end_ptr))
        {
            break;
        }

        // figure out the type of section
        if (!oscl_strncmp(line_start_ptr, "v=", 2))
        {
            // this is the session-level
            if (media_sections_found || session_section_found)
            {
                // there were already media sections or already a session-level section
                // so a session-level section at this point is not allowed.
                PVMF_SDP_PARSER_LOGERROR((0, "SDP_Parser::parseSDP - Duplicate Session Sections"));
                return SDP_BAD_FORMAT;
            }

            section_end_ptr = line_end_ptr;

            // record that the session-level section has been found
            session_section_found = 1;

            while (get_next_line(section_end_ptr, end_ptr,
                                 line_start_ptr, line_end_ptr))
            {
                // check if this is the start of another section
                if (!oscl_strncmp(line_start_ptr, "v=", 2) ||
                        !oscl_strncmp(line_start_ptr, "m=", 2))
                {
                    break;
                }
                section_end_ptr = line_end_ptr;
            }


            OsclMemoryFragment session_frag;
            session_frag.ptr = (void *)section_start_ptr;
            session_frag.len = section_end_ptr - section_start_ptr;

            SDP_ERROR_CODE retval =
                parseSDPSessionInfo(section_start_ptr,
                                    section_end_ptr - section_start_ptr,
                                    sdp);
            if (retval != SDP_SUCCESS)
            {
                PVMF_SDP_PARSER_LOGERROR((0, "SDP_Parser::parseSDP - parseSDPSessionInfo Failed=%d", retval));
                return retval;
            }
            else
            {
                session_info_parsed = true;
            }
        }
        else if (!oscl_strncmp(line_start_ptr, "m=", 2))
        {

            // now look for the end of the section
            section_end_ptr = line_end_ptr;
            bool supported_media = true;

            ++media_sections_found;
            sdp->setSegmentCount(media_sections_found);

            /* SUPPORTING MULTIPLE PAYLOAD TYPE PER MEDIA NOW */
            // check to see how many payload types are present
            Oscl_Vector<int, SDPParserAlloc> payload_type;
            Oscl_Vector<int, SDPParserAlloc> rtpmap_pt;
            Oscl_Vector<OsclMemoryFragment, SDPParserAlloc> encoding_name_vector;

            uint32 portNumber = 0;
            if (!validate_media_line(line_start_ptr, line_end_ptr, payload_type, portNumber))
            {
                // skip this section
                supported_media = false;
            }
            else
            {
                PVMF_SDP_PARSER_LOGINFO((0, "SDP_Parser::parseSDP - Validated MediaSection"));
            }

            int rtpmap_lines = 0;

            // get the next line
            OsclMemoryFragment encoding_name;
            encoding_name.ptr = NULL;
            encoding_name.len = 0;

            Oscl_Vector<int, SDPParserAlloc> AltId;
            while (get_next_line(section_end_ptr, end_ptr, line_start_ptr, line_end_ptr))
            {
                // check if this is the start of another section
                if (!oscl_strncmp(line_start_ptr, "v=", 2) ||
                        !oscl_strncmp(line_start_ptr, "m=", 2))
                {
                    break;
                }
                if (supported_media && (applicationFlag == false))
                {
                    // check for lines which will give the media type
                    // so the parser can be allocated. Simply look for
                    // the "a=rtpmap" lines which contain the MIME type.
                    StrPtrLen rtpmap_str("a=rtpmap:");
                    if (!oscl_strncmp(line_start_ptr, rtpmap_str.c_str(), rtpmap_str.length()))
                    {
                        ++rtpmap_lines;
                        int rtpmap_cu;
                        // get encoding name
                        if (!parse_rtpmap(line_start_ptr, line_end_ptr, rtpmap_cu, encoding_name))
                        {
                            // invalid format
                            PVMF_SDP_PARSER_LOGERROR((0, "SDP_Parser::parseSDP - parse_rtpmap Failed"));
                            return SDP_BAD_MEDIA_FORMAT;
                        }
                        rtpmap_pt.push_back(rtpmap_cu);
                        encoding_name_vector.push_back(encoding_name);
                        OSCL_StackString<15> mime((const char*)(encoding_name.ptr), encoding_name.len);
                        PVMF_SDP_PARSER_LOGINFO((0, "SDP_Parser::parseSDP - a=rtpmap mime=%s", mime.get_cstr()));
                    }
                    StrPtrLen alt_def("a=alt-default-id:");
                    if (!oscl_strncmp(line_start_ptr, alt_def.c_str(), alt_def.length()))
                    {
                        uint32 id;
                        const char *sptr = line_start_ptr + alt_def.length();
                        sptr = skip_whitespace(sptr, line_end_ptr);
                        if (!PV_atoi(sptr, 'd', line_end_ptr - sptr, id))
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDP_Parser::parseSDP - Parsing a=alt-default-id: Failed"));
                            return SDP_BAD_MEDIA_ALT_ID;
                        }
                        AltId.push_back(id);
                    }
                    StrPtrLen alt_id("a=alt:");
                    if (!oscl_strncmp(line_start_ptr, alt_id.c_str(), alt_id.length()))
                    {
                        uint32 id;
                        const char *sptr = line_start_ptr + alt_id.length();
                        sptr = skip_whitespace(sptr, line_end_ptr);
                        const char *eptr = sptr;
                        for (; *eptr != ':'; eptr++);
                        if (!PV_atoi(sptr, 'd', eptr - sptr, id))
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDP_Parser::parseSDP - Parsing a=alt: Failed"));
                            return SDP_BAD_MEDIA_ALT_ID;
                        }
                        if (AltId.back() != (int)id)
                            AltId.push_back(id);
                    }

                    // fmtp and framesize payload check is provided below
                    // this is done to make sure that the payload coming in these fields
                    // is one of the payloads in the m= segment
                    StrPtrLen fmtp("a=fmtp:");
                    if (!oscl_strncmp(line_start_ptr, fmtp.c_str(), fmtp.length()))
                    {
                        uint32 payload;
                        const char *sptr = line_start_ptr + fmtp.length();
                        sptr = skip_whitespace(sptr, line_end_ptr);
                        const char* eptr = skip_to_whitespace(sptr, line_end_ptr);
                        if (!PV_atoi(sptr, 'd', eptr - sptr, payload))
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDP_Parser::parseSDP - Parsing a=fmtp: Failed"));
                            return SDP_BAD_MEDIA_FORMAT;
                        }
                        // The format is proper match the payload with payloads in m= segment
                        bool matched = false;
                        for (uint32 ii = 0; ii < payload_type.size(); ii++)
                        {
                            if (payload == (uint32)payload_type[ii])
                            {
                                matched = true;
                                break;
                            }
                        }
                        if (!matched)
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDP_Parser::parseSDP - Payload Mismatch in fmtp line"));
                            return SDP_PAYLOAD_MISMATCH;
                        }
                    }
                    StrPtrLen framesize("a=framesize:");
                    if (!oscl_strncmp(line_start_ptr, framesize.c_str(), framesize.length()))
                    {
                        uint32 payload;
                        const char *sptr = line_start_ptr + framesize.length();
                        sptr = skip_whitespace(sptr, line_end_ptr);
                        const char* eptr = skip_to_whitespace(sptr, line_end_ptr);
                        if (!PV_atoi(sptr, 'd', eptr - sptr, payload))
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDP_Parser::parseSDP - Parsing a=framesize: Failed"));
                            return SDP_BAD_MEDIA_FORMAT;
                        }
                        // The format is proper match the payload with payloads in m= segment
                        bool matched = false;
                        for (uint32 ii = 0; ii < payload_type.size(); ii++)
                        {
                            if (payload == (uint32)payload_type[ii])
                            {
                                matched = true;
                                break;
                            }
                        }
                        if (!matched)
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDP_Parser::parseSDP - Payload Mismatch in a=framesize: line"));
                            return SDP_PAYLOAD_MISMATCH;
                        }
                    }

                } // end if media is supported
                else
                {
                    PVMF_SDP_PARSER_LOGINFO((0, "SDP_Parser::parseSDP - Skipping over an entire m= section"));
                }

                section_end_ptr = line_end_ptr;
            } // end loop over the entire media section


            // The checking for rtpmap vs payloads is not required if the port number is 0 in case it is a sip sdp
            bool check_for_rtpmap = true;
            if (isSipSdp && portNumber == 0)
            {
                check_for_rtpmap = false;
            }

            // Checking for rtpmap with the payloads
            if (supported_media && check_for_rtpmap)
            {
                // Validate the payload type and rtpmap if required
                int static_payload_count = 0;
                int ii = 0;
                for (; ii < (int)payload_type.size(); ii++)
                {
                    // If any payload_type is in static range we do not care
                    // for the rtpmap field. We will process for this static payload type sdp
                    // the dynamic payload type if any will be ignored if it's rtpmap
                    // is missing
                    if ((payload_type[ii] >= FIRST_STATIC_PAYLOAD) &&
                            (payload_type[ii] <= LAST_STATIC_PAYLOAD))
                    {
                        PVMF_SDP_PARSER_LOGINFO((0, "SDP_Parser::parseSDP - Static Payload =%d", payload_type[ii]));
                        static_payload_count++;
                    }
                }
                if (static_payload_count == 0)
                {
                    // The payload type present are all in the dynamic range
                    if (rtpmap_pt.size() != payload_type.size())
                    {
                        PVMF_SDP_PARSER_LOGERROR((0, "SDP_Parser::parseSDP - Mismatch between number of payloads and rtpmap lines"));
                        return SDP_BAD_MEDIA_FORMAT;
                    }
                    for (int ii = 0; ii < (int)rtpmap_pt.size(); ii++)
                    {
                        if (rtpmap_pt[ii] != payload_type[ii])
                        {
                            // this is an error
                            PVMF_SDP_PARSER_LOGERROR((0, "SDP_Parser::parseSDP - Incorrect payload number  in rtpmap line"));
                            return SDP_PAYLOAD_MISMATCH;
                        }
                    }
                }
                else if (static_payload_count >= 1)
                {
                    // All of the payloads can either be static or one of them
                    // for every dynamic payload there should be matching rtpmap field
                    if (rtpmap_pt.size() != (payload_type.size() - static_payload_count))
                    {
                        PVMF_SDP_PARSER_LOGERROR((0, "SDP_Parser::parseSDP - Mismatch between number of payloads and rtpmap lines"));
                        return SDP_BAD_MEDIA_FORMAT;
                    }
                    for (uint32 rtpmap_count = 0; rtpmap_count < rtpmap_pt.size(); rtpmap_count++)
                    {
                        bool match_found = false;
                        for (int jj = 0; jj < (int)payload_type.size(); jj++)
                        {
                            if (rtpmap_pt[rtpmap_count] == payload_type[jj])
                            {
                                match_found = true;
                                break;
                            }
                        }
                        if (match_found == false)
                        {
                            PVMF_SDP_PARSER_LOGERROR((0, "SDP_Parser::parseSDP - No matching rtpmap line"));
                            // this is an error
                            return SDP_PAYLOAD_MISMATCH;
                        }
                    }
                }
            }

            if (session_info_parsed == false)
            {
                PVMF_SDP_PARSER_LOGERROR((0, "SDP_Parser::parseSDP - Missing Session Section"));
                return SDP_BAD_FORMAT;
            }

            if (supported_media)
            {
                StrPtrLen image("X-MP4V-IMAGE");
                if (applicationFlag == true)
                {
                    if (rtpmap_lines == 0)
                    {
                        rtpmap_lines++;
                        encoding_name.ptr = (void *) image.c_str();
                        encoding_name.len = image.length();
                        applicationFlag = false;
                        encoding_name_vector.push_back(encoding_name);
                    }
                }

                // Compose all media encoding names and put them in one vector
                // this will carry all encoding names including static and dynamic PT
                StrPtrLen pcma(PVMF_MIME_PCMA);
                StrPtrLen pcmu(PVMF_MIME_PCMU);
                StrPtrLen amr("AMR");
                Oscl_Vector<OsclMemoryFragment, SDPParserAlloc> all_media_encoding_names;
                uint32 ii = 0;
                uint32 jj = 0;
                for (; ii < payload_type.size(); ii++)
                {
                    if (payload_type[ii] == PVMF_PCMU)
                    {
                        rtpmap_lines++;
                        encoding_name.ptr = (void *) pcmu.c_str();
                        encoding_name.len = pcmu.length();
                    }
                    else if (payload_type[ii] == PVMF_PCMA)
                    {
                        rtpmap_lines++;
                        encoding_name.ptr = (void *) pcma.c_str();
                        encoding_name.len = pcma.length();
                    }
                    else if (check_for_rtpmap == false)
                    {
                        // This means that the port is 0 and they payload is in dynamic range
                        // rtpmap field is not present.
                        // To map it internally let's put it under PCMU again
                        rtpmap_lines++;
                        encoding_name.ptr = (void *) pcmu.c_str();
                        encoding_name.len = pcmu.length();
                    }
                    else
                    {
                        // All other payload encoding names are already present in the
                        // encoding_name_vector.
                        if (jj < encoding_name_vector.size())
                        {
                            encoding_name = encoding_name_vector[jj];
                            jj++;
                        }
                    }
                    all_media_encoding_names.push_back(encoding_name);
                }

                // compose all the above information in the following format
                // Any Mime type coming repeatedly with different payload numbers
                // Then club them together
                Oscl_Vector<mime_payload_pair, SDPParserAlloc> mime_payload_pair_vector;

                for (uint32 ll = 0; ll < payload_type.size(); ll++)
                {
                    if (check_for_rtpmap == true)
                    {
                        bool matched = false;
                        uint32 ii = 0;
                        for (; ii < mime_payload_pair_vector.size(); ii++)
                        {
                            if (oscl_strncmp((char*)mime_payload_pair_vector[ii].mime.ptr,
                                             (char*)all_media_encoding_names[ll].ptr,
                                             all_media_encoding_names[ll].len) == 0)
                            {
                                matched = true;
                                break;
                            }
                        }
                        if (matched)
                        {
                            mime_payload_pair_vector[ii].payload_no.push_back(payload_type[ll]);
                        }
                        else
                        {
                            mime_payload_pair mpp;
                            mpp.payload_no.push_back(payload_type[ll]);
                            mpp.mime.len = all_media_encoding_names[ll].len;
                            mpp.mime.ptr = all_media_encoding_names[ll].ptr;
                            mime_payload_pair_vector.push_back(mpp);
                        }
                    }
                    else
                    {
                        // It is a sip sdp with port = 0. Hence do not make specific checks
                        mime_payload_pair mpp;
                        mpp.payload_no.push_back(payload_type[ll]);
                        mpp.mime.len = all_media_encoding_names[ll].len;
                        mpp.mime.ptr = all_media_encoding_names[ll].ptr;
                        mime_payload_pair_vector.push_back(mpp);
                    }

                }

                if (rtpmap_lines >= 1)
                {
                    SDPMediaParserFactory  *mediaParserFactory;
                    SDPBaseMediaInfoParser *mediaParser;
                    SDP_ERROR_CODE retval;

                    for (uint32 kk = 0; kk < mime_payload_pair_vector.size(); kk++)
                    {
                        encoding_name = mime_payload_pair_vector[kk].mime;
                        if ((mediaParserFactory =
                                    _pSDPMediaParserRegistry->lookupSDPMediaParserFactory(encoding_name)) != NULL)
                        {
                            mediaParser = mediaParserFactory->createSDPMediaParserInstance();
                            if (mediaParser == NULL)
                            {
                                PVMF_SDP_PARSER_LOGERROR((0, "SDP_Parser::parseSDP - Unable to create media parser"));
                                return SDP_FAILURE;
                            }
                            if (AltId.size() > 0)
                            {
                                int alt_id;
                                bool alt_def_id = false;
                                for (int ss = 0; ss < (int)AltId.size(); ss++)
                                {
                                    if (ss == 0) alt_def_id = true;
                                    else alt_def_id = false;

                                    alt_id = AltId[ss];

                                    if ((retval = mediaParser->parseMediaInfo(section_start_ptr, section_end_ptr - section_start_ptr, sdp, mime_payload_pair_vector[kk].payload_no, isSipSdp, alt_id, alt_def_id)) != SDP_SUCCESS)
                                    {
                                        OSCL_StackString<8> mime((const char*)(encoding_name.ptr), encoding_name.len);
                                        PVMF_SDP_PARSER_LOGERROR((0, "SDP_Parser::parseSDP - Parsing m= section failed, mime=%s", mime.get_cstr()));
                                        OSCL_DELETE((mediaParser));
                                        sdp->freeLastMediaInfoObject();
                                        return retval;
                                    }

                                }
                            }
                            else
                            {
                                if ((retval = mediaParser->parseMediaInfo(section_start_ptr, section_end_ptr - section_start_ptr, sdp, mime_payload_pair_vector[kk].payload_no, isSipSdp)) != SDP_SUCCESS)
                                {
                                    OSCL_StackString<8> mime((const char*)(encoding_name.ptr), encoding_name.len);
                                    PVMF_SDP_PARSER_LOGERROR((0, "SDP_Parser::parseSDP - Parsing m= section failed, mime=%s", mime.get_cstr()));
                                    OSCL_DELETE((mediaParser));
                                    sdp->freeLastMediaInfoObject();
                                    return retval;
                                }
                            }
                            sdp->IncrementAlternateMediaInfoVectorIndex();
                            OSCL_DELETE((mediaParser));
                        }
                        mediaParser = NULL;
                    }	// End of for
                }
                if (rtpmap_lines == 0) // no rtpmap found in media
                {
                    PVMF_SDP_PARSER_LOGERROR((0, "SDP_Parser::parseSDP - Parsing m= section failed, No rtpmap line"));
                    return SDP_BAD_MEDIA_MISSING_RTPMAP;
                }
            }
        } // end this is a media section
        else
        {
            PVMF_SDP_PARSER_LOGERROR((0, "SDP_Parser::parseSDP - Unrecognized Syntax"));
            // unknown section type -- this is an error
            return SDP_FAILURE;
        }

        section_start_ptr = skip_whitespace_and_line_term(section_end_ptr, end_ptr);

    }
    {
        //for SDP which doesn't have session level range, set the session level range
        //to be the MAX of media ranges.
        if (NULL == sdp->getSessionInfo())
        {
            PVMF_SDP_PARSER_LOGERROR((0, "SDP_Parser::parseSDP - Missing Session Info"));
            return SDP_BAD_FORMAT;
        }
        RtspRangeType *mySdpRange = ((RtspRangeType *)sdp->getSessionInfo()->getRange());
        if (NULL == mySdpRange)
        {
            PVMF_SDP_PARSER_LOGERROR((0, "SDP_Parser::parseSDP - Unable to retrieve session range"));
            return SDP_BAD_FORMAT;
        }
        if (mySdpRange->format == RtspRangeType::INVALID_RANGE)
        {
            PVMF_SDP_PARSER_LOGINFO((0, "SDP_Parser::parseSDP - No Valid Session Range - Setting it to Max of all media ranges"));
            for (int32 i = sdp->getNumMediaObjects() - 1; i >= 0; i--)
            {
                Oscl_Vector<mediaInfo*, SDPParserAlloc> mediaInfoVec =
                    sdp->getMediaInfo(i);
                for (uint32 j = 0; j < mediaInfoVec.size(); j++)
                {
                    mediaInfo* mInfo = mediaInfoVec[j];
                    if (mInfo == NULL)
                    {
                        continue;
                    }
                    const RtspRangeType *mInfoSdpRange = mInfo->getRtspRange();
                    if (NULL == mInfoSdpRange)
                    {
                        continue;
                    }
                    if (mInfoSdpRange->format != RtspRangeType::NPT_RANGE)
                    {
                        continue;
                    }
                    if (mySdpRange->format == RtspRangeType::INVALID_RANGE)
                    {
                        *mySdpRange = *mInfoSdpRange;
                    }
                    if (!mInfoSdpRange->end_is_set)
                    {//live streaming
                        *mySdpRange = *mInfoSdpRange;
                        mySdpRange->start_is_set = true;//just to make sure
                        mySdpRange->npt_start.npt_format = NptTimeFormat::NOW;
                        return SDP_SUCCESS;
                    }
                    if (mInfoSdpRange->npt_start.npt_format == NptTimeFormat::NPT_SEC)
                    {
                        if (mInfoSdpRange->npt_start.npt_sec.sec < mySdpRange->npt_start.npt_sec.sec)
                        {
                            mySdpRange->npt_start.npt_sec = mInfoSdpRange->npt_start.npt_sec;
                        }
                        else if ((mInfoSdpRange->npt_start.npt_sec.sec == mySdpRange->npt_start.npt_sec.sec)
                                 && ((mInfoSdpRange->npt_start.npt_sec.milli_sec < mySdpRange->npt_start.npt_sec.milli_sec)))
                        {
                            mySdpRange->npt_start.npt_sec = mInfoSdpRange->npt_start.npt_sec;
                        }
                    }
                    if (mInfoSdpRange->npt_end.npt_format == NptTimeFormat::NPT_SEC)
                    {
                        if (mInfoSdpRange->npt_end.npt_sec.sec > mySdpRange->npt_end.npt_sec.sec)
                        {
                            mySdpRange->npt_end.npt_sec = mInfoSdpRange->npt_end.npt_sec;
                        }
                        else if ((mInfoSdpRange->npt_end.npt_sec.sec == mySdpRange->npt_end.npt_sec.sec)
                                 && ((mInfoSdpRange->npt_end.npt_sec.milli_sec > mySdpRange->npt_end.npt_sec.milli_sec)))
                        {
                            mySdpRange->npt_end.npt_sec = mInfoSdpRange->npt_end.npt_sec;
                        }
                    }
                }
            }
        }
    }
    return SDP_SUCCESS;
}

OSCL_EXPORT_REF SDP_ERROR_CODE
SDP_Parser::parseSDPDownload(const char *sdpText,
                             int length,
                             SDPInfo *sdp,
                             movieInfo *mv)
{
    SDP_ERROR_CODE retval = parseSDP(sdpText, length, sdp);
    if (retval != SDP_SUCCESS)
    {
        mv->trackCount = 0;
        mv->movieName[0] = '\0';
        mv->creationDate[0] = '\0';
        return retval;
    }
    else
    {
        /*Get Movie name*/
        int len = oscl_strlen(sdp->getSessionInfo()->getSessionName());
        if (len >= MAX_STRING_LEN)
        {
            oscl_strncpy(mv->movieName, sdp->getSessionInfo()->getSessionName(), (MAX_STRING_LEN - 1));
            mv->movieName[MAX_STRING_LEN-1] = '\0';
        }
        else
        {
            oscl_strncpy(mv->movieName, sdp->getSessionInfo()->getSessionName(), len);
            mv->movieName[len] = '\0';
        }

        /*Get creation date*/
        len = oscl_strlen(sdp->getSessionInfo()->getCreationDate());
        if (len >= MAX_STRING_LEN)
        {
            oscl_strncpy(mv->creationDate, sdp->getSessionInfo()->getCreationDate(), (MAX_STRING_LEN - 1));
            mv->creationDate[MAX_STRING_LEN-1] = '\0';
        }
        else
        {
            oscl_strncpy(mv->creationDate, sdp->getSessionInfo()->getCreationDate(), len);
            mv->creationDate[len] = '\0';
        }

        /*Get number of tracks*/
        mv->trackCount = sdp->getNumMediaObjects();

        /*Get start stop times*/
        convertToMilliSec(*sdp->getSessionInfo()->getRange(), mv->duration.startTime, mv->duration.stopTime);

        /*Get MIMEType and other track info*/
        for (int ii = 0; ii < mv->trackCount; ii++)
        {
            Oscl_Vector<mediaInfo*, SDPParserAlloc> mediaInfoVec =
                sdp->getMediaInfo(ii);

            /*
             * There would only be one element in this vector
             * for fast track download content.
             */
            mediaInfo* minfo = mediaInfoVec[0];

            mv->TrackArray[ii].bitrate = minfo->getBitrate();
            len = oscl_strlen(minfo->getMIMEType());
            if (len >= MAX_STRING_LEN)
            {
                oscl_strncpy(mv->TrackArray[ii].codec_type,
                             minfo->getMIMEType(),
                             (MAX_STRING_LEN - 1));
                mv->TrackArray[ii].codec_type[MAX_STRING_LEN-1] = '\0';
            }
            else
            {
                oscl_strncpy(mv->TrackArray[ii].codec_type,
                             minfo->getMIMEType(),
                             len);
                mv->TrackArray[ii].codec_type[len] = '\0';
            }
            const char *trackID = minfo->getControlURL();
            int track = 0;
            if (trackID != NULL)
            {
                const char *locateID = oscl_strstr(trackID, "=");
                if (locateID != NULL)
                {
                    locateID += 1;
                    uint32 atoi_tmp;
                    PV_atoi(locateID, 'd', atoi_tmp);
                    track = atoi_tmp;
                }
            }
            mv->TrackArray[ii].trackID = track;
        }
    }
    return SDP_SUCCESS;
}


int
SDP_Parser::convertToMilliSec(RtspRangeType range , int &startTime, int &stopTime)
{
    switch (range.format)
    {
        case RtspRangeType::NPT_RANGE:
        {
            if (range.start_is_set)
            {
                switch (range.npt_start.npt_format)
                {
                    case NptTimeFormat::NOW:
                    {
                        startTime = 0;
                    }
                    break;
                    case NptTimeFormat::NPT_SEC:
                    {
                        startTime = (int)(1000 * ((float)range.npt_start.npt_sec.sec + range.npt_start.npt_sec.milli_sec));
                    }
                    break;
                    case NptTimeFormat::NPT_HHMMSS:
                    {
                        startTime = 3600000 * range.npt_start.npt_hhmmss.hours + 60000 * range.npt_start.npt_hhmmss.min + 1000 * range.npt_start.npt_hhmmss.sec + (int)(10 * range.npt_start.npt_hhmmss.frac_sec);
                    }
                    break;
                }
            }
            else
            {
                startTime = 0;
            }
            if (range.end_is_set)
            {
                switch (range.npt_end.npt_format)
                {
                    case NptTimeFormat::NOW:
                    {
                        stopTime = 0;
                    }
                    break;
                    case NptTimeFormat::NPT_SEC:
                    {
                        stopTime = (int)(1000 * ((float)range.npt_end.npt_sec.sec + range.npt_end.npt_sec.milli_sec));
                    }
                    break;
                    case NptTimeFormat::NPT_HHMMSS:
                    {
                        stopTime = 3600000 * range.npt_end.npt_hhmmss.hours + 60000 * range.npt_end.npt_hhmmss.min + 1000 * range.npt_end.npt_hhmmss.sec + (int)(100 * range.npt_end.npt_hhmmss.frac_sec);
                    }
                    break;
                }
            }
            else
            {
                stopTime = false;
            }
        }
        break;
        case RtspRangeType::SMPTE_RANGE:
        case RtspRangeType::SMPTE_25_RANGE:
        case RtspRangeType::SMPTE_30_RANGE:
        {
            if (range.start_is_set)
            {
                startTime = 3600000 * range.smpte_start.hours + 60000 * range.smpte_start.minutes + 1000 * range.smpte_start.seconds;
            }
            else
            {
                startTime = 0;
            }
            if (range.end_is_set)
            {
                stopTime = 3600000 * range.smpte_end.hours + 60000 * range.smpte_end.minutes + 1000 * range.smpte_end.seconds;
            }
            else
            {
                stopTime = 0;
            }
        }
        break;
        case RtspRangeType::ABS_RANGE:
        {
            startTime = 0;
            stopTime = 0;
        }
        break;
        case RtspRangeType::UNKNOWN_RANGE:
        case RtspRangeType::INVALID_RANGE:
        {
            startTime = 0;
            stopTime = 0;
            return -1;
        }
        // break;	This statement was removed to avoid compiler warning for Unreachable Code

        default:
        {
            startTime = 0;
            stopTime = 0;
            return -1;
        }
        // break;	This statement was removed to avoid compiler warning for Unreachable Code
    }
    return 0;
}

OSCL_EXPORT_REF int SDP_Parser::getNumberOfTracks()
{
    return mediaArrayIndex;
}

OSCL_EXPORT_REF int SDP_Parser::setNumberOfTracks(int tracks)
{
    return (mediaArrayIndex = tracks);
}


