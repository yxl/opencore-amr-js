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
 * @file pvmi_mio_fileinput.cpp
 * @brief PV Media IO interface implementation using file input
 */

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef PVMI_MIO_COMM_LOOPBACK_H_INCLUDED
#include "pvmi_mio_comm_loopback.h"
#endif
#ifndef OSCL_MIME_STRING_UTILS_H
#include "pv_mime_string_utils.h"
#endif
#ifndef OSCL_DLL_H_INCLUDED
#include "oscl_dll.h"
#endif
#ifndef PVMF_MEDIA_MSG_FORMAT_IDS_H_INCLUDED
#include "pvmf_media_msg_format_ids.h"
#endif

// Define entry point for this DLL
OSCL_DLL_ENTRY_POINT_DEFAULT()

#define PVMIO_MEDIADATA_POOLNUM 8
const uint32 AMR_FRAME_DELAY = 20; // 20ms

#define INPUT_TRANSFER_MODEL_VAL ".../input/transfer_model;valtype=uint32"
#define OUTPUT_TRANSFER_MODEL_VAL ".../output/transfer_model;valtype=uin32"

// Logging macros
#define LOG_STACK_TRACE(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, m)
#define LOG_DEBUG(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, m)
#define LOG_ERR(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iLogger,PVLOGMSG_ERR,m)

OSCL_EXPORT_REF PvmiMIOControl* PvmiMIOCommLoopbackFactory::Create(const PvmiMIOCommLoopbackSettings& aSettings)
{
    PvmiMIOControl *mioFilein = OSCL_STATIC_CAST(PvmiMIOControl*, OSCL_NEW(PvmiMIOCommLoopback, (aSettings)));

    return mioFilein;
}

OSCL_EXPORT_REF bool PvmiMIOCommLoopbackFactory::Delete(PvmiMIOControl* aMio)
{
    PvmiMIOCommLoopback *mioFilein = OSCL_STATIC_CAST(PvmiMIOCommLoopback*, aMio);
    if (!mioFilein)
        return false;
    OSCL_DELETE(mioFilein);

    mioFilein = NULL;
    return true;

}


////////////////////////////////////////////////////////////////////////////
PvmiMIOCommLoopback::~PvmiMIOCommLoopback()
{
    while (!iObservers.empty())
    {
        iObservers.pop_back();
    }
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PvmiMIOCommLoopback::connect(PvmiMIOSession& aSession, PvmiMIOObserver* aObserver)
{
    if (!aObserver)
    {
        return PVMFFailure;
    }

    int32 err = 0;
    OSCL_TRY(err, iObservers.push_back(aObserver));
    OSCL_FIRST_CATCH_ANY(err, return PVMFErrNoMemory);
    aSession = (PvmiMIOSession)(iObservers.size() - 1); // Session ID is the index of observer in the vector
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PvmiMIOCommLoopback::disconnect(PvmiMIOSession aSession)
{
    uint32 index = (uint32)aSession;
    if (index >= iObservers.size())
    {
        // Invalid session ID
        return PVMFFailure;
    }

    iObservers.erase(iObservers.begin() + index);
    iObservers[index] = 0;
    iObservers.destroy();
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PvmiMediaTransfer* PvmiMIOCommLoopback::createMediaTransfer(PvmiMIOSession& aSession,
        PvmiKvp* read_formats,
        int32 read_flags,
        PvmiKvp* write_formats,
        int32 write_flags)
{
    OSCL_UNUSED_ARG(read_formats);
    OSCL_UNUSED_ARG(read_flags);
    OSCL_UNUSED_ARG(write_formats);
    OSCL_UNUSED_ARG(write_flags);

    uint32 index = (uint32)aSession;
    if (index >= iObservers.size())
    {
        // Invalid session ID
        OSCL_LEAVE(OsclErrArgument);
        return NULL;
    }

    return (PvmiMediaTransfer*)this;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PvmiMIOCommLoopback::deleteMediaTransfer(PvmiMIOSession& aSession,
        PvmiMediaTransfer* media_transfer)
{
    uint32 index = (uint32)aSession;
    if (!media_transfer || index >= iObservers.size())
    {
        // Invalid session ID
        OSCL_LEAVE(OsclErrArgument);
    }
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PvmiMIOCommLoopback::QueryUUID(const PvmfMimeString& aMimeType,
        Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids,
        bool aExactUuidsOnly,
        const OsclAny* aContext)
{
    OSCL_UNUSED_ARG(aMimeType);
    OSCL_UNUSED_ARG(aExactUuidsOnly);

    int32 err = 0;
    OSCL_TRY(err, aUuids.push_back(PVMI_CAPABILITY_AND_CONFIG_PVUUID););
    OSCL_FIRST_CATCH_ANY(err, OSCL_LEAVE(OsclErrNoMemory););

    return AddCmdToQueue(CMD_QUERY_UUID, aContext);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PvmiMIOCommLoopback::QueryInterface(const PVUuid& aUuid,
        PVInterface*& aInterfacePtr,
        const OsclAny* aContext)
{
    if (aUuid == PVMI_CAPABILITY_AND_CONFIG_PVUUID)
    {
        PvmiCapabilityAndConfig* myInterface = OSCL_STATIC_CAST(PvmiCapabilityAndConfig*, this);
        aInterfacePtr = OSCL_STATIC_CAST(PVInterface*, myInterface);
    }
    else
    {
        aInterfacePtr = NULL;
    }

    return AddCmdToQueue(CMD_QUERY_INTERFACE, aContext, (OsclAny*)&aInterfacePtr);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PvmiMIOCommLoopback:: Init(const OsclAny* aContext)
{
    if (iState != STATE_IDLE)
    {
        OSCL_LEAVE(OsclErrInvalidState);
        return -1;
    }

    return AddCmdToQueue(CMD_INIT, aContext);
}


////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PvmiMIOCommLoopback::Start(const OsclAny* aContext)
{
    if (iState != STATE_INITIALIZED && iState != STATE_PAUSED)
    {
        OSCL_LEAVE(OsclErrInvalidState);
        return -1;
    }

    return AddCmdToQueue(CMD_START, aContext);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PvmiMIOCommLoopback::Pause(const OsclAny* aContext)
{
    if (iState != STATE_STARTED)
    {
        OSCL_LEAVE(OsclErrInvalidState);
        return -1;
    }

    return AddCmdToQueue(CMD_PAUSE, aContext);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PvmiMIOCommLoopback::Flush(const OsclAny* aContext)
{
    if (iState != STATE_STARTED || iState != STATE_PAUSED)
    {
        OSCL_LEAVE(OsclErrInvalidState);
        return -1;
    }

    return AddCmdToQueue(CMD_FLUSH, aContext);
}

OSCL_EXPORT_REF PVMFCommandId PvmiMIOCommLoopback::Reset(const OsclAny* aContext)
{
    return AddCmdToQueue(CMD_RESET, aContext);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PvmiMIOCommLoopback::DiscardData(const OsclAny* aContext)
{
    OSCL_UNUSED_ARG(aContext);
    OSCL_LEAVE(OsclErrNotSupported);
    return -1;
}

OSCL_EXPORT_REF PVMFCommandId PvmiMIOCommLoopback::DiscardData(PVMFTimestamp aTimestamp, const OsclAny* aContext)
{
    OSCL_UNUSED_ARG(aTimestamp);
    OSCL_UNUSED_ARG(aContext);
    OSCL_LEAVE(OsclErrNotSupported);
    return -1;
}


////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PvmiMIOCommLoopback::Stop(const OsclAny* aContext)
{
    if (iState != STATE_STARTED && iState != STATE_PAUSED)
    {
        OSCL_LEAVE(OsclErrInvalidState);
        return -1;
    }

    return AddCmdToQueue(CMD_STOP, aContext);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PvmiMIOCommLoopback::ThreadLogon()
{
    if (!iThreadLoggedOn)
    {
        AddToScheduler();
        iLogger = PVLogger::GetLoggerObject("PvmiMIOCommLoopback");
        iThreadLoggedOn = true;
    }
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PvmiMIOCommLoopback::ThreadLogoff()
{
    if (iThreadLoggedOn)
    {
        RemoveFromScheduler();
        iLogger = NULL;
        iThreadLoggedOn = false;
    }
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PvmiMIOCommLoopback::CancelAllCommands(const OsclAny* aContext)
{
    OSCL_UNUSED_ARG(aContext);
    OSCL_LEAVE(OsclErrNotSupported);
    return -1;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PvmiMIOCommLoopback::CancelCommand(PVMFCommandId aCmdId, const OsclAny* aContext)
{
    OSCL_UNUSED_ARG(aCmdId);
    OSCL_UNUSED_ARG(aContext);
    OSCL_LEAVE(OsclErrNotSupported);
    return -1;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PvmiMIOCommLoopback::setPeer(PvmiMediaTransfer* aPeer)
{
    iPeer = aPeer;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PvmiMIOCommLoopback::useMemoryAllocators(OsclMemAllocator* write_alloc)
{
    OSCL_UNUSED_ARG(write_alloc);
    OSCL_LEAVE(OsclErrNotSupported);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PvmiMIOCommLoopback::writeAsync(uint8 aFormatType, int32 aFormatIndex,
        uint8* aData, uint32 aDataLen,
        const PvmiMediaXferHeader& data_header_info,
        OsclAny* aContext)
{
    OSCL_UNUSED_ARG(aFormatType);
    OSCL_UNUSED_ARG(aFormatIndex);

    uint32 aSeqNum = data_header_info.seq_num;
    PVMFTimestamp aTimestamp = data_header_info.timestamp;

    // writeAsync will copy the input data, and then make
    // arrangements to send a writeComplete callback, and also
    // echo back the receieved data to its peer

    PVMFSharedMediaDataPtr mediaData;
    OsclRefCounterMemFrag frag;
    OsclSharedPtr<PVMFMediaDataImpl> mediaDataImpl;
    int32 err = 0, cmdId;
    if (iCommandCounter == 0x7FFFFFFF)
        iCommandCounter = 0;
    cmdId = iCommandCounter++;


    mediaDataImpl = iMediaDataAlloc.allocate(aDataLen);
    mediaDataImpl->getMediaFragment(0, frag);
    oscl_memcpy(frag.getMemFragPtr(), aData, aDataLen);

    OSCL_TRY(err, mediaData = PVMFMediaData::createMediaData(mediaDataImpl, &iMediaDataMemPool));
    OSCL_FIRST_CATCH_ANY(err, mediaDataImpl.Unbind();
                         OSCL_LEAVE(err));

    // Set timestamp
    mediaData->setTimestamp(aTimestamp);
    mediaData->setSeqNum(aSeqNum);
    mediaData->setMediaFragFilledLen(0, aDataLen);

    // Convert media data to MediaMsg, and store for loopback echo
    PVMFSharedMediaMsgPtr mediaMsg;
    convertToPVMFMediaMsg(mediaMsg, mediaData);

    iOutgoingQueue.push_back(mediaMsg);

    // Queue a writeComplete response for async sending
    cmdId = iCommandCounter++;
    WriteResponse resp(PVMFSuccess, cmdId, aContext, aTimestamp);
    iWriteResponseQueue.push_back(resp);
    RunIfNotReady();
    return cmdId;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PvmiMIOCommLoopback::writeComplete(PVMFStatus aStatus, PVMFCommandId aCmdId,
        OsclAny* aContext)

{
    OSCL_UNUSED_ARG(aStatus);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO,
                    (0, "PvmiMIOCommLoopback::writeComplete status %d cmdId %d context 0x%x", aStatus, aCmdId, aContext));


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
                if (iSettings.iTestObserver)
                {
                    PVMFTimestamp ts = mediaData->getTimestamp();
                    iSettings.iTestObserver->LoopbackPos(ts);
                }
                iCleanupQueue.erase(&iCleanupQueue[i]);
                break;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PvmiMIOCommLoopback::readAsync(uint8* data, uint32 max_data_len,
        OsclAny* aContext, int32* formats, uint16 num_formats)
{
    OSCL_UNUSED_ARG(data);
    OSCL_UNUSED_ARG(max_data_len);
    OSCL_UNUSED_ARG(aContext);
    OSCL_UNUSED_ARG(formats);
    OSCL_UNUSED_ARG(num_formats);
    // This is an active data source. readAsync is not supported.
    OSCL_LEAVE(OsclErrNotSupported);
    return -1;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PvmiMIOCommLoopback::readComplete(PVMFStatus aStatus, PVMFCommandId read_cmd_id,
        int32 format_index, const PvmiMediaXferHeader& data_header_info,
        OsclAny* aContext)
{
    OSCL_UNUSED_ARG(aStatus);
    OSCL_UNUSED_ARG(read_cmd_id);
    OSCL_UNUSED_ARG(format_index);
    OSCL_UNUSED_ARG(data_header_info);
    OSCL_UNUSED_ARG(aContext);
    // This is an active data source. readComplete is not supported.
    OSCL_LEAVE(OsclErrNotSupported);
    return;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PvmiMIOCommLoopback::statusUpdate(uint32 status_flags)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO,
                    (0, "PvmiMIOCommLoopback::statusUpdate flags %d", status_flags));

    if (status_flags & PVMI_MEDIAXFER_STATUS_WRITE)
    {
        //recover from a previous async write error.
        if (iWriteState == EWriteWait)
        {
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


////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PvmiMIOCommLoopback::cancelCommand(PVMFCommandId aCmdId)
{
    OSCL_UNUSED_ARG(aCmdId);
    // This cancel command ( with a small "c" in cancel ) is for the media transfer interface.
    // implementation is similar to the cancel command of the media I/O interface.
    OSCL_LEAVE(OsclErrNotSupported);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PvmiMIOCommLoopback::cancelAllCommands()
{
    OSCL_LEAVE(OsclErrNotSupported);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PvmiMIOCommLoopback::setObserver(PvmiConfigAndCapabilityCmdObserver* aObserver)
{
    OSCL_UNUSED_ARG(aObserver);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PvmiMIOCommLoopback::getParametersSync(PvmiMIOSession session,
        PvmiKeyType identifier,
        PvmiKvp*& parameters,
        int& num_parameter_elements,
        PvmiCapabilityContext context)
{
    LOG_STACK_TRACE((0, "PvmiMIOCommLoopback::getParametersSync"));
    OSCL_UNUSED_ARG(session);
    OSCL_UNUSED_ARG(context);

    parameters = NULL;
    num_parameter_elements = 0;
    PVMFStatus status = PVMFFailure;

    if (pv_mime_strcmp(identifier, OUTPUT_FORMATS_CAP_QUERY) == 0 ||
            pv_mime_strcmp(identifier, OUTPUT_FORMATS_CUR_QUERY) == 0)
    {
        num_parameter_elements = 1;
        status = AllocateKvp(parameters,
                             OSCL_CONST_CAST(char*, OUTPUT_FORMATS_VALTYPE), num_parameter_elements);
        if (status != PVMFSuccess)
        {
            LOG_ERR((0, "PvmiMIOCommLoopback::GetOutputParametersSync: Error - AllocateKvp failed. status=%d", status));
        }
        else
        {
            parameters[0].value.pChar_value = OSCL_STATIC_CAST(mbchar*, iSettings.iMediaFormat.getMIMEStrPtr());
        }
    }
    else if (pv_mime_strcmp(identifier, INPUT_FORMATS_CAP_QUERY) == 0)
    {
        num_parameter_elements = 1;
        status = AllocateKvp(parameters,
                             OSCL_CONST_CAST(char*, INPUT_FORMATS_VALTYPE), num_parameter_elements);
        if (status != PVMFSuccess)
        {
            LOG_ERR((0, "PvmiMIOCommLoopback::GetOutputParametersSync: Error - AllocateKvp failed. status=%d", status));
        }
        else
        {
            parameters[0].value.pChar_value = OSCL_STATIC_CAST(mbchar*, iSettings.iMediaFormat.getMIMEStrPtr());
        }
    }
    else if (pv_mime_strcmp(identifier, INPUT_TRANSFER_MODEL_VAL) == 0)
    {
        num_parameter_elements = 1;
        status = AllocateKvp(parameters,
                             OSCL_CONST_CAST(char*, INPUT_TRANSFER_MODEL_VAL), num_parameter_elements);
        if (status != PVMFSuccess)
        {
            LOG_ERR((0, "PvmiMIOCommLoopback::GetInputModelParametersSync: Error - AllocateKvp failed. status=%d", status));
        }
        else
        {
            parameters[0].value.uint32_value = 1; // push model for media input
        }
    }
    else if (pv_mime_strcmp(identifier, OUTPUT_TRANSFER_MODEL_VAL) == 0)
    {
        num_parameter_elements = 1;
        status = AllocateKvp(parameters,
                             OSCL_CONST_CAST(char*, OUTPUT_TRANSFER_MODEL_VAL), num_parameter_elements);
        if (status != PVMFSuccess)
        {
            LOG_ERR((0, "PvmiMIOCommLoopback::GetOutputModelParametersSync: Error - AllocateKvp failed. status=%d", status));
        }
        else
        {
            parameters[0].value.uint32_value = 1; // push model for media output
        }
    }
    return status;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PvmiMIOCommLoopback::releaseParameters(PvmiMIOSession session,
        PvmiKvp* parameters,
        int num_elements)
{
    OSCL_UNUSED_ARG(session);
    OSCL_UNUSED_ARG(num_elements);

    if (parameters)
    {
        iAlloc.deallocate((OsclAny*)parameters);
        return PVMFSuccess;
    }
    else
    {
        return PVMFFailure;
    }
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PvmiMIOCommLoopback::createContext(PvmiMIOSession session, PvmiCapabilityContext& context)
{
    OSCL_UNUSED_ARG(session);
    OSCL_UNUSED_ARG(context);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PvmiMIOCommLoopback::setContextParameters(PvmiMIOSession session,
        PvmiCapabilityContext& context,
        PvmiKvp* parameters, int num_parameter_elements)
{
    OSCL_UNUSED_ARG(session);
    OSCL_UNUSED_ARG(context);
    OSCL_UNUSED_ARG(parameters);
    OSCL_UNUSED_ARG(num_parameter_elements);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PvmiMIOCommLoopback::DeleteContext(PvmiMIOSession session, PvmiCapabilityContext& context)
{
    OSCL_UNUSED_ARG(session);
    OSCL_UNUSED_ARG(context);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PvmiMIOCommLoopback::setParametersSync(PvmiMIOSession session, PvmiKvp* parameters,
        int num_elements, PvmiKvp*& ret_kvp)
{
    OSCL_UNUSED_ARG(session);
    PVMFStatus status = PVMFSuccess;
    ret_kvp = NULL;

    for (int32 i = 0; i < num_elements; i++)
    {
        status = VerifyAndSetParameter(&(parameters[i]), true);
        if (status != PVMFSuccess)
        {
            LOG_ERR((0, "PvmiMIOCommLoopback::setParametersSync: Error - VerifiyAndSetParameter failed on parameter #%d", i));
            ret_kvp = &(parameters[i]);
        }
    }
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PvmiMIOCommLoopback::setParametersAsync(PvmiMIOSession session,
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
    OSCL_LEAVE(OsclErrNotSupported);
    return -1;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF uint32 PvmiMIOCommLoopback::getCapabilityMetric(PvmiMIOSession session)
{
    OSCL_UNUSED_ARG(session);
    return 0;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PvmiMIOCommLoopback::verifyParametersSync(PvmiMIOSession session,
        PvmiKvp* parameters, int num_elements)
{
    OSCL_UNUSED_ARG(session);
    OSCL_UNUSED_ARG(parameters);
    OSCL_UNUSED_ARG(num_elements);
    return PVMFErrNotSupported;
}

////////////////////////////////////////////////////////////////////////////
//                            Private methods
////////////////////////////////////////////////////////////////////////////
PvmiMIOCommLoopback::PvmiMIOCommLoopback(const PvmiMIOCommLoopbackSettings& aSettings)
        : OsclTimerObject(OsclActiveObject::EPriorityNominal, "PvmiMIOCommLoopback"),
        iCommandCounter(1000),
        iPeer(NULL),
        iThreadLoggedOn(false),
        iSettings(aSettings),
        iMediaDataAlloc(&iAlloc),
        iMediaDataMemPool(PVMIO_MEDIADATA_POOLNUM),
        iLogger(NULL),
        iState(STATE_IDLE),
        iWriteState(EWriteOK),
        iResend(false),
        iWriteAsyncContext(0)
{
    ConstructL(aSettings);
}

////////////////////////////////////////////////////////////////////////////
void PvmiMIOCommLoopback::ConstructL(const PvmiMIOCommLoopbackSettings& aSettings)
{
    iSettings.iMediaFormat = aSettings.iMediaFormat;
}

////////////////////////////////////////////////////////////////////////////
void PvmiMIOCommLoopback::Run()
{
    if (!iCmdQueue.empty())
    {
        PvmiMIOCommLoopbackCmd cmd = iCmdQueue[0];
        iCmdQueue.erase(iCmdQueue.begin());

        switch (cmd.iType)
        {

            case CMD_INIT:
                DoRequestCompleted(cmd, DoInit());
                break;

            case CMD_START:
                DoRequestCompleted(cmd, DoStart());
                break;

            case CMD_PAUSE:
                DoRequestCompleted(cmd, DoPause());
                break;

            case CMD_FLUSH:
                DoRequestCompleted(cmd, DoFlush());
                break;

            case CMD_RESET:
                DoRequestCompleted(cmd, DoReset());
                break;

            case CMD_STOP:
                DoRequestCompleted(cmd, DoStop());
                break;

            case CMD_QUERY_UUID:
            case CMD_QUERY_INTERFACE:
                DoRequestCompleted(cmd, PVMFSuccess);
                break;

            case CMD_CANCEL_ALL_COMMANDS:
            case CMD_CANCEL_COMMAND:
                DoRequestCompleted(cmd, PVMFFailure);
                break;

            default:
                break;
        }
    }

    if (!iCmdQueue.empty())
    {
        // Run again if there are more things to process
        RunIfNotReady();
    }

    if (!iOutgoingQueue.empty() && iWriteState == EWriteOK)
    {
        DoLoopback();
    }

    while (!iWriteResponseQueue.empty())
    {
        //report write complete
        if (iPeer)
        {
            iPeer->writeComplete(iWriteResponseQueue[0].iStatus, iWriteResponseQueue[0].iCmdId, (OsclAny*)iWriteResponseQueue[0].iContext);
            iWriteResponseQueue.erase(iWriteResponseQueue.begin());
        }
    }
}

////////////////////////////////////////////////////////////////////////////
PVMFCommandId PvmiMIOCommLoopback::AddCmdToQueue(PvmiMIOCommLoopbackCmdType aType,
        const OsclAny* aContext, OsclAny* aData1)
{
    if (aType == DATA_EVENT)
        OSCL_LEAVE(OsclErrArgument);

    PvmiMIOCommLoopbackCmd cmd;
    cmd.iType = aType;
    cmd.iContext = OSCL_STATIC_CAST(OsclAny*, aContext);
    cmd.iData1 = aData1;
    cmd.iId = iCommandCounter++;
    iCmdQueue.push_back(cmd);
    RunIfNotReady();
    return cmd.iId;
}

////////////////////////////////////////////////////////////////////////////
void PvmiMIOCommLoopback::DoRequestCompleted(const PvmiMIOCommLoopbackCmd& aCmd, PVMFStatus aStatus, OsclAny* aEventData)
{
    PVMFCmdResp response(aCmd.iId, aCmd.iContext, aStatus, aEventData);

    for (uint32 i = 0; i < iObservers.size(); i++)
        iObservers[i]->RequestCompleted(response);
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PvmiMIOCommLoopback::DoInit()
{
    iState = STATE_INITIALIZED;
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PvmiMIOCommLoopback::DoStart()
{
    iState = STATE_STARTED;
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PvmiMIOCommLoopback::DoPause()
{
    iState = STATE_PAUSED;
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PvmiMIOCommLoopback::DoFlush()
{
    // This method should stop capturing media data but continue to send captured
    // media data that is already in buffer and then go to stopped state.
    // However, in this case of file input we do not have such a buffer for
    // captured data, so this behaves the same way as stop.
    return DoStop();
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PvmiMIOCommLoopback::DoStop()
{
    iState = STATE_STOPPED;
    return PVMFSuccess;
}

PVMFStatus PvmiMIOCommLoopback::DoReset()
{
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PvmiMIOCommLoopback::AllocateKvp(PvmiKvp*& aKvp, PvmiKeyType aKey, int32 aNumParams)
{
    LOG_STACK_TRACE((0, "PvmiMIOCommLoopback::AllocateKvp"));
    uint8* buf = NULL;
    uint32 keyLen = oscl_strlen(aKey) + 1;
    int32 err = 0;

    OSCL_TRY(err,
             buf = (uint8*)iAlloc.allocate(aNumParams * (sizeof(PvmiKvp) + keyLen));
             if (!buf)
             OSCL_LEAVE(OsclErrNoMemory);
            );
    OSCL_FIRST_CATCH_ANY(err,
                         LOG_ERR((0, "PvmiMIOCommLoopback::AllocateKvp: Error - kvp allocation failed"));
                         return PVMFErrNoMemory;
                        );

    int32 i = 0;
    PvmiKvp* curKvp = aKvp = new(buf) PvmiKvp;
    buf += sizeof(PvmiKvp);
    for (i = 1; i < aNumParams; i++)
    {
        curKvp += i;
        curKvp = new(buf) PvmiKvp;
        buf += sizeof(PvmiKvp);
    }

    for (i = 0; i < aNumParams; i++)
    {
        aKvp[i].key = (char*)buf;
        oscl_strncpy(aKvp[i].key, aKey, keyLen);
        buf += keyLen;
    }

    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PvmiMIOCommLoopback::VerifyAndSetParameter(PvmiKvp* aKvp, bool aSetParam)
{
    OSCL_UNUSED_ARG(aSetParam);
    LOG_STACK_TRACE((0, "PvmiMIOCommLoopback::VerifyAndSetParameter: aKvp=0x%x, aSetParam=%d", aKvp, aSetParam));

    if (!aKvp)
    {
        LOG_ERR((0, "PvmiMIOCommLoopback::VerifyAndSetParameter: Error - Invalid key-value pair"));
        return PVMFFailure;
    }

    if (pv_mime_strcmp(aKvp->key, OUTPUT_FORMATS_VALTYPE) == 0)
    {
        if (pv_mime_strcmp(iSettings.iMediaFormat.getMIMEStrPtr(), aKvp->value.pChar_value) == 0)
        {
            return PVMFSuccess;
        }
        else
        {
            LOG_ERR((0, "PvmiMIOCommLoopback::VerifyAndSetParameter: Error - Unsupported format %s",
                     aKvp->value.pChar_value));
            return PVMFFailure;
        }
    }
    if (pv_mime_strcmp(aKvp->key, INPUT_FORMATS_VALTYPE) == 0)
    {
        if (pv_mime_strcmp(iSettings.iMediaFormat.getMIMEStrPtr(), aKvp->value.pChar_value) == 0)
        {
            return PVMFSuccess;
        }
        else
        {
            LOG_ERR((0, "PvmiMIOCommLoopback::VerifyAndSetParameter: Error - Unsupported format %s",
                     aKvp->value.pChar_value));
            return PVMFFailure;
        }
    }

    LOG_ERR((0, "PvmiMIOCommLoopback::VerifyAndSetParameter: Error - Unsupported parameter"));
    return PVMFFailure;
}


void PvmiMIOCommLoopback::DoLoopback()
{
    PVMFSharedMediaMsgPtr aMsg;
    PVMFSharedMediaDataPtr aMediaData;
    uint32 fragment = 0;
    uint32 fragindex;

    while (!iOutgoingQueue.empty())
    {
        aMsg = iOutgoingQueue[0];

        convertToPVMFMediaData(aMediaData, aMsg);

        if (aMsg->getFormatID() == PVMF_MEDIA_CMD_EOS_FORMAT_ID)
        {
            return;
        }

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
            err = WriteAsync(cmdId, frag, data_hdr);

            if (err != OsclErrNone)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "PVCommsIONodePort::SendData WriteAsync Leave code %d", err));

                //if a leave occurs in the writeAsync call, we suspend data
                //transfer until a statusUpdate call from the MIO component
                //tells us to resume.
                //this is not an error-- it's the normal flow control mechanism.
                iWriteState = EWriteWait;

                //save the data to re-send later.
                iResend = true;
                iResendFragment = fragindex;
                iResendSeqNum = aMediaData->getSeqNum();

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
                    }
                    //else the write already completed synchronously.
                    iOutgoingQueue.erase(iOutgoingQueue.begin());
                }
                iWriteState = EWriteOK;
            }
        }
    }
}

int32 PvmiMIOCommLoopback::WriteAsync(int32& cmdId,
                                      OsclRefCounterMemFrag frag,
                                      PvmiMediaXferHeader data_hdr)
{
    int32 err;
    OSCL_TRY(err,
             cmdId = iPeer->writeAsync(PVMI_MEDIAXFER_FMT_TYPE_DATA,  /*format_type*/
                                       PVMI_MEDIAXFER_FMT_INDEX_DATA, /*format_index*/
                                       (uint8*)frag.getMemFragPtr(),
                                       frag.getMemFragSize(),
                                       data_hdr,
                                       (OsclAny*)iWriteAsyncContext);
            );
    return err;
}

