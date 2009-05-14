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
#include "oscl_snprintf.h"
#include "rtsp_par_com.h"
#include "rtsp_range_utils.h"
#include "oscl_time.h"



OSCL_EXPORT_REF void
RTSPOutgoingMessage::reset()
{
    RTSPGenericMessage::reset();

    fullRequestBufferSizeUsed = 0;
    fullRequestBufferSpace = fullRequestBuffer;

    boundMessage = NULL;
}

OSCL_EXPORT_REF StrPtrLen *
RTSPOutgoingMessage::retrieveComposedBuffer()
{
    return &fullRequestPLS;
}

OSCL_EXPORT_REF bool
RTSPOutgoingMessage::addField(
    StrCSumPtrLen * newFieldName,
    const StrPtrLen *    newFieldValue
)
{
    StrPtrLen * fieldVal = const_cast<StrPtrLen *>(queryField(* newFieldName));

    uint32  extraSize;

    // check if this field already exists in the message
    //
    if (NULL == fieldVal)
    { // this field is new to the message

        // check that there are enough pointers
        if (RTSP_MAX_NUMBER_OF_FIELDS == numPtrFields)
        {
            return false;
        }

        // check for the extra size
        extraSize = newFieldName->length() + newFieldValue->length() + 2;
        if (RTSP_MAX_FULL_REQUEST_SIZE < secondaryBufferSizeUsed + extraSize)
        {
            return false;
        }

        // oscl_memcpy is unsafe for overlaps, but source and target memory come from
        // different sources
        //
        oscl_memcpy(secondaryBufferSpace, newFieldName->c_str(),
                    newFieldName->length() + 1);
        oscl_memcpy(secondaryBufferSpace + newFieldName->length() + 1,
                    newFieldValue->c_str(), newFieldValue->length() + 1);

        // save the incoming structures, but reset pointers to their new home in the
        // secondary buffer

        fieldKeys[ numPtrFields ].setPtrLen(secondaryBufferSpace,
                                            newFieldName->length());
        fieldVals[ numPtrFields ].setPtrLen(
            secondaryBufferSpace + newFieldName->length() + 1,
            newFieldValue->length()
        );

        // pop up the number of used pointers
        //
        ++ numPtrFields;

        // do buffer accounting
        //
    }
    else
    { // this field is known to the message, we just have to replace its value

        // check for the extra size
        extraSize = newFieldValue->length() + 1;
        if (RTSP_MAX_FULL_REQUEST_SIZE < secondaryBufferSizeUsed + extraSize)
        {
            return false;
        }

        // oscl_memcpy is unsafe for overlaps, but source and target memory come from
        // different sources
        //
        oscl_memcpy(secondaryBufferSpace, newFieldValue->c_str(),
                    newFieldValue->length() + 1);

        // save the incoming structures, but reset pointers to their new home in the
        // secondary buffer

        fieldVal->setPtrLen(secondaryBufferSpace, newFieldValue->length());
    }

    secondaryBufferSizeUsed += extraSize;
    secondaryBufferSpace = secondaryBuffer + secondaryBufferSizeUsed;

    return true;
}

OSCL_EXPORT_REF bool
RTSPOutgoingMessage::addField(
    StrCSumPtrLen * newFieldName,
    const char *      newValue
)
{
    StrPtrLen valuePLS(newValue);

    return addField(newFieldName, &valuePLS);
}

OSCL_EXPORT_REF bool
RTSPOutgoingMessage::compose()
{



    // compose the first line
    //
    switch (msgType)
    {
        case RTSPResponseMsg:

        {
            // RTSP version
            //
            oscl_memcpy(fullRequestBuffer, RTSPVersionString, RTSPVersionString_len);
            fullRequestBufferSpace += RTSPVersionString_len;

            *(fullRequestBufferSpace++) = ' ';


            // Status code
            //
            oscl_snprintf(fullRequestBufferSpace, RTSP_MAX_FULL_REQUEST_SIZE - 1, "%d", statusCode);

            // resync the pointer and size used
            fullRequestBufferSizeUsed = oscl_strlen(fullRequestBufferSpace);
            fullRequestBufferSpace += fullRequestBufferSizeUsed;
            *(fullRequestBufferSpace++) = ' ';
            fullRequestBufferSizeUsed += 1 + RTSPVersionString_len + 1;

            if (0 != reasonString.length())
            {
                // user specified his own string
                //
                oscl_memcpy(fullRequestBufferSpace, reasonString.c_str(),
                            reasonString.length());

                fullRequestBufferSpace += reasonString.length();
                fullRequestBufferSizeUsed += reasonString.length();
            }
            else
            {
                StrPtrLen  realReasonString ;

                // user wants the built-in default
                //
                switch (statusCode)
                {
                    case CodeContinue:

                        realReasonString =  RtspReasonStringContinue;

                        break;

                    case CodeOK:

                        realReasonString =  RtspReasonStringOK;

                        break;

                    case CodeCreated:
                        realReasonString =  RtspReasonStringCreated;

                        break;

                    case CodeLowOnStorageSpace:
                        realReasonString =  RtspReasonStringLowOnStorageSpace;

                        break;

                    case CodeMultipleChoices:
                        realReasonString =  RtspReasonStringMultipleChoices;

                        break;

                    case CodeMovedPermanently:
                        realReasonString =  RtspReasonStringMovedPermanently;

                        break;

                    case CodeMovedTemporarily:
                        realReasonString =  RtspReasonStringMovedTemporarily;

                        break;

                    case CodeSeeOther:
                        realReasonString =  RtspReasonStringSeeOther;

                        break;

                    case CodeNotModified:
                        realReasonString =  RtspReasonStringNotModified;

                        break;

                    case CodeUseProxy:
                        realReasonString =  RtspReasonStringUseProxy;

                        break;

                    case CodeBadRequest:
                        realReasonString =  RtspReasonStringBadRequest;

                        break;

                    case CodeUnauthorized:
                        realReasonString =  RtspReasonStringUnauthorized;

                        break;

                    case CodePaymentRequired:
                        realReasonString =  RtspReasonStringPaymentRequired;

                        break;

                    case CodeForbidden:
                        realReasonString =  RtspReasonStringForbidden;

                        break;

                    case CodeNotFound:
                        realReasonString =  RtspReasonStringNotFound;

                        break;

                    case CodeMethodNotAllowed:
                        realReasonString =  RtspReasonStringMethodNotAllowed;

                        break;

                    case CodeNotAcceptable:
                        realReasonString =  RtspReasonStringNotAcceptable;

                        break;

                    case CodeProxyAuthenticationRequired:
                        realReasonString =
                            RtspReasonStringProxyAuthenticationRequired;

                        break;

                    case CodeRequestTimeOut:
                        realReasonString =  RtspReasonStringRequestTimeOut;

                        break;

                    case CodeGone:
                        realReasonString =  RtspReasonStringGone;

                        break;

                    case CodeLengthRequired:
                        realReasonString =  RtspReasonStringLengthRequired;

                        break;

                    case CodePreconditionFailed:
                        realReasonString =  RtspReasonStringPreconditionFailed;

                        break;

                    case CodeRequestEntityTooLarge:
                        realReasonString =
                            RtspReasonStringRequestEntityTooLarge;

                        break;

                    case CodeRequestURITooLarge:
                        realReasonString =  RtspReasonStringRequestURITooLarge;

                        break;

                    case CodeUnsupportedMediaType:
                        realReasonString =  RtspReasonStringUnsupportedMediaType;

                        break;

                    case CodeSessionNotFound:
                        realReasonString =  RtspReasonStringSessionNotFound;

                        break;

                    case CodeMethodNotValidInThisState:
                        realReasonString =  RtspReasonStringMethodNotValidInThisState;

                        break;

                    case CodeHeaderFieldNotValidForResource:
                        realReasonString =
                            RtspReasonStringHeaderFieldNotValidForResource;

                        break;

                    case CodeInvalidRange:
                        realReasonString =  RtspReasonStringInvalidRange;

                        break;

                    case CodeParameterIsReadOnly:
                        realReasonString =  RtspReasonStringParameterIsReadOnly;

                        break;

                    case CodeAggregateOperationNotAllowed:
                        realReasonString =
                            RtspReasonStringAggregateOperationNotAllowed;

                        break;

                    case CodeOnlyAggregateOperationAllowed:
                        realReasonString =
                            RtspReasonStringOnlyAggregateOperationAllowed;

                        break;

                    case CodeUnsupportedTransport:
                        realReasonString =  RtspReasonStringUnsupportedTransport;

                        break;

                    case CodeDestinationUnreachable:
                        realReasonString =  RtspReasonStringDestinationUnreachable;

                        break;

                    case CodeUnsupportedClient:
                        realReasonString =  RtspReasonStringUnsupportedClient;

                        break;

                    case CodeInternalServerError:
                        realReasonString =  RtspReasonStringInternalServerError;

                        break;

                    case CodeNotImplemented:
                        realReasonString =  RtspReasonStringNotImplemented;

                        break;

                    case CodeBadGateway:
                        realReasonString =  RtspReasonStringBadGateway;

                        break;

                    case CodeServiceUnavailable:
                        realReasonString =  RtspReasonStringServiceUnavailable;

                        break;

                    case CodeGatewayTimeout:
                        realReasonString =  RtspReasonStringGatewayTimeout;

                        break;

                    case CodeRTSPVersionNotSupported:
                        realReasonString =  RtspReasonStringRTSPVersionNotSupported;

                        break;

                    case CodeOptionNotSupported:
                        realReasonString =  RtspReasonStringOptionNotSupported;

                        break;

                    case CodeParameterNotUnderstood:
                        realReasonString =  RtspReasonStringParameterNotUnderstood;

                        break;

                    default:
                        // no string was found, since code is unknown...
                        ;;;
                }

                if (realReasonString.length())
                {
                    oscl_memcpy(fullRequestBufferSpace, realReasonString.c_str(), realReasonString.length());

                    fullRequestBufferSpace += realReasonString.length();
                    fullRequestBufferSizeUsed += realReasonString.length();
                }
            }

            break;
        }


        case RTSPRequestMsg:
        {
            if (METHOD_BINARY_DATA == method)
            { // it's interleaved stuff

                // leading dollar
                *(fullRequestBufferSpace++) = CHAR_DOLLAR;

                // 8-bit channel-id from the content-type
                *(fullRequestBufferSpace++) = contentType.c_str()[0];

                // 16-bit content length, in network byte order
                *(fullRequestBufferSpace++) = char((contentLength & 0xFF00) >> 8);
                *(fullRequestBufferSpace++) = char((contentLength & 0xFF));

                *fullRequestBufferSpace = CHAR_NULL;

                fullRequestBufferSizeUsed = 4;

                fullRequestPLS = fullRequestBuffer;

                return true;
            }

            // okay, it's a normal request

            // do the method
            if (method >= METHOD_NUM_ENTRIES)
            { // method unknown
                return false;
            }

            uint32 method_strlen = oscl_strlen(RtspMethodStringPLSS[method]);
            oscl_memcpy(fullRequestBufferSpace,
                        RtspMethodStringPLSS[method],
                        method_strlen
                       );
            fullRequestBufferSpace += method_strlen;
            *(fullRequestBufferSpace++) = CHAR_SPACE;
            fullRequestBufferSizeUsed += method_strlen + 1;

            // do the URI

            oscl_memcpy(fullRequestBufferSpace,
                        originalURI.c_str(),
                        originalURI.length()
                       );
            fullRequestBufferSpace += originalURI.length();
            *(fullRequestBufferSpace++) = CHAR_SPACE;
            fullRequestBufferSizeUsed += originalURI.length() + 1;

            // do the RTSP version
#ifdef SIMPLE_HTTP_SUPPORT
            if ((method == METHOD_GET) || (method == METHOD_POST))
            {
                oscl_memcpy(fullRequestBufferSpace,
                            HTTPVersion_1_0_String,
                            HTTPVersionString_len
                           );
            }
            else
#endif
                oscl_memcpy(fullRequestBufferSpace,
                            RTSPVersionString,
                            RTSPVersionString_len
                           );
            fullRequestBufferSpace += RTSPVersionString_len;
            fullRequestBufferSizeUsed += RTSPVersionString_len;

            break;
        }

        default:
        {	// cannot encode an unknown type of message

            return false;
        }
    }


    // add the final newline to the first string
    *(fullRequestBufferSpace++) = CHAR_CR;
    *(fullRequestBufferSpace++) = CHAR_LF;
    fullRequestBufferSizeUsed += 2;


    // add the shortcut fields

    if ((method != METHOD_GET) && (method != METHOD_POST))
    {
        if (cseqIsSet)
        {
            // The Symbian version of oscl_snprintf does not support %ld format, should use %d or %u instead.
            // Since cseq is an uint32 it's ok to use %u.
            oscl_snprintf(fullRequestBufferSpace, RTSP_MAX_FULL_REQUEST_SIZE - 1, "CSeq: %u%c%c",
                          cseq, CHAR_CR, CHAR_LF);
            int addSize = oscl_strlen(fullRequestBufferSpace);
            fullRequestBufferSizeUsed += addSize;
            fullRequestBufferSpace += addSize;
        }
        else
        {
            oscl_snprintf(fullRequestBufferSpace, RTSP_MAX_FULL_REQUEST_SIZE - 1, "CSeq: %c%c",
                          CHAR_CR, CHAR_LF);
            int addSize = 8;
            fullRequestBufferSizeUsed += addSize;
            fullRequestBufferSpace += addSize;
        }
    }

    if (RTSPResponseMsg == msgType)
    {
        TimeValue current_time;
        const int DATE_BUFSIZE = 29;
        char tmp[DATE_BUFSIZE+1];

        int max_len =  RTSP_MAX_FULL_REQUEST_SIZE -
                       fullRequestBufferSizeUsed;
        if (max_len < DATE_BUFSIZE + 8)
        {
            // not enough room ("8" represents the size of Date: CRLF)
            return false;
        }
        current_time.get_rfc822_gmtime_str(DATE_BUFSIZE + 1, tmp);
        if (tmp[0] != '\0')
        {
            // date string is not empty
            oscl_snprintf(fullRequestBufferSpace, RTSP_MAX_FULL_REQUEST_SIZE - 1, "Date: %s%c%c",
                          tmp, CHAR_CR, CHAR_LF);
            int addSize = oscl_strlen(fullRequestBufferSpace);
            fullRequestBufferSizeUsed += addSize;
            fullRequestBufferSpace += addSize;
        }

    }

    if (bufferSizeIsSet)
    {
        oscl_snprintf(fullRequestBufferSpace, RTSP_MAX_FULL_REQUEST_SIZE - 1, "Buffersize: %u%c%c",
                      bufferSize, CHAR_CR, CHAR_LF);
        int addSize = oscl_strlen(fullRequestBufferSpace);
        fullRequestBufferSizeUsed += addSize;
        fullRequestBufferSpace += addSize;
    }

    if (sessionIdIsSet)
    {
        oscl_snprintf(fullRequestBufferSpace, RTSP_MAX_FULL_REQUEST_SIZE - 1, "Session: %s%c%c",
                      sessionId.c_str(), CHAR_CR, CHAR_LF);
        int addSize = oscl_strlen(fullRequestBufferSpace);
        fullRequestBufferSizeUsed += addSize;
        fullRequestBufferSpace += addSize;
    }

    /*
    #ifdef RTSP_PLAYLIST_SUPPORT
    // hard code these for now, later should have a general supported field build that adds on entries as needed
    if(methodEosIsSet)
    {
    	if(comPvServerPlaylistIsSet)
    	{
    		oscl_snprintf( fullRequestBufferSpace, RTSP_MAX_FULL_REQUEST_SIZE-1,
    			"Supported: com.pv.server_playlist,method.eos%c%c",
    			CHAR_CR, CHAR_LF );
    	}
    	else
    	{
    		oscl_snprintf( fullRequestBufferSpace, RTSP_MAX_FULL_REQUEST_SIZE-1,
    			"Supported: method.eos%c%c",
    			CHAR_CR, CHAR_LF );
    	}
    }
    int addSize = oscl_strlen( fullRequestBufferSpace );
    fullRequestBufferSizeUsed += addSize;
    fullRequestBufferSpace += addSize;
    #endif
    */
    if (userAgentIsSet)
    {
        oscl_snprintf(fullRequestBufferSpace, RTSP_MAX_FULL_REQUEST_SIZE - 1, "User-Agent: %s%c%c",
                      userAgent.c_str(), CHAR_CR, CHAR_LF);
        int addSize = oscl_strlen(fullRequestBufferSpace);
        fullRequestBufferSizeUsed += addSize;
        fullRequestBufferSpace += addSize;
    }

    if (acceptIsSet)
    {
        oscl_snprintf(fullRequestBufferSpace, RTSP_MAX_FULL_REQUEST_SIZE - 1, "Accept: %s%c%c",
                      accept.c_str(), CHAR_CR, CHAR_LF);
        int addSize = oscl_strlen(fullRequestBufferSpace);
        fullRequestBufferSizeUsed += addSize;
        fullRequestBufferSpace += addSize;
    }

    if (requireIsSet)
    {
        oscl_snprintf(fullRequestBufferSpace, RTSP_MAX_FULL_REQUEST_SIZE - 1, "Require: %s%c%c",
                      require.c_str(), CHAR_CR, CHAR_LF);
        int addSize = oscl_strlen(fullRequestBufferSpace);
        fullRequestBufferSizeUsed += addSize;
        fullRequestBufferSpace += addSize;
    }

    if (contentTypeIsSet)
    {
        oscl_snprintf(fullRequestBufferSpace, RTSP_MAX_FULL_REQUEST_SIZE - 1, "Content-Type: %s%c%c",
                      contentType.c_str(), CHAR_CR, CHAR_LF);
        int addSize = oscl_strlen(fullRequestBufferSpace);
        fullRequestBufferSizeUsed += addSize;
        fullRequestBufferSpace += addSize;
    }

    if (contentBaseMode != NO_CONTENT_BASE)
    {
        if (contentBaseMode == INCLUDE_TRAILING_SLASH &&
                *(contentBase.c_str() + oscl_strlen(contentBase.c_str()) - 1) != CHAR_SLASH)
        {
            oscl_snprintf(fullRequestBufferSpace, RTSP_MAX_FULL_REQUEST_SIZE - 1, "Content-Base: %s%c%c%c",
                          contentBase.c_str(), CHAR_SLASH, CHAR_CR, CHAR_LF);
        }
        else
        {
            oscl_snprintf(fullRequestBufferSpace, RTSP_MAX_FULL_REQUEST_SIZE - 1, "Content-Base: %s%c%c",
                          contentBase.c_str(), CHAR_CR, CHAR_LF);
        }
        int addSize = oscl_strlen(fullRequestBufferSpace);
        fullRequestBufferSizeUsed += addSize;
        fullRequestBufferSpace += addSize;
    }

    if (contentLengthIsSet)
    {
        oscl_snprintf(fullRequestBufferSpace, RTSP_MAX_FULL_REQUEST_SIZE - 1, "Content-Length: %u%c%c",
                      contentLength, CHAR_CR, CHAR_LF);
        int addSize = oscl_strlen(fullRequestBufferSpace);
        fullRequestBufferSizeUsed += addSize;
        fullRequestBufferSpace += addSize;
    }

    if (numOfTransportEntries)
    {
        oscl_strncpy(fullRequestBufferSpace, "Transport: ", 11);
        fullRequestBufferSpace[11] = NULL_TERM_CHAR;
        int addSize = oscl_strlen(fullRequestBufferSpace);
        fullRequestBufferSizeUsed += addSize;
        fullRequestBufferSpace += addSize;

        for (uint ii = 0; ii < numOfTransportEntries; ++ii)
        {
            composeTransport(fullRequestBufferSpace, transport + ii);
            int addSize = oscl_strlen(fullRequestBufferSpace);
            fullRequestBufferSizeUsed += addSize;
            fullRequestBufferSpace += addSize;

            if (ii < numOfTransportEntries - 1)
            {
                oscl_snprintf(fullRequestBufferSpace, RTSP_MAX_FULL_REQUEST_SIZE - 1, ",");
                fullRequestBufferSizeUsed += 1;
                fullRequestBufferSpace += 1;
            }
        }

        oscl_snprintf(fullRequestBufferSpace, RTSP_MAX_FULL_REQUEST_SIZE - 1, "%c%c", CHAR_CR, CHAR_LF);
        fullRequestBufferSizeUsed += 2;
        fullRequestBufferSpace += 2;

    }

    if (rangeIsSet)
    {
        int addSize;
        unsigned int max_len = RTSP_MAX_FULL_REQUEST_SIZE -
                               fullRequestBufferSizeUsed;
        if (!compose_RTSP_string(fullRequestBufferSpace,
                                 max_len,
                                 range, addSize))
        {
            return false;
        }
        fullRequestBufferSizeUsed += addSize;
        fullRequestBufferSpace += addSize;
        // put a null terminator on the end
        *fullRequestBufferSpace = '\0';

    }

    // compose the RTP Info
    //
    if (numOfRtpInfoEntries)
    {
        oscl_strncpy(fullRequestBufferSpace, "RTP-Info: ", 10);
        fullRequestBufferSpace[10] = NULL_TERM_CHAR;
        int addSize = oscl_strlen(fullRequestBufferSpace);
        fullRequestBufferSizeUsed += addSize;
        fullRequestBufferSpace += addSize;
        for (uint32 ii = 0; ii < numOfRtpInfoEntries; ++ii)
        {
            bool somethingIsPresent = false;

            if (ii)
            {
                // put some leading whitespace on the line
                oscl_strncpy(fullRequestBufferSpace, "  ", 2);
                fullRequestBufferSpace[2] = NULL_TERM_CHAR;
                fullRequestBufferSizeUsed += 2;
                fullRequestBufferSpace += 2;
            }

            int addSize = oscl_strlen(fullRequestBufferSpace);
            fullRequestBufferSizeUsed += addSize;
            fullRequestBufferSpace += addSize;
            if (rtpInfo[ii].urlIsSet)
            {
                oscl_snprintf(fullRequestBufferSpace, RTSP_MAX_FULL_REQUEST_SIZE - 1, "url=%s", rtpInfo[ii].url.c_str());
                int addSize = oscl_strlen(fullRequestBufferSpace);
                fullRequestBufferSizeUsed += addSize;
                fullRequestBufferSpace += addSize;
                somethingIsPresent = true;
            }
            if (rtpInfo[ii].seqIsSet)
            {
                if (somethingIsPresent)
                {
                    oscl_snprintf(fullRequestBufferSpace, RTSP_MAX_FULL_REQUEST_SIZE - 1, ";seq=%d", rtpInfo[ii].seq);
                }
                else
                {
                    oscl_snprintf(fullRequestBufferSpace, RTSP_MAX_FULL_REQUEST_SIZE - 1, "seq=%d", rtpInfo[ii].seq);
                }
                int addSize = oscl_strlen(fullRequestBufferSpace);
                fullRequestBufferSizeUsed += addSize;
                fullRequestBufferSpace += addSize;
                somethingIsPresent = true;
            }
            if (rtpInfo[ii].rtptimeIsSet)
            {
                if (somethingIsPresent)
                {
                    oscl_snprintf(fullRequestBufferSpace, RTSP_MAX_FULL_REQUEST_SIZE - 1, ";rtptime=%u", rtpInfo[ii].rtptime);
                }
                else
                {
                    oscl_snprintf(fullRequestBufferSpace, RTSP_MAX_FULL_REQUEST_SIZE - 1, "rtptime=%u", rtpInfo[ii].rtptime);
                }
                int addSize = oscl_strlen(fullRequestBufferSpace);
                fullRequestBufferSizeUsed += addSize;
                fullRequestBufferSpace += addSize;
                somethingIsPresent = true;
            }

            if (ii < numOfRtpInfoEntries - 1)
            {
                oscl_snprintf(fullRequestBufferSpace, RTSP_MAX_FULL_REQUEST_SIZE - 1, ",%c%c", CHAR_CR, CHAR_LF);
                fullRequestBufferSizeUsed += 3;
                fullRequestBufferSpace += 3;
            }


        }
        oscl_snprintf(fullRequestBufferSpace, RTSP_MAX_FULL_REQUEST_SIZE - 1, "%c%c", CHAR_CR, CHAR_LF);
        fullRequestBufferSizeUsed += 2;
        fullRequestBufferSpace += 2;

    }

    // add the outstanding fields

    for (uint32 jj = 0; jj < numPtrFields; ++jj)
    {
        oscl_snprintf(fullRequestBufferSpace, RTSP_MAX_FULL_REQUEST_SIZE - 1, "%s: %s%c%c",
                      fieldKeys[ jj ].c_str(), fieldVals[ jj ].c_str(), CHAR_CR, CHAR_LF);

        int addSize = oscl_strlen(fullRequestBufferSpace);
        fullRequestBufferSizeUsed += addSize;
        fullRequestBufferSpace += addSize;
    }

#ifdef ASF_STREAMING
    if (XMLIsSet)
    {
        // now, add a newline before the XML message
        *(fullRequestBufferSpace++) = CHAR_CR;
        *(fullRequestBufferSpace++) = CHAR_LF;
        fullRequestBufferSizeUsed += 2;

        int addSize = strlen(XMLBufferPtr);
        oscl_memcpy(fullRequestBufferSpace, XMLBufferPtr, addSize);
        fullRequestBufferSizeUsed += addSize;
        fullRequestBufferSpace += addSize;
    }

#endif

    // now, add the final newline to the whole message
    *(fullRequestBufferSpace++) = CHAR_CR;
    *(fullRequestBufferSpace++) = CHAR_LF;
    fullRequestBufferSizeUsed += 2;

    *(fullRequestBufferSpace) = CHAR_NULL;

    // finally, set up to return

    fullRequestPLS = fullRequestBuffer;

    return true;
}

OSCL_EXPORT_REF void
RTSPOutgoingMessage::bind(const RTSPIncomingMessage & incoming)
{

    StrCSumPtrLen timeStampName("Timestamp");


    cseqIsSet = incoming.cseqIsSet;
    cseq = incoming.cseq;

    sessionIdIsSet = incoming.sessionIdIsSet;
    sessionId = incoming.sessionId;

    const StrPtrLen * timeStampVal;
    if (NULL != (timeStampVal = incoming.queryField(timeStampName)))
    {
        addField(&timeStampName, timeStampVal);
    }
}

void
RTSPOutgoingMessage::composeTransport(char* trans, RtspTransport* rtspTrans)
{
    const int tmp_size = 64;
    char tmp[tmp_size];
    if (rtspTrans->protocolIsSet)
    {
        if (rtspTrans->protocol == RtspTransport::RTP_PROTOCOL)
        {
            oscl_strcat(trans, "RTP");
        }
        else if (rtspTrans->protocol == RtspTransport::RDT_PROTOCOL)
        {
            oscl_strcat(trans, "x-pn-tng");
        }
    }
    if (rtspTrans->profileIsSet)
    {
        if (rtspTrans->profile == RtspTransport::AVP_PROFILE)
        {
            oscl_strcat(trans, "/AVP");
        }
        else if (rtspTrans->profile == RtspTransport::TCP_PROFILE)
        {//Real
            oscl_strcat(trans, "/tcp");
        }
    }
    if (rtspTrans->transportTypeIsSet)
    {
        if (rtspTrans->transportType == RtspTransport::UDP_TRANSPORT)
        {
            oscl_strcat(trans, "/UDP");
        }
        else if (rtspTrans->transportType == RtspTransport::TCP_TRANSPORT)
        {
            oscl_strcat(trans, "/TCP");
        }
    }
    if (rtspTrans->deliveryIsSet)
    {
        if (rtspTrans->delivery == RtspTransport::UNICAST_DELIVERY)
        {
            oscl_strcat(trans, ";unicast");
        }
        if (rtspTrans->delivery == RtspTransport::MULTICAST_DELIVERY)
        {
            oscl_strcat(trans, ";multicast");
        }
    }

    if (rtspTrans->destinationIsSet)
    {
        if (rtspTrans->destination.c_str() && rtspTrans->destination.length())
        {
            oscl_strcat(trans, rtspTrans->destination.c_str());
        }
    }

    if (rtspTrans->channelIsSet)
    {
        oscl_snprintf(tmp, 63, ";interleaved=%d-%d", rtspTrans->channel1,
                      rtspTrans->channel2);
        oscl_strcat(trans, tmp);
    }

    if (rtspTrans->client_portIsSet)
#ifdef ASF_STREAMING
        if (rtspTrans->client_port2 == NULL)
        {
            oscl_snprintf(tmp, 63, ";client_port=%d", rtspTrans->client_port1);
            oscl_strcat(trans, tmp);
        }
        else
#endif
        {
            oscl_snprintf(tmp, 63, ";client_port=%d-%d", rtspTrans->client_port1, rtspTrans->client_port2);
            oscl_strcat(trans, tmp);
        }
    if (rtspTrans->server_portIsSet)
    {
        oscl_snprintf(tmp, 63, ";server_port=%d-%d", rtspTrans->server_port1, rtspTrans->server_port2);
        oscl_strcat(trans, tmp);
    }
    if (rtspTrans->modeIsSet)
    {
        if (rtspTrans->mode.play_mode)
        {
            oscl_strncpy(tmp, ";mode=play", 9);
            tmp[9] = NULL_TERM_CHAR;
            oscl_strcat(trans, tmp);
        }
        if (rtspTrans->mode.record_mode)
        {
            oscl_strncpy(tmp, ";mode=record", 11);
            tmp[11] = NULL_TERM_CHAR;
            oscl_strcat(trans, tmp);
            if (rtspTrans->append)
            {
                oscl_strncpy(tmp, ";append", 6);
                tmp[6] = NULL_TERM_CHAR;
                oscl_strcat(trans, tmp);
            }
        }
    }
    if (rtspTrans->ttlIsSet)
    {
        oscl_snprintf(tmp, 63, ";ttl=%d", rtspTrans->ttl);
        oscl_strcat(trans, tmp);
    }
    if (rtspTrans->layersIsSet)
    {
        oscl_snprintf(tmp, 63, ";layers=%u", rtspTrans->layers);
        oscl_strcat(trans, tmp);
    }
    if (rtspTrans->ssrcIsSet)
    {
        oscl_snprintf(tmp, 63, ";ssrc=%.8x", rtspTrans->ssrc);
        oscl_strcat(trans, tmp);
    }


}


