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
#ifndef PVMF_PROTOCOL_ENGINE_INTERNAL_H_INCLUDED
#define PVMF_PROTOCOL_ENGINE_INTERNAL_H_INCLUDED

#ifndef PVMF_MEDIA_DATA_H_INCLUDED
#include "pvmf_media_data.h"
#endif

#ifndef OSCL_STR_PTR_LEN_H_INCLUDED
#include "oscl_str_ptr_len.h"
#endif

#ifndef OSCL_TIME_H_INCLUDED
#include "oscl_time.h"
#endif

#ifndef OSCL_STRING_CONSTAINERS_H_INCLUDED
#include "oscl_string_utils.h"
#endif

#ifndef OSCL_SNPRINTF_H_INCLUDED
#include "oscl_snprintf.h"
#endif

#ifndef OSCL_STRING_CONSTAINERS_H_INCLUDED
#include "oscl_string_containers.h"
#endif

#ifndef HTTP_COMPOSER_H_INCLUDED
#include "http_composer.h"
#endif

#ifndef HTTP_PARSER_H_INCLUDED
#include "http_parser.h"
#endif

#ifndef PVMF_PROTOCOL_ENGINE_NODE_EXTENSION_H_INCLUDED
#include "pvmf_protocol_engine_node_extension.h" // for class PVMFProtocolEngineNodeMSHTTPStreamingParams
#endif

#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif

// log macros
#define LOGERROR(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iLogger,PVLOGMSG_ERR,m);
#define LOGINFOHI(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG,iLogger,PVLOGMSG_INFO,m);
#define LOGINFOMED(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG,iLogger,PVLOGMSG_INFO,m);
#define LOGINFOLOW(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG,iLogger,PVLOGMSG_INFO,m);
#define LOGINFO(m) LOGINFOMED(m)
#define LOGINFODATAPATH(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG,iDataPathLogger,PVLOGMSG_INFO,m);
#define LOGERRORDATAPATH(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iDataPathLogger,PVLOGMSG_ERR,m);
#define LOGINFOCLOCK(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG,iClockLogger,PVLOGMSG_INFO,m);
#define PVMF_PROTOCOL_ENGINE_LOGBIN(iPortLogger, m) PVLOGGER_LOGBIN(PVLOGMSG_INST_LLDBG, iPortLogger, PVLOGMSG_ERR, m);
#define PVMF_PROTOCOL_ENGINE_LOGINFODATAPATH(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG,iDataPathLogger,PVLOGMSG_INFO,m);
#define PVMF_PROTOCOL_ENGINE_LOGERRINFODATAPATH(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG,iDataPathErrLogger,PVLOGMSG_INFO,m);
#define	NODEDATAPATHLOGGER_TAG "datapath.sourcenode.protocolenginenode"
#define DATAPATHLOGGER_TAG "protocolenginenode.protocolengine"
#define DATAPATHERRLOGGER_TAG "datapath.sourcenode.protocolenginenode"


enum pvHttpProcessingMicroState
{
    EHttpProcessingMicroState_SendRequest,
    EHttpProcessingMicroState_GetResponse,
};

enum ProcessingStateReturnCodes
{
    PROCESS_SUCCESS							 = 0,
    PROCESS_SUCCESS_END_OF_MESSAGE			 = 1,
    PROCESS_SUCCESS_END_OF_MESSAGE_WITH_EXTRA_DATA = 2,
    PROCESS_SUCCESS_END_OF_MESSAGE_TRUNCATED = 3,
    PROCESS_SUCCESS_END_OF_MESSAGE_BY_SERVER_DISCONNECT = 4,
    PROCESS_SUCCESS_GOT_EOS					 = 5,
    PROCESS_OUTPUT_PORT_IS_BUSY				 = 6,
    PROCESS_WAIT_FOR_INCOMING_DATA			 = 7,

    // info from server
    PROCESS_SERVER_RESPONSE_ERROR			 = 10,  // status code of the first http response line >= 300

    // errors
    PROCESS_ERROR_FIRST						 = -100,
    PROCESS_GENERAL_ERROR					 = PROCESS_ERROR_FIRST - 1,
    PROCESS_INPUT_OUTPUT_NOT_READY			 = PROCESS_ERROR_FIRST - 2,		// input(uri) or output(port or data stream) for the http parcom object is not ready
    PROCESS_BAD_URL							 = PROCESS_ERROR_FIRST - 3,
    PROCESS_MEDIA_DATA_CREATE_FAILURE		 = PROCESS_ERROR_FIRST - 4,		// createMediaData() fails
    PROCESS_COMPOSE_HTTP_REQUEST_BUFFER_SIZE_NOT_MATCH_REQUEST_SIZE = PROCESS_ERROR_FIRST - 5,
    PROCESS_COMPOSE_HTTP_REQUEST_FAILURE	 = PROCESS_ERROR_FIRST - 6,		// fail in composing http request, iComposer->compose() fails
    PROCESS_PARSE_HTTP_RESPONSE_FAILURE		 = PROCESS_ERROR_FIRST - 7,		// fail in parsing response
    PROCESS_DATA_STREAM_OPEN_FAILURE		 = PROCESS_ERROR_FIRST - 8,		// fail in data stream OpenSession()
    PROCESS_OUTPUT_TO_DATA_STREAM_FAILURE	 = PROCESS_ERROR_FIRST - 9,		// fail in write data to data stream object
    PROCESS_MEMORY_ALLOCATION_FAILURE		 = PROCESS_ERROR_FIRST - 10,
    PROCESS_HTTP_VERSION_NOT_SUPPORTED	     = PROCESS_ERROR_FIRST - 11,
    PROCESS_ASF_HEADER_SIZE_EXCEED_LIMIT	 = PROCESS_ERROR_FIRST - 12,
    PROCESS_CHUNKED_TRANSFER_ENCODING_NOT_SUPPORT = PROCESS_ERROR_FIRST - 13,
    PROCESS_TIMEOUT_SERVER_NO_RESPONCE	     = PROCESS_ERROR_FIRST - 14,
    PROCESS_TIMEOUT_SERVER_INACTIVITY		 = PROCESS_ERROR_FIRST - 15,
    PROCESS_CONTENT_LENGTH_NOT_MATCH		 = PROCESS_ERROR_FIRST - 16,
    PROCESS_CONTENT_RANGE_INFO_NOT_MATCH	 = PROCESS_ERROR_FIRST - 17,
    PROCESS_OUTPUT_TO_OUTPUT_PORT_FAILURE	 = PROCESS_ERROR_FIRST - 18,
    PROCESS_REACHED_MAXIMUM_SIZE_LIMITATION	 = PROCESS_ERROR_FIRST - 19

};

enum ProtocolType
{
    PROGRESSIVE_DOWNLOAD = 0,
    FASTTRACK_DOWNLOAD,
    MS_HTTP_STREAMING
};

#define DEFAULT_HTTP_PORT_NUMBER		80
#define NUM_PROGRESSIVE_DOWNLOAD_STATE	2
#define NUM_FASTTRACK_DOWNLOAD_STATE	2
#define NUM_MS_STREAMING_STATE			5
#define MAX_NUM_RUNTIME_MS_STREAMING_STATE 16
#define DEFAULT_VECTOR_RESERVE_NUMBER	4
#define DEFAULT_CLIENT_GUID				_STRLIT_CHAR("{70CD3310-1598-CE7E-919A-456A4E6E26A0}")
#define MAX_NUM_EOS_MESSAGES_FOR_SAME_REQUEST 2
#define MIN_URL_LENGTH 1
#define DEFAULT_MS_HTTP_STREAMING_SERVER_VERSION 9
#define DEFAULT_MS_STREAMING_ACCEL_BITRATE 3670016
#define DEFAULT_MS_STREAMING_ACCEL_DURATION 10000

// mask for checking HTTP methods for each extension header
#define MASK_HTTPGET_EXTENSIONHEADER	0x1	// bit 0 for HTTP GET
#define MASK_HTTPPOST_EXTENSIONHEADER	0x2	// bit 1 for HTTP POST
#define MASK_HTTPHEAD_EXTENSIONHEADER	0x4	// bit 2 for HTTP HEAD

// sequence number for different type packet, used in ProtocolEngineOutputDataSideInfo
#define ASF_HEADER_SEQNUM  0xffffffff
#define EOS_COMMAND_SEQNUM 0xfffffffe

typedef Oscl_Vector<PVMFSharedMediaMsgPtr, OsclMemAllocator> INPUT_DATA_QUEUE;
typedef Oscl_Vector<OsclRefCounterMemFrag, OsclMemAllocator> OUTPUT_DATA_QUEUE;

#define PE_isDigit(c) ((c) >= 48 && (c) <= 57)
#define PROTOCOLENGINE_REDIRECT_STATUS_CODE_START	300
#define PROTOCOLENGINE_REDIRECT_STATUS_CODE_END		399


////////////////////////////////////////////////////////////////////////////////////
class BasicAlloc : public Oscl_DefAlloc
{
    public:
        void* allocate_fl(const uint32 size, const char *file_name, const int line_number)
        {
            void* tmp = (void*)iDefAlloc.allocate_fl(size, file_name, line_number);
            OSCL_ASSERT(tmp != 0);
            return tmp;
        }
        void* allocate(const uint32 size)
        {
            void* tmp = (void*)iDefAlloc.ALLOCATE(size);
            OSCL_ASSERT(tmp != 0);
            return tmp;
        }
        void deallocate(void* p)
        {
            iDefAlloc.deallocate(p);
        }
    private:
        OsclMemAllocator iDefAlloc;
};

template <class T> class PVDlSharedPtrAlloc: public OsclDestructDealloc
{
    public:
        T* allocate()
        {
            BasicAlloc alloc;
            T* ptr = OSCL_PLACEMENT_NEW(alloc.allocate(sizeof(T)), T());
            return ptr;
        }

        virtual void destruct_and_dealloc(OsclAny* ptr)
        {
            T* tptr ;
            tptr = reinterpret_cast<T*>(ptr);
            tptr->~T();
            BasicAlloc alloc;
            alloc.deallocate(ptr);
        }
};

// To create OsclRefCounterMemFrag with the actual memory allocation
#define OSCL_REFCOUNTER_MEMFRAG_DEFAULT_SIZE 256
// for clean up object in reference counter object
class OsclRefCounterMemFragCleanupDA : public OsclDestructDealloc
{
    public:
        OsclRefCounterMemFragCleanupDA(Oscl_DefAlloc* in_gen_alloc) :
                iGenAlloc(in_gen_alloc) {};
        virtual ~OsclRefCounterMemFragCleanupDA() {};

        void destruct_and_dealloc(OsclAny* ptr)
        {
            iGenAlloc->deallocate(ptr);
        }

    private:
        Oscl_DefAlloc* iGenAlloc;
};

class OsclRefCounterMemFragCleanupSA : public OsclDestructDealloc
{
    public:
        virtual ~OsclRefCounterMemFragCleanupSA() {};
        virtual void destruct_and_dealloc(OsclAny* ptr)
        {
            // no need to call destructors in this case just dealloc
            OsclMemAllocator alloc;
            alloc.deallocate(ptr);
        }
};

class OsclRefCounterMemFragAlloc
{
    public:

        //! constructor
        OsclRefCounterMemFragAlloc(Oscl_DefAlloc* aGenAlloc = NULL) : iGenAlloc(aGenAlloc)
        {
            ;
        }

        /**
         * Create a OsclRefCounterMemFrag object that has the actual memory
         * @return the shared pointer of the OsclRefCounterMemFrag object
         */
        OsclRefCounterMemFrag allocate(uint32 requested_size = OSCL_REFCOUNTER_MEMFRAG_DEFAULT_SIZE)
        {

            if (!iGenAlloc) return allocateSA(requested_size);

            uint32 aligned_cleanup_size	= oscl_mem_aligned_size(sizeof(OsclRefCounterMemFragCleanupDA));
            uint32 aligned_refcnt_size	= oscl_mem_aligned_size(sizeof(OsclRefCounterDA));
            uint32 aligned_class_size   = oscl_mem_aligned_size(sizeof(OsclMemoryFragment));

            uint32 total_size = aligned_refcnt_size + aligned_cleanup_size + aligned_class_size + requested_size;
            uint8* my_ptr = (uint8*) iGenAlloc->ALLOCATE(total_size);

            // 1. create clean up object (for the reference count object). Note that the first pointer should be for reference count object
            OsclRefCounterMemFragCleanupDA* cleanup_ptr = OSCL_PLACEMENT_NEW(my_ptr + aligned_refcnt_size, OsclRefCounterMemFragCleanupDA(iGenAlloc));

            // 2. create the refcounter after the cleanup object, at the very beginning
            OsclRefCounter* my_refcnt = OSCL_PLACEMENT_NEW(my_ptr, OsclRefCounterDA(my_ptr, cleanup_ptr));
            my_ptr += (aligned_refcnt_size + aligned_cleanup_size);

            // 3. create OsclMemoryFragment object after refcounter object and cleanup object
            OsclMemoryFragment* memfrag_ptr = OSCL_PLACEMENT_NEW(my_ptr, OsclMemoryFragment);
            memfrag_ptr->ptr = (void*)(my_ptr + aligned_class_size);
            memfrag_ptr->len = requested_size;

            OsclRefCounterMemFrag refMemFrag(*memfrag_ptr, my_refcnt, requested_size);
            return refMemFrag;
        }

    private:
        OsclRefCounterMemFrag allocateSA(uint32 requested_size = OSCL_REFCOUNTER_MEMFRAG_DEFAULT_SIZE)
        {
            uint32 aligned_refcnt_size = oscl_mem_aligned_size(sizeof(OsclRefCounterSA<OsclRefCounterMemFragCleanupSA>));
            uint32 aligned_class_size  = oscl_mem_aligned_size(sizeof(OsclMemoryFragment));
            uint32 total_size = aligned_refcnt_size + aligned_class_size + requested_size;

            OsclMemAllocator my_alloc;
            uint8* my_ptr = (uint8*) my_alloc.ALLOCATE(total_size);

            // 1. create the reference counter object
            OsclRefCounter* my_refcnt = OSCL_PLACEMENT_NEW(my_ptr, OsclRefCounterSA<OsclRefCounterMemFragCleanupSA>(my_ptr));
            my_ptr += aligned_refcnt_size;

            // 2. create OsclMemoryFragment object after refcounter object object
            OsclMemoryFragment* memfrag_ptr = OSCL_PLACEMENT_NEW(my_ptr, OsclMemoryFragment);
            memfrag_ptr->ptr = (void*)(my_ptr + aligned_class_size);
            memfrag_ptr->len = requested_size;

            OsclRefCounterMemFrag refMemFrag(*memfrag_ptr, my_refcnt, requested_size);
            return refMemFrag;
        }

    private:
        Oscl_DefAlloc* iGenAlloc;
};

class PVProtocolEngineMSHttpStreamingParams
{
    public:
        PVProtocolEngineMSHttpStreamingParams()
        {
            reset();
            iStreamIDList.reserve(DEFAULT_VECTOR_RESERVE_NUMBER);
            iStreamPlayBackModeList.reserve(DEFAULT_VECTOR_RESERVE_NUMBER);
            iStreamExtensionHeaderKeys.reserve(DEFAULT_VECTOR_RESERVE_NUMBER);
            iStreamExtensionHeaderValues.reserve(DEFAULT_VECTOR_RESERVE_NUMBER);
            iMethodMaskForExtensionHeaders.reserve(DEFAULT_VECTOR_RESERVE_NUMBER);
            iExtensionHeadersPurgeOnRedirect.reserve(DEFAULT_VECTOR_RESERVE_NUMBER);

        };

        ~PVProtocolEngineMSHttpStreamingParams()
        {
            reset();
        };

        // reset to default values
        void reset()
        {
            iStreamRate = 1;
            iStreamByteOffset = 0xFFFFFFFF;
            iStreamStartPacketNumber = 0xFFFFFFFF;
            iStreamStartTimeInMS = 0;
            iMaxStreamDurationInMS = 0x800188A3;
            iPacketGrouping = 0;
            iNumStreams = 1;
            iHttpVersion = 0;
            iPlayBackMode = 1;
            iMaxASFHeaderSize = PVPROTOCOLENGINE_DEFAULT_MAXIMUM_ASF_HEADER_SIZE;
            iAccelBitrate = DEFAULT_MS_STREAMING_ACCEL_BITRATE;
            iAccelDuration = DEFAULT_MS_STREAMING_ACCEL_DURATION;
            iMaxHttpStreamingSize = 0;
            iStreamIDList.clear();
            iStreamPlayBackModeList.clear();
            iUserAgent = OSCL_HeapString<OsclMemAllocator> (_STRLIT_CHAR("NSPlayer/10.0.0.3646"));
            iStreamExtensionHeaderKeys.clear();
            iStreamExtensionHeaderValues.clear();
            iMethodMaskForExtensionHeaders.clear();
            iExtensionHeadersPurgeOnRedirect.clear();
        }

        // copy constructor
        PVProtocolEngineMSHttpStreamingParams(const PVMFProtocolEngineNodeMSHTTPStreamingParams &x)
        {
            iStreamRate					= x.iStreamRate;
            iStreamByteOffset			= x.iStreamByteOffset;
            iStreamStartTimeInMS		= x.iStreamStartTimeInMS;
            iMaxStreamDurationInMS		= x.iMaxStreamDurationInMS;
            iStreamStartPacketNumber	= x.iStreamStartPacketNumber;
            iPacketGrouping				= x.iPacketGrouping;
            iNumStreams					= x.iNumStreams;
            iPlayBackMode				= x.iPlayBackMode;
            iStreamIDList				= x.iStreamIDList;
            iStreamPlayBackModeList		= x.iStreamPlayBackModeList;
        }

        // assignment operator
        PVProtocolEngineMSHttpStreamingParams& operator=(const PVMFProtocolEngineNodeMSHTTPStreamingParams& x)
        {
            iStreamRate					= x.iStreamRate;
            iStreamByteOffset			= x.iStreamByteOffset;
            iStreamStartTimeInMS		= x.iStreamStartTimeInMS;
            iMaxStreamDurationInMS		= x.iMaxStreamDurationInMS;
            iStreamStartPacketNumber	= x.iStreamStartPacketNumber;
            iPacketGrouping				= x.iPacketGrouping;
            iNumStreams					= x.iNumStreams;
            iPlayBackMode				= x.iPlayBackMode;
            iStreamIDList				= x.iStreamIDList;
            iStreamPlayBackModeList		= x.iStreamPlayBackModeList;
            return *this;
        }

        PVProtocolEngineMSHttpStreamingParams& operator=(const PVProtocolEngineMSHttpStreamingParams& x)
        {
            iStreamRate					= x.iStreamRate;
            iStreamByteOffset			= x.iStreamByteOffset;
            iStreamStartTimeInMS		= x.iStreamStartTimeInMS;
            iMaxStreamDurationInMS		= x.iMaxStreamDurationInMS;
            iStreamStartPacketNumber	= x.iStreamStartPacketNumber;
            iPacketGrouping				= x.iPacketGrouping;
            iHttpVersion				= x.iHttpVersion;
            iPlayBackMode				= x.iPlayBackMode;
            iNumStreams					= x.iNumStreams;
            iMaxASFHeaderSize			= x.iMaxASFHeaderSize;
            iUserAgent					= x.iUserAgent;
            iUserID						= x.iUserID;
            iUserPasswd					= x.iUserPasswd;
            iStreamIDList				= x.iStreamIDList;
            iStreamPlayBackModeList		= x.iStreamPlayBackModeList;
            iStreamExtensionHeaderKeys		= x.iStreamExtensionHeaderKeys;
            iStreamExtensionHeaderValues	= x.iStreamExtensionHeaderValues;
            iMethodMaskForExtensionHeaders	= x.iMethodMaskForExtensionHeaders;
            iExtensionHeadersPurgeOnRedirect = x.iExtensionHeadersPurgeOnRedirect;
            iAccelBitrate					= x.iAccelBitrate;
            iAccelDuration					= x.iAccelDuration;
            iMaxHttpStreamingSize           = x.iMaxHttpStreamingSize;
            return *this;
        }

        uint32 iStreamRate;
        uint32 iStreamByteOffset;
        uint32 iStreamStartTimeInMS;
        uint32 iMaxStreamDurationInMS;
        uint32 iStreamStartPacketNumber;
        uint32 iPacketGrouping;
        uint32 iHttpVersion;
        uint32 iPlayBackMode;
        uint32 iNumStreams;
        uint32 iMaxASFHeaderSize;
        uint32 iAccelBitrate;
        uint32 iAccelDuration;
        uint32 iMaxHttpStreamingSize;
        OSCL_HeapString<OsclMemAllocator> iUserAgent;
        OSCL_HeapString<OsclMemAllocator> iUserID;
        OSCL_HeapString<OsclMemAllocator> iUserPasswd;
        Oscl_Vector<uint32, OsclMemAllocator> iStreamIDList;
        Oscl_Vector<uint32, OsclMemAllocator> iStreamPlayBackModeList;
        Oscl_Vector<OSCL_HeapString<OsclMemAllocator>, OsclMemAllocator> iStreamExtensionHeaderKeys;
        Oscl_Vector<OSCL_HeapString<OsclMemAllocator>, OsclMemAllocator> iStreamExtensionHeaderValues;
        Oscl_Vector<uint32, OsclMemAllocator> iMethodMaskForExtensionHeaders; // bit 0 = 1 => HTTP GET method
        // bit 1 = 1 => HTTP POST method
        Oscl_Vector<bool, OsclMemAllocator> iExtensionHeadersPurgeOnRedirect;

};


////////////////////////////////////////////////////////////////////////////////////
// This class encapsulates input part, the major function is getValidMediaData
class pvHttpDownloadInput
{
    public:
        // constructor
        pvHttpDownloadInput()
        {
            unbind();
        }

        // destructor
        virtual ~pvHttpDownloadInput()
        {
            unbind();
        }

        // reset
        void unbind()
        {
            iCurrentInputMediaData.Unbind();
        }

        // get valid media data from input data queue, valid => timestamp != 0xFFFFFFFF
        bool getValidMediaData(INPUT_DATA_QUEUE &aDataInQueue, PVMFSharedMediaDataPtr &aMediaData, bool &isEOS);

    private:
        bool isValidInput();

    private:
        PVMFSharedMediaDataPtr iCurrentInputMediaData;
};

////////////////////////////////////////////////////////////////////////////////////
// Wrapper class for url provides url parsing
class INetURI
{
    public:
        bool setURI(OSCL_String &aUri, const bool aRedirectURI = false)
        {
            iURI = OSCL_HeapString<OsclMemAllocator> (aUri.get_cstr(), aUri.get_size());
            iHostName.set(NULL, 0); // clear iHost
            iRedirectURI = aRedirectURI;
            return true;
        }
        OSCL_IMPORT_REF bool setURI(OSCL_wString &aUri, const bool aRedirectURI = false);

        // get APIs
        OSCL_String &getURI()
        {
            return iURI;
        }
        OSCL_String &getHost()
        {
            return iHostName;
        }
        OSCL_IMPORT_REF bool getHostAndPort(OSCL_String &aSerAdd, int32 &aSerPort);
        bool empty()
        {
            return (iURI.get_size() == 0);
        }
        bool isGoodUri()
        {
            OSCL_HeapString<OsclMemAllocator> tmpUrl8;
            int32 port;
            return getHostAndPort(tmpUrl8, port);
        }
        bool isUseAbsoluteURI() const
        {
            return !iUseRelativeURI;
        }
        void setUsAbsoluteURI()
        {
            iUseRelativeURI = false;
        }
        bool isRedirectURI() const
        {
            return iRedirectURI;
        }

        // constructor
        INetURI() : iHostPort(DEFAULT_HTTP_PORT_NUMBER), iUseRelativeURI(true), iRedirectURI(false)
        {
            ;
        }

        // assignment operator
        INetURI& operator=(const INetURI& x)
        {
            iURI			= x.iURI;
            iHostName		= x.iHostName;
            iHostPort		= x.iHostPort;
            iUseRelativeURI = x.iUseRelativeURI;
            iRedirectURI	= x.iRedirectURI;
            return *this;
        }

    private:
        bool parseURL(OSCL_String &aUrl8, OSCL_String &aSerAdd, int32 &aSerPort);

    private:
        OSCL_HeapString<OsclMemAllocator> iURI;
        OSCL_HeapString<OsclMemAllocator> iHostName;
        int32 iHostPort;
        bool iUseRelativeURI;
        bool iRedirectURI;
};

////////////////////////////////////////////////////////////////////////////////////
// This class wraps http parser and do basic parsing for every input media data at a time, and also provides
// necessary information for other objects. The objects of this class are running in callback mode, i.e.
// when output data in terms of each entity unit (e.g. http header or data chunk) is available, it will
// notify user to retrieve the output data, since the major consideration here is input is assumed arbitrary.
// Also, the output data is input data fragment queue, which needs to be provided by user. That means
// HttpParsingBasicObject object doesn't want to do any memory copy. In other words, user provides this output
// data queue, and this object will write data to this queue.

//typedef Oscl_Vector<OsclRefCounterMemFrag, OsclMemAllocator> OUTPUT_DATA_QUEUE;

class HttpParsingBasicObjectObserver
{
    public:
        virtual ~HttpParsingBasicObjectObserver() {}

        // when HttpParsingBasicObject object generates output data, it will notify the observer the data is available
        // aOutputQueue, data inside the queue needs to be processed/copied as soon as possible
        // Return value would be HttpParsingBasicObject enum return codes, especially for end of message declaration
        virtual int32 OutputDataAvailable(OUTPUT_DATA_QUEUE *aOutputQueue, const bool isHttpHeader) = 0;
};

struct BandwidthEstimationInfo
{
    uint32 iFirstMediaDataTsPerRequest;
    uint32 iFirstMediaDataSizePerRequest;
    uint32 iTotalSizePerRequest;
    uint32 iLatestMediaDataTimestamp;

    // constructor
    BandwidthEstimationInfo()
    {
        clear();
    }

    // clear()
    void clear()
    {
        iFirstMediaDataTsPerRequest		= 0;
        iFirstMediaDataSizePerRequest	= 0;
        iTotalSizePerRequest			= 0;
        iLatestMediaDataTimestamp		= 0;
        iPrevHttpHeaderParsed			= false;
        iMediaDataFragPtr				= NULL;
    }

    // update
    void update(PVMFSharedMediaDataPtr &aMediaData, const bool aHttpHeaderParsed = true)
    {
        if (!aHttpHeaderParsed) return;

        // aHttpHeaderParsed=true, HTTP header should be already parsed
        OsclRefCounterMemFrag fragIn;
        aMediaData->getMediaFragment(0, fragIn);
        if (!iPrevHttpHeaderParsed)
        {
            iFirstMediaDataTsPerRequest = aMediaData->getTimestamp();
            iFirstMediaDataSizePerRequest = fragIn.getMemFragSize();
            iTotalSizePerRequest = 0;
            iLatestMediaDataTimestamp = 0;
            iPrevHttpHeaderParsed = true;
        }

        // update iLatestMediaDataTimestamp
        if (iLatestMediaDataTimestamp < aMediaData->getTimestamp())
        {
            iLatestMediaDataTimestamp = aMediaData->getTimestamp();
        }
        // update iTotalSizePerRequest
        if (iMediaDataFragPtr != (uint8*)fragIn.getMemFragPtr())
        {
            iTotalSizePerRequest += fragIn.getMemFragSize();
            iMediaDataFragPtr = (uint8*)fragIn.getMemFragPtr();
        }
    }

private:
    bool iPrevHttpHeaderParsed;
    uint8 *iMediaDataFragPtr;

};

class HttpParsingBasicObject
{
    public:
        // get/query functions
        uint32 getContentLength(const bool aRefresh = false)
        {
            if (aRefresh) iParser->getContentInfo(iContentInfo);
            return iContentInfo.iContentLength;
        }
        uint32 getDownloadSize()
        {
            return iTotalDLHttpBodySize;
        }
        OSCL_IMPORT_REF void setDownloadSize(const uint32 aInitialSize = 0);

        uint32 getTotalHttpStreamingSize()
        {
            return iTotalHttpStreamingSize;
        }
        void resetTotalHttpStreamingSize()
        {
            iTotalHttpStreamingSize = 0;
        }

        uint32 getStatusCode()
        {
            return (iParser == NULL ? 0 : iParser->getHTTPStatusCode());
        }
        HTTPParser *getHttpParser()
        {
            return iParser;
        }
        bool isHttpHeaderParsed()
        {
            return iHttpHeaderParsed;
        }
        OSCL_IMPORT_REF bool getRedirectURI(OSCL_String &aRedirectUri);
        OSCL_IMPORT_REF bool getContentType(OSCL_String &aContentType);
        OSCL_IMPORT_REF bool getAuthenInfo(OSCL_String &aRealm);
        OSCL_IMPORT_REF bool isServerSupportBasicAuthentication();
        OSCL_IMPORT_REF bool isServerSendAuthenticationHeader();
        OSCL_IMPORT_REF void getBasicPtr(const StrPtrLen aAuthenValue, uint32 &length);
        OSCL_IMPORT_REF void getRealmPtr(const char *&ptrRealm, uint32 &len, uint32 &length);

        uint32 getServerVersionNumber()
        {
            return iServerVersionNumber;
        }
        OSCL_IMPORT_REF int32 isNewContentRangeInfoMatchingCurrentOne(const uint32 aPrevContentLength);
        bool isDownloadReallyHappen() const
        {
            return (iTotalDLHttpBodySize -iTotalDLSizeForPrevEOS > 0);
        }
        uint32 getLatestMediaDataTimestamp() const
        {
            return iLatestMediaDataTimestamp;
        }
        BandwidthEstimationInfo *getBandwidthEstimationInfo()
        {
            return &iBWEstInfo;
        }
        void setNumRetry(const uint32 aNumRetry = MAX_NUM_EOS_MESSAGES_FOR_SAME_REQUEST)
        {
            iNumRetry = aNumRetry;
        }

        // return false if (aObserver==NULL || outputQueue==NULL)
        bool registerObserverAndOutputQueue(HttpParsingBasicObjectObserver *aObserver, OUTPUT_DATA_QUEUE *aOutputQueue)
        {
            iObserver = aObserver;
            iOutputQueue = aOutputQueue;
            return (iObserver != NULL && iOutputQueue != NULL);
        }

        // see the following enum definition to get to know the return values
        OSCL_IMPORT_REF int32 parseResponse(INPUT_DATA_QUEUE &aDataQueue);

        // return codes for parseResponse function
        enum PARSE_RETURN_CODES
        {
            PARSE_SUCCESS						 = 0,
            PARSE_HEADER_AVAILABLE				 = 1,	// HTTP header is parsed
            PARSE_SUCCESS_END_OF_MESSAGE         = 2,	// success with end of the message (get data with the size of content-length)
            PARSE_SUCCESS_END_OF_MESSAGE_WITH_EXTRA_DATA = 3, // success with end of the message (get data with the size of content-length), but with extran data appended
            PARSE_SUCCESS_END_OF_INPUT           = 4,	// success with end of the input
            PARSE_NEED_MORE_DATA				 = 5,	// no ouput or could have output, need more data
            PARSE_STATUS_LINE_SHOW_NOT_SUCCESSFUL = 6,	// parse the first http status line,
            // got status code >= 300, 3xx=>redirection, 4xx=>client error, 5xx=>server error
            // note that this is return code when parser just finishes parsing the first line,
            // user can continue calling parse function to get the complete http header

            PARSE_NO_INPUT_DATA					 = 10,  // input data queue has no data or eos
            PARSE_EOS_INPUT_DATA				 = 11,

            // errors
            PARSE_GENERAL_ERROR					 = -1,
            PARSE_SYNTAX_ERROR					 = -2,	// syntax is not understandable
            PARSE_HTTP_VERSION_NOT_SUPPORTED     = -3,
            PARSE_TRANSFER_ENCODING_NOT_SUPPORTED = -4,
            PARSE_CONTENT_RANGE_INFO_NOT_MATCH   = -5,	// range info in the request doesn't match content-range info from the response
            PARSE_CONTENT_LENGTH_NOT_MATCH		 = -6,	// content-length info from the response doesn't match from the content-length in config file
            PARSE_BAD_URL						 = -7	// bad url causes server to shut down the connection, so view first (server)
            // response (from socket node) being EOS as bad url
        };

        // reset for parsing each response
        void reset()
        {
            if (iParser) iParser->reset();
            iInput.unbind();
            iContentInfo.clear();
            iBWEstInfo.clear();
            iHttpHeaderParsed = false;
            iTotalDLHttpBodySize = 0;
            iLatestMediaDataTimestamp = 0;
        }

        // reset for parsing multiple responses for each protocol state or node state
        void resetForBadConnectionDetection()
        {
            iNumEOSMessagesAfterRequest = 0;
            iTotalDLSizeAtCurrEOS  = 0;
            iTotalDLSizeForPrevEOS = 0;
        }

        // factory method
        OSCL_IMPORT_REF static HttpParsingBasicObject *create();
        // destructor
        virtual ~HttpParsingBasicObject()
        {
            iLogger = NULL;
            iDataPathLogger = NULL;
            iClockLogger = NULL;
            OSCL_DELETE(iParser);
        }

    private:
        // constructor
        HttpParsingBasicObject() :	iParser(NULL),
                iObserver(NULL),
                iOutputQueue(NULL),
                iNumRetry(MAX_NUM_EOS_MESSAGES_FOR_SAME_REQUEST)
        {
            iLogger = PVLogger::GetLoggerObject("PVMFProtocolEngineNode");
            iDataPathLogger = PVLogger::GetLoggerObject("protocolenginenode.protocolengine");
            iDataPathErrLogger = PVLogger::GetLoggerObject("datapath.sourcenode.protocolenginenode");
            iClockLogger = PVLogger::GetLoggerObject("clock");
            iTotalHttpStreamingSize = 0;
            reset();
        }

        // called by create()
        bool construct();
        // return code: PARSE_SUCCESS for new data, PARSE_EOS_INPUT_DATA for eos, PARSE_NO_INPUT_DATA for no input data
        int32 getNextMediaData(INPUT_DATA_QUEUE &aDataInQueue, PVMFSharedMediaDataPtr &aMediaData);
        // EOS input can be indicated as the sign of bad request (the request just sent is bad request, especially badk URL)
        int32 validateEOSInput(int32 parsingStatus);
        // return total size of all data in entityUnit, ~0=0xffffffff means error
        bool saveOutputData(RefCountHTTPEntityUnit &entityUnit, OUTPUT_DATA_QUEUE &aOutputData, uint32 &aTotalEntityDataSize);
        // return codes conversion, may send out callback for end of message or end of input cases
        int32 checkParsingDone(const int32 parsingStatus);
        // extract the server version number from server field of a HTTP response
        void extractServerVersionNum();
        // called by getNextMediaData()
        bool isRedirectResponse();

        void clearInputOutput()
        {
            iInput.unbind();
            if (iOutputQueue) iOutputQueue->clear();
        }
        friend class HttpParsingBasicObjectAutoCleanup;

    private:
        HTTPParser   *iParser;
        pvHttpDownloadInput iInput;
        HTTPContentInfo iContentInfo;
        bool iHttpHeaderParsed;
        HttpParsingBasicObjectObserver *iObserver;
        OUTPUT_DATA_QUEUE *iOutputQueue;
        uint32 iServerVersionNumber;
        uint32 iTotalDLHttpBodySize;
        uint32 iNumEOSMessagesAfterRequest;// number of continuous EOS messages received after sending request
        uint32 iTotalDLSizeForPrevEOS; // for detecting download size change between two adjacent EOSs or start and first EOS
        uint32 iTotalDLSizeAtCurrEOS;
        uint32  iLatestMediaDataTimestamp;
        BandwidthEstimationInfo iBWEstInfo;
        uint32 iNumRetry;

        PVLogger* iLogger;
        PVLogger* iDataPathLogger;
        PVLogger* iDataPathErrLogger;
        PVLogger* iClockLogger;

        uint32 iTotalHttpStreamingSize;
};


// A simple container class to do HttpParsingBasicObject::clearInputOutput automatically
class HttpParsingBasicObjectAutoCleanup
{
    public:
        HttpParsingBasicObjectAutoCleanup(HttpParsingBasicObject *aParser) : iParser(aParser)
        {
            ;
        }
        ~HttpParsingBasicObjectAutoCleanup()
        {
            if (iParser) iParser->clearInputOutput();
        }

    private:
        HttpParsingBasicObject *iParser;
};

#endif

