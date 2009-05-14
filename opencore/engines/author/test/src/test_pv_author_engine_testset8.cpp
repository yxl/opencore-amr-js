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
#ifndef TEST_PV_AUTHOR_ENGINE_TESTSET7_H_INCLUDED
#include "test_pv_author_engine_testset8.h"
#endif

#ifndef PVMF_COMPOSER_SIZE_AND_DURATION_H_INCLUDED
#include "pvmf_composer_size_and_duration.h"
#endif

#ifndef PVMF_FILEOUTPUT_CONFIG_H_INCLUDED
#include "pvmf_fileoutput_config.h"
#endif

#ifndef PVMP4FFCN_CLIPCONFIG_H_INCLUDED
#include "pvmp4ffcn_clipconfig.h"
#endif

#ifndef PV_MP4_H263_ENC_EXTENSION_H_INCLUDED
#include "pvmp4h263encextension.h"
#endif

#ifndef PVAETEST_NODE_CONFIG_H_INCLUDED
#include "pvaetest_node_config.h"
#endif

void pv_mediainput_async_test_delete::StartTest()
{
    AddToScheduler();
    iState = PVAE_CMD_CREATE;
    RunIfNotReady();
}

////////////////////////////////////////////////////////////////////////////
void pv_mediainput_async_test_delete::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    OSCL_UNUSED_ARG(aEvent);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR, (0, "pv_mediainput_async_test_delete::HandleErrorEvent"));
    iState = PVAE_CMD_RESET;
    RunIfNotReady();
}

////////////////////////////////////////////////////////////////////////////
void pv_mediainput_async_test_delete::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "pv_mediainput_async_test_delete::HandleInformationalEvent"));
    OsclAny* eventData = NULL;
    switch (aEvent.GetEventType())
    {
        case PVMF_COMPOSER_MAXFILESIZE_REACHED:
        case PVMF_COMPOSER_MAXDURATION_REACHED:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "pv_mediainput_async_test_delete::HandleNodeInformationalEvent: Max file size reached"));
            Cancel();
            PVPATB_TEST_IS_TRUE(true);
            iObserver->CompleteTest(*iTestCase);
            break;

        case PVMF_COMPOSER_DURATION_PROGRESS:
            aEvent.GetEventData(eventData);
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "pv_mediainput_async_test_delete::HandleNodeInformationalEvent: Duration progress: %d ms",
                             (int32)eventData));
            break;

        case PVMF_COMPOSER_FILESIZE_PROGRESS:
            aEvent.GetEventData(eventData);
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "pv_mediainput_async_test_delete::HandleNodeInformationalEvent: File size progress: %d bytes",
                             (int32)eventData));
            break;

        case PVMF_COMPOSER_EOS_REACHED:
            //Engine already stopped at EOS so send reset command.
            iState = PVAE_CMD_RESET;
            //cancel recording timeout scheduled for timer object.
            Cancel();
            RunIfNotReady();
            break;

        default:
            break;
    }
}

////////////////////////////////////////////////////////////////////////////
bool pv_mediainput_async_test_delete::CreateTestInputs()
{
    int32 status = 0;
    int32 error = 0;
    iFileParser = NULL;
    iFileServer.Connect();

    if (iMediaInputType == PVMF_MIME_AVIFF)
    {
        OSCL_TRY(error, iFileParser = PVAviFile::CreateAviFileParser(iInputFileName, error, &iFileServer););

        if (error || (NULL == iFileParser))
        {
            if (iFileParser)
            {
                goto ERROR_CODE;
            }
            else
            {
                return false;
            }
        }

        uint32 numStreams = ((PVAviFile*)iFileParser)->GetNumStreams();

        iAddAudioMediaTrack = false;
        iAddVideoMediaTrack = false;

        for (uint32 ii = 0; ii < numStreams; ii++)
        {
            if (oscl_strstr(((PVAviFile*)iFileParser)->GetStreamMimeType(ii).get_cstr(), "audio"))
            {
                iAddAudioMediaTrack = true;
            }

            if (oscl_strstr(((PVAviFile*)iFileParser)->GetStreamMimeType(ii).get_cstr(), "video"))
            {
                iAddVideoMediaTrack = true;
            }

        }

    }
    if (iMediaInputType == PVMF_MIME_WAVFF)
    {
        OSCL_TRY(error, iFileParser = OSCL_NEW(PV_Wav_Parser, ()););
        if (error || (NULL == iFileParser))
        {
            return false;
        }
        if (((PV_Wav_Parser*)iFileParser)->InitWavParser(iInputFileName, NULL) != PVWAVPARSER_OK)
        {
            goto ERROR_CODE;
        }

        iAddAudioMediaTrack = true;
        iAddVideoMediaTrack = false;
    }

    {
        PVMIOControlComp MIOComp(iMediaInputType, (OsclAny*)iFileParser, 0);

        status = MIOComp.CreateMIOInputNode(false, iMediaInputType, iInputFileName);
        if (status != PVMFSuccess)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                            (0, "pv_mediainput_async_test_opencomposestop::CreateTestInputs: Error - CreateMIOInputNode failed"));

            goto ERROR_CODE;

        }

        iMIOComponent = MIOComp;
        if (!AddDataSource())
        {
            //delete any MIO Comp created.
            MIOComp.DeleteInputNode();
            goto ERROR_CODE;
        }

        return true;
    }

ERROR_CODE:
    {
        //remove file parser
        if (iMediaInputType == PVMF_MIME_AVIFF)
        {
            PVAviFile* fileparser = OSCL_STATIC_CAST(PVAviFile*, iFileParser);
            PVAviFile::DeleteAviFileParser(fileparser);
            fileparser = NULL;
            iFileParser = NULL;
        }
        if (iMediaInputType == PVMF_MIME_WAVFF)
        {
            PV_Wav_Parser* fileparser = OSCL_STATIC_CAST(PV_Wav_Parser*, iFileParser);
            delete(fileparser);
            fileparser = NULL;
            iFileParser = NULL;
        }

        return false;
    }

}
////////////////////////////////////////////////////////////////////////////
bool pv_mediainput_async_test_delete::AddDataSource()
{
    int32 err = 0;
    uint32 noOfNodes = iMIOComponent.iMIONode.size();

    OSCL_TRY(err,
             for (uint32 ii = 0; ii < noOfNodes; ii++)
{
    AddEngineCommand();

        //OSCL_TRY(err, iAuthor->AddDataSource(*(iMIOComponent.iMIONode[ii]), (OsclAny*)iAuthor););
        iAuthor->AddDataSource(*(iMIOComponent.iMIONode[ii]), (OsclAny*)iAuthor);
    }
            );
    if (err != OSCL_ERR_NONE)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                        (0, "pvauthor_async_test_miscellaneous::AddDataSource: Error - iAuthor->AddDataSource failed. err=0x%x", err));

        iMIOComponent.DeleteInputNode();
        return false;
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////
bool pv_mediainput_async_test_delete::ConfigComposer()
{
    if (!ConfigOutputFile())
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "pv_mediainput_async_test_delete::ConfigComposer: Error - ConfigOutputFile failed"));

        return false;
    }

    if ((oscl_strstr(iComposerMimeType.get_str(), "mp4")) || (oscl_strstr(iComposerMimeType.get_str(), "3gp")))
    {
        if (!ConfigMp43gpComposer())
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "pv_mediainput_async_test_delete::ConfigComposer: Error - ConfigMp43gpComposer failed"));

            return false;
        }
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////
bool pv_mediainput_async_test_delete::ConfigOutputFile()
{

    PvmfFileOutputNodeConfigInterface* clipConfig = OSCL_STATIC_CAST(PvmfFileOutputNodeConfigInterface*, iComposerConfig);
    if (!clipConfig)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "pv_mediainput_async_test_delete::ConfigOutputFile: Error - Invalid iComposerConfig"));

        return false;
    }

    if (clipConfig->SetOutputFileName(iOutputFileName) != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "pv_mediainput_async_test_delete::ConfigOutputFile: Error - SetOutputFileName failed"));

        return false;
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////
bool pv_mediainput_async_test_delete::ConfigMp43gpComposer()
{

    PVMp4FFCNClipConfigInterface* clipConfig;
    clipConfig = OSCL_STATIC_CAST(PVMp4FFCNClipConfigInterface*, iComposerConfig);
    if (!clipConfig)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "pv_mediainput_async_test_delete::ConfigMp43gpComposer: Error - iComposerConfig==NULL"));

        return false;
    }

    OSCL_wHeapString<OsclMemAllocator> versionString = _STRLIT("version");
    OSCL_wHeapString<OsclMemAllocator> titleString = _STRLIT("title");
    OSCL_wHeapString<OsclMemAllocator> authorString = _STRLIT("author");
    OSCL_wHeapString<OsclMemAllocator> copyrightString = _STRLIT("copyright");
    OSCL_wHeapString<OsclMemAllocator> descriptionString = _STRLIT("description");
    OSCL_wHeapString<OsclMemAllocator> ratingString = _STRLIT("rating");
    OSCL_wHeapString<OsclMemAllocator> iAlbumTitle = _STRLIT("albumtitle");
    uint16 iRecordingYear = 2008;
    OSCL_HeapString<OsclMemAllocator> lang_code = "eng";

    clipConfig->SetOutputFileName(iOutputFileName);
    clipConfig->SetPresentationTimescale(1000);
    clipConfig->SetVersion(versionString, lang_code);
    clipConfig->SetTitle(titleString, lang_code);
    clipConfig->SetAuthor(authorString, lang_code);
    clipConfig->SetCopyright(copyrightString, lang_code);
    clipConfig->SetDescription(descriptionString, lang_code);
    clipConfig->SetRating(ratingString, lang_code);
    clipConfig->SetAlbumInfo(iAlbumTitle, lang_code);
    clipConfig->SetRecordingYear(iRecordingYear);


    return true;
}

////////////////////////////////////////////////////////////////////////////
bool pv_mediainput_async_test_delete::AddMediaTrack()
{
    PVMIOControlComp MIOComp;

    if (iAddAudioMediaTrack)
    {
        if (iMediaInputType == PVMF_MIME_AVIFF)
        {
            Oscl_Vector<uint32, OsclMemAllocator> audioStrNum;

            audioStrNum = (iMIOComponent.iPVAviFile)->GetAudioStreamCountList();

            if (audioStrNum.size() == 0)
            {
                return false;
            }

            iAuthor->AddMediaTrack(*(iMIOComponent.iMIONode[audioStrNum[0]]), iAudioEncoderMimeType,
                                   iComposer, iAudioEncoderConfig, (OsclAny*)iAuthor);

            AddEngineCommand();

        }
        if (iMediaInputType == PVMF_MIME_WAVFF)
        {
            PVWAVFileInfo wavFileInfo;
            (iMIOComponent.iPVWavFile)->RetrieveFileInfo(wavFileInfo);


            iAuthor->AddMediaTrack(*(iMIOComponent.iMIONode[0]), iAudioEncoderMimeType,
                                   iComposer, iAudioEncoderConfig, (OsclAny*)iAuthor);

            AddEngineCommand();


        }


    }

    if (iAddVideoMediaTrack)
    {
        if (iMediaInputType == PVMF_MIME_AVIFF)
        {
            Oscl_Vector<uint32, OsclMemAllocator> vidStrNum;
            vidStrNum = (iMIOComponent.iPVAviFile)->GetVideoStreamCountList();

            if (vidStrNum.size() == 0)
            {
                return false;
            }

            iAuthor->AddMediaTrack(*(iMIOComponent.iMIONode[vidStrNum[0]]), iVideoEncoderMimeType,
                                   iComposer, iVideoEncoderConfig, (OsclAny*)iAuthor);

            AddEngineCommand();

        }
        if (iMediaInputType == PVMF_MIME_WAVFF)
        {
            return false;
        }

    }

    return true;
}

////////////////////////////////////////////////////////////////////////////
bool pv_mediainput_async_test_delete::ConfigureVideoEncoder()
{

    PVMp4H263EncExtensionInterface* config;
    config = OSCL_STATIC_CAST(PVMp4H263EncExtensionInterface*, iVideoEncoderConfig);
    if (!config)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "pv_mediainput_async_test_delete::ConfigureVideoEncoder: No configuration needed"));

        return true;
    }

    uint32 width = 0;
    uint32 height = 0;
    OsclFloat frameRate = 0;
    uint32 frameInterval = 0;

    if (iMediaInputType == PVMF_MIME_AVIFF)
    {
        Oscl_Vector<uint32, OsclMemAllocator> vidStrNum =
            (iMIOComponent.iPVAviFile)->GetVideoStreamCountList();

        width = (iMIOComponent.iPVAviFile)->GetWidth(vidStrNum[0]);
        bool orient = false;
        height = (iMIOComponent.iPVAviFile)->GetHeight(orient, vidStrNum[0]);
        frameRate = (iMIOComponent.iPVAviFile)->GetFrameRate(vidStrNum[0]);
        frameInterval = (iMIOComponent.iPVAviFile)->GetFrameDuration();

    }

    config->SetNumLayers(KNumLayers);
    config->SetOutputBitRate(0, KVideoBitrate);
    config->SetOutputFrameSize(0, width , height);
    config->SetOutputFrameRate(0, frameRate);
    config->SetIFrameInterval(KVideoIFrameInterval);
    config->SetSceneDetection(true);

    return true;
}

bool pv_mediainput_async_test_delete::ConfigureAudioEncoder()
{

    PVAudioEncExtensionInterface* config;
    config = OSCL_STATIC_CAST(PVAudioEncExtensionInterface*, iAudioEncoderConfig);
    if (!config)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "pv_mediainput_async_test_delete::Encoder: No configuration needed"));

        return true;
    }

    if (!PVAETestNodeConfig::ConfigureAudioEncoder(iAudioEncoderConfig, iAudioEncoderMimeType))
    {
        return false;
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////
void pv_mediainput_async_test_delete::ResetAuthorConfig()
{
    if (iComposerConfig)
    {
        iComposerConfig->removeRef();
        iComposerConfig = NULL;
    }
    if (iAudioEncoderConfig)
    {
        iAudioEncoderConfig->removeRef();
        iAudioEncoderConfig = NULL;
    }
    if (iVideoEncoderConfig)
    {
        iVideoEncoderConfig->removeRef();
        iVideoEncoderConfig = NULL;
    }
}

////////////////////////////////////////////////////////////////////////////
void pv_mediainput_async_test_delete::Cleanup()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "pv_mediainput_async_test_delete::Cleanup"));

    iComposer = NULL;

    ResetAuthorConfig();
    if (iAuthor)
    {
        PVAuthorEngineFactory::DeleteAuthor(iAuthor);
        iAuthor = NULL;
    }

    iOutputFileName = NULL;
    iMIOComponent.DeleteInputNode();
    if (iMediaInputType == PVMF_MIME_AVIFF)
    {
        PVAviFile* fileparser = OSCL_STATIC_CAST(PVAviFile*, iFileParser);
        PVAviFile::DeleteAviFileParser(fileparser);
        fileparser = NULL;
        iFileParser = NULL;
    }
    if (iMediaInputType == PVMF_MIME_WAVFF)
    {
        PV_Wav_Parser* fileparser = OSCL_STATIC_CAST(PV_Wav_Parser*, iFileParser);
        delete(fileparser);
        fileparser = NULL;
        iFileParser = NULL;
    }
    iFileParser = NULL;

}


////////////////////////////////////////////////////////////////////////////
void pv_mediainput_async_test_delete::Run()
{
    if (IsEngineCmdPending())
    {
        return;
    }

    switch (iState)
    {
        case PVAE_CMD_CREATE:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "******pv_mediainput_async_test_delete::iTestCaseNum:%d******", iTestCaseNum));

            iAuthor = PVAuthorEngineFactory::CreateAuthor(this, this, this);
            if (!iAuthor)
            {
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_CLEANUPANDCOMPLETE;
                RunIfNotReady();

            }
            else
            {
                if (PVAE_CMD_CREATE == iCurrentResetState)
                {
                    iNextResetState = PVAE_CMD_OPEN;
                    iState = PVAE_CMD_CLEANUPANDCOMPLETE;
                    PVPATB_TEST_IS_TRUE(true);
                    RunIfNotReady();
                }
                else
                {
                    iState = PVAE_CMD_OPEN;
                    RunIfNotReady();
                }
            }
        }
        break;

        case PVAE_CMD_OPEN:
        {
            iAuthor->Open((OsclAny*)iAuthor);
        }
        break;

        case PVAE_CMD_ADD_DATA_SOURCE:
        {
            bool aStatus = CreateTestInputs();
            if (aStatus == 0) //Failed while creating test input
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                (0, "pv_mediainput_async_test_delete::CreateTestInputs: Error - failed"));

                PVPATB_TEST_IS_TRUE(true); //expected to come here

                iState = PVAE_CMD_CLEANUPANDCOMPLETE;

                RunIfNotReady();
            }

        }
        break;

        case PVAE_CMD_SELECT_COMPOSER:
        {
            iAuthor->SelectComposer(iComposerMimeType, iComposerConfig,
                                    (OsclAny*)iAuthor);
        }
        break;

        case PVAE_CMD_ADD_MEDIA_TRACK:
        {
            if (!AddMediaTrack())
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                (0, "pv_mediainput_async_test_opencomposestop::AddMediaTrack Error - No track added"));
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_REMOVE_DATA_SOURCE;
                RunIfNotReady();
            }
        }
        break;

        case PVAE_CMD_INIT:
        {
            iAuthor->Init((OsclAny*)iAuthor);
        }
        break;

        case PVAE_CMD_START:
        {
            iAuthor->Start();
        }
        break;

        case PVAE_CMD_STOP:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                            (0, "pv_mediainput_async_test_delete::Run: Command Stop"));

            iAuthor->Stop((OsclAny*)iAuthor);
        }
        break;

        case PVAE_CMD_RESET:
        {
            ResetAuthorConfig();
            if (iAuthor->GetPVAuthorState() != PVAE_STATE_IDLE)
            {
                iAuthor->Reset((OsclAny*)iAuthor);
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                (0, "pv_mediainput_async_test_opencomposestop::PVAE_CMD_RESET - State PVAE_STATE_IDLE"));
                iState = PVAE_CMD_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
        }
        break;

        case PVAE_CMD_REMOVE_DATA_SOURCE:
        {
            for (uint32 ii = 0; ii < iMIOComponent.iMIONode.size(); ii++)
            {
                iAuthor->RemoveDataSource(*(iMIOComponent.iMIONode[ii]), (OsclAny*)iAuthor);
                AddEngineCommand();
            }
        }
        break;

        case PVAE_CMD_CLOSE:
        {
            iAuthor->Close((OsclAny*)iAuthor);
        }
        break;

        case PVAE_CMD_PAUSE:
        {
            iAuthor->Pause((OsclAny*)iAuthor);
        }
        break;

        case PVAE_CMD_RESUME:
        {
            iAuthor->Resume((OsclAny*)iAuthor);
        }
        break;

        case PVAE_CMD_CLEANUPANDCOMPLETE:
        {
            Cleanup();
            iObserver->CompleteTest(*iTestCase);
        }
        break;

        case PVAE_CMD_QUERY_INTERFACE2:
        case PVAE_CMD_ADD_DATA_SINK:
        case PVAE_CMD_REMOVE_DATA_SINK:
            break;

        case PVAE_CMD_RECORDING:
        {
            if (PVAE_CMD_RECORDING == iCurrentResetState)
            {
                iNextResetState = PVAE_CMD_STOP;
                iState = PVAE_CMD_CLEANUPANDCOMPLETE;
                PVPATB_TEST_IS_TRUE(true);
                RunIfNotReady();
            }
            else
            {
                iState = PVAE_CMD_STOP;
                RunIfNotReady(iTestDuration);
            }

        }
        break;

        default:
            break;
    } //end switch
}

////////////////////////////////////////////////////////////////////////////
void pv_mediainput_async_test_delete::CommandCompleted(const PVCmdResponse& aResponse)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "pv_mediainput_async_test_delete::CommandCompleted iState:%d", iState));

    if (aResponse.GetCmdStatus() != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "pv_mediainput_async_test_delete::CommandCompleted iState:%d FAILED", iState));
    }

    switch (iState)
    {
        case PVAE_CMD_OPEN:
        {
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if (PVAE_CMD_OPEN == iCurrentResetState)
                {
                    iNextResetState = PVAE_CMD_ADD_DATA_SOURCE;
                    iState = PVAE_CMD_CLEANUPANDCOMPLETE;
                    PVPATB_TEST_IS_TRUE(true);
                }
                else
                {
                    iState = PVAE_CMD_ADD_DATA_SOURCE;
                }

                RunIfNotReady();
            }
            else
            {
                // Open failed
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_RESET;
                RunIfNotReady();
            }
        }
        break;

        case PVAE_CMD_ADD_DATA_SOURCE:
        {
            if (EngineCmdComplete())
            {
                if (PVAE_CMD_ADD_DATA_SOURCE == iCurrentResetState)
                {
                    iNextResetState = PVAE_CMD_SELECT_COMPOSER;
                    iState = PVAE_CMD_CLEANUPANDCOMPLETE;
                    PVPATB_TEST_IS_TRUE(true);
                }
                else
                {
                    iState = PVAE_CMD_SELECT_COMPOSER;
                }
            }

            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                RunIfNotReady();
            }
            else
            {
                // AddDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_RESET;
                RunIfNotReady();
            }
        }
        break;

        case PVAE_CMD_SELECT_COMPOSER:
        {
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iComposer = aResponse.GetResponseData();
                if (!ConfigComposer())
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                    (0, "pv_mediainput_async_test_delete::CommandCompleted: Error - ConfigComposer failed"));

                    PVPATB_TEST_IS_TRUE(false);
                    iState = PVAE_CMD_RESET;
                    RunIfNotReady();
                    return;
                }
                else
                {
                    if (PVAE_CMD_SELECT_COMPOSER == iCurrentResetState)
                    {
                        iNextResetState = PVAE_CMD_ADD_MEDIA_TRACK;
                        iState = PVAE_CMD_CLEANUPANDCOMPLETE;
                        PVPATB_TEST_IS_TRUE(true);
                    }
                    else
                    {
                        iState = PVAE_CMD_ADD_MEDIA_TRACK;
                    }

                    RunIfNotReady();
                }
            }
            else
            {
                // SelectComposer failed
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_RESET;
                RunIfNotReady();
            }
        }
        break;

        case PVAE_CMD_ADD_MEDIA_TRACK:
        {
            if (EngineCmdComplete())
            {
                if (iAddVideoMediaTrack)
                {
                    ConfigureVideoEncoder();
                }
                if (iAddAudioMediaTrack)
                {
                    ConfigureAudioEncoder();
                }
                if (PVAE_CMD_ADD_MEDIA_TRACK == iCurrentResetState)
                {
                    iNextResetState = PVAE_CMD_INIT;
                    iState = PVAE_CMD_CLEANUPANDCOMPLETE;
                    PVPATB_TEST_IS_TRUE(true);
                }
                else
                {
                    iState = PVAE_CMD_INIT;
                }

            }

            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                RunIfNotReady();
            }
            else
            {
                // AddMediaTrack failed
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_RESET;
                RunIfNotReady();
            }
        }
        break;

        case PVAE_CMD_INIT:
        {
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if (PVAE_CMD_INIT == iCurrentResetState)
                {
                    iNextResetState = PVAE_CMD_START;
                    iState = PVAE_CMD_CLEANUPANDCOMPLETE;
                    PVPATB_TEST_IS_TRUE(true);
                }
                else
                {
                    iState = PVAE_CMD_START;
                }

                RunIfNotReady();
            }
            else
            {
                // Init failed
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_RESET;
                RunIfNotReady();
            }
        }
        break;

        case PVAE_CMD_START:
        {
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if (iPauseResumeEnable)
                {
                    if (PVAE_CMD_START == iCurrentResetState)
                    {
                        iNextResetState = PVAE_CMD_PAUSE;
                        iState = PVAE_CMD_CLEANUPANDCOMPLETE;
                        PVPATB_TEST_IS_TRUE(true);
                    }
                    else
                    {
                        iState = PVAE_CMD_PAUSE;
                    }

                    RunIfNotReady(KPauseDuration); //Pause after 5 sec
                }
                else
                {
                    if (PVAE_CMD_START == iCurrentResetState)
                    {
                        iNextResetState = PVAE_CMD_RECORDING;
                        iState = PVAE_CMD_CLEANUPANDCOMPLETE;
                        PVPATB_TEST_IS_TRUE(true);
                    }
                    else
                    {
                        iState = PVAE_CMD_RECORDING;
                    }

                    RunIfNotReady();
                }
            }
            else
            {
                // Start failed
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_RESET;
                RunIfNotReady();
            }
        }
        break;

        case PVAE_CMD_PAUSE:
        {
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if (PVAE_CMD_PAUSE == iCurrentResetState)
                {
                    iNextResetState = PVAE_CMD_STOP;
                    iState = PVAE_CMD_CLEANUPANDCOMPLETE;
                    PVPATB_TEST_IS_TRUE(true);
                    RunIfNotReady();
                }
                else
                {
                    iState = PVAE_CMD_RESUME;
                    /* Stay paused for 10 seconds */
                    RunIfNotReady(10*1000*1000);
                }

            }
            else
            {
                //Pause failed
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_RESET;
                RunIfNotReady();
            }
        }
        break;

        case PVAE_CMD_RESUME:
        {
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = PVAE_CMD_STOP;
                iNextResetState = PVAE_CMD_STOP;
                //Run for another 10 sec before stopping
                RunIfNotReady(10*1000*1000);
            }
            else
            {
                //Resume failed
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_RESET;
                RunIfNotReady();
            }
        }
        break;

        case PVAE_CMD_STOP:
        {
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iOutputFileName = NULL;
                iState = PVAE_CMD_RESET;
                RunIfNotReady();
            }
            else
            {
                // Stop failed
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_RESET;
                RunIfNotReady();
            }
        }
        break;

        case PVAE_CMD_RESET:
        {
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if ((iMIOComponent.iMediaInput.size() == 0) || (iMIOComponent.iMIONode.size() == 0))
                {
                    if (aResponse.GetCmdStatus() == PVMFSuccess)
                    {
                        PVPATB_TEST_IS_TRUE(true);
                    }
                    else
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }
                    //Since there are no MIO Components/Nodes, we end here
                    //No need to call RemoveDataSource. Just call cleanup.
                    iState = PVAE_CMD_CLEANUPANDCOMPLETE;
                    RunIfNotReady();
                    break;
                }

                iState = PVAE_CMD_REMOVE_DATA_SOURCE;
                RunIfNotReady();
            }
            else
            {
                if (PVAE_CMD_CREATE == iCurrentResetState)
                {
                    //Reset in create state will return error.
                    PVPATB_TEST_IS_TRUE(true);
                    iState = PVAE_CMD_CLEANUPANDCOMPLETE;
                    RunIfNotReady();
                }
                else
                {
                    // Reset failed
                    PVPATB_TEST_IS_TRUE(false);
                    OSCL_ASSERT("ERROR -- Response failure for CMD_RESET");
                    iObserver->CompleteTest(*iTestCase);
                }
            }
        }
        break;

        case PVAE_CMD_REMOVE_DATA_SOURCE:
        {
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if (EngineCmdComplete())
                {
                    iOutputFileName = NULL;
                    iMIOComponent.DeleteInputNode();
                    if (iMediaInputType == PVMF_MIME_AVIFF)
                    {
                        PVAviFile* fileparser = OSCL_STATIC_CAST(PVAviFile*, iFileParser);
                        PVAviFile::DeleteAviFileParser(fileparser);
                        fileparser = NULL;
                        iFileParser = NULL;
                    }
                    if (iMediaInputType == PVMF_MIME_WAVFF)
                    {
                        PV_Wav_Parser* fileparser = OSCL_STATIC_CAST(PV_Wav_Parser*, iFileParser);
                        delete(fileparser);
                        fileparser = NULL;
                        iFileParser = NULL;
                    }

                    iFileParser = NULL;
                    iState = PVAE_CMD_CLOSE;
                    RunIfNotReady();

                }
                else
                {
                    return; //wait for completion of all RemoveDataSource calls.
                }
            }
            else
            {
                // RemoveDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iObserver->CompleteTest(*iTestCase);
            }
        }
        break;

        case PVAE_CMD_CLOSE:
        {
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                PVPATB_TEST_IS_TRUE(true);
            }
            else
            {
                PVPATB_TEST_IS_TRUE(false);
            }
            iObserver->CompleteTest(*iTestCase);
        }
        break;

        default:
        {
            // Testing error if this is reached
            PVPATB_TEST_IS_TRUE(false);
            iObserver->CompleteTest(*iTestCase);
        }
        break;
    }  //end switch
}

