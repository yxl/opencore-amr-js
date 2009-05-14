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
#ifndef TEST_PV_PLAYER_ENGINE_TESTSET8_H_INCLUDED
#include "test_pv_player_engine_testset8.h"
#endif

#ifndef OSCL_ERROR_CODES_H_INCLUDED
#include "oscl_error_codes.h"
#endif

#ifndef OSCL_TICKCOUNT_H_INCLUDED
#include "oscl_tickcount.h"
#endif

#ifndef PV_PLAYER_DATASINKFILENAME_H_INCLUDED
#include "pv_player_datasinkfilename.h"
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

#ifndef PVMF_ERRORINFOMESSAGE_EXTENSION_H_INCLUDED
#include "pvmf_errorinfomessage_extension.h"
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

#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif

#ifndef OSCL_MEM_AUDIT_H_INCLUDED
#include "oscl_mem_audit.h"
#endif

//
// pvplayer_async_test_printmetadata section
//
void pvplayer_async_test_printmetadata::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_printmetadata::Run()
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
            oscl_UTF8ToUnicode(iFileName, oscl_strlen(iFileName), iTmpWCharBuffer, 512);
            iTmpWCharBuffer[511] = '\0';
            iFileNameWStr = SOURCENAME_PREPEND_WSTRING;
            iFileNameWStr += iTmpWCharBuffer;
            /*
             * In case of HTTP URLs always attempt rollover,
             * since we donot know if it is a download or a streaming url
             */
            if (iFileType == PVMF_MIME_DATA_SOURCE_HTTP_URL)
            {
                iSourceContextData = new PVMFSourceContextData();
                iSourceContextData->EnableStreamingSourceContext();
                iSourceContextData->EnableCommonSourceContext();
                PVInterface* sourceContextStream = NULL;

                PVUuid streamingContextUuid(PVMF_SOURCE_CONTEXT_DATA_STREAMING_UUID);
                if (iSourceContextData->queryInterface(streamingContextUuid, sourceContextStream))
                {
                    PVMFSourceContextDataStreaming* streamingContext =
                        OSCL_STATIC_CAST(PVMFSourceContextDataStreaming*, sourceContextStream);
                    streamingContext->iStreamStatsLoggingURL = iFileNameWStr;

                    if (iProxyEnabled)
                    {
                        streamingContext->iProxyName = _STRLIT_WCHAR("");
                        streamingContext->iProxyPort = 8080;
                    }
                }
                PVInterface* sourceContextDownload = NULL;
                iSourceContextData->EnableDownloadHTTPSourceContext();
                PVUuid downloadContextUuid(PVMF_SOURCE_CONTEXT_DATA_DOWNLOAD_HTTP_UUID);
                if (iSourceContextData->queryInterface(downloadContextUuid, sourceContextDownload))
                {
                    //create the opaque data
                    iDownloadProxy = _STRLIT_CHAR("");
                    int32 iDownloadProxyPort = 0;
                    if (iProxyEnabled)
                    {
                        iDownloadProxy = _STRLIT_CHAR("");
                        iDownloadProxyPort = 8080;
                    }
                    iDownloadConfigFilename = OUTPUTNAME_PREPEND_WSTRING;
                    iDownloadConfigFilename += _STRLIT_WCHAR("mydlconfig");
                    iDownloadMaxfilesize = 0x7FFFFFFF;
                    iDownloadFilename = OUTPUTNAME_PREPEND_WSTRING;
                    iDownloadFilename += _STRLIT_WCHAR("test_ftdownload.dl");
                    bool aIsNewSession = true;

                    iSourceContextData->DownloadHTTPData()->bIsNewSession = aIsNewSession;
                    iSourceContextData->DownloadHTTPData()->iConfigFileName = iDownloadConfigFilename;
                    iSourceContextData->DownloadHTTPData()->iDownloadFileName = iDownloadFilename;
                    iSourceContextData->DownloadHTTPData()->iMaxFileSize = iDownloadMaxfilesize;
                    iSourceContextData->DownloadHTTPData()->iPlaybackControl = PVMFSourceContextDataDownloadHTTP::EAsap;
                    iSourceContextData->DownloadHTTPData()->iProxyName = iDownloadProxy;
                    iSourceContextData->DownloadHTTPData()->iProxyPort = iDownloadProxyPort;

                }
                iDataSource->SetDataSourceContextData((OsclAny*)iSourceContextData);

                iDataSource->SetDataSourceFormatType(PVMF_MIME_DATA_SOURCE_MS_HTTP_STREAMING_URL);
                iDataSource->SetAlternateSourceFormatType(PVMF_MIME_DATA_SOURCE_HTTP_URL);
            }
            else
            {
                iDataSource->SetDataSourceFormatType(iFileType);
            }

            iDataSource->SetDataSourceURL(iFileNameWStr);

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

        case STATE_GETMETADATAKEYLIST1:
        {
            iMetadataKeyList.clear();
            OSCL_TRY(error, iCurrentCmdId = iPlayer->GetMetadataKeys(iMetadataKeyList, 0, -1, NULL, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_GETMETADATAVALUELIST1:
        {
            iMetadataValueList.clear();
            iNumValues = 0;
            OSCL_TRY(error, iCurrentCmdId = iPlayer->GetMetadataValues(iMetadataKeyList, 0, -1, iNumValues, iMetadataValueList, (OsclAny*) & iContextObject, !iReleaseMetadataByApp));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_RELEASEMETADATAVALUES1:
        {
            PrintMetadataInfo();
            OSCL_TRY(error, iCurrentCmdId = iPlayer->ReleaseMetadataValues(iMetadataValueList, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_VIDEO:
        {
            OSCL_wHeapString<OsclMemAllocator> videosinkfilename = OUTPUTNAME_PREPEND_WSTRING;
            videosinkfilename += _STRLIT_WCHAR("test_player_printmetadata_video.dat");

            iMIOFileOutVideo = iMioFactory->CreateVideoOutput((OsclAny*) & videosinkfilename, MEDIATYPE_VIDEO, iCompressedVideo);
            iIONodeVideo = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutVideo);
            iDataSinkVideo = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkVideo)->SetDataSinkNode(iIONodeVideo);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_AUDIO:
        {
            OSCL_wHeapString<OsclMemAllocator> audiosinkfilename = OUTPUTNAME_PREPEND_WSTRING;
            audiosinkfilename += _STRLIT_WCHAR("test_player_printmetadata_audio.dat");

            iMIOFileOutAudio = iMioFactory->CreateAudioOutput((OsclAny*) & audiosinkfilename, MEDIATYPE_AUDIO, iCompressedAudio);
            iIONodeAudio = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutAudio);
            iDataSinkAudio = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkAudio)->SetDataSinkNode(iIONodeAudio);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_TEXT:
        {
            OSCL_wHeapString<OsclMemAllocator> textsinkfilename = OUTPUTNAME_PREPEND_WSTRING;
            textsinkfilename += _STRLIT_WCHAR("test_player_printmetadata_text.dat");

            iMIOFileOutText = iMioFactory->CreateTextOutput((OsclAny*) & textsinkfilename, MEDIATYPE_TEXT);
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

        case STATE_GETMETADATAKEYLIST2:
        {
            iMetadataKeyList.clear();
            OSCL_TRY(error, iCurrentCmdId = iPlayer->GetMetadataKeys(iMetadataKeyList, 0, 50, NULL, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_GETMETADATAVALUELIST2:
        {
            iMetadataValueList.clear();
            iNumValues = 0;
            OSCL_TRY(error, iCurrentCmdId = iPlayer->GetMetadataValues(iMetadataKeyList, 0, 50, iNumValues, iMetadataValueList, (OsclAny*) & iContextObject, !iReleaseMetadataByApp));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_RELEASEMETADATAVALUES2:
        {
            PrintMetadataInfo();
            OSCL_TRY(error, iCurrentCmdId = iPlayer->ReleaseMetadataValues(iMetadataValueList, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_STOP:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Stop((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_GETMETADATAKEYLIST3:
        {
            iMetadataKeyList.clear();
            OSCL_TRY(error, iCurrentCmdId = iPlayer->GetMetadataKeys(iMetadataKeyList, 0, 50, NULL, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_GETMETADATAVALUELIST3:
        {
            iMetadataValueList.clear();
            iNumValues = 0;
            OSCL_TRY(error, iCurrentCmdId = iPlayer->GetMetadataValues(iMetadataKeyList, 0, 50, iNumValues, iMetadataValueList, (OsclAny*) & iContextObject, !iReleaseMetadataByApp));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_RELEASEMETADATAVALUES3:
        {
            PrintMetadataInfo();
            OSCL_TRY(error, iCurrentCmdId = iPlayer->ReleaseMetadataValues(iMetadataValueList, (OsclAny*) & iContextObject));
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

            iMioFactory->DestroyVideoOutput(iMIOFileOutVideo);
            iMIOFileOutVideo = NULL;

            iMioFactory->DestroyAudioOutput(iMIOFileOutAudio);
            iMIOFileOutAudio = NULL;

            iMioFactory->DestroyTextOutput(iMIOFileOutText);
            iMIOFileOutText = NULL;

            iObserver->TestCompleted(*iTestCase);
        }
        break;

        default:
            break;

    }
}


void pvplayer_async_test_printmetadata::CommandCompleted(const PVCmdResponse& aResponse)
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
                iState = STATE_GETMETADATAKEYLIST1;
                // Uncomment following to skip the first metadata retrieval
                //iState=STATE_ADDDATASINK_VIDEO;
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

        case STATE_GETMETADATAKEYLIST1:
            if (aResponse.GetCmdStatus() == PVMFSuccess || aResponse.GetCmdStatus() == PVMFErrArgument)
            {
                iState = STATE_GETMETADATAVALUELIST1;
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

        case STATE_GETMETADATAVALUELIST1:
            if (aResponse.GetCmdStatus() == PVMFSuccess || aResponse.GetCmdStatus() == PVMFErrArgument)
            {
                fprintf(iTestMsgOutputFile, "After Init()\n");
                if (iReleaseMetadataByApp)
                {
                    iState = STATE_RELEASEMETADATAVALUES1;
                }
                else
                {
                    PrintMetadataInfo();
                    iState = STATE_ADDDATASINK_VIDEO;
                }
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

        case STATE_RELEASEMETADATAVALUES1:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                // ReleaseMetadataValues failed
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
                iState = STATE_GETMETADATAKEYLIST2;
                // Uncomment following to skip the second metadata retrieval
                //iState=STATE_STOP;
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

        case STATE_GETMETADATAKEYLIST2:
            if (aResponse.GetCmdStatus() == PVMFSuccess || aResponse.GetCmdStatus() == PVMFErrArgument)
            {
                iState = STATE_GETMETADATAVALUELIST2;
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

        case STATE_GETMETADATAVALUELIST2:
            if (aResponse.GetCmdStatus() == PVMFSuccess || aResponse.GetCmdStatus() == PVMFErrArgument)
            {
                fprintf(iTestMsgOutputFile, "After Start()\n");
                if (iReleaseMetadataByApp)
                {
                    iState = STATE_RELEASEMETADATAVALUES2;
                }
                else
                {
                    PrintMetadataInfo();
                    iState = STATE_STOP;
                }
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

        case STATE_RELEASEMETADATAVALUES2:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_STOP;
                RunIfNotReady();
            }
            else
            {
                // ReleaseMetadataValues failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_STOP:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_GETMETADATAKEYLIST3;
                // Uncomment following to skip the third metadata retrieval
                //iState=STATE_REMOVEDATASINK_VIDEO;
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

        case STATE_GETMETADATAKEYLIST3:
            if (aResponse.GetCmdStatus() == PVMFSuccess || aResponse.GetCmdStatus() == PVMFErrArgument)
            {
                iState = STATE_GETMETADATAVALUELIST3;
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

        case STATE_GETMETADATAVALUELIST3:
            if (aResponse.GetCmdStatus() == PVMFSuccess || aResponse.GetCmdStatus() == PVMFErrArgument)
            {
                fprintf(iTestMsgOutputFile, "After Stop()\n");
                if (iReleaseMetadataByApp)
                {
                    iState = STATE_RELEASEMETADATAVALUES3;
                }
                else
                {
                    PrintMetadataInfo();
                    iState = STATE_REMOVEDATASINK_VIDEO;
                }
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

        case STATE_RELEASEMETADATAVALUES3:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                // ReleaseMetadataValues failed
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


void pvplayer_async_test_printmetadata::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
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


void pvplayer_async_test_printmetadata::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingStart)
    {
        fprintf(iTestMsgOutputFile, "PVMFInfoErrorHandlingStart...\n");
    }
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingComplete)
    {
        fprintf(iTestMsgOutputFile, "PVMFInfoErrorHandlingComplete...\n");
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
    }
}

/* PrintCodecSpecificInfo() prints the codec specific information for
   the respective AUDIO TYPE stream and VIDEO TYPE stream.

   Please note that the usage of numerical constants for transversing
   the bytes should not be treated as hard coded values usage, because
   the starting offset of each filed in codec specific info is fixed and
   these locations are not tunable. That is why these constants are not
   defined under #define.

*/
void pvplayer_async_test_printmetadata::PrintCodecSpecificInfo(char* aData, uint32 aIndex)
{

    if (INDEX_CODEC_SPECIFIC_INFO_AUDIO == aIndex)
    {
        uint16 CodecId = ((*(aData + 0))		 & 0x000000FF)
                         + (((*(aData + 1)) << 8)  & 0x0000FF00);

        fprintf(iTestMsgOutputFile, "	CodecId:%d \n", CodecId);

        uint16 NumChannels = ((*(aData + 2))		 & 0x000000FF)
                             + (((*(aData + 3)) << 8)  & 0x0000FF00);

        fprintf(iTestMsgOutputFile, "	NumChannels:%d \n", NumChannels);

        uint32 SamplesPerSecond = ((*(aData + 4))		  & 0x000000FF)
                                  + (((*(aData + 5)) << 8)  & 0x0000FF00)
                                  + (((*(aData + 6)) << 16) & 0x00FF0000)
                                  + (((*(aData + 7)) << 24) & 0xFF000000);

        fprintf(iTestMsgOutputFile, "	SamplesPerSecond:%d \n", SamplesPerSecond);

        uint32 AvgNumBytesPerSecond = ((*(aData + 8))		   & 0x000000FF)
                                      + (((*(aData + 9)) << 8)   & 0x0000FF00)
                                      + (((*(aData + 10)) << 16) & 0x00FF0000)
                                      + (((*(aData + 11)) << 24) & 0xFF000000);

        fprintf(iTestMsgOutputFile, "	AvgNumBytesPerSecond:%d \n", AvgNumBytesPerSecond);

        uint16 BlockAlignment = ((*(aData + 12))		 & 0x000000FF)
                                + (((*(aData + 13)) << 8)  & 0x0000FF00);

        fprintf(iTestMsgOutputFile, "	BlockAlignment:%d \n", BlockAlignment);

        uint16 BitsPerSample = ((*(aData + 14))		    & 0x000000FF)
                               + (((*(aData + 15)) << 8)  & 0x0000FF00);

        fprintf(iTestMsgOutputFile, "	BitsPerSample:%d \n", BitsPerSample);

        uint16 CodecSpecificDataSize = ((*(aData + 16))		    & 0x000000FF)
                                       + (((*(aData + 17)) << 8)  & 0x0000FF00);

        fprintf(iTestMsgOutputFile, "	CodecSpecificDataSize:%d \n", CodecSpecificDataSize);

        fprintf(iTestMsgOutputFile, "	CodecSpecifcData: \n");
        fprintf(iTestMsgOutputFile, "\t");
        for (uint16 ii = 0; ii < CodecSpecificDataSize; ii++)
        {
            fprintf(iTestMsgOutputFile, "0x%02x ", (*(aData + 18 + ii) & 0xff));

        }
        fprintf(iTestMsgOutputFile, "\n");


    }
    else if (INDEX_CODEC_SPECIFIC_INFO_VIDEO == aIndex)
    {
        uint32 ImageWidth = ((*(aData + 0))		    & 0x000000FF)
                            + (((*(aData + 1)) << 8)  & 0x0000FF00)
                            + (((*(aData + 2)) << 16) & 0x00FF0000)
                            + (((*(aData + 3)) << 24) & 0xFF000000);
        fprintf(iTestMsgOutputFile, "	ImageWidth:%d \n", ImageWidth);

        uint32 Imageheight = ((*(aData + 4))		 & 0x000000FF)
                             + (((*(aData + 5)) << 8)  & 0x0000FF00)
                             + (((*(aData + 6)) << 16) & 0x00FF0000)
                             + (((*(aData + 7)) << 24) & 0xFF000000);
        fprintf(iTestMsgOutputFile, "	Imageheight:%d \n", Imageheight);

        uint8 ReservedFlags = ((*(aData + 8)) & 0x000000FF);
        fprintf(iTestMsgOutputFile, "	ReservedFlags:%d \n", ReservedFlags);

        uint16 FormatDataSize = ((*(aData + 9))		     & 0x000000FF)
                                + (((*(aData + 10)) << 8)  & 0x0000FF00);

        fprintf(iTestMsgOutputFile, "	FormatDataSize:%d \n", FormatDataSize);

        uint32 FormatDataFormatDataSize = ((*(aData + 11))		   & 0x000000FF)
                                          + (((*(aData + 12)) << 8)  & 0x0000FF00)
                                          + (((*(aData + 13)) << 16) & 0x00FF0000)
                                          + (((*(aData + 14)) << 24) & 0xFF000000);
        fprintf(iTestMsgOutputFile, "	FormatDataFormatDataSize:%d \n", FormatDataFormatDataSize);

        uint32 FormatDataImageWidth = ((*(aData + 15))		   & 0x000000FF)
                                      + (((*(aData + 16)) << 8)  & 0x0000FF00)
                                      + (((*(aData + 17)) << 16) & 0x00FF0000)
                                      + (((*(aData + 18)) << 24) & 0xFF000000);
        fprintf(iTestMsgOutputFile, "	FormatDataImageWidth:%d \n", FormatDataImageWidth);

        uint32 FormatDataImageHeight = ((*(aData + 19))			& 0x000000FF)
                                       + (((*(aData + 20)) << 8)  & 0x0000FF00)
                                       + (((*(aData + 21)) << 16) & 0x00FF0000)
                                       + (((*(aData + 22)) << 24) & 0xFF000000);
        fprintf(iTestMsgOutputFile, "	FormatDataImageHeight:%d \n", FormatDataImageHeight);

        uint16 FormatDataReserved = ((*(aData + 23))		 & 0x000000FF)
                                    + (((*(aData + 24)) << 8)  & 0x0000FF00);

        fprintf(iTestMsgOutputFile, "	FormatDataReserved:%d \n", FormatDataReserved);

        uint16 FormatDataBitsPerPixelCount = ((*(aData + 25))		  & 0x000000FF)
                                             + (((*(aData + 26)) << 8)  & 0x0000FF00);

        fprintf(iTestMsgOutputFile, "	FormatDataBitsPerPixelCount:%d \n", FormatDataBitsPerPixelCount);

        uint32 FormatDataCompressionId = ((*(aData + 27))		  & 0x000000FF)
                                         + (((*(aData + 28)) << 8)  & 0x0000FF00)
                                         + (((*(aData + 29)) << 16) & 0x00FF0000)
                                         + (((*(aData + 30)) << 24) & 0xFF000000);

        fprintf(iTestMsgOutputFile, "	FormatDataCompressionId:%d \n", FormatDataCompressionId);

        uint32 FormatDataImageSize = ((*(aData + 31))		  & 0x000000FF)
                                     + (((*(aData + 32)) << 8)  & 0x0000FF00)
                                     + (((*(aData + 33)) << 16) & 0x00FF0000)
                                     + (((*(aData + 34)) << 24) & 0xFF000000);

        fprintf(iTestMsgOutputFile, "	FormatDataImageSize:%d \n", FormatDataImageSize);

        uint32 FormatDataHorizontalPixelsPerMeter = ((*(aData + 35))		 & 0x000000FF)
                + (((*(aData + 36)) << 8)  & 0x0000FF00)
                + (((*(aData + 37)) << 16) & 0x00FF0000)
                + (((*(aData + 38)) << 24) & 0xFF000000);

        fprintf(iTestMsgOutputFile, "	FormatDataHorizontalPixelsPerMeter:%d \n", FormatDataHorizontalPixelsPerMeter);

        uint32 FormatDataVerticalPixelsPerMeter = ((*(aData + 39))		   & 0x000000FF)
                + (((*(aData + 40)) << 8)  & 0x0000FF00)
                + (((*(aData + 41)) << 16) & 0x00FF0000)
                + (((*(aData + 42)) << 24) & 0xFF000000);

        fprintf(iTestMsgOutputFile, "	FormatDataVerticalPixelsPerMeter:%d \n", FormatDataVerticalPixelsPerMeter);

        uint32 FormatDataColorsUsedCount = ((*(aData + 43))		    & 0x000000FF)
                                           + (((*(aData + 44)) << 8)  & 0x0000FF00)
                                           + (((*(aData + 45)) << 16) & 0x00FF0000)
                                           + (((*(aData + 46)) << 24) & 0xFF000000);

        fprintf(iTestMsgOutputFile, "	FormatDataColorsUsedCount:%d \n", FormatDataColorsUsedCount);

        uint32 FormatDataImportantColorsCount = ((*(aData + 47))		 & 0x000000FF)
                                                + (((*(aData + 48)) << 8)  & 0x0000FF00)
                                                + (((*(aData + 49)) << 16) & 0x00FF0000)
                                                + (((*(aData + 50)) << 24) & 0xFF000000);

        fprintf(iTestMsgOutputFile, "	FormatDataImportantColorsCount:%d \n", FormatDataImportantColorsCount);

        fprintf(iTestMsgOutputFile, "	CodecSpecifcData: \n");
        fprintf(iTestMsgOutputFile, "\t");
        for (uint16 ii = 0; ii < (FormatDataSize - SIZE_FORMATDATA_VIDEO); ii++)
        {
            fprintf(iTestMsgOutputFile, "0x%02x ", (*(aData + 51 + ii) & 0xff));

        }
        fprintf(iTestMsgOutputFile, "\n");
    }
    else
    {
        fprintf(iTestMsgOutputFile, "Codec Specific Info with index %d not supported\n", aIndex);

    }

}

PVMFStatus pvplayer_async_test_printmetadata::GetIndexParamValues(const char* aString,
        uint32& aStartIndex,
        uint32& aEndIndex)
{
    /*
     * This parses a string of the form "index=N1...N2" and extracts the integers N1 and N2.
     * If string is of the format "index=N1" then N2=N1
     */
    if (NULL == aString)
    {
        return PVMFErrArgument;
    }

    /* Go to end of "index=" */
    char* pN1string = (char*)aString + 6;
    const char ch = 'd';
    PV_atoi(pN1string, ch, (int32)oscl_strlen(pN1string), (uint32&)aStartIndex);
    const char* pN2string = oscl_strstr(aString, _STRLIT_CHAR("..."));
    if (NULL == pN2string)
    {
        aEndIndex = aStartIndex;
    }
    else
    {
        /* Go to end of "index=N1..." */
        pN2string += 3;
        PV_atoi(pN2string, ch, (int32)oscl_strlen(pN2string), (uint32&)aEndIndex);
    }
    return PVMFSuccess;
}

void pvplayer_async_test_printmetadata::PrintMetadataInfo()
{
    uint32 i = 0, StartIndex = 0, EndIndex = 0, nCnt = 0, jj = 0;
    iCodecSpecificInfoAudioIndex = INDEX_CODEC_SPECIFIC_INFO_UNDEFINED;
    iCodecSpecificInfoVideoIndex = INDEX_CODEC_SPECIFIC_INFO_UNDEFINED;
    CodecSpecificInfo  sCSI[MAX_CODEC_SPECIFIC_INFO_SUPPORTED];
    for (jj = 0; jj < MAX_CODEC_SPECIFIC_INFO_SUPPORTED; jj++)
    {
        sCSI[jj].CodecSpecificInfoIndex		= -1;
        sCSI[jj].MetadataKeyIndex			= -1;
        sCSI[jj].ValueIndex					= -1;
    }

    fprintf(iTestMsgOutputFile, "Metadata key list (count=%d):\n", iMetadataKeyList.size());
    for (i = 0; i < iMetadataKeyList.size(); ++i)
    {
        fprintf(iTestMsgOutputFile, "Key %d: %s\n", (i + 1), iMetadataKeyList[i].get_cstr());
    }

    fprintf(iTestMsgOutputFile, "\nMetadata value list (count=%d):\n", iMetadataValueList.size());
    for (i = 0; i < iMetadataValueList.size(); ++i)
    {

        // Skip the logging of metadata if it is track-info/codec-specific-info, it will require track-info to be extracted first
        // And as the track-info matches, the startindex is extracted and compared to
        // iCodecSpecificInfoAudioIndex/ iCodecSpecificInfoVideoIndex to print the respective codec-specific-info.

        if ((oscl_strncmp(iMetadataValueList[i].key, PVMF_ASF_PARSER_NODE_TRACKINFO_CODEC_DATA_KEY, oscl_strlen(PVMF_ASF_PARSER_NODE_TRACKINFO_CODEC_DATA_KEY)) == 0))
        {
            char *pIndexPtr = (char*)oscl_strstr(iMetadataValueList[i].key, "track-info/codec-specific-info;valtype=uint8*;");
            if (NULL != pIndexPtr)
            {
                pIndexPtr = (char *)oscl_strstr(pIndexPtr, "index=");
                GetIndexParamValues(pIndexPtr, StartIndex, EndIndex);
                sCSI[nCnt].CodecSpecificInfoIndex = StartIndex;
                sCSI[nCnt].MetadataKeyIndex = i;
                sCSI[nCnt].ValueIndex = (i + 1);
                nCnt++;

                if ((int32)StartIndex == iCodecSpecificInfoAudioIndex)
                {
                    fprintf(iTestMsgOutputFile, "Value %d:\n", (i + 1));
                    fprintf(iTestMsgOutputFile, "   Key string: %s\n", iMetadataValueList[sCSI[jj].MetadataKeyIndex].key);
                    PrintCodecSpecificInfo(iMetadataValueList[i].value.pChar_value, INDEX_CODEC_SPECIFIC_INFO_AUDIO);
                }
                else if ((int32)StartIndex == iCodecSpecificInfoVideoIndex)
                {
                    fprintf(iTestMsgOutputFile, "Value %d:\n", (i + 1));
                    fprintf(iTestMsgOutputFile, "   Key string: %s\n", iMetadataValueList[sCSI[jj].MetadataKeyIndex].key);
                    PrintCodecSpecificInfo(iMetadataValueList[i].value.pChar_value, INDEX_CODEC_SPECIFIC_INFO_VIDEO);
                }
            }
            continue;
        }


        fprintf(iTestMsgOutputFile, "Value %d:\n", (i + 1));
        fprintf(iTestMsgOutputFile, "   Key string: %s\n", iMetadataValueList[i].key);

        switch (GetValTypeFromKeyString(iMetadataValueList[i].key))
        {
            case PVMI_KVPVALTYPE_CHARPTR:

            {
                fprintf(iTestMsgOutputFile, "   Value:%s\n", iMetadataValueList[i].value.pChar_value);
                fprintf(iTestMsgOutputFile, "   Length:%d  Capacity:%d\n", iMetadataValueList[i].length, iMetadataValueList[i].capacity);

                // Checks whether the iMetadataValueList[i].key matches the track-info/type.
                // If matches, then it checks for the index value, extracts the index and stores it in startindex.
                // Then it verifies for WMA/WMV and accordingly prints the codec-specific-info.
                // And if it does not match then it continues till it matches.

                char *pIndexPtr = (char *)oscl_strstr(iMetadataValueList[i].key, "track-info/type;valtype=char*;");
                if (NULL != pIndexPtr)
                {
                    pIndexPtr = (char *)oscl_strstr(iMetadataValueList[i].key, "index=");
                    GetIndexParamValues(pIndexPtr, StartIndex, EndIndex);
                    if ((oscl_strncmp(iMetadataValueList[i].value.pChar_value, "audio/x-ms-wma", oscl_strlen("audio/x-ms-wma")) == 0))
                    {
                        iCodecSpecificInfoAudioIndex = (int32)StartIndex;
                        for (uint32 jj = 0; jj <= nCnt; jj++)
                        {
                            if ((uint32)sCSI[jj].CodecSpecificInfoIndex == StartIndex)
                            {
                                fprintf(iTestMsgOutputFile, "Value %d:\n", sCSI[jj].ValueIndex);
                                fprintf(iTestMsgOutputFile, "   Key string: %s\n", iMetadataValueList[sCSI[jj].MetadataKeyIndex].key);
                                fprintf(iTestMsgOutputFile, "   Value: UNKNOWN VALUE TYPE\n");
                                fprintf(iTestMsgOutputFile, "   Length:%d  Capacity:%d\n", iMetadataValueList[sCSI[jj].MetadataKeyIndex].length, iMetadataValueList[sCSI[jj].MetadataKeyIndex].capacity);
                                PrintCodecSpecificInfo(iMetadataValueList[sCSI[jj].MetadataKeyIndex].value.pChar_value, INDEX_CODEC_SPECIFIC_INFO_AUDIO);
                                break;
                            }
                        }
                    }

                    else if ((oscl_strncmp(iMetadataValueList[i].value.pChar_value, "video/x-ms-wmv", oscl_strlen("video/x-ms-wmv")) == 0))
                    {
                        iCodecSpecificInfoVideoIndex = StartIndex;
                        for (uint jj = 0; jj <= nCnt; jj++)
                        {
                            if ((uint32)sCSI[jj].CodecSpecificInfoIndex == StartIndex)
                            {
                                fprintf(iTestMsgOutputFile, "Value %d:\n", sCSI[jj].ValueIndex);
                                fprintf(iTestMsgOutputFile, "   Key string: %s\n", iMetadataValueList[sCSI[jj].MetadataKeyIndex].key);
                                fprintf(iTestMsgOutputFile, "   Value: UNKNOWN VALUE TYPE\n");
                                fprintf(iTestMsgOutputFile, "   Length:%d  Capacity:%d\n", iMetadataValueList[sCSI[jj].MetadataKeyIndex].length, iMetadataValueList[sCSI[jj].MetadataKeyIndex].capacity);
                                PrintCodecSpecificInfo(iMetadataValueList[sCSI[jj].MetadataKeyIndex].value.pChar_value, INDEX_CODEC_SPECIFIC_INFO_VIDEO);
                                break;
                            }
                        }
                    }

                }

            }
            break;

            case PVMI_KVPVALTYPE_WCHARPTR:
            {
                // Assume string is in UCS-2 encoding so convert to UTF-8
                char tmpstr[65];
                oscl_UnicodeToUTF8(iMetadataValueList[i].value.pWChar_value,
                                   oscl_strlen(iMetadataValueList[i].value.pWChar_value), tmpstr, 65);
                tmpstr[64] = '\0';
                fprintf(iTestMsgOutputFile, "   Value(in UTF-8, first 64 chars):%s\n", tmpstr);
            }
            fprintf(iTestMsgOutputFile, "   Length:%d  Capacity:%d\n", iMetadataValueList[i].length, iMetadataValueList[i].capacity);
            break;

            case PVMI_KVPVALTYPE_UINT32:
                fprintf(iTestMsgOutputFile, "   Value:%d\n", iMetadataValueList[i].value.uint32_value);
                fprintf(iTestMsgOutputFile, "   Length:%d  Capacity:%d\n", iMetadataValueList[i].length, iMetadataValueList[i].capacity);
                break;

            case PVMI_KVPVALTYPE_INT32:
                fprintf(iTestMsgOutputFile, "   Value:%d\n", iMetadataValueList[i].value.int32_value);
                fprintf(iTestMsgOutputFile, "   Length:%d  Capacity:%d\n", iMetadataValueList[i].length, iMetadataValueList[i].capacity);
                break;

            case PVMI_KVPVALTYPE_UINT8:
                fprintf(iTestMsgOutputFile, "   Value:%d\n", iMetadataValueList[i].value.uint8_value);
                fprintf(iTestMsgOutputFile, "   Length:%d  Capacity:%d\n", iMetadataValueList[i].length, iMetadataValueList[i].capacity);
                break;

            case PVMI_KVPVALTYPE_FLOAT:
                fprintf(iTestMsgOutputFile, "   Value:%f\n", iMetadataValueList[i].value.float_value);
                fprintf(iTestMsgOutputFile, "   Length:%d  Capacity:%d\n", iMetadataValueList[i].length, iMetadataValueList[i].capacity);
                break;

            case PVMI_KVPVALTYPE_DOUBLE:
                fprintf(iTestMsgOutputFile, "   Value:%f\n", iMetadataValueList[i].value.double_value);
                fprintf(iTestMsgOutputFile, "   Length:%d  Capacity:%d\n", iMetadataValueList[i].length, iMetadataValueList[i].capacity);
                break;

            case PVMI_KVPVALTYPE_BOOL:
                if (iMetadataValueList[i].value.bool_value)
                {
                    fprintf(iTestMsgOutputFile, "   Value:true(1)\n");
                }
                else
                {
                    fprintf(iTestMsgOutputFile, "   Value:false(0)\n");
                }
                fprintf(iTestMsgOutputFile, "   Length:%d  Capacity:%d\n", iMetadataValueList[i].length, iMetadataValueList[i].capacity);
                break;

            default:
                fprintf(iTestMsgOutputFile, "   Value: UNKNOWN VALUE TYPE\n");
                fprintf(iTestMsgOutputFile, "   Length:%d  Capacity:%d\n", iMetadataValueList[i].length, iMetadataValueList[i].capacity);
                break;
        }


    }

    fprintf(iTestMsgOutputFile, "\n\n");
}


//
// pvplayer_async_test_printmemstats section
//
void pvplayer_async_test_printmemstats::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_printmemstats::Run()
{
    int error = 0;

    switch (iState)
    {
        case STATE_CREATE:
        {
#if !(OSCL_BYPASS_MEMMGT)
            // Obtain the current mem stats
            OsclAuditCB auditCB;
            OsclMemInit(auditCB);
            if (auditCB.pAudit)
            {
                MM_Stats_t* stats = auditCB.pAudit->MM_GetStats("");
                if (stats)
                {
                    iInitialNumBytes = stats->numBytes;
                    iInitialNumAllocs = stats->numAllocs;
                    fprintf(iTestMsgOutputFile, "Initial memstats: %d allocs, %d bytes\n", iInitialNumAllocs, iInitialNumBytes);
                }
                else
                {
                    fprintf(iTestMsgOutputFile, "Retrieving initial memory statistics failed! Memory statistics result would be invalid.\n");
                }
            }
            else
            {
                fprintf(iTestMsgOutputFile, "Memory audit not available! Memory statistics result would be invalid.\n");
            }
#else
            fprintf(iTestMsgOutputFile, "MEMORY AUDIT DISABLED\n");
#endif

            iPlayer = NULL;

            OSCL_TRY(error, iPlayer = PVPlayerFactory::CreatePlayer(this, this, this));
            if (error)
            {
                PVPATB_TEST_IS_TRUE(false);
                iObserver->TestCompleted(*iTestCase);
            }
            else
            {
                fprintf(iTestMsgOutputFile, "After player instantiation: ");
                PrintMemStats();
                iState = STATE_ADDDATASOURCE;
                RunIfNotReady();
            }
        }
        break;

        case STATE_ADDDATASOURCE:
        {
            iDataSource = new PVPlayerDataSourceURL;
            oscl_UTF8ToUnicode(iFileName, oscl_strlen(iFileName), iTmpWCharBuffer, 512);
            iTmpWCharBuffer[511] = '\0';
            iFileNameWStr = SOURCENAME_PREPEND_WSTRING;
            iFileNameWStr += iTmpWCharBuffer;
            /*
             * In case of HTTP URLs always attempt rollover,
             * since we donot know if it is a download or a streaming url
             */
            if (iFileType == PVMF_MIME_DATA_SOURCE_HTTP_URL)
            {
                iSourceContextData = new PVMFSourceContextData();
                iSourceContextData->EnableStreamingSourceContext();
                iSourceContextData->EnableCommonSourceContext();
                PVInterface* sourceContextStream = NULL;

                PVUuid streamingContextUuid(PVMF_SOURCE_CONTEXT_DATA_STREAMING_UUID);
                if (iSourceContextData->queryInterface(streamingContextUuid, sourceContextStream))
                {
                    PVMFSourceContextDataStreaming* streamingContext =
                        OSCL_STATIC_CAST(PVMFSourceContextDataStreaming*, sourceContextStream);
                    streamingContext->iStreamStatsLoggingURL = iFileNameWStr;

                    if (iProxyEnabled)
                    {
                        streamingContext->iProxyName = _STRLIT_WCHAR("");
                        streamingContext->iProxyPort = 8080;
                    }
                }
                PVInterface* sourceContextDownload = NULL;
                iSourceContextData->EnableDownloadHTTPSourceContext();
                PVUuid downloadContextUuid(PVMF_SOURCE_CONTEXT_DATA_DOWNLOAD_HTTP_UUID);
                if (iSourceContextData->queryInterface(downloadContextUuid, sourceContextDownload))
                {
                    //create the opaque data
                    iDownloadProxy = _STRLIT_CHAR("");
                    int32 iDownloadProxyPort = 0;
                    if (iProxyEnabled)
                    {
                        iDownloadProxy = _STRLIT_CHAR("");
                        iDownloadProxyPort = 8080;
                    }
                    iDownloadConfigFilename = OUTPUTNAME_PREPEND_WSTRING;
                    iDownloadConfigFilename += _STRLIT_WCHAR("mydlconfig");
                    iDownloadMaxfilesize = 0x7FFFFFFF;
                    iDownloadFilename = OUTPUTNAME_PREPEND_WSTRING;
                    iDownloadFilename += _STRLIT_WCHAR("test_ftdownload.dl");
                    bool aIsNewSession = true;

                    iSourceContextData->DownloadHTTPData()->bIsNewSession = aIsNewSession;
                    iSourceContextData->DownloadHTTPData()->iConfigFileName = iDownloadConfigFilename;
                    iSourceContextData->DownloadHTTPData()->iDownloadFileName = iDownloadFilename;
                    iSourceContextData->DownloadHTTPData()->iMaxFileSize = iDownloadMaxfilesize;
                    iSourceContextData->DownloadHTTPData()->iPlaybackControl = PVMFSourceContextDataDownloadHTTP::EAsap;
                    iSourceContextData->DownloadHTTPData()->iProxyName = iDownloadProxy;
                    iSourceContextData->DownloadHTTPData()->iProxyPort = iDownloadProxyPort;

                }
                iDataSource->SetDataSourceContextData((OsclAny*)iSourceContextData);

                iDataSource->SetDataSourceFormatType(PVMF_MIME_DATA_SOURCE_MS_HTTP_STREAMING_URL);
                iDataSource->SetAlternateSourceFormatType(PVMF_MIME_DATA_SOURCE_HTTP_URL);
            }
            else
            {
                iDataSource->SetDataSourceFormatType(iFileType);
            }

            iDataSource->SetDataSourceURL(iFileNameWStr);

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
            OSCL_wHeapString<OsclMemAllocator> videosinkfilename = OUTPUTNAME_PREPEND_WSTRING;
            videosinkfilename += _STRLIT_WCHAR("test_player_printmemstats_video.dat");

            iMIOFileOutVideo = iMioFactory->CreateVideoOutput((OsclAny*) & videosinkfilename, MEDIATYPE_VIDEO, iCompressedVideo);
            iIONodeVideo = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutVideo);
            iDataSinkVideo = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkVideo)->SetDataSinkNode(iIONodeVideo);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_AUDIO:
        {
            OSCL_wHeapString<OsclMemAllocator> audiosinkfilename = OUTPUTNAME_PREPEND_WSTRING;
            audiosinkfilename += _STRLIT_WCHAR("test_player_printmemstats_audio.dat");

            iMIOFileOutAudio = iMioFactory->CreateAudioOutput((OsclAny*) & audiosinkfilename, MEDIATYPE_AUDIO, iCompressedAudio);
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

        case STATE_PRINTMEMSTATS:
        {
            PVPPlaybackPosition curpos;
            curpos.iPosValue.millisec_value = 0;
            curpos.iPosUnit = PVPPBPOSUNIT_MILLISEC;
            iPlayer->GetCurrentPositionSync(curpos);
            fprintf(iTestMsgOutputFile, "After playing %d ms: ", curpos.iPosValue.millisec_value);
            PrintMemStats();

            ++iPlayTimeCtr;
            if (iPlayTimeCtr < 20)
            {
                iState = STATE_PRINTMEMSTATS;
                RunIfNotReady(1000000);
            }
            else
            {
                iState = STATE_STOP;
                RunIfNotReady();
            }
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

            fprintf(iTestMsgOutputFile, "After player destruction: ");
            PrintMemStats();

            delete iSourceContextData;
            iSourceContextData = NULL;

            delete iDataSource;
            iDataSource = NULL;

            delete iDataSinkVideo;
            iDataSinkVideo = NULL;

            delete iDataSinkAudio;
            iDataSinkAudio = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeVideo);
            iIONodeVideo = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeAudio);
            iIONodeAudio = NULL;

            iMioFactory->DestroyVideoOutput(iMIOFileOutVideo);
            iMIOFileOutVideo = NULL;

            iMioFactory->DestroyAudioOutput(iMIOFileOutAudio);
            iMIOFileOutAudio = NULL;

            fprintf(iTestMsgOutputFile, "After cleanup: ");
            PrintMemStats();

            iObserver->TestCompleted(*iTestCase);
        }
        break;

        default:
            break;

    }
}


void pvplayer_async_test_printmemstats::CommandCompleted(const PVCmdResponse& aResponse)
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
                fprintf(iTestMsgOutputFile, "After AddDataSource(): ");
                PrintMemStats();

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
                fprintf(iTestMsgOutputFile, "After Init(): ");
                PrintMemStats();

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
                fprintf(iTestMsgOutputFile, "After GetMetadataKeys(): ");
                PrintMemStats();

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
                fprintf(iTestMsgOutputFile, "After GetMetadataValue(): ");
                PrintMemStats();

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
                fprintf(iTestMsgOutputFile, "After AddDataSink() video: ");
                PrintMemStats();

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
                fprintf(iTestMsgOutputFile, "After AddDataSink() audio: ");
                PrintMemStats();

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
                fprintf(iTestMsgOutputFile, "After Prepare(): ");
                PrintMemStats();

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
                fprintf(iTestMsgOutputFile, "After Start(): ");
                PrintMemStats();

                iState = STATE_PRINTMEMSTATS;
                RunIfNotReady(1000000);
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
                fprintf(iTestMsgOutputFile, "After Stop(): ");
                PrintMemStats();

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
                fprintf(iTestMsgOutputFile, "After RemoveDataSink() video: ");
                PrintMemStats();

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
                fprintf(iTestMsgOutputFile, "After RemoveDataSink() audio: ");
                PrintMemStats();

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
                fprintf(iTestMsgOutputFile, "After Reset(): ");
                PrintMemStats();

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
                fprintf(iTestMsgOutputFile, "After RemoveDataSource(): ");
                PrintMemStats();

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


void pvplayer_async_test_printmemstats::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
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


void pvplayer_async_test_printmemstats::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingStart)
    {
        fprintf(iTestMsgOutputFile, "PVMFInfoErrorHandlingStart...\n");
    }
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingComplete)
    {
        fprintf(iTestMsgOutputFile, "PVMFInfoErrorHandlingComplete...\n");
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
    }
}


void pvplayer_async_test_printmemstats::PrintMemStats()
{
#if !(OSCL_BYPASS_MEMMGT)
    // Obtain the current mem stats
    OsclAuditCB auditCB;
    OsclMemInit(auditCB);
    if (auditCB.pAudit)
    {
        MM_Stats_t* stats = auditCB.pAudit->MM_GetStats("");
        if (stats)
        {
            fprintf(iTestMsgOutputFile, "In use %d allocs, %d bytes.\n", stats->numAllocs - iInitialNumAllocs, stats->numBytes - iInitialNumBytes);
        }
        else
        {
            fprintf(iTestMsgOutputFile, "Retrieving memory statistics failed! Memory statistics result would be invalid.\n");
        }
    }
    else
    {
        fprintf(iTestMsgOutputFile, "Memory audit not available! Memory statistics result would be invalid.\n");
    }
#else
    fprintf(iTestMsgOutputFile, "MEMORY AUDIT DISABLED\n");
#endif
}


//
// pvplayer_async_test_playuntileos section
//
void pvplayer_async_test_playuntileos::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_playuntileos::Run()
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
            oscl_UTF8ToUnicode(iFileName, oscl_strlen(iFileName), iTmpWCharBuffer, 512);
            iFileNameWStr.set(iTmpWCharBuffer, oscl_strlen(iTmpWCharBuffer));
            /*
             * In case of HTTP URLs always attempt rollover,
             * since we donot know if it is a download or a streaming url
             */
            if (iFileType == PVMF_MIME_DATA_SOURCE_HTTP_URL)
            {
                iSourceContextData = new PVMFSourceContextData();
                iSourceContextData->EnableStreamingSourceContext();
                iSourceContextData->EnableCommonSourceContext();
                PVInterface* sourceContextStream = NULL;

                PVUuid streamingContextUuid(PVMF_SOURCE_CONTEXT_DATA_STREAMING_UUID);
                if (iSourceContextData->queryInterface(streamingContextUuid, sourceContextStream))
                {
                    PVMFSourceContextDataStreaming* streamingContext =
                        OSCL_STATIC_CAST(PVMFSourceContextDataStreaming*, sourceContextStream);
                    streamingContext->iStreamStatsLoggingURL = iFileNameWStr;

                    if (iProxyEnabled)
                    {
                        streamingContext->iProxyName = _STRLIT_WCHAR("");
                        streamingContext->iProxyPort = 8080;
                    }
                }
                PVInterface* sourceContextDownload = NULL;
                iSourceContextData->EnableDownloadHTTPSourceContext();
                PVUuid downloadContextUuid(PVMF_SOURCE_CONTEXT_DATA_DOWNLOAD_HTTP_UUID);
                if (iSourceContextData->queryInterface(downloadContextUuid, sourceContextDownload))
                {
                    //create the opaque data
                    iDownloadProxy = _STRLIT_CHAR("");
                    int32 iDownloadProxyPort = 0;
                    if (iProxyEnabled)
                    {
                        iDownloadProxy = _STRLIT_CHAR("");
                        iDownloadProxyPort = 8080;
                    }
                    iDownloadConfigFilename = OUTPUTNAME_PREPEND_WSTRING;
                    iDownloadConfigFilename += _STRLIT_WCHAR("mydlconfig");
                    iDownloadMaxfilesize = 0x7FFFFFFF;
                    iDownloadFilename = OUTPUTNAME_PREPEND_WSTRING;
                    iDownloadFilename += _STRLIT_WCHAR("test_ftdownload.dl");
                    bool aIsNewSession = true;

                    iSourceContextData->DownloadHTTPData()->bIsNewSession = aIsNewSession;
                    iSourceContextData->DownloadHTTPData()->iConfigFileName = iDownloadConfigFilename;
                    iSourceContextData->DownloadHTTPData()->iDownloadFileName = iDownloadFilename;
                    iSourceContextData->DownloadHTTPData()->iMaxFileSize = iDownloadMaxfilesize;
                    iSourceContextData->DownloadHTTPData()->iPlaybackControl = PVMFSourceContextDataDownloadHTTP::EAsap;
                    iSourceContextData->DownloadHTTPData()->iProxyName = iDownloadProxy;
                    iSourceContextData->DownloadHTTPData()->iProxyPort = iDownloadProxyPort;

                }
                iDataSource->SetDataSourceContextData((OsclAny*)iSourceContextData);

                iDataSource->SetDataSourceFormatType(PVMF_MIME_DATA_SOURCE_MS_HTTP_STREAMING_URL);
                iDataSource->SetAlternateSourceFormatType(PVMF_MIME_DATA_SOURCE_HTTP_URL);
            }
            else
            {
                iDataSource->SetDataSourceFormatType(iFileType);
            }

            iDataSource->SetDataSourceURL(iFileNameWStr);

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
            OSCL_wHeapString<OsclMemAllocator> sinkfilename;
            sinkfilename = OUTPUTNAME_PREPEND_WSTRING;
            sinkfilename += _STRLIT_WCHAR("test_player_playuntileos_");
            if (iCompressedVideo)
            {
                sinkfilename += _STRLIT_WCHAR("compressed_");
            }
            OSCL_wHeapString<OsclMemAllocator> inputfilename;
            RetrieveFilename(iFileNameWStr.get_str(), inputfilename);
            sinkfilename += inputfilename;
            sinkfilename += _STRLIT_WCHAR("_video.dat");

            iMIOFileOutVideo = iMioFactory->CreateVideoOutput((OsclAny*) & sinkfilename, MEDIATYPE_VIDEO, iCompressedVideo);
            iIONodeVideo = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutVideo);
            iDataSinkVideo = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkVideo)->SetDataSinkNode(iIONodeVideo);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_AUDIO:
        {
            OSCL_wHeapString<OsclMemAllocator> sinkfilename;
            sinkfilename = OUTPUTNAME_PREPEND_WSTRING;
            sinkfilename += _STRLIT_WCHAR("test_player_playuntileos_");
            if (iCompressedAudio)
            {
                sinkfilename += _STRLIT_WCHAR("compressed_");
            }
            OSCL_wHeapString<OsclMemAllocator> inputfilename;
            RetrieveFilename(iFileNameWStr.get_str(), inputfilename);
            sinkfilename += inputfilename;
            sinkfilename += _STRLIT_WCHAR("_audio.dat");

            iMIOFileOutAudio = iMioFactory->CreateAudioOutput((OsclAny*) & sinkfilename, MEDIATYPE_AUDIO, iCompressedAudio);
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
            sinkfilename += _STRLIT_WCHAR("test_player_playuntileos_");
            OSCL_wHeapString<OsclMemAllocator> inputfilename;
            RetrieveFilename(iFileNameWStr.get_str(), inputfilename);
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

            iMioFactory->DestroyVideoOutput(iMIOFileOutVideo);
            iMIOFileOutVideo = NULL;

            iMioFactory->DestroyAudioOutput(iMIOFileOutAudio);
            iMIOFileOutAudio = NULL;

            iMioFactory->DestroyTextOutput(iMIOFileOutText);
            iMIOFileOutText = NULL;

            iObserver->TestCompleted(*iTestCase);
        }
        break;

        default:
            break;

    }
}


void pvplayer_async_test_playuntileos::CommandCompleted(const PVCmdResponse& aResponse)
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
                // The delay is added between Prepare and Start to test that player
                // does not start the clock and send playstatus events prior to start.
                RunIfNotReady(PVPLAYER_ASYNC_TEST_PLAYUNTILEOS_DELAY_AFTER_PREPARE);
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
                // Do nothing and wait for EOS event
                fprintf(iTestMsgOutputFile, "Playback started.\n");
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


void pvplayer_async_test_playuntileos::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
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


void pvplayer_async_test_playuntileos::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingStart)
    {
        fprintf(iTestMsgOutputFile, "PVMFInfoErrorHandlingStart...\n");
    }
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingComplete)
    {
        fprintf(iTestMsgOutputFile, "PVMFInfoErrorHandlingComplete...\n");
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
                fprintf(iTestMsgOutputFile, "EOS received. Stopping playback.\n");
                iState = STATE_STOP;
                Cancel();
                RunIfNotReady();
            }
        }
    }
    // Check and print out playback position status
    else if (aEvent.GetEventType() == PVMFInfoPositionStatus)
    {
        PVPlayerState pstate;
        iPlayer->GetPVPlayerStateSync(pstate);
        if (pstate != PVP_STATE_STARTED)
        {
            fprintf(iTestMsgOutputFile, "Playback status recived in Wrong Engine State\n");
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
            return;
        }

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
                uint8* localbuf = aEvent.GetLocalBuffer();
                uint32 pbpos = 0;
                oscl_memcpy(&pbpos, &(localbuf[4]), 4);
                fprintf(iTestMsgOutputFile, "Playback status(time) %d ms\n", pbpos);
            }
        }

        PVPPlaybackPosition curpos;

        curpos.iPosValue.percent_value = 0;
        curpos.iPosUnit = PVPPBPOSUNIT_PERCENT;
        if (iPlayer->GetCurrentPositionSync(curpos) == PVMFSuccess)
        {
            fprintf(iTestMsgOutputFile, "Playback status(percentage) %d%%\n", curpos.iPosValue.percent_value);
        }


        curpos.iPosValue.samplenum_value = 0;
        curpos.iPosUnit = PVPPBPOSUNIT_SAMPLENUMBER;
        if (iPlayer->GetCurrentPositionSync(curpos) == PVMFSuccess)
        {
            fprintf(iTestMsgOutputFile, "Playback status(sample number) %d\n", curpos.iPosValue.samplenum_value);
        }

        curpos.iPosValue.datapos_value = 0;
        curpos.iPosUnit = PVPPBPOSUNIT_DATAPOSITION;
        if (iPlayer->GetCurrentPositionSync(curpos) == PVMFSuccess)
        {
            fprintf(iTestMsgOutputFile, "Playback status(data position) %d bytes\n", curpos.iPosValue.datapos_value);
        }

        fprintf(iTestMsgOutputFile, "------------------------------\n");
    }
}






