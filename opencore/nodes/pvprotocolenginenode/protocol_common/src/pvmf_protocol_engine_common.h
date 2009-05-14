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
#ifndef PVMF_PROTOCOLENGINE_COMMON_H_INCLUDED
#define PVMF_PROTOCOLENGINE_COMMON_H_INCLUDED

#include "pvmf_protocol_engine_internal.h"

#define DATAPATHLOGGER_TAG "protocolenginenode.protocolengine"
#define DATAPATHERRLOGGER_TAG "datapath.sourcenode.protocolenginenode"

class UserCommands
{
    public:
        virtual ~UserCommands() {}

        // aSeekPosition can be time-based (in MS HTTP streaming) or byte-based (in progressive streaming
        virtual void seek(const uint32 aSeekPosition) = 0;
        virtual void stop(const bool isAfterEOS = false) = 0;
        virtual void pause(const bool isFirstCallInPause = true) = 0;
        virtual void resume() = 0;
        virtual void sendRequest() = 0;
        virtual void gotoNextState() = 0;
        virtual void bitstreamSwitch() = 0;
};

// Any http-based protocol(progressive download, fasttrack, ms http streaming and real http cloaking)
// can be viewed as a http request-response sequence, which can be addressed by GoF state pattern, i.e. ProtocolState class
// And protocol variances can be abstracted and hidden by using a common interface class, HttpBasedProtocol
// HttpBasedProtocol serves as the context of state class, and also provides the APIs for protocol user.

enum ProtocolEngineOutputDataType
{
    ProtocolEngineOutputDataType_HttpHeader = 0,
    ProtocolEngineOutputDataType_FirstDataPacket,
    ProtocolEngineOutputDataType_NormalData
};

struct ProtocolEngineOutputDataSideInfo
{
    ProtocolEngineOutputDataType iDataType;
    // for OutputDataType_FirstDataPacket,	iData = iFirstPacketNumber
    // for OutputDataType_NormalData,		iData = iCurrentDataStreamOffset (for fasttrack) / iCurrPacketNum (for http streaming)
    OsclAny *iData;

    // constructors
    ProtocolEngineOutputDataSideInfo() : iDataType(ProtocolEngineOutputDataType_HttpHeader), iData(0)
    {
        ;
    }
    ProtocolEngineOutputDataSideInfo(const ProtocolEngineOutputDataType aType, OsclAny *aData) :
            iDataType(aType), iData(aData)
    {
        ;
    }

    void set(const ProtocolEngineOutputDataType aDataType, const OsclAny *aData = 0)
    {
        iDataType = (ProtocolEngineOutputDataType)aDataType;
        iData = (OsclAny *)aData;
    }
};

struct ProtocolEngineOutputDataSideInfoForFasttrack
{
    uint32 iCurrDataStreamOffset;
    uint32 iCurrPlaybackTime;

    ProtocolEngineOutputDataSideInfoForFasttrack() :
            iCurrDataStreamOffset(0), iCurrPlaybackTime(0) {}
};


enum ProtocolRequestType
{
    ProtocolRequestType_Normaldata = 0,
    ProtocolRequestType_Logging
};

struct ProtocolStateCompleteInfo
{
    bool isDownloadStreamingDone;  // true => current state complete means download or streaming is done/complete
    bool isWholeSessionDone;		 // true => current state is the last state of the state transition table
    bool isEOSAchieved;				 // true => EOS packet is received in streaming, or download reaches EOS (content-length, server discconnect or maximum file size)
    // for protocol engine side, isDownloadStreamingDone=true <=> isEOSAchieved=true, but node will use this structure for other
    // purposes, e.g. this flag can be used to differentiate stop case and true EOS case
    // constructors
    ProtocolStateCompleteInfo()
    {
        clear();
    }
    ProtocolStateCompleteInfo(const ProtocolStateCompleteInfo &x)
    {
        isDownloadStreamingDone = x.isDownloadStreamingDone;
        isWholeSessionDone		= x.isWholeSessionDone;
        isEOSAchieved			= x.isEOSAchieved;
    }
    ProtocolStateCompleteInfo(const bool aDownloadStreamingDone, const bool aSessionDone, const bool aEOSAchieved) :
            isDownloadStreamingDone(aDownloadStreamingDone),
            isWholeSessionDone(aSessionDone),
            isEOSAchieved(aEOSAchieved)
    {
        ;
    }

    // assignment operator
    ProtocolStateCompleteInfo& operator=(const ProtocolStateCompleteInfo& x)
    {
        isDownloadStreamingDone = x.isDownloadStreamingDone;
        isWholeSessionDone		= x.isWholeSessionDone;
        isEOSAchieved			= x.isEOSAchieved;
        return *this;
    }

    // clear
    void clear()
    {
        isDownloadStreamingDone = false;
        isWholeSessionDone		= false;
        isEOSAchieved			= false;
    }
};


// This observer class is designed to notify state user (specifically, protocol) when one protocol state is completely finished, i.e.
// one http request-response is completely done or parsing response is completely done. Then user may change to next protocol state
class ProtocolStateObserver
{
    public:
        virtual ~ProtocolStateObserver() {}

        virtual void ProtocolStateComplete(const ProtocolStateCompleteInfo &aInfo) = 0;
        virtual void OutputDataAvailable(OUTPUT_DATA_QUEUE &aOutputQueue, ProtocolEngineOutputDataSideInfo& aSideInfo) = 0;
        virtual void ProtocolStateError(int32 aErrorCode) = 0; // server response error or other internal fatal error
        virtual bool GetBufferForRequest(PVMFSharedMediaDataPtr &aMediaData) = 0; // to contruct HTTP request
        virtual void ProtocolRequestAvailable(uint32 aRequestType = ProtocolRequestType_Normaldata) = 0; // need to send to port
};

// This class is based on state pattern, to encapsulate all state specific behavior.
class ProtocolState : public HttpParsingBasicObjectObserver,
            public UserCommands
{
    public:
        // has base implementation, basically create a templete
        OSCL_IMPORT_REF virtual int32 processMicroState(INPUT_DATA_QUEUE &aDataQueue);

        // protocol objects own these objects, observer, composer and parser
        // need to pass these objects down to state objects
        void setObserver(ProtocolStateObserver *aObserver)
        {
            iObserver = aObserver;
        }
        void setComposer(HTTPComposer *aComposer)
        {
            iComposer = aComposer;
        }
        void setParser(HttpParsingBasicObject *aParser)
        {
            iParser = aParser;
        }

        // set functions, will be delegated to ProtocolState to handle
        // set config info for composing request
        void setURI(const INetURI &aUri)
        {
            iURI = aUri;
        }
        virtual void setLoggingURI(const INetURI &aUri)
        {
            OSCL_UNUSED_ARG(aUri);
        }
        virtual void setConfigInfo(OsclAny* aConfigInfo) = 0;

        // get functions to expose the information that node needs
        // The header could be http header, sdp or asf header
        OSCL_IMPORT_REF virtual bool getHeader(Oscl_Vector<OsclRefCounterMemFrag, OsclMemAllocator> &aHeader) = 0;
        virtual uint32 getContentLength()
        {
            return (iParser == NULL ? 0 : iParser->getContentLength());
        }
        virtual uint32 getDownloadSize()
        {
            return (iParser == NULL ? 0 : iParser->getDownloadSize());
        }
        virtual uint32 getRemainingSize()
        {
            if (iParser == NULL || iParser->getContentLength() == 0) return 0;
            return iParser->getContentLength() - iParser->getDownloadSize();
        }
        OSCL_IMPORT_REF virtual uint32 getDownloadRate();
        OSCL_IMPORT_REF uint32 getDownloadTimeForEstimation();
        uint32 getResponseStatusCode()
        {
            return (iParser == NULL ? 0 : iParser->getStatusCode());
        }
        bool getRedirectURI(OSCL_String &aRedirectUri)
        {
            return iParser->getRedirectURI(aRedirectUri);
        }
        bool getContentType(OSCL_String &aContentType)
        {
            return iParser->getContentType(aContentType);
        }
        bool getAuthenInfo(OSCL_String &aRealm)
        {
            return iParser->getAuthenInfo(aRealm);
        }
        bool isServerSupportBasicAuthentication()
        {
            return iParser->isServerSupportBasicAuthentication();
        }
        bool isServerSendAuthenticationHeader()
        {
            return iParser->isServerSendAuthenticationHeader();
        }
        void getBasicPtr(const StrPtrLen aAuthenValue, uint32 &length)
        {
            iParser->getBasicPtr(aAuthenValue, length);
        }
        void getRealmPtr(const char *&ptrRealm, uint32 &len, uint32 &length)
        {
            iParser->getRealmPtr(ptrRealm, len, length);
        }
        virtual uint32 getCurrentPlaybackTime()
        {
            return 0;    // only used in fast track
        }
        virtual uint32 getTimeoutInMs()
        {
            return 0;    // ms http streaming only
        }
        virtual uint32 getServerVersionNumber()
        {
            return (iParser == NULL ? 0 : iParser->getServerVersionNumber());
        }
        virtual void prepare()
        {
            ;    // prepare for the new state, especially store data from the previous state, for the current state
        }
        bool isSendingNewRequest()
        {
            return (iProcessingState == EHttpProcessingMicroState_SendRequest);
        }
        virtual bool isCurrentStateOptional()
        {
            return false;    // optional state can be by-passed regardless of any error happened
        }
        virtual void setLastState()
        {
            ;
        }
        virtual uint32 getMediaDataLength()
        {
            return 0;
        }
        virtual uint32 getContenBitrate()
        {
            return 0;
        }

        // user commands
        virtual void seek(const uint32 aSeekPosition)
        {
            OSCL_UNUSED_ARG(aSeekPosition);    // only used in ms http streaming for now
        }
        virtual void stop(const bool isAfterEOS = false)
        {
            OSCL_UNUSED_ARG(isAfterEOS);    // only used in ms http streaming for now
        }
        virtual void pause(const bool isFirstCallInPause = true)
        {
            OSCL_UNUSED_ARG(isFirstCallInPause);    // only used in ms http streaming for now
        }
        virtual void resume()
        {
            ;    // only used in ms http streaming for now
        }
        virtual void sendRequest()
        {
            iProcessingState = EHttpProcessingMicroState_SendRequest;
        }
        virtual void bitstreamSwitch()
        {
            ;
        }
        void gotoNextState()
        {
            ;
        }

        // constructor
        ProtocolState() : iComposer(NULL),
                iParser(NULL),
                iProcessingState(EHttpProcessingMicroState_SendRequest),
                iObserver(NULL),
                iNeedGetResponsePreCheck(true)
        {
            iDataPathLogger = PVLogger::GetLoggerObject("datapath.sourcenode.protocolenginenode");
        }

        virtual ~ProtocolState()
        {
            iComposer = NULL;
            iParser   = NULL;
            iObserver = NULL;
            iDataPathLogger = NULL;
        };

        virtual void reset()
        {
            if (iComposer) iComposer->reset();
            if (iParser) iParser->reset();
            iNeedGetResponsePreCheck = true;
        }

    protected:
        // From HttpParsingBasicObjectObserver
        virtual int32 OutputDataAvailable(OUTPUT_DATA_QUEUE *aOutputQueue, const bool isHttpHeader)
        {
            OSCL_UNUSED_ARG(aOutputQueue);
            OSCL_UNUSED_ARG(isHttpHeader);
            return PROCESS_SUCCESS;
        }

        /////////////////////////////////////////////////////////////////////////////
        /////// Following APIs are related composing and sending http request ///////
        /////////////////////////////////////////////////////////////////////////////
        // check all the info is ready for composing and sending a request
        OSCL_IMPORT_REF virtual int32 processMicroStateSendRequestPreCheck();
        OSCL_IMPORT_REF virtual int32 processMicroStateSendRequest();
        int32 composeRequest(OsclMemoryFragment &aFrag);
        // By default HTTP GET method, derived class may need to override this one
        virtual void setRequestBasics() = 0;
        // Each derived class needs to implement this one
        virtual bool setHeaderFields() = 0;
        // do final compose, fixed for all derived classes
        OSCL_IMPORT_REF virtual int32 doCompose(OsclMemoryFragment &aFrag);
        OSCL_IMPORT_REF bool setExtensionFields(Oscl_Vector<OSCL_HeapString<OsclMemAllocator>, OsclMemAllocator> &aExtensionHeaderKeys,
                                                Oscl_Vector<OSCL_HeapString<OsclMemAllocator>, OsclMemAllocator> &aExtensionHeaderValues,
                                                Oscl_Vector<uint32, OsclMemAllocator> &aMaskBitForHTTPMethod,
                                                Oscl_Vector<bool, OsclMemAllocator> &aExtensionHeadersPurgeOnRedirect,
                                                const HTTPMethod aMethod = HTTP_METHOD_GET);
        virtual bool getProtocolRequestType()
        {
            return (uint32)ProtocolRequestType_Normaldata;
        }

        // HTTP basic/digest authentication (RFC 2617)
        OSCL_IMPORT_REF bool constructAuthenHeader(OSCL_String &aUserID, OSCL_String &aPasswd);

        /////////////////////////////////////////////////////////////////////////////
        /////// Following APIs are related parsing http response ////////////////////
        /////////////////////////////////////////////////////////////////////////////
        // check all the info is ready for parsing a new response
        OSCL_IMPORT_REF virtual int32 processMicroStateGetResponsePreCheck();
        OSCL_IMPORT_REF virtual int32 processMicroStateGetResponse(INPUT_DATA_QUEUE &aDataQueue);
        // shared routine for all the download protocols
        OSCL_IMPORT_REF virtual int32 checkParsingStatus(int32 parsingStatus);

        virtual bool isDownloadStreamingDoneState()
        {
            return false;
        }
        virtual bool isLastState()
        {
            return false;
        }

    private:

        // factor processMicroState() into the following methods to prevent processMicroState() getting bloated
        int32 doProcessMicroStateSendRequestPreCheck();
        int32 doProcessMicroStateSendRequest();
        int32 doProcessMicroStateGetResponsePreCheck();
        int32 doProcessMicroStateGetResponse(INPUT_DATA_QUEUE &aDataQueue);

        // support setExtensionFields()
        uint32 getBitMaskForHttpMethod(Oscl_Vector<uint32, OsclMemAllocator> &aMaskBitForHTTPMethod,
                                       const HTTPMethod aMethod);
        // called by checkParsingStatus()
        int32 handleParsingSyntaxError();

        // called by constructAuthenHeader()
        int32 base64enc(char *data, char *out);

    protected:
        // http composer and parser should be life-time long, shouldn't be affected by state transition.
        // So protocol object owns these two objects.
        HTTPComposer *iComposer;
        HttpParsingBasicObject *iParser; // wrap http parser to do parsing for each input media data
        pvHttpProcessingMicroState iProcessingState;

        ProtocolStateObserver *iObserver;
        INetURI iURI; // wrapper for url parsing
        TimeValue iStartTime;
        bool iNeedGetResponsePreCheck;
        ProtocolEngineOutputDataSideInfo iDataSideInfo;
        PVLogger *iDataPathLogger;
};

// This observer class is designed to notify protocol user (specifically, node) when one protocol state is completely finished, i.e.
// one http request-response or parsing response is completely done. Then user may change to next protocol state
class ProtocolObserver
{
    public:
        virtual ~ProtocolObserver() {}

        virtual void ProtocolStateComplete(const ProtocolStateCompleteInfo &aInfo) = 0;
        virtual void OutputDataAvailable(OUTPUT_DATA_QUEUE &aOutputQueue, ProtocolEngineOutputDataSideInfo &aSideInfo) = 0;
        virtual void ProtocolStateError(int32 aErrorCode) = 0; // server response error or other internal fatal error
        virtual bool GetBufferForRequest(PVMFSharedMediaDataPtr &aMediaData) = 0; // to contruct HTTP request
        virtual void ProtocolRequestAvailable(uint32 aRequestType = ProtocolRequestType_Normaldata) = 0; // need to send to port
};

// Any http-based protocol(progressive download, fasttrack, ms http streaming and real http cloaking)
// can be viewed as a http request-response sequence, which can be addressed by GoF state pattern
class HttpBasedProtocol : public ProtocolStateObserver,
            public UserCommands
{
    public:
        // each http based protocol must implment this interface
        virtual int32 runStateMachine(INPUT_DATA_QUEUE &aDataQueue)
        {
            return iCurrState->processMicroState(aDataQueue);
        }

        // From ProtocolStateObserver
        virtual void ProtocolStateComplete(const ProtocolStateCompleteInfo &aInfo)
        {
            // change to the next protocol state and notify the user that data processing at the current state is completely done
            if (iObserver) iObserver->ProtocolStateComplete(aInfo);
            //if(isSuccess) iCurrState = getNextState();
        }
        virtual void OutputDataAvailable(OUTPUT_DATA_QUEUE &aOutputQueue, ProtocolEngineOutputDataSideInfo &aSideInfo)
        {
            if (iObserver) iObserver->OutputDataAvailable(aOutputQueue, aSideInfo);
        }
        virtual void ProtocolStateError(int32 aErrorCode)
        {
            if (iObserver) iObserver->ProtocolStateError(aErrorCode);
        }

        virtual bool GetBufferForRequest(PVMFSharedMediaDataPtr &aMediaData)
        {
            return iObserver->GetBufferForRequest(aMediaData);
        }

        virtual void ProtocolRequestAvailable(uint32 aRequestType = ProtocolRequestType_Normaldata)
        {
            if (iObserver) iObserver->ProtocolRequestAvailable(aRequestType);
        }

        // initialize means passing protocol owned objects down to state objects
        virtual void initialize() = 0;

        // user commands
        void stop(const bool isAfterEOS = false)
        {
            iCurrState->stop(isAfterEOS);
        }
        virtual void seek(const uint32 aSeekPosition)
        {
            iCurrState->seek(aSeekPosition);
        }
        virtual void pause(const bool isFirstCallInPause = true)
        {
            iCurrState->pause(isFirstCallInPause);
        }
        virtual void resume()
        {
            iCurrState->resume();
        }
        virtual void bitstreamSwitch()
        {
            iCurrState->bitstreamSwitch();
        }
        void sendRequest()
        {
            iCurrState->sendRequest();
        }
        void gotoNextState()
        {
            iCurrState = getNextState();
            iCurrState->prepare();
            iCurrState->reset();
            iCurrState->sendRequest();
        }


        // set protocol observer for protocol user
        void setObserver(ProtocolObserver *aObserver)
        {
            iObserver = aObserver;
        }

        // set functions, will be delegated to ProtocolState to handle
        void setURI(const INetURI &aUri)
        {
            iCurrState->setURI(aUri);
            ProtocolState *state = NULL;
            while ((state = getNextState()) != iCurrState) state->setURI(aUri); // set uri for all states
        }
        void setLoggingURI(const INetURI &aUri)
        {
            iCurrState->setLoggingURI(aUri);
            ProtocolState *state = NULL;
            while ((state = getNextState()) != iCurrState) state->setLoggingURI(aUri); // set uri for all states
        }
        virtual void setConfigInfo(OsclAny* aConfigInfo)
        {
            iCurrState->setConfigInfo(aConfigInfo);
            ProtocolState *state = NULL;
            while ((state = getNextState()) != iCurrState) state->setConfigInfo(aConfigInfo); // set config info for all states
        }

        // get functions to expose the information that node needs
        // The header could be http header, sdp or asf header
        bool getHeader(Oscl_Vector<OsclRefCounterMemFrag, OsclMemAllocator> &aHeader)
        {
            return iCurrState->getHeader(aHeader);
        }
        uint32 getContentLength()
        {
            return iCurrState->getContentLength();
        }
        uint32 getDownloadSize()
        {
            return iCurrState->getDownloadSize();
        }
        uint32 getRemainingSize()
        {
            return iCurrState->getRemainingSize();
        }
        uint32 getDownloadRate()
        {
            return iCurrState->getDownloadRate();
        }
        uint32 getDownloadTimeForEstimation()
        {
            return iCurrState->getDownloadTimeForEstimation();
        }
        uint32 getResponseStatusCode()
        {
            return iCurrState->getResponseStatusCode();
        }
        bool getRedirectURI(OSCL_String &aRedirectUri)
        {
            return iCurrState->getRedirectURI(aRedirectUri);
        }
        bool getContentType(OSCL_String &aContentType)
        {
            return iCurrState->getContentType(aContentType);
        }
        bool getAuthenInfo(OSCL_String &aRealm)
        {
            return iCurrState->getAuthenInfo(aRealm);
        }
        bool isServerSupportBasicAuthentication()
        {
            return iParser->isServerSupportBasicAuthentication();
        }
        bool isServerSendAuthenticationHeader()
        {
            return iParser->isServerSendAuthenticationHeader();
        }
        void getBasicPtr(const StrPtrLen aAuthenValue, uint32 &length)
        {
            iParser->getBasicPtr(aAuthenValue, length);
        }
        void getRealmPtr(const char *&ptrRealm, uint32 &len, uint32 &length)
        {
            iParser->getRealmPtr(ptrRealm, len, length);
        }
        uint32 getCurrentPlaybackTime()
        {
            return iCurrState->getCurrentPlaybackTime();    // only used in fast track
        }
        uint32 getTimeoutInMs()
        {
            return iCurrState->getTimeoutInMs();    // only used in ms http streaming
        }
        uint32 getServerVersionNum()
        {
            return iCurrState->getServerVersionNumber();
        }
        bool isSendingNewRequest()
        {
            return iCurrState->isSendingNewRequest();
        }
        bool isCurrentStateOptional()
        {
            return iCurrState->isCurrentStateOptional();    // optional state can be by-passed regardless of any error happened
        }
        uint32 getMediaDataLength()
        {
            return iCurrState->getMediaDataLength();    // only used in Shoutcast streaming
        }
        uint32 getContenBitrate()
        {
            return iCurrState->getContenBitrate();    // only used in Shoutcast streaming
        }

        void resetTotalHttpStreamingSize()
        {
            if (iParser) iParser->resetTotalHttpStreamingSize();
        }

        virtual void reset()
        {
            if (iParser) iParser->resetForBadConnectionDetection();
            iCurrState->reset();
        }

        // constructor
        HttpBasedProtocol() : iCurrState(NULL),
                iObserver(NULL),
                iComposer(NULL),
                iParser(NULL)
        {
            ;
        }

        virtual ~HttpBasedProtocol()
        {
            ;
        }

    protected:
        virtual ProtocolState* getNextState() = 0;

    protected:
        ProtocolState *iCurrState;
        ProtocolObserver *iObserver;
        HTTPComposer *iComposer;
        HttpParsingBasicObject *iParser; // wrap http parser to do parsing for each input media data
};

#endif // PVMF_PROTOCOLENGINE_H_INCLUDED

