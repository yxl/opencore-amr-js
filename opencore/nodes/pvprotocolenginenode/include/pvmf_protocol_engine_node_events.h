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
 * @file pvmf_protocolengine_node_events.h
 * @brief
 */

#ifndef PVMF_PROTOCOL_ENGINE_NODE_EVENTS_H_INCLUDED
#define PVMF_PROTOCOL_ENGINE_NODE_EVENTS_H_INCLUDED

#ifndef PVMF_BASIC_ERRORINFOMESSAGE_H_INCLUDED
#include "pvmf_basic_errorinfomessage.h"
#endif

/**
 UUID for Protocol Engine Node info events.  These events use the basic event type
 and the event codes in PVProtocolEngineNodeInfoEventTypes
 **/
#define PVMFPROTOCOLENGINENODEInfoEventTypesUUID PVUuid(0xaae693a0,0xac71,0x11d9,0x96,0x69,0x08,0x00,0x20,0x0c,0x9a,0x66)

/**
 Event codes for FTDM Node Info events.
**/
enum PVMFPROTOCOLENGINENODEInfoEventTypes
{
    PVMFPROTOCOLENGINE_INFO_MovieAtomCompleted = PVMF_NODE_INFO_EVENT_LAST + 1,	// 4096+1=4097

    PVMFPROTOCOLENGINENODEInfo_First, //placeholder							// 4098

    //This event code is used to report download progress.
    //It is reported with the PVMFInfoBufferingStatus event type
    //as a node information event.  The percentage complete
    //will be in the Local Buffer in the first 32 bits
    PVMFPROTOCOLENGINENODEInfo_BufferingStatus,								// 4099

    PVMFPROTOCOLENGINENODEInfo_TruncatedContentByServerDisconnect,			// 4100


    // redirect status code
    PVMFPROTOCOLENGINENODEInfo_Redirect,									// 4101
    //"300"      ; Multiple Choices
    PVMFPROTOCOLENGINENODEInfo_HTTPRedirectCode300 = PVMFPROTOCOLENGINENODEInfo_Redirect + 300, // 4401
    //"301"      ; Moved Permanently
    PVMFPROTOCOLENGINENODEInfo_HTTPRedirectCode301,	// 4402
    //"302"      ; Found
    PVMFPROTOCOLENGINENODEInfo_HTTPRedirectCode302,	// 4403
    //"303"      ; See Other
    PVMFPROTOCOLENGINENODEInfo_HTTPRedirectCode303,	// 4404
    //"304"      ; Not Modified
    PVMFPROTOCOLENGINENODEInfo_HTTPRedirectCode304,	// 4405
    //"305"      ; Use Proxy
    PVMFPROTOCOLENGINENODEInfo_HTTPRedirectCode305,	// 4406
    // "306"	  ; Uused
    PVMFPROTOCOLENGINENODEInfo_HTTPRedirectCode306,	// 4407
    // "307"      ; Use Temporary Redirect
    PVMFPROTOCOLENGINENODEInfo_HTTPRedirectCode307,	// 4408
    // unknown re-direct code
    PVMFPROTOCOLENGINENODEInfo_HTTPRedirectCode3xxUnknown, // 4409

    PVMFPROTOCOLENGINENODEInfo_Last //placeholder

};

/**
 UUID for FTDM Node error events.  These events use the type PVMFPROTOCOLENGINENODEErrorEvent
 defined below and the event codes in PVMFPROTOCOLENGINENODEErrorEventTypes
 **/
#define PVProtocolEngineNodeErrorEventTypesUUID PVUuid(0x0a9ad2b0,0xaaed,0x11d9,0x96,0x69,0x08,0x00,0x20,0x0c,0x9a,0x66)


/**
  Error codes used in Protocol Engine node error event messages
 **/
enum PVProtocolEngineNodeErrorEventType
{
    PVProtocolEngineNodeErrorEventStart = 1024,

    // some processing errors, especially for pending commands with failure, within the range [200, 299], which is not HTTP status error code range
    PVProtocolEngineNodeErrorNotHTTPErrorStart									 = PVProtocolEngineNodeErrorEventStart + 200,
    PVProtocolEngineNodeErrorProcessingFailure									 = PVProtocolEngineNodeErrorNotHTTPErrorStart + 90, // 1314
    PVProtocolEngineNodeErrorProcessingFailure_MaximumSizeLimitation			 = PVProtocolEngineNodeErrorProcessingFailure - 19, // 1295
    PVProtocolEngineNodeErrorProcessingFailure_ContentLengthNotMatch			 = PVProtocolEngineNodeErrorProcessingFailure - 16, // 1298
    PVProtocolEngineNodeErrorProcessingFailure_TimeoutServerInactivity		     = PVProtocolEngineNodeErrorProcessingFailure - 15, // 1299
    PVProtocolEngineNodeErrorProcessingFailure_TimeoutServerNoResponce			 = PVProtocolEngineNodeErrorProcessingFailure - 14, // 1300
    PVProtocolEngineNodeErrorProcessingFailure_ChunkedTransferEncodingNotSupport = PVProtocolEngineNodeErrorProcessingFailure - 13, // 1301
    PVProtocolEngineNodeErrorProcessingFailure_AsfHeaderSizeExceedLimit			 = PVProtocolEngineNodeErrorProcessingFailure - 12, // 1302
    PVProtocolEngineNodeErrorProcessingFailure_HttpVersionNotSupported			 = PVProtocolEngineNodeErrorProcessingFailure - 11,	// 1303
    PVProtocolEngineNodeErrorProcessingFailure_MemoryAllocationFailure			 = PVProtocolEngineNodeErrorProcessingFailure - 10,	// 1304
    PVProtocolEngineNodeErrorProcessingFailure_OutputToDataStreamFailure		 = PVProtocolEngineNodeErrorProcessingFailure - 9,	// 1305
    PVProtocolEngineNodeErrorProcessingFailure_DataStreamOpenFailure			 = PVProtocolEngineNodeErrorProcessingFailure - 8,	// 1306
    PVProtocolEngineNodeErrorProcessingFailure_ParseHttpResponseFailure			 = PVProtocolEngineNodeErrorProcessingFailure - 7,	// 1307
    PVProtocolEngineNodeErrorProcessingFailure_ComposeHttpRequestFailure		 = PVProtocolEngineNodeErrorProcessingFailure - 6,	// 1308
    PVProtocolEngineNodeErrorProcessingFailure_ComposeHttpRequestBufferSizeNotMatchRequestSize = PVProtocolEngineNodeErrorProcessingFailure - 5, // 1309
    PVProtocolEngineNodeErrorProcessingFailure_MediaDataCreateFailure			 = PVProtocolEngineNodeErrorProcessingFailure - 4,	// 1310
    PVProtocolEngineNodeErrorProcessingFailure_BadUrl							 = PVProtocolEngineNodeErrorProcessingFailure - 3,	// 1311
    PVProtocolEngineNodeErrorProcessingFailure_InputOutputNotReady				 = PVProtocolEngineNodeErrorProcessingFailure - 2,	// 1312
    PVProtocolEngineNodeErrorProcessingFailure_GeneralError						 = PVProtocolEngineNodeErrorProcessingFailure - 1,	// 1313


    PVProtocolEngineNodeErrorHTTPRedirect_NotValidUrl						= PVProtocolEngineNodeErrorProcessingFailure + 1,	// 1315
    PVProtocolEngineNodeErrorHTTPRedirect_TrialsExceedLimit,																// 1316
    PVProtocolEngineNodeErrorHTTPErrorCode401_InvalidRealm,																	// 1317
    PVProtocolEngineNodeErrorHTTPErrorCode401_UnsupportedAuthenticationType,												// 1318
    PVProtocolEngineNodeErrorHTTPErrorCode401_NoAuthenticationHeader,														// 1319


    PVProtocolEngineNodeErrorNotHTTPErrorEnd								= PVProtocolEngineNodeErrorEventStart + 299,

    // 1xx code unsupported in Http/1.0
    PVProtocolEngineNodeErrorHTTPInfoCodeStart = PVProtocolEngineNodeErrorEventStart + 100, // 1124
    PVProtocolEngineNodeErrorHTTPInfoCode100_UnsupportInHttp10 = PVProtocolEngineNodeErrorHTTPInfoCodeStart, // 1124
    PVProtocolEngineNodeErrorHTTPInfoCode101_UnsupportInHttp10,	// 1125
    PVProtocolEngineNodeErrorHTTPInfoCodeEnd = PVProtocolEngineNodeErrorEventStart + 199,

    // 2xx code error with zero content length
    PVProtocolEngineNodeErrorHTTP2xxCodeStart = PVProtocolEngineNodeErrorEventStart + 200, // 1224
    PVProtocolEngineNodeErrorHTTP200_ZeroContentLength = PVProtocolEngineNodeErrorHTTP2xxCodeStart, // 1224
    PVProtocolEngineNodeErrorHTTP201_ZeroContentLength, // 1225
    PVProtocolEngineNodeErrorHTTP202_ZeroContentLength, // 1226
    PVProtocolEngineNodeErrorHTTP203_ZeroContentLength, // 1227
    PVProtocolEngineNodeErrorHTTP204_NoContent,			// 1228
    PVProtocolEngineNodeErrorHTTP205_ZeroContentLength, // 1229
    PVProtocolEngineNodeErrorHTTP206_ZeroContentLength, // 1230
    PVProtocolEngineNodeErrorHTTP2xxCodeEnd,			// 1231

    // redirect errors
    PVProtocolEngineNodeErrorHTTPRedirectCodeStart = PVProtocolEngineNodeErrorEventStart + 300,
    PVProtocolEngineNodeErrorHTTPRedirectCode300_InvalidUrl = PVProtocolEngineNodeErrorHTTPRedirectCodeStart,	// 1324
    //"301"      ; Moved Permanently
    PVProtocolEngineNodeErrorHTTPRedirectCode301_InvalidUrl,	// 1325
    //"302"      ; Found
    PVProtocolEngineNodeErrorHTTPRedirectCode302_InvalidUrl,	// 1326
    //"303"      ; See Other
    PVProtocolEngineNodeErrorHTTPRedirectCode303_InvalidUrl,	// 1327
    //"304"      ; Not Modified
    PVProtocolEngineNodeErrorHTTPRedirectCode304_InvalidUrl,	// 1328
    //"305"      ; Use Proxy
    PVProtocolEngineNodeErrorHTTPRedirectCode305_InvalidUrl,	// 1329
    // "306"	  ; Uused
    PVProtocolEngineNodeErrorHTTPRedirectCode306_InvalidUrl,	// 1330
    // "307"      ; Use Temporary Redirect
    PVProtocolEngineNodeErrorHTTPRedirectCode307_InvalidUrl,	// 1331

    PVProtocolEngineNodeErrorHTTPRedirectCodeEnd = PVProtocolEngineNodeErrorEventStart + 399,

    //"400"      ; Bad Request
    PVProtocolEngineNodeErrorHTTPErrorCode400 = PVProtocolEngineNodeErrorEventStart + 400, // 1424
    //"401"      ; Unauthorized
    PVProtocolEngineNodeErrorHTTPErrorCode401,	// 1425
    //"402"      ; Payment Required
    PVProtocolEngineNodeErrorHTTPErrorCode402,	// 1426
    //"403"      ; Forbidden
    PVProtocolEngineNodeErrorHTTPErrorCode403,	// 1427
    //"404"      ; Not Found
    PVProtocolEngineNodeErrorHTTPErrorCode404,	// 1428
    //"405"      ; Method Not Allowed
    PVProtocolEngineNodeErrorHTTPErrorCode405,	// 1429
    //"406"      ; Not Acceptable
    PVProtocolEngineNodeErrorHTTPErrorCode406,	// 1430
    //"407"      ; Proxy Authentication Required
    PVProtocolEngineNodeErrorHTTPErrorCode407,	// 1431
    //"408"      ; Request Time-out
    PVProtocolEngineNodeErrorHTTPErrorCode408,	// 1432
    //"409"		 ; Conflict
    PVProtocolEngineNodeErrorHTTPErrorCode409,	// 1433
    //"410"      ; Gone
    PVProtocolEngineNodeErrorHTTPErrorCode410,	// 1434
    //"411"      ; Length Required
    PVProtocolEngineNodeErrorHTTPErrorCode411,	// 1435
    //"412"      ; Precondition Failed
    PVProtocolEngineNodeErrorHTTPErrorCode412,	// 1436
    //"413"      ; Request Entity Too Large
    PVProtocolEngineNodeErrorHTTPErrorCode413,	// 1437
    //"414"      ; Request-URI Too Large
    PVProtocolEngineNodeErrorHTTPErrorCode414,	// 1438
    //"415"      ; Unsupported Media Type
    PVProtocolEngineNodeErrorHTTPErrorCode415,	// 1439
    //"416"      ; Requested range not satisfiable
    PVProtocolEngineNodeErrorHTTPErrorCode416,	// 1440
    //"417"      ; Expectation Failed
    PVProtocolEngineNodeErrorHTTPErrorCode417,	// 1441

    // Unknown code
    PVProtocolEngineNodeErrorHTTPCode4xxUnknown = PVProtocolEngineNodeErrorEventStart + 499,

    //"500"      ; Internal Server Error
    PVProtocolEngineNodeErrorHTTPErrorCode500 = PVProtocolEngineNodeErrorEventStart + 500, // 1524
    //"501"      ; Not Implemented
    PVProtocolEngineNodeErrorHTTPErrorCode501,	// 1525
    //"502"      ; Bad Gateway
    PVProtocolEngineNodeErrorHTTPErrorCode502,	// 1526
    //"503"      ; Service Unavailable
    PVProtocolEngineNodeErrorHTTPErrorCode503,	// 1527
    //"504"      ; Gateway Time-out
    PVProtocolEngineNodeErrorHTTPErrorCode504,	// 1528
    //"505"      ; HTTP Version not supported
    PVProtocolEngineNodeErrorHTTPErrorCode505,	// 1529

    PVProtocolEngineNodeErrorHTTPCode5xxUnknownStart,

    // general Unknown code
    PVProtocolEngineNodeErrorHTTPCodeUnknown,
    PVProtocolEngineNodeErrorEventEnd
};

#endif // PVMF_PROTOCOL_ENGINE_NODE_EVENTS_H_INCLUDED

