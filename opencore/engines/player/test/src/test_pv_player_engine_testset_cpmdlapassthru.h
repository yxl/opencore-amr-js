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
#ifndef TEST_PV_PLAYER_ENGINE_TESTSET_CPMDLAPASSTHRU_H_INCLUDED
#define TEST_PV_PLAYER_ENGINE_TESTSET_CPMDLAPASSTHRU_H_INCLUDED

/**
 *  @file test_pv_player_engine_testset_cpmdlapassthru.h
 *  @brief This file contains the class definitions for the cpmdlaspassthru set of
 *         test cases for PVPlayerEngine
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

#ifndef PVMF_CPMPLUGIN_FACTORY_REGISTRY_H_INCLUDED
#include "pvmf_cpmplugin_factory_registry.h"
#endif


class PVPlayerDataSourceURL;
class PVPlayerDataSink;
class PVPlayerDataSink;
class PVPlayerDataSinkFilename;
class PvmfFileOutputNodeConfigInterface;
class PvmiCapabilityAndConfig;
class PVMFDownloadDataSourcePVX;

enum OMA1_DLA_TESTMODE
{
    OMA1_DLA_FAIL = 0, //AuthorizeUsage will fail, call Acquire license and Init again
    OMA1_DLA_NORMAL, //Normal Processing , no failre during AuthorizeUsage
    OMA1_DLA_UNKNOWN_CPM_CONTENTTYPE, //Set the cpm to return UnknownCotentFormat instead of AUTH_BEFORE_ACCESS
    OMA1_DLA_CANCEL_ACQUIRE_LICENSE_FAILS, //CancelAcquireLic will fail, GetLicense already returned CommandCompleted
    OMA1_DLA_CANCEL_ACQUIRE_LICENSE_SUCCEEDS, //CancelAcquireLic will succeed, GetLicense should not return CommandCompleted
    OMA1_DLA_CONTENT_NOTSUPPORTED //RegiserContent will handle "notsupported" case
};

/*!
 *  A test case to test CPM DLA Pass Thru Plugin
 *  - Data Source: Any Media ranging from amr, aac, mp4, mp3, asf, pvx, sdp, rtsp, PDL, rm, ASF streaming
 *  - Data Sink(s): Video[test_player__<rtspurl>_cpmdlapassthru_video.dat]\n
 *                  Audio[test_player__<rtspurl>_cpmdlapassthru_audio.dat]
 *  - Sequence:
 *             -# CreatePlayer()
 *             -# AddDataSource()
 *             -# Init() will Fail depending on the TestMode chosen
 *	       -# Query License
 *	       -# Acquire License
 *             -# AddDataSink() (video)
 *             -# AddDataSink() (audio)
 *             -# Prepare()
 *             -# Start()
 *             -# WAIT for EOS.
 *             -# Stop()
 *             -# RemoveDataSink() (video)
 *             -# RemoveDataSink() (audio)
 *             -# Reset()
 *             -# RemoveDataSource()
 *             -# DeletePlayer()
 *
 */
class PVMFLocalDataSource;
class PVMFOma1PassthruPluginFactory;
class PVPlayerLicenseAcquisitionInterface;
class pvplayer_async_test_cpmdlapassthru : public pvplayer_async_test_base
{
    public:
        pvplayer_async_test_cpmdlapassthru(PVPlayerAsyncTestParam aTestParam,
                                           PVMFFormatType aVideoSinkFormat,
                                           PVMFFormatType aAudioSinkFormat,
                                           uint32 aTestID,
                                           bool aPauseResumeEnable,
                                           bool aSeekEnable,
                                           bool aWaitForEOS,
                                           int atestMode,
                                           bool aCancelAcquireLicense)
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
                , iAcquireLicenseCmdId(0)
                , iSessionDuration(0)
                , oLiveSession(false)
                , iProtocolRollOver(false)
                , iProtocolRollOverWithUnknownURLType(false)
                , iPlayListURL(false)
                , iStreamDataSource(NULL)
                , iSourceContextData(NULL)
                , iLocalDataSource(NULL)
                , iDownloadContextDataPVX(NULL)

        {
            iVideoSinkFormatType = aVideoSinkFormat;
            iAudioSinkFormatType = aAudioSinkFormat;
            oPauseResumeEnable = aPauseResumeEnable;
            oSeekEnable = aSeekEnable;
            oWaitForEOS = aWaitForEOS;
            iTestID = aTestID;
            iNumPlay = 0;
            iTargetNumPlay = 1;
            iNumBufferingStart = iNumBufferingComplete = iNumUnderflow = iNumDataReady = iNumEOS = 0;
            iDownloadOnly = iDownloadThenPlay = false;
            iTestMode = atestMode;
            oCancelAcquireLicense = aCancelAcquireLicense;
        }

        ~pvplayer_async_test_cpmdlapassthru() {}

        void StartTest();
        void Run();

        void CommandCompleted(const PVCmdResponse& aResponse);
        void HandleErrorEvent(const PVAsyncErrorEvent& aEvent);
        void HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent);

        PVTestState iState;
        PVTestState iEndState;

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
        PVCommandId iCancelCmdId;
        PVCommandId iAcquireLicenseCmdId;
        int iTestMode;

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
        bool oPauseResumeEnable;
        bool oSeekEnable;
        bool oWaitForEOS;
        bool oCancelAcquireLicense;
        uint32 iTestID;

        PVPMetadataList iMetadataKeyList;
        Oscl_Vector<PvmiKvp, OsclMemAllocator> iMetadataValueList;
        int32 iNumValues;

        uint32 iSessionDuration;
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
        PVMFLocalDataSource* iLocalDataSource;

        char iClipType[100];
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
        PVMFCPMPluginFactoryRegistryClient iPluginRegistryClient;
        PVMFOma1PassthruPluginFactory* iPluginFactory;
        OSCL_HeapString<OsclMemAllocator> iPluginMimeType;
        //for license acquisition.
        PVPlayerLicenseAcquisitionInterface* iLicenseIF;
};

#endif
