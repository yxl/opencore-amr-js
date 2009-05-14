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
#ifndef PVAETEST_H_INCLUDED
#define PVAETEST_H_INCLUDED

#ifndef TEST_PV_AUTHOR_ENGINE_TYPEDEFS_H
#include "test_pv_author_engine_typedefs.h"
#endif

////////////////////////////////////////////////////////////////////////////
// test_base-based class which will run async tests on pvPlayer engine
class PVAuthorEngineTest : public test_case,
            public pvauthor_async_test_observer,
            public PVLoggerSchedulerSetup

{
    public:
        PVAuthorEngineTest(FILE* aStdOut, int32 aFirstTest, int32 aLastTest, const char* aInputFileName1,
                           const char* aInputFileName2, const char* aInputFileName3, const char* aOutputFileName,
                           AVTConfig iAVTConfig, PVAETestInputType aAudioInputType,
                           PVAETestInputType aVideoInputType, PVAETestInputType aTextInputType, const char* aComposerMimeType,
                           const char* aAudioEncoderMimeType, const char* aVideoEncoderMimeType, const char* aTextEncoderMimeType, uint32 aAuthoringTime);

        ~PVAuthorEngineTest();
        // From test_case
        virtual void test();
        // From pvauthor_async_test_observer
        void CompleteTest(test_case&);

        void RunTestCases();

    private:
        pvauthor_async_test_base* iCurrentTest;

        int32 iFirstTest;
        int32 iLastTest;
        int32 iNextTestCase;

        FILE* iStdOut;

        // For test results
        int iTotalSuccess;
        int iTotalError;
        int iTotalFail;

        // For memory statistics
        uint32 iTotalAlloc;
        uint32 iTotalBytes;
        uint32 iAllocFails;
        uint32 iNumAllocs;

        //Needed by the test to be run
        OSCL_HeapString<OsclMemAllocator> iOutputFileName;
        OSCL_HeapString<OsclMemAllocator> iInputFileNameAudio;
        OSCL_HeapString<OsclMemAllocator> iInputFileNameVideo;
        OSCL_HeapString<OsclMemAllocator> iInputFileNameText;
        OSCL_HeapString<OsclMemAllocator> iInputFileNameTextLog;
        OSCL_HeapString<OsclMemAllocator> iInputFileNameTextConfig;
        PVAETestInputType iAudioInputType;
        PVAETestInputType iVideoInputType;
        PVAETestInputType iTextInputType;
        OSCL_HeapString<OsclMemAllocator> iComposerMimeType;
        OSCL_HeapString<OsclMemAllocator> iAudioEncoderMimeType;
        OSCL_HeapString<OsclMemAllocator> iVideoEncoderMimeType;
        OSCL_HeapString<OsclMemAllocator> iTextEncoderMimeType;
        AVTConfig iAVTConfig;
        uint32 iAuthoringTime;
};


///////////////////////////////////////////////////////////////////////////
class PVAuthorEngineTestSuite : public test_case
{
    public:
        PVAuthorEngineTestSuite(FILE* aStdOut, int32 aFirstTest, int32 aLastTest, const char* iInputFileName1, const char* iInputFileName2, const char* iInputFileName3, const char* iOutputFileName, AVTConfig iAVTConfig, PVAETestInputType aAudioInputType, PVAETestInputType aVideoInputType,  PVAETestInputType aTextInputType, const char* aComposerMimeType, const char* aAudioEncoderMimeType, const char* aVideoEncoderMimeType, const char* aTextEncoderMimeType, uint aAuthoringTime) : test_case()
        {
            adopt_test_case(new PVAuthorEngineTest(aStdOut, aFirstTest, aLastTest, iInputFileName1, iInputFileName2, iInputFileName3, iOutputFileName,
                                                   iAVTConfig, aAudioInputType, aVideoInputType, aTextInputType, aComposerMimeType, aAudioEncoderMimeType, aVideoEncoderMimeType, aTextEncoderMimeType, aAuthoringTime));
        }
        ~PVAuthorEngineTestSuite() {};
};

#endif // PVAETEST_H_INCLUDED
