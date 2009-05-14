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
* @file pv_comms_io_node_port.cpp
* @brief Port for Comms I/O interfacing; can be unidirectional or bidirectional
*/

#ifndef PV_COMMS_IO_NODE_OUTPORT_H_INCLUDED
#include "pv_comms_io_node_port.h"
#endif
#ifndef PV_COMMS_IO_NODE_H_INCLUDED
#include "pv_comms_io_node.h"
#endif
#ifndef PVMF_MEDIA_MSG_FORMAT_IDS_H_INCLUDED
#include "pvmf_media_msg_format_ids.h"
#endif

#ifndef OSCL_BASE_MACROS_H_INCLUDED
#include "oscl_base_macros.h"
#endif

#define PVMIO_MEDIADATA_POOLNUM 9
#define MIO_PORT_QUEUE_LIMIT 10

#define INPUT_TRANSFER_MODEL_VAL ".../input/transfer_model;valtype=uint32"
#define OUTPUT_TRANSFER_MODEL_VAL ".../output/transfer_model;valtype=uin32"

// Logging macros
#define LOGDATAPATH(x)	PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iDatapathLogger, PVLOGMSG_INFO, x);
#define LOG_STACK_TRACE(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, m);
#define LOG_DEBUG(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, m);
#define LOG_ERR(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iLogger,PVLOGMSG_ERR,m);

#define IS_INPUT_PORT 		(iTag&PVMF_COMMSIO_NODE_INPUT_PORT_TAG)

#define IS_OUTPUT_PORT     	(iTag&PVMF_COMMSIO_NODE_OUTPUT_PORT_TAG)


////////////////////////////////////////////////////////////////////////////
PVCommsIONodePort::PVCommsIONodePort(int32 aPortTag, PVCommsIONode* aNode)
        : OsclTimerObject(OsclActiveObject::EPriorityNominal, "PVCommsIONodePort")
        , PvmfPortBaseImpl(aPortTag
                           //this port handles its own port activity
                           , this
                           // incoming data queue config
                           , 0
                           , DEFAULT_DATA_QUEUE_CAPACITY
                           , DEFAULT_READY_TO_RECEIVE_THRESHOLD_PERCENT
                           // outgoing data queue config
                           , 0
                           , DEFAULT_DATA_QUEUE_CAPACITY
                           , DEFAULT_READY_TO_RECEIVE_THRESHOLD_PERCENT,
                           "PVCommsIONodePort")
        , iNode(aNode)
        , iMediaOutputTransfer(NULL)
        , iMediaInputTransfer(NULL)
        , iMediaDataMemPool(PVMIO_MEDIADATA_POOLNUM)
        , iState(PVCommsIONodePort::PORT_STATE_BUFFERING)
        , iWriteState(EWriteOK)
        , iActiveCommsWrite(true)
        , iActiveCommsRead(false)
        , iEndOfInputPortDataReached(false)
        , iEndOfOutputPortDataReached(false)
{
    cmdId = 0;
    AddToScheduler();
    iFormatType = PVMF_MIME_FORMAT_UNKNOWN;
    iWriteFailed = false;
    iPeer = NULL;
    iCleanupQueue.reserve(1);
    iWriteAsyncContext = 0;
    iQueueLimit = MIO_PORT_QUEUE_LIMIT;
    iTxLogger = iRxLogger = NULL;

    switch (iTag)
    {
        case PVMF_COMMSIO_NODE_INPUT_PORT_TAG:
            // configure default incoming queue size,
            // and we don't need outgoing queue
            SetCapacity(EPVOutgoingDataQueue, 0);
            SetReserve(EPVOutgoingDataQueue, 0);
            SetThreshold(EPVOutgoingDataQueue, 0);
            if (iNode->iLogBitstream)
            {
                iTxLogger = PVLogger::GetLoggerObject("pvcommionode.tx.bin");
                iTxLogger->DisableAppenderInheritance();
            }
            break;
        case PVMF_COMMSIO_NODE_OUTPUT_PORT_TAG:
            // configure default outgoing queue size,
            // and we don't need icomiing queue
            SetCapacity(EPVIncomingDataQueue, 0);
            SetReserve(EPVIncomingDataQueue, 0);
            SetThreshold(EPVIncomingDataQueue, 0);
            if (iNode->iLogBitstream)
            {
                iRxLogger = PVLogger::GetLoggerObject("pvcommionode.rx.bin");
                iRxLogger->DisableAppenderInheritance();
            }
            break;
        default:
            if (iNode->iLogBitstream)
            {
                iTxLogger = PVLogger::GetLoggerObject("pvcommionode.tx.bin");
                iRxLogger = PVLogger::GetLoggerObject("pvcommionode.rx.bin");
                iTxLogger->DisableAppenderInheritance();
                iRxLogger->DisableAppenderInheritance();
            }
            // defaults are already set for I/O
            break;
    }

#ifdef USE_COPY_BUFFER
    for (int i = 0; i < NUM_COPY_BUFFERS; i++)
    {
        oscl_memset(&iCopyBuffer[i][0], 0, COPY_BUFFER_SIZE);
        iCopyBufferSize[i] = 0;
    }
    iCopyBufferIndex = 0;
    iCopyBufferSent = false;
#endif
}

void PVCommsIONodePort::ClearCleanupQueue()
{//clear the media transfer cleanup queue and log all messages.
    while (!iCleanupQueue.empty())
    {
        PVMFSharedMediaDataPtr mediaData = iCleanupQueue.begin()->iData;
        PVMFCommandId cmdId = iCleanupQueue.begin()->iCmdId;
        iCleanupQueue.erase(iCleanupQueue.begin());
        LogMediaDataInfo("Cleared"
                         , mediaData
                         , cmdId
                         , iCleanupQueue.size()
                        );
    }
}


////////////////////////////////////////////////////////////////////////////
PVCommsIONodePort::~PVCommsIONodePort()
{
    PvmfPortBaseImpl::ClearMsgQueues();

    //cancel any pending write operations
    if (!iCleanupQueue.empty())
    {
        int32 err;
        OSCL_TRY(err, iMediaOutputTransfer->cancelAllCommands(););
        ClearCleanupQueue();
    }

}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PVCommsIONodePort::Start()
{
    iState = PVCommsIONodePort::PORT_STATE_STARTED;
    if (iNode->iMediaIOState == PVCommsIONode::MIO_STATE_STARTED)
    {
        RunIfNotReady();
    }
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PVCommsIONodePort::MediaIOStarted()
{
    if (iState == PVCommsIONodePort::PORT_STATE_STARTED)
        RunIfNotReady();
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PVCommsIONodePort::Pause()
{
    iState = PVCommsIONodePort::PORT_STATE_BUFFERING;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PVCommsIONodePort::Stop()
{
    ClearMsgQueues();
    iState = PVCommsIONodePort::PORT_STATE_BUFFERING;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVCommsIONodePort::Configure(PVMFFormatType aPortProperty)
{
    if (iConnectedPort)
    {
        // Must disconnect before changing port properties, so return error
        return PVMFFailure;
    }

    iFormatType = aPortProperty;
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVCommsIONodePort::Connect(PVMFPortInterface* aPort)
{
    PVMFStatus status = PvmfPortBaseImpl::Connect(aPort);

    if (status != PVMFSuccess)
        return status;

    if (!iNode->CreateMediaTransfer(iTag, iMediaInputTransfer, iMediaOutputTransfer))
        return PVMFFailure;

    if (iMediaInputTransfer)
        iMediaInputTransfer->setPeer(this);
    if (iMediaOutputTransfer &&
            iMediaOutputTransfer != iMediaInputTransfer)
        iMediaOutputTransfer->setPeer(this);

    iEndOfInputPortDataReached = false;
    iEndOfOutputPortDataReached = false;

    SetDataTransferModels();
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVCommsIONodePort::PeerConnect(PVMFPortInterface* aPort)
{
    PVMFStatus status = PvmfPortBaseImpl::PeerConnect(aPort);
    if (status != PVMFSuccess)
        return status;

    if (!iNode->CreateMediaTransfer(iTag, iMediaInputTransfer, iMediaOutputTransfer))
        return PVMFFailure;

    if (iMediaInputTransfer)
        iMediaInputTransfer->setPeer(this);
    if (iMediaOutputTransfer &&
            iMediaOutputTransfer != iMediaInputTransfer)
        iMediaOutputTransfer->setPeer(this);

    SetDataTransferModels();

    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVCommsIONodePort::Disconnect()
{
    PVMFStatus status = PvmfPortBaseImpl::Disconnect();
    if (status != PVMFSuccess)
        return status;

    iNode->DeleteMediaTransfer(iTag, iMediaInputTransfer, iMediaOutputTransfer);
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVCommsIONodePort::PeerDisconnect()
{
    PVMFStatus status = PvmfPortBaseImpl::PeerDisconnect();
    if (status != PVMFSuccess)
        return status;

    iNode->DeleteMediaTransfer(iTag, iMediaInputTransfer, iMediaOutputTransfer);
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
void PVCommsIONodePort::setPeer(PvmiMediaTransfer *aPeer)
{
    iPeer = aPeer;
}

////////////////////////////////////////////////////////////////////////////
void PVCommsIONodePort::useMemoryAllocators(OsclMemAllocator* write_alloc)
{
    OSCL_UNUSED_ARG(write_alloc);
    OSCL_LEAVE(OsclErrNotSupported);
}

////////////////////////////////////////////////////////////////////////////
PVMFCommandId PVCommsIONodePort::writeAsync(uint8 format_type, int32 format_index, uint8* data, uint32 data_len,
        const PvmiMediaXferHeader& data_header_info, OsclAny* aContext)
{
    OSCL_UNUSED_ARG(format_type);
    OSCL_UNUSED_ARG(format_index);

    // TODO: Handle incoming data here. Create a media data using PvmiMIOSourceDataBufferAlloc::allocate,
    // save the data there, put the media data to outgoing queue.
    // If the port is started, schedule to send in Run
    if (!IS_OUTPUT_PORT || isActiveCommsRead())
    {
        // Not configured for input or passive read
        OsclError::Leave(OsclErrNotSupported);
    }

    PVLOGGER_LOGBIN(PVLOGMSG_INST_LLDBG, iRxLogger, PVLOGMSG_DEBUG, (0, 1, data_len, data));

    //if the outgoing queue is full, we can't accept data
    //now.
    if (IsOutgoingQueueBusy())
    {
        iWriteFailed = true;
        OsclError::Leave(OsclErrGeneral);
    }

    // Create new media data buffer
    PVMFSharedMediaDataPtr mediaData;
    int32 err = 0;
    if (cmdId == 0x7FFFFFFF)
        cmdId = 0;
    OSCL_TRY(err,
             OsclSharedPtr<PVMFMediaDataImpl> mediaDataImpl = iMediaDataAlloc.allocate(iMediaInputTransfer, data,
                     data_len, cmdId, aContext);
             mediaData = PVMFMediaData::createMediaData(mediaDataImpl, &iMediaDataMemPool);
            );

    // Set timestamp
    mediaData->setTimestamp(data_header_info.timestamp);
    mediaData->setSeqNum(data_header_info.seq_num);
    mediaData->setMediaFragFilledLen(0, data_len);

    // Convert media data to MediaMsg
    PVMFSharedMediaMsgPtr mediaMsg;
    convertToPVMFMediaMsg(mediaMsg, mediaData);


    if (mediaMsg->getFormatID() == PVMF_MEDIA_CMD_EOS_FORMAT_ID)
    {
        iEndOfOutputPortDataReached = true;
        RunIfNotReady();
    }

    PVMFStatus status = QueueOutgoingMsg(mediaMsg);
    if (status != PVMFSuccess)
        iNode->ReportErrorEvent(PVMFErrPortProcessing, (OsclAny*)status);

    if (iState == PVCommsIONodePort::PORT_STATE_STARTED)
        RunIfNotReady();
    return cmdId++;
}

////////////////////////////////////////////////////////////////////////////

void PVCommsIONodePort::writeComplete(PVMFStatus status, PVMFCommandId aCmdId, OsclAny* aContext)
//for PvmiMediaTransfer
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO,
                    (0, "PVCommsIONodePort::writeComplete status %d cmdId %d context 0x%x", status, aCmdId, aContext));



#ifdef USE_COPY_BUFFER
    PVLOGGER_LOGBIN(PVLOGMSG_INST_LLDBG, iTxLogger, PVLOGMSG_DEBUG, (0, 1, iCopyBufferSize[(iCopyBufferIndex - 1) % NUM_COPY_BUFFERS], iCopyBuffer[(iCopyBufferIndex -1) % NUM_COPY_BUFFERS]));
    iCopyBufferSize[(iCopyBufferIndex - 1) % NUM_COPY_BUFFERS] = 0;
    iCopyBufferSent = false;
    RunIfNotReady();
    return;
#endif

    //we don't expect any error status to be returned here
    //except possibly cancelled
    if (status != PVMFSuccess
            && status != PVMFErrCancelled)
        iNode->ReportErrorEvent(PVMFErrPortProcessing, NULL, PVCommsIONodeErr_WriteComplete);

    //detect cases where the current call is completing synchronously.
    if (iWriteState == EWriteBusy
            && (uint32)aContext == iWriteAsyncContext)
    {
        //synchronous completion
        iWriteState = EWriteOK;
    }
    else
    {
        //asynchronous completion
        //do any memory cleanup
        uint32 i;
        for (i = 0;i < iCleanupQueue.size();i++)
        {
            if (iCleanupQueue[i].iCmdId == aCmdId)
            {
                PVMFSharedMediaDataPtr mediaData = iCleanupQueue[i].iData;
                iCleanupQueue.erase(&iCleanupQueue[i]);

                LogMediaDataInfo("Async Write Complete"
                                 , mediaData
                                 , aCmdId
                                 , iCleanupQueue.size()
                                );
                //may need to generate port flow control now
                PvmfPortBaseImpl::EvaluateIncomingBusy();
                break;
            }
        }
        //we may be waiting on completion of EOS
        if (EndOfData(PVMF_COMMSIO_NODE_INPUT_PORT_TAG))
            RunIfNotReady();
    }
}



////////////////////////////////////////////////////////////////////////////
PVMFCommandId PVCommsIONodePort::readAsync(uint8* data, uint32 max_data_len, OsclAny* context,
        int32* formats, uint16 num_formats)
{
    OSCL_UNUSED_ARG(data);
    OSCL_UNUSED_ARG(max_data_len);
    OSCL_UNUSED_ARG(context);
    OSCL_UNUSED_ARG(formats);
    OSCL_UNUSED_ARG(num_formats);

    if (!IS_INPUT_PORT || isActiveCommsWrite())
    {
        OsclError::Leave(OsclErrNotSupported);
    }

    return -1;
}

////////////////////////////////////////////////////////////////////////////
void PVCommsIONodePort::readComplete(PVMFStatus status, PVMFCommandId read_cmd_id,
                                     int32 format_index, const PvmiMediaXferHeader& data_header_info,
                                     OsclAny* context)
{
    OSCL_UNUSED_ARG(status);
    OSCL_UNUSED_ARG(read_cmd_id);
    OSCL_UNUSED_ARG(format_index);
    OSCL_UNUSED_ARG(data_header_info);
    OSCL_UNUSED_ARG(context);
    OSCL_LEAVE(OsclErrNotSupported);
}

////////////////////////////////////////////////////////////////////////////
void PVCommsIONodePort::cancelCommand(PVMFCommandId command_id)
{
    OSCL_UNUSED_ARG(command_id);
    OSCL_LEAVE(OsclErrNotSupported);
}

////////////////////////////////////////////////////////////////////////////
void PVCommsIONodePort::cancelAllCommands()
{
    OSCL_LEAVE(OsclErrNotSupported);
}

////////////////////////////////////////////////////////////////////////////
//                  PvmiCapabilityAndConfig
////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PVCommsIONodePort::setObserver(PvmiConfigAndCapabilityCmdObserver* aObserver)
{
    // Not supported
    OSCL_UNUSED_ARG(aObserver);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVCommsIONodePort::getParametersSync(PvmiMIOSession session,
        PvmiKeyType identifier,
        PvmiKvp*& parameters,
        int& num_parameter_elements,
        PvmiCapabilityContext context)
{
    LOG_STACK_TRACE((0, "PVCommsIONodePort::getParametersSync"));

    MIOControlContextSet set;

    if (!iNode)
    {
        LOG_ERR((0, "PVCommsIONodePort::getParametersSync: Error - Config object for media IO not available"));
        return PVMFFailure;

    }
    set = iNode->ContextSetFromTag(iTag);

    if (set.iMediaInputElement &&
            set.iMediaInputElement->hasConfig() &&
            (set.iMediaInputElement->iMediaIOConfig->getParametersSync(session, identifier, parameters, num_parameter_elements, context) == PVMFSuccess))
        return PVMFSuccess;

    // if this is a proxy port, we don't really have any way of determining
    // which config we want to query, so if the input query fails, we'll
    // try on the output config
    if (set.iMediaOutputElement &&
            set.iMediaOutputElement->hasConfig() &&
            (set.iMediaOutputElement->iMediaIOConfig->getParametersSync(session, identifier, parameters, num_parameter_elements, context) == PVMFSuccess))
        return PVMFSuccess;

    return PVMFFailure;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVCommsIONodePort::releaseParameters(PvmiMIOSession session,
        PvmiKvp* parameters,
        int num_elements)
{
    LOG_STACK_TRACE((0, "PVCommsIONodePort::releaseParameters"));

    MIOControlContextSet set;

    if (!iNode)
    {
        LOG_ERR((0, "PVCommsIONodePort::releaseParameters: Error - Config object for media IO not available"));
        return PVMFFailure;

    }

    set = iNode->ContextSetFromTag(iTag);

    if (set.iMediaInputElement &&
            set.iMediaInputElement->hasConfig() &&
            (set.iMediaInputElement->iMediaIOConfig->releaseParameters(session, parameters, num_elements) == PVMFSuccess))
        return PVMFSuccess;

    // if this is a proxy port, we don't really have any way of determining
    // which config we want to query, so if the input query fails, we'll
    // try on the output config
    if (set.iMediaOutputElement &&
            set.iMediaOutputElement->hasConfig() &&
            (set.iMediaOutputElement->iMediaIOConfig->releaseParameters(session, parameters, num_elements) == PVMFSuccess))
        return PVMFSuccess;

    return PVMFFailure;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PVCommsIONodePort::createContext(PvmiMIOSession session, PvmiCapabilityContext& context)
{
    OSCL_UNUSED_ARG(session);
    OSCL_UNUSED_ARG(context);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PVCommsIONodePort::setContextParameters(PvmiMIOSession session,
        PvmiCapabilityContext& context,
        PvmiKvp* parameters, int num_parameter_elements)
{
    OSCL_UNUSED_ARG(session);
    OSCL_UNUSED_ARG(context);
    OSCL_UNUSED_ARG(parameters);
    OSCL_UNUSED_ARG(num_parameter_elements);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PVCommsIONodePort::DeleteContext(PvmiMIOSession session, PvmiCapabilityContext& context)
{
    OSCL_UNUSED_ARG(session);
    OSCL_UNUSED_ARG(context);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PVCommsIONodePort::setParametersSync(PvmiMIOSession session, PvmiKvp* parameters,
        int num_elements, PvmiKvp*& ret_kvp)
{
    LOG_STACK_TRACE((0, "PVCommsIONodePort::setParametersSync"));

    MIOControlContextSet set;

    if (!iNode)
    {
        LOG_ERR((0, "PVCommsIONodePort::setParametersSync: Error - Config object for media IO not available"));
        ret_kvp = parameters;
        OSCL_LEAVE(OsclErrGeneral);
    }

    PvmiKvp *ret_kvp1 = NULL;
    PvmiKvp *ret_kvp2 = NULL;

    set = iNode->ContextSetFromTag(iTag);

    if (set.iMediaInputElement &&
            set.iMediaInputElement->hasConfig())
    {
        set.iMediaInputElement->iMediaIOConfig->setParametersSync(session, parameters, num_elements, ret_kvp1);
    }
    // if this is a proxy port, we don't really have any way of determining
    // which config we want to query, so we'll just try setting both of them
    if (set.iMediaOutputElement &&
            set.iMediaOutputElement != set.iMediaInputElement &&
            set.iMediaOutputElement->hasConfig())
    {
        set.iMediaOutputElement->iMediaIOConfig->setParametersSync(session, parameters, num_elements, ret_kvp);
    }
    // Since we aren't sure about which IO the request goes to we'll
    // have to settle for considering 1 successful setParam() a success

    if ((ret_kvp1 == NULL && ret_kvp2 == NULL) ||
            (set.iMediaInputElement && (ret_kvp1 == NULL)) ||
            (set.iMediaOutputElement && (ret_kvp2 == NULL)))
    {
        ret_kvp = ret_kvp1 ? ret_kvp1 : ret_kvp2;
    }
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVCommsIONodePort::setParametersAsync(PvmiMIOSession session,
        PvmiKvp* parameters,
        int num_elements,
        PvmiKvp*& ret_kvp,
        OsclAny* context)
{
    OSCL_UNUSED_ARG(session);
    OSCL_UNUSED_ARG(parameters);
    OSCL_UNUSED_ARG(num_elements);
    OSCL_UNUSED_ARG(ret_kvp);
    OSCL_UNUSED_ARG(context);
    OsclError::Leave(OsclErrNotSupported);
    return -1;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF uint32 PVCommsIONodePort::getCapabilityMetric(PvmiMIOSession session)
{
    OSCL_UNUSED_ARG(session);
    return 0;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVCommsIONodePort::verifyParametersSync(PvmiMIOSession session,
        PvmiKvp* parameters, int num_elements)
{
    LOG_STACK_TRACE((0, "PVCommsIONodePort::verifyParametersSync"));

    MIOControlContextSet set;

    if (!iNode)
    {
        LOG_ERR((0, "PVCommsIONodePort::verifyParametersSync: Error - Config object for media IO not available"));
        return PVMFFailure;
    }

    set = iNode->ContextSetFromTag(iTag);

    if (set.iMediaInputElement &&
            set.iMediaInputElement->hasConfig() &&
            (set.iMediaInputElement->iMediaIOConfig->verifyParametersSync(session, parameters, num_elements) == PVMFSuccess))
        return PVMFSuccess;

    // if this is a proxy port, we don't really have any way of determining
    // which config we want to query, so if the input query fails, we'll
    // try on the output config
    if (set.iMediaOutputElement &&
            set.iMediaOutputElement->hasConfig() &&
            (set.iMediaOutputElement->iMediaIOConfig->verifyParametersSync(session, parameters, num_elements) == PVMFSuccess))
        return PVMFSuccess;

    return PVMFFailure;
}

////////////////////////////////////////////////////////////////////////////
void PVCommsIONodePort::Run()
{
    if (iState == PVCommsIONodePort::PORT_STATE_BUFFERING)
        return;

#ifdef USE_COPY_BUFFER
    SendCopyBuffer();
#endif

    if (iReadAsyncCmds.size() > 0)
    {
        PvmiMediaXferHeader data_hdr;
        data_hdr.seq_num = 0;
        data_hdr.timestamp = 0;
        data_hdr.flags = 0;
        data_hdr.duration = 0;
        data_hdr.stream_id = 0;
        //writeAsyncID = iPeer->writeAsync(0, 0, data, bytesToRead, data_hdr);

        iMediaOutputTransfer->readComplete(PVMFSuccess,
                                           iReadAsyncCmds.begin()->iID,
                                           0,
                                           data_hdr,
                                           iReadAsyncCmds.begin()->iContext);
        iReadAsyncCmds.erase(iReadAsyncCmds.begin());
    }

    if (IS_OUTPUT_PORT && OutgoingMsgQueueSize() > 0 && !IsConnectedPortBusy())
    {
        //transfer data to connected port.
        PVMFStatus status = Send();
        if (status != PVMFSuccess)
            iNode->ReportErrorEvent(PVMFErrPortProcessing, (OsclAny*)status);
    }

    if (CanSendCommsData())
        SendCommsData();

    if (iNode->IsFlushPending())
    {
        if (IncomingMsgQueueSize() == 0 && OutgoingMsgQueueSize() == 0
                && iCleanupQueue.size() == 0)
            iNode->FlushComplete();
    }

    // Send EOS event
    if (EndOfData(PVMF_COMMSIO_NODE_INPUT_PORT_TAG))
    {
        iNode->ReportInfoEvent(PVMFInfoEndOfInputPortData, (OsclAny*)NULL);
        iEndOfInputPortDataReached = false;
    }

    if (EndOfData(PVMF_COMMSIO_NODE_OUTPUT_PORT_TAG))
    {
        iNode->ReportInfoEvent(PVMFInfoEndOfOutputPortData, (OsclAny*)NULL);
        iEndOfOutputPortDataReached = false;
    }

    //reschedule if necessary
    if ((OutgoingMsgQueueSize() > 0 && !IsConnectedPortBusy()) ||
            CanSendCommsData() ||
            iReadAsyncCmds.size() > 0)
    {
        RunIfNotReady();
    }
}

////////////////////////////////////////////////////////////////////////////
void PVCommsIONodePort::HandlePortActivity(const PVMFPortActivity& aActivity)
{
    switch (aActivity.iType)
    {
        case PVMF_PORT_ACTIVITY_OUTGOING_MSG:
            RunIfNotReady();
            break;

        case PVMF_PORT_ACTIVITY_CONNECTED_PORT_READY:
            //wakeup the AO when the connected port is
            //ready to accept data again.
            RunIfNotReady();
            break;

        case PVMF_PORT_ACTIVITY_INCOMING_MSG:
#ifdef USE_COPY_BUFFER
            CopyBuffer();
#else
            if (CanSendCommsData())
                RunIfNotReady();
#endif
            break;

        case PVMF_PORT_ACTIVITY_OUTGOING_QUEUE_READY:
            if (iWriteFailed)
            {
                iWriteFailed = false;
                //let the peer know they can try to write again.
                if (iPeer)
                    iPeer->statusUpdate(PVMI_MEDIAXFER_STATUS_WRITE);
            }
            break;

        default:
            break;
    }
}

bool PVCommsIONodePort::CanSendCommsData()
{
    return !iIncomingQueue.iQ.empty()
           && iNode->GetState() == EPVMFNodeStarted
           && iWriteState == EWriteOK
           && isActiveCommsWrite()
           && IS_INPUT_PORT;
}

void PVCommsIONodePort::SendCommsData()
{
    if (!CanSendCommsData()) return;


    PVMFSharedMediaMsgPtr aMsg;
    PVMFSharedMediaDataPtr aMediaData;
    uint32 fragment = 0;
    uint32 fragindex;

    // get the input message at the front of the queue.
    aMsg = iIncomingQueue.iQ.front();

    if (aMsg->getFormatID() == PVMF_MEDIA_CMD_EOS_FORMAT_ID)
    {
        iEndOfInputPortDataReached = true;
        // remove the message from the queue
        iIncomingQueue.iQ.pop();
        RunIfNotReady();
        return;
    }

    convertToPVMFMediaData(aMediaData, aMsg);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVCommsIOPort::SendCommsData Seq %d TS %d Rep 0x%x Count %d"
                     , aMediaData->getSeqNum()
                     , aMediaData->getTimestamp()
                     , aMediaData.GetRep()
                     , aMediaData.get_count()));

    OSCL_ASSERT(iWriteState == EWriteOK);

    if (iResend
            && aMediaData->getSeqNum() == iResendSeqNum)
    {
        fragment = iResendFragment;
        iResend = false;
    }

    for (fragindex = fragment; fragindex < aMediaData->getNumFragments();)
    {
        OsclRefCounterMemFrag frag;
        aMediaData->getMediaFragment(fragindex, frag);

        ++iWriteAsyncContext;
        iWriteState = EWriteBusy;
        int32 err;
        int32 cmdId = 0;

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVCommsIONodePort::SendCommsData Calling WriteAsync context 0x%x", iWriteAsyncContext));

        PvmiMediaXferHeader data_hdr;
        data_hdr.seq_num = aMediaData->getSeqNum();
        data_hdr.timestamp = aMediaData->getTimestamp();
        data_hdr.flags = 0;
        data_hdr.duration = 0;
        data_hdr.stream_id = 0;
        //writeAsyncID = iPeer->writeAsync(0, 0, data, bytesToRead, data_hdr);

        err = WriteAsync(cmdId, frag, data_hdr);

        if (err != OsclErrNone)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVCommsIONodePort::SendData WriteAsync Leave code %d", err));

            LogDatapath("MIO Component Busy!  Waiting on Status Update");

            //if a leave occurs in the writeAsync call, we suspend data
            //transfer until a statusUpdate call from the MIO component
            //tells us to resume.
            //this is not an error-- it's the normal flow control mechanism.
            iWriteState = EWriteWait;

            //save the data to re-send later.
            iResend = true;
            iResendFragment = fragindex;
            iResendSeqNum = aMediaData->getSeqNum();

            // leave the message on front of the queue and return

            return ;//wait on statusUpdate call from the MIO component.
        }
        else
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVCommsIONodePort::SendData WriteAsync returned cmdId %d", cmdId));

            fragindex++;
            if (fragindex == aMediaData->getNumFragments())
            {
                //all fragments have been sent.  see whether completion
                //is synchronous or asynchronous.
                if (iWriteState == EWriteBusy)
                {
                    //asynchronous completion.
                    //push the data onto the cleanup stack so it won't get cleaned
                    //up until the component consumes it.
                    iCleanupQueue.push_back(CleanupQueueElement(aMediaData, cmdId));

                    LogMediaDataInfo("Async Write"
                                     , aMediaData
                                     , cmdId
                                     , iCleanupQueue.size()
                                    );

                }
                //else the write already completed synchronously.
                else
                {
                    LogMediaDataInfo("Sync Write"
                                     , aMediaData
                                    );
                }
            }
            iWriteState = EWriteOK;
        }
    }

    // remove the message from the fron of the queue
    iIncomingQueue.iQ.pop();

}

int32 PVCommsIONodePort::WriteAsync(int32& cmdId,
                                    OsclRefCounterMemFrag frag,
                                    PvmiMediaXferHeader data_hdr)
{
    int32 err = 0;
    OSCL_TRY(err,
             cmdId = iMediaOutputTransfer->writeAsync(PVMI_MEDIAXFER_FMT_TYPE_DATA,  /*format_type*/
                     PVMI_MEDIAXFER_FMT_INDEX_DATA, /*format_index*/
                     (uint8*)frag.getMemFragPtr(),
                     frag.getMemFragSize(),
                     data_hdr,
                     (OsclAny*)iWriteAsyncContext);
            );
    return err;
}

//for logging media data info
void PVCommsIONodePort::LogMediaDataInfo(const char*msg, PVMFSharedMediaDataPtr mediaData)
{
    OSCL_UNUSED_ARG(msg);
    OSCL_UNUSED_ARG(mediaData);
    LOGDATAPATH(
        (0, "MOUT %s %s MediaData SeqNum %d, SId %d, TS %d"
         , PortName()
         , msg
         , mediaData->getSeqNum()
         , mediaData->getStreamID()
         , mediaData->getTimestamp()
        ));
}
//for logging media data info plus write ID and cleanup q depth
void PVCommsIONodePort::LogMediaDataInfo(const char*msg, PVMFSharedMediaDataPtr mediaData, int32 cmdid, int32 qdepth)
{
    OSCL_UNUSED_ARG(msg);
    OSCL_UNUSED_ARG(mediaData);
    OSCL_UNUSED_ARG(cmdid);
    OSCL_UNUSED_ARG(qdepth);
    LOGDATAPATH(
        (0, "MOUT %s %s, Write Id %d, MediaData SeqNum %d, SId %d, TS %d, Cleanup Q-depth %d"
         , PortName()
         , msg
         , cmdid
         , mediaData->getSeqNum()
         , mediaData->getStreamID()
         , mediaData->getTimestamp()
         , qdepth
        ));
}
//for logging media xfer info
void PVCommsIONodePort::LogDatapath(const char*msg)
{
    OSCL_UNUSED_ARG(msg);
    LOGDATAPATH(
        (0, "MOUT %s %s"
         , PortName()
         , msg
        ));
}

void PVCommsIONodePort::statusUpdate(uint32 status_flags)
//for PvmiMediaTransfer
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO,
                    (0, "PVCommsIONodePort::statusUpdate flags %d", status_flags));

    if (status_flags & PVMI_MEDIAXFER_STATUS_WRITE)
    {
        //recover from a previous async write error.
        if (iWriteState == EWriteWait)
        {
            LogDatapath("Data Transfer re-enabled");
            iWriteState = EWriteOK;
            RunIfNotReady();
        }
    }
    else
    {
        //disable write
        iWriteState = EWriteWait;
    }
}

OSCL_EXPORT_REF bool PVCommsIONodePort::isIncomingFull()
// override the PvmfPortBaseImpl routine to impose a
// limit on the total buffers in this node.
// Since we are doing this we also need to call EvaluateIncomingBusy
// when any data is consumed.
{
    return (iIncomingQueue.iQ.size()
            + iCleanupQueue.size()) >= iQueueLimit;
}

////////////////////////////////////////////////////////////////////////////
bool PVCommsIONodePort::EndOfData(int32 aPortTag)
{
    //this is really just necessary for the unit test,
    //which uses file I/O.
    //end of data is reached when we have received the end-of-data
    //command in the media data and all data is played out.
    if (aPortTag == PVMF_COMMSIO_NODE_INPUT_PORT_TAG)
    {
        return (iEndOfInputPortDataReached
                && iIncomingQueue.iQ.empty()
                && iCleanupQueue.empty());
    }
    else
    {
        return (iEndOfOutputPortDataReached
                && iOutgoingQueue.iQ.empty());
    }
}

void PVCommsIONodePort::SetDataTransferModels()
{
    // This will determine how the port writes
    // and writes data to its mediaDataTransfer
    // object.  Default is active writing to media
    // object, and passive reading from media object

    PvmiKvp* parameters = NULL;
    int num_parameter_elements = 0;
    PvmiCapabilityContext context = NULL;
    MIOControlContextSet set;

    set = iNode->ContextSetFromTag(iTag);

    if (set.iMediaInputElement &&
            set.iMediaInputElement->hasConfig() &&
            set.iMediaInputElement->iMediaIOConfig->getParametersSync(set.iMediaInputElement->iMediaSession,
                    OSCL_CONST_CAST(char*, OUTPUT_TRANSFER_MODEL_VAL),
                    parameters, num_parameter_elements, context) == PVMFSuccess)
    {
        if (num_parameter_elements == 1)
        {
            // In the future we will be able to handle
            // both push and pull for input from the
            // MIO object, but for now we only want to
            // do push input, meaning the object
            // calls our writeAsync() method to send data
            // to us.
            OSCL_ASSERT(parameters[0].value.uint32_value == 1);
            iActiveCommsRead = false;
        }
        set.iMediaInputElement->iMediaIOConfig->releaseParameters(set.iMediaInputElement->iMediaSession, parameters, num_parameter_elements);
    }
    num_parameter_elements = 0;
    parameters = NULL;
    if (set.iMediaOutputElement &&
            set.iMediaOutputElement->hasConfig() &&
            set.iMediaOutputElement->iMediaIOConfig->getParametersSync(set.iMediaOutputElement->iMediaSession,
                    OSCL_CONST_CAST(char*, INPUT_TRANSFER_MODEL_VAL),
                    parameters, num_parameter_elements, context) == PVMFSuccess)
    {
        if (num_parameter_elements == 1)
        {
            // In the future we will be able to handle
            // both push and pull for output to the
            // MIO object, but for now we only want to
            // do push output, meaning we will call
            // the object's writeAsync() method to send
            // data to it.
            OSCL_ASSERT(parameters[0].value.uint32_value == 1);
            iActiveCommsWrite = true;
        }
        set.iMediaOutputElement->iMediaIOConfig->releaseParameters(set.iMediaOutputElement->iMediaSession, parameters, num_parameter_elements);
    }
}

#ifdef USE_COPY_BUFFER
void PVCommsIONodePort::CopyBuffer()
{
    PVMFSharedMediaMsgPtr aMsg;
    PVMFSharedMediaDataPtr aMediaData;

    while (IncomingMsgQueueSize())
    {
        DequeueIncomingMsg(aMsg);
        convertToPVMFMediaData(aMediaData, aMsg);

        for (uint32 fragindex = 0; fragindex < aMediaData->getNumFragments(); fragindex++)
        {
            OsclRefCounterMemFrag frag;
            aMediaData->getMediaFragment(fragindex, frag);

            //If going to overflow flush data
            if ((frag.getMemFragSize() + iCopyBufferSize[iCopyBufferIndex % NUM_COPY_BUFFERS]) > COPY_BUFFER_SIZE)
            {
                iCopyBufferSize[iCopyBufferIndex % NUM_COPY_BUFFERS] = 0;
            }

            //If frag is too large, drop it
            if (frag.getMemFragSize() > COPY_BUFFER_SIZE)
            {
                continue;
            }

            oscl_memcpy(&iCopyBuffer[iCopyBufferIndex % NUM_COPY_BUFFERS][iCopyBufferSize[iCopyBufferIndex % NUM_COPY_BUFFERS]],
                        frag.getMemFragPtr(),
                        frag.getMemFragSize());

            iCopyBufferSize[iCopyBufferIndex % NUM_COPY_BUFFERS] += frag.getMemFragSize();
        }

        SendCopyBuffer();
    }
}

void PVCommsIONodePort::SendCopyBuffer()
{
    int32 err;
    int32 cmdId;

    if (!iCopyBufferSent && (iCopyBufferSize[iCopyBufferIndex % NUM_COPY_BUFFERS] > 0))
    {
        PvmiMediaXferHeader data_hdr;
        data_hdr.seq_num = iCopyBufferIndex;
        data_hdr.timestamp = iCopyBufferIndex;
        data_hdr.flags = 0;
        data_hdr.duration = 0;
        data_hdr.stream_id = 0;
        //writeAsyncID = iPeer->writeAsync(0, 0, data, bytesToRead, data_hdr);

        OSCL_TRY(err,
                 cmdId = iMediaOutputTransfer->writeAsync(PVMI_MEDIAXFER_FMT_TYPE_DATA,  /*format_type*/
                         PVMI_MEDIAXFER_FMT_INDEX_DATA, /*format_index*/
                         (uint8*) & iCopyBuffer[iCopyBufferIndex % NUM_COPY_BUFFERS][0],
                         iCopyBufferSize[iCopyBufferIndex % NUM_COPY_BUFFERS],
                         data_hdr,
                         (OsclAny*)iWriteAsyncContext);
                );

        if (err)
        {
            iCopyBufferSize[iCopyBufferIndex % NUM_COPY_BUFFERS] = 0;
        }
        else
        {
            iCopyBufferIndex++;
            iCopyBufferSent = true;
        }
    }
}
#endif



