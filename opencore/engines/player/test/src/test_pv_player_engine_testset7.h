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
#ifndef TEST_PV_PLAYER_ENGINE_TESTSET7_H_INCLUDED
#define TEST_PV_PLAYER_ENGINE_TESTSET7_H_INCLUDED

/**
 *  @file test_pv_player_engine_testset7.h
 *  @brief This file contains the class definitions for the seventh set of
 *         test cases for PVPlayerEngine dealing with use of media IO node for data sink
 *
 */

#ifndef TEST_PV_PLAYER_ENGINE_H_INCLUDED
#include "test_pv_player_engine.h"
#endif

#ifndef PV_PLAYER_DATASOURCEURL_H_INCLUDED
#include "pv_player_datasourceurl.h"
#endif

#ifndef PVMF_NODE_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
#endif

#ifndef PV_ENGINE_TYPES_H_INCLUDED
#include "pv_engine_types.h"
#endif

#ifndef TEST_PV_PLAYER_ENGINE_CONFIG_H_INCLUDED
#include "test_pv_player_engine_config.h"
#endif

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif

#ifndef PVMI_CONFIG_AND_CAPABILITY_OBSERVER_H_INCLUDED
#include "pvmi_config_and_capability_observer.h"
#endif

#ifndef PVMF_STREAMING_DATA_SOURCE_H_INCLUDED
#include "pvmf_streaming_data_source.h"
#endif

#ifndef PVMF_SOURCE_CONTEXT_DATA_H_INCLUDED
#include "pvmf_source_context_data.h"
#endif

class PVPlayerDataSink;
class PVPlayerDataSinkFilename;
class PvmfFileOutputNodeConfigInterface;
class PvmiCapabilityAndConfig;

#define	FIRST_PAUSE_AFTER_START_LOCAL 10
#define PAUSE_RESUME_INTERVAL_LOCAL 1
#define	SEQUENTIAL_PAUSE_INTERVAL_LOCAL 5

/*!
 *  A test case to test playback of specified source with file output media IO node. Also tests out call to retrieve
 *  metadata keys and values from the source node
 *  - Data Source: Specified by user of test case
 *  - Data Sink(s): Video[File Output MediaIO Interface Node-test_player_mionode_openplaystop_%SOURCEFILENAME%_video.dat]\n
 *                  Audio[File Output MediaIO Interface Node-test_player_mionode_openplaystop_%SOURCEFILENAME%_audio.dat]\n
 *                  Text[File Output MediaIO Interface Node-test_player_mionode_openplaystop_%SOURCEFILENAME%_text.dat]
 *  - Sequence:
 *             -# CreatePlayer()
 *             -# AddDataSource()
 *             -# Init()
 *             -# GetMetadataKeys()
 *             -# GetMetadataValues()
 *             -# AddDataSink() (video)
 *             -# AddDataSink() (audio)
 *             -# AddDataSink() (text)
 *             -# Prepare()
 *             -# Start()
 *             -# WAIT 20 sec.
 *             -# Stop()
 *             -# RemoveDataSink() (video)
 *             -# RemoveDataSink() (audio)
 *             -# RemoveDataSink() (text)
 *             -# Reset()
 *             -# RemoveDataSource()
 *             -# DeletePlayer()
 *
 */
class pvplayer_async_test_mediaionode_openplaystop : public pvplayer_async_test_base
{
    public:
        pvplayer_async_test_mediaionode_openplaystop(PVPlayerAsyncTestParam aTestParam):
                pvplayer_async_test_base(aTestParam)
                , iPlayer(NULL)
                , iDataSource(NULL)
                , iDataSinkVideo(NULL)
                , iDataSinkAudio(NULL)
                , iDataSinkText(NULL)
                , iIONodeVideo(NULL)
                , iIONodeAudio(NULL)
                , iIONodeText(NULL)
                , iMIOFileOutVideo(NULL)
                , iMIOFileOutAudio(NULL)
                , iMIOFileOutText(NULL)
                , iCurrentCmdId(0)
        {
            iTestCaseName = _STRLIT_CHAR("MIONode Open-Play-Stop");
        }

        ~pvplayer_async_test_mediaionode_openplaystop() {}

        void StartTest();
        void Run();

        void CommandCompleted(const PVCmdResponse& aResponse);
        void HandleErrorEvent(const PVAsyncErrorEvent& aEvent);
        void HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent);

        enum PVTestState
        {
            STATE_CREATE,
            STATE_ADDDATASOURCE,
            STATE_INIT,
            STATE_GETMETADATAKEYLIST,
            STATE_GETMETADATAVALUELIST,
            STATE_ADDDATASINK_VIDEO,
            STATE_ADDDATASINK_AUDIO,
            STATE_ADDDATASINK_TEXT,
            STATE_PREPARE,
            STATE_START,
            STATE_STOP,
            STATE_REMOVEDATASINK_VIDEO,
            STATE_REMOVEDATASINK_AUDIO,
            STATE_REMOVEDATASINK_TEXT,
            STATE_RESET,
            STATE_REMOVEDATASOURCE,
            STATE_CLEANUPANDCOMPLETE
        };

        PVTestState iState;

        PVPlayerInterface* iPlayer;
        PVPlayerDataSourceURL* iDataSource;
        PVPlayerDataSink* iDataSinkVideo;
        PVPlayerDataSink* iDataSinkAudio;
        PVPlayerDataSink* iDataSinkText;
        PVMFNodeInterface* iIONodeVideo;
        PVMFNodeInterface* iIONodeAudio;
        PVMFNodeInterface* iIONodeText;
        PvmiMIOControl* iMIOFileOutVideo;
        PvmiMIOControl* iMIOFileOutAudio;
        PvmiMIOControl* iMIOFileOutText;
        PVCommandId iCurrentCmdId;

        OSCL_wHeapString<OsclMemAllocator> iFileNameWStr;
        oscl_wchar iTmpWCharBuffer[512];

        PVPMetadataList iMetadataKeyList;
        Oscl_Vector<PvmiKvp, OsclMemAllocator> iMetadataValueList;
        int32 iNumValues;
};


/*!
 *  A test case to test repeating playback(play-stop-play) of specified source with file output media IO node
 *  - Data Source: Specified by user of test case
 *  - Data Sink(s): Video[File Output MediaIO Interface Node-test_player_mionode_playstopplay_%SOURCEFILENAME%_video.dat]\n
 *                  Audio[File Output MediaIO Interface Node-test_player_mionode_playstopplay_%SOURCEFILENAME%_audio.dat]\n
 *                  Text[File Output MediaIO Interface Node-test_player_mionode_playstopplay_%SOURCEFILENAME%_text.dat]
 *  - Sequence:
 *             -# CreatePlayer()
 *             -# AddDataSource()
 *             -# Init()
 *             -# AddDataSink() (video)
 *             -# AddDataSink() (audio)
 *             -# AddDataSink() (text)
 *             -# Prepare()
 *             -# Start()
 *             -# WAIT 5 sec.
 *             -# Stop()
 *             -# WAIT 5 sec.
 *             -# Prepare()
 *             -# Start()
 *             -# WAIT 10 sec
 *             -# Stop()
 *             -# RemoveDataSink() (video)
 *             -# RemoveDataSink() (audio)
 *             -# RemoveDataSink() (text)
 *             -# Reset()
 *             -# RemoveDataSource()
 *             -# DeletePlayer()
 *
 */
class pvplayer_async_test_mediaionode_playstopplay : public pvplayer_async_test_base
{
    public:
        pvplayer_async_test_mediaionode_playstopplay(PVPlayerAsyncTestParam aTestParam):
                pvplayer_async_test_base(aTestParam)
                , iPlayer(NULL)
                , iDataSource(NULL)
                , iDataSinkVideo(NULL)
                , iDataSinkAudio(NULL)
                , iDataSinkText(NULL)
                , iIONodeVideo(NULL)
                , iIONodeAudio(NULL)
                , iIONodeText(NULL)
                , iMIOFileOutVideo(NULL)
                , iMIOFileOutAudio(NULL)
                , iMIOFileOutText(NULL)
                , iCurrentCmdId(0)
        {
            iTestCaseName = _STRLIT_CHAR("MIONode Play-Stop-Play-Stop");
        }

        ~pvplayer_async_test_mediaionode_playstopplay() {}

        void StartTest();
        void Run();

        void CommandCompleted(const PVCmdResponse& aResponse);
        void HandleErrorEvent(const PVAsyncErrorEvent& aEvent);
        void HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent);

        enum PVTestState
        {
            STATE_CREATE,
            STATE_ADDDATASOURCE,
            STATE_INIT,
            STATE_ADDDATASINK_VIDEO,
            STATE_ADDDATASINK_AUDIO,
            STATE_ADDDATASINK_TEXT,
            STATE_PREPARE1,
            STATE_START1,
            STATE_STOP1,
            STATE_PREPARE2,
            STATE_START2,
            STATE_STOP2,
            STATE_REMOVEDATASINK_VIDEO,
            STATE_REMOVEDATASINK_AUDIO,
            STATE_REMOVEDATASINK_TEXT,
            STATE_RESET,
            STATE_REMOVEDATASOURCE,
            STATE_CLEANUPANDCOMPLETE
        };

        PVTestState iState;

        PVPlayerInterface* iPlayer;
        PVPlayerDataSourceURL* iDataSource;
        PVPlayerDataSink* iDataSinkVideo;
        PVPlayerDataSink* iDataSinkAudio;
        PVPlayerDataSink* iDataSinkText;
        PVMFNodeInterface* iIONodeVideo;
        PVMFNodeInterface* iIONodeAudio;
        PVMFNodeInterface* iIONodeText;
        PvmiMIOControl* iMIOFileOutVideo;
        PvmiMIOControl* iMIOFileOutAudio;
        PvmiMIOControl* iMIOFileOutText;
        PVCommandId iCurrentCmdId;

        OSCL_wHeapString<OsclMemAllocator> iFileNameWStr;
        oscl_wchar iTmpWCharBuffer[512];
};


/*!
 *  A test case to test playback pause-resume of specified source with file output media IO node
 *  - Data Source: Specified by user of test case
 *  - Data Sink(s): Video[File Output MediaIO Interface Node-test_player_mionode_pauseresume_%SOURCEFILENAME%_video.dat]\n
 *                  Audio[File Output MediaIO Interface Node-test_player_mionode_pauseresume_%SOURCEFILENAME%_audio.dat]\n
 *                  Text[File Output MediaIO Interface Node-test_player_mionode_pauseresume_%SOURCEFILENAME%_text.dat]
 *  - Sequence:
 *             -# CreatePlayer()
 *             -# AddDataSource()
 *             -# Init()
 *             -# AddDataSink() (video)
 *             -# AddDataSink() (audio)
 *             -# AddDataSink() (text)
 *             -# Prepare()
 *             -# Start()
 *             -# WAIT 10 sec.
 *             -# Pause()
 *             -# WAIT 5 sec.
 *             -# Resume()
 *             -# WAIT 10 sec.
 *             -# Stop()
 *             -# RemoveDataSink() (video)
 *             -# RemoveDataSink() (audio)
 *             -# RemoveDataSink() (text)
 *             -# Reset()
 *             -# RemoveDataSource()
 *             -# DeletePlayer()
 *
 */
class pvplayer_async_test_mediaionode_pauseresume : public pvplayer_async_test_base
{
    public:
        pvplayer_async_test_mediaionode_pauseresume(PVPlayerAsyncTestParam aTestParam):
                pvplayer_async_test_base(aTestParam)
                , iPlayer(NULL)
                , iDataSource(NULL)
                , iDataSinkVideo(NULL)
                , iDataSinkAudio(NULL)
                , iDataSinkText(NULL)
                , iIONodeVideo(NULL)
                , iIONodeAudio(NULL)
                , iIONodeText(NULL)
                , iMIOFileOutVideo(NULL)
                , iMIOFileOutAudio(NULL)
                , iMIOFileOutText(NULL)
                , iCurrentCmdId(0)
        {
            iTestCaseName = _STRLIT_CHAR("MIONode Pause-Resume");
        }

        ~pvplayer_async_test_mediaionode_pauseresume() {}

        void StartTest();
        void Run();

        void CommandCompleted(const PVCmdResponse& aResponse);
        void HandleErrorEvent(const PVAsyncErrorEvent& aEvent);
        void HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent);

        enum PVTestState
        {
            STATE_CREATE,
            STATE_ADDDATASOURCE,
            STATE_INIT,
            STATE_ADDDATASINK_VIDEO,
            STATE_ADDDATASINK_AUDIO,
            STATE_ADDDATASINK_TEXT,
            STATE_PREPARE,
            STATE_START,
            STATE_PAUSE,
            STATE_RESUME,
            STATE_STOP,
            STATE_REMOVEDATASINK_VIDEO,
            STATE_REMOVEDATASINK_AUDIO,
            STATE_REMOVEDATASINK_TEXT,
            STATE_RESET,
            STATE_REMOVEDATASOURCE,
            STATE_CLEANUPANDCOMPLETE
        };

        PVTestState iState;

        PVPlayerInterface* iPlayer;
        PVPlayerDataSourceURL* iDataSource;
        PVPlayerDataSink* iDataSinkVideo;
        PVPlayerDataSink* iDataSinkAudio;
        PVPlayerDataSink* iDataSinkText;
        PVMFNodeInterface* iIONodeVideo;
        PVMFNodeInterface* iIONodeAudio;
        PVMFNodeInterface* iIONodeText;
        PvmiMIOControl* iMIOFileOutVideo;
        PvmiMIOControl* iMIOFileOutAudio;
        PvmiMIOControl* iMIOFileOutText;
        PVCommandId iCurrentCmdId;

        OSCL_wHeapString<OsclMemAllocator> iFileNameWStr;
        oscl_wchar iTmpWCharBuffer[512];
};


/*!
 *  A test case to test repositioning playback of specified source with file output media IO node
 *  - Data Source: Specified by user of test case
 *  - Data Sink(s): Video[File Output MediaIO Interface Node-test_player_mionode_playsetplaybackrange_%SOURCEFILENAME%_video.dat]\n
 *                  Audio[File Output MediaIO Interface Node-test_player_mionode_playsetplaybackrange_%SOURCEFILENAME%_audio.dat]\n
 *                  Text[File Output MediaIO Interface Node-test_player_mionode_playsetplaybackrange_%SOURCEFILENAME%_text.dat]
 *  - Sequence:
 *             -# CreatePlayer()
 *             -# AddDataSource()
 *             -# Init()
 *             -# AddDataSink() (video)
 *             -# AddDataSink() (audio)
 *             -# AddDataSink() (text)
 *             -# Prepare()
 *             -# Start()
 *             -# WAIT 10 sec.
 *             -# SetPlaybackRange(20 sec, indeterminate)
 *             -# WAIT 10 sec.
 *             -# Stop()
 *             -# RemoveDataSink() (video)
 *             -# RemoveDataSink() (audio)
 *             -# RemoveDataSink() (text)
 *             -# Reset()
 *             -# RemoveDataSource()
 *             -# DeletePlayer()
 *
 */
class pvplayer_async_test_mediaionode_playsetplaybackrange : public pvplayer_async_test_base
{
    public:
        pvplayer_async_test_mediaionode_playsetplaybackrange(PVPlayerAsyncTestParam aTestParam):
                pvplayer_async_test_base(aTestParam)
                , iPlayer(NULL)
                , iDataSource(NULL)
                , iDataSinkVideo(NULL)
                , iDataSinkAudio(NULL)
                , iDataSinkText(NULL)
                , iIONodeVideo(NULL)
                , iIONodeAudio(NULL)
                , iIONodeText(NULL)
                , iMIOFileOutVideo(NULL)
                , iMIOFileOutAudio(NULL)
                , iMIOFileOutText(NULL)
                , iCurrentCmdId(0)
        {
            iTestCaseName = _STRLIT_CHAR("MIONode Play-SetPlaybackRange");
        }

        ~pvplayer_async_test_mediaionode_playsetplaybackrange() {}

        void StartTest();
        void Run();

        void CommandCompleted(const PVCmdResponse& aResponse);
        void HandleErrorEvent(const PVAsyncErrorEvent& aEvent);
        void HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent);

        enum PVTestState
        {
            STATE_CREATE,
            STATE_ADDDATASOURCE,
            STATE_INIT,
            STATE_ADDDATASINK_VIDEO,
            STATE_ADDDATASINK_AUDIO,
            STATE_ADDDATASINK_TEXT,
            STATE_PREPARE,
            STATE_START,
            STATE_SETPLAYBACKRANGE,
            STATE_STOP,
            STATE_REMOVEDATASINK_VIDEO,
            STATE_REMOVEDATASINK_AUDIO,
            STATE_REMOVEDATASINK_TEXT,
            STATE_RESET,
            STATE_REMOVEDATASOURCE,
            STATE_CLEANUPANDCOMPLETE
        };

        PVTestState iState;

        PVPlayerInterface* iPlayer;
        PVPlayerDataSourceURL* iDataSource;
        PVPlayerDataSink* iDataSinkVideo;
        PVPlayerDataSink* iDataSinkAudio;
        PVPlayerDataSink* iDataSinkText;
        PVMFNodeInterface* iIONodeVideo;
        PVMFNodeInterface* iIONodeAudio;
        PVMFNodeInterface* iIONodeText;
        PvmiMIOControl* iMIOFileOutVideo;
        PvmiMIOControl* iMIOFileOutAudio;
        PvmiMIOControl* iMIOFileOutText;
        PVCommandId iCurrentCmdId;

        OSCL_wHeapString<OsclMemAllocator> iFileNameWStr;
        oscl_wchar iTmpWCharBuffer[512];
};


/*!
 *  A test case to test 3X playback of specified source with file output media IO node
 *  - Data Source: Specified by user of test case
 *  - Data Sink(s): Video[File Output MediaIO Interface Node-test_player_mionode_3Xplayrate_%SOURCEFILENAME%_video.dat]\n
 *                  Audio[File Output MediaIO Interface Node-test_player_mionode_3Xplayrate_%SOURCEFILENAME%_audio.dat]\n
 *                  Text[File Output MediaIO Interface Node-test_player_mionode_3Xplayrate_%SOURCEFILENAME%_text.dat]
 *  - Sequence:
 *             -# CreatePlayer()
 *             -# AddDataSource()
 *             -# Init()
 *             -# AddDataSink() (video)
 *             -# AddDataSink() (audio)
 *             -# AddDataSink() (text)
 *             -# Prepare()
 *             -# SetPlaybackRate(300000)
 *             -# Start()
 *             -# WAIT 20 sec.
 *             -# Stop()
 *             -# RemoveDataSink() (video)
 *             -# RemoveDataSink() (audio)
 *             -# RemoveDataSink() (text)
 *             -# Reset()
 *             -# RemoveDataSource()
 *             -# DeletePlayer()
 *
 */
class pvplayer_async_test_mediaionode_3Xplayrate : public pvplayer_async_test_base
{
    public:
        pvplayer_async_test_mediaionode_3Xplayrate(PVPlayerAsyncTestParam aTestParam):
                pvplayer_async_test_base(aTestParam)
                , iPlayer(NULL)
                , iDataSource(NULL)
                , iDataSinkVideo(NULL)
                , iDataSinkAudio(NULL)
                , iDataSinkText(NULL)
                , iIONodeVideo(NULL)
                , iIONodeAudio(NULL)
                , iIONodeText(NULL)
                , iMIOFileOutVideo(NULL)
                , iMIOFileOutAudio(NULL)
                , iMIOFileOutText(NULL)
                , iCurrentCmdId(0)
        {
            iTestCaseName = _STRLIT_CHAR("MIONode SetPlaybackRate 3X");
        }

        ~pvplayer_async_test_mediaionode_3Xplayrate() {}

        void StartTest();
        void Run();

        void CommandCompleted(const PVCmdResponse& aResponse);
        void HandleErrorEvent(const PVAsyncErrorEvent& aEvent);
        void HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent);

        enum PVTestState
        {
            STATE_CREATE,
            STATE_ADDDATASOURCE,
            STATE_INIT,
            STATE_ADDDATASINK_VIDEO,
            STATE_ADDDATASINK_AUDIO,
            STATE_ADDDATASINK_TEXT,
            STATE_PREPARE,
            STATE_SETPLAYBACKRATE,
            STATE_START,
            STATE_STOP,
            STATE_REMOVEDATASINK_VIDEO,
            STATE_REMOVEDATASINK_AUDIO,
            STATE_REMOVEDATASINK_TEXT,
            STATE_RESET,
            STATE_REMOVEDATASOURCE,
            STATE_CLEANUPANDCOMPLETE
        };

        PVTestState iState;

        PVPlayerInterface* iPlayer;
        PVPlayerDataSourceURL* iDataSource;
        PVPlayerDataSink* iDataSinkVideo;
        PVPlayerDataSink* iDataSinkAudio;
        PVPlayerDataSink* iDataSinkText;
        PVMFNodeInterface* iIONodeVideo;
        PVMFNodeInterface* iIONodeAudio;
        PVMFNodeInterface* iIONodeText;
        PvmiMIOControl* iMIOFileOutVideo;
        PvmiMIOControl* iMIOFileOutAudio;
        PvmiMIOControl* iMIOFileOutText;
        PVCommandId iCurrentCmdId;

        OSCL_wHeapString<OsclMemAllocator> iFileNameWStr;
        oscl_wchar iTmpWCharBuffer[512];
};


/*!
 *  A test case to test 0.5X playback of specified source with file output media IO node
 *  - Data Source: Specified by user of test case
 *  - Data Sink(s): Video[File Output MediaIO Interface Node-test_player_mionode_halfplayrate_%SOURCEFILENAME%_video.dat]\n
 *                  Audio[File Output MediaIO Interface Node-test_player_mionode_halfplayrate_%SOURCEFILENAME%_audio.dat]\n
 *                  Text[File Output MediaIO Interface Node-test_player_mionode_halfplayrate_%SOURCEFILENAME%_text.dat]
 *  - Sequence:
 *             -# CreatePlayer()
 *             -# AddDataSource()
 *             -# Init()
 *             -# AddDataSink() (video)
 *             -# AddDataSink() (audio)
 *             -# AddDataSink() (text)
 *             -# Prepare()
 *             -# SetPlaybackRate(50000)
 *             -# Start()
 *             -# WAIT 20 sec.
 *             -# Stop()
 *             -# RemoveDataSink() (video)
 *             -# RemoveDataSink() (audio)
 *             -# RemoveDataSink() (text)
 *             -# Reset()
 *             -# RemoveDataSource()
 *             -# DeletePlayer()
 *
 */
class pvplayer_async_test_mediaionode_halfplayrate : public pvplayer_async_test_base
{
    public:
        pvplayer_async_test_mediaionode_halfplayrate(PVPlayerAsyncTestParam aTestParam):
                pvplayer_async_test_base(aTestParam)
                , iPlayer(NULL)
                , iDataSource(NULL)
                , iDataSinkVideo(NULL)
                , iDataSinkAudio(NULL)
                , iDataSinkText(NULL)
                , iIONodeVideo(NULL)
                , iIONodeAudio(NULL)
                , iIONodeText(NULL)
                , iMIOFileOutVideo(NULL)
                , iMIOFileOutAudio(NULL)
                , iMIOFileOutText(NULL)
                , iCurrentCmdId(0)
        {
            iTestCaseName = _STRLIT_CHAR("MIONode SetPlaybackRate 1/2X");
        }

        ~pvplayer_async_test_mediaionode_halfplayrate() {}

        void StartTest();
        void Run();

        void CommandCompleted(const PVCmdResponse& aResponse);
        void HandleErrorEvent(const PVAsyncErrorEvent& aEvent);
        void HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent);

        enum PVTestState
        {
            STATE_CREATE,
            STATE_ADDDATASOURCE,
            STATE_INIT,
            STATE_ADDDATASINK_VIDEO,
            STATE_ADDDATASINK_AUDIO,
            STATE_ADDDATASINK_TEXT,
            STATE_PREPARE,
            STATE_SETPLAYBACKRATE,
            STATE_START,
            STATE_STOP,
            STATE_REMOVEDATASINK_VIDEO,
            STATE_REMOVEDATASINK_AUDIO,
            STATE_REMOVEDATASINK_TEXT,
            STATE_RESET,
            STATE_REMOVEDATASOURCE,
            STATE_CLEANUPANDCOMPLETE
        };

        PVTestState iState;

        PVPlayerInterface* iPlayer;
        PVPlayerDataSourceURL* iDataSource;
        PVPlayerDataSink* iDataSinkVideo;
        PVPlayerDataSink* iDataSinkAudio;
        PVPlayerDataSink* iDataSinkText;
        PVMFNodeInterface* iIONodeVideo;
        PVMFNodeInterface* iIONodeAudio;
        PVMFNodeInterface* iIONodeText;
        PvmiMIOControl* iMIOFileOutVideo;
        PvmiMIOControl* iMIOFileOutAudio;
        PvmiMIOControl* iMIOFileOutText;
        PVCommandId iCurrentCmdId;

        OSCL_wHeapString<OsclMemAllocator> iFileNameWStr;
        oscl_wchar iTmpWCharBuffer[512];
};


/*!
 *  A test case to test looping playback of specified source with file output media IO node
 *  - Data Source: Specified by user of test case
 *  - Data Sink(s): Video[File Output MediaIO Interface Node-test_player_mionode_looping_%SOURCEFILENAME%_video.dat]\n
 *                  Audio[File Output MediaIO Interface Node-test_player_mionode_looping_%SOURCEFILENAME%_audio.dat]\n
 *                  Text[File Output MediaIO Interface Node-test_player_mionode_looping_%SOURCEFILENAME%_text.dat]
 *  - Sequence:
 *             -# CreatePlayer()
 *             -# AddDataSource()
 *             -# Init()
 *             -# AddDataSink() (video)
 *             -# AddDataSink() (audio)
 *             -# AddDataSink() (text)
 *             -# Prepare()
 *             -# Start()
 *             -# SetPlaybackRange(indeterminate, end at 10 sec)
 *             -# WAIT 15 sec. for end time reached event
 *             -# SetPlaybackRange(0, end at 10 sec)
 *             -# Resume()
 *             -# WAIT 15 sec. for end time reached event
 *             -# Stop()
 *             -# RemoveDataSink() (video)
 *             -# RemoveDataSink() (audio)
 *             -# RemoveDataSink() (text)
 *             -# Reset()
 *             -# RemoveDataSource()
 *             -# DeletePlayer()
 *
 */
class pvplayer_async_test_mediaionode_looping : public pvplayer_async_test_base
{
    public:
        pvplayer_async_test_mediaionode_looping(PVPlayerAsyncTestParam aTestParam):
                pvplayer_async_test_base(aTestParam)
                , iPlayer(NULL)
                , iDataSource(NULL)
                , iDataSinkVideo(NULL)
                , iDataSinkAudio(NULL)
                , iDataSinkText(NULL)
                , iIONodeVideo(NULL)
                , iIONodeAudio(NULL)
                , iIONodeText(NULL)
                , iMIOFileOutVideo(NULL)
                , iMIOFileOutAudio(NULL)
                , iMIOFileOutText(NULL)
                , iCurrentCmdId(0)
        {
            iTestCaseName = _STRLIT_CHAR("MIONode Looping");
        }

        ~pvplayer_async_test_mediaionode_looping() {}

        void StartTest();
        void Run();

        void CommandCompleted(const PVCmdResponse& aResponse);
        void HandleErrorEvent(const PVAsyncErrorEvent& aEvent);
        void HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent);

        enum PVTestState
        {
            STATE_CREATE,
            STATE_ADDDATASOURCE,
            STATE_INIT,
            STATE_ADDDATASINK_VIDEO,
            STATE_ADDDATASINK_AUDIO,
            STATE_ADDDATASINK_TEXT,
            STATE_PREPARE,
            STATE_START,
            STATE_SETPLAYBACKRANGE1,
            STATE_ENDTIMENOTREACHED1,
            STATE_SETPLAYBACKRANGE2,
            STATE_RESUME,
            STATE_ENDTIMENOTREACHED2,
            STATE_STOP,
            STATE_REMOVEDATASINK_VIDEO,
            STATE_REMOVEDATASINK_AUDIO,
            STATE_REMOVEDATASINK_TEXT,
            STATE_RESET,
            STATE_REMOVEDATASOURCE,
            STATE_CLEANUPANDCOMPLETE
        };

        PVTestState iState;

        PVPlayerInterface* iPlayer;
        PVPlayerDataSourceURL* iDataSource;
        PVPlayerDataSink* iDataSinkVideo;
        PVPlayerDataSink* iDataSinkAudio;
        PVPlayerDataSink* iDataSinkText;
        PVMFNodeInterface* iIONodeVideo;
        PVMFNodeInterface* iIONodeAudio;
        PVMFNodeInterface* iIONodeText;
        PvmiMIOControl* iMIOFileOutVideo;
        PvmiMIOControl* iMIOFileOutAudio;
        PvmiMIOControl* iMIOFileOutText;
        PVCommandId iCurrentCmdId;

        OSCL_wHeapString<OsclMemAllocator> iFileNameWStr;
        oscl_wchar iTmpWCharBuffer[512];
};


/*!
 *  A test case to test playback of specified source with file output media IO node till EOS
 *  - Data Source: Specified by user of test case
 *  - Data Sink(s): Video[File Output MediaIO Interface Node-test_player_mionode_waitforeos_%SOURCEFILENAME%_video.dat]\n
 *                  Audio[File Output MediaIO Interface Node-test_player_mionode_waitforeos_%SOURCEFILENAME%_audio.dat]\n
 *                  Text[File Output MediaIO Interface Node-test_player_mionode_waitforeos_%SOURCEFILENAME%_text.dat]
 *  - Sequence:
 *             -# CreatePlayer()
 *             -# AddDataSource()
 *             -# Init()
 *             -# AddDataSink() (video)
 *             -# AddDataSink() (audio)
 *             -# AddDataSink() (text)
 *             -# Prepare()
 *             -# Start()
 *             -# WAIT FOR EOS EVENT OR TIMEOUT AFTER 180 sec.
 *             -# IF PAUSED DUE TO EOS Resume() (should fail)
 *             -# Stop()
 *             -# RemoveDataSink() (video)
 *             -# RemoveDataSink() (audio)
 *             -# RemoveDataSink() (text)
 *             -# Reset()
 *             -# RemoveDataSource()
 *             -# DeletePlayer()
 *
 */
class pvplayer_async_test_mediaionode_waitforeos : public pvplayer_async_test_base
{
    public:
        pvplayer_async_test_mediaionode_waitforeos(PVPlayerAsyncTestParam aTestParam):
                pvplayer_async_test_base(aTestParam)
                , iPlayer(NULL)
                , iDataSource(NULL)
                , iDataSinkVideo(NULL)
                , iDataSinkAudio(NULL)
                , iDataSinkText(NULL)
                , iIONodeVideo(NULL)
                , iIONodeAudio(NULL)
                , iIONodeText(NULL)
                , iMIOFileOutVideo(NULL)
                , iMIOFileOutAudio(NULL)
                , iMIOFileOutText(NULL)
                , iCurrentCmdId(0)
        {
            iTestCaseName = _STRLIT_CHAR("MIONode Wait For EOS");
        }

        ~pvplayer_async_test_mediaionode_waitforeos() {}

        void StartTest();
        void Run();

        void CommandCompleted(const PVCmdResponse& aResponse);
        void HandleErrorEvent(const PVAsyncErrorEvent& aEvent);
        void HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent);

        enum PVTestState
        {
            STATE_CREATE,
            STATE_ADDDATASOURCE,
            STATE_INIT,
            STATE_ADDDATASINK_VIDEO,
            STATE_ADDDATASINK_AUDIO,
            STATE_ADDDATASINK_TEXT,
            STATE_PREPARE,
            STATE_START,
            STATE_EOSNOTREACHED,
            STATE_RESUMEAFTEREOS,
            STATE_STOP,
            STATE_REMOVEDATASINK_VIDEO,
            STATE_REMOVEDATASINK_AUDIO,
            STATE_REMOVEDATASINK_TEXT,
            STATE_RESET,
            STATE_REMOVEDATASOURCE,
            STATE_CLEANUPANDCOMPLETE
        };

        PVTestState iState;

        PVPlayerInterface* iPlayer;
        PVPlayerDataSourceURL* iDataSource;
        PVPlayerDataSink* iDataSinkVideo;
        PVPlayerDataSink* iDataSinkAudio;
        PVPlayerDataSink* iDataSinkText;
        PVMFNodeInterface* iIONodeVideo;
        PVMFNodeInterface* iIONodeAudio;
        PVMFNodeInterface* iIONodeText;
        PvmiMIOControl* iMIOFileOutVideo;
        PvmiMIOControl* iMIOFileOutAudio;
        PvmiMIOControl* iMIOFileOutText;
        PVCommandId iCurrentCmdId;

        OSCL_wHeapString<OsclMemAllocator> iFileNameWStr;
        oscl_wchar iTmpWCharBuffer[512];
};


/*!
 *  A test case to test if the player engine can handle multiple pause-resume requests with media IO for sinks
 *  - Data Source: Specified source
 *  - Data Sink(s): Video[FileOutputNode-test_player_mionode_multipauseresume_[SRCFILENAME]_video.dat]\n
 *                  Audio[FileOutputNode-test_player_mionode_multipauseresume_[SRCFILENAME]_audio.dat]\n
 *                  Text[FileOutputNode-test_player_mionode_multipauseresume_[SRCFILENAME]_text.dat]
 *  - Sequence:
 *             -# CreatePlayer()
 *             -# AddDataSource()/Init()/AddDataSink() (video)/AddDataSink() (audio)/AddDataSink() (text)/Prepare()/Start()
 *             -# WAIT 5 sec.
 *             -# Pause()/Resume() X 5
 *             -# WAIT 2 sec.
 *             -# Pause()
 *             -# WAIT 5 sec.
 *             -# Resume()
 *             -# WAIT 3 sec.
 *             -# Pause()/Resume() X 3
 *             -# Wait 5 sec.
 *             -# Stop()/RemoveDataSink() (video)/RemoveDataSink() (audio)/RemoveDataSink() (text)/Reset()/RemoveDataSource()
 *             -# DeletePlayer()
 *
 */
class pvplayer_async_test_mediaionode_multipauseresume : public pvplayer_async_test_base
{
    public:
        pvplayer_async_test_mediaionode_multipauseresume(PVPlayerAsyncTestParam aTestParam):
                pvplayer_async_test_base(aTestParam)
                , iPlayer(NULL)
                , iDataSource(NULL)
                , iDataSinkVideo(NULL)
                , iDataSinkAudio(NULL)
                , iDataSinkText(NULL)
                , iIONodeVideo(NULL)
                , iIONodeAudio(NULL)
                , iIONodeText(NULL)
                , iMIOFileOutVideo(NULL)
                , iMIOFileOutAudio(NULL)
                , iMIOFileOutText(NULL)
                , iCurrentCmdId(0)
                , iSourceContextData(NULL)
        {
            iTestCaseName = _STRLIT_CHAR("MIONode Multiple Pause-Resume");
        }

        ~pvplayer_async_test_mediaionode_multipauseresume() {}

        void StartTest();
        void Run();

        void CommandCompleted(const PVCmdResponse& aResponse);
        void HandleErrorEvent(const PVAsyncErrorEvent& aEvent);
        void HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent);

        enum PVTestState
        {
            STATE_CREATE,
            STATE_STARTPLAYBACK,
            STATE_PAUSERESUME1,
            STATE_PAUSE,
            STATE_RESUME,
            STATE_PAUSERESUME2,
            STATE_SHUTDOWN,
            STATE_CLEANUPANDCOMPLETE
        };

        PVTestState iState;

        PVPlayerInterface* iPlayer;
        PVPlayerDataSourceURL* iDataSource;
        PVPlayerDataSink* iDataSinkVideo;
        PVPlayerDataSink* iDataSinkAudio;
        PVPlayerDataSink* iDataSinkText;
        PVMFNodeInterface* iIONodeVideo;
        PVMFNodeInterface* iIONodeAudio;
        PVMFNodeInterface* iIONodeText;
        PvmiMIOControl* iMIOFileOutVideo;
        PvmiMIOControl* iMIOFileOutAudio;
        PvmiMIOControl* iMIOFileOutText;
        PVCommandId iCurrentCmdId;

        Oscl_Vector<PVCommandId, OsclMemAllocator> iCmdIds;

    private:
        OSCL_wHeapString<OsclMemAllocator> wFileName;
        oscl_wchar output[512];
        PVMFSourceContextData* iSourceContextData;
};


/*!
 *  A test case to test if the player engine can handle multiple repositioning during playback with media IO for sinks
 *  - Data Source: Specified source
 *  - Data Sink(s): Video[FileOutputNode-test_player_mionode_multireposition_[SRCFILENAME]_video.dat]\n
 *                  Audio[FileOutputNode-test_player_mionode_multireposition_[SRCFILENAME]_audio.dat]\n
 *                  Text[FileOutputNode-test_player_mionode_multireposition_[SRCFILENAME]_text.dat]
 *  - Sequence:
 *             -# CreatePlayer()
 *             -# AddDataSource()/Init()/AddDataSink() (video)/AddDataSink() (audio)/AddDataSink() (text)/Prepare()/Start()
 *             -# WAIT 5 sec.
 *             -# SetPlaybackRange(20 sec, indeterminate)
 *             -# WAIT 5 sec.
 *             -# SetPlaybackRange(5 sec, indeterminate)
 *             -# WAIT 5 sec.
 *             -# SetPlaybackRange(25 sec, indeterminate)
 *             -# WAIT 5 sec.
 *             -# SetPlaybackRange(10 sec, indeterminate)
 *             -# WAIT 5 sec.
 *             -# SetPlaybackRange(20 sec, indeterminate)
 *             -# SetPlaybackRange(0 sec, indeterminate)
 *             -# WAIT 5 sec.
 *             -# Stop()/RemoveDataSink() (video)/RemoveDataSink() (audio)/RemoveDataSink() (text)/Reset()/RemoveDataSource()
 *             -# DeletePlayer()
 *
 */
class pvplayer_async_test_mediaionode_multireposition : public pvplayer_async_test_base
{
    public:
        pvplayer_async_test_mediaionode_multireposition(PVPlayerAsyncTestParam aTestParam):
                pvplayer_async_test_base(aTestParam)
                , iPlayer(NULL)
                , iDataSource(NULL)
                , iDataSinkVideo(NULL)
                , iDataSinkAudio(NULL)
                , iDataSinkText(NULL)
                , iIONodeVideo(NULL)
                , iIONodeAudio(NULL)
                , iIONodeText(NULL)
                , iMIOFileOutVideo(NULL)
                , iMIOFileOutAudio(NULL)
                , iMIOFileOutText(NULL)
                , iCurrentCmdId(0)
        {
            iTestCaseName = _STRLIT_CHAR("MIONode Multiple SetPlaybackRange");
        }

        ~pvplayer_async_test_mediaionode_multireposition() {}

        void StartTest();
        void Run();

        void CommandCompleted(const PVCmdResponse& aResponse);
        void HandleErrorEvent(const PVAsyncErrorEvent& aEvent);
        void HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent);

        enum PVTestState
        {
            STATE_CREATE,
            STATE_STARTPLAYBACK,
            STATE_SETPLAYBACKRANGE1,
            STATE_SETPLAYBACKRANGE2,
            STATE_SETPLAYBACKRANGE3,
            STATE_SETPLAYBACKRANGE4,
            STATE_SETPLAYBACKRANGE5,
            STATE_SETPLAYBACKRANGE6,
            STATE_SHUTDOWN,
            STATE_CLEANUPANDCOMPLETE
        };

        PVTestState iState;

        PVPlayerInterface* iPlayer;
        PVPlayerDataSourceURL* iDataSource;
        PVPlayerDataSink* iDataSinkVideo;
        PVPlayerDataSink* iDataSinkAudio;
        PVPlayerDataSink* iDataSinkText;
        PVMFNodeInterface* iIONodeVideo;
        PVMFNodeInterface* iIONodeAudio;
        PVMFNodeInterface* iIONodeText;
        PvmiMIOControl* iMIOFileOutVideo;
        PvmiMIOControl* iMIOFileOutAudio;
        PvmiMIOControl* iMIOFileOutText;
        PVCommandId iCurrentCmdId;

        Oscl_Vector<PVCommandId, OsclMemAllocator> iCmdIds;

        Oscl_FileServer iFS;
        Oscl_File iTimeLogFile;

    private:
        OSCL_wHeapString<OsclMemAllocator> wFileName;
        oscl_wchar output[512];
};


/*!
 *  A test case to test various configurations of repositioning playback of specified source with file output media IO node
 *  - Data Source: Specified by user of test case
 *  - Data Sink(s): Video[File Output MediaIO Interface Node-test_player_mionode_repositionconfig_%SOURCEFILENAME%_video.dat]\n
 *                  Audio[File Output MediaIO Interface Node-test_player_mionode_repositionconfig_%SOURCEFILENAME%_audio.dat]\n
 *                  Text[File Output MediaIO Interface Node-test_player_mionode_repositionconfig_%SOURCEFILENAME%_text.dat]
 *  - Sequence:
 *             -# CreatePlayer()/QueryInterface() (capability-and-config interface)
 *             -# AddDataSource()
 *             -# Init()
 *             -# AddDataSink() (video)
 *             -# AddDataSink() (audio)
 *             -# AddDataSink() (text)
 *             -# setParametersSync() seektosyncpoint=true, seekwindow=0, skiptorequestedpos=false
 *             -# SetPlaybackRange(10sec, indeterminate)
 *             -# Prepare()
 *             -# Start()
 *             -# WAIT 5 sec.
 *             -# SetPlaybackRange(20 sec, indeterminate)
 *             -# WAIT 10 sec.
 *             -# setParametersSync() seektosyncpoint=true, seekwindow=1000, skiptorequestedpos=false
 *             -# SetPlaybackRange(20 sec, indeterminate)
 *             -# WAIT 10 sec.
 *             -# setParametersSync() seektosyncpoint=true, seekwindow=1000, skiptorequestedpos=true
 *             -# SetPlaybackRange(20 sec, indeterminate)
 *             -# WAIT 10 sec.
 *             -# setParametersSync() seektosyncpoint=false, skiptorequestedpos=false
 *             -# SetPlaybackRange(20 sec, indeterminate)
 *             -# WAIT 10 sec.
 *             -# setParametersSync() seektosyncpoint=false, skiptorequestedpos=true
 *             -# SetPlaybackRange(20 sec, indeterminate)
 *             -# WAIT 5 sec.
 *             -# setParametersSync() seektosyncpoint=true, seekwindow=100, skiptorequestedpos=false
 *             -# SetPlaybackRange(30 sec, indeterminate)
 *             -# WAIT 5 sec.
 *             -# Stop()
 *             -# RemoveDataSink() (video)
 *             -# RemoveDataSink() (audio)
 *             -# RemoveDataSink() (text)
 *             -# Reset()
 *             -# RemoveDataSource()
 *             -# DeletePlayer()
 *
 */
class pvplayer_async_test_mediaionode_repositionconfig : public pvplayer_async_test_base
{
    public:
        pvplayer_async_test_mediaionode_repositionconfig(PVPlayerAsyncTestParam aTestParam):
                pvplayer_async_test_base(aTestParam)
                , iPlayer(NULL)
                , iPlayerCapConfigIF(NULL)
                , iDataSource(NULL)
                , iDataSinkVideo(NULL)
                , iDataSinkAudio(NULL)
                , iDataSinkText(NULL)
                , iIONodeVideo(NULL)
                , iIONodeAudio(NULL)
                , iIONodeText(NULL)
                , iMIOFileOutVideo(NULL)
                , iMIOFileOutAudio(NULL)
                , iMIOFileOutText(NULL)
                , iCurrentCmdId(0)
        {
            iTestCaseName = _STRLIT_CHAR("MIONode Different Reposition Config");
        }

        ~pvplayer_async_test_mediaionode_repositionconfig() {}

        void StartTest();
        void Run();

        void CommandCompleted(const PVCmdResponse& aResponse);
        void HandleErrorEvent(const PVAsyncErrorEvent& aEvent);
        void HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent);

        enum PVTestState
        {
            STATE_CREATE,
            STATE_ADDDATASOURCE,
            STATE_INIT,
            STATE_ADDDATASINK_VIDEO,
            STATE_ADDDATASINK_AUDIO,
            STATE_ADDDATASINK_TEXT,
            STATE_SETPLAYBACKRANGE1,
            STATE_PREPARE,
            STATE_START,
            STATE_SETPLAYBACKRANGE2,
            STATE_SETPLAYBACKRANGE3,
            STATE_SETPLAYBACKRANGE4,
            STATE_SETPLAYBACKRANGE5,
            STATE_SETPLAYBACKRANGE6,
            STATE_SETPLAYBACKRANGE7,
            STATE_STOP,
            STATE_REMOVEDATASINK_VIDEO,
            STATE_REMOVEDATASINK_AUDIO,
            STATE_REMOVEDATASINK_TEXT,
            STATE_RESET,
            STATE_REMOVEDATASOURCE,
            STATE_CLEANUPANDCOMPLETE
        };

        PVTestState iState;

        PVPlayerInterface* iPlayer;
        PvmiCapabilityAndConfig* iPlayerCapConfigIF;
        PVPlayerDataSourceURL* iDataSource;
        PVPlayerDataSink* iDataSinkVideo;
        PVPlayerDataSink* iDataSinkAudio;
        PVPlayerDataSink* iDataSinkText;
        PVMFNodeInterface* iIONodeVideo;
        PVMFNodeInterface* iIONodeAudio;
        PVMFNodeInterface* iIONodeText;
        PvmiMIOControl* iMIOFileOutVideo;
        PvmiMIOControl* iMIOFileOutAudio;
        PvmiMIOControl* iMIOFileOutText;
        PVCommandId iCurrentCmdId;

        OSCL_wHeapString<OsclMemAllocator> iFileNameWStr;
        oscl_wchar iTmpWCharBuffer[512];
};


/*!
 *  A test case to test multiple looping playback of the whole specified source with file output media IO node
 *  - Data Source: Specified by user of test case
 *  - Data Sink(s): Video[File Output MediaIO Interface Node-test_player_mionode_eoslooping_%SOURCEFILENAME%_video.dat]\n
 *                  Audio[File Output MediaIO Interface Node-test_player_mionode_eoslooping_%SOURCEFILENAME%_audio.dat]\n
 *                  Text[File Output MediaIO Interface Node-test_player_mionode_eoslooping_%SOURCEFILENAME%_text.dat]
 *  - Sequence:
 *             -# CreatePlayer()
 *             -# AddDataSource()
 *             -# Init()
 *             -# AddDataSink() (video)
 *             -# AddDataSink() (audio)
 *             -# AddDataSink() (text)
 *             -# Prepare()
 *             -# Start()
 *             -# WAIT 180 sec. for EOS event
 *             -# SetPlaybackRange(0, indeterminate)
 *             -# Resume()
 *             -# WAIT 180 sec. for EOS event
 *             -# SetPlaybackRange(0, indeterminate)
 *             -# Resume()
 *             -# WAIT 180 sec. for EOS event
 *             -# Stop()
 *             -# RemoveDataSink() (video)
 *             -# RemoveDataSink() (audio)
 *             -# RemoveDataSink() (text)
 *             -# Reset()
 *             -# RemoveDataSource()
 *             -# DeletePlayer()
 *
 */
class pvplayer_async_test_mediaionode_eoslooping : public pvplayer_async_test_base
{
    public:
        pvplayer_async_test_mediaionode_eoslooping(PVPlayerAsyncTestParam aTestParam):
                pvplayer_async_test_base(aTestParam)
                , iPlayer(NULL)
                , iDataSource(NULL)
                , iDataSinkVideo(NULL)
                , iDataSinkAudio(NULL)
                , iDataSinkText(NULL)
                , iIONodeVideo(NULL)
                , iIONodeAudio(NULL)
                , iIONodeText(NULL)
                , iMIOFileOutVideo(NULL)
                , iMIOFileOutAudio(NULL)
                , iMIOFileOutText(NULL)
                , iCurrentCmdId(0)
        {
            iTestCaseName = _STRLIT_CHAR("MIONode Looping After EOS");
        }

        ~pvplayer_async_test_mediaionode_eoslooping() {}

        void StartTest();
        void Run();

        void CommandCompleted(const PVCmdResponse& aResponse);
        void HandleErrorEvent(const PVAsyncErrorEvent& aEvent);
        void HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent);

        enum PVTestState
        {
            STATE_CREATE,
            STATE_ADDDATASOURCE,
            STATE_INIT,
            STATE_ADDDATASINK_VIDEO,
            STATE_ADDDATASINK_AUDIO,
            STATE_ADDDATASINK_TEXT,
            STATE_PREPARE,
            STATE_START,
            STATE_ENDTIMENOTREACHED1,
            STATE_SETPLAYBACKRANGE1,
            STATE_RESUME1,
            STATE_ENDTIMENOTREACHED2,
            STATE_SETPLAYBACKRANGE2,
            STATE_RESUME2,
            STATE_ENDTIMENOTREACHED3,
            STATE_STOP,
            STATE_REMOVEDATASINK_VIDEO,
            STATE_REMOVEDATASINK_AUDIO,
            STATE_REMOVEDATASINK_TEXT,
            STATE_RESET,
            STATE_REMOVEDATASOURCE,
            STATE_CLEANUPANDCOMPLETE
        };

        PVTestState iState;

        PVPlayerInterface* iPlayer;
        PVPlayerDataSourceURL* iDataSource;
        PVPlayerDataSink* iDataSinkVideo;
        PVPlayerDataSink* iDataSinkAudio;
        PVPlayerDataSink* iDataSinkText;
        PVMFNodeInterface* iIONodeVideo;
        PVMFNodeInterface* iIONodeAudio;
        PVMFNodeInterface* iIONodeText;
        PvmiMIOControl* iMIOFileOutVideo;
        PvmiMIOControl* iMIOFileOutAudio;
        PvmiMIOControl* iMIOFileOutText;
        PVCommandId iCurrentCmdId;

        OSCL_wHeapString<OsclMemAllocator> iFileNameWStr;
        oscl_wchar iTmpWCharBuffer[512];
};


/*!
 *  A test case to test repositioning during Prepared state
 *  - Data Source: Specified by user of test case
 *  - Data Sink(s): Video[File Output MediaIO Interface Node-test_player_mionode_preparedrepo_%SOURCEFILENAME%_video.dat]\n
 *                  Audio[File Output MediaIO Interface Node-test_player_mionode_preparedrepo_%SOURCEFILENAME%_audio.dat]\n
 *                  Text[File Output MediaIO Interface Node-test_player_mionode_preparedrepo_%SOURCEFILENAME%_text.dat]
 *  - Sequence:
 *             -# CreatePlayer()
 *             -# AddDataSource()
 *             -# Init()
 *             -# AddDataSink() (video)
 *             -# AddDataSink() (audio)
 *             -# AddDataSink() (text)
 *             -# Prepare()
 *             -# SetPlaybackRange(20 sec, indeterminate)
 *             -# Start()
 *             -# WAIT 10 sec.
 *             -# Stop()
 *             -# RemoveDataSink() (video)
 *             -# RemoveDataSink() (audio)
 *             -# RemoveDataSink() (text)
 *             -# Reset()
 *             -# RemoveDataSource()
 *             -# DeletePlayer()
 *
 */
class pvplayer_async_test_mediaionode_repositionduringprepared : public pvplayer_async_test_base
{
    public:
        pvplayer_async_test_mediaionode_repositionduringprepared(PVPlayerAsyncTestParam aTestParam):
                pvplayer_async_test_base(aTestParam)
                , iPlayer(NULL)
                , iDataSource(NULL)
                , iDataSinkVideo(NULL)
                , iDataSinkAudio(NULL)
                , iDataSinkText(NULL)
                , iIONodeVideo(NULL)
                , iIONodeAudio(NULL)
                , iIONodeText(NULL)
                , iMIOFileOutVideo(NULL)
                , iMIOFileOutAudio(NULL)
                , iMIOFileOutText(NULL)
                , iCurrentCmdId(0)
        {
            iTestCaseName = _STRLIT_CHAR("MIONode Reposition During Prepared");
        }

        ~pvplayer_async_test_mediaionode_repositionduringprepared() {}

        void StartTest();
        void Run();

        void CommandCompleted(const PVCmdResponse& aResponse);
        void HandleErrorEvent(const PVAsyncErrorEvent& aEvent);
        void HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent);

        enum PVTestState
        {
            STATE_CREATE,
            STATE_ADDDATASOURCE,
            STATE_INIT,
            STATE_ADDDATASINK_VIDEO,
            STATE_ADDDATASINK_AUDIO,
            STATE_ADDDATASINK_TEXT,
            STATE_PREPARE,
            STATE_SETPLAYBACKRANGE,
            STATE_START,
            STATE_STOP,
            STATE_REMOVEDATASINK_VIDEO,
            STATE_REMOVEDATASINK_AUDIO,
            STATE_REMOVEDATASINK_TEXT,
            STATE_RESET,
            STATE_REMOVEDATASOURCE,
            STATE_CLEANUPANDCOMPLETE
        };

        PVTestState iState;

        PVPlayerInterface* iPlayer;
        PVPlayerDataSourceURL* iDataSource;
        PVPlayerDataSink* iDataSinkVideo;
        PVPlayerDataSink* iDataSinkAudio;
        PVPlayerDataSink* iDataSinkText;
        PVMFNodeInterface* iIONodeVideo;
        PVMFNodeInterface* iIONodeAudio;
        PVMFNodeInterface* iIONodeText;
        PvmiMIOControl* iMIOFileOutVideo;
        PvmiMIOControl* iMIOFileOutAudio;
        PvmiMIOControl* iMIOFileOutText;
        PVCommandId iCurrentCmdId;

        OSCL_wHeapString<OsclMemAllocator> iFileNameWStr;
        oscl_wchar iTmpWCharBuffer[512];
};


/*!
 *  A test case to test that second time playback starts from the beginning
 *  - Data Source: Specified by user of test case
 *  - Data Sink(s): Video[File Output MediaIO Interface Node-test_player_mionode_playrepostopplay_%SOURCEFILENAME%_video.dat]\n
 *                  Audio[File Output MediaIO Interface Node-test_player_mionode_playrepostopplay_%SOURCEFILENAME%_audio.dat]\n
 *                  Text[File Output MediaIO Interface Node-test_player_mionode_playrepostopplay_%SOURCEFILENAME%_text.dat]
 *  - Sequence:
 *             -# CreatePlayer()
 *             -# AddDataSource()
 *             -# Init()
 *             -# AddDataSink() (video)
 *             -# AddDataSink() (audio)
 *             -# AddDataSink() (text)
 *             -# Prepare()
 *             -# Start()
 *             -# WAIT 5 sec.
 *             -# SetPlaybackRange(20 sec, indeterminate)
 *			   -# WAIT 5 sec.
 *             -# Stop()
 *             -# Prepare()
 *             -# Start()
 *             -# WAIT 10 sec.
 *             -# Stop()
 *             -# RemoveDataSink() (video)
 *             -# RemoveDataSink() (audio)
 *             -# RemoveDataSink() (text)
 *             -# Reset()
 *             -# RemoveDataSource()
 *             -# DeletePlayer()
 *
 */
class pvplayer_async_test_mediaionode_playsetplaybackrangestopplay : public pvplayer_async_test_base
{
    public:
        pvplayer_async_test_mediaionode_playsetplaybackrangestopplay(PVPlayerAsyncTestParam aTestParam):
                pvplayer_async_test_base(aTestParam)
                , iPlayer(NULL)
                , iDataSource(NULL)
                , iDataSinkVideo(NULL)
                , iDataSinkAudio(NULL)
                , iDataSinkText(NULL)
                , iIONodeVideo(NULL)
                , iIONodeAudio(NULL)
                , iIONodeText(NULL)
                , iMIOFileOutVideo(NULL)
                , iMIOFileOutAudio(NULL)
                , iMIOFileOutText(NULL)
                , iCurrentCmdId(0)
        {
            iTestCaseName = _STRLIT_CHAR("MIONode Play-Reposition-Stop-Play");
        }

        ~pvplayer_async_test_mediaionode_playsetplaybackrangestopplay() {}

        void StartTest();
        void Run();

        void CommandCompleted(const PVCmdResponse& aResponse);
        void HandleErrorEvent(const PVAsyncErrorEvent& aEvent);
        void HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent);

        enum PVTestState
        {
            STATE_CREATE,
            STATE_ADDDATASOURCE,
            STATE_INIT,
            STATE_ADDDATASINK_VIDEO,
            STATE_ADDDATASINK_AUDIO,
            STATE_ADDDATASINK_TEXT,
            STATE_PREPARE1,
            STATE_START1,
            STATE_SETPLAYBACKRANGE,
            STATE_STOP1,
            STATE_PREPARE2,
            STATE_START2,
            STATE_STOP2,
            STATE_REMOVEDATASINK_VIDEO,
            STATE_REMOVEDATASINK_AUDIO,
            STATE_REMOVEDATASINK_TEXT,
            STATE_RESET,
            STATE_REMOVEDATASOURCE,
            STATE_CLEANUPANDCOMPLETE
        };

        PVTestState iState;

        PVPlayerInterface* iPlayer;
        PVPlayerDataSourceURL* iDataSource;
        PVPlayerDataSink* iDataSinkVideo;
        PVPlayerDataSink* iDataSinkAudio;
        PVPlayerDataSink* iDataSinkText;
        PVMFNodeInterface* iIONodeVideo;
        PVMFNodeInterface* iIONodeAudio;
        PVMFNodeInterface* iIONodeText;
        PvmiMIOControl* iMIOFileOutVideo;
        PvmiMIOControl* iMIOFileOutAudio;
        PvmiMIOControl* iMIOFileOutText;
        PVCommandId iCurrentCmdId;

        OSCL_wHeapString<OsclMemAllocator> iFileNameWStr;
        oscl_wchar iTmpWCharBuffer[512];
};


/*!
 *  A test case to test SetPlaybackRange() before Prepare() for second time playback
 *  - Data Source: Specified by user of test case
 *  - Data Sink(s): Video[File Output MediaIO Interface Node-test_player_mionode_playstopsetplaybackrangeplaystop_%SOURCEFILENAME%_video.dat]\n
 *                  Audio[File Output MediaIO Interface Node-test_player_mionode_playstopsetplaybackrangeplaystop_%SOURCEFILENAME%_audio.dat]\n
 *                  Text[File Output MediaIO Interface Node-test_player_mionode_playstopsetplaybackrangeplaystop_%SOURCEFILENAME%_text.dat]
 *  - Sequence:
 *             -# CreatePlayer()
 *             -# AddDataSource()
 *             -# Init()
 *             -# AddDataSink() (video)
 *             -# AddDataSink() (audio)
 *             -# AddDataSink() (text)
 *             -# Prepare()
 *             -# Start()
 *             -# WAIT 10 sec.
 *             -# Stop()
 *             -# SetPlaybackRange(30 sec, X)
 *             -# Prepare()
 *             -# Start()
 *             -# WAIT 10 sec. and check playback position
 *             -# Stop()
 *             -# RemoveDataSink() (video)
 *             -# RemoveDataSink() (audio)
 *             -# RemoveDataSink() (text)
 *             -# Reset()
 *             -# RemoveDataSource()
 *             -# DeletePlayer()
 *
 */
class pvplayer_async_test_mediaionode_playstopsetplaybackrangeplaystop : public pvplayer_async_test_base
{
    public:
        pvplayer_async_test_mediaionode_playstopsetplaybackrangeplaystop(PVPlayerAsyncTestParam aTestParam):
                pvplayer_async_test_base(aTestParam)
                , iPlayer(NULL)
                , iDataSource(NULL)
                , iDataSinkVideo(NULL)
                , iDataSinkAudio(NULL)
                , iDataSinkText(NULL)
                , iIONodeVideo(NULL)
                , iIONodeAudio(NULL)
                , iIONodeText(NULL)
                , iMIOFileOutVideo(NULL)
                , iMIOFileOutAudio(NULL)
                , iMIOFileOutText(NULL)
                , iCurrentCmdId(0)
        {
            iTestCaseName = _STRLIT_CHAR("MIONode Play-Stop-SetPlaybackRange-Play-Stop");
        }

        ~pvplayer_async_test_mediaionode_playstopsetplaybackrangeplaystop() {}

        void StartTest();
        void Run();

        void CommandCompleted(const PVCmdResponse& aResponse);
        void HandleErrorEvent(const PVAsyncErrorEvent& aEvent);
        void HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent);

        enum PVTestState
        {
            STATE_CREATE,
            STATE_ADDDATASOURCE,
            STATE_INIT,
            STATE_ADDDATASINK_VIDEO,
            STATE_ADDDATASINK_AUDIO,
            STATE_ADDDATASINK_TEXT,
            STATE_PREPARE1,
            STATE_START1,
            STATE_STOP1,
            STATE_SETPLAYBACKRANGE,
            STATE_PREPARE2,
            STATE_START2,
            STATE_STOP2,
            STATE_REMOVEDATASINK_VIDEO,
            STATE_REMOVEDATASINK_AUDIO,
            STATE_REMOVEDATASINK_TEXT,
            STATE_RESET,
            STATE_REMOVEDATASOURCE,
            STATE_CLEANUPANDCOMPLETE
        };

        PVTestState iState;

        PVPlayerInterface* iPlayer;
        PVPlayerDataSourceURL* iDataSource;
        PVPlayerDataSink* iDataSinkVideo;
        PVPlayerDataSink* iDataSinkAudio;
        PVPlayerDataSink* iDataSinkText;
        PVMFNodeInterface* iIONodeVideo;
        PVMFNodeInterface* iIONodeAudio;
        PVMFNodeInterface* iIONodeText;
        PvmiMIOControl* iMIOFileOutVideo;
        PvmiMIOControl* iMIOFileOutAudio;
        PvmiMIOControl* iMIOFileOutText;
        PVCommandId iCurrentCmdId;

        OSCL_wHeapString<OsclMemAllocator> iFileNameWStr;
        oscl_wchar iTmpWCharBuffer[512];
};


/*!
 *  A test case to test SetPlaybackRange() to near end of clip and then start
 *  - Data Source: Specified by user of test case
 *  - Data Sink(s): Video[File Output MediaIO Interface Node-test_player_mionode_setplaybackrangenearendplay_%SOURCEFILENAME%_video.dat]\n
 *                  Audio[File Output MediaIO Interface Node-test_player_mionode_setplaybackrangenearendplay_%SOURCEFILENAME%_audio.dat]\n
 *                  Text[File Output MediaIO Interface Node-test_player_mionode_setplaybackrangenearendplay_%SOURCEFILENAME%_text.dat]
 *  - Sequence:
 *             -# CreatePlayer()
 *             -# AddDataSource()
 *             -# Init()
 *             -# AddDataSink() (video)
 *             -# AddDataSink() (audio)
 *             -# AddDataSink() (text)
 *             -# GetMetadataValues() (retrieve clip duration)
 *             -# Prepare()
 *             -# SetPlaybackRange(1 sec before end of clip, X)
 *             -# Start()
 *             -# WAIT FOR EOS
 *             -# RemoveDataSink() (video)
 *             -# RemoveDataSink() (audio)
 *             -# RemoveDataSink() (text)
 *             -# Reset()
 *             -# RemoveDataSource()
 *             -# DeletePlayer()
 *
 */
class pvplayer_async_test_mediaionode_setplaybackrangenearendplay : public pvplayer_async_test_base
{
    public:
        pvplayer_async_test_mediaionode_setplaybackrangenearendplay(PVPlayerAsyncTestParam aTestParam):
                pvplayer_async_test_base(aTestParam)
                , iPlayer(NULL)
                , iDataSource(NULL)
                , iDataSinkVideo(NULL)
                , iDataSinkAudio(NULL)
                , iDataSinkText(NULL)
                , iIONodeVideo(NULL)
                , iIONodeAudio(NULL)
                , iIONodeText(NULL)
                , iMIOFileOutVideo(NULL)
                , iMIOFileOutAudio(NULL)
                , iMIOFileOutText(NULL)
                , iCurrentCmdId(0)
                , iSourceContextData(NULL)
        {
            iTestCaseName = _STRLIT_CHAR("MIONode SetPlaybackRange Near End-Play");
        }

        ~pvplayer_async_test_mediaionode_setplaybackrangenearendplay() {}

        void StartTest();
        void Run();

        void CommandCompleted(const PVCmdResponse& aResponse);
        void HandleErrorEvent(const PVAsyncErrorEvent& aEvent);
        void HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent);

        enum PVTestState
        {
            STATE_CREATE,
            STATE_ADDDATASOURCE,
            STATE_INIT,
            STATE_ADDDATASINK_VIDEO,
            STATE_ADDDATASINK_AUDIO,
            STATE_ADDDATASINK_TEXT,
            STATE_GETMETADATAVALUES,
            STATE_PREPARE,
            STATE_SETPLAYBACKRANGE,
            STATE_START,
            STATE_ENDTIMENOTREACHED,
            STATE_STOP,
            STATE_REMOVEDATASINK_VIDEO,
            STATE_REMOVEDATASINK_AUDIO,
            STATE_REMOVEDATASINK_TEXT,
            STATE_RESET,
            STATE_REMOVEDATASOURCE,
            STATE_CLEANUPANDCOMPLETE
        };

        PVTestState iState;

        PVPlayerInterface* iPlayer;
        PVPlayerDataSourceURL* iDataSource;
        PVPlayerDataSink* iDataSinkVideo;
        PVPlayerDataSink* iDataSinkAudio;
        PVPlayerDataSink* iDataSinkText;
        PVMFNodeInterface* iIONodeVideo;
        PVMFNodeInterface* iIONodeAudio;
        PVMFNodeInterface* iIONodeText;
        PvmiMIOControl* iMIOFileOutVideo;
        PvmiMIOControl* iMIOFileOutAudio;
        PvmiMIOControl* iMIOFileOutText;
        PVCommandId iCurrentCmdId;

        OSCL_wHeapString<OsclMemAllocator> iFileNameWStr;
        oscl_wchar iTmpWCharBuffer[512];

        PVPMetadataList iMetadataKeyList;
        Oscl_Vector<PvmiKvp, OsclMemAllocator> iMetadataValueList;
        int32 iNumValues;
        uint32 iStartPosition;

    private:
        OSCL_wHeapString<OsclMemAllocator> wFileName;
        oscl_wchar output[512];

        PVMFSourceContextData* iSourceContextData;
};


/*!
 *  A test case to test SetPlaybackRange() during playback near the end of the clip. Tests how EOS media command is handled when repositioning.
 *  - Data Source: Specified by user of test case
 *  - Data Sink(s): Video[File Output MediaIO Interface Node-test_player_mionode_playrepositionnearendofclip_%SOURCEFILENAME%_video.dat]\n
 *                  Audio[File Output MediaIO Interface Node-test_player_mionode_playrepositionnearendofclip_%SOURCEFILENAME%_audio.dat]\n
 *                  Text[File Output MediaIO Interface Node-test_player_mionode_playrepositionnearendofclip_%SOURCEFILENAME%_text.dat]
 *  - Sequence:
 *             -# CreatePlayer()
 *             -# AddDataSource()
 *             -# Init()
 *             -# AddDataSink() (video)
 *             -# AddDataSink() (audio)
 *             -# AddDataSink() (text)
 *             -# GetMetadataValues() (retrieve clip duration)
 *             -# Prepare()
 *             -# Start()
 *             -# WAIT UNTIL 2 SECONDS BEFORE CLIP DURATION
 *             -# SetPlaybackRange(0,X)
 *             -# WAIT FOR 5 SEC
 *             -# Stop()
 *             -# RemoveDataSink() (video)
 *             -# RemoveDataSink() (audio)
 *             -# RemoveDataSink() (text)
 *             -# Reset()
 *             -# RemoveDataSource()
 *             -# DeletePlayer()
 *
 */
class pvplayer_async_test_mediaionode_playrepositionnearendofclip : public pvplayer_async_test_base
{
    public:
        pvplayer_async_test_mediaionode_playrepositionnearendofclip(PVPlayerAsyncTestParam aTestParam):
                pvplayer_async_test_base(aTestParam)
                , iPlayer(NULL)
                , iDataSource(NULL)
                , iDataSinkVideo(NULL)
                , iDataSinkAudio(NULL)
                , iDataSinkText(NULL)
                , iIONodeVideo(NULL)
                , iIONodeAudio(NULL)
                , iIONodeText(NULL)
                , iMIOFileOutVideo(NULL)
                , iMIOFileOutAudio(NULL)
                , iMIOFileOutText(NULL)
                , iCurrentCmdId(0)
                , iSetPlayBackRangeIssued(false)
        {
            iTestCaseName = _STRLIT_CHAR("MIONode Play-Reposition Near End Of Clip");
        }

        ~pvplayer_async_test_mediaionode_playrepositionnearendofclip() {}

        void StartTest();
        void Run();

        void CommandCompleted(const PVCmdResponse& aResponse);
        void HandleErrorEvent(const PVAsyncErrorEvent& aEvent);
        void HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent);

        enum PVTestState
        {
            STATE_CREATE,
            STATE_ADDDATASOURCE,
            STATE_INIT,
            STATE_ADDDATASINK_VIDEO,
            STATE_ADDDATASINK_AUDIO,
            STATE_ADDDATASINK_TEXT,
            STATE_GETMETADATAVALUES,
            STATE_PREPARE,
            STATE_START,
            STATE_SETPLAYBACKRANGE,
            STATE_STOP,
            STATE_REMOVEDATASINK_VIDEO,
            STATE_REMOVEDATASINK_AUDIO,
            STATE_REMOVEDATASINK_TEXT,
            STATE_RESET,
            STATE_REMOVEDATASOURCE,
            STATE_CLEANUPANDCOMPLETE
        };

        PVTestState iState;

        PVPlayerInterface* iPlayer;
        PVPlayerDataSourceURL* iDataSource;
        PVPlayerDataSink* iDataSinkVideo;
        PVPlayerDataSink* iDataSinkAudio;
        PVPlayerDataSink* iDataSinkText;
        PVMFNodeInterface* iIONodeVideo;
        PVMFNodeInterface* iIONodeAudio;
        PVMFNodeInterface* iIONodeText;
        PvmiMIOControl* iMIOFileOutVideo;
        PvmiMIOControl* iMIOFileOutAudio;
        PvmiMIOControl* iMIOFileOutText;
        PVCommandId iCurrentCmdId;

        OSCL_wHeapString<OsclMemAllocator> iFileNameWStr;
        oscl_wchar iTmpWCharBuffer[512];

        PVPMetadataList iMetadataKeyList;
        Oscl_Vector<PvmiKvp, OsclMemAllocator> iMetadataValueList;
        int32 iNumValues;
        uint32 iClipDuration;
        bool iSetPlayBackRangeIssued;
};

/*!
 *  A test case to test forward step specified source with file output media IO node
 *  - Data Source: Specified by user of test case
 *  - Data Sink(s): Video[File Output MediaIO Interface Node-test_player_mionode_3Xplayrate_%SOURCEFILENAME%_video.dat]\n
 *                  Audio[File Output MediaIO Interface Node-test_player_mionode_3Xplayrate_%SOURCEFILENAME%_audio.dat]\n
 *                  Text[File Output MediaIO Interface Node-test_player_mionode_3Xplayrate_%SOURCEFILENAME%_text.dat]
 *  - Sequence:
 *             -# CreatePlayer()
 *             -# AddDataSource()
 *             -# Init()
 *             -# AddDataSink() (video)
 *             -# AddDataSink() (audio)
 *             -# AddDataSink() (text)
 *             -# Prepare()
 *             -# Start()
 *             (play for 2 seconds)
 *             -# Pause
 *             -# SetPlaybackRate (1x, step mode)
 *             -# Resume
 *             -# Step 1 frame, wait 1/2 sec, until 10 steps total
 *             -# Reposition to 20 second.
 *             -# Step 1 frame, wait 1/2 sec, until 25 steps total (incl the 10 prior)
 *             -# Pause
 *             -# SetPlaybackRate for normal 1x playback.
 *             -# Resume
 *             (play for 2 seconds)
 *             -# Stop()
 *             -# RemoveDataSink() (video)
 *             -# RemoveDataSink() (audio)
 *             -# RemoveDataSink() (text)
 *             -# Reset()
 *             -# RemoveDataSource()
 *             -# DeletePlayer()
 *
 */
class pvplayer_async_test_mediaionode_forwardstep : public pvplayer_async_test_base
{
    public:
        pvplayer_async_test_mediaionode_forwardstep(PVPlayerAsyncTestParam aTestParam, bool aActiveAudio = false):
                pvplayer_async_test_base(aTestParam)
                , iPlayer(NULL)
                , iDataSource(NULL)
                , iDataSinkVideo(NULL)
                , iDataSinkAudio(NULL)
                , iDataSinkText(NULL)
                , iIONodeVideo(NULL)
                , iIONodeAudio(NULL)
                , iIONodeText(NULL)
                , iMIOFileOutVideo(NULL)
                , iMIOFileOutAudio(NULL)
                , iMIOFileOutText(NULL)
                , iCurrentCmdId(0)
                , iActiveAudio(aActiveAudio)
                , iEOSReached(false)
        {
            if (iActiveAudio)
                iTestCaseName = _STRLIT_CHAR("MIONode Forward Step Active Audio");
            else
                iTestCaseName = _STRLIT_CHAR("MIONode Forward Step");
            iReposComplete = false;
        }

        ~pvplayer_async_test_mediaionode_forwardstep() {}

        void StartTest();
        void Run();

        void CommandCompleted(const PVCmdResponse& aResponse);
        void HandleErrorEvent(const PVAsyncErrorEvent& aEvent);
        void HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent);

        enum PVTestState
        {
            STATE_CREATE,
            STATE_ADDDATASOURCE,
            STATE_INIT,
            STATE_ADDDATASINK_VIDEO,
            STATE_ADDDATASINK_AUDIO,
            STATE_ADDDATASINK_TEXT,
            STATE_PREPARE,
            STATE_START,
            STATE_PAUSE,
            STATE_ENTERSTEPMODE,
            STATE_STARTSTEPPING,
            STATE_STEP,
            STATE_STOPSTEPPING,
            STATE_EXITSTEPMODE,
            STATE_RESUME,
            STATE_STOP,
            STATE_REMOVEDATASINK_VIDEO,
            STATE_REMOVEDATASINK_AUDIO,
            STATE_REMOVEDATASINK_TEXT,
            STATE_RESET,
            STATE_REMOVEDATASOURCE,
            STATE_CLEANUPANDCOMPLETE
        };

        PVTestState iState;

        PVPlayerInterface* iPlayer;
        PVPlayerDataSourceURL* iDataSource;
        PVPlayerDataSink* iDataSinkVideo;
        PVPlayerDataSink* iDataSinkAudio;
        PVPlayerDataSink* iDataSinkText;
        PVMFNodeInterface* iIONodeVideo;
        PVMFNodeInterface* iIONodeAudio;
        PVMFNodeInterface* iIONodeText;
        PvmiMIOControl* iMIOFileOutVideo;
        PvmiMIOControl* iMIOFileOutAudio;
        PvmiMIOControl* iMIOFileOutText;
        PVCommandId iCurrentCmdId;

        OSCL_wHeapString<OsclMemAllocator> iFileNameWStr;
        oscl_wchar iTmpWCharBuffer[512];

        PVMFTimebase_Count iSteppingTimebase;
        bool iReposComplete;
        bool iActiveAudio;
        int32 iCount;
        bool iEOSReached;
};

/*!
 *  A test case to test forward step playback of specified source with media output media IO node
 *  - Data Source: Specified by user of test case
 *  - Data Sink(s): Video[File Output MediaIO Interface Node-test_player_mionode_3Xplayrate_%SOURCEFILENAME%_video.dat]\n
 *                  Audio[File Output MediaIO Interface Node-test_player_mionode_3Xplayrate_%SOURCEFILENAME%_audio.dat]\n
 *                  Text[File Output MediaIO Interface Node-test_player_mionode_3Xplayrate_%SOURCEFILENAME%_text.dat]
 *  - Sequence:
 *             -# CreatePlayer()
 *             -# AddDataSource()
 *             -# Init()
 *             -# AddDataSink() (video)
 *             -# AddDataSink() (audio)
 *             -# AddDataSink() (text)
 *             -# Prepare()
 *             -# SetPlaybackRate(1x, step mode)
 *             -# Start()
 *             -# Step, wait 1/5 sec, step , wait, etc. until EOS reached.
 *             -# Stop()
 *             -# RemoveDataSink() (video)
 *             -# RemoveDataSink() (audio)
 *             -# RemoveDataSink() (text)
 *             -# Reset()
 *             -# RemoveDataSource()
 *             -# DeletePlayer()
 *
 */
class pvplayer_async_test_mediaionode_forwardsteptoeos : public pvplayer_async_test_base
{
    public:
        pvplayer_async_test_mediaionode_forwardsteptoeos(PVPlayerAsyncTestParam aTestParam):
                pvplayer_async_test_base(aTestParam)
                , iPlayer(NULL)
                , iDataSource(NULL)
                , iDataSinkVideo(NULL)
                , iDataSinkAudio(NULL)
                , iDataSinkText(NULL)
                , iIONodeVideo(NULL)
                , iIONodeAudio(NULL)
                , iIONodeText(NULL)
                , iMIOFileOutVideo(NULL)
                , iMIOFileOutAudio(NULL)
                , iMIOFileOutText(NULL)
                , iCurrentCmdId(0)
        {
            iTestCaseName = _STRLIT_CHAR("MIONode Forward Step to EOS");
        }

        ~pvplayer_async_test_mediaionode_forwardsteptoeos() {}

        void StartTest();
        void Run();

        void CommandCompleted(const PVCmdResponse& aResponse);
        void HandleErrorEvent(const PVAsyncErrorEvent& aEvent);
        void HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent);

        enum PVTestState
        {
            STATE_CREATE,
            STATE_ADDDATASOURCE,
            STATE_INIT,
            STATE_ADDDATASINK_VIDEO,
            STATE_ADDDATASINK_AUDIO,
            STATE_ADDDATASINK_TEXT,
            STATE_PREPARE,
            STATE_SETPLAYBACKRATE,
            STATE_START,
            STATE_STEP,
            STATE_STOP,
            STATE_REMOVEDATASINK_VIDEO,
            STATE_REMOVEDATASINK_AUDIO,
            STATE_REMOVEDATASINK_TEXT,
            STATE_RESET,
            STATE_REMOVEDATASOURCE,
            STATE_CLEANUPANDCOMPLETE
        };

        PVTestState iState;

        PVPlayerInterface* iPlayer;
        PVPlayerDataSourceURL* iDataSource;
        PVPlayerDataSink* iDataSinkVideo;
        PVPlayerDataSink* iDataSinkAudio;
        PVPlayerDataSink* iDataSinkText;
        PVMFNodeInterface* iIONodeVideo;
        PVMFNodeInterface* iIONodeAudio;
        PVMFNodeInterface* iIONodeText;
        PvmiMIOControl* iMIOFileOutVideo;
        PvmiMIOControl* iMIOFileOutAudio;
        PvmiMIOControl* iMIOFileOutText;
        PVCommandId iCurrentCmdId;

        OSCL_wHeapString<OsclMemAllocator> iFileNameWStr;
        oscl_wchar iTmpWCharBuffer[512];

        PVMFTimebase_Count iSteppingTimebase;

};

/*!
 *  A test case to test backward playback and backward stepping.
 *  - Data Source: Specified by user of test case
 *  - Data Sink(s): Video[File Output MediaIO Interface Node-test_player_mionode_3Xplayrate_%SOURCEFILENAME%_video.dat]\n
 *                  Audio[File Output MediaIO Interface Node-test_player_mionode_3Xplayrate_%SOURCEFILENAME%_audio.dat]\n
 *                  Text[File Output MediaIO Interface Node-test_player_mionode_3Xplayrate_%SOURCEFILENAME%_text.dat]
 *  - Sequence:
 *             -# CreatePlayer()
 *             -# AddDataSource()
 *             -# Init()
 *             -# AddDataSink() (video)
 *             -# AddDataSink() (audio)
 *             -# AddDataSink() (text)
 *             -# Prepare()
 *             -# Start()
 *             (play for 40 seconds)
 *             -# SetPlaybackRate(-1x)
 *             (play backward for 20 seconds)
 *             -# SetPlaybackRate(1x)
 *             (play forward for 10 seconds)
 *             -# Pause
 *             -# SetPlaybackRate(-1x, stepping mode)
 *             -# Resume
 *             -# Step, wait 1/2 sec, step , wait, etc. until EOS reached.
 *             -# Stop()
 *             -# RemoveDataSink() (video)
 *             -# RemoveDataSink() (audio)
 *             -# RemoveDataSink() (text)
 *             -# Reset()
 *             -# RemoveDataSource()
 *             -# DeletePlayer()
 *
 */
class pvplayer_async_test_mediaionode_backwardplayback : public pvplayer_async_test_base
{
    public:
        pvplayer_async_test_mediaionode_backwardplayback(PVPlayerAsyncTestParam aTestParam):
                pvplayer_async_test_base(aTestParam)
                , iPlayer(NULL)
                , iDataSource(NULL)
                , iDataSinkVideo(NULL)
                , iDataSinkAudio(NULL)
                , iDataSinkText(NULL)
                , iIONodeVideo(NULL)
                , iIONodeAudio(NULL)
                , iIONodeText(NULL)
                , iMIOFileOutVideo(NULL)
                , iMIOFileOutAudio(NULL)
                , iMIOFileOutText(NULL)
                , iCurrentCmdId(0)
        {
            iTestCaseName = _STRLIT_CHAR("MIONode Backward Playback");
        }

        ~pvplayer_async_test_mediaionode_backwardplayback() {}

        void StartTest();
        void Run();

        void CommandCompleted(const PVCmdResponse& aResponse);
        void HandleErrorEvent(const PVAsyncErrorEvent& aEvent);
        void HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent);

        enum PVTestState
        {
            STATE_CREATE,
            STATE_ADDDATASOURCE,
            STATE_INIT,
            STATE_ADDDATASINK_VIDEO,
            STATE_ADDDATASINK_AUDIO,
            STATE_ADDDATASINK_TEXT,
            STATE_PREPARE,
            STATE_START,
            STATE_GOBACKWARD,
            STATE_GOFORWARD,
            STATE_PAUSE,
            STATE_GOBACKWARDSTEP,
            STATE_RESUME,
            STATE_STEP,
            STATE_STOP,
            STATE_REMOVEDATASINK_VIDEO,
            STATE_REMOVEDATASINK_AUDIO,
            STATE_REMOVEDATASINK_TEXT,
            STATE_RESET,
            STATE_REMOVEDATASOURCE,
            STATE_CLEANUPANDCOMPLETE
        };

        PVTestState iState;

        PVPlayerInterface* iPlayer;
        PVPlayerDataSourceURL* iDataSource;
        PVPlayerDataSink* iDataSinkVideo;
        PVPlayerDataSink* iDataSinkAudio;
        PVPlayerDataSink* iDataSinkText;
        PVMFNodeInterface* iIONodeVideo;
        PVMFNodeInterface* iIONodeAudio;
        PVMFNodeInterface* iIONodeText;
        PvmiMIOControl* iMIOFileOutVideo;
        PvmiMIOControl* iMIOFileOutAudio;
        PvmiMIOControl* iMIOFileOutText;
        PVCommandId iCurrentCmdId;
        PVMFTimebase_Count iSteppingTimebase;

        OSCL_wHeapString<OsclMemAllocator> iFileNameWStr;
        oscl_wchar iTmpWCharBuffer[512];


};

/*!
 *  A test case to test Play->2xBackward playback->3x forward playback->normal playback
 *  - Data Source: Specified by user of test case
 *  - Data Sink(s): Video[File Output MediaIO Interface Node-test_player_mionode_backwardforwardplayback_%SOURCEFILENAME%_video.dat]\n
 *                  Audio[File Output MediaIO Interface Node-test_player_mionode_backwardforwardplayback_%SOURCEFILENAME%_audio.dat]\n
 *                  Text[File Output MediaIO Interface Node-test_player_mionode_backwardforwardplayback_%SOURCEFILENAME%_text.dat]
 *  - Sequence:
 *             -# CreatePlayer()
 *             -# AddDataSource()
 *             -# Init()
 *             -# AddDataSink() (video)
 *             -# AddDataSink() (audio)
 *             -# AddDataSink() (text)
 *             -# Prepare()
 *             -# Start()
 *             (play for 40 seconds)
 *             -# SetPlaybackRate(-2x)
 *             (play backward for 10 seconds)
 *             -# SetPlaybackRate(3x)
 *             (play forward for 10 seconds)
 *			   -# SetPlaybackRate(1x)
 *             (play until EOS)
 *             -# Stop()
 *             -# RemoveDataSink() (video)
 *             -# RemoveDataSink() (audio)
 *             -# RemoveDataSink() (text)
 *             -# Reset()
 *             -# RemoveDataSource()
 *             -# DeletePlayer()
 *
 */
class pvplayer_async_test_mediaionode_backwardforwardplayback : public pvplayer_async_test_base
{
    public:
        pvplayer_async_test_mediaionode_backwardforwardplayback(PVPlayerAsyncTestParam aTestParam):
                pvplayer_async_test_base(aTestParam)
                , iPlayer(NULL)
                , iDataSource(NULL)
                , iDataSinkVideo(NULL)
                , iDataSinkAudio(NULL)
                , iDataSinkText(NULL)
                , iIONodeVideo(NULL)
                , iIONodeAudio(NULL)
                , iIONodeText(NULL)
                , iMIOFileOutVideo(NULL)
                , iMIOFileOutAudio(NULL)
                , iMIOFileOutText(NULL)
                , iCurrentCmdId(0)
        {
            iTestCaseName = _STRLIT_CHAR("MIONode Backward Forward Playback");
        }

        ~pvplayer_async_test_mediaionode_backwardforwardplayback() {}

        void StartTest();
        void Run();

        void CommandCompleted(const PVCmdResponse& aResponse);
        void HandleErrorEvent(const PVAsyncErrorEvent& aEvent);
        void HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent);

        enum PVTestState
        {
            STATE_CREATE,
            STATE_ADDDATASOURCE,
            STATE_INIT,
            STATE_ADDDATASINK_VIDEO,
            STATE_ADDDATASINK_AUDIO,
            STATE_ADDDATASINK_TEXT,
            STATE_PREPARE,
            STATE_START,
            STATE_GOBACKWARD,
            STATE_GOFORWARD,
            STATE_GONORMAL,
            STATE_STOP,
            STATE_REMOVEDATASINK_VIDEO,
            STATE_REMOVEDATASINK_AUDIO,
            STATE_REMOVEDATASINK_TEXT,
            STATE_RESET,
            STATE_REMOVEDATASOURCE,
            STATE_CLEANUPANDCOMPLETE
        };

        PVTestState iState;

        PVPlayerInterface* iPlayer;
        PVPlayerDataSourceURL* iDataSource;
        PVPlayerDataSink* iDataSinkVideo;
        PVPlayerDataSink* iDataSinkAudio;
        PVPlayerDataSink* iDataSinkText;
        PVMFNodeInterface* iIONodeVideo;
        PVMFNodeInterface* iIONodeAudio;
        PVMFNodeInterface* iIONodeText;
        PvmiMIOControl* iMIOFileOutVideo;
        PvmiMIOControl* iMIOFileOutAudio;
        PvmiMIOControl* iMIOFileOutText;
        PVCommandId iCurrentCmdId;
        PVMFTimebase_Count iSteppingTimebase;

        OSCL_wHeapString<OsclMemAllocator> iFileNameWStr;
        oscl_wchar iTmpWCharBuffer[512];


};

/*!
 *  A test case to test Pause near end of clip then SetPlaybackRate(-4x) and then resume
 *  - Data Source: Specified by user of test case
 *  - Data Sink(s): Video[File Output MediaIO Interface Node-test_player_mionode_pauseneareosbackwardresume_%SOURCEFILENAME%_video.dat]\n
 *                  Audio[File Output MediaIO Interface Node-test_player_mionode_pauseneareosbackwardresume_%SOURCEFILENAME%_audio.dat]\n
 *                  Text[File Output MediaIO Interface Node-test_player_mionode_pauseneareosbackwardresume_%SOURCEFILENAME%_text.dat]
 *  - Sequence:
 *             -# CreatePlayer()
 *             -# AddDataSource()
 *             -# Init()
 *             -# AddDataSink() (video)
 *             -# AddDataSink() (audio)
 *             -# AddDataSink() (text)
 *             -# GetMetadataValues() (retrieve clip duration)
 *             -# Prepare()
 *             -# Start()
 *             -# Pause() (just 1 sec before EOS)
 *             -# SetPlaybackRate(-4x)
 *             -# Resume()
 *             -# SetPlaybackRate(1x) (After 20 secs of resume)
				  (normal play until EOS)
 *             -# Stop()
 *             -# RemoveDataSink() (video)
 *             -# RemoveDataSink() (audio)
 *             -# RemoveDataSink() (text)
 *             -# Reset()
 *             -# RemoveDataSource()
 *             -# DeletePlayer()
 *
 */
class pvplayer_async_test_mediaionode_pauseneareosbackwardresume : public pvplayer_async_test_base
{
    public:
        pvplayer_async_test_mediaionode_pauseneareosbackwardresume(PVPlayerAsyncTestParam aTestParam):
                pvplayer_async_test_base(aTestParam)
                , iPlayer(NULL)
                , iDataSource(NULL)
                , iDataSinkVideo(NULL)
                , iDataSinkAudio(NULL)
                , iDataSinkText(NULL)
                , iIONodeVideo(NULL)
                , iIONodeAudio(NULL)
                , iIONodeText(NULL)
                , iMIOFileOutVideo(NULL)
                , iMIOFileOutAudio(NULL)
                , iMIOFileOutText(NULL)
                , iCurrentCmdId(0)
        {
            iTestCaseName = _STRLIT_CHAR("MIONode Pause Near End-Play, SetPlaybackRate -4x, resume");
        }

        ~pvplayer_async_test_mediaionode_pauseneareosbackwardresume() {}

        void StartTest();
        void Run();

        void CommandCompleted(const PVCmdResponse& aResponse);
        void HandleErrorEvent(const PVAsyncErrorEvent& aEvent);
        void HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent);

        enum PVTestState
        {
            STATE_CREATE,
            STATE_ADDDATASOURCE,
            STATE_INIT,
            STATE_ADDDATASINK_VIDEO,
            STATE_ADDDATASINK_AUDIO,
            STATE_ADDDATASINK_TEXT,
            STATE_GETMETADATAVALUES,
            STATE_PREPARE,
            STATE_START,
            STATE_PAUSE,
            STATE_SETBACKWARD,
            STATE_RESUME,
            STATE_GONORMAL,
            STATE_STOP,
            STATE_REMOVEDATASINK_VIDEO,
            STATE_REMOVEDATASINK_AUDIO,
            STATE_REMOVEDATASINK_TEXT,
            STATE_RESET,
            STATE_REMOVEDATASOURCE,
            STATE_CLEANUPANDCOMPLETE
        };

        PVTestState iState;

        PVPlayerInterface* iPlayer;
        PVPlayerDataSourceURL* iDataSource;
        PVPlayerDataSink* iDataSinkVideo;
        PVPlayerDataSink* iDataSinkAudio;
        PVPlayerDataSink* iDataSinkText;
        PVMFNodeInterface* iIONodeVideo;
        PVMFNodeInterface* iIONodeAudio;
        PVMFNodeInterface* iIONodeText;
        PvmiMIOControl* iMIOFileOutVideo;
        PvmiMIOControl* iMIOFileOutAudio;
        PvmiMIOControl* iMIOFileOutText;
        PVCommandId iCurrentCmdId;

        OSCL_wHeapString<OsclMemAllocator> iFileNameWStr;
        oscl_wchar iTmpWCharBuffer[512];

        PVPMetadataList iMetadataKeyList;
        Oscl_Vector<PvmiKvp, OsclMemAllocator> iMetadataValueList;
        int32 iNumValues;
        uint32 iStartPosition;
};

/*!
 *  A test case to test Multiple occurences of Pause then SetPlaybackRate and then resume combination
 *  - Data Source: Specified by user of test case
 *  - Data Sink(s): Video[File Output MediaIO Interface Node-test_player_mionode_multiplepausesetplaybackrateresume_%SOURCEFILENAME%_video.dat]\n
 *                  Audio[File Output MediaIO Interface Node-test_player_mionode_multiplepausesetplaybackrateresume_%SOURCEFILENAME%_audio.dat]\n
 *                  Text[File Output MediaIO Interface Node-test_player_mionode_multiplepausesetplaybackrateresume_%SOURCEFILENAME%_text.dat]
 *  - Sequence:
 *             -# CreatePlayer()
 *             -# AddDataSource()
 *             -# Init()
 *             -# AddDataSink() (video)
 *             -# AddDataSink() (audio)
 *             -# AddDataSink() (text)
 *             -# GetMetadataValues() (retrieve clip duration)
 *             -# Prepare()
 *             -# Start()
 *             -# Pause()   (Multiple times in loop)
 *             -# SetPlaybackRate(x)
 *             -# Resume()
 *             -# SetPlaybackRate(1x) (After 20 secs of resume)
 *             -# Stop()
 *             -# RemoveDataSink() (video)
 *             -# RemoveDataSink() (audio)
 *             -# RemoveDataSink() (text)
 *             -# Reset()
 *             -# RemoveDataSource()
 *             -# DeletePlayer()
 *
 */
class pvplayer_async_test_mediaionode_multiplepausesetplaybackrateresume : public pvplayer_async_test_base
{
    public:
        pvplayer_async_test_mediaionode_multiplepausesetplaybackrateresume(PVPlayerAsyncTestParam aTestParam):
                pvplayer_async_test_base(aTestParam)
                , iPlayer(NULL)
                , iDataSource(NULL)
                , iDataSinkVideo(NULL)
                , iDataSinkAudio(NULL)
                , iDataSinkText(NULL)
                , iIONodeVideo(NULL)
                , iIONodeAudio(NULL)
                , iIONodeText(NULL)
                , iMIOFileOutVideo(NULL)
                , iMIOFileOutAudio(NULL)
                , iMIOFileOutText(NULL)
                , iCurrentCmdId(0)
        {
            iTestCaseName = _STRLIT_CHAR("MIONode Multiple Pause-SetPlaybackRate-Resume");
            iNumSetRate = 0;
            iPlaybackRate = 0;
        }

        ~pvplayer_async_test_mediaionode_multiplepausesetplaybackrateresume() {}

        void StartTest();
        void Run();

        void CommandCompleted(const PVCmdResponse& aResponse);
        void HandleErrorEvent(const PVAsyncErrorEvent& aEvent);
        void HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent);

        enum PVTestState
        {
            STATE_CREATE,
            STATE_ADDDATASOURCE,
            STATE_INIT,
            STATE_ADDDATASINK_VIDEO,
            STATE_ADDDATASINK_AUDIO,
            STATE_ADDDATASINK_TEXT,
            STATE_GETMETADATAVALUES,
            STATE_PREPARE,
            STATE_START,
            STATE_PAUSE,
            STATE_SETPLAYBACKRATE,
            STATE_RESUME,
            STATE_GONORMAL,
            STATE_STOP,
            STATE_REMOVEDATASINK_VIDEO,
            STATE_REMOVEDATASINK_AUDIO,
            STATE_REMOVEDATASINK_TEXT,
            STATE_RESET,
            STATE_REMOVEDATASOURCE,
            STATE_CLEANUPANDCOMPLETE
        };

        PVTestState iState;

        PVPlayerInterface* iPlayer;
        PVPlayerDataSourceURL* iDataSource;
        PVPlayerDataSink* iDataSinkVideo;
        PVPlayerDataSink* iDataSinkAudio;
        PVPlayerDataSink* iDataSinkText;
        PVMFNodeInterface* iIONodeVideo;
        PVMFNodeInterface* iIONodeAudio;
        PVMFNodeInterface* iIONodeText;
        PvmiMIOControl* iMIOFileOutVideo;
        PvmiMIOControl* iMIOFileOutAudio;
        PvmiMIOControl* iMIOFileOutText;
        PVCommandId iCurrentCmdId;

        OSCL_wHeapString<OsclMemAllocator> iFileNameWStr;
        oscl_wchar iTmpWCharBuffer[512];

        PVPMetadataList iMetadataKeyList;
        Oscl_Vector<PvmiKvp, OsclMemAllocator> iMetadataValueList;
        int32 iNumValues;
        uint32 iStartPosition;

        int32 iNumSetRate;
        int32 iPlaybackRate;
};

/*!
 *  A test case to test Pause near end of clip then SetPlaybackRate(-4x) and then resume till Beginning of stream
 *  then play forward normal
 *  - Data Source: Specified by user of test case
 *  - Data Sink(s): Video[File Output MediaIO Interface Node-test_player_mionode_backwardneareosforwardnearbos_%SOURCEFILENAME%_video.dat]\n
 *                  Audio[File Output MediaIO Interface Node-test_player_mionode_backwardneareosforwardnearbos_%SOURCEFILENAME%_audio.dat]\n
 *                  Text[File Output MediaIO Interface Node-test_player_mionode_backwardneareosforwardnearbos_%SOURCEFILENAME%_text.dat]
 *  - Sequence:
 *             -# CreatePlayer()
 *             -# AddDataSource()
 *             -# Init()
 *             -# AddDataSink() (video)
 *             -# AddDataSink() (audio)
 *             -# AddDataSink() (text)
 *             -# GetMetadataValues() (retrieve clip duration)
 *             -# Prepare()
 *             -# Start()
 *             -# Pause() (just 1 sec before EOS)
 *             -# SetPlaybackRate(-4x)
 *             -# Resume() (Fast backward till BOS)
 *             -# SetPlaybackRate(1x)
				  (normal play until EOS)
 *             -# Stop()
 *             -# RemoveDataSink() (video)
 *             -# RemoveDataSink() (audio)
 *             -# RemoveDataSink() (text)
 *             -# Reset()
 *             -# RemoveDataSource()
 *             -# DeletePlayer()
 *
 */
class pvplayer_async_test_mediaionode_backwardneareosforwardnearbos : public pvplayer_async_test_base
{
    public:
        pvplayer_async_test_mediaionode_backwardneareosforwardnearbos(PVPlayerAsyncTestParam aTestParam):
                pvplayer_async_test_base(aTestParam)
                , iPlayer(NULL)
                , iDataSource(NULL)
                , iDataSinkVideo(NULL)
                , iDataSinkAudio(NULL)
                , iDataSinkText(NULL)
                , iIONodeVideo(NULL)
                , iIONodeAudio(NULL)
                , iIONodeText(NULL)
                , iMIOFileOutVideo(NULL)
                , iMIOFileOutAudio(NULL)
                , iMIOFileOutText(NULL)
                , iCurrentCmdId(0)
                , iNumPlay(0)
        {
            iTestCaseName = _STRLIT_CHAR("MIONode Fast Rewind Near EOS till BOS then Normal Forward");
        }

        ~pvplayer_async_test_mediaionode_backwardneareosforwardnearbos() {}

        void StartTest();
        void Run();

        void CommandCompleted(const PVCmdResponse& aResponse);
        void HandleErrorEvent(const PVAsyncErrorEvent& aEvent);
        void HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent);

        enum PVTestState
        {
            STATE_CREATE,
            STATE_ADDDATASOURCE,
            STATE_INIT,
            STATE_ADDDATASINK_VIDEO,
            STATE_ADDDATASINK_AUDIO,
            STATE_ADDDATASINK_TEXT,
            STATE_GETMETADATAVALUES,
            STATE_PREPARE,
            STATE_START,
            STATE_PAUSE,
            STATE_SETBACKWARD,
            STATE_RESUME,
            STATE_GONORMAL,
            STATE_STOP,
            STATE_REMOVEDATASINK_VIDEO,
            STATE_REMOVEDATASINK_AUDIO,
            STATE_REMOVEDATASINK_TEXT,
            STATE_RESET,
            STATE_REMOVEDATASOURCE,
            STATE_CLEANUPANDCOMPLETE
        };

        PVTestState iState;

        PVPlayerInterface* iPlayer;
        PVPlayerDataSourceURL* iDataSource;
        PVPlayerDataSink* iDataSinkVideo;
        PVPlayerDataSink* iDataSinkAudio;
        PVPlayerDataSink* iDataSinkText;
        PVMFNodeInterface* iIONodeVideo;
        PVMFNodeInterface* iIONodeAudio;
        PVMFNodeInterface* iIONodeText;
        PvmiMIOControl* iMIOFileOutVideo;
        PvmiMIOControl* iMIOFileOutAudio;
        PvmiMIOControl* iMIOFileOutText;
        PVCommandId iCurrentCmdId;

        OSCL_wHeapString<OsclMemAllocator> iFileNameWStr;
        oscl_wchar iTmpWCharBuffer[512];

        PVPMetadataList iMetadataKeyList;
        Oscl_Vector<PvmiKvp, OsclMemAllocator> iMetadataValueList;
        int32 iNumValues;
        uint32 iStartPosition;
        uint32 iNumPlay;
};
#endif // TEST_PV_PLAYER_ENGINE_TESTSET7_H_INCLUDED


