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
 * @file pvmf_media_input_node_outport.cpp
 * @brief Output port for media io interface wrapper node
 */

#ifndef PVMF_MEDIA_INPUT_NODE_OUTPORT_H_INCLUDED
#include "pvmf_media_input_node_outport.h"
#endif
#ifndef PVMF_MEDIA_INPUT_NODE_H_INCLUDED
#include "pvmf_media_input_node.h"
#endif
#ifndef PVMF_FORMAT_TYPE_H_INCLUDED
#include "pvmf_format_type.h"
#endif

#ifdef TEXT_TRACK_DESC_INFO
#ifndef TEXTSAMPLEDESCINFO_H
#include "textsampledescinfo.h"
#endif
#endif

#ifndef PVMF_MEDIA_MSG_FORMAT_IDS_H_INCLUDED
#include "pvmf_media_msg_format_ids.h"
#endif
#ifndef PVMF_MEDIA_CMD_H_INCLUDED
#include "pvmf_media_cmd.h"
#endif

#define PVMIO_MEDIADATA_POOLNUM 9

// Logging macros
#define LOG_STACK_TRACE(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, m);
#define LOG_DEBUG(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, m);
#define LOG_ERR(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iLogger,PVLOGMSG_ERR,m);
#define LOGDATATRAFFIC(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iDataPathLogger,PVLOGMSG_INFO,m);

////////////////////////////////////////////////////////////////////////////
PvmfMediaInputNodeOutPort::PvmfMediaInputNodeOutPort(PvmfMediaInputNode* aNode, const char* aName)
        : OsclTimerObject(OsclActiveObject::EPriorityNominal, "PvmfMediaInputNodeOutPort")
        , PvmfPortBaseImpl(PVMF_MEDIAIO_NODE_OUTPUT_PORT_TAG
                           //this port handles its own port activity
                           , this, aName)
        , iNode(aNode)
        , iState(PvmfMediaInputNodeOutPort::PORT_STATE_BUFFERING)
        , inum_text_sample(0)
        , imax_num_sample(0)
{
    iCmdId = 0;
    AddToScheduler();
    iFormatType = PVMF_MIME_FORMAT_UNKNOWN;
    iPeer = NULL;
    iWriteState = EWriteOK;
    iMediaDataAllocMemPool = OSCL_NEW(OsclMemPoolFixedChunkAllocator, (PVMIO_MEDIADATA_POOLNUM));
    iMediaDataMemPool = OSCL_NEW(OsclMemPoolFixedChunkAllocator, (PVMIO_MEDIADATA_POOLNUM));
    iMediaDataAlloc = OSCL_NEW(PvmfMediaInputDataBufferAlloc, (iMediaDataAllocMemPool));
    iDataPathLogger = PVLogger::GetLoggerObject("datapath.sourcenode");
#ifdef _TEST_AE_EROR_HANDLING
    iTimeStampJunk = 0x000FFFFF;
#endif
}

////////////////////////////////////////////////////////////////////////////
PvmfMediaInputNodeOutPort::~PvmfMediaInputNodeOutPort()
{
    PvmfPortBaseImpl::ClearMsgQueues();


    if (iMediaDataAlloc != NULL)
    {
        OSCL_DELETE(iMediaDataAlloc);
    }

    if (iMediaDataAllocMemPool)
    {
        iMediaDataAllocMemPool->removeRef();
    }

    if (iMediaDataMemPool)
    {
        iMediaDataMemPool->removeRef();
    }
    iDataPathLogger = NULL;
}

////////////////////////////////////////////////////////////////////////////
void PvmfMediaInputNodeOutPort::Start()
{
    iWriteState = EWriteOK;
#ifdef _TEST_AE_ERROR_HANDLING

    if (iNode->iError_Out_Queue_Busy)
    {
        PvmfPortBaseImpl::SetCapacity(EPVIncomingDataQueue, 0);
        PvmfPortBaseImpl::SetReserve(EPVIncomingDataQueue, 0);
        PvmfPortBaseImpl::SetThreshold(EPVIncomingDataQueue, 0);
        PvmfPortBaseImpl::SetCapacity(EPVOutgoingDataQueue, 1);
        PvmfPortBaseImpl::SetReserve(EPVOutgoingDataQueue, 2);
        PvmfPortBaseImpl::SetThreshold(EPVOutgoingDataQueue, 100);
    }
    else
    {
        PvmfPortBaseImpl::SetCapacity(EPVIncomingDataQueue, 0);
        PvmfPortBaseImpl::SetReserve(EPVIncomingDataQueue, 0);
        PvmfPortBaseImpl::SetThreshold(EPVIncomingDataQueue, 0);
        PvmfPortBaseImpl::SetCapacity(EPVOutgoingDataQueue, 10);
        PvmfPortBaseImpl::SetReserve(EPVOutgoingDataQueue, 10);
        PvmfPortBaseImpl::SetThreshold(EPVOutgoingDataQueue, 70);
    }
#else
    PvmfPortBaseImpl::SetCapacity(EPVIncomingDataQueue, 0);
    PvmfPortBaseImpl::SetReserve(EPVIncomingDataQueue, 0);
    PvmfPortBaseImpl::SetThreshold(EPVIncomingDataQueue, 0);
    PvmfPortBaseImpl::SetCapacity(EPVOutgoingDataQueue, 10);
    PvmfPortBaseImpl::SetReserve(EPVOutgoingDataQueue, 10);
    PvmfPortBaseImpl::SetThreshold(EPVOutgoingDataQueue, 70);

#endif
    iState = PvmfMediaInputNodeOutPort::PORT_STATE_STARTED;
    if (iNode->iMediaIOState == PvmfMediaInputNode::MIO_STATE_STARTED)
    {
        RunIfNotReady();
    }
}

////////////////////////////////////////////////////////////////////////////
void PvmfMediaInputNodeOutPort::MediaIOStarted()
{
    if (iState == PvmfMediaInputNodeOutPort::PORT_STATE_STARTED)
        RunIfNotReady();
}

////////////////////////////////////////////////////////////////////////////
void PvmfMediaInputNodeOutPort::Pause()
{
    iState = PvmfMediaInputNodeOutPort::PORT_STATE_BUFFERING;
}

////////////////////////////////////////////////////////////////////////////
void PvmfMediaInputNodeOutPort::Stop()
{
    iWriteState = EWriteOK;
    PvmfPortBaseImpl::ClearMsgQueues();
    iState = PvmfMediaInputNodeOutPort::PORT_STATE_BUFFERING;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PvmfMediaInputNodeOutPort::Configure(PVMFFormatType aPortProperty,
        OSCL_String* aMime)
{
    if (iConnectedPort)
    {
        // Must disconnect before changing port properties, so return error
        return PVMFFailure;
    }

    iFormatType = aPortProperty;
    iMimeType = *aMime;
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PvmfMediaInputNodeOutPort::Connect(PVMFPortInterface* aPort)
{
    PVMFStatus status = PvmfPortBaseImpl::Connect(aPort);
    if (status != PVMFSuccess)
        return status;

    iMediaInput = iNode->iMediaIOControl->createMediaTransfer(iNode->iMediaIOSession);
    if (iMediaInput)
        iMediaInput->setPeer(this);
    else
        return PVMFFailure;

    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PvmfMediaInputNodeOutPort::PeerConnect(PVMFPortInterface* aPort)
{
    PVMFStatus status = PvmfPortBaseImpl::PeerConnect(aPort);
    if (status != PVMFSuccess)
        return status;

    iMediaInput = iNode->iMediaIOControl->createMediaTransfer(iNode->iMediaIOSession);
    if (iMediaInput)
        iMediaInput->setPeer(this);
    else
        return PVMFFailure;

    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PvmfMediaInputNodeOutPort::Disconnect()
{
    PVMFStatus status = PvmfPortBaseImpl::Disconnect();
    if (status != PVMFSuccess)
        return status;

    iNode->iMediaIOControl->deleteMediaTransfer(iNode->iMediaIOSession, iMediaInput);
    if (iMediaInput)
        iMediaInput->setPeer(0);
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PvmfMediaInputNodeOutPort::PeerDisconnect()
{
    PVMFStatus status = PvmfPortBaseImpl::PeerDisconnect();
    if (status != PVMFSuccess)
        return status;

    if (iMediaInput)
        iMediaInput->setPeer(0);
    iNode->iMediaIOControl->deleteMediaTransfer(iNode->iMediaIOSession, iMediaInput);
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
void PvmfMediaInputNodeOutPort::setPeer(PvmiMediaTransfer *aPeer)
{
    iPeer = aPeer;
}

////////////////////////////////////////////////////////////////////////////
void PvmfMediaInputNodeOutPort::useMemoryAllocators(OsclMemAllocator* write_alloc)
{
    OSCL_UNUSED_ARG(write_alloc);
    OSCL_LEAVE(OsclErrNotSupported);
}

////////////////////////////////////////////////////////////////////////////
PVMFCommandId PvmfMediaInputNodeOutPort::writeAsync(uint8 format_type, int32 format_index,
        uint8* data, uint32 data_len,
        const PvmiMediaXferHeader& data_header_info,
        OsclAny* context)
{
    if (iWriteState == EWriteOK)
    {
        if (PVMI_MEDIAXFER_FMT_TYPE_NOTIFICATION == format_type)
        {
            switch (format_index)
            {
                    //added for timed text support
                    //here it handles the configuration information for timed text and passes it to the composernode

                case PVMI_MEDIAXFER_FMT_INDEX_FMT_SPECIFIC_INFO:
                {
#ifdef TEXT_TRACK_DESC_INFO

                    if (iFormatType == PVMF_MIME_3GPP_TIMEDTEXT)
                    {
                        //get indexing info to map text data with config info
                        PvmiKvp* textKvp = OSCL_STATIC_CAST(PvmiKvp*, data);
                        PVA_FF_TextSampleDescInfo* pDecoderinfo;
                        pDecoderinfo = OSCL_STATIC_CAST(PVA_FF_TextSampleDescInfo*, textKvp->value.key_specific_value);

                        itext_sample_index.push_back(pDecoderinfo->sdindex);
                        istart_text_sample.push_back(pDecoderinfo->start_sample_num);
                        iend_text_sample.push_back(pDecoderinfo->end_sample_num);
                        imax_num_sample = pDecoderinfo->end_sample_num + 1;
                    }

                    uint32 cmdId = *(OSCL_STATIC_CAST(uint32*, context));

                    OsclAny* temp = NULL;
                    iConnectedPort->QueryInterface(PVMI_CAPABILITY_AND_CONFIG_PVUUID, temp);
                    PvmiCapabilityAndConfig *config = OSCL_STATIC_CAST(PvmiCapabilityAndConfig*, temp);

                    PvmiKvp* mioKvp = OSCL_STATIC_CAST(PvmiKvp*, data);
                    PvmiKvp* ret_kvp = NULL;
                    config->setParametersSync(NULL, mioKvp, 1, ret_kvp);
                    iMediaInput->writeComplete(PVMFSuccess, cmdId, NULL);
                    return 0;

#endif
                }
                break;
                case PVMI_MEDIAXFER_FMT_INDEX_END_OF_STREAM:
                {

                    SendEndOfTrackCommand(data_header_info);
                    MediaIOStarted();

                    return iCmdId++;
                }
                break;
                default:
                {
                    LOG_DEBUG((0, "Ignoring Format Index :%d since not supported\n", format_index));

                    iNode->ReportErrorEvent(PVMFErrPortProcessing, (OsclAny*)NULL);
                    OsclError::Leave(OsclErrGeneral);

                }

            }

        }

        // TODO: Handle incoming data here. Create a media data using PvmiMIOSourceDataBufferAlloc::allocate,
        // save the data there, put the media data to outgoing queue.
        // If the port is started, schedule to send in Run

        else if (PVMI_MEDIAXFER_FMT_TYPE_DATA == format_type)
        {
            //if the outgoing queue is full, we can't accept data
            //now.

            if (IsOutgoingQueueBusy())
            {
                iWriteState = EWriteBusy;
                OsclError::Leave(OsclErrBusy);
            }

            // Create new media data buffer
            PVMFSharedMediaDataPtr mediaData;
            int32 err = 0;

            if (iCmdId == 0x7FFFFFFF)
                iCmdId = 0;
#ifdef _TEST_AE_ERROR_HANDLING

            if (data_header_info.stream_id == iNode->iTrackID)
            {
                uint32 ii = 0;
                while (iNode->iChunkCount > 0)
                {
                    uint32 sz = data_len - 5;

                    for (ii = 0; ii <= sz; ii++)
                    {
                        uint8* ptr = data + ii;
                        *ptr = 0;
                    }

                    iNode->iChunkCount--;
                }

            }
#endif
#ifdef _TEST_AE_ERROR_HANDLING
            if (iNode->iError_No_Memory)
            {
                err = OsclErrBusy;
                if (IsAdded())
                {
                    RunIfNotReady();
                }
            }
            else
            {
                OSCL_TRY(err,
                         OsclSharedPtr<PVMFMediaDataImpl> mediaDataImpl = iMediaDataAlloc->allocate(iMediaInput, data,
                                 data_len, iCmdId, context);
                         mediaData = PVMFMediaData::createMediaData(mediaDataImpl, iMediaDataMemPool););
            }
#else
            OSCL_TRY(err,
                     OsclSharedPtr<PVMFMediaDataImpl> mediaDataImpl = iMediaDataAlloc->allocate(iMediaInput, data,
                             data_len, iCmdId, context);
                     mediaData = PVMFMediaData::createMediaData(mediaDataImpl, iMediaDataMemPool););


#endif
            if (err)
            {
                iMediaDataAllocMemPool->notifyfreechunkavailable(*this);
                iWriteState = EWriteBusy;
                OsclError::Leave(OsclErrBusy);
            }
            // Set timestamp
#ifdef _TEST_AE_ERROR_HANDLING

            if ((data_header_info.stream_id == iNode->iErrorTimeStamp.track_no) && (1 == iNode->iErrorTimeStamp.mode))
            {

                if (iTimeStampJunk == 0)
                {
                    iTimeStampJunk = 0x000FFFFF;
                }

                mediaData->setTimestamp(iTimeStampJunk);
                iTimeStampJunk = iTimeStampJunk - 33;
            }
            else if ((data_header_info.stream_id == iNode->iErrorTimeStamp.track_no) && (2 == iNode->iErrorTimeStamp.mode))
            {
                if (data_header_info.timestamp >= iNode->iErrorTimeStamp.duration)
                {
                    mediaData->setTimestamp(0);
                }
            }
            else if ((data_header_info.stream_id == iNode->iErrorTimeStamp.track_no) && (3 == iNode->iErrorTimeStamp.mode))
            {
                mediaData->setTimestamp(0);
            }
            else
            {
                mediaData->setTimestamp(data_header_info.timestamp);
            }
#else
            mediaData->setTimestamp(data_header_info.timestamp);
#endif
            mediaData->setSeqNum(data_header_info.seq_num);
            mediaData->setMediaFragFilledLen(0, data_len);
            mediaData->setStreamID(data_header_info.stream_id);

            LOGDATATRAFFIC((0, "PvmfMediaInputNodeOutPort::writeAsync:"
                            "StreamID=%d, TS=%d, Len=%d, SN=%d, MimeType=%s",
                            data_header_info.stream_id,  data_header_info.timestamp, data_len,
                            data_header_info.seq_num, iMimeType.get_cstr()));
            if (itext_sample_index.size())
            {
                bool found = false;
                OsclMemAllocDestructDealloc<uint8> my_alloc;
                OsclRefCounter* my_refcnt;
                uint aligned_refcnt_size = oscl_mem_aligned_size(sizeof(OsclRefCounterSA< OsclMemAllocDestructDealloc<uint8> >));
                uint8* my_ptr = (uint8*) my_alloc.allocate(aligned_refcnt_size + sizeof(int32));
                my_refcnt = OSCL_PLACEMENT_NEW(my_ptr, OsclRefCounterSA< OsclMemAllocDestructDealloc<uint8> >(my_ptr));
                my_ptr += aligned_refcnt_size;

                OsclMemoryFragment memfrag;
                memfrag.len = 0;
                memfrag.ptr = my_ptr;

                memfrag.len = sizeof(int32);

                if (inum_text_sample >= imax_num_sample)
                {
                    while (inum_text_sample >= imax_num_sample)
                    {
                        inum_text_sample = inum_text_sample - imax_num_sample;
                    }
                }
                for (uint32 ii = 0; ii < itext_sample_index.size(); ii++)
                {
                    if (inum_text_sample >= istart_text_sample[ii] && inum_text_sample <= iend_text_sample[ii])
                    {
                        found = true;
                        memfrag.ptr = &(itext_sample_index[ii]);//vector stores the sample index no.
                    }//this index no. gives the sample description index information
                    if (found)
                    {
                        break;
                    }
                }

                inum_text_sample += 1;
                // Save format specific info
                OsclRefCounterMemFrag configinfo(memfrag, my_refcnt, sizeof(int32));
                iFormatSpecificInfo = configinfo;
                mediaData->setFormatSpecificInfo(iFormatSpecificInfo);
            }
            // Convert media data to MediaMsg
            PVMFSharedMediaMsgPtr mediaMsg;
            convertToPVMFMediaMsg(mediaMsg, mediaData);
            PVMFStatus status;
#ifdef _TEST_AE_ERROR_HANDLING

            if ((iNode->iErrorTrackID > 0) && (data_header_info.stream_id == (uint32)iNode->iErrorTrackID))
            {
                status = PVMFSuccess;
            }
            else
            {
                status = QueueOutgoingMsg(mediaMsg);
            }
#else
            status = QueueOutgoingMsg(mediaMsg);

#endif
            if (status != PVMFSuccess)
            {
                iNode->ReportErrorEvent(PVMFErrPortProcessing, (OsclAny*)status);
                OsclError::Leave(OsclErrGeneral);
            }

            if (iState == PvmfMediaInputNodeOutPort::PORT_STATE_STARTED)
                RunIfNotReady();

            return iCmdId++;
        }
        else
        {
            LOG_DEBUG((0, "Ignoring Format Type :%d since not supported\n", format_type));
            iNode->ReportErrorEvent(PVMFErrPortProcessing, (OsclAny*)NULL);
            OsclError::Leave(OsclErrGeneral);
        }
    }
    else
    {
        OsclError::Leave(OsclErrBusy);
    }

    return 0;

}

////////////////////////////////////////////////////////////////////////////
void PvmfMediaInputNodeOutPort::writeComplete(PVMFStatus status, PVMFCommandId write_cmd_id, OsclAny* context)
{
    OSCL_UNUSED_ARG(context);
    OSCL_UNUSED_ARG(status);
    OSCL_UNUSED_ARG(write_cmd_id);
    OSCL_LEAVE(OsclErrNotSupported);
}

////////////////////////////////////////////////////////////////////////////
PVMFCommandId PvmfMediaInputNodeOutPort::readAsync(uint8* data, uint32 max_data_len, OsclAny* context,
        int32* formats, uint16 num_formats)
{
    OSCL_UNUSED_ARG(data);
    OSCL_UNUSED_ARG(max_data_len);
    OSCL_UNUSED_ARG(context);
    OSCL_UNUSED_ARG(formats);
    OSCL_UNUSED_ARG(num_formats);
    OSCL_LEAVE(OsclErrNotSupported);
    return -1;
}

////////////////////////////////////////////////////////////////////////////
void PvmfMediaInputNodeOutPort::readComplete(PVMFStatus status, PVMFCommandId read_cmd_id,
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
void PvmfMediaInputNodeOutPort::statusUpdate(uint32 status_flags)
{
    OSCL_UNUSED_ARG(status_flags);
}

////////////////////////////////////////////////////////////////////////////
void PvmfMediaInputNodeOutPort::cancelCommand(PVMFCommandId command_id)
{
    OSCL_UNUSED_ARG(command_id);
    OSCL_LEAVE(OsclErrNotSupported);
}

////////////////////////////////////////////////////////////////////////////
void PvmfMediaInputNodeOutPort::cancelAllCommands()
{
    OSCL_LEAVE(OsclErrNotSupported);
}

////////////////////////////////////////////////////////////////////////////
//                  PvmiCapabilityAndConfig
////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PvmfMediaInputNodeOutPort::setObserver(PvmiConfigAndCapabilityCmdObserver* aObserver)
{
    // Not supported
    OSCL_UNUSED_ARG(aObserver);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PvmfMediaInputNodeOutPort::getParametersSync(PvmiMIOSession session,
        PvmiKeyType identifier,
        PvmiKvp*& parameters,
        int& num_parameter_elements,
        PvmiCapabilityContext context)
{
    LOG_STACK_TRACE((0, "PvmfMediaInputNodeOutPort::getParametersSync"));

    if (!iNode || !iNode->iMediaIOConfig)
    {
        LOG_ERR((0, "PvmfMediaInputNodeOutPort::getParametersSync: Error - Config object for media IO not available"));
        return PVMFFailure;

    }

    return iNode->iMediaIOConfig->getParametersSync(session, identifier, parameters,
            num_parameter_elements, context);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PvmfMediaInputNodeOutPort::releaseParameters(PvmiMIOSession session,
        PvmiKvp* parameters,
        int num_elements)
{
    LOG_STACK_TRACE((0, "PvmfMediaInputNodeOutPort::releaseParameters"));

    if (!iNode || !iNode->iMediaIOConfig)
    {
        LOG_ERR((0, "PvmfMediaInputNodeOutPort::releaseParameters: Error - Config object for media IO not available"));
        return PVMFFailure;

    }

    return iNode->iMediaIOConfig->releaseParameters(session, parameters, num_elements);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PvmfMediaInputNodeOutPort::createContext(PvmiMIOSession session, PvmiCapabilityContext& context)
{
    OSCL_UNUSED_ARG(session);
    OSCL_UNUSED_ARG(context);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PvmfMediaInputNodeOutPort::setContextParameters(PvmiMIOSession session,
        PvmiCapabilityContext& context,
        PvmiKvp* parameters, int num_parameter_elements)
{
    OSCL_UNUSED_ARG(session);
    OSCL_UNUSED_ARG(context);
    OSCL_UNUSED_ARG(parameters);
    OSCL_UNUSED_ARG(num_parameter_elements);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PvmfMediaInputNodeOutPort::DeleteContext(PvmiMIOSession session, PvmiCapabilityContext& context)
{
    OSCL_UNUSED_ARG(session);
    OSCL_UNUSED_ARG(context);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PvmfMediaInputNodeOutPort::setParametersSync(PvmiMIOSession session, PvmiKvp* parameters,
        int num_elements, PvmiKvp*& ret_kvp)
{
    LOG_STACK_TRACE((0, "PvmfMediaInputNodeOutPort::setParametersSync"));

    if (!iNode || !iNode->iMediaIOConfig)
    {
        LOG_ERR((0, "PvmfMediaInputNodeOutPort::setParametersSync: Error - Config object for media IO not available"));
        ret_kvp = parameters;
        OSCL_LEAVE(OsclErrGeneral);
    }

    iNode->iMediaIOConfig->setParametersSync(session, parameters, num_elements, ret_kvp);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PvmfMediaInputNodeOutPort::setParametersAsync(PvmiMIOSession session,
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
OSCL_EXPORT_REF uint32 PvmfMediaInputNodeOutPort::getCapabilityMetric(PvmiMIOSession session)
{
    OSCL_UNUSED_ARG(session);
    return 0;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PvmfMediaInputNodeOutPort::verifyParametersSync(PvmiMIOSession session,
        PvmiKvp* parameters, int num_elements)
{
    LOG_STACK_TRACE((0, "PvmfMediaInputNodeOutPort::verifyParametersSync"));

    if (!iNode || !iNode->iMediaIOConfig)
    {
        LOG_ERR((0, "PvmfMediaInputNodeOutPort::verifyParametersSync: Error - Config object for media IO not available"));
        return PVMFFailure;
    }

    return iNode->iMediaIOConfig->verifyParametersSync(session, parameters, num_elements);
}

////////////////////////////////////////////////////////////////////////////
void PvmfMediaInputNodeOutPort::Run()
{
    if (iState == PvmfMediaInputNodeOutPort::PORT_STATE_BUFFERING)
        return;

#ifdef _TEST_AE_ERROR_HANDLING
    if (iNode->iError_No_Memory)
    {
        if (iMediaInput && iWriteState == EWriteBusy)
        {
            iWriteState = EWriteOK;
            iMediaInput->statusUpdate(PVMI_MEDIAXFER_STATUS_WRITE);
            iNode->iError_No_Memory = false;
        }
    }
#endif
    if ((OutgoingMsgQueueSize() > 0) && (!IsConnectedPortBusy()))
    {
        //transfer data to connected port.
        PVMFStatus status = Send();
        if (status != PVMFSuccess)
            iNode->ReportErrorEvent(PVMFErrPortProcessing, (OsclAny*)status);

        //reschedule as long as there's data queued...
        if (OutgoingMsgQueueSize() > 0
                && !IsConnectedPortBusy())
        {
            RunIfNotReady();
        }

        if (iNode->IsFlushPending())
        {
            if (IncomingMsgQueueSize() == 0 && OutgoingMsgQueueSize() == 0)
                iNode->FlushComplete();
        }
    }
}

////////////////////////////////////////////////////////////////////////////
void PvmfMediaInputNodeOutPort::HandlePortActivity(const PVMFPortActivity& aActivity)
{
    switch (aActivity.iType)
    {
        case PVMF_PORT_ACTIVITY_CONNECT:
        {
            //get the mimetype
            OsclAny* temp = NULL;
            iConnectedPort->QueryInterface(PVMI_CAPABILITY_AND_CONFIG_PVUUID, temp);
            PvmiCapabilityAndConfig *config = OSCL_STATIC_CAST(PvmiCapabilityAndConfig*, temp);

            if (config != NULL)
            {
                int numKvp = 0;
                PvmiKvp* kvpPtr = NULL;
                PVMFStatus status =
                    config->getParametersSync(NULL,
                                              (char*)INPUT_FORMATS_CUR_QUERY, kvpPtr, numKvp, NULL);
                if (status == PVMFSuccess)
                {
                    iFormatType = kvpPtr[0].value.pChar_value;
                    iMimeType = iFormatType.getMIMEStrPtr();
                }
                config->releaseParameters(NULL, kvpPtr, numKvp);
            }
        }
        break;

        case PVMF_PORT_ACTIVITY_DISCONNECT:
            break;

        case PVMF_PORT_ACTIVITY_OUTGOING_MSG:
            //wakeup the AO when the first message arrives.
            if (OutgoingMsgQueueSize() == 1)
                RunIfNotReady();
            break;

        case PVMF_PORT_ACTIVITY_CONNECTED_PORT_READY:
            //wakeup the AO when the connected port is
            //ready to accept data again.
            RunIfNotReady();
            break;

        case PVMF_PORT_ACTIVITY_OUTGOING_QUEUE_READY:
            if (iMediaInput && (iWriteState == EWriteBusy))
            {
                iWriteState = EWriteOK;
                //let the peer know they can try to write again.
                iMediaInput->statusUpdate(PVMI_MEDIAXFER_STATUS_WRITE);
            }
            break;
        case PVMF_PORT_ACTIVITY_OUTGOING_QUEUE_BUSY:
        {
            iWriteState = EWriteBusy;
        }
        break;
        case PVMF_PORT_ACTIVITY_CONNECTED_PORT_BUSY:
            break;
        default:
            break;
    }
}

////////////////////////////////////////////////////////////////////////////
void PvmfMediaInputNodeOutPort::SendEndOfTrackCommand(const PvmiMediaXferHeader& data_header_info)
{
    PVMFSharedMediaCmdPtr sharedMediaCmdPtr = PVMFMediaCmd::createMediaCmd();

    sharedMediaCmdPtr->setFormatID(PVMF_MEDIA_CMD_EOS_FORMAT_ID);

    // Set the timestamp
    sharedMediaCmdPtr->setTimestamp(data_header_info.timestamp);

    // Set the sequence number
    sharedMediaCmdPtr->setSeqNum(data_header_info.seq_num);

    PVMFSharedMediaMsgPtr mediaMsgOut;
    convertToPVMFMediaCmdMsg(mediaMsgOut, sharedMediaCmdPtr);

    PVMFStatus status = QueueOutgoingMsg(mediaMsgOut);
    if (status != PVMFSuccess)
    {
        iNode->ReportErrorEvent(PVMFErrPortProcessing, (OsclAny*)status);
        OsclError::Leave(OsclErrGeneral);
    }

    LOGDATATRAFFIC((0, "PvmfMediaInputNodeOutPort::SendEndOfTrackCommand - EOS Sent:"
                    "StreamID=%d, TS=%d, SN=%d, MimeType=%s",
                    data_header_info.stream_id,  data_header_info.timestamp,
                    data_header_info.seq_num, iMimeType.get_cstr()));
}

void PvmfMediaInputNodeOutPort :: freechunkavailable(OsclAny*)
{
    if (iWriteState == EWriteBusy)
    {
        iWriteState = EWriteOK;
        if (IsAdded())
        {
            iMediaInput->statusUpdate(PVMI_MEDIAXFER_STATUS_WRITE);
        }
    }
}
