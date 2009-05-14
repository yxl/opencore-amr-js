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
#include "oscl_dll.h"
// Define entry point for this DLL
OSCL_DLL_ENTRY_POINT_DEFAULT()

// This function searches for a substring in a string, just like strstr(), but
// with a couple of specific features:
//  1. It is case-insensitive, so "Ces" can be found in the word "success";
//  2. It is looking for substrings left-to-right, and if the string starts
//  matching, the state of search gets increased; if the whole string does not
//  match, this function does NOT step back; therefore, the string "KoKoMo"
//  will not be found correctly in the string "KoKoKoMo"; however, it will
//  work for substrings that have unique beginnings that don't repeat
//  themselves, such as "Content-length" ('c' is never repeated). Therefore,
//  such a search works for "Content-length", which is all that RtspParser
//  needs.
//
// This function is not part of OSCL precisely because of its limitations;
// however, these limitations are ok here because they contribute to the
// overall performance by not doing extra steps.
//
// added the size argument to deal with Real cloaking "H\02\00\00" 02/22/06
static inline char *
ci_local_strstr(char * bigString, int32 bigStringSize,
                const StrPtrLen & subString
               )
{
    uint32 state = 0;
    uint32 targetState = subString.length();
    char * pp;
    //char * eos = bigString + oscl_strlen(bigString);
    char * eos = bigString + bigStringSize;

    for (pp = bigString;
            pp != eos;
            ++pp
        )
    {
        if (OSCL_ASCII_CASE_MAGIC_BIT
                == (((*pp) ^(subString.c_str()[state])) | OSCL_ASCII_CASE_MAGIC_BIT)
           )
        {
            ++state;
            if (state == targetState)
            {
                return pp - targetState + 1;
            }
        }
        else
        {
            state = 0;
        }

    }

    return NULL;
}


static inline bool
isspaceNotNL(char ch)
{
    if ((CHAR_CR == ch) || (CHAR_LF == ch) || (CHAR_NULL == ch))
    {
        return false;
    }

    return (0 != ((ch >= 0x09 && ch <= 0x0D) || ch == 0x20));
}

OSCL_EXPORT_REF RTSPParser::ParserState
RTSPParser::getState()
{
    ParserState stateToReturn;

    switch (internalState)
    {
        case	IS_WAITING_FOR_REQUEST_MEMORY:
            stateToReturn = WAITING_FOR_REQUEST_MEMORY;
            break;

        case	IS_LOOKING_FOR_END_OF_REQUEST:
        case  IS_CONTINUING_TO_FILL_OUT_ENTITY_BODY:
        case  IS_SKIPPING_OVER_ENTITY_BODY:
        case  IS_LOOKING_FOR_RESYNC:
        case  IS_CONTINUING_TO_FILL_OUT_EMBEDDED_DATA:
        case  IS_SKIPPING_OVER_EMBEDDED_DATA:
            stateToReturn = WAITING_FOR_DATA;
            break;

        case  IS_START_LOOKING_FOR_RESYNC:
            stateToReturn = ERROR_REQUEST_TOO_BIG;
            internalState = IS_LOOKING_FOR_RESYNC;
            break;

        case  IS_WAITING_FOR_ENTITY_BODY_MEMORY:
            stateToReturn = WAITING_FOR_ENTITY_BODY_MEMORY;
            break;

        case IS_WAITING_FOR_EMBEDDED_DATA_MEMORY:
            stateToReturn = WAITING_FOR_EMBEDDED_DATA_MEMORY;
            break;

        case  IS_ERROR_REQUEST_TOO_BIG:
        case 	IS_REQUEST_IS_READY:
            // both states are processed the same way,
            // except that the state returned is different
            //
            stateToReturn = (IS_REQUEST_IS_READY == internalState) ?
                            REQUEST_IS_READY : ERROR_REQUEST_TOO_BIG;

            // take care of the entity bodies
            if (0 == ebFullSizeExpected)
            { // no eb expected
                internalState = IS_WAITING_FOR_REQUEST_MEMORY;
            }
            else
            { // check to see the nature of the entity body. if it
                // is embedded data, change internal state appropriately
                if (requestStruct->method != METHOD_BINARY_DATA)
                {
                    internalState = IS_WAITING_FOR_ENTITY_BODY_MEMORY;
                }
                else
                {
                    internalState = IS_WAITING_FOR_EMBEDDED_DATA_MEMORY;
                }
            }
            continueProcessing();

            break;

        case  IS_ENTITY_BODY_IS_READY:
            stateToReturn = ENTITY_BODY_IS_READY;
            internalState = IS_WAITING_FOR_REQUEST_MEMORY;
            break;

        case IS_EMBEDDED_DATA_IS_READY:
            stateToReturn = EMBEDDED_DATA_IS_READY;
            internalState = IS_WAITING_FOR_REQUEST_MEMORY;
            break;

        default:

            // now, this is an internal error
            internalState = IS_INTERNAL_ERROR;
            stateToReturn = INTERNAL_ERROR;
    }

    return stateToReturn;
}

OSCL_EXPORT_REF RTSPParser::RTSPParser()
{
    flush();
};

void
RTSPParser::continueProcessing()
{
    switch (internalState)
    {
        case IS_WAITING_FOR_REQUEST_MEMORY:
            break;

        case IS_LOOKING_FOR_END_OF_REQUEST:
            lookForEndOfRequest();
            break;

        case IS_START_LOOKING_FOR_RESYNC:
        case IS_LOOKING_FOR_RESYNC:
            lookForResync();
            break;

        case IS_WAITING_FOR_ENTITY_BODY_MEMORY:
            // nothing to do, really
            break;

        case IS_SKIPPING_OVER_ENTITY_BODY:
            skipOverEntityBody();
            break;

        case IS_STARTING_TO_FILL_OUT_ENTITY_BODY:
            startFillingOutEntityBody();
            break;

        case IS_CONTINUING_TO_FILL_OUT_ENTITY_BODY:
            break;

        case IS_WAITING_FOR_EMBEDDED_DATA_MEMORY:
            break;

        case IS_SKIPPING_OVER_EMBEDDED_DATA:
            skipOverEmbeddedData();
            break;

        case IS_STARTING_TO_FILL_OUT_EMBEDDED_DATA:
            startFillingOutEmbeddedData();
            break;

        case IS_CONTINUING_TO_FILL_OUT_EMBEDDED_DATA:
            break;


        default:
            internalState = IS_INTERNAL_ERROR;
    }
}

OSCL_EXPORT_REF bool
RTSPParser::registerNewRequestStruct(RTSPIncomingMessage * newRequestStruct)
{
    if (IS_WAITING_FOR_REQUEST_MEMORY != internalState)
    {
        return false;
    }

    requestStruct = newRequestStruct;

    internalState = IS_LOOKING_FOR_END_OF_REQUEST;

    continueProcessing();

    return true;
}

OSCL_EXPORT_REF const StrPtrLen *
RTSPParser::getDataBufferSpec()
{
    if (internalState == IS_LOOKING_FOR_END_OF_REQUEST
            ||  internalState == IS_START_LOOKING_FOR_RESYNC
            ||  internalState == IS_LOOKING_FOR_RESYNC
       )
    {
        // normal case - just put stuff into the buffer
        dataBufferSpec.setPtrLen(mainBufferSpace,
                                 mainBuffer + RTSP_PARSER_BUFFER_SIZE - mainBufferSpace);
    }

    else if (internalState == IS_CONTINUING_TO_FILL_OUT_ENTITY_BODY
             ||	internalState == IS_CONTINUING_TO_FILL_OUT_EMBEDDED_DATA
            )
    {

        // the whole entity body arithmetic is done here, not in
        // registerDataWritten(), so here it comes
        while (entityBody[ebCurrentIndex].len == ebCurrentOffset)
        {
            ++ebCurrentIndex;
            ebCurrentOffset = 0;
        }

        // so, we are at the appropriate fragment;
        // figure out which is smaller
        //
        uint32 sizePossibleToWrite =
            entityBody[ebCurrentIndex].len - ebCurrentOffset;
        uint32 sizeLeftToWrite = ebFullSizeExpected - ebSizeCoveredSoFar;

        // set the data buffer spec
        dataBufferSpec.setPtrLen(
            static_cast<char*>(entityBody[ebCurrentIndex].ptr) + ebCurrentOffset,
            (sizePossibleToWrite < sizeLeftToWrite) ?
            sizePossibleToWrite : sizeLeftToWrite
        );
    }

    else if (IS_SKIPPING_OVER_ENTITY_BODY == internalState)
    {
        mainBufferEntry = mainBuffer;
        mainBufferSpace = mainBuffer;

        dataBufferSpec.setPtrLen(mainBufferSpace, RTSP_PARSER_BUFFER_SIZE);
    }

    else
    {
        // it's some kind of inconsistency on the Engine's part, or Parser's
        // internal error

        return NULL;
    }

    return & dataBufferSpec;
}

void
RTSPParser::lookForEndOfRequest()
{
    // eorptr = mainBufferEntry;
    uint32	 newMessageSize;

    *mainBufferSpace = CHAR_NULL;

    bool  shouldMoveOverToBeginning = false;

    // now, it's either a binary data thing, or a regular message
    if (CHAR_DOLLAR == *mainBufferEntry)
    {

        if (mainBufferSpace - mainBufferEntry < 4)
        { // not a complete message
            shouldMoveOverToBeginning = true;
        }
        else
        { // it is a complete thing
            requestStruct->msgType = RTSPRequestMsg;
            requestStruct->method = METHOD_BINARY_DATA;

            requestStruct->contentLength =
                ((static_cast<uint16>(
                      (*(reinterpret_cast<unsigned char*>(mainBufferEntry + 2))
                      ))) << 8)
                + static_cast<uint16>(
                    *(reinterpret_cast<unsigned char*>(mainBufferEntry + 3))
                );
            requestStruct->contentLengthIsSet = true;

            *(mainBufferEntry + 2) = CHAR_NULL;
            requestStruct->contentType = mainBufferEntry + 1;
            requestStruct->contentTypeIsSet = true;

            requestStruct->channelID = static_cast<uint8>(
                                           *(reinterpret_cast<unsigned char*>(mainBufferEntry + 1)));

            mainBufferEntry += 4;

            eorptr = mainBufferEntry;

            ebFullSizeExpected = requestStruct->contentLength;

            internalState = IS_REQUEST_IS_READY;

            return;
        }
    }

    else
    { // it's a normal message
        bool found = false;

        for (/*eorptr = mainBufferEntry*/; eorptr < mainBufferSpace - 1; ++eorptr)
        {
            if (CHAR_LF == *eorptr || CHAR_CR == *eorptr)
            {	// it's a possible

                // is it two newlines?
                if (*eorptr == *(eorptr + 1))
                {	// yes, CR-CR or LF-LF format

                    found = true;
                    eorptr += 2;
                    break;
                }
                else if ((eorptr <= mainBufferSpace - 4)
                         && (CHAR_CR == *(eorptr)) && (CHAR_LF == *(eorptr + 1))
                         && (CHAR_CR == *(eorptr + 2)) && (CHAR_LF == *(eorptr + 3))
                        )
                {	// yes, MS-WINDOWS format

                    found = true;
                    eorptr += 4;
                    break;
                }
                // else, continue on
            }
        }

        if (found)
        {
            // transfer the buffer, if necessary

            newMessageSize = eorptr - mainBufferEntry;

            // quickly take a peek at content-length
            char * cl = ci_local_strstr(mainBufferEntry, newMessageSize,
                                        RtspRecognizedFieldContentLength);
            if (NULL == cl)
            { // nothing visible
                ebFullSizeExpected = 0;
            }
            else if (cl >= mainBufferEntry + newMessageSize)
            { // it's not part of this particular message
                ebFullSizeExpected = 0;
            }
            else
            {
                cl += oscl_strlen(RtspRecognizedFieldContentLength);
                while (cl < eorptr && (isspaceNotNL(*cl) || (CHAR_COLON == *cl)))
                {
                    ++cl;
                }

                uint32 atoi_tmp;
                PV_atoi(cl, 'd', atoi_tmp);
                ebFullSizeExpected = atoi_tmp;
            }

            // now, on with the moving around ...

            if (RTSP_MAX_FULL_REQUEST_SIZE < newMessageSize)
            {
                // request too big

                oscl_memcpy(requestStruct->secondaryBuffer, mainBufferEntry,
                            RTSP_MAX_FULL_REQUEST_SIZE);
                requestStruct->secondaryBuffer[RTSP_MAX_FULL_REQUEST_SIZE] = CHAR_NULL;

                requestStruct->secondaryBufferSizeUsed = newMessageSize;

                requestStruct->amMalformed = RTSPErrorTooBig;

                mainBufferEntry += newMessageSize;

                eorptr = mainBufferEntry;

                internalState = IS_ERROR_REQUEST_TOO_BIG;
            }
            else
            {
                // everything is ok

                oscl_memcpy(requestStruct->secondaryBuffer, mainBufferEntry,
                            newMessageSize);
                requestStruct->secondaryBuffer[ newMessageSize ] = CHAR_NULL;

                requestStruct->secondaryBufferSizeUsed = newMessageSize;

                mainBufferEntry += newMessageSize;

                eorptr = mainBufferEntry;

                internalState = IS_REQUEST_IS_READY;
            }

            dealWithLineContinuations(requestStruct);

            requestStruct->parseFirstFields();

        }
        else
        {
            shouldMoveOverToBeginning = true;
        }
    }

    if (shouldMoveOverToBeginning)
    {
        // i.e. end of request was not found

        int sizeUsedSoFar = mainBufferSpace - mainBufferEntry;

        if (RTSP_PARSER_BUFFER_SIZE == sizeUsedSoFar)
        {	// we hit the parser's buffer size
            internalState = IS_START_LOOKING_FOR_RESYNC;
            continueProcessing();

            return;
        }

        if (mainBufferEntry != mainBuffer)
        {
            oscl_memmove(mainBuffer, mainBufferEntry, sizeUsedSoFar);
            mainBufferEntry = mainBuffer;
            eorptr = mainBufferEntry;
            mainBufferSpace = mainBufferEntry + sizeUsedSoFar;
        }
        else
        { // rewind eorptr

            eorptr -= 4;

            eorptr = (eorptr < mainBufferEntry) ? mainBufferEntry : eorptr;
        }
    }
}

OSCL_EXPORT_REF bool
RTSPParser::registerDataBufferWritten(uint32 sizeWritten)
{
    // take care of entity bodies and stuff

    if (IS_LOOKING_FOR_END_OF_REQUEST == internalState)
    { // the memory being filled out is Parser's
        mainBufferSpace += sizeWritten;

        continueProcessing();

        return true;
    }
    else if (IS_CONTINUING_TO_FILL_OUT_ENTITY_BODY == internalState
             ||	IS_CONTINUING_TO_FILL_OUT_EMBEDDED_DATA == internalState
            )
    { // the memory being filled out is Engine's entity body
        ebCurrentOffset += sizeWritten;
        ebSizeCoveredSoFar += sizeWritten;

        if (ebSizeCoveredSoFar == ebFullSizeExpected)
        {
            eorptr = mainBufferEntry;
            if (internalState == IS_CONTINUING_TO_FILL_OUT_ENTITY_BODY)
            {
                internalState = IS_ENTITY_BODY_IS_READY;
            }
            else // internalState=IS_CONTINUING_TO_FILL_OUT_EMBEDDED_DATA
            {
                internalState = IS_EMBEDDED_DATA_IS_READY;
            }
        }

        return true;
    }

    else if (IS_SKIPPING_OVER_ENTITY_BODY == internalState)
    {
        mainBufferSpace += sizeWritten;
        continueProcessing();
    }

    else if (IS_START_LOOKING_FOR_RESYNC == internalState
             ||  IS_LOOKING_FOR_RESYNC == internalState
            )
    {
        mainBufferSpace += sizeWritten;
        continueProcessing();
    }

    else
    {
        // some kind of error on Engine's part, or Parser's internal inconsistency
        return false;
    }

    return false; // to appease compiler
}

OSCL_EXPORT_REF bool
RTSPParser::registerEntityBody(RTSPEntityBody * newBody)
{
    if (IS_WAITING_FOR_ENTITY_BODY_MEMORY != internalState)
    {
        return false;
    }

    entityBody = newBody;

    if (NULL == entityBody)
    {
        internalState = IS_SKIPPING_OVER_ENTITY_BODY;
        ebSizeCoveredSoFar = 0;
        ebCurrentIndex = 0;
        ebCurrentOffset = 0;
    }
    else
    {
        internalState = IS_STARTING_TO_FILL_OUT_ENTITY_BODY;
        ebSizeCoveredSoFar = 0;
        ebCurrentIndex = 0;
        ebCurrentOffset = 0;
    }

    continueProcessing();

    return true;
}

void
RTSPParser::skipOverEntityBody()
{
    uint32 sizeNeededToSkip = ebFullSizeExpected - ebSizeCoveredSoFar;
    uint32 sizePossibleToSkip = mainBufferSpace - mainBufferEntry;

    uint32 sizeSkipped = (sizeNeededToSkip < sizePossibleToSkip) ?
                         sizeNeededToSkip : sizePossibleToSkip;

    ebSizeCoveredSoFar += sizeSkipped;
    mainBufferEntry += sizeSkipped;

    if (ebSizeCoveredSoFar == ebFullSizeExpected)
    {
        eorptr = mainBufferEntry;
        internalState = IS_WAITING_FOR_REQUEST_MEMORY;
    }
}

void
RTSPParser::startFillingOutEntityBody()
{
    while (ebFullSizeExpected != ebSizeCoveredSoFar)
    {
        // find an appropriate fragment
        if (entityBody[ebCurrentIndex].len == ebCurrentOffset)
        {
            ++ebCurrentIndex;
            ebCurrentOffset = 0;
            continue;
        }

        uint32 fragmentSizeAvailable = entityBody[ebCurrentIndex].len - ebCurrentOffset;

        // decide, which is smaller - size left in the main buffer, or size left
        // in the current target buffer
        uint32 sizeAvailableForWriting = mainBufferSpace - mainBufferEntry;
        uint32 sizeActuallyToBeWritten =
            (fragmentSizeAvailable < sizeAvailableForWriting) ?
            fragmentSizeAvailable : sizeAvailableForWriting;

        // now decide, which is smaller - the previous result or the size actually
        // left to fill out
        uint32  sizeLeftToWrite = ebFullSizeExpected - ebSizeCoveredSoFar;
        sizeActuallyToBeWritten = (sizeActuallyToBeWritten < sizeLeftToWrite) ?
                                  sizeActuallyToBeWritten : sizeLeftToWrite;

        if (0 == sizeActuallyToBeWritten)
        { // ran out of available data

            // now, further filling out will actually be done by the Engine, we'll
            // just direct it to memory within entity body;
            //
            // this means a change of states
            //
            internalState = IS_CONTINUING_TO_FILL_OUT_ENTITY_BODY;

            return;
        }

        // memory comes from different sources, so we can copy it without fear of
        // overlaps

        oscl_memcpy(static_cast<char*>(entityBody[ebCurrentIndex].ptr) + ebCurrentOffset,
                    mainBufferEntry,
                    sizeActuallyToBeWritten
                   );

        ebCurrentOffset += sizeActuallyToBeWritten;
        mainBufferEntry += sizeActuallyToBeWritten;

        ebSizeCoveredSoFar += sizeActuallyToBeWritten;

    }

    // at this point, either we covered the whole thing, or we didn't

    // if we covered the whole thing, we jumped out from the loop according to
    // the loop invariant
    //
    // if, however, we did not fill out the whole thing, then we have already
    // returned from within the loop, switching to hungry state
    //
    // in other words, at this point we could ONLY succeed

    eorptr = mainBufferEntry;

    internalState = IS_ENTITY_BODY_IS_READY;
}

OSCL_EXPORT_REF bool
RTSPParser::registerEmbeddedDataMemory(RTSPEntityBody * newBody)
{
    if (IS_WAITING_FOR_EMBEDDED_DATA_MEMORY != internalState)
    {
        return false;
    }

    entityBody = newBody;

    if (NULL == entityBody)
    {
        internalState = IS_SKIPPING_OVER_EMBEDDED_DATA;
        ebSizeCoveredSoFar = 0;
        ebCurrentIndex = 0;
        ebCurrentOffset = 0;
    }
    else
    {
        internalState = IS_STARTING_TO_FILL_OUT_EMBEDDED_DATA;
        ebSizeCoveredSoFar = 0;
        ebCurrentIndex = 0;
        ebCurrentOffset = 0;
    }

    continueProcessing();

    return true;
}

void
RTSPParser::skipOverEmbeddedData()
{
    uint32 sizeNeededToSkip = ebFullSizeExpected - ebSizeCoveredSoFar;
    uint32 sizePossibleToSkip = mainBufferSpace - mainBufferEntry;

    uint32 sizeSkipped = (sizeNeededToSkip < sizePossibleToSkip) ?
                         sizeNeededToSkip : sizePossibleToSkip;

    ebSizeCoveredSoFar += sizeSkipped;
    mainBufferEntry += sizeSkipped;

    if (ebSizeCoveredSoFar == ebFullSizeExpected)
    {
        eorptr = mainBufferEntry;
        internalState = IS_WAITING_FOR_REQUEST_MEMORY;
    }
}

void
RTSPParser::startFillingOutEmbeddedData()
{
    while (ebFullSizeExpected != ebSizeCoveredSoFar)
    {    // find an appropriate fragment
        if (entityBody[ebCurrentIndex].len == ebCurrentOffset)
        {
            ++ebCurrentIndex;
            ebCurrentOffset = 0;
            continue;
        }

        uint32 fragmentSizeAvailable = entityBody[ebCurrentIndex].len - ebCurrentOffset;

        // decide, which is smaller - size left in the main buffer, or size left
        // in the current target buffer
        //
        uint32 sizeAvailableForWriting = mainBufferSpace - mainBufferEntry;
        uint32 sizeActuallyToBeWritten =
            (fragmentSizeAvailable < sizeAvailableForWriting) ?
            fragmentSizeAvailable : sizeAvailableForWriting;

        // now decide, which is smaller - the previous result or the size actually
        // left to fill out
        uint32  sizeLeftToWrite = ebFullSizeExpected - ebSizeCoveredSoFar;
        sizeActuallyToBeWritten = (sizeActuallyToBeWritten < sizeLeftToWrite) ?
                                  sizeActuallyToBeWritten : sizeLeftToWrite;

        if (0 == sizeActuallyToBeWritten)
        { // ran out of available data

            // now, further filling out will actually be done by the Engine, we'll
            // just direct it to memory within entity body;
            //
            // this means a change of states
            //
            internalState = IS_CONTINUING_TO_FILL_OUT_EMBEDDED_DATA;

            return;
        }

        // memory comes from different sources, so we can copy it without fear of
        // overlaps

        oscl_memcpy(static_cast<char*>(entityBody[ebCurrentIndex].ptr) + ebCurrentOffset,
                    mainBufferEntry,
                    sizeActuallyToBeWritten
                   );

        ebCurrentOffset += sizeActuallyToBeWritten;
        mainBufferEntry += sizeActuallyToBeWritten;

        ebSizeCoveredSoFar += sizeActuallyToBeWritten;

    }

    // at this point, either we covered the whole thing, or we didn't

    // if we covered the whole thing, we jumped out from the loop according to
    // the loop invariant
    //
    // if, however, we did not fill out the whole thing, then we have already
    // returned from within the loop, switching to hungry state
    //
    // in other words, at this point we could ONLY succeed

    eorptr = mainBufferEntry;

    internalState = IS_EMBEDDED_DATA_IS_READY;
}

OSCL_EXPORT_REF void
RTSPParser::flush(void)
{
    internalState = IS_WAITING_FOR_REQUEST_MEMORY;
    mainBufferEntry = mainBuffer;
    mainBufferSpace = mainBuffer;
    mainBufferSizeUsed = 0;
    eorptr = mainBuffer;
    mainBuffer[ RTSP_PARSER_BUFFER_SIZE ] = CHAR_NULL;
    mainBuffer[ RTSP_PARSER_BUFFER_SIZE+1 ] = CHAR_NULL;
}

void
RTSPParser::dealWithLineContinuations(RTSPIncomingMessage * theStruct)
{
    char * cPtr;      // current pointer
    char * nlEnd;     // new-line end pointer
    char * finishPtr; // end-of-buffer pointer

    finishPtr = theStruct->secondaryBuffer + theStruct->secondaryBufferSizeUsed;

    for (cPtr = theStruct->secondaryBuffer;
            cPtr < finishPtr;
        )
    {
        // check if this point is suspicious
        //
        if (CHAR_CR == *cPtr)
        {
            if (cPtr < finishPtr - 1      // there's room for CR-LF
                    &&  CHAR_LF == *(cPtr + 1)  // next char is LF
               )
            {
                nlEnd = cPtr + 1;

                // newline, CR-LF
            }
            else
            {
                nlEnd = cPtr;

                // newline, CR
            }
        }
        else if (CHAR_LF == *cPtr)
        {
            nlEnd = cPtr;

            // newline, LF
        }
        else
        { // not a newline
            ++cPtr;
            continue;
        }

        // it was a newline

        // now, is there a whitespace after it?
        if (nlEnd >= finishPtr      // could be no room for a whitespace
                || (CHAR_SPACE != *(nlEnd + 1)      // could be a non-whitespace
                    &&  CHAR_TAB   != *(nlEnd + 1)
                   )
           )
        {
            cPtr = nlEnd + 1;
            continue;
        }

        char * sPtr;
        for (sPtr = nlEnd + 1;
                sPtr < finishPtr
                && (CHAR_SPACE == *sPtr
                    ||  CHAR_TAB   == *sPtr
                   );
                ++sPtr
            )
        { // nothing
            ;;;;
        }

        size_t sizeToMove = finishPtr - sPtr + 1;
        size_t sizeCut = sPtr - cPtr - 1;

        // set a space
        * cPtr = CHAR_SPACE;

        // move the rest, careful with overlaps
        oscl_memmove(cPtr + 1, sPtr, sizeToMove);

        // reset the length
        theStruct->secondaryBufferSizeUsed -= sizeCut;

        // reiterate
        finishPtr -= sizeCut;
        ++cPtr; // safe enough, still one whitespace has been written

        continue;
    }
}


void
RTSPParser::dealWithFieldRepetitions(RTSPIncomingMessage * theStruct)
{
    char * cPtr;
    char * finishPtr =
        theStruct->secondaryBuffer + theStruct->secondaryBufferSizeUsed;
    char * nlEnd;

    for (cPtr = theStruct->secondaryBuffer;
            cPtr < finishPtr;
            ++cPtr
        )
    {
        if (CHAR_CR == *cPtr)
        {
            if (cPtr < finishPtr - 1
                    &&  CHAR_LF == *(cPtr + 1)
               )
            {
                nlEnd = cPtr + 1;
            }
            else
            {
                nlEnd = cPtr;
            }
        }
        else if (CHAR_LF == *cPtr)
        {
            nlEnd = cPtr;
        }
        else
        {
            continue;
        }

        char * fieldNamePtr = nlEnd + 1;

        if (fieldNamePtr >= finishPtr
                ||  CHAR_CR     == *fieldNamePtr
                ||  CHAR_LF     == *fieldNamePtr
                ||  CHAR_SPACE  == *fieldNamePtr
                ||  CHAR_TAB    == *fieldNamePtr
           )
        {
            // not a field, an end of message or an empty string

            continue;
        }

        // we have a field
    }
}


void
RTSPParser::lookForResync()
{
    bool found = false;

    for (; eorptr < mainBufferSpace - 1; ++eorptr)
    {
        if (CHAR_LF == *eorptr || CHAR_CR == *eorptr)
        {
            if (*eorptr == *(eorptr + 1))
            {
                found = true;
                eorptr += 2;
                break;
            }
            else if ((eorptr <= mainBufferSpace - 4)
                     && (CHAR_CR == *(eorptr)) && (CHAR_LF == *(eorptr + 1))
                     && (CHAR_CR == *(eorptr + 2)) && (CHAR_LF == *(eorptr + 3))
                    )
            {
                found = true;
                eorptr += 4;
                break;
            }
        }
    }

    if (found)
    {
        mainBufferEntry = eorptr;

        if (mainBufferEntry == mainBufferSpace)
        {
            mainBufferEntry = mainBuffer;
            eorptr = mainBufferEntry;
            mainBufferSpace = mainBufferEntry;
        }

        internalState = IS_WAITING_FOR_REQUEST_MEMORY;

        continueProcessing();
    }
    else
    {
        int sizeToMove = (RTSP_RESYNC_PRESERVE_SIZE < (mainBufferSpace - mainBufferEntry)) ?
                         RTSP_RESYNC_PRESERVE_SIZE : (mainBufferSpace - mainBufferEntry);

        oscl_memmove(mainBuffer, mainBufferSpace - sizeToMove, sizeToMove);
        mainBufferEntry = mainBuffer;
        mainBufferSpace = mainBufferEntry + sizeToMove;
        eorptr = mainBuffer;
    }

    return;
}
