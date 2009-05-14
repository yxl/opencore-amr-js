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
#ifndef TEST_PV_AUTHOR_ENGINE_TESTSET6_H_INCLUDED
#include "test_pv_author_engine_testset6.h"
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

#include "pvmf_media_data.h"
#include "pvmf_media_msg_format_ids.h"
void pv_mediainput_async_test_errorhandling::StartTest()
{
    AddToScheduler();
    iState = PVAE_CMD_CREATE;
    RunIfNotReady();
}

////////////////////////////////////////////////////////////////////////////
void pv_mediainput_async_test_errorhandling::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    OSCL_UNUSED_ARG(aEvent);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR, (0, "pv_mediainput_async_test_errorhandling::HandleErrorEvent"));
    iState = PVAE_CMD_RESET;
    RunIfNotReady();
}

////////////////////////////////////////////////////////////////////////////
void pv_mediainput_async_test_errorhandling::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "pv_mediainput_async_test_errorhandling::HandleInformationalEvent"));

    switch (aEvent.GetEventType())
    {
        case PVMF_COMPOSER_EOS_REACHED:
            //Engine already stopped at EOS so send reset command.
            iState = PVAE_CMD_RESET;
            Cancel();
            RunIfNotReady();
            PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "pv_mediainput_async_test_errorhandling::HandleInformationalEvent: EOS reached"));

            break;

        default:
            break;
    }

}

////////////////////////////////////////////////////////////////////////////
bool pv_mediainput_async_test_errorhandling::CreateTestInputs()
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
            if (PVMediaInput_ErrorHandling_Test_WrongIPFileName == iTestErrorType)
            {
                PVPATB_TEST_IS_TRUE(true);
            }
            else
            {
                PVPATB_TEST_IS_TRUE(false);

            }
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
    else if (iMediaInputType == PVMF_MIME_WAVFF)
    {
        OSCL_TRY(error, iFileParser = OSCL_NEW(PV_Wav_Parser, ()););
        if (error || (NULL == iFileParser))
        {
            return false;
        }
        if (((PV_Wav_Parser*)iFileParser)->InitWavParser(iInputFileName, &iFileServer) != PVWAVPARSER_OK)
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

            if (PVMediaInput_ErrorHandling_Test_WrongFormat == iTestErrorType)
            {
                PVPATB_TEST_IS_TRUE(true);
            }

            goto ERROR_CODE;
        }

        if (PVMediaInput_ErrorHandling_Test_WrongFormat == iTestErrorType)
        {

            PVPATB_TEST_IS_TRUE(false);
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
        else if (iMediaInputType == PVMF_MIME_WAVFF)
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
bool pv_mediainput_async_test_errorhandling::AddDataSource()
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
bool pv_mediainput_async_test_errorhandling::ConfigComposer()
{
    if (!ConfigOutputFile())
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "pv_mediainput_async_test_errorhandling::ConfigComposer: Error - ConfigOutputFile failed"));

        return false;
    }

    if ((oscl_strstr(iComposerMimeType.get_str(), "mp4")) || (oscl_strstr(iComposerMimeType.get_str(), "3gp")))
    {
        if (!ConfigMp43gpComposer())
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "pv_mediainput_async_test_errorhandling::ConfigComposer: Error - ConfigMp43gpComposer failed"));

            return false;
        }
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////
bool pv_mediainput_async_test_errorhandling::ConfigOutputFile()
{

    PvmfFileOutputNodeConfigInterface* clipConfig = OSCL_STATIC_CAST(PvmfFileOutputNodeConfigInterface*, iComposerConfig);
    if (!clipConfig)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "pv_mediainput_async_test_errorhandling::ConfigOutputFile: Error - Invalid iComposerConfig"));

        return false;
    }

    if (clipConfig->SetOutputFileName(iOutputFileName) != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "pv_mediainput_async_test_errorhandling::ConfigOutputFile: Error - SetOutputFileName failed"));

        return false;
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////
bool pv_mediainput_async_test_errorhandling::ConfigMp43gpComposer()
{

    PVMp4FFCNClipConfigInterface* clipConfig;
    clipConfig = OSCL_STATIC_CAST(PVMp4FFCNClipConfigInterface*, iComposerConfig);
    if (!clipConfig)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "pv_mediainput_async_test_errorhandling::ConfigMp43gpComposer: Error - iComposerConfig==NULL"));

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
bool pv_mediainput_async_test_errorhandling::AddMediaTrack()
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
        else if (iMediaInputType == PVMF_MIME_WAVFF)
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
        else if (iMediaInputType == PVMF_MIME_WAVFF)
        {
            return false;
        }

    }

    return true;
}

////////////////////////////////////////////////////////////////////////////
bool pv_mediainput_async_test_errorhandling::ConfigureVideoEncoder()
{

    PVMp4H263EncExtensionInterface* config;
    config = OSCL_STATIC_CAST(PVMp4H263EncExtensionInterface*, iVideoEncoderConfig);
    if (!config)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "pv_mediainput_async_test_errorhandling::ConfigureVideoEncoder: No configuration needed"));

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

bool pv_mediainput_async_test_errorhandling::ConfigureAudioEncoder()
{

    PVAudioEncExtensionInterface* config;
    config = OSCL_STATIC_CAST(PVAudioEncExtensionInterface*, iAudioEncoderConfig);
    if (!config)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "pv_mediainput_async_test_errorhandling::Encoder: No configuration needed"));

        return true;
    }

    if (!PVAETestNodeConfig::ConfigureAudioEncoder(iAudioEncoderConfig, iAudioEncoderMimeType))
    {
        return false;
    }
    return true;
}
////////////////////////////////////////////////////////////////////////////
void pv_mediainput_async_test_errorhandling::ResetAuthorConfig()
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
void pv_mediainput_async_test_errorhandling::Cleanup()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "pv_mediainput_async_test_errorhandling::Cleanup"));

    iComposer = NULL;

    ResetAuthorConfig();

    if (iAuthor)
    {
        PVAuthorEngineFactory::DeleteAuthor(iAuthor);
        iAuthor = NULL;
    }

//	iMIOComponent.DeleteInputNode();
    iOutputFileName = NULL;
    iFileServer.Close();
}


////////////////////////////////////////////////////////////////////////////
void pv_mediainput_async_test_errorhandling::Run()
{
    if (IsEngineCmdPending())
    {
        return;
    }
    iCheckState = iState;
    switch (iState)
    {
        case PVAE_CMD_CREATE:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "******pv_mediainput_async_test_errorhandling::iTestCaseNum:%d******", iTestCaseNum));

            iAuthor = PVAuthorEngineFactory::CreateAuthor(this, this, this);
            if (!iAuthor)
            {
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_CLEANUPANDCOMPLETE;
                RunIfNotReady();

            }
            else
            {
                iState = PVAE_CMD_OPEN;
                RunIfNotReady();
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
                                (0, "pv_mediainput_async_test_errorhandling::CreateTestInputs: Error - failed"));

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

        case PVAE_CMD_QUERY_INTERFACE1:
            iAuthor->QueryInterface(PVMI_CAPABILITY_AND_CONFIG_PVUUID,
                                    (PVInterface*&)iAuthorCapConfigIF,
                                    (OsclAny*)iAuthor);
            break;

        case PVAE_CMD_CAPCONFIG_SYNC1:
            CapConfigSync1();
            iState = PVAE_CMD_START;
            RunIfNotReady();
            break;

        case PVAE_CMD_QUERY_INTERFACE_COMP:
            iAuthor->QueryInterface(PvmfComposerSizeAndDurationUuid,
                                    iOutputSizeAndDurationConfig, (OsclAny*)iAuthor);
            break;
        case PVAE_CMD_START:
        {
            iAuthor->Start();
        }
        break;

        case PVAE_CMD_STOP:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                            (0, "pv_mediainput_async_test_errorhandling::Run: Command Stop"));

            fprintf(iFile, "Error: Recording timeout, stop engine please wait *******\n");
            iAuthor->Stop((OsclAny*)iAuthor);
            PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                            (0, "pv_mediainput_async_test_errorhandling::Run: Error: Recording timeout, stop engine please wait *******\n"));
        }
        break;

        case PVAE_CMD_RESET:
        {
            ResetAuthorConfig();
            if (iAuthor->GetPVAuthorState() != PVAE_STATE_IDLE)
            {
                iAuthor->Reset((OsclAny*)iAuthor);
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


        case PVAE_CMD_ADD_DATA_SINK:
        case PVAE_CMD_REMOVE_DATA_SINK:
            break;

        case PVAE_CMD_RECORDING:
        {
            iState = PVAE_CMD_RESET;
            RunIfNotReady(iTestDuration);
        }
        break;

        default:
            break;
    } //end switch
}

////////////////////////////////////////////////////////////////////////////
void pv_mediainput_async_test_errorhandling::CommandCompleted(const PVCmdResponse& aResponse)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "pv_mediainput_async_test_errorhandling::CommandCompleted iState:%d", iState));

    if (aResponse.GetCmdStatus() != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "pv_mediainput_async_test_errorhandling::CommandCompleted iState:%d FAILED", iState));
    }

    /* This check is done in order to verify that the response from the engine
       is for the current command or for any other previously issued asynchronous command
    */
    if (iCheckState != iState) // Current command and the response state doesn't match
    {
//		fprintf(iFile, "ERROR: The current state and the response's state doesn't match *******\n");
        return;
    }
    switch (iState)
    {
        case PVAE_CMD_OPEN:
        {
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = PVAE_CMD_ADD_DATA_SOURCE;
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
                iState = PVAE_CMD_SELECT_COMPOSER;
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
                                    (0, "pv_mediainput_async_test_errorhandling::CommandCompleted: Error - ConfigComposer failed"));

                    PVPATB_TEST_IS_TRUE(true);
                    iState = PVAE_CMD_RESET;
                    RunIfNotReady();
                }
                else
                {
                    iState = PVAE_CMD_ADD_MEDIA_TRACK;
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
                iState = PVAE_CMD_INIT;
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
                iState = PVAE_CMD_QUERY_INTERFACE_COMP;
                RunIfNotReady();
            }
            else
            {
                // Init failed
                PVPATB_TEST_IS_TRUE(true);
                iState = PVAE_CMD_RESET;
                RunIfNotReady();
            }
        }
        break;
        case PVAE_CMD_QUERY_INTERFACE_COMP:
        {
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                ConfigMp4Composer();
                iState = PVAE_CMD_QUERY_INTERFACE1;
                RunIfNotReady();
            }
            else
            {
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_RESET;
                RunIfNotReady();
            }
        }
        break;
        case PVAE_CMD_QUERY_INTERFACE1:
        {
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = PVAE_CMD_CAPCONFIG_SYNC1;
                RunIfNotReady();
            }
            else
            {
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
                    iState = PVAE_CMD_PAUSE;
                    RunIfNotReady();
                }
                else
                {
                    iState = PVAE_CMD_RECORDING;
                    RunIfNotReady();
                }
            }
            else
            {
                // Start failed
                if (ErrorHandling_MediaInputNodeStopFailed != iTestErrorType)
                {
                    PVPATB_TEST_IS_TRUE(true);
                }
                else
                {
                    PVPATB_TEST_IS_TRUE(false);
                }

                iState = PVAE_CMD_RESET;
                RunIfNotReady();
            }
        }
        break;

        case PVAE_CMD_PAUSE:
        {
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = PVAE_CMD_RESUME;
                /* Stay paused for 10 seconds */
                RunIfNotReady(10*1000*1000);
            }
            else
            {
                // Pause failed
                if ((ErrorHandling_MediaInputNode_StateFailure_EPause_SendMIORequest == iTestErrorType) ||
                        (ErrorHandling_MediaInputNode_Node_Cmd_Pause == iTestErrorType)	||
                        (ErrorHandling_MP4Composer_Node_Cmd_Pause == iTestErrorType) ||
                        (ErrorHandling_VideoEncodeNode_Node_Cmd_Pause == iTestErrorType) ||
                        (ErrorHandling_AudioEncodeNode_Node_Cmd_Pause == iTestErrorType) ||
                        (ErrorHandling_AVCEncodeNode_Node_Cmd_Pause == iTestErrorType))
                {
                    PVPATB_TEST_IS_TRUE(true);
                }
                else
                {
                    //Pause failed
                    PVPATB_TEST_IS_TRUE(false);
                }
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

                //Run for another 10 sec before stopping
                RunIfNotReady(10*1000*1000);
            }
            else
            {
                //Resume failed
                Cancel();
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
                if (ErrorHandling_MediaInputNodeStopFailed == iTestErrorType)

                {
                    PVPATB_TEST_IS_TRUE(true);
                }
                else
                {
                    PVPATB_TEST_IS_TRUE(false);
                }

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
                    //No need to call RemoveDataSource
                    iObserver->CompleteTest(*iTestCase);
                    break;
                }

                iState = PVAE_CMD_REMOVE_DATA_SOURCE;
                RunIfNotReady();

            }
            else
            {
                if ((ErrorHandling_VideoEncodeNode_Node_Cmd_Stop == iTestErrorType) ||
                        (ErrorHandling_AudioEncodeNode_Node_Cmd_Stop == iTestErrorType) ||
                        (ErrorHandling_AVCEncodeNode_Node_Cmd_Stop == iTestErrorType) ||
                        (ErrorHandling_MediaInputNode_Node_Cmd_Stop == iTestErrorType) ||
                        (ErrorHandling_MediaInputNodeStopFailed == iTestErrorType))
                {
                    PVPATB_TEST_IS_TRUE(true);
                }
                else
                {
                    // Reset failed
                    PVPATB_TEST_IS_TRUE(false);
                    OSCL_ASSERT("ERROR -- Response failure for CMD_RESET");
                    iState = PVAE_CMD_REMOVE_DATA_SOURCE;
                    RunIfNotReady();
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
                    RemoveMIOComp();
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
                RemoveMIOComp();
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

void pv_mediainput_async_test_errorhandling::RemoveMIOComp()
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
    else if (iMediaInputType == PVMF_MIME_WAVFF)
    {
        PV_Wav_Parser* fileparser = OSCL_STATIC_CAST(PV_Wav_Parser*, iFileParser);
        delete(fileparser);
        fileparser = NULL;
        iFileParser = NULL;
    }

    iFileParser = NULL;
}

bool pv_mediainput_async_test_errorhandling::CapConfigSync1()
{
    // set the mime strings here
    // capabilty and configuration feature implemented here
    // set the config parameters using MIME strings here

    OSCL_StackString<64> paramkey;
    // KVP array
    PvmiKvp paramkvp;
    paramkvp.value.bool_value = true;
    key_type* key = NULL;
    key = (key_type*)oscl_malloc(sizeof(key_type));
    if (ErrorHandling_VideoInitFailed == iTestErrorType) //To fail VideoEncNode Initialize
    {
        paramkey = _STRLIT_CHAR("x-pvmf/encoder/video/error_start_init;valtype=bool");
    }
    else if (ErrorHandling_ComposerAddFragFailed == iTestErrorType)  //To fail Mp4ComposerNode on AddMemFragToTrack
    {
        paramkey = _STRLIT_CHAR("x-pvmf/composer/mp4/error_start_addmemfrag;valtype=bool");
    }
    else if (ErrorHandling_ComposerAddTrackFailed == iTestErrorType)  //To fail Mp4ComposerNode on AddTrack
    {
        paramkey = _STRLIT_CHAR("x-pvmf/composer/mp4/error_start_addtrack;valtype=bool");
    }
    else if (ErrorHandling_MediaInputNodeStartFailed == iTestErrorType)  //To fail MediaInputNode on Start
    {
        paramkey = _STRLIT_CHAR("x-pvmf/datasource/error_adddatasource_start;valtype=bool");
    }
    else if (ErrorHandling_MediaInputNodeStopFailed == iTestErrorType)  //To fail MediaInputNode on Stop
    {
        paramkey = _STRLIT_CHAR("x-pvmf/datasource/error_adddatasource_stop;valtype=bool");
    }
    else if (ErrorHandling_AudioInitFailed == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/encoder/audio/error_start_init;valtype=bool");
    }
    else if ((ErrorHandling_AVCVideoEncodeFailed == iTestErrorType))			          //To fail AVCEncNode on Encode
    {
        paramkey = _STRLIT_CHAR("x-pvmf/encoder/video/error-encode;mode=frames;valtype=uint32");
        paramkvp.value.uint32_value = 1;

    }
    else if (ErrorHandling_VideoEncodeFailed == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/encoder/video/error-encode;mode=frames;valtype=uint32");
        paramkvp.value.uint32_value = 1;

    }
    else if (ErrorHandling_AudioEncodeFailed == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/encoder/audio/error-encode;mode=frames;valtype=uint32");
        paramkvp.value.uint32_value = 1;

    }
    else if ((ErrorHandling_AVCVideoEncode5FramesFailed == iTestErrorType))		//To fail AVCEncNode on Encode 5 frames

    {
        paramkey = _STRLIT_CHAR("x-pvmf/encoder/video/error-encode;mode=frames;valtype=uint32");
        paramkvp.value.uint32_value = 5;
    }
    else if (ErrorHandling_VideoEncode5FramesFailed == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/encoder/video/error-encode;mode=frames;valtype=uint32");
        paramkvp.value.uint32_value = 5;

    }
    else if (ErrorHandling_MediaInputNode_NoMemBuffer == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/datasource/error_no_memorybuffer_avaliable;valtype=bool");
    }
    else if (ErrorHandling_MediaInputNode_Out_Queue_busy == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/datasource/error_out_queue_busy;valtype=bool");
    }
    else if (ErrorHandling_MediaInputNode_large_time_stamp == iTestErrorType)
    {

        paramkey = _STRLIT_CHAR("x-pvmf/datasource/error-time-stamp;valtype=ksv");

        uint32 numStreams = ((PVAviFile*)iFileParser)->GetNumStreams();

        key->mode = 1;
        key->duration = 0;
        key->track_no = numStreams - 1;

        paramkvp.value.key_specific_value = OSCL_DYNAMIC_CAST(void*, key);
    }
    else if (ErrorHandling_MediaInputNode_wrong_time_stamp_after_duration == iTestErrorType)
    {

        paramkey = _STRLIT_CHAR("x-pvmf/datasource/error-time-stamp;valtype=ksv");

        uint32 numStreams = ((PVAviFile*)iFileParser)->GetNumStreams();

        key->mode = 2;
        key->duration = 60;
        key->track_no = numStreams - 1;

        paramkvp.value.key_specific_value = OSCL_DYNAMIC_CAST(void*, key);
    }

    else if (ErrorHandling_MediaInputNode_zero_time_stamp == iTestErrorType)
    {

        paramkey = _STRLIT_CHAR("x-pvmf/datasource/error-time-stamp;valtype=ksv");

        uint32 numStreams = ((PVAviFile*)iFileParser)->GetNumStreams();

        key->mode = 3;
        key->duration = 0;
        key->track_no = numStreams - 1;

        paramkvp.value.key_specific_value = OSCL_DYNAMIC_CAST(void*, key);
    }

    else if (ErrorHandling_MediaInputNode_StateFailure_EPause_SendMIORequest == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/datasource/error-sendmiorequest;valtype=uint32");
        paramkvp.value.uint32_value = 2;
        iPauseResumeEnable = true;
    }
    else if (ErrorHandling_MediaInputNode_StateFailure_CancelMIORequest == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/datasource/error-cancelmiorequest;valtype=bool");
    }
    else if (ErrorHandling_MediaInputNode_Corrupt_Video_InputData == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/datasource/error-corruptinputdata;index=0;valtype=uint32");
        paramkvp.value.uint32_value = 100;
    }
    else if (ErrorHandling_MediaInputNode_Corrupt_Audio_InputData == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/datasource/error-corruptinputdata;index=1;valtype=uint32");
        paramkvp.value.uint32_value = 100;
    }
    else if (ErrorHandling_MediaInputNode_Node_Cmd_Start  == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/datasource/error-node-cmd;index=2;valtype=uint32");
        paramkvp.value.uint32_value = 2;
    }
    else if (ErrorHandling_MediaInputNode_Node_Cmd_Stop  == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/datasource/error-node-cmd;index=2;valtype=uint32");
        paramkvp.value.uint32_value = 3;
    }
    else if (ErrorHandling_MediaInputNode_Node_Cmd_Flush  == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/datasource/error-node-cmd;index=2;valtype=uint32");
        paramkvp.value.uint32_value = 4;
    }
    else if (ErrorHandling_MediaInputNode_Node_Cmd_Pause  == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/datasource/error-node-cmd;index=2;valtype=uint32");
        paramkvp.value.uint32_value = 5;
        iPauseResumeEnable = true;
    }
    else if (ErrorHandling_MediaInputNode_Node_Cmd_ReleasePort  == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/datasource/error-node-cmd;index=2;valtype=uint32");
        paramkvp.value.uint32_value = 7;
    }
    else if (ErrorHandling_MediaInputNode_DataPath_Stall == iTestErrorType)
    {
        uint32 numStreams = ((PVAviFile*)iFileParser)->GetNumStreams();
        paramkey = _STRLIT_CHAR("x-pvmf/datasource/error-data-path-stall;valtype=uint32");
        if (numStreams > 0)
        {
            paramkvp.value.uint32_value = numStreams - 1;
        }
        else
        {
            paramkvp.value.uint32_value = numStreams;
        }

    }
    else if (ErrorHandling_MP4Composer_AddTrack_PVMF_AMR_IETF == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/composer/mp4/error-addtrack;valtype=char*");
        OSCL_HeapString<OsclMemAllocator> mode1 = "PVMF_AMR_IETF";
        paramkvp.value.pChar_value = mode1.get_str();
    }
    else if (ErrorHandling_MP4Composer_AddTrack_PVMF_3GPP_TIMEDTEXT == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/composer/mp4/error-addtrack;valtype=char*");
        OSCL_HeapString<OsclMemAllocator> mode2 = "PVMF_3GPP_TIMEDTEXT";
        paramkvp.value.pChar_value = mode2.get_str();
    }

    else if (ErrorHandling_MP4Composer_AddTrack_PVMF_M4V == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/composer/mp4/error-addtrack;valtype=char*");
        OSCL_HeapString<OsclMemAllocator> mode3 = "PVMF_M4V";
        paramkvp.value.pChar_value = mode3.get_str();
    }

    else if (ErrorHandling_MP4Composer_AddTrack_PVMF_H263 == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/composer/mp4/error-addtrack;valtype=char*");
        OSCL_HeapString<OsclMemAllocator> mode4 = "PVMF_H263";
        paramkvp.value.pChar_value = mode4.get_str();
    }
    else if (ErrorHandling_MP4Composer_AddTrack_PVMF_H264_MP4 == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/composer/mp4/error-addtrack;valtype=char*");
        OSCL_HeapString<OsclMemAllocator> mode5 = "PVMF_H264_MP4";
        paramkvp.value.pChar_value = mode5.get_str();
    }
    else if (ErrorHandling_MP4Composer_Node_Cmd_Start == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/composer/mp4/error-node-cmd;index=2;valtype=uint32");
        paramkvp.value.uint32_value = 2;
    }
    else if (ErrorHandling_MP4Composer_Node_Cmd_Stop == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/composer/mp4/error-node-cmd;index=2;valtype=uint32");
        paramkvp.value.uint32_value = 3;
    }
    else if (ErrorHandling_MP4Composer_Node_Cmd_Flush == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/composer/mp4/error-node-cmd;index=2;valtype=uint32");
        paramkvp.value.uint32_value = 4;
    }
    else if (ErrorHandling_MP4Composer_Node_Cmd_Pause == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/composer/mp4/error-node-cmd;index=2;valtype=uint32");
        paramkvp.value.uint32_value = 5;
        iPauseResumeEnable = true;
    }
    else if (ErrorHandling_MP4Composer_Node_Cmd_ReleasePort == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/composer/mp4/error-node-cmd;index=2;valtype=uint32");
        paramkvp.value.uint32_value = 7;
    }

    else if (ErrorHandling_MP4Composer_Create_FileParser == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/composer/mp4/error-create-composer;valtype=bool");
    }
    else if (ErrorHandling_MP4Composer_RenderToFile == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/composer/mp4/error-render-to-file;valtype=bool");
    }
    else if (ErrorHandling_MP4Composer_FailAfter_FileSize == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/composer/mp4/error-addsample;mode=filesize;valtype=uint32"); //shorten the key to fit in buffer
        paramkvp.value.uint32_value = 5000;
    }
    else if (ErrorHandling_MP4Composer_FailAfter_Duration == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/composer/mp4/error-addsample;mode=duration;valtype=uint32");
        paramkvp.value.uint32_value = 500;
    }
    else if (ErrorHandling_MP4Composer_DataPathStall == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/composer/mp4/data-path-stall;valtype=uint32");
        paramkvp.value.uint32_value = 1;
    }
    else if (ErrorHandling_VideoEncodeNode_Node_Cmd_Start == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/encoder/video/error-node-cmd;index=2;valtype=uint32");
        paramkvp.value.uint32_value = 2;
    }
    else if (ErrorHandling_VideoEncodeNode_Node_Cmd_Stop == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/encoder/video/error-node-cmd;index=2;valtype=uint32");
        paramkvp.value.uint32_value = 3;
    }
    else if (ErrorHandling_VideoEncodeNode_Node_Cmd_Flush == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/encoder/video/error-node-cmd;index=2;valtype=uint32");
        paramkvp.value.uint32_value = 4;
    }
    else if (ErrorHandling_VideoEncodeNode_Node_Cmd_Pause == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/encoder/video/error-node-cmd;index=2;valtype=uint32");
        paramkvp.value.uint32_value = 5;
        iPauseResumeEnable = true;
    }
    else if (ErrorHandling_VideoEncodeNode_Node_Cmd_ReleasePort == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/encoder/video/error-node-cmd;index=2;valtype=uint32;");
        paramkvp.value.uint32_value = 7;
    }
    else if (ErrorHandling_VideoEncodeNode_ConfigHeader == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/encoder/video/error-config-header;valtype=bool");
    }
    else if (ErrorHandling_VideoEncodeNode_DataPathStall_Before_ProcessingData == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/encoder/video/data-path-stall;valtype=uint32");
        paramkvp.value.uint32_value = 1;
    }
    else if (ErrorHandling_VideoEncodeNode_DataPathStall_After_ProcessingData == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/encoder/video/data-path-stall;valtype=uint32");
        paramkvp.value.uint32_value = 2;
    }
    else if (ErrorHandling_VideoEncodeNode_FailEncode_AfterDuration == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/encoder/video/error-encode;mode=duration;valtype=uint32");
        paramkvp.value.uint32_value = 300;
    }
    else if (ErrorHandling_AudioEncodeNode_Node_Cmd_Start == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/encoder/audio/error-node-cmd;index=2;valtype=uint32");
        paramkvp.value.uint32_value = 2;

    }
    else if (ErrorHandling_AudioEncodeNode_Node_Cmd_Stop == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/encoder/audio/error-node-cmd;index=2;valtype=uint32");
        paramkvp.value.uint32_value = 3;

    }
    else if (ErrorHandling_AudioEncodeNode_Node_Cmd_Flush == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/encoder/audio/error-node-cmd;index=2;valtype=uint32");
        paramkvp.value.uint32_value = 4;

    }
    else if (ErrorHandling_AudioEncodeNode_Node_Cmd_Pause == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/encoder/audio/error-node-cmd;index=2;valtype=uint32");
        paramkvp.value.uint32_value = 5;
        iPauseResumeEnable = true;

    }
    else if (ErrorHandling_AudioEncodeNode_Node_Cmd_ReleasePort == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/encoder/audio/error-node-cmd;index=2;valtype=uint32");
        paramkvp.value.uint32_value = 7;

    }

    else if (ErrorHandling_AudioEncodeNode_FailEncode_AfterDuration == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/encoder/audio/error-encode;mode=duration;valtype=uint32");
        paramkvp.value.uint32_value = 700;
    }
    else if (ErrorHandling_AudioEncodeNode_DataPathStall_Before_ProcessingData == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/encoder/audio/data-path-stall;valtype=uint32");
        paramkvp.value.uint32_value = 1;
    }

    else if (ErrorHandling_AudioEncodeNode_DataPathStall_After_ProcessingData == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/encoder/audio/data-path-stall;valtype=uint32");
        paramkvp.value.uint32_value = 2;
    }
    else if (ErrorHandling_AVCEncodeNode_Node_Cmd_Start == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/encoder/video/error-node-cmd;index=2;valtype=uint32");
        paramkvp.value.uint32_value = 2;

    }
    else if (ErrorHandling_AVCEncodeNode_Node_Cmd_Stop == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/encoder/video/error-node-cmd;index=2;valtype=uint32");
        paramkvp.value.uint32_value = 3;

    }
    else if (ErrorHandling_AVCEncodeNode_Node_Cmd_Flush == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/encoder/video/error-node-cmd;index=2;valtype=uint32");
        paramkvp.value.uint32_value = 4;

    }
    else if (ErrorHandling_AVCEncodeNode_Node_Cmd_Pause == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/encoder/video/error-node-cmd;index=2;valtype=uint32");
        paramkvp.value.uint32_value = 5;
        iPauseResumeEnable = true;

    }
    else if (ErrorHandling_AVCEncodeNode_Node_Cmd_ReleasePort == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/encoder/video/error-node-cmd;index=2;valtype=uint32");
        paramkvp.value.uint32_value = 7;

    }
    else if (ErrorHandling_AVCEncodeNode_ConfigHeader == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/encoder/video/error-config-header;valtype=bool");
    }
    else if (ErrorHandling_AVCEncodeNode_DataPathStall_Before_ProcessingData == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/encoder/video/data-path-stall;valtype=uint32");
        paramkvp.value.uint32_value = 1;
    }
    else if (ErrorHandling_AVCEncodeNode_DataPathStall_After_ProcessingData == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/encoder/video/data-path-stall;valtype=uint32");
        paramkvp.value.uint32_value = 2;
    }
    else if (ErrorHandling_AVCEncodeNode_FailEncode_AfterDuration == iTestErrorType)
    {
        paramkey = _STRLIT_CHAR("x-pvmf/encoder/video/error-encode;mode=duration;valtype=uint32");
        paramkvp.value.uint32_value = 100;
    }
    paramkvp.key = paramkey.get_str();

    // set the value in node using SetParameterSync f(n) here
    iAuthorCapConfigIF->setParametersSync(NULL, &paramkvp, 1, iErrorKVP);
    oscl_free(key);
    return true;
}

void pv_mediainput_async_test_errorhandling::ConfigMp4Composer()
{
    PvmfComposerSizeAndDurationInterface* config =
        OSCL_REINTERPRET_CAST(PvmfComposerSizeAndDurationInterface*, iOutputSizeAndDurationConfig);

    config->SetFileSizeProgressReport(true, KFileSizeProgressFreq);

}
