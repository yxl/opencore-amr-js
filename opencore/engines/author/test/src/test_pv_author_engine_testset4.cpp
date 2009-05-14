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
#ifndef TEST_PV_AUTHOR_ENGINE_TESTSET4_H_INCLUDED
#include "test_pv_author_engine_testset4.h"
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

void pvauthor_async_compressed_test_errorhandling::StartTest()
{
    AddToScheduler();
    iState = PVAE_CMD_CREATE;
    RunIfNotReady();
}



////////////////////////////////////////////////////////////////////////////
void pvauthor_async_compressed_test_errorhandling::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    OSCL_UNUSED_ARG(aEvent);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR, (0, "pvauthor_async_compressed_test_errorhandling::HandleErrorEvent"));
    iState = PVAE_CMD_RESET;
    RunIfNotReady();
}

////////////////////////////////////////////////////////////////////////////
void pvauthor_async_compressed_test_errorhandling::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    OSCL_UNUSED_ARG(aEvent);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "pvauthor_async_compressed_test_errorhandling::HandleInformationalEvent"));

}

////////////////////////////////////////////////////////////////////////////
int pvauthor_async_compressed_test_errorhandling::CreateAudioInput()
{
    int status = 0;
    PVAETestInput testInput;

    switch (iTestCaseNum)
    {
        case ErrorHandling_WrongTextInputFileNameTest:
        case ErrorHandling_MediaInputNodeStartFailed:

            PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_DEBUG,
                            (0, "pvauthor_async_compressed_test_errorhandling::CreateAudioTestInput: AMR input"));
            if (testInput.IsTestInputTypeSupported(AMR_IETF_FILE))
            {
                iAudioInputType = AMR_IETF_FILE;
                status = testInput.CreateInputNode(AMR_IETF_FILE, iInputFileNameAudio, iAVTConfig);
            }
            break;

        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_DEBUG,
                            (0, "pvauthor_async_compressed_test_errorhandling::CreateAudioTestInput: Audio input node not needed for this test case"));
            return -1;
    }

    if (!status)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                        (0, "pvauthor_async_compressed_test_errorhandling::CreateAudioTestInput: Error - Create input node failed"));
        return status;
    }

    return AddDataSource(testInput);
}

////////////////////////////////////////////////////////////////////////////
int pvauthor_async_compressed_test_errorhandling::CreateVideoInput()
{
    int status = 0;
    PVAETestInput testInput;

    switch (iTestCaseNum)
    {
        case ErrorHandling_WrongTextInputFileNameTest:
        case ErrorHandling_MediaInputNodeStartFailed:

            PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_DEBUG,
                            (0, "pvauthor_async_compressed_test_errorhandling::CreateVideoTestInput: YUV input"));
            if (testInput.IsTestInputTypeSupported(YUV_FILE))
            {
                iVideoInputType = YUV_FILE;
                testInput = PVAETestInput();
                status = testInput.CreateInputNode(YUV_FILE, iInputFileNameVideo, iAVTConfig);
            }
            break;
        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_DEBUG,
                            (0, "pvauthor_async_compressed_test_errorhandling::CreateVideoTestInput: Video input node not needed for this test case"));
            return -1;
    }

    if (!status)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                        (0, "pvauthor_async_compressed_test_errorhandling::CreateTestInputs: Error - CreateInputNode failed"));
        return status;
    }

    return AddDataSource(testInput);
}

////////////////////////////////////////////////////////////////////////////
int pvauthor_async_compressed_test_errorhandling::CreateTextInput()
{
    int status = 0;
    PVAETestInput testInput;

    switch (iTestCaseNum)
    {
        case ErrorHandling_WrongTextInputFileNameTest:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_DEBUG,
                            (0, "pvauthor_async_compressed_test_errorhandling::CreateTextTestInput: TEXT input"));
            if (testInput.IsTestInputTypeSupported(TEXT_FILE))
            {
                iTextInputType = TEXT_FILE;
                testInput = PVAETestInput();
                iInputFileNameText = KTEXTTestInputWrong;
                status = testInput.CreateInputNode(TEXT_FILE, iInputFileNameText, iAVTConfig);
            }
            break;

        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_DEBUG,
                            (0, "pvauthor_async_compressed_test_errorhandling::CreateTextTestInput:  input node not needed for this test case"));
            return -1;
    }

    if (!status)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                        (0, "pvauthor_async_compressed_test_errorhandling::CreateTestInputs: Error - CreateInputNode failed"));
        return status;
    }

    return AddDataSource(testInput);
}

////////////////////////////////////////////////////////////////////////////
bool pvauthor_async_compressed_test_errorhandling::AddDataSource(PVAETestInput& aInput)
{
    int32 err = 0;

    OSCL_TRY(err, iTestInputs.push_back(aInput););
    if (err != OSCL_ERR_NONE)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                        (0, "pvauthor_async_compressed_test_errorhandling::AddDataSource: Error - iTestInputs.push_back failed. err=0x%x", err));
        aInput.DeleteInputNode();
        return false;
    }

    OSCL_TRY(err, iAuthor->AddDataSource(*(aInput.iNode), (OsclAny*)iAuthor););
    if (err != OSCL_ERR_NONE)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                        (0, "pvauthor_async_compressed_test_errorhandling::AddDataSource: Error - iAuthor->AddDataSource failed. err=0x%x", err));
        aInput.DeleteInputNode();
        return false;
    }

    return true;
}

void pvauthor_async_compressed_test_errorhandling::SelectComposer()
{
    switch (iTestCaseNum)
    {
        case ErrorHandling_WrongTextInputFileNameTest:
        case ErrorHandling_MediaInputNodeStartFailed:

            iComposerMimeType = K3gpComposerMimeType;
            break;

        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                            (0, "pvauthor_async_compressed_test_errorhandling::SelectComposer: Error - No composer type for test case"));
            PVPATB_TEST_IS_TRUE(false);
            iObserver->CompleteTest(*iTestCase);
            return;
    }

    iAuthor->SelectComposer(iComposerMimeType, iComposerConfig, (OsclAny*)iAuthor);
}

////////////////////////////////////////////////////////////////////////////
bool pvauthor_async_compressed_test_errorhandling::ConfigComposer()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "pvauthor_async_compressed_test_errorhandling::ConfigComposer"));
    if (!ConfigAmrAacComposer())
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "pvauthor_async_compressed_test_errorhandling::ConfigComposer: Error - ConfigAmrAacComposer failed"));
        return false;
    }

    //Not needed we pick up the output filename from cmd line

    if (!ConfigMp43gpComposer())
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "pvauthor_async_compressed_test_errorhandling::ConfigComposer: Error - ConfigMp43gpComposer failed"));
        return false;
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////
bool pvauthor_async_compressed_test_errorhandling::ConfigAmrAacComposer()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "pvauthor_async_compressed_test_errorhandling::ConfigAmrAacComposer"));


    switch (iTestCaseNum)
    {
        case AMR_FOutput_Test:
        case AMR_FileOutput_Test_UsingExternalFileHandle:
            if (iOutputFileName == NULL)
            {
                iOutputFileName = KFOAOnlyAMRTestOutput;
            }
            break;

        case AMRWB_FOutput_Test:
            if (iOutputFileName == NULL)
            {
                iOutputFileName = KFOAOnlyAMRWBTestOutput;
            }
            break;

        case AACADIF_FOutput_Test:
            if (iOutputFileName == NULL)
            {
                iOutputFileName = KFOAOnlyAACADIFTestOutput;
            }
            break;


        case AACADTS_FOutput_Test:
            if (iOutputFileName == NULL)
            {
                iOutputFileName = KFOAOnlyAACADTSTestOutput;
            }
            break;

        default:
            if (!((iComposerMimeType == KAMRNbComposerMimeType) ||
                    (iComposerMimeType == KAMRWBComposerMimeType) ||
                    (iComposerMimeType == KAACADTSComposerMimeType) ||
                    (iComposerMimeType == KAACADIFComposerMimeType)))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "pvauthor_async_compressed_test_errorhandling::ConfigAmrAacComposer: AMR-AAC Composer not used in this test case"));
                return true;
            }
    }

    PvmfFileOutputNodeConfigInterface* clipConfig = OSCL_STATIC_CAST(PvmfFileOutputNodeConfigInterface*, iComposerConfig);
    if (!clipConfig)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "pvauthor_async_compressed_test_errorhandling::ConfigAmrAacComposer: Error - Invalid iComposerConfig"));
        return false;
    }

    if (clipConfig->SetOutputFileName(iOutputFileName) != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "pvauthor_async_compressed_test_errorhandling::ConfigAmrAacComposer: Error - SetOutputFileName failed"));
        return false;
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////
bool pvauthor_async_compressed_test_errorhandling::ConfigMp43gpComposer()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                    (0, "pvauthor_async_compressed_test_errorhandling::ConfigMp43gpComposer"));

    switch (iTestCaseNum)
    {
        case ErrorHandling_WrongTextInputFileNameTest:
        case ErrorHandling_MediaInputNodeStartFailed:
            if (iOutputFileName == NULL)
            {
                iOutputFileName = KAMRYUVInputAV3gpTestOutput;
            }
            break;

        default:
            if (!(iComposerMimeType == KAMRNbComposerMimeType) &&
                    !(iComposerMimeType == KAMRWBComposerMimeType) &&
                    !(iComposerMimeType == KAACADTSComposerMimeType) &&
                    !(iComposerMimeType == KAACADIFComposerMimeType))
            {
                break;
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "pvauthor_async_compressed_test_errorhandling::ConfigMp43gpComposer: Mp4-3GPP composer not used in this test case"));
                return true;
            }
    }

    PVMp4FFCNClipConfigInterface* clipConfig;
    clipConfig = OSCL_STATIC_CAST(PVMp4FFCNClipConfigInterface*, iComposerConfig);
    if (!clipConfig)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "pvauthor_async_compressed_test_errorhandling::ConfigMp43gpComposer: Error - iComposerConfig==NULL"));
        return false;
    }

    iVersionString = _STRLIT("version");
    iTitleString = _STRLIT("title");
    iAuthorString = _STRLIT("author");
    iCopyrightString = _STRLIT("copyright");
    iDescriptionString = _STRLIT("description");
    iRatingString = _STRLIT("rating");
    iAlbumTitle   = _STRLIT("albumtitle");
    iRecordingYear = 2008;

    OSCL_HeapString<OsclMemAllocator> lang_code = "eng";

    clipConfig->SetOutputFileName(iOutputFileName);
    clipConfig->SetPresentationTimescale(1000);
    clipConfig->SetVersion(iVersionString, lang_code);
    clipConfig->SetTitle(iTitleString, lang_code);
    clipConfig->SetAuthor(iAuthorString, lang_code);
    clipConfig->SetCopyright(iCopyrightString, lang_code);
    clipConfig->SetDescription(iDescriptionString, lang_code);
    clipConfig->SetRating(iRatingString, lang_code);
    clipConfig->SetAlbumInfo(iAlbumTitle, lang_code);
    clipConfig->SetRecordingYear(iRecordingYear);

    if (iTestCaseNum == K3GPPDownloadModeTest)
    {
        clipConfig->SetAuthoringMode(PVMP4FFCN_3GPP_DOWNLOAD_MODE);
    }
    else if (iTestCaseNum == K3GPPProgressiveDownloadModeTest)
    {
        clipConfig->SetAuthoringMode(PVMP4FFCN_3GPP_PROGRESSIVE_DOWNLOAD_MODE);
    }


    return true;
}

////////////////////////////////////////////////////////////////////////////
bool pvauthor_async_compressed_test_errorhandling::ConfigComposerOutput()
{
    PvmfComposerSizeAndDurationInterface* config =
        OSCL_REINTERPRET_CAST(PvmfComposerSizeAndDurationInterface*, iOutputSizeAndDurationConfig);
    if (!config)
    {
        return false;
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////
bool pvauthor_async_compressed_test_errorhandling::QueryComposerOutputInterface()
{
    return false;
}

////////////////////////////////////////////////////////////////////////////
bool pvauthor_async_compressed_test_errorhandling::AddAudioMediaTrack()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "pvauthor_async_compressed_test_errorhandling::AddAudioMediaTrack"));

    switch (iTestCaseNum)
    {
        case ErrorHandling_WrongTextInputFileNameTest:
        case ErrorHandling_MediaInputNodeStartFailed:
            iAudioEncoderMimeType = KAMRNbEncMimeType;
            break;

        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "pvauthor_async_compressed_test_errorhandling::AddAudioMediaTrackL: Error - No audio for this current test case"));
            return false;
    }

    if (iAudioInputType == INVALID_INPUT_TYPE)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "pvauthor_async_compressed_test_errorhandling::AddAudioMediaTrackL: Error - Invalid audio input type"));
        return false;
    }

    bool testInputFound = false;
    PVAETestInput testInput;
    for (uint32 ii = 0; ii < iTestInputs.size(); ii++)
    {
        if (iTestInputs[ii].iType == iAudioInputType)
        {
            testInputFound = true;
            testInput = iTestInputs[ii];
            break;
        }
    }

    if (!testInputFound)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "pvauthor_async_compressed_test_errorhandling::AddAudioMediaTrack: Error - Test input not found"));
        return false;
    }

    iAuthor->AddMediaTrack(*(testInput.iNode), iAudioEncoderMimeType, iComposer,
                           iAudioEncoderConfig, (OsclAny*)iAuthor);
    return true;
}

////////////////////////////////////////////////////////////////////////////
bool pvauthor_async_compressed_test_errorhandling::AddVideoMediaTrack()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "pvauthor_async_compressed_test_errorhandling::AddVideoMediaTrack"));

    // Add video media track
    switch (iTestCaseNum)
    {
        case ErrorHandling_WrongTextInputFileNameTest:
        case ErrorHandling_MediaInputNodeStartFailed:
            iVideoEncoderMimeType = KH263EncMimeType;
            break;

        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "pvauthor_async_compressed_test_errorhandling::AddVideoMediaTrack: Video encoder not used in this test case."));
            return false;
    }

    if (iVideoInputType == INVALID_INPUT_TYPE)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "pvauthor_async_compressed_test_errorhandling::AddVideoMediaTrack: Error - Invalid input type"));
        return false;
    }

    bool testInputFound = false;
    PVAETestInput testInput;

    for (uint32 ii = 0; ii < iTestInputs.size(); ii++)
    {
        if (iTestInputs[ii].iType == iVideoInputType)
        {
            testInputFound = true;
            testInput = iTestInputs[ii];
            break;
        }
    }

    if (!testInputFound)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "pvauthor_async_compressed_test_errorhandling::AddVideoMediaTrack: Error - Test input not found"));
        return false;
    }

    iAuthor->AddMediaTrack(*(testInput.iNode), iVideoEncoderMimeType, iComposer,
                           iVideoEncoderConfig, (OsclAny*)iAuthor);
    return true;
}

////////////////////////////////////////////////////////////////////////////
bool pvauthor_async_compressed_test_errorhandling::AddTextMediaTrack()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "pvauthor_async_compressed_test_errorhandling::AddTextMediaTrack"));

    switch (iTestCaseNum)
    {
        case ErrorHandling_WrongTextInputFileNameTest:
            iTextEncoderMimeType = KTextEncMimeType;
            break;

        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "pvauthor_async_compressed_test_errorhandling::AddTextMediaTrackL: Error - No text for this current test case"));
            return false;
    }

    if (iTextInputType == INVALID_INPUT_TYPE)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "pvauthor_async_compressed_test_errorhandling::AddTextMediaTrackL: Error - Invalid text input type"));
        return false;
    }

    bool testInputFound = false;
    PVAETestInput testInput;
    for (uint32 ii = 0; ii < iTestInputs.size(); ii++)
    {
        if (iTestInputs[ii].iType == iTextInputType)
        {
            testInputFound = true;
            testInput = iTestInputs[ii];
            break;
        }
    }

    if (!testInputFound)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "pvauthor_async_compressed_test_errorhandling::AddTextMediaTrack: Error - Test input not found"));
        return false;
    }

    iAuthor->AddMediaTrack(*(testInput.iNode), iTextEncoderMimeType, iComposer,
                           iTextEncoderConfig, (OsclAny*)iAuthor);
    return true;
}

////////////////////////////////////////////////////////////////////////////
bool pvauthor_async_compressed_test_errorhandling::ConfigureVideoEncoder()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "pvauthor_async_compressed_test_errorhandling::ConfigureVideoEncoder"));

    PVMp4H263EncExtensionInterface* config;
    config = OSCL_STATIC_CAST(PVMp4H263EncExtensionInterface*, iVideoEncoderConfig);
    if (!config)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "pvauthor_async_compressed_test_errorhandling::ConfigureVideoEncoder: No configuration needed"));
        return true;
    }

    config->SetNumLayers(1);
    config->SetOutputBitRate(0, KVideoBitrate);
    config->SetOutputFrameSize(0, iAVTConfig.iWidth , iAVTConfig.iHeight);
    config->SetOutputFrameRate(0, iAVTConfig.iFps);
    config->SetIFrameInterval(KVideoIFrameInterval);
    config->SetSceneDetection(true);
    return true;
}

////////////////////////////////////////////////////////////////////////////
bool pvauthor_async_compressed_test_errorhandling::ConfigureAudioEncoder()
{
    // Single core AMR encoder node support
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "pvauthor_async_compressed_test_errorhandling::ConfigureAudioEncoder"));

    if (!iAudioEncoderConfig)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "pvauthor_async_compressed_test_errorhandling::ConfigureAudioEncoder: No configuration needed"));
        return true;
    }

    return PVAETestNodeConfig::ConfigureAudioEncoder(iAudioEncoderConfig, iAudioEncoderMimeType);
}

////////////////////////////////////////////////////////////////////////////
bool pvauthor_async_compressed_test_errorhandling::ConfigureTextEncoder()
{
    // Single core AMR encoder node support
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "pvauthor_async_compressed_test_errorhandling::ConfigureTextEncoder"));

    if (!iTextEncoderConfig)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "pvauthor_async_compressed_test_errorhandling::ConfigureTextEncoder: No configuration needed"));
        return true;
    }

    return true;
}
bool pvauthor_async_compressed_test_errorhandling::DeleteTestInputs()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "pvauthor_async_compressed_test_errorhandling::DeleteTestInputs"));

    for (uint32 ii = 0; ii < iTestInputs.size(); ii++)
        iTestInputs[ii].DeleteInputNode();

    iTestInputs.clear();
    return true;
}

void pvauthor_async_compressed_test_errorhandling::ResetAuthorConfig()
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
    if (iTextEncoderConfig)
    {
        iTextEncoderConfig->removeRef();
        iTextEncoderConfig = NULL;
    }
}

void pvauthor_async_compressed_test_errorhandling::Cleanup()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "pvauthor_async_compressed_test_errorhandling::Cleanup"));

    iComposer = NULL;

    ResetAuthorConfig();

    if (iAuthor)
    {
        PVAuthorEngineFactory::DeleteAuthor(iAuthor);
        iAuthor = NULL;
    }

    DeleteTestInputs();
    iOutputFileName = NULL;
}


////////////////////////////////////////////////////////////////////////////
void pvauthor_async_compressed_test_errorhandling::Run()
{
    switch (iState)
    {
        case PVAE_CMD_CREATE:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "******pvauthor_async_compressed_test_errorhandling::iTestCaseNum:%d******", iTestCaseNum));
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
            iAuthor->Open((OsclAny*)iAuthor);
            break;

        case PVAE_CMD_ADD_DATA_SOURCE_AUDIO:
        {
            int aStatus = -1;
            // Create audio input
            aStatus = CreateAudioInput();
            if (aStatus == 0) //Failed while creating audio input
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                (0, "pvauthor_async_compressed_test_errorhandling::CreateTestInputs: Error - CreateAudioInput() failed"));
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else if (aStatus == -1) //Failed due to test being audio only or video only
            {
                iState = PVAE_CMD_ADD_DATA_SOURCE_VIDEO;
                RunIfNotReady();
            }
        }
        break;

        case PVAE_CMD_ADD_DATA_SOURCE_VIDEO:
        {
            // Create video input
            int aStatus = -1;
            aStatus = CreateVideoInput();
            if (aStatus == 0)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                (0, "pvauthor_async_compressed_test_errorhandling::CreateTestInputs: Error - CreateVideoInput() failed"));
                Cleanup();
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else if (aStatus == -1) //Failed due to test being audio only or video only
            {
                iState = PVAE_CMD_ADD_DATA_SOURCE_TEXT;
                RunIfNotReady();
            }
        }
        break;

        case PVAE_CMD_ADD_DATA_SOURCE_TEXT:
        {
            // Create text input
            int aStatus = -1;
            aStatus = CreateTextInput();
            if (aStatus == 0)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                (0, "pvauthor_async_compressed_test_errorhandling::CreateTestInputs: Error - CreateVideoInput() failed"));
                Cleanup();
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else if (aStatus == -1) //Failed due to test being audio only or video only
            {
                iState = PVAE_CMD_SELECT_COMPOSER;
                RunIfNotReady();
            }
        }
        break;

        case PVAE_CMD_SELECT_COMPOSER:
            SelectComposer();
            break;

        case PVAE_CMD_QUERY_INTERFACE:
            if (!QueryComposerOutputInterface())
            {
                iState = PVAE_CMD_ADD_AUDIO_MEDIA_TRACK;
                RunIfNotReady();
            }
            break;

        case PVAE_CMD_ADD_AUDIO_MEDIA_TRACK:
            if (!AddAudioMediaTrack())
            {
                bAudioTrack = false;
                iState = PVAE_CMD_ADD_VIDEO_MEDIA_TRACK;
                RunIfNotReady();
            }
            break;

        case PVAE_CMD_ADD_VIDEO_MEDIA_TRACK:
        {
            bVideoTrack = AddVideoMediaTrack();
            if (!bVideoTrack && !bAudioTrack) //No tracks have been added
            {
                bVideoTrack = false;
                iState = PVAE_CMD_ADD_TEXT_MEDIA_TRACK;
                RunIfNotReady();
            }
            else if (!bVideoTrack) //Audio track added but no video track
            {
                iState = PVAE_CMD_ADD_TEXT_MEDIA_TRACK;
                RunIfNotReady();
            }
        }
        break;

        case PVAE_CMD_ADD_TEXT_MEDIA_TRACK:
        {
            bool bTextTrack = AddTextMediaTrack();
            if (!bTextTrack && !bAudioTrack && !bVideoTrack) //No tracks have been added
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                (0, "pvauthor_async_compressed_test_errorhandling::Run: Error - No track added"));
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else if (!bTextTrack) //Audio and video track added but no text track
            {
                iState = PVAE_CMD_INIT;
                RunIfNotReady();
            }
        }
        break;

        case PVAE_CMD_INIT:
            iAuthor->Init((OsclAny*)iAuthor);
            break;

        case PVAE_CMD_START:
            iAuthor->Start();
            break;
        case PVAE_CMD_STOP:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                            (0, "pvauthor_async_compressed_test_errorhandling::Run: Fire Stop"));
            iAuthor->Stop((OsclAny*)iAuthor);
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
            for (uint ii = 0; ii < iTestInputs.size(); ii++)
            {
                iAuthor->RemoveDataSource(*(iTestInputs[ii].iNode), (OsclAny*)iAuthor);
            }
        }
        break;
        case PVAE_CMD_CLOSE:
            iAuthor->Close((OsclAny*)iAuthor);
            break;
        case PVAE_CMD_PAUSE:
            iAuthor->Pause((OsclAny*)iAuthor);
            break;
        case PVAE_CMD_RESUME:
            iAuthor->Resume((OsclAny*)iAuthor);
            break;
        case PVAE_CMD_CLEANUPANDCOMPLETE:
        {
            Cleanup();
            iObserver->CompleteTest(*iTestCase);
        }
        break;

        case PVAE_CMD_ADD_DATA_SINK:
            break;
        case PVAE_CMD_REMOVE_DATA_SINK:
            break;
        case PVAE_CMD_RECORDING:
            if (!iAuthoringCounter)
            {
                iState = PVAE_CMD_STOP;
                // this is for the finshing the loop count
                if (iAuthoringTime > KAuthoringSessionUnit)
                {
                    iTestDuration = (iAuthoringTime % KAuthoringSessionUnit) * 1000 * 1000;
                }
                // otherwise iTestDuration is already set through Constructor
                RunIfNotReady(iTestDuration);
            }
            else
            {
                // carry with the loops
                iState = PVAE_CMD_RECORDING;
                iAuthoringCounter--;
                RunIfNotReady(iTestDuration);
            }
            break;
        default:
            break;
    }
}

////////////////////////////////////////////////////////////////////////////
void pvauthor_async_compressed_test_errorhandling::CommandCompleted(const PVCmdResponse& aResponse)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "pvauthor_async_compressed_test_errorhandling::CommandCompleted iState:%d", iState));

    if (aResponse.GetCmdStatus() != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "pvauthor_async_compressed_test_errorhandling::CommandCompleted iState:%d FAILED", iState));
    }
    switch (iState)
    {
        case PVAE_CMD_OPEN:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = PVAE_CMD_ADD_DATA_SOURCE_AUDIO;
                RunIfNotReady();
            }
            else
            {
                // Open failed
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_RESET;
                RunIfNotReady();
            }
            break;
        case PVAE_CMD_ADD_DATA_SOURCE_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = PVAE_CMD_ADD_DATA_SOURCE_VIDEO;
                RunIfNotReady();
            }
            else
            {
                // AddDataSourceAudio failed
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_RESET;
                RunIfNotReady();
            }
            break;
        case PVAE_CMD_ADD_DATA_SOURCE_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = PVAE_CMD_ADD_DATA_SOURCE_TEXT;
                RunIfNotReady();
            }
            else
            {
                // AddDataSourceVideo failed
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_RESET;
                RunIfNotReady();
            }
            break;
        case PVAE_CMD_ADD_DATA_SOURCE_TEXT:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = PVAE_CMD_SELECT_COMPOSER;
                RunIfNotReady();
            }
            else
            {
                // AddDataSourceText failed
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_RESET;
                RunIfNotReady();
            }
            break;
        case PVAE_CMD_SELECT_COMPOSER:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iComposer = aResponse.GetResponseData();
                if (!ConfigComposer())
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                    (0, "pvauthor_async_compressed_test_errorhandling::CommandCompleted: Error - ConfigComposer failed"));
                    PVPATB_TEST_IS_TRUE(false);
                    iState = PVAE_CMD_RESET;
                    RunIfNotReady();
                    return;
                }
                else
                {
                    iState = PVAE_CMD_QUERY_INTERFACE;
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
            break;
        case PVAE_CMD_QUERY_INTERFACE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                ConfigComposerOutput();
                iState = PVAE_CMD_ADD_AUDIO_MEDIA_TRACK;
                RunIfNotReady();
            }
            else
            {
                // QueryInterface failed
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_RESET;
                RunIfNotReady();
            }
            break;
        case PVAE_CMD_ADD_AUDIO_MEDIA_TRACK:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if (!ConfigureAudioEncoder())
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                    (0, "pvauthor_async_compressed_test_errorhandling::CommandCompleted: Error - ConfigureAudioEncoder failed"));
                    PVPATB_TEST_IS_TRUE(false);
                    iState = PVAE_CMD_CLOSE;
                    RunIfNotReady();
                }
                else
                {
                    iState = PVAE_CMD_ADD_VIDEO_MEDIA_TRACK;
                    RunIfNotReady();
                }
            }
            else
            {
                // AddAudioMediaTrack failed
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_RESET;
                RunIfNotReady();
            }
            break;

        case PVAE_CMD_ADD_VIDEO_MEDIA_TRACK:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if (!ConfigureVideoEncoder())
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                    (0, "pvauthor_async_compressed_test_errorhandling::CommandCompleted: Error - ConfigureVideoEncoder failed"));
                    PVPATB_TEST_IS_TRUE(false);
                    iObserver->CompleteTest(*iTestCase);
                }
                else
                {
                    iState = PVAE_CMD_ADD_TEXT_MEDIA_TRACK;
                    RunIfNotReady();
                }
            }
            else
            {
                // AddVideoMediaTrack failed
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_RESET;
                RunIfNotReady();
            }
            break;

        case PVAE_CMD_ADD_TEXT_MEDIA_TRACK:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if (!ConfigureTextEncoder())
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                    (0, "pvauthor_async_compressed_test_errorhandling::CommandCompleted: Error - ConfigureTextEncoder failed"));
                    PVPATB_TEST_IS_TRUE(false);
                    iObserver->CompleteTest(*iTestCase);
                }
                else
                {
                    iState = PVAE_CMD_INIT;
                    RunIfNotReady();
                }
            }
            else
            {
                // AddTextMediaTrack failed
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_RESET;
                RunIfNotReady();
            }
            break;

        case PVAE_CMD_INIT:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = PVAE_CMD_START;
                RunIfNotReady();
            }
            else
            {
                // Init failed
                //Test does not hang, so return true
                PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                (0, "pvauthor_async_compressed_test_errorhandling::INIT FAILED"));
                PVPATB_TEST_IS_TRUE(true);
                iState = PVAE_CMD_RESET;
                RunIfNotReady();
            }
            break;

        case PVAE_CMD_START:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if (iPauseResumeEnable)
                {
                    iState = PVAE_CMD_PAUSE;
                    RunIfNotReady(KPauseDuration); //Pause after 5 sec
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
                //Test does not hang, so return true
                PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                (0, "pvauthor_async_compressed_test_errorhandling::START FAILED"));
                PVPATB_TEST_IS_TRUE(true);
                iState = PVAE_CMD_RESET;
                RunIfNotReady();
            }
            break;
        case PVAE_CMD_PAUSE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = PVAE_CMD_RESUME;
                /* Stay paused for 10 seconds */
                RunIfNotReady(10*1000*1000);
            }
            else
            {
                //Pause failed
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_RESET;
                RunIfNotReady();
            }
            break;
        case PVAE_CMD_RESUME:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = PVAE_CMD_STOP;
                RunIfNotReady(10*1000*1000); //Run for another 10 sec before stopping
            }
            else
            {
                //Resume failed
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_RESET;
                RunIfNotReady();
            }
            break;
        case PVAE_CMD_STOP:
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
            break;
        case PVAE_CMD_RESET:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if (iTestInputs.size() == 0)
                {
                    if (aResponse.GetCmdStatus() == PVMFSuccess)
                    {
                        PVPATB_TEST_IS_TRUE(true);
                    }
                    else
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }
                    //Since there are no testInputs, we end here
                    //No need to call RemoveDataSource
                    iObserver->CompleteTest(*iTestCase);
                    break;
                }
                iState = PVAE_CMD_REMOVE_DATA_SOURCE;
                RunIfNotReady();
            }
            else
            {
                // Reset failed
                PVPATB_TEST_IS_TRUE(false);
                OSCL_ASSERT("ERROR -- Response failure for CMD_RESET");
                iObserver->CompleteTest(*iTestCase);
            }
            break;

        case PVAE_CMD_REMOVE_DATA_SOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iRemoveDataSourceDone++;
                if (iRemoveDataSourceDone < iTestInputs.size())
                {
                    return;//We will wait for all RemoveDataSource calls to complete
                }
                iOutputFileName = NULL;
                if (!DeleteTestInputs())
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                    (0, "pvauthor_async_compressed_test_errorhandling::CommandCompleted: Error - DeleteTestInputs failed"));
                    PVPATB_TEST_IS_TRUE(false);
                    iObserver->CompleteTest(*iTestCase);
                    return;
                }
                else
                {
                    iState = PVAE_CMD_CLOSE;
                    RunIfNotReady();
                }
            }
            else
            {
                // RemoveDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iObserver->CompleteTest(*iTestCase);
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
    }
}


