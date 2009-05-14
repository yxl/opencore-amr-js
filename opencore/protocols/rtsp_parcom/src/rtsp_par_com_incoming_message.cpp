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

#include "rtsp_par_com.h"
#include "oscl_string_utils.h"
#include "rtsp_range_utils.h"

OSCL_EXPORT_REF void
RTSPIncomingMessage::reset()
{
    RTSPGenericMessage::reset();

#ifdef RTSP_PLAYLIST_SUPPORT
    playlistRangeField.setPtrLen("", 0);
    playlistRangeFieldIsSet = false;
    // need to figure out how to store all the error fields.. we may not actually need to
    playlistErrorFieldIsSet = false;
    playlistErrorFieldCount = 0;
    for (int ii = 0; ii < RTSP_MAX_NUMBER_OF_PLAYLIST_ERROR_ENTRIES; ++ii)
    {
        playlistErrorField[ii].setPtrLen("", 0);
    }

    supportedFieldIsSet = false;
    numOfSupportedEntries = 0;
    for (int jj = 0; jj < RTSP_MAX_NUMBER_OF_SUPPORTED_ENTRIES; ++jj)
    {
        supportedField[jj].setPtrLen("", 0);
    }

    playlistRangeUrl.setPtrLen("", 0);
    playlistRangeClipIndex = 0;
    playlistRangeClipOffset = 0;
    playlistRangeNptTime = 0;
#endif

    amMalformed = RTSPOk;
    rtspVersionString = "";
}

void
RTSPIncomingMessage::parseFirstFields()
{

    char * endOfString;

    // The first thing to do is to parse the status line;
    // the rest of the parsing (regular fields) can be parsed out by
    // parseNextPortion() method, since it has to know how to do that anyway.
    // We also have to find out if Content-length is there, etc.

    // so, let's set up the pointers first
    //
    secondaryBufferSpace = secondaryBuffer;
    if (CHAR_CR == *secondaryBufferSpace)
    {
        ++secondaryBufferSpace;
    }
    if (CHAR_LF == *secondaryBufferSpace)
    {
        ++secondaryBufferSpace;
    }

#ifdef SIMPLE_HTTP_SUPPORT
    if (secondaryBufferSizeUsed >= 4)
    {// Real http cloaking. H\02\00\00
        if (('H' != secondaryBufferSpace[0])
                ||	('T' != secondaryBufferSpace[1])
                ||	('T' != secondaryBufferSpace[2])
                ||	('P' != secondaryBufferSpace[3])
           )
        {
            if ('H' == secondaryBufferSpace[0])
            {//HTTP_RESPONSE
                uint32 nOptionLen = secondaryBufferSpace[1];
                secondaryBufferSpace += (2 + nOptionLen);
            }
            /* TBD
            else if('r' == secondaryBufferSpace[0])
            {//HTTP_RV_RESPONSE
            }
            */
        }
    }
#endif

    numPtrFields = 0;

    char * wordPtr[3] = { NULL, NULL, NULL };
    char * endOfLine;
    int   wordCount;
    bool  wordCountTrigger;

    for (endOfLine = secondaryBufferSpace, wordCount = 0, wordCountTrigger = true;
            (endOfLine < secondaryBufferSpace + secondaryBufferSizeUsed)
            && (CHAR_CR != *endOfLine)
            && (CHAR_LF != *endOfLine);
            ++endOfLine
        )
    {
        switch (wordCountTrigger)
        {
            case true:  // if non-space, then it's a beginning of a new word
            {
                if ((*endOfLine >= 0x09 && *endOfLine <= 0x0D) || *endOfLine == 0x20)
                { // spaces
                }
                else
                { // word
                    if (3 > wordCount)
                    {
                        wordPtr[wordCount] = endOfLine;
                    }
                    ++wordCount;

                    // uncock the trigger
                    wordCountTrigger = false;
                }

                break;
            }

            case false: // if space, then cock the trigger
            {
                if ((*endOfLine >= 0x09 && *endOfLine <= 0x0D) || *endOfLine == 0x20)
                { // yeah, space
                    wordCountTrigger = true;
                }
                else
                { // still a word
                }
                break;
            }

            default:
                // ERROR: internal inconsistency, a switch on a boolean should not hit
                // this point
                ;;
        }
    }

    if (3 > wordCount)
    { // only two words in the status line - error

        amMalformed = RTSPErrorSyntax;
        return;
    }

    if (CHAR_CR == *endOfLine)
    {
        *(endOfLine++) = CHAR_NULL;
        if (CHAR_LF == *endOfLine)
        {
            *(endOfLine++) = CHAR_NULL;
        }
    }
    else if (CHAR_LF == *endOfLine)
    {
        *(endOfLine++) = CHAR_NULL;
    }
    else
    {
        // ran to the end of input, didn't find a single newline....
        // most probably the message was too big to handle, but parser still
        // created a message hoping that at least something can be found...
        //

        amMalformed = RTSPErrorSyntax;

        return;
    }



    // figure out the format of the message - request or response?
    //
    // make out the first word... don't forget about '$'
    //

////  endOfString = secondaryBufferSpace;
////  while( ! isspace(*endOfString) )
////    ++endOfString;

    endOfString = wordPtr[1];
    while (--endOfString >= secondaryBufferSpace
            && ((*endOfString >= 0x09 && *endOfString <= 0x0D) || *endOfString == 0x20)
          )
    {
        *endOfString = CHAR_NULL;
    }

////  *endOfString = CHAR_NULL;
////  StrCSumPtrLen firstWordPLSS = secondaryBufferSpace;
    StrCSumPtrLen firstWordPLSS = wordPtr[0];

    if (('R' == *(wordPtr[0]))
            &&	('T' == *(wordPtr[0] + 1))
            &&	('S' == *(wordPtr[0] + 2))
            &&	('P' == *(wordPtr[0] + 3))
            &&	('/' == *(wordPtr[0] + 4))
       )
    {
        // it has to be a response
        //
        msgType = RTSPResponseMsg;
        rtspVersionString = firstWordPLSS;

        // if there is a mismatch, let the Engine know about it and stop parsing
        if (rtspVersionString != RTSPVersionString)
        {
            amMalformed = RTSPErrorVersion;
            return;
        }

        // get the code
////      secondaryBufferSpace = endOfString+1;
////     while( isspace(*secondaryBufferSpace) )
////      {
////          ++secondaryBufferSpace;
////        }

////      statusCode = atoi( secondaryBufferSpace );
        uint32 atoi_tmp;
        PV_atoi(wordPtr[1], 'd', atoi_tmp);
        statusCode = (RTSPStatusCode)atoi_tmp;

        // get the reason string

////      while( ! isspace( *secondaryBufferSpace ) )
////        {
////          ++secondaryBufferSpace;
////        }

////      while( isspace( *secondaryBufferSpace ) )
////        {
////          ++secondaryBufferSpace;
////        }

////      endOfString = secondaryBufferSpace;
////      while( 			(CHAR_CR != *endOfString)
////                                &&	(CHAR_LF != *endOfString)
////                                )
////        {
////          ++endOfString;
////        }

////      *endOfString = CHAR_NULL;
////      reasonString = secondaryBufferSpace;
        reasonString = wordPtr[2];

        // now, resync to the next line
////      secondaryBufferSpace = endOfString+1;
////      if( CHAR_LF == *secondaryBufferSpace )
////        {
////          ++secondaryBufferSpace;
////        }

        secondaryBufferSpace = endOfLine;
    }
#ifdef SIMPLE_HTTP_SUPPORT
    else if (('H' == *(wordPtr[0]))
             &&	('T' == *(wordPtr[0] + 1))
             &&	('T' == *(wordPtr[0] + 2))
             &&	('P' == *(wordPtr[0] + 3))
             &&	('/' == *(wordPtr[0] + 4))
            )
    {
        msgType = RTSPResponseMsg;
        rtspVersionString = firstWordPLSS;

        // if there is a mismatch, let the Engine know about it and stop parsing
        if ((rtspVersionString != HTTPVersion_1_0_String)
                && (rtspVersionString != HTTPVersion_1_1_String))
        {
            amMalformed = RTSPErrorVersion;
            return;
        }
        uint32 atoi_tmp;
        PV_atoi(wordPtr[1], 'd', atoi_tmp);
        statusCode = (RTSPStatusCode)atoi_tmp;
        reasonString = wordPtr[2];
        secondaryBufferSpace = endOfLine;
    }
#endif
    else
    {	// okay, it could be a request

        // but for requests there must be exactly three words on the
        // status line
        if (3 != wordCount)
        {
            amMalformed = RTSPErrorSyntax;
            return;
        }

        msgType = RTSPRequestMsg;

        methodString = firstWordPLSS;

        if (firstWordPLSS == RtspRequestMethodStringDescribe)
        {
            method = METHOD_DESCRIBE;
        }
        else if (firstWordPLSS == RtspRequestMethodStringGetParameter)
        {
            method = METHOD_GET_PARAMETER;
        }
        else if (firstWordPLSS == RtspRequestMethodStringOptions)
        {
            method = METHOD_OPTIONS;
        }
        else if (firstWordPLSS == RtspRequestMethodStringPause)
        {
            method = METHOD_PAUSE;
        }
        else if (firstWordPLSS == RtspRequestMethodStringPlay)
        {
            method = METHOD_PLAY;
        }
        else if (firstWordPLSS == RtspRequestMethodStringSetup)
        {
            method = METHOD_SETUP;
        }
        else if (firstWordPLSS == RtspRequestMethodStringRecord)
        {
            method = METHOD_RECORD;
        }
        else if (firstWordPLSS == RtspRequestMethodStringTeardown)
        {
            method = METHOD_TEARDOWN;
        }
        else if (firstWordPLSS == RtspRequestMethodStringEndOfStream)
        {
            method = METHOD_END_OF_STREAM;
        }
        /* TBD: add Announce, SetParameter and Redirect later
        else if( firstWordPLSS == RtspRequestMethodStringAnnounce )
          {
            method = METHOD_ANNOUNCE;
          }
        */
        else if (firstWordPLSS == RtspRequestMethodStringSetParameter)
        {
            method = METHOD_SET_PARAMETER;
        }

        else if (firstWordPLSS == RtspRequestMethodStringRedirect)  //SS
        {
            method = METHOD_REDIRECT; ////SS
        }
        // now, we could put binary data here, but it's not exactly the same
        // thing; since it requires special care, let's not confuse things by
        // putting generic-looking stuff here
        else
        {
            method = METHOD_UNRECOGNIZED;
        }

        // get the URI

////      secondaryBufferSpace = endOfString +1;
////      while( isspace( *secondaryBufferSpace ) )
////        {
////          ++secondaryBufferSpace;
////        }

////      endOfString = secondaryBufferSpace;
////      while( ! isspace( *endOfString ) )
////        {
////          ++endOfString;
////        }
////      *endOfString = CHAR_NULL;

////      originalURI = secondaryBufferSpace;
        endOfString = wordPtr[2];
        while ((--endOfString) >= wordPtr[1]
                && ((*endOfString >= 0x09 && *endOfString <= 0x0D) || *endOfString == 0x20)
              )
        {
            *endOfString = CHAR_NULL;
        }
        originalURI = wordPtr[1];

        // get the RTSP version
////      secondaryBufferSpace = endOfString +1;
////      while( isspace( *secondaryBufferSpace ) )
////        {
////          ++secondaryBufferSpace;
////        }
////      endOfString = secondaryBufferSpace;
////      while( !isspace(*endOfString) )
////        {
////          ++endOfString;
////        }

////      *endOfString = CHAR_NULL;
////      rtspVersionString = secondaryBufferSpace;
        endOfString = endOfLine;
        while (--endOfString >= wordPtr[2]
                && (((*endOfString >= 0x09 && *endOfString <= 0x0D) || *endOfString == 0x20)
                    ||  CHAR_NULL == *endOfString
                   )
              )
        {
            *endOfString = CHAR_NULL;
        }
        rtspVersionString = wordPtr[2];

        // if there is a mismatch, let the Engine know about it and stop parsing
        if (rtspVersionString != RTSPVersionString)
        {
            amMalformed = RTSPErrorVersion;
            return;
        }

        // now, resync to the next line
////      secondaryBufferSpace = endOfString + 1;
////      if( CHAR_LF == *secondaryBufferSpace )
////        {
////          ++secondaryBufferSpace;
////        }
        secondaryBufferSpace = endOfLine;

        // extra URI validation
        {
            if (1 == originalURI.length())
            { // Is it a star
                if (CHAR_STAR != originalURI.c_str()[0])
                {
                    amMalformed = RTSPErrorSyntax;
                    return;
                }
            }
            else
            { // it's a normal URI

                // let's validate the scheme
                char colonStr[2];
                colonStr[0] = CHAR_COLON;
                colonStr[1] = NULL_TERM_CHAR;
                const char * colonPtr = oscl_strstr(originalURI.c_str(), colonStr);
                if (NULL == colonPtr)
                {
                    // no colon
                    amMalformed = RTSPErrorSyntax;
                    return;
                }

                // now, the first character of the scheme is a character
                const char * ptr = originalURI.c_str();
                if (!((*ptr >= 'A' && *ptr <= 'Z') || (*ptr >= 'a' && *ptr <= 'z')))

                {
                    // the URI doesn't start with a character
                    amMalformed = RTSPErrorSyntax;
                    return;
                }

                // the rest of the characters must be either characters, or
                // plus/minus, or dots...
                for (++ptr; ptr < colonPtr; ++ptr)
                {
                    if (!(((*ptr >= 'A' && *ptr <= 'Z') || (*ptr >= 'a' && *ptr <= 'z'))
                            || (*ptr >= '0' || *ptr <= '9')
                            ||  CHAR_PLUS == *ptr
                            ||  CHAR_MINUS == *ptr
                            ||  CHAR_DOT == *ptr
                         ))
                    {
                        amMalformed = RTSPErrorSyntax;
                        return;
                    }
                }
            }
        }

        // finish off the URI processing

        {
            // look for the base
            uint32  ii;
            int32   ee;
            uint32 length = originalURI.length();

            for (ii = 0; ii < length; ++ii)
            {
                if (CHAR_SLASH == originalURI.c_str()[ii])
                {
                    if (CHAR_SLASH != originalURI.c_str()[++ii])
                    {
                        break;
                    }
                    else
                    { // no, it's a separator, skip over it, continue
                    }
                }
            }
            originalURIBase = originalURI.c_str() + ii;

            // look for the control candidate
            for (ee = originalURIBase.length() - 1;
                    ee >= 0;
                    --ee
                )
            {
                if ((CHAR_SLASH      ==  originalURIBase.c_str()[ee])
                        || (CHAR_SEMICOLON  ==  originalURIBase.c_str()[ee])
                   )
                {
                    break;
                }
            }
            originalURIControlCandidate = originalURIBase.c_str() + ee + 1;

        }
    }


    // determine the total number of fields remaining to be seen

    endOfString = secondaryBuffer + secondaryBufferSizeUsed;
    *(--endOfString) = CHAR_NULL;
    --secondaryBufferSizeUsed;
    if (CHAR_CR == *(endOfString - 1))
    {
        *(--endOfString) = CHAR_NULL;
        --secondaryBufferSizeUsed;
    }


    totalFields = 0;
    for (char * ptr = secondaryBufferSpace; ptr < endOfString; ++ptr)
    {
        if (CHAR_LF == *ptr)
        {
            ++totalFields;
        }
        else if (CHAR_CR == *ptr)
        {
            if (CHAR_LF != *(ptr + 1))
            {
                ++totalFields;
            }
        }
    }

    totalFieldsParsed = 0;
    parseNextPortion();

    if (getTotalFields() != getTotalFieldsParsed())
    {
        amMalformed = RTSPErrorTooManyFields;
    }
}

bool
RTSPIncomingMessage::parseNextPortion()
{
    if (totalFieldsParsed == totalFields)
    {
        return false;
    }

    char * endOfMessage = secondaryBuffer + secondaryBufferSizeUsed;
    char * ptr = secondaryBufferSpace;

    //  printf ("parcom::parseNextPortion: secondaryBuffer is \n %s \n",secondaryBuffer);


    for (numPtrFields = 0;
            (numPtrFields < RTSP_MAX_NUMBER_OF_FIELDS) &&	(ptr < endOfMessage);
            ++numPtrFields)
    {
        char *endOfValue = ptr;
        while (CHAR_LF != *(endOfValue)
                &&  CHAR_CR != *(endOfValue)
                &&  CHAR_NULL != *(endOfValue))
        {
            ++endOfValue;
        }

        if (CHAR_CR == *(endOfValue) && CHAR_LF == *(endOfValue + 1))
        {
            // need to increment endOfValue for CR-LF, because
            // it is needed later to step into the next field by shifting by 1
            *(endOfValue) = CHAR_NULL;
            *(++endOfValue) = CHAR_NULL;
        }
        else
        {
            *endOfValue = CHAR_NULL;
        }

        char * separator = ptr;

        while ((CHAR_COLON != *separator)	&&	(CHAR_NULL != *separator))
        {
            ++separator;
        }

        if (CHAR_COLON != *separator)
        {
            //amMalformed = RTSPErrorSyntax;
            //ignore the unknown lines
            ptr = endOfValue + 1;
            continue;
        }
        else
        {
            *separator = CHAR_NULL;

            // get name pointer
            char * namePtr = ptr;
            // eat through trailing whitespace
            for (char * wPtr1 = separator - 1;
                    (wPtr1 >= namePtr) && ((*wPtr1 >= 0x09 && *wPtr1 <= 0x0D) || *wPtr1 == 0x20);
                    --wPtr1)
            {
                *wPtr1 = CHAR_NULL;
            }
            // eat through leading whitespace
            while ((*namePtr >= 0x09 && *namePtr <= 0x0D) || *namePtr == 0x20)
            {
                ++namePtr;
            }

            // get value pointer
            char * valuePtr = separator + 1;
            // eat through trailing whitespace
            for (char * wPtr2 = endOfValue - 1;
                    (wPtr2 > separator)
                    && ((*wPtr2 >= 0x09 && *wPtr2 <= 0x0D) || *wPtr2 == 0x20);
                    --wPtr2)
            {
                *wPtr2 = CHAR_NULL;
            }
            //eat through leading whitespace
            while ((*valuePtr >= 0x09 && *valuePtr <= 0x0D) || *valuePtr == 0x20)
            {
                ++valuePtr;
            }

            // all right, set the pointers

            fieldKeys[ numPtrFields ] = namePtr;
            fieldVals[ numPtrFields ] = valuePtr;

            // determine if we are supposed to recognize this
            if (fieldKeys[ numPtrFields ].isCIEquivalentTo(
                        RtspRecognizedFieldSessionId))
            {
                {
                    StrPtrLen tmp = fieldVals[ numPtrFields ];
                    int Len = tmp.length();
                    char * beginPtr = (char*)tmp.c_str();
                    char * endPtr = beginPtr + Len;

                    while (beginPtr < endPtr)
                    {
                        if (*beginPtr == ';')
                        {//we got timeout field
                            while (beginPtr < endPtr)
                            {
                                if ((*beginPtr == 't') && (beginPtr[6] == 't'))
                                {//a little bit validation
                                    beginPtr += 8;
                                    PV_atoi(beginPtr, 'd', timeout);
                                    beginPtr = endPtr;
                                }
                                beginPtr++;
                            }
                        }
                        beginPtr++;
                    }
                }
                sessionId = fieldVals[ numPtrFields ];
                sessionIdIsSet = true;
            }
            if (fieldKeys[ numPtrFields ].isCIEquivalentTo(
                        RtspRecognizedFieldCSeq))
            {
                PV_atoi(valuePtr, 'd', cseq);
                cseqIsSet = true;
            }
            if (fieldKeys[ numPtrFields ].isCIEquivalentTo(
                        RtspRecognizedFieldBufferSize
                    ))
            {
                PV_atoi(valuePtr, 'd', bufferSize);
                bufferSizeIsSet = true;
            }
            if (fieldKeys[ numPtrFields ].isCIEquivalentTo(
                        RtspRecognizedFieldContentType))
            {
                contentType = fieldVals[ numPtrFields ];
                contentTypeIsSet = true;
            }
            if (fieldKeys[ numPtrFields ].isCIEquivalentTo(
                        RtspRecognizedFieldContentBase))
            {
                contentBase = fieldVals[ numPtrFields ];
                contentBaseMode = CONTENT_BASE_SET;
            }
            if (fieldKeys[ numPtrFields ].isCIEquivalentTo(
                        RtspRecognizedFieldContentLength))
            {
                PV_atoi(valuePtr, 'd', contentLength);
                contentLengthIsSet = true;
            }
            if (fieldKeys[ numPtrFields].isCIEquivalentTo(
                        RtspRecognizedFieldUserAgent))
            {
                userAgent = fieldVals[ numPtrFields ];
                userAgentIsSet = true;
            }
            if (fieldKeys[ numPtrFields ].isCIEquivalentTo(
                        RtspRecognizedFieldAccept))
            {
                accept = fieldVals[ numPtrFields ];
                acceptIsSet = true;
            }
            if (fieldKeys[ numPtrFields ].isCIEquivalentTo(
                        RtspRecognizedFieldRequire))
            {
                require = fieldVals[ numPtrFields ];
                requireIsSet = true;
            }
            if (fieldKeys[ numPtrFields ].isCIEquivalentTo(
                        RtspRecognizedFieldRTPInfo
                    ))
            {
                parseRTPInfo(numPtrFields);
            }

            if (fieldKeys[ numPtrFields ].isCIEquivalentTo(
                        RtspRecognizedFieldRange))
            {
                parseRtspRange(fieldVals[ numPtrFields].c_str(), fieldVals[ numPtrFields].length(), range);
                rangeIsSet = true;
            }
            if (fieldKeys[ numPtrFields ].isCIEquivalentTo(
                        RtspRecognizedFieldTransport))
            {
                parseTransport(numPtrFields);
            }
#ifdef RTSP_PLAYLIST_SUPPORT
            if (fieldKeys[ numPtrFields ].isCIEquivalentTo(
                        RtspRecognizedFieldSupported))
            {
                parseSupported(fieldVals[ numPtrFields].c_str(), fieldVals[ numPtrFields].length() + 1);
                supportedFieldIsSet = true;
            }
#endif
        }

        ptr = endOfValue + 1;
    }

    totalFieldsParsed += numPtrFields;

    secondaryBufferSpace = ptr;

    return true;
}

#ifdef RTSP_PLAYLIST_SUPPORT

void
RTSPIncomingMessage::parseSupported(const char *supportedString, int length)
{
    const StrPtrLen methodEosString("method.eos");
    const StrPtrLen comPvPlaylistString("com.pv.server_playlist");

    const char *end = supportedString + length;

    char *sptr;//, *eptr;

    sptr = (char*)supportedString;

    while ((sptr < end) && (numOfSupportedEntries < RTSP_MAX_NUMBER_OF_SUPPORTED_ENTRIES))
    {
        char * separator = sptr;

        while ((CHAR_COMMA != *separator)
                &&	(CHAR_NULL != *separator)
                && (separator < end)
              )
        {
            ++separator;
        }

        if (CHAR_COMMA == *separator)
        {
            // make sure there is a terminating char
            // in the other case, there will already be one because its the end of the line
            // and the field parsing that called this function put one there
            *separator = CHAR_NULL;
        }

        // get name pointer
        char * namePtr = sptr;
        // eat through trailing whitespace
        for (char * wPtr1 = separator - 1;
                (wPtr1 >= namePtr)
                && ((*wPtr1 >= 0x09 && *wPtr1 <= 0x0D) || *wPtr1 == 0x20);
                --wPtr1)
        {
            *wPtr1 = CHAR_NULL;
        }
        // eat through leading whitespace
        while ((*namePtr >= 0x09 && *namePtr <= 0x0D) || *namePtr == 0x20)
        {
            ++namePtr;
        }

        supportedField[numOfSupportedEntries++] = namePtr;

        if (!oscl_strncmp(namePtr, methodEosString.c_str(), methodEosString.length()))
        {
            methodEosIsSet = true;
        }
        else if (!oscl_strncmp(namePtr, comPvPlaylistString.c_str(), comPvPlaylistString.length()))
        {
            comPvServerPlaylistIsSet = true;
        }

        sptr = separator + 1;

    }  // end while
}

#endif /* RTSP_PLAYLIST_SUPPORT */


// relevant constants
const int MIN_CHANNEL_VALUE = 0;
const int MAX_CHANNEL_VALUE = 255;
const int PORT_MIN_VALUE = 0;
const int PORT_MAX_VALUE = 65535;

#define interleaved_str "interleaved="
#define interleaved_str_len 12

#define client_port_str "client_port="
#define client_port_str_len 12

#define server_port_str "server_port="
#define server_port_str_len 12

#define ttl_str "ttl="
#define ttl_str_len 4

#define mode_str "mode="
#define mode_str_len 5

#define port_str "port="
#define port_str_len 5

#define layers_str "layers="
#define layers_str_len 7

#define ssrc_str "ssrc="
#define ssrc_str_len 5

#define destination_str "destination"
#define destination_str_len 11

void
RTSPIncomingMessage::parseTransport(uint16 fieldIdx)
{
    char * cPtr;
    char * finishPtr;

    cPtr = const_cast<char*>(fieldVals[ fieldIdx ].c_str());
    finishPtr = cPtr + fieldVals[ fieldIdx ].length();

    do
    {
        parseOneTransportEntry(cPtr, finishPtr);

    }
    while (RTSPOk == amMalformed
            &&  cPtr < finishPtr
          );
}


void
RTSPIncomingMessage::parseOneTransportEntry(char*& trans, char *final_end)
{
    const char *startPtr, *endPtr, *nxtPtr;
    char *transSepPtr;



    // check the counter
    //
    if (RTSP_MAX_NUMBER_OF_TRANSPORT_ENTRIES == numOfTransportEntries)
    { // limit reached
        amMalformed = RTSPErrorSyntax;  // should it be different?
        return;
    }

    RtspTransport* rtspTrans = transport + numOfTransportEntries;
    numOfTransportEntries++;

    startPtr = trans;

    if (*startPtr == ',')
    {
        // skip over any starting commas
        ++startPtr;
    }

    rtspTrans->appendIsSet = false;
    rtspTrans->channelIsSet = false;
    rtspTrans->client_portIsSet = false;
    rtspTrans->deliveryIsSet = false;
    rtspTrans->layersIsSet = false;
    rtspTrans->modeIsSet = false;
    rtspTrans->portIsSet = false;
    rtspTrans->profileIsSet = false;
    rtspTrans->protocolIsSet = false;
    rtspTrans->transportTypeIsSet = false;
    rtspTrans->server_portIsSet = false;
    rtspTrans->ttlIsSet = false;
    rtspTrans->destinationIsSet = false;



    // see if there is a tranport list separator
    transSepPtr = OSCL_CONST_CAST(char*, oscl_strstr(startPtr, ","));
    if (transSepPtr)
    {

        const char *quotePtr = oscl_strstr(startPtr, "\"");
        if (quotePtr && quotePtr < transSepPtr)
        {
            // this may be a comma in the mode list -- so find the end of the mode list
            const char *quote_end = oscl_strstr(quotePtr + 1, "\"");
            if (quote_end)
            {
                // look for another comma
                if (NULL != (transSepPtr = OSCL_CONST_CAST(char*, oscl_strstr(quote_end, ","))))
                {
                    // temporarily write a terminator to separate the transport specs.
                    *transSepPtr = '\0';
                }
            }
        }
        else
        {
            // temporarily write a terminator to separate the transport specs.
            *transSepPtr = '\0';
        }
    }


//  for (; startPtr < final_end && isspace(*startPtr); ++startPtr);
    for (; startPtr < final_end && ((*startPtr >= 0x09 && *startPtr <= 0x0D) || *startPtr == 0x20); ++startPtr);

    do
    {
        if (NULL == (endPtr = oscl_strstr(startPtr, "/")))
        {
            endPtr = final_end;
        }

        rtspTrans->protocolIsSet = true;

        if (!oscl_strncmp(startPtr, "RTP", endPtr - startPtr))
        {
            rtspTrans->protocol = RtspTransport::RTP_PROTOCOL;
        }
        else if (!oscl_strncmp(startPtr, "x-pn-tng", endPtr - startPtr))
        {
            rtspTrans->protocol = RtspTransport::RDT_PROTOCOL;
        }
        else
        {
            rtspTrans->protocol = RtspTransport::UNKNOWN_PROTOCOL;
        }

        if (endPtr == final_end)
        {
            break;
        }

        // increment the startPtr past the separator
        startPtr = endPtr + 1;

        // look for either the '/' or ';' separators
        endPtr = oscl_strstr(startPtr, "/");
        const char *endPtr2 = oscl_strstr(startPtr, ";");

        if (!endPtr)
        {
            endPtr = endPtr2;
        }

        if (endPtr2 && endPtr2 < endPtr)
        {
            endPtr = endPtr2;
        }

        if (!endPtr)
        {
            endPtr = final_end;
        }

        rtspTrans->profile = RtspTransport::UNKNOWN_PROFILE;
        rtspTrans->profileIsSet = true;
        if (!oscl_strncmp(startPtr, "AVP", endPtr - startPtr))
        {
            rtspTrans->profile = RtspTransport::AVP_PROFILE;
        }
        else if (!oscl_strncmp(startPtr, "tcp", endPtr - startPtr))
        {//RDT "x-pn-tng/tcp"
            rtspTrans->profile = RtspTransport::TCP_PROFILE;
        }

        if (*endPtr == '/')
        {
            // there is an optional transport spec
            startPtr = endPtr + 1;

            if (NULL == (endPtr = oscl_strstr(startPtr, ";")))
            {
                endPtr = final_end;
            }

            rtspTrans->transportType = RtspTransport::UNKNOWN_TRANSPORT;
            rtspTrans->transportTypeIsSet = true;

            if (!oscl_strncmp(startPtr, "UDP", endPtr - startPtr))
            {
                rtspTrans->transportType = RtspTransport::UDP_TRANSPORT;
            }
            else if (!oscl_strncmp(startPtr, "TCP", endPtr - startPtr))
            {
                rtspTrans->transportType = RtspTransport::TCP_TRANSPORT;
            }
        }

        if (endPtr == final_end)
        {
            break;
        }

        // increment the startPtr past the separator
        startPtr = endPtr + 1;

    }
    while (0); // only go through this once


    while (startPtr < final_end)
    {

        for (; startPtr < final_end && ((*startPtr >= 0x09 && *startPtr <= 0x0D) || *startPtr == 0x20); ++startPtr);

        char *sepPtr;
        // find the next separator
        if (NULL == (sepPtr = (char*)(endPtr = oscl_strstr(startPtr, ";"))))
        {
            endPtr = final_end;
        }

        if (sepPtr)
        {
            *sepPtr = '\0';
        }

        if (!oscl_strncmp(startPtr, "unicast", endPtr - startPtr))
        {
            rtspTrans->delivery = RtspTransport::UNICAST_DELIVERY;
            rtspTrans->deliveryIsSet = true;
        }
        if (!oscl_strncmp(startPtr, "multicast", endPtr - startPtr))
        {
            rtspTrans->delivery = RtspTransport::MULTICAST_DELIVERY;
            rtspTrans->deliveryIsSet = true;
        }
        else if (!oscl_strncmp(startPtr, "append", endPtr - startPtr))
        {
            rtspTrans->append = true;
            rtspTrans->appendIsSet = true;
        }
        else if (!oscl_strncmp(startPtr, destination_str,
                               destination_str_len))
        {
            nxtPtr = startPtr + destination_str_len;
            char *curEndPtr = OSCL_CONST_CAST(char*, oscl_strstr(nxtPtr, "="));
            if (curEndPtr)
            {
                nxtPtr = curEndPtr + 1;

                // skip leading whitespace
                for (; nxtPtr < endPtr && ((*nxtPtr >= 0x09 && *nxtPtr <= 0x0D) || *nxtPtr == 0x20); ++nxtPtr);

                // now find the end
                for (curEndPtr = (char*)nxtPtr; curEndPtr < endPtr && !((*curEndPtr >= 0x09 && *curEndPtr <= 0x0D) || *curEndPtr == 0x20);
                        ++curEndPtr);

                if (curEndPtr - nxtPtr > 0)
                {
                    if (curEndPtr < endPtr)
                    {
                        *curEndPtr = '\0';
                    }
                    else if (sepPtr)
                    {
                        // clear sepPtr so it won't overwrite the NULL character
                        sepPtr = NULL;
                    }
                    rtspTrans->destination.setPtrLen(nxtPtr,
                                                     curEndPtr - nxtPtr);
                    rtspTrans->destinationIsSet = true;
                }
            }

        }
        else if (!oscl_strncmp(startPtr, interleaved_str,
                               interleaved_str_len))
        {
            int tmp;
            nxtPtr = startPtr + interleaved_str_len;
            char *curEndPtr = OSCL_CONST_CAST(char*, oscl_strstr(nxtPtr, "-"));
            if (curEndPtr) *curEndPtr = '\0';
            uint32 atoi_tmp;
            PV_atoi(nxtPtr, 'd', atoi_tmp);
            tmp = atoi_tmp;
            if (tmp < MIN_CHANNEL_VALUE || tmp > MAX_CHANNEL_VALUE)
            {
                amMalformed = RTSPErrorSyntax;
                if (sepPtr)
                {
                    *sepPtr = ';';
                };
                break;
            }
            rtspTrans->channel1 = tmp;
            if (!curEndPtr)
            {
                if (rtspTrans->channel1 >= MAX_CHANNEL_VALUE)
                {
                    amMalformed = RTSPErrorSyntax;
                    if (sepPtr)
                    {
                        *sepPtr = ';';
                    };
                    break;
                }
                rtspTrans->channel2 = rtspTrans->channel1 + 1;
                rtspTrans->channelIsSet = true;
            }
            else
            {
                nxtPtr = curEndPtr + 1;
                *curEndPtr = '-';
                PV_atoi(nxtPtr, 'd', atoi_tmp);
                tmp = atoi_tmp;
                if (tmp < MIN_CHANNEL_VALUE || tmp > MAX_CHANNEL_VALUE)
                {
                    amMalformed = RTSPErrorSyntax;
                    if (sepPtr)
                    {
                        *sepPtr = ';';
                    };
                    break;
                }
                rtspTrans->channel2 = tmp;
                rtspTrans->channelIsSet = true;
            }
        }
        else if (!oscl_strncmp(startPtr, client_port_str,
                               client_port_str_len))
        {
            int tmp;
            uint32 atoi_tmp;
            nxtPtr = startPtr + client_port_str_len;
            char *curEndPtr = OSCL_CONST_CAST(char*, oscl_strstr(nxtPtr, "-"));
            if (curEndPtr) *curEndPtr = '\0';
            PV_atoi(nxtPtr, 'd', atoi_tmp);
            tmp = atoi_tmp;
            if (tmp < PORT_MIN_VALUE || tmp > PORT_MAX_VALUE)
            {
                amMalformed = RTSPErrorSyntax;
                if (sepPtr)
                {
                    *sepPtr = ';';
                };
                break;
            }
            rtspTrans->client_port1 = tmp;
            if (!curEndPtr)
            {
                if (rtspTrans->client_port1 >= PORT_MAX_VALUE)
                {
                    amMalformed = RTSPErrorSyntax;
                    if (sepPtr)
                    {
                        *sepPtr = ';';
                    };
                    break;
                }
                rtspTrans->client_port2 = rtspTrans->client_port1 + 1;
                rtspTrans->client_portIsSet = true;
            }
            else
            {
                nxtPtr = curEndPtr + 1;
                *curEndPtr = '-';
                PV_atoi(nxtPtr, 'd', atoi_tmp);
                tmp = atoi_tmp;
                if (tmp < PORT_MIN_VALUE || tmp > PORT_MAX_VALUE)
                {
                    amMalformed = RTSPErrorSyntax;
                    if (sepPtr)
                    {
                        *sepPtr = ';';
                    };
                    break;
                }
                rtspTrans->client_port2 = tmp;
                rtspTrans->client_portIsSet = true;
            }
        }
        else if (!oscl_strncmp(startPtr, server_port_str,
                               server_port_str_len))
        {
            int tmp;
            uint32 atoi_tmp;
            nxtPtr = startPtr + server_port_str_len;
            char *curEndPtr = OSCL_CONST_CAST(char*, oscl_strstr(nxtPtr, "-"));
            if (curEndPtr) *curEndPtr = '\0';
            PV_atoi(nxtPtr, 'd', atoi_tmp);
            tmp = atoi_tmp;
            if (tmp < PORT_MIN_VALUE || tmp > PORT_MAX_VALUE)
            {
                amMalformed = RTSPErrorSyntax;
                if (sepPtr)
                {
                    *sepPtr = ';';
                };
                break;
            }
            rtspTrans->server_port1 = tmp;
            if (!curEndPtr)
            {
                if (rtspTrans->server_port1 >= PORT_MAX_VALUE)
                {
                    amMalformed = RTSPErrorSyntax;
                    if (sepPtr)
                    {
                        *sepPtr = ';';
                    };
                    break;
                }
                rtspTrans->server_port2 = rtspTrans->server_port1 + 1;
                rtspTrans->server_portIsSet = true;
            }
            else
            {
                nxtPtr = curEndPtr + 1;
                *curEndPtr = '-';
                PV_atoi(nxtPtr, 'd', atoi_tmp);
                tmp = atoi_tmp;
                if (tmp < PORT_MIN_VALUE || tmp > PORT_MAX_VALUE)
                {
                    amMalformed = RTSPErrorSyntax;
                    if (sepPtr)
                    {
                        *sepPtr = ';';
                    };
                    break;
                }
                rtspTrans->server_port2 = tmp;
                rtspTrans->server_portIsSet = true;
            }
        }
        else if (!oscl_strncmp(startPtr, ttl_str, ttl_str_len))
        {
            nxtPtr = startPtr + ttl_str_len;
            int tmp;
            uint32 atoi_tmp;
            PV_atoi(nxtPtr, 'd', atoi_tmp);
            tmp = atoi_tmp;
            if (tmp < 0 || tmp > 255)
            {
                amMalformed = RTSPErrorSyntax;
                if (sepPtr)
                {
                    *sepPtr = ';';
                };
                break;
            }
            rtspTrans->ttl = tmp;
            rtspTrans->ttlIsSet = true;
        }
        else if (!oscl_strncmp(startPtr, mode_str, mode_str_len))
        {
            rtspTrans->mode.play_mode = false;
            rtspTrans->mode.record_mode = false;
            nxtPtr = startPtr + mode_str_len;
            rtspTrans->modeIsSet = true;
            char *quotePtr = NULL;
            const char *wordEndPtr;
            if (*nxtPtr == '\"')
            {
                ++nxtPtr;
                if (NULL != (quotePtr = OSCL_CONST_CAST(char*, oscl_strstr(nxtPtr, "\""))))
                {
                    *quotePtr = '\0';
                }
            }
            while (nxtPtr < endPtr)
            {
                if (NULL == (wordEndPtr = oscl_strstr(nxtPtr, ",")))
                {
                    wordEndPtr = endPtr;
                }

                if (!oscl_strncmp(nxtPtr, "PLAY", wordEndPtr - nxtPtr))
                {
                    rtspTrans->mode.play_mode = true;
                }
                else if (!oscl_strncmp(nxtPtr, "RECORD", wordEndPtr - nxtPtr))
                {
                    rtspTrans->mode.record_mode = true;
                }
                nxtPtr = wordEndPtr + 1;

            }

            if (quotePtr)
            {
                *quotePtr = '\"';
            }
        }
        else if (!oscl_strncmp(startPtr, port_str, port_str_len))
        {
            int tmp;
            uint32 atoi_tmp;
            nxtPtr = startPtr + port_str_len;
            char *curEndPtr = OSCL_CONST_CAST(char*, oscl_strstr(nxtPtr, "-"));
            if (curEndPtr) *curEndPtr = '\0';
            PV_atoi(nxtPtr, 'd', atoi_tmp);
            tmp = atoi_tmp;
            if (tmp < PORT_MIN_VALUE || tmp > PORT_MAX_VALUE)
            {
                amMalformed = RTSPErrorSyntax;
                if (sepPtr)
                {
                    *sepPtr = ';';
                };
                break;
            }
            rtspTrans->port1 = tmp;
            if (!curEndPtr)
            {
                if (rtspTrans->port1 >= PORT_MAX_VALUE)
                {
                    amMalformed = RTSPErrorSyntax;
                    if (sepPtr)
                    {
                        *sepPtr = ';';
                    };
                    break;
                }
                rtspTrans->port2 = rtspTrans->port1 + 1;
                rtspTrans->portIsSet = true;
            }
            else
            {
                nxtPtr = curEndPtr + 1;
                *curEndPtr = '-';
                PV_atoi(nxtPtr, 'd', atoi_tmp);
                tmp = atoi_tmp;
                if (tmp < PORT_MIN_VALUE || tmp > PORT_MAX_VALUE)
                {
                    amMalformed = RTSPErrorSyntax;
                    if (sepPtr)
                    {
                        *sepPtr = ';';
                    };
                    break;
                }
                rtspTrans->port2 = tmp;
                rtspTrans->portIsSet = true;
            }
        }
        else if (!oscl_strncmp(startPtr, layers_str, layers_str_len))
        {
            nxtPtr = startPtr + layers_str_len;
            PV_atoi(nxtPtr, 'd', rtspTrans->layers);
            rtspTrans->layersIsSet = true;
        }
        else if (!oscl_strncmp(startPtr, ssrc_str, ssrc_str_len))
        {
            nxtPtr = startPtr + ssrc_str_len;
            rtspTrans->ssrc = 0;
            rtspTrans->ssrcIsSet = false;
            if (!PV_atoi(nxtPtr, 'x', rtspTrans->ssrc))
            {
                amMalformed = RTSPErrorSyntax;
                if (sepPtr)
                {
                    *sepPtr = ';';
                };
                break;
            }
            rtspTrans->ssrcIsSet = true;
        }

        if (sepPtr)
        {
            *sepPtr = ';';
        }

        startPtr = endPtr + 1;

    }

    if (transSepPtr)
    {
        // put the separator character back
        *transSepPtr = ',';
        trans = transSepPtr;
    }
    else
    {
        trans = final_end;
    }

}

void
RTSPIncomingMessage::parseRTPInfo(uint16 fieldIdx)
{
    char * cPtr;
    char * finishPtr;

    cPtr = const_cast<char*>(fieldVals[ fieldIdx ].c_str());
    finishPtr = cPtr + fieldVals[ fieldIdx ].length();

    do
    {
        parseOneRTPInfoEntry(cPtr, finishPtr);

    }
    while (RTSPOk == amMalformed
            &&  cPtr < finishPtr
          );
}

void
RTSPIncomingMessage::parseOneRTPInfoEntry(char * & cPtr, char * finishPtr)
{
    bool   getOut;
    char * subFieldName;
    char * separator;
    char * endOfIt;

    // check the counter
    //
    if (RTSP_MAX_NUMBER_OF_RTP_INFO_ENTRIES == numOfRtpInfoEntries)
    { // limit reached
        amMalformed = RTSPErrorSyntax;  // should it be different?
        return;
    }
    ++numOfRtpInfoEntries;

    // skip comma, if necessary
    //
    if (CHAR_COMMA == *cPtr)
    {
        ++cPtr;
    }

    getOut = false;

    while ((!getOut) && cPtr < finishPtr && CHAR_COMMA != *cPtr)
    {
        // reset pointers

        subFieldName = NULL;
        separator = NULL;
        endOfIt = NULL;

        // skip whitespace

        while (cPtr < finishPtr
                && (CHAR_SPACE == *cPtr
                    || CHAR_TAB  == *cPtr
                    || CHAR_SEMICOLON == *cPtr
                    || CHAR_NULL == *cPtr
                   )
              )
        {
            ++cPtr;
        }
        if (cPtr >= finishPtr  || CHAR_COMMA  == *cPtr)
        {
            return;
        }

        // we see the beginning
        subFieldName = cPtr;

        // get the separator
        while (cPtr < finishPtr
                && CHAR_EQUAL != *cPtr
              )
        {
            ++cPtr;
        }
        if (cPtr >= finishPtr)
        {
            return;
        }
        // we see the separator
        separator = cPtr;

        // get the end of it
        while (cPtr < finishPtr
                && CHAR_SEMICOLON != *cPtr
                && CHAR_SPACE != *cPtr
                && CHAR_TAB != *cPtr
                && CHAR_COMMA != *cPtr
              )
        {
            ++cPtr;
        }

        endOfIt = cPtr;

        if (3 == separator - subFieldName)
        { // url?
            if (('u' == (subFieldName[0] | OSCL_ASCII_CASE_MAGIC_BIT))
                    && ('r' == (subFieldName[1] | OSCL_ASCII_CASE_MAGIC_BIT))
                    && ('l' == (subFieldName[2] | OSCL_ASCII_CASE_MAGIC_BIT))
               )
            {
                rtpInfo[ numOfRtpInfoEntries-1 ].url.setPtrLen(
                    separator + 1,
                    endOfIt - separator - 1
                );
                rtpInfo[ numOfRtpInfoEntries-1 ].urlIsSet = true;
                if (CHAR_COMMA == *endOfIt)
                {
                    getOut = true;
                }
                *endOfIt = CHAR_NULL;
                if (getOut)
                {
                    // skip ahead beyond the whitespace
                    // since we overwrote the comma with a NULL
                    while (cPtr < finishPtr
                            && (CHAR_SPACE == *cPtr
                                || CHAR_TAB  == *cPtr
                                || CHAR_SEMICOLON == *cPtr
                                || CHAR_NULL == *cPtr
                               )
                          )
                    {
                        ++cPtr;
                    }
                }
            }
            // or, maybe, seq?
            else if (('s' == (subFieldName[0] | OSCL_ASCII_CASE_MAGIC_BIT))
                     && ('e' == (subFieldName[1] | OSCL_ASCII_CASE_MAGIC_BIT))
                     && ('q' == (subFieldName[2] | OSCL_ASCII_CASE_MAGIC_BIT))
                    )
            {
                uint32 atoi_tmp;
                PV_atoi(separator + 1, 'd', atoi_tmp);
                rtpInfo[ numOfRtpInfoEntries-1 ].seq  = (RtpSeqType)atoi_tmp;

                rtpInfo[ numOfRtpInfoEntries-1 ].seqIsSet = true;
            }
            else
            {
                amMalformed = RTSPErrorSyntax;
                return;
            }
        }
        else if (7 == separator - subFieldName)
        { // rtptime?
            if (('r' == (subFieldName[0] | OSCL_ASCII_CASE_MAGIC_BIT))
                    && ('t' == (subFieldName[1] | OSCL_ASCII_CASE_MAGIC_BIT))
                    && ('p' == (subFieldName[2] | OSCL_ASCII_CASE_MAGIC_BIT))
                    && ('t' == (subFieldName[3] | OSCL_ASCII_CASE_MAGIC_BIT))
                    && ('i' == (subFieldName[4] | OSCL_ASCII_CASE_MAGIC_BIT))
                    && ('m' == (subFieldName[5] | OSCL_ASCII_CASE_MAGIC_BIT))
                    && ('e' == (subFieldName[6] | OSCL_ASCII_CASE_MAGIC_BIT))
               )
            {
                uint32 rtptime = 0;
                PV_atoi((separator + 1), 'd', rtptime);
                rtpInfo[ numOfRtpInfoEntries-1 ].rtptime = rtptime;
                rtpInfo[ numOfRtpInfoEntries-1 ].rtptimeIsSet = true;
            }
            else
            {
                amMalformed = RTSPErrorSyntax;
                return;
            }
        }

        else
        {
            amMalformed = RTSPErrorSyntax;
            return;
        }

        cPtr = endOfIt;
    }
}

#if (defined(ASF_STREAMING) || defined(RTSP_PLAYLIST_SUPPORT))
OSCL_EXPORT_REF bool
RTSPIncomingMessage::parseEntityBody(RTSPEntityBody * entityBody)
{
    char * endOfMessage = (char *) entityBody->ptr + entityBody->len;
    char * ptr;

    for (ptr = (char *) entityBody->ptr;
            ptr < endOfMessage ;
            ++numPtrFields
        )
    {
        char * endOfValue = ptr;
        while (CHAR_LF != *(endOfValue)
                &&  CHAR_CR != *(endOfValue)
                &&  CHAR_NULL != *(endOfValue))
        {
            ++endOfValue;
        }

        if (CHAR_CR == *(endOfValue) && CHAR_LF == *(endOfValue + 1))
        {
            // need to increment endOfValue for CR-LF, because
            // it is needed later to step into the next field by shifting by 1
            *(endOfValue) = CHAR_NULL;
            *(++endOfValue) = CHAR_NULL;
        }
        else
        {
            *endOfValue = CHAR_NULL;
        }

        char * separator = ptr;

        while ((CHAR_COLON != *separator)
                &&	(CHAR_NULL != *separator)
              )
        {
            ++separator;
        }

        if (CHAR_COLON != *separator)
        {
            amMalformed = RTSPErrorSyntax;
        }
        else
        {	// field is pretty much normal

            *separator = CHAR_NULL;

            // get name pointer
            char * namePtr = ptr;
            // eat through trailing whitespace
            for (char * wPtr1 = separator - 1;
                    (wPtr1 >= namePtr)
                    && ((*wPtr1 >= 0x09 && *wPtr1 <= 0x0D) || *wPtr1 == 0x20);
                    --wPtr1)
            {
                *wPtr1 = CHAR_NULL;
            }
            // eat through leading whitespace
            while ((*namePtr >= 0x09 && *namePtr <= 0x0D) || *namePtr == 0x20)
            {
                ++namePtr;
            }

            // get value pointer
            char * valuePtr = separator + 1;
            // eat through trailing whitespace
            for (char * wPtr2 = endOfValue - 1;
                    (wPtr2 > separator)
                    && ((*wPtr2 >= 0x09 && *wPtr2 <= 0x0D) || *wPtr2 == 0x20);
                    --wPtr2)
            {
                *wPtr2 = CHAR_NULL;
            }
            //eat through leading whitespace
            while ((*valuePtr >= 0x09 && *valuePtr <= 0x0D) || *valuePtr == 0x20)
            {
                ++valuePtr;
            }

            // set the pointers

            fieldKeys[ numPtrFields ] = namePtr;
            fieldVals[ numPtrFields ] = valuePtr;

            // now, figure out if we are supposed to recognize this
            if (fieldKeys[ numPtrFields ].isCIEquivalentTo(
                        RtspRecognizedFieldSessionId))
            {
                sessionId = fieldVals[ numPtrFields ];
                sessionIdIsSet = true;
            }
//RTSP_PAR_COM_CONSTANTS_H_
#define RtspRecognizedFieldEOF "EOF"

            if (fieldKeys[ numPtrFields ].isCIEquivalentTo(
                        RtspRecognizedFieldEOF))
            {
                eofField = fieldVals[ numPtrFields ];
                eofFieldIsSet = true;
            }
#ifdef RTSP_PLAYLIST_SUPPORT
            if (fieldKeys[ numPtrFields ].isCIEquivalentTo(
                        RtspRecognizedFieldPlaylistRange))
            {
                playlistRangeField = fieldVals[ numPtrFields ];  // not sure we really need to store this, but do it for now anyway
                playlistRangeFieldIsSet = true;
                // now parse the entry
                /*playlistRangeField.setPtrLen("",0);
                playlistRangeFieldIsSet = false;
                playlistErrorFieldIsSet = false;
                playlistErrorFieldCount = 0;
                playlistRangeUrl.setPtrLen("",0);
                playlistRangeClipIndex=0;
                playlistRangeClipOffset=0;
                playlistRangeNptTime=0;*/
                char* curr = valuePtr; // set curr to the start of the entry after the colon, whitespace was already removed
                StrPtrLen eStr;

                eStr = curr;
                // look for playlist_play_time
                const StrPtrLen RtspPlaylistPlayTimeStr = "playlist_play_time";
                if (0 && RtspPlaylistPlayTimeStr.isCIPrefixOf(eStr))
                {
                    curr += 18; // move past "playlist_play_time"
                    // eat through leading whitespace
                    while ((*curr >= 0x09 && *curr <= 0x0D) || *curr == 0x20)
                    {
                        ++curr;
                    }
                    if (*curr != CHAR_EQUAL)
                    {
                        // problem
                    }
                    // could be more whitespace
                    while ((*curr >= 0x09 && *curr <= 0x0D) || *curr == 0x20)
                    {
                        ++curr;
                    }
                    if (*curr != CHAR_LT)
                    {
                        // problem
                    }
                    ++curr; // move past '<'
                    // could be even more whitespace
                    while ((*curr >= 0x09 && *curr <= 0x0D) || *curr == 0x20)
                    {
                        ++curr;
                    }
                    // now comes the url
                    // find the end first
                    char* end = curr;
                    while (*end != CHAR_COMMA)
                    {
                        ++end;
                    }
                    char* endtmp = end;
                    // may have trailing whitespace
                    if (*(endtmp - 1) == 0x09 || *(endtmp - 1) == 0x0D || *(endtmp - 1) == 0x20)
                    {
                        --endtmp;
                        while ((*endtmp >= 0x09 && *endtmp <= 0x0D) || *endtmp == 0x20)
                        {
                            --endtmp;
                        }
                        // will be on a non space char, move it over
                        ++endtmp;
                    }
                    // to do - not sure if we need a '\0' here..
                    playlistRangeUrl.setPtrLen(curr, endtmp - curr);

                    curr = end + 1;
                    // next should be the clip index
                    // of course, could be more whitespace
                    while ((*curr >= 0x09 && *curr <= 0x0D) || *curr == 0x20)
                    {
                        ++curr;
                    }
                    // find next comma
                    end = curr;
                    while (*end != CHAR_COMMA)
                    {
                        ++end;
                    }
                    // may have trailing whitespace
                    // dont worry about it for now  atoi should be ok
                    /*if(*(end-1) == 0x09 ||*(end-1) == 0x0D ||*(end-1) == 0x20)
                    {
                    --end;
                    while( (*end >= 0x09 && *end <= 0x0D) || *end == 0x20 )
                    {
                    --end;
                    }
                    // will be on a non space char, move it over
                    ++end;
                    }*/
                    // finally get the value
                    uint32 atoi_tmp;
                    PV_atoi(curr, 'd', atoi_tmp);
                    playlistRangeClipIndex = atoi_tmp;

                    curr = end + 1;
                    // next should be the clip offset
                    // of course, could be more whitespace
                    while ((*curr >= 0x09 && *curr <= 0x0D) || *curr == 0x20)
                    {
                        ++curr;
                    }
                    // find next comma
                    end = curr;
                    while (*end != CHAR_GT)
                    {
                        ++end;
                    }
                    // may have trailing whitespace
                    // dont worry about it for now  atoi should be ok
                    /*if(*(end-1) == 0x09 ||*(end-1) == 0x0D ||*(end-1) == 0x20)
                    {
                    --end;
                    while( (*end >= 0x09 && *end <= 0x0D) || *end == 0x20 )
                    {
                    --end;
                    }
                    // will be on a non space char, move it over
                    ++end;
                    }*/
                    // finally get the value
                    PV_atoi(curr, 'd', atoi_tmp);
                    playlistRangeClipOffset = atoi_tmp;

                }
                else
                {
                    // problem
                }
            }
            if (fieldKeys[ numPtrFields ].isCIEquivalentTo(
                        RtspRecognizedFieldPlaylistError))
            {
                if (playlistErrorFieldCount < RTSP_MAX_NUMBER_OF_PLAYLIST_ERROR_ENTRIES)
                {
                    playlistErrorField[playlistErrorFieldCount++] = fieldVals[ numPtrFields ];  // this needs to be an array since the number can be unbounded
                    playlistErrorFieldIsSet = true;
                }
            }
#endif
        }
        ptr = endOfValue + 1;
    }
    return true;
}

#endif

