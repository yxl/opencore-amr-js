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

#include "session_info_parser.h"
#include "oscl_string_utils.h"
#include "rtsp_time_formats.h"
#include "sdp_info.h"
#include "oscl_str_ptr_len.h"
#include "rtsp_range_utils.h"
#include "oscl_string_containers.h"


SDP_ERROR_CODE parseSDPSessionInfo(const char *sdp_text, int length, SDPInfo *sdp)
{

    sessionDescription *session = sdp->getSessionInfo();

    bool o_field_found =  false;
    bool s_field_found = false;

    bool a_control_found = false;
    bool a_range_found = false;
    bool t_field_found = false;
    bool u_field_found = false;

    const char *end = sdp_text + length;

    const char *current_start = sdp_text;
    const char *line_start_ptr, *line_end_ptr;

    OsclMemoryFragment memFrag;

    // setup the known attribute fields here
    StrPtrLen author("a=author:");
    StrPtrLen version("a=version:");
    StrPtrLen copyright("a=copyright:");
    StrPtrLen rating("a=rating:");
    StrPtrLen range("a=range:");
    StrPtrLen wmf_version("a=X-wmfversion:");
    StrPtrLen allowrecord("a=X-allowrecord");
    StrPtrLen random_access("a=random_access_denied");
    StrPtrLen control("a=control:");
    StrPtrLen creation_date("a=creation_date:");
    StrPtrLen video_only_allowed_true("a=video_only_allowed:true");
    StrPtrLen video_only_allowed_false("a=video_only_allowed:false");

    StrPtrLen etag("a=etag:");
    //StrPtrLen security("a=X-security_flag:");
    StrPtrLen random_filler("a=X-random_filler:");
    StrPtrLen digital_signature("a=X-digital_signature:");
    StrPtrLen end_of_clip("a=X-end_of_clip:");
    StrPtrLen launch_URL("a=X-launch_URL:");
    StrPtrLen app_specific_string("a=X-app_specific_string:");
    StrPtrLen pause_denied("a=X-pause_denied");

    StrPtrLen qoe_metrics("a=3GPP-QoE-Metrics:");
    StrPtrLen sdp_int_key("a=3GPP-Integrity-Key:");
    StrPtrLen sdp_auth("a=3GPP-SDP-Auth:");
    StrPtrLen alt_gp("a=alt-group:");
    StrPtrLen asset_info("a=3GPP-Asset-Information:");

    // added for RM

    StrPtrLen sdpplinversion("a=SdpplinVersion:");
    StrPtrLen flags("a=Flags:");
    StrPtrLen isrealdatatype("a=IsRealDataType:");
    StrPtrLen streamcount("a=StreamCount:");
    StrPtrLen title("a=Title:");
    StrPtrLen keywords("a=Keywords:");
    StrPtrLen abstract("a=Abstract:");
    StrPtrLen ASMrulebook("a=ASMRuleBook");



    while (get_next_line(current_start, end,
                         line_start_ptr, line_end_ptr))
    {
        // check for line type
        switch (*line_start_ptr)
        {
            case 'v':
            {
                uint32 version;
                if (*(line_start_ptr + 1) != '=')
                {
                    return SDP_BAD_SESSION_FORMAT;
                }
                if (PV_atoi(line_start_ptr + 2, 'd', line_end_ptr - line_start_ptr - 2, version) == false)
                {
                    return SDP_BAD_SESSION_FORMAT;
                }

                if (version)
                    return SDP_BAD_SESSION_FORMAT;
                session->setProtocolVersion(version);

                break;
            }

            case 'o':
            {
                if (*(line_start_ptr + 1) != '=')
                {
                    return SDP_BAD_SESSION_FORMAT;
                }

                o_field_found = true;
                // parse through each field
                const char *sptr, *eptr;
                sptr = skip_whitespace(line_start_ptr + 2, line_end_ptr);
                if (sptr >= line_end_ptr)
                {
                    return SDP_BAD_SESSION_FORMAT;
                }

                eptr = skip_to_whitespace(sptr, line_end_ptr);
                if (eptr <= sptr)
                {
                    return SDP_BAD_SESSION_FORMAT;
                }

                // get the username
                memFrag.ptr = (void*)sptr;
                memFrag.len = (eptr - sptr);
                session->setOUsername(memFrag);

                // get the sessionID
                sptr = skip_whitespace(eptr, line_end_ptr);
                //o=- -1586996066 -1586996066 IN IP4 172.31.105.21
                //workaround for server found during IOT. 12/12/06
                while ((*sptr < '0') || (*sptr > '9'))
                {
                    sptr++;
                    if (sptr >= line_end_ptr)
                    {
                        return SDP_BAD_SESSION_FORMAT;
                    }
                }

                eptr = skip_to_whitespace(sptr, line_end_ptr);
                if (eptr <= sptr)
                {
                    return SDP_BAD_SESSION_FORMAT;
                }

                uint64 sessionId64;
                if (!PV_atoi(sptr, 'd', eptr - sptr, sessionId64)) // session ID should be numeric
                    return SDP_BAD_SESSION_FORMAT;

                uint32 sessionId =
                    Oscl_Int64_Utils::get_uint64_lower32(sessionId64);
                session->setOSessionID(sessionId);

                // get the version
                sptr = skip_whitespace(eptr, line_end_ptr);
                //o=- -1586996066 -1586996066 IN IP4 172.31.105.21
                //workaround for server found during IOT. 12/12/06
                while ((*sptr < '0') || (*sptr > '9'))
                {
                    sptr++;
                    if (sptr >= line_end_ptr)
                    {
                        return SDP_BAD_SESSION_FORMAT;
                    }
                }

                eptr = skip_to_whitespace(sptr, line_end_ptr);
                if (eptr <= sptr)
                {
                    return SDP_BAD_SESSION_FORMAT;
                }

                uint64 version64;
                if (!PV_atoi(sptr, 'd', eptr - sptr, version64))
                    return SDP_BAD_SESSION_FORMAT;

                uint32 version =
                    Oscl_Int64_Utils::get_uint64_lower32(version64);
                session->setOVersion(version);

                // get the network type
                sptr = skip_whitespace(eptr, line_end_ptr);
                if (sptr >= line_end_ptr)
                {
                    return SDP_BAD_SESSION_FORMAT;
                }

                eptr = skip_to_whitespace(sptr, line_end_ptr);
                if (eptr <= sptr)
                {
                    return SDP_BAD_SESSION_FORMAT;
                }
                memFrag.ptr = (void*)sptr;
                memFrag.len = (eptr - sptr);
                session->setONetworkType(memFrag);

                // get the address type
                sptr = skip_whitespace(eptr, line_end_ptr);
                if (sptr >= line_end_ptr)
                {
                    return SDP_BAD_SESSION_FORMAT;
                }

                eptr = skip_to_whitespace(sptr, line_end_ptr);
                if (eptr <= sptr)
                {
                    return SDP_BAD_SESSION_FORMAT;
                }
                memFrag.ptr = (void*)sptr;
                memFrag.len = (eptr - sptr);
                session->setOAddressType(memFrag);


                // get the orginator address
                sptr = skip_whitespace(eptr, line_end_ptr);
                if (sptr >= line_end_ptr)
                {
                    return SDP_BAD_SESSION_FORMAT;
                }

                eptr = skip_to_whitespace(sptr, line_end_ptr);
                if (eptr <= sptr)
                {
                    return SDP_BAD_SESSION_FORMAT;
                }
                memFrag.ptr = (void*)sptr;
                memFrag.len = (eptr - sptr);
                session->setOAddress(memFrag);


                break;
            }

            case 's':
            {
                // the session Name is a string which can contain whitespace

                if (s_field_found) // only one s field allowed at session level
                    return SDP_BAD_SESSION_FORMAT;

                if (*(line_start_ptr + 1) != '=')
                {
                    return SDP_BAD_SESSION_FORMAT;
                }

                s_field_found = true;

                // parse through each field
                const char *sptr, *eptr;

                sptr = skip_whitespace(line_start_ptr + 2, line_end_ptr);
                if (sptr >= line_end_ptr)
                {
                    break;
                    //          return SDP_BAD_SESSION_FORMAT;
                }

                eptr = skip_to_line_term(sptr, line_end_ptr);
                if (eptr <= sptr)
                {

                    return SDP_BAD_SESSION_FORMAT;
                }

                // get the session name
                memFrag.ptr = (void*)sptr;
                memFrag.len = (eptr - sptr);
                session->setSessionName(memFrag);

                break;
            }

            case 'u':
            {
                // the session Name is a string which can contain whitespace

                if (u_field_found) // only one s field allowed at session level
                    return SDP_BAD_SESSION_FORMAT;

                if (*(line_start_ptr + 1) != '=')
                {
                    return SDP_BAD_SESSION_FORMAT;
                }

                u_field_found = true;

                // parse through each field
                const char *sptr, *eptr;

                sptr = skip_whitespace(line_start_ptr + 2, line_end_ptr);
                if (sptr >= line_end_ptr)
                {
                    break;
                    //          return SDP_BAD_SESSION_FORMAT;
                }

                eptr = skip_to_line_term(sptr, line_end_ptr);
                if (eptr <= sptr)
                {

                    return SDP_BAD_SESSION_FORMAT;
                }

                // get the session name
                memFrag.ptr = (void*)sptr;
                memFrag.len = (eptr - sptr);
                session->setSessionURI(memFrag);

                break;
            }

            case 'i':
            {
                // the session info is a string which can contain whitespace
                if (*(line_start_ptr + 1) != '=')
                {
                    return SDP_BAD_SESSION_FORMAT;
                }

                // parse through each field
                const char *sptr, *eptr;

                sptr = skip_whitespace(line_start_ptr + 2, line_end_ptr);
                if (sptr >= line_end_ptr)
                {
                    break;
                    //          return SDP_BAD_SESSION_FORMAT;
                }

                eptr = skip_to_line_term(sptr, line_end_ptr);
                if (eptr <= sptr)
                {
                    return SDP_BAD_SESSION_FORMAT;
                }

                // get the session name
                memFrag.ptr = (void*)sptr;
                memFrag.len = (eptr - sptr);
                session->setSessionInformation(memFrag);


                break;
            }

            case 'c':
            {
                if (*(line_start_ptr + 1) != '=')
                {
                    return SDP_BAD_SESSION_FORMAT;
                }

                session->setCFieldStatus(true);
                // parse through each field
                const char *sptr, *eptr;

                // get the connection network type
                sptr = skip_whitespace(line_start_ptr + 2, line_end_ptr);
                if (sptr >= line_end_ptr)
                {
                    return SDP_BAD_SESSION_FORMAT;
                }

                eptr = skip_to_whitespace(sptr, line_end_ptr);
                if (eptr <= sptr)
                {
                    return SDP_BAD_SESSION_FORMAT;
                }

                memFrag.ptr = (void*)sptr;
                memFrag.len = (eptr - sptr);
                session->setCNetworkType(memFrag);


                // get the address type
                sptr = skip_whitespace(eptr, line_end_ptr);
                if (sptr >= line_end_ptr)
                {
                    return SDP_BAD_SESSION_FORMAT;
                }

                eptr = skip_to_whitespace(sptr, line_end_ptr);
                if (eptr <= sptr)
                {
                    return SDP_BAD_SESSION_FORMAT;
                }
                memFrag.ptr = (void*)sptr;
                memFrag.len = (eptr - sptr);
                session->setCAddressType(memFrag);
                uint32 len = OSCL_MIN((uint32)(eptr - sptr), oscl_strlen("IP6"));
                if (oscl_strncmp(sptr, "IP6", len) == 0)
                {
                    //TBD
                    break;
                }


                // get the address
                sptr = skip_whitespace(eptr, line_end_ptr);
                if (sptr >= line_end_ptr)
                {
                    return SDP_BAD_SESSION_FORMAT;
                }

                eptr = skip_to_whitespace(sptr, line_end_ptr);
                if (eptr < sptr)
                {
                    return SDP_BAD_SESSION_FORMAT;
                }
                memFrag.ptr = (void*)sptr;
                memFrag.len = (eptr - sptr);
                session->setCAddress(memFrag);
                uint32 address;
                const char *addrend = sptr;
                for (;*addrend != '.';++addrend);

                PV_atoi(sptr, 'd', addrend - sptr, address);

                if (address >= 224 && address <= 239) //multicast address look for TTL
                {
                    for (; (*sptr != '/') && (sptr < eptr); ++sptr);
                    if (sptr == eptr)
                        return SDP_BAD_SESSION_FORMAT; // no TTL found in multicast address.
                    else
                    {
                        uint32 ttl;
                        sptr = sptr + 1;
                        if (!PV_atoi(sptr, 'd', eptr - sptr, ttl))
                            return SDP_BAD_SESSION_FORMAT;
                        if (!(ttl <= 255))
                            return SDP_BAD_SESSION_FORMAT; // ttl out of range.

                    }
                }
                else  // unicast address
                {
                    for (; (*sptr != '/') && (sptr < eptr); ++sptr);
                    if (!oscl_strncmp(sptr, "/", 1))
                        return SDP_BAD_SESSION_FORMAT; //unicast address can not have TTL.
                }

                if (eptr < line_end_ptr)
                    return SDP_BAD_SESSION_FORMAT;
                break;
            }

            case 't':
            {
                uint64 start_time, stop_time;
                if (*(line_start_ptr + 1) != '=')
                {
                    return SDP_BAD_SESSION_FORMAT;
                }

                t_field_found = true;

                const char *sptr, *eptr;
                sptr = skip_whitespace(line_start_ptr + 2, line_end_ptr);
                if (sptr >= line_end_ptr)
                {
                    return SDP_BAD_SESSION_FORMAT;
                }

                eptr = skip_to_whitespace(sptr, line_end_ptr);
                if (eptr <= sptr)
                {
                    return SDP_BAD_SESSION_FORMAT;
                }

                if (PV_atoi(sptr, 'd', (eptr - sptr), start_time) == false)
                {
                    return SDP_BAD_SESSION_FORMAT;
                }

                session->setSessionStartTime(start_time);

                sptr = skip_whitespace(eptr, line_end_ptr);
                if (sptr >= line_end_ptr)
                {
                    return SDP_BAD_SESSION_FORMAT;
                }

                eptr = skip_to_whitespace(sptr, line_end_ptr);
                if (eptr <= sptr)
                {
                    return SDP_BAD_SESSION_FORMAT;
                }

                if (PV_atoi(sptr, 'd', (eptr - sptr),  stop_time) == false)
                {
                    return SDP_BAD_SESSION_FORMAT;
                }

                session->setSessionStopTime(stop_time);


                memFrag.ptr = (void*)sptr;
                memFrag.len = (eptr - sptr);
                session->setExpiryDate(memFrag);
                break;
            }

            case 'r':
            {
                uint32 repeat_interval, active_duration;
                char convert_to_sec = NULL_TERM_CHAR;

                if (*(line_start_ptr + 1) != '=')
                {
                    return SDP_BAD_SESSION_FORMAT;
                }

                const char *sptr, *eptr;
                sptr = skip_whitespace(line_start_ptr + 2, line_end_ptr);
                if (sptr >= line_end_ptr)
                {
                    return SDP_BAD_SESSION_FORMAT;
                }

                eptr = skip_to_whitespace(sptr, line_end_ptr);
                if (eptr <= sptr)
                {
                    return SDP_BAD_SESSION_FORMAT;
                }

                int size = 0;
                if (!oscl_strncmp((eptr - 1), "d", 1) || !oscl_strncmp((eptr - 1), "h", 1) || !oscl_strncmp((eptr - 1), "m", 1) || !oscl_strncmp((eptr - 1), "s", 1))
                {
                    convert_to_sec = *(eptr - 1);
                    size = eptr - 1 - sptr;
                }
                else
                    size = eptr - sptr;


                if (PV_atoi(sptr, 'd', size, repeat_interval) == false)
                {
                    return SDP_BAD_SESSION_FORMAT;
                }

                if (convert_to_sec == 'd')
                {
                    repeat_interval = repeat_interval * 24 * 3600;
                    convert_to_sec = NULL_TERM_CHAR;
                }
                else if (convert_to_sec == 'h')
                {
                    repeat_interval = repeat_interval * 3600;
                    convert_to_sec = NULL_TERM_CHAR;
                }
                else if (convert_to_sec == 'm')
                {
                    repeat_interval = repeat_interval * 60;
                    convert_to_sec = NULL_TERM_CHAR;
                }
                else if (convert_to_sec == 's')
                {
                    convert_to_sec = NULL_TERM_CHAR; // already in seconds.
                }



                session->setRepeatInterval(repeat_interval);
                eptr = skip_whitespace(eptr, line_end_ptr);

                sptr = skip_whitespace(eptr, line_end_ptr);
                if (sptr >= line_end_ptr)
                {
                    return SDP_BAD_SESSION_FORMAT;
                }

                eptr = skip_to_whitespace(sptr, line_end_ptr);
                if (eptr <= sptr)
                {
                    return SDP_BAD_SESSION_FORMAT;
                }

                if (!oscl_strncmp((eptr - 1), "d", 1) || !oscl_strncmp((eptr - 1), "h", 1) || !oscl_strncmp((eptr - 1), "m", 1) || !oscl_strncmp((eptr - 1), "s", 1))
                {
                    convert_to_sec = *(eptr - 1);
                    size = eptr - 1 - sptr;
                }
                else
                    size = eptr - sptr;

                if (PV_atoi(sptr, 'd', size,  active_duration) == false)
                {
                    return SDP_BAD_SESSION_FORMAT;
                }

                if (convert_to_sec == 'd')
                {
                    active_duration = active_duration * 24 * 3600;
                    convert_to_sec = NULL_TERM_CHAR;
                }
                else if (convert_to_sec == 'h')
                {
                    active_duration = active_duration * 3600;
                    convert_to_sec = NULL_TERM_CHAR;
                }
                else if (convert_to_sec == 'm')
                {
                    active_duration = active_duration * 60;
                    convert_to_sec = NULL_TERM_CHAR;
                }
                else if (convert_to_sec == 's')
                {
                    convert_to_sec = NULL_TERM_CHAR;
                }

                session->setActiveDuration(active_duration);

                while (sptr < line_end_ptr)
                {
                    uint32 offset;

                    sptr = skip_whitespace(eptr, line_end_ptr);
                    if (sptr >= line_end_ptr)
                    {
                        return SDP_BAD_SESSION_FORMAT;
                    }

                    eptr = skip_to_whitespace(sptr, line_end_ptr);
                    if (eptr <= sptr)
                    {
                        return SDP_BAD_SESSION_FORMAT;
                    }

                    if (!oscl_strncmp((eptr - 1), "d", 1) || !oscl_strncmp((eptr - 1), "h", 1) || !oscl_strncmp((eptr - 1), "m", 1) || !oscl_strncmp((eptr - 1), "s", 1))
                    {
                        size = eptr - 1 - sptr;
                        convert_to_sec = *(eptr - 1);
                    }
                    else
                        size = eptr - sptr;

                    if (PV_atoi(sptr, 'd', size,  offset) == false)
                    {
                        return SDP_BAD_SESSION_FORMAT;
                    }

                    if (convert_to_sec == 'd')
                    {
                        offset = offset * 24 * 3600;
                        convert_to_sec = NULL_TERM_CHAR;
                    }
                    else if (convert_to_sec == 'h')
                    {
                        offset = offset * 3600;
                        convert_to_sec = NULL_TERM_CHAR;
                    }
                    else if (convert_to_sec == 'm')
                    {
                        offset = offset * 60;
                        convert_to_sec = NULL_TERM_CHAR;
                    }
                    else if (convert_to_sec == 's')
                    {
                        convert_to_sec = NULL_TERM_CHAR;
                    }

                    session->addOffset(offset);

                    sptr = eptr;
                }
                break;
            }

            case 'b':
            {
                if (!oscl_strncmp(line_start_ptr, "b=AS:", oscl_strlen("b=AS:")))
                {
                    const char *sptr;
                    sptr = line_start_ptr + oscl_strlen("b=AS:");
                    sptr = skip_whitespace(sptr, line_end_ptr);
                    if (sptr >= line_end_ptr)
                    {
                        return SDP_BAD_SESSION_FORMAT;
                    }

                    uint32 bitRate;
                    if (PV_atoi(sptr, 'd', (line_end_ptr - sptr),  bitRate) == true)
                    {
                        session->setBitrate(1000*bitRate);
                    }
                    else
                    {
                        return SDP_BAD_SESSION_FORMAT;
                    }
                }
                else if (!oscl_strncmp(line_start_ptr, "b=RS:", oscl_strlen("b=RS:")))
                {
                    const char *sptr;
                    sptr = line_start_ptr + oscl_strlen("b=AS:");
                    sptr = skip_whitespace(sptr, line_end_ptr);
                    if (sptr >= line_end_ptr)
                    {
                        return SDP_BAD_SESSION_FORMAT;
                    }

                    uint32 rtcpBWSender;
                    if (PV_atoi(sptr, 'd', (line_end_ptr - sptr),  rtcpBWSender) == true)
                    {
                        session->setRTCPSenderBitRate(rtcpBWSender);
                    }
                    else
                    {
                        return SDP_BAD_SESSION_FORMAT;
                    }
                }
                else if (!oscl_strncmp(line_start_ptr, "b=RR:", oscl_strlen("b=RR:")))
                {
                    const char *sptr;
                    sptr = line_start_ptr + oscl_strlen("b=AS:");
                    sptr = skip_whitespace(sptr, line_end_ptr);
                    if (sptr >= line_end_ptr)
                    {
                        return SDP_BAD_SESSION_FORMAT;
                    }

                    uint32 rtcpBWReceiver;
                    if (PV_atoi(sptr, 'd', (line_end_ptr - sptr),  rtcpBWReceiver) == true)
                    {
                        session->setRTCPReceiverBitRate(rtcpBWReceiver);
                    }
                    else
                    {
                        return SDP_BAD_SESSION_FORMAT;
                    }
                }
                else if (!oscl_strncmp(line_start_ptr, "b=TIAS:", oscl_strlen("b=TIAS:")))
                {
                    const char *sptr;
                    sptr = line_start_ptr + oscl_strlen("b=TIAS:");
                    sptr = skip_whitespace(sptr, line_end_ptr);
                    if (sptr >= line_end_ptr)
                    {
                        return SDP_BAD_SESSION_FORMAT;
                    }

                    uint32 bMod;
                    if (PV_atoi(sptr, 'd', (line_end_ptr - sptr),  bMod) == true)
                    {
                        session->setBWtias(1000 * bMod);
                    }
                    else
                    {
                        return SDP_BAD_SESSION_FORMAT;
                    }
                }
                break;
            }

            case 'k':
            {
                const char *sptr, *eptr;
                if (!oscl_strncmp(line_start_ptr, "k=clear:", oscl_strlen("k=clear:")))
                {
                    sptr = line_start_ptr + oscl_strlen("k=clear:");
                    sptr = skip_whitespace(sptr, line_end_ptr);
                    if (sptr >= line_end_ptr)
                    {
                        return SDP_BAD_SESSION_FORMAT;
                    }

                    eptr = skip_to_line_term(sptr, line_end_ptr);
                    if (eptr <= sptr)
                    {
                        return SDP_BAD_SESSION_FORMAT;
                    }

                    memFrag.ptr = (void*)sptr;
                    memFrag.len = (eptr - sptr);
                    session->setEncryptionKey(memFrag);
                }
                else if (!oscl_strncmp(line_start_ptr, "k=base64:", oscl_strlen("k=base64:")))
                {

                    sptr = line_start_ptr + oscl_strlen("k=base64:");
                    sptr = skip_whitespace(sptr, line_end_ptr);
                    if (sptr >= line_end_ptr)
                    {
                        return SDP_BAD_SESSION_FORMAT;
                    }

                    eptr = skip_to_line_term(sptr, line_end_ptr);
                    if (eptr <= sptr)
                    {
                        return SDP_BAD_SESSION_FORMAT;
                    }

                    memFrag.ptr = (void*)sptr;
                    memFrag.len = (eptr - sptr);
                    session->setEncryptionKey(memFrag);
                }
                else if (!oscl_strncmp(line_start_ptr, "k=uri:", oscl_strlen("k=uri:")))
                {

                    sptr = line_start_ptr + oscl_strlen("k=uri:");
                    sptr = skip_whitespace(sptr, line_end_ptr);
                    if (sptr >= line_end_ptr)
                    {
                        return SDP_BAD_SESSION_FORMAT;
                    }

                    eptr = skip_to_line_term(sptr, line_end_ptr);
                    if (eptr <= sptr)
                    {
                        return SDP_BAD_SESSION_FORMAT;
                    }

                    memFrag.ptr = (void*)sptr;
                    memFrag.len = (eptr - sptr);
                    session->setEncryptionKey(memFrag);
                }
                else
                {
                    sptr = line_start_ptr;
                    eptr = skip_to_line_term(sptr, line_end_ptr);
                    if (eptr <= sptr)
                    {
                        return SDP_BAD_SESSION_FORMAT;
                    }
                }
                break;
            }

            case 'a':
            {

                // check to see if it is a known attribute -- otherwise ignore it.
                const char *sptr, *eptr;
                if (!oscl_strncmp(line_start_ptr, author.c_str(), author.length()))
                {
                    sptr = skip_whitespace(line_start_ptr + author.length(),
                                           line_end_ptr);
                    if (sptr >= line_end_ptr)
                    {
                        break;
                        //            return SDP_BAD_SESSION_FORMAT;
                    }

                    eptr = skip_to_line_term(sptr, line_end_ptr);
                    if (eptr <= sptr)
                    {
                        return SDP_BAD_SESSION_FORMAT;
                    }

                    memFrag.ptr = (void*)sptr;
                    memFrag.len = (eptr - sptr);
                    session->setAuthor(memFrag);

                }
                else if (!oscl_strncmp(line_start_ptr, version.c_str(),
                                       version.length()))
                {
                    sptr = skip_whitespace(line_start_ptr + version.length(),
                                           line_end_ptr);
                    if (sptr >= line_end_ptr)
                    {
                        break;
                        //            return SDP_BAD_SESSION_FORMAT;
                    }

                    eptr = skip_to_line_term(sptr, line_end_ptr);
                    if (eptr <= sptr)
                    {
                        return SDP_BAD_SESSION_FORMAT;
                    }

                    memFrag.ptr = (void*)sptr;
                    memFrag.len = (eptr - sptr);
                    session->setVersion(memFrag);


                }
                else if (!oscl_strncmp(line_start_ptr, copyright.c_str(),
                                       copyright.length()))
                {

                    sptr = skip_whitespace(line_start_ptr + copyright.length(),
                                           line_end_ptr);
                    if (sptr >= line_end_ptr)
                    {
                        break;
                        //            return SDP_BAD_SESSION_FORMAT;
                    }

                    eptr = skip_to_line_term(sptr, line_end_ptr);
                    if (eptr <= sptr)
                    {
                        return SDP_BAD_SESSION_FORMAT;
                    }

                    memFrag.ptr = (void*)sptr;
                    memFrag.len = (eptr - sptr);
                    session->setCopyright(memFrag);

                }
                else if (!oscl_strncmp(line_start_ptr, rating.c_str(), rating.length()))
                {

                    sptr = skip_whitespace(line_start_ptr + rating.length(),
                                           line_end_ptr);
                    if (sptr >= line_end_ptr)
                    {
                        break;
                        //            return SDP_BAD_SESSION_FORMAT;
                    }

                    eptr = skip_to_line_term(sptr, line_end_ptr);
                    if (eptr <= sptr)
                    {
                        return SDP_BAD_SESSION_FORMAT;
                    }
                    memFrag.ptr = (void*)sptr;
                    memFrag.len = (eptr - sptr);
                    session->setRating(memFrag);
                }
                else if (!oscl_strncmp(line_start_ptr, range.c_str(), range.length()))
                {
                    RtspRangeType rangeVal;
                    if (parseRtspRange((line_start_ptr + range.length()), (line_end_ptr - line_start_ptr - range.length()),
                                       rangeVal))
                    {
                        session->setRange(rangeVal);
                        a_range_found = true;
                    }
                }
                else if (!oscl_strncmp(line_start_ptr, wmf_version.c_str(),
                                       wmf_version.length()))
                {

                    sptr = skip_whitespace(line_start_ptr + wmf_version.length(),
                                           line_end_ptr);
                    if (sptr >= line_end_ptr)
                    {
                        break;
                        //            return SDP_BAD_SESSION_FORMAT;
                    }

                    eptr = skip_to_line_term(sptr, line_end_ptr);
                    if (eptr <= sptr)
                    {
                        return SDP_BAD_SESSION_FORMAT;
                    }

                    memFrag.ptr = (void*)sptr;
                    memFrag.len = (eptr - sptr);
                    session->setWmfVersion(memFrag);
                }
                else if (!oscl_strncmp(line_start_ptr, allowrecord.c_str(),
                                       allowrecord.length()))
                {
                    session->setAllowRecord(true);
                }
                else if (!oscl_strncmp(line_start_ptr, random_access.c_str(),
                                       random_access.length()))
                {
                    session->setRandomAccessDenied(true);
                }

                else if (!oscl_strncmp(line_start_ptr, control.c_str(),
                                       control.length()))
                {

                    sptr = skip_whitespace(line_start_ptr + control.length(),
                                           line_end_ptr);
                    if (sptr >= line_end_ptr)
                    {
                        break;
                        //            return SDP_BAD_SESSION_FORMAT;
                    }

                    eptr = skip_to_line_term(sptr, line_end_ptr);
                    if (eptr <= sptr)
                    {
                        return SDP_BAD_SESSION_FORMAT;
                    }

                    OSCL_HeapString<SDPParserAlloc> control_str(sptr, eptr - sptr);
                    session->setControlURL(control_str);
                    a_control_found = true;

                }

                else if (!oscl_strncmp(line_start_ptr, creation_date.c_str(),
                                       creation_date.length()))
                {

                    sptr = skip_whitespace(line_start_ptr + creation_date.length(),
                                           line_end_ptr);
                    if (sptr >= line_end_ptr)
                    {

                        return SDP_BAD_SESSION_FORMAT;
                    }

                    eptr = skip_to_line_term(sptr, line_end_ptr);
                    if (eptr <= sptr)
                    {
                        return SDP_BAD_SESSION_FORMAT;
                    }

                    memFrag.ptr = (void*)sptr;
                    memFrag.len = (eptr - sptr);
                    session->setCreationDate(memFrag);
                }

                else if (!oscl_strncmp(line_start_ptr, video_only_allowed_true.c_str(),
                                       video_only_allowed_true.length()))
                {

                    session->setVideoOnlyAllowed(ALLOWED);
                }

                else if (!oscl_strncmp(line_start_ptr, video_only_allowed_false.c_str(),
                                       video_only_allowed_false.length()))
                {
                    session->setVideoOnlyAllowed(NOT_ALLOWED);
                }

                else if (!oscl_strncmp(line_start_ptr, etag.c_str(), etag.length()))
                {

                    sptr = skip_whitespace(line_start_ptr + etag.length(),
                                           line_end_ptr);
                    if (sptr >= line_end_ptr)
                    {
                        break;
                    }

                    eptr = skip_to_line_term(sptr, line_end_ptr);
                    if (eptr <= sptr)
                    {
                        return SDP_BAD_SESSION_FORMAT;
                    }

                    memFrag.ptr = (void*)sptr;
                    memFrag.len = (eptr - sptr);
                    session->setETag(memFrag);
                }

                else if (!oscl_strncmp(line_start_ptr, random_filler.c_str(), random_filler.length()))
                {

                    sptr = skip_whitespace(line_start_ptr + random_filler.length(), line_end_ptr);

                    if (sptr >= line_end_ptr)
                    {
                        break;
                    }

                    eptr = skip_to_line_term(sptr, line_end_ptr);
                    if (eptr <= sptr)
                    {
                        return SDP_BAD_SESSION_FORMAT;
                    }

                    memFrag.ptr = (void*)sptr;
                    memFrag.len = (eptr - sptr);
                    session->setRandomFiller(memFrag);
                }

                else if (!oscl_strncmp(line_start_ptr, digital_signature.c_str(),
                                       digital_signature.length()))
                {

                    sptr = skip_whitespace(line_start_ptr + digital_signature.length(), line_end_ptr);

                    if (sptr >= line_end_ptr)
                    {
                        break;
                    }

                    eptr = skip_to_line_term(sptr, line_end_ptr);
                    if (eptr <= sptr)
                    {
                        return SDP_BAD_SESSION_FORMAT;
                    }

                    memFrag.ptr = (void*)sptr;
                    memFrag.len = (eptr - sptr);
                    session->setDigitalSignature(memFrag);
                }

                else if (!oscl_strncmp(line_start_ptr, launch_URL.c_str(), launch_URL.length()))
                {

                    sptr = skip_whitespace(line_start_ptr + launch_URL.length(), line_end_ptr);

                    if (sptr >= line_end_ptr)
                    {
                        break;
                    }

                    eptr = skip_to_line_term(sptr, line_end_ptr);
                    if (eptr <= sptr)
                    {
                        return SDP_BAD_SESSION_FORMAT;
                    }

                    memFrag.ptr = (void*)sptr;
                    memFrag.len = (eptr - sptr);
                    session->setLaunchURL(memFrag);
                }

                else if (!oscl_strncmp(line_start_ptr, end_of_clip.c_str(), end_of_clip.length()))
                {

                    sptr = skip_whitespace(line_start_ptr + end_of_clip.length(),
                                           line_end_ptr);

                    if (sptr >= line_end_ptr)
                    {
                        break;
                    }

                    eptr = skip_to_line_term(sptr, line_end_ptr);
                    if (eptr <= sptr)
                    {
                        return SDP_BAD_SESSION_FORMAT;
                    }
                    //uint32 action = 0;
                    StrPtrLen nothing("NOTHING");
                    StrPtrLen close("CLOSE");
                    StrPtrLen launch("LAUNCH");

                    if (!oscl_strncmp(nothing.c_str(), sptr, nothing.length()))
                    {
                        session->setEndOfClipAction(DEFAULT);
                    }
                    else if (!oscl_strncmp(close.c_str(), sptr, close.length()))
                    {
                        session->setEndOfClipAction(CLOSE_PLAYER);
                    }
                    else if (!oscl_strncmp(launch.c_str(), sptr, launch.length()))
                    {
                        session->setEndOfClipAction(LAUNCH_URL);
                    }
                }

                // added for RM
                else if (!oscl_strncmp(line_start_ptr, sdpplinversion.c_str(),
                                       sdpplinversion.length()))
                {
                    uint32 version;

                    sptr = skip_whitespace(line_start_ptr + sdpplinversion.length(), line_end_ptr);
                    if (sptr >= line_end_ptr)
                    {
                        return SDP_BAD_SESSION_FORMAT;
                    }

                    eptr = skip_to_whitespace(sptr, line_end_ptr);
                    if (eptr <= sptr)
                    {
                        return SDP_BAD_SESSION_FORMAT;
                    }

                    if (PV_atoi(sptr, 'd', (eptr - sptr), version) == false)
                    {
                        return SDP_BAD_SESSION_FORMAT;
                    }

                    session->setSdpplinVersion(version);

                }

                else if (!oscl_strncmp(line_start_ptr, flags.c_str(),
                                       flags.length()))
                {
                    uint32 flag;
                    sptr = skip_whitespace(line_start_ptr + flags.length(), line_end_ptr);
                    if (sptr >= line_end_ptr)
                    {
                        return SDP_BAD_SESSION_FORMAT;
                    }

                    eptr = skip_to_whitespace(sptr, line_end_ptr);
                    if (eptr <= sptr)
                    {
                        return SDP_BAD_SESSION_FORMAT;
                    }

                    for (; *sptr != ';'; sptr++);
                    sptr = sptr + 1;

                    if (PV_atoi(sptr, 'd', (eptr - sptr), flag) == false)
                    {
                        return SDP_BAD_SESSION_FORMAT;
                    }

                    session->setFlags(flag);

                }

                else if (!oscl_strncmp(line_start_ptr, streamcount.c_str(),
                                       streamcount.length()))
                {
                    uint32 count;

                    sptr = skip_whitespace(line_start_ptr + streamcount.length(), line_end_ptr);
                    if (sptr >= line_end_ptr)
                    {
                        return SDP_BAD_SESSION_FORMAT;
                    }

                    eptr = skip_to_whitespace(sptr, line_end_ptr);
                    if (eptr <= sptr)
                    {
                        return SDP_BAD_SESSION_FORMAT;
                    }

                    for (; *sptr != ';'; sptr++);
                    sptr = sptr + 1;

                    if (PV_atoi(sptr, 'd', (eptr - sptr), count) == false)
                    {
                        return SDP_BAD_SESSION_FORMAT;
                    }

                    session->setStreamCount(count);

                }

                else if (!oscl_strncmp(line_start_ptr, isrealdatatype.c_str(),
                                       isrealdatatype.length()))
                {
                    uint32 isrealdata;

                    sptr = skip_whitespace(line_start_ptr + isrealdatatype.length(), line_end_ptr);
                    if (sptr >= line_end_ptr)
                    {
                        return SDP_BAD_SESSION_FORMAT;
                    }

                    eptr = skip_to_whitespace(sptr, line_end_ptr);
                    if (eptr <= sptr)
                    {
                        return SDP_BAD_SESSION_FORMAT;
                    }

                    for (; *sptr != ';'; sptr++);
                    sptr = sptr + 1;

                    if (PV_atoi(sptr, 'd', (eptr - sptr), isrealdata) == false)
                    {
                        return SDP_BAD_SESSION_FORMAT;
                    }

                    session->setIsRealDataType(isrealdata);
                }
                else if (!oscl_strncmp(line_start_ptr, "a=Author:", author.length()))
                {
                    sptr = skip_whitespace(line_start_ptr + author.length(),
                                           line_end_ptr);
                    if (sptr >= line_end_ptr)
                    {
                        break;
                        //            return SDP_BAD_SESSION_FORMAT;
                    }

                    eptr = skip_to_line_term(sptr, line_end_ptr);
                    if (eptr <= sptr)
                    {
                        return SDP_BAD_SESSION_FORMAT;
                    }

                    for (; *sptr != ';'; sptr++);
                    sptr = sptr + 1;

                    memFrag.ptr = (void*)sptr;
                    memFrag.len = (eptr - sptr);
                    session->setAuthor(memFrag);

                }
                else if (!oscl_strncmp(line_start_ptr, "a=Copyright:", copyright.length()))
                {
                    sptr = skip_whitespace(line_start_ptr + copyright.length(),
                                           line_end_ptr);
                    if (sptr >= line_end_ptr)
                    {
                        break;
                        //            return SDP_BAD_SESSION_FORMAT;
                    }

                    eptr = skip_to_line_term(sptr, line_end_ptr);
                    if (eptr <= sptr)
                    {
                        return SDP_BAD_SESSION_FORMAT;
                    }

                    for (; *sptr != ';'; sptr++);
                    sptr = sptr + 1;

                    memFrag.ptr = (void*)sptr;
                    memFrag.len = (eptr - sptr);
                    session->setCopyright(memFrag);

                }
                else if (!oscl_strncmp(line_start_ptr, title.c_str(),
                                       title.length()))
                {

                    sptr = skip_whitespace(line_start_ptr + title.length(),
                                           line_end_ptr);
                    if (sptr >= line_end_ptr)
                    {
                        break;
                        //            return SDP_BAD_SESSION_FORMAT;
                    }

                    eptr = skip_to_line_term(sptr, line_end_ptr);
                    if (eptr <= sptr)
                    {
                        return SDP_BAD_SESSION_FORMAT;
                    }

                    for (; *sptr != ';'; sptr++);
                    sptr = sptr + 1;

                    memFrag.ptr = (void*)(sptr + 1);
                    memFrag.len = ((eptr - 2) - sptr);
                    session->setTitle(memFrag);

                }

                else if (!oscl_strncmp(line_start_ptr, keywords.c_str(),
                                       keywords.length()))
                {

                    sptr = skip_whitespace(line_start_ptr + keywords.length(),
                                           line_end_ptr);
                    if (sptr >= line_end_ptr)
                    {
                        break;
                        //            return SDP_BAD_SESSION_FORMAT;
                    }

                    eptr = skip_to_line_term(sptr, line_end_ptr);
                    if (eptr <= sptr)
                    {
                        return SDP_BAD_SESSION_FORMAT;
                    }

                    for (; *sptr != ';'; sptr++);
                    sptr = sptr + 1;

                    memFrag.ptr = (void*)(sptr + 1);
                    memFrag.len = ((eptr - 2) - sptr);
                    session->setKeywords(memFrag);

                }

                else if (!oscl_strncmp(line_start_ptr, abstract.c_str(),
                                       abstract.length()))
                {

                    sptr = skip_whitespace(line_start_ptr + abstract.length(),
                                           line_end_ptr);
                    if (sptr >= line_end_ptr)
                    {
                        break;
                        //            return SDP_BAD_SESSION_FORMAT;
                    }

                    eptr = skip_to_line_term(sptr, line_end_ptr);
                    if (eptr <= sptr)
                    {
                        return SDP_BAD_SESSION_FORMAT;
                    }

                    for (; *sptr != ';'; sptr++);
                    sptr = sptr + 1;

                    memFrag.ptr = (void*)(sptr + 1);
                    memFrag.len = ((eptr - 2) - sptr);
                    session->setAbstract(memFrag);

                }

                else if (!oscl_strncmp(line_start_ptr, ASMrulebook.c_str(),
                                       ASMrulebook.length()))
                {

                    sptr = skip_whitespace(line_start_ptr + ASMrulebook.length(),
                                           line_end_ptr);
                    if (sptr >= line_end_ptr)
                    {
                        break;
                        //            return SDP_BAD_SESSION_FORMAT;
                    }

                    eptr = skip_to_line_term(sptr, line_end_ptr);
                    if (eptr <= sptr)
                    {
                        return SDP_BAD_SESSION_FORMAT;
                    }

                    for (; *sptr != ';'; sptr++);
                    sptr = sptr + 1;

                    memFrag.ptr = (void*)sptr;
                    memFrag.len = (eptr - sptr);
                    session->setASMRuleBook(memFrag);

                }
                else if (!oscl_strncmp(line_start_ptr, app_specific_string.c_str(), app_specific_string.length()))
                {

                    sptr = skip_whitespace(line_start_ptr + app_specific_string.length(), line_end_ptr);

                    if (sptr >= line_end_ptr)
                    {
                        break;
                    }

                    eptr = skip_to_line_term(sptr, line_end_ptr);
                    if (eptr <= sptr)
                    {
                        return SDP_BAD_SESSION_FORMAT;
                    }

                    memFrag.ptr = (void*)sptr;
                    memFrag.len = (eptr - sptr);
                    session->setAppSpecificString(memFrag);
                }

                else if (!oscl_strncmp(line_start_ptr, pause_denied.c_str(),
                                       pause_denied.length()))
                {
                    session->setPauseDenied(true);
                }
                else if (!oscl_strncmp(line_start_ptr, qoe_metrics.c_str(),
                                       qoe_metrics.length()))
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
                        return SDP_BAD_SESSION_FORMAT;

                    session->setQoEMetrics(qMetrics);

                }
                else if (!oscl_strncmp(line_start_ptr, sdp_int_key.c_str(),
                                       sdp_int_key.length()))
                {
                    const char *sptr = line_start_ptr + sdp_int_key.length();
                    const char *eptr = sptr;
                    bool found_OMADRMv2_keyMethod = false;


                    sptr = skip_whitespace(sptr, line_end_ptr);
                    if (sptr > line_end_ptr)
                        return SDP_BAD_SESSION_FORMAT;


                    if (oscl_strncmp(sptr, "OMADRMv2:", oscl_strlen("OMADRMv2:")))
                    {

                        for (;*eptr != ':' ; ++eptr);
                        session->setKeyMethod(sptr, eptr - sptr);

                    }
                    else
                    {
                        found_OMADRMv2_keyMethod = true;
                        eptr = sptr + oscl_strlen("OMADRMv2:");
                        session->setKeyMethod(sptr, eptr - sptr);
                    }

                    sptr = eptr ;

                    sptr = skip_whitespace(sptr, line_end_ptr);

                    if (sptr > line_end_ptr)
                        return SDP_BAD_SESSION_FORMAT;
                    else if (sptr == line_end_ptr)
                        break;


                    if (!found_OMADRMv2_keyMethod)
                    {

                        session->setKeyData(sptr, line_end_ptr - sptr);
                        break;
                    }

                    eptr = sptr;
                    for (; *eptr != ','; ++eptr);

                    session->setKeyData(sptr, (eptr - sptr));

                    sptr = eptr + 1;

                    if (sptr > line_end_ptr)
                        return SDP_BAD_SESSION_FORMAT;

                    if (oscl_strncmp(sptr, "\"", 1))
                        return SDP_BAD_SESSION_FORMAT;
                    else
                        sptr = sptr + 1;

                    if (sptr > line_end_ptr)
                        return SDP_BAD_SESSION_FORMAT;

                    eptr = sptr;

                    for (; *eptr != '\"'; ++eptr);

                    session->setKeyContentIdURI(sptr , (eptr  - sptr));

                    sptr = eptr + 1;

                    if (sptr > line_end_ptr)
                        return SDP_BAD_SESSION_FORMAT;
                    if (oscl_strncmp(sptr, ",", 1))
                        return SDP_BAD_SESSION_FORMAT;

                    sptr = sptr + 1;

                    if (oscl_strncmp(sptr, "\"", 1))
                        return SDP_BAD_SESSION_FORMAT;
                    else
                        sptr = sptr + 1;

                    if (sptr > line_end_ptr)
                        return SDP_BAD_SESSION_FORMAT;

                    eptr = sptr;

                    for (; *eptr != '"'; ++eptr);

                    session->setKeyRightIssuerURI(sptr , ((eptr - 1) - sptr));

                    sptr = eptr + 1;
                    sptr = skip_whitespace(sptr, line_end_ptr);

                    if (sptr > line_end_ptr)
                        return SDP_BAD_SESSION_FORMAT;


                    session->setKeyFreshToken(sptr, (line_end_ptr - sptr));

                }
                else if (!oscl_strncmp(line_start_ptr, sdp_auth.c_str(),
                                       sdp_auth.length()))
                {
                    const char *sptr = line_start_ptr + sdp_auth.length();
                    memFrag.ptr = (void *)sptr;
                    memFrag.len = line_end_ptr - sptr;

                    session->setSdpAuthTag(memFrag);
                }
                else if (!oscl_strncmp(line_start_ptr, asset_info.c_str(),
                                       asset_info.length()))
                {
                    const char *sptr = line_start_ptr + asset_info.length();
                    AssetInfoType assetInfo;
                    if (!parseAssetInfo(sptr, line_end_ptr, assetInfo))
                        return SDP_BAD_SESSION_FORMAT;

                    session->setAssetInfo(assetInfo);

                }
                else if (!oscl_strncmp(line_start_ptr, alt_gp.c_str(),
                                       alt_gp.length()))
                {
                    const char *sptr = line_start_ptr + alt_gp.length();
                    sptr = skip_whitespace(sptr, line_end_ptr);

                    if (!oscl_strncmp(sptr, "BW", oscl_strlen("BW")))
                        session->setAltGroupBW(sptr, line_end_ptr - sptr);
                    else if (!oscl_strncmp(sptr, "LANG", oscl_strlen("LANG")))
                        session->setAltGroupLANG(sptr, line_end_ptr - sptr);
                    else
                        return SDP_BAD_SESSION_FORMAT;

                }
                if (!oscl_strncmp(line_start_ptr, "a=maxprate:", oscl_strlen("a=maxprate:")))
                {
                    line_start_ptr += oscl_strlen("a=maxprate:");
                    OsclFloat rate;
                    if (!PV_atof(line_start_ptr, line_end_ptr - line_start_ptr, rate))
                        return SDP_BAD_SESSION_FORMAT;
                    session->setMaxprate(rate);
                }

                break;
            }
            case 'e':
            {
                if (*(line_start_ptr + 1) != '=')
                {
                    return SDP_BAD_SESSION_FORMAT;
                }

                const char *sptr, *eptr;

                sptr = skip_whitespace(line_start_ptr + 2, line_end_ptr);
                if (sptr >= line_end_ptr)
                {
                    break;
                }

                eptr = skip_to_line_term(sptr, line_end_ptr);
                if (eptr <= sptr)
                {
                    return SDP_BAD_SESSION_FORMAT;
                }

                memFrag.ptr = (void*)sptr;
                memFrag.len = (eptr - sptr);
                session->setEmail(memFrag);
                break;
            }
            case 'p':
            {
                if (*(line_start_ptr + 1) != '=')
                {
                    return SDP_BAD_SESSION_FORMAT;
                }

                const char *sptr, *eptr;

                sptr = skip_whitespace(line_start_ptr + 2, line_end_ptr);
                if (sptr >= line_end_ptr)
                {
                    break;
                }

                eptr = skip_to_line_term(sptr, line_end_ptr);
                if (eptr <= sptr)
                {
                    return SDP_BAD_SESSION_FORMAT;
                }

                memFrag.ptr = (void*)sptr;
                memFrag.len = (eptr - sptr);
                session->setPhoneNumber(memFrag);
                break;
            }
        }
        current_start = line_end_ptr;
    }

    return SDP_SUCCESS;
}
