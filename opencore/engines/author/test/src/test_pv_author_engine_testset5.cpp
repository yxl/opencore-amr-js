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
#ifndef TEST_PV_AUTHOR_ENGINE_TESTSET5_H_INCLUDED
#include "test_pv_author_engine_testset5.h"
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

#ifndef PVMI_KVP_H_INCLUDED
#include "pvmi_kvp.h"
#endif
void pv_mediainput_async_test_opencomposestop::StartTest()
{
    AddToScheduler();
    iState = PVAE_CMD_CREATE;
    RunIfNotReady();
}


/*@todo add error handling here */
////////////////////////////////////////////////////////////////////////////
void pv_mediainput_async_test_opencomposestop::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    OSCL_UNUSED_ARG(aEvent);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR, (0, "pv_mediainput_async_test_opencomposestop::HandleErrorEvent"));
    iState = PVAE_CMD_RESET;
    Cancel();//added to cancel previous running AO
    RunIfNotReady();
}

////////////////////////////////////////////////////////////////////////////
void pv_mediainput_async_test_opencomposestop::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "pv_mediainput_async_test_opencomposestop::HandleInformationalEvent"));

    OsclAny* eventData = NULL;
    switch (aEvent.GetEventType())
    {
        case PVMF_COMPOSER_MAXFILESIZE_REACHED:
        case PVMF_COMPOSER_MAXDURATION_REACHED:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "pv_mediainput_async_test_opencomposestop::HandleNodeInformationalEvent: Max file size reached"));
            iState = PVAE_CMD_RESET;

            Cancel();
            PVPATB_TEST_IS_TRUE(true);
            RunIfNotReady();

            fprintf(iFile, "Recording finished, Closing file please wait *******\n");
            break;

        case PVMF_COMPOSER_DURATION_PROGRESS:
            aEvent.GetEventData(eventData);
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "pv_mediainput_async_test_opencomposestop::HandleNodeInformationalEvent: Duration progress: %d ms",
                             (int32)eventData));
            fprintf(iStdOut, "Duration: %d ms\n", (int32)eventData);
            break;

        case PVMF_COMPOSER_FILESIZE_PROGRESS:
            aEvent.GetEventData(eventData);
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "pv_mediainput_async_test_opencomposestop::HandleNodeInformationalEvent: File size progress: %d bytes",
                             (int32)eventData));
            fprintf(iStdOut, "File size: %d bytes\n", (int32)eventData);
            break;

        case PVMF_COMPOSER_EOS_REACHED:
            //Engine already stopped at EOS so send reset command.
            iState = PVAE_CMD_RESET;
            //cancel recording timeout scheduled for timer object.
            Cancel();
            RunIfNotReady();

            fprintf(iFile, "Recording finished, Closing file please wait *******\n");

            break;

        default:
            break;
    }
}

////////////////////////////////////////////////////////////////////////////
bool pv_mediainput_async_test_opencomposestop::CreateTestInputs()
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

        //set recording duration to TEST_TIMEOUT_FACTOR times the file duration in order to time out
        //if EOS is not received by this time.
        iTestDuration = (TEST_TIMEOUT_FACTOR * ((PVAviFile*)iFileParser)->GetFileDuration());

        if (iTestDuration > KAuthoringSessionUnit)
        {
            //if test duration, in microsec, is too large, the uint32
            //variable will rollover. Hence the test run is divided
            //in small sessions of KAuthoringSessionUnit duration
            iAuthoringCount = iTestDuration / KAuthoringSessionUnit;
            iTestDuration = KAuthoringSessionUnit * 1000 * 1000; //in microsec
        }
        else
        {
            iTestDuration = iTestDuration * 1000 * 1000;
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

        PVWAVFileInfo fileInfo;

        ((PV_Wav_Parser*)iFileParser)->RetrieveFileInfo(fileInfo);

        //set recording duration to TEST_TIMEOUT_FACTOR times the file duration in order to time out
        //if EOS is not received by this time.
        iTestDuration = (TEST_TIMEOUT_FACTOR * ((fileInfo.NumSamples * fileInfo.BytesPerSample) / fileInfo.SampleRate)) ; // in sec

        if (iTestDuration > KAuthoringSessionUnit)
        {
            //if test duration, in microsec, is too large, the uint32
            //variable will rollover. Hence the test run is divided
            //in small sessions of KAuthoringSessionUnit duration
            iAuthoringCount = iTestDuration / KAuthoringSessionUnit;
            iTestDuration = KAuthoringSessionUnit * 1000 * 1000; //in microsec
        }
        else
        {
            iTestDuration = iTestDuration * 1000 * 1000;
        }

    }

    {

        PVMIOControlComp MIOComp(iMediaInputType, (OsclAny*)iFileParser, iLoopTime);

        if (iLoopTime)
        {

            iTestDuration = TEST_TIMEOUT_FACTOR * iLoopTime ; //timeout duration in microsec
            if (iTestDuration > KAuthoringSessionUnit)
            {
                //if test duration, in microsec, is too large, the uint32
                //variable will rollover. Hence the test run is divided
                //in small sessions of KAuthoringSessionUnit duration
                iAuthoringCount = iTestDuration / KAuthoringSessionUnit;
                iTestDuration = KAuthoringSessionUnit * 1000 * 1000; //in microsec
            }
            else
            {
                iTestDuration = iTestDuration * 1000 * 1000;
            }

        }

        status = MIOComp.CreateMIOInputNode(iRealTimeAuthoring, iMediaInputType, iInputFileName);
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
bool pv_mediainput_async_test_opencomposestop::AddDataSource()
{
    int32 err = 0;
    uint32 noOfNodes = iMIOComponent.iMIONode.size();

    OSCL_TRY(err,
             for (uint32 ii = 0; ii < noOfNodes; ii++)
{
    AddEngineCommand();

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
bool pv_mediainput_async_test_opencomposestop::ConfigComposer()
{
    if (!ConfigOutputFile())
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "pv_mediainput_async_test_opencomposestop::ConfigComposer: Error - ConfigOutputFile failed"));

        return false;
    }

    if ((oscl_strstr(iComposerMimeType.get_str(), "mp4")) || (oscl_strstr(iComposerMimeType.get_str(), "3gp")))
    {
        if (!ConfigMp43gpComposer())
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "pv_mediainput_async_test_opencomposestop::ConfigComposer: Error - ConfigMp43gpComposer failed"));

            return false;
        }
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////
bool pv_mediainput_async_test_opencomposestop::ConfigOutputFile()
{

    PvmfFileOutputNodeConfigInterface* clipConfig = OSCL_STATIC_CAST(PvmfFileOutputNodeConfigInterface*, iComposerConfig);
    if (!clipConfig)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "pv_mediainput_async_test_opencomposestop::ConfigAmrComposer: Error - Invalid iComposerConfig"));

        return false;
    }

    if (!iUseExtrnFileDesc)
    {
        if (clipConfig->SetOutputFileName(iOutputFileName) != PVMFSuccess)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "pv_mediainput_async_test_opencomposestop::ConfigAmrComposer: Error - SetOutputFileName failed"));

            return false;
        }
    }
    else
    {
        char* fname[ARRAY_SIZE];
        oscl_UnicodeToUTF8(iOutputFileName.get_str(), iOutputFileName.get_size(), (char*)fname, ARRAY_SIZE);

        FILE *fp = fopen((char*)fname, "w+b");
        if (fp)
        {
            iFileHandle = OSCL_NEW(OsclFileHandle, (fp));
            if (clipConfig->SetOutputFileDescriptor(iFileHandle) != PVMFSuccess)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "pv_mediainput_async_test_opencomposestop::ConfigAmrComposer: Error - SetOutputFileName failed"));

                return false;
            }
        }

    }
    return true;
}

////////////////////////////////////////////////////////////////////////////
bool pv_mediainput_async_test_opencomposestop::ConfigMp43gpComposer()
{

    PVMp4FFCNClipConfigInterface* clipConfig;
    clipConfig = OSCL_STATIC_CAST(PVMp4FFCNClipConfigInterface*, iComposerConfig);
    if (!clipConfig)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "pv_mediainput_async_test_opencomposestop::ConfigMp43gpComposer: Error - iComposerConfig==NULL"));

        return false;
    }
    PvmfAssetInfo3GPPLocationStruct aLocation_info;

    OSCL_wHeapString<OsclMemAllocator> versionString = _STRLIT("Latest Version");
    OSCL_wHeapString<OsclMemAllocator> titleString = _STRLIT("This is the test title");
    OSCL_wHeapString<OsclMemAllocator> authorString = _STRLIT("Unknown");
    OSCL_wHeapString<OsclMemAllocator> copyrightString = _STRLIT("PV Copyright");
    OSCL_wHeapString<OsclMemAllocator> descriptionString = _STRLIT("New Song");
    OSCL_wHeapString<OsclMemAllocator> ratingString = _STRLIT("Five");
    OSCL_wHeapString<OsclMemAllocator> performerString = _STRLIT("Performer");
    OSCL_wHeapString<OsclMemAllocator> genreString = _STRLIT("Rock");
    OSCL_wHeapString<OsclMemAllocator> classificationInfoString = _STRLIT("Rock Music");
    OSCL_wHeapString<OsclMemAllocator> keyWordInfoString1 = _STRLIT("One");
    OSCL_wHeapString<OsclMemAllocator> keyWordInfoString2 = _STRLIT("Two");
    OSCL_wHeapString<OsclMemAllocator> keyWordInfoString3 = _STRLIT("Three");
    OSCL_wHeapString<OsclMemAllocator> location_name = _STRLIT("PV");
    OSCL_wHeapString<OsclMemAllocator> astronomical_body = _STRLIT("PV");
    OSCL_wHeapString<OsclMemAllocator> additional_notes = _STRLIT("PV");
    OSCL_wHeapString<OsclMemAllocator> iAlbumTitle = _STRLIT("albumtitle");
    uint16 iRecordingYear = 2008;

    OSCL_HeapString<OsclMemAllocator> lang_code = "eng";

    aLocation_info._location_name = NULL;
    uint32 size = location_name.get_size();
    aLocation_info._location_name = (oscl_wchar *)oscl_malloc(sizeof(oscl_wchar) * size + 10);
    oscl_strncpy(aLocation_info._location_name, location_name.get_cstr(), size);
    aLocation_info._location_name[size+1] = 0;

    size = astronomical_body.get_size();
    aLocation_info._astronomical_body = NULL;
    aLocation_info._astronomical_body = (oscl_wchar *)oscl_malloc(sizeof(oscl_wchar) * size + 10);
    oscl_strncpy(aLocation_info._astronomical_body, astronomical_body.get_cstr(), size);
    aLocation_info._astronomical_body[size+1] = 0;


    size = additional_notes.get_size();
    aLocation_info._additional_notes = NULL;
    aLocation_info._additional_notes = (oscl_wchar *)oscl_malloc(sizeof(oscl_wchar) * size + 10);
    oscl_strncpy(aLocation_info._additional_notes, additional_notes.get_cstr(), size);
    aLocation_info._additional_notes[size+1] = 0;

    aLocation_info._role = 2;
    aLocation_info._longitude = 0;
    aLocation_info._latitude = 0;
    aLocation_info._altitude = 0;
    aLocation_info.Lang_code = lang_code;


    uint32 classificationEntity = 0;
    uint16 classificationTable = 0;

    clipConfig->SetOutputFileName(iOutputFileName);
    clipConfig->SetPresentationTimescale(1000);
    clipConfig->SetVersion(versionString, lang_code);
    clipConfig->SetTitle(titleString, lang_code);
    clipConfig->SetAuthor(authorString, lang_code);
    clipConfig->SetCopyright(copyrightString, lang_code);
    clipConfig->SetDescription(descriptionString, lang_code);
    clipConfig->SetRating(ratingString, lang_code);
    clipConfig->SetPerformer(performerString, lang_code);
    clipConfig->SetGenre(genreString, lang_code);
    clipConfig->SetClassification(classificationInfoString, classificationEntity, classificationTable, lang_code);
    clipConfig->SetKeyWord(keyWordInfoString1, lang_code);
    clipConfig->SetKeyWord(keyWordInfoString2, lang_code);
    clipConfig->SetKeyWord(keyWordInfoString3, lang_code);


    clipConfig->SetLocationInfo(aLocation_info);


    clipConfig->SetAlbumInfo(iAlbumTitle, lang_code);


    clipConfig->SetRecordingYear(iRecordingYear);

    if (iTestCaseNum == K3GPPDownloadModeTest || iTestCaseNum == K3GPPDownloadModeLongetivityTest)
    {
        clipConfig->SetAuthoringMode(PVMP4FFCN_3GPP_DOWNLOAD_MODE);
    }
    else if (iTestCaseNum == K3GPPProgressiveDownloadModeTest || iTestCaseNum == K3GPPProgressiveDownloadModeLongetivityTest)
    {
        clipConfig->SetAuthoringMode(PVMP4FFCN_3GPP_PROGRESSIVE_DOWNLOAD_MODE);
    }
    else if (iTestCaseNum == KMovieFragmentModeTest || iTestCaseNum == KMovieFragmentModeLongetivityTest)
    {
        clipConfig->SetAuthoringMode(PVMP4FFCN_MOVIE_FRAGMENT_MODE);
    }




    if (aLocation_info._location_name != NULL)
        OSCL_FREE(aLocation_info._location_name);

    if (aLocation_info._astronomical_body != NULL)
        OSCL_FREE(aLocation_info._astronomical_body);

    if (aLocation_info._additional_notes != NULL)
        OSCL_FREE(aLocation_info._additional_notes);


    return true;
}

////////////////////////////////////////////////////////////////////////////
bool pv_mediainput_async_test_opencomposestop::AddMediaTrack()
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
bool pv_mediainput_async_test_opencomposestop::ConfigureVideoEncoder()
{

    PVMp4H263EncExtensionInterface* config;
    config = OSCL_STATIC_CAST(PVMp4H263EncExtensionInterface*, iVideoEncoderConfig);
    if (!config)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "pv_mediainput_async_test_opencomposestop::ConfigureVideoEncoder: No configuration needed"));

        return true;
    }

    uint32 width = 0;
    uint32 height = 0;
    OsclFloat frameRate = 0.0;
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

    //setting the width and height
    config->SetOutputFrameSize(0, width , height);

    //setting video bitrate
    if (0 == iVideoBitrate)
    {
        config->SetOutputBitRate(0, KVideoBitrate);
    }
    else
    {
        config->SetOutputBitRate(0, iVideoBitrate);//values from config file
    }

    config->SetOutputFrameRate(0, frameRate);
    config->SetNumLayers(KNumLayers);
    config->SetIFrameInterval(KVideoIFrameInterval);
    config->SetSceneDetection(true);

    return true;
}

bool pv_mediainput_async_test_opencomposestop::ConfigureAudioEncoder()
{

    PVAudioEncExtensionInterface* config;
    config = OSCL_STATIC_CAST(PVAudioEncExtensionInterface*, iAudioEncoderConfig);
    if (!config)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "pv_mediainput_async_test_opencomposestop::Encoder: No configuration needed"));

        return true;
    }

    if (!PVAETestNodeConfig::ConfigureAudioEncoder(iAudioEncoderConfig, iAudioEncoderMimeType, iAudioBitrate))
    {
        return false;
    }
    return true;
}
////////////////////////////////////////////////////////////////////////////
void pv_mediainput_async_test_opencomposestop::ResetAuthorConfig()
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
void pv_mediainput_async_test_opencomposestop::Cleanup()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "pv_mediainput_async_test_opencomposestop::Cleanup"));

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
    if (iFileHandle)
    {
        OSCL_DELETE(iFileHandle);
        iFileHandle = NULL;
    }
}


////////////////////////////////////////////////////////////////////////////
void pv_mediainput_async_test_opencomposestop::Run()
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
                            (0, "******pv_mediainput_async_test_opencomposestop::iTestCaseNum:%d******", iTestCaseNum));
            fprintf(iFile, "Creating Author Engine \n");

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
            fprintf(iFile, "Opening Author Engine\n");
            iAuthor->Open((OsclAny*)iAuthor);
        }
        break;

        case PVAE_CMD_ADD_DATA_SOURCE:
        {
            fprintf(iFile, "Add Data Source\n");
            bool aStatus = CreateTestInputs();
            if (aStatus == 0) //Failed while creating test input
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                (0, "pv_mediainput_async_test_opencomposestop::CreateTestInputs: Error - failed"));

                PVPATB_TEST_IS_TRUE(false);

                iState = PVAE_CMD_CLEANUPANDCOMPLETE;

                RunIfNotReady();
            }

        }
        break;

        case PVAE_CMD_SELECT_COMPOSER:
        {
            fprintf(iFile, "Select Composer\n");
            iAuthor->SelectComposer(iComposerMimeType, iComposerConfig,
                                    (OsclAny*)iAuthor);
        }
        break;

        case PVAE_CMD_QUERY_INTERFACE:

            fprintf(iFile, "Query Interface\n");

            if (!QueryComposerOutputInterface())
            {
                iState = PVAE_CMD_ADD_MEDIA_TRACK;
                RunIfNotReady();
            }
            break;

        case PVAE_CMD_ADD_MEDIA_TRACK:
        {
            fprintf(iFile, "Add Media Track\n");
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
            fprintf(iFile, "Initializing  Author Engine\n");
            iAuthor->Init((OsclAny*)iAuthor);
        }
        break;

        case PVAE_CMD_START:
        {
            fprintf(iFile, "Starting Author Engine\n");
            iAuthor->Start();
        }
        break;

        case PVAE_CMD_STOP:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                            (0, "pv_mediainput_async_test_opencomposestop::Run: Command Stop"));

            fprintf(iFile, "Error: Recording timeout, stop engine please wait *******\n");
            iAuthor->Stop((OsclAny*)iAuthor);

            //flag error as no EOS was found and recording timedout
            PVPATB_TEST_IS_TRUE(false);

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
            fprintf(iFile, "Pause Initiated ********\n");
            iAuthor->Pause((OsclAny*)iAuthor);
        }
        break;

        case PVAE_CMD_RESUME:
        {
            fprintf(iFile, "Resume Athoring *******\n");
            iAuthor->Resume((OsclAny*)iAuthor);
        }
        break;

        case PVAE_CMD_QUERY_INTERFACE2:
            fprintf(iFile, "Query Interface2\n");
            iAuthor->QueryInterface(PVMI_CAPABILITY_AND_CONFIG_PVUUID, (PVInterface*&)iAuthorCapConfigIF, (OsclAny*)iAuthor);
            break;
        case PVAE_CMD_CAPCONFIG_ASYNC:
        {
            fprintf(iFile, "Cap Config ASync\n");
            CapConfigAsync();

        }
        break;

        case PVAE_CMD_CAPCONFIG_SYNC:
            // set configuration MIME strings Synchronously
        {
            fprintf(iFile, "Cap Config Sync\n");

            if (!CapConfigSync()) //CapConfig failed
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                (0, "pv_mediainput_async_test_opencomposestop::Run: Error - CapConfigSync failed"));
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                iState = PVAE_CMD_START;
                RunIfNotReady();
            }
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
            fprintf(iFile, "Recording Please Wait*******\n");
            if (!iAuthoringCount)
            {
                iState = PVAE_CMD_STOP;
                RunIfNotReady(iTestDuration);
            }
            else
            {
                iState = PVAE_CMD_RECORDING;
                RunIfNotReady(iTestDuration);
                iAuthoringCount--;
            }
        }
        break;

        default:
            break;
    } //end switch
}

////////////////////////////////////////////////////////////////////////////
void pv_mediainput_async_test_opencomposestop::CommandCompleted(const PVCmdResponse& aResponse)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "pv_mediainput_async_test_opencomposestop::CommandCompleted iState:%d", iState));


    if (aResponse.GetCmdStatus() != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "pv_mediainput_async_test_opencomposestop::CommandCompleted iState:%d FAILED", iState));
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
                                    (0, "pv_mediainput_async_test_opencomposestop::CommandCompleted: Error - ConfigComposer failed"));

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

        }
        break;
        case PVAE_CMD_QUERY_INTERFACE:
        {
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                ConfigComposerOutput();
                iState = PVAE_CMD_ADD_MEDIA_TRACK;
                RunIfNotReady();
            }
            else
            {
                // QueryInterface failed
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

                iState = PVAE_CMD_QUERY_INTERFACE2;
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
        case PVAE_CMD_QUERY_INTERFACE2:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = PVAE_CMD_INIT;
                RunIfNotReady();
            }
            else
            {
                // QueryInterface2 failed
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_RESET;
                RunIfNotReady();
            }
            break;

        case PVAE_CMD_INIT:
        {
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if (iTestCaseNum == CapConfigTest)
                {
                    iState = PVAE_CMD_CAPCONFIG_ASYNC;
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
        case PVAE_CMD_CAPCONFIG_ASYNC:
        {

            if (aResponse.GetCmdStatus() != PVMFSuccess)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                (0, "pv_mediainput_async_test_opencomposestop::Run: Error - CapConfigAsync failed"));
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                PVPATB_TEST_IS_TRUE(true);
                iState = PVAE_CMD_START;
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
                fprintf(iFile, "PAUSED: Resume after 10 seconds*******\n");
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
        }
        break;

        case PVAE_CMD_RESUME:
        {
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = PVAE_CMD_RECORDING;

                //Start Authoring again
                RunIfNotReady();
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
                    else if (iMediaInputType == PVMF_MIME_WAVFF)
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

bool pv_mediainput_async_test_opencomposestop::ConfigComposerOutput()
{
    PvmfComposerSizeAndDurationInterface* config =
        OSCL_REINTERPRET_CAST(PvmfComposerSizeAndDurationInterface*, iOutputSizeAndDurationConfig);
    if (!config)
    {
        return false;
    }
    bool enabled = false;
    uint32 configData = 0;
    switch (iTestCaseNum)
    {
        case KMaxFileSizeTest:
        case KMaxFileSizeLongetivityTest:
        {
            if (config->SetMaxFileSize(true, KMaxFileSize) != PVMFSuccess)
            {
                return false;
            }
            config->GetMaxFileSizeConfig(enabled, configData);
            if (!enabled || configData != KMaxFileSize)
            {
                return false;
            }
        }
        break;

        case PVMediaInput_Open_Compose_Stop_Test:
        case PVMediaInput_Open_Compose_Stop_Test_UsingExternalFileHandle:
        {
            if (config->SetFileSizeProgressReport(true, KFileSizeProgressFreq) != PVMFSuccess)
            {
                return false;
            }
            config->GetFileSizeProgressReportConfig(enabled, configData);
            if (!enabled || configData != KFileSizeProgressFreq)
            {
                return false;
            }
            if (config->SetDurationProgressReport(true, KDurationProgressFreq) != PVMFSuccess)
            {
                return false;
            }
            config->GetDurationProgressReportConfig(enabled, configData);
            if (!enabled || configData != KDurationProgressFreq)
            {
                return false;
            }
        }
        break;

        default:
            break;
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////
bool pv_mediainput_async_test_opencomposestop::QueryComposerOutputInterface()
{
    switch (iTestCaseNum)
    {
        case PVMediaInput_Open_Compose_Stop_Test:
        case PVMediaInput_Open_Compose_Stop_Test_UsingExternalFileHandle:
        case KMaxFileSizeTest:
        {
            //iPendingCmds.push_back(PVAE_CMD_QUERY_INTERFACE);
            iAuthor->QueryInterface(PvmfComposerSizeAndDurationUuid,
                                    iOutputSizeAndDurationConfig, (OsclAny*)iAuthor);
            return true;
        }
        break;
        default:
            return false;
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////
// function for setting configuration MIME strings Asynchronously

bool pv_mediainput_async_test_opencomposestop::CapConfigAsync()
{
    if (iTestCaseNum == CapConfigTest)
    {
        // Set the observer
        iAuthorCapConfigIF->setObserver(this);

        iKeyStringSetAsync = _STRLIT_CHAR("x-pvmf/author/productinfo/dummyprod1;valtype=int32");
        iKVPSetAsync.key = iKeyStringSetAsync.get_str();
        iKVPSetAsync.value.int32_value = 2;

        // set the parameter
        iErrorKVP = NULL;
        iAuthorCapConfigIF->setParametersAsync(NULL, &iKVPSetAsync, 1, iErrorKVP);

        return true;
    }
    else
    {
        return false;
    }
}

////////////////////////////////////////////////////////////////////////////
// function for setting configuration MIME strings Synchronously

bool pv_mediainput_async_test_opencomposestop::CapConfigSync()
{
    if (iTestCaseNum == CapConfigTest)
    {
        // set the mime strings here
        // capabilty and configuration feature implemented here
        // set the config parameters using MIME strings here

        // set the KVP array
        PvmiKvp paramkvp1;
        // set cache size for composer node
        //	PVMI_FILEIO_PV_CACHE_SIZE (MACRO defined in "pvmi_fileio_kvp.h")
        OSCL_StackString<64> paramkey1(_STRLIT_CHAR(PVMI_FILEIO_PV_CACHE_SIZE));
        paramkey1 += _STRLIT_CHAR(";valtype=uint32");
        // set KVP values
        // for composer node
        paramkvp1.key = paramkey1.get_str();
        paramkvp1.value.uint32_value = 1024;

        iErrorKVP = NULL;

        // verifyParametersSync will give Failure for test cases not using MP4 composer("fileio") node so commented
        // Verify the new settings
        //if (iAuthorCapConfigIF->verifyParametersSync(NULL, &paramkvp1, 1) == PVMFSuccess)
        {
            // set the parameter
            iAuthorCapConfigIF->setParametersSync(NULL, &paramkvp1, 1, iErrorKVP);
            if (iErrorKVP == NULL)
            {
                // Check by calling get
                PvmiKvp* retparam = NULL;
                int retnumparam = 0;
                paramkey1 += _STRLIT_CHAR(";attr=cur");
                // pass the string
                // retrieve a MIME string back
                iAuthorCapConfigIF->getParametersSync(NULL, paramkey1.get_str(), retparam, retnumparam, NULL);

                if ((retparam != NULL) && (retnumparam == 1))
                {
                    // release the parameters
                    if (iAuthorCapConfigIF->releaseParameters(NULL, retparam, retnumparam) != PVMFSuccess)
                    {
                        return false;
                    }
                }
            }
        }
        /*
        else
        {
        	return false;
        }*/

        //MIME for file output node
        // set here any parameter here
        OSCL_StackString<64> paramkey2(_STRLIT_CHAR("x-pvmf/file/output/parameter1;valtype=uint32"));

        // for file output node
        paramkvp1.key = paramkey2.get_str();
        paramkvp1.value.int32_value = 144;

        // set the value in node using SetParameterSync f(n) here
        iAuthorCapConfigIF->setParametersSync(NULL, &paramkvp1, 1, iErrorKVP);

        // MIME for media io node
        // set parameters here
        OSCL_StackString<64> paramkey3(_STRLIT_CHAR("x-pvmf/media-io/parameter1;valtype=uint32"));

        // for media io node
        paramkvp1.key = paramkey3.get_str();
        paramkvp1.value.int32_value = 144;

        // set the value in node using SetParameterSync f(n) here
        iAuthorCapConfigIF->setParametersSync(NULL, &paramkvp1, 1, iErrorKVP);

        if (iErrorKVP == NULL)
        {
            // Check by calling get
            PvmiKvp* retparam = NULL;
            int retnumparam = 0;
            paramkey3 += _STRLIT_CHAR(";attr=cur");
            // pass the string
            // retrieve a MIME string back
            iAuthorCapConfigIF->getParametersSync(NULL, paramkey3.get_str(), retparam, retnumparam, NULL);
            if ((retparam != NULL) && (retnumparam == 1))
            {
                // release the parameters
                if (iAuthorCapConfigIF->releaseParameters(NULL, retparam, retnumparam) != PVMFSuccess)
                {
                    return false;
                }
            }
        }

        // MIME for amr encoder
        // set the parameter here

        // MIME string here "x-pvmf/encoder/audio/sampling_rate;valtype=uint32"
        OSCL_StackString<64> paramkey4(_STRLIT_CHAR(MOUT_AUDIO_SAMPLING_RATE_KEY));
        // MOUT_AUDIO_SAMPLING_RATE_KEY (MACRO defined in "pvmi_kvp.h")
        // for amr encoder node
        paramkvp1.key = paramkey4.get_str();
        paramkvp1.value.int32_value = 8000;

        // set the value in node using SetParameterSync f(n) here
        iAuthorCapConfigIF->setParametersSync(NULL, &paramkvp1, 1, iErrorKVP);

        if (iErrorKVP == NULL)
        {
            // Check by calling get
            PvmiKvp* retparam = NULL;
            int retnumparam = 0;
            paramkey1 += _STRLIT_CHAR(";attr=cur");
            // pass the string
            // retrieve a MIME string back
            iAuthorCapConfigIF->getParametersSync(NULL, paramkey4.get_str(), retparam, retnumparam, NULL);
            if ((retparam != NULL) && (retnumparam == 1))
            {
                // release the parameters
                if (iAuthorCapConfigIF->releaseParameters(NULL, retparam, retnumparam) != PVMFSuccess)
                {
                    return false;
                }
            }
        }


        // MIME for video encoder
        // set the parameter frame width in the node (use combinations for width n height default is 176 by 144)

        // MIME string here "x-pvmf/video/render/output_width;valtype=uint32"
        OSCL_StackString<64> paramkey5(_STRLIT_CHAR(MOUT_VIDEO_OUTPUT_WIDTH_KEY));
        // MACRO "MOUT_VIDEO_OUTPUT_WIDTH_KEY" been defined in "pvmi_kvp.h"
        // for video encoder node
        paramkvp1.key = paramkey5.get_str();
        paramkvp1.value.uint32_value = 128;
        // set the value in node using SetParameterSync f(n) here
        iAuthorCapConfigIF->setParametersSync(NULL, &paramkvp1, 1, iErrorKVP);

        if (iErrorKVP == NULL)
        {
            // Check by calling get
            PvmiKvp* retparam = NULL;
            int retnumparam = 0;
            paramkey1 += _STRLIT_CHAR(";attr=cur");
            // pass the string
            // retrieve a MIME string back
            iAuthorCapConfigIF->getParametersSync(NULL, paramkey5.get_str(), retparam, retnumparam, NULL);
            if ((retparam != NULL) && (retnumparam == 1))
            {
                // release the parameters
                if (iAuthorCapConfigIF->releaseParameters(NULL, retparam, retnumparam) != PVMFSuccess)
                {
                    return false;
                }
            }
        }

        // MIME for AVC Encoder
        // set the parameter encoding mode

        // MIME string here "x-pvmf/avc/encoder/encoding_mode;valtype=uint32"
        OSCL_StackString<64> paramkey6(_STRLIT_CHAR(PVMF_AVC_ENCODER_ENCODINGMODE_KEY));
        // MACRO "PVMF_AVC_ENCODER_ENCODINGMODE_KEY" been defined in "pvmi_kvp.h"
        // for AVC Encoder node
        paramkvp1.key = paramkey6.get_str();
        paramkvp1.value.uint32_value = 3;//EAVCEI_ENCMODE_TWOWAY;
        // set the value in node using SetParameterSync f(n) here
        iAuthorCapConfigIF->setParametersSync(NULL, &paramkvp1, 1, iErrorKVP);

        if (iErrorKVP == NULL)
        {
            // Check by calling get
            PvmiKvp* retparam = NULL;
            int retnumparam = 0;
            paramkey1 += _STRLIT_CHAR(";attr=cur");
            // pass the string
            // retrieve a MIME string back
            iAuthorCapConfigIF->getParametersSync(NULL, paramkey6.get_str(), retparam, retnumparam, NULL);
            if ((retparam != NULL) && (retnumparam == 1))
            {
                // release the parameters
                if (iAuthorCapConfigIF->releaseParameters(NULL, retparam, retnumparam) != PVMFSuccess)
                {
                    return false;
                }
            }
        }

        return true;
    }
    else
    {
        return true;
    }

}

void pv_mediainput_async_test_opencomposestop::SignalEvent(int32 aReq_Id)
{
    if (aReq_Id == 0) //For PVAE_CMD_CAPCONFIG_SET_PARAMETERS
    {
        if (iErrorKVP != NULL)
        {
            // There was an error in setParameterAsync()
            PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                            (0, "pv_mediainput_async_test_opencomposestop::SignalEvent: setParametersAsync failed"));
            PVPATB_TEST_IS_TRUE(false);
            iObserver->CompleteTest(*iTestCase);
        }
        else
        {
            iState = PVAE_CMD_CAPCONFIG_SYNC;
            RunIfNotReady();
        }
    }
}

