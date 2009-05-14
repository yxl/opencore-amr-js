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
#define TEST_PV_AUTHOR_ENGINE_TESTSET5_H_INCLUDED

#ifndef TEST_PV_MEDIAINPUT_AUTHOR_ENGINE_H
#include "test_pv_mediainput_author_engine.h"
#endif
#ifndef PVMI_FILEIO_KVP_H_INCLUDED
#include "pvmi_fileio_kvp.h"
#endif

#define PVPATB_TEST_IS_TRUE( condition ) (iTestCase->test_is_true_stub( (condition), (#condition), __FILE__, __LINE__ ))

#define TEST_TIMEOUT_FACTOR 8  //take into account AVC encoder and symbian emulator
class pv_mediainput_async_test_opencomposestop: public pvauthor_async_test_base,
            public PvmiConfigAndCapabilityCmdObserver
{
    public:
        pv_mediainput_async_test_opencomposestop(PVAuthorAsyncTestParam aTestParam, PVMediaInputTestParam aMediaParam, bool aPauseResumeEnable,
                bool aUseExtrnFileDesc = false)
                : pvauthor_async_test_base(aTestParam)
                , iOutputFileName(NULL)
                , iInputFileName(NULL)
                , iComposerMimeType(aMediaParam.iComposerInfo)
                , iAudioEncoderMimeType(aMediaParam.iAudioEncInfo)
                , iVideoEncoderMimeType(aMediaParam.iVideoEncInfo)
                , iMediaInputType(aMediaParam.iInputFormat)
                , iLoopTime(aMediaParam.iLoopTime)
                , iAuthor(NULL)
                , iComposer(NULL)
                , iFileParser(NULL)
                , iComposerConfig(NULL)
                , iAudioEncoderConfig(NULL)
                , iVideoEncoderConfig(NULL)
                , iPendingCmds(0)
                , iPauseResumeEnable(aPauseResumeEnable)
                , iRemoveDataSourceDone(0)
                , iAddAudioMediaTrack(false)
                , iAddVideoMediaTrack(false)
                , iFile(aTestParam.iStdOut)
                , iRealTimeAuthoring(aMediaParam.iRealTimeAuthoring)
                , iVideoBitrate(aMediaParam.iVideoBitrate)
                , iAudioBitrate(aMediaParam.iAudioBitrate)
                , iFrameRate(aMediaParam.iFrameRate)
                , iSamplingRate(aMediaParam.iSamplingRate)
                , iUseExtrnFileDesc(aUseExtrnFileDesc)

        {
            OSCL_UNUSED_ARG(iAudioBitrate);
            OSCL_UNUSED_ARG(iFrameRate);
            OSCL_UNUSED_ARG(iSamplingRate);
            iFileHandle = NULL;

            iLogger = PVLogger::GetLoggerObject("pv_mediainput_async_test_opencomposestop");

            iTestDuration = KTestDuration * 1000 * 1000;
            iAuthoringCount = 0;
            if (oscl_strlen(aMediaParam.iIPFileInfo.get_cstr()) != 0)
            {
                oscl_wchar output2[ARRAY_SIZE];
                oscl_UTF8ToUnicode(aMediaParam.iIPFileInfo.get_cstr(), oscl_strlen(aMediaParam.iIPFileInfo.get_cstr()), output2, ARRAY_SIZE);
                iInputFileName.set(output2, oscl_strlen(output2));

            }

            if (oscl_strlen(aMediaParam.iOPFileInfo.get_cstr()) != 0)
            {
                oscl_wchar output1[ARRAY_SIZE];
                oscl_UTF8ToUnicode(aMediaParam.iOPFileInfo.get_cstr(), oscl_strlen(aMediaParam.iOPFileInfo.get_cstr()), output1, ARRAY_SIZE);
                iOutputFileName.set(output1, oscl_strlen(output1));
            }
        }

        ~pv_mediainput_async_test_opencomposestop()
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
        OSCL_wHeapString<OsclMemAllocator> iInputFileName;

        OSCL_HeapString<OsclMemAllocator> iComposerMimeType;
        OSCL_HeapString<OsclMemAllocator> iAudioEncoderMimeType;
        OSCL_HeapString<OsclMemAllocator> iVideoEncoderMimeType;

        //test input type
        PVMFFormatType  iMediaInputType;
        uint32 iLoopTime;

    private:
        // Methods to create test input nodes and add to author engine
        bool CreateTestInputs();
        bool AddDataSource();

        // Methods to configure composers
        bool ConfigComposer();
        bool ConfigOutputFile();
        OsclFileHandle *iFileHandle;
        bool ConfigMp43gpComposer();

        bool CapConfigSync();
        bool CapConfigAsync();
        // From PvmiConfigAndCapabilityCmdObserver
        void SignalEvent(int32 req_id);

        // Method to configure max filesize/duration interface
        bool QueryComposerOutputInterface();
        bool ConfigComposerOutput();
        // Methods to add media tracks
        bool AddMediaTrack();

        // Methods to configure encoders
        bool ConfigureVideoEncoder();
        bool ConfigureAudioEncoder();
        void ResetAuthorConfig();

        void Cleanup();

        void AddEngineCommand()
        {
            iPendingCmds++;
        }

        bool EngineCmdComplete()
        {
            if (iPendingCmds > 0)
            {
                --iPendingCmds;
            }

            if (iPendingCmds == 0)
            {
                return true;
            }

            return false;
        }

        bool IsEngineCmdPending()
        {
            if (iPendingCmds > 0)
                return true;
            else
                return false;
        }

        // Author engine related variables
        PVAuthorEngineInterface* iAuthor;
        OsclAny*				 iComposer;
        OsclAny*				 iFileParser;
        PVInterface*			 iComposerConfig;
        PVInterface*			 iAudioEncoderConfig;
        PVInterface*			 iVideoEncoderConfig;
        PVInterface*			 iOutputSizeAndDurationConfig;
        PVMIOControlComp		 iMIOComponent;
        uint32					 iPendingCmds;
        PVLogger*                iLogger;
        bool                     iPauseResumeEnable;
        uint                     iRemoveDataSourceDone;
        uint32                   iTestDuration;
        bool					 iAddAudioMediaTrack;
        bool					 iAddVideoMediaTrack;
        Oscl_FileServer          iFileServer;
        uint32					 iAuthoringCount;
        FILE*                    iFile;
        bool					 iRealTimeAuthoring;
        PvmiCapabilityAndConfig* iAuthorCapConfigIF;
        PvmiKvp					 iKVPSetAsync;
        OSCL_StackString<64>	 iKeyStringSetAsync;
        PvmiKvp*				 iErrorKVP;
        uint32					 iVideoBitrate;
        uint32					 iAudioBitrate;
        OsclFloat 				 iFrameRate;
        uint32					 iSamplingRate;
        bool					 iUseExtrnFileDesc;

};

#endif
