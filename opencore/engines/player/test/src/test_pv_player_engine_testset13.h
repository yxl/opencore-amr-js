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
#ifndef TEST_PV_PLAYER_ENGINE_TESTSET13_H_INCLUDED
#define TEST_PV_PLAYER_ENGINE_TESTSET13_H_INCLUDED

/**
 *  @file test_pv_player_engine_testset13.h
 *  @brief This file contains the class definitions for the Thirteenth set of
 *         test cases for PVPlayerEngine - Generic tests
 *
 */

#ifndef TEST_PV_PLAYER_ENGINE_H_INCLUDED
#include "test_pv_player_engine.h"
#endif

#ifndef TEST_PV_PLAYER_ENGINE_CONFIG_H_INCLUDED
#include "test_pv_player_engine_config.h"
#endif

#ifndef PVMF_STREAMING_DATA_SOURCE_H_INCLUDED
#include "pvmf_streaming_data_source.h"
#endif

#if RUN_FASTTRACK_TESTCASES
#ifndef PVPVXPARSER_H_INCLUDED
#include "pvpvxparser.h"
#endif
#endif

#ifndef PVMF_DOWNLOAD_DATA_SOURCE_H_INCLUDED
#include "pvmf_download_data_source.h"
#endif

#ifndef PVMF_SOURCE_CONTEXT_DATA_H_INCLUDED
#include "pvmf_source_context_data.h"
#endif

#ifndef DEFAULT_URLS_DEFINED
#define DEFAULT_URLS_DEFINED
#define AMR_MPEG4_RTSP_URL ""
#define AMR_MPEG4_RTSP_URL_2 ""
#define H263_AMR_RTSP_URL ""
#define MPEG4_RTSP_URL ""
#define MPEG4_SHRT_HDR_RTSP_URL ""
#define AAC_RTSP_URL     ""
#define MPEG4_AAC_RTSP_URL ""
#define AMR_MPEG4_SDP_FILE "pv_amr_mpeg4.sdp"
#endif

#define DEFAULT_GATEWAY "10.112.43.1"

class PVPlayerDataSourceURL;
class PVPlayerDataSink;
class PVPlayerDataSink;
class PVPlayerDataSinkFilename;
class PvmfFileOutputNodeConfigInterface;
class PvmiCapabilityAndConfig;
class PVMFDownloadDataSourcePVX;


/*!
 *  A test case to count time taken in each command completion when playing an local/PDL/rtsp url
 *  - Data Source: Specified by user of test case
 *  - Data Sink(s): Video[FileOutputNode-test_player_genericprofiling_video.dat]\n
 *                  Audio[FileOutputNode-test_player_genericprofiling_audio.dat]
 *  - Sequence:
 *             -# CreatePlayer()
 *             -# AddDataSource()
 *             -# Init()
 *             -# AddDataSink() (video)
 *             -# AddDataSink() (audio)
 *             -# Prepare()
 *             -# Start()
 *             -# Play 10 sec
 *             -# SetPlaybackRange() if seekEnable flag is made true
 *             -# Pause() if pauseresumeEnable flag is made true
 *             -# WAIT 10 sec
 *             -# Resume()
 *             -# Play until EOS
 *             -# Stop()
 *             -# RemoveDataSink() (video)
 *             -# RemoveDataSink() (audio)
 *             -# Reset()
 *             -# RemoveDataSource()
 *             -# DeletePlayer()
 *
 */
class pvplayer_async_test_genericprofiling : public pvplayer_async_test_base
{
    public:
        pvplayer_async_test_genericprofiling(PVPlayerAsyncTestParam aTestParam,
                                             PVMFFormatType aVideoSinkFormat,
                                             PVMFFormatType aAudioSinkFormat,
                                             uint32 aTestID,
                                             bool aWaitForEOS,
                                             bool aCloaking,
                                             bool aCancelDuringPrepare)
                : pvplayer_async_test_base(aTestParam)
                , iPlayer(NULL)
                , iDataSource(NULL)
                , iDataSinkVideo(NULL)
                , iDataSinkAudio(NULL)
                , iIONodeVideo(NULL)
                , iIONodeAudio(NULL)
                , iMIOFileOutVideo(NULL)
                , iMIOFileOutAudio(NULL)
                , iCurrentCmdId(0)
                , iCancelAllCmdId(0)
                , iSeekAfterEOSEnable(false)
                , iSessionDuration(0)
                , bcloaking(aCloaking)
                , oLiveSession(false)
                , iProtocolRollOver(false)
                , iProtocolRollOverWithUnknownURLType(false)
                , iPlayListURL(false)
                , iStreamDataSource(NULL)
                , iSourceContextData(NULL)
                , iDownloadContextDataPVX(NULL)
        {
            iVideoSinkFormatType = aVideoSinkFormat;
            iAudioSinkFormatType = aAudioSinkFormat;
            oWaitForEOS = aWaitForEOS;
            oCancelDuringPrepare = aCancelDuringPrepare;
            iTestID = aTestID;
            iNumPlay = 0;
            iTargetNumPlay = 1;
            iNumBufferingStart = iNumBufferingComplete = iNumUnderflow = iNumDataReady = iNumEOS = 0;
            iDownloadOnly = iDownloadThenPlay = false;
            iCurrenttime = 0;
            iPrevtime = 0;
            iOverflow = false;
            iPauseResumeDone = false;
            iSeekDone = false;
        }

        ~pvplayer_async_test_genericprofiling() {}

        void StartTest();
        void Run();

        void CommandCompleted(const PVCmdResponse& aResponse);
        void HandleErrorEvent(const PVAsyncErrorEvent& aEvent);
        void HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent);

        /*enum PVTestState
        {
        	STATE_CREATE,
            STATE_QUERYINTERFACE,
        	STATE_ADDDATASOURCE,
        	STATE_CONFIGPARAMS,
        	STATE_INIT,
        	STATE_GETMETADATAKEYLIST,
        	STATE_GETMETADATAVALUELIST,
        	STATE_ADDDATASINK_VIDEO,
        	STATE_ADDDATASINK_AUDIO,
        	STATE_PREPARE,
        	STATE_CANCELALL,
        	STATE_WAIT_FOR_CANCELALL,
        	STATE_START,
        	STATE_SETPLAYBACKRANGE,
        	STATE_PAUSE,
        	STATE_RESUME,
        	STATE_EOSNOTREACHED,
        	STATE_STOP,
        	STATE_REMOVEDATASINK_VIDEO,
        	STATE_REMOVEDATASINK_AUDIO,
        	STATE_RESET,
        	STATE_REMOVEDATASOURCE,
        	STATE_CLEANUPANDCOMPLETE
        };*/

        PVTestState iState;

        PVPlayerInterface* iPlayer;
        PVPlayerDataSourceURL* iDataSource;
        PVPlayerDataSink* iDataSinkVideo;
        PVPlayerDataSink* iDataSinkAudio;
        PVMFNodeInterface* iIONodeVideo;
        PVMFNodeInterface* iIONodeAudio;
        PvmiMIOControl* iMIOFileOutVideo;
        PvmiMIOControl* iMIOFileOutAudio;
        PVCommandId iCurrentCmdId;
        PVCommandId iCancelAllCmdId;

        void setMultiplePlayMode(uint32 aNum)
        {
            iMultiplePlay = true;
            iTargetNumPlay = aNum;
        }

        void setProtocolRollOverMode()
        {
            iProtocolRollOver = true;
        }

        void setProtocolRollOverModeWithUnknownURL()
        {
            iProtocolRollOverWithUnknownURLType = true;
        }

        void setPlayListMode()
        {
            iPlayListURL = true;
        }

        void setSeekAfterEOSMode()
        {
            iSeekAfterEOSEnable = true;
        }

    private:
        void HandleSocketNodeErrors(int32 aErr);
        void HandleRTSPNodeErrors(int32 aErr);
        void HandleStreamingManagerNodeErrors(int32 aErr);
        void HandleJitterBufferNodeErrors(int32 aErr);
        void HandleMediaLayerNodeErrors(int32 aErr);
        void HandleProtocolEngineNodeErrors(int32 aErr);

        void PrintMetadataInfo();

        PVMFFormatType iVideoSinkFormatType;
        PVMFFormatType iAudioSinkFormatType;
        OSCL_wHeapString<OsclMemAllocator> wFileName;
        oscl_wchar output[512];
        bool oCancelDuringPrepare;
        bool oWaitForEOS;
        uint32 iTestID;
        bool iSeekAfterEOSEnable;

        PVPMetadataList iMetadataKeyList;
        Oscl_Vector<PvmiKvp, OsclMemAllocator> iMetadataValueList;
        int32 iNumValues;

        uint32 iSessionDuration;
        bool bcloaking;
        bool oLiveSession;

        bool iMultiplePlay;
        uint32 iNumPlay;
        uint32 iTargetNumPlay;
        bool iPauseResumeDone;
        bool iSeekDone;

        bool iProtocolRollOver;
        bool iProtocolRollOverWithUnknownURLType;
        bool iPlayListURL;

        PVMFTimebase_Tickcount tb;
        PVMFMediaClock iClock;
        //For clock usage
        uint32 iCurrenttime;
        uint32 iPrevtime;
        bool iOverflow;

        PvmiCapabilityAndConfig* iPlayerCapConfigIF;
        PvmiKvp* iErrorKVP;
        PvmiKvp iKVPSetAsync;
        OSCL_StackString<128> iKeyStringSetAsync;

        PVMFStreamingDataSource* iStreamDataSource;

        PVMFSourceContextData* iSourceContextData;

        //FTDL
        void CreateDownloadDataSource();
        uint8 iPVXFileBuf[4096];
        PVMFDownloadDataSourcePVX* iDownloadContextDataPVX;
        PVMFDownloadDataSourceHTTP* iDownloadContextDataHTTP;
        int32 iDownloadMaxfilesize;
#if RUN_FASTTRACK_TESTCASES
        CPVXInfo iDownloadPvxInfo;
#endif
        OSCL_wHeapString<OsclMemAllocator> iDownloadURL;
        OSCL_wHeapString<OsclMemAllocator> iDownloadFilename;
        OSCL_HeapString<OsclMemAllocator> iDownloadProxy;
        OSCL_wHeapString<OsclMemAllocator> iDownloadConfigFilename;
        int32 iNumBufferingStart, iNumBufferingComplete, iNumUnderflow, iNumDataReady, iNumEOS;
        bool iDownloadOnly, iDownloadThenPlay;
        bool iContentTooLarge;
};

/*!
 *  A test case to test seek while in paused state when playing an local/PDL/rtsp url
 *  - Data Source: Specified by user of test case
 *  - Data Sink(s): Video[FileOutputNode-test_player_genericplaypauserepositionresume_video.dat]\n
 *                  Audio[FileOutputNode-test_player_genericplaypauserepositionresume_audio.dat]
 *  - Sequence:
 *             -# CreatePlayer()
 *             -# AddDataSource()
 *             -# Init()
 *             -# AddDataSink() (video)
 *             -# AddDataSink() (audio)
 *             -# Prepare()
 *             -# Start()
 *             -# Play 10 sec
 *             -# Pause()
 *             -# WAIT 5 sec
 *             -# SetPlaybackRange()
 *             -# WAIT 5 sec
 *             -# Resume()
 *             -# Play until EOS
 *             -# Stop()
 *             -# RemoveDataSink() (video)
 *             -# RemoveDataSink() (audio)
 *             -# Reset()
 *             -# RemoveDataSource()
 *             -# DeletePlayer()
 *
 */
class pvplayer_async_test_genericplaypauserepositionresumetest : public pvplayer_async_test_base
{
    public:
        pvplayer_async_test_genericplaypauserepositionresumetest(PVPlayerAsyncTestParam aTestParam,
                PVMFFormatType aVideoSinkFormat,
                PVMFFormatType aAudioSinkFormat,
                uint32 aTestID,
                bool aWaitForEOS,
                bool aCloaking,
                bool aCancelDuringPrepare)
                : pvplayer_async_test_base(aTestParam)
                , iPlayer(NULL)
                , iDataSource(NULL)
                , iDataSinkVideo(NULL)
                , iDataSinkAudio(NULL)
                , iIONodeVideo(NULL)
                , iIONodeAudio(NULL)
                , iMIOFileOutVideo(NULL)
                , iMIOFileOutAudio(NULL)
                , iCurrentCmdId(0)
                , iCancelAllCmdId(0)
                , iSeekAfterEOSEnable(false)
                , iSessionDuration(0)
                , bcloaking(aCloaking)
                , oLiveSession(false)
                , iProtocolRollOver(false)
                , iProtocolRollOverWithUnknownURLType(false)
                , iPlayListURL(false)
                , iStreamDataSource(NULL)
                , iSourceContextData(NULL)
                , iDownloadContextDataPVX(NULL)
        {
            iVideoSinkFormatType = aVideoSinkFormat;
            iAudioSinkFormatType = aAudioSinkFormat;
            oWaitForEOS = aWaitForEOS;
            oCancelDuringPrepare = aCancelDuringPrepare;
            iTestID = aTestID;
            iNumPlay = 0;
            iTargetNumPlay = 1;
            iNumBufferingStart = iNumBufferingComplete = iNumUnderflow = iNumDataReady = iNumEOS = 0;
            iDownloadOnly = iDownloadThenPlay = false;
            iPauseDone = false;
            iSeekDone = false;
        }

        ~pvplayer_async_test_genericplaypauserepositionresumetest() {}

        void StartTest();
        void Run();

        void CommandCompleted(const PVCmdResponse& aResponse);
        void HandleErrorEvent(const PVAsyncErrorEvent& aEvent);
        void HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent);

        /*enum PVTestState
        {
        	STATE_CREATE,
            STATE_QUERYINTERFACE,
        	STATE_ADDDATASOURCE,
        	STATE_CONFIGPARAMS,
        	STATE_INIT,
        	STATE_GETMETADATAKEYLIST,
        	STATE_GETMETADATAVALUELIST,
        	STATE_ADDDATASINK_VIDEO,
        	STATE_ADDDATASINK_AUDIO,
        	STATE_PREPARE,
        	STATE_CANCELALL,
        	STATE_WAIT_FOR_CANCELALL,
        	STATE_START,
        	STATE_SETPLAYBACKRANGE,
        	STATE_PAUSE,
        	STATE_RESUME,
        	STATE_EOSNOTREACHED,
        	STATE_STOP,
        	STATE_REMOVEDATASINK_VIDEO,
        	STATE_REMOVEDATASINK_AUDIO,
        	STATE_RESET,
        	STATE_REMOVEDATASOURCE,
        	STATE_CLEANUPANDCOMPLETE
        };*/

        PVTestState iState;

        PVPlayerInterface* iPlayer;
        PVPlayerDataSourceURL* iDataSource;
        PVPlayerDataSink* iDataSinkVideo;
        PVPlayerDataSink* iDataSinkAudio;
        PVMFNodeInterface* iIONodeVideo;
        PVMFNodeInterface* iIONodeAudio;
        PvmiMIOControl* iMIOFileOutVideo;
        PvmiMIOControl* iMIOFileOutAudio;
        PVCommandId iCurrentCmdId;
        PVCommandId iCancelAllCmdId;

        void setMultiplePlayMode(uint32 aNum)
        {
            iMultiplePlay = true;
            iTargetNumPlay = aNum;
        }

        void setProtocolRollOverMode()
        {
            iProtocolRollOver = true;
        }

        void setProtocolRollOverModeWithUnknownURL()
        {
            iProtocolRollOverWithUnknownURLType = true;
        }

        void setPlayListMode()
        {
            iPlayListURL = true;
        }

        void setSeekAfterEOSMode()
        {
            iSeekAfterEOSEnable = true;
        }

    private:
        void HandleSocketNodeErrors(int32 aErr);
        void HandleRTSPNodeErrors(int32 aErr);
        void HandleStreamingManagerNodeErrors(int32 aErr);
        void HandleJitterBufferNodeErrors(int32 aErr);
        void HandleMediaLayerNodeErrors(int32 aErr);
        void HandleProtocolEngineNodeErrors(int32 aErr);

        void PrintMetadataInfo();

        PVMFFormatType iVideoSinkFormatType;
        PVMFFormatType iAudioSinkFormatType;
        OSCL_wHeapString<OsclMemAllocator> wFileName;
        oscl_wchar output[512];
        bool oCancelDuringPrepare;
        bool oWaitForEOS;
        uint32 iTestID;
        bool iSeekAfterEOSEnable;

        PVPMetadataList iMetadataKeyList;
        Oscl_Vector<PvmiKvp, OsclMemAllocator> iMetadataValueList;
        int32 iNumValues;

        uint32 iSessionDuration;
        bool bcloaking;
        bool oLiveSession;


        bool iMultiplePlay;
        uint32 iNumPlay;
        uint32 iTargetNumPlay;
        bool iPauseDone;
        bool iSeekDone;

        bool iProtocolRollOver;
        bool iProtocolRollOverWithUnknownURLType;
        bool iPlayListURL;

        PvmiCapabilityAndConfig* iPlayerCapConfigIF;
        PvmiKvp* iErrorKVP;
        PvmiKvp iKVPSetAsync;
        OSCL_StackString<128> iKeyStringSetAsync;

        PVMFStreamingDataSource* iStreamDataSource;

        PVMFSourceContextData* iSourceContextData;

        //FTDL
        void CreateDownloadDataSource();
        uint8 iPVXFileBuf[4096];
        PVMFDownloadDataSourcePVX* iDownloadContextDataPVX;
        PVMFDownloadDataSourceHTTP* iDownloadContextDataHTTP;
        int32 iDownloadMaxfilesize;
#if RUN_FASTTRACK_TESTCASES
        CPVXInfo iDownloadPvxInfo;
#endif
        OSCL_wHeapString<OsclMemAllocator> iDownloadURL;
        OSCL_wHeapString<OsclMemAllocator> iDownloadFilename;
        OSCL_HeapString<OsclMemAllocator> iDownloadProxy;
        OSCL_wHeapString<OsclMemAllocator> iDownloadConfigFilename;
        int32 iNumBufferingStart, iNumBufferingComplete, iNumUnderflow, iNumDataReady, iNumEOS;
        bool iDownloadOnly, iDownloadThenPlay;
        bool iContentTooLarge;
};

/*!
 *  A test case to test seek before start when playing an local/PDL/rtsp url
 *  - Data Source: Specified by user of test case
 *  - Data Sink(s): Video[FileOutputNode-test_player_genericopensetplaybackrangestartplaystop_video.dat]\n
 *                  Audio[FileOutputNode-test_player_genericopensetplaybackrangestartplaystop_audio.dat]
 *  - Sequence:
 *             -# CreatePlayer()
 *             -# AddDataSource()
 *             -# Init()
 *             -# AddDataSink() (video)
 *             -# AddDataSink() (audio)
 *             -# Prepare()
 *             -# SetPlaybackRange()
 *             -# Start()
 *             -# Play 30 sec
 *             -# Stop()
 *             -# RemoveDataSink() (video)
 *             -# RemoveDataSink() (audio)
 *             -# Reset()
 *             -# RemoveDataSource()
 *             -# DeletePlayer()
 *
 */
class pvplayer_async_test_genericopensetplaybackrangestartplaystoptest : public pvplayer_async_test_base
{
    public:
        pvplayer_async_test_genericopensetplaybackrangestartplaystoptest(PVPlayerAsyncTestParam aTestParam,
                PVMFFormatType aVideoSinkFormat,
                PVMFFormatType aAudioSinkFormat,
                uint32 aTestID,
                bool aWaitForEOS,
                bool aCloaking,
                bool aCancelDuringPrepare)
                : pvplayer_async_test_base(aTestParam)
                , iPlayer(NULL)
                , iDataSource(NULL)
                , iDataSinkVideo(NULL)
                , iDataSinkAudio(NULL)
                , iIONodeVideo(NULL)
                , iIONodeAudio(NULL)
                , iMIOFileOutVideo(NULL)
                , iMIOFileOutAudio(NULL)
                , iCurrentCmdId(0)
                , iCancelAllCmdId(0)
                , iSeekAfterEOSEnable(false)
                , iSessionDuration(0)
                , bcloaking(aCloaking)
                , oLiveSession(false)
                , iProtocolRollOver(false)
                , iProtocolRollOverWithUnknownURLType(false)
                , iPlayListURL(false)
                , iStreamDataSource(NULL)
                , iSourceContextData(NULL)
                , iDownloadContextDataPVX(NULL)
        {
            iVideoSinkFormatType = aVideoSinkFormat;
            iAudioSinkFormatType = aAudioSinkFormat;
            oWaitForEOS = aWaitForEOS;
            oCancelDuringPrepare = aCancelDuringPrepare;
            iTestID = aTestID;
            iNumPlay = 0;
            iTargetNumPlay = 1;
            iNumBufferingStart = iNumBufferingComplete = iNumUnderflow = iNumDataReady = iNumEOS = 0;
            iDownloadOnly = iDownloadThenPlay = false;
            iSeekDone = false;
            iInitDone = false;
        }

        ~pvplayer_async_test_genericopensetplaybackrangestartplaystoptest() {}

        void StartTest();
        void Run();

        void CommandCompleted(const PVCmdResponse& aResponse);
        void HandleErrorEvent(const PVAsyncErrorEvent& aEvent);
        void HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent);

        /*enum PVTestState
        {
        	STATE_CREATE,
            STATE_QUERYINTERFACE,
        	STATE_ADDDATASOURCE,
        	STATE_CONFIGPARAMS,
        	STATE_INIT,
        	STATE_GETMETADATAKEYLIST,
        	STATE_GETMETADATAVALUELIST,
        	STATE_ADDDATASINK_VIDEO,
        	STATE_ADDDATASINK_AUDIO,
        	STATE_PREPARE,
        	STATE_CANCELALL,
        	STATE_WAIT_FOR_CANCELALL,
        	STATE_START,
        	STATE_SETPLAYBACKRANGE,
        	STATE_PAUSE,
        	STATE_RESUME,
        	STATE_EOSNOTREACHED,
        	STATE_STOP,
        	STATE_REMOVEDATASINK_VIDEO,
        	STATE_REMOVEDATASINK_AUDIO,
        	STATE_RESET,
        	STATE_REMOVEDATASOURCE,
        	STATE_CLEANUPANDCOMPLETE
        };*/

        PVTestState iState;

        PVPlayerInterface* iPlayer;
        PVPlayerDataSourceURL* iDataSource;
        PVPlayerDataSink* iDataSinkVideo;
        PVPlayerDataSink* iDataSinkAudio;
        PVMFNodeInterface* iIONodeVideo;
        PVMFNodeInterface* iIONodeAudio;
        PvmiMIOControl* iMIOFileOutVideo;
        PvmiMIOControl* iMIOFileOutAudio;
        PVCommandId iCurrentCmdId;
        PVCommandId iCancelAllCmdId;

        void setMultiplePlayMode(uint32 aNum)
        {
            iMultiplePlay = true;
            iTargetNumPlay = aNum;
        }

        void setProtocolRollOverMode()
        {
            iProtocolRollOver = true;
        }

        void setProtocolRollOverModeWithUnknownURL()
        {
            iProtocolRollOverWithUnknownURLType = true;
        }

        void setPlayListMode()
        {
            iPlayListURL = true;
        }

        void setSeekAfterEOSMode()
        {
            iSeekAfterEOSEnable = true;
        }

    private:
        void HandleSocketNodeErrors(int32 aErr);
        void HandleRTSPNodeErrors(int32 aErr);
        void HandleStreamingManagerNodeErrors(int32 aErr);
        void HandleJitterBufferNodeErrors(int32 aErr);
        void HandleMediaLayerNodeErrors(int32 aErr);
        void HandleProtocolEngineNodeErrors(int32 aErr);

        void PrintMetadataInfo();

        PVMFFormatType iVideoSinkFormatType;
        PVMFFormatType iAudioSinkFormatType;
        OSCL_wHeapString<OsclMemAllocator> wFileName;
        oscl_wchar output[512];
        bool oCancelDuringPrepare;
        bool oWaitForEOS;
        uint32 iTestID;
        bool iSeekAfterEOSEnable;

        PVPMetadataList iMetadataKeyList;
        Oscl_Vector<PvmiKvp, OsclMemAllocator> iMetadataValueList;
        int32 iNumValues;

        uint32 iSessionDuration;
        bool bcloaking;
        bool oLiveSession;

        bool iMultiplePlay;
        uint32 iNumPlay;
        uint32 iTargetNumPlay;
        bool iSeekDone;
        bool iInitDone;

        bool iProtocolRollOver;
        bool iProtocolRollOverWithUnknownURLType;
        bool iPlayListURL;

        PvmiCapabilityAndConfig* iPlayerCapConfigIF;
        PvmiKvp* iErrorKVP;
        PvmiKvp iKVPSetAsync;
        OSCL_StackString<128> iKeyStringSetAsync;

        PVMFStreamingDataSource* iStreamDataSource;

        PVMFSourceContextData* iSourceContextData;

        //FTDL
        void CreateDownloadDataSource();
        uint8 iPVXFileBuf[4096];
        PVMFDownloadDataSourcePVX* iDownloadContextDataPVX;
        PVMFDownloadDataSourceHTTP* iDownloadContextDataHTTP;
        int32 iDownloadMaxfilesize;
#if RUN_FASTTRACK_TESTCASES
        CPVXInfo iDownloadPvxInfo;
#endif
        OSCL_wHeapString<OsclMemAllocator> iDownloadURL;
        OSCL_wHeapString<OsclMemAllocator> iDownloadFilename;
        OSCL_HeapString<OsclMemAllocator> iDownloadProxy;
        OSCL_wHeapString<OsclMemAllocator> iDownloadConfigFilename;
        int32 iNumBufferingStart, iNumBufferingComplete, iNumUnderflow, iNumDataReady, iNumEOS;
        bool iDownloadOnly, iDownloadThenPlay;
        bool iContentTooLarge;
};

/*!
 *  A test case to test seek to end of clip when playing an local/PDL/rtsp url
 *  - Data Source: Specified by user of test case
 *  - Data Sink(s): Video[FileOutputNode-test_player_genericopenplayrepositiontoend_video.dat]\n
 *                  Audio[FileOutputNode-test_player_genericopenplayrepositiontoend_audio.dat]
 *  - Sequence:
 *             -# CreatePlayer()
 *             -# AddDataSource()
 *             -# Init()
 *             -# AddDataSink() (video)
 *             -# AddDataSink() (audio)
 *             -# Prepare()
 *             -# Start()
 *             -# Play 10 sec
 *             -# SetPlaybackRange() to the end of clip
 *             -# Play until EOS
 *             -# Stop()
 *             -# RemoveDataSink() (video)
 *             -# RemoveDataSink() (audio)
 *             -# Reset()
 *             -# RemoveDataSource()
 *             -# DeletePlayer()
 *
 */
class pvplayer_async_test_genericopenplayrepositiontoendtest : public pvplayer_async_test_base
{
    public:
        pvplayer_async_test_genericopenplayrepositiontoendtest(PVPlayerAsyncTestParam aTestParam,
                PVMFFormatType aVideoSinkFormat,
                PVMFFormatType aAudioSinkFormat,
                uint32 aTestID,
                bool aWaitForEOS,
                bool aCloaking,
                bool aCancelDuringPrepare)
                : pvplayer_async_test_base(aTestParam)
                , iPlayer(NULL)
                , iDataSource(NULL)
                , iDataSinkVideo(NULL)
                , iDataSinkAudio(NULL)
                , iIONodeVideo(NULL)
                , iIONodeAudio(NULL)
                , iMIOFileOutVideo(NULL)
                , iMIOFileOutAudio(NULL)
                , iCurrentCmdId(0)
                , iCancelAllCmdId(0)
                , iSeekAfterEOSEnable(false)
                , iSessionDuration(0)
                , bcloaking(aCloaking)
                , oLiveSession(false)
                , iProtocolRollOver(false)
                , iProtocolRollOverWithUnknownURLType(false)
                , iPlayListURL(false)
                , iStreamDataSource(NULL)
                , iSourceContextData(NULL)
                , iDownloadContextDataPVX(NULL)
        {
            iVideoSinkFormatType = aVideoSinkFormat;
            iAudioSinkFormatType = aAudioSinkFormat;
            oWaitForEOS = aWaitForEOS;
            oCancelDuringPrepare = aCancelDuringPrepare;
            iTestID = aTestID;
            iNumPlay = 0;
            iTargetNumPlay = 1;
            iNumBufferingStart = iNumBufferingComplete = iNumUnderflow = iNumDataReady = iNumEOS = 0;
            iDownloadOnly = iDownloadThenPlay = false;
            iPlayStarted = false;
            iSeekDone = false;
        }

        ~pvplayer_async_test_genericopenplayrepositiontoendtest() {}

        void StartTest();
        void Run();

        void CommandCompleted(const PVCmdResponse& aResponse);
        void HandleErrorEvent(const PVAsyncErrorEvent& aEvent);
        void HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent);

        /*enum PVTestState
        {
        	STATE_CREATE,
            STATE_QUERYINTERFACE,
        	STATE_ADDDATASOURCE,
        	STATE_CONFIGPARAMS,
        	STATE_INIT,
        	STATE_GETMETADATAKEYLIST,
        	STATE_GETMETADATAVALUELIST,
        	STATE_ADDDATASINK_VIDEO,
        	STATE_ADDDATASINK_AUDIO,
        	STATE_PREPARE,
        	STATE_CANCELALL,
        	STATE_WAIT_FOR_CANCELALL,
        	STATE_START,
        	STATE_SETPLAYBACKRANGE,
        	STATE_PAUSE,
        	STATE_RESUME,
        	STATE_EOSNOTREACHED,
        	STATE_STOP,
        	STATE_REMOVEDATASINK_VIDEO,
        	STATE_REMOVEDATASINK_AUDIO,
        	STATE_RESET,
        	STATE_REMOVEDATASOURCE,
        	STATE_CLEANUPANDCOMPLETE
        };*/

        PVTestState iState;

        PVPlayerInterface* iPlayer;
        PVPlayerDataSourceURL* iDataSource;
        PVPlayerDataSink* iDataSinkVideo;
        PVPlayerDataSink* iDataSinkAudio;
        PVMFNodeInterface* iIONodeVideo;
        PVMFNodeInterface* iIONodeAudio;
        PvmiMIOControl* iMIOFileOutVideo;
        PvmiMIOControl* iMIOFileOutAudio;
        PVCommandId iCurrentCmdId;
        PVCommandId iCancelAllCmdId;

        void setMultiplePlayMode(uint32 aNum)
        {
            iMultiplePlay = true;
            iTargetNumPlay = aNum;
        }

        void setProtocolRollOverMode()
        {
            iProtocolRollOver = true;
        }

        void setProtocolRollOverModeWithUnknownURL()
        {
            iProtocolRollOverWithUnknownURLType = true;
        }

        void setPlayListMode()
        {
            iPlayListURL = true;
        }

        void setSeekAfterEOSMode()
        {
            iSeekAfterEOSEnable = true;
        }

    private:
        void HandleSocketNodeErrors(int32 aErr);
        void HandleRTSPNodeErrors(int32 aErr);
        void HandleStreamingManagerNodeErrors(int32 aErr);
        void HandleJitterBufferNodeErrors(int32 aErr);
        void HandleMediaLayerNodeErrors(int32 aErr);
        void HandleProtocolEngineNodeErrors(int32 aErr);

        void PrintMetadataInfo();

        PVMFFormatType iVideoSinkFormatType;
        PVMFFormatType iAudioSinkFormatType;
        OSCL_wHeapString<OsclMemAllocator> wFileName;
        oscl_wchar output[512];
        bool oCancelDuringPrepare;
        bool oWaitForEOS;
        uint32 iTestID;
        bool iSeekAfterEOSEnable;

        PVPMetadataList iMetadataKeyList;
        Oscl_Vector<PvmiKvp, OsclMemAllocator> iMetadataValueList;
        int32 iNumValues;

        uint32 iSessionDuration;
        bool bcloaking;
        bool oLiveSession;

        bool iMultiplePlay;
        uint32 iNumPlay;
        uint32 iTargetNumPlay;
        bool iPlayStarted;
        bool iSeekDone;

        bool iProtocolRollOver;
        bool iProtocolRollOverWithUnknownURLType;
        bool iPlayListURL;

        PvmiCapabilityAndConfig* iPlayerCapConfigIF;
        PvmiKvp* iErrorKVP;
        PvmiKvp iKVPSetAsync;
        OSCL_StackString<128> iKeyStringSetAsync;

        PVMFStreamingDataSource* iStreamDataSource;

        PVMFSourceContextData* iSourceContextData;

        //FTDL
        void CreateDownloadDataSource();
        uint8 iPVXFileBuf[4096];
        PVMFDownloadDataSourcePVX* iDownloadContextDataPVX;
        PVMFDownloadDataSourceHTTP* iDownloadContextDataHTTP;
        int32 iDownloadMaxfilesize;
#if RUN_FASTTRACK_TESTCASES
        CPVXInfo iDownloadPvxInfo;
#endif
        OSCL_wHeapString<OsclMemAllocator> iDownloadURL;
        OSCL_wHeapString<OsclMemAllocator> iDownloadFilename;
        OSCL_HeapString<OsclMemAllocator> iDownloadProxy;
        OSCL_wHeapString<OsclMemAllocator> iDownloadConfigFilename;
        int32 iNumBufferingStart, iNumBufferingComplete, iNumUnderflow, iNumDataReady, iNumEOS;
        bool iDownloadOnly, iDownloadThenPlay;
        bool iContentTooLarge;
};

/*!
 *  Test cases to test engine behavior when network is disconnected DURING/RIGHT AFTER various engine command's
 *  execution when playing an PDL/streaming url
 *  - Data Source: Specified by user of test case
 *  - Data Sink(s): Video[FileOutputNode-test_player_genericnetworkdisconnect_video.dat]\n
 *                  Audio[FileOutputNode-test_player_genericnetworkdisconnect_audio.dat]
 *  - Sequence:
 *             -# CreatePlayer()
 *             -# AddDataSource()
 *             -# Init()
 *             -# AddDataSink() (video)
 *             -# AddDataSink() (audio)
 *             -# Prepare()
 *             -# Start()
 *             -# Play 10 sec
 *             -# SetPlaybackRange() if seekEnable flag is made true
 *             -# Pause() if pauseresumeEnable flag is made true
 *             -# WAIT 10 sec
 *             -# Resume()
 *             -# Play until EOS
 *             -# Stop()
 *             -# RemoveDataSink() (video)
 *             -# RemoveDataSink() (audio)
 *             -# Reset()
 *             -# RemoveDataSource()
 *             -# DeletePlayer()
 *
 */
class pvplayer_async_test_genericnetworkdisconnect : public pvplayer_async_test_base
{
    public:
        pvplayer_async_test_genericnetworkdisconnect(PVPlayerAsyncTestParam aTestParam,
                PVMFFormatType aVideoSinkFormat,
                PVMFFormatType aAudioSinkFormat,
                uint32 aTestID,
                bool aPauseResumeEnable,
                bool aSeekEnable,
                bool aWaitForEOS,
                bool aCloaking,
                bool aCancelDuringPrepare,
                int aDisconnectState,
                bool aDisconnectWhileProc)
                : pvplayer_async_test_base(aTestParam)
                , iPlayer(NULL)
                , iDataSource(NULL)
                , iDataSinkVideo(NULL)
                , iDataSinkAudio(NULL)
                , iIONodeVideo(NULL)
                , iIONodeAudio(NULL)
                , iMIOFileOutVideo(NULL)
                , iMIOFileOutAudio(NULL)
                , iCurrentCmdId(0)
                , iCancelAllCmdId(0)
                , iSessionDuration(0)
                , bcloaking(aCloaking)
                , oLiveSession(false)
                , iProtocolRollOver(false)
                , iProtocolRollOverWithUnknownURLType(false)
                , iPlayListURL(false)
                , iSourceContextData(NULL)
                , iStreamDataSource(NULL)
                , iDownloadContextDataPVX(NULL)
        {
            iVideoSinkFormatType = aVideoSinkFormat;
            iAudioSinkFormatType = aAudioSinkFormat;
            oPauseResumeEnable = aPauseResumeEnable;
            oSeekEnable = aSeekEnable;
            oWaitForEOS = aWaitForEOS;
            oCancelDuringPrepare = aCancelDuringPrepare;
            iTestID = aTestID;
            iNumPlay = 0;
            iTargetNumPlay = 1;
            iNumBufferingStart = iNumBufferingComplete = iNumUnderflow = iNumDataReady = iNumEOS = 0;
            iDownloadOnly = iDownloadThenPlay = false;
            iDisconnectState = (PVTestState)aDisconnectState;
            iDisconnectWhileProc = aDisconnectWhileProc;
        }

        ~pvplayer_async_test_genericnetworkdisconnect() {}

        void StartTest();
        void Run();

        void CommandCompleted(const PVCmdResponse& aResponse);
        void HandleErrorEvent(const PVAsyncErrorEvent& aEvent);
        void HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent);


        PVTestState iState;
        PVTestState iDisconnectState;
        bool iDisconnectWhileProc;

        PVPlayerInterface* iPlayer;
        PVPlayerDataSourceURL* iDataSource;
        PVPlayerDataSink* iDataSinkVideo;
        PVPlayerDataSink* iDataSinkAudio;
        PVMFNodeInterface* iIONodeVideo;
        PVMFNodeInterface* iIONodeAudio;
        PvmiMIOControl* iMIOFileOutVideo;
        PvmiMIOControl* iMIOFileOutAudio;
        PVCommandId iCurrentCmdId;
        PVCommandId iCancelAllCmdId;


        void setMultiplePlayMode(uint32 aNum)
        {
            iMultiplePlay = true;
            iTargetNumPlay = aNum;
        }

        void setProtocolRollOverMode()
        {
            iProtocolRollOver = true;
        }

        void setProtocolRollOverModeWithUnknownURL()
        {
            iProtocolRollOverWithUnknownURLType = true;
        }

        void setPlayListMode()
        {
            iPlayListURL = true;
        }

        void disableNetwork()
        {

#if	PVPLAYER_TEST_LINUX_DISABLE_NETWORK
            fprintf(iTestMsgOutputFile, "***Disconnecting Network...\n");
            system("/sbin/ifconfig eth0 down");
#endif

#if	PVPLAYER_TEST_WIN32_DISABLE_NETWORK
            fprintf(iTestMsgOutputFile, "***Disconnecting Network...\n");
            system("ipconfig /release");
#endif

        }

        void enableNetwork()
        {
            char cmd_str[48] = "/sbin/route add default gw ";
            oscl_strcat(cmd_str, DEFAULT_GATEWAY);

#if	PVPLAYER_TEST_LINUX_DISABLE_NETWORK
            fprintf(iTestMsgOutputFile, "***Reconnecting Network...\n");
            system("/sbin/ifconfig eth0 up");
            system(cmd_str);
#endif

#if	PVPLAYER_TEST_WIN32_DISABLE_NETWORK
            fprintf(iTestMsgOutputFile, "***Reconnecting Network...\n");
            system("ipconfig /renew");
#endif

        }

    private:
        void HandleSocketNodeErrors(int32 aErr);
        void HandleRTSPNodeErrors(int32 aErr);
        void HandleStreamingManagerNodeErrors(int32 aErr);
        void HandleJitterBufferNodeErrors(int32 aErr);
        void HandleMediaLayerNodeErrors(int32 aErr);
        void HandleProtocolEngineNodeErrors(int32 aErr);


        void PrintMetadataInfo();

        PVMFFormatType iVideoSinkFormatType;
        PVMFFormatType iAudioSinkFormatType;
        OSCL_wHeapString<OsclMemAllocator> wFileName;
        oscl_wchar output[512];
        bool oPauseResumeEnable;
        bool oSeekEnable;
        bool oCancelDuringPrepare;
        bool oWaitForEOS;
        uint32 iTestID;

        PVPMetadataList iMetadataKeyList;
        Oscl_Vector<PvmiKvp, OsclMemAllocator> iMetadataValueList;
        int32 iNumValues;

        uint32 iSessionDuration;
        bool bcloaking;
        bool oLiveSession;

        bool iMultiplePlay;
        uint32 iNumPlay;
        uint32 iTargetNumPlay;

        bool iProtocolRollOver;
        bool iProtocolRollOverWithUnknownURLType;
        bool iPlayListURL;

        PvmiCapabilityAndConfig* iPlayerCapConfigIF;
        PvmiKvp* iErrorKVP;
        PvmiKvp iKVPSetAsync;
        OSCL_StackString<128> iKeyStringSetAsync;
        PVMFSourceContextData* iSourceContextData;

        PVMFStreamingDataSource* iStreamDataSource;

        //FTDL
        void CreateDownloadDataSource();
        uint8 iPVXFileBuf[4096];
        PVMFDownloadDataSourcePVX* iDownloadContextDataPVX;
        PVMFDownloadDataSourceHTTP* iDownloadContextDataHTTP;
        int32 iDownloadMaxfilesize;
#if RUN_FASTTRACK_TESTCASES
        CPVXInfo iDownloadPvxInfo;
#endif
        OSCL_wHeapString<OsclMemAllocator> iDownloadURL;
        OSCL_wHeapString<OsclMemAllocator> iDownloadFilename;
        OSCL_HeapString<OsclMemAllocator> iDownloadProxy;
        OSCL_wHeapString<OsclMemAllocator> iDownloadConfigFilename;
        int32 iNumBufferingStart, iNumBufferingComplete, iNumUnderflow, iNumDataReady, iNumEOS;
        bool iDownloadOnly, iDownloadThenPlay;
        bool iContentTooLarge;
};

/*!
 *  Test cases to test engine behavior when network is disconnected and then reconnected after
 *  NW_RECONNECT_AFTER_DISCONNECT milli secs, DURING/RIGHT AFTER various engine command's
 *  execution, when playing an PDL/streaming url
 *  - Data Source: Specified by user of test case
 *  - Data Sink(s): Video[FileOutputNode-test_player_genericnetworkdisconnectreconnect_video.dat]\n
 *                  Audio[FileOutputNode-test_player_genericnetworkdisconnectreconnect_audio.dat]
 *  - Sequence:
 *             -# CreatePlayer()
 *             -# AddDataSource()
 *             -# Init()
 *             -# AddDataSink() (video)
 *             -# AddDataSink() (audio)
 *             -# Prepare()
 *             -# Start()
 *             -# Play 10 sec
 *             -# SetPlaybackRange() if seekEnable flag is made true
 *             -# Pause() if pauseresumeEnable flag is made true
 *             -# WAIT 10 sec
 *             -# Resume()
 *             -# Play until EOS
 *             -# Stop()
 *             -# RemoveDataSink() (video)
 *             -# RemoveDataSink() (audio)
 *             -# Reset()
 *             -# RemoveDataSource()
 *             -# DeletePlayer()
 *
 */
class pvplayer_async_test_genericnetworkdisconnectreconnect : public pvplayer_async_test_base
{
    public:
        pvplayer_async_test_genericnetworkdisconnectreconnect(PVPlayerAsyncTestParam aTestParam,
                PVMFFormatType aVideoSinkFormat,
                PVMFFormatType aAudioSinkFormat,
                uint32 aTestID,
                bool aPauseResumeEnable,
                bool aSeekEnable,
                bool aWaitForEOS,
                bool aCloaking,
                bool aCancelDuringPrepare,
                int aDisconnectState,
                bool aDisconnectWhileProc)
                : pvplayer_async_test_base(aTestParam)
                , iPlayer(NULL)
                , iDataSource(NULL)
                , iDataSinkVideo(NULL)
                , iDataSinkAudio(NULL)
                , iIONodeVideo(NULL)
                , iIONodeAudio(NULL)
                , iMIOFileOutVideo(NULL)
                , iMIOFileOutAudio(NULL)
                , iCurrentCmdId(0)
                , iCancelAllCmdId(0)
                , iSessionDuration(0)
                , bcloaking(aCloaking)
                , oLiveSession(false)
                , iProtocolRollOver(false)
                , iProtocolRollOverWithUnknownURLType(false)
                , iPlayListURL(false)
                , iSourceContextData(NULL)
                , iStreamDataSource(NULL)
                , iDownloadContextDataPVX(NULL)
        {
            iVideoSinkFormatType = aVideoSinkFormat;
            iAudioSinkFormatType = aAudioSinkFormat;
            oPauseResumeEnable = aPauseResumeEnable;
            oSeekEnable = aSeekEnable;
            oWaitForEOS = aWaitForEOS;
            oCancelDuringPrepare = aCancelDuringPrepare;
            iTestID = aTestID;
            iNumPlay = 0;
            iTargetNumPlay = 1;
            iNumBufferingStart = iNumBufferingComplete = iNumUnderflow = iNumDataReady = iNumEOS = 0;
            iDownloadOnly = iDownloadThenPlay = false;
            iDisconnectState = (PVTestState)aDisconnectState;
            iDisconnectWhileProc = aDisconnectWhileProc;
            iNextState = STATE_RECONNECT;
            iNextStateActiveTime = 0;
            iNetworkIsDisabled = false;
        }

        ~pvplayer_async_test_genericnetworkdisconnectreconnect() {}

        void StartTest();
        void Run();

        void CommandCompleted(const PVCmdResponse& aResponse);
        void HandleErrorEvent(const PVAsyncErrorEvent& aEvent);
        void HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent);

        PVTestState iState;
        PVTestState iDisconnectState;
        bool iDisconnectWhileProc;
        bool iNetworkIsDisabled;

        PVTestState iNextState;
        uint32 iNextStateActiveTime;

        PVPlayerInterface* iPlayer;
        PVPlayerDataSourceURL* iDataSource;
        PVPlayerDataSink* iDataSinkVideo;
        PVPlayerDataSink* iDataSinkAudio;
        PVMFNodeInterface* iIONodeVideo;
        PVMFNodeInterface* iIONodeAudio;
        PvmiMIOControl* iMIOFileOutVideo;
        PvmiMIOControl* iMIOFileOutAudio;
        PVCommandId iCurrentCmdId;
        PVCommandId iCancelAllCmdId;


        void setMultiplePlayMode(uint32 aNum)
        {
            iMultiplePlay = true;
            iTargetNumPlay = aNum;
        }

        void setProtocolRollOverMode()
        {
            iProtocolRollOver = true;
        }

        void setProtocolRollOverModeWithUnknownURL()
        {
            iProtocolRollOverWithUnknownURLType = true;
        }

        void setPlayListMode()
        {
            iPlayListURL = true;
        }

        void disableNetwork()
        {

#if	PVPLAYER_TEST_LINUX_DISABLE_NETWORK
            fprintf(iTestMsgOutputFile, "***Disconnecting Network...\n");
            system("/sbin/ifconfig eth0 down");
#endif

#if	PVPLAYER_TEST_WIN32_DISABLE_NETWORK
            fprintf(iTestMsgOutputFile, "***Disconnecting Network...\n");
            system("ipconfig /release");
#endif
            iNetworkIsDisabled = true;
        }

        void enableNetwork()
        {
            if (iNetworkIsDisabled)
            {
                char cmd_str[48] = "/sbin/route add default gw ";
                oscl_strcat(cmd_str, DEFAULT_GATEWAY);
#if	PVPLAYER_TEST_LINUX_DISABLE_NETWORK
                fprintf(iTestMsgOutputFile, "***Reconnecting Network...\n");
                system("/sbin/ifconfig eth0 up");
                system(cmd_str);
#endif

#if	PVPLAYER_TEST_WIN32_DISABLE_NETWORK
                fprintf(iTestMsgOutputFile, "***Reconnecting Network...\n");
                system("ipconfig /renew");
#endif
                iNetworkIsDisabled = false;
            }
        }

    private:
        void HandleSocketNodeErrors(int32 aErr);
        void HandleRTSPNodeErrors(int32 aErr);
        void HandleStreamingManagerNodeErrors(int32 aErr);
        void HandleJitterBufferNodeErrors(int32 aErr);
        void HandleMediaLayerNodeErrors(int32 aErr);
        void HandleProtocolEngineNodeErrors(int32 aErr);


        void PrintMetadataInfo();

        PVMFFormatType iVideoSinkFormatType;
        PVMFFormatType iAudioSinkFormatType;
        OSCL_wHeapString<OsclMemAllocator> wFileName;
        oscl_wchar output[512];
        bool oPauseResumeEnable;
        bool oSeekEnable;
        bool oCancelDuringPrepare;
        bool oWaitForEOS;
        uint32 iTestID;

        PVPMetadataList iMetadataKeyList;
        Oscl_Vector<PvmiKvp, OsclMemAllocator> iMetadataValueList;
        int32 iNumValues;

        uint32 iSessionDuration;
        bool bcloaking;
        bool oLiveSession;

        bool iMultiplePlay;
        uint32 iNumPlay;
        uint32 iTargetNumPlay;

        bool iProtocolRollOver;
        bool iProtocolRollOverWithUnknownURLType;
        bool iPlayListURL;

        PvmiCapabilityAndConfig* iPlayerCapConfigIF;
        PvmiKvp* iErrorKVP;
        PvmiKvp iKVPSetAsync;
        OSCL_StackString<128> iKeyStringSetAsync;
        PVMFSourceContextData* iSourceContextData;

        PVMFStreamingDataSource* iStreamDataSource;

        //FTDL
        void CreateDownloadDataSource();
        uint8 iPVXFileBuf[4096];
        PVMFDownloadDataSourcePVX* iDownloadContextDataPVX;
        PVMFDownloadDataSourceHTTP* iDownloadContextDataHTTP;
        int32 iDownloadMaxfilesize;
#if RUN_FASTTRACK_TESTCASES
        CPVXInfo iDownloadPvxInfo;
#endif
        OSCL_wHeapString<OsclMemAllocator> iDownloadURL;
        OSCL_wHeapString<OsclMemAllocator> iDownloadFilename;
        OSCL_HeapString<OsclMemAllocator> iDownloadProxy;
        OSCL_wHeapString<OsclMemAllocator> iDownloadConfigFilename;
        int32 iNumBufferingStart, iNumBufferingComplete, iNumUnderflow, iNumDataReady, iNumEOS;
        bool iDownloadOnly, iDownloadThenPlay;
        bool iContentTooLarge;
};

/*!
 *  Test cases to test engine behavior when network is disconnected, followed by cancelAllCommands() immediately,
 *  DURING/RIGHT AFTER various engine commands processing
 *  - Data Source: Specified by user of test case
 *  - Data Sink(s): Video[FileOutputNode-test_player_genericcancelall_video.dat]\n
 *                  Audio[FileOutputNode-test_player_genericcancelall_audio.dat]
 *  - Sequence:
 *             -# CreatePlayer()
 *             -# CancelAllCommands() is called after/in one of the following states based on the test case
 *             -# AddDataSource()
 *             -# Init()
 *             -# AddDataSink() (video)
 *             -# AddDataSink() (audio)
 *             -# Prepare()
 *             -# Start()
 *             -# Play 10 sec
 *             -# SetPlaybackRange() if seekEnable flag is made true
 *             -# Pause() if pauseresumeEnable flag is made true
 *             -# WAIT 10 sec
 *             -# Resume()
 *             -# Play until EOS
 *             -# Stop()
 *             -# RemoveDataSink() (video)
 *             -# RemoveDataSink() (audio)
 *             -# Reset()
 *             -# RemoveDataSource()
 *             -# DeletePlayer()
 *
 */
class pvplayer_async_test_genericcancelallnetworkdisconnect : public pvplayer_async_test_base
{
    public:
        pvplayer_async_test_genericcancelallnetworkdisconnect(PVPlayerAsyncTestParam aTestParam,
                PVMFFormatType aVideoSinkFormat,
                PVMFFormatType aAudioSinkFormat,
                uint32 aTestID,
                bool aPauseResumeEnable,
                bool aSeekEnable,
                bool aWaitForEOS,
                bool aCloaking,
                bool aCancelDuringPrepare,
                int lastState,
                bool aCancelAllWhileProc)
                : pvplayer_async_test_base(aTestParam)
                , iPlayer(NULL)
                , iDataSource(NULL)
                , iDataSinkVideo(NULL)
                , iDataSinkAudio(NULL)
                , iIONodeVideo(NULL)
                , iIONodeAudio(NULL)
                , iMIOFileOutVideo(NULL)
                , iMIOFileOutAudio(NULL)
                , iCurrentCmdId(0)
                , iCancelAllCmdId(0)
                , iSessionDuration(0)
                , bcloaking(aCloaking)
                , oLiveSession(false)
                , iProtocolRollOver(false)
                , iProtocolRollOverWithUnknownURLType(false)
                , iPlayListURL(false)
                , iSourceContextData(NULL)
                , iStreamDataSource(NULL)
                , iDownloadContextDataPVX(NULL)
        {
            iVideoSinkFormatType = aVideoSinkFormat;
            iAudioSinkFormatType = aAudioSinkFormat;
            oPauseResumeEnable = aPauseResumeEnable;
            oSeekEnable = aSeekEnable;
            oWaitForEOS = aWaitForEOS;
            oCancelDuringPrepare = aCancelDuringPrepare;
            iTestID = aTestID;
            iNumPlay = 0;
            iTargetNumPlay = 1;
            iNumBufferingStart = iNumBufferingComplete = iNumUnderflow = iNumDataReady = iNumEOS = 0;
            iDownloadOnly = iDownloadThenPlay = false;
            iEndState = (PVTestState)lastState;
            iCancelAllWhileProc = aCancelAllWhileProc;
            iNetworkIsDisabled = false;
        }

        ~pvplayer_async_test_genericcancelallnetworkdisconnect() {}

        void StartTest();
        void Run();

        void CommandCompleted(const PVCmdResponse& aResponse);
        void HandleErrorEvent(const PVAsyncErrorEvent& aEvent);
        void HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent);


        PVTestState iState;
        PVTestState iEndState;
        bool iCancelAllWhileProc;
        bool iNetworkIsDisabled;

        PVPlayerInterface* iPlayer;
        PVPlayerDataSourceURL* iDataSource;
        PVPlayerDataSink* iDataSinkVideo;
        PVPlayerDataSink* iDataSinkAudio;
        PVMFNodeInterface* iIONodeVideo;
        PVMFNodeInterface* iIONodeAudio;
        PvmiMIOControl* iMIOFileOutVideo;
        PvmiMIOControl* iMIOFileOutAudio;
        PVCommandId iCurrentCmdId;
        PVCommandId iCancelAllCmdId;


        void setMultiplePlayMode(uint32 aNum)
        {
            iMultiplePlay = true;
            iTargetNumPlay = aNum;
        }

        void setProtocolRollOverMode()
        {
            iProtocolRollOver = true;
        }

        void setProtocolRollOverModeWithUnknownURL()
        {
            iProtocolRollOverWithUnknownURLType = true;
        }

        void setPlayListMode()
        {
            iPlayListURL = true;
        }

        void disableNetwork()
        {

#if	PVPLAYER_TEST_LINUX_DISABLE_NETWORK
            fprintf(iTestMsgOutputFile, "***Disconnecting Network...\n");
            system("/sbin/ifconfig eth0 down");
#endif

#if	PVPLAYER_TEST_WIN32_DISABLE_NETWORK
            fprintf(iTestMsgOutputFile, "***Disconnecting Network...\n");
            system("ipconfig /release");
#endif
            iNetworkIsDisabled = true;

        }

        void enableNetwork()
        {
            if (iNetworkIsDisabled)
            {
                char cmd_str[48] = "/sbin/route add default gw ";
                oscl_strcat(cmd_str, DEFAULT_GATEWAY);

#if	PVPLAYER_TEST_LINUX_DISABLE_NETWORK
                fprintf(iTestMsgOutputFile, "***Reconnecting Network...\n");
                system("/sbin/ifconfig eth0 up");
                system(cmd_str);
#endif

#if	PVPLAYER_TEST_WIN32_DISABLE_NETWORK
                fprintf(iTestMsgOutputFile, "***Reconnecting Network...\n");
                system("ipconfig /renew");
#endif
                iNetworkIsDisabled = false;
            }
        }

    private:
        void HandleSocketNodeErrors(int32 aErr);
        void HandleRTSPNodeErrors(int32 aErr);
        void HandleStreamingManagerNodeErrors(int32 aErr);
        void HandleJitterBufferNodeErrors(int32 aErr);
        void HandleMediaLayerNodeErrors(int32 aErr);
        void HandleProtocolEngineNodeErrors(int32 aErr);


        void PrintMetadataInfo();

        PVMFFormatType iVideoSinkFormatType;
        PVMFFormatType iAudioSinkFormatType;
        OSCL_wHeapString<OsclMemAllocator> wFileName;
        oscl_wchar output[512];
        bool oPauseResumeEnable;
        bool oSeekEnable;
        bool oCancelDuringPrepare;
        bool oWaitForEOS;
        uint32 iTestID;

        PVPMetadataList iMetadataKeyList;
        Oscl_Vector<PvmiKvp, OsclMemAllocator> iMetadataValueList;
        int32 iNumValues;

        uint32 iSessionDuration;
        bool bcloaking;
        bool oLiveSession;

        bool iMultiplePlay;
        uint32 iNumPlay;
        uint32 iTargetNumPlay;

        bool iProtocolRollOver;
        bool iProtocolRollOverWithUnknownURLType;
        bool iPlayListURL;

        PvmiCapabilityAndConfig* iPlayerCapConfigIF;
        PvmiKvp* iErrorKVP;
        PvmiKvp iKVPSetAsync;
        OSCL_StackString<128> iKeyStringSetAsync;
        PVMFSourceContextData* iSourceContextData;

        PVMFStreamingDataSource* iStreamDataSource;

        //FTDL
        void CreateDownloadDataSource();
        uint8 iPVXFileBuf[4096];
        PVMFDownloadDataSourcePVX* iDownloadContextDataPVX;
        PVMFDownloadDataSourceHTTP* iDownloadContextDataHTTP;
        int32 iDownloadMaxfilesize;
#if RUN_FASTTRACK_TESTCASES
        CPVXInfo iDownloadPvxInfo;
#endif
        OSCL_wHeapString<OsclMemAllocator> iDownloadURL;
        OSCL_wHeapString<OsclMemAllocator> iDownloadFilename;
        OSCL_HeapString<OsclMemAllocator> iDownloadProxy;
        OSCL_wHeapString<OsclMemAllocator> iDownloadConfigFilename;
        int32 iNumBufferingStart, iNumBufferingComplete, iNumUnderflow, iNumDataReady, iNumEOS;
        bool iDownloadOnly, iDownloadThenPlay;
        bool iContentTooLarge;
};

/*!
 *  A test case to test Play -> pause -> seek -> Resume -> network Disconnection -> CancelAllCmds
 *  when playing an local/PDL/rtsp url
 *  - Data Source: Specified by user of test case
 *  - Data Sink(s): Video[FileOutputNode-test_player_genericplaypauserepositionresumenwdisconnect_video.dat]\n
 *                  Audio[FileOutputNode-test_player_genericplaypauserepositionresumenwdisconnect_audio.dat]
 *  - Sequence:
 *             -# CreatePlayer()
 *             -# AddDataSource()
 *             -# Init()
 *             -# AddDataSink() (video)
 *             -# AddDataSink() (audio)
 *             -# Prepare()
 *             -# Start()
 *             -# Play 10 sec
 *             -# Pause()
 *             -# WAIT 5 sec
 *             -# SetPlaybackRange()
 *             -# WAIT 5 sec
 *             -# Resume()
 *			   -# Network disconnection followed by CancelAllCmds()
 *             -# RemoveDataSink() (video)
 *             -# RemoveDataSink() (audio)
 *             -# Reset()
 *             -# RemoveDataSource()
 *             -# DeletePlayer()
 *
 */
class pvplayer_async_test_genericplaypauserepositionresumenwdisconnectcancelalltest : public pvplayer_async_test_base
{
    public:
        pvplayer_async_test_genericplaypauserepositionresumenwdisconnectcancelalltest(PVPlayerAsyncTestParam aTestParam,
                PVMFFormatType aVideoSinkFormat,
                PVMFFormatType aAudioSinkFormat,
                uint32 aTestID,
                bool aWaitForEOS,
                bool aCloaking,
                bool aCancelDuringPrepare)
                : pvplayer_async_test_base(aTestParam)
                , iPlayer(NULL)
                , iDataSource(NULL)
                , iDataSinkVideo(NULL)
                , iDataSinkAudio(NULL)
                , iIONodeVideo(NULL)
                , iIONodeAudio(NULL)
                , iMIOFileOutVideo(NULL)
                , iMIOFileOutAudio(NULL)
                , iCurrentCmdId(0)
                , iCancelAllCmdId(0)
                , iSeekAfterEOSEnable(false)
                , iSessionDuration(0)
                , bcloaking(aCloaking)
                , oLiveSession(false)
                , iProtocolRollOver(false)
                , iProtocolRollOverWithUnknownURLType(false)
                , iPlayListURL(false)
                , iStreamDataSource(NULL)
                , iSourceContextData(NULL)
                , iDownloadContextDataPVX(NULL)
        {
            iVideoSinkFormatType = aVideoSinkFormat;
            iAudioSinkFormatType = aAudioSinkFormat;
            oWaitForEOS = aWaitForEOS;
            oCancelDuringPrepare = aCancelDuringPrepare;
            iTestID = aTestID;
            iNumPlay = 0;
            iTargetNumPlay = 1;
            iNumBufferingStart = iNumBufferingComplete = iNumUnderflow = iNumDataReady = iNumEOS = 0;
            iDownloadOnly = iDownloadThenPlay = false;
            iPauseDone = false;
            iSeekDone = false;
        }

        ~pvplayer_async_test_genericplaypauserepositionresumenwdisconnectcancelalltest() {}

        void StartTest();
        void Run();

        void CommandCompleted(const PVCmdResponse& aResponse);
        void HandleErrorEvent(const PVAsyncErrorEvent& aEvent);
        void HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent);

        /*enum PVTestState
        {
        	STATE_CREATE,
            STATE_QUERYINTERFACE,
        	STATE_ADDDATASOURCE,
        	STATE_CONFIGPARAMS,
        	STATE_INIT,
        	STATE_GETMETADATAKEYLIST,
        	STATE_GETMETADATAVALUELIST,
        	STATE_ADDDATASINK_VIDEO,
        	STATE_ADDDATASINK_AUDIO,
        	STATE_PREPARE,
        	STATE_WAIT_FOR_DATAREADY,
        	STATE_WAIT_FOR_BUFFCOMPLETE,
        	STATE_CANCELALL,
        	STATE_WAIT_FOR_CANCELALL,
        	STATE_START,
        	STATE_SETPLAYBACKRANGE,
        	STATE_PAUSE,
        	STATE_RESUME,
        	STATE_EOSNOTREACHED,
        	STATE_STOP,
        	STATE_REMOVEDATASINK_VIDEO,
        	STATE_REMOVEDATASINK_AUDIO,
        	STATE_RESET,
        	STATE_REMOVEDATASOURCE,
        	STATE_CLEANUPANDCOMPLETE
        };*/

        PVTestState iState;

        PVPlayerInterface* iPlayer;
        PVPlayerDataSourceURL* iDataSource;
        PVPlayerDataSink* iDataSinkVideo;
        PVPlayerDataSink* iDataSinkAudio;
        PVMFNodeInterface* iIONodeVideo;
        PVMFNodeInterface* iIONodeAudio;
        PvmiMIOControl* iMIOFileOutVideo;
        PvmiMIOControl* iMIOFileOutAudio;
        PVCommandId iCurrentCmdId;
        PVCommandId iCancelAllCmdId;

        void setMultiplePlayMode(uint32 aNum)
        {
            iMultiplePlay = true;
            iTargetNumPlay = aNum;
        }

        void setProtocolRollOverMode()
        {
            iProtocolRollOver = true;
        }

        void setProtocolRollOverModeWithUnknownURL()
        {
            iProtocolRollOverWithUnknownURLType = true;
        }

        void setPlayListMode()
        {
            iPlayListURL = true;
        }

        void setSeekAfterEOSMode()
        {
            iSeekAfterEOSEnable = true;
        }

        void disableNetwork()
        {

#if	PVPLAYER_TEST_LINUX_DISABLE_NETWORK
            fprintf(iTestMsgOutputFile, "***Disconnecting Network...\n");
            system("/sbin/ifconfig eth0 down");
#endif

#if	PVPLAYER_TEST_WIN32_DISABLE_NETWORK
            fprintf(iTestMsgOutputFile, "***Disconnecting Network...\n");
            system("ipconfig /release");
#endif

        }

        void enableNetwork()
        {
            char cmd_str[48] = "/sbin/route add default gw ";
            oscl_strcat(cmd_str, DEFAULT_GATEWAY);

#if	PVPLAYER_TEST_LINUX_DISABLE_NETWORK
            fprintf(iTestMsgOutputFile, "***Reconnecting Network...\n");
            system("/sbin/ifconfig eth0 up");
            system(cmd_str);
#endif

#if	PVPLAYER_TEST_WIN32_DISABLE_NETWORK
            fprintf(iTestMsgOutputFile, "***Reconnecting Network...\n");
            system("ipconfig /renew");
#endif

        }

    private:
        void HandleSocketNodeErrors(int32 aErr);
        void HandleRTSPNodeErrors(int32 aErr);
        void HandleStreamingManagerNodeErrors(int32 aErr);
        void HandleJitterBufferNodeErrors(int32 aErr);
        void HandleMediaLayerNodeErrors(int32 aErr);
        void HandleProtocolEngineNodeErrors(int32 aErr);

        void PrintMetadataInfo();

        PVMFFormatType iVideoSinkFormatType;
        PVMFFormatType iAudioSinkFormatType;
        OSCL_wHeapString<OsclMemAllocator> wFileName;
        oscl_wchar output[512];
        bool oCancelDuringPrepare;
        bool oWaitForEOS;
        uint32 iTestID;
        bool iSeekAfterEOSEnable;

        PVPMetadataList iMetadataKeyList;
        Oscl_Vector<PvmiKvp, OsclMemAllocator> iMetadataValueList;
        int32 iNumValues;

        uint32 iSessionDuration;
        bool bcloaking;
        bool oLiveSession;

        bool iMultiplePlay;
        uint32 iNumPlay;
        uint32 iTargetNumPlay;
        bool iPauseDone;
        bool iSeekDone;

        bool iProtocolRollOver;
        bool iProtocolRollOverWithUnknownURLType;
        bool iPlayListURL;

        PvmiCapabilityAndConfig* iPlayerCapConfigIF;
        PvmiKvp* iErrorKVP;
        PvmiKvp iKVPSetAsync;
        OSCL_StackString<128> iKeyStringSetAsync;

        PVMFStreamingDataSource* iStreamDataSource;

        PVMFSourceContextData* iSourceContextData;

        //FTDL
        void CreateDownloadDataSource();
        uint8 iPVXFileBuf[4096];
        PVMFDownloadDataSourcePVX* iDownloadContextDataPVX;
        PVMFDownloadDataSourceHTTP* iDownloadContextDataHTTP;
        int32 iDownloadMaxfilesize;
#if RUN_FASTTRACK_TESTCASES
        CPVXInfo iDownloadPvxInfo;
#endif
        OSCL_wHeapString<OsclMemAllocator> iDownloadURL;
        OSCL_wHeapString<OsclMemAllocator> iDownloadFilename;
        OSCL_HeapString<OsclMemAllocator> iDownloadProxy;
        OSCL_wHeapString<OsclMemAllocator> iDownloadConfigFilename;
        int32 iNumBufferingStart, iNumBufferingComplete, iNumUnderflow, iNumDataReady, iNumEOS;
        bool iDownloadOnly, iDownloadThenPlay;
        bool iContentTooLarge;
};

/*!
 *  A test case to verify PAN-13 fix that is to check if PVMFErrCorrupt is
 *  notified after Init then calling AdddataSource and init again should notify
 *  same error PVMFErrCorrupt instead of PVMFErrBusy; when playing an local/PDL/rtsp url
 *  - Data Source: Specified by user of test case
 *  - Data Sink(s): Video[FileOutputNode-test_player_genericpvmferrorcorruptrenotified_video.dat]\n
 *                  Audio[FileOutputNode-test_player_genericpvmferrorcorruptrenotified_audio.dat]
 *  - Sequence:
 *             -# CreatePlayer()
 *             -# AddDataSource()
 *             -# Init() PVMFErrCorrupt is notified in callback of init
 *             -# RemoveDataSource()
 *             -# AddDataSource()
 *             -# Init() PVMFErrCorrupt is notified in callback of init
 *             -# RemoveDataSource()
 *             -# DeletePlayer()
 *
 */
class pvplayer_async_test_genericpvmferrorcorruptrenotified : public pvplayer_async_test_base
{
    public:
        pvplayer_async_test_genericpvmferrorcorruptrenotified(PVPlayerAsyncTestParam aTestParam,
                PVMFFormatType aVideoSinkFormat,
                PVMFFormatType aAudioSinkFormat,
                uint32 aTestID,
                bool aCloaking)
                : pvplayer_async_test_base(aTestParam)
                , iPlayer(NULL)
                , iDataSource(NULL)
                , iDataSinkVideo(NULL)
                , iDataSinkAudio(NULL)
                , iIONodeVideo(NULL)
                , iIONodeAudio(NULL)
                , iMIOFileOutVideo(NULL)
                , iMIOFileOutAudio(NULL)
                , iCurrentCmdId(0)
                , iCancelAllCmdId(0)
                , iSessionDuration(0)
                , bcloaking(aCloaking)
                , oLiveSession(false)
                , iProtocolRollOver(false)
                , iProtocolRollOverWithUnknownURLType(false)
                , iPlayListURL(false)
                , iStreamDataSource(NULL)
                , iSourceContextData(NULL)
                , iDownloadContextDataPVX(NULL)
        {
            iVideoSinkFormatType = aVideoSinkFormat;
            iAudioSinkFormatType = aAudioSinkFormat;
            iTestID = aTestID;
            iNumPlay = 0;
            iTargetNumPlay = 2;// 2 times PVMFErrCorrupt is required
            iNumBufferingStart = iNumBufferingComplete = iNumUnderflow = iNumDataReady = iNumEOS = 0;
            iDownloadOnly = iDownloadThenPlay = false;
            iOverflow = false;
        }

        ~pvplayer_async_test_genericpvmferrorcorruptrenotified() {}

        void StartTest();
        void Run();

        void CommandCompleted(const PVCmdResponse& aResponse);
        void HandleErrorEvent(const PVAsyncErrorEvent& aEvent);
        void HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent);

        /*enum PVTestState
        {
        	STATE_CREATE,
            STATE_QUERYINTERFACE,
        	STATE_ADDDATASOURCE,
        	STATE_CONFIGPARAMS,
        	STATE_INIT,
        	STATE_GETMETADATAKEYLIST,
        	STATE_GETMETADATAVALUELIST,
        	STATE_ADDDATASINK_VIDEO,
        	STATE_ADDDATASINK_AUDIO,
        	STATE_PREPARE,
        	STATE_CANCELALL,
        	STATE_WAIT_FOR_CANCELALL,
        	STATE_START,
        	STATE_SETPLAYBACKRANGE,
        	STATE_PAUSE,
        	STATE_RESUME,
        	STATE_EOSNOTREACHED,
        	STATE_STOP,
        	STATE_REMOVEDATASINK_VIDEO,
        	STATE_REMOVEDATASINK_AUDIO,
        	STATE_RESET,
        	STATE_REMOVEDATASOURCE,
        	STATE_CLEANUPANDCOMPLETE
        };*/

        PVTestState iState;

        PVPlayerInterface* iPlayer;
        PVPlayerDataSourceURL* iDataSource;
        PVPlayerDataSink* iDataSinkVideo;
        PVPlayerDataSink* iDataSinkAudio;
        PVMFNodeInterface* iIONodeVideo;
        PVMFNodeInterface* iIONodeAudio;
        PvmiMIOControl* iMIOFileOutVideo;
        PvmiMIOControl* iMIOFileOutAudio;
        PVCommandId iCurrentCmdId;
        PVCommandId iCancelAllCmdId;

        void setMultiplePlayMode(uint32 aNum)
        {
            iMultiplePlay = true;
            iTargetNumPlay = aNum;
        }

        void setProtocolRollOverMode()
        {
            iProtocolRollOver = true;
        }

        void setProtocolRollOverModeWithUnknownURL()
        {
            iProtocolRollOverWithUnknownURLType = true;
        }

        void setPlayListMode()
        {
            iPlayListURL = true;
        }

    private:
        void HandleSocketNodeErrors(int32 aErr);
        void HandleRTSPNodeErrors(int32 aErr);
        void HandleStreamingManagerNodeErrors(int32 aErr);
        void HandleJitterBufferNodeErrors(int32 aErr);
        void HandleMediaLayerNodeErrors(int32 aErr);
        void HandleProtocolEngineNodeErrors(int32 aErr);

        void PrintMetadataInfo();

        PVMFFormatType iVideoSinkFormatType;
        PVMFFormatType iAudioSinkFormatType;
        OSCL_wHeapString<OsclMemAllocator> wFileName;
        oscl_wchar output[512];
        uint32 iTestID;

        PVPMetadataList iMetadataKeyList;
        Oscl_Vector<PvmiKvp, OsclMemAllocator> iMetadataValueList;
        int32 iNumValues;

        uint32 iSessionDuration;
        bool bcloaking;
        bool oLiveSession;

        bool iMultiplePlay;
        uint32 iNumPlay;
        uint32 iTargetNumPlay;

        bool iProtocolRollOver;
        bool iProtocolRollOverWithUnknownURLType;
        bool iPlayListURL;

        bool iOverflow;

        PvmiCapabilityAndConfig* iPlayerCapConfigIF;
        PvmiKvp* iErrorKVP;
        PvmiKvp iKVPSetAsync;
        OSCL_StackString<128> iKeyStringSetAsync;

        PVMFStreamingDataSource* iStreamDataSource;
        PVMFSourceContextData* iSourceContextData;

        //FTDL
        void CreateDownloadDataSource();
        uint8 iPVXFileBuf[4096];
        PVMFDownloadDataSourcePVX* iDownloadContextDataPVX;
        PVMFDownloadDataSourceHTTP* iDownloadContextDataHTTP;
        int32 iDownloadMaxfilesize;
#if RUN_FASTTRACK_TESTCASES
        CPVXInfo iDownloadPvxInfo;
#endif
        OSCL_wHeapString<OsclMemAllocator> iDownloadURL;
        OSCL_wHeapString<OsclMemAllocator> iDownloadFilename;
        OSCL_HeapString<OsclMemAllocator> iDownloadProxy;
        OSCL_wHeapString<OsclMemAllocator> iDownloadConfigFilename;
        int32 iNumBufferingStart, iNumBufferingComplete, iNumUnderflow, iNumDataReady, iNumEOS;
        bool iDownloadOnly, iDownloadThenPlay;
        bool iContentTooLarge;
};

/*!
 *  A test case to test fetching meta data during play, when playing an local/PDL/rtsp url
 *  - Data Source: Specified by user of test case
 *  - Data Sink(s): Video[FileOutputNode-test_player_genericopenplaygetmetadata_video.dat]\n
 *                  Audio[FileOutputNode-test_player_genericopenplaygetmetadata_audio.dat]
 *  - Sequence:
 *             -# CreatePlayer()
 *             -# AddDataSource()
 *             -# Init()
 *             -# AddDataSink() (video)
 *             -# AddDataSink() (audio)
 *             -# Prepare()
 *             -# Start()
 *             -# Play 10 sec
 *             -# getmetadata and print
 *             -# Play until EOS
 *             -# Stop()
 *             -# RemoveDataSink() (video)
 *             -# RemoveDataSink() (audio)
 *             -# Reset()
 *             -# RemoveDataSource()
 *             -# DeletePlayer()
 *
 */
class pvplayer_async_test_genericopenplaygetmetadatatest : public pvplayer_async_test_base
{
    public:
        pvplayer_async_test_genericopenplaygetmetadatatest(PVPlayerAsyncTestParam aTestParam,
                PVMFFormatType aVideoSinkFormat,
                PVMFFormatType aAudioSinkFormat,
                uint32 aTestID,
                bool aWaitForEOS,
                bool aCloaking)
                : pvplayer_async_test_base(aTestParam)
                , iPlayer(NULL)
                , iDataSource(NULL)
                , iDataSinkVideo(NULL)
                , iDataSinkAudio(NULL)
                , iIONodeVideo(NULL)
                , iIONodeAudio(NULL)
                , iMIOFileOutVideo(NULL)
                , iMIOFileOutAudio(NULL)
                , iCurrentCmdId(0)
                , iCancelAllCmdId(0)
                , iSessionDuration(0)
                , bcloaking(aCloaking)
                , oLiveSession(false)
                , iProtocolRollOverWithUnknownURLType(false)
                , iPlayListURL(false)
                , iStreamDataSource(NULL)
                , iSourceContextData(NULL)
                , iDownloadContextDataPVX(NULL)
        {
            iVideoSinkFormatType = aVideoSinkFormat;
            iAudioSinkFormatType = aAudioSinkFormat;
            oWaitForEOS = aWaitForEOS;
            iTestID = aTestID;
            iNumPlay = 0;
            iTargetNumPlay = 1;
            iNumBufferingStart = iNumBufferingComplete = iNumUnderflow = iNumDataReady = iNumEOS = 0;
            iDownloadOnly = iDownloadThenPlay = false;
        }

        ~pvplayer_async_test_genericopenplaygetmetadatatest() {}

        void StartTest();
        void Run();

        void CommandCompleted(const PVCmdResponse& aResponse);
        void HandleErrorEvent(const PVAsyncErrorEvent& aEvent);
        void HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent);

        enum PVTestState
        {
            STATE_CREATE,
            STATE_QUERYINTERFACE,
            STATE_ADDDATASOURCE,
            STATE_CONFIGPARAMS,
            STATE_INIT,
            STATE_GETMETADATAKEYLIST,
            STATE_GETMETADATAVALUELIST,
            STATE_ADDDATASINK_VIDEO,
            STATE_ADDDATASINK_AUDIO,
            STATE_PREPARE,
            STATE_WAIT_FOR_DATAREADY,
            STATE_WAIT_FOR_BUFFCOMPLETE,
            STATE_CANCELALL,
            STATE_WAIT_FOR_CANCELALL,
            STATE_START,
            STATE_SETPLAYBACKRANGE,
            STATE_PAUSE,
            STATE_RESUME,
            STATE_EOSNOTREACHED,
            STATE_STOP,
            STATE_REMOVEDATASINK_VIDEO,
            STATE_REMOVEDATASINK_AUDIO,
            STATE_RESET,
            STATE_REMOVEDATASOURCE,
            STATE_CLEANUPANDCOMPLETE,
            STATE_GETMETADATAKEYLIST1,
            STATE_GETMETADATAVALUELIST1
        };

        PVTestState iState;

        PVPlayerInterface* iPlayer;
        PVPlayerDataSourceURL* iDataSource;
        PVPlayerDataSink* iDataSinkVideo;
        PVPlayerDataSink* iDataSinkAudio;
        PVMFNodeInterface* iIONodeVideo;
        PVMFNodeInterface* iIONodeAudio;
        PvmiMIOControl* iMIOFileOutVideo;
        PvmiMIOControl* iMIOFileOutAudio;
        PVCommandId iCurrentCmdId;
        PVCommandId iCancelAllCmdId;

        void setMultiplePlayMode(uint32 aNum)
        {
            iMultiplePlay = true;
            iTargetNumPlay = aNum;
        }

        void setProtocolRollOverModeWithUnknownURL()
        {
            iProtocolRollOverWithUnknownURLType = true;
        }

        void setPlayListMode()
        {
            iPlayListURL = true;
        }

    private:
        void HandleSocketNodeErrors(int32 aErr);
        void HandleRTSPNodeErrors(int32 aErr);
        void HandleStreamingManagerNodeErrors(int32 aErr);
        void HandleJitterBufferNodeErrors(int32 aErr);
        void HandleMediaLayerNodeErrors(int32 aErr);
        void HandleProtocolEngineNodeErrors(int32 aErr);

        void PrintMetadataInfo();

        PVMFFormatType iVideoSinkFormatType;
        PVMFFormatType iAudioSinkFormatType;
        OSCL_wHeapString<OsclMemAllocator> wFileName;
        oscl_wchar output[512];
        bool oWaitForEOS;
        uint32 iTestID;

        PVPMetadataList iMetadataKeyList;
        Oscl_Vector<PvmiKvp, OsclMemAllocator> iMetadataValueList;
        int32 iNumValues;

        uint32 iSessionDuration;
        bool bcloaking;
        bool oLiveSession;

        bool iMultiplePlay;
        uint32 iNumPlay;
        uint32 iTargetNumPlay;

        bool iProtocolRollOver;
        bool iProtocolRollOverWithUnknownURLType;
        bool iPlayListURL;

        PvmiCapabilityAndConfig* iPlayerCapConfigIF;
        PvmiKvp* iErrorKVP;
        PvmiKvp iKVPSetAsync;
        OSCL_StackString<128> iKeyStringSetAsync;

        PVMFStreamingDataSource* iStreamDataSource;

        PVMFSourceContextData* iSourceContextData;

        //FTDL
        void CreateDownloadDataSource();
        uint8 iPVXFileBuf[4096];
        PVMFDownloadDataSourcePVX* iDownloadContextDataPVX;
        PVMFDownloadDataSourceHTTP* iDownloadContextDataHTTP;
        int32 iDownloadMaxfilesize;
#if RUN_FASTTRACK_TESTCASES
        CPVXInfo iDownloadPvxInfo;
#endif
        OSCL_wHeapString<OsclMemAllocator> iDownloadURL;
        OSCL_wHeapString<OsclMemAllocator> iDownloadFilename;
        OSCL_HeapString<OsclMemAllocator> iDownloadProxy;
        OSCL_wHeapString<OsclMemAllocator> iDownloadConfigFilename;
        int32 iNumBufferingStart, iNumBufferingComplete, iNumUnderflow, iNumDataReady, iNumEOS;
        bool iDownloadOnly, iDownloadThenPlay;
        bool iContentTooLarge;
};

/*!
 *  A test case to check whether meta data contains any picture, if yes then retrieve and save
 *  while playing a local, PDL or streaming source
 *  - Data Source: Specified by user of test case
 *  - Data Sink(s): Video[FileOutputNode-test_player_genericopenplaygetmetadatapic_video.dat]\n
 *                  Audio[FileOutputNode-test_player_genericopenplaygetmetadatapic_audio.dat]
 *  - Sequence:
 *             -# CreatePlayer()
 *             -# AddDataSource()
 *             -# Init()
 *             -# AddDataSink() (video)
 *             -# AddDataSink() (audio)
 *             -# Prepare()
 *             -# Start()
 *             -# Stop()
 *             -# RemoveDataSink() (video)
 *             -# RemoveDataSink() (audio)
 *             -# Reset()
 *             -# RemoveDataSource()
 *             -# DeletePlayer()
 *
 */
class pvplayer_async_test_genericopengetmetadatapictest : public pvplayer_async_test_base
{
    public:
        pvplayer_async_test_genericopengetmetadatapictest(PVPlayerAsyncTestParam aTestParam,
                PVMFFormatType aVideoSinkFormat,
                PVMFFormatType aAudioSinkFormat,
                uint32 aTestID,
                bool aWaitForEOS,
                bool aCloaking)
                : pvplayer_async_test_base(aTestParam)
                , iPlayer(NULL)
                , iDataSource(NULL)
                , iDataSinkVideo(NULL)
                , iDataSinkAudio(NULL)
                , iIONodeVideo(NULL)
                , iIONodeAudio(NULL)
                , iMIOFileOutVideo(NULL)
                , iMIOFileOutAudio(NULL)
                , iCurrentCmdId(0)
                , iCancelAllCmdId(0)
                , iSessionDuration(0)
                , bcloaking(aCloaking)
                , oLiveSession(false)
                , iProtocolRollOverWithUnknownURLType(false)
                , iPlayListURL(false)
                , iStreamDataSource(NULL)
                , iSourceContextData(NULL)
                , iDownloadContextDataPVX(NULL)
        {
            iVideoSinkFormatType = aVideoSinkFormat;
            iAudioSinkFormatType = aAudioSinkFormat;
            oWaitForEOS = aWaitForEOS;
            iTestID = aTestID;
            iNumPlay = 0;
            iTargetNumPlay = 1;
            iNumBufferingStart = iNumBufferingComplete = iNumUnderflow = iNumDataReady = iNumEOS = 0;
            iDownloadOnly = iDownloadThenPlay = false;
        }

        ~pvplayer_async_test_genericopengetmetadatapictest() {}

        void StartTest();
        void Run();

        void CommandCompleted(const PVCmdResponse& aResponse);
        void HandleErrorEvent(const PVAsyncErrorEvent& aEvent);
        void HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent);

        enum PVTestState
        {
            STATE_CREATE,
            STATE_QUERYINTERFACE,
            STATE_ADDDATASOURCE,
            STATE_CONFIGPARAMS,
            STATE_INIT,
            STATE_GETMETADATAKEYLIST,
            STATE_GETMETADATAVALUELIST,
            STATE_ADDDATASINK_VIDEO,
            STATE_ADDDATASINK_AUDIO,
            STATE_PREPARE,
            STATE_WAIT_FOR_DATAREADY,
            STATE_WAIT_FOR_BUFFCOMPLETE,
            STATE_CANCELALL,
            STATE_WAIT_FOR_CANCELALL,
            STATE_START,
            STATE_SETPLAYBACKRANGE,
            STATE_PAUSE,
            STATE_RESUME,
            STATE_EOSNOTREACHED,
            STATE_STOP,
            STATE_REMOVEDATASINK_VIDEO,
            STATE_REMOVEDATASINK_AUDIO,
            STATE_RESET,
            STATE_REMOVEDATASOURCE,
            STATE_CLEANUPANDCOMPLETE,
            STATE_GETMETADATAKEYLIST1,
            STATE_GETMETADATAVALUELIST1
        };

        PVTestState iState;

        PVPlayerInterface* iPlayer;
        PVPlayerDataSourceURL* iDataSource;
        PVPlayerDataSink* iDataSinkVideo;
        PVPlayerDataSink* iDataSinkAudio;
        PVMFNodeInterface* iIONodeVideo;
        PVMFNodeInterface* iIONodeAudio;
        PvmiMIOControl* iMIOFileOutVideo;
        PvmiMIOControl* iMIOFileOutAudio;
        PVCommandId iCurrentCmdId;
        PVCommandId iCancelAllCmdId;

        void setMultiplePlayMode(uint32 aNum)
        {
            iMultiplePlay = true;
            iTargetNumPlay = aNum;
        }

        void setPlayListMode()
        {
            iPlayListURL = true;
        }

    private:
        void HandleSocketNodeErrors(int32 aErr);
        void HandleRTSPNodeErrors(int32 aErr);
        void HandleStreamingManagerNodeErrors(int32 aErr);
        void HandleJitterBufferNodeErrors(int32 aErr);
        void HandleMediaLayerNodeErrors(int32 aErr);
        void HandleProtocolEngineNodeErrors(int32 aErr);

        void PrintMetadataInfo();

        PVMFFormatType iVideoSinkFormatType;
        PVMFFormatType iAudioSinkFormatType;
        OSCL_wHeapString<OsclMemAllocator> wFileName;
        oscl_wchar output[512];
        bool oWaitForEOS;
        uint32 iTestID;

        PVPMetadataList iMetadataKeyList;
        Oscl_Vector<PvmiKvp, OsclMemAllocator> iMetadataValueList;
        int32 iNumValues;

        uint32 iSessionDuration;
        bool bcloaking;
        bool oLiveSession;

        bool iMultiplePlay;
        uint32 iNumPlay;
        uint32 iTargetNumPlay;

        bool iProtocolRollOver;
        bool iProtocolRollOverWithUnknownURLType;
        bool iPlayListURL;

        PvmiCapabilityAndConfig* iPlayerCapConfigIF;
        PvmiKvp* iErrorKVP;
        PvmiKvp iKVPSetAsync;
        OSCL_StackString<128> iKeyStringSetAsync;

        PVMFStreamingDataSource* iStreamDataSource;

        PVMFSourceContextData* iSourceContextData;

        //FTDL
        void CreateDownloadDataSource();
        uint8 iPVXFileBuf[4096];
        PVMFDownloadDataSourcePVX* iDownloadContextDataPVX;
        PVMFDownloadDataSourceHTTP* iDownloadContextDataHTTP;
        int32 iDownloadMaxfilesize;
#if RUN_FASTTRACK_TESTCASES
        CPVXInfo iDownloadPvxInfo;
#endif
        OSCL_wHeapString<OsclMemAllocator> iDownloadURL;
        OSCL_wHeapString<OsclMemAllocator> iDownloadFilename;
        OSCL_HeapString<OsclMemAllocator> iDownloadProxy;
        OSCL_wHeapString<OsclMemAllocator> iDownloadConfigFilename;
        int32 iNumBufferingStart, iNumBufferingComplete, iNumUnderflow, iNumDataReady, iNumEOS;
        bool iDownloadOnly, iDownloadThenPlay;
        bool iContentTooLarge;
};
#endif
