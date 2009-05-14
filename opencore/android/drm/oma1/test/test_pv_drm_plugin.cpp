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

#include "test_pv_drm_plugin.h"
#include "oscl_error_codes.h"
#include "oscl_tickcount.h"
#include "pv_player_datasinkpvmfnode.h"
#include "pvmi_media_io_fileoutput.h"
#include "pv_media_output_node_factory.h"
#include "oscl_utf8conv.h"
#include "pvmi_kvp.h"
#include "pvmi_kvp_util.h"
#include "pvmf_timestamp.h"
#include "pvmf_track_level_info_extension.h"
#include "pvmf_errorinfomessage_extension.h"
#include "pvmi_config_and_capability.h"
#include "pvmf_local_data_source.h"
#include "pvmf_cpmplugin_kmj_oma1_factory.h"
#include "pvmf_cpmplugin_kmj_oma1_types.h"
#include "pvmi_media_io_fileoutput.h"
#include "pv_media_output_node_factory.h"
#include "pv_player_engine.h"
#include "pvmf_cpmplugin_kmj_oma1.h"
#include "pvmi_drm_kvp.h"

//
// pvplayer_async_test_drmcpmplugin section
//
void pvplayer_async_test_drmcpmplugin::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
    aLicenseData = NULL;
    bDoGetLicenseProc = false;
    bForceDownloadRights = false;

    if (DRM_DOWNLOAD_RIGHTS_AND_PLAY == iUseMethod)
    {
        bForceDownloadRights = true;
    }
}


void pvplayer_async_test_drmcpmplugin::Run()
{
    int error = 0;

    switch(iState)
    {
        case STATE_CREATE:
        {
            iPlayer = NULL;

            OSCL_TRY(error, iPlayer = PVPlayerFactory::CreatePlayer(this, this, this));
            if (error)
            {
                PVPATB_TEST_IS_TRUE(false);
                iObserver->TestCompleted(*iTestCase);
                break;
            }
            else
            {
                if(PVMFSuccess != PVMFRecognizerRegistry::Init())
                {
                    PVPATB_TEST_IS_TRUE(false);
                    iObserver->TestCompleted(*iTestCase);
                }
                else
                {
                    // Create recognizer plug-in factories
                    iRecognizePluginFactory =
                        (PVMFRecognizerPluginFactory*)OSCL_NEW(PVMFRecognizerPluginFactoryBasic<PVOMA1KMJRecognizerPlugin>, ());

                    if(!iRecognizePluginFactory)
                    {
                        PVPATB_TEST_IS_TRUE(false);
                        iObserver->TestCompleted(*iTestCase);
                        break;
                    }

                    // Register recognizer plug-in factories
                    if ((PVMFSuccess != PVMFRecognizerRegistry::RegisterPlugin(*iRecognizePluginFactory)))
                    {
                        PVPATB_TEST_IS_TRUE(false);
                        iObserver->TestCompleted(*iTestCase);
                        break;
                    }

                    //Connect to plugin registry
                    PVMFStatus status = iPluginRegistryClient.Connect();
                    if(status != PVMFSuccess)
                    {
                        PVPATB_TEST_IS_TRUE(false);
                        iState = STATE_CLEANUPANDCOMPLETE;
                        RunIfNotReady();
                        break;
                    }

                    //Create kmj plugin factory.
                    iPluginFactory = new PVMFOma1KmjPluginFactory();
                    if(!iPluginFactory)
                    {
                        PVPATB_TEST_IS_TRUE(false);
                        iState = STATE_CLEANUPANDCOMPLETE;
                        RunIfNotReady();
                        break;
                    }

                    //Register kmj plugin factory.
                    iPluginMimeType = PVMF_CPM_MIME_KMJ_OMA1;
                    if(iPluginRegistryClient.RegisterPlugin(iPluginMimeType,*iPluginFactory) != PVMFSuccess)
                    {
                        PVPATB_TEST_IS_TRUE(false);
                        iState = STATE_CLEANUPANDCOMPLETE;
                        RunIfNotReady();
                        break;
                    }

                    iState=STATE_ADDDATASOURCE;
                    RunIfNotReady();
                }
            }
        }
        break;

        case STATE_ADDDATASOURCE:
        {
            iDataSource = new PVPlayerDataSourceURL;
            oscl_UTF8ToUnicode(iFileName, oscl_strlen(iFileName),output,512);
            wFileName.set(output,oscl_strlen(output));
            iDataSource->SetDataSourceURL(wFileName);
            iDataSource->SetDataSourceFormatType(iFileType);

            //Create a data source 
            iLocalDataSource = new PVMFLocalDataSource();
            iDataSource->SetDataSourceContextData((OsclAny*)iLocalDataSource);

            //Add the data source
            OSCL_TRY( error,
                      iCurrentCmdId = iPlayer->AddDataSource( *iDataSource,
                                                             (OsclAny*)&iContextObject)
                                                            );
            OSCL_FIRST_CATCH_ANY( error,
                                  PVPATB_TEST_IS_TRUE(false);
                                  iState = STATE_CLEANUPANDCOMPLETE;
                                  RunIfNotReady()
                                 );

            if(DRM_NEW_CD == iUseMethod)
            {
                if(iFileServer.Connect())
                {
                    return ;
                }

                iFileObject = OSCL_NEW(Oscl_File,(4096,iFileHandle));
                if (iFileObject == NULL)
                {
                    return ;
                }

                int32 ret = iFileObject->Open( iFileName,
                                               Oscl_File::MODE_READ | Oscl_File::MODE_BINARY,
                                               iFileServer);

                if (ret)
                {
                    OSCL_DELETE( iFileObject);
                    iFileObject = NULL;
                    return ;
                }

                int32_t mimeType = GetMimeType(iFileName);

                T_DRM_Input_Data drmInputData =
                {
                    (int32_t)iFileObject,
                    mimeType,
                    DrmPluginSTGetFileDataLen,
                    DrmPluginSTReadFileData,
                    DrmPluginSTSeekFileData
                };
                T_DRM_Rights_Info rightsInfo;

                memset(&rightsInfo, 0, sizeof(T_DRM_Rights_Info));
                ret = SVC_drm_installRights(drmInputData, &rightsInfo);

                iFileObject->Close();
                OSCL_DELETE(iFileObject);
                iFileObject = NULL;
                iFileServer.Close();

                if( DRM_SUCCESS != ret)
                {
                    return;
                }
           }
        }
        break;

        case STATE_INIT:
        {
            OSCL_TRY(error, iCurrentCmdId=iPlayer->Init((OsclAny*)&iContextObject));
            OSCL_FIRST_CATCH_ANY( error,
                                  PVPATB_TEST_IS_TRUE(false);
                                  iState = STATE_CLEANUPANDCOMPLETE;
                                  RunIfNotReady()
                                 );
        }
        break;

        case STATE_ACQUIRELICENSE:
        {
            aLicenseData = OSCL_ARRAY_NEW(uint8_t,(1024));

            if(NULL == aLicenseData)
            {
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                PVPlayerEngine* pEngine = OSCL_DYNAMIC_CAST(PVPlayerEngine*,iPlayer);

                if (pEngine)
                {
                    OSCL_TRY(error, iCurrentCmdId=pEngine->AcquireLicense(aLicenseData,
                                                                          1024,
                                                                          iFileName,
                                                                          1000,
                                                                          (OsclAny*)&iContextObject));
                    OSCL_FIRST_CATCH_ANY( error,
                                          PVPATB_TEST_IS_TRUE(false);
                                          iState = STATE_CLEANUPANDCOMPLETE;
                                          RunIfNotReady()
                                         );
                }
                else
                {
                    PVPATB_TEST_IS_TRUE(false);
                    iState = STATE_CLEANUPANDCOMPLETE;
                    RunIfNotReady();
                }
            }
        }
        break;

        // Tests the Ability to get Metadata after the Init State
        case STATE_GETMETADATAKEYS1:
        {
            iKeyList.clear();
            OSCL_TRY(error, iCurrentCmdId = iPlayer->GetMetadataKeys(iKeyList,0,-1,NULL,(OsclAny*)&iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_GETMETADATAVALUES1:
        {
            iValueList.clear();
            iNumAvailableValues = 0;
            OSCL_TRY(error, iCurrentCmdId = iPlayer->GetMetadataValues(iKeyList,0,-1,iNumAvailableValues,iValueList,(OsclAny*)&iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_VIDEO:
        {
            OSCL_wHeapString<OsclMemAllocator> SinkFileName;
            SinkFileName=OUTPUTNAME_PREPEND_WSTRING;
            SinkFileName+=_STRLIT_WCHAR("test_player_cpmopenplaystop_");
            OSCL_wHeapString<OsclMemAllocator> inputfilename;
            RetrieveFilename(wFileName.get_str(), inputfilename);
            SinkFileName+=inputfilename;
            SinkFileName+=_STRLIT_WCHAR("_video.dat");

            iMIOFileOutVideo = PVPlayerTestMioFactory::CreateVideoOutput((OsclAny*)&SinkFileName);
            iIONodeVideo = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutVideo);
            iDataSinkVideo = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkVideo)->SetDataSinkNode(iIONodeVideo);
            ((PVPlayerDataSinkPVMFNode*)iDataSinkVideo)->SetDataSinkFormatType(PVMF_YUV420);

            OSCL_TRY( error,
                      iCurrentCmdId = iPlayer->AddDataSink( *iDataSinkVideo,
                                                            (OsclAny*)&iContextObject)
                                                           );

            OSCL_FIRST_CATCH_ANY( error,
                                  PVPATB_TEST_IS_TRUE(false);
                                  iState = STATE_CLEANUPANDCOMPLETE;
                                  RunIfNotReady()
                                 );
        }
        break;

        case STATE_ADDDATASINK_AUDIO:
        {
            OSCL_wHeapString<OsclMemAllocator> SinkFileName;
            SinkFileName=OUTPUTNAME_PREPEND_WSTRING;
            SinkFileName+=_STRLIT_WCHAR("test_player_cpmopenplaystop_");
            OSCL_wHeapString<OsclMemAllocator> inputfilename;
            RetrieveFilename(wFileName.get_str(), inputfilename);
            SinkFileName+=inputfilename;
            SinkFileName+=_STRLIT_WCHAR("_audio.dat");

            iMIOFileOutAudio = PVPlayerTestMioFactory::CreateAudioOutput((OsclAny*)&SinkFileName);
            iIONodeAudio = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutAudio);
            iDataSinkAudio = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkAudio)->SetDataSinkNode(iIONodeAudio);
            ((PVPlayerDataSinkPVMFNode*)iDataSinkAudio)->SetDataSinkFormatType(AUDIOSINK_FORMAT_TYPE);

            OSCL_TRY( error,
                      iCurrentCmdId = iPlayer->AddDataSink( *iDataSinkAudio,
                                                            (OsclAny*)&iContextObject)
                                                          );

            OSCL_FIRST_CATCH_ANY( error,
                                  PVPATB_TEST_IS_TRUE(false);
                                  iState = STATE_CLEANUPANDCOMPLETE;
                                  RunIfNotReady()
                                 );
        }
        break;

        case STATE_ADDDATASINK_TEXT:
        {
            OSCL_wHeapString<OsclMemAllocator> SinkFileName;
            SinkFileName=OUTPUTNAME_PREPEND_WSTRING;
            SinkFileName+=_STRLIT_WCHAR("test_player_cpmopenplaystop_");
            OSCL_wHeapString<OsclMemAllocator> inputfilename;
            RetrieveFilename(wFileName.get_str(), inputfilename);
            SinkFileName+=inputfilename;
            SinkFileName+=_STRLIT_WCHAR("_text.dat");

            iMIOFileOutText = PVPlayerTestMioFactory::CreateAudioOutput((OsclAny*)&SinkFileName);
            iIONodeText = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutText);
            iDataSinkText = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkText)->SetDataSinkNode(iIONodeText);
            ((PVPlayerDataSinkPVMFNode*)iDataSinkText)->SetDataSinkFormatType(PVMF_3GPP_TIMEDTEXT);

            OSCL_TRY( error,
                      iCurrentCmdId=iPlayer->AddDataSink( *iDataSinkText,
                                                          (OsclAny*)&iContextObject)
                                                        );

            OSCL_FIRST_CATCH_ANY( error,
                                  PVPATB_TEST_IS_TRUE(false);
                                  iState=STATE_CLEANUPANDCOMPLETE;
                                  RunIfNotReady()
                                );
        }
        break;

        case STATE_PREPARE:
        {
            OSCL_TRY(error, iCurrentCmdId=iPlayer->Prepare((OsclAny*)&iContextObject));
            OSCL_FIRST_CATCH_ANY( error,
                                  PVPATB_TEST_IS_TRUE(false);
                                  iState=STATE_CLEANUPANDCOMPLETE;
                                  RunIfNotReady()
                                 );
        }
        break;

        case STATE_START:
        {
            OSCL_TRY(error, iCurrentCmdId=iPlayer->Start((OsclAny*)&iContextObject));
            OSCL_FIRST_CATCH_ANY( error,
                                  PVPATB_TEST_IS_TRUE(false);
                                  iState=STATE_CLEANUPANDCOMPLETE;
                                  RunIfNotReady()
                                 );
        }
        break;

        case STATE_STOP:
        {
            OSCL_TRY(error, iCurrentCmdId=iPlayer->Stop((OsclAny*)&iContextObject));
            OSCL_FIRST_CATCH_ANY( error,
                                  PVPATB_TEST_IS_TRUE(false);
                                  iState=STATE_CLEANUPANDCOMPLETE;
                                  RunIfNotReady()
                                 );
        }
        break;

        case STATE_REMOVEDATASINK_VIDEO:
        {
            OSCL_TRY( error,
                      iCurrentCmdId=iPlayer->RemoveDataSink( *iDataSinkVideo,
                                                            (OsclAny*)&iContextObject)
                                                           );

            OSCL_FIRST_CATCH_ANY( error,
                                  PVPATB_TEST_IS_TRUE(false);
                                  iState=STATE_CLEANUPANDCOMPLETE;
                                  RunIfNotReady()
                                 );
        }
        break;

        case STATE_REMOVEDATASINK_AUDIO:
        {
            OSCL_TRY( error,
                      iCurrentCmdId=iPlayer->RemoveDataSink( *iDataSinkAudio,
                                                             (OsclAny*)&iContextObject)
                                                           );

            OSCL_FIRST_CATCH_ANY( error,
                                  PVPATB_TEST_IS_TRUE(false);
                                  iState=STATE_CLEANUPANDCOMPLETE;
                                  RunIfNotReady()
                                 );
        }
        break;

        case STATE_REMOVEDATASINK_TEXT:
        {
            OSCL_TRY( error,
                      iCurrentCmdId=iPlayer->RemoveDataSink( *iDataSinkText,
                      (OsclAny*)&iContextObject)
                    );

            OSCL_FIRST_CATCH_ANY( error,
                                  PVPATB_TEST_IS_TRUE(false);
                                  iState=STATE_CLEANUPANDCOMPLETE;
                                  RunIfNotReady()
                                 );
        }
        break;

        case STATE_RESET:
        {
            OSCL_TRY(error, iCurrentCmdId=iPlayer->Reset((OsclAny*)&iContextObject));
            OSCL_FIRST_CATCH_ANY( error,
                                  PVPATB_TEST_IS_TRUE(false);
                                  iState=STATE_CLEANUPANDCOMPLETE;
                                  RunIfNotReady()
                                 );
        }
        break;

        case STATE_REMOVEDATASOURCE:
        {
            OSCL_TRY( error,
                      iCurrentCmdId=iPlayer->RemoveDataSource( *iDataSource,
                                                               (OsclAny*)&iContextObject)
                                                             );

            OSCL_FIRST_CATCH_ANY( error,
                                  PVPATB_TEST_IS_TRUE(false);
                                  iState=STATE_CLEANUPANDCOMPLETE;
                                  RunIfNotReady()
                                );
        }
        break;

        case STATE_WAIT_FOR_ERROR_HANDLING:
        {
            // Timed out waiting for error handling to complete
            PVPATB_TEST_IS_TRUE(false);

            // Forcibly terminate the test
            iState=STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;

        case STATE_CLEANUPANDCOMPLETE:
        {
            PVPATB_TEST_IS_TRUE(PVPlayerFactory::DeletePlayer(iPlayer));
            iPlayer=NULL;

            delete iDataSource;
            iDataSource=NULL;

            delete iDataSinkVideo;
            iDataSinkVideo = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeVideo);
            iIONodeVideo=NULL;

            PVPlayerTestMioFactory::DestroyVideoOutput(iMIOFileOutVideo);
            iMIOFileOutVideo=NULL;

            delete iDataSinkAudio;
            iDataSinkAudio=NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeAudio);
            iIONodeAudio=NULL;

            PVPlayerTestMioFactory::DestroyAudioOutput(iMIOFileOutAudio);
            iMIOFileOutAudio=NULL;

            delete iDataSinkText;
            iDataSinkText=NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeText);
            iIONodeText=NULL;

            PVPlayerTestMioFactory::DestroyTextOutput(iMIOFileOutText);
            iMIOFileOutText=NULL;

            //close and cleanup the CPM plugin registry.
            iPluginRegistryClient.Close();

            //delete the plugin factory.
            if(iPluginFactory)
            {
                delete iPluginFactory;
                iPluginFactory=NULL;
            }

            delete iLocalDataSource;
            iLocalDataSource=NULL;

            if(aLicenseData)
            {
                OSCL_ARRAY_DELETE(aLicenseData);
                aLicenseData = NULL;
            }

            if (bDoGetLicenseProc)
            {
                bDoGetLicenseProc = false;
                iState=STATE_CREATE;
                RunIfNotReady();
            }
            else
            {
                iObserver->TestCompleted(*iTestCase);
            }

            // Unregister the plug-ins
            PVMFRecognizerRegistry::RemovePlugin(*iRecognizePluginFactory);

            // Destroy the plug-in factory instances
            if (iRecognizePluginFactory)
            {
                OSCL_TEMPLATED_DELETE(((PVMFRecognizerPluginFactoryBasic<PVOMA1KMJRecognizerPlugin>*)iRecognizePluginFactory),
                    PVMFRecognizerPluginFactoryBasic<PVOMA1KMJRecognizerPlugin>, PVMFRecognizerPluginFactoryBasic);
                iRecognizePluginFactory = NULL;
            }

            PVMFRecognizerRegistry::Cleanup();
        }
        break;

    default:
        break;

    }
}

void pvplayer_async_test_drmcpmplugin::CommandCompleted(const PVCmdResponse& aResponse)
{
    if (aResponse.GetCmdId()!=iCurrentCmdId)
    {
        // Wrong command ID.
        PVPATB_TEST_IS_TRUE(false);
        iState=STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
        return;
    }

    if (aResponse.GetContext()!=NULL)
    {
        if (aResponse.GetContext()==(OsclAny*)&iContextObject)
        {
            if (iContextObject!=iContextObjectRefValue)
            {
                // Context data value was corrupted
                PVPATB_TEST_IS_TRUE(false);
                iState=STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                return;
            }
        }
        else
        {
            // Context data pointer was corrupted
            PVPATB_TEST_IS_TRUE(false);
            iState=STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
            return;
        }
    }

    switch (iState)
    {
        case STATE_ADDDATASOURCE:
            if (aResponse.GetCmdStatus()==PVMFSuccess)
            {
                iState=STATE_INIT;
                RunIfNotReady();
            }
            else
            {
                // AddDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState=STATE_WAIT_FOR_ERROR_HANDLING;
                RunIfNotReady(3000000);
            }
            break;

        case STATE_INIT:
            if (aResponse.GetCmdStatus()==PVMFSuccess)
            {
                if (bForceDownloadRights)
                {
                	  bForceDownloadRights = false;
                    iState=STATE_ACQUIRELICENSE;
                }
                else
                {
                    iState=STATE_GETMETADATAKEYS1;

                    SavePureMediaForCompare();
                }

                RunIfNotReady();
            }
            else if (aResponse.GetCmdStatus()==PVMFErrLicenseRequired)
            {
                if (DRM_DOWNLOAD_RIGHTS_AND_PLAY == iUseMethod)
                {
                	  if(true == bForceDownloadRights)
                	 	{
                        bForceDownloadRights = false;
                        iState=STATE_ACQUIRELICENSE;
                        RunIfNotReady();
                    }
                    else
                    {
                        PVPATB_TEST_IS_TRUE(false);
                        iState=STATE_WAIT_FOR_ERROR_HANDLING;
                        RunIfNotReady(3000000);
                    }
                }
                else
                {
                    PVPATB_TEST_IS_TRUE(false);
                    iState=STATE_WAIT_FOR_ERROR_HANDLING;
                    RunIfNotReady(3000000);
                }
            }
            else
            {
                // Init failed
                PVPATB_TEST_IS_TRUE(false);
                iState=STATE_WAIT_FOR_ERROR_HANDLING;
                RunIfNotReady(3000000);
            }
            break;

        case STATE_ACQUIRELICENSE:
            if (aResponse.GetCmdStatus()==PVMFSuccess)
            {
                SavePureMediaForCompare();

                if (bForceDownloadRights)
                {
                    bForceDownloadRights = false;
                    iState=STATE_GETMETADATAKEYS1;
                }
                else
                {
                    bDoGetLicenseProc = true;
                    iState=STATE_CLEANUPANDCOMPLETE;
                }

                RunIfNotReady();
            }
            else if (aResponse.GetCmdStatus()==PVMFErrResource)
            {
                // rights downloaded are error
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_WAIT_FOR_ERROR_HANDLING;
                RunIfNotReady(3000000);
            }
            else
            {
                // Acquire license failed
                PVPATB_TEST_IS_TRUE(false);
                iState=STATE_WAIT_FOR_ERROR_HANDLING;
                RunIfNotReady(3000000);
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
                iState = STATE_WAIT_FOR_ERROR_HANDLING;
                RunIfNotReady(3000000);
            }
            break;

        case STATE_GETMETADATAVALUES1:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                /* save the media object data to a temp file for ST,
                   and this file will be remove by ST script. */
                int count = iValueList.size();

                for (int i = 0; i < count; i++)
                {
                    if (0 == strcmp(PVMF_DRM_INFO_ENVELOPE_DATA_VALUE, iValueList[i].key))
                    {
                        FILE *handle = fopen("/data/drm/drm1_core_files/tmp1.drm", "wb");

                        if (NULL != handle)
                        {
                            if (iValueList[i].length > 0 && iValueList[i].value.pUint8_value != NULL)
                            {
                                fwrite(iValueList[i].value.pUint8_value, iValueList[i].length, 1, handle);
                            }

                            fclose(handle);
                        }
                        break;
                    }
                }

                PVPATB_TEST_IS_TRUE(true);
                iState = STATE_ADDDATASINK_VIDEO;
                RunIfNotReady();
                break;
            }
            else
            {
                // GetMetadataValues failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_WAIT_FOR_ERROR_HANDLING;
                RunIfNotReady(3000000);
            }
            break;

        case STATE_ADDDATASINK_VIDEO:
            if (aResponse.GetCmdStatus()==PVMFSuccess)
            {
                iState=STATE_ADDDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState=STATE_WAIT_FOR_ERROR_HANDLING;
                RunIfNotReady(3000000);
            }
            break;

        case STATE_ADDDATASINK_AUDIO:
            if (aResponse.GetCmdStatus()==PVMFSuccess)
            {
                iState=STATE_ADDDATASINK_TEXT;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState=STATE_WAIT_FOR_ERROR_HANDLING;
                RunIfNotReady(3000000);
            }
            break;

        case STATE_ADDDATASINK_TEXT:
            if (aResponse.GetCmdStatus()==PVMFSuccess)
            {
                iState=STATE_PREPARE;
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState=STATE_WAIT_FOR_ERROR_HANDLING;
                RunIfNotReady(3000000);
            }
            break;

        case STATE_PREPARE:
            if (aResponse.GetCmdStatus()==PVMFSuccess)
            {
                iState=STATE_START;
                RunIfNotReady();
            }
            else
            {
                // Prepare failed
                PVPATB_TEST_IS_TRUE(false);
                iState=STATE_WAIT_FOR_ERROR_HANDLING;
                RunIfNotReady(3000000);
            }
            break;

        case STATE_START:
            if (aResponse.GetCmdStatus()==PVMFSuccess)
            {
                iState=STATE_STOP;
                RunIfNotReady(5000000);
            }
            else
            {
                // Start failed
                PVPATB_TEST_IS_TRUE(false);
                iState=STATE_WAIT_FOR_ERROR_HANDLING;
                RunIfNotReady(3000000);
            }
            break;

        case STATE_STOP:
            if (aResponse.GetCmdStatus()==PVMFSuccess)
            {
                iState=STATE_REMOVEDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                // Stop failed
                PVPATB_TEST_IS_TRUE(false);
                iState=STATE_WAIT_FOR_ERROR_HANDLING;
                RunIfNotReady(3000000);
            }
            break;

        case STATE_REMOVEDATASINK_VIDEO:
            if (aResponse.GetCmdStatus()==PVMFSuccess)
            {
                iState=STATE_REMOVEDATASINK_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState=STATE_WAIT_FOR_ERROR_HANDLING;
                RunIfNotReady(3000000);
            }
            break;

        case STATE_REMOVEDATASINK_AUDIO:
            if (aResponse.GetCmdStatus()==PVMFSuccess)
            {
                iState=STATE_REMOVEDATASINK_TEXT;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState=STATE_WAIT_FOR_ERROR_HANDLING;
                RunIfNotReady(3000000);
            }
            break;

        case STATE_REMOVEDATASINK_TEXT:
            if (aResponse.GetCmdStatus()==PVMFSuccess)
            {
                iState=STATE_RESET;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState=STATE_WAIT_FOR_ERROR_HANDLING;
                RunIfNotReady(3000000);
            }
            break;

        case STATE_RESET:
            if (aResponse.GetCmdStatus()==PVMFSuccess)
            {
                iState=STATE_REMOVEDATASOURCE;
                RunIfNotReady();
            }
            else
            {
                // Reset failed
                PVPATB_TEST_IS_TRUE(false);
                iState=STATE_WAIT_FOR_ERROR_HANDLING;
                RunIfNotReady(3000000);
            }
            break;

        case STATE_REMOVEDATASOURCE:
            if (aResponse.GetCmdStatus()==PVMFSuccess)
            {
                PVPATB_TEST_IS_TRUE(true);
                iState=STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState=STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        default:
        {
            // Testing error if this is reached
            PVPATB_TEST_IS_TRUE(false);
            iState=STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;
    }
}


void pvplayer_async_test_drmcpmplugin::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    switch(aEvent.GetEventType())
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
    iState=STATE_WAIT_FOR_ERROR_HANDLING;
    RunIfNotReady(3000000);
}


void pvplayer_async_test_drmcpmplugin::HandleInformationalEvent(
                             const PVAsyncInformationalEvent& aEvent)
{
    // Wait for error handling to complete
    if (iState==STATE_WAIT_FOR_ERROR_HANDLING
        && aEvent.GetEventType()==PVMFInfoErrorHandlingComplete)
    {
        PVPlayerState pstate;
        iPlayer->GetPVPlayerStateSync(pstate);

        switch(pstate)
        {
            case PVP_STATE_INITIALIZED:
                Cancel();
                iState=STATE_REMOVEDATASINK_VIDEO;
                RunIfNotReady();
                break;

            case PVP_STATE_IDLE:
                Cancel();
                iState=STATE_REMOVEDATASOURCE;
                RunIfNotReady();
                break;

            default:
                // Engine should not be in any other state when error handling completes
                PVPATB_TEST_IS_TRUE(false);
                Cancel();
                iState=STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                break;
        }
    }

    if (aEvent.GetEventType()==PVMFInfoPositionStatus)
    {
        PVInterface* iface=(PVInterface*)(aEvent.GetEventExtensionInterface());
        if(iface==NULL)
        {
            return;
        }

        PVUuid infomsguuid=PVMFErrorInfoMessageInterfaceUUID;
        PVMFErrorInfoMessageInterface* infomsgiface=NULL;

        if (iface->queryInterface(infomsguuid, (PVInterface*&)infomsgiface)==true)
        {
            int32 infocode;
            PVUuid infouuid;
            infomsgiface->GetCodeUUID(infocode, infouuid);

            if ((infouuid==PVPlayerErrorInfoEventTypesUUID)
                && (infocode==PVPlayerInfoPlaybackPositionStatus))
            {
                PVPPlaybackPosition aPos1;
                uint32 aPos2;

                aPos1.iPosUnit=PVPPBPOSUNIT_MILLISEC;
                iPlayer->GetCurrentPositionSync(aPos1);

                uint8* localbuf=aEvent.GetLocalBuffer();

                if (aEvent.GetLocalBufferSize()==8 && localbuf[0]==1)
                {
                    oscl_memcpy(&aPos2, &localbuf[4], sizeof(uint32));
                }
                else
                {
                    aPos2=0;
                }
            }
        }
    }
}

int32_t pvplayer_async_test_drmcpmplugin::GetMimeType(char* drmFileName)
{
    if(!drmFileName)
    {
        return TYPE_DRM_UNKNOWN;
    }

    const char* extension = strrchr(drmFileName, '.');

    if(!extension)
    {
        return TYPE_DRM_UNKNOWN;
    }

    if (strcasecmp(extension, ".dm") == 0)
    {
        return TYPE_DRM_MESSAGE;
    }
    else if (strcasecmp(extension, ".dcf") == 0)
    {
        return TYPE_DRM_CONTENT;
    }
    else if (strcasecmp(extension, ".dr") == 0)
    {
        return TYPE_DRM_RIGHTS_XML;
    }
    else if (strcasecmp(extension, ".drc") == 0)
    {
        return TYPE_DRM_RIGHTS_WBXML;
    }
    else
    {
        return TYPE_DRM_UNKNOWN;
    }
}

void pvplayer_async_test_drmcpmplugin::SavePureMediaForCompare(void)
{
  if(iFileServer.Connect())
    {
        return ;
    }

    iFileObject = OSCL_NEW(Oscl_File,(4096,iFileHandle));
    if (iFileObject == NULL)
    {
    iFileServer.Close();
        return ;
    }

    int32 ret = iFileObject->Open( iFileName,
                                   Oscl_File::MODE_READ | Oscl_File::MODE_BINARY,
                                   iFileServer);

    if (ret)
    {
    iFileServer.Close();
        OSCL_DELETE( iFileObject);
        iFileObject = NULL;
        return ;
    }

    int32_t mimeType = GetMimeType(iFileName);

    T_DRM_Input_Data drmInputData =
    {
        (int32_t)iFileObject,
        mimeType,
        DrmPluginSTGetFileDataLen,
        DrmPluginSTReadFileData,
        DrmPluginSTSeekFileData
    };

    drmSession = SVC_drm_openSession(drmInputData);

    if((DRM_MEDIA_DATA_INVALID == drmSession) || (DRM_FAILURE == drmSession))
    {
    iFileObject->Close();
      OSCL_DELETE(iFileObject);
      iFileObject = NULL;
      iFileServer.Close();
        return ;
    }

  int32_t bufLen = SVC_drm_getContentLength(drmSession);

    if (bufLen > 0)
    {
        uint8_t *buf = OSCL_ARRAY_NEW(uint8_t, (bufLen));
        if (buf)
        {
            if (DRM_SUCCESS == SVC_drm_getContent(drmSession,0, buf, bufLen))
            {
                FILE *handle = fopen(DRM_MEDIA_FILE_FOR_COMPARE, "wb");

                if (handle)
                {
                    fwrite(buf, bufLen, 1, handle);
                    fclose(handle);
                }
            }

            OSCL_FREE(buf);
        }
    }

  iFileObject->Close();
    OSCL_DELETE(iFileObject);
    iFileObject = NULL;
    iFileServer.Close();
}

int32_t DrmPluginSTGetFileDataLen(int32_t fileHandle)
{
    Oscl_File* fileObject = (Oscl_File*)fileHandle;
    return fileObject->Size();
}

int32_t DrmPluginSTSeekFileData( int32_t fileHandle,int32_t offset)
{
    Oscl_File* fileObject = (Oscl_File*)fileHandle;
    return fileObject->Seek(offset,Oscl_File::SEEKSET);
}

int32_t DrmPluginSTReadFileData( int32_t fileHandle,uint8_t* buf,int32_t bufLen)
{
    Oscl_File* fileObject = (Oscl_File*)fileHandle;
    return fileObject->Read(buf,sizeof(uint8_t),bufLen);
}
