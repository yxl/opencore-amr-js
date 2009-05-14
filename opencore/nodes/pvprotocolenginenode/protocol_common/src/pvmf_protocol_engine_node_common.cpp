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
#include "pvmf_protocol_engine_node.h"
#include "pvmf_protocol_engine_command_format_ids.h"
#include "pvmf_protocolengine_node_tunables.h"

#include "pvlogger.h"
#include "oscl_utf8conv.h"

/**
//Macros for calling PVLogger
*/
#define LOGERROR(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iLogger,PVLOGMSG_ERR,m);
#define LOGINFOHI(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG,iLogger,PVLOGMSG_INFO,m);
#define LOGINFOMED(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG,iLogger,PVLOGMSG_INFO,m);
#define LOGINFOLOW(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG,iLogger,PVLOGMSG_INFO,m);
#define LOGINFO(m) LOGINFOMED(m)
#define LOGINFODATAPATH(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG,iDataPathLogger,PVLOGMSG_INFO,m);
#define LOGERRORDATAPATH(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iDataPathLogger,PVLOGMSG_ERR,m);
#define LOGINFOCLOCK(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG,iClockLogger,PVLOGMSG_INFO,m);
#define PVMF_PROTOCOL_ENGINE_LOGBIN(iPortLogger, m) PVLOGGER_LOGBIN(PVLOGMSG_INST_LLDBG, iPortLogger, PVLOGMSG_ERR, m);
#define	NODEDATAPATHLOGGER_TAG "datapath.sourcenode.protocolenginenode"

////////////////////////////////////////////////////////////////////////////////////
//////	ProtocolContainer implementation
////////////////////////////////////////////////////////////////////////////////////

// constructor
OSCL_EXPORT_REF ProtocolContainer::ProtocolContainer(PVMFProtocolEngineNode *aNode) : iNode(aNode)
{
    clear();
    iDataPathLogger = PVLogger::GetLoggerObject(NODEDATAPATHLOGGER_TAG);
}

OSCL_EXPORT_REF void ProtocolContainer::clear()
{
    iProtocol			= NULL;
    iNodeOutput			= NULL;
    iDownloadControl	= NULL;
    iDownloadProgess	= NULL;
    iEventReport		= NULL;
    iCfgFileContainer	= NULL;
    iDownloadSource		= NULL;
    iNodeTimer			= NULL;
    iInterfacingObjectContainer = NULL;
    iUserAgentField		= NULL;
    iPortInForData = iPortInForLogging = iPortOut = NULL;
    iInternalEventQueue = NULL;
}

OSCL_EXPORT_REF OsclAny* ProtocolContainer::getObject(const NodeObjectType aObjectType)
{
    switch (aObjectType)
    {
        case NodeObjectType_Protocol:
            return (OsclAny*)iProtocol;
            break;

        case NodeObjectType_Output:
            return (OsclAny*)iNodeOutput;
            break;

        case NodeObjectType_DownloadControl:
            return (OsclAny*)iDownloadControl;
            break;

        case NodeObjectType_DownloadProgress:
            return (OsclAny*)iDownloadProgess;
            break;

        case NodeObjectType_EventReport:
            return (OsclAny*)iEventReport;
            break;

        case NodeObjectType_DlCfgFileContainer:
            return (OsclAny*)iCfgFileContainer;
            break;

        case NodeObjectType_DataSourceContainer:
            return (OsclAny*)iDownloadSource;
            break;

        case NodeObjectType_Timer:
            return (OsclAny*)iNodeTimer;
            break;

        case NodeObjectType_InterfacingObjectContainer:
            return (OsclAny*)iInterfacingObjectContainer;
            break;

        case NodeObjectType_UseAgentField:
            return (OsclAny*)iUserAgentField;
            break;

        default:
            break;
    }
    return NULL;
}

OSCL_EXPORT_REF bool ProtocolContainer::isObjectsReady()
{
    if (!iProtocol			||
            !iNodeOutput			||
            !iInterfacingObjectContainer ||
            iInterfacingObjectContainer->getURIObject().empty() ||
            !iInterfacingObjectContainer->getDataStreamFactory() ||
            !iPortInForData) return false;
    return true;
}

OSCL_EXPORT_REF void ProtocolContainer::setSupportObject(OsclAny* aSupportObject, const uint32 aType)
{
    switch ((NodeObjectType)aType)
    {
        case NodeObjectType_InputPortForData:
            iPortInForData = (PVMFProtocolEnginePort*)aSupportObject;
            break;

        case NodeObjectType_InputPortForLogging:
            iPortInForLogging = (PVMFProtocolEnginePort*)aSupportObject;
            break;

        case NodeObjectType_OutPort:
            iPortOut = (PVMFProtocolEnginePort*)aSupportObject;
            break;

        case NodeObjectType_InternalEventQueue:
            iInternalEventQueue = (Oscl_Vector<PVProtocolEngineNodeInternalEvent, PVMFProtocolEngineNodeAllocator>*)aSupportObject;
            break;
        default:
            break;
    }
}

OSCL_EXPORT_REF PVMFStatus ProtocolContainer::doPrepare()
{
    return initImpl();
}

OSCL_EXPORT_REF PVMFStatus ProtocolContainer::initImpl()
{
    if (!isObjectsReady())
    {
        return PVMFErrNotReady;
    }

    // initialize output object
    int32 status = initNodeOutput();
    if (status != PVMFSuccess) return status;

    // initialize protocol object
    if (!initProtocol()) return PVMFFailure;

    // initialize download control object
    initDownloadControl();

    // start data flow
    // if the current socket connection is down, then do socket reconnect
    bool needSocketReconnect = !iInterfacingObjectContainer->isSocketConnectionUp();
    startDataFlowByCommand(needSocketReconnect);

    return PVMFPending;
}


OSCL_EXPORT_REF bool ProtocolContainer::initProtocol()
{
    // then pass objects to protocol object (note that the order matters)
    iProtocol->setURI(iInterfacingObjectContainer->getURIObject());

    // update user-agent field
    if (!initProtocol_SetConfigInfo()) return false;
    iProtocol->setObserver(iNode);

    // protocol initialization and objects dispatch
    iProtocol->initialize();
    return true;
}

OSCL_EXPORT_REF PVMFStatus ProtocolContainer::doStop()
{
    // send socket disconnect command if necessary
    sendSocketDisconnectCmd();

    // disable sending logging message, but try to disconnect socket
    // use end of processing event to streamline all end of processing cases for stop
    EndOfDataProcessingInfo *aEOPInfo = iInterfacingObjectContainer->getEOPInfo();
    aEOPInfo->clear();
    aEOPInfo->iForceStop = true;
    PVProtocolEngineNodeInternalEvent aEvent(PVProtocolEngineNodeInternalEventType_EndOfProcessing, (OsclAny*)aEOPInfo);
    iObserver->DispatchEvent(&aEvent);

    return PVMFSuccess;
}

OSCL_EXPORT_REF void ProtocolContainer::sendSocketDisconnectCmd()
{
    if (iObserver->SendMediaCommand(iPortInForData, PVMF_MEDIA_CMD_SOCKET_DISCONNECT_FORMAT_ID))
    {
        if (iPortInForData->Send())
        {
            LOGINFODATAPATH((0, "ProtocolContainer::doStop()->sendSocketDisconnectCmd(), Send() SUCCESS: MsgID=%d(SOCKET DISCONNECT)", (uint32)PVMF_MEDIA_CMD_SOCKET_DISCONNECT_FORMAT_ID));
        }
    }
}

void ProtocolContainer::startDataFlowByCommand(const bool needDoSocketReconnect)
{
    // flush out existing data at this point
    checkEOSMsgFromInputPort();
    iObserver->ClearRest();

    // cancel all the existing timers
    iNodeTimer->clear();

    // disable info update at this point, will be enabled when new response comes in
    enableInfoUpdate(false);

    // socket reconnect
    if (needDoSocketReconnect) reconnectSocket();

    iObserver->RecheduleDataFlow();
}

void ProtocolContainer::checkEOSMsgFromInputPort()
{
    if (iPortInForData->IncomingMsgQueueSize() == 0)  return;

    // input port has media message
    while (iPortInForData->IncomingMsgQueueSize() > 0)
    {
        PVMFSharedMediaMsgPtr msg;
        PVMFStatus status = iPortInForData->DequeueIncomingMsg(msg);
        if (status != PVMFSuccess) continue;
        if (msg->getFormatID() == PVMF_MEDIA_CMD_EOS_FORMAT_ID)
        {
            iInterfacingObjectContainer->updateSocketConnectFlags(true);
            return;
        }
    }
}

OSCL_EXPORT_REF void ProtocolContainer::doClear(const bool aNeedDelete)
{
    iObserver->ClearRest(aNeedDelete);
    if (iInternalEventQueue) iInternalEventQueue->clear();
    if (iInterfacingObjectContainer) iInterfacingObjectContainer->clear();
    if (iNodeTimer) iNodeTimer->clear();
}

OSCL_EXPORT_REF void ProtocolContainer::doStopClear()
{
    doClear();
    if (iDownloadControl) iDownloadControl->clear();
    iEventReport->clear();
}

OSCL_EXPORT_REF void ProtocolContainer::doCancelClear()
{
    iObserver->ClearRest();
    if (iInternalEventQueue) iInternalEventQueue->clear();
    if (iNodeTimer) iNodeTimer->clear();
    if (iDownloadControl) iDownloadControl->clear();
    if (iEventReport) iEventReport->clear();
    //if (iInterfacingObjectContainer) iInterfacingObjectContainer->setInputDataUnwanted();

    // if re-do cancelled command, start from sending http request
    if (iProtocol) iProtocol->sendRequest();
}

OSCL_EXPORT_REF bool ProtocolContainer::reconnectSocket(const bool aForceSocketReconnect)
{
    if (!aForceSocketReconnect)
    {
        // Do not force to do socket reconnect, and then need to check the possibility
        if (iInterfacingObjectContainer->isSocketReconnectCmdSent()) return true;
    }
    if (!iObserver->SendMediaCommand(iPortInForData, PVMF_MEDIA_CMD_SOCKET_CONNECT_FORMAT_ID)) return false;
    iProtocol->sendRequest();
    iInterfacingObjectContainer->setSocketReconnectCmdSent();
    return true;
}

OSCL_EXPORT_REF bool ProtocolContainer::doEOS(const bool isTrueEOS)
{
    // download done
    if (isTrueEOS)
    {
        iObserver->SendMediaCommand(iPortInForData, PVMF_MEDIA_CMD_SOCKET_DISCONNECT_FORMAT_ID);
    }
    else   // EOS packet hasn't been received, so re-connect socket
    {
        bool aForceSocketReconnect = false;
        if (!iInterfacingObjectContainer->isPrevSocketConnectionUp())
        {
            // the situation is, previous connection is down and the current connection is down.
            // then force reconnect
            aForceSocketReconnect = true;
        }
        reconnectSocket(aForceSocketReconnect);
    }

    return true;
}

OSCL_EXPORT_REF uint32 ProtocolContainer::getBitMaskForHTTPMethod(const HttpMethod aMethod)
{
    uint32 bitMaskForHttpMethod = 0;
    if (aMethod == HTTP_GET) bitMaskForHttpMethod  = BITMASK_HTTPGET;
    if (aMethod == HTTP_POST) bitMaskForHttpMethod = BITMASK_HTTPPOST;
    if (aMethod == HTTP_HEAD) bitMaskForHttpMethod = BITMASK_HTTPHEAD;
    if (aMethod == HTTP_ALLMETHOD) bitMaskForHttpMethod = ~0;
    return bitMaskForHttpMethod;
}

OSCL_EXPORT_REF bool ProtocolContainer::createProtocolObjects()
{
    // create iInterfacingObjectContainer
    iInterfacingObjectContainer = OSCL_NEW(InterfacingObjectContainer, ());
    if (!iInterfacingObjectContainer) return false;

    // create iNodeTimer
    return createNetworkTimer();
}

bool ProtocolContainer::createNetworkTimer()
{
    iNodeTimer = PVMFProtocolEngineNodeTimer::create(iNode);
    if (!iNodeTimer) return false;
    iNodeTimer->set(SERVER_RESPONSE_TIMER_ID);
    iNodeTimer->set(SERVER_INACTIVITY_TIMER_ID);
    iNodeTimer->set(SERVER_RESPONSE_TIMER_ID_FOR_STOPEOS_LOGGING);
    iNodeTimer->set(WALL_CLOCK_TIMER_ID);
    iNodeTimer->set(BUFFER_STATUS_TIMER_ID);
    return true;
}

OSCL_EXPORT_REF void ProtocolContainer::deleteProtocolObjects()
{
    if (iInterfacingObjectContainer)	OSCL_DELETE(iInterfacingObjectContainer);
    iInterfacingObjectContainer = NULL;

    if (iNodeTimer)	OSCL_DELETE(iNodeTimer);
    iNodeTimer	= NULL;


    if (iProtocol)					OSCL_DELETE(iProtocol);
    iProtocol			  = NULL;
    if (iNodeOutput)					OSCL_DELETE(iNodeOutput);
    iNodeOutput			  = NULL;
    if (iDownloadControl)				OSCL_DELETE(iDownloadControl);
    iDownloadControl		  = NULL;
    if (iDownloadProgess)				OSCL_DELETE(iDownloadProgess);
    iDownloadProgess		  = NULL;
    if (iUserAgentField)				OSCL_DELETE(iUserAgentField);
    iUserAgentField		  = NULL;
    if (iEventReport)					OSCL_DELETE(iEventReport);
    iEventReport			  = NULL;
}

OSCL_EXPORT_REF void ProtocolContainer::handleTimeout(const int32 timerID)
{
    if (ignoreThisTimeout(timerID)) return;
    handleTimeoutErr(timerID);

    // currently for wm http streaming only
    handleTimeoutInPause(timerID);
    // may clean flags to cause handleTimeoutErr() to get excecuted, so move handleTimeoutErr() above
    handleTimeoutInDownloadStreamingDone(timerID);
}

OSCL_EXPORT_REF bool ProtocolContainer::ignoreThisTimeout(const int32 timerID)
{
    // check the end processing status: EOS recved and whole session is done
    if (iInterfacingObjectContainer->isEOSAchieved() &&
            iInterfacingObjectContainer->isWholeSessionDone()) return true;

    // Next, all focus on checking server inactivity timeout
    if (timerID != (int32)SERVER_INACTIVITY_TIMER_ID) return false;

    // inactivity timeout should be ignored in the following cases:
    // (i)  input/output port queue still has data,
    // (ii) buffer full in progressive streaming
    if (iPortInForData)
    {
        if (iPortInForData->IncomingMsgQueueSize() > 0) return true;
    }
    if (iPortOut)
    {
        if (iPortOut->OutgoingMsgQueueSize() > 0) return true;
    }

    if (iNodeOutput)
    {
        if (iNodeOutput->getAvailableOutputSize() == 0) return true;
    }
    return false;
}

bool ProtocolContainer::handleTimeoutErr(const int32 timerID)
{
    if (iInterfacingObjectContainer->isDownloadStreamingDone() ||
            iObserver->GetObserverState() == EPVMFNodePaused) return false;

    int32 timeoutErr = PVMFErrTimeout;
    if (timerID == SERVER_RESPONSE_TIMER_ID)   timeoutErr = PROCESS_TIMEOUT_SERVER_NO_RESPONCE;
    if (timerID == SERVER_INACTIVITY_TIMER_ID) timeoutErr = PROCESS_TIMEOUT_SERVER_INACTIVITY;

    ProtocolStateErrorInfo aInfo(timeoutErr);
    PVProtocolEngineNodeInternalEvent aEvent(PVProtocolEngineNodeInternalEventType_ProtocolStateError, (OsclAny*)(&aInfo));
    iObserver->DispatchEvent(&aEvent);
    return true;
}

OSCL_EXPORT_REF bool ProtocolContainer::handleProtocolStateComplete(PVProtocolEngineNodeInternalEvent &aEvent, PVProtocolEngineNodeInternalEventHandler *aEventHandler)
{
    bool aSessionDone = iInterfacingObjectContainer->isWholeSessionDone();
    bool aDownloadStreamingDone = iInterfacingObjectContainer->isDownloadStreamingDone();
    OSCL_UNUSED_ARG(aDownloadStreamingDone);
    bool aEOSArrived = iInterfacingObjectContainer->isEOSAchieved();

    iInterfacingObjectContainer->setInputDataUnwanted();
    if (aSessionDone)
    {
        // flush all the remaining output
        iNodeOutput->flushData();
        iNodeTimer->clear();
        if (aEOSArrived && iInterfacingObjectContainer->getOutputPortConnect())
        {
            doEOS(); // true EOS
            return aEventHandler->completePendingCommand(aEvent);
        }
    }
    return aEventHandler->completePendingCommand(aEvent);
}


////////////////////////////////////////////////////////////////////////////////////
//////	PVMFProtocolEngineNodeOutput implementation
////////////////////////////////////////////////////////////////////////////////////

// constructor
OSCL_EXPORT_REF PVMFProtocolEngineNodeOutput::PVMFProtocolEngineNodeOutput(PVMFProtocolEngineNodeOutputObserver *aObserver) :
        iPortIn(NULL),
        iContentDataMemPool(NULL),
        iMediaDataAlloc(NULL),
        iMediaDataMemPool("PVMFProtocolEngineNodeOutput(PVMFProtocolEngineNode)",
                          PVHTTPDOWNLOADOUTPUT_CONTENTDATA_POOLNUM,
                          PVHTTPDOWNLOADOUTPUT_MEDIADATA_CHUNKSIZE),
        iObserver(aObserver),
        iCurrTotalOutputSize(0)

{
    iOutputFramesQueue.reserve(PVPROTOCOLENGINE_RESERVED_NUMBER_OF_FRAMES);
    iLogger = PVLogger::GetLoggerObject("PVMFProtocolEngineNode");
    iDataPathLogger = PVLogger::GetLoggerObject(NODEDATAPATHLOGGER_TAG);
    iClockLogger = PVLogger::GetLoggerObject("clock");
    iMediaDataMemPool.enablenullpointerreturn();
}

OSCL_EXPORT_REF PVMFProtocolEngineNodeOutput::~PVMFProtocolEngineNodeOutput()
{
    reset();
}

// reset
OSCL_EXPORT_REF void PVMFProtocolEngineNodeOutput::reset()
{
    iPortIn = NULL;
    iLogger = NULL;
    iDataPathLogger = NULL;
    iClockLogger = NULL;
    iOutputFramesQueue.clear();
    iMediaData.Unbind();
    deleteMemPool();
}

OSCL_EXPORT_REF void PVMFProtocolEngineNodeOutput::setOutputObject(OsclAny* aOutputObject, const uint32 aObjectType)
{
    if (aObjectType == NodeOutputType_InputPortForData && aOutputObject) iPortIn = (PVMFProtocolEnginePort *)aOutputObject;
}


OSCL_EXPORT_REF bool PVMFProtocolEngineNodeOutput::sendToPort(PVMFSharedMediaDataPtr &aMediaData, const uint32 aPortType)
{
    OSCL_UNUSED_ARG(aPortType);
    return sendToDestPort(aMediaData, iPortIn);
}

OSCL_EXPORT_REF bool PVMFProtocolEngineNodeOutput::createMediaData(PVMFSharedMediaDataPtr &aMediaData, uint32 aRequestSize)
{
    // check if need to create memory pool
    int32 errcode = OsclErrNone;
    if (!iMediaDataAlloc)
    {
        errcode = createMemPool();
        if (errcode != PVMFSuccess) return false;
    }

    OsclSharedPtr<PVMFMediaDataImpl> mediadataImpl;
    errcode = 0;
    OSCL_TRY(errcode, mediadataImpl = iMediaDataAlloc->allocate(aRequestSize));
    if (errcode != OsclErrNone) return false;


    // Then wrap it around with PVMFMediaData
    iMediaData.Unbind();

    errcode = OsclErrNoResources;

    iMediaData = PVMFMediaData::createMediaData(mediadataImpl, &iMediaDataMemPool);

    if (iMediaData.GetRep() != NULL)
    {
        errcode = OsclErrNone;
    }

    if (errcode != OsclErrNone) return false;

    aMediaData = iMediaData;
    return true;
}

OSCL_EXPORT_REF PVMFStatus PVMFProtocolEngineNodeOutput::createMemPool()
{
    // Create the memory pool
    int32 errcode = 0;
    OSCL_TRY(errcode, iContentDataMemPool = OSCL_NEW(OsclMemPoolFixedChunkAllocator, (PVHTTPDOWNLOADOUTPUT_CONTENTDATA_POOLNUM)));
    if (errcode || iContentDataMemPool == NULL) return PVMFErrNoMemory;


    OSCL_TRY(errcode, iMediaDataAlloc = OSCL_NEW(PVMFSimpleMediaBufferCombinedAlloc, (iContentDataMemPool)));
    if (errcode || iMediaDataAlloc == NULL) return PVMFErrNoMemory;

    return PVMFSuccess;
}

OSCL_EXPORT_REF void PVMFProtocolEngineNodeOutput::deleteMemPool()
{
    // Cleanup output media data memory pool
    if (iMediaDataAlloc != NULL)
    {
        OSCL_DELETE(iMediaDataAlloc);
        iMediaDataAlloc = NULL;
    }

    if (iContentDataMemPool != NULL)
    {
        OSCL_DELETE(iContentDataMemPool);
        iContentDataMemPool = NULL;
    }
}

OSCL_EXPORT_REF bool PVMFProtocolEngineNodeOutput::sendToDestPort(PVMFSharedMediaDataPtr &aMediaData, PVMFProtocolEnginePort *aPort)
{
    // compute data frag size for log purposes
    uint32 dataSize = 0;
    uint32 numFrags = aMediaData->getNumFragments();
    for (uint32 i = 0; i < numFrags; i++)
    {
        OsclRefCounterMemFrag memFragIn;
        aMediaData->getMediaFragment(i, memFragIn);
        uint32 fragLen = memFragIn.getMemFrag().len;
        dataSize += fragLen;
    }

    // Send frame to downstream node
    PVMFSharedMediaMsgPtr mediaMsgOut;
    convertToPVMFMediaMsg(mediaMsgOut, aMediaData);

    LOGINFODATAPATH((0, "PVMFProtocolEngineNodeOutput::sendToDestPort() SEQNUM= %d, SIZE= %d, port = 0x%x",
                     mediaMsgOut->getSeqNum(), dataSize, aPort));

    PVMFStatus status = aPort->QueueOutgoingMsg(mediaMsgOut);
    return iObserver->QueueOutgoingMsgSentComplete(aPort, mediaMsgOut, status);
}

OSCL_EXPORT_REF bool PVMFProtocolEngineNodeOutput::passDownNewOutputData(OUTPUT_DATA_QUEUE &aOutputQueue, OsclAny* aSideInfo)
{
    OSCL_UNUSED_ARG(aSideInfo);
    int32 err = 0;
    OSCL_TRY(err, iOutputFramesQueue.push_back(aOutputQueue););
    return (err == 0);
}

OSCL_EXPORT_REF int32 PVMFProtocolEngineNodeOutput::flushData(const uint32 aOutputType)
{
    if (iMediaData.GetRep() == NULL)  return PROCESS_SUCCESS;

    // send to port
    if (!sendToPort(iMediaData, aOutputType)) return PROCESS_OUTPUT_PORT_IS_BUSY;
    iMediaData.Unbind();
    return PROCESS_SUCCESS;
}

OSCL_EXPORT_REF bool PVMFProtocolEngineNodeOutput::getBuffer(PVMFSharedMediaDataPtr &aMediaData, uint32 aRequestSize)
{
    if (!createMediaData(aMediaData, aRequestSize)) return false;
    return true;
}

OSCL_EXPORT_REF void PVMFProtocolEngineNodeOutput::discardData(const bool aNeedReopen)
{
    OSCL_UNUSED_ARG(aNeedReopen);
    iOutputFramesQueue.clear();
    iMediaData.Unbind();
}

OSCL_EXPORT_REF bool PVMFProtocolEngineNodeOutput::isPortBusy()
{
    return iPortIn->IsOutgoingQueueBusy();
}



////////////////////////////////////////////////////////////////////////////////////
//////	UserAgentField implementation
////////////////////////////////////////////////////////////////////////////////////
// constructor
OSCL_EXPORT_REF UserAgentField::UserAgentField(OSCL_wString &aUserAgent, const bool isOverwritable)
{
    setUserAgent(aUserAgent, isOverwritable);
}

OSCL_EXPORT_REF UserAgentField::UserAgentField(OSCL_String &aUserAgent, const bool isOverwritable)
{
    setUserAgent(aUserAgent, isOverwritable);
}

// set user agent
OSCL_EXPORT_REF bool UserAgentField::setUserAgent(OSCL_wString &aUserAgent, const bool isOverwritable)
{
    iOverwritable = isOverwritable;

    // check for empty string
    if (aUserAgent.get_size() == 0) return true;

    OsclMemAllocator alloc;
    char *buf = (char*)alloc.allocate(aUserAgent.get_size() + 1);
    if (!buf) return false;
    uint32 size = 0;
    if ((size = oscl_UnicodeToUTF8(aUserAgent.get_cstr(), aUserAgent.get_size(), buf, aUserAgent.get_size() + 1)) == 0)
    {
        alloc.deallocate(buf);
        return false;
    }
    iInputUserAgent = OSCL_HeapString<OsclMemAllocator> (buf, size);
    alloc.deallocate(buf);
    return true;
}

OSCL_EXPORT_REF bool UserAgentField::setUserAgent(OSCL_String &aUserAgent, const bool isOverwritable)
{
    iOverwritable = isOverwritable;
    // check for empty string
    if (aUserAgent.get_size() == 0) return true;

    iInputUserAgent = OSCL_HeapString<OsclMemAllocator> (aUserAgent.get_str(), aUserAgent.get_size());
    return true;
}

// get the actual user agent (not wide string version) based on overwrite mode or replace mode (set the input user agent to the default one)
OSCL_EXPORT_REF bool UserAgentField::getUserAgent(OSCL_String &aUserAgent)
{
    if (iActualUserAgent.get_size() > 0)
    {
        aUserAgent = iActualUserAgent;
        return true;
    }

    // create iActualUserAgent at the first call
    if (iOverwritable && iInputUserAgent.get_size() > 0)
    {
        iActualUserAgent = iInputUserAgent;
    }
    else   // append
    {
        //OSCL_FastString defaultUserAgent(_STRLIT_CHAR("PVPlayer/4.0 (Beta release)"));
        OSCL_HeapString<OsclMemAllocator> defaultUserAgent;
        getDefaultUserAgent(defaultUserAgent);
        uint32 size = defaultUserAgent.get_size() + iInputUserAgent.get_size() + 1; // 1 => space
        OsclMemAllocator alloc;
        char *buf = (char*)alloc.allocate(size + 1);
        if (!buf) return false;
        oscl_memcpy(buf, defaultUserAgent.get_cstr(), defaultUserAgent.get_size());
        buf[defaultUserAgent.get_size()] = PROTOCOLENGINENODE_SPACE_ASCIICODE;
        if (iInputUserAgent.get_size() > 0)
        {
            oscl_memcpy(buf + defaultUserAgent.get_size() + 1, iInputUserAgent.get_cstr(), iInputUserAgent.get_size());
        }
        buf[size] = 0;
        iActualUserAgent = OSCL_HeapString<OsclMemAllocator> (buf, size);
        alloc.deallocate(buf);
    }
    aUserAgent = iActualUserAgent;
    return true;
}

////////////////////////////////////////////////////////////////////////////////////
//////	EventReporter implementation
////////////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF EventReporter::EventReporter(EventReporterObserver *aObserver) : iObserver(aObserver)
{
    clear();
    iDataPathLogger = PVLogger::GetLoggerObject(NODEDATAPATHLOGGER_TAG);
}

OSCL_EXPORT_REF void EventReporter::clear()
{
    iStarted = false;
}

OSCL_EXPORT_REF void EventReporter::startRealDataflow()
{
    iStarted = true;
}


////////////////////////////////////////////////////////////////////////////////////
//////	InterfacingObjectContainer implementation
////////////////////////////////////////////////////////////////////////////////////

// constructor
InterfacingObjectContainer::InterfacingObjectContainer() :
        iDownloadFormat(PVMF_MIME_DATA_SOURCE_HTTP_URL),
        iDataStreamFactory(NULL),
        iNumBuffersInAllocator(PVHTTPSTREAMINGOUTPUT_CONTENTDATA_POOLNUM),
        iNumRedirectTrials(PVPROTOCOLENGINE_DEFAULT_NUMBER_OF_REDIRECT_TRIALS),
        iNumBuffersInMediaDataPoolSMCalc(PVHTTPSTREAMINGOUTPUT_CONTENTDATA_POOLNUM),
        iCurrSocketConnection(true),
        iPrevSocketConnection(true),
        iLatestDataPacketNumSent(0),
        iOutputPortConnected(false)
{
    clear();
    iOutgoingMsgSentSuccessInfoVec.reserve(PVMF_PROTOCOLENGINE_NODE_COMMAND_VECTOR_RESERVE / 4);
}

// set and get http header
OSCL_EXPORT_REF uint32 InterfacingObjectContainer::setHttpHeader(OUTPUT_DATA_QUEUE &aHttpHeader)
{
    iHttpHeaderLength = 0;
    uint32 i = 0;
    char *ptr = (char*)iHttpHeaderBuffer;
    for (i = 0; i < aHttpHeader.size(); i++)
    {
        uint32 fragSize = aHttpHeader[i].getMemFragSize();
        oscl_memcpy(ptr, (char*)aHttpHeader[i].getMemFragPtr(), fragSize);
        iHttpHeaderLength += fragSize;
        ptr += fragSize;
    }
    iHttpHeaderBuffer[iHttpHeaderLength] = 0;
    return iHttpHeaderLength;
}

OSCL_EXPORT_REF bool InterfacingObjectContainer::setStreamingProxy(OSCL_wString& aProxyName, const uint32 aProxyPort)
{
    if (aProxyName.get_size() == 0) return false;

    OsclMemAllocator alloc;
    char *buf = (char*)alloc.allocate(aProxyName.get_size() + 1);
    if (!buf) return false;
    uint32 size = oscl_UnicodeToUTF8(aProxyName.get_cstr(), aProxyName.get_size(), buf, aProxyName.get_size() + 1);
    if (size == 0)
    {
        alloc.deallocate(buf);
        return false;
    }
    iProxyName = OSCL_HeapString<OsclMemAllocator> (buf, size);
    iProxyPort = aProxyPort;
    alloc.deallocate(buf);
    return true;
}

OSCL_EXPORT_REF void InterfacingObjectContainer::setNumBuffersInMediaDataPoolSMCalc(uint32 aVal)
{
    iNumBuffersInMediaDataPoolSMCalc = aVal;
}


////////////////////////////////////////////////////////////////////////////////////
//////	PVMFProtocolEngineNodeTimer implementation
////////////////////////////////////////////////////////////////////////////////////

OSCL_EXPORT_REF PVMFProtocolEngineNodeTimer* PVMFProtocolEngineNodeTimer::create(OsclTimerObserver *aObserver)
{
    PVMFProtocolEngineNodeTimer *timer = OSCL_NEW(PVMFProtocolEngineNodeTimer, ());
    if (!timer) return NULL;
    if (!timer->construct(aObserver))
    {
        OSCL_DELETE(timer);
        return NULL;
    }
    return timer;
}

bool PVMFProtocolEngineNodeTimer::construct(OsclTimerObserver *aObserver)
{
    // create iWatchdogTimer
    iWatchdogTimer = OSCL_NEW(OsclTimer<PVMFProtocolEngineNodeAllocator>, ("ProtEngineNodeWatchdogTimer"));
    if (!iWatchdogTimer) return false;
    iWatchdogTimer->SetObserver(aObserver);
    iWatchdogTimer->SetFrequency(PVPROTOCOLENGINENODE_TIMER_FREQUENCY);

    int32 err = 0;
    OSCL_TRY(err, iTimerVec.reserve(DEFAULT_TIMER_VECTOR_RESERVE_NUMBER);)
    return (err ? false : true);
}

OSCL_EXPORT_REF PVMFProtocolEngineNodeTimer::~PVMFProtocolEngineNodeTimer()
{
    if (iWatchdogTimer)
    {
        OSCL_DELETE(iWatchdogTimer);
        iWatchdogTimer = NULL;
    }

    iTimerVec.clear();
}

OSCL_EXPORT_REF void PVMFProtocolEngineNodeTimer::set(const uint32 aTimerID, const int32 aTimeout)
{
    uint32 index = getTimerVectorIndex(aTimerID);
    if (index == 0xffffffff)
    {
        // attach the new timer
        uint32 timeout = aTimeout;
        if (timeout == 0) timeout = getDefaultTimeout(aTimerID);
        TimerUnit timerUnit(aTimerID, timeout);
        iTimerVec.push_back(timerUnit);
    }
    else
    {
        // existing timer
        uint32 timeout = iTimerVec[index].iTimeout;
        if (aTimeout > 0 || (aTimeout == 0 && aTimerID == SERVER_RESPONSE_TIMER_ID_FOR_STOPEOS_LOGGING))
        {
            // for stop/eos logging timeout, allow zero timeout
            iTimerVec[index].iTimeout = aTimeout;
        }
        timeout = iTimerVec[index].iTimeout;
    }
}

OSCL_EXPORT_REF uint32 PVMFProtocolEngineNodeTimer::getTimeout(const uint32 aTimerID)
{
    uint32 aTimeout = 0xffffffff;
    uint32 index = getTimerVectorIndex(aTimerID);
    if (index != 0xffffffff)
    {
        // existing timer
        aTimeout = iTimerVec[index].iTimeout;
    }
    return aTimeout;
}


OSCL_EXPORT_REF void PVMFProtocolEngineNodeTimer::cancel(const uint32 aTimerID)
{
    uint32 index = getTimerVectorIndex(aTimerID);
    if (index != 0xffffffff)
    {
        // find this timer and cancel it
        iWatchdogTimer->Cancel(aTimerID);
    }
}

void PVMFProtocolEngineNodeTimer::clear()
{
    for (uint32 i = 0; i < iTimerVec.size(); i++)
    {
        iWatchdogTimer->Cancel(iTimerVec[i].iTimerID);
    }
}

OSCL_EXPORT_REF void PVMFProtocolEngineNodeTimer::clearExcept(const uint32 aTimerID)
{
    for (uint32 i = 0; i < iTimerVec.size(); i++)
    {
        if (iTimerVec[i].iTimerID != aTimerID)
        {
            iWatchdogTimer->Cancel(iTimerVec[i].iTimerID);
        }
    }
}

OSCL_EXPORT_REF bool PVMFProtocolEngineNodeTimer::start(const uint32 aTimerID, const int32 aTimeout)
{
    uint32 index = getTimerVectorIndex(aTimerID);
    if (index == 0xffffffff) return false;
    if (aTimeout) iTimerVec[index].iTimeout = aTimeout;
    if (iTimerVec[index].iTimeout == 0)
    {
        iTimerVec[index].iTimeout = getDefaultTimeout(aTimerID);
    }

    iWatchdogTimer->Cancel(iTimerVec[index].iTimerID);
    iWatchdogTimer->Request(iTimerVec[index].iTimerID, 0, iTimerVec[index].iTimeout);
    return true;
}

uint32 PVMFProtocolEngineNodeTimer::getDefaultTimeout(const uint32 aTimerID)
{
    switch (aTimerID)
    {
        case SERVER_RESPONSE_TIMER_ID:
            return DEFAULT_MAX_SERVER_RESPONSE_DURATION_IN_SEC;
        case SERVER_INACTIVITY_TIMER_ID:
            return DEFAULT_MAX_SERVER_INACTIVITY_DURATION_IN_SEC;
        case SERVER_KEEPALIVE_TIMER_ID:
            return DEFAULT_KEEPALIVE_TIMEOUT_IN_SEC;
        case SERVER_RESPONSE_TIMER_ID_FOR_STOPEOS_LOGGING:
            return DEFAULT_MAX_SERVER_RESPONSE_DURATION_IN_SEC_FOR_STOPEOS_LOGGING;
        case WALL_CLOCK_TIMER_ID:
            return DEFAULT_WALLCLOCK_TIMEOUT_IN_SEC;
        case BUFFER_STATUS_TIMER_ID:
            return DEFAULT_BUFFER_STATUS_CLOCK_TIMEOUT_IN_SEC;
        default:
            break;
    }
    return 0;
}

uint32 PVMFProtocolEngineNodeTimer::getTimerVectorIndex(const uint32 aTimerID)
{
    for (uint32 i = 0; i < iTimerVec.size(); i++)
    {
        if (iTimerVec[i].iTimerID == aTimerID) return i;
    }
    return ~0;
}

