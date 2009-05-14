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
#define TEST_PV_AUTHOR_ENGINE_TESTSET4_H_INCLUDED

#ifndef PVAETEST_H_INCLUDED
#include "pvaetest.h"
#endif

#ifndef TEST_PV_AUTHOR_ENGINE_TYPEDEFS_H
#include "test_pv_author_engine_typedefs.h"
#endif
#ifndef PVMI_CONFIG_AND_CAPABILITY_H_INCLUDED
#include "pvmi_config_and_capability.h"
#endif
#define PVPATB_TEST_IS_TRUE( condition ) (iTestCase->test_is_true_stub( (condition), (#condition), __FILE__, __LINE__ ))


class pvauthor_async_compressed_test_errorhandling: public pvauthor_async_test_base
{
    public:
        pvauthor_async_compressed_test_errorhandling(PVAuthorAsyncTestParam aTestParam, const char* aInputFileNameAudio,
                const char* aInputFileNameVideo, const char* aInputFileNameText, const char* aOutputFileName,
                PVAETestInputType aAudioInputType, PVAETestInputType aVideoInputType,
                PVAETestInputType aTextInputType, const char* aComposerMimeType,
                const char* aAudioEncoderMimeType, const char* aVideoEncoderMimeType,
                const char* aTextEncoderMimeType, AVTConfig aAVTConfig, bool aPauseResumeEnable, uint32 aAuthoringTime,
                FAIL_STATE aFailState)

                : pvauthor_async_test_base(aTestParam)

                , iComposerMimeType(aComposerMimeType)
                , iAudioEncoderMimeType(aAudioEncoderMimeType)
                , iVideoEncoderMimeType(aVideoEncoderMimeType)
                , iTextEncoderMimeType(aTextEncoderMimeType)
                , iAudioInputType(aAudioInputType)
                , iVideoInputType(aVideoInputType)
                , iTextInputType(aTextInputType)
                , iAuthor(NULL)
                , iComposer(NULL)
                , iComposerConfig(NULL)
                , iAudioEncoderConfig(NULL)
                , iVideoEncoderConfig(NULL)
                , iTextEncoderConfig(NULL)
                , iOutputSizeAndDurationConfig(NULL)
                , iAVTConfig(aAVTConfig)
                , bAudioTrack(true)
                , bVideoTrack(true)
                , iPauseResumeEnable(aPauseResumeEnable)
                , iAuthoringTime(aAuthoringTime)
                , iAuthoringCounter(0)
                , iFailureState(aFailState)

        {
            iLogger = PVLogger::GetLoggerObject("pvauthor_async_compressed_test_errorhandling");

            if (iAuthoringTime > 0)
            {
                // KAuthoringSessionUnit is kept here as 1800 secs
                // if iAuthoringTime > AuthoringSessionUnit
                // set the counter here for loop
                if (iAuthoringTime > KAuthoringSessionUnit)
                {
                    // iAuthoringTime time set at cmd line is in secs
                    iAuthoringCounter = iAuthoringTime / KAuthoringSessionUnit;
                    iTestDuration = KAuthoringSessionUnit * 1000 * 1000;
                }
                else
                {
                    // iAuthoringCounter is not set here
                    iTestDuration = iAuthoringTime * 1000 * 1000;
                }
            }
            else
            {
                // no Authoring time given
                iTestDuration = KAuthoringSessionUnit * 1000 * 1000; // use default authoring time here
            }
            iInputFileNameAudio = NULL;
            iInputFileNameVideo = NULL;
            iInputFileNameText = NULL;
            iOutputFileName = NULL;

            if (oscl_strlen(aInputFileNameAudio) != 0)
            {
                oscl_wchar output1[ARRAY_SIZE];
                oscl_UTF8ToUnicode(aInputFileNameAudio, oscl_strlen(aInputFileNameAudio), output1, ARRAY_SIZE);
                iInputFileNameAudio.set(output1, oscl_strlen(output1));
            }

            if (oscl_strlen(aInputFileNameVideo) != 0)
            {
                oscl_wchar output2[ARRAY_SIZE];
                oscl_UTF8ToUnicode(aInputFileNameVideo, oscl_strlen(aInputFileNameVideo), output2, ARRAY_SIZE);
                iInputFileNameVideo.set(output2, oscl_strlen(output2));
            }

            if (oscl_strlen(aOutputFileName) != 0)
            {
                oscl_wchar output3[ARRAY_SIZE];
                oscl_UTF8ToUnicode(aOutputFileName, oscl_strlen(aOutputFileName), output3, ARRAY_SIZE);
                iOutputFileName.set(output3, oscl_strlen(output3));
            }

            if (oscl_strlen(aInputFileNameText) != 0)
            {
                oscl_wchar output4[ARRAY_SIZE];
                oscl_UTF8ToUnicode(aInputFileNameText, oscl_strlen(aInputFileNameText), output4, ARRAY_SIZE);
                iInputFileNameText.set(output4, oscl_strlen(output4));
            }
            iRemoveDataSourceDone = 0;
        }

        ~pvauthor_async_compressed_test_errorhandling()
        {
            Cleanup();
        }
        void Run();
        void StartTest();

        // Author engine observer functions
        void HandleErrorEvent(const PVAsyncErrorEvent& aEvent);
        void HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent);
        void CommandCompleted(const PVCmdResponse& aResponse);

        PVAECmdType iState;
        // Test output
        OSCL_wHeapString<OsclMemAllocator> iOutputFileName;
        OSCL_wHeapString<OsclMemAllocator> iInputFileNameAudio;
        OSCL_wHeapString<OsclMemAllocator> iInputFileNameVideo;
        OSCL_wHeapString<OsclMemAllocator> iInputFileNameText;

        OSCL_HeapString<OsclMemAllocator> iComposerMimeType;
        OSCL_HeapString<OsclMemAllocator> iAudioEncoderMimeType;
        OSCL_HeapString<OsclMemAllocator> iVideoEncoderMimeType;
        OSCL_HeapString<OsclMemAllocator> iTextEncoderMimeType;
        // Test inputs
        PVAETestInputType iAudioInputType;
        PVAETestInputType iVideoInputType;
        PVAETestInputType iTextInputType;

    private:
        // Methods to create test input nodes and add to author engine
        //bool CreateTestInputs();
        int CreateAudioInput();
        int CreateVideoInput();
        int CreateTextInput();
        bool AddDataSource(PVAETestInput& aInput);

        // Methods to configure composers
        void SelectComposer();
        bool ConfigComposer();
        bool ConfigAmrAacComposer();
        bool ConfigMp43gpComposer();

        // Method to configure max filesize/duration interface
        bool QueryComposerOutputInterface();
        bool ConfigComposerOutput();

        // Methods to add media tracks
        bool AddAudioMediaTrack();
        bool AddVideoMediaTrack();
        bool AddTextMediaTrack();

        // Methods to configure encoders
        bool ConfigureVideoEncoder();
        bool ConfigureAudioEncoder();
        bool ConfigureTextEncoder();

        bool DeleteTestInputs();

        void ResetAuthorConfig();

        void Cleanup();
        /////////////////////////////////////////////
        // Author engine related variables
        PVAuthorEngineInterface* iAuthor;
        OsclAny* iComposer;
        PVInterface* iComposerConfig;
        PVInterface* iAudioEncoderConfig;
        PVInterface* iVideoEncoderConfig;
        PVInterface* iTextEncoderConfig;
        PVInterface* iOutputSizeAndDurationConfig;

        // Meta data strings
        OSCL_wHeapString<OsclMemAllocator> iVersionString;
        OSCL_wHeapString<OsclMemAllocator> iTitleString;
        OSCL_wHeapString<OsclMemAllocator> iAuthorString;
        OSCL_wHeapString<OsclMemAllocator> iCopyrightString;
        OSCL_wHeapString<OsclMemAllocator> iDescriptionString;
        OSCL_wHeapString<OsclMemAllocator> iRatingString;
        OSCL_wHeapString<OsclMemAllocator> iAlbumTitle;
        uint16							   iRecordingYear;

        AVTConfig iAVTConfig;
        Oscl_Vector<PVAETestInput, OsclMemAllocator> iTestInputs;

        PVLogger* iLogger;
        bool bAudioTrack; //keep track if audio track has been added
        bool bVideoTrack;
        bool iPauseResumeEnable;
        uint iRemoveDataSourceDone;//to keep track of how many removeDataSource calls done
        uint32 iAuthoringTime;
        int iAuthoringCounter;
        uint32 iTestDuration;
        FAIL_STATE	iFailureState;
        //For CapConfig
        PvmiCapabilityAndConfig* iAuthorCapConfigIF;
        PvmiKvp* iErrorKVP;
};

#endif
