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
#ifndef TEST_PV_PLAYER_ENGINE_TESTSET1_H_INCLUDED
#include "test_pv_player_engine_testset1.h"
#endif

#ifndef OSCL_ERROR_CODES_H_INCLUDED
#include "oscl_error_codes.h"
#endif

#ifndef OSCL_TICKCOUNT_H_INCLUDED
#include "oscl_tickcount.h"
#endif

#ifndef PV_PLAYER_DATASINKPVMFNODE_H_INCLUDED
#include "pv_player_datasinkpvmfnode.h"
#endif

#ifndef PVMI_MEDIA_IO_FILEOUTPUT_H_INCLUDED
#include "pvmi_media_io_fileoutput.h"
#endif

#ifndef PV_MEDIA_OUTPUT_NODE_FACTORY_H_INCLUDED
#include "pv_media_output_node_factory.h"
#endif

#ifndef OSCL_UTF8CONV_H
#include "oscl_utf8conv.h"
#endif

#ifndef PVMI_KVP_H_INCLUDED
#include "pvmi_kvp.h"
#endif

#ifndef PVMI_KVP_UTIL_H_INCLUDED
#include "pvmi_kvp_util.h"
#endif

#ifndef PVMF_TIMESTAMP_H_INCLUDED
#include "pvmf_timestamp.h"
#endif

#ifndef PVMF_TRACK_LEVEL_INFO_EXTENSION_H_INCLUDED
#include "pvmf_track_level_info_extension.h"
#endif

#ifndef PVMF_ERRORINFOMESSAGE_EXTENSION_H_INCLUDED
#include "pvmf_errorinfomessage_extension.h"
#endif

#ifndef PVMI_CONFIG_AND_CAPABILITY_H_INCLUDED
#include "pvmi_config_and_capability.h"
#endif

#ifndef PVMF_LOCAL_DATA_SOURCE_H_INCLUDED
#include "pvmf_local_data_source.h"
#endif

#ifndef PVMF_CPMPLUGIN_PASSTHRU_OMA1_FACTORY_H_INCLUDED
#include "pvmf_cpmplugin_passthru_oma1_factory.h"
#endif

#ifndef PVMF_CPMPLUGIN_PASSTHRU_OMA1_TYPES_H_INCLUDED
#include "pvmf_cpmplugin_passthru_oma1_types.h"
#endif

#include "pvmi_media_io_fileoutput.h"
#include "pv_media_output_node_factory.h"

#ifndef PVMF_SOURCE_CONTEXT_DATA_H_INCLUDED
#include "pvmf_source_context_data.h"
#endif

#ifndef PVMI_DATASTREAMSYNCINTERFACE_REF_FACTORY_H_INCLUDED
#include "pvmi_datastreamsyncinterface_ref_factory.h"
#endif

extern FILE* file;

#define MAX_AUDIO_TRACKS 256
#define MAX_VIDEO_TRACKS 256


//
// pvplayer_async_test_newdelete section
//
void pvplayer_async_test_newdelete::StartTest()
{
    AddToScheduler();
    RunIfNotReady();
}


void pvplayer_async_test_newdelete::Run()
{
    PVPlayerInterface *temp = NULL;
    int error = 0;

    OSCL_TRY(error, temp = PVPlayerFactory::CreatePlayer(this, this, this));
    if (error)
    {
        PVPATB_TEST_IS_TRUE(false);
    }
    else
    {
        PVPATB_TEST_IS_TRUE(PVPlayerFactory::DeletePlayer(temp));
        temp = NULL;

        PVPATB_TEST_IS_TRUE(true);
    }

    iObserver->TestCompleted(*iTestCase);
}


void pvplayer_async_test_newdelete::CommandCompleted(const PVCmdResponse& /*aResponse*/)
{
    // No callbacks in this test.
}


void pvplayer_async_test_newdelete::HandleErrorEvent(const PVAsyncErrorEvent& /*aEvent*/)
{
    // No callbacks in this test
}


void pvplayer_async_test_newdelete::HandleInformationalEvent(const PVAsyncInformationalEvent& /*aEvent*/)
{
    // No callbacks in this test
}



//
// pvplayer_async_test_openplaystopreset section
//
void pvplayer_async_test_openplaystopreset::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_openplaystopreset::Run()
{
    int error = 0;

    switch (iState)
    {
        case STATE_CREATE:
        {
            iPlayer = NULL;

            OSCL_TRY(error, iPlayer = PVPlayerFactory::CreatePlayer(this, this, this));
            if (error)
            {
                PVPATB_TEST_IS_TRUE(false);
                iObserver->TestCompleted(*iTestCase);
            }
            else
            {
                iState = STATE_ADDDATASOURCE;
                RunIfNotReady();
            }
        }
        break;

        case STATE_ADDDATASOURCE:
        {
            iDataSource = new PVPlayerDataSourceURL;
            oscl_UTF8ToUnicode(iFileName, oscl_strlen(iFileName), output, 512);
            wFileName.set(output, oscl_strlen(output));
            iDataSource->SetDataSourceURL(wFileName);
            iDataSource->SetDataSourceFormatType(iFileType);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }

        break;

        case STATE_INIT:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Init((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_VIDEO:
        {
            OSCL_wHeapString<OsclMemAllocator> SinkFileName;
            SinkFileName = OUTPUTNAME_PREPEND_WSTRING;
            SinkFileName += _STRLIT_WCHAR("test_player_openplaystop_");
            OSCL_wHeapString<OsclMemAllocator> inputfilename;
            RetrieveFilename(wFileName.get_str(), inputfilename);
            SinkFileName += inputfilename;
            SinkFileName += _STRLIT_WCHAR("_video.dat");

            iMIOFileOutVideo = iMioFactory->CreateVideoOutput((OsclAny*) & SinkFileName, MEDIATYPE_VIDEO, iCompressedVideo);
            iIONodeVideo = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutVideo);
            iDataSinkVideo = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkVideo)->SetDataSinkNode(iIONodeVideo);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_AUDIO:
        {
            OSCL_wHeapString<OsclMemAllocator> SinkFileName;
            SinkFileName = OUTPUTNAME_PREPEND_WSTRING;
            SinkFileName += _STRLIT_WCHAR("test_player_openplaystop_");
            OSCL_wHeapString<OsclMemAllocator> inputfilename;
            RetrieveFilename(wFileName.get_str(), inputfilename);
            SinkFileName += inputfilename;
            SinkFileName += _STRLIT_WCHAR("_audio.dat");

            iMIOFileOutAudio = iMioFactory->CreateAudioOutput((OsclAny*) & SinkFileName, MEDIATYPE_AUDIO, iCompressedAudio);
            iIONodeAudio = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutAudio);
            iDataSinkAudio = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkAudio)->SetDataSinkNode(iIONodeAudio);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_TEXT:
        {
            OSCL_wHeapString<OsclMemAllocator> SinkFileName;
            SinkFileName = OUTPUTNAME_PREPEND_WSTRING;
            SinkFileName += _STRLIT_WCHAR("test_player_openplaystop_");
            OSCL_wHeapString<OsclMemAllocator> inputfilename;
            RetrieveFilename(wFileName.get_str(), inputfilename);
            SinkFileName += inputfilename;
            SinkFileName += _STRLIT_WCHAR("_text.dat");

            iMIOFileOutText = iMioFactory->CreateTextOutput((OsclAny*) & SinkFileName, MEDIATYPE_TEXT);
            iIONodeText = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutText);
            iDataSinkText = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkText)->SetDataSinkNode(iIONodeText);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkText, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_PREPARE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Prepare((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_START:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Start((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_STOP:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Stop((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_VIDEO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_AUDIO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_TEXT:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkText, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_RESET:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Reset((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASOURCE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_CLEANUPANDCOMPLETE:
        {
            PVPATB_TEST_IS_TRUE(PVPlayerFactory::DeletePlayer(iPlayer));
            iPlayer = NULL;

            delete iDataSource;
            iDataSource = NULL;

            delete iDataSinkVideo;
            iDataSinkVideo = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeVideo);
            iIONodeVideo = NULL;

            iMioFactory->DestroyVideoOutput(iMIOFileOutVideo);
            iMIOFileOutVideo = NULL;

            delete iDataSinkAudio;
            iDataSinkAudio = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeAudio);
            iIONodeAudio = NULL;

            iMioFactory->DestroyAudioOutput(iMIOFileOutAudio);
            iMIOFileOutAudio = NULL;

            delete iDataSinkText;
            iDataSinkText = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeText);
            iIONodeText = NULL;

            iMioFactory->DestroyTextOutput(iMIOFileOutText);
            iMIOFileOutText = NULL;

            iObserver->TestCompleted(*iTestCase);
        }
        break;

        default:
            break;

    }
}


void pvplayer_async_test_openplaystopreset::CommandCompleted(const PVCmdResponse& aResponse)
{
    if (aResponse.GetCmdId() != iCurrentCmdId)
    {
        // Wrong command ID.
        PVPATB_TEST_IS_TRUE(false);
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
        return;
    }

    if (aResponse.GetContext() != NULL)
    {
        if (aResponse.GetContext() == (OsclAny*)&iContextObject)
        {
            if (iContextObject != iContextObjectRefValue)
            {
                // Context data value was corrupted
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                return;
            }
        }
        else
        {
            // Context data pointer was corrupted
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
            return;
        }
    }

    switch (iState)
    {
        case STATE_ADDDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_INIT;
                RunIfNotReady();
            }
            else
            {
                // AddDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_INIT:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                // Init failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_TEXT;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_TEXT:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_PREPARE;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_PREPARE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_START;
                RunIfNotReady();
            }
            else
            {
                // Prepare failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_START:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_STOP;
                RunIfNotReady(15000000);
            }
            else
            {
                // Start failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_STOP:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                // Stop failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_TEXT;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_TEXT:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_RESET;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_RESET:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASOURCE;
                RunIfNotReady();
            }
            else
            {
                // Reset failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                PVPATB_TEST_IS_TRUE(true);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        default:
        {
            // Testing error if this is reached
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;
    }
}


void pvplayer_async_test_openplaystopreset::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    switch (aEvent.GetEventType())
    {
        case PVMFErrResourceConfiguration:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrResource:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrCorrupt:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrProcessing:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        default:
            // Unknown error and just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;
    }

    // Wait for engine to handle the error
    Cancel();
}


void pvplayer_async_test_openplaystopreset::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingStart)
    {
        // do nothing
    }
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingComplete)
    {
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
    }
    if (aEvent.GetEventType() == PVMFInfoPositionStatus)
    {
        PVInterface* iface = (PVInterface*)(aEvent.GetEventExtensionInterface());
        if (iface == NULL)
        {
            return;
        }
        PVUuid infomsguuid = PVMFErrorInfoMessageInterfaceUUID;
        PVMFErrorInfoMessageInterface* infomsgiface = NULL;
        if (iface->queryInterface(infomsguuid, (PVInterface*&)infomsgiface) == true)
        {
            int32 infocode;
            PVUuid infouuid;
            infomsgiface->GetCodeUUID(infocode, infouuid);
            if ((infouuid == PVPlayerErrorInfoEventTypesUUID) && (infocode == PVPlayerInfoPlaybackPositionStatus))
            {
                PVPPlaybackPosition aPos1;
                uint32 aPos2;

                aPos1.iPosUnit = PVPPBPOSUNIT_MILLISEC;
                iPlayer->GetCurrentPositionSync(aPos1);

                uint8* localbuf = aEvent.GetLocalBuffer();
                if (aEvent.GetLocalBufferSize() == 8 && localbuf[0] == 1)
                {
                    oscl_memcpy(&aPos2, &localbuf[4], sizeof(uint32));
                }
                else
                {
                    aPos2 = 0;
                }

                if (!(aPos1.iPosValue.millisec_value <= aPos2 + 50))
                {
                    PVPATB_TEST_IS_TRUE(false);
                }
            }
        }
    }
}


//
// pvplayer_async_test_cpmopenplaystopreset section
//
void pvplayer_async_test_cpmopenplaystopreset::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_cpmopenplaystopreset::Run()
{
    int error = 0;

    switch (iState)
    {
        case STATE_CREATE:
        {
            iPlayer = NULL;

            OSCL_TRY(error, iPlayer = PVPlayerFactory::CreatePlayer(this, this, this));
            if (error)
            {
                PVPATB_TEST_IS_TRUE(false);
                iObserver->TestCompleted(*iTestCase);
            }
            else
            {
                iState = STATE_ADDDATASOURCE;
                RunIfNotReady();
            }
        }
        break;

        case STATE_ADDDATASOURCE:
        {
            iDataSource = new PVPlayerDataSourceURL;
            oscl_UTF8ToUnicode(iFileName, oscl_strlen(iFileName), output, 512);
            wFileName.set(output, oscl_strlen(output));
            iDataSource->SetDataSourceURL(wFileName);
            iDataSource->SetDataSourceFormatType(iFileType);

            //This illustrates the use of CPM with a file source.

            //Connect to plugin registry
            PVMFStatus status;
            status = iPluginRegistryClient.Connect();
            if (status != PVMFSuccess)
            {
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                break;
            }
            //Create & the passthru plugin factory.
            iPluginFactory = new PVMFOma1PassthruPluginFactory();
            if (!iPluginFactory)
            {
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                break;
            }
            //Register the passthru plugin factory.
            iPluginMimeType = PVMF_CPM_MIME_PASSTHRU_OMA1;
            if (iPluginRegistryClient.RegisterPlugin(iPluginMimeType, *iPluginFactory) != PVMFSuccess)
            {
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                break;
            }

            if (iUsingDataStreamInput)
            {
                //Create a data stream factory for recognizer (a)
                int32 leavecode = 0;
                OSCL_TRY(leavecode, iDataStreamFactory = OSCL_STATIC_CAST(PVMFDataStreamFactory*, new PVMIDataStreamSyncInterfaceRefFactory(wFileName)));
                OSCL_FIRST_CATCH_ANY(leavecode,
                                     PVPATB_TEST_IS_TRUE(false);
                                     iState = STATE_CLEANUPANDCOMPLETE;
                                     RunIfNotReady();
                                     break;
                                    );

                iSourceContextData = new PVMFSourceContextData();
                iSourceContextData->EnableCommonSourceContext();
                iSourceContextData->CommonData()->iRecognizerDataStreamFactory = iDataStreamFactory;

                iDataSource->SetDataSourceContextData((OsclAny*)iSourceContextData);
            }
            else
            {
                //Create a data source
                iLocalDataSource = new PVMFLocalDataSource();

                iDataSource->SetDataSourceContextData((OsclAny*)iLocalDataSource);
            }


            //Add the data source
            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }

        break;

        case STATE_INIT:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Init((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_VIDEO:
        {
            OSCL_wHeapString<OsclMemAllocator> SinkFileName;
            SinkFileName = OUTPUTNAME_PREPEND_WSTRING;
            SinkFileName += _STRLIT_WCHAR("test_player_cpmopenplaystop_");
            OSCL_wHeapString<OsclMemAllocator> inputfilename;
            RetrieveFilename(wFileName.get_str(), inputfilename);
            SinkFileName += inputfilename;
            SinkFileName += _STRLIT_WCHAR("_video.dat");

            iMIOFileOutVideo = iMioFactory->CreateVideoOutput((OsclAny*) & SinkFileName, MEDIATYPE_VIDEO, iCompressedVideo);
            iIONodeVideo = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutVideo);
            iDataSinkVideo = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkVideo)->SetDataSinkNode(iIONodeVideo);


            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_AUDIO:
        {
            OSCL_wHeapString<OsclMemAllocator> SinkFileName;
            SinkFileName = OUTPUTNAME_PREPEND_WSTRING;
            SinkFileName += _STRLIT_WCHAR("test_player_cpmopenplaystop_");
            OSCL_wHeapString<OsclMemAllocator> inputfilename;
            RetrieveFilename(wFileName.get_str(), inputfilename);
            SinkFileName += inputfilename;
            SinkFileName += _STRLIT_WCHAR("_audio.dat");

            iMIOFileOutAudio = iMioFactory->CreateAudioOutput((OsclAny*) & SinkFileName, MEDIATYPE_AUDIO, iCompressedAudio);
            iIONodeAudio = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutAudio);
            iDataSinkAudio = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkAudio)->SetDataSinkNode(iIONodeAudio);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_TEXT:
        {
            OSCL_wHeapString<OsclMemAllocator> SinkFileName;
            SinkFileName = OUTPUTNAME_PREPEND_WSTRING;
            SinkFileName += _STRLIT_WCHAR("test_player_cpmopenplaystop_");
            OSCL_wHeapString<OsclMemAllocator> inputfilename;
            RetrieveFilename(wFileName.get_str(), inputfilename);
            SinkFileName += inputfilename;
            SinkFileName += _STRLIT_WCHAR("_text.dat");

            iMIOFileOutText = iMioFactory->CreateTextOutput((OsclAny*) & SinkFileName, MEDIATYPE_TEXT);
            iIONodeText = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutText);
            iDataSinkText = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkText)->SetDataSinkNode(iIONodeText);


            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkText, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_PREPARE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Prepare((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_START:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Start((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_STOP:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Stop((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_VIDEO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_AUDIO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_TEXT:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkText, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_RESET:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Reset((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASOURCE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_CLEANUPANDCOMPLETE:
        {
            PVPATB_TEST_IS_TRUE(PVPlayerFactory::DeletePlayer(iPlayer));
            iPlayer = NULL;

            delete iDataSource;
            iDataSource = NULL;

            delete iDataSinkVideo;
            iDataSinkVideo = NULL;

            delete iDataStreamFactory;
            iDataStreamFactory = NULL;

            delete iSourceContextData;
            iSourceContextData = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeVideo);
            iIONodeVideo = NULL;

            iMioFactory->DestroyVideoOutput(iMIOFileOutVideo);
            iMIOFileOutVideo = NULL;

            delete iDataSinkAudio;
            iDataSinkAudio = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeAudio);
            iIONodeAudio = NULL;

            iMioFactory->DestroyAudioOutput(iMIOFileOutAudio);
            iMIOFileOutAudio = NULL;

            delete iDataSinkText;
            iDataSinkText = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeText);
            iIONodeText = NULL;

            iMioFactory->DestroyTextOutput(iMIOFileOutText);
            iMIOFileOutText = NULL;

            //close and cleanup the CPM plugin registry.
            iPluginRegistryClient.Close();

            //delete the plugin factory.
            if (iPluginFactory)
            {
                delete iPluginFactory;
                iPluginFactory = NULL;
            }

            delete iLocalDataSource;
            iLocalDataSource = NULL;

            iObserver->TestCompleted(*iTestCase);
        }
        break;

        default:
            break;

    }
}


void pvplayer_async_test_cpmopenplaystopreset::CommandCompleted(const PVCmdResponse& aResponse)
{
    if (aResponse.GetCmdId() != iCurrentCmdId)
    {
        // Wrong command ID.
        PVPATB_TEST_IS_TRUE(false);
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
        return;
    }

    if (aResponse.GetContext() != NULL)
    {
        if (aResponse.GetContext() == (OsclAny*)&iContextObject)
        {
            if (iContextObject != iContextObjectRefValue)
            {
                // Context data value was corrupted
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                return;
            }
        }
        else
        {
            // Context data pointer was corrupted
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
            return;
        }
    }

    switch (iState)
    {
        case STATE_ADDDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_INIT;
                RunIfNotReady();
            }
            else
            {
                // AddDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_INIT:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                // Init failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_TEXT;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_TEXT:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_PREPARE;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_PREPARE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_START;
                RunIfNotReady();
            }
            else
            {
                // Prepare failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_START:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_STOP;
                RunIfNotReady(15000000);
            }
            else
            {
                // Start failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_STOP:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                // Stop failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_TEXT;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_TEXT:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_RESET;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_RESET:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASOURCE;
                RunIfNotReady();
            }
            else
            {
                // Reset failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                PVPATB_TEST_IS_TRUE(true);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        default:
        {
            // Testing error if this is reached
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;
    }
}


void pvplayer_async_test_cpmopenplaystopreset::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    switch (aEvent.GetEventType())
    {
        case PVMFErrResourceConfiguration:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrResource:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrCorrupt:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrProcessing:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        default:
            // Unknown error and just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;
    }

    // Wait for engine to handle the error
    Cancel();
}


void pvplayer_async_test_cpmopenplaystopreset::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingStart)
    {
        // do nothing
    }
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingComplete)
    {
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
    }
    if (aEvent.GetEventType() == PVMFInfoPositionStatus)
    {
        PVInterface* iface = (PVInterface*)(aEvent.GetEventExtensionInterface());
        if (iface == NULL)
        {
            return;
        }
        PVUuid infomsguuid = PVMFErrorInfoMessageInterfaceUUID;
        PVMFErrorInfoMessageInterface* infomsgiface = NULL;
        if (iface->queryInterface(infomsguuid, (PVInterface*&)infomsgiface) == true)
        {
            int32 infocode;
            PVUuid infouuid;
            infomsgiface->GetCodeUUID(infocode, infouuid);
            if ((infouuid == PVPlayerErrorInfoEventTypesUUID) && (infocode == PVPlayerInfoPlaybackPositionStatus))
            {
                PVPPlaybackPosition aPos1;
                uint32 aPos2;

                aPos1.iPosUnit = PVPPBPOSUNIT_MILLISEC;
                iPlayer->GetCurrentPositionSync(aPos1);

                uint8* localbuf = aEvent.GetLocalBuffer();
                if (aEvent.GetLocalBufferSize() == 8 && localbuf[0] == 1)
                {
                    oscl_memcpy(&aPos2, &localbuf[4], sizeof(uint32));
                }
                else
                {
                    aPos2 = 0;
                }

                if (!(aPos1.iPosValue.millisec_value <= aPos2 + 50))
                {
                    PVPATB_TEST_IS_TRUE(false);
                }
            }
        }
    }
}

//
// pvplayer_async_test_metadata section
//

// Expected number of keys and values
#define METADATA_GETMETADATAKEYS1_NUMKEYS 21
#define METADATA_GETMETADATAVALUES1_NUMVALUES 28
#define METADATA_GETMETADATAKEYS2_QUERYSTRING "codec-info/video"
#define METADATA_GETMETADATAKEYS2_NUMKEYS 3
#define METADATA_GETMETADATAVALUES2_NUMVALUES 3
#define METADATA_GETMETADATAKEYS3_NUMKEYS 21
#define METADATA_GETMETADATAVALUES3_NUMVALUES 28
#define METADATA_GETMETADATAKEYSSEG_NUMKEYS 24
#define METADATA_GETMETADATAVALUESSEG_NUMVALUES 31

// The metadata keys and values to compare
// Modify the following #defines for different source files
#define METADATA_TITLE_KEY "title;valtype=wchar*"
#define METADATA_TITLE_VALUE "alien-14-x-1d66-x-128-10-g4.mp4"
#define METADATA_TITLE_VALUE_W _STRLIT_WCHAR("alien-14-x-1d66-x-128-10-g4.mp4")
#define METADATA_AUTHOR_KEY "author;valtype=wchar*"
#define METADATA_AUTHOR_VALUE "PacketVideo Embedded Solutions Test Content"
#define METADATA_AUTHOR_VALUE_W _STRLIT_WCHAR("PacketVideo Embedded Solutions Test Content")
#define METADATA_DESCRIPTION_KEY "description;valtype=wchar*"
#define METADATA_DESCRIPTION_VALUE "This test clip is in support of Embedded Solutions project testing."
#define METADATA_DESCRIPTION_VALUE_W _STRLIT_WCHAR("This test clip is in support of Embedded Solutions project testing.")
#define METADATA_RATING_KEY "rating;valtype=wchar*"
#define METADATA_RATING_VALUE "Test Material Only; Not for general distribution"
#define METADATA_RATING_VALUE_W _STRLIT_WCHAR("Test Material Only; Not for general distribution")
#define METADATA_COPYRIGHT_KEY "copyright;valtype=wchar*"
#define METADATA_COPYRIGHT_VALUE "PacketVideo, Corp, Test Suite 2003"
#define METADATA_COPYRIGHT_VALUE_W _STRLIT_WCHAR("PacketVideo, Corp, Test Suite 2003")
#define METADATA_VERSION_KEY "version;valtype=wchar*"
#define METADATA_VERSION_VALUE "Content V3.3 PVAuthor V3.3 Build 005"
#define METADATA_VERSION_VALUE_W _STRLIT_WCHAR("Content V3.3 PVAuthor V3.3 Build 005")
#define METADATA_DATE_KEY "date;valtype=wchar*"
#define METADATA_DATE_VALUE "20030210T193127.546Z"
#define METADATA_DATE_VALUE_W _STRLIT_WCHAR("20030210T193127.546Z")
#define METADATA_DURATION_KEY "duration;valtype=uint32;timescale=1000"
#define METADATA_DURATION_VALUE 42749
#define METADATA_NUMTRACKS_KEY "num-tracks;valtype=uint32"
#define METADATA_NUMTRACKS_VALUE 2
#define METADATA_TRACKINFOTYPE0_KEY "track-info/type;valtype=char*;index=0"
#define METADATA_TRACKINFOTYPE0_VALUE "video/MP4V-ES"
#define METADATA_TRACKINFOTYPE1_KEY "track-info/type;valtype=char*;index=1"
#define METADATA_TRACKINFOTYPE1_VALUE "X-AMR-IETF-SEPARATE"
#define METADATA_TRACKINFO_AUDIOFORMAT1_KEY "track-info/audio/format;valtype=char*;index=1"
#define METADATA_TRACKINFO_AUDIOFORMAT1_VALUE "X-AMR-IETF-SEPARATE"
#define METADATA_TRACKINFODURATION0_KEY "track-info/duration;valtype=uint32;index=0;timescale=1000"
#define METADATA_TRACKINFODURATION0_VALUE 42749
#define METADATA_TRACKINFODURATION1_KEY "track-info/duration;valtype=uint32;index=1;timescale=1000"
#define METADATA_TRACKINFODURATION1_VALUE 42360
#define METADATA_TRACKINFOBITRATE0_KEY "track-info/bit-rate;valtype=uint32;index=0"
#define METADATA_TRACKINFOBITRATE0_VALUE 6770
#define METADATA_TRACKINFOBITRATE1_KEY "track-info/bit-rate;valtype=uint32;index=1"
#define METADATA_TRACKINFOBITRATE1_VALUE 4750
#define METADATA_TRACKINFOVIDEOWIDTH0_KEY "track-info/video/width;valtype=uint32;index=0"
#define METADATA_TRACKINFOVIDEOWIDTH0_VALUE 128
#define METADATA_TRACKINFOVIDEOWIDTH1_KEY "track-info/video/width;valtype=uint32;index=1"
#define METADATA_TRACKINFOVIDEOWIDTH1_VALUE 0
#define METADATA_TRACKINFOVIDEOHEIGHT0_KEY "track-info/video/height;valtype=uint32;index=0"
#define METADATA_TRACKINFOVIDEOHEIGHT0_VALUE 96
#define METADATA_TRACKINFOVIDEOHEIGHT1_KEY "track-info/video/height;valtype=uint32;index=1"
#define METADATA_TRACKINFOVIDEOHEIGHT1_VALUE 0
#define METADATA_CODECINFOVIDEOWIDTH_KEY "codec-info/video/width;valtype=uint32"
#define METADATA_CODECINFOVIDEOWIDTH_VALUE 128
#define METADATA_CODECINFOVIDEOHEIGHT_KEY "codec-info/video/height;valtype=uint32"
#define METADATA_CODECINFOVIDEOHEIGHT_VALUE 96

int32 pvplayer_async_test_metadata::CheckMetadataValue(Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList)
{
    uint32 valcount = 0;
    int32 errorbitarray = 0; // 0x1=> Error in returned value;
    // 0x2=> Number of values entries do not match the known expected
    // 0x4=> Unknown value type

    for (uint32 i = 0; i < aValueList.size(); i++)
    {
        if (oscl_strcmp(aValueList[i].key, _STRLIT_CHAR(METADATA_TITLE_KEY)) == 0)
        {
            valcount++;

            switch (GetValTypeFromKeyString(aValueList[i].key))
            {
                case PVMI_KVPVALTYPE_WCHARPTR:
                    if (oscl_strcmp(aValueList[i].value.pWChar_value, METADATA_TITLE_VALUE_W) != 0)
                    {
                        errorbitarray |= 0x1;
                    }
                    break;

                case PVMI_KVPVALTYPE_CHARPTR:
                    if (oscl_strcmp(aValueList[i].value.pChar_value, _STRLIT_CHAR(METADATA_TITLE_VALUE)) != 0)
                    {
                        errorbitarray |= 0x1;
                    }
                    break;
                default:
                    // Unknown value type so can't compare
                    errorbitarray |= 0x4;
                    break;
            }
        }
        else if (oscl_strcmp(aValueList[i].key, _STRLIT_CHAR(METADATA_AUTHOR_KEY)) == 0)
        {
            valcount++;

            switch (GetValTypeFromKeyString(aValueList[i].key))
            {
                case PVMI_KVPVALTYPE_WCHARPTR:
                    if (oscl_strcmp(aValueList[i].value.pWChar_value, METADATA_AUTHOR_VALUE_W) != 0)
                    {
                        errorbitarray |= 0x1;
                    }
                    break;

                case PVMI_KVPVALTYPE_CHARPTR:
                    if (oscl_strcmp(aValueList[i].value.pChar_value, _STRLIT_CHAR(METADATA_AUTHOR_VALUE)) != 0)
                    {
                        errorbitarray |= 0x1;
                    }
                    break;

                default:
                    // Unknown value type so can't compare
                    errorbitarray |= 0x4;
                    break;
            }
        }
        else if (oscl_strcmp(aValueList[i].key, _STRLIT_CHAR(METADATA_DESCRIPTION_KEY)) == 0)
        {
            valcount++;

            switch (GetValTypeFromKeyString(aValueList[i].key))
            {
                case PVMI_KVPVALTYPE_WCHARPTR:
                    if (oscl_strcmp(aValueList[i].value.pWChar_value, METADATA_DESCRIPTION_VALUE_W) != 0)
                    {
                        errorbitarray |= 0x1;
                    }
                    break;

                case PVMI_KVPVALTYPE_CHARPTR:
                    if (oscl_strcmp(aValueList[i].value.pChar_value, _STRLIT_CHAR(METADATA_DESCRIPTION_VALUE)) != 0)
                    {
                        errorbitarray |= 0x1;
                    }
                    break;

                default:
                    // Unknown value type so can't compare
                    errorbitarray |= 0x4;
                    break;
            }
        }
        else if (oscl_strcmp(aValueList[i].key, _STRLIT_CHAR(METADATA_RATING_KEY)) == 0)
        {
            valcount++;

            switch (GetValTypeFromKeyString(aValueList[i].key))
            {
                case PVMI_KVPVALTYPE_WCHARPTR:
                    if (oscl_strcmp(aValueList[i].value.pWChar_value, METADATA_RATING_VALUE_W) != 0)
                    {
                        errorbitarray |= 0x1;
                    }
                    break;

                case PVMI_KVPVALTYPE_CHARPTR:
                    if (oscl_strcmp(aValueList[i].value.pChar_value, _STRLIT_CHAR(METADATA_RATING_VALUE)) != 0)
                    {
                        errorbitarray |= 0x1;
                    }
                    break;

                default:
                    // Unknown value type so can't compare
                    errorbitarray |= 0x4;
                    break;
            }
        }
        else if (oscl_strcmp(aValueList[i].key, _STRLIT_CHAR(METADATA_COPYRIGHT_KEY)) == 0)
        {
            valcount++;

            switch (GetValTypeFromKeyString(aValueList[i].key))
            {
                case PVMI_KVPVALTYPE_WCHARPTR:
                    if (oscl_strcmp(aValueList[i].value.pWChar_value, METADATA_COPYRIGHT_VALUE_W) != 0)
                    {
                        errorbitarray |= 0x1;
                    }
                    break;

                case PVMI_KVPVALTYPE_CHARPTR:
                    if (oscl_strcmp(aValueList[i].value.pChar_value, _STRLIT_CHAR(METADATA_COPYRIGHT_VALUE)) != 0)
                    {
                        errorbitarray |= 0x1;
                    }
                    break;

                default:
                    // Unknown value type so can't compare
                    errorbitarray |= 0x4;
                    break;
            }
        }
        else if (oscl_strcmp(aValueList[i].key, _STRLIT_CHAR(METADATA_VERSION_KEY)) == 0)
        {
            valcount++;

            switch (GetValTypeFromKeyString(aValueList[i].key))
            {
                case PVMI_KVPVALTYPE_WCHARPTR:
                    if (oscl_strcmp(aValueList[i].value.pWChar_value, METADATA_VERSION_VALUE_W) != 0)
                    {
                        errorbitarray |= 0x1;
                    }
                    break;

                case PVMI_KVPVALTYPE_CHARPTR:
                    if (oscl_strcmp(aValueList[i].value.pChar_value, _STRLIT_CHAR(METADATA_VERSION_VALUE)) != 0)
                    {
                        errorbitarray |= 0x1;
                    }
                    break;

                default:
                    // Unknown value type so can't compare
                    errorbitarray |= 0x4;
                    break;
            }
        }
        else if (oscl_strcmp(aValueList[i].key, _STRLIT_CHAR(METADATA_DATE_KEY)) == 0)
        {
            valcount++;

            switch (GetValTypeFromKeyString(aValueList[i].key))
            {
                case PVMI_KVPVALTYPE_WCHARPTR:
                    if (oscl_strcmp(aValueList[i].value.pWChar_value, METADATA_DATE_VALUE_W) != 0)
                    {
                        errorbitarray |= 0x1;
                    }
                    break;

                case PVMI_KVPVALTYPE_CHARPTR:
                    if (oscl_strcmp(aValueList[i].value.pChar_value, _STRLIT_CHAR(METADATA_DATE_VALUE)) != 0)
                    {
                        errorbitarray |= 0x1;
                    }
                    break;

                default:
                    // Unknown value type so can't compare
                    errorbitarray |= 0x4;
                    break;
            }
        }
        else if (oscl_strcmp(aValueList[i].key, _STRLIT_CHAR(METADATA_DURATION_KEY)) == 0)
        {
            valcount++;

            switch (GetValTypeFromKeyString(aValueList[i].key))
            {
                case PVMI_KVPVALTYPE_UINT32:
                    if (aValueList[i].value.uint32_value != METADATA_DURATION_VALUE)
                    {
                        errorbitarray |= 0x1;
                    }
                    break;

                case PVMI_KVPVALTYPE_INT32:
                    if (aValueList[i].value.int32_value != METADATA_DURATION_VALUE)
                    {
                        errorbitarray |= 0x1;
                    }
                    break;

                default:
                    // Unknown value type so can't compare
                    errorbitarray |= 0x4;
                    break;
            }
        }
        else if (oscl_strcmp(aValueList[i].key, _STRLIT_CHAR(METADATA_NUMTRACKS_KEY)) == 0)
        {
            valcount++;

            switch (GetValTypeFromKeyString(aValueList[i].key))
            {
                case PVMI_KVPVALTYPE_UINT32:
                    if (aValueList[i].value.uint32_value != METADATA_NUMTRACKS_VALUE)
                    {
                        errorbitarray |= 0x1;
                    }
                    break;

                case PVMI_KVPVALTYPE_INT32:
                    if (aValueList[i].value.int32_value != METADATA_NUMTRACKS_VALUE)
                    {
                        errorbitarray |= 0x1;
                    }
                    break;

                case PVMI_KVPVALTYPE_UINT8:
                    if (aValueList[i].value.uint8_value != METADATA_NUMTRACKS_VALUE)
                    {
                        errorbitarray |= 0x1;
                    }
                    break;

                default:
                    // Unknown value type so can't compare
                    errorbitarray |= 0x4;
                    break;
            }
        }
        else if (oscl_strcmp(aValueList[i].key, _STRLIT_CHAR(METADATA_TRACKINFOTYPE0_KEY)) == 0)
        {
            valcount++;

            switch (GetValTypeFromKeyString(aValueList[i].key))
            {
                case PVMI_KVPVALTYPE_CHARPTR:
                    if (oscl_strcmp(aValueList[i].value.pChar_value, _STRLIT_CHAR(METADATA_TRACKINFOTYPE0_VALUE)) != 0)
                    {
                        errorbitarray |= 0x1;
                    }
                    break;

                default:
                    // Unknown value type so can't compare
                    errorbitarray |= 0x4;
                    break;
            }
        }
        else if (oscl_strcmp(aValueList[i].key, _STRLIT_CHAR(METADATA_TRACKINFOTYPE1_KEY)) == 0)
        {
            valcount++;

            switch (GetValTypeFromKeyString(aValueList[i].key))
            {
                case PVMI_KVPVALTYPE_CHARPTR:
                    if (oscl_strcmp(aValueList[i].value.pChar_value, _STRLIT_CHAR(METADATA_TRACKINFOTYPE1_VALUE)) != 0)
                    {
                        errorbitarray |= 0x1;
                    }
                    break;

                default:
                    // Unknown value type so can't compare
                    errorbitarray |= 0x4;
                    break;
            }
        }
        else if (oscl_strcmp(aValueList[i].key, _STRLIT_CHAR(METADATA_TRACKINFODURATION0_KEY)) == 0)
        {
            valcount++;

            switch (GetValTypeFromKeyString(aValueList[i].key))
            {
                case PVMI_KVPVALTYPE_UINT32:
                    if (aValueList[i].value.uint32_value != METADATA_TRACKINFODURATION0_VALUE)
                    {
                        errorbitarray |= 0x1;
                    }
                    break;

                case PVMI_KVPVALTYPE_INT32:
                    if (aValueList[i].value.int32_value != METADATA_TRACKINFODURATION0_VALUE)
                    {
                        errorbitarray |= 0x1;
                    }
                    break;

                default:
                    // Unknown value type so can't compare
                    errorbitarray |= 0x4;
                    break;
            }
        }
        else if (oscl_strcmp(aValueList[i].key, _STRLIT_CHAR(METADATA_TRACKINFODURATION1_KEY)) == 0)
        {
            valcount++;

            switch (GetValTypeFromKeyString(aValueList[i].key))
            {
                case PVMI_KVPVALTYPE_UINT32:
                    if (aValueList[i].value.uint32_value != METADATA_TRACKINFODURATION1_VALUE)
                    {
                        errorbitarray |= 0x1;
                    }
                    break;

                case PVMI_KVPVALTYPE_INT32:
                    if (aValueList[i].value.int32_value != METADATA_TRACKINFODURATION1_VALUE)
                    {
                        errorbitarray |= 0x1;
                    }
                    break;

                default:
                    // Unknown value type so can't compare
                    errorbitarray |= 0x4;
                    break;
            }
        }
        else if (oscl_strcmp(aValueList[i].key, _STRLIT_CHAR(METADATA_TRACKINFOBITRATE0_KEY)) == 0)
        {
            valcount++;

            switch (GetValTypeFromKeyString(aValueList[i].key))
            {
                case PVMI_KVPVALTYPE_UINT32:
                    if (aValueList[i].value.uint32_value != METADATA_TRACKINFOBITRATE0_VALUE)
                    {
                        errorbitarray |= 0x1;
                    }
                    break;

                case PVMI_KVPVALTYPE_INT32:
                    if (aValueList[i].value.int32_value != METADATA_TRACKINFOBITRATE0_VALUE)
                    {
                        errorbitarray |= 0x1;
                    }
                    break;

                default:
                    // Unknown value type so can't compare
                    errorbitarray |= 0x4;
                    break;
            }
        }
        else if (oscl_strcmp(aValueList[i].key, _STRLIT_CHAR(METADATA_TRACKINFOBITRATE1_KEY)) == 0)
        {
            valcount++;

            switch (GetValTypeFromKeyString(aValueList[i].key))
            {
                case PVMI_KVPVALTYPE_UINT32:
                    if (aValueList[i].value.uint32_value != METADATA_TRACKINFOBITRATE1_VALUE)
                    {
                        errorbitarray |= 0x1;
                    }
                    break;

                case PVMI_KVPVALTYPE_INT32:
                    if (aValueList[i].value.int32_value != METADATA_TRACKINFOBITRATE1_VALUE)
                    {
                        errorbitarray |= 0x1;
                    }
                    break;

                default:
                    // Unknown value type so can't compare
                    errorbitarray |= 0x4;
                    break;
            }
        }
        else if (oscl_strcmp(aValueList[i].key, _STRLIT_CHAR(METADATA_TRACKINFOVIDEOWIDTH0_KEY)) == 0)
        {
            valcount++;

            switch (GetValTypeFromKeyString(aValueList[i].key))
            {
                case PVMI_KVPVALTYPE_UINT32:
                    if (aValueList[i].value.uint32_value != METADATA_TRACKINFOVIDEOWIDTH0_VALUE)
                    {
                        errorbitarray |= 0x1;
                    }
                    break;

                case PVMI_KVPVALTYPE_INT32:
                    if (aValueList[i].value.int32_value != METADATA_TRACKINFOVIDEOWIDTH0_VALUE)
                    {
                        errorbitarray |= 0x1;
                    }
                    break;

                case PVMI_KVPVALTYPE_UINT8:
                    if (aValueList[i].value.uint8_value != METADATA_TRACKINFOVIDEOWIDTH0_VALUE)
                    {
                        errorbitarray |= 0x1;
                    }
                    break;

                default:
                    // Unknown value type so can't compare
                    errorbitarray |= 0x4;
                    break;
            }
        }
        else if (oscl_strcmp(aValueList[i].key, _STRLIT_CHAR(METADATA_TRACKINFOVIDEOWIDTH1_KEY)) == 0)
        {
            valcount++;

            switch (GetValTypeFromKeyString(aValueList[i].key))
            {
                case PVMI_KVPVALTYPE_UINT32:
                    if (aValueList[i].value.uint32_value != METADATA_TRACKINFOVIDEOWIDTH1_VALUE)
                    {
                        errorbitarray |= 0x1;
                    }
                    break;

                case PVMI_KVPVALTYPE_INT32:
                    if (aValueList[i].value.int32_value != METADATA_TRACKINFOVIDEOWIDTH1_VALUE)
                    {
                        errorbitarray |= 0x1;
                    }
                    break;

                case PVMI_KVPVALTYPE_UINT8:
                    if (aValueList[i].value.uint8_value != METADATA_TRACKINFOVIDEOWIDTH1_VALUE)
                    {
                        errorbitarray |= 0x1;
                    }
                    break;

                default:
                    // Unknown value type so can't compare
                    errorbitarray |= 0x4;
                    break;
            }
        }
        else if (oscl_strcmp(aValueList[i].key, _STRLIT_CHAR(METADATA_TRACKINFOVIDEOHEIGHT0_KEY)) == 0)
        {
            valcount++;

            switch (GetValTypeFromKeyString(aValueList[i].key))
            {
                case PVMI_KVPVALTYPE_UINT32:
                    if (aValueList[i].value.uint32_value != METADATA_TRACKINFOVIDEOHEIGHT0_VALUE)
                    {
                        errorbitarray |= 0x1;
                    }
                    break;

                case PVMI_KVPVALTYPE_INT32:
                    if (aValueList[i].value.int32_value != METADATA_TRACKINFOVIDEOHEIGHT0_VALUE)
                    {
                        errorbitarray |= 0x1;
                    }
                    break;

                case PVMI_KVPVALTYPE_UINT8:
                    if (aValueList[i].value.uint8_value != METADATA_TRACKINFOVIDEOHEIGHT0_VALUE)
                    {
                        errorbitarray |= 0x1;
                    }
                    break;

                default:
                    // Unknown value type so can't compare
                    errorbitarray |= 0x4;
                    break;
            }
        }
        else if (oscl_strcmp(aValueList[i].key, _STRLIT_CHAR(METADATA_TRACKINFOVIDEOHEIGHT1_KEY)) == 0)
        {
            valcount++;

            switch (GetValTypeFromKeyString(aValueList[i].key))
            {
                case PVMI_KVPVALTYPE_UINT32:
                    if (aValueList[i].value.uint32_value != METADATA_TRACKINFOVIDEOHEIGHT1_VALUE)
                    {
                        errorbitarray |= 0x1;
                    }
                    break;

                case PVMI_KVPVALTYPE_INT32:
                    if (aValueList[i].value.int32_value != METADATA_TRACKINFOVIDEOHEIGHT1_VALUE)
                    {
                        errorbitarray |= 0x1;
                    }
                    break;

                case PVMI_KVPVALTYPE_UINT8:
                    if (aValueList[i].value.uint8_value != METADATA_TRACKINFOVIDEOHEIGHT1_VALUE)
                    {
                        errorbitarray |= 0x1;
                    }
                    break;

                default:
                    // Unknown value type so can't compare
                    errorbitarray |= 0x4;
                    break;
            }
        }
        else if (oscl_strcmp(aValueList[i].key, _STRLIT_CHAR(METADATA_CODECINFOVIDEOWIDTH_KEY)) == 0)
        {
            valcount++;

            switch (GetValTypeFromKeyString(aValueList[i].key))
            {
                case PVMI_KVPVALTYPE_UINT32:
                    if (aValueList[i].value.uint32_value != METADATA_CODECINFOVIDEOWIDTH_VALUE)
                    {
                        errorbitarray |= 0x1;
                    }
                    break;

                case PVMI_KVPVALTYPE_INT32:
                    if (aValueList[i].value.int32_value != METADATA_CODECINFOVIDEOWIDTH_VALUE)
                    {
                        errorbitarray |= 0x1;
                    }
                    break;

                case PVMI_KVPVALTYPE_UINT8:
                    if (aValueList[i].value.uint8_value != METADATA_CODECINFOVIDEOWIDTH_VALUE)
                    {
                        errorbitarray |= 0x1;
                    }
                    break;

                default:
                    // Unknown value type so can't compare
                    errorbitarray |= 0x4;
                    break;
            }
        }
        else if (oscl_strcmp(aValueList[i].key, _STRLIT_CHAR(METADATA_CODECINFOVIDEOHEIGHT_KEY)) == 0)
        {
            valcount++;

            switch (GetValTypeFromKeyString(aValueList[i].key))
            {
                case PVMI_KVPVALTYPE_UINT32:
                    if (aValueList[i].value.uint32_value != METADATA_CODECINFOVIDEOHEIGHT_VALUE)
                    {
                        errorbitarray |= 0x1;
                    }
                    break;

                case PVMI_KVPVALTYPE_INT32:
                    if (aValueList[i].value.int32_value != METADATA_CODECINFOVIDEOHEIGHT_VALUE)
                    {
                        errorbitarray |= 0x1;
                    }
                    break;

                case PVMI_KVPVALTYPE_UINT8:
                    if (aValueList[i].value.uint8_value != METADATA_CODECINFOVIDEOHEIGHT_VALUE)
                    {
                        errorbitarray |= 0x1;
                    }
                    break;

                default:
                    // Unknown value type so can't compare
                    errorbitarray |= 0x4;
                    break;
            }
        }
    }

    if (aValueList.size() != valcount)
    {
        errorbitarray |= 0x2;
    }

    return errorbitarray;
}


void pvplayer_async_test_metadata::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_metadata::Run()
{
    int32 error = 0;

    switch (iState)
    {
        case STATE_CREATE:
        {
            iPlayer = NULL;

            OSCL_TRY(error, iPlayer = PVPlayerFactory::CreatePlayer(this, this, this));
            if (error)
            {
                PVPATB_TEST_IS_TRUE(false);
                iObserver->TestCompleted(*iTestCase);
            }
            else
            {
                iState = STATE_ADDDATASOURCE;
                RunIfNotReady();
            }
        }
        break;

        case STATE_ADDDATASOURCE:
        {
            iDataSource = new PVPlayerDataSourceURL;

            OSCL_wHeapString<OsclMemAllocator> sourcefile = SOURCENAME_PREPEND_WSTRING;
            sourcefile += _STRLIT_WCHAR("test_metadata.mp4");
            iDataSource->SetDataSourceURL(sourcefile);

            iDataSource->SetDataSourceFormatType(PVMF_MIME_MPEG4FF);
            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }

        break;

        case STATE_INIT:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Init((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        // Tests the Ability to get Metadata after the Init State
        case STATE_GETMETADATAKEYS1:
        {
            iKeyList.clear();
            OSCL_TRY(error, iCurrentCmdId = iPlayer->GetMetadataKeys(iKeyList, 0, -1, NULL, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_GETMETADATAVALUES1:
        {
            iValueList.clear();
            iNumAvailableValues = 0;
            OSCL_TRY(error, iCurrentCmdId = iPlayer->GetMetadataValues(iKeyList, 0, -1, iNumAvailableValues, iValueList, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;


        case STATE_ADDDATASINK_VIDEO:
        {
            OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
            sinkfile += _STRLIT_WCHAR("test_player_metadata_video.dat");

            iMIOFileOutVideo = iMioFactory->CreateVideoOutput((OsclAny*) & sinkfile, MEDIATYPE_VIDEO, iCompressedVideo);
            iIONodeVideo = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutVideo);
            iDataSinkVideo = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkVideo)->SetDataSinkNode(iIONodeVideo);


            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_AUDIO:
        {
            OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
            sinkfile += _STRLIT_WCHAR("test_player_metadata_audio.dat");

            iMIOFileOutAudio = iMioFactory->CreateAudioOutput((OsclAny*) & sinkfile, MEDIATYPE_AUDIO, iCompressedAudio);
            iIONodeAudio = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutAudio);
            iDataSinkAudio = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkAudio)->SetDataSinkNode(iIONodeAudio);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_PREPARE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Prepare((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        // Check The Ability to return MetaData  At the prepared state
        case STATE_GETMETADATAKEYS2:
        {
            iKeyList.clear();
            iKeyQueryString = _STRLIT_CHAR(METADATA_GETMETADATAKEYS2_QUERYSTRING);
            OSCL_TRY(error, iCurrentCmdId = iPlayer->GetMetadataKeys(iKeyList, 0, -1, iKeyQueryString.get_str(), (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_GETMETADATAVALUES2:
        {
            iValueList.clear();
            OSCL_TRY(error, iCurrentCmdId = iPlayer->GetMetadataValues(iKeyList, 0, -1, iNumAvailableValues, iValueList, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_GETMETADATAKEYSSEG:
        {
            iKeyList.clear();
            OSCL_TRY(error, iCurrentCmdId = iPlayer->GetMetadataKeys(iKeyList, 0, -1, NULL, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_GETMETADATAVALUESSEG1:
        {
            iValueList.clear();
            OSCL_TRY(error, iCurrentCmdId = iPlayer->GetMetadataValues(iKeyList, 0, -1, iNumAvailableValues, iValueList, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_GETMETADATAVALUESSEG2:
        {
            iValueListSeg1.clear();
            OSCL_TRY(error, iCurrentCmdId = iPlayer->GetMetadataValues(iKeyList, 0, 6, iNumAvailableValues, iValueListSeg1, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_GETMETADATAVALUESSEG3:
        {
            iValueListSeg2.clear();
            OSCL_TRY(error, iCurrentCmdId = iPlayer->GetMetadataValues(iKeyList, 6, -1, iNumAvailableValues, iValueListSeg2, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_START:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Start((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_STOP:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Stop((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        // Check The Ability to return MetaData after Stop is called
        case STATE_GETMETADATAKEYS3:
        {
            iKeyList.clear();
            OSCL_TRY(error, iCurrentCmdId = iPlayer->GetMetadataKeys(iKeyList, 0, -1, NULL, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_GETMETADATAVALUES3:
        {
            iValueList.clear();
            OSCL_TRY(error, iCurrentCmdId = iPlayer->GetMetadataValues(iKeyList, 0, -1, iNumAvailableValues, iValueList, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;


        case STATE_REMOVEDATASINK_VIDEO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_AUDIO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_RESET:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Reset((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASOURCE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_CLEANUPANDCOMPLETE:
        {
            PVPATB_TEST_IS_TRUE(PVPlayerFactory::DeletePlayer(iPlayer));
            iPlayer = NULL;

            delete iDataSource;
            iDataSource = NULL;

            delete iDataSinkVideo;
            iDataSinkVideo = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeVideo);
            iIONodeVideo = NULL;

            iMioFactory->DestroyVideoOutput(iMIOFileOutVideo);
            iMIOFileOutVideo = NULL;

            delete iDataSinkAudio;
            iDataSinkAudio = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeAudio);
            iIONodeAudio = NULL;

            iMioFactory->DestroyAudioOutput(iMIOFileOutAudio);
            iMIOFileOutAudio = NULL;

            iObserver->TestCompleted(*iTestCase);
        }
        break;

        default:
            break;

    }
}


void pvplayer_async_test_metadata::CommandCompleted(const PVCmdResponse& aResponse)
{
    if (aResponse.GetCmdId() != iCurrentCmdId)
    {
        // Wrong command ID.
        PVPATB_TEST_IS_TRUE(false);
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
        return;
    }

    if (aResponse.GetContext() != NULL)
    {
        if (aResponse.GetContext() == (OsclAny*)&iContextObject)
        {
            if (iContextObject != iContextObjectRefValue)
            {
                // Context data value was corrupted
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                return;
            }
        }
        else
        {
            // Context data pointer was corrupted
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
            return;
        }
    }

    switch (iState)
    {
        case STATE_ADDDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_INIT;
                RunIfNotReady();
            }
            else
            {
                // AddDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_INIT:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_GETMETADATAKEYS1;
                RunIfNotReady();
            }
            else
            {
                // Init failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GETMETADATAKEYS1:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_GETMETADATAVALUES1;
                RunIfNotReady();
            }
            else
            {
                // GetMetadataKeys failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;


        case STATE_GETMETADATAVALUES1:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                int32 retcode = CheckMetadataValue(iValueList);

                if (retcode)
                {
                    if (retcode&0x1)
                    {
                        // Error in returned value content
                        PVPATB_TEST_IS_TRUE(false);
                    }
                    if (retcode&0x2)
                    {
                        // Known value entries does not match the number of entries
                        // This is OK
                    }
                    if (retcode&0x4)
                    {
                        // Unknown valtype encountered
                        PVPATB_TEST_IS_TRUE(false);
                    }
                }
                else
                {
                    PVPATB_TEST_IS_TRUE(true);
                }

                iState = STATE_ADDDATASINK_VIDEO;
                RunIfNotReady();
                break;
            }
            else
            {
                // GetMetadataValues failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_PREPARE;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_PREPARE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_GETMETADATAKEYS2;
                RunIfNotReady();
            }
            else
            {
                // Prepare failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GETMETADATAKEYS2:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_GETMETADATAVALUES2;
                RunIfNotReady();
            }
            else
            {
                // GetMetadataKeys failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GETMETADATAVALUES2:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                int32 retcode = CheckMetadataValue(iValueList);

                if (retcode)
                {
                    if (retcode&0x1)
                    {
                        // Error in returned value content
                        PVPATB_TEST_IS_TRUE(false);
                    }
                    if (retcode&0x2)
                    {
                        // Known value entries does not match the number of entries
                        // This is OK
                    }
                    if (retcode&0x4)
                    {
                        // Unknown valtype encountered
                        PVPATB_TEST_IS_TRUE(false);
                    }
                }
                else
                {
                    PVPATB_TEST_IS_TRUE(true);
                }

                iState = STATE_GETMETADATAKEYSSEG;
                RunIfNotReady();
                break;
            }
            else
            {
                // GetMetadataValues failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GETMETADATAKEYSSEG:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_GETMETADATAVALUESSEG1;
                RunIfNotReady();
            }
            else
            {
                // GetMetadataKeys failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GETMETADATAVALUESSEG1:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_GETMETADATAVALUESSEG2;
                RunIfNotReady();
                break;
            }
            else
            {
                // GetMetadataValues failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GETMETADATAVALUESSEG2:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if (iValueListSeg1.size() != 6)
                {
                    PVPATB_TEST_IS_TRUE(false);
                }
                else
                {
                    PVPATB_TEST_IS_TRUE(true);
                }

                iState = STATE_GETMETADATAVALUESSEG3;
                RunIfNotReady();
                break;
            }
            else
            {
                // GetMetadataValues failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GETMETADATAVALUESSEG3:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if ((iValueListSeg1.size() + iValueListSeg2.size()) != iValueList.size())
                {
                    PVPATB_TEST_IS_TRUE(false);
                }
                else
                {
                    PVPATB_TEST_IS_TRUE(true);
                }

                iState = STATE_START;
                RunIfNotReady();
                break;
            }
            else
            {
                // GetMetadataValues failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_START:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_STOP;
                RunIfNotReady(3000000);
            }
            else
            {
                // Start failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_STOP:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_GETMETADATAKEYS3;
                RunIfNotReady();
            }
            else
            {
                // Stop failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GETMETADATAKEYS3:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_GETMETADATAVALUES3;
                RunIfNotReady();
            }
            else
            {
                // GetMetadataKeys failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GETMETADATAVALUES3:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                int32 retcode = CheckMetadataValue(iValueList);

                if (retcode)
                {
                    if (retcode&0x1)
                    {
                        // Error in returned value content
                        PVPATB_TEST_IS_TRUE(false);
                    }
                    if (retcode&0x2)
                    {
                        // Known value entries does not match the number of entries
                        // This is OK
                    }
                    if (retcode&0x4)
                    {
                        // Unknown valtype encountered
                        PVPATB_TEST_IS_TRUE(false);
                    }
                }
                else
                {
                    PVPATB_TEST_IS_TRUE(true);
                }

                iState = STATE_REMOVEDATASINK_VIDEO;
                RunIfNotReady();
                break;
            }
            else
            {
                // GetMetadataValues failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_RESET;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_RESET:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASOURCE;
                RunIfNotReady();
            }
            else
            {
                // Reset failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                PVPATB_TEST_IS_TRUE(true);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        default:
        {
            // Testing error if this is reached
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;
    }
}


void pvplayer_async_test_metadata::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    switch (aEvent.GetEventType())
    {
        case PVMFErrResourceConfiguration:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrResource:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrCorrupt:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrProcessing:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        default:
            // Unknown error and just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;
    }

    // Wait for engine to handle the error
    Cancel();
}


void pvplayer_async_test_metadata::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingStart)
    {
        // do nothing
    }
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingComplete)
    {
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
    }
}



//
// pvplayer_async_test_timing section
//
void pvplayer_async_test_timing::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}

#ifdef __WINS__
#define pvplayer_async_test_timing_TOLERANCE 200
#else
#define pvplayer_async_test_timing_TOLERANCE 150
#endif

void pvplayer_async_test_timing::Run()
{
    int error = 0;

    switch (iState)
    {
        case STATE_CREATE:
        {
            iPlayer = NULL;

            OSCL_TRY(error, iPlayer = PVPlayerFactory::CreatePlayer(this, this, this));
            if (error)
            {
                PVPATB_TEST_IS_TRUE(false);
                iObserver->TestCompleted(*iTestCase);
            }
            else
            {
                iState = STATE_ADDDATASOURCE;
                RunIfNotReady();
            }
        }
        break;

        case STATE_ADDDATASOURCE:
        {
            iDataSource = new PVPlayerDataSourceURL;
            OSCL_wHeapString<OsclMemAllocator>  wFileName;
            oscl_wchar output[100];
            oscl_UTF8ToUnicode(iFileName, oscl_strlen(iFileName), output, 100);
            wFileName.set(output, oscl_strlen(output));
            iDataSource->SetDataSourceURL(wFileName);
            iDataSource->SetDataSourceFormatType(iFileType);
            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_INIT:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Init((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_VIDEO:
        {
            OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
            sinkfile += _STRLIT_WCHAR("test_player_timing_video.dat");

            iMIOFileOutVideo = iMioFactory->CreateVideoOutput((OsclAny*) & sinkfile, MEDIATYPE_VIDEO, iCompressedVideo);
            iIONodeVideo = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutVideo);
            iDataSinkVideo = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkVideo)->SetDataSinkNode(iIONodeVideo);


            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_AUDIO:
        {
            OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
            sinkfile += _STRLIT_WCHAR("test_player_timing_audio.dat");

            iMIOFileOutAudio = iMioFactory->CreateAudioOutput((OsclAny*) & sinkfile, MEDIATYPE_AUDIO, iCompressedAudio);
            iIONodeAudio = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutAudio);
            iDataSinkAudio = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkAudio)->SetDataSinkNode(iIONodeAudio);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_PREPARE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Prepare((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_START:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Start((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_STOP:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Stop((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_VIDEO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_AUDIO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_RESET:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Reset((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASOURCE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_WAIT:
        {
            aPos.iPosUnit = PVPPBPOSUNIT_MILLISEC;
            iPlayer->GetCurrentPositionSync(aPos);

            uint32 curtime = OsclTickCount::TicksToMsec(OsclTickCount::TickCount()) - iStartTime;

            // Taking too long so stop playback
            if (curtime > 10000)
            {
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_STOP;
                RunIfNotReady();
            }

            //make sure reported playback position is within a small tolerance of elapsed clock time.
            int32 delta = aPos.iPosValue.millisec_value - curtime;
            //fprintf(file,"curtime %d, msec value %d, delta %d\n",curtime,aPos.iPosValue.millisec_value,delta);
            if (delta > pvplayer_async_test_timing_TOLERANCE || delta < (-pvplayer_async_test_timing_TOLERANCE))
            {
                PVPATB_TEST_IS_TRUE(false);
            }

            if (aPos.iPosValue.millisec_value <= 4000)
            {
                RunIfNotReady(100000);
            }
            else
            {
                iState = STATE_STOP;
                RunIfNotReady();
            }
        }
        break;

        case STATE_CLEANUPANDCOMPLETE:
        {
            PVPATB_TEST_IS_TRUE(PVPlayerFactory::DeletePlayer(iPlayer));
            iPlayer = NULL;

            delete iDataSource;
            iDataSource = NULL;

            delete iDataSinkVideo;
            iDataSinkVideo = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeVideo);
            iIONodeVideo = NULL;

            iMioFactory->DestroyVideoOutput(iMIOFileOutVideo);
            iMIOFileOutVideo = NULL;

            delete iDataSinkAudio;
            iDataSinkAudio = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeAudio);
            iIONodeAudio = NULL;

            iMioFactory->DestroyAudioOutput(iMIOFileOutAudio);
            iMIOFileOutAudio = NULL;

            iObserver->TestCompleted(*iTestCase);
        }
        break;

        default:
            break;

    }
}


void pvplayer_async_test_timing::CommandCompleted(const PVCmdResponse& aResponse)
{
    if (aResponse.GetCmdId() != iCurrentCmdId)
    {
        // Wrong command ID.
        PVPATB_TEST_IS_TRUE(false);
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
        return;
    }

    if (aResponse.GetContext() != NULL)
    {
        if (aResponse.GetContext() == (OsclAny*)&iContextObject)
        {
            if (iContextObject != iContextObjectRefValue)
            {
                // Context data value was corrupted
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                return;
            }
        }
        else
        {
            // Context data pointer was corrupted
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
            return;
        }
    }

    switch (iState)
    {
        case STATE_ADDDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_INIT;
                RunIfNotReady();
            }
            else
            {
                // AddDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_INIT:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                // Init failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_PREPARE;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_PREPARE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_START;
                RunIfNotReady();
            }
            else
            {
                // Prepare failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_START:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_WAIT;
                iStartTime = OsclTickCount::TicksToMsec(OsclTickCount::TickCount());
                RunIfNotReady();
            }
            else
            {
                // Start failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_STOP:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                // Stop failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_RESET;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_RESET:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASOURCE;
                RunIfNotReady();
            }
            else
            {
                // Reset failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                PVPATB_TEST_IS_TRUE(true);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        default:
        {
            // Testing error if this is reached
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;
    }
}


void pvplayer_async_test_timing::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    switch (aEvent.GetEventType())
    {
        case PVMFErrResourceConfiguration:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrResource:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrCorrupt:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrProcessing:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        default:
            // Unknown error and just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;
    }
}


void pvplayer_async_test_timing::HandleInformationalEvent(const PVAsyncInformationalEvent& /*aEvent*/)
{
}



//
// pvplayer_async_test_invalidstate section
//
void pvplayer_async_test_invalidstate::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_invalidstate::Run()
{
    int error = 0;

    switch (iState)
    {
        case STATE_CREATE:
        {
            iPlayer = NULL;

            OSCL_TRY(error, iPlayer = PVPlayerFactory::CreatePlayer(this, this, this));
            if (error)
            {
                PVPATB_TEST_IS_TRUE(false);
                iObserver->TestCompleted(*iTestCase);
            }
            else
            {
                iState = STATE_IDLE_INVALID_START;
                RunIfNotReady();
            }
        }
        break;

        case STATE_IDLE_INVALID_START:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Start((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_IDLE_INVALID_STOP:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Stop((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASOURCE:
        {
            iDataSource = new PVPlayerDataSourceURL;

            OSCL_wHeapString<OsclMemAllocator> sourcefile = SOURCENAME_PREPEND_WSTRING;
            sourcefile += _STRLIT_WCHAR("test.mp4");
            iDataSource->SetDataSourceURL(sourcefile);

            iDataSource->SetDataSourceFormatType(PVMF_MIME_MPEG4FF);
            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_INIT:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Init((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_INITIALIZED_INVALID_ADDDATASOURCE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_INITIALIZED_INVALID_START:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Start((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_VIDEO:
        {
            OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
            sinkfile += _STRLIT_WCHAR("test_player_invalid_video.dat");

            iMIOFileOutVideo = iMioFactory->CreateVideoOutput((OsclAny*) & sinkfile, MEDIATYPE_VIDEO, iCompressedVideo);
            iIONodeVideo = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutVideo);
            iDataSinkVideo = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkVideo)->SetDataSinkNode(iIONodeVideo);


            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_AUDIO:
        {
            OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
            sinkfile += _STRLIT_WCHAR("test_player_invalid_audio.dat");

            iMIOFileOutAudio = iMioFactory->CreateAudioOutput((OsclAny*) & sinkfile, MEDIATYPE_AUDIO, iCompressedAudio);
            iIONodeAudio = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutAudio);
            iDataSinkAudio = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkAudio)->SetDataSinkNode(iIONodeAudio);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_PREPARE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Prepare((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_PREPARED_INVALID_INIT:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Init((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_PREPARED_INVALID_PAUSE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Pause((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_START:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Start((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_STARTED_INVALID_RESUME:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Resume((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_STARTED_INVALID_PREPARE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Prepare((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_PAUSE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Pause((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_PAUSED_INVALID_INIT:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Init((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_PAUSED_INVALID_PREPARE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Prepare((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_STOP:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Stop((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_VIDEO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_AUDIO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_RESET:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Reset((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASOURCE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_CLEANUPANDCOMPLETE:
        {
            PVPATB_TEST_IS_TRUE(PVPlayerFactory::DeletePlayer(iPlayer));
            iPlayer = NULL;

            delete iDataSource;
            iDataSource = NULL;

            delete iDataSinkVideo;
            iDataSinkVideo = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeVideo);
            iIONodeVideo = NULL;

            iMioFactory->DestroyVideoOutput(iMIOFileOutVideo);
            iMIOFileOutVideo = NULL;

            delete iDataSinkAudio;
            iDataSinkAudio = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeAudio);
            iIONodeAudio = NULL;

            iMioFactory->DestroyAudioOutput(iMIOFileOutAudio);
            iMIOFileOutAudio = NULL;

            iObserver->TestCompleted(*iTestCase);
        }
        break;

        default:
            break;

    }
}


void pvplayer_async_test_invalidstate::CommandCompleted(const PVCmdResponse& aResponse)
{
    if (aResponse.GetCmdId() != iCurrentCmdId)
    {
        // Wrong command ID.
        PVPATB_TEST_IS_TRUE(false);
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
        return;
    }

    if (aResponse.GetContext() != NULL)
    {
        if (aResponse.GetContext() == (OsclAny*)&iContextObject)
        {
            if (iContextObject != iContextObjectRefValue)
            {
                // Context data value was corrupted
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                return;
            }
        }
        else
        {
            // Context data pointer was corrupted
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
            return;
        }
    }

    switch (iState)
    {
        case STATE_IDLE_INVALID_START:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                // Invalid call. Should not succeed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                iState = STATE_IDLE_INVALID_STOP;
                RunIfNotReady();
            }
            break;

        case STATE_IDLE_INVALID_STOP:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                // Invalid call. Should not succeed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                iState = STATE_ADDDATASOURCE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_INIT;
                RunIfNotReady();
            }
            else
            {
                // AddDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_INIT:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_INITIALIZED_INVALID_ADDDATASOURCE;
                RunIfNotReady();
            }
            else
            {
                // Init failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_INITIALIZED_INVALID_ADDDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                // Invalid call. Should not succeed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                iState = STATE_INITIALIZED_INVALID_START;
                RunIfNotReady();
            }
            break;

        case STATE_INITIALIZED_INVALID_START:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                // Invalid call. Should not succeed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                iState = STATE_ADDDATASINK_VIDEO;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_PREPARE;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_PREPARE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_PREPARED_INVALID_INIT;
                RunIfNotReady();
            }
            else
            {
                // Prepare failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_PREPARED_INVALID_INIT:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                // Invalid call. Should not succeed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                iState = STATE_PREPARED_INVALID_PAUSE;
                RunIfNotReady();
            }
            break;

        case STATE_PREPARED_INVALID_PAUSE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                // Invalid call. Should not succeed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                iState = STATE_START;
                RunIfNotReady();
            }
            break;

        case STATE_START:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_STARTED_INVALID_RESUME;
                RunIfNotReady(1000);
            }
            else
            {
                // Start failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_STARTED_INVALID_RESUME:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                // Invalid Call, resume should not be called in Started state but engine will return
                // command success to Resume if its already in a started state.
                iState = STATE_STARTED_INVALID_PREPARE;
                RunIfNotReady();
            }
            else
            {
                // Command failed. Resume should not fail if engine is already in started state.
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_STARTED_INVALID_PREPARE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                // Invalid call. Should not succeed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                iState = STATE_PAUSE;
                RunIfNotReady();
            }
            break;

        case STATE_PAUSE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_PAUSED_INVALID_INIT;
                RunIfNotReady(1000);
            }
            else
            {
                // Pause failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_PAUSED_INVALID_INIT:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                // Invalid call. Should not succeed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                iState = STATE_PAUSED_INVALID_PREPARE;
                RunIfNotReady();
            }
            break;

        case STATE_PAUSED_INVALID_PREPARE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                // Invalid call. Should not succeed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                iState = STATE_STOP;
                RunIfNotReady();
            }
            break;

        case STATE_STOP:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                // Stop failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_RESET;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_RESET:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASOURCE;
                RunIfNotReady();
            }
            else
            {
                // Reset failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                PVPATB_TEST_IS_TRUE(true);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        default:
        {
            // Testing error if this is reached
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;
    }
}


void pvplayer_async_test_invalidstate::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    switch (aEvent.GetEventType())
    {
        case PVMFErrResourceConfiguration:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrResource:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrCorrupt:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrProcessing:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        default:
            // Unknown error and just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;
    }
}


void pvplayer_async_test_invalidstate::HandleInformationalEvent(const PVAsyncInformationalEvent& /*aEvent*/)
{
}


//
// pvplayer_async_test_preparedstop section
//
void pvplayer_async_test_preparedstop::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_preparedstop::Run()
{
    int error = 0;

    switch (iState)
    {
        case STATE_CREATE:
        {
            iPlayer = NULL;

            OSCL_TRY(error, iPlayer = PVPlayerFactory::CreatePlayer(this, this, this));
            if (error)
            {
                PVPATB_TEST_IS_TRUE(false);
                iObserver->TestCompleted(*iTestCase);
            }
            else
            {
                iState = STATE_ADDDATASOURCE;
                RunIfNotReady();
            }
        }
        break;

        case STATE_ADDDATASOURCE:
        {
            iDataSource = new PVPlayerDataSourceURL;

            OSCL_wHeapString<OsclMemAllocator> sourcefile = SOURCENAME_PREPEND_WSTRING;
            sourcefile += _STRLIT_WCHAR("test.mp4");
            iDataSource->SetDataSourceURL(sourcefile);

            iDataSource->SetDataSourceFormatType(PVMF_MIME_MPEG4FF);
            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_INIT:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Init((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_VIDEO:
        {
            OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
            sinkfile += _STRLIT_WCHAR("test_player_preparedstop_video.dat");

            iMIOFileOutVideo = iMioFactory->CreateVideoOutput((OsclAny*) & sinkfile, MEDIATYPE_VIDEO, iCompressedVideo);
            iIONodeVideo = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutVideo);
            iDataSinkVideo = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkVideo)->SetDataSinkNode(iIONodeVideo);


            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_AUDIO:
        {
            OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
            sinkfile += _STRLIT_WCHAR("test_player_preparedstop_audio.dat");

            iMIOFileOutAudio = iMioFactory->CreateAudioOutput((OsclAny*) & sinkfile, MEDIATYPE_AUDIO, iCompressedAudio);
            iIONodeAudio = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutAudio);
            iDataSinkAudio = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkAudio)->SetDataSinkNode(iIONodeAudio);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_PREPARE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Prepare((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_STOP:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Stop((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_VIDEO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_AUDIO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_RESET:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Reset((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASOURCE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_CLEANUPANDCOMPLETE:
        {
            PVPATB_TEST_IS_TRUE(PVPlayerFactory::DeletePlayer(iPlayer));
            iPlayer = NULL;

            delete iDataSource;
            iDataSource = NULL;

            delete iDataSinkVideo;
            iDataSinkVideo = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeVideo);
            iIONodeVideo = NULL;

            iMioFactory->DestroyVideoOutput(iMIOFileOutVideo);
            iMIOFileOutVideo = NULL;

            delete iDataSinkAudio;
            iDataSinkAudio = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeAudio);
            iIONodeAudio = NULL;

            iMioFactory->DestroyAudioOutput(iMIOFileOutAudio);
            iMIOFileOutAudio = NULL;

            iObserver->TestCompleted(*iTestCase);
        }
        break;

        default:
            break;

    }
}


void pvplayer_async_test_preparedstop::CommandCompleted(const PVCmdResponse& aResponse)
{
    if (aResponse.GetCmdId() != iCurrentCmdId)
    {
        // Wrong command ID.
        PVPATB_TEST_IS_TRUE(false);
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
        return;
    }

    if (aResponse.GetContext() != NULL)
    {
        if (aResponse.GetContext() == (OsclAny*)&iContextObject)
        {
            if (iContextObject != iContextObjectRefValue)
            {
                // Context data value was corrupted
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                return;
            }
        }
        else
        {
            // Context data pointer was corrupted
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
            return;
        }
    }

    switch (iState)
    {
        case STATE_ADDDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_INIT;
                RunIfNotReady();
            }
            else
            {
                // AddDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_INIT:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                // Init failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_PREPARE;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_PREPARE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_STOP;
                RunIfNotReady();
            }
            else
            {
                // Prepare failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_STOP:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                // Stop failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_RESET;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_RESET:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASOURCE;
                RunIfNotReady();
            }
            else
            {
                // Reset failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                PVPATB_TEST_IS_TRUE(true);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        default:
        {
            // Testing error if this is reached
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;
    }
}


void pvplayer_async_test_preparedstop::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    switch (aEvent.GetEventType())
    {
        case PVMFErrResourceConfiguration:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrResource:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrCorrupt:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrProcessing:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        default:
            // Unknown error and just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;
    }
}


void pvplayer_async_test_preparedstop::HandleInformationalEvent(const PVAsyncInformationalEvent& /*aEvent*/)
{
}


//
// pvplayer_async_test_videoonlyplay7seconds section
//

void pvplayer_async_test_videoonlyplay7seconds::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_videoonlyplay7seconds::Run()
{
    int error = 0;

    switch (iState)
    {
        case STATE_CREATE:
        {
            iPlayer = NULL;

            OSCL_TRY(error, iPlayer = PVPlayerFactory::CreatePlayer(this, this, this));
            if (error)
            {
                PVPATB_TEST_IS_TRUE(false);
                iObserver->TestCompleted(*iTestCase);
            }
            else
            {
                iState = STATE_ADDDATASOURCE;
                RunIfNotReady();
            }
        }
        break;

        case STATE_ADDDATASOURCE:
        {
            iDataSource = new PVPlayerDataSourceURL;

            OSCL_wHeapString<OsclMemAllocator> sourcefile = SOURCENAME_PREPEND_WSTRING;
            sourcefile += _STRLIT_WCHAR("test.mp4");
            iDataSource->SetDataSourceURL(sourcefile);

            iDataSource->SetDataSourceFormatType(PVMF_MIME_MPEG4FF);
            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_INIT:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Init((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_VIDEO:
        {
            OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
            sinkfile += _STRLIT_WCHAR("test_player_videoonly7s_video.dat");

            iMIOFileOutVideo = iMioFactory->CreateVideoOutput((OsclAny*) & sinkfile, MEDIATYPE_VIDEO, iCompressedVideo);
            iIONodeVideo = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutVideo);
            iDataSinkVideo = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkVideo)->SetDataSinkNode(iIONodeVideo);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;


        case STATE_PREPARE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Prepare((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_START:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Start((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_STOP:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Stop((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_VIDEO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_RESET:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Reset((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASOURCE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_CLEANUPANDCOMPLETE:
        {
            PVPATB_TEST_IS_TRUE(PVPlayerFactory::DeletePlayer(iPlayer));
            iPlayer = NULL;

            delete iDataSource;
            iDataSource = NULL;

            delete iDataSinkVideo;
            iDataSinkVideo = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeVideo);
            iIONodeVideo = NULL;

            iMioFactory->DestroyVideoOutput(iMIOFileOutVideo);
            iMIOFileOutVideo = NULL;

            iObserver->TestCompleted(*iTestCase);
        }
        break;

        default:
            break;

    }
}


void pvplayer_async_test_videoonlyplay7seconds::CommandCompleted(const PVCmdResponse& aResponse)
{
    if (aResponse.GetCmdId() != iCurrentCmdId)
    {
        // Wrong command ID.
        PVPATB_TEST_IS_TRUE(false);
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
        return;
    }

    if (aResponse.GetContext() != NULL)
    {
        if (aResponse.GetContext() == (OsclAny*)&iContextObject)
        {
            if (iContextObject != iContextObjectRefValue)
            {
                // Context data value was corrupted
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                return;
            }
        }
        else
        {
            // Context data pointer was corrupted
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
            return;
        }
    }

    switch (iState)
    {
        case STATE_ADDDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_INIT;
                RunIfNotReady();
            }
            else
            {
                // AddDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_INIT:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                // Init failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_PREPARE;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;


        case STATE_PREPARE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_START;
                RunIfNotReady();
            }
            else
            {
                // Prepare failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_START:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_STOP;
                RunIfNotReady(7000000);
            }
            else
            {
                // Start failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_STOP:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                // Stop failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_RESET;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_RESET:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASOURCE;
                RunIfNotReady();
            }
            else
            {
                // Reset failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                PVPATB_TEST_IS_TRUE(true);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        default:
        {
            // Testing error if this is reached
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;
    }
}


void pvplayer_async_test_videoonlyplay7seconds::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    switch (aEvent.GetEventType())
    {
        case PVMFErrResourceConfiguration:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrResource:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrCorrupt:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrProcessing:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        default:
            // Unknown error and just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;
    }
}


void pvplayer_async_test_videoonlyplay7seconds::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingStart)
    {
        // do nothing
    }
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingComplete)
    {
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
    }
}


//
// pvplayer_async_test_play5stopplay10stopreset section
//
void pvplayer_async_test_play5stopplay10stopreset::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_play5stopplay10stopreset::Run()
{
    int error = 0;

    switch (iState)
    {
        case STATE_CREATE:
        {
            iPlayer = NULL;

            OSCL_TRY(error, iPlayer = PVPlayerFactory::CreatePlayer(this, this, this));
            if (error)
            {
                PVPATB_TEST_IS_TRUE(false);
                iObserver->TestCompleted(*iTestCase);
            }
            else
            {
                iState = STATE_ADDDATASOURCE;
                RunIfNotReady();
            }
        }
        break;

        case STATE_ADDDATASOURCE:
        {
            iDataSource = new PVPlayerDataSourceURL;
            OSCL_wHeapString<OsclMemAllocator>  wFileName;
            oscl_wchar output[100];
            oscl_UTF8ToUnicode(iFileName, oscl_strlen(iFileName), output, 100);
            wFileName.set(output, oscl_strlen(output));
            iDataSource->SetDataSourceURL(wFileName);
            iDataSource->SetDataSourceFormatType(iFileType);
            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_INIT:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Init((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_VIDEO:
        {
            OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
            sinkfile += _STRLIT_WCHAR("test_player_playstop2times_video.dat");

            iMIOFileOutVideo = iMioFactory->CreateVideoOutput((OsclAny*) & sinkfile, MEDIATYPE_VIDEO, iCompressedVideo);
            iIONodeVideo = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutVideo);
            iDataSinkVideo = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkVideo)->SetDataSinkNode(iIONodeVideo);


            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_AUDIO:
        {
            OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
            sinkfile += _STRLIT_WCHAR("test_player_playstop2times_audio.dat");

            iMIOFileOutAudio = iMioFactory->CreateAudioOutput((OsclAny*) & sinkfile, MEDIATYPE_AUDIO, iCompressedAudio);
            iIONodeAudio = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutAudio);
            iDataSinkAudio = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkAudio)->SetDataSinkNode(iIONodeAudio);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_PREPARE_FIRST:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Prepare((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_START_FIRST:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Start((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_STOP_FIRST:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Stop((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;
        case STATE_PREPARE_SECOND:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Prepare((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;
        case STATE_START_SECOND:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Start((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_STOP_SECOND:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Stop((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_VIDEO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_AUDIO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_RESET:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Reset((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASOURCE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_CLEANUPANDCOMPLETE:
        {
            PVPATB_TEST_IS_TRUE(PVPlayerFactory::DeletePlayer(iPlayer));
            iPlayer = NULL;

            delete iDataSource;
            iDataSource = NULL;

            delete iDataSinkVideo;
            iDataSinkVideo = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeVideo);
            iIONodeVideo = NULL;

            iMioFactory->DestroyVideoOutput(iMIOFileOutVideo);
            iMIOFileOutVideo = NULL;

            delete iDataSinkAudio;
            iDataSinkAudio = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeAudio);
            iIONodeAudio = NULL;

            iMioFactory->DestroyAudioOutput(iMIOFileOutAudio);
            iMIOFileOutAudio = NULL;

            iObserver->TestCompleted(*iTestCase);
        }
        break;

        default:
            break;

    }
}


void pvplayer_async_test_play5stopplay10stopreset::CommandCompleted(const PVCmdResponse& aResponse)
{
    if (aResponse.GetCmdId() != iCurrentCmdId)
    {
        // Wrong command ID.
        PVPATB_TEST_IS_TRUE(false);
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
        return;
    }

    if (aResponse.GetContext() != NULL)
    {
        if (aResponse.GetContext() == (OsclAny*)&iContextObject)
        {
            if (iContextObject != iContextObjectRefValue)
            {
                // Context data value was corrupted
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                return;
            }
        }
        else
        {
            // Context data pointer was corrupted
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
            return;
        }
    }

    switch (iState)
    {
        case STATE_ADDDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_INIT;
                RunIfNotReady();
            }
            else
            {
                // AddDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_INIT:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                // Init failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_PREPARE_FIRST;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_PREPARE_FIRST:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_START_FIRST;
                RunIfNotReady();
            }
            else
            {
                // Prepare failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_START_FIRST:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_STOP_FIRST;
                RunIfNotReady(5000000);
            }
            else
            {
                // Start failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_STOP_FIRST:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_PREPARE_SECOND;
                RunIfNotReady();
            }
            else
            {
                // Stop failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_PREPARE_SECOND:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_START_SECOND;
                RunIfNotReady();
            }
            else
            {
                // Prepare failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_START_SECOND:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_STOP_SECOND;
                RunIfNotReady(10000000);
            }
            else
            {
                // Start failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_STOP_SECOND:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                // Stop failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;


        case STATE_REMOVEDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_RESET;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_RESET:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASOURCE;
                RunIfNotReady();
            }
            else
            {
                // Reset failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                PVPATB_TEST_IS_TRUE(true);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        default:
        {
            // Testing error if this is reached
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;
    }
}


void pvplayer_async_test_play5stopplay10stopreset::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    switch (aEvent.GetEventType())
    {
        case PVMFErrResourceConfiguration:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrResource:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrCorrupt:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrProcessing:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        default:
            // Unknown error and just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;
    }
}


void pvplayer_async_test_play5stopplay10stopreset::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingStart)
    {
        // do nothing
    }
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingComplete)
    {
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
    }
}


//
// pvplayer_async_test_pauseresume section
//
void pvplayer_async_test_pauseresume::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_pauseresume::Run()
{
    int error = 0;

    switch (iState)
    {
        case STATE_CREATE:
        {
            iPlayer = NULL;

            OSCL_TRY(error, iPlayer = PVPlayerFactory::CreatePlayer(this, this, this));
            if (error)
            {
                PVPATB_TEST_IS_TRUE(false);
                iObserver->TestCompleted(*iTestCase);
            }
            else
            {
                iState = STATE_ADDDATASOURCE;
                RunIfNotReady();
            }
        }
        break;

        case STATE_ADDDATASOURCE:
        {
            iDataSource = new PVPlayerDataSourceURL;
            OSCL_wHeapString<OsclMemAllocator>  wFileName;
            oscl_wchar output[100];
            oscl_UTF8ToUnicode(iFileName, oscl_strlen(iFileName), output, 100);
            wFileName.set(output, oscl_strlen(output));
            iDataSource->SetDataSourceURL(wFileName);
            iDataSource->SetDataSourceFormatType(iFileType);
            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_INIT:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Init());
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_VIDEO:
        {
            OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
            sinkfile += _STRLIT_WCHAR("test_player_pauseresume_video.dat");

            iMIOFileOutVideo = iMioFactory->CreateVideoOutput((OsclAny*) & sinkfile, MEDIATYPE_VIDEO, iCompressedVideo);
            iIONodeVideo = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutVideo);
            iDataSinkVideo = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkVideo)->SetDataSinkNode(iIONodeVideo);


            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_AUDIO:
        {
            OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
            sinkfile += _STRLIT_WCHAR("test_player_pauseresume_audio.dat");

            iMIOFileOutAudio = iMioFactory->CreateAudioOutput((OsclAny*) & sinkfile, MEDIATYPE_AUDIO, iCompressedAudio);
            iIONodeAudio = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutAudio);
            iDataSinkAudio = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkAudio)->SetDataSinkNode(iIONodeAudio);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_PREPARE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Prepare((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_START:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Start((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;
        case STATE_PAUSE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Pause((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_RESUME:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Resume((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_STOP:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Stop((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_VIDEO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_AUDIO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_RESET:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Reset((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASOURCE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_CLEANUPANDCOMPLETE:
        {
            PVPATB_TEST_IS_TRUE(PVPlayerFactory::DeletePlayer(iPlayer));
            iPlayer = NULL;

            delete iDataSource;
            iDataSource = NULL;

            delete iDataSinkVideo;
            iDataSinkVideo = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeVideo);
            iIONodeVideo = NULL;

            iMioFactory->DestroyVideoOutput(iMIOFileOutVideo);
            iMIOFileOutVideo = NULL;

            delete iDataSinkAudio;
            iDataSinkAudio = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeAudio);
            iIONodeAudio = NULL;

            iMioFactory->DestroyAudioOutput(iMIOFileOutAudio);
            iMIOFileOutAudio = NULL;

            iObserver->TestCompleted(*iTestCase);
        }
        break;

        default:
            break;

    }
}


void pvplayer_async_test_pauseresume::CommandCompleted(const PVCmdResponse& aResponse)
{
    if (aResponse.GetCmdId() != iCurrentCmdId)
    {
        // Wrong command ID.
        PVPATB_TEST_IS_TRUE(false);
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
        return;
    }

    if (aResponse.GetContext() != NULL)
    {
        if (aResponse.GetContext() == (OsclAny*)&iContextObject)
        {
            if (iContextObject != iContextObjectRefValue)
            {
                // Context data value was corrupted
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                return;
            }
        }
        else
        {
            // Context data pointer was corrupted
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
            return;
        }
    }

    switch (iState)
    {
        case STATE_ADDDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_INIT;
                RunIfNotReady();
            }
            else
            {
                // AddDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_INIT:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                // Init failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_PREPARE;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_PREPARE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_START;
                RunIfNotReady();
            }
            else
            {
                // Prepare failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_START:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_PAUSE;
                RunIfNotReady(10000000);
            }
            else
            {
                // Start failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_PAUSE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_RESUME;
                RunIfNotReady(5000000);
            }
            else
            {
                // Pause failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_RESUME:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_STOP;
                RunIfNotReady(10000000);
            }
            else
            {
                // Resume failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_STOP:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                // Stop failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_RESET;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_RESET:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASOURCE;
                RunIfNotReady();
            }
            else
            {
                // Reset failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                PVPATB_TEST_IS_TRUE(true);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        default:
        {
            // Testing error if this is reached
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;
    }
}


void pvplayer_async_test_pauseresume::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    switch (aEvent.GetEventType())
    {
        case PVMFErrResourceConfiguration:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrResource:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrCorrupt:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrProcessing:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        default:
            // Unknown error and just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;
    }
}


void pvplayer_async_test_pauseresume::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingStart)
    {
        // do nothing
    }
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingComplete)
    {
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
    }
}


//
// pvplayer_async_test_playpausestop section
//


void pvplayer_async_test_playpausestop::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_playpausestop::Run()
{
    int error = 0;

    switch (iState)
    {
        case STATE_CREATE:
        {
            iPlayer = NULL;

            OSCL_TRY(error, iPlayer = PVPlayerFactory::CreatePlayer(this, this, this));
            if (error)
            {
                PVPATB_TEST_IS_TRUE(false);
                iObserver->TestCompleted(*iTestCase);
            }
            else
            {
                iState = STATE_ADDDATASOURCE;
                RunIfNotReady();
            }
        }
        break;

        case STATE_ADDDATASOURCE:
        {
            iDataSource = new PVPlayerDataSourceURL;
            OSCL_wHeapString<OsclMemAllocator> wFileName;
            oscl_wchar output[100];
            oscl_UTF8ToUnicode(iFileName, oscl_strlen(iFileName), output, 100);
            wFileName.set(output, oscl_strlen(output));
            iDataSource->SetDataSourceURL(wFileName);
            iDataSource->SetDataSourceFormatType(iFileType);
            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_INIT:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Init((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_VIDEO:
        {
            OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
            sinkfile += _STRLIT_WCHAR("test_player_playpausestop_video.dat");

            iMIOFileOutVideo = iMioFactory->CreateVideoOutput((OsclAny*) & sinkfile, MEDIATYPE_VIDEO, iCompressedVideo);
            iIONodeVideo = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutVideo);
            iDataSinkVideo = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkVideo)->SetDataSinkNode(iIONodeVideo);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_AUDIO:
        {
            OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
            sinkfile += _STRLIT_WCHAR("test_player_playpausestop_audio.dat");

            iMIOFileOutAudio = iMioFactory->CreateAudioOutput((OsclAny*) & sinkfile, MEDIATYPE_AUDIO, iCompressedAudio);
            iIONodeAudio = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutAudio);
            iDataSinkAudio = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkAudio)->SetDataSinkNode(iIONodeAudio);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_PREPARE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Prepare((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_START:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Start((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_PAUSE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Pause((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_STOP:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Stop((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_VIDEO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_AUDIO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_RESET:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Reset((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASOURCE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_CLEANUPANDCOMPLETE:
        {
            PVPATB_TEST_IS_TRUE(PVPlayerFactory::DeletePlayer(iPlayer));
            iPlayer = NULL;

            delete iDataSource;
            iDataSource = NULL;

            delete iDataSinkVideo;
            iDataSinkVideo = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeVideo);
            iIONodeVideo = NULL;

            iMioFactory->DestroyVideoOutput(iMIOFileOutVideo);
            iMIOFileOutVideo = NULL;

            delete iDataSinkAudio;
            iDataSinkAudio = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeAudio);
            iIONodeAudio = NULL;

            iMioFactory->DestroyAudioOutput(iMIOFileOutAudio);
            iMIOFileOutAudio = NULL;

            iObserver->TestCompleted(*iTestCase);
        }
        break;

        default:
            break;

    }
}


void pvplayer_async_test_playpausestop::CommandCompleted(const PVCmdResponse& aResponse)
{
    if (aResponse.GetCmdId() != iCurrentCmdId)
    {
        // Wrong command ID.
        PVPATB_TEST_IS_TRUE(false);
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
        return;
    }

    if (aResponse.GetContext() != NULL)
    {
        if (aResponse.GetContext() == (OsclAny*)&iContextObject)
        {
            if (iContextObject != iContextObjectRefValue)
            {
                // Context data value was corrupted
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                return;
            }
        }
        else
        {
            // Context data pointer was corrupted
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
            return;
        }
    }

    switch (iState)
    {
        case STATE_ADDDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_INIT;
                RunIfNotReady();
            }
            else
            {
                // AddDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_INIT:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                // Init failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_PREPARE;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_PREPARE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_START;
                RunIfNotReady();
            }
            else
            {
                // Prepare failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_START:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_PAUSE;
                RunIfNotReady(30000000);
            }
            else
            {
                // Start failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;
        case STATE_PAUSE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_STOP;
                RunIfNotReady();
            }
            else
            {
                // Pause failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_STOP:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                // Stop failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_RESET;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_RESET:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASOURCE;
                RunIfNotReady();
            }
            else
            {
                // Reset failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                PVPATB_TEST_IS_TRUE(true);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        default:
        {
            // Testing error if this is reached
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;
    }
}


void pvplayer_async_test_playpausestop::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    switch (aEvent.GetEventType())
    {
        case PVMFErrResourceConfiguration:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrResource:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrCorrupt:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrProcessing:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        default:
            // Unknown error and just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;
    }
}


void pvplayer_async_test_playpausestop::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingStart)
    {
        // do nothing
    }
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingComplete)
    {
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
    }
}





//
// pvplayer_async_test_outsidenodeforvideosink section
//
void pvplayer_async_test_outsidenodeforvideosink::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_outsidenodeforvideosink::Run()
{
    int error = 0;

    switch (iState)
    {
        case STATE_CREATE:
        {
            iPlayer = NULL;

            OSCL_TRY(error, iPlayer = PVPlayerFactory::CreatePlayer(this, this, this));
            if (error)
            {
                PVPATB_TEST_IS_TRUE(false);
                iObserver->TestCompleted(*iTestCase);
            }
            else
            {
                iState = STATE_ADDDATASOURCE;
                RunIfNotReady();
            }
        }
        break;

        case STATE_ADDDATASOURCE:
        {
            iDataSource = new PVPlayerDataSourceURL;

            OSCL_wHeapString<OsclMemAllocator> sourcefile = SOURCENAME_PREPEND_WSTRING;
            sourcefile += _STRLIT_WCHAR("test.mp4");
            iDataSource->SetDataSourceURL(sourcefile);

            iDataSource->SetDataSourceFormatType(PVMF_MIME_MPEG4FF);
            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_INIT:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Init((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;


        case STATE_ADDDATASINK_VIDEO:
        {
            OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
            sinkfile += _STRLIT_WCHAR("test_player_outsidevideosink_video.dat");

            iMOutVideo = iMioFactory->CreateVideoOutput((OsclAny*) & sinkfile, MEDIATYPE_VIDEO, iCompressedVideo);
            iIONodeVideo = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMOutVideo);
            iDataSinkVideo = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkVideo)->SetDataSinkNode(iIONodeVideo);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_AUDIO:
        {
            OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
            sinkfile += _STRLIT_WCHAR("test_player_outsidevideosink_audio.dat");

            iMOutAudio = iMioFactory->CreateAudioOutput((OsclAny*) & sinkfile, MEDIATYPE_AUDIO, iCompressedAudio);
            iIONodeAudio = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMOutAudio);
            iDataSinkAudio = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkAudio)->SetDataSinkNode(iIONodeAudio);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_PREPARE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Prepare((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_START:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Start((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_STOP:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Stop((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_VIDEO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_AUDIO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_RESET:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Reset((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASOURCE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_CLEANUPANDCOMPLETE:
        {
            PVPATB_TEST_IS_TRUE(PVPlayerFactory::DeletePlayer(iPlayer));
            iPlayer = NULL;

            delete iDataSource;
            iDataSource = NULL;

            delete iDataSinkVideo;
            iDataSinkVideo = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeVideo);
            iIONodeVideo = NULL;

            iMioFactory->DestroyVideoOutput(iMOutVideo);
            iMOutVideo = NULL;

            delete iDataSinkAudio;
            iDataSinkAudio = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeAudio);
            iIONodeAudio = NULL;

            iMioFactory->DestroyAudioOutput(iMOutAudio);
            iMOutAudio = NULL;

            iObserver->TestCompleted(*iTestCase);
        }
        break;

        default:
            break;

    }
}


void pvplayer_async_test_outsidenodeforvideosink::CommandCompleted(const PVCmdResponse& aResponse)
{
    if (aResponse.GetCmdId() != iCurrentCmdId)
    {
        // Wrong command ID.
        PVPATB_TEST_IS_TRUE(false);
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
        return;
    }

    if (aResponse.GetContext() != NULL)
    {
        if (aResponse.GetContext() == (OsclAny*)&iContextObject)
        {
            if (iContextObject != iContextObjectRefValue)
            {
                // Context data value was corrupted
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                return;
            }
        }
        else
        {
            // Context data pointer was corrupted
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
            return;
        }
    }

    switch (iState)
    {
        case STATE_ADDDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_INIT;
                RunIfNotReady();
            }
            else
            {
                // AddDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_INIT:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                // Init failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_PREPARE;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_PREPARE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_START;
                RunIfNotReady();
            }
            else
            {
                // Prepare failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_START:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_STOP;
                RunIfNotReady(30000000);
            }
            else
            {
                // Start failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_STOP:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                // Stop failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_RESET;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_RESET:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASOURCE;
                RunIfNotReady();
            }
            else
            {
                // Reset failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                PVPATB_TEST_IS_TRUE(true);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        default:
        {
            // Testing error if this is reached
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;
    }
}


void pvplayer_async_test_outsidenodeforvideosink::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    switch (aEvent.GetEventType())
    {
        case PVMFErrResourceConfiguration:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrResource:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrCorrupt:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrProcessing:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        default:
            // Unknown error and just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;
    }
}


void pvplayer_async_test_outsidenodeforvideosink::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingStart)
    {
        // do nothing
    }
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingComplete)
    {
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
    }
}


void pvplayer_async_test_outsidenodeforvideosink::NodeCommandCompleted(const PVMFCmdResp &aResponse)
{
    OSCL_UNUSED_ARG(aResponse);
}


//
// pvplayer_async_test_getplayerstate section
//
void pvplayer_async_test_getplayerstate::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_getplayerstate::Run()
{
    int error = 0;

    switch (iState)
    {
        case STATE_CREATE:
        {
            iPlayer = NULL;

            OSCL_TRY(error, iPlayer = PVPlayerFactory::CreatePlayer(this, this, this));
            if (error)
            {
                PVPATB_TEST_IS_TRUE(false);
                iObserver->TestCompleted(*iTestCase);
            }
            else
            {
                iState = STATE_ADDDATASOURCE;
                RunIfNotReady();
            }
        }
        break;

        case STATE_ADDDATASOURCE:
        {
            iDataSource = new PVPlayerDataSourceURL;

            OSCL_wHeapString<OsclMemAllocator> sourcefile = SOURCENAME_PREPEND_WSTRING;
            sourcefile += _STRLIT_WCHAR("test.mp4");
            iDataSource->SetDataSourceURL(sourcefile);

            iDataSource->SetDataSourceFormatType(PVMF_MIME_MPEG4FF);
            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_INIT:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Init((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_VIDEO:
        {
            OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
            sinkfile += _STRLIT_WCHAR("test_player_getplayerstate_video.dat");

            iMIOFileOutVideo = iMioFactory->CreateVideoOutput((OsclAny*) & sinkfile, MEDIATYPE_VIDEO, iCompressedVideo);
            iIONodeVideo = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutVideo);
            iDataSinkVideo = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkVideo)->SetDataSinkNode(iIONodeVideo);


            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_AUDIO:
        {
            OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
            sinkfile += _STRLIT_WCHAR("test_player_getplayerstate_audio.dat");

            iMIOFileOutAudio = iMioFactory->CreateAudioOutput((OsclAny*) & sinkfile, MEDIATYPE_AUDIO, iCompressedAudio);
            iIONodeAudio = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutAudio);
            iDataSinkAudio = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkAudio)->SetDataSinkNode(iIONodeAudio);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_PREPARE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Prepare((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_START:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Start((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_STOP:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Stop((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_PLAYER_STATE_FIRST:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->GetPVPlayerState(aState, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_VIDEO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_PLAYER_STATE_SECOND:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->GetPVPlayerState(aState, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_AUDIO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_PLAYER_STATE_THIRD:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->GetPVPlayerState(aState, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_RESET:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Reset((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_PLAYER_STATE_FOURTH:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->GetPVPlayerState(aState, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASOURCE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_CLEANUPANDCOMPLETE:
        {
            PVPATB_TEST_IS_TRUE(PVPlayerFactory::DeletePlayer(iPlayer));
            iPlayer = NULL;

            delete iDataSource;
            iDataSource = NULL;

            delete iDataSinkVideo;
            iDataSinkVideo = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeVideo);
            iIONodeVideo = NULL;

            iMioFactory->DestroyVideoOutput(iMIOFileOutVideo);
            iMIOFileOutVideo = NULL;

            delete iDataSinkAudio;
            iDataSinkAudio = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeAudio);
            iIONodeAudio = NULL;

            iMioFactory->DestroyAudioOutput(iMIOFileOutAudio);
            iMIOFileOutAudio = NULL;

            iObserver->TestCompleted(*iTestCase);
        }
        break;

        default:
            break;

    }
}


void pvplayer_async_test_getplayerstate::CommandCompleted(const PVCmdResponse& aResponse)
{
    if (aResponse.GetCmdId() != iCurrentCmdId)
    {
        // Wrong command ID.
        PVPATB_TEST_IS_TRUE(false);
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
        return;
    }

    if (aResponse.GetContext() != NULL)
    {
        if (aResponse.GetContext() == (OsclAny*)&iContextObject)
        {
            if (iContextObject != iContextObjectRefValue)
            {
                // Context data value was corrupted
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                return;
            }
        }
        else
        {
            // Context data pointer was corrupted
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
            return;
        }
    }

    switch (iState)
    {
        case STATE_ADDDATASOURCE:
            if (iPlayer->GetPVPlayerStateSync(aState) == PVMFSuccess)
            {
                if (aState == PVP_STATE_IDLE)
                {
                    iState = STATE_INIT;
                    RunIfNotReady();
                }
                else
                {
                    // AddDataSource failed
                    PVPATB_TEST_IS_TRUE(false);
                    iState = STATE_CLEANUPANDCOMPLETE;
                    RunIfNotReady();
                }
            }
            else
            {
                // AddDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_INIT:
            if (iPlayer->GetPVPlayerStateSync(aState) == PVMFSuccess)
            {
                if (aState == PVP_STATE_INITIALIZED)
                {
                    iState = STATE_ADDDATASINK_VIDEO;
                    RunIfNotReady();
                }
                else
                {
                    // AddDataSource failed
                    PVPATB_TEST_IS_TRUE(false);
                    iState = STATE_CLEANUPANDCOMPLETE;
                    RunIfNotReady();
                }
            }
            else
            {
                // AddDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_VIDEO:
            if (iPlayer->GetPVPlayerStateSync(aState) == PVMFSuccess)
            {
                if (aState == PVP_STATE_INITIALIZED)
                {
                    iState = STATE_ADDDATASINK_AUDIO;
                    RunIfNotReady();
                }
                else
                {

                    PVPATB_TEST_IS_TRUE(false);
                    iState = STATE_CLEANUPANDCOMPLETE;
                    RunIfNotReady();
                }
            }
            else
            {

                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_AUDIO:
            if (iPlayer->GetPVPlayerStateSync(aState) == PVMFSuccess)
            {
                if (aState == PVP_STATE_INITIALIZED)
                {
                    iState = STATE_PREPARE;
                    RunIfNotReady();
                }
                else
                {

                    PVPATB_TEST_IS_TRUE(false);
                    iState = STATE_CLEANUPANDCOMPLETE;
                    RunIfNotReady();
                }
            }
            else
            {

                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;
        case STATE_PREPARE:
            if (iPlayer->GetPVPlayerStateSync(aState) == PVMFSuccess)
            {
                if (aState == PVP_STATE_PREPARED)
                {
                    iState = STATE_START;
                    RunIfNotReady();
                }
                else
                {

                    PVPATB_TEST_IS_TRUE(false);
                    iState = STATE_CLEANUPANDCOMPLETE;
                    RunIfNotReady();
                }
            }
            else
            {

                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;
        case STATE_START:
            if (iPlayer->GetPVPlayerStateSync(aState) == PVMFSuccess)
            {
                if (aState == PVP_STATE_STARTED)
                {
                    iState = STATE_STOP;
                    RunIfNotReady(20000000);
                }
                else
                {

                    PVPATB_TEST_IS_TRUE(false);
                    iState = STATE_CLEANUPANDCOMPLETE;
                    RunIfNotReady();
                }
            }
            else
            {

                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_STOP:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_PLAYER_STATE_FIRST;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_PLAYER_STATE_FIRST:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if (aState == PVP_STATE_INITIALIZED)
                {
                    iState = STATE_REMOVEDATASINK_VIDEO;
                    RunIfNotReady();
                }
                else
                {

                    PVPATB_TEST_IS_TRUE(false);
                    iState = STATE_CLEANUPANDCOMPLETE;
                    RunIfNotReady();
                }
            }
            else
            {

                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_PLAYER_STATE_SECOND;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_PLAYER_STATE_SECOND:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if (aState == PVP_STATE_INITIALIZED)
                {
                    iState = STATE_REMOVEDATASINK_AUDIO;
                    RunIfNotReady();
                }
                else
                {

                    PVPATB_TEST_IS_TRUE(false);
                    iState = STATE_CLEANUPANDCOMPLETE;
                    RunIfNotReady();
                }
            }
            else
            {

                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_PLAYER_STATE_THIRD;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_PLAYER_STATE_THIRD:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if (aState == PVP_STATE_INITIALIZED)
                {
                    iState = STATE_RESET;
                    RunIfNotReady();
                }
                else
                {

                    PVPATB_TEST_IS_TRUE(false);
                    iState = STATE_CLEANUPANDCOMPLETE;
                    RunIfNotReady();
                }
            }
            else
            {

                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_RESET:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_PLAYER_STATE_FOURTH;
                RunIfNotReady();
            }
            else
            {
                // Reset failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_PLAYER_STATE_FOURTH:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if (aState == PVP_STATE_IDLE)
                {
                    iState = STATE_REMOVEDATASOURCE;
                    RunIfNotReady();
                }
                else
                {

                    PVPATB_TEST_IS_TRUE(false);
                    iState = STATE_CLEANUPANDCOMPLETE;
                    RunIfNotReady();
                }
            }
            else
            {

                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                PVPATB_TEST_IS_TRUE(true);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        default:
        {
            // Testing error if this is reached
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;
    }
}


void pvplayer_async_test_getplayerstate::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    switch (aEvent.GetEventType())
    {
        case PVMFErrResourceConfiguration:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrResource:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrCorrupt:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrProcessing:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        default:
            // Unknown error and just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;
    }
}


void pvplayer_async_test_getplayerstate::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingStart)
    {
        // do nothing
    }
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingComplete)
    {
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
    }
}


//
// pvplayer_async_test_getcurrentposition section
//
void pvplayer_async_test_getcurrentposition::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_getcurrentposition::Run()
{
    int error = 0;

    switch (iState)
    {
        case STATE_CREATE:
        {
            iPlayer = NULL;

            OSCL_TRY(error, iPlayer = PVPlayerFactory::CreatePlayer(this, this, this));
            if (error)
            {
                PVPATB_TEST_IS_TRUE(false);
                iObserver->TestCompleted(*iTestCase);
            }
            else
            {
                iState = STATE_ADDDATASOURCE;
                RunIfNotReady();
            }
        }
        break;

        case STATE_ADDDATASOURCE:
        {
            iDataSource = new PVPlayerDataSourceURL;
            OSCL_wHeapString<OsclMemAllocator>  wFileName;
            oscl_wchar output[100] ;
            oscl_UTF8ToUnicode(iFileName, oscl_strlen(iFileName), output, 100);
            wFileName.set(output, oscl_strlen(output));
            iDataSource->SetDataSourceURL(wFileName);
            iDataSource->SetDataSourceFormatType(iFileType);
            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_INIT:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Init((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_VIDEO:
        {
            OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
            sinkfile += _STRLIT_WCHAR("test_player_getcurrentposition_video.dat");

            iMIOFileOutVideo = iMioFactory->CreateVideoOutput((OsclAny*) & sinkfile, MEDIATYPE_VIDEO, iCompressedVideo);
            iIONodeVideo = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutVideo);
            iDataSinkVideo = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkVideo)->SetDataSinkNode(iIONodeVideo);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_AUDIO:
        {
            OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
            sinkfile += _STRLIT_WCHAR("test_player_getcurrentposition_audio.dat");

            iMIOFileOutAudio = iMioFactory->CreateAudioOutput((OsclAny*) & sinkfile, MEDIATYPE_AUDIO, iCompressedAudio);
            iIONodeAudio = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutAudio);
            iDataSinkAudio = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkAudio)->SetDataSinkNode(iIONodeAudio);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_PREPARE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Prepare((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_START:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Start((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_GETCURRENT_POSITION_SYNC:
        {
            Position.iPosUnit = PVPPBPOSUNIT_MILLISEC;
            Status = iPlayer->GetCurrentPositionSync(Position);
            if (Status == PVMFSuccess)
            {
                if (iCount < 10)
                {
                    iState = STATE_GETCURRENT_POSITION_SYNC;
                    iCount++;
                    RunIfNotReady(1000000);
                }
                else
                {
                    iCount = 0;
                    iState = STATE_GETCURRENT_POSITION;
                    RunIfNotReady();
                }
            }
            else
            {
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }

        }
        break;

        case STATE_GETCURRENT_POSITION:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->GetCurrentPosition(Position, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_STOP:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Stop((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_VIDEO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_AUDIO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_RESET:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Reset((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASOURCE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_CLEANUPANDCOMPLETE:
        {
            PVPATB_TEST_IS_TRUE(PVPlayerFactory::DeletePlayer(iPlayer));
            iPlayer = NULL;

            delete iDataSource;
            iDataSource = NULL;

            delete iDataSinkVideo;
            iDataSinkVideo = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeVideo);
            iIONodeVideo = NULL;

            iMioFactory->DestroyVideoOutput(iMIOFileOutVideo);
            iMIOFileOutVideo = NULL;

            delete iDataSinkAudio;
            iDataSinkAudio = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeAudio);
            iIONodeAudio = NULL;

            iMioFactory->DestroyAudioOutput(iMIOFileOutAudio);
            iMIOFileOutAudio = NULL;

            iObserver->TestCompleted(*iTestCase);
        }
        break;

        default:
            break;

    }
}


void pvplayer_async_test_getcurrentposition::CommandCompleted(const PVCmdResponse& aResponse)
{
    if (aResponse.GetCmdId() != iCurrentCmdId)
    {
        // Wrong command ID.
        PVPATB_TEST_IS_TRUE(false);
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
        return;
    }

    if (aResponse.GetContext() != NULL)
    {
        if (aResponse.GetContext() == (OsclAny*)&iContextObject)
        {
            if (iContextObject != iContextObjectRefValue)
            {
                // Context data value was corrupted
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                return;
            }
        }
        else
        {
            // Context data pointer was corrupted
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
            return;
        }
    }

    switch (iState)
    {
        case STATE_ADDDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_INIT;
                RunIfNotReady();
            }
            else
            {
                // AddDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_INIT:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                // Init failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_PREPARE;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_PREPARE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_START;
                RunIfNotReady();
            }
            else
            {
                // Prepare failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_START:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_GETCURRENT_POSITION_SYNC;
                RunIfNotReady();
            }
            else
            {
                // Start failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GETCURRENT_POSITION:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if (iCount < 10)
                {
                    iState = STATE_GETCURRENT_POSITION;
                    iCount++;
                    RunIfNotReady(1000000);
                }
                else
                {
                    iState = STATE_STOP;
                    RunIfNotReady();
                }

            }
            else
            {
                // Start failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_STOP:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                // Stop failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_RESET;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_RESET:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASOURCE;
                RunIfNotReady();
            }
            else
            {
                // Reset failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                PVPATB_TEST_IS_TRUE(true);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        default:
        {
            // Testing error if this is reached
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;
    }
}


void pvplayer_async_test_getcurrentposition::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    switch (aEvent.GetEventType())
    {
        case PVMFErrResourceConfiguration:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrResource:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrCorrupt:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrProcessing:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        default:
            // Unknown error and just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;
    }
}


void pvplayer_async_test_getcurrentposition::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingStart)
    {
        // do nothing
    }
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingComplete)
    {
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
    }
}


//
// pvplayer_async_test_playsetstopposition section
//
void pvplayer_async_test_playsetstopposition::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_playsetstopposition::Run()
{
    int error = 0;

    switch (iState)
    {
        case STATE_CREATE:
        {
            iPlayer = NULL;

            OSCL_TRY(error, iPlayer = PVPlayerFactory::CreatePlayer(this, this, this));
            if (error)
            {
                PVPATB_TEST_IS_TRUE(false);
                iObserver->TestCompleted(*iTestCase);
            }
            else
            {
                iState = STATE_ADDDATASOURCE;
                RunIfNotReady();
            }
        }
        break;

        case STATE_ADDDATASOURCE:
        {
            iDataSource = new PVPlayerDataSourceURL;

            OSCL_wHeapString<OsclMemAllocator> sourcefile = SOURCENAME_PREPEND_WSTRING;
            sourcefile += _STRLIT_WCHAR("test_reposition.mp4");
            iDataSource->SetDataSourceURL(sourcefile);

            iDataSource->SetDataSourceFormatType(PVMF_MIME_MPEG4FF);
            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_INIT:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Init((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_VIDEO:
        {
            OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
            sinkfile += _STRLIT_WCHAR("test_player_playsetstopposition_video.dat");

            iMIOFileOutVideo = iMioFactory->CreateVideoOutput((OsclAny*) & sinkfile, MEDIATYPE_VIDEO, iCompressedVideo);
            iIONodeVideo = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutVideo);
            iDataSinkVideo = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkVideo)->SetDataSinkNode(iIONodeVideo);


            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_AUDIO:
        {
            OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
            sinkfile += _STRLIT_WCHAR("test_player_playsetstopposition_audio.dat");

            iMIOFileOutAudio = iMioFactory->CreateAudioOutput((OsclAny*) & sinkfile, MEDIATYPE_AUDIO, iCompressedAudio);
            iIONodeAudio = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutAudio);
            iDataSinkAudio = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkAudio)->SetDataSinkNode(iIONodeAudio);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_PREPARE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Prepare((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_START:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Start((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_SETPLAYBACKRANGE:
        {
            PVPPlaybackPosition start, end;
            start.iIndeterminate = true;
            start.iMode = PVPPBPOS_MODE_NOW;
            end.iIndeterminate = false;
            end.iPosUnit = PVPPBPOSUNIT_MILLISEC;
            end.iPosValue.millisec_value = 20000;
            OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRange(start, end, false, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_STOPTIMENOTREACHED:
        {
            // Timeout when player doesn't stop automatically when stop time is reached
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;

        case STATE_STOP:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Stop((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_VIDEO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_AUDIO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_RESET:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Reset((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASOURCE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_CLEANUPANDCOMPLETE:
        {
            PVPATB_TEST_IS_TRUE(PVPlayerFactory::DeletePlayer(iPlayer));
            iPlayer = NULL;

            delete iDataSource;
            iDataSource = NULL;

            delete iDataSinkVideo;
            iDataSinkVideo = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeVideo);
            iIONodeVideo = NULL;

            iMioFactory->DestroyVideoOutput(iMIOFileOutVideo);
            iMIOFileOutVideo = NULL;

            delete iDataSinkAudio;
            iDataSinkAudio = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeAudio);
            iIONodeAudio = NULL;

            iMioFactory->DestroyAudioOutput(iMIOFileOutAudio);
            iMIOFileOutAudio = NULL;

            iObserver->TestCompleted(*iTestCase);
        }
        break;

        default:
            break;

    }
}


void pvplayer_async_test_playsetstopposition::CommandCompleted(const PVCmdResponse& aResponse)
{
    if (aResponse.GetCmdId() != iCurrentCmdId)
    {
        // Wrong command ID.
        PVPATB_TEST_IS_TRUE(false);
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
        return;
    }

    if (aResponse.GetContext() != NULL)
    {
        if (aResponse.GetContext() == (OsclAny*)&iContextObject)
        {
            if (iContextObject != iContextObjectRefValue)
            {
                // Context data value was corrupted
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                return;
            }
        }
        else
        {
            // Context data pointer was corrupted
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
            return;
        }
    }

    switch (iState)
    {
        case STATE_ADDDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_INIT;
                RunIfNotReady();
            }
            else
            {
                // AddDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_INIT:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                // Init failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_PREPARE;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_PREPARE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_START;
                RunIfNotReady();
            }
            else
            {
                // Prepare failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_START:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_SETPLAYBACKRANGE;
                RunIfNotReady();
            }
            else
            {
                // Start failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_SETPLAYBACKRANGE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_STOPTIMENOTREACHED;
                RunIfNotReady(30000000);
            }
            else
            {
                // SetPlaybackRange failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_STOP:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                // Stop failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_RESET;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_RESET:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASOURCE;
                RunIfNotReady();
            }
            else
            {
                // Reset failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                PVPATB_TEST_IS_TRUE(true);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        default:
        {
            // Testing error if this is reached
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;
    }
}


void pvplayer_async_test_playsetstopposition::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    switch (aEvent.GetEventType())
    {
        case PVMFErrResourceConfiguration:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrResource:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrCorrupt:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrProcessing:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        default:
            // Unknown error and just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;
    }
}


void pvplayer_async_test_playsetstopposition::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingStart)
    {
        // do nothing
    }
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingComplete)
    {
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
    }
    // Check for stop time reached event
    if (aEvent.GetEventType() == PVMFInfoEndOfData)
    {
        PVInterface* iface = (PVInterface*)(aEvent.GetEventExtensionInterface());
        if (iface == NULL)
        {
            return;
        }
        PVUuid infomsguuid = PVMFErrorInfoMessageInterfaceUUID;
        PVMFErrorInfoMessageInterface* infomsgiface = NULL;
        if (iface->queryInterface(infomsguuid, (PVInterface*&)infomsgiface) == true)
        {
            int32 infocode;
            PVUuid infouuid;
            infomsgiface->GetCodeUUID(infocode, infouuid);
            if ((infouuid == PVPlayerErrorInfoEventTypesUUID) && (infocode == PVPlayerInfoEndTimeReached))
            {
                iState = STATE_STOP;
                Cancel();
                RunIfNotReady();
            }
        }
    }
}


//
// pvplayer_async_test_playsetstoppositionvidframenum section
//
void pvplayer_async_test_playsetstoppositionvidframenum::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_playsetstoppositionvidframenum::Run()
{
    int error = 0;

    switch (iState)
    {
        case STATE_CREATE:
        {
            iPlayer = NULL;

            OSCL_TRY(error, iPlayer = PVPlayerFactory::CreatePlayer(this, this, this));
            if (error)
            {
                PVPATB_TEST_IS_TRUE(false);
                iObserver->TestCompleted(*iTestCase);
            }
            else
            {
                iState = STATE_ADDDATASOURCE;
                RunIfNotReady();
            }
        }
        break;

        case STATE_ADDDATASOURCE:
        {
            iDataSource = new PVPlayerDataSourceURL;

            OSCL_wHeapString<OsclMemAllocator> sourcefile = SOURCENAME_PREPEND_WSTRING;
            sourcefile += _STRLIT_WCHAR("test_reposition.mp4");
            iDataSource->SetDataSourceURL(sourcefile);

            iDataSource->SetDataSourceFormatType(PVMF_MIME_MPEG4FF);
            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_INIT:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Init((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_VIDEO:
        {
            OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
            sinkfile += _STRLIT_WCHAR("test_player_playsetstoppositionvidframenum_video.dat");

            iMIOFileOutVideo = iMioFactory->CreateVideoOutput((OsclAny*) & sinkfile, MEDIATYPE_VIDEO, iCompressedVideo);
            iIONodeVideo = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutVideo);
            iDataSinkVideo = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkVideo)->SetDataSinkNode(iIONodeVideo);


            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_AUDIO:
        {
            OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
            sinkfile += _STRLIT_WCHAR("test_player_playsetstoppositionvidframenum_audio.dat");

            iMIOFileOutAudio = iMioFactory->CreateAudioOutput((OsclAny*) & sinkfile, MEDIATYPE_AUDIO, iCompressedAudio);
            iIONodeAudio = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutAudio);
            iDataSinkAudio = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkAudio)->SetDataSinkNode(iIONodeAudio);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_PREPARE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Prepare((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_START:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Start((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_SETPLAYBACKRANGE:
        {
            PVPPlaybackPosition start, end;
            start.iIndeterminate = true;
            start.iMode = PVPPBPOS_MODE_NOW;
            end.iIndeterminate = false;
            end.iPosUnit = PVPPBPOSUNIT_SAMPLENUMBER;
            end.iPosValue.samplenum_value = 150;
            OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRange(start, end, false, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_STOPTIMENOTREACHED:
        {
            // Timeout when player doesn't stop automatically when stop time is reached
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;

        case STATE_STOP:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Stop((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_VIDEO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_AUDIO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_RESET:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Reset((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASOURCE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_CLEANUPANDCOMPLETE:
        {
            PVPATB_TEST_IS_TRUE(PVPlayerFactory::DeletePlayer(iPlayer));
            iPlayer = NULL;

            delete iDataSource;
            iDataSource = NULL;

            delete iDataSinkVideo;
            iDataSinkVideo = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeVideo);
            iIONodeVideo = NULL;

            iMioFactory->DestroyVideoOutput(iMIOFileOutVideo);
            iMIOFileOutVideo = NULL;

            delete iDataSinkAudio;
            iDataSinkAudio = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeAudio);
            iIONodeAudio = NULL;

            iMioFactory->DestroyAudioOutput(iMIOFileOutAudio);
            iMIOFileOutAudio = NULL;

            iObserver->TestCompleted(*iTestCase);
        }
        break;

        default:
            break;

    }
}


void pvplayer_async_test_playsetstoppositionvidframenum::CommandCompleted(const PVCmdResponse& aResponse)
{
    if (aResponse.GetCmdId() != iCurrentCmdId)
    {
        // Wrong command ID.
        PVPATB_TEST_IS_TRUE(false);
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
        return;
    }

    if (aResponse.GetContext() != NULL)
    {
        if (aResponse.GetContext() == (OsclAny*)&iContextObject)
        {
            if (iContextObject != iContextObjectRefValue)
            {
                // Context data value was corrupted
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                return;
            }
        }
        else
        {
            // Context data pointer was corrupted
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
            return;
        }
    }

    switch (iState)
    {
        case STATE_ADDDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_INIT;
                RunIfNotReady();
            }
            else
            {
                // AddDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_INIT:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                // Init failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_PREPARE;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_PREPARE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_START;
                RunIfNotReady();
            }
            else
            {
                // Prepare failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_START:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_SETPLAYBACKRANGE;
                RunIfNotReady();
            }
            else
            {
                // Start failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_SETPLAYBACKRANGE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_STOPTIMENOTREACHED;
                RunIfNotReady(30000000);
            }
            else
            {
                // SetPlaybackRange failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_STOP:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                // Stop failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_RESET;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_RESET:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASOURCE;
                RunIfNotReady();
            }
            else
            {
                // Reset failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                PVPATB_TEST_IS_TRUE(true);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        default:
        {
            // Testing error if this is reached
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;
    }
}


void pvplayer_async_test_playsetstoppositionvidframenum::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    switch (aEvent.GetEventType())
    {
        case PVMFErrResourceConfiguration:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrResource:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrCorrupt:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrProcessing:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        default:
            // Unknown error and just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;
    }
}


void pvplayer_async_test_playsetstoppositionvidframenum::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingStart)
    {
        // do nothing
    }
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingComplete)
    {
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
    }
    // Check for stop time reached event
    if (aEvent.GetEventType() == PVMFInfoEndOfData)
    {
        PVInterface* iface = (PVInterface*)(aEvent.GetEventExtensionInterface());
        if (iface == NULL)
        {
            return;
        }
        PVUuid infomsguuid = PVMFErrorInfoMessageInterfaceUUID;
        PVMFErrorInfoMessageInterface* infomsgiface = NULL;
        if (iface->queryInterface(infomsguuid, (PVInterface*&)infomsgiface) == true)
        {
            int32 infocode;
            PVUuid infouuid;
            infomsgiface->GetCodeUUID(infocode, infouuid);
            if ((infouuid == PVPlayerErrorInfoEventTypesUUID) && (infocode == PVPlayerInfoEndTimeReached))
            {
                iState = STATE_STOP;
                Cancel();
                RunIfNotReady();
            }
        }
    }
}


//
// pvplayer_async_test_setstartpositionplaystop section
//
void pvplayer_async_test_setstartpositionplaystop::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_setstartpositionplaystop::Run()
{
    int error = 0;

    switch (iState)
    {
        case STATE_CREATE:
        {
            iPlayer = NULL;

            OSCL_TRY(error, iPlayer = PVPlayerFactory::CreatePlayer(this, this, this));
            if (error)
            {
                PVPATB_TEST_IS_TRUE(false);
                iObserver->TestCompleted(*iTestCase);
            }
            else
            {
                iState = STATE_ADDDATASOURCE;
                RunIfNotReady();
            }
        }
        break;

        case STATE_ADDDATASOURCE:
        {
            PVMFFormatType fileType;
            iDataSource = new PVPlayerDataSourceURL;
            OSCL_HeapString<OsclMemAllocator> filename;

            if (oscl_strstr(iFileName, "test.mp4") != NULL)
            {
                filename = SOURCENAME_PREPEND_STRING;
                filename += _STRLIT_CHAR("test_reposition.mp4");
                fileType = PVMF_MIME_MPEG4FF;
                iFileType = PVMF_MIME_MPEG4FF;
            }
            else
            {
                filename = iFileName;
                fileType = iFileType;
            }
            fprintf(iTestMsgOutputFile, "***Source URL=%s\n", filename.get_str());
            oscl_UTF8ToUnicode(filename.get_str(), filename.get_size(), output, 256);
            wFileName.set(output, oscl_strlen(output));

            if (iFileType == PVMF_MIME_DATA_SOURCE_HTTP_URL)
            {
                fileType = PVMF_MIME_DATA_SOURCE_HTTP_URL;
                iDataSource->SetAlternateSourceFormatType(PVMF_MIME_DATA_SOURCE_MS_HTTP_STREAMING_URL);
                iSourceContextData = new PVMFSourceContextData();
                iSourceContextData->EnableCommonSourceContext();
                iSourceContextData->EnableStreamingSourceContext();

                PVInterface* sourceContext = NULL;
                PVUuid streamingContextUuid(PVMF_SOURCE_CONTEXT_DATA_STREAMING_UUID);
                if (iSourceContextData->queryInterface(streamingContextUuid, sourceContext))
                {
                    PVMFSourceContextDataStreaming* streamingContext =
                        OSCL_STATIC_CAST(PVMFSourceContextDataStreaming*, sourceContext);
                    streamingContext->iStreamStatsLoggingURL = wFileName;

                    if (iProxyEnabled)
                    {
                        streamingContext->iProxyName = _STRLIT_WCHAR("63.215.241.33");
                        streamingContext->iProxyPort = 7070;
                    }

                    streamingContext->iUserID = _STRLIT_CHAR("abc");
                    streamingContext->iUserPasswd = _STRLIT_CHAR("xyz");
                }
                iSourceContextData->EnableDownloadHTTPSourceContext();
                sourceContext = NULL;
                PVUuid downloadContextUuid(PVMF_SOURCE_CONTEXT_DATA_DOWNLOAD_HTTP_UUID);
                if (iSourceContextData->queryInterface(downloadContextUuid, sourceContext))
                {
                    PVMFSourceContextDataDownloadHTTP* downloadContext =
                        OSCL_STATIC_CAST(PVMFSourceContextDataDownloadHTTP*, sourceContext);
                    if (iProxyEnabled)
                    {
                        downloadContext->iProxyName = _STRLIT_CHAR("63.215.241.33");
                        downloadContext->iProxyPort = 7070;
                    }
                    downloadContext->iDownloadFileName += _STRLIT_WCHAR("test_ftdownload.loc");
                    downloadContext->iConfigFileName += _STRLIT_WCHAR("mydlconfig");
                    downloadContext->iUserID = _STRLIT_CHAR("abc");
                    downloadContext->iUserPasswd = _STRLIT_CHAR("xyz");
                    downloadContext->bIsNewSession = true;
                    downloadContext->iMaxFileSize = 0x7FFFFFFF;
                    downloadContext->iPlaybackControl = PVMFSourceContextDataDownloadHTTP::EAsap;
                }
                iDataSource->SetDataSourceContextData((OsclAny*)iSourceContextData);
            }
            iDataSource->SetDataSourceURL(wFileName);
            iDataSource->SetDataSourceFormatType(fileType);
            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_INIT:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Init((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_VIDEO:
        {
            OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
            sinkfile += _STRLIT_WCHAR("test_player_setstartpositionplaystop_video.dat");

            iMIOFileOutVideo = iMioFactory->CreateVideoOutput((OsclAny*) & sinkfile, MEDIATYPE_VIDEO, iCompressedVideo);
            iIONodeVideo = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutVideo);
            iDataSinkVideo = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkVideo)->SetDataSinkNode(iIONodeVideo);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_AUDIO:
        {
            OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
            sinkfile += _STRLIT_WCHAR("test_player_setstartpositionplaystop_audio.dat");

            iMIOFileOutAudio = iMioFactory->CreateAudioOutput((OsclAny*) & sinkfile, MEDIATYPE_AUDIO, iCompressedAudio);
            iIONodeAudio = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutAudio);
            iDataSinkAudio = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkAudio)->SetDataSinkNode(iIONodeAudio);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_SETPLAYBACKRANGE:
        {
            PVPPlaybackPosition start, end;
            start.iIndeterminate = false;
            start.iPosUnit = PVPPBPOSUNIT_MILLISEC;
            start.iMode = PVPPBPOS_MODE_NOW;
            start.iPosValue.millisec_value = 10000;
            end.iIndeterminate = true;
            OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRange(start, end, false, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_PREPARE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Prepare((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_START:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Start((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_STOP:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Stop((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_VIDEO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_AUDIO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_RESET:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Reset((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASOURCE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_CLEANUPANDCOMPLETE:
        {
            PVPATB_TEST_IS_TRUE(PVPlayerFactory::DeletePlayer(iPlayer));
            iPlayer = NULL;

            delete iSourceContextData;
            iSourceContextData = NULL;

            delete iDataSource;
            iDataSource = NULL;

            delete iDataSinkVideo;
            iDataSinkVideo = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeVideo);
            iIONodeVideo = NULL;

            iMioFactory->DestroyVideoOutput(iMIOFileOutVideo);
            iMIOFileOutVideo = NULL;

            delete iDataSinkAudio;
            iDataSinkAudio = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeAudio);
            iIONodeAudio = NULL;

            iMioFactory->DestroyAudioOutput(iMIOFileOutAudio);
            iMIOFileOutAudio = NULL;

            iObserver->TestCompleted(*iTestCase);
        }
        break;

        default:
            break;

    }
}


void pvplayer_async_test_setstartpositionplaystop::CommandCompleted(const PVCmdResponse& aResponse)
{
    if (aResponse.GetCmdId() != iCurrentCmdId)
    {
        // Wrong command ID.
        PVPATB_TEST_IS_TRUE(false);
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
        return;
    }

    if (aResponse.GetContext() != NULL)
    {
        if (aResponse.GetContext() == (OsclAny*)&iContextObject)
        {
            if (iContextObject != iContextObjectRefValue)
            {
                // Context data value was corrupted
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                return;
            }
        }
        else
        {
            // Context data pointer was corrupted
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
            return;
        }
    }

    switch (iState)
    {
        case STATE_ADDDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_INIT;
                RunIfNotReady();
            }
            else
            {
                // AddDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_INIT:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                // Init failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_SETPLAYBACKRANGE;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_SETPLAYBACKRANGE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iNumSeek++;
                iState = STATE_PREPARE;
                RunIfNotReady();
            }
            else
            {
                // SetPlaybackRange failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_PREPARE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_START;
                RunIfNotReady();
            }
            else
            {
                // Prepare failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_START:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_STOP;
                RunIfNotReady(20000000);
            }
            else
            {
                // Start failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_STOP:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if (iTargetNumSeek != iNumSeek)
                {
                    iState = STATE_SETPLAYBACKRANGE;
                    RunIfNotReady();
                }
                else
                {
                    iState = STATE_REMOVEDATASINK_VIDEO;
                    RunIfNotReady();
                }
            }
            else
            {
                // Stop failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_RESET;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_RESET:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASOURCE;
                RunIfNotReady();
            }
            else
            {
                // Reset failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                PVPATB_TEST_IS_TRUE(true);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        default:
        {
            // Testing error if this is reached
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;
    }
}


void pvplayer_async_test_setstartpositionplaystop::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    switch (aEvent.GetEventType())
    {
        case PVMFErrResourceConfiguration:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrResource:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrCorrupt:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrProcessing:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        default:
            // Unknown error and just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;
    }
}


void pvplayer_async_test_setstartpositionplaystop::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingStart)
    {
        // do nothing
    }
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingComplete)
    {
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
    }
    // Check for end of clip event
    if (aEvent.GetEventType() == PVMFInfoEndOfData)
    {
        PVInterface* iface = (PVInterface*)(aEvent.GetEventExtensionInterface());
        if (iface == NULL)
        {
            return;
        }
        PVUuid infomsguuid = PVMFErrorInfoMessageInterfaceUUID;
        PVMFErrorInfoMessageInterface* infomsgiface = NULL;
        if (iface->queryInterface(infomsguuid, (PVInterface*&)infomsgiface) == true)
        {
            int32 infocode;
            PVUuid infouuid;
            infomsgiface->GetCodeUUID(infocode, infouuid);
            if ((infouuid == PVPlayerErrorInfoEventTypesUUID) && (infocode == PVPlayerInfoEndOfClipReached))
            {
                iState = STATE_STOP;
                Cancel();
                RunIfNotReady();
            }
        }
    }
    else if (aEvent.GetEventType() == PVMFInfoSourceFormatNotSupported)
    {
        // source rollover...
        fprintf(iTestMsgOutputFile, "###PVMFInfoSourceFormatNotSupported...\n");
        PVInterface* iface = (PVInterface*)(aEvent.GetEventExtensionInterface());
        if (iface != NULL)
        {
            PVUuid infomsguuid = PVMFErrorInfoMessageInterfaceUUID;
            PVMFErrorInfoMessageInterface* infomsgiface = NULL;
            if (iface->queryInterface(infomsguuid, (PVInterface*&)infomsgiface) == true)
            {
                int32 infocode;
                PVUuid infouuid;
                infomsgiface->GetCodeUUID(infocode, infouuid);
                if ((infouuid == PVPlayerErrorInfoEventTypesUUID) &&
                        (infocode == PVPlayerInfoAttemptingSourceRollOver))
                {
                    uint8* localBuf = aEvent.GetLocalBuffer();
                    if (localBuf != NULL)
                    {
                        uint32 srcFormat = 0;
                        oscl_memcpy(&srcFormat, &localBuf[4], sizeof(uint32));
                        fprintf(iTestMsgOutputFile, "###Attempting Source RollOver - NewSrcFormat=%d\n", srcFormat);
                    }
                }
            }
        }
    }
}


//
// pvplayer_async_test_setplayrangeplay section
//
void pvplayer_async_test_setplayrangeplay::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_setplayrangeplay::Run()
{
    int error = 0;

    switch (iState)
    {
        case STATE_CREATE:
        {
            iPlayer = NULL;

            OSCL_TRY(error, iPlayer = PVPlayerFactory::CreatePlayer(this, this, this));
            if (error)
            {
                PVPATB_TEST_IS_TRUE(false);
                iObserver->TestCompleted(*iTestCase);
            }
            else
            {
                iState = STATE_ADDDATASOURCE;
                RunIfNotReady();
            }
        }
        break;

        case STATE_ADDDATASOURCE:
        {
            iDataSource = new PVPlayerDataSourceURL;

            OSCL_wHeapString<OsclMemAllocator> sourcefile = SOURCENAME_PREPEND_WSTRING;
            sourcefile += _STRLIT_WCHAR("test_reposition.mp4");
            iDataSource->SetDataSourceURL(sourcefile);

            iDataSource->SetDataSourceFormatType(PVMF_MIME_MPEG4FF);
            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_INIT:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Init((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_VIDEO:
        {
            OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
            sinkfile += _STRLIT_WCHAR("test_player_setplayrangeplay_video.dat");

            iMIOFileOutVideo = iMioFactory->CreateVideoOutput((OsclAny*) & sinkfile, MEDIATYPE_VIDEO, iCompressedVideo);
            iIONodeVideo = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutVideo);
            iDataSinkVideo = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkVideo)->SetDataSinkNode(iIONodeVideo);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_AUDIO:
        {
            OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
            sinkfile += _STRLIT_WCHAR("test_player_setplayrangeplay_audio.dat");

            iMIOFileOutAudio = iMioFactory->CreateAudioOutput((OsclAny*) & sinkfile, MEDIATYPE_AUDIO, iCompressedAudio);
            iIONodeAudio = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutAudio);
            iDataSinkAudio = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkAudio)->SetDataSinkNode(iIONodeAudio);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_SETPLAYBACKRANGE:
        {
            PVPPlaybackPosition start, end;
            start.iIndeterminate = false;
            start.iPosUnit = PVPPBPOSUNIT_MILLISEC;
            start.iMode = PVPPBPOS_MODE_NOW;
            start.iPosValue.millisec_value = 10000;
            end.iIndeterminate = false;
            end.iPosUnit = PVPPBPOSUNIT_MILLISEC;
            end.iPosValue.millisec_value = 25000;
            OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRange(start, end, false, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_PREPARE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Prepare((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_START:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Start((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_STOPTIMENOTREACHED:
        {
            // Timeout when player doesn't stop automatically when stop time is reached
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;

        case STATE_STOP:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Stop((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_VIDEO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_AUDIO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_RESET:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Reset((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASOURCE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_CLEANUPANDCOMPLETE:
        {
            PVPATB_TEST_IS_TRUE(PVPlayerFactory::DeletePlayer(iPlayer));
            iPlayer = NULL;

            delete iDataSource;
            iDataSource = NULL;

            delete iDataSinkVideo;
            iDataSinkVideo = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeVideo);
            iIONodeVideo = NULL;

            iMioFactory->DestroyVideoOutput(iMIOFileOutVideo);
            iMIOFileOutVideo = NULL;

            delete iDataSinkAudio;
            iDataSinkAudio = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeAudio);
            iIONodeAudio = NULL;

            iMioFactory->DestroyAudioOutput(iMIOFileOutAudio);
            iMIOFileOutAudio = NULL;

            iObserver->TestCompleted(*iTestCase);
        }
        break;

        default:
            break;

    }
}


void pvplayer_async_test_setplayrangeplay::CommandCompleted(const PVCmdResponse& aResponse)
{
    if (aResponse.GetCmdId() != iCurrentCmdId)
    {
        // Wrong command ID.
        PVPATB_TEST_IS_TRUE(false);
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
        return;
    }

    if (aResponse.GetContext() != NULL)
    {
        if (aResponse.GetContext() == (OsclAny*)&iContextObject)
        {
            if (iContextObject != iContextObjectRefValue)
            {
                // Context data value was corrupted
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                return;
            }
        }
        else
        {
            // Context data pointer was corrupted
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
            return;
        }
    }

    switch (iState)
    {
        case STATE_ADDDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_INIT;
                RunIfNotReady();
            }
            else
            {
                // AddDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_INIT:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                // Init failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_SETPLAYBACKRANGE;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_SETPLAYBACKRANGE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_PREPARE;
                RunIfNotReady();
            }
            else
            {
                // SetPlaybackRange failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_PREPARE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_START;
                RunIfNotReady();
            }
            else
            {
                // Prepare failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_START:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_STOPTIMENOTREACHED;
                RunIfNotReady(25000000);
            }
            else
            {
                // Start failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_STOP:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                // Stop failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_RESET;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_RESET:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASOURCE;
                RunIfNotReady();
            }
            else
            {
                // Reset failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                PVPATB_TEST_IS_TRUE(true);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        default:
        {
            // Testing error if this is reached
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;
    }
}


void pvplayer_async_test_setplayrangeplay::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    switch (aEvent.GetEventType())
    {
        case PVMFErrResourceConfiguration:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrResource:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrCorrupt:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrProcessing:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        default:
            // Unknown error and just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;
    }
}


void pvplayer_async_test_setplayrangeplay::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingStart)
    {
        // do nothing
    }
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingComplete)
    {
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
    }
    // Check for stop time reached event
    if (aEvent.GetEventType() == PVMFInfoEndOfData)
    {
        PVInterface* iface = (PVInterface*)(aEvent.GetEventExtensionInterface());
        if (iface == NULL)
        {
            return;
        }
        PVUuid infomsguuid = PVMFErrorInfoMessageInterfaceUUID;
        PVMFErrorInfoMessageInterface* infomsgiface = NULL;
        if (iface->queryInterface(infomsguuid, (PVInterface*&)infomsgiface) == true)
        {
            int32 infocode;
            PVUuid infouuid;
            infomsgiface->GetCodeUUID(infocode, infouuid);
            if ((infouuid == PVPlayerErrorInfoEventTypesUUID) && (infocode == PVPlayerInfoEndTimeReached))
            {
                iState = STATE_STOP;
                Cancel();
                RunIfNotReady();
            }
        }
    }
}


//
// pvplayer_async_test_setplayrangevidframenumplay section
//
void pvplayer_async_test_setplayrangevidframenumplay::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_setplayrangevidframenumplay::Run()
{
    int error = 0;

    switch (iState)
    {
        case STATE_CREATE:
        {
            iPlayer = NULL;

            OSCL_TRY(error, iPlayer = PVPlayerFactory::CreatePlayer(this, this, this));
            if (error)
            {
                PVPATB_TEST_IS_TRUE(false);
                iObserver->TestCompleted(*iTestCase);
            }
            else
            {
                iState = STATE_ADDDATASOURCE;
                RunIfNotReady();
            }
        }
        break;

        case STATE_ADDDATASOURCE:
        {
            iDataSource = new PVPlayerDataSourceURL;

            OSCL_wHeapString<OsclMemAllocator> sourcefile = SOURCENAME_PREPEND_WSTRING;
            sourcefile += _STRLIT_WCHAR("test_reposition.mp4");
            iDataSource->SetDataSourceURL(sourcefile);

            iDataSource->SetDataSourceFormatType(PVMF_MIME_MPEG4FF);
            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_INIT:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Init((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_VIDEO:
        {
            OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
            sinkfile += _STRLIT_WCHAR("test_player_setplayrangevidframenumplay_video.dat");

            iMIOFileOutVideo = iMioFactory->CreateVideoOutput((OsclAny*) & sinkfile, MEDIATYPE_VIDEO, iCompressedVideo);
            iIONodeVideo = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutVideo);
            iDataSinkVideo = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkVideo)->SetDataSinkNode(iIONodeVideo);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_AUDIO:
        {
            OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
            sinkfile += _STRLIT_WCHAR("test_player_setplayrangevidframenumplay_audio.dat");

            iMIOFileOutAudio = iMioFactory->CreateAudioOutput((OsclAny*) & sinkfile, MEDIATYPE_AUDIO, iCompressedAudio);
            iIONodeAudio = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutAudio);
            iDataSinkAudio = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkAudio)->SetDataSinkNode(iIONodeAudio);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_SETPLAYBACKRANGE:
        {
            PVPPlaybackPosition start, end;
            start.iIndeterminate = false;
            start.iPosUnit = PVPPBPOSUNIT_SAMPLENUMBER;
            start.iMode = PVPPBPOS_MODE_NOW;
            start.iPosValue.samplenum_value = 100;
            end.iIndeterminate = false;
            end.iPosUnit = PVPPBPOSUNIT_MILLISEC;
            end.iPosValue.millisec_value = 20000;
            OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRange(start, end, false, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_PREPARE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Prepare((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_START:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Start((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_STOPTIMENOTREACHED:
        {
            // Timeout when player doesn't stop automatically when stop time is reached
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;

        case STATE_STOP:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Stop((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_VIDEO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_AUDIO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_RESET:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Reset((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASOURCE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_CLEANUPANDCOMPLETE:
        {
            PVPATB_TEST_IS_TRUE(PVPlayerFactory::DeletePlayer(iPlayer));
            iPlayer = NULL;

            delete iDataSource;
            iDataSource = NULL;

            delete iDataSinkVideo;
            iDataSinkVideo = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeVideo);
            iIONodeVideo = NULL;

            iMioFactory->DestroyVideoOutput(iMIOFileOutVideo);
            iMIOFileOutVideo = NULL;

            delete iDataSinkAudio;
            iDataSinkAudio = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeAudio);
            iIONodeAudio = NULL;

            iMioFactory->DestroyAudioOutput(iMIOFileOutAudio);
            iMIOFileOutAudio = NULL;

            iObserver->TestCompleted(*iTestCase);
        }
        break;

        default:
            break;

    }
}


void pvplayer_async_test_setplayrangevidframenumplay::CommandCompleted(const PVCmdResponse& aResponse)
{
    if (aResponse.GetCmdId() != iCurrentCmdId)
    {
        // Wrong command ID.
        PVPATB_TEST_IS_TRUE(false);
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
        return;
    }

    if (aResponse.GetContext() != NULL)
    {
        if (aResponse.GetContext() == (OsclAny*)&iContextObject)
        {
            if (iContextObject != iContextObjectRefValue)
            {
                // Context data value was corrupted
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                return;
            }
        }
        else
        {
            // Context data pointer was corrupted
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
            return;
        }
    }

    switch (iState)
    {
        case STATE_ADDDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_INIT;
                RunIfNotReady();
            }
            else
            {
                // AddDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_INIT:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                // Init failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_SETPLAYBACKRANGE;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_SETPLAYBACKRANGE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_PREPARE;
                RunIfNotReady();
            }
            else
            {
                // SetPlaybackRange failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_PREPARE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_START;
                RunIfNotReady();
            }
            else
            {
                // Prepare failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_START:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_STOPTIMENOTREACHED;
                RunIfNotReady(25000000);
            }
            else
            {
                // Start failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_STOP:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                // Stop failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_RESET;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_RESET:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASOURCE;
                RunIfNotReady();
            }
            else
            {
                // Reset failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                PVPATB_TEST_IS_TRUE(true);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        default:
        {
            // Testing error if this is reached
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;
    }
}


void pvplayer_async_test_setplayrangevidframenumplay::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    switch (aEvent.GetEventType())
    {
        case PVMFErrResourceConfiguration:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrResource:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrCorrupt:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrProcessing:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        default:
            // Unknown error and just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;
    }
}


void pvplayer_async_test_setplayrangevidframenumplay::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingStart)
    {
        // do nothing
    }
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingComplete)
    {
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
    }
    // Check for stop time reached event
    if (aEvent.GetEventType() == PVMFInfoEndOfData)
    {
        PVInterface* iface = (PVInterface*)(aEvent.GetEventExtensionInterface());
        if (iface == NULL)
        {
            return;
        }
        PVUuid infomsguuid = PVMFErrorInfoMessageInterfaceUUID;
        PVMFErrorInfoMessageInterface* infomsgiface = NULL;
        if (iface->queryInterface(infomsguuid, (PVInterface*&)infomsgiface) == true)
        {
            int32 infocode;
            PVUuid infouuid;
            infomsgiface->GetCodeUUID(infocode, infouuid);
            if ((infouuid == PVPlayerErrorInfoEventTypesUUID) && (infocode == PVPlayerInfoEndTimeReached))
            {
                iState = STATE_STOP;
                Cancel();
                RunIfNotReady();
            }
        }
    }
}


//
// pvplayer_async_test_playsetplayrangestop section
//
void pvplayer_async_test_playsetplayrangestop::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_playsetplayrangestop::Run()
{
    int error = 0;

    switch (iState)
    {
        case STATE_CREATE:
        {
            iPlayer = NULL;

            OSCL_TRY(error, iPlayer = PVPlayerFactory::CreatePlayer(this, this, this));
            if (error)
            {
                PVPATB_TEST_IS_TRUE(false);
                iObserver->TestCompleted(*iTestCase);
            }
            else
            {
                iState = STATE_ADDDATASOURCE;
                RunIfNotReady();
            }
        }
        break;

        case STATE_ADDDATASOURCE:
        {
            iDataSource = new PVPlayerDataSourceURL;

            OSCL_wHeapString<OsclMemAllocator> sourcefile = SOURCENAME_PREPEND_WSTRING;
            sourcefile += _STRLIT_WCHAR("test_reposition.mp4");
            iDataSource->SetDataSourceURL(sourcefile);

            iDataSource->SetDataSourceFormatType(PVMF_MIME_MPEG4FF);
            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_INIT:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Init((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_VIDEO:
        {
            OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
            sinkfile += _STRLIT_WCHAR("test_player_playsetplayrangestop_video.dat");

            iMIOFileOutVideo = iMioFactory->CreateVideoOutput((OsclAny*) & sinkfile, MEDIATYPE_VIDEO, iCompressedVideo);
            iIONodeVideo = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutVideo);
            iDataSinkVideo = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkVideo)->SetDataSinkNode(iIONodeVideo);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_AUDIO:
        {
            OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
            sinkfile += _STRLIT_WCHAR("test_player_playsetplayrangestop_audio.dat");

            iMIOFileOutAudio = iMioFactory->CreateAudioOutput((OsclAny*) & sinkfile, MEDIATYPE_AUDIO, iCompressedAudio);
            iIONodeAudio = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutAudio);
            iDataSinkAudio = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkAudio)->SetDataSinkNode(iIONodeAudio);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_PREPARE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Prepare((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_START:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Start((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_SETPLAYBACKRANGE:
        {
            PVPPlaybackPosition start, end;
            start.iIndeterminate = false;
            start.iPosUnit = PVPPBPOSUNIT_MILLISEC;
            start.iMode = PVPPBPOS_MODE_NOW;
            start.iPosValue.millisec_value = 20000;
            end.iIndeterminate = true;
            OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRange(start, end, false, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_STOP:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Stop((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_VIDEO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_AUDIO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_RESET:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Reset((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASOURCE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_CLEANUPANDCOMPLETE:
        {
            PVPATB_TEST_IS_TRUE(PVPlayerFactory::DeletePlayer(iPlayer));
            iPlayer = NULL;

            delete iDataSource;
            iDataSource = NULL;

            delete iDataSinkVideo;
            iDataSinkVideo = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeVideo);
            iIONodeVideo = NULL;

            iMioFactory->DestroyVideoOutput(iMIOFileOutVideo);
            iMIOFileOutVideo = NULL;

            delete iDataSinkAudio;
            iDataSinkAudio = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeAudio);
            iIONodeAudio = NULL;

            iMioFactory->DestroyAudioOutput(iMIOFileOutAudio);
            iMIOFileOutAudio = NULL;

            iObserver->TestCompleted(*iTestCase);
        }
        break;

        default:
            break;

    }
}


void pvplayer_async_test_playsetplayrangestop::CommandCompleted(const PVCmdResponse& aResponse)
{
    if (aResponse.GetCmdId() != iCurrentCmdId)
    {
        // Wrong command ID.
        PVPATB_TEST_IS_TRUE(false);
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
        return;
    }

    if (aResponse.GetContext() != NULL)
    {
        if (aResponse.GetContext() == (OsclAny*)&iContextObject)
        {
            if (iContextObject != iContextObjectRefValue)
            {
                // Context data value was corrupted
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                return;
            }
        }
        else
        {
            // Context data pointer was corrupted
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
            return;
        }
    }

    switch (iState)
    {
        case STATE_ADDDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_INIT;
                RunIfNotReady();
            }
            else
            {
                // AddDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_INIT:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                // Init failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_PREPARE;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_PREPARE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_START;
                RunIfNotReady();
            }
            else
            {
                // Prepare failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_START:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_SETPLAYBACKRANGE;
                RunIfNotReady(10000000);
            }
            else
            {
                // Start failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_SETPLAYBACKRANGE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_STOP;
                RunIfNotReady(10000000);
            }
            else
            {
                // SetPlaybackRange failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_STOP:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                // Stop failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;
            break;

        case STATE_REMOVEDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_RESET;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_RESET:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASOURCE;
                RunIfNotReady();
            }
            else
            {
                // Reset failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                PVPATB_TEST_IS_TRUE(true);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        default:
        {
            // Testing error if this is reached
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;
    }
}


void pvplayer_async_test_playsetplayrangestop::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    switch (aEvent.GetEventType())
    {
        case PVMFErrResourceConfiguration:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrResource:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrCorrupt:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrProcessing:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        default:
            // Unknown error and just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;
    }
}


void pvplayer_async_test_playsetplayrangestop::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingStart)
    {
        // do nothing
    }
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingComplete)
    {
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
    }
}



//
// pvplayer_async_test_playsetplayrangevidframenumstop section
//
void pvplayer_async_test_playsetplayrangevidframenumstop::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_playsetplayrangevidframenumstop::Run()
{
    int error = 0;

    switch (iState)
    {
        case STATE_CREATE:
        {
            iPlayer = NULL;

            OSCL_TRY(error, iPlayer = PVPlayerFactory::CreatePlayer(this, this, this));
            if (error)
            {
                PVPATB_TEST_IS_TRUE(false);
                iObserver->TestCompleted(*iTestCase);
            }
            else
            {
                iState = STATE_ADDDATASOURCE;
                RunIfNotReady();
            }
        }
        break;

        case STATE_ADDDATASOURCE:
        {
            iDataSource = new PVPlayerDataSourceURL;

            OSCL_wHeapString<OsclMemAllocator> sourcefile = SOURCENAME_PREPEND_WSTRING;
            sourcefile += _STRLIT_WCHAR("test_reposition.mp4");
            iDataSource->SetDataSourceURL(sourcefile);

            iDataSource->SetDataSourceFormatType(PVMF_MIME_MPEG4FF);
            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_INIT:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Init((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_VIDEO:
        {
            OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
            sinkfile += _STRLIT_WCHAR("test_player_playsetplayrangevidframenumstop_video.dat");

            iMIOFileOutVideo = iMioFactory->CreateVideoOutput((OsclAny*) & sinkfile, MEDIATYPE_VIDEO, iCompressedVideo);
            iIONodeVideo = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutVideo);
            iDataSinkVideo = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkVideo)->SetDataSinkNode(iIONodeVideo);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_AUDIO:
        {
            OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
            sinkfile += _STRLIT_WCHAR("test_player_playsetplayrangevidframenumstop_audio.dat");

            iMIOFileOutAudio = iMioFactory->CreateAudioOutput((OsclAny*) & sinkfile, MEDIATYPE_AUDIO, iCompressedAudio);
            iIONodeAudio = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutAudio);
            iDataSinkAudio = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkAudio)->SetDataSinkNode(iIONodeAudio);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_PREPARE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Prepare((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_START:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Start((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_SETPLAYBACKRANGE:
        {
            PVPPlaybackPosition start, end;
            start.iIndeterminate = false;
            start.iPosUnit = PVPPBPOSUNIT_SAMPLENUMBER;
            start.iMode = PVPPBPOS_MODE_NOW;
            start.iPosValue.samplenum_value = 100;
            end.iIndeterminate = true;
            OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRange(start, end, false, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_STOP:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Stop((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_VIDEO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_AUDIO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_RESET:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Reset((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASOURCE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_CLEANUPANDCOMPLETE:
        {
            PVPATB_TEST_IS_TRUE(PVPlayerFactory::DeletePlayer(iPlayer));
            iPlayer = NULL;

            delete iDataSource;
            iDataSource = NULL;

            delete iDataSinkVideo;
            iDataSinkVideo = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeVideo);
            iIONodeVideo = NULL;

            iMioFactory->DestroyVideoOutput(iMIOFileOutVideo);
            iMIOFileOutVideo = NULL;

            delete iDataSinkAudio;
            iDataSinkAudio = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeAudio);
            iIONodeAudio = NULL;

            iMioFactory->DestroyAudioOutput(iMIOFileOutAudio);
            iMIOFileOutAudio = NULL;

            iObserver->TestCompleted(*iTestCase);
        }
        break;

        default:
            break;

    }
}


void pvplayer_async_test_playsetplayrangevidframenumstop::CommandCompleted(const PVCmdResponse& aResponse)
{
    if (aResponse.GetCmdId() != iCurrentCmdId)
    {
        // Wrong command ID.
        PVPATB_TEST_IS_TRUE(false);
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
        return;
    }

    if (aResponse.GetContext() != NULL)
    {
        if (aResponse.GetContext() == (OsclAny*)&iContextObject)
        {
            if (iContextObject != iContextObjectRefValue)
            {
                // Context data value was corrupted
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                return;
            }
        }
        else
        {
            // Context data pointer was corrupted
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
            return;
        }
    }

    switch (iState)
    {
        case STATE_ADDDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_INIT;
                RunIfNotReady();
            }
            else
            {
                // AddDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_INIT:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                // Init failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_PREPARE;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_PREPARE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_START;
                RunIfNotReady();
            }
            else
            {
                // Prepare failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_START:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_SETPLAYBACKRANGE;
                RunIfNotReady(10000000);
            }
            else
            {
                // Start failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_SETPLAYBACKRANGE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_STOP;
                RunIfNotReady(10000000);
            }
            else
            {
                // SetPlaybackRange failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_STOP:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                // Stop failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;
            break;

        case STATE_REMOVEDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_RESET;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_RESET:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASOURCE;
                RunIfNotReady();
            }
            else
            {
                // Reset failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                PVPATB_TEST_IS_TRUE(true);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        default:
        {
            // Testing error if this is reached
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;
    }
}


void pvplayer_async_test_playsetplayrangevidframenumstop::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    switch (aEvent.GetEventType())
    {
        case PVMFErrResourceConfiguration:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrResource:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrCorrupt:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrProcessing:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        default:
            // Unknown error and just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;
    }
}


void pvplayer_async_test_playsetplayrangevidframenumstop::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingStart)
    {
        // do nothing
    }
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingComplete)
    {
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
    }
}


//
// pvplayer_async_test_tracklevelinfo section
//
void pvplayer_async_test_tracklevelinfo::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_tracklevelinfo::Run()
{
    int error = 0;

    switch (iState)
    {
        case STATE_CREATE:
        {
            iPlayer = NULL;

            OSCL_TRY(error, iPlayer = PVPlayerFactory::CreatePlayer(this, this, this));
            if (error)
            {
                PVPATB_TEST_IS_TRUE(false);
                iObserver->TestCompleted(*iTestCase);
            }
            else
            {
                iState = STATE_INVALIDQUERYINTERFACE1;
                RunIfNotReady();
            }
        }
        break;

        case STATE_INVALIDQUERYINTERFACE1:
        {
            PVUuid tracklevelinfouuid = PVMF_TRACK_LEVEL_INFO_INTERFACE_UUID;
            OSCL_TRY(error, iCurrentCmdId = iPlayer->QueryInterface(tracklevelinfouuid, (PVInterface*&)iTrackLevelInfoIF, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASOURCE:
        {
            iDataSource = new PVPlayerDataSourceURL;

            oscl_UTF8ToUnicode(iFileName, oscl_strlen(iFileName), output, 512);
            wFileName.set(output, oscl_strlen(output));
            iDataSource->SetDataSourceURL(wFileName);

            iDataSource->SetDataSourceFormatType(iFileType);
            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }

        break;

        case STATE_INIT:
        {

            OSCL_TRY(error, iCurrentCmdId = iPlayer->Init((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_VIDEO:
        {
            OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
            sinkfile += _STRLIT_WCHAR("test_player_tracklevelinfo_video.dat");

            iMIOFileOutVideo = iMioFactory->CreateVideoOutput((OsclAny*) & sinkfile, MEDIATYPE_VIDEO, iCompressedVideo);
            iIONodeVideo = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutVideo);
            iDataSinkVideo = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkVideo)->SetDataSinkNode(iIONodeVideo);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_AUDIO:
        {
            OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
            sinkfile += _STRLIT_WCHAR("test_player_tracklevelinfo_audio.dat");

            iMIOFileOutAudio = iMioFactory->CreateAudioOutput((OsclAny*) & sinkfile, MEDIATYPE_AUDIO, iCompressedAudio);
            iIONodeAudio = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutAudio);
            iDataSinkAudio = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkAudio)->SetDataSinkNode(iIONodeAudio);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_PREPARE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Prepare((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_START:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Start((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_QUERYINTERFACE:
        {
            PVUuid tracklevelinfouuid = PVMF_TRACK_LEVEL_INFO_INTERFACE_UUID;
            OSCL_TRY(error, iCurrentCmdId = iPlayer->QueryInterface(tracklevelinfouuid, (PVInterface*&)iTrackLevelInfoIF, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_TRACKLEVELINFOTEST:
        {
            Oscl_Vector<PVMFTrackInfo, OsclMemAllocator> availabletracks;
            if (iTrackLevelInfoIF->GetAvailableTracks(availabletracks) != PVMFSuccess)
            {
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                break;
            }

            uint32 audioTrackID[MAX_AUDIO_TRACKS];
            uint32 videoTrackID[MAX_VIDEO_TRACKS];

            oscl_memset(audioTrackID, '\0', MAX_AUDIO_TRACKS);
            oscl_memset(videoTrackID, '\0', MAX_VIDEO_TRACKS);

            uint32 audioTrackCnt = 0;
            uint32 videoTrackCnt = 0;

            for (uint32 i = 0; i < availabletracks.size(); ++i)
            {
                PVMFFormatType Format = availabletracks[i].getTrackMimeType().get_str();
                if (Format.isAudio())
                {
                    audioTrackID[audioTrackCnt++] = i;
                }
                else if (Format.isVideo() || Format.isImage())
                {
                    videoTrackID[videoTrackCnt++] = i;
                }
            }

            // Check the video track info
            PVMFTimestamp samplets = 0;
            uint32 samplenum = 0;
            int32 numsyncsamples = 0;
            PVMFSampleNumTSList syncnumtslist;

            // There is reason to fail the test case just 'cos a clip doesnt have audio or video.
            // Below loops of getting audio or video sample info will not be executed anyways if videoTrackCnt or audioTrackCnt are 0.

            for (uint32 j = 0; j < videoTrackCnt; j++)
            {
                uint32 tempID = videoTrackID[j];
                if (iTrackLevelInfoIF->GetNumberOfSyncSamples(availabletracks[tempID], numsyncsamples) != PVMFSuccess)
                {
                    PVPATB_TEST_IS_TRUE(false);
                }
                else
                {
                    // Check the number of sync samples
                    if (!(oscl_strcmp(iFileName, "test_tracklevelinfo.mp4")))
                        PVPATB_TEST_IS_TRUE(numsyncsamples == 6);
                }

                int32 aTargetTimeInMS = 35000;
                int32 EndIdx = 2;
                uint32 howManySamples = 2, startIdx = 3;
                if (iTrackLevelInfoIF->GetSyncSampleInfo(syncnumtslist, availabletracks[tempID], aTargetTimeInMS, howManySamples) != PVMFSuccess)
                {
                    PVPATB_TEST_IS_TRUE(false);
                }

                if (iTrackLevelInfoIF->GetSyncSampleInfo(availabletracks[tempID], syncnumtslist, startIdx, EndIdx) != PVMFSuccess)
                {
                    PVPATB_TEST_IS_TRUE(false);
                }
                else
                {
                    // Check the returned info on sync samples
                    if (!(oscl_strcmp(iFileName, "test_tracklevelinfo.mp4")))
                    {
                        PVPATB_TEST_IS_TRUE(syncnumtslist[0].iSampleNumber == 256);
                        PVPATB_TEST_IS_TRUE(syncnumtslist[0].iTimestamp == 22250);
                        PVPATB_TEST_IS_TRUE(syncnumtslist[1].iSampleNumber == 331);
                        PVPATB_TEST_IS_TRUE(syncnumtslist[1].iTimestamp == 32333);
                    }
                }

                if (!(oscl_strcmp(iFileName, "test_tracklevelinfo.mp4")))
                {
                    PVMFTimestamp samplets = 0;
                    if (iTrackLevelInfoIF->GetTimestampForSampleNumber(availabletracks[tempID], 100, samplets) != PVMFSuccess)
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }
                    else
                    {
                        PVPATB_TEST_IS_TRUE(samplets == 8416);
                    }

                    samplets = 0;
                    if (iTrackLevelInfoIF->GetTimestampForSampleNumber(availabletracks[tempID], 361, samplets) != PVMFSuccess)
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }
                    else
                    {
                        PVPATB_TEST_IS_TRUE(samplets == 36166);
                    }

                    samplets = 0;
                    if (iTrackLevelInfoIF->GetTimestampForSampleNumber(availabletracks[tempID], 600, samplets) != PVMFSuccess)
                    {
                        PVPATB_TEST_IS_TRUE(true);
                    }
                    else
                    {
                        // Beyond the last sample so the call should fail
                        PVPATB_TEST_IS_TRUE(false);
                    }

                    uint32 samplenum = 0;
                    if (iTrackLevelInfoIF->GetSampleNumberForTimestamp(availabletracks[tempID], 5700, samplenum) != PVMFSuccess)
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }
                    else
                    {
                        PVPATB_TEST_IS_TRUE(samplenum == 67);
                    }

                    samplenum = 0;
                    if (iTrackLevelInfoIF->GetSampleNumberForTimestamp(availabletracks[tempID], 12000, samplenum) != PVMFSuccess)
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }
                    else
                    {
                        PVPATB_TEST_IS_TRUE(samplenum == 143);
                    }

                    samplenum = 0;
                    if (iTrackLevelInfoIF->GetSampleNumberForTimestamp(availabletracks[tempID], 45000, samplenum) != PVMFSuccess)
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }
                    else
                    {
                        // Beyond the duration of track so sample number should be the (last sample+1)
                        PVPATB_TEST_IS_TRUE(samplenum == 409);
                    }
                }
            }

            for (uint32 k = 0; k < audioTrackCnt; k++)
            {
                // Check the audio track info
                numsyncsamples = 0;
                uint32 tempID = audioTrackID[k];

                if (!(oscl_strcmp(iFileName, "test_tracklevelinfo.mp4")))
                {
                    samplets = 0;
                    if (iTrackLevelInfoIF->GetTimestampForSampleNumber(availabletracks[tempID], 33, samplets) != PVMFSuccess)
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }
                    else
                    {
                        PVPATB_TEST_IS_TRUE(samplets == 660);
                    }

                    samplets = 0;
                    if (iTrackLevelInfoIF->GetTimestampForSampleNumber(availabletracks[tempID], 1483, samplets) != PVMFSuccess)
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }
                    else
                    {
                        PVPATB_TEST_IS_TRUE(samplets == 29660);
                    }

                    samplets = 0;
                    if (iTrackLevelInfoIF->GetTimestampForSampleNumber(availabletracks[tempID], 2500, samplets) != PVMFSuccess)
                    {
                        PVPATB_TEST_IS_TRUE(true);
                    }
                    else
                    {
                        // Beyond the last sample so query should fail
                        PVPATB_TEST_IS_TRUE(false);
                    }

                    samplenum = 0;
                    if (iTrackLevelInfoIF->GetSampleNumberForTimestamp(availabletracks[tempID], 844, samplenum) != PVMFSuccess)
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }
                    else
                    {
                        PVPATB_TEST_IS_TRUE(samplenum == 42);
                    }

                    samplenum = 0;
                    if (iTrackLevelInfoIF->GetSampleNumberForTimestamp(availabletracks[tempID], 12100, samplenum) != PVMFSuccess)
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }
                    else
                    {
                        PVPATB_TEST_IS_TRUE(samplenum == 605);
                    }

                    samplenum = 0;
                    if (iTrackLevelInfoIF->GetSampleNumberForTimestamp(availabletracks[tempID], 45000, samplenum) != PVMFSuccess)
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }
                    else
                    {
                        PVPATB_TEST_IS_TRUE(samplenum == 2120);
                    }
                }
            }
            iTrackLevelInfoIF->removeRef();
            iTrackLevelInfoIF = NULL;

            iState = STATE_STOP;
            RunIfNotReady();
        }
        break;

        case STATE_STOP:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Stop((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_VIDEO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_AUDIO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_RESET:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Reset((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASOURCE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_INVALIDQUERYINTERFACE2:
        {
            PVUuid tracklevelinfouuid = PVMF_TRACK_LEVEL_INFO_INTERFACE_UUID;
            OSCL_TRY(error, iCurrentCmdId = iPlayer->QueryInterface(tracklevelinfouuid, (PVInterface*&)iTrackLevelInfoIF, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_CLEANUPANDCOMPLETE:
        {
            if (iTrackLevelInfoIF)
            {
                iTrackLevelInfoIF->removeRef();
                iTrackLevelInfoIF = NULL;
            }

            PVPATB_TEST_IS_TRUE(PVPlayerFactory::DeletePlayer(iPlayer));
            iPlayer = NULL;

            delete iDataSource;
            iDataSource = NULL;

            delete iDataSinkVideo;
            iDataSinkVideo = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeVideo);
            iIONodeVideo = NULL;

            iMioFactory->DestroyVideoOutput(iMIOFileOutVideo);
            iMIOFileOutVideo = NULL;

            delete iDataSinkAudio;
            iDataSinkAudio = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeAudio);
            iIONodeAudio = NULL;

            iMioFactory->DestroyAudioOutput(iMIOFileOutAudio);
            iMIOFileOutAudio = NULL;

            iObserver->TestCompleted(*iTestCase);
        }
        break;

        default:
            break;

    }
}


void pvplayer_async_test_tracklevelinfo::CommandCompleted(const PVCmdResponse& aResponse)
{
    if (aResponse.GetCmdId() != iCurrentCmdId)
    {
        // Wrong command ID.
        PVPATB_TEST_IS_TRUE(false);
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
        return;
    }

    if (aResponse.GetContext() != NULL)
    {
        if (aResponse.GetContext() == (OsclAny*)&iContextObject)
        {
            if (iContextObject != iContextObjectRefValue)
            {
                // Context data value was corrupted
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                return;
            }
        }
        else
        {
            // Context data pointer was corrupted
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
            return;
        }
    }

    switch (iState)
    {
        case STATE_INVALIDQUERYINTERFACE1:
            if (aResponse.GetCmdStatus() != PVMFSuccess)
            {
                iState = STATE_ADDDATASOURCE;
                RunIfNotReady();
            }
            else
            {
                // QueryInterface() should not succeed at this state
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_INIT;
                RunIfNotReady();
            }
            else
            {
                // AddDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;



        case STATE_INIT:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                // Init failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_PREPARE;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_PREPARE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_START;
                RunIfNotReady();
            }
            else
            {
                // Prepare failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_START:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_QUERYINTERFACE;
                RunIfNotReady();
            }
            else
            {
                // Start failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_QUERYINTERFACE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_TRACKLEVELINFOTEST;
                RunIfNotReady();
            }
            else
            {
                // QueryInterface failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_STOP:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                // Stop failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_RESET;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_RESET:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASOURCE;
                RunIfNotReady();
            }
            else
            {
                // Reset failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_INVALIDQUERYINTERFACE2;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_INVALIDQUERYINTERFACE2:
            if (aResponse.GetCmdStatus() != PVMFSuccess)
            {
                PVPATB_TEST_IS_TRUE(true);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                // QueryInterface() should not succeed at this state
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        default:
        {
            // Testing error if this is reached
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;
    }
}


void pvplayer_async_test_tracklevelinfo::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    switch (aEvent.GetEventType())
    {
        case PVMFErrResourceConfiguration:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrResource:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrCorrupt:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrProcessing:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        default:
            // Unknown error and just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;
    }
}


void pvplayer_async_test_tracklevelinfo::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingStart)
    {
        // do nothing
    }
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingComplete)
    {
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
    }
}



//
// pvplayer_async_test_setplaybackrate2X section
//
void pvplayer_async_test_setplaybackrate2X::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_setplaybackrate2X::Run()
{
    int error = 0;

    switch (iState)
    {
        case STATE_CREATE:
        {
            iPlayer = NULL;

            OSCL_TRY(error, iPlayer = PVPlayerFactory::CreatePlayer(this, this, this));
            if (error)
            {
                PVPATB_TEST_IS_TRUE(false);
                iObserver->TestCompleted(*iTestCase);
            }
            else
            {
                iState = STATE_ADDDATASOURCE;
                RunIfNotReady();
            }
        }
        break;

        case STATE_ADDDATASOURCE:
        {
            iDataSource = new PVPlayerDataSourceURL;

            OSCL_wHeapString<OsclMemAllocator> sourcefile = SOURCENAME_PREPEND_WSTRING;
            sourcefile += _STRLIT_WCHAR("test_reposition.mp4");
            iDataSource->SetDataSourceURL(sourcefile);

            iDataSource->SetDataSourceFormatType(PVMF_MIME_MPEG4FF);
            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_INIT:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Init((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_VIDEO:
        {
            OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
            sinkfile += _STRLIT_WCHAR("test_player_setplaybackrate2x_video.dat");

            iMIOFileOutVideo = iMioFactory->CreateVideoOutput((OsclAny*) & sinkfile, MEDIATYPE_VIDEO, iCompressedVideo);
            iIONodeVideo = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutVideo);
            iDataSinkVideo = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkVideo)->SetDataSinkNode(iIONodeVideo);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_AUDIO:
        {
            OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
            sinkfile += _STRLIT_WCHAR("test_player_setplaybackrate2x_audio.dat");

            iMIOFileOutAudio = iMioFactory->CreateAudioOutput((OsclAny*) & sinkfile, MEDIATYPE_AUDIO, iCompressedAudio);
            iIONodeAudio = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutAudio);
            iDataSinkAudio = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkAudio)->SetDataSinkNode(iIONodeAudio);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_PREPARE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Prepare((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_SETPLAYBACKRATE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRate(200000, NULL, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_START:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Start((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_STOP:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Stop((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_VIDEO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_AUDIO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_RESET:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Reset((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASOURCE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_CLEANUPANDCOMPLETE:
        {
            PVPATB_TEST_IS_TRUE(PVPlayerFactory::DeletePlayer(iPlayer));
            iPlayer = NULL;

            delete iDataSource;
            iDataSource = NULL;

            delete iDataSinkVideo;
            iDataSinkVideo = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeVideo);
            iIONodeVideo = NULL;

            iMioFactory->DestroyVideoOutput(iMIOFileOutVideo);
            iMIOFileOutVideo = NULL;

            delete iDataSinkAudio;
            iDataSinkAudio = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeAudio);
            iIONodeAudio = NULL;

            iMioFactory->DestroyAudioOutput(iMIOFileOutAudio);
            iMIOFileOutAudio = NULL;

            iObserver->TestCompleted(*iTestCase);
        }
        break;

        default:
            break;

    }
}


void pvplayer_async_test_setplaybackrate2X::CommandCompleted(const PVCmdResponse& aResponse)
{
    if (aResponse.GetCmdId() != iCurrentCmdId)
    {
        // Wrong command ID.
        PVPATB_TEST_IS_TRUE(false);
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
        return;
    }

    if (aResponse.GetContext() != NULL)
    {
        if (aResponse.GetContext() == (OsclAny*)&iContextObject)
        {
            if (iContextObject != iContextObjectRefValue)
            {
                // Context data value was corrupted
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                return;
            }
        }
        else
        {
            // Context data pointer was corrupted
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
            return;
        }
    }

    switch (iState)
    {
        case STATE_ADDDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_INIT;
                RunIfNotReady();
            }
            else
            {
                // AddDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_INIT:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                // Init failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_PREPARE;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_PREPARE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_SETPLAYBACKRATE;
                RunIfNotReady();
            }
            else
            {
                // Prepare failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_SETPLAYBACKRATE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_START;
                RunIfNotReady();
            }
            else
            {
                // SetPlaybackRate failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_STOP;
                RunIfNotReady();
            }
            break;

        case STATE_START:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_STOP;
                RunIfNotReady(15000000);
            }
            else
            {
                // Start failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_STOP:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                // Start failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_RESET;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_RESET:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASOURCE;
                RunIfNotReady();
            }
            else
            {
                // Reset failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                PVPATB_TEST_IS_TRUE(true);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        default:
        {
            // Testing error if this is reached
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;
    }
}


void pvplayer_async_test_setplaybackrate2X::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    switch (aEvent.GetEventType())
    {
        case PVMFErrResourceConfiguration:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrResource:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrCorrupt:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrProcessing:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        default:
            // Unknown error and just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;
    }
}


void pvplayer_async_test_setplaybackrate2X::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingStart)
    {
        // do nothing
    }
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingComplete)
    {
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
    }
}


//
// pvplayer_async_test_setplaybackratefifth section
//
void pvplayer_async_test_setplaybackratefifth::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_setplaybackratefifth::Run()
{
    int error = 0;

    switch (iState)
    {
        case STATE_CREATE:
        {
            iPlayer = NULL;

            OSCL_TRY(error, iPlayer = PVPlayerFactory::CreatePlayer(this, this, this));
            if (error)
            {
                PVPATB_TEST_IS_TRUE(false);
                iObserver->TestCompleted(*iTestCase);
            }
            else
            {
                iState = STATE_ADDDATASOURCE;
                RunIfNotReady();
            }
        }
        break;

        case STATE_ADDDATASOURCE:
        {
            iDataSource = new PVPlayerDataSourceURL;

            OSCL_wHeapString<OsclMemAllocator> sourcefile = SOURCENAME_PREPEND_WSTRING;
            sourcefile += _STRLIT_WCHAR("test_reposition.mp4");
            iDataSource->SetDataSourceURL(sourcefile);

            iDataSource->SetDataSourceFormatType(PVMF_MIME_MPEG4FF);
            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_INIT:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Init((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_VIDEO:
        {
            OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
            sinkfile += _STRLIT_WCHAR("test_player_setplaybackratefifth_video.dat");

            iMIOFileOutVideo = iMioFactory->CreateVideoOutput((OsclAny*) & sinkfile, MEDIATYPE_VIDEO, iCompressedVideo);
            iIONodeVideo = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutVideo);
            iDataSinkVideo = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkVideo)->SetDataSinkNode(iIONodeVideo);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_AUDIO:
        {
            OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
            sinkfile += _STRLIT_WCHAR("test_player_setplaybackratefifth_audio.dat");

            iMIOFileOutAudio = iMioFactory->CreateAudioOutput((OsclAny*) & sinkfile, MEDIATYPE_AUDIO, iCompressedAudio);
            iIONodeAudio = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutAudio);
            iDataSinkAudio = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkAudio)->SetDataSinkNode(iIONodeAudio);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_PREPARE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Prepare((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_SETPLAYBACKRATE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRate(20000, NULL, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_START:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Start((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_STOP:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Stop((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_VIDEO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_AUDIO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_RESET:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Reset((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASOURCE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_CLEANUPANDCOMPLETE:
        {
            PVPATB_TEST_IS_TRUE(PVPlayerFactory::DeletePlayer(iPlayer));
            iPlayer = NULL;

            delete iDataSource;
            iDataSource = NULL;

            delete iDataSinkVideo;
            iDataSinkVideo = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeVideo);
            iIONodeVideo = NULL;

            iMioFactory->DestroyVideoOutput(iMIOFileOutVideo);
            iMIOFileOutVideo = NULL;

            delete iDataSinkAudio;
            iDataSinkAudio = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeAudio);
            iIONodeAudio = NULL;

            iMioFactory->DestroyAudioOutput(iMIOFileOutAudio);
            iMIOFileOutAudio = NULL;

            iObserver->TestCompleted(*iTestCase);
        }
        break;

        default:
            break;

    }
}


void pvplayer_async_test_setplaybackratefifth::CommandCompleted(const PVCmdResponse& aResponse)
{
    if (aResponse.GetCmdId() != iCurrentCmdId)
    {
        // Wrong command ID.
        PVPATB_TEST_IS_TRUE(false);
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
        return;
    }

    if (aResponse.GetContext() != NULL)
    {
        if (aResponse.GetContext() == (OsclAny*)&iContextObject)
        {
            if (iContextObject != iContextObjectRefValue)
            {
                // Context data value was corrupted
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                return;
            }
        }
        else
        {
            // Context data pointer was corrupted
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
            return;
        }
    }

    switch (iState)
    {
        case STATE_ADDDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_INIT;
                RunIfNotReady();
            }
            else
            {
                // AddDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_INIT:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                // Init failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_PREPARE;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_PREPARE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_SETPLAYBACKRATE;
                RunIfNotReady();
            }
            else
            {
                // Prepare failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_SETPLAYBACKRATE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_START;
                RunIfNotReady();
            }
            else
            {
                // SetPlaybackRate failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_STOP;
                RunIfNotReady();
            }
            break;

        case STATE_START:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_STOP;
                RunIfNotReady(15000000);
            }
            else
            {
                // Start failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_STOP:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                // Start failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_RESET;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_RESET:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASOURCE;
                RunIfNotReady();
            }
            else
            {
                // Reset failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                PVPATB_TEST_IS_TRUE(true);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        default:
        {
            // Testing error if this is reached
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;
    }
}


void pvplayer_async_test_setplaybackratefifth::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    switch (aEvent.GetEventType())
    {
        case PVMFErrResourceConfiguration:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrResource:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrCorrupt:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrProcessing:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        default:
            // Unknown error and just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;
    }
}


void pvplayer_async_test_setplaybackratefifth::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingStart)
    {
        // do nothing
    }
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingComplete)
    {
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
    }
}


//
// pvplayer_async_test_queuedcommands section
//
void pvplayer_async_test_queuedcommands::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_queuedcommands::Run()
{
    int error = 0;

    switch (iState)
    {
        case STATE_CREATE:
        {
            iPlayer = NULL;

            OSCL_TRY(error, iPlayer = PVPlayerFactory::CreatePlayer(this, this, this));
            if (error)
            {
                PVPATB_TEST_IS_TRUE(false);
                iObserver->TestCompleted(*iTestCase);
            }
            else
            {
                iState = STATE_QUEUEDCMD1;
                RunIfNotReady();
            }
        }
        break;

        case STATE_QUEUEDCMD1:
        {
            OSCL_wHeapString<OsclMemAllocator> SinkFileName;
            // Add the data source
            iDataSource = new PVPlayerDataSourceURL;
            oscl_UTF8ToUnicode(iFileName, oscl_strlen(iFileName), output, 512);
            wFileName.set(output, oscl_strlen(output));
            iDataSource->SetDataSourceURL(wFileName);
            iDataSource->SetDataSourceFormatType(iFileType);
            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            iCmdIds.push_back(iCurrentCmdId);
            // Initialize
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Init((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            iCmdIds.push_back(iCurrentCmdId);

            // Add video data sink
            SinkFileName = OUTPUTNAME_PREPEND_WSTRING;
            SinkFileName += _STRLIT_WCHAR("test_player_queuedcmd_");
            OSCL_wHeapString<OsclMemAllocator> inputfilename;
            RetrieveFilename(wFileName.get_str(), inputfilename);
            SinkFileName += inputfilename;
            SinkFileName += _STRLIT_WCHAR("_video.dat");

            iMIOFileOutVideo = iMioFactory->CreateVideoOutput((OsclAny*) & SinkFileName, MEDIATYPE_VIDEO, iCompressedVideo);
            iIONodeVideo = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutVideo);
            iDataSinkVideo = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkVideo)->SetDataSinkNode(iIONodeVideo);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            iCmdIds.push_back(iCurrentCmdId);


            // Add audio data sink
            SinkFileName = OUTPUTNAME_PREPEND_WSTRING;
            SinkFileName += _STRLIT_WCHAR("test_player_queuedcmd_");
            SinkFileName += inputfilename;
            SinkFileName += _STRLIT_WCHAR("_audio.dat");

            iMIOFileOutAudio = iMioFactory->CreateAudioOutput((OsclAny*) & SinkFileName, MEDIATYPE_AUDIO, iCompressedAudio);
            iIONodeAudio = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutAudio);
            iDataSinkAudio = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkAudio)->SetDataSinkNode(iIONodeAudio);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            iCmdIds.push_back(iCurrentCmdId);
        }
        break;

        case STATE_QUEUEDCMD2:
        {
            // Prepare
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Prepare((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            iCmdIds.push_back(iCurrentCmdId);
            // Start
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Start((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            iCmdIds.push_back(iCurrentCmdId);
        }
        break;

        case STATE_QUEUEDCMD3:
        {
            // Stop
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Stop((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            iCmdIds.push_back(iCurrentCmdId);
            // Remove video data sink
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            iCmdIds.push_back(iCurrentCmdId);
        }
        break;

        case STATE_QUEUEDCMD4:
        {
            // Remove audio data sink
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            iCmdIds.push_back(iCurrentCmdId);
            // Reset
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Reset((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            iCmdIds.push_back(iCurrentCmdId);
            // Remove data source
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            iCmdIds.push_back(iCurrentCmdId);
        }
        break;

        case STATE_CLEANUPANDCOMPLETE:
        {
            PVPATB_TEST_IS_TRUE(PVPlayerFactory::DeletePlayer(iPlayer));
            iPlayer = NULL;

            delete iDataSource;
            iDataSource = NULL;

            delete iDataSinkVideo;
            iDataSinkVideo = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeVideo);
            iIONodeVideo = NULL;

            iMioFactory->DestroyVideoOutput(iMIOFileOutVideo);
            iMIOFileOutVideo = NULL;

            delete iDataSinkAudio;
            iDataSinkAudio = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeAudio);
            iIONodeAudio = NULL;

            iMioFactory->DestroyAudioOutput(iMIOFileOutAudio);
            iMIOFileOutAudio = NULL;

            iObserver->TestCompleted(*iTestCase);
        }
        break;

        default:
            break;

    }
}


void pvplayer_async_test_queuedcommands::CommandCompleted(const PVCmdResponse& aResponse)
{
    // Assumes the engine commands are completed in the order they are requested
    if (aResponse.GetCmdId() != iCmdIds[0])
    {
        // Wrong command ID.
        PVPATB_TEST_IS_TRUE(false);
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
        return;
    }

    if (aResponse.GetContext() != NULL)
    {
        if (aResponse.GetContext() == (OsclAny*)&iContextObject)
        {
            if (iContextObject != iContextObjectRefValue)
            {
                // Context data value was corrupted
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                return;
            }
        }
        else
        {
            // Context data pointer was corrupted
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
            return;
        }
    }

    switch (iState)
    {
        case STATE_QUEUEDCMD1:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iCmdIds.erase(iCmdIds.begin());
                if (iCmdIds.empty())
                {
                    // All requested commands have completed
                    iState = STATE_QUEUEDCMD2;
                    RunIfNotReady();
                }
            }
            else
            {
                // One of queued commands failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_QUEUEDCMD2:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iCmdIds.erase(iCmdIds.begin());
                if (iCmdIds.empty())
                {
                    // All requested commands have completed
                    iState = STATE_QUEUEDCMD3;
                    RunIfNotReady(10000000);
                }
            }
            else
            {
                // One of queued commands failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_QUEUEDCMD3:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iCmdIds.erase(iCmdIds.begin());
                if (iCmdIds.empty())
                {
                    // All requested commands have completed
                    iState = STATE_QUEUEDCMD4;
                    RunIfNotReady();
                }
            }
            else
            {
                // One of queued commands failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_QUEUEDCMD4:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iCmdIds.erase(iCmdIds.begin());
                if (iCmdIds.empty())
                {
                    // All requested commands have completed
                    PVPATB_TEST_IS_TRUE(true);
                    iState = STATE_CLEANUPANDCOMPLETE;
                    RunIfNotReady();
                }
            }
            else
            {
                // One of queued commands failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        default:
        {
            // Testing error if this is reached
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;
    }
}


void pvplayer_async_test_queuedcommands::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    switch (aEvent.GetEventType())
    {
        case PVMFErrResourceConfiguration:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrResource:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrCorrupt:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrProcessing:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        default:
            // Unknown error and just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;
    }
}


void pvplayer_async_test_queuedcommands::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingStart)
    {
        // do nothing
    }
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingComplete)
    {
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
    }
}



//
// pvplayer_async_test_looping section
//
void pvplayer_async_test_looping::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_looping::Run()
{
    int error = 0;

    switch (iState)
    {
        case STATE_CREATE:
        {
            iPlayer = NULL;

            OSCL_TRY(error, iPlayer = PVPlayerFactory::CreatePlayer(this, this, this));
            if (error)
            {
                PVPATB_TEST_IS_TRUE(false);
                iObserver->TestCompleted(*iTestCase);
            }
            else
            {
                iState = STATE_STARTPLAYBACK;
                RunIfNotReady();
            }
        }
        break;

        case STATE_STARTPLAYBACK:
        {
            OSCL_wHeapString<OsclMemAllocator> SinkFileName;
            OSCL_wHeapString<OsclMemAllocator> inputfilename;
            RetrieveFilename(wFileName.get_str(), inputfilename);

            // Add the data source
            iDataSource = new PVPlayerDataSourceURL;
            oscl_UTF8ToUnicode(iFileName, oscl_strlen(iFileName), output, 512);
            wFileName.set(output, oscl_strlen(output));
            iDataSource->SetDataSourceURL(wFileName);
            iDataSource->SetDataSourceFormatType(iFileType);
            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            iCmdIds.push_back(iCurrentCmdId);
            // Initialize
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Init((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            iCmdIds.push_back(iCurrentCmdId);

            // Add video data sink
            SinkFileName = OUTPUTNAME_PREPEND_WSTRING;
            SinkFileName += _STRLIT_WCHAR("test_player_looping_");
            SinkFileName += inputfilename;
            SinkFileName += _STRLIT_WCHAR("_video.dat");

            iMIOFileOutVideo = iMioFactory->CreateVideoOutput((OsclAny*) & SinkFileName, MEDIATYPE_VIDEO, iCompressedVideo);
            iIONodeVideo = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutVideo);
            iDataSinkVideo = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkVideo)->SetDataSinkNode(iIONodeVideo);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            iCmdIds.push_back(iCurrentCmdId);

            // Add audio data sink
            SinkFileName = OUTPUTNAME_PREPEND_WSTRING;
            SinkFileName += _STRLIT_WCHAR("test_player_looping_");
            SinkFileName += inputfilename;
            SinkFileName += _STRLIT_WCHAR("_audio.dat");

            iMIOFileOutAudio = iMioFactory->CreateAudioOutput((OsclAny*) & SinkFileName, MEDIATYPE_AUDIO, iCompressedAudio);
            iIONodeAudio = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutAudio);
            iDataSinkAudio = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkAudio)->SetDataSinkNode(iIONodeAudio);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            iCmdIds.push_back(iCurrentCmdId);

            // Add text data sink
            SinkFileName = OUTPUTNAME_PREPEND_WSTRING;
            SinkFileName += _STRLIT_WCHAR("test_player_looping_");
            SinkFileName += inputfilename;
            SinkFileName += _STRLIT_WCHAR("_text.dat");

            iMIOFileOutText = iMioFactory->CreateTextOutput((OsclAny*) & SinkFileName, MEDIATYPE_TEXT);
            iIONodeText = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutText);
            iDataSinkText = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkText)->SetDataSinkNode(iIONodeText);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkText, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            iCmdIds.push_back(iCurrentCmdId);

            // Prepare
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Prepare((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            iCmdIds.push_back(iCurrentCmdId);
            // Start
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Start((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            iCmdIds.push_back(iCurrentCmdId);
        }
        break;

        case STATE_SETPLAYBACKRANGE1:
        {
            PVPPlaybackPosition start, end;
            start.iIndeterminate = true;
            start.iMode = PVPPBPOS_MODE_NOW;
            end.iIndeterminate = false;
            end.iPosUnit = PVPPBPOSUNIT_MILLISEC;
            end.iPosValue.millisec_value = 10000;
            OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRange(start, end, false, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            iCmdIds.push_back(iCurrentCmdId);
        }
        break;

        case STATE_ENDTIMENOTREACHED1:
        {
            // Timeout when player doesn't stop automatically when stop time is reached
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;

        case STATE_SETPLAYBACKRANGE2:
        {
            PVPPlaybackPosition start, end;
            start.iIndeterminate = false;
            start.iPosUnit = PVPPBPOSUNIT_MILLISEC;
            start.iMode = PVPPBPOS_MODE_NOW;
            start.iPosValue.millisec_value = 0;
            end.iIndeterminate = false;
            end.iPosUnit = PVPPBPOSUNIT_MILLISEC;
            end.iPosValue.millisec_value = 10000;
            OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRange(start, end, false, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            iCmdIds.push_back(iCurrentCmdId);
        }
        break;

        case STATE_RESUME:
        {
            // Resume
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Resume((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            iCmdIds.push_back(iCurrentCmdId);
        }
        break;

        case STATE_ENDTIMENOTREACHED2:
        {
            // Timeout when player doesn't stop automatically when stop time is reached
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;

        case STATE_SHUTDOWN:
        {
            // Stop
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Stop((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            iCmdIds.push_back(iCurrentCmdId);
            // Remove video data sink
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            iCmdIds.push_back(iCurrentCmdId);
            // Remove audio data sink
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            iCmdIds.push_back(iCurrentCmdId);
            // Remove text data sink
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkText, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            iCmdIds.push_back(iCurrentCmdId);
            // Reset
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Reset((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            iCmdIds.push_back(iCurrentCmdId);
            // Remove data source
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            iCmdIds.push_back(iCurrentCmdId);
        }
        break;

        case STATE_CLEANUPANDCOMPLETE:
        {
            PVPATB_TEST_IS_TRUE(PVPlayerFactory::DeletePlayer(iPlayer));
            iPlayer = NULL;

            delete iDataSource;
            iDataSource = NULL;

            delete iDataSinkVideo;
            iDataSinkVideo = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeVideo);
            iIONodeVideo = NULL;

            iMioFactory->DestroyVideoOutput(iMIOFileOutVideo);
            iMIOFileOutVideo = NULL;

            delete iDataSinkAudio;
            iDataSinkAudio = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeAudio);
            iIONodeAudio = NULL;

            iMioFactory->DestroyAudioOutput(iMIOFileOutAudio);
            iMIOFileOutAudio = NULL;

            delete iDataSinkText;
            iDataSinkText = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeText);
            iIONodeText = NULL;

            iMioFactory->DestroyTextOutput(iMIOFileOutText);
            iMIOFileOutText = NULL;

            iObserver->TestCompleted(*iTestCase);
        }
        break;

        default:
            break;

    }
}


void pvplayer_async_test_looping::CommandCompleted(const PVCmdResponse& aResponse)
{
    // Assumes the engine commands are completed in the order they are requested
    if (aResponse.GetCmdId() != iCmdIds[0])
    {
        // Wrong command ID.
        PVPATB_TEST_IS_TRUE(false);
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
        return;
    }

    if (aResponse.GetContext() != NULL)
    {
        if (aResponse.GetContext() == (OsclAny*)&iContextObject)
        {
            if (iContextObject != iContextObjectRefValue)
            {
                // Context data value was corrupted
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                return;
            }
        }
        else
        {
            // Context data pointer was corrupted
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
            return;
        }
    }

    switch (iState)
    {
        case STATE_STARTPLAYBACK:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iCmdIds.erase(iCmdIds.begin());
                if (iCmdIds.empty())
                {
                    // All requested commands have completed
                    iState = STATE_SETPLAYBACKRANGE1;
                    RunIfNotReady();
                }
            }
            else
            {
                // One of queued commands failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_SETPLAYBACKRANGE1:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iCmdIds.erase(iCmdIds.begin());
                if (iCmdIds.empty())
                {
                    // All requested commands have completed
                    iState = STATE_ENDTIMENOTREACHED1;
                    RunIfNotReady(18000000);
                }
            }
            else
            {
                // One of queued commands failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_SETPLAYBACKRANGE2:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iCmdIds.erase(iCmdIds.begin());
                if (iCmdIds.empty())
                {
                    // All requested commands have completed
                    iState = STATE_RESUME;
                    RunIfNotReady();
                }
            }
            else
            {
                // One of queued commands failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_RESUME:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iCmdIds.erase(iCmdIds.begin());
                if (iCmdIds.empty())
                {
                    // All requested commands have completed
                    iState = STATE_ENDTIMENOTREACHED2;
                    RunIfNotReady(20000000);
                }
            }
            else
            {
                // One of queued commands failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_SHUTDOWN:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iCmdIds.erase(iCmdIds.begin());
                if (iCmdIds.empty())
                {
                    // All requested commands have completed
                    PVPATB_TEST_IS_TRUE(true);
                    iState = STATE_CLEANUPANDCOMPLETE;
                    RunIfNotReady();
                }
            }
            else
            {
                // One of queued commands failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        default:
        {
            // Testing error if this is reached
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;
    }
}


void pvplayer_async_test_looping::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    switch (aEvent.GetEventType())
    {
        case PVMFErrResourceConfiguration:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrResource:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrCorrupt:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrProcessing:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        default:
            // Unknown error and just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;
    }
}


void pvplayer_async_test_looping::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingStart)
    {
        // do nothing
    }
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingComplete)
    {
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
    }
    // Check for stop time reached event
    if (aEvent.GetEventType() == PVMFInfoEndOfData)
    {
        PVInterface* iface = (PVInterface*)(aEvent.GetEventExtensionInterface());
        if (iface == NULL)
        {
            return;
        }
        PVUuid infomsguuid = PVMFErrorInfoMessageInterfaceUUID;
        PVMFErrorInfoMessageInterface* infomsgiface = NULL;
        if (iface->queryInterface(infomsguuid, (PVInterface*&)infomsgiface) == true)
        {
            int32 infocode;
            PVUuid infouuid;
            infomsgiface->GetCodeUUID(infocode, infouuid);
            if ((infouuid == PVPlayerErrorInfoEventTypesUUID) && (infocode == PVPlayerInfoEndTimeReached))
            {
                if (iState == STATE_ENDTIMENOTREACHED1)
                {
                    iState = STATE_SETPLAYBACKRANGE2;
                }
                else if (iState == STATE_ENDTIMENOTREACHED2)
                {
                    iState = STATE_SHUTDOWN;
                }
                else
                {
                    // Wrong test case state to get this event
                    PVPATB_TEST_IS_TRUE(false);
                    iState = STATE_CLEANUPANDCOMPLETE;
                }
                Cancel();
                RunIfNotReady();
            }
        }
    }
}


//
// pvplayer_async_test_waitforeos section
//
void pvplayer_async_test_waitforeos::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_waitforeos::Run()
{
    int error = 0;

    switch (iState)
    {
        case STATE_CREATE:
        {
            iPlayer = NULL;

            OSCL_TRY(error, iPlayer = PVPlayerFactory::CreatePlayer(this, this, this));
            if (error)
            {
                PVPATB_TEST_IS_TRUE(false);
                iObserver->TestCompleted(*iTestCase);
            }
            else
            {
                iState = STATE_ADDDATASOURCE;
                RunIfNotReady();
            }
        }
        break;

        case STATE_ADDDATASOURCE:
        {
            iDataSource = new PVPlayerDataSourceURL;
            oscl_UTF8ToUnicode(iFileName, oscl_strlen(iFileName), output, 512);
            wFileName.set(output, oscl_strlen(output));
            iDataSource->SetDataSourceURL(wFileName);
            iDataSource->SetDataSourceFormatType(iFileType);
            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }

        break;

        case STATE_INIT:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Init((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_VIDEO:
        {
            OSCL_wHeapString<OsclMemAllocator> SinkFileName;
            SinkFileName = OUTPUTNAME_PREPEND_WSTRING;
            SinkFileName += _STRLIT_WCHAR("test_player_waitforeos_");
            OSCL_wHeapString<OsclMemAllocator> inputfilename;
            RetrieveFilename(wFileName.get_str(), inputfilename);
            SinkFileName += inputfilename;
            SinkFileName += _STRLIT_WCHAR("_video.dat");

            iMIOFileOutVideo = iMioFactory->CreateVideoOutput((OsclAny*) & SinkFileName, MEDIATYPE_VIDEO, iCompressedVideo);
            iIONodeVideo = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutVideo);
            iDataSinkVideo = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkVideo)->SetDataSinkNode(iIONodeVideo);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_AUDIO:
        {
            OSCL_wHeapString<OsclMemAllocator> SinkFileName;
            SinkFileName = OUTPUTNAME_PREPEND_WSTRING;
            SinkFileName += _STRLIT_WCHAR("test_player_waitforeos_");
            OSCL_wHeapString<OsclMemAllocator> inputfilename;
            RetrieveFilename(wFileName.get_str(), inputfilename);
            SinkFileName += inputfilename;
            SinkFileName += _STRLIT_WCHAR("_audio.dat");

            iMIOFileOutAudio = iMioFactory->CreateAudioOutput((OsclAny*) & SinkFileName, MEDIATYPE_AUDIO, iCompressedAudio);
            iIONodeAudio = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutAudio);
            iDataSinkAudio = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkAudio)->SetDataSinkNode(iIONodeAudio);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_TEXT:
        {
            OSCL_wHeapString<OsclMemAllocator> SinkFileName;
            SinkFileName = OUTPUTNAME_PREPEND_WSTRING;
            SinkFileName += _STRLIT_WCHAR("test_player_waitforeos_");
            OSCL_wHeapString<OsclMemAllocator> inputfilename;
            RetrieveFilename(wFileName.get_str(), inputfilename);
            SinkFileName += inputfilename;
            SinkFileName += _STRLIT_WCHAR("_text.dat");

            iMIOFileOutText = iMioFactory->CreateTextOutput((OsclAny*) & SinkFileName, MEDIATYPE_TEXT);
            iIONodeText = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutText);
            iDataSinkText = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkText)->SetDataSinkNode(iIONodeText);


            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkText, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_PREPARE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Prepare((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_START:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Start((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_EOSNOTREACHED:
        {
            // EOS event not received so initiate stop
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_STOP;
            RunIfNotReady();
        }
        break;

        case STATE_STOP:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Stop((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_VIDEO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_AUDIO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_TEXT:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkText, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_RESET:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Reset((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASOURCE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_CLEANUPANDCOMPLETE:
        {
            PVPATB_TEST_IS_TRUE(PVPlayerFactory::DeletePlayer(iPlayer));
            iPlayer = NULL;

            delete iDataSource;
            iDataSource = NULL;

            delete iDataSinkVideo;
            iDataSinkVideo = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeVideo);
            iIONodeVideo = NULL;

            iMioFactory->DestroyVideoOutput(iMIOFileOutVideo);
            iMIOFileOutVideo = NULL;

            delete iDataSinkAudio;
            iDataSinkAudio = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeAudio);
            iIONodeAudio = NULL;

            iMioFactory->DestroyAudioOutput(iMIOFileOutAudio);
            iMIOFileOutAudio = NULL;

            delete iDataSinkText;
            iDataSinkText = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeText);
            iIONodeText = NULL;

            iMioFactory->DestroyTextOutput(iMIOFileOutText);
            iMIOFileOutText = NULL;

            iObserver->TestCompleted(*iTestCase);
        }
        break;

        default:
            break;

    }
}


void pvplayer_async_test_waitforeos::CommandCompleted(const PVCmdResponse& aResponse)
{
    if (aResponse.GetCmdId() != iCurrentCmdId)
    {
        // Wrong command ID.
        PVPATB_TEST_IS_TRUE(false);
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
        return;
    }

    if (aResponse.GetContext() != NULL)
    {
        if (aResponse.GetContext() == (OsclAny*)&iContextObject)
        {
            if (iContextObject != iContextObjectRefValue)
            {
                // Context data value was corrupted
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                return;
            }
        }
        else
        {
            // Context data pointer was corrupted
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
            return;
        }
    }

    switch (iState)
    {
        case STATE_ADDDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_INIT;
                RunIfNotReady();
            }
            else
            {
                // AddDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_INIT:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                // Init failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_TEXT;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_TEXT:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_PREPARE;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_PREPARE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_START;
                RunIfNotReady();
            }
            else
            {
                // Prepare failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_START:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_EOSNOTREACHED;
                RunIfNotReady(500000000);
            }
            else
            {
                // Start failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_STOP:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                // Stop failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_TEXT;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_TEXT:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_RESET;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_RESET:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASOURCE;
                RunIfNotReady();
            }
            else
            {
                // Reset failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                PVPATB_TEST_IS_TRUE(true);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        default:
        {
            // Testing error if this is reached
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;
    }
}


void pvplayer_async_test_waitforeos::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    switch (aEvent.GetEventType())
    {
        case PVMFErrResourceConfiguration:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrResource:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrCorrupt:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrProcessing:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        default:
            // Unknown error and just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;
    }
}


void pvplayer_async_test_waitforeos::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingStart)
    {
        // do nothing
    }
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingComplete)
    {
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
    }
    // Check for stop time reached event
    if (aEvent.GetEventType() == PVMFInfoEndOfData)
    {
        PVInterface* iface = (PVInterface*)(aEvent.GetEventExtensionInterface());
        if (iface == NULL)
        {
            return;
        }
        PVUuid infomsguuid = PVMFErrorInfoMessageInterfaceUUID;
        PVMFErrorInfoMessageInterface* infomsgiface = NULL;
        if (iface->queryInterface(infomsguuid, (PVInterface*&)infomsgiface) == true)
        {
            int32 infocode;
            PVUuid infouuid;
            infomsgiface->GetCodeUUID(infocode, infouuid);
            if ((infouuid == PVPlayerErrorInfoEventTypesUUID) && (infocode == PVPlayerInfoEndOfClipReached))
            {
                iState = STATE_STOP;
                Cancel();
                RunIfNotReady();
            }
        }
    }
}



//
// pvplayer_async_test_multipauseresume section
//
void pvplayer_async_test_multipauseresume::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_multipauseresume::Run()
{
    int error = 0;

    switch (iState)
    {
        case STATE_CREATE:
        {
            iPlayer = NULL;

            OSCL_TRY(error, iPlayer = PVPlayerFactory::CreatePlayer(this, this, this));
            if (error)
            {
                PVPATB_TEST_IS_TRUE(false);
                iObserver->TestCompleted(*iTestCase);
            }
            else
            {
                iState = STATE_STARTPLAYBACK;
                RunIfNotReady();
            }
        }
        break;

        case STATE_STARTPLAYBACK:
        {
            OSCL_wHeapString<OsclMemAllocator> SinkFileName;
            OSCL_wHeapString<OsclMemAllocator> inputfilename;

            // Add the data source
            iDataSource = new PVPlayerDataSourceURL;
            oscl_UTF8ToUnicode(iFileName, oscl_strlen(iFileName), output, 512);
            wFileName.set(output, oscl_strlen(output));
            RetrieveFilename(wFileName.get_str(), inputfilename);
            iDataSource->SetDataSourceURL(wFileName);
            iDataSource->SetDataSourceFormatType(iFileType);
            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); break);
            iCmdIds.push_back(iCurrentCmdId);
            // Initialize
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Init((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); break);
            iCmdIds.push_back(iCurrentCmdId);

            // Add video data sink
            SinkFileName = OUTPUTNAME_PREPEND_WSTRING;
            SinkFileName += _STRLIT_WCHAR("test_player_multipauseresume_");
            SinkFileName += inputfilename;
            SinkFileName += _STRLIT_WCHAR("_video.dat");

            iMIOFileOutVideo = iMioFactory->CreateVideoOutput((OsclAny*) & SinkFileName, MEDIATYPE_VIDEO, iCompressedVideo);
            iIONodeVideo = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutVideo);
            iDataSinkVideo = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkVideo)->SetDataSinkNode(iIONodeVideo);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); break);
            iCmdIds.push_back(iCurrentCmdId);

            // Add audio data sink
            SinkFileName = OUTPUTNAME_PREPEND_WSTRING;
            SinkFileName += _STRLIT_WCHAR("test_player_multipauseresume_");
            SinkFileName += inputfilename;
            SinkFileName += _STRLIT_WCHAR("_audio.dat");

            iMIOFileOutAudio = iMioFactory->CreateAudioOutput((OsclAny*) & SinkFileName, MEDIATYPE_AUDIO, iCompressedAudio);
            iIONodeAudio = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutAudio);
            iDataSinkAudio = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkAudio)->SetDataSinkNode(iIONodeAudio);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); break);
            iCmdIds.push_back(iCurrentCmdId);

            // Add text data sink
            SinkFileName = OUTPUTNAME_PREPEND_WSTRING;
            SinkFileName += _STRLIT_WCHAR("test_player_multipauseresume_");
            SinkFileName += inputfilename;
            SinkFileName += _STRLIT_WCHAR("_text.dat");

            iMIOFileOutText = iMioFactory->CreateTextOutput((OsclAny*) & SinkFileName, MEDIATYPE_TEXT);
            iIONodeText = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutText);
            iDataSinkText = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkText)->SetDataSinkNode(iIONodeText);


            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkText, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); break);
            iCmdIds.push_back(iCurrentCmdId);

            // Prepare
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Prepare((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); break);
            iCmdIds.push_back(iCurrentCmdId);
            // Start
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Start((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); break);
            iCmdIds.push_back(iCurrentCmdId);
        }
        break;

        case STATE_PAUSERESUME1:
        {
            // Pause
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Pause((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); break);
            iCmdIds.push_back(iCurrentCmdId);
            // Resume
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Resume((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); break);
            iCmdIds.push_back(iCurrentCmdId);
            // Pause
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Pause((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); break);
            iCmdIds.push_back(iCurrentCmdId);
            // Resume
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Resume((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); break);
            iCmdIds.push_back(iCurrentCmdId);
            // Pause
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Pause((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); break);
            iCmdIds.push_back(iCurrentCmdId);
            // Resume
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Resume((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); break);
            iCmdIds.push_back(iCurrentCmdId);
            // Pause
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Pause((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); break);
            iCmdIds.push_back(iCurrentCmdId);
            // Resume
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Resume((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); break);
            iCmdIds.push_back(iCurrentCmdId);
            // Pause
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Pause((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); break);
            iCmdIds.push_back(iCurrentCmdId);
            // Resume
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Resume((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); break);
            iCmdIds.push_back(iCurrentCmdId);
        }
        break;

        case STATE_PAUSE:
        {
            // Pause
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Pause((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            iCmdIds.push_back(iCurrentCmdId);
        }
        break;

        case STATE_RESUME:
        {
            // Resume
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Resume((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            iCmdIds.push_back(iCurrentCmdId);
        }
        break;

        case STATE_PAUSERESUME2:
        {
            // Pause
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Pause((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); break);
            iCmdIds.push_back(iCurrentCmdId);
            // Resume
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Resume((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); break);
            iCmdIds.push_back(iCurrentCmdId);
            // Pause
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Pause((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); break);
            iCmdIds.push_back(iCurrentCmdId);
            // Resume
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Resume((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); break);
            iCmdIds.push_back(iCurrentCmdId);
            // Pause
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Pause((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); break);
            iCmdIds.push_back(iCurrentCmdId);
            // Resume
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Resume((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); break);
            iCmdIds.push_back(iCurrentCmdId);
        }
        break;

        case STATE_SHUTDOWN:
        {
            // Stop
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Stop((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); break);
            iCmdIds.push_back(iCurrentCmdId);
            // Remove video data sink
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); break);
            iCmdIds.push_back(iCurrentCmdId);
            // Remove audio data sink
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); break);
            iCmdIds.push_back(iCurrentCmdId);
            // Remove text data sink
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkText, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); break);
            iCmdIds.push_back(iCurrentCmdId);
            // Reset
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Reset((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); break);
            iCmdIds.push_back(iCurrentCmdId);
            // Remove data source
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); break);
            iCmdIds.push_back(iCurrentCmdId);
        }
        break;

        case STATE_CLEANUPANDCOMPLETE:
        {
            PVPATB_TEST_IS_TRUE(PVPlayerFactory::DeletePlayer(iPlayer));
            iPlayer = NULL;

            delete iDataSource;
            iDataSource = NULL;

            delete iDataSinkVideo;
            iDataSinkVideo = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeVideo);
            iIONodeVideo = NULL;

            iMioFactory->DestroyVideoOutput(iMIOFileOutVideo);
            iMIOFileOutVideo = NULL;

            delete iDataSinkAudio;
            iDataSinkAudio = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeAudio);
            iIONodeAudio = NULL;

            iMioFactory->DestroyAudioOutput(iMIOFileOutAudio);
            iMIOFileOutAudio = NULL;

            delete iDataSinkText;
            iDataSinkText = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeText);
            iIONodeText = NULL;

            iMioFactory->DestroyTextOutput(iMIOFileOutText);
            iMIOFileOutText = NULL;

            iObserver->TestCompleted(*iTestCase);
        }
        break;

        default:
            break;

    }
}


void pvplayer_async_test_multipauseresume::CommandCompleted(const PVCmdResponse& aResponse)
{
    // Assumes the engine commands are completed in the order they are requested
    if (aResponse.GetCmdId() != iCmdIds[0])
    {
        // Wrong command ID.
        PVPATB_TEST_IS_TRUE(false);
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
        return;
    }

    if (aResponse.GetContext() != NULL)
    {
        if (aResponse.GetContext() == (OsclAny*)&iContextObject)
        {
            if (iContextObject != iContextObjectRefValue)
            {
                // Context data value was corrupted
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                return;
            }
        }
        else
        {
            // Context data pointer was corrupted
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
            return;
        }
    }

    switch (iState)
    {
        case STATE_STARTPLAYBACK:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iCmdIds.erase(iCmdIds.begin());
                if (iCmdIds.empty())
                {
                    // All requested commands have completed
                    iState = STATE_PAUSERESUME1;
                    RunIfNotReady(5000000);
                }
            }
            else
            {
                // One of queued commands failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_PAUSERESUME1:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iCmdIds.erase(iCmdIds.begin());
                if (iCmdIds.empty())
                {
                    // All requested commands have completed
                    iState = STATE_PAUSE;
                    RunIfNotReady(2000000);
                }
            }
            else
            {
                // One of queued commands failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_PAUSE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iCmdIds.erase(iCmdIds.begin());
                if (iCmdIds.empty())
                {
                    // All requested commands have completed
                    iState = STATE_RESUME;
                    RunIfNotReady(5000000);
                }
            }
            else
            {
                // One of queued commands failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_RESUME:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iCmdIds.erase(iCmdIds.begin());
                if (iCmdIds.empty())
                {
                    // All requested commands have completed
                    iState = STATE_PAUSERESUME2;
                    RunIfNotReady(3000000);
                }
            }
            else
            {
                // One of queued commands failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_PAUSERESUME2:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iCmdIds.erase(iCmdIds.begin());
                if (iCmdIds.empty())
                {
                    // All requested commands have completed
                    iState = STATE_SHUTDOWN;
                    RunIfNotReady(5000000);
                }
            }
            else
            {
                // One of queued commands failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_SHUTDOWN:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iCmdIds.erase(iCmdIds.begin());
                if (iCmdIds.empty())
                {
                    // All requested commands have completed
                    PVPATB_TEST_IS_TRUE(true);
                    iState = STATE_CLEANUPANDCOMPLETE;
                    RunIfNotReady();
                }
            }
            else
            {
                // One of queued commands failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        default:
        {
            // Testing error if this is reached
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;
    }
}


void pvplayer_async_test_multipauseresume::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    switch (aEvent.GetEventType())
    {
        case PVMFErrResourceConfiguration:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrResource:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrCorrupt:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrProcessing:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        default:
            // Unknown error and just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;
    }

    // Wait for engine to handle the error
    Cancel();
}


void pvplayer_async_test_multipauseresume::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingStart)
    {
        // do nothing
    }
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingComplete)
    {
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
    }
}


//
// pvplayer_async_test_multireposition section
//
void pvplayer_async_test_multireposition::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_multireposition::Run()
{
    int error = 0;

    switch (iState)
    {
        case STATE_CREATE:
        {
            iPlayer = NULL;

            OSCL_TRY(error, iPlayer = PVPlayerFactory::CreatePlayer(this, this, this));
            if (error)
            {
                PVPATB_TEST_IS_TRUE(false);
                iObserver->TestCompleted(*iTestCase);
            }
            else
            {
                iState = STATE_STARTPLAYBACK;
                RunIfNotReady();
            }
        }
        break;

        case STATE_STARTPLAYBACK:
        {
            OSCL_wHeapString<OsclMemAllocator> SinkFileName;
            OSCL_wHeapString<OsclMemAllocator> inputfilename;

            // Add the data source
            iDataSource = new PVPlayerDataSourceURL;
            oscl_UTF8ToUnicode(iFileName, oscl_strlen(iFileName), output, 512);
            wFileName.set(output, oscl_strlen(output));
            RetrieveFilename(wFileName.get_str(), inputfilename);
            iDataSource->SetDataSourceURL(wFileName);
            iDataSource->SetDataSourceFormatType(iFileType);
            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); break);
            iCmdIds.push_back(iCurrentCmdId);

            // Open file to log reposition times
            iFS.Connect();

            SinkFileName = OUTPUTNAME_PREPEND_WSTRING;
            SinkFileName += _STRLIT_WCHAR("test_player_multireposition_");
            SinkFileName += inputfilename;
            SinkFileName += _STRLIT_WCHAR("_repositiontime.dat");

            if (iTimeLogFile.Open(SinkFileName.get_str(), Oscl_File::MODE_READWRITE | Oscl_File::MODE_TEXT, iFS))
            {
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                break;
            }
            // Initialize
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Init((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); break);
            iCmdIds.push_back(iCurrentCmdId);

            // Add video data sink
            SinkFileName = OUTPUTNAME_PREPEND_WSTRING;
            SinkFileName += _STRLIT_WCHAR("test_player_multireposition_");
            SinkFileName += inputfilename;
            SinkFileName += _STRLIT_WCHAR("_video.dat");

            iMIOFileOutVideo = iMioFactory->CreateVideoOutput((OsclAny*) & SinkFileName, MEDIATYPE_VIDEO, iCompressedVideo);
            iIONodeVideo = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutVideo);
            iDataSinkVideo = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkVideo)->SetDataSinkNode(iIONodeVideo);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); break);
            iCmdIds.push_back(iCurrentCmdId);

            // Add audio data sink
            SinkFileName = OUTPUTNAME_PREPEND_WSTRING;
            SinkFileName += _STRLIT_WCHAR("test_player_multireposition_");
            SinkFileName += inputfilename;
            SinkFileName += _STRLIT_WCHAR("_audio.dat");

            iMIOFileOutAudio = iMioFactory->CreateAudioOutput((OsclAny*) & SinkFileName, MEDIATYPE_AUDIO, iCompressedAudio);
            iIONodeAudio = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutAudio);
            iDataSinkAudio = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkAudio)->SetDataSinkNode(iIONodeAudio);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); break);
            iCmdIds.push_back(iCurrentCmdId);

            // Add text data sink
            SinkFileName = OUTPUTNAME_PREPEND_WSTRING;
            SinkFileName += _STRLIT_WCHAR("test_player_multireposition_");
            SinkFileName += inputfilename;
            SinkFileName += _STRLIT_WCHAR("_text.dat");

            iMIOFileOutText = iMioFactory->CreateTextOutput((OsclAny*) & SinkFileName, MEDIATYPE_TEXT);
            iIONodeText = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutText);
            iDataSinkText = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkText)->SetDataSinkNode(iIONodeText);


            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkText, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); break);
            iCmdIds.push_back(iCurrentCmdId);

            // Prepare
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Prepare((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); break);
            iCmdIds.push_back(iCurrentCmdId);

            // Start
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Start((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); break);
            iCmdIds.push_back(iCurrentCmdId);
        }
        break;

        case STATE_SETPLAYBACKRANGE1:
        {
            // Reposition
            PVPPlaybackPosition start, end;
            start.iIndeterminate = false;
            start.iPosUnit = PVPPBPOSUNIT_MILLISEC;
            start.iMode = PVPPBPOS_MODE_NOW;
            start.iPosValue.millisec_value = 20000;
            end.iIndeterminate = true;
            OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRange(start, end, false, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            iCmdIds.push_back(iCurrentCmdId);

            char outstr[128];
            oscl_snprintf(outstr, 128, "SetPlaybackRange() called; Start time 20 sec; Time %d\n", OsclTickCount::TicksToMsec(OsclTickCount::TickCount()));
            iTimeLogFile.Write(outstr, sizeof(char), oscl_strlen(outstr));
            iTimeLogFile.Flush();
        }
        break;

        case STATE_SETPLAYBACKRANGE2:
        {
            // Reposition
            PVPPlaybackPosition start, end;
            start.iIndeterminate = false;
            start.iPosUnit = PVPPBPOSUNIT_MILLISEC;
            start.iMode = PVPPBPOS_MODE_NOW;
            start.iPosValue.millisec_value = 5000;
            end.iIndeterminate = true;
            OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRange(start, end, false, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            iCmdIds.push_back(iCurrentCmdId);

            char outstr[128];
            oscl_snprintf(outstr, 128, "SetPlaybackRange() called; Start time 5 sec; Time %d\n", OsclTickCount::TicksToMsec(OsclTickCount::TickCount()));
            iTimeLogFile.Write(outstr, sizeof(char), oscl_strlen(outstr));
            iTimeLogFile.Flush();
        }
        break;

        case STATE_SETPLAYBACKRANGE3:
        {
            // Reposition
            PVPPlaybackPosition start, end;
            start.iIndeterminate = false;
            start.iPosUnit = PVPPBPOSUNIT_MILLISEC;
            start.iMode = PVPPBPOS_MODE_NOW;
            start.iPosValue.millisec_value = 25000;
            end.iIndeterminate = true;
            OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRange(start, end, false, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            iCmdIds.push_back(iCurrentCmdId);

            char outstr[128];
            oscl_snprintf(outstr, 128, "SetPlaybackRange() called; Start time 25 sec; Time %d\n", OsclTickCount::TicksToMsec(OsclTickCount::TickCount()));
            iTimeLogFile.Write(outstr, sizeof(char), oscl_strlen(outstr));
            iTimeLogFile.Flush();
        }
        break;

        case STATE_SETPLAYBACKRANGE4:
        {
            // Reposition
            PVPPlaybackPosition start, end;
            start.iIndeterminate = false;
            start.iPosUnit = PVPPBPOSUNIT_MILLISEC;
            start.iMode = PVPPBPOS_MODE_NOW;
            start.iPosValue.millisec_value = 10000;
            end.iIndeterminate = true;
            OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRange(start, end, false, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            iCmdIds.push_back(iCurrentCmdId);

            char outstr[128];
            oscl_snprintf(outstr, 128, "SetPlaybackRange() called; Start time 10 sec; Time %d\n", OsclTickCount::TicksToMsec(OsclTickCount::TickCount()));
            iTimeLogFile.Write(outstr, sizeof(char), oscl_strlen(outstr));
            iTimeLogFile.Flush();
        }
        break;

        case STATE_SETPLAYBACKRANGE5:
        {
            // Reposition
            PVPPlaybackPosition start, end;
            start.iIndeterminate = false;
            start.iPosUnit = PVPPBPOSUNIT_MILLISEC;
            start.iMode = PVPPBPOS_MODE_NOW;
            start.iPosValue.millisec_value = 20000;
            end.iIndeterminate = true;
            OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRange(start, end, false, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            iCmdIds.push_back(iCurrentCmdId);

            char outstr[128];
            oscl_snprintf(outstr, 128, "SetPlaybackRange() called; Start time 20 sec; Time %d\n", OsclTickCount::TicksToMsec(OsclTickCount::TickCount()));
            iTimeLogFile.Write(outstr, sizeof(char), oscl_strlen(outstr));
            iTimeLogFile.Flush();
        }
        break;

        case STATE_SETPLAYBACKRANGE6:
        {
            // Reposition
            PVPPlaybackPosition start, end;
            start.iIndeterminate = false;
            start.iPosUnit = PVPPBPOSUNIT_MILLISEC;
            start.iMode = PVPPBPOS_MODE_NOW;
            start.iPosValue.millisec_value = 0;
            end.iIndeterminate = true;
            OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRange(start, end, false, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            iCmdIds.push_back(iCurrentCmdId);

            char outstr[128];
            oscl_snprintf(outstr, 128, "SetPlaybackRange() called; Start time 0 sec; Time %d\n", OsclTickCount::TicksToMsec(OsclTickCount::TickCount()));
            iTimeLogFile.Write(outstr, sizeof(char), oscl_strlen(outstr));
            iTimeLogFile.Flush();
        }
        break;

        case STATE_SHUTDOWN:
        {
            // Close the reposition log time file
            iTimeLogFile.Close();
            iFS.Close();
            // Stop
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Stop((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); break);
            iCmdIds.push_back(iCurrentCmdId);
            // Remove video data sink
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); break);
            iCmdIds.push_back(iCurrentCmdId);
            // Remove audio data sink
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); break);
            iCmdIds.push_back(iCurrentCmdId);
            // Remove text data sink
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkText, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); break);
            iCmdIds.push_back(iCurrentCmdId);
            // Reset
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Reset((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); break);
            iCmdIds.push_back(iCurrentCmdId);
            // Remove data source
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); break);
            iCmdIds.push_back(iCurrentCmdId);
        }
        break;

        case STATE_CLEANUPANDCOMPLETE:
        {
            PVPATB_TEST_IS_TRUE(PVPlayerFactory::DeletePlayer(iPlayer));
            iPlayer = NULL;

            delete iDataSource;
            iDataSource = NULL;

            delete iDataSinkVideo;
            iDataSinkVideo = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeVideo);
            iIONodeVideo = NULL;

            iMioFactory->DestroyVideoOutput(iMIOFileOutVideo);
            iMIOFileOutVideo = NULL;

            delete iDataSinkAudio;
            iDataSinkAudio = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeAudio);
            iIONodeAudio = NULL;

            iMioFactory->DestroyAudioOutput(iMIOFileOutAudio);
            iMIOFileOutAudio = NULL;

            delete iDataSinkText;
            iDataSinkText = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeText);
            iIONodeText = NULL;

            iMioFactory->DestroyTextOutput(iMIOFileOutText);
            iMIOFileOutText = NULL;

            iObserver->TestCompleted(*iTestCase);
        }
        break;

        default:
            break;

    }
}


void pvplayer_async_test_multireposition::CommandCompleted(const PVCmdResponse& aResponse)
{
    // Assumes the engine commands are completed in the order they are requested
    if (aResponse.GetCmdId() != iCmdIds[0])
    {
        // Wrong command ID.
        PVPATB_TEST_IS_TRUE(false);
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
        return;
    }

    if (aResponse.GetContext() != NULL)
    {
        if (aResponse.GetContext() == (OsclAny*)&iContextObject)
        {
            if (iContextObject != iContextObjectRefValue)
            {
                // Context data value was corrupted
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                return;
            }
        }
        else
        {
            // Context data pointer was corrupted
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
            return;
        }
    }

    switch (iState)
    {
        case STATE_STARTPLAYBACK:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iCmdIds.erase(iCmdIds.begin());
                if (iCmdIds.empty())
                {
                    // All requested commands have completed
                    iState = STATE_SETPLAYBACKRANGE1;
                    RunIfNotReady(5000000);
                }
            }
            else
            {
                // One of queued commands failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_SETPLAYBACKRANGE1:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iCmdIds.erase(iCmdIds.begin());
                if (iCmdIds.empty())
                {
                    // All requested commands have completed
                    iState = STATE_SETPLAYBACKRANGE2;
                    RunIfNotReady(5000000);

                    char outstr[128];
                    oscl_snprintf(outstr, 128, "SetPlaybackRange() Complete; Time %d\n", OsclTickCount::TicksToMsec(OsclTickCount::TickCount()));
                    iTimeLogFile.Write(outstr, sizeof(char), oscl_strlen(outstr));
                    iTimeLogFile.Flush();
                }
            }
            else
            {
                // One of queued commands failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_SETPLAYBACKRANGE2:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iCmdIds.erase(iCmdIds.begin());
                if (iCmdIds.empty())
                {
                    // All requested commands have completed
                    iState = STATE_SETPLAYBACKRANGE3;
                    RunIfNotReady(5000000);

                    char outstr[128];
                    oscl_snprintf(outstr, 128, "SetPlaybackRange() Complete; Time %d\n", OsclTickCount::TicksToMsec(OsclTickCount::TickCount()));
                    iTimeLogFile.Write(outstr, sizeof(char), oscl_strlen(outstr));
                    iTimeLogFile.Flush();
                }
            }
            else
            {
                // One of queued commands failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_SETPLAYBACKRANGE3:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iCmdIds.erase(iCmdIds.begin());
                if (iCmdIds.empty())
                {
                    // All requested commands have completed
                    iState = STATE_SETPLAYBACKRANGE4;
                    RunIfNotReady(5000000);

                    char outstr[128];
                    oscl_snprintf(outstr, 128, "SetPlaybackRange() Complete; Time %d\n", OsclTickCount::TicksToMsec(OsclTickCount::TickCount()));
                    iTimeLogFile.Write(outstr, sizeof(char), oscl_strlen(outstr));
                    iTimeLogFile.Flush();
                }
            }
            else
            {
                // One of queued commands failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_SETPLAYBACKRANGE4:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iCmdIds.erase(iCmdIds.begin());
                if (iCmdIds.empty())
                {
                    // All requested commands have completed
                    iState = STATE_SETPLAYBACKRANGE5;
                    RunIfNotReady(5000000);

                    char outstr[128];
                    oscl_snprintf(outstr, 128, "SetPlaybackRange() Complete; Time %d\n", OsclTickCount::TicksToMsec(OsclTickCount::TickCount()));
                    iTimeLogFile.Write(outstr, sizeof(char), oscl_strlen(outstr));
                    iTimeLogFile.Flush();
                }
            }
            else
            {
                // One of queued commands failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_SETPLAYBACKRANGE5:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iCmdIds.erase(iCmdIds.begin());
                if (iCmdIds.empty())
                {
                    // All requested commands have completed
                    iState = STATE_SETPLAYBACKRANGE6;
                    RunIfNotReady();

                    char outstr[128];
                    oscl_snprintf(outstr, 128, "SetPlaybackRange() Complete; Time %d\n", OsclTickCount::TicksToMsec(OsclTickCount::TickCount()));
                    iTimeLogFile.Write(outstr, sizeof(char), oscl_strlen(outstr));
                    iTimeLogFile.Flush();
                }
            }
            else
            {
                // One of queued commands failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_SETPLAYBACKRANGE6:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iCmdIds.erase(iCmdIds.begin());
                if (iCmdIds.empty())
                {
                    // All requested commands have completed
                    iState = STATE_SHUTDOWN;
                    RunIfNotReady(5000000);
                }
            }
            else
            {
                // One of queued commands failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_SHUTDOWN:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iCmdIds.erase(iCmdIds.begin());
                if (iCmdIds.empty())
                {
                    // All requested commands have completed
                    PVPATB_TEST_IS_TRUE(true);
                    iState = STATE_CLEANUPANDCOMPLETE;
                    RunIfNotReady();
                }
            }
            else
            {
                // One of queued commands failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        default:
        {
            // Testing error if this is reached
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;
    }
}


void pvplayer_async_test_multireposition::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    switch (aEvent.GetEventType())
    {
        case PVMFErrResourceConfiguration:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrResource:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrCorrupt:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrProcessing:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        default:
            // Unknown error and just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;
    }
}


void pvplayer_async_test_multireposition::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingStart)
    {
        // do nothing
    }
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingComplete)
    {
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
    }
    // Check reposition time here
    if (aEvent.GetEventType() == PVMFInfoPositionStatus)
    {
        PVInterface* iface = (PVInterface*)(aEvent.GetEventExtensionInterface());
        if (iface == NULL)
        {
            return;
        }
        PVUuid infomsguuid = PVMFErrorInfoMessageInterfaceUUID;
        PVMFErrorInfoMessageInterface* infomsgiface = NULL;
        if (iface->queryInterface(infomsguuid, (PVInterface*&)infomsgiface) == true)
        {
            int32 infocode;
            PVUuid infouuid;
            infomsgiface->GetCodeUUID(infocode, infouuid);
            if ((infouuid == PVPlayerErrorInfoEventTypesUUID) && (infocode == PVPlayerInfoPlaybackFromBeginTime))
            {
                char outstr[128];
                oscl_snprintf(outstr, 128, "Begin playback position reached; Time %d\n", OsclTickCount::TicksToMsec(OsclTickCount::TickCount()));
                iTimeLogFile.Write(outstr, sizeof(char), oscl_strlen(outstr));
                iTimeLogFile.Flush();
            }
        }
    }
}


//
// pvplayer_async_test_capconfigiftest section
//
void pvplayer_async_test_capconfigiftest::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_capconfigiftest::Run()
{
    int error = 0;

    switch (iState)
    {
        case STATE_CREATE:
        {
            iPlayer = NULL;

            OSCL_TRY(error, iPlayer = PVPlayerFactory::CreatePlayer(this, this, this));
            if (error)
            {
                PVPATB_TEST_IS_TRUE(false);
                iObserver->TestCompleted(*iTestCase);
            }
            else
            {
                iState = STATE_QUERYINTERFACE;
                RunIfNotReady();
            }
        }
        break;

        case STATE_QUERYINTERFACE:
        {
            PVUuid capconfigifuuid = PVMI_CAPABILITY_AND_CONFIG_PVUUID;
            OSCL_TRY(error, iCurrentCmdId = iPlayer->QueryInterface(capconfigifuuid, (PVInterface*&)iPlayerCapConfigIF, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_CAPCONFIG1:
        {
            PvmiKvp* retparam = NULL;
            int retnumparam = 0;

            // Set the observer
            iPlayerCapConfigIF->setObserver(this);

            // Check that capability metric is 0
            PVPATB_TEST_IS_TRUE(iPlayerCapConfigIF->getCapabilityMetric(NULL) == 0);

            // Query using get
            OSCL_StackString<64> querykey;

            retparam = NULL;
            retnumparam = 0;
            querykey = _STRLIT_CHAR("x-pvmf/player;attr=cap");
            iPlayerCapConfigIF->getParametersSync(NULL, querykey.get_str(), retparam, retnumparam, NULL);
            // Just check the number of returned entries
            if (retparam != NULL && retnumparam == 13)
            {
                PVPATB_TEST_IS_TRUE(true);

                if (iPlayerCapConfigIF->releaseParameters(NULL, retparam, retnumparam) != PVMFSuccess)
                {
                    PVPATB_TEST_IS_TRUE(false);
                }
            }
            else
            {
                PVPATB_TEST_IS_TRUE(false);
            }

            retparam = NULL;
            retnumparam = 0;
            querykey = _STRLIT_CHAR("x-pvmf/player/productinfo;attr=cap");
            iPlayerCapConfigIF->getParametersSync(NULL, querykey.get_str(), retparam, retnumparam, NULL);
            // Just check the number of returned entries
            if (retparam != NULL && retnumparam == 5)
            {
                PVPATB_TEST_IS_TRUE(true);

                if (iPlayerCapConfigIF->releaseParameters(NULL, retparam, retnumparam) != PVMFSuccess)
                {
                    PVPATB_TEST_IS_TRUE(false);
                }
            }
            else
            {
                PVPATB_TEST_IS_TRUE(false);
            }

            // Set up KVPs for verify, set, and get
            PvmiKvp paramkvp[2];
            OSCL_StackString<64> paramkey1(_STRLIT_CHAR("x-pvmf/player/endtimecheck_interval;valtype=uint32"));
            OSCL_StackString<64> paramkey2(_STRLIT_CHAR("x-pvmf/player/nodecmd_timeout;valtype=uint32"));
            paramkvp[0].key = paramkey1.get_str();
            paramkvp[0].value.uint32_value = 2000;
            paramkvp[1].key = paramkey2.get_str();
            paramkvp[1].value.uint32_value = 4421;

            // Verify the new settings
            if (iPlayerCapConfigIF->verifyParametersSync(NULL, paramkvp, 2) == PVMFSuccess)
            {
                // Set the new settings
                iErrorKVP = NULL;
                iPlayerCapConfigIF->setParametersSync(NULL, paramkvp, 2, iErrorKVP);
                if (iErrorKVP == NULL)
                {
                    // Check by calling get
                    retparam = NULL;
                    retnumparam = 0;
                    paramkey1 += _STRLIT_CHAR(";attr=cur");
                    iPlayerCapConfigIF->getParametersSync(NULL, paramkey1.get_str(), retparam, retnumparam, NULL);
                    if (retparam != NULL && retnumparam == 1)
                    {
                        PVPATB_TEST_IS_TRUE(retparam->value.uint32_value == paramkvp[0].value.uint32_value);

                        if (iPlayerCapConfigIF->releaseParameters(NULL, retparam, retnumparam) != PVMFSuccess)
                        {
                            PVPATB_TEST_IS_TRUE(false);
                        }
                    }
                    else
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }

                    retparam = NULL;
                    retnumparam = 0;
                    paramkey2 += _STRLIT_CHAR(";attr=cur");
                    iPlayerCapConfigIF->getParametersSync(NULL, paramkey2.get_str(), retparam, retnumparam, NULL);
                    if (retparam != NULL && retnumparam == 1)
                    {
                        PVPATB_TEST_IS_TRUE(retparam->value.uint32_value == paramkvp[1].value.uint32_value);

                        if (iPlayerCapConfigIF->releaseParameters(NULL, retparam, retnumparam) != PVMFSuccess)
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
                    // Set failed
                    PVPATB_TEST_IS_TRUE(false);
                }
            }
            else
            {
                // Verify failed
                PVPATB_TEST_IS_TRUE(false);
            }

            // Try the set asynchronously
            iKeyStringSetAsync = _STRLIT_CHAR("x-pvmf/player/syncpointseekwindow;valtype=uint32");
            iKVPSetAsync.key = iKeyStringSetAsync.get_str();
            iKVPSetAsync.value.uint32_value = 2134;

            iErrorKVP = NULL;
            OSCL_TRY(error, iCurrentCmdId = iPlayerCapConfigIF->setParametersAsync(NULL, &iKVPSetAsync, 1, iErrorKVP));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_CAPCONFIG2:
        {
            // Check by calling get
            PvmiKvp* retparam = NULL;
            int retnumparam = 0;
            iKeyStringSetAsync += _STRLIT_CHAR(";attr=cur");
            iPlayerCapConfigIF->getParametersSync(NULL, iKeyStringSetAsync.get_str(), retparam, retnumparam, NULL);
            if (retparam != NULL && retnumparam == 1)
            {
                PVPATB_TEST_IS_TRUE(retparam->value.uint32_value == iKVPSetAsync.value.uint32_value);

                if (iPlayerCapConfigIF->releaseParameters(NULL, retparam, retnumparam) != PVMFSuccess)
                {
                    PVPATB_TEST_IS_TRUE(false);
                }
            }
            else
            {
                PVPATB_TEST_IS_TRUE(false);
            }

            iState = STATE_ADDDATASOURCE;
            RunIfNotReady();
        }
        break;

        case STATE_ADDDATASOURCE:
        {
            iDataSource = new PVPlayerDataSourceURL;
            oscl_UTF8ToUnicode(iFileName, oscl_strlen(iFileName), output, 512);
            wFileName.set(output, oscl_strlen(output));
            iDataSource->SetDataSourceURL(wFileName);
            iDataSource->SetDataSourceFormatType(iFileType);
            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }

        break;

        case STATE_INIT:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Init((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_CAPCONFIG3:
        {
            // Set up KVPs for verify, set, and get
            PvmiKvp paramkvp[2];
            OSCL_StackString<64> paramkey1(_STRLIT_CHAR("x-pvmf/player/endtimecheck_interval;valtype=uint32"));
            OSCL_StackString<64> paramkey2(_STRLIT_CHAR("x-pvmf/player/nodecmd_timeout;valtype=uint32"));
            paramkvp[0].key = paramkey1.get_str();
            paramkvp[0].value.uint32_value = 2000;
            paramkvp[1].key = paramkey2.get_str();
            paramkvp[1].value.uint32_value = 4421;

            // Verify the new settings
            if (iPlayerCapConfigIF->verifyParametersSync(NULL, paramkvp, 2) == PVMFSuccess)
            {
                // Set the new settings
                iErrorKVP = NULL;
                iPlayerCapConfigIF->setParametersSync(NULL, paramkvp, 2, iErrorKVP);
                if (iErrorKVP == NULL)
                {
                    // Check by calling get
                    PvmiKvp* retparam = NULL;
                    int retnumparam = 0;
                    paramkey1 += _STRLIT_CHAR(";attr=cur");
                    iPlayerCapConfigIF->getParametersSync(NULL, paramkey1.get_str(), retparam, retnumparam, NULL);
                    if (retparam != NULL && retnumparam == 1)
                    {
                        PVPATB_TEST_IS_TRUE(retparam->value.uint32_value == paramkvp[0].value.uint32_value);

                        if (iPlayerCapConfigIF->releaseParameters(NULL, retparam, retnumparam) != PVMFSuccess)
                        {
                            PVPATB_TEST_IS_TRUE(false);
                        }
                    }
                    else
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }

                    retparam = NULL;
                    retnumparam = 0;
                    paramkey2 += _STRLIT_CHAR(";attr=cur");
                    iPlayerCapConfigIF->getParametersSync(NULL, paramkey2.get_str(), retparam, retnumparam, NULL);
                    if (retparam != NULL && retnumparam == 1)
                    {
                        PVPATB_TEST_IS_TRUE(retparam->value.uint32_value == paramkvp[1].value.uint32_value);

                        if (iPlayerCapConfigIF->releaseParameters(NULL, retparam, retnumparam) != PVMFSuccess)
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
                    // Set failed
                    PVPATB_TEST_IS_TRUE(false);
                }
            }
            else
            {
                // Verify failed
                PVPATB_TEST_IS_TRUE(false);
            }

            iState = STATE_ADDDATASINK_VIDEO;
            RunIfNotReady();
        }
        break;

        case STATE_ADDDATASINK_VIDEO:
        {
            OSCL_wHeapString<OsclMemAllocator> SinkFileName;
            SinkFileName = OUTPUTNAME_PREPEND_WSTRING;
            SinkFileName += _STRLIT_WCHAR("test_player_capconfigif_");
            OSCL_wHeapString<OsclMemAllocator> inputfilename;
            RetrieveFilename(wFileName.get_str(), inputfilename);
            SinkFileName += inputfilename;
            SinkFileName += _STRLIT_WCHAR("_video.dat");

            iMIOFileOutVideo = iMioFactory->CreateVideoOutput((OsclAny*) & SinkFileName, MEDIATYPE_VIDEO, iCompressedVideo);
            iIONodeVideo = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutVideo);
            iDataSinkVideo = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkVideo)->SetDataSinkNode(iIONodeVideo);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_AUDIO:
        {
            OSCL_wHeapString<OsclMemAllocator> SinkFileName;
            SinkFileName = OUTPUTNAME_PREPEND_WSTRING;
            SinkFileName += _STRLIT_WCHAR("test_player_capconfigif_");
            OSCL_wHeapString<OsclMemAllocator> inputfilename;
            RetrieveFilename(wFileName.get_str(), inputfilename);
            SinkFileName += inputfilename;
            SinkFileName += _STRLIT_WCHAR("_audio.dat");

            iMIOFileOutAudio = iMioFactory->CreateAudioOutput((OsclAny*) & SinkFileName, MEDIATYPE_AUDIO, iCompressedAudio);
            iIONodeAudio = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutAudio);
            iDataSinkAudio = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkAudio)->SetDataSinkNode(iIONodeAudio);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_PREPARE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Prepare((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_START:
        {
            // Set up KVPs to enable video dec node postproc
            PvmiKvp paramkvp[2];
            OSCL_StackString<64> paramkey1(_STRLIT_CHAR("x-pvmf/video/decoder/postproc_enable;valtype=bool"));
            OSCL_StackString<64> paramkey2(_STRLIT_CHAR("x-pvmf/video/decoder/postproc_type;valtype=bitarray32"));
            paramkvp[0].key = paramkey1.get_str();
            paramkvp[0].value.bool_value = true;
            paramkvp[1].key = paramkey2.get_str();
            paramkvp[1].value.uint32_value = 1;

            // Set the new settings
            iErrorKVP = NULL;
            iPlayerCapConfigIF->setParametersSync(NULL, paramkvp, 2, iErrorKVP);
            // Don't check if set works

            OSCL_TRY(error, iCurrentCmdId = iPlayer->Start((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_CAPCONFIG4:
        {
            // Set up KVPs for verify, set, and get
            PvmiKvp paramkvp[2];
            OSCL_StackString<64> paramkey1(_STRLIT_CHAR("x-pvmf/player/pbpos_interval;valtype=uint32"));
            OSCL_StackString<64> paramkey2(_STRLIT_CHAR("x-pvmf/player/seektosyncpoint;valtype=bool"));
            paramkvp[0].key = paramkey1.get_str();
            paramkvp[0].value.uint32_value = 2000;
            paramkvp[1].key = paramkey2.get_str();
            paramkvp[1].value.bool_value = false;

            // Verify the new settings
            if (iPlayerCapConfigIF->verifyParametersSync(NULL, paramkvp, 2) == PVMFSuccess)
            {
                // Set the new settings
                iErrorKVP = NULL;
                iPlayerCapConfigIF->setParametersSync(NULL, paramkvp, 2, iErrorKVP);
                if (iErrorKVP == NULL)
                {
                    // Check by calling get
                    PvmiKvp* retparam = NULL;
                    int retnumparam = 0;
                    paramkey1 += _STRLIT_CHAR(";attr=cur");
                    iPlayerCapConfigIF->getParametersSync(NULL, paramkey1.get_str(), retparam, retnumparam, NULL);
                    if (retparam != NULL && retnumparam == 1)
                    {
                        PVPATB_TEST_IS_TRUE(retparam->value.uint32_value == paramkvp[0].value.uint32_value);

                        if (iPlayerCapConfigIF->releaseParameters(NULL, retparam, retnumparam) != PVMFSuccess)
                        {
                            PVPATB_TEST_IS_TRUE(false);
                        }
                    }
                    else
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }

                    retparam = NULL;
                    retnumparam = 0;
                    paramkey2 += _STRLIT_CHAR(";attr=cur");
                    iPlayerCapConfigIF->getParametersSync(NULL, paramkey2.get_str(), retparam, retnumparam, NULL);
                    if (retparam != NULL && retnumparam == 1)
                    {
                        PVPATB_TEST_IS_TRUE(retparam->value.bool_value == paramkvp[1].value.bool_value);

                        if (iPlayerCapConfigIF->releaseParameters(NULL, retparam, retnumparam) != PVMFSuccess)
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
                    // Set failed
                    PVPATB_TEST_IS_TRUE(false);
                }
            }
            else
            {
                // Verify failed
                PVPATB_TEST_IS_TRUE(false);
            }

            // Check if video decoder cap-config is available
            {
                PvmiKvp* retparam = NULL;
                int retnumparam = 0;
                OSCL_StackString<64> querykey(_STRLIT_CHAR("x-pvmf/video/decoder;attr=cap"));
                iPlayerCapConfigIF->getParametersSync(NULL, querykey.get_str(), retparam, retnumparam, NULL);
                // Just check the number of returned entries
                if (retparam != NULL && retnumparam == 6)
                {
                    PVPATB_TEST_IS_TRUE(true);

                    if (iPlayerCapConfigIF->releaseParameters(NULL, retparam, retnumparam) != PVMFSuccess)
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }
                }
                else
                {
                    // Depending on configuration video dec node might be available
                    // so don't flag this as error in test case
                }
            }


            // Done with cap-config testing
            iPlayerCapConfigIF = NULL;

            iState = STATE_STOP;
            RunIfNotReady();
        }
        break;

        case STATE_STOP:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Stop((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_VIDEO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_AUDIO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_RESET:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Reset((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASOURCE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_CLEANUPANDCOMPLETE:
        {
            PVPATB_TEST_IS_TRUE(PVPlayerFactory::DeletePlayer(iPlayer));
            iPlayer = NULL;

            delete iDataSource;
            iDataSource = NULL;

            delete iDataSinkVideo;
            iDataSinkVideo = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeVideo);
            iIONodeVideo = NULL;

            iMioFactory->DestroyVideoOutput(iMIOFileOutVideo);
            iMIOFileOutVideo = NULL;

            delete iDataSinkAudio;
            iDataSinkAudio = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeAudio);
            iIONodeAudio = NULL;

            iMioFactory->DestroyAudioOutput(iMIOFileOutAudio);
            iMIOFileOutAudio = NULL;

            iObserver->TestCompleted(*iTestCase);
        }
        break;

        default:
            break;

    }
}


void pvplayer_async_test_capconfigiftest::CommandCompleted(const PVCmdResponse& aResponse)
{
    if (aResponse.GetCmdId() != iCurrentCmdId)
    {
        // Wrong command ID.
        PVPATB_TEST_IS_TRUE(false);
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
        return;
    }

    if (aResponse.GetContext() != NULL)
    {
        if (aResponse.GetContext() == (OsclAny*)&iContextObject)
        {
            if (iContextObject != iContextObjectRefValue)
            {
                // Context data value was corrupted
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                return;
            }
        }
        else
        {
            // Context data pointer was corrupted
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
            return;
        }
    }

    switch (iState)
    {
        case STATE_QUERYINTERFACE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_CAPCONFIG1;
                RunIfNotReady();
            }
            else
            {
                // QueryInterface failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_INIT;
                RunIfNotReady();
            }
            else
            {
                // AddDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_INIT:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_CAPCONFIG3;
                RunIfNotReady();
            }
            else
            {
                // Init failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_PREPARE;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_PREPARE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_START;
                RunIfNotReady();
            }
            else
            {
                // Prepare failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_START:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_CAPCONFIG4;
                RunIfNotReady();
            }
            else
            {
                // Start failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_STOP:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                // Stop failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_RESET;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_RESET:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASOURCE;
                RunIfNotReady();
            }
            else
            {
                // Reset failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                PVPATB_TEST_IS_TRUE(true);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        default:
        {
            // Testing error if this is reached
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;
    }
}


void pvplayer_async_test_capconfigiftest::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    switch (aEvent.GetEventType())
    {
        case PVMFErrResourceConfiguration:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrResource:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrCorrupt:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrProcessing:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        default:
            // Unknown error and just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;
    }

    // Wait for engine to handle the error
    Cancel();
}


void pvplayer_async_test_capconfigiftest::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingStart)
    {
        // do nothing
    }
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingComplete)
    {
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
    }
}

void pvplayer_async_test_capconfigiftest::SignalEvent(int32 req_id)
{
    if (req_id != iCurrentCmdId)
    {
        // Wrong command ID.
        PVPATB_TEST_IS_TRUE(false);
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
        return;
    }

    if (iState == STATE_CAPCONFIG1)
    {
        if (iErrorKVP != NULL)
        {
            // There was an error in setParameterAsync()
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_ADDDATASOURCE;
        }
        else
        {
            iState = STATE_CAPCONFIG2;
        }
        RunIfNotReady();
    }
}

void pvplayer_async_test_setplaybackafterprepare::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_setplaybackafterprepare::Run()
{
    int error = 0;

    switch (iState)
    {
        case STATE_CREATE:
        {
            iPlayer = NULL;

            OSCL_TRY(error, iPlayer = PVPlayerFactory::CreatePlayer(this, this, this));
            if (error)
            {
                PVPATB_TEST_IS_TRUE(false);
                iObserver->TestCompleted(*iTestCase);
            }
            else
            {
                iState = STATE_ADDDATASOURCE;
                RunIfNotReady();
            }
        }
        break;

        case STATE_ADDDATASOURCE:
        {
            PVMFFormatType fileType;
            iDataSource = new PVPlayerDataSourceURL;
            OSCL_HeapString<OsclMemAllocator> filename;

            if (oscl_strstr(iFileName, "test.mp4") != NULL)
            {
                filename = SOURCENAME_PREPEND_STRING;
                filename += _STRLIT_CHAR("test_reposition.mp4");
                fileType = PVMF_MIME_MPEG4FF;
                iFileType = PVMF_MIME_MPEG4FF;
            }
            else
            {
                filename = iFileName;
                fileType = iFileType;
            }
            fprintf(iTestMsgOutputFile, "***Source URL=%s\n", filename.get_str());
            oscl_UTF8ToUnicode(filename.get_str(), filename.get_size(), output, 256);
            wFileName.set(output, oscl_strlen(output));

            if (iFileType == PVMF_MIME_DATA_SOURCE_HTTP_URL)
            {
                fileType = PVMF_MIME_DATA_SOURCE_HTTP_URL;
                iDataSource->SetAlternateSourceFormatType(PVMF_MIME_DATA_SOURCE_MS_HTTP_STREAMING_URL);
                iSourceContextData = new PVMFSourceContextData();
                iSourceContextData->EnableCommonSourceContext();
                iSourceContextData->EnableStreamingSourceContext();

                PVInterface* sourceContext = NULL;
                PVUuid streamingContextUuid(PVMF_SOURCE_CONTEXT_DATA_STREAMING_UUID);
                if (iSourceContextData->queryInterface(streamingContextUuid, sourceContext))
                {
                    PVMFSourceContextDataStreaming* streamingContext =
                        OSCL_STATIC_CAST(PVMFSourceContextDataStreaming*, sourceContext);
                    streamingContext->iStreamStatsLoggingURL = wFileName;

                    if (iProxyEnabled)
                    {
                        streamingContext->iProxyName = _STRLIT_WCHAR("63.215.241.33");
                        streamingContext->iProxyPort = 7070;
                    }

                    streamingContext->iUserID = _STRLIT_CHAR("abc");
                    streamingContext->iUserPasswd = _STRLIT_CHAR("xyz");
                }
                iSourceContextData->EnableDownloadHTTPSourceContext();
                sourceContext = NULL;
                PVUuid downloadContextUuid(PVMF_SOURCE_CONTEXT_DATA_DOWNLOAD_HTTP_UUID);
                if (iSourceContextData->queryInterface(downloadContextUuid, sourceContext))
                {
                    PVMFSourceContextDataDownloadHTTP* downloadContext =
                        OSCL_STATIC_CAST(PVMFSourceContextDataDownloadHTTP*, sourceContext);
                    if (iProxyEnabled)
                    {
                        downloadContext->iProxyName = _STRLIT_CHAR("63.215.241.33");
                        downloadContext->iProxyPort = 7070;
                    }
                    downloadContext->iDownloadFileName += _STRLIT_WCHAR("test_ftdownload.loc");
                    downloadContext->iConfigFileName += _STRLIT_WCHAR("mydlconfig");
                    downloadContext->iUserID = _STRLIT_CHAR("abc");
                    downloadContext->iUserPasswd = _STRLIT_CHAR("xyz");
                    downloadContext->bIsNewSession = true;
                    downloadContext->iMaxFileSize = 0x7FFFFFFF;
                    downloadContext->iPlaybackControl = PVMFSourceContextDataDownloadHTTP::EAsap;
                }
                iDataSource->SetDataSourceContextData((OsclAny*)iSourceContextData);
            }
            iDataSource->SetDataSourceURL(wFileName);
            iDataSource->SetDataSourceFormatType(fileType);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }

        break;

        case STATE_INIT:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Init((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_VIDEO:
        {
            OSCL_wHeapString<OsclMemAllocator> SinkFileName;
            SinkFileName = OUTPUTNAME_PREPEND_WSTRING;
            SinkFileName += _STRLIT_WCHAR("test_player_setplaybackafterprepare_");
            OSCL_wHeapString<OsclMemAllocator> inputfilename;
            RetrieveFilename(wFileName.get_str(), inputfilename);
            SinkFileName += inputfilename;
            SinkFileName += _STRLIT_WCHAR("_video.dat");

            iMIOFileOutVideo = iMioFactory->CreateVideoOutput((OsclAny*) & SinkFileName, MEDIATYPE_VIDEO, iCompressedVideo);
            iIONodeVideo = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutVideo);
            iDataSinkVideo = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkVideo)->SetDataSinkNode(iIONodeVideo);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());

        }
        break;

        case STATE_ADDDATASINK_AUDIO:
        {
            OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
            sinkfile += _STRLIT_WCHAR("test_player_setplaybackafterprepare_audio.dat");
            OSCL_wHeapString<OsclMemAllocator> inputfilename;
            RetrieveFilename(wFileName.get_str(), inputfilename);
            sinkfile += inputfilename;
            sinkfile += _STRLIT_WCHAR("_audio.dat");

            iMIOFileOutAudio = iMioFactory->CreateAudioOutput((OsclAny*) & sinkfile, MEDIATYPE_AUDIO, iCompressedAudio);
            iIONodeAudio = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutAudio);
            iDataSinkAudio = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkAudio)->SetDataSinkNode(iIONodeAudio);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_TEXT:
        {
            OSCL_wHeapString<OsclMemAllocator> sinkfilename;
            sinkfilename = OUTPUTNAME_PREPEND_WSTRING;
            sinkfilename += _STRLIT_WCHAR("test_player_setplaybackafterprepare_");
            OSCL_wHeapString<OsclMemAllocator> inputfilename;
            RetrieveFilename(wFileName.get_str(), inputfilename);
            sinkfilename += inputfilename;
            sinkfilename += _STRLIT_WCHAR("_text.dat");

            iMIOFileOutText = iMioFactory->CreateTextOutput((OsclAny*) & sinkfilename, MEDIATYPE_TEXT);
            iIONodeText = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutText);
            iDataSinkText = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkText)->SetDataSinkNode(iIONodeText);


            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkText, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_PREPARE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Prepare((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_SETPLAYBACKRANGE:
        {
            PVPPlaybackPosition start, end;
            start.iIndeterminate = false;
            end.iIndeterminate = true;
            start.iPosUnit = PVPPBPOSUNIT_MILLISEC;
            start.iMode = PVPPBPOS_MODE_NOW;
            start.iPosValue.millisec_value = 10000;
            OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRange(start, end, false, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_START:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Start((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_STOP:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Stop((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_VIDEO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_AUDIO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_TEXT:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkText, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_RESET:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Reset((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASOURCE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_CLEANUPANDCOMPLETE:
        {
            PVPATB_TEST_IS_TRUE(PVPlayerFactory::DeletePlayer(iPlayer));
            iPlayer = NULL;

            delete iSourceContextData;
            iSourceContextData = NULL;

            delete iDataSource;
            iDataSource = NULL;

            delete iDataSinkVideo;
            iDataSinkVideo = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeVideo);
            iIONodeVideo = NULL;

            iMioFactory->DestroyVideoOutput(iMIOFileOutVideo);
            iMIOFileOutVideo = NULL;

            delete iDataSinkAudio;
            iDataSinkAudio = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeAudio);
            iIONodeAudio = NULL;

            iMioFactory->DestroyAudioOutput(iMIOFileOutAudio);
            iMIOFileOutAudio = NULL;

            delete iDataSinkText;
            iDataSinkText = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeText);
            iIONodeText = NULL;

            iMioFactory->DestroyTextOutput(iMIOFileOutText);
            iMIOFileOutText = NULL;

            iObserver->TestCompleted(*iTestCase);
        }
        break;

        default:
            break;

    }
}


void pvplayer_async_test_setplaybackafterprepare::CommandCompleted(const PVCmdResponse& aResponse)
{
    if (aResponse.GetCmdId() != iCurrentCmdId)
    {
        // Wrong command ID.
        PVPATB_TEST_IS_TRUE(false);
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
        return;
    }

    if (aResponse.GetContext() != NULL)
    {
        if (aResponse.GetContext() == (OsclAny*)&iContextObject)
        {
            if (iContextObject != iContextObjectRefValue)
            {
                // Context data value was corrupted
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                return;
            }
        }
        else
        {
            // Context data pointer was corrupted
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
            return;
        }
    }

    switch (iState)
    {
        case STATE_ADDDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_INIT;
                RunIfNotReady();
            }
            else
            {
                // AddDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_INIT:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                // Init failed


                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_TEXT;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_TEXT:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_PREPARE;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_PREPARE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_SETPLAYBACKRANGE;
                RunIfNotReady();
            }
            else
            {
                // Prepare failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_SETPLAYBACKRANGE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_START;
                RunIfNotReady();
            }
            else if (aResponse.GetCmdStatus() == PVMFErrNotSupported)
            {
                fprintf(file, " SetPlaybackRange not supported \n");
                iState = STATE_START;
                RunIfNotReady();
            }
            else
            {
                // Stop failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_START:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_STOP;
                RunIfNotReady(5000000);
            }
            else
            {
                // Start failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_STOP:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_VIDEO;
                RunIfNotReady(3000000);
            }
            else
            {
                // Stop failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_TEXT;
                //	iState=STATE_RESET;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_TEXT:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_RESET;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_RESET:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASOURCE;
                RunIfNotReady();
            }
            else
            {
                // Reset failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                PVPATB_TEST_IS_TRUE(true);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        default:
        {
            // Testing error if this is reached
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;
    }
}


void pvplayer_async_test_setplaybackafterprepare::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    switch (aEvent.GetEventType())
    {
        case PVMFErrResourceConfiguration:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrResource:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrCorrupt:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrProcessing:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        default:
            // Unknown error and just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;
    }
}


void pvplayer_async_test_setplaybackafterprepare::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingStart)
    {
        // do nothing
    }
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingComplete)
    {
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
    }
    if (aEvent.GetEventType() == PVMFInfoPositionStatus)
    {
        PVInterface* iface = (PVInterface*)(aEvent.GetEventExtensionInterface());
        if (iface == NULL)
        {
            return;
        }
        PVUuid infomsguuid = PVMFErrorInfoMessageInterfaceUUID;
        PVMFErrorInfoMessageInterface* infomsgiface = NULL;
        if (iface->queryInterface(infomsguuid, (PVInterface*&)infomsgiface) == true)
        {
            int32 infocode;
            PVUuid infouuid;
            infomsgiface->GetCodeUUID(infocode, infouuid);
            if ((infouuid == PVPlayerErrorInfoEventTypesUUID) && (infocode == PVPlayerInfoPlaybackPositionStatus))
            {
                PVPPlaybackPosition aPos1;
                uint32 aPos2;

                aPos1.iPosUnit = PVPPBPOSUNIT_MILLISEC;
                iPlayer->GetCurrentPositionSync(aPos1);

                uint8* localbuf = aEvent.GetLocalBuffer();
                if (aEvent.GetLocalBufferSize() == 8 && localbuf[0] == 1)
                {
                    oscl_memcpy(&aPos2, &localbuf[4], sizeof(uint32));
                }
                else
                {
                    aPos2 = 0;
                }
                fprintf(file, " PVMFInfoPositionStatus: %d\n", aPos2);

                /*
                				if ( !(aPos1.iPosValue.millisec_value <= aPos2 + 50) )
                				{
                					PVPATB_TEST_IS_TRUE(false);
                				}
                */
            }
        }
    }
}

//
// pvplayer_async_test_multipauseseekresume section
//
void pvplayer_async_test_multipauseseekresume::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_multipauseseekresume::Run()
{
    int error = 0;

    switch (iState)
    {
        case STATE_CREATE:
        {
            iPlayer = NULL;

            OSCL_TRY(error, iPlayer = PVPlayerFactory::CreatePlayer(this, this, this));
            if (error)
            {
                PVPATB_TEST_IS_TRUE(false);
                iObserver->TestCompleted(*iTestCase);
            }
            else
            {
                iState = STATE_ADDDATASOURCE;
                RunIfNotReady();
            }
        }
        break;

        case STATE_ADDDATASOURCE:
        {
            PVMFFormatType fileType;
            iDataSource = new PVPlayerDataSourceURL;
            OSCL_HeapString<OsclMemAllocator> filename;

            if (oscl_strstr(iFileName, "test.mp4") != NULL)
            {
                filename = SOURCENAME_PREPEND_STRING;
                filename += _STRLIT_CHAR("test_reposition.mp4");
                fileType = PVMF_MIME_MPEG4FF;
                iFileType = PVMF_MIME_MPEG4FF;
            }
            else
            {
                filename = iFileName;
                fileType = iFileType;
            }
            fprintf(iTestMsgOutputFile, "***Source URL=%s\n", filename.get_str());
            oscl_UTF8ToUnicode(filename.get_str(), filename.get_size(), output, 256);
            wFileName.set(output, oscl_strlen(output));

            if (iFileType == PVMF_MIME_DATA_SOURCE_HTTP_URL)
            {
                fileType = PVMF_MIME_DATA_SOURCE_HTTP_URL;
                iDataSource->SetAlternateSourceFormatType(PVMF_MIME_DATA_SOURCE_MS_HTTP_STREAMING_URL);
                iSourceContextData = new PVMFSourceContextData();
                iSourceContextData->EnableCommonSourceContext();
                iSourceContextData->EnableStreamingSourceContext();

                PVInterface* sourceContext = NULL;
                PVUuid streamingContextUuid(PVMF_SOURCE_CONTEXT_DATA_STREAMING_UUID);
                if (iSourceContextData->queryInterface(streamingContextUuid, sourceContext))
                {
                    PVMFSourceContextDataStreaming* streamingContext =
                        OSCL_STATIC_CAST(PVMFSourceContextDataStreaming*, sourceContext);
                    streamingContext->iStreamStatsLoggingURL = wFileName;

                    if (iProxyEnabled)
                    {
                        streamingContext->iProxyName = _STRLIT_WCHAR("63.215.241.33");
                        streamingContext->iProxyPort = 7070;
                    }

                    streamingContext->iUserID = _STRLIT_CHAR("abc");
                    streamingContext->iUserPasswd = _STRLIT_CHAR("xyz");
                }
                iSourceContextData->EnableDownloadHTTPSourceContext();
                sourceContext = NULL;
                PVUuid downloadContextUuid(PVMF_SOURCE_CONTEXT_DATA_DOWNLOAD_HTTP_UUID);
                if (iSourceContextData->queryInterface(downloadContextUuid, sourceContext))
                {
                    PVMFSourceContextDataDownloadHTTP* downloadContext =
                        OSCL_STATIC_CAST(PVMFSourceContextDataDownloadHTTP*, sourceContext);
                    if (iProxyEnabled)
                    {
                        downloadContext->iProxyName = _STRLIT_CHAR("63.215.241.33");
                        downloadContext->iProxyPort = 7070;
                    }
                    downloadContext->iDownloadFileName += _STRLIT_WCHAR("test_ftdownload.loc");
                    downloadContext->iConfigFileName += _STRLIT_WCHAR("mydlconfig");
                    downloadContext->iUserID = _STRLIT_CHAR("abc");
                    downloadContext->iUserPasswd = _STRLIT_CHAR("xyz");
                    downloadContext->bIsNewSession = true;
                    downloadContext->iMaxFileSize = 0x7FFFFFFF;
                    downloadContext->iPlaybackControl = PVMFSourceContextDataDownloadHTTP::EAsap;
                }
                iDataSource->SetDataSourceContextData((OsclAny*)iSourceContextData);
            }
            iDataSource->SetDataSourceURL(wFileName);
            iDataSource->SetDataSourceFormatType(fileType);
            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_INIT:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Init((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_GETMETADATAKEYLIST:
        {
            iMetadataKeyList.clear();
            OSCL_TRY(error, iCurrentCmdId = iPlayer->GetMetadataKeys(iMetadataKeyList, 0, -1, NULL, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_GETMETADATAVALUELIST:
        {
            iMetadataValueList.clear();
            iNumValues = 0;
            OSCL_TRY(error, iCurrentCmdId = iPlayer->GetMetadataValues(iMetadataKeyList, 0, -1, iNumValues, iMetadataValueList, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_VIDEO:
        {
            OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
            sinkfile += _STRLIT_WCHAR("test_player_setstartpositionplaystop_video.dat");

            iMIOFileOutVideo = iMioFactory->CreateVideoOutput((OsclAny*) & sinkfile, MEDIATYPE_VIDEO, iCompressedVideo);
            iIONodeVideo = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutVideo);
            iDataSinkVideo = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkVideo)->SetDataSinkNode(iIONodeVideo);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_AUDIO:
        {
            OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
            sinkfile += _STRLIT_WCHAR("test_player_setstartpositionplaystop_audio.dat");

            iMIOFileOutAudio = iMioFactory->CreateAudioOutput((OsclAny*) & sinkfile, MEDIATYPE_AUDIO, iCompressedAudio);
            iIONodeAudio = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutAudio);
            iDataSinkAudio = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkAudio)->SetDataSinkNode(iIONodeAudio);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_PREPARE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Prepare((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_START:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Start((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_PAUSE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Pause((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            iCmdIds.push_back(iCurrentCmdId);
        }
        break;

        case STATE_SETPLAYBACKRANGE:
        {
            PVPPlaybackPosition start, end;
            start.iIndeterminate = false;
            start.iPosUnit = PVPPBPOSUNIT_MILLISEC;
            start.iMode = PVPPBPOS_MODE_NOW;
            start.iPosValue.millisec_value = iSessionDuration / 4;
            end.iIndeterminate = true;
            OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRange(start, end, false, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_RESUME:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Resume((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            iCmdIds.push_back(iCurrentCmdId);
        }
        break;

        case STATE_STOP:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Stop((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_VIDEO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_AUDIO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_RESET:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Reset((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASOURCE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_CLEANUPANDCOMPLETE:
        {
            PVPATB_TEST_IS_TRUE(PVPlayerFactory::DeletePlayer(iPlayer));
            iPlayer = NULL;

            delete iSourceContextData;
            iSourceContextData = NULL;

            delete iDataSource;
            iDataSource = NULL;

            delete iDataSinkVideo;
            iDataSinkVideo = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeVideo);
            iIONodeVideo = NULL;

            iMioFactory->DestroyVideoOutput(iMIOFileOutVideo);
            iMIOFileOutVideo = NULL;

            delete iDataSinkAudio;
            iDataSinkAudio = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeAudio);
            iIONodeAudio = NULL;

            iMioFactory->DestroyAudioOutput(iMIOFileOutAudio);
            iMIOFileOutAudio = NULL;

            iObserver->TestCompleted(*iTestCase);
        }
        break;

        default:
            break;

    }
}


void pvplayer_async_test_multipauseseekresume::CommandCompleted(const PVCmdResponse& aResponse)
{
    if (aResponse.GetCmdId() != iCurrentCmdId)
    {
        // Wrong command ID.
        PVPATB_TEST_IS_TRUE(false);
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
        return;
    }

    if (aResponse.GetContext() != NULL)
    {
        if (aResponse.GetContext() == (OsclAny*)&iContextObject)
        {
            if (iContextObject != iContextObjectRefValue)
            {
                // Context data value was corrupted
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                return;
            }
        }
        else
        {
            // Context data pointer was corrupted
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
            return;
        }
    }

    switch (iState)
    {
        case STATE_ADDDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_INIT;
                RunIfNotReady();
            }
            else
            {
                // AddDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_INIT:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_GETMETADATAKEYLIST;
                RunIfNotReady();
            }
            else
            {
                // Init failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GETMETADATAKEYLIST:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_GETMETADATAVALUELIST;
                RunIfNotReady();
            }
            else
            {
                // GetMetadataKeys failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GETMETADATAVALUELIST:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                for (uint32 i = 0; i < iMetadataValueList.size(); ++i)
                {
                    if ((oscl_strstr(iMetadataValueList[i].key, "duration")) && iSessionDuration == 0)
                    {
                        iSessionDuration = iMetadataValueList[i].value.uint32_value;

                        // Check the timescale. If not available, assume millisecond (1000)
                        const char *retTSstr;
                        retTSstr = oscl_strstr(iMetadataValueList[i].key, "timescale=");
                        uint32 retTSstrLen = 0;
                        uint32 tsstrlen = oscl_strlen(_STRLIT_CHAR("timescale="));
                        if (retTSstr != NULL)
                        {
                            retTSstrLen = oscl_strlen(retTSstr);
                            if (retTSstrLen > tsstrlen)
                            {
                                uint32 timescale = 0;
                                PV_atoi((char*)(retTSstr + tsstrlen), 'd', (retTSstrLen - tsstrlen), timescale);
                                if (timescale > 0 && timescale != 1000)
                                {
                                    // Convert to milliseconds
                                    MediaClockConverter mcc(timescale);
                                    mcc.update_clock(iSessionDuration);
                                    iSessionDuration = mcc.get_converted_ts(1000);
                                }
                            }
                        }
                    }
                }
                iState = STATE_ADDDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                // GetMetadataValue failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_PREPARE;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_PREPARE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_START;
                RunIfNotReady();
            }
            else
            {
                // Prepare failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_START:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_PAUSE;
                uint32 iPauseDurationMS = iSessionDuration / 10;
                RunIfNotReady(iPauseDurationMS*1000);
            }
            else
            {
                // Start failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_PAUSE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iNumPause++;
                iState = STATE_SETPLAYBACKRANGE;
                RunIfNotReady(1000000);
            }
            else
            {
                // One of queued commands failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_SETPLAYBACKRANGE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_RESUME;
                RunIfNotReady(1000000);
            }
            else
            {
                // SetPlaybackRange failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_RESUME:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if (iTargetNumPause != iNumPause)
                {
                    iState = STATE_PAUSE;
                    uint32 iPauseDurationMS = iSessionDuration / 10;
                    RunIfNotReady(iPauseDurationMS*1000);
                }
            }
            else
            {
                // One of queued commands failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_STOP:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                // Stop failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_RESET;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_RESET:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASOURCE;
                RunIfNotReady();
            }
            else
            {
                // Reset failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                PVPATB_TEST_IS_TRUE(true);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        default:
        {
            // Testing error if this is reached
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;
    }
}


void pvplayer_async_test_multipauseseekresume::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    switch (aEvent.GetEventType())
    {
        case PVMFErrResourceConfiguration:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrResource:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrCorrupt:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrProcessing:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        default:
            // Unknown error and just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;
    }

    // Wait for engine to handle the error
    Cancel();
}


void pvplayer_async_test_multipauseseekresume::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingStart)
    {
        // do nothing
    }
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingComplete)
    {
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
    }
    // Check for end of clip event
    if (aEvent.GetEventType() == PVMFInfoEndOfData)
    {
        PVInterface* iface = (PVInterface*)(aEvent.GetEventExtensionInterface());
        if (iface == NULL)
        {
            return;
        }
        PVUuid infomsguuid = PVMFErrorInfoMessageInterfaceUUID;
        PVMFErrorInfoMessageInterface* infomsgiface = NULL;
        if (iface->queryInterface(infomsguuid, (PVInterface*&)infomsgiface) == true)
        {
            int32 infocode;
            PVUuid infouuid;
            infomsgiface->GetCodeUUID(infocode, infouuid);
            if ((infouuid == PVPlayerErrorInfoEventTypesUUID) && (infocode == PVPlayerInfoEndOfClipReached))
            {
                iState = STATE_STOP;
                Cancel();
                RunIfNotReady();
            }
        }
    }
    else if (aEvent.GetEventType() == PVMFInfoSourceFormatNotSupported)
    {
        // source rollover...
        fprintf(iTestMsgOutputFile, "###PVMFInfoSourceFormatNotSupported...\n");
        PVInterface* iface = (PVInterface*)(aEvent.GetEventExtensionInterface());
        if (iface != NULL)
        {
            PVUuid infomsguuid = PVMFErrorInfoMessageInterfaceUUID;
            PVMFErrorInfoMessageInterface* infomsgiface = NULL;
            if (iface->queryInterface(infomsguuid, (PVInterface*&)infomsgiface) == true)
            {
                int32 infocode;
                PVUuid infouuid;
                infomsgiface->GetCodeUUID(infocode, infouuid);
                if ((infouuid == PVPlayerErrorInfoEventTypesUUID) &&
                        (infocode == PVPlayerInfoAttemptingSourceRollOver))
                {
                    uint8* localBuf = aEvent.GetLocalBuffer();
                    if (localBuf != NULL)
                    {
                        uint32 srcFormat = 0;
                        oscl_memcpy(&srcFormat, &localBuf[4], sizeof(uint32));
                        fprintf(iTestMsgOutputFile, "###Attempting Source RollOver - NewSrcFormat=%d\n", srcFormat);
                    }
                }
            }
        }
    }
}

//
// pvplayer_async_test_multiple_instance section
//
#include "pvmf_local_data_source.h"
#include "oscl_scheduler.h"
#include "OMX_Core.h"
#include "oscl_mem_audit.h"

//test observer function for the 2nd instance.
void pvplayer_async_test_multiple_instance::ChildObserver::TestCompleted(test_case &tc)
{
    OSCL_UNUSED_ARG(tc);
    OsclExecScheduler* sched = OsclExecScheduler::Current();
    if (sched)
        sched->StopScheduler();
}

void pvplayer_async_test_multiple_instance::ChildTestIsTrue(bool aCondition)
//add failures from child to the parent error count.
{
    OSCL_ASSERT(iParentInstance);
    if (!aCondition)
        iParentInstance->iChildThreadFailures++;
}

//To avoid thread contention for the test result, only the parent thread can register
//test failures directly in the test case.  The child thread increments a counter that
//will get added to test case results after the child thread exits.
#define MULTI_TEST_IS_TRUE(x) (iParentInstance)?ChildTestIsTrue(x):PVPATB_TEST_IS_TRUE(x);

void pvplayer_async_test_multiple_instance::InThread(pvplayer_async_test_multiple_instance* parent)
{
    //Init Oscl
    OsclMem::Init();
    OMX_Init();
    PVLogger::Init();
    OsclScheduler::Init("pvplayer_async_test_multiple_instance");

    {
        //create a 2nd test instance.
        pvplayer_async_test_multiple_instance* test = new pvplayer_async_test_multiple_instance(*parent->iParam);

        //replace the observer with one appropriate for this thread.
        ChildObserver* obs = new ChildObserver;
        test->iObserver = obs;

        //replace sink name with a unique name for this thread.
        test->iSinkName = _STRLIT_WCHAR("pvplayer_async_test_multiple_instance_2nd_");

        //save parent instance pointer
        test->iParentInstance = parent;

        //add to scheduler and go
        test->StartTest();
        OsclExecScheduler* sched = OsclExecScheduler::Current();
        sched->StartScheduler();

        //cleanup.
        delete(obs);
        delete(test);
    }

    //Cleanup Oscl.
    OsclScheduler::Cleanup();
    PVLogger::Cleanup();
    OMX_Deinit();
    //note: there's no memory leak check here since
    //this thread exits before the main thread.
    OsclMem::Cleanup();
}

static TOsclThreadFuncRet OSCL_THREAD_DECL pvplayer_async_test_multiple_instance_thread(TOsclThreadFuncArg arg)
{
    //parent test is passed in as arg
    pvplayer_async_test_multiple_instance* parent = (pvplayer_async_test_multiple_instance*)arg;

    //Init Oscl
    OsclBase::Init();
    OsclErrorTrap::Init();

    //Run remainder of the thread under a trap.
    OSCL_TRY(parent->iChildThreadLeave,
             pvplayer_async_test_multiple_instance::InThread(parent););

    //Cleanup Oscl
    OsclErrorTrap::Cleanup();
    OsclBase::Cleanup();

    //parent thread waits on this signal to know this thread exited.
    parent->iChildThreadExit = true;

    return 0;
}

void pvplayer_async_test_multiple_instance::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}

void pvplayer_async_test_multiple_instance::Run()
{
    int error = 0;

    switch (iState)
    {
        case STATE_CREATE:
        {
            //If this is the original instance, then launch a 2nd thread.
            if (!iParentInstance)
            {
                OsclThread thread;
                OsclProcStatus::eOsclProcError  result = thread.Create(pvplayer_async_test_multiple_instance_thread, 0, this);
                if (result != OsclProcStatus::SUCCESS_ERROR)
                {
                    PVPATB_TEST_IS_TRUE(false);
                    iObserver->TestCompleted(*iTestCase);
                    break;
                }
            }

            OSCL_TRY(error, iPlayer = PVPlayerFactory::CreatePlayer(this, this, this));
            if (error)
            {
                MULTI_TEST_IS_TRUE(false);
                iObserver->TestCompleted(*iTestCase);
            }
            else
            {
                iState = STATE_ADDDATASOURCE;
                RunIfNotReady();
            }
        }
        break;

        case STATE_ADDDATASOURCE:
        {
            iDataSource = new PVPlayerDataSourceURL;

            OSCL_HeapString<OsclMemAllocator> fname(iFileName);
            oscl_UTF8ToUnicode(fname.get_cstr(), fname.get_size(), iTmpWCharBuffer, 512);
            iFileNameWStr.set(iTmpWCharBuffer, oscl_strlen(iTmpWCharBuffer));
            iDataSource->SetDataSourceURL(iFileNameWStr);
            iDataSource->SetDataSourceFormatType(iFileType);
            iLocalDataSource = new PVMFLocalDataSource();
            iDataSource->SetDataSourceContextData((OsclAny*)iLocalDataSource);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, MULTI_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }

        break;

        case STATE_INIT:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Init((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, MULTI_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_GETMETADATAKEYLIST:
        {
            iMetadataKeyList.clear();
            OSCL_TRY(error, iCurrentCmdId = iPlayer->GetMetadataKeys(iMetadataKeyList, 0, 100, NULL, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, MULTI_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_GETMETADATAVALUELIST:
        {
            iMetadataValueList.clear();
            iNumValues = 0;
            OSCL_TRY(error, iCurrentCmdId = iPlayer->GetMetadataValues(iMetadataKeyList, 0, 100, iNumValues, iMetadataValueList, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, MULTI_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_VIDEO:
        {

            OSCL_wHeapString<OsclMemAllocator> sinkfilename;
            sinkfilename = OUTPUTNAME_PREPEND_WSTRING;
            sinkfilename += iSinkName;
            if (iCompressedVideo)
            {
                sinkfilename += _STRLIT_WCHAR("compressed_");
            }
            OSCL_wHeapString<OsclMemAllocator> inputfilename;
            RetrieveFilename(iFileNameWStr.get_str(), inputfilename);
            sinkfilename += inputfilename;
            sinkfilename += _STRLIT_WCHAR("_video.dat");

            iMOutVideo = iMioFactory->CreateVideoOutput((OsclAny*) & sinkfilename, MEDIATYPE_VIDEO, iCompressedVideo);
            iIONodeVideo = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMOutVideo);
            iDataSinkVideo = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkVideo)->SetDataSinkNode(iIONodeVideo);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, MULTI_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_AUDIO:
        {
            OSCL_wHeapString<OsclMemAllocator> sinkfilename;
            sinkfilename = OUTPUTNAME_PREPEND_WSTRING;
            sinkfilename += iSinkName;
            if (iCompressedAudio)
            {
                sinkfilename += _STRLIT_WCHAR("compressed_");
            }
            OSCL_wHeapString<OsclMemAllocator> inputfilename;
            RetrieveFilename(iFileNameWStr.get_str(), inputfilename);
            sinkfilename += inputfilename;
            sinkfilename += _STRLIT_WCHAR("_audio.dat");

            iMOutAudio = iMioFactory->CreateAudioOutput((OsclAny*) & sinkfilename, MEDIATYPE_AUDIO, iCompressedAudio);

            iIONodeAudio = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMOutAudio);
            iDataSinkAudio = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkAudio)->SetDataSinkNode(iIONodeAudio);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, MULTI_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_TEXT:
        {
            OSCL_wHeapString<OsclMemAllocator> sinkfilename;
            sinkfilename = OUTPUTNAME_PREPEND_WSTRING;
            sinkfilename += iSinkName;
            OSCL_wHeapString<OsclMemAllocator> inputfilename;
            RetrieveFilename(iFileNameWStr.get_str(), inputfilename);
            sinkfilename += inputfilename;
            sinkfilename += _STRLIT_WCHAR("_text.dat");

            iMOutText = iMioFactory->CreateTextOutput((OsclAny*) & sinkfilename, MEDIATYPE_TEXT);
            iIONodeText = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMOutText);
            iDataSinkText = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkText)->SetDataSinkNode(iIONodeText);


            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkText, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, MULTI_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_PREPARE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Prepare((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, MULTI_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_START:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Start((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, MULTI_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_STOP:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Stop((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, MULTI_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_VIDEO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, MULTI_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_AUDIO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, MULTI_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_TEXT:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkText, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, MULTI_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_RESET:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Reset((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, MULTI_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASOURCE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, MULTI_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_CLEANUPANDCOMPLETE:
        {
            MULTI_TEST_IS_TRUE(PVPlayerFactory::DeletePlayer(iPlayer));
            iPlayer = NULL;

            delete iDataSource;
            iDataSource = NULL;

            delete iDataSinkVideo;
            iDataSinkVideo = NULL;

            delete iDataSinkAudio;
            iDataSinkAudio = NULL;

            delete iDataSinkText;
            iDataSinkText = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeVideo);
            iIONodeVideo = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeAudio);
            iIONodeAudio = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeText);
            iIONodeText = NULL;

            iMioFactory->DestroyVideoOutput(iMOutVideo);
            iMOutVideo = NULL;

            iMioFactory->DestroyAudioOutput(iMOutAudio);
            iMOutAudio = NULL;

            iMioFactory->DestroyTextOutput(iMOutText);
            iMOutText = NULL;

            delete iLocalDataSource;
            iLocalDataSource = NULL;

            //If this is the original instance, then wait on the 2nd instance to finish
            //so we can check its error results and add to this test case results.
            if (!iParentInstance)
            {
                if (!iChildThreadExit)
                {
                    fprintf(file, "Waiting on 2nd instance to exit...\n");
                    for (uint32 i = 0;!iChildThreadExit && i < 20;i++)
                        OsclThread::SleepMillisec(1000);
                }
                //make sure child thread exited within the allowed time
                //and didn't report any test failures, exceptions, or memory leaks.
                PVPATB_TEST_IS_TRUE(iChildThreadExit);
                if (iChildThreadFailures > 0)
                    PVPATB_TEST_IS_TRUE(false);
                if (iChildThreadLeave != OsclErrNone)
                    PVPATB_TEST_IS_TRUE(false);
            }

            iObserver->TestCompleted(*iTestCase);
        }
        break;

        default:
            break;

    }
}


void pvplayer_async_test_multiple_instance::CommandCompleted(const PVCmdResponse& aResponse)
{
    if (aResponse.GetCmdId() != iCurrentCmdId)
    {
        // Wrong command ID.
        MULTI_TEST_IS_TRUE(false);
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
        return;
    }

    if (aResponse.GetContext() != NULL)
    {
        if (aResponse.GetContext() == (OsclAny*)&iContextObject)
        {
            if (iContextObject != iContextObjectRefValue)
            {
                // Context data value was corrupted
                MULTI_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                return;
            }
        }
        else
        {
            // Context data pointer was corrupted
            MULTI_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
            return;
        }
    }

    switch (iState)
    {
        case STATE_ADDDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_INIT;
                RunIfNotReady();
            }
            else
            {
                // AddDataSource failed
                MULTI_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_INIT:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_GETMETADATAKEYLIST;
                RunIfNotReady();
            }
            else
            {
                // Init failed
                MULTI_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GETMETADATAKEYLIST:
            if (aResponse.GetCmdStatus() == PVMFSuccess || aResponse.GetCmdStatus() == PVMFErrArgument)
            {
                iState = STATE_GETMETADATAVALUELIST;
                RunIfNotReady();
            }
            else
            {
                // GetMetadataKeys failed
                MULTI_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GETMETADATAVALUELIST:
            if (aResponse.GetCmdStatus() == PVMFSuccess || aResponse.GetCmdStatus() == PVMFErrArgument)
            {
                //Display the resulting metadata.
                PrintMetadata();
                iState = STATE_ADDDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                // GetMetadataValue failed
                MULTI_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                MULTI_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_TEXT;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                MULTI_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_TEXT:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_PREPARE;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                MULTI_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_PREPARE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_START;
                RunIfNotReady();
            }
            else
            {
                // Prepare failed
                MULTI_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_START:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                //stop in 20 seconds.
                iState = STATE_STOP;
                RunIfNotReady(20000000);
            }
            else
            {
                // Start failed
                MULTI_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_STOP:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                // Stop failed
                MULTI_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                MULTI_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_TEXT;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                MULTI_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_TEXT:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_RESET;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                MULTI_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_RESET:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASOURCE;
                RunIfNotReady();
            }
            else
            {
                // Reset failed
                MULTI_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                MULTI_TEST_IS_TRUE(true);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSource failed
                MULTI_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        default:
        {
            // Testing error if this is reached
            MULTI_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;
    }
}


void pvplayer_async_test_multiple_instance::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    switch (aEvent.GetEventType())
    {
        case PVMFErrResourceConfiguration:
            // Just log the error
            MULTI_TEST_IS_TRUE(false);
            break;

        case PVMFErrResource:
            // Just log the error
            MULTI_TEST_IS_TRUE(false);
            break;

        case PVMFErrCorrupt:
            // Just log the error
            MULTI_TEST_IS_TRUE(false);
            break;

        case PVMFErrProcessing:
            // Just log the error
            MULTI_TEST_IS_TRUE(false);
            break;

        default:
            // Unknown error and just log the error
            MULTI_TEST_IS_TRUE(false);
            break;
    }

    // Wait for engine to handle the error
    Cancel();
}


void pvplayer_async_test_multiple_instance::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingStart)
    {
        // do nothing
    }
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingComplete)
    {
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
    }
}

void pvplayer_async_test_multiple_instance::PrintMetadata()
{
    //to avoid thread contention for the output file, only print metadata
    //from the original instance (with NULL parent).
    if (iParentInstance)
        return;

    for (uint32 i = 0;i < iMetadataValueList.size();i++)
    {
        if (!iMetadataValueList[i].key)
        {
            fprintf(file, "  Metadata Key Missing!, value ?\n");
        }
        else if (oscl_strstr(iMetadataValueList[i].key, "valtype=char*"))
        {
            fprintf(file, "  Metadata Key '%s', value '%s'\n", iMetadataValueList[i].key, iMetadataValueList[i].value.pChar_value);
        }
        else if (oscl_strstr(iMetadataValueList[i].key, "valtype=wchar*"))
        {
            OSCL_HeapString<OsclMemAllocator> ostr;
            char buf[2];
            buf[1] = '\0';
            for (uint32 j = 0;;j++)
            {
                if (iMetadataValueList[i].value.pWChar_value[j] == '\0')
                    break;
                buf[0] = (char)iMetadataValueList[i].value.pWChar_value[j];
                ostr += buf;
            }
            fprintf(file, "  Metadata Key '%s', value '%s'\n", iMetadataValueList[i].key, ostr.get_str());
        }
        else if (oscl_strstr(iMetadataValueList[i].key, "valtype=uint32"))
        {
            fprintf(file, "  Metadata Key '%s', value %d\n", iMetadataValueList[i].key, iMetadataValueList[i].value.uint32_value);
        }
        else if (oscl_strstr(iMetadataValueList[i].key, "valtype=bool"))
        {
            fprintf(file, "  Metadata Key '%s', value %d\n", iMetadataValueList[i].key, iMetadataValueList[i].value.bool_value);
        }
        else if (oscl_strstr(iMetadataValueList[i].key, "valtype=uint8*"))
        {
            fprintf(file, "  Metadata Key '%s', len %d\n", iMetadataValueList[i].key, iMetadataValueList[i].length);
        }
        else
        {
            fprintf(file, "  Metadata Key '%s', value ?\n", iMetadataValueList[i].key);
        }
    }
    fprintf(file, "\n\n");
}

//
// pvplayer_async_test_multiple_thread section
//
#include "pvmf_local_data_source.h"
#include "oscl_scheduler.h"
#include "OMX_Core.h"
#include "oscl_mem_audit.h"



//
#undef MULTI_TEST_IS_TRUE
#define MULTI_TEST_IS_TRUE(x) PVPATB_TEST_IS_TRUE(x);


void pvplayer_async_test_multiple_thread::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}

#include "oscl_init.h"
#include "oscl_mem.h"

static TOsclThreadFuncRet OSCL_THREAD_DECL pvplayer_async_test_multiple_thread_thread(TOsclThreadFuncArg arg)
{
    //parent test is passed in as arg
    pvplayer_async_test_multiple_thread* parent = (pvplayer_async_test_multiple_thread*)arg;

    //Init Oscl.
    OsclSelect select;
    select.iOutputFile = file;//for memory leak output.
    int32 err;
    OsclInit::Init(err, &select);
    if (err)
    {
        parent->iThreadErrors++;
        parent->iThreadReadySem.Signal();
        return 0;
    }

    OSCL_TRY(err,

             //get scheduler
             parent->iThreadScheduler = OsclExecScheduler::Current();

             //create player
             parent->iPlayer = PVPlayerFactory::CreatePlayer(parent, parent, parent);

             //signal parent thread that player is ready to accept commands.
             parent->iThreadReadySem.Signal();

             //launch scheduler.  this call blocks.
             parent->iThreadScheduler->StartScheduler();

             //delete player
             PVPlayerFactory::DeletePlayer(parent->iPlayer);
             parent->iPlayer = NULL;

            ); //OSCL_TRY
    if (err)
        parent->iThreadErrors++;

    //Cleanup Oscl
    OsclInit::Cleanup(err, &select);
    if (err)
        parent->iThreadErrors++;

    parent->iThreadScheduler = NULL;

    //Signal that this thread is cleaned up.
    parent->iThreadExitSem.Signal();

    return 0;
}

void pvplayer_async_test_multiple_thread::Run()
{
    int error = 0;

    switch (iState)
    {
        case STATE_CREATE:
        {
            //Configure the threadsafe queues that are used for engine
            //thread callbacks.
            iThreadSafeCommandQueue.Configure(this);
            iThreadSafeErrorQueue.Configure(this);
            iThreadSafeInfoQueue.Configure(this);

            //Create engine thread
            {
                iThreadReadySem.Create();
                iThreadExitSem.Create();
                iThreadErrors = 0;
                iPlayer = NULL;
                iThreadScheduler = NULL;

                OsclThread thread;
                OsclProcStatus::eOsclProcError  result = thread.Create(pvplayer_async_test_multiple_thread_thread, 0, this);
                if (result != OsclProcStatus::SUCCESS_ERROR)
                {
                    PVPATB_TEST_IS_TRUE(false);
                    iObserver->TestCompleted(*iTestCase);
                    break;
                }
                //wait on engine to be ready to use.
                iThreadReadySem.Wait();
            }

            //player should have been created in the other thread
            if (!iPlayer)
            {
                MULTI_TEST_IS_TRUE(false);
                iObserver->TestCompleted(*iTestCase);
            }
            else
            {
                iState = STATE_ADDDATASOURCE;
                RunIfNotReady();
            }
        }
        break;

        case STATE_ADDDATASOURCE:
        {
            iDataSource = new PVPlayerDataSourceURL;

            OSCL_HeapString<OsclMemAllocator> fname(iFileName);
            oscl_UTF8ToUnicode(fname.get_cstr(), fname.get_size(), iTmpWCharBuffer, 512);
            iFileNameWStr.set(iTmpWCharBuffer, oscl_strlen(iTmpWCharBuffer));
            iDataSource->SetDataSourceURL(iFileNameWStr);
            iDataSource->SetDataSourceFormatType(iFileType);
            iLocalDataSource = new PVMFLocalDataSource();
            iDataSource->SetDataSourceContextData((OsclAny*)iLocalDataSource);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, MULTI_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }

        break;

        case STATE_INIT:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Init((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, MULTI_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_GETMETADATAKEYLIST:
        {
            iMetadataKeyList.clear();
            OSCL_TRY(error, iCurrentCmdId = iPlayer->GetMetadataKeys(iMetadataKeyList, 0, 100, NULL, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, MULTI_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_GETMETADATAVALUELIST:
        {
            iMetadataValueList.clear();
            iNumValues = 0;
            OSCL_TRY(error, iCurrentCmdId = iPlayer->GetMetadataValues(iMetadataKeyList, 0, 100, iNumValues, iMetadataValueList, (OsclAny*) & iContextObject, false));
            OSCL_FIRST_CATCH_ANY(error, MULTI_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_RELEASEMETADATAVALUES:
        {
            PrintMetadata();
            OSCL_TRY(error, iCurrentCmdId = iPlayer->ReleaseMetadataValues(iMetadataValueList, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_VIDEO:
        {
            //Synchronous API test
            {
                PVPlayerState state;
                PVMFStatus status = PVMFFailure;
                OSCL_TRY(error, status = iPlayer->GetPVPlayerStateSync(state););
                OSCL_FIRST_CATCH_ANY(error, MULTI_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
                MULTI_TEST_IS_TRUE(status == PVMFSuccess && state == PVP_STATE_INITIALIZED);
            }

            OSCL_wHeapString<OsclMemAllocator> sinkfilename;
            sinkfilename = OUTPUTNAME_PREPEND_WSTRING;
            sinkfilename += iSinkName;
            if (iCompressedVideo)
            {
                sinkfilename += _STRLIT_WCHAR("compressed_");
            }
            OSCL_wHeapString<OsclMemAllocator> inputfilename;
            RetrieveFilename(iFileNameWStr.get_str(), inputfilename);
            sinkfilename += inputfilename;
            sinkfilename += _STRLIT_WCHAR("_video.dat");

            iMOutVideo = iMioFactory->CreateVideoOutput((OsclAny*) & sinkfilename, MEDIATYPE_VIDEO, iCompressedVideo);
            iIONodeVideo = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMOutVideo);
            iDataSinkVideo = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkVideo)->SetDataSinkNode(iIONodeVideo);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, MULTI_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_AUDIO:
        {
            OSCL_wHeapString<OsclMemAllocator> sinkfilename;
            sinkfilename = OUTPUTNAME_PREPEND_WSTRING;
            sinkfilename += iSinkName;
            if (iCompressedAudio)
            {
                sinkfilename += _STRLIT_WCHAR("compressed_");
            }
            OSCL_wHeapString<OsclMemAllocator> inputfilename;
            RetrieveFilename(iFileNameWStr.get_str(), inputfilename);
            sinkfilename += inputfilename;
            sinkfilename += _STRLIT_WCHAR("_audio.dat");

            iMOutAudio = iMioFactory->CreateAudioOutput((OsclAny*) & sinkfilename, MEDIATYPE_AUDIO, iCompressedAudio);

            iIONodeAudio = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMOutAudio);
            iDataSinkAudio = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkAudio)->SetDataSinkNode(iIONodeAudio);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, MULTI_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_TEXT:
        {
            OSCL_wHeapString<OsclMemAllocator> sinkfilename;
            sinkfilename = OUTPUTNAME_PREPEND_WSTRING;
            sinkfilename += iSinkName;
            OSCL_wHeapString<OsclMemAllocator> inputfilename;
            RetrieveFilename(iFileNameWStr.get_str(), inputfilename);
            sinkfilename += inputfilename;
            sinkfilename += _STRLIT_WCHAR("_text.dat");

            iMOutText = iMioFactory->CreateTextOutput((OsclAny*) & sinkfilename, MEDIATYPE_TEXT);
            iIONodeText = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMOutText);
            iDataSinkText = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkText)->SetDataSinkNode(iIONodeText);


            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkText, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, MULTI_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_PREPARE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Prepare((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, MULTI_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_START:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Start((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, MULTI_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_STOP:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Stop((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, MULTI_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_VIDEO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, MULTI_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_AUDIO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, MULTI_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_TEXT:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkText, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, MULTI_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_RESET:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Reset((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, MULTI_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASOURCE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, MULTI_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_WAIT_FOR_ERROR_HANDLING:
        {
            // Timed out waiting for error handling to complete
            MULTI_TEST_IS_TRUE(false);
            // Forcibly terminate the test
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;

        case STATE_CLEANUPANDCOMPLETE:
        {
            //Engine thread cleanup
            {
                if (iPlayer)
                {
                    //Stop scheduler
                    iThreadScheduler->StopScheduler();

                    //Wait on engine cleanup
                    iThreadExitSem.Wait();

                    //Check for errors.
                    if (iThreadErrors > 0)
                        PVPATB_TEST_IS_TRUE(false);

                    iThreadReadySem.Close();
                    iThreadExitSem.Close();
                }
            }

            delete iDataSource;
            iDataSource = NULL;

            delete iDataSinkVideo;
            iDataSinkVideo = NULL;

            delete iDataSinkAudio;
            iDataSinkAudio = NULL;

            delete iDataSinkText;
            iDataSinkText = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeVideo);
            iIONodeVideo = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeAudio);
            iIONodeAudio = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeText);
            iIONodeText = NULL;

            iMioFactory->DestroyVideoOutput(iMOutVideo);
            iMOutVideo = NULL;

            iMioFactory->DestroyAudioOutput(iMOutAudio);
            iMOutAudio = NULL;

            iMioFactory->DestroyTextOutput(iMOutText);
            iMOutText = NULL;

            delete iLocalDataSource;
            iLocalDataSource = NULL;

            iObserver->TestCompleted(*iTestCase);
        }
        break;

        default:
            break;

    }
}

//data is available on one of the thread-safe queues.
void pvplayer_async_test_multiple_thread::ThreadSafeQueueDataAvailable(ThreadSafeQueue* aQueue)
{
    if (aQueue == &iThreadSafeCommandQueue)
    {
        for (uint32 ndata = 1;ndata;)
        {
            ThreadSafeQueueId id;
            OsclAny* data;
            ndata = iThreadSafeCommandQueue.DeQueue(id, data);
            if (ndata)
            {
                PVCmdResponse* cmd = (PVCmdResponse*)data;
                CommandCompleted(*cmd);
                OSCL_DELETE(cmd);
            }
        }
    }
    if (aQueue == &iThreadSafeErrorQueue)
    {
        for (uint32 ndata = 1;ndata;)
        {
            ThreadSafeQueueId id;
            OsclAny* data;
            ndata = iThreadSafeErrorQueue.DeQueue(id, data);
            if (ndata)
            {
                int32 eventType = (PVMFEventType)data;
                PVAsyncErrorEvent event(eventType);
                HandleErrorEvent(event);
            }
        }
    }
    if (aQueue == &iThreadSafeInfoQueue)
    {
        for (uint32 ndata = 1;ndata;)
        {
            ThreadSafeQueueId id;
            OsclAny* data;
            ndata = iThreadSafeInfoQueue.DeQueue(id, data);
            if (ndata)
            {
                int32 eventType = (PVMFEventType)data;
                PVAsyncInformationalEvent event(eventType);
                HandleInformationalEvent(event);
            }
        }
    }
}

void pvplayer_async_test_multiple_thread::CommandCompleted(const PVCmdResponse& aResponse)
{
    //if this callback is from engine thread, then push it across the thread
    //boundary.
    if (!iThreadSafeCommandQueue.IsInThread())
    {
        PVCmdResponse* cmd = OSCL_NEW(PVCmdResponse, (aResponse));
        iThreadSafeCommandQueue.AddToQueue(cmd);
        return;
    }

    if (aResponse.GetCmdId() != iCurrentCmdId)
    {
        // Wrong command ID.
        MULTI_TEST_IS_TRUE(false);
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
        return;
    }

    if (aResponse.GetContext() != NULL)
    {
        if (aResponse.GetContext() == (OsclAny*)&iContextObject)
        {
            if (iContextObject != iContextObjectRefValue)
            {
                // Context data value was corrupted
                MULTI_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                return;
            }
        }
        else
        {
            // Context data pointer was corrupted
            MULTI_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
            return;
        }
    }

    switch (iState)
    {
        case STATE_ADDDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_INIT;
                RunIfNotReady();
            }
            else
            {
                // AddDataSource failed
                MULTI_TEST_IS_TRUE(false);
                iState = STATE_WAIT_FOR_ERROR_HANDLING;
                RunIfNotReady(5000000);
            }
            break;

        case STATE_INIT:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_GETMETADATAKEYLIST;
                RunIfNotReady();
            }
            else
            {
                // Init failed
                MULTI_TEST_IS_TRUE(false);
                iState = STATE_WAIT_FOR_ERROR_HANDLING;
                RunIfNotReady(5000000);
            }
            break;

        case STATE_GETMETADATAKEYLIST:
            if (aResponse.GetCmdStatus() == PVMFSuccess || aResponse.GetCmdStatus() == PVMFErrArgument)
            {
                iState = STATE_GETMETADATAVALUELIST;
                RunIfNotReady();
            }
            else
            {
                // GetMetadataKeys failed
                MULTI_TEST_IS_TRUE(false);
                iState = STATE_WAIT_FOR_ERROR_HANDLING;
                RunIfNotReady(5000000);
            }
            break;

        case STATE_GETMETADATAVALUELIST:
            if (aResponse.GetCmdStatus() == PVMFSuccess || aResponse.GetCmdStatus() == PVMFErrArgument)
            {
                iState = STATE_RELEASEMETADATAVALUES;
                RunIfNotReady();
            }
            else
            {
                // GetMetadataValue failed
                MULTI_TEST_IS_TRUE(false);
                iState = STATE_WAIT_FOR_ERROR_HANDLING;
                RunIfNotReady(5000000);
            }
            break;

        case STATE_RELEASEMETADATAVALUES:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                // ReleaseMetadataValue failed
                MULTI_TEST_IS_TRUE(false);
                iState = STATE_WAIT_FOR_ERROR_HANDLING;
                RunIfNotReady(5000000);
            }
            break;

        case STATE_ADDDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                MULTI_TEST_IS_TRUE(false);
                iState = STATE_WAIT_FOR_ERROR_HANDLING;
                RunIfNotReady(5000000);
            }
            break;

        case STATE_ADDDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_TEXT;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                MULTI_TEST_IS_TRUE(false);
                iState = STATE_WAIT_FOR_ERROR_HANDLING;
                RunIfNotReady(5000000);
            }
            break;

        case STATE_ADDDATASINK_TEXT:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_PREPARE;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                MULTI_TEST_IS_TRUE(false);
                iState = STATE_WAIT_FOR_ERROR_HANDLING;
                RunIfNotReady(5000000);
            }
            break;

        case STATE_PREPARE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_START;
                RunIfNotReady();
            }
            else
            {
                // Prepare failed
                MULTI_TEST_IS_TRUE(false);
                iState = STATE_WAIT_FOR_ERROR_HANDLING;
                RunIfNotReady(5000000);
            }
            break;

        case STATE_START:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                //stop in 20 seconds.
                iState = STATE_STOP;
                RunIfNotReady(20000000);
            }
            else
            {
                // Start failed
                MULTI_TEST_IS_TRUE(false);
                iState = STATE_WAIT_FOR_ERROR_HANDLING;
                RunIfNotReady(5000000);
            }
            break;

        case STATE_STOP:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                // Stop failed
                MULTI_TEST_IS_TRUE(false);
                iState = STATE_WAIT_FOR_ERROR_HANDLING;
                RunIfNotReady(5000000);
            }
            break;

        case STATE_REMOVEDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                MULTI_TEST_IS_TRUE(false);
                iState = STATE_WAIT_FOR_ERROR_HANDLING;
                RunIfNotReady(5000000);
            }
            break;

        case STATE_REMOVEDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_TEXT;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                MULTI_TEST_IS_TRUE(false);
                iState = STATE_WAIT_FOR_ERROR_HANDLING;
                RunIfNotReady(5000000);
            }
            break;

        case STATE_REMOVEDATASINK_TEXT:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_RESET;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                MULTI_TEST_IS_TRUE(false);
                iState = STATE_WAIT_FOR_ERROR_HANDLING;
                RunIfNotReady(5000000);
            }
            break;

        case STATE_RESET:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASOURCE;
                RunIfNotReady();
            }
            else
            {
                // Reset failed
                MULTI_TEST_IS_TRUE(false);
                iState = STATE_WAIT_FOR_ERROR_HANDLING;
                RunIfNotReady(5000000);
            }
            break;

        case STATE_REMOVEDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                MULTI_TEST_IS_TRUE(true);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSource failed
                MULTI_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        default:
        {
            // Testing error if this is reached
            MULTI_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;
    }
}


void pvplayer_async_test_multiple_thread::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    //if this callback is from engine thread, then push it across the thread
    //boundary.
    if (!iThreadSafeErrorQueue.IsInThread())
    {
        //note: we discard all data except the event type here.
        //if we need more data, memory has to be copied.
        iThreadSafeErrorQueue.AddToQueue((OsclAny*)aEvent.GetEventType());
        return;
    }

    switch (aEvent.GetEventType())
    {
        case PVMFErrResourceConfiguration:
            // Just log the error
            MULTI_TEST_IS_TRUE(false);
            break;

        case PVMFErrResource:
            // Just log the error
            MULTI_TEST_IS_TRUE(false);
            break;

        case PVMFErrCorrupt:
            // Just log the error
            MULTI_TEST_IS_TRUE(false);
            break;

        case PVMFErrProcessing:
            // Just log the error
            MULTI_TEST_IS_TRUE(false);
            break;

        default:
            // Unknown error and just log the error
            MULTI_TEST_IS_TRUE(false);
            break;
    }

    // Wait for engine to handle the error
    Cancel();
    iState = STATE_WAIT_FOR_ERROR_HANDLING;
    RunIfNotReady(5000000);
}


void pvplayer_async_test_multiple_thread::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    //if this callback is from engine thread, then push it across the thread
    //boundary.
    if (!iThreadSafeInfoQueue.IsInThread())
    {
        //note: we discard all data except the event type here.
        //if we need more data, memory has to be copied.
        iThreadSafeInfoQueue.AddToQueue((OsclAny*)aEvent.GetEventType());
        return;
    }

    // Wait for error handling to complete
    if (iState == STATE_WAIT_FOR_ERROR_HANDLING && aEvent.GetEventType() == PVMFInfoErrorHandlingComplete)
    {
        PVPlayerState pstate;
        iPlayer->GetPVPlayerStateSync(pstate);
        switch (pstate)
        {
            case PVP_STATE_INITIALIZED:
                Cancel();
                iState = STATE_REMOVEDATASINK_VIDEO;
                RunIfNotReady();
                break;

            case PVP_STATE_IDLE:
                Cancel();
                iState = STATE_REMOVEDATASOURCE;
                RunIfNotReady();
                break;

            default:
                // Engine should not be in any other state when
                // error handling completes
                MULTI_TEST_IS_TRUE(false);
                Cancel();
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                break;
        }
    }
}

void pvplayer_async_test_multiple_thread::PrintMetadata()
{
    for (uint32 i = 0;i < iMetadataValueList.size();i++)
    {
        if (!iMetadataValueList[i].key)
        {
            fprintf(file, "  Metadata Key Missing!, value ?\n");
        }
        else if (oscl_strstr(iMetadataValueList[i].key, "valtype=char*"))
        {
            fprintf(file, "  Metadata Key '%s', value '%s'\n", iMetadataValueList[i].key, iMetadataValueList[i].value.pChar_value);
        }
        else if (oscl_strstr(iMetadataValueList[i].key, "valtype=wchar*"))
        {
            OSCL_HeapString<OsclMemAllocator> ostr;
            char buf[2];
            buf[1] = '\0';
            for (uint32 j = 0;;j++)
            {
                if (iMetadataValueList[i].value.pWChar_value[j] == '\0')
                    break;
                buf[0] = (char)iMetadataValueList[i].value.pWChar_value[j];
                ostr += buf;
            }
            fprintf(file, "  Metadata Key '%s', value '%s'\n", iMetadataValueList[i].key, ostr.get_str());
        }
        else if (oscl_strstr(iMetadataValueList[i].key, "valtype=uint32"))
        {
            fprintf(file, "  Metadata Key '%s', value %d\n", iMetadataValueList[i].key, iMetadataValueList[i].value.uint32_value);
        }
        else if (oscl_strstr(iMetadataValueList[i].key, "valtype=bool"))
        {
            fprintf(file, "  Metadata Key '%s', value %d\n", iMetadataValueList[i].key, iMetadataValueList[i].value.bool_value);
        }
        else if (oscl_strstr(iMetadataValueList[i].key, "valtype=uint8*"))
        {
            fprintf(file, "  Metadata Key '%s', len %d\n", iMetadataValueList[i].key, iMetadataValueList[i].length);
        }
        else
        {
            fprintf(file, "  Metadata Key '%s', value ?\n", iMetadataValueList[i].key);
        }
    }
    fprintf(file, "\n\n");
}
