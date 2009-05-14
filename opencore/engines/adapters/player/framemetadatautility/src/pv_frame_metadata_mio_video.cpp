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
#include "pv_frame_metadata_mio_video.h"
#include "pvlogger.h"
#include "pv_mime_string_utils.h"
#include "oscl_snprintf.h"
#include "cczoomrotationbase.h"

PVFMVideoMIO::PVFMVideoMIO() :
        OsclTimerObject(OsclActiveObject::EPriorityNominal, "PVFMVideoMIO")
{
    InitData();
}


void PVFMVideoMIO::InitData()
{
    iVideoFormat = PVMF_MIME_FORMAT_UNKNOWN;
	iVideoSubFormat = PVMF_MIME_FORMAT_UNKNOWN;
    iVideoHeightValid = false;
    iVideoWidthValid = false;
    iVideoDisplayHeightValid = false;
    iVideoDisplayWidthValid = false;
    iIsMIOConfigured = false;
    iWriteBusy = false;
    iVideoHeight = 0;
    iVideoWidth = 0;
    iVideoDisplayHeight = 0;
    iVideoDisplayWidth = 0;

    iThumbnailWidth = 320;
    iThumbnailHeight = 240;


    iColorConverter = NULL;
    iCCRGBFormatType = PVMF_MIME_FORMAT_UNKNOWN;

    iCommandCounter = 0;
    iLogger = NULL;
    iCommandResponseQueue.reserve(5);
    iWriteResponseQueue.reserve(5);
    iObserver = NULL;
    iLogger = NULL;
    iPeer = NULL;
    iState = STATE_IDLE;

    iFrameRetrievalInfo.iRetrievalRequested = false;
    iFrameRetrievalInfo.iGetFrameObserver = NULL;
    iFrameRetrievalInfo.iUseFrameIndex = false;
    iFrameRetrievalInfo.iUseTimeOffset = false;
    iFrameRetrievalInfo.iFrameIndex = 0;
    iFrameRetrievalInfo.iTimeOffset = 0;
    iFrameRetrievalInfo.iFrameBuffer = NULL;
    iFrameRetrievalInfo.iBufferSize = NULL;
}


void PVFMVideoMIO::ResetData()
{
    Cleanup();

    // Reset all the received media parameters.
    iVideoFormat = PVMF_MIME_FORMAT_UNKNOWN;
    iVideoSubFormat = PVMF_MIME_FORMAT_UNKNOWN;
    iVideoHeightValid = false;
    iVideoWidthValid = false;
    iVideoDisplayHeightValid = false;
    iVideoDisplayWidthValid = false;
    iVideoHeight = 0;
    iVideoWidth = 0;
    iVideoDisplayHeight = 0;
    iVideoDisplayWidth = 0;
    iIsMIOConfigured = false;
    iWriteBusy = false;
}


void PVFMVideoMIO::Cleanup()
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


PVFMVideoMIO::~PVFMVideoMIO()
{
    Cleanup();

    if (iColorConverter)
    {
        DestroyYUVToRGBColorConverter(iColorConverter, iCCRGBFormatType);
    }
}

void PVFMVideoMIO::setThumbnailDimensions(uint32 aWidth, uint32 aHeight)
{
    iThumbnailWidth = aWidth;
    iThumbnailHeight = aHeight;
}

PVMFStatus PVFMVideoMIO::GetFrameByFrameNumber(uint32 aFrameIndex, uint8* aFrameBuffer, uint32& aBufferSize,
        PVMFFormatType aFormatType, PVFMVideoMIOGetFrameObserver& aObserver)
{
    if (iFrameRetrievalInfo.iRetrievalRequested)
    {
        // Get frame request is already pending so don't accept this request
        return PVMFErrBusy;
    }

    if (aFrameBuffer == NULL || aBufferSize == 0)
    {
        // Bad parameters
        return PVMFErrArgument;
    }

    // Signal for frame retrieval by frame number
    iFrameRetrievalInfo.iRetrievalRequested = true;
    iFrameRetrievalInfo.iGetFrameObserver = &aObserver;
    iFrameRetrievalInfo.iUseFrameIndex = true;
    iFrameRetrievalInfo.iUseTimeOffset = false;
    iFrameRetrievalInfo.iFrameIndex = aFrameIndex;
    iFrameRetrievalInfo.iFrameBuffer = aFrameBuffer;
    iFrameRetrievalInfo.iBufferSize = &aBufferSize;
    iFrameRetrievalInfo.iFrameFormatType = aFormatType;

    iFrameRetrievalInfo.iReceivedFrameCount = 0;
    iFrameRetrievalInfo.iStartingTSSet = false;
    iFrameRetrievalInfo.iStartingTS = 0;

    return PVMFPending;
}


PVMFStatus PVFMVideoMIO::GetFrameByTimeoffset(uint32 aTimeOffset, uint8* aFrameBuffer, uint32& aBufferSize,
        PVMFFormatType aFormatType, PVFMVideoMIOGetFrameObserver& aObserver)
{
    if (iFrameRetrievalInfo.iRetrievalRequested)
    {
        // Get frame request is already pending don't accept this request
        return PVMFErrBusy;
    }

    if (aFrameBuffer == NULL || aBufferSize == 0)
    {
        // Bad parameters
        return PVMFErrArgument;
    }

    // Signal for frame retrieval by time offset
    iFrameRetrievalInfo.iRetrievalRequested = true;
    iFrameRetrievalInfo.iGetFrameObserver = &aObserver;
    iFrameRetrievalInfo.iUseFrameIndex = false;
    iFrameRetrievalInfo.iUseTimeOffset = true;
    iFrameRetrievalInfo.iTimeOffset = aTimeOffset;
    iFrameRetrievalInfo.iFrameBuffer = aFrameBuffer;
    iFrameRetrievalInfo.iBufferSize = &aBufferSize;
    iFrameRetrievalInfo.iFrameFormatType = aFormatType;

    iFrameRetrievalInfo.iReceivedFrameCount = 0;
    iFrameRetrievalInfo.iStartingTSSet = false;
    iFrameRetrievalInfo.iStartingTS = 0;

    return PVMFPending;
}


PVMFStatus PVFMVideoMIO::CancelGetFrame(void)
{
    // Cancel any pending frame retrieval and reset variables
    iFrameRetrievalInfo.iRetrievalRequested = false;
    iFrameRetrievalInfo.iUseFrameIndex = false;
    iFrameRetrievalInfo.iUseTimeOffset = false;

    return PVMFSuccess;
}


PVMFStatus PVFMVideoMIO::GetFrameProperties(uint32& aFrameWidth, uint32& aFrameHeight, uint32& aDisplayWidth, uint32& aDisplayHeight)
{
    if (iVideoWidthValid == false || iVideoHeightValid == false ||
            iVideoDisplayWidthValid == false || iVideoDisplayHeightValid == false)
    {
        return PVMFErrNotReady;
    }

    aFrameWidth = iVideoWidth;
    aFrameHeight = iVideoHeight;
    aDisplayWidth = iVideoDisplayWidth;
    aDisplayHeight = iVideoDisplayHeight;

    return PVMFSuccess;
}


PVMFStatus PVFMVideoMIO::connect(PvmiMIOSession& aSession, PvmiMIOObserver* aObserver)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMVideoMIO::connect() called"));

    // Currently supports only one session
    OSCL_UNUSED_ARG(aSession);

    if (iObserver)
    {
        return PVMFFailure;
    }

    iObserver = aObserver;
    return PVMFSuccess;
}


PVMFStatus PVFMVideoMIO::disconnect(PvmiMIOSession aSession)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMVideoMIO::disconnect() called"));

    // Currently supports only one session
    OSCL_UNUSED_ARG(aSession);

    iObserver = NULL;
    return PVMFSuccess;
}


PvmiMediaTransfer* PVFMVideoMIO::createMediaTransfer(PvmiMIOSession& aSession, PvmiKvp* read_formats, int32 read_flags,
        PvmiKvp* write_formats, int32 write_flags)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMVideoMIO::createMediaTransfer() called"));

    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(read_formats);
    OSCL_UNUSED_ARG(read_flags);
    OSCL_UNUSED_ARG(write_formats);
    OSCL_UNUSED_ARG(write_flags);

    return (PvmiMediaTransfer*)this;
}


void PVFMVideoMIO::QueueCommandResponse(CommandResponse& aResp)
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


PVMFCommandId PVFMVideoMIO::QueryUUID(const PvmfMimeString& aMimeType, Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids,
                                      bool aExactUuidsOnly, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMVideoMIO::QueryUUID() called"));

    OSCL_UNUSED_ARG(aMimeType);
    OSCL_UNUSED_ARG(aExactUuidsOnly);

    PVMFCommandId cmdid = iCommandCounter++;

    PVMFStatus status = PVMFFailure;
    int32 err ;
    OSCL_TRY(err,
             aUuids.push_back(PVMI_CAPABILITY_AND_CONFIG_PVUUID);
             PVUuid uuid;
             iActiveTiming.queryUuid(uuid);
             aUuids.push_back(uuid);
            );

    if (err == OsclErrNone)
    {
        status = PVMFSuccess;
    }

    CommandResponse resp(status, cmdid, aContext);
    QueueCommandResponse(resp);
    return cmdid;
}


PVMFCommandId PVFMVideoMIO::QueryInterface(const PVUuid& aUuid, PVInterface*& aInterfacePtr, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMVideoMIO::QueryInterface() called"));

    PVMFCommandId cmdid = iCommandCounter++;

    PVMFStatus status = PVMFFailure;
    if (aUuid == PVMI_CAPABILITY_AND_CONFIG_PVUUID)
    {
        PvmiCapabilityAndConfig* myInterface = OSCL_STATIC_CAST(PvmiCapabilityAndConfig*, this);
        aInterfacePtr = OSCL_STATIC_CAST(PVInterface*, myInterface);
        status = PVMFSuccess;
    }
    else if (aUuid == PvmiClockExtensionInterfaceUuid)
    {
        PvmiClockExtensionInterface* myInterface = OSCL_STATIC_CAST(PvmiClockExtensionInterface*, &iActiveTiming);
        aInterfacePtr = OSCL_STATIC_CAST(PVInterface*, myInterface);
        status = PVMFSuccess;
        iActiveTiming.addRef();
    }
    else
    {
        status = PVMFFailure;
    }

    CommandResponse resp(status, cmdid, aContext);
    QueueCommandResponse(resp);
    return cmdid;
}


void PVFMVideoMIO::deleteMediaTransfer(PvmiMIOSession& aSession, PvmiMediaTransfer* media_transfer)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMVideoMIO::deleteMediaTransfer() called"));

    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(media_transfer);

    // This class is implementing the media transfer, so no cleanup is needed
}


PVMFCommandId PVFMVideoMIO::Init(const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMVideoMIO::Init() called"));

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

PVMFCommandId PVFMVideoMIO::Reset(const OsclAny* aContext)
{
    ResetData();

    PVMFCommandId cmdid = iCommandCounter++;
    PVMFStatus status = PVMFSuccess;

    CommandResponse resp(status, cmdid, aContext);
    QueueCommandResponse(resp);
    return cmdid;
}

PVMFCommandId PVFMVideoMIO::Start(const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMVideoMIO::Start() called"));

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


PVMFCommandId PVFMVideoMIO::Pause(const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMVideoMIO::Pause() called"));

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


PVMFCommandId PVFMVideoMIO::Flush(const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMVideoMIO::Flush() called"));

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


PVMFCommandId PVFMVideoMIO::DiscardData(const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMVideoMIO::DiscardData() called"));

    PVMFCommandId cmdid = iCommandCounter++;

    // This component doesn't buffer data, so there's nothing needed here.
    PVMFStatus status = PVMFSuccess;

    CommandResponse resp(status, cmdid, aContext);
    QueueCommandResponse(resp);
    return cmdid;
}

//////////////////////////////////////////////////////////////////////////////////
PVMFCommandId PVFMVideoMIO::DiscardData(PVMFTimestamp aTimestamp, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMVideoMIO::DiscardData(timestamp,context) called"));

    OSCL_UNUSED_ARG(aTimestamp);
    return DiscardData(aContext);
}


PVMFCommandId PVFMVideoMIO::Stop(const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMVideoMIO::Stop() called"));

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


PVMFCommandId PVFMVideoMIO::CancelAllCommands(const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMVideoMIO::CancelAllCommands() called"));

    PVMFCommandId cmdid = iCommandCounter++;

    // Commands are executed immediately upon being received, so it isn't really possible to cancel them.

    PVMFStatus status = PVMFSuccess;

    CommandResponse resp(status, cmdid, aContext);
    QueueCommandResponse(resp);
    return cmdid;
}


PVMFCommandId PVFMVideoMIO::CancelCommand(PVMFCommandId aCmdId, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMVideoMIO::CancelCommand() called"));

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


void PVFMVideoMIO::ThreadLogon()
{
    if (iState == STATE_IDLE)
    {
        iLogger = PVLogger::GetLoggerObject("PVFMVideoMIO");
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMVideoMIO::ThreadLogon() called"));
        AddToScheduler();
        iState = STATE_LOGGED_ON;
    }
}


void PVFMVideoMIO::ThreadLogoff()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMVideoMIO::ThreadLogoff() called"));

    if (iState != STATE_IDLE)
    {
        RemoveFromScheduler();
        iLogger = NULL;
        iState = STATE_IDLE;
    }
}


void PVFMVideoMIO::setPeer(PvmiMediaTransfer* aPeer)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMVideoMIO::setPeer() called"));

    // Set the observer
    iPeer = aPeer;
}


void PVFMVideoMIO::useMemoryAllocators(OsclMemAllocator* write_alloc)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMVideoMIO::useMemoryAllocators() called"));

    OSCL_UNUSED_ARG(write_alloc);

    // Not supported.
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO, (0, "PVFMVideoMIO::useMemoryAllocators() NOT SUPPORTED"));
}


PVMFCommandId PVFMVideoMIO::writeAsync(uint8 aFormatType, int32 aFormatIndex, uint8* aData, uint32 aDataLen,
                                       const PvmiMediaXferHeader& data_header_info, OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMVideoMIO::writeAsync() seqnum %d ts %d context %d", data_header_info.seq_num, data_header_info.timestamp, aContext));

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
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMVideoMIO::writeAsync() called with Command info."));
            // Ignore
            status = PVMFSuccess;
            break;

        case PVMI_MEDIAXFER_FMT_TYPE_NOTIFICATION :
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMVideoMIO::writeAsync() called with Notification info."));
            switch (aFormatIndex)
            {
                case PVMI_MEDIAXFER_FMT_INDEX_END_OF_STREAM:
                    // If waiting for frame then return errMaxReached
                    if (iFrameRetrievalInfo.iRetrievalRequested)
                    {
                        iFrameRetrievalInfo.iRetrievalRequested = false;
                        iFrameRetrievalInfo.iUseFrameIndex = false;
                        iFrameRetrievalInfo.iUseTimeOffset = false;
                        iFrameRetrievalInfo.iGetFrameObserver->HandleFrameReadyEvent(PVMFErrMaxReached);
                    }
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
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMVideoMIO::writeAsync() called with format-specific info."));

                    if (iState < STATE_INITIALIZED)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR, (0, "PVFMVideoMIO::writeAsync: Error - Invalid state"));
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
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO, (0, "PVFMVideoMIO::writeAsync() called aDataLen==0."));
                            status = PVMFSuccess;
                        }
                    }
                    break;

                case PVMI_MEDIAXFER_FMT_INDEX_DATA:
                    // Data contains the media bitstream.

                    // Verify the state
                    if (iState != STATE_STARTED)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR, (0, "PVFMVideoMIO::writeAsync: Error - Invalid state"));
                        iWriteBusy = true;
                        OSCL_LEAVE(OsclErrInvalidState);
                        return -1;
                    }
                    else
                    {
                        if (iFrameRetrievalInfo.iRetrievalRequested)
                        {
                            if (iFrameRetrievalInfo.iUseFrameIndex)
                            {
                                ++iFrameRetrievalInfo.iReceivedFrameCount;
                                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMVideoMIO::writeAsync() Received frames %d", iFrameRetrievalInfo.iReceivedFrameCount));
                            }
                            else if (iFrameRetrievalInfo.iUseTimeOffset && iFrameRetrievalInfo.iStartingTSSet == false)
                            {
                                iFrameRetrievalInfo.iStartingTSSet = true;
                                iFrameRetrievalInfo.iStartingTS = data_header_info.timestamp;
                                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMVideoMIO::writeAsync() Starting timestamp set %d", iFrameRetrievalInfo.iStartingTS));
                            }
                        }

                        if (aDataLen > 0)
                        {
                            // Check if a frame retrieval was requested
                            if (iFrameRetrievalInfo.iRetrievalRequested)
                            {

                                // scale down output proportionally if smaller thumbnail requested
                                if (iVideoDisplayWidth > iThumbnailWidth || iVideoDisplayHeight > iThumbnailHeight)
                                {
                                    float fScaleWidth = (float)iThumbnailWidth / iVideoDisplayWidth;
                                    float fScaleHeight = (float)iThumbnailHeight / iVideoDisplayHeight;
                                    float fScale = (fScaleWidth > fScaleHeight) ? fScaleHeight : fScaleWidth;
                                    iVideoDisplayWidth = (uint32)(iVideoDisplayWidth * fScale);
                                    iVideoDisplayHeight = (uint32)(iVideoDisplayHeight * fScale);
                                }

                                if (iFrameRetrievalInfo.iUseFrameIndex == true &&
                                        iFrameRetrievalInfo.iReceivedFrameCount > iFrameRetrievalInfo.iFrameIndex)
                                {
                                    PVMFStatus evstatus = PVMFFailure;
                                    // Copy the frame data
                                    evstatus = CopyVideoFrameData(aData, aDataLen, iVideoFormat,
                                                                  iFrameRetrievalInfo.iFrameBuffer, *(iFrameRetrievalInfo.iBufferSize), iFrameRetrievalInfo.iFrameFormatType,
                                                                  iVideoWidth, iVideoHeight, iVideoDisplayWidth, iVideoDisplayHeight);

                                    iFrameRetrievalInfo.iRetrievalRequested = false;
                                    iFrameRetrievalInfo.iUseFrameIndex = false;
                                    iFrameRetrievalInfo.iUseTimeOffset = false;

                                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMVideoMIO::writeAsync() Retrieved requested frame by frame index(%d) Status %d", iFrameRetrievalInfo.iFrameIndex, evstatus));
                                    iFrameRetrievalInfo.iGetFrameObserver->HandleFrameReadyEvent(evstatus);
                                }
                                else if (iFrameRetrievalInfo.iUseTimeOffset == true &&
                                         iFrameRetrievalInfo.iStartingTSSet == true &&
                                         (data_header_info.timestamp - iFrameRetrievalInfo.iStartingTS) >= iFrameRetrievalInfo.iTimeOffset)
                                {
                                    PVMFStatus evstatus = PVMFFailure;

                                    // Copy the frame data
                                    evstatus = CopyVideoFrameData(aData, aDataLen, iVideoFormat,
                                                                  iFrameRetrievalInfo.iFrameBuffer, *(iFrameRetrievalInfo.iBufferSize), iFrameRetrievalInfo.iFrameFormatType,
                                                                  iVideoWidth, iVideoHeight, iVideoDisplayWidth, iVideoDisplayHeight);

                                    iFrameRetrievalInfo.iRetrievalRequested = false;
                                    iFrameRetrievalInfo.iUseFrameIndex = false;
                                    iFrameRetrievalInfo.iUseTimeOffset = false;

                                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMVideoMIO::writeAsync() Retrieved requested frame by time(%d) Actual TS %d Status %d", iFrameRetrievalInfo.iTimeOffset, data_header_info.timestamp, evstatus));
                                    iFrameRetrievalInfo.iGetFrameObserver->HandleFrameReadyEvent(evstatus);
                                }
                            }

                            status = PVMFSuccess;
                        }
                        else
                        {
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO, (0, "PVFMVideoMIO::writeAsync() called aDataLen==0."));
                            status = PVMFSuccess;
                        }
                    }
                    break;

                default:
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR, (0, "PVFMVideoMIO::writeAsync: Error - unrecognized format index"));
                    status = PVMFFailure;
                    break;
            }
            break;

        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR, (0, "PVFMVideoMIO::writeAsync: Error - unrecognized format type"));
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


PVMFStatus PVFMVideoMIO::CopyVideoFrameData(uint8* aSrcBuffer, uint32 aSrcSize, PVMFFormatType aSrcFormat,
        uint8* aDestBuffer, uint32& aDestSize, PVMFFormatType aDestFormat,
        uint32 aSrcWidth, uint32 aSrcHeight, uint32 aDestWidth, uint32 aDestHeight)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMVideoMIO::CopyVideoFrameData() In"));

    if (aSrcBuffer == NULL || aSrcSize == 0 || aSrcFormat == PVMF_MIME_FORMAT_UNKNOWN ||
            aDestBuffer == NULL || aDestSize == 0 || aDestFormat == PVMF_MIME_FORMAT_UNKNOWN)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFMVideoMIO::CopyVideoFrameData() Color converter instantiation did a leave"));
        return PVMFErrArgument;
    }

    if (aSrcFormat == aDestFormat)
    {
        // Same format so direct copy
        if (aDestSize < aSrcSize)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFMVideoMIO::CopyVideoFrameData() Color converter instantiation did a leave"));
            return PVMFErrArgument;
        }

        if (iVideoSubFormat == PVMF_MIME_YUV420_SEMIPLANAR_YVU)
		{
			PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, 
				(0, "PVFMVideoMIO::CopyVideoFrameData() SubFormat is YUV SemiPlanar, Convert to YUV planar first"));
            convertYUV420SPtoYUV420(aSrcBuffer, aDestBuffer, aSrcSize);
        } 
		else 
		{
            oscl_memcpy(aDestBuffer, aSrcBuffer, aSrcSize);
        }
        aDestSize = aSrcSize;
    }
    else if (aSrcFormat == PVMF_MIME_YUV420 &&
             (aDestFormat == PVMF_MIME_RGB12 || aDestFormat == PVMF_MIME_RGB16 || aDestFormat == PVMF_MIME_RGB24))
    {
        // Source is YUV 4:2:0 and dest is RGB 12, 16, or 24 bit

        // Validate the source and dest dimensions
        if (aSrcWidth == 0 || aSrcHeight == 0 || aDestWidth == 0 || aDestHeight == 0)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFMVideoMIO::CopyVideoFrameData() Invalid frame dimensions Src(WxH): %dx%d Dest(WxH): %dx%d",
                            aSrcWidth, aSrcHeight, aDestWidth, aDestHeight));
            return PVMFErrArgument;
        }

        // Check if the proper color converter is available
        if (iColorConverter && iCCRGBFormatType != aDestFormat)
        {
            DestroyYUVToRGBColorConverter(iColorConverter, iCCRGBFormatType);
        }

        // Instantiate a new color converter if needed
        if (iColorConverter == NULL)
        {
            PVMFStatus retval = CreateYUVToRGBColorConverter(iColorConverter, aDestFormat);
            if (retval != PVMFSuccess)
            {
                // Color converter could not be instantiated
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFMVideoMIO::CopyVideoFrameData() Appropriate YUV to RGB color converter could not be instantiated"));
                return retval;
            }
            iCCRGBFormatType = aDestFormat;
        }

        if (!(iColorConverter->Init((aSrcWidth + 1)&(~1), (aSrcHeight + 1)&(~1), (aSrcWidth + 1)&(~1), aDestWidth, (aDestHeight + 1)&(~1), (aDestWidth + 1)&(~1), CCROTATE_NONE)))
        {
            iColorConverter = NULL;
            return PVMFFailure;
        }


        iColorConverter->SetMemHeight((iVideoHeight + 1)&(~1));
        iColorConverter->SetMode(1); // Do scaling if needed.

        uint32 rgbbufsize = (uint32)(iColorConverter->GetOutputBufferSize());
        if (rgbbufsize > aDestSize)
        {
            // Specified buffer does not have enough space
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFMVideoMIO::CopyVideoFrameData() Specified output RGB buffer does not have enough space. Needed %d Available %d", rgbbufsize, aDestSize));
            return PVMFErrArgument;
        }

        // Do the color conversion
        if (iColorConverter->Convert(aSrcBuffer, aDestBuffer) == 0)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFMVideoMIO::CopyVideoFrameData() Color conversion failed"));
            return PVMFErrResource;
        }
        // Save the RGB frame size
        aDestSize = rgbbufsize;
    }
    else
    {
        // Other conversions not supported yet
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFMVideoMIO::CopyVideoFrameData() Unsupported conversion mode."));
        return PVMFErrNotSupported;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMVideoMIO::CopyVideoFrameData() Out"));
    return PVMFSuccess;
}


void PVFMVideoMIO::writeComplete(PVMFStatus aStatus, PVMFCommandId write_cmd_id, OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMVideoMIO::writeComplete() called"));

    OSCL_UNUSED_ARG(aStatus);
    OSCL_UNUSED_ARG(write_cmd_id);
    OSCL_UNUSED_ARG(aContext);

    // Won't be called since this component is a sink.
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO, (0, "PVFMVideoMIO::writeComplete() Should not be called since this MIO is a sink"));
}


PVMFCommandId PVFMVideoMIO::readAsync(uint8* data, uint32 max_data_len, OsclAny* aContext, int32* formats, uint16 num_formats)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMVideoMIO::readAsync() called"));

    OSCL_UNUSED_ARG(data);
    OSCL_UNUSED_ARG(max_data_len);
    OSCL_UNUSED_ARG(aContext);
    OSCL_UNUSED_ARG(formats);
    OSCL_UNUSED_ARG(num_formats);

    // Read not supported.
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO, (0, "PVFMVideoMIO::readAsync() NOT SUPPORTED"));
    OsclError::Leave(OsclErrNotSupported);
    return -1;
}


void PVFMVideoMIO::readComplete(PVMFStatus aStatus, PVMFCommandId read_cmd_id, int32 format_index,
                                const PvmiMediaXferHeader& data_header_info, OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMVideoMIO::readComplete() called"));

    OSCL_UNUSED_ARG(aStatus);
    OSCL_UNUSED_ARG(read_cmd_id);
    OSCL_UNUSED_ARG(format_index);
    OSCL_UNUSED_ARG(data_header_info);
    OSCL_UNUSED_ARG(aContext);

    // Won't be called since this component is a sink.
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO, (0, "PVFMVideoMIO::readComplete() Should not be called since this MIO is a sink"));
}


void PVFMVideoMIO::statusUpdate(uint32 status_flags)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMVideoMIO::statusUpdate() called"));

    OSCL_UNUSED_ARG(status_flags);

    // Won't be called since this component is a sink.
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO, (0, "PVFMVideoMIO::statusUpdate() Should not be called since this MIO is a sink"));
}


void PVFMVideoMIO::cancelCommand(PVMFCommandId  command_id)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMVideoMIO::cancelCommand() called"));

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


void PVFMVideoMIO::cancelAllCommands()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMVideoMIO::cancelAllCommands() called"));

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


void PVFMVideoMIO::setObserver(PvmiConfigAndCapabilityCmdObserver* aObserver)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMVideoMIO::setObserver() called"));

    OSCL_UNUSED_ARG(aObserver);

    // Not needed since this component only supports synchronous capability & config APIs.
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO, (0, "PVFMVideoMIO::setObserver() NOT SUPPORTED"));
}


PVMFStatus PVFMVideoMIO::getParametersSync(PvmiMIOSession aSession, PvmiKeyType aIdentifier, PvmiKvp*& aParameters,
        int& num_parameter_elements, PvmiCapabilityContext aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMVideoMIO::getParametersSync() called"));

    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aContext);

    aParameters = NULL;
    num_parameter_elements = 0;

    if (pv_mime_strcmp(aIdentifier, MOUT_VIDEO_FORMAT_KEY) == 0)
    {
        //query for video format string
        aParameters = (PvmiKvp*)oscl_malloc(sizeof(PvmiKvp));
        aParameters->value.pChar_value = (char*)iVideoFormat.getMIMEStrPtr();
        //don't bother to set the key string.
        return PVMFSuccess;
    }

    else if (pv_mime_strcmp(aIdentifier, INPUT_FORMATS_CAP_QUERY) == 0)
    {
        // This is a query for the list of supported formats.
        // This component supports all uncompressed video format
        // Generate a list of all the PVMF video formats...
        int32 count = PVMF_SUPPORTED_UNCOMPRESSED_VIDEO_FORMATS_COUNT;

        aParameters = (PvmiKvp*)oscl_malloc(count * sizeof(PvmiKvp));

        if (aParameters)
        {
            aParameters[num_parameter_elements++].value.pChar_value = (char*)PVMF_MIME_YUV420;
            aParameters[num_parameter_elements++].value.pChar_value = (char*)PVMF_MIME_YUV422;
            aParameters[num_parameter_elements++].value.pChar_value = (char*)PVMF_MIME_RGB8;
            aParameters[num_parameter_elements++].value.pChar_value = (char*)PVMF_MIME_RGB12;
            aParameters[num_parameter_elements++].value.pChar_value = (char*)PVMF_MIME_RGB16;
            aParameters[num_parameter_elements++].value.pChar_value = (char*)PVMF_MIME_RGB24;
            return PVMFSuccess;
        }
        return PVMFErrNoMemory;
    }

    // Other queries are not currently supported so report as unrecognized key.
    return PVMFFailure;
}


PVMFStatus PVFMVideoMIO::releaseParameters(PvmiMIOSession aSession, PvmiKvp* aParameters, int num_elements)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMVideoMIO::releaseParameters() called"));

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


void PVFMVideoMIO::createContext(PvmiMIOSession aSession, PvmiCapabilityContext& aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMVideoMIO::createContext() called"));

    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aContext);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO, (0, "PVFMVideoMIO::createContext() NOT SUPPORTED"));
    OsclError::Leave(OsclErrNotSupported);
}


void PVFMVideoMIO::setContextParameters(PvmiMIOSession aSession, PvmiCapabilityContext& aContext,
                                        PvmiKvp* aParameters, int num_parameter_elements)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMVideoMIO::setContextParameters() called"));

    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aContext);
    OSCL_UNUSED_ARG(aParameters);
    OSCL_UNUSED_ARG(num_parameter_elements);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO, (0, "PVFMVideoMIO::setContextParameters() NOT SUPPORTED"));
    OsclError::Leave(OsclErrNotSupported);
}


void PVFMVideoMIO::DeleteContext(PvmiMIOSession aSession, PvmiCapabilityContext& aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMVideoMIO::DeleteContext() called"));

    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aContext);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO, (0, "PVFMVideoMIO::DeleteContext() NOT SUPPORTED"));
    OsclError::Leave(OsclErrNotSupported);
}


void PVFMVideoMIO::setParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters, int num_elements, PvmiKvp*& aRet_kvp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMVideoMIO::setParametersSync() called"));

    OSCL_UNUSED_ARG(aSession);

    aRet_kvp = NULL;

    for (int32 i = 0;i < num_elements;i++)
    {
        //Check against known video parameter keys...
        if (pv_mime_strcmp(aParameters[i].key, MOUT_VIDEO_FORMAT_KEY) == 0)
        {
            iVideoFormat = aParameters[i].value.pChar_value;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMVideoMIO::setParametersSync() Video Format Key, Value %s", iVideoFormat.getMIMEStrPtr()));
        }
        else if (pv_mime_strcmp(aParameters[i].key, MOUT_VIDEO_WIDTH_KEY) == 0)
        {
            iVideoWidth = (int32)aParameters[i].value.uint32_value;
            iVideoWidthValid = true;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMVideoMIO::setParametersSync() Video Width Key, Value %d", iVideoWidth));
        }
        else if (pv_mime_strcmp(aParameters[i].key, MOUT_VIDEO_HEIGHT_KEY) == 0)
        {
            iVideoHeight = (int32)aParameters[i].value.uint32_value;
            iVideoHeightValid = true;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMVideoMIO::setParametersSync() Video Height Key, Value %d", iVideoHeight));
        }
        else if (pv_mime_strcmp(aParameters[i].key, MOUT_VIDEO_DISPLAY_HEIGHT_KEY) == 0)
        {
            iVideoDisplayHeight = (int32)aParameters[i].value.uint32_value;
            iVideoDisplayHeightValid = true;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMVideoMIO::setParametersSync() Video Display Height Key, Value %d", iVideoDisplayHeight));
        }
        else if (pv_mime_strcmp(aParameters[i].key, MOUT_VIDEO_DISPLAY_WIDTH_KEY) == 0)
        {
            iVideoDisplayWidth = (int32)aParameters[i].value.uint32_value;
            iVideoDisplayWidthValid = true;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMVideoMIO::setParametersSync() Video Display Width Key, Value %d", iVideoDisplayWidth));
        }
        else if (pv_mime_strcmp(aParameters[i].key, PVMF_FORMAT_SPECIFIC_INFO_KEY) == 0)
        {
            //	iOutputFile.Write(aParameters[i].value.pChar_value, sizeof(uint8), (int32)aParameters[i].capacity);
        }
        else if (pv_mime_strcmp(aParameters[i].key, MOUT_VIDEO_SUBFORMAT_KEY) == 0)
        {
            iVideoSubFormat = aParameters[i].value.pChar_value;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0,"PVFMVideoMIO::setParametersSync() Video SubFormat Key, Value %s", iVideoSubFormat.getMIMEStrPtr()));
        }
        else
        {
            if (iVideoWidthValid && iVideoHeightValid && iVideoDisplayHeightValid && iVideoDisplayHeightValid && !iIsMIOConfigured)
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
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMVideoMIO::setParametersSync() Error, unrecognized key "));

            // Set the return value to indicate the unrecognized key and return.
            aRet_kvp = &aParameters[i];
            return;
        }
    }
    if (iVideoWidthValid && iVideoHeightValid && iVideoDisplayHeightValid && iVideoDisplayHeightValid && !iIsMIOConfigured)
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


PVMFCommandId PVFMVideoMIO::setParametersAsync(PvmiMIOSession aSession, PvmiKvp* aParameters, int num_elements, PvmiKvp*& aRet_kvp, OsclAny* context)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMVideoMIO::setParametersAsync() called"));

    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aParameters);
    OSCL_UNUSED_ARG(num_elements);
    OSCL_UNUSED_ARG(aRet_kvp);
    OSCL_UNUSED_ARG(context);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO, (0, "PVFMVideoMIO::setParamaetersAsync() NOT SUPPORTED"));
    OsclError::Leave(OsclErrNotSupported);
    return -1;
}


uint32 PVFMVideoMIO::getCapabilityMetric(PvmiMIOSession aSession)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMVideoMIO::getCapabilityMetric() called"));

    OSCL_UNUSED_ARG(aSession);

    return 0;
}


PVMFStatus PVFMVideoMIO::verifyParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters, int num_elements)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFMVideoMIO::verifyParametersSync() called"));

    OSCL_UNUSED_ARG(aSession);

    // Go through each parameter
    for (int32 paramind = 0; paramind < num_elements; ++paramind)
    {
        // Retrieve the first component from the key string
        char* compstr = NULL;
        pv_mime_string_extract_type(0, aParameters[paramind].key, compstr);

        if (pv_mime_strcmp(compstr, _STRLIT_CHAR("x-pvmf/media/format-type")) == 0)
        {
            //This component supports only uncompressed formats
            if ((pv_mime_strcmp(aParameters[paramind].value.pChar_value, PVMF_MIME_YUV420) == 0) ||
                    (pv_mime_strcmp(aParameters[paramind].value.pChar_value, PVMF_MIME_YUV422) == 0) ||
                    (pv_mime_strcmp(aParameters[paramind].value.pChar_value, PVMF_MIME_RGB8) == 0) ||
                    (pv_mime_strcmp(aParameters[paramind].value.pChar_value, PVMF_MIME_RGB12) == 0) ||
                    (pv_mime_strcmp(aParameters[paramind].value.pChar_value, PVMF_MIME_RGB16) == 0) ||
                    (pv_mime_strcmp(aParameters[paramind].value.pChar_value, PVMF_MIME_RGB24) == 0))
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
PVMFStatus PVFMVideoMIOActiveTimingSupport::SetClock(PVMFMediaClock *clockVal)
{
    iClock = clockVal;
    return PVMFSuccess;
}


void PVFMVideoMIOActiveTimingSupport::addRef()
{
}


void PVFMVideoMIOActiveTimingSupport::removeRef()
{
}


bool PVFMVideoMIOActiveTimingSupport::queryInterface(const PVUuid& aUuid, PVInterface*& aInterface)
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


void PVFMVideoMIOActiveTimingSupport::queryUuid(PVUuid& uuid)
{
    uuid = PvmiClockExtensionInterfaceUuid;
}


//
// Private section
//
void PVFMVideoMIO::Run()
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

static inline void* byteOffset(void* p, uint32 offset) { return (void*)((uint8*)p + offset); }

// convert a frame in YUV420 semiplanar format with VU ordering to YUV420 planar format
void PVFMVideoMIO::convertYUV420SPtoYUV420(void* src, void* dst, uint32 len)
{
    // copy the Y plane
    uint32 y_plane_size = iVideoWidth * iVideoHeight;
    memcpy(dst, src, y_plane_size + iVideoWidth);

    // re-arrange U's and V's
    uint32* p = (uint32*)byteOffset(src, y_plane_size);
    uint16* pu = (uint16*)byteOffset(dst, y_plane_size);
    uint16* pv = (uint16*)byteOffset(pu, y_plane_size / 4);

    int count = y_plane_size / 8;
    do 
	{
        uint32 uvuv = *p++;
        *pu++ = (uint16) (((uvuv >> 8) & 0xff) | ((uvuv >> 16) & 0xff00));
        *pv++ = (uint16) ((uvuv & 0xff) | ((uvuv >> 8) & 0xff00));
    } while (--count);
}


