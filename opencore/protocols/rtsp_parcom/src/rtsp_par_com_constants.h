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

#ifndef RTSP_PAR_COM_CONSTANTS_H_
#define RTSP_PAR_COM_CONSTANTS_H_



#include "oscl_str_ptr_len.h"
#include "rtsp_par_com_basic_ds.h"


//An array of strings corresponding to RTSPMethod enum
static const char* const RtspMethodStringPLSS[] =
{
    ""  //METHOD_UNRECOGNIZED
    , "DESCRIBE" //METHOD_DESCRIBE
    , "GET_PARAMETER" //METHOD_GET_PARAMETER
    , "OPTIONS" //METHOD_OPTIONS
    , "PAUSE" //METHOD_PAUSE
    , "PLAY" //METHOD_PLAY
    , "SETUP" //METHOD_SETUP
    , "RECORD" //METHOD_RECORD
    , "TEARDOWN" //METHOD_TEARDOWN
    , "END_OF_STREAM" //METHOD_END_OF_STREAM
    , "$" //METHOD_BINARY_DATA
    , "REDIRECT" // METHOD_REDIRECT
    , "SET_PARAMETER" //METHOD_SET_PARAMETER
#ifdef SIMPLE_HTTP_SUPPORT
    , "GET" //METHOD_GET
    , "POST" //METHOD_POST
#endif
#ifdef RTSP_PLAYLIST_SUPPORT
    , "PLAYLIST_PLAY" //METHOD_PLAYLIST_PLAY
#endif
};

#define RtspReasonStringContinue "Continue"
#define RtspReasonStringOK "OK"
#define RtspReasonStringCreated "Created"
#define RtspReasonStringLowOnStorageSpace "Low on Storage Space"
#define RtspReasonStringMultipleChoices "Multiple Choices"
#define RtspReasonStringMovedPermanently "Moved Permanently"
#define RtspReasonStringMovedTemporarily "Moved Temporarily"
#define RtspReasonStringSeeOther "See Other"
#define RtspReasonStringNotModified "Not Modified"
#define RtspReasonStringUseProxy "Use Proxy"
#define RtspReasonStringBadRequest "Bad Request"
#define RtspReasonStringUnauthorized "Unauthorized"
#define RtspReasonStringPaymentRequired "Payment Required"
#define RtspReasonStringForbidden "Forbidden"
#define RtspReasonStringNotFound "Not Found"
#define RtspReasonStringMethodNotAllowed "Method Not Allowed"
#define RtspReasonStringNotAcceptable "Not Acceptable"
#define RtspReasonStringProxyAuthenticationRequired "Proxy Authentication Required"
#define RtspReasonStringRequestTimeOut "Request Time-out"
#define RtspReasonStringGone "Gone"
#define RtspReasonStringLengthRequired "Length Required"
#define RtspReasonStringPreconditionFailed "Precondition Failed"
#define RtspReasonStringRequestEntityTooLarge "Request Entity Too Large"
#define RtspReasonStringRequestURITooLarge "Request-URI Too Large"
#define RtspReasonStringUnsupportedMediaType "Unsupported Media Type"
#define RtspReasonStringParameterNotUnderstood "Parameter Not Understood"
#define RtspReasonStringConferenceNotFound "Conference Not Found"
#define RtspReasonStringNotEnoughBandwidth "Not Enough Bandwidth"
#define RtspReasonStringSessionNotFound "Session Not Found"
#define RtspReasonStringMethodNotValidInThisState "Method Not Valid In This State"
#define RtspReasonStringHeaderFieldNotValidForResource "Header Field Not Valid For Resource"
#define RtspReasonStringInvalidRange "Invalid Range"
#define RtspReasonStringParameterIsReadOnly "Parameter Is Read-Only"
#define RtspReasonStringAggregateOperationNotAllowed "Aggregate Operation Not Allowed"
#define RtspReasonStringOnlyAggregateOperationAllowed "Only Aggregate Operation Allowed"
#define RtspReasonStringUnsupportedTransport "Unsupported Transport"
#define RtspReasonStringDestinationUnreachable "Destination Unreachable"
#define RtspReasonStringUnsupportedClient "Unsupported Client"
#define RtspReasonStringInternalServerError "Internal Server Error"
#define RtspReasonStringNotImplemented "Not Implemented"
#define RtspReasonStringBadGateway "Bad Gateway"
#define RtspReasonStringServiceUnavailable "Service Unavailable"
#define RtspReasonStringGatewayTimeout "Gateway Timeout"
#define RtspReasonStringRTSPVersionNotSupported "RTSP Version Not Supported"
#define RtspReasonStringOptionNotSupported "Option Not Supported"

#define RtspRequestMethodStringDescribe "DESCRIBE"
#define RtspRequestMethodStringGetParameter "GET_PARAMETER"
#define RtspRequestMethodStringSetParameter "SET_PARAMETER"
#define RtspRequestMethodStringRedirect "REDIRECT"
#define RtspRequestMethodStringOptions "OPTIONS"
#define RtspRequestMethodStringPause "PAUSE"
#define RtspRequestMethodStringPlay "PLAY"
#define RtspRequestMethodStringSetup "SETUP"
#define RtspRequestMethodStringRecord "RECORD"
#define RtspRequestMethodStringTeardown "TEARDOWN"
#define RtspRequestMethodStringEndOfStream "END_OF_STREAM"
#define RtspRequestMethodStringBinaryData "$"

#ifdef RTSP_PLAYLIST_SUPPORT
#define RtspRequestMethodStringPlaylistPlay "PLAYLIST_PLAY"
#endif

#define	RTSPVersionString  "RTSP/1.0"
#define	RTSPVersionString_len  8

#define	HTTPVersion_1_0_String  "HTTP/1.0"
#define	HTTPVersion_1_1_String  "HTTP/1.1"
#define	HTTPVersionString_len  8

const char CHAR_CR 		  = 13;
const char CHAR_LF 		  = 10;
const char CHAR_NULL 	  = 0;
const char CHAR_COLON	  =	':';
const char CHAR_DOLLAR  = '$';
const char CHAR_SPACE   = ' ';
const char CHAR_SLASH   = '/';
const char CHAR_SEMICOLON = ';';
const char CHAR_STAR    = '*';
const char CHAR_PLUS    = '+';
const char CHAR_MINUS   = '-';
const char CHAR_DOT     = '.';
const char CHAR_TAB     = '\t';
const char CHAR_COMMA   = ',';
const char CHAR_EQUAL   = '=';
const char CHAR_LT   = '<';
const char CHAR_GT   = '>';

#define RtspRecognizedFieldSessionId "Session"
#define RtspRecognizedFieldCSeq "CSeq"
#define RtspRecognizedFieldContentBase "Content-Base"
#define RtspRecognizedFieldContentType "Content-Type"
#define RtspRecognizedFieldContentLength "Content-Length"
#define RtspRecognizedFieldUserAgent "User-Agent"
#define RtspRecognizedFieldAccept "Accept"
#define RtspRecognizedFieldRequire "Require"
#define RtspRecognizedFieldRange "Range"
#define RtspRecognizedFieldTransport "Transport"
#define RtspRecognizedFieldRTPInfo "RTP-Info"
#define RtspRecognizedFieldBufferSize "Buffersize"
#define RtspRecognizedFieldSupported "Supported"

#ifdef RTSP_PLAYLIST_SUPPORT
#define RtspRecognizedFieldPlaylistRange "playlist_range"
#define RtspRecognizedFieldPlaylistError "playlist_error"
//#define RtspPlaylistPlayTimeStr "playlist_play_time"
#endif


#endif // RTSP_PAR_COM_CONSTANTS_H_

