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

#include <stdio.h>
#include "oscl_utf8conv.h"
#include "pvmi_drm_kvp.h"
#include "test_pv_drm_plugin_ut.h"
#include "pvmi_datastreamsyncinterface_ref_factory.h"
#include "pvmi_data_stream_interface.h"

pvplayer_test_drmcpmplugin_ut::pvplayer_test_drmcpmplugin_ut(PVPlayerAsyncTestParam aTestParam)
                  : pvplayer_async_test_base(aTestParam)
{
    iTestCaseName =_STRLIT_CHAR("DRM plugin unit test");
    pluginInterface = PVMFCPMKmjPlugInOMA1::CreatePlugIn();
    kmjPlugin = OSCL_DYNAMIC_CAST(PVMFCPMKmjPlugInOMA1*,pluginInterface);

    recognizerInterface = OSCL_STATIC_CAST(PVMFRecognizerPluginInterface*, OSCL_NEW(PVOMA1KMJRecognizerPlugin, ()));

    if (recognizerInterface == NULL)
    {
        OSCL_LEAVE(OsclErrNoMemory);
        return;
    }

    recognizer = OSCL_DYNAMIC_CAST(PVOMA1KMJRecognizerPlugin*,recognizerInterface);
}

pvplayer_test_drmcpmplugin_ut::~pvplayer_test_drmcpmplugin_ut()
{
    PVMFCPMKmjPlugInOMA1::DestroyPlugIn(pluginInterface);
    if (recognizerInterface)
    {
        OSCL_DELETE(((PVOMA1KMJRecognizerPlugin*)recognizerInterface));
    }
}

bool pvplayer_test_drmcpmplugin_ut::InitBeforeOneCaseStart()
{
    if(kmjPlugin)
    {
        kmjPlugin->bRunUT = true;
        kmjPlugin->iAvailableMetadataKeys.clear();
        kmjPlugin->iInputCommands.Construct(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START,
                                            PVMF_CPM_KMJ_PLUGIN_OMA1_INTERNAL_CMDQ_SIZE);

        OSCL_wHeapString<OsclMemAllocator> wFileName;
        oscl_wchar output[512];

        oscl_UTF8ToUnicode(iFileName, oscl_strlen(iFileName),output,512);
        wFileName.set(output,oscl_strlen(output));

        if(PVMFSuccess != kmjPlugin->SetSourceInitializationData(wFileName,iFileType,NULL))
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    GetAvailableKeys();

    return true;
}

void pvplayer_test_drmcpmplugin_ut::ResetAfterOneCaseComplete()
{
    numKeys = 0;
    kmjPlugin->bRunUT = false;

    kmjPlugin->m_pvfile.Close();

    if(kmjPlugin->m_dataAccessFile)
    {
        OSCL_DELETE(kmjPlugin->m_dataAccessFile);
        kmjPlugin->m_dataAccessFile = NULL;
    }

    if(iAvailableMetadataKeys.size() > 0)
    {
        iAvailableMetadataKeys.clear();
    }

    if(kmjPlugin)
    {
        kmjPlugin->iAvailableMetadataKeys.clear();
        kmjPlugin->iInputCommands.clear();
    }
}

void pvplayer_test_drmcpmplugin_ut::GetAvailableKeys()
{
    numKeys = 0;
    iAvailableMetadataKeys.clear();

    T_DRM_Rights_Info rightsInfo;
    oscl_memset(&rightsInfo,0,sizeof(T_DRM_Rights_Info));

    if(FALSE == SVC_drm_getRightsInfo(kmjPlugin->m_drmSession,&rightsInfo))
    {
        return;
    }

    if(0 == rightsInfo.playRights.indicator)
    {
        return;
    }

    //init metadata keys
    iAvailableMetadataKeys.push_front(PVMF_DRM_INFO_IS_PROTECTED_QUERY);
    iAvailableMetadataKeys.push_front(PVMF_DRM_INFO_IS_LICENSE_AVAILABLE_QUERY);
    iAvailableMetadataKeys.push_front(PVMF_DRM_INFO_IS_FORWARD_LOCKED_QUERY);
    iAvailableMetadataKeys.push_front(PVMF_DRM_INFO_CAN_USE_AS_RINGTONE_QUERY);
    iAvailableMetadataKeys.push_front(PVMF_DRM_INFO_LICENSE_TYPE_QUERY);

    //has start time license
    if(rightsInfo.playRights.indicator & 0x01)
    {
        iAvailableMetadataKeys.push_front(PVMF_DRM_INFO_LICENSE_START_TIME_QUERY);
    }

    // has count license
    if(rightsInfo.playRights.indicator & 0x02)
    {
        iAvailableMetadataKeys.push_front(PVMF_DRM_INFO_LICENSE_COUNT_QUERY);
    }

    // has duration license
    if(rightsInfo.playRights.indicator & 0x04)
    {
        iAvailableMetadataKeys.push_front(PVMF_DRM_INFO_LICENSE_DURATION_SINCE_FIRST_USE_QUERY);
    }

    // has end time license
    if(rightsInfo.playRights.indicator & 0x08)
    {
        iAvailableMetadataKeys.push_front(PVMF_DRM_INFO_LICENSE_EXPIRATION_TIME_QUERY);
    }

    iAvailableMetadataKeys.push_front(PVMF_DRM_INFO_ENVELOPE_DATA_SIZE_QUERY);
    iAvailableMetadataKeys.push_front(PVMF_DRM_INFO_ENVELOPE_DATA_QUERY);

    numKeys = iAvailableMetadataKeys.size();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_QueryUUID()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        OSCL_HeapString<OsclMemAllocator> mimeType = PVMF_CPMPLUGIN_LICENSE_INTERFACE_MIMETYPE;
        Oscl_Vector<PVUuid, OsclMemAllocator> uuIds;
        int retCmdId = kmjPlugin->QueryUUID( 0,
                                             mimeType,
                                             uuIds,
                                             false,
                                             NULL);

        if(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START == retCmdId)
        {
             PVPATB_TEST_IS_TRUE(true);
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_DoQueryUuid_1()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        OSCL_HeapString<OsclMemAllocator> mimeType = PVMF_CPMPLUGIN_CONTENTMANAGEMENT_INTERFACE_MIMETYPE;
        Oscl_Vector<PVUuid, OsclMemAllocator> uuIds;
        int retCmdId = kmjPlugin->QueryUUID( 0,
                                             mimeType,
                                             uuIds,
                                             false,
                                             NULL);

        if(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START == retCmdId)
        {
            PVMFCPMKmjPlugInOMA1Command* cmd = kmjPlugin->iInputCommands.FindById(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START);

            if(NULL == cmd)
            {
                PVPATB_TEST_IS_TRUE(false);
            }
            else
            {
                kmjPlugin->DoQueryUuid(*cmd);

                if(0 == uuIds.size())
                {
                    PVPATB_TEST_IS_TRUE(true);
                }
                else
                {
                    PVPATB_TEST_IS_TRUE(false);
                }
            }
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_DoQueryUuid_2()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        OSCL_HeapString<OsclMemAllocator> mimeType = PVMF_CPMPLUGIN_AUTHENTICATION_INTERFACE_MIMETYPE;
        Oscl_Vector<PVUuid, OsclMemAllocator> uuIds;
        int retCmdId = kmjPlugin->QueryUUID( 0,
                                             mimeType,
                                             uuIds,
                                             false,
                                             NULL);

        if(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START == retCmdId)
        {
            PVMFCPMKmjPlugInOMA1Command* cmd = kmjPlugin->iInputCommands.FindById(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START);

            if(NULL == cmd)
            {
                PVPATB_TEST_IS_TRUE(false);
            }
            else
            {
                kmjPlugin->DoQueryUuid(*cmd);

                if(1 == uuIds.size())
                {
                    PVPATB_TEST_IS_TRUE(true);
                }
                else
                {
                    PVPATB_TEST_IS_TRUE(false);
                }
            }
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_DoQueryUuid_3()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        OSCL_HeapString<OsclMemAllocator> mimeType = PVMF_CPMPLUGIN_AUTHORIZATION_INTERFACE_MIMETYPE;
        Oscl_Vector<PVUuid, OsclMemAllocator> uuIds;
        int retCmdId = kmjPlugin->QueryUUID( 0,
                                             mimeType,
                                             uuIds,
                                             false,
                                             NULL);

        if(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START == retCmdId)
        {
            PVMFCPMKmjPlugInOMA1Command* cmd = kmjPlugin->iInputCommands.FindById(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START);

            if(NULL == cmd)
            {
                PVPATB_TEST_IS_TRUE(false);
            }
            else
            {
                kmjPlugin->DoQueryUuid(*cmd);

                if(1 == uuIds.size())
                {
                    PVPATB_TEST_IS_TRUE(true);
                }
                else
                {
                    PVPATB_TEST_IS_TRUE(false);
                }
            }
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_DoQueryUuid_4()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        OSCL_HeapString<OsclMemAllocator> mimeType = PVMF_CPMPLUGIN_ACCESS_INTERFACE_FACTORY_MIMETYPE;
        Oscl_Vector<PVUuid, OsclMemAllocator> uuIds;
        int retCmdId = kmjPlugin->QueryUUID( 0,
                                             mimeType,
                                             uuIds,
                                             false,
                                             NULL);

        if(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START == retCmdId)
        {
            PVMFCPMKmjPlugInOMA1Command* cmd = kmjPlugin->iInputCommands.FindById(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START);

            if(NULL == cmd)
            {
                PVPATB_TEST_IS_TRUE(false);
            }
            else
            {
                kmjPlugin->DoQueryUuid(*cmd);

                if(1 == uuIds.size())
                {
                    PVPATB_TEST_IS_TRUE(true);
                }
                else
                {
                    PVPATB_TEST_IS_TRUE(false);
                }
            }
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_DoQueryUuid_5()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        OSCL_HeapString<OsclMemAllocator> mimeType = PVMF_CPMPLUGIN_LICENSE_INTERFACE_MIMETYPE;
        Oscl_Vector<PVUuid, OsclMemAllocator> uuIds;
        int retCmdId = kmjPlugin->QueryUUID( 0,
                                             mimeType,
                                             uuIds,
                                             false,
                                             NULL);

        if(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START == retCmdId)
        {
            PVMFCPMKmjPlugInOMA1Command* cmd = kmjPlugin->iInputCommands.FindById(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START);

            if(NULL == cmd)
            {
                PVPATB_TEST_IS_TRUE(false);
            }
            else
            {
                kmjPlugin->DoQueryUuid(*cmd);

                if(1 == uuIds.size())
                {
                    PVPATB_TEST_IS_TRUE(true);
                }
                else
                {
                    PVPATB_TEST_IS_TRUE(false);
                }
            }
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_DoQueryUuid_6()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        OSCL_HeapString<OsclMemAllocator> mimeType = PVMF_META_DATA_EXTENSION_INTERFACE_MIMETYPE;
        Oscl_Vector<PVUuid, OsclMemAllocator> uuIds;
        int retCmdId = kmjPlugin->QueryUUID( 0,
                                             mimeType,
                                             uuIds,
                                             false,
                                             NULL);

        if(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START == retCmdId)
        {
            PVMFCPMKmjPlugInOMA1Command* cmd = kmjPlugin->iInputCommands.FindById(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START);

            if(NULL == cmd)
            {
                PVPATB_TEST_IS_TRUE(false);
            }
            else
            {
                kmjPlugin->DoQueryUuid(*cmd);

                if(1 == uuIds.size())
                {
                    PVPATB_TEST_IS_TRUE(true);
                }
                else
                {
                    PVPATB_TEST_IS_TRUE(false);
                }
            }
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_QueryInterface()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        PVInterface* ptrPvInterface = NULL;
        int retCmdId = kmjPlugin->QueryInterface( 0,
                                                 PVMFCPMPluginAuthorizationInterfaceUuid,
                                                 ptrPvInterface,
                                                 NULL);

        if(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START == retCmdId)
        {
            PVPATB_TEST_IS_TRUE(true);
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_DoQueryInterface_1()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        PVInterface* ptrPvInterface = NULL;
        int retCmdId = kmjPlugin->QueryInterface( 0,
                                                 PVMFCPMPluginAuthorizationInterfaceUuid,
                                                 ptrPvInterface,
                                                 NULL);

        if(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START == retCmdId)
        {
            PVMFCPMKmjPlugInOMA1Command* cmd = kmjPlugin->iInputCommands.FindById(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START);

            if(NULL == cmd)
            {
                PVPATB_TEST_IS_TRUE(false);
            }
            else
            {
                kmjPlugin->m_oSourceSet = false;
                kmjPlugin->DoQueryInterface(*cmd);

                if(PVMFErrInvalidState == kmjPlugin->cmdStatus)
                {
                    PVPATB_TEST_IS_TRUE(true);
                }
                else
                {
                    PVPATB_TEST_IS_TRUE(false);
                }
            }
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_DoQueryInterface_2()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        PVInterface* ptrPvInterface = NULL;
        int retCmdId = kmjPlugin->QueryInterface( 0,
                                                 PVMFCPMPluginAccessInterfaceFactoryUuid,
                                                 ptrPvInterface,
                                                 NULL);

        if(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START == retCmdId)
        {
            PVMFCPMKmjPlugInOMA1Command* cmd = kmjPlugin->iInputCommands.FindById(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START);

            if(NULL == cmd)
            {
                PVPATB_TEST_IS_TRUE(false);
            }
            else
            {
                kmjPlugin->m_oSourceSet = false;
                kmjPlugin->DoQueryInterface(*cmd);

                if(PVMFErrInvalidState == kmjPlugin->cmdStatus)
                {
                    PVPATB_TEST_IS_TRUE(true);
                }
                else
                {
                    PVPATB_TEST_IS_TRUE(false);
                }
            }
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_DoQueryInterface_3()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        kmjPlugin->m_oSourceSet == false;
        PVInterface* ptrPvInterface = NULL;
        int retCmdId = kmjPlugin->QueryInterface( 0,
                                                 PVMFCPMPluginContentManagementInterfaceUuid,
                                                 ptrPvInterface,
                                                 NULL);

        if(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START == retCmdId)
        {
            PVMFCPMKmjPlugInOMA1Command* cmd = kmjPlugin->iInputCommands.FindById(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START);

            if(NULL == cmd)
            {
                PVPATB_TEST_IS_TRUE(false);
            }
            else
            {
                kmjPlugin->DoQueryInterface(*cmd);

                if(PVMFErrNotSupported == kmjPlugin->cmdStatus)
                {
                    PVPATB_TEST_IS_TRUE(true);
                }
                else
                {
                    PVPATB_TEST_IS_TRUE(false);
                }
            }
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_DoQueryInterface_4()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        kmjPlugin->m_oSourceSet == false;
        PVInterface* ptrPvInterface = NULL;
        int retCmdId = kmjPlugin->QueryInterface( 0,
                                                 PVMFCPMPluginAuthenticationInterfaceUuid,
                                                 ptrPvInterface,
                                                 NULL);

        if(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START == retCmdId)
        {
            PVMFCPMKmjPlugInOMA1Command* cmd = kmjPlugin->iInputCommands.FindById(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START);

            if(NULL == cmd)
            {
                PVPATB_TEST_IS_TRUE(false);
            }
            else
            {
                kmjPlugin->DoQueryInterface(*cmd);

                if(PVMFSuccess == kmjPlugin->cmdStatus)
                {
                    if(ptrPvInterface)
                    {
                        PVPATB_TEST_IS_TRUE(true);
                    }
                    else
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }
                }
                else
                {
                    PVPATB_TEST_IS_TRUE(false);
                }
            }
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_DoQueryInterface_5()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        kmjPlugin->m_oSourceSet == false;
        PVInterface* ptrPvInterface = NULL;
        int retCmdId = kmjPlugin->QueryInterface( 0,
                                                 PVMFCPMPluginAuthorizationInterfaceUuid,
                                                 ptrPvInterface,
                                                 NULL);

        if(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START == retCmdId)
        {
            PVMFCPMKmjPlugInOMA1Command* cmd = kmjPlugin->iInputCommands.FindById(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START);

            if(NULL == cmd)
            {
                PVPATB_TEST_IS_TRUE(false);
            }
            else
            {
                kmjPlugin->DoQueryInterface(*cmd);

                if(PVMFSuccess == kmjPlugin->cmdStatus)
                {
                    if(ptrPvInterface)
                    {
                        PVPATB_TEST_IS_TRUE(true);
                    }
                    else
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }
                }
                else
                {
                    PVPATB_TEST_IS_TRUE(false);
                }
            }
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_DoQueryInterface_6()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        kmjPlugin->m_oSourceSet == false;
        PVInterface* ptrPvInterface = NULL;
        int retCmdId = kmjPlugin->QueryInterface( 0,
                                                 PVMFCPMPluginAccessInterfaceFactoryUuid,
                                                 ptrPvInterface,
                                                 NULL);

        if(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START == retCmdId)
        {
            PVMFCPMKmjPlugInOMA1Command* cmd = kmjPlugin->iInputCommands.FindById(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START);

            if(NULL == cmd)
            {
                PVPATB_TEST_IS_TRUE(false);
            }
            else
            {
                kmjPlugin->DoQueryInterface(*cmd);

                if(PVMFSuccess == kmjPlugin->cmdStatus)
                {
                    if(ptrPvInterface)
                    {
                        PVPATB_TEST_IS_TRUE(true);
                    }
                    else
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }
                }
                else
                {
                    PVPATB_TEST_IS_TRUE(false);
                }
            }
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_DoQueryInterface_7()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        kmjPlugin->m_oSourceSet == false;
        PVInterface* ptrPvInterface = NULL;
        int retCmdId = kmjPlugin->QueryInterface( 0,
                                                 PVMFCPMPluginLicenseInterfaceUuid,
                                                 ptrPvInterface,
                                                 NULL);

        if(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START == retCmdId)
        {
            PVMFCPMKmjPlugInOMA1Command* cmd = kmjPlugin->iInputCommands.FindById(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START);

            if(NULL == cmd)
            {
                PVPATB_TEST_IS_TRUE(false);
            }
            else
            {
                kmjPlugin->DoQueryInterface(*cmd);

                if(PVMFSuccess == kmjPlugin->cmdStatus)
                {
                    if(ptrPvInterface)
                    {
                        PVPATB_TEST_IS_TRUE(true);
                    }
                    else
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }
                }
                else
                {
                    PVPATB_TEST_IS_TRUE(false);
                }
            }
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_DoQueryInterface_8()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        kmjPlugin->m_oSourceSet == false;
        PVInterface* ptrPvInterface = NULL;
        int retCmdId = kmjPlugin->QueryInterface( 0,
                                                  KPVMFMetadataExtensionUuid,
                                                  ptrPvInterface,
                                                  NULL);

        if(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START == retCmdId)
        {
            PVMFCPMKmjPlugInOMA1Command* cmd = kmjPlugin->iInputCommands.FindById(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START);

            if(NULL == cmd)
            {
                PVPATB_TEST_IS_TRUE(false);
            }
            else
            {
                kmjPlugin->DoQueryInterface(*cmd);

                if(PVMFSuccess == kmjPlugin->cmdStatus)
                {
                    if(ptrPvInterface)
                    {
                        PVPATB_TEST_IS_TRUE(true);
                    }
                    else
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }
                }
                else
                {
                    PVPATB_TEST_IS_TRUE(false);
                }
            }
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}
void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_AuthorizeUsage()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        uint32_t requestTime = 1000;
        PvmiKvp requestUsage = {(char*)"request",1,1,{0}};
        requestUsage.value.uint32_value = BITMASK_PVMF_CPM_DRM_INTENT_PLAY;

        PvmiKvp approveUsage = {(char*)"approve",1,1,{0}};
        approveUsage.value.uint32_value = BITMASK_PVMF_CPM_DRM_INTENT_PLAY;

        PvmiKvp authorizeData = {(char*)"authorize",1,1,{0}};
        approveUsage.value.uint32_value = BITMASK_PVMF_CPM_DRM_INTENT_PLAY;

        int retCmdId = kmjPlugin->AuthorizeUsage( 0,
                                                  requestUsage,
                                                  approveUsage,
                                                  authorizeData,
                                                  requestTime,
                                                  NULL);

        if(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START == retCmdId)
        {
             PVPATB_TEST_IS_TRUE(true);
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_UsageComplete()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        int retCmdId = kmjPlugin->UsageComplete( 0,NULL);

        if(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START == retCmdId)
        {
             PVPATB_TEST_IS_TRUE(true);
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_DoAuthorizeUsage_1(void)
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        uint32_t requestTime = 1000;
        PvmiKvp requestUsage = {(char*)"request",1,1,{0}};
        requestUsage.value.uint32_value = BITMASK_PVMF_CPM_DRM_INTENT_PREVIEW;

        PvmiKvp approveUsage = {(char*)"approve",1,1,{0}};
        approveUsage.value.uint32_value = BITMASK_PVMF_CPM_DRM_INTENT_PLAY;

        PvmiKvp authorizeData = {(char*)"authorize",1,1,{0}};
        approveUsage.value.uint32_value = BITMASK_PVMF_CPM_DRM_INTENT_PLAY;

        int retCmdId = kmjPlugin->AuthorizeUsage( 0,
                                                  requestUsage,
                                                  approveUsage,
                                                  authorizeData,
                                                  requestTime,
                                                  NULL);

        if(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START == retCmdId)
        {
            PVMFCPMKmjPlugInOMA1Command* cmd = kmjPlugin->iInputCommands.FindById(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START);

            if(NULL == cmd)
            {
                PVPATB_TEST_IS_TRUE(false);
            }
            else
            {
                int32_t drmMethod = SVC_drm_getDeliveryMethod(kmjPlugin->m_drmSession);

                kmjPlugin->DoAuthorizeUsage(*cmd);

                if(FORWARD_LOCK == drmMethod)
                {
                    if(PVMFSuccess == kmjPlugin->cmdStatus)
                    {
                        PVPATB_TEST_IS_TRUE(true);
                    }
                    else
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }
                }
                else
                {
                    if( PVMFFailure == kmjPlugin->cmdStatus
                        || PVMFErrLicenseRequired == kmjPlugin->cmdStatus
                        || PVMFErrAccessDenied == kmjPlugin->cmdStatus)
                    {
                        PVPATB_TEST_IS_TRUE(true);
                    }
                    else
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }
                }
            }
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_DoAuthorizeUsage_2(void)
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        uint32_t requestTime = 1000;
        PvmiKvp requestUsage = {(char*)"request",1,1,{0}};
        requestUsage.value.uint32_value = BITMASK_PVMF_CPM_DRM_INTENT_EXECUTE;

        PvmiKvp approveUsage = {(char*)"approve",1,1,{0}};
        approveUsage.value.uint32_value = BITMASK_PVMF_CPM_DRM_INTENT_PLAY;

        PvmiKvp authorizeData = {(char*)"authorize",1,1,{0}};
        approveUsage.value.uint32_value = BITMASK_PVMF_CPM_DRM_INTENT_PLAY;

        int retCmdId = kmjPlugin->AuthorizeUsage( 0,
                                                  requestUsage,
                                                  approveUsage,
                                                  authorizeData,
                                                  requestTime,
                                                  NULL);

        if(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START == retCmdId)
        {
            PVMFCPMKmjPlugInOMA1Command* cmd = kmjPlugin->iInputCommands.FindById(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START);

            if(NULL == cmd)
            {
                PVPATB_TEST_IS_TRUE(false);
            }
            else
            {
                int32_t drmMethod = SVC_drm_getDeliveryMethod(kmjPlugin->m_drmSession);

                kmjPlugin->DoAuthorizeUsage(*cmd);

                if(FORWARD_LOCK == drmMethod)
                {
                    if(PVMFSuccess == kmjPlugin->cmdStatus)
                    {
                        PVPATB_TEST_IS_TRUE(true);
                    }
                    else
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }

                }
                else
                {
                     if( PVMFFailure == kmjPlugin->cmdStatus
                        || PVMFErrLicenseRequired == kmjPlugin->cmdStatus
                        || PVMFErrAccessDenied == kmjPlugin->cmdStatus)
                    {
                        PVPATB_TEST_IS_TRUE(true);
                    }
                    else
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }
                }
            }
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_DoAuthorizeUsage_3(void)
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        uint32_t requestTime = 1000;
        PvmiKvp requestUsage = {(char*)"request",1,1,{0}};
        requestUsage.value.uint32_value = BITMASK_PVMF_CPM_DRM_INTENT_PRINT;

        PvmiKvp approveUsage = {(char*)"approve",1,1,{0}};
        approveUsage.value.uint32_value = BITMASK_PVMF_CPM_DRM_INTENT_PLAY;

        PvmiKvp authorizeData = {(char*)"authorize",1,1,{0}};
        approveUsage.value.uint32_value = BITMASK_PVMF_CPM_DRM_INTENT_PLAY;

        int retCmdId = kmjPlugin->AuthorizeUsage( 0,
                                                  requestUsage,
                                                  approveUsage,
                                                  authorizeData,
                                                  requestTime,
                                                  NULL);

        if(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START == retCmdId)
        {
            PVMFCPMKmjPlugInOMA1Command* cmd = kmjPlugin->iInputCommands.FindById(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START);

            if(NULL == cmd)
            {
                PVPATB_TEST_IS_TRUE(false);
            }
            else
            {
                int32_t drmMethod = SVC_drm_getDeliveryMethod(kmjPlugin->m_drmSession);

                kmjPlugin->DoAuthorizeUsage(*cmd);

                if(FORWARD_LOCK == drmMethod)
                {
                    if(PVMFSuccess == kmjPlugin->cmdStatus)
                    {
                        PVPATB_TEST_IS_TRUE(true);
                    }
                    else
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }

                }
                else
                {
                    if( PVMFFailure == kmjPlugin->cmdStatus
                        || PVMFErrLicenseRequired == kmjPlugin->cmdStatus
                        || PVMFErrAccessDenied == kmjPlugin->cmdStatus)
                    {
                        PVPATB_TEST_IS_TRUE(true);
                    }
                    else
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }
                }
            }
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_DoAuthorizeUsage_4(void)
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        uint32_t requestTime = 1000;
        PvmiKvp requestUsage = {(char*)"request",1,1,{0}};
        requestUsage.value.uint32_value = ( BITMASK_PVMF_CPM_DRM_INTENT_PLAY );

        PvmiKvp approveUsage = {(char*)"approve",1,1,{0}};
        approveUsage.value.uint32_value = BITMASK_PVMF_CPM_DRM_INTENT_PLAY;

        PvmiKvp authorizeData = {(char*)"authorize",1,1,{0}};
        approveUsage.value.uint32_value = BITMASK_PVMF_CPM_DRM_INTENT_PLAY;

        int retCmdId = kmjPlugin->AuthorizeUsage( 0,
                                                  requestUsage,
                                                  approveUsage,
                                                  authorizeData,
                                                  requestTime,
                                                  NULL);

        if(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START == retCmdId)
        {
            PVMFCPMKmjPlugInOMA1Command* cmd = kmjPlugin->iInputCommands.FindById(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START);

            if(NULL == cmd)
            {
                PVPATB_TEST_IS_TRUE(false);
            }
            else
            {
                int32_t drmMethod = SVC_drm_getDeliveryMethod(kmjPlugin->m_drmSession);
                int32_t retCheckRights = SVC_drm_checkRights(kmjPlugin->m_drmSession,DRM_PERMISSION_PLAY);
                kmjPlugin->DoAuthorizeUsage(*cmd);

                if(DRM_SUCCESS == retCheckRights)
                {
                    if(PVMFSuccess == kmjPlugin->cmdStatus)
                    {
                        PVPATB_TEST_IS_TRUE(true);
                    }
                    else
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }
                }
                else
                {
                    if( PVMFFailure == kmjPlugin->cmdStatus
                        || PVMFErrLicenseRequired == kmjPlugin->cmdStatus
                        || PVMFErrAccessDenied == kmjPlugin->cmdStatus)
                    {
                        PVPATB_TEST_IS_TRUE(true);
                    }
                    else
                    {
                        printf("cmd status = %d\r\n",kmjPlugin->cmdStatus);
                        PVPATB_TEST_IS_TRUE(false);
                    }
                }
            }
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_ParseUrlHost(void)
{
    char* url = (char*)"http://starfish.esmertec.com.cn/tc/drm/Others/mp4_sd_ip.dr";

    if(kmjPlugin)
    {
        kmjPlugin->ParseUrl(url);
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
        return;
    }

    if( strncmp( kmjPlugin->urlHost.get_cstr(),
                 "starfish.esmertec.com.cn",
                 strlen("starfish.esmertec.com.cn")))
    {
        PVPATB_TEST_IS_TRUE(false);
        return;
    }

    if(kmjPlugin->urlPort != 80)
    {
        PVPATB_TEST_IS_TRUE(false);
        return;
    }

    if( strncmp( kmjPlugin->urlPath.get_cstr(),
                 "/tc/drm/Others/mp4_sd_ip.dr",
                 strlen("/tc/drm/Others/mp4_sd_ip.dr")))
    {
        PVPATB_TEST_IS_TRUE(false);
        return;
    }

    PVPATB_TEST_IS_TRUE(true);
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_ParseUrlIp(void)
{
    char* url = (char*)"http://221.10.65.38:8080/tc/drm/Others/mp4_sd_ip.dr";

    if(kmjPlugin)
    {
        kmjPlugin->ParseUrl(url);
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
        return;
    }

    if( strncmp( kmjPlugin->urlHost.get_cstr(),
                 "221.10.65.38",
                 strlen("221.10.65.38")))
    {
        PVPATB_TEST_IS_TRUE(false);
        return;
    }

    if(kmjPlugin->urlPort != 8080)
    {
        PVPATB_TEST_IS_TRUE(false);
        return;
    }

    if( strncmp( kmjPlugin->urlPath.get_cstr(),
                 "/tc/drm/Others/mp4_sd_ip.dr",
                 strlen("/tc/drm/Others/mp4_sd_ip.dr")))
    {
        PVPATB_TEST_IS_TRUE(false);
        return;
    }

    PVPATB_TEST_IS_TRUE(true);
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_GetLicense()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        OSCL_HeapString<OsclMemAllocator> contentName;

        uint8_t licensData[1024] = {0};

        int retCmdId = kmjPlugin->GetLicense( 0,
                                              contentName,
                                              licensData,
                                              sizeof(licensData),
                                              1000,
                                              NULL);

        if(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START == retCmdId)
        {
             PVPATB_TEST_IS_TRUE(true);
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_GetLicense_w()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        OSCL_wHeapString<OsclMemAllocator> contentName;
        uint8_t licensData[1024] = {0};

        int retCmdId = kmjPlugin->GetLicense( 0,
                                              contentName,
                                              licensData,
                                              sizeof(licensData),
                                              1000,
                                              NULL);

        if(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START == retCmdId)
        {
            PVPATB_TEST_IS_TRUE(true);
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_DoGetLicense(void)
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        OSCL_wHeapString<OsclMemAllocator> contentName;
        uint8_t licensData[1024] = {0};

        int retCmdId = kmjPlugin->GetLicense( 0,
                                              contentName,
                                              licensData,
                                              sizeof(licensData),
                                              1000,
                                              NULL);

        if(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START == retCmdId)
        {
            PVMFCPMKmjPlugInOMA1Command* cmd = kmjPlugin->iInputCommands.FindById(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START);

            if(NULL == cmd)
            {
                PVPATB_TEST_IS_TRUE(false);
            }
            else
            {
                char url[256] = {0};

                // get rights url
                if(DRM_SUCCESS != SVC_drm_getRightsIssuer(kmjPlugin->m_drmSession,(uint8_t*)url))
                {
                    kmjPlugin->DoGetLicense(*cmd,true);

                    if(PVMFFailure == kmjPlugin->cmdStatus)
                    {
                        PVPATB_TEST_IS_TRUE(true);
                    }
                    else
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }
                }
                else
                {
                    kmjPlugin->DoGetLicense(*cmd,true);

                    if(PVMFSuccess == kmjPlugin->cmdStatus)
                    {
                        PVPATB_TEST_IS_TRUE(true);
                    }
                    else
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }
                }
            }
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_GetNumMetadataKeys_1()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    T_DRM_Rights_Info rightsInfo;
    oscl_memset(&rightsInfo,0,sizeof(T_DRM_Rights_Info));

    int32_t ret = SVC_drm_getRightsInfo(kmjPlugin->m_drmSession,&rightsInfo);
    if(FALSE == ret || 0 == rightsInfo.playRights.indicator)
    {
        PVPATB_TEST_IS_TRUE(true);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        uint32_t retNum = kmjPlugin->GetNumMetadataKeys(NULL);
        if(numKeys == retNum)
        {
             PVPATB_TEST_IS_TRUE(true);
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_GetNumMetadataKeys_2()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    T_DRM_Rights_Info rightsInfo;
    oscl_memset(&rightsInfo,0,sizeof(T_DRM_Rights_Info));

    int32_t ret = SVC_drm_getRightsInfo(kmjPlugin->m_drmSession,&rightsInfo);
    if(FALSE == ret || 0 == rightsInfo.playRights.indicator)
    {
        PVPATB_TEST_IS_TRUE(true);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        if(1 == kmjPlugin->GetNumMetadataKeys((char*)PVMF_DRM_INFO_LICENSE_TYPE_QUERY))
        {
             PVPATB_TEST_IS_TRUE(true);
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_GetNumMetadataKeys_3()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    T_DRM_Rights_Info rightsInfo;
    oscl_memset(&rightsInfo,0,sizeof(T_DRM_Rights_Info));

    int32_t ret = SVC_drm_getRightsInfo(kmjPlugin->m_drmSession,&rightsInfo);
    if(FALSE == ret || 0 == rightsInfo.playRights.indicator)
    {
        PVPATB_TEST_IS_TRUE(true);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        char* keyString = (char*)kmjPlugin->GetLicenseType();

        if(NULL == keyString)
        {
            PVPATB_TEST_IS_TRUE(false);
            ResetAfterOneCaseComplete();
            return;
        }

        bool licenseUnlimited = false;
        bool licenseTime = false;
        bool licenseDuration = false;
        bool licenseCount = false;
        bool licenseTimeCount = false;
        bool licenseDurationCount = false;

        if(strstr(keyString,"unlimited"))
        {
            licenseUnlimited = true;
        }
        else if(strstr(keyString,"time"))
        {
            licenseTime = true;
        }
        else if(strstr(keyString,"duration"))
        {
            licenseDuration = true;
        }
        else if(strstr(keyString,"count"))
        {
            licenseCount = true;
        }
        else if(strstr(keyString,"time-count"))
        {
            licenseTimeCount = true;
        }
        else if(strstr(keyString,"duration-count"))
        {
            licenseDurationCount = true;
        }

        uint32_t retNumKeys = 0;

        if(false == licenseUnlimited)
        {
      if(false == licenseTime)
      {
              retNumKeys = kmjPlugin->GetNumMetadataKeys((char*)PVMF_DRM_INFO_LICENSE_EXPIRATION_TIME_QUERY);
      }
      else if(false == licenseCount)
      {
        retNumKeys = kmjPlugin->GetNumMetadataKeys((char*)PVMF_DRM_INFO_LICENSE_COUNT_QUERY);
      }
      else if(false == licenseDuration)
      {
        retNumKeys = kmjPlugin->GetNumMetadataKeys((char*)PVMF_DRM_INFO_LICENSE_DURATION_SINCE_FIRST_USE_QUERY);
      }
      else if(false == licenseTimeCount)
      {
        retNumKeys = kmjPlugin->GetNumMetadataKeys((char*)PVMF_DRM_INFO_LICENSE_COUNT_QUERY);
      }
      else if(false == licenseDurationCount)
      {
        retNumKeys = kmjPlugin->GetNumMetadataKeys((char*)PVMF_DRM_INFO_LICENSE_COUNT_QUERY);
      }

            if(0 == retNumKeys)
            {
                PVPATB_TEST_IS_TRUE(true);
            }
            else
            {
                PVPATB_TEST_IS_TRUE(false);
            }
        }

        if(false == licenseTime)
        {
            retNumKeys = kmjPlugin->GetNumMetadataKeys((char*)PVMF_DRM_INFO_LICENSE_START_TIME_QUERY);

            if(0 == retNumKeys)
            {
                PVPATB_TEST_IS_TRUE(true);
            }
            else
            {
                PVPATB_TEST_IS_TRUE(false);
            }
        }

        if(false == licenseDuration)
        {
            retNumKeys = kmjPlugin->GetNumMetadataKeys((char*)PVMF_DRM_INFO_LICENSE_DURATION_SINCE_FIRST_USE_QUERY);

            if(0 == retNumKeys)
            {
                PVPATB_TEST_IS_TRUE(true);
            }
            else
            {
                PVPATB_TEST_IS_TRUE(false);
            }
        }

        if(false == licenseCount)
        {
            retNumKeys = kmjPlugin->GetNumMetadataKeys((char*)PVMF_DRM_INFO_LICENSE_COUNT_QUERY);

            if(0 == retNumKeys)
            {
                PVPATB_TEST_IS_TRUE(true);
            }
            else
            {
                PVPATB_TEST_IS_TRUE(false);
            }
        }

        if(false == licenseTimeCount)
        {
            if(false == licenseCount)
            {
                retNumKeys = kmjPlugin->GetNumMetadataKeys((char*)PVMF_DRM_INFO_LICENSE_COUNT_QUERY);
            }
            else
            {
                retNumKeys = kmjPlugin->GetNumMetadataKeys((char*)PVMF_DRM_INFO_LICENSE_START_TIME_QUERY);
            }

            if(0 == retNumKeys)
            {
                PVPATB_TEST_IS_TRUE(true);
            }
            else
            {
                PVPATB_TEST_IS_TRUE(false);
            }
        }

        if(false == licenseDurationCount)
        {
            if(false == licenseCount)
            {
                retNumKeys = kmjPlugin->GetNumMetadataKeys((char*)PVMF_DRM_INFO_LICENSE_COUNT_QUERY);
            }
            else
            {
                retNumKeys = kmjPlugin->GetNumMetadataKeys((char*)PVMF_DRM_INFO_LICENSE_START_TIME_QUERY);
            }

            if(0 == retNumKeys)
            {
                PVPATB_TEST_IS_TRUE(true);
            }
            else
            {
                PVPATB_TEST_IS_TRUE(false);
            }
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_GetNumMetadataValules_1()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    T_DRM_Rights_Info rightsInfo;
    oscl_memset(&rightsInfo,0,sizeof(T_DRM_Rights_Info));

    int32_t ret = SVC_drm_getRightsInfo(kmjPlugin->m_drmSession,&rightsInfo);
    if(FALSE == ret || 0 == rightsInfo.playRights.indicator)
    {
        PVPATB_TEST_IS_TRUE(true);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        if(numKeys == kmjPlugin->GetNumMetadataValues(iAvailableMetadataKeys))
        {
             PVPATB_TEST_IS_TRUE(true);
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_GetNumMetadataValules_2()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    T_DRM_Rights_Info rightsInfo;
    oscl_memset(&rightsInfo,0,sizeof(T_DRM_Rights_Info));

    int32_t ret = SVC_drm_getRightsInfo(kmjPlugin->m_drmSession,&rightsInfo);
    if(FALSE == ret || 0 == rightsInfo.playRights.indicator)
    {
        PVPATB_TEST_IS_TRUE(true);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        Oscl_Vector<OSCL_HeapString<OsclMemAllocator>, OsclMemAllocator>  metaKeys;
        metaKeys.push_back(PVMF_DRM_INFO_IS_LICENSE_AVAILABLE_QUERY);

        if(1 == kmjPlugin->GetNumMetadataValues(metaKeys))
        {
             PVPATB_TEST_IS_TRUE(true);
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_GetNumMetadataValules_3()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    T_DRM_Rights_Info rightsInfo;
    oscl_memset(&rightsInfo,0,sizeof(T_DRM_Rights_Info));

    int32_t ret = SVC_drm_getRightsInfo(kmjPlugin->m_drmSession,&rightsInfo);
    if(FALSE == ret || 0 == rightsInfo.playRights.indicator)
    {
        PVPATB_TEST_IS_TRUE(true);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        char* keyString = (char*)kmjPlugin->GetLicenseType();

        if(NULL == keyString)
        {
            PVPATB_TEST_IS_TRUE(false);
            ResetAfterOneCaseComplete();
            return;
        }

        bool licenseUnlimited = false;
        bool licenseTime = false;
        bool licenseDuration = false;
        bool licenseCount = false;
        bool licenseTimeCount = false;
        bool licenseDurationCount = false;

        if(strstr(keyString,"unlimited"))
        {
            licenseUnlimited = true;
        }
        else if(strstr(keyString,"time"))
        {
            licenseTime = true;
        }
        else if(strstr(keyString,"duration"))
        {
            licenseDuration = true;
        }
        else if(strstr(keyString,"count"))
        {
            licenseCount = true;
        }
        else if(strstr(keyString,"time-count"))
        {
            licenseTimeCount = true;
        }
        else if(strstr(keyString,"duration-count"))
        {
            licenseDurationCount = true;
        }

        Oscl_Vector<OSCL_HeapString<OsclMemAllocator>, OsclMemAllocator>  metaKeys;

        if(false == licenseUnlimited)
        {
      if(false == licenseTime)
      {
        metaKeys.push_back(PVMF_DRM_INFO_LICENSE_EXPIRATION_TIME_QUERY);
      }
      else if(false == licenseCount)
      {
        metaKeys.push_back(PVMF_DRM_INFO_LICENSE_COUNT_QUERY);
      }
      else if(false == licenseDuration)
      {
        metaKeys.push_back(PVMF_DRM_INFO_LICENSE_DURATION_SINCE_FIRST_USE_QUERY);
      }
      else if(false == licenseTimeCount)
      {
        metaKeys.push_back(PVMF_DRM_INFO_LICENSE_COUNT_QUERY);
      }
      else if(false == licenseDurationCount)
      {
        metaKeys.push_back(PVMF_DRM_INFO_LICENSE_COUNT_QUERY);
      }
        }

        if(false == licenseTime)
        {
            metaKeys.push_back(PVMF_DRM_INFO_LICENSE_START_TIME_QUERY);
        }

        if(false == licenseDuration)
        {
            metaKeys.push_back(PVMF_DRM_INFO_LICENSE_DURATION_SINCE_FIRST_USE_QUERY);
        }

        if(false == licenseCount)
        {
            metaKeys.push_back(PVMF_DRM_INFO_LICENSE_COUNT_QUERY);
        }

        if(false == licenseTimeCount)
        {
            if(false == licenseCount)
            {
                metaKeys.push_back(PVMF_DRM_INFO_LICENSE_COUNT_QUERY);
            }
            else
            {
                metaKeys.push_back(PVMF_DRM_INFO_LICENSE_START_TIME_QUERY);
            }
        }

        if(false == licenseDurationCount)
        {
            if(false == licenseCount)
            {
                metaKeys.push_back(PVMF_DRM_INFO_LICENSE_COUNT_QUERY);
            }
            else
            {
                metaKeys.push_back(PVMF_DRM_INFO_LICENSE_START_TIME_QUERY);
            }
        }

        uint32_t retNum = kmjPlugin->GetNumMetadataValues(metaKeys);
        if(0 == retNum)
        {
            PVPATB_TEST_IS_TRUE(true);
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_GetNodeMetadataKeys()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        PVMFMetadataList aKeyList;
        int retCmdId = kmjPlugin->GetNodeMetadataKeys( 0,
                                                       aKeyList,
                                                       0,
                                                       iAvailableMetadataKeys.size());

        if(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START == retCmdId)
        {
             PVPATB_TEST_IS_TRUE(true);
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_GetNodeMetadataValues()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        PVMFMetadataList aKeyList;
        Oscl_Vector<PvmiKvp,OsclMemAllocator> aValueList;

        int retCmdId = kmjPlugin->GetNodeMetadataValues( 0,
                                                         aKeyList,
                                                         aValueList,
                                                         0,
                                                         iAvailableMetadataKeys.size());

        if( PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START == retCmdId)
        {
             PVPATB_TEST_IS_TRUE(true);
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_DoGetMetadataKeys_1()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    T_DRM_Rights_Info rightsInfo;
    oscl_memset(&rightsInfo,0,sizeof(T_DRM_Rights_Info));

    int32_t ret = SVC_drm_getRightsInfo(kmjPlugin->m_drmSession,&rightsInfo);
    if(FALSE == ret || 0 == rightsInfo.playRights.indicator)
    {
        PVPATB_TEST_IS_TRUE(true);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        PVMFMetadataList aKeyList;
        int retCmdId = kmjPlugin->GetNodeMetadataKeys( 0,
                                                       aKeyList,
                                                       0,
                                                       -2);

        if(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START == retCmdId)
        {
            PVMFCPMKmjPlugInOMA1Command* cmd = kmjPlugin->iInputCommands.FindById(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START);

            if(NULL == cmd)
            {
                PVPATB_TEST_IS_TRUE(false);
            }
            else
            {
                kmjPlugin->DoGetMetadataKeys(*cmd);

                if(PVMFErrArgument == kmjPlugin->cmdStatus)
                {
                    PVPATB_TEST_IS_TRUE(true);
                }
                else
                {
                    PVPATB_TEST_IS_TRUE(false);
                }
            }
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_DoGetMetadataKeys_2()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    T_DRM_Rights_Info rightsInfo;
    oscl_memset(&rightsInfo,0,sizeof(T_DRM_Rights_Info));

    int32_t ret = SVC_drm_getRightsInfo(kmjPlugin->m_drmSession,&rightsInfo);
    if(FALSE == ret || 0 == rightsInfo.playRights.indicator)
    {
        PVPATB_TEST_IS_TRUE(true);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        PVMFMetadataList aKeyList;
        int retCmdId = kmjPlugin->GetNodeMetadataKeys( 0,
                                                       aKeyList,
                                                       1,
                                                       0);

        if(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START == retCmdId)
        {
            PVMFCPMKmjPlugInOMA1Command* cmd = kmjPlugin->iInputCommands.FindById(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START);

            if(NULL == cmd)
            {
                PVPATB_TEST_IS_TRUE(false);
            }
            else
            {
                kmjPlugin->DoGetMetadataKeys(*cmd);

                if(PVMFErrArgument == kmjPlugin->cmdStatus)
                {
                    PVPATB_TEST_IS_TRUE(true);
                }
                else
                {
                    PVPATB_TEST_IS_TRUE(false);
                }
            }
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_DoGetMetadataKeys_3()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    T_DRM_Rights_Info rightsInfo;
    oscl_memset(&rightsInfo,0,sizeof(T_DRM_Rights_Info));

    int32_t ret = SVC_drm_getRightsInfo(kmjPlugin->m_drmSession,&rightsInfo);
    if(FALSE == ret || 0 == rightsInfo.playRights.indicator)
    {
        PVPATB_TEST_IS_TRUE(true);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        PVMFMetadataList aKeyList;
        int retCmdId = kmjPlugin->GetNodeMetadataKeys( 0,
                                                       aKeyList,
                                                       iAvailableMetadataKeys.size(),
                                                       iAvailableMetadataKeys.size());

        if(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START == retCmdId)
        {
            PVMFCPMKmjPlugInOMA1Command* cmd = kmjPlugin->iInputCommands.FindById(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START);

            if(NULL == cmd)
            {
                PVPATB_TEST_IS_TRUE(false);
            }
            else
            {
                kmjPlugin->DoGetMetadataKeys(*cmd);

                if(PVMFErrArgument == kmjPlugin->cmdStatus)
                {
                    PVPATB_TEST_IS_TRUE(true);
                }
                else
                {
                    PVPATB_TEST_IS_TRUE(false);
                }
            }
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_DoGetMetadataKeys_4()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    T_DRM_Rights_Info rightsInfo;
    oscl_memset(&rightsInfo,0,sizeof(T_DRM_Rights_Info));

    int32_t ret = SVC_drm_getRightsInfo(kmjPlugin->m_drmSession,&rightsInfo);
    if(FALSE == ret || 0 == rightsInfo.playRights.indicator)
    {
        PVPATB_TEST_IS_TRUE(true);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        char* keyString = (char*)kmjPlugin->GetLicenseType();

        if(NULL == keyString)
        {
            PVPATB_TEST_IS_TRUE(false);
            ResetAfterOneCaseComplete();
            return;
        }

        bool licenseUnlimited = false;
        bool licenseTime = false;
        bool licenseDuration = false;
        bool licenseCount = false;
        bool licenseTimeCount = false;
        bool licenseDurationCount = false;

        if(strstr(keyString,"unlimited"))
        {
            licenseUnlimited = true;
        }
        else if(strstr(keyString,"time"))
        {
            licenseTime = true;
        }
        else if(strstr(keyString,"duration"))
        {
            licenseDuration = true;
        }
        else if(strstr(keyString,"count"))
        {
            licenseCount = true;
        }
        else if(strstr(keyString,"time-count"))
        {
            licenseTimeCount = true;
        }
        else if(strstr(keyString,"duration-count"))
        {
            licenseDurationCount = true;
        }

        int i = 0;

        if(false == licenseUnlimited)
        {
            PVMFMetadataList aKeyList;
            int retCmdId = kmjPlugin->GetNodeMetadataKeys( 0,
                                                           aKeyList,
                                                           0,
                                                           0,
                                                           (char*)PVMF_DRM_INFO_LICENSE_EXPIRATION_TIME_QUERY);

            if(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START == retCmdId)
            {
                PVMFCPMKmjPlugInOMA1Command* cmd = kmjPlugin->iInputCommands.FindById(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START);

                if(NULL == cmd)
                {
                    PVPATB_TEST_IS_TRUE(false);
                }
                else
                {
                    kmjPlugin->DoGetMetadataKeys(*cmd);

                    if(PVMFSuccess == kmjPlugin->cmdStatus)
                    {
                        if(0 == aKeyList.size())
                        {
                            PVPATB_TEST_IS_TRUE(true);
                        }
                        else
                        {
                            PVPATB_TEST_IS_TRUE(false);
                        }
                    }
                    else
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }
                }
            }
            else
            {
                PVPATB_TEST_IS_TRUE(false);
            }

            i++;
        }

        if(false == licenseTime)
        {
            PVMFMetadataList aKeyList;
            int retCmdId = kmjPlugin->GetNodeMetadataKeys( 0,
                                                           aKeyList,
                                                           0,
                                                           0,
                                                           (char*)PVMF_DRM_INFO_LICENSE_START_TIME_QUERY);
            if(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START + i == retCmdId)
            {
                PVMFCPMKmjPlugInOMA1Command* cmd = kmjPlugin->iInputCommands.FindById(retCmdId);

                if(NULL == cmd)
                {
                    PVPATB_TEST_IS_TRUE(false);
                }
                else
                {
                    kmjPlugin->DoGetMetadataKeys(*cmd);

                    if(PVMFSuccess == kmjPlugin->cmdStatus)
                    {
                        if(0 == aKeyList.size())
                        {
                            PVPATB_TEST_IS_TRUE(true);
                        }
                        else
                        {
                            PVPATB_TEST_IS_TRUE(false);
                        }
                    }
                    else
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }
                }
            }
            else
            {
                PVPATB_TEST_IS_TRUE(false);
            }

            i++;
        }

        if(false == licenseDuration)
        {
            PVMFMetadataList aKeyList;
            int retCmdId = kmjPlugin->GetNodeMetadataKeys( 0,
                                                           aKeyList,
                                                           0,
                                                           0,
                                                           (char*)PVMF_DRM_INFO_LICENSE_DURATION_SINCE_FIRST_USE_QUERY);
            if(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START + i == retCmdId)
            {
                PVMFCPMKmjPlugInOMA1Command* cmd = kmjPlugin->iInputCommands.FindById(retCmdId);

                if(NULL == cmd)
                {
                    PVPATB_TEST_IS_TRUE(false);
                }
                else
                {
                    kmjPlugin->DoGetMetadataKeys(*cmd);

                    if(PVMFSuccess == kmjPlugin->cmdStatus)
                    {
                        if(0 == aKeyList.size())
                        {
                            PVPATB_TEST_IS_TRUE(true);
                        }
                        else
                        {
                            PVPATB_TEST_IS_TRUE(false);
                        }
                    }
                    else
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }
                }
            }
            else
            {
                PVPATB_TEST_IS_TRUE(false);
            }

             i++;
        }

        if(false == licenseCount)
        {
            PVMFMetadataList aKeyList;
            int retCmdId = kmjPlugin->GetNodeMetadataKeys( 0,
                                                           aKeyList,
                                                           0,
                                                           0,
                                                           (char*)PVMF_DRM_INFO_LICENSE_COUNT_QUERY);
            if(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START + i == retCmdId)
            {
                PVMFCPMKmjPlugInOMA1Command* cmd = kmjPlugin->iInputCommands.FindById(retCmdId);

                if(NULL == cmd)
                {
                    PVPATB_TEST_IS_TRUE(false);
                }
                else
                {
                    kmjPlugin->DoGetMetadataKeys(*cmd);

                    if(PVMFSuccess == kmjPlugin->cmdStatus)
                    {
                        if(0 == aKeyList.size())
                        {
                            PVPATB_TEST_IS_TRUE(true);
                        }
                        else
                        {
                            PVPATB_TEST_IS_TRUE(false);
                        }
                    }
                    else
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }
                }
            }
            else
            {
                PVPATB_TEST_IS_TRUE(false);
            }

            i++;
        }

        if(false == licenseTimeCount)
        {
            if(false == licenseCount)
            {
                PVMFMetadataList aKeyList;
                int retCmdId = kmjPlugin->GetNodeMetadataKeys( 0,
                                                               aKeyList,
                                                               0,
                                                               0,
                                                               (char*)PVMF_DRM_INFO_LICENSE_COUNT_QUERY);

                if(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START + i == retCmdId)
                {
                    PVMFCPMKmjPlugInOMA1Command* cmd = kmjPlugin->iInputCommands.FindById(retCmdId);

                    if(NULL == cmd)
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }
                    else
                    {
                        kmjPlugin->DoGetMetadataKeys(*cmd);

                        if(PVMFSuccess == kmjPlugin->cmdStatus)
                        {
                            if(0 == aKeyList.size())
                            {
                                PVPATB_TEST_IS_TRUE(true);
                            }
                            else
                            {
                                PVPATB_TEST_IS_TRUE(false);
                            }
                        }
                        else
                        {
                            PVPATB_TEST_IS_TRUE(false);
                        }
                    }
                }
                else
                {
                    PVPATB_TEST_IS_TRUE(false);
                }

                i++;
            }
            else
            {
                PVMFMetadataList aKeyList;
                int retCmdId = kmjPlugin->GetNodeMetadataKeys( 0,
                                                               aKeyList,
                                                               0,
                                                               0,
                                                               (char*)PVMF_DRM_INFO_LICENSE_START_TIME_QUERY);

                if(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START + i == retCmdId)
                {
                    PVMFCPMKmjPlugInOMA1Command* cmd = kmjPlugin->iInputCommands.FindById(retCmdId);

                    if(NULL == cmd)
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }
                    else
                    {
                        kmjPlugin->DoGetMetadataKeys(*cmd);

                        if(PVMFSuccess == kmjPlugin->cmdStatus)
                        {
                            if(0 == aKeyList.size())
                            {
                                PVPATB_TEST_IS_TRUE(true);
                            }
                            else
                            {
                                PVPATB_TEST_IS_TRUE(false);
                            }
                        }
                        else
                        {
                            PVPATB_TEST_IS_TRUE(false);
                        }
                    }
                }
                else
                {
                    PVPATB_TEST_IS_TRUE(false);
                }

                i++;
            }
        }

        if(false == licenseDurationCount)
        {
            if(false == licenseCount)
            {
                PVMFMetadataList aKeyList;
                int retCmdId = kmjPlugin->GetNodeMetadataKeys( 0,
                                                               aKeyList,
                                                               0,
                                                               0,
                                                               (char*)PVMF_DRM_INFO_LICENSE_COUNT_QUERY);

                if(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START + i == retCmdId)
                {
                    PVMFCPMKmjPlugInOMA1Command* cmd = kmjPlugin->iInputCommands.FindById(retCmdId);

                    if(NULL == cmd)
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }
                    else
                    {
                        kmjPlugin->DoGetMetadataKeys(*cmd);

                        if(PVMFSuccess == kmjPlugin->cmdStatus)
                        {
                            if(0 == aKeyList.size())
                            {
                                PVPATB_TEST_IS_TRUE(true);
                            }
                            else
                            {
                                PVPATB_TEST_IS_TRUE(false);
                            }
                        }
                        else
                        {
                            PVPATB_TEST_IS_TRUE(false);
                        }
                    }
                }
                else
                {
                    PVPATB_TEST_IS_TRUE(false);
                }

                i++;
            }
            else
            {
                PVMFMetadataList aKeyList;
                int retCmdId = kmjPlugin->GetNodeMetadataKeys( 0,
                                                               aKeyList,
                                                               0,
                                                               0,
                                                               (char*)PVMF_DRM_INFO_LICENSE_START_TIME_QUERY);

                if(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START + i == retCmdId)
                {
                    PVMFCPMKmjPlugInOMA1Command* cmd = kmjPlugin->iInputCommands.FindById(retCmdId);

                    if(NULL == cmd)
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }
                    else
                    {
                        kmjPlugin->DoGetMetadataKeys(*cmd);

                        if(PVMFSuccess == kmjPlugin->cmdStatus)
                        {
                            if(0 == aKeyList.size())
                            {
                                PVPATB_TEST_IS_TRUE(true);
                            }
                            else
                            {
                                PVPATB_TEST_IS_TRUE(false);
                            }
                        }
                        else
                        {
                            PVPATB_TEST_IS_TRUE(false);
                        }
                    }
                }
                else
                {
                    PVPATB_TEST_IS_TRUE(false);
                }

                i++;
            }
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_DoGetMetadataKeys_5()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    T_DRM_Rights_Info rightsInfo;
    oscl_memset(&rightsInfo,0,sizeof(T_DRM_Rights_Info));

    int32_t ret = SVC_drm_getRightsInfo(kmjPlugin->m_drmSession,&rightsInfo);
    if(FALSE == ret || 0 == rightsInfo.playRights.indicator)
    {
        PVPATB_TEST_IS_TRUE(true);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        PVMFMetadataList aKeyList;
        int retCmdId = kmjPlugin->GetNodeMetadataKeys( 0,
                                                       aKeyList,
                                                       0,
                                                       0);

        if(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START == retCmdId)
        {
            PVMFCPMKmjPlugInOMA1Command* cmd = kmjPlugin->iInputCommands.FindById(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START);

            if(NULL == cmd)
            {
                PVPATB_TEST_IS_TRUE(false);
            }
            else
            {
                kmjPlugin->DoGetMetadataKeys(*cmd);

                if(PVMFSuccess == kmjPlugin->cmdStatus)
                {
                    if(1 == aKeyList.size())
                    {
                        PVPATB_TEST_IS_TRUE(true);
                    }
                    else
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }
                }
                else
                {
                    PVPATB_TEST_IS_TRUE(false);
                }
            }
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_DoGetMetadataKeys_6()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    T_DRM_Rights_Info rightsInfo;
    oscl_memset(&rightsInfo,0,sizeof(T_DRM_Rights_Info));

    int32_t ret = SVC_drm_getRightsInfo(kmjPlugin->m_drmSession,&rightsInfo);
    if(FALSE == ret || 0 == rightsInfo.playRights.indicator)
    {
        PVPATB_TEST_IS_TRUE(true);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        PVMFMetadataList aKeyList;
        int retCmdId = kmjPlugin->GetNodeMetadataKeys( 0,
                                                       aKeyList,
                                                       iAvailableMetadataKeys.size() - 1,
                                                       iAvailableMetadataKeys.size() - 1);

        if(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START == retCmdId)
        {
            PVMFCPMKmjPlugInOMA1Command* cmd = kmjPlugin->iInputCommands.FindById(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START);

            if(NULL == cmd)
            {
                PVPATB_TEST_IS_TRUE(false);
            }
            else
            {
                kmjPlugin->DoGetMetadataKeys(*cmd);

                if(PVMFSuccess == kmjPlugin->cmdStatus)
                {
                    if(1 == aKeyList.size())
                    {
                        PVPATB_TEST_IS_TRUE(true);
                    }
                    else
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }
                }
                else
                {
                    PVPATB_TEST_IS_TRUE(false);
                }
            }
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_DoGetMetadataKeys_7()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    T_DRM_Rights_Info rightsInfo;
    oscl_memset(&rightsInfo,0,sizeof(T_DRM_Rights_Info));

    int32_t ret = SVC_drm_getRightsInfo(kmjPlugin->m_drmSession,&rightsInfo);
    if(FALSE == ret || 0 == rightsInfo.playRights.indicator)
    {
        PVPATB_TEST_IS_TRUE(true);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        PVMFMetadataList aKeyList;
        int retCmdId = kmjPlugin->GetNodeMetadataKeys( 0,
                                                       aKeyList,
                                                       0,
                                                       -1,
                                                       (char*)PVMF_DRM_INFO_CAN_USE_AS_RINGTONE_QUERY);

        if(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START == retCmdId)
        {
            PVMFCPMKmjPlugInOMA1Command* cmd = kmjPlugin->iInputCommands.FindById(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START);

            if(NULL == cmd)
            {
                PVPATB_TEST_IS_TRUE(false);
            }
            else
            {
                kmjPlugin->DoGetMetadataKeys(*cmd);

                if(PVMFSuccess == kmjPlugin->cmdStatus)
                {
                    if(1 == aKeyList.size())
                    {
                        PVPATB_TEST_IS_TRUE(true);
                    }
                    else
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }
                }
                else
                {
                    PVPATB_TEST_IS_TRUE(false);
                }
            }
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_DoGetMetadataKeys_8()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    T_DRM_Rights_Info rightsInfo;
    oscl_memset(&rightsInfo,0,sizeof(T_DRM_Rights_Info));

    int32_t ret = SVC_drm_getRightsInfo(kmjPlugin->m_drmSession,&rightsInfo);
    if(FALSE == ret || 0 == rightsInfo.playRights.indicator)
    {
        PVPATB_TEST_IS_TRUE(true);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        PVMFMetadataList aKeyList;
        int retCmdId = kmjPlugin->GetNodeMetadataKeys( 0,
                                                       aKeyList,
                                                       0,
                                                       -1);

        if(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START == retCmdId)
        {
            PVMFCPMKmjPlugInOMA1Command* cmd = kmjPlugin->iInputCommands.FindById(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START);

            if(NULL == cmd)
            {
                PVPATB_TEST_IS_TRUE(false);
            }
            else
            {
                kmjPlugin->DoGetMetadataKeys(*cmd);

                if(PVMFSuccess == kmjPlugin->cmdStatus)
                {
                    if(iAvailableMetadataKeys.size() == aKeyList.size())
                    {
                        PVPATB_TEST_IS_TRUE(true);
                    }
                    else
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }
                }
                else
                {
                    PVPATB_TEST_IS_TRUE(false);
                }
            }
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_DoGetMetadataKeys_9()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    T_DRM_Rights_Info rightsInfo;
    oscl_memset(&rightsInfo,0,sizeof(T_DRM_Rights_Info));

    int32_t ret = SVC_drm_getRightsInfo(kmjPlugin->m_drmSession,&rightsInfo);
    if(FALSE == ret || 0 == rightsInfo.playRights.indicator)
    {
        PVPATB_TEST_IS_TRUE(true);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        PVMFMetadataList aKeyList;
        int retCmdId = kmjPlugin->GetNodeMetadataKeys( 0,
                                                       aKeyList,
                                                       0,
                                                       iAvailableMetadataKeys.size());

        if(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START == retCmdId)
        {
            PVMFCPMKmjPlugInOMA1Command* cmd = kmjPlugin->iInputCommands.FindById(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START);

            if(NULL == cmd)
            {
                PVPATB_TEST_IS_TRUE(false);
            }
            else
            {
                kmjPlugin->DoGetMetadataKeys(*cmd);

                if(PVMFSuccess == kmjPlugin->cmdStatus)
                {
                    if(iAvailableMetadataKeys.size() == aKeyList.size())
                    {
                        PVPATB_TEST_IS_TRUE(true);
                    }
                    else
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }
                }
                else
                {
                    PVPATB_TEST_IS_TRUE(false);
                }
            }
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_DoGetMetadataValues_1()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    T_DRM_Rights_Info rightsInfo;
    oscl_memset(&rightsInfo,0,sizeof(T_DRM_Rights_Info));

    int32_t ret = SVC_drm_getRightsInfo(kmjPlugin->m_drmSession,&rightsInfo);
    if(FALSE == ret || 0 == rightsInfo.playRights.indicator)
    {
        PVPATB_TEST_IS_TRUE(true);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        PVMFMetadataList aKeyList;
        Oscl_Vector<PvmiKvp,OsclMemAllocator> aValueList;

        int retCmdId = kmjPlugin->GetNodeMetadataValues( 0,
                                                         aKeyList,
                                                         aValueList,
                                                         0,
                                                         iAvailableMetadataKeys.size());

        if( PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START == retCmdId)
        {
            PVMFCPMKmjPlugInOMA1Command* cmd = kmjPlugin->iInputCommands.FindById(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START);

            if(NULL == cmd)
            {
                PVPATB_TEST_IS_TRUE(false);
            }
            else
            {
                kmjPlugin->DoGetMetadataValues(*cmd);

                if(PVMFErrArgument == kmjPlugin->cmdStatus)
                {
                    PVPATB_TEST_IS_TRUE(true);
                }
                else
                {
                    PVPATB_TEST_IS_TRUE(false);
                }
            }
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_DoGetMetadataValues_2()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    T_DRM_Rights_Info rightsInfo;
    oscl_memset(&rightsInfo,0,sizeof(T_DRM_Rights_Info));

    int32_t ret = SVC_drm_getRightsInfo(kmjPlugin->m_drmSession,&rightsInfo);
    if(FALSE == ret || 0 == rightsInfo.playRights.indicator)
    {
        PVPATB_TEST_IS_TRUE(true);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        PVMFMetadataList aKeyList;
        aKeyList.push_back(PVMF_DRM_INFO_IS_FORWARD_LOCKED_QUERY);
        Oscl_Vector<PvmiKvp,OsclMemAllocator> aValueList;

        int retCmdId = kmjPlugin->GetNodeMetadataValues( 0,
                                                         aKeyList,
                                                         aValueList,
                                                         2,
                                                         iAvailableMetadataKeys.size());

        if( PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START == retCmdId)
        {
            PVMFCPMKmjPlugInOMA1Command* cmd = kmjPlugin->iInputCommands.FindById(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START);

            if(NULL == cmd)
            {
                PVPATB_TEST_IS_TRUE(false);
            }
            else
            {
                kmjPlugin->DoGetMetadataValues(*cmd);

                if(PVMFErrArgument == kmjPlugin->cmdStatus)
                {
                    PVPATB_TEST_IS_TRUE(true);
                }
                else
                {
                    PVPATB_TEST_IS_TRUE(false);
                }
            }
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_DoGetMetadataValues_3()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    T_DRM_Rights_Info rightsInfo;
    oscl_memset(&rightsInfo,0,sizeof(T_DRM_Rights_Info));

    int32_t ret = SVC_drm_getRightsInfo(kmjPlugin->m_drmSession,&rightsInfo);
    if(FALSE == ret || 0 == rightsInfo.playRights.indicator)
    {
        PVPATB_TEST_IS_TRUE(true);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        PVMFMetadataList aKeyList;
        aKeyList.push_back(PVMF_DRM_INFO_IS_FORWARD_LOCKED_QUERY);
        aKeyList.push_back(PVMF_DRM_INFO_LICENSE_TYPE_QUERY);
        Oscl_Vector<PvmiKvp,OsclMemAllocator> aValueList;

        int retCmdId = kmjPlugin->GetNodeMetadataValues( 0,
                                                         aKeyList,
                                                         aValueList,
                                                         2,
                                                         1);

        if( PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START == retCmdId)
        {
            PVMFCPMKmjPlugInOMA1Command* cmd = kmjPlugin->iInputCommands.FindById(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START);

            if(NULL == cmd)
            {
                PVPATB_TEST_IS_TRUE(false);
            }
            else
            {
                kmjPlugin->DoGetMetadataValues(*cmd);

                if(PVMFErrArgument == kmjPlugin->cmdStatus)
                {
                    PVPATB_TEST_IS_TRUE(true);
                }
                else
                {
                    PVPATB_TEST_IS_TRUE(false);
                }
            }
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_DoGetMetadataValues_4()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    T_DRM_Rights_Info rightsInfo;
    oscl_memset(&rightsInfo,0,sizeof(T_DRM_Rights_Info));

    int32_t ret = SVC_drm_getRightsInfo(kmjPlugin->m_drmSession,&rightsInfo);
    if(FALSE == ret || 0 == rightsInfo.playRights.indicator)
    {
        PVPATB_TEST_IS_TRUE(true);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        PVMFMetadataList aKeyList;
        aKeyList.push_back(PVMF_DRM_INFO_IS_PROTECTED_QUERY);
        Oscl_Vector<PvmiKvp,OsclMemAllocator> aValueList;

        int retCmdId = kmjPlugin->GetNodeMetadataValues( 0,
                                                         aKeyList,
                                                         aValueList,
                                                         0,
                                                         1);

        if( PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START == retCmdId)
        {
            PVMFCPMKmjPlugInOMA1Command* cmd = kmjPlugin->iInputCommands.FindById(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START);

            if(NULL == cmd)
            {
                PVPATB_TEST_IS_TRUE(false);
            }
            else
            {
                kmjPlugin->DoGetMetadataValues(*cmd);

                if(PVMFSuccess == kmjPlugin->cmdStatus)
                {
                    if(1 == aValueList.size())
                    {
                        PvmiKvp KeyVal = aValueList[0];

                        if(0 == oscl_strcmp(KeyVal.key,PVMF_DRM_INFO_IS_PROTECTED_VALUE))
                        {
                            if( (1 == KeyVal.length) && (1 == KeyVal.capacity))
                            {
                                if(true == KeyVal.value.bool_value)
                                {
                                    PVPATB_TEST_IS_TRUE(true);
                                }
                                else
                                {
                                    PVPATB_TEST_IS_TRUE(false);
                                }
                            }
                            else
                            {
                                PVPATB_TEST_IS_TRUE(false);
                            }
                        }
                        else
                        {
                            PVPATB_TEST_IS_TRUE(false);
                        }
                    }
                    else
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }
                }
                else
                {
                    PVPATB_TEST_IS_TRUE(false);
                }
            }
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_DoGetMetadataValues_5()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    T_DRM_Rights_Info rightsInfo;
    oscl_memset(&rightsInfo,0,sizeof(T_DRM_Rights_Info));

    int32_t ret = SVC_drm_getRightsInfo(kmjPlugin->m_drmSession,&rightsInfo);
    if(FALSE == ret || 0 == rightsInfo.playRights.indicator)
    {
        PVPATB_TEST_IS_TRUE(true);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        PVMFMetadataList aKeyList;
        aKeyList.push_back(PVMF_DRM_INFO_IS_LICENSE_AVAILABLE_QUERY);
        Oscl_Vector<PvmiKvp,OsclMemAllocator> aValueList;

        int retCmdId = kmjPlugin->GetNodeMetadataValues( 0,
                                                         aKeyList,
                                                         aValueList,
                                                         0,
                                                         1);

        if( PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START == retCmdId)
        {
            PVMFCPMKmjPlugInOMA1Command* cmd = kmjPlugin->iInputCommands.FindById(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START);

            if(NULL == cmd)
            {
                PVPATB_TEST_IS_TRUE(false);
            }
            else
            {
                kmjPlugin->DoGetMetadataValues(*cmd);

                if(PVMFSuccess == kmjPlugin->cmdStatus)
                {
                    if(1 == aValueList.size())
                    {
                        PvmiKvp KeyVal = aValueList[0];

                        if(0 == oscl_strcmp(KeyVal.key,PVMF_DRM_INFO_IS_LICENSE_AVAILABLE_VALUE))
                        {
                            if( (1 == KeyVal.length) && (1 == KeyVal.capacity))
                            {
                                bool bIsLicenseAvailable = false;

                                if(TRUE == SVC_drm_checkRights(kmjPlugin->m_drmSession,DRM_PERMISSION_PLAY))
                                {
                                    bIsLicenseAvailable = true;
                                }

                                if(bIsLicenseAvailable == KeyVal.value.bool_value)
                                {
                                    PVPATB_TEST_IS_TRUE(true);
                                }
                                else
                                {
                                    PVPATB_TEST_IS_TRUE(false);
                                }
                            }
                            else
                            {
                                PVPATB_TEST_IS_TRUE(false);
                            }
                        }
                        else
                        {
                            PVPATB_TEST_IS_TRUE(false);
                        }
                    }
                    else
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }
                }
                else
                {
                    PVPATB_TEST_IS_TRUE(false);
                }
            }
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_DoGetMetadataValues_6()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    T_DRM_Rights_Info rightsInfo;
    oscl_memset(&rightsInfo,0,sizeof(T_DRM_Rights_Info));

    int32_t ret = SVC_drm_getRightsInfo(kmjPlugin->m_drmSession,&rightsInfo);
    if(FALSE == ret || 0 == rightsInfo.playRights.indicator)
    {
        PVPATB_TEST_IS_TRUE(true);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        PVMFMetadataList aKeyList;
        aKeyList.push_back(PVMF_DRM_INFO_IS_FORWARD_LOCKED_QUERY);
        Oscl_Vector<PvmiKvp,OsclMemAllocator> aValueList;

        int retCmdId = kmjPlugin->GetNodeMetadataValues( 0,
                                                         aKeyList,
                                                         aValueList,
                                                         0,
                                                         1);

        if( PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START == retCmdId)
        {
            PVMFCPMKmjPlugInOMA1Command* cmd = kmjPlugin->iInputCommands.FindById(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START);

            if(NULL == cmd)
            {
                PVPATB_TEST_IS_TRUE(false);
            }
            else
            {
                kmjPlugin->DoGetMetadataValues(*cmd);

                if(PVMFSuccess == kmjPlugin->cmdStatus)
                {
                    if(1 == aValueList.size())
                    {
                        PvmiKvp KeyVal = aValueList[0];

                        if(0 == oscl_strcmp(KeyVal.key,PVMF_DRM_INFO_IS_FORWARD_LOCKED_VALUE))
                        {
                            if( (1 == KeyVal.length) && (1 == KeyVal.capacity))
                            {
                                bool bIsFL = true;

                                if(SEPARATE_DELIVERY == SVC_drm_getDeliveryMethod(kmjPlugin->m_drmSession))
                                {
                                    bIsFL = false;
                                }

                                if(bIsFL == KeyVal.value.bool_value)
                                {
                                    PVPATB_TEST_IS_TRUE(true);
                                }
                                else
                                {
                                    PVPATB_TEST_IS_TRUE(false);
                                }
                            }
                            else
                            {
                                PVPATB_TEST_IS_TRUE(false);
                            }
                        }
                        else
                        {
                            PVPATB_TEST_IS_TRUE(false);
                        }
                    }
                    else
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }
                }
                else
                {
                    PVPATB_TEST_IS_TRUE(false);
                }
            }
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_DoGetMetadataValues_7()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    T_DRM_Rights_Info rightsInfo;
    oscl_memset(&rightsInfo,0,sizeof(T_DRM_Rights_Info));

    int32_t ret = SVC_drm_getRightsInfo(kmjPlugin->m_drmSession,&rightsInfo);
    if(FALSE == ret || 0 == rightsInfo.playRights.indicator)
    {
        PVPATB_TEST_IS_TRUE(true);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        PVMFMetadataList aKeyList;
        aKeyList.push_back(PVMF_DRM_INFO_CAN_USE_AS_RINGTONE_QUERY);
        Oscl_Vector<PvmiKvp,OsclMemAllocator> aValueList;

        int retCmdId = kmjPlugin->GetNodeMetadataValues( 0,
                                                         aKeyList,
                                                         aValueList,
                                                         0,
                                                         1);

        if( PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START == retCmdId)
        {
            PVMFCPMKmjPlugInOMA1Command* cmd = kmjPlugin->iInputCommands.FindById(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START);

            if(NULL == cmd)
            {
                PVPATB_TEST_IS_TRUE(false);
            }
            else
            {
                kmjPlugin->DoGetMetadataValues(*cmd);

                if(PVMFSuccess == kmjPlugin->cmdStatus)
                {
                    if(1 == aValueList.size())
                    {
                        PvmiKvp KeyVal = aValueList[0];

                        if(0 == oscl_strcmp(KeyVal.key,PVMF_DRM_INFO_CAN_USE_AS_RINGTONE_VALUE))
                        {
                            if( (1 == KeyVal.length) && (1 == KeyVal.capacity))
                            {
                                bool bIsCanBeRingTone = false;

                                if(TRUE == kmjPlugin->IsStatelessLicense())
                                {
                                    bIsCanBeRingTone = true;
                                }

                                if(bIsCanBeRingTone == KeyVal.value.bool_value)
                                {
                                    PVPATB_TEST_IS_TRUE(true);
                                }
                                else
                                {
                                    PVPATB_TEST_IS_TRUE(false);
                                }
                            }
                            else
                            {
                                PVPATB_TEST_IS_TRUE(false);
                            }
                        }
                        else
                        {
                            PVPATB_TEST_IS_TRUE(false);
                        }
                    }
                    else
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }
                }
                else
                {
                    PVPATB_TEST_IS_TRUE(false);
                }
            }
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_DoGetMetadataValues_8()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    T_DRM_Rights_Info rightsInfo;
    oscl_memset(&rightsInfo,0,sizeof(T_DRM_Rights_Info));

    int32_t ret = SVC_drm_getRightsInfo(kmjPlugin->m_drmSession,&rightsInfo);
    if(FALSE == ret || 0 == rightsInfo.playRights.indicator)
    {
        PVPATB_TEST_IS_TRUE(true);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        PVMFMetadataList aKeyList;
        aKeyList.push_back(PVMF_DRM_INFO_LICENSE_TYPE_QUERY);
        Oscl_Vector<PvmiKvp,OsclMemAllocator> aValueList;

        int retCmdId = kmjPlugin->GetNodeMetadataValues( 0,
                                                         aKeyList,
                                                         aValueList,
                                                         0,
                                                         1);

        if( PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START == retCmdId)
        {
            PVMFCPMKmjPlugInOMA1Command* cmd = kmjPlugin->iInputCommands.FindById(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START);

            if(NULL == cmd)
            {
                PVPATB_TEST_IS_TRUE(false);
            }
            else
            {
                kmjPlugin->DoGetMetadataValues(*cmd);

                if(PVMFSuccess == kmjPlugin->cmdStatus)
                {
                    if(1 == aValueList.size())
                    {
                        PvmiKvp KeyVal = aValueList[0];

                        if(0 == oscl_strcmp(KeyVal.key,PVMF_DRM_INFO_LICENSE_TYPE_VALUE))
                        {
                            char* value = (char*)kmjPlugin->GetLicenseType();

                            if(NULL == value)
                            {
                                PVPATB_TEST_IS_TRUE(false);
                            }
                            else
                            {
                                int32_t len = (int32_t)oscl_strlen(value)+1;

                                if( (len == KeyVal.length) && (len == KeyVal.capacity))
                                {
                                    if(0 == oscl_strcmp(value,KeyVal.value.pChar_value))
                                    {
                                        PVPATB_TEST_IS_TRUE(true);
                                    }
                                    else
                                    {
                                        PVPATB_TEST_IS_TRUE(false);
                                    }
                                }
                                else
                                {
                                    PVPATB_TEST_IS_TRUE(false);
                                }
                            }
                        }
                        else
                        {
                            PVPATB_TEST_IS_TRUE(false);
                        }
                    }
                    else
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }
                }
                else
                {
                    PVPATB_TEST_IS_TRUE(false);
                }
            }
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_DoGetMetadataValues_9()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    T_DRM_Rights_Info rightsInfo;
    oscl_memset(&rightsInfo,0,sizeof(T_DRM_Rights_Info));

    int32_t ret = SVC_drm_getRightsInfo(kmjPlugin->m_drmSession,&rightsInfo);
    if(FALSE == ret || 0 == rightsInfo.playRights.indicator)
    {
        PVPATB_TEST_IS_TRUE(true);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        PVMFMetadataList aKeyList;
        aKeyList.push_back(PVMF_DRM_INFO_LICENSE_COUNT_QUERY);
        Oscl_Vector<PvmiKvp,OsclMemAllocator> aValueList;

        int retCmdId = kmjPlugin->GetNodeMetadataValues( 0,
                                                         aKeyList,
                                                         aValueList,
                                                         0,
                                                         1);

        if( PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START == retCmdId)
        {
            PVMFCPMKmjPlugInOMA1Command* cmd = kmjPlugin->iInputCommands.FindById(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START);

            if(NULL == cmd)
            {
                PVPATB_TEST_IS_TRUE(false);
            }
            else
            {
                kmjPlugin->DoGetMetadataValues(*cmd);

                if(PVMFSuccess == kmjPlugin->cmdStatus)
                {
                    T_DRM_Rights_Info rightsInfo;
                    oscl_memset(&rightsInfo,0,sizeof(T_DRM_Rights_Info));

                    if(FALSE == SVC_drm_getRightsInfo(kmjPlugin->m_drmSession,&rightsInfo))
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }
                    else
                    {
                        /// no license count
                        if(0 == (rightsInfo.playRights.indicator & DRM_COUNT_CONSTRAINT))
                        {
                            if(0 == aValueList.size())
                            {
                                PVPATB_TEST_IS_TRUE(true);
                            }
                            else
                            {
                                PVPATB_TEST_IS_TRUE(false);
                            }
                        }
                        else
                        {
                            PvmiKvp KeyVal = aValueList[0];

                            if(1 == aValueList.size())
                            {
                                if(0 == oscl_strcmp(KeyVal.key,PVMF_DRM_INFO_LICENSE_COUNT_VALUE))
                                {
                                    if( (1 == KeyVal.length) && (1 == KeyVal.capacity))
                                    {
                                        if((uint32_t)rightsInfo.playRights.count == KeyVal.value.uint32_value)
                                        {
                                            PVPATB_TEST_IS_TRUE(true);
                                        }
                                        else
                                        {
                                            PVPATB_TEST_IS_TRUE(false);
                                        }
                                    }
                                    else
                                    {
                                        PVPATB_TEST_IS_TRUE(false);
                                    }
                                }
                                else
                                {
                                    PVPATB_TEST_IS_TRUE(false);
                                }
                            }
                            else
                            {
                                PVPATB_TEST_IS_TRUE(false);
                            }
                        }
                    }
                }
                else
                {
                    PVPATB_TEST_IS_TRUE(false);
                }
            }
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_DoGetMetadataValues_10()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    T_DRM_Rights_Info rightsInfo;
    oscl_memset(&rightsInfo,0,sizeof(T_DRM_Rights_Info));

    int32_t ret = SVC_drm_getRightsInfo(kmjPlugin->m_drmSession,&rightsInfo);
    if(FALSE == ret || 0 == rightsInfo.playRights.indicator)
    {
        PVPATB_TEST_IS_TRUE(true);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        PVMFMetadataList aKeyList;
        aKeyList.push_back(PVMF_DRM_INFO_LICENSE_START_TIME_QUERY);
        Oscl_Vector<PvmiKvp,OsclMemAllocator> aValueList;

        int retCmdId = kmjPlugin->GetNodeMetadataValues( 0,
                                                         aKeyList,
                                                         aValueList,
                                                         0,
                                                         1);

        if( PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START == retCmdId)
        {
            PVMFCPMKmjPlugInOMA1Command* cmd = kmjPlugin->iInputCommands.FindById(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START);

            if(NULL == cmd)
            {
                PVPATB_TEST_IS_TRUE(false);
            }
            else
            {
                kmjPlugin->DoGetMetadataValues(*cmd);

                if(PVMFSuccess == kmjPlugin->cmdStatus)
                {
                    T_DRM_Rights_Info rightsInfo;
                    oscl_memset(&rightsInfo,0,sizeof(T_DRM_Rights_Info));

                    if(FALSE == SVC_drm_getRightsInfo(kmjPlugin->m_drmSession,&rightsInfo))
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }
                    else
                    {
                        // no license start time
                        if(0 == (rightsInfo.playRights.indicator & DRM_START_TIME_CONSTRAINT))
                        {
                            if(0 == aValueList.size())
                            {
                                PVPATB_TEST_IS_TRUE(true);
                            }
                            else
                            {
                                PVPATB_TEST_IS_TRUE(false);
                            }
                        }
                        else
                        {
                            PvmiKvp KeyVal = aValueList[0];

                            if(1 == aValueList.size())
                            {
                                if(0 == oscl_strcmp(KeyVal.key,PVMF_DRM_INFO_LICENSE_START_TIME_VALUE))
                                {
                                    if( (KeyVal.length > 1) && (KeyVal.capacity > 1))
                                    {
                                        if(KeyVal.value.pChar_value)
                                        {
                                            PVPATB_TEST_IS_TRUE(true);
                                        }
                                        else
                                        {
                                            PVPATB_TEST_IS_TRUE(false);
                                        }
                                    }
                                    else
                                    {
                                        PVPATB_TEST_IS_TRUE(false);
                                    }
                                }
                                else
                                {
                                    PVPATB_TEST_IS_TRUE(false);
                                }
                            }
                            else
                            {
                                PVPATB_TEST_IS_TRUE(false);
                            }
                        }
                    }
                }
                else
                {
                    PVPATB_TEST_IS_TRUE(false);
                }
            }
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_DoGetMetadataValues_11()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    T_DRM_Rights_Info rightsInfo;
    oscl_memset(&rightsInfo,0,sizeof(T_DRM_Rights_Info));

    int32_t ret = SVC_drm_getRightsInfo(kmjPlugin->m_drmSession,&rightsInfo);
    if(FALSE == ret || 0 == rightsInfo.playRights.indicator)
    {
        PVPATB_TEST_IS_TRUE(true);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        PVMFMetadataList aKeyList;
        aKeyList.push_back(PVMF_DRM_INFO_LICENSE_EXPIRATION_TIME_QUERY);
        Oscl_Vector<PvmiKvp,OsclMemAllocator> aValueList;

        int retCmdId = kmjPlugin->GetNodeMetadataValues( 0,
                                                         aKeyList,
                                                         aValueList,
                                                         0,
                                                         1);

        if( PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START == retCmdId)
        {
            PVMFCPMKmjPlugInOMA1Command* cmd = kmjPlugin->iInputCommands.FindById(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START);

            if(NULL == cmd)
            {
                PVPATB_TEST_IS_TRUE(false);
            }
            else
            {
                kmjPlugin->DoGetMetadataValues(*cmd);

                if(PVMFSuccess == kmjPlugin->cmdStatus)
                {
                    T_DRM_Rights_Info rightsInfo;
                    oscl_memset(&rightsInfo,0,sizeof(T_DRM_Rights_Info));

                    if(FALSE == SVC_drm_getRightsInfo(kmjPlugin->m_drmSession,&rightsInfo))
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }
                    else
                    {
                        // no license expiration time
                        if(0 == (rightsInfo.playRights.indicator & DRM_END_TIME_CONSTRAINT))
                        {
                            if(0 == aValueList.size())
                            {
                                PVPATB_TEST_IS_TRUE(true);
                            }
                            else
                            {
                                PVPATB_TEST_IS_TRUE(false);
                            }
                        }
                        else
                        {
                            PvmiKvp KeyVal = aValueList[0];

                            if(1 == aValueList.size())
                            {
                                if(0 == oscl_strcmp(KeyVal.key,PVMF_DRM_INFO_LICENSE_EXPIRATION_TIME_VALUE))
                                {
                                    if( (KeyVal.length > 1) && (KeyVal.capacity > 1))
                                    {
                                        if(KeyVal.value.pChar_value)
                                        {
                                            PVPATB_TEST_IS_TRUE(true);
                                        }
                                        else
                                        {
                                            PVPATB_TEST_IS_TRUE(false);
                                        }
                                    }
                                    else
                                    {
                                        PVPATB_TEST_IS_TRUE(false);
                                    }
                                }
                                else
                                {
                                    PVPATB_TEST_IS_TRUE(false);
                                }
                            }
                            else
                            {
                                PVPATB_TEST_IS_TRUE(false);
                            }
                        }
                    }
                }
                else
                {
                    PVPATB_TEST_IS_TRUE(false);
                }
            }
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_DoGetMetadataValues_12()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    T_DRM_Rights_Info rightsInfo;
    oscl_memset(&rightsInfo,0,sizeof(T_DRM_Rights_Info));

    int32_t ret = SVC_drm_getRightsInfo(kmjPlugin->m_drmSession,&rightsInfo);
    if(FALSE == ret || 0 == rightsInfo.playRights.indicator)
    {
        PVPATB_TEST_IS_TRUE(true);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        PVMFMetadataList aKeyList;
        aKeyList.push_back(PVMF_DRM_INFO_LICENSE_DURATION_SINCE_FIRST_USE_QUERY);
        Oscl_Vector<PvmiKvp,OsclMemAllocator> aValueList;

        int retCmdId = kmjPlugin->GetNodeMetadataValues( 0,
                                                         aKeyList,
                                                         aValueList,
                                                         0,
                                                         1);

        if( PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START == retCmdId)
        {
            PVMFCPMKmjPlugInOMA1Command* cmd = kmjPlugin->iInputCommands.FindById(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START);

            if(NULL == cmd)
            {
                PVPATB_TEST_IS_TRUE(false);
            }
            else
            {
                kmjPlugin->DoGetMetadataValues(*cmd);

                if(PVMFSuccess == kmjPlugin->cmdStatus)
                {
                    T_DRM_Rights_Info rightsInfo;
                    oscl_memset(&rightsInfo,0,sizeof(T_DRM_Rights_Info));

                    if(FALSE == SVC_drm_getRightsInfo(kmjPlugin->m_drmSession,&rightsInfo))
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }
                    else
                    {
                        // no license remain duration
                        if(0 == (rightsInfo.playRights.indicator & DRM_INTERVAL_CONSTRAINT))
                        {
                            if(0 == aValueList.size())
                            {
                                PVPATB_TEST_IS_TRUE(true);
                            }
                            else
                            {
                                PVPATB_TEST_IS_TRUE(false);
                            }
                        }
                        else
                        {
                            PvmiKvp KeyVal = aValueList[0];

                            if(1 == aValueList.size())
                            {
                                if(0 == oscl_strcmp(KeyVal.key,PVMF_DRM_INFO_LICENSE_DURATION_SINCE_FIRST_USE_VALUE))
                                {
                                    if( (KeyVal.length > 1) && (KeyVal.capacity > 1))
                                    {
                                        if(KeyVal.value.pChar_value)
                                        {
                                            PVPATB_TEST_IS_TRUE(true);
                                        }
                                        else
                                        {
                                            PVPATB_TEST_IS_TRUE(false);
                                        }
                                    }
                                    else
                                    {
                                        PVPATB_TEST_IS_TRUE(false);
                                    }
                                }
                                else
                                {
                                    PVPATB_TEST_IS_TRUE(false);
                                }
                            }
                            else
                            {
                                PVPATB_TEST_IS_TRUE(false);
                            }
                        }
                    }
                }
                else
                {
                    PVPATB_TEST_IS_TRUE(false);
                }
            }
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_DoGetMetadataValues_13()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    T_DRM_Rights_Info rightsInfo;
    oscl_memset(&rightsInfo,0,sizeof(T_DRM_Rights_Info));

    int32_t ret = SVC_drm_getRightsInfo(kmjPlugin->m_drmSession,&rightsInfo);
    if(FALSE == ret || 0 == rightsInfo.playRights.indicator)
    {
        PVPATB_TEST_IS_TRUE(true);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        PVMFMetadataList aKeyList;
        aKeyList.push_back(PVMF_DRM_INFO_ENVELOPE_DATA_SIZE_QUERY);
        Oscl_Vector<PvmiKvp,OsclMemAllocator> aValueList;

        int retCmdId = kmjPlugin->GetNodeMetadataValues( 0,
                                                         aKeyList,
                                                         aValueList,
                                                         0,
                                                         1);

        if( PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START == retCmdId)
        {
            PVMFCPMKmjPlugInOMA1Command* cmd = kmjPlugin->iInputCommands.FindById(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START);

            if(NULL == cmd)
            {
                PVPATB_TEST_IS_TRUE(false);
            }
            else
            {
                kmjPlugin->DoGetMetadataValues(*cmd);

                if(PVMFSuccess == kmjPlugin->cmdStatus)
                {
                    if(1 == aValueList.size())
                    {
                        PvmiKvp KeyVal = aValueList[0];

                        if(0 == oscl_strcmp(KeyVal.key,PVMF_DRM_INFO_ENVELOPE_DATA_SIZE_VALUE))
                        {
                            if( (1 == KeyVal.length) && (1 == KeyVal.capacity))
                            {
                                int32_t contentSize = SVC_drm_getContentLength(kmjPlugin->m_drmSession);

                                if(0 == contentSize)
                                {
                                    PVPATB_TEST_IS_TRUE(false);
                                }
                                else
                                {
                                    if((uint32_t)contentSize == KeyVal.value.uint32_value)
                                    {
                                        PVPATB_TEST_IS_TRUE(true);
                                    }
                                    else
                                    {
                                        PVPATB_TEST_IS_TRUE(false);
                                    }
                                }
                            }
                            else
                            {
                                PVPATB_TEST_IS_TRUE(false);
                            }
                        }
                        else
                        {
                            PVPATB_TEST_IS_TRUE(false);
                        }
                    }
                    else
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }
                }
                else
                {
                    PVPATB_TEST_IS_TRUE(false);
                }
            }
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_DoGetMetadataValues_14()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    T_DRM_Rights_Info rightsInfo;
    oscl_memset(&rightsInfo,0,sizeof(T_DRM_Rights_Info));

    int32_t ret = SVC_drm_getRightsInfo(kmjPlugin->m_drmSession,&rightsInfo);
    if(FALSE == ret || 0 == rightsInfo.playRights.indicator)
    {
        PVPATB_TEST_IS_TRUE(true);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        PVMFMetadataList aKeyList;
        aKeyList.push_back(PVMF_DRM_INFO_ENVELOPE_DATA_QUERY);
        Oscl_Vector<PvmiKvp,OsclMemAllocator> aValueList;

        int retCmdId = kmjPlugin->GetNodeMetadataValues( 0,
                                                         aKeyList,
                                                         aValueList,
                                                         0,
                                                         1);

        if( PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START == retCmdId)
        {
            PVMFCPMKmjPlugInOMA1Command* cmd = kmjPlugin->iInputCommands.FindById(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START);

            if(NULL == cmd)
            {
                PVPATB_TEST_IS_TRUE(false);
            }
            else
            {
                kmjPlugin->DoGetMetadataValues(*cmd);

                if(PVMFSuccess == kmjPlugin->cmdStatus)
                {
                    if(1 == aValueList.size())
                    {
                        PvmiKvp KeyVal = aValueList[0];

                        if(0 == oscl_strcmp(KeyVal.key,PVMF_DRM_INFO_ENVELOPE_DATA_VALUE))
                        {
                            int32_t contentSize = SVC_drm_getContentLength(kmjPlugin->m_drmSession);

                            if(0 == contentSize)
                            {
                                PVPATB_TEST_IS_TRUE(false);
                            }
                            else
                            {
                                if( (contentSize == KeyVal.length) && (contentSize == KeyVal.capacity))
                                {
                                    if(KeyVal.value.pUint8_value)
                                    {
                                        PVPATB_TEST_IS_TRUE(true);
                                    }
                                    else
                                    {
                                        PVPATB_TEST_IS_TRUE(false);
                                    }
                                }
                                else
                                {
                                    PVPATB_TEST_IS_TRUE(false);
                                }
                            }
                        }
                        else
                        {
                            PVPATB_TEST_IS_TRUE(false);
                        }
                    }
                    else
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }
                }
                else
                {
                    PVPATB_TEST_IS_TRUE(false);
                }
            }
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_DoGetMetadataValues_15()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    T_DRM_Rights_Info rightsInfo;
    oscl_memset(&rightsInfo,0,sizeof(T_DRM_Rights_Info));

    int32_t ret = SVC_drm_getRightsInfo(kmjPlugin->m_drmSession,&rightsInfo);
    if(FALSE == ret || 0 == rightsInfo.playRights.indicator)
    {
        PVPATB_TEST_IS_TRUE(true);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        Oscl_Vector<PvmiKvp,OsclMemAllocator> aValueList;

        int retCmdId = kmjPlugin->GetNodeMetadataValues( 0,
                                                         iAvailableMetadataKeys,
                                                         aValueList,
                                                         0,
                                                         iAvailableMetadataKeys.size());

        if( PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START == retCmdId)
        {
            PVMFCPMKmjPlugInOMA1Command* cmd = kmjPlugin->iInputCommands.FindById(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START);

            if(NULL == cmd)
            {
                PVPATB_TEST_IS_TRUE(false);
            }
            else
            {
                kmjPlugin->DoGetMetadataValues(*cmd);

                if(PVMFSuccess == kmjPlugin->cmdStatus)
                {
                    if(iAvailableMetadataKeys.size() == aValueList.size())
                    {
                        PVPATB_TEST_IS_TRUE(true);
                    }
                    else
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }
                }
                else
                {
                    PVPATB_TEST_IS_TRUE(false);
                }
            }
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_ReleaseNodeMetadataValues_1()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        Oscl_Vector<PvmiKvp,OsclMemAllocator> aValueList;

        if(PVMFErrArgument == kmjPlugin->ReleaseNodeMetadataValues(aValueList,0,0))
        {
            PVPATB_TEST_IS_TRUE(true);
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_ReleaseNodeMetadataValues_2()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        Oscl_Vector<PvmiKvp,OsclMemAllocator> aValueList;
        PvmiKvp KeyVal = {NULL,1,1,{0}};
        KeyVal.key = (char*)PVMF_DRM_INFO_IS_PROTECTED_VALUE;
        KeyVal.value.bool_value = true;

        aValueList.push_back(KeyVal);

        if(PVMFErrArgument == kmjPlugin->ReleaseNodeMetadataValues(aValueList,1,0))
        {
            PVPATB_TEST_IS_TRUE(true);
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_ReleaseNodeMetadataValues_3()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        Oscl_Vector<PvmiKvp,OsclMemAllocator> aValueList;

        for( uint32_t i = 0; i < 2; i++)
        {
            PvmiKvp KeyVal = {NULL,1,1,{0}};
            KeyVal.key = (char*)PVMF_DRM_INFO_IS_PROTECTED_VALUE;
            KeyVal.value.bool_value = true;

            aValueList.push_back(KeyVal);
        }

        if(PVMFErrArgument == kmjPlugin->ReleaseNodeMetadataValues(aValueList,1,0))
        {
            PVPATB_TEST_IS_TRUE(true);
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_ReleaseNodeMetadataValues_4()
{
    if(false == InitBeforeOneCaseStart())
    {
        PVPATB_TEST_IS_TRUE(false);
        ResetAfterOneCaseComplete();
        return;
    }

    T_DRM_Rights_Info rightsInfo;
    oscl_memset(&rightsInfo,0,sizeof(T_DRM_Rights_Info));

    int32_t ret = SVC_drm_getRightsInfo(kmjPlugin->m_drmSession,&rightsInfo);
    if(FALSE == ret || 0 == rightsInfo.playRights.indicator)
    {
        PVPATB_TEST_IS_TRUE(true);
        ResetAfterOneCaseComplete();
        return;
    }

    if(kmjPlugin)
    {
        Oscl_Vector<PvmiKvp,OsclMemAllocator> aValueList;

        int retCmdId = kmjPlugin->GetNodeMetadataValues( 0,
                                                         iAvailableMetadataKeys,
                                                         aValueList,
                                                         0,
                                                         iAvailableMetadataKeys.size());

        if( PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START == retCmdId)
        {
            PVMFCPMKmjPlugInOMA1Command* cmd = kmjPlugin->iInputCommands.FindById(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START);

            if(NULL == cmd)
            {
                PVPATB_TEST_IS_TRUE(false);
            }
            else
            {
                kmjPlugin->DoGetMetadataValues(*cmd);

                if(PVMFSuccess == kmjPlugin->cmdStatus)
                {
                    if(iAvailableMetadataKeys.size() == aValueList.size())
                    {
                        if(PVMFSuccess == kmjPlugin->ReleaseNodeMetadataValues(aValueList,0,aValueList.size()))
                        {
                            PVPATB_TEST_IS_TRUE(true);
                        }
                        else
                        {
                            PVPATB_TEST_IS_TRUE(false);
                        }
                    }
                    else
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }
                }
                else
                {
                    PVPATB_TEST_IS_TRUE(false);
                }
            }
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    ResetAfterOneCaseComplete();
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_GetIso8601Time_1()
{
    if(kmjPlugin)
    {
        if(NULL == kmjPlugin->GetIso8601Time(200709266,110000))
        {
            PVPATB_TEST_IS_TRUE(true);
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_GetIso8601Time_2()
{
    if(kmjPlugin)
    {
        if(NULL == kmjPlugin->GetIso8601Time(-2007926,110000))
        {
            PVPATB_TEST_IS_TRUE(true);
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_GetIso8601Time_3()
{
    if(kmjPlugin)
    {
        if(NULL == kmjPlugin->GetIso8601Time(20070926,1102000))
        {
            PVPATB_TEST_IS_TRUE(true);
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_GetIso8601Time_4()
{
    if(kmjPlugin)
    {
        char* retTime = kmjPlugin->GetIso8601Time(20070926,111111);
        if(retTime)
        {
            if(0 == oscl_strcmp(retTime,"2007-09-26T11:11:11Z"))
            {
                PVPATB_TEST_IS_TRUE(true);
            }
            else
            {
                PVPATB_TEST_IS_TRUE(false);
            }
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_GetDuration_1()
{
    if(kmjPlugin)
    {
        if(0 == kmjPlugin->GetDuration(-1,1))
        {
            PVPATB_TEST_IS_TRUE(true);
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_GetDuration_2()
{
    if(kmjPlugin)
    {
        if(0 == kmjPlugin->GetDuration(1,-1))
        {
            PVPATB_TEST_IS_TRUE(true);
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_GetDuration_3()
{
    if(kmjPlugin)
    {
        if(0 == kmjPlugin->GetDuration(0,0))
        {
            PVPATB_TEST_IS_TRUE(true);
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_plugin_GetDuration_4()
{
    if(kmjPlugin)
    {
        uint32_t expectRet = 1 * 24 * 60 * 60 + 1 * 60 * 60 + 1 * 60 + 1;

        if(expectRet == kmjPlugin->GetDuration(1,10101))
        {
            PVPATB_TEST_IS_TRUE(true);
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_dataaccess_DrmPluginGetDataLen_file()
{
    Oscl_FileServer fileServ;
    PVFile pvfile;
    oscl_wchar wcharFileName[512];

    oscl_UTF8ToUnicode(iFileName, oscl_strlen(iFileName),wcharFileName,512);

    // open file
    int32 ret = pvfile.Open( wcharFileName,Oscl_File::MODE_READ | Oscl_File::MODE_BINARY,fileServ);
    if(ret)  // open file failed
    {
        PVPATB_TEST_IS_TRUE(false);
        return ;
    }

    DrmPluginDataAccess dataAccess(&pvfile);

    int32_t len = DrmPluginGetDataLen((int32_t)&dataAccess);

    if(len > 0)
    {
        PVPATB_TEST_IS_TRUE(true);
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_dataaccess_DrmPluginSeekData_file()
{
    Oscl_FileServer fileServ;
    PVFile pvfile;
    oscl_wchar wcharFileName[512];

    oscl_UTF8ToUnicode(iFileName, oscl_strlen(iFileName),wcharFileName,512);

    // open file
    int32 ret = pvfile.Open( wcharFileName,Oscl_File::MODE_READ | Oscl_File::MODE_BINARY,fileServ);
    if(ret)  // open file failed
    {
        PVPATB_TEST_IS_TRUE(false);
        return ;
    }

    DrmPluginDataAccess dataAccess(&pvfile);

    int32_t len = DrmPluginGetDataLen((int32_t)&dataAccess);

    if(len > 0)
    {
        PVPATB_TEST_IS_TRUE(true);
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
        return;
    }

    if(0 == DrmPluginSeekData((int32_t)&dataAccess, len - 1))
    {
        int32 filePos = pvfile.Tell();

        if((len - 1) == filePos)
        {
            PVPATB_TEST_IS_TRUE(true);
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
            return;
        }

        if(0 == DrmPluginSeekData((int32_t)&dataAccess,0))
        {
            int32 filePos = pvfile.Tell();

            if(0 == filePos)
            {
                PVPATB_TEST_IS_TRUE(true);
            }
            else
            {
                PVPATB_TEST_IS_TRUE(false);
                return;
            }
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_dataaccess_DrmPluginReadData_file()
{
    Oscl_FileServer fileServ;
    PVFile pvfile;
    oscl_wchar wcharFileName[512];

    oscl_UTF8ToUnicode(iFileName, oscl_strlen(iFileName),wcharFileName,512);

    // open file
    int32 ret = pvfile.Open( wcharFileName,Oscl_File::MODE_READ | Oscl_File::MODE_BINARY,fileServ);
    if(ret)  // open file failed
    {
        PVPATB_TEST_IS_TRUE(false);
        return ;
    }

    DrmPluginDataAccess dataAccess(&pvfile);

    int32_t len = DrmPluginGetDataLen((int32_t)&dataAccess);

    if(len > 0)
    {
        PVPATB_TEST_IS_TRUE(true);
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
        return;
    }

    int32_t firstReadLen = len/2;
    uint8_t* firstReadBuf = OSCL_ARRAY_NEW(uint8_t,firstReadLen);
    if(!firstReadBuf)
    {
        PVPATB_TEST_IS_TRUE(false);
        return;
    }

    int32_t actulLen = DrmPluginReadData((int32_t)&dataAccess,firstReadBuf,firstReadLen);
    OSCL_ARRAY_DELETE(firstReadBuf);
    if(firstReadLen == actulLen)
    {
        PVPATB_TEST_IS_TRUE(true);
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
        return;
    }

    int32_t secondReadLen = len - firstReadLen;
    uint8_t* secondReadBuf = OSCL_ARRAY_NEW(uint8_t,secondReadLen);
    if(!secondReadBuf)
    {
        PVPATB_TEST_IS_TRUE(false);
        return;
    }

    actulLen = DrmPluginReadData((int32_t)&dataAccess,secondReadBuf,secondReadLen);
    if(secondReadLen == actulLen)
    {
        PVPATB_TEST_IS_TRUE(true);
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
        OSCL_ARRAY_DELETE(secondReadBuf);
        return;
    }

    actulLen = DrmPluginReadData((int32_t)&dataAccess,secondReadBuf,secondReadLen);
    if(actulLen > 0)
    {
        PVPATB_TEST_IS_TRUE(false);
        OSCL_ARRAY_DELETE(secondReadBuf);
        return;
    }
    else
    {
        PVPATB_TEST_IS_TRUE(true);
    }

    if(0 == DrmPluginSeekData((int32_t)&dataAccess, len - 1))
    {
        actulLen = DrmPluginReadData((int32_t)&dataAccess,secondReadBuf,1);
        if(actulLen != 1)
        {
            PVPATB_TEST_IS_TRUE(false);
            OSCL_ARRAY_DELETE(secondReadBuf);
            return;
        }
        else
        {
            PVPATB_TEST_IS_TRUE(true);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    OSCL_ARRAY_DELETE(secondReadBuf);
}

static uint8_t testData_buf[] =
{
    0x01, 0x0a, 0x1a, 0x74, 0x65, 0x78, 0x74, 0x2f, 0x70, 0x6c, 0x61, 0x69, 0x6e, 0x63, 0x69, 0x64,
    0x3a, 0x68, 0x74, 0x74, 0x70, 0x3a, 0x2f, 0x2f, 0x63, 0x6f, 0x6e, 0x74, 0x65, 0x6e, 0x74, 0x2d,
    0x69, 0x64, 0x2d, 0x68, 0x65, 0x72, 0x65, 0x4f, 0x20, 0x45, 0x6e, 0x63, 0x72, 0x79, 0x70, 0x74,
    0x69, 0x6f, 0x6e, 0x2d, 0x4d, 0x65, 0x74, 0x68, 0x6f, 0x64, 0x3a, 0x20, 0x41, 0x45, 0x53, 0x31,
    0x32, 0x38, 0x43, 0x42, 0x43, 0x0d, 0x0a, 0x52, 0x69, 0x67, 0x68, 0x74, 0x73, 0x2d, 0x49, 0x73,
    0x73, 0x75, 0x65, 0x72, 0x3a, 0x20, 0x68, 0x74, 0x74, 0x70, 0x3a, 0x2f, 0x2f, 0x72, 0x69, 0x67,
    0x68, 0x74, 0x73, 0x2d, 0x69, 0x73, 0x73, 0x75, 0x65, 0x72, 0x2e, 0x63, 0x6f, 0x6d, 0x2f, 0x63,
    0x6f, 0x6e, 0x74, 0x65, 0x6e, 0x74, 0x0d, 0x0a, 0xbe, 0xc0, 0xcb, 0x13, 0xcd, 0x35, 0xc3, 0xe1,
    0xbf, 0x47, 0xb6, 0x89, 0x33, 0xb1, 0x06, 0x9b, 0xd8, 0x3b, 0x1c, 0xe5, 0xba, 0xe6, 0x32, 0x19,
    0x50, 0xba, 0x1c, 0xcd, 0x2d, 0x32, 0xec, 0xd7
};

static int32_t testData_len = sizeof(testData_buf);

void pvplayer_test_drmcpmplugin_ut::test_kmj_dataaccess_DrmPluginGetDataLen_buffer()
{
    DrmPluginDataAccess dataAccess( NULL,
                                    DrmPluginDataAccess::ACCESS_BUFFER,
                                    (char*)testData_buf,
                                    testData_len);

    int32_t len = DrmPluginGetDataLen((int32_t)&dataAccess);

    if(len == testData_len)
    {
        PVPATB_TEST_IS_TRUE(true);
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_dataaccess_DrmPluginSeekData_buffer()
{
    DrmPluginDataAccess dataAccess( NULL,
                                    DrmPluginDataAccess::ACCESS_BUFFER,
                                    (char*)testData_buf,
                                    testData_len);

    int32_t len = DrmPluginGetDataLen((int32_t)&dataAccess);

    if(len == testData_len)
    {
        PVPATB_TEST_IS_TRUE(true);
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
        return;
    }

    if(0 == DrmPluginSeekData((int32_t)&dataAccess, len - 1))
    {
        PVPATB_TEST_IS_TRUE(true);
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
        return;
    }

    if(0 == DrmPluginSeekData((int32_t)&dataAccess, len + 1))
    {
        PVPATB_TEST_IS_TRUE(true);
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }

    if(0 == DrmPluginSeekData((int32_t)&dataAccess, -100))
    {
        PVPATB_TEST_IS_TRUE(false);
    }
    else
    {
        PVPATB_TEST_IS_TRUE(true);
    }
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_dataaccess_DrmPluginReadData_buffer()
{
    DrmPluginDataAccess dataAccess( NULL,
                                    DrmPluginDataAccess::ACCESS_BUFFER,
                                    (char*)testData_buf,
                                    testData_len);

    int32_t len = DrmPluginGetDataLen((int32_t)&dataAccess);

    if(len == testData_len)
    {
        PVPATB_TEST_IS_TRUE(true);
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
        return;
    }

    int32_t firstReadLen = len/2;
    uint8_t* firstReadBuf = OSCL_ARRAY_NEW(uint8_t,firstReadLen);
    if(!firstReadBuf)
    {
        PVPATB_TEST_IS_TRUE(false);
        return;
    }

    int32_t actulLen = DrmPluginReadData((int32_t)&dataAccess,firstReadBuf,firstReadLen);
    OSCL_ARRAY_DELETE(firstReadBuf);
    if(firstReadLen == actulLen)
    {
        PVPATB_TEST_IS_TRUE(true);
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
        return;
    }

    int32_t secondReadLen = len - firstReadLen;
    uint8_t* secondReadBuf = OSCL_ARRAY_NEW(uint8_t,secondReadLen);
    if(!secondReadBuf)
    {
        PVPATB_TEST_IS_TRUE(false);
        return;
    }

    DrmPluginSeekData((int32_t)&dataAccess,secondReadLen);
    actulLen = DrmPluginReadData((int32_t)&dataAccess,secondReadBuf,secondReadLen);
    if(secondReadLen == actulLen)
    {
        PVPATB_TEST_IS_TRUE(true);
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
        OSCL_ARRAY_DELETE(secondReadBuf);
        return;
    }

    DrmPluginSeekData((int32_t)&dataAccess,len);
    actulLen = DrmPluginReadData((int32_t)&dataAccess,secondReadBuf,secondReadLen);
    if(actulLen > 0)
    {
        PVPATB_TEST_IS_TRUE(false);
        OSCL_ARRAY_DELETE(secondReadBuf);
        return;
    }
    else
    {
        PVPATB_TEST_IS_TRUE(true);
    }

    OSCL_ARRAY_DELETE(secondReadBuf);
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_recognizer_RecognizeContentType_mp4()
{
    if(recognizer)
    {
        OSCL_HeapString<OsclMemAllocator> recognizedFormat;

        recognizer->RecognizeContentType("viDeo/Mp4",recognizedFormat);

        if(0 == oscl_strcmp(recognizedFormat.get_cstr(),PVMF_MIME_MPEG4FF))
        {
            PVPATB_TEST_IS_TRUE(true);
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }

        recognizer->RecognizeContentType("video/mp4",recognizedFormat);

        if(0 == oscl_strcmp(recognizedFormat.get_cstr(),PVMF_MIME_MPEG4FF))
        {
            PVPATB_TEST_IS_TRUE(true);
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }

        recognizer->RecognizeContentType("audio/mp4",recognizedFormat);

        if(0 == oscl_strcmp(recognizedFormat.get_cstr(),PVMF_MIME_MPEG4FF))
        {
            PVPATB_TEST_IS_TRUE(true);
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }

        recognizer->RecognizeContentType("video/3gpp2",recognizedFormat);

        if(0 == oscl_strcmp(recognizedFormat.get_cstr(),PVMF_MIME_MPEG4FF))
        {
            PVPATB_TEST_IS_TRUE(true);
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }

        recognizer->RecognizeContentType("video/3gpp",recognizedFormat);

        if(0 == oscl_strcmp(recognizedFormat.get_cstr(),PVMF_MIME_MPEG4FF))
        {
            PVPATB_TEST_IS_TRUE(true);
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }

        recognizer->RecognizeContentType("audio/3gpp2",recognizedFormat);

        if(0 == oscl_strcmp(recognizedFormat.get_cstr(),PVMF_MIME_MPEG4FF))
        {
            PVPATB_TEST_IS_TRUE(true);
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }

        recognizer->RecognizeContentType("audio/3gpp",recognizedFormat);

        if(0 == oscl_strcmp(recognizedFormat.get_cstr(),PVMF_MIME_MPEG4FF))
        {
            PVPATB_TEST_IS_TRUE(true);
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_recognizer_RecognizeContentType_mp3()
{
    if(recognizer)
    {
        OSCL_HeapString<OsclMemAllocator> recognizedFormat;

        recognizer->RecognizeContentType("Audio/Mp3",recognizedFormat);

        if(0 == oscl_strcmp(recognizedFormat.get_cstr(),PVMF_MIME_MP3FF))
        {
            PVPATB_TEST_IS_TRUE(true);
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }

        recognizer->RecognizeContentType("audio/mp3",recognizedFormat);

        if(0 == oscl_strcmp(recognizedFormat.get_cstr(),PVMF_MIME_MP3FF))
        {
            PVPATB_TEST_IS_TRUE(true);
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }

        recognizer->RecognizeContentType("audio/MPA",recognizedFormat);

        if(0 == oscl_strcmp(recognizedFormat.get_cstr(),PVMF_MIME_MP3FF))
        {
            PVPATB_TEST_IS_TRUE(true);
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }

        recognizer->RecognizeContentType("audio/mpeg",recognizedFormat);

        if(0 == oscl_strcmp(recognizedFormat.get_cstr(),PVMF_MIME_MP3FF))
        {
            PVPATB_TEST_IS_TRUE(true);
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_recognizer_RecognizeContentType_amr()
{
    if(recognizer)
    {
        OSCL_HeapString<OsclMemAllocator> recognizedFormat;

        recognizer->RecognizeContentType("aUdio/AmR",recognizedFormat);

        if(0 == oscl_strcmp(recognizedFormat.get_cstr(),PVMF_MIME_AMRFF))
        {
            PVPATB_TEST_IS_TRUE(true);
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }

        recognizer->RecognizeContentType("audio/AMR",recognizedFormat);

        if(0 == oscl_strcmp(recognizedFormat.get_cstr(),PVMF_MIME_AMRFF))
        {
            PVPATB_TEST_IS_TRUE(true);
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }

        recognizer->RecognizeContentType("audio/AMR-WB",recognizedFormat);

        if(0 == oscl_strcmp(recognizedFormat.get_cstr(),PVMF_MIME_AMRFF))
        {
           PVPATB_TEST_IS_TRUE(true);
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }

        recognizer->RecognizeContentType("audio/amr-wb+",recognizedFormat);

        if(0 == oscl_strcmp(recognizedFormat.get_cstr(),PVMF_MIME_AMRFF))
        {
            PVPATB_TEST_IS_TRUE(true);
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_recognizer_RecognizeContentType_aac()
{
    if(recognizer)
    {
        OSCL_HeapString<OsclMemAllocator> recognizedFormat;

        recognizer->RecognizeContentType("aUdiO/mpeG4-geneRic",recognizedFormat);

        if(0 == oscl_strcmp(recognizedFormat.get_cstr(),PVMF_MIME_AACFF))
        {
            PVPATB_TEST_IS_TRUE(true);
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }

        recognizer->RecognizeContentType("audio/mpeg4-generic",recognizedFormat);

        if(0 == oscl_strcmp(recognizedFormat.get_cstr(),PVMF_MIME_AACFF))
        {
            PVPATB_TEST_IS_TRUE(true);
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_recognizer_RecognizeContentType_unknown()
{
    if(recognizer)
    {
        OSCL_HeapString<OsclMemAllocator> recognizedFormat;

        recognizer->RecognizeContentType("aUdi/mpeG4-geneRic",recognizedFormat);

        if(0 == oscl_strcmp(recognizedFormat.get_cstr(),PVMF_MIME_FORMAT_UNKNOWN))
        {
            PVPATB_TEST_IS_TRUE(true);
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }

        recognizer->RecognizeContentType("audio/AM",recognizedFormat);

        if(0 == oscl_strcmp(recognizedFormat.get_cstr(),PVMF_MIME_FORMAT_UNKNOWN))
        {
            PVPATB_TEST_IS_TRUE(true);
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }

        recognizer->RecognizeContentType("ideo/mp4",recognizedFormat);

        if(0 == oscl_strcmp(recognizedFormat.get_cstr(),PVMF_MIME_FORMAT_UNKNOWN))
        {
            PVPATB_TEST_IS_TRUE(true);
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }

        recognizer->RecognizeContentType("aio/mp3",recognizedFormat);

        if(0 == oscl_strcmp(recognizedFormat.get_cstr(),PVMF_MIME_FORMAT_UNKNOWN))
        {
            PVPATB_TEST_IS_TRUE(true);
        }
        else
        {
            PVPATB_TEST_IS_TRUE(false);
        }
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }
}

void pvplayer_test_drmcpmplugin_ut::test_kmj_recognizer_Recognize()
{
    if(recognizer)
    {
        oscl_wchar iTempWCharBuf[512];

        // Convert the source file name to UCS2
        OSCL_wHeapString<OsclMemAllocator> filename_wstr;
        oscl_UTF8ToUnicode(iFileName, oscl_strlen(iFileName),iTempWCharBuf,512);
        filename_wstr.set(iTempWCharBuf,oscl_strlen(iTempWCharBuf));

        // Create a ref data stream access factory based on the filename
        PVMFDataStreamFactory* iRefDataStreamFactory = OSCL_STATIC_CAST(PVMFDataStreamFactory*, OSCL_NEW(PVMIDataStreamSyncInterfaceRefFactory, (filename_wstr)));

        Oscl_Vector<PVMFRecognizerResult,OsclMemAllocator> recognizerResult;
        if(PVMFSuccess != recognizer->Recognize(*iRefDataStreamFactory,NULL,recognizerResult))
        {
              PVPATB_TEST_IS_TRUE(false);
        }
        else if(recognizerResult.empty())
        {
            PVPATB_TEST_IS_TRUE(false);
        }
        else
        {
            PVMFRecognizerResult& recogResult = recognizerResult.front();
            const char* contentType = recogResult.iRecognizedFormat.get_cstr();

            if(0 == oscl_strcmp(contentType,PVMF_MIME_MPEG4FF))
            {
                printf("PVOMA1KMJRecognizerPlugin::Recognize: MP4\r\n");
            }
            else if(0 == oscl_strcmp(contentType,PVMF_MIME_MP3FF))
            {
                printf("PVOMA1KMJRecognizerPlugin::Recognize: MP3\r\n");
            }
            else if(0 == oscl_strcmp(contentType,PVMF_MIME_AMRFF))
            {
                printf("PVOMA1KMJRecognizerPlugin::Recognize: AMR\r\n");
            }
            else if(0 == oscl_strcmp(contentType,PVMF_MIME_AACFF))
            {
                printf("PVOMA1KMJRecognizerPlugin::Recognize: AAC\r\n");
            }
            else if(0 == oscl_strcmp(contentType,PVMF_MIME_FORMAT_UNKNOWN))
            {
                printf("PVOMA1KMJRecognizerPlugin::Recognize: UNKNOWN\r\n");
            }
            else
            {
                PVPATB_TEST_IS_TRUE(false);
            }
        }

        // Destroy the ref data stream sync
        OSCL_DELETE(OSCL_STATIC_CAST(PVMIDataStreamSyncInterfaceRefFactory*, iRefDataStreamFactory));
        iRefDataStreamFactory = NULL;
    }
    else
    {
        PVPATB_TEST_IS_TRUE(false);
    }
}


void pvplayer_test_drmcpmplugin_ut::StartTest()
{
    AddToScheduler();
    RunIfNotReady();
}

void pvplayer_test_drmcpmplugin_ut::Run()
{
    //////// test basic plugin interfaces
    test_kmj_plugin_QueryUUID();
    test_kmj_plugin_DoQueryUuid_1();
    test_kmj_plugin_DoQueryUuid_2();
    test_kmj_plugin_DoQueryUuid_3();
    test_kmj_plugin_DoQueryUuid_4();
    test_kmj_plugin_DoQueryUuid_5();
    test_kmj_plugin_DoQueryUuid_6();
    test_kmj_plugin_QueryInterface();
    test_kmj_plugin_DoQueryInterface_1();
    test_kmj_plugin_DoQueryInterface_2();
    test_kmj_plugin_DoQueryInterface_3();
    test_kmj_plugin_DoQueryInterface_4();
    test_kmj_plugin_DoQueryInterface_5();
    test_kmj_plugin_DoQueryInterface_6();
    test_kmj_plugin_DoQueryInterface_7();
    test_kmj_plugin_DoQueryInterface_8();

    //////// test authorize license useage
    test_kmj_plugin_AuthorizeUsage();
    test_kmj_plugin_UsageComplete();
    test_kmj_plugin_DoAuthorizeUsage_1();
    test_kmj_plugin_DoAuthorizeUsage_2();
    test_kmj_plugin_DoAuthorizeUsage_3();
    test_kmj_plugin_DoAuthorizeUsage_4();

    ///////////test get license
    test_kmj_plugin_ParseUrlHost();
    test_kmj_plugin_ParseUrlIp();
    test_kmj_plugin_GetLicense();
    test_kmj_plugin_GetLicense_w();
    test_kmj_plugin_DoGetLicense();

    //////////test get metadata
    test_kmj_plugin_GetNumMetadataKeys_1();
    test_kmj_plugin_GetNumMetadataKeys_2();
    test_kmj_plugin_GetNumMetadataKeys_3();
    test_kmj_plugin_GetNumMetadataValules_1();
    test_kmj_plugin_GetNumMetadataValules_2();
    test_kmj_plugin_GetNumMetadataValules_3();
    test_kmj_plugin_GetNodeMetadataKeys();
    test_kmj_plugin_GetNodeMetadataValues();
    test_kmj_plugin_DoGetMetadataKeys_1();
    test_kmj_plugin_DoGetMetadataKeys_2();
    test_kmj_plugin_DoGetMetadataKeys_3();
    test_kmj_plugin_DoGetMetadataKeys_4();
    test_kmj_plugin_DoGetMetadataKeys_5();
    test_kmj_plugin_DoGetMetadataKeys_6();
    test_kmj_plugin_DoGetMetadataKeys_7();
    test_kmj_plugin_DoGetMetadataKeys_8();
    test_kmj_plugin_DoGetMetadataKeys_9();
    test_kmj_plugin_DoGetMetadataValues_1();
    test_kmj_plugin_DoGetMetadataValues_2();
    test_kmj_plugin_DoGetMetadataValues_3();
    test_kmj_plugin_DoGetMetadataValues_4();
    test_kmj_plugin_DoGetMetadataValues_5();
    test_kmj_plugin_DoGetMetadataValues_6();
    test_kmj_plugin_DoGetMetadataValues_7();
    test_kmj_plugin_DoGetMetadataValues_8();
    test_kmj_plugin_DoGetMetadataValues_9();
    test_kmj_plugin_DoGetMetadataValues_10();
    test_kmj_plugin_DoGetMetadataValues_11();
    test_kmj_plugin_DoGetMetadataValues_12();
    test_kmj_plugin_DoGetMetadataValues_13();
    test_kmj_plugin_DoGetMetadataValues_14();
    test_kmj_plugin_DoGetMetadataValues_15();
    test_kmj_plugin_ReleaseNodeMetadataValues_1();
    test_kmj_plugin_ReleaseNodeMetadataValues_2();
    test_kmj_plugin_ReleaseNodeMetadataValues_3();
    test_kmj_plugin_ReleaseNodeMetadataValues_4();
    test_kmj_plugin_GetIso8601Time_1();
    test_kmj_plugin_GetIso8601Time_2();
    test_kmj_plugin_GetIso8601Time_3();
    test_kmj_plugin_GetIso8601Time_4();
    test_kmj_plugin_GetDuration_1();
    test_kmj_plugin_GetDuration_2();
    test_kmj_plugin_GetDuration_3();
    test_kmj_plugin_GetDuration_4();

  // test access data through file
  test_kmj_dataaccess_DrmPluginGetDataLen_file();
  test_kmj_dataaccess_DrmPluginSeekData_file();
  test_kmj_dataaccess_DrmPluginReadData_file();

  // test access data through buffer
  test_kmj_dataaccess_DrmPluginGetDataLen_buffer();
  test_kmj_dataaccess_DrmPluginSeekData_buffer();
  test_kmj_dataaccess_DrmPluginReadData_buffer();

  // test recognize content type
  test_kmj_recognizer_RecognizeContentType_mp4();
  test_kmj_recognizer_RecognizeContentType_mp3();
  test_kmj_recognizer_RecognizeContentType_amr();
  test_kmj_recognizer_RecognizeContentType_aac();
  test_kmj_recognizer_RecognizeContentType_unknown();

  // test recognizer
  test_kmj_recognizer_Recognize();

    iObserver->TestCompleted(*iTestCase);
}

