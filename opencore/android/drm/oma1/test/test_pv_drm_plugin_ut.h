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

#ifndef TEST_PV_DRM_PLUGIN_UT_H_
#define TEST_PV_DRM_PLUGIN_UT_H_

#include "test_pv_player_engine_main.h"
#include "pvmf_cpmplugin_kmj_oma1_types.h"
#include "pvmf_cpmplugin_kmj_oma1_factory.h"
#include "pvmf_cpmplugin_kmj_oma1.h"
#include "pvmf_cpmplugin_contentmanagement_interface.h"
#include "pvoma1_kmj_recognizer.h"

class pvplayer_test_drmcpmplugin_ut : public pvplayer_async_test_base
{
public:
    pvplayer_test_drmcpmplugin_ut(PVPlayerAsyncTestParam aTestParam);
    ~pvplayer_test_drmcpmplugin_ut();

    void StartTest();
    void Run();
private:
    /// common methods
    bool InitBeforeOneCaseStart();
    void ResetAfterOneCaseComplete();
    void GetAvailableKeys();

    ///Query UUID command
    void test_kmj_plugin_QueryUUID();

    /// query unsupported UUID
    void test_kmj_plugin_DoQueryUuid_1();

    /// query supported UUID
    void test_kmj_plugin_DoQueryUuid_2();
    void test_kmj_plugin_DoQueryUuid_3();
    void test_kmj_plugin_DoQueryUuid_4();
    void test_kmj_plugin_DoQueryUuid_5();
    void test_kmj_plugin_DoQueryUuid_6();

    ///Query Interface command
    void test_kmj_plugin_QueryInterface();

    /// invalid state to query interface
    void test_kmj_plugin_DoQueryInterface_1();
    void test_kmj_plugin_DoQueryInterface_2();

    /// query unsupported interface
    void test_kmj_plugin_DoQueryInterface_3();

    /// query supported interface
    void test_kmj_plugin_DoQueryInterface_4();
    void test_kmj_plugin_DoQueryInterface_5();
    void test_kmj_plugin_DoQueryInterface_6();
    void test_kmj_plugin_DoQueryInterface_7();
    void test_kmj_plugin_DoQueryInterface_8();

    /// authorize command
    void test_kmj_plugin_AuthorizeUsage();

    /// useage complete command
    void test_kmj_plugin_UsageComplete();

    /// unsupported license
    void test_kmj_plugin_DoAuthorizeUsage_1(void);
    void test_kmj_plugin_DoAuthorizeUsage_2(void);
    void test_kmj_plugin_DoAuthorizeUsage_3(void);

    /// supported license
    void test_kmj_plugin_DoAuthorizeUsage_4(void);

    /// get license char command
    void test_kmj_plugin_GetLicense();

    /// get license wchar command
    void test_kmj_plugin_GetLicense_w();

    /// parse IP format URL
    void test_kmj_plugin_ParseUrlIp(void);

    /// parse host format URL
    void test_kmj_plugin_ParseUrlHost(void);

    /// do get license
    void test_kmj_plugin_DoGetLicense(void);

    // get all available keys num
    void test_kmj_plugin_GetNumMetadataKeys_1();

    // get any one valid key num
    void test_kmj_plugin_GetNumMetadataKeys_2();

    // get invalid key num
    void test_kmj_plugin_GetNumMetadataKeys_3();

    // get all available Values num
    void test_kmj_plugin_GetNumMetadataValules_1();

    // get any one valid Value num
    void test_kmj_plugin_GetNumMetadataValules_2();

    // get invalid Vaule num
    void test_kmj_plugin_GetNumMetadataValules_3();

    /// get metadata keys command
    void test_kmj_plugin_GetNodeMetadataKeys();

    /// get metadata values command
    void test_kmj_plugin_GetNodeMetadataValues();

    /// invalid arguments
    void test_kmj_plugin_DoGetMetadataKeys_1();
    void test_kmj_plugin_DoGetMetadataKeys_2();
    void test_kmj_plugin_DoGetMetadataKeys_3();

    /// get one unavailable key
    void test_kmj_plugin_DoGetMetadataKeys_4();

    /// get any one key
    void test_kmj_plugin_DoGetMetadataKeys_5();
    void test_kmj_plugin_DoGetMetadataKeys_6();
    void test_kmj_plugin_DoGetMetadataKeys_7();

    /// get all keys
    void test_kmj_plugin_DoGetMetadataKeys_8();
    void test_kmj_plugin_DoGetMetadataKeys_9();

    /// invalid arguments
    void test_kmj_plugin_DoGetMetadataValues_1();
    void test_kmj_plugin_DoGetMetadataValues_2();
    void test_kmj_plugin_DoGetMetadataValues_3();

    // get is protected
    void test_kmj_plugin_DoGetMetadataValues_4();

    // get is license available
    void test_kmj_plugin_DoGetMetadataValues_5();

    // get is forward-lock
    void test_kmj_plugin_DoGetMetadataValues_6();

    // get can be used as ring-tone
    void test_kmj_plugin_DoGetMetadataValues_7();

    // get license type
    void test_kmj_plugin_DoGetMetadataValues_8();

    // get license count
    void test_kmj_plugin_DoGetMetadataValues_9();

    // get license start time
    void test_kmj_plugin_DoGetMetadataValues_10();

    // get license expiration time
    void test_kmj_plugin_DoGetMetadataValues_11();

    // get license remain duration
    void test_kmj_plugin_DoGetMetadataValues_12();

    // get enveloped content length
    void test_kmj_plugin_DoGetMetadataValues_13();

    // get enveloped content
    void test_kmj_plugin_DoGetMetadataValues_14();

    // get all available values
    void test_kmj_plugin_DoGetMetadataValues_15();

    /// invalid arguments
    void test_kmj_plugin_ReleaseNodeMetadataValues_1();
    void test_kmj_plugin_ReleaseNodeMetadataValues_2();
    void test_kmj_plugin_ReleaseNodeMetadataValues_3();

    // release all values newed from memory
    void test_kmj_plugin_ReleaseNodeMetadataValues_4();

    /// invalid arguments
    void test_kmj_plugin_GetIso8601Time_1();
    void test_kmj_plugin_GetIso8601Time_2();
    void test_kmj_plugin_GetIso8601Time_3();

    // get valid ISO8601 time
    void test_kmj_plugin_GetIso8601Time_4();

    /// invalid arguments
    void test_kmj_plugin_GetDuration_1();
    void test_kmj_plugin_GetDuration_2();

    // get valid duration
    void test_kmj_plugin_GetDuration_3();
    void test_kmj_plugin_GetDuration_4();

    // test access data through file
  void test_kmj_dataaccess_DrmPluginGetDataLen_file();
  void test_kmj_dataaccess_DrmPluginSeekData_file();
  void test_kmj_dataaccess_DrmPluginReadData_file();

  // test access data through buffer
  void test_kmj_dataaccess_DrmPluginGetDataLen_buffer();
  void test_kmj_dataaccess_DrmPluginSeekData_buffer();
  void test_kmj_dataaccess_DrmPluginReadData_buffer();

  void test_kmj_recognizer_RecognizeContentType_mp4();
  void test_kmj_recognizer_RecognizeContentType_mp3();
  void test_kmj_recognizer_RecognizeContentType_amr();
  void test_kmj_recognizer_RecognizeContentType_aac();
  void test_kmj_recognizer_RecognizeContentType_unknown();

  void test_kmj_recognizer_Recognize();

  PVMFCPMKmjPlugInOMA1*  kmjPlugin;
  PVMFCPMPluginInterface* pluginInterface;

  PVMFRecognizerPluginInterface* recognizerInterface;
  PVOMA1KMJRecognizerPlugin* recognizer;
  uint32_t numKeys;
  Oscl_Vector<OSCL_HeapString<OsclMemAllocator>, OsclMemAllocator> iAvailableMetadataKeys;
};

#endif
