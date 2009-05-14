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
#ifndef PVAUTHORENGINE_H_INCLUDED
#include "pvauthorengine.h"
#endif
#ifndef OSCL_ERROR_CODES_H_INCLUDED
#include "oscl_error_codes.h"
#endif
#ifndef PVLOGGER_ACCESSORIES_H_INCLUDED
#include "pvlogger_accessories.h"
#endif
#ifndef PVAUTHORENGINEFACTORY_H_INCLUDED
#include "pvauthorenginefactory.h"
#endif
#ifndef PVAE_NODE_FACTORY_UTILITY_H_INCLUDED
#include "pvaenodefactoryutility.h"
#endif
#ifndef OSCL_DLL_H_INCLUDED
#include "oscl_dll.h"
#endif
#ifndef PVMF_COMPOSER_SIZE_AND_DURATION_H_INCLUDED
#include "pvmf_composer_size_and_duration.h"
#endif
#ifndef PVAE_TUNEABLES_H_INCLUDED
#include "pvae_tuneables.h"
#endif

#ifndef PVMI_CONFIG_AND_CAPABILITY_OBSERVER_H_INCLUDED
#include "pvmi_config_and_capability_observer.h"
#endif
#ifndef OSCL_MIME_STRING_UTILS_H
#include "pv_mime_string_utils.h"
#endif

#ifndef PVMI_KVP_UTIL_H_INCLUDED
#include "pvmi_kvp_util.h"
#endif
#define PVAE_NUM_PENDING_CMDS 10
#define PVAE_NUM_PENDING_EVENTS 10

#include "pv_author_sdkinfo.h"


// Define entry point for this DLL
OSCL_DLL_ENTRY_POINT_DEFAULT()

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVAuthorEngineInterface* PVAuthorEngineFactory::CreateAuthor(PVCommandStatusObserver* aCmdStatusObserver,
        PVErrorEventObserver* aErrorEventObserver,
        PVInformationalEventObserver* aInfoEventObserver)
{
    return PVAuthorEngine::Create(aCmdStatusObserver, aErrorEventObserver, aInfoEventObserver);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF bool PVAuthorEngineFactory::DeleteAuthor(PVAuthorEngineInterface* aAuthor)
{
    if (aAuthor)
    {
        OSCL_DELETE(aAuthor);
        return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////
PVAuthorEngineInterface* PVAuthorEngine::Create(PVCommandStatusObserver* aCmdStatusObserver,
        PVErrorEventObserver *aErrorEventObserver,
        PVInformationalEventObserver *aInfoEventObserver)
{
    PVAuthorEngine* engine = OSCL_NEW(PVAuthorEngine, ());
    if (engine)
        engine->Construct(aCmdStatusObserver, aErrorEventObserver, aInfoEventObserver);
    return (PVAuthorEngineInterface*)engine;
}

////////////////////////////////////////////////////////////////////////////
PVAuthorEngine::PVAuthorEngine() :
        OsclTimerObject(OsclActiveObject::EPriorityNominal, "PVAuthorEngine"),
        iCommandId(0),
        iCmdStatusObserver(NULL),
        iInfoEventObserver(NULL),
        iErrorEventObserver(NULL),
        iEncodedVideoFormat(PVMF_MIME_FORMAT_UNKNOWN),
        iState(PVAE_STATE_IDLE),
        iCfgCapCmdObserver(NULL),
        iAsyncNumElements(0)
{
    iLogger = PVLogger::GetLoggerObject("PVAuthorEngine");
    iDoResetNodeContainers = false;
}

////////////////////////////////////////////////////////////////////////////
void PVAuthorEngine::Construct(PVCommandStatusObserver* aCmdStatusObserver,
                               PVErrorEventObserver *aErrorEventObserver,
                               PVInformationalEventObserver *aInfoEventObserver)
{
    iCmdStatusObserver = aCmdStatusObserver;
    iInfoEventObserver = aInfoEventObserver;
    iErrorEventObserver = aErrorEventObserver;

    iPendingCmds.reserve(PVAE_NUM_PENDING_CMDS);
    iPendingEvents.reserve(PVAE_NUM_PENDING_EVENTS);

    iNodeUtil.SetObserver(*this);

    AddToScheduler();
    return;
}

////////////////////////////////////////////////////////////////////////////
PVAuthorEngine::~PVAuthorEngine()
{
    Cancel();
    iPendingCmds.clear();
    iPendingEvents.clear();
    ResetNodeContainers();
    while (!iDataSourcePool.empty())
    {
        DeallocateNodeContainer(iDataSourcePool, iDataSourcePool[0]->iNode);
    }

    while (!iDataSinkPool.empty())
    {
        DeallocateNodeContainer(iDataSinkPool, iDataSinkPool[0]->iNode);
    }
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVCommandId PVAuthorEngine::SetLogAppender(const char* aTag,
        PVLoggerAppender& aAppender,
        const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVAuthorEngine::SetLogAppender"));
    OSCL_UNUSED_ARG(aTag);
    OSCL_UNUSED_ARG(aAppender);
    OSCL_UNUSED_ARG(aContextData);
    OSCL_LEAVE(OsclErrNotSupported);
    return -1;
}


////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVCommandId PVAuthorEngine::RemoveLogAppender(const char* aTag,
        PVLoggerAppender& aAppender,
        const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVAuthorEngine::RemoveLogAppender"));
    OSCL_UNUSED_ARG(aTag);
    OSCL_UNUSED_ARG(aAppender);
    OSCL_UNUSED_ARG(aContextData);
    OSCL_LEAVE(OsclErrNotSupported);
    return -1;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVCommandId PVAuthorEngine::SetLogLevel(const char* aTag, int32 aLevel, bool aSetSubtree,
        const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVAuthorEngine::SetLogLevel"));
    OSCL_UNUSED_ARG(aTag);
    OSCL_UNUSED_ARG(aLevel);
    OSCL_UNUSED_ARG(aSetSubtree);
    OSCL_UNUSED_ARG(aContextData);
    OSCL_LEAVE(OsclErrNotSupported);
    return -1;
}


////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVCommandId PVAuthorEngine::GetLogLevel(const char* aTag, PVLogLevelInfo& aLogInfo,
        const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVAuthorEngine::GetLogLevel"));
    OSCL_UNUSED_ARG(aTag);
    OSCL_UNUSED_ARG(aLogInfo);
    OSCL_UNUSED_ARG(aContextData);
    OSCL_LEAVE(OsclErrNotSupported);
    return -1;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVCommandId PVAuthorEngine::Open(const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVAuthorEngine::Open: aContextData=0x%x", aContextData));

    PVEngineCommand cmd(PVAE_CMD_OPEN, iCommandId, (OsclAny*)aContextData, NULL);
    Dispatch(cmd);
    return iCommandId++;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVCommandId PVAuthorEngine::Close(const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVAuthorEngine::Close: aContextData=0x%x", aContextData));

    PVEngineCommand cmd(PVAE_CMD_CLOSE, iCommandId, (OsclAny*)aContextData);
    Dispatch(cmd);
    return iCommandId++;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVCommandId PVAuthorEngine::AddDataSource(const PVMFNodeInterface& aDataSource, const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVAuthorEngine::AddDataSource: &aDataSource=0x%x, aContextData=0x%x", &aDataSource, aContextData));

    PVEngineCommand cmd(PVAE_CMD_ADD_DATA_SOURCE, iCommandId, (OsclAny*)aContextData, (OsclAny*)&aDataSource);
    Dispatch(cmd);
    return iCommandId++;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVCommandId PVAuthorEngine::RemoveDataSource(const PVMFNodeInterface& aDataSource,
        const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVAuthorEngine::RemoveDataSource: &aDataSource=0x%x, aContextData=0x%x", &aDataSource, aContextData));

    PVEngineCommand cmd(PVAE_CMD_REMOVE_DATA_SOURCE, iCommandId, (OsclAny*)aContextData, (OsclAny*)&aDataSource);
    Dispatch(cmd);
    return iCommandId++;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVCommandId PVAuthorEngine::SelectComposer(const PvmfMimeString& aComposerType,
        PVInterface*& aConfigInterface,
        const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVAuthorEngine::SelectComposer: aConfigInterface=0x%x, aContextData=0x%x",
                     aConfigInterface, aContextData));
    PVEngineCommand cmd(PVAE_CMD_SELECT_COMPOSER, iCommandId, (OsclAny*)aContextData, (OsclAny*)(&aConfigInterface));
    cmd.SetMimeType(aComposerType);
    Dispatch(cmd);

    return iCommandId++;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVCommandId PVAuthorEngine::SelectComposer(const PVUuid& aComposerUuid,
        PVInterface*& aConfigInterface,
        const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVAuthorEngine::SelectComposer: aConfigInterface=0x%x, aContextData=0x%x",
                     aConfigInterface, aContextData));

    PVEngineCommand cmd(PVAE_CMD_SELECT_COMPOSER, iCommandId, (OsclAny*)aContextData, (OsclAny*)&aConfigInterface);
    cmd.SetUuid(aComposerUuid);
    Dispatch(cmd);
    return iCommandId++;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVCommandId PVAuthorEngine::AddMediaTrack(const PVMFNodeInterface& aDataSource,
        const PvmfMimeString& aEncoderType,
        const OsclAny* aComposer,
        PVInterface*& aConfigInterface,
        const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVAuthorEngine::AddMediaTrack: &aDataSource=0x%x, aComposer=0x%x, aConfigInterface=0x%x, aContextData=0x%x",
                     &aDataSource, aComposer, aConfigInterface, aContextData));

    PVEngineCommand cmd(PVAE_CMD_ADD_MEDIA_TRACK, iCommandId, (OsclAny*)aContextData,
                        (OsclAny*)&aDataSource, (OsclAny*)aComposer, (OsclAny*)&aConfigInterface);
    cmd.SetMimeType(aEncoderType);
    Dispatch(cmd);
    return iCommandId++;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVCommandId PVAuthorEngine::AddMediaTrack(const PVMFNodeInterface& aDataSource,
        const PVUuid& aEncoderUuid,
        const OsclAny* aComposer,
        PVInterface*& aConfigInterface,
        const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVAuthorEngine::AddMediaTrack: &aDataSource=0x%x, aComposer=0x%x, \
                     aConfigInterface=0x%x, aContextData=0x%x",
                     &aDataSource, aConfigInterface, aContextData));


    PVEngineCommand cmd(PVAE_CMD_ADD_MEDIA_TRACK, iCommandId, (OsclAny*)aContextData,
                        (OsclAny*)&aDataSource, (OsclAny*)aComposer, (OsclAny*)&aConfigInterface);
    cmd.SetUuid(aEncoderUuid);
    Dispatch(cmd);
    return iCommandId++;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVCommandId PVAuthorEngine::AddDataSink(const PVMFNodeInterface& aDataSink,
        const OsclAny* aComposer,
        const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVAuthorEngine::AddDataSink: &aDataSink=0x%x, aComposer=0x%x, aContextData=0x%x",
                     &aDataSink, aComposer, aContextData));

    PVEngineCommand cmd(PVAE_CMD_ADD_DATA_SINK, iCommandId, (OsclAny*)aContextData,
                        (OsclAny*)&aDataSink, (OsclAny*)aComposer);
    Dispatch(cmd);
    return iCommandId++;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVCommandId PVAuthorEngine::RemoveDataSink(const PVMFNodeInterface& aDataSink, const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVAuthorEngine::RemoveDataSink: &aDataSink=0x%x, aContextData=0x%x", &aDataSink, aContextData));

    PVEngineCommand cmd(PVAE_CMD_REMOVE_DATA_SINK, iCommandId, (OsclAny*)aContextData, (OsclAny*)&aDataSink);
    Dispatch(cmd);
    return iCommandId++;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVCommandId PVAuthorEngine::Init(const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVAuthorEngine::Init: aContextData=0x%x", aContextData));

    PVEngineCommand cmd(PVAE_CMD_INIT, iCommandId, (OsclAny*)aContextData);
    Dispatch(cmd);
    return iCommandId++;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVCommandId PVAuthorEngine::Reset(const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVAuthorEngine::Reset: aContextData=0x%x", aContextData));

    PVEngineCommand cmd(PVAE_CMD_RESET, iCommandId, (OsclAny*)aContextData);
    Dispatch(cmd);
    return iCommandId++;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVCommandId PVAuthorEngine::Start(const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVAuthorEngine::Start: aContextData=0x%x", aContextData));

    PVEngineCommand cmd(PVAE_CMD_START, iCommandId, (OsclAny*)aContextData);
    Dispatch(cmd);
    return iCommandId++;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVCommandId PVAuthorEngine::Pause(const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVAuthorEngine::Pause: aContextData=0x%x", aContextData));

    PVEngineCommand cmd(PVAE_CMD_PAUSE, iCommandId, (OsclAny*)aContextData);
    Dispatch(cmd);
    return iCommandId++;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVCommandId PVAuthorEngine::Resume(const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVAuthorEngine::Resume: aContextData=0x%x", aContextData));

    PVEngineCommand cmd(PVAE_CMD_RESUME, iCommandId, (OsclAny*)aContextData);
    Dispatch(cmd);
    return iCommandId++;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVCommandId PVAuthorEngine::Stop(const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVAuthorEngine::Stop: aContextData=0x%x", aContextData));

    PVEngineCommand cmd(PVAE_CMD_STOP, iCommandId, (OsclAny*)aContextData);
    Dispatch(cmd);
    return iCommandId++;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVCommandId PVAuthorEngine::QueryUUID(const PvmfMimeString& aMimeType,
        Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids,
        bool aExactUuidsOnly,
        const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVAuthorEngine::QueryUUID: &aUuid=0x%x, aExactUuidsOnly=%d, aContextData=0x%x",
                     &aUuids, aExactUuidsOnly, aContextData));

    PVEngineCommand cmd(PVAE_CMD_QUERY_UUID, iCommandId, (OsclAny*)aContextData,
                        (OsclAny*)&aUuids, (OsclAny*)&aExactUuidsOnly);
    cmd.SetMimeType(aMimeType);
    Dispatch(cmd);
    return iCommandId++;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVCommandId PVAuthorEngine::QueryInterface(const PVUuid& aUuid,
        PVInterface*& aInterfacePtr,
        const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVAuthorEngine::QueryInterface: aInterfacePtr=0x%x, aContextData=0x%x", aInterfacePtr, aContextData));

    PVEngineCommand cmd(PVAE_CMD_QUERY_INTERFACE, iCommandId, (OsclAny*)aContextData, (OsclAny*)&aInterfacePtr);
    cmd.SetUuid(aUuid);
    Dispatch(cmd);
    return iCommandId++;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVAEState PVAuthorEngine::GetPVAuthorState()
{
    return GetPVAEState();
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVCommandId PVAuthorEngine::GetSDKInfo(PVSDKInfo &aSDKInfo, const OsclAny* aContextData)
{
    PVEngineCommand cmd(PVAE_CMD_GET_SDK_INFO, iCommandId, (OsclAny*)aContextData, (OsclAny*)&aSDKInfo);
    Dispatch(cmd);
    return iCommandId++;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVCommandId PVAuthorEngine::GetSDKModuleInfo(PVSDKModuleInfo &aSDKModuleInfo, const OsclAny* aContextData)
{
    PVEngineCommand cmd(PVAE_CMD_GET_SDK_MODULE_INFO, iCommandId, (OsclAny*)aContextData, (OsclAny*)&aSDKModuleInfo);
    Dispatch(cmd);
    return iCommandId++;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVCommandId PVAuthorEngine::CancelAllCommands(const OsclAny* aContextData)
{
    PVEngineCommand cmd(PVAE_CMD_CANCEL_ALL_COMMANDS, iCommandId, (OsclAny*)aContextData);
    Dispatch(cmd);

    return iCommandId++;
}

////////////////////////////////////////////////////////////////////////////
void PVAuthorEngine::HandleNodeErrorEvent(const PVMFAsyncEvent& aEvent)
{
    OSCL_UNUSED_ARG(aEvent);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                    (0, "PVAuthorEngine::HandleNodeErrorEvent"));

    if ((!iPendingCmds.empty()) && (iState != PVAE_STATE_ERROR)) //if there is a pending command
    {
        PVEngineCommand cmd = iPendingCmds.front();
        int cmdtype = cmd.GetCmdType();

        if (cmdtype == PVAE_CMD_RESET)
        {
            return; //ignore
        }
        else
        {
            SetPVAEState(PVAE_STATE_ERROR);
            CompleteEngineCommand(iPendingCmds[0], PVMFFailure);
        }
    }
    else if (iState != PVAE_STATE_ERROR) //no pending command*
    {
        SetPVAEState(PVAE_STATE_ERROR);
    }
}

////////////////////////////////////////////////////////////////////////////
void PVAuthorEngine::HandleNodeInformationalEvent(const PVMFAsyncEvent& aEvent)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVAuthorEngine::HandleNodeInformationalEvent"));

    switch (aEvent.GetEventType())
    {
        case PVMF_COMPOSER_MAXFILESIZE_REACHED:
        {
            PVEngineCommand cmd(PVAE_CMD_STOP_MAX_SIZE, 0, NULL, NULL);
            PushCmdInFront(cmd);
        }
        break;
        case PVMF_COMPOSER_MAXDURATION_REACHED:
        {
            PVEngineCommand cmd(PVAE_CMD_STOP_MAX_DURATION, 0, NULL, NULL);
            PushCmdInFront(cmd);
        }
        break;
        case PVMF_COMPOSER_EOS_REACHED:
        case PVMFInfoEndOfData:
        {
            PVEngineCommand cmd(PVAE_CMD_STOP_EOS_REACHED, 0, NULL, NULL);
            PushCmdInFront(cmd);
        }
        break;
        case PVMF_COMPOSER_FILESIZE_PROGRESS:
        case PVMF_COMPOSER_DURATION_PROGRESS:
        {
            PVAsyncInformationalEvent event(aEvent.GetEventType(), aEvent.GetEventData());
            iInfoEventObserver->HandleInformationalEvent(event);
        }
        break;

        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVAuthorEngine::HandleNodeInformationalEvent sends unknown eventType:%d", aEvent.GetEventType()));
            break;
    }
}

////////////////////////////////////////////////////////////////////////////
void PVAuthorEngine::NodeUtilCommandCompleted(const PVMFCmdResp& aResponse)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVAuthorEngine::NodeUtilCommandCompleted"));

    // Retrieve the first pending command from queue
    PVEngineCommand cmd(iPendingCmds[0]);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVAuthorEngine::NodeUtilCommandCompleted cmdType:%d", cmd.GetCmdType()));

    if (aResponse.GetCmdStatus() != PVMFSuccess)
    {
        if (cmd.GetCmdType() == PVAE_CMD_RESET)
        {
            OSCL_ASSERT(false);//Reset can't fail.
            return;
        }
        else
            SetPVAEState(PVAE_STATE_ERROR);
    }
    //RESET needs to be handled seperately, if the EngineState is ERROR, ignore all cmds till
    //there are more pending commands, else send out commandComplete Failure
    if ((iState == PVAE_STATE_ERROR) && (cmd.GetCmdType() != PVAE_CMD_RESET))
    {
        if (iNodeUtil.GetCommandQueueSize() > 0)
        {
            return; //Ignore the command
        }
        else
        {
            CompleteEngineCommand(cmd, PVMFFailure); //Send Failure to this command, engine is in error state
            return;
        }
    }

    PVMFStatus status = PVMFSuccess;
    OsclAny* responseData = NULL;
    int32 responseDataSize = 0;

    switch (cmd.GetCmdType())
    {
        case PVAE_CMD_ADD_DATA_SOURCE:
            status = PVMFSuccess;
            break;
        case PVAE_CMD_REMOVE_DATA_SOURCE:
            break;
        case PVAE_CMD_SELECT_COMPOSER:
            if (iNodeUtil.GetCommandQueueSize() > 0)
            {
                status = PVMFPending;
            }
            else
            {
                responseData = OSCL_REINTERPRET_CAST(OsclAny*, iComposerNodes.back()->iNode);
                responseDataSize = sizeof(OsclAny*);
            }
            break;

        case PVAE_CMD_ADD_MEDIA_TRACK:
            if (iNodeUtil.GetCommandQueueSize() > 0)
                status = PVMFPending;
            break;

        case PVAE_CMD_ADD_DATA_SINK:
            break;

        case PVAE_CMD_REMOVE_DATA_SINK:
            break;

        case PVAE_CMD_INIT:
            if (iNodeUtil.GetCommandQueueSize() > 0)
                status = PVMFPending;
            else
                SetPVAEState(PVAE_STATE_INITIALIZED); // Init done. Change state
            break;

        case PVAE_CMD_RESET:
            if (iNodeUtil.GetCommandQueueSize() > 0)
            {
                status = PVMFPending;
            }
            else
            {
                // Reset done. Change state
                SetPVAEState(PVAE_STATE_OPENED);
                iDoResetNodeContainers = true;
                // Composer and Encoders cannot be deleted here right away as
                // we are still in call back of node command complete invoked
                // by one of node which can be a composer/encoder node itself
                // just return from here after scheduling itself.
                // Because Reset command is not yet pop-ed out from cmd queue
                // when in next run reset is processed with state
                // PVAE_STATE_OPENED composer/encoder nodes get deleted.

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger,
                                PVLOGMSG_STACK_TRACE, (0, "PVAuthorEngine::NodeUtilCommandCompleted: Exit"));
                RunIfNotReady();
                return ;
            }
            break;

        case PVAE_CMD_START:
        case PVAE_CMD_RESUME:
            if (iNodeUtil.GetCommandQueueSize() > 0)
                status = PVMFPending;
            else
                SetPVAEState(PVAE_STATE_RECORDING); // Start done. Change state
            break;

        case PVAE_CMD_PAUSE:
            if (iNodeUtil.GetCommandQueueSize() > 0)
                status = PVMFPending;
            else
                SetPVAEState(PVAE_STATE_PAUSED); // Pause done. Change state
            break;

        case PVAE_CMD_STOP:
        case PVAE_CMD_STOP_MAX_SIZE:
        case PVAE_CMD_STOP_MAX_DURATION:
        case PVAE_CMD_STOP_EOS_REACHED:
            if (iNodeUtil.GetCommandQueueSize() > 0)
                status = PVMFPending;
            else
                SetPVAEState(PVAE_STATE_INITIALIZED); // Stop done. Change state
            break;

        default:
            break;
    }

    if (status != PVMFPending)
    {
        if (iState == PVAE_STATE_ERROR)
        {
            CompleteEngineCommand(cmd, PVMFFailure);
        }
        else
        {
            CompleteEngineCommand(cmd, status, responseData, responseDataSize);
        }
    }
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVAuthorEngine::NodeUtilCommandCompleted: Exit"));
}

////////////////////////////////////////////////////////////////////////////
void PVAuthorEngine::NodeUtilErrorEvent(const PVMFAsyncEvent& aEvent)
{
    LOG_ERR((0, "PVAuthorEngine::NodeUtilErrorEvent"));
    HandleNodeErrorEvent(aEvent);
}

////////////////////////////////////////////////////////////////////////////
void PVAuthorEngine::Run()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVAuthorEngine::Run: Enter"));

    if (iPendingCmds.empty())
        return;

    // Retrieve the first pending command from queue
    PVEngineCommand cmd(iPendingCmds[0]);

    PVMFStatus status = PVMFFailure;
    switch (cmd.GetCmdType())
    {
        case PVAE_CMD_GET_SDK_INFO:
        case PVAE_CMD_GET_SDK_MODULE_INFO:
        case PVAE_CMD_SET_LOG_APPENDER:
        case PVAE_CMD_REMOVE_LOG_APPENDER:
        case PVAE_CMD_SET_LOG_LEVEL:
        case PVAE_CMD_GET_LOG_LEVEL:
        case PVAE_CMD_QUERY_UUID:
        case PVAE_CMD_CANCEL_ALL_COMMANDS:
            status = PVMFErrNotSupported;
            break;
        case PVAE_CMD_QUERY_INTERFACE:
            status = DoQueryInterface(cmd);
            break;
        case PVAE_CMD_OPEN:
            status = DoOpen(cmd);
            break;
        case PVAE_CMD_CLOSE:
            status = DoClose(cmd);
            break;
        case PVAE_CMD_ADD_DATA_SOURCE:
            status = DoAddDataSource(cmd);
            break;
        case PVAE_CMD_REMOVE_DATA_SOURCE:
            status = DoRemoveDataSource(cmd);
            break;
        case PVAE_CMD_SELECT_COMPOSER:
            status = DoSelectComposer(cmd);
            break;
        case PVAE_CMD_ADD_MEDIA_TRACK:
            status = DoAddMediaTrack(cmd);
            break;
        case PVAE_CMD_ADD_DATA_SINK:
            status = DoAddDataSink(cmd);
            break;
        case PVAE_CMD_REMOVE_DATA_SINK:
            status = DoRemoveDataSink(cmd);
            break;
        case PVAE_CMD_INIT:
            status = DoInit(cmd);
            break;
        case PVAE_CMD_RESET:
            status = DoReset(cmd);
            break;
        case PVAE_CMD_START:
            status = DoStart(cmd);
            break;
        case PVAE_CMD_PAUSE:
            status = DoPause(cmd);
            break;
        case PVAE_CMD_RESUME:
            status = DoResume(cmd);
            break;
        case PVAE_CMD_STOP:
            status = DoStop(cmd);
            break;
        case PVAE_CMD_STOP_MAX_SIZE:
        case PVAE_CMD_STOP_MAX_DURATION:
        case PVAE_CMD_STOP_EOS_REACHED:
            status = DoStopMaxSizeDuration();
            break;
        case PVAE_CMD_CAPCONFIG_SET_PARAMETERS:
            status = DoCapConfigSetParameters(cmd, false);
            break;
        default:
            status = PVMFErrNotSupported;
            break;
    }

    if (status != PVMFPending)
        CompleteEngineCommand(cmd, status);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVAuthorEngine::Run: Exit"));
}

////////////////////////////////////////////////////////////////////////////
void PVAuthorEngine::Dispatch(PVEngineCommand& aCmd)
{
    iPendingCmds.push_back(aCmd);

    // Call RunIfNotReady only if the newly added command is the only one
    // in the queue.  Otherwise, it will be processed after the current
    // command is complete
    if (iPendingCmds.size() == 1)
        RunIfNotReady();
}

////////////////////////////////////////////////////////////////////////////
void PVAuthorEngine::Dispatch(PVEngineAsyncEvent& aEvent)
{
    iPendingEvents.push_back(aEvent);

    // Call RunIfNotReady only if the newly added command is the only one
    // in the queue.  Otherwise, it will be processed after the current
    // command is complete
    if (iPendingCmds.size() == 1)
        RunIfNotReady();
}

void PVAuthorEngine::PushCmdInFront(PVEngineCommand& aCmd)
{
    uint32 idx = 0;
    while (idx < iPendingCmds.size())
    {
        PVEngineCommand cmdPenCmd(iPendingCmds[idx++]);
        if (cmdPenCmd.GetCmdType() == PVAE_CMD_RESET) //dont erase RESET
        {
            return;
        }
    }
    //Author engine processes commands in the order recvd.This is true for API commands, since author does
    //not support cancelall yet.when composer node eventually reports any informational event like PVMF_COMPOSER_MAXFILESIZE_REACHED,
    //author engine should queue this cmd into iPendingCmds using "push_front"
    iPendingCmds.push_front(aCmd);
    // Call RunIfNotReady only if the newly added command is the only one
    // in the queue.  Otherwise, it will be processed after the current
    // command is complete
    if (iPendingCmds.size() == 1)
        RunIfNotReady();
}

////////////////////////////////////////////////////////////////////////////
void PVAuthorEngine::CompleteEngineCommand(PVEngineCommand& aCmd, PVMFStatus aStatus,
        OsclAny* aResponseData, int32 aResponseDataSize)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVAuthorEngine::CompleteEngineCommand: aStatus=0x%x, aResponseData=0x%x, aResponseDataSize=%d",
                     aStatus, aResponseData, aResponseDataSize));

    // Erase command from command queue
    if (!iPendingCmds.empty())
    {
        iPendingCmds.erase(iPendingCmds.begin());
    }

    if (IsPVMFErrCode(aStatus))
    {
        PVCmdResponse response(aCmd.GetCmdId(), aCmd.GetContext(), aStatus, aResponseData, aResponseDataSize);
        iCmdStatusObserver->CommandCompleted(response);
        // if there are pending commands, send command complete and erase them
        while (!iPendingCmds.empty())
        {
            PVEngineCommand cmdPenCmd(iPendingCmds[0]);
            if (cmdPenCmd.GetCmdType() != PVAE_CMD_RESET) //dont erase RESET
            {
                PVCmdResponse response(cmdPenCmd.GetCmdId(), cmdPenCmd.GetContext(), aStatus, NULL, 0);
                iCmdStatusObserver->CommandCompleted(response);
                iPendingCmds.erase(iPendingCmds.begin());
            }
            else
            {
                break;
            }
        }
        if (!iPendingCmds.empty()) //if the RESET is still pending
        {
            RunIfNotReady();
        }
        return;
    }

    // Callback to engine observer
    switch (aCmd.GetCmdType())
    {
        case PVAE_CMD_STOP_MAX_SIZE:
        {
            PVAsyncInformationalEvent event(PVMF_COMPOSER_MAXFILESIZE_REACHED, NULL);
            iInfoEventObserver->HandleInformationalEvent(event);
        }
        break;

        case PVAE_CMD_STOP_MAX_DURATION:
        {
            PVAsyncInformationalEvent event(PVMF_COMPOSER_MAXDURATION_REACHED, NULL);
            iInfoEventObserver->HandleInformationalEvent(event);
        }
        break;
        case PVAE_CMD_STOP_EOS_REACHED:
        {
            PVAsyncInformationalEvent event(PVMF_COMPOSER_EOS_REACHED, NULL);
            iInfoEventObserver->HandleInformationalEvent(event);
        }
        break;
        case PVAE_CMD_CAPCONFIG_SET_PARAMETERS:
            // Send callback to the specified observer
            if (iCfgCapCmdObserver)
            {
                iCfgCapCmdObserver->SignalEvent(0);
            }
            break;
        default:
        {
            PVCmdResponse response(aCmd.GetCmdId(), aCmd.GetContext(), aStatus, aResponseData, aResponseDataSize);
            iCmdStatusObserver->CommandCompleted(response);
        }
        break;
    }
    // Run next command if there are pending commands
    if (!iPendingCmds.empty())
        RunIfNotReady();
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVAuthorEngine::DoOpen(PVEngineCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVAuthorEngine::DoOpen"));

    OSCL_UNUSED_ARG(aCmd);

    if (GetPVAEState() != PVAE_STATE_IDLE)
    {
        return PVMFErrInvalidState;
    }
    else
    {
        SetPVAEState(PVAE_STATE_OPENED);
        return PVMFSuccess;
    }
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVAuthorEngine::DoClose(PVEngineCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVAuthorEngine::DoClose"));

    OSCL_UNUSED_ARG(aCmd);

    switch (GetPVAEState())
    {
        case PVAE_STATE_OPENED:
            while (!iDataSourcePool.empty())
            {
                OSCL_DELETE(iDataSourcePool[0]);
                iDataSourcePool.erase(iDataSourcePool.begin());
            }

            while (!iDataSinkPool.empty())
            {
                OSCL_DELETE(iDataSinkPool[0]);
                iDataSinkPool.erase(iDataSinkPool.begin());
            }

            SetPVAEState(PVAE_STATE_IDLE);
            return PVMFSuccess;

        case PVAE_STATE_IDLE:
            return PVMFSuccess;

        default:
            return PVMFErrInvalidState;
    }
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVAuthorEngine::DoAddDataSource(PVEngineCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVAuthorEngine::DoAddDataSource"));

    if (GetPVAEState() != PVAE_STATE_OPENED)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVAuthorEngine::DoAddDataSource: Error - State is not OPENED. state=%d", GetPVAEState()));
        return PVMFErrInvalidState;
    }

    PVMFNodeInterface* node = OSCL_REINTERPRET_CAST(PVMFNodeInterface*, aCmd.GetParam1());
    if (!node)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVAuthorEngine::DoAddDataSource: Error - Data source node == NULL"));
        return PVMFFailure;
    }

    int32 err = 0;
    PVAENodeContainer* nodeContainer = AllocateNodeContainer(node);
    if (!nodeContainer)
    {
        LOG_ERR((0, "PVAuthorEngine::DoSelectComposer: Error - AllocateNodeContainer failed"));
        return PVMFErrNoMemory;
    }

    OSCL_TRY(err, iDataSourcePool.push_back(nodeContainer););
    OSCL_FIRST_CATCH_ANY(err,
                         LOG_ERR((0, "PVAuthorEngine::DoSelectComposer: Error - iComposerNodes.push_back() failed"));
                         OSCL_DELETE(nodeContainer);
                         return PVMFErrNoMemory;
                        );

    PVMFStatus retval = iNodeUtil.Init(nodeContainer);
    if (retval != PVMFPending)
    {
        LOG_ERR((0, "PVAuthorEngine::DoAddDataSource: Error - Init(datasrc) failed"));
        DeallocateNodeContainer(iDataSourcePool, node);
    }

    return retval;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVAuthorEngine::DoRemoveDataSource(PVEngineCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVAuthorEngine::DoRemoveDataSource"));

    if (GetPVAEState() != PVAE_STATE_OPENED)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVAuthorEngine::DoRemoveDataSource: Error - State is not OPENED. state=%d", GetPVAEState()));
        return PVMFErrInvalidState;
    }

    PVMFNodeInterface* node = OSCL_REINTERPRET_CAST(PVMFNodeInterface*, aCmd.GetParam1());
    return DeallocateNodeContainer(iDataSourcePool, node);
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVAuthorEngine::DoSelectComposer(PVEngineCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVAuthorEngine::DoSelectComposer"));

    if (GetPVAEState() != PVAE_STATE_OPENED)
    {
        LOG_ERR((0, "PVAuthorEngine::DoSelectComposer: Error - State is not OPENED. state=%d", GetPVAEState()));
        return PVMFErrInvalidState;
    }

    PVUuid uuid;
    if (aCmd.GetUuid() == uuid)
    {
        // No specified UUID, go query for a suitable one
        OSCL_HeapString<OsclMemAllocator> mimeType = aCmd.GetMimeType();
        PVAuthorEngineNodeFactoryUtility::QueryRegistry(mimeType, uuid);
    }
    else
    {
        // Use the specified UUID
        uuid = aCmd.GetUuid();
    }

    PVMFNodeInterface* node = PVAuthorEngineNodeFactoryUtility::CreateComposer(uuid);
    if (!node)
    {
        LOG_ERR((0, "PVAuthorEngine::DoSelectComposer: Error - PVAuthorEngineNodeFactoryUtility::CreateComposer failed"));
        return PVMFFailure;
    }

    int32 err = 0;
    PVAENodeContainer* nodeContainer = AllocateNodeContainer(node);
    if (!nodeContainer)
    {
        LOG_ERR((0, "PVAuthorEngine::DoSelectComposer: Error - AllocateNodeContainer failed"));
        PVAuthorEngineNodeFactoryUtility::Delete(uuid, node);
        return PVMFErrNoMemory;
    }
    nodeContainer->iUuid = uuid;

    OSCL_TRY(err, iComposerNodes.push_back(nodeContainer););
    OSCL_FIRST_CATCH_ANY(err,
                         LOG_ERR((0, "PVAuthorEngine::DoSelectComposer: Error - iComposerNodes.push_back() failed"));
                         OSCL_DELETE(nodeContainer);
                         PVAuthorEngineNodeFactoryUtility::Delete(uuid, node);
                         return PVMFErrNoMemory;
                        );
    OSCL_TRY(err,
             if (QueryNodeConfig(nodeContainer, *((PVInterface**)aCmd.GetParam1())) != PVMFPending)
{
    LOG_ERR((0, "PVAuthorEngine::DoSelectComposer: Error - QueryNodeConfig failed"));
        OSCL_LEAVE(OsclErrGeneral);
    }

    if (iNodeUtil.Init(nodeContainer) != PVMFPending)
{
    LOG_ERR((0, "PVAuthorEngine::DoSelectComposer: Error - Init(composer) failed"));
        OSCL_LEAVE(OsclErrGeneral);
    }
            );

    OSCL_FIRST_CATCH_ANY(err,
                         DeallocateNodeContainer(iComposerNodes, node);
                         PVAuthorEngineNodeFactoryUtility::Delete(uuid, node);
                         return PVMFFailure;
                        );

    return PVMFPending;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVAuthorEngine::DoAddMediaTrack(PVEngineCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVAuthorEngine::DoAddMediaTrack"));

    if (GetPVAEState() != PVAE_STATE_OPENED)
    {
        return PVMFErrInvalidState;
    }

    // Find container for data source node
    PVAENodeContainer* inputNodeContainer = GetNodeContainer(iDataSourcePool, (PVMFNodeInterface*)aCmd.GetParam1());
    if (!inputNodeContainer)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVAuthorEngine::DoAddMediaTrack: Error - Source node is not among the ones added to AddDataSource()"));
        return PVMFFailure;
    }

    // Find container for composer node and get its capability
    PVAENodeContainer* composerNodeContainer = GetNodeContainer(iComposerNodes, (PVMFNodeInterface*)aCmd.GetParam2());
    if (!composerNodeContainer)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVAuthorEngine::DoAddMediaTrack: Error - Composer node is not among the ones selected by SelectComposer()"));
        return PVMFFailure;
    }
    PVMFNodeCapability composerNodeCapability;
    composerNodeContainer->iNode->GetCapability(composerNodeCapability);

    // Set encoder output format type in PVMFFormatType
    OSCL_HeapString<OsclMemAllocator> compressedFormatMimeType;
    if (GetPvmfFormatString(compressedFormatMimeType, aCmd.GetMimeType()) != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVAuthorEngine::DoAddMediaTrack: Error - Encoder MIME type not supported"));
        return PVMFFailure;
    }

    bool compressedDataSrc = false;
    if (IsCompressedFormatDataSource(inputNodeContainer, compressedDataSrc) != PVMFSuccess)
    {
        LOG_ERR((0, "PVAuthorEngine::DoAddMediaTrack: Error - IsCompressedFormatDataSource() failed"));
        return PVMFFailure;
    }
    LOG_DEBUG((0, "PVAuthorEngine::DoAddMediaTrack: compressedDataSrc=%d", compressedDataSrc));

    int32 err = 0;
    OSCL_TRY(err, iDataSourceNodes.push_back(inputNodeContainer););
    OSCL_FIRST_CATCH_ANY(err,
                         LOG_ERR((0, "PVAuthorEngine::DoAddMediaTrack: Error - iDataSourceNodes.push_back failed"));
                         return PVMFFailure;
                        );

    if (compressedDataSrc)
    {
        // Encoder is not used in the graph. Connect source node with composer directly.
        // The composer node will be the master port in the connection attempt
        if (iNodeUtil.Connect(composerNodeContainer, PVAE_NODE_INPUT_PORT_TAG,
                              inputNodeContainer, PVAE_NODE_OUTPUT_PORT_TAG, compressedFormatMimeType) != PVMFPending)
        {
            LOG_ERR((0, "PVAuthorEngine::DoAddMediaTrack: Error - Connect(composer, datasrc) failed"));
            return PVMFFailure;
        }
    }
    else
    {
        PVUuid uuid;
        if (aCmd.GetUuid() == uuid)
        {
            // Input node outputs unencoded data. Query for a suitable encoder
            PVAuthorEngineNodeFactoryUtility::QueryRegistry(aCmd.GetMimeType(), uuid);
        }
        else
        {
            // Use the specified UUID
            uuid = aCmd.GetUuid();
        }

        PVMFNodeInterface* node = PVAuthorEngineNodeFactoryUtility::CreateEncoder(uuid);
        if (!node)
        {
            LOG_ERR((0, "PVAuthorEngine::DoAddMediaTrack: Error - PVAuthorEngineNodeFactoryUtility::CreateEncoder failed"));
            return PVMFErrNoMemory;
        }

        PVAENodeContainer* encoderNodeContainer = AllocateNodeContainer(node);
        if (!encoderNodeContainer)
        {
            LOG_ERR((0, "PVAuthorEngine::DoAddMediaTrack: Error - AllocateNodeContainer failed"));
            PVAuthorEngineNodeFactoryUtility::Delete(uuid, node);
            return PVMFErrNoMemory;
        }
        encoderNodeContainer->iUuid = uuid;

        OSCL_TRY(err, iEncoderNodes.push_back(encoderNodeContainer););
        OSCL_FIRST_CATCH_ANY(err,
                             LOG_ERR((0, "PVAuthorEngine::DoAddMediaTrack: Error - iEncoderNodes.push_back() failed"));
                             OSCL_DELETE(encoderNodeContainer);
                             PVAuthorEngineNodeFactoryUtility::Delete(uuid, node);
                             return PVMFErrNoMemory;
                            );

        OSCL_TRY(err,
                 if (QueryNodeConfig(encoderNodeContainer, *((PVInterface**)aCmd.GetParam3())) != PVMFPending)
    {
        LOG_ERR((0, "PVAuthorEngine::DoAddMediaTrack: Error - QueryNodeConfig failed"));
            OSCL_LEAVE(OsclErrGeneral);
        }

        if (iNodeUtil.Init(encoderNodeContainer) != PVMFPending)
    {
        LOG_ERR((0, "PVAuthorEngine::DoAddMediaTrack: Error - Init(encoder) failed"));
            OSCL_LEAVE(OsclErrGeneral);
        }

        // Connect data source to encoder. The encoder node is the master port
        // in this connection. An empty string is specified as the mime type
        // to allow the ports to negotiate formats
        if (iNodeUtil.Connect(encoderNodeContainer, PVAE_NODE_INPUT_PORT_TAG,
                              inputNodeContainer, PVAE_NODE_OUTPUT_PORT_TAG, OSCL_StackString<1>("")) != PVMFPending)
    {
        LOG_ERR((0, "PVAuthorEngine::DoAddMediaTrack: Error - Connect(encoder, datasrc) failed"));
            OSCL_LEAVE(OsclErrGeneral);
        }

        // Connect encoder to composer.  The encoder node will be the master port in the connection attempt
        // The PVMF mime type of the compressed format is specified here
        if (iNodeUtil.Connect(encoderNodeContainer, PVAE_NODE_OUTPUT_PORT_TAG,
                              composerNodeContainer, PVAE_NODE_INPUT_PORT_TAG, compressedFormatMimeType) != PVMFPending)
    {
        LOG_ERR((0, "PVAuthorEngine::DoAddMediaTrack: Error - Connect(encoder, composer) failed"));
            OSCL_LEAVE(OsclErrGeneral);
        }
                );

        OSCL_FIRST_CATCH_ANY(err,
                             DeallocateNodeContainer(iEncoderNodes, node);
                             PVAuthorEngineNodeFactoryUtility::Delete(uuid, node);
                             return PVMFFailure;
                            );
    }

    return PVMFPending;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVAuthorEngine::DoAddDataSink(PVEngineCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVAuthorEngine::DoAddDataSink"));

    OSCL_UNUSED_ARG(aCmd);

    if (GetPVAEState() != PVAE_STATE_OPENED)
    {
        return PVMFErrInvalidState;
    }
    else
    {
        return PVMFSuccess;
    }
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVAuthorEngine::DoRemoveDataSink(PVEngineCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVAuthorEngine::DoRemoveDataSink"));

    OSCL_UNUSED_ARG(aCmd);

    if (GetPVAEState() != PVAE_STATE_OPENED)
    {
        return PVMFErrInvalidState;
    }
    else
    {
        return PVMFSuccess;
    }
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVAuthorEngine::DoInit(PVEngineCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVAuthorEngine::DoInit"));

    OSCL_UNUSED_ARG(aCmd);

    if (GetPVAEState() != PVAE_STATE_OPENED)
    {
        return PVMFErrInvalidState;
    }

    if (iComposerNodes.size() == 0 || iDataSourceNodes.size() == 0)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVAuthorEngine::DoInit: Error - SelectComposer() or AddMediaTrack() has not been called"));
        return PVMFFailure;
    }

    iNodeUtil.Prepare(iComposerNodes);
    if (iEncoderNodes.size() > 0)
        iNodeUtil.Prepare(iEncoderNodes);
    iNodeUtil.Prepare(iDataSourceNodes);
    PVUuid iUuid1 = PVMI_CAPABILITY_AND_CONFIG_PVUUID;

    for (uint ii = 0; ii < iEncoderNodes.size(); ii++)
    {
        // call queryInterface of amr/video encoder node
        // retrieve capability class pointer in iEncoderNode
        // capability class pointer fetched from amr/video encoder node in iEncoderNode
        iEncoderNodes[ii]->iNode->QueryInterface(iEncoderNodes[ii]->iSessionId,
                iUuid1,
                iEncoderNodes[ii]->iNodeCapConfigIF,
                NULL);
    }

    for (uint jj = 0; jj < iComposerNodes.size(); jj++)
    {
        // call queryInterface of file-ouput/mp4-composer node
        // retrieve capability class pointer in iComposerNode
        // capability class pointer fetched from file-ouput/mp4-composer node in iComposerNode
        iComposerNodes[jj]->iNode->QueryInterface(iComposerNodes[jj]->iSessionId,
                iUuid1,
                iComposerNodes[jj]->iNodeCapConfigIF,
                NULL);
    }

    for (uint kk = 0; kk < iDataSourceNodes.size(); kk++)
    {
        // call queryInterface of media io node
        // retrieve capability class pointer in iDataSourceNodes
        // capability class pointer fetched from media io node in iDataSourceNodes
        iDataSourceNodes[kk]->iNode->QueryInterface(iDataSourceNodes[kk]->iSessionId,
                iUuid1,
                iDataSourceNodes[kk]->iNodeCapConfigIF,
                NULL);
    }
    return PVMFPending;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVAuthorEngine::DoReset(PVEngineCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVAuthorEngine::DoReset"));

    OSCL_UNUSED_ARG(aCmd);

    if (GetPVAEState() == PVAE_STATE_IDLE)
    {
        return PVMFErrInvalidState;
    }

    else
    {
        if ((iDataSourceNodes.size() == 0)
                && (iEncoderNodes.size() == 0)
                && (iComposerNodes.size() == 0))
        {
            // If there is no source/encoder/composer node present
            // there is nothing to reset, return success rightaway.
            return PVMFSuccess;
        }
        //First call reset on all child nodes, source nodes, encoder nodes, composer nodes
        //Pls note that since reset on child nodes can be called from any state, and since
        //node reset needs to clean up everything, there is no need to thru an asynchronous
        //release port sequence
        //Once all these resets complete delete them all. We use the boolean iDoResetNodeContainers
        //to get back in the context of author engine AO to delete stuff. We cannot delete stuff in
        //NodeUtilCommandCompleted
        if (iDoResetNodeContainers)
        {
            iDoResetNodeContainers = false;
            // While RESET-ing, the Composer and Encoder nodes are to be
            // deleted as there no user API to do deallocations for the
            // allocations done in the api's DoSelectComposer() and AddMediaTrack()
            ResetNodeContainers();
            return PVMFSuccess;
        }
        ResetGraph();
    }
    return PVMFPending;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVAuthorEngine::DoStart(PVEngineCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVAuthorEngine::DoStart"));

    OSCL_UNUSED_ARG(aCmd);

    if (GetPVAEState() != PVAE_STATE_INITIALIZED)
    {
        return PVMFErrInvalidState;
    }

    iNodeUtil.Start(iComposerNodes);
    if (iEncoderNodes.size() > 0)
        iNodeUtil.Start(iEncoderNodes);
    iNodeUtil.Start(iDataSourceNodes);
    return PVMFPending;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVAuthorEngine::DoPause(PVEngineCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVAuthorEngine::DoPause"));

    OSCL_UNUSED_ARG(aCmd);

    if (GetPVAEState() != PVAE_STATE_RECORDING)
    {
        return PVMFErrInvalidState;
    }

    iNodeUtil.Pause(iDataSourceNodes);
    if (iEncoderNodes.size() > 0)
        iNodeUtil.Pause(iEncoderNodes);
    iNodeUtil.Pause(iComposerNodes);
    return PVMFPending;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVAuthorEngine::DoResume(PVEngineCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVAuthorEngine::DoResume"));

    OSCL_UNUSED_ARG(aCmd);

    if (GetPVAEState() != PVAE_STATE_PAUSED)
    {
        return PVMFErrInvalidState;
    }

    iNodeUtil.Start(iComposerNodes);
    if (iEncoderNodes.size() > 0)
        iNodeUtil.Start(iEncoderNodes);
    iNodeUtil.Start(iDataSourceNodes);
    return PVMFPending;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVAuthorEngine::DoStop(PVEngineCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVAuthorEngine::DoStop"));

    OSCL_UNUSED_ARG(aCmd);

    switch (GetPVAEState())
    {
        case PVAE_STATE_RECORDING:
        case PVAE_STATE_PAUSED:
            iNodeUtil.Flush(iDataSourceNodes);
            if (iEncoderNodes.size() > 0)
                iNodeUtil.Flush(iEncoderNodes);
            iNodeUtil.Flush(iComposerNodes);
            return PVMFPending;

        default:
            return PVMFErrInvalidState;
    }
}

////////////////////////////////////////////////////////////////////////////
void PVAuthorEngine::ResetNodeContainers()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVAuthorEngine::ResetNodeContainers"));

    if (iAllNodes.size() > 0)
        iAllNodes.clear();

    uint32 i, j;
    for (i = 0; i < iDataSourceNodes.size(); i++)
    {
        for (j = 0; j < iDataSourceNodes[i]->iOutputPorts.size(); j++)
        {
            iDataSourceNodes[i]->iOutputPorts[j]->Disconnect();
        }
    }
    iDataSourceNodes.clear();

    PVUuid uuid;
    PVMFNodeInterface* node;

    while (!iComposerNodes.empty())
    {
        uuid = iComposerNodes[0]->iUuid;
        node = iComposerNodes[0]->iNode;
        for (j = 0; j < iComposerNodes[0]->iExtensions.size(); j++)
        {
            iComposerNodes[0]->iExtensions[j]->removeRef();
        }
        iComposerNodes[0]->iExtensions.clear();
        iComposerNodes[0]->iExtensionUuids.clear();
        iComposerNodes[0]->iInputPorts.clear();
        iComposerNodes[0]->iOutputPorts.clear();
        DeallocateNodeContainer(iComposerNodes, node);
        PVAuthorEngineNodeFactoryUtility::Delete(uuid, node);

    }

    while (!iEncoderNodes.empty())
    {
        uuid = iEncoderNodes[0]->iUuid;
        node = iEncoderNodes[0]->iNode;
        for (j = 0; j < iEncoderNodes[0]->iExtensions.size(); j++)
        {
            iEncoderNodes[0]->iExtensions[j]->removeRef();
        }
        iEncoderNodes[0]->iExtensions.clear();
        iEncoderNodes[0]->iExtensionUuids.clear();
        iEncoderNodes[0]->iInputPorts.clear();
        iEncoderNodes[0]->iOutputPorts.clear();
        DeallocateNodeContainer(iEncoderNodes, node);
        PVAuthorEngineNodeFactoryUtility::Delete(uuid, node);

    }

    return;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVAuthorEngine::DoQueryInterface(PVEngineCommand& aCmd)
{
    if (aCmd.GetUuid() == PvmfComposerSizeAndDurationUuid && !iComposerNodes.empty())
    {
        return iNodeUtil.QueryInterface(iComposerNodes[0], aCmd.GetUuid(),
                                        *((PVInterface**)(aCmd.iParam1)), aCmd.GetContext());
    }

    else if (aCmd.GetUuid() == PVMI_CAPABILITY_AND_CONFIG_PVUUID)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVAuthorEngine::DoQueryInterface() In"));
        PVInterface** ifptr = (PVInterface**)(aCmd.GetParam1());
        PVUuid uuid = aCmd.GetUuid();
        if (NULL == ifptr)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVAuthorEngine::DoQueryInterface() Passed in parameter invalid."));
            return PVMFErrArgument;
        }

        PVMFStatus cmdstatus = PVMFSuccess;
        if (false == queryInterface(uuid, *ifptr))
        {
            cmdstatus = PVMFErrNotSupported;
        }
        else
        {
            (*ifptr)->addRef();
        }

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVAuthorEngine::DoQueryInterface() Out"));
        return PVMFSuccess;
    }
    // No interface available
    return PVMFFailure;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVAuthorEngine::DoStopMaxSizeDuration()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVAuthorEngine::DoStopMaxSizeDuration"));

    switch (GetPVAEState())
    {
        case PVAE_STATE_RECORDING:
        case PVAE_STATE_PAUSED:
            iNodeUtil.Stop(iDataSourceNodes);
            if (iEncoderNodes.size() > 0)
                iNodeUtil.Stop(iEncoderNodes);
            return PVMFPending;

        default:
            return PVMFErrInvalidState;
    }
}

////////////////////////////////////////////////////////////////////////////
void PVAuthorEngine::SetPVAEState(PVAEState aState)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVAuthorEngine::SetPVAEState: aState=%d", aState));
    iState = aState;
}

////////////////////////////////////////////////////////////////////////////
PVAENodeContainer* PVAuthorEngine::GetNodeContainer(PVAENodeContainerVector& aNodeContainerVector,
        PVMFNodeInterface* aNode)
{
    for (uint32 i = 0; i < aNodeContainerVector.size(); i++)
    {
        if (aNodeContainerVector[i]->iNode == aNode)
            return aNodeContainerVector[i];
    }

    return NULL;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVAuthorEngine::IsCompressedFormatDataSource(PVAENodeContainer* aDataSrc, bool& aIsCompressedFormat)
{
    LOG_STACK_TRACE((0, "PVAuthorEngine::IsCompressedFormatDataSource"));

    // Get input node capability
    PVMFNodeCapability capability;
    if (aDataSrc->iNode->GetCapability(capability) != PVMFSuccess)
    {
        LOG_ERR((0, "PVAuthorEngine::IsCompressedFormatDataSource: Error - GetCapability failed"));
        return PVMFFailure;
    }
    aIsCompressedFormat = false;
    for (uint32 i = 0; i < capability.iOutputFormatCapability.size(); i++)
    {
        PVMFFormatType format = (capability.iOutputFormatCapability[i]);
        if (format.isCompressed() || format.isText())
        {
            aIsCompressedFormat = true;
            return PVMFSuccess;
        }
    }

    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
void PVAuthorEngine::ResetGraph()
{

    if (iAllNodes.size() != (iEncoderNodes.size() + iDataSourceNodes.size() + iComposerNodes.size()))
    {

        for (uint ii = 0; ii < iEncoderNodes.size(); ii++)
        {
            iAllNodes.push_back(iEncoderNodes[ii]);
        }
        for (uint jj = 0; jj < iDataSourceNodes.size(); jj++)
        {
            iAllNodes.push_back(iDataSourceNodes[jj]);
        }
        for (uint kk = 0; kk < iComposerNodes.size(); kk++)
        {
            iAllNodes.push_back(iComposerNodes[kk]);
        }
    }


    if (iAllNodes.size() > 0)
    {
        iNodeUtil.Reset(iAllNodes);
    }

}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVAuthorEngine::QueryNodeConfig(PVAENodeContainer* aNodeContainer, PVInterface*& aInterface)
{
    PVUuid uuid;
    if (!PVAuthorEngineNodeFactoryUtility::QueryNodeConfigUuid(aNodeContainer->iUuid, uuid))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "PVAuthorEngine::QueryNodeConfig: No configuration interface available for this node."));
        return PVMFSuccess;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                    (0, "PVAuthorEngine::QueryNodeConfig: Configuration interface available."));

    if (iNodeUtil.QueryInterface(aNodeContainer, uuid, aInterface) != PVMFPending)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVAuthorEngine::QueryNodeConfig: Error - QueryInterface failed"));
        return PVMFFailure;
    }

    return PVMFPending;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVAuthorEngine::GetPvmfFormatString(PvmfMimeString& aMimeType, const PvmfMimeString& aNodeMimeType)
{
    if (aNodeMimeType == KMp4EncMimeType)
    {
        aMimeType = PVMF_MIME_M4V;
    }
    else if (aNodeMimeType == KH263EncMimeType)
    {
        aMimeType = PVMF_MIME_H2632000;
    }
    else if (aNodeMimeType == KAmrNbEncMimeType ||
             aNodeMimeType == KAMRNbComposerMimeType)
    {
        aMimeType = PVMF_MIME_AMR_IETF;
    }
    else if (aNodeMimeType == KAMRWbEncMimeType)
    {
        aMimeType = PVMF_MIME_AMRWB_IETF;
    }
    else if (aNodeMimeType == KAACADIFEncMimeType ||
             aNodeMimeType == KAACADIFComposerMimeType)
    {
        aMimeType = PVMF_MIME_ADIF;
    }
    else if (aNodeMimeType == KAACADTSEncMimeType ||
             aNodeMimeType == KAACADTSComposerMimeType)
    {
        aMimeType = PVMF_MIME_ADTS;
    }
    else if (aNodeMimeType == KAACMP4EncMimeType)
    {
        aMimeType = PVMF_MIME_MPEG4_AUDIO;
    }
    else if (aNodeMimeType == KH264EncMimeType)
    {
        aMimeType = PVMF_MIME_H264_VIDEO_MP4;
    }
    else if (aNodeMimeType == KTextEncMimeType)
    {
        aMimeType = PVMF_MIME_3GPP_TIMEDTEXT;
    }
    ////////////////////////////////////////////////////////////////////////////
    // Future development: When integrating support for a new format, add a
    // mapping for the new format and return the corresponding PVMFFormatType
    // for the Mime type
    ////////////////////////////////////////////////////////////////////////////
    else
    {
        LOG_ERR((0, "PVAuthorEngine::GetPvmfFormatString: Error - Mapping not found"));
        return PVMFFailure;
    }

    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
PVAENodeContainer* PVAuthorEngine::AllocateNodeContainer(PVMFNodeInterface* aNode)
{
    int32 err = 0;
    PVAENodeContainer* nodeContainer = NULL;
    OSCL_TRY(err,
             nodeContainer = OSCL_NEW(PVAENodeContainer, ());
             if (!nodeContainer)
             OSCL_LEAVE(OsclErrNoMemory);

             // Do thread logon and create a session to the node
             PVMFNodeSessionInfo session((PVMFNodeCmdStatusObserver*)&iNodeUtil, this, nodeContainer,
                                         this, nodeContainer);

             aNode->ThreadLogon();
             nodeContainer->iSessionId = aNode->Connect(session);

             nodeContainer->iNode = aNode;
            );

    OSCL_FIRST_CATCH_ANY(err,
                         LOG_ERR((0, "PVAuthorEngine::AllocateNodeContainer: Error - Node container allocation failed"));
                         return NULL;
                        );

    return nodeContainer;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVAuthorEngine::DeallocateNodeContainer(PVAENodeContainerVector& aVector,
        PVMFNodeInterface* aNode)
{
    for (uint32 i = 0; i < aVector.size(); i++)
    {
        if (aVector[i]->iNode == aNode)
        {
            aVector[i]->iNode->ThreadLogoff();
            aVector[i]->iNode->Disconnect(aVector[i]->iSessionId);
            OSCL_DELETE(aVector[i]);
            aVector.erase(&aVector[i]);
            return PVMFSuccess;
        }
    }

    LOG_ERR((0, "PVAuthorEngine::DeallocateNodeContainer: Error - Container of node 0x%x not found", aNode));
    return PVMFFailure;
}



PVMFStatus PVAuthorEngine::DoCapConfigSetParameters(PVEngineCommand& aCmd, bool aSyncCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVAuthorEngine::DoCapConfigSetParameters() In"));

    PvmiKvp* paramkvp = NULL;
    int numparam = 0;
    PvmiKvp** retkvp;

    paramkvp = (PvmiKvp*)(aCmd.GetParam1());
    numparam = *(int*)(aCmd.GetParam2());
    retkvp = (PvmiKvp**)(aCmd.GetParam3());

    if (NULL == paramkvp || NULL == retkvp || numparam < 1)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVAuthorEngine::DoCapConfigSetParameters() Passed in parameter invalid"));
        return PVMFErrArgument;
    }

    // Go through each parameter
    for (int32 paramind = 0; paramind < numparam; ++paramind)
    {
        // Count the number of components and parameters in the key
        int compcount = pv_mime_string_compcnt(paramkvp[paramind].key);
        // Retrieve the first component from the key string

        if (compcount < 2)
        {
            *retkvp = &paramkvp[paramind];
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVAuthorEngine::DoCapConfigSetParameters() Unsupported key"));
            return PVMFErrArgument;
        }

        if (pv_mime_strcmp(paramkvp[paramind].key, _STRLIT_CHAR("x-pvmf/author")) >= 0)
        {
            if (3 == compcount)
            {
                // Verify and set the passed-in author setting
                PVMFStatus retval = DoVerifyAndSetAuthorParameter(paramkvp[paramind], true);
                if (retval != PVMFSuccess)
                {
                    *retkvp = &paramkvp[paramind];
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVAuthorEngine::DoCapConfigSetParameters() Setting parameter %d failed", paramind));
                    return retval;
                }
            }
            else if (4 == compcount)
            {
                // Only product info keys have four components
                // Verify and set the passed-in product info setting
                PVMFStatus retval = DoVerifyAndSetAuthorProductInfoParameter(paramkvp[paramind], true);
                if (retval != PVMFSuccess)
                {
                    *retkvp = &paramkvp[paramind];
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVAuthorEngine::DoCapConfigSetParameters() Setting parameter %d failed", paramind));
                    return retval;
                }
            }
            else
            {
                // Do not support more than 4 components right now
                *retkvp = &paramkvp[paramind];
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVAuthorEngine::DoCapConfigSetParameters() Unsupported key"));
                return PVMFErrArgument;
            }
        }
        else
        {
            // Determine which node's cap-config IF needs to be used
            Oscl_Vector<PVInterface*, OsclMemAllocator> nodecapconfigif;
            PVMFStatus retval = DoQueryNodeCapConfig(paramkvp[paramind].key, nodecapconfigif);
            *retkvp = &paramkvp[paramind];
            if (retval == PVMFSuccess && !(nodecapconfigif.empty()))
            {
                uint32 nodeind = 0;
                bool anysuccess = false;
                // Go through each returned node's cap-config until successful
                while (nodeind < nodecapconfigif.size())
                {
                    *retkvp = NULL;
                    ((PvmiCapabilityAndConfig*)nodecapconfigif[nodeind])->setParametersSync(NULL, &paramkvp[paramind], 1, *retkvp);
                    ++nodeind;
                    if (*retkvp == NULL && anysuccess == false)
                    {
                        anysuccess = true;
                    }
                }
                if (anysuccess == false)
                {
                    // setParametersSync was not accepted by the node(s)
                    *retkvp = &paramkvp[paramind];
                    return PVMFErrArgument;
                }
            }
            else
            {
                // Unknown key string
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVAuthorEngine::DoCapConfigSetParameters() Unsupported key"));
                return PVMFErrArgument;
            }
        }
    }

    if (!aSyncCmd)
    {
        return PVMFSuccess;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVAuthorEngine::DoCapConfigSetParameters() Out"));
    return PVMFSuccess;
}


PVMFStatus PVAuthorEngine::DoQueryNodeCapConfig(char* aKeySubString, Oscl_Vector<PVInterface*, OsclMemAllocator>& aNodeCapConfigIF)
{
    int32 leavecode = 0;

    aNodeCapConfigIF.clear();

    if (aKeySubString == NULL)
    {
        return PVMFErrArgument;
    }

    // check for "encoder/video" or "encoder/audio" keyword in MIME string
    if ((pv_mime_strcmp(aKeySubString, _STRLIT_CHAR("x-pvmf/encoder/video")) >= 0) ||
            (pv_mime_strcmp(aKeySubString, _STRLIT_CHAR("x-pvmf/encoder/audio")) >= 0))
    {
        leavecode = 0;
        // insert capability class pointer of amr/video encoder/avc encoder node in aNodeCapConfigIF
        OSCL_TRY(leavecode,
                 for (uint ii = 0; ii < iEncoderNodes.size(); ii++)
    {
        if (iEncoderNodes[ii]->iNodeCapConfigIF)
            {
                aNodeCapConfigIF.push_back(iEncoderNodes[ii]->iNodeCapConfigIF);
            }
        }
                );
        OSCL_FIRST_CATCH_ANY(leavecode, return PVMFErrNoMemory);

    }
    // check for "fileio" or "file/output" or "x-pvmf/composer" keyword in MIME string
    else if ((pv_mime_strcmp(aKeySubString, _STRLIT_CHAR("x-pvmf/composer")) >= 0) ||
             (pv_mime_strcmp(aKeySubString, _STRLIT_CHAR("fileio")) >= 0) ||
             (pv_mime_strcmp(aKeySubString, _STRLIT_CHAR("x-pvmf/file/output")) >= 0))
    {
        leavecode = 0;
        // insert capability class pointer of mp4-composer/file-output node in aNodeCapConfigIF
        OSCL_TRY(leavecode,
                 for (uint jj = 0; jj < iComposerNodes.size(); jj++)
    {
        if (iComposerNodes[jj]->iNodeCapConfigIF)
            {
                aNodeCapConfigIF.push_back(iComposerNodes[jj]->iNodeCapConfigIF);
            }
        }
                );
        OSCL_FIRST_CATCH_ANY(leavecode, return PVMFErrNoMemory);

    }
    // check for "x-pvmf/media-io" or "x-pvmf/datasource" keyword in MIME string
    else if ((pv_mime_strcmp(aKeySubString, _STRLIT_CHAR("x-pvmf/datasource")) >= 0) ||
             (pv_mime_strcmp(aKeySubString, _STRLIT_CHAR("x-pvmf/media-io")) >= 0))
    {
        leavecode = 0;
        // insert capability class pointer of media io node in aNodeCapConfigIF
        OSCL_TRY(leavecode,
                 for (uint k = 0; k < iDataSourceNodes.size(); k++)
    {
        if (iDataSourceNodes[k]->iNodeCapConfigIF)
            {
                aNodeCapConfigIF.push_back(iDataSourceNodes[k]->iNodeCapConfigIF);
            }
        }
                );
        OSCL_FIRST_CATCH_ANY(leavecode, return PVMFErrNoMemory);
    }

    return PVMFSuccess;
}

void PVAuthorEngine::setParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters, int aNumElements, PvmiKvp* &aRetKVP)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVAuthorEngine::setParametersSync()"));
    OSCL_UNUSED_ARG(aSession);
    PVEngineCommand cmd(PVAE_CMD_CAPCONFIG_SET_PARAMETERS, 0, NULL, aParameters, (OsclAny*)&aNumElements, (OsclAny*)&aRetKVP);

    // Complete the request synchronously
    DoCapConfigSetParameters(cmd, true);
}

PVMFCommandId PVAuthorEngine::setParametersAsync(PvmiMIOSession aSession, PvmiKvp* aParameters, int aNumElements, PvmiKvp*& aRetKVP, OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVAuthorEngine::setParametersAsync()"));
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aContext);
    iAsyncNumElements = aNumElements;
    PVEngineCommand cmd(PVAE_CMD_CAPCONFIG_SET_PARAMETERS, 0, NULL, aParameters, (OsclAny*)&iAsyncNumElements, (OsclAny*)&aRetKVP);
    Dispatch(cmd);
    return iCommandId++;

}


uint32 PVAuthorEngine::getCapabilityMetric(PvmiMIOSession aSession)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVAuthorEngine::getCapabilityMetric()"));
    OSCL_UNUSED_ARG(aSession);
    // Not supported so return 0
    return 0;
}


PVMFStatus PVAuthorEngine::verifyParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters, int aNumElements)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVAuthorEngine::verifyParametersSync()"));
    OSCL_UNUSED_ARG(aSession);

    return DoCapConfigVerifyParameters(aParameters, aNumElements);
}


void PVAuthorEngine::setObserver(PvmiConfigAndCapabilityCmdObserver* aObserver)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVAuthorEngine::setObserver()"));
    iCfgCapCmdObserver = aObserver;
}


PVMFStatus PVAuthorEngine::getParametersSync(PvmiMIOSession aSession, PvmiKeyType aIdentifier, PvmiKvp*& aParameters, int& aNumParamElements, PvmiCapabilityContext aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVAuthorEngine::getParametersSync()"));
    OSCL_UNUSED_ARG(aSession);

    return DoCapConfigGetParametersSync(aIdentifier, aParameters, aNumParamElements, aContext);
}


PVMFStatus PVAuthorEngine::releaseParameters(PvmiMIOSession aSession, PvmiKvp* aParameters, int aNumElements)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVAuthorEngine::releaseParameters()"));
    OSCL_UNUSED_ARG(aSession);
    return DoCapConfigReleaseParameters(aParameters, aNumElements);
}

PVMFStatus PVAuthorEngine::DoCapConfigVerifyParameters(PvmiKvp* aParameters, int aNumElements)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVAuthorEngine::DoCapConfigVerifyParameters() In"));

    if (NULL == aParameters || aNumElements < 1)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVAuthorEngine::DoCapConfigVerifyParameters() Passed in parameter invalid"));
        return PVMFErrArgument;
    }

    // Go through each parameter and verify
    for (int32 paramind = 0; paramind < aNumElements; ++paramind)
    {
        // Count the number of components and parameters in the key
        int compcount = pv_mime_string_compcnt(aParameters[paramind].key);
        // Retrieve the first component from the key string
        char* compstr = NULL;
        pv_mime_string_extract_type(0, aParameters[paramind].key, compstr);

        if (compcount < 2)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVAuthorEngine::DoCapConfigVerifyParameters() Unsupported key"));
            return PVMFErrArgument;
        }

        // Retrieve the second component from the key string
        pv_mime_string_extract_type(0, aParameters[paramind].key, compstr);

        // First check if it is key string for engine ("x-pvmf/author")
        if (pv_mime_strcmp(compstr, _STRLIT_CHAR("x-pvmf/author")) >= 0)
        {
            if (3 == compcount)
            {

                // Verify the passed-in author setting
                PVMFStatus retval = DoVerifyAndSetAuthorParameter(aParameters[paramind], false);
                if (retval != PVMFSuccess)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVAuthorEngine::DoCapConfigVerifyParameters() Verifying parameter %d failed", paramind));
                    return retval;
                }
            }
            else if (4 == compcount)
            {
                // Only product info keys have four components
                // Verify the passed-in product info setting
                PVMFStatus retval = DoVerifyAndSetAuthorProductInfoParameter(aParameters[paramind], false);
                if (retval != PVMFSuccess)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVAuthorEngine::DoCapConfigVerifyParameters() Verifying parameter %d failed", paramind));
                    return retval;
                }
            }
            else
            {
                // Do not support more than 4 components right now
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVAuthorEngine::DoCapConfigVerifyParameters() Unsupported key"));
                return PVMFErrArgument;
            }
        }
        else
        {
            // Determine which node's cap-config IF needs to be used
            Oscl_Vector<PVInterface*, OsclMemAllocator> nodecapconfigif;
            PVMFStatus retval = DoQueryNodeCapConfig(compstr, nodecapconfigif);
            if (retval == PVMFSuccess && !(nodecapconfigif.empty()))
            {
                uint32 nodeind = 0;
                retval = PVMFErrArgument;
                // Go through each returned node's cap-config until successful
                while (nodeind < nodecapconfigif.size() && retval != PVMFSuccess)
                {
                    retval =
                        ((PvmiCapabilityAndConfig*)nodecapconfigif[nodeind])->verifyParametersSync(NULL, &aParameters[paramind], 1);
                    ++nodeind;
                }

                if (retval != PVMFSuccess)
                {
                    return retval;
                }
            }
            else
            {
                // Unknown key string
                return PVMFErrArgument;
            }
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVAuthorEngine::DoCapConfigVerifyParameters() Out"));
    return PVMFSuccess;
}


void PVAuthorEngine::createContext(PvmiMIOSession aSession, PvmiCapabilityContext& aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVAuthorEngine::createContext()"));
    OSCL_UNUSED_ARG(aSession);
    // Context is not really supported so just return some member variable pointer
    aContext = (PvmiCapabilityContext) & iCapConfigContext;
}


void PVAuthorEngine::setContextParameters(PvmiMIOSession aSession, PvmiCapabilityContext& aContext, PvmiKvp* aParameters, int aNumParamElements)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVAuthorEngine::setContextParameters()"));
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aContext);
    OSCL_UNUSED_ARG(aParameters);
    OSCL_UNUSED_ARG(aNumParamElements);
    // This method is not supported so leave
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVAuthorEngine::setContextParameters() is not supported!"));
    OSCL_LEAVE(PVMFErrNotSupported);
}


void PVAuthorEngine::DeleteContext(PvmiMIOSession aSession, PvmiCapabilityContext& aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVAuthorEngine::DeleteContext()"));
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aContext);
    // Do nothing since the context is just the a member variable of the engine
}

void PVAuthorEngine::addRef()
{
}

void PVAuthorEngine::removeRef()
{
}

bool PVAuthorEngine::queryInterface(const PVUuid& uuid, PVInterface*& iface)
{
    if (uuid == PVMI_CAPABILITY_AND_CONFIG_PVUUID)
    {
        PvmiCapabilityAndConfig* capconfigiface = OSCL_STATIC_CAST(PvmiCapabilityAndConfig*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, capconfigiface);
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVAuthorEngine::queryInterface() Unsupported interface UUID."));
        return false;
    }

    return true;
}
PVMFStatus PVAuthorEngine::DoCapConfigGetParametersSync(PvmiKeyType aIdentifier, PvmiKvp*& aParameters, int& aNumParamElements, PvmiCapabilityContext aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVAuthorEngine::DoCapConfigGetParametersSync() In"));
    OSCL_UNUSED_ARG(aContext);

    // Initialize the output parameters
    aNumParamElements = 0;
    aParameters = NULL;

    // Count the number of components and parameters in the key
    int compcount = pv_mime_string_compcnt(aIdentifier);
    // Retrieve the first component from the key string
    char* compstr = NULL;
    pv_mime_string_extract_type(0, aIdentifier, compstr);

    if (compcount < 2)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVAuthorEngine::DoCapConfigGetParametersSync() Invalid key string"));
        return PVMFErrArgument;
    }

    // Retrieve the second component from the key string
    pv_mime_string_extract_type(0, aIdentifier, compstr);

    // First check if it is key string for engine ("x-pvmf/author")
    if (pv_mime_strcmp(compstr, _STRLIT_CHAR("x-pvmf/author")) >= 0)
    {
        // Key is for author
        if (2 == compcount)
        {
            // Since key is "x-pvmf/author" return all
            // nodes available at this level. Ignore attribute
            // since capability is only allowed

            // Allocate memory for the KVP list
            aParameters = (PvmiKvp*)oscl_malloc(PVAUTHORCONFIG_BASE_NUMKEYS * sizeof(PvmiKvp));
            if (aParameters == NULL)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVAuthorEngine::DoCapConfigGetParametersSync() Memory allocation for KVP failed"));
                return PVMFErrNoMemory;
            }
            oscl_memset(aParameters, 0, PVAUTHORCONFIG_BASE_NUMKEYS*sizeof(PvmiKvp));
            // Allocate memory for the key strings in each KVP
            PvmiKeyType memblock = (PvmiKeyType)oscl_malloc(PVAUTHORCONFIG_BASE_NUMKEYS * PVAUTHORCONFIG_KEYSTRING_SIZE * sizeof(char));
            if (memblock == NULL)
            {
                oscl_free(aParameters);
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVAuthorEngine::DoCapConfigGetParametersSync() Memory allocation for key string failed"));
                return PVMFErrNoMemory;
            }
            oscl_strset(memblock, 0, PVAUTHORCONFIG_BASE_NUMKEYS*PVAUTHORCONFIG_KEYSTRING_SIZE*sizeof(char));
            // Assign the key string buffer to each KVP
            int32 jj;

            for (jj = 0; jj < PVAUTHORCONFIG_BASE_NUMKEYS; ++jj)
            {
                aParameters[jj].key = memblock + (jj * PVAUTHORCONFIG_KEYSTRING_SIZE);
            }
            // Copy the requested info
            for (jj = 0; jj < PVAUTHORCONFIG_BASE_NUMKEYS; ++jj)
            {
                oscl_strncat(aParameters[jj].key, _STRLIT_CHAR("x-pvmf/author/"), 14);
                oscl_strncat(aParameters[jj].key, PVAuthorConfigBaseKeys[jj].iString, oscl_strlen(PVAuthorConfigBaseKeys[jj].iString));
                oscl_strncat(aParameters[jj].key, _STRLIT_CHAR(";type="), 6);
                switch (PVAuthorConfigBaseKeys[jj].iType)
                {
                    case PVMI_KVPTYPE_AGGREGATE:
                        oscl_strncat(aParameters[jj].key, _STRLIT_CHAR(PVMI_KVPTYPE_AGGREGATE_STRING), oscl_strlen(PVMI_KVPTYPE_AGGREGATE_STRING));
                        break;

                    case PVMI_KVPTYPE_POINTER:
                        oscl_strncat(aParameters[jj].key, _STRLIT_CHAR(PVMI_KVPTYPE_POINTER_STRING), oscl_strlen(PVMI_KVPTYPE_POINTER_STRING));
                        break;

                    case PVMI_KVPTYPE_VALUE:
                    default:
                        oscl_strncat(aParameters[jj].key, _STRLIT_CHAR(PVMI_KVPTYPE_VALUE_STRING), oscl_strlen(PVMI_KVPTYPE_VALUE_STRING));
                        break;
                }
                oscl_strncat(aParameters[jj].key, _STRLIT_CHAR(";valtype="), 9);
                switch (PVAuthorConfigBaseKeys[jj].iValueType)
                {
                    case PVMI_KVPVALTYPE_RANGE_INT32:
                        oscl_strncat(aParameters[jj].key, _STRLIT_CHAR(PVMI_KVPVALTYPE_RANGE_INT32_STRING), oscl_strlen(PVMI_KVPVALTYPE_RANGE_INT32_STRING));
                        break;

                    case PVMI_KVPVALTYPE_KSV:
                        oscl_strncat(aParameters[jj].key, _STRLIT_CHAR(PVMI_KVPVALTYPE_KSV_STRING), oscl_strlen(PVMI_KVPVALTYPE_KSV_STRING));
                        break;

                    case PVMI_KVPVALTYPE_CHARPTR:
                        oscl_strncat(aParameters[jj].key, _STRLIT_CHAR(PVMI_KVPVALTYPE_CHARPTR_STRING), oscl_strlen(PVMI_KVPVALTYPE_CHARPTR_STRING));
                        break;

                    case PVMI_KVPVALTYPE_BOOL:
                        oscl_strncat(aParameters[jj].key, _STRLIT_CHAR(PVMI_KVPVALTYPE_BOOL_STRING), oscl_strlen(PVMI_KVPVALTYPE_BOOL_STRING));
                        break;

                    case PVMI_KVPVALTYPE_UINT32:
                    default:
                        oscl_strncat(aParameters[jj].key, _STRLIT_CHAR(PVMI_KVPVALTYPE_UINT32_STRING), oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING));
                        break;
                }
                aParameters[jj].key[PVAUTHORCONFIG_KEYSTRING_SIZE-1] = 0;
            }

            aNumParamElements = PVAUTHORCONFIG_BASE_NUMKEYS;
        }
        else
        {
            // Retrieve the third component from the key string
            pv_mime_string_extract_type(2, aIdentifier, compstr);

            for (int32 engcomp3ind = 0; engcomp3ind < PVAUTHORCONFIG_BASE_NUMKEYS; ++engcomp3ind)
            {
                // Go through each engine component string at 3rd level
                if (pv_mime_strcmp(compstr, (char*)(PVAuthorConfigBaseKeys[engcomp3ind].iString)) >= 0)
                {
                    if (engcomp3ind == 2) //Here '2' refers to the array index in PVAuthorConfigBaseKeys where productinfo is placed
                    {
                        // "x-pvmf/author/productinfo"
                        if (compcount == 3)
                        {
                            // Return list of product info. Ignore the
                            // attribute since capability is only allowed

                            // Allocate memory for the KVP list
                            aParameters = (PvmiKvp*)oscl_malloc(PVAUTHORCONFIG_PRODINFO_NUMKEYS * sizeof(PvmiKvp));
                            if (aParameters == NULL)
                            {
                                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVAuthorEngine::DoCapConfigGetParametersSync() Memory allocation for KVP failed"));
                                return PVMFErrNoMemory;
                            }
                            oscl_memset(aParameters, 0, PVAUTHORCONFIG_PRODINFO_NUMKEYS*sizeof(PvmiKvp));
                            // Allocate memory for the key strings in each KVP
                            PvmiKeyType memblock = (PvmiKeyType)oscl_malloc(PVAUTHORCONFIG_PRODINFO_NUMKEYS * PVAUTHORCONFIG_KEYSTRING_SIZE * sizeof(char));
                            if (memblock == NULL)
                            {
                                oscl_free(aParameters);
                                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVAuthorEngine::DoCapConfigGetParametersSync() Memory allocation for key string failed"));
                                return PVMFErrNoMemory;
                            }
                            oscl_strset(memblock, 0, PVAUTHORCONFIG_PRODINFO_NUMKEYS*PVAUTHORCONFIG_KEYSTRING_SIZE*sizeof(char));
                            // Assign the key string buffer to each KVP
                            int32 jj;
                            for (jj = 0; jj < PVAUTHORCONFIG_PRODINFO_NUMKEYS; ++jj)
                            {
                                aParameters[jj].key = memblock + (jj * PVAUTHORCONFIG_KEYSTRING_SIZE);
                            }
                            // Copy the requested info
                            for (jj = 0; jj < PVAUTHORCONFIG_PRODINFO_NUMKEYS; ++jj)
                            {
                                oscl_strncat(aParameters[jj].key, _STRLIT_CHAR("x-pvmf/author/productinfo/"), 26);
                                oscl_strncat(aParameters[jj].key, PVAuthorConfigProdInfoKeys[jj].iString, oscl_strlen(PVAuthorConfigProdInfoKeys[jj].iString));
                                oscl_strncat(aParameters[jj].key, _STRLIT_CHAR(";type=value;valtype=char*"), 25);
                                aParameters[jj].key[PVAUTHORCONFIG_KEYSTRING_SIZE-1] = 0;
                            }

                            aNumParamElements = PVAUTHORCONFIG_PRODINFO_NUMKEYS;
                        }
                        else if (compcount == 4)
                        {
                            // Retrieve the fourth component from the key string
                            pv_mime_string_extract_type(3, aIdentifier, compstr);

                            for (int32 engcomp4ind = 0; engcomp4ind < PVAUTHORCONFIG_PRODINFO_NUMKEYS; ++engcomp4ind)
                            {
                                if (pv_mime_strcmp(compstr, (char*)(PVAuthorConfigProdInfoKeys[engcomp4ind].iString)) >= 0)
                                {
                                    // Determine what is requested
                                    PvmiKvpAttr reqattr = GetAttrTypeFromKeyString(aIdentifier);
                                    if (reqattr == PVMI_KVPATTR_UNKNOWN)
                                    {
                                        // Default is current setting
                                        reqattr = PVMI_KVPATTR_CUR;
                                    }

                                    // Return the requested info
                                    PVMFStatus retval = DoGetAuthorProductInfoParameter(aParameters, aNumParamElements, engcomp4ind, reqattr);
                                    if (retval != PVMFSuccess)
                                    {
                                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVAuthorEngine::DoCapConfigGetParametersSync() Retrieving product info failed"));
                                        return retval;
                                    }

                                    // Break out of the for(engcomp4ind) loop
                                    break;
                                }
                            }
                        }
                        else
                        {
                            // Right now engine doesn't support more than 4 components
                            // so error out
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVAuthorEngine::DoCapConfigGetParametersSync() Unsupported key"));
                            return PVMFErrArgument;
                        }
                    }
                    else
                    {
                        if (compcount == 3)
                        {
                            // Determine what is requested
                            PvmiKvpAttr reqattr = GetAttrTypeFromKeyString(aIdentifier);
                            if (reqattr == PVMI_KVPATTR_UNKNOWN)
                            {
                                reqattr = PVMI_KVPATTR_CUR;
                            }

                            // Return the requested info
                            PVMFStatus retval = DoGetAuthorParameter(aParameters, aNumParamElements, engcomp3ind, reqattr);
                            if (retval != PVMFSuccess)
                            {
                                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVAuthorEngine::DoCapConfigGetParametersSync() Retrieving author parameter failed"));
                                return retval;
                            }
                        }
                        else
                        {
                            // Right now engine doesn't support more than 3 components
                            // for this sub-key string so error out
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVAuthorEngine::DoCapConfigGetParametersSync() Unsupported key"));
                            return PVMFErrArgument;
                        }
                    }

                    // Breakout of the for(engcomp3ind) loop
                    break;
                }
            }
        }
    }
    else
    {
        // Determine which node's cap-config IF needs to be used
        Oscl_Vector<PVInterface*, OsclMemAllocator> nodecapconfigif;
        PVMFStatus retval = DoQueryNodeCapConfig(compstr, nodecapconfigif);
        if (PVMFSuccess == retval && !(nodecapconfigif.empty()))
        {
            uint32 nodeind = 0;
            // Go through each returned node's cap-config until successful
            while (nodeind < nodecapconfigif.size() && 0 == aNumParamElements)
            {
                retval =
                    ((PvmiCapabilityAndConfig*)nodecapconfigif[nodeind])->getParametersSync(NULL, aIdentifier, aParameters, aNumParamElements, aContext);
                ++nodeind;
            }
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVAuthorEngine::DoCapConfigGetParametersSync() Out"));
    if (0 == aNumParamElements)
    {
        // If no one could get the parameter, return error
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVAuthorEngine::DoCapConfigGetParametersSync() Unsupported key"));
        return PVMFFailure;
    }
    return PVMFSuccess;
}



PVMFStatus PVAuthorEngine::DoCapConfigReleaseParameters(PvmiKvp* aParameters, int aNumElements)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVAuthorEngine::DoCapConfigReleaseParameters() In"));

    if (NULL == aParameters || aNumElements < 1)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVAuthorEngine::DoCapConfigReleaseParameters() KVP list is NULL or number of elements is 0"));
        return PVMFErrArgument;
    }

    // Count the number of components and parameters in the key
    int compcount = pv_mime_string_compcnt(aParameters[0].key);
    // Retrieve the first component from the key string
    char* compstr = NULL;
    pv_mime_string_extract_type(0, aParameters[0].key, compstr);

    if (compcount < 2)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVAuthorEngine::DoCapConfigReleaseParameters() Unsupported key"));
        return PVMFErrArgument;
    }

    // Retrieve the second component from the key string
    pv_mime_string_extract_type(0, aParameters[0].key, compstr);

    // Assume all the parameters come from the same component so the base components are the same
    // First check if it is key string for engine ("x-pvmf/author")
    if (pv_mime_strcmp(compstr, _STRLIT_CHAR("x-pvmf/author")) >= 0)
    {
        // Go through each KVP and release memory for value if allocated from heap
        for (int32 ii = 0; ii < aNumElements; ++ii)
        {
            // Next check if it is a value type that allocated memory
            PvmiKvpType kvptype = GetTypeFromKeyString(aParameters[ii].key);
            if (PVMI_KVPTYPE_VALUE == kvptype || PVMI_KVPTYPE_UNKNOWN == kvptype)
            {
                PvmiKvpValueType keyvaltype = GetValTypeFromKeyString(aParameters[ii].key);
                if (PVMI_KVPVALTYPE_UNKNOWN == keyvaltype)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVAuthorEngine::DoCapConfigReleaseParameters() Valtype not specified in key string"));
                    return PVMFErrArgument;
                }

                if (PVMI_KVPVALTYPE_CHARPTR == keyvaltype && NULL != aParameters[ii].value.pChar_value)
                {
                    oscl_free(aParameters[ii].value.pChar_value);
                    aParameters[ii].value.pChar_value = NULL;
                }
                else if (PVMI_KVPVALTYPE_KSV == keyvaltype && NULL != aParameters[ii].value.key_specific_value)
                {
                    oscl_free(aParameters[ii].value.key_specific_value);
                    aParameters[ii].value.key_specific_value = NULL;
                }
                else if (PVMI_KVPVALTYPE_RANGE_INT32 == keyvaltype && NULL != aParameters[ii].value.key_specific_value)
                {
                    range_int32* ri32 = (range_int32*)aParameters[ii].value.key_specific_value;
                    aParameters[ii].value.key_specific_value = NULL;
                    oscl_free(ri32);
                }
                else if (PVMI_KVPVALTYPE_RANGE_UINT32 == keyvaltype && NULL != aParameters[ii].value.key_specific_value)
                {
                    range_uint32* rui32 = (range_uint32*)aParameters[ii].value.key_specific_value;
                    aParameters[ii].value.key_specific_value = NULL;
                    oscl_free(rui32);
                }
                // @TODO Add more types if engine starts returning more types
            }
        }

        // Author engine allocated its key strings in one chunk so just free the first key string ptr
        oscl_free(aParameters[0].key);

        // Free memory for the parameter list
        oscl_free(aParameters);
        aParameters = NULL;
    }
    else
    {
        // Determine which node's cap-config IF needs to be used
        Oscl_Vector<PVInterface*, OsclMemAllocator> nodecapconfigif;
        PVMFStatus retval = DoQueryNodeCapConfig(compstr, nodecapconfigif);
        if (PVMFSuccess == retval && !(nodecapconfigif.empty()))
        {
            uint32 nodeind = 0;
            retval = PVMFErrArgument;
            // Go through each returned node's cap-config until successful
            while (nodeind < nodecapconfigif.size() && PVMFSuccess != retval)
            {
                retval =
                    ((PvmiCapabilityAndConfig*)nodecapconfigif[nodeind])->releaseParameters(NULL, aParameters, aNumElements);
                ++nodeind;
            }

            if (retval != PVMFSuccess)
            {
                return retval;
            }
        }
        else
        {
            // Unknown key string
            return PVMFErrArgument;
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVAuthorEngine::DoCapConfigReleaseParameters() Out"));
    return PVMFSuccess;
}

PVMFStatus PVAuthorEngine::DoVerifyAndSetAuthorParameter(PvmiKvp& aParameter, bool aSetParam)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVAuthorEngine::DoVerifyAndSetAuthorParameter() In"));

    // Determine the valtype
    PvmiKvpValueType keyvaltype = GetValTypeFromKeyString(aParameter.key);
    if (keyvaltype == PVMI_KVPVALTYPE_UNKNOWN)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVAuthorEngine::DoVerifyAndSetAuthorParameter() Valtype in key string unknown"));
        return PVMFErrArgument;
    }
    // Retrieve the third component from the key string
    char* compstr = NULL;
    pv_mime_string_extract_type(2, aParameter.key, compstr);

    int32 engcomp3ind = 0;
    for (engcomp3ind = 0; engcomp3ind < PVAUTHORCONFIG_BASE_NUMKEYS; ++engcomp3ind)
    {
        // Go through each engine component string at 3rd level
        if (pv_mime_strcmp(compstr, (char*)(PVAuthorConfigBaseKeys[engcomp3ind].iString)) >= 0)
        {
            // Break out of the for loop
            break;
        }
    }

    if (engcomp3ind >= PVAUTHORCONFIG_BASE_NUMKEYS || engcomp3ind == 2) //'2' refers to the place where productinfo is placed in PVAuthorConfigBaseKeys
    {
        // Match couldn't be found or non-leaf node ("productinfo") specified
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVAuthorEngine::DoVerifyAndSetAuthorParameter() Unsupported key or non-leaf node"));
        return PVMFErrArgument;
    }

    // Verify the valtype
    if (keyvaltype != PVAuthorConfigBaseKeys[engcomp3ind].iValueType)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVAuthorEngine::DoVerifyAndSetAuthorParameter() Valtype does not match for key"));
        return PVMFErrArgument;
    }

    switch (engcomp3ind)
    {
        case 0:
            // Change the config if to set
            if (aSetParam)
            {

            }
            break;

        case 1:
            // Change the config if to set
            if (aSetParam)
            {
            }
            break;

        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVAuthorEngine::DoVerifyAndSetAuthorParameter() Invalid index for author parameter"));
            return PVMFErrArgument;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVAuthorEngine::DoVerifyAndSetAuthorParameter() Out"));
    return PVMFSuccess;
}


PVMFStatus PVAuthorEngine::DoVerifyAndSetAuthorProductInfoParameter(PvmiKvp& aParameter, bool aSetParam)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVAuthorEngine::DoVerifyAndSetAuthorProductInfoParameter() In"));

    // Determine the valtype
    PvmiKvpValueType keyvaltype = GetValTypeFromKeyString(aParameter.key);
    if (keyvaltype == PVMI_KVPVALTYPE_UNKNOWN)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVAuthorEngine::DoVerifyAndSetAuthorProductInfoParameter() Valtype unknown"));
        return PVMFErrArgument;
    }
    // Retrieve the 4th component from the key string
    char* compstr = NULL;
    pv_mime_string_extract_type(3, aParameter.key, compstr);

    int32 engcomp4ind = 0;
    for (engcomp4ind = 0; engcomp4ind < PVAUTHORCONFIG_PRODINFO_NUMKEYS; ++engcomp4ind)
    {
        // Go through each engine component string at 4th level
        if (pv_mime_strcmp(compstr, (char*)(PVAuthorConfigProdInfoKeys[engcomp4ind].iString)) >= 0)
        {
            // Break out of the for loop
            break;
        }
    }

    if (engcomp4ind >= PVAUTHORCONFIG_PRODINFO_NUMKEYS)
    {
        // Match couldn't be found
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVAuthorEngine::DoVerifyAndSetAuthorProductInfoParameter() Unsupported key"));
        return PVMFErrArgument;
    }

    // Verify the valtype
    if (keyvaltype != PVAuthorConfigProdInfoKeys[engcomp4ind].iValueType)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVAuthorEngine::DoVerifyAndSetAuthorProductInfoParameter() Valtype does not match for key"));
        return PVMFErrArgument;
    }

    switch (engcomp4ind)
    {
        case 0:
            // Change the config if to set
            if (aSetParam)
            {
            }
            break;

        case 1:
            // Change the config if to set
            if (aSetParam)
            {
            }
            break;

        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVAuthorEngine::DoVerifyAndSetAuthorProductInfoParameter() Invalid index for product info"));
            return PVMFErrArgument;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVAuthorEngine::DoVerifyAndSetAuthorProductInfoParameter() Out"));
    return PVMFSuccess;
}

PVMFStatus PVAuthorEngine::DoGetAuthorParameter(PvmiKvp*& aParameters, int& aNumParamElements, int32 aIndex, PvmiKvpAttr reqattr)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVAuthorEngine::DoGetAuthorParameter() In"));

    aNumParamElements = 0;

    // Allocate memory for the KVP
    aParameters = (PvmiKvp*)oscl_malloc(sizeof(PvmiKvp));
    if (aParameters == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVAuthorEngine::DoGetAuthorParameter() Memory allocation for KVP failed"));
        return PVMFErrNoMemory;
    }
    oscl_memset(aParameters, 0, sizeof(PvmiKvp));
    // Allocate memory for the key string in KVP
    PvmiKeyType memblock = (PvmiKeyType)oscl_malloc(PVAUTHORCONFIG_KEYSTRING_SIZE * sizeof(char));
    if (memblock == NULL)
    {
        oscl_free(aParameters);
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVAuthorEngine::DoGetAuthorParameter() Memory allocation for key string failed"));
        return PVMFErrNoMemory;
    }
    oscl_strset(memblock, 0, PVAUTHORCONFIG_KEYSTRING_SIZE*sizeof(char));
    // Assign the key string buffer to KVP
    aParameters[0].key = memblock;

    // Copy the key string
    oscl_strncat(aParameters[0].key, _STRLIT_CHAR("x-pvmf/author/"), 14);
    oscl_strncat(aParameters[0].key, PVAuthorConfigBaseKeys[aIndex].iString, oscl_strlen(PVAuthorConfigBaseKeys[aIndex].iString));
    oscl_strncat(aParameters[0].key, _STRLIT_CHAR(";type=value;valtype="), 20);
    switch (PVAuthorConfigBaseKeys[aIndex].iValueType)
    {
        case PVMI_KVPVALTYPE_RANGE_INT32:
            oscl_strncat(aParameters[0].key, _STRLIT_CHAR(PVMI_KVPVALTYPE_RANGE_INT32_STRING), oscl_strlen(PVMI_KVPVALTYPE_RANGE_INT32_STRING));
            break;

        case PVMI_KVPVALTYPE_KSV:
            oscl_strncat(aParameters[0].key, _STRLIT_CHAR(PVMI_KVPVALTYPE_KSV_STRING), oscl_strlen(PVMI_KVPVALTYPE_KSV_STRING));
            break;

        case PVMI_KVPVALTYPE_CHARPTR:
            oscl_strncat(aParameters[0].key, _STRLIT_CHAR(PVMI_KVPVALTYPE_CHARPTR_STRING), oscl_strlen(PVMI_KVPVALTYPE_CHARPTR_STRING));
            break;

        case PVMI_KVPVALTYPE_BOOL:
            oscl_strncat(aParameters[0].key, _STRLIT_CHAR(PVMI_KVPVALTYPE_BOOL_STRING), oscl_strlen(PVMI_KVPVALTYPE_BOOL_STRING));
            break;

        case PVMI_KVPVALTYPE_UINT32:
        default:
            if (reqattr == PVMI_KVPATTR_CAP)
            {
                oscl_strncat(aParameters[0].key, _STRLIT_CHAR(PVMI_KVPVALTYPE_RANGE_UINT32_STRING), oscl_strlen(PVMI_KVPVALTYPE_RANGE_UINT32_STRING));
            }
            else
            {
                oscl_strncat(aParameters[0].key, _STRLIT_CHAR(PVMI_KVPVALTYPE_UINT32_STRING), oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING));
            }
            break;
    }
    aParameters[0].key[PVAUTHORCONFIG_KEYSTRING_SIZE-1] = 0;

    // Copy the requested info
    switch (aIndex)
    {
        case 0:
            if (reqattr == PVMI_KVPATTR_CUR)
            {
                // Return current value
                // Allocate memory for the string
            }
            else if (reqattr == PVMI_KVPATTR_DEF)
            {
                // Return default
                // Allocate memory for the string
            }
            else
            {
                // Return capability
                // Allocate memory for the string
            }
            break;

        case 1:
            if (reqattr == PVMI_KVPATTR_CUR)
            {
                // Return current value
            }
            else if (reqattr == PVMI_KVPATTR_DEF)
            {
                // Return default
            }
            else
            {
                // Return capability
            }
            break;

        default:
            // Invalid index
            oscl_free(aParameters[0].key);
            oscl_free(aParameters);
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVAuthorEngine::DoGetAuthorParameter() Invalid index to author parameter"));
            return PVMFErrArgument;
    }

    aNumParamElements = 1;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVAuthorEngine::DoGetAuthorParameter() Out"));
    return PVMFSuccess;
}

PVMFStatus PVAuthorEngine::DoGetAuthorProductInfoParameter(PvmiKvp*& aParameters, int& aNumParamElements, int32 aIndex, PvmiKvpAttr reqattr)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVAuthorEngine::DoGetAuthorProductInfoParameter() In"));

    aNumParamElements = 0;

    // Allocate memory for the KVP
    aParameters = (PvmiKvp*)oscl_malloc(sizeof(PvmiKvp));
    if (aParameters == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVAuthorEngine::DoGetAuthorProductInfoParameter() Memory allocation for KVP failed"));
        return PVMFErrNoMemory;
    }
    oscl_memset(aParameters, 0, sizeof(PvmiKvp));
    // Allocate memory for the key string in KVP
    PvmiKeyType memblock = (PvmiKeyType)oscl_malloc(PVAUTHORCONFIG_KEYSTRING_SIZE * sizeof(char));
    if (memblock == NULL)
    {
        oscl_free(aParameters);
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVAuthorEngine::DoGetAuthorProductInfoParameter() Memory allocation for key string"));
        return PVMFErrNoMemory;
    }
    oscl_strset(memblock, 0, PVAUTHORCONFIG_KEYSTRING_SIZE*sizeof(char));
    // Assign the key string buffer to KVP
    aParameters[0].key = memblock;

    // Copy the key string
    oscl_strncat(aParameters[0].key, _STRLIT_CHAR("x-pvmf/author/productinfo/"), 26);
    oscl_strncat(aParameters[0].key, PVAuthorConfigProdInfoKeys[aIndex].iString, oscl_strlen(PVAuthorConfigProdInfoKeys[aIndex].iString));
    oscl_strncat(aParameters[0].key, _STRLIT_CHAR(";type=value;valtype=char*"), 25);
    aParameters[0].key[PVAUTHORCONFIG_KEYSTRING_SIZE-1] = 0;

    // Copy the requested info
    switch (aIndex)
    {
        case 0:
            if (reqattr == PVMI_KVPATTR_CUR)
            {
                // Return current value
                // Allocate memory for the string
            }
            else if (reqattr == PVMI_KVPATTR_DEF)
            {
                // Return default
                // Allocate memory for the string
            }
            else
            {
                // Return capability
            }
            break;

        case 1:
            if (reqattr == PVMI_KVPATTR_CUR)
            {
                // Return current value
                // Allocate memory for the string
            }
            else if (reqattr == PVMI_KVPATTR_DEF)
            {
                // Return default
                // Allocate memory for the string
            }
            else
            {
                // Return capability
            }
            break;
        default:
            // Invalid index
            oscl_free(aParameters[0].key);
            oscl_free(aParameters);
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVAuthorEngine::DoGetAuthorProductInfoParameter() Invalid index for product info"));
            return PVMFErrArgument;
    }

    aNumParamElements = 1;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVAuthorEngine::DoGetAuthorProductInfoParameter() Out"));
    return PVMFSuccess;
}


OSCL_EXPORT_REF
void
PVAuthorEngineInterface::GetSDKInfo
(
    PVSDKInfo& aSdkInfo
)
{
    aSdkInfo.iLabel = PVAUTHOR_ENGINE_SDKINFO_LABEL;
    aSdkInfo.iDate  = PVAUTHOR_ENGINE_SDKINFO_DATE;
}

