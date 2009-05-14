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
/**
 * @file pvmfprotocolenginenode_extension.h
 * @brief Extension interface for PVMFProtocolEngineNode
 */

#ifndef PVMFPROTOCOLENGINENODE_EXTENSION_H_INCLUDED
#define PVMFPROTOCOLENGINENODE_EXTENSION_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef PV_INTERFACE_H
#include "pv_interface.h"
#endif
#ifndef PV_UUID_H_INCLUDED
#include "pv_uuid.h"
#endif
#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif
#ifndef OSCL_REFCOUNTER_MEMFRAG_H_INCLUDED
#include "oscl_refcounter_memfrag.h"
#endif
#ifndef OSCL_STRING_CONSTAINERS_H_INCLUDED
#include "oscl_string_containers.h"
#endif
#ifndef PVMF_EVENT_HANDLING_H_INCLUDED
#include "pvmf_event_handling.h"
#endif
#ifndef PVMF_PORT_INTERFACE_H_INCLUDED
#include "pvmf_port_interface.h"
#endif

// default number of redirect trials
#define PVPROTOCOLENGINE_DEFAULT_NUMBER_OF_REDIRECT_TRIALS 10
#define PVPROTOCOLENGINE_DEFAULT_MAXIMUM_ASF_HEADER_SIZE 262144 // 256K



#define PVMF_PROTOCOL_ENGINE_GENERIC_EXTENSION_MIMETYPE "pvxxx/PVMFProtocolEngineNode/GenericExtensionInterface"
#define KPVMFProtocolEngineNodeExtensionUuid PVUuid(0xca27cb64,0x83ed,0x40d6,0x96,0xa3,0xed,0x1d,0x8b,0x60,0x11,0x38)

enum HttpVersion
{
    HTTP_V10 = 0, // Http version 1.0
    HTTP_V11	// Http version 1.1
};

enum HttpMethod
{
    HTTP_GET = 0,
    HTTP_POST,
    HTTP_HEAD,
    HTTP_ALLMETHOD
};

enum DownloadProgressMode
{
    DownloadProgressMode_TimeBased = 0,
    DownloadProgressMode_ByteBased
};

/**
 * PVMFProtocolEngineNodeExtensionInterface allows a client to do exercise extended functions
 * of a PVMFProtocolEngineNode
 */
class PVMFProtocolEngineNodeExtensionInterface : public PVInterface
{
    public:

        /**
         * Retrieves the HTTP header. The memory persists until node gets destroyed or reset
         *
         * @param aHeader pointer to header data.
         * @param aHeaderLen length of header data.
         * @return true if config info is provided, else false.
         */
        virtual PVMFStatus GetHTTPHeader(uint8*& aHeader, uint32& aHeaderLen) = 0;

        /**
         * Retrieves the file size. The file size is retrieved from http header
         * But server might not announce the file size info. In this case, file size
         * is not available, and set to zero.
         * @param aFileSize returned download file size.
         */
        virtual void GetFileSize(uint32& aFileSize) = 0;

        /**
         * Retrieves the host name and port number for socket node port request
         * set up by download manager node
         *
         * @param aPortConfig string of IP/DNS address + port number, like "TCP/remote_address=pvserveroha.pv.com;remote_port=554"
         * @return true if config info is provided, else false.
         * Note that this extension interface must be called AFTER data source
         * initialization interface (url is provided) gets called, otherwise it would fail
         */
        virtual bool GetSocketConfig(OSCL_String &aPortConfig) = 0;

        /**
         * Set the user agent field for a http request. Basically there are two modes, by default,
         * the new user agent field will be attached to the default user agent field (PVPlayer 4.0(Beta Release)).
         * Another one is the new user agent will replace our default one
         *
         * @param aUserAgent wide-character string of user agent
         * @param aOverwritable, flag to show whether the input user agent will replace or append the default one
         *		  aOverwritable=true => replace
         * @return true if user agent is set successfully, else false.
         */
        virtual bool SetUserAgent(OSCL_wString &aUserAgent, const bool isOverwritable = true) = 0;

        /**
         * Set the http version number (HTTP/1.0 or HTTP/1.1) for a http request.
         * @param aHttpVersion, a http version. The version is defined as an enum (see above)
         */
        virtual void SetHttpVersion(const uint32 aHttpVersion = HTTP_V11) = 0;

        /**
         * Set the NetworkTimeout for a http request/response. The unit of NetworkTimeout value is SECOND
         * @param aTimeout, timeout value in SECONDS
         */
        virtual void SetNetworkTimeout(const uint32 aTimeout) = 0;

        /**
         * Set the number of trials for redirect to avoid rediret looping. If the actual number redirect trials exceeds
         * the specified number, then there will be an error out.
         * @param aNumTrials, specified number of redirect trials
         */
        virtual void SetNumRedirectTrials(const uint32 aNumTrials) = 0;

        /**
         * Set the http extension header field. Three inputs, field key & field value plus method (field belongs to which HTTP method),
         * are needed for composing like "key: value"
         * @param aFieldKey, extension field key
         * @param aFieldValue, extension field value
         * @param aMethod, HTTP method, GET, POST or both
         * @param aPurgeOnRedirect - boolean indicating whether this header is carried over across redirects
         */
        virtual void SetHttpExtensionHeaderField(OSCL_String &aFieldKey,
                OSCL_String &aFieldValue,
                const HttpMethod aMethod = HTTP_GET,
                const bool aPurgeOnRedirect = false) = 0;

        /**
         * Set the download progress mode, i.e. the download percentage is time-based or byte-based
         * @param aMode, specified download progress mode
         */
        virtual void SetDownloadProgressMode(const DownloadProgressMode aMode = DownloadProgressMode_TimeBased) = 0;

        /**
         * Disable HTTP HEAD request
         * @param aDisableHeadRequest, flag to disabling sending HTTP HEAD request
         */
        virtual void DisableHttpHeadRequest(const bool aDisableHeadRequest = true) = 0;

};

#define PVMF_PROTOCOL_ENGINE_MSHTTP_STREAMING_EXTENSION_MIMETYPE "pvxxx/PVMFProtocolEngineNod/MSHTTPStreamingExtensionInterface"
#define KPVMFProtocolEngineNodeMSHTTPStreamingExtensionUuid PVUuid(0xe3fb7c31,0x9fb4,0x4263,0x8f,0x1f,0xa0,0xbc,0x77,0x86,0x10,0xea)

class PVMFProtocolEngineNodeMSHTTPStreamingParams
{
    public:
        PVMFProtocolEngineNodeMSHTTPStreamingParams()
        {
            reset();
        };

        ~PVMFProtocolEngineNodeMSHTTPStreamingParams()
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
            iPlayBackMode = 1;
            iStreamIDList.clear();
            iStreamPlayBackModeList.clear();
        }

        // copy constructor
        PVMFProtocolEngineNodeMSHTTPStreamingParams(const PVMFProtocolEngineNodeMSHTTPStreamingParams &x)
        {
            iStreamRate					= x.iStreamRate;
            iStreamByteOffset			= x.iStreamByteOffset;
            iStreamStartTimeInMS		= x.iStreamStartTimeInMS;
            iMaxStreamDurationInMS		= x.iMaxStreamDurationInMS;
            iStreamStartPacketNumber	= x.iStreamStartPacketNumber;
            iPacketGrouping				= x.iPacketGrouping;
            iNumStreams					= x.iNumStreams;
            iPlayBackMode               = x.iPlayBackMode;
            iStreamIDList               = x.iStreamIDList;
            iStreamPlayBackModeList     = x.iStreamPlayBackModeList;
        }

        // assignment operator
        PVMFProtocolEngineNodeMSHTTPStreamingParams& operator=(const PVMFProtocolEngineNodeMSHTTPStreamingParams& x)
        {
            iStreamRate					= x.iStreamRate;
            iStreamByteOffset			= x.iStreamByteOffset;
            iStreamStartTimeInMS		= x.iStreamStartTimeInMS;
            iMaxStreamDurationInMS		= x.iMaxStreamDurationInMS;
            iStreamStartPacketNumber	= x.iStreamStartPacketNumber;
            iPacketGrouping				= x.iPacketGrouping;
            iNumStreams					= x.iNumStreams;
            iPlayBackMode               = x.iPlayBackMode;
            iStreamIDList               = x.iStreamIDList;
            iStreamPlayBackModeList     = x.iStreamPlayBackModeList;
            return *this;
        }

        uint32 iPlayBackMode;
        uint32 iStreamRate;
        uint32 iStreamByteOffset;
        uint32 iStreamStartTimeInMS;
        uint32 iMaxStreamDurationInMS;
        uint32 iStreamStartPacketNumber;
        uint32 iPacketGrouping;
        uint32 iNumStreams;
        Oscl_Vector<uint32, OsclMemAllocator> iStreamIDList;
        Oscl_Vector<uint32, OsclMemAllocator> iStreamPlayBackModeList;
};

/**
 * PVMFProtocolEngineNodeMSHTTPStreamingExtensionInterface allows a client to exercise
 * MS HTTP Streaming extended functions of a PVMFProtocolEngineNode
 */
class PVMFProtocolEngineNodeMSHTTPStreamingExtensionInterface : public PVInterface
{
    public:

        /**
         * Retrieves the ASF header. The ASF header can be retrieved in fragments
         *
         * @param aHeader a vector of fragments
         * @param aHeaderLen length of header data.
         * @return true if the asf header is provided, else false.
         */
        virtual bool GetASFHeader(Oscl_Vector<OsclRefCounterMemFrag, OsclMemAllocator> &aHeader) = 0;

        /**
         * Sets ASF streaming params, viz rate, start time, start packet num etc
         * Used by HTTP PE Node to compose GET request
         *
         * @param aParams class containing all the params
         * @return true if params are accepted by PE Node, else false.
         */
        virtual bool SetStreamParams(const PVMFProtocolEngineNodeMSHTTPStreamingParams &aParams) = 0;

        /**
         * Retrieves the host name and port number for socket node port request
         * set up by streaming manager node
         *
         * @param aPortConfig string of IP/DNS address + port number, like "TCP/remote_address=pvserveroha.pv.com;remote_port=554"
         * @return true if config info is provided, else false.
         * Note that this extension interface must be called AFTER data source
         * initialization interface (url is provided) gets called, otherwise it would fail
         */
        virtual bool GetSocketConfig(OSCL_String &aPortConfig) = 0;

        /**
         * Retrieves the host name and port number for socket node port request, based on logging URL
         * set up by streaming manager node
         *
         * @param aPortConfig string of IP/DNS address + port number, like "TCP/remote_address=pvserveroha.pv.com;remote_port=554"
         * @return true if config info is provided, else false.
         * Note that this extension interface must be called AFTER data source
         * initialization interface (url is provided) gets called, otherwise it would fail
         */
        virtual bool GetSocketConfigForLogging(OSCL_String &aPortConfig) = 0;

        /**
         * Sends a Seek request to the server (HTTP GET) and returns seq number of
         * first data packet after seek. The API is asynchronous
         *
         * @param aNPTInMS Seek NPT in milliseconds .
         * @param aFirstSeqNumAfterSeek
         * @param aContext - Opaque data provided by caller, to be returned as part of
         * command completion.
         * @return PVMFCommandId
         */
        virtual PVMFCommandId Seek(PVMFSessionId aSessionId,
                                   uint64 aNPTInMS,
                                   uint32& aFirstSeqNumAfterSeek,
                                   OsclAny* aContext) = 0;

        /**
         * Sends a BitstreamSwitch request to the PE node and PE node will send a seek request
         * (HTTP GET) and returns seq number of first data packet after seek. The API is asynchronous
         *
         * @param aNPTInMS Seek NPT in milliseconds .
         * @param aFirstSeqNumAfterSwitch
         * @param aContext - Opaque data provided by caller, to be returned as part of
         * command completion.
         * @return PVMFCommandId
         */
        virtual PVMFCommandId BitstreamSwitch(PVMFSessionId aSessionId,
                                              uint64 aNPTInMS,
                                              uint32& aFirstSeqNumAfterSwitch,
                                              OsclAny* aContext) = 0;
        /**
         * Sets the size of the mem pool interms of number of memory buffers
         * for the media msg allocator associated with the port.
         *
         * @param aPort Port pointer .
         * @param aNumBuffersInAllocator - Number of buffers in allocator
         * @return PVMFStatus - PVMFSuccess or PVMFFailure
         */
        virtual PVMFStatus SetMediaMsgAllocatorNumBuffers(PVMFPortInterface* aPort,
                uint32 aNumBuffersInAllocator) = 0;

        /**
         * Set the user agent field for a http request. Basically there are two modes, by default,
         * the new user agent field will be attached to the default user agent field (PVPlayer 4.0(Beta Release)).
         * Another one is the new user agent will replace our default one
         *
         * @param aUserAgent wide-character string of user agent
         * @param aOverwritable, flag to show whether the input user agent will replace or append the default one
         *		  aOverwritable=true => replace
         * @return true if user agent is set successfully, else false.
         */
        virtual bool SetUserAgent(OSCL_wString &aUserAgent, const bool isOverwritable = true) = 0;

        /**
         * Set the NetworkTimeout for a http request/response. The unit of NetworkTimeout value is SECOND
         * @param aTimeout, timeout value in SECONDS
         */
        virtual void SetNetworkTimeout(const uint32 aTimeout) = 0;

        /**
         * Set the NetworkLoggingTimeout for MS streaming Logging POST request/response at stop or EOS. The unit of NetworkLoggingTimeout value is SECOND
         * @param aTimeout, timeout value in SECONDS
         */
        virtual void SetNetworkLoggingTimeout(const uint32 aTimeout) = 0;

        /**
         * Set the timeout value for Keep-Alive message. The unit of KeepAliveTimeout value is SECOND
         * @param aTimeout, timeout value in SECONDS
         */
        virtual void SetKeepAliveTimeout(const uint32 aTimeout) = 0;

        /**
         * Set the number of trials for redirect to avoid rediret looping. If the actual number redirect trials exceeds
         * the specified number, then there will be an error out.
         * @param aNumTrials, specified number of redirect trials
         */
        virtual void SetNumRedirectTrials(const uint32 aNumTrials) = 0;

        /**
         * Set the http extension header field. Three inputs, field key & field value plus method (field belongs to which HTTP method),
         * are needed for composing like "key: value"
         * @param aFieldKey, extension field key
         * @param aFieldValue, extension field value
         * @param aMethod, HTTP method, GET, POST or both
         * @param aPurgeOnRedirect - boolean indicating whether this header is carried over across redirects
         */
        virtual void SetHttpExtensionHeaderField(OSCL_String &aFieldKey,
                OSCL_String &aFieldValue,
                const HttpMethod aMethod = HTTP_GET,
                const bool aPurgeOnRedirect = false) = 0;

        /**
         * Set the logging URL
         * @param aSourceURL, a different URL for logging statistics
         */
        virtual void SetLoggingURL(OSCL_wString& aSourceURL) = 0;

        /**
         * Set the proxy URL and the port
         * @param aProxyURL, a proxy URL for streaming
         * @param aProxyPort, proxy number
         * @return true if proxy is set successfully, else false.
         */
        virtual bool SetStreamingProxy(OSCL_wString& aProxyURL, const uint32 aProxyPort) = 0;

        /**
         * Set the maximum ASF header size
         * @param aMaxASFHeaderSize, upper limit for ASF header supported
         */
        virtual void SetMaxASFHeaderSize(const uint32 aMaxASFHeaderSize = PVPROTOCOLENGINE_DEFAULT_MAXIMUM_ASF_HEADER_SIZE) = 0;

        /**
         * Set user authentication information, such as userID and password
         * @param aUserID, user id
         * @param aPasswd, password
         * @param aMaxASFHeaderSize, upper limit for ASF header supported
         */
        virtual void SetUserAuthInfo(OSCL_String &aUserID, OSCL_String &aPasswd) = 0;

        /**
         * Check the WM server 4.1 or not
         * @return true if the streaming server is wms4.1 else false.
         */
        virtual bool IsWMServerVersion4() = 0;

        /**
         * Set the accel bitrate for fast cache.
         * @param aAccelBitrate, specified accel bitrate
         */
        virtual void SetAccelBitrate(const uint32 aAccelBitrate) = 0;

        /**
         * Set the accel duration for fast cache.
         * @param aAccelDuration, specified accel duration
         */
        virtual void SetAccelDuration(const uint32 aAccelDuration) = 0;

        /**
         * Set number of buffers to be allocated for media data pool
         * @param aVal, the number calculated by SM node
         * @return void
         */
        virtual void SetNumBuffersInMediaDataPoolSMCalc(uint32 aVal) = 0;

        /**
         * Set the max http stremaing size.
         * @param aMaxHttpStreamingSize
         */
        virtual void SetMaxHttpStreamingSize(const uint32 aMaxHttpStreamingSize) = 0;

};

#endif // PVMFPROTOCOLENGINENODE_EXTENSION_H_INCLUDED


