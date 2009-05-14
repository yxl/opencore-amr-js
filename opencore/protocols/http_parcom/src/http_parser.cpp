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
#include "http_parser.h"
#include "http_parser_internal.h"
#include "oscl_string_utils.h"
#include "oscl_string_containers.h"

// Use default DLL entry point for Symbian
#include "oscl_dll.h"

OSCL_DLL_ENTRY_POINT_DEFAULT()


// three inline functions for multiple class implementation
inline bool isLetter(const char c)
{
    return ((c >= 65 && c <= 90) || (c >= 97 && c <= 122) || (c == 45)); // A-Z, or a-z or -
}

inline bool isDigit(const char c)
{
    return (c >= 48 && c <= 57);
}

inline bool isHexDigit(const char c)
{
    return (isDigit(c) || (c >= 65 && c <= 70) || (c >= 97 && c <= 102)); // 0-9, A-F or a-f
}


////////////////////////////////////////////////////////////////////////////////////
////// HTTPParser implementation ///////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

OSCL_EXPORT_REF int32 HTTPParser::parse(const OsclRefCounterMemFrag &aInputDataStream, RefCountHTTPEntityUnit &aEntityUnit)
{
    if (!iParserInput->push_back((OsclRefCounterMemFrag &)aInputDataStream)) // not a new data fragment
        return PARSE_NEED_MORE_DATA;

    if (!aEntityUnit.empty()) aEntityUnit.clear();
    if (!iHeader->isParsed()) return iHeader->parse(*iParserInput, aEntityUnit);

    return parseEntityBody(aEntityUnit);
}

////////////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF int32 HTTPParser::doSanityCheckForResponseHeader()
{
    if (!iHeader->isParsed()) return PARSE_HEADER_NOT_PARSED_YET;
    return iHeader->doSanityCheckForResponseHeader();
}


////////////////////////////////////////////////////////////////////////////////////
// assume aFieldKeyList has enough space to hold all parsed key list, if it is small, it will cause crash
// since we have no way to check the space
OSCL_EXPORT_REF uint32 HTTPParser::getFieldKeyListInHeader(StrPtrLen *&aFieldKeyList)
{
    return iHeader->getKeyValuesStore()->getCurrentKeyList(aFieldKeyList); // iHeader should be created successfully in factory method
}

////////////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF bool HTTPParser::getField(const StrCSumPtrLen &aNewFieldName, StrPtrLen &aNewFieldValue, uint32 index)
{
    return iHeader->getField(aNewFieldName, aNewFieldValue, index);
}

////////////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF uint32 HTTPParser::getNumberOfFieldsByKey(const StrCSumPtrLen &aNewFieldName)
{
    return iHeader->getNumberOfFieldsByKey(aNewFieldName);
}

////////////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF uint32 HTTPParser::getTotalFieldsInHeader()
{
    return iHeader->getNumFields();
}

////////////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF uint32 HTTPParser::getHTTPStatusCode()
{
    return iHeader->getStatusCode();
}

////////////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void HTTPParser::getContentInfo(HTTPContentInfo &aContentInfo)
{
    aContentInfo.clear();
    if (iContentInfo) iContentInfo->get(aContentInfo);
}


////////////////////////////////////////////////////////////////////////////////////
// reset the parser to parse a new HTTP response
OSCL_EXPORT_REF void HTTPParser::reset()
{
    if (iParserInput) iParserInput->clear();
    if (iContentInfo) iContentInfo->clear();
    if (iHeader) iHeader->reset();

    // delete iEntityBody
    if (iEntityBody) OSCL_DELETE(iEntityBody);
    iEntityBody = NULL;
}

////////////////////////////////////////////////////////////////////////////////////
// factory method
OSCL_EXPORT_REF HTTPParser* HTTPParser::create()
{
    HTTPParser *parser = OSCL_NEW(HTTPParser, ());
    if (!parser) return NULL;
    if (!parser->construct())
    {
        OSCL_DELETE(parser);
        return NULL;
    }
    return parser;
}

////////////////////////////////////////////////////////////////////////////////////
bool HTTPParser::construct()
{
    reset();

    // create the component objects
    if ((iContentInfo = OSCL_NEW(HTTPContentInfoInternal, ())) == NULL) return false;
    if ((iParserInput = HTTPParserInput::create()) == NULL) return false;
    if ((iHeader = HTTPParserHeaderObject::create(iContentInfo)) == NULL) return false;
    return true;
}

////////////////////////////////////////////////////////////////////////////////////
// destructor
OSCL_EXPORT_REF HTTPParser::~HTTPParser()
{
    reset();

    // delete iParserInput
    if (iParserInput) OSCL_DELETE(iParserInput);
    iParserInput = NULL;

    // delete iContentInfo
    if (iContentInfo) OSCL_DELETE(iContentInfo);
    iContentInfo = NULL;

    // delete iHeader
    if (iHeader) OSCL_DELETE(iHeader);
    iHeader = NULL;

    // delete iEntityBody
    if (iEntityBody) OSCL_DELETE(iEntityBody);
    iEntityBody = NULL;
}

////////////////////////////////////////////////////////////////////////////////////
int32 HTTPParser::parseEntityBody(RefCountHTTPEntityUnit &aEntityUnit)
{
    if (!iHeader || !iHeader->isParsed()) return PARSE_HEADER_NOT_PARSED_YET;
    if (iHeader->isWholeResponseParsed()) return PARSE_SUCCESS_END_OF_MESSAGE;

    if (!iEntityBody)
    {
        // create iEntityBody
        // After parsing the header, we should get right content type, iContentType
        if (iContentInfo->getContentType() == HTTP_CONTENT_NORMAL)
            iEntityBody = OSCL_NEW(HTTPParserNormalContentObject, (iHeader->getKeyValuesStore(), iHeader->getAllocator(), iContentInfo));
        else if (iContentInfo->getContentType() == HTTP_CONTENT_NULTIPART)
            iEntityBody = OSCL_NEW(HTTPParserMultipartContentObject, (iHeader->getKeyValuesStore(), iHeader->getAllocator(), iContentInfo));
        else if (iContentInfo->getContentType() == HTTP_CONTENT_CHUNKED_TRANSFER_ENCODING)
            iEntityBody = OSCL_NEW(HTTPParserCTEContentObject, (iHeader->getKeyValuesStore(), iHeader->getAllocator(), iContentInfo));

        if (!iEntityBody) return PARSE_MEMORY_ALLOCATION_FAILURE;
    }

    return iEntityBody->parse(*iParserInput, aEntityUnit);
}


/////////////////////////////////////////////////////////////////////////////////////
////////// HTTPContentInfoInternal Implementation ///////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
bool HTTPContentInfoInternal::parseContentInfo(StringKeyValueStore &aKeyValueStore)
{
    // Content-Length
    StrCSumPtrLen contenLengthKey("Content-Length");
    StrPtrLen contentLengthValue;
    if (aKeyValueStore.getValueByKey(contenLengthKey, contentLengthValue))
    {
        // has content length
        PV_atoi(contentLengthValue.c_str(), 'd', iContentLength);
    }

    // Content-Type
    StrCSumPtrLen contentTypeKey("Content-Type");
    StrPtrLen contentTypeValue;
    if (aKeyValueStore.getValueByKey(contentTypeKey, contentTypeValue))
    {
        // has content type
        if (!parseContentType(contentTypeValue)) return false;
    }

    // Content-Range
    StrCSumPtrLen contentRangeKey("Content-Range");
    StrPtrLen contentRangeValue;
    if (aKeyValueStore.getValueByKey(contentRangeKey, contentRangeValue))
    {
        // has content type
        parseContentRange(contentRangeValue);
    }

    // check Chunked Transfer-Encoding, "Transfer-Encoding : chunked"
    StrCSumPtrLen transferEncodingKey("Transfer-Encoding");
    StrPtrLen transferEncodingValue;
    if (aKeyValueStore.getValueByKey(transferEncodingKey, transferEncodingValue))
    {
        // has content type
        verifyTransferEncoding(transferEncodingValue);
    }
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////
void HTTPContentInfoInternal::parseContentRange(const StrPtrLen &aContentRange)
{
    char *ptr = (char *)aContentRange.c_str();
    uint32 len = aContentRange.length();

    while (!isLetter(*ptr) && len > 0)
    {
        ptr++;
        len--;
    }
    OSCL_FastString bytesString(_STRLIT_CHAR("bytes"));
    if (len <= oscl_strlen(bytesString.get_cstr())) return;

    if (((ptr[0] | OSCL_ASCII_CASE_MAGIC_BIT) == 'b') &&
            ((ptr[1] | OSCL_ASCII_CASE_MAGIC_BIT) == 'y') &&
            ((ptr[2] | OSCL_ASCII_CASE_MAGIC_BIT) == 't') &&
            ((ptr[3] | OSCL_ASCII_CASE_MAGIC_BIT) == 'e') &&
            ((ptr[4] | OSCL_ASCII_CASE_MAGIC_BIT) == 's'))
    {
        // find "bytes"
        ptr += 5;
        if ((len -= 5) <= 0) return;

        // get the left side of the range
        while (!isDigit(*ptr) && len > 0)
        {
            ptr++;
            len--;
        }
        char *start_ptr = ptr;
        uint32 start_len = len;
        while (isDigit(*ptr) && len > 0)
        {
            ptr++;
            len--;
        }
        PV_atoi(start_ptr, 'd', start_len - len, iContentRangeLeft);

        // get the right side of the range
        while (!isDigit(*ptr) && len > 0)
        {
            ptr++;
            len--;
        }
        start_ptr = ptr;
        start_len = len;
        while (isDigit(*ptr) && len > 0)
        {
            ptr++;
            len--;
        }
        PV_atoi(start_ptr, 'd', start_len - len, iContentRangeRight);

        // get the content length
        while (!isDigit(*ptr) && len > 0)
        {
            ptr++;
            len--;
        }
        start_ptr = ptr;
        start_len = len;
        while (isDigit(*ptr) && len > 0)
        {
            ptr++;
            len--;
        }
        PV_atoi(start_ptr, 'd', start_len - len, iContentLength);
    }
}

/////////////////////////////////////////////////////////////////////////////////////
bool HTTPContentInfoInternal::parseContentType(const StrPtrLen &aContentType)
{
    // identify multipart content type and chunked transfer-encoding
    // Content-type: multipart/byteranges; boundary=THIS_STRING_SEPARATES
    char *ptr = (char *)aContentType.c_str();
    uint32 len = aContentType.length();

    // eat non-letter characters
    while (!isLetter(*ptr) && len > 0)
    {
        ptr++;
        len--;
    }
    OSCL_FastString typeString(_STRLIT_CHAR("multipart/byteranges"));
    if (len <= oscl_strlen(typeString.get_cstr())) return true;

    if (((ptr[0] | OSCL_ASCII_CASE_MAGIC_BIT) == 'm') &&
            ((ptr[1] | OSCL_ASCII_CASE_MAGIC_BIT) == 'u') &&
            ((ptr[2] | OSCL_ASCII_CASE_MAGIC_BIT) == 'l') &&
            ((ptr[3] | OSCL_ASCII_CASE_MAGIC_BIT) == 't') &&
            ((ptr[4] | OSCL_ASCII_CASE_MAGIC_BIT) == 'i') &&
            ((ptr[5] | OSCL_ASCII_CASE_MAGIC_BIT) == 'p') &&
            ((ptr[6] | OSCL_ASCII_CASE_MAGIC_BIT) == 'a') &&
            ((ptr[7] | OSCL_ASCII_CASE_MAGIC_BIT) == 'r') &&
            ((ptr[8] | OSCL_ASCII_CASE_MAGIC_BIT) == 't') &&
            (ptr[9]								  == '/') &&
            ((ptr[10] | OSCL_ASCII_CASE_MAGIC_BIT) == 'b') &&
            ((ptr[11] | OSCL_ASCII_CASE_MAGIC_BIT) == 'y') &&
            ((ptr[12] | OSCL_ASCII_CASE_MAGIC_BIT) == 't') &&
            ((ptr[13] | OSCL_ASCII_CASE_MAGIC_BIT) == 'e') &&
            ((ptr[14] | OSCL_ASCII_CASE_MAGIC_BIT) == 'r') &&
            ((ptr[15] | OSCL_ASCII_CASE_MAGIC_BIT) == 'a') &&
            ((ptr[16] | OSCL_ASCII_CASE_MAGIC_BIT) == 'n') &&
            ((ptr[17] | OSCL_ASCII_CASE_MAGIC_BIT) == 'g') &&
            ((ptr[18] | OSCL_ASCII_CASE_MAGIC_BIT) == 'e') &&
            ((ptr[19] | OSCL_ASCII_CASE_MAGIC_BIT) == 's'))
    {
        // find "multipart/byteranges"
        // constinue search "boundary"
        ptr += 20;
        if ((len -= 20) <= 8) return false;

        while (!isLetter(*ptr) && len > 0)
        {
            ptr++;
            len--;
        }
        if (((ptr[0] | OSCL_ASCII_CASE_MAGIC_BIT) == 'b') &&
                ((ptr[1] | OSCL_ASCII_CASE_MAGIC_BIT) == 'o') &&
                ((ptr[2] | OSCL_ASCII_CASE_MAGIC_BIT) == 'u') &&
                ((ptr[3] | OSCL_ASCII_CASE_MAGIC_BIT) == 'n') &&
                ((ptr[4] | OSCL_ASCII_CASE_MAGIC_BIT) == 'd') &&
                ((ptr[5] | OSCL_ASCII_CASE_MAGIC_BIT) == 'a') &&
                ((ptr[6] | OSCL_ASCII_CASE_MAGIC_BIT) == 'r') &&
                ((ptr[7] | OSCL_ASCII_CASE_MAGIC_BIT) == 'y'))
        {
            // find "boundary"
            ptr += 8;
            if ((len -= 8) <= 0) return false;

            // find "="
            while (*ptr != HTTP_CHAR_EQUAL && len > 0)
            {
                ptr++;
                len--;
            }
            if (len <= 0) return false;
            ptr++;

            while ((*ptr == HTTP_CHAR_SPACE || *ptr == HTTP_CHAR_TAB) && len > 0)
            {
                ptr++;
                len--;
            }
            char *boundaryStartPtr = ptr;
            uint32 start_len = (uint32)len;
            while (!(*ptr == HTTP_CHAR_NULL || *ptr == HTTP_CHAR_SPACE || *ptr == HTTP_CHAR_TAB ||
                     *ptr == HTTP_CHAR_CR   || *ptr == HTTP_CHAR_LF) && len > 0)
            {
                ptr++;
                len--;
            }
            iContentType = HTTP_CONTENT_NULTIPART;
            return copyBoundaryString(boundaryStartPtr, start_len - (uint32)len);
        }
    }
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////
void HTTPContentInfoInternal::verifyTransferEncoding(const StrPtrLen &aTransferEncodingValue)
{
    char *ptr = (char *)aTransferEncodingValue.c_str();
    uint32 len = aTransferEncodingValue.length();

    // eat non-letter characters
    while (!isLetter(*ptr) && len > 0)
    {
        ptr++;
        len--;
    }
    OSCL_FastString chunkedString(_STRLIT_CHAR("chunked"));
    if (len < oscl_strlen(chunkedString.get_cstr())) return;

    if (((ptr[0] | OSCL_ASCII_CASE_MAGIC_BIT) == 'c') &&
            ((ptr[1] | OSCL_ASCII_CASE_MAGIC_BIT) == 'h') &&
            ((ptr[2] | OSCL_ASCII_CASE_MAGIC_BIT) == 'u') &&
            ((ptr[3] | OSCL_ASCII_CASE_MAGIC_BIT) == 'n') &&
            ((ptr[4] | OSCL_ASCII_CASE_MAGIC_BIT) == 'k') &&
            ((ptr[5] | OSCL_ASCII_CASE_MAGIC_BIT) == 'e') &&
            ((ptr[6] | OSCL_ASCII_CASE_MAGIC_BIT) == 'd'))
    {
        // find "chunked"
        iContentType = HTTP_CONTENT_CHUNKED_TRANSFER_ENCODING;
    }
}

bool HTTPContentInfoInternal::copyBoundaryString(const char* aBoundaryString, const uint32 aBoundaryStringLength)
{
    // allocate memory for boundary string
    if (!iBoundaryBuffer) iBoundaryBuffer = new char[aBoundaryStringLength+1];
    if (!iBoundaryBuffer) return false;
    if (!aBoundaryString) return false;

    oscl_memcpy(iBoundaryBuffer, aBoundaryString, aBoundaryStringLength);
    iBoundaryBuffer[aBoundaryStringLength] = HTTP_CHAR_NULL;
    iBoundary.setPtrLen(iBoundaryBuffer, aBoundaryStringLength);
    return true;
}

////////////////////////////////////////////////////////////////////////////////////
bool HTTPContentInfoInternal::parseBoudaryLine(HTTPMemoryFragment &aInputLineData, bool &isFinalBoundary)
{
    isFinalBoundary = false;

    // check boundary line : --BOUNDARY STRING
    char *ptr = (char *)aInputLineData.getPtr();
    int32 len = aInputLineData.getAvailableSpace();
    while (*ptr != HTTP_CHAR_MINUS && len > 0)
    {
        ptr++;
        len--;
    }
    if (len <= 0) return false; // this line is not boundary line
    if (*(++ptr) != HTTP_CHAR_MINUS) return false; // not "--"
    ptr++;
    len -= 2;
    if (len < iBoundary.length()) return false;
    char *boundaryString = (char *)iBoundary.c_str();
    int32 i = 0;
    for (i = 0; i < iBoundary.length(); i++)
    {
        if (ptr[i] != boundaryString[i]) return false;
    }

    // check the last "--" as the flag of final boundary string
    ptr += iBoundary.length();
    len -= iBoundary.length();
    if (len >= 2)
    {
        if (*ptr == HTTP_CHAR_MINUS && *(ptr + 1) == HTTP_CHAR_MINUS)
        {
            isFinalBoundary = true;
        }
    }
    return true;
}


/////////////////////////////////////////////////////////////////////////////////////
////////// HTTPParserInput Implementation ///////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
HTTPParserInput* HTTPParserInput::create()
{
    HTTPParserInput *parserInput = OSCL_NEW(HTTPParserInput, ());
    if (!parserInput) return NULL;
    if (!parserInput->construct())
    {
        OSCL_DELETE(parserInput);
        return NULL;
    }
    return parserInput;
}

bool HTTPParserInput::construct()
{
    // create iLineBuffer
    OsclMemAllocator alloc;
    iLineBuffer = (char *)alloc.allocate(iLineBufferSize);
    if (!iLineBuffer) return false;

    int32 err = 0;
    OSCL_TRY(err,
             iDataInQueue.reserve(DATA_QUEUE_VECTOR_RESERVE_SIZE);
             iDataOutQueue.reserve(DATA_QUEUE_VECTOR_RESERVE_SIZE);
            );
    return (err == 0);

}

HTTPParserInput::~HTTPParserInput()
{
    clear();
    iDataInQueue.clear();
    iDataOutQueue.clear();
    if (iLineBuffer)
    {
        OsclMemAllocator alloc;
        alloc.deallocate(iLineBuffer);
        iLineBuffer = NULL;
    }
}

bool HTTPParserInput::push_back(OsclRefCounterMemFrag &aFrag)
{
    if (!aFrag.getMemFragPtr() || !aFrag.getRefCounter())  // empty fragment
    {
        return (!iDataInQueue.empty()); // true for iDataInQueue not being empty
    }

    // check if this input is same to the previous one
    if (!iDataInQueue.empty())
    {
        if ((uint8*)aFrag.getMemFragPtr() == (uint8*)iDataInQueue[iDataInQueue.size()-1].getMemFragPtr())
        {
            return true; // true for iDataInQueue not being empty
        }
    }

    // push into the data queue
    int32 err = 0;
    OSCL_TRY(err, iDataInQueue.push_back(aFrag););
    if (err) return false;

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////
bool HTTPParserInput::getNextCompleteLine(HTTPMemoryFragment &aHttpFrag, bool aHeaderParsed)
{
    int32 offset = isNextLineAvailable(aHeaderParsed);
    if (offset <= 0) return false;
    aHttpFrag.bind(iHTTPMemFrag.getPtr(), offset);
    return true;
}

// aRequestDataSize==0 means no request size, the function needs to send out whatever amount of data it has,
// but with one input data fragment each time.
// return value: actual size, if aRequestDataSize > 0, actual size <= aRequestDataSize
// actual size = 0, => no data, -1 means error
int32 HTTPParserInput::getData(HTTPMemoryFragment &aHttpFrag, const uint32 aRequestDataSize)
{
    if (iDataInQueue.empty()) return 0;

    uint32 requestSize = (aRequestDataSize > 0 ? aRequestDataSize : 0xffffffff);
    uint32 availableFragSize = iDataInQueue[0].getMemFragSize() - iDataInQueueMemFragOffset;
    uint32 actualSize = OSCL_MIN(requestSize, availableFragSize);

    if (actualSize > 0)
    {
        // create the output fragments
        uint8* fragStartPtr = (uint8*)iDataInQueue[0].getMemFragPtr() + iDataInQueueMemFragOffset;
        aHttpFrag.bind((void *)fragStartPtr, actualSize);
        if (!constructOutputFragment(actualSize, (void *)fragStartPtr, (iDataInQueueMemFragOffset == 0))) return -1;
    }

    // check if iDataInQueue[0] needs to be removed and update iDataInQueueMemFragOffset
    if (availableFragSize <= requestSize)
    {
        iDataInQueue.erase(iDataInQueue.begin());
        iDataInQueueMemFragOffset = 0;
    }
    else
    {
        // updata iDataInQueueMemFragOffset
        iDataInQueueMemFragOffset += actualSize;
    }

    return (int32)actualSize;
}

// This function is for parsing multipart content, specically for the final boundary string like --boundaryString--, which could
// has no "\r\n", so getNextCompleteLine may not work in this case
// In general, if iLineBuffer has data, then send out iLineBuffer, then check if input data queue has data, if it has, then send
// out the first buffer. Return false for no any data (both iLineBuffer and data queue are empty)
// Note that this function doesn't do "get" that means changing internal pointers, instead, only does "view"
bool HTTPParserInput::viewAvailableInputData(HTTPMemoryFragment &aHttpFrag)
{
    if (iLineBufferOccupied == 0 && iDataInQueue.empty()) return false;

    if (iLineBufferOccupied)
    {
        aHttpFrag.bind(iLineBuffer, iLineBufferOccupied);
    }
    else   // iDataInQueue is not empty
    {
        int32 availableFragSize = iDataInQueue[0].getMemFragSize() - iDataInQueueMemFragOffset;
        if (availableFragSize == 0) return false;
        uint8* fragStartPtr = (uint8*)iDataInQueue[0].getMemFragPtr() + iDataInQueueMemFragOffset;
        aHttpFrag.bind(fragStartPtr, availableFragSize);
    }
    return true;
}


/////////////////////////////////////////////////////////////////////////////////////
bool HTTPParserInput::getOutputMemFrag(OsclRefCounterMemFrag &aMemFrag)
{
    if (iDataOutQueue.empty()) return false;
    iDataOutQueue[0].getRefCountMemFrag(aMemFrag);
    iDataOutQueue.erase(iDataOutQueue.begin());
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////
// pass ending CRLF
void HTTPParserInput::skipCRLF()
{
    if (iDataInQueue.empty()) return;
    uint8* fragStartPtr = (uint8*)iDataInQueue[0].getMemFragPtr() + iDataInQueueMemFragOffset;
    int32 availableFragSize = iDataInQueue[0].getMemFragSize() - iDataInQueueMemFragOffset;
    while ((*fragStartPtr == HTTP_CHAR_CR || *fragStartPtr == HTTP_CHAR_LF) && availableFragSize > 0)
    {
        *fragStartPtr++;
        availableFragSize--;
        iDataInQueueMemFragOffset++;
    }
}

// return value: 0 => not available ; >0 means the offset of the next complete line from the current point
// -1 error
int32 HTTPParserInput::isNextLineAvailable(bool aHeaderParsed)
{
    if (iDataInQueue.empty()) return 0;

    while (iDataInQueue.size() > 0)
    {
        if (iDataInQueueMemFragOffset >= iDataInQueue[0].getMemFragSize())
        {
            // remove iDataInQueue[0], since it is copied to iDataOutQueue
            iDataInQueue.erase(iDataInQueue.begin());
            iDataInQueueMemFragOffset = 0;
            iHTTPMemFrag.clear();
            return 0;
        }

        bool bNewData = (iDataInQueueMemFragOffset == 0);
        iHTTPMemFrag.bind((void *)((uint8*)iDataInQueue[0].getMemFragPtr() + iDataInQueueMemFragOffset),
                          iDataInQueue[0].getMemFragSize() - iDataInQueueMemFragOffset);

        int32 offset = checkNextLine(iHTTPMemFrag);
        iDataInQueueMemFragOffset += offset;
        if (offset > 0 && iLineBufferOccupied == 0)
        {
            // construct an output fragment
            if (!constructOutputFragment(iDataInQueueMemFragOffset, NULL, bNewData)) return -1;
            return offset;
        }

        // copy to iLineBuffer to concatenate the line fragments
        uint32 remaining_bytes = (offset > 0 ? offset : iHTTPMemFrag.getAvailableSpace());
        HTTPMemoryFragment aFrag(iHTTPMemFrag.getPtr(), remaining_bytes);
        if (assemblyLineFragments(aFrag)) return -1;

        if (offset > 0)
        {
            iHTTPMemFrag.bind(iLineBuffer, iLineBufferOccupied);
            if (!constructOutputFragment(iDataInQueueMemFragOffset, NULL, bNewData)) return -1;
            iLineBufferOccupied = 0;
            return iHTTPMemFrag.getCapacity();
        }

        if (!aHeaderParsed)
        {
            // save iDataInQueue[0]
            if (!bNewData && !iDataOutQueue.empty()) iDataOutQueue.erase(&iDataOutQueue.back());
            int32 err = 0;
            OSCL_TRY(err, iDataOutQueue.push_back(iDataInQueue[0]););
            if (err) return -1;
        }
        // remove iDataInQueue[0]
        iDataInQueue.erase(iDataInQueue.begin());
        iDataInQueueMemFragOffset = 0;
        iHTTPMemFrag.clear();
    }

    return 0;
}

int32 HTTPParserInput::assemblyLineFragments(HTTPMemoryFragment &aFrag)
{
    if (aFrag.getCapacity() <= iLineBufferSize - iLineBufferOccupied)
    {
        oscl_memcpy(iLineBuffer + iLineBufferOccupied, (char*)aFrag.getPtr(), aFrag.getCapacity());
    }
    else
    {
        // realloc iLineBuffer
        // aFrag.getCapacity()+iLineBufferOccupied>iLineBufferSize
        iLineBufferSize = (aFrag.getCapacity() + iLineBufferOccupied) << 1;

        OsclMemAllocator alloc;
        char *aNewLineBuffer = (char*)alloc.allocate(iLineBufferSize);
        if (!aNewLineBuffer) return -1;
        if (iLineBufferOccupied) oscl_memcpy(aNewLineBuffer, iLineBuffer, iLineBufferOccupied);
        oscl_memcpy(aNewLineBuffer + iLineBufferOccupied, (char*)aFrag.getPtr(), aFrag.getCapacity());

        // deallocate iLineBuffer
        alloc.deallocate(iLineBuffer);
        iLineBuffer = aNewLineBuffer;
    }
    iLineBufferOccupied += aFrag.getCapacity();
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////
int32 HTTPParserInput::checkNextLine(HTTPMemoryFragment &aInputDataStream)
{
    char *ptr = (char *)aInputDataStream.getPtr(), *start_ptr = ptr;
    int32 streamLength = aInputDataStream.getAvailableSpace();
    while (streamLength > 1 && (*ptr != HTTP_CHAR_CR && *ptr != HTTP_CHAR_LF))
    {
        ptr++;
        streamLength--;
    }

    if (*ptr == HTTP_CHAR_CR || *ptr == HTTP_CHAR_LF)
    {
        if (streamLength > 1 &&
                (ptr[1] == HTTP_CHAR_CR || ptr[1] == HTTP_CHAR_LF) &&
                ptr[1] != ptr[0]) ptr++;	// avoid double CR or double LF, should treat it as different lines
        // Note that double CR(CRLFCRLF) or double LF (CRLFCRLF, or LFLF) means end of HTTP header
        return ptr -start_ptr + 1;
    }

    return 0; // no complete key-value pair available
}

/////////////////////////////////////////////////////////////////////////////////////
// if aNewMemFragPtr=NULL, no change to memory fragment pointer, existing fragment with larger length
bool HTTPParserInput::constructOutputFragment(const uint32 aNewMemFragLen, const void *aNewMemFragPtr, const bool isNewFrag)
{
    if (isNewFrag || iDataOutQueue.empty())
    {
        RefCounterMemoryFragment refCountMemfrag(iDataInQueue[0].getMemFrag(), iDataInQueue[0].getRefCounter());
        int32 err = 0;
        OSCL_TRY(err, iDataOutQueue.push_back(refCountMemfrag););
        if (err) return false;
    }

    // update ptr and len of the memory fragment
    iDataOutQueue.back().update(aNewMemFragLen);
    if (aNewMemFragPtr) iDataOutQueue.back().update((void*)aNewMemFragPtr);
    return true;
}

bool HTTPParserInput::empty()
{
    if (iDataInQueue.empty()) return true;
    if (iDataInQueue.size() > 1) return false;

    // iDataInQueue.size() = 1
    if (iDataInQueueMemFragOffset == iDataInQueue[0].getMemFragSize()) return true;
    return false;
}

/////////////////////////////////////////////////////////////////////////////////////
////////// HTTPParserBaseObject Implementation //////////////////////*/////////////
/////////////////////////////////////////////////////////////////////////////////////
int32 HTTPParserBaseObject::parseHeaderFields(HTTPMemoryFragment &aInputLineData, const bool aReplaceOldValue)
{
    // parse header fields
    char *fieldKey;
    uint32 fieldKeyLength;
    char *fieldValue;
    uint32 fieldValueLength = 0;
    int32 status = getNextFieldKeyValuePair(aInputLineData, fieldKey, fieldKeyLength, fieldValue, fieldValueLength);
    if (status == 1) return HTTPParser::PARSE_HEADER_AVAILABLE; // end of header
    if (status < 0)
    {
        LOGINFO((0, "HTTPParserBaseObject::parseHeaderFields() : Syntax Error founded!!"));
        return HTTPParser::PARSE_SYNTAX_ERROR;	   // no divider characters found!
    }

    // exception handling (no key or no value case)
    if (fieldKeyLength == 0) return HTTPParser::PARSE_SUCCESS; // for no key, just ignore it
    char spaceChar = HTTP_CHAR_SPACE;
    if (fieldValueLength == 0)  // for no value, just set ''
    {
        fieldValue = &spaceChar;
        fieldValueLength = 1;
    }
    if (status != 0) return HTTPParser::PARSE_SUCCESS; // just ignore

    // add a key-value pair(fieldKey, fieldValue) to store
    return addKeyValuePairToStore(fieldKey, fieldKeyLength, fieldValue, fieldValueLength, aReplaceOldValue);
}

int32 HTTPParserBaseObject::addKeyValuePairToStore(const char *aFieldKey, const uint32 aFieldKeyLength,
        const char *aFieldValue, const uint32 aFieldValueLength,
        const bool aNeedReplaceOldValue)
{
    if (aFieldKeyLength + aFieldValueLength < iKeyValueStore->getAvailableSize())
    {
        if (iKeyValueStore->addKeyValuePair(aFieldKey, aFieldKeyLength, aFieldValue, aFieldValueLength, aNeedReplaceOldValue) != 0)
        {
            return HTTPParser::PARSE_GENERAL_ERROR;
        }
    }
    else
    {
        // not enough memory
        if (!reallocKeyValueStore(aFieldKeyLength + aFieldValueLength)) return HTTPParser::PARSE_MEMORY_ALLOCATION_FAILURE;
        // re-add key-value pair to store
        if (iKeyValueStore->addKeyValuePair(aFieldKey, aFieldKeyLength, aFieldValue, aFieldValueLength, aNeedReplaceOldValue) ==
                StringKeyValueStore::StringKeyValueStore_Failure) return HTTPParser::PARSE_GENERAL_ERROR;
    }
    return HTTPParser::PARSE_SUCCESS;
}

int32 HTTPParserBaseObject::addKeyValuePairToStore(const StrCSumPtrLen &aNewKey,
        const StrPtrLen &aNewValue,
        const bool aNeedReplaceOldValue)
{
    return addKeyValuePairToStore((char*)aNewKey.c_str(), aNewKey.length(),
                                  (char*)aNewValue.c_str(), aNewValue.length(),
                                  aNeedReplaceOldValue);
}


bool HTTPParserBaseObject::reallocKeyValueStore(const uint32 aCurrKeyValueSize)
{
    // calculate the new KeyValueStore size
    uint32 miniSize = iKeyValueStore->getCurrentMemoryUsage() + aCurrKeyValueSize;
    uint32 aNewStoreSize = OSCL_MAX(miniSize, iKeyValueStore->getStoreSize()) << 1;

    // create a new store
    StringKeyValueStore *aKeyValueStore = StringKeyValueStore::create(aNewStoreSize);
    if (!aKeyValueStore) return false;
    if (!aKeyValueStore->copy(*iKeyValueStore))
    {
        OSCL_DELETE(aKeyValueStore);
        return false;
    }
    OSCL_DELETE(iKeyValueStore);
    iKeyValueStore = aKeyValueStore;
    return true;

}

bool HTTPParserBaseObject::constructEntityUnit(HTTPParserInput &aParserInput, RefCountHTTPEntityUnit &aEntityUnit)
{
    OsclRefCounterMemFrag entityUnit;
    int32 err = 0;
    OSCL_TRY(err, entityUnit = iEntityUnitAlloc->get(););
    if (err) return false;

    // add memory fragments into the entity unit
    HTTPEntityUnit* entityUnitFrag = OSCL_PLACEMENT_NEW(((uint8*) entityUnit.getMemFragPtr()), HTTPEntityUnit());
    OsclRefCounterMemFrag memfrag;
    while (aParserInput.getOutputMemFrag(memfrag))
    {
        if (!entityUnitFrag->addMemFrag(memfrag))
        {
            entityUnitFrag->~HTTPEntityUnit();
            return false;
        }
    }
    //if(entityUnitFrag->getNumFragments() == 0) { entityUnitFrag->~HTTPEntityUnit(); return false; } // no memory fragments
    aEntityUnit = RefCountHTTPEntityUnit(*entityUnitFrag, entityUnit.getRefCounter()); // no memory fragments is possible
    entityUnitFrag->~HTTPEntityUnit(); // destruct entityUnitFrag
    return true;
}

// return value: 0 normal, 1 end of header, 2 ignore, -1 error
int32 HTTPParserBaseObject::getNextFieldKeyValuePair(HTTPMemoryFragment &aInputDataStream, char *&aFieldKey, uint32 &aFieldKeyLength,
        char *&aFieldValue, uint32 &aFieldValueLength)
{
    // get key
    int32 status = parseNextValueItem(aInputDataStream, aFieldKey, aFieldKeyLength, true);
    if (status != 0) return status; // end of header or error

    // get value
    return parseNextValueItem(aInputDataStream, aFieldValue, aFieldValueLength, false);
}

// return value: 0 normal,
//				 1 end of header,
//				 2 ignore (for CRLF, to handle CRLF split into separate fragments)
//				-1 error
int32 HTTPParserBaseObject::parseNextValueItem(HTTPMemoryFragment &aInputDataStream, char *&valueItemPtr, uint32 &valueItemLength, const bool isKeyItem)
{
    char dividerChar0 = (isKeyItem ? HTTP_CHAR_COLON : HTTP_CHAR_CR);
    char dividerChar1 = (isKeyItem ? HTTP_CHAR_COLON : HTTP_CHAR_LF);

    char *ptr = (char *)aInputDataStream.getPtr();
    int32 len = aInputDataStream.getAvailableSpace();

    // eat all non-letter characters at the beginning
    int32 status = getLineStartPoint(ptr, len, isKeyItem);
    if (status == 2) return status; // ignore
    if (status == 1)
    {
        aInputDataStream.update(ptr);    // Final CRLF, end of HTTP header
        return 1;
    }

    // search divider characters to identify the value item
    valueItemPtr = ptr;
    while (*ptr != dividerChar0 && *ptr != dividerChar1 && len > 0)
    {
        ptr++;    // assuming there is no case like "zzz key :"
        len--;
    }
    if (len <= 0) return -1; // no divider chars

    char *end_ptr = ptr--;
    while (*ptr == HTTP_CHAR_SPACE || *ptr == HTTP_CHAR_TAB) ptr--; // eat space or tab characater
    valueItemLength = (ptr > valueItemPtr ? (ptr - valueItemPtr + 1) : 0); // ptr is the ending pointer for a value item

    ptr = end_ptr;
    if (isKeyItem) ptr++;
    else
    {
        saveEndingCRLF(ptr, len, iPrevCRLF);
        if (len > 0 && (ptr[1] == HTTP_CHAR_CR || ptr[1] == HTTP_CHAR_LF)) ptr++; // pass CRLF at each line
    }

    aInputDataStream.update(ptr);
    return 0;
}

// return value: 0 normal, 1 end of header, 2 ignore
int32 HTTPParserBaseObject::getLineStartPoint(char *&ptr, int32 &len, const bool isKeyItem)
{
    if (isKeyItem)
    {
        while (!isLetter(*ptr) && (*ptr != HTTP_CHAR_CR && *ptr != HTTP_CHAR_LF) && len > 0)
        {
            iPrevCRLF = 0;
            ptr++;
            len--; // eat all non-letter characters except CRLF
        }
        if (*ptr == HTTP_CHAR_CR || *ptr == HTTP_CHAR_LF)  // key guarantees to have very first LETTER character
        {
            if (iPrevCRLF == 0)
            {
                saveEndingCRLF(ptr, len, iPrevCRLF);
                if (len > 0 && (ptr[1] == HTTP_CHAR_CR || ptr[1] == HTTP_CHAR_LF)) ptr++;
                return 2; // save CRLF to iPrevCRLF, and ignore
            }
            else   // iPrevCRLF has something
            {
                uint8 currCRLF = 0;
                saveEndingCRLF(ptr, len, currCRLF);
                if (len > 0 && (ptr[1] == HTTP_CHAR_CR || ptr[1] == HTTP_CHAR_LF)) ptr++;
                if (iPrevCRLF & currCRLF) return 1; // double CR or LF, end of HTTP header
                iPrevCRLF = currCRLF;
                return 2; // ignore
            }
        }
    }
    else
    {
        while ((*ptr == HTTP_CHAR_SPACE || *ptr == HTTP_CHAR_TAB) && len > 0)
        {
            ptr++;
            len--;
        } // eat space or tab character
    }

    return 0;
}

// aNeedReset=1, set aCRLF; aNeedReset=0, update aCRLF
void HTTPParserBaseObject::saveEndingCRLF(char *ptr, uint32 len, uint8& aCRLF, bool aNeedReset)
{
    char *tmpPtr = ptr;
    int32 tmpLen = (int32)len;
    if (aNeedReset) aCRLF = 0;

    // get to CRLF point
    while ((*tmpPtr != HTTP_CHAR_CR && *tmpPtr != HTTP_CHAR_LF) && tmpLen > 0)
    {
        tmpPtr++;
        tmpLen--;
    }

    while ((*tmpPtr == HTTP_CHAR_CR || *tmpPtr == HTTP_CHAR_LF) && tmpLen > 0)
    {
        if (*tmpPtr == HTTP_CHAR_CR) aCRLF |= 0x2; // bit 1 = 1
        if (*tmpPtr == HTTP_CHAR_LF) aCRLF |= 0x1; // bit 0 = 1
        tmpPtr++;
        tmpLen--;
    }
}


/////////////////////////////////////////////////////////////////////////////////////
////////// HTTPParserHeaderObject Implementation ////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
int32 HTTPParserHeaderObject::parse(HTTPParserInput &aParserInput, RefCountHTTPEntityUnit &aEntityUnit)
{
    HTTPMemoryFragment aInputLineData;

    while (aParserInput.getNextCompleteLine(aInputLineData))
    {
        if (!iHeaderFirstLineParsed)
        {
            // parse the first line : Status-Line = HTTP-Version SP Status-Code SP Reason-Phrase CRLF
            int32 status = parseFirstLine(aInputLineData);
            if (status < 0) return status; // syntax error
            iHeaderFirstLineParsed = true;
        }
        else
        {
            int32 status = parseHeaderFields(aInputLineData);
            if (status == HTTPParser::PARSE_HEADER_AVAILABLE)
            {
                iHeaderParsed = true;
                // check content info
                if (!iContentInfo->parseContentInfo(*iKeyValueStore)) return HTTPParser::PARSE_MEMORY_ALLOCATION_FAILURE;
                // construct output entity unit
                if (!constructEntityUnit(aParserInput, aEntityUnit)) return HTTPParser::PARSE_MEMORY_ALLOCATION_FAILURE;
                if (!isGoodStatusCode())
                {
                    LOGINFO((0, "HTTPParserHeaderObject::parse() : NOT GOOD STATUS CODE"));
                    return HTTPParser::PARSE_STATUS_LINE_SHOW_NOT_SUCCESSFUL;
                }
                if (checkResponseParsedComplete()) iResponseParsedComplete = true;
                return HTTPParser::PARSE_HEADER_AVAILABLE;
            }
            if (status != HTTPParser::PARSE_SUCCESS) return status;
        }

    } // end of: while(iParserInput->getNextCompleteLine(aInputLineData))

    // check content info
    return HTTPParser::PARSE_NEED_MORE_DATA;
}

// return value: 0 => ok , or HTTPParser enum codes
int32 HTTPParserHeaderObject::parseFirstLine(HTTPMemoryFragment &aInputDataStream)
{
    // Status-Line = HTTP-Version SP Status-Code SP Reason-Phrase CRLF
    char *ptr = (char *)aInputDataStream.getPtr();
    int32 len = (int32)aInputDataStream.getAvailableSpace();

    while (!isLetter(*ptr) && len > 0)
    {
        ptr++;
        len--;
    }
    if (len < 8) return HTTPParser::PARSE_SYNTAX_ERROR;

    // check HTTP/1.x
    if (((ptr[0] | OSCL_ASCII_CASE_MAGIC_BIT) == 'h') &&
            ((ptr[1] | OSCL_ASCII_CASE_MAGIC_BIT) == 't') &&
            ((ptr[2] | OSCL_ASCII_CASE_MAGIC_BIT) == 't') &&
            ((ptr[3] | OSCL_ASCII_CASE_MAGIC_BIT) == 'p') &&
            (ptr[4]								 == '/'))
    {
        ptr += 5; // size of "http/"
        if (!checkHTTPVersion(ptr))
        {
            return HTTPParser::PARSE_HTTP_VERSION_NOT_SUPPORTED;
        }

        // ptr should be updated in checkHTTPVersion()
        while (!isDigit(*ptr) && len > 0)
        {
            ptr++;
            len--;
        }
        if (len <= 0) return 0; // no digital status code
        char *start_ptr = ptr;
        uint32 start_len = len;
        while (isDigit(*ptr) && len > 0)
        {
            ptr++;
            len--;
        }
        //if(len <= 0) return 1; // no Reason-Phrase

        // get status code
        PV_atoi(start_ptr, 'd', start_len - len, iStatusCode);
        saveEndingCRLF(ptr, len, iPrevCRLF);
        return 0;
    }

    // add first-line into the key-value store
    StrPtrLen firstLine = "Response-Line";
    addKeyValuePairToStore(firstLine.c_str(), firstLine.length(),
                           (char *)aInputDataStream.getPtr(), aInputDataStream.getAvailableSpace(),
                           true);

    return HTTPParser::PARSE_SYNTAX_ERROR; // looks like the status line has something we don't understand.
}

bool HTTPParserHeaderObject::checkHTTPVersion(char* &aPtr)
{
    if (aPtr[0] == '1' && aPtr[0] == HTTP_CHAR_SPACE)
    {
        // support HTTP/1
        aPtr += 2;
        iHttpVersionNum = 0;
        return true;
    }

    if (aPtr[0] == '1' &&
            aPtr[1] == '.' &&
            (aPtr[2] == '0' || aPtr[2] == '1'))
    {
        iHttpVersionNum = (aPtr[2] == '0' ? 0 : 1);
        aPtr += 3;
        return true;
    }

    return false;
}

// This function gets complicated since a couple of new cases have been added
bool HTTPParserHeaderObject::isGoodStatusCode()
{
    if (iStatusCode < GOOD_HTTP_STATUS_CODE_START_FROM100 ||
            iStatusCode > GOOD_HTTP_STATUS_CODE_END_AT299) return false;

    // check 1xx code, 1xx code is only allowed in Http/1.1
    bool goodStatusCode = checkGood1xxCode();
    if (!goodStatusCode) return false;

    // check 2xx code, if 204 (no content) or 2xx code with content-length=0, then we need to error out
    goodStatusCode = checkGood2xxCode();
    return goodStatusCode;
}

// check 1xx code, 1xx code is only allowed in Http/1.1
bool HTTPParserHeaderObject::checkGood1xxCode()
{
    if (iHttpVersionNum == 0 &&
            (GOOD_HTTP_STATUS_CODE_START_FROM100 <= iStatusCode && iStatusCode < GOOD_HTTP_STATUS_CODE_START_FROM200))
    {
        return false;
    }
    return true;
}

// check 2xx code, if 2xx code with content-length=0, then we need to error out
bool HTTPParserHeaderObject::checkGood2xxCode()
{
    uint32 goodStatusCodeStart = GOOD_HTTP_STATUS_CODE_START_FROM200;
    if (GOOD_HTTP_STATUS_CODE_START_FROM200 <= iStatusCode && iStatusCode <= GOOD_HTTP_STATUS_CODE_END_AT299)
    {
        // for Http status code 204 (no content), error our right away
        if (iStatusCode == HTTP_STATUS_CODE_204_NO_CONTENT)
        {
            LOGINFO((0, "HTTPParserHeaderObject::checkGood2xxCode() : iStatusCode=HTTP_STATUS_CODE_204_NO_CONTENT"));
            return false;
        }

        // other 2xx code, check the zero or empty content-length
        StrCSumPtrLen contenLengthKey("Content-Length");
        StrPtrLen contentLengthValue;
        if (iKeyValueStore->getValueByKey(contenLengthKey, contentLengthValue))
        {
            // has Content-Length field
            uint32 aContentLength = 0;
            PV_atoi(contentLengthValue.c_str(), 'd', aContentLength);

            // check the empty Content-Length case
            char *ptr = (char *)contentLengthValue.c_str();
            if (aContentLength == 0 || ptr[0] == HTTP_CHAR_SPACE)
            {
                LOGINFO((0, "HTTPParserHeaderObject::checkGood2xxCode() : zero or empty content length for 2xx code"));
                return false;
            }
        }
    }
    return (goodStatusCodeStart <= iStatusCode && iStatusCode <= GOOD_HTTP_STATUS_CODE_END_AT299);
}

int32 HTTPParserHeaderObject::doSanityCheckForResponseHeader()
{
    // check Chunked Transfer Encoding supported for Http/1.1 only
    if (!checkChunkedTransferEncodingSupported()) return HTTPParser::PARSE_TRANSFER_ENCODING_NOT_SUPPORTED;
    return HTTPParser::PARSE_SUCCESS;
}


// check Chunked Transfer Encoding supported by Http/1.1 only
bool HTTPParserHeaderObject::checkChunkedTransferEncodingSupported()
{
    StrCSumPtrLen transferEncodingKey("Transfer-Encoding");
    StrPtrLen transferEncodingValue;
    if (iKeyValueStore->getValueByKey(transferEncodingKey, transferEncodingValue))
    {
        LOGINFO((0, "HTTPParserHeaderObject::checkChunkedTransferEncodingSupported() : has Transfer-encoding field, HttpVersionNum=%d", iHttpVersionNum));
        // has Transfer-encoding field
        if (iHttpVersionNum == 0) return false;
    }
    return true;
}

bool HTTPParserHeaderObject::checkResponseParsedComplete()
{
    // check "Content-Length"
    StrCSumPtrLen contentLengthKey = "Content-Length";
    StrPtrLen contentLengthValue;

    if (!getField(contentLengthKey, contentLengthValue)) return false; // no "Content-Length"

    // get "Content-Length" value
    uint32 contentLength = 0;
    PV_atoi(contentLengthValue.c_str(), 'd', contentLength);
    return (contentLength == 0);
}

HTTPParserHeaderObject *HTTPParserHeaderObject::create(HTTPContentInfoInternal *aContentInfo)
{
    HTTPParserHeaderObject *header = OSCL_NEW(HTTPParserHeaderObject, ());
    if (!header) return NULL;
    if (!header->construct(aContentInfo))
    {
        OSCL_DELETE(header);
        return NULL;
    }
    return header;
}

bool HTTPParserHeaderObject::construct(HTTPContentInfoInternal *aContentInfo)
{
    reset();
    iContentInfo = aContentInfo;

    if ((iKeyValueStore = StringKeyValueStore::create()) == NULL) return false;

    iEntityUnitAlloc = OSCL_NEW(PVMFBufferPoolAllocator, ());
    if (!iEntityUnitAlloc) return false;

    int32 err = 0;
    OSCL_TRY(err, iEntityUnitAlloc->size(HTTP_ENTITY_UNIT_POOLNUM, sizeof(HTTPEntityUnit)));
    if (err) return false;

    return true;
}

HTTPParserHeaderObject::~HTTPParserHeaderObject()
{
    reset();

    // delete iKeyValueStore
    if (iKeyValueStore) OSCL_DELETE(iKeyValueStore);
    iKeyValueStore = NULL;

    // delete iEntityUnitAlloc
    if (iEntityUnitAlloc) OSCL_DELETE(iEntityUnitAlloc);
    iEntityUnitAlloc = NULL;
}


/////////////////////////////////////////////////////////////////////////////////////
////////// HTTPParserEntityBodyObject Implementation ////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
int32 HTTPParserEntityBodyObject::parseEnityBodyChunkData(HTTPParserInput &aParserInput, RefCountHTTPEntityUnit &aEntityUnit)
{
    int32 requestSize = iContentInfo->getContentRangeLength() - iCurrentChunkDataLength;
    HTTPMemoryFragment aFrag;
    int32 actualSize = 0;
    while (requestSize > 0)
    {
        if ((actualSize = aParserInput.getData(aFrag, requestSize)) <= 0) break;
        iCurrentChunkDataLength += actualSize;
        iNumChunks++;
        requestSize -= actualSize;
    }
    if (actualSize < 0) return HTTPParser::PARSE_MEMORY_ALLOCATION_FAILURE;
    if (actualSize == 0 && requestSize > 0) return HTTPParser::PARSE_NEED_MORE_DATA;

    // get complete chunk, and then construct output entity unit
    if (!constructEntityUnit(aParserInput, aEntityUnit)) return HTTPParser::PARSE_MEMORY_ALLOCATION_FAILURE;

    // pass ending CRLF for the chunk data for next chunk parsing
    aParserInput.skipCRLF();
    return HTTPParser::PARSE_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////
////////// HTTPParserNormalContentObject Implementation /////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
int32 HTTPParserNormalContentObject::parse(HTTPParserInput &aParserInput, RefCountHTTPEntityUnit &aEntityUnit)
{
    HTTPMemoryFragment aFrag;
    int32 actualSize = 0;
    if (iContentInfo->iContentLength == 0) iContentInfo->iContentLength = 0x7fffffff; // 0=>7fff ffff
    if (iCurrTotalLengthObtained == 0 && iContentInfo->iContentRangeLeft > 0) iCurrTotalLengthObtained = iContentInfo->iContentRangeLeft;
    int32 requestSize = (int32)iContentInfo->iContentLength - (int32)iCurrTotalLengthObtained;
    if (requestSize <= 0)
    {
        if (requestSize == 0) return HTTPParser::PARSE_SUCCESS_END_OF_MESSAGE;
        return HTTPParser::PARSE_SUCCESS_END_OF_MESSAGE_WITH_EXTRA_DATA;
    }
    while ((actualSize = aParserInput.getData(aFrag, requestSize)) > 0)
    {
        iCurrTotalLengthObtained += actualSize;
        if (requestSize > 0)
        {
            if ((requestSize -= actualSize) <= 0) break; // we don't need to process aFrag
        }
    }
    if (actualSize < 0) return HTTPParser::PARSE_MEMORY_ALLOCATION_FAILURE;

    // construct output entity unit
    if (!constructEntityUnit(aParserInput, aEntityUnit)) return HTTPParser::PARSE_MEMORY_ALLOCATION_FAILURE;
    if (iCurrTotalLengthObtained >= iContentInfo->iContentLength)
    {
        if (iCurrTotalLengthObtained > iContentInfo->iContentLength ||
                !aParserInput.empty()) return HTTPParser::PARSE_SUCCESS_END_OF_MESSAGE_WITH_EXTRA_DATA;
        return HTTPParser::PARSE_SUCCESS_END_OF_MESSAGE;
    }
    if (actualSize == 0 && iContentInfo->iContentLength > iCurrTotalLengthObtained) return HTTPParser::PARSE_SUCCESS_END_OF_INPUT;
    return HTTPParser::PARSE_SUCCESS;
}


/////////////////////////////////////////////////////////////////////////////////////
////////// HTTPParserCTEContentObject Implementation /////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
int32 HTTPParserCTEContentObject::parse(HTTPParserInput &aParserInput, RefCountHTTPEntityUnit &aEntityUnit)
{
    // get CTE chunk length
    if (iContentInfo->iContentRangeRight == 0)
    {
        HTTPMemoryFragment aInputLineData;
        int32 chunkLength = -1;
        while (aParserInput.getNextCompleteLine(aInputLineData, true))  // true means header is already parsed
        {
            if (getCTEChunkLength(aInputLineData, chunkLength)) break;
        }
        if (chunkLength == -1) return HTTPParser::PARSE_NEED_MORE_DATA;
        if (chunkLength == 0)
        {
            if (!aParserInput.empty()) return HTTPParser::PARSE_SUCCESS_END_OF_MESSAGE_WITH_EXTRA_DATA;
            return HTTPParser::PARSE_SUCCESS_END_OF_MESSAGE;
        }

        iContentInfo->iContentRangeRight = chunkLength - 1;
        iContentInfo->iContentLength += chunkLength;
        aParserInput.clearOutputQueue();
    }

    // get CTE chunk data
    aParserInput.skipCRLF();
    int32 status = parseEnityBodyChunkData(aParserInput, aEntityUnit);
    if (status == HTTPParser::PARSE_SUCCESS) reset(); // for next chunk parsing
    return status;
}

bool HTTPParserCTEContentObject::getCTEChunkLength(HTTPMemoryFragment &aInputLineData, int32 &aChunkSize)
{
    char *ptr = (char *)aInputLineData.getPtr();
    int32 len = (int32)aInputLineData.getAvailableSpace();
    while (!isHexDigit(*ptr) && len > 0)
    {
        ptr++;
        len--;
    }
    if (len <= 0) return false;
    char *start_ptr = ptr;
    int32 start_len = len;
    while (isHexDigit(*ptr) && len > 0)
    {
        ptr++;
        len--;
    }
    if (len <= 0) return false;
    uint32 chunkSize;
    PV_atoi(start_ptr, 'x', start_len - len, chunkSize);
    aChunkSize = (int32)chunkSize;
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////
////////// HTTPParserMultipartContentObject Implementation /////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
int32 HTTPParserMultipartContentObject::parse(HTTPParserInput &aParserInput, RefCountHTTPEntityUnit &aEntityUnit)
{
    // parse boundary string and chunk header
    int32 status = parseChunkHeader(aParserInput);
    if (status != HTTPParser::PARSE_SUCCESS) return status;

    // get chunk data

    status = parseEnityBodyChunkData(aParserInput, aEntityUnit);
    if (status != HTTPParser::PARSE_SUCCESS) return status;

    reset(); // for next chunk parsing
    if (aParserInput.empty()) return HTTPParser::PARSE_SUCCESS_END_OF_INPUT;
    return HTTPParser::PARSE_SUCCESS;
}

int32 HTTPParserMultipartContentObject::parseChunkHeader(HTTPParserInput &aParserInput)
{
    if (iHeaderInEntityBodyParsed) return HTTPParser::PARSE_SUCCESS;

    // parse boundary line
    if (!iBoudaryLineParsed)
    {
        int32 status = parseChunkBoundaryLine(aParserInput);
        if (status != HTTPParser::PARSE_SUCCESS) return status;
        if (!iBoudaryLineParsed) return HTTPParser::PARSE_NEED_MORE_DATA; // try next time
    }

    // parse chunk header
    HTTPMemoryFragment aInputLineData;
    while (aParserInput.getNextCompleteLine(aInputLineData))
    {
        if (!iBoudaryLineParsed)
        {
            return HTTPParser::PARSE_SYNTAX_ERROR;
        }
        int32 status = parseHeaderFields(aInputLineData, true); // true means replace the old field value with the new one
        if (status == HTTPParser::PARSE_HEADER_AVAILABLE)
        {
            iHeaderInEntityBodyParsed = true;
            iCounter++;
            // update content info
            if (!iContentInfo->parseContentInfo(*iKeyValueStore)) return HTTPParser::PARSE_MEMORY_ALLOCATION_FAILURE;
            aParserInput.clearOutputQueue();
            saveEndingCRLF((char *)aInputLineData.getPtr(), (int32)aInputLineData.getAvailableSpace(), iPrevCRLF);
            break;
        }
        if (status != HTTPParser::PARSE_SUCCESS) return status;
    }
    if (!iHeaderInEntityBodyParsed) return HTTPParser::PARSE_NEED_MORE_DATA;

    // check the extra CRLF
    if (needSkipCRLF()) aParserInput.skipCRLF();
    return HTTPParser::PARSE_SUCCESS;
}


int32 HTTPParserMultipartContentObject::parseChunkBoundaryLine(HTTPParserInput &aParserInput)
{
    HTTPMemoryFragment aInputLineData;
    if (aParserInput.getNextCompleteLine(aInputLineData))
    {

        // parse boundary line : --BOUNDARY STRING
        bool isFinalBoundary = false;
        if (iContentInfo->parseBoudaryLine(aInputLineData, isFinalBoundary)) iBoudaryLineParsed = true;
        if (isFinalBoundary) return HTTPParser::PARSE_SUCCESS_END_OF_MESSAGE;
        saveEndingCRLF((char *)aInputLineData.getPtr(), (int32)aInputLineData.getAvailableSpace(), iPrevCRLF);

        if (!iBoudaryLineParsed)
        {
            if (aParserInput.getNextCompleteLine(aInputLineData))
            {
                bool isFinalBoundary = false;
                if (iContentInfo->parseBoudaryLine(aInputLineData, isFinalBoundary)) iBoudaryLineParsed = true;
            }
        }
    }

    if (!iBoudaryLineParsed)
    {
        // try to see whether it is final boundary line, like --boundaryString-- (no "\r\n")
        HTTPMemoryFragment frag;
        if (!aParserInput.viewAvailableInputData(frag)) return HTTPParser::PARSE_NEED_MORE_DATA;

        bool isFinalBoundary = false;
        iContentInfo->parseBoudaryLine(frag, isFinalBoundary);
        if (isFinalBoundary) return HTTPParser::PARSE_SUCCESS_END_OF_MESSAGE;
    }
    return HTTPParser::PARSE_SUCCESS;
}
