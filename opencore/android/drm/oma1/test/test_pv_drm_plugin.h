/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TEST_PV_DRM_PLUGIN_H_INCLUDED
#define TEST_PV_DRM_PLUGIN_H_INCLUDED

/**
 *  @file test_pv_drm_plugin.h
 *  @brief This file contains the class definitions for the oma drm 1.0
 *         test cases for PVPlayerEngine
 */
#include "test_pv_player_engine_main.h"
#include "pv_player_datasourceurl.h"
#include "pvmf_node_interface.h"
#include "pv_engine_types.h"
#include "test_pv_player_engine_config.h"
#include "oscl_file_io.h"
#include "pvmi_config_and_capability_observer.h"
#include "pvmf_cpmplugin_factory_registry.h"
#include "pvmf_recognizer_types.h"
#include "pvmf_recognizer_registry.h"
#include "pvoma1_kmj_recognizer.h"
#include "pvmi_datastreamsyncinterface_ref_factory.h"
#include "pvmi_data_stream_interface.h"


class PVMFLocalDataSource;
class PVMFOma1KmjPluginFactory;

/*!
 *  A test case to test the normal engine sequence of playing a specified
 *  source with kmj CPM plug-in
 *  - Data Source: Passed in parameter
 *  - Data Sink(s):
 *     Video[FileOutputNode-test_player_cpmopenplaystop_[SRCFILENAME]_video.dat]\n
 *     Audio[FileOutputNode-test_player_cpmopenplaystop_[SRCFILENAME]_audio.dat]\n
 *     Text[FileOutputNode-test_player_cpmopenplaystop_[SRCFILENAME]_text.dat]
 *  - Sequence:
 *             -# CreatePlayer()
 *             -# AddDataSource() with pass-through CPM plug-in
 *             -# Init()
 *             -# AddDataSink() (video)
 *             -# AddDataSink() (audio)
 *             -# AddDataSink() (text)
 *             -# Prepare()
 *             -# Start()
 *             -# WAIT 15 sec.
 *             -# Stop()
 *             -# RemoveDataSink() (video)
 *             -# RemoveDataSink() (audio)
 *             -# RemoveDataSink() (text)
 *             -# Reset()
 *             -# RemoveDataSource()
 *             -# DeletePlayer()
 *
 */
class pvplayer_async_test_drmcpmplugin : public pvplayer_async_test_base,
                                         public PVMFRecognizerCommmandHandler
{
public:
    pvplayer_async_test_drmcpmplugin(PVPlayerAsyncTestParam aTestParam):
        pvplayer_async_test_base(aTestParam)
        ,iPlayer(NULL)
        ,iDataSource(NULL)
        ,iDataSinkVideo(NULL)
        ,iIONodeVideo(NULL)
        ,iMIOFileOutVideo(NULL)
        ,iDataSinkAudio(NULL)
        ,iIONodeAudio(NULL)
        ,iMIOFileOutAudio(NULL)
        ,iDataSinkText(NULL)
        ,iIONodeText(NULL)
        ,iMIOFileOutText(NULL)
        ,iCurrentCmdId(0)
        {
            iTestCaseName=_STRLIT_CHAR("DRM plugin Open-Play-Stop-Reset");
            iLocalDataSource=NULL;
            iPluginFactory=NULL;
        }

    ~pvplayer_async_test_drmcpmplugin() {}

    void StartTest();
    void Run();
    void RecognizerCommandCompleted(const PVMFCmdResp& aResponse){}

    void CommandCompleted(const PVCmdResponse& aResponse);
    void HandleErrorEvent(const PVAsyncErrorEvent& aEvent);
    void HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent);

    enum PVTestState
    {
        STATE_CREATE,
        STATE_ADDDATASOURCE,
        STATE_INIT,
        STATE_ACQUIRELICENSE,
        STATE_GETMETADATAKEYS1,
        STATE_GETMETADATAVALUES1,
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
        STATE_WAIT_FOR_ERROR_HANDLING,
        STATE_CLEANUPANDCOMPLETE
    };

    PVTestState iState;

    PVPlayerInterface* iPlayer;
    PVPlayerDataSourceURL* iDataSource;
    PVPlayerDataSink* iDataSinkVideo;
    PVMFNodeInterface* iIONodeVideo;
    PvmiMIOControl* iMIOFileOutVideo;
    PVPlayerDataSink* iDataSinkAudio;
    PVMFNodeInterface* iIONodeAudio;
    PvmiMIOControl* iMIOFileOutAudio;
    PVPlayerDataSink* iDataSinkText;
    PVMFNodeInterface* iIONodeText;
    PvmiMIOControl* iMIOFileOutText;
    PVCommandId iCurrentCmdId;

    PVMFRecognizerPluginFactory* iRecognizePluginFactory;
    PVMFLocalDataSource* iLocalDataSource;
    PVMFCPMPluginFactoryRegistryClient iPluginRegistryClient;
    PVMFOma1KmjPluginFactory* iPluginFactory;
    OSCL_HeapString<OsclMemAllocator> iPluginMimeType;

private:
    uint8_t* aLicenseData;
    PVPMetadataList iKeyList;
    int32 iNumAvailableValues;
    Oscl_Vector<PvmiKvp,OsclMemAllocator> iValueList;
    OSCL_wHeapString<OsclMemAllocator> wFileName;
    oscl_wchar output[512];
    bool bDoGetLicenseProc;
    bool bForceDownloadRights;
    int32_t drmSession;
    Oscl_FileServer iFileServer;
    OsclFileHandle* iFileHandle;
    Oscl_File* iFileObject;

    #define DRM_MEDIA_FILE_FOR_COMPARE "/data/drm/drm1_core_files/tmp1.drm"
    void SavePureMediaForCompare(void);
    int32_t GetMimeType(char* drmFileName);
};

extern int32_t DrmPluginSTGetFileDataLen(int32_t fileHandle);
extern int32_t DrmPluginSTSeekFileData( int32_t fileHandle,int32_t offset);
extern int32_t DrmPluginSTReadFileData( int32_t fileHandle,uint8_t* buf,int32_t bufLen);

#endif /* TEST_PV_DRM_PLUGIN_H_INCLUDED */
