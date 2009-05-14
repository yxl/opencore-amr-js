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
#ifndef HTTP_PARSER_INTERNAL_H_
#define HTTP_PARSER_INTERNAL_H_

#include "oscl_refcounter_memfrag.h"
#include "oscl_vector.h"
#include "http_parcom_internal.h"
#include "http_parser_external.h"

#ifndef PVMF_POOL_BUFFER_ALLOCATOR_H_INCLUDED
#include "pvmf_pool_buffer_allocator.h"
#endif

#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif

#ifndef STRING_KEYVALUE_STORE_H_INCLUDED
#include "string_keyvalue_store.h"
#endif

#define HTTP_ENTITY_UNIT_POOLNUM  4
#define DEFAULT_MAX_LINE_BUFFER_SIZE 512
#define DATA_QUEUE_VECTOR_RESERVE_SIZE 4
#define GOOD_HTTP_STATUS_CODE_START_FROM100	100
#define GOOD_HTTP_STATUS_CODE_START_FROM200	200
#define GOOD_HTTP_STATUS_CODE_END_AT299		299
#define HTTP_STATUS_CODE_204_NO_CONTENT		204


enum HTTPContentType
{
    HTTP_CONTENT_NORMAL = 0,					// no chunk header info
    HTTP_CONTENT_NULTIPART,					// for Content-Type : multipart/byteranges
    HTTP_CONTENT_CHUNKED_TRANSFER_ENCODING	// for Transfer-Encoding : chunked
};

#define LOGINFO(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG,iLogger,PVLOGMSG_INFO,m);



///////////////////////////////////////////////////////////////////////////////////////
// HTTPContentInfoInternal object encapsulates content type/length/range infomation parsing happening
// on header parsing and entity body parsing
struct HTTPContentInfoInternal
{
    uint32 iContentLength;		// for "Content-Length"
    uint32 iContentRangeLeft;	// for "Content-Range"
    uint32 iContentRangeRight;

    // constructor
    HTTPContentInfoInternal() : iBoundaryBuffer(NULL)
    {
        clear();
    }

    // ~destructor
    ~HTTPContentInfoInternal()
    {
        clear();
        if (iBoundaryBuffer) OSCL_ARRAY_DELETE(iBoundaryBuffer);
        iBoundaryBuffer = NULL;
    }

    // clear
    void clear()
    {
        iContentLength     = 0;
        iContentRangeLeft  = 0;
        iContentRangeRight = 0;

        iContentType = HTTP_CONTENT_NORMAL;
        iBoundary.setPtrLen("", 0);
    }

    void get(HTTPContentInfo &aContentInfo)
    {
        aContentInfo.iContentLength = iContentLength;
        aContentInfo.iContentRangeLeft = iContentRangeLeft;
        aContentInfo.iContentRangeRight = iContentRangeRight;
    }

    // operator "="
    HTTPContentInfoInternal &operator=(const HTTPContentInfoInternal& x)
    {
        iContentLength	   = x.iContentLength;
        iContentRangeLeft  = x.iContentRangeLeft;
        iContentRangeRight = x.iContentRangeRight;
        iContentType       = x.iContentType;
        return *this;
    }

    bool parseContentInfo(StringKeyValueStore &aKeyValueStore);
    uint32 getContentType() const
    {
        return (uint32)iContentType;
    }
    // range lengh = range right - range left
    uint32 getContentRangeLength() const
    {
        return (iContentRangeRight == 0 ? 0 : iContentRangeRight - iContentRangeLeft + 1);
    }

    bool parseBoudaryLine(HTTPMemoryFragment &aInputLineData, bool &isFinalBoundary);

private:
    void parseContentRange(const StrPtrLen &aContentRange);
    bool parseContentType(const StrPtrLen &aContentType);
    void verifyTransferEncoding(const StrPtrLen &aTransferEncodingValue);
    bool copyBoundaryString(const char* aBoundaryString, const uint32 aBoundaryStringLength);

private:
    HTTPContentType iContentType;
    char *iBoundaryBuffer;
    StrPtrLen iBoundary;	// for "Content-Type : multipart/byteranges"
};


///////////////////////////////////////////////////////////////////////////////////////
// HTTPParserInput object hides fragment concatenation and grouping for header parsing and output generation
class HTTPParserInput
{

    public:
        // factory method
        static HTTPParserInput *create();

        // destructor
        ~HTTPParserInput();

        // clear
        void clear()
        {
            iDataInQueue.clear();
            iDataOutQueue.clear();
            iHTTPMemFrag.clear();
            iDataInQueueMemFragOffset = 0;
            iLineBufferOccupied = 0;
        }

        void clearOutputQueue()
        {
            iDataOutQueue.clear();
        }

        // add data
        bool push_back(OsclRefCounterMemFrag &aFrag);
        bool getNextCompleteLine(HTTPMemoryFragment &aHttpFrag, bool aHeaderParsed = false);

        // aRequestDataSize==0 means no request size, the function needs to send out whatever amount of data it has,
        // but with one input data fragment each time.
        // return value: actual size, if aRequestDataSize > 0, actual size <= aRequestDataSize
        // actual size = 0, => no data, -1 means error
        int32 getData(HTTPMemoryFragment &aHttpFrag, const uint32 aRequestDataSize = 0);
        bool getOutputMemFrag(OsclRefCounterMemFrag &aMemFrag);

        // This function is for parsing multipart content, specically for the final boundary string like --boundaryString--, which could
        // has no "\r\n", so getNextCompleteLine may not work in this case
        // In general, if iLineBuffer has data, then send out iLineBuffer, then check if input data queue has data, if it has, then send
        // out the first buffer. Return false for no any data (both iLineBuffer and data queue are empty)
        // Note that this function doesn't do "get" (i.e. not changing internal pointers), instead, only does "view"
        bool viewAvailableInputData(HTTPMemoryFragment &aHttpFrag);
        bool empty();

        // pass ending CRLF
        void skipCRLF();

    private:

        // constructor
        HTTPParserInput() : iLineBufferSize(DEFAULT_MAX_LINE_BUFFER_SIZE)
        {
            clear();
        }

        // create iLineBuffer
        bool construct();

        // return value: 0 => not available ; >0 means the offset of the next complete line from the current point
        // -1 error
        int32 isNextLineAvailable(bool aHeaderParsed);
        // called by isNextLineAvailable()
        int32 assemblyLineFragments(HTTPMemoryFragment &aFrag);

        // if aNewMemFragPtr=NULL, no change to memory fragment pointer. isNewFrag=false means update the existing fragment
        // from iDataOutQueue; if isNewFrag=true, get fragment from iDataInQueue
        bool constructOutputFragment(const uint32 aNewMemFragLen, const void *aNewMemFragPtr = NULL, bool isNewFrag = false);
        int32 checkNextLine(HTTPMemoryFragment &aInputDataStream);

    private:
        Oscl_Vector<OsclRefCounterMemFrag, OsclMemAllocator> iDataInQueue;
        Oscl_Vector<RefCounterMemoryFragment, OsclMemAllocator> iDataOutQueue; // RefCounterMemoryFragment defined in http_parcom_internal.h
        uint32 iDataInQueueMemFragOffset;
        HTTPMemoryFragment iHTTPMemFrag; // hold the input data fragment being processed
        char *iLineBuffer;				 // concatenate multiple fragments of each header line
        uint32 iLineBufferSize;
        uint32 iLineBufferOccupied;
};

///////////////////////////////////////////////////////////////////////////////////////
// HTTPParserBaseObject is the base class for header class and entity body class that still needs some kind of
// header parsing inside the entity body

#define LOGGER_TAG "datapath.sourcenode.protocolenginenode" // "protocolenginenode.protocolengine"
//#define LOGGER_TAG "protocolenginenode.protocolengine"


class HTTPParserBaseObject
{
    public:
        int32 parseHeaderFields(HTTPMemoryFragment &aInputLineData, const bool aReplaceOldValue = false);
        bool constructEntityUnit(HTTPParserInput &aParserInput, RefCountHTTPEntityUnit &aEntityUnit);
        void saveEndingCRLF(char *ptr, uint32 len, uint8& aCRLF, bool aNeedReset = true);

        // constructor
        HTTPParserBaseObject(StringKeyValueStore *aKeyValueStore = NULL, PVMFBufferPoolAllocator *aEntityUnitAlloc = NULL) :
                iKeyValueStore(aKeyValueStore), iEntityUnitAlloc(aEntityUnitAlloc), iPrevCRLF(0)
        {
            iLogger = PVLogger::GetLoggerObject(LOGGER_TAG);
        }

    protected:
        // due to the fact that Key-Value store may increase size in the fly, create this method to
        // to wrap up all the details
        // return code comes from HTTPParser return code
        int32 addKeyValuePairToStore(const char *aFieldKey, const uint32 aFieldKeyLength,
                                     const char *aFieldValue, const uint32 aFieldValueLength,
                                     const bool aNeedReplaceOldValue = false);

        int32 addKeyValuePairToStore(const StrCSumPtrLen &aNewKey,
                                     const StrPtrLen &aNewValue,
                                     const bool aNeedReplaceOldValue = false);

    private:
        // return value: 0 normal, 1 end of header
        int32 getNextFieldKeyValuePair(HTTPMemoryFragment &aInputDataStream, char *&aFieldKey, uint32 &aFieldKeyLength,
                                       char *&aFieldValue, uint32 &aFieldValueLength);
        // return value: 0 normal, 1 end of header, 2 ignore
        int32 getLineStartPoint(char *&ptr, int32 &len, const bool isKeyItem);
        // return value: 0 normal, 1 end of header
        int32 parseNextValueItem(HTTPMemoryFragment &aInputDataStream, char *&valueItemPtr, uint32 &valueItemLength, const bool isKeyItem);

        // called by addKeyValuePairToStore, increase the size of the current store
        // due to not enough memory to hold new key-value pair
        bool reallocKeyValueStore(const uint32 aCurrKeyValueSize);

    protected:
        StringKeyValueStore *iKeyValueStore;
        PVMFBufferPoolAllocator *iEntityUnitAlloc;
        uint8 iPrevCRLF; // bit0 -- LF , bit1 -- CR
        PVLogger *iLogger;
};

///////////////////////////////////////////////////////////////////////////////////////
// HTTPParserHeaderObject encapsulates HTTP header parsing, basically parses every field inside the header and save it to
// key-value store, and also triggers content info parsing
class HTTPParserHeaderObject : public HTTPParserBaseObject
{
    public:
        int32 parse(HTTPParserInput &aParserInput, RefCountHTTPEntityUnit &aEntityUnit);
        bool isParsed() const
        {
            return iHeaderParsed;
        }
        // query this case: parsing header completely means parsing the whole response completely
        // in case of no http body with content-length setting to zero
        bool isWholeResponseParsed() const
        {
            return iResponseParsedComplete;
        }
        // check if there are some unsupported headers
        int32 doSanityCheckForResponseHeader();

        // The following "get" functions are for "get" functions of parser
        uint32 getStatusCode() const
        {
            return iStatusCode;
        }
        uint32 getNumFields()
        {
            return (iKeyValueStore == NULL ? 0 : iKeyValueStore->getNumberOfKeyValuePairs());
        }
        uint32 getFieldKeyList(StrPtrLen *&aFieldKeyList)
        {
            return (iKeyValueStore == NULL ? 0 : iKeyValueStore->getCurrentKeyList(aFieldKeyList));
        }
        bool getField(const StrCSumPtrLen &aNewFieldKey, StrPtrLen &aNewFieldValue, const uint32 index = 0)
        {
            return (iKeyValueStore == NULL ? false : iKeyValueStore->getValueByKey(aNewFieldKey, aNewFieldValue, index));
        }
        uint32 getNumberOfFieldsByKey(const StrCSumPtrLen &aNewFieldName)
        {
            return (iKeyValueStore == NULL ? 0 : iKeyValueStore->getNumberOfValuesByKey(aNewFieldName));
        }
        ///////////////////////////////////////////////////////////////////////////

        // get iKeyValueStore and iEntityUnitAlloc, will be used HTTPParserEntityBodyObject
        StringKeyValueStore *getKeyValuesStore() const
        {
            return iKeyValueStore;
        }
        PVMFBufferPoolAllocator *getAllocator() const
        {
            return iEntityUnitAlloc;
        }

        // constructor
        HTTPParserHeaderObject()
        {
            ;
        }

        // destructor
        ~HTTPParserHeaderObject();

        // reset
        void reset()
        {
            iStatusCode				= 0; //200;
            iHttpVersionNum			= 0;
            iHeaderParsed			= false;
            iHeaderFirstLineParsed  = false;
            iResponseParsedComplete = false;
            if (iKeyValueStore) iKeyValueStore->clear();
            iPrevCRLF = 0;
        }

        // factory method, create iKeyValueStore and iEntityUnitAlloc
        static HTTPParserHeaderObject *create(HTTPContentInfoInternal *aContentInfo);

    private:
        // return value: 0 => ok , 1 => data not enough -1 => syntax error
        int32 parseFirstLine(HTTPMemoryFragment &aInputDataStream);
        bool construct(HTTPContentInfoInternal *aContentInfo);
        bool isGoodStatusCode();
        bool checkGood1xxCode();
        bool checkGood2xxCode();
        bool checkHTTPVersion(char* &aPtr);
        bool checkResponseParsedComplete();
        bool checkChunkedTransferEncodingSupported();

    private:
        HTTPContentInfoInternal *iContentInfo;
        uint32 iStatusCode;
        uint32 iHttpVersionNum;
        bool iHeaderParsed;
        bool iHeaderFirstLineParsed;
        bool iResponseParsedComplete;
};

///////////////////////////////////////////////////////////////////////////////////////
// HTTPParserEntityBodyObject encapsulates HTTP entity body parsing. Here we support three types of entity body,
// normal body without any internal headers (usually in HTTP 1.0), multipart/byteranges used in pv fasttrack download
// and chunked transfer encoding, both having internal headers. To handle these three types of content, the following
// classes are designed to remove conditional using polymorphism, or remove type code with class, i.e. derived classes
// implement the interface defined in the base class, parse().
class HTTPParserEntityBodyObject : public HTTPParserBaseObject
{
    public:
        virtual int32 parse(HTTPParserInput &aParserInput, RefCountHTTPEntityUnit &aEntityUnit) = 0;

        // constructor
        HTTPParserEntityBodyObject(StringKeyValueStore *aKeyValueStore,
                                   PVMFBufferPoolAllocator *aEntityUnitAlloc,
                                   HTTPContentInfoInternal *aContentInfo) :
                HTTPParserBaseObject(aKeyValueStore, aEntityUnitAlloc),
                iContentInfo(aContentInfo),
                iCurrentChunkDataLength(0),
                iNumChunks(0),
                iCounter(0)
        {
            ;
        }

        virtual ~HTTPParserEntityBodyObject()
        {
            ;
        }

    protected:
        // used in HTTPParserCTEContentObject and HTTPParserMultipartContentObject
        int32 parseEnityBodyChunkData(HTTPParserInput &aParserInput, RefCountHTTPEntityUnit &aEntityUnit);

    protected:
        HTTPContentInfoInternal *iContentInfo;
        uint32 iCurrentChunkDataLength;
        uint32 iNumChunks;
        uint32 iCounter; // for debugging purpose
};

///////////////////////////////////////////////////////////////////////////////////////
class HTTPParserNormalContentObject : public HTTPParserEntityBodyObject
{
    public:
        int32 parse(HTTPParserInput &aParserInput, RefCountHTTPEntityUnit &aEntityUnit);

        // constructor
        HTTPParserNormalContentObject(StringKeyValueStore *aKeyValueStore,
                                      PVMFBufferPoolAllocator *aEntityUnitAlloc,
                                      HTTPContentInfoInternal *aContentInfo) :
                HTTPParserEntityBodyObject(aKeyValueStore, aEntityUnitAlloc, aContentInfo),
                iCurrTotalLengthObtained(0)
        {
            ;
        }
        virtual ~HTTPParserNormalContentObject()
        {
            ;
        }

    private:
        uint32 iCurrTotalLengthObtained;
};

///////////////////////////////////////////////////////////////////////////////////////
// CTE = Chunked Transfer Encoding
class HTTPParserCTEContentObject : public HTTPParserEntityBodyObject
{
    public:
        int32 parse(HTTPParserInput &aParserInput, RefCountHTTPEntityUnit &aEntityUnit);

        // constructor
        HTTPParserCTEContentObject(StringKeyValueStore *aKeyValueStore,
                                   PVMFBufferPoolAllocator *aEntityUnitAlloc,
                                   HTTPContentInfoInternal *aContentInfo) :
                HTTPParserEntityBodyObject(aKeyValueStore, aEntityUnitAlloc, aContentInfo)
        {
            ;
        }

        virtual ~HTTPParserCTEContentObject()
        {
            ;
        }
    private:
        bool getCTEChunkLength(HTTPMemoryFragment &aInputLineData, int32 &aChunkSize);
        // reset for next chunk parsing
        void reset()
        {
            iCurrentChunkDataLength = 0;
            if (iContentInfo)
            {
                iContentInfo->iContentRangeLeft  = 0;
                iContentInfo->iContentRangeRight = 0;
            }
        }
};

///////////////////////////////////////////////////////////////////////////////////////
class HTTPParserMultipartContentObject : public HTTPParserEntityBodyObject
{
    public:
        int32 parse(HTTPParserInput &aParserInput, RefCountHTTPEntityUnit &aEntityUnit);

        // constructor
        HTTPParserMultipartContentObject(StringKeyValueStore *aKeyValueStore,
                                         PVMFBufferPoolAllocator *aEntityUnitAlloc,
                                         HTTPContentInfoInternal *aContentInfo) :
                HTTPParserEntityBodyObject(aKeyValueStore, aEntityUnitAlloc, aContentInfo)
        {
            reset();
        }

        virtual ~HTTPParserMultipartContentObject()
        {
            ;
        }
    private:
        void reset()
        {
            iBoudaryLineParsed = iHeaderInEntityBodyParsed = false;
            iCurrentChunkDataLength = 0;
            iNumChunks = 0;
        }

        bool needSkipCRLF()
        {
            // iPrevCRLF&0x3!=0x3 means iPrevCRLF hasn't got complete CRLF (i.e.both CR and LF)
            return ((iNumChunks == 0) && ((iPrevCRLF&0x3) != 0x3));
        }

        int32 parseChunkHeader(HTTPParserInput &aParserInput);
        int32 parseChunkBoundaryLine(HTTPParserInput &aParserInput);

    private:
        bool iBoudaryLineParsed;
        bool iHeaderInEntityBodyParsed;

};


#endif // HTTP_PARSER_INTERNAL_H_

