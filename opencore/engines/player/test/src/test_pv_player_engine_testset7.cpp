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
#include "test_pv_player_engine_testset7.h"
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

#ifndef OSCL_STRING_UTILS_H_INCLUDED
#include "oscl_string_utils.h"
#endif

extern FILE* file;

//
// pvplayer_async_test_mediaionode_openplaystop section
//
void pvplayer_async_test_mediaionode_openplaystop::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_mediaionode_openplaystop::Run()
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
            iDataSource->SetDataSourceURL(iFileNameWStr);
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

        case STATE_GETMETADATAKEYLIST:
        {
            iMetadataKeyList.clear();
            OSCL_TRY(error, iCurrentCmdId = iPlayer->GetMetadataKeys(iMetadataKeyList, 0, 20, NULL, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_GETMETADATAVALUELIST:
        {
            iMetadataValueList.clear();
            iNumValues = 0;
            OSCL_TRY(error, iCurrentCmdId = iPlayer->GetMetadataValues(iMetadataKeyList, 0, 20, iNumValues, iMetadataValueList, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_VIDEO:
        {
            OSCL_wHeapString<OsclMemAllocator> sinkfilename;
            sinkfilename = OUTPUTNAME_PREPEND_WSTRING;
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_openplaystop_");
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
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_openplaystop_");
            if (iCompressedAudio)
            {
                sinkfilename += _STRLIT_WCHAR("compressed_");
            }
            OSCL_wHeapString<OsclMemAllocator> inputfilename;
            RetrieveFilename(iFileNameWStr.get_str(), inputfilename);
            sinkfilename += inputfilename;
            sinkfilename += _STRLIT_WCHAR("_audio.wav");

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
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_openplaystop_");
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


void pvplayer_async_test_mediaionode_openplaystop::CommandCompleted(const PVCmdResponse& aResponse)
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
            if (aResponse.GetCmdStatus() == PVMFSuccess || aResponse.GetCmdStatus() == PVMFErrArgument)
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
            if (aResponse.GetCmdStatus() == PVMFSuccess || aResponse.GetCmdStatus() == PVMFErrArgument)
            {
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


void pvplayer_async_test_mediaionode_openplaystop::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
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


void pvplayer_async_test_mediaionode_openplaystop::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
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


//
// pvplayer_async_test_mediaionode_playstopplay section
//
void pvplayer_async_test_mediaionode_playstopplay::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_mediaionode_playstopplay::Run()
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
            iDataSource->SetDataSourceURL(iFileNameWStr);
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
            OSCL_wHeapString<OsclMemAllocator> sinkfilename;
            sinkfilename = OUTPUTNAME_PREPEND_WSTRING;
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_playstopplay_");
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
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_playstopplay_");
            if (iCompressedAudio)
            {
                sinkfilename += _STRLIT_WCHAR("compressed_");
            }
            OSCL_wHeapString<OsclMemAllocator> inputfilename;
            RetrieveFilename(iFileNameWStr.get_str(), inputfilename);
            sinkfilename += inputfilename;
            sinkfilename += _STRLIT_WCHAR("_audio.wav");

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
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_playstopplay_");
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

        case STATE_PREPARE1:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Prepare((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_START1:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Start((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_STOP1:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Stop((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_PREPARE2:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Prepare((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_START2:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Start((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_STOP2:
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


void pvplayer_async_test_mediaionode_playstopplay::CommandCompleted(const PVCmdResponse& aResponse)
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
                iState = STATE_PREPARE1;
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

        case STATE_PREPARE1:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_START1;
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

        case STATE_START1:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_STOP1;
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

        case STATE_STOP1:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_PREPARE2;
                RunIfNotReady(5000000);
            }
            else
            {
                // Stop failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_PREPARE2:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_START2;
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

        case STATE_START2:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_STOP2;
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

        case STATE_STOP2:
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


void pvplayer_async_test_mediaionode_playstopplay::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
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


void pvplayer_async_test_mediaionode_playstopplay::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
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



//
// pvplayer_async_test_mediaionode_pauseresume section
//
void pvplayer_async_test_mediaionode_pauseresume::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_mediaionode_pauseresume::Run()
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
            iDataSource->SetDataSourceURL(iFileNameWStr);
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
            OSCL_wHeapString<OsclMemAllocator> sinkfilename;
            sinkfilename = OUTPUTNAME_PREPEND_WSTRING;
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_pauseresume_");
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
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_pauseresume_");
            if (iCompressedAudio)
            {
                sinkfilename += _STRLIT_WCHAR("compressed_");
            }
            OSCL_wHeapString<OsclMemAllocator> inputfilename;
            RetrieveFilename(iFileNameWStr.get_str(), inputfilename);
            sinkfilename += inputfilename;
            sinkfilename += _STRLIT_WCHAR("_audio.wav");

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
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_pauseresume_");
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


void pvplayer_async_test_mediaionode_pauseresume::CommandCompleted(const PVCmdResponse& aResponse)
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


void pvplayer_async_test_mediaionode_pauseresume::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
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


void pvplayer_async_test_mediaionode_pauseresume::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
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



//
// pvplayer_async_test_mediaionode_playsetplaybackrange section
//
void pvplayer_async_test_mediaionode_playsetplaybackrange::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_mediaionode_playsetplaybackrange::Run()
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
            iDataSource->SetDataSourceURL(iFileNameWStr);
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
            OSCL_wHeapString<OsclMemAllocator> sinkfilename;
            sinkfilename = OUTPUTNAME_PREPEND_WSTRING;
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_playsetplaybackrange_");
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
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_playsetplaybackrange_");
            if (iCompressedAudio)
            {
                sinkfilename += _STRLIT_WCHAR("compressed_");
            }
            OSCL_wHeapString<OsclMemAllocator> inputfilename;
            RetrieveFilename(iFileNameWStr.get_str(), inputfilename);
            sinkfilename += inputfilename;
            sinkfilename += _STRLIT_WCHAR("_audio.wav");

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
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_playsetplaybackrange_");
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

        case STATE_SETPLAYBACKRANGE:
        {
            PVPPlaybackPosition start, end;
            start.iIndeterminate = false;
            start.iPosUnit = PVPPBPOSUNIT_SEC;
            start.iMode = PVPPBPOS_MODE_NOW;
            start.iPosValue.sec_value = 6;
            end.iIndeterminate = true;
            fprintf(iTestMsgOutputFile, "***Set PlaybackRange, Pos=%d\n", start.iPosValue.sec_value);
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


void pvplayer_async_test_mediaionode_playsetplaybackrange::CommandCompleted(const PVCmdResponse& aResponse)
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
                iState = STATE_SETPLAYBACKRANGE;
                RunIfNotReady(3*1000*1000);
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
                fprintf(iTestMsgOutputFile, "***Set PlaybackRange Complete....\n");
                iState = STATE_STOP;
                RunIfNotReady(10000000);
            }
            else
            {
                // SetPlaybackRange failed
                PVPATB_TEST_IS_TRUE(false);
                if (aResponse.GetCmdStatus() == PVMFErrNotSupported || aResponse.GetCmdStatus() == PVMFErrArgument)
                {
                    iState = STATE_STOP;
                    RunIfNotReady();
                }
                else
                {
                    iState = STATE_CLEANUPANDCOMPLETE;
                    RunIfNotReady();
                }
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


void pvplayer_async_test_mediaionode_playsetplaybackrange::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    switch (aEvent.GetEventType())
    {
        case PVMFErrNotSupported:
        case PVMFErrArgument:
            // Log the error but don't handle it
            PVPATB_TEST_IS_TRUE(false);
            return;

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


void pvplayer_async_test_mediaionode_playsetplaybackrange::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
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



//
// pvplayer_async_test_mediaionode_3Xplayrate section
//
void pvplayer_async_test_mediaionode_3Xplayrate::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_mediaionode_3Xplayrate::Run()
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
            iDataSource->SetDataSourceURL(iFileNameWStr);
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
            OSCL_wHeapString<OsclMemAllocator> sinkfilename;
            sinkfilename = OUTPUTNAME_PREPEND_WSTRING;
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_setplaybackrate3x_");
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
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_setplaybackrate3x_");
            if (iCompressedAudio)
            {
                sinkfilename += _STRLIT_WCHAR("compressed_");
            }
            OSCL_wHeapString<OsclMemAllocator> inputfilename;
            RetrieveFilename(iFileNameWStr.get_str(), inputfilename);
            sinkfilename += inputfilename;
            sinkfilename += _STRLIT_WCHAR("_audio.wav");

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
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_setplaybackrate3x_");
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

        case STATE_SETPLAYBACKRATE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRate(300000, NULL, (OsclAny*) & iContextObject));
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


void pvplayer_async_test_mediaionode_3Xplayrate::CommandCompleted(const PVCmdResponse& aResponse)
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
                // SetPlaybackRate failed. Not fatal so just stop test
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_STOP;
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


void pvplayer_async_test_mediaionode_3Xplayrate::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
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


void pvplayer_async_test_mediaionode_3Xplayrate::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
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


//
// pvplayer_async_test_mediaionode_halfplayrate section
//
void pvplayer_async_test_mediaionode_halfplayrate::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_mediaionode_halfplayrate::Run()
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
            iDataSource->SetDataSourceURL(iFileNameWStr);
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
            OSCL_wHeapString<OsclMemAllocator> sinkfilename;
            sinkfilename = OUTPUTNAME_PREPEND_WSTRING;
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_setplaybackratehalf_");
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
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_setplaybackratehalf_");
            if (iCompressedAudio)
            {
                sinkfilename += _STRLIT_WCHAR("compressed_");
            }
            OSCL_wHeapString<OsclMemAllocator> inputfilename;
            RetrieveFilename(iFileNameWStr.get_str(), inputfilename);
            sinkfilename += inputfilename;
            sinkfilename += _STRLIT_WCHAR("_audio.wav");

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
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_setplaybackratehalf_");
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

        case STATE_SETPLAYBACKRATE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRate(50000, NULL, (OsclAny*) & iContextObject));
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


void pvplayer_async_test_mediaionode_halfplayrate::CommandCompleted(const PVCmdResponse& aResponse)
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
                // SetPlaybackRate failed. Not fatal so just stop test
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_STOP;
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


void pvplayer_async_test_mediaionode_halfplayrate::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
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


void pvplayer_async_test_mediaionode_halfplayrate::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
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


//
// pvplayer_async_test_mediaionode_looping section
//
void pvplayer_async_test_mediaionode_looping::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_mediaionode_looping::Run()
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
            iDataSource->SetDataSourceURL(iFileNameWStr);
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
            OSCL_wHeapString<OsclMemAllocator> sinkfilename;
            sinkfilename = OUTPUTNAME_PREPEND_WSTRING;
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_looping_");
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
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_looping_");
            if (iCompressedAudio)
            {
                sinkfilename += _STRLIT_WCHAR("compressed_");
            }
            OSCL_wHeapString<OsclMemAllocator> inputfilename;
            RetrieveFilename(iFileNameWStr.get_str(), inputfilename);
            sinkfilename += inputfilename;
            sinkfilename += _STRLIT_WCHAR("_audio.wav");

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
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_looping_");
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

        case STATE_SETPLAYBACKRANGE1:
        {
            PVPPlaybackPosition start, end;
            start.iIndeterminate = true;
            end.iIndeterminate = false;
            end.iPosUnit = PVPPBPOSUNIT_MILLISEC;
            start.iMode = PVPPBPOS_MODE_NOW;
            end.iPosValue.millisec_value = 10000;
            OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRange(start, end, false, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
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
        }
        break;

        case STATE_RESUME:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Resume((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
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


void pvplayer_async_test_mediaionode_looping::CommandCompleted(const PVCmdResponse& aResponse)
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
                iState = STATE_SETPLAYBACKRANGE1;
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

        case STATE_SETPLAYBACKRANGE1:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ENDTIMENOTREACHED1;
                RunIfNotReady(15*1000*1000*10);
            }
            else
            {
                // SetPlaybackRange failed
                PVPATB_TEST_IS_TRUE(false);
                if (aResponse.GetCmdStatus() == PVMFErrNotSupported ||
                        aResponse.GetCmdStatus() == PVMFErrArgument)
                {
                    iState = STATE_STOP;
                    RunIfNotReady();
                }
                else
                {
                    iState = STATE_CLEANUPANDCOMPLETE;
                    RunIfNotReady();
                }
            }
            break;

        case STATE_SETPLAYBACKRANGE2:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_RESUME;
                RunIfNotReady();
            }
            else
            {
                // SetPlaybackRange failed
                PVPATB_TEST_IS_TRUE(false);
                if (aResponse.GetCmdStatus() == PVMFErrNotSupported ||
                        aResponse.GetCmdStatus() == PVMFErrArgument)
                {
                    iState = STATE_STOP;
                    RunIfNotReady();
                }
                else
                {
                    iState = STATE_CLEANUPANDCOMPLETE;
                    RunIfNotReady();
                }
            }
            break;

        case STATE_RESUME:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ENDTIMENOTREACHED2;
                RunIfNotReady(15*1000*1000);
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


void pvplayer_async_test_mediaionode_looping::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    switch (aEvent.GetEventType())
    {
        case PVMFErrNotSupported:
        case PVMFErrArgument:
            // Log the error and don't handle error
            PVPATB_TEST_IS_TRUE(false);
            return;

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


void pvplayer_async_test_mediaionode_looping::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
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
            if ((infouuid == PVPlayerErrorInfoEventTypesUUID) && (infocode == PVPlayerInfoEndTimeReached))
            {
                if (iState == STATE_ENDTIMENOTREACHED1)
                {
                    iState = STATE_SETPLAYBACKRANGE2;
                }
                else if (iState == STATE_ENDTIMENOTREACHED2)
                {
                    iState = STATE_STOP;
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
            else
            {
                if (iState == STATE_ENDTIMENOTREACHED1)
                {
                    iState = STATE_SETPLAYBACKRANGE2;
                }
                else if (iState == STATE_ENDTIMENOTREACHED2)
                {
                    iState = STATE_STOP;
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
// pvplayer_async_test_mediaionode_waitforeos section
//
void pvplayer_async_test_mediaionode_waitforeos::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_mediaionode_waitforeos::Run()
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
            iDataSource->SetDataSourceURL(iFileNameWStr);
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
            OSCL_wHeapString<OsclMemAllocator> sinkfilename;
            sinkfilename = OUTPUTNAME_PREPEND_WSTRING;
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_waitforeos_");
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
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_waitforeos_");
            if (iCompressedAudio)
            {
                sinkfilename += _STRLIT_WCHAR("compressed_");
            }
            OSCL_wHeapString<OsclMemAllocator> inputfilename;
            RetrieveFilename(iFileNameWStr.get_str(), inputfilename);
            sinkfilename += inputfilename;
            sinkfilename += _STRLIT_WCHAR("_audio.wav");

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
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_waitforeos_");
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

        case STATE_EOSNOTREACHED:
        {
            // EOS event not received so initiate stop
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_STOP;
            RunIfNotReady();
        }
        break;

        case STATE_RESUMEAFTEREOS:
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


void pvplayer_async_test_mediaionode_waitforeos::CommandCompleted(const PVCmdResponse& aResponse)
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

        case STATE_RESUMEAFTEREOS:
            if (aResponse.GetCmdStatus() != PVMFErrInvalidState)
            {
                // Resume() should fail since playback
                // has already been paused due to EOS
                PVPATB_TEST_IS_TRUE(false);
            }

            // Continue onto stopping playback and shutting down
            iState = STATE_STOP;
            RunIfNotReady();
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


void pvplayer_async_test_mediaionode_waitforeos::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
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


void pvplayer_async_test_mediaionode_waitforeos::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
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
                iState = STATE_RESUMEAFTEREOS;
                Cancel();
                RunIfNotReady();
            }
        }
    }
}


//
// pvplayer_async_test_mediaionode_multipauseresume section
//
void pvplayer_async_test_mediaionode_multipauseresume::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_mediaionode_multipauseresume::Run()
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
            OSCL_wHeapString<OsclMemAllocator> sinkfilename;
            OSCL_wHeapString<OsclMemAllocator> inputfilename;

            iDataSource = new PVPlayerDataSourceURL;

            oscl_UTF8ToUnicode(iFileName, oscl_strlen(iFileName), output, 512);
            wFileName.set(output, oscl_strlen(output));

            if (iFileType == PVMF_MIME_DATA_SOURCE_HTTP_URL)
            {

                //iFileType = PVMF_MIME_DATA_SOURCE_MS_HTTP_STREAMING_URL;
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
                    downloadContext->iDownloadFileName = OUTPUTNAME_PREPEND_WSTRING;
                    downloadContext->iDownloadFileName += _STRLIT_WCHAR("test_ftdownload.loc");
                    downloadContext->iConfigFileName = OUTPUTNAME_PREPEND_WSTRING;
                    downloadContext->iConfigFileName += _STRLIT_WCHAR("mydlconfig");
                    downloadContext->iUserID = _STRLIT_CHAR("abc");
                    downloadContext->iUserPasswd = _STRLIT_CHAR("xyz");
                    downloadContext->bIsNewSession = true;
                    downloadContext->iMaxFileSize = 0x7FFFFFFF;
                    downloadContext->iPlaybackControl = PVMFSourceContextDataDownloadHTTP::EAsap;
                }
                iDataSource->SetDataSourceContextData((OsclAny*)iSourceContextData);

            }

            RetrieveFilename(wFileName.get_str(), inputfilename);

            // Add Data Source
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
            sinkfilename = OUTPUTNAME_PREPEND_WSTRING;
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_multipauseresume_");
            if (iCompressedVideo)
            {
                sinkfilename += _STRLIT_WCHAR("compressed_");
            }
            sinkfilename += inputfilename;
            sinkfilename += _STRLIT_WCHAR("_video.dat");

            iMIOFileOutVideo = iMioFactory->CreateVideoOutput((OsclAny*) & sinkfilename, MEDIATYPE_VIDEO, iCompressedVideo);
            iIONodeVideo = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutVideo);
            iDataSinkVideo = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkVideo)->SetDataSinkNode(iIONodeVideo);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); break);
            iCmdIds.push_back(iCurrentCmdId);

            // Add audio data sink
            sinkfilename = OUTPUTNAME_PREPEND_WSTRING;
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_multipauseresume_");
            if (iCompressedAudio)
            {
                sinkfilename += _STRLIT_WCHAR("compressed_");
            }
            sinkfilename += inputfilename;
            sinkfilename += _STRLIT_WCHAR("_audio.wav");

            iMIOFileOutAudio = iMioFactory->CreateAudioOutput((OsclAny*) & sinkfilename, MEDIATYPE_AUDIO, iCompressedAudio);
            iIONodeAudio = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutAudio);
            iDataSinkAudio = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkAudio)->SetDataSinkNode(iIONodeAudio);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); break);
            iCmdIds.push_back(iCurrentCmdId);

            // Add text data sink
            sinkfilename = OUTPUTNAME_PREPEND_WSTRING;
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_multipauseresume_");
            sinkfilename += inputfilename;
            sinkfilename += _STRLIT_WCHAR("_text.dat");

            iMIOFileOutText = iMioFactory->CreateTextOutput((OsclAny*) & sinkfilename, MEDIATYPE_TEXT);
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


void pvplayer_async_test_mediaionode_multipauseresume::CommandCompleted(const PVCmdResponse& aResponse)
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


void pvplayer_async_test_mediaionode_multipauseresume::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
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


void pvplayer_async_test_mediaionode_multipauseresume::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
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

    if (aEvent.GetEventType() == PVMFInfoSourceFormatNotSupported)
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
// pvplayer_async_test_mediaionode_multireposition section
//
void pvplayer_async_test_mediaionode_multireposition::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_mediaionode_multireposition::Run()
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
            OSCL_wHeapString<OsclMemAllocator> sinkfilename;
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

            // Open file to log reposition times
            iFS.Connect();

            sinkfilename = OUTPUTNAME_PREPEND_WSTRING;
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_multireposition_");
            sinkfilename += inputfilename;
            sinkfilename += _STRLIT_WCHAR("_repositiontime.dat");

            if (iTimeLogFile.Open(sinkfilename.get_str(), Oscl_File::MODE_READWRITE | Oscl_File::MODE_TEXT, iFS))
            {
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                break;
            }

            // Add video data sink
            sinkfilename = OUTPUTNAME_PREPEND_WSTRING;
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_multireposition_");
            if (iCompressedVideo)
            {
                sinkfilename += _STRLIT_WCHAR("compressed_");
            }
            sinkfilename += inputfilename;
            sinkfilename += _STRLIT_WCHAR("_video.dat");

            iMIOFileOutVideo = iMioFactory->CreateVideoOutput((OsclAny*) & sinkfilename, MEDIATYPE_VIDEO, iCompressedVideo);
            iIONodeVideo = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutVideo);
            iDataSinkVideo = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkVideo)->SetDataSinkNode(iIONodeVideo);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); break);
            iCmdIds.push_back(iCurrentCmdId);

            // Add audio data sink
            sinkfilename = OUTPUTNAME_PREPEND_WSTRING;
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_multireposition_");
            if (iCompressedAudio)
            {
                sinkfilename += _STRLIT_WCHAR("compressed_");
            }
            sinkfilename += inputfilename;
            sinkfilename += _STRLIT_WCHAR("_audio.wav");

            iMIOFileOutAudio = iMioFactory->CreateAudioOutput((OsclAny*) & sinkfilename, MEDIATYPE_AUDIO, iCompressedAudio);
            iIONodeAudio = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutAudio);
            iDataSinkAudio = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkAudio)->SetDataSinkNode(iIONodeAudio);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); break);
            iCmdIds.push_back(iCurrentCmdId);

            // Add text data sink
            sinkfilename = OUTPUTNAME_PREPEND_WSTRING;
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_multireposition_");
            sinkfilename += inputfilename;
            sinkfilename += _STRLIT_WCHAR("_text.dat");

            iMIOFileOutText = iMioFactory->CreateTextOutput((OsclAny*) & sinkfilename, MEDIATYPE_TEXT);
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


void pvplayer_async_test_mediaionode_multireposition::CommandCompleted(const PVCmdResponse& aResponse)
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


void pvplayer_async_test_mediaionode_multireposition::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
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


void pvplayer_async_test_mediaionode_multireposition::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
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
// pvplayer_async_test_mediaionode_repositionconfig section
//
void pvplayer_async_test_mediaionode_repositionconfig::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_mediaionode_repositionconfig::Run()
{
    int error = 0;

    PvmiKvp* retkvp = NULL;
    PvmiKvp paramkvp[3];
    OSCL_StackString<64> paramkey1(_STRLIT_CHAR("x-pvmf/player/seektosyncpoint;valtype=bool"));
    paramkvp[0].key = paramkey1.get_str();
    OSCL_StackString<64> paramkey2(_STRLIT_CHAR("x-pvmf/player/syncpointseekwindow;valtype=uint32"));
    paramkvp[1].key = paramkey2.get_str();
    OSCL_StackString<64> paramkey3(_STRLIT_CHAR("x-pvmf/player/skiptorequestedpos;valtype=bool"));
    paramkvp[2].key = paramkey3.get_str();

    switch (iState)
    {
        case STATE_CREATE:
        {
            iPlayer = NULL;
            fprintf(iTestMsgOutputFile, "***Creating Player Engine...\n");
            OSCL_TRY(error, iPlayer = PVPlayerFactory::CreatePlayer(this, this, this));
            if (error)
            {
                PVPATB_TEST_IS_TRUE(false);
                iObserver->TestCompleted(*iTestCase);
            }
            else
            {
                PVUuid capconfigifuuid = PVMI_CAPABILITY_AND_CONFIG_PVUUID;
                OSCL_TRY(error, iCurrentCmdId = iPlayer->QueryInterface(capconfigifuuid, (PVInterface*&)iPlayerCapConfigIF, (OsclAny*) & iContextObject));
                OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            }
        }
        break;

        case STATE_ADDDATASOURCE:
        {
            fprintf(iTestMsgOutputFile, "***Adding Data Source...\n");
            iDataSource = new PVPlayerDataSourceURL;
            oscl_UTF8ToUnicode(iFileName, oscl_strlen(iFileName), iTmpWCharBuffer, 512);
            iFileNameWStr.set(iTmpWCharBuffer, oscl_strlen(iTmpWCharBuffer));
            iDataSource->SetDataSourceURL(iFileNameWStr);
            iDataSource->SetDataSourceFormatType(iFileType);
            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }

        break;

        case STATE_INIT:
        {
            fprintf(iTestMsgOutputFile, "***Initializing...\n");
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Init((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_VIDEO:
        {
            fprintf(iTestMsgOutputFile, "***Adding Video Data Sink...\n");
            OSCL_wHeapString<OsclMemAllocator> sinkfilename;
            sinkfilename = OUTPUTNAME_PREPEND_WSTRING;
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_repositionconfig_");
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
            fprintf(iTestMsgOutputFile, "***Adding Audio Data Sink...\n");
            OSCL_wHeapString<OsclMemAllocator> sinkfilename;
            sinkfilename = OUTPUTNAME_PREPEND_WSTRING;
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_repositionconfig_");
            if (iCompressedAudio)
            {
                sinkfilename += _STRLIT_WCHAR("compressed_");
            }
            OSCL_wHeapString<OsclMemAllocator> inputfilename;
            RetrieveFilename(iFileNameWStr.get_str(), inputfilename);
            sinkfilename += inputfilename;
            sinkfilename += _STRLIT_WCHAR("_audio.wav");

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
            fprintf(iTestMsgOutputFile, "***Adding Text Data Sink...\n");
            OSCL_wHeapString<OsclMemAllocator> sinkfilename;
            sinkfilename = OUTPUTNAME_PREPEND_WSTRING;
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_repositionconfig_");
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

        case STATE_SETPLAYBACKRANGE1:
        {
            paramkvp[0].value.bool_value = true;
            paramkvp[1].value.uint32_value = 0;
            paramkvp[2].value.bool_value = false;
            fprintf(iTestMsgOutputFile, "***seektosyncpoint=%d, syncpointseekwindow=%d, skiptorequestedpos=%d\n",
                    paramkvp[0].value.bool_value,
                    paramkvp[1].value.uint32_value,
                    paramkvp[2].value.bool_value);
            iPlayerCapConfigIF->setParametersSync(NULL, paramkvp, 3, retkvp);
            PVPATB_TEST_IS_TRUE(retkvp == NULL);

            PVPPlaybackPosition start, end;
            start.iIndeterminate = false;
            start.iPosUnit = PVPPBPOSUNIT_SEC;
            start.iMode = PVPPBPOS_MODE_NOW;
            start.iPosValue.sec_value = 10;
            end.iIndeterminate = true;
            fprintf(iTestMsgOutputFile, "***Set PlayBackRange_1 - TargetNPT=%d secs\n", start.iPosValue.sec_value);
            OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRange(start, end, false, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_PREPARE:
        {
            fprintf(iTestMsgOutputFile, "***Preparing...\n");
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Prepare((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_START:
        {
            fprintf(iTestMsgOutputFile, "***Starting...\n");
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Start((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_SETPLAYBACKRANGE2:
        {
            PVPPlaybackPosition start, end;
            start.iIndeterminate = false;
            start.iPosUnit = PVPPBPOSUNIT_SEC;
            start.iMode = PVPPBPOS_MODE_NOW;
            start.iPosValue.sec_value = 20;
            end.iIndeterminate = true;
            fprintf(iTestMsgOutputFile, "***Set PlayBackRange_2 - TargetNPT=%d secs\n", start.iPosValue.sec_value);
            OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRange(start, end, false, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_SETPLAYBACKRANGE3:
        {
            paramkvp[0].value.bool_value = true;
            paramkvp[1].value.uint32_value = 1000;
            paramkvp[2].value.bool_value = false;
            fprintf(iTestMsgOutputFile, "***seektosyncpoint=%d, syncpointseekwindow=%d, skiptorequestedpos=%d\n",
                    paramkvp[0].value.bool_value,
                    paramkvp[1].value.uint32_value,
                    paramkvp[2].value.bool_value);
            iPlayerCapConfigIF->setParametersSync(NULL, paramkvp, 3, retkvp);
            PVPATB_TEST_IS_TRUE(retkvp == NULL);

            PVPPlaybackPosition start, end;
            start.iIndeterminate = false;
            start.iPosUnit = PVPPBPOSUNIT_SEC;
            start.iMode = PVPPBPOS_MODE_NOW;
            start.iPosValue.sec_value = 20;
            end.iIndeterminate = true;
            fprintf(iTestMsgOutputFile, "***Set PlayBackRange_3 - TargetNPT=%d secs\n", start.iPosValue.sec_value);
            OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRange(start, end, false, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_SETPLAYBACKRANGE4:
        {
            paramkvp[0].value.bool_value = true;
            paramkvp[1].value.uint32_value = 1000;
            paramkvp[2].value.bool_value = true;
            fprintf(iTestMsgOutputFile, "***seektosyncpoint=%d, syncpointseekwindow=%d, skiptorequestedpos=%d\n",
                    paramkvp[0].value.bool_value,
                    paramkvp[1].value.uint32_value,
                    paramkvp[2].value.bool_value);
            iPlayerCapConfigIF->setParametersSync(NULL, paramkvp, 3, retkvp);
            PVPATB_TEST_IS_TRUE(retkvp == NULL);

            PVPPlaybackPosition start, end;
            start.iIndeterminate = false;
            start.iPosUnit = PVPPBPOSUNIT_SEC;
            start.iMode = PVPPBPOS_MODE_NOW;
            start.iPosValue.sec_value = 20;
            end.iIndeterminate = true;
            fprintf(iTestMsgOutputFile, "***Set PlayBackRange_4 - TargetNPT=%d secs\n", start.iPosValue.sec_value);
            OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRange(start, end, false, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_SETPLAYBACKRANGE5:
        {
            paramkvp[0].value.bool_value = false;
            paramkvp[1].value.uint32_value = 0;
            paramkvp[2].value.bool_value = false;
            fprintf(iTestMsgOutputFile, "***seektosyncpoint=%d, syncpointseekwindow=%d, skiptorequestedpos=%d\n",
                    paramkvp[0].value.bool_value,
                    paramkvp[1].value.uint32_value,
                    paramkvp[2].value.bool_value);
            iPlayerCapConfigIF->setParametersSync(NULL, paramkvp, 3, retkvp);
            PVPATB_TEST_IS_TRUE(retkvp == NULL);

            PVPPlaybackPosition start, end;
            start.iIndeterminate = false;
            start.iPosUnit = PVPPBPOSUNIT_SEC;
            start.iMode = PVPPBPOS_MODE_NOW;
            start.iPosValue.sec_value = 20;
            end.iIndeterminate = true;
            fprintf(iTestMsgOutputFile, "***Set PlayBackRange_5 - TargetNPT=%d secs\n", start.iPosValue.sec_value);
            OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRange(start, end, false, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_SETPLAYBACKRANGE6:
        {
            paramkvp[0].value.bool_value = false;
            paramkvp[1].value.uint32_value = 0;
            paramkvp[2].value.bool_value = true;
            fprintf(iTestMsgOutputFile, "***seektosyncpoint=%d, syncpointseekwindow=%d, skiptorequestedpos=%d\n",
                    paramkvp[0].value.bool_value,
                    paramkvp[1].value.uint32_value,
                    paramkvp[2].value.bool_value);
            iPlayerCapConfigIF->setParametersSync(NULL, paramkvp, 3, retkvp);
            PVPATB_TEST_IS_TRUE(retkvp == NULL);

            PVPPlaybackPosition start, end;
            start.iIndeterminate = false;
            start.iPosUnit = PVPPBPOSUNIT_SEC;
            start.iMode = PVPPBPOS_MODE_NOW;
            start.iPosValue.sec_value = 20;
            end.iIndeterminate = true;
            fprintf(iTestMsgOutputFile, "***Set PlayBackRange_6 - TargetNPT=%d secs\n", start.iPosValue.sec_value);
            OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRange(start, end, false, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_SETPLAYBACKRANGE7:
        {
            paramkvp[0].value.bool_value = true;
            paramkvp[1].value.uint32_value = 100;
            paramkvp[2].value.bool_value = false;
            fprintf(iTestMsgOutputFile, "***seektosyncpoint=%d, syncpointseekwindow=%d, skiptorequestedpos=%d\n",
                    paramkvp[0].value.bool_value,
                    paramkvp[1].value.uint32_value,
                    paramkvp[2].value.bool_value);
            iPlayerCapConfigIF->setParametersSync(NULL, paramkvp, 3, retkvp);
            PVPATB_TEST_IS_TRUE(retkvp == NULL);

            PVPPlaybackPosition start, end;
            start.iIndeterminate = false;
            start.iPosUnit = PVPPBPOSUNIT_SEC;
            start.iMode = PVPPBPOS_MODE_NOW;
            start.iPosValue.sec_value = 30;
            end.iIndeterminate = true;
            fprintf(iTestMsgOutputFile, "***Set PlayBackRange_7 - TargetNPT=%d secs\n", start.iPosValue.sec_value);
            OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRange(start, end, false, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_STOP:
        {
            fprintf(iTestMsgOutputFile, "***Stopping...\n");
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Stop((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_VIDEO:
        {
            fprintf(iTestMsgOutputFile, "***Removing Video Data Sink...\n");
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_AUDIO:
        {
            fprintf(iTestMsgOutputFile, "***Removing Audio Data Sink...\n");
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_TEXT:
        {
            fprintf(iTestMsgOutputFile, "***Removing Text Data Sink...\n");
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkText, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_RESET:
        {
            fprintf(iTestMsgOutputFile, "***Resetting...\n");
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Reset((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASOURCE:
        {
            fprintf(iTestMsgOutputFile, "***Removing DataSource...\n");
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_CLEANUPANDCOMPLETE:
        {
            fprintf(iTestMsgOutputFile, "***Cleanup And Complete...\n");
            iPlayerCapConfigIF = NULL;

            PVPATB_TEST_IS_TRUE(PVPlayerFactory::DeletePlayer(iPlayer));
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


void pvplayer_async_test_mediaionode_repositionconfig::CommandCompleted(const PVCmdResponse& aResponse)
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
        case STATE_CREATE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                fprintf(iTestMsgOutputFile, "***Create Complete...\n");
                iState = STATE_ADDDATASOURCE;
                RunIfNotReady();
            }
            else
            {
                // QueryInterface() failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                fprintf(iTestMsgOutputFile, "***AddDataSource complete...\n");
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
                fprintf(iTestMsgOutputFile, "***Init complete...\n");
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
                fprintf(iTestMsgOutputFile, "***AddVideoSink complete...\n");
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
                fprintf(iTestMsgOutputFile, "***AddAudioSink complete...\n");
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
                fprintf(iTestMsgOutputFile, "***AddTextSink complete...\n");
                iState = STATE_SETPLAYBACKRANGE1;
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

        case STATE_SETPLAYBACKRANGE1:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                fprintf(iTestMsgOutputFile, "***SetPlayBackRange_1 complete...\n");
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
                fprintf(iTestMsgOutputFile, "***Prepare complete...\n");
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
                fprintf(iTestMsgOutputFile, "***Start complete...\n");
                iState = STATE_SETPLAYBACKRANGE2;
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

        case STATE_SETPLAYBACKRANGE2:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                fprintf(iTestMsgOutputFile, "***SetPlayBackRange_2 complete...\n");
                iState = STATE_SETPLAYBACKRANGE3;
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

        case STATE_SETPLAYBACKRANGE3:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                fprintf(iTestMsgOutputFile, "***SetPlayBackRange_3 complete...\n");
                iState = STATE_SETPLAYBACKRANGE4;
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

        case STATE_SETPLAYBACKRANGE4:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                fprintf(iTestMsgOutputFile, "***SetPlayBackRange_4 complete...\n");
                iState = STATE_SETPLAYBACKRANGE5;
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

        case STATE_SETPLAYBACKRANGE5:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                fprintf(iTestMsgOutputFile, "***SetPlayBackRange_5 complete...\n");
                iState = STATE_SETPLAYBACKRANGE6;
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

        case STATE_SETPLAYBACKRANGE6:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                fprintf(iTestMsgOutputFile, "***SetPlayBackRange_6 complete...\n");
                iState = STATE_SETPLAYBACKRANGE7;
                RunIfNotReady(5000000);
            }
            else
            {
                // SetPlaybackRange failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_SETPLAYBACKRANGE7:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                fprintf(iTestMsgOutputFile, "***SetPlayBackRange_7 complete...\n");
                iState = STATE_STOP;
                RunIfNotReady(5000000);
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
                fprintf(iTestMsgOutputFile, "***Stop complete...\n");
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
                fprintf(iTestMsgOutputFile, "***RemoveVideoSink complete...\n");
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
                fprintf(iTestMsgOutputFile, "***RemoveAudioSink complete...\n");
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
                fprintf(iTestMsgOutputFile, "***RemoveTextSink complete...\n");
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
                fprintf(iTestMsgOutputFile, "***Reset complete...\n");
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
                fprintf(iTestMsgOutputFile, "***RemoveDataSrc complete...\n");
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


void pvplayer_async_test_mediaionode_repositionconfig::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
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


void pvplayer_async_test_mediaionode_repositionconfig::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
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


//
// pvplayer_async_test_mediaionode_eoslooping section
//
void pvplayer_async_test_mediaionode_eoslooping::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_mediaionode_eoslooping::Run()
{
    int error = 0;

    switch (iState)
    {
        case STATE_CREATE:
        {
            iPlayer = NULL;
            fprintf(iTestMsgOutputFile, "***Creating Player...\n");
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
            fprintf(iTestMsgOutputFile, "***Adding Data Source...\n");
            iDataSource = new PVPlayerDataSourceURL;
            oscl_UTF8ToUnicode(iFileName, oscl_strlen(iFileName), iTmpWCharBuffer, 512);
            iFileNameWStr.set(iTmpWCharBuffer, oscl_strlen(iTmpWCharBuffer));
            iDataSource->SetDataSourceURL(iFileNameWStr);
            iDataSource->SetDataSourceFormatType(iFileType);
            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }

        break;

        case STATE_INIT:
        {
            fprintf(iTestMsgOutputFile, "***Initializing...\n");
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Init((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_VIDEO:
        {
            fprintf(iTestMsgOutputFile, "***AddVideoDataSink...\n");
            OSCL_wHeapString<OsclMemAllocator> sinkfilename;
            sinkfilename = OUTPUTNAME_PREPEND_WSTRING;
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_eoslooping_");
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
            fprintf(iTestMsgOutputFile, "***AddAudioDataSink...\n");
            OSCL_wHeapString<OsclMemAllocator> sinkfilename;
            sinkfilename = OUTPUTNAME_PREPEND_WSTRING;
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_eoslooping_");
            if (iCompressedAudio)
            {
                sinkfilename += _STRLIT_WCHAR("compressed_");
            }
            OSCL_wHeapString<OsclMemAllocator> inputfilename;
            RetrieveFilename(iFileNameWStr.get_str(), inputfilename);
            sinkfilename += inputfilename;
            sinkfilename += _STRLIT_WCHAR("_audio.wav");

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
            fprintf(iTestMsgOutputFile, "***AddTextDataSink...\n");
            OSCL_wHeapString<OsclMemAllocator> sinkfilename;
            sinkfilename = OUTPUTNAME_PREPEND_WSTRING;
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_eoslooping_");
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
            fprintf(iTestMsgOutputFile, "***Preparing...\n");
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Prepare((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_START:
        {
            fprintf(iTestMsgOutputFile, "***Starting...\n");
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Start((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ENDTIMENOTREACHED1:
        {
            fprintf(iTestMsgOutputFile, "***End Of Time Not Reached 1...\n");
            // Timeout when player doesn't stop automatically at EOS
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;

        case STATE_SETPLAYBACKRANGE1:
        {
            PVPPlaybackPosition start, end;
            start.iIndeterminate = false;
            start.iPosUnit = PVPPBPOSUNIT_MILLISEC;
            start.iMode = PVPPBPOS_MODE_NOW;
            start.iPosValue.millisec_value = 0;
            end.iIndeterminate = true;
            fprintf(iTestMsgOutputFile, "***SetPlayBackRange_1 - TargetNPT=%d\n",
                    start.iPosValue.millisec_value);
            OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRange(start, end, false, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_RESUME1:
        {
            fprintf(iTestMsgOutputFile, "***Resume_1...\n");
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Resume((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ENDTIMENOTREACHED2:
        {
            fprintf(iTestMsgOutputFile, "***End Of Time Not Reached 2...\n");
            // Timeout when player doesn't stop automatically at EOS
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
            end.iIndeterminate = true;
            fprintf(iTestMsgOutputFile, "***SetPlayBackRange_2 - TargetNPT=%d\n",
                    start.iPosValue.millisec_value);
            OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRange(start, end, false, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_RESUME2:
        {
            fprintf(iTestMsgOutputFile, "***Resume_2...\n");
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Resume((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ENDTIMENOTREACHED3:
        {
            fprintf(iTestMsgOutputFile, "***End Of Time Not Reached 3...\n");
            // Timeout when player doesn't stop automatically at EOS
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;

        case STATE_STOP:
        {
            fprintf(iTestMsgOutputFile, "***Stopping...\n");
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Stop((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_VIDEO:
        {
            fprintf(iTestMsgOutputFile, "***RemoveVideoDataSink...\n");
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_AUDIO:
        {
            fprintf(iTestMsgOutputFile, "***RemoveAudioDataSink...\n");
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_TEXT:
        {
            fprintf(iTestMsgOutputFile, "***RemoveTextDataSink...\n");
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkText, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_RESET:
        {
            fprintf(iTestMsgOutputFile, "***Resetting...\n");
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Reset((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASOURCE:
        {
            fprintf(iTestMsgOutputFile, "***RemoveDataSource...\n");
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_CLEANUPANDCOMPLETE:
        {
            fprintf(iTestMsgOutputFile, "***CleanUpAndComplete...\n");
            PVPATB_TEST_IS_TRUE(PVPlayerFactory::DeletePlayer(iPlayer));
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


void pvplayer_async_test_mediaionode_eoslooping::CommandCompleted(const PVCmdResponse& aResponse)
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
                fprintf(iTestMsgOutputFile, "***AddDataSrc Complete...\n");
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
                fprintf(iTestMsgOutputFile, "***Init Complete...\n");
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
                fprintf(iTestMsgOutputFile, "***AddVideoDataSink Complete...\n");
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
                fprintf(iTestMsgOutputFile, "***AddAudioDataSink Complete...\n");
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
                fprintf(iTestMsgOutputFile, "***AddTextDataSink Complete...\n");
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
                fprintf(iTestMsgOutputFile, "***Prepare Complete...\n");
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
                fprintf(iTestMsgOutputFile, "***Start Complete...\n");
                iState = STATE_ENDTIMENOTREACHED1;
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

        case STATE_SETPLAYBACKRANGE1:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                fprintf(iTestMsgOutputFile, "***SetPlayBackRange1 Complete...\n");
                iState = STATE_RESUME1;
                RunIfNotReady();
            }
            else
            {
                // SetPlaybackRange failed
                PVPATB_TEST_IS_TRUE(false);
                if (aResponse.GetCmdStatus() == PVMFErrNotSupported ||
                        aResponse.GetCmdStatus() == PVMFErrArgument)
                {
                    iState = STATE_STOP;
                    RunIfNotReady();
                }
                else
                {
                    iState = STATE_CLEANUPANDCOMPLETE;
                    RunIfNotReady();
                }
            }
            break;

        case STATE_RESUME1:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                fprintf(iTestMsgOutputFile, "***Resume1 Complete...\n");
                iState = STATE_ENDTIMENOTREACHED2;
                RunIfNotReady(500000000);
            }
            else
            {
                // Resume failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_SETPLAYBACKRANGE2:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                fprintf(iTestMsgOutputFile, "***SetPlayBackRange2 Complete...\n");
                iState = STATE_RESUME2;
                RunIfNotReady();
            }
            else
            {
                // SetPlaybackRange failed
                PVPATB_TEST_IS_TRUE(false);
                if (aResponse.GetCmdStatus() == PVMFErrNotSupported ||
                        aResponse.GetCmdStatus() == PVMFErrArgument)
                {
                    iState = STATE_STOP;
                    RunIfNotReady();
                }
                else
                {
                    iState = STATE_CLEANUPANDCOMPLETE;
                    RunIfNotReady();
                }
            }
            break;

        case STATE_RESUME2:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                fprintf(iTestMsgOutputFile, "***Resume2 Complete...\n");
                iState = STATE_ENDTIMENOTREACHED3;
                RunIfNotReady(500000000);
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
                fprintf(iTestMsgOutputFile, "***Stop Complete...\n");
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
                fprintf(iTestMsgOutputFile, "***RemoveVideoDataSink Complete...\n");
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
                fprintf(iTestMsgOutputFile, "***RemoveAudioDataSink Complete...\n");
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
                fprintf(iTestMsgOutputFile, "***RemoveTextDataSink Complete...\n");
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
                fprintf(iTestMsgOutputFile, "***Reset Complete...\n");
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
                fprintf(iTestMsgOutputFile, "***RemoveDataSrc Complete...\n");
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


void pvplayer_async_test_mediaionode_eoslooping::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    switch (aEvent.GetEventType())
    {
        case PVMFErrNotSupported:
        case PVMFErrArgument:
            // Log the error and don't handle error
            PVPATB_TEST_IS_TRUE(false);
            return;

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


void pvplayer_async_test_mediaionode_eoslooping::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
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
                if (iState == STATE_ENDTIMENOTREACHED1)
                {
                    iState = STATE_SETPLAYBACKRANGE1;
                }
                else if (iState == STATE_ENDTIMENOTREACHED2)
                {
                    iState = STATE_SETPLAYBACKRANGE2;
                }
                else if (iState == STATE_ENDTIMENOTREACHED3)
                {
                    iState = STATE_STOP;
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
    // Check and print out playback position status
    else if (aEvent.GetEventType() == PVMFInfoPositionStatus)
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


//
// pvplayer_async_test_mediaionode_repositionduringprepared section
//
void pvplayer_async_test_mediaionode_repositionduringprepared::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_mediaionode_repositionduringprepared::Run()
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
            iDataSource->SetDataSourceURL(iFileNameWStr);
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
            OSCL_wHeapString<OsclMemAllocator> sinkfilename;
            sinkfilename = OUTPUTNAME_PREPEND_WSTRING;
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_preparedrepo_");
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
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_preparedrepo_");
            if (iCompressedAudio)
            {
                sinkfilename += _STRLIT_WCHAR("compressed_");
            }
            OSCL_wHeapString<OsclMemAllocator> inputfilename;
            RetrieveFilename(iFileNameWStr.get_str(), inputfilename);
            sinkfilename += inputfilename;
            sinkfilename += _STRLIT_WCHAR("_audio.wav");

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
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_preparedrepo_");
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

        case STATE_SETPLAYBACKRANGE:
        {
            PVPPlaybackPosition start, end;
            start.iIndeterminate = false;
            start.iPosUnit = PVPPBPOSUNIT_SEC;
            start.iMode = PVPPBPOS_MODE_NOW;
            start.iPosValue.millisec_value = 20;
            end.iIndeterminate = true;
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


void pvplayer_async_test_mediaionode_repositionduringprepared::CommandCompleted(const PVCmdResponse& aResponse)
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
                RunIfNotReady(1000000);
            }
            else
            {
                // SetPlaybackRange failed
                PVPATB_TEST_IS_TRUE(false);
                if (aResponse.GetCmdStatus() == PVMFErrNotSupported ||
                        aResponse.GetCmdStatus() == PVMFErrArgument)
                {
                    iState = STATE_STOP;
                    RunIfNotReady();
                }
                else
                {
                    iState = STATE_CLEANUPANDCOMPLETE;
                    RunIfNotReady();
                }
            }
            break;

        case STATE_START:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_STOP;
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


void pvplayer_async_test_mediaionode_repositionduringprepared::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    switch (aEvent.GetEventType())
    {
        case PVMFErrNotSupported:
        case PVMFErrArgument:
            // Log the error and don't handle error
            PVPATB_TEST_IS_TRUE(false);
            return;

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


void pvplayer_async_test_mediaionode_repositionduringprepared::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
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


//
// pvplayer_async_test_mediaionode_playsetplaybackrangestopplay section
//
void pvplayer_async_test_mediaionode_playsetplaybackrangestopplay::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_mediaionode_playsetplaybackrangestopplay::Run()
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
            iDataSource->SetDataSourceURL(iFileNameWStr);
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
            OSCL_wHeapString<OsclMemAllocator> sinkfilename;
            sinkfilename = OUTPUTNAME_PREPEND_WSTRING;
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_playrepostopplay_");
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
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_playrepostopplay_");
            if (iCompressedAudio)
            {
                sinkfilename += _STRLIT_WCHAR("compressed_");
            }
            OSCL_wHeapString<OsclMemAllocator> inputfilename;
            RetrieveFilename(iFileNameWStr.get_str(), inputfilename);
            sinkfilename += inputfilename;
            sinkfilename += _STRLIT_WCHAR("_audio.wav");

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
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_playrepostopplay_");
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

        case STATE_PREPARE1:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Prepare((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_START1:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Start((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_SETPLAYBACKRANGE:
        {
            PVPPlaybackPosition start, end;
            start.iIndeterminate = false;
            start.iPosUnit = PVPPBPOSUNIT_SEC;
            start.iMode = PVPPBPOS_MODE_NOW;
            start.iPosValue.millisec_value = 20;
            end.iIndeterminate = true;
            OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRange(start, end, false, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_STOP1:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Stop((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_PREPARE2:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Prepare((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_START2:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Start((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_STOP2:
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


void pvplayer_async_test_mediaionode_playsetplaybackrangestopplay::CommandCompleted(const PVCmdResponse& aResponse)
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
                iState = STATE_PREPARE1;
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

        case STATE_PREPARE1:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_START1;
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

        case STATE_START1:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_SETPLAYBACKRANGE;
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

        case STATE_SETPLAYBACKRANGE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_STOP1;
                RunIfNotReady(5000000);
            }
            else
            {
                // SetPlaybackRange failed
                PVPATB_TEST_IS_TRUE(false);
                if (aResponse.GetCmdStatus() == PVMFErrNotSupported ||
                        aResponse.GetCmdStatus() == PVMFErrArgument)
                {
                    iState = STATE_STOP1;
                    RunIfNotReady();
                }
                else
                {
                    iState = STATE_CLEANUPANDCOMPLETE;
                    RunIfNotReady();
                }
            }
            break;

        case STATE_STOP1:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_PREPARE2;
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

        case STATE_PREPARE2:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_START2;
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

        case STATE_START2:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_STOP2;
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

        case STATE_STOP2:
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


void pvplayer_async_test_mediaionode_playsetplaybackrangestopplay::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    switch (aEvent.GetEventType())
    {
        case PVMFErrNotSupported:
        case PVMFErrArgument:
            // Log the error and don't handle error
            PVPATB_TEST_IS_TRUE(false);
            return;

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


void pvplayer_async_test_mediaionode_playsetplaybackrangestopplay::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
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


//
// pvplayer_async_test_mediaionode_playstopsetplaybackrangeplaystop section
//
void pvplayer_async_test_mediaionode_playstopsetplaybackrangeplaystop::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_mediaionode_playstopsetplaybackrangeplaystop::Run()
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
            iDataSource->SetDataSourceURL(iFileNameWStr);
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
            OSCL_wHeapString<OsclMemAllocator> sinkfilename;
            sinkfilename = OUTPUTNAME_PREPEND_WSTRING;
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_playstopsetplaybackrangeplaystop_");
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
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_playstopsetplaybackrangeplaystop_");
            if (iCompressedAudio)
            {
                sinkfilename += _STRLIT_WCHAR("compressed_");
            }
            OSCL_wHeapString<OsclMemAllocator> inputfilename;
            RetrieveFilename(iFileNameWStr.get_str(), inputfilename);
            sinkfilename += inputfilename;
            sinkfilename += _STRLIT_WCHAR("_audio.wav");

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
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_playstopsetplaybackrangeplaystop_");
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

        case STATE_PREPARE1:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Prepare((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_START1:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Start((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_STOP1:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Stop((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_SETPLAYBACKRANGE:
        {
            PVPPlaybackPosition start, end;
            start.iIndeterminate = false;
            start.iPosUnit = PVPPBPOSUNIT_SEC;
            start.iMode = PVPPBPOS_MODE_NOW;
            start.iPosValue.millisec_value = 30;
            end.iIndeterminate = true;
            OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRange(start, end, false, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_PREPARE2:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Prepare((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_START2:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Start((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_STOP2:
        {
            // Check that current position is not past 5 secs
            // from the expected stop position (40 sec)=>45sec
            PVPPlaybackPosition position;
            position.iPosValue.millisec_value = 999999999;
            position.iPosUnit = PVPPBPOSUNIT_MILLISEC;
            if (iPlayer->GetCurrentPositionSync(position) == PVMFSuccess)
            {
                PVPATB_TEST_IS_TRUE(position.iPosValue.millisec_value <= 45000);
            }
            else
            {
                PVPATB_TEST_IS_TRUE(false);
            }

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


void pvplayer_async_test_mediaionode_playstopsetplaybackrangeplaystop::CommandCompleted(const PVCmdResponse& aResponse)
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
                iState = STATE_PREPARE1;
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

        case STATE_PREPARE1:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_START1;
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

        case STATE_START1:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_STOP1;
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

        case STATE_STOP1:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_SETPLAYBACKRANGE;
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

        case STATE_SETPLAYBACKRANGE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_PREPARE2;
                RunIfNotReady();
            }
            else
            {
                // SetPlaybackRange failed
                PVPATB_TEST_IS_TRUE(false);
                if (aResponse.GetCmdStatus() == PVMFErrNotSupported ||
                        aResponse.GetCmdStatus() == PVMFErrArgument)
                {
                    iState = STATE_STOP2;
                    RunIfNotReady();
                }
                else
                {
                    iState = STATE_CLEANUPANDCOMPLETE;
                    RunIfNotReady();
                }
            }
            break;

        case STATE_PREPARE2:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_START2;
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

        case STATE_START2:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                // Check that current position is not past 5 secs
                // from the requested start position (30 sec)=>35sec
                PVPPlaybackPosition position;
                position.iPosValue.millisec_value = 999999999;
                position.iPosUnit = PVPPBPOSUNIT_MILLISEC;
                if (iPlayer->GetCurrentPositionSync(position) == PVMFSuccess)
                {
                    PVPATB_TEST_IS_TRUE(position.iPosValue.millisec_value <= 35000);
                }
                else
                {
                    PVPATB_TEST_IS_TRUE(false);
                }

                iState = STATE_STOP2;
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

        case STATE_STOP2:
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


void pvplayer_async_test_mediaionode_playstopsetplaybackrangeplaystop::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    switch (aEvent.GetEventType())
    {
        case PVMFErrNotSupported:
        case PVMFErrArgument:
            // Log the error and don't handle error
            PVPATB_TEST_IS_TRUE(false);
            return;

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


void pvplayer_async_test_mediaionode_playstopsetplaybackrangeplaystop::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
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


//
// pvplayer_async_test_mediaionode_setplaybackrangenearendplay section
//
void pvplayer_async_test_mediaionode_setplaybackrangenearendplay::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_mediaionode_setplaybackrangenearendplay::Run()
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
            OSCL_wHeapString<OsclMemAllocator> sinkfilename;
            sinkfilename = OUTPUTNAME_PREPEND_WSTRING;
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_setplaybackrangenearendstart_");
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
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_setplaybackrangenearendstart_");
            if (iCompressedAudio)
            {
                sinkfilename += _STRLIT_WCHAR("compressed_");
            }
            OSCL_wHeapString<OsclMemAllocator> inputfilename;
            RetrieveFilename(iFileNameWStr.get_str(), inputfilename);
            sinkfilename += inputfilename;
            sinkfilename += _STRLIT_WCHAR("_audio.wav");

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
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_setplaybackrangenearendstart_");
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

        case STATE_GETMETADATAVALUES:
        {
            iMetadataKeyList.push_back(OSCL_HeapString<OsclMemAllocator>("duration"));
            iMetadataValueList.clear();
            iNumValues = 0;
            iStartPosition = 0;
            OSCL_TRY(error, iCurrentCmdId = iPlayer->GetMetadataValues(iMetadataKeyList, 0, -1, iNumValues, iMetadataValueList, (OsclAny*) & iContextObject));
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
            start.iPosUnit = PVPPBPOSUNIT_MILLISEC;
            start.iMode = PVPPBPOS_MODE_NOW;
            start.iPosValue.millisec_value = iStartPosition;
            end.iIndeterminate = true;
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

        case STATE_ENDTIMENOTREACHED:
        {
            // Timeout when player doesn't stop automatically at EOS
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_STOP;
            RunIfNotReady();
        }
        break;

        case STATE_STOP:
        {
            // Stop the playback
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


void pvplayer_async_test_mediaionode_setplaybackrangenearendplay::CommandCompleted(const PVCmdResponse& aResponse)
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
                iState = STATE_GETMETADATAVALUES;
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

        case STATE_GETMETADATAVALUES:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_PREPARE;
                RunIfNotReady();

                // If duration is available, set start position
                // to 1 second before end. Default is starting from beginning
                iStartPosition = 0;
                if (iMetadataValueList.empty() == false)
                {
                    for (uint32 i = 0; i < iMetadataValueList.size(); ++i)
                    {
                        // Search for the duration
                        const char* substr = oscl_strstr(iMetadataValueList[i].key, _STRLIT_CHAR("duration;valtype=uint32;timescale="));
                        if (substr != NULL)
                        {
                            uint32 timescale = 1000;
                            if (PV_atoi((substr + 34), 'd', timescale) == false)
                            {
                                // Retrieving timescale failed so default to 1000
                                PVPATB_TEST_IS_TRUE(false);
                                timescale = 1000;
                            }
                            uint32 duration = iMetadataValueList[i].value.uint32_value;
                            if (duration > 0 && timescale > 0)
                            {
                                // Set start position to 1 sec before end of clip in millisec
                                iStartPosition = ((duration * 1000) / timescale) - 1000;
                            }
                        }
                    }
                }
            }
            else
            {
                // GetMetadataValues failed
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
            else
            {
                // SetPlaybackRange failed
                PVPATB_TEST_IS_TRUE(false);
                if (aResponse.GetCmdStatus() == PVMFErrNotSupported ||
                        aResponse.GetCmdStatus() == PVMFErrArgument)
                {
                    iState = STATE_STOP;
                    RunIfNotReady();
                }
                else
                {
                    iState = STATE_CLEANUPANDCOMPLETE;
                    RunIfNotReady();
                }
            }
            break;

        case STATE_START:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ENDTIMENOTREACHED;
                RunIfNotReady(9000000);
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


void pvplayer_async_test_mediaionode_setplaybackrangenearendplay::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    switch (aEvent.GetEventType())
    {
        case PVMFErrNotSupported:
        case PVMFErrArgument:
            // Log the error and don't handle error
            PVPATB_TEST_IS_TRUE(false);
            return;

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


void pvplayer_async_test_mediaionode_setplaybackrangenearendplay::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
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
// pvplayer_async_test_mediaionode_playrepositionnearendofclip section
//
void pvplayer_async_test_mediaionode_playrepositionnearendofclip::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_mediaionode_playrepositionnearendofclip::Run()
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
            iDataSource->SetDataSourceURL(iFileNameWStr);
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
            OSCL_wHeapString<OsclMemAllocator> sinkfilename;
            sinkfilename = OUTPUTNAME_PREPEND_WSTRING;
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_playrepositionnearendofclip_");
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
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_playrepositionnearendofclip_");
            if (iCompressedAudio)
            {
                sinkfilename += _STRLIT_WCHAR("compressed_");
            }
            OSCL_wHeapString<OsclMemAllocator> inputfilename;
            RetrieveFilename(iFileNameWStr.get_str(), inputfilename);
            sinkfilename += inputfilename;
            sinkfilename += _STRLIT_WCHAR("_audio.wav");

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
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_playrepositionnearendofclip_");
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

        case STATE_GETMETADATAVALUES:
        {
            iMetadataKeyList.push_back(OSCL_HeapString<OsclMemAllocator>("duration"));
            iMetadataValueList.clear();
            iNumValues = 0;
            OSCL_TRY(error, iCurrentCmdId = iPlayer->GetMetadataValues(iMetadataKeyList, 0, -1, iNumValues, iMetadataValueList, (OsclAny*) & iContextObject));
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
            start.iPosValue.millisec_value = 0;
            end.iIndeterminate = true;
            OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRange(start, end, false, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            iSetPlayBackRangeIssued = true;
        }
        break;

        case STATE_STOP:
        {
            // Stop the playback
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


void pvplayer_async_test_mediaionode_playrepositionnearendofclip::CommandCompleted(const PVCmdResponse& aResponse)
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
                //iState=STATE_ADDDATASINK_AUDIO;
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
                //iState=STATE_ADDDATASINK_TEXT;
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
                iState = STATE_GETMETADATAVALUES;
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

        case STATE_GETMETADATAVALUES:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_PREPARE;
                RunIfNotReady();

                // If duration is available, get it. Default is 10 sec
                iClipDuration = 10000;
                if (iMetadataValueList.empty() == false)
                {
                    for (uint32 i = 0; i < iMetadataValueList.size(); ++i)
                    {
                        // Search for the duration
                        const char* substr = oscl_strstr(iMetadataValueList[i].key, _STRLIT_CHAR("duration;valtype=uint32;timescale="));
                        if (substr != NULL)
                        {
                            uint32 timescale = 1000;
                            if (PV_atoi((substr + 34), 'd', timescale) == false)
                            {
                                // Retrieving timescale failed so default to 1000
                                PVPATB_TEST_IS_TRUE(false);
                                timescale = 1000;
                            }
                            uint32 duration = iMetadataValueList[i].value.uint32_value;
                            if (duration > 0 && timescale > 0)
                            {
                                // Save the clip duration in milliseconds
                                iClipDuration = ((duration * 1000) / timescale);
                            }
                        }
                    }
                }
            }
            else
            {
                // GetMetadataValues failed
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
                // Play until 1 sec before clip duration then reposition
                RunIfNotReady((iClipDuration - 1000)*1000);
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
                if (aResponse.GetCmdStatus() == PVMFErrNotSupported ||
                        aResponse.GetCmdStatus() == PVMFErrArgument)
                {
                    iState = STATE_STOP;
                    RunIfNotReady();
                }
                else
                {
                    iState = STATE_CLEANUPANDCOMPLETE;
                    RunIfNotReady();
                }
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


void pvplayer_async_test_mediaionode_playrepositionnearendofclip::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    switch (aEvent.GetEventType())
    {
        case PVMFErrNotSupported:
        case PVMFErrArgument:
            // Log the error and don't handle error
            PVPATB_TEST_IS_TRUE(false);
            return;

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


void pvplayer_async_test_mediaionode_playrepositionnearendofclip::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
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
                // Should not receive EOS event since repositioning before the end
                if ((iState == STATE_SETPLAYBACKRANGE) && (iSetPlayBackRangeIssued == false))
                {
                    fprintf(file, "EOS received before we could do SET_PLAYBACKRANGE\n");
                    RunIfNotReady();
                }
                else if ((iState == STATE_SETPLAYBACKRANGE) && (iSetPlayBackRangeIssued == true))
                {
                    PVPATB_TEST_IS_TRUE(false);
                    iState = STATE_STOP;
                    Cancel();
                    RunIfNotReady();
                }
                else if (iState == STATE_STOP)
                {
                    RunIfNotReady();
                }
            }
        }
    }
}


//
// pvplayer_async_test_mediaionode_forwardstep section
//
void pvplayer_async_test_mediaionode_forwardstep::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}

//set this to 1 to get some progress printfs on the console in the frame-step tests.
#define DO_FPRINTF 1

void pvplayer_async_test_mediaionode_forwardstep::Run()
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
            iDataSource->SetDataSourceURL(iFileNameWStr);
            iDataSource->SetDataSourceFormatType(iFileType);
            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }

        break;

        case STATE_INIT:
        {
#if(DO_FPRINTF)
            fprintf(file, "Init\n");
#endif
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Init((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_VIDEO:
        {
            OSCL_wHeapString<OsclMemAllocator> sinkfilename;
            sinkfilename = OUTPUTNAME_PREPEND_WSTRING;
            if (iActiveAudio)
                sinkfilename += _STRLIT_WCHAR("test_player_mionode_forwardstepactiveaudio_");
            else
                sinkfilename += _STRLIT_WCHAR("test_player_mionode_forwardstep_");
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
            if (iActiveAudio)
                sinkfilename += _STRLIT_WCHAR("test_player_mionode_forwardstepactiveaudio_");
            else
                sinkfilename += _STRLIT_WCHAR("test_player_mionode_forwardstep_");
            if (iCompressedAudio)
            {
                sinkfilename += _STRLIT_WCHAR("compressed_");
            }
            OSCL_wHeapString<OsclMemAllocator> inputfilename;
            RetrieveFilename(iFileNameWStr.get_str(), inputfilename);
            sinkfilename += inputfilename;
            sinkfilename += _STRLIT_WCHAR("_audio.wav");

            if (iActiveAudio)
            {//create MIO with active timing simulation
                iMIOFileOutAudio = iMioFactory->CreateAudioOutput(
                                       (OsclAny*) & sinkfilename  // aParam
                                       , NULL   // aObserver
                                       , true   // aActiveTiming
                                       , 10     // aQueueLimit
                                       , false  // sSimFlowControl
                                       , false); // logStrings
            }
            else
            {//create passive MIO
                iMIOFileOutAudio = iMioFactory->CreateAudioOutput((OsclAny*) & sinkfilename, MEDIATYPE_AUDIO, iCompressedAudio);
            }

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
            if (iActiveAudio)
                sinkfilename += _STRLIT_WCHAR("test_player_mionode_forwardstepactiveaudio_");
            else
                sinkfilename += _STRLIT_WCHAR("test_player_mionode_forwardstep_");
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
#if(DO_FPRINTF)
            fprintf(file, "Prepare\n");
#endif
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Prepare((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_START:
        {
#if(DO_FPRINTF)
            fprintf(file, "Start\n");
#endif
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Start((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_PAUSE:
        {
            //pause so we can enter step mode.
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Pause((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ENTERSTEPMODE:
        {
#if(DO_FPRINTF)
            fprintf(file, "Entering Step mode\n");
#endif
            //replace the player timebase with the step-mode timebase.
            OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRate(0, &iSteppingTimebase, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_STARTSTEPPING:
        {
#if(DO_FPRINTF)
            fprintf(file, "Starting Step mode\n");
#endif
            //resume from pause so that data can flow
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Resume((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_STEP:
        {
            //step frame-by-frame for 25 frames, waiting 1/2 second between
            //each frame.
            iCount = 0;
            iSteppingTimebase.GetCount(iCount);
            if (iCount == 25)
            {
                iState = STATE_STOPSTEPPING;
                RunIfNotReady();
            }
            //on 10th step, reposition.
            else if (!iReposComplete && Oscl_Int64_Utils::get_int64_lower32(iCount) == 10)
            {
                iReposComplete = true;//to avoid multiple repos...
#if(DO_FPRINTF)
                fprintf(file, "Repositioning to 20 sec\n");
#endif
                PVPPlaybackPosition start, end;
                start.iIndeterminate = false;
                start.iPosUnit = PVPPBPOSUNIT_SEC;
                start.iMode = PVPPBPOS_MODE_NOW;
                start.iPosValue.sec_value = 20;
                end.iIndeterminate = true;
                OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRange(start, end, false, (OsclAny*) & iContextObject));
                OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            }
            else
            {
                OSCL_TRY(error, iSteppingTimebase.SetCount(++iCount););
                OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
#if(DO_FPRINTF)
                fprintf(file, "Step - %d\n", Oscl_Int64_Utils::get_int64_lower32(iCount));
#endif
                RunIfNotReady(500*1000);
            }
        }
        break;

        case STATE_STOPSTEPPING:
        {
            //pause again so we can exit step mode.
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Pause((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_EXITSTEPMODE:
        {
            //restore normal timebase and 1x play rate.
            OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRate(100000, NULL, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_RESUME:
        {
#if(DO_FPRINTF)
            fprintf(file, "Resuming normal playback\n");
#endif
            //resume from pause with normal timebase.
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Resume((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_STOP:
        {
            //check the count to make sure we actually did the stepping mode.
            int32 count;
            iSteppingTimebase.GetCount(count);
            PVPATB_TEST_IS_TRUE((count == 25) || iEOSReached);
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


void pvplayer_async_test_mediaionode_forwardstep::CommandCompleted(const PVCmdResponse& aResponse)
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
                //play for 2 sec, then go into stepping mode.
                iState = STATE_PAUSE;
                RunIfNotReady(2000*1000);
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
                iState = STATE_ENTERSTEPMODE;
                RunIfNotReady();
            }
            else
            {
                // pause failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ENTERSTEPMODE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_STARTSTEPPING;
                RunIfNotReady();
            }
            else
            {
                // set playback rate failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_STARTSTEPPING:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_STEP;
                RunIfNotReady();
            }
            else
            {
                // start failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_STEP:
            //stepping is synchronous, but there is a SetPlaybackRange
            //command in the middle of the steping sequence.
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                //continue stepping
                RunIfNotReady();
            }
            else
            {
                //set playback range failed.
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_STOPSTEPPING:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_EXITSTEPMODE;
                RunIfNotReady();
            }
            else
            {
                // pause failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_EXITSTEPMODE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_RESUME;
                RunIfNotReady();
            }
            else
            {
                // set playback rate failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_RESUME:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                //play for 2 more seconds, then stop.
                iState = STATE_STOP;
                RunIfNotReady(2000*1000);
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


void pvplayer_async_test_mediaionode_forwardstep::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
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


void pvplayer_async_test_mediaionode_forwardstep::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
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

    switch (aEvent.GetEventType())
    {

        case PVMFInfoEndOfData:
            //if we get end-of-data at any point, then stop playback.
            if (iCount < 25)
            {
                iEOSReached = true;
                fprintf(file, "EOS reached on Step - %d only, So can't Step to till 25\n", Oscl_Int64_Utils::get_int64_lower32(iCount));
            }
            iState = STATE_STOP;
            RunIfNotReady();
            break;

        case PVMFInfoPositionStatus:
        {
            uint8* pos = (uint8*)aEvent.GetLocalBuffer();
            //look for 1 in first byte as a clue that this message has the position status
            //in the local buffer.
            if ((*pos) == 1)
            {
                pos += 4;
                uint32* vpos = (uint32*)pos;
                if ((int32)*vpos < 0)
                    PVPATB_TEST_IS_TRUE(false);
#if(DO_FPRINTF)
                fprintf(file, "Position Status %d\n", *vpos);
#endif
            }
        }
        break;

        default:
            break;//ignore
    }
}

//
// pvplayer_async_test_mediaionode_forwardsteptoeos section
//
void pvplayer_async_test_mediaionode_forwardsteptoeos::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_mediaionode_forwardsteptoeos::Run()
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
            iDataSource->SetDataSourceURL(iFileNameWStr);
            iDataSource->SetDataSourceFormatType(iFileType);
            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }

        break;

        case STATE_INIT:
        {
#if(DO_FPRINTF)
            fprintf(file, "Init\n");
#endif
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Init((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_VIDEO:
        {
            OSCL_wHeapString<OsclMemAllocator> sinkfilename;
            sinkfilename = OUTPUTNAME_PREPEND_WSTRING;
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_forwardsteptoeos_");
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
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_forwardsteptoeos_");
            if (iCompressedAudio)
            {
                sinkfilename += _STRLIT_WCHAR("compressed_");
            }
            OSCL_wHeapString<OsclMemAllocator> inputfilename;
            RetrieveFilename(iFileNameWStr.get_str(), inputfilename);
            sinkfilename += inputfilename;
            sinkfilename += _STRLIT_WCHAR("_audio.wav");

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
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_forwardsteptoeos_");
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
#if(DO_FPRINTF)
            fprintf(file, "Prepare\n");
#endif
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Prepare((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_SETPLAYBACKRATE:
        {
#if(DO_FPRINTF)
            fprintf(file, "SetPlayBackRate\n");
#endif
            //install the frame-stepping timebase.
            OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRate(0, &iSteppingTimebase, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_START:
        {
#if(DO_FPRINTF)
            fprintf(file, "Start\n");
#endif
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Start((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_STEP:
        {
            //step frame-by-frame, waiting 1/2 second between
            //each frame.
            int32 count;
            iSteppingTimebase.GetCount(count);
#if(DO_FPRINTF)
            fprintf(file, "Step - %d\n", count);
#endif
            OSCL_TRY(error, iSteppingTimebase.SetCount(++count););
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            RunIfNotReady(500*1000);
        }
        break;

        case STATE_STOP:
        {
            //check the count to make sure we actually did the stepping mode.
            int32 count;
            iSteppingTimebase.GetCount(count);
            PVPATB_TEST_IS_TRUE(count > 0);
#if(DO_FPRINTF)
            fprintf(file, "Finished %d steps\n", (int32)count);
#endif
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


void pvplayer_async_test_mediaionode_forwardsteptoeos::CommandCompleted(const PVCmdResponse& aResponse)
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
                // SetPlaybackRate failed. Not fatal so just stop test
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_STOP;
                RunIfNotReady();
            }
            break;

        case STATE_START:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                //start stepping.
                iState = STATE_STEP;
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

        case STATE_STEP:
            PVPATB_TEST_IS_TRUE(false);//shouldn't get here-- stepping is synchronous
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


void pvplayer_async_test_mediaionode_forwardsteptoeos::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
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


void pvplayer_async_test_mediaionode_forwardsteptoeos::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
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

    switch (aEvent.GetEventType())
    {

        case PVMFInfoEndOfData:
            //if we get end-of-data at any point, then stop playback.
            iState = STATE_STOP;
            RunIfNotReady();
            break;

        case PVMFInfoPositionStatus:
        {
            uint8* pos = (uint8*)aEvent.GetLocalBuffer();
            //look for 1 in first byte as a clue that this message has the position status
            //in the local buffer.
            if ((*pos) == 1)
            {
                pos += 4;
                uint32* vpos = (uint32*)pos;
                if ((int32)*vpos < 0)
                    PVPATB_TEST_IS_TRUE(false);
#if(DO_FPRINTF)
                //fprintf(file,"Position Status %d\n",*vpos);
#endif
            }
        }
        break;

        default:
            break;//ignore
    }
}

//
// pvplayer_async_test_mediaionode_backwardplayback section
//
void pvplayer_async_test_mediaionode_backwardplayback::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_mediaionode_backwardplayback::Run()
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
            if (iFileType == PVMF_MIME_ASFFF)
            {
                oscl_UTF8ToUnicode(iFileName, oscl_strlen(iFileName), iTmpWCharBuffer, 512);
                iFileNameWStr.set(iTmpWCharBuffer, oscl_strlen(iTmpWCharBuffer));
            }
            else
            {
                // If not ASF file, use the sample ASF file.
                iFileNameWStr = SOURCENAME_PREPEND_WSTRING;
                iFileNameWStr += _STRLIT_WCHAR("test.asf");
                iFileType = PVMF_MIME_ASFFF;
            }

            iDataSource->SetDataSourceURL(iFileNameWStr);
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
            OSCL_wHeapString<OsclMemAllocator> sinkfilename;
            sinkfilename = OUTPUTNAME_PREPEND_WSTRING;
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_backwardplayback_");
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
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_backwardplayback_");
            if (iCompressedAudio)
            {
                sinkfilename += _STRLIT_WCHAR("compressed_");
            }
            OSCL_wHeapString<OsclMemAllocator> inputfilename;
            RetrieveFilename(iFileNameWStr.get_str(), inputfilename);
            sinkfilename += inputfilename;
            sinkfilename += _STRLIT_WCHAR("_audio.wav");

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
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_backwardplayback_");
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

        case STATE_GOBACKWARD:
        {
#if(DO_FPRINTF)
            fprintf(file, "Going backward...\n");
#endif
            OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRate(-100000, NULL, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_GOFORWARD:
        {
#if(DO_FPRINTF)
            fprintf(file, "Going forward...\n");
#endif
            OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRate(100000, NULL, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_PAUSE:
        {
#if(DO_FPRINTF)
            fprintf(file, "Pause...\n");
#endif
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Pause((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_GOBACKWARDSTEP:
        {
#if(DO_FPRINTF)
            fprintf(file, "Going backward in single-step mode...\n");
#endif
            OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRate(-100000, &iSteppingTimebase, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_RESUME:
        {
#if(DO_FPRINTF)
            fprintf(file, "Resume...\n");
#endif
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Resume((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_STEP:
        {
            //step frame-by-frame, waiting 1/2 second between
            //each frame.
            int32 count;
            iSteppingTimebase.GetCount(count);
            iSteppingTimebase.SetCount(count + 1);
#if(DO_FPRINTF)
            fprintf(file, "Step - %d\n", count);
#endif
            RunIfNotReady(500*1000);
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


void pvplayer_async_test_mediaionode_backwardplayback::CommandCompleted(const PVCmdResponse& aResponse)
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
                //play for 40 sec, then go backward
                iState = STATE_GOBACKWARD;
                RunIfNotReady(40000*1000);
            }
            else
            {
                // Start failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GOBACKWARD:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                //play backward for 15 sec, then go forward
                iState = STATE_GOFORWARD;
                RunIfNotReady(15000*1000);
            }
            else
            {
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GOFORWARD:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                //play forward for 15 sec
                iState = STATE_PAUSE;
                RunIfNotReady(15000*1000);
            }
            else
            {
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_PAUSE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                //pause so we can enter step mode.
                iState = STATE_GOBACKWARDSTEP;
                RunIfNotReady();
            }
            else
            {
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GOBACKWARDSTEP:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                //resume from pause.
                iState = STATE_RESUME;
                RunIfNotReady();
            }
            else
            {
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_RESUME:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                //start stepping.
                iState = STATE_STEP;
                RunIfNotReady();
            }
            else
            {
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_STEP:
            PVPATB_TEST_IS_TRUE(false);//shouldn't get here since stepping is synchronous.
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


void pvplayer_async_test_mediaionode_backwardplayback::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
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


void pvplayer_async_test_mediaionode_backwardplayback::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
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

    switch (aEvent.GetEventType())
    {

        case PVMFInfoEndOfData:
            //if we get end-of-data at any point, then stop playback.
            iState = STATE_STOP;
            RunIfNotReady();
#if(DO_FPRINTF)
            fprintf(file, "End of Data!");
#endif
            break;

        case PVMFInfoPositionStatus:
        {
            uint8* pos = (uint8*)aEvent.GetLocalBuffer();
            //look for 1 in first byte as a clue that this message has the position status
            //in the local buffer.
            if ((*pos) == 1)
            {
                pos += 4;
                uint32* vpos = (uint32*)pos;
                if ((int32)*vpos < 0)
                    PVPATB_TEST_IS_TRUE(false);
#if(DO_FPRINTF)
                fprintf(file, "Position Status %d\n", *vpos);
#endif
            }
        }
        break;

        default:
            break;//ignore
    }
}


//
// pvplayer_async_test_mediaionode_backwardforwardplayback section
//
void pvplayer_async_test_mediaionode_backwardforwardplayback::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_mediaionode_backwardforwardplayback::Run()
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
            if (iFileType == PVMF_MIME_ASFFF)
            {
                oscl_UTF8ToUnicode(iFileName, oscl_strlen(iFileName), iTmpWCharBuffer, 512);
                iFileNameWStr.set(iTmpWCharBuffer, oscl_strlen(iTmpWCharBuffer));
            }
            else
            {
                // If not ASF file, use the sample ASF file.
                iFileNameWStr = SOURCENAME_PREPEND_WSTRING;
                iFileNameWStr += _STRLIT_WCHAR("test.asf");
                iFileType = PVMF_MIME_ASFFF;
            }
            iDataSource->SetDataSourceURL(iFileNameWStr);
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
            OSCL_wHeapString<OsclMemAllocator> sinkfilename;
            sinkfilename = OUTPUTNAME_PREPEND_WSTRING;
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_backwardplayback_");
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
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_backwardplayback_");
            if (iCompressedAudio)
            {
                sinkfilename += _STRLIT_WCHAR("compressed_");
            }
            OSCL_wHeapString<OsclMemAllocator> inputfilename;
            RetrieveFilename(iFileNameWStr.get_str(), inputfilename);
            sinkfilename += inputfilename;
            sinkfilename += _STRLIT_WCHAR("_audio.wav");

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
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_backwardplayback_");
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

        case STATE_GOBACKWARD:
        {
#if(DO_FPRINTF)
            fprintf(file, "Going backward...\n");
#endif
            OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRate(-200000, NULL, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_GOFORWARD:
        {
#if(DO_FPRINTF)
            fprintf(file, "Going forward...\n");
#endif
            OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRate(300000, NULL, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_GONORMAL:
        {
#if(DO_FPRINTF)
            fprintf(file, "Going normal forward...\n");
#endif
            OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRate(100000, NULL, (OsclAny*) & iContextObject));
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


void pvplayer_async_test_mediaionode_backwardforwardplayback::CommandCompleted(const PVCmdResponse& aResponse)
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
                //play for 40 sec, then go backward
                iState = STATE_GOBACKWARD;
                RunIfNotReady(40000*1000);
            }
            else
            {
                // Start failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GOBACKWARD:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                //play backward for 10 sec, then go forward
                iState = STATE_GOFORWARD;
                RunIfNotReady(10000*1000);
            }
            else
            {
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GOFORWARD:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                //play forward for 10 sec
                iState = STATE_GONORMAL;
                RunIfNotReady(10000*1000);
            }
            else
            {
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GONORMAL:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_STOP;
                //Run till 20 secs or EOS
                RunIfNotReady(20000000);
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


void pvplayer_async_test_mediaionode_backwardforwardplayback::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
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


void pvplayer_async_test_mediaionode_backwardforwardplayback::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
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

    switch (aEvent.GetEventType())
    {

        case PVMFInfoEndOfData:
            //if we get end-of-data at any point, then stop playback.
            iState = STATE_STOP;
            RunIfNotReady();
#if(DO_FPRINTF)
            fprintf(file, "End of Data!");
#endif
            break;

        case PVMFInfoPositionStatus:
        {
            uint8* pos = (uint8*)aEvent.GetLocalBuffer();
            //look for 1 in first byte as a clue that this message has the position status
            //in the local buffer.
            if ((*pos) == 1)
            {
                pos += 4;
                uint32* vpos = (uint32*)pos;
                if ((int32)*vpos < 0)
                    PVPATB_TEST_IS_TRUE(false);
#if(DO_FPRINTF)
                fprintf(file, "Position Status %d\n", *vpos);
#endif
            }
        }
        break;

        default:
            break;//ignore
    }
}


//
// pvplayer_async_test_mediaionode_pauseneareosbackwardresume section
//
void pvplayer_async_test_mediaionode_pauseneareosbackwardresume::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}

void pvplayer_async_test_mediaionode_pauseneareosbackwardresume::Run()
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
            if (iFileType == PVMF_MIME_ASFFF)
            {
                oscl_UTF8ToUnicode(iFileName, oscl_strlen(iFileName), iTmpWCharBuffer, 512);
                iFileNameWStr.set(iTmpWCharBuffer, oscl_strlen(iTmpWCharBuffer));
            }
            else
            {
                // If not ASF file, use the sample ASF file.
                iFileNameWStr = SOURCENAME_PREPEND_WSTRING;
                iFileNameWStr += _STRLIT_WCHAR("test.asf");
                iFileType = PVMF_MIME_ASFFF;
            }
            iDataSource->SetDataSourceURL(iFileNameWStr);
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
            OSCL_wHeapString<OsclMemAllocator> sinkfilename;
            sinkfilename = OUTPUTNAME_PREPEND_WSTRING;
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_setplaybackrangenearendstart_");
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
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_setplaybackrangenearendstart_");
            if (iCompressedAudio)
            {
                sinkfilename += _STRLIT_WCHAR("compressed_");
            }
            OSCL_wHeapString<OsclMemAllocator> inputfilename;
            RetrieveFilename(iFileNameWStr.get_str(), inputfilename);
            sinkfilename += inputfilename;
            sinkfilename += _STRLIT_WCHAR("_audio.wav");

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
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_setplaybackrangenearendstart_");
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

        case STATE_GETMETADATAVALUES:
        {
            iMetadataKeyList.push_back(OSCL_HeapString<OsclMemAllocator>("duration"));
            iMetadataValueList.clear();
            iNumValues = 0;
            iStartPosition = 0;
            OSCL_TRY(error, iCurrentCmdId = iPlayer->GetMetadataValues(iMetadataKeyList, 0, -1, iNumValues, iMetadataValueList, (OsclAny*) & iContextObject));
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
#if(DO_FPRINTF)
            fprintf(file, "Pausing...\n");
#endif
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Pause((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_SETBACKWARD:
        {
#if(DO_FPRINTF)
            fprintf(file, "Setting backward playback...\n");
#endif
            OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRate(-400000, NULL, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_RESUME:
        {
#if(DO_FPRINTF)
            fprintf(file, "Resuming...\n");
#endif
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Resume((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_GONORMAL:
        {
#if(DO_FPRINTF)
            fprintf(file, "Going normal forward...\n");
#endif
            OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRate(100000, NULL, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_STOP:
        {
            // Stop the playback
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


void pvplayer_async_test_mediaionode_pauseneareosbackwardresume::CommandCompleted(const PVCmdResponse& aResponse)
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
                iState = STATE_GETMETADATAVALUES;
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

        case STATE_GETMETADATAVALUES:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_PREPARE;
                RunIfNotReady();

                // If duration is available, set start position
                // to 1 second before end. Default is starting from beginning
                iStartPosition = 0;
                if (iMetadataValueList.empty() == false)
                {
                    for (uint32 i = 0; i < iMetadataValueList.size(); ++i)
                    {
                        // Search for the duration
                        const char* substr = oscl_strstr(iMetadataValueList[i].key, _STRLIT_CHAR("duration;valtype=uint32;timescale="));
                        if (substr != NULL)
                        {
                            uint32 timescale = 1000;
                            if (PV_atoi((substr + 34), 'd', timescale) == false)
                            {
                                // Retrieving timescale failed so default to 1000
                                PVPATB_TEST_IS_TRUE(false);
                                timescale = 1000;
                            }
                            uint32 duration = iMetadataValueList[i].value.uint32_value;
                            if (duration > 0 && timescale > 0)
                            {
                                // Set start position to 1 sec before end of clip in millisec
                                iStartPosition = ((duration * 1000) / timescale) - 1000;
                            }
                        }
                    }
                }
            }
            else
            {
                // GetMetadataValues failed
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
                RunIfNotReady(iStartPosition*1000);
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
                iState = STATE_SETBACKWARD;
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

        case STATE_SETBACKWARD:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_RESUME;
                RunIfNotReady(5*1000*1000);
            }
            else
            {
                // SetPlaybackRate failed. Not fatal so just stop test
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_STOP;
                RunIfNotReady();
            }
            break;

        case STATE_RESUME:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_GONORMAL;
                RunIfNotReady(10*1000*1000);
            }
            else
            {
                // Resume failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GONORMAL:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_STOP;
                //Run till 20 secs or EOS
                RunIfNotReady(20*1000*1000);
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


void pvplayer_async_test_mediaionode_pauseneareosbackwardresume::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    switch (aEvent.GetEventType())
    {
        case PVMFErrNotSupported:
        case PVMFErrArgument:
            // Log the error and don't handle error
            PVPATB_TEST_IS_TRUE(false);
            return;

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


void pvplayer_async_test_mediaionode_pauseneareosbackwardresume::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
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

    // Check for end of clip event
    if (aEvent.GetEventType() == PVMFInfoEndOfData)
    {
#if(DO_FPRINTF)
        fprintf(file, "\nEnd of Data!");
#endif
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
    else if (aEvent.GetEventType() == PVMFInfoPositionStatus)
    {
        uint8* pos = (uint8*)aEvent.GetLocalBuffer();
        //look for 1 in first byte as a clue that this message has the position status
        //in the local buffer.
        if ((*pos) == 1)
        {
            pos += 4;
            uint32* vpos = (uint32*)pos;
            if ((int32)*vpos < 0)
                PVPATB_TEST_IS_TRUE(false);
#if(DO_FPRINTF)
            fprintf(file, "Position Status %d\n", *vpos);
#endif
        }
    }
}

//
// pvplayer_async_test_mediaionode_multiplepausesetplaybackrateresume section
//
void pvplayer_async_test_mediaionode_multiplepausesetplaybackrateresume::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}

void pvplayer_async_test_mediaionode_multiplepausesetplaybackrateresume::Run()
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
            if (iFileType == PVMF_MIME_ASFFF)
            {
                oscl_UTF8ToUnicode(iFileName, oscl_strlen(iFileName), iTmpWCharBuffer, 512);
                iFileNameWStr.set(iTmpWCharBuffer, oscl_strlen(iTmpWCharBuffer));
            }
            else
            {
                // If not ASF file, use the sample ASF file.
                iFileNameWStr = SOURCENAME_PREPEND_WSTRING;
                iFileNameWStr += _STRLIT_WCHAR("test.asf");
                iFileType = PVMF_MIME_ASFFF;
            }
            iDataSource->SetDataSourceURL(iFileNameWStr);
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
            OSCL_wHeapString<OsclMemAllocator> sinkfilename;
            sinkfilename = OUTPUTNAME_PREPEND_WSTRING;
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_setplaybackrangenearendstart_");
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
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_setplaybackrangenearendstart_");
            if (iCompressedAudio)
            {
                sinkfilename += _STRLIT_WCHAR("compressed_");
            }
            OSCL_wHeapString<OsclMemAllocator> inputfilename;
            RetrieveFilename(iFileNameWStr.get_str(), inputfilename);
            sinkfilename += inputfilename;
            sinkfilename += _STRLIT_WCHAR("_audio.wav");

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
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_setplaybackrangenearendstart_");
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

        case STATE_GETMETADATAVALUES:
        {
            iMetadataKeyList.push_back(OSCL_HeapString<OsclMemAllocator>("duration"));
            iMetadataValueList.clear();
            iNumValues = 0;
            iStartPosition = 0;
            OSCL_TRY(error, iCurrentCmdId = iPlayer->GetMetadataValues(iMetadataKeyList, 0, -1, iNumValues, iMetadataValueList, (OsclAny*) & iContextObject));
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
#if(DO_FPRINTF)
            fprintf(file, "Pausing...\n");
#endif
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Pause((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_SETPLAYBACKRATE:
        {
            if (0 == iNumSetRate)
                iPlaybackRate = 2;
            else if (1 == iNumSetRate)
                iPlaybackRate = -1;
            else if (2 == iNumSetRate)
                iPlaybackRate = 3;
            else if (3 == iNumSetRate)
                iPlaybackRate = -3;
            else if (4 == iNumSetRate)
                iPlaybackRate = 4;
            else if (5 == iNumSetRate)
                iPlaybackRate = -3;
            else if (6 == iNumSetRate)
                iPlaybackRate = 5;
            else if (7 == iNumSetRate)
                iPlaybackRate = -4;
            else if (8 == iNumSetRate)
                iPlaybackRate = 3;
            else if (9 == iNumSetRate)
                iPlaybackRate = 1;

#if(DO_FPRINTF)
            fprintf(file, "Setting playback rate to %dX...\n", iPlaybackRate);
#endif
            OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRate(iPlaybackRate * 100000, NULL, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_RESUME:
        {
#if(DO_FPRINTF)
            fprintf(file, "Resuming...\n");
#endif
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Resume((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_GONORMAL:
        {
#if(DO_FPRINTF)
            fprintf(file, "Going normal forward...\n");
#endif
            OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRate(100000, NULL, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_STOP:
        {
            // Stop the playback
#if(DO_FPRINTF)
            fprintf(file, "Stopping...\n");
#endif
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

void pvplayer_async_test_mediaionode_multiplepausesetplaybackrateresume::CommandCompleted(const PVCmdResponse& aResponse)
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
                iState = STATE_GETMETADATAVALUES;
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

        case STATE_GETMETADATAVALUES:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_PREPARE;
                RunIfNotReady();

                // If duration is available, set start position
                // to 1 second before end. Default is starting from beginning
                iStartPosition = 0;
                if (iMetadataValueList.empty() == false)
                {
                    for (uint32 i = 0; i < iMetadataValueList.size(); ++i)
                    {
                        // Search for the duration
                        const char* substr = oscl_strstr(iMetadataValueList[i].key, _STRLIT_CHAR("duration;valtype=uint32;timescale="));
                        if (substr != NULL)
                        {
                            uint32 timescale = 1000;
                            if (PV_atoi((substr + 34), 'd', timescale) == false)
                            {
                                // Retrieving timescale failed so default to 1000
                                PVPATB_TEST_IS_TRUE(false);
                                timescale = 1000;
                            }
                            uint32 duration = iMetadataValueList[i].value.uint32_value;
                            if (duration > 0 && timescale > 0)
                            {
                                // Set start position to 1 sec before end of clip in millisec
                                iStartPosition = ((duration * 1000) / timescale) - 1000;
                            }
                        }
                    }
                }
            }
            else
            {
                // GetMetadataValues failed
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
                RunIfNotReady(FIRST_PAUSE_AFTER_START_LOCAL*1000*1000);
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
                iState = STATE_SETPLAYBACKRATE;
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

        case STATE_SETPLAYBACKRATE:

            iNumSetRate++;

            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_RESUME;
                RunIfNotReady(PAUSE_RESUME_INTERVAL_LOCAL*1000*1000);
            }
            else
            {
                // SetPlaybackRate failed. Not fatal so just stop test
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_STOP;
                RunIfNotReady();
            }
            break;

        case STATE_RESUME:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if (iNumSetRate < 10)
                    iState = STATE_PAUSE;
                else
                    iState = STATE_GONORMAL;
                RunIfNotReady(SEQUENTIAL_PAUSE_INTERVAL_LOCAL*1000*1000);
            }
            else
            {
                // Resume failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GONORMAL:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_STOP;
                //Run till 20 secs or EOS
                RunIfNotReady(20*1000*1000);
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

void pvplayer_async_test_mediaionode_multiplepausesetplaybackrateresume::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    switch (aEvent.GetEventType())
    {
        case PVMFErrNotSupported:
        case PVMFErrArgument:
            // Log the error and don't handle error
            PVPATB_TEST_IS_TRUE(false);
            return;

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


void pvplayer_async_test_mediaionode_multiplepausesetplaybackrateresume::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
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

    // Check for end of clip event
    if (aEvent.GetEventType() == PVMFInfoEndOfData)
    {
#if(DO_FPRINTF)
        fprintf(file, "\nEnd of Data!");
#endif
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
    else if (aEvent.GetEventType() == PVMFInfoPositionStatus)
    {
        uint8* pos = (uint8*)aEvent.GetLocalBuffer();
        //look for 1 in first byte as a clue that this message has the position status
        //in the local buffer.
        if ((*pos) == 1)
        {
            pos += 4;
            uint32* vpos = (uint32*)pos;
            if ((int32)*vpos < 0)
                PVPATB_TEST_IS_TRUE(false);
#if(DO_FPRINTF)
            fprintf(file, "Position Status %d\n", *vpos);
#endif
        }
    }
}

//
// pvplayer_async_test_mediaionode_backwardneareosforwardnearbos section
//
void pvplayer_async_test_mediaionode_backwardneareosforwardnearbos::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}

void pvplayer_async_test_mediaionode_backwardneareosforwardnearbos::Run()
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
            if (iFileType == PVMF_MIME_ASFFF)
            {
                oscl_UTF8ToUnicode(iFileName, oscl_strlen(iFileName), iTmpWCharBuffer, 512);
                iFileNameWStr.set(iTmpWCharBuffer, oscl_strlen(iTmpWCharBuffer));
            }
            else
            {
                // If not ASF file, use the sample ASF file.
                iFileNameWStr = SOURCENAME_PREPEND_WSTRING;
                iFileNameWStr += _STRLIT_WCHAR("test.asf");
                iFileType = PVMF_MIME_ASFFF;
            }
            iDataSource->SetDataSourceURL(iFileNameWStr);
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
            OSCL_wHeapString<OsclMemAllocator> sinkfilename;
            sinkfilename = OUTPUTNAME_PREPEND_WSTRING;
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_setplaybackrangenearendstart_");
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
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_setplaybackrangenearendstart_");
            if (iCompressedAudio)
            {
                sinkfilename += _STRLIT_WCHAR("compressed_");
            }
            OSCL_wHeapString<OsclMemAllocator> inputfilename;
            RetrieveFilename(iFileNameWStr.get_str(), inputfilename);
            sinkfilename += inputfilename;
            sinkfilename += _STRLIT_WCHAR("_audio.wav");

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
            sinkfilename += _STRLIT_WCHAR("test_player_mionode_setplaybackrangenearendstart_");
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

        case STATE_GETMETADATAVALUES:
        {
            iMetadataKeyList.push_back(OSCL_HeapString<OsclMemAllocator>("duration"));
            iMetadataValueList.clear();
            iNumValues = 0;
            iStartPosition = 0;
            OSCL_TRY(error, iCurrentCmdId = iPlayer->GetMetadataValues(iMetadataKeyList, 0, -1, iNumValues, iMetadataValueList, (OsclAny*) & iContextObject));
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
#if(DO_FPRINTF)
            fprintf(file, "Pausing...\n");
#endif
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Pause((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_SETBACKWARD:
        {
#if(DO_FPRINTF)
            fprintf(file, "Setting backward playback -4x...\n");
#endif
            OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRate(-400000, NULL, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_RESUME:
        {
#if(DO_FPRINTF)
            fprintf(file, "Resuming...\n");
#endif
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Resume((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_GONORMAL:
        {
#if(DO_FPRINTF)
            fprintf(file, "Going normal forward...\n");
#endif
            OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRate(100000, NULL, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_STOP:
        {
            // Stop the playback
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


void pvplayer_async_test_mediaionode_backwardneareosforwardnearbos::CommandCompleted(const PVCmdResponse& aResponse)
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
                iState = STATE_GETMETADATAVALUES;
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

        case STATE_GETMETADATAVALUES:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_PREPARE;
                RunIfNotReady();

                // If duration is available, set start position
                // to 1 second before end. Default is starting from beginning
                iStartPosition = 0;
                if (iMetadataValueList.empty() == false)
                {
                    for (uint32 i = 0; i < iMetadataValueList.size(); ++i)
                    {
                        // Search for the duration
                        const char* substr = oscl_strstr(iMetadataValueList[i].key, _STRLIT_CHAR("duration;valtype=uint32;timescale="));
                        if (substr != NULL)
                        {
                            uint32 timescale = 1000;
                            if (PV_atoi((substr + 34), 'd', timescale) == false)
                            {
                                // Retrieving timescale failed so default to 1000
                                PVPATB_TEST_IS_TRUE(false);
                                timescale = 1000;
                            }
                            uint32 duration = iMetadataValueList[i].value.uint32_value;
                            if (duration > 0 && timescale > 0)
                            {
                                // Set start position to 1 sec before end of clip in millisec
                                iStartPosition = ((duration * 1000) / timescale) - 1000;
                            }
                        }
                    }
                }
            }
            else
            {
                // GetMetadataValues failed
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
                RunIfNotReady(iStartPosition*1000);
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
                iState = STATE_SETBACKWARD;
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

        case STATE_SETBACKWARD:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_RESUME;
                RunIfNotReady(5*1000*1000);
            }
            else
            {
                // SetPlaybackRate failed. Not fatal so just stop test
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_STOP;
                RunIfNotReady();
            }
            break;

        case STATE_RESUME:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_GONORMAL;
                RunIfNotReady((iStartPosition / 4)*1000);
            }
            else
            {
                // Resume failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GONORMAL:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_STOP;
                //Run till 20 secs or EOS
                RunIfNotReady(20*1000*1000);
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
                if (iNumPlay == 0)
                    iState = STATE_PREPARE;
                else
                    iState = STATE_REMOVEDATASINK_VIDEO;

                iNumPlay++;
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


void pvplayer_async_test_mediaionode_backwardneareosforwardnearbos::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    switch (aEvent.GetEventType())
    {
        case PVMFErrNotSupported:
        case PVMFErrArgument:
            // Log the error and don't handle error
            PVPATB_TEST_IS_TRUE(false);
            return;

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


void pvplayer_async_test_mediaionode_backwardneareosforwardnearbos::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
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

    // Check for end of clip event
    if (aEvent.GetEventType() == PVMFInfoEndOfData)
    {
#if(DO_FPRINTF)
        fprintf(file, "\nEnd of Data!");
#endif
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
    else if (aEvent.GetEventType() == PVMFInfoPositionStatus)
    {
        uint8* pos = (uint8*)aEvent.GetLocalBuffer();
        //look for 1 in first byte as a clue that this message has the position status
        //in the local buffer.
        if ((*pos) == 1)
        {
            pos += 4;
            uint32* vpos = (uint32*)pos;
            if ((int32)*vpos < 0)
                PVPATB_TEST_IS_TRUE(false);
#if(DO_FPRINTF)
            fprintf(file, "Position Status %d\n", *vpos);
#endif
        }
    }
}


