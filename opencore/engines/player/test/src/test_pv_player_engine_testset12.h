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
#ifndef TEST_PV_PLAYER_ENGINE_TESTSET12_H_INCLUDED
#define TEST_PV_PLAYER_ENGINE_TESTSET12_H_INCLUDED

/**
 *  @file test_pv_player_engine_testset12.h
 *  @brief This file contains the class definitions for the twelfth set of
 *         test cases for PVPlayerEngine - Generic(local, PDL & streaming) playback tests
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

#define NUM_PAUSE -1 // No of times 'PAUSE' infinite in case of Multiple Pause
#define	SEQUENTIAL_PAUSE_INTERVAL 1
#define	FIRST_PAUSE_AFTER_START 10
#define	PAUSE_RESUME_INTERVAL 1

#define	SEQUENTIAL_SEEK_INTERVAL 2
#define	FIRST_SEEK_AFTER_START 2

#define SLEEP_TIME_SECS 60

class PVPlayerDataSourceURL;
class PVPlayerDataSink;
class PVPlayerDataSink;
class PVPlayerDataSinkFilename;
class PvmfFileOutputNodeConfigInterface;
class PvmiCapabilityAndConfig;
class PVMFDownloadDataSourcePVX;


/*!
 *  Test cases to test Multiple pause resume, multiple seek during playback
 *  and Sleep of 90 secs right after each state when playing an local/PDL/rtsp url
 *  - Data Source: Specified by user of test case
 *  - Data Sink(s): Video[FileOutputNode-test_player_genericopenplaystop_video.dat]\n
 *                  Audio[FileOutputNode-test_player_genericopenplaystop_audio.dat]
 *  - Sequence:
 *             -# CreatePlayer()
 *             -# Sleep() is called in each of the states for sleep tests
 *             -# AddDataSource()
 *             -# Init()
 *             -# AddDataSink() (video)
 *             -# AddDataSink() (audio)
 *             -# Prepare()
 *             -# Start()
 *             -# Play 10 sec
 *             -# SetPlaybackRange() if seekEnable flag is made true, set 10 times
 *				   if in case of multiple seek test
 *             -# Pause() if pauseresumeEnable flag is made true, pauseresume multiple times
 *					in multiple pauseresume test
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
class pvplayer_async_test_genericopenplaystop : public pvplayer_async_test_base
{
    public:
        pvplayer_async_test_genericopenplaystop(PVPlayerAsyncTestParam aTestParam,
                                                PVMFFormatType aVideoSinkFormat,
                                                PVMFFormatType aAudioSinkFormat,
                                                uint32 aTestID,
                                                bool aPauseResumeEnable,
                                                bool aSeekEnable,
                                                bool aWaitForEOS,
                                                bool aCloaking,
                                                bool aCancelDuringPrepare,
                                                uint32 aSleepState = STATE_CLEANUPANDCOMPLETE,
                                                bool aSleepInState = false)
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
                , iSeekBeyondClipDurationMode(false)
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
            oPauseResumeEnable = aPauseResumeEnable;
            oSeekEnable = aSeekEnable;
            oWaitForEOS = aWaitForEOS;
            oCancelDuringPrepare = aCancelDuringPrepare;
            iTestID = aTestID;
            iNumPlay = 0;
            iTargetNumPlay = 1;
            iNumBufferingStart = iNumBufferingComplete = iNumUnderflow = iNumDataReady = iNumEOS = 0;
            iNumPause = 0;
            iTargetNumPause = -1; // No of times 'Pause' is infinite
            iTimeIntervalSeqPause = 1;
            iFirstPauseTimeAfterStart = 10;
            iDownloadOnly = iDownloadThenPlay = false;
            iNumSeek = 0;
            iTimeIntervalSeqSeek = 10;
            iFirstSeekTimeAfterStart = 10;
            iRolloverDone = false;
            iPlayStarted = false;
            iSleepState = (PVTestState)aSleepState;
            iSleepInState = aSleepInState;
        }

        ~pvplayer_async_test_genericopenplaystop() {}

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
        PVTestState iSleepState;

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

        void setMultiplePauseMode(int32 aNum1, uint32 aNum2 = 10, uint32 aNum3 = 10)
        {
            iMultiplePause = true;
            iTargetNumPause = aNum1;
            iTimeIntervalSeqPause = aNum2;
            iFirstPauseTimeAfterStart = aNum3;
        }

        void setMultipleSeekMode(uint32 aNum1 = 10, uint32 aNum2 = 10)
        {
            iTimeIntervalSeqSeek = aNum1;
            iFirstSeekTimeAfterStart = aNum2;
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

        void setSeekBeyondClipDurationMode()
        {
            oSeekEnable = true;
            iSeekBeyondClipDurationMode = true;
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
        bool iSeekAfterEOSEnable;
        bool iSeekBeyondClipDurationMode;

        PVPMetadataList iMetadataKeyList;
        Oscl_Vector<PvmiKvp, OsclMemAllocator> iMetadataValueList;
        int32 iNumValues;

        uint32 iSessionDuration;
        bool bcloaking;
        bool oLiveSession;

        bool iMultiplePause;
        int32 iNumPause;
        int32 iTargetNumPause;
        uint32 iTimeIntervalSeqPause;
        uint32 iFirstPauseTimeAfterStart;

        uint32 iNumSeek;
        uint32 iTimeIntervalSeqSeek;
        uint32 iFirstSeekTimeAfterStart;

        bool iMultiplePlay;
        uint32 iNumPlay;
        uint32 iTargetNumPlay;

        bool iProtocolRollOver;
        bool iProtocolRollOverWithUnknownURLType;
        bool iPlayListURL;
        bool iRolloverDone;
        bool iPlayStarted;
        bool iSleepInState;

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
