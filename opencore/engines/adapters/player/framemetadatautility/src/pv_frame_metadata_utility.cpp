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
#include "pv_frame_metadata_utility.h"
#include "pv_player_interface.h"
#include "pv_player_factory.h"
#include "pv_frame_metadata_mio_video.h"
#include "pv_media_output_node_factory.h"
#include "pv_mime_string_utils.h"
#include "oscl_mem_mempool.h"

#define PVFMUTIL_TIMERID_PLAYERERRORTIMEOUT 1
#define PVFMUTIL_ERRORHANDLINGTIMEOUT_VALUE 30

#define PVFMUTIL_TIMERID_FRAMEREADYTIMEOUT 2
#define PVFMUTIL_FRAMEREADYTIMEOUT_VALUE_DEFAULT 30

static const char PVFMUTIL_FRAMERETRIEVAL_TIMEOUT_KEY[] = "x-pvmf/fmu/timeout-frameretrieval-in-seconds;valtype=uint32";

#define PVFMUTIL_VIDEOFRAMEBUFFER_WIDTH 320
#define PVFMUTIL_VIDEOFRAMEBUFFER_HEIGHT 240
#define PVFMUTIL_VIDEOFRAMEBUFFER_MAXSIZE PVFMUTIL_VIDEOFRAMEBUFFER_WIDTH * PVFMUTIL_VIDEOFRAMEBUFFER_HEIGHT * 3
#define PVFMUTIL_VIDEOFRAMEBUFFER_MEMPOOL_BUFFERSIZE PVFMUTIL_VIDEOFRAMEBUFFER_MAXSIZE*2

PVFrameAndMetadataUtility* PVFrameAndMetadataUtility::New(char *aOutputFormatMIMEType, PVCommandStatusObserver *aCmdObserver,
        PVErrorEventObserver *aErrorObserver, PVInformationalEventObserver *aInfoObserver)
{
    if (aOutputFormatMIMEType == NULL || aCmdObserver == NULL ||
            aErrorObserver == NULL || aInfoObserver == NULL)
    {
        OSCL_LEAVE(OsclErrArgument);
        return NULL;
    }

    PVFrameAndMetadataUtility* util = NULL;
    util = OSCL_NEW(PVFrameAndMetadataUtility, ());
    if (util)
    {
        util->Construct(aOutputFormatMIMEType,
                        aCmdObserver,
                        aErrorObserver,
                        aInfoObserver);
    }

    return util;
}


PVFrameAndMetadataUtility::~PVFrameAndMetadataUtility()
{
    Cancel();

    if (!iPendingCmds.empty())
    {
        iPendingCmds.pop();
    }

    if (iPlayer != NULL)
    {
        PVPlayerFactory::DeletePlayer(iPlayer);
        iPlayer = NULL;
    }

    if (iVideoFrameBufferMemPool)
    {
        iVideoFrameBufferMemPool->removeRef();
        iVideoFrameBufferMemPool = NULL;
    }

    // Shutdown and destroy the timer
    if (iTimeoutTimer)
    {
        iTimeoutTimer->Clear();
    }
    // Memoryleak
    iVideoDataSink.SetDataSinkNode(NULL);
    if (iVideoNode)
    {
        PVMediaOutputNodeFactory::DeleteMediaOutputNode(iVideoNode);
        iVideoNode = NULL;
    }
    if (iVideoMIO)
    {
        OSCL_DELETE(iVideoMIO);
        iVideoMIO = NULL;
    }
    // Cleanup the audio data sink
    iAudioDataSink.SetDataSinkNode(NULL);
    if (iAudioNode)
    {
        PVMediaOutputNodeFactory::DeleteMediaOutputNode(iAudioNode);
        iAudioNode = NULL;
    }
    if (iAudioMIO)
    {
        OSCL_DELETE(iAudioMIO);
        iAudioMIO = NULL;
    }
    // Remove the data source handle
    iDataSource = NULL;

    OSCL_DELETE(iTimeoutTimer);
}

PVMFStatus PVFrameAndMetadataUtility::SetMode(uint32 aMode)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::SetMode() = %d", aMode));
    // Mode can have values of only 1, 2 or 3.
    if ((aMode != PV_FRAME_METADATA_INTERFACE_MODE_SOURCE_METADATA_ONLY) &&
            (aMode != PV_FRAME_METADATA_INTERFACE_MODE_SOURCE_METADATA_AND_THUMBNAIL) &&
            (aMode != PV_FRAME_METADATA_INTERFACE_MODE_ALL))
    {
        return PVMFErrArgument;
    }


#ifdef SUPPORT_PARSER_LEVEL_METADATA_EXTRACTION_ONLY
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::SetMode in NOT supported in current utility(), Value(= %d) is ignored and default(1) is used", aMode));
    iMode = PV_FRAME_METADATA_INTERFACE_MODE_SOURCE_METADATA_ONLY;
    return PVMFSuccess;
#else
    if (iState == PVFM_UTILITY_STATE_IDLE)
    {
        iMode = aMode;
        return PVMFSuccess;
    }
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::SetMode() - Invalid State"));
    return PVMFErrInvalidState;
#endif
}

PVCommandId PVFrameAndMetadataUtility::QueryUUID(const PvmfMimeString& aMimeType, Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids,
        bool aExactUuidsOnly, const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::QueryUUID()"));

    Oscl_Vector<PVFMUtilityCommandParamUnion, OsclMemAllocator> paramvec;
    paramvec.reserve(3);
    paramvec.clear();
    PVFMUtilityCommandParamUnion param;
    param.pOsclAny_value = (OsclAny*) & aMimeType;
    paramvec.push_back(param);
    param.pOsclAny_value = (OsclAny*) & aUuids;
    paramvec.push_back(param);
    param.bool_value = aExactUuidsOnly;
    paramvec.push_back(param);
    return AddCommandToQueue(PVFM_UTILITY_COMMAND_QUERY_UUID, (OsclAny*)aContextData, &paramvec);
}


PVCommandId PVFrameAndMetadataUtility::QueryInterface(const PVUuid& aUuid, PVInterface*& aInterfacePtr, const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::QueryInterface()"));

    Oscl_Vector<PVFMUtilityCommandParamUnion, OsclMemAllocator> paramvec;
    paramvec.reserve(1);
    paramvec.clear();
    PVFMUtilityCommandParamUnion param;
    param.pOsclAny_value = (OsclAny*) & aInterfacePtr;
    paramvec.push_back(param);
    return AddCommandToQueue(PVFM_UTILITY_COMMAND_QUERY_INTERFACE, (OsclAny*)aContextData, &paramvec, &aUuid);
}


PVCommandId PVFrameAndMetadataUtility::CancelAllCommands(const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::CancelAllCommands()"));

    return AddCommandToQueue(PVFM_UTILITY_COMMAND_CANCEL_ALL_COMMANDS, (OsclAny*)aContextData);
}


PVCommandId PVFrameAndMetadataUtility::GetState(PVFrameAndMetadataState& aState, const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::GetState()"));

    Oscl_Vector<PVFMUtilityCommandParamUnion, OsclMemAllocator> paramvec;
    paramvec.reserve(1);
    paramvec.clear();
    PVFMUtilityCommandParamUnion param;
    param.pOsclAny_value = (OsclAny*) & aState;
    paramvec.push_back(param);
    return AddCommandToQueue(PVFM_UTILITY_COMMAND_GET_STATE, (OsclAny*)aContextData, &paramvec);
}


PVMFStatus PVFrameAndMetadataUtility::GetStateSync(PVFrameAndMetadataState& aState)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::GetStateSync()"));

    Oscl_Vector<PVFMUtilityCommandParamUnion, OsclMemAllocator> paramvec;
    paramvec.reserve(1);
    paramvec.clear();
    PVFMUtilityCommandParamUnion param;
    param.pOsclAny_value = (OsclAny*) & aState;
    paramvec.push_back(param);
    PVFMUtilityCommand cmd(PVFM_UTILITY_COMMAND_GET_STATE, -1, NULL, &paramvec);
    return DoGetState(cmd, true);
}


PVCommandId PVFrameAndMetadataUtility::AddDataSource(PVPlayerDataSource& aDataSource, const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::AddDataSource()"));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                    (0, "PVFrameAndMetadataUtility::AddDataSource called Tick=%d", OsclTickCount::TickCount()));

    Oscl_Vector<PVFMUtilityCommandParamUnion, OsclMemAllocator> paramvec;
    paramvec.reserve(1);
    paramvec.clear();
    PVFMUtilityCommandParamUnion param;
    param.pOsclAny_value = (OsclAny*) & aDataSource;
    paramvec.push_back(param);
    return AddCommandToQueue(PVFM_UTILITY_COMMAND_ADD_DATA_SOURCE, (OsclAny*)aContextData, &paramvec);
}


PVCommandId PVFrameAndMetadataUtility::GetMetadataKeys(PVPMetadataList& aKeyList, int32 aStartingIndex, int32 aMaxEntries,
        char* aQueryKey, const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::GetMetadataKeys()"));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                    (0, "PVFrameAndMetadataUtility::GetMetadataKeys called Tick=%d", OsclTickCount::TickCount()));

    Oscl_Vector<PVFMUtilityCommandParamUnion, OsclMemAllocator> paramvec;
    paramvec.reserve(4);
    paramvec.clear();
    PVFMUtilityCommandParamUnion param;
    param.pOsclAny_value = (OsclAny*) & aKeyList;
    paramvec.push_back(param);
    param.int32_value = aStartingIndex;
    paramvec.push_back(param);
    param.int32_value = aMaxEntries;
    paramvec.push_back(param);
    param.pChar_value = aQueryKey;
    paramvec.push_back(param);
    return AddCommandToQueue(PVFM_UTILITY_COMMAND_GET_METADATA_KEYS, (OsclAny*)aContextData, &paramvec);
}


PVCommandId PVFrameAndMetadataUtility::GetMetadataValues(PVPMetadataList& aKeyList, int32 aStartingValueIndex, int32 aMaxValueEntries,
        int32& aNumAvailableValueEntries, Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList,
        const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::GetMetadataValues()"));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                    (0, "PVFrameAndMetadataUtility::GetMetadataValues called Tick=%d", OsclTickCount::TickCount()));

    Oscl_Vector<PVFMUtilityCommandParamUnion, OsclMemAllocator> paramvec;
    paramvec.reserve(5);
    paramvec.clear();
    PVFMUtilityCommandParamUnion param;
    param.pOsclAny_value = (OsclAny*) & aKeyList;
    paramvec.push_back(param);
    param.int32_value = aStartingValueIndex;
    paramvec.push_back(param);
    param.int32_value = aMaxValueEntries;
    paramvec.push_back(param);
    param.pOsclAny_value = (OsclAny*) & aNumAvailableValueEntries;
    paramvec.push_back(param);
    param.pOsclAny_value = (OsclAny*) & aValueList;
    paramvec.push_back(param);
    return AddCommandToQueue(PVFM_UTILITY_COMMAND_GET_METADATA_VALUES, (OsclAny*)aContextData, &paramvec);
}

void PVFrameAndMetadataUtility::setParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters, int32 aNumElements, PvmiKvp* &aRetKVP)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::SetParametersSync()"));
    OSCL_UNUSED_ARG(aSession);

    Oscl_Vector<PVFMUtilityCommandParamUnion, OsclMemAllocator> paramvec;
    paramvec.reserve(3);
    paramvec.clear();
    PVFMUtilityCommandParamUnion param;
    param.pOsclAny_value = (OsclAny*)aParameters;
    paramvec.push_back(param);
    param.int32_value = (int32)aNumElements;
    paramvec.push_back(param);
    param.pOsclAny_value = (OsclAny*) & aRetKVP;
    paramvec.push_back(param);

    PVFMUtilityCommand cmd(PVFM_UTILITY_COMMAND_SET_PARAMETERS, -1, NULL, &paramvec);

    DoCapConfigSetParameters(cmd, true);
}


PVCommandId PVFrameAndMetadataUtility::GetFrame(PVFrameSelector& aFrameInfo, uint8* aProvidedFrameBuffer,
        uint32& aBufferSize, PVFrameBufferProperty& aBufferProp, const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::GetFrame() User provided buffer version"));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                    (0, "PVFrameAndMetadataUtility::GetFrame called Tick=%d", OsclTickCount::TickCount()));

    Oscl_Vector<PVFMUtilityCommandParamUnion, OsclMemAllocator> paramvec;
    paramvec.reserve(3);
    paramvec.clear();
    PVFMUtilityCommandParamUnion param;
    param.pOsclAny_value = (OsclAny*) & aFrameInfo;
    paramvec.push_back(param);
    param.pUint8_value = aProvidedFrameBuffer;
    paramvec.push_back(param);
    param.pUint32_value = &aBufferSize;
    paramvec.push_back(param);
    param.pOsclAny_value = (OsclAny*) & aBufferProp;
    paramvec.push_back(param);
    return AddCommandToQueue(PVFM_UTILITY_COMMAND_GET_FRAME_USER_BUFFER, (OsclAny*)aContextData, &paramvec);
}


PVCommandId PVFrameAndMetadataUtility::GetFrame(PVFrameSelector& aFrameInfo, uint8** aFrameBufferPtr,
        uint32& aBufferSize, PVFrameBufferProperty& aBufferProp, const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::GetFrame() Utility provided buffer version"));

    Oscl_Vector<PVFMUtilityCommandParamUnion, OsclMemAllocator> paramvec;
    paramvec.reserve(4);
    paramvec.clear();
    PVFMUtilityCommandParamUnion param;
    param.pOsclAny_value = (OsclAny*) & aFrameInfo;
    paramvec.push_back(param);
    param.pOsclAny_value = (OsclAny*)aFrameBufferPtr;
    paramvec.push_back(param);
    param.pUint32_value = &aBufferSize;
    paramvec.push_back(param);
    param.pOsclAny_value = (OsclAny*) & aBufferProp;
    paramvec.push_back(param);
    return AddCommandToQueue(PVFM_UTILITY_COMMAND_GET_FRAME_UTILITY_BUFFER, (OsclAny*)aContextData, &paramvec);
}


PVCommandId PVFrameAndMetadataUtility::ReturnBuffer(uint8* aFrameBufferPtr, const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::ReturnBuffer()"));

    Oscl_Vector<PVFMUtilityCommandParamUnion, OsclMemAllocator> paramvec;
    paramvec.reserve(1);
    paramvec.clear();
    PVFMUtilityCommandParamUnion param;
    param.pUint8_value = aFrameBufferPtr;
    paramvec.push_back(param);
    return AddCommandToQueue(PVFM_UTILITY_COMMAND_RETURN_BUFFER, (OsclAny*)aContextData, &paramvec);
}


PVCommandId PVFrameAndMetadataUtility::RemoveDataSource(PVPlayerDataSource& aDataSource, const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::RemoveDataSource()"));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                    (0, "PVFrameAndMetadataUtility::RemoveDataSource called Tick=%d", OsclTickCount::TickCount()));

    Oscl_Vector<PVFMUtilityCommandParamUnion, OsclMemAllocator> paramvec;
    paramvec.reserve(1);
    paramvec.clear();
    PVFMUtilityCommandParamUnion param;
    param.pOsclAny_value = (OsclAny*) & aDataSource;
    paramvec.push_back(param);
    return AddCommandToQueue(PVFM_UTILITY_COMMAND_REMOVE_DATA_SOURCE, (OsclAny*)aContextData, &paramvec);
}


PVFrameAndMetadataUtility::PVFrameAndMetadataUtility() :
        OsclTimerObject(OsclActiveObject::EPriorityNominal, "PVFrameMetadataUtility"),
        iCommandId(0),
        iState(PVFM_UTILITY_STATE_IDLE),
        iCmdStatusObserver(NULL),
        iErrorEventObserver(NULL),
        iInfoEventObserver(NULL),
        iPlayer(NULL),
        iOutputFormatType(PVMF_MIME_FORMAT_UNKNOWN),
        iDataSource(NULL),
        iVideoNode(NULL),
        iVideoMIO(NULL),
        iAudioNode(NULL),
        iAudioMIO(NULL),
        iLogger(NULL),
        iErrorHandlingInUtilityAO(false),
        iVideoFrameBufferMemPool(NULL),
        iCurrentVideoFrameBuffer(NULL),
        iVideoFrameBufferSize(NULL),
        iVideoFrameSelector(NULL),
        iVideoFrameBufferProp(NULL),
        iFrameReceived(false),
        iPlayerStartCompleted(false),
        iAPICmdStatus(PVMFSuccess),
        iAPICmdErrMsg(NULL),
        iTimeoutTimer(NULL),
        iErrorHandlingWaitTime(PVFMUTIL_ERRORHANDLINGTIMEOUT_VALUE),
        iFrameReadyWaitTime(PVFMUTIL_FRAMEREADYTIMEOUT_VALUE_DEFAULT),
        iThumbnailWidth(PVFMUTIL_VIDEOFRAMEBUFFER_WIDTH),
        iThumbnailHeight(PVFMUTIL_VIDEOFRAMEBUFFER_HEIGHT)
{
    //define this Macro in mmp build file only if mode 1 of FrMU is required.
#ifdef SUPPORT_PARSER_LEVEL_METADATA_EXTRACTION_ONLY
    iMode = PV_FRAME_METADATA_INTERFACE_MODE_SOURCE_METADATA_ONLY;
#else
    iMode = PV_FRAME_METADATA_INTERFACE_MODE_ALL;
#endif
    iPlayerCapConfigIF = NULL;
    iPlayerCapConfigIFPVI = NULL;
}


void PVFrameAndMetadataUtility::Construct(char *aOutputFormatMIMEType, PVCommandStatusObserver *aCmdObserver,
        PVErrorEventObserver *aErrorObserver, PVInformationalEventObserver *aInfoObserver)
{
    OSCL_ASSERT(aOutputFormatMIMEType != NULL);

    iOutputFormatType = aOutputFormatMIMEType;

    if (iOutputFormatType == PVMF_MIME_FORMAT_UNKNOWN)
    {
        OSCL_LEAVE(OsclErrArgument);
        return;
    }
    iCmdStatusObserver = aCmdObserver;
    iErrorEventObserver = aErrorObserver;
    iInfoEventObserver = aInfoObserver;

    // Create the player instance
    iPlayer = PVPlayerFactory::CreatePlayer(this, this, this);
    OSCL_ASSERT(iPlayer != NULL);

    // Allocate memory for vectors
    // If a leave occurs, let it bubble up
    iCurrentCmd.reserve(1);
    iCurrentCmd.clear();
    iCmdToCancel.reserve(1);
    iCmdToCancel.clear();
    iPendingCmds.reserve(4);

    // Add this AO to the scheduler
    AddToScheduler();

    // Retrieve the logger object
    iLogger = PVLogger::GetLoggerObject("PVFrameAndMetadataUtility");
    iPerfLogger = PVLogger::GetLoggerObject("fmudiagnostics");


    // Initialize the OSCL timer for timeouts
    iTimeoutTimer = OSCL_NEW(OsclTimer<OsclMemAllocator>, ("pvfmutility_timeout"));
    iTimeoutTimer->SetObserver(this);
    iTimeoutTimer->SetFrequency(1);  // 1 sec resolution
}


void PVFrameAndMetadataUtility::Run()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::Run() In"));
    int32 leavecode = 0;

    if (iErrorHandlingInUtilityAO)
    {
        iErrorHandlingInUtilityAO = false;

        // Forcibly restart the player engine
        PVMFStatus retval = DoPlayerShutdownRestart();
        if (retval != PVMFSuccess)
        {
            iAPICmdStatus = retval;
        }
        // Cleanup the video data sink
        iVideoDataSink.SetDataSinkNode(NULL);
        if (iVideoNode)
        {
            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iVideoNode);
            iVideoNode = NULL;
        }
        if (iVideoMIO)
        {
            OSCL_DELETE(iVideoMIO);
            iVideoMIO = NULL;
        }
        // Cleanup the audio data sink
        iAudioDataSink.SetDataSinkNode(NULL);
        if (iAudioNode)
        {
            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iAudioNode);
            iAudioNode = NULL;
        }
        if (iAudioMIO)
        {
            OSCL_DELETE(iAudioMIO);
            iAudioMIO = NULL;
        }
        // Remove the data source handle
        iDataSource = NULL;

        // Cancel any pending timers
        if (iTimeoutTimer)
        {
            iTimeoutTimer->Clear();
        }

        SetUtilityState(PVFM_UTILITY_STATE_IDLE);

        // Complete any command if waiting to be completed
        if (iCurrentCmd.empty() == false)
        {
            UtilityCommandCompleted(iCurrentCmd[0].GetCmdId(), iCurrentCmd[0].GetContext(), iAPICmdStatus, OSCL_STATIC_CAST(PVInterface*, iAPICmdErrMsg));
        }
        iAPICmdStatus = PVMFSuccess;
        if (iAPICmdErrMsg)
        {
            iAPICmdErrMsg->removeRef();
            iAPICmdErrMsg = NULL;
        }
        return;
    }

    // Check if CancelAll() request was made
    if (!iPendingCmds.empty())
    {
        if (iPendingCmds.top().GetCmdType() == PVFM_UTILITY_COMMAND_CANCEL_ALL_COMMANDS)
        {
            // Process it right away
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::Run() Processing CancelAllCommands() request"));
            PVFMUtilityCommand cmd(iPendingCmds.top());
            iPendingCmds.pop();
            DoCancelAllCommands(cmd);
            return;
        }
    }

    // Handle other requests normally
    if (!iPendingCmds.empty() && iCurrentCmd.empty())
    {
        // Retrieve the first pending command from queue
        PVFMUtilityCommand cmd(iPendingCmds.top());
        iPendingCmds.pop();

        // Put in on the current command queue
        leavecode = 0;
        OSCL_TRY(leavecode, iCurrentCmd.push_front(cmd));
        OSCL_FIRST_CATCH_ANY(leavecode,
                             PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::Run() Command could not be pushed onto iCurrentCmd vector"));
                             UtilityCommandCompleted(cmd.GetCmdId(), cmd.GetContext(), PVMFErrNoMemory);
                             OSCL_ASSERT(false);
                             return;);

        // Process the command according to the cmd type
        PVMFStatus cmdstatus = PVMFSuccess;
        switch (cmd.GetCmdType())
        {
            case PVFM_UTILITY_COMMAND_QUERY_UUID:
                cmdstatus = DoQueryUUID(cmd);;
                break;

            case PVFM_UTILITY_COMMAND_QUERY_INTERFACE:
                cmdstatus = DoQueryInterface(cmd);
                break;

            case PVFM_UTILITY_COMMAND_GET_STATE:
                cmdstatus = DoGetState(cmd, false);
                break;

            case PVFM_UTILITY_COMMAND_ADD_DATA_SOURCE:
                cmdstatus = DoAddDataSource(cmd);
                break;

            case PVFM_UTILITY_COMMAND_GET_METADATA_KEYS:
                cmdstatus = DoGetMetadataKeys(cmd);
                break;

            case PVFM_UTILITY_COMMAND_GET_METADATA_VALUES:
                cmdstatus = DoGetMetadataValues(cmd);
                break;

            case PVFM_UTILITY_COMMAND_SET_PARAMETERS:
                cmdstatus = DoCapConfigSetParameters(cmd, false);
                break;

            case PVFM_UTILITY_COMMAND_GET_FRAME_USER_BUFFER:
            case PVFM_UTILITY_COMMAND_GET_FRAME_UTILITY_BUFFER:
                cmdstatus = DoGetFrame(cmd);
                break;

            case PVFM_UTILITY_COMMAND_RETURN_BUFFER:
                cmdstatus = DoReturnBuffer(cmd);
                break;

            case PVFM_UTILITY_COMMAND_REMOVE_DATA_SOURCE:
                cmdstatus = DoRemoveDataSource(cmd);
                break;

            case PVFM_UTILITY_COMMAND_CANCEL_ALL_COMMANDS:
                // CancelAll() should not be handled here
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::Run() CancelAllCommands should be not handled in here. Asserting."));
                OSCL_ASSERT(false);
                // Just handle as "not supported"
                cmdstatus = PVMFErrNotSupported;
                break;

            case PVFM_UTILITY_COMMAND_HANDLE_PLAYER_ERROR:
                // Internal command so should not be handled here
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::Run() Internal error handling command should be not handled in here. Asserting."));
                OSCL_ASSERT(false);
                // Just handle as "not supported"
                cmdstatus = PVMFErrNotSupported;
                break;

            default:
                cmdstatus = PVMFErrNotSupported;
                break;
        }

        if (cmdstatus != PVMFSuccess)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::Run() Command failed CmdId %d Status %d", cmd.GetCmdId(), cmdstatus));
            UtilityCommandCompleted(cmd.GetCmdId(), cmd.GetContext(), cmdstatus);
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::Run() Out"));
}


void PVFrameAndMetadataUtility::CommandCompleted(const PVCmdResponse& aResponse)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::CommandCompleted() In"));

    // Check if a cancel command on player engine completed
    int32* context_int32 = (int32*)(aResponse.GetContext());
    if (context_int32 == &iCancelContext)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::CommandCompleted() Player engine cancel command completed"));

        // Check if the player engine state matches the utility's expected state
        PVPlayerState playerstate = PVP_STATE_IDLE;
        iPlayer->GetPVPlayerStateSync(playerstate);
        if (playerstate == PVP_STATE_IDLE && iState != PVFM_UTILITY_STATE_IDLE)
        {
            iDataSource = NULL;
            iState = PVFM_UTILITY_STATE_IDLE;
        }

        // Complete the utility's CancelAllCommands() request
        iCmdToCancel.clear();
        UtilityCommandCompleted(iCurrentCmd[0].GetCmdId(), iCurrentCmd[0].GetContext(), PVMFSuccess);
        return;
    }

    // Ignore other player command completion if cancelling
    if (!iCmdToCancel.empty())
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::CommandCompleted() Player command completion ignored due to cancel process"));
        iUtilityContext.iCmdType = -1;
        return;
    }

    // Process normal player engine command completions
    if (aResponse.GetContext() == (OsclAny*)&iUtilityContext)
    {
        PVFMUtilityContext* context = (PVFMUtilityContext*)(aResponse.GetContext());
        switch (context->iCmdType)
        {
            case PVFM_CMD_PlayerQueryUUID:
                HandlePlayerQueryUUID(*context, aResponse);
                break;

            case PVFM_CMD_PlayerQueryInterface:
            case PVFM_CMD_PlayerQueryCapConfigInterface:
                HandlePlayerQueryInterface(*context, aResponse);
                break;

            case PVFM_CMD_ADSPlayerAddDataSource:
                HandleADSPlayerAddDataSource(*context, aResponse);
                break;

            case PVFM_CMD_ADSPlayerInit:
                HandleADSPlayerInit(*context, aResponse);
                break;

            case PVFM_CMD_ADSPlayerAddVideoDataSink:
                HandleADSPlayerAddVideoDataSink(*context, aResponse);
                break;

            case PVFM_CMD_ADSPlayerAddAudioDataSink:
                HandleADSPlayerAddAudioDataSink(*context, aResponse);
                break;

            case PVFM_CMD_ADSPlayerPrepare:
                HandleADSPlayerPrepare(*context, aResponse);
                break;

            case PVFM_CMD_ADSPlayerStart:
                HandleADSPlayerStart(*context, aResponse);
                break;

            case PVFM_CMD_ADSPlayerPause:
                HandleADSPlayerPause(*context, aResponse);
                break;

            case PVFM_CMD_PlayerGetMetadataKeys:
                HandlePlayerGetMetadataKeys(*context, aResponse);
                break;

            case PVFM_CMD_PlayerGetMetadataValues:
                HandlePlayerGetMetadataValues(*context, aResponse);
                break;

            case PVFM_CMD_PlayerSetParametersSync:
                HandlePlayerSetParametersSync(*context, aResponse);
                break;

            case PVFM_CMD_GFPlayerStopFromPaused:
                HandleGFPlayerStopFromPaused(*context, aResponse);
                break;

            case PVFM_CMD_GFPlayerPrepare:
                HandleGFPlayerPrepare(*context, aResponse);
                break;

            case PVFM_CMD_GFPlayerStart:
                HandleGFPlayerStart(*context, aResponse);
                break;

            case PVFM_CMD_GFPlayerPause:
                HandleGFPlayerPause(*context, aResponse);
                break;

            case PVFM_CMD_RDSPlayerStopFromPaused:
                HandleRDSPlayerStopFromPaused(*context, aResponse);
                break;

            case PVFM_CMD_RDSPlayerRemoveVideoDataSink:
                HandleRDSPlayerRemoveVideoDataSink(*context, aResponse);
                break;

            case PVFM_CMD_RDSPlayerRemoveAudioDataSink:
                HandleRDSPlayerRemoveAudioDataSink(*context, aResponse);
                break;

            case PVFM_CMD_RDSPlayerReset:
                HandleRDSPlayerReset(*context, aResponse);
                break;

            case PVFM_CMD_RDSPlayerRemoveDataSource:
                HandleRDSPlayerRemoveDataSource(*context, aResponse);
                break;

            default:
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::CommandCompleted() Unknown player command type. Asserting"));
                OSCL_ASSERT(false);
                break;
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::CommandCompleted() Out"));
}


void PVFrameAndMetadataUtility::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::HandleErrorEvent() In"));

    // Check the player state
    PVPlayerState pstate;
    iPlayer->GetPVPlayerStateSync(pstate);

    switch (pstate)
    {
        case PVP_STATE_ERROR:
        {
            if (iCurrentCmd.empty() == true)
            {
                // Since error occurred while not processing a command so put in an internal
                // utility command so a pending command would not be processing while handling error
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::HandleErrorEvent() Queuing an internal command for error handlin"));
                PVFMUtilityCommand errorcmd(PVFM_UTILITY_COMMAND_HANDLE_PLAYER_ERROR, -1, NULL, NULL, false);
                iCurrentCmd.push_front(errorcmd);
            }

            // Wait for error handling to complete
            PVMFErrorInfoMessageInterface* nextmsg = NULL;
            if (aEvent.GetEventExtensionInterface())
            {
                nextmsg = GetErrorInfoMessageInterface(*(aEvent.GetEventExtensionInterface()));
            }
            PVMFBasicErrorInfoMessage* errmsg = NULL;
            errmsg = CreateBasicErrInfoMessage(nextmsg);

            iAPICmdStatus = (PVMFStatus)(aEvent.GetEventType());
            if (iAPICmdErrMsg)
            {
                iAPICmdErrMsg->removeRef();
                iAPICmdErrMsg = NULL;
            }
            iAPICmdErrMsg = errmsg;

            // Start a timer just in case the player does not report error handling complete
            iTimeoutTimer->Request(PVFMUTIL_TIMERID_PLAYERERRORTIMEOUT, 0, iErrorHandlingWaitTime, this, false);
        }
        break;

        case PVP_STATE_IDLE:
            if (iState == PVFM_UTILITY_STATE_IDLE)
            {
                // Just report the error event up to the app
                iErrorEventObserver->HandleErrorEvent(aEvent);
                break;
            }
            OSCL_ASSERT(iErrorHandlingInUtilityAO == false);
            iErrorHandlingInUtilityAO = true;
            RunIfNotReady();
            break;

        default:
            // Need to shutdown/restart player and cleanup in utility's AO
            OSCL_ASSERT(iErrorHandlingInUtilityAO == false);
            iErrorHandlingInUtilityAO = true;
            RunIfNotReady();
            break;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::HandleErrorEvent() Out"));
}


void PVFrameAndMetadataUtility::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::HandleInformationalEvent() In"));

    // If error handling complete event, complete the utility command
    // that was waiting for player to complete error handling
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingComplete)
    {
        if (iAPICmdStatus == PVMFSuccess)
        {
            // Error handling wait timed out so ignore this event
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::HandleInformationalEvent() Error handling already timed out so ignoring this event from player."));
            return;
        }

        // Cancel the timeout timer
        iTimeoutTimer->Cancel(PVFMUTIL_TIMERID_PLAYERERRORTIMEOUT);

        // Check the player state and make it sure matches with expected one
        PVPlayerState pstate;
        iPlayer->GetPVPlayerStateSync(pstate);

        switch (pstate)
        {
            case PVP_STATE_INITIALIZED:
                if (iState == PVFM_UTILITY_STATE_IDLE || iState == PVFM_UTILITY_STATE_RESETTING)
                {
                    // This should not happen
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::HandleInformationalEvent() Player is initialized even though util is in idle. Asserting."));
                    OSCL_ASSERT(false);

                    // Need to shutdown/restart player and cleanup in utility's AO
                    OSCL_ASSERT(iErrorHandlingInUtilityAO == false);
                    iErrorHandlingInUtilityAO = true;
                    RunIfNotReady();
                    return;
                }
                else
                {
                    SetUtilityState(PVFM_UTILITY_STATE_INITIALIZED);

                    // If the current command is AddDataSource(), player engine
                    // is in initialized state so complete the command with success
                    if (iCurrentCmd.empty() == false)
                    {
                        if (iCurrentCmd[0].GetCmdType() == PVFM_UTILITY_COMMAND_ADD_DATA_SOURCE)
                        {
                            //if the mode is in which prepare of engine is called and prepare is failing
                            //we cannot send successs of adddata source
                            if (iMode == PV_FRAME_METADATA_INTERFACE_MODE_SOURCE_METADATA_ONLY)
                            {
                                iAPICmdStatus = PVMFSuccess;
                            }

                            if (iAPICmdErrMsg)
                            {
                                iAPICmdErrMsg->removeRef();
                                iAPICmdErrMsg = NULL;
                            }

                            // Cleanup the video data sink
                            iVideoDataSink.SetDataSinkNode(NULL);
                            if (iVideoNode)
                            {
                                PVMediaOutputNodeFactory::DeleteMediaOutputNode(iVideoNode);
                                iVideoNode = NULL;
                            }
                            if (iVideoMIO)
                            {
                                OSCL_DELETE(iVideoMIO);
                                iVideoMIO = NULL;
                            }
                            // Cleanup the audio data sink
                            iAudioDataSink.SetDataSinkNode(NULL);
                            if (iAudioNode)
                            {
                                PVMediaOutputNodeFactory::DeleteMediaOutputNode(iAudioNode);
                                iAudioNode = NULL;
                            }
                            if (iAudioMIO)
                            {
                                OSCL_DELETE(iAudioMIO);
                                iAudioMIO = NULL;
                            }
                        }
                    }
                }
                break;

            case PVP_STATE_IDLE:
                if (iState != PVFM_UTILITY_STATE_IDLE)
                {
                    // Player went back to idle state so change utility's state to idle as well
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::HandleInformationalEvent() Player went back to idle so set utility to idle."));

                    // Cleanup the video data sink
                    iVideoDataSink.SetDataSinkNode(NULL);
                    if (iVideoNode)
                    {
                        PVMediaOutputNodeFactory::DeleteMediaOutputNode(iVideoNode);
                        iVideoNode = NULL;
                    }
                    if (iVideoMIO)
                    {
                        OSCL_DELETE(iVideoMIO);
                        iVideoMIO = NULL;
                    }
                    // Cleanup the audio data sink
                    iAudioDataSink.SetDataSinkNode(NULL);
                    if (iAudioNode)
                    {
                        PVMediaOutputNodeFactory::DeleteMediaOutputNode(iAudioNode);
                        iAudioNode = NULL;
                    }
                    if (iAudioMIO)
                    {
                        OSCL_DELETE(iAudioMIO);
                        iAudioMIO = NULL;
                    }
                    // Remove the data source handle
                    iDataSource = NULL;

                    SetUtilityState(PVFM_UTILITY_STATE_IDLE);
                }
                break;

            default:
                // Player should not be in any other state when
                // error handling completes
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::HandleInformationalEvent() Unexpected player state after error handling. Asserting."));
                OSCL_ASSERT(false);
                // Need to shutdown/restart player and cleanup in utility's AO
                OSCL_ASSERT(iErrorHandlingInUtilityAO == false);
                iErrorHandlingInUtilityAO = true;
                RunIfNotReady();
                break;
        }

        // Report the command as failed if command completion was pending
        if (iCurrentCmd.empty() == false)
        {
            UtilityCommandCompleted(iCurrentCmd[0].GetCmdId(), iCurrentCmd[0].GetContext(), iAPICmdStatus, OSCL_STATIC_CAST(PVInterface*, iAPICmdErrMsg));
        }
        iAPICmdStatus = PVMFSuccess;
        if (iAPICmdErrMsg)
        {
            iAPICmdErrMsg->removeRef();
            iAPICmdErrMsg = NULL;
        }
    }
    else
    {
        // Just pass up other info events up to app
        iInfoEventObserver->HandleInformationalEvent(aEvent);
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::HandleInformationalEvent() Out"));
}


void PVFrameAndMetadataUtility::HandleFrameReadyEvent(PVMFStatus aEventStatus)
{
    if (iCurrentCmd[0].GetCmdType() == PVFM_UTILITY_COMMAND_GET_FRAME_USER_BUFFER ||
            iCurrentCmd[0].GetCmdType() == PVFM_UTILITY_COMMAND_GET_FRAME_UTILITY_BUFFER)
    {
        iFrameReceived = true;

        // Cancel the timeout timer
        iTimeoutTimer->Cancel(PVFMUTIL_TIMERID_FRAMEREADYTIMEOUT);

        iAPICmdStatus = aEventStatus;

        if (aEventStatus != PVMFSuccess)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::HandleFrameReadyEvent() Frame retrieval from video MIO failed."));

            if (iCurrentCmd[0].GetCmdType() == PVFM_UTILITY_COMMAND_GET_FRAME_UTILITY_BUFFER && iCurrentVideoFrameBuffer)
            {
                // Return the buffer if allocated from utility's mempool
                iVideoFrameBufferMemPool->deallocate(iCurrentVideoFrameBuffer);
                iCurrentVideoFrameBuffer = NULL;
            }

            PVUuid puuid = PVFrameAndMetadataErrorInfoEventTypesUUID;
            PVMFBasicErrorInfoMessage* errmsg = NULL;
            int32 leavecode = 0;
            OSCL_TRY(leavecode, errmsg = OSCL_NEW(PVMFBasicErrorInfoMessage, (PVFMErrMIOComponent, puuid, NULL)));
            OSCL_FIRST_CATCH_ANY(leavecode,
                                 PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::HandleFrameReadyEvent() Instantiation of error msg did a leave!"));
                                 errmsg = NULL;
                                );

            if (iAPICmdErrMsg)
            {
                iAPICmdErrMsg->removeRef();
                iAPICmdErrMsg = NULL;
            }
            iAPICmdErrMsg = errmsg;
        }
        else
        {
            OSCL_ASSERT(iVideoFrameBufferProp != NULL);
            // Retrieve the video frame properties from MIO component
            uint32 fw = 0;
            uint32 fh = 0;
            uint32 dw = 0;
            uint32 dh = 0;
            if (iVideoMIO->GetFrameProperties(fw, fh, dw, dh) == PVMFSuccess)
            {
                iVideoFrameBufferProp->iFrameWidth = fw;
                iVideoFrameBufferProp->iFrameHeight = fh;
                iVideoFrameBufferProp->iDisplayWidth = dw;
                iVideoFrameBufferProp->iDisplayHeight = dh;
            }
            else
            {
                iVideoFrameBufferProp->iFrameWidth = 0;
                iVideoFrameBufferProp->iFrameHeight = 0;
                iVideoFrameBufferProp->iDisplayWidth = 0;
                iVideoFrameBufferProp->iDisplayHeight = 0;
            }
        }

        // Initiate pause on player if start already completed
        if (iPlayerStartCompleted)
        {
            PVMFStatus retval = DoGFPlayerPause(iCurrentCmd[0].GetCmdId(), iCurrentCmd[0].GetContext());
            if (retval == PVMFErrInvalidState)
            {
                // Playback already paused so GetFrame() command completed
                if (iAPICmdStatus != PVMFSuccess)
                {
                    if (iCurrentCmd[0].GetCmdType() == PVFM_UTILITY_COMMAND_GET_FRAME_UTILITY_BUFFER && iCurrentVideoFrameBuffer)
                    {
                        // Return the buffer if allocated from utility's mempool
                        iVideoFrameBufferMemPool->deallocate(iCurrentVideoFrameBuffer);
                        iCurrentVideoFrameBuffer = NULL;
                    }
                }

                UtilityCommandCompleted(iCurrentCmd[0].GetCmdId(), iCurrentCmd[0].GetContext(), iAPICmdStatus, OSCL_STATIC_CAST(PVInterface*, iAPICmdErrMsg));

                iAPICmdStatus = PVMFSuccess;
                if (iAPICmdErrMsg)
                {
                    iAPICmdErrMsg->removeRef();
                    iAPICmdErrMsg = NULL;
                }
            }
            else if (retval != PVMFSuccess)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::HandleFrameReadyEvent() Stop on player failed. Report command as failed"));

                if (iCurrentCmd[0].GetCmdType() == PVFM_UTILITY_COMMAND_GET_FRAME_UTILITY_BUFFER && iCurrentVideoFrameBuffer)
                {
                    // Return the buffer if allocated from utility's mempool
                    iVideoFrameBufferMemPool->deallocate(iCurrentVideoFrameBuffer);
                    iCurrentVideoFrameBuffer = NULL;
                }

                if (iAPICmdStatus == PVMFSuccess)
                {
                    iAPICmdStatus = retval;
                }

                // Need to shutdown/restart player and cleanup in utility's AO
                OSCL_ASSERT(iErrorHandlingInUtilityAO == false);
                iErrorHandlingInUtilityAO = true;
                RunIfNotReady();
            }
        }
        // Else just wait for player to report start completed
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::HandleFrameReadyEvent() Frame ready received outside of GetFrame() command. Asserting."));
        //OSCL_ASSERT(false);
    }
}


void PVFrameAndMetadataUtility::TimeoutOccurred(int32 timerID, int32 /*timeoutInfo*/)
{
    if (timerID == PVFMUTIL_TIMERID_PLAYERERRORTIMEOUT)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::TimeoutOccurred() Timer for error handling timeout triggered"));

        if (iAPICmdStatus == PVMFSuccess)
        {
            // Error handling wait timed out so ignore this event
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::TimeoutOccurred() Error handling already reported complete by player."));
            return;
        }

        // Check the player state and make it sure matches with expected one
        PVPlayerState pstate;
        iPlayer->GetPVPlayerStateSync(pstate);
        PVFrameAndMetadataState ustate;
        ustate = GetUtilityState();

        switch (pstate)
        {
            case PVP_STATE_INITIALIZED:
                if (ustate == PVFM_STATE_IDLE)
                {
                    // This should not happen
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::TimeoutOccurred() Player is initialized even though util is in idle. Asserting."));
                    OSCL_ASSERT(false);
                }
                break;

            case PVP_STATE_IDLE:
                if (ustate != PVFM_STATE_IDLE)
                {
                    // Player went back to idle state so change utility's state to idle as well
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::TimeoutOccurred() Player went back to idle so set utility to idle."));
                    SetUtilityState(PVFM_UTILITY_STATE_IDLE);
                }
                break;

            default:
                // Player should not be in any other state when
                // error handling completes
                OSCL_ASSERT(false);
                break;
        }

        // Report the command as failed
        OSCL_ASSERT(iCurrentCmd.empty() == false);
        UtilityCommandCompleted(iCurrentCmd[0].GetCmdId(), iCurrentCmd[0].GetContext(), iAPICmdStatus, OSCL_STATIC_CAST(PVInterface*, iAPICmdErrMsg));
        iAPICmdStatus = PVMFSuccess;
        if (iAPICmdErrMsg)
        {
            iAPICmdErrMsg->removeRef();
            iAPICmdErrMsg = NULL;
        }
    }
    else if (timerID == PVFMUTIL_TIMERID_FRAMEREADYTIMEOUT)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::TimeoutOccurred() Frame retrieval from video MIO timed out."));

        // Cancel the pending frame retrieval
        iVideoMIO->CancelGetFrame();

        if (iCurrentCmd[0].GetCmdType() == PVFM_UTILITY_COMMAND_GET_FRAME_UTILITY_BUFFER && iCurrentVideoFrameBuffer)
        {
            // Return the buffer if allocated from utility's mempool
            iVideoFrameBufferMemPool->deallocate(iCurrentVideoFrameBuffer);
            iCurrentVideoFrameBuffer = NULL;
        }

        // Create an error message
        PVUuid puuid = PVFrameAndMetadataErrorInfoEventTypesUUID;
        PVMFBasicErrorInfoMessage* errmsg = NULL;
        int32 leavecode = 0;
        OSCL_TRY(leavecode, errmsg = OSCL_NEW(PVMFBasicErrorInfoMessage, (PVFMErrMIOComponent, puuid, NULL)));
        OSCL_FIRST_CATCH_ANY(leavecode,
                             PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::TimeoutOccurred() Instantiation of error msg did a leave!"));
                             errmsg = NULL;
                            );

        iAPICmdStatus = PVMFErrTimeout;
        if (iAPICmdErrMsg)
        {
            iAPICmdErrMsg->removeRef();
            iAPICmdErrMsg = NULL;
        }
        iAPICmdErrMsg = errmsg;

        // Timer is started after player start completes so initiate pause on player
        PVMFStatus retval = DoGFPlayerPause(iCurrentCmd[0].GetCmdId(), iCurrentCmd[0].GetContext());
        if (retval == PVMFErrInvalidState)
        {
            // Playback already paused so GetFrame() command completed
            UtilityCommandCompleted(iCurrentCmd[0].GetCmdId(), iCurrentCmd[0].GetContext(), iAPICmdStatus, OSCL_STATIC_CAST(PVInterface*, iAPICmdErrMsg));

            iAPICmdStatus = PVMFSuccess;
            if (iAPICmdErrMsg)
            {
                iAPICmdErrMsg->removeRef();
                iAPICmdErrMsg = NULL;
            }
        }
        else if (retval != PVMFSuccess)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::TimeoutOccurred() Pause on player failed. Report command as failed"));
            // Need to shutdown/restart player and cleanup in utility's AO
            OSCL_ASSERT(iErrorHandlingInUtilityAO == false);
            iErrorHandlingInUtilityAO = true;
            RunIfNotReady();
        }
    }
    else
    {
        OSCL_ASSERT(false);
    }
}


PVCommandId PVFrameAndMetadataUtility::AddCommandToQueue(int32 aCmdType, OsclAny* aContextData, Oscl_Vector < PVFMUtilityCommandParamUnion,
        OsclMemAllocator > * aParamVector, const PVUuid* aUuid, bool aAPICommand)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::AddCommandToQueue() In CmdType %d, CmdId %d", aCmdType, iCommandId));

    PVFMUtilityCommand cmd(aCmdType, iCommandId, aContextData, aParamVector, aAPICommand);
    if (aUuid)
    {
        cmd.SetUuid(*aUuid);
    }

    int32 leavecode = 0;
    OSCL_TRY(leavecode, iPendingCmds.push(cmd));
    OSCL_FIRST_CATCH_ANY(leavecode,
                         PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::AddCommandToQueue() Adding command to pending command list did a leave!"));
                         OSCL_ASSERT(false);
                         return -1;);

    RunIfNotReady();

    ++iCommandId;
    if (iCommandId == 0x7FFFFFFF)
    {
        iCommandId = 0;
    }


    return cmd.GetCmdId();
}


void PVFrameAndMetadataUtility::UtilityCommandCompleted(PVCommandId aId, OsclAny* aContext, PVMFStatus aStatus,
        PVInterface* aExtInterface, OsclAny* aEventData, int32 aEventDataSize)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::UtilityCommandCompleted() In CmdId %d, Status %d", aId, aStatus));

    // Update the current command vector

    // Assert if the current cmd is not saved or the cmd ID does not match
    OSCL_ASSERT(iCurrentCmd.size() == 1);
    OSCL_ASSERT(iCurrentCmd[0].GetCmdId() == aId);

    // Empty out the current cmd vector and set active if there are other pending commands
    PVFMUtilityCommand completedcmd(iCurrentCmd[0]);
    iCurrentCmd.erase(iCurrentCmd.begin());
    if (!iPendingCmds.empty())
    {
        RunIfNotReady();
    }

    // Send the command completed event
    if (iCmdStatusObserver)
    {
        if (aId != -1 && completedcmd.IsAPICommand())
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::UtilityCommandCompleted() Notifying utility command as completed. CmdId %d Status %d", aId, aStatus));
            PVCmdResponse cmdcompleted(aId, aContext, aStatus, aExtInterface, aEventData, aEventDataSize);
            iCmdStatusObserver->CommandCompleted(cmdcompleted);
        }
        else
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::UtilityCommandCompleted() aId is -1 or not an API command. CmdType %d", completedcmd.GetCmdType()));
        }
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::UtilityCommandCompleted() iCmdStatusObserver is NULL"));
    }
}

void PVFrameAndMetadataUtility::setObserver(PvmiConfigAndCapabilityCmdObserver* aObserver)
{
    OSCL_UNUSED_ARG(aObserver);
    OsclError::Leave(OsclErrNotSupported);
}

PVMFStatus PVFrameAndMetadataUtility::getParametersSync(PvmiMIOSession aSession, PvmiKeyType aIdentifier, PvmiKvp*& aParameters, int& aNumParamElements, PvmiCapabilityContext aContext)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aIdentifier);
    OSCL_UNUSED_ARG(aParameters);
    OSCL_UNUSED_ARG(aNumParamElements);
    OSCL_UNUSED_ARG(aContext);

    return PVMFErrNotSupported;
}

PVMFStatus PVFrameAndMetadataUtility::releaseParameters(PvmiMIOSession aSession, PvmiKvp* aParameters, int aNumElements)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aParameters);
    OSCL_UNUSED_ARG(aNumElements);

    return PVMFErrNotSupported;
}

void PVFrameAndMetadataUtility::createContext(PvmiMIOSession aSession, PvmiCapabilityContext& aContext)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aContext);
    OsclError::Leave(OsclErrNotSupported);
}

void PVFrameAndMetadataUtility::DeleteContext(PvmiMIOSession aSession, PvmiCapabilityContext& aContext)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aContext);
}

void PVFrameAndMetadataUtility::setContextParameters(PvmiMIOSession aSession, PvmiCapabilityContext& aContext, PvmiKvp* aParameters, int aNumParamElements)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aParameters);
    OSCL_UNUSED_ARG(aNumParamElements);
    OSCL_UNUSED_ARG(aContext);
}

PVMFCommandId PVFrameAndMetadataUtility::setParametersAsync(PvmiMIOSession aSession, PvmiKvp* aParameters, int aNumElements, PvmiKvp*& aRetKVP, OsclAny* aContext)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aParameters);
    OSCL_UNUSED_ARG(aNumElements);
    OSCL_UNUSED_ARG(aRetKVP);
    OSCL_UNUSED_ARG(aContext);

    return PVMFErrNotSupported;
}

uint32 PVFrameAndMetadataUtility::getCapabilityMetric(PvmiMIOSession aSession)
{
    OSCL_UNUSED_ARG(aSession);
    OsclError::Leave(OsclErrNotSupported);
    return 0;
}

PVMFStatus PVFrameAndMetadataUtility::verifyParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters, int aNumElements)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aParameters);
    OSCL_UNUSED_ARG(aNumElements);

    return PVMFErrNotSupported;
}


void PVFrameAndMetadataUtility::SetUtilityState(PVFMUtilityState aState)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::SetUtilityState() In Current state %d, New state %d", iState, aState));
    iState = aState;
}


PVFrameAndMetadataState PVFrameAndMetadataUtility::GetUtilityState(void)
{
    switch (iState)
    {
        case PVFM_UTILITY_STATE_IDLE:
        case PVFM_UTILITY_STATE_INITIALIZING:
            return PVFM_STATE_IDLE;

        case PVFM_UTILITY_STATE_INITIALIZED:
        case PVFM_UTILITY_STATE_RESETTING:
            return PVFM_STATE_INITIALIZED;

        case PVFM_UTILITY_STATE_HANDLINGERROR:
        case PVFM_UTILITY_STATE_ERROR:
            return PVFM_STATE_ERROR;

        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::GetUtilityState() Unknown utility state. Asserting"));
            OSCL_ASSERT(false);
            break;
    }

    return PVFM_STATE_ERROR;
}


void PVFrameAndMetadataUtility::DoCancelAllCommands(PVFMUtilityCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoCancelAllCommands() In"));

    // Utility cannot be processing another cancel command
    OSCL_ASSERT(iCmdToCancel.empty() == true);

    // Cancel the current command first
    if (iCurrentCmd.size() == 1)
    {
        // First save the current command being processed
        iCmdToCancel.push_front(iCurrentCmd[0]);
        // Cancel it
        UtilityCommandCompleted(iCurrentCmd[0].GetCmdId(), iCurrentCmd[0].GetContext(), PVMFErrCancelled);
    }

    // Cancel all the pending commands
    while (!iPendingCmds.empty())
    {
        // Retrieve the pending command from queue so it can be cancelled
        PVFMUtilityCommand cmd(iPendingCmds.top());
        iPendingCmds.pop();
        // Save it temporary as "current command" and then cancel it
        iCurrentCmd.push_front(cmd);
        UtilityCommandCompleted(cmd.GetCmdId(), cmd.GetContext(), PVMFErrCancelled);
    }

    // Make the CancelAllCommands() command the current command
    iCurrentCmd.push_front(aCmd);

    // Check if there was an ongoing command that needs to be properly cancelled
    if (!iCmdToCancel.empty())
    {
        // Properly cancel a command being currently processed
        DoCancelCommandBeingProcessed();
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoCancelAllCommands() Out"));
    }
    else
    {
        // CancelAllCommands() command is completed so send the completion event
        UtilityCommandCompleted(aCmd.GetCmdId(), aCmd.GetContext(), PVMFSuccess);
    }
}


void PVFrameAndMetadataUtility::DoCancelCommandBeingProcessed()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoCancelCommandBeingProcessed() In"));

    switch (iCmdToCancel[0].GetCmdType())
    {
        case PVFM_UTILITY_COMMAND_QUERY_UUID:
        case PVFM_UTILITY_COMMAND_QUERY_INTERFACE:
        case PVFM_UTILITY_COMMAND_ADD_DATA_SOURCE:
        case PVFM_UTILITY_COMMAND_GET_METADATA_KEYS:
        case PVFM_UTILITY_COMMAND_GET_METADATA_VALUES:
        case PVFM_UTILITY_COMMAND_GET_FRAME_USER_BUFFER:
        case PVFM_UTILITY_COMMAND_GET_FRAME_UTILITY_BUFFER:
        case PVFM_UTILITY_COMMAND_RETURN_BUFFER:
        case PVFM_UTILITY_COMMAND_REMOVE_DATA_SOURCE:
            if (iUtilityContext.iCmdType != -1)
            {
                // Player command needs to be cancelled
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                                (0, "PVFrameAndMetadataUtility::CancelAllCommands Called Tick=%d", OsclTickCount::TickCount()));
                int32 leavecode = 0;
                OSCL_TRY(leavecode, iPlayer->CancelAllCommands((const OsclAny*) &iCancelContext));
                OSCL_FIRST_CATCH_ANY(leavecode,
                                     PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoCancelCommandBeingProcessed() Cancel on player engine did a leave. Asserting"));
                                     OSCL_ASSERT(false);
                                     iCmdToCancel.clear();
                                     UtilityCommandCompleted(iCurrentCmd[0].GetCmdId(), iCurrentCmd[0].GetContext(), PVMFSuccess));
            }
            else
            {
                // No pending player command to cancel so complete the CancelAllCommands()
                iCmdToCancel.clear();
                UtilityCommandCompleted(iCurrentCmd[0].GetCmdId(), iCurrentCmd[0].GetContext(), PVMFSuccess);
            }
            break;

        case PVFM_UTILITY_COMMAND_GET_STATE:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoCancelCommandBeingProcessed() Should not need to cancel GetState() since it completes in one Run."));
            // Complete the CancelAllCommands()
            UtilityCommandCompleted(iCurrentCmd[0].GetCmdId(), iCurrentCmd[0].GetContext(), PVMFSuccess);
            break;

        case PVFM_UTILITY_COMMAND_CANCEL_ALL_COMMANDS:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoCancelCommandBeingProcessed() Cannot cancel a CancelAllCommands(). Asserting"));
            OSCL_ASSERT(false);
            // Complete the current CancelAllCommands()
            UtilityCommandCompleted(iCurrentCmd[0].GetCmdId(), iCurrentCmd[0].GetContext(), PVMFSuccess);
            break;

        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoCancelCommandBeingProcessed() Unknown command to cancel. Asserting"));
            OSCL_ASSERT(false);
            // Complete the CancelAllCommands()
            UtilityCommandCompleted(iCurrentCmd[0].GetCmdId(), iCurrentCmd[0].GetContext(), PVMFSuccess);
            break;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoCancelCommandBeingProcessed() Out"));
}


PVMFStatus PVFrameAndMetadataUtility::DoQueryUUID(PVFMUtilityCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoQueryUUID() In"));

    PvmfMimeString* mimetype;
    Oscl_Vector<PVUuid, OsclMemAllocator> *uuidvec;
    bool exactmatch;

    mimetype = (PvmfMimeString*)(aCmd.GetParam(0).pOsclAny_value);
    uuidvec = (Oscl_Vector<PVUuid, OsclMemAllocator>*)(aCmd.GetParam(1).pOsclAny_value);
    exactmatch = aCmd.GetParam(2).bool_value;

    if (mimetype == NULL || uuidvec == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoQueryUUID() Passed in parameter invalid."));
        return PVMFErrArgument;
    }

    // For now, no extension interface available from utility

    // Call QueryUUID() on the player
    PVMFStatus cmdstatus = DoPlayerQueryUUID(aCmd.GetCmdId(), aCmd.GetContext(), *mimetype, *uuidvec, exactmatch);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoQueryUUID() Out"));
    return cmdstatus;
}


PVMFStatus PVFrameAndMetadataUtility::DoPlayerQueryUUID(PVCommandId aCmdId, OsclAny* aCmdContext,
        PvmfMimeString& aMIMEType, Oscl_Vector<PVUuid, OsclMemAllocator>& aUUIDVec, bool aExactMatch)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoPlayerQueryUUID() In"));

    iUtilityContext.iCmdId = aCmdId;
    iUtilityContext.iCmdContext = aCmdContext;
    iUtilityContext.iCmdType = PVFM_CMD_PlayerQueryUUID;
    int32 leavecode = 0;
    OSCL_ASSERT(iPlayer != NULL);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                    (0, "PVFrameAndMetadataUtility::QueryUUID Called Tick=%d", OsclTickCount::TickCount()));
    OSCL_TRY(leavecode, iPlayer->QueryUUID(aMIMEType, aUUIDVec, aExactMatch, (const OsclAny*)&iUtilityContext));
    OSCL_FIRST_CATCH_ANY(leavecode,
                         PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoPlayerQueryUUID() QueryUUID() on player did a leave!"));
                         return PVMFFailure;);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoPlayerQueryUUID() Out"));
    return PVMFSuccess;
}


PVMFStatus PVFrameAndMetadataUtility::DoQueryInterface(PVFMUtilityCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoQueryInterface() In"));

    PVInterface** ifptr = (PVInterface**)(aCmd.GetParam(0).pOsclAny_value);
    PVMFStatus cmdstatus = PVMFSuccess;

    iPlayerQueryIFUUID = aCmd.GetUuid();
    PVCommandId cmdid = aCmd.GetCmdId();
    OsclAny* context = aCmd.GetContext();

    if (ifptr == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoQueryInterface() Passed in parameter invalid."));
        return PVMFErrArgument;
    }

    if (queryInterface(iPlayerQueryIFUUID, *ifptr, cmdid, context) == false)
    {
        cmdstatus = PVMFErrNotSupported;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoQueryInterface() Out"));
    return cmdstatus;
}

bool PVFrameAndMetadataUtility::queryInterface(const PVUuid& uuid, PVInterface*& iface, PVCommandId cmdid, OsclAny* context)
{
    bool status = true;

    if (uuid == PVMI_CAPABILITY_AND_CONFIG_PVUUID)
    {
        PvmiCapabilityAndConfig* capconfigiface = OSCL_STATIC_CAST(PvmiCapabilityAndConfig*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, capconfigiface);

        // Call QueryInterface() on the player too in case of usage using setParametersSync()
        PVMFStatus cmdstatus = DoPlayerQueryInterface(cmdid, context, iPlayerQueryIFUUID, iPlayerCapConfigIFPVI);

        if (PVMFSuccess != cmdstatus)
            status = false;
    }
    else
    {
        // Call QueryInterface() on the player
        PVMFStatus cmdstatus = DoPlayerQueryInterface(cmdid, context, iPlayerQueryIFUUID, (PVInterface*&)iface);
        if (PVMFSuccess != cmdstatus)
            status = false;
    }
    return status;
}


PVMFStatus PVFrameAndMetadataUtility::DoPlayerQueryInterface(PVCommandId aCmdId, OsclAny* aCmdContext, PVUuid& aUuid, PVInterface*& aInterfacePtr)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoPlayerQueryInterface() In"));

    iUtilityContext.iCmdId = aCmdId;
    iUtilityContext.iCmdContext = aCmdContext;
    iUtilityContext.iCmdType = PVFM_CMD_PlayerQueryInterface;

    if (aUuid == PVMI_CAPABILITY_AND_CONFIG_PVUUID)
    {
        iUtilityContext.iCmdType = PVFM_CMD_PlayerQueryCapConfigInterface;
    }
    int32 leavecode = 0;
    OSCL_ASSERT(iPlayer != NULL);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                    (0, "PVFrameAndMetadataUtility::QueryInterface Called Tick=%d", OsclTickCount::TickCount()));
    OSCL_TRY(leavecode, iPlayer->QueryInterface(aUuid, aInterfacePtr, (const OsclAny*)&iUtilityContext));
    OSCL_FIRST_CATCH_ANY(leavecode,
                         PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoPlayerQueryInterface() QueryInterface() on player did a leave!"));
                         return PVMFFailure;);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoPlayerQueryInterface() Out"));
    return PVMFSuccess;
}


PVMFStatus PVFrameAndMetadataUtility::DoGetState(PVFMUtilityCommand& aCmd, bool aSyncCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoGetState() In"));

    PVFrameAndMetadataState* state = (PVFrameAndMetadataState*)(aCmd.GetParam(0).pOsclAny_value);
    if (state == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoGetState() Passed in parameter invalid."));
        return PVMFErrArgument;
    }

    // Get current utility state using internal function
    *state = GetUtilityState();

    if (!aSyncCmd)
    {
        UtilityCommandCompleted(aCmd.GetCmdId(), aCmd.GetContext(), PVMFSuccess);
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoGetState() Out"));
    return PVMFSuccess;
}


PVMFStatus PVFrameAndMetadataUtility::DoAddDataSource(PVFMUtilityCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoAddDataSource() In"));

    if (GetUtilityState() != PVFM_STATE_IDLE)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoAddDataSource() Wrong state"));
        return PVMFErrInvalidState;
    }

    if (aCmd.GetParam(0).pOsclAny_value == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoAddDataSource() Passed in parameter invalid."));
        return PVMFErrArgument;
    }

    // Save the data source
    iDataSource = (PVPlayerDataSource*)(aCmd.GetParam(0).pOsclAny_value);

    // Initiate the player setup sequence
    PVMFStatus cmdstatus = DoADSPlayerAddDataSource(aCmd.GetCmdId(), aCmd.GetContext());


    if (cmdstatus == PVMFSuccess)
    {
        SetUtilityState(PVFM_UTILITY_STATE_INITIALIZING);
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoAddDataSource() Out"));
    return cmdstatus;
}


PVMFStatus PVFrameAndMetadataUtility::DoADSPlayerAddDataSource(PVCommandId aCmdId, OsclAny* aCmdContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoADSPlayerAddDataSource() In"));

    iUtilityContext.iCmdId = aCmdId;
    iUtilityContext.iCmdContext = aCmdContext;
    iUtilityContext.iCmdType = PVFM_CMD_ADSPlayerAddDataSource;
    int32 leavecode = 0;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                    (0, "PVFrameAndMetadataUtility::PlayerADS called Tick=%d", OsclTickCount::TickCount()));

    OSCL_TRY(leavecode, iPlayer->AddDataSource(*iDataSource, (const OsclAny*)&iUtilityContext));
    OSCL_FIRST_CATCH_ANY(leavecode,
                         PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoADSPlayerAddDataSource() AddDataSource() on player did a leave!"));
                         return PVMFFailure;);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoADSPlayerAddDataSource() Out"));
    return PVMFSuccess;
}


PVMFStatus PVFrameAndMetadataUtility::DoADSPlayerInit(PVCommandId aCmdId, OsclAny* aCmdContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoADSPlayerInit() In"));

    iUtilityContext.iCmdId = aCmdId;
    iUtilityContext.iCmdContext = aCmdContext;
    iUtilityContext.iCmdType = PVFM_CMD_ADSPlayerInit;
    int32 leavecode = 0;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                    (0, "PVFrameAndMetadataUtility::PlayerInit() called Tick=%d", OsclTickCount::TickCount()));
    OSCL_TRY(leavecode, iPlayer->Init((const OsclAny*)&iUtilityContext));
    OSCL_FIRST_CATCH_ANY(leavecode,
                         PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoADSPlayerInit() Init() on player did a leave!"));
                         return PVMFFailure;);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoADSPlayerInit() Out"));
    return PVMFSuccess;
}


PVMFStatus PVFrameAndMetadataUtility::DoADSPlayerAddVideoDataSink(PVCommandId aCmdId, OsclAny* aCmdContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoADSPlayerAddVideoDataSink() In"));

    int32 leavecode = 0;
    OSCL_TRY(leavecode, iVideoMIO = OSCL_NEW(PVFMVideoMIO, ());
             iVideoNode = PVMediaOutputNodeFactory::CreateMediaOutputNode(iVideoMIO));
    OSCL_FIRST_CATCH_ANY(leavecode,
                         PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoADSPlayerAddVideoDataSink() Instantiation of MIO component and node for frame retrieval did a leave!"));
                         return PVMFErrNoMemory;
                        );
    iVideoDataSink.SetDataSinkNode(iVideoNode);
    iVideoDataSink.SetDataSinkFormatType(PVMF_MIME_YUV420);
    iVideoMIO->setThumbnailDimensions(iThumbnailWidth, iThumbnailHeight);

    iUtilityContext.iCmdId = aCmdId;
    iUtilityContext.iCmdContext = aCmdContext;
    iUtilityContext.iCmdType = PVFM_CMD_ADSPlayerAddVideoDataSink;

    leavecode = 0;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                    (0, "PVFrameAndMetadataUtility::PlayerAddVideoSink Called Tick=%d", OsclTickCount::TickCount()));

    OSCL_TRY(leavecode, iPlayer->AddDataSink(iVideoDataSink, (const OsclAny*)&iUtilityContext));
    OSCL_FIRST_CATCH_ANY(leavecode,
                         PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoADSPlayerAddVideoDataSink() AddDataSink() on player did a leave!"));
                         return PVMFFailure;);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoADSPlayerAddVideoDataSink() Out"));
    return PVMFSuccess;
}

PVMFStatus PVFrameAndMetadataUtility::DoADSPlayerAddAudioDataSink(PVCommandId aCmdId, OsclAny* aCmdContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoADSPlayerAddAudioDataSink() In"));

    int32 leavecode = 0;
    OSCL_TRY(leavecode, iAudioMIO = OSCL_NEW(PVFMAudioMIO, ());
             iAudioNode = PVMediaOutputNodeFactory::CreateMediaOutputNode(iAudioMIO));
    OSCL_FIRST_CATCH_ANY(leavecode,
                         PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoADSPlayerAddAudioDataSink() Instantiation of Audio MIO component and node for frame retrieval did a leave!"));
                         return PVMFErrNoMemory;
                        );
    iAudioDataSink.SetDataSinkNode(iAudioNode);
    iAudioDataSink.SetDataSinkFormatType(PVMF_MIME_PCM16);

    iUtilityContext.iCmdId = aCmdId;
    iUtilityContext.iCmdContext = aCmdContext;
    iUtilityContext.iCmdType = PVFM_CMD_ADSPlayerAddAudioDataSink;

    leavecode = 0;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                    (0, "PVFrameAndMetadataUtility::PlayerAddAudioSink Called Tick=%d", OsclTickCount::TickCount()));
    OSCL_TRY(leavecode, iPlayer->AddDataSink(iAudioDataSink, (const OsclAny*)&iUtilityContext));
    OSCL_FIRST_CATCH_ANY(leavecode,
                         PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoADSPlayerAddAudioDataSink() AddDataSink() on player did a leave!"));
                         return PVMFFailure;);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoADSPlayerAddAudioDataSink() Out"));
    return PVMFSuccess;
}

PVMFStatus PVFrameAndMetadataUtility::DoADSPlayerPrepare(PVCommandId aCmdId, OsclAny* aCmdContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoADSPlayerPrepare() In"));

    iUtilityContext.iCmdId = aCmdId;
    iUtilityContext.iCmdContext = aCmdContext;
    iUtilityContext.iCmdType = PVFM_CMD_ADSPlayerPrepare;
    int32 leavecode = 0;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                    (0, "PVFrameAndMetadataUtility::PlayerPrepare Called Tick=%d", OsclTickCount::TickCount()));
    OSCL_TRY(leavecode, iPlayer->Prepare((const OsclAny*)&iUtilityContext));
    OSCL_FIRST_CATCH_ANY(leavecode,
                         PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoADSPlayerPrepare() Prepare() on player did a leave!"));
                         return PVMFFailure;);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoADSPlayerPrepare() Out"));
    return PVMFSuccess;
}


PVMFStatus PVFrameAndMetadataUtility::DoADSPlayerStart(PVCommandId aCmdId, OsclAny* aCmdContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoADSPlayerStart() In"));

    iUtilityContext.iCmdId = aCmdId;
    iUtilityContext.iCmdContext = aCmdContext;
    iUtilityContext.iCmdType = PVFM_CMD_ADSPlayerStart;
    int32 leavecode = 0;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                    (0, "PVFrameAndMetadataUtility::PlayerStart Called Tick=%d", OsclTickCount::TickCount()));
    OSCL_TRY(leavecode, iPlayer->Start((const OsclAny*)&iUtilityContext));
    OSCL_FIRST_CATCH_ANY(leavecode,
                         PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoADSPlayerStart() Start() on player did a leave!"));
                         return PVMFFailure;);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoADSPlayerStart() Out"));
    return PVMFSuccess;
}


PVMFStatus PVFrameAndMetadataUtility::DoADSPlayerPause(PVCommandId aCmdId, OsclAny* aCmdContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoADSPlayerPause() In"));

    iUtilityContext.iCmdId = aCmdId;
    iUtilityContext.iCmdContext = aCmdContext;
    iUtilityContext.iCmdType = PVFM_CMD_ADSPlayerPause;
    int32 leavecode = 0;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                    (0, "PVFrameAndMetadataUtility::PlayerPause Called Tick=%d", OsclTickCount::TickCount()));
    OSCL_TRY(leavecode, iPlayer->Pause((const OsclAny*)&iUtilityContext));
    OSCL_FIRST_CATCH_ANY(leavecode,
                         PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoADSPlayerPause() Pause() on player did a leave!"));
                         return PVMFFailure;);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoADSPlayerPause() Out"));
    return PVMFSuccess;
}


PVMFStatus PVFrameAndMetadataUtility::DoGetMetadataKeys(PVFMUtilityCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoGetMetadataKeys() In"));

    if (GetUtilityState() == PVFM_STATE_IDLE || GetUtilityState() == PVFM_STATE_ERROR)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoGetMetadataKeys() Wrong state."));
        return PVMFErrInvalidState;
    }

    PVPMetadataList* keylist = (PVPMetadataList*)(aCmd.GetParam(0).pOsclAny_value);
    int32 startingkeyindex = aCmd.GetParam(1).int32_value;
    int32 maxkeyentries = aCmd.GetParam(2).int32_value;
    char* querykey = aCmd.GetParam(3).pChar_value;

    if (keylist == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoGetMetadataKeys() Passed in parameter invalid."));
        return PVMFErrArgument;
    }

    if (maxkeyentries < -1 || maxkeyentries == 0 || startingkeyindex < 0)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoGetMetadataKeys() Passed in parameter invalid."));
        return PVMFErrArgument;
    }

    // Call player engine API to retrieve metadata keys
    PVMFStatus cmdstatus = DoPlayerGetMetadataKeys(aCmd.GetCmdId(), aCmd.GetContext(), *keylist, startingkeyindex, maxkeyentries, querykey);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoGetMetadataKeys() Out"));
    return cmdstatus;
}


PVMFStatus PVFrameAndMetadataUtility::DoPlayerGetMetadataKeys(PVCommandId aCmdId, OsclAny* aCmdContext,
        PVPMetadataList& aKeyList, int32 aStartingIndex, int32 aMaxEntries, char* aQueryKey)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoPlayerGetMetadataKeys() In"));

    iUtilityContext.iCmdId = aCmdId;
    iUtilityContext.iCmdContext = aCmdContext;
    iUtilityContext.iCmdType = PVFM_CMD_PlayerGetMetadataKeys;
    int32 leavecode = 0;
    OSCL_ASSERT(iPlayer != NULL);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                    (0, "PVFrameAndMetadataUtility::PlayerGetMetaDataKeys Called Tick=%d", OsclTickCount::TickCount()));
    OSCL_TRY(leavecode, iPlayer->GetMetadataKeys(aKeyList, aStartingIndex, aMaxEntries, aQueryKey, (const OsclAny*)&iUtilityContext));
    OSCL_FIRST_CATCH_ANY(leavecode,
                         PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoPlayerGetMetadataKeys() GetMetadataKeys() on player did a leave!"));
                         return PVMFFailure;);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoPlayerGetMetadataKeys() Out"));
    return PVMFSuccess;
}


PVMFStatus PVFrameAndMetadataUtility::DoGetMetadataValues(PVFMUtilityCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoGetMetadataValues() In"));

    if (GetUtilityState() == PVFM_STATE_IDLE || GetUtilityState() == PVFM_STATE_ERROR)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoGetMetadataValues() Wrong state"));
        return PVMFErrInvalidState;
    }

    PVPMetadataList* keylist = (PVPMetadataList*)(aCmd.GetParam(0).pOsclAny_value);
    int32 startingvalueindex = aCmd.GetParam(1).int32_value;
    int32 maxvalueentries = aCmd.GetParam(2).int32_value;
    int32* numavailablevalues = (int32*)(aCmd.GetParam(3).pOsclAny_value);
    Oscl_Vector<PvmiKvp, OsclMemAllocator>* valuelist = (Oscl_Vector<PvmiKvp, OsclMemAllocator>*)(aCmd.GetParam(4).pOsclAny_value);

    if (keylist == NULL || valuelist == NULL || numavailablevalues == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoGetMetadataValues() Passed in parameter invalid."));
        return PVMFErrArgument;
    }

    if (maxvalueentries < -1 || maxvalueentries == 0 || startingvalueindex < 0)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoGetMetadataValues() Passed in parameter invalid."));
        return PVMFErrArgument;
    }

    // Call the player engine API to retrieve metadata values
    PVMFStatus cmdstatus = DoPlayerGetMetadataValues(aCmd.GetCmdId(), aCmd.GetContext(), *keylist, startingvalueindex, maxvalueentries, *numavailablevalues, *valuelist);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoGetMetadataValues() Out"));
    return cmdstatus;
}


PVMFStatus PVFrameAndMetadataUtility::DoPlayerGetMetadataValues(PVCommandId aCmdId, OsclAny* aCmdContext,
        PVPMetadataList& aKeyList, int32 aStartingValueIndex, int32 aMaxValueEntries,
        int32& aNumAvailableValueEntries, Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoPlayerGetMetadataValues() In"));

    iUtilityContext.iCmdId = aCmdId;
    iUtilityContext.iCmdContext = aCmdContext;
    iUtilityContext.iCmdType = PVFM_CMD_PlayerGetMetadataValues;
    int32 leavecode = 0;
    OSCL_ASSERT(iPlayer != NULL);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                    (0, "PVFrameAndMetadataUtility::PlayerGetMetaDataValues Called Tick=%d", OsclTickCount::TickCount()));
    OSCL_TRY(leavecode, iPlayer->GetMetadataValues(aKeyList, aStartingValueIndex, aMaxValueEntries, aNumAvailableValueEntries, aValueList, (const OsclAny*)&iUtilityContext));
    OSCL_FIRST_CATCH_ANY(leavecode,
                         PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoPlayerGetMetadataValues() GetMetadataValues() on player did a leave!"));
                         return PVMFFailure;);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoPlayerGetMetadataValues() Out"));
    return PVMFSuccess;
}

PVMFStatus PVFrameAndMetadataUtility::DoCapConfigSetParameters(PVFMUtilityCommand& aCmd, bool aSyncCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoCapConfigSetParameters() In"));

    PvmiKvp* paramkvp;
    int32 numparam;
    PvmiKvp** retkvp;
    paramkvp = (PvmiKvp*)(aCmd.GetParam(0).pOsclAny_value);
    numparam = aCmd.GetParam(1).int32_value;
    retkvp = (PvmiKvp**)(aCmd.GetParam(2).pOsclAny_value);

    if (paramkvp == NULL || retkvp == NULL || numparam < 1)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoCapConfigSetParameters() Passed in parameter invalid"));
        return PVMFErrArgument;
    }

    // Go through each parameter
    for (int32 paramind = 0; paramind < numparam; ++paramind)
    {
        // Count the number of components and parameters in the key
        int compcount = pv_mime_string_compcnt(paramkvp[paramind].key);
        // Retrieve the first component from the key string
        char* compstr = NULL;
        pv_mime_string_extract_type(0, paramkvp[paramind].key, compstr);

        if ((pv_mime_strcmp(compstr, _STRLIT_CHAR("x-pvmf")) < 0) || compcount < 2)
        {
            // First component should be "x-pvmf" and there must
            // be at least two components to go past x-pvmf
            *retkvp = &paramkvp[paramind];
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoCapConfigSetParameters() Unsupported key"));
            return PVMFErrArgument;
        }

        // Retrieve the second component from the key string
        pv_mime_string_extract_type(1, paramkvp[paramind].key, compstr);

        // First check if it is key string for fmu ("fmu")
        if (pv_mime_strcmp(compstr, _STRLIT_CHAR("fmu")) >= 0)
        {
            if (compcount == 3)
            {
                // Verify and set the passed-in fmu setting
                PVMFStatus retval = DoVerifyAndSetFMUParameter(paramkvp[paramind], true);
                if (retval != PVMFSuccess)
                {
                    *retkvp = &paramkvp[paramind];
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoCapConfigSetParameters() Setting parameter %d failed", paramind));
                    return retval;
                }
            }
            else
            {
                // Do not support other keys right now
                *retkvp = &paramkvp[paramind];
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoCapConfigSetParameters() Unsupported key"));
                return PVMFErrArgument;
            }
        }
        else
        {
            PVMFStatus cmdstatus = DoPlayerSetParametersSync(aCmd.GetCmdId(), aCmd.GetContext(), paramkvp, numparam, *retkvp);
            if (PVMFSuccess != cmdstatus)
            {
                return cmdstatus;
            }
        }
    }

    if (!aSyncCmd)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoCapConfigSetParameters() - Queuing up CommandComplete for a ASync command"));
        UtilityCommandCompleted(aCmd.GetCmdId(), aCmd.GetContext(), PVMFSuccess);
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoCapConfigSetParameters() Out"));
    return PVMFSuccess;
}

PVMFStatus PVFrameAndMetadataUtility::DoPlayerSetParametersSync(PVCommandId aCmdId, OsclAny* aCmdContext, PvmiKvp* aParameters, int aNumElements, PvmiKvp* &aRetKVP)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoPlayerSetParameterSync() In"));

    iUtilityContext.iCmdId = aCmdId;
    iUtilityContext.iCmdContext = aCmdContext;
    iUtilityContext.iCmdType = PVFM_CMD_PlayerSetParametersSync;
    int32 leavecode = 0;
    OSCL_ASSERT(iPlayerCapConfigIF != NULL);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                    (0, "PVFrameAndMetadataUtility::Player setParametersSync Called Tick=%d", OsclTickCount::TickCount()));
    OSCL_TRY(leavecode, iPlayerCapConfigIF->setParametersSync(NULL, aParameters, aNumElements, aRetKVP));
    OSCL_FIRST_CATCH_ANY(leavecode,
                         PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoPlayerQueryInterface() QueryInterface() on player did a leave!"));
                         return PVMFFailure;);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoPlayerSetParameterSync() Out"));
    return PVMFSuccess;
}


PVMFStatus PVFrameAndMetadataUtility::DoVerifyAndSetFMUParameter(PvmiKvp& aParameter, bool aSetParam)
{
    OSCL_UNUSED_ARG(aSetParam);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoVerifyAndSetFMUParameter() In"));

    PVMFStatus status = PVMFErrNotSupported;
    // Retrieve the third component from the key string
    char* compstr = NULL;
    pv_mime_string_extract_type(2, aParameter.key, compstr);

    // Check if it is key string "timeout"
    if (pv_mime_strcmp(compstr, _STRLIT_CHAR("timeout")) >= 0)
    {
        if (oscl_strncmp(aParameter.key, PVFMUTIL_FRAMERETRIEVAL_TIMEOUT_KEY, oscl_strlen(PVFMUTIL_FRAMERETRIEVAL_TIMEOUT_KEY)) == 0)
        {
            iFrameReadyWaitTime = aParameter.value.uint32_value;
            status = PVMFSuccess;
        }
        // Possible addition: Error handling timeout
    }

    return status;
}
bool PVFrameAndMetadataUtility::HasVideo()
{
    //Query the MIO component for it's video format.  If the format
    //is "unknown" then it's very likely the clip has no video.
    bool hasVideo = false;
    if (iVideoMIO)
    {
        PvmiKvp* kvp = NULL;
        int32 count = 0;
        OSCL_HeapString<OsclMemAllocator> str;
        str = MOUT_VIDEO_FORMAT_KEY;
        if (iVideoMIO->getParametersSync(0, (char*)str.get_cstr(), kvp, count, NULL) == PVMFSuccess)
        {
            if (kvp
                    && kvp->value.pChar_value)
            {
                str = kvp->value.pChar_value;
                if (!(str == PVMF_MIME_FORMAT_UNKNOWN))
                    hasVideo = true;
            }
            iVideoMIO->releaseParameters(0, kvp, count);
        }
    }
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::HasVideo() %d", hasVideo));
    return hasVideo;
}


PVMFStatus PVFrameAndMetadataUtility::DoGetFrame(PVFMUtilityCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoGetFrame() In"));

    /*
    GetFrame can be called only when UI sets the correct mode either
    PV_FRAME_METADATA_INTERFACE_MODE_SOURCE_METADATA_AND_THUMBNAIL or PV_FRAME_METADATA_INTERFACE_MODE_ALL
    */
    if (iMode == PV_FRAME_METADATA_INTERFACE_MODE_SOURCE_METADATA_ONLY)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoGetFrame() called in wrong mode (%d)", iMode));
        return PVMFErrArgument;
    }
    int32 leavecode = 0;

    // Retrieve the requested video frame
    uint8* userframebuffer = NULL;
    uint8** utilityframebuffer = NULL;

    iVideoFrameSelector = (PVFrameSelector*)(aCmd.GetParam(0).pOsclAny_value);
    iVideoFrameBufferSize = aCmd.GetParam(2).pUint32_value;
    iVideoFrameBufferProp = (PVFrameBufferProperty*)(aCmd.GetParam(3).pOsclAny_value);

    if (iVideoFrameSelector == NULL || iVideoFrameBufferProp == NULL || iVideoFrameBufferSize == NULL)
    {
        return PVMFErrArgument;
    }

    // Validate the frame selection mode
    if (iVideoFrameSelector->iSelectionMethod != PVFrameSelector::SPECIFIC_FRAME &&
            iVideoFrameSelector->iSelectionMethod != PVFrameSelector::TIMESTAMP)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoGetFrame() Unsupported frame selection method."));
        return PVMFErrNotSupported;
    }

    if (aCmd.GetCmdType() == PVFM_UTILITY_COMMAND_GET_FRAME_UTILITY_BUFFER)
    {
        utilityframebuffer = (uint8**)(aCmd.GetParam(1).pOsclAny_value);
        if (utilityframebuffer == NULL)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoGetFrame() Passed in parameter invalid."));
            return PVMFErrArgument;
        }

        // Create the memory pool for the video frame buffer if not created yet
        if (iVideoFrameBufferMemPool == NULL)
        {
            leavecode = 0;
            OSCL_TRY(leavecode, iVideoFrameBufferMemPool = OSCL_NEW(OsclMemPoolResizableAllocator, (PVFMUTIL_VIDEOFRAMEBUFFER_MEMPOOL_BUFFERSIZE)));
            OSCL_FIRST_CATCH_ANY(leavecode,
                                 PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoGetFrame() Memory pool for video frame buffer could not be instantiated."));
                                 return PVMFErrNoMemory;
                                );
        }

        // Allocate video buffer from memory pool
        leavecode = 0;
        OSCL_TRY(leavecode, *utilityframebuffer = (uint8*)(iVideoFrameBufferMemPool->allocate(PVFMUTIL_VIDEOFRAMEBUFFER_MAXSIZE)));
        OSCL_FIRST_CATCH_ANY(leavecode,
                             PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoGetFrame() Memory pool for video frame buffer could not be instantiated."));
                             return PVMFErrNoMemory;
                            );

        *iVideoFrameBufferSize = PVFMUTIL_VIDEOFRAMEBUFFER_MAXSIZE;
        iCurrentVideoFrameBuffer = *utilityframebuffer;
    }
    else
    {
        userframebuffer = aCmd.GetParam(1).pUint8_value;
        if (userframebuffer == NULL || *iVideoFrameBufferSize == 0)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoGetFrame() Passed in parameter invalid."));
            return PVMFErrArgument;
        }

        iCurrentVideoFrameBuffer = userframebuffer;
    }

    PVPlayerState playerstate;
    PVMFStatus retval = iPlayer->GetPVPlayerStateSync(playerstate);
    if (retval == PVMFSuccess)
    {
        if (playerstate == PVP_STATE_INITIALIZED)
        {
            // Start playback
            retval = DoGFPlayerPrepare(aCmd.GetCmdId(), aCmd.GetContext());
        }
        else if (playerstate == PVP_STATE_PAUSED)
        {
            if (!HasVideo())
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoGetFrame() No video present"));
                return PVMFFailure;
            }
            // Stop first to return to initialized state
            retval = DoGFPlayerStopFromPaused(aCmd.GetCmdId(), aCmd.GetContext());
        }
        else
        {
            // Player engine should not be in any other state
            // Report as underlying resource error
            OSCL_ASSERT(false);
            retval = PVMFErrResource;
        }
    }

    if (retval != PVMFSuccess && aCmd.GetCmdType() == PVFM_UTILITY_COMMAND_GET_FRAME_UTILITY_BUFFER && iCurrentVideoFrameBuffer)
    {
        // Return the buffer if allocated from utility's mempool
        iVideoFrameBufferMemPool->deallocate(iCurrentVideoFrameBuffer);
    }
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoGetFrame() Out"));
    return retval;
}


PVMFStatus PVFrameAndMetadataUtility::DoGFPlayerStopFromPaused(PVCommandId aCmdId, OsclAny* aCmdContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoGFPlayerStopFromPaused() In"));

    iUtilityContext.iCmdId = aCmdId;
    iUtilityContext.iCmdContext = aCmdContext;
    iUtilityContext.iCmdType = PVFM_CMD_GFPlayerStopFromPaused;
    int32 leavecode = 0;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                    (0, "PVFrameAndMetadataUtility::PlayerStop Called Tick=%d", OsclTickCount::TickCount()));
    OSCL_TRY(leavecode, iPlayer->Stop((const OsclAny*)&iUtilityContext));
    OSCL_FIRST_CATCH_ANY(leavecode,
                         PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoGFPlayerStopFromPaused() Stop() on player did a leave!"));
                         return PVMFFailure;);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoGFPlayerStopFromPaused() Out"));
    return PVMFSuccess;
}


PVMFStatus PVFrameAndMetadataUtility::DoGFPlayerPrepare(PVCommandId aCmdId, OsclAny* aCmdContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoGFPlayerPrepare() In"));

    // Assert if the frame retrieval variables are not set yet
    OSCL_ASSERT(iVideoFrameSelector != NULL);
    OSCL_ASSERT(iCurrentVideoFrameBuffer != NULL);
    OSCL_ASSERT(iVideoFrameBufferSize != NULL);
    OSCL_ASSERT(*iVideoFrameBufferSize > 0);

    // Reset the flag for frame received
    iFrameReceived = false;

    PVMFStatus retval;
    if (iVideoFrameSelector->iSelectionMethod == PVFrameSelector::SPECIFIC_FRAME)
    {
        // Request the frame retrieval video MIO to retrieve the specified frame
        retval = iVideoMIO->GetFrameByFrameNumber(iVideoFrameSelector->iFrameInfo.iFrameIndex, iCurrentVideoFrameBuffer, *iVideoFrameBufferSize, iOutputFormatType, *this);
    }
    else if (iVideoFrameSelector->iSelectionMethod == PVFrameSelector::TIMESTAMP)
    {
        // Request the frame retrieval video MIO to retrieve the specified timestamp
        retval = iVideoMIO->GetFrameByTimeoffset(iVideoFrameSelector->iFrameInfo.iTimeOffsetMilliSec, iCurrentVideoFrameBuffer, *iVideoFrameBufferSize, iOutputFormatType, *this);
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoGFPlayerPrepare() Unsupported frame selection method."));
        retval = PVMFErrNotSupported;
    }

    if (retval != PVMFPending)
    {
        OSCL_ASSERT(retval != PVMFSuccess);
        return retval;
    }

    iUtilityContext.iCmdId = aCmdId;
    iUtilityContext.iCmdContext = aCmdContext;
    iUtilityContext.iCmdType = PVFM_CMD_GFPlayerPrepare;
    int32 leavecode = 0;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                    (0, "PVFrameAndMetadataUtility::PlayerPrepare Called Tick=%d", OsclTickCount::TickCount()));
    OSCL_TRY(leavecode, iPlayer->Prepare((const OsclAny*)&iUtilityContext));
    OSCL_FIRST_CATCH_ANY(leavecode,
                         PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoGFPlayerPrepare() Prepare() on player did a leave!"));
                         return PVMFFailure;);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoGFPlayerPrepare() Out"));
    return PVMFSuccess;
}


PVMFStatus PVFrameAndMetadataUtility::DoGFPlayerStart(PVCommandId aCmdId, OsclAny* aCmdContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoGFPlayerStart() In"));

    iUtilityContext.iCmdId = aCmdId;
    iUtilityContext.iCmdContext = aCmdContext;
    iUtilityContext.iCmdType = PVFM_CMD_GFPlayerStart;
    int32 leavecode = 0;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                    (0, "PVFrameAndMetadataUtility::PlayerStart Called Tick=%d", OsclTickCount::TickCount()));
    OSCL_TRY(leavecode, iPlayer->Start((const OsclAny*)&iUtilityContext));
    OSCL_FIRST_CATCH_ANY(leavecode,
                         PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoGFPlayerStart() Start() on player did a leave!"));
                         return PVMFFailure;);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoGFPlayerStart() Out"));
    return PVMFSuccess;
}


PVMFStatus PVFrameAndMetadataUtility::DoGFPlayerPause(PVCommandId aCmdId, OsclAny* aCmdContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoGFPlayerPause() In"));

    PVPlayerState playerstate;
    PVMFStatus retval = iPlayer->GetPVPlayerStateSync(playerstate);
    if (retval == PVMFSuccess && playerstate == PVP_STATE_PAUSED)
    {
        // Player is already in paused state (due to EOS?)
        // so need to pause playback
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoGFPlayerPause() Playback already paused"));
        return PVMFErrInvalidState;
    }

    iUtilityContext.iCmdId = aCmdId;
    iUtilityContext.iCmdContext = aCmdContext;
    iUtilityContext.iCmdType = PVFM_CMD_GFPlayerPause;
    int32 leavecode = 0;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                    (0, "PVFrameAndMetadataUtility::PlayerPause Called Tick=%d", OsclTickCount::TickCount()));
    OSCL_TRY(leavecode, iPlayer->Pause((const OsclAny*)&iUtilityContext));
    OSCL_FIRST_CATCH_ANY(leavecode,
                         PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoGFPlayerPause() Pause() on player did a leave!"));
                         return PVMFFailure;);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoGFPlayerPause() Out"));
    return PVMFSuccess;
}


PVMFStatus PVFrameAndMetadataUtility::DoReturnBuffer(PVFMUtilityCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoReturnBuffer() In"));

    uint8* retbuffer = NULL;
    retbuffer = aCmd.GetParam(0).pUint8_value;
    if (retbuffer == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoReturnBuffer() Specified buffer is NULL"));
        return PVMFErrArgument;
    }

    // Return the video frame buffer
    int32 leavecode = 0;
    OSCL_TRY(leavecode, iVideoFrameBufferMemPool->deallocate((OsclAny*)retbuffer));
    OSCL_FIRST_CATCH_ANY(leavecode,
                         PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoReturnBuffer() Specified buffer causes a leave in mempool when deallocating"));
                         return PVMFErrArgument;
                        );

    UtilityCommandCompleted(aCmd.GetCmdId(), aCmd.GetContext(), PVMFSuccess);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoReturnBuffer() Out"));
    return PVMFSuccess;
}


PVMFStatus PVFrameAndMetadataUtility::DoRemoveDataSource(PVFMUtilityCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoRemoveDataSource() In"));

    if (GetUtilityState() != PVFM_STATE_INITIALIZED)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoRemoveDataSource() Wrong state"));
        return PVMFErrInvalidState;
    }

    if (iDataSource != (PVPlayerDataSource*)(aCmd.GetParam(0).pOsclAny_value))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoRemoveDataSource() Passed in parameter invalid"));
        return PVMFErrArgument;
    }

    // Tell player engine to remove data sink, reset, and remove data source
    PVMFStatus cmdstatus = PVMFFailure;
    PVPlayerState playerstate;
    PVMFStatus pretval = iPlayer->GetPVPlayerStateSync(playerstate);
    if (pretval == PVMFSuccess)
    {
        if (playerstate == PVP_STATE_INITIALIZED)
        {
            // Start with remove data sink
            if (iVideoNode && iVideoMIO)
            {
                cmdstatus = DoRDSPlayerRemoveVideoDataSink(aCmd.GetCmdId(), aCmd.GetContext());
            }
            else if (iAudioNode && iAudioMIO)
            {
                cmdstatus = DoRDSPlayerRemoveAudioDataSink(aCmd.GetCmdId(), aCmd.GetContext());
            }
            else
            {
                cmdstatus = DoRDSPlayerReset(aCmd.GetCmdId(), aCmd.GetContext());
            }
        }
        else if (playerstate == PVP_STATE_PAUSED || playerstate == PVP_STATE_STARTED || playerstate == PVP_STATE_PREPARED)
        {
            // Stop first to return to initialized state
            cmdstatus = DoRDSPlayerStop(aCmd.GetCmdId(), aCmd.GetContext());
        }
        else if (playerstate == PVP_STATE_IDLE)
        {
            // Call RemoveDataSource() on player
            cmdstatus = DoRDSPlayerRemoveDataSource(aCmd.GetCmdId(), aCmd.GetContext());
            if (cmdstatus != PVMFSuccess)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoRemoveDataSource: RemoveDataSource on player failed. Report command as failed"));

                iAPICmdStatus = cmdstatus;
                if (iAPICmdErrMsg)
                {
                    iAPICmdErrMsg->removeRef();
                    iAPICmdErrMsg = NULL;
                }
                // Need to shutdown/restart player and cleanup in utility's AO
                OSCL_ASSERT(iErrorHandlingInUtilityAO == false);
                iErrorHandlingInUtilityAO = true;
                RunIfNotReady();
            }
        }
        else
        {
            // Player engine should not be in any other state
            // Report as underlying resource error
            OSCL_ASSERT(false);
            cmdstatus = PVMFErrResource;
        }
    }
    else
    {
        // Player engine state could not be checked so error out
        cmdstatus = pretval;
    }

    if (cmdstatus == PVMFSuccess)
    {
        SetUtilityState(PVFM_UTILITY_STATE_RESETTING);
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoRemoveDataSource() Out"));
    return cmdstatus;
}


PVMFStatus PVFrameAndMetadataUtility::DoRDSPlayerStop(PVCommandId aCmdId, OsclAny* aCmdContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoRDSPlayerStop In"));

    iUtilityContext.iCmdId = aCmdId;
    iUtilityContext.iCmdContext = aCmdContext;
    iUtilityContext.iCmdType = PVFM_CMD_RDSPlayerStopFromPaused;
    int32 leavecode = 0;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                    (0, "PVFrameAndMetadataUtility::RDSPlayerStop Called Tick=%d", OsclTickCount::TickCount()));
    OSCL_TRY(leavecode, iPlayer->Stop((const OsclAny*)&iUtilityContext));
    OSCL_FIRST_CATCH_ANY(leavecode,
                         PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoRDSPlayerStop Stop() on player did a leave!"));
                         return PVMFFailure;);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoRDSPlayerStop Out"));
    return PVMFSuccess;
}


PVMFStatus PVFrameAndMetadataUtility::DoRDSPlayerRemoveVideoDataSink(PVCommandId aCmdId, OsclAny* aCmdContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoRDSPlayerRemoveVideoDataSink() In"));

    iUtilityContext.iCmdId = aCmdId;
    iUtilityContext.iCmdContext = aCmdContext;
    iUtilityContext.iCmdType = PVFM_CMD_RDSPlayerRemoveVideoDataSink;
    int32 leavecode = 0;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                    (0, "PVFrameAndMetadataUtility::RDSPlayerRemoveVideoSink Called Tick=%d", OsclTickCount::TickCount()));
    OSCL_TRY(leavecode, iPlayer->RemoveDataSink(iVideoDataSink, (const OsclAny*)&iUtilityContext));
    OSCL_FIRST_CATCH_ANY(leavecode,
                         PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoRDSPlayerRemoveVideoDataSink() RemoveDataSink() on player did a leave!"));
                         return PVMFFailure;);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoRDSPlayerRemoveVideoDataSink() Out"));
    return PVMFSuccess;
}

PVMFStatus PVFrameAndMetadataUtility::DoRDSPlayerRemoveAudioDataSink(PVCommandId aCmdId, OsclAny* aCmdContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoRDSPlayerRemoveAudioDataSink() In"));

    iUtilityContext.iCmdId = aCmdId;
    iUtilityContext.iCmdContext = aCmdContext;
    iUtilityContext.iCmdType = PVFM_CMD_RDSPlayerRemoveAudioDataSink;
    int32 leavecode = 0;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                    (0, "PVFrameAndMetadataUtility::RDSPlayerRemoveAudioSink Called Tick=%d", OsclTickCount::TickCount()));
    OSCL_TRY(leavecode, iPlayer->RemoveDataSink(iAudioDataSink, (const OsclAny*)&iUtilityContext));
    OSCL_FIRST_CATCH_ANY(leavecode,
                         PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoRDSPlayerRemoveAudioDataSink() RemoveDataSink() on player did a leave!"));
                         return PVMFFailure;);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoRDSPlayerRemoveAudioDataSink() Out"));
    return PVMFSuccess;
}

PVMFStatus PVFrameAndMetadataUtility::DoRDSPlayerReset(PVCommandId aCmdId, OsclAny* aCmdContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoRDSPlayerReset() In"));

    iUtilityContext.iCmdId = aCmdId;
    iUtilityContext.iCmdContext = aCmdContext;
    iUtilityContext.iCmdType = PVFM_CMD_RDSPlayerReset;
    int32 leavecode = 0;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                    (0, "PVFrameAndMetadataUtility::RDSPlayerReset Called Tick=%d", OsclTickCount::TickCount()));
    OSCL_TRY(leavecode, iPlayer->Reset((const OsclAny*)&iUtilityContext));
    OSCL_FIRST_CATCH_ANY(leavecode,
                         PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoRDSPlayerReset() Reset() on player did a leave!"));
                         return PVMFFailure;);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoRDSPlayerReset() Out"));
    return PVMFSuccess;
}


PVMFStatus PVFrameAndMetadataUtility::DoRDSPlayerRemoveDataSource(PVCommandId aCmdId, OsclAny* aCmdContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoRDSPlayerRemoveDataSource() In"));

    iUtilityContext.iCmdId = aCmdId;
    iUtilityContext.iCmdContext = aCmdContext;
    iUtilityContext.iCmdType = PVFM_CMD_RDSPlayerRemoveDataSource;
    int32 leavecode = 0;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                    (0, "PVFrameAndMetadataUtility::RDSPlayerRemoveDataSource Called Tick=%d", OsclTickCount::TickCount()));
    OSCL_TRY(leavecode, iPlayer->RemoveDataSource(*iDataSource, (const OsclAny*)&iUtilityContext));
    OSCL_FIRST_CATCH_ANY(leavecode,
                         PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoRDSPlayerRemoveDataSource() RemoveDataSource() on player did a leave!"));
                         return PVMFFailure;);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoRDSPlayerRemoveDataSource() Out"));
    return PVMFSuccess;
}


PVMFStatus PVFrameAndMetadataUtility::DoPlayerShutdownRestart(void)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoPlayerShutdownRestart() In"));

    int32 leavecode = 0;

    // First destroy the player engine instance
    OSCL_ASSERT(iPlayer != NULL);
    if (iPlayer != NULL)
    {
        PVPlayerFactory::DeletePlayer(iPlayer);
        iPlayer = NULL;
    }

    // Recreate the player instance
    leavecode = 0;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                    (0, "PVFrameAndMetadataUtility::CreatePlayer Called Tick=%d", OsclTickCount::TickCount()));
    OSCL_TRY(leavecode, iPlayer = PVPlayerFactory::CreatePlayer(this, this, this));
    OSCL_FIRST_CATCH_ANY(leavecode,
                         PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::DoPlayerShutdownRestart() Player engine could not be instantiated! Asserting"));
                         OSCL_ASSERT(false);
                         return PVMFErrNoMemory;);
    OSCL_ASSERT(iPlayer != NULL);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::DoPlayerShutdownRestart() Out"));
    return PVMFSuccess;
}


void PVFrameAndMetadataUtility::HandlePlayerQueryUUID(PVFMUtilityContext& aUtilContext, const PVCmdResponse& aCmdResp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::HandlePlayerQueryUUID() In"));

    aUtilContext.iCmdType = -1;

    switch (aCmdResp.GetCmdStatus())
    {
        case PVMFSuccess:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                            (0, "PVFrameAndMetadataUtility::PlayerQueryUUID completed successfully Tick=%d", OsclTickCount::TickCount()));
            // Report QueryUUID() command as succesfully complete
            UtilityCommandCompleted(aUtilContext.iCmdId, aUtilContext.iCmdContext, PVMFSuccess);
            break;

        default:
        {
            PVMFErrorInfoMessageInterface* nextmsg = NULL;
            if (aCmdResp.GetEventExtensionInterface())
            {
                nextmsg = GetErrorInfoMessageInterface(*(aCmdResp.GetEventExtensionInterface()));
            }
            PVMFBasicErrorInfoMessage* errmsg = NULL;
            errmsg = CreateBasicErrInfoMessage(nextmsg);

            UtilityCommandCompleted(aUtilContext.iCmdId, aUtilContext.iCmdContext, aCmdResp.GetCmdStatus(), OSCL_STATIC_CAST(PVInterface*, errmsg));
            if (errmsg)
            {
                errmsg->removeRef();
            }
        }
        break;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::HandlePlayerQueryUUID() Out"));
}


void PVFrameAndMetadataUtility::HandlePlayerQueryInterface(PVFMUtilityContext& aUtilContext, const PVCmdResponse& aCmdResp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::HandlePlayerQueryInterface() In"));

    switch (aCmdResp.GetCmdStatus())
    {
        case PVMFSuccess:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                            (0, "PVFrameAndMetadataUtility::PlayerQueryInterface completed successfully Tick=%d", OsclTickCount::TickCount()));
            if (aUtilContext.iCmdType == PVFM_CMD_PlayerQueryCapConfigInterface)
                iPlayerCapConfigIF = OSCL_STATIC_CAST(PvmiCapabilityAndConfig*, iPlayerCapConfigIFPVI);
            // Report QueryInterface() command as succesfully complete
            UtilityCommandCompleted(aUtilContext.iCmdId, aUtilContext.iCmdContext, PVMFSuccess);
            break;

        default:
        {
            PVMFErrorInfoMessageInterface* nextmsg = NULL;
            if (aCmdResp.GetEventExtensionInterface())
            {
                nextmsg = GetErrorInfoMessageInterface(*(aCmdResp.GetEventExtensionInterface()));
            }
            PVMFBasicErrorInfoMessage* errmsg = NULL;
            errmsg = CreateBasicErrInfoMessage(nextmsg);

            UtilityCommandCompleted(aUtilContext.iCmdId, aUtilContext.iCmdContext, aCmdResp.GetCmdStatus(), OSCL_STATIC_CAST(PVInterface*, errmsg));
            if (errmsg)
            {
                errmsg->removeRef();
            }
        }
        break;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::HandlePlayerQueryInterface() Out"));
}


void PVFrameAndMetadataUtility::HandleADSPlayerAddDataSource(PVFMUtilityContext& aUtilContext, const PVCmdResponse& aCmdResp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::HandleADSPlayerAddDataSource() In"));

    aUtilContext.iCmdType = -1;

    PVMFStatus cmdstatus = PVMFFailure;

    switch (aCmdResp.GetCmdStatus())
    {
        case PVMFSuccess:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                            (0, "PVFrameAndMetadataUtility::PlayerADS completed successfully Tick=%d", OsclTickCount::TickCount()));
            // Call Init() on player
            cmdstatus = DoADSPlayerInit(aUtilContext.iCmdId, aUtilContext.iCmdContext);
            if (cmdstatus != PVMFSuccess)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::HandleADSPlayerAddDataSource() Init on player failed. Report command as failed"));
                iAPICmdStatus = cmdstatus;
                if (iAPICmdErrMsg)
                {
                    iAPICmdErrMsg->removeRef();
                    iAPICmdErrMsg = NULL;
                }
                // Need to shutdown/restart player and cleanup in utility's AO
                OSCL_ASSERT(iErrorHandlingInUtilityAO == false);
                iErrorHandlingInUtilityAO = true;
                RunIfNotReady();
            }
            break;

        default:
        {
            PVMFErrorInfoMessageInterface* nextmsg = NULL;
            if (aCmdResp.GetEventExtensionInterface())
            {
                nextmsg = GetErrorInfoMessageInterface(*(aCmdResp.GetEventExtensionInterface()));
            }
            PVMFBasicErrorInfoMessage* errmsg = NULL;
            errmsg = CreateBasicErrInfoMessage(nextmsg);

            iAPICmdStatus = aCmdResp.GetCmdStatus();
            if (iAPICmdErrMsg)
            {
                iAPICmdErrMsg->removeRef();
                iAPICmdErrMsg = NULL;
            }
            iAPICmdErrMsg = errmsg;

            // Check if player is handling the error
            PVPlayerState playerstate;
            PVMFStatus pretval = iPlayer->GetPVPlayerStateSync(playerstate);
            if (pretval == PVMFSuccess && playerstate == PVP_STATE_ERROR)
            {
                // Yes so wait for error handling to complete
                // Start a timer just in case the player does not report error handling complete
                iTimeoutTimer->Request(PVFMUTIL_TIMERID_PLAYERERRORTIMEOUT, 0, iErrorHandlingWaitTime, this, false);
            }
            else
            {
                // Need to shutdown/restart player and cleanup in utility's AO
                OSCL_ASSERT(iErrorHandlingInUtilityAO == false);
                iErrorHandlingInUtilityAO = true;
                RunIfNotReady();
            }
        }
        break;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::HandleADSPlayerAddDataSource() Out"));
}


void PVFrameAndMetadataUtility::HandleADSPlayerInit(PVFMUtilityContext& aUtilContext, const PVCmdResponse& aCmdResp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::HandleADSPlayerInit() In"));

    aUtilContext.iCmdType = -1;

    PVMFStatus cmdstatus = PVMFFailure;

    switch (aCmdResp.GetCmdStatus())
    {
        case PVMFSuccess:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                            (0, "PVFrameAndMetadataUtility::PlayerInit completed successfully Tick=%d", OsclTickCount::TickCount()));
            if (iMode == PV_FRAME_METADATA_INTERFACE_MODE_SOURCE_METADATA_ONLY)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO, (0, "PVFrameAndMetadataUtility::HandleADSPlayerInit - ADS Complete"));
                // Utility's AddDataSource() successfully completed
                SetUtilityState(PVFM_UTILITY_STATE_INITIALIZED);
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                                (0, "PVFrameAndMetadataUtility::AddDataSource completed sucessfully Tick=%d", OsclTickCount::TickCount()));

                UtilityCommandCompleted(aUtilContext.iCmdId, aUtilContext.iCmdContext, PVMFSuccess);
            }
            else
            {
                // Call AddDataSink() on player
                cmdstatus = DoADSPlayerAddVideoDataSink(aUtilContext.iCmdId, aUtilContext.iCmdContext);
                if (cmdstatus != PVMFSuccess)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::HandleADSPlayerInit() AddDataSink on player failed. Report command as failed"));
                    iAPICmdStatus = aCmdResp.GetCmdStatus();
                    if (iAPICmdErrMsg)
                    {
                        iAPICmdErrMsg->removeRef();
                        iAPICmdErrMsg = NULL;
                    }
                    // Need to shutdown/restart player and cleanup in utility's AO
                    OSCL_ASSERT(iErrorHandlingInUtilityAO == false);
                    iErrorHandlingInUtilityAO = true;
                    RunIfNotReady();
                }
            }
        }
        break;

        default:
        {
            PVMFErrorInfoMessageInterface* nextmsg = NULL;
            if (aCmdResp.GetEventExtensionInterface())
            {
                nextmsg = GetErrorInfoMessageInterface(*(aCmdResp.GetEventExtensionInterface()));
            }
            PVMFBasicErrorInfoMessage* errmsg = NULL;
            errmsg = CreateBasicErrInfoMessage(nextmsg);

            iAPICmdStatus = aCmdResp.GetCmdStatus();
            if (iAPICmdErrMsg)
            {
                iAPICmdErrMsg->removeRef();
                iAPICmdErrMsg = NULL;
            }
            iAPICmdErrMsg = errmsg;

            // Check if player is handling the error
            PVPlayerState playerstate;
            PVMFStatus pretval = iPlayer->GetPVPlayerStateSync(playerstate);
            if (pretval == PVMFSuccess && playerstate == PVP_STATE_ERROR)
            {
                // Wait for error handling to complete
                // Start a timer just in case the player does not report error handling complete
                iTimeoutTimer->Request(PVFMUTIL_TIMERID_PLAYERERRORTIMEOUT, 0, iErrorHandlingWaitTime, this, false);
            }
            else
            {
                // Need to shutdown/restart player and cleanup in utility's AO
                OSCL_ASSERT(iErrorHandlingInUtilityAO == false);
                iErrorHandlingInUtilityAO = true;
                RunIfNotReady();
            }
        }
        break;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::HandleADSPlayerInit() Out"));
}

void PVFrameAndMetadataUtility::HandleADSPlayerAddVideoDataSink(PVFMUtilityContext& aUtilContext, const PVCmdResponse& aCmdResp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::HandleADSPlayerAddVideoDataSink() In"));

    aUtilContext.iCmdType = -1;

    switch (aCmdResp.GetCmdStatus())
    {
        case PVMFSuccess:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                            (0, "PVFrameAndMetadataUtility::PlayerAddVideoDataSink completed successfully Tick=%d", OsclTickCount::TickCount()));
            if (iMode == PV_FRAME_METADATA_INTERFACE_MODE_SOURCE_METADATA_AND_THUMBNAIL)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO, (0, "PVFrameAndMetadataUtility::HandleADSPlayerAddVideoDataSink - ADS Complete"));
                // Utility's AddDataSource() successfully completed
                SetUtilityState(PVFM_UTILITY_STATE_INITIALIZED);
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                                (0, "PVFrameAndMetadataUtility::AddDataSource completed sucessfully Tick=%d", OsclTickCount::TickCount()));

                UtilityCommandCompleted(aUtilContext.iCmdId, aUtilContext.iCmdContext, PVMFSuccess);
            }
            else
            {
                // Add Audio data sink
                PVMFStatus cmdstatus = DoADSPlayerAddAudioDataSink(aUtilContext.iCmdId, aUtilContext.iCmdContext);
                if (cmdstatus != PVMFSuccess)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::HandleADSPlayerAddVideoDataSink() AddAudioDataSink on player failed. Report command as failed"));
                    iAPICmdStatus = aCmdResp.GetCmdStatus();
                    if (iAPICmdErrMsg)
                    {
                        iAPICmdErrMsg->removeRef();
                        iAPICmdErrMsg = NULL;
                    }
                    // Need to shutdown/restart player and cleanup in utility's AO
                    OSCL_ASSERT(iErrorHandlingInUtilityAO == false);
                    iErrorHandlingInUtilityAO = true;
                    RunIfNotReady();
                }
            }
        }
        break;

        default:
        {
            PVMFErrorInfoMessageInterface* nextmsg = NULL;
            if (aCmdResp.GetEventExtensionInterface())
            {
                nextmsg = GetErrorInfoMessageInterface(*(aCmdResp.GetEventExtensionInterface()));
            }

            PVMFBasicErrorInfoMessage* errmsg = NULL;
            errmsg = CreateBasicErrInfoMessage(nextmsg);

            iAPICmdStatus = aCmdResp.GetCmdStatus();
            if (iAPICmdErrMsg)
            {
                iAPICmdErrMsg->removeRef();
                iAPICmdErrMsg = NULL;
            }
            iAPICmdErrMsg = errmsg;

            // Check if player is handling the error
            PVPlayerState playerstate;
            PVMFStatus pretval = iPlayer->GetPVPlayerStateSync(playerstate);
            if (pretval == PVMFSuccess && playerstate == PVP_STATE_ERROR)
            {
                // Wait for error handling to complete
                // Start a timer just in case the player does not report error handling complete
                iTimeoutTimer->Request(PVFMUTIL_TIMERID_PLAYERERRORTIMEOUT, 0, iErrorHandlingWaitTime, this, false);
            }
            else
            {
                // Need to shutdown/restart player and cleanup in utility's AO
                OSCL_ASSERT(iErrorHandlingInUtilityAO == false);
                iErrorHandlingInUtilityAO = true;
                RunIfNotReady();
            }
        }
        break;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::HandleADSPlayerAddVideoDataSink() Out"));
}

void PVFrameAndMetadataUtility::HandleADSPlayerAddAudioDataSink(PVFMUtilityContext& aUtilContext, const PVCmdResponse& aCmdResp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::HandleADSPlayerAddAudioDataSink() In"));

    aUtilContext.iCmdType = -1;

    switch (aCmdResp.GetCmdStatus())
    {
        case PVMFSuccess:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                            (0, "PVFrameAndMetadataUtility::PlayerAddAudioDataSink completed successfully Tick=%d", OsclTickCount::TickCount()));
            // Call Prepare() on player
            PVMFStatus cmdstatus = DoADSPlayerPrepare(aUtilContext.iCmdId, aUtilContext.iCmdContext);
            if (cmdstatus != PVMFSuccess)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::HandleADSPlayerAddAudioDataSink() Prepare on player failed. Report command as failed"));
                iAPICmdStatus = aCmdResp.GetCmdStatus();
                if (iAPICmdErrMsg)
                {
                    iAPICmdErrMsg->removeRef();
                    iAPICmdErrMsg = NULL;
                }
                // Need to shutdown/restart player and cleanup in utility's AO
                OSCL_ASSERT(iErrorHandlingInUtilityAO == false);
                iErrorHandlingInUtilityAO = true;
                RunIfNotReady();
            }
        }
        break;

        default:
        {
            PVMFErrorInfoMessageInterface* nextmsg = NULL;
            if (aCmdResp.GetEventExtensionInterface())
            {
                nextmsg = GetErrorInfoMessageInterface(*(aCmdResp.GetEventExtensionInterface()));
            }

            PVMFBasicErrorInfoMessage* errmsg = NULL;
            errmsg = CreateBasicErrInfoMessage(nextmsg);

            iAPICmdStatus = aCmdResp.GetCmdStatus();
            if (iAPICmdErrMsg)
            {
                iAPICmdErrMsg->removeRef();
                iAPICmdErrMsg = NULL;
            }
            iAPICmdErrMsg = errmsg;

            // Check if player is handling the error
            PVPlayerState playerstate;
            PVMFStatus pretval = iPlayer->GetPVPlayerStateSync(playerstate);
            if (pretval == PVMFSuccess && playerstate == PVP_STATE_ERROR)
            {
                // Wait for error handling to complete
                // Start a timer just in case the player does not report error handling complete
                iTimeoutTimer->Request(PVFMUTIL_TIMERID_PLAYERERRORTIMEOUT, 0, iErrorHandlingWaitTime, this, false);
            }
            else
            {
                // Need to shutdown/restart player and cleanup in utility's AO
                OSCL_ASSERT(iErrorHandlingInUtilityAO == false);
                iErrorHandlingInUtilityAO = true;
                RunIfNotReady();
            }
        }
        break;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::HandleADSPlayerAddVideoDataSink() Out"));
}


void PVFrameAndMetadataUtility::HandleADSPlayerPrepare(PVFMUtilityContext& aUtilContext, const PVCmdResponse& aCmdResp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::HandleADSPlayerPrepare() In"));

    aUtilContext.iCmdType = -1;

    PVMFStatus cmdstatus = PVMFFailure;

    switch (aCmdResp.GetCmdStatus())
    {
        case PVMFSuccess:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                            (0, "PVFrameAndMetadataUtility::PlayerPrepare completed successfully Tick=%d", OsclTickCount::TickCount()));
            // Call Start() on player
            cmdstatus = DoADSPlayerStart(aUtilContext.iCmdId, aUtilContext.iCmdContext);
            if (cmdstatus != PVMFSuccess)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::HandleADSPlayerPrepare() Start on player failed. Report command as failed"));
                iAPICmdStatus = aCmdResp.GetCmdStatus();
                if (iAPICmdErrMsg)
                {
                    iAPICmdErrMsg->removeRef();
                    iAPICmdErrMsg = NULL;
                }
                // Need to shutdown/restart player and cleanup in utility's AO
                OSCL_ASSERT(iErrorHandlingInUtilityAO == false);
                iErrorHandlingInUtilityAO = true;
                RunIfNotReady();
            }
            break;

        default:
        {
            // If the player engine is in initialized state, complete AddDataSource()
            // command with success here. User can still retrieve metadata
            PVPlayerState pstate;
            PVMFStatus playerretval = iPlayer->GetPVPlayerStateSync(pstate);
            if (playerretval == PVMFSuccess && pstate == PVP_STATE_INITIALIZED)
            {
                // Utility's AddDataSource() successfully completed
                SetUtilityState(PVFM_UTILITY_STATE_INITIALIZED);
                UtilityCommandCompleted(aUtilContext.iCmdId, aUtilContext.iCmdContext, PVMFSuccess);
                break;
            }

            // Else fall through to the error handling in the default case since fatal error
            PVMFErrorInfoMessageInterface* nextmsg = NULL;
            if (aCmdResp.GetEventExtensionInterface())
            {
                nextmsg = GetErrorInfoMessageInterface(*(aCmdResp.GetEventExtensionInterface()));
            }
            PVMFBasicErrorInfoMessage* errmsg = NULL;
            errmsg = CreateBasicErrInfoMessage(nextmsg);

            iAPICmdStatus = aCmdResp.GetCmdStatus();
            if (iAPICmdErrMsg)
            {
                iAPICmdErrMsg->removeRef();
                iAPICmdErrMsg = NULL;
            }
            iAPICmdErrMsg = errmsg;

            // Check if player is handling the error
            PVPlayerState playerstate;
            PVMFStatus pretval = iPlayer->GetPVPlayerStateSync(playerstate);
            if (pretval == PVMFSuccess && playerstate == PVP_STATE_ERROR)
            {
                // Wait for error handling to complete
                // Start a timer just in case the player does not report error handling complete
                iTimeoutTimer->Request(PVFMUTIL_TIMERID_PLAYERERRORTIMEOUT, 0, iErrorHandlingWaitTime, this, false);
            }
            else
            {
                // Need to shutdown/restart player and cleanup in utility's AO
                OSCL_ASSERT(iErrorHandlingInUtilityAO == false);
                iErrorHandlingInUtilityAO = true;
                RunIfNotReady();
            }
        }
        break;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::HandleADSPlayerPrepare() Out"));
}


void PVFrameAndMetadataUtility::HandleADSPlayerStart(PVFMUtilityContext& aUtilContext, const PVCmdResponse& aCmdResp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::HandleADSPlayerStart() In"));

    aUtilContext.iCmdType = -1;

    PVMFStatus cmdstatus = PVMFFailure;

    switch (aCmdResp.GetCmdStatus())
    {
        case PVMFSuccess:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                            (0, "PVFrameAndMetadataUtility::PlayerStart completed successfully Tick=%d", OsclTickCount::TickCount()));
            // Call Pause() on player
            cmdstatus = DoADSPlayerPause(aUtilContext.iCmdId, aUtilContext.iCmdContext);
            if (cmdstatus != PVMFSuccess)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::HandleADSPlayerStart() Start on player failed. Report command as failed"));
                iAPICmdStatus = aCmdResp.GetCmdStatus();
                if (iAPICmdErrMsg)
                {
                    iAPICmdErrMsg->removeRef();
                    iAPICmdErrMsg = NULL;
                }
                // Need to shutdown/restart player and cleanup in utility's AO
                OSCL_ASSERT(iErrorHandlingInUtilityAO == false);
                iErrorHandlingInUtilityAO = true;
                RunIfNotReady();
            }
            break;

        default:
        {
            PVMFErrorInfoMessageInterface* nextmsg = NULL;
            if (aCmdResp.GetEventExtensionInterface())
            {
                nextmsg = GetErrorInfoMessageInterface(*(aCmdResp.GetEventExtensionInterface()));
            }
            PVMFBasicErrorInfoMessage* errmsg = NULL;
            errmsg = CreateBasicErrInfoMessage(nextmsg);

            iAPICmdStatus = aCmdResp.GetCmdStatus();
            if (iAPICmdErrMsg)
            {
                iAPICmdErrMsg->removeRef();
                iAPICmdErrMsg = NULL;
            }
            iAPICmdErrMsg = errmsg;

            // Check if player is handling the error
            PVPlayerState playerstate;
            PVMFStatus pretval = iPlayer->GetPVPlayerStateSync(playerstate);
            if (pretval == PVMFSuccess && playerstate == PVP_STATE_ERROR)
            {
                // Wait for error handling to complete
                // Start a timer just in case the player does not report error handling complete
                iTimeoutTimer->Request(PVFMUTIL_TIMERID_PLAYERERRORTIMEOUT, 0, iErrorHandlingWaitTime, this, false);
            }
            else
            {
                // Need to shutdown/restart player and cleanup in utility's AO
                OSCL_ASSERT(iErrorHandlingInUtilityAO == false);
                iErrorHandlingInUtilityAO = true;
                RunIfNotReady();
            }
        }
        break;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::HandleADSPlayerStart() Out"));
}


void PVFrameAndMetadataUtility::HandleADSPlayerPause(PVFMUtilityContext& aUtilContext, const PVCmdResponse& aCmdResp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::HandleADSPlayerPause() In"));

    aUtilContext.iCmdType = -1;

    switch (aCmdResp.GetCmdStatus())
    {
        case PVMFSuccess:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                            (0, "PVFrameAndMetadataUtility::PlayerPause completed successfully Tick=%d", OsclTickCount::TickCount()));
            // Utility's AddDataSource() successfully completed
            SetUtilityState(PVFM_UTILITY_STATE_INITIALIZED);
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                            (0, "PVFrameAndMetadataUtility::AddDataSource completed sucessfully Tick=%d", OsclTickCount::TickCount()));

            UtilityCommandCompleted(aUtilContext.iCmdId, aUtilContext.iCmdContext, PVMFSuccess);
            break;

        default:
        {
            PVMFErrorInfoMessageInterface* nextmsg = NULL;
            if (aCmdResp.GetEventExtensionInterface())
            {
                nextmsg = GetErrorInfoMessageInterface(*(aCmdResp.GetEventExtensionInterface()));
            }
            PVMFBasicErrorInfoMessage* errmsg = NULL;
            errmsg = CreateBasicErrInfoMessage(nextmsg);

            iAPICmdStatus = aCmdResp.GetCmdStatus();
            if (iAPICmdErrMsg)
            {
                iAPICmdErrMsg->removeRef();
                iAPICmdErrMsg = NULL;
            }
            iAPICmdErrMsg = errmsg;

            // Check if player is handling the error
            PVPlayerState playerstate;
            PVMFStatus pretval = iPlayer->GetPVPlayerStateSync(playerstate);
            if (pretval == PVMFSuccess && playerstate == PVP_STATE_ERROR)
            {
                // Wait for error handling to complete
                // Start a timer just in case the player does not report error handling complete
                iTimeoutTimer->Request(PVFMUTIL_TIMERID_PLAYERERRORTIMEOUT, 0, iErrorHandlingWaitTime, this, false);
            }
            else
            {
                // Need to shutdown/restart player and cleanup in utility's AO
                OSCL_ASSERT(iErrorHandlingInUtilityAO == false);
                iErrorHandlingInUtilityAO = true;
                RunIfNotReady();
            }
        }
        break;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::HandleADSPlayerPause() Out"));
}


void PVFrameAndMetadataUtility::HandlePlayerGetMetadataKeys(PVFMUtilityContext& aUtilContext, const PVCmdResponse& aCmdResp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::HandlePlayerGetMetadataKeys() In"));

    aUtilContext.iCmdType = -1;

    switch (aCmdResp.GetCmdStatus())
    {
        case PVMFSuccess:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                            (0, "PVFrameAndMetadataUtility::PlayerGetMetadataKeys completed successfully Tick=%d", OsclTickCount::TickCount()));

            // Report GetMetadataKeys() command as succesfully complete
            UtilityCommandCompleted(aUtilContext.iCmdId, aUtilContext.iCmdContext, PVMFSuccess);
            break;

        default:
        {
            PVMFErrorInfoMessageInterface* nextmsg = NULL;
            if (aCmdResp.GetEventExtensionInterface())
            {
                nextmsg = GetErrorInfoMessageInterface(*(aCmdResp.GetEventExtensionInterface()));
            }
            PVMFBasicErrorInfoMessage* errmsg = NULL;
            errmsg = CreateBasicErrInfoMessage(nextmsg);

            // Check if player is handling the error
            PVPlayerState playerstate;
            PVMFStatus pretval = iPlayer->GetPVPlayerStateSync(playerstate);
            if (pretval == PVMFSuccess && playerstate == PVP_STATE_ERROR)
            {
                // Yes so wait for error handling to complete
                iAPICmdStatus = aCmdResp.GetCmdStatus();
                if (iAPICmdErrMsg)
                {
                    iAPICmdErrMsg->removeRef();
                    iAPICmdErrMsg = NULL;
                }
                iAPICmdErrMsg = errmsg;
                // Start a timer just in case the player does not report error handling complete
                iTimeoutTimer->Request(PVFMUTIL_TIMERID_PLAYERERRORTIMEOUT, 0, iErrorHandlingWaitTime, this, false);
            }
            else
            {
                // Report the command as failed
                UtilityCommandCompleted(aUtilContext.iCmdId, aUtilContext.iCmdContext, aCmdResp.GetCmdStatus(), OSCL_STATIC_CAST(PVInterface*, errmsg));
                if (errmsg)
                {
                    errmsg->removeRef();
                }
            }
        }
        break;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::HandlePlayerGetMetadataKeys() Out"));
}


void PVFrameAndMetadataUtility::HandlePlayerGetMetadataValues(PVFMUtilityContext& aUtilContext, const PVCmdResponse& aCmdResp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::HandlePlayerGetMetadataValues() In"));

    aUtilContext.iCmdType = -1;

    switch (aCmdResp.GetCmdStatus())
    {
        case PVMFSuccess:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                            (0, "PVFrameAndMetadataUtility::PlayerGetMetadataValues completed successfully Tick=%d", OsclTickCount::TickCount()));
            // Report GetMetadataValues() command as succesfully complete
            UtilityCommandCompleted(aUtilContext.iCmdId, aUtilContext.iCmdContext, PVMFSuccess);
            break;

        default:
        {
            PVMFErrorInfoMessageInterface* nextmsg = NULL;
            if (aCmdResp.GetEventExtensionInterface())
            {
                nextmsg = GetErrorInfoMessageInterface(*(aCmdResp.GetEventExtensionInterface()));
            }
            PVMFBasicErrorInfoMessage* errmsg = NULL;
            errmsg = CreateBasicErrInfoMessage(nextmsg);

            // Check if player is handling the error
            PVPlayerState playerstate;
            PVMFStatus pretval = iPlayer->GetPVPlayerStateSync(playerstate);
            if (pretval == PVMFSuccess && playerstate == PVP_STATE_ERROR)
            {
                // Yes so wait for error handling to complete
                iAPICmdStatus = aCmdResp.GetCmdStatus();
                if (iAPICmdErrMsg)
                {
                    iAPICmdErrMsg->removeRef();
                    iAPICmdErrMsg = NULL;
                }
                iAPICmdErrMsg = errmsg;
                // Start a timer just in case the player does not report error handling complete
                iTimeoutTimer->Request(PVFMUTIL_TIMERID_PLAYERERRORTIMEOUT, 0, iErrorHandlingWaitTime, this, false);
            }
            else
            {
                // Report the command as failed
                UtilityCommandCompleted(aUtilContext.iCmdId, aUtilContext.iCmdContext, aCmdResp.GetCmdStatus(), OSCL_STATIC_CAST(PVInterface*, errmsg));
                if (errmsg)
                {
                    errmsg->removeRef();
                }
            }
        }
        break;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::HandlePlayerGetMetadataValues() Out"));
}

void PVFrameAndMetadataUtility::HandlePlayerSetParametersSync(PVFMUtilityContext& aUtilContext, const PVCmdResponse& aCmdResp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::HandlePlayerSetParametersSync() In"));

    aUtilContext.iCmdType = -1;

    switch (aCmdResp.GetCmdStatus())
    {
        case PVMFSuccess:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                            (0, "PVFrameAndMetadataUtility::PlayerSetParametersSync completed successfully Tick=%d", OsclTickCount::TickCount()));
            // Report SetParametersSync() command as succesfully complete
            UtilityCommandCompleted(aUtilContext.iCmdId, aUtilContext.iCmdContext, PVMFSuccess);
            break;

        default:
        {
            PVMFErrorInfoMessageInterface* nextmsg = NULL;
            if (aCmdResp.GetEventExtensionInterface())
            {
                nextmsg = GetErrorInfoMessageInterface(*(aCmdResp.GetEventExtensionInterface()));
            }
            PVMFBasicErrorInfoMessage* errmsg = NULL;
            errmsg = CreateBasicErrInfoMessage(nextmsg);

            // Check if player is handling the error
            PVPlayerState playerstate;
            PVMFStatus pretval = iPlayer->GetPVPlayerStateSync(playerstate);
            if (pretval == PVMFSuccess && playerstate == PVP_STATE_ERROR)
            {
                // Yes so wait for error handling to complete
                iAPICmdStatus = aCmdResp.GetCmdStatus();
                if (iAPICmdErrMsg)
                {
                    iAPICmdErrMsg->removeRef();
                    iAPICmdErrMsg = NULL;
                }
                iAPICmdErrMsg = errmsg;
                // Start a timer just in case the player does not report error handling complete
                iTimeoutTimer->Request(PVFMUTIL_TIMERID_PLAYERERRORTIMEOUT, 0, iErrorHandlingWaitTime, this, false);
            }
            else
            {
                // Report the command as failed
                UtilityCommandCompleted(aUtilContext.iCmdId, aUtilContext.iCmdContext, aCmdResp.GetCmdStatus(), OSCL_STATIC_CAST(PVInterface*, errmsg));
                if (errmsg)
                {
                    errmsg->removeRef();
                }
            }
        }
        break;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::HandlePlayerSetParametersSync() Out"));
}

void PVFrameAndMetadataUtility::HandleGFPlayerStopFromPaused(PVFMUtilityContext& aUtilContext, const PVCmdResponse& aCmdResp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::HandleGFPlayerStopFromPaused() In"));

    aUtilContext.iCmdType = -1;

    switch (aCmdResp.GetCmdStatus())
    {
        case PVMFSuccess:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                            (0, "PVFrameAndMetadataUtility::GFPlayerStopFromPaused completed successfully Tick=%d", OsclTickCount::TickCount()));
            // Call DoPrepare() on player
            PVMFStatus cmdstatus = DoGFPlayerPrepare(aUtilContext.iCmdId, aUtilContext.iCmdContext);
            if (cmdstatus != PVMFSuccess)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::HandleGFPlayerStopFromPaused() Prepare on player failed. Report command as failed"));

                // Check if video frame buffer was allocated from utility's memory pool
                OSCL_ASSERT(iCurrentCmd.empty() == false);
                OSCL_ASSERT(iCurrentVideoFrameBuffer != NULL);
                if (iCurrentCmd[0].GetCmdType() == PVFM_UTILITY_COMMAND_GET_FRAME_UTILITY_BUFFER && iCurrentVideoFrameBuffer)
                {
                    // Return the buffer if allocated from utility's mempool
                    iVideoFrameBufferMemPool->deallocate(iCurrentVideoFrameBuffer);
                    iCurrentVideoFrameBuffer = NULL;
                }

                iAPICmdStatus = cmdstatus;
                if (iAPICmdErrMsg)
                {
                    iAPICmdErrMsg->removeRef();
                    iAPICmdErrMsg = NULL;
                }
                // Need to shutdown/restart player and cleanup in utility's AO
                OSCL_ASSERT(iErrorHandlingInUtilityAO == false);
                iErrorHandlingInUtilityAO = true;
                RunIfNotReady();
            }
        }
        break;

        default:
        {
            // Check if video frame buffer was allocated from utility's memory pool
            OSCL_ASSERT(iCurrentCmd.empty() == false);
            OSCL_ASSERT(iCurrentVideoFrameBuffer != NULL);
            if (iCurrentCmd[0].GetCmdType() == PVFM_UTILITY_COMMAND_GET_FRAME_UTILITY_BUFFER && iCurrentVideoFrameBuffer)
            {
                // Return the buffer if allocated from utility's mempool
                iVideoFrameBufferMemPool->deallocate(iCurrentVideoFrameBuffer);
                iCurrentVideoFrameBuffer = NULL;
            }
            PVMFErrorInfoMessageInterface* nextmsg = NULL;
            if (aCmdResp.GetEventExtensionInterface())
            {
                nextmsg = GetErrorInfoMessageInterface(*(aCmdResp.GetEventExtensionInterface()));
            }
            PVMFBasicErrorInfoMessage* errmsg = NULL;
            errmsg = CreateBasicErrInfoMessage(nextmsg);

            iAPICmdStatus = aCmdResp.GetCmdStatus();
            if (iAPICmdErrMsg)
            {
                iAPICmdErrMsg->removeRef();
                iAPICmdErrMsg = NULL;
            }
            iAPICmdErrMsg = errmsg;

            // Check if player is handling the error
            PVPlayerState playerstate;
            PVMFStatus pretval = iPlayer->GetPVPlayerStateSync(playerstate);
            if (pretval == PVMFSuccess && playerstate == PVP_STATE_ERROR)
            {
                // Yes so wait for error handling to complete
                // Start a timer just in case the player does not report error handling complete
                iTimeoutTimer->Request(PVFMUTIL_TIMERID_PLAYERERRORTIMEOUT, 0, iErrorHandlingWaitTime, this, false);
            }
            else if (pretval == PVMFSuccess && playerstate == PVP_STATE_INITIALIZED)
            {
                SetUtilityState(PVFM_UTILITY_STATE_INITIALIZED);

                // Report the command as failed
                UtilityCommandCompleted(aUtilContext.iCmdId, aUtilContext.iCmdContext, iAPICmdStatus, OSCL_STATIC_CAST(PVInterface*, iAPICmdErrMsg));
                iAPICmdStatus = PVMFSuccess;
                if (iAPICmdErrMsg)
                {
                    iAPICmdErrMsg->removeRef();
                    iAPICmdErrMsg = NULL;
                }
            }
            else
            {
                // Need to shutdown/restart player and cleanup in utility's AO
                OSCL_ASSERT(iErrorHandlingInUtilityAO == false);
                iErrorHandlingInUtilityAO = true;
                RunIfNotReady();
            }
        }
        break;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::HandleGFPlayerStopFromPaused() Out"));
}


void PVFrameAndMetadataUtility::HandleGFPlayerPrepare(PVFMUtilityContext& aUtilContext, const PVCmdResponse& aCmdResp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::HandleGFPlayerPrepare() In"));

    aUtilContext.iCmdType = -1;

    PVMFStatus cmdstatus = PVMFFailure;

    switch (aCmdResp.GetCmdStatus())
    {
        case PVMFSuccess:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                            (0, "PVFrameAndMetadataUtility::GFPlayerPrepare completed successfully Tick=%d", OsclTickCount::TickCount()));
            // Call Start() on player
            iPlayerStartCompleted = false;
            cmdstatus = DoGFPlayerStart(aUtilContext.iCmdId, aUtilContext.iCmdContext);
            if ((cmdstatus != PVMFSuccess) || (!HasVideo()))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::HandleGFPlayerPrepare() Prepare on player failed. Report command as failed"));
                // Cancel the frame retrieval
                iVideoMIO->CancelGetFrame();
                if (iCurrentCmd[0].GetCmdType() == PVFM_UTILITY_COMMAND_GET_FRAME_UTILITY_BUFFER && iCurrentVideoFrameBuffer)
                {
                    // Return the buffer if allocated from utility's mempool
                    iVideoFrameBufferMemPool->deallocate(iCurrentVideoFrameBuffer);
                    iCurrentVideoFrameBuffer = NULL;
                }

                iAPICmdStatus = cmdstatus;
                if (iAPICmdErrMsg)
                {
                    iAPICmdErrMsg->removeRef();
                    iAPICmdErrMsg = NULL;
                }
                // Need to shutdown/restart player and cleanup in utility's AO
                OSCL_ASSERT(iErrorHandlingInUtilityAO == false);
                iErrorHandlingInUtilityAO = true;
                RunIfNotReady();
            }
            break;

        default:
        {
            // Cancel the frame retrieval
            iVideoMIO->CancelGetFrame();
            if (iCurrentCmd[0].GetCmdType() == PVFM_UTILITY_COMMAND_GET_FRAME_UTILITY_BUFFER && iCurrentVideoFrameBuffer)
            {
                // Return the buffer if allocated from utility's mempool
                iVideoFrameBufferMemPool->deallocate(iCurrentVideoFrameBuffer);
                iCurrentVideoFrameBuffer = NULL;
            }
            PVFMErrorEventType errCode = PVFMErrPlayerEngine;
            if (aCmdResp.GetCmdStatus() == PVMFErrResourceConfiguration)
            {
                errCode = PVFMErrNoVideoTrack;
            }

            PVMFErrorInfoMessageInterface* nextmsg = NULL;
            if (aCmdResp.GetEventExtensionInterface())
            {
                nextmsg = GetErrorInfoMessageInterface(*(aCmdResp.GetEventExtensionInterface()));
            }
            PVMFBasicErrorInfoMessage* errmsg = NULL;
            errmsg = CreateBasicErrInfoMessage(nextmsg, errCode);

            iAPICmdStatus = aCmdResp.GetCmdStatus();
            if (iAPICmdErrMsg)
            {
                iAPICmdErrMsg->removeRef();
                iAPICmdErrMsg = NULL;
            }
            iAPICmdErrMsg = errmsg;

            // Check if player is handling the error
            PVPlayerState playerstate;
            PVMFStatus pretval = iPlayer->GetPVPlayerStateSync(playerstate);
            if (pretval == PVMFSuccess && playerstate == PVP_STATE_ERROR)
            {
                // Wait for error handling to complete
                // Start a timer just in case the player does not report error handling complete
                iTimeoutTimer->Request(PVFMUTIL_TIMERID_PLAYERERRORTIMEOUT, 0, iErrorHandlingWaitTime, this, false);
            }
            else if (aCmdResp.GetCmdStatus() == PVMFErrResourceConfiguration)
            {
                // Track selection in player engine failed since there is no video track.
                // Complete GetFrame() with not supported for this source
                UtilityCommandCompleted(iCurrentCmd[0].GetCmdId(), iCurrentCmd[0].GetContext(), PVMFErrNotSupported, OSCL_STATIC_CAST(PVInterface*, iAPICmdErrMsg));
                iAPICmdStatus = PVMFSuccess;
                if (iAPICmdErrMsg)
                {
                    iAPICmdErrMsg->removeRef();
                    iAPICmdErrMsg = NULL;
                }
            }
            else
            {
                // Need to shutdown/restart player and cleanup in utility's AO
                OSCL_ASSERT(iErrorHandlingInUtilityAO == false);
                iErrorHandlingInUtilityAO = true;
                RunIfNotReady();
            }
        }
        break;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::HandleGFPlayerPrepare() Out"));
}


void PVFrameAndMetadataUtility::HandleGFPlayerStart(PVFMUtilityContext& aUtilContext, const PVCmdResponse& aCmdResp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::HandleGFPlayerStart() In"));

    aUtilContext.iCmdType = -1;
    PVMFStatus cmdstatus = aCmdResp.GetCmdStatus();

    switch (cmdstatus)
    {
        case PVMFSuccess:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                            (0, "PVFrameAndMetadataUtility::GFPlayerStart completed successfully Tick=%d", OsclTickCount::TickCount()));
            iPlayerStartCompleted = true;

            if (iFrameReceived)
            {
                PVMFStatus retval = DoGFPlayerPause(aUtilContext.iCmdId, aUtilContext.iCmdContext);
                if (retval == PVMFErrInvalidState)
                {
                    // Playback already paused so GetFrame() command completed
                    if (iAPICmdStatus != PVMFSuccess)
                    {
                        if (iCurrentCmd[0].GetCmdType() == PVFM_UTILITY_COMMAND_GET_FRAME_UTILITY_BUFFER && iCurrentVideoFrameBuffer)
                        {
                            // Return the buffer if allocated from utility's mempool
                            iVideoFrameBufferMemPool->deallocate(iCurrentVideoFrameBuffer);
                            iCurrentVideoFrameBuffer = NULL;
                        }
                    }

                    UtilityCommandCompleted(aUtilContext.iCmdId, aUtilContext.iCmdContext, iAPICmdStatus, OSCL_STATIC_CAST(PVInterface*, iAPICmdErrMsg));

                    iAPICmdStatus = PVMFSuccess;
                    if (iAPICmdErrMsg)
                    {
                        iAPICmdErrMsg->removeRef();
                        iAPICmdErrMsg = NULL;
                    }
                }
                else if (retval != PVMFSuccess)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::HandleGFPlayerStart() Pause on player failed. Report command as failed"));

                    if (iCurrentCmd[0].GetCmdType() == PVFM_UTILITY_COMMAND_GET_FRAME_UTILITY_BUFFER && iCurrentVideoFrameBuffer)
                    {
                        // Return the buffer if allocated from utility's mempool
                        iVideoFrameBufferMemPool->deallocate(iCurrentVideoFrameBuffer);
                        iCurrentVideoFrameBuffer = NULL;
                    }

                    iAPICmdStatus = retval;
                    if (iAPICmdErrMsg)
                    {
                        iAPICmdErrMsg->removeRef();
                        iAPICmdErrMsg = NULL;
                    }

                    // Need to shutdown/restart player and cleanup in utility's AO
                    OSCL_ASSERT(iErrorHandlingInUtilityAO == false);
                    iErrorHandlingInUtilityAO = true;
                    RunIfNotReady();
                }
            }
            else
            {
                // Wait for the video MIO to report frame is ready
                // Start a timer just in case frame retrieval hangs
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::HandleGFPlayerStart() Set a timeout of %d secs for frame retrieval", iFrameReadyWaitTime));
                iTimeoutTimer->Request(PVFMUTIL_TIMERID_FRAMEREADYTIMEOUT, 0, iFrameReadyWaitTime, this, false);
            }
            break;

        default:
        {
            // Cancel the frame retrieval
            iVideoMIO->CancelGetFrame();
            if (iCurrentCmd[0].GetCmdType() == PVFM_UTILITY_COMMAND_GET_FRAME_UTILITY_BUFFER && iCurrentVideoFrameBuffer)
            {
                // Return the buffer if allocated from utility's mempool
                iVideoFrameBufferMemPool->deallocate(iCurrentVideoFrameBuffer);
                iCurrentVideoFrameBuffer = NULL;
            }

            PVMFErrorInfoMessageInterface* nextmsg = NULL;
            if (aCmdResp.GetEventExtensionInterface())
            {
                nextmsg = GetErrorInfoMessageInterface(*(aCmdResp.GetEventExtensionInterface()));
            }
            PVMFBasicErrorInfoMessage* errmsg = NULL;
            errmsg = CreateBasicErrInfoMessage(nextmsg);

            iAPICmdStatus = aCmdResp.GetCmdStatus();
            if (iAPICmdErrMsg)
            {
                iAPICmdErrMsg->removeRef();
                iAPICmdErrMsg = NULL;
            }
            iAPICmdErrMsg = errmsg;

            // Check if player is handling the error
            PVPlayerState playerstate;
            PVMFStatus pretval = iPlayer->GetPVPlayerStateSync(playerstate);
            if (pretval == PVMFSuccess && playerstate == PVP_STATE_ERROR)
            {
                // Wait for error handling to complete
                // Start a timer just in case the player does not report error handling complete
                iTimeoutTimer->Request(PVFMUTIL_TIMERID_PLAYERERRORTIMEOUT, 0, iErrorHandlingWaitTime, this, false);
            }
            else
            {
                // Need to shutdown/restart player and cleanup in utility's AO
                OSCL_ASSERT(iErrorHandlingInUtilityAO == false);
                iErrorHandlingInUtilityAO = true;
                RunIfNotReady();
            }
        }
        break;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::HandleGFPlayerStart() Out"));
}


void PVFrameAndMetadataUtility::HandleGFPlayerPause(PVFMUtilityContext& aUtilContext, const PVCmdResponse& aCmdResp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::HandleGFPlayerPause() In"));

    aUtilContext.iCmdType = -1;

    switch (aCmdResp.GetCmdStatus())
    {
        case PVMFSuccess:

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                            (0, "PVFrameAndMetadataUtility::GFPlayerPause completed successfully Tick=%d", OsclTickCount::TickCount()));
            // GetFrame() command completed
            if (iAPICmdStatus != PVMFSuccess)
            {
                if (iCurrentCmd[0].GetCmdType() == PVFM_UTILITY_COMMAND_GET_FRAME_UTILITY_BUFFER && iCurrentVideoFrameBuffer)
                {
                    // Return the buffer if allocated from utility's mempool
                    iVideoFrameBufferMemPool->deallocate(iCurrentVideoFrameBuffer);
                    iCurrentVideoFrameBuffer = NULL;
                }
            }

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                            (0, "PVFrameAndMetadataUtility::GetFrame() completed successfully Tick=%d", OsclTickCount::TickCount()));

            UtilityCommandCompleted(aUtilContext.iCmdId, aUtilContext.iCmdContext, iAPICmdStatus, OSCL_STATIC_CAST(PVInterface*, iAPICmdErrMsg));

            iAPICmdStatus = PVMFSuccess;
            if (iAPICmdErrMsg)
            {
                iAPICmdErrMsg->removeRef();
                iAPICmdErrMsg = NULL;
            }
            break;

        default:
        {
            if (iCurrentCmd[0].GetCmdType() == PVFM_UTILITY_COMMAND_GET_FRAME_UTILITY_BUFFER && iCurrentVideoFrameBuffer)
            {
                // Return the buffer if allocated from utility's mempool
                iVideoFrameBufferMemPool->deallocate(iCurrentVideoFrameBuffer);
                iCurrentVideoFrameBuffer = NULL;
            }

            PVMFErrorInfoMessageInterface* nextmsg = NULL;
            if (aCmdResp.GetEventExtensionInterface())
            {
                nextmsg = GetErrorInfoMessageInterface(*(aCmdResp.GetEventExtensionInterface()));
            }

            PVMFBasicErrorInfoMessage* errmsg = NULL;
            errmsg = CreateBasicErrInfoMessage(nextmsg);

            // Check if player is handling the error
            PVPlayerState playerstate;
            PVMFStatus pretval = iPlayer->GetPVPlayerStateSync(playerstate);
            if (pretval == PVMFSuccess && playerstate == PVP_STATE_ERROR)
            {
                // Yes so wait for error handling to complete
                if (iAPICmdStatus == PVMFSuccess)
                {
                    // Use the player error only if there is no existing error (from frame retrieval)
                    iAPICmdStatus = aCmdResp.GetCmdStatus();
                    if (iAPICmdErrMsg)
                    {
                        iAPICmdErrMsg->removeRef();
                        iAPICmdErrMsg = NULL;
                    }
                    iAPICmdErrMsg = errmsg;
                }
                // Start a timer just in case the player does not report error handling complete
                iTimeoutTimer->Request(PVFMUTIL_TIMERID_PLAYERERRORTIMEOUT, 0, iErrorHandlingWaitTime, this, false);
            }
            else if (pretval == PVMFSuccess && playerstate == PVP_STATE_INITIALIZED)
            {
                SetUtilityState(PVFM_UTILITY_STATE_INITIALIZED);

                // Report the command as failed
                if (iAPICmdStatus == PVMFSuccess)
                {
                    UtilityCommandCompleted(aUtilContext.iCmdId, aUtilContext.iCmdContext, aCmdResp.GetCmdStatus(), OSCL_STATIC_CAST(PVInterface*, errmsg));
                    if (errmsg)
                    {
                        errmsg->removeRef();
                    }
                }
                else
                {
                    UtilityCommandCompleted(aUtilContext.iCmdId, aUtilContext.iCmdContext, iAPICmdStatus, OSCL_STATIC_CAST(PVInterface*, iAPICmdErrMsg));
                    iAPICmdStatus = PVMFSuccess;
                    if (iAPICmdErrMsg)
                    {
                        iAPICmdErrMsg->removeRef();
                        iAPICmdErrMsg = NULL;
                    }
                }
            }
            else
            {
                iAPICmdStatus = aCmdResp.GetCmdStatus();
                if (iAPICmdErrMsg)
                {
                    iAPICmdErrMsg->removeRef();
                    iAPICmdErrMsg = NULL;
                }
                iAPICmdErrMsg = errmsg;
                // Need to shutdown/restart player and cleanup in utility's AO
                OSCL_ASSERT(iErrorHandlingInUtilityAO == false);
                iErrorHandlingInUtilityAO = true;
                RunIfNotReady();
            }
        }
        break;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::HandleGFPlayerPause() Out"));
}


void PVFrameAndMetadataUtility::HandleRDSPlayerStopFromPaused(PVFMUtilityContext& aUtilContext, const PVCmdResponse& aCmdResp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::HandleRDSPlayerStopFromPaused() In"));

    aUtilContext.iCmdType = -1;

    switch (aCmdResp.GetCmdStatus())
    {
        case PVMFSuccess:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                            (0, "PVFrameAndMetadataUtility::RDSPlayerStopFromPaused completed successfully Tick=%d", OsclTickCount::TickCount()));
            // Call RemoveDataSink() on player
            PVMFStatus cmdstatus = DoRDSPlayerRemoveVideoDataSink(aUtilContext.iCmdId, aUtilContext.iCmdContext);
            if (cmdstatus != PVMFSuccess)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::HandleRDSPlayerStopFromPaused() RemoveDataSink on player failed. Report command as failed"));
                iAPICmdStatus = cmdstatus;
                if (iAPICmdErrMsg)
                {
                    iAPICmdErrMsg->removeRef();
                    iAPICmdErrMsg = NULL;
                }
                // Need to shutdown/restart player and cleanup in utility's AO
                OSCL_ASSERT(iErrorHandlingInUtilityAO == false);
                iErrorHandlingInUtilityAO = true;
                RunIfNotReady();
            }
        }
        break;

        default:
        {
            PVMFErrorInfoMessageInterface* nextmsg = NULL;
            if (aCmdResp.GetEventExtensionInterface())
            {
                nextmsg = GetErrorInfoMessageInterface(*(aCmdResp.GetEventExtensionInterface()));
            }
            PVMFBasicErrorInfoMessage* errmsg = NULL;
            errmsg = CreateBasicErrInfoMessage(nextmsg);

            iAPICmdStatus = aCmdResp.GetCmdStatus();
            if (iAPICmdErrMsg)
            {
                iAPICmdErrMsg->removeRef();
                iAPICmdErrMsg = NULL;
            }
            iAPICmdErrMsg = errmsg;

            // Check if player is handling the error
            PVPlayerState playerstate;
            PVMFStatus pretval = iPlayer->GetPVPlayerStateSync(playerstate);
            if (pretval == PVMFSuccess && playerstate == PVP_STATE_ERROR)
            {
                // Yes so wait for error handling to complete
                // Start a timer just in case the player does not report error handling complete
                iTimeoutTimer->Request(PVFMUTIL_TIMERID_PLAYERERRORTIMEOUT, 0, iErrorHandlingWaitTime, this, false);
            }
            else if (pretval == PVMFSuccess && playerstate == PVP_STATE_INITIALIZED)
            {
                SetUtilityState(PVFM_UTILITY_STATE_INITIALIZED);

                // Report the command as failed
                UtilityCommandCompleted(aUtilContext.iCmdId, aUtilContext.iCmdContext, iAPICmdStatus, OSCL_STATIC_CAST(PVInterface*, iAPICmdErrMsg));
                iAPICmdStatus = PVMFSuccess;
                if (iAPICmdErrMsg)
                {
                    iAPICmdErrMsg->removeRef();
                    iAPICmdErrMsg = NULL;
                }
            }
            else
            {
                // Need to shutdown/restart player and cleanup in utility's AO
                OSCL_ASSERT(iErrorHandlingInUtilityAO == false);
                iErrorHandlingInUtilityAO = true;
                RunIfNotReady();
            }
        }
        break;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::HandleRDSPlayerStopFromPaused() Out"));
}

void PVFrameAndMetadataUtility::HandleRDSPlayerRemoveVideoDataSink(PVFMUtilityContext& aUtilContext, const PVCmdResponse& aCmdResp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility:HandleRDSPlayerRemoveVideoDataSink() In"));

    aUtilContext.iCmdType = -1;

    PVMFStatus cmdstatus = PVMFFailure;

    switch (aCmdResp.GetCmdStatus())
    {
        case PVMFSuccess:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                            (0, "PVFrameAndMetadataUtility::RDSPlayerRemoveVideoDataSink completed successfully Tick=%d", OsclTickCount::TickCount()));
            // Cleanup the data sink
            iVideoDataSink.SetDataSinkNode(NULL);
            if (iVideoNode)
            {
                PVMediaOutputNodeFactory::DeleteMediaOutputNode(iVideoNode);
                iVideoNode = NULL;
            }
            if (iVideoMIO)
            {
                OSCL_DELETE(iVideoMIO);
                iVideoMIO = NULL;
            }

            // Call Remove audio data sink on player.
            if (iAudioNode && iAudioMIO)
            {
                cmdstatus = DoRDSPlayerRemoveAudioDataSink(aUtilContext.iCmdId, aUtilContext.iCmdContext);
            }
            else
            {
                // Audio sink not available so go to player reset
                cmdstatus = DoRDSPlayerReset(aUtilContext.iCmdId, aUtilContext.iCmdContext);
            }
            if (cmdstatus != PVMFSuccess)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::HandleRDSPlayerRemoveVideoDataSink() RemoveAudioDataSink on player failed. Report command as failed"));
                iAPICmdStatus = cmdstatus;
                if (iAPICmdErrMsg)
                {
                    iAPICmdErrMsg->removeRef();
                    iAPICmdErrMsg = NULL;
                }
                // Need to shutdown/restart player and cleanup in utility's AO
                OSCL_ASSERT(iErrorHandlingInUtilityAO == false);
                iErrorHandlingInUtilityAO = true;
                RunIfNotReady();
            }
            break;

        default:
        {
            // Cleanup the data sink
            iVideoDataSink.SetDataSinkNode(NULL);
            if (iVideoNode)
            {
                PVMediaOutputNodeFactory::DeleteMediaOutputNode(iVideoNode);
                iVideoNode = NULL;
            }
            if (iVideoMIO)
            {
                OSCL_DELETE(iVideoMIO);
                iVideoMIO = NULL;
            }
            PVMFErrorInfoMessageInterface* nextmsg = NULL;
            if (aCmdResp.GetEventExtensionInterface())
            {
                nextmsg = GetErrorInfoMessageInterface(*(aCmdResp.GetEventExtensionInterface()));
            }
            PVMFBasicErrorInfoMessage* errmsg = NULL;
            errmsg = CreateBasicErrInfoMessage(nextmsg);

            iAPICmdStatus = aCmdResp.GetCmdStatus();
            if (iAPICmdErrMsg)
            {
                iAPICmdErrMsg->removeRef();
                iAPICmdErrMsg = NULL;
            }
            iAPICmdErrMsg = errmsg;

            // Check if player is handling the error
            PVPlayerState playerstate;
            PVMFStatus pretval = iPlayer->GetPVPlayerStateSync(playerstate);
            if (pretval == PVMFSuccess && playerstate == PVP_STATE_ERROR)
            {
                // Wait for error handling to complete
                // Start a timer just in case the player does not report error handling complete
                iTimeoutTimer->Request(PVFMUTIL_TIMERID_PLAYERERRORTIMEOUT, 0, iErrorHandlingWaitTime, this, false);
            }
            else if (pretval == PVMFSuccess && playerstate == PVP_STATE_INITIALIZED)
            {
                // Call Reset() on player
                cmdstatus = DoRDSPlayerReset(aUtilContext.iCmdId, aUtilContext.iCmdContext);
                if (cmdstatus != PVMFSuccess)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::HandleRDSPlayerRemoveVideoDataSink() Reset on player failed. Report command as failed"));
                    iAPICmdStatus = cmdstatus;
                    if (iAPICmdErrMsg)
                    {
                        iAPICmdErrMsg->removeRef();
                        iAPICmdErrMsg = NULL;
                    }
                    // Need to shutdown/restart player and cleanup in utility's AO
                    OSCL_ASSERT(iErrorHandlingInUtilityAO == false);
                    iErrorHandlingInUtilityAO = true;
                    RunIfNotReady();
                }
            }
            else
            {
                // Need to shutdown/restart player and cleanup in utility's AO
                OSCL_ASSERT(iErrorHandlingInUtilityAO == false);
                iErrorHandlingInUtilityAO = true;
                RunIfNotReady();
            }
        }
        break;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::HandleRDSPlayerRemoveVideoDataSink() Out"));
}

void PVFrameAndMetadataUtility::HandleRDSPlayerRemoveAudioDataSink(PVFMUtilityContext& aUtilContext, const PVCmdResponse& aCmdResp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::HandleRDSPlayerRemoveAudioDataSink() In"));

    aUtilContext.iCmdType = -1;

    PVMFStatus cmdstatus = PVMFFailure;

    switch (aCmdResp.GetCmdStatus())
    {
        case PVMFSuccess:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                            (0, "PVFrameAndMetadataUtility::RDSPlayerRemoveAudioDataSink completed successfully Tick=%d", OsclTickCount::TickCount()));
            // Cleanup the data sink
            iAudioDataSink.SetDataSinkNode(NULL);
            if (iAudioNode)
            {
                PVMediaOutputNodeFactory::DeleteMediaOutputNode(iAudioNode);
                iAudioNode = NULL;
            }
            if (iAudioMIO)
            {
                OSCL_DELETE(iAudioMIO);
                iAudioMIO = NULL;
            }

            // Call Reset() on player
            cmdstatus = DoRDSPlayerReset(aUtilContext.iCmdId, aUtilContext.iCmdContext);
            if (cmdstatus != PVMFSuccess)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::HandleRDSPlayerRemoveAudioDataSink() Reset on player failed. Report command as failed"));
                iAPICmdStatus = cmdstatus;
                if (iAPICmdErrMsg)
                {
                    iAPICmdErrMsg->removeRef();
                    iAPICmdErrMsg = NULL;
                }
                // Need to shutdown/restart player and cleanup in utility's AO
                OSCL_ASSERT(iErrorHandlingInUtilityAO == false);
                iErrorHandlingInUtilityAO = true;
                RunIfNotReady();
            }
            break;

        default:
        {
            // Cleanup the data sink
            iAudioDataSink.SetDataSinkNode(NULL);
            if (iAudioNode)
            {
                PVMediaOutputNodeFactory::DeleteMediaOutputNode(iAudioNode);
                iAudioNode = NULL;
            }
            if (iAudioMIO)
            {
                OSCL_DELETE(iAudioMIO);
                iAudioMIO = NULL;
            }
            PVMFErrorInfoMessageInterface* nextmsg = NULL;
            if (aCmdResp.GetEventExtensionInterface())
            {
                nextmsg = GetErrorInfoMessageInterface(*(aCmdResp.GetEventExtensionInterface()));
            }
            PVMFBasicErrorInfoMessage* errmsg = NULL;
            errmsg = CreateBasicErrInfoMessage(nextmsg);

            iAPICmdStatus = aCmdResp.GetCmdStatus();
            if (iAPICmdErrMsg)
            {
                iAPICmdErrMsg->removeRef();
                iAPICmdErrMsg = NULL;
            }
            iAPICmdErrMsg = errmsg;

            // Check if player is handling the error
            PVPlayerState playerstate;
            PVMFStatus pretval = iPlayer->GetPVPlayerStateSync(playerstate);
            if (pretval == PVMFSuccess && playerstate == PVP_STATE_ERROR)
            {
                // Wait for error handling to complete
                // Start a timer just in case the player does not report error handling complete
                iTimeoutTimer->Request(PVFMUTIL_TIMERID_PLAYERERRORTIMEOUT, 0, iErrorHandlingWaitTime, this, false);
            }
            else if (pretval == PVMFSuccess && playerstate == PVP_STATE_INITIALIZED)
            {
                // Call Reset() on player
                cmdstatus = DoRDSPlayerReset(aUtilContext.iCmdId, aUtilContext.iCmdContext);
                if (cmdstatus != PVMFSuccess)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::HandleRDSPlayerRemoveAudioDataSink() Reset on player failed. Report command as failed"));
                    iAPICmdStatus = cmdstatus;
                    if (iAPICmdErrMsg)
                    {
                        iAPICmdErrMsg->removeRef();
                        iAPICmdErrMsg = NULL;
                    }
                    // Need to shutdown/restart player and cleanup in utility's AO
                    OSCL_ASSERT(iErrorHandlingInUtilityAO == false);
                    iErrorHandlingInUtilityAO = true;
                    RunIfNotReady();
                }
            }
            else
            {
                // Need to shutdown/restart player and cleanup in utility's AO
                OSCL_ASSERT(iErrorHandlingInUtilityAO == false);
                iErrorHandlingInUtilityAO = true;
                RunIfNotReady();
            }
        }
        break;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::HandleRDSPlayerRemoveAudioDataSink() Out"));
}


void PVFrameAndMetadataUtility::HandleRDSPlayerReset(PVFMUtilityContext& aUtilContext, const PVCmdResponse& aCmdResp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::HandleRDSPlayerReset() In"));

    aUtilContext.iCmdType = -1;

    PVMFStatus cmdstatus = PVMFFailure;

    switch (aCmdResp.GetCmdStatus())
    {
        case PVMFSuccess:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                            (0, "PVFrameAndMetadataUtility::RDSPlayerReset completed successfully Tick=%d", OsclTickCount::TickCount()));
            // Call RemoveDataSource() on player
            cmdstatus = DoRDSPlayerRemoveDataSource(aUtilContext.iCmdId, aUtilContext.iCmdContext);
            if (cmdstatus != PVMFSuccess)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::HandleRDSPlayerReset() RemoveDataSource on player failed. Report command as failed"));

                iAPICmdStatus = cmdstatus;
                if (iAPICmdErrMsg)
                {
                    iAPICmdErrMsg->removeRef();
                    iAPICmdErrMsg = NULL;
                }
                // Need to shutdown/restart player and cleanup in utility's AO
                OSCL_ASSERT(iErrorHandlingInUtilityAO == false);
                iErrorHandlingInUtilityAO = true;
                RunIfNotReady();
            }
            break;

        default:
        {
            PVMFErrorInfoMessageInterface* nextmsg = NULL;
            if (aCmdResp.GetEventExtensionInterface())
            {
                nextmsg = GetErrorInfoMessageInterface(*(aCmdResp.GetEventExtensionInterface()));
            }
            PVMFBasicErrorInfoMessage* errmsg = NULL;
            errmsg = CreateBasicErrInfoMessage(nextmsg);

            iAPICmdStatus = aCmdResp.GetCmdStatus();
            if (iAPICmdErrMsg)
            {
                iAPICmdErrMsg->removeRef();
                iAPICmdErrMsg = NULL;
            }
            iAPICmdErrMsg = errmsg;

            // Check if player is handling the error
            PVPlayerState playerstate;
            PVMFStatus pretval = iPlayer->GetPVPlayerStateSync(playerstate);
            if (pretval == PVMFSuccess && playerstate == PVP_STATE_ERROR)
            {
                // Wait for error handling to complete
                // Start a timer just in case the player does not report error handling complete
                iTimeoutTimer->Request(PVFMUTIL_TIMERID_PLAYERERRORTIMEOUT, 0, iErrorHandlingWaitTime, this, false);
            }
            else if (pretval == PVMFSuccess && playerstate == PVP_STATE_IDLE)
            {
                // Call RemoveDataSource() on player
                cmdstatus = DoRDSPlayerRemoveDataSource(aUtilContext.iCmdId, aUtilContext.iCmdContext);
                if (cmdstatus != PVMFSuccess)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::HandleRDSPlayerReset() RemoveDataSource on player failed. Report command as failed"));
                    iAPICmdStatus = cmdstatus;
                    if (iAPICmdErrMsg)
                    {
                        iAPICmdErrMsg->removeRef();
                        iAPICmdErrMsg = NULL;
                    }
                    // Need to shutdown/restart player and cleanup in utility's AO
                    OSCL_ASSERT(iErrorHandlingInUtilityAO == false);
                    iErrorHandlingInUtilityAO = true;
                    RunIfNotReady();
                }
            }
            else
            {
                // Need to shutdown/restart player and cleanup in utility's AO
                OSCL_ASSERT(iErrorHandlingInUtilityAO == false);
                iErrorHandlingInUtilityAO = true;
                RunIfNotReady();
            }
        }
        break;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::HandleRDSPlayerReset() Out"));
}


void PVFrameAndMetadataUtility::HandleRDSPlayerRemoveDataSource(PVFMUtilityContext& aUtilContext, const PVCmdResponse& aCmdResp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::HandleRDSPlayerRemoveDataSource() In"));

    aUtilContext.iCmdType = -1;

    switch (aCmdResp.GetCmdStatus())
    {
        case PVMFSuccess:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                            (0, "PVFrameAndMetadataUtility::RDSPlayerRemoveDataSource completed successfully Tick=%d", OsclTickCount::TickCount()));
            // Utility's RemoveDataSource() successfully completed
            SetUtilityState(PVFM_UTILITY_STATE_IDLE);
            iDataSource = NULL;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                            (0, "PVFrameAndMetadataUtility::RemoveDataSource() completed successfully Tick=%d", OsclTickCount::TickCount()));

            UtilityCommandCompleted(aUtilContext.iCmdId, aUtilContext.iCmdContext, PVMFSuccess);
            break;

        default:
        {
            // Even if the player RemoveDataSource fails,
            // set the data source handle to NULL and change to idle state
            SetUtilityState(PVFM_UTILITY_STATE_IDLE);
            iDataSource = NULL;

            PVMFErrorInfoMessageInterface* nextmsg = NULL;
            if (aCmdResp.GetEventExtensionInterface())
            {
                nextmsg = GetErrorInfoMessageInterface(*(aCmdResp.GetEventExtensionInterface()));
            }

            PVMFBasicErrorInfoMessage* errmsg = NULL;
            errmsg = CreateBasicErrInfoMessage(nextmsg);

            iAPICmdStatus = aCmdResp.GetCmdStatus();
            if (iAPICmdErrMsg)
            {
                iAPICmdErrMsg->removeRef();
                iAPICmdErrMsg = NULL;
            }
            iAPICmdErrMsg = errmsg;

            // Check if player is handling the error
            PVPlayerState playerstate;
            PVMFStatus pretval = iPlayer->GetPVPlayerStateSync(playerstate);
            if (pretval == PVMFSuccess && playerstate == PVP_STATE_ERROR)
            {
                // Wait for error handling to complete
                // Start a timer just in case the player does not report error handling complete
                iTimeoutTimer->Request(PVFMUTIL_TIMERID_PLAYERERRORTIMEOUT, 0, iErrorHandlingWaitTime, this, false);
            }
            else
            {
                // Need to shutdown/restart player and cleanup in utility's AO
                OSCL_ASSERT(iErrorHandlingInUtilityAO == false);
                iErrorHandlingInUtilityAO = true;
                RunIfNotReady();
            }
        }
        break;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVFrameAndMetadataUtility::HandleRDSPlayerRemoveDataSource() Out"));
}

void PVFrameAndMetadataUtility::SetThumbnailDimensions(uint32 aWidth, uint32 aHeight)
{
    iThumbnailWidth =  aWidth;
    iThumbnailHeight = aHeight;
}

void PVFrameAndMetadataUtility::GetThumbnailDimensions(uint32 &aWidth, uint32 &aHeight)
{
    aWidth = iThumbnailWidth;
    aHeight = iThumbnailHeight;
}

PVMFErrorInfoMessageInterface* PVFrameAndMetadataUtility::GetErrorInfoMessageInterface(PVInterface& aInterface)
{
    PVInterface* temp = NULL;
    if (aInterface.queryInterface(PVMFErrorInfoMessageInterfaceUUID, temp))
    {
        PVMFErrorInfoMessageInterface* extiface = OSCL_STATIC_CAST(PVMFErrorInfoMessageInterface*, temp);
        return extiface;
    }
    else
    {
        return NULL;
    }
}

PVMFBasicErrorInfoMessage* PVFrameAndMetadataUtility::CreateBasicErrInfoMessage(PVMFErrorInfoMessageInterface* nextmsg, PVFMErrorEventType aErrEvent)
{
    PVUuid puuid = PVFrameAndMetadataErrorInfoEventTypesUUID;
    PVMFBasicErrorInfoMessage* errmsg = NULL;
    int32 leavecode = OsclErrNone;
    OSCL_TRY(leavecode, errmsg = OSCL_NEW(PVMFBasicErrorInfoMessage, (aErrEvent, puuid, nextmsg)));
    OSCL_FIRST_CATCH_ANY(leavecode,
                         PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFrameAndMetadataUtility::CreateBasicErrInfoMessage() Instantiation of error msg did a leave!"));
                         errmsg = NULL;);
    return errmsg;
}
