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
#include "pv_player_engine.h"

#include "pv_player_config.h"

#ifndef USE_CML2_CONFIG
#include "pv_player_engine_tunables.h"
#endif

#include "pv_player_sdkinfo.h"

#include "pvmf_node_interface.h"

#include "pvmf_ffparsernode_extension.h"

#include "pvmf_data_source_init_extension.h"

#include "pvmf_track_selection_extension.h"

#include "pvmf_data_source_playback_control.h"

#include "pvmf_data_source_direction_control.h"

#include "pvmf_track_level_info_extension.h"

#include "pvmf_fileoutput_factory.h"

#include "pvmf_fileoutput_config.h"

#include "pvmf_nodes_sync_control.h"

#include "pvlogger.h"

#include "oscl_error_codes.h"

#include "pvmf_basic_errorinfomessage.h"

#include "pvmf_duration_infomessage.h"

#include "pvmf_metadata_infomessage.h"

#include "pv_mime_string_utils.h"

#include "pvmi_kvp_util.h"

#include "oscl_string_utils.h"

#include "media_clock_converter.h"

#include "time_comparison_utils.h"

#include "pvmf_local_data_source.h"

#include "pvmf_cpmplugin_license_interface.h"

#include "oscl_registry_access_client.h"

#include "pvmf_source_context_data.h"

#include "pv_player_node_registry.h"
#include "pv_player_registry_interface.h"

// For recognizer registry
#include "pvmf_recognizer_registry.h"

#include "pvmi_datastreamsyncinterface_ref_factory.h"

#include "pvmf_recognizer_plugin.h"

//


#define PVPLAYERENGINE_NUM_COMMANDS 10

#define PVPLAYERENGINE_TIMERID_ENDTIMECHECK 1

#define PVP_MIN_PLAYSTATUS_PERCENT_OVERFLOW_THRESHOLD 1000



PVPlayerEngine* PVPlayerEngine::New(PVCommandStatusObserver* aCmdStatusObserver,
                                    PVErrorEventObserver *aErrorEventObserver,
                                    PVInformationalEventObserver *aInfoEventObserver)
{
    PVPlayerEngine* engine = NULL;
    engine = OSCL_NEW(PVPlayerEngine, ());
    if (engine)
    {
        engine->Construct(aCmdStatusObserver,
                          aErrorEventObserver,
                          aInfoEventObserver);
    }

    return engine;
}


PVPlayerEngine::~PVPlayerEngine()
{
    Cancel();

    // Clear the Track selection List
    iTrackSelectionList.clear();

    // Remove Stored KVP Values
    DeleteKVPValues();

    if (!iPendingCmds.empty())
    {
        iPendingCmds.pop();
    }

    // Clean up the datapaths
    for (uint32 i = 0; i < iDatapathList.size(); ++i)
    {
        DoEngineDatapathCleanup(iDatapathList[i]);
    }
    iDatapathList.clear();

    // Clean up the source node
    DoSourceNodeCleanup();

    // Shutdown and destroy the timer
    if (iPollingCheckTimer)
    {
        iPollingCheckTimer->Clear();
    }

    if (iWatchDogTimer)
    {
        iWatchDogTimer->Cancel();
        OSCL_DELETE(iWatchDogTimer);
    }

    OSCL_DELETE(iPollingCheckTimer);

    //Destroy media clock notifications interface
    if (iClockNotificationsInf != NULL)
    {
        iPlaybackClock.DestroyMediaClockNotificationsInterface(iClockNotificationsInf);
        iClockNotificationsInf = NULL;
    }

    // Return all engine contexts to pool
    while (!iCurrentContextList.empty())
    {
        FreeEngineContext(iCurrentContextList[0]);
    }

    PVPlayerRegistryPopulator::Depopulate(iPlayerNodeRegistry, iPlayerRecognizerRegistry);

    iNodeUuids.clear();

    iCommandIdMut.Close();
    iOOTSyncCommandSem.Close();
}


PVCommandId PVPlayerEngine::GetSDKInfo(PVSDKInfo &aSDKInfo, const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::GetSDKInfo()"));
    Oscl_Vector<PVPlayerEngineCommandParamUnion, OsclMemAllocator> paramvec;
    paramvec.reserve(1);
    paramvec.clear();
    PVPlayerEngineCommandParamUnion param;
    param.pOsclAny_value = (OsclAny*) & aSDKInfo;
    paramvec.push_back(param);
    return AddCommandToQueue(PVP_ENGINE_COMMAND_GET_SDK_INFO, (OsclAny*)aContextData, &paramvec);
}


PVCommandId PVPlayerEngine::GetSDKModuleInfo(PVSDKModuleInfo &aSDKModuleInfo, const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::GetSDKModuleInfo()"));
    Oscl_Vector<PVPlayerEngineCommandParamUnion, OsclMemAllocator> paramvec;
    paramvec.reserve(1);
    paramvec.clear();
    PVPlayerEngineCommandParamUnion param;
    param.pOsclAny_value = (OsclAny*) & aSDKModuleInfo;
    paramvec.push_back(param);
    return AddCommandToQueue(PVP_ENGINE_COMMAND_GET_SDK_MODULE_INFO, (OsclAny*)aContextData, &paramvec);
}


PVCommandId PVPlayerEngine::SetLogAppender(const char* aTag, OsclSharedPtr<PVLoggerAppender>& aAppender, const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::SetLogAppender()"));
    Oscl_Vector<PVPlayerEngineCommandParamUnion, OsclMemAllocator> paramvec;
    paramvec.reserve(2);
    paramvec.clear();
    PVPlayerEngineCommandParamUnion param;
    param.pChar_value = (char*)aTag;
    paramvec.push_back(param);
    param.pOsclAny_value = (OsclAny*) & aAppender;
    paramvec.push_back(param);
    return AddCommandToQueue(PVP_ENGINE_COMMAND_SET_LOG_APPENDER, (OsclAny*)aContextData, &paramvec);
}


PVCommandId PVPlayerEngine::RemoveLogAppender(const char* aTag, OsclSharedPtr<PVLoggerAppender>& aAppender, const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::RemoveLogAppender()"));
    Oscl_Vector<PVPlayerEngineCommandParamUnion, OsclMemAllocator> paramvec;
    paramvec.reserve(2);
    paramvec.clear();
    PVPlayerEngineCommandParamUnion param;
    param.pChar_value = (char*)aTag;
    paramvec.push_back(param);
    param.pOsclAny_value = (OsclAny*) & aAppender;
    paramvec.push_back(param);
    return AddCommandToQueue(PVP_ENGINE_COMMAND_REMOVE_LOG_APPENDER, (OsclAny*)aContextData, &paramvec);
}


PVCommandId PVPlayerEngine::SetLogLevel(const char* aTag, int32 aLevel, bool aSetSubtree, const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::SetLogLevel()"));
    Oscl_Vector<PVPlayerEngineCommandParamUnion, OsclMemAllocator> paramvec;
    paramvec.reserve(3);
    paramvec.clear();
    PVPlayerEngineCommandParamUnion param;
    param.pChar_value = (char*)aTag;
    paramvec.push_back(param);
    param.int32_value = aLevel;
    paramvec.push_back(param);
    param.bool_value = aSetSubtree;
    paramvec.push_back(param);
    return AddCommandToQueue(PVP_ENGINE_COMMAND_SET_LOG_LEVEL, (OsclAny*)aContextData, &paramvec);
}


PVCommandId PVPlayerEngine::GetLogLevel(const char* aTag, PVLogLevelInfo& aLogInfo, const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::GetLogLevel()"));
    Oscl_Vector<PVPlayerEngineCommandParamUnion, OsclMemAllocator> paramvec;
    paramvec.reserve(2);
    paramvec.clear();
    PVPlayerEngineCommandParamUnion param;
    param.pChar_value = (char*)aTag;
    paramvec.push_back(param);
    param.pOsclAny_value = (OsclAny*) & aLogInfo;
    paramvec.push_back(param);
    return AddCommandToQueue(PVP_ENGINE_COMMAND_GET_LOG_LEVEL, (OsclAny*)aContextData, &paramvec);
}


PVCommandId PVPlayerEngine::QueryUUID(const PvmfMimeString& aMimeType, Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids,
                                      bool aExactUuidsOnly, const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::QueryUUID()"));
    Oscl_Vector<PVPlayerEngineCommandParamUnion, OsclMemAllocator> paramvec;
    paramvec.reserve(3);
    paramvec.clear();
    PVPlayerEngineCommandParamUnion param;
    param.pOsclAny_value = (OsclAny*) & aMimeType;
    paramvec.push_back(param);
    param.pOsclAny_value = (OsclAny*) & aUuids;
    paramvec.push_back(param);
    param.bool_value = aExactUuidsOnly;
    paramvec.push_back(param);
    return AddCommandToQueue(PVP_ENGINE_COMMAND_QUERY_UUID, (OsclAny*)aContextData, &paramvec);
}


PVCommandId PVPlayerEngine::QueryInterface(const PVUuid& aUuid, PVInterface*& aInterfacePtr, const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::QueryInterface()"));
    Oscl_Vector<PVPlayerEngineCommandParamUnion, OsclMemAllocator> paramvec;
    paramvec.reserve(1);
    paramvec.clear();
    PVPlayerEngineCommandParamUnion param;
    param.pOsclAny_value = (OsclAny*) & aInterfacePtr;
    paramvec.push_back(param);
    return AddCommandToQueue(PVP_ENGINE_COMMAND_QUERY_INTERFACE, (OsclAny*)aContextData, &paramvec, &aUuid);
}


PVCommandId PVPlayerEngine::CancelCommand(PVCommandId aCancelCmdId, const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::CancelCommand()"));
    Oscl_Vector<PVPlayerEngineCommandParamUnion, OsclMemAllocator> paramvec;
    paramvec.reserve(1);
    paramvec.clear();
    PVPlayerEngineCommandParamUnion param;
    param.int32_value = aCancelCmdId;
    paramvec.push_back(param);
    return AddCommandToQueue(PVP_ENGINE_COMMAND_CANCEL_COMMAND, (OsclAny*)aContextData, &paramvec);
}


PVCommandId PVPlayerEngine::CancelAllCommands(const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::CancelAllCommands()"));
    return AddCommandToQueue(PVP_ENGINE_COMMAND_CANCEL_ALL_COMMANDS, (OsclAny*)aContextData);
}


PVCommandId PVPlayerEngine::GetPVPlayerState(PVPlayerState& aState, const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::GetPVPlayerState()"));
    Oscl_Vector<PVPlayerEngineCommandParamUnion, OsclMemAllocator> paramvec;
    paramvec.reserve(1);
    paramvec.clear();
    PVPlayerEngineCommandParamUnion param;
    param.pOsclAny_value = (OsclAny*) & aState;
    paramvec.push_back(param);
    return AddCommandToQueue(PVP_ENGINE_COMMAND_GET_PVPLAYER_STATE, (OsclAny*)aContextData, &paramvec);
}


PVMFStatus PVPlayerEngine::GetPVPlayerStateSync(PVPlayerState& aState)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::GetPVPlayerStateSync()"));
    Oscl_Vector<PVPlayerEngineCommandParamUnion, OsclMemAllocator> paramvec;
    paramvec.reserve(1);
    paramvec.clear();
    PVPlayerEngineCommandParamUnion param;
    param.pOsclAny_value = (OsclAny*) & aState;
    paramvec.push_back(param);
    if (iThreadSafeQueue.IsInThread())
    {
        PVPlayerEngineCommand cmd(PVP_ENGINE_COMMAND_GET_PVPLAYER_STATE, -1, NULL, &paramvec);
        return DoGetPVPlayerState(cmd, true);
    }
    else
    {
        return DoOOTSyncCommand(PVP_ENGINE_COMMAND_GET_PVPLAYER_STATE_OOTSYNC, &paramvec);
    }
}


PVCommandId PVPlayerEngine::AddDataSource(PVPlayerDataSource& aDataSource, const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::AddDataSource()"));
    Oscl_Vector<PVPlayerEngineCommandParamUnion, OsclMemAllocator> paramvec;
    paramvec.reserve(1);
    paramvec.clear();
    PVPlayerEngineCommandParamUnion param;
    param.pOsclAny_value = (OsclAny*) & aDataSource;
    paramvec.push_back(param);
    return AddCommandToQueue(PVP_ENGINE_COMMAND_ADD_DATA_SOURCE, (OsclAny*)aContextData, &paramvec);
}


PVCommandId PVPlayerEngine::Init(const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::Init()"));
    return AddCommandToQueue(PVP_ENGINE_COMMAND_INIT, (OsclAny*)aContextData);
}


PVCommandId PVPlayerEngine::GetMetadataKeys(PVPMetadataList& aKeyList, int32 aStartingIndex, int32 aMaxEntries,
        char* aQueryKey, const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::GetMetadataKeys()"));
    Oscl_Vector<PVPlayerEngineCommandParamUnion, OsclMemAllocator> paramvec;
    paramvec.reserve(4);
    paramvec.clear();
    PVPlayerEngineCommandParamUnion param;

    param.pOsclAny_value = (OsclAny*) & aKeyList;
    paramvec.push_back(param);
    param.int32_value = aStartingIndex;
    paramvec.push_back(param);
    param.int32_value = aMaxEntries;
    paramvec.push_back(param);
    param.pChar_value = aQueryKey;
    paramvec.push_back(param);

    return AddCommandToQueue(PVP_ENGINE_COMMAND_GET_METADATA_KEY, (OsclAny*)aContextData, &paramvec);
}

PVCommandId PVPlayerEngine::GetMetadataValues(PVPMetadataList& aKeyList, int32 aStartingValueIndex, int32 aMaxValueEntries, int32& aNumAvailableValueEntries,
        Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList, const OsclAny* aContextData, bool aMetadataValuesCopiedInCallBack)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::GetMetadataValues()"));
    Oscl_Vector<PVPlayerEngineCommandParamUnion, OsclMemAllocator> paramvec;
    paramvec.reserve(6);
    paramvec.clear();
    PVPlayerEngineCommandParamUnion param;

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
    param.bool_value = aMetadataValuesCopiedInCallBack;
    paramvec.push_back(param);

    return AddCommandToQueue(PVP_ENGINE_COMMAND_GET_METADATA_VALUE, (OsclAny*)aContextData, &paramvec);
}

PVCommandId PVPlayerEngine::ReleaseMetadataValues(Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList, const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::ReleaseMetadataValues()"));
    Oscl_Vector<PVPlayerEngineCommandParamUnion, OsclMemAllocator> paramvec;
    paramvec.reserve(1);
    paramvec.clear();
    PVPlayerEngineCommandParamUnion param;

    param.pOsclAny_value = (OsclAny*) & aValueList;
    paramvec.push_back(param);

    return AddCommandToQueue(PVP_ENGINE_COMMAND_RELEASE_METADATA_VALUE, (OsclAny*)aContextData, &paramvec);
}

PVCommandId PVPlayerEngine::AddDataSink(PVPlayerDataSink& aDataSink, const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::AddDataSink()"));
    Oscl_Vector<PVPlayerEngineCommandParamUnion, OsclMemAllocator> paramvec;
    paramvec.reserve(1);
    paramvec.clear();
    PVPlayerEngineCommandParamUnion param;
    param.pOsclAny_value = (OsclAny*) & aDataSink;
    paramvec.push_back(param);
    return AddCommandToQueue(PVP_ENGINE_COMMAND_ADD_DATA_SINK, (OsclAny*)aContextData, &paramvec);
}


PVCommandId PVPlayerEngine::SetPlaybackRange(PVPPlaybackPosition aBeginPos, PVPPlaybackPosition aEndPos, bool aQueueRange, const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::SetPlaybackRange()"));
    PVPPlaybackPosition curpos;
    curpos.iPosUnit = PVPPBPOSUNIT_MILLISEC;

    // Additional checks to ensure a valid mode is
    // used for the begin position.
    // Assumes: aBegin.iPosUnit is always valid (when applicable).

    // Check 1 || Check 2
    // Check 1: Is the begin position indeterminate?
    // Check 2: Is the position unit something other than playlist?
    // If so, set mode to NOW.
    if ((aBeginPos.iIndeterminate) || (aBeginPos.iPosUnit != PVPPBPOSUNIT_PLAYLIST))
    {
        aBeginPos.iMode = PVPPBPOS_MODE_NOW;
    }
    // Check 3: Is the position unit playlist and the mode something other than the three valid
    // modes? If so, set mode to NOW.
    else if (aBeginPos.iPosUnit == PVPPBPOSUNIT_PLAYLIST)
    {
        switch (aBeginPos.iMode)
        {
            case PVPPBPOS_MODE_NOW:
            case PVPPBPOS_MODE_END_OF_CURRENT_PLAY_ELEMENT:
            case PVPPBPOS_MODE_END_OF_CURRENT_PLAY_SESSION:
                break;
            case PVPPBPOS_MODE_UNKNOWN:
            default:
                aBeginPos.iMode = PVPPBPOS_MODE_NOW;
                break;
        }
    }
    iPlaybackPositionMode = aBeginPos.iMode;
    GetPlaybackClockPosition(curpos);
    Oscl_Vector<PVPlayerEngineCommandParamUnion, OsclMemAllocator> paramvec;
    paramvec.reserve(3);
    paramvec.clear();
    PVPlayerEngineCommandParamUnion param;
    param.playbackpos_value = aBeginPos;
    paramvec.push_back(param);
    param.playbackpos_value = aEndPos;
    paramvec.push_back(param);
    param.bool_value = aQueueRange;
    paramvec.push_back(param);
    if (!iOverflowFlag)
    {
        return AddCommandToQueue(PVP_ENGINE_COMMAND_SET_PLAYBACK_RANGE, (OsclAny*)aContextData, &paramvec);
    }
    else
    {
        return AddCommandToQueue(PVP_ENGINE_COMMAND_SET_PLAYBACK_RANGE, (OsclAny*)aContextData, &paramvec, NULL, false);
    }
}


PVCommandId PVPlayerEngine::GetPlaybackRange(PVPPlaybackPosition &aBeginPos, PVPPlaybackPosition &aEndPos, bool aQueued, const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::GetPlaybackRange()"));
    Oscl_Vector<PVPlayerEngineCommandParamUnion, OsclMemAllocator> paramvec;
    paramvec.reserve(3);
    paramvec.clear();
    PVPlayerEngineCommandParamUnion param;
    param.pPlaybackpos_value = &aBeginPos;
    paramvec.push_back(param);
    param.pPlaybackpos_value = &aEndPos;
    paramvec.push_back(param);
    param.bool_value = aQueued;
    paramvec.push_back(param);
    return AddCommandToQueue(PVP_ENGINE_COMMAND_GET_PLAYBACK_RANGE, (OsclAny*)aContextData, &paramvec);
}


PVCommandId PVPlayerEngine::GetCurrentPosition(PVPPlaybackPosition &aPos, const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::GetCurrentPosition()"));
    Oscl_Vector<PVPlayerEngineCommandParamUnion, OsclMemAllocator> paramvec;
    paramvec.clear();
    PVPlayerEngineCommandParamUnion param;
    param.pPlaybackpos_value = &aPos;
    paramvec.push_back(param);
    return AddCommandToQueue(PVP_ENGINE_COMMAND_GET_CURRENT_POSITION, (OsclAny*)aContextData, &paramvec);
}


PVMFStatus PVPlayerEngine::GetCurrentPositionSync(PVPPlaybackPosition &aPos)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::GetCurrentPositionSync()"));
    Oscl_Vector<PVPlayerEngineCommandParamUnion, OsclMemAllocator> paramvec;
    paramvec.reserve(1);
    paramvec.clear();
    PVPlayerEngineCommandParamUnion param;
    param.pPlaybackpos_value = &aPos;
    paramvec.push_back(param);
    if (iThreadSafeQueue.IsInThread())
    {
        PVPlayerEngineCommand cmd(PVP_ENGINE_COMMAND_GET_CURRENT_POSITION, -1, NULL, &paramvec);
        return DoGetCurrentPosition(cmd, true);
    }
    else
    {
        return DoOOTSyncCommand(PVP_ENGINE_COMMAND_GET_CURRENT_POSITION_OOTSYNC, &paramvec);
    }
}


PVCommandId PVPlayerEngine::SetPlaybackRate(int32 aRate, PVMFTimebase* aTimebase, const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::SetPlaybackRate()"));
    Oscl_Vector<PVPlayerEngineCommandParamUnion, OsclMemAllocator> paramvec;
    paramvec.reserve(2);
    paramvec.clear();
    PVPlayerEngineCommandParamUnion param;
    param.int32_value = aRate;
    paramvec.push_back(param);
    param.pOsclAny_value = (OsclAny*)aTimebase;
    paramvec.push_back(param);
    return AddCommandToQueue(PVP_ENGINE_COMMAND_SET_PLAYBACK_RATE, (OsclAny*)aContextData, &paramvec);
}


PVCommandId PVPlayerEngine::GetPlaybackRate(int32& aRate, PVMFTimebase*& aTimebase, const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::GetPlaybackRate()"));
    Oscl_Vector<PVPlayerEngineCommandParamUnion, OsclMemAllocator> paramvec;
    paramvec.reserve(2);
    paramvec.clear();
    PVPlayerEngineCommandParamUnion param;
    param.pInt32_value = &aRate;
    paramvec.push_back(param);
    param.pOsclAny_value = (OsclAny*) & aTimebase;
    paramvec.push_back(param);
    return AddCommandToQueue(PVP_ENGINE_COMMAND_GET_PLAYBACK_RATE, (OsclAny*)aContextData, &paramvec);
}


PVCommandId PVPlayerEngine::GetPlaybackMinMaxRate(int32& aMinRate, int32& aMaxRate, const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::GetPlaybackMinMaxRate()"));
    Oscl_Vector<PVPlayerEngineCommandParamUnion, OsclMemAllocator> paramvec;
    paramvec.reserve(2);
    paramvec.clear();
    PVPlayerEngineCommandParamUnion param;
    param.pInt32_value = &aMinRate;
    paramvec.push_back(param);
    param.pInt32_value = &aMaxRate;
    paramvec.push_back(param);
    return AddCommandToQueue(PVP_ENGINE_COMMAND_GET_PLAYBACK_MINMAX_RATE, (OsclAny*)aContextData, &paramvec);
}


PVCommandId PVPlayerEngine::Prepare(const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::Prepare()"));
    return AddCommandToQueue(PVP_ENGINE_COMMAND_PREPARE, (OsclAny*)aContextData);
}


PVCommandId PVPlayerEngine::Start(const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::Start() "));
    return AddCommandToQueue(PVP_ENGINE_COMMAND_START, (OsclAny*)aContextData);
}


PVCommandId PVPlayerEngine::Pause(const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::Pause()"));
    return AddCommandToQueue(PVP_ENGINE_COMMAND_PAUSE, (OsclAny*)aContextData);
}


PVCommandId PVPlayerEngine::Resume(const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::Resume()"));
    return AddCommandToQueue(PVP_ENGINE_COMMAND_RESUME, (OsclAny*)aContextData);
}


PVCommandId PVPlayerEngine::Stop(const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::Stop()"));
    return AddCommandToQueue(PVP_ENGINE_COMMAND_STOP, (OsclAny*)aContextData);
}


PVCommandId PVPlayerEngine::RemoveDataSink(PVPlayerDataSink& aDataSink, const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::RemoveDataSink()"));
    Oscl_Vector<PVPlayerEngineCommandParamUnion, OsclMemAllocator> paramvec;
    paramvec.reserve(1);
    paramvec.clear();
    PVPlayerEngineCommandParamUnion param;
    param.pOsclAny_value = (OsclAny*) & aDataSink;
    paramvec.push_back(param);
    return AddCommandToQueue(PVP_ENGINE_COMMAND_REMOVE_DATA_SINK, (OsclAny*)aContextData, &paramvec);
}


PVCommandId PVPlayerEngine::Reset(const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::Reset()"));
    return AddCommandToQueue(PVP_ENGINE_COMMAND_RESET, (OsclAny*)aContextData);
}


PVCommandId PVPlayerEngine::RemoveDataSource(PVPlayerDataSource& aDataSource, const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::RemoveDataSource()"));
    Oscl_Vector<PVPlayerEngineCommandParamUnion, OsclMemAllocator> paramvec;
    paramvec.reserve(1);
    paramvec.clear();
    PVPlayerEngineCommandParamUnion param;
    param.pOsclAny_value = (OsclAny*) & aDataSource;
    paramvec.push_back(param);
    return AddCommandToQueue(PVP_ENGINE_COMMAND_REMOVE_DATA_SOURCE, (OsclAny*)aContextData, &paramvec);
}


void PVPlayerEngine::setObserver(PvmiConfigAndCapabilityCmdObserver* aObserver)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::setObserver()"));

    if (iThreadSafeQueue.IsInThread())
    {
        iCfgCapCmdObserver = aObserver;
    }
    else
    {
        Oscl_Vector<PVPlayerEngineCommandParamUnion, OsclMemAllocator> paramvec;
        paramvec.reserve(2);
        paramvec.clear();
        PVPlayerEngineCommandParamUnion param;
        param.pOsclAny_value = aObserver;
        paramvec.push_back(param);
        DoOOTSyncCommand(PVP_ENGINE_COMMAND_CAPCONFIG_SET_OBSERVER_OOTSYNC, &paramvec);
    }
}

PVMFStatus PVPlayerEngine::DoSetObserverSync(PVPlayerEngineCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSetObserverSync() In"));

    iCfgCapCmdObserver = (PvmiConfigAndCapabilityCmdObserver*)(aCmd.GetParam(0).pOsclAny_value);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSetObserverSync() Out"));
    return PVMFSuccess;
}

PVMFStatus PVPlayerEngine::getParametersSync(PvmiMIOSession aSession, PvmiKeyType aIdentifier, PvmiKvp*& aParameters, int& aNumParamElements, PvmiCapabilityContext aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::getParametersSync()"));
    OSCL_UNUSED_ARG(aSession);

    if (iThreadSafeQueue.IsInThread())
    {
        return DoCapConfigGetParametersSync(aIdentifier, aParameters, aNumParamElements, aContext);
    }
    else
    {
        Oscl_Vector<PVPlayerEngineCommandParamUnion, OsclMemAllocator> paramvec;
        paramvec.reserve(5);
        paramvec.clear();
        PVPlayerEngineCommandParamUnion param;
        param.pOsclAny_value = &aIdentifier;
        paramvec.push_back(param);
        param.pOsclAny_value = &aParameters;
        paramvec.push_back(param);
        param.pOsclAny_value = &aNumParamElements;
        paramvec.push_back(param);
        param.pOsclAny_value = &aContext;
        paramvec.push_back(param);
        return DoOOTSyncCommand(PVP_ENGINE_COMMAND_CAPCONFIG_GET_PARAMETERS_OOTSYNC, &paramvec);
    }
}


PVMFStatus PVPlayerEngine::DoGetParametersSync(PVPlayerEngineCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoGetParametersSync() In"));

    PVMFStatus status = DoCapConfigGetParametersSync(
                            *((PvmiKeyType*)aCmd.GetParam(0).pOsclAny_value)
                            , *((PvmiKvp**)aCmd.GetParam(1).pOsclAny_value)
                            , *((int*)aCmd.GetParam(2).pOsclAny_value)
                            , *((PvmiCapabilityContext*)aCmd.GetParam(3).pOsclAny_value));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoGetParametersSync() Out"));
    return status;
}


PVMFStatus PVPlayerEngine::releaseParameters(PvmiMIOSession aSession, PvmiKvp* aParameters, int aNumElements)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::releaseParameters()"));
    OSCL_UNUSED_ARG(aSession);

    if (iThreadSafeQueue.IsInThread())
    {
        return DoCapConfigReleaseParameters(aParameters, aNumElements);
    }
    else
    {
        Oscl_Vector<PVPlayerEngineCommandParamUnion, OsclMemAllocator> paramvec;
        paramvec.reserve(3);
        paramvec.clear();
        PVPlayerEngineCommandParamUnion param;
        param.pOsclAny_value = aParameters;
        paramvec.push_back(param);
        param.int32_value = aNumElements;
        paramvec.push_back(param);
        return DoOOTSyncCommand(PVP_ENGINE_COMMAND_CAPCONFIG_RELEASE_PARAMETERS_OOTSYNC, &paramvec);
    }
}

PVMFStatus PVPlayerEngine::DoReleaseParametersSync(PVPlayerEngineCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoReleaseParametersSync() In"));

    PVMFStatus status = DoCapConfigReleaseParameters(
                            (PvmiKvp*)aCmd.GetParam(0).pOsclAny_value
                            , aCmd.GetParam(1).int32_value);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoReleaseParametersSync() Out"));
    return status;
}

void PVPlayerEngine::createContext(PvmiMIOSession aSession, PvmiCapabilityContext& aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::createContext()"));
    OSCL_UNUSED_ARG(aSession);
    // Context is not really supported so just return some member variable pointer
    aContext = (PvmiCapabilityContext) & iCapConfigContext;
}


void PVPlayerEngine::setContextParameters(PvmiMIOSession aSession, PvmiCapabilityContext& aContext, PvmiKvp* aParameters, int aNumParamElements)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::setContextParameters()"));
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aContext);
    OSCL_UNUSED_ARG(aParameters);
    OSCL_UNUSED_ARG(aNumParamElements);
    // This method is not supported so leave
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::setContextParameters() is not supported!"));
    OSCL_LEAVE(OsclErrNotSupported);
}


void PVPlayerEngine::DeleteContext(PvmiMIOSession aSession, PvmiCapabilityContext& aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DeleteContext()"));
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aContext);
    // Do nothing since the context is just the a member variable of the engine
}


void PVPlayerEngine::setParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters, int aNumElements, PvmiKvp* &aRetKVP)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::setParametersSync()"));
    OSCL_UNUSED_ARG(aSession);

    // Save the parameters in an engine command object
    Oscl_Vector<PVPlayerEngineCommandParamUnion, OsclMemAllocator> paramvec;
    paramvec.reserve(3);
    paramvec.clear();
    PVPlayerEngineCommandParamUnion param;
    param.pOsclAny_value = (OsclAny*)aParameters;
    paramvec.push_back(param);
    param.int32_value = (int32) aNumElements;
    paramvec.push_back(param);
    param.pOsclAny_value = (OsclAny*) & aRetKVP;
    paramvec.push_back(param);
    if (iThreadSafeQueue.IsInThread())
    {
        PVPlayerEngineCommand cmd(PVP_ENGINE_COMMAND_CAPCONFIG_SET_PARAMETERS, -1, NULL, &paramvec);

        // Complete the request synchronously
        DoCapConfigSetParameters(cmd, true);
    }
    else
    {
        DoOOTSyncCommand(PVP_ENGINE_COMMAND_CAPCONFIG_SET_PARAMETERS_OOTSYNC, &paramvec);
    }
}


PVMFCommandId PVPlayerEngine::setParametersAsync(PvmiMIOSession aSession, PvmiKvp* aParameters, int aNumElements, PvmiKvp*& aRetKVP, OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::setParametersAsync()"));
    OSCL_UNUSED_ARG(aSession);

    // Save the parameters in an engine command object
    Oscl_Vector<PVPlayerEngineCommandParamUnion, OsclMemAllocator> paramvec;
    paramvec.reserve(3);
    paramvec.clear();
    PVPlayerEngineCommandParamUnion param;
    param.pOsclAny_value = (OsclAny*)aParameters;
    paramvec.push_back(param);
    param.int32_value = (int32) aNumElements;
    paramvec.push_back(param);
    param.pOsclAny_value = (OsclAny*) & aRetKVP;
    paramvec.push_back(param);

    // Push it to command queue to be processed asynchronously
    return AddCommandToQueue(PVP_ENGINE_COMMAND_CAPCONFIG_SET_PARAMETERS, (OsclAny*)aContext, &paramvec, NULL, false);
}


uint32 PVPlayerEngine::getCapabilityMetric(PvmiMIOSession aSession)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::getCapabilityMetric()"));
    OSCL_UNUSED_ARG(aSession);
    // Not supported so return 0
    return 0;
}


PVMFStatus PVPlayerEngine::verifyParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters, int aNumElements)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::verifyParametersSync()"));
    OSCL_UNUSED_ARG(aSession);

    if (iThreadSafeQueue.IsInThread())
    {
        return DoCapConfigVerifyParameters(aParameters, aNumElements);
    }
    else
    {
        Oscl_Vector<PVPlayerEngineCommandParamUnion, OsclMemAllocator> paramvec;
        paramvec.reserve(3);
        paramvec.clear();
        PVPlayerEngineCommandParamUnion param;
        param.pOsclAny_value = aParameters;
        paramvec.push_back(param);
        param.int32_value = aNumElements;
        paramvec.push_back(param);
        return DoOOTSyncCommand(PVP_ENGINE_COMMAND_CAPCONFIG_VERIFY_PARAMETERS_OOTSYNC, &paramvec);
    }
}

PVMFStatus PVPlayerEngine::DoVerifyParametersSync(PVPlayerEngineCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoVerifyParametersSync() In"));

    PVMFStatus status = DoCapConfigVerifyParameters(
                            (PvmiKvp*)aCmd.GetParam(0).pOsclAny_value
                            , aCmd.GetParam(1).int32_value);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoVerifyParametersSync() Out"));
    return status;
}

PVMFCommandId PVPlayerEngine::AcquireLicense(OsclAny* aLicenseData, uint32 aDataSize, oscl_wchar* aContentName, int32 aTimeoutMsec, const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::AcquireLicense() wchar"));
    Oscl_Vector<PVPlayerEngineCommandParamUnion, OsclMemAllocator> paramvec;
    paramvec.reserve(3);
    paramvec.clear();
    PVPlayerEngineCommandParamUnion param;
    param.pOsclAny_value = aLicenseData;
    paramvec.push_back(param);
    param.uint32_value = aDataSize;
    paramvec.push_back(param);
    param.pWChar_value = aContentName;
    paramvec.push_back(param);
    param.int32_value = aTimeoutMsec;
    paramvec.push_back(param);
    return AddCommandToQueue(PVP_ENGINE_COMMAND_ACQUIRE_LICENSE_WCHAR, (OsclAny*)aContextData, &paramvec);
}


PVMFCommandId PVPlayerEngine::AcquireLicense(OsclAny* aLicenseData, uint32 aDataSize, char* aContentName, int32 aTimeoutMsec, const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::AcquireLicense() char"));
    Oscl_Vector<PVPlayerEngineCommandParamUnion, OsclMemAllocator> paramvec;
    paramvec.reserve(3);
    paramvec.clear();
    PVPlayerEngineCommandParamUnion param;
    param.pOsclAny_value = aLicenseData;
    paramvec.push_back(param);
    param.uint32_value = aDataSize;
    paramvec.push_back(param);
    param.pChar_value = aContentName;
    paramvec.push_back(param);
    param.int32_value = aTimeoutMsec;
    paramvec.push_back(param);
    return AddCommandToQueue(PVP_ENGINE_COMMAND_ACQUIRE_LICENSE_CHAR, (OsclAny*)aContextData, &paramvec);
}

PVMFCommandId PVPlayerEngine::CancelAcquireLicense(PVMFCommandId aCmdId, const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::CancelAcquireLicense()"));
    Oscl_Vector<PVPlayerEngineCommandParamUnion, OsclMemAllocator> paramvec;
    paramvec.reserve(1);
    paramvec.clear();
    PVPlayerEngineCommandParamUnion param;
    param.int32_value = aCmdId;
    paramvec.push_back(param);
    return AddCommandToQueue(PVP_ENGINE_COMMAND_CANCEL_ACQUIRE_LICENSE, (OsclAny*)aContextData, &paramvec);
}

PVMFStatus PVPlayerEngine::GetLicenseStatus(PVMFCPMLicenseStatus& aStatus)
{
    if (iThreadSafeQueue.IsInThread())
    {
        if (iSourceNodeCPMLicenseIF)
            return iSourceNodeCPMLicenseIF->GetLicenseStatus(aStatus);
        return PVMFFailure;
    }
    else
    {
        Oscl_Vector<PVPlayerEngineCommandParamUnion, OsclMemAllocator> paramvec;
        paramvec.reserve(2);
        paramvec.clear();
        PVPlayerEngineCommandParamUnion param;
        param.pOsclAny_value = &aStatus;
        paramvec.push_back(param);
        return DoOOTSyncCommand(PVP_ENGINE_COMMAND_GET_LICENSE_STATUS_OOTSYNC, &paramvec);
    }
}

PVMFStatus PVPlayerEngine::DoGetLicenseStatusSync(PVPlayerEngineCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoGetLicenseStatusSync() In"));

    PVMFStatus status;
    PVMFCPMLicenseStatus* licstatus = (PVMFCPMLicenseStatus*)(aCmd.GetParam(0).pOsclAny_value);
    if (!licstatus)
    {
        return PVMFFailure;
    }

    if (iSourceNodeCPMLicenseIF)
        status = iSourceNodeCPMLicenseIF->GetLicenseStatus(*licstatus);
    else
        status = PVMFFailure;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoGetLicenseStatusSync() Out"));
    return status;
}

void PVPlayerEngine::addRef()
{
}


void PVPlayerEngine::removeRef()
{
}


bool PVPlayerEngine::queryInterface(const PVUuid& uuid, PVInterface*& iface)
{
    if (uuid == PVMI_CAPABILITY_AND_CONFIG_PVUUID)
    {
        PvmiCapabilityAndConfig* capconfigiface = OSCL_STATIC_CAST(PvmiCapabilityAndConfig*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, capconfigiface);
    }
    else if (uuid == PVPlayerLicenseAcquisitionInterfaceUuid)
    {
        PVPlayerLicenseAcquisitionInterface* licacqiface = OSCL_STATIC_CAST(PVPlayerLicenseAcquisitionInterface*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, licacqiface);
    }
    // Check if track level info IF from source node was requested
    else if (uuid == PVMF_TRACK_LEVEL_INFO_INTERFACE_UUID && iSourceNodeTrackLevelInfoIF)
    {
        iface = OSCL_STATIC_CAST(PVInterface*, iSourceNodeTrackLevelInfoIF);
    }
    //Check if track selection IF from source node was requested
    else if (uuid == PVPlayerTrackSelectionInterfaceUuid)
    {
        PVPlayerTrackSelectionInterface* tseliface = OSCL_STATIC_CAST(PVPlayerTrackSelectionInterface*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, tseliface);
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::queryInterface() Unsupported interface UUID."));
        return false;
    }

    return true;
}



PVPlayerEngine::PVPlayerEngine() :
        OsclTimerObject(OsclActiveObject::EPriorityNominal, "PVPlayerEngine"),
        iCommandId(0),
        iState(PVP_ENGINE_STATE_IDLE),
        iCmdStatusObserver(NULL),
        iErrorEventObserver(NULL),
        iInfoEventObserver(NULL),
        iCfgCapCmdObserver(NULL),
        iPollingCheckTimer(NULL),
        iCommandCompleteStatusInErrorHandling(PVMFSuccess),
        iCommandCompleteErrMsgInErrorHandling(NULL),
        iCapConfigContext(0),
        iNumPendingNodeCmd(0),
        iNumPendingSkipCompleteEvent(0),
        iNumPendingDatapathCmd(0),
        iNumPVMFInfoStartOfDataPending(0),
        iDataSource(NULL),
        iSourceFormatType(PVMF_MIME_FORMAT_UNKNOWN),
        iSourceNode(NULL),
        iSourceNodeSessionId(0),
        iSourceNodeInitIF(NULL),
        iSourceNodeTrackSelIF(NULL),
        iSourceNodePBCtrlIF(NULL),
        iSourceNodeDirCtrlIF(NULL),
        iSourceNodeTrackLevelInfoIF(NULL),
        iSourceNodeMetadataExtIF(NULL),
        iSourceNodeCapConfigIF(NULL),
        iSourceNodeRegInitIF(NULL),
        iSourceNodeCPMLicenseIF(NULL),
        iSourceNodePVInterfaceInit(NULL),
        iSourceNodePVInterfaceTrackSel(NULL),
        iSourceNodePVInterfacePBCtrl(NULL),
        iSourceNodePVInterfaceDirCtrl(NULL),
        iSourceNodePVInterfaceTrackLevelInfo(NULL),
        iSourceNodePVInterfaceMetadataExt(NULL),
        iSourceNodePVInterfaceCapConfig(NULL),
        iSourceNodePVInterfaceRegInit(NULL),
        iSourceNodePVInterfaceCPMLicense(NULL),
        iCPMGetLicenseCmdId(0),
        iMetadataValuesCopiedInCallBack(true),
        iReleaseMetadataValuesPending(false),
        iCurrentContextListMemPool(12),
        iNumberCancelCmdPending(0),
        iLogger(NULL),
        iReposLogger(NULL),
        iPerfLogger(NULL),
        iClockNotificationsInf(NULL),
        iPlayStatusCallbackTimerID(0),
        iPlayStatusCallbackTimerMarginWindow(0),
        iCurrCallbackTimerLatency(0),
        iPlaybackClockRate(100000),
        iOutsideTimebase(NULL),
        iPlaybackClockRate_New(100000),
        iOutsideTimebase_New(NULL),
        iPlaybackDirection(1),
        iPlaybackDirection_New(1),
        iChangePlaybackDirectionWhenResuming(false),
        iEndTimeCheckEnabled(false),
        iQueuedRangePresent(false),
        iChangePlaybackPositionWhenResuming(false),
        iActualNPT(0),
        iTargetNPT(0),
        iActualMediaDataTS(0),
        iSkipMediaDataTS(0),
        iStartNPT(0),
        iStartMediaDataTS(0),
        iWatchDogTimerInterval(0),
        iSeekPointBeforeTargetNPT(0),
        iSeekPointAfterTargetNPT(0),
        iForwardReposFlag(false),
        iBackwardReposFlag(false),
        iPlayStatusTimerEnabled(false),
        iDataReadySent(false),
        iPlaybackPausedDueToEndOfClip(false),
        iSourceDurationAvailable(false),
        iSourceDurationInMS(0),
        iPBPosEnable(true),
        iPBPosStatusUnit(PVPLAYERENGINE_CONFIG_PBPOSSTATUSUNIT_DEF),
        iPBPosStatusInterval(PVPLAYERENGINE_CONFIG_PBPOSSTATUSINTERVAL_DEF),
        iEndTimeCheckInterval(PVPLAYERENGINE_CONFIG_ENDTIMECHECKINTERVAL_DEF),
        iSeekToSyncPoint(PVPLAYERENGINE_CONFIG_SEEKTOSYNCPOINT_DEF),
        iSkipToRequestedPosition(PVPLAYERENGINE_CONFIG_SKIPTOREQUESTEDPOS_DEF),
        iBackwardRepos(false),
        iSyncPointSeekWindow(PVPLAYERENGINE_CONFIG_SEEKTOSYNCPOINTWINDOW_DEF),
        iNodeCmdTimeout(PVPLAYERENGINE_CONFIG_NODECMDTIMEOUT_DEF),
        iNodeDataQueuingTimeout(PVPLAYERENGINE_CONFIG_NODEDATAQUEUINGTIMEOUT_DEF),
        iProdInfoProdName(_STRLIT_CHAR(PVPLAYERENGINE_PRODINFO_PRODNAME_STRING)),
        iProdInfoPartNum(_STRLIT_CHAR(PVPLAYERENGINE_PRODINFO_PARTNUM_STRING)),
        iProdInfoHWPlatform(_STRLIT_CHAR(PVPLAYERENGINE_PRODINFO_HWPLATFORM_STRING)),
        iProdInfoSWPlatform(_STRLIT_CHAR(PVPLAYERENGINE_PRODINFO_SWPLATFORM_STRING)),
        iProdInfoDevice(_STRLIT_CHAR(PVPLAYERENGINE_PRODINFO_DEVICE_STRING)),
        iStreamID(0),
        iAlternateSrcFormatIndex(0),
        iRollOverState(RollOverStateIdle),
        iTrackSelectionHelper(NULL),
        iPlaybackPositionMode(PVPPBPOS_MODE_UNKNOWN),
        iOverflowFlag(false)
{
    iCurrentBeginPosition.iIndeterminate = true;
    iCurrentEndPosition.iIndeterminate = true;
    iCurrentBeginPosition.iPlayListUri = NULL;
    iQueuedBeginPosition.iIndeterminate = true;
    iQueuedEndPosition.iIndeterminate = true;
    iChangeDirectionNPT.iIndeterminate = true;

    iSyncMarginVideo.min = PVPLAYERENGINE_CONFIG_SYNCMARGIN_EARLY_DEF;
    iSyncMarginVideo.max = PVPLAYERENGINE_CONFIG_SYNCMARGIN_LATE_DEF;
    iSyncMarginAudio.min = PVPLAYERENGINE_CONFIG_SYNCMARGIN_EARLY_DEF;
    iSyncMarginAudio.max = PVPLAYERENGINE_CONFIG_SYNCMARGIN_LATE_DEF;
    iSyncMarginText.min = PVPLAYERENGINE_CONFIG_SYNCMARGIN_EARLY_DEF;
    iSyncMarginText.max = PVPLAYERENGINE_CONFIG_SYNCMARGIN_LATE_DEF;

    iNodeUuids.clear();
}


void PVPlayerEngine::Construct(PVCommandStatusObserver* aCmdStatusObserver,
                               PVErrorEventObserver *aErrorEventObserver,
                               PVInformationalEventObserver *aInfoEventObserver)
{
    iCommandIdMut.Create();
    iOOTSyncCommandSem.Create();
    iThreadSafeQueue.Configure(this);

    iCmdStatusObserver = aCmdStatusObserver;
    iInfoEventObserver = aInfoEventObserver;
    iErrorEventObserver = aErrorEventObserver;

    // Allocate memory for vectors
    // If a leave occurs, let it bubble up
    iCurrentCmd.reserve(1);
    iCmdToCancel.reserve(1);
    iCmdToDlaCancel.reserve(1);
    iPendingCmds.reserve(PVPLAYERENGINE_NUM_COMMANDS);
    iPvmiKvpCapNConfig.reserve(20);

    iDatapathList.reserve(3);

    iCurrentContextList.reserve(12);

    iMetadataIFList.reserve(6);
    iMetadataIFList.clear();

    iMetadataKeyReleaseList.reserve(6);
    iMetadataKeyReleaseList.clear();

    iMetadataValueReleaseList.reserve(6);
    iMetadataValueReleaseList.clear();

    AddToScheduler();

    // Retrieve the logger object
    iLogger = PVLogger::GetLoggerObject("PVPlayerEngine");
    iPerfLogger = PVLogger::GetLoggerObject("pvplayerdiagnostics.perf.engine");
    iReposLogger = PVLogger::GetLoggerObject("pvplayerrepos.engine");

    // Initialize the playback clock to use tickcount timebase
    iPlaybackClock.SetClockTimebase(iPlaybackTimebase);
    uint32 starttime = 0;
    bool overflow = 0;
    iPlaybackClock.SetStartTime32(starttime, PVMF_MEDIA_CLOCK_MSEC, overflow);
    iPlaybackClock.ConstructMediaClockNotificationsInterface(iClockNotificationsInf, *this,
            iCurrCallbackTimerLatency);

    // Initialize the OSCL timer for polling checks
    iPollingCheckTimer = OSCL_NEW(OsclTimer<OsclMemAllocator>, ("playerengine_pollingcheck"));
    iPollingCheckTimer->SetObserver(this);
    iPollingCheckTimer->SetFrequency(10);  // 100 ms resolution

    iWatchDogTimer = OSCL_NEW(PVPlayerWatchdogTimer, (this));

    PVPlayerRegistryPopulator::Populate(iPlayerNodeRegistry, iPlayerRecognizerRegistry);

    return;
}


void PVPlayerEngine::Run()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::Run() In"));
    int32 leavecode = 0;

    /* Engine AO will execute commands in the following sequence
     * 1) If Engine state is Resetting, which will happen when Engine does ErrorHandling,
     * processing Reset or CancelAllCommands
     * issued by the app, engine will not try to execute any other command during this state.
     * 2) If Engine is not in Resetting state then it will process commands in the following order, which ever is true:
     *    (i) If Engine needs to do Error handling because of some error from Source Node or Datapath.
     *    Either start error handling or complete it.
     *    (ii) If Engine has Reset or CancelAllCommands in CurrentCommandQueue,
     *    engine will do CommandComplete for the CurrentCommand.
     *	  (iii) If Engine has Prepare in CurrentCommandQueue, engine will call DoPrepare again
     *    as a part of track selection logic
     *    (iv) If Engine has CancelAllCommands or CancelAcquireLicense in Pending CommandQueue,
     *    engine will start Cancel commands.
     *    (v) Go for Rollover if in Init State and Roll-over is ongoing.
     *    (vi) Process which ever command is pushed in Pending queue.
     * Engine will process any one of the command as listed above in the same order.
     * Every time engine AO is scheduled, engine will go through
     * these steps.
     */

    if (iState == PVP_ENGINE_STATE_RESETTING)
    {
        //this means error handling, reset or cancelall is still in progress
        //pls note that the state will be set to idle
        //in either HandleSourceNodeReset or HandleDataPathReset
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVPlayerEngine::Run() Return engine in resetting state, No processing until engine is in Idle state"));
        return;
    }

    /* Check if ErrorHandling request was made */
    if (!iPendingCmds.empty())
    {
        switch (iPendingCmds.top().GetCmdType())
        {
            case PVP_ENGINE_COMMAND_ERROR_HANDLING_ADD_DATA_SOURCE:
            case PVP_ENGINE_COMMAND_ERROR_HANDLING_INIT:
            case PVP_ENGINE_COMMAND_ERROR_HANDLING_PREPARE:
            case PVP_ENGINE_COMMAND_ERROR_HANDLING_PAUSE:
            case PVP_ENGINE_COMMAND_ERROR_HANDLING_RESUME:
            case PVP_ENGINE_COMMAND_ERROR_HANDLING_SET_PLAYBACK_RANGE:
            case PVP_ENGINE_COMMAND_ERROR_HANDLING_SET_PLAYBACK_RATE:
            case PVP_ENGINE_COMMAND_ERROR_HANDLING_STOP:
            case PVP_ENGINE_COMMAND_ERROR_HANDLING_CANCEL_ALL_COMMANDS:
            case PVP_ENGINE_COMMAND_ERROR_HANDLING_GENERAL:
            {
                // go in error handling right away
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::Run() Processing Error Handling request"));
                PVMFStatus retVal = DoErrorHandling();
                if (retVal == PVMFSuccess)
                {
                    iPendingCmds.pop();
                    RunIfNotReady(); // schedule the engine AO to process other commands in queue if any.
                }
                return;
            }

            default:
                break;
        }
    }

    // if current command being processed is reset or cancelAll and
    // Player engine state is idle then remove the data source
    // and do reset/cancelAll command complete
    // OR
    // if current command being processed is prepare, need to call
    // DoPrepare again because of track selection
    if (!iCurrentCmd.empty())
    {
        if ((iCurrentCmd[0].GetCmdType() == PVP_ENGINE_COMMAND_RESET) ||
                (iCurrentCmd[0].GetCmdType() == PVP_ENGINE_COMMAND_CANCEL_COMMAND) ||
                (iCurrentCmd[0].GetCmdType() == PVP_ENGINE_COMMAND_CANCEL_ALL_COMMANDS))
        {
            if (iState != PVP_ENGINE_STATE_IDLE)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::Run() Engine not in Idle State, asserting"));
                OSCL_ASSERT(false);
            }
            // First destroy all datapaths.
            DoRemoveAllSinks();
            // now remove the source node.
            if (iDataSource)
            {
                RemoveDataSourceSync(*iDataSource);
            }

            EngineCommandCompleted(iCurrentCmd[0].GetCmdId(), iCurrentCmd[0].GetContext(), PVMFSuccess);
        }
        else if (iCurrentCmd[0].GetCmdType() == PVP_ENGINE_COMMAND_PREPARE)
        {
            PVMFStatus cmdstatus = DoPrepare(iCurrentCmd[0]);

            if (cmdstatus != PVMFSuccess && cmdstatus != PVMFPending)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::Run() Command failed CmdId %d Status %d",
                                iCurrentCmd[0].GetCmdId(), cmdstatus));
                EngineCommandCompleted(iCurrentCmd[0].GetCmdId(), iCurrentCmd[0].GetContext(), cmdstatus);
            }
        }
    }

    /* Check if Cancel()/CancelAll()/CancelAcquireLicense request was made */
    if (!iPendingCmds.empty())
    {
        if (iPendingCmds.top().GetCmdType() == PVP_ENGINE_COMMAND_CANCEL_COMMAND)
        {
            // Process it right away
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::Run() Processing Cancel() request"));
            PVPlayerEngineCommand cmd(iPendingCmds.top());
            iPendingCmds.pop();
            if ((!iCurrentCmd.empty()) && (iCurrentCmd[0].GetCmdId() == cmd.GetParam(0).int32_value))
            {
                // We need to cancel the ongoing command. In this case issue cancelAll
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "PVPlayerEngine::Run: Command to Cancel is ongoing so issue CancelAll"));
                DoCancelAllCommands(cmd);
            }
            else
            {
                // The command to be cancelled is in the pending queue
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "PVPlayerEngine::Run: Command to Cancel is pending so just Cancel"));
                DoCancelCommand(cmd);
            }
            return;
        }
        else if (iPendingCmds.top().GetCmdType() == PVP_ENGINE_COMMAND_CANCEL_ALL_COMMANDS)
        {
            // Process it right away
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::Run() Processing CancelAll() request"));
            PVPlayerEngineCommand cmd(iPendingCmds.top());
            iPendingCmds.pop();
            DoCancelAllCommands(cmd);
            return;
        }
        else if (iPendingCmds.top().GetCmdType() == PVP_ENGINE_COMMAND_CANCEL_ACQUIRE_LICENSE)
        {
            // Process it right away
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::Run() Processing CancelAcquireLicesense() request"));
            PVPlayerEngineCommand cmd(iPendingCmds.top());
            iPendingCmds.pop();
            DoCancelAcquireLicense(cmd);
            return;
        }
    }

    if (iRollOverState == RollOverStateStart)
    {
        if (iCurrentCmd[0].GetCmdType() == PVP_ENGINE_COMMAND_INIT)
        {
            //implies that we are doing a source rollover
            PVMFStatus status =
                DoSourceNodeRollOver(iCurrentCmd[0].iCmdId,
                                     iCurrentCmd[0].iContextData);

            if (status != PVMFPending)
            {
                if (CheckForSourceRollOver())
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::Run() DoSourceNodeRollOver Failed, alternate source node for rollover is available"));
                    RunIfNotReady();
                    return;
                }
                // roll over failed
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::Run() DoSourceNodeRollOver Failed, go in error handling"));
                bool ehPending = CheckForPendingErrorHandlingCmd();
                if (ehPending)
                {
                    // there should be no error handling queued.
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::Run() Already EH pending, should never happen"));
                    return;
                }
                // go in error handling
                iCommandCompleteStatusInErrorHandling = status;
                iCommandCompleteErrMsgInErrorHandling = NULL;
                AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_INIT, NULL, NULL, NULL, false);
                iRollOverState = RollOverStateIdle;
                return;
            }
            else
            {
                iRollOverState = RollOverStateInProgress;
            }
        }
        else
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::Run() Source Roll Over In Progress But Incorrect Engine Cmd"));
            EngineCommandCompleted(iCurrentCmd[0].GetCmdId(), iCurrentCmd[0].GetContext(), PVMFErrInvalidState);
        }
        return;
    }

    if (iRollOverState == RollOverStateInProgress)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::RunL() Source Roll Over In Progress "));
        return;
    }

    // Handle other requests normally
    if (!iPendingCmds.empty() && iCurrentCmd.empty())
    {
        // Retrieve the first pending command from queue
        PVPlayerEngineCommand cmd(iPendingCmds.top());
        iPendingCmds.pop();

        // Put in on the current command queue
        leavecode = 0;
        OSCL_TRY(leavecode, iCurrentCmd.push_front(cmd));
        OSCL_FIRST_CATCH_ANY(leavecode,
                             PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::Run() Command could not be pushed onto iCurrentCmd vector"));
                             EngineCommandCompleted(cmd.GetCmdId(), cmd.GetContext(), PVMFErrNoMemory);
                             return;);

        // Process the command according to the cmd type
        PVMFStatus cmdstatus = PVMFSuccess;
        bool ootsync = false;
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::Run() Processing command with type=%d", cmd.GetCmdType()));
        switch (cmd.GetCmdType())
        {
            case PVP_ENGINE_COMMAND_GET_SDK_INFO:
                cmdstatus = DoGetSDKInfo(cmd);
                break;

            case PVP_ENGINE_COMMAND_GET_SDK_MODULE_INFO:
                // GetSDKModuleInfo is currently not supported
                cmdstatus = PVMFErrNotSupported;
                break;

            case PVP_ENGINE_COMMAND_SET_LOG_APPENDER:
                cmdstatus = DoSetLogAppender(cmd);
                break;

            case PVP_ENGINE_COMMAND_REMOVE_LOG_APPENDER:
                cmdstatus = DoRemoveLogAppender(cmd);
                break;

            case PVP_ENGINE_COMMAND_SET_LOG_LEVEL:
                cmdstatus = DoSetLogLevel(cmd);
                break;

            case PVP_ENGINE_COMMAND_GET_LOG_LEVEL:
                cmdstatus = DoGetLogLevel(cmd);
                break;

            case PVP_ENGINE_COMMAND_QUERY_UUID:
                cmdstatus = DoQueryUUID(cmd);;
                break;

            case PVP_ENGINE_COMMAND_QUERY_INTERFACE:
                cmdstatus = DoQueryInterface(cmd);
                break;

            case PVP_ENGINE_COMMAND_GET_PVPLAYER_STATE:
                cmdstatus = DoGetPVPlayerState(cmd, false);
                break;

            case PVP_ENGINE_COMMAND_GET_PVPLAYER_STATE_OOTSYNC:
                ootsync = true;
                cmdstatus = DoGetPVPlayerState(cmd, true);
                break;

            case PVP_ENGINE_COMMAND_ADD_DATA_SOURCE:
                cmdstatus = DoAddDataSource(cmd);
                break;

            case PVP_ENGINE_COMMAND_INIT:
                cmdstatus = DoInit(cmd);
                break;

            case PVP_ENGINE_COMMAND_GET_METADATA_KEY:
                cmdstatus = DoGetMetadataKey(cmd);
                break;

            case PVP_ENGINE_COMMAND_GET_METADATA_VALUE:
                cmdstatus = DoGetMetadataValue(cmd);
                break;

            case PVP_ENGINE_COMMAND_RELEASE_METADATA_VALUE:
                cmdstatus = DoReleaseMetadataValues(cmd);
                break;

            case PVP_ENGINE_COMMAND_ADD_DATA_SINK:
                cmdstatus = DoAddDataSink(cmd);
                break;

            case PVP_ENGINE_COMMAND_GET_CURRENT_POSITION:
                cmdstatus = DoGetCurrentPosition(cmd, false);
                break;

            case PVP_ENGINE_COMMAND_GET_CURRENT_POSITION_OOTSYNC:
                ootsync = true;
                cmdstatus = DoGetCurrentPosition(cmd, true);
                break;

            case PVP_ENGINE_COMMAND_SET_PLAYBACK_RANGE:
                cmdstatus = DoSetPlaybackRange(cmd);
                break;

            case PVP_ENGINE_COMMAND_GET_PLAYBACK_RANGE:
                cmdstatus = DoGetPlaybackRange(cmd);
                break;

            case PVP_ENGINE_COMMAND_SET_PLAYBACK_RATE:
                cmdstatus = DoSetPlaybackRate(cmd);
                break;

            case PVP_ENGINE_COMMAND_GET_PLAYBACK_RATE:
                cmdstatus = DoGetPlaybackRate(cmd);
                break;

            case PVP_ENGINE_COMMAND_GET_PLAYBACK_MINMAX_RATE:
                cmdstatus = DoGetPlaybackMinMaxRate(cmd);
                break;

            case PVP_ENGINE_COMMAND_PREPARE:
                cmdstatus = DoPrepare(cmd);
                break;

            case PVP_ENGINE_COMMAND_START:
                cmdstatus = DoStart(cmd);
                break;

            case PVP_ENGINE_COMMAND_PAUSE:
            case PVP_ENGINE_COMMAND_PAUSE_DUE_TO_ENDOFCLIP:
            case PVP_ENGINE_COMMAND_PAUSE_DUE_TO_ENDTIME_REACHED:
                cmdstatus = DoPause(cmd);
                break;

            case PVP_ENGINE_COMMAND_RESUME:
                cmdstatus = DoResume(cmd);
                break;

            case PVP_ENGINE_COMMAND_STOP:
                cmdstatus = DoStop(cmd);
                break;

            case PVP_ENGINE_COMMAND_REMOVE_DATA_SINK:
                cmdstatus = DoRemoveDataSink(cmd);
                break;

            case PVP_ENGINE_COMMAND_RESET:
                cmdstatus = DoReset(cmd);
                break;

            case PVP_ENGINE_COMMAND_REMOVE_DATA_SOURCE:
                cmdstatus = DoRemoveDataSource(cmd);
                break;

            case PVP_ENGINE_COMMAND_CAPCONFIG_SET_PARAMETERS:
                cmdstatus = DoCapConfigSetParameters(cmd, false);
                break;

            case PVP_ENGINE_COMMAND_CAPCONFIG_SET_PARAMETERS_OOTSYNC:
                ootsync = true;
                cmdstatus = DoCapConfigSetParameters(cmd, true);
                break;

            case PVP_ENGINE_COMMAND_CAPCONFIG_GET_PARAMETERS_OOTSYNC:
                ootsync = true;
                cmdstatus = DoGetParametersSync(cmd);
                break;

            case PVP_ENGINE_COMMAND_CAPCONFIG_RELEASE_PARAMETERS_OOTSYNC:
                ootsync = true;
                cmdstatus = DoReleaseParametersSync(cmd);
                break;

            case PVP_ENGINE_COMMAND_CAPCONFIG_VERIFY_PARAMETERS_OOTSYNC:
                ootsync = true;
                cmdstatus = DoVerifyParametersSync(cmd);
                break;

            case PVP_ENGINE_COMMAND_ACQUIRE_LICENSE_WCHAR:
            case PVP_ENGINE_COMMAND_ACQUIRE_LICENSE_CHAR:
                cmdstatus = DoAcquireLicense(cmd);
                break;

            case PVP_ENGINE_COMMAND_PAUSE_DUE_TO_BUFFER_UNDERFLOW:
                cmdstatus = DoSourceUnderflowAutoPause(cmd);
                break;

            case PVP_ENGINE_COMMAND_RESUME_DUE_TO_BUFFER_DATAREADY:
                cmdstatus = DoSourceDataReadyAutoResume(cmd);
                break;

            case PVP_ENGINE_COMMAND_CAPCONFIG_SET_OBSERVER_OOTSYNC:
                ootsync = true;
                cmdstatus = DoSetObserverSync(cmd);
                break;

            case PVP_ENGINE_COMMAND_GET_LICENSE_STATUS_OOTSYNC:
                ootsync = true;
                cmdstatus = DoGetLicenseStatusSync(cmd);
                break;

            case PVP_ENGINE_COMMAND_CANCEL_COMMAND:
                // Cancel() should not be handled here
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::Run() CancelCommand should be not handled in here. Return Failure"));
                cmdstatus = PVMFFailure;
                break;

            case PVP_ENGINE_COMMAND_CANCEL_ALL_COMMANDS:
                // CancelAll() should not be handled here
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::Run() CancelAllCommands should be not handled in here. Return Failure"));
                cmdstatus = PVMFFailure;
                break;

            default:
                // Just handle as "not supported"
                cmdstatus = PVMFErrNotSupported;
                break;
        }

        if (ootsync)
        {
            OOTSyncCommandComplete(cmd, cmdstatus);
            // Empty out the current cmd vector and set active if there are other pending commands
            iCurrentCmd.erase(iCurrentCmd.begin());
            if (!iPendingCmds.empty())
            {
                RunIfNotReady();
            }
        }
        else if (cmdstatus != PVMFSuccess && cmdstatus != PVMFPending)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::Run() Command failed CmdId %d Status %d",
                            cmd.GetCmdId(), cmdstatus));
            EngineCommandCompleted(cmd.GetCmdId(), cmd.GetContext(), cmdstatus);
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::Run() Out"));
}

bool PVPlayerEngine::FindNodeTypeByNode(PVMFNodeInterface* aUnknownNode, PVPlayerNodeType& aNodeType, int32& aDatapathListIndex)
{
    if (aUnknownNode == NULL)
    {
        // Cannot check with node pointer being NULL
        // Might bring up false positives
        aNodeType = PVP_NODETYPE_UNKNOWN;
        aDatapathListIndex = -1;
        return false;
    }

    // Go through each engine datapath and find whether
    // the specified node is a dec node or sink node
    for (uint32 i = 0; i < iDatapathList.size(); ++i)
    {
        if (iDatapathList[i].iDecNode == aUnknownNode)
        {
            aNodeType = PVP_NODETYPE_DECODER;
            aDatapathListIndex = i;
            return true;
        }
        else if (iDatapathList[i].iSinkNode == aUnknownNode)
        {
            aNodeType = PVP_NODETYPE_SINK;
            aDatapathListIndex = i;
            return true;
        }
    }

    // Could not determine the types
    aNodeType = PVP_NODETYPE_UNKNOWN;
    aDatapathListIndex = -1;
    return false;
}

bool PVPlayerEngine::FindTrackForDatapathUsingMimeString(bool& aVideoTrack, bool& aAudioTrack, bool& aTextTrack, PVPlayerEngineDatapath* aDatapath)
{
    if (aDatapath->iTrackInfo)
    {
        char* mimeString = aDatapath->iTrackInfo->getTrackMimeType().get_str();

        if ((pv_mime_strcmp(mimeString, PVMF_MIME_YUV420) == 0) ||
                (pv_mime_strcmp(mimeString, PVMF_MIME_YUV422) == 0) ||
                (pv_mime_strcmp(mimeString, PVMF_MIME_RGB8) == 0) ||
                (pv_mime_strcmp(mimeString, PVMF_MIME_RGB12) == 0) ||
                (pv_mime_strcmp(mimeString, PVMF_MIME_RGB16) == 0) ||
                (pv_mime_strcmp(mimeString, PVMF_MIME_RGB24) == 0) ||
                (pv_mime_strcmp(mimeString, PVMF_MIME_M4V) == 0) ||
                (pv_mime_strcmp(mimeString, PVMF_MIME_H2631998) == 0) ||
                (pv_mime_strcmp(mimeString, PVMF_MIME_H2632000) == 0) ||
                (pv_mime_strcmp(mimeString, PVMF_MIME_H264_VIDEO_RAW) == 0) ||
                (pv_mime_strcmp(mimeString, PVMF_MIME_H264_VIDEO_MP4) == 0) ||
                (pv_mime_strcmp(mimeString, PVMF_MIME_H264_VIDEO) == 0) ||
                (pv_mime_strcmp(mimeString, PVMF_MIME_WMV) == 0) ||
                (pv_mime_strcmp(mimeString, PVMF_MIME_REAL_VIDEO) == 0))
        {
            aVideoTrack = true;
            aAudioTrack = false;
            aTextTrack = false;
            return true;
        }
        else if (pv_mime_strcmp(mimeString, PVMF_MIME_3GPP_TIMEDTEXT) == 0)
        {
            aVideoTrack = false;
            aAudioTrack = false;
            aTextTrack = true;
            return true;
        }
        else if ((pv_mime_strcmp(mimeString, PVMF_MIME_PCM) == 0) ||
                 (pv_mime_strcmp(mimeString, PVMF_MIME_PCM8) == 0) ||
                 (pv_mime_strcmp(mimeString, PVMF_MIME_PCM16) == 0) ||
                 (pv_mime_strcmp(mimeString, PVMF_MIME_PCM16_BE) == 0) ||
                 (pv_mime_strcmp(mimeString, PVMF_MIME_ULAW) == 0) ||
                 (pv_mime_strcmp(mimeString, PVMF_MIME_ALAW) == 0) ||
                 (pv_mime_strcmp(mimeString, PVMF_MIME_AMR) == 0) ||
                 (pv_mime_strcmp(mimeString, PVMF_MIME_AMRWB) == 0) ||
                 (pv_mime_strcmp(mimeString, PVMF_MIME_AMR_IETF) == 0) ||
                 (pv_mime_strcmp(mimeString, PVMF_MIME_AMRWB_IETF) == 0) ||
                 (pv_mime_strcmp(mimeString, PVMF_MIME_AMR_IF2) == 0) ||
                 (pv_mime_strcmp(mimeString, PVMF_MIME_EVRC) == 0) ||
                 (pv_mime_strcmp(mimeString, PVMF_MIME_MP3) == 0) ||
                 (pv_mime_strcmp(mimeString, PVMF_MIME_ADIF) == 0) ||
                 (pv_mime_strcmp(mimeString, PVMF_MIME_ADTS) == 0) ||
                 (pv_mime_strcmp(mimeString, PVMF_MIME_LATM) == 0) ||
                 (pv_mime_strcmp(mimeString, PVMF_MIME_MPEG4_AUDIO) == 0) ||
                 (pv_mime_strcmp(mimeString, PVMF_MIME_G723) == 0) ||
                 (pv_mime_strcmp(mimeString, PVMF_MIME_G726) == 0) ||
                 (pv_mime_strcmp(mimeString, PVMF_MIME_WMA) == 0) ||
                 (pv_mime_strcmp(mimeString, PVMF_MIME_ASF_AMR) == 0) ||
                 (pv_mime_strcmp(mimeString, PVMF_MIME_REAL_AUDIO) == 0) ||
                 (pv_mime_strcmp(mimeString, PVMF_MIME_ASF_MPEG4_AUDIO) == 0) ||
                 (pv_mime_strcmp(mimeString, PVMF_MIME_3640) == 0))
        {
            aVideoTrack = false;
            aAudioTrack = true;
            aTextTrack = false;
            return true;
        }
        else
        {
            aVideoTrack = false;
            aAudioTrack = false;
            aTextTrack = false;
            return false;
        }
    }

    aVideoTrack = false;
    aAudioTrack = false;
    aTextTrack = false;
    return false;
}

bool PVPlayerEngine::FindDatapathForTrackUsingMimeString(bool aVideoTrack, bool aAudioTrack, bool aTextTrack, int32& aDatapathListIndex)
{
    for (uint32 i = 0; i < iDatapathList.size(); i++)
    {
        if (iDatapathList[i].iTrackInfo)
        {
            char* mimeString = iDatapathList[i].iTrackInfo->getTrackMimeType().get_str();
            if (aVideoTrack)
            {
                // find a datapath using the mime string for Video track
                if ((pv_mime_strcmp(mimeString, PVMF_MIME_YUV420) == 0) ||
                        (pv_mime_strcmp(mimeString, PVMF_MIME_YUV422) == 0) ||
                        (pv_mime_strcmp(mimeString, PVMF_MIME_RGB8) == 0) ||
                        (pv_mime_strcmp(mimeString, PVMF_MIME_RGB12) == 0) ||
                        (pv_mime_strcmp(mimeString, PVMF_MIME_RGB16) == 0) ||
                        (pv_mime_strcmp(mimeString, PVMF_MIME_RGB24) == 0) ||
                        (pv_mime_strcmp(mimeString, PVMF_MIME_M4V) == 0) ||
                        (pv_mime_strcmp(mimeString, PVMF_MIME_H2631998) == 0) ||
                        (pv_mime_strcmp(mimeString, PVMF_MIME_H2632000) == 0) ||
                        (pv_mime_strcmp(mimeString, PVMF_MIME_H264_VIDEO_RAW) == 0) ||
                        (pv_mime_strcmp(mimeString, PVMF_MIME_H264_VIDEO_MP4) == 0) ||
                        (pv_mime_strcmp(mimeString, PVMF_MIME_H264_VIDEO) == 0) ||
                        (pv_mime_strcmp(mimeString, PVMF_MIME_WMV) == 0) ||
                        (pv_mime_strcmp(mimeString, PVMF_MIME_REAL_VIDEO) == 0))
                {
                    aDatapathListIndex = i;
                    return true;
                }
            }
            else if (aAudioTrack)
            {
                // find a datapath using the mime string for Audio track
                if ((pv_mime_strcmp(mimeString, PVMF_MIME_PCM) == 0) ||
                        (pv_mime_strcmp(mimeString, PVMF_MIME_PCM8) == 0) ||
                        (pv_mime_strcmp(mimeString, PVMF_MIME_PCM16) == 0) ||
                        (pv_mime_strcmp(mimeString, PVMF_MIME_PCM16_BE) == 0) ||
                        (pv_mime_strcmp(mimeString, PVMF_MIME_ULAW) == 0) ||
                        (pv_mime_strcmp(mimeString, PVMF_MIME_ALAW) == 0) ||
                        (pv_mime_strcmp(mimeString, PVMF_MIME_AMR) == 0) ||
                        (pv_mime_strcmp(mimeString, PVMF_MIME_AMRWB) == 0) ||
                        (pv_mime_strcmp(mimeString, PVMF_MIME_AMR_IETF) == 0) ||
                        (pv_mime_strcmp(mimeString, PVMF_MIME_AMRWB_IETF) == 0) ||
                        (pv_mime_strcmp(mimeString, PVMF_MIME_AMR_IF2) == 0) ||
                        (pv_mime_strcmp(mimeString, PVMF_MIME_EVRC) == 0) ||
                        (pv_mime_strcmp(mimeString, PVMF_MIME_MP3) == 0) ||
                        (pv_mime_strcmp(mimeString, PVMF_MIME_ADIF) == 0) ||
                        (pv_mime_strcmp(mimeString, PVMF_MIME_ADTS) == 0) ||
                        (pv_mime_strcmp(mimeString, PVMF_MIME_LATM) == 0) ||
                        (pv_mime_strcmp(mimeString, PVMF_MIME_MPEG4_AUDIO) == 0) ||
                        (pv_mime_strcmp(mimeString, PVMF_MIME_G723) == 0) ||
                        (pv_mime_strcmp(mimeString, PVMF_MIME_G726) == 0) ||
                        (pv_mime_strcmp(mimeString, PVMF_MIME_WMA) == 0) ||
                        (pv_mime_strcmp(mimeString, PVMF_MIME_ASF_AMR) == 0) ||
                        (pv_mime_strcmp(mimeString, PVMF_MIME_REAL_AUDIO) == 0) ||
                        (pv_mime_strcmp(mimeString, PVMF_MIME_ASF_MPEG4_AUDIO) == 0) ||
                        (pv_mime_strcmp(mimeString, PVMF_MIME_3640) == 0))
                {
                    aDatapathListIndex = i;
                    return true;
                }
            }
            else if (aTextTrack)
            {
                // find a datapath using the mime string for Text track
                if (pv_mime_strcmp(mimeString, PVMF_MIME_3GPP_TIMEDTEXT) == 0)
                {
                    aDatapathListIndex = i;
                    return true;
                }
            }
            else
            {
                // Unknown track
                aDatapathListIndex = -1;
                return false;
            }
        }
    }

    // Unknown track
    aDatapathListIndex = -1;
    return false;
}


void PVPlayerEngine::NodeCommandCompleted(const PVMFCmdResp& aResponse)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::NodeCommandCompleted() In"));

    int32 leavecode = 0;

    // Check if a cancel command completed
    uint32* context_uint32 = (uint32*)(aResponse.GetContext());
    if (context_uint32 == &iNumberCancelCmdPending)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::NodeCommandCompleted() Cancel in node completed for cancel command. Pending %d", iNumberCancelCmdPending));
        --iNumberCancelCmdPending;

        // If cmd to cancel was GetMetadataKeys() or GetMetadataValues() and if these commands return with
        // success then first release the memory for the node which return with success.
        if (iCmdToCancel[0].GetCmdType() == PVP_ENGINE_COMMAND_GET_METADATA_KEY &&
                aResponse.GetCmdStatus() == PVMFSuccess)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVPlayerEngine::NodeCommandCompleted() Cancel in node completed for GetMetadataKeys with success, release memory."));
            // Release the memory allocated for the metadata keys
            uint32 numkeysadded = iGetMetadataKeysParam.iKeyList->size() - iGetMetadataKeysParam.iNumKeyEntriesInList;
            uint32 start = iGetMetadataKeysParam.iNumKeyEntriesInList;
            uint32 end = iGetMetadataKeysParam.iNumKeyEntriesInList + numkeysadded - 1;

            PVMFMetadataExtensionInterface* mdif = iMetadataIFList[iGetMetadataKeysParam.iCurrentInterfaceIndex].iInterface;
            OSCL_ASSERT(mdif != NULL);
            mdif->ReleaseNodeMetadataKeys(*(iGetMetadataKeysParam.iKeyList), start, end);
        }
        else if (iCmdToCancel[0].GetCmdType() == PVP_ENGINE_COMMAND_GET_METADATA_VALUE &&
                 aResponse.GetCmdStatus() == PVMFSuccess)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVPlayerEngine::NodeCommandCompleted() Cancel in node completed for GetMetadataValue with success, release memory."));
            // Release the memory allocated for the metadata values
            uint32 numkeysadded = iGetMetadataValuesParam.iKeyList->size() - iGetMetadataValuesParam.iNumValueEntriesInList;
            uint32 start = iGetMetadataValuesParam.iNumValueEntriesInList;
            uint32 end = iGetMetadataValuesParam.iNumValueEntriesInList + numkeysadded - 1;

            PVMFMetadataExtensionInterface* mdif = iMetadataIFList[iGetMetadataValuesParam.iCurrentInterfaceIndex].iInterface;
            OSCL_ASSERT(mdif != NULL);
            mdif->ReleaseNodeMetadataValues(*(iGetMetadataValuesParam.iValueList), start, end);

            iReleaseMetadataValuesPending = false;
        }

        if (iNumberCancelCmdPending == 0)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::NodeCommandCompleted() Cancelling of all node/datapath commands complete, now reset all nodes"));
            // Clear the CancelCmd queue as the cmd has been cancelled.
            iCmdToCancel.clear();

            RemoveDatapathContextFromList(); // empty left over contexts from cancelled datapath commands
            // Now reset the source node
            PVPlayerEngineContext* context = AllocateEngineContext(NULL, iSourceNode, NULL, -1, NULL, -1);

            PVMFCommandId cmdid = -1;
            int32 leavecode = 0;
            OSCL_TRY(leavecode, cmdid = iSourceNode->Reset(iSourceNodeSessionId, (OsclAny*)context));
            OSCL_FIRST_CATCH_ANY(leavecode,

                                 PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::NodeCommandCompleted() Reset on iSourceNode did a leave!"));
                                 FreeEngineContext(context);
                                 OSCL_ASSERT(false);
                                 return);

            SetEngineState(PVP_ENGINE_STATE_RESETTING);
        }
        return;
    }

    PVPlayerEngineContext* nodecontext = (PVPlayerEngineContext*)(aResponse.GetContext());
    OSCL_ASSERT(nodecontext);

    // Ignore other node completion if cancelling
    if (!iCmdToCancel.empty() || (CheckForPendingErrorHandlingCmd() && aResponse.GetCmdStatus() == PVMFErrCancelled))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::NodeCommandCompleted() Node command completion ignored due to cancel process, id=%d", aResponse.GetCmdId()));
        // Remove the context from the list
        FreeEngineContext(nodecontext);
        return;
    }

    // Process according to cmd type in the engine context data, node type, or engine state
    if (nodecontext->iCmdType == PVP_CMD_SinkNodeSkipMediaData)
    {
        HandleSinkNodeSkipMediaData(*nodecontext, aResponse);
    }
    else if (nodecontext->iCmdType == PVP_CMD_SinkNodeSkipMediaDataDuringPlayback)
    {
        HandleSinkNodeSkipMediaDataDuringPlayback(*nodecontext, aResponse);
    }
    else if (nodecontext->iCmdType == PVP_CMD_SinkNodeAutoPause)
    {
        HandleSinkNodePause(*nodecontext, aResponse);
    }
    else if (nodecontext->iCmdType == PVP_CMD_SinkNodeAutoResume)
    {
        HandleSinkNodeResume(*nodecontext, aResponse);
    }
    else if (nodecontext->iCmdType == PVP_CMD_DecNodeReset)
    {
        HandleDecNodeReset(*nodecontext, aResponse);
    }
    else if (nodecontext->iCmdType == PVP_CMD_SinkNodeReset)
    {
        HandleSinkNodeReset(*nodecontext, aResponse);
    }
    else if (nodecontext->iCmdType == PVP_CMD_GetNodeMetadataKey)
    {
        // Ignore the command status since it does not matter and continue going through the metadata interface list

        // Determine the number of keys were added
        uint32 numkeysadded = iGetMetadataKeysParam.iKeyList->size() - iGetMetadataKeysParam.iNumKeyEntriesInList;
        if (numkeysadded > 0)
        {
            // Create an entry for the metadata key release list
            PVPlayerEngineMetadataReleaseEntry releaseentry;
            releaseentry.iMetadataIFListIndex = iGetMetadataKeysParam.iCurrentInterfaceIndex;
            // Save the start and end indices into the key list for keys that this node added
            releaseentry.iStartIndex = iGetMetadataKeysParam.iNumKeyEntriesInList;
            releaseentry.iEndIndex = iGetMetadataKeysParam.iNumKeyEntriesInList + numkeysadded - 1;

            leavecode = 0;
            OSCL_TRY(leavecode, iMetadataKeyReleaseList.push_back(releaseentry));
            if (leavecode != 0)
            {
                // An element could not be added to the release list vector
                // so notify completion of GetMetadataKey() command with memory failure
                EngineCommandCompleted(nodecontext->iCmdId, (OsclAny*)nodecontext->iCmdContext, PVMFErrNoMemory);

                // Release the last requested keys
                PVMFMetadataExtensionInterface* mdif = iMetadataIFList[releaseentry.iMetadataIFListIndex].iInterface;
                OSCL_ASSERT(mdif != NULL);
                mdif->ReleaseNodeMetadataKeys(*(iGetMetadataKeysParam.iKeyList), releaseentry.iStartIndex, releaseentry.iEndIndex);

                // Release the memory allocated for rest of the metadata keys
                while (iMetadataKeyReleaseList.empty() == false)
                {
                    mdif = iMetadataIFList[iMetadataKeyReleaseList[0].iMetadataIFListIndex].iInterface;
                    OSCL_ASSERT(mdif != NULL);
                    mdif->ReleaseNodeMetadataKeys(*(iGetMetadataKeysParam.iKeyList), iMetadataKeyReleaseList[0].iStartIndex, iMetadataKeyReleaseList[0].iEndIndex);
                    iMetadataKeyReleaseList.erase(iMetadataKeyReleaseList.begin());
                }

                // Remove the context from the list
                // Need to do this since we're calling return from here
                FreeEngineContext(nodecontext);
                return;
            }

            // Update the variables tracking the key list
            if (iGetMetadataKeysParam.iNumKeyEntriesToFill != -1)
            {
                iGetMetadataKeysParam.iNumKeyEntriesToFill -= numkeysadded;
            }
            iGetMetadataKeysParam.iNumKeyEntriesInList += numkeysadded;
        }

        // Update the interface index to the next one
        ++iGetMetadataKeysParam.iCurrentInterfaceIndex;

        // Loop until GetNodeMetadataKeys() is called or command is completed
        bool endloop = false;
        while (endloop == false)
        {
            // Check if there is another metadata interface to check
            if (iGetMetadataKeysParam.iCurrentInterfaceIndex < iMetadataIFList.size())
            {
                PVMFMetadataExtensionInterface* mdif = iMetadataIFList[iGetMetadataKeysParam.iCurrentInterfaceIndex].iInterface;
                OSCL_ASSERT(mdif != NULL);
                PVMFSessionId sessionid = iMetadataIFList[iGetMetadataKeysParam.iCurrentInterfaceIndex].iSessionId;

                // Determine the number of keys available for the specified query key
                int32 numkeys = mdif->GetNumMetadataKeys(iGetMetadataKeysParam.iQueryKey);
                if (numkeys <= 0)
                {
                    // Since there is no keys from this node, go to the next one
                    ++iGetMetadataKeysParam.iCurrentInterfaceIndex;
                    continue;
                }

                // If more key entries can be added, retrieve from the node
                if (iGetMetadataKeysParam.iNumKeyEntriesToFill > 0 || iGetMetadataKeysParam.iNumKeyEntriesToFill == -1)
                {
                    int32 leavecode = 0;
                    PVMFCommandId cmdid = -1;
                    PVPlayerEngineContext* newcontext = AllocateEngineContext(iMetadataIFList[iGetMetadataKeysParam.iCurrentInterfaceIndex].iEngineDatapath, iMetadataIFList[iGetMetadataKeysParam.iCurrentInterfaceIndex].iNode, NULL, nodecontext->iCmdId, nodecontext->iCmdContext, PVP_CMD_GetNodeMetadataKey);
                    OSCL_TRY(leavecode, cmdid = mdif->GetNodeMetadataKeys(sessionid,
                                                *(iGetMetadataKeysParam.iKeyList),
                                                0,
                                                iGetMetadataKeysParam.iNumKeyEntriesToFill,
                                                iGetMetadataKeysParam.iQueryKey,
                                                (OsclAny*)newcontext));
                    OSCL_FIRST_CATCH_ANY(leavecode,
                                         PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::NodeCommandCompleted() GetNodeMetadataKeys on a node did a leave!"));
                                         FreeEngineContext(newcontext);
                                         // Go to the next metadata IF in the list and continue
                                         ++iGetMetadataKeysParam.iCurrentInterfaceIndex;
                                         continue;);

                    // End the loop since GetNodeMetadataKeys() was called
                    endloop = true;
                }
                else
                {
                    // Retrieved the requested number of keys so notify completion of GetMetadataKey() command
                    EngineCommandCompleted(nodecontext->iCmdId, (OsclAny*)nodecontext->iCmdContext, aResponse.GetCmdStatus());

                    // Release the memory allocated for the metadata keys
                    while (iMetadataKeyReleaseList.empty() == false)
                    {
                        mdif = iMetadataIFList[iMetadataKeyReleaseList[0].iMetadataIFListIndex].iInterface;
                        OSCL_ASSERT(mdif != NULL);
                        mdif->ReleaseNodeMetadataKeys(*(iGetMetadataKeysParam.iKeyList), iMetadataKeyReleaseList[0].iStartIndex, iMetadataKeyReleaseList[0].iEndIndex);
                        iMetadataKeyReleaseList.erase(iMetadataKeyReleaseList.begin());
                    }

                    // End the loop since finished command
                    endloop = true;
                }
            }
            else
            {
                // No more so notify completion of GetMetadataKey() command
                EngineCommandCompleted(nodecontext->iCmdId, (OsclAny*)nodecontext->iCmdContext, aResponse.GetCmdStatus());

                // Release the memory allocated for the metadata keys
                while (iMetadataKeyReleaseList.empty() == false)
                {
                    PVMFMetadataExtensionInterface* mdif = iMetadataIFList[iMetadataKeyReleaseList[0].iMetadataIFListIndex].iInterface;
                    OSCL_ASSERT(mdif != NULL);
                    mdif->ReleaseNodeMetadataKeys(*(iGetMetadataKeysParam.iKeyList), iMetadataKeyReleaseList[0].iStartIndex, iMetadataKeyReleaseList[0].iEndIndex);
                    iMetadataKeyReleaseList.erase(iMetadataKeyReleaseList.begin());
                }

                // End the loop since reached the end of the metadata IF list
                endloop = true;
            }
        }
    }
    else if (nodecontext->iCmdType == PVP_CMD_GetNodeMetadataValue)
    {
        // Ignore the command status since it does not matter and continue going through the metadata interface list

        // Determine the number of values were added
        uint32 numvaluesadded = iGetMetadataValuesParam.iValueList->size() - iGetMetadataValuesParam.iNumValueEntriesInList;
        if (numvaluesadded > 0)
        {
            // Create an entry for the metadata value release list
            PVPlayerEngineMetadataReleaseEntry releaseentry;
            releaseentry.iMetadataIFListIndex = iGetMetadataValuesParam.iCurrentInterfaceIndex;
            // Save the start and end indices into the value list for values that this node added
            releaseentry.iStartIndex = iGetMetadataValuesParam.iNumValueEntriesInList;
            releaseentry.iEndIndex = iGetMetadataValuesParam.iNumValueEntriesInList + numvaluesadded - 1;

            leavecode = 0;
            OSCL_TRY(leavecode, iMetadataValueReleaseList.push_back(releaseentry));
            if (leavecode != 0)
            {
                // An element could not be added to the release list vector
                // so notify completion of GetMetadataValue() command with memory failure
                EngineCommandCompleted(nodecontext->iCmdId, (OsclAny*)nodecontext->iCmdContext, PVMFErrNoMemory);

                // Release the last requested values
                PVMFMetadataExtensionInterface* mdif = iMetadataIFList[releaseentry.iMetadataIFListIndex].iInterface;
                OSCL_ASSERT(mdif != NULL);
                mdif->ReleaseNodeMetadataValues(*(iGetMetadataValuesParam.iValueList), releaseentry.iStartIndex, releaseentry.iEndIndex);

                // Release the memory allocated for rest of the metadata values
                while (iMetadataValueReleaseList.empty() == false)
                {
                    mdif = iMetadataIFList[iMetadataValueReleaseList[0].iMetadataIFListIndex].iInterface;
                    OSCL_ASSERT(mdif != NULL);
                    mdif->ReleaseNodeMetadataValues(*(iGetMetadataValuesParam.iValueList), iMetadataValueReleaseList[0].iStartIndex, iMetadataValueReleaseList[0].iEndIndex);
                    iMetadataValueReleaseList.erase(iMetadataValueReleaseList.begin());
                }

                // Remove the context from the list
                // Need to do this since we're calling return from here
                FreeEngineContext(nodecontext);
                return;
            }

            // Update the variables tracking the value list
            if (iGetMetadataValuesParam.iNumValueEntriesToFill != -1)
            {
                iGetMetadataValuesParam.iNumValueEntriesToFill -= numvaluesadded;
            }
            iGetMetadataValuesParam.iNumValueEntriesInList += numvaluesadded;
        }

        // Update the interface index to the next one
        ++iGetMetadataValuesParam.iCurrentInterfaceIndex;

        // Loop until GetNodeMetadataValues() is called or command is completed
        bool endloop = false;
        while (endloop == false)
        {
            // Check if there is another metadata interface to check
            if (iGetMetadataValuesParam.iCurrentInterfaceIndex < iMetadataIFList.size())
            {
                PVMFMetadataExtensionInterface* mdif = iMetadataIFList[iGetMetadataValuesParam.iCurrentInterfaceIndex].iInterface;
                OSCL_ASSERT(mdif != NULL);
                PVMFSessionId sessionid = iMetadataIFList[iGetMetadataValuesParam.iCurrentInterfaceIndex].iSessionId;

                // Determine the number of values available for the specified key list
                int32 numvalues = mdif->GetNumMetadataValues(*(iGetMetadataValuesParam.iKeyList));
                if (numvalues > 0)
                {
                    // Add it to the total available
                    *(iGetMetadataValuesParam.iNumAvailableValues) += numvalues;
                }
                else
                {
                    // Since there is no values from this node, go to the next one
                    ++iGetMetadataValuesParam.iCurrentInterfaceIndex;
                    continue;
                }

                // If more value entries can be added, retrieve from the node
                if (iGetMetadataValuesParam.iNumValueEntriesToFill > 0 || iGetMetadataValuesParam.iNumValueEntriesToFill == -1)
                {
                    int32 leavecode = 0;
                    PVMFCommandId cmdid = -1;
                    PVPlayerEngineContext* newcontext = AllocateEngineContext(iMetadataIFList[iGetMetadataValuesParam.iCurrentInterfaceIndex].iEngineDatapath, iMetadataIFList[iGetMetadataValuesParam.iCurrentInterfaceIndex].iNode, NULL, nodecontext->iCmdId, nodecontext->iCmdContext, PVP_CMD_GetNodeMetadataValue);
                    OSCL_TRY(leavecode, cmdid = mdif->GetNodeMetadataValues(sessionid,
                                                *(iGetMetadataValuesParam.iKeyList),
                                                *(iGetMetadataValuesParam.iValueList),
                                                0,
                                                iGetMetadataValuesParam.iNumValueEntriesToFill,
                                                (OsclAny*)newcontext));
                    OSCL_FIRST_CATCH_ANY(leavecode,
                                         PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::NodeCommandCompleted() GetNodeMetadataValues on a node did a leave!"));
                                         FreeEngineContext(newcontext);
                                         // Go to the next metadata IF in the list and continue
                                         ++iGetMetadataValuesParam.iCurrentInterfaceIndex;
                                         continue;);

                    // End the loop since GetNodeMetadataValues() was called
                    endloop = true;
                }
                else
                {
                    // Retrieved requested number of values so notify completion of GetMetadataValue() command
                    EngineCommandCompleted(nodecontext->iCmdId, (OsclAny*)nodecontext->iCmdContext, aResponse.GetCmdStatus());

                    if (iMetadataValuesCopiedInCallBack)
                    {
                        // Release the memory allocated for the metadata values
                        while (iMetadataValueReleaseList.empty() == false)
                        {
                            mdif = iMetadataIFList[iMetadataValueReleaseList[0].iMetadataIFListIndex].iInterface;
                            OSCL_ASSERT(mdif != NULL);
                            mdif->ReleaseNodeMetadataValues(*(iGetMetadataValuesParam.iValueList), iMetadataValueReleaseList[0].iStartIndex, iMetadataValueReleaseList[0].iEndIndex);
                            iMetadataValueReleaseList.erase(iMetadataValueReleaseList.begin());
                        }
                    }
                    else
                    {
                        iReleaseMetadataValuesPending = true;
                    }

                    // End the loop since finished command
                    endloop = true;
                }
            }
            else
            {
                // No more so notify completion of GetMetadataValue() command
                EngineCommandCompleted(nodecontext->iCmdId, (OsclAny*)nodecontext->iCmdContext, aResponse.GetCmdStatus());

                if (iMetadataValuesCopiedInCallBack)
                {
                    // Release the memory allocated for the metadata values
                    while (iMetadataValueReleaseList.empty() == false)
                    {
                        PVMFMetadataExtensionInterface* mdif = iMetadataIFList[iMetadataValueReleaseList[0].iMetadataIFListIndex].iInterface;
                        OSCL_ASSERT(mdif != NULL);
                        mdif->ReleaseNodeMetadataValues(*(iGetMetadataValuesParam.iValueList), iMetadataValueReleaseList[0].iStartIndex, iMetadataValueReleaseList[0].iEndIndex);
                        iMetadataValueReleaseList.erase(iMetadataValueReleaseList.begin());
                    }
                }
                else
                {
                    iReleaseMetadataValuesPending = true;
                }

                // End the loop since reached the end of the metadata IF list
                endloop = true;
            }
        }
    }
    else if (nodecontext->iNode == iSourceNode)
    {
        if (nodecontext->iCmdType == PVP_CMD_SourceNodeQueryDataSourcePositionDuringPlayback)
        {
            HandleSourceNodeQueryDataSourcePositionDuringPlayback(*nodecontext, aResponse);
        }
        else if (nodecontext->iCmdType == PVP_CMD_SourceNodeSetDataSourcePositionDuringPlayback)
        {
            HandleSourceNodeSetDataSourcePositionDuringPlayback(*nodecontext, aResponse);
        }
        else if (nodecontext->iCmdType == PVP_CMD_SourceNodeSetDataSourceDirection)
        {
            HandleSourceNodeSetDataSourceDirection(*nodecontext, aResponse);
        }
        else if (nodecontext->iCmdType == PVP_CMD_SourceNodeSetDataSourceRate)
        {
            HandleSourceNodeSetDataSourceRate(*nodecontext, aResponse);
        }
        else
        {
            switch (iState)
            {
                case PVP_ENGINE_STATE_IDLE:
                    switch (nodecontext->iCmdType)
                    {
                        case PVP_CMD_SourceNodeQueryInitIF:
                            HandleSourceNodeQueryInitIF(*nodecontext, aResponse);
                            break;

                        case PVP_CMD_SourceNodeQueryTrackSelIF:
                            HandleSourceNodeQueryTrackSelIF(*nodecontext, aResponse);
                            break;

                        case PVP_CMD_SourceNodeQueryTrackLevelInfoIF:
                        case PVP_CMD_SourceNodeQueryPBCtrlIF:
                        case PVP_CMD_SourceNodeQueryDirCtrlIF:
                        case PVP_CMD_SourceNodeQueryMetadataIF:
                        case PVP_CMD_SourceNodeQueryCapConfigIF:
                        case PVP_CMD_SourceNodeQueryCPMLicenseIF:
                        case PVP_CMD_SourceNodeQuerySrcNodeRegInitIF:
                            HandleSourceNodeQueryInterfaceOptional(*nodecontext, aResponse);
                            break;

                        case PVP_CMD_SourceNodeGetLicense:
                            HandleSourceNodeGetLicense(*nodecontext, aResponse);
                            break;

                        case PVP_CMD_SourceNodeCancelGetLicense:
                            HandleSourceNodeCancelGetLicense(*nodecontext, aResponse);
                            break;

                        default:
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::NodeCommandCompleted() Invalid source node command type in PVP_ENGINE_STATE_IDLE. Asserting"));
                            OSCL_ASSERT(false);
                            break;
                    }
                    break;

                case PVP_ENGINE_STATE_INITIALIZED:
                    switch (nodecontext->iCmdType)
                    {
                        case PVP_CMD_SourceNodeGetLicense:
                            HandleSourceNodeGetLicense(*nodecontext, aResponse);
                            break;
                        default:
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::NodeCommandCompleted() Invalid source node command type in PVP_ENGINE_STATE_IDLE. Asserting"));
                            OSCL_ASSERT(false);
                            break;
                    }
                    break;

                case PVP_ENGINE_STATE_INITIALIZING:
                    switch (nodecontext->iCmdType)
                    {
                        case PVP_CMD_SourceNodeInit:
                            HandleSourceNodeInit(*nodecontext, aResponse);
                            break;

                        case PVP_CMD_SourceNodeGetDurationValue:
                            HandleSourceNodeGetDurationValue(*nodecontext, aResponse);
                            break;

                        default:
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::NodeCommandCompleted() Invalid source node command type in PVP_ENGINE_STATE_INITIALIZING. Asserting"));
                            OSCL_ASSERT(false);
                            break;
                    }
                    break;

                case PVP_ENGINE_STATE_PREPARING:
                    switch (nodecontext->iCmdType)
                    {
                        case PVP_CMD_SourceNodePrepare:
                            HandleSourceNodePrepare(*nodecontext, aResponse);
                            break;

                        case PVP_CMD_SourceNodeQueryDataSourcePosition:
                            HandleSourceNodeQueryDataSourcePosition(*nodecontext, aResponse);
                            break;

                        case PVP_CMD_SourceNodeSetDataSourcePosition:
                            HandleSourceNodeSetDataSourcePosition(*nodecontext, aResponse);
                            break;

                        case PVP_CMD_SourceNodeSetDataSourceDirection:
                            //currently not allowed
                            break;

                        case PVP_CMD_SourceNodeStart:
                            HandleSourceNodeStart(*nodecontext, aResponse);
                            break;

                        default:
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::NodeCommandCompleted() Invalid source node command type in PVP_ENGINE_STATE_PREPARING. Asserting"));
                            OSCL_ASSERT(false);
                            break;
                    }
                    break;

                case PVP_ENGINE_STATE_PAUSING:
                    HandleSourceNodePause(*nodecontext, aResponse);
                    break;

                case PVP_ENGINE_STATE_RESUMING:
                    switch (nodecontext->iCmdType)
                    {
                        case PVP_CMD_SourceNodeQueryDataSourcePosition:
                            HandleSourceNodeQueryDataSourcePosition(*nodecontext, aResponse);
                            break;

                        case PVP_CMD_SourceNodeSetDataSourcePosition:
                            HandleSourceNodeSetDataSourcePosition(*nodecontext, aResponse);
                            break;

                        case PVP_CMD_SourceNodeSetDataSourceDirection:
                            HandleSourceNodeSetDataSourceDirection(*nodecontext, aResponse);
                            break;

                        case PVP_CMD_SourceNodeStart:
                            HandleSourceNodeResume(*nodecontext, aResponse);
                            break;

                        default:
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::NodeCommandCompleted() Invalid source node command type in PVP_ENGINE_STATE_RESUMING. Asserting"));
                            OSCL_ASSERT(false);
                            break;
                    }
                    break;

                case PVP_ENGINE_STATE_STOPPING:
                    HandleSourceNodeStop(*nodecontext, aResponse);
                    break;

                case PVP_ENGINE_STATE_RESETTING:
                    HandleSourceNodeReset(*nodecontext, aResponse);
                    break;

                default:
                    break;
            }
        }
    }
    else if (iState == PVP_ENGINE_STATE_PREPARING)
    {
        switch (nodecontext->iCmdType)
        {
            case PVP_CMD_SinkNodeQueryCapConfigIF:
                HandleSinkNodeQueryCapConfigIF(*nodecontext, aResponse);
                break;

            case PVP_CMD_SinkNodeInit:
                HandleSinkNodeInit(*nodecontext, aResponse);
                break;

            case PVP_CMD_DecNodeQueryCapConfigIF:
                HandleDecNodeQueryCapConfigIF(*nodecontext, aResponse);
                break;

            case PVP_CMD_DecNodeInit:
                HandleDecNodeInit(*nodecontext, aResponse);
                break;

            case PVP_CMD_SinkNodeDecNodeReset:
                HandleSinkNodeDecNodeReset(*nodecontext, aResponse);
                break;

            case PVP_CMD_SinkNodeQuerySyncCtrlIF:
            case PVP_CMD_SinkNodeQueryMetadataIF:
                HandleSinkNodeQueryInterfaceOptional(*nodecontext, aResponse);
                break;

            case PVP_CMD_DecNodeQueryMetadataIF:
                HandleDecNodeQueryInterfaceOptional(*nodecontext, aResponse);
                break;

            default:
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::NodeCommandCompleted() Invalid node command type in PVP_ENGINE_STATE_PREPARING. Asserting"));
                OSCL_ASSERT(false);
                break;
        }
    }
    else
    {
        // Unknown node command completion. Assert
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::NodeCommandCompleted() Unknown node command completion"));
        OSCL_ASSERT(false);
    }

    // Remove the context from the list
    FreeEngineContext(nodecontext);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::NodeCommandCompleted() Out"));
}


void PVPlayerEngine::HandleNodeInformationalEvent(const PVMFAsyncEvent& aEvent)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleNodeInformationalEvent() In"));

    PVMFNodeInterface* nodeorigin = (PVMFNodeInterface*)(aEvent.GetContext());

    PVPlayerNodeType nodetype = PVP_NODETYPE_UNKNOWN;
    int32 datapathindex = -1;

    // Process the info event based on the node type reporting the event
    if (nodeorigin == iSourceNode)
    {
        HandleSourceNodeInfoEvent(aEvent);
    }
    else if (FindNodeTypeByNode(nodeorigin, nodetype, datapathindex) == true)
    {
        if (nodetype == PVP_NODETYPE_SINK)
        {
            HandleSinkNodeInfoEvent(aEvent, datapathindex);
        }
        else if (nodetype == PVP_NODETYPE_DECODER)
        {
            HandleDecNodeInfoEvent(aEvent, datapathindex);
        }
        else
        {
            // Event from unknown node or component. Do nothing but log it
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleNodeInformationalEvent() Info event from unknown node type Event type 0x%x Context 0x%x Data 0x%x",
                            aEvent.GetEventType(), aEvent.GetContext(), aEvent.GetEventData()));
        }
    }
    else
    {
        // Event from unknown node or component. Do nothing but log it
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleNodeInformationalEvent() Info event from unknown node Event type 0x%x Context 0x%x Data 0x%x",
                        aEvent.GetEventType(), aEvent.GetContext(), aEvent.GetEventData()));
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleNodeInformationalEvent() Out"));
}


void PVPlayerEngine::HandleNodeErrorEvent(const PVMFAsyncEvent& aEvent)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleNodeErrorEvent() In"));

    PVMFNodeInterface* nodeorigin = (PVMFNodeInterface*)(aEvent.GetContext());

    PVPlayerNodeType nodetype = PVP_NODETYPE_UNKNOWN;
    int32 datapathindex = -1;

    // Process the error event based on the node type reporting the event
    if (nodeorigin == iSourceNode)
    {
        HandleSourceNodeErrorEvent(aEvent);
    }
    else if (FindNodeTypeByNode(nodeorigin, nodetype, datapathindex) == true)
    {
        if (nodetype == PVP_NODETYPE_SINK)
        {
            HandleSinkNodeErrorEvent(aEvent, datapathindex);
        }
        else if (nodetype == PVP_NODETYPE_DECODER)
        {
            HandleDecNodeErrorEvent(aEvent, datapathindex);
        }
        else
        {
            // Event from unknown node or component. Do nothing but log it
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleNodeErrorEvent() Error event from unknown node type Event type 0x%x Context 0x%x Data 0x%x",
                            aEvent.GetEventType(), aEvent.GetContext(), aEvent.GetEventData()));
        }
    }
    else
    {
        // Event from unknown node or component. Do nothing but log it
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleNodeErrorEvent() Error event from unknown node Event type 0x%x Context 0x%x Data 0x%x",
                        aEvent.GetEventType(), aEvent.GetContext(), aEvent.GetEventData()));
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleNodeErrorEvent() Out"));
}

void PVPlayerEngine::RemoveDatapathContextFromList()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::RemoveDatapathContextFromList(): Erasing from ContextList iCurrentContextList.size() in : %d",
                    iCurrentContextList.size()));
    for (int32 i = iCurrentContextList.size() - 1; i >= 0; --i)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::RemoveDatapathContextFromList(): iCurrentContextList[i]->iCmdType %d",
                        iCurrentContextList[i]->iCmdType));

        switch (iCurrentContextList[i]->iCmdType)
        {
            case PVP_CMD_DPPrepare:
            case PVP_CMD_DPStart:
            case PVP_CMD_DPStop:
            case PVP_CMD_DPTeardown:
            case PVP_CMD_DPReset:
                FreeEngineContext(iCurrentContextList[i]);
                break;
            default:
                break;
        }
    }
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::RemoveDatapathContextFromList(): iCurrentContextList.size() out : %d",
                    iCurrentContextList.size()));
}


void PVPlayerEngine::HandlePlayerDatapathEvent(int32 /*aDatapathEvent*/, PVMFStatus aEventStatus, OsclAny* aContext, PVMFCmdResp* aCmdResp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandlePlayerDatapathEvent() In"));

    // Check if a cancel command completed
    uint32* context_uint32 = (uint32*)aContext;
    if (context_uint32 == &iNumberCancelCmdPending)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandlePlayerDatapathEvent() Cancel in datapath completed for cancel command. Pending %d", iNumberCancelCmdPending));
        --iNumberCancelCmdPending;
        if (iNumberCancelCmdPending == 0)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandlePlayerDatapathEvent() Cancelling of all node/datapath commands complete, now reset all nodes"));
            // Clear the CancelCmd queue as the cmd has been cancelled.
            iCmdToCancel.clear();

            RemoveDatapathContextFromList(); // empty left over contexts from cancelled datapath commands
            // Now reset the source node
            PVPlayerEngineContext* context = AllocateEngineContext(NULL, iSourceNode, NULL, -1, NULL, -1);

            PVMFCommandId cmdid = -1;
            int32 leavecode = 0;
            OSCL_TRY(leavecode, cmdid = iSourceNode->Reset(iSourceNodeSessionId, (OsclAny*)context));
            OSCL_FIRST_CATCH_ANY(leavecode,

                                 PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandlePlayerDatapathEvent() Reset on iSourceNode did a leave!"));
                                 FreeEngineContext(context);
                                 OSCL_ASSERT(false);
                                 return);

            SetEngineState(PVP_ENGINE_STATE_RESETTING);
        }
        return;
    }

    PVPlayerEngineContext* datapathcontext = (PVPlayerEngineContext*)aContext;
    OSCL_ASSERT(datapathcontext);

    // Ignore other datapath event if cancelling
    if (!iCmdToCancel.empty() || (CheckForPendingErrorHandlingCmd() && (aCmdResp && aCmdResp->GetCmdStatus() == PVMFErrCancelled)))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandlePlayerDatapathEvent() Datapath event ignored due to cancel process"));
        // Remove the context from the list
        FreeEngineContext(datapathcontext);
        return;
    }

    // Process the datapath event based on the engine state
    if (iState == PVP_ENGINE_STATE_PREPARING)
    {
        switch (datapathcontext->iCmdType)
        {
            case PVP_CMD_DPPrepare:
                HandleDatapathPrepare(*datapathcontext, aEventStatus, aCmdResp);
                break;

            case PVP_CMD_DPStart:
                HandleDatapathStart(*datapathcontext, aEventStatus, aCmdResp);
                break;

            default:
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandlePlayerDatapathEvent() Invalid datapath command type in PVP_ENGINE_STATE_PREPARING."));
                break;
        }
    }
    else if (iState == PVP_ENGINE_STATE_PAUSING)
    {
        HandleDatapathPause(*datapathcontext, aEventStatus, aCmdResp);
    }
    else if (iState == PVP_ENGINE_STATE_RESUMING)
    {
        HandleDatapathResume(*datapathcontext, aEventStatus, aCmdResp);
    }
    else if (iState == PVP_ENGINE_STATE_STOPPING)
    {
        switch (datapathcontext->iCmdType)
        {
            case PVP_CMD_DPStop:
                HandleDatapathStop(*datapathcontext, aEventStatus, aCmdResp);
                break;

            case PVP_CMD_DPTeardown:
                HandleDatapathTeardown(*datapathcontext, aEventStatus, aCmdResp);
                break;

            case PVP_CMD_DPReset:
                HandleDatapathReset(*datapathcontext, aEventStatus, aCmdResp);
                break;

            default:
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandlePlayerDatapathEvent() Invalid datapath command type in PVP_ENGINE_STATE_STOPPING."));
                break;
        }
    }
    else if (iState == PVP_ENGINE_STATE_RESETTING)
    {
        switch (datapathcontext->iCmdType)
        {
            case PVP_CMD_DPReset:
                HandleDatapathReset(*datapathcontext, aEventStatus, aCmdResp);
                break;

            default:
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandlePlayerDatapathEvent() Invalid datapath command type in PVP_ENGINE_STATE_RESETTING"));
                break;
        }
    }
    else
    {
        // Unknown datapath.
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandlePlayerDatapathEvent() Invalid state for datapath command completion."));
    }

    // Remove the context from the list
    FreeEngineContext(datapathcontext);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandlePlayerDatapathEvent() Out"));
}

void PVPlayerEngine::NotificationsInterfaceDestroyed()
{
    iClockNotificationsInf = NULL;
}

void PVPlayerEngine::ProcessCallBack(uint32 aCallBackID, PVTimeComparisonUtils::MediaTimeStatus aTimerAccuracy, uint32 aDelta,
                                     const OsclAny* acontextData, PVMFStatus aStatus)
{
    OSCL_UNUSED_ARG(aTimerAccuracy);
    OSCL_UNUSED_ARG(aDelta);
    OSCL_UNUSED_ARG(acontextData);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::TimeoutOccurred() Timer for PlayStatus event triggered"));

    if (aCallBackID == iPlayStatusCallbackTimerID)
    {
        //Callback timer needs to be restarted if status is success
        if ((PVMFSuccess == aStatus) && iPlayStatusTimerEnabled)
        {
            SendPositionStatusUpdate();
            iPlayStatusTimerEnabled = false;
            iPlayStatusCallbackTimerID = 0;

            StartPlaybackStatusTimer();
        }
        else
        {
            if (aStatus == PVMFErrCallbackClockStopped)
            {
                iPlayStatusTimerEnabled = false;
                iPlayStatusCallbackTimerID = 0;
            }
        }
    }
}
void PVPlayerEngine::SendPositionStatusUpdate(void)
{
    PVPPlaybackPosition curpos;
    curpos.iPosUnit = iPBPosStatusUnit;
    GetPlaybackClockPosition(curpos);

    uint8 poslocalbuffer[8];
    oscl_memset(poslocalbuffer, 0, 8);
    poslocalbuffer[0] = 1;
    switch (iPBPosStatusUnit)
    {
        case PVPPBPOSUNIT_SEC:
            oscl_memcpy(&poslocalbuffer[4], &(curpos.iPosValue.sec_value), sizeof(uint32));
            break;

        case PVPPBPOSUNIT_MIN:
            oscl_memcpy(&poslocalbuffer[4], &(curpos.iPosValue.min_value), sizeof(uint32));
            break;

        case PVPPBPOSUNIT_MILLISEC:
        default:
            oscl_memcpy(&poslocalbuffer[4], &(curpos.iPosValue.millisec_value), sizeof(uint32));
            break;
    }

    PVUuid puuid = PVPlayerErrorInfoEventTypesUUID;
    PVMFBasicErrorInfoMessage* infomsg = OSCL_NEW(PVMFBasicErrorInfoMessage, (PVPlayerInfoPlaybackPositionStatus, puuid, NULL));
    // EventData parameter will be deprecated, and curpos will not be sent through EventData in future.
    SendInformationalEvent(PVMFInfoPositionStatus, OSCL_STATIC_CAST(PVInterface*, infomsg), (OsclAny*)&curpos, poslocalbuffer, 8);
    infomsg->removeRef();
}


void PVPlayerEngine::TimeoutOccurred(int32 timerID, int32 /*timeoutInfo*/)
{
    if (timerID == PVPLAYERENGINE_TIMERID_ENDTIMECHECK)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::TimeoutOccurred() Timer for EndTime check triggered"));

        PVPPlaybackPosition curpos;
        curpos.iPosUnit = PVPPBPOSUNIT_MILLISEC;
        GetPlaybackClockPosition(curpos);

        if (iCurrentEndPosition.iIndeterminate || iCurrentEndPosition.iPosUnit != PVPPBPOSUNIT_MILLISEC)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::TimeoutOccurred() End time unit is invalid. Disabling end time check."));
            iEndTimeCheckEnabled = false;
            iPollingCheckTimer->Cancel(PVPLAYERENGINE_TIMERID_ENDTIMECHECK);
            OSCL_ASSERT(false);
            return;
        }

        if (curpos.iPosValue.millisec_value >= iCurrentEndPosition.iPosValue.millisec_value)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_NOTICE, (0, "PVPlayerEngine::TimeoutOccurred() Specified end time reached so issuing pause command"));

            iEndTimeCheckEnabled = false;
            iPollingCheckTimer->Cancel(PVPLAYERENGINE_TIMERID_ENDTIMECHECK);
            // Issues end time reached command
            AddCommandToQueue(PVP_ENGINE_COMMAND_PAUSE_DUE_TO_ENDTIME_REACHED, NULL, NULL, NULL, false);
        }
        else if (!iEndTimeCheckEnabled)
        {
            iPollingCheckTimer->Cancel(PVPLAYERENGINE_TIMERID_ENDTIMECHECK);
        }
    }
}

void PVPlayerEngine::RecognizeCompleted(PVMFFormatType aSourceFormatType, OsclAny* aContext)
{
    // Check if a cancel command completed
    uint32* context_uint32 = (uint32*)(aContext);
    if (context_uint32 == &iNumberCancelCmdPending)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::RecognizeCompleted() Recognize request cancelled"));
        --iNumberCancelCmdPending;
        if (iNumberCancelCmdPending == 0)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::RecognizeCompleted() Cancelling of all node/datapath commands complete, now reset all nodes"));
            // Clear the CancelCmd queue as the cmd has been cancelled.
            iCmdToCancel.clear();

            RemoveDatapathContextFromList(); // empty left over contexts from cancelled datapath commands
            // Now reset the source node
            PVPlayerEngineContext* context = AllocateEngineContext(NULL, iSourceNode, NULL, -1, NULL, -1);

            PVMFCommandId cmdid = -1;
            int32 leavecode = 0;
            OSCL_TRY(leavecode, cmdid = iSourceNode->Reset(iSourceNodeSessionId, (OsclAny*)context));
            OSCL_FIRST_CATCH_ANY(leavecode,

                                 PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::RecognizeCompleted() Reset on iSourceNode did a leave!"));
                                 FreeEngineContext(context);
                                 OSCL_ASSERT(false);
                                 return);

            SetEngineState(PVP_ENGINE_STATE_RESETTING);
        }
        return;
    }

    // Ignore recognize completion if cancelling
    if (!iCmdToCancel.empty() || CheckForPendingErrorHandlingCmd())
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::RecognizeCompleted() Recognize completion ignored due to cancel process"));
        // Remove the context from the list
        FreeEngineContext((PVPlayerEngineContext*)(aContext));
        return;
    }

    // Save the recognized source format
    iSourceFormatType = aSourceFormatType;

    // Free the engine context after saving the cmd id and context
    PVPlayerEngineContext* reccontext = (PVPlayerEngineContext*)(aContext);
    OSCL_ASSERT(reccontext != NULL);
    PVCommandId cmdid = reccontext->iCmdId;
    OsclAny* cmdcontext = reccontext->iCmdContext;
    FreeEngineContext(reccontext);

    // Start the source node creation and setup sequence
    PVMFStatus retval = DoSetupSourceNode(cmdid, cmdcontext);

    if (retval != PVMFSuccess)
    {
        bool ehPending = CheckForPendingErrorHandlingCmd();
        if (ehPending)
        {
            // there should be no error handling queued.
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::RecognizeCompleted() Already EH pending, should never happen"));
            return;
        }
        else
        {
            // Queue up Error Handling
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::RecognizeCompleted() DoSetupSourceNode failed, Add EH command"));
            iCommandCompleteStatusInErrorHandling = retval;
            iCommandCompleteErrMsgInErrorHandling = NULL;
            AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_ADD_DATA_SOURCE, NULL, NULL, NULL, false);
        }
        return;
    }
}

//A callback from the threadsafe queue
void PVPlayerEngine::ThreadSafeQueueDataAvailable(ThreadSafeQueue* aQueue)
{
    OSCL_UNUSED_ARG(aQueue);

    //pull all available data off the thread-safe queue and transfer
    //it to the internal queue.
    for (uint32 ndata = 1;ndata;)
    {
        ThreadSafeQueueId id;
        OsclAny* data;
        ndata = iThreadSafeQueue.DeQueue(id, data);
        if (ndata)
        {
            PVPlayerEngineCommand* cmd = (PVPlayerEngineCommand*)data;
            AddCommandToQueue(cmd->iCmdType
                              , cmd->iContextData
                              , &cmd->iParamVector
                              , &cmd->iUuid
                              , true//assume all out-of-thread data is an API command.
                              , (PVCommandId*)&id);//use the command ID that was returned to the caller.
            OSCL_DELETE(cmd);
        }
    }
}

PVMFStatus PVPlayerEngine::DoOOTSyncCommand(int32 aCmdType,
        Oscl_Vector<PVPlayerEngineCommandParamUnion, OsclMemAllocator>* aParamVector,
        const PVUuid* aUuid)
{
    //Called from out-of-thread to perform a synchronous command


    //Add a PVMFStatus* to the end of the command param vec to hold the result.
    PVMFStatus status;
    PVPlayerEngineCommandParamUnion param;
    param.pOsclAny_value = (OsclAny*) & status;
    aParamVector->push_back(param);

    //push the command across the thread boundary
    PVCommandId id = 0;
    PVPlayerEngineCommand* cmd = OSCL_NEW(PVPlayerEngineCommand, (aCmdType, id, NULL, aParamVector));
    if (aUuid)
        cmd->SetUuid(*aUuid);
    iThreadSafeQueue.AddToQueue(cmd);

    //block and wait for completion by engine thread.
    iOOTSyncCommandSem.Wait();
    return status;
}

void PVPlayerEngine::OOTSyncCommandComplete(PVPlayerEngineCommand& aCmd, PVMFStatus aStatus)
{
    //Called in engine thread to complete an out-of-thread synchronous command

    //Put the result status into the last element of the command param vector.
    PVMFStatus* status = (PVMFStatus*)(aCmd.GetParam(aCmd.iParamVector.size() - 1).pOsclAny_value);
    OSCL_ASSERT(status);
    *status = aStatus;

    //Signal the calling thread.
    iOOTSyncCommandSem.Signal();
}

PVCommandId PVPlayerEngine::AddCommandToQueue(int32 aCmdType, OsclAny* aContextData,
        Oscl_Vector<PVPlayerEngineCommandParamUnion, OsclMemAllocator>* aParamVector,
        const PVUuid* aUuid, bool aAPICommand, PVCommandId* aId)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::AddCommandToQueue() In CmdType %d, CmdId %d", aCmdType, iCommandId));

    PVCommandId commandId;
    if (aId)
    {
        //This command is being transferred from the thread-safe queue to the
        //internal queue, in engine thread context.
        //The input ID is the one that was returned to the
        //caller, so use that ID instead of generating a new one.
        commandId = *aId;
    }
    else
    {
        //Generate the next command ID, being careful to avoid thread contention
        //for "iCommandId".
        iCommandIdMut.Lock();
        commandId = iCommandId;
        ++iCommandId;
        if (iCommandId == 0x7FFFFFFF)
        {
            iCommandId = 0;
        }
        iCommandIdMut.Unlock();

        //If this is from outside engine thread context, then push the command across the
        //thread boundary.
        if (!iThreadSafeQueue.IsInThread())
        {
            PVPlayerEngineCommand* cmd = OSCL_NEW(PVPlayerEngineCommand, (aCmdType, commandId, aContextData, aParamVector, aAPICommand));
            if (aUuid)
                cmd->SetUuid(*aUuid);
            iThreadSafeQueue.AddToQueue(cmd, (ThreadSafeQueueId*)&commandId);
            return commandId;
        }
    }

    PVPlayerEngineCommand cmd(aCmdType, commandId, aContextData, aParamVector, aAPICommand);
    if (aUuid)
    {
        cmd.SetUuid(*aUuid);
    }

    int32 leavecode = 0;
    OSCL_TRY(leavecode, iPendingCmds.push(cmd));
    OSCL_FIRST_CATCH_ANY(leavecode,
                         PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::AddCommandToQueue() Adding command to pending command list did a leave!"));
                         OSCL_ASSERT(false);
                         return -1;);

    // if engine needs to queue any error handling command set the engine state as PVP_ENGINE_STATE_ERROR.
    switch (aCmdType)
    {
        case PVP_ENGINE_COMMAND_ERROR_HANDLING_ADD_DATA_SOURCE:
        case PVP_ENGINE_COMMAND_ERROR_HANDLING_INIT:
        case PVP_ENGINE_COMMAND_ERROR_HANDLING_PREPARE:
        case PVP_ENGINE_COMMAND_ERROR_HANDLING_PAUSE:
        case PVP_ENGINE_COMMAND_ERROR_HANDLING_RESUME:
        case PVP_ENGINE_COMMAND_ERROR_HANDLING_SET_PLAYBACK_RANGE:
        case PVP_ENGINE_COMMAND_ERROR_HANDLING_SET_PLAYBACK_RATE:
        case PVP_ENGINE_COMMAND_ERROR_HANDLING_STOP:
        case PVP_ENGINE_COMMAND_ERROR_HANDLING_CANCEL_ALL_COMMANDS:
        case PVP_ENGINE_COMMAND_ERROR_HANDLING_GENERAL:
            SetEngineState(PVP_ENGINE_STATE_ERROR);
            SendInformationalEvent(PVMFInfoErrorHandlingStart, NULL);
            break;

        default:
            break;
    }

    RunIfNotReady();

    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_NOTICE,
                    (0, "PVPlayerEngine::AddCommandToQueue() Type=%d ID=%d APIcmd=%d Tick=%d",
                     aCmdType, cmd.GetCmdId(), aAPICommand, OsclTickCount::TickCount()));

    return cmd.GetCmdId();
}


void PVPlayerEngine::SetEngineState(PVPlayerEngineState aState)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::SetEngineState() In Current state %d, New state %d", iState, aState));
    iState = aState;
}


PVPlayerState PVPlayerEngine::GetPVPlayerState(void)
{
    switch (iState)
    {
        case PVP_ENGINE_STATE_IDLE:
        case PVP_ENGINE_STATE_INITIALIZING:
            return PVP_STATE_IDLE;

        case PVP_ENGINE_STATE_INITIALIZED:
        case PVP_ENGINE_STATE_PREPARING:
        case PVP_ENGINE_STATE_TRACK_SELECTION_1_DONE:
        case PVP_ENGINE_STATE_TRACK_SELECTION_2_DONE:
        case PVP_ENGINE_STATE_TRACK_SELECTION_3_DONE:
            return PVP_STATE_INITIALIZED;

        case PVP_ENGINE_STATE_PREPARED:
        case PVP_ENGINE_STATE_STARTING:
            return PVP_STATE_PREPARED;

        case PVP_ENGINE_STATE_STARTED:
        case PVP_ENGINE_STATE_AUTO_PAUSING:
        case PVP_ENGINE_STATE_AUTO_PAUSED:
        case PVP_ENGINE_STATE_AUTO_RESUMING:
        case PVP_ENGINE_STATE_PAUSING:
        case PVP_ENGINE_STATE_STOPPING:
            return PVP_STATE_STARTED;

        case PVP_ENGINE_STATE_PAUSED:
        case PVP_ENGINE_STATE_RESUMING:
            return PVP_STATE_PAUSED;

        case PVP_ENGINE_STATE_RESETTING:
        {
            bool ehPending = CheckForPendingErrorHandlingCmd();
            if (ehPending)
            {
                return PVP_STATE_ERROR;
            }
            else
            {
                return PVP_STATE_IDLE;
            }
        }

        case PVP_ENGINE_STATE_ERROR:
            return PVP_STATE_ERROR;

        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::GetPVPlayerState() Unknown engine state. Asserting"));
            OSCL_ASSERT(false);
            break;
    }

    return PVP_STATE_ERROR;
}


void PVPlayerEngine::GetPlaybackClockPosition(PVPPlaybackPosition& aClockPos)
{
    bool tmpbool = false;
    uint32 clockcurpos = 0;
    aClockPos.iIndeterminate = false;

    int32 nptcurpos;

    if (!iChangeDirectionNPT.iIndeterminate)
    {
        // report the expected NPT after the direction change
        // to avoid weird transient values between the direction change
        // and the repositioning completion.
        nptcurpos = iChangeDirectionNPT.iPosValue.millisec_value;
    }
    else
    {
        // Get current playback clock position
        iPlaybackClock.GetCurrentTime32(clockcurpos, tmpbool, PVMF_MEDIA_CLOCK_MSEC);

        nptcurpos = iStartNPT + iPlaybackDirection * (clockcurpos - iStartMediaDataTS);
    }
    if (nptcurpos < 0)
    {
        nptcurpos = 0;
    }

    if (ConvertFromMillisec((uint32)nptcurpos, aClockPos) != PVMFSuccess)
    {
        // Other position units are not supported yet
        aClockPos.iIndeterminate = true;
    }
}


PVMFStatus PVPlayerEngine::ConvertToMillisec(PVPPlaybackPosition& aPBPos, uint32& aTimeMS)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_NOTICE, (0, "PVPlayerEngine::ConvertToMillisec() In"));

    if (aPBPos.iIndeterminate)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_NOTICE, (0, "PVPlayerEngine::ConvertToMillisec() Indeterminate position"));
        return PVMFErrArgument;
    }

    bool owallclockunits = false;
    switch (aPBPos.iPosUnit)
    {
        case PVPPBPOSUNIT_MILLISEC:
            aTimeMS = aPBPos.iPosValue.millisec_value;
            owallclockunits = true;
            break;

        case PVPPBPOSUNIT_SEC:
            aTimeMS = aPBPos.iPosValue.sec_value * 1000;
            owallclockunits = true;
            break;

        case PVPPBPOSUNIT_MIN:
            aTimeMS = aPBPos.iPosValue.min_value * 60000;
            owallclockunits = true;
            break;

        case PVPPBPOSUNIT_HOUR:
            aTimeMS = aPBPos.iPosValue.hour_value * 3600000;
            owallclockunits = true;
            break;

        case PVPPBPOSUNIT_SMPTE:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::ConvertToMillisec() SMPTE not supported yet"));
            return PVMFErrArgument;

        case PVPPBPOSUNIT_PERCENT:
        {
            if (iSourceDurationAvailable == false)
            {
                // Duration info not available from source node so can't convert
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::ConvertToMillisec() Duration not available so can't convert"));
                return PVMFErrArgument;
            }

            if (iSourceDurationInMS == 0)
            {
                // Duration is 0 so can't convert
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::ConvertToMillisec() Duration value is 0 so can't convert"));
                return PVMFErrArgument;
            }

            if (aPBPos.iPosValue.percent_value >= 100)
            {
                // If percentage greater than 100, cap to 100%
                aTimeMS = iSourceDurationInMS;
            }
            else
            {
                // Calculate time in millseconds based on percentage of duration
                aTimeMS = (aPBPos.iPosValue.percent_value * iSourceDurationInMS) / 100;
            }
        }
        break;

        case PVPPBPOSUNIT_SAMPLENUMBER:
        {
            if (iSourceNodeTrackLevelInfoIF == NULL)
            {
                // The source node doesn't have the query IF to convert samplenum to time
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::ConvertToMillisec() Sample number to time conversion not available"));
                return PVMFErrArgument;
            }

            // Determine which track to use for conversion.
            // Give preference to video track, then text, and finally audio
            PVMFTrackInfo* track = NULL;
            int32 datapathIndex = -1;

            // Search from the datapath list.
            // 1) Try for video track
            bool retVal = FindDatapathForTrackUsingMimeString(true, false, false, datapathIndex);
            if (retVal == false)
            {
                // Video track not available, look for text track
                retVal = FindDatapathForTrackUsingMimeString(false, false, true, datapathIndex);
                if (retVal == false)
                {
                    // Text track also not avaliable, look for audio track
                    retVal = FindDatapathForTrackUsingMimeString(false, true, false, datapathIndex);
                    if (retVal == false)
                    {
                        // Track is not available to do the conversion
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::ConvertToMillisec() Track not selected for conversion"));
                        return PVMFErrArgument;
                    }
                }
            }

            // Track avalaible.
            track = iDatapathList[datapathIndex].iTrackInfo;

            // Convert the sample number to time in milliseconds
            PVMFTimestamp framets = 0;
            if (iSourceNodeTrackLevelInfoIF->GetTimestampForSampleNumber(*track, aPBPos.iPosValue.samplenum_value, framets) != PVMFSuccess)
            {
                // Conversion failed
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::ConvertToMillisec() Sample number to time conversion failed"));
                return PVMFErrArgument;
            }

            aTimeMS = framets;
        }
        break;

        case PVPPBPOSUNIT_DATAPOSITION:
        {
            if (iSourceNodeTrackLevelInfoIF == NULL)
            {
                // The source node doesn't have the ext IF to convert data position to time
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::ConvertToMillisec() Data position to time conversion not available"));
                return PVMFErrArgument;
            }

            // Go through each active track and find the minimum time for given data position
            bool mintsvalid = false;
            PVMFTimestamp mints = 0xFFFFFFFF;
            for (uint32 i = 0; i < iDatapathList.size(); ++i)
            {
                if (iDatapathList[i].iDatapath)
                {
                    PVMFTimestamp curts = 0;
                    if (iSourceNodeTrackLevelInfoIF->GetTimestampForDataPosition(*(iDatapathList[i].iTrackInfo), aPBPos.iPosValue.datapos_value, curts) != PVMFSuccess)
                    {
                        // Conversion failed
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::ConvertToMillisec() Data position to time conversion failed"));
                    }
                    else
                    {
                        // Conversion succeeded. Save only if it is the minimum encountered so far.
                        mintsvalid = true;
                        if (curts < mints)
                        {
                            mints = curts;
                        }
                    }
                }
            }

            if (mintsvalid == false)
            {
                // Conversion on all active tracks failed
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::ConvertToMillisec() Data position to time conversion could not be done on any active track"));
                return PVMFErrArgument;
            }

            aTimeMS = mints;
        }
        break;

        case PVPPBPOSUNIT_PLAYLIST:
        {
            switch (aPBPos.iPlayListPosUnit)
            {
                case PVPPBPOSUNIT_MILLISEC:
                    aTimeMS = aPBPos.iPlayListPosValue.millisec_value;
                    break;

                case PVPPBPOSUNIT_SEC:
                    aTimeMS = aPBPos.iPlayListPosValue.sec_value * 1000;
                    break;

                case PVPPBPOSUNIT_MIN:
                    aTimeMS = aPBPos.iPlayListPosValue.min_value * 60000;
                    break;

                case PVPPBPOSUNIT_HOUR:
                    aTimeMS = aPBPos.iPlayListPosValue.hour_value * 3600000;
                    break;

                default:
                    // Don't support the other units for now
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::ConvertToMillisec() Unsupported playlist position units"));
                    return PVMFErrArgument;
            }
        }
        break;

        default:
            // Don't support the other units for now
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::ConvertToMillisec() Unsupported position units"));
            return PVMFErrArgument;
    }

    if (owallclockunits == true)
    {
        if ((aTimeMS > iSourceDurationInMS) && (iSourceDurationAvailable == true))
        {
            //cap time to clip duration
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "PVPlayerEngine::ConvertToMillisec() Capping value - Acutal=%d, CappedValue=%d",
                            aTimeMS, iSourceDurationInMS));
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iReposLogger, PVLOGMSG_DEBUG, (0, "PVPlayerEngine::ConvertToMillisec() Capping value - Acutal=%d, CappedValue=%d",
                            aTimeMS, iSourceDurationInMS));
            aTimeMS = iSourceDurationInMS;
        }
        else
        {
            // just pass the converted time even if duration is not available and let
            // source node handle the request.
        }
    }

    if (aPBPos.iPosUnit == PVPPBPOSUNIT_PLAYLIST)
    {
        aPBPos.iPlayListPosValue.millisec_value = aTimeMS;
        aPBPos.iPlayListPosUnit = PVPPBPOSUNIT_MILLISEC;
    }
    else
    {
        aPBPos.iPosValue.millisec_value = aTimeMS;
        aPBPos.iPosUnit = PVPPBPOSUNIT_MILLISEC;
    }
    iTargetNPT = aTimeMS;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::ConvertToMillisec() Out"));
    return PVMFSuccess;
}


PVMFStatus PVPlayerEngine::ConvertFromMillisec(uint32 aTimeMS, PVPPlaybackPosition& aPBPos)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::ConvertFromMillisec() In"));

    // Convert to specified time units
    switch (aPBPos.iPosUnit)
    {
        case PVPPBPOSUNIT_MILLISEC:
            aPBPos.iPosValue.millisec_value = aTimeMS;
            break;

        case PVPPBPOSUNIT_SEC:
            aPBPos.iPosValue.sec_value = aTimeMS / 1000;
            break;

        case PVPPBPOSUNIT_MIN:
            aPBPos.iPosValue.min_value = aTimeMS / 60000;
            break;

        case PVPPBPOSUNIT_HOUR:
            aPBPos.iPosValue.hour_value = aTimeMS / 3600000;
            break;

        case PVPPBPOSUNIT_SMPTE:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_NOTICE, (0, "PVPlayerEngine::ConvertFromMillisec() SMPTE units not supported yet"));
            return PVMFErrArgument;

        case PVPPBPOSUNIT_PERCENT:
        {
            if (iSourceDurationAvailable == false)
            {
                // Duration info not available from source node so can't convert
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::ConvertFromMillisec() Duration not available so can't convert"));
                return PVMFErrArgument;
            }

            if (iSourceDurationInMS == 0)
            {
                // Duration is 0 so can't convert
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::ConvertFromMillisec() Duration value is 0 so can't convert"));
                return PVMFErrArgument;
            }

            if (aTimeMS >= iSourceDurationInMS)
            {
                // Put a ceiling of 100%
                aPBPos.iPosValue.percent_value = 100;
            }
            else
            {
                // Calculate percentage of playback, avoiding overflow
                if (iSourceDurationInMS >= PVP_MIN_PLAYSTATUS_PERCENT_OVERFLOW_THRESHOLD)
                {
                    aPBPos.iPosValue.percent_value = aTimeMS / (iSourceDurationInMS / 100);
                }
                else
                {
                    aPBPos.iPosValue.percent_value = (aTimeMS * 100) / iSourceDurationInMS;
                }
            }
        }
        break;

        case PVPPBPOSUNIT_SAMPLENUMBER:
        {
            if (iSourceNodeTrackLevelInfoIF == NULL)
            {
                // The source node doesn't have the query IF to convert time to sample number
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::ConvertFromMillisec() Time to sample numberconversion not available"));
                return PVMFErrArgument;
            }

            // Determine which track to use for conversion.
            // Give preference to video track, then text, and finally audio
            PVMFTrackInfo* track = NULL;
            int32 datapathIndex = -1;

            // Search from the datapath list.
            // 1) Try for video track
            bool retVal = FindDatapathForTrackUsingMimeString(true, false, false, datapathIndex);
            if (retVal == false)
            {
                // Video track not available, look for text track
                retVal = FindDatapathForTrackUsingMimeString(false, false, true, datapathIndex);
                if (retVal == false)
                {
                    // Text track also not avaliable, look for audio track
                    retVal = FindDatapathForTrackUsingMimeString(false, true, false, datapathIndex);
                    if (retVal == false)
                    {
                        // Track is not available to do the conversion
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::ConvertToMillisec() Track not selected for conversion"));
                        return PVMFErrArgument;
                    }
                }
            }

            // Track avalaible.
            track = iDatapathList[datapathIndex].iTrackInfo;

            // Convert the time to sample number
            PVMFTimestamp ts = aTimeMS;
            uint32 samplenum = 0;
            if (iSourceNodeTrackLevelInfoIF->GetSampleNumberForTimestamp(*track, ts, samplenum) != PVMFSuccess)
            {
                // Conversion failed
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::ConvertFromMillisec() Sample number to time conversion failed"));
                return PVMFErrArgument;
            }

            aPBPos.iPosValue.samplenum_value = samplenum;
        }
        break;

        case PVPPBPOSUNIT_DATAPOSITION:
        {
            if (iSourceNodeTrackLevelInfoIF == NULL)
            {
                // The source node doesn't have the ext IF to convert time to data position
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::ConvertFromMillisec() Time to data position conversion not available in source node"));
                return PVMFErrArgument;
            }

            // Query each active track for its data position
            // Return the max data position
            PVMFTimestamp ts = aTimeMS;
            uint32 maxdatapos = 0;
            bool maxdataposvalid = false;

            // Go through each active track
            for (uint32 i = 0; i < iDatapathList.size(); ++i)
            {
                if (iDatapathList[i].iDatapath)
                {
                    uint32 curdatapos = 0;
                    // Convert the time to data position
                    if (iSourceNodeTrackLevelInfoIF->GetDataPositionForTimestamp(*(iDatapathList[i].iTrackInfo), ts, curdatapos) != PVMFSuccess)
                    {
                        // Conversion failed
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::ConvertFromMillisec() Time to data position conversion failed"));
                    }
                    else
                    {
                        // Save the data position if it is greater than
                        // any position encountered so far.
                        maxdataposvalid = true;
                        if (curdatapos > maxdatapos)
                        {
                            maxdatapos = curdatapos;
                        }
                    }
                }
            }

            if (maxdataposvalid == false)
            {
                // Conversion failed for all active tracks
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::ConvertFromMillisec() Track not selected for conversion"));
                return PVMFErrArgument;
            }
            // Save the data position to return
            aPBPos.iPosValue.datapos_value = maxdatapos;
        }
        break;;

        default:
            // Don't support the other units for now
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_NOTICE, (0, "PVPlayerEngine::ConvertFromMillisec() Unsupported position units"));
            return PVMFErrArgument;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::ConvertFromMillisec() Out"));
    return PVMFSuccess;
}


void PVPlayerEngine::EngineCommandCompleted(PVCommandId aId, OsclAny* aContext, PVMFStatus aStatus, PVInterface* aExtInterface, OsclAny* aEventData, int32 aEventDataSize)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::EngineCommandCompleted() In CmdId %d, Status %d", aId, aStatus));

    // Update the current command vector

    // Assert if the current cmd is not saved or the cmd ID does not match
    OSCL_ASSERT(iCurrentCmd.size() == 1);
    OSCL_ASSERT(iCurrentCmd[0].GetCmdId() == aId);

    // Empty out the current cmd vector and set active if there are other pending commands
    PVPlayerEngineCommand completedcmd(iCurrentCmd[0]);
    iCurrentCmd.erase(iCurrentCmd.begin());
    if (!iPendingCmds.empty())
    {
        RunIfNotReady();
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_NOTICE,
                    (0, "PVPlayerEngine::EngineCommandCompleted() Type=%d ID=%d APIcmd=%d Tick=%d",
                     completedcmd.GetCmdType(), completedcmd.GetCmdId(), completedcmd.IsAPICommand(), OsclTickCount::TickCount()));

    // Send informational event or send other callback if needed
    switch (completedcmd.GetCmdType())
    {
        case PVP_ENGINE_COMMAND_PAUSE_DUE_TO_ENDOFCLIP:
            SendEndOfClipInfoEvent(aStatus, aExtInterface);
            break;

        case PVP_ENGINE_COMMAND_PAUSE_DUE_TO_ENDTIME_REACHED:
            SendEndTimeReachedInfoEvent(aStatus, aExtInterface);
            break;

        case PVP_ENGINE_COMMAND_PAUSE_DUE_TO_BUFFER_UNDERFLOW:
            SendSourceUnderflowInfoEvent(aStatus, aExtInterface);
            break;

        case PVP_ENGINE_COMMAND_RESUME_DUE_TO_BUFFER_DATAREADY:
            SendSourceDataReadyInfoEvent(aStatus, aExtInterface);
            break;

        case PVP_ENGINE_COMMAND_CAPCONFIG_SET_PARAMETERS:
            // Send callback to the specified observer
            if (iCfgCapCmdObserver)
            {
                iCfgCapCmdObserver->SignalEvent(aId);
            }
            break;

        default:
            // None to be sent
            break;
    }

    // Send the command completed event
    if (iCmdStatusObserver)
    {
        if (aId != -1 && completedcmd.IsAPICommand())
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::EngineCommandCompleted() Notifying engine command as completed. CmdId %d Status %d", aId, aStatus));
            PVCmdResponse cmdcompleted(aId, aContext, aStatus, aExtInterface, aEventData, aEventDataSize);
            iCmdStatusObserver->CommandCompleted(cmdcompleted);
        }
        else
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::EngineCommandCompleted() aId is -1 or not an API command. CmdType %d", completedcmd.GetCmdType()));
        }
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::EngineCommandCompleted() iCmdStatusObserver is NULL"));
    }
}


void PVPlayerEngine::SendInformationalEvent(PVMFEventType aEventType, PVInterface* aExtInterface, OsclAny* aEventData, uint8* aLocalBuffer, uint32 aLocalBufferSize)
{
    // Send the info event if observer has been specified
    if (iInfoEventObserver)
    {
        PVAsyncInformationalEvent infoevent((PVEventType)aEventType, NULL, aExtInterface, (PVExclusivePtr)aEventData, aLocalBuffer, aLocalBufferSize);
        iInfoEventObserver->HandleInformationalEvent(infoevent);
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::SendInformationalEvent() iInfoEventObserver is NULL"));
    }
}


void PVPlayerEngine::SendErrorEvent(PVMFEventType aEventType, PVInterface* aExtInterface, OsclAny* aEventData, uint8* aLocalBuffer, uint32 aLocalBufferSize)
{
    // Send the error event if observer has been specified
    if (iErrorEventObserver)
    {
        PVAsyncErrorEvent errorevent((PVEventType)aEventType, NULL, aExtInterface, (PVExclusivePtr)aEventData, aLocalBuffer, aLocalBufferSize);
        iErrorEventObserver->HandleErrorEvent(errorevent);
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::SendErrorEvent() iErrorEventObserver is NULL"));
    }
}


void PVPlayerEngine::DoCancelCommand(PVPlayerEngineCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoCancelCommand() In"));

    // Boolean to check if the command is cancelled or not.
    bool commandCancelled = false;

    // cmd to cancel either has been completed or is in pending queue.
    // Create a temporary queue for pending commands and current command if any.
    OsclPriorityQueue<PVPlayerEngineCommand, OsclMemAllocator, Oscl_Vector<PVPlayerEngineCommand, OsclMemAllocator>, PVPlayerEngineCommandCompareLess> iTempPendingCmds;
    Oscl_Vector<PVPlayerEngineCommand, OsclMemAllocator> iTempCurrentCmd;
    // Copy the pending commands to the new queue
    iTempPendingCmds = iPendingCmds;
    while (!iTempPendingCmds.empty())
    {
        // Get the queue from the top
        PVPlayerEngineCommand cmd(iTempPendingCmds.top());
        // Check if it needs to be cancelled
        if (aCmd.GetParam(0).int32_value == cmd.GetCmdId())
        {
            // Found command to be cancelled in the Pending Queue, set the
            // commandCancelled boolean to true.
            commandCancelled = true;

            // Remove it from the pending commands queue
            iPendingCmds.remove(cmd);
            // Save it temporary as "current command" and then cancel it. If CurrentCmd has some
            // command, first move it to TempCurrentCmd queue.
            if (!iCurrentCmd.empty())
            {
                iTempCurrentCmd.push_front(iCurrentCmd[0]);
                iCurrentCmd.erase(iCurrentCmd.begin());
            }

            iCurrentCmd.push_front(cmd);
            EngineCommandCompleted(cmd.GetCmdId(), cmd.GetContext(), PVMFErrCancelled);

            // send command complete for CancelCommand also.
            iCurrentCmd.push_front(aCmd);
            EngineCommandCompleted(aCmd.GetCmdId(), aCmd.GetContext(), PVMFSuccess);

            // If TempCurrentCmd queue is holding up any command, move it back to CurrentCmd queue.
            if (!iTempCurrentCmd.empty())
            {
                iCurrentCmd.push_front(iTempCurrentCmd[0]);
                iTempCurrentCmd.erase(iTempCurrentCmd.begin());
            }
        }
        // Pop each cmd from the temporary queue
        iTempPendingCmds.pop();
    }

    if (!commandCancelled)
    {
        // There was no command cancelled, user might have given a wrong Argument
        // Fail the command with PVMFErrArgument
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoCancelCommand() Wrong Argument, No comand cancelled"));
        if (!iCurrentCmd.empty())
        {
            PVPlayerEngineCommand currentcmd(iCurrentCmd[0]);
            iCurrentCmd.erase(iCurrentCmd.begin());
            iCurrentCmd.push_front(aCmd);
            EngineCommandCompleted(aCmd.GetCmdId(), aCmd.GetContext(), PVMFErrArgument);
            iCurrentCmd.push_front(currentcmd);
        }
        else
        {
            // Current Command is empty, just push CancelCommand and do Command Complete.
            iCurrentCmd.push_front(aCmd);
            EngineCommandCompleted(aCmd.GetCmdId(), aCmd.GetContext(), PVMFErrArgument);
        }
    }
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoCancelCommand() Out"));
}


void PVPlayerEngine::DoCancelAllCommands(PVPlayerEngineCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoCancelAllCommands() In"));


    // Engine cannot be processing another cancel command
    OSCL_ASSERT(iCmdToCancel.empty() == true);

    // While AcquireLicense and CancelAcquireLicense is processing, CancelAllCommands is prohibited.
    if (!iCmdToDlaCancel.empty() ||
            (!iCurrentCmd.empty() &&
             (iCurrentCmd[0].GetCmdType() == PVP_ENGINE_COMMAND_CANCEL_ACQUIRE_LICENSE ||
              iCurrentCmd[0].GetCmdType() == PVP_ENGINE_COMMAND_ACQUIRE_LICENSE_WCHAR ||
              iCurrentCmd[0].GetCmdType() == PVP_ENGINE_COMMAND_ACQUIRE_LICENSE_CHAR)))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoCancelAllCommands() Error due to processing AcquireLicense or CancelAcquireLicense,CmdType=%d", iCurrentCmd[0].GetCmdType()));
        PVPlayerEngineCommand currentcmd(iCurrentCmd[0]);
        iCurrentCmd.erase(iCurrentCmd.begin());
        iCurrentCmd.push_front(aCmd);
        EngineCommandCompleted(aCmd.GetCmdId(), aCmd.GetContext(), PVMFErrArgument);
        iCurrentCmd.push_front(currentcmd);
        return;
    }
    // set engine state to Resetting, as cancel command completion will take Engine to Idle state, after internal reset.
    SetEngineState(PVP_ENGINE_STATE_RESETTING);
    iRollOverState = RollOverStateIdle; //reset roll over state to Idle, as engine is resetting itself
    // Stop the playback clock
    iPlaybackClock.Stop();
    // Cancel the current command first
    if (iCurrentCmd.size() == 1)
    {
        // First save the current command being processed
        iCmdToCancel.push_front(iCurrentCmd[0]);
        // Cancel it
        EngineCommandCompleted(iCurrentCmd[0].GetCmdId(), iCurrentCmd[0].GetContext(), PVMFErrCancelled);
    }

    // Cancel all the pending commands

    // Create a temporary queue for pending commands
    OsclPriorityQueue<PVPlayerEngineCommand, OsclMemAllocator, Oscl_Vector<PVPlayerEngineCommand, OsclMemAllocator>, PVPlayerEngineCommandCompareLess> iTempPendingCmds;
    // Copy the pending commands to the new queue
    iTempPendingCmds = iPendingCmds;
    while (!iTempPendingCmds.empty())
    {
        // Get the queue from the top
        PVPlayerEngineCommand cmd(iTempPendingCmds.top());
        // Check if it needs to be cancelled
        if ((aCmd.GetCmdId() > cmd.GetCmdId()) && !((aCmd.GetCmdId() - cmd.GetCmdId()) > 0x7FFFFFFF))
        {
            // Remove it from the pending commands queue
            iPendingCmds.remove(cmd);
            // Save it temporary as "current command" and then cancel it
            iCurrentCmd.push_front(cmd);
            EngineCommandCompleted(cmd.GetCmdId(), cmd.GetContext(), PVMFErrCancelled);
        }
        // Pop each cmd from the temporary queue
        iTempPendingCmds.pop();
    }


    // Make the CancelAll() command the current command
    iCurrentCmd.push_front(aCmd);

    // Check if there was an ongoing command that needs to be properly cancelled
    if (!iCmdToCancel.empty())
    {

        // Properly cancel a command being currently processed
        DoCancelCommandBeingProcessed();
    }
    else
    {
        // Nothing to cancel, move on to resetting Source Nodes and Datapaths
        if (iSourceNode)
        {
            int32 leavecode = 0;
            // call reset on source node if not in created state
            if (iSourceNode->GetState() != EPVMFNodeCreated)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVPlayerEngine::DoCancelAllCommands() Issue reset on Source Node"));
                // error handling code set engine state to resetting
                SetEngineState(PVP_ENGINE_STATE_RESETTING);

                PVPlayerEngineContext* context = AllocateEngineContext(NULL, iSourceNode, NULL, -1, NULL, -1);

                PVMFCommandId cmdid = -1;
                leavecode = 0;
                OSCL_TRY(leavecode, cmdid = iSourceNode->Reset(iSourceNodeSessionId, (OsclAny*)context));
                OSCL_FIRST_CATCH_ANY(leavecode,

                                     PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                                     (0, "PVPlayerEngine::DoCancelAllCommands() Reset on iSourceNode did a leave!"));
                                     FreeEngineContext(context);
                                     OSCL_ASSERT(false););

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoCancelAllCommands() Out"));
                return;
            }
        }

        if (iDataSource)
        {
            RemoveDataSourceSync(*iDataSource);
        }
        SetEngineState(PVP_ENGINE_STATE_IDLE);
        EngineCommandCompleted(aCmd.GetCmdId(), aCmd.GetContext(), PVMFSuccess);
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoCancelAllCommands() Out"));
}


void PVPlayerEngine::DoCancelCommandBeingProcessed(void)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoCancelCommandBeingProcessed() In"));

    // There should be a command to cancel
    OSCL_ASSERT(iCmdToCancel.empty() == false);

    // If cmd to cancel is GetMetadataKeys() or GetMetadataValues(), first release the memory for
    // nodes which have already completed the call and then issue cancel on other nodes.
    switch (iCmdToCancel[0].GetCmdType())
    {
        case PVP_ENGINE_COMMAND_GET_METADATA_KEY:
        {
            // Release the memory allocated for the metadata keys
            while (iMetadataKeyReleaseList.empty() == false)
            {
                PVMFMetadataExtensionInterface* mdif = iMetadataIFList[iMetadataKeyReleaseList[0].iMetadataIFListIndex].iInterface;
                OSCL_ASSERT(mdif != NULL);
                mdif->ReleaseNodeMetadataKeys(*(iGetMetadataKeysParam.iKeyList), iMetadataKeyReleaseList[0].iStartIndex, iMetadataKeyReleaseList[0].iEndIndex);
                iMetadataKeyReleaseList.erase(iMetadataKeyReleaseList.begin());
            }
            // no need to break from the current switch, as we need to issue Cancel on nodes. Continue.
        }
        case PVP_ENGINE_COMMAND_GET_METADATA_VALUE:
        {
            // Release the memory allocated for the metadata values
            while (iMetadataValueReleaseList.empty() == false)
            {
                PVMFMetadataExtensionInterface* mdif = iMetadataIFList[iMetadataValueReleaseList[0].iMetadataIFListIndex].iInterface;
                OSCL_ASSERT(mdif != NULL);
                mdif->ReleaseNodeMetadataValues(*(iGetMetadataValuesParam.iValueList), iMetadataValueReleaseList[0].iStartIndex, iMetadataValueReleaseList[0].iEndIndex);
                iMetadataValueReleaseList.erase(iMetadataValueReleaseList.begin());
            }

            iReleaseMetadataValuesPending = false;
            // no need to break from the current switch, as we need to issue Cancel on nodes. Continue.
        }
        case PVP_ENGINE_COMMAND_ADD_DATA_SOURCE:
        case PVP_ENGINE_COMMAND_INIT:
        case PVP_ENGINE_COMMAND_PREPARE:
        case PVP_ENGINE_COMMAND_PAUSE:
        case PVP_ENGINE_COMMAND_RESUME:
        case PVP_ENGINE_COMMAND_SET_PLAYBACK_RANGE:
        case PVP_ENGINE_COMMAND_SET_PLAYBACK_RATE:
        case PVP_ENGINE_COMMAND_CANCEL_COMMAND:
        case PVP_ENGINE_COMMAND_PAUSE_DUE_TO_ENDTIME_REACHED:
        case PVP_ENGINE_COMMAND_PAUSE_DUE_TO_ENDOFCLIP:
        case PVP_ENGINE_COMMAND_PAUSE_DUE_TO_BUFFER_UNDERFLOW:
        case PVP_ENGINE_COMMAND_RESUME_DUE_TO_BUFFER_DATAREADY:
        case PVP_ENGINE_COMMAND_STOP:
        {
            // go ahead and issue cancel on nodes and datapath if needed.
            if (!iCurrentContextList.empty())
            {
                // Since there is a pending node or datapath, cancel it
                PVMFStatus status = DoCancelPendingNodeDatapathCommand();
                if (status == PVMFPending)
                {
                    // There are some commands which need to be cancelled so wait for cancel complete
                    // once cancels complete, we would start the reset sequence from either
                    // NodeCommandComplete, HandlePlayerDataPathEvent, RecognizeComplete
                    break;
                }
            }

            // No pending command so reset the nodes now
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoCancelCommandBeingProcessed() No command to cancel, now reset all nodes"));

            // reset the source node
            PVPlayerEngineContext* context = AllocateEngineContext(NULL, iSourceNode, NULL, -1, NULL, -1);

            PVMFCommandId cmdid = -1;
            int32 leavecode = 0;
            OSCL_TRY(leavecode, cmdid = iSourceNode->Reset(iSourceNodeSessionId, (OsclAny*)context));
            OSCL_FIRST_CATCH_ANY(leavecode,

                                 PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoCancelCommandBeingProcessed() Reset on iSourceNode did a leave!"));
                                 FreeEngineContext(context);
                                 OSCL_ASSERT(false);
                                 return);

            SetEngineState(PVP_ENGINE_STATE_RESETTING);
            break;
        }

        case PVP_ENGINE_COMMAND_QUERY_UUID:
        case PVP_ENGINE_COMMAND_QUERY_INTERFACE:
        case PVP_ENGINE_COMMAND_GET_PLAYBACK_RANGE:
        case PVP_ENGINE_COMMAND_GET_PLAYBACK_RATE:
        case PVP_ENGINE_COMMAND_GET_PLAYBACK_MINMAX_RATE:
        case PVP_ENGINE_COMMAND_GET_SDK_INFO:
        case PVP_ENGINE_COMMAND_GET_SDK_MODULE_INFO:
        case PVP_ENGINE_COMMAND_SET_LOG_APPENDER:
        case PVP_ENGINE_COMMAND_REMOVE_LOG_APPENDER:
        case PVP_ENGINE_COMMAND_SET_LOG_LEVEL:
        case PVP_ENGINE_COMMAND_GET_LOG_LEVEL:
        case PVP_ENGINE_COMMAND_CANCEL_ALL_COMMANDS:
        case PVP_ENGINE_COMMAND_GET_PVPLAYER_STATE:
        case PVP_ENGINE_COMMAND_ADD_DATA_SINK:
        case PVP_ENGINE_COMMAND_GET_CURRENT_POSITION:
        case PVP_ENGINE_COMMAND_START:
        case PVP_ENGINE_COMMAND_REMOVE_DATA_SINK:
        case PVP_ENGINE_COMMAND_REMOVE_DATA_SOURCE:
        default:
            // These commands should complete in one AO scheduling so there should be no need to cancel.
            // CancelAll() is done so complete it
            EngineCommandCompleted(iCurrentCmd[0].GetCmdId(), iCurrentCmd[0].GetContext(), PVMFSuccess);
            break;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoCancelCommandBeingProcessed() Out"));
}


PVMFStatus PVPlayerEngine::DoCancelPendingNodeDatapathCommand()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoCancelPendingNodeDatapathCommand() In"));

    OSCL_ASSERT(iCurrentContextList.empty() == false);

    // Determine where the pending commands were issued to and then cancel them
    int32 leavecode = 0;
    iNumberCancelCmdPending = 0;
    for (uint32 i = 0; i < iCurrentContextList.size(); ++i)
    {
        if (iCurrentContextList[i]->iNode)
        {
            if (iCurrentContextList[i]->iNode == iSourceNode)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoCancelPendingNodeDatapathCommand() Calling CancelAllCommands() on source node"));
                leavecode = IssueNodeCancelCommand(iCurrentContextList[i], iSourceNodeSessionId, (OsclAny*) & iNumberCancelCmdPending);
                if (leavecode == 0)
                {
                    ++iNumberCancelCmdPending;
                }
                else
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "PVPlayerEngine::DoCancelPendingNodeDatapathCommand() CancelAllCommands() on source node did a leave"));
                    FreeEngineContext(iCurrentContextList[i]);
                }
            }
            else if (iCurrentContextList[i]->iEngineDatapath != NULL)
            {
                if (iCurrentContextList[i]->iNode == iCurrentContextList[i]->iEngineDatapath->iSinkNode)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoCancelPendingNodeDatapathCommand() Calling CancelAllCommands() on sink node"));
                    leavecode = IssueNodeCancelCommand(iCurrentContextList[i], iCurrentContextList[i]->iEngineDatapath->iSinkNodeSessionId, (OsclAny*) & iNumberCancelCmdPending);
                    if (leavecode == 0)
                    {
                        ++iNumberCancelCmdPending;
                    }
                    else
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                        (0, "PVPlayerEngine::DoCancelPendingNodeDatapathCommand() CancelAllCommands() on sink node did a leave"));
                        FreeEngineContext(iCurrentContextList[i]);
                    }
                }
                else if (iCurrentContextList[i]->iNode == iCurrentContextList[i]->iEngineDatapath->iDecNode)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoCancelPendingNodeDatapathCommand() Calling CancelAllCommands() on dec node"));
                    leavecode = IssueNodeCancelCommand(iCurrentContextList[i], iCurrentContextList[i]->iEngineDatapath->iDecNodeSessionId, (OsclAny*) & iNumberCancelCmdPending);
                    if (leavecode == 0)
                    {
                        ++iNumberCancelCmdPending;
                    }
                    else
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                        (0, "PVPlayerEngine::DoCancelPendingNodeDatapathCommand() CancelAllCommands() on dec node did a leave"));
                        FreeEngineContext(iCurrentContextList[i]);
                    }
                }
                else
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoCancelPendingNodeDatapathCommand() Unknown node type. Asserting"));
                    OSCL_ASSERT(false);
                }
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoCancelPendingNodeDatapathCommand() Unknown node. Asserting"));
                OSCL_ASSERT(false);
            }
        }
        else if (iCurrentContextList[i]->iDatapath != NULL)
        {
            if (iCurrentContextList[i]->iEngineDatapath != NULL)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoCancelPendingNodeDatapathCommand() Calling CancelAllCommands() on datapath"));
                leavecode = IssueDatapathCancelCommand(iCurrentContextList[i], (OsclAny*) & iNumberCancelCmdPending);
                if (leavecode == 0)
                {
                    ++iNumberCancelCmdPending;
                }
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoCancelPendingNodeDatapathCommand() Unknown datapath. Asserting"));
                OSCL_ASSERT(false);
            }
        }
        else if (iCurrentContextList[i]->iCmdType == PVP_CMD_QUERYSOURCEFORMATTYPE)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoCancelPendingNodeDatapathCommand() Calling CancelAllCommands() on recognizer"));
            leavecode = IssueRecognizerRegistryCancel((OsclAny*) & iNumberCancelCmdPending);
            if (leavecode == 0)
            {
                ++iNumberCancelCmdPending;
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVPlayerEngine::DoCancelPendingNodeDatapathCommand() CancelAllCommands() on recognizer node did a leave"));
                FreeEngineContext(iCurrentContextList[i]);
            }
        }
        else
        {
            // Either a node or datapath should be pending
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoCancelPendingNodeDatapathCommand() No pending node or datapath. Asserting"));
            OSCL_ASSERT(false);
        }
    }

    if (iNumberCancelCmdPending == 0)
    {
        // Cancel on the node / datapath failed so go to next step
        // Note that we do not care about not being able to queue cancel since
        // we are going to reset the components anyway
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoCancelPendingNodeDatapathCommand() CancelAllCommands() on the node did a leave"));
        RemoveDatapathContextFromList(); // remove left-over datapath contexts
        return PVMFSuccess;
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoCancelPendingNodeDatapathCommand() %d CancelAllCommands are pending", iNumberCancelCmdPending));
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoCancelPendingNodeDatapathCommand() Out"));
    return PVMFPending;
}

PVMFStatus PVPlayerEngine::DoGetSDKInfo(PVPlayerEngineCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoGetSDKInfo() In"));

    PVSDKInfo* sdkinfo = (PVSDKInfo*)(aCmd.GetParam(0).pOsclAny_value);
    if (sdkinfo == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoGetSDKInfo() Passed in parameter invalid."));
        return PVMFErrArgument;
    }

    // Set the SDK info to the ones defined in the header file pv_player_sdkinfo.h generated at build time
    sdkinfo->iLabel = PVPLAYER_ENGINE_SDKINFO_LABEL;
    sdkinfo->iDate = PVPLAYER_ENGINE_SDKINFO_DATE;

    EngineCommandCompleted(iCurrentCmd[0].GetCmdId(), iCurrentCmd[0].GetContext(), PVMFSuccess);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoGetSDKInfo() Out"));
    return PVMFSuccess;
}


PVMFStatus PVPlayerEngine::DoSetLogAppender(PVPlayerEngineCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSetLogAppender() In"));

    char* tag = (char*)(aCmd.GetParam(0).pChar_value);
    OsclSharedPtr<PVLoggerAppender>* appender = (OsclSharedPtr<PVLoggerAppender>*)(aCmd.GetParam(1).pOsclAny_value);

    if (tag == NULL || appender == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSetLogAppender() Passed in parameter invalid."));
        return PVMFErrArgument;
    }

    // Get the logger node based on the specified tag
    PVLogger *rootnode = PVLogger::GetLoggerObject(tag);
    if (rootnode == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSetLogAppender() Node specified by tag is invalid"));
        return PVMFErrBadHandle;
    }

    // Add the specified appender to this node
    rootnode->AddAppender(*appender);

    EngineCommandCompleted(aCmd.GetCmdId(), aCmd.GetContext(), PVMFSuccess);


    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSetLogAppender() Out"));
    return PVMFSuccess;
}


PVMFStatus PVPlayerEngine::DoRemoveLogAppender(PVPlayerEngineCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoRemoveLogAppender() In"));

    char* tag = (char*)(aCmd.GetParam(0).pChar_value);
    OsclSharedPtr<PVLoggerAppender>* appender = (OsclSharedPtr<PVLoggerAppender>*)(aCmd.GetParam(1).pOsclAny_value);

    if (tag == NULL || appender == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoRemoveLogAppender() Passed in parameter invalid."));
        return PVMFErrArgument;
    }

    // Get the logger node based on the specified tag
    PVLogger *lognode = PVLogger::GetLoggerObject(tag);
    if (lognode == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoRemoveLogAppender() Node specified by tag is invalid"));
        return PVMFErrBadHandle;
    }

    // Remove the specified appender to this node
    lognode->RemoveAppender(*appender);

    EngineCommandCompleted(aCmd.GetCmdId(), aCmd.GetContext(), PVMFSuccess);


    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoRemoveLogAppender() Out"));
    return PVMFSuccess;
}


PVMFStatus PVPlayerEngine::DoSetLogLevel(PVPlayerEngineCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSetLogLevel() In"));

    char* tag = (char*)(aCmd.GetParam(0).pChar_value);
    int32 level = aCmd.GetParam(1).int32_value;
    bool subtree = aCmd.GetParam(2).bool_value;

    if (tag == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSetLogLevel() Passed in parameter invalid."));
        return PVMFErrArgument;
    }

    // Get the logger node based on the specified tag
    PVLogger *lognode = PVLogger::GetLoggerObject(tag);
    if (lognode == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSetLogLevel() Node specified by tag is invalid"));
        return PVMFErrBadHandle;
    }

    // Set the log level
    if (subtree)
    {
        lognode->SetLogLevelAndPropagate(level);
    }
    else
    {
        lognode->SetLogLevel(level);
    }

    EngineCommandCompleted(aCmd.GetCmdId(), aCmd.GetContext(), PVMFSuccess);


    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSetLogLevel() Out"));
    return PVMFSuccess;
}


PVMFStatus PVPlayerEngine::DoGetLogLevel(PVPlayerEngineCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoGetLogLevel() In"));

    char* tag = (char*)(aCmd.GetParam(0).pChar_value);
    PVLogLevelInfo* levelinfo = (PVLogLevelInfo*)(aCmd.GetParam(1).pOsclAny_value);

    if (tag == NULL || levelinfo == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoGetLogLevel() Passed in parameter invalid."));
        return PVMFErrArgument;
    }

    // Get the logger node based on the specified tag
    PVLogger *lognode = PVLogger::GetLoggerObject(tag);
    if (lognode == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoGetLogLevel() Node specified by tag is invalid"));
        return PVMFErrBadHandle;
    }

    // Get the log level info
    *levelinfo = lognode->GetLogLevel();

    EngineCommandCompleted(aCmd.GetCmdId(), aCmd.GetContext(), PVMFSuccess);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoGetLogLevel() Out"));
    return PVMFSuccess;
}


PVMFStatus PVPlayerEngine::DoQueryUUID(PVPlayerEngineCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoQueryUUID() In"));

    PvmfMimeString* mimetype;
    Oscl_Vector<PVUuid, OsclMemAllocator> *uuidvec;
    bool exactmatch;

    mimetype = (PvmfMimeString*)(aCmd.GetParam(0).pOsclAny_value);
    uuidvec = (Oscl_Vector<PVUuid, OsclMemAllocator>*)(aCmd.GetParam(1).pOsclAny_value);
    exactmatch = aCmd.GetParam(2).bool_value;

    if (mimetype == NULL || uuidvec == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoQueryUUID() Passed in parameter invalid."));
        return PVMFErrArgument;
    }

    int32 leavecode = 0;

    // For now just return all available extension interface UUID
    OSCL_TRY(leavecode,
             // Capability and config interface
             uuidvec->push_back(PVMI_CAPABILITY_AND_CONFIG_PVUUID);
             // License acquisition interface
             uuidvec->push_back(PVPlayerLicenseAcquisitionInterfaceUuid);
             // Track level info interface from source node
             if (iSourceNodeTrackLevelInfoIF)
{
    uuidvec->push_back(PVMF_TRACK_LEVEL_INFO_INTERFACE_UUID);
    }
            );
    OSCL_FIRST_CATCH_ANY(leavecode,
                         PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoQueryUUID() Leaved"));
                         EngineCommandCompleted(aCmd.GetCmdId(), aCmd.GetContext(), PVMFErrNoMemory);
                         PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoQueryUUID() Out"));
                         return PVMFSuccess;);

    EngineCommandCompleted(aCmd.GetCmdId(), aCmd.GetContext(), PVMFSuccess);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoQueryUUID() Out"));
    return PVMFSuccess;
}


PVMFStatus PVPlayerEngine::DoQueryInterface(PVPlayerEngineCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoQueryInterface() In"));

    PVInterface** ifptr = (PVInterface**)(aCmd.GetParam(0).pOsclAny_value);
    PVUuid uuid = aCmd.GetUuid();
    if (ifptr == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoQueryInterface() Passed in parameter invalid."));
        return PVMFErrArgument;
    }

    PVMFStatus cmdstatus = PVMFSuccess;
    if (queryInterface(uuid, *ifptr) == false)
    {
        cmdstatus = PVMFErrNotSupported;
    }
    else
    {
        (*ifptr)->addRef();
    }

    EngineCommandCompleted(aCmd.GetCmdId(), aCmd.GetContext(), cmdstatus);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoQueryInterface() Out"));
    return PVMFSuccess;
}


PVMFStatus PVPlayerEngine::DoGetPVPlayerState(PVPlayerEngineCommand& aCmd, bool aSyncCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoGetPVPlayerState() In"));

    PVPlayerState* state = (PVPlayerState*)(aCmd.GetParam(0).pOsclAny_value);
    if (state == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoGetPVPlayerState() Passed in parameter invalid."));
        return PVMFErrArgument;
    }

    // Get player state using internal function
    *state = GetPVPlayerState();

    if (!aSyncCmd)
    {
        EngineCommandCompleted(aCmd.GetCmdId(), aCmd.GetContext(), PVMFSuccess);
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoGetPVPlayerState() Out"));
    return PVMFSuccess;
}


PVMFStatus PVPlayerEngine::DoAddDataSource(PVPlayerEngineCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_NOTICE,
                    (0, "PVPlayerEngine::DoAddDataSource() Tick=%d", OsclTickCount::TickCount()));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoAddDataSource() In"));

    if (GetPVPlayerState() != PVP_STATE_IDLE)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoAddDataSource() Wrong engine state"));
        return PVMFErrInvalidState;
    }

    if (aCmd.GetParam(0).pOsclAny_value == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoAddDataSource() Passed in parameter invalid."));
        return PVMFErrArgument;
    }

    // Save the data source
    iDataSource = (PVPlayerDataSource*)(aCmd.GetParam(0).pOsclAny_value);

    // (mg) For rollover reset to first available alternate
    iAlternateSrcFormatIndex = 0;
    iDataReadySent = false;

    // Check the source format and do a recognize if unknown
    PVMFStatus retval = PVMFSuccess;
    iSourceFormatType = iDataSource->GetDataSourceFormatType();

    if (iSourceFormatType == PVMF_MIME_FORMAT_UNKNOWN)
    {
        retval = DoQuerySourceFormatType(aCmd.GetCmdId(), aCmd.GetContext());
    }
    else
    {
        if (iSourceFormatType == PVMF_MIME_DATA_SOURCE_UNKNOWN_URL)
        {
            retval = SetupDataSourceForUnknownURLAccess();
            if (retval != PVMFSuccess)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoAddDataSource() - SetupDataSourceForUnknownURLAccess Failed"));
                return retval;
            }
        }

        // Start the source node creation and setup sequence
        retval = DoSetupSourceNode(aCmd.GetCmdId(), aCmd.GetContext());

        if (retval != PVMFSuccess)
        {
            bool ehPending = CheckForPendingErrorHandlingCmd();
            if (ehPending)
            {
                // there should be no error handling queued.
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoAddDataSource() Already EH pending, should never happen"));
                return PVMFPending;
            }
            // Queue up Error Handling
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoAddDataSource() DoSetupSourceNode failed, Add EH command"));
            iCommandCompleteStatusInErrorHandling = retval;
            iCommandCompleteErrMsgInErrorHandling = NULL;
            AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_ADD_DATA_SOURCE, NULL, NULL, NULL, false);
            return PVMFPending;
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoAddDataSource() Out"));
    return retval;

}


PVMFStatus PVPlayerEngine::DoQuerySourceFormatType(PVCommandId aCmdId, OsclAny* aCmdContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_INFO,
                    (0, "PVPlayerEngine::DoQuerySourceFormatType() Tick=%d", OsclTickCount::TickCount()));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoQuerySourceFormatType() In"));

    // Use the recognizer if the source format type is unknown
    OSCL_ASSERT(iDataSource != NULL);
    PVPlayerEngineContext* context = AllocateEngineContext(NULL, NULL, NULL, aCmdId, aCmdContext, PVP_CMD_QUERYSOURCEFORMATTYPE);
    PVMFStatus retval = PVMFSuccess;
    int32 leavecode = 0;

    OsclAny * opaqueData = iDataSource->GetDataSourceContextData();
    PVInterface* pvInterface = OSCL_STATIC_CAST(PVInterface*, opaqueData);
    PVInterface* SourceContextData = NULL;
    PVUuid SourceContextDataUuid(PVMF_SOURCE_CONTEXT_DATA_UUID);
    PVMFCPMPluginAccessInterfaceFactory * DataStreamDataFactory = NULL;

    if (pvInterface != NULL && pvInterface->queryInterface(SourceContextDataUuid, SourceContextData))
    {
        PVMFSourceContextData * aSourceContextData = OSCL_STATIC_CAST(PVMFSourceContextData*, SourceContextData);
        PVMFSourceContextDataCommon * aSourceContextDataCommon = aSourceContextData->CommonData();
        if (aSourceContextDataCommon)
        {
            DataStreamDataFactory = aSourceContextDataCommon->iRecognizerDataStreamFactory;
        }
    }

    if (DataStreamDataFactory)
    {
        OSCL_TRY(leavecode, retval = iPlayerRecognizerRegistry.QueryFormatType(DataStreamDataFactory, *this, (OsclAny*) context));
        OSCL_FIRST_CATCH_ANY(leavecode,
                             FreeEngineContext(context);
                             return PVMFErrNotSupported;
                            );
    }
    else
    {
        OSCL_TRY(leavecode, retval = iPlayerRecognizerRegistry.QueryFormatType(iDataSource->GetDataSourceURL(), *this, (OsclAny*) context));
        OSCL_FIRST_CATCH_ANY(leavecode,
                             FreeEngineContext(context);
                             return PVMFErrNotSupported;
                            );
    }

    if (retval != PVMFSuccess)
    {
        FreeEngineContext(context);
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoQuerySourceFormatType() Out"));
    return retval;
}


PVMFStatus PVPlayerEngine::DoSetupSourceNode(PVCommandId aCmdId, OsclAny* aCmdContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_INFO,
                    (0, "PVPlayerEngine::DoSetupSourceNode() Tick=%d", OsclTickCount::TickCount()));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSetupSourceNode() In"));

    OSCL_ASSERT(iDataSource != NULL);

    if (iSourceNode == NULL)
    {
        PVMFFormatType outputformattype = PVMF_MIME_FORMAT_UNKNOWN ;

        Oscl_Vector<PVUuid, OsclMemAllocator> foundUuids;
        // Query the node registry
        if (iPlayerNodeRegistry.QueryRegistry(iSourceFormatType, outputformattype, foundUuids) == PVMFSuccess)
        {
            if (foundUuids.empty())
            {
                // No matching node found
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVPlayerEngine::DoSetupSourceNode() Query Regsitry successful, No matching source node found."));
                return PVMFErrNotSupported;
            }

            int32 leavecode = 0;
            OSCL_TRY(leavecode, iSourceNode = iPlayerNodeRegistry.CreateNode(foundUuids[0]));
            OSCL_FIRST_CATCH_ANY(leavecode,
                                 PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSetupDecNode() Error in creating SourceNode"));
                                 return PVMFFailure;);

            iNodeUuids.push_back(PVPlayerEngineUuidNodeMapping(foundUuids[0], iSourceNode));

            if (iSourceNode == NULL)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSetupSourceNode() Source node create failed"));
                return PVMFErrNoMemory;
            }
        }
        else
        {
            // Registry query failed
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSetupSourceNode() Registry query for source node failed"));
            return PVMFErrNotSupported;
        }
    }

    if (iSourceNode->ThreadLogon() != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSetupSourceNode() ThreadLogon() on the source node failed"));
        OSCL_ASSERT(false);
        return PVMFFailure;
    }

    PVMFNodeSessionInfo nodesessioninfo(this, this, (OsclAny*)iSourceNode, this, (OsclAny*)iSourceNode);
    int32 leavecode = 0;
    OSCL_TRY(leavecode, iSourceNodeSessionId = iSourceNode->Connect(nodesessioninfo));
    OSCL_FIRST_CATCH_ANY(leavecode,
                         PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSetupSourceNode() Connect on iSourceNode did a leave!"));
                         OSCL_ASSERT(false);
                         return PVMFFailure);

    PVPlayerEngineContext* context = AllocateEngineContext(NULL, iSourceNode, NULL, aCmdId, aCmdContext, PVP_CMD_SourceNodeQueryInitIF);

    PVUuid sourceinituuid = PVMF_DATA_SOURCE_INIT_INTERFACE_UUID;
    leavecode = 0;
    PVMFCommandId cmdid = -1;
    iSourceNodePVInterfaceInit = NULL;
    OSCL_TRY(leavecode, cmdid = iSourceNode->QueryInterface(iSourceNodeSessionId, sourceinituuid, iSourceNodePVInterfaceInit, (OsclAny*)context));
    OSCL_FIRST_CATCH_ANY(leavecode,
                         iSourceNodePVInterfaceInit = NULL;
                         FreeEngineContext(context);
                         PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSetupSourceNode() QueryInterface on iSourceNode did a leave!"));
                         return PVMFFailure);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSetupSourceNode() Out"));
    return PVMFSuccess;
}


PVMFStatus PVPlayerEngine::DoSourceNodeQueryTrackSelIF(PVCommandId aCmdId, OsclAny* aCmdContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVPlayerEngine::DoSourceNodeQueryTrackSelIF() Tick=%d", OsclTickCount::TickCount()));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSourceNodeQueryTrackSelIF() In"));

    int32 leavecode = 0;

    if (iDataSource->GetDataSourceType() == PVP_DATASRCTYPE_URL)
    {
        // Setup the source node via the initialization IF
        OSCL_ASSERT(iSourceFormatType != PVMF_MIME_FORMAT_UNKNOWN);

        OSCL_wHeapString<OsclMemAllocator> sourceURL;
        // In case the URL starts with file:// skip it
        OSCL_wStackString<8> fileScheme(_STRLIT_WCHAR("file"));
        OSCL_wStackString<8> schemeDelimiter(_STRLIT_WCHAR("://"));
        const oscl_wchar* actualURL = NULL;

        if (oscl_strncmp(fileScheme.get_cstr(), iDataSource->GetDataSourceURL().get_cstr(), 4) == 0)
        {
            actualURL = oscl_strstr(iDataSource->GetDataSourceURL().get_cstr(), schemeDelimiter.get_cstr());
            if (actualURL == NULL)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSourceNodeQueryTrackSelIF() Unable to skip over file://"));
                return PVMFErrArgument;
            }
            //skip over ://
            actualURL += schemeDelimiter.get_size();
            sourceURL += actualURL;
        }
        else
        {
            sourceURL += iDataSource->GetDataSourceURL().get_cstr();
        }

        PVMFStatus retval = iSourceNodeInitIF->SetSourceInitializationData(sourceURL, iSourceFormatType, iDataSource->GetDataSourceContextData());
        if (retval != PVMFSuccess)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSourceNodeQueryTrackSelIF() SetSourceInitializationData failed"));
            return PVMFFailure;
        }
        // Set Playback Clock
        retval = iSourceNodeInitIF->SetClientPlayBackClock(&iPlaybackClock);
        if (retval != PVMFSuccess)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSourceNodeQueryTrackSelIF() SetClientPlayBackClock failed!"));
            return PVMFFailure;
        }
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSourceNodeQueryTrackSelIF() Data source type not supported yet so asserting"));
        OSCL_ASSERT(false);
        return PVMFFailure;
    }

    PVPlayerEngineContext* context = AllocateEngineContext(NULL, iSourceNode, NULL, aCmdId, aCmdContext, PVP_CMD_SourceNodeQueryTrackSelIF);

    // Query the source node for the track selection IF
    PVUuid trackseluuid = PVMF_TRACK_SELECTION_INTERFACE_UUID;
    PVMFCommandId cmdid = -1;
    leavecode = 0;
    iSourceNodePVInterfaceTrackSel = NULL;
    OSCL_TRY(leavecode, cmdid = iSourceNode->QueryInterface(iSourceNodeSessionId, trackseluuid, iSourceNodePVInterfaceTrackSel, (OsclAny*)context));
    OSCL_FIRST_CATCH_ANY(leavecode,
                         iSourceNodePVInterfaceTrackSel = NULL;
                         FreeEngineContext(context);
                         PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSourceNodeQueryTrackSelIF() QueryInterface on iSourceNode did a leave!"));
                         return PVMFFailure);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSourceNodeQueryTrackSelIF() Out"));
    return PVMFSuccess;
}


PVMFStatus PVPlayerEngine::DoSourceNodeQueryInterfaceOptional(PVCommandId aCmdId, OsclAny* aCmdContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVPlayerEngine::DoSourceNodeQueryInterfaceOptional() Tick=%d", OsclTickCount::TickCount()));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSourceNodeQueryInterfaceOptional() In"));

    PVPlayerEngineContext* context = NULL;
    PVMFCommandId cmdid = -1;
    int32 leavecode = 0;

    iNumPendingNodeCmd = 0;

    // Query for Track Level Info IF
    context = AllocateEngineContext(NULL, iSourceNode, NULL, aCmdId, aCmdContext, PVP_CMD_SourceNodeQueryTrackLevelInfoIF);
    PVUuid tracklevelinfouuid = PVMF_TRACK_LEVEL_INFO_INTERFACE_UUID;
    cmdid = -1;
    leavecode = 0;
    iSourceNodePVInterfaceTrackLevelInfo = NULL;
    OSCL_TRY(leavecode, cmdid = iSourceNode->QueryInterface(iSourceNodeSessionId, tracklevelinfouuid, iSourceNodePVInterfaceTrackLevelInfo, (OsclAny*)context));
    if (leavecode)
    {
        iSourceNodePVInterfaceTrackLevelInfo = NULL;
        FreeEngineContext(context);
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSourceNodeQueryInterfaceOptional() QueryInterface on iSourceNode did a leave!"));
    }
    else
    {
        ++iNumPendingNodeCmd;
    }

    // Query for Playback Control IF
    context = NULL;
    context = AllocateEngineContext(NULL, iSourceNode, NULL, aCmdId, aCmdContext, PVP_CMD_SourceNodeQueryPBCtrlIF);
    PVUuid pbctrluuid = PvmfDataSourcePlaybackControlUuid;
    cmdid = -1;
    leavecode = 0;
    iSourceNodePVInterfacePBCtrl = NULL;
    OSCL_TRY(leavecode, cmdid = iSourceNode->QueryInterface(iSourceNodeSessionId, pbctrluuid, iSourceNodePVInterfacePBCtrl, (OsclAny*)context));
    if (leavecode)
    {
        iSourceNodePVInterfacePBCtrl = NULL;
        FreeEngineContext(context);
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSourceNodeQueryInterfaceOptional() QueryInterface on iSourceNode did a leave!"));
    }
    else
    {
        ++iNumPendingNodeCmd;
    }

    // Query for direction control IF
    context = NULL;
    context = AllocateEngineContext(NULL, iSourceNode, NULL, aCmdId, aCmdContext, PVP_CMD_SourceNodeQueryDirCtrlIF);
    PVUuid dirctrluuid = PvmfDataSourceDirectionControlUuid;
    cmdid = -1;
    leavecode = 0;
    iSourceNodePVInterfaceDirCtrl = NULL;
    OSCL_TRY(leavecode, cmdid = iSourceNode->QueryInterface(iSourceNodeSessionId, dirctrluuid, iSourceNodePVInterfaceDirCtrl, (OsclAny*)context));
    if (leavecode)
    {
        iSourceNodePVInterfaceDirCtrl = NULL;
        FreeEngineContext(context);
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSourceNodeQueryInterfaceOptional() QueryInterface on iSourceNode did a leave!"));
    }
    else
    {
        ++iNumPendingNodeCmd;
    }

    // Query for Metadata IF
    context = NULL;
    context = AllocateEngineContext(NULL, iSourceNode, NULL, aCmdId, aCmdContext, PVP_CMD_SourceNodeQueryMetadataIF);
    PVUuid metadatauuid = KPVMFMetadataExtensionUuid;
    cmdid = -1;
    leavecode = 0;
    iSourceNodePVInterfaceMetadataExt = NULL;
    OSCL_TRY(leavecode, cmdid = iSourceNode->QueryInterface(iSourceNodeSessionId, metadatauuid, iSourceNodePVInterfaceMetadataExt, (OsclAny*)context));
    if (leavecode)
    {
        iSourceNodePVInterfaceMetadataExt = NULL;
        FreeEngineContext(context);
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSourceNodeQueryInterfaceOptional() QueryInterface on iSourceNode did a leave!"));
    }
    else
    {
        ++iNumPendingNodeCmd;
    }

    // Query for Cap-Config IF
    context = NULL;
    context = AllocateEngineContext(NULL, iSourceNode, NULL, aCmdId, aCmdContext, PVP_CMD_SourceNodeQueryCapConfigIF);
    PVUuid capconfiguuid = PVMI_CAPABILITY_AND_CONFIG_PVUUID;
    cmdid = -1;
    leavecode = 0;
    iSourceNodePVInterfaceCapConfig = NULL;
    OSCL_TRY(leavecode, cmdid = iSourceNode->QueryInterface(iSourceNodeSessionId, capconfiguuid, iSourceNodePVInterfaceCapConfig, (OsclAny*)context));
    if (leavecode)
    {
        iSourceNodePVInterfaceCapConfig = NULL;
        FreeEngineContext(context);
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSourceNodeQueryInterfaceOptional() QueryInterface on iSourceNode did a leave!"));
    }
    else
    {
        ++iNumPendingNodeCmd;
    }

    // Query for CPM License interface
    context = NULL;
    context = AllocateEngineContext(NULL, iSourceNode, NULL, aCmdId, aCmdContext, PVP_CMD_SourceNodeQueryCPMLicenseIF);
    PVUuid licUuid = PVMFCPMPluginLicenseInterfaceUuid;
    cmdid = -1;
    leavecode = 0;
    iSourceNodePVInterfaceCPMLicense = NULL;
    OSCL_TRY(leavecode, cmdid = iSourceNode->QueryInterface(iSourceNodeSessionId, licUuid, iSourceNodePVInterfaceCPMLicense, (OsclAny*)context));
    if (leavecode)
    {
        iSourceNodePVInterfaceCPMLicense = NULL;
        FreeEngineContext(context);
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSourceNodeQueryInterfaceOptional() QueryInterface on iSourceNode did a leave!"));
    }
    else
    {
        ++iNumPendingNodeCmd;
    }

    // Query for source node registry init extension IF
    context = NULL;
    context = AllocateEngineContext(NULL, iSourceNode, NULL, aCmdId, aCmdContext, PVP_CMD_SourceNodeQuerySrcNodeRegInitIF);
    PVUuid regInitUuid = PVMF_DATA_SOURCE_NODE_REGISRTY_INIT_INTERFACE_UUID;
    cmdid = -1;
    leavecode = 0;
    iSourceNodePVInterfaceRegInit = NULL;
    OSCL_TRY(leavecode, cmdid = iSourceNode->QueryInterface(iSourceNodeSessionId, regInitUuid, iSourceNodePVInterfaceRegInit, (OsclAny*)context));
    if (leavecode)
    {
        iSourceNodePVInterfaceRegInit = NULL;
        FreeEngineContext(context);
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSourceNodeQueryInterfaceOptional() QueryInterface on iSourceNode did a leave!"));
    }
    else
    {
        ++iNumPendingNodeCmd;
    }

    if (iNumPendingNodeCmd > 0)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSourceNodeQueryInterfaceOptional() Out"));
        return PVMFSuccess;
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSourceNodeQueryInterfaceOptional() Out No pending QueryInterface() on source node"));
        return PVMFFailure;
    }
}

PVMFStatus PVPlayerEngine::DoGetMetadataKey(PVPlayerEngineCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_NOTICE,
                    (0, "PVPlayerEngine::DoGetMetadataKey() Tick=%d", OsclTickCount::TickCount()));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoGetMetadataKey() In"));

    if (GetPVPlayerState() == PVP_STATE_ERROR)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoGetMetadataKey() Wrong engine state."));
        return PVMFFailure;
    }

    iGetMetadataKeysParam.iKeyList = (PVPMetadataList*)(aCmd.GetParam(0).pOsclAny_value);
    iGetMetadataKeysParam.iStartingKeyIndex = aCmd.GetParam(1).int32_value;
    iGetMetadataKeysParam.iMaxKeyEntries = aCmd.GetParam(2).int32_value;
    iGetMetadataKeysParam.iQueryKey = aCmd.GetParam(3).pChar_value;

    if (iGetMetadataKeysParam.iKeyList == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoGetMetadataKey() Passed in parameter invalid."));
        return PVMFErrArgument;
    }

    if (iGetMetadataKeysParam.iMaxKeyEntries < -1 || iGetMetadataKeysParam.iMaxKeyEntries == 0 || iGetMetadataKeysParam.iStartingKeyIndex < 0)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoGetMetadataKey() Passed in parameter invalid."));
        return PVMFErrArgument;
    }

    // Determine which node's metadata interface to start the retrieval based on the starting index
    uint32 i = 0;
    int32 totalnumkey = 0;
    uint32 nodestartindex = 0;
    while (i < iMetadataIFList.size())
    {
        int32 numkey = iMetadataIFList[i].iInterface->GetNumMetadataKeys(iGetMetadataKeysParam.iQueryKey);
        if (iGetMetadataKeysParam.iStartingKeyIndex < (totalnumkey + numkey))
        {
            // Found the node to start the key retrieval
            // Determine the start index for this node
            nodestartindex = iGetMetadataKeysParam.iStartingKeyIndex - totalnumkey;
            break;
        }
        else
        {
            // Keep checking
            totalnumkey += numkey;
            ++i;
        }
    }

    // Check if the search succeeded
    if (i == iMetadataIFList.size() || iMetadataIFList.size() == 0)
    {
        // Starting index is too large or there is no metadata interface available
        return PVMFErrArgument;
    }

    // Retrieve the metadata key from the first node
    PVPlayerEngineContext* context = AllocateEngineContext(iMetadataIFList[i].iEngineDatapath, iMetadataIFList[i].iNode, NULL, aCmd.GetCmdId(), aCmd.GetContext(), PVP_CMD_GetNodeMetadataKey);
    PVMFMetadataExtensionInterface* metadataif = iMetadataIFList[i].iInterface;
    PVMFSessionId sessionid = iMetadataIFList[i].iSessionId;
    PVMFCommandId cmdid = -1;
    cmdid = metadataif->GetNodeMetadataKeys(sessionid,
                                            *(iGetMetadataKeysParam.iKeyList),
                                            nodestartindex,
                                            iGetMetadataKeysParam.iMaxKeyEntries,
                                            iGetMetadataKeysParam.iQueryKey,
                                            (OsclAny*)context);
    if (cmdid == -1)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVPlayerEngine::DoGetMetadataKey() GetNodeMetadataKeys failed"));
        return PVMFFailure;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoGetMetadataKey() Out"));

    // Save the current metadata value retrieval status
    iGetMetadataKeysParam.iCurrentInterfaceIndex = i;
    iGetMetadataKeysParam.iNumKeyEntriesToFill = iGetMetadataKeysParam.iMaxKeyEntries;
    iGetMetadataKeysParam.iNumKeyEntriesInList = iGetMetadataKeysParam.iKeyList->size();

    return PVMFSuccess;
}


PVMFStatus PVPlayerEngine::DoGetMetadataValue(PVPlayerEngineCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_NOTICE,
                    (0, "PVPlayerEngine::DoGetMetadataValue() Tick=%d", OsclTickCount::TickCount()));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoGetMetadataValue() In"));

    if (GetPVPlayerState() == PVP_STATE_ERROR)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoGetMetadataValue() Wrong engine state"));
        return PVMFErrInvalidState;
    }

    if (iReleaseMetadataValuesPending)
    {
        // App has called GetMetadataValues without calling ReleaseMetadata values on previous list. Wrong
        // usage. Failure needs to be returned in this case.
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVPlayerEngine::GetMetadataValues() Wrong usage, called again without releasing earlier metadata list"));
        return PVMFErrReleaseMetadataValueNotDone;
    }

    iGetMetadataValuesParam.iKeyList = (PVPMetadataList*)(aCmd.GetParam(0).pOsclAny_value);
    iGetMetadataValuesParam.iStartingValueIndex = aCmd.GetParam(1).int32_value;
    iGetMetadataValuesParam.iMaxValueEntries = aCmd.GetParam(2).int32_value;
    iGetMetadataValuesParam.iNumAvailableValues = (int32*)(aCmd.GetParam(3).pOsclAny_value);
    iGetMetadataValuesParam.iValueList = (Oscl_Vector<PvmiKvp, OsclMemAllocator>*)(aCmd.GetParam(4).pOsclAny_value);
    iMetadataValuesCopiedInCallBack = aCmd.GetParam(5).bool_value;

    if (iGetMetadataValuesParam.iKeyList == NULL || iGetMetadataValuesParam.iValueList == NULL || iGetMetadataValuesParam.iNumAvailableValues == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoGetMetadataValue() Passed in parameter invalid."));
        return PVMFErrArgument;
    }

    if (iGetMetadataValuesParam.iMaxValueEntries < -1 || iGetMetadataValuesParam.iMaxValueEntries == 0 || iGetMetadataValuesParam.iStartingValueIndex < 0)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoGetMetadataValue() Passed in parameter invalid."));
        return PVMFErrArgument;
    }

    // Determine which node's metadata interface to start the retrieval based on the starting index
    uint32 i = 0;
    int32 totalnumvalue = 0;
    uint32 nodestartindex = 0;
    while (i < iMetadataIFList.size())
    {
        int32 numvalue = iMetadataIFList[i].iInterface->GetNumMetadataValues(*(iGetMetadataValuesParam.iKeyList));
        if (iGetMetadataValuesParam.iStartingValueIndex < (totalnumvalue + numvalue))
        {
            // Found the node to start the value retrieval
            // Determine the start index for this node
            nodestartindex = iGetMetadataValuesParam.iStartingValueIndex - totalnumvalue;
            // Save the number of available values so far
            *(iGetMetadataValuesParam.iNumAvailableValues) = totalnumvalue + numvalue;
            break;
        }
        else
        {
            // Keep checking
            totalnumvalue += numvalue;
            ++i;
        }
    }

    // Check if the search succeeded
    if (i == iMetadataIFList.size() || iMetadataIFList.size() == 0)
    {
        // Starting index is too large or there is no metadata interface available
        return PVMFErrArgument;
    }

    // Retrieve the metadata value from the first node
    PVPlayerEngineContext* context = AllocateEngineContext(iMetadataIFList[i].iEngineDatapath, iMetadataIFList[i].iNode, NULL, aCmd.GetCmdId(), aCmd.GetContext(), PVP_CMD_GetNodeMetadataValue);
    PVMFMetadataExtensionInterface* metadataif = iMetadataIFList[i].iInterface;
    PVMFSessionId sessionid = iMetadataIFList[i].iSessionId;
    PVMFCommandId cmdid = -1;
    cmdid = metadataif->GetNodeMetadataValues(sessionid,
            *(iGetMetadataValuesParam.iKeyList),
            *(iGetMetadataValuesParam.iValueList),
            nodestartindex,
            iGetMetadataValuesParam.iMaxValueEntries,
            (OsclAny*)context);

    if (cmdid == -1)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVPlayerEngine::DoGetMetadataValue() GetNodeMetadataValues failed"));
        return PVMFFailure;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoGetMetadataValue() Out"));

    // Save the current metadata value retrieval status
    iGetMetadataValuesParam.iCurrentInterfaceIndex = i;
    iGetMetadataValuesParam.iNumValueEntriesToFill = iGetMetadataValuesParam.iMaxValueEntries;
    iGetMetadataValuesParam.iNumValueEntriesInList = iGetMetadataValuesParam.iValueList->size();

    return PVMFSuccess;
}

PVMFStatus PVPlayerEngine::DoReleaseMetadataValues(PVPlayerEngineCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_NOTICE,
                    (0, "PVPlayerEngine::DoReleaseMetadataValues() Tick=%d", OsclTickCount::TickCount()));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoReleaseMetadataValues() In"));

    if (GetPVPlayerState() == PVP_STATE_ERROR ||
            GetPVPlayerState() == PVP_STATE_IDLE)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoReleaseMetadataValues() Wrong engine state"));
        return PVMFErrInvalidState;
    }

    iGetMetadataValuesParam.iValueList = (Oscl_Vector<PvmiKvp, OsclMemAllocator>*)(aCmd.GetParam(0).pOsclAny_value);

    if (iGetMetadataValuesParam.iValueList == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoReleaseMetadataValues() Passed in parameter invalid."));
        return PVMFErrArgument;
    }

    // Release the memory allocated for the metadata values
    while (iMetadataValueReleaseList.empty() == false)
    {
        PVMFMetadataExtensionInterface* mdif = iMetadataIFList[iMetadataValueReleaseList[0].iMetadataIFListIndex].iInterface;
        OSCL_ASSERT(mdif != NULL);
        mdif->ReleaseNodeMetadataValues(*(iGetMetadataValuesParam.iValueList), iMetadataValueReleaseList[0].iStartIndex, iMetadataValueReleaseList[0].iEndIndex);
        iMetadataValueReleaseList.erase(iMetadataValueReleaseList.begin());
    }

    iReleaseMetadataValuesPending = false;

    EngineCommandCompleted(aCmd.GetCmdId(), aCmd.GetContext(), PVMFSuccess);

    return PVMFSuccess;
}

PVMFStatus PVPlayerEngine::DoInit(PVPlayerEngineCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_NOTICE,
                    (0, "PVPlayerEngine::DoInit() Tick=%d", OsclTickCount::TickCount()));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoInit() In"));

    if (GetPVPlayerState() == PVP_STATE_INITIALIZED)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoInit() Engine already in Initialized State"));
        EngineCommandCompleted(aCmd.GetCmdId(), aCmd.GetContext(), PVMFSuccess);
        return PVMFSuccess;
    }

    if ((GetPVPlayerState() != PVP_STATE_IDLE) || (iSourceNode == NULL))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoInit() Wrong engine state"));
        return PVMFErrInvalidState;
    }

    iRollOverState = RollOverStateIdle;

    PVMFStatus retval = DoSourceNodeInit(aCmd.GetCmdId(), aCmd.GetContext());

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoInit() Out"));

    if (retval == PVMFSuccess)
    {
        SetEngineState(PVP_ENGINE_STATE_INITIALIZING);
        return PVMFSuccess;
    }
    else
    {
        bool ehPending = CheckForPendingErrorHandlingCmd();
        if (ehPending)
        {
            // there should be no error handling queued.
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoInit() Already EH pending, should never happen"));
            return PVMFPending;
        }
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoInit() DoSourceNodeInit failed, Add EH command"));
        iCommandCompleteStatusInErrorHandling = retval;
        iCommandCompleteErrMsgInErrorHandling = NULL;
        AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_INIT, NULL, NULL, NULL, false);
        return PVMFPending;
    }
}


PVMFStatus PVPlayerEngine::DoSourceNodeInit(PVCommandId aCmdId, OsclAny* aCmdContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_INFO,
                    (0, "PVPlayerEngine::DoSourceNodeInit() Tick=%d", OsclTickCount::TickCount()));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSourceNodeInit() In"));

    OSCL_ASSERT(iSourceNode != NULL);

    int32 leavecode = 0;

    // Initialize the source node
    PVPlayerEngineContext* context = AllocateEngineContext(NULL, iSourceNode, NULL, aCmdId, aCmdContext, PVP_CMD_SourceNodeInit);

    leavecode = 0;
    PVMFCommandId cmdid = -1;
    OSCL_TRY(leavecode, cmdid = iSourceNode->Init(iSourceNodeSessionId, (OsclAny*)context));
    OSCL_FIRST_CATCH_ANY(leavecode,
                         FreeEngineContext(context);
                         PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSourceNodeInit() Init on iSourceNode did a leave!"));
                         return PVMFFailure);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSourceNodeInit() Out"));

    return PVMFSuccess;
}


PVMFStatus PVPlayerEngine::DoSourceNodeGetDurationValue(PVCommandId aCmdId, OsclAny* aCmdContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSourceNodeGetDurationValue() In"));


    // Create a key list with just duration key
    iSourceDurationKeyList.clear();
    OSCL_HeapString<OsclMemAllocator> tmpstr = _STRLIT_CHAR("duration");
    iSourceDurationKeyList.push_back(tmpstr);
    // Clear the value list
    iSourceDurationValueList.clear();

    if (iSourceNodeMetadataExtIF == NULL)
    {
        return PVMFErrArgument;
    }

    // Call GetNodeMetadataValues on the source node to retrieve duration
    PVPlayerEngineContext* context = AllocateEngineContext(NULL, iSourceNode, NULL, aCmdId, aCmdContext, PVP_CMD_SourceNodeGetDurationValue);

    int32 leavecode = 0;
    PVMFCommandId cmdid = -1;
    OSCL_TRY(leavecode, cmdid = iSourceNodeMetadataExtIF->GetNodeMetadataValues(iSourceNodeSessionId,
                                iSourceDurationKeyList,
                                iSourceDurationValueList,
                                0 /*starting index*/, 1 /*max entries*/, (OsclAny*)context));
    OSCL_FIRST_CATCH_ANY(leavecode,
                         FreeEngineContext(context);
                         PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSourceNodeGetDurationValue() GetNodeMetadataValues on iSourceNode did a leave!"));
                         return PVMFFailure);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSourceNodeGetDurationValue() Out"));
    return PVMFSuccess;
}

PVMFStatus PVPlayerEngine::DoSourceNodeRollOver(PVCommandId aCmdId, OsclAny* aCmdContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSourceNodeRollOver() In"));
    /* Clean up any exisiting source node */
    DoSourceNodeCleanup();

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO, (0, "PVPlayerEngine::DoSourceNodeRollOver() DoSourceNodeCleanup Complete"));
    if (CheckForSourceRollOver())
    {
        if (iDataSource->GetAlternateSourceFormatType(iSourceFormatType,
                iAlternateSrcFormatIndex))
        {
            uint8 localbuffer[8];
            oscl_memset(localbuffer, 0, 8);
            localbuffer[0] = 1;
            oscl_memcpy(&localbuffer[4], &iSourceFormatType, sizeof(uint32));

            PVUuid puuid = PVPlayerErrorInfoEventTypesUUID;
            PVMFBasicErrorInfoMessage* infomsg =
                OSCL_NEW(PVMFBasicErrorInfoMessage, (PVPlayerInfoAttemptingSourceRollOver, puuid, NULL));
            SendInformationalEvent(PVMFInfoSourceFormatNotSupported, OSCL_STATIC_CAST(PVInterface*, infomsg), NULL, localbuffer, 8);
            infomsg->removeRef();

            iAlternateSrcFormatIndex++;
            PVMFStatus status = DoSetupSourceNode(aCmdId, aCmdContext);
            if (status != PVMFSuccess)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSourceNodeRollOver() SourceNodeRollOver Failed, return status"));
                return status;
            }
            //roll over pending
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO, (0, "PVPlayerEngine::DoSourceNodeRollOver() SourceNodeRollOver In Progress"));
            return PVMFPending;
        }
    }
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSourceNodeRollOver() Invalid State"));
    return PVMFErrInvalidState;
}

PVMFStatus PVPlayerEngine::DoAcquireLicense(PVPlayerEngineCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_NOTICE,
                    (0, "PVPlayerEngine::DoAcquireLicense() Tick=%d", OsclTickCount::TickCount()));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoAcquireLicense() In"));

    // Retrieve the command parameters and validate
    iCPMAcquireLicenseParam.iContentNameChar = NULL;
    iCPMAcquireLicenseParam.iContentNameWChar = NULL;
    iCPMAcquireLicenseParam.iTimeoutMsec = (-1);
    iCPMAcquireLicenseParam.iLicenseData = NULL;
    iCPMAcquireLicenseParam.iLicenseDataSize = 0;

    if (aCmd.GetParam(0).pOsclAny_value != NULL)
    {
        iCPMAcquireLicenseParam.iLicenseData = aCmd.GetParam(0).pOsclAny_value;
    }

    if (aCmd.GetParam(1).uint32_value != 0)
    {
        iCPMAcquireLicenseParam.iLicenseDataSize = aCmd.GetParam(1).uint32_value;
    }

    if (aCmd.GetCmdType() == PVP_ENGINE_COMMAND_ACQUIRE_LICENSE_WCHAR)
    {
        iCPMAcquireLicenseParam.iContentNameWChar = aCmd.GetParam(2).pWChar_value;
    }
    else
    {
        iCPMAcquireLicenseParam.iContentNameChar = aCmd.GetParam(2).pChar_value;
    }
    iCPMAcquireLicenseParam.iTimeoutMsec = aCmd.GetParam(3).int32_value;

    if (iCPMAcquireLicenseParam.iContentNameWChar == NULL && iCPMAcquireLicenseParam.iContentNameChar == NULL)
    {
        // Content name not specified
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoAcquireLicense() Content name not specified."));
        return PVMFErrArgument;
    }

    if (iCPMAcquireLicenseParam.iTimeoutMsec < -1)
    {
        // Timeout parameter not valid
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoAcquireLicense() Timeout value not valid."));
        return PVMFErrArgument;
    }

    // To acquire license, player data source and local data source need to be available
    if (iDataSource == NULL)
    {
        // Player data source not available
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoAcquireLicense() Player data source not specified."));
        return PVMFErrNotReady;
    }
    if (iDataSource->GetDataSourceContextData() == NULL)
    {
        // Pointer to the local data source if not available
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoAcquireLicense() Local data source in player data source not specified."));
        return PVMFErrBadHandle;
    }

    //If the license interface is available from the source node, use that.
    if (iSourceNodeCPMLicenseIF != NULL)
    {
        PVMFStatus status = DoSourceNodeGetLicense(aCmd.GetCmdId(), aCmd.GetContext());
        if (status != PVMFSuccess)
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoAcquireLicense() DoSourceNodeGetLicense failed."));

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoAcquireLicense() Out"));
        return status;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoAcquireLicense() Out"));
    // if the license interface is not available from the source node, fail the command
    return PVMFFailure;
}

void PVPlayerEngine::DoCancelAcquireLicense(PVPlayerEngineCommand& aCmd)
{

    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_NOTICE,
                    (0, "PVPlayerEngine::DoCancelAcquireLicense() Tick=%d", OsclTickCount::TickCount()));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoCancelAcquireLicense() In"));

    /* Engine cannot be processing another cancel command */
    OSCL_ASSERT(iCmdToDlaCancel.empty() == true);

    PVMFCommandId id = aCmd.GetParam(0).int32_value;
    PVMFStatus status = PVMFSuccess;

    if (iCurrentCmd.size() == 1)
    {
        /* First save the current command being processed */
        PVPlayerEngineCommand currentcmd(iCurrentCmd[0]);

        /* First check "current" command if any */
        if (id == iCurrentCmd[0].GetCmdId())
        {
            /* Cancel the current command first */
            if (iCurrentCmd[0].GetCmdType() == PVP_ENGINE_COMMAND_ACQUIRE_LICENSE_CHAR
                    || iCurrentCmd[0].GetCmdType() == PVP_ENGINE_COMMAND_ACQUIRE_LICENSE_WCHAR)
            {
                /* Make the CancelAll() command the current command */
                iCmdToDlaCancel.push_front(aCmd);
                /* Properly cancel a command being currently processed */
                if (iSourceNodeCPMLicenseIF != NULL)
                {
                    /* Cancel the GetLicense */
                    PVPlayerEngineContext* context = NULL;
                    PVMFCommandId cmdid = -1;
                    int32 leavecode = 0;
                    context = AllocateEngineContext(NULL, iSourceNode, NULL, aCmd.GetCmdId(), aCmd.GetContext(), PVP_CMD_SourceNodeCancelGetLicense);

                    OSCL_TRY(leavecode, cmdid = iSourceNodeCPMLicenseIF->CancelGetLicense(iSourceNodeSessionId, iCPMGetLicenseCmdId, (OsclAny*)context));
                    if (leavecode)
                    {
                        FreeEngineContext(context);
                        status = PVMFErrNotSupported;
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoCancelAcquireLicense() CancelGetLicense on iSourceNode did a leave!"));
                    }
                }
                else
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoCancelAcquireLicense() CPM plug-in registry in local data source not specified."));
                    OSCL_ASSERT(false);
                    status = PVMFErrBadHandle;
                }
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoCancelAcquireLicense() Current cmd is not AquireLicense."));
                status = PVMFErrArgument;
            }
        }
        else
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoCancelAcquireLicense() Current cmd ID is not equal with App specified cmd ID."));
            status = PVMFErrArgument;
        }
        if (status != PVMFSuccess)
        {
            /* We send error completetion for CancelAcquireLicense API*/
            iCurrentCmd.erase(iCurrentCmd.begin());
            iCurrentCmd.push_front(aCmd);
            EngineCommandCompleted(aCmd.GetCmdId(), aCmd.GetContext(), status);
            iCurrentCmd.push_front(currentcmd);
        }
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoCancelAcquireLicense() No Current cmd"));
        iCurrentCmd.push_front(aCmd);
        status = PVMFErrArgument;
        /* If we get here the command isn't queued so the cancel fails */
        EngineCommandCompleted(aCmd.GetCmdId(), aCmd.GetContext(), status);
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoCancelAcquireLicense() Out"));
    return;
}

PVMFStatus PVPlayerEngine::DoSourceNodeGetLicense(PVCommandId aCmdId, OsclAny* aCmdContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_INFO,
                    (0, "PVPlayerEngine::DoSourceNodeGetLicense() Tick=%d", OsclTickCount::TickCount()));

    OSCL_UNUSED_ARG(aCmdId);
    OSCL_UNUSED_ARG(aCmdContext);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSourceNodeGetLicense() In"));

    if (iSourceNodeCPMLicenseIF == NULL)
    {
        OSCL_ASSERT(false);
        return PVMFErrBadHandle;
    }

    // Get the license
    PVPlayerEngineContext* context = NULL;
    int32 leavecode = 0;
    context = AllocateEngineContext(NULL, iSourceNode, NULL, aCmdId, aCmdContext, PVP_CMD_SourceNodeGetLicense);
    if (iCPMAcquireLicenseParam.iContentNameChar)
    {
        // Use the char version
        iCPMContentNameStr = iCPMAcquireLicenseParam.iContentNameChar;
        OSCL_TRY(leavecode, iCPMGetLicenseCmdId = iSourceNodeCPMLicenseIF->GetLicense(iSourceNodeSessionId,
                 iCPMContentNameStr,
                 iCPMAcquireLicenseParam.iLicenseData,
                 iCPMAcquireLicenseParam.iLicenseDataSize,
                 iCPMAcquireLicenseParam.iTimeoutMsec,
                 (OsclAny*)context));
    }
    else if (iCPMAcquireLicenseParam.iContentNameWChar)
    {
        // Use the wchar version
        iCPMContentNameWStr = iCPMAcquireLicenseParam.iContentNameWChar;
        OSCL_TRY(leavecode, iCPMGetLicenseCmdId = iSourceNodeCPMLicenseIF->GetLicense(iSourceNodeSessionId,
                 iCPMContentNameWStr,
                 iCPMAcquireLicenseParam.iLicenseData,
                 iCPMAcquireLicenseParam.iLicenseDataSize,
                 iCPMAcquireLicenseParam.iTimeoutMsec,
                 (OsclAny*)context));
    }
    else
    {
        // This should not happen
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSourceNodeGetLicense() Content name not specified. Asserting"));
        OSCL_ASSERT(false);
        return PVMFErrArgument;
    }

    if (leavecode)
    {
        FreeEngineContext(context);
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSourceNodeGetLicense() GetLicense on iSourceNode did a leave!"));
    }
    else
    {
        ++iNumPendingNodeCmd;
    }

    if (iNumPendingNodeCmd <= 0)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,	(0, "PVPlayerEngine::DoSourceNodeGetLicense() Out No pending QueryInterface() on source node"));
        return PVMFFailure;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,	(0, "PVPlayerEngine::DoSourceNodeGetLicense() Out"));
    return PVMFSuccess;
}

PVMFStatus PVPlayerEngine::DoAddDataSink(PVPlayerEngineCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_NOTICE,
                    (0, "PVPlayerEngine::DoAddDataSink() Tick=%d", OsclTickCount::TickCount()));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoAddDataSink() In"));

    if (GetPVPlayerState() != PVP_STATE_INITIALIZED)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoAddDataSink() Wrong engine state"));
        return PVMFErrInvalidState;
    }

    if (aCmd.GetParam(0).pOsclAny_value == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoAddDataSink() Passed in parameter invalid."));
        return PVMFErrArgument;
    }

    PVPlayerDataSink* datasink = (PVPlayerDataSink*)(aCmd.GetParam(0).pOsclAny_value);

    PVPlayerEngineDatapath newdatapath;
    newdatapath.iDataSink = datasink;

    // Add a new engine datapath to the list for the data sink
    iDatapathList.push_back(newdatapath);

    EngineCommandCompleted(aCmd.GetCmdId(), aCmd.GetContext(), PVMFSuccess);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoAddDataSink() Out"));
    return PVMFSuccess;
}


PVMFStatus PVPlayerEngine::DoSetPlaybackRange(PVPlayerEngineCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_NOTICE,
                    (0, "PVPlayerEngine::DoSetPlaybackRange() Tick=%d", OsclTickCount::TickCount()));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSetPlaybackRange() In"));

    PVMFStatus retval;

    if (GetPVPlayerState() == PVP_STATE_ERROR)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSetPlaybackRange() Wrong engine state"));
        return PVMFErrInvalidState;
    }

    if (aCmd.GetParam(2).bool_value)
    {
        // Queueing of playback range is not supported yet
        iQueuedRangePresent = false;
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSetPlaybackRange() Queued playback range is not supported yet"));
        return PVMFErrNotSupported;
    }

    // Change the end position
    iCurrentEndPosition = aCmd.GetParam(1).playbackpos_value;
    retval = UpdateCurrentEndPosition(iCurrentEndPosition);
    if (retval != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSetPlaybackRange() Changing end position failed"));
        return retval;
    }

    if (aCmd.GetParam(0).playbackpos_value.iIndeterminate)
    {
        // Start position not specified so return as success
        EngineCommandCompleted(aCmd.GetCmdId(), aCmd.GetContext(), PVMFSuccess);
        return PVMFSuccess;
    }

    // reset repos related variables except the StreamID.
    ResetReposVariables(false);
    iStreamID++;

    // Reset the paused-due-to-EOS flag
    iPlaybackPausedDueToEndOfClip = false;

    // Change the begin position
    iCurrentBeginPosition = aCmd.GetParam(0).playbackpos_value;
    iTargetNPT = iCurrentBeginPosition.iPosValue.millisec_value;
    retval = UpdateCurrentBeginPosition(iCurrentBeginPosition, aCmd);

    if (retval == PVMFSuccess)
    {
        // Notify completion of engine command
        EngineCommandCompleted(aCmd.GetCmdId(), aCmd.GetContext(), PVMFSuccess);
    }
    else if (retval == PVMFPending)
    {
        // SetPlaybackRange command is still being processed
        // so change the return status so command is not completed yet
        retval = PVMFSuccess;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSetPlaybackRange() Out"));
    return retval;
}


PVMFStatus PVPlayerEngine::UpdateCurrentEndPosition(PVPPlaybackPosition& aEndPos)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::UpdateCurrentEndPosition() In"));

    if (aEndPos.iIndeterminate)
    {
        // Disable end time checking if running
        if (iEndTimeCheckEnabled)
        {
            iEndTimeCheckEnabled = false;
            iPollingCheckTimer->Cancel(PVPLAYERENGINE_TIMERID_ENDTIMECHECK);
        }
    }
    else
    {
        // Convert the end time to milliseconds to have consistent units internally
        uint32 timems = 0;
        PVMFStatus retval = ConvertToMillisec(aEndPos, timems);
        if (retval != PVMFSuccess)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::UpdateCurrentEndPosition() Converting to millisec failed"));
            return retval;
        }
        aEndPos.iPosValue.millisec_value = timems;
        aEndPos.iPosUnit = PVPPBPOSUNIT_MILLISEC;

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::UpdateCurrentEndPosition() Changing end time to %d ms", timems));

        // Enable the end time checking if not running
        if (!iEndTimeCheckEnabled)
        {
            iEndTimeCheckEnabled = true;

            if (GetPVPlayerState() == PVP_STATE_STARTED)
            {
                // Determine the check cycle based on interval setting in milliseconds
                // and timer frequency of 100 millisec
                int32 checkcycle = iEndTimeCheckInterval / 100;
                if (checkcycle == 0)
                {
                    ++checkcycle;
                }
                iPollingCheckTimer->Cancel(PVPLAYERENGINE_TIMERID_ENDTIMECHECK);
                iPollingCheckTimer->Request(PVPLAYERENGINE_TIMERID_ENDTIMECHECK, 0, checkcycle, this, true);
            }
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::UpdateCurrentEndPosition() Out"));
    return PVMFSuccess;
}


PVMFStatus PVPlayerEngine::UpdateCurrentBeginPosition(PVPPlaybackPosition& aBeginPos, PVPlayerEngineCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::UpdateCurrentBeginPosition() In"));

    PVMFStatus retval = PVMFSuccess;
    uint32 timems = 0;

    switch (GetPVPlayerState())
    {
        case PVP_STATE_PREPARED:
        case PVP_STATE_STARTED:
        {
            // Change the playback position immediately
            retval = ConvertToMillisec(aBeginPos, timems);
            if (retval != PVMFSuccess)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::UpdateCurrentBeginPosition() Converting to millisec failed"));
                return retval;
            }

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::UpdateCurrentBeginPosition() Requested begin position is %d ms", timems));

            retval = DoChangePlaybackPosition(aCmd.GetCmdId(), aCmd.GetContext());
        }
        break;

        case PVP_STATE_PAUSED:
        {
            // This is for use-case: Pause - SetPlaybackRate - Resume.
            // In DoResume engine will call SetDataSourceDirection and then from HandleSourceNodeSetDataSourceDirection
            // will call UpdateCurrentBeginPosition.
            if (iCurrentCmd[0].GetCmdType() == PVP_ENGINE_COMMAND_RESUME)
            {
                // Reposition occurred during the paused state so need to change the source position first
                retval = DoSourceNodeQueryDataSourcePosition(aCmd.GetCmdId(), aCmd.GetContext());
                if (retval == PVMFSuccess)
                {
                    //return Pending to indicate a node command was issued
                    return PVMFPending;
                }
                else
                {
                    //ignore failure, continue with resume sequence
                    return PVMFSuccess;
                }
            }
            else
            {
                //if there's already a direction change pending, then don't
                //allow a reposition also
                if (iChangePlaybackDirectionWhenResuming)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::UpdateCurrentBeginPosition() Direction change already pending, fail."));
                    return PVMFErrInvalidState;
                }

                // Convert the time units but flag to change playback position when resuming
                retval = ConvertToMillisec(aBeginPos, timems);
                if (retval != PVMFSuccess)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::UpdateCurrentBeginPosition() Converting to millisec failed in paused state"));
                    return retval;
                }

                iChangePlaybackPositionWhenResuming = true;

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::UpdateCurrentBeginPosition() Saving requested begin position(%d ms) for resume", timems));
            }
        }
        break;

        default:
            // Playback is stopped and start position is set so wait for playback to start
            break;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::UpdateCurrentBeginPosition() Out"));
    return retval;
}

PVMFStatus PVPlayerEngine::DoChangePlaybackPosition(PVCommandId aCmdId, OsclAny* aCmdContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoChangePlaybackPosition() In"));

    // Check if the source node has position control IF or
    // begin position is indeterminate
    if (iSourceNodePBCtrlIF == NULL ||
            iCurrentBeginPosition.iIndeterminate ||
            ((iCurrentBeginPosition.iPosUnit != PVPPBPOSUNIT_MILLISEC) &&
             (iCurrentBeginPosition.iPlayListPosUnit != PVPPBPOSUNIT_MILLISEC)))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoChangePlaybackPosition() Playback control IF on source node not available or invalid begin position"));
        return PVMFFailure;
    }

    PVMFCommandId cmdid = -1;

    if (iSeekToSyncPoint && iSyncPointSeekWindow > 0)
    {
        PVPlayerEngineContext* context = AllocateEngineContext(NULL, iSourceNode, NULL, aCmdId, aCmdContext, PVP_CMD_SourceNodeQueryDataSourcePositionDuringPlayback);

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoChangePlaybackPosition() Querying source position. Position %d ms, SeekToSyncPt %d", iTargetNPT, iSeekToSyncPoint));
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iReposLogger, PVLOGMSG_INFO, (0, "PVPlayerEngine::DoChangePlaybackPosition() Querying source position. Position %d ms, SeekToSyncPt %d", iTargetNPT, iSeekToSyncPoint));
        int32 leavecode = 0;

        // As in case of MP4 file we need to call overload function of QueryDataSourcePosition which retruns
        // I frame before and after instead of actaul NPT, format type will be checked here to first find if
        // format-type is one of the MP4 varient

        PVMFNodeCapability nodeCapability;
        iSourceNode->GetCapability(nodeCapability);
        PVMFFormatType * formatType = nodeCapability.iInputFormatCapability.begin();
        bool mpeg4FormatType = false;
        if (formatType != NULL)
        {
            if ((pv_mime_strcmp((char*)formatType->getMIMEStrPtr(), PVMF_MIME_MPEG4FF)) == 0)
            {
                mpeg4FormatType = true;
            }
            else
            {
                mpeg4FormatType = false;
            }
        }

        if (mpeg4FormatType)
        {
            OSCL_TRY(leavecode, cmdid = iSourceNodePBCtrlIF->QueryDataSourcePosition(iSourceNodeSessionId, iTargetNPT,
                                        iSeekPointBeforeTargetNPT, iSeekPointAfterTargetNPT, (OsclAny*)context, iSeekToSyncPoint));
        }
        else
        {
            OSCL_TRY(leavecode, cmdid = iSourceNodePBCtrlIF->QueryDataSourcePosition(iSourceNodeSessionId, iTargetNPT, iActualNPT,
                                        iSeekToSyncPoint, (OsclAny*)context));
        }

        OSCL_FIRST_CATCH_ANY(leavecode,
                             FreeEngineContext(context);
                             PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoChangePlaybackPosition() QueryDataSourcePosition on iSourceNodePBCtrlIF did a leave!"));
                             if (leavecode == PVMFErrNotSupported || leavecode == PVMFErrArgument)
    {
        return leavecode;
    }
    else
    {
        return PVMFFailure;
    }
                        );
    }
    else
    {
        // Go straight to repositioning the data source
        PVMFStatus retval = DoSourceNodeSetDataSourcePositionDuringPlayback(aCmdId, aCmdContext);
        if (retval == PVMFSuccess)
        {
            return PVMFPending;
        }
        else
        {
            return retval;
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoChangePlaybackPosition() Out"));

    return PVMFPending;
}

PVMFStatus PVPlayerEngine::DoSourceNodeSetDataSourcePositionDuringPlayback(PVCommandId aCmdId, OsclAny* aCmdContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSourceNodeSetDataSourcePositionDuringPlayback() In"));

    // Check if the source node has position control IF
    if (iSourceNodePBCtrlIF == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSourceNodeSetDataSourcePositionDuringPlayback() No source playback control IF"));
        return PVMFFailure;
    }
    bool clockpausedhere = false;
    switch (iPlaybackPositionMode)
    {
        case PVPPBPOS_MODE_END_OF_CURRENT_PLAY_ELEMENT:
        case PVPPBPOS_MODE_END_OF_CURRENT_PLAY_SESSION:
            break;
        case PVPPBPOS_MODE_NOW:
        default:
            // Pause the playback clock
            clockpausedhere = iPlaybackClock.Pause();

            // Stop the playback position status timer
            StopPlaybackStatusTimer();
            break;
    }
    // Set the new position on the source node
    PVPlayerEngineContext* context = AllocateEngineContext(NULL, iSourceNode, NULL, aCmdId, aCmdContext, PVP_CMD_SourceNodeSetDataSourcePositionDuringPlayback);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSourceNodeSetDataSourcePositionDuringPlayback() Calling SetDataSourcePosition() on source node. TargetNPT %d ms, SeekToSyncPoint %d", iTargetNPT, iSeekToSyncPoint));

    int32 leavecode = 0;
    if (iCurrentBeginPosition.iPosUnit == PVPPBPOSUNIT_PLAYLIST)
    {
        iDataSourcePosParams.iActualMediaDataTS = 0;
        iDataSourcePosParams.iActualNPT = 0;
        if ((iCurrentBeginPosition.iMode == PVPPBPOS_MODE_UNKNOWN) ||
                (iCurrentBeginPosition.iMode == PVPPBPOS_MODE_NOW))
        {
            iDataSourcePosParams.iMode = PVMF_SET_DATA_SOURCE_POSITION_MODE_NOW;
        }
        else if (iCurrentBeginPosition.iMode == PVPPBPOS_MODE_END_OF_CURRENT_PLAY_ELEMENT)
        {
            iDataSourcePosParams.iMode = PVMF_SET_DATA_SOURCE_POSITION_END_OF_CURRENT_PLAY_ELEMENT;
        }
        else if (iCurrentBeginPosition.iMode == PVPPBPOS_MODE_END_OF_CURRENT_PLAY_SESSION)
        {
            iDataSourcePosParams.iMode = PVMF_SET_DATA_SOURCE_POSITION_MODE_END_OF_CURRENT_PLAY_SESSION;
        }
        iDataSourcePosParams.iPlayElementIndex = iCurrentBeginPosition.iPlayElementIndex;
        iDataSourcePosParams.iSeekToSyncPoint = iSeekToSyncPoint;
        iDataSourcePosParams.iTargetNPT = iCurrentBeginPosition.iPlayListPosValue.millisec_value;
        iDataSourcePosParams.iStreamID = iStreamID;
        iDataSourcePosParams.iPlaylistUri = iCurrentBeginPosition.iPlayListUri;

        leavecode = IssueSourceSetDataSourcePosition(true, (OsclAny*)context);
        if (leavecode != 0)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSourceNodeSetDataSourcePositionDuringPlayback() SetDataSourcePosition on iSourceNodePBCtrlIF did a leave!"));
            FreeEngineContext(context);
            if (clockpausedhere)
            {
                // Resume the clock if paused in this function
                StartPlaybackClock();
            }

            --iStreamID;

            if (leavecode == PVMFErrNotSupported || leavecode == PVMFErrArgument)
            {
                return leavecode;
            }
            else
            {
                return PVMFFailure;
            }
        }
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iReposLogger, PVLOGMSG_INFO, (0, "PVPlayerEngine::DoSourceNodeSetDataSourcePositionDuringPlayback() SetDataSourcePosition on iSourceNodePBCtrlIF - TargetNPT=%d, SeekToSyncPoint=%d", iTargetNPT, iSeekToSyncPoint));
        leavecode = IssueSourceSetDataSourcePosition(false, (OsclAny*)context);

        if (leavecode != 0)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSourceNodeSetDataSourcePositionDuringPlayback() SetDataSourcePosition on iSourceNodePBCtrlIF did a leave!"));
            FreeEngineContext(context);
            if (clockpausedhere)
            {
                // Resume the clock if paused in this function
                StartPlaybackClock();
            }
            --iStreamID;
            if (leavecode == PVMFErrNotSupported || leavecode == PVMFErrArgument)
            {
                return leavecode;
            }
            else
            {
                return PVMFFailure;
            }
        }
    }
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSourceNodeSetDataSourcePositionDuringPlayback() Out"));

    return PVMFSuccess;
}

PVMFStatus PVPlayerEngine::DoSinkNodeSkipMediaDataDuringPlayback(PVCommandId aCmdId,
        OsclAny* aCmdContext,
        bool aSFR)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_INFO,
                    (0, "PVPlayerEngine::DoSinkNodeSkipMediaDataDuringPlayback() Tick=%d", OsclTickCount::TickCount()));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSinkNodeSkipMediaDataDuringPlayback() In"));

    // Pause the playback clock
    bool clockpausedhere = iPlaybackClock.Pause();

    // Tell the sink nodes to skip the unneeded media data
    iNumPendingNodeCmd = 0;
    int32 leavecode = 0;

    // For all sink node with sync control IF, call SkipMediaData()
    for (uint32 i = 0; i < iDatapathList.size(); ++i)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVPlayerEngine::DoSinkNodeSkipMediaDataDuringPlayback() Calling SkipMediaData() on sink nodes. MediadataTS to flush to %d ms, MediadataTS to skip to %d ms", iActualMediaDataTS, iSkipMediaDataTS));

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iReposLogger, PVLOGMSG_INFO,
                        (0, "PVPlayerEngine::DoSinkNodeSkipMediaDataDuringPlayback() Calling SkipMediaData() on sink nodes. MediadataTS to flush to %d ms, MediadataTS to skip to %d ms", iActualMediaDataTS, iSkipMediaDataTS));

        if (iDatapathList[i].iDatapath &&
                iDatapathList[i].iEndOfDataReceived == false &&
                iDatapathList[i].iSinkNodeSyncCtrlIF != NULL)
        {
            PVPlayerEngineContext* context = AllocateEngineContext(&(iDatapathList[i]), iDatapathList[i].iSinkNode, NULL, aCmdId, aCmdContext, PVP_CMD_SinkNodeSkipMediaDataDuringPlayback);
            leavecode = IssueSinkSkipMediaData(&(iDatapathList[i]), aSFR, (OsclAny*) context);

            if (leavecode == 0)
            {
                ++iNumPendingNodeCmd;
                ++iNumPendingSkipCompleteEvent;
                ++iNumPVMFInfoStartOfDataPending;
            }
            else
            {
                FreeEngineContext(context);
            }
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSinkNodeSkipMediaDataDuringPlayback() Out"));
    if (iNumPendingNodeCmd > 0)
    {
        return PVMFSuccess;
    }
    else
    {
        if (clockpausedhere)
        {
            // Resume the clock if paused in this function
            StartPlaybackClock();
        }

        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSinkNodeSkipMediaDataDuringPlayback() Skip on sink nodes failed"));
        return PVMFFailure;
    }
}


PVMFStatus PVPlayerEngine::DoGetPlaybackRange(PVPlayerEngineCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoGetPlaybackRange() In"));

    if (aCmd.GetParam(0).pPlaybackpos_value == NULL ||
            aCmd.GetParam(1).pPlaybackpos_value == NULL)
    {
        // User did not pass in the reference to write the start and stop positions
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoGetPlaybackRange() Passed in parameter invalid."));
        return PVMFErrArgument;
    }

    if (aCmd.GetParam(2).bool_value)
    {
        // Return the queued playback range
        if (iQueuedRangePresent)
        {
            *(aCmd.GetParam(0).pPlaybackpos_value) = iQueuedBeginPosition;
            *(aCmd.GetParam(1).pPlaybackpos_value) = iQueuedEndPosition;
        }
        else
        {
            // Queued range has not been set
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoGetPlaybackRange() Queued range not set"));
            return PVMFErrNotReady;
        }
    }
    else
    {
        PVMFStatus retval = PVMFSuccess;

        // Return the current playback range
        if (iCurrentBeginPosition.iIndeterminate)
        {
            // Since indeterminate, just directly copy
            *(aCmd.GetParam(0).pPlaybackpos_value) = iCurrentBeginPosition;
        }
        else
        {
            retval = ConvertFromMillisec(iCurrentBeginPosition.iPosValue.millisec_value, *(aCmd.GetParam(0).pPlaybackpos_value));
            if (retval != PVMFSuccess)
            {
                // The conversion failed.
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoGetPlaybackRange() Conversion from millisec failed (1)"));
                return retval;
            }
        }

        if (iCurrentEndPosition.iIndeterminate)
        {
            // Since indeterminate, just directly copy
            *(aCmd.GetParam(1).pPlaybackpos_value) = iCurrentEndPosition;
        }
        else
        {
            retval = ConvertFromMillisec(iCurrentEndPosition.iPosValue.millisec_value, *(aCmd.GetParam(1).pPlaybackpos_value));
            if (retval != PVMFSuccess)
            {
                // The conversion failed.
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoGetPlaybackRange() Conversion from millisec failed (2)"));
                return retval;
            }
        }
    }

    EngineCommandCompleted(aCmd.GetCmdId(), aCmd.GetContext(), PVMFSuccess);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoGetPlaybackRange() Out"));
    return PVMFSuccess;
}


PVMFStatus PVPlayerEngine::DoGetCurrentPosition(PVPlayerEngineCommand& aCmd, bool aSyncCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoGetCurrentPosition() In"));

    PVPPlaybackPosition* pbpos = aCmd.GetParam(0).pPlaybackpos_value;

    if (GetPVPlayerState() == PVP_STATE_IDLE ||
            GetPVPlayerState() == PVP_STATE_ERROR)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoGetCurrentPosition() Wrong engine state"));
        return PVMFErrInvalidState;
    }

    if (pbpos == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoGetCurrentPosition() Passed in parameter invalid."));
        return PVMFErrArgument;
    }

    // Query playback clock for current playback position
    GetPlaybackClockPosition(*pbpos);

    if (pbpos->iIndeterminate)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoGetCurrentPosition() Passed in parameter invalid."));
        return PVMFErrArgument;
    }

    if (!aSyncCmd)
    {
        EngineCommandCompleted(aCmd.GetCmdId(), aCmd.GetContext(), PVMFSuccess);
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoGetCurrentPosition() Out"));
    return PVMFSuccess;
}


PVMFStatus PVPlayerEngine::DoSetPlaybackRate(PVPlayerEngineCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSetPlaybackRate() In"));

    int32 rate = aCmd.GetParam(0).int32_value;
    PVMFTimebase* timebase = (PVMFTimebase*)(aCmd.GetParam(1).pOsclAny_value);

    // Split the rate into the absolute value plus the direction 1 or -1.
    int32 direction = 1;
    if (rate < 0)
    {
        direction = (-1);
        rate = (-rate);
    }

    // Check if called in valid states.
    if (GetPVPlayerState() != PVP_STATE_PREPARED
            && GetPVPlayerState() != PVP_STATE_STARTED
            && GetPVPlayerState() != PVP_STATE_PAUSED)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSetPlaybackRate() Wrong engine state to change rate"));
        return PVMFErrInvalidState;
    }

    // Timebase can only be changed when prepared or paused.
    if (timebase != iOutsideTimebase
            && GetPVPlayerState() != PVP_STATE_PREPARED
            && GetPVPlayerState() != PVP_STATE_PAUSED)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSetPlaybackRate() Wrong engine state to change timebase"));
        return PVMFErrInvalidState;
    }

    // Don't allow a direction change while paused, if there's already
    // a pending reposition.  Engine doesn't have logic to handle both repos and
    // direction change during the Resume.
    if (direction != iPlaybackDirection
            && GetPVPlayerState() == PVP_STATE_PAUSED
            && iChangePlaybackPositionWhenResuming)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSetPlaybackRate() Repos already pending-- can't change direction."));
        return PVMFErrInvalidState;
    }

    // Switching from forward to backward really only makes sense when playing or paused,
    // otherwise we'll be at the end of clip.  If we ever allow combined repositioning
    // and direction change, this restriction could be removed.
    if (direction != iPlaybackDirection
            && direction < 0
            && GetPVPlayerState() != PVP_STATE_STARTED
            && GetPVPlayerState() != PVP_STATE_PAUSED)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSetPlaybackRate() Wrong engine state to go backward"));
        return PVMFErrInvalidState;
    }

    // Validate the playback rate parameters.

    // Rate zero is only valid with an outside timebase.
    if (rate == 0
            && timebase == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSetPlaybackRate() Invalid parameter-- rate 0 with no outside timbase."));
        return PVMFErrArgument;
    }

    // Rate must be within allowed range
    if (rate > 0
            && (rate < PVP_PBRATE_MIN || rate > PVP_PBRATE_MAX))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSetPlaybackRate() Invalid parameter-- rate outside allowed range"));
        return PVMFErrArgument;
    }

    // With an outside timebase, we can't really support rates.  If -1x is input,
    // it means backward direction, but otherwise, rate is ignored.
    // So flag an error for any rate other than zero, 1x, or -1x.
    if (timebase != NULL
            && (rate != 0 && rate != 100000))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSetPlaybackRate() Invalid rate with outside timebase"));
        return PVMFErrInvalidState;
    }

    // To do any rate change, the source node must have the playback control IF.
    if (rate != iPlaybackClockRate
            && iSourceNodePBCtrlIF == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSetPlaybackRate() iSourceNodePBCtrlIF is NULL"));
        return PVMFFailure;
    }

    // To do any direction change, the source node must have the direction control IF.
    if (direction != iPlaybackDirection
            && iSourceNodeDirCtrlIF == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSetPlaybackRate() iSourceNodeDirCtrlIF is NULL"));
        return PVMFFailure;
    }

    // Reset the paused-due-to-EOS flag if direction changes
    if (direction != iPlaybackDirection)
    {
        iPlaybackPausedDueToEndOfClip = false;
        //a direction change also involves an internal repositioning
        //so reset repos related variables and increment iStreamID
        ResetReposVariables(false);
        iStreamID++;
    }

    // Save the new values.  They won't be installed until they're verified
    iOutsideTimebase_New = timebase;
    iPlaybackDirection_New = direction;
    iPlaybackClockRate_New = rate;

    // Start the sequence.

    if (iPlaybackClockRate_New != iPlaybackClockRate)
    {
        // This code starts a rate change.  Any direction and/or timebase change
        // will happen once the rate change is complete.

        // Query the source node if the new playback rate is supported
        PVPlayerEngineContext* context = AllocateEngineContext(NULL, iSourceNode, NULL, aCmd.GetCmdId(), aCmd.GetContext(), PVP_CMD_SourceNodeSetDataSourceRate);

        PVMFCommandId cmdid = -1;
        int32 leavecode = 0;
        OSCL_TRY(leavecode, cmdid = iSourceNodePBCtrlIF->SetDataSourceRate(iSourceNodeSessionId, iPlaybackClockRate_New, iOutsideTimebase_New, (OsclAny*)context));
        OSCL_FIRST_CATCH_ANY(leavecode,
                             FreeEngineContext(context);
                             PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSetPlaybackRate() SetDataSourceRate on iSourceNodePBCtrlIF did a leave!"));
                             return PVMFFailure);

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSetPlaybackRate() Out"));

        return PVMFSuccess;
        // wait for the source node callback, then HandleSourceNodeSetDataSourceRate
    }

    if (iPlaybackDirection_New != iPlaybackDirection)
    {
        // Do a direction change without a rate change.
        PVMFStatus status = UpdateCurrentDirection(aCmd.GetCmdId(), aCmd.GetContext());
        switch (status)
        {
            case PVMFPending:
                // If we get here, engine is Prepared or Started, and we're now
                // waiting on source node command completion followed
                // by a call to HandleSourceNodeSetDataSource.
                // Set the return status to Success, since the caller does not expect
                // PVMFPending.
                status = PVMFSuccess;
                break;
            case PVMFSuccess:
                // If we get here, engine is Paused or Stopped.  The SetPlaybackRate
                // command is done for now, but we need to set the direction when the
                // engine is resumed or prepared.
                if (iOutsideTimebase_New != iOutsideTimebase)
                {
                    UpdateTimebaseAndRate();
                }
                EngineCommandCompleted(aCmd.GetCmdId(), aCmd.GetContext(), PVMFSuccess);
                break;
            default:
                //failure!
                break;
        }
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSetPlaybackRate() Out"));
        return status;
    }

    //If we get here it's either a timebase change, or no change at all, so
    //the engine command is complete.
    if (iOutsideTimebase_New != iOutsideTimebase)
    {
        UpdateTimebaseAndRate();
    }

    EngineCommandCompleted(aCmd.GetCmdId(), aCmd.GetContext(), PVMFSuccess);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSetPlaybackRate() Out"));
    return PVMFSuccess;
}

PVMFStatus PVPlayerEngine::UpdateCurrentDirection(PVMFCommandId aCmdId, OsclAny* aCmdContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::UpdateCurrentDirection() In"));

    // Launch a direction change sequence.

    PVMFStatus status = PVMFFailure;

    // Check if the source node has direction control
    if (!iSourceNodeDirCtrlIF)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::UpdateCurrentDirection() Direction control IF on source node not available "));
        status = PVMFFailure;
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::UpdateCurrentDirection() Out"));
        return status;
    }

    switch (GetPVPlayerState())
    {
        case PVP_STATE_PREPARED:
        case PVP_STATE_STARTED:

            // Change the playback direction immediately
            status = DoSourceNodeSetDataSourceDirection(aCmdId, aCmdContext);
            if (status == PVMFSuccess)
            {
                //return Pending to indicate there is still a node command pending.
                status = PVMFPending;
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::UpdateCurrentDirection() DoSourceNodeSetDataSourceDirection failed."));
            }
            break;

        case PVP_STATE_PAUSED:
            if (iChangePlaybackPositionWhenResuming)
            {
                //if there's already a reposition pending, don't allow
                //a direction change also.
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::UpdateCurrentDirection() Reposition already pending-- can't change direction."));
                status = PVMFFailure;
            }
            else
            {
                //The command will complete now-- but the direction change
                //won't actually occur until the engine Resume command.
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::UpdateCurrentDirection() Setting iChangePlaybackDirectionWhenResuming."));
                iChangePlaybackDirectionWhenResuming = true;
                status = PVMFSuccess;
            }
            break;

        default:
            //not supported.
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::UpdateCurrentDirection() Invalid engine state"));
            status = PVMFErrInvalidState;
            break;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::UpdateCurrentDirection() Out"));
    return status;
}

PVMFStatus PVPlayerEngine::DoGetPlaybackRate(PVPlayerEngineCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoGetPlaybackRate() In"));

    int32* rate = aCmd.GetParam(0).pInt32_value;
    PVMFTimebase** timebase = (PVMFTimebase**)(aCmd.GetParam(1).pOsclAny_value);

    if (rate == NULL || timebase == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoGetPlaybackRate() Passed in parameter invalid"));
        return PVMFErrArgument;
    }

    if (GetPVPlayerState() != PVP_STATE_PREPARED &&
            GetPVPlayerState() != PVP_STATE_STARTED &&
            GetPVPlayerState() != PVP_STATE_PAUSED)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoGetPlaybackRate() Wrong engine state"));
        return PVMFErrInvalidState;
    }

    // Fill in with current engine settings for playback rate
    *rate = iPlaybackClockRate * iPlaybackDirection;
    *timebase = iOutsideTimebase;

    EngineCommandCompleted(aCmd.GetCmdId(), aCmd.GetContext(), PVMFSuccess);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoGetPlaybackRate() Out"));
    return PVMFSuccess;
}


PVMFStatus PVPlayerEngine::DoGetPlaybackMinMaxRate(PVPlayerEngineCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoGetPlaybackMinMaxRate() In"));

    int32* minrate = aCmd.GetParam(0).pInt32_value;
    int32* maxrate = aCmd.GetParam(1).pInt32_value;

    if (minrate == NULL || maxrate == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoGetPlaybackMinMaxRate() Passed in parameter invalid"));
        return PVMFErrArgument;
    }

    // Use hardcoded ranges for now
    *minrate = PVP_PBRATE_MIN;
    *maxrate = PVP_PBRATE_MAX;

    EngineCommandCompleted(aCmd.GetCmdId(), aCmd.GetContext(), PVMFSuccess);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoGetPlaybackMinMaxRate() Out"));
    return PVMFSuccess;
}


PVMFStatus PVPlayerEngine::DoPrepare(PVPlayerEngineCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_NOTICE,
                    (0, "PVPlayerEngine::DoPrepare() Tick=%d", OsclTickCount::TickCount()));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoPrepare() In"));

    if (GetPVPlayerState() == PVP_STATE_PREPARED)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoPrepare() Engine already in Prepared State"));
        EngineCommandCompleted(aCmd.GetCmdId(), aCmd.GetContext(), PVMFSuccess);
        return PVMFSuccess;
    }

    if (GetPVPlayerState() != PVP_STATE_INITIALIZED)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoPrepare() Wrong engine state"));
        return PVMFErrInvalidState;
    }

    if (iState == PVP_ENGINE_STATE_PREPARING)
    {
        // Engine is already in PREPARING STATE and doing Track selection. DoPrepare will be called everytime
        // engine completes a stage of track selection and flips the state to _TRACK_SELECTION_1_DONE etc.
        // If DoPrepare called without flipping the state, that means in _PREPARING state, do nothing here
        // just return.
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVPlayerEngine::DoPrepare() Engine state PVP_ENGINE_STATE_PREPARING - Do Nothing"));
        return PVMFSuccess;
    }

    PVMFStatus cmdstatus = PVMFFailure;

    /* Engine will call DoPrepare 4 times
     * 1) When Engine is in PVP_ENGINE_STATE_INITIALIZED state, here engine will start the Track Selection, which
     * will start with Sink Nodes.
     * 2) After Init completes on Sink nodes Engine will be in PVP_ENGINE_STATE_TRACK_SELECTION_1_DONE and Engine will
     * start creating Dec nodes and call Init on dec nodes, if needed.
     * 3) Init completion on Dec nodes will take Engine to PVP_ENGINE_STATE_TRACK_SELECTION_2_DONE and Engine will
     * start populating the Playable List after verifying parameters of different tracks. Engine after selecting
     * tracks will call Reset on Sink and Dec nodes.
     * 4) Once Reset completes on Sink and Dec nodes, Engine will be in PVP_ENGINE_STATE_TRACK_SELECTION_3_DONE and then
     * Engine will delete all the unused dec nodes.
     */
    if (iState == PVP_ENGINE_STATE_INITIALIZED)
    {
        SetEngineState(PVP_ENGINE_STATE_PREPARING);

        // Reset the paused-due-to-EOS flag
        iPlaybackPausedDueToEndOfClip = false;

        if (iDatapathList.empty() == true)
        {
            // No sink added so fail
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoPrepare() Data sinks not added."));
            return PVMFErrNotReady;
        }

        // Query cap-config based on available engine datapaths
        cmdstatus = DoSinkNodeQueryCapConfigIF(aCmd.GetCmdId(), aCmd.GetContext());
        if (cmdstatus != PVMFSuccess)
        {
            bool ehPending = CheckForPendingErrorHandlingCmd();
            if (ehPending)
            {
                // there should be no error handling queued.
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoPrepare() Already EH pending, should never happen"));
                return PVMFPending;
            }
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoPrepare() DoSinkNodeQueryCapConfigIF: failed, Add EH command"));
            iCommandCompleteStatusInErrorHandling = cmdstatus;
            iCommandCompleteErrMsgInErrorHandling = NULL;
            AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_PREPARE, NULL, NULL, NULL, false);
            return PVMFPending;
        }
    }
    else if (iState == PVP_ENGINE_STATE_TRACK_SELECTION_1_DONE)
    {
        SetEngineState(PVP_ENGINE_STATE_PREPARING);

        // Now check for the tracks which can be played only using the Sink nodes, that means no Decoder node needed.
        cmdstatus = DoSinkNodeTrackSelection(aCmd.GetCmdId(), aCmd.GetContext());
        if (cmdstatus != PVMFSuccess)
        {
            bool ehPending = CheckForPendingErrorHandlingCmd();
            if (ehPending)
            {
                // there should be no error handling queued.
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoPrepare() Already EH pending, should never happen"));
                return PVMFPending;
            }
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoPrepare() DoSinkNodeTrackSelection: failed, Add EH command"));
            iCommandCompleteStatusInErrorHandling = cmdstatus;
            iCommandCompleteErrMsgInErrorHandling = NULL;
            AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_PREPARE, NULL, NULL, NULL, false);
            return PVMFPending;
        }

        // For the tracks which cannot be played by Sink nodes only, we need to instantiate decoder nodes.
        // Create Decoder nodes and query for the cap and config IF here.
        cmdstatus = DoDecNodeQueryCapConfigIF(aCmd.GetCmdId(), aCmd.GetContext());
        if (cmdstatus != PVMFSuccess)
        {
            bool ehPending = CheckForPendingErrorHandlingCmd();
            if (ehPending)
            {
                // there should be no error handling queued.
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoPrepare() Already EH pending, should never happen"));
                return PVMFPending;
            }
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoPrepare() DoDecNodeQueryCapConfigIF: failed, Add EH command"));
            iCommandCompleteStatusInErrorHandling = cmdstatus;
            iCommandCompleteErrMsgInErrorHandling = NULL;
            AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_PREPARE, NULL, NULL, NULL, false);
            return PVMFPending;
        }
    }
    else if (iState == PVP_ENGINE_STATE_TRACK_SELECTION_2_DONE)
    {
        SetEngineState(PVP_ENGINE_STATE_PREPARING);

        cmdstatus = DoSourceNodeTrackSelection(aCmd.GetCmdId(), aCmd.GetContext());
        if (cmdstatus != PVMFSuccess)
        {
            bool ehPending = CheckForPendingErrorHandlingCmd();
            if (ehPending)
            {
                // there should be no error handling queued.
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoPrepare() Already EH pending, should never happen"));
                return PVMFPending;
            }
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoPrepare() DoSourceNodeTrackSelection: failed, Add EH command"));
            iCommandCompleteStatusInErrorHandling = cmdstatus;
            iCommandCompleteErrMsgInErrorHandling = NULL;
            AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_PREPARE, NULL, NULL, NULL, false);
            return PVMFPending;
        }
        // Reset all the sink and decoder nodes.
        cmdstatus = DoSinkNodeDecNodeReset(aCmd.GetCmdId(), aCmd.GetContext());
        if (cmdstatus != PVMFSuccess)
        {
            bool ehPending = CheckForPendingErrorHandlingCmd();
            if (ehPending)
            {
                // there should be no error handling queued.
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoPrepare() Already EH pending, should never happen"));
                return PVMFPending;
            }
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoPrepare() DoSinkNodeDecNodeReset: failed, Add EH command"));
            iCommandCompleteStatusInErrorHandling = cmdstatus;
            iCommandCompleteErrMsgInErrorHandling = NULL;
            AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_PREPARE, NULL, NULL, NULL, false);
            return PVMFPending;
        }
    }
    else if (iState == PVP_ENGINE_STATE_TRACK_SELECTION_3_DONE)
    {
        SetEngineState(PVP_ENGINE_STATE_PREPARING);

        cmdstatus = DoSinkDecCleanupSourcePrepare(aCmd.GetCmdId(), aCmd.GetContext());
        if (cmdstatus != PVMFSuccess)
        {
            bool ehPending = CheckForPendingErrorHandlingCmd();
            if (ehPending)
            {
                // there should be no error handling queued.
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoPrepare() Already EH pending, should never happen"));
                return PVMFPending;
            }
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoPrepare() DoDecNodeCleanup: failed, Add EH command"));
            iCommandCompleteStatusInErrorHandling = cmdstatus;
            iCommandCompleteErrMsgInErrorHandling = NULL;
            AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_PREPARE, NULL, NULL, NULL, false);
            return PVMFPending;
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoPrepare() Out"));
    return PVMFSuccess;
}

PVMFStatus PVPlayerEngine::DoSinkNodeQueryCapConfigIF(PVCommandId aCmdId, OsclAny* aCmdContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSinkNodeQueryCapConfigIF() In"));

    if (iSourceNodeTrackSelIF == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSinkNodeQueryCapConfigIF() Source node track sel IF not available. Asserting"));
        return PVMFFailure;
    }

    uint32 i = 0;
    int32 leavecode = 0;
    uint32 numTracks = 0;
    PVPlayerEngineContext* context = NULL;
    PVMFCommandId cmdid = -1;
    iNumPendingNodeCmd = 0;

    if (iSourceNodeTrackSelIF->GetMediaPresentationInfo(iSourcePresInfoList) != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSinkNodeQueryCapConfigIF() GetMediaPresentationInfo() call on source node failed"));
        return PVMFFailure;
    }

    numTracks = iSourcePresInfoList.getNumTracks();
    iTrackSelectionList.reserve(numTracks);

    for (i = 0; i < numTracks; i++)
    {
        // Create the track selection list, which will store a set of sink and dec node (if needed) for each track.
        PVPlayerEngineTrackSelection trackSelection;

        PVMFTrackInfo* trackInfo = iSourcePresInfoList.getTrackInfo(i);
        trackSelection.iTsTrackID = trackInfo->getTrackID();

        iTrackSelectionList.push_back(trackSelection);
    }

    for (i = 0;i < iDatapathList.size(); ++i)
    {
        // Destroy the track info if present
        if (iDatapathList[i].iTrackInfo)
        {
            OSCL_DELETE(iDatapathList[i].iTrackInfo);
            iDatapathList[i].iTrackInfo = NULL;
        }

        if (iDatapathList[i].iDataSink->GetDataSinkType() == PVP_DATASINKTYPE_FILENAME)
        {
            // Create file output node for sink
            leavecode = 0;
            OSCL_TRY(leavecode, iDatapathList[i].iSinkNode = PVFileOutputNodeFactory::CreateFileOutput());
            OSCL_FIRST_CATCH_ANY(leavecode,
                                 PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSinkNodeQueryCapConfigIF() Creation of file output node did a leave!"));
                                 return PVMFErrNoMemory);
        }
        else if (iDatapathList[i].iDataSink->GetDataSinkType() == PVP_DATASINKTYPE_SINKNODE)
        {
            // Use the specified output node for sink node
            iDatapathList[i].iSinkNode = iDatapathList[i].iDataSink->GetDataSinkNodeInterface();
            if (iDatapathList[i].iSinkNode == NULL)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSinkNodeQueryCapConfigIF() Passed in sink node is NULL"));
                return PVMFFailure;
            }
        }
        else
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSinkNodeQueryCapConfigIF() Unsupported player data sink type"));
            return PVMFErrNotSupported;
        }

        if (iDatapathList[i].iSinkNode->ThreadLogon() != PVMFSuccess)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSinkNodeQueryCapConfigIF() ThreadLogon() on passed-in sink node failed"));
            OSCL_ASSERT(false);
            return PVMFFailure;
        }

        PVMFNodeSessionInfo nodesessioninfo(this, this, (OsclAny*)iDatapathList[i].iSinkNode, this, (OsclAny*)iDatapathList[i].iSinkNode);
        iDatapathList[i].iSinkNodeSessionId = iDatapathList[i].iSinkNode->Connect(nodesessioninfo);

        // Query for Cap-Config IF
        context = AllocateEngineContext(&iDatapathList[i], iDatapathList[i].iSinkNode, NULL, aCmdId, aCmdContext, PVP_CMD_SinkNodeQueryCapConfigIF);

        PVUuid capconfiguuid = PVMI_CAPABILITY_AND_CONFIG_PVUUID;
        cmdid = -1;
        leavecode = 0;
        iDatapathList[i].iSinkNodePVInterfaceCapConfig = NULL;
        leavecode = IssueQueryInterface(iDatapathList[i].iSinkNode, iDatapathList[i].iSinkNodeSessionId, capconfiguuid, iDatapathList[i].iSinkNodePVInterfaceCapConfig, (OsclAny*)context, cmdid);
        if (leavecode != 0 || cmdid == -1)
        {
            iDatapathList[i].iSinkNodePVInterfaceCapConfig = NULL;
            FreeEngineContext(context);
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSinkNodeQueryCapConfigIF() QueryInterface on sink node for cap-config IF did a leave!"));
            return PVMFFailure;
        }
        else
        {
            ++iNumPendingNodeCmd;
        }
    }

    if (iNumPendingNodeCmd <= 0)
    {
        // NumPendingNodeCmd less than or equal to zero means that none of the Sink nodes support Cap-Config interface, which means that these
        // sinks cannot be used for playing the content. Return PVMFErrNotSupported from here which will take engine into Error handling and will fail
        // Prepare command.
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSinkNodeQueryCapConfigIF() Out No pending QueryInterface() on sink node"));
        return PVMFErrNotSupported;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSinkNodeQueryCapConfigIF() Out"));
    return PVMFSuccess;
}

PVMFStatus PVPlayerEngine::DoSinkNodeInit(PVCommandId aCmdId, OsclAny* aCmdContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_INFO,
                    (0, "PVPlayerEngine::DoSinkNodeInit() Tick=%d", OsclTickCount::TickCount()));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSinkNodeInit() In"));

    iNumPendingNodeCmd = 0;
    PVMFCommandId cmdid = -1;
    int32 leavecode = 0;

    for (uint32 i = 0; i < iDatapathList.size(); ++i)
    {
        if (iDatapathList[i].iSinkNode != NULL)
        {
            // Call Init() on the sink node
            PVPlayerEngineContext* context = AllocateEngineContext(&(iDatapathList[i]), iDatapathList[i].iSinkNode, NULL, aCmdId, aCmdContext, PVP_CMD_SinkNodeInit);

            leavecode = IssueSinkNodeInit(&(iDatapathList[i]), (OsclAny*) context, cmdid);

            if (cmdid != -1 && leavecode == 0)
            {
                ++iNumPendingNodeCmd;
            }
            else
            {
                FreeEngineContext(context);
                return PVMFFailure;
            }
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSinkNodeInit() Out"));
    return PVMFSuccess;
}

PVMFStatus PVPlayerEngine::DoSinkNodeTrackSelection(PVCommandId aCmdId, OsclAny* aCmdContext)
{
    OSCL_UNUSED_ARG(aCmdId);
    OSCL_UNUSED_ARG(aCmdContext);
    // For a track to be playable only by sink node, the Sink node should support the Format Type and Format Specific Info
    // for the track. If any of the 2 variables are not supported by the sink node, the track needs to have a decoder which
    // will be created in next stage of track selection. If Sink node supports both the above variables for a particular
    // track, the track will make to the Playable list without any decoder node.
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSinkNodeTrackSelection() In"));

    PVMFStatus status = PVMFFailure;

    PvmiKvp kvpFormatType;
    PvmiKvp kvpFSI;

    OSCL_StackString<64> iKVPFormatType = _STRLIT_CHAR(PVMF_FORMAT_TYPE_VALUE_KEY);

    const char* aFormatValType = PVMF_FORMAT_SPECIFIC_INFO_KEY;

    OsclMemAllocator alloc;

    kvpFormatType.key = NULL;
    kvpFSI.key = NULL;

    kvpFormatType.key = iKVPFormatType.get_str();

    kvpFSI.length = oscl_strlen(aFormatValType) + 1; // +1 for \0
    kvpFSI.key = (PvmiKeyType)alloc.ALLOCATE(kvpFSI.length);
    if (kvpFSI.key == NULL)
    {
        return PVMFErrNoMemory;
    }
    oscl_strncpy(kvpFSI.key, aFormatValType, kvpFSI.length);

    for (uint32 i = 0; i < iDatapathList.size(); i++)
    {
        if (iDatapathList[i].iSinkNodeCapConfigIF != NULL)
        {
            for (uint32 j = 0; j < iSourcePresInfoList.getNumTracks(); j++)
            {
                // if any track is already been added to the playlist then no need to check with the next Datapath
                // go onto the next track
                if (!iTrackSelectionList[j].iTsTrackValidForPlayableList)
                {
                    OsclRefCounterMemFrag aConfig;
                    PVMFTrackInfo* currTrack = iSourcePresInfoList.getTrackInfo(j);
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSinkNodeTrackSelection() Check format type for %s", currTrack->getTrackMimeType().get_cstr()));

                    kvpFormatType.value.pChar_value = currTrack->getTrackMimeType().get_str();

                    // Check for the format type of the track first. If Supported move to Format specific info, if not
                    // move on to the next track.

                    status = iDatapathList[i].iSinkNodeCapConfigIF->verifyParametersSync(NULL, &kvpFormatType, 1);

                    if (status == PVMFSuccess)
                    {
                        // go ahead and check for Format specific info
                        aConfig = currTrack->getTrackConfigInfo();
                        kvpFSI.value.key_specific_value = (OsclAny*)(aConfig.getMemFragPtr());
                        kvpFSI.capacity = aConfig.getMemFragSize();

                        status = iDatapathList[i].iSinkNodeCapConfigIF->verifyParametersSync(NULL, &kvpFSI, 1);

                        if (status == PVMFSuccess)
                        {
                            // This track can be played just using the Sink nodes we need not have decoders for this track
                            // Set the boolean iTsTrackValidForPlayableList to true in the TrackSelectionList.
                            // Check the boolean iTsTrackValidForPlayableList before creating the decoder nodes, if
                            // already set no need for decoders.
                            iTrackSelectionList[j].iTsSinkNode = iDatapathList[i].iSinkNode;
                            iTrackSelectionList[j].iTsSinkNodeCapConfigIF = iDatapathList[i].iSinkNodeCapConfigIF;
                            iTrackSelectionList[j].iTsTrackValidForPlayableList = true;
                        }
                    }
                }
                // if any of the above verifyParameterSync returns a failure, just move onto the next track.
            }
        }
    }

    alloc.deallocate((OsclAny*)(kvpFSI.key));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSinkNodeTrackSelection() Out"));
    return PVMFSuccess;
}

PVMFStatus PVPlayerEngine::DoDecNodeQueryCapConfigIF(PVCommandId aCmdId, OsclAny* aCmdContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoDecNodeQueryCapConfigIF() In"));

    int32 leavecode = 0;
    PVPlayerEngineContext* context = NULL;
    PVMFCommandId cmdid = -1;

    PVMFFormatType iSrcFormat = 0;
    PVMFFormatType iSinkFormat = 0;

    iNumPendingNodeCmd = 0;

    uint32 numTracks = iSourcePresInfoList.getNumTracks();

    for (uint32 i = 0; i < numTracks; i++)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoDecNodeQueryCapConfigIF() Check the track"));
        for (uint32 j = 0; j < iDatapathList.size(); j++)
        {
            // Start creating decoder nodes for tracks which cannot be played with Sink nodes alone.
            // It is also possible that there can be similar tracks with same mime strings but with
            // different config parameters which share the same decoder node. For these similar tracks
            // engine should not create decoder nodes again, it should use the same decoder node instance.
            if (iTrackSelectionList[i].iTsDecNode == NULL && !iTrackSelectionList[i].iTsTrackValidForPlayableList)
            {
                PVMFTrackInfo* currTrack = iSourcePresInfoList.getTrackInfo(i);
                iSrcFormat = currTrack->getTrackMimeType().get_str();

                //Try to get supported formats from the media I/O component.
                PvmiKvp* kvp = NULL;
                int numParams = 0;
                PVMFStatus status = iDatapathList[j].iSinkNodeCapConfigIF->getParametersSync(NULL, (char*)INPUT_FORMATS_CAP_QUERY, kvp, numParams, NULL);
                if (status == PVMFSuccess)
                {
                    for (int k = 0; k < numParams; k++)
                    {
                        iSinkFormat = kvp[k].value.pChar_value;

                        Oscl_Vector<PVUuid, OsclMemAllocator> foundUuids;
                        // Query the player node registry for the required decoder node
                        if (iPlayerNodeRegistry.QueryRegistry(iSrcFormat, iSinkFormat, foundUuids) == PVMFSuccess)
                        {
                            if (!foundUuids.empty())
                            {
                                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoDecNodeQueryCapConfigIF() Node found for %s, sink %s", currTrack->getTrackMimeType().get_str(), iSinkFormat.getMIMEStrPtr()));
                                iTrackSelectionList[i].iTsDecNode = iPlayerNodeRegistry.CreateNode(foundUuids[0]);

                                if (iTrackSelectionList[i].iTsDecNode != NULL)
                                {
                                    iNodeUuids.push_back(PVPlayerEngineUuidNodeMapping(foundUuids[0], iTrackSelectionList[i].iTsDecNode));

                                    if (iTrackSelectionList[i].iTsDecNode->ThreadLogon() != PVMFSuccess)
                                    {
                                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoDecNodeQueryCapConfigIF() ThreadLogon() on dec node failed"));
                                        OSCL_ASSERT(false);
                                    }

                                    PVMFNodeSessionInfo nodesessioninfo(this, this, (OsclAny*)iTrackSelectionList[i].iTsDecNode, this, (OsclAny*)iTrackSelectionList[i].iTsDecNode);
                                    iTrackSelectionList[i].iTsDecNodeSessionId = iTrackSelectionList[i].iTsDecNode->Connect(nodesessioninfo);

                                    // Query for CapConfig IF
                                    context = AllocateEngineContext(NULL, iTrackSelectionList[i].iTsDecNode, NULL, aCmdId, aCmdContext, PVP_CMD_DecNodeQueryCapConfigIF);

                                    PVUuid capconfiguuid = PVMI_CAPABILITY_AND_CONFIG_PVUUID;
                                    cmdid = -1;
                                    iTrackSelectionList[i].iTsDecNodePVInterfaceCapConfig = NULL;
                                    leavecode = IssueQueryInterface(iTrackSelectionList[i].iTsDecNode, iTrackSelectionList[i].iTsDecNodeSessionId, capconfiguuid, iTrackSelectionList[i].iTsDecNodePVInterfaceCapConfig, (OsclAny*)context, cmdid);
                                    if (cmdid == -1 || leavecode != 0)
                                    {
                                        iTrackSelectionList[i].iTsDecNodePVInterfaceCapConfig = NULL;
                                        FreeEngineContext(context);
                                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoDecNodeQueryCapConfigIF() QueryInterface on dec node for cap-config IF did a leave!"));
                                    }
                                    else
                                    {
                                        ++iNumPendingNodeCmd;
                                    }

                                    // A decoder is found in the registry and succesfully created for the particular set of track and datapath.
                                    // Set the sink nodes and its cap and config IF for the track in trackSelectionList
                                    iTrackSelectionList[i].iTsSinkNode = iDatapathList[j].iSinkNode;
                                    iTrackSelectionList[i].iTsSinkNodeCapConfigIF = iDatapathList[j].iSinkNodeCapConfigIF;
                                    iTrackSelectionList[i].iTsSinkNodeSessionId = iDatapathList[j].iSinkNodeSessionId;

                                    // Set the sink format for the datapath.
                                    iDatapathList[j].iSinkFormat = iSinkFormat;

                                    // Valid decoder node set in TrackSelectionList. Scan the TrackSelectionList further and if
                                    // any similar MIME track is there just use the same decoder and the same sink nodes.
                                    for (uint32 s = i + 1; s < numTracks; s++)
                                    {
                                        PVMFTrackInfo* tmpTrack = iSourcePresInfoList.getTrackInfo(s);
                                        if (!(pv_mime_strcmp(currTrack->getTrackMimeType().get_str(), tmpTrack->getTrackMimeType().get_str())))
                                        {
                                            iTrackSelectionList[s].iTsSinkNode = iTrackSelectionList[i].iTsSinkNode;
                                            iTrackSelectionList[s].iTsSinkNodeCapConfigIF = iTrackSelectionList[i].iTsSinkNodeCapConfigIF;
                                            iTrackSelectionList[s].iTsSinkNodeSessionId = iTrackSelectionList[i].iTsSinkNodeSessionId;

                                            iTrackSelectionList[s].iTsDecNode = iTrackSelectionList[i].iTsDecNode;
                                            iTrackSelectionList[s].iTsDecNodeSessionId = iTrackSelectionList[i].iTsDecNodeSessionId;
                                        }
                                    }

                                    k = numParams;
                                }
                                else
                                {
                                    // Create node on decoder failed, check with the next sink format
                                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoDecNodeQueryCapConfigIF() Dec node creation failed"));
                                }
                            }
                            else
                            {
                                // No matching node found with the given Sinkformat, check with the next Sink Format
                                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoDecNodeQueryCapConfigIF() No matching decoder node found"));
                            }
                        }
                        else
                        {
                            // Registry query failed with the given Sinkformat, check with the next Sink Format.
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoDecNodeQueryCapConfigIF() Registry query for dec node failed"));
                        }
                    }
                    iDatapathList[j].iSinkNodeCapConfigIF->releaseParameters(0, kvp, numParams);
                }
                else
                {
                    // getParamterSync on MIO node to get the supported formats by MIO failed
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoDecNodeQueryCapConfigIF() getParamterSync on MIO node to get the supported formats by MIO failed"));
                }
            }
        }
    }

    if (iNumPendingNodeCmd == 0)
    {
        // no decoder nodes are needed, go ahead for track selection logic
        SetEngineState(PVP_ENGINE_STATE_TRACK_SELECTION_2_DONE);
        RunIfNotReady();
    }
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoDecNodeQueryCapConfigIF() Out"));
    return PVMFSuccess;
}

PVMFStatus PVPlayerEngine::DoDecNodeInit(PVCommandId aCmdId, OsclAny* aCmdContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_INFO,
                    (0, "PVPlayerEngine::DoDecNodeInit() Tick=%d", OsclTickCount::TickCount()));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoDecNodeInit() In"));

    iNumPendingNodeCmd = 0;
    PVMFCommandId cmdid = -1;
    int32 leavecode = 0;

    for (uint32 i = 0; i < iTrackSelectionList.size(); ++i)
    {
        if (iTrackSelectionList[i].iTsDecNode != NULL)
        {
            // Call Init() on the dec node
            PVPlayerEngineContext* context = AllocateEngineContext(NULL, iTrackSelectionList[i].iTsDecNode, NULL, aCmdId, aCmdContext, PVP_CMD_DecNodeInit);

            leavecode = IssueDecNodeInit(iTrackSelectionList[i].iTsDecNode, iTrackSelectionList[i].iTsDecNodeSessionId, (OsclAny*) context, cmdid);

            if (cmdid != -1 && leavecode == 0)
            {
                ++iNumPendingNodeCmd;
            }
            else
            {
                FreeEngineContext(context);
                return PVMFFailure;
            }
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoDecNodeInit() Out"));
    return PVMFSuccess;
}

PVMFStatus PVPlayerEngine::DoSourceNodeTrackSelection(PVCommandId /*aCmdId*/, OsclAny* /*aCmdContext*/)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSourceNodeTrackSelection() In"));

    if (iSourceNodeTrackSelIF == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSourceNodeTrackSelection() Source node track sel IF not available."));
        return PVMFFailure;
    }

    //populate playable list first
    PVMFStatus retval = DoTrackSelection(true, false);
    if (retval != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSourceNodeTrackSelection() DoTrackSelection - Populating playable list Failed"));
        return retval;
    }

    bool usepreferencelist = false;
    if (iTrackSelectionHelper != NULL)
    {
        PVMFMediaPresentationInfo localList;
        iPreferenceList.Reset();
        localList.setPresentationType(iPlayableList.getPresentationType());
        localList.setSeekableFlag(iPlayableList.IsSeekable());
        localList.SetDurationAvailable(iPlayableList.IsDurationAvailable());
        localList.setDurationValue(iPlayableList.getDurationValue());
        localList.setDurationTimeScale(iPlayableList.getDurationTimeScale());
        //if track selection helper is present, it means that
        //user of engine wants to provide inputs
        //the reason we use a local list instead of iPreferenceList is
        //due to memory consideration. This call to "SelectTracks" goes
        //to the app and the app allocates memory to populate the local list
        //This memory needs to be released right away. So we make a copy
        //and release the memory for local list.
        PVMFStatus status =
            iTrackSelectionHelper->SelectTracks(iPlayableList, localList);
        if ((status == PVMFSuccess) &&
                (localList.getNumTracks() != 0))
        {
            usepreferencelist = true;
            iPreferenceList = localList;
        }
        //release memory now that we have made a copy
        iTrackSelectionHelper->ReleasePreferenceList(localList);
        //else user made no choice, use playable list
    }

    retval = DoTrackSelection(false, usepreferencelist);
    if (retval != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSourceNodeTrackSelection() DoTrackSelection - TrackSelection Failed"));
        return retval;
    }

    uint32 i = 0;

    // Create a selected track list
    PVMFMediaPresentationInfo selectedtracks;
    for (i = 0; i < iDatapathList.size(); ++i)
    {
        if (iDatapathList[i].iTrackInfo != NULL)
        {
            selectedtracks.addTrackInfo(*(iDatapathList[i].iTrackInfo));
        }
    }

    // Check that at least one track was selected
    if (selectedtracks.getNumTracks() == 0)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSourceNodeTrackSelection() No tracks were selected"));
        // @TODO Provide a more specific error info
        return PVMFErrResourceConfiguration;
    }

    // Select in source node
    retval = iSourceNodeTrackSelIF->SelectTracks(selectedtracks);
    if (retval != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSourceNodeTrackSelection() SelectTracks() on source node failed"));
        return retval;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSourceNodeTrackSelection() Out"));
    return retval;
}

PVMFStatus PVPlayerEngine::DoTrackSelection(bool oPopulatePlayableListOnly, bool oUsePreferenceList)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoTrackSelection() In"));

    if (iSourceNodeTrackSelIF == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoTrackSelection() Source node track sel IF not available."));
        return PVMFFailure;
    }

    PVMFMediaPresentationInfo sourcepresinfo;
    if (oPopulatePlayableListOnly)
    {
        // use already saved presentation info list from source node
        sourcepresinfo = iSourcePresInfoList;

        iPlayableList.Reset();

        iPlayableList.setPresentationType(iSourcePresInfoList.getPresentationType());
        iPlayableList.setSeekableFlag(iSourcePresInfoList.IsSeekable());
        iPlayableList.SetDurationAvailable(iSourcePresInfoList.IsDurationAvailable());
        iPlayableList.setDurationValue(iSourcePresInfoList.getDurationValue());
        iPlayableList.setDurationTimeScale(iSourcePresInfoList.getDurationTimeScale());
    }
    else
    {
        if (oUsePreferenceList)
        {
            //perform track selection based on playable list
            sourcepresinfo = iPreferenceList;
        }
        else
        {
            //perform track selection based on playable list
            sourcepresinfo = iPlayableList;
        }
    }

    PVMFStatus retVal = PVMFSuccess;
    uint32 i = 0;
    uint32 k = 0;

    if (oPopulatePlayableListOnly)
    {
        for (i = 0; i < iDatapathList.size(); i++)
        {
            // Destroy the track info if present
            if (iDatapathList[i].iTrackInfo)
            {
                OSCL_DELETE(iDatapathList[i].iTrackInfo);
                iDatapathList[i].iTrackInfo = NULL;
            }
        }

        for (i = 0; i < sourcepresinfo.getNumTracks(); i++)
        {
            PVMFStatus checkcodec = PVMFFailure;
            int32 trackId = -1;

            // Go through each track, check codec type, and save the track info
            PVMFTrackInfo* curtrack = sourcepresinfo.getTrackInfo(i);
            trackId = curtrack->getTrackID();

            // check if this track can be directly pushed in playable list. This will be the case where decoder node is not needed
            // and Sink node supports the format and format specific info. OR
            // The track is a text track.

            if (iTrackSelectionList[i].iTsDecNode == NULL)
            {
                // check if it is a valid track or not, decoder node can be NULL only in 2 cases
                // 1) Track can be played without decoder nodes, here iTsTrackValidForPlayableList should be true OR track is TEXT track
                // 2) If the track is not valid at all, if this is the case, move onto next track.

                if (iTrackSelectionList[i].iTsTrackValidForPlayableList ||
                        (pv_mime_strcmp(curtrack->getTrackMimeType().get_str(), PVMF_MIME_3GPP_TIMEDTEXT)) == 0)
                {
                    // this can make directly to the Playable list, since it satisfies condition# 1 above.
                    iPlayableList.addTrackInfo(*curtrack);
                    iTrackSelectionList[i].iTsTrackValidForPlayableList = false;
                    checkcodec = PVMFSuccess;
                }
            }
            else
            {
                // if sink node alone does not support this track verify its parameters.
                retVal = DoVerifyTrackInfo(iTrackSelectionList[i], curtrack, checkcodec);
                if (retVal != PVMFSuccess)
                    return retVal;

                if (checkcodec == PVMFSuccess)
                {
                    //add it to playable list
                    iPlayableList.addTrackInfo(*curtrack);
                }
            }

            if (checkcodec != PVMFSuccess && trackId >= 0)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoTrackSelection() Bad track config for TrackId=%d", trackId));
                SendInformationalEvent(PVMFInfoTrackDisable, NULL, (OsclAny*)trackId, NULL, 0);
            }
        }
    }
    else
    {
        for (i = 0; i < sourcepresinfo.getNumTracks(); i++)
        {
            PVMFTrackInfo* curtrack = sourcepresinfo.getTrackInfo(i);

            for (k = 0; k < iTrackSelectionList.size(); k++)
            {
                // check which track is selected by comparing the TrackId
                int32 trackId = curtrack->getTrackID();
                if (trackId == iTrackSelectionList[k].iTsTrackID)
                {
                    for (uint32 j = 0; j < iDatapathList.size(); j++)
                    {
                        // if a track has already been added to the datapath, move onto the next datapath.
                        if (!iDatapathList[j].iTrackInfo)
                        {
                            // check for the corresponding datapath to the track
                            if (iDatapathList[j].iSinkNode == iTrackSelectionList[k].iTsSinkNode)
                            {
                                if (curtrack->DoesTrackHaveDependency() == true)
                                {
                                    // Track has dependency, move onto next track
                                    j = iDatapathList.size();
                                    k = iTrackSelectionList.size();
                                }
                                else
                                {
                                    // The track has been added to the final list, this means that this track is valid
                                    // and will make to DatapathList and all other video decoders will be destroyed.
                                    iDatapathList[j].iTrackInfo = OSCL_NEW(PVMFTrackInfo, (*curtrack));

                                    // set the decoder node, its session id and decoder cap&config IF in the Datapath, since the track is the
                                    // selected one.
                                    if (iTrackSelectionList[k].iTsDecNode)
                                    {
                                        iDatapathList[j].iDecNode = iTrackSelectionList[k].iTsDecNode;
                                        iDatapathList[j].iDecNodeSessionId = iTrackSelectionList[k].iTsDecNodeSessionId;
                                        iDatapathList[j].iDecNodeCapConfigIF = iTrackSelectionList[k].iTsDecNodeCapConfigIF;
                                    }

                                    iTrackSelectionList[k].iTsSinkNode = NULL;
                                    iTrackSelectionList[k].iTsSinkNodeSessionId = 0;
                                    iTrackSelectionList[k].iTsSinkNodeCapConfigIF = NULL;

                                    iTrackSelectionList[k].iTsDecNode = NULL;
                                    iTrackSelectionList[k].iTsDecNodeSessionId = 0;
                                    iTrackSelectionList[k].iTsDecNodeCapConfigIF = NULL;
                                    iTrackSelectionList[k].iTsDecNodePVInterfaceCapConfig = NULL;

                                    iTrackSelectionList[k].iTsTrackID = -1;
                                    iTrackSelectionList[k].iTsTrackValidForPlayableList = false;
                                }
                                j = iDatapathList.size();
                            }
                            // Datapath sinknode does not match, check the next datapath for the track
                        }
                        // a track has already been assigned for the datapath, check next datapath for the track
                    }
                    k = iTrackSelectionList.size();
                }
                // The trackId of the track does not match with the track in the List, check the next track.
            }
        }

        // Go through the track selection list and set similar decoder nodes to NULL. There should be only 1 entry
        // of a decoder node either in DatapathList (this will be for the final playable track) or in
        // TrackSelectionList which will be for tracks which will not be used for playback.
        for (i = 0; i < iTrackSelectionList.size(); i++)
        {
            PVMFTrackInfo* currTrack = iSourcePresInfoList.getTrackInfo(i);
            for (uint32 j = i + 1; j < iTrackSelectionList.size(); j++)
            {
                PVMFTrackInfo* tmpTrack = iSourcePresInfoList.getTrackInfo(j);
                if (!(pv_mime_strcmp(currTrack->getTrackMimeType().get_str(), tmpTrack->getTrackMimeType().get_str())))
                {
                    iTrackSelectionList[j].iTsDecNode = NULL;
                    iTrackSelectionList[j].iTsDecNodeSessionId = 0;
                    iTrackSelectionList[j].iTsDecNodeCapConfigIF = NULL;
                }
            }
        }

        // now go through whole TrackSelectionList and look for similar sink and decoder nodes
        // to the track selected i.e. added to datapath, set all sink and decoder nodes to NULL for the track selected
        for (k = 0; k < iDatapathList.size(); k++)
        {
            for (uint32 s = 0; s < iTrackSelectionList.size(); s++)
            {
                if ((iDatapathList[k].iSinkNode == iTrackSelectionList[s].iTsSinkNode))
                {
                    iTrackSelectionList[s].iTsSinkNode = NULL;
                    iTrackSelectionList[s].iTsSinkNodeSessionId = 0;
                    iTrackSelectionList[s].iTsSinkNodeCapConfigIF = NULL;
                }

                if ((iDatapathList[k].iDecNode != NULL) &&
                        (iDatapathList[k].iDecNode == iTrackSelectionList[s].iTsDecNode))
                {
                    iTrackSelectionList[s].iTsDecNode = NULL;
                    iTrackSelectionList[s].iTsDecNodeSessionId = 0;
                    iTrackSelectionList[s].iTsDecNodeCapConfigIF = NULL;
                    iTrackSelectionList[s].iTsDecNodePVInterfaceCapConfig = NULL;
                }

                iTrackSelectionList[s].iTsTrackID = -1;
                iTrackSelectionList[s].iTsTrackValidForPlayableList = false;
            }
        }
    }
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoTrackSelection() Out"));
    return retVal;
}

PVMFStatus PVPlayerEngine::DoVerifyTrackInfo(PVPlayerEngineTrackSelection &aTrackSelection, PVMFTrackInfo* aTrack, PVMFStatus& aCheckcodec)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoVerifyTrackInfo() In %s", aTrack->getTrackMimeType().get_cstr()));

    PVMFStatus status = PVMFSuccess;
    OsclMemAllocator alloc;
    PvmiKvp kvp;
    kvp.key = NULL;

    const char* aFormatValType = PVMF_FORMAT_SPECIFIC_INFO_KEY;
    OsclRefCounterMemFrag aConfig;

    kvp.length = oscl_strlen(aFormatValType) + 1; // +1 for \0
    kvp.key = (PvmiKeyType)alloc.ALLOCATE(kvp.length);
    if (kvp.key == NULL)
    {
        return PVMFErrNoMemory;
    }
    oscl_strncpy(kvp.key, aFormatValType, kvp.length);
    aConfig = aTrack->getTrackConfigInfo();
    kvp.value.key_specific_value = (OsclAny*)(aConfig.getMemFragPtr());
    kvp.capacity = aConfig.getMemFragSize();

    //Check if we have decoder node cap-config
    if (aTrackSelection.iTsDecNodeCapConfigIF != NULL)
    {
        PVMFFormatType DecnodeFormatType = aTrack->getTrackMimeType().get_str();

        PvmiKvp* iErrorKVP = NULL;
        PvmiKvp iKVPSetFormat;
        iKVPSetFormat.key = NULL;
        OSCL_StackString<64> iKeyStringSetFormat;
        iKVPSetFormat.value.pChar_value = (char*)DecnodeFormatType.getMIMEStrPtr();

        // Query for video decoder first with the track, if no success, then check for audio decoder. Only one query will succeed.
        // If both fails, check the status.
        iKeyStringSetFormat = _STRLIT_CHAR(PVMF_VIDEO_DEC_FORMAT_TYPE_VALUE_KEY);
        iKVPSetFormat.key = iKeyStringSetFormat.get_str();

        aTrackSelection.iTsDecNodeCapConfigIF->setParametersSync(NULL, &iKVPSetFormat, 1, iErrorKVP);
        if (iErrorKVP == NULL)
        {
            //verify codec specific info
            int32 leavecode = 0;
            OSCL_TRY(leavecode, aCheckcodec = aTrackSelection.iTsDecNodeCapConfigIF->verifyParametersSync(NULL, &kvp, 1));
            OSCL_FIRST_CATCH_ANY(leavecode,
                                 PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoVerifyTrackInfo() unsupported verifyParametersSync did a leave!"));
                                 alloc.deallocate((OsclAny*)(kvp.key));
                                 aCheckcodec = PVMFSuccess; // set it success in case track selection info is not yet available;
                                 return PVMFSuccess;);

            if (aCheckcodec != PVMFSuccess)
            {
                alloc.deallocate((OsclAny*)(kvp.key));
                //In case of other error code, this is operation error.
                if (aCheckcodec != PVMFErrNotSupported)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoVerifyTrackInfo() verifyParametersSync() on decoder node failed"));
                    return aCheckcodec;
                }
                return status;
            }

            int numKvp = 0;
            PvmiKvp* kvpPtr;
            // Query using get
            OSCL_StackString<64> querykey;

            querykey = _STRLIT_CHAR("x-pvmf/video/render");
            if (aTrackSelection.iTsDecNodeCapConfigIF->getParametersSync(NULL, querykey.get_str(), kvpPtr, numKvp, NULL) == PVMFSuccess)
            {
                //verify width/height
                if (aTrackSelection.iTsSinkNodeCapConfigIF != NULL)
                    aCheckcodec = aTrackSelection.iTsSinkNodeCapConfigIF->verifyParametersSync(NULL, kvpPtr, numKvp);
                status = aTrackSelection.iTsDecNodeCapConfigIF->releaseParameters(NULL, kvpPtr, numKvp);
            }
        }
        else
        {
            // Query failed for video decoder next try the audio decoder.
            iErrorKVP = NULL;
            iKeyStringSetFormat = NULL;
            iKVPSetFormat.key = NULL;

            iKeyStringSetFormat += _STRLIT_CHAR(PVMF_AUDIO_DEC_FORMAT_TYPE_VALUE_KEY);
            iKVPSetFormat.key = iKeyStringSetFormat.get_str();

            aTrackSelection.iTsDecNodeCapConfigIF->setParametersSync(NULL, &iKVPSetFormat, 1, iErrorKVP);

            if (iErrorKVP == NULL)
            {
                //verify codec specific info
                int32 leavecodeaudio = 0;
                OSCL_TRY(leavecodeaudio, aCheckcodec = aTrackSelection.iTsDecNodeCapConfigIF->verifyParametersSync(NULL, &kvp, 1));
                OSCL_FIRST_CATCH_ANY(leavecodeaudio,
                                     PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoVerifyTrackInfo() unsupported verifyParametersSync did a leave!"));
                                     alloc.deallocate((OsclAny*)(kvp.key));
                                     aCheckcodec = PVMFSuccess; // set it success in case track selection info is not yet available;
                                     return PVMFSuccess;);

                if (aCheckcodec != PVMFSuccess)
                {
                    alloc.deallocate((OsclAny*)(kvp.key));
                    //In case of other error code, this is operation error.
                    if (aCheckcodec != PVMFErrNotSupported)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoVerifyTrackInfo() verifyParametersSync() on decoder node failed"));
                        return aCheckcodec;
                    }
                    return status;
                }

                int numKvp = 0;
                PvmiKvp* kvpPtr;
                // Query using get
                OSCL_StackString<64> querykey;

                querykey = _STRLIT_CHAR("x-pvmf/audio/render");
                if (aTrackSelection.iTsDecNodeCapConfigIF->getParametersSync(NULL, querykey.get_str(), kvpPtr, numKvp, NULL) == PVMFSuccess)
                {
                    //verify samplerate and channels
                    if (aTrackSelection.iTsSinkNodeCapConfigIF != NULL)
                        aCheckcodec = aTrackSelection.iTsSinkNodeCapConfigIF->verifyParametersSync(NULL, kvpPtr, numKvp);
                    status = aTrackSelection.iTsDecNodeCapConfigIF->releaseParameters(NULL, kvpPtr, numKvp);
                }
            }
        }
    }
    else
    {
        if (aTrackSelection.iTsSinkNodeCapConfigIF != NULL)
        {
            aCheckcodec = aTrackSelection.iTsSinkNodeCapConfigIF->verifyParametersSync(NULL, &kvp, 1);
        }
    }

    alloc.deallocate((OsclAny*)(kvp.key));
    if (aCheckcodec != PVMFSuccess)
    {
        //In case of other error code, this is operation error.
        if (aCheckcodec != PVMFErrNotSupported)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoVerifyTrackInfo() verifyParametersSync() on sink node failed"));
            return aCheckcodec;
        }
        return status;
    }

    //verify bitrate
    PvmiKvp iKVPBitRate;
    iKVPBitRate.key = NULL;

    OSCL_StackString<64> iKVPStringBitRate = _STRLIT_CHAR(PVMF_BITRATE_VALUE_KEY);
    iKVPBitRate.key = iKVPStringBitRate.get_str();
    iKVPBitRate.value.uint32_value = aTrack->getTrackBitRate();

    if (aTrackSelection.iTsSinkNodeCapConfigIF != NULL)
        aCheckcodec = aTrackSelection.iTsSinkNodeCapConfigIF->verifyParametersSync(NULL, &iKVPBitRate, 1);
    //In case of other error code, this is operation error.
    if (aCheckcodec != PVMFSuccess)
    {
        //In case of other error code, this is operation error.
        if (aCheckcodec != PVMFErrNotSupported)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoVerifyTrackInfo() verifyParametersSync() on sinknode bitrate failed"));
            return aCheckcodec;
        }
        return status;
    }

    //verify video framerate, if track is not video, sink will return ErrNotSupported.
    if (aTrack->getTrackFrameRate() > 0)
    {
        PvmiKvp iKVPFrameRate;
        iKVPFrameRate.key = NULL;

        OSCL_StackString<64> iKVPStringFrameRate = _STRLIT_CHAR(PVMF_FRAMERATE_VALUE_KEY);
        iKVPFrameRate.key = iKVPStringFrameRate.get_str();
        iKVPFrameRate.value.uint32_value = aTrack->getTrackFrameRate();

        if (aTrackSelection.iTsSinkNodeCapConfigIF != NULL)
            aCheckcodec = aTrackSelection.iTsSinkNodeCapConfigIF->verifyParametersSync(NULL, &iKVPFrameRate, 1);
        //In case of other error code, this is operation error.
        if (aCheckcodec != PVMFErrNotSupported && aCheckcodec != PVMFSuccess)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoVerifyTrackInfo() verifyParametersSync() on sink node framerate failed"));
            return aCheckcodec;
        }
    }

    return status;
}

PVMFStatus PVPlayerEngine::DoSinkNodeDecNodeReset(PVCommandId aCmdId, OsclAny* aCmdContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_INFO,
                    (0, "PVPlayerEngine::DoSinkNodeDecNodeReset() Tick=%d", OsclTickCount::TickCount()));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSinkNodeDecNodeReset() In"));

    iNumPendingNodeCmd = 0;
    PVMFCommandId cmdid = -1;
    int32 leavecode = 0;

    for (uint32 i = 0; i < iDatapathList.size(); ++i)
    {
        if (iDatapathList[i].iSinkNode != NULL)
        {
            // Call Reset() on the sink node
            PVPlayerEngineContext* context = AllocateEngineContext(&(iDatapathList[i]), iDatapathList[i].iSinkNode, NULL, aCmdId, aCmdContext, PVP_CMD_SinkNodeDecNodeReset);

            leavecode = IssueSinkNodeReset(&(iDatapathList[i]), (OsclAny*) context, cmdid);

            if (cmdid != -1 && leavecode == 0)
            {
                ++iNumPendingNodeCmd;
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSinkNodeDecNodeReset() Reset on sink node leaved, asserting"));
                FreeEngineContext(context);
                OSCL_ASSERT(false);
            }
        }

        if (iDatapathList[i].iDecNode != NULL)
        {
            // Call Reset() on the dec node
            PVPlayerEngineContext* context = AllocateEngineContext(&(iDatapathList[i]), iDatapathList[i].iDecNode, NULL, aCmdId, aCmdContext, PVP_CMD_SinkNodeDecNodeReset);

            leavecode = IssueDecNodeReset(iDatapathList[i].iDecNode, iDatapathList[i].iDecNodeSessionId, (OsclAny*) context, cmdid);

            if (cmdid != -1 && leavecode == 0)
            {
                ++iNumPendingNodeCmd;
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSinkNodeDecNodeReset() Reset on sink node leaved, asserting"));
                FreeEngineContext(context);
                OSCL_ASSERT(false);
            }
        }
    }

    // There can be some more decoders in TrackSelectionList on which engine needs to call Reset. call Reset on those decoders now.
    for (uint32 j = 0; j < iTrackSelectionList.size(); j++)
    {
        if (iTrackSelectionList[j].iTsDecNode != NULL)
        {
            // Call Reset() on the dec node
            PVPlayerEngineContext* context = AllocateEngineContext(NULL, iTrackSelectionList[j].iTsDecNode, NULL, aCmdId, aCmdContext, PVP_CMD_SinkNodeDecNodeReset);

            leavecode = IssueDecNodeReset(iTrackSelectionList[j].iTsDecNode, iTrackSelectionList[j].iTsDecNodeSessionId, (OsclAny*) context, cmdid);

            if (cmdid != -1 && leavecode == 0)
            {
                ++iNumPendingNodeCmd;
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSinkNodeDecNodeReset() Reset on sink node leaved, asserting"));
                FreeEngineContext(context);
                OSCL_ASSERT(false);
            }
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSinkNodeDecNodeReset() Out"));
    if (iNumPendingNodeCmd == 0)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSinkNodeDecNodeReset() No datapath could be prepared!"));
        return PVMFFailure;
    }
    else
    {
        return PVMFSuccess;
    }
}

PVMFStatus PVPlayerEngine::DoSinkDecCleanupSourcePrepare(PVCommandId aCmdId, OsclAny* aCmdContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVPlayerEngine::DoSinkDecCleanupSourcePrepare() Tick=%d", OsclTickCount::TickCount()));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSinkDecCleanupSourcePrepare() In"));

    uint32 i = 0;
    //Destroy created temporal decoder node and sink node
    for (i = 0; i < iTrackSelectionList.size(); ++i)
    {
        if (iTrackSelectionList[i].iTsDecNode != NULL)
        {
            if (iTrackSelectionList[i].iTsDecNodeCapConfigIF)
                iTrackSelectionList[i].iTsDecNodeCapConfigIF = NULL;
            PVMFStatus status = PVMFFailure;
            status = iTrackSelectionList[i].iTsDecNode->Disconnect(iTrackSelectionList[i].iTsDecNodeSessionId);
            if (status == PVMFFailure)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSinkDecCleanupSourcePrepare() Disconnect on dec node Failed"));
                OSCL_ASSERT(false);
            }
            status = iTrackSelectionList[i].iTsDecNode->ThreadLogoff();
            if (status == PVMFFailure)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSinkDecCleanupSourcePrepare() ThreadLogoff on dec node Failed"));
                OSCL_ASSERT(false);
            }
            PVPlayerEngineUuidNodeMapping* iter = iNodeUuids.begin();
            for (; iter != iNodeUuids.end(); ++iter)
                if (iter->iNode == iTrackSelectionList[i].iTsDecNode)
                    break;

            if (iter != iNodeUuids.end())
            {
                bool release_status = false;

                release_status = iPlayerNodeRegistry.ReleaseNode(iter->iUuid, iTrackSelectionList[i].iTsDecNode);
                if (release_status == false)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSinkDecCleanupSourcePrepare() Factory returned false while releasing the decnode"));
                    OSCL_ASSERT(false);
                    return PVMFFailure;
                }

                iNodeUuids.erase(iter);
                iTrackSelectionList[i].iTsDecNode = NULL;
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSinkDecCleanupSourcePrepare() decnode not found"));
                return PVMFFailure;
            }
        }
        if (iTrackSelectionList[i].iTsSinkNode != NULL)
        {
            for (uint32 j = 0; j < iDatapathList.size(); j++)
            {
                if (iDatapathList[j].iSinkNode == iTrackSelectionList[i].iTsSinkNode)
                {
                    PVMFStatus status = PVMFFailure;
                    status = iDatapathList[j].iSinkNode->Disconnect(iDatapathList[j].iSinkNodeSessionId);
                    if (status == PVMFFailure)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSinkDecCleanupSourcePrepare() Disconnect on sink node Failed"));
                        OSCL_ASSERT(false);
                    }
                    status = iDatapathList[j].iSinkNode->ThreadLogoff();
                    if (status == PVMFFailure)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSinkDecCleanupSourcePrepare() ThreadLogoff on sink node Failed"));
                        OSCL_ASSERT(false);
                    }
                    if (iDatapathList[j].iSinkNodeCapConfigIF)
                        iDatapathList[j].iSinkNodeCapConfigIF = NULL;
                    if (iDatapathList[j].iDataSink->GetDataSinkType() == PVP_DATASINKTYPE_FILENAME)
                    {
                        PVFileOutputNodeFactory::DeleteFileOutput(iDatapathList[j].iSinkNode);
                        iDatapathList[j].iSinkNode = NULL;
                    }
                    else if (iDatapathList[j].iDataSink->GetDataSinkType() == PVP_DATASINKTYPE_SINKNODE)
                    {
                        iDatapathList[j].iSinkNode = NULL;
                    }
                    else
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSinkDecCleanupSourcePrepare() Unsupported player data sink type"));
                        return PVMFFailure;
                    }
                }
            }
        }
    }

    // Also go through the datapathList and if for any datapath TrackInfo is not created, delete that datapath
    for (i = 0; i < iDatapathList.size(); i++)
    {
        if (iDatapathList[i].iTrackInfo == NULL)
        {
            // destroy the sinks first.
            if (iDatapathList[i].iSinkNode != NULL)
            {
                PVMFStatus status = PVMFFailure;
                status = iDatapathList[i].iSinkNode->Disconnect(iDatapathList[i].iSinkNodeSessionId);
                if (status == PVMFFailure)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSinkDecCleanupSourcePrepare() Disconnect on sink node Failed"));
                    OSCL_ASSERT(false);
                }
                status = iDatapathList[i].iSinkNode->ThreadLogoff();
                if (status == PVMFFailure)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSinkDecCleanupSourcePrepare() ThreadLogoff on sink node Failed"));
                    OSCL_ASSERT(false);
                }
                if (iDatapathList[i].iSinkNodeCapConfigIF)
                    iDatapathList[i].iSinkNodeCapConfigIF = NULL;
                if (iDatapathList[i].iDataSink->GetDataSinkType() == PVP_DATASINKTYPE_FILENAME)
                {
                    PVFileOutputNodeFactory::DeleteFileOutput(iDatapathList[i].iSinkNode);
                    iDatapathList[i].iSinkNode = NULL;
                }
                else if (iDatapathList[i].iDataSink->GetDataSinkType() == PVP_DATASINKTYPE_SINKNODE)
                {
                    iDatapathList[i].iSinkNode = NULL;
                }
                else
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSinkDecCleanupSourcePrepare() Unsupported player data sink type"));
                    return PVMFFailure;
                }
            }

            // next delete the decoder nodes
            if (iDatapathList[i].iDecNode != NULL)
            {
                if (iDatapathList[i].iDecNodeCapConfigIF)
                    iDatapathList[i].iDecNodeCapConfigIF = NULL;
                PVMFStatus status = PVMFFailure;
                status = iDatapathList[i].iDecNode->Disconnect(iDatapathList[i].iDecNodeSessionId);
                if (status == PVMFFailure)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSinkDecCleanupSourcePrepare() Disconnect on dec node Failed"));
                    OSCL_ASSERT(false);
                }
                status = iDatapathList[i].iDecNode->ThreadLogoff();
                if (status == PVMFFailure)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSinkDecCleanupSourcePrepare() ThreadLogoff on dec node Failed"));
                    OSCL_ASSERT(false);
                }
                PVPlayerEngineUuidNodeMapping* iter = iNodeUuids.begin();
                for (; iter != iNodeUuids.end(); ++iter)
                    if (iter->iNode == iDatapathList[i].iDecNode)
                        break;

                if (iter != iNodeUuids.end())
                {
                    bool release_status = false;

                    release_status = iPlayerNodeRegistry.ReleaseNode(iter->iUuid, iDatapathList[i].iDecNode);
                    if (release_status == false)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSinkDecCleanupSourcePrepare() Factory returned false while releasing the decnode"));
                        OSCL_ASSERT(false);
                        return PVMFFailure;
                    }

                    iNodeUuids.erase(iter);
                    iDatapathList[i].iDecNode = NULL;
                }
                else
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSinkDecCleanupSourcePrepare() decnode not found"));
                    return PVMFFailure;
                }
            }
        }
    }

    // Reset the Presentation Info list
    iSourcePresInfoList.Reset();

    // Clear the Track selection List
    iTrackSelectionList.clear();

    PVMFStatus cmdstatus = PVMFFailure;

    // Notify the TargetNPT to the source node before calling Prepare.
    if (iSourceNodePBCtrlIF)
    {
        cmdstatus = iSourceNodePBCtrlIF->NotifyTargetPositionSync(iTargetNPT);
    }
    if (cmdstatus == PVMFSuccess || cmdstatus == PVMFErrNotSupported)
    {
        // Prepare the source node
        cmdstatus = DoSourceNodePrepare(aCmdId, aCmdContext);
    }
    if (cmdstatus != PVMFSuccess)
    {
        bool ehPending = CheckForPendingErrorHandlingCmd();
        if (ehPending)
        {
            // there should be no error handling queued.
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSinkDecCleanupSourcePrepare() Already EH pending, should never happen"));
            return PVMFPending;
        }
        else
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVPlayerEngine::DoSinkDecCleanupSourcePrepare() DoSourceNodePrepare failed, Add EH command"));
            iCommandCompleteErrMsgInErrorHandling = NULL;
            iCommandCompleteStatusInErrorHandling = cmdstatus;
            AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_PREPARE, NULL, NULL, NULL, false);
        }
        return PVMFFailure;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSinkDecCleanupSourcePrepare() Out"));
    return PVMFSuccess;
}

PVMFStatus PVPlayerEngine::DoSourceNodePrepare(PVCommandId aCmdId, OsclAny* aCmdContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_INFO,
                    (0, "PVPlayerEngine::DoSourceNodePrepare() Tick=%d", OsclTickCount::TickCount()));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSourceNodePrepare() In"));

    if (iSourceNode == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSourceNodePrepare() Source node not available."));
        return PVMFFailure;
    }

    // If source node is already in Prepared state then don't call Prepare()
    if (iSourceNode->GetState() == EPVMFNodePrepared)
    {
        // Datapaths are already set during intelligent track selection, just query for optional interfaces.
        iNumPendingDatapathCmd = 0;
        for (uint32 i = 0; i < iDatapathList.size(); ++i)
        {
            if (iDatapathList[i].iTrackInfo != NULL)
            {
                PVMFStatus cmdstatus = DoSinkNodeQueryInterfaceOptional(iDatapathList[i], aCmdId, aCmdContext);
                if (cmdstatus == PVMFSuccess)
                {
                    ++iNumPendingDatapathCmd;
                }
            }
        }

        if (iNumPendingDatapathCmd == 0)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSourceNodePrepare() No datapath could be prepared!"));
            return PVMFFailure;
        }
        else
        {
            return PVMFSuccess;
        }
    }

    // Call Prepare() on the source node
    PVPlayerEngineContext* context = AllocateEngineContext(NULL, iSourceNode, NULL, aCmdId, aCmdContext, PVP_CMD_SourceNodePrepare);

    PVMFCommandId cmdid = -1;
    int32 leavecode = 0;
    OSCL_TRY(leavecode, cmdid = iSourceNode->Prepare(iSourceNodeSessionId, (OsclAny*)context));
    OSCL_FIRST_CATCH_ANY(leavecode,
                         FreeEngineContext(context);
                         PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSourceNodePrepare() Prepare on iSourceNode did a leave!"));
                         return PVMFFailure);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSourceNodePrepare() Out"));

    return PVMFSuccess;
}

PVMFStatus PVPlayerEngine::DoSinkNodeQueryInterfaceOptional(PVPlayerEngineDatapath &aDatapath, PVCommandId aCmdId, OsclAny* aCmdContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVPlayerEngine::DoSinkNodeQueryInterfaceOptional() Tick=%d", OsclTickCount::TickCount()));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSinkNodeQueryInterfaceOptional() In %s", aDatapath.iTrackInfo->getTrackMimeType().get_cstr()));

    int32 leavecode = 0;

    // Request optional extension interface from the sink node
    PVPlayerEngineContext* context = NULL;
    PVMFCommandId cmdid = -1;
    aDatapath.iNumPendingCmd = 0;

    // Request the sync control interface for the sink node
    context = AllocateEngineContext(&aDatapath, aDatapath.iSinkNode, NULL, aCmdId, aCmdContext, PVP_CMD_SinkNodeQuerySyncCtrlIF);
    cmdid = -1;
    leavecode = 0;
    aDatapath.iSinkNodePVInterfaceSyncCtrl = NULL;
    OSCL_TRY(leavecode, cmdid = aDatapath.iSinkNode->QueryInterface(aDatapath.iSinkNodeSessionId, PvmfNodesSyncControlUuid, aDatapath.iSinkNodePVInterfaceSyncCtrl, (OsclAny*)context));
    if (leavecode)
    {
        aDatapath.iSinkNodePVInterfaceSyncCtrl = NULL;
        FreeEngineContext(context);
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSinkNodeQueryInterfaceOptional() QueryInterface on sink node for sync control IF did a leave!"));
    }
    else
    {
        ++aDatapath.iNumPendingCmd;
    }

    // Query for Metadata IF
    context = AllocateEngineContext(&aDatapath, aDatapath.iSinkNode, NULL, aCmdId, aCmdContext, PVP_CMD_SinkNodeQueryMetadataIF);
    PVUuid metadatauuid = KPVMFMetadataExtensionUuid;
    cmdid = -1;
    leavecode = 0;
    aDatapath.iSinkNodePVInterfaceMetadataExt = NULL;
    OSCL_TRY(leavecode, cmdid = aDatapath.iSinkNode->QueryInterface(aDatapath.iSinkNodeSessionId, metadatauuid, aDatapath.iSinkNodePVInterfaceMetadataExt, (OsclAny*)context));
    if (leavecode)
    {
        aDatapath.iSinkNodePVInterfaceMetadataExt = NULL;
        FreeEngineContext(context);
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSinkNodeQueryInterfaceOptional() QueryInterface on sink node for metadata IF did a leave!"));
    }
    else
    {
        ++aDatapath.iNumPendingCmd;
    }

    if (aDatapath.iNumPendingCmd > 0)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSinkNodeQueryInterfaceOptional() Out"));
        return PVMFSuccess;
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSinkNodeQueryInterfaceOptional() Out No pending QueryInterface() on sink node"));
        return PVMFErrNotSupported;
    }
}

PVMFStatus PVPlayerEngine::DoDecNodeQueryInterfaceOptional(PVPlayerEngineDatapath &aDatapath, PVCommandId aCmdId, OsclAny* aCmdContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVPlayerEngine::DoDecNodeQueryInterfaceOptional() Tick=%d", OsclTickCount::TickCount()));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoDecNodeQueryInterfaceOptional() In %s", aDatapath.iTrackInfo->getTrackMimeType().get_cstr()));

    // Check if the dec node is present
    if (aDatapath.iDecNode == NULL)
    {
        return PVMFErrNotSupported;
    }

    PVPlayerEngineContext* context = NULL;
    PVMFCommandId cmdid = -1;
    int32 leavecode = 0;

    aDatapath.iNumPendingCmd = 0;

    // Query for Metadata IF
    context = AllocateEngineContext(&aDatapath, aDatapath.iDecNode, NULL, aCmdId, aCmdContext, PVP_CMD_DecNodeQueryMetadataIF);
    PVUuid metadatauuid = KPVMFMetadataExtensionUuid;
    cmdid = -1;
    leavecode = 0;
    aDatapath.iDecNodePVInterfaceMetadataExt = NULL;
    OSCL_TRY(leavecode, cmdid = aDatapath.iDecNode->QueryInterface(aDatapath.iDecNodeSessionId, metadatauuid, aDatapath.iDecNodePVInterfaceMetadataExt, (OsclAny*)context));
    if (leavecode)
    {
        aDatapath.iDecNodePVInterfaceMetadataExt = NULL;
        FreeEngineContext(context);
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoDecNodeQueryInterfaceOptional() QueryInterface on dec node for metadata IF did a leave!"));
    }
    else
    {
        ++aDatapath.iNumPendingCmd;
    }

    if (aDatapath.iNumPendingCmd > 0)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoDecNodeQueryInterfaceOptional() Out"));
        return PVMFSuccess;
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoDecNodeQueryInterfaceOptional() Out No pending QueryInterface() on dec node"));
        return PVMFErrNotSupported;
    }
}


PVMFStatus PVPlayerEngine::DoDatapathPrepare(PVPlayerEngineDatapath &aDatapath, PVCommandId aCmdId, OsclAny* aCmdContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_INFO,
                    (0, "PVPlayerEngine::DoDatapathPrepare() for %s Tick=%d",
                     aDatapath.iTrackInfo->getTrackMimeType().get_cstr(), OsclTickCount::TickCount()));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoDatapathPrepare() In %s", aDatapath.iTrackInfo->getTrackMimeType().get_cstr()));
    int32 leavecode = 0;

    // Create the datapath utility object
    if (aDatapath.iDatapath == NULL)
    {
        leavecode = 0;
        OSCL_TRY(leavecode, aDatapath.iDatapath = OSCL_NEW(PVPlayerDatapath, ()));
        OSCL_FIRST_CATCH_ANY(leavecode,
                             PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoDatapathPrepare() Could not create datapath object"));
                             return PVMFErrNoMemory);
    }

    // Configure the datapath utility based on datapath configuration
    aDatapath.iDatapath->SetObserver(*this, *this, *this);
    aDatapath.iDatapath->SetSourceNode(iSourceNode);
    aDatapath.iDatapath->SetSinkNode(aDatapath.iSinkNode);

    if (aDatapath.iDecNode)
    {
        aDatapath.iDatapath->SetDecNode(aDatapath.iDecNode);
        aDatapath.iDatapath->SetSourceDecTrackInfo(*(aDatapath.iTrackInfo));
        aDatapath.iDatapath->SetDecSinkFormatType(aDatapath.iSinkFormat);
    }
    else
    {
        aDatapath.iDatapath->SetSourceSinkTrackInfo(*(aDatapath.iTrackInfo));
    }

    // Prepare the datapath
    PVPlayerEngineContext* context = AllocateEngineContext(&aDatapath, NULL, aDatapath.iDatapath, aCmdId, aCmdContext, PVP_CMD_DPPrepare);

    PVMFStatus retval = aDatapath.iDatapath->Prepare((OsclAny*)context);
    if (retval != PVMFSuccess)
    {
        FreeEngineContext(context);
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoDatapathPrepare() Out"));
    return retval;
}


PVMFStatus PVPlayerEngine::DoSourceNodeQueryDataSourcePosition(PVCommandId aCmdId, OsclAny* aCmdContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSourceNodeQueryDataSourcePosition() In"));

    // Check if the source node has position control IF
    if (iSourceNodePBCtrlIF == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSourceNodeQueryDataSourcePosition() Playback control IF on source node not available"));
        return PVMFErrNotSupported;
    }

    uint32 timems = 0;
    if (iCurrentBeginPosition.iIndeterminate == false)
    {
        // Convert beginning position to milliseconds
        PVMFStatus retval = ConvertToMillisec(iCurrentBeginPosition, timems);
        if (retval != PVMFSuccess)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSourceNodeQueryDataSourcePosition() Converting to millisec failed"));
            return retval;
        }
    }

    PVMFCommandId cmdid = -1;

    if (iSeekToSyncPoint && iSyncPointSeekWindow > 0)
    {
        PVPlayerEngineContext* context = AllocateEngineContext(NULL, iSourceNode, NULL, aCmdId, aCmdContext, PVP_CMD_SourceNodeQueryDataSourcePosition);

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSourceNodeQueryDataSourcePosition() Calling QueryDataSourcePosition() Starting pos %d ms, SeekToSyncPt %d", iTargetNPT, iSeekToSyncPoint));
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iReposLogger, PVLOGMSG_INFO, (0, "PVPlayerEngine::DoSourceNodeQueryDataSourcePosition() Calling QueryDataSourcePosition() Starting pos %d ms, SeekToSyncPt %d", iTargetNPT, iSeekToSyncPoint));
        // As in case of MP4 file we need to call overload function of QueryDataSourcePosition which retruns
        // I frame before and after instead of actaul NPT, format type will be checked here to first find if
        // format-type is one of the MP4 varient

        PVMFNodeCapability nodeCapability;
        iSourceNode->GetCapability(nodeCapability);
        PVMFFormatType * formatType = nodeCapability.iInputFormatCapability.begin();
        bool mpeg4FormatType = false;
        if (formatType != NULL)
        {
            if ((pv_mime_strcmp((char*)formatType->getMIMEStrPtr(), PVMF_MIME_MPEG4FF)) == 0)
            {
                mpeg4FormatType = true;
            }
            else
            {
                mpeg4FormatType = false;
            }
        }
        int32 leavecode = 0;
        if (mpeg4FormatType)
        {
            OSCL_TRY(leavecode, cmdid = iSourceNodePBCtrlIF->QueryDataSourcePosition(iSourceNodeSessionId, iTargetNPT,
                                        iSeekPointBeforeTargetNPT, iSeekPointAfterTargetNPT, (OsclAny*) context, iSeekToSyncPoint));
        }
        else
        {
            OSCL_TRY(leavecode, cmdid = iSourceNodePBCtrlIF->QueryDataSourcePosition(iSourceNodeSessionId, iTargetNPT, iActualNPT,
                                        iSeekToSyncPoint, (OsclAny*)context));
        }

        if (leavecode != 0)
        {
            FreeEngineContext(context);
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSourceNodeQueryDataSourcePosition() QueryDataSourcePosition on iSourceNodePBCtrlIF did a leave!"));
            if (leavecode == PVMFErrNotSupported || leavecode == PVMFErrArgument)
            {
                // Since position query is not supported, assume the repositioning will
                // go to the requested position
                // Do the source positioning
                return DoSourceNodeSetDataSourcePosition(aCmdId, aCmdContext);
            }
            else
            {
                return PVMFFailure;
            }
        }
    }
    else
    {
        // Go straight to repositioning the data source
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iReposLogger, PVLOGMSG_INFO, (0, "PVPlayerEngine::DoSourceNodeQueryDataSourcePosition: Straight call SetDataSourcePosition Starting pos %d ms, SeekToSyncPt %d", iTargetNPT, iSeekToSyncPoint));
        return DoSourceNodeSetDataSourcePosition(aCmdId, aCmdContext);
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSourceNodeQueryDataSourcePosition() Out"));

    return PVMFSuccess;
}


PVMFStatus PVPlayerEngine::DoSourceNodeSetDataSourcePosition(PVCommandId aCmdId, OsclAny* aCmdContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSourceNodeSetDataSourcePosition() In"));

    // Check if the source node has position control IF
    if (iSourceNodePBCtrlIF == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSourceNodeSetDataSourcePosition() Playback control IF on source node not available"));
        // Since repositioning IF is not supported by this source node, assume the playback
        // will start from time 0
        iActualNPT = 0;
        iActualMediaDataTS = 0;
        iSkipMediaDataTS = 0;
        // Then continue to handle like success case
        iStartNPT = 0;
        iStartMediaDataTS = 0;
        // Save the actual starting position for GetPlaybackRange() query
        iCurrentBeginPosition.iPosValue.millisec_value = iActualNPT;
        iCurrentBeginPosition.iPosUnit = PVPPBPOSUNIT_MILLISEC;
        iTargetNPT = iActualNPT;

        // Repositioning so reset the EOS received flag for each active datapath
        for (uint32 i = 0; i < iDatapathList.size(); ++i)
        {
            if (iDatapathList[i].iDatapath)
            {
                iDatapathList[i].iEndOfDataReceived = false;
            }
        }

        // Start the source node
        return DoSourceNodeStart(aCmdId, aCmdContext);
    }

    // Set the position of the source node
    PVPlayerEngineContext* context = AllocateEngineContext(NULL, iSourceNode, NULL, aCmdId, aCmdContext, PVP_CMD_SourceNodeSetDataSourcePosition);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSourceNodeSetDataSourcePosition() Calling SetDataSourcePosition() TargetNPT %d ms, SeekToSyncPoint %d", iTargetNPT, iSeekToSyncPoint));

    int32 leavecode = 0;
    PVMFCommandId cmdid = -1;

    OSCL_TRY(leavecode, cmdid = iSourceNodePBCtrlIF->SetDataSourcePosition(iSourceNodeSessionId, iTargetNPT, iActualNPT, iActualMediaDataTS, iSeekToSyncPoint, iStreamID, (OsclAny*)context));
    OSCL_FIRST_CATCH_ANY(leavecode,
                         FreeEngineContext(context);
                         PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSourceNodeSetDataSourcePosition() SetDataSourcePosition on iSourceNodePBCtrlIF did a leave!"));
                         if (leavecode == PVMFErrNotSupported || leavecode == PVMFErrArgument)
{
    // Since this repositioning was not supported, assume the playback
    // will start from same location as before reposition request.
    PVPPlaybackPosition curpos;
    curpos.iPosUnit = PVPPBPOSUNIT_MILLISEC;
    GetPlaybackClockPosition(curpos);
        uint32 clockcurpos = 0;
        bool tmpbool = false;
        iPlaybackClock.GetCurrentTime32(clockcurpos, tmpbool, PVMF_MEDIA_CLOCK_MSEC);

        // since repositioning is not supported continue playing from current position.
        iWatchDogTimerInterval = 0;
        iActualNPT = curpos.iPosValue.millisec_value;
        iActualMediaDataTS = clockcurpos;
        iSkipMediaDataTS = clockcurpos;

        iStartNPT = iActualNPT;
        iStartMediaDataTS = iSkipMediaDataTS;

        // also decrement the stream id as no skip will be called on MIO node.
        --iStreamID;

        // Save the actual starting position for GetPlaybackRange() query
        iCurrentBeginPosition.iPosValue.millisec_value = iActualNPT;
        iCurrentBeginPosition.iPosUnit = PVPPBPOSUNIT_MILLISEC;
        iTargetNPT = iActualNPT;

        // Repositioning so reset the EOS received flag for each active datapath
        for (uint32 i = 0; i < iDatapathList.size(); ++i)
        {
            if (iDatapathList[i].iDatapath)
            {
                iDatapathList[i].iEndOfDataReceived = false;
            }
        }

        // Start the source node
        return DoSourceNodeStart(aCmdId, aCmdContext);
    }
    else
    {
        return PVMFFailure;
    }
                        );

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSourceNodeSetDataSourcePosition() Out"));

    return PVMFSuccess;
}

PVMFStatus PVPlayerEngine::DoSourceNodeSetDataSourceDirection(PVCommandId aCmdId, OsclAny* aCmdContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSourceNodeSetDataSourceDirection() In"));

    if (iChangePlaybackDirectionWhenResuming)
    {
        //Setting direction during engine Resume, due to a SetPlaybackRate that
        //occurred during engine Paused state.

        // Check if the source node has position control IF
        if (iSourceNodeDirCtrlIF == NULL)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSourceNodeSetDataSourceDirection() Direction control IF on source node not available"));

            // Since repositioning IF is not supported by this source node, assume the playback
            // will start from time 0
            iActualNPT = 0;
            iActualMediaDataTS = 0;
            iSkipMediaDataTS = 0;
            // Then continue to handle like success case
            iStartNPT = 0;
            iStartMediaDataTS = 0;
            // Save the actual starting position for GetPlaybackRange() query
            iCurrentBeginPosition.iPosValue.millisec_value = iActualNPT;
            iCurrentBeginPosition.iPosUnit = PVPPBPOSUNIT_MILLISEC;
            iTargetNPT = iActualNPT;

            // Repositioning so reset the EOS flag for each active datapath
            for (uint32 i = 0; i < iDatapathList.size(); ++i)
            {
                if (iDatapathList[i].iDatapath)
                {
                    iDatapathList[i].iEndOfDataReceived = false;
                }
            }

            return PVMFErrNotSupported;
        }

        // Set the position of the source node
        PVPlayerEngineContext* context = AllocateEngineContext(NULL, iSourceNode, NULL, aCmdId, aCmdContext, PVP_CMD_SourceNodeSetDataSourceDirection);

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSourceNodeSetDataSourceDirection() Calling SetDataSourcePosition() "));

        int32 leavecode = 0;
        PVMFCommandId cmdid = -1;
        OSCL_TRY(leavecode, cmdid = iSourceNodeDirCtrlIF->SetDataSourceDirection(iSourceNodeSessionId
                                    , (iPlaybackDirection_New < 0) ? PVMF_DATA_SOURCE_DIRECTION_REVERSE : PVMF_DATA_SOURCE_DIRECTION_FORWARD
                                    , iActualNPT
                                    , iActualMediaDataTS
                                    , iOutsideTimebase
                                    , (OsclAny*)context));
        OSCL_FIRST_CATCH_ANY(leavecode,
                             FreeEngineContext(context);
                             PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSourceNodeSetDataSourceDirection() SetDataSourceDirection on iSourceNodeDirCtrlIF did a leave!"));
                             if (leavecode == PVMFErrNotSupported || leavecode == PVMFErrArgument)
    {
        // Since this repositioning was not supported, assume the playback
        // will start from same location as before repos request.
        PVPPlaybackPosition curpos;
        curpos.iPosUnit = PVPPBPOSUNIT_MILLISEC;
        GetPlaybackClockPosition(curpos);
            uint32 clockcurpos = 0;
            bool tmpbool = false;
            iPlaybackClock.GetCurrentTime32(clockcurpos, tmpbool, PVMF_MEDIA_CLOCK_MSEC);

            // since repositioning is not supported continue playing from current position.
            iWatchDogTimerInterval = 0;
            iActualNPT = curpos.iPosValue.millisec_value;
            iActualMediaDataTS = clockcurpos;
            iSkipMediaDataTS = clockcurpos;

            iStartNPT = iActualNPT;
            iStartMediaDataTS = iSkipMediaDataTS;

            // Save the actual starting position for GetPlaybackRange() query
            iCurrentBeginPosition.iPosValue.millisec_value = iActualNPT;
            iCurrentBeginPosition.iPosUnit = PVPPBPOSUNIT_MILLISEC;
            iTargetNPT = iActualNPT;

            // Repositioning so reset the EOS flag for each active datapath
            for (uint32 i = 0; i < iDatapathList.size(); ++i)
            {
                if (iDatapathList[i].iDatapath)
                {
                    iDatapathList[i].iEndOfDataReceived = false;
                }
            }

            return PVMFErrNotSupported;
        }
        else
        {
            return PVMFFailure;
        }
                            );

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSourceNodeSetDataSourceDirection() Out"));

        return PVMFSuccess;
    }
    else
    {
        //changing direction during SetPlaybackRate command

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSourceNodeSetDataSourceDirection() In"));

        // Check if the source node has direction control IF
        if (iSourceNodeDirCtrlIF == NULL)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSourceNodeSetDataSourceDirection() No source direction control IF"));
            return PVMFFailure;
        }

        // Pause the playback clock
        bool clockpausedhere = iPlaybackClock.Pause();

        // Set the new direction on the source node
        PVPlayerEngineContext* context = AllocateEngineContext(NULL, iSourceNode, NULL, aCmdId, aCmdContext, PVP_CMD_SourceNodeSetDataSourceDirection);

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSourceNodeSetDataSourceDirection() Calling SetDataSourceDirection() on source node."));

        int32 leavecode = 0;
        PVMFCommandId cmdid = -1;
        OSCL_TRY(leavecode, cmdid = iSourceNodeDirCtrlIF->SetDataSourceDirection(iSourceNodeSessionId
                                    , (iPlaybackDirection_New < 0) ? PVMF_DATA_SOURCE_DIRECTION_REVERSE : PVMF_DATA_SOURCE_DIRECTION_FORWARD
                                    , iActualNPT
                                    , iActualMediaDataTS
                                    , iOutsideTimebase_New
                                    , (OsclAny*)context));
        OSCL_FIRST_CATCH_ANY(leavecode,
                             FreeEngineContext(context);
                             PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSourceNodeSetDataSourceDirection() SetDataSourceDirection on iSourceNodeDirCtrlIF did a leave!"));
                             if (clockpausedhere)
    {
        // Resume the clock if paused in this function
        StartPlaybackClock();
        }

        if (leavecode == PVMFErrNotSupported || leavecode == PVMFErrArgument)
    {
        return leavecode;
    }
    else
    {
        return PVMFFailure;
    }
                        );

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSourceNodeSetDataSourceDirection() Out"));

        return PVMFSuccess;
        //wait on node command complete and a call to HandleSourceNodeSetDataSourceDirection
    }
}

PVMFStatus PVPlayerEngine::DoSourceNodeStart(PVCommandId aCmdId, OsclAny* aCmdContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_INFO,
                    (0, "PVPlayerEngine::DoSourceNodeStart() Tick=%d", OsclTickCount::TickCount()));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSourceNodeStart() In"));

    if (iSourceNode == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSourceNodeStart() Source node not available."));
        return PVMFFailure;
    }

    // Start the source node
    PVPlayerEngineContext* context = AllocateEngineContext(NULL, iSourceNode, NULL, aCmdId, aCmdContext, PVP_CMD_SourceNodeStart);

    PVMFCommandId cmdid = -1;
    int32 leavecode = 0;
    OSCL_TRY(leavecode, cmdid = iSourceNode->Start(iSourceNodeSessionId, (OsclAny*)context));
    OSCL_FIRST_CATCH_ANY(leavecode,
                         FreeEngineContext(context);
                         PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSourceNodeStart() Start on iSourceNode did a leave!"));
                         return PVMFFailure);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSourceNodeStart() Out"));

    return PVMFSuccess;
}


PVMFStatus PVPlayerEngine::DoDatapathStart(PVPlayerEngineDatapath &aDatapath, PVCommandId aCmdId, OsclAny* aCmdContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_INFO,
                    (0, "PVPlayerEngine::DoDatapathStart() for %s Tick=%d",
                     aDatapath.iTrackInfo->getTrackMimeType().get_cstr(), OsclTickCount::TickCount()));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoDatapathStart() In %s", aDatapath.iTrackInfo->getTrackMimeType().get_cstr()));

    // Start the datapath
    PVPlayerEngineContext* context = AllocateEngineContext(&aDatapath, NULL, aDatapath.iDatapath, aCmdId, aCmdContext, PVP_CMD_DPStart);

    PVMFStatus retval = aDatapath.iDatapath->Start((OsclAny*)context);
    if (retval != PVMFSuccess)
    {
        FreeEngineContext(context);
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoDatapathStart() Out"));
    return retval;
}


PVMFStatus PVPlayerEngine::DoSinkNodeSkipMediaData(PVCommandId aCmdId, OsclAny* aCmdContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_INFO,
                    (0, "PVPlayerEngine::DoSinkNodeSkipMediaData() Tick=%d", OsclTickCount::TickCount()));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSinkNodeSkipMediaData() In"));

    // Tell the sink nodes to skip the unneeded media data
    iNumPendingNodeCmd = 0;
    int32 leavecode = 0;

    // Call SkipMediaData() for each active datapath with sink nodes that have the sync control IF
    for (uint32 i = 0; i < iDatapathList.size(); ++i)
    {
        if (iDatapathList[i].iDatapath &&
                iDatapathList[i].iEndOfDataReceived == false &&
                iDatapathList[i].iSinkNodeSyncCtrlIF)
        {
            PVPlayerEngineContext* context = AllocateEngineContext(&(iDatapathList[i]), iDatapathList[i].iSinkNode, NULL, aCmdId, aCmdContext, PVP_CMD_SinkNodeSkipMediaData);

            leavecode = IssueSinkSkipMediaData(&(iDatapathList[i]), false, (OsclAny*) context);
            if (leavecode == 0)
            {
                ++iNumPendingNodeCmd;
                ++iNumPendingSkipCompleteEvent;
                ++iNumPVMFInfoStartOfDataPending;
            }
            else
            {
                FreeEngineContext(context);
            }
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSinkNodeSkipMediaData() Out"));
    if (iNumPendingNodeCmd > 0)
    {
        return PVMFSuccess;
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSinkNodeSkipMediaData() Skip on sink nodes failed"));
        return PVMFFailure;
    }
}


PVMFStatus PVPlayerEngine::DoStart(PVPlayerEngineCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_NOTICE,
                    (0, "PVPlayerEngine::DoStart() Tick=%d", OsclTickCount::TickCount()));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoStart() In, State=%d", iState));

    if (GetPVPlayerState() == PVP_STATE_STARTED)
    {
        if (iState == PVP_ENGINE_STATE_AUTO_PAUSED)
        {
            // Engine in AUTO-PAUSED state since it recieved an Underflow event
            // during Prepare. Set the engine state to STARTED and return success
            // Wait for DataReady event for the playback to start.
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVPlayerEngine::DoStart() Engine in auto-paused state, set it to started"));
            SetEngineState(PVP_ENGINE_STATE_STARTED);
        }
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoStart() Engine already in Started State"));
        EngineCommandCompleted(aCmd.GetCmdId(), aCmd.GetContext(), PVMFSuccess);
        return PVMFSuccess;
    }

    if (iPlaybackPausedDueToEndOfClip)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoStart() Playback already paused due to End of clip"));
        EngineCommandCompleted(aCmd.GetCmdId(), aCmd.GetContext(), PVMFSuccess);
        return PVMFSuccess;
    }
    if (GetPVPlayerState() != PVP_STATE_PREPARED)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoStart() Wrong engine state"));
        return PVMFErrInvalidState;
    }

    if (iNumPVMFInfoStartOfDataPending == 0)
    {
        // start the clock only if Skip is complete and InfoStartOfData has been received
        // Enable the end time check if specified
        UpdateCurrentEndPosition(iCurrentEndPosition);
        StartPlaybackClock();
    }
    else
    {
        // Sink nodes have not reported InfoStartOfData yet.
        // Check if WatchDogTimer has already been set or not,
        // if Timer has not been set or has expired then set it to a default value timer.
        if (!(iWatchDogTimer->IsBusy()))
        {
            // set a dafault timer
            iWatchDogTimerInterval = 0;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iReposLogger, PVLOGMSG_INFO,
                            (0, "PVPlayerEngine::DoStart() Setting WatchDogTimer to a dafult value of 1 second"));
            iWatchDogTimer->setTimerDuration(iWatchDogTimerInterval); // this will set a timer to a default
            iWatchDogTimer->Start();                                  // value of 1 sec. since iWatchDogTimerInterval is zero.
        }
    }

    SetEngineState(PVP_ENGINE_STATE_STARTED);
    EngineCommandCompleted(aCmd.GetCmdId(), aCmd.GetContext(), PVMFSuccess);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoStart() Out"));
    return PVMFSuccess;
}


PVMFStatus PVPlayerEngine::DoPause(PVPlayerEngineCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_NOTICE,
                    (0, "PVPlayerEngine::DoPause() Tick=%d", OsclTickCount::TickCount()));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoPause() In"));

    // Check engine state
    switch (GetPVPlayerState())
    {
        case PVP_STATE_PAUSED :
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoPause() Engine already in Paused State"));
            EngineCommandCompleted(aCmd.GetCmdId(), aCmd.GetContext(), PVMFSuccess);
            return PVMFSuccess;

        case PVP_STATE_STARTED :
            break;

        case PVP_STATE_PREPARED :
            if (aCmd.GetCmdType() == PVP_ENGINE_COMMAND_PAUSE_DUE_TO_ENDOFCLIP)
            {
                //It is possible in repositioning to end use-case that
                //engine receives StartofData and EndofData before Prepare
                //completes. So we should not fail Pause, just process it.
                break;
            }

        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoPause() Wrong engine state"));
            return PVMFErrInvalidState;
    }

    // Send position update to app.
    SendPositionStatusUpdate();

    // Stop the end time check timer
    iPollingCheckTimer->Cancel(PVPLAYERENGINE_TIMERID_ENDTIMECHECK);

    // Stop the watchdog timer if active. We will Start the timer again in resume.
    // this should only be done when engine is waiting for StartofData info event
    // after reposition.
    if (iNumPVMFInfoStartOfDataPending > 0)
    {
        if (iWatchDogTimer->IsBusy())
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iReposLogger, PVLOGMSG_INFO,
                            (0, "PVPlayerEngine::DoPause - Pause after setplayback, Cancelling Watchdog timer, iNumPVMFInfoStartOfDataPending=%d", iNumPVMFInfoStartOfDataPending));
            iWatchDogTimer->Cancel();
        }
    }

    // Pause the clock and notify sinks if not auto-paused
    uint32 i;
    if (iState != PVP_ENGINE_STATE_AUTO_PAUSED)
    {
        // Pause the playback clock
        iPlaybackClock.Pause();
        // Notify data sinks that clock has paused
        for (i = 0; i < iDatapathList.size(); ++i)
        {
            if (iDatapathList[i].iDatapath && iDatapathList[i].iSinkNodeSyncCtrlIF)
            {
                iDatapathList[i].iSinkNodeSyncCtrlIF->ClockStopped();
            }
        }
    }

    PVMFStatus retval = PVMFErrNotSupported;

    // Issue pause to all active datapaths
    iNumPendingDatapathCmd = 0;

    for (i = 0; i < iDatapathList.size(); ++i)
    {
        if (iDatapathList[i].iDatapath)
        {
            if (iState == PVP_ENGINE_STATE_AUTO_PAUSED)
            {
                // Since sinks are already paused in auto-pause state, skip pausing the sink in the datapath
                retval = DoDatapathPause(iDatapathList[i], aCmd.GetCmdId(), aCmd.GetContext(), true);
            }
            else
            {
                // Pause all nodes in the datapath
                retval = DoDatapathPause(iDatapathList[i], aCmd.GetCmdId(), aCmd.GetContext(), false);
            }

            if (retval == PVMFSuccess)
            {
                ++iNumPendingDatapathCmd;
            }
            else
            {
                bool ehPending = CheckForPendingErrorHandlingCmd();
                if (ehPending)
                {
                    // there should be no error handling queued.
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoPause() Already EH pending, should never happen"));
                    return PVMFPending;
                }
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoPause() DoDatapathPause Failed, Add EH command"));
                iCommandCompleteStatusInErrorHandling = retval;
                iCommandCompleteErrMsgInErrorHandling = NULL;
                AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_PAUSE, NULL, NULL, NULL, false);
                return PVMFPending;
            }
        }
    }

    if (iNumPendingDatapathCmd == 0)
    {
        // If there are no active datapaths, continue on to pause the source node
        retval = DoSourceNodePause(aCmd.GetCmdId(), aCmd.GetContext());
    }

    if (retval != PVMFSuccess)
    {
        bool ehPending = CheckForPendingErrorHandlingCmd();
        if (ehPending)
        {
            // there should be no error handling queued.
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoPause() Already EH pending, should never happen"));
            return PVMFPending;
        }
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoPause() Pausing datapath and source node failed, Add EH command"));
        iCommandCompleteStatusInErrorHandling = retval;
        iCommandCompleteErrMsgInErrorHandling = NULL;
        AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_PAUSE, NULL, NULL, NULL, false);
        return PVMFPending;
    }

    // TEMP Until queued playback range is available
    // Reset the flag when doing a pause
    iChangePlaybackPositionWhenResuming = false;
    iChangePlaybackDirectionWhenResuming = false;
    // END TEMP

    SetEngineState(PVP_ENGINE_STATE_PAUSING);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoPause() Out"));
    return PVMFSuccess;
}


PVMFStatus PVPlayerEngine::DoDatapathPause(PVPlayerEngineDatapath& aDatapath, PVCommandId aCmdId, OsclAny* aCmdContext, bool aSinkPaused)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_INFO,
                    (0, "PVPlayerEngine::DoDatapathPause() for %s Tick=%d",
                     aDatapath.iTrackInfo->getTrackMimeType().get_cstr(), OsclTickCount::TickCount()));

    if (aDatapath.iTrackInfo == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoDatapathPause() TrackInfo not available, failure"));
        return PVMFFailure;
    }
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoDatapathPause() In %s", aDatapath.iTrackInfo->getTrackMimeType().get_cstr()));

    // Pause the datapath
    PVPlayerEngineContext* context = AllocateEngineContext(&aDatapath, NULL, aDatapath.iDatapath, aCmdId, aCmdContext, -1);

    PVMFStatus retval = aDatapath.iDatapath->Pause((OsclAny*)context, aSinkPaused);
    if (retval != PVMFSuccess)
    {
        FreeEngineContext(context);
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoDatapathPause() Out"));
    return retval;
}


PVMFStatus PVPlayerEngine::DoSourceNodePause(PVCommandId aCmdId, OsclAny* aCmdContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSourceNodePause() In"));

    if (iSourceNode == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSourceNodePause() Source node not available"));
        return PVMFFailure;
    }

    // Pause the source node
    PVPlayerEngineContext* context = AllocateEngineContext(NULL, iSourceNode, NULL, aCmdId, aCmdContext, -1);

    PVMFCommandId cmdid = -1;
    int32 leavecode = 0;
    OSCL_TRY(leavecode, cmdid = iSourceNode->Pause(iSourceNodeSessionId, (OsclAny*)context));
    OSCL_FIRST_CATCH_ANY(leavecode,
                         FreeEngineContext(context);
                         PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSourceNodePause() Pause on iSourceNode did a leave!"));
                         return PVMFFailure);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSourceNodePause() Out"));

    return PVMFSuccess;
}


PVMFStatus PVPlayerEngine::DoResume(PVPlayerEngineCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoResume() In"));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoResume() iNumPendingSkipCompleteEvent: %d", iNumPendingSkipCompleteEvent));

    if (GetPVPlayerState() == PVP_STATE_STARTED)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoResume() Engine already in Started State"));
        EngineCommandCompleted(aCmd.GetCmdId(), aCmd.GetContext(), PVMFSuccess);
        return PVMFSuccess;
    }

    // Check engine state
    if (GetPVPlayerState() != PVP_STATE_PAUSED)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoResume() Wrong engine state"));
        return PVMFErrInvalidState;
    }

    // Disallow resume when paused due to EOS and position/direction
    // hasn't been changed
    if (iPlaybackPausedDueToEndOfClip)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoResume() Currently paused due to EOS so not allowed!"));
        return PVMFErrInvalidState;
    }

    PVMFStatus retval;
    if (iChangePlaybackPositionWhenResuming)
    {
        // Reposition occurred during the paused state so need to change the source position first
        retval = DoSourceNodeQueryDataSourcePosition(aCmd.GetCmdId(), aCmd.GetContext());
        // ignore failure, continue on to Start
        if (retval != PVMFSuccess)
        {
            PVUuid puuid = PVPlayerErrorInfoEventTypesUUID;
            PVMFBasicErrorInfoMessage* infomsg = OSCL_NEW(PVMFBasicErrorInfoMessage, (PVPlayerInfoChangePlaybackPositionNotSupported, puuid, NULL));
            SendInformationalEvent(PVMFInfoChangePlaybackPositionNotSupported, OSCL_STATIC_CAST(PVInterface*, infomsg));
            infomsg->removeRef();
            retval = DoSourceNodeStart(aCmd.GetCmdId(), aCmd.GetContext());
        }
    }
    else if (iChangePlaybackDirectionWhenResuming)
    {
        // Direction change occurred during the paused state so need to change the source direction first
        retval = DoSourceNodeSetDataSourceDirection(aCmd.GetCmdId(), aCmd.GetContext());
        // ignore failure, continue on to Start
        if (retval != PVMFSuccess)
        {
            PVUuid puuid = PVPlayerErrorInfoEventTypesUUID;
            PVMFBasicErrorInfoMessage* infomsg = OSCL_NEW(PVMFBasicErrorInfoMessage, (PVPlayerInfoChangePlaybackPositionNotSupported, puuid, NULL));
            SendInformationalEvent(PVMFInfoChangePlaybackPositionNotSupported, OSCL_STATIC_CAST(PVInterface*, infomsg));
            infomsg->removeRef();
            retval = DoSourceNodeStart(aCmd.GetCmdId(), aCmd.GetContext());
        }
    }
    else
    {
        retval = DoSourceNodeStart(aCmd.GetCmdId(), aCmd.GetContext());
    }

    if (retval != PVMFSuccess)
    {
        bool ehPending = CheckForPendingErrorHandlingCmd();
        if (ehPending)
        {
            // there should be no error handling queued.
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoResume() Already EH pending, should never happen"));
            return PVMFPending;
        }
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoResume() Resuming source node or changing position failed, Add EH command"));
        iCommandCompleteStatusInErrorHandling = retval;
        iCommandCompleteErrMsgInErrorHandling = NULL;
        AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_RESUME, NULL, NULL, NULL, false);
        return PVMFPending;
    }

    SetEngineState(PVP_ENGINE_STATE_RESUMING);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoResume() Out"));
    return PVMFSuccess;
}


PVMFStatus PVPlayerEngine::AddToMetadataInterfaceList(PVMFMetadataExtensionInterface* aMetadataIF, PVMFSessionId aSessionId, PVPlayerEngineDatapath* aEngineDatapath, PVMFNodeInterface* aNode)
{
    // Validate the interface ptr
    if (aMetadataIF == NULL)
    {
        return PVMFErrArgument;
    }

    // Add the specified interface ptr and session ID to the list
    PVPlayerEngineMetadataIFInfo mdifinfo;
    mdifinfo.iInterface = aMetadataIF;
    mdifinfo.iSessionId = aSessionId;
    mdifinfo.iEngineDatapath = aEngineDatapath;
    mdifinfo.iNode = aNode;
    int32 leavecode = 0;
    OSCL_TRY(leavecode, iMetadataIFList.push_back(mdifinfo));
    OSCL_FIRST_CATCH_ANY(leavecode, return PVMFErrNoMemory);

    return PVMFSuccess;
}

PVMFStatus PVPlayerEngine::RemoveFromMetadataInterfaceList(PVMFMetadataExtensionInterface* aMetadataIF, PVMFSessionId aSessionId)
{
    // Validate the interface ptr
    if (aMetadataIF == NULL)
    {
        return PVMFErrArgument;
    }

    // Go through the list to find the specified entry
    for (uint32 i = 0; i < iMetadataIFList.size(); ++i)
    {
        if (aMetadataIF == iMetadataIFList[i].iInterface &&
                aSessionId == iMetadataIFList[i].iSessionId)
        {
            // Found it. Now erase it from the list
            iMetadataIFList.erase(iMetadataIFList.begin() + i);
            return PVMFSuccess;
        }
    }

    // If here that means the specified entry wasn't found in the list
    return PVMFErrArgument;
}


PVMFStatus PVPlayerEngine::DoStop(PVPlayerEngineCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoStop() In"));

    if (GetPVPlayerState() == PVP_STATE_INITIALIZED)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoStop() Engine already in Initialized State"));
        EngineCommandCompleted(aCmd.GetCmdId(), aCmd.GetContext(), PVMFSuccess);
        return PVMFSuccess;
    }

    if (GetPVPlayerState() != PVP_STATE_PREPARED &&
            GetPVPlayerState() != PVP_STATE_STARTED &&
            GetPVPlayerState() != PVP_STATE_PAUSED)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoStop() Wrong engine state"));
        return PVMFErrInvalidState;
    }

    if (iReleaseMetadataValuesPending)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVPlayerEngine::DoStop() Wrong engine Usage, Stop called without releasing metadata values"));
        return PVMFErrReleaseMetadataValueNotDone;
    }

    // reset the dataReady event boolean
    iDataReadySent = false;

    // reset all repos related variables
    ResetReposVariables(true);

    // Stop the playback position status timer
    StopPlaybackStatusTimer();

    // Stop the playback clock
    iPlaybackClock.Stop();
    uint32 starttime = 0;
    bool overflow = 0;
    iPlaybackClock.SetStartTime32(starttime, PVMF_MEDIA_CLOCK_MSEC, overflow);
    iPlaybackDirection = 1;

    // Reset the begin/end time variables
    iCurrentBeginPosition.iIndeterminate = true;
    iCurrentEndPosition.iIndeterminate = true;
    iQueuedBeginPosition.iIndeterminate = true;
    iQueuedEndPosition.iIndeterminate = true;

    // Reset the paused-due-to-EOS flag
    iPlaybackPausedDueToEndOfClip = false;

    // Stop the end time check
    if (iEndTimeCheckEnabled)
    {
        iEndTimeCheckEnabled = false;
        iPollingCheckTimer->Cancel(PVPLAYERENGINE_TIMERID_ENDTIMECHECK);
    }

    PVMFStatus retval = PVMFErrNotSupported;

    // Start the stopping sequence
    // First stop all the active datapaths
    iNumPendingDatapathCmd = 0;
    for (uint32 i = 0; i < iDatapathList.size(); ++i)
    {
        if (iDatapathList[i].iDatapath)
        {
            PVMFStatus retcode = DoDatapathStop(iDatapathList[i], aCmd.GetCmdId(), aCmd.GetContext());
            if (retcode == PVMFSuccess)
            {
                ++iNumPendingDatapathCmd;
                retval = PVMFSuccess;
            }
            else
            {
                retval = retcode;
                break;
            }
        }
    }

    if (iNumPendingDatapathCmd == 0 && retval == PVMFErrNotSupported)
    {
        // If there are no active datapath, stop the source node
        retval = DoSourceNodeStop(aCmd.GetCmdId(), aCmd.GetContext());
    }

    if (retval != PVMFSuccess)
    {
        bool ehPending = CheckForPendingErrorHandlingCmd();
        if (ehPending)
        {
            // there should be no error handling queued.
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoStop() Already EH pending, should never happen"));
            return PVMFPending;
        }
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoStop() source node failed, go in Error handling, Add EH command"));
        iCommandCompleteStatusInErrorHandling = retval;
        iCommandCompleteErrMsgInErrorHandling = NULL;

        AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_STOP, NULL, NULL, NULL, false);
        return PVMFPending;
    }

    SetEngineState(PVP_ENGINE_STATE_STOPPING);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoStop() Out"));
    return PVMFSuccess;
}


PVMFStatus PVPlayerEngine::DoDatapathStop(PVPlayerEngineDatapath& aDatapath, PVCommandId aCmdId, OsclAny* aCmdContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoDatapathStop() In %s", aDatapath.iTrackInfo->getTrackMimeType().get_cstr()));
    if (aDatapath.iTrackInfo == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoDatapathStop() TrackInfo not available, failure"));
        return PVMFFailure;
    }

    // Stop the datapath
    PVPlayerEngineContext* context = AllocateEngineContext(&aDatapath, NULL, aDatapath.iDatapath, aCmdId, aCmdContext, PVP_CMD_DPStop);

    PVMFStatus retval = aDatapath.iDatapath->Stop((OsclAny*)context);
    if (retval != PVMFSuccess)
    {
        FreeEngineContext(context);
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoDatapathStop() Out"));
    return retval;
}


PVMFStatus PVPlayerEngine::DoSourceNodeStop(PVCommandId aCmdId, OsclAny* aCmdContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSourceNodeStop() In"));

    if (iSourceNode == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSourceNodeStop() Source node not available."));
        return PVMFFailure;
    }

    // Stop the source node
    PVPlayerEngineContext* context = AllocateEngineContext(NULL, iSourceNode, NULL, aCmdId, aCmdContext, -1);

    PVMFCommandId cmdid = -1;
    int32 leavecode = 0;
    OSCL_TRY(leavecode, cmdid = iSourceNode->Stop(iSourceNodeSessionId, (OsclAny*)context));
    OSCL_FIRST_CATCH_ANY(leavecode,
                         FreeEngineContext(context);
                         PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSourceNodeStop() Stop on iSourceNode did a leave!"));
                         return PVMFFailure);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSourceNodeStop() Out"));

    return PVMFSuccess;
}


PVMFStatus PVPlayerEngine::DoDatapathTeardown(PVPlayerEngineDatapath &aDatapath, PVCommandId aCmdId, OsclAny* aCmdContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoDatapathTeardown() In %s", aDatapath.iTrackInfo->getTrackMimeType().get_cstr()));

    PVPlayerEngineContext* context = AllocateEngineContext(&aDatapath, NULL, aDatapath.iDatapath, aCmdId, aCmdContext, PVP_CMD_DPTeardown);

    PVMFStatus retval = aDatapath.iDatapath->Teardown((OsclAny*)context);
    if (retval != PVMFSuccess)
    {
        FreeEngineContext(context);
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoDatapathTeardown() Out"));
    return retval;
}


PVMFStatus PVPlayerEngine::DoDatapathReset(PVPlayerEngineDatapath &aDatapath, PVCommandId aCmdId, OsclAny* aCmdContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoDatapathReset() In %s", aDatapath.iTrackInfo->getTrackMimeType().get_cstr()));

    // Reset the datapath
    PVPlayerEngineContext* context = AllocateEngineContext(&aDatapath, NULL, aDatapath.iDatapath, aCmdId, aCmdContext, PVP_CMD_DPReset);

    PVMFStatus retval = aDatapath.iDatapath->Reset((OsclAny*)context);
    if (retval != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoDatapathReset() Reset failed. Asserting"));
        FreeEngineContext(context);
        OSCL_ASSERT(false);
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoDatapathReset() Out"));
    return retval;
}


PVMFStatus PVPlayerEngine::DoRemoveDataSink(PVPlayerEngineCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoRemoveDataSink() In"));

    // previously removed, e.g. during error handling ?
    if (iDatapathList.empty() && GetPVPlayerState() == PVP_STATE_IDLE)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoRemoveDataSink() All sinks were previously deleted"));
        EngineCommandCompleted(aCmd.GetCmdId(), aCmd.GetContext(), PVMFSuccess);
        return PVMFSuccess;
    }

    if (GetPVPlayerState() != PVP_STATE_INITIALIZED)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoRemoveDataSink() Wrong engine state"));
        return PVMFErrInvalidState;
    }

    PVPlayerDataSink* datasink = (PVPlayerDataSink*)(aCmd.GetParam(0).pOsclAny_value);

    if (datasink == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoRemoveDataSink() Passed in parameter invalid"));
        return PVMFErrArgument;
    }

    // Find the track that the passed-in sink belongs to
    PVPlayerEngineDatapath* pvpedp = NULL;
    int32 dpindex = -1;
    for (uint32 i = 0; i < iDatapathList.size(); ++i)
    {
        if (iDatapathList[i].iDataSink == datasink)
        {
            pvpedp = &(iDatapathList[i]);
            dpindex = i;
            break;
        }
    }

    if (pvpedp == NULL || dpindex == -1)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoRemoveDataSink() Passed in data sink does not match with ones in engine"));
        return PVMFFailure;
    }
    else
    {
        // Cleanup and remove the datapath associated with the data sink from the list
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO, (0, "PVPlayerEngine::DoRemoveDataSink() Removing datapath"));
        DoEngineDatapathCleanup(*pvpedp);

        iDatapathList.erase(iDatapathList.begin() + dpindex);
    }

    EngineCommandCompleted(aCmd.GetCmdId(), aCmd.GetContext(), PVMFSuccess);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoRemoveDataSink() Out"));
    return PVMFSuccess;
}

void PVPlayerEngine::DoRemoveAllSinks()
{
    // Clean up the datapaths
    for (uint32 i = 0; i < iDatapathList.size(); ++i)
    {
        DoEngineDatapathCleanup(iDatapathList[i]);
    }
    iDatapathList.clear();
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO, (0, "PVPlayerEngine::DoRemoveAllSinks() all datapaths removed"));
}


PVMFStatus PVPlayerEngine::DoReset(PVPlayerEngineCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoReset() In"));

    // set engine state to Resetting
    SetEngineState(PVP_ENGINE_STATE_RESETTING);
    iRollOverState = RollOverStateIdle; //reset roll over state to Idle, as engine is resetting itself

    // reset all repos related variables
    ResetReposVariables(true);

    // Stop the playback position status timer
    StopPlaybackStatusTimer();

    // Stop the playback clock
    iPlaybackClock.Stop();
    uint32 starttime = 0;
    bool overflow = 0;
    iPlaybackClock.SetStartTime32(starttime, PVMF_MEDIA_CLOCK_MSEC, overflow);
    iPlaybackDirection = 1;

    // Reset the begin/end time variables
    iCurrentBeginPosition.iIndeterminate = true;
    iCurrentEndPosition.iIndeterminate = true;
    iQueuedBeginPosition.iIndeterminate = true;
    iQueuedEndPosition.iIndeterminate = true;

    // Reset the paused-due-to-EOS flag
    iPlaybackPausedDueToEndOfClip = false;

    // Reset the Presentation Info list
    iSourcePresInfoList.Reset();

    // Clear the Track selection List
    iTrackSelectionList.clear();

    // Stop the end time check
    if (iEndTimeCheckEnabled)
    {
        iEndTimeCheckEnabled = false;
        iPollingCheckTimer->Cancel(PVPLAYERENGINE_TIMERID_ENDTIMECHECK);
    }

    int32 leavecode = 0;
    PVMFStatus status = PVMFSuccess;

    if (iSourceNode)
    {
        if (iSourceNode->GetState() != EPVMFNodeCreated)
        {
            PVPlayerEngineContext* context = AllocateEngineContext(NULL, iSourceNode, NULL, -1, NULL, -1);

            PVMFCommandId cmdid = -1;
            leavecode = 0;
            OSCL_TRY(leavecode, cmdid = iSourceNode->Reset(iSourceNodeSessionId, (OsclAny*)context));
            OSCL_FIRST_CATCH_ANY(leavecode,

                                 PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoReset() Reset on iSourceNode did a leave!"));
                                 FreeEngineContext(context);
                                 OSCL_ASSERT(false);
                                 return PVMFFailure);
        }
        else
        {
            // It is assumed that if SourceNode is in created state then datapaths if present,
            // has to be in IDLE State
            if (!iDatapathList.empty())
            {
                for (uint32 i = 0; i < iDatapathList.size(); ++i)
                {
                    if (!iDatapathList[i].iDatapath)
                    {
                        if (iDatapathList[i].iDatapath->iState != PVPDP_IDLE)
                        {
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                            (0, "PVPlayerEngine::DoReset() Source Node already in created state, Datapath not in idle state, asserting"));
                            OSCL_ASSERT(false);
                        }
                    }
                }
                DoRemoveAllSinks();
            }
            if (iDataSource)
            {
                RemoveDataSourceSync(*iDataSource);
            }
            SetEngineState(PVP_ENGINE_STATE_IDLE);
            EngineCommandCompleted(aCmd.GetCmdId(), aCmd.GetContext(), PVMFSuccess);
        }
    }
    else
    {
        if (iDataSource)
        {
            RemoveDataSourceSync(*iDataSource);
        }
        SetEngineState(PVP_ENGINE_STATE_IDLE);
        EngineCommandCompleted(aCmd.GetCmdId(), aCmd.GetContext(), PVMFSuccess);
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoReset() Out"));
    return status;
}

PVMFStatus PVPlayerEngine::RemoveDataSourceSync(PVPlayerDataSource &aSrc)
{
    OSCL_UNUSED_ARG(aSrc);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::RemoveDataSourceSync() In"));

    // Destroy the source node if present
    DoSourceNodeCleanup();

    // Remove Stored KVP Values
    DeleteKVPValues();

    // Remove all metadata IF from the list
    iMetadataIFList.clear();

    iDataSource = NULL;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::RemoveDataSourceSync() Out"));
    return PVMFSuccess;
}


PVMFStatus PVPlayerEngine::DoRemoveDataSource(PVPlayerEngineCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoRemoveDataSource() In"));

    if (GetPVPlayerState() != PVP_STATE_IDLE)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoRemoveDataSource() called when engine is not in the IDLE state"));
        return PVMFErrInvalidState;
    }

    if (iDataSource == NULL) // previously removed, e.g. during errorhandling
    {
        EngineCommandCompleted(aCmd.GetCmdId(), aCmd.GetContext(), PVMFSuccess);
        return PVMFSuccess;
    }

    PVPlayerDataSource* src = (PVPlayerDataSource*)(aCmd.GetParam(0).pOsclAny_value);

    if (iDataSource != src || src == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoRemoveDataSource() Passed in parameter invalid"));
        return PVMFErrArgument;
    }

    PVMFStatus result = RemoveDataSourceSync(*src);

    if (result == PVMFSuccess)
        EngineCommandCompleted(aCmd.GetCmdId(), aCmd.GetContext(), PVMFSuccess);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoRemoveDataSource() Out"));
    return result;
}


PVMFStatus PVPlayerEngine::DoSourceUnderflowAutoPause(PVPlayerEngineCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSourceUnderflowAutoPause() In"));

    /*
     * Underflow can happen in the following States
     * Prepared - Use-cases where-in Source Node reports DataReady and goes into
     * underflow immediately when it starts to retrieve data.
     * Started - This will be the most generic use-case where in the source node
     * is retreiving data and because of low bandwidth conditions runs out of data.
     * In such case Source Node will send an Underflow event to Engine.
     * Paused - A rare use-case but can happen. Use-case where Source Node is about
     * to run out of data and just before Source Node sends an Underflow event to
     * engine, user presses Pause on Engine. Engine gets Pause from user and Underflow
     * from Source Node back-to-back.
     */
    bool pauseSinkNodes = false;

    switch (iState)
    {
        case PVP_ENGINE_STATE_PREPARED:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVPlayerEngine::DoSourceUnderflowAutoPause() in Prepared state"));
            SetEngineState(PVP_ENGINE_STATE_AUTO_PAUSED);
            break;

        case PVP_ENGINE_STATE_STARTED:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVPlayerEngine::DoSourceUnderflowAutoPause() in Started state"));
            pauseSinkNodes = true;
            break;

        case PVP_ENGINE_STATE_PAUSED:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVPlayerEngine::DoSourceUnderflowAutoPause() in Paused state"));
            break;

        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVPlayerEngine::DoSourceUnderflowAutoPause() Invalid state so cancel auto-pause request!"));
            return PVMFErrCancelled;
    }

    // Stop the watchdog timer if active. We will Start the timer again in auto-resume.
    // this should only be done when engine is waiting for StartofData info event
    // after reposition.
    if (iNumPVMFInfoStartOfDataPending > 0)
    {
        if (iWatchDogTimer->IsBusy())
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iReposLogger, PVLOGMSG_INFO,
                            (0, "PVPlayerEngine::DoSourceUnderflowAutoPause - Pause after setplayback, Cancelling Watchdog timer, iNumPVMFInfoStartOfDataPending=%d", iNumPVMFInfoStartOfDataPending));
            iWatchDogTimer->Cancel();
        }
    }

    if (!pauseSinkNodes)
    {
        // Sink nodes already in Paused state or have no data to process, so
        // no need to call Pause on the sink nodes.
        return PVMFErrNotSupported;
    }

    // Pause the playback clock
    iPlaybackClock.Pause();

    uint32 i;
    // Notify data sinks that clock has paused
    for (i = 0; i < iDatapathList.size(); ++i)
    {
        if (iDatapathList[i].iDatapath && iDatapathList[i].iSinkNodeSyncCtrlIF)
        {
            iDatapathList[i].iSinkNodeSyncCtrlIF->ClockStopped();
        }
    }

    PVMFStatus retval = PVMFErrNotSupported;

    // Pause all active sink nodes
    iNumPendingDatapathCmd = 0;
    for (i = 0; i < iDatapathList.size(); ++i)
    {
        if (iDatapathList[i].iDatapath)
        {
            retval = DoSinkNodePause(iDatapathList[i], aCmd.GetCmdId(), aCmd.GetContext());
            if (retval == PVMFSuccess)
            {
                ++iNumPendingDatapathCmd;
            }
            else
            {
                break;
            }
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSourceUnderflowAutoPause() Out"));
    if (iNumPendingDatapathCmd == 0)
    {
        return PVMFErrNotSupported;
    }
    else
    {
        SetEngineState(PVP_ENGINE_STATE_AUTO_PAUSING);
        return retval;
    }
}


PVMFStatus PVPlayerEngine::DoSourceDataReadyAutoResume(PVPlayerEngineCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSourceDataReadyAutoResume() In"));

    // Don't need to worry about transitional states(...ING).
    // Auto-pause/resume cmds are just regular engine cmds and won't be interrupted by normal ones

    // Check if Datapaths (Sink Node) are already in Started state.
    bool datapathSinkNodeStarted = false;
    for (uint32 j = 0; j < iDatapathList.size(); j++)
    {
        if (iDatapathList[j].iSinkNode)
        {
            if (iDatapathList[j].iSinkNode->GetState() != EPVMFNodeStarted)
            {
                // One of the nodes is not in Started state break from the loop
                // keeping the boolean datapathSinkNodeStaretd as false.
                datapathSinkNodeStarted = false;
                break;
            }
            // this will be true only when all Sink Nodes are in started state.
            datapathSinkNodeStarted = true;
        }
    }

    // Next check to see if it is any one of the use-cases:
    // Prepare->Underflow->Start->DataReady or
    // Prepare->Underflow->DataReady->Start or
    // Underflow->Pause->Resume->DataReady or
    // Underflow->Pause->SetPlaybackRange->Resume->DataReady
    // These are cases where Sink Nodes are already in Started state and
    // engine might be still waiting for PVMFInfoStartOfData.
    // Here if all PVMFInfoStartofData have not been received yet,
    // then iNumPVMFInfoStartOfDataPending would be non-zero,
    // In few of these usecase, engine starts playback clock in Resume, source nodes are sposed to pause the
    // clock, since they are the ones in underflow. Once source nodes report dataready, engine would
    // already be in STARTED state. So if the clock is still paused, then start it here.
    // Here just send NotSupported so engine can send DataReady Event to the app.
    // and set the watchdog timer which was cancelled when underflow was recieved.
    if (datapathSinkNodeStarted)
    {
        if (iState == PVP_ENGINE_STATE_PREPARED)
        {
            // DataReady recieved during Prepare, Engine just needs to send
            // DataReady event.
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVPlayerEngine::DoSourceDataReadyAutoResume: DataReady rcvd, Engine in Prepared state"));
        }
        else if (iState == PVP_ENGINE_STATE_STARTED)
        {
            // Usecases for this scenario:
            // Underflow->Pause->Resume->DataReady
            // Underflow->Pause->SetPlaybackRange->Resume->DataReady
            // Prepare->Underflow->Start->DataReady
            // do nothing here
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVPlayerEngine::DoSourceDataReadyAutoResume: DataReady rcvd, Engine already in Started state"));
        }
        else if (iState == PVP_ENGINE_STATE_AUTO_PAUSED)
        {
            // Usecase for this scenario:
            // Prepare->Underflow->DataReady->Started
            // Change state to STARTED
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVPlayerEngine::DoSourceDataReadyAutoResume: DataReady rcvd, Prepare->Underflow->DataReady->Started, datapaths already started"));
            SetEngineState(PVP_ENGINE_STATE_STARTED);
        }
        else
        {
            // This should never happen
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVPlayerEngine::DoSourceDataReadyAutoResume() Invalid state %d, Sinks in Started state", iState));
            OSCL_ASSERT(false);
        }

        if (iNumPVMFInfoStartOfDataPending > 0)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVPlayerEngine::DoSourceDataReadyAutoResume: Clock in Stopped state, waiting for StartofData"));

            if (iWatchDogTimerInterval > 0)
            {
                if (iWatchDogTimer->IsBusy())
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iReposLogger, PVLOGMSG_INFO,
                                    (0, "PVPlayerEngine::DoSourceDataReadyAutoResume - Pause after setplayback, Cancelling Watchdog timer, iNumPVMFInfoStartOfDataPending=%d", iNumPVMFInfoStartOfDataPending));
                    iWatchDogTimer->Cancel();
                }

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iReposLogger, PVLOGMSG_INFO,
                                (0, "PVPlayerEngine::DoSourceDataReadyAutoResume Setting WatchDogTimer for %d ms, iNumPVMFInfoStartOfDataPending=%d",
                                 iWatchDogTimerInterval, iNumPVMFInfoStartOfDataPending));
                iWatchDogTimer->setTimerDuration(iWatchDogTimerInterval);
                iWatchDogTimer->Start();
            }
        }
        else if (iNumPVMFInfoStartOfDataPending == 0)
        {
            StartPlaybackClock();

            // Notify data sinks that clock has started
            for (uint32 i = 0; i < iDatapathList.size(); ++i)
            {
                if (iDatapathList[i].iDatapath && iDatapathList[i].iSinkNodeSyncCtrlIF)
                {
                    iDatapathList[i].iSinkNodeSyncCtrlIF->ClockStarted();
                }
            }
        }
        //return PVMFErrNotSupported so the the DataReady can be sent, depending on iDataReadySent flag.
        return PVMFErrNotSupported;
    }

    // Next check to see if it is Underflow->Pause->DataReady->Resume usecase.
    // Then we CANNOT start clock in here, because clock is paused by app.
    // After Pause done, the engine is in PAUSED state. By allowing auto-resume only when
    // auto-paused we deal with this usecase ok.
    if (iState != PVP_ENGINE_STATE_AUTO_PAUSED)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSourceDataReadyAutoResume() Invalid state %d", iState));
        //return PVMFErrNotSupported so the the DataReady can be sent, depending on iDataReadySent flag.
        return PVMFErrNotSupported;
    }

    PVMFStatus retval = PVMFErrNotSupported;

    // Resume all active sink nodes
    iNumPendingDatapathCmd = 0;
    for (uint32 i = 0; i < iDatapathList.size(); ++i)
    {
        if (iDatapathList[i].iDatapath)
        {
            retval = DoSinkNodeResume(iDatapathList[i], aCmd.GetCmdId(), aCmd.GetContext());
            if (retval == PVMFSuccess)
            {
                ++iNumPendingDatapathCmd;
            }
            else
            {
                break;
            }
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSourceDataReadyAutoResume() Out"));
    if (iNumPendingDatapathCmd == 0)
    {
        return PVMFErrNotSupported;
    }
    else
    {
        SetEngineState(PVP_ENGINE_STATE_AUTO_RESUMING);
        return retval;
    }
}


PVMFStatus PVPlayerEngine::DoSinkNodePause(PVPlayerEngineDatapath &aDatapath, PVCommandId aCmdId, OsclAny* aCmdContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSinkNodePause() In %s", aDatapath.iTrackInfo->getTrackMimeType().get_cstr()));

    if (!(aDatapath.iTrackInfo) || !(aDatapath.iSinkNode))
    {
        return PVMFErrNotSupported;
    }

    // Pause the sink node
    PVPlayerEngineContext* context = AllocateEngineContext(&aDatapath, aDatapath.iSinkNode, NULL, aCmdId, aCmdContext, PVP_CMD_SinkNodeAutoPause);

    PVMFCommandId cmdid = -1;
    int32 leavecode = 0;
    OSCL_TRY(leavecode, cmdid = aDatapath.iSinkNode->Pause(aDatapath.iSinkNodeSessionId, (OsclAny*)context));
    OSCL_FIRST_CATCH_ANY(leavecode,
                         FreeEngineContext(context);
                         PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSinkNodePause() Pause on sink node did a leave!"));
                         return PVMFFailure);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSinkNodePause() Out"));

    return PVMFSuccess;
}


PVMFStatus PVPlayerEngine::DoSinkNodeResume(PVPlayerEngineDatapath &aDatapath, PVCommandId aCmdId, OsclAny* aCmdContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSinkNodeResume() In %s", aDatapath.iTrackInfo->getTrackMimeType().get_cstr()));

    if (!(aDatapath.iTrackInfo) || !(aDatapath.iSinkNode))
    {
        return PVMFErrNotSupported;
    }

    // Start the sink node to resume
    PVPlayerEngineContext* context = AllocateEngineContext(&aDatapath, aDatapath.iSinkNode, NULL, aCmdId, aCmdContext, PVP_CMD_SinkNodeAutoResume);

    PVMFCommandId cmdid = -1;
    int32 leavecode = 0;
    OSCL_TRY(leavecode, cmdid = aDatapath.iSinkNode->Start(aDatapath.iSinkNodeSessionId, (OsclAny*)context));
    OSCL_FIRST_CATCH_ANY(leavecode,
                         FreeEngineContext(context);
                         PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSinkNodeResume() Start on sink node did a leave!"));
                         return PVMFFailure);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSinkNodeResume() Out"));

    return PVMFSuccess;
}

void PVPlayerEngine::DoEngineDatapathTeardown(PVPlayerEngineDatapath& aDatapath)
{
    if (aDatapath.iTrackInfo)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoEngineDatapathTeardown() In %s", aDatapath.iTrackInfo->getTrackMimeType().get_cstr()));
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoEngineDatapathTeardown() In"));
    }

    if (aDatapath.iDatapath)
    {
        // Shutdown and reset the datapath
        aDatapath.iDatapath->DisconnectNodeSession();
        aDatapath.iDatapath->SetSinkNode(NULL);
        aDatapath.iDatapath->SetDecNode(NULL);
        aDatapath.iDatapath->SetSourceNode(NULL);
    }

    if (aDatapath.iSinkNode)
    {
        PVMFStatus status = PVMFFailure;
        status = aDatapath.iSinkNode->Disconnect(aDatapath.iSinkNodeSessionId);
        if (status == PVMFFailure)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoEngineDatapathTeardown() Disconnect on Sink Node Failed"));
            OSCL_ASSERT(false);
        }
        status = aDatapath.iSinkNode->ThreadLogoff();
        if (status == PVMFFailure)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoEngineDatapathTeardown() Threadlogoff on SinkNode Failed"));
            OSCL_ASSERT(false);
        }

        // Remove sync ctrl IF if available
        if (aDatapath.iSinkNodeSyncCtrlIF)
        {
            aDatapath.iSinkNodeSyncCtrlIF->SetClock(NULL);
            aDatapath.iSinkNodeSyncCtrlIF->removeRef();
            aDatapath.iSinkNodeSyncCtrlIF = NULL;
        }

        // Remove metadata IF if available
        if (aDatapath.iSinkNodeMetadataExtIF)
        {
            RemoveFromMetadataInterfaceList(aDatapath.iSinkNodeMetadataExtIF, aDatapath.iSinkNodeSessionId);
            aDatapath.iSinkNodeMetadataExtIF->removeRef();
            aDatapath.iSinkNodeMetadataExtIF = NULL;
        }

        // Remove cap-config IF if available
        if (aDatapath.iSinkNodeCapConfigIF)
        {
            aDatapath.iSinkNodeCapConfigIF = NULL;
        }

        if (aDatapath.iDataSink)
        {
            if (aDatapath.iDataSink->GetDataSinkType() == PVP_DATASINKTYPE_FILENAME)
            {
                // Remove file output config IF if available
                if (aDatapath.iSinkNodeFOConfigIF)
                {
                    aDatapath.iSinkNodeFOConfigIF->removeRef();
                    aDatapath.iSinkNodeFOConfigIF = NULL;
                }
                // Delete the sink node since engine created it.
                PVFileOutputNodeFactory::DeleteFileOutput(aDatapath.iSinkNode);
            }
        }
        aDatapath.iSinkNode = NULL;
    }

    if (aDatapath.iDecNode)
    {
        // Remove metadata IF if available
        if (aDatapath.iDecNodeMetadataExtIF)
        {
            RemoveFromMetadataInterfaceList(aDatapath.iDecNodeMetadataExtIF, aDatapath.iDecNodeSessionId);
            aDatapath.iDecNodeMetadataExtIF->removeRef();
            aDatapath.iDecNodeMetadataExtIF = NULL;
        }

        // Remove cap-config IF if available
        if (aDatapath.iDecNodeCapConfigIF)
        {
            aDatapath.iDecNodeCapConfigIF = NULL;
        }

        PVMFStatus status = PVMFFailure;
        status = aDatapath.iDecNode->Disconnect(aDatapath.iDecNodeSessionId);
        if (status == PVMFFailure)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoEngineDatapathTeardown() Disconnect on dec node Failed"));
            OSCL_ASSERT(false);
        }
        status = aDatapath.iDecNode->ThreadLogoff();
        if (status == PVMFFailure)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoEngineDatapathTeardown() ThreadLogoff on dec node Failed"));
            OSCL_ASSERT(false);
        }

        // search for matching uuid entry in list of instantiated nodes
        PVPlayerEngineUuidNodeMapping* iter = iNodeUuids.begin();
        for (; iter != iNodeUuids.end(); ++iter)
            if (iter->iNode == aDatapath.iDecNode)
                break;

        if (iter != iNodeUuids.end())
        {
            bool release_status = false;

            release_status = iPlayerNodeRegistry.ReleaseNode(iter->iUuid, aDatapath.iDecNode);

            if (release_status == false)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoEngineDatapathTeardown() Factory returned false while releasing the decnode"));
                return;
            }

            iNodeUuids.erase(iter);
            aDatapath.iDecNode = NULL;
        }
        else
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoEngineDatapathTeardown() decnode not found"));
            return;
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoEngineDatapathTeardown() Out"));
}


void PVPlayerEngine::DoEngineDatapathCleanup(PVPlayerEngineDatapath& aDatapath)
{
    if (aDatapath.iTrackInfo)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoEngineDatapathCleanup() In %s", aDatapath.iTrackInfo->getTrackMimeType().get_cstr()));
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoEngineDatapathCleanup() In"));
    }

    DoEngineDatapathTeardown(aDatapath);

    // Destroy the datapath utility object instance
    if (aDatapath.iDatapath)
    {
        OSCL_DELETE(aDatapath.iDatapath);
        aDatapath.iDatapath = NULL;
    }

    // Destroy the track info
    if (aDatapath.iTrackInfo)
    {
        OSCL_DELETE(aDatapath.iTrackInfo);
        aDatapath.iTrackInfo = NULL;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoEngineDatapathCleanup() Out"));
}


void PVPlayerEngine::DoSourceNodeCleanup(void)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSourceNodeCleanup() In"));

    if (iSourceNode)
    {
        // Remove reference to the parser node init interface if available
        if (iSourceNodeInitIF)
        {
            iSourceNodeInitIF->removeRef();
            iSourceNodeInitIF = NULL;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO, (0, "PVPlayerEngine::DoSourceNodeCleanup() - iSourceNodeInitIF Released"));
        }

        // Remove reference to the parser node track sel interface if available
        if (iSourceNodeTrackSelIF)
        {
            iPlayableList.Reset();
            iPreferenceList.Reset();
            iSourceNodeTrackSelIF->removeRef();
            iSourceNodeTrackSelIF = NULL;
            iTrackSelectionHelper = NULL;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO, (0, "PVPlayerEngine::DoSourceNodeCleanup() - iSourceNodeTrackSelIF Released"));
        }

        // Remove reference to the parser node track level info interface if available
        if (iSourceNodeTrackLevelInfoIF)
        {
            iSourceNodeTrackLevelInfoIF->removeRef();
            iSourceNodeTrackLevelInfoIF = NULL;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO, (0, "PVPlayerEngine::DoSourceNodeCleanup() - iSourceNodeTrackLevelInfoIF Released"));
        }

        // Remove reference to the parser node position control interface if available
        if (iSourceNodePBCtrlIF)
        {
            iSourceNodePBCtrlIF->removeRef();
            iSourceNodePBCtrlIF = NULL;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO, (0, "PVPlayerEngine::DoSourceNodeCleanup() - iSourceNodePBCtrlIF Released"));
        }

        // Remove reference to the parser node direction control interface if available
        if (iSourceNodeDirCtrlIF)
        {
            iSourceNodeDirCtrlIF->removeRef();
            iSourceNodeDirCtrlIF = NULL;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO, (0, "PVPlayerEngine::DoSourceNodeCleanup() - iSourceNodeDirCtrlIF Released"));
        }

        // Remove reference to the parser node metadata interface if available
        if (iSourceNodeMetadataExtIF)
        {
            RemoveFromMetadataInterfaceList(iSourceNodeMetadataExtIF, iSourceNodeSessionId);
            iSourceNodeMetadataExtIF->removeRef();
            iSourceNodeMetadataExtIF = NULL;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO, (0, "PVPlayerEngine::DoSourceNodeCleanup() - iSourceNodeMetadataExtIF Released"));
        }

        // Reset the duration value retrieved from source
        iSourceDurationAvailable = false;
        iSourceDurationInMS = 0;

        // Remove reference to the parser node cap-config interface if available
        if (iSourceNodeCapConfigIF)
        {
            iSourceNodeCapConfigIF->removeRef();
            iSourceNodeCapConfigIF = NULL;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO, (0, "PVPlayerEngine::DoSourceNodeCleanup() - iSourceNodeCapConfigIF Released"));
        }

        if (iSourceNodeCPMLicenseIF)
        {
            iSourceNodeCPMLicenseIF->removeRef();
            iSourceNodeCPMLicenseIF = NULL;
        }

        // Reset the Presentation Info list
        iSourcePresInfoList.Reset();

        PVMFStatus status = PVMFFailure;
        status = iSourceNode->Disconnect(iSourceNodeSessionId);
        if (status == PVMFFailure)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSourceNodeCleanup() Disconnect Failed"));
            OSCL_ASSERT(false);
        }
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO, (0, "PVPlayerEngine::DoSourceNodeCleanup() - DisConnect Done"));

        status = iSourceNode->ThreadLogoff();
        if (status == PVMFFailure)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSourceNodeCleanup() ThreadLogoff Failed"));
            OSCL_ASSERT(false);
        }
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO, (0, "PVPlayerEngine::DoSourceNodeCleanup() - ThreadLogoff Done"));

        // search for matching uuid entry in list of instantiated nodes
        PVPlayerEngineUuidNodeMapping* iter = iNodeUuids.begin();
        for (; iter != iNodeUuids.end(); ++iter)
            if (iter->iNode == iSourceNode)
                break;

        if (iter != iNodeUuids.end())
        {
            bool release_status = false;

            release_status = iPlayerNodeRegistry.ReleaseNode(iter->iUuid, iSourceNode);

            if (release_status == false)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSourceNodeCleanup() Factory returned false while releasing the sourcenode"));
                return;
            }


            iNodeUuids.erase(iter);
            iSourceNode = NULL;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO, (0, "PVPlayerEngine::DoSourceNodeCleanup() - iSourceNode Delete Done"));
        }
        else
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO, (0, "PVPlayerEngine::DoSourceNodeCleanup() - iSourceNode not found"));
            return;
        }
    }

    // Cleanup the control varibles related to rate & direction changes.
    iPlaybackDirection_New = iPlaybackDirection;
    iOutsideTimebase_New = iOutsideTimebase;
    iPlaybackClockRate_New = iPlaybackClockRate;
    iChangeDirectionNPT.iIndeterminate = true;
    // Reset the flag that tracks pause-due-to-EOS
    iPlaybackPausedDueToEndOfClip = false;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSourceNodeCleanup() Out"));
}


PVMFStatus PVPlayerEngine::DoCapConfigGetParametersSync(PvmiKeyType aIdentifier, PvmiKvp*& aParameters, int& aNumParamElements, PvmiCapabilityContext aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoCapConfigGetParametersSync() In"));
    OSCL_UNUSED_ARG(aContext);

    // Initialize the output parameters
    aNumParamElements = 0;
    aParameters = NULL;

    // Count the number of components and parameters in the key
    int compcount = pv_mime_string_compcnt(aIdentifier);
    // Retrieve the first component from the key string
    char* compstr = NULL;
    pv_mime_string_extract_type(0, aIdentifier, compstr);

    if ((pv_mime_strcmp(compstr, _STRLIT_CHAR("x-pvmf")) < 0) || compcount < 2)
    {
        // First component should be "x-pvmf" and there must
        // be at least two components to go past x-pvmf
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoCapConfigGetParametersSync() Invalid key string"));
        return PVMFErrArgument;
    }

    // Retrieve the second component from the key string
    pv_mime_string_extract_type(1, aIdentifier, compstr);

    // First check if it is key string for engine ("player")
    if (pv_mime_strcmp(compstr, _STRLIT_CHAR("player")) >= 0)
    {
        // Key is for player
        if (compcount == 2)
        {
            // Since key is "x-pvmf/player" return all
            // nodes available at this level. Ignore attribute
            // since capability is only allowed

            // Allocate memory for the KVP list
            aParameters = (PvmiKvp*)oscl_malloc(PVPLAYERCONFIG_BASE_NUMKEYS * sizeof(PvmiKvp));
            if (aParameters == NULL)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoCapConfigGetParametersSync() Memory allocation for KVP failed"));
                return PVMFErrNoMemory;
            }
            oscl_memset(aParameters, 0, PVPLAYERCONFIG_BASE_NUMKEYS*sizeof(PvmiKvp));
            // Allocate memory for the key strings in each KVP
            PvmiKeyType memblock = (PvmiKeyType)oscl_malloc(PVPLAYERCONFIG_BASE_NUMKEYS * PVPLAYERCONFIG_KEYSTRING_SIZE * sizeof(char));
            if (memblock == NULL)
            {
                oscl_free(aParameters);
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoCapConfigGetParametersSync() Memory allocation for key string failed"));
                return PVMFErrNoMemory;
            }
            oscl_strset(memblock, 0, PVPLAYERCONFIG_BASE_NUMKEYS*PVPLAYERCONFIG_KEYSTRING_SIZE*sizeof(char));
            // Assign the key string buffer to each KVP
            int32 j;
            for (j = 0; j < PVPLAYERCONFIG_BASE_NUMKEYS; ++j)
            {
                aParameters[j].key = memblock + (j * PVPLAYERCONFIG_KEYSTRING_SIZE);
            }
            // Copy the requested info
            for (j = 0; j < PVPLAYERCONFIG_BASE_NUMKEYS; ++j)
            {
                oscl_strncat(aParameters[j].key, _STRLIT_CHAR("x-pvmf/player/"), 14);
                oscl_strncat(aParameters[j].key, PVPlayerConfigBaseKeys[j].iString, oscl_strlen(PVPlayerConfigBaseKeys[j].iString));
                oscl_strncat(aParameters[j].key, _STRLIT_CHAR(";type="), 6);
                switch (PVPlayerConfigBaseKeys[j].iType)
                {
                    case PVMI_KVPTYPE_AGGREGATE:
                        oscl_strncat(aParameters[j].key, _STRLIT_CHAR(PVMI_KVPTYPE_AGGREGATE_STRING), oscl_strlen(PVMI_KVPTYPE_AGGREGATE_STRING));
                        break;

                    case PVMI_KVPTYPE_POINTER:
                        oscl_strncat(aParameters[j].key, _STRLIT_CHAR(PVMI_KVPTYPE_POINTER_STRING), oscl_strlen(PVMI_KVPTYPE_POINTER_STRING));
                        break;

                    case PVMI_KVPTYPE_VALUE:
                    default:
                        oscl_strncat(aParameters[j].key, _STRLIT_CHAR(PVMI_KVPTYPE_VALUE_STRING), oscl_strlen(PVMI_KVPTYPE_VALUE_STRING));
                        break;
                }
                oscl_strncat(aParameters[j].key, _STRLIT_CHAR(";valtype="), 9);
                switch (PVPlayerConfigBaseKeys[j].iValueType)
                {
                    case PVMI_KVPVALTYPE_RANGE_INT32:
                        oscl_strncat(aParameters[j].key, _STRLIT_CHAR(PVMI_KVPVALTYPE_RANGE_INT32_STRING), oscl_strlen(PVMI_KVPVALTYPE_RANGE_INT32_STRING));
                        break;

                    case PVMI_KVPVALTYPE_KSV:
                        oscl_strncat(aParameters[j].key, _STRLIT_CHAR(PVMI_KVPVALTYPE_KSV_STRING), oscl_strlen(PVMI_KVPVALTYPE_KSV_STRING));
                        break;

                    case PVMI_KVPVALTYPE_CHARPTR:
                        oscl_strncat(aParameters[j].key, _STRLIT_CHAR(PVMI_KVPVALTYPE_CHARPTR_STRING), oscl_strlen(PVMI_KVPVALTYPE_CHARPTR_STRING));
                        break;

                    case PVMI_KVPVALTYPE_BOOL:
                        oscl_strncat(aParameters[j].key, _STRLIT_CHAR(PVMI_KVPVALTYPE_BOOL_STRING), oscl_strlen(PVMI_KVPVALTYPE_BOOL_STRING));
                        break;

                    case PVMI_KVPVALTYPE_UINT32:
                    default:
                        oscl_strncat(aParameters[j].key, _STRLIT_CHAR(PVMI_KVPVALTYPE_UINT32_STRING), oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING));
                        break;
                }
                aParameters[j].key[PVPLAYERCONFIG_KEYSTRING_SIZE-1] = 0;
            }

            aNumParamElements = PVPLAYERCONFIG_BASE_NUMKEYS;
        }
        else
        {
            // Retrieve the third component from the key string
            pv_mime_string_extract_type(2, aIdentifier, compstr);

            for (int32 engcomp3ind = 0; engcomp3ind < PVPLAYERCONFIG_BASE_NUMKEYS; ++engcomp3ind)
            {
                // Go through each engine component string at 3rd level
                if (pv_mime_strcmp(compstr, (char*)(PVPlayerConfigBaseKeys[engcomp3ind].iString)) >= 0)
                {
                    if (engcomp3ind == PRODUCTINFO)
                    {
                        // "x-pvmf/player/productinfo"
                        if (compcount == 3)
                        {
                            // Return list of product info. Ignore the
                            // attribute since capability is only allowed

                            // Allocate memory for the KVP list
                            aParameters = (PvmiKvp*)oscl_malloc(PVPLAYERCONFIG_PRODINFO_NUMKEYS * sizeof(PvmiKvp));
                            if (aParameters == NULL)
                            {
                                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoCapConfigGetParametersSync() Memory allocation for KVP failed"));
                                return PVMFErrNoMemory;
                            }
                            oscl_memset(aParameters, 0, PVPLAYERCONFIG_PRODINFO_NUMKEYS*sizeof(PvmiKvp));
                            // Allocate memory for the key strings in each KVP
                            PvmiKeyType memblock = (PvmiKeyType)oscl_malloc(PVPLAYERCONFIG_PRODINFO_NUMKEYS * PVPLAYERCONFIG_KEYSTRING_SIZE * sizeof(char));
                            if (memblock == NULL)
                            {
                                oscl_free(aParameters);
                                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoCapConfigGetParametersSync() Memory allocation for key string failed"));
                                return PVMFErrNoMemory;
                            }
                            oscl_strset(memblock, 0, PVPLAYERCONFIG_PRODINFO_NUMKEYS*PVPLAYERCONFIG_KEYSTRING_SIZE*sizeof(char));
                            // Assign the key string buffer to each KVP
                            int32 j;
                            for (j = 0; j < PVPLAYERCONFIG_PRODINFO_NUMKEYS; ++j)
                            {
                                aParameters[j].key = memblock + (j * PVPLAYERCONFIG_KEYSTRING_SIZE);
                            }
                            // Copy the requested info
                            for (j = 0; j < PVPLAYERCONFIG_PRODINFO_NUMKEYS; ++j)
                            {
                                oscl_strncat(aParameters[j].key, _STRLIT_CHAR("x-pvmf/player/productinfo/"), 26);
                                oscl_strncat(aParameters[j].key, PVPlayerConfigProdInfoKeys[j].iString, oscl_strlen(PVPlayerConfigProdInfoKeys[j].iString));
                                oscl_strncat(aParameters[j].key, _STRLIT_CHAR(";type=value;valtype=char*"), 25);
                                aParameters[j].key[PVPLAYERCONFIG_KEYSTRING_SIZE-1] = 0;
                            }

                            aNumParamElements = PVPLAYERCONFIG_PRODINFO_NUMKEYS;
                        }
                        else if (compcount == 4)
                        {
                            // Retrieve the fourth component from the key string
                            pv_mime_string_extract_type(3, aIdentifier, compstr);

                            for (int32 engcomp4ind = 0; engcomp4ind < PVPLAYERCONFIG_PRODINFO_NUMKEYS; ++engcomp4ind)
                            {
                                if (pv_mime_strcmp(compstr, (char*)(PVPlayerConfigProdInfoKeys[engcomp4ind].iString)) >= 0)
                                {
                                    // Determine what is requested
                                    PvmiKvpAttr reqattr = GetAttrTypeFromKeyString(aIdentifier);
                                    if (reqattr == PVMI_KVPATTR_UNKNOWN)
                                    {
                                        // Default is current setting
                                        reqattr = PVMI_KVPATTR_CUR;
                                    }

                                    // Return the requested info
                                    PVMFStatus retval = DoGetPlayerProductInfoParameter(aParameters, aNumParamElements, engcomp4ind, reqattr);
                                    if (retval != PVMFSuccess)
                                    {
                                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoCapConfigGetParametersSync() Retrieving product info failed"));
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
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoCapConfigGetParametersSync() Unsupported key"));
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
                            PVMFStatus retval = DoGetPlayerParameter(aParameters, aNumParamElements, engcomp3ind, reqattr);
                            if (retval != PVMFSuccess)
                            {
                                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoCapConfigGetParametersSync() Retrieving player parameter failed"));
                                return retval;
                            }
                        }
                        else
                        {
                            // Right now engine doesn't support more than 3 components
                            // for this sub-key string so error out
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoCapConfigGetParametersSync() Unsupported key"));
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
        PVMFStatus retval = PVMFFailure;

        // Go through each datapath's cap-config IF in the datapath list and check for the string.
        for (uint32 i = 0; i < iDatapathList.size(); i++)
        {
            if (iDatapathList[i].iDecNodeCapConfigIF != NULL)
            {
                retval = iDatapathList[i].iDecNodeCapConfigIF->getParametersSync(NULL, aIdentifier, aParameters, aNumParamElements, aContext);
                if (retval == PVMFSuccess)
                {
                    // Key matched break the loop.
                    break;
                }
            }

            if (iDatapathList[i].iSinkNodeCapConfigIF != NULL)
            {
                retval = iDatapathList[i].iSinkNodeCapConfigIF->getParametersSync(NULL, aIdentifier, aParameters, aNumParamElements, aContext);
                if (retval == PVMFSuccess)
                {
                    // Key matched break the loop.
                    break;
                }
            }
        }

        if (retval != PVMFSuccess)
        {
            return retval;
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoCapConfigGetParametersSync() Out"));
    if (aNumParamElements == 0)
    {
        // If no one could get the parameter, return error
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoCapConfigGetParametersSync() Unsupported key"));
        return PVMFFailure;
    }
    else
    {
        return PVMFSuccess;
    }
}


PVMFStatus PVPlayerEngine::DoCapConfigReleaseParameters(PvmiKvp* aParameters, int aNumElements)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoCapConfigReleaseParameters() In"));

    if (aParameters == NULL || aNumElements < 1)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoCapConfigReleaseParameters() KVP list is NULL or number of elements is 0"));
        return PVMFErrArgument;
    }

    // Count the number of components and parameters in the key
    int compcount = pv_mime_string_compcnt(aParameters[0].key);
    // Retrieve the first component from the key string
    char* compstr = NULL;
    pv_mime_string_extract_type(0, aParameters[0].key, compstr);

    if ((pv_mime_strcmp(compstr, _STRLIT_CHAR("x-pvmf")) < 0) || compcount < 2)
    {
        // First component should be "x-pvmf" and there must
        // be at least two components to go past x-pvmf
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoCapConfigReleaseParameters() Unsupported key"));
        return PVMFErrArgument;
    }

    // Retrieve the second component from the key string
    pv_mime_string_extract_type(1, aParameters[0].key, compstr);

    // Assume all the parameters come from the same component so the base components are the same
    // First check if it is key string for engine ("player")
    if (pv_mime_strcmp(compstr, _STRLIT_CHAR("player")) >= 0)
    {
        // Go through each KVP and release memory for value if allocated from heap
        for (int32 i = 0; i < aNumElements; ++i)
        {
            // Next check if it is a value type that allocated memory
            PvmiKvpType kvptype = GetTypeFromKeyString(aParameters[i].key);
            if (kvptype == PVMI_KVPTYPE_VALUE || kvptype == PVMI_KVPTYPE_UNKNOWN)
            {
                PvmiKvpValueType keyvaltype = GetValTypeFromKeyString(aParameters[i].key);
                if (keyvaltype == PVMI_KVPVALTYPE_UNKNOWN)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoCapConfigReleaseParameters() Valtype not specified in key string"));
                    return PVMFErrArgument;
                }

                if (keyvaltype == PVMI_KVPVALTYPE_CHARPTR && aParameters[i].value.pChar_value != NULL)
                {
                    oscl_free(aParameters[i].value.pChar_value);
                    aParameters[i].value.pChar_value = NULL;
                }
                else if (keyvaltype == PVMI_KVPVALTYPE_KSV && aParameters[i].value.key_specific_value != NULL)
                {
                    oscl_free(aParameters[i].value.key_specific_value);
                    aParameters[i].value.key_specific_value = NULL;
                }
                else if (keyvaltype == PVMI_KVPVALTYPE_RANGE_INT32 && aParameters[i].value.key_specific_value != NULL)
                {
                    range_int32* ri32 = (range_int32*)aParameters[i].value.key_specific_value;
                    aParameters[i].value.key_specific_value = NULL;
                    oscl_free(ri32);
                }
                else if (keyvaltype == PVMI_KVPVALTYPE_RANGE_UINT32 && aParameters[i].value.key_specific_value != NULL)
                {
                    range_uint32* rui32 = (range_uint32*)aParameters[i].value.key_specific_value;
                    aParameters[i].value.key_specific_value = NULL;
                    oscl_free(rui32);
                }
                // @TODO Add more types if engine starts returning more types
            }
        }

        // Player engine allocated its key strings in one chunk so just free the first key string ptr
        oscl_free(aParameters[0].key);

        // Free memory for the parameter list
        oscl_free(aParameters);
        aParameters = NULL;
    }
    else
    {
        PVMFStatus retval = PVMFFailure;

        // Go through each datapath's cap-config IF in the datapath list and check for the string.
        for (uint32 i = 0; i < iDatapathList.size(); i++)
        {
            if (iDatapathList[i].iDecNodeCapConfigIF != NULL)
            {
                retval = iDatapathList[i].iDecNodeCapConfigIF->releaseParameters(NULL, aParameters, aNumElements);
                if (retval == PVMFSuccess)
                {
                    // Key matched break the loop.
                    break;
                }
            }

            if (iDatapathList[i].iSinkNodeCapConfigIF != NULL)
            {
                retval = iDatapathList[i].iSinkNodeCapConfigIF->releaseParameters(NULL, aParameters, aNumElements);
                if (retval == PVMFSuccess)
                {
                    // Key matched break the loop.
                    break;
                }
            }
        }

        if (retval != PVMFSuccess)
        {
            return retval;
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoCapConfigReleaseParameters() Out"));
    return PVMFSuccess;
}


PVMFStatus PVPlayerEngine::DoCapConfigSetParameters(PVPlayerEngineCommand& aCmd, bool aSyncCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoCapConfigSetParameters() In"));

    PvmiKvp* paramkvp;
    int32 numparam;
    PvmiKvp** retkvp;
    paramkvp = (PvmiKvp*)(aCmd.GetParam(0).pOsclAny_value);
    numparam = aCmd.GetParam(1).int32_value;
    retkvp = (PvmiKvp**)(aCmd.GetParam(2).pOsclAny_value);

    if (paramkvp == NULL || retkvp == NULL || numparam < 1)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoCapConfigSetParameters() Passed in parameter invalid"));
        return PVMFErrArgument;
    }

    // Go through each parameter
    for (int32 paramind = 0; paramind < numparam; ++paramind)
    {
        if (iRollOverState == RollOverStateIdle)
        {
            PVMFStatus ret = VerifyAndSaveKVPValues(&paramkvp[paramind]);
            if (ret != PVMFSuccess)
            {
                return ret;
            };
        }
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
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoCapConfigSetParameters() Unsupported key"));
            return PVMFErrArgument;
        }

        // Retrieve the second component from the key string
        pv_mime_string_extract_type(1, paramkvp[paramind].key, compstr);

        // First check if it is key string for engine ("player")
        if (pv_mime_strcmp(compstr, _STRLIT_CHAR("player")) >= 0)
        {
            if (compcount == 3)
            {
                // Verify and set the passed-in player setting
                PVMFStatus retval = DoVerifyAndSetPlayerParameter(paramkvp[paramind], true);
                if (retval != PVMFSuccess)
                {
                    *retkvp = &paramkvp[paramind];
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoCapConfigSetParameters() Setting parameter %d failed", paramind));
                    return retval;
                }
            }
            else if (compcount == 4)
            {
                // Only product info keys have four components
                // Verify and set the passed-in product info setting
                PVMFStatus retval = DoVerifyAndSetPlayerProductInfoParameter(paramkvp[paramind], true);
                if (retval != PVMFSuccess)
                {
                    *retkvp = &paramkvp[paramind];
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoCapConfigSetParameters() Setting parameter %d failed", paramind));
                    return retval;
                }
            }
            else
            {
                // Do not support more than 4 components right now
                *retkvp = &paramkvp[paramind];
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoCapConfigSetParameters() Unsupported key"));
                return PVMFErrArgument;
            }
        }
        else
        {
            *retkvp = &paramkvp[paramind];
            bool anysuccess = false;


            if (iSourceNodeCapConfigIF != NULL)
            {
                *retkvp = NULL;
                iSourceNodeCapConfigIF->setParametersSync(NULL, &paramkvp[paramind], 1, *retkvp);
                if (*retkvp == NULL)
                {
                    anysuccess = true;
                }
            }
            // Go through each datapath's cap-config IF in the datapath list and check for the string.
            for (uint32 i = 0; i < iDatapathList.size(); i++)
            {
                if (iDatapathList[i].iDecNodeCapConfigIF != NULL)
                {
                    *retkvp = NULL;
                    iDatapathList[i].iDecNodeCapConfigIF->setParametersSync(NULL, &paramkvp[paramind], 1, *retkvp);
                    if (*retkvp == NULL)
                    {
                        anysuccess = true;
                        break;
                    }
                }

                if (iDatapathList[i].iSinkNodeCapConfigIF != NULL)
                {
                    *retkvp = NULL;
                    iDatapathList[i].iSinkNodeCapConfigIF->setParametersSync(NULL, &paramkvp[paramind], 1, *retkvp);
                    if (*retkvp == NULL)
                    {
                        anysuccess = true;
                        break;
                    }
                }
            }

            if (anysuccess == false)
            {
                // setParametersSync was not accepted by the node(s)
                *retkvp = &paramkvp[paramind];
                return PVMFErrArgument;
            }
        }

    }

    if (!aSyncCmd)
    {
        EngineCommandCompleted(aCmd.GetCmdId(), aCmd.GetContext(), PVMFSuccess);
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoCapConfigSetParameters() Out"));
    return PVMFSuccess;
}


PVMFStatus PVPlayerEngine::DoCapConfigVerifyParameters(PvmiKvp* aParameters, int aNumElements)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoCapConfigVerifyParameters() In"));

    if (aParameters == NULL || aNumElements < 1)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoCapConfigVerifyParameters() Passed in parameter invalid"));
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

        if ((pv_mime_strcmp(compstr, _STRLIT_CHAR("x-pvmf")) < 0) || compcount < 2)
        {
            // First component should be "x-pvmf" and there must
            // be at least two components to go past x-pvmf
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoCapConfigVerifyParameters() Unsupported key"));
            return PVMFErrArgument;
        }

        // Retrieve the second component from the key string
        pv_mime_string_extract_type(1, aParameters[paramind].key, compstr);

        // First check if it is key string for engine ("player")
        if (pv_mime_strcmp(compstr, _STRLIT_CHAR("player")) >= 0)
        {
            if (compcount == 3)
            {
                // Verify the passed-in player setting
                PVMFStatus retval = DoVerifyAndSetPlayerParameter(aParameters[paramind], false);
                if (retval != PVMFSuccess)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoCapConfigVerifyParameters() Verifying parameter %d failed", paramind));
                    return retval;
                }
            }
            else if (compcount == 4)
            {
                // Only product info keys have four components
                // Verify the passed-in product info setting
                PVMFStatus retval = DoVerifyAndSetPlayerProductInfoParameter(aParameters[paramind], false);
                if (retval != PVMFSuccess)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoCapConfigVerifyParameters() Verifying parameter %d failed", paramind));
                    return retval;
                }
            }
            else
            {
                // Do not support more than 4 components right now
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoCapConfigVerifyParameters() Unsupported key"));
                return PVMFErrArgument;
            }
        }
        else
        {
            PVMFStatus retval = PVMFFailure;

            // Go through each datapath's cap-config IF in the datapath list and check for the string.
            for (uint32 i = 0; i < iDatapathList.size(); i++)
            {
                if (iDatapathList[i].iDecNodeCapConfigIF != NULL)
                {
                    retval = iDatapathList[i].iDecNodeCapConfigIF->verifyParametersSync(NULL, &aParameters[paramind], 1);
                    if (retval == PVMFSuccess)
                    {
                        // Key matched break the loop.
                        break;
                    }
                }

                if (iDatapathList[i].iSinkNodeCapConfigIF != NULL)
                {
                    retval = iDatapathList[i].iSinkNodeCapConfigIF->verifyParametersSync(NULL, &aParameters[paramind], 1);
                    if (retval == PVMFSuccess)
                    {
                        // Key matched break the loop.
                        break;
                    }
                }
            }

            if (retval != PVMFSuccess)
            {
                return retval;
            }
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoCapConfigVerifyParameters() Out"));
    return PVMFSuccess;
}

PVMFStatus PVPlayerEngine::DoGetPlayerParameter(PvmiKvp*& aParameters, int& aNumParamElements, int32 aIndex, PvmiKvpAttr reqattr)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoGetPlayerParameter() In"));

    aNumParamElements = 0;

    // Allocate memory for the KVP
    aParameters = (PvmiKvp*)oscl_malloc(sizeof(PvmiKvp));
    if (aParameters == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoGetPlayerParameter() Memory allocation for KVP failed"));
        return PVMFErrNoMemory;
    }
    oscl_memset(aParameters, 0, sizeof(PvmiKvp));
    // Allocate memory for the key string in KVP
    PvmiKeyType memblock = (PvmiKeyType)oscl_malloc(PVPLAYERCONFIG_KEYSTRING_SIZE * sizeof(char));
    if (memblock == NULL)
    {
        oscl_free(aParameters);
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoGetPlayerParameter() Memory allocation for key string failed"));
        return PVMFErrNoMemory;
    }
    oscl_strset(memblock, 0, PVPLAYERCONFIG_KEYSTRING_SIZE*sizeof(char));
    // Assign the key string buffer to KVP
    aParameters[0].key = memblock;

    // Copy the key string
    oscl_strncat(aParameters[0].key, _STRLIT_CHAR("x-pvmf/player/"), 14);
    oscl_strncat(aParameters[0].key, PVPlayerConfigBaseKeys[aIndex].iString, oscl_strlen(PVPlayerConfigBaseKeys[aIndex].iString));
    oscl_strncat(aParameters[0].key, _STRLIT_CHAR(";type=value;valtype="), 20);
    switch (PVPlayerConfigBaseKeys[aIndex].iValueType)
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
    aParameters[0].key[PVPLAYERCONFIG_KEYSTRING_SIZE-1] = 0;

    // Copy the requested info
    switch (aIndex)
    {
        case PBPOS_UNITS: // "pbpops_units"
            if (reqattr == PVMI_KVPATTR_CUR)
            {
                // Return current value
                // Allocate memory for the string
                char* curstr = (char*)oscl_malloc(32 * sizeof(char));
                if (curstr == NULL)
                {
                    oscl_free(aParameters[0].key);
                    oscl_free(aParameters);
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoGetPlayerParameter() Memory allocation for char* string failed"));
                    return PVMFErrNoMemory;
                }
                oscl_strset(curstr, 0, 32);
                // Copy the appropriate string based on units being used
                switch (iPBPosStatusUnit)
                {
                    case PVPPBPOSUNIT_SEC:
                        oscl_strncpy(curstr, _STRLIT_CHAR("PVPPBPOSUNIT_SEC"), 16);
                        aParameters[0].length = 16;
                        break;

                    case PVPPBPOSUNIT_MIN:
                        oscl_strncpy(curstr, _STRLIT_CHAR("PVPPBPOSUNIT_MIN"), 16);
                        aParameters[0].length = 16;
                        break;

                    case PVPPBPOSUNIT_MILLISEC:
                    default:
                        oscl_strncpy(curstr, _STRLIT_CHAR("PVPPBPOSUNIT_MILLISEC"), 21);
                        aParameters[0].length = 21;
                        break;
                }

                aParameters[0].value.pChar_value = curstr;
                aParameters[0].capacity = 32;
            }
            else if (reqattr == PVMI_KVPATTR_DEF)
            {
                // Return default
                // Allocate memory for the string
                int32 defstrlen = oscl_strlen(PVPLAYERENGINE_CONFIG_PBPOSSTATUSUNIT_DEF_STRING);
                char* defstr = (char*)oscl_malloc((defstrlen + 1) * sizeof(char));
                if (defstr == NULL)
                {
                    oscl_free(aParameters[0].key);
                    oscl_free(aParameters);
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoGetPlayerParameter() Memory allocation for char* string failed"));
                    return PVMFErrNoMemory;
                }
                // Copy and set
                oscl_strncpy(defstr, _STRLIT_CHAR(PVPLAYERENGINE_CONFIG_PBPOSSTATUSUNIT_DEF_STRING), defstrlen);
                defstr[defstrlen] = 0;
                aParameters[0].value.pChar_value = defstr;
                aParameters[0].capacity = defstrlen + 1;
                aParameters[0].length = defstrlen;
            }
            else
            {
                // Return capability
                // Allocate memory for the string
                int32 capstrlen = oscl_strlen(PVPLAYERENGINE_CONFIG_PBPOSSTATUSINTERVAL_CAP_STRING);
                char* capstr = (char*)oscl_malloc((capstrlen + 1) * sizeof(char));
                if (capstr == NULL)
                {
                    oscl_free(aParameters[0].key);
                    oscl_free(aParameters);
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoGetPlayerParameter() Memory allocation for char* string failed"));
                    return PVMFErrNoMemory;
                }
                // Copy and set
                oscl_strncpy(capstr, _STRLIT_CHAR(PVPLAYERENGINE_CONFIG_PBPOSSTATUSINTERVAL_CAP_STRING), capstrlen);
                capstr[capstrlen] = 0;
                aParameters[0].value.pChar_value = capstr;
                aParameters[0].capacity = capstrlen + 1;
                aParameters[0].length = capstrlen;
            }
            break;

        case PBPOS_INTERVAL:	// "pbpos_interval"
            if (reqattr == PVMI_KVPATTR_CUR)
            {
                // Return current value
                aParameters[0].value.uint32_value = iPBPosStatusInterval;
            }
            else if (reqattr == PVMI_KVPATTR_DEF)
            {
                // Return default
                aParameters[0].value.uint32_value = PVPLAYERENGINE_CONFIG_PBPOSSTATUSINTERVAL_DEF;
            }
            else
            {
                // Return capability
                range_uint32* rui32 = (range_uint32*)oscl_malloc(sizeof(range_uint32));
                if (rui32 == NULL)
                {
                    oscl_free(aParameters[0].key);
                    oscl_free(aParameters);
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoGetPlayerParameter() Memory allocation for range uint32 failed"));
                    return PVMFErrNoMemory;
                }
                rui32->min = PVPLAYERENGINE_CONFIG_PBPOSSTATUSINTERVAL_MIN;
                rui32->max = PVPLAYERENGINE_CONFIG_PBPOSSTATUSINTERVAL_MAX;
                aParameters[0].value.key_specific_value = (void*)rui32;
            }
            break;

        case ENDTIMECHECK_INTERVAL:	// "endtimecheck_interval"
            if (reqattr == PVMI_KVPATTR_CUR)
            {
                // Return current value
                aParameters[0].value.uint32_value = iEndTimeCheckInterval;
            }
            else if (reqattr == PVMI_KVPATTR_DEF)
            {
                // Return default
                aParameters[0].value.uint32_value = PVPLAYERENGINE_CONFIG_ENDTIMECHECKINTERVAL_DEF;
            }
            else
            {
                // Return capability
                range_uint32* rui32 = (range_uint32*)oscl_malloc(sizeof(range_uint32));
                if (rui32 == NULL)
                {
                    oscl_free(aParameters[0].key);
                    oscl_free(aParameters);
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoGetPlayerParameter() Memory allocation for range uint32 failed"));
                    return PVMFErrNoMemory;
                }
                rui32->min = PVPLAYERENGINE_CONFIG_ENDTIMECHECKINTERVAL_MIN;
                rui32->max = PVPLAYERENGINE_CONFIG_ENDTIMECHECKINTERVAL_MAX;
                aParameters[0].value.key_specific_value = (void*)rui32;
            }
            break;

        case SEEKTOSYNCPOINT:	// "seektosyncpoint"
            if (reqattr == PVMI_KVPATTR_CUR)
            {
                // Return current value
                aParameters[0].value.bool_value = iSeekToSyncPoint;
            }
            else if (reqattr == PVMI_KVPATTR_DEF)
            {
                // Return default
                aParameters[0].value.bool_value = PVPLAYERENGINE_CONFIG_SEEKTOSYNCPOINT_DEF;
            }
            else
            {
                // Return capability
                // Bool so no capability
            }
            break;

        case SKIPTOREQUESTEDPOSITION:	// "skiptorequestedpos"
            if (reqattr == PVMI_KVPATTR_CUR)
            {
                // Return current value
                aParameters[0].value.bool_value = iSkipToRequestedPosition;
            }
            else if (reqattr == PVMI_KVPATTR_DEF)
            {
                // Return default
                aParameters[0].value.bool_value = PVPLAYERENGINE_CONFIG_SKIPTOREQUESTEDPOS_DEF;
            }
            else
            {
                // Return capability
                // Bool so no capability
            }
            break;

        case SYNCPOINTSEEKWINDOW:	// "syncpointseekwindow"
            if (reqattr == PVMI_KVPATTR_CUR)
            {
                // Return current value
                aParameters[0].value.uint32_value = iSyncPointSeekWindow;
            }
            else if (reqattr == PVMI_KVPATTR_DEF)
            {
                // Return default
                aParameters[0].value.uint32_value = PVPLAYERENGINE_CONFIG_SEEKTOSYNCPOINTWINDOW_DEF;
            }
            else
            {
                // Return capability
                range_uint32* rui32 = (range_uint32*)oscl_malloc(sizeof(range_uint32));
                if (rui32 == NULL)
                {
                    oscl_free(aParameters[0].key);
                    oscl_free(aParameters);
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoGetPlayerParameter() Memory allocation for range uint32 failed"));
                    return PVMFErrNoMemory;
                }
                rui32->min = PVPLAYERENGINE_CONFIG_SEEKTOSYNCPOINTWINDOW_MIN;
                rui32->max = PVPLAYERENGINE_CONFIG_SEEKTOSYNCPOINTWINDOW_MAX;
                aParameters[0].value.key_specific_value = (void*)rui32;
            }
            break;

        case SYNCMARGIN_VIDEO:	// "syncmargin_video"
        case SYNCMARGIN_AUDIO:	// "syncmargin_audio"
        case SYNCMARGIN_TEXT:	// "syncmargin_text"
        {
            range_int32* ri32 = (range_int32*)oscl_malloc(sizeof(range_int32));
            if (ri32 == NULL)
            {
                oscl_free(aParameters[0].key);
                oscl_free(aParameters);
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoGetPlayerParameter() Memory allocation for range int32 failed"));
                return PVMFErrNoMemory;
            }

            if (reqattr == PVMI_KVPATTR_CUR)
            {
                // Return current value
                if (aIndex == SYNCMARGIN_VIDEO)
                {
                    // Video
                    ri32->min = iSyncMarginVideo.min;
                    ri32->max = iSyncMarginVideo.max;
                }
                else if (aIndex == SYNCMARGIN_AUDIO)
                {
                    // Audio
                    ri32->min = iSyncMarginAudio.min;
                    ri32->max = iSyncMarginAudio.max;
                }
                else
                {
                    // Text
                    ri32->min = iSyncMarginText.min;
                    ri32->max = iSyncMarginText.max;
                }
                aParameters[0].value.key_specific_value = (void*)ri32;
            }
            else if (reqattr == PVMI_KVPATTR_DEF)
            {
                // Return default
                ri32->min = PVPLAYERENGINE_CONFIG_SYNCMARGIN_EARLY_DEF;
                ri32->max = PVPLAYERENGINE_CONFIG_SYNCMARGIN_LATE_DEF;
                aParameters[0].value.key_specific_value = (void*)ri32;
            }
            else
            {
                // Return capability
                ri32->min = PVPLAYERENGINE_CONFIG_SYNCMARGIN_MIN;
                ri32->max = PVPLAYERENGINE_CONFIG_SYNCMARGIN_MAX;
                aParameters[0].value.key_specific_value = (void*)ri32;
            }
        }
        break;

        case NODECMD_TIMEOUT:	// "nodecmd_timeout"
            if (reqattr == PVMI_KVPATTR_CUR)
            {
                // Return current value
                aParameters[0].value.uint32_value = iNodeCmdTimeout;
            }
            else if (reqattr == PVMI_KVPATTR_DEF)
            {
                // Return default
                aParameters[0].value.uint32_value = PVPLAYERENGINE_CONFIG_NODECMDTIMEOUT_DEF;
            }
            else
            {
                // Return capability
                range_uint32* rui32 = (range_uint32*)oscl_malloc(sizeof(range_uint32));
                if (rui32 == NULL)
                {
                    oscl_free(aParameters[0].key);
                    oscl_free(aParameters);
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoGetPlayerParameter() Memory allocation for range uint32 failed"));
                    return PVMFErrNoMemory;
                }
                rui32->min = PVPLAYERENGINE_CONFIG_NODECMDTIMEOUT_MIN;
                rui32->max = PVPLAYERENGINE_CONFIG_NODECMDTIMEOUT_MAX;
                aParameters[0].value.key_specific_value = (void*)rui32;
            }
            break;

        case NODEDATAQUEIUING_TIMEOUT:	// "nodedataqueuing_timeout"
            if (reqattr == PVMI_KVPATTR_CUR)
            {
                // Return current value
                aParameters[0].value.uint32_value = iNodeDataQueuingTimeout;
            }
            else if (reqattr == PVMI_KVPATTR_DEF)
            {
                // Return default
                aParameters[0].value.uint32_value = PVPLAYERENGINE_CONFIG_NODEDATAQUEUINGTIMEOUT_DEF;
            }
            else
            {
                // Return capability
                range_uint32* rui32 = (range_uint32*)oscl_malloc(sizeof(range_uint32));
                if (rui32 == NULL)
                {
                    oscl_free(aParameters[0].key);
                    oscl_free(aParameters);
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoGetPlayerParameter() Memory allocation for range uint32"));
                    return PVMFErrNoMemory;
                }
                rui32->min = PVPLAYERENGINE_CONFIG_NODEDATAQUEUINGTIMEOUT_MIN;
                rui32->max = PVPLAYERENGINE_CONFIG_NODEDATAQUEUINGTIMEOUT_MAX;
                aParameters[0].value.key_specific_value = (void*)rui32;
            }
            break;

        case PBPOS_ENABLE:	// "pbpos_enable"
            if (reqattr == PVMI_KVPATTR_CUR)
            {
                // Return current value
                aParameters[0].value.bool_value = iPBPosEnable;
            }
            else if (reqattr == PVMI_KVPATTR_DEF)
            {
                // Return default
                aParameters[0].value.bool_value = true;
            }
            else
            {
                // Return capability
                // Bool so no capability
            }
            break;
        default:
            // Invalid index
            oscl_free(aParameters[0].key);
            oscl_free(aParameters);
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoGetPlayerParameter() Invalid index to player parameter"));
            return PVMFErrArgument;
    }

    aNumParamElements = 1;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoGetPlayerParameter() Out"));
    return PVMFSuccess;
}


PVMFStatus PVPlayerEngine::DoGetPlayerProductInfoParameter(PvmiKvp*& aParameters, int& aNumParamElements, int32 aIndex, PvmiKvpAttr reqattr)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoGetPlayerProductInfoParameter() In"));

    aNumParamElements = 0;

    // Allocate memory for the KVP
    aParameters = (PvmiKvp*)oscl_malloc(sizeof(PvmiKvp));
    if (aParameters == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoGetPlayerProductInfoParameter() Memory allocation for KVP failed"));
        return PVMFErrNoMemory;
    }
    oscl_memset(aParameters, 0, sizeof(PvmiKvp));
    // Allocate memory for the key string in KVP
    PvmiKeyType memblock = (PvmiKeyType)oscl_malloc(PVPLAYERCONFIG_KEYSTRING_SIZE * sizeof(char));
    if (memblock == NULL)
    {
        oscl_free(aParameters);
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoGetPlayerProductInfoParameter() Memory allocation for key string"));
        return PVMFErrNoMemory;
    }
    oscl_strset(memblock, 0, PVPLAYERCONFIG_KEYSTRING_SIZE*sizeof(char));
    // Assign the key string buffer to KVP
    aParameters[0].key = memblock;

    // Copy the key string
    oscl_strncat(aParameters[0].key, _STRLIT_CHAR("x-pvmf/player/productinfo/"), 26);
    oscl_strncat(aParameters[0].key, PVPlayerConfigProdInfoKeys[aIndex].iString, oscl_strlen(PVPlayerConfigProdInfoKeys[aIndex].iString));
    oscl_strncat(aParameters[0].key, _STRLIT_CHAR(";type=value;valtype=char*"), 25);
    aParameters[0].key[PVPLAYERCONFIG_KEYSTRING_SIZE-1] = 0;

    // Copy the requested info
    switch (aIndex)
    {
        case PRODUCTNAME: // "productname"
            if (reqattr == PVMI_KVPATTR_CUR)
            {
                // Return current value
                // Allocate memory for the string
                char* curstr = (char*)oscl_malloc(iProdInfoProdName.get_size() + 1);
                if (curstr == NULL)
                {
                    oscl_free(aParameters[0].key);
                    oscl_free(aParameters);
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoGetPlayerProductInfoParameter() Memory allocation for char* string failed"));
                    return PVMFErrNoMemory;
                }
                oscl_strset(curstr, 0, iProdInfoProdName.get_size() + 1);
                // Copy and set
                oscl_strncpy(curstr, iProdInfoProdName.get_cstr(), iProdInfoProdName.get_size());
                aParameters[0].value.pChar_value = curstr;
                aParameters[0].length = iProdInfoProdName.get_size();
                aParameters[0].capacity = iProdInfoProdName.get_size() + 1;
            }
            else if (reqattr == PVMI_KVPATTR_DEF)
            {
                // Return default
                // Allocate memory for the string
                int32 defstrlen = oscl_strlen(PVPLAYERENGINE_PRODINFO_PRODNAME_STRING);
                char* defstr = (char*)oscl_malloc((defstrlen + 1) * sizeof(char));
                if (defstr == NULL)
                {
                    oscl_free(aParameters[0].key);
                    oscl_free(aParameters);
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoGetPlayerProductInfoParameter() Memory allocation for char* string failed"));
                    return PVMFErrNoMemory;
                }
                // Copy and set
                oscl_strncpy(defstr, _STRLIT_CHAR(PVPLAYERENGINE_PRODINFO_PRODNAME_STRING), defstrlen);
                defstr[defstrlen] = 0;
                aParameters[0].value.pChar_value = defstr;
                aParameters[0].capacity = defstrlen + 1;
                aParameters[0].length = defstrlen;
            }
            else
            {
                // Return capability
                // Empty string
                aParameters[0].value.pChar_value = NULL;
                aParameters[0].capacity = 0;
                aParameters[0].length = 0;
            }
            break;

        case PARTNUMBER: // "partnumber"
            if (reqattr == PVMI_KVPATTR_CUR)
            {
                // Return current value
                // Allocate memory for the string
                char* curstr = (char*)oscl_malloc(iProdInfoPartNum.get_size() + 1);
                if (curstr == NULL)
                {
                    oscl_free(aParameters[0].key);
                    oscl_free(aParameters);
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoGetPlayerProductInfoParameter() Memory allocation for char* string failed"));
                    return PVMFErrNoMemory;
                }
                oscl_strset(curstr, 0, iProdInfoPartNum.get_size() + 1);
                // Copy and set
                oscl_strncpy(curstr, iProdInfoPartNum.get_cstr(), iProdInfoPartNum.get_size());
                aParameters[0].value.pChar_value = curstr;
                aParameters[0].length = iProdInfoPartNum.get_size();
                aParameters[0].capacity = iProdInfoPartNum.get_size() + 1;
            }
            else if (reqattr == PVMI_KVPATTR_DEF)
            {
                // Return default
                // Allocate memory for the string
                int32 defstrlen = oscl_strlen(PVPLAYERENGINE_PRODINFO_PARTNUM_STRING);
                char* defstr = (char*)oscl_malloc((defstrlen + 1) * sizeof(char));
                if (defstr == NULL)
                {
                    oscl_free(aParameters[0].key);
                    oscl_free(aParameters);
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoGetPlayerProductInfoParameter() Memory allocation for char* string failed"));
                    return PVMFErrNoMemory;
                }
                // Copy and set
                oscl_strncpy(defstr, _STRLIT_CHAR(PVPLAYERENGINE_PRODINFO_PARTNUM_STRING), defstrlen);
                defstr[defstrlen] = 0;
                aParameters[0].value.pChar_value = defstr;
                aParameters[0].capacity = defstrlen + 1;
                aParameters[0].length = defstrlen;
            }
            else
            {
                // Return capability
                // Empty string
                aParameters[0].value.pChar_value = NULL;
                aParameters[0].capacity = 0;
                aParameters[0].length = 0;
            }
            break;

        case HARDWAREPLATFORM: // "hardwareplatform"
            if (reqattr == PVMI_KVPATTR_CUR)
            {
                // Return current value
                // Allocate memory for the string
                char* curstr = (char*)oscl_malloc(iProdInfoHWPlatform.get_size() + 1);
                if (curstr == NULL)
                {
                    oscl_free(aParameters[0].key);
                    oscl_free(aParameters);
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoGetPlayerProductInfoParameter() Memory allocation for char* string failed"));
                    return PVMFErrNoMemory;
                }
                oscl_strset(curstr, 0, iProdInfoHWPlatform.get_size() + 1);
                // Copy and set
                oscl_strncpy(curstr, iProdInfoHWPlatform.get_cstr(), iProdInfoHWPlatform.get_size());
                aParameters[0].value.pChar_value = curstr;
                aParameters[0].length = iProdInfoHWPlatform.get_size();
                aParameters[0].capacity = iProdInfoHWPlatform.get_size() + 1;
            }
            else if (reqattr == PVMI_KVPATTR_DEF)
            {
                // Return default
                // Allocate memory for the string
                int32 defstrlen = oscl_strlen(PVPLAYERENGINE_PRODINFO_HWPLATFORM_STRING);
                char* defstr = (char*)oscl_malloc((defstrlen + 1) * sizeof(char));
                if (defstr == NULL)
                {
                    oscl_free(aParameters[0].key);
                    oscl_free(aParameters);
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoGetPlayerProductInfoParameter() Memory allocation for char* string failed"));
                    return PVMFErrNoMemory;
                }
                // Copy and set
                oscl_strncpy(defstr, _STRLIT_CHAR(PVPLAYERENGINE_PRODINFO_HWPLATFORM_STRING), defstrlen);
                defstr[defstrlen] = 0;
                aParameters[0].value.pChar_value = defstr;
                aParameters[0].capacity = defstrlen + 1;
                aParameters[0].length = defstrlen;
            }
            else
            {
                // Return capability
                // Empty string
                aParameters[0].value.pChar_value = NULL;
                aParameters[0].capacity = 0;
                aParameters[0].length = 0;
            }
            break;

        case SOFTWAREPLATFORM: // "softwareplatform"
            if (reqattr == PVMI_KVPATTR_CUR)
            {
                // Return current value
                // Allocate memory for the string
                char* curstr = (char*)oscl_malloc(iProdInfoSWPlatform.get_size() + 1);
                if (curstr == NULL)
                {
                    oscl_free(aParameters[0].key);
                    oscl_free(aParameters);
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoGetPlayerProductInfoParameter() Memory allocation for char* string failed"));
                    return PVMFErrNoMemory;
                }
                oscl_strset(curstr, 0, iProdInfoSWPlatform.get_size() + 1);
                // Copy and set
                oscl_strncpy(curstr, iProdInfoSWPlatform.get_cstr(), iProdInfoSWPlatform.get_size());
                aParameters[0].value.pChar_value = curstr;
                aParameters[0].length = iProdInfoSWPlatform.get_size();
                aParameters[0].capacity = iProdInfoSWPlatform.get_size() + 1;
            }
            else if (reqattr == PVMI_KVPATTR_DEF)
            {
                // Return default
                // Allocate memory for the string
                int32 defstrlen = oscl_strlen(PVPLAYERENGINE_PRODINFO_SWPLATFORM_STRING);
                char* defstr = (char*)oscl_malloc((defstrlen + 1) * sizeof(char));
                if (defstr == NULL)
                {
                    oscl_free(aParameters[0].key);
                    oscl_free(aParameters);
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoGetPlayerProductInfoParameter() Memory allocation for char* string failed"));
                    return PVMFErrNoMemory;
                }
                // Copy and set
                oscl_strncpy(defstr, _STRLIT_CHAR(PVPLAYERENGINE_PRODINFO_SWPLATFORM_STRING), defstrlen);
                defstr[defstrlen] = 0;
                aParameters[0].value.pChar_value = defstr;
                aParameters[0].capacity = defstrlen + 1;
                aParameters[0].length = defstrlen;
            }
            else
            {
                // Return capability
                // Empty string
                aParameters[0].value.pChar_value = NULL;
                aParameters[0].capacity = 0;
                aParameters[0].length = 0;
            }
            break;

        case DEVICE: // "device"
            if (reqattr == PVMI_KVPATTR_CUR)
            {
                // Return current value
                // Allocate memory for the string
                char* curstr = (char*)oscl_malloc(iProdInfoDevice.get_size() + 1);
                if (curstr == NULL)
                {
                    oscl_free(aParameters[0].key);
                    oscl_free(aParameters);
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoGetPlayerProductInfoParameter() Memory allocation for char* string failed"));
                    return PVMFErrNoMemory;
                }
                oscl_strset(curstr, 0, iProdInfoDevice.get_size() + 1);
                // Copy and set
                oscl_strncpy(curstr, iProdInfoPartNum.get_cstr(), iProdInfoDevice.get_size());
                aParameters[0].value.pChar_value = curstr;
                aParameters[0].length = iProdInfoDevice.get_size();
                aParameters[0].capacity = iProdInfoDevice.get_size() + 1;
            }
            else if (reqattr == PVMI_KVPATTR_DEF)
            {
                // Return default
                // Allocate memory for the string
                int32 defstrlen = oscl_strlen(PVPLAYERENGINE_PRODINFO_DEVICE_STRING);
                char* defstr = (char*)oscl_malloc((defstrlen + 1) * sizeof(char));
                if (defstr == NULL)
                {
                    oscl_free(aParameters[0].key);
                    oscl_free(aParameters);
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoGetPlayerProductInfoParameter() Memory allocation for char* string failed"));
                    return PVMFErrNoMemory;
                }
                // Copy and set
                oscl_strncpy(defstr, _STRLIT_CHAR(PVPLAYERENGINE_PRODINFO_DEVICE_STRING), defstrlen);
                defstr[defstrlen] = 0;
                aParameters[0].value.pChar_value = defstr;
                aParameters[0].capacity = defstrlen + 1;
                aParameters[0].length = defstrlen;
            }
            else
            {
                // Return capability
                // Empty string
                aParameters[0].value.pChar_value = NULL;
                aParameters[0].capacity = 0;
                aParameters[0].length = 0;
            }
            break;

        default:
            // Invalid index
            oscl_free(aParameters[0].key);
            oscl_free(aParameters);
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoGetPlayerProductInfoParameter() Invalid index for product info"));
            return PVMFErrArgument;
    }

    aNumParamElements = 1;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoGetPlayerProductInfoParameter() Out"));
    return PVMFSuccess;
}


PVMFStatus PVPlayerEngine::DoVerifyAndSetPlayerParameter(PvmiKvp& aParameter, bool aSetParam)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoVerifyAndSetPlayerParameter() In"));

    // Determine the valtype
    PvmiKvpValueType keyvaltype = GetValTypeFromKeyString(aParameter.key);
    if (keyvaltype == PVMI_KVPVALTYPE_UNKNOWN)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoVerifyAndSetPlayerParameter() Valtype in key string unknown"));
        return PVMFErrArgument;
    }
    // Retrieve the third component from the key string
    char* compstr = NULL;
    pv_mime_string_extract_type(2, aParameter.key, compstr);

    int32 engcomp3ind = 0;
    for (engcomp3ind = 0; engcomp3ind < PVPLAYERCONFIG_BASE_NUMKEYS; ++engcomp3ind)
    {
        // Go through each engine component string at 3rd level
        if (pv_mime_strcmp(compstr, (char*)(PVPlayerConfigBaseKeys[engcomp3ind].iString)) >= 0)
        {
            // Break out of the for loop
            break;
        }
    }

    if (engcomp3ind >= PVPLAYERCONFIG_BASE_NUMKEYS || engcomp3ind == PRODUCTINFO)
    {
        // Match couldn't be found or non-leaf node ("productinfo") specified
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoVerifyAndSetPlayerParameter() Unsupported key or non-leaf node"));
        return PVMFErrArgument;
    }

    // Verify the valtype
    if (keyvaltype != PVPlayerConfigBaseKeys[engcomp3ind].iValueType)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoVerifyAndSetPlayerParameter() Valtype does not match for key"));
        return PVMFErrArgument;
    }

    switch (engcomp3ind)
    {
        case PBPOS_UNITS: // "pbpos_units"
        {
            // Validate
            if (aParameter.value.pChar_value == NULL)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoVerifyAndSetPlayerParameter() char* string for pbpos_units is NULL"));
                return PVMFErrArgument;
            }

            // Check the specified unit
            // Use sample number as the invalid default since it is not allowed
            PVPPlaybackPositionUnit newposunit = PVPPBPOSUNIT_UNKNOWN;
            if (oscl_strncmp(aParameter.value.pChar_value, _STRLIT_CHAR("PVPPBPOSUNIT_SEC"), 16) == 0)
            {
                newposunit = PVPPBPOSUNIT_SEC;
            }
            else if (oscl_strncmp(aParameter.value.pChar_value, _STRLIT_CHAR("PVPPBPOSUNIT_MIN"), 16) == 0)
            {
                newposunit = PVPPBPOSUNIT_MIN;
            }
            else if (oscl_strncmp(aParameter.value.pChar_value, _STRLIT_CHAR("PVPPBPOSUNIT_MILLISEC"), 21) == 0)
            {
                newposunit = PVPPBPOSUNIT_MILLISEC;
            }

            if (newposunit == PVPPBPOSUNIT_UNKNOWN)
            {
                // Couldn't determine the new units
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoVerifyAndSetPlayerParameter() Invalid units for pbpos_units"));
                return PVMFErrArgument;
            }
            // Change the config if to set
            if (aSetParam)
            {
                iPBPosStatusUnit = newposunit;
            }
        }
        break;

        case PBPOS_INTERVAL: // "pbpos_interval"
            // Check if within range
            if (aParameter.value.uint32_value < PVPLAYERENGINE_CONFIG_PBPOSSTATUSINTERVAL_MIN ||
                    aParameter.value.uint32_value > PVPLAYERENGINE_CONFIG_PBPOSSTATUSINTERVAL_MAX)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoVerifyAndSetPlayerParameter() Invalid value for pbpos_interval"));
                return PVMFErrArgument;
            }
            // Change the config if to set
            if (aSetParam)
            {
                iPBPosStatusInterval = aParameter.value.uint32_value;
            }
            break;

        case ENDTIMECHECK_INTERVAL: // "endtimecheck_interval"
            // Check if within range
            if (aParameter.value.uint32_value < PVPLAYERENGINE_CONFIG_ENDTIMECHECKINTERVAL_MIN ||
                    aParameter.value.uint32_value > PVPLAYERENGINE_CONFIG_ENDTIMECHECKINTERVAL_MAX)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoVerifyAndSetPlayerParameter() Invalid value for endtimecheck_interval"));
                return PVMFErrArgument;
            }
            // Change the config if to set
            if (aSetParam)
            {
                iEndTimeCheckInterval = aParameter.value.uint32_value;
            }
            break;

        case SEEKTOSYNCPOINT: // "seektosyncpoint"
            // Nothing to validate since it is boolean
            // Change the config if to set
            if (aSetParam)
            {
                iSeekToSyncPoint = aParameter.value.bool_value;
            }
            break;

        case SKIPTOREQUESTEDPOSITION: // "skiptorequestedpos"
            // Nothing to validate since it is boolean
            // Change the config if to set
            if (aSetParam)
            {
                iSkipToRequestedPosition = aParameter.value.bool_value;
            }
            break;

        case SYNCPOINTSEEKWINDOW: // "syncpointseekwindow"
            // Check if within range
            if (aParameter.value.uint32_value > PVPLAYERENGINE_CONFIG_SEEKTOSYNCPOINTWINDOW_MAX)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoVerifyAndSetPlayerParameter() Invalid value for syncpointseekwindow"));
                return PVMFErrArgument;
            }
            // Change the config if to set
            if (aSetParam)
            {
                iSyncPointSeekWindow = aParameter.value.uint32_value;
            }
            break;

        case SYNCMARGIN_VIDEO: // "syncmargin_video"
        case SYNCMARGIN_AUDIO: // "syncmargin_audio"
        case SYNCMARGIN_TEXT: // "syncmargin_text"
        {
            range_int32* ri32 = (range_int32*)aParameter.value.key_specific_value;
            if (ri32 == NULL)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoVerifyAndSetPlayerParameter() ksv for syncmargin is NULL"));
                return PVMFErrArgument;
            }

            // Check if within range
            if (ri32->min < PVPLAYERENGINE_CONFIG_SYNCMARGIN_MIN ||
                    ri32->min > PVPLAYERENGINE_CONFIG_SYNCMARGIN_MAX ||
                    ri32->max < PVPLAYERENGINE_CONFIG_SYNCMARGIN_MIN ||
                    ri32->max > PVPLAYERENGINE_CONFIG_SYNCMARGIN_MAX)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoVerifyAndSetPlayerParameter() Invalid range for syncmargin"));
                return PVMFErrArgument;
            }

            // Change the config if to set
            if (aSetParam)
            {
                return DoSetConfigSyncMargin(ri32->min, ri32->max, engcomp3ind - 7);
            }
        }
        break;

        case NODECMD_TIMEOUT: // "nodecmd_timeout"
            // Check if within range
            if (aParameter.value.uint32_value < PVPLAYERENGINE_CONFIG_NODECMDTIMEOUT_MIN ||
                    aParameter.value.uint32_value > PVPLAYERENGINE_CONFIG_NODECMDTIMEOUT_MAX)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoVerifyAndSetPlayerParameter() Invalid value for ndoecmd_timeout"));
                return PVMFErrArgument;
            }
            // Change the config if to set
            if (aSetParam)
            {
                iNodeCmdTimeout = aParameter.value.uint32_value;
            }
            break;

        case NODEDATAQUEIUING_TIMEOUT: // "nodedataqueuing_timeout"
            // Check if within range
            if (aParameter.value.uint32_value < PVPLAYERENGINE_CONFIG_NODEDATAQUEUINGTIMEOUT_MIN ||
                    aParameter.value.uint32_value > PVPLAYERENGINE_CONFIG_NODEDATAQUEUINGTIMEOUT_MAX)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoVerifyAndSetPlayerParameter() Invalid value for nodedataqueuing_timeout"));
                return PVMFErrArgument;
            }
            // Change the config if to set
            if (aSetParam)
            {
                iNodeDataQueuingTimeout = aParameter.value.uint32_value;
            }
            break;

        case PBPOS_ENABLE: // "pbpos_enable"
            // Nothing to validate since it is boolean
            // Change the config if to set
            if (aSetParam)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "PVPlayerEngine::DoVerifyAndSetPlayerParameter() pbpos_enable set to %d", iPBPosEnable));
                bool prevPBPosEnable = iPBPosEnable;
                iPBPosEnable = aParameter.value.bool_value;
                if (prevPBPosEnable && !(aParameter.value.bool_value))
                {
                    // Stop playback position reporting
                    StopPlaybackStatusTimer();
                }
                else if (!prevPBPosEnable && (aParameter.value.bool_value))
                {
                    // Start playback position reporting only when playback clock is running
                    if (iPlaybackClock.GetState() == PVMFMediaClock::RUNNING)
                    {
                        StartPlaybackStatusTimer();
                    }
                }

            }
            break;

        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoVerifyAndSetPlayerParameter() Invalid index for player parameter"));
            return PVMFErrArgument;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoVerifyAndSetPlayerParameter() Out"));
    return PVMFSuccess;
}


PVMFStatus PVPlayerEngine::DoVerifyAndSetPlayerProductInfoParameter(PvmiKvp& aParameter, bool aSetParam)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoVerifyAndSetPlayerProductInfoParameter() In"));

    // Determine the valtype
    PvmiKvpValueType keyvaltype = GetValTypeFromKeyString(aParameter.key);
    if (keyvaltype == PVMI_KVPVALTYPE_UNKNOWN)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoVerifyAndSetPlayerProductInfoParameter() Valtype unknown"));
        return PVMFErrArgument;
    }
    // Retrieve the 4th component from the key string
    char* compstr = NULL;
    pv_mime_string_extract_type(3, aParameter.key, compstr);

    int32 engcomp4ind = 0;
    for (engcomp4ind = 0; engcomp4ind < PVPLAYERCONFIG_PRODINFO_NUMKEYS; ++engcomp4ind)
    {
        // Go through each engine component string at 4th level
        if (pv_mime_strcmp(compstr, (char*)(PVPlayerConfigProdInfoKeys[engcomp4ind].iString)) >= 0)
        {
            // Break out of the for loop
            break;
        }
    }

    if (engcomp4ind >= PVPLAYERCONFIG_PRODINFO_NUMKEYS)
    {
        // Match couldn't be found
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoVerifyAndSetPlayerProductInfoParameter() Unsupported key"));
        return PVMFErrArgument;
    }

    // Verify the valtype
    if (keyvaltype != PVPlayerConfigProdInfoKeys[engcomp4ind].iValueType)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoVerifyAndSetPlayerProductInfoParameter() Valtype does not match for key"));
        return PVMFErrArgument;
    }

    switch (engcomp4ind)
    {
        case PRODUCTNAME: // "productname"
            // Check if within range
            if (aParameter.value.pChar_value == NULL)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoVerifyAndSetPlayerProductInfoParameter() char* string for productname is NULL"));
                return PVMFErrArgument;
            }
            // Change the config if to set
            if (aSetParam)
            {
                iProdInfoProdName = aParameter.value.pChar_value;
            }
            break;

        case PARTNUMBER: // "partnumber"
            // Check if within range
            if (aParameter.value.pChar_value == NULL)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoVerifyAndSetPlayerProductInfoParameter() char* string for productname is NULL"));
                return PVMFErrArgument;
            }
            // Change the config if to set
            if (aSetParam)
            {
                iProdInfoPartNum = aParameter.value.pChar_value;
            }
            break;

        case HARDWAREPLATFORM: // "hardwareplatform"
            // Check if within range
            if (aParameter.value.pChar_value == NULL)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoVerifyAndSetPlayerProductInfoParameter() char* string for productname is NULL"));
                return PVMFErrArgument;
            }
            // Change the config if to set
            if (aSetParam)
            {
                iProdInfoHWPlatform = aParameter.value.pChar_value;
            }
            break;

        case SOFTWAREPLATFORM: // "softwareplatform"
            // Check if within range
            if (aParameter.value.pChar_value == NULL)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoVerifyAndSetPlayerProductInfoParameter() char* string for productname is NULL"));
                return PVMFErrArgument;
            }
            // Change the config if to set
            if (aSetParam)
            {
                iProdInfoSWPlatform = aParameter.value.pChar_value;
            }
            break;

        case DEVICE: // "device"
            // Check if within range
            if (aParameter.value.pChar_value == NULL)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoVerifyAndSetPlayerProductInfoParameter() char* string for productname is NULL"));
                return PVMFErrArgument;
            }
            // Change the config if to set
            if (aSetParam)
            {
                iProdInfoDevice = aParameter.value.pChar_value;
            }
            break;

        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoVerifyAndSetPlayerProductInfoParameter() Invalid index for product info"));
            return PVMFErrArgument;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoVerifyAndSetPlayerProductInfoParameter() Out"));
    return PVMFSuccess;
}


PVMFStatus PVPlayerEngine::DoSetConfigSyncMargin(int32 aEarlyMargin, int32 aLateMargin, int32 aMediaType)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSetConfigSyncMargin() In"));

    if (aMediaType == 0)
    {
        // Video
        iSyncMarginVideo.min = aEarlyMargin;
        iSyncMarginVideo.max = aLateMargin;

        // Find the video datapath in the list
        int32 vdpind = -1;
        if (FindDatapathForTrackUsingMimeString(true, false, false, vdpind) == true)
        {
            PVPlayerEngineDatapath* pvpedp = &(iDatapathList[vdpind]);
            if (pvpedp->iDatapath)
            {
                if (pvpedp->iSinkNodeSyncCtrlIF)
                {
                    pvpedp->iSinkNodeSyncCtrlIF->SetMargins((-1*iSyncMarginVideo.min), iSyncMarginVideo.max);
                }
            }
        }
    }
    else if (aMediaType == 1)
    {
        // Audio
        iSyncMarginAudio.min = aEarlyMargin;
        iSyncMarginAudio.max = aLateMargin;

        // Find the audio datapath in the list
        int32 adpind = -1;
        if (FindDatapathForTrackUsingMimeString(false, true, false, adpind) == true)
        {
            PVPlayerEngineDatapath* pvpedp = &(iDatapathList[adpind]);
            if (pvpedp->iDatapath)
            {
                if (pvpedp->iSinkNodeSyncCtrlIF)
                {
                    pvpedp->iSinkNodeSyncCtrlIF->SetMargins((-1*iSyncMarginAudio.min), iSyncMarginAudio.max);
                }
            }
        }
    }
    else if (aMediaType == 2)
    {
        // Text
        iSyncMarginText.min = aEarlyMargin;
        iSyncMarginText.max = aLateMargin;

        // Find the text datapath in the list
        int32 tdpind = -1;
        if (FindDatapathForTrackUsingMimeString(false, false, true, tdpind) == true)
        {
            PVPlayerEngineDatapath* pvpedp = &(iDatapathList[tdpind]);
            if (pvpedp->iDatapath)
            {
                if (pvpedp->iSinkNodeSyncCtrlIF)
                {
                    pvpedp->iSinkNodeSyncCtrlIF->SetMargins((-1*iSyncMarginText.min), iSyncMarginText.max);
                }
            }
        }
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoSetConfigSyncMargin() Invalid media type index"));
        return PVMFErrArgument;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSetConfigSyncMargin() Out"));
    return PVMFSuccess;
}


PVPlayerEngineContext* PVPlayerEngine::AllocateEngineContext(PVPlayerEngineDatapath* aEngineDatapath, PVMFNodeInterface* aNode, PVPlayerDatapath* aDatapath, PVCommandId aCmdId, OsclAny* aCmdContext, int32 aCmdType)
{
    // Allocate memory for the context from the fixed size memory pool
    PVPlayerEngineContext* context = NULL;
    int32 leavecode = 0;
    OSCL_TRY(leavecode, context = (PVPlayerEngineContext*)(iCurrentContextListMemPool.allocate(sizeof(PVPlayerEngineContext))));
    OSCL_FIRST_CATCH_ANY(leavecode,
                         PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::AllocateEngineContext() allocate on iCurrentContextListMemPool did a leave!"));
                         OSCL_ASSERT(false));

    OSCL_ASSERT(context);

    // Set the context info
    context->iEngineDatapath = aEngineDatapath;
    context->iNode = aNode;
    context->iDatapath = aDatapath;
    context->iCmdId = aCmdId;
    context->iCmdContext = aCmdContext;
    context->iCmdType = aCmdType;

    // Save the context in the list
    leavecode = 0;
    OSCL_TRY(leavecode, iCurrentContextList.push_back(context));
    OSCL_FIRST_CATCH_ANY(leavecode,
                         PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::AllocateEngineContext() Push back on the context list did a leave!"));
                         iCurrentContextListMemPool.deallocate((OsclAny*)context);
                         OSCL_ASSERT(false);
                         return NULL;);

    return context;
}


void PVPlayerEngine::FreeEngineContext(PVPlayerEngineContext* aContext)
{
    OSCL_ASSERT(aContext);

    // Remove the context from the list
    uint32 i = 0;
    bool foundcontext = false;
    for (i = 0; i < iCurrentContextList.size(); ++i)
    {
        if (iCurrentContextList[i] == aContext)
        {
            foundcontext = true;
            break;
        }
    }

    if (foundcontext)
    {
        iCurrentContextList.erase(iCurrentContextList.begin() + i);
        // Free the memory used by context in the memory pool
        iCurrentContextListMemPool.deallocate((OsclAny*)aContext);
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::FreeEngineContext() Context not on current list (0x%x). CmdType %d", aContext, aContext->iCmdType));
        OSCL_ASSERT(false);
        // Don't return to memory pool since it could add multiple entries
        // of same address in free list
    }
}


void PVPlayerEngine::HandleSourceNodeQueryInitIF(PVPlayerEngineContext& aNodeContext, const PVMFCmdResp& aNodeResp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVPlayerEngine::HandleSourceNodeQueryInitIF() Tick=%d", OsclTickCount::TickCount()));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeQueryInitIF() In"));

    PVMFStatus cmdstatus;
    bool rescheduleAO = false;

    switch (aNodeResp.GetCmdStatus())
    {
        case PVMFSuccess:
            if (iSourceNodePVInterfaceInit)
            {
                iSourceNodeInitIF = (PVMFDataSourceInitializationExtensionInterface*)iSourceNodePVInterfaceInit;
                iSourceNodePVInterfaceInit = NULL;
            }
            // Query for track selection interface
            cmdstatus = DoSourceNodeQueryTrackSelIF(aNodeContext.iCmdId, aNodeContext.iCmdContext);
            if (cmdstatus != PVMFSuccess)
            {
                if ((CheckForSourceRollOver() == true) && (iRollOverState == RollOverStateInProgress))
                {
                    iRollOverState = RollOverStateStart;
                    rescheduleAO = true;
                }
                else
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "PVPlayerEngine::HandleSourceNodeQueryInitIF() DoSourceNodeQueryTrackSelIF failed, Add EH command"));
                    iCommandCompleteStatusInErrorHandling = cmdstatus;
                    iCommandCompleteErrMsgInErrorHandling = NULL;
                    if (iRollOverState == RollOverStateInProgress)
                    {
                        // Init is the current ongoing cmd so queue Init EH cmd
                        AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_INIT, NULL, NULL, NULL, false);
                    }
                    else
                    {
                        // AddDataSource cmd is Current Command, queue ADS EH cmd
                        AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_ADD_DATA_SOURCE, NULL, NULL, NULL, false);
                    }
                    iRollOverState = RollOverStateIdle;
                }
            }
            break;

        default:
        {
            iSourceNodePVInterfaceInit = NULL;
            iSourceNodeInitIF = NULL;

            if ((CheckForSourceRollOver() == true) && (iRollOverState == RollOverStateInProgress))
            {
                iRollOverState = RollOverStateStart;
                rescheduleAO = true;
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVPlayerEngine::HandleSourceNodeQueryInitIF() failed, Add EH command"));
                cmdstatus = aNodeResp.GetCmdStatus();
                PVMFErrorInfoMessageInterface* nextmsg = NULL;
                if (aNodeResp.GetEventExtensionInterface())
                {
                    nextmsg = GetErrorInfoMessageInterface(*(aNodeResp.GetEventExtensionInterface()));
                }

                PVUuid puuid = PVPlayerErrorInfoEventTypesUUID;
                iCommandCompleteErrMsgInErrorHandling = OSCL_NEW(PVMFBasicErrorInfoMessage, (PVPlayerErrSourceInit, puuid, nextmsg));
                iCommandCompleteStatusInErrorHandling = cmdstatus;
                if (iRollOverState == RollOverStateInProgress)
                {
                    // Init is the current ongoing cmd so queue Init EH cmd
                    AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_INIT, NULL, NULL, NULL, false);
                }
                else
                {
                    // AddDataSource cmd is Current Command, queue ADS EH cmd
                    AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_ADD_DATA_SOURCE, NULL, NULL, NULL, false);
                }
                iRollOverState = RollOverStateIdle;
            }
        }
        break;
    }
    if (rescheduleAO)
    {
        if (IsBusy())
        {
            Cancel();
        }

        RunIfNotReady();
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeQueryInitIF() Out"));
}


void PVPlayerEngine::HandleSourceNodeQueryTrackSelIF(PVPlayerEngineContext& aNodeContext, const PVMFCmdResp& aNodeResp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVPlayerEngine::HandleSourceNodeQueryTrackSelIF() Tick=%d", OsclTickCount::TickCount()));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeQueryTrackSelIF() In"));

    PVMFStatus cmdstatus;
    bool rescheduleAO = false;

    switch (aNodeResp.GetCmdStatus())
    {
        case PVMFSuccess:
            if (iSourceNodePVInterfaceTrackSel)
            {
                iSourceNodeTrackSelIF = (PVMFTrackSelectionExtensionInterface*)iSourceNodePVInterfaceTrackSel;
                iSourceNodePVInterfaceTrackSel = NULL;
            }
            // Query the source node for optional extension IFs
            cmdstatus = DoSourceNodeQueryInterfaceOptional(aNodeContext.iCmdId, aNodeContext.iCmdContext);
            if (cmdstatus != PVMFSuccess)
            {
                // If optional extension IFs are not available, just complete the AddDataSource command as success
                if ((CheckForSourceRollOver() == true) && (iRollOverState == RollOverStateInProgress))
                {
                    iRollOverState = RollOverStateStart;
                    rescheduleAO = true;
                }
                else
                {
                    iRollOverState = RollOverStateIdle;
                    EngineCommandCompleted(aNodeContext.iCmdId, aNodeContext.iCmdContext, PVMFSuccess);
                }
            }
            break;

        default:
        {
            iSourceNodePVInterfaceTrackSel = NULL;
            iSourceNodeTrackSelIF = NULL;

            if ((CheckForSourceRollOver() == true) && (iRollOverState == RollOverStateInProgress))
            {
                iRollOverState = RollOverStateStart;
                rescheduleAO = true;
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVPlayerEngine::HandleSourceNodeQueryTrackSelIF() failed, Add EH Command"));
                cmdstatus = aNodeResp.GetCmdStatus();
                PVMFErrorInfoMessageInterface* nextmsg = NULL;
                if (aNodeResp.GetEventExtensionInterface())
                {
                    nextmsg = GetErrorInfoMessageInterface(*(aNodeResp.GetEventExtensionInterface()));
                }

                PVUuid puuid = PVPlayerErrorInfoEventTypesUUID;
                iCommandCompleteErrMsgInErrorHandling = OSCL_NEW(PVMFBasicErrorInfoMessage, (PVPlayerErrSourceInit, puuid, nextmsg));

                iCommandCompleteStatusInErrorHandling = cmdstatus;
                if (iRollOverState == RollOverStateInProgress)
                {
                    // Init is the current ongoing cmd so queue Init EH cmd
                    AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_INIT, NULL, NULL, NULL, false);
                }
                else
                {
                    // AddDataSource cmd is Current Command, queue ADS EH cmd
                    AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_ADD_DATA_SOURCE, NULL, NULL, NULL, false);
                }
                iRollOverState = RollOverStateIdle;
            }
        }
        break;
    }
    if (rescheduleAO)
    {
        if (IsBusy())
        {
            Cancel();
        }

        RunIfNotReady();
    }
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeQueryTrackSelIF() Out"));
}


void PVPlayerEngine::HandleSourceNodeQueryInterfaceOptional(PVPlayerEngineContext& aNodeContext, const PVMFCmdResp& aNodeResp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVPlayerEngine::HandleSourceNodeQueryInterfaceOptional() Tick=%d", OsclTickCount::TickCount()));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeQueryInterfaceOptional() In"));

    // Determine QueryInterface() for which interface completed
    if (aNodeContext.iCmdType == PVP_CMD_SourceNodeQueryTrackLevelInfoIF)
    {
        if ((aNodeResp.GetCmdStatus() == PVMFSuccess) && iSourceNodePVInterfaceTrackLevelInfo)
        {
            iSourceNodeTrackLevelInfoIF = (PVMFTrackLevelInfoExtensionInterface*)iSourceNodePVInterfaceTrackLevelInfo;
            iSourceNodePVInterfaceTrackLevelInfo = NULL;
        }
        else
        {
            // Track level info IF is not available in this data source
            iSourceNodePVInterfaceTrackLevelInfo = NULL;
            iSourceNodeTrackLevelInfoIF = NULL;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_NOTICE, (0, "PVPlayerEngine::HandleSourceNodeQueryInterfaceOptional() Track level info IF not available"));
        }
    }
    else if (aNodeContext.iCmdType == PVP_CMD_SourceNodeQueryPBCtrlIF)
    {
        if ((aNodeResp.GetCmdStatus() == PVMFSuccess) && iSourceNodePVInterfacePBCtrl)
        {
            iSourceNodePBCtrlIF = (PvmfDataSourcePlaybackControlInterface*)iSourceNodePVInterfacePBCtrl;
            iSourceNodePVInterfacePBCtrl = NULL;
        }
        else
        {
            // Playback control is not available in this data source
            iSourceNodePVInterfacePBCtrl = NULL;
            iSourceNodePBCtrlIF = NULL;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_NOTICE, (0, "PVPlayerEngine::HandleSourceNodeQueryInterfaceOptional() Position control IF not available"));
        }
    }
    else if (aNodeContext.iCmdType == PVP_CMD_SourceNodeQueryDirCtrlIF)
    {
        if ((aNodeResp.GetCmdStatus() == PVMFSuccess) && iSourceNodePVInterfaceDirCtrl)
        {
            iSourceNodeDirCtrlIF = (PvmfDataSourceDirectionControlInterface*)iSourceNodePVInterfaceDirCtrl;
            iSourceNodePVInterfaceDirCtrl = NULL;
        }
        else
        {
            // Direction control is not available in this data source
            iSourceNodePVInterfaceDirCtrl = NULL;
            iSourceNodeDirCtrlIF = NULL;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_NOTICE, (0, "PVPlayerEngine::HandleSourceNodeQueryInterfaceOptional() Direction control IF not available"));
        }
    }
    else if (aNodeContext.iCmdType == PVP_CMD_SourceNodeQueryMetadataIF)
    {
        if ((aNodeResp.GetCmdStatus() == PVMFSuccess) && iSourceNodePVInterfaceMetadataExt)
        {
            iSourceNodeMetadataExtIF = (PVMFMetadataExtensionInterface*)iSourceNodePVInterfaceMetadataExt;
            iSourceNodePVInterfaceMetadataExt = NULL;

            // Add the parser node's metadata extension IF to the list
            if (AddToMetadataInterfaceList(iSourceNodeMetadataExtIF, iSourceNodeSessionId, NULL, iSourceNode) != PVMFSuccess)
            {
                iSourceNodeMetadataExtIF->removeRef();
                iSourceNodeMetadataExtIF = NULL;

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_NOTICE, (0, "PVPlayerEngine::HandleSourceNodeQueryInterfaceOptional() Metadata IF could not be added to list"));
            }
        }
        else
        {
            // Metadata is not available in this data source
            iSourceNodePVInterfaceMetadataExt = NULL;
            iSourceNodeMetadataExtIF = NULL;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_NOTICE, (0, "PVPlayerEngine::HandleSourceNodeQueryInterfaceOptional() Metadata IF not available"));
        }
    }
    else if (aNodeContext.iCmdType == PVP_CMD_SourceNodeQueryCapConfigIF)
    {
        if ((aNodeResp.GetCmdStatus() == PVMFSuccess) && iSourceNodePVInterfaceCapConfig)
        {
            iSourceNodeCapConfigIF = (PvmiCapabilityAndConfig*)iSourceNodePVInterfaceCapConfig;
            iSourceNodePVInterfaceCapConfig = NULL;
        }
        else
        {
            // Cap-config is not available in this data source
            iSourceNodePVInterfaceCapConfig = NULL;
            iSourceNodeCapConfigIF = NULL;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_NOTICE, (0, "PVPlayerEngine::HandleSourceNodeQueryInterfaceOptional() Cap-Config IF not available"));
        }
    }
    else if (aNodeContext.iCmdType == PVP_CMD_SourceNodeQueryCPMLicenseIF)
    {
        if ((aNodeResp.GetCmdStatus() == PVMFSuccess) && iSourceNodePVInterfaceCPMLicense)
        {
            iSourceNodeCPMLicenseIF = (PVMFCPMPluginLicenseInterface*)iSourceNodePVInterfaceCPMLicense;
            iSourceNodePVInterfaceCPMLicense = NULL;
        }
        else
        {
            //CPM License is not available in this data source
            iSourceNodePVInterfaceCPMLicense = NULL;
            iSourceNodeCPMLicenseIF = NULL;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_NOTICE, (0, "PVPlayerEngine::HandleSourceNodeQueryInterfaceOptional() CPM License IF not available"));
        }
    }
    else if (aNodeContext.iCmdType == PVP_CMD_SourceNodeQuerySrcNodeRegInitIF)
    {
        if ((aNodeResp.GetCmdStatus() == PVMFSuccess) && iSourceNodePVInterfaceRegInit)
        {
            iSourceNodeRegInitIF = (PVMFDataSourceNodeRegistryInitInterface*)iSourceNodePVInterfaceRegInit;
            iSourceNodePVInterfaceRegInit = NULL;

            // Set source node regsitry
            iSourceNodeRegInitIF->SetPlayerNodeRegistry(&iPlayerNodeRegistry);
        }
        else
        {
            //Node Registry Init Extension Interface is not available in this data source
            iSourceNodePVInterfaceRegInit = NULL;
            iSourceNodeRegInitIF = NULL;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_NOTICE, (0, "PVPlayerEngine::HandleSourceNodeQueryInterfaceOptional() Src Node Registry Init IF not available"));
        }
    }

    // Decrement the pending counter and complete the AddDataSource command if 0.
    --iNumPendingNodeCmd;
    if (iNumPendingNodeCmd == 0)
    {
        if (iRollOverState == RollOverStateInProgress)
        {
            SetRollOverKVPValues();
            PVMFStatus retval = DoSourceNodeInit(aNodeContext.iCmdId, aNodeContext.iCmdContext);
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO, (0, "PVPlayerEngine::HandleSourceNodeQueryInterfaceOptional() - Source Roll Over In Progress - Doing Source Node Init"));

            if (retval == PVMFSuccess)
            {
                SetEngineState(PVP_ENGINE_STATE_INITIALIZING);
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO, (0, "PVPlayerEngine::HandleSourceNodeQueryInterfaceOptional() - Source Roll Over In Progress - DoSourceNodeInit Failed"));
                if ((CheckForSourceRollOver() == true) && (iRollOverState == RollOverStateInProgress))
                {
                    iRollOverState = RollOverStateStart;
                    RunIfNotReady();
                }
                else
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "PVPlayerEngine::HandleSourceNodeQueryInterfaceOptional() DoSourceNodeInit failed, Add EH Command"));
                    iRollOverState = RollOverStateIdle;
                    iCommandCompleteStatusInErrorHandling = retval;
                    iCommandCompleteErrMsgInErrorHandling = NULL;
                    AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_INIT, NULL, NULL, NULL, false);
                }
            }
        }
        else
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeQueryInterfaceOptional() All QueryInterface() commands complete so AddDataSource is complete"));
            EngineCommandCompleted(aNodeContext.iCmdId, aNodeContext.iCmdContext, PVMFSuccess);
        }
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeQueryInterfaceOptional() %d QueryInterface() commands are still pending", iNumPendingNodeCmd));
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeQueryInterfaceOptional() Out"));
}

void PVPlayerEngine::HandleSourceNodeInit(PVPlayerEngineContext& aNodeContext, const PVMFCmdResp& aNodeResp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_INFO,
                    (0, "PVPlayerEngine::HandleSourceNodeInit() Tick=%d", OsclTickCount::TickCount()));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeInit() In"));

    iRollOverState = RollOverStateIdle;
    PVMFStatus cmdstatus;

    switch (aNodeResp.GetCmdStatus())
    {
        case PVMFSuccess:
        {
            // Try to retrieve the duration from the source node via metadata IF
            // Only if we din't got that value through PVMFInfoDurationAvailable informational event.
            if (!iSourceDurationAvailable)
            {
                if (DoSourceNodeGetDurationValue(aNodeContext.iCmdId, aNodeContext.iCmdContext) != PVMFSuccess)
                {
                    // Duration could not be retrieved.
                    // Not an error so complete so engine's Init()
                    SetEngineState(PVP_ENGINE_STATE_INITIALIZED);
                    EngineCommandCompleted(aNodeContext.iCmdId, aNodeContext.iCmdContext, PVMFSuccess);
                }
            }
            else
            {
                // Duration is already available through Info event.
                // so complete  engine's Init()
                SetEngineState(PVP_ENGINE_STATE_INITIALIZED);
                EngineCommandCompleted(aNodeContext.iCmdId, aNodeContext.iCmdContext, PVMFSuccess);
            }
        }
        break;


        case PVMFErrLicenseRequired:
        case PVMFErrHTTPAuthenticationRequired:
        case PVMFErrRedirect:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeInit() PVMFErrLicenseRequired/PVMFErrHTTPAuthenticationRequired/PVMFErrRedirect"));

            SetEngineState(PVP_ENGINE_STATE_IDLE);
            cmdstatus = aNodeResp.GetCmdStatus();

            PVMFErrorInfoMessageInterface* nextmsg = NULL;
            if (aNodeResp.GetEventExtensionInterface())
            {
                nextmsg = GetErrorInfoMessageInterface(*(aNodeResp.GetEventExtensionInterface()));
            }

            PVUuid puuid = PVPlayerErrorInfoEventTypesUUID;
            PVMFBasicErrorInfoMessage* errmsg = OSCL_NEW(PVMFBasicErrorInfoMessage, (PVPlayerErrSourceInit, puuid, nextmsg));

            EngineCommandCompleted(aNodeContext.iCmdId, aNodeContext.iCmdContext, cmdstatus, OSCL_STATIC_CAST(PVInterface*, errmsg), aNodeResp.GetEventData());
            errmsg->removeRef();

        }
        break;

        default:
        {
            if (iState == PVP_ENGINE_STATE_INITIALIZING)
            {
                SetEngineState(PVP_ENGINE_STATE_IDLE);
                if (CheckForSourceRollOver() == false)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSourceNodeInit() failed, Add EH Command"));
                    cmdstatus = aNodeResp.GetCmdStatus();

                    PVMFErrorInfoMessageInterface* nextmsg = NULL;
                    if (aNodeResp.GetEventExtensionInterface())
                    {
                        nextmsg = GetErrorInfoMessageInterface(*(aNodeResp.GetEventExtensionInterface()));
                    }

                    PVUuid puuid = PVPlayerErrorInfoEventTypesUUID;
                    iCommandCompleteErrMsgInErrorHandling = OSCL_NEW(PVMFBasicErrorInfoMessage, (PVPlayerErrSourceInit, puuid, nextmsg));
                    iCommandCompleteStatusInErrorHandling = cmdstatus;
                    AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_INIT, NULL, NULL, NULL, false);
                }
                else
                {
                    // Initialization of source node failed so try alternates
                    //reschedule to do source node roll over
                    iRollOverState = RollOverStateStart;
                    //remove any queued up auto-pause/auto-resume commands
                    //they are no longer applicable since we are doing a change of sourcenode
                    removeCmdFromQ(iPendingCmds, PVP_ENGINE_COMMAND_PAUSE_DUE_TO_BUFFER_UNDERFLOW, true);
                    removeCmdFromQ(iPendingCmds, PVP_ENGINE_COMMAND_RESUME_DUE_TO_BUFFER_DATAREADY, true);
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSourceNodeInit() - Rescheduling to do source roll over"));
                    RunIfNotReady();
                }
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSourceNodeInit() - Incorrect State - Asserting"));
                OSCL_ASSERT(false);
            }
        }
        break;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeInit() Out"));
}


void PVPlayerEngine::HandleSourceNodeGetDurationValue(PVPlayerEngineContext& aNodeContext, const PVMFCmdResp& aNodeResp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVPlayerEngine::HandleSourceNodeGetDurationValue() Tick=%d", OsclTickCount::TickCount()));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeGetDurationValue() In"));

    switch (aNodeResp.GetCmdStatus())
    {
        case PVMFSuccess:
        {
            // Extract the duration and save it
            // Check that there is one KVP in value list
            if (iSourceDurationValueList.size() != 1)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO, (0, "PVPlayerEngine::HandleSourceNodeGetDurationValue() Value list size is not 1 (size=%d)",
                                iSourceDurationValueList.size()));
                break;
            }

            // Check that the key in KVP is not NULL
            if (iSourceDurationValueList[0].key == NULL)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO, (0, "PVPlayerEngine::HandleSourceNodeGetDurationValue() Value list key string is NULL"));
                break;
            }

            // Check that value is for duration
            int retval = pv_mime_strstr(iSourceDurationValueList[0].key, (char*)_STRLIT_CHAR("duration"));
            if (retval == -1)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO, (0, "PVPlayerEngine::HandleSourceNodeGetDurationValue() Key string does not contain duration"));
                break;
            }

            // Check that duration value is uint32. If not available assume uint32.
            PvmiKvpValueType durvaltype = GetValTypeFromKeyString(iSourceDurationValueList[0].key);
            if (durvaltype != PVMI_KVPVALTYPE_UINT32 && durvaltype != PVMI_KVPVALTYPE_UNKNOWN)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO, (0, "PVPlayerEngine::HandleSourceNodeGetDurationValue() Value type is not uint32 or unknown"));
                break;
            }
            iSourceDurationInMS = iSourceDurationValueList[0].value.uint32_value;

            // Check the timescale. If not available, assume millisecond (1000)
            const char* retsubstr = NULL;
            uint32 retsubstrlen = 0;
            uint32 tsparamlen = oscl_strlen(_STRLIT_CHAR("timescale="));
            retsubstr = oscl_strstr(iSourceDurationValueList[0].key, _STRLIT_CHAR("timescale="));
            if (retsubstr != NULL)
            {
                retsubstrlen = oscl_strlen(retsubstr);
                if (retsubstrlen > tsparamlen)
                {
                    uint32 timescale = 0;
                    PV_atoi((char*)(retsubstr + tsparamlen), 'd', (retsubstrlen - tsparamlen), timescale);
                    if (timescale > 0 && timescale != 1000)
                    {
                        // Convert to milliseconds
                        MediaClockConverter mcc(timescale);
                        mcc.update_clock(iSourceDurationInMS);
                        iSourceDurationInMS = mcc.get_converted_ts(1000);
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO, (0, "PVPlayerEngine::HandleSourceNodeGetDurationValue() Timescale for duration is %d",
                                        timescale));
                    }
                }
            }

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO, (0, "PVPlayerEngine::HandleSourceNodeGetDurationValue() Duration in millisec is %d",
                            iSourceDurationInMS));
            iSourceDurationAvailable = true;
        }
        break;

        default:
        {
            // Duration is not available
            // Do nothing
        }
        break;
    }

    // Release any metadata values back to source node
    // and then clear it
    if (iSourceDurationValueList.empty() == false)
    {
        if (iSourceNodeMetadataExtIF != NULL)
        {
            iSourceNodeMetadataExtIF->ReleaseNodeMetadataValues(iSourceDurationValueList, 0, iSourceDurationValueList.size());
        }
        iSourceDurationValueList.clear();
    }

    // Engine's Init() is now complete
    SetEngineState(PVP_ENGINE_STATE_INITIALIZED);
    EngineCommandCompleted(aNodeContext.iCmdId, aNodeContext.iCmdContext, PVMFSuccess);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeGetDurationValue() Out"));
}

void PVPlayerEngine::HandleSourceNodeSetDataSourceRate(PVPlayerEngineContext& aNodeContext, const PVMFCmdResp& aNodeResp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeSetDataSourceRate() In"));

    PVMFStatus cmdstatus = aNodeResp.GetCmdStatus();

    if (cmdstatus != PVMFSuccess)
    {
        EngineCommandCompleted(aNodeContext.iCmdId, aNodeContext.iCmdContext, cmdstatus);
    }
    else
    {
        //Continue on to sink rate change.
        cmdstatus = DoSinkNodeChangeClockRate();
        if (cmdstatus != PVMFSuccess)
        {
            EngineCommandCompleted(aNodeContext.iCmdId, aNodeContext.iCmdContext, cmdstatus);
        }
        else
        {
            //Rate Change is complete.

            //Install the updated rate and timebase.
            UpdateTimebaseAndRate();

            //Start direction change sequence if needed.
            if (iPlaybackDirection_New != iPlaybackDirection)
            {
                cmdstatus = UpdateCurrentDirection(aNodeContext.iCmdId, aNodeContext.iCmdContext);
                switch (cmdstatus)
                {
                    case PVMFPending:
                        //wait on node command completion and call to HandleSourceNodeSetDataSourceDirection
                        break;
                    case PVMFSuccess:
                        //engine command is done, but direction is not actually set on the
                        //source until the Resume or Prepare happens.
                        EngineCommandCompleted(aNodeContext.iCmdId, aNodeContext.iCmdContext, PVMFSuccess);
                        break;
                    default:
                        //failed!
                        EngineCommandCompleted(aNodeContext.iCmdId, aNodeContext.iCmdContext, cmdstatus);
                        break;
                }
            }
            else
            {
                //SetPlaybackRate is complete!
                EngineCommandCompleted(aNodeContext.iCmdId, aNodeContext.iCmdContext, PVMFSuccess);
            }
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeSetDataSourceRate() Out"));
}

PVMFStatus PVPlayerEngine::DoSinkNodeChangeClockRate()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSinkNodeChangeClockRate() In"));

    // Check with sink nodes
    PVMFStatus cmdstatus = PVMFSuccess;

    for (uint32 i = 0; i < iDatapathList.size(); ++i)
    {
        if (iDatapathList[i].iDatapath && iDatapathList[i].iSinkNodeSyncCtrlIF)
        {
            cmdstatus = iDatapathList[i].iSinkNodeSyncCtrlIF->ChangeClockRate(iPlaybackClockRate_New);

            if (cmdstatus != PVMFSuccess)
            {
                // One of the sinks reported not supported so don't allow the clock rate change.
                break;
            }
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::DoSinkNodeChangeClockRate() Out"));
    return cmdstatus;
}

void PVPlayerEngine::UpdateDirection(PVMFTimestamp aNPT, PVMFTimestamp aMediaTS, PVPPlaybackPosition& aPos)
{
    //First note the current observed NPT value.  We will reposition to this
    //value.
    PVPPlaybackPosition curpos;
    curpos.iIndeterminate = false;
    curpos.iPosUnit = PVPPBPOSUNIT_MILLISEC;
    GetPlaybackClockPosition(curpos);
    aPos = curpos;

    //Install the new value for direction.
    iPlaybackDirection = iPlaybackDirection_New;

    //Save the start NPT and TS
    iStartNPT = aNPT;
    iStartMediaDataTS = aMediaTS;

    if (iPlaybackDirection_New < 0)
    {
        if (aPos.iPosValue.millisec_value >= iSourceDurationInMS)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::UpdateDirection() Current pos %dms is more than Duration %dms", aPos.iPosValue.millisec_value, iSourceDurationInMS));
            if (ConvertFromMillisec((uint32)(iSourceDurationInMS - 1), aPos) != PVMFSuccess)
            {
                // Other position units are not supported yet
                aPos.iIndeterminate = true;
            }
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVPlayerEngine::UpdateDirection() Direction %d New Start NPT %d Start Media Data TS %d, Repos NPT %d, Pos Indeterminate %d"
                     , iPlaybackDirection
                     , iStartNPT
                     , iStartMediaDataTS
                     , aPos.iPosValue.millisec_value
                     , aPos.iIndeterminate));
}

void PVPlayerEngine::UpdateTimebaseAndRate()
{
    if (iPlaybackClockRate_New == iPlaybackClockRate
            && iOutsideTimebase_New == iOutsideTimebase)
        return;//no update needed

    //Install the new values for rate & timebase.
    iPlaybackClockRate = iPlaybackClockRate_New;
    iOutsideTimebase = iOutsideTimebase_New;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVPlayerEngine::UpdateTimebaseAndRate() Rate %d OutsideTB 0x%x CurDir %d NewDir %d"
                     , iPlaybackClockRate, iOutsideTimebase
                     , iPlaybackDirection, iPlaybackDirection_New));

    // Pause the clock if running. If already stopped or paused, the call would fail
    bool clockpaused = iPlaybackClock.Pause();

    if (iOutsideTimebase)
    {
        //use the outside timebase & ignore the rate.
        iPlaybackClock.SetClockTimebase(*iOutsideTimebase);
    }
    else
    {
        //use the player timebase and set the rate.
        iPlaybackTimebase.SetRate(iPlaybackClockRate);
        iPlaybackClock.SetClockTimebase(iPlaybackTimebase);
    }

    // Only restart the clock if the clock was paused in this function
    if (clockpaused)
    {
        StartPlaybackClock();
    }
}

void PVPlayerEngine::HandleSinkNodeQueryCapConfigIF(PVPlayerEngineContext& aNodeContext, const PVMFCmdResp& aNodeResp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVPlayerEngine::HandleSinkNodeQueryCapConfigIF() Tick=%d", OsclTickCount::TickCount()));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSinkNodeQueryCapConfigIF() In"));

    switch (aNodeResp.GetCmdStatus())
    {
        case PVMFSuccess:
        {
            if (aNodeContext.iEngineDatapath->iSinkNodePVInterfaceCapConfig)
            {
                aNodeContext.iEngineDatapath->iSinkNodeCapConfigIF = (PvmiCapabilityAndConfig*)aNodeContext.iEngineDatapath->iSinkNodePVInterfaceCapConfig;
                aNodeContext.iEngineDatapath->iSinkNodePVInterfaceCapConfig = NULL;
            }
        }
        break;

        default:
        {
            if (aNodeContext.iNode == aNodeContext.iEngineDatapath->iSinkNode)
            {
                aNodeContext.iEngineDatapath->iSinkNodePVInterfaceCapConfig = NULL;
                aNodeContext.iEngineDatapath->iSinkNodeCapConfigIF = NULL;
            }
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVPlayerEngine::HandleSinkNodeQueryCapConfigIF() Node dont support Cap-Config Interface, ignoring"));
        }
        break;
    }

    // Decrement the pending counter and go to next step if 0.
    --iNumPendingNodeCmd;
    if (iNumPendingNodeCmd == 0)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVPlayerEngine::HandleSinkNodeQueryCapConfigIF() All QueryInterface() commands complete"));

        PVMFStatus cmdstatus = DoSinkNodeInit(aNodeContext.iCmdId, aNodeContext.iCmdContext);
        if (cmdstatus != PVMFSuccess)
        {
            bool ehPending = CheckForPendingErrorHandlingCmd();
            if (ehPending)
            {
                // there should be no error handling queued.
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSinkNodeQueryCapConfigIF() Already EH pending, should never happen"));
                return;
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVPlayerEngine::HandleSinkNodeQueryCapConfigIF() DoSinkNodeInit failed, Add EH Command"));
                iCommandCompleteStatusInErrorHandling = cmdstatus;
                iCommandCompleteErrMsgInErrorHandling = NULL;
                AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_PREPARE, NULL, NULL, NULL, false);
            }
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSinkNodeQueryCapConfigIF() Out"));
}

void PVPlayerEngine::HandleSinkNodeInit(PVPlayerEngineContext& aNodeContext, const PVMFCmdResp& aNodeResp)
{
    OSCL_UNUSED_ARG(aNodeContext);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVPlayerEngine::HandleSinkNodeInit() Tick=%d", OsclTickCount::TickCount()));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSinkNodeInit() In"));

    if (aNodeResp.GetCmdStatus() != PVMFSuccess)
    {
        bool ehPending = CheckForPendingErrorHandlingCmd();
        if (ehPending)
        {
            // there should be no error handling queued.
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSinkNodeInit() Already EH pending, should never happen"));
            return;
        }
        else
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVPlayerEngine::HandleSinkNodeInit() cmd response is failure, Add EH Command"));
            PVMFErrorInfoMessageInterface* nextmsg = NULL;
            if (aNodeResp.GetEventExtensionInterface())
            {
                nextmsg = GetErrorInfoMessageInterface(*(aNodeResp.GetEventExtensionInterface()));
            }

            PVUuid puuid = PVPlayerErrorInfoEventTypesUUID;
            iCommandCompleteErrMsgInErrorHandling = OSCL_NEW(PVMFBasicErrorInfoMessage, (PVPlayerErrSinkInit, puuid, nextmsg));
            iCommandCompleteStatusInErrorHandling = aNodeResp.GetCmdStatus();
            AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_PREPARE, NULL, NULL, NULL, false);
            return;
        }
    }

    // Decrement the pending counter and go to next step if 0.
    --iNumPendingNodeCmd;
    if (iNumPendingNodeCmd == 0)
    {
        // Init on sink nodes is complete, next step in track selection is to check with the sink nodes alone if the track is
        // playable or not. If track can be played using sink nodes only move to track selection or else go further to
        // instantiate decoder nodes for the tracks.

        // set the Engine state to Track selection so that engine calls DoPrepare and do further processing in Track selection

        SetEngineState(PVP_ENGINE_STATE_TRACK_SELECTION_1_DONE);
        RunIfNotReady();
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSinkNodeInit() Out"));

    return;
}

void PVPlayerEngine::HandleDecNodeQueryCapConfigIF(PVPlayerEngineContext& aNodeContext, const PVMFCmdResp& aNodeResp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVPlayerEngine::HandleDecNodeQueryCapConfigIF() Tick=%d", OsclTickCount::TickCount()));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleDecNodeQueryCapConfigIF() In"));

    for (uint32 i = 0; i < iTrackSelectionList.size(); i++)
    {
        switch (aNodeResp.GetCmdStatus())
        {
            case PVMFSuccess:
            {
                if (aNodeContext.iNode == iTrackSelectionList[i].iTsDecNode && iTrackSelectionList[i].iTsDecNodePVInterfaceCapConfig)
                {
                    iTrackSelectionList[i].iTsDecNodeCapConfigIF = (PvmiCapabilityAndConfig*)iTrackSelectionList[i].iTsDecNodePVInterfaceCapConfig;
                    iTrackSelectionList[i].iTsDecNodePVInterfaceCapConfig = NULL;

                    PVMFTrackInfo* currTrack = iSourcePresInfoList.getTrackInfo(i);
                    // Valid decoder node set in TrackSelectionList. Scan the TrackSelectionList further and if
                    // any similar MIME track is present just set the decoders to NULL for now.
                    // This is to avoid multiple Init calls on same decoder nodes for similar tracks.
                    // Set the decoder nodes and its cap and config I/F once decoder node Inits complete.
                    for (uint32 j = i + 1; j < iTrackSelectionList.size(); j++)
                    {
                        PVMFTrackInfo* tmpTrack = iSourcePresInfoList.getTrackInfo(j);
                        if (!(pv_mime_strcmp(currTrack->getTrackMimeType().get_str(), tmpTrack->getTrackMimeType().get_str())))
                        {
                            // These were earlier set in DoDecNodeQueryCapConfifIF to avoid multiple creation of same
                            // decoder nodes.
                            iTrackSelectionList[j].iTsDecNode = NULL;
                            iTrackSelectionList[j].iTsDecNodeSessionId = 0;
                            iTrackSelectionList[j].iTsDecNodeCapConfigIF = NULL;
                        }
                    }
                }
            }
            break;

            default:
            {
                if (aNodeContext.iNode == iTrackSelectionList[i].iTsDecNode)
                {
                    iTrackSelectionList[i].iTsDecNodePVInterfaceCapConfig = NULL;
                    iTrackSelectionList[i].iTsDecNodeCapConfigIF = NULL;
                }
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVPlayerEngine::HandleDecNodeQueryCapConfigIF() Node dont support Cap-Config Interface, ignoring"));
            }
            break;
        }
    }

    // Decrement the pending counter and go to next step if 0.
    --iNumPendingNodeCmd;
    if (iNumPendingNodeCmd == 0)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVPlayerEngine::HandleDecNodeQueryCapConfigIF() All QueryInterface() commands complete"));

        PVMFStatus cmdstatus = DoDecNodeInit(aNodeContext.iCmdId, aNodeContext.iCmdContext);
        if (cmdstatus != PVMFSuccess)
        {
            bool ehPending = CheckForPendingErrorHandlingCmd();
            if (ehPending)
            {
                // there should be no error handling queued.
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleDecNodeQueryCapConfigIF() Already EH pending, should never happen"));
                return;
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVPlayerEngine::HandleDecNodeQueryCapConfigIF() DoDecNodeInit failed, Add EH Command"));
                iCommandCompleteStatusInErrorHandling = cmdstatus;
                iCommandCompleteErrMsgInErrorHandling = NULL;
                AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_PREPARE, NULL, NULL, NULL, false);
            }
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleDecNodeQueryCapConfigIF() Out"));
}

void PVPlayerEngine::HandleDecNodeInit(PVPlayerEngineContext& aNodeContext, const PVMFCmdResp& aNodeResp)
{
    OSCL_UNUSED_ARG(aNodeContext);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVPlayerEngine::HandleDecNodeInit() Tick=%d", OsclTickCount::TickCount()));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleDecNodeInit() In"));

    if (aNodeResp.GetCmdStatus() != PVMFSuccess)
    {
        bool ehPending = CheckForPendingErrorHandlingCmd();
        if (ehPending)
        {
            // there should be no error handling queued.
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleDecNodeInit() Already EH pending, should never happen"));
            return;
        }
        else
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVPlayerEngine::HandleDecNodeInit() cmd response is failure, Add EH Command"));
            PVMFErrorInfoMessageInterface* nextmsg = NULL;
            if (aNodeResp.GetEventExtensionInterface())
            {
                nextmsg = GetErrorInfoMessageInterface(*(aNodeResp.GetEventExtensionInterface()));
            }

            PVUuid puuid = PVPlayerErrorInfoEventTypesUUID;
            iCommandCompleteErrMsgInErrorHandling = OSCL_NEW(PVMFBasicErrorInfoMessage, (PVPlayerErrDatapath, puuid, nextmsg));
            iCommandCompleteStatusInErrorHandling = aNodeResp.GetCmdStatus();
            AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_PREPARE, NULL, NULL, NULL, false);
            return;
        }
    }

    // Decrement the pending counter and go to next step if 0.
    --iNumPendingNodeCmd;
    if (iNumPendingNodeCmd == 0)
    {
        // All decoder node Init is complete, now set the decoder node for similar tracks in TrackSelectionList.
        for (uint32 i = 0; i < iTrackSelectionList.size(); i++)
        {
            PVMFTrackInfo* currTrack = iSourcePresInfoList.getTrackInfo(i);

            for (uint32 j = i + 1; j < iTrackSelectionList.size(); j++)
            {
                // If the track has the same MIME type and needs to have a decoder node for it to be playabale track
                // use the already created decoder node for the track.
                PVMFTrackInfo* tmpTrack = iSourcePresInfoList.getTrackInfo(j);

                if ((iTrackSelectionList[i].iTsDecNode != NULL) &&
                        !(pv_mime_strcmp(currTrack->getTrackMimeType().get_str(), tmpTrack->getTrackMimeType().get_str())) &&
                        !(iTrackSelectionList[j].iTsTrackValidForPlayableList))
                {
                    // These were earlier set in DoDecNodeQueryCapConfifIF to avoid multiple creation of same
                    // decoder nodes.
                    iTrackSelectionList[j].iTsDecNode = iTrackSelectionList[i].iTsDecNode;
                    iTrackSelectionList[j].iTsDecNodeSessionId = iTrackSelectionList[i].iTsDecNodeSessionId;
                    iTrackSelectionList[j].iTsDecNodeCapConfigIF = iTrackSelectionList[i].iTsDecNodeCapConfigIF;
                }
            }
        }

        // Now engine has all decoder and sink nodes setup for each valid track. Engine will now do the track selection based
        // on config paramters of each track

        SetEngineState(PVP_ENGINE_STATE_TRACK_SELECTION_2_DONE);
        RunIfNotReady();
    }
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleDecNodeInit() Out"));
    return;
}

void PVPlayerEngine::HandleSourceNodePrepare(PVPlayerEngineContext& aNodeContext, const PVMFCmdResp& aNodeResp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_INFO,
                    (0, "PVPlayerEngine::HandleSourceNodePrepare() Tick=%d", OsclTickCount::TickCount()));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodePrepare() In"));

    PVMFStatus cmdstatus = PVMFErrNotSupported;

    switch (aNodeResp.GetCmdStatus())
    {
        case PVMFSuccess:
        {
            // Datapaths are already set during intelligent track selection, just query for optional interfaces.
            iNumPendingDatapathCmd = 0;
            for (uint32 i = 0; i < iDatapathList.size(); ++i)
            {
                if (iDatapathList[i].iTrackInfo != NULL)
                {
                    PVMFStatus retcode = DoSinkNodeQueryInterfaceOptional(iDatapathList[i], aNodeContext.iCmdId, aNodeContext.iCmdContext);
                    if (retcode == PVMFSuccess)
                    {
                        ++iNumPendingDatapathCmd;
                        cmdstatus = PVMFSuccess;
                    }
                    else
                    {
                        cmdstatus = retcode;
                    }
                }
            }

            if (iNumPendingDatapathCmd == 0)
            {
                if (cmdstatus == PVMFErrNotSupported)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSourceNodePrepare() No datapath could be setup. Asserting"));
                    OSCL_ASSERT(false);
                }
                bool ehPending = CheckForPendingErrorHandlingCmd();
                if (ehPending)
                {
                    // there should be no error handling queued.
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSourceNodePrepare() Already EH pending, should never happen"));
                    return;
                }
                else
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "PVPlayerEngine::HandleSourceNodePrepare() Report command as failed, Add EH Command"));
                    iCommandCompleteStatusInErrorHandling = cmdstatus;
                    iCommandCompleteErrMsgInErrorHandling = NULL;
                    AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_PREPARE, NULL, NULL, NULL, false);
                }
            }
        }
        break;

        default:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVPlayerEngine::HandleSourceNodePrepare() failed, Add EH Command"));
            bool ehPending = CheckForPendingErrorHandlingCmd();
            if (ehPending)
            {
                // there should be no error handling queued.
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSourceNodePrepare() Already EH pending, should never happen"));
                return;
            }
            PVMFErrorInfoMessageInterface* nextmsg = NULL;
            if (aNodeResp.GetEventExtensionInterface())
            {
                nextmsg = GetErrorInfoMessageInterface(*(aNodeResp.GetEventExtensionInterface()));
            }

            PVUuid puuid = PVPlayerErrorInfoEventTypesUUID;
            iCommandCompleteErrMsgInErrorHandling = OSCL_NEW(PVMFBasicErrorInfoMessage, (PVPlayerErrSourceFatal, puuid, nextmsg));
            iCommandCompleteStatusInErrorHandling = aNodeResp.GetCmdStatus();

            AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_PREPARE, NULL, NULL, NULL, false);
        }
        break;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodePrepare() Out"));
}

void PVPlayerEngine::HandleSinkNodeQueryInterfaceOptional(PVPlayerEngineContext& aNodeContext, const PVMFCmdResp& aNodeResp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVPlayerEngine::HandleSinkNodeQueryInterfaceOptional() Tick=%d", OsclTickCount::TickCount()));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSinkNodeQueryInterfaceOptional() In %s", aNodeContext.iEngineDatapath->iTrackInfo->getTrackMimeType().get_cstr()));

    // Determine QueryInterface() for which interface completed
    if (aNodeContext.iCmdType == PVP_CMD_SinkNodeQuerySyncCtrlIF)
    {
        if (aNodeResp.GetCmdStatus() == PVMFSuccess && aNodeContext.iEngineDatapath->iSinkNodePVInterfaceSyncCtrl)
        {
            aNodeContext.iEngineDatapath->iSinkNodeSyncCtrlIF = (PvmfNodesSyncControlInterface*)aNodeContext.iEngineDatapath->iSinkNodePVInterfaceSyncCtrl;
            aNodeContext.iEngineDatapath->iSinkNodePVInterfaceSyncCtrl = NULL;

            // Pass the playback clock to the sync control
            aNodeContext.iEngineDatapath->iSinkNodeSyncCtrlIF->SetClock(&iPlaybackClock);

            // Set the sync margin, find corresponding track for the datapath using mime string
            bool videoTrack = false;
            bool audioTrack = false;
            bool textTrack = false;
            bool retVal = FindTrackForDatapathUsingMimeString(videoTrack, audioTrack, textTrack, aNodeContext.iEngineDatapath);
            if (textTrack && retVal)
            {
                // Text track
                aNodeContext.iEngineDatapath->iSinkNodeSyncCtrlIF->SetMargins((-1*iSyncMarginText.min), iSyncMarginText.max);
            }
            else if (audioTrack && retVal)
            {
                // Audio track
                aNodeContext.iEngineDatapath->iSinkNodeSyncCtrlIF->SetMargins((-1*iSyncMarginAudio.min), iSyncMarginAudio.max);
            }
            else
            {
                // Video track available or an unknown datapath
                aNodeContext.iEngineDatapath->iSinkNodeSyncCtrlIF->SetMargins((-1*iSyncMarginVideo.min), iSyncMarginVideo.max);
            }
        }
        else
        {
            // sync control interface is not available in this sink node
            aNodeContext.iEngineDatapath->iSinkNodePVInterfaceSyncCtrl = NULL;
            aNodeContext.iEngineDatapath->iSinkNodeSyncCtrlIF = NULL;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_NOTICE, (0, "PVPlayerEngine::HandleSinkNodeQueryInterfaceOptional() Metadata IF not available"));
            OSCL_ASSERT(false);
        }
    }
    else if (aNodeContext.iCmdType == PVP_CMD_SinkNodeQueryMetadataIF)
    {
        if (aNodeResp.GetCmdStatus() == PVMFSuccess && aNodeContext.iEngineDatapath->iSinkNodePVInterfaceMetadataExt)
        {
            aNodeContext.iEngineDatapath->iSinkNodeMetadataExtIF = (PVMFMetadataExtensionInterface*)aNodeContext.iEngineDatapath->iSinkNodePVInterfaceMetadataExt;
            aNodeContext.iEngineDatapath->iSinkNodePVInterfaceMetadataExt = NULL;

            // Add the video sink node's metadata extension IF to the list
            if (AddToMetadataInterfaceList(aNodeContext.iEngineDatapath->iSinkNodeMetadataExtIF, aNodeContext.iEngineDatapath->iSinkNodeSessionId, aNodeContext.iEngineDatapath, aNodeContext.iEngineDatapath->iSinkNode) != PVMFSuccess)
            {
                aNodeContext.iEngineDatapath->iSinkNodeMetadataExtIF->removeRef();
                aNodeContext.iEngineDatapath->iSinkNodeMetadataExtIF = NULL;
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_NOTICE, (0, "PVPlayerEngine::HandleSinkNodeQueryInterfaceOptional() Metadata IF could not be added to list"));
            }
        }
        else
        {
            // Metadata is not available in this video sink node
            aNodeContext.iEngineDatapath->iSinkNodePVInterfaceMetadataExt = NULL;
            aNodeContext.iEngineDatapath->iSinkNodeMetadataExtIF = NULL;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_NOTICE, (0, "PVPlayerEngine::HandleSinkNodeQueryInterfaceOptional() Metadata IF not available"));
        }
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSinkNodeQueryInterfaceOptional() Unknown cmd type. Asserting"));
        OSCL_ASSERT(false);
    }

    // Decrement the pending counter and go to next step if 0.
    --aNodeContext.iEngineDatapath->iNumPendingCmd;
    if (aNodeContext.iEngineDatapath->iNumPendingCmd == 0)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSinkNodeQueryInterfaceOptional() All QueryInterface() commands complete"));

        // Create the decoder node if necessary
        PVMFStatus cmdstatus = DoDecNodeQueryInterfaceOptional(*(aNodeContext.iEngineDatapath), aNodeContext.iCmdId, aNodeContext.iCmdContext);
        if (cmdstatus == PVMFErrNotSupported)
        {
            cmdstatus = DoDatapathPrepare(*(aNodeContext.iEngineDatapath), aNodeContext.iCmdId, aNodeContext.iCmdContext);
        }

        if (cmdstatus != PVMFSuccess)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVPlayerEngine::HandleSinkNodeQueryInterfaceOptional() Report command as failed, Add EH Command"));
            bool ehPending = CheckForPendingErrorHandlingCmd();
            if (ehPending)
            {
                // there should be no error handling queued.
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSinkNodeQueryInterfaceOptional() Already EH pending, should never happen"));
                return;
            }
            else
            {
                iCommandCompleteStatusInErrorHandling = cmdstatus;
                iCommandCompleteErrMsgInErrorHandling = NULL;
                AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_PREPARE, NULL, NULL, NULL, false);
            }
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSinkNodeQueryInterfaceOptional() Out"));
}

void PVPlayerEngine::HandleSinkNodeDecNodeReset(PVPlayerEngineContext& aNodeContext, const PVMFCmdResp& aNodeResp)
{
    OSCL_UNUSED_ARG(aNodeContext);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVPlayerEngine::HandleSinkNodeDecNodeReset() Tick=%d", OsclTickCount::TickCount()));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSinkNodeDecNodeReset() In"));

    if (aNodeResp.GetCmdStatus() != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVPlayerEngine::HandleSinkNodeDecNodeReset() Reset failed, assert"));
        OSCL_ASSERT(false);
        return;
    }

    // Decrement the pending counter and go to next step if 0.
    --iNumPendingNodeCmd;
    if (iNumPendingNodeCmd == 0)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSinkNodeDecNodeReset() All Sinknode and decnode Reset() commands complete"));
        SetEngineState(PVP_ENGINE_STATE_TRACK_SELECTION_3_DONE);
        RunIfNotReady();
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSinkNodeDecNodeReset() Out"));
}

void PVPlayerEngine::HandleDecNodeQueryInterfaceOptional(PVPlayerEngineContext& aNodeContext, const PVMFCmdResp& aNodeResp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVPlayerEngine::HandleDecNodeQueryInterfaceOptional() Tick=%d", OsclTickCount::TickCount()));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleDecNodeQueryInterfaceOptional() In %s", aNodeContext.iEngineDatapath->iTrackInfo->getTrackMimeType().get_cstr()));

    // Determine QueryInterface() for which interface completed
    if (aNodeContext.iCmdType == PVP_CMD_DecNodeQueryMetadataIF)
    {
        if (aNodeResp.GetCmdStatus() == PVMFSuccess && aNodeContext.iEngineDatapath->iDecNodePVInterfaceMetadataExt)
        {
            aNodeContext.iEngineDatapath->iDecNodeMetadataExtIF = (PVMFMetadataExtensionInterface*)aNodeContext.iEngineDatapath->iDecNodePVInterfaceMetadataExt;
            aNodeContext.iEngineDatapath->iDecNodePVInterfaceMetadataExt = NULL;

            // Add the video dec node's metadata extension IF to the list
            if (AddToMetadataInterfaceList(aNodeContext.iEngineDatapath->iDecNodeMetadataExtIF, aNodeContext.iEngineDatapath->iDecNodeSessionId, aNodeContext.iEngineDatapath, aNodeContext.iEngineDatapath->iDecNode) != PVMFSuccess)
            {
                aNodeContext.iEngineDatapath->iDecNodeMetadataExtIF->removeRef();
                aNodeContext.iEngineDatapath->iDecNodeMetadataExtIF = NULL;

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_NOTICE, (0, "PVPlayerEngine::HandleDecNodeQueryInterfaceOptional() Metadata IF could not be added to list"));
            }
        }
        else
        {
            // Metadata is not available in this dec node
            aNodeContext.iEngineDatapath->iDecNodePVInterfaceMetadataExt = NULL;
            aNodeContext.iEngineDatapath->iDecNodeMetadataExtIF = NULL;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_NOTICE, (0, "PVPlayerEngine::HandleDecNodeQueryInterfaceOptional() Metadata IF not available"));
        }
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleDecNodeQueryInterfaceOptional() Unknown cmd type. Asserting"));
        OSCL_ASSERT(false);
    }

    if (aNodeContext.iEngineDatapath->iDecNodeCapConfigIF)
    {
        // Configure the dec node for player use
        PvmiKvp kvpparam;
        PvmiKvp* retkvp = NULL;
        OSCL_StackString<64> kvpparamkey;

        bool videoTrack = false;
        bool audioTrack = false;
        bool textTrack = false;
        bool retVal = FindTrackForDatapathUsingMimeString(videoTrack, audioTrack, textTrack, aNodeContext.iEngineDatapath);
        if (videoTrack && retVal)
        {
            // Video track
            // Disable drop frame mode
            kvpparamkey = _STRLIT_CHAR("x-pvmf/video/decoder/dropframe_enable;valtype=bool");
            kvpparam.value.bool_value = false;
        }
        else if (audioTrack && retVal)
        {
            // Audio track
            // Disable silence insertion
            kvpparamkey = _STRLIT_CHAR("x-pvmf/audio/decoder/silenceinsertion_enable;valtype=bool");
            kvpparam.value.bool_value = false;
            aNodeContext.iEngineDatapath->iSinkNodeSyncCtrlIF->SetMargins((-1*iSyncMarginAudio.min), iSyncMarginAudio.max);
        }

        if (kvpparamkey.get_size() > 0)
        {
            kvpparam.key = kvpparamkey.get_str();
            aNodeContext.iEngineDatapath->iDecNodeCapConfigIF->setParametersSync(NULL, &kvpparam, 1, retkvp);
            if (retkvp != NULL)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_NOTICE, (0, "PVPlayerEngine::HandleDecNodeQueryInterfaceOptional() Configuring dec node for player use via cap-config IF failed"));
            }
        }
    }

    // Decrement the pending counter and go to next step if 0.
    OSCL_ASSERT(aNodeContext.iEngineDatapath->iNumPendingCmd > 0);
    --aNodeContext.iEngineDatapath->iNumPendingCmd;
    if (aNodeContext.iEngineDatapath->iNumPendingCmd == 0)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleDecNodeQueryInterfaceOptional() All QueryInterface() commands complete"));

        // Prepare the datapath
        PVMFStatus cmdstatus = DoDatapathPrepare(*(aNodeContext.iEngineDatapath), aNodeContext.iCmdId, aNodeContext.iCmdContext);

        if (cmdstatus != PVMFSuccess)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVPlayerEngine::HandleDecNodeQueryInterfaceOptional() Report command as failed, Add EH command"));
            bool ehPending = CheckForPendingErrorHandlingCmd();
            if (ehPending)
            {
                // there should be no error handling queued.
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleDecNodeQueryInterfaceOptional() Already EH pending, should never happen"));
                return;
            }
            else
            {
                iCommandCompleteStatusInErrorHandling = cmdstatus;
                iCommandCompleteErrMsgInErrorHandling = NULL;
                AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_PREPARE, NULL, NULL, NULL, false);
            }
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleDecNodeQueryInterfaceOptional() Out"));
}


void PVPlayerEngine::HandleSourceNodeQueryDataSourcePosition(PVPlayerEngineContext& aNodeContext, const PVMFCmdResp& aNodeResp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeQueryDataSourcePosition() In"));

    PVMFTimestamp requesttime = iTargetNPT;

    if (aNodeResp.GetCmdStatus() == PVMFErrNotSupported || aNodeResp.GetCmdStatus() == PVMFErrArgument)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSourceNodeQueryDataSourcePosition() QueryDataSourcePosition failed. Assume position goes to requested position"));
    }
    else if (aNodeResp.GetCmdStatus() != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVPlayerEngine::HandleSourceNodeQueryDataSourcePosition() QueryDataSourcePosition failed, Add EH command"));
        bool ehPending = CheckForPendingErrorHandlingCmd();
        if (ehPending)
        {
            // there should be no error handling queued.
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSourceNodePause() Already EH pending, should never happen"));
            return;
        }
        PVMFErrorInfoMessageInterface* nextmsg = NULL;
        if (aNodeResp.GetEventExtensionInterface())
        {
            nextmsg = GetErrorInfoMessageInterface(*(aNodeResp.GetEventExtensionInterface()));
        }

        PVUuid puuid = PVPlayerErrorInfoEventTypesUUID;
        iCommandCompleteErrMsgInErrorHandling = OSCL_NEW(PVMFBasicErrorInfoMessage, (PVPlayerErrSourceFatal, puuid, nextmsg));
        iCommandCompleteStatusInErrorHandling = aNodeResp.GetCmdStatus();

        if (iState == PVP_ENGINE_STATE_PREPARING)
            AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_PREPARE, NULL, NULL, NULL, false);
        else if (iState == PVP_ENGINE_STATE_RESUMING)
            AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_RESUME, NULL, NULL, NULL, false);
        return;
    }
    else
    {
        PVMFNodeCapability nodeCapability;
        iSourceNode->GetCapability(nodeCapability);
        PVMFFormatType * formatType = nodeCapability.iInputFormatCapability.begin();
        bool mpeg4FormatType = false;
        if (formatType != NULL)
        {
            if ((pv_mime_strcmp((char*)formatType->getMIMEStrPtr(), PVMF_MIME_MPEG4FF)) == 0)
            {
                mpeg4FormatType = true;
            }
            else
            {
                mpeg4FormatType = false;
            }
        }

        if (mpeg4FormatType)
        {
            // Every thing is OK.. Calculate the modified target position depending upon nearest before and after syncPoints.
            // For MPEG4 files
            CalculateActualPlaybackPosition();
        }
    }

    // Determine the SetDataSourcePosition parameter based on query result and reposition settings
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0,
                    "PVPlayerEngine::HandleSourceNodeQueryDataSourcePosition()"
                    "Requested NPT %d, Modified Target NPT %d", requesttime, iTargetNPT));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iReposLogger, PVLOGMSG_INFO, (0,
                    "PVPlayerEngine::HandleSourceNodeQueryDataSourcePosition()"
                    "Requested NPT %d, Modified Target NPT %d", requesttime, iTargetNPT));

    uint32 startOfSeekWindow = 0;
    if (requesttime > iSyncPointSeekWindow)
    {
        startOfSeekWindow = (requesttime - iSyncPointSeekWindow);
    }
    uint32 endOfSeekWindow = requesttime + iSyncPointSeekWindow;

    // 1) Check if the modified target position falls within the window
    if ((iTargetNPT >= startOfSeekWindow) &&
            (iTargetNPT <= endOfSeekWindow))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO, (0,
                        "PVPlayerEngine::HandleSourceNodeQueryDataSourcePosition() - "
                        "RequestedNPT(%d) ModifiedTargetNPT(%d) is in the window (%d, %d), Seeking To %d",
                        requesttime, iTargetNPT, startOfSeekWindow, endOfSeekWindow, iTargetNPT));

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iReposLogger, PVLOGMSG_INFO, (0,
                        "PVPlayerEngine::HandleSourceNodeQueryDataSourcePosition() - "
                        "RequestedNPT(%d) ModifiedTargetNPT(%d) is in the window (%d, %d), Seeking To %d",
                        requesttime, iTargetNPT, startOfSeekWindow, endOfSeekWindow, iTargetNPT));

        requesttime = iTargetNPT;
    }
    else
    {
        // 1) Check if the actual seek point is before the window start, then set the
        // request time to start of the window
        // 2) Check if the actual seek point is after the window end, then
        // go back to start of the seek window
        // SFR is not really an option here since we are not playing yet, therefore always
        // go to start of the window
        if ((iTargetNPT < startOfSeekWindow) ||
                (iTargetNPT > endOfSeekWindow))
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO, (0,
                            "PVPlayerEngine::HandleSourceNodeQueryDataSourcePosition() - "
                            "RequestedNPT(%d) ModifiedTargetNPT(%d) is outside the window (%d, %d), Seeking To %d Seek-To-SyncPt False",
                            requesttime, iTargetNPT, startOfSeekWindow, endOfSeekWindow, startOfSeekWindow));

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iReposLogger, PVLOGMSG_INFO, (0,
                            "PVPlayerEngine::HandleSourceNodeQueryDataSourcePosition() - "
                            "RequestedNPT(%d) ModifiedTargetNPT(%d) is outside the window (%d, %d), Seeking To %d Seek-To-SyncPt False",
                            requesttime, iTargetNPT, startOfSeekWindow, endOfSeekWindow, startOfSeekWindow));

            requesttime = startOfSeekWindow;
            iTargetNPT = requesttime;
        }
        else
        {
            //error
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0,
                            "PVPlayerEngine::HandleSourceNodeQueryDataSourcePosition() - "
                            "RequestedNPT(%d) ModifiedTargetNPT(%d) window (%d, %d), Error Condition Asserting",
                            requesttime, iTargetNPT, startOfSeekWindow, endOfSeekWindow));
            OSCL_ASSERT(false);
        }
    }

    // Do the source positioning
    PVMFStatus retval = DoSourceNodeSetDataSourcePosition(aNodeContext.iCmdId, aNodeContext.iCmdContext);
    if (retval != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVPlayerEngine::HandleSourceNodeQueryDataSourcePosition() Report command as failed, Add EH command"));
        bool ehPending = CheckForPendingErrorHandlingCmd();
        if (ehPending)
        {
            // there should be no error handling queued.
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSourceNodeQueryDataSourcePosition() Already EH pending, should never happen"));
            return;
        }
        else
        {
            iCommandCompleteStatusInErrorHandling = retval;
            iCommandCompleteErrMsgInErrorHandling = NULL;
            if (iState == PVP_ENGINE_STATE_PREPARING)
                AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_PREPARE, NULL, NULL, NULL, false);
            else if (iState == PVP_ENGINE_STATE_RESUMING)
                AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_RESUME, NULL, NULL, NULL, false);
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeQueryDataSourcePosition() Out"));
}


void PVPlayerEngine::HandleSourceNodeSetDataSourcePosition(PVPlayerEngineContext& aNodeContext, const PVMFCmdResp& aNodeResp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeSetDataSourcePosition() In"));

    PVMFStatus cmdstatus = PVMFFailure;

    switch (aNodeResp.GetCmdStatus())
    {
        case PVMFErrArgument:
        case PVMFErrNotSupported:
        {
            if (iChangePlaybackPositionWhenResuming)
            {
                PVPPlaybackPosition curpos;
                curpos.iPosUnit = PVPPBPOSUNIT_MILLISEC;
                GetPlaybackClockPosition(curpos);
                uint32 clockcurpos = 0;
                bool tmpbool = false;
                iPlaybackClock.GetCurrentTime32(clockcurpos, tmpbool, PVMF_MEDIA_CLOCK_MSEC);

                // since repositioning is not supported and if the playback position change request was
                // issued during paused state, then continue from paused position.
                iChangePlaybackPositionWhenResuming = false;
                iWatchDogTimerInterval = 0;
                iActualNPT = curpos.iPosValue.millisec_value;
                iActualMediaDataTS = clockcurpos;
                iSkipMediaDataTS = clockcurpos;

                iStartNPT = iActualNPT;
                iStartMediaDataTS = iSkipMediaDataTS;

                // also decrement the stream id as no skip will be called on MIO node.
                --iStreamID;

                PVUuid puuid = PVPlayerErrorInfoEventTypesUUID;
                PVMFBasicErrorInfoMessage* infomsg = OSCL_NEW(PVMFBasicErrorInfoMessage, (PVPlayerInfoChangePlaybackPositionNotSupported, puuid, NULL));
                SendInformationalEvent(PVMFInfoChangePlaybackPositionNotSupported, OSCL_STATIC_CAST(PVInterface*, infomsg));
                infomsg->removeRef();
            }
            else
            {
                // This happens when we are in preparing state
                // Since this repositioning was not supported, assume the playback
                // will start from time 0
                iWatchDogTimerInterval = 0;
                iActualNPT = 0;
                iActualMediaDataTS = 0;
                iSkipMediaDataTS = 0;
                // Then continue to handle like success case
                iStartNPT = 0;
                iStartMediaDataTS = 0;
            }

            // Save the actual starting position for GetPlaybackRange() query
            iTargetNPT = iActualNPT;
            iCurrentBeginPosition.iPosValue.millisec_value = iActualNPT;
            iCurrentBeginPosition.iPosUnit = PVPPBPOSUNIT_MILLISEC;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVPlayerEngine::HandleSourceNodeSetDataSourcePosition() Requested begin position(%d ms) is not supported so start from prev location.",
                             iTargetNPT));
        }
        break;

        case PVMFSuccess:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVPlayerEngine::HandleSourceNodeSetDataSourcePosition() SetDataSourcePosition() successful. StartMediaTS %d ms, ActualNPT %d ms, TargetNPT %d ms",
                             iActualMediaDataTS, iActualNPT, iTargetNPT));
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iReposLogger, PVLOGMSG_INFO,
                            (0, "PVPlayerEngine::HandleSourceNodeSetDataSourcePosition() SetDataSourcePosition() successful. StartMediaTS %d ms, ActualNPT %d ms, TargetNPT %d ms",
                             iActualMediaDataTS, iActualNPT, iTargetNPT));
            // Compute the difference between actualNPT and targetNPT before any adjustments
            if (iTargetNPT >= iActualNPT)
            {
                iWatchDogTimerInterval = iTargetNPT - iActualNPT;
            }

            // Determine if adjustment needed to skip to requested time
            if (iSkipToRequestedPosition && (iActualNPT < iTargetNPT))
            {
                if (iTargetNPT - iActualNPT > iNodeDataQueuingTimeout)
                {
                    // Sync point seems to be far away in the stream
                    // Can't adjust the skip time back so use the returned values to skip to
                    iSkipMediaDataTS = iActualMediaDataTS;
                    iTargetNPT = iActualNPT;
                    iWatchDogTimerInterval = 0;
                    iCurrentBeginPosition.iPosValue.millisec_value = iActualNPT;
                }
                else
                {
                    //check if source node wants to override
                    uint32 startNPTFrmSource = iActualNPT;
                    if (iSourceNodePBCtrlIF->ComputeSkipTimeStamp(iTargetNPT,
                            iActualNPT,
                            iActualMediaDataTS,
                            iSkipMediaDataTS,
                            startNPTFrmSource) == PVMFSuccess)
                    {
                        iWatchDogTimerInterval = startNPTFrmSource - iActualNPT;
                        iActualNPT = startNPTFrmSource;
                        iTargetNPT = iActualNPT;
                        iCurrentBeginPosition.iPosValue.millisec_value = iActualNPT;
                    }
                    else
                    {
                        // Adjust the media data time to skip-to to correspond to the requested time
                        // Add the difference of target NPT with actual playback position in NPT to the actual media data time to get time to skip to.
                        iSkipMediaDataTS = iActualMediaDataTS + (iTargetNPT - iActualNPT);
                        iActualNPT = iTargetNPT;
                    }
                }
            }
            else
            {
                // Can't adjust the skip time back so use the returned values to skip to
                iSkipMediaDataTS = iActualMediaDataTS;
                iTargetNPT = iActualNPT;
                iCurrentBeginPosition.iPosValue.millisec_value = iActualNPT;
                iCurrentBeginPosition.iPosUnit = PVPPBPOSUNIT_MILLISEC;
                iWatchDogTimerInterval = 0;
            }

            // Save initial NTP and TS values
            iStartNPT = iActualNPT;
            iStartMediaDataTS = iSkipMediaDataTS;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVPlayerEngine::HandleSourceNodeSetDataSourcePosition() After adjustment StartMediaTS %d ms, AdjustedMediaTS %d ms, ActualPBPos %d ms Start NPT %d Start TS %d",
                             iActualMediaDataTS, iSkipMediaDataTS, iActualNPT, iStartNPT, iStartMediaDataTS));
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iReposLogger, PVLOGMSG_INFO,
                            (0, "PVPlayerEngine::HandleSourceNodeSetDataSourcePosition() After adjustment StartMediaTS %d ms, AdjustedMediaTS %d ms, ActualNPT %d ms StartNPT %d StartTS %d",
                             iActualMediaDataTS, iSkipMediaDataTS, iActualNPT, iStartNPT, iStartMediaDataTS));
        }
        break;

        default:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVPlayerEngine::HandleSourceNodeSetDataSourcePosition() failed, Add EH command"));
            bool ehPending = CheckForPendingErrorHandlingCmd();
            if (ehPending)
            {
                // there should be no error handling queued.
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSourceNodeSetDataSourcePosition() Already EH pending, should never happen"));
                return;
            }
            PVMFErrorInfoMessageInterface* nextmsg = NULL;
            if (aNodeResp.GetEventExtensionInterface())
            {
                nextmsg = GetErrorInfoMessageInterface(*(aNodeResp.GetEventExtensionInterface()));
            }

            PVUuid puuid = PVPlayerErrorInfoEventTypesUUID;
            iCommandCompleteErrMsgInErrorHandling = OSCL_NEW(PVMFBasicErrorInfoMessage, (PVPlayerErrSourceFatal, puuid, nextmsg));
            iCommandCompleteStatusInErrorHandling = aNodeResp.GetCmdStatus();

            if (iState == PVP_ENGINE_STATE_PREPARING)
                AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_PREPARE, NULL, NULL, NULL, false);
            else if (iState == PVP_ENGINE_STATE_RESUMING)
                AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_RESUME, NULL, NULL, NULL, false);

            return;
        }
    }
    // Repositioning so reset the EOS flag for each active datapath
    for (uint32 i = 0; i < iDatapathList.size(); ++i)
    {
        if (iDatapathList[i].iDatapath)
        {
            iDatapathList[i].iEndOfDataReceived = false;
        }
    }

    // Contine on and start the source node
    cmdstatus = DoSourceNodeStart(aNodeContext.iCmdId, aNodeContext.iCmdContext);
    if (cmdstatus != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSourceNodeSetDataSourcePosition() Report command as failed, Add EH command"));
        bool ehPending = CheckForPendingErrorHandlingCmd();
        if (ehPending)
        {
            // there should be no error handling queued.
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSourceNodeSetDataSourcePosition() Already EH pending, should never happen"));
            return;
        }
        else
        {
            iCommandCompleteStatusInErrorHandling = cmdstatus;
            iCommandCompleteErrMsgInErrorHandling = NULL;
            if (iState == PVP_ENGINE_STATE_PREPARING)
                AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_PREPARE, NULL, NULL, NULL, false);
            else if (iState == PVP_ENGINE_STATE_RESUMING)
                AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_RESUME, NULL, NULL, NULL, false);
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeSetDataSourcePosition() Out"));
}

void PVPlayerEngine::HandleSourceNodeSetDataSourceDirection(PVPlayerEngineContext& aNodeContext, const PVMFCmdResp& aNodeResp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeSetDataSourceDirection() In"));

    if (iChangePlaybackDirectionWhenResuming)
    {
        // Continuation of Engine Resume sequence.

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeSetDataSourceDirection() Context RESUME"));
        PVMFStatus cmdstatus = PVMFFailure;

        switch (aNodeResp.GetCmdStatus())
        {
            case PVMFErrArgument:
            case PVMFErrNotSupported:
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVPlayerEngine::HandleSourceNodeSetDataSourceDirection() Requested direction is not supported!"));
                PVUuid puuid = PVPlayerErrorInfoEventTypesUUID;
                PVMFBasicErrorInfoMessage* infomsg = OSCL_NEW(PVMFBasicErrorInfoMessage, (PVPlayerInfoChangePlaybackPositionNotSupported, puuid, NULL));
                SendInformationalEvent(PVMFInfoChangePlaybackPositionNotSupported, OSCL_STATIC_CAST(PVInterface*, infomsg));
                infomsg->removeRef();
            }
            break;

            case PVMFSuccess:
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "PVPlayerEngine::HandleSourceNodeSetDataSourceDirection() SetDataSourceDirection() successful. StartMediaTS %d ms, ActualPBPos %d ms",
                                 iActualMediaDataTS, iActualNPT));

                //there's no adjustment to the media TS here.
                iSkipMediaDataTS = iActualMediaDataTS;
                iCurrentBeginPosition.iPosValue.millisec_value = iActualNPT;
                iCurrentBeginPosition.iPosUnit = PVPPBPOSUNIT_MILLISEC;

                //Install the new direction and get the repositioning target.
                UpdateDirection(iActualNPT, iSkipMediaDataTS, iChangeDirectionNPT);

                //Reposition the source to the desired playback time
                if (!iChangeDirectionNPT.iIndeterminate)
                {
                    iChangePlaybackDirectionWhenResuming = false;
                    iChangePlaybackPositionWhenResuming = true;
                    PVPlayerEngineCommand cmd(0, aNodeContext.iCmdId, aNodeContext.iCmdContext, NULL, false);
                    iCurrentBeginPosition = iChangeDirectionNPT;
                    PVMFStatus retval = UpdateCurrentBeginPosition(iCurrentBeginPosition, cmd);
                    if (retval == PVMFPending)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeSetDataSourceDirection() Repos to %d started", iChangeDirectionNPT.iPosValue));
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeSetDataSourceDirection() Out"));
                        return;//wait on the repos sequence...
                    }
                    else if (retval != PVMFSuccess)
                    {
                        //else can't reposition, ignore failure and continue.
                        iChangeDirectionNPT.iIndeterminate = true;
                        iChangePlaybackPositionWhenResuming = false;
                        //need to leave the flag set for later in HandleDatapathResume,
                        //to trigger the skip media data.
                        iChangePlaybackDirectionWhenResuming = true;
                    }
                }
            }
            break;

            default:
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVPlayerEngine::HandleSourceNodeSetDataSourceDirection() failed, Add EH command"));
                bool ehPending = CheckForPendingErrorHandlingCmd();
                if (ehPending)
                {
                    // there should be no error handling queued.
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSourceNodeSetDataSourceDirection() Already EH pending, should never happen"));
                    return;
                }
                PVMFErrorInfoMessageInterface* nextmsg = NULL;
                if (aNodeResp.GetEventExtensionInterface())
                {
                    nextmsg = GetErrorInfoMessageInterface(*(aNodeResp.GetEventExtensionInterface()));
                }

                PVUuid puuid = PVPlayerErrorInfoEventTypesUUID;
                iCommandCompleteErrMsgInErrorHandling = OSCL_NEW(PVMFBasicErrorInfoMessage, (PVPlayerErrSourceFatal, puuid, nextmsg));
                iCommandCompleteStatusInErrorHandling = aNodeResp.GetCmdStatus();
                AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_RESUME, NULL, NULL, NULL, false);
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeSetDataSourceDirection() Out"));
                return;
            }
        }


        // Repositioning so reset the EOS flag for each active datapath
        for (uint32 i = 0; i < iDatapathList.size(); ++i)
        {
            if (iDatapathList[i].iDatapath)
            {
                iDatapathList[i].iEndOfDataReceived = false;
            }
        }

        // Start the source node.
        cmdstatus = DoSourceNodeStart(aNodeContext.iCmdId, aNodeContext.iCmdContext);
        if (cmdstatus != PVMFSuccess)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVPlayerEngine::HandleSourceNodeSetDataSourcePosition() Report command as failed, Add EH command"));
            bool ehPending = CheckForPendingErrorHandlingCmd();
            if (ehPending)
            {
                // there should be no error handling queued.
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSourceNodeSetDataSourceDirection() Already EH pending, should never happen"));
                return;
            }
            else
            {
                iCommandCompleteStatusInErrorHandling = cmdstatus;
                iCommandCompleteErrMsgInErrorHandling = NULL;
                AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_RESUME, NULL, NULL, NULL, false);
            }
        }
    }
    else
    {
        //Continuation of SetPlaybackRate sequence.

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeSetDataSourceDirection() Context SETPLAYBACKRATE"));

        if (aNodeResp.GetCmdStatus() != PVMFSuccess)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSourceNodeSetDataSourceDirection() SetDataSourceDirection failed. Playback position change has been cancelled"));

            if (aNodeResp.GetCmdStatus() == PVMFErrNotSupported || aNodeResp.GetCmdStatus() == PVMFErrArgument)
            {
                // For non-fatal error, continue playback by resuming the clock
                StartPlaybackClock();
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVPlayerEngine::HandleSourceNodeSetDataSourceDirection() failed, Add EH command"));
                // Initiate error handling
                bool ehPending = CheckForPendingErrorHandlingCmd();
                if (ehPending)
                {
                    // there should be no error handling queued.
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSourceNodeSetDataSourceDirection() Already EH pending, should never happen"));
                    return;
                }
                PVMFErrorInfoMessageInterface* nextmsg = NULL;
                if (aNodeResp.GetEventExtensionInterface())
                {
                    nextmsg = GetErrorInfoMessageInterface(*(aNodeResp.GetEventExtensionInterface()));
                }

                PVUuid puuid = PVPlayerErrorInfoEventTypesUUID;
                iCommandCompleteErrMsgInErrorHandling = OSCL_NEW(PVMFBasicErrorInfoMessage, (PVPlayerErrSourceFatal, puuid, nextmsg));
                iCommandCompleteStatusInErrorHandling = aNodeResp.GetCmdStatus();
                AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_SET_PLAYBACK_RATE, NULL, NULL, NULL, false);
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeSetDataSourceDirection() Out"));
                return;
            }

            PVMFErrorInfoMessageInterface* nextmsg = NULL;
            if (aNodeResp.GetEventExtensionInterface() != NULL)
            {
                nextmsg = GetErrorInfoMessageInterface(*(aNodeResp.GetEventExtensionInterface()));
            }
            PVUuid puuid = PVPlayerErrorInfoEventTypesUUID;
            PVMFBasicErrorInfoMessage* errmsg = OSCL_NEW(PVMFBasicErrorInfoMessage, (PVPlayerErrSource, puuid, nextmsg));

            // Complete the SetPlaybackRate() command as failed
            EngineCommandCompleted(aNodeContext.iCmdId, aNodeContext.iCmdContext, aNodeResp.GetCmdStatus(), OSCL_STATIC_CAST(PVInterface*, errmsg));

            errmsg->removeRef();
            errmsg = NULL;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeSetDataSourceDirection() Out"));
            return;
        }

        // no adjustement here.
        iSkipMediaDataTS = iActualMediaDataTS;
        iCurrentBeginPosition.iPosValue.millisec_value = iActualNPT;
        iCurrentBeginPosition.iPosUnit = PVPPBPOSUNIT_MILLISEC;

        //Install the new direction and get the repositioning target
        UpdateDirection(iActualNPT, iSkipMediaDataTS, iChangeDirectionNPT);

        //Launch a repositioning sequence now.
        if (!iChangeDirectionNPT.iIndeterminate)
        {
            PVPlayerEngineCommand cmd(0, aNodeContext.iCmdId, aNodeContext.iCmdContext, NULL, false);
            iCurrentBeginPosition = iChangeDirectionNPT;
            PVMFStatus retval = UpdateCurrentBeginPosition(iCurrentBeginPosition, cmd);
            if (retval == PVMFPending)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeSetDataSourceDirection() Repos to %d started", iChangeDirectionNPT.iPosValue));
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeSetDataSourceDirection() Out"));
                return;//wait on the repos sequence...
            }
            else if (retval != PVMFSuccess)
            {
                //the direction is already changed, so just ignore this failure and continue
                iChangeDirectionNPT.iIndeterminate = true;
            }
        }

        // Repositioning so reset the EOS flag for each active datapath
        for (uint32 i = 0; i < iDatapathList.size(); ++i)
        {
            if (iDatapathList[i].iDatapath)
            {
                iDatapathList[i].iEndOfDataReceived = false;
            }
        }

        // Skip to the new source node position, so that all the data that was queued
        // when the command was received will get flushed.

        PVMFStatus retval = DoSinkNodeSkipMediaDataDuringPlayback(aNodeContext.iCmdId, aNodeContext.iCmdContext);
        if (retval != PVMFSuccess)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSourceNodeSetDataSourceDirection() Skipping media data request in sink nodes failed. Repositioning did not complete."));
            PVUuid puuid = PVPlayerErrorInfoEventTypesUUID;
            PVMFBasicErrorInfoMessage* errmsg = OSCL_NEW(PVMFBasicErrorInfoMessage, (PVPlayerErrSink, puuid, NULL));

            //clear the pending direction change NPT.
            iChangeDirectionNPT.iIndeterminate = true;

            // Complete the SetPlaybackRate() command as failed
            EngineCommandCompleted(aNodeContext.iCmdId, aNodeContext.iCmdContext, retval, OSCL_STATIC_CAST(PVInterface*, errmsg));

            errmsg->removeRef();
            errmsg = NULL;
        }
        // else wait on HandleSinkNodeSkipMediaDataDuringPlayback.
    }
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeSetDataSourceDirection() Out"));
}

void PVPlayerEngine::HandleSourceNodeStart(PVPlayerEngineContext& aNodeContext, const PVMFCmdResp& aNodeResp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_INFO,
                    (0, "PVPlayerEngine::HandleSourceNodeStart() Tick=%d", OsclTickCount::TickCount()));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeStart() In"));

    PVMFStatus cmdstatus = PVMFErrNotSupported;

    switch (aNodeResp.GetCmdStatus())
    {
        case PVMFSuccess:
        {
            // Issue Skip on Sink Node and Start on datapaths back to back. This is done to make sure that
            // sink node is started only after discarding the data from an earlier stream.
            cmdstatus = DoSinkNodeSkipMediaData(aNodeContext.iCmdId, aNodeContext.iCmdContext);
            if (cmdstatus != PVMFSuccess)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSourceNodeStart() Skip of sink node did a leave, asserting"));
                OSCL_ASSERT(false);
            }

            // Start the available datapaths
            iNumPendingDatapathCmd = 0;
            for (uint32 i = 0; i < iDatapathList.size(); ++i)
            {
                if (iDatapathList[i].iDatapath)
                {
                    PVMFStatus retval = DoDatapathStart(iDatapathList[i], aNodeContext.iCmdId, aNodeContext.iCmdContext);
                    if (retval == PVMFSuccess)
                    {
                        ++iNumPendingDatapathCmd;
                        cmdstatus = PVMFSuccess;
                    }
                    else
                    {
                        cmdstatus = retval;
                        break;
                    }
                }
            }
            if (iNumPendingDatapathCmd == 0)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVPlayerEngine:HandleSourceNodeStart() DoDatapathStart failed, Add EH command"));
                bool ehPending = CheckForPendingErrorHandlingCmd();
                if (ehPending)
                {
                    // there should be no error handling queued.
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine:HandleSourceNodeStart() Already EH pending, should never happen"));
                    return;
                }
                iCommandCompleteStatusInErrorHandling = cmdstatus;
                iCommandCompleteErrMsgInErrorHandling = NULL;
                AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_PREPARE, NULL, NULL, NULL, false);
            }
        }
        break;

        default:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVPlayerEngine:HandleSourceNodeStart() failed, Add EH command"));
            bool ehPending = CheckForPendingErrorHandlingCmd();
            if (ehPending)
            {
                // there should be no error handling queued.
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine:HandleSourceNodeStart() Already EH pending, should never happen"));
                return;
            }
            PVMFErrorInfoMessageInterface* nextmsg = NULL;
            if (aNodeResp.GetEventExtensionInterface())
            {
                nextmsg = GetErrorInfoMessageInterface(*(aNodeResp.GetEventExtensionInterface()));
            }

            PVUuid puuid = PVPlayerErrorInfoEventTypesUUID;
            iCommandCompleteErrMsgInErrorHandling = OSCL_NEW(PVMFBasicErrorInfoMessage, (PVPlayerErrSourceFatal, puuid, nextmsg));
            iCommandCompleteStatusInErrorHandling = aNodeResp.GetCmdStatus();
            AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_PREPARE, NULL, NULL, NULL, false);
        }
        break;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeStart() Out"));
}


void PVPlayerEngine::HandleSinkNodeSkipMediaData(PVPlayerEngineContext& aNodeContext, const PVMFCmdResp& aNodeResp)
{
    OSCL_UNUSED_ARG(aNodeContext);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_INFO,
                    (0, "PVPlayerEngine::HandleSinkNodeSkipMediaData() for %s Tick=%d",
                     aNodeContext.iEngineDatapath->iTrackInfo->getTrackMimeType().get_cstr(), OsclTickCount::TickCount()));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iReposLogger, PVLOGMSG_INFO,
                    (0, "PVPlayerEngine::HandleSinkNodeSkipMediaData() for %s, iNumPVMFInfoStartOfDataPending=%d",
                     aNodeContext.iEngineDatapath->iTrackInfo->getTrackMimeType().get_cstr(), iNumPVMFInfoStartOfDataPending));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVPlayerEngine::HandleSinkNodeSkipMediaData() In"));

    --iNumPendingNodeCmd;

    if (aNodeResp.GetCmdStatus() != PVMFSuccess)
    {
        // Sink node report error with SkipMediaData()
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSinkNodeSkipMediaData() Sink node report error for SkipMediaData(). Asserting"));
        OSCL_ASSERT(false);
    }

    --iNumPendingSkipCompleteEvent;

    if (iNumPendingNodeCmd == 0)
    {
        // We dont start the playback clock here since engine is waiting for Start on Sink nodes to complete
        // This will also check the order in which Sink Node completes the command. Sinks should complete Skip
        // first and then Start, therefore iNumPendingDatapathCmd should always be greater than zero when
        // this happens. If not just assert.
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iReposLogger, PVLOGMSG_INFO,
                        (0, "PVPlayerEngine::HandleSinkNodeSkipMediaData() Skip Complete"));
    }

    if ((iNumPendingSkipCompleteEvent == 0) && (iNumPVMFInfoStartOfDataPending == 0))
    {
        // we have received all the bos event so cancel the watchDogTimer if any.
        if (iWatchDogTimer->IsBusy())
        {
            iWatchDogTimer->Cancel();
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iReposLogger, PVLOGMSG_INFO, (0, "PVPlayerEngine::HandleSinkNodeSkipMediaData - WatchDogTimer cancelled"));
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSinkNodeSkipMediaData() Out"));
}


void PVPlayerEngine::HandleSourceNodeQueryDataSourcePositionDuringPlayback(PVPlayerEngineContext& aNodeContext, const PVMFCmdResp& aNodeResp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeQueryDataSourcePositionDuringPlayback() In"));

    PVMFTimestamp requesttime = iTargetNPT;
    if (aNodeResp.GetCmdStatus() != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSourceNodeQueryDataSourcePositionDuringPlayback() QueryDataSourcePosition failed. Assume position goes to requested position"));
    }
    else
    {
        PVMFNodeCapability nodeCapability;
        iSourceNode->GetCapability(nodeCapability);
        PVMFFormatType * formatType = nodeCapability.iInputFormatCapability.begin();
        bool mpeg4FormatType = false;
        if (formatType != NULL)
        {
            if ((pv_mime_strcmp((char*)formatType->getMIMEStrPtr(), PVMF_MIME_MPEG4FF)) == 0)
            {
                mpeg4FormatType = true;
            }
            else
            {
                mpeg4FormatType = false;
            }
        }
        if (mpeg4FormatType)
        {
            // Every thing is OK.. Calculate the modified target position depending upon nearest before and after syncPoints.
            // For MPEG4 files
            CalculateActualPlaybackPosition();
        }
    }

    // Determine the SetDataSourcePosition parameter based on query result and reposition settings
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0,
                    "PVPlayerEngine::HandleSourceNodeQueryDataSourcePositionDuringPlayback()"
                    "Requested NPT %d, ModifiedTarget NPT %d", requesttime, iTargetNPT));

    uint32 startOfSeekWindow = 0;
    if (requesttime > iSyncPointSeekWindow)
    {
        startOfSeekWindow = (requesttime - iSyncPointSeekWindow);
    }
    uint32 endOfSeekWindow = requesttime + iSyncPointSeekWindow;

    PVPPlaybackPosition curpos;
    curpos.iPosUnit = PVPPBPOSUNIT_MILLISEC;
    GetPlaybackClockPosition(curpos);
    bool oSFR = false;

    //depending on whether it is fwd or rwnd, the window is different
    //if doing a rwnd, the worst case window is (0, currentplaybackposition)
    //if doing a fwd, the worst case window is (currentplaybackposition, endofclip)
    if (requesttime <= curpos.iPosValue.millisec_value)
    {
        //requested pos <= currpos => rwnd
        //cap end of seek window to be the current play back pos
        endOfSeekWindow = curpos.iPosValue.millisec_value;
    }
    if (requesttime > curpos.iPosValue.millisec_value)
    {
        //requested pos > currpos => fwd
        //cap start of seek window to be the current play back pos
        startOfSeekWindow = curpos.iPosValue.millisec_value;
    }

    // 1) Check if the Modified target position falls within the window
    if ((iTargetNPT >= startOfSeekWindow) &&
            (iTargetNPT <= endOfSeekWindow))
    {
        // Check for SFR
        // In case if actual playback position
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO, (0,
                        "PVPlayerEngine::HandleSourceNodeQueryDataSourcePositionDuringPlayback() - "
                        "RequestedNPT(%d) ModifiedTargetNPT(%d) is in the window (%d, %d), Seeking To %d",
                        requesttime, iTargetNPT, startOfSeekWindow, endOfSeekWindow, iTargetNPT));

        requesttime = iTargetNPT;
    }
    else
    {
        // Check for SFR
        // SFR means currplaybackpos < requestedpos
        if (curpos.iPosValue.millisec_value < requesttime)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO, (0,
                            "PVPlayerEngine::HandleSourceNodeQueryDataSourcePositionDuringPlayback() - "
                            "CurrNPT(%d) less than RequestedNPT(%d) Ignoring ModifiedTargetNPT(%d) and the window (%d, %d), Doing SFR",
                            curpos.iPosValue.millisec_value, requesttime, iTargetNPT, startOfSeekWindow, endOfSeekWindow, startOfSeekWindow));

            oSFR = true;

            PVUuid puuid = PVPlayerErrorInfoEventTypesUUID;
            PVMFBasicErrorInfoMessage* infomsg = OSCL_NEW(PVMFBasicErrorInfoMessage, (PVPlayerInfoAttemptingSFRAsPartOfSetPlayBackRange, puuid, NULL));
            SendInformationalEvent(PVMFInfoPositionStatus, OSCL_STATIC_CAST(PVInterface*, infomsg));
            infomsg->removeRef();

        }
        else
        {
            // if the actual seek point is before the window start,
            // or if the actual seek point is after the window end,
            // then go back to start of the seek window only in case of a finite window
            // in case of infinite window just go to the requested position and do normal
            // repositioning
            if (iSyncPointSeekWindow == 0x7FFFFFFF)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO, (0,
                                "PVPlayerEngine::HandleSourceNodeQueryDataSourcePositionDuringPlayback() - "
                                "RequestedNPT(%d) ModifiedTargetNPT(%d) is outside the window (%d, %d), Seeking To %d Seek-To-SyncPt True",
                                requesttime, iTargetNPT, startOfSeekWindow, endOfSeekWindow, startOfSeekWindow));

                iTargetNPT = requesttime;
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO, (0,
                                "PVPlayerEngine::HandleSourceNodeQueryDataSourcePositionDuringPlayback() - "
                                "RequestedNPT(%d) ModifiedTargetNPT(%d) is outside the window (%d, %d), Seeking To %d Seek-To-SyncPt False",
                                requesttime, iTargetNPT, startOfSeekWindow, endOfSeekWindow, startOfSeekWindow));

                requesttime = startOfSeekWindow;
                iTargetNPT = requesttime;
            }
        }
    }

    if (oSFR)
    {
        // No need to change source position so go to skipping at sink nodes
        // First determine to what time sink nodes should skip to
        // Get current playback clock position in media data time
        uint32 clockcurpos = 0;
        bool tmpbool = false;
        iPlaybackClock.GetCurrentTime32(clockcurpos, tmpbool, PVMF_MEDIA_CLOCK_MSEC);

        // for SFR since the source node is bypassed, there will be no frames generated with new
        // Stream ID so, for skip to complete on Sink Node, Stream ID needs to be decremented. As
        // there will be no new Stream in case of SFR.
        --iStreamID;

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVPlayerEngine::HandleSourceNodeQueryDataSourcePositionDuringPlayback() New source reposition before current position so no need to change source position."));

        if (iSkipToRequestedPosition)
        {
            // Skip to the requested begin position
            // Add the difference of target NPT with current time in NPT to the current clock to get media data time to skip to.
            iActualMediaDataTS = clockcurpos;
            iSkipMediaDataTS = (requesttime - curpos.iPosValue.millisec_value) + clockcurpos;
            iActualNPT = requesttime;
            iWatchDogTimerInterval = requesttime - curpos.iPosValue.millisec_value;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVPlayerEngine::HandleSourceNodeQueryDataSourcePositionDuringPlayback() Skip-to-requested position SET. ActualNPT=%d, ActualMediaTS=%d, AdjustedMediaTS=%d",
                             iActualNPT, iActualMediaDataTS, iSkipMediaDataTS));
        }
        else
        {
            // Just continue playback from current position
            iActualMediaDataTS = clockcurpos;
            iSkipMediaDataTS = clockcurpos;
            iActualNPT = curpos.iPosValue.millisec_value;
            iWatchDogTimerInterval = 0;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVPlayerEngine::HandleSourceNodeQueryDataSourcePositionDuringPlayback() Skip-to-requested position NOT SET so continue playing. ActualNPT=%d, ActualMediaTS=%d, AdjustedMediaTS=%d",
                             iActualNPT, iActualMediaDataTS, iSkipMediaDataTS));
        }

        PVMFStatus retval = DoSinkNodeSkipMediaDataDuringPlayback(aNodeContext.iCmdId, aNodeContext.iCmdContext, true);
        if (retval != PVMFSuccess)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSourceNodeQueryDataSourcePositionDuringPlayback() Skipping media data request in sink nodes failed. Repositioning did not complete."));
            PVUuid puuid = PVPlayerErrorInfoEventTypesUUID;
            PVMFBasicErrorInfoMessage* errmsg = OSCL_NEW(PVMFBasicErrorInfoMessage, (PVPlayerErrSink, puuid, NULL));

            //clear the pending direction change NPT.
            iChangeDirectionNPT.iIndeterminate = true;

            // Complete the SetPlaybackRange() command as failed
            EngineCommandCompleted(aNodeContext.iCmdId, aNodeContext.iCmdContext, retval, OSCL_STATIC_CAST(PVInterface*, errmsg));

            errmsg->removeRef();
            errmsg = NULL;
        }

    }
    else
    {
        // Do the source positioning
        PVMFStatus retval = DoSourceNodeSetDataSourcePositionDuringPlayback(aNodeContext.iCmdId, aNodeContext.iCmdContext);
        if (retval != PVMFSuccess)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSourceNodeQueryDataSourcePositionDuringPlayback() SetDataSourcePosition failed. Playback position change has been cancelled"));
            PVUuid puuid = PVPlayerErrorInfoEventTypesUUID;
            PVMFBasicErrorInfoMessage* errmsg = OSCL_NEW(PVMFBasicErrorInfoMessage, (PVPlayerErrSource, puuid, NULL));

            // Complete the SetPlaybackRange() command as failed
            EngineCommandCompleted(aNodeContext.iCmdId, aNodeContext.iCmdContext, retval, OSCL_STATIC_CAST(PVInterface*, errmsg));

            errmsg->removeRef();
            errmsg = NULL;
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeQueryDataSourcePositionDuringPlayback() Out"));
}

void PVPlayerEngine::CalculateActualPlaybackPosition()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::CalculateActualPlaybackPosition In"));

    PVPPlaybackPosition curpos;
    curpos.iPosUnit = PVPPBPOSUNIT_MILLISEC;
    GetPlaybackClockPosition(curpos);

    // Following code has been taken from MP4 parser node, all the vars are kept very near to the MP4 parser node.
    // Previously the calculation of before and after sync point was done in MP4 parser node.

    if (curpos.iPosValue.millisec_value > iTargetNPT)
    {
        // curpos.iPosValue.millisec_value was passed as iActualNPT in QueryDataSourcePosition
        // which became aActualNPT while collection, and used to decide forward and reverse repos.
        iBackwardReposFlag = true;
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::CalculateActualPlaybackPosition In: Backward Reposition"));
    }
    else
    {
        iForwardReposFlag = true;
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::CalculateActualPlaybackPosition In: Forward Reposition"));
    }

    // pick the closest time to targetNPT
    uint32 delta = 0;
    uint32 diffBetSeekPointBeforeAndTarget = 0;
    if (PVTimeComparisonUtils::IsEarlier(iSeekPointBeforeTargetNPT, iTargetNPT, delta))
    {
        // this should always be true when checking the SeekPointBefore with
        // targetNPT.
        diffBetSeekPointBeforeAndTarget = delta;
        delta = 0;
    }
    else
    {
        // this will only happen when mp4ff library returns an SeekPointBefore which
        // is after targetNPT with small delta because of some rounding off error in
        // media clock converter class.
        diffBetSeekPointBeforeAndTarget = delta;
        delta = 0;
    }

    uint32 diffBetSeekPointAfterAndTarget = 0;
    if (PVTimeComparisonUtils::IsEarlier(iTargetNPT, iSeekPointAfterTargetNPT, delta))
    {
        // this should always be true when checking the SeekPointAfter with
        // targetNPT.
        diffBetSeekPointAfterAndTarget = delta;
        delta = 0;
    }
    else
    {
        // this should never happen.
        diffBetSeekPointAfterAndTarget = delta;
        delta = 0;
    }

    // modify the target NPT and set it to the closest I-frame returned by parser node.
    if (diffBetSeekPointAfterAndTarget < diffBetSeekPointBeforeAndTarget)
    {
        iTargetNPT = iSeekPointAfterTargetNPT;
    }
    else
    {
        if (iSeekPointBeforeTargetNPT < curpos.iPosValue.millisec_value && iForwardReposFlag)
        {
            iTargetNPT = iSeekPointAfterTargetNPT;
            iForwardReposFlag = false;
        }
        else
        {
            iTargetNPT = iSeekPointBeforeTargetNPT;
            iForwardReposFlag = false;
        }
    }
    if (iBackwardReposFlag) // To avoid backwardlooping :: A flag to remember backward repositioning
    {
        iTargetNPT = iSeekPointBeforeTargetNPT;
        iBackwardReposFlag = false;
    }


    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0,
                    "PVPlayerEngine::CalculateActualPlaybackPosition()"
                    "targetNPT %d Current NPT %d, Modified Target NPT %d, SeekPointBeforeTargetNPT %d, SeekPointAfterTargetNPT %d ",
                    iCurrentBeginPosition.iPosValue.millisec_value, curpos.iPosValue.millisec_value, iTargetNPT, iSeekPointBeforeTargetNPT, iSeekPointAfterTargetNPT));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::CalculateActualPlaybackPosition Out"));
}

void PVPlayerEngine::HandleSourceNodeSetDataSourcePositionDuringPlayback(PVPlayerEngineContext& aNodeContext, const PVMFCmdResp& aNodeResp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeSetDataSourcePositionDuringPlayback() In"));

    if (aNodeResp.GetCmdStatus() != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSourceNodeSetDataSourcePositionDuringPlayback() SetDataSourcePosition failed. Playback position change has been cancelled"));

        if (aNodeResp.GetCmdStatus() == PVMFErrNotSupported || aNodeResp.GetCmdStatus() == PVMFErrArgument)
        {
            PVPPlaybackPosition curpos;
            curpos.iPosUnit = PVPPBPOSUNIT_MILLISEC;
            GetPlaybackClockPosition(curpos);
            uint32 clockcurpos = 0;
            bool tmpbool = false;
            iPlaybackClock.GetCurrentTime32(clockcurpos, tmpbool, PVMF_MEDIA_CLOCK_MSEC);

            // since repositioning is not supported continue playing from current position.
            iWatchDogTimerInterval = 0;
            iActualNPT = curpos.iPosValue.millisec_value;
            iActualMediaDataTS = clockcurpos;
            iSkipMediaDataTS = clockcurpos;

            iStartNPT = iActualNPT;
            iStartMediaDataTS = iSkipMediaDataTS;

            // also decrement the stream id as no skip will be called on MIO node.
            --iStreamID;

            // For non-fatal error, continue playback by resuming the clock
            StartPlaybackClock();
            // Complete the SetPlaybackRange() command as notsupported / failed
            EngineCommandCompleted(aNodeContext.iCmdId, aNodeContext.iCmdContext, aNodeResp.GetCmdStatus());
        }
        else
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVPlayerEngine::HandleSourceNodeSetDataSourcePositionDuringPlayback() failed, Add EH command"));
            bool ehPending = CheckForPendingErrorHandlingCmd();
            if (ehPending)
            {
                // there should be no error handling queued.
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVPlayerEngine::HandleSourceNodeSetDataSourcePositionDuringPlayback() Already EH pending, should never happen"));
                return;
            }
            PVMFErrorInfoMessageInterface* nextmsg = NULL;
            if (aNodeResp.GetEventExtensionInterface())
            {
                nextmsg = GetErrorInfoMessageInterface(*(aNodeResp.GetEventExtensionInterface()));
            }

            PVUuid puuid = PVPlayerErrorInfoEventTypesUUID;
            iCommandCompleteErrMsgInErrorHandling = OSCL_NEW(PVMFBasicErrorInfoMessage, (PVPlayerErrSourceFatal, puuid, nextmsg));
            iCommandCompleteStatusInErrorHandling = aNodeResp.GetCmdStatus();
            AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_SET_PLAYBACK_RANGE, NULL, NULL, NULL, false);
        }
        return;
    }

    if ((iCurrentBeginPosition.iMode != PVPPBPOS_MODE_END_OF_CURRENT_PLAY_ELEMENT) &&
            (iCurrentBeginPosition.iMode != PVPPBPOS_MODE_END_OF_CURRENT_PLAY_SESSION))
    {
        if (iCurrentBeginPosition.iPosUnit == PVPPBPOSUNIT_PLAYLIST)
        {
            iActualMediaDataTS = iDataSourcePosParams.iActualMediaDataTS;
            iActualNPT = iDataSourcePosParams.iActualNPT;
        }

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVPlayerEngine::HandleSourceNodeSetDataSourcePositionDuringPlayback() SetDataSourcePosition() successful. StartMediaTS %d ms, ActualPBPos %d ms",
                         iActualMediaDataTS, iActualNPT));

        if (iCurrentBeginPosition.iPosUnit == PVPPBPOSUNIT_PLAYLIST)
        {
            if (iTargetNPT >= iActualNPT)
            {
                iWatchDogTimerInterval = iTargetNPT - iActualNPT;
            }
        }
        // Compute the difference between actualNPT and targetNPT before any adjustments
        else if (iTargetNPT >= iActualNPT)
        {
            iWatchDogTimerInterval = iTargetNPT - iActualNPT;
        }

        //iCurrentBeginPosition.iPosUnit has served its purpose, it is ok if it is overwritten
        if (iSkipToRequestedPosition && (iActualNPT < iTargetNPT))
        {
            if (iTargetNPT - iActualNPT >= iNodeDataQueuingTimeout)
            {
                // Can't adjust the skip time back so use the returned values to skip to
                iSkipMediaDataTS = iActualMediaDataTS;
                iActualNPT = iTargetNPT;
                iWatchDogTimerInterval = 0;
            }
            else
            {
                //check if source node wants to override
                uint32 startNPTFrmSource = iActualNPT;
                if (iSourceNodePBCtrlIF->ComputeSkipTimeStamp(iTargetNPT,
                        iActualNPT,
                        iActualMediaDataTS,
                        iSkipMediaDataTS,
                        startNPTFrmSource) == PVMFSuccess)
                {
                    iWatchDogTimerInterval = startNPTFrmSource - iActualNPT;
                    iActualNPT = startNPTFrmSource;
                }
                else
                {
                    // Adjust the media data time to skip-to to correspond to the requested time
                    // Add the difference of target NPT with actual playback position in NPT to the actual media data time to get time to skip to.
                    iSkipMediaDataTS = iActualMediaDataTS + (iTargetNPT - iActualNPT);
                    // Set the actual playback position to the requested time since actual media data TS was adjusted
                    // This is important since the difference between the two is used to calculate the NPT to media data offset in HandleSinkNodeskipMediaDataDuringPlayback()
                    iActualNPT = iTargetNPT;
                }

            }
            iCurrentBeginPosition.iPosUnit = PVPPBPOSUNIT_MILLISEC;
        }
        else
        {
            // Can't adjust the skip time back so just use the returned values to skip to
            iSkipMediaDataTS = iActualMediaDataTS;
            iTargetNPT = iActualNPT;
            iCurrentBeginPosition.iPosValue.millisec_value = iActualNPT;
            iCurrentBeginPosition.iPosUnit = PVPPBPOSUNIT_MILLISEC;
            iWatchDogTimerInterval = 0;
        }

        uint32 clockcurpos = 0;
        bool tmpbool;
        // Get current playback clock position
        iPlaybackClock.GetCurrentTime32(clockcurpos, tmpbool, PVMF_MEDIA_CLOCK_MSEC);

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVPlayerEngine::HandleSourceNodeSetDataSourcePositionDuringPlayback() After adjustment StartMediaTS %d ms, AdjustedMediaTS %d ms, ActualPBPos %d ms Clock %d ms",
                         iActualMediaDataTS, iSkipMediaDataTS, iActualNPT, clockcurpos));

        // Repositioning so reset the EOS flag for each active datapath
        for (uint32 i = 0; i < iDatapathList.size(); ++i)
        {
            if (iDatapathList[i].iDatapath)
            {
                iDatapathList[i].iEndOfDataReceived = false;
            }
        }

        PVMFStatus retval = DoSinkNodeSkipMediaDataDuringPlayback(aNodeContext.iCmdId, aNodeContext.iCmdContext);
        if (retval != PVMFSuccess)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSourceNodeSetDataSourcePositionDuringPlayback() Skipping media data request in sink nodes failed. Repositioning did not complete."));

            // clear the pending direction change NPT.
            iChangeDirectionNPT.iIndeterminate = true;

            // Complete the SetPlaybackRange() command as failed
            EngineCommandCompleted(aNodeContext.iCmdId, aNodeContext.iCmdContext, retval);
        }
    }
    else
    {
        PVMFErrorInfoMessageInterface* nextmsg = NULL;
        if (aNodeResp.GetEventExtensionInterface() != NULL)
        {
            nextmsg = GetErrorInfoMessageInterface(*(aNodeResp.GetEventExtensionInterface()));
        }
        PVUuid puuid = PVPlayerErrorInfoEventTypesUUID;
        PVMFBasicErrorInfoMessage* errmsg = OSCL_NEW(PVMFBasicErrorInfoMessage, (PVPlayerErrSource, puuid, nextmsg));

        // Complete the SetPlaybackRange() command as Success
        EngineCommandCompleted(aNodeContext.iCmdId, aNodeContext.iCmdContext, aNodeResp.GetCmdStatus(), OSCL_STATIC_CAST(PVInterface*, errmsg));
        if (errmsg)
        {
            errmsg->removeRef();
            errmsg = NULL;
        }
    }
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeSetDataSourcePositionDuringPlayback() Out"));
}

void PVPlayerEngine::HandleSinkNodeSkipMediaDataDuringPlayback(PVPlayerEngineContext& aNodeContext, const PVMFCmdResp& aNodeResp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_INFO,
                    (0, "PVPlayerEngine::HandleSinkNodeSkipMediaDataDuringPlayback() for %s Tick=%d",
                     aNodeContext.iEngineDatapath->iTrackInfo->getTrackMimeType().get_cstr(), OsclTickCount::TickCount()));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iReposLogger, PVLOGMSG_INFO,
                    (0, "PVPlayerEngine::HandleSinkNodeSkipMediaDataDuringPlayback() for %s, iNumPVMFInfoStartOfDataPending=%d",
                     aNodeContext.iEngineDatapath->iTrackInfo->getTrackMimeType().get_cstr(), iNumPVMFInfoStartOfDataPending));

    if (aNodeResp.GetCmdStatus() != PVMFSuccess)
    {
        // Sink node report error with SkipMediaData()
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSinkNodeSkipMediaDataDuringPlayback() Sink node report error for SkipMediaData(). Asserting"));
        OSCL_ASSERT(false);
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSinkNodeSkipMediaDataDuringPlayback() In %s", aNodeContext.iEngineDatapath->iTrackInfo->getTrackMimeType().get_cstr()));

    // Stop the sink that has reached the skipping end point until other sinks are ready
    if (aNodeContext.iEngineDatapath->iDatapath && aNodeContext.iEngineDatapath->iSinkNodeSyncCtrlIF)
    {
        aNodeContext.iEngineDatapath->iSinkNodeSyncCtrlIF->ClockStopped();
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSinkNodeSkipMediaDataDuringPlayback() Datapath does not exist or sync control IF not available."));
    }

    --iNumPendingSkipCompleteEvent;

    --iNumPendingNodeCmd;

    if (iNumPendingNodeCmd == 0)
    {
        // Set the clock to the specified begin time just before Start of playback Clock
        iPlaybackClock.Stop();
        bool overflow = 0;
        iPlaybackClock.SetStartTime32(iSkipMediaDataTS, PVMF_MEDIA_CLOCK_MSEC, overflow);

        if (iOverflowFlag)
        {
            iOverflowFlag = false;
            iActualNPT = iSkipMediaDataTS;
        }
        if ((iNumPVMFInfoStartOfDataPending == 0) &&
                (iState == PVP_ENGINE_STATE_STARTED))
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iReposLogger, PVLOGMSG_INFO,
                            (0, "PVPlayerEngine::HandleSinkNodeSkipMediaDataDuringPlayback() Skipping WatchDogTimer - Starting PlayBackClock"));
            StartPlaybackClock();
        }
        else
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iReposLogger, PVLOGMSG_INFO,
                            (0, "PVPlayerEngine::HandleSinkNodeSkipMediaDataDuringPlayback() Setting WatchDogTimer for %d ms, TargetNPT=%d  ActualNPT=%d, if interval zero set to default 1 second",
                             iWatchDogTimerInterval, iTargetNPT, iActualNPT));
            // There can be a case in which WatchDogTimerInterval is zero and iNumPVMFInfoStartOfDataPending is greater than zero.
            // In this case it is possible InfoStartofData is not sent by Sink Nodes for some time and player hangs. To avoid the player hang
            // set the watchdog timer Interval to the default value of 1 second. If interval is 0, it will be set to default 1 second.
            iWatchDogTimer->setTimerDuration(iWatchDogTimerInterval);
            iWatchDogTimer->Start();
        }

        // Set the actual playback position to the requested time since actual media data TS was adjusted
        // This is important since the difference between the two is used to calculate the NPT to media data offset
        // This is not required here as the ActualPlaybackPosition is already adjusted before calling Skip on Sink Node.
        // iActualNPT=iCurrentBeginPosition.iPosValue.millisec_value;

        //clear the pending direction change NPT.
        iChangeDirectionNPT.iIndeterminate = true;
        // Save the start NPT and TS
        iStartNPT = iActualNPT;
        iStartMediaDataTS = iSkipMediaDataTS;
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO,
                        (0, "PVPlayerEngine::HandleSinkNodeSkipMediaDataDuringPlayback() TargetNPT %d, ActualNPT %d StartTS %d",
                         iTargetNPT, iStartNPT, iStartMediaDataTS));
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iReposLogger, PVLOGMSG_INFO,
                        (0, "PVPlayerEngine::HandleSinkNodeSkipMediaDataDuringPlayback() TargetNPT %d, ActualNPT %d StartTS %d",
                         iTargetNPT, iStartNPT, iStartMediaDataTS));

        // Send event to the observer indicating start of data
        if (iDataSourcePosParams.iMode == PVMF_SET_DATA_SOURCE_POSITION_MODE_NOW)
        {
            /* Reset */
            iDataSourcePosParams.iActualMediaDataTS = 0;
            iDataSourcePosParams.iActualNPT = 0;
            iDataSourcePosParams.iMode = PVMF_SET_DATA_SOURCE_POSITION_MODE_UNKNOWN;
            iDataSourcePosParams.iPlayElementIndex = -1;
            iDataSourcePosParams.iSeekToSyncPoint = true;
            iDataSourcePosParams.iTargetNPT = 0;
            SendInformationalEvent(PVMFInfoStartOfData);
        }

        // send the actual playback position from where playback will resume after reposition.
        PVPPlaybackPosition actualPlaybackPosition;
        actualPlaybackPosition.iPosValue.millisec_value = iStartNPT;
        actualPlaybackPosition.iPosUnit = PVPPBPOSUNIT_MILLISEC;

        PVUuid puuid = PVPlayerErrorInfoEventTypesUUID;
        PVMFBasicErrorInfoMessage* infomsg = OSCL_NEW(PVMFBasicErrorInfoMessage, (PVPlayerInfoPlaybackFromBeginTime, puuid, NULL));
        SendInformationalEvent(PVMFInfoActualPlaybackPosition, OSCL_STATIC_CAST(PVInterface*, infomsg), (OsclAny*)&actualPlaybackPosition);
        infomsg->removeRef();

        // Complete the SetPlaybackRange() or SetPlaybackRate()
        EngineCommandCompleted(aNodeContext.iCmdId, aNodeContext.iCmdContext, PVMFSuccess);
    }

    if ((iNumPendingSkipCompleteEvent == 0) && (iNumPVMFInfoStartOfDataPending == 0))
    {
        if (iWatchDogTimer->IsBusy())
        {
            iWatchDogTimer->Cancel();
        }
        // we have received all the bos event for
        // playback hasnt started yet

        StartPlaybackClock();
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iReposLogger, PVLOGMSG_INFO, (0, "PVPlayerEngine::HandleSinkNodeSkipMediaDataDuringPlayback - PlayClock Started"));
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSinkNodeSkipMediaDataDuringPlayback() Out"));
}


void PVPlayerEngine::HandleSourceNodePause(PVPlayerEngineContext& aNodeContext, const PVMFCmdResp& aNodeResp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodePause() In"));

    switch (aNodeResp.GetCmdStatus())
    {
        case PVMFSuccess:
        {
            // Pause command is complete
            SetEngineState(PVP_ENGINE_STATE_PAUSED);
            EngineCommandCompleted(aNodeContext.iCmdId, aNodeContext.iCmdContext, PVMFSuccess);
        }
        break;

        default:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVPlayerEngine::HandleSourceNodePause() failed, Add EH command"));
            bool ehPending = CheckForPendingErrorHandlingCmd();
            if (ehPending)
            {
                // there should be no error handling queued.
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSourceNodePause() Already EH pending, should never happen"));
                return;
            }
            PVMFErrorInfoMessageInterface* nextmsg = NULL;
            if (aNodeResp.GetEventExtensionInterface())
            {
                nextmsg = GetErrorInfoMessageInterface(*(aNodeResp.GetEventExtensionInterface()));
            }

            PVUuid puuid = PVPlayerErrorInfoEventTypesUUID;
            iCommandCompleteErrMsgInErrorHandling = OSCL_NEW(PVMFBasicErrorInfoMessage, (PVPlayerErrSourceFatal, puuid, nextmsg));
            iCommandCompleteStatusInErrorHandling = aNodeResp.GetCmdStatus();

            AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_PAUSE, NULL, NULL, NULL, false);
        }
        break;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodePause() Out"));
}


void PVPlayerEngine::HandleSourceNodeResume(PVPlayerEngineContext& aNodeContext, const PVMFCmdResp& aNodeResp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeResume() In"));

    PVMFStatus cmdstatus = PVMFErrNotSupported;

    switch (aNodeResp.GetCmdStatus())
    {
        case PVMFSuccess:
        {
            // Issue Skip on Sink Node and Start on datapaths back to back. This is done to make sure that
            // sink node is started only after discarding the data from an earlier stream.
            if (iChangePlaybackPositionWhenResuming || iChangePlaybackDirectionWhenResuming)
            {
                PVMFStatus cmdstatus = DoSinkNodeSkipMediaData(aNodeContext.iCmdId, aNodeContext.iCmdContext);
                if (cmdstatus != PVMFSuccess)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSourceNodeResume() Skip of sink node did a leave, asserting"));
                    OSCL_ASSERT(false);
                }
            }
            // Issue start to all active datapaths
            iNumPendingDatapathCmd = 0;
            for (uint32 i = 0; i < iDatapathList.size(); ++i)
            {
                if (iDatapathList[i].iDatapath)
                {
                    PVMFStatus retval = DoDatapathStart(iDatapathList[i], aNodeContext.iCmdId, aNodeContext.iCmdContext);
                    if (retval == PVMFSuccess)
                    {
                        ++iNumPendingDatapathCmd;
                        cmdstatus = PVMFSuccess;
                    }
                    else
                    {
                        cmdstatus = retval;
                        break;
                    }
                }
            }

            if (iNumPendingDatapathCmd == 0)
            {
                bool ehPending = CheckForPendingErrorHandlingCmd();
                if (ehPending)
                {
                    // there should be no error handling queued.
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSourceNodeResume() Already EH pending, should never happen"));
                    return;
                }
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSourceNodeResume() Report command as failed, Add EH command"));
                iCommandCompleteStatusInErrorHandling = cmdstatus;
                iCommandCompleteErrMsgInErrorHandling = NULL;
                AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_RESUME, NULL, NULL, NULL, false);
            }
        }
        break;

        default:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSourceNodeResume() failed, Add EH command"));
            bool ehPending = CheckForPendingErrorHandlingCmd();
            if (ehPending)
            {
                // there should be no error handling queued.
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSourceNodeResume() Already EH pending, should never happen"));
                return;
            }
            cmdstatus = aNodeResp.GetCmdStatus();
            PVMFErrorInfoMessageInterface* nextmsg = NULL;
            if (aNodeResp.GetEventExtensionInterface())
            {
                nextmsg = GetErrorInfoMessageInterface(*(aNodeResp.GetEventExtensionInterface()));
            }

            PVUuid puuid = PVPlayerErrorInfoEventTypesUUID;
            iCommandCompleteErrMsgInErrorHandling = OSCL_NEW(PVMFBasicErrorInfoMessage, (PVPlayerErrSourceFatal, puuid, nextmsg));
            iCommandCompleteStatusInErrorHandling = cmdstatus;

            AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_RESUME, NULL, NULL, NULL, false);
        }
        break;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeResume() Out"));
}


void PVPlayerEngine::HandleSourceNodeStop(PVPlayerEngineContext& aNodeContext, const PVMFCmdResp& aNodeResp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeStop() In"));

    PVMFStatus cmdstatus = PVMFFailure;

    switch (aNodeResp.GetCmdStatus())
    {
        case PVMFSuccess:
        {
            // Issue teardown sequence to all active datapaths
            iNumPendingDatapathCmd = 0;
            for (uint32 i = 0; i < iDatapathList.size(); ++i)
            {
                if (iDatapathList[i].iDatapath)
                {
                    PVMFStatus retval = DoDatapathTeardown(iDatapathList[i], aNodeContext.iCmdId, aNodeContext.iCmdContext);
                    if (retval == PVMFSuccess)
                    {
                        ++iNumPendingDatapathCmd;
                        cmdstatus = PVMFSuccess;
                    }
                    else
                    {
                        cmdstatus = retval;
                        break;
                    }
                }
            }
        }

        if (iNumPendingDatapathCmd == 0)
        {
            bool ehPending = CheckForPendingErrorHandlingCmd();
            if (ehPending)
            {
                // there should be no error handling queued.
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSourceNodeStop() Already EH pending, should never happen"));
                return;
            }
            // No active datapath to shutdown - not possible in stop
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSourceNodeStop() Datapath Teardown failed, Add EH command"));
            iCommandCompleteErrMsgInErrorHandling = NULL;
            iCommandCompleteStatusInErrorHandling = cmdstatus;
            AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_STOP, NULL, NULL, NULL, false);
        }
        break;

        default:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVPlayerEngine::HandleSourceNodeStop() Source node stop failed, go in error handling, Add EH command"));
            bool ehPending = CheckForPendingErrorHandlingCmd();
            if (ehPending)
            {
                // there should be no error handling queued.
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSourceNodeStop() Already EH pending, should never happen"));
                return;
            }
            cmdstatus = aNodeResp.GetCmdStatus();
            PVMFErrorInfoMessageInterface* nextmsg = NULL;
            if (aNodeResp.GetEventExtensionInterface())
            {
                nextmsg = GetErrorInfoMessageInterface(*(aNodeResp.GetEventExtensionInterface()));
            }

            PVUuid puuid = PVPlayerErrorInfoEventTypesUUID;
            iCommandCompleteErrMsgInErrorHandling = OSCL_NEW(PVMFBasicErrorInfoMessage, (PVPlayerErrSourceFatal, puuid, nextmsg));
            iCommandCompleteStatusInErrorHandling = cmdstatus;

            AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_STOP, NULL, NULL, NULL, false);
        }
        break;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeStop() Out"));
}


void PVPlayerEngine::HandleSourceNodeReset(PVPlayerEngineContext& aNodeContext, const PVMFCmdResp& aNodeResp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeReset() In"));

    PVMFStatus cmdstatus = PVMFErrNotSupported;

    if (aNodeResp.GetCmdStatus() == PVMFSuccess)
    {
        if (iSourceNode->GetState() != EPVMFNodeIdle)
        {
            // when reset completes on Source node, source node should be in Idle State,
            // If not then just assert.
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVPlayerEngine::HandleSourceNodeReset() Source Node not in a idle state after reset, Asserting"));
            OSCL_ASSERT(false);
        }

        PVMFStatus status = PVMFFailure;
        status = iSourceNode->ThreadLogoff();
        if (status != PVMFSuccess)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoEngineDatapathTeardown() Threadlogoff on SourceNode Failed"));
            OSCL_ASSERT(false);
        }

        if (iSourceNode->GetState() != EPVMFNodeCreated)
        {
            // when reset completes on Source node, source node should be in Idle State,
            // If not then just assert.
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVPlayerEngine::HandleSourceNodeReset() Source Node not in a created state after threadlogoff, Asserting"));
            OSCL_ASSERT(false);
        }

        // Reset active datapaths
        if (!iDatapathList.empty())
        {
            iNumPendingDatapathCmd = 0;
            PVMFCommandId cmdid = -1;
            int32 leavecode = 0;
            for (uint32 i = 0; i < iDatapathList.size(); ++i)
            {
                if ((iDatapathList[i].iDatapath != NULL) &&
                        (iDatapathList[i].iTrackInfo != NULL) &&
                        (iDatapathList[i].iDatapath->iState != PVPDP_IDLE))
                {
                    PVMFStatus retval = DoDatapathReset(iDatapathList[i], aNodeContext.iCmdId, aNodeContext.iCmdContext);
                    if (retval == PVMFSuccess)
                    {
                        ++iNumPendingDatapathCmd;
                        cmdstatus = PVMFSuccess;
                    }
                    else
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                        (0, "PVPlayerEngine::HandleSourceNodeReset() Reset failed, Asserting"));
                        OSCL_ASSERT(false);
                        break;
                    }
                }
                else if (iDatapathList[i].iDecNode != NULL)
                {
                    // This happens in case of error during prepare, when datapaths have not yet
                    // been fully constructed.
                    // reset the decoder node during inteligent track selection
                    // Call Reset() on the decoder node
                    PVPlayerEngineContext* context = AllocateEngineContext(&(iDatapathList[i]), iDatapathList[i].iDecNode, NULL, aNodeContext.iCmdId, aNodeContext.iCmdContext, PVP_CMD_DecNodeReset);

                    leavecode = IssueDecNodeReset(iDatapathList[i].iDecNode, iDatapathList[i].iDecNodeSessionId, (OsclAny*) context, cmdid);

                    if (cmdid != -1 && leavecode == 0)
                    {
                        ++iNumPendingDatapathCmd;
                    }
                    else
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSourceNodeReset() Reset on dec node leaved, asserting"));
                        FreeEngineContext(context);
                        OSCL_ASSERT(false);
                    }
                }
                else if (iDatapathList[i].iSinkNode != NULL)
                {
                    // This happens in case of error during prepare, when datapaths have not yet
                    // been fully constructed.
                    // reset the sink node during inteligent track selection
                    // Call Reset() on the sink node
                    PVPlayerEngineContext* context = AllocateEngineContext(&(iDatapathList[i]), iDatapathList[i].iSinkNode, NULL, aNodeContext.iCmdId, aNodeContext.iCmdContext, PVP_CMD_SinkNodeReset);

                    leavecode = IssueSinkNodeReset(&(iDatapathList[i]), (OsclAny*) context, cmdid);

                    if (cmdid != -1 && leavecode == 0)
                    {
                        ++iNumPendingDatapathCmd;
                    }
                    else
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSourceNodeReset() Reset on sink node leaved, asserting"));
                        FreeEngineContext(context);
                        OSCL_ASSERT(false);
                    }
                }
                else
                {
                    // No Sink nodes and no datapaths created yet so just do Enginedatapath Teardown.
                    DoEngineDatapathTeardown(iDatapathList[i]);
                }
            }

            if (iNumPendingDatapathCmd == 0)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVPlayerEngine::HandleSourceNodeReset() Reset on SourceNode completed and no datapath or sink node to reset."));
                // Reset on source node is complete and there are no datapaths or sink nodes to reset,
                // now need to remove the data sinks and sources
                // so schedule engine AO - we cannnot delete datapaths in callback, hence the reschedule
                SetEngineState(PVP_ENGINE_STATE_IDLE);
                RunIfNotReady();
            }
        }
        else
        {
            // Reset on source node is complete and there are no datapaths or sink nodes to reset,
            // now need to remove the data sinks and sources
            // so schedule engine AO - we cannnot delete datapaths in callback, hence the reschedule
            SetEngineState(PVP_ENGINE_STATE_IDLE);
            RunIfNotReady();
        }
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVPlayerEngine::HandleSourceNodeReset() Reset failed on Source Node, Asserting"));
        OSCL_ASSERT(false);
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeReset() Out"));
}


void PVPlayerEngine::HandleSinkNodePause(PVPlayerEngineContext& aNodeContext, const PVMFCmdResp& aNodeResp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSinkNodePause() In %s", aNodeContext.iEngineDatapath->iTrackInfo->getTrackMimeType().get_cstr()));

    // Decrement the counter for pending cmds
    --iNumPendingDatapathCmd;

    if (aNodeResp.GetCmdStatus() != PVMFSuccess)
    {
        bool ehPending = CheckForPendingErrorHandlingCmd();
        if (ehPending)
        {
            // there should be no error handling queued.
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSinkNodePause() Already EH pending, should never happen"));
            return;
        }
        else
        {
            // Cancel any pending node/datapath commands
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSinkNodePause() Failed, Add EH command"));
            PVMFErrorInfoMessageInterface* nextmsg = NULL;
            if (aNodeResp.GetEventExtensionInterface())
            {
                nextmsg = GetErrorInfoMessageInterface(*(aNodeResp.GetEventExtensionInterface()));
            }

            PVUuid puuid = PVPlayerErrorInfoEventTypesUUID;
            iCommandCompleteErrMsgInErrorHandling = OSCL_NEW(PVMFBasicErrorInfoMessage, (PVPlayerErrSinkFatal, puuid, nextmsg));
            iCommandCompleteStatusInErrorHandling = aNodeResp.GetCmdStatus();
            AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_PAUSE, NULL, NULL, NULL, false);
        }

        return;
    }

    if (iNumPendingDatapathCmd == 0)
    {
        // Auto-pause is complete
        SetEngineState(PVP_ENGINE_STATE_AUTO_PAUSED);
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSinkNodePause() Report command as success"));
        EngineCommandCompleted(aNodeContext.iCmdId, aNodeContext.iCmdContext, PVMFSuccess);
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSinkNodePause() Out"));
}


void PVPlayerEngine::HandleSinkNodeResume(PVPlayerEngineContext& aNodeContext, const PVMFCmdResp& aNodeResp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSinkNodeResume() In %s", aNodeContext.iEngineDatapath->iTrackInfo->getTrackMimeType().get_cstr()));

    // Decrement the counter for pending cmds
    --iNumPendingDatapathCmd;

    if (aNodeResp.GetCmdStatus() != PVMFSuccess)
    {
        bool ehPending = CheckForPendingErrorHandlingCmd();
        if (ehPending)
        {
            // there should be no error handling queued.
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSinkNodeResume() Already EH pending, should never happen"));
            return;
        }
        else
        {
            // Cancel any pending node/datapath commands
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSinkNodeResume() Failed, Add EH command"));
            PVMFErrorInfoMessageInterface* nextmsg = NULL;
            if (aNodeResp.GetEventExtensionInterface())
            {
                nextmsg = GetErrorInfoMessageInterface(*(aNodeResp.GetEventExtensionInterface()));
            }

            PVUuid puuid = PVPlayerErrorInfoEventTypesUUID;
            iCommandCompleteErrMsgInErrorHandling = OSCL_NEW(PVMFBasicErrorInfoMessage, (PVPlayerErrSinkFatal, puuid, nextmsg));
            iCommandCompleteStatusInErrorHandling = aNodeResp.GetCmdStatus();
            AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_RESUME, NULL, NULL, NULL, false);
        }
        return;
    }

    if (iNumPendingDatapathCmd == 0)
    {
        // Auto-resume is complete
        // Resume the playback clock only if InfoStartofData has been received for all tracks.
        // If SetPlaybackRange is called in Auto-Pause state, its possible that engine is waiting for InfoStartofData.
        // If waiting, restart the watchdog timer which was cancelled in Auto-Pause command.
        // this should only be done when engine is waiting for StartofData info event
        // after reposition. If already busy after SetPlaybackRange command complete,
        // then cancel it and start again. Playback clock will be started in either HandleSinkNodeInfoEvent or
        // PVPlayerWatchdogTimerEvent
        if (iNumPVMFInfoStartOfDataPending > 0)
        {
            if (iWatchDogTimerInterval > 0)
            {
                if (iWatchDogTimer->IsBusy())
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iReposLogger, PVLOGMSG_INFO,
                                    (0, "PVPlayerEngine::HandleSinkNodeResume - Pause after setplayback, Cancelling Watchdog timer, iNumPVMFInfoStartOfDataPending=%d", iNumPVMFInfoStartOfDataPending));
                    iWatchDogTimer->Cancel();
                }

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iReposLogger, PVLOGMSG_INFO,
                                (0, "PVPlayerEngine::HandleSinkNodeResume Setting WatchDogTimer for %d ms, iNumPVMFInfoStartOfDataPending=%d",
                                 iWatchDogTimerInterval, iNumPVMFInfoStartOfDataPending));
                iWatchDogTimer->setTimerDuration(iWatchDogTimerInterval);
                iWatchDogTimer->Start();
            }
        }
        // Auto-resume is complete & not waiting on PVMFInfoStartOfData, so go ahead and
        // start the clock
        if (iNumPVMFInfoStartOfDataPending == 0)
        {
            StartPlaybackClock();

            // Notify data sinks that clock has started
            for (uint32 i = 0; i < iDatapathList.size(); ++i)
            {
                if (iDatapathList[i].iDatapath && iDatapathList[i].iSinkNodeSyncCtrlIF)
                {
                    iDatapathList[i].iSinkNodeSyncCtrlIF->ClockStarted();
                }
            }
        }

        SetEngineState(PVP_ENGINE_STATE_STARTED);
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSinkNodeResume() Report command as success"));
        EngineCommandCompleted(aNodeContext.iCmdId, aNodeContext.iCmdContext, PVMFSuccess);
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSinkNodeResume() Out"));
}

void PVPlayerEngine::HandleSinkNodeReset(PVPlayerEngineContext& aNodeContext, const PVMFCmdResp& aNodeResp)
{
    OSCL_ASSERT(aNodeContext.iEngineDatapath != NULL);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSinkNodeReset() In"));

    // Decrement the counter for pending cmds
    --iNumPendingDatapathCmd;

    if (aNodeResp.GetCmdStatus() != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVPlayerEngine::HandleSinkNodeReset() Reset failed, assert"));
        OSCL_ASSERT(false);
        return;
    }
    else
    {
        // Reset for this sink node is complete
        //a sync call in engine - no async cmds issued here
        DoEngineDatapathTeardown(*(aNodeContext.iEngineDatapath));
    }

    if (iNumPendingDatapathCmd == 0)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVPlayerEngine::HandleSinkNodeReset() Reset on SourceNode and Sink nodes completed"));
        // Reset on source node and sink node is complete, now need to remove the data sinks and sources
        // so schedule engine AO - we cannnot delete datapaths in callback, hence the reschedule
        SetEngineState(PVP_ENGINE_STATE_IDLE);
        RunIfNotReady();
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSinkNodeReset() Out"));
}

void PVPlayerEngine::HandleDecNodeReset(PVPlayerEngineContext& aNodeContext, const PVMFCmdResp& aNodeResp)
{
    OSCL_ASSERT(aNodeContext.iEngineDatapath != NULL);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleDecNodeReset() In"));

    // Decrement the counter for pending cmds
    --iNumPendingDatapathCmd;

    if (aNodeResp.GetCmdStatus() != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVPlayerEngine::HandleDecNodeReset() Reset failed, assert"));
        OSCL_ASSERT(false);
        return;
    }

    if (iNumPendingDatapathCmd == 0)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVPlayerEngine::HandleDecNodeReset() Reset on SourceNode and dec nodes completed, now reset Sink nodes"));
        // Reset on source node and dec node is complete, now need to reset the sink nodes
        if (!iDatapathList.empty())
        {
            iNumPendingDatapathCmd = 0;
            PVMFCommandId cmdid = -1;
            int32 leavecode = 0;
            for (uint32 i = 0; i < iDatapathList.size(); ++i)
            {
                if (iDatapathList[i].iSinkNode != NULL)
                {
                    // This happens in case of error during prepare, when datapaths have not yet
                    // been fully constructed.
                    // reset the sink node during inteligent track selection
                    // Call Reset() on the sink node
                    PVPlayerEngineContext* context = AllocateEngineContext(&(iDatapathList[i]), iDatapathList[i].iSinkNode, NULL, aNodeContext.iCmdId, aNodeContext.iCmdContext, PVP_CMD_SinkNodeReset);

                    leavecode = IssueSinkNodeReset(&(iDatapathList[i]), (OsclAny*) context, cmdid);

                    if (cmdid != -1 && leavecode == 0)
                    {
                        ++iNumPendingDatapathCmd;
                    }
                    else
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleDecNodeReset() Reset on sink node leaved, asserting"));
                        FreeEngineContext(context);
                        OSCL_ASSERT(false);
                    }
                }
            }
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleDecNodeReset() Out"));
}

void PVPlayerEngine::HandleDatapathPrepare(PVPlayerEngineContext& aDatapathContext, PVMFStatus aDatapathStatus, PVMFCmdResp* aCmdResp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_INFO,
                    (0, "PVPlayerEngine::HandleDatapathPrepare() for %s Tick=%d",
                     aDatapathContext.iEngineDatapath->iTrackInfo->getTrackMimeType().get_cstr(), OsclTickCount::TickCount()));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleDatapathPrepare() In %s", aDatapathContext.iEngineDatapath->iTrackInfo->getTrackMimeType().get_cstr()));

    // Decrement the counter for pending datapath cmds
    --iNumPendingDatapathCmd;

    if (aDatapathStatus != PVMFSuccess)
    {
        bool ehPending = CheckForPendingErrorHandlingCmd();
        if (ehPending)
        {
            // there should be no error handling queued.
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleDatapathPrepare() Already EH pending, should never happen"));
            return;
        }
        else
        {
            // Cancel any pending node/datapath commands
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVPlayerEngine::HandleDatapathPrepare() In %s Failed, Add EH command", aDatapathContext.iEngineDatapath->iTrackInfo->getTrackMimeType().get_cstr()));
            PVMFErrorInfoMessageInterface* nextmsg = NULL;
            if (aCmdResp)
            {
                if (aCmdResp->GetEventExtensionInterface())
                {
                    nextmsg = GetErrorInfoMessageInterface(*(aCmdResp->GetEventExtensionInterface()));
                }
            }

            PVUuid puuid = PVPlayerErrorInfoEventTypesUUID;
            iCommandCompleteErrMsgInErrorHandling = OSCL_NEW(PVMFBasicErrorInfoMessage, (PVPlayerErrDatapathFatal, puuid, nextmsg));
            iCommandCompleteStatusInErrorHandling = aDatapathStatus;
            AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_PREPARE, NULL, NULL, NULL, false);
        }
        return;
    }

    if (iNumPendingDatapathCmd == 0)
    {
        // Reposition and/or start the source node
        PVMFStatus cmdstatus = DoSourceNodeQueryDataSourcePosition(aDatapathContext.iCmdId, aDatapathContext.iCmdContext);
        if (cmdstatus != PVMFSuccess)
        {
            // Setting position not supported so start the source node
            cmdstatus = DoSourceNodeStart(aDatapathContext.iCmdId, aDatapathContext.iCmdContext);
        }

        if (cmdstatus != PVMFSuccess)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleDatapathPrepare() Report command as failed, Add EH command"));
            bool ehPending = CheckForPendingErrorHandlingCmd();
            if (ehPending)
            {
                // there should be no error handling queued.
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleDatapathPrepare() Already EH pending, should never happen"));
                return;
            }
            else
            {
                iCommandCompleteStatusInErrorHandling = cmdstatus;
                iCommandCompleteErrMsgInErrorHandling = NULL;
                AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_PREPARE, NULL, NULL, NULL, false);
            }
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleDatapathPrepare() Out"));
}


void PVPlayerEngine::HandleDatapathStart(PVPlayerEngineContext& aDatapathContext, PVMFStatus aDatapathStatus, PVMFCmdResp* aCmdResp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_INFO,
                    (0, "PVPlayerEngine::HandleDatapathStart() for %s Tick=%d",
                     aDatapathContext.iEngineDatapath->iTrackInfo->getTrackMimeType().get_cstr(), OsclTickCount::TickCount()));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleDatapathStart() In %s", aDatapathContext.iEngineDatapath->iTrackInfo->getTrackMimeType().get_cstr()));

    // Decrement the counter for pending datapath cmds
    --iNumPendingDatapathCmd;

    if (aDatapathStatus != PVMFSuccess)
    {
        bool ehPending = CheckForPendingErrorHandlingCmd();
        if (ehPending)
        {
            // there should be no error handling queued.
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleDatapathStart() Already EH pending, should never happen"));
            return;
        }
        else
        {
            // Cancel any pending node/datapath commands
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVPlayerEngine::HandleDatapathStart() In %s Failed, Add EH command", aDatapathContext.iEngineDatapath->iTrackInfo->getTrackMimeType().get_cstr()));
            PVMFErrorInfoMessageInterface* nextmsg = NULL;
            if (aCmdResp)
            {
                if (aCmdResp->GetEventExtensionInterface())
                {
                    nextmsg = GetErrorInfoMessageInterface(*(aCmdResp->GetEventExtensionInterface()));
                }
            }

            PVUuid puuid = PVPlayerErrorInfoEventTypesUUID;
            iCommandCompleteErrMsgInErrorHandling = OSCL_NEW(PVMFBasicErrorInfoMessage, (PVPlayerErrDatapathFatal, puuid, nextmsg));
            iCommandCompleteStatusInErrorHandling = aDatapathStatus;
            AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_PREPARE, NULL, NULL, NULL, false);
        }
        return;
    }

    if (iNumPendingDatapathCmd == 0)
    {
        // This should never be happen if Skip on sink node has not completed (i.e. iNumPendingSkipCompleteEvent
        // is greater than zero). If this happens with iNumPendingSkipCompleteEvent > 0 then just assert.
        // Set the clock to the specified begin time just before Start of playback Clock
        iPlaybackClock.Stop();
        bool overflow = 0;
        iPlaybackClock.SetStartTime32(iSkipMediaDataTS, PVMF_MEDIA_CLOCK_MSEC, overflow);

        if (!(iWatchDogTimer->IsBusy()))
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iReposLogger, PVLOGMSG_INFO,
                            (0, "PVPlayerEngine::HandleDatapathStart() Setting WatchDogTimer for %d ms, TargetNPT=%d  ActualNPT=%d, if interval zero set to default 1 second",
                             iWatchDogTimerInterval, iTargetNPT, iActualNPT));
            iWatchDogTimer->setTimerDuration(iWatchDogTimerInterval);
            iWatchDogTimer->Start();
        }
        // Set the actual playback position to the requested time since actual media data TS was adjusted
        // This is important since the difference between the two is used to calculate the NPT to media data offset
        // This is not required here as the ActualPlaybackPosition is already adjusted before calling Skip on Sink Node.
        // iActualNPT=iCurrentBeginPosition.iPosValue.millisec_value;

        // Save the start NPT and TS
        iStartNPT = iActualNPT;
        iStartMediaDataTS = iSkipMediaDataTS;
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO,
                        (0, "PVPlayerEngine::HandleDatapathStart() TargetNPT %d, StartNPT %d StartTS %d",
                         iTargetNPT, iStartNPT, iStartMediaDataTS));
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iReposLogger, PVLOGMSG_INFO,
                        (0, "PVPlayerEngine::HandleDatapathStart() TargetNPT %d, StartNPT %d StartTS %d",
                         iTargetNPT, iStartNPT, iStartMediaDataTS));

        SetEngineState(PVP_ENGINE_STATE_PREPARED);

        // send the actual playback position from where playback will resume after reposition.
        PVPPlaybackPosition actualPlaybackPosition;
        actualPlaybackPosition.iPosValue.millisec_value = iStartNPT;
        actualPlaybackPosition.iPosUnit = PVPPBPOSUNIT_MILLISEC;

        PVUuid puuid = PVPlayerErrorInfoEventTypesUUID;
        PVMFBasicErrorInfoMessage* infomsg = OSCL_NEW(PVMFBasicErrorInfoMessage, (PVPlayerInfoPlaybackFromBeginTime, puuid, NULL));
        SendInformationalEvent(PVMFInfoActualPlaybackPosition, OSCL_STATIC_CAST(PVInterface*, infomsg), (OsclAny*)&actualPlaybackPosition);
        infomsg->removeRef();

        EngineCommandCompleted(aDatapathContext.iCmdId, aDatapathContext.iCmdContext, PVMFSuccess);
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleDatapathStart() Out"));
}


void PVPlayerEngine::HandleDatapathPause(PVPlayerEngineContext& aDatapathContext, PVMFStatus aDatapathStatus, PVMFCmdResp* aCmdResp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_INFO,
                    (0, "PVPlayerEngine::HandleDatapathPause() for %s Tick=%d",
                     aDatapathContext.iEngineDatapath->iTrackInfo->getTrackMimeType().get_cstr(), OsclTickCount::TickCount()));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleDatapathPause() In %s", aDatapathContext.iEngineDatapath->iTrackInfo->getTrackMimeType().get_cstr()));

    // Decrement the counter for pending datapath cmds
    --iNumPendingDatapathCmd;

    if (aDatapathStatus != PVMFSuccess)
    {
        bool ehPending = CheckForPendingErrorHandlingCmd();
        if (ehPending)
        {
            // there should be no error handling queued.
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleDatapathPause() Already EH pending, should never happen"));
            return;
        }
        else
        {
            // Cancel any pending node/datapath commands
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVPlayerEngine::HandleDatapathPause() In %s Failed, Add EH command", aDatapathContext.iEngineDatapath->iTrackInfo->getTrackMimeType().get_cstr()));
            PVMFErrorInfoMessageInterface* nextmsg = NULL;
            if (aCmdResp)
            {
                if (aCmdResp->GetEventExtensionInterface())
                {
                    nextmsg = GetErrorInfoMessageInterface(*(aCmdResp->GetEventExtensionInterface()));
                }
            }

            PVUuid puuid = PVPlayerErrorInfoEventTypesUUID;
            iCommandCompleteErrMsgInErrorHandling = OSCL_NEW(PVMFBasicErrorInfoMessage, (PVPlayerErrDatapathFatal, puuid, nextmsg));
            iCommandCompleteStatusInErrorHandling = aDatapathStatus;
            AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_PAUSE, NULL, NULL, NULL, false);
        }
        return;
    }

    if (iNumPendingDatapathCmd == 0)
    {
        // Continue on by pauseing the source node
        PVMFStatus cmdstatus = DoSourceNodePause(aDatapathContext.iCmdId, aDatapathContext.iCmdContext);

        if (cmdstatus != PVMFSuccess)
        {
            bool ehPending = CheckForPendingErrorHandlingCmd();
            if (ehPending)
            {
                // there should be no error handling queued.
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleDatapathPause() Already EH pending, should never happen"));
                return;
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleDatapathPause() Report command as failed, Add EH command"));
                iCommandCompleteStatusInErrorHandling = cmdstatus;
                iCommandCompleteErrMsgInErrorHandling = NULL;
                AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_PAUSE, NULL, NULL, NULL, false);
            }
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleDatapathPause() Out"));
}


void PVPlayerEngine::HandleDatapathResume(PVPlayerEngineContext& aDatapathContext, PVMFStatus aDatapathStatus, PVMFCmdResp* aCmdResp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_INFO,
                    (0, "PVPlayerEngine::HandleDatapathResume() for %s Tick=%d",
                     aDatapathContext.iEngineDatapath->iTrackInfo->getTrackMimeType().get_cstr(), OsclTickCount::TickCount()));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleDatapathResume() In %s", aDatapathContext.iEngineDatapath->iTrackInfo->getTrackMimeType().get_cstr()));

    // Decrement the counter for pending datapath cmds
    --iNumPendingDatapathCmd;

    if (aDatapathStatus != PVMFSuccess)
    {
        bool ehPending = CheckForPendingErrorHandlingCmd();
        if (ehPending)
        {
            // there should be no error handling queued.
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleDatapathResume() Already EH pending, should never happen"));
            return;
        }
        else
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVPlayerEngine::HandleDatapathResume() In %s Failed, Add EH command", aDatapathContext.iEngineDatapath->iTrackInfo->getTrackMimeType().get_cstr()));
            PVMFErrorInfoMessageInterface* nextmsg = NULL;
            if (aCmdResp)
            {
                if (aCmdResp->GetEventExtensionInterface())
                {
                    nextmsg = GetErrorInfoMessageInterface(*(aCmdResp->GetEventExtensionInterface()));
                }
            }

            PVUuid puuid = PVPlayerErrorInfoEventTypesUUID;
            iCommandCompleteErrMsgInErrorHandling = OSCL_NEW(PVMFBasicErrorInfoMessage, (PVPlayerErrDatapathFatal, puuid, nextmsg));
            iCommandCompleteStatusInErrorHandling = aDatapathStatus;
            AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_STOP, NULL, NULL, NULL, false);
        }
        return;
    }

    if (iNumPendingDatapathCmd == 0)
    {
        if (iChangePlaybackPositionWhenResuming || iChangePlaybackDirectionWhenResuming)
        {
            // This should never be happen if Skip on sink node has not completed (i.e. iNumPendingSkipCompleteEvent
            // is greater than zero). If this happens with iNumPendingSkipCompleteEvent > 0 then just assert.
            // Set the clock to the specified begin time just before Start of playback Clock
            iPlaybackClock.Stop();
            bool overflow = 0;
            iPlaybackClock.SetStartTime32(iSkipMediaDataTS, PVMF_MEDIA_CLOCK_MSEC, overflow);

            if (iNumPVMFInfoStartOfDataPending == 0)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iReposLogger, PVLOGMSG_INFO,
                                (0, "PVPlayerEngine::HandleDatapathResume() Skipping WatchDogTimer - Starting PlayBackClock"));
                StartPlaybackClock();
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iReposLogger, PVLOGMSG_INFO,
                                (0, "PVPlayerEngine::HandleDatapathResume() Setting WatchDogTimer for %d ms, TargetNPT=%d  ActualNPT=%d",
                                 iWatchDogTimerInterval, iTargetNPT, iActualNPT));

                // There can be a case in which WatchDogTimerInterval is zero and iNumPVMFInfoStartOfDataPending is greater than zero.
                // In this case it is possible InfoStartofData is not sent by Sink Nodes for some time and player hangs. To avoid the player hang
                // set the watchdog timer Interval to the default value of 1 second.
                iWatchDogTimer->setTimerDuration(iWatchDogTimerInterval);
                iWatchDogTimer->Start();
            }

            // Set the actual playback position to the requested time since actual media data TS was adjusted
            // This is important since the difference between the two is used to calculate the NPT to media data offset
            // This is not required here as the ActualPlaybackPosition is already adjusted before calling Skip on Sink Node.
            // iActualNPT=iCurrentBeginPosition.iPosValue.millisec_value;

            //clear the pending direction change NPT.
            iChangeDirectionNPT.iIndeterminate = true;
            // Save the start NPT and TS
            iStartNPT = iActualNPT;
            iStartMediaDataTS = iSkipMediaDataTS;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO,
                            (0, "PVPlayerEngine::HandleDatapathResume() TargetNPT %d, ActualNPT %d StartTS %d",
                             iTargetNPT, iStartNPT, iStartMediaDataTS));
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iReposLogger, PVLOGMSG_INFO,
                            (0, "PVPlayerEngine::HandleDatapathResume() TargetNPT %d, ActualNPT %d StartTS %d",
                             iTargetNPT, iStartNPT, iStartMediaDataTS));

            // send the actual playback position from where playback will resume after reposition.
            PVPPlaybackPosition actualPlaybackPosition;
            actualPlaybackPosition.iPosValue.millisec_value = iStartNPT;
            actualPlaybackPosition.iPosUnit = PVPPBPOSUNIT_MILLISEC;

            PVUuid puuid = PVPlayerErrorInfoEventTypesUUID;
            PVMFBasicErrorInfoMessage* infomsg = OSCL_NEW(PVMFBasicErrorInfoMessage, (PVPlayerInfoPlaybackFromBeginTime, puuid, NULL));
            SendInformationalEvent(PVMFInfoActualPlaybackPosition, OSCL_STATIC_CAST(PVInterface*, infomsg), (OsclAny*)&actualPlaybackPosition);
            infomsg->removeRef();
        }
        else
        {
            // Resume the playback clock - only if we have come out of any previous auto pause, if any
            if (iPlaybackClock.GetState() == PVMFMediaClock::PAUSED)
            {
                StartPlaybackClock();

                // Notify all sink nodes that have sync control IF that clock has started
                for (uint32 i = 0; i < iDatapathList.size(); ++i)
                {
                    if (iDatapathList[i].iDatapath && iDatapathList[i].iSinkNodeSyncCtrlIF)
                    {
                        iDatapathList[i].iSinkNodeSyncCtrlIF->ClockStarted();
                    }
                }
            }

            // Restart the watchdog timer which was cancelled in Pause command.
            // this should only be done when engine is waiting for StartofData info event
            // after reposition.
            if (iNumPVMFInfoStartOfDataPending > 0)
            {
                if (iWatchDogTimerInterval > 0)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iReposLogger, PVLOGMSG_INFO,
                                    (0, "PVPlayerEngine::HandleDatapathResume Setting WatchDogTimer for %d ms, iNumPVMFInfoStartOfDataPending=%d",
                                     iWatchDogTimerInterval, iNumPVMFInfoStartOfDataPending));
                    iWatchDogTimer->setTimerDuration(iWatchDogTimerInterval);
                    iWatchDogTimer->Start();
                }
            }

            // Restart the end time check if enabled
            if (iEndTimeCheckEnabled)
            {
                // Determine the check cycle based on interval setting in milliseconds
                // and timer frequency of 100 millisec
                int32 checkcycle = iEndTimeCheckInterval / 100;
                if (checkcycle == 0)
                {
                    ++checkcycle;
                }
                iPollingCheckTimer->Cancel(PVPLAYERENGINE_TIMERID_ENDTIMECHECK);
                iPollingCheckTimer->Request(PVPLAYERENGINE_TIMERID_ENDTIMECHECK, 0, checkcycle, this, true);
            }
        }

        SetEngineState(PVP_ENGINE_STATE_STARTED);

        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleDatapathResume() Report command as completed"));
        EngineCommandCompleted(aDatapathContext.iCmdId, aDatapathContext.iCmdContext, PVMFSuccess);
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleDatapathResume() Out"));
}


void PVPlayerEngine::HandleDatapathStop(PVPlayerEngineContext& aDatapathContext, PVMFStatus aDatapathStatus, PVMFCmdResp* aCmdResp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_INFO,
                    (0, "PVPlayerEngine::HandleDatapathStop() for %s Tick=%d",
                     aDatapathContext.iEngineDatapath->iTrackInfo->getTrackMimeType().get_cstr(), OsclTickCount::TickCount()));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleDatapathStop() In %s", aDatapathContext.iEngineDatapath->iTrackInfo->getTrackMimeType().get_cstr()));

    // Decrement the counter for pending datapath cmds
    --iNumPendingDatapathCmd;

    if (aDatapathStatus != PVMFSuccess)
    {
        bool ehPending = CheckForPendingErrorHandlingCmd();
        if (ehPending)
        {
            // there should be no error handling queued.
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleDatapathStop() Already EH pending, should never happen"));
            return;
        }
        else
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVPlayerEngine::HandleDatapathStop() In %s Failed, Add EH command", aDatapathContext.iEngineDatapath->iTrackInfo->getTrackMimeType().get_cstr()));
            PVMFErrorInfoMessageInterface* nextmsg = NULL;
            if (aCmdResp)
            {
                if (aCmdResp->GetEventExtensionInterface())
                {
                    nextmsg = GetErrorInfoMessageInterface(*(aCmdResp->GetEventExtensionInterface()));
                }
            }

            PVUuid puuid = PVPlayerErrorInfoEventTypesUUID;
            iCommandCompleteErrMsgInErrorHandling = OSCL_NEW(PVMFBasicErrorInfoMessage, (PVPlayerErrDatapathFatal, puuid, nextmsg));
            iCommandCompleteStatusInErrorHandling = aDatapathStatus;
            AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_STOP, NULL, NULL, NULL, false);
        }
        return;
    }

    if (iNumPendingDatapathCmd == 0)
    {
        // Continue on by stopping the source node
        PVMFStatus cmdstatus = DoSourceNodeStop(aDatapathContext.iCmdId, aDatapathContext.iCmdContext);

        if (cmdstatus != PVMFSuccess)
        {
            bool ehPending = CheckForPendingErrorHandlingCmd();
            if (ehPending)
            {
                // there should be no error handling queued.
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleDatapathStop() Already EH pending, should never happen"));
                return;
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleDatapathStop() Report command as failed, Add EH command"));
                iCommandCompleteErrMsgInErrorHandling = NULL;
                iCommandCompleteStatusInErrorHandling = cmdstatus;
                AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_STOP, NULL, NULL, NULL, false);
            }
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleDatapathStop() Out"));
}


void PVPlayerEngine::HandleDatapathTeardown(PVPlayerEngineContext& aDatapathContext, PVMFStatus aDatapathStatus, PVMFCmdResp* aCmdResp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_INFO,
                    (0, "PVPlayerEngine::HandleDatapathTeardown() for %s Tick=%d",
                     aDatapathContext.iEngineDatapath->iTrackInfo->getTrackMimeType().get_cstr(), OsclTickCount::TickCount()));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleDatapathTeardown() In %s", aDatapathContext.iEngineDatapath->iTrackInfo->getTrackMimeType().get_cstr()));

    PVMFStatus cmdstatus;

    switch (aDatapathStatus)
    {
        case PVMFSuccess:
            // Reset this datapath next
            cmdstatus = DoDatapathReset(*(aDatapathContext.iEngineDatapath), aDatapathContext.iCmdId, aDatapathContext.iCmdContext);
            break;

        default:
        {
            bool ehPending = CheckForPendingErrorHandlingCmd();
            if (ehPending)
            {
                // there should be no error handling queued.
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleDatapathTeardown() Already EH pending, should never happen"));
                return;
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVPlayerEngine::HandleDatapathTeardown() In %s Failed, Add EH command", aDatapathContext.iEngineDatapath->iTrackInfo->getTrackMimeType().get_cstr()));
                PVMFErrorInfoMessageInterface* nextmsg = NULL;
                if (aCmdResp)
                {
                    if (aCmdResp->GetEventExtensionInterface())
                    {
                        nextmsg = GetErrorInfoMessageInterface(*(aCmdResp->GetEventExtensionInterface()));
                    }
                }

                PVUuid puuid = PVPlayerErrorInfoEventTypesUUID;
                iCommandCompleteErrMsgInErrorHandling = OSCL_NEW(PVMFBasicErrorInfoMessage, (PVPlayerErrDatapathFatal, puuid, nextmsg));
                iCommandCompleteStatusInErrorHandling = aDatapathStatus;
                AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_STOP, NULL, NULL, NULL, false);
            }
            return;
        }
    }

    if (cmdstatus != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleDatapathTeardown() Reset failed, asserting"));
        OSCL_ASSERT(false);
        EngineCommandCompleted(aDatapathContext.iCmdId, aDatapathContext.iCmdContext, cmdstatus);
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleVideoDatapathTeardown() Out"));
}


void PVPlayerEngine::HandleDatapathReset(PVPlayerEngineContext& aDatapathContext, PVMFStatus aDatapathStatus, PVMFCmdResp* aCmdResp)
{
    OSCL_UNUSED_ARG(aCmdResp);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iPerfLogger, PVLOGMSG_INFO,
                    (0, "PVPlayerEngine::HandleDatapathReset() for %s Tick=%d",
                     aDatapathContext.iEngineDatapath->iTrackInfo->getTrackMimeType().get_cstr(), OsclTickCount::TickCount()));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleDatapathReset() In %s", aDatapathContext.iEngineDatapath->iTrackInfo->getTrackMimeType().get_cstr()));

    // Decrement the counter for pending datapath cmds
    --iNumPendingDatapathCmd;

    if (aDatapathStatus != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVPlayerEngine::HandleDatapathReset() In %s Reset failed, assert", aDatapathContext.iEngineDatapath->iTrackInfo->getTrackMimeType().get_cstr()));
        OSCL_ASSERT(false);
        return;
    }
    else
    {
        // Reset for this datapath is complete
        //a sync call in engine - no async cmds issued here
        DoEngineDatapathTeardown(*(aDatapathContext.iEngineDatapath));
    }

    //this means datapath reset and teardown is complete for all datapaths
    if (iNumPendingDatapathCmd == 0)
    {
        if (iState == PVP_ENGINE_STATE_RESETTING)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVPlayerEngine::HandleDatapathReset() Reset on SourceNode and Datapath completed"));
            // Reset on source node and datapath is complete, now need to remove the data sinks and sources
            // so schedule engine AO - we cannnot delete datapaths in callback, hence the reschedule
            SetEngineState(PVP_ENGINE_STATE_IDLE);
            RunIfNotReady();
        }
        else if (iState == PVP_ENGINE_STATE_STOPPING)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleDatapathReset() Stop Completed"));
            // stop has completed
            SetEngineState(PVP_ENGINE_STATE_INITIALIZED);
            EngineCommandCompleted(aDatapathContext.iCmdId, aDatapathContext.iCmdContext, PVMFSuccess);
        }
        else
        {
            //engine cannot be in any other state
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVPlayerEngine::HandleDatapathReset() Wrong Engine state for Reset to complete, asserting"));
            OSCL_ASSERT(false);
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleDatapathReset() Out"));
}

void PVPlayerEngine::HandleSourceNodeGetLicense(PVPlayerEngineContext& aNodeContext, const PVMFCmdResp& aNodeResp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeGetLicense() In"));

    switch (aNodeResp.GetCmdStatus())
    {
        case PVMFSuccess:
            EngineCommandCompleted(aNodeContext.iCmdId, aNodeContext.iCmdContext, PVMFSuccess);
            break;

        case PVMFErrCancelled:
        default:
        {
            // report command complete
            PVMFStatus cmdstatus = aNodeResp.GetCmdStatus();

            PVMFErrorInfoMessageInterface* nextmsg = NULL;
            if (aNodeResp.GetEventExtensionInterface())
            {
                nextmsg = GetErrorInfoMessageInterface(*(aNodeResp.GetEventExtensionInterface()));
            }

            PVUuid puuid = PVPlayerErrorInfoEventTypesUUID;
            PVMFBasicErrorInfoMessage* errmsg = OSCL_NEW(PVMFBasicErrorInfoMessage, (PVPlayerErrSource, puuid, nextmsg));
            EngineCommandCompleted(aNodeContext.iCmdId, aNodeContext.iCmdContext, cmdstatus, OSCL_STATIC_CAST(PVInterface*, errmsg));
            errmsg->removeRef();

        }
        break;
    }
    /* Set CancelAcquireLicense cmd in current cmd */
    if (!iCmdToDlaCancel.empty())
    {
        iCurrentCmd.push_front(iCmdToDlaCancel[0]);
        iCmdToDlaCancel.clear();
    }
}

void PVPlayerEngine::HandleSourceNodeCancelGetLicense(PVPlayerEngineContext& aNodeContext, const PVMFCmdResp& aNodeResp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeCancelGetLicense() In"));

    switch (aNodeResp.GetCmdStatus())
    {
        case PVMFSuccess:
            EngineCommandCompleted(aNodeContext.iCmdId, aNodeContext.iCmdContext, PVMFSuccess);
            break;

        default:
        {
            /* report command complete */
            PVMFStatus cmdstatus = aNodeResp.GetCmdStatus();

            PVMFErrorInfoMessageInterface* nextmsg = NULL;
            if (aNodeResp.GetEventExtensionInterface())
            {
                nextmsg = GetErrorInfoMessageInterface(*(aNodeResp.GetEventExtensionInterface()));
            }

            PVUuid puuid = PVPlayerErrorInfoEventTypesUUID;
            PVMFBasicErrorInfoMessage* errmsg = OSCL_NEW(PVMFBasicErrorInfoMessage, (PVPlayerErrSource, puuid, nextmsg));

            if (iCurrentCmd[0].GetCmdType() == PVP_ENGINE_COMMAND_ACQUIRE_LICENSE_CHAR
                    || iCurrentCmd[0].GetCmdType() == PVP_ENGINE_COMMAND_ACQUIRE_LICENSE_WCHAR)
            {
                if (!iCmdToDlaCancel.empty())
                {
                    PVPlayerEngineCommand currentcmd(iCurrentCmd[0]);
                    iCurrentCmd.erase(iCurrentCmd.begin());
                    iCurrentCmd.push_front(iCmdToDlaCancel[0]);
                    /* If we get here the command isn't queued so the cancel fails */
                    EngineCommandCompleted(aNodeContext.iCmdId, aNodeContext.iCmdContext, cmdstatus, OSCL_STATIC_CAST(PVInterface*, errmsg));
                    iCurrentCmd.push_front(currentcmd);
                    iCmdToDlaCancel.erase(iCmdToDlaCancel.begin());
                }
                else
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSourceNodeCancelGetLicense() ASSERT"));
                    OSCL_ASSERT(false);
                }
            }
            else if (iCurrentCmd[0].GetCmdType() == PVP_ENGINE_COMMAND_CANCEL_ACQUIRE_LICENSE)
            {
                EngineCommandCompleted(aNodeContext.iCmdId, aNodeContext.iCmdContext, cmdstatus, OSCL_STATIC_CAST(PVInterface*, errmsg));
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSourceNodeCancelGetLicense() ASSERT"));
                OSCL_ASSERT(false);
            }
            errmsg->removeRef();
        }
        break;
    }
}

void PVPlayerEngine::HandleSourceNodeErrorEvent(const PVMFAsyncEvent& aEvent)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSourceNodeErrorEvent() In"));

    if (iState == PVP_ENGINE_STATE_RESETTING)
    {
        //this means error handling, reset or cancelall is still in progress
        //no need to look for error event
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVPlayerEngine::HandleSourceNodeErrorEvent() Return engine in resetting state no need to process the error event"));
        return;
    }

    for (uint32 i = 0; i < iCurrentContextList.size(); ++i)
    {
        if (iCurrentContextList[i]->iNode)
        {
            if (iCurrentContextList[i]->iNode == iSourceNode)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVPlayerEngine::HandleSourceNodeErrorEvent() Cmd Pending on Source node, so src node should not send error events - ignoring err event"));
                return;
            }
        }
    }

    PVMFEventType event = aEvent.GetEventType();

    switch (event)
    {
        case PVMFErrCorrupt:
        case PVMFErrOverflow:
        case PVMFErrResource:
        case PVMFErrProcessing:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVPlayerEngine::HandleSourceNodeErrorEvent() Sending PVPlayerErrSourceMediaData for error event %d, Add EH command if not present", event));
            bool ehPending = CheckForPendingErrorHandlingCmd();
            if (ehPending)
            {
                // there should be no error handling queued.
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSourceNodeErrorEvent() Already EH pending, should never happen"));
                return;
            }
            else
            {
                PVMFErrorInfoMessageInterface* nextmsg = NULL;
                if (aEvent.GetEventExtensionInterface() != NULL)
                {
                    nextmsg = GetErrorInfoMessageInterface(*(aEvent.GetEventExtensionInterface()));
                }
                PVUuid puuid = PVPlayerErrorInfoEventTypesUUID;
                iCommandCompleteErrMsgInErrorHandling = OSCL_NEW(PVMFBasicErrorInfoMessage, (PVPlayerErrSourceMediaData, puuid, nextmsg));
                iCommandCompleteStatusInErrorHandling = event;
                AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_GENERAL, NULL, NULL, NULL, false);
                if (iCurrentCmd.empty())
                {
                    // this error was received when no cmd was being processed so send the error event from here
                    SendErrorEvent(iCommandCompleteStatusInErrorHandling,
                                   OSCL_STATIC_CAST(PVInterface*, iCommandCompleteErrMsgInErrorHandling),
                                   aEvent.GetEventData(), aEvent.GetLocalBuffer(), aEvent.GetLocalBufferSize());
                    iCommandCompleteErrMsgInErrorHandling->removeRef();
                    iCommandCompleteErrMsgInErrorHandling = NULL;
                }
            }
        }
        break;

        case PVMFErrUnderflow:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVPlayerEngine::HandleSourceNodeErrorEvent() Sending PVPlayerErrSourceMediaDataUnavailable for error event %d, Add EH command if not present", event));
            bool ehPending = CheckForPendingErrorHandlingCmd();
            if (ehPending)
            {
                // there should be no error handling queued.
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSourceNodeErrorEvent() Already EH pending, should never happen"));
                return;
            }
            else
            {
                PVMFErrorInfoMessageInterface* nextmsg = NULL;
                if (aEvent.GetEventExtensionInterface() != NULL)
                {
                    nextmsg = GetErrorInfoMessageInterface(*(aEvent.GetEventExtensionInterface()));
                }
                PVUuid puuid = PVPlayerErrorInfoEventTypesUUID;
                iCommandCompleteErrMsgInErrorHandling = OSCL_NEW(PVMFBasicErrorInfoMessage, (PVPlayerErrSourceMediaDataUnavailable, puuid, nextmsg));
                iCommandCompleteStatusInErrorHandling = event;
                AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_GENERAL, NULL, NULL, NULL, false);
                if (iCurrentCmd.empty())
                {
                    // this error was received when no cmd was being processed so send the error event from here
                    SendErrorEvent(iCommandCompleteStatusInErrorHandling,
                                   OSCL_STATIC_CAST(PVInterface*, iCommandCompleteErrMsgInErrorHandling),
                                   aEvent.GetEventData(), aEvent.GetLocalBuffer(), aEvent.GetLocalBufferSize());
                    iCommandCompleteErrMsgInErrorHandling->removeRef();
                    iCommandCompleteErrMsgInErrorHandling = NULL;
                }
            }
        }
        break;

        case PVMFErrNoResources:
        case PVMFErrResourceConfiguration:
        case PVMFErrTimeout:
        case PVMFErrNoMemory:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVPlayerEngine::HandleSourceNodeErrorEvent() Sending PVPlayerErrSourceFatal for error event %d, Add EH command if not present", event));
            bool ehPending = CheckForPendingErrorHandlingCmd();
            if (ehPending)
            {
                // there should be no error handling queued.
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSourceNodeErrorEvent() Already EH pending, should never happen"));
                return;
            }
            else
            {
                PVMFErrorInfoMessageInterface* nextmsg = NULL;
                if (aEvent.GetEventExtensionInterface() != NULL)
                {
                    nextmsg = GetErrorInfoMessageInterface(*(aEvent.GetEventExtensionInterface()));
                }
                PVUuid puuid = PVPlayerErrorInfoEventTypesUUID;
                iCommandCompleteErrMsgInErrorHandling = OSCL_NEW(PVMFBasicErrorInfoMessage, (PVPlayerErrSourceFatal, puuid, nextmsg));
                iCommandCompleteStatusInErrorHandling = event;
                AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_GENERAL, NULL, NULL, NULL, false);
                if (iCurrentCmd.empty())
                {
                    // this error was received when no cmd was being processed so send the error event from here
                    SendErrorEvent(iCommandCompleteStatusInErrorHandling,
                                   OSCL_STATIC_CAST(PVInterface*, iCommandCompleteErrMsgInErrorHandling),
                                   aEvent.GetEventData(), aEvent.GetLocalBuffer(), aEvent.GetLocalBufferSize());
                    iCommandCompleteErrMsgInErrorHandling->removeRef();
                    iCommandCompleteErrMsgInErrorHandling = NULL;
                }
            }
        }
        break;

        default:
            // Do nothing but log it
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeErrorEvent() Do nothing for this event %d", event));
            break;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeErrorEvent() Out"));
}


void PVPlayerEngine::HandleDecNodeErrorEvent(const PVMFAsyncEvent& aEvent, int32 aDatapathIndex)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleDecNodeErrorEvent() In"));

    OSCL_UNUSED_ARG(aDatapathIndex);

    if (iState == PVP_ENGINE_STATE_RESETTING)
    {
        //this means error handling, reset or cancelall is still in progress
        //no need to look for error event
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVPlayerEngine::HandleDecNodeErrorEvent() Return engine in resetting state no need to process the error event"));
        return;
    }

    PVMFEventType event = aEvent.GetEventType();

    switch (event)
    {
        case PVMFErrCorrupt:
        case PVMFErrOverflow:
        case PVMFErrUnderflow:
        case PVMFErrProcessing:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVPlayerEngine::HandleDecNodeErrorEvent() Sending PVPlayerErrDatapathMediaData for error event %d, Add EH command if not present", event));
            bool ehPending = CheckForPendingErrorHandlingCmd();
            if (ehPending)
            {
                // there should be no error handling queued.
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleDecNodeErrorEvent() Already EH pending, should never happen"));
                return;
            }
            else
            {
                PVMFErrorInfoMessageInterface* nextmsg = NULL;
                if (aEvent.GetEventExtensionInterface() != NULL)
                {
                    nextmsg = GetErrorInfoMessageInterface(*(aEvent.GetEventExtensionInterface()));
                }
                PVUuid puuid = PVPlayerErrorInfoEventTypesUUID;
                iCommandCompleteErrMsgInErrorHandling = OSCL_NEW(PVMFBasicErrorInfoMessage, (PVPlayerErrDatapathMediaData, puuid, nextmsg));
                iCommandCompleteStatusInErrorHandling = event;
                AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_GENERAL, NULL, NULL, NULL, false);
                if (iCurrentCmd.empty())
                {
                    // this error was received when no cmd was being processed so send the error event from here
                    SendErrorEvent(iCommandCompleteStatusInErrorHandling,
                                   OSCL_STATIC_CAST(PVInterface*, iCommandCompleteErrMsgInErrorHandling),
                                   aEvent.GetEventData(), aEvent.GetLocalBuffer(), aEvent.GetLocalBufferSize());
                    iCommandCompleteErrMsgInErrorHandling->removeRef();
                    iCommandCompleteErrMsgInErrorHandling = NULL;
                }
            }
        }
        break;

        case PVMFErrTimeout:
        case PVMFErrNoResources:
        case PVMFErrResourceConfiguration:
        case PVMFErrResource:
        case PVMFErrNoMemory:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVPlayerEngine::HandleDecNodeErrorEvent() Sending PVPlayerErrDatapathFatal for error event %d, Add EH command if not present", event));
            bool ehPending = CheckForPendingErrorHandlingCmd();
            if (ehPending)
            {
                // there should be no error handling queued.
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleDecNodeErrorEvent() Already EH pending, should never happen"));
                return;
            }
            else
            {
                PVMFErrorInfoMessageInterface* nextmsg = NULL;
                if (aEvent.GetEventExtensionInterface() != NULL)
                {
                    nextmsg = GetErrorInfoMessageInterface(*(aEvent.GetEventExtensionInterface()));
                }
                PVUuid puuid = PVPlayerErrorInfoEventTypesUUID;
                iCommandCompleteErrMsgInErrorHandling = OSCL_NEW(PVMFBasicErrorInfoMessage, (PVPlayerErrDatapathFatal, puuid, nextmsg));
                iCommandCompleteStatusInErrorHandling = event;
                AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_GENERAL, NULL, NULL, NULL, false);
                if (iCurrentCmd.empty())
                {
                    // this error was received when no cmd was being processed so send the error event from here
                    SendErrorEvent(iCommandCompleteStatusInErrorHandling,
                                   OSCL_STATIC_CAST(PVInterface*, iCommandCompleteErrMsgInErrorHandling),
                                   aEvent.GetEventData(), aEvent.GetLocalBuffer(), aEvent.GetLocalBufferSize());
                    iCommandCompleteErrMsgInErrorHandling->removeRef();
                    iCommandCompleteErrMsgInErrorHandling = NULL;
                }
            }
        }
        break;

        default:
            // Do nothing but log it
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleDecNodeErrorEvent() Do nothing for this event %d", event));
            break;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleDecNodeErrorEvent() Out"));
}


void PVPlayerEngine::HandleSinkNodeErrorEvent(const PVMFAsyncEvent& aEvent, int32 aDatapathIndex)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSinkNodeErrorEvent() In"));

    OSCL_UNUSED_ARG(aDatapathIndex);

    if (iState == PVP_ENGINE_STATE_RESETTING)
    {
        //this means error handling, reset or cancelall is still in progress
        //no need to look for error event
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVPlayerEngine::HandleSinkNodeErrorEvent() Return engine in resetting state no need to process the error event"));
        return;
    }

    PVMFEventType event = aEvent.GetEventType();

    switch (event)
    {
        case PVMFErrCorrupt:
        case PVMFErrOverflow:
        case PVMFErrUnderflow:
        case PVMFErrProcessing:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVPlayerEngine::HandleSinkNodeErrorEvent() Sending PVPlayerErrDatapathMediaData for error event %d, Add EH command if not present", event));
            bool ehPending = CheckForPendingErrorHandlingCmd();
            if (ehPending)
            {
                // there should be no error handling queued.
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSinkNodeErrorEvent() Already EH pending, should never happen"));
                return;
            }
            else
            {
                PVMFErrorInfoMessageInterface* nextmsg = NULL;
                if (aEvent.GetEventExtensionInterface() != NULL)
                {
                    nextmsg = GetErrorInfoMessageInterface(*(aEvent.GetEventExtensionInterface()));
                }
                PVUuid puuid = PVPlayerErrorInfoEventTypesUUID;
                iCommandCompleteErrMsgInErrorHandling = OSCL_NEW(PVMFBasicErrorInfoMessage, (PVPlayerErrSinkMediaData, puuid, nextmsg));
                iCommandCompleteStatusInErrorHandling = event;
                AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_GENERAL, NULL, NULL, NULL, false);
                if (iCurrentCmd.empty())
                {
                    // this error was received when no cmd was being processed so send the error event from here
                    SendErrorEvent(iCommandCompleteStatusInErrorHandling,
                                   OSCL_STATIC_CAST(PVInterface*, iCommandCompleteErrMsgInErrorHandling),
                                   aEvent.GetEventData(), aEvent.GetLocalBuffer(), aEvent.GetLocalBufferSize());
                    iCommandCompleteErrMsgInErrorHandling->removeRef();
                    iCommandCompleteErrMsgInErrorHandling = NULL;
                }
            }
        }
        break;

        case PVMFErrTimeout:
        case PVMFErrNoResources:
        case PVMFErrResourceConfiguration:
        case PVMFErrResource:
        case PVMFErrNoMemory:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVPlayerEngine::HandleSinkNodeErrorEvent() Sending PVPlayerErrSinkFatal for error event %d, Add EH command if not present", event));
            bool ehPending = CheckForPendingErrorHandlingCmd();
            if (ehPending)
            {
                // there should be no error handling queued.
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSinkNodeErrorEvent() Already EH pending, should never happen"));
                return;
            }
            else
            {
                PVMFErrorInfoMessageInterface* nextmsg = NULL;
                if (aEvent.GetEventExtensionInterface() != NULL)
                {
                    nextmsg = GetErrorInfoMessageInterface(*(aEvent.GetEventExtensionInterface()));
                }
                PVUuid puuid = PVPlayerErrorInfoEventTypesUUID;
                iCommandCompleteErrMsgInErrorHandling = OSCL_NEW(PVMFBasicErrorInfoMessage, (PVPlayerErrSinkFatal, puuid, nextmsg));
                iCommandCompleteStatusInErrorHandling = event;
                AddCommandToQueue(PVP_ENGINE_COMMAND_ERROR_HANDLING_GENERAL, NULL, NULL, NULL, false);
                if (iCurrentCmd.empty())
                {
                    // this error was received when no cmd was being processed so send the error event from here
                    SendErrorEvent(iCommandCompleteStatusInErrorHandling,
                                   OSCL_STATIC_CAST(PVInterface*, iCommandCompleteErrMsgInErrorHandling),
                                   aEvent.GetEventData(), aEvent.GetLocalBuffer(), aEvent.GetLocalBufferSize());
                    iCommandCompleteErrMsgInErrorHandling->removeRef();
                    iCommandCompleteErrMsgInErrorHandling = NULL;
                }
            }
        }
        break;

        default:
            // Do nothing but log it
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSinkNodeErrorEvent() Do nothing for this event %d", event));
            break;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSinkNodeErrorEvent() Out"));
}

//Return val: true means found/deleted cmd(s).
bool PVPlayerEngine::removeCmdFromQ(Oscl_Vector<PVPlayerEngineCommand, OsclMemAllocator> &aVec, const PVPlayerEngineCommandType aCmdType, bool aRemove)
{
    if (aVec.size() == 0)
    {
        return false;
    }
    // OSCL priority queue doesn't allow index access so will need to
    // go through each pending cmd one-by-one, save commands that are not auto-resume
    // and put the commands back into the pending queue.
    // Vector to hold the pending cmds temporarily
    bool ret = false;
    Oscl_Vector<PVPlayerEngineCommand, OsclMemAllocator> tmpvec;
    tmpvec.reserve(aVec.size());
    tmpvec.clear();
    // Go through each pending command

    for (int i = aVec.size() - 1; i >= 0; i--)
    {
        if (aVec[i].GetCmdType() == aCmdType)
        {
            ret = true;
            if (!aRemove)
            {
                return ret;
            }
            continue;
        }
        tmpvec.push_back(aVec[i]);
    }
    aVec.clear();
    // Put the pending commands back in the priqueue
    while (tmpvec.empty() == false)
    {
        aVec.push_front(tmpvec[0]);
        tmpvec.erase(tmpvec.begin());
    }
    return ret;
}

//Return val: true means found/deleted cmd(s).
bool PVPlayerEngine::removeCmdFromQ(OsclPriorityQueue<PVPlayerEngineCommand, OsclMemAllocator, Oscl_Vector<PVPlayerEngineCommand, OsclMemAllocator>, PVPlayerEngineCommandCompareLess> &aVec, const PVPlayerEngineCommandType aCmdType, bool aRemove)
{
    // OSCL priority queue doesn't allow index access so will need to
    // go through each pending cmd one-by-one, save commands that are not auto-resume
    // and put the commands back into the pending queue.
    // Vector to hold the pending cmds temporarily
    bool ret = false;
    Oscl_Vector<PVPlayerEngineCommand, OsclMemAllocator> tmpvec;
    tmpvec.reserve(aVec.size());
    tmpvec.clear();
    // Go through each pending command
    while (aVec.empty() == false)
    {
        if (aVec.top().GetCmdType() == aCmdType)
        {
            ret = true;
            if (!aRemove)
            {
                return ret;
            }
            aVec.pop();
            continue;
        }
        tmpvec.push_back(aVec.top());
        aVec.pop();
    }
    // Put the pending commands back in the priqueue
    while (tmpvec.empty() == false)
    {
        aVec.push(tmpvec[0]);
        tmpvec.erase(tmpvec.begin());
    }
    return ret;
}

void PVPlayerEngine::HandleSourceNodeInfoEvent(const PVMFAsyncEvent& aEvent)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeInfoEvent() In"));

    if (iState == PVP_ENGINE_STATE_RESETTING)
    {
        //this means error handling, reset or cancelall is still in progress
        //no need to look for info event
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVPlayerEngine::HandleSourceNodeInfoEvent() Return engine in resetting state no need to process the info event %d", aEvent.GetEventType()));
        return;
    }

    PVMFEventType event = aEvent.GetEventType();

    switch (event)
    {
        case PVMFInfoBufferingStart:
        case PVMFInfoBufferingComplete:
        case PVMFInfoOverflow:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeInfoEvent() Sending buffering event %d", event));
            SendInformationalEvent(event, NULL, aEvent.GetEventData(), aEvent.GetLocalBuffer(), aEvent.GetLocalBufferSize());
        }
        break;

        case PVMFInfoBufferingStatus:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeInfoEvent() Sending buffering status event %d", event));
            SendInformationalEvent(event, NULL, aEvent.GetEventData(), aEvent.GetLocalBuffer(), aEvent.GetLocalBufferSize());
        }
        break;

        case PVMFInfoUnderflow:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeInfoEvent() Received PVMFInfoUnderflow"));
            // remove pending auto-resume if there is any
            if (removeCmdFromQ(iPendingCmds, PVP_ENGINE_COMMAND_RESUME_DUE_TO_BUFFER_DATAREADY, true))
            {
                //cancelled the pending auto-resume
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeInfoEvent() PVMFInfoUnderflow got cancelled"));
                break;
            }

            // if no auto-pause in the queue and no auto-pause in progress, add one
            if ((! removeCmdFromQ(iCurrentCmd, PVP_ENGINE_COMMAND_PAUSE_DUE_TO_BUFFER_UNDERFLOW, false))
                    && (! removeCmdFromQ(iPendingCmds, PVP_ENGINE_COMMAND_PAUSE_DUE_TO_BUFFER_UNDERFLOW, false)))
            {
                AddCommandToQueue(PVP_ENGINE_COMMAND_PAUSE_DUE_TO_BUFFER_UNDERFLOW, NULL, NULL, NULL, false);
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeInfoEvent() Received source underflow event, issue auto-pause command"));
            }
        }
        break;

        case PVMFInfoDataReady:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeInfoEvent() Received PVMFInfoDataReady"));
            // remove pending auto-pause if there is any
            if (removeCmdFromQ(iPendingCmds, PVP_ENGINE_COMMAND_PAUSE_DUE_TO_BUFFER_UNDERFLOW, true))
            {
                //cancelled the pending auto-pause
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeInfoEvent() PVMFInfoDataReady got cancelled"));
                break;
            }

            // if no resume in the queue and no resume in progress, add one
            if ((! removeCmdFromQ(iCurrentCmd, PVP_ENGINE_COMMAND_RESUME_DUE_TO_BUFFER_DATAREADY, false))
                    && (! removeCmdFromQ(iPendingCmds, PVP_ENGINE_COMMAND_RESUME_DUE_TO_BUFFER_DATAREADY, false)))
            {
                AddCommandToQueue(PVP_ENGINE_COMMAND_RESUME_DUE_TO_BUFFER_DATAREADY, NULL, NULL, NULL, false);
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeInfoEvent() Received source data ready event, issue auto-resume command"));
            }
        }
        break;

        case PVMFInfoContentLength:
        case PVMFInfoContentType:
        case PVMFInfoContentTruncated:
        case PVMFInfoRemoteSourceNotification:
        case PVMFInfoPlayListClipTransition:
        case PVMFInfoPlayListSwitch:
        {
            PVInterface* intf = NULL;
            PVMFBasicErrorInfoMessage* infomsg = NULL;
            PVMFErrorInfoMessageInterface* nextmsg = NULL;
            if (aEvent.GetEventExtensionInterface() != NULL)
            {
                nextmsg = GetErrorInfoMessageInterface(*(aEvent.GetEventExtensionInterface()));
                PVUuid puuid = PVPlayerErrorInfoEventTypesUUID;
                infomsg =
                    OSCL_NEW(PVMFBasicErrorInfoMessage, (PVPlayerInfoSourceMediaData,
                                                         puuid,
                                                         nextmsg));
                intf = OSCL_STATIC_CAST(PVInterface*, infomsg);
            }
            SendInformationalEvent(event, intf, aEvent.GetEventData(), aEvent.GetLocalBuffer(), aEvent.GetLocalBufferSize());
            if (infomsg != NULL)
            {
                infomsg->removeRef();
            }
            infomsg = NULL;
        }
        break;

        case PVMFInfoTrackDisable:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeInfoEvent() Sending bad track disabled event %d", event));
            SendInformationalEvent(event, NULL, aEvent.GetEventData(), aEvent.GetLocalBuffer(), aEvent.GetLocalBufferSize());
        }
        break;

        case PVMFInfoUnexpectedData:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeInfoEvent() Sending unexpected data event %d", event));
            SendInformationalEvent(event, NULL, aEvent.GetEventData(), aEvent.GetLocalBuffer(), aEvent.GetLocalBufferSize());
        }
        break;

        case PVMFInfoSessionDisconnect:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeInfoEvent() Sending session disconnect %d", event));
            SendInformationalEvent(event, NULL, aEvent.GetEventData(), aEvent.GetLocalBuffer(), aEvent.GetLocalBufferSize());
        }
        break;
        case PVMFInfoMetadataAvailable:
        {
            PVUuid infomsguuid = PVMFMetadataInfoMessageInterfaceUUID;
            PVMFMetadataInfoMessageInterface* eventMsg = NULL;
            PVInterface* infoExtInterface = aEvent.GetEventExtensionInterface();
            if (infoExtInterface &&
                    infoExtInterface->queryInterface(infomsguuid, (PVInterface*&)eventMsg))
            {
                PVUuid eventuuid;
                int32 infoCode;
                eventMsg->GetCodeUUID(infoCode, eventuuid);
                if (eventuuid == infomsguuid)
                {
                    Oscl_Vector<PvmiKvp, OsclMemAllocator> kvpVector = eventMsg->GetMetadataVector();
                    SendInformationalEvent(aEvent.GetEventType(), infoExtInterface, aEvent.GetEventData(), aEvent.GetLocalBuffer(), aEvent.GetLocalBufferSize());
                }
            }
        }
        break;
        case PVMFInfoDurationAvailable:
        {
            PVUuid infomsguuid = PVMFDurationInfoMessageInterfaceUUID;
            PVMFDurationInfoMessageInterface* eventMsg = NULL;
            PVInterface* infoExtInterface = aEvent.GetEventExtensionInterface();
            PVInterface* temp = NULL;
            if (infoExtInterface &&
                    infoExtInterface->queryInterface(infomsguuid, temp))
            {
                PVUuid eventuuid;
                int32 infoCode;
                eventMsg = OSCL_STATIC_CAST(PVMFDurationInfoMessageInterface*, temp);
                eventMsg->GetCodeUUID(infoCode, eventuuid);
                if (eventuuid == infomsguuid)
                {
                    iSourceDurationInMS = eventMsg->GetDuration();
                    iSourceDurationAvailable = true;
                    SendInformationalEvent(aEvent.GetEventType(), infoExtInterface, aEvent.GetEventData(), aEvent.GetLocalBuffer(), aEvent.GetLocalBufferSize());
                }
            }
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeInfoEvent() Sending duration available %d", event));
        }
        break;

        case PVMFInfoPoorlyInterleavedContent:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeInfoEvent() Sending Poorly Interleaved Content Info %d", event));
            SendInformationalEvent(event, NULL, aEvent.GetEventData(), aEvent.GetLocalBuffer(), aEvent.GetLocalBufferSize());
        }
        break;
        case PVMFInfoSourceOverflow:
        {
            PVPPlaybackPosition aBeginPos;
            aBeginPos.iIndeterminate = false;
            aBeginPos.iPosUnit = PVPPBPOSUNIT_MILLISEC;
            aBeginPos.iPosValue.millisec_value = 0;
            PVPPlaybackPosition aEndPos;
            aEndPos.iIndeterminate = true;
            iOverflowFlag = true;
            SetPlaybackRange(aBeginPos, aEndPos, false);
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeInfoEvent() Received source overflow event, issue auto-reposition command"));
        }
        break;
        case PVMFInfoEndOfData:
        default:
            // Do nothing but log it
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeInfoEvent() Do nothing for this event %d", event));
            break;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSourceNodeInfoEvent() Out"));
}


void PVPlayerEngine::HandleDecNodeInfoEvent(const PVMFAsyncEvent& aEvent, int32 aDatapathIndex)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleDecNodeInfoEvent() In"));

    if (iState == PVP_ENGINE_STATE_RESETTING)
    {
        //this means error handling, reset or cancelall is still in progress
        //no need to look for info event
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVPlayerEngine::HandleDecNodeInfoEvent() Return engine in resetting state no need to process the info event %d", aEvent.GetEventType()));
        return;
    }

    OSCL_UNUSED_ARG(aDatapathIndex);

    PVMFEventType event = aEvent.GetEventType();

    switch (event)
    {
        case PVMFInfoProcessingFailure:

            SendInformationalEvent(PVMFInfoProcessingFailure);
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleDecNodeInfoEvent() Report ProcessingFailure event %d", event));
            break;

        case PVMFInfoOverflow:
        case PVMFInfoEndOfData:
        default:
            // Do nothing but log it
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleDecNodeInfoEvent() Do nothing for this event %d", event));
            break;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleDecNodeInfoEvent() Out"));
}


void PVPlayerEngine::HandleSinkNodeInfoEvent(const PVMFAsyncEvent& aEvent, int32 aDatapathIndex)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSinkNodeInfoEvent() In"));

    if (iState == PVP_ENGINE_STATE_RESETTING)
    {
        //this means error handling, reset or cancelall is still in progress
        //no need to look for info event
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVPlayerEngine::HandleSinkNodeInfoEvent() Return engine in resetting state no need to process the info event %d", aEvent.GetEventType()));
        return;
    }

    PVMFEventType event = aEvent.GetEventType();

    switch (event)
    {
        case PVMFInfoStartOfData:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSinkNodeInfoEvent() PVMFInfoStartOfData event received for %s",
                            iDatapathList[aDatapathIndex].iTrackInfo->getTrackMimeType().get_cstr()));

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iReposLogger, PVLOGMSG_INFO, (0, "PVPlayerEngine::HandleSinkNodeInfoEvent() PVMFInfoStartOfData event received for %s, iNumPendingSkipCompleteEvent=%d iNumPVMFInfoStartOfDataPending=%d",
                            iDatapathList[aDatapathIndex].iTrackInfo->getTrackMimeType().get_cstr(),
                            iNumPendingSkipCompleteEvent, iNumPVMFInfoStartOfDataPending));

            uint32 *data = (uint32*) aEvent.GetEventData();
            uint32 streamID = *data;

            if (streamID != iStreamID)
            {
                // received StartOfData for previous streamId, ignoring these events
                break;
            }

            if (iNumPVMFInfoStartOfDataPending > 0)
            {
                --iNumPVMFInfoStartOfDataPending;
            }

            if ((iNumPendingSkipCompleteEvent == 0) && (iNumPVMFInfoStartOfDataPending == 0))
            {
                if (iWatchDogTimer->IsBusy())
                {
                    iWatchDogTimer->Cancel();
                }
                //check engine internal state here prior to starting the clock
                //this is to make sure that we do not start the clock in case engine is still
                //auto-paused (think usecase: auto-pause, setplaybackrange, auto-resume)
                if (iState == PVP_ENGINE_STATE_STARTED)
                {
                    // start the clock only if engine is in started state
                    StartPlaybackClock();
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iReposLogger, PVLOGMSG_INFO, (0, "PVPlayerEngine::HandleSinkNodeInfoEvent() - PlayClock Started"));
                }
            }
            //else it could mean duplicate or old PVMFInfoStartOfData, ignore both
        }
        break;
        case PVMFInfoEndOfData:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSinkNodeInfoEvent() PVMFInfoEndOfData event received for %s",
                            iDatapathList[aDatapathIndex].iTrackInfo->getTrackMimeType().get_cstr()));

            uint32 *data = (uint32*) aEvent.GetEventData();
            uint32 streamID = *data;

            if (streamID != iStreamID)
            {
                // received EndOfData for previous streamId, ignoring these events
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iReposLogger, PVLOGMSG_INFO, (0, "PVPlayerEngine::HandleSinkNodeInfoEvent() PVMFInfoEndOfData event received for %s with StreamID=%d, Engine StreamID=%d",
                                iDatapathList[aDatapathIndex].iTrackInfo->getTrackMimeType().get_cstr(),
                                streamID, iStreamID));
                break;
            }

            if (iDatapathList[aDatapathIndex].iDatapath && iDatapathList[aDatapathIndex].iEndOfDataReceived == false)
            {
                iDatapathList[aDatapathIndex].iEndOfDataReceived = true;
                // If all datapath received EOS, initiate a pause-due-to-EOS
                if (AllDatapathReceivedEndOfData() == true)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSinkNodeInfoEvent() Issue Pause due to end of clip"));
                    AddCommandToQueue(PVP_ENGINE_COMMAND_PAUSE_DUE_TO_ENDOFCLIP, NULL, NULL, NULL, false);
                }
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::HandleSinkNodeInfoEvent() PVMFInfoEndOfData event received for non-active or already-ended datapath. Asserting"));
                OSCL_ASSERT(false);
            }
        }
        break;

        case PVMFInfoDataDiscarded:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSinkNodeInfoEvent() PVMFInfoDataDiscarded event received"));
            SendInformationalEvent(event, NULL, aEvent.GetEventData(), aEvent.GetLocalBuffer(), aEvent.GetLocalBufferSize());
            break;

        case PVMFInfoVideoTrackFallingBehind:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSinkNodeInfoEvent() PVMFInfoVideoTrackFallingBehind event received"));
            SendInformationalEvent(event, NULL, aEvent.GetEventData(), aEvent.GetLocalBuffer(), aEvent.GetLocalBufferSize());
            break;

        default:
            // Do nothing but log it
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSinkNodeInfoEvent() Do nothing for this event %d", event));
            break;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::HandleSinkNodeInfoEvent() Out"));
}


bool PVPlayerEngine::AllDatapathReceivedEndOfData()
{
    // Return false if any active datapath hasn't
    // received EOS yet. Else true.
    for (uint32 i = 0; i < iDatapathList.size(); ++i)
    {
        if (iDatapathList[i].iDatapath &&
                iDatapathList[i].iEndOfDataReceived == false)
        {
            return false;
        }
    }

    return true;
}


void PVPlayerEngine::SendEndOfClipInfoEvent(PVMFStatus aStatus, PVInterface* aExtInterface)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::SendEndOfClipInfoEvent() In"));

    // If paused succeeded or already paused
    if (aStatus == PVMFSuccess || aStatus == PVMFErrInvalidState)
    {
        // Set the flag so we can disable resume unless user stops, repositions, or changes directrion
        iPlaybackPausedDueToEndOfClip = true;

        PVUuid puuid = PVPlayerErrorInfoEventTypesUUID;
        PVMFBasicErrorInfoMessage* infomsg = OSCL_NEW(PVMFBasicErrorInfoMessage, (PVPlayerInfoEndOfClipReached, puuid, NULL));
        SendInformationalEvent(PVMFInfoEndOfData, OSCL_STATIC_CAST(PVInterface*, infomsg));
        infomsg->removeRef();
    }
    else
    {
        SendErrorEvent(aStatus, aExtInterface);
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::SendEndOfClipInfoEvent() Out"));
}


void PVPlayerEngine::SendEndTimeReachedInfoEvent(PVMFStatus aStatus, PVInterface* aExtInterface)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::SendEndTimeReachedInfoEvent() In"));

    if (aStatus == PVMFSuccess)
    {
        PVUuid puuid = PVPlayerErrorInfoEventTypesUUID;
        PVMFBasicErrorInfoMessage* infomsg = OSCL_NEW(PVMFBasicErrorInfoMessage, (PVPlayerInfoEndTimeReached, puuid, NULL));
        SendInformationalEvent(PVMFInfoEndOfData, OSCL_STATIC_CAST(PVInterface*, infomsg));
        infomsg->removeRef();
    }
    else
    {
        SendErrorEvent(aStatus, aExtInterface);
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::SendEndTimeReachedInfoEvent() Out"));
}


void PVPlayerEngine::SendSourceUnderflowInfoEvent(PVMFStatus aStatus, PVInterface* aExtInterface)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::SendSourceUnderflowInfoEvent() In"));

    if (aStatus == PVMFSuccess || aStatus == PVMFErrNotSupported)
    {
        if (iDataReadySent)
        {
            iDataReadySent = false;
            SendInformationalEvent(PVMFInfoUnderflow);
        }
    }
    else if (aStatus == PVMFErrCancelled)
    {
        // Do nothing since the auto-pause was cancelled
    }
    else
    {
        SendErrorEvent(aStatus, aExtInterface);
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::SendSourceUnderflowInfoEvent() Out"));
}


void PVPlayerEngine::SendSourceDataReadyInfoEvent(PVMFStatus aStatus, PVInterface* aExtInterface)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::SendSourceDataReadyInfoEvent() In"));


    if (aStatus == PVMFSuccess || aStatus == PVMFErrNotSupported)
    {
        // send DataReady event only if it is not sent earlier and if EOS is not reported.
        // It is possible that for Streaming repositioning to end use-cases engine receives
        // EOS before prepare completes, in that case suppress DataReady event.
        if (!iDataReadySent && !iPlaybackPausedDueToEndOfClip)
        {
            iDataReadySent = true;
            SendInformationalEvent(PVMFInfoDataReady);
        }
    }
    else if (aStatus == PVMFErrCancelled)
    {
        // Do nothing since the auto-resume was cancelled
    }
    else
    {
        SendErrorEvent(aStatus, aExtInterface);
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::SendSourceDataReadyInfoEvent() Out"));
}


PVMFErrorInfoMessageInterface* PVPlayerEngine::GetErrorInfoMessageInterface(PVInterface& aInterface)
{
    PVMFErrorInfoMessageInterface* extiface = NULL;
    PVInterface* temp = NULL;
    if (aInterface.queryInterface(PVMFErrorInfoMessageInterfaceUUID, temp))
    {
        extiface = OSCL_STATIC_CAST(PVMFErrorInfoMessageInterface*, temp);
        return extiface;
    }
    else
    {
        return NULL;
    }
}


void PVPlayerEngine::StartPlaybackStatusTimer(void)
{
    // Check if playback position reporting is enabled
    if (!iPBPosEnable || iPlayStatusTimerEnabled)
        return;
    // To get regular play status events
    iPlayStatusTimerEnabled = true;

    iClockNotificationsInf->SetCallbackDeltaTime(iPBPosStatusInterval,
            iPlayStatusCallbackTimerMarginWindow, (PVMFMediaClockNotificationsObs*)this, false, NULL,
            iPlayStatusCallbackTimerID);
}


void PVPlayerEngine::StopPlaybackStatusTimer(void)
{

    // Stop the playback position status timer
    iPlayStatusTimerEnabled = false;
    if (iClockNotificationsInf && iPlayStatusCallbackTimerID)
    {
        iClockNotificationsInf->CancelCallback(iPlayStatusCallbackTimerID, false);
        iPlayStatusCallbackTimerID = 0;
    }
}

bool PVPlayerEngine::CheckForSourceRollOver()
{
    uint32 alternates = iDataSource->GetNumAlternateSourceFormatTypes();
    if ((alternates > 0) && (iAlternateSrcFormatIndex < alternates))
    {
        return true;
    }
    return false;
}

PVMFStatus PVPlayerEngine::SetupDataSourceForUnknownURLAccess()
{
    if (iDataSource == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::SetupDataSourceForUnknownURLAccess() - No Data Source"));
        return PVMFErrInvalidState;
    }
    else
    {
        /*
         * In case of unknown url, here is the sequence that engine would attempt:
         *  1) First Alternate source format would be PVMF_DATA_SOURCE_RTSP_URL,
         *  implying that we would attempt a RTSP streaming session
         *
         *	2) Primary source format would be set to PVMF_DATA_SOURCE_HTTP_URL,
         *  implying that we would attempt a progressive download first.
         *
         *	3) Second Alternate source format would be PVMF_DATA_SOURCE_REAL_HTTP_CLOAKING_URL,
         *  implying that we would attempt a real media cloaking session
         *
         *  4) Third alternate source format would be PVMF_DATA_SOURCE_MS_HTTP_STREAMING_URL,
         *  implying that we would attempt a MS HTTP streaming session
         */
        iSourceFormatType = PVMF_MIME_DATA_SOURCE_RTSP_URL; ;
        if (iDataSource->SetAlternateSourceFormatType(PVMF_MIME_DATA_SOURCE_HTTP_URL) != true)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::SetupDataSourceForUnknownURLAccess() - SetAlternateSourceFormatType Failed"));
            return PVMFFailure;
        }
        if (iDataSource->SetAlternateSourceFormatType(PVMF_MIME_DATA_SOURCE_REAL_HTTP_CLOAKING_URL) != true)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::SetupDataSourceForUnknownURLAccess() - SetAlternateSourceFormatType Failed"));
            return PVMFFailure;
        }
        if (iDataSource->SetAlternateSourceFormatType(PVMF_MIME_DATA_SOURCE_MS_HTTP_STREAMING_URL) != true)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::SetupDataSourceForUnknownURLAccess() - SetAlternateSourceFormatType Failed"));
            return PVMFFailure;
        }
        return PVMFSuccess;
    }
}

/* scan thru aKvpValue, create a new PvmiKvp object and push
 * than onto iPvmiKvpCapNConfig.
 *  return PVMFSuccess unless allocation error
 */
PVMFStatus PVPlayerEngine:: VerifyAndSaveKVPValues(PvmiKvp *aKvpValue)
{

    PvmiKvp* KvpCapNConfig = (PvmiKvp *)OSCL_MALLOC(sizeof(PvmiKvp));
    if (! KvpCapNConfig)  return PVMFErrNoMemory;
    oscl_memcpy(KvpCapNConfig, aKvpValue, sizeof(PvmiKvp));


    KvpCapNConfig->key = (char*)OSCL_MALLOC(oscl_strlen(aKvpValue->key) + 1);
    if (! KvpCapNConfig->key)  return PVMFErrNoMemory;
    oscl_strncpy(KvpCapNConfig->key, aKvpValue->key, oscl_strlen(aKvpValue->key) + 1);


    // all the values have copied automatically so just need to allocate memory of pointer type.

    if (oscl_strstr(aKvpValue->key, _STRLIT_CHAR("valtype=wchar*")) != NULL)
    {
        KvpCapNConfig->value.pWChar_value = (oscl_wchar*)OSCL_MALLOC((oscl_strlen(aKvpValue->value.pWChar_value) + 1) * sizeof(oscl_wchar));
        if (! KvpCapNConfig->value.pWChar_value)  return PVMFErrNoMemory;
        oscl_strncpy(KvpCapNConfig->value.pWChar_value, aKvpValue->value.pWChar_value, oscl_strlen(aKvpValue->value.pWChar_value) + 1);

    }
    else if (oscl_strstr(aKvpValue->key, _STRLIT_CHAR("valtype=char*")) != NULL)
    {
        KvpCapNConfig->value.pChar_value = (char*)OSCL_MALLOC(oscl_strlen(aKvpValue->value.pChar_value) + 1);
        if (! KvpCapNConfig->value.pChar_value)  return PVMFErrNoMemory;
        oscl_strncpy(KvpCapNConfig->value.pChar_value, aKvpValue->value.pChar_value, oscl_strlen(aKvpValue->value.pChar_value) + 1);

    }
    else if (oscl_strstr(aKvpValue->key, _STRLIT_CHAR("valtype=uint8*")) != NULL)
    {
        KvpCapNConfig->value.pUint8_value = (uint8*)OSCL_MALLOC(oscl_strlen((char *)aKvpValue->value.pUint8_value) + 1);
        if (! KvpCapNConfig->value.pUint8_value)  return PVMFErrNoMemory;
        oscl_memcpy(KvpCapNConfig->value.pUint8_value, aKvpValue->value.pUint8_value, oscl_strlen((char *)aKvpValue->value.pUint8_value) + 1);

    }
    else if (oscl_strstr(aKvpValue->key, _STRLIT_CHAR("valtype=int32*")) != NULL)
    {
        KvpCapNConfig->value.pInt32_value = (int32*)OSCL_MALLOC(sizeof(int32));
        if (! KvpCapNConfig->value.pInt32_value)  return PVMFErrNoMemory;
        oscl_memcpy(KvpCapNConfig->value.pInt32_value, aKvpValue->value.pInt32_value, sizeof(int32));
        // @TODO: Future support will be based on length , currently support only one element
    }
    else if (oscl_strstr(aKvpValue->key, _STRLIT_CHAR("valtype=uint32*")) != NULL)
    {
        KvpCapNConfig->value.pUint32_value = (uint32*)OSCL_MALLOC(sizeof(uint32));
        if (! KvpCapNConfig->value.pUint32_value)  return PVMFErrNoMemory;
        oscl_memcpy(KvpCapNConfig->value.pUint32_value, aKvpValue->value.pUint32_value, sizeof(uint32));
        // @TODO: Future support will be based on length, currently support only one element
    }
    else if (oscl_strstr(aKvpValue->key, _STRLIT_CHAR("valtype=int64*")) != NULL)
    {
        KvpCapNConfig->value.pInt64_value = (int64*)OSCL_MALLOC(sizeof(int64));
        if (! KvpCapNConfig->value.pInt64_value)  return PVMFErrNoMemory;
        oscl_memcpy(KvpCapNConfig->value.pInt64_value, aKvpValue->value.pInt64_value, sizeof(int64));
        // @TODO: Future support will be based on length, currently support only one element
    }
    else if (oscl_strstr(aKvpValue->key, _STRLIT_CHAR("valtype=uint64*")) != NULL)
    {
        KvpCapNConfig->value.pUint64_value = (uint64*)OSCL_MALLOC(sizeof(uint64));
        if (! KvpCapNConfig->value.pUint64_value)  return PVMFErrNoMemory;
        oscl_memcpy(KvpCapNConfig->value.pUint64_value, aKvpValue->value.pUint64_value, sizeof(uint64));
        // @TODO: Future support will be based on length, currently support only one element
    }
    else if (oscl_strstr(aKvpValue->key, _STRLIT_CHAR("valtype=float*")) != NULL)
    {
        KvpCapNConfig->value.pFloat_value = (float*)OSCL_MALLOC(sizeof(float));
        if (! KvpCapNConfig->value.pFloat_value)  return PVMFErrNoMemory;
        oscl_memcpy(KvpCapNConfig->value.pFloat_value, aKvpValue->value.pFloat_value, sizeof(float));
        // @TODO: Future support will be based on length, currently support only one element
    }
    else if (oscl_strstr(aKvpValue->key, _STRLIT_CHAR("valtype=double*")) != NULL)
    {
        KvpCapNConfig->value.pDouble_value = (double*)OSCL_MALLOC(sizeof(double));
        if (! KvpCapNConfig->value.pDouble_value)  return PVMFErrNoMemory;
        oscl_memcpy(KvpCapNConfig->value.pDouble_value, aKvpValue->value.pDouble_value, sizeof(double));
        // @TODO: Future support will be based on length, currently support only one element
    }
    else if (oscl_strstr(aKvpValue->key, _STRLIT_CHAR("valtype=range_int32")) != NULL)
    {
        KvpCapNConfig->value.key_specific_value = (void*)OSCL_MALLOC(sizeof(range_int32));
        if (! KvpCapNConfig->value.key_specific_value)  return PVMFErrNoMemory;
        oscl_memcpy(KvpCapNConfig->value.key_specific_value, aKvpValue->value.key_specific_value, sizeof(range_int32));
        // @TODO: Future support will be based on length, currently support only one element
    }
    else if (oscl_strstr(aKvpValue->key, _STRLIT_CHAR("valtype=range_uint32")) != NULL)
    {
        KvpCapNConfig->value.key_specific_value = (void*)OSCL_MALLOC(sizeof(range_uint32));
        if (! KvpCapNConfig->value.key_specific_value)  return PVMFErrNoMemory;
        oscl_memcpy(KvpCapNConfig->value.key_specific_value, aKvpValue->value.key_specific_value, sizeof(range_uint32));
        // @TODO: Future support will be based on length, currently support only one element
    }

    iPvmiKvpCapNConfig.push_back(KvpCapNConfig);
    return PVMFSuccess;
}


void PVPlayerEngine::SetRollOverKVPValues()
{
    PvmiKvp *SaveKvp;
    PvmiKvp *ErrorKVP;

    for (uint i = 0; i < iPvmiKvpCapNConfig.size(); i++)
    {
        SaveKvp = iPvmiKvpCapNConfig[i];
        setParametersSync(NULL, SaveKvp, 1 , ErrorKVP);

        if (ErrorKVP != NULL)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::SetRollOverKVPValues() Configuring cap-config failed"));
        }
    }

}

void PVPlayerEngine::DeleteKVPValues()
{
    uint i = 0;
    while (i < iPvmiKvpCapNConfig.size())
    {
        PvmiKvp **Setkvp = iPvmiKvpCapNConfig.begin();

        if (oscl_strstr((*Setkvp)->key, _STRLIT_CHAR("valtype=wchar*")) != NULL)
        {
            OSCL_FREE((*Setkvp)->value.pWChar_value);
            (*Setkvp)->value.pWChar_value = NULL;

        }
        else if (oscl_strstr((*Setkvp)->key, _STRLIT_CHAR("valtype=char*")) != NULL)
        {
            OSCL_FREE((*Setkvp)->value.pChar_value);
            (*Setkvp)->value.pChar_value = NULL;
        }
        else if (oscl_strstr((*Setkvp)->key, _STRLIT_CHAR("valtype=uint8*")) != NULL)
        {
            OSCL_FREE((*Setkvp)->value.pUint8_value);
            (*Setkvp)->value.pUint8_value = NULL;
        }

        else if (oscl_strstr((*Setkvp)->key, _STRLIT_CHAR("valtype=int32*")) != NULL)
        {
            OSCL_FREE((*Setkvp)->value.pInt32_value);
            (*Setkvp)->value.pInt32_value = NULL;

        }
        else if (oscl_strstr((*Setkvp)->key, _STRLIT_CHAR("valtype=uint32*")) != NULL)
        {
            OSCL_FREE((*Setkvp)->value.pUint32_value);
            (*Setkvp)->value.pUint32_value = NULL;

        }
        else if (oscl_strstr((*Setkvp)->key, _STRLIT_CHAR("valtype=int64*")) != NULL)
        {
            OSCL_FREE((*Setkvp)->value.pInt64_value);
            (*Setkvp)->value.pInt64_value = NULL;

        }
        else if (oscl_strstr((*Setkvp)->key, _STRLIT_CHAR("valtype=uint64*")) != NULL)
        {
            OSCL_FREE((*Setkvp)->value.pUint64_value);
            (*Setkvp)->value.pUint64_value = NULL;
        }
        else if (oscl_strstr((*Setkvp)->key, _STRLIT_CHAR("valtype=float*")) != NULL)
        {
            OSCL_FREE((*Setkvp)->value.pFloat_value);
            (*Setkvp)->value.pFloat_value = NULL;
        }
        else if (oscl_strstr((*Setkvp)->key, _STRLIT_CHAR("valtype=double*")) != NULL)
        {
            OSCL_FREE((*Setkvp)->value.pDouble_value);
            (*Setkvp)->value.pDouble_value = NULL;
        }
        else if ((oscl_strstr((*Setkvp)->key, _STRLIT_CHAR("valtype=range_int32")) != NULL) ||
                 (oscl_strstr((*Setkvp)->key, _STRLIT_CHAR("valtype=range_uint32")) != NULL))
        {
            OSCL_FREE((*Setkvp)->value.key_specific_value);
            (*Setkvp)->value.key_specific_value = NULL;
        }

        OSCL_FREE((*Setkvp)->key);
        (*Setkvp)->key = NULL;

        OSCL_FREE(*Setkvp);
        *Setkvp = NULL;

        iPvmiKvpCapNConfig.erase(iPvmiKvpCapNConfig.begin());
    }

}


void PVPlayerEngine::PVPlayerWatchdogTimerEvent()
{
    //check engine internal state here prior to starting the clock
    //this is to make sure that we do not start the clock in case engine is still
    //auto-paused (think usecase: auto-pause, setplaybackrange, auto-resume)
    if (iState == PVP_ENGINE_STATE_STARTED)
    {
        // start the clock only if engine is in started state
        StartPlaybackClock();
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerEngine::PVPlayerWatchdogTimerEvent() WatchDog timer expired"));
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iReposLogger, PVLOGMSG_INFO, (0, "PVPlayerEngine::PVPlayerWatchdogTimerEvent() WatchDog timer expired"));
    }
}

void PVPlayerEngine::StartPlaybackClock()
{
    if (iWatchDogTimer->IsBusy())
    {
        iWatchDogTimer->Cancel();
    }

    if (iPlaybackClock.GetState() == PVMFMediaClock::RUNNING)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING, (0, "PVPlayerEngine::StartPlaybackClock() clock already started"));
        return;
    }

    iPlaybackClock.Start();
    // Notify all sink nodes that have sync control IF that clock has started
    for (uint32 i = 0; i < iDatapathList.size(); ++i)
    {
        if (iDatapathList[i].iDatapath && iDatapathList[i].iSinkNodeSyncCtrlIF)
        {
            iDatapathList[i].iSinkNodeSyncCtrlIF->ClockStarted();
        }
    }

    // To get regular play status events
    StartPlaybackStatusTimer();

    // Restart the end time check if enabled
    if (iEndTimeCheckEnabled)
    {
        // Determine the check cycle based on interval setting in milliseconds
        // and timer frequency of 100 millisec
        int32 checkcycle = iEndTimeCheckInterval / 100;
        if (checkcycle == 0)
        {
            ++checkcycle;
        }
        iPollingCheckTimer->Cancel(PVPLAYERENGINE_TIMERID_ENDTIMECHECK);
        iPollingCheckTimer->Request(PVPLAYERENGINE_TIMERID_ENDTIMECHECK, 0, checkcycle, this, true);
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::StartPlaybackClock() StartNPT %d StartTS %d", iStartNPT, iStartMediaDataTS));
}

PVMFStatus PVPlayerEngine::GetCompleteList(PVMFMediaPresentationInfo& aList)
{
    if (iSourceNodeTrackSelIF)
    {
        PVPlayerState state = GetPVPlayerState();
        if ((state == PVP_STATE_INITIALIZED) ||
                (state == PVP_STATE_PREPARED) ||
                (state == PVP_STATE_STARTED) ||
                (state == PVP_STATE_PAUSED))
        {
            aList.Reset();
            PVMFStatus retval = PVMFFailure;
            int32 leavecode = 0;
            OSCL_TRY(leavecode, retval = iSourceNodeTrackSelIF->GetMediaPresentationInfo(aList));
            OSCL_FIRST_CATCH_ANY(leavecode,
                                 PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::GetCompleteList() GetMediaPresentationInfo on iSourceNodeTrackSelIF did a leave!"));
                                 return PVMFFailure);
            if (retval != PVMFSuccess)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::GetCompleteList() GetMediaPresentationInfo() call on source node failed"));
            }
            return retval;
        }
    }
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::GetCompleteList() iSourceNodeTrackSelIF Invalid"));
    return PVMFFailure;
}

PVMFStatus PVPlayerEngine::ReleaseCompleteList(PVMFMediaPresentationInfo& aList)
{
    aList.Reset();
    return PVMFSuccess;
}

PVMFStatus PVPlayerEngine::GetPlayableList(PVMFMediaPresentationInfo& aList)
{
    PVPlayerState state = GetPVPlayerState();
    if ((state == PVP_STATE_PREPARED) ||
            (state == PVP_STATE_STARTED) ||
            (state == PVP_STATE_PAUSED))
    {
        aList = iPlayableList;
        if (aList.getNumTracks() == 0)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::GetPlayableList() No tracks"));
            return PVMFFailure;
        }
        return PVMFSuccess;
    }
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::GetPlayableList() Invalid Engine State"));
    return PVMFErrInvalidState;
}

PVMFStatus PVPlayerEngine::ReleasePlayableList(PVMFMediaPresentationInfo& aList)
{
    aList.Reset();
    return PVMFSuccess;
}

PVMFStatus PVPlayerEngine::GetSelectedList(PVMFMediaPresentationInfo& aList)
{
    PVPlayerState state = GetPVPlayerState();
    if ((state == PVP_STATE_PREPARED) ||
            (state == PVP_STATE_STARTED) ||
            (state == PVP_STATE_PAUSED))
    {
        aList.Reset();
        aList.setPresentationType(iPlayableList.getPresentationType());
        aList.setSeekableFlag(iPlayableList.IsSeekable());
        aList.SetDurationAvailable(iPlayableList.IsDurationAvailable());
        aList.setDurationValue(iPlayableList.getDurationValue());
        aList.setDurationTimeScale(iPlayableList.getDurationTimeScale());
        for (uint32 i = 0; i < iDatapathList.size(); ++i)
        {
            if (iDatapathList[i].iTrackInfo != NULL)
            {
                aList.addTrackInfo(*(iDatapathList[i].iTrackInfo));
            }
        }
        if (aList.getNumTracks() == 0)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::GetSelectedList() No tracks"));
            return PVMFFailure;
        }
        return PVMFSuccess;
    }
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::GetSelectedList() Invalid Engine State"));
    return PVMFErrInvalidState;
}

PVMFStatus PVPlayerEngine::ReleaseSelectedList(PVMFMediaPresentationInfo& aList)
{
    aList.Reset();
    return PVMFSuccess;
}

PVMFStatus PVPlayerEngine::RegisterHelperObject(PVMFTrackSelectionHelper* aObject)
{
    if (aObject != NULL)
    {
        if (iTrackSelectionHelper != NULL)
        {
            return PVMFErrAlreadyExists;
        }
    }
    iTrackSelectionHelper = aObject;
    return PVMFSuccess;
}

void PVPlayerEngine::ResetReposVariables(bool aResetAll)
{
    if (aResetAll)
    {
        iStreamID = 0;
    }
    if (iWatchDogTimer != NULL)
    {
        if (iWatchDogTimer->IsBusy())
        {
            iWatchDogTimer->Cancel();
        }
    }
    iNumPendingSkipCompleteEvent = 0;
    iNumPVMFInfoStartOfDataPending = 0;
    iTargetNPT = 0;
    iActualNPT = 0;
    iActualMediaDataTS = 0;
    iSkipMediaDataTS = 0;
}

PVMFStatus PVPlayerEngine::DoErrorHandling()
{
    //pls note that we come into this method twice, in case of error handling
    //first time, we come here to start the error handling seq (cancelall if any, followed by reset)
    //second time, we come here when all resets are complete to cleanup and send command completes
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoErrorHandling() In"));
    // Stop the playback clock
    iPlaybackClock.Stop();

    // 1st check if anything needs to be cancelled on Source Node or Datapaths
    if (!iCurrentContextList.empty())
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVPlayerEngine::DoErrorHandling() Some Command is being processed, cancel it"));
        for (uint32 i = 0; i < iCurrentContextList.size(); ++i)
        {
            if (iCurrentContextList[i]->iNode)
            {
                if (iCurrentContextList[i]->iNode == iSourceNode)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerEngine::DoErrorHandling() Cmd Pending on Source node, should never happend, asserting"));
                    OSCL_ASSERT(false);
                }
            }
        }

        // error handling code set engine state to resetting
        SetEngineState(PVP_ENGINE_STATE_RESETTING);
        iRollOverState = RollOverStateIdle; //reset roll over state to Idle, as engine is resetting itself
        // Since there is a pending node or datapath, cancel it
        PVMFStatus status = DoCancelPendingNodeDatapathCommand();
        if (status == PVMFPending)
        {
            // There are some commands which need to be cancelled so wait for cancel complete
            // once cancels complete, we would start the reset sequence from either
            // NodeCommandComplete, HandlePlayerDataPathEvent, RecognizeComplete
            return PVMFPending;
        }
        // if there is nothing to cancel move forward to reset.
    }

    // move on to resetting Source Nodes and Datapaths
    if (iSourceNode)
    {
        int32 leavecode = 0;
        // call reset on source node if not in created state
        if (iSourceNode->GetState() != EPVMFNodeCreated)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVPlayerEngine::DoErrorHandling() Issue reset on Source Node"));
            // error handling code set engine state to resetting
            SetEngineState(PVP_ENGINE_STATE_RESETTING);
            iRollOverState = RollOverStateIdle; //reset roll over state to Idle, as engine is resetting itself

            PVPlayerEngineContext* context = AllocateEngineContext(NULL, iSourceNode, NULL, -1, NULL, -1);

            PVMFCommandId cmdid = -1;
            leavecode = 0;
            OSCL_TRY(leavecode, cmdid = iSourceNode->Reset(iSourceNodeSessionId, (OsclAny*)context));
            OSCL_FIRST_CATCH_ANY(leavecode,

                                 PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                                 (0, "PVPlayerEngine::DoErrorHandling() Reset on iSourceNode did a leave!"));
                                 FreeEngineContext(context);
                                 OSCL_ASSERT(false);
                                 return PVMFFailure);

            return PVMFPending;
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                    (0, "PVPlayerEngine::DoErrorHandling() Source node is deleted or in created state, so start removing sinks"));

    // Now delete the datapath.
    DoRemoveAllSinks();

    // finally do the source node cleanup
    if (iDataSource)
    {
        RemoveDataSourceSync(*iDataSource);
    }

    SetEngineState(PVP_ENGINE_STATE_IDLE);

    // Send the command completion if there is any command in Current command
    if (!iCurrentCmd.empty())
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVPlayerEngine::DoErrorHandling() Complete the engine command being processed"));
        if (iCommandCompleteErrMsgInErrorHandling)
        {
            EngineCommandCompleted(iCurrentCmd[0].GetCmdId(),
                                   iCurrentCmd[0].GetContext(),
                                   iCommandCompleteStatusInErrorHandling,
                                   OSCL_STATIC_CAST(PVInterface*, iCommandCompleteErrMsgInErrorHandling));
            iCommandCompleteErrMsgInErrorHandling->removeRef();
            iCommandCompleteErrMsgInErrorHandling = NULL;
        }
        else
        {
            EngineCommandCompleted(iCurrentCmd[0].GetCmdId(),
                                   iCurrentCmd[0].GetContext(),
                                   iCommandCompleteStatusInErrorHandling);
        }
    }

    // just send the error handling complete event
    SendInformationalEvent(PVMFInfoErrorHandlingComplete, NULL);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,	(0, "PVPlayerEngine::DoErrorHandling() Out"));

    return PVMFSuccess;
}

bool PVPlayerEngine::CheckForPendingErrorHandlingCmd()
{
    //if an error handling cmd had been queued previously
    //it must be in top, since error handling cmds have the
    //highest priority and pending cmds queue is a priority
    //queue
    bool retval = false;
    if (!iPendingCmds.empty())
    {
        switch (iPendingCmds.top().GetCmdType())
        {
            case PVP_ENGINE_COMMAND_ERROR_HANDLING_ADD_DATA_SOURCE:
            case PVP_ENGINE_COMMAND_ERROR_HANDLING_INIT:
            case PVP_ENGINE_COMMAND_ERROR_HANDLING_PREPARE:
            case PVP_ENGINE_COMMAND_ERROR_HANDLING_PAUSE:
            case PVP_ENGINE_COMMAND_ERROR_HANDLING_RESUME:
            case PVP_ENGINE_COMMAND_ERROR_HANDLING_SET_PLAYBACK_RANGE:
            case PVP_ENGINE_COMMAND_ERROR_HANDLING_SET_PLAYBACK_RATE:
            case PVP_ENGINE_COMMAND_ERROR_HANDLING_STOP:
            case PVP_ENGINE_COMMAND_ERROR_HANDLING_CANCEL_ALL_COMMANDS:
            case PVP_ENGINE_COMMAND_ERROR_HANDLING_GENERAL:
                retval = true;
                break;

            default:
                break;
        }
    }
    return retval;
}

PVMFStatus PVPlayerEngine::IssueNodeCancelCommand(PVPlayerEngineContext* aCurrentListContext, PVMFSessionId aSessionId, OsclAny* aNumberCancelCmdPending)
{
    PVMFStatus leavecode = 0;
    OSCL_TRY(leavecode, aCurrentListContext->iNode->CancelAllCommands(aSessionId, aNumberCancelCmdPending));
    OSCL_FIRST_CATCH_ANY(leavecode,;);
    return leavecode;
}

PVMFStatus PVPlayerEngine::IssueDatapathCancelCommand(PVPlayerEngineContext* aCurrentListContext, OsclAny* aNumberCancelCmdPending)
{
    PVMFStatus leavecode = 0;
    OSCL_TRY(leavecode, aCurrentListContext->iDatapath->CancelCommand(aNumberCancelCmdPending));
    OSCL_FIRST_CATCH_ANY(leavecode,;);
    return leavecode;
}

PVMFStatus PVPlayerEngine::IssueRecognizerRegistryCancel(OsclAny* aNumberCancelCmdPending)
{
    PVMFStatus leavecode = 0;
    OSCL_TRY(leavecode, iPlayerRecognizerRegistry.CancelQuery(aNumberCancelCmdPending));
    OSCL_FIRST_CATCH_ANY(leavecode,;);
    return leavecode;
}

PVMFStatus PVPlayerEngine::IssueSinkNodeInit(PVPlayerEngineDatapath* aDatapath, OsclAny* aCmdContext, PVMFCommandId &aCmdId)
{
    PVMFStatus leavecode = 0;
    OSCL_TRY(leavecode, aCmdId = aDatapath->iSinkNode->Init(aDatapath->iSinkNodeSessionId, aCmdContext));
    OSCL_FIRST_CATCH_ANY(leavecode,;);
    return leavecode;
}

PVMFStatus PVPlayerEngine::IssueSinkNodeReset(PVPlayerEngineDatapath* aDatapath, OsclAny* aCmdContext, PVMFCommandId &aCmdId)
{
    PVMFStatus leavecode = 0;
    OSCL_TRY(leavecode, aCmdId = aDatapath->iSinkNode->Reset(aDatapath->iSinkNodeSessionId, aCmdContext));
    OSCL_FIRST_CATCH_ANY(leavecode,;);
    return leavecode;
}

PVMFStatus PVPlayerEngine::IssueSinkSkipMediaData(PVPlayerEngineDatapath* aDatapath, bool aSFR, OsclAny* aCmdContext)
{
    PVMFStatus leavecode = 0;
    OSCL_TRY(leavecode, aDatapath->iSinkNodeSyncCtrlIF->SkipMediaData(aDatapath->iSinkNodeSessionId, iSkipMediaDataTS, iStreamID, aSFR, aCmdContext));
    OSCL_FIRST_CATCH_ANY(leavecode,;);
    return leavecode;
}

PVMFStatus PVPlayerEngine::IssueSourceSetDataSourcePosition(bool aIsPosUnitPlayList, OsclAny* aCmdContext)
{
    PVMFStatus leavecode = 0;
    if (aIsPosUnitPlayList)
    {
        OSCL_TRY(leavecode, iSourceNodePBCtrlIF->SetDataSourcePosition(iSourceNodeSessionId,
                 iDataSourcePosParams,
                 aCmdContext));

    }
    else
    {
        OSCL_TRY(leavecode, iSourceNodePBCtrlIF->SetDataSourcePosition(iSourceNodeSessionId,
                 iTargetNPT, iActualNPT, iActualMediaDataTS, iSeekToSyncPoint, iStreamID, aCmdContext));
    }
    OSCL_FIRST_CATCH_ANY(leavecode,;);
    return leavecode;
}

PVMFStatus PVPlayerEngine::IssueDecNodeInit(PVMFNodeInterface* aNode, PVMFSessionId aDecNodeSessionId, OsclAny* aCmdContext, PVMFCommandId &aCmdId)
{
    PVMFStatus leavecode = 0;
    OSCL_TRY(leavecode, aCmdId = aNode->Init(aDecNodeSessionId, aCmdContext));
    OSCL_FIRST_CATCH_ANY(leavecode,;);
    return leavecode;
}

PVMFStatus PVPlayerEngine::IssueDecNodeReset(PVMFNodeInterface* aNode, PVMFSessionId aDecNodeSessionId, OsclAny* aCmdContext, PVMFCommandId &aCmdId)
{
    PVMFStatus leavecode = 0;
    OSCL_TRY(leavecode, aCmdId = aNode->Reset(aDecNodeSessionId, aCmdContext));
    OSCL_FIRST_CATCH_ANY(leavecode,;);
    return leavecode;
}

PVMFStatus PVPlayerEngine::IssueQueryInterface(PVMFNodeInterface* aNode, PVMFSessionId aSessionId, const PVUuid aUuid, PVInterface*& aInterfacePtr, OsclAny* aCmdContext, PVMFCommandId& aCmdId)
{
    PVMFStatus leavecode = 0;
    OSCL_TRY(leavecode, aCmdId = aNode->QueryInterface(aSessionId, aUuid, aInterfacePtr, aCmdContext));
    OSCL_FIRST_CATCH_ANY(leavecode,;);
    return leavecode;
}

OSCL_EXPORT_REF void
PVPlayerInterface::GetSDKInfo
(
    PVSDKInfo& aSdkInfo
)
{
    aSdkInfo.iLabel = PVPLAYER_ENGINE_SDKINFO_LABEL;
    aSdkInfo.iDate  = PVPLAYER_ENGINE_SDKINFO_DATE;
}

// END FILE












