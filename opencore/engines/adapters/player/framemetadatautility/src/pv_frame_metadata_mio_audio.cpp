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
#include "pv_frame_metadata_mio_audio.h"
#include "pvlogger.h"
#include "pv_mime_string_utils.h"
#include "oscl_snprintf.h"
#include "cczoomrotationbase.h"

PVFMAudioMIO::PVFMAudioMIO() :
        OsclTimerObject(OsclActiveObject::EPriorityNominal, "PVFMAudioMIO")
{
    InitData();
}


void PVFMAudioMIO::InitData()
{
    iAudioFormat = PVMF_MIME_FORMAT_UNKNOWN;
    iAudioSamplingRateValid = false;
    iAudioNumChannelsValid = false;
    iIsMIOConfigured = false;
    iAudioSamplingRate = 0;
    iAudioNumChannels = 0;
    iWriteBusy = false;

    iCommandCounter = 0;
    iLogger = NULL;
    iCommandResponseQueue.reserve(5);
    iWriteResponseQueue.reserve(5);
    iObserver = NULL;
    iLogger = NULL;
    iPeer = NULL;
    iState = STATE_IDLE;


}


void PVFMAudioMIO::ResetData()
{
    Cleanup();

    // Reset all the received media parameters.
    iAudioFormat = PVMF_MIME_FORMAT_UNKNOWN;
    iAudioSamplingRateValid = false;
    iAudioNumChannelsValid = false;
    iAudioSamplingRate = 0;
    iAudioNumChannels = 0;
    iIsMIOConfigured = false;
    iWriteBusy = false;
}


void PVFMAudioMIO::Cleanup()
{
    while (!iCommandResponseQueue.empty())
    {
        if (iObserver)
        {
            iObserver->RequestCompleted(PVMFCmdResp(iCommandResponseQueue[0].iCmdId, iCommandResponseQueue[0].iContext, iCommandResponseQueue[0].iStatus));
        }
        iCommandResponseQueue.erase(&iCommandResponseQueue[0]);
    }

    while (!iWriteResponseQueue.empty())
    {
        if (iPeer)
        {
            iPeer->writeComplete(iWriteResponseQueue[0].iStatus, iWriteResponseQueue[0].iCmdId, (OsclAny*)iWriteResponseQueue[0].iContext);
        }
        iWriteResponseQueue.erase(&iWriteResponseQueue[0]);
    }
}


PVFMAudioMIO::~PVFMAudioMIO()
{
    Cleanup();

}


PVMFStatus PVFMAudioMIO::connect(PvmiMIOSession& aSession, PvmiMIOObserver* aObserver)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMAudioMIO::connect() called"));

    // Currently supports only one session
    OSCL_UNUSED_ARG(aSession);

    if (iObserver)
    {
        return PVMFFailure;
    }

    iObserver = aObserver;
    return PVMFSuccess;
}


PVMFStatus PVFMAudioMIO::disconnect(PvmiMIOSession aSession)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMAudioMIO::disconnect() called"));

    // Currently supports only one session
    OSCL_UNUSED_ARG(aSession);

    iObserver = NULL;
    return PVMFSuccess;
}


PvmiMediaTransfer* PVFMAudioMIO::createMediaTransfer(PvmiMIOSession& aSession, PvmiKvp* read_formats, int32 read_flags,
        PvmiKvp* write_formats, int32 write_flags)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMAudioMIO::createMediaTransfer() called"));

    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(read_formats);
    OSCL_UNUSED_ARG(read_flags);
    OSCL_UNUSED_ARG(write_formats);
    OSCL_UNUSED_ARG(write_flags);

    return (PvmiMediaTransfer*)this;
}


void PVFMAudioMIO::QueueCommandResponse(CommandResponse& aResp)
{
    // Queue a command response and schedule processing.
    iCommandResponseQueue.push_back(aResp);

    // Cancel any timer delay so the command response will happen ASAP.
    if (IsBusy())
    {
        Cancel();
    }

    RunIfNotReady();
}


PVMFCommandId PVFMAudioMIO::QueryUUID(const PvmfMimeString& aMimeType, Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids,
                                      bool aExactUuidsOnly, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMAudioMIO::QueryUUID() called"));

    OSCL_UNUSED_ARG(aMimeType);
    OSCL_UNUSED_ARG(aExactUuidsOnly);

    PVMFCommandId cmdid = iCommandCounter++;

    PVMFStatus status = PVMFFailure;
    int32 err ;
    OSCL_TRY(err,
             aUuids.push_back(PVMI_CAPABILITY_AND_CONFIG_PVUUID);
            );

    if (err == OsclErrNone)
    {
        status = PVMFSuccess;
    }

    CommandResponse resp(status, cmdid, aContext);
    QueueCommandResponse(resp);
    return cmdid;
}


PVMFCommandId PVFMAudioMIO::QueryInterface(const PVUuid& aUuid, PVInterface*& aInterfacePtr, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMAudioMIO::QueryInterface() called"));

    PVMFCommandId cmdid = iCommandCounter++;

    PVMFStatus status = PVMFFailure;
    if (aUuid == PVMI_CAPABILITY_AND_CONFIG_PVUUID)
    {
        PvmiCapabilityAndConfig* myInterface = OSCL_STATIC_CAST(PvmiCapabilityAndConfig*, this);
        aInterfacePtr = OSCL_STATIC_CAST(PVInterface*, myInterface);
        status = PVMFSuccess;
    }
    else
    {
        status = PVMFFailure;
    }

    CommandResponse resp(status, cmdid, aContext);
    QueueCommandResponse(resp);
    return cmdid;
}


void PVFMAudioMIO::deleteMediaTransfer(PvmiMIOSession& aSession, PvmiMediaTransfer* media_transfer)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMAudioMIO::deleteMediaTransfer() called"));

    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(media_transfer);

    // This class is implementing the media transfer, so no cleanup is needed
}


PVMFCommandId PVFMAudioMIO::Init(const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMAudioMIO::Init() called"));

    PVMFCommandId cmdid = iCommandCounter++;

    PVMFStatus status = PVMFFailure;

    switch (iState)
    {
        case STATE_LOGGED_ON:
            status = PVMFSuccess;
            iState = STATE_INITIALIZED;
            break;

        default:
            status = PVMFErrInvalidState;
            break;
    }

    CommandResponse resp(status, cmdid, aContext);
    QueueCommandResponse(resp);
    return cmdid;
}

PVMFCommandId PVFMAudioMIO::Reset(const OsclAny* aContext)
{
    ResetData();

    PVMFCommandId cmdid = iCommandCounter++;
    PVMFStatus status = PVMFSuccess;

    CommandResponse resp(status, cmdid, aContext);
    QueueCommandResponse(resp);
    return cmdid;
}

PVMFCommandId PVFMAudioMIO::Start(const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMAudioMIO::Start() called"));

    PVMFCommandId cmdid = iCommandCounter++;

    PVMFStatus status = PVMFFailure;

    switch (iState)
    {
        case STATE_INITIALIZED:
        case STATE_PAUSED:
            iState = STATE_STARTED;
            status = PVMFSuccess;
            if (iPeer && iWriteBusy)
            {
                iWriteBusy = false;
                iPeer->statusUpdate(PVMI_MEDIAXFER_STATUS_WRITE);
            }
            break;

        default:
            status = PVMFErrInvalidState;
            break;
    }

    CommandResponse resp(status, cmdid, aContext);
    QueueCommandResponse(resp);
    return cmdid;
}


PVMFCommandId PVFMAudioMIO::Pause(const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMAudioMIO::Pause() called"));

    PVMFCommandId cmdid = iCommandCounter++;

    PVMFStatus status = PVMFFailure;

    switch (iState)
    {
        case STATE_STARTED:
            iState = STATE_PAUSED;
            status = PVMFSuccess;
            break;

        default:
            status = PVMFErrInvalidState;
            break;
    }

    CommandResponse resp(status, cmdid, aContext);
    QueueCommandResponse(resp);
    return cmdid;
}


PVMFCommandId PVFMAudioMIO::Flush(const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMAudioMIO::Flush() called"));

    PVMFCommandId cmdid = iCommandCounter++;

    PVMFStatus status = PVMFFailure;

    switch (iState)
    {
        case STATE_STARTED:
            iState = STATE_INITIALIZED;
            status = PVMFSuccess;
            break;

        default:
            status = PVMFErrInvalidState;
            break;
    }

    CommandResponse resp(status, cmdid, aContext);
    QueueCommandResponse(resp);
    return cmdid;
}


PVMFCommandId PVFMAudioMIO::DiscardData(const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMAudioMIO::DiscardData() called"));

    PVMFCommandId cmdid = iCommandCounter++;

    // This component doesn't buffer data, so there's nothing needed here.
    PVMFStatus status = PVMFSuccess;

    CommandResponse resp(status, cmdid, aContext);
    QueueCommandResponse(resp);
    return cmdid;
}

//////////////////////////////////////////////////////////////////////////////////
PVMFCommandId PVFMAudioMIO::DiscardData(PVMFTimestamp aTimestamp, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMAudioMIO::DiscardData(timestamp,context) called"));

    OSCL_UNUSED_ARG(aTimestamp);
    return DiscardData(aContext);
}


PVMFCommandId PVFMAudioMIO::Stop(const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMAudioMIO::Stop() called"));

    PVMFCommandId cmdid = iCommandCounter++;

    PVMFStatus status = PVMFFailure;

    switch (iState)
    {
        case STATE_STARTED:
        case STATE_PAUSED:
            iState = STATE_INITIALIZED;
            status = PVMFSuccess;
            break;

        default:
            status = PVMFErrInvalidState;
            break;
    }

    CommandResponse resp(status, cmdid, aContext);
    QueueCommandResponse(resp);
    return cmdid;
}


PVMFCommandId PVFMAudioMIO::CancelAllCommands(const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMAudioMIO::CancelAllCommands() called"));

    PVMFCommandId cmdid = iCommandCounter++;

    // Commands are executed immediately upon being received, so it isn't really possible to cancel them.

    PVMFStatus status = PVMFSuccess;

    CommandResponse resp(status, cmdid, aContext);
    QueueCommandResponse(resp);
    return cmdid;
}


PVMFCommandId PVFMAudioMIO::CancelCommand(PVMFCommandId aCmdId, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMAudioMIO::CancelCommand() called"));

    PVMFCommandId cmdid = iCommandCounter++;

    // Commands are executed immediately upon being received, so it isn't really possible to cancel them.

    // See if the response is still queued.
    PVMFStatus status = PVMFFailure;
    for (uint32 i = 0;i < iCommandResponseQueue.size();i++)
    {
        if (iCommandResponseQueue[i].iCmdId == aCmdId)
        {
            status = PVMFSuccess;
            break;
        }
    }

    CommandResponse resp(status, cmdid, aContext);
    QueueCommandResponse(resp);
    return cmdid;
}


void PVFMAudioMIO::ThreadLogon()
{
    if (iState == STATE_IDLE)
    {
        iLogger = PVLogger::GetLoggerObject("PVFMAudioMIO");
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMAudioMIO::ThreadLogon() called"));
        AddToScheduler();
        iState = STATE_LOGGED_ON;
    }
}


void PVFMAudioMIO::ThreadLogoff()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMAudioMIO::ThreadLogoff() called"));

    if (iState != STATE_IDLE)
    {
        RemoveFromScheduler();
        iLogger = NULL;
        iState = STATE_IDLE;
    }
}


void PVFMAudioMIO::setPeer(PvmiMediaTransfer* aPeer)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMAudioMIO::setPeer() called"));

    // Set the observer
    iPeer = aPeer;
}


void PVFMAudioMIO::useMemoryAllocators(OsclMemAllocator* write_alloc)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMAudioMIO::useMemoryAllocators() called"));

    OSCL_UNUSED_ARG(write_alloc);

    // Not supported.
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO, (0, "PVFMAudioMIO::useMemoryAllocators() NOT SUPPORTED"));
}


PVMFCommandId PVFMAudioMIO::writeAsync(uint8 aFormatType, int32 aFormatIndex, uint8* aData, uint32 aDataLen,
                                       const PvmiMediaXferHeader& data_header_info, OsclAny* aContext)
{
    OSCL_UNUSED_ARG(aData);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMAudioMIO::writeAsync() seqnum %d ts %d context %d", data_header_info.seq_num, data_header_info.timestamp, aContext));

    PVMFStatus status = PVMFFailure;

    // Do a leave if MIO is not configured except when it is an EOS
    if (!iIsMIOConfigured &&
            !((PVMI_MEDIAXFER_FMT_TYPE_NOTIFICATION == aFormatType) &&
              (PVMI_MEDIAXFER_FMT_INDEX_END_OF_STREAM == aFormatIndex)))
    {
        iWriteBusy = true;
        OSCL_LEAVE(OsclErrInvalidState);
        return -1;
    }

    switch (aFormatType)
    {
        case PVMI_MEDIAXFER_FMT_TYPE_COMMAND :
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMAudioMIO::writeAsync() called with Command info."));
            // Ignore
            status = PVMFSuccess;
            break;

        case PVMI_MEDIAXFER_FMT_TYPE_NOTIFICATION :
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMAudioMIO::writeAsync() called with Notification info."));
            switch (aFormatIndex)
            {
                case PVMI_MEDIAXFER_FMT_INDEX_END_OF_STREAM:
                    break;

                default:
                    break;
            }
            // Ignore
            status = PVMFSuccess;
            break;

        case PVMI_MEDIAXFER_FMT_TYPE_DATA :
            switch (aFormatIndex)
            {
                case PVMI_MEDIAXFER_FMT_INDEX_FMT_SPECIFIC_INFO:
                    // Format-specific info contains codec headers.
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMAudioMIO::writeAsync() called with format-specific info."));

                    if (iState < STATE_INITIALIZED)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR, (0, "PVFMAudioMIO::writeAsync: Error - Invalid state"));
                        iWriteBusy = true;
                        OSCL_LEAVE(OsclErrInvalidState);
                        return -1;
                    }
                    else
                    {
                        if (aDataLen > 0)
                        {
                            status = PVMFSuccess;
                        }
                        else
                        {
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO, (0, "PVFMAudioMIO::writeAsync() called aDataLen==0."));
                            status = PVMFSuccess;
                        }
                    }
                    break;

                case PVMI_MEDIAXFER_FMT_INDEX_DATA:
                    // Data contains the media bitstream.

                    // Verify the state
                    if (iState != STATE_STARTED)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR, (0, "PVFMAudioMIO::writeAsync: Error - Invalid state"));
                        iWriteBusy = true;
                        OSCL_LEAVE(OsclErrInvalidState);
                        return -1;
                    }
                    else
                    {

                        if (aDataLen > 0)
                        {
                            status = PVMFSuccess;
                        }
                        else
                        {
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO, (0, "PVFMAudioMIO::writeAsync() called aDataLen==0."));
                            status = PVMFSuccess;
                        }
                    }
                    break;

                default:
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR, (0, "PVFMAudioMIO::writeAsync: Error - unrecognized format index"));
                    status = PVMFFailure;
                    break;
            }
            break;

        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR, (0, "PVFMAudioMIO::writeAsync: Error - unrecognized format type"));
            status = PVMFFailure;
            break;
    }

    //Schedule asynchronous response
    PVMFCommandId cmdid = iCommandCounter++;
    WriteResponse resp(status, cmdid, aContext, data_header_info.timestamp);
    iWriteResponseQueue.push_back(resp);
    RunIfNotReady();
    return cmdid;

}


void PVFMAudioMIO::writeComplete(PVMFStatus aStatus, PVMFCommandId write_cmd_id, OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMAudioMIO::writeComplete() called"));

    OSCL_UNUSED_ARG(aStatus);
    OSCL_UNUSED_ARG(write_cmd_id);
    OSCL_UNUSED_ARG(aContext);

    // Won't be called since this component is a sink.
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO, (0, "PVFMAudioMIO::writeComplete() Should not be called since this MIO is a sink"));
}


PVMFCommandId PVFMAudioMIO::readAsync(uint8* data, uint32 max_data_len, OsclAny* aContext, int32* formats, uint16 num_formats)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMAudioMIO::readAsync() called"));

    OSCL_UNUSED_ARG(data);
    OSCL_UNUSED_ARG(max_data_len);
    OSCL_UNUSED_ARG(aContext);
    OSCL_UNUSED_ARG(formats);
    OSCL_UNUSED_ARG(num_formats);

    // Read not supported.
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO, (0, "PVFMAudioMIO::readAsync() NOT SUPPORTED"));
    OsclError::Leave(OsclErrNotSupported);
    return -1;
}


void PVFMAudioMIO::readComplete(PVMFStatus aStatus, PVMFCommandId read_cmd_id, int32 format_index,
                                const PvmiMediaXferHeader& data_header_info, OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMAudioMIO::readComplete() called"));

    OSCL_UNUSED_ARG(aStatus);
    OSCL_UNUSED_ARG(read_cmd_id);
    OSCL_UNUSED_ARG(format_index);
    OSCL_UNUSED_ARG(data_header_info);
    OSCL_UNUSED_ARG(aContext);

    // Won't be called since this component is a sink.
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO, (0, "PVFMAudioMIO::readComplete() Should not be called since this MIO is a sink"));
}


void PVFMAudioMIO::statusUpdate(uint32 status_flags)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMAudioMIO::statusUpdate() called"));

    OSCL_UNUSED_ARG(status_flags);

    // Won't be called since this component is a sink.
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO, (0, "PVFMAudioMIO::statusUpdate() Should not be called since this MIO is a sink"));
}


void PVFMAudioMIO::cancelCommand(PVMFCommandId  command_id)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMAudioMIO::cancelCommand() called"));

    // The purpose of this API is to cancel a writeAsync command and report completion ASAP.

    // In this implementation, the write commands are executed immediately when received so it isn't
    // really possible to cancel. Just report completion immediately.

    for (uint32 i = 0;i < iWriteResponseQueue.size();i++)
    {
        if (iWriteResponseQueue[i].iCmdId == command_id)
        {
            //report completion
            if (iPeer)
            {
                iPeer->writeComplete(iWriteResponseQueue[i].iStatus, iWriteResponseQueue[i].iCmdId, (OsclAny*)iWriteResponseQueue[i].iContext);
            }
            iWriteResponseQueue.erase(&iWriteResponseQueue[i]);
            break;
        }
    }
}


void PVFMAudioMIO::cancelAllCommands()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMAudioMIO::cancelAllCommands() called"));

    // The purpose of this API is to cancel all writeAsync commands and report completion ASAP.

    // In this implementaiton, the write commands are executed immediately when received so it isn't
    // really possible to cancel. Just report completion immediately.

    for (uint32 i = 0;i < iWriteResponseQueue.size();i++)
    {
        //report completion
        if (iPeer)
        {
            iPeer->writeComplete(iWriteResponseQueue[i].iStatus, iWriteResponseQueue[i].iCmdId, (OsclAny*)iWriteResponseQueue[i].iContext);
        }
        iWriteResponseQueue.erase(&iWriteResponseQueue[i]);
    }
}


void PVFMAudioMIO::setObserver(PvmiConfigAndCapabilityCmdObserver* aObserver)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMAudioMIO::setObserver() called"));

    OSCL_UNUSED_ARG(aObserver);

    // Not needed since this component only supports synchronous capability & config APIs.
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO, (0, "PVFMAudioMIO::setObserver() NOT SUPPORTED"));
}


PVMFStatus PVFMAudioMIO::getParametersSync(PvmiMIOSession aSession, PvmiKeyType aIdentifier, PvmiKvp*& aParameters,
        int& num_parameter_elements, PvmiCapabilityContext aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMAudioMIO::getParametersSync() called"));

    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aContext);

    aParameters = NULL;
    num_parameter_elements = 0;

    if (pv_mime_strcmp(aIdentifier, INPUT_FORMATS_CAP_QUERY) == 0)
    {
        // This is a query for the list of supported formats.
        // This component supports any audio format
        // Generate a list of all the PVMF audio formats...

        uint32 count = PVMF_SUPPORTED_UNCOMPRESSED_AUDIO_FORMATS_COUNT;
        aParameters = (PvmiKvp*)oscl_malloc(count * sizeof(PvmiKvp));

        if (aParameters)
        {
            aParameters[num_parameter_elements++].value.pChar_value = (char*)PVMF_MIME_PCM;
            aParameters[num_parameter_elements++].value.pChar_value = (char*)PVMF_MIME_PCM8;
            aParameters[num_parameter_elements++].value.pChar_value = (char*)PVMF_MIME_PCM16;
            aParameters[num_parameter_elements++].value.pChar_value = (char*)PVMF_MIME_PCM16_BE;
            aParameters[num_parameter_elements++].value.pChar_value = (char*)PVMF_MIME_ULAW;
            aParameters[num_parameter_elements++].value.pChar_value = (char*)PVMF_MIME_ALAW;
            return PVMFSuccess;
        }
        return PVMFErrNoMemory;
    }

    // Other queries are not currently supported so report as unrecognized key.
    return PVMFFailure;
}


PVMFStatus PVFMAudioMIO::releaseParameters(PvmiMIOSession aSession, PvmiKvp* aParameters, int num_elements)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMAudioMIO::releaseParameters() called"));

    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(num_elements);

    // Release parameters that were allocated by this component.
    if (aParameters)
    {
        oscl_free(aParameters);
        return PVMFSuccess;
    }
    return PVMFFailure;
}


void PVFMAudioMIO::createContext(PvmiMIOSession aSession, PvmiCapabilityContext& aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMAudioMIO::createContext() called"));

    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aContext);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO, (0, "PVFMAudioMIO::createContext() NOT SUPPORTED"));
    OsclError::Leave(OsclErrNotSupported);
}


void PVFMAudioMIO::setContextParameters(PvmiMIOSession aSession, PvmiCapabilityContext& aContext,
                                        PvmiKvp* aParameters, int num_parameter_elements)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMAudioMIO::setContextParameters() called"));

    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aContext);
    OSCL_UNUSED_ARG(aParameters);
    OSCL_UNUSED_ARG(num_parameter_elements);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO, (0, "PVFMAudioMIO::setContextParameters() NOT SUPPORTED"));
    OsclError::Leave(OsclErrNotSupported);
}


void PVFMAudioMIO::DeleteContext(PvmiMIOSession aSession, PvmiCapabilityContext& aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMAudioMIO::DeleteContext() called"));

    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aContext);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO, (0, "PVFMAudioMIO::DeleteContext() NOT SUPPORTED"));
    OsclError::Leave(OsclErrNotSupported);
}


void PVFMAudioMIO::setParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters, int num_elements, PvmiKvp*& aRet_kvp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMAudioMIO::setParametersSync() called"));

    OSCL_UNUSED_ARG(aSession);

    aRet_kvp = NULL;

    for (int32 i = 0;i < num_elements;i++)
    {
        //Check against known audio parameter keys...
        if (pv_mime_strcmp(aParameters[i].key, MOUT_AUDIO_FORMAT_KEY) == 0)
        {
            iAudioFormat = aParameters[i].value.pChar_value;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMAudioMIO::setParametersSync() Audio Format Key, Value %s", iAudioFormat.getMIMEStrPtr()));
        }
        else if (pv_mime_strcmp(aParameters[i].key, MOUT_AUDIO_SAMPLING_RATE_KEY) == 0)
        {
            iAudioSamplingRate = (int32)aParameters[i].value.uint32_value;
            iAudioSamplingRateValid = true;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMAudioMIO::setParametersSync() Audio Sampling Rate Key, Value %d", iAudioSamplingRate));
        }
        else if (pv_mime_strcmp(aParameters[i].key, MOUT_AUDIO_NUM_CHANNELS_KEY) == 0)
        {
            iAudioNumChannels = (int32)aParameters[i].value.uint32_value;
            iAudioNumChannelsValid = true;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMAudioMIO::setParametersSync() Audio Num Channels Key, Value %d", iAudioNumChannels));
        }
        else if (pv_mime_strcmp(aParameters[i].key, PVMF_FORMAT_SPECIFIC_INFO_KEY) == 0)
        {
            //	iOutputFile.Write(aParameters[i].value.pChar_value, sizeof(uint8), (int32)aParameters[i].capacity);
        }
        else
        {
            if (iAudioSamplingRateValid && iAudioNumChannelsValid && !iIsMIOConfigured)
            {
                if (iObserver)
                {
                    iIsMIOConfigured = true;
                    iObserver->ReportInfoEvent(PVMFMIOConfigurationComplete);
                    if (iPeer && iWriteBusy)
                    {
                        iWriteBusy = false;
                        iPeer->statusUpdate(PVMI_MEDIAXFER_STATUS_WRITE);
                    }
                }
            }

            // If we get here the key is unrecognized.
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMAudioMIO::setParametersSync() Error, unrecognized key "));

            // Set the return value to indicate the unrecognized key and return.
            aRet_kvp = &aParameters[i];
            return;
        }
    }
    if (iAudioSamplingRateValid && iAudioNumChannelsValid && !iIsMIOConfigured)
    {
        if (iObserver)
        {
            iIsMIOConfigured = true;
            iObserver->ReportInfoEvent(PVMFMIOConfigurationComplete);
            if (iPeer && iWriteBusy)
            {
                iWriteBusy = false;
                iPeer->statusUpdate(PVMI_MEDIAXFER_STATUS_WRITE);
            }
        }
    }
}


PVMFCommandId PVFMAudioMIO::setParametersAsync(PvmiMIOSession aSession, PvmiKvp* aParameters, int num_elements, PvmiKvp*& aRet_kvp, OsclAny* context)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMAudioMIO::setParametersAsync() called"));

    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aParameters);
    OSCL_UNUSED_ARG(num_elements);
    OSCL_UNUSED_ARG(aRet_kvp);
    OSCL_UNUSED_ARG(context);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO, (0, "PVFMAudioMIO::setParamaetersAsync() NOT SUPPORTED"));
    OsclError::Leave(OsclErrNotSupported);
    return -1;
}


uint32 PVFMAudioMIO::getCapabilityMetric(PvmiMIOSession aSession)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMAudioMIO::getCapabilityMetric() called"));

    OSCL_UNUSED_ARG(aSession);

    return 0;
}


PVMFStatus PVFMAudioMIO::verifyParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters, int num_elements)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMAudioMIO::verifyParametersSync() called"));

    OSCL_UNUSED_ARG(aSession);

    // Go through each parameter
    for (int32 paramind = 0; paramind < num_elements; ++paramind)
    {
        // Retrieve the first component from the key string
        char* compstr = NULL;
        pv_mime_string_extract_type(0, aParameters[paramind].key, compstr);

        if (pv_mime_strcmp(compstr, _STRLIT_CHAR("x-pvmf/media/format-type")) == 0)
        {
            //This component supports PCM8 or PCM16 only.
            if ((pv_mime_strcmp(aParameters[paramind].value.pChar_value, PVMF_MIME_PCM8) == 0) ||
                    (pv_mime_strcmp(aParameters[paramind].value.pChar_value, PVMF_MIME_PCM16) == 0) ||
                    (pv_mime_strcmp(aParameters[paramind].value.pChar_value, PVMF_MIME_PCM16) == 0) ||
                    (pv_mime_strcmp(aParameters[paramind].value.pChar_value, PVMF_MIME_PCM16_BE) == 0) ||
                    (pv_mime_strcmp(aParameters[paramind].value.pChar_value, PVMF_MIME_ULAW) == 0) ||
                    (pv_mime_strcmp(aParameters[paramind].value.pChar_value, PVMF_MIME_ALAW) == 0))
            {
                return PVMFSuccess;
            }
            else
            {
                return PVMFErrNotSupported;
            }
        }
    }
    // For all other parameters return success.
    return PVMFSuccess;
}


//
// For active timing support
//
PVMFStatus PVFMAudioMIOActiveTimingSupport::SetClock(PVMFMediaClock *clockVal)
{
    iClock = clockVal;
    return PVMFSuccess;
}


void PVFMAudioMIOActiveTimingSupport::addRef()
{
}


void PVFMAudioMIOActiveTimingSupport::removeRef()
{
}


bool PVFMAudioMIOActiveTimingSupport::queryInterface(const PVUuid& aUuid, PVInterface*& aInterface)
{
    aInterface = NULL;
    PVUuid uuid;
    queryUuid(uuid);
    if (uuid == aUuid)
    {
        PvmiClockExtensionInterface* myInterface = OSCL_STATIC_CAST(PvmiClockExtensionInterface*, this);
        aInterface = OSCL_STATIC_CAST(PVInterface*, myInterface);
        return true;
    }
    return false;
}


void PVFMAudioMIOActiveTimingSupport::queryUuid(PVUuid& uuid)
{
    uuid = PvmiClockExtensionInterfaceUuid;
}


//
// Private section
//
void PVFMAudioMIO::Run()
{
    // Send async command responses
    while (!iCommandResponseQueue.empty())
    {
        if (iObserver)
        {
            iObserver->RequestCompleted(PVMFCmdResp(iCommandResponseQueue[0].iCmdId, iCommandResponseQueue[0].iContext, iCommandResponseQueue[0].iStatus));
        }
        iCommandResponseQueue.erase(&iCommandResponseQueue[0]);
    }

    // Send async write completion
    while (!iWriteResponseQueue.empty())
    {
        // Report write complete
        if (iPeer)
        {
            iPeer->writeComplete(iWriteResponseQueue[0].iStatus, iWriteResponseQueue[0].iCmdId, (OsclAny*)iWriteResponseQueue[0].iContext);
        }
        iWriteResponseQueue.erase(&iWriteResponseQueue[0]);
    }
}






