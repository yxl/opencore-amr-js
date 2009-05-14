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
#include "pvaetest.h"
#endif

#ifndef PVAETESTINPUT_H_INCLUDED
#include "pvaetestinput.h"
#endif

#ifndef TEST_PV_MEDIAINPUT_AUTHOR_ENGINE_H
#include "test_pv_mediainput_author_engine.h"
#endif

#ifndef TEST_PV_AUTHOR_ENGINE_TESTSET1_H_INCLUDED
#include "test_pv_author_engine_testset1.h"
#endif

#ifndef TEST_PV_AUTHOR_ENGINE_TESTSET4_H_INCLUDED
#include "test_pv_author_engine_testset4.h"
#endif

#ifndef TEST_PV_MEDIAINPUT_AUTHOR_ENGINE_H
#include "test_pv_mediainput_author_engine.h"
#endif

#if USE_OMX_ENC_NODE
#include "OMX_Core.h"
#endif

FILE* file;

// Default input settings
const uint32 KVideoBitrate = 52000;
const uint32 KVideoFrameWidth = 176;
const uint32 KVideoFrameHeight = 144;
const uint32 KVideoTimescale = 1000;
const uint32 KNumLayers = 1;

const uint32 KVideoFrameRate = 15;
const uint32 KNum20msFramesPerChunk = 10;
const uint32 KAudioBitsPerSample = 16;
const uint16 KVideoIFrameInterval = 10;
const uint8 KH263VideoProfile = 0;
const uint8 KH263VideoLevel = 10;
const uint32 KAudioBitrate = 12200;
const uint32 KAudioBitrateWB = 15850;
const uint32 KAACAudioBitrate = 64000;
const uint32 KAudioTimescale = 8000;
const uint32 KAudioTimescaleWB = 16000;
const uint32 KAudioNumChannels = 1;
const uint32 KTextTimescale = 90000;
const uint32 KTextFrameWidth = 176;
const uint32 KTextFrameHeight = 177;



const uint32 KMaxFileSize = 50000; // 50 KB
const uint32 KMaxDuration = 5000; // 5 seconds
const uint32 KFileSizeProgressFreq = 1000; // 1 KB
const uint32 KDurationProgressFreq = 1000; // 1 second
const uint32 KTestDuration = 10; // for 10 sec

// it's for setting Authoring Time Unit for selecting counter loop
// this time unit is used as default authoring time for longetivity test
const uint32 KAuthoringSessionUnit = 60; //in seconds
const uint32 KPauseDuration = 5000000; // microseconds

#define MAXLINELENGTH 200

////////////////////////////////////////////////////////////////////////////

PVAuthorEngineTest::PVAuthorEngineTest(FILE* aStdOut, int32 aFirstTest, int32 aLastTest,
                                       const char* aInputFileNameAudio, const char* aInputFileNameVideo, const char* aInputFileNameText, const char* aOutputFileName, AVTConfig aAVTConfig,
                                       PVAETestInputType aAudioInputType, PVAETestInputType aVideoInputType,  PVAETestInputType aTextInputType,
                                       const char* aComposerMimeType, const char* aAudioEncoderMimeType, const char* aVideoEncoderMimeType,  const char* aTextEncoderMimeType, uint32 aAuthoringTime):

        iCurrentTest(NULL),
        iFirstTest(aFirstTest),
        iLastTest(aLastTest),
        iNextTestCase(aFirstTest),
        iStdOut(aStdOut),

        iAudioInputType(aAudioInputType),
        iVideoInputType(aVideoInputType),
        iTextInputType(aTextInputType),
        iComposerMimeType(aComposerMimeType),
        iAudioEncoderMimeType(aAudioEncoderMimeType),
        iVideoEncoderMimeType(aVideoEncoderMimeType),
        iTextEncoderMimeType(aTextEncoderMimeType),
        iAVTConfig(aAVTConfig),
        iAuthoringTime(aAuthoringTime)

{
    iInputFileNameAudio = NULL;
    iInputFileNameVideo = NULL;
    iInputFileNameText = NULL;
    iOutputFileName = NULL;

    if (oscl_strlen(aInputFileNameAudio) != 0)
    {
        iInputFileNameAudio.set(aInputFileNameAudio, oscl_strlen(aInputFileNameAudio));
    }

    if (oscl_strlen(aInputFileNameVideo) != 0)
    {
        iInputFileNameVideo.set(aInputFileNameVideo, oscl_strlen(aInputFileNameVideo));
    }

    if (oscl_strlen(aInputFileNameText) != 0)
    {
        iInputFileNameText.set(aInputFileNameText, oscl_strlen(aInputFileNameText));
    }

    if (oscl_strlen(aOutputFileName) != 0)
    {
        iOutputFileName.set(aOutputFileName, oscl_strlen(aOutputFileName));
    }

}

////////////////////////////////////////////////////////////////////////////
PVAuthorEngineTest::~PVAuthorEngineTest()
{
}

////////////////////////////////////////////////////////////////////////////
void PVAuthorEngineTest::test()
{
    iTotalSuccess = iTotalFail = iTotalError = 0;

    while ((iNextTestCase <= iLastTest) || (iNextTestCase < Invalid_Test))
    {
        if (iCurrentTest)
        {
            delete iCurrentTest;
            iCurrentTest = NULL;

            // Shutdown PVLogger and scheduler before checking mem stats
            CleanupLoggerScheduler();

#if !(OSCL_BYPASS_MEMMGT)
            // Print out the memory usage results for this test case
            OsclAuditCB auditCB;
            OsclMemInit(auditCB);
            if (auditCB.pAudit)
            {
                MM_Stats_t* stats = auditCB.pAudit->MM_GetStats("");
                if (stats)
                {
                    fprintf(file, "  Mem stats: TotalAllocs(%d), TotalBytes(%d),\n             AllocFailures(%d), AllocLeak(%d)\n",
                            stats->totalNumAllocs - iTotalAlloc, stats->totalNumBytes - iTotalBytes, stats->numAllocFails - iAllocFails, stats->numAllocs - iNumAllocs);
                }
                else
                {
                    fprintf(file, "Retrieving memory statistics after running test case failed! Memory statistics result is not available.\n");
                }
            }
            else
            {
                fprintf(file, "Memory audit not available! Memory statistics result is not available.\n");
            }
#endif
        }

#if !(OSCL_BYPASS_MEMMGT)
        // Obtain the current mem stats before running the test case
        OsclAuditCB auditCB;
        OsclMemInit(auditCB);
        if (auditCB.pAudit)
        {
            MM_Stats_t* stats = auditCB.pAudit->MM_GetStats("");
            if (stats)
            {
                iTotalAlloc = stats->totalNumAllocs;
                iTotalBytes = stats->totalNumBytes;
                iAllocFails = stats->numAllocFails;
                iNumAllocs = stats->numAllocs;
            }
            else
            {
                fprintf(file, "Retrieving memory statistics before running test case failed! Memory statistics result would be invalid.\n");
            }
        }
        else
        {
            fprintf(file, "Memory audit not available! Memory statistics result would be invalid.\n");
        }
#endif
        if (iNextTestCase > iLastTest)
        {
            iNextTestCase = Invalid_Test;
        }
        else
        {
            //skip the placeholders and empty ranges.
            if ((iNextTestCase == K3GP_OUTPUT_TestEnd)
                    || (iNextTestCase == AMR_OUTPUT_TestEnd) || (iNextTestCase == AAC_OUTPUT_TestEnd)
                    || (iNextTestCase == CompressedLongetivityTestBegin) || (CompressedNormalTestEnd == iNextTestCase)
                    || (iNextTestCase == KCompressed_Errorhandling_TestBegin))
            {
                fprintf(file, "\nPlace Holder Not actual testcase %d: ", iNextTestCase);
                iNextTestCase++;//go to next test
            }
            if ((iNextTestCase >= CompressedNormalTestEnd && iNextTestCase <= CompressedLongetivityTestBegin) && (iLastTest >= CompressedLongetivityTestBegin))
            {
                iNextTestCase = CompressedLongetivityTestBegin;
                iNextTestCase++;
            }

            if ((iNextTestCase > CompressedNormalTestEnd) && (iLastTest < CompressedLongetivityTestBegin))
            {
                iNextTestCase = Invalid_Test;
            }//stop at last test of selected range.
            else if ((Compressed_LongetivityTestEnd == iNextTestCase) || (KCompressed_Errorhandling_TestEnd == iNextTestCase))
            {
                fprintf(file, "\nPlace Holder Not actual testcase %d: ", iNextTestCase);
                iNextTestCase = Invalid_Test;
            }
            else
            {
                fprintf(file, "\nStarting Test %d: ", iNextTestCase);
                InitLoggerScheduler();
            }
        }

        RunTestCases();
        if (iCurrentTest)
        {
            // Setup Scheduler
            OsclExecScheduler *sched = OsclExecScheduler::Current();
            if (sched)
            {
                uint32 currticks  = 0;
                currticks = OsclTickCount::TickCount();
                uint32 starttime = OsclTickCount::TicksToMsec(currticks);

                iCurrentTest->StartTest();
#if USE_NATIVE_SCHEDULER
                // Have PV scheduler use the scheduler native to the system
                sched->StartNativeScheduler();
#else
                int32 err;
                OSCL_TRY(err, sched->StartScheduler(););
#endif
                currticks = OsclTickCount::TickCount();
                uint32 endtime = OsclTickCount::TicksToMsec(currticks);
                fprintf(file, "  Time taken by the test:  %d\n", (endtime - starttime));

            }
            else
            {
                fprintf(file, "ERROR! Scheduler is not available. Test case could not run.");
                iNextTestCase++;
            }
        }
        else
        {
            iNextTestCase++;

            if (iNextTestCase < Invalid_Test)
            {
                CleanupLoggerScheduler();
            }
        }
    }//while iNextTest loop
}

////////////////////////////////////////////////////////////////////////////
// Normal test set
void PVAuthorEngineTest::RunTestCases()
{
    // Setup the standard test case parameters based on current unit test settings
    PVAuthorAsyncTestParam testparam;
    testparam.iObserver = this;
    testparam.iTestCase = this;
    testparam.iTestCaseNum = iNextTestCase;
    testparam.iStdOut = iStdOut;
    switch (iNextTestCase)
    {
        case AMR_Input_AOnly_3gpTest:
            fprintf(iStdOut, "AMR to A-Only .3gp Test\n");
            iCurrentTest = new pvauthor_async_test_miscellaneous(testparam, (const char*)iInputFileNameAudio.get_cstr(), (const char*)iInputFileNameVideo.get_cstr(), (const char*)iInputFileNameText.get_cstr(),
                    (const char*)iOutputFileName.get_cstr(), iAudioInputType, iVideoInputType, iTextInputType,
                    iComposerMimeType.get_cstr(), iAudioEncoderMimeType.get_cstr(), iVideoEncoderMimeType.get_cstr(),  iTextEncoderMimeType.get_cstr(), iAVTConfig, false, KTestDuration);
            break;
        case H263_Input_VOnly_3gpTest:
            fprintf(iStdOut, "H263 to V-Only .3gp Test\n");
            iCurrentTest = new pvauthor_async_test_miscellaneous(testparam, (const char*)iInputFileNameAudio.get_cstr(), (const char*)iInputFileNameVideo.get_cstr(), (const char*)iInputFileNameText.get_cstr(),
                    (const char*)iOutputFileName.get_cstr(), iAudioInputType, iVideoInputType, iTextInputType,
                    iComposerMimeType.get_cstr(), iAudioEncoderMimeType.get_cstr(), iVideoEncoderMimeType.get_cstr(),  iTextEncoderMimeType.get_cstr(), iAVTConfig, false, KTestDuration);
            break;
        case H264_AMR_Input_AV_3gpTest:
            fprintf(iStdOut, "H264 & AMR to AV .3gp Test\n");
            iCurrentTest = new pvauthor_async_test_miscellaneous(testparam, (const char*)iInputFileNameAudio.get_cstr(), (const char*)iInputFileNameVideo.get_cstr(), (const char*)iInputFileNameText.get_cstr(),
                    (const char*)iOutputFileName.get_cstr(), iAudioInputType, iVideoInputType, iTextInputType,
                    iComposerMimeType.get_cstr(), iAudioEncoderMimeType.get_cstr(), iVideoEncoderMimeType.get_cstr(),  iTextEncoderMimeType.get_cstr(), iAVTConfig, false, KTestDuration);
            break;
        case AMR_YUV_Input_AV_3gpTest:
            fprintf(iStdOut, "AMR & YUV to AV .3gp Test\n");
            iCurrentTest = new pvauthor_async_test_miscellaneous(testparam, (const char*)iInputFileNameAudio.get_cstr(), (const char*)iInputFileNameVideo.get_cstr(), (const char*)iInputFileNameText.get_cstr(),
                    (const char*)iOutputFileName.get_cstr(), iAudioInputType, iVideoInputType, iTextInputType,
                    iComposerMimeType.get_cstr(), iAudioEncoderMimeType.get_cstr(), iVideoEncoderMimeType.get_cstr(),  iTextEncoderMimeType.get_cstr(), iAVTConfig, false, KTestDuration);
            break;
        case AMR_H263_Input_AV_3gpTest:
            fprintf(iStdOut, "AMR & H263 to AV .3gp Test\n");
            iCurrentTest = new pvauthor_async_test_miscellaneous(testparam, (const char*)iInputFileNameAudio.get_cstr(), (const char*)iInputFileNameVideo.get_cstr(), (const char*)iInputFileNameText.get_cstr(),
                    (const char*)iOutputFileName.get_cstr(), iAudioInputType, iVideoInputType, iTextInputType,
                    iComposerMimeType.get_cstr(), iAudioEncoderMimeType.get_cstr(), iVideoEncoderMimeType.get_cstr(),  iTextEncoderMimeType.get_cstr(), iAVTConfig, false, KTestDuration);
            break;
        case AMR_YUV_Input_AV_M4V_AMR_Output_3gpTest:
            fprintf(iStdOut, "AMR & YUV to AV using M4V Encoder .3gp Test\n");
            iCurrentTest = new pvauthor_async_test_miscellaneous(testparam, (const char*)iInputFileNameAudio.get_cstr(), (const char*)iInputFileNameVideo.get_cstr(), (const char*)iInputFileNameText.get_cstr(),
                    (const char*)iOutputFileName.get_cstr(), iAudioInputType, iVideoInputType, iTextInputType,
                    iComposerMimeType.get_cstr(), iAudioEncoderMimeType.get_cstr(), iVideoEncoderMimeType.get_cstr(),  iTextEncoderMimeType.get_cstr(), iAVTConfig, false, KTestDuration);
            break;
        case AMR_FOutput_Test:
            fprintf(iStdOut, "AMR Input to .amr Test\n");
            iCurrentTest = new pvauthor_async_test_miscellaneous(testparam, (const char*)iInputFileNameAudio.get_cstr(), (const char*)iInputFileNameVideo.get_cstr(), (const char*)iInputFileNameText.get_cstr(),
                    (const char*)iOutputFileName.get_cstr(), iAudioInputType, iVideoInputType, iTextInputType,
                    iComposerMimeType.get_cstr(), iAudioEncoderMimeType.get_cstr(), iVideoEncoderMimeType.get_cstr(),  iTextEncoderMimeType.get_cstr(), iAVTConfig, false, KTestDuration);
            break;
        case AACADIF_FOutput_Test:
            fprintf(iStdOut, "AAC-ADIF Input to .aac Test\n");
            iCurrentTest = new pvauthor_async_test_miscellaneous(testparam, (const char*)iInputFileNameAudio.get_cstr(), (const char*)iInputFileNameVideo.get_cstr(), (const char*)iInputFileNameText.get_cstr(),
                    (const char*)iOutputFileName.get_cstr(), iAudioInputType, iVideoInputType, iTextInputType,
                    iComposerMimeType.get_cstr(), iAudioEncoderMimeType.get_cstr(), iVideoEncoderMimeType.get_cstr(),  iTextEncoderMimeType.get_cstr(), iAVTConfig, false, KTestDuration);
            break;
        case AACADTS_FOutput_Test:
            fprintf(iStdOut, "AAC-ADTS Input to .aac Test\n");
            iCurrentTest = new pvauthor_async_test_miscellaneous(testparam, (const char*)iInputFileNameAudio.get_cstr(), (const char*)iInputFileNameVideo.get_cstr(), (const char*)iInputFileNameText.get_cstr(),
                    (const char*)iOutputFileName.get_cstr(), iAudioInputType, iVideoInputType, iTextInputType,
                    iComposerMimeType.get_cstr(), iAudioEncoderMimeType.get_cstr(), iVideoEncoderMimeType.get_cstr(),  iTextEncoderMimeType.get_cstr(), iAVTConfig, false, KTestDuration);
            break;
        case AMRWB_Input_AOnly_3gpTest:
            fprintf(iStdOut, "AMR-WB to A-Only .3gp Test\n");
            iCurrentTest = new pvauthor_async_test_miscellaneous(testparam, (const char*)iInputFileNameAudio.get_cstr(), (const char*)iInputFileNameVideo.get_cstr(), (const char*)iInputFileNameText.get_cstr(),
                    (const char*)iOutputFileName.get_cstr(), iAudioInputType, iVideoInputType, iTextInputType,
                    iComposerMimeType.get_cstr(), iAudioEncoderMimeType.get_cstr(), iVideoEncoderMimeType.get_cstr(),  iTextEncoderMimeType.get_cstr(), iAVTConfig, false, KTestDuration);
            break;
        case AMRWB_FOutput_Test:
            fprintf(iStdOut, "AMR-WB Input to .awb Test\n");
            iCurrentTest = new pvauthor_async_test_miscellaneous(testparam, (const char*)iInputFileNameAudio.get_cstr(), (const char*)iInputFileNameVideo.get_cstr(), (const char*)iInputFileNameText.get_cstr(),
                    (const char*)iOutputFileName.get_cstr(), iAudioInputType, iVideoInputType, iTextInputType,
                    iComposerMimeType.get_cstr(), iAudioEncoderMimeType.get_cstr(), iVideoEncoderMimeType.get_cstr(),  iTextEncoderMimeType.get_cstr(), iAVTConfig, false, KTestDuration);
            break;
        case ErrorHandling_WrongTextInputFileNameTest:
            fprintf(iStdOut, "ErrorHandling_WrongTextInputFileNameTest Test\n");
            iCurrentTest = new pvauthor_async_compressed_test_errorhandling(testparam, (const char*)iInputFileNameAudio.get_cstr(), (const char*)iInputFileNameVideo.get_cstr(), (const char*)iInputFileNameText.get_cstr(),
                    (const char*)iOutputFileName.get_cstr(), iAudioInputType, iVideoInputType, iTextInputType,
                    iComposerMimeType.get_cstr(), iAudioEncoderMimeType.get_cstr(), iVideoEncoderMimeType.get_cstr(),  iTextEncoderMimeType.get_cstr(), iAVTConfig, false, KTestDuration, ERROR_NOSTATE);
            break;
        case ErrorHandling_WrongOutputPathTest:
            fprintf(iStdOut, "ErrorHandling_WrongOutputPathTest Test\n");
            iCurrentTest = new pvauthor_async_compressed_test_errorhandling(testparam, (const char*)iInputFileNameAudio.get_cstr(), (const char*)iInputFileNameVideo.get_cstr(), (const char*)iInputFileNameText.get_cstr(),
                    (const char*)iOutputFileName.get_cstr(), iAudioInputType, iVideoInputType, iTextInputType,
                    iComposerMimeType.get_cstr(), iAudioEncoderMimeType.get_cstr(), iVideoEncoderMimeType.get_cstr(),  iTextEncoderMimeType.get_cstr(), iAVTConfig, false, KTestDuration, ERROR_NOSTATE);
            break;

        case ErrorHandling_MediaInputNodeStartFailed:
            fprintf(iStdOut, "ErrorHandling_MediaInputNodeStartFailed Test\n");
#ifndef _TEST_AE_ERROR_HANDLING
            fprintf(iStdOut, "test not implemented\n");
            iCurrentTest = NULL;
            break;
#else
            iCurrentTest = new pvauthor_async_compressed_test_errorhandling(testparam, (const char*)iInputFileNameAudio.get_cstr(), (const char*)iInputFileNameVideo.get_cstr(), (const char*)iInputFileNameText.get_cstr(),
                    (const char*)iOutputFileName.get_cstr(), iAudioInputType, iVideoInputType, iTextInputType,
                    iComposerMimeType.get_cstr(), iAudioEncoderMimeType.get_cstr(), iVideoEncoderMimeType.get_cstr(),  iTextEncoderMimeType.get_cstr(), iAVTConfig, false, KTestDuration, ERROR_MEDIAINPUTNODE_ADDDATASOURCE_START);
            break;
#endif
        case TEXT_Input_TOnly_3gpTest:
            fprintf(iStdOut, "TEXT Only .3gp Test\n");
            iCurrentTest = new pvauthor_async_test_miscellaneous(testparam, (const char*)iInputFileNameAudio.get_cstr(), (const char*)iInputFileNameVideo.get_cstr(), (const char*)iInputFileNameText.get_cstr(),
                    (const char*)iOutputFileName.get_cstr(), iAudioInputType, iVideoInputType, iTextInputType,
                    iComposerMimeType.get_cstr(), iAudioEncoderMimeType.get_cstr(), iVideoEncoderMimeType.get_cstr(),  iTextEncoderMimeType.get_cstr(), iAVTConfig, false, KTestDuration);
            break;
        case AMR_TEXT_Input_AT_3gpTest:
            fprintf(iStdOut, "AMR/TEXT .3gp Test\n");
            iCurrentTest = new pvauthor_async_test_miscellaneous(testparam, (const char*)iInputFileNameAudio.get_cstr(), (const char*)iInputFileNameVideo.get_cstr(), (const char*)iInputFileNameText.get_cstr(),
                    (const char*)iOutputFileName.get_cstr(), iAudioInputType, iVideoInputType, iTextInputType,
                    iComposerMimeType.get_cstr(), iAudioEncoderMimeType.get_cstr(), iVideoEncoderMimeType.get_cstr(),  iTextEncoderMimeType.get_cstr(), iAVTConfig, false, KTestDuration);
            break;
        case YUV_TEXT_Input_VT_3gpTest:
            fprintf(iStdOut, "YUV/TEXT .3gp Test\n");
            iCurrentTest = new pvauthor_async_test_miscellaneous(testparam, (const char*)iInputFileNameAudio.get_cstr(), (const char*)iInputFileNameVideo.get_cstr(), (const char*)iInputFileNameText.get_cstr(),
                    (const char*)iOutputFileName.get_cstr(), iAudioInputType, iVideoInputType, iTextInputType,
                    iComposerMimeType.get_cstr(), iAudioEncoderMimeType.get_cstr(), iVideoEncoderMimeType.get_cstr(),  iTextEncoderMimeType.get_cstr(), iAVTConfig, false, KTestDuration);
            break;
        case AMR_YUV_TEXT_Input_AVT_Mp4Test:
            fprintf(iStdOut, "YUV/AMR/TEXT .3gp Test\n");
            iCurrentTest = new pvauthor_async_test_miscellaneous(testparam, (const char*)iInputFileNameAudio.get_cstr(), (const char*)iInputFileNameVideo.get_cstr(), (const char*)iInputFileNameText.get_cstr(),
                    (const char*)iOutputFileName.get_cstr(), iAudioInputType, iVideoInputType, iTextInputType,
                    iComposerMimeType.get_cstr(), iAudioEncoderMimeType.get_cstr(), iVideoEncoderMimeType.get_cstr(),  iTextEncoderMimeType.get_cstr(), iAVTConfig, false, KTestDuration);
            break;

            // longetivity test

        case TEXT_Input_TOnly_3gp_LongetivityTest:
            fprintf(iStdOut, "TEXT_Input_TOnly_3gp_LongetivityTest test \n");
            iCurrentTest = new pvauthor_async_test_miscellaneous(testparam, (const char*)iInputFileNameAudio.get_cstr(), (const char*)iInputFileNameVideo.get_cstr(), (const char*)iInputFileNameText.get_cstr(),
                    (const char*)iOutputFileName.get_cstr(), iAudioInputType, iVideoInputType, iTextInputType,
                    iComposerMimeType.get_cstr(), iAudioEncoderMimeType.get_cstr(), iVideoEncoderMimeType.get_cstr(),  iTextEncoderMimeType.get_cstr(), iAVTConfig, false, iAuthoringTime);
            break;

        case AMR_TEXT_Input_AT_3gp_LongetivityTest:
            fprintf(iStdOut, "AMR_TEXT_Input_AT_3gp_LongetivityTest test \n");
            iCurrentTest = new pvauthor_async_test_miscellaneous(testparam, (const char*)iInputFileNameAudio.get_cstr(), (const char*)iInputFileNameVideo.get_cstr(), (const char*)iInputFileNameText.get_cstr(),
                    (const char*)iOutputFileName.get_cstr(), iAudioInputType, iVideoInputType, iTextInputType,
                    iComposerMimeType.get_cstr(), iAudioEncoderMimeType.get_cstr(), iVideoEncoderMimeType.get_cstr(),  iTextEncoderMimeType.get_cstr(), iAVTConfig, false, iAuthoringTime);
            break;

        case YUV_TEXT_Input_VT_3gp_LongetivityTest:
            fprintf(iStdOut, "YUV_TEXT_Input_VT_3gp_LongetivityTest test \n");
            iCurrentTest = new pvauthor_async_test_miscellaneous(testparam, (const char*)iInputFileNameAudio.get_cstr(), (const char*)iInputFileNameVideo.get_cstr(), (const char*)iInputFileNameText.get_cstr(),
                    (const char*)iOutputFileName.get_cstr(), iAudioInputType, iVideoInputType, iTextInputType,
                    iComposerMimeType.get_cstr(), iAudioEncoderMimeType.get_cstr(), iVideoEncoderMimeType.get_cstr(),  iTextEncoderMimeType.get_cstr(), iAVTConfig, false, iAuthoringTime);
            break;

        case AMR_YUV_TEXT_Input_AVT_3gp_LongetivityTest:
            fprintf(iStdOut, "AMR_YUV_TEXT_Input_AVT_3gp_LongetivityTest test \n");
            iCurrentTest = new pvauthor_async_test_miscellaneous(testparam, (const char*)iInputFileNameAudio.get_cstr(), (const char*)iInputFileNameVideo.get_cstr(), (const char*)iInputFileNameText.get_cstr(),
                    (const char*)iOutputFileName.get_cstr(), iAudioInputType, iVideoInputType, iTextInputType,
                    iComposerMimeType.get_cstr(), iAudioEncoderMimeType.get_cstr(), iVideoEncoderMimeType.get_cstr(),  iTextEncoderMimeType.get_cstr(), iAVTConfig, false, iAuthoringTime);
            break;

        case AMR_FileOutput_Test_UsingExternalFileHandle:
            fprintf(iStdOut, "AMR Input to .amr using external file handle Test\n");
            iCurrentTest = new pvauthor_async_test_miscellaneous(testparam, (const char*)iInputFileNameAudio.get_cstr(), (const char*)iInputFileNameVideo.get_cstr(), (const char*)iInputFileNameText.get_cstr(),
                    (const char*)iOutputFileName.get_cstr(), iAudioInputType, iVideoInputType, iTextInputType,
                    iComposerMimeType.get_cstr(), iAudioEncoderMimeType.get_cstr(), iVideoEncoderMimeType.get_cstr(),  iTextEncoderMimeType.get_cstr(), iAVTConfig, false, KTestDuration, true);
            break;

        default:
            iCurrentTest = NULL;
            break;
    }
}
void PVAuthorEngineTest::CompleteTest(test_case &aTC)
{
    // Print out the result for this test case
    const test_result the_result = aTC.last_result();
    fprintf(file, "  Successes %d, Failures %d\n"
            , the_result.success_count() - iTotalSuccess, the_result.failures().size() - iTotalFail);
    iTotalSuccess = the_result.success_count();
    iTotalFail = the_result.failures().size();
    iTotalError = the_result.errors().size();

    // Go to next test
    ++iNextTestCase;

    // Stop the scheduler
    OsclExecScheduler *sched = OsclExecScheduler::Current();
    if (sched)
    {
        sched->StopScheduler();
    }
}
//This functions finds the name of AudioConfigFile from command line specified with switch -audioconfigfile <filename>
//Returns true if filename found, false otherwise
bool FindAudioConfigFile(cmd_line* aCommandLine, OSCL_HeapString<OsclMemAllocator> & aFileNameAudioConfig, FILE *aFile)
{
    OSCL_UNUSED_ARG(aFile);
    int iFileArgument = 0;
    bool iFileFound = false;
    bool cmdline_iswchar = aCommandLine->is_wchar();

    int count = aCommandLine->get_count();

    // Searcgh for the "-audioconfigfile" argument
    // go through the each argument

    for (int iFileSearch = 0;iFileSearch < count;iFileSearch++)
    {
        char argstr[128];
        // Convert to UTF8 if necessary
        if (cmdline_iswchar)
        {
            oscl_wchar* argwstr = NULL;
            aCommandLine->get_arg(iFileSearch, argwstr);
            oscl_UnicodeToUTF8(argwstr, oscl_strlen(argwstr), argstr, 128);
            argstr[127] = '\0';
        }
        else
        {
            char* tmpstr = NULL;
            aCommandLine->get_arg(iFileSearch, tmpstr);
            int32 tmpstrlen = oscl_strlen(tmpstr) + 1;
            if (tmpstrlen > 128)
            {
                tmpstrlen = 128;
            }
            oscl_strncpy(argstr, tmpstr, tmpstrlen);
            argstr[tmpstrlen-1] = '\0';
        }

        // Do the string compare
        if (oscl_strcmp(argstr, "-audioconfigfile") == 0)
        {
            iFileFound = true;
            iFileArgument = ++iFileSearch;
            break;
        }
    }

    if (iFileFound)
    {
        // Convert to UTF8 if necessary
        if (cmdline_iswchar)
        {
            oscl_wchar* cmd;
            aCommandLine->get_arg(iFileArgument, cmd);
            char tmpstr[256];
            oscl_UnicodeToUTF8(cmd, oscl_strlen(cmd), tmpstr, 256);
            tmpstr[255] = '\0';
            aFileNameAudioConfig = tmpstr;
        }
        else
        {
            char* cmdlinefilename = NULL;
            aCommandLine->get_arg(iFileArgument, cmdlinefilename);
            aFileNameAudioConfig = cmdlinefilename;
        }
        return true;
    }
    return false;
}


//This functions finds the name of VideoConfigFile from command line specified with switch -videoconfigfile <filename>
//Returns true if filename found, false otherwise
bool FindVideoConfigFile(cmd_line* aCommandLine, OSCL_HeapString<OsclMemAllocator>& aFileNameVideoConfig, FILE *aFile)
{
    OSCL_UNUSED_ARG(aFile);
    int	iFileArgument = 0;
    bool iFileFound = false;
    bool cmdline_iswchar = aCommandLine->is_wchar();

    int count = aCommandLine->get_count();

    // Search for the "-videoconfigfile" argument
    // go through the each argument

    for (int iFileSearch = 0; iFileSearch < count; iFileSearch++)
    {
        char argstr[128];
        // Convert to UTF8 if necessary
        if (cmdline_iswchar)
        {
            oscl_wchar* argwstr = NULL;
            aCommandLine->get_arg(iFileSearch, argwstr);
            oscl_UnicodeToUTF8(argwstr, oscl_strlen(argwstr), argstr, 128);
            argstr[127] = '\0';
        }
        else
        {
            char* tmpstr = NULL;
            aCommandLine->get_arg(iFileSearch, tmpstr);
            int32 tmpstrlen = oscl_strlen(tmpstr) + 1;
            if (tmpstrlen > 128)
            {
                tmpstrlen = 128;
            }
            oscl_strncpy(argstr, tmpstr, tmpstrlen);
            argstr[tmpstrlen-1] = '\0';
        }

        // Do the string compare
        if (oscl_strcmp(argstr, "-videoconfigfile") == 0)
        {
            iFileFound = true;
            iFileArgument = ++iFileSearch;
            break;
        }
    }

    if (iFileFound)
    {
        // Convert to UTF8 if necessary
        if (cmdline_iswchar)
        {
            oscl_wchar* cmd;
            aCommandLine->get_arg(iFileArgument, cmd);
            char tmpstr[256];
            oscl_UnicodeToUTF8(cmd, oscl_strlen(cmd), tmpstr, 256);
            tmpstr[255] = '\0';
            aFileNameVideoConfig = tmpstr;
        }
        else
        {
            char* cmdlinefilename = NULL;
            aCommandLine->get_arg(iFileArgument, cmdlinefilename);
            aFileNameVideoConfig = cmdlinefilename;
        }
        return true;
    }
    return false;
}

//This functions finds the name of AVIConfigFile from command line specified with switch -aviconfigfile <filename>
//Returns true if filename found, false otherwise
bool FindAVIConfigFile(cmd_line* aCommandLine, OSCL_HeapString<OsclMemAllocator>& aFileNameAVIConfig, FILE *aFile)
{
    OSCL_UNUSED_ARG(aFile);
    int	iFileArgument = 0;
    bool iFileFound = false;
    bool cmdline_iswchar = aCommandLine->is_wchar();

    int count = aCommandLine->get_count();

    // Search for the "-aviconfigfile" argument
    // go through the each argument

    for (int iFileSearch = 0; iFileSearch < count; iFileSearch++)
    {
        char argstr[128];
        // Convert to UTF8 if necessary
        if (cmdline_iswchar)
        {
            oscl_wchar* argwstr = NULL;
            aCommandLine->get_arg(iFileSearch, argwstr);
            oscl_UnicodeToUTF8(argwstr, oscl_strlen(argwstr), argstr, 128);
            argstr[127] = '\0';
        }
        else
        {
            char* tmpstr = NULL;
            aCommandLine->get_arg(iFileSearch, tmpstr);
            int32 tmpstrlen = oscl_strlen(tmpstr) + 1;
            if (tmpstrlen > 128)
            {
                tmpstrlen = 128;
            }
            oscl_strncpy(argstr, tmpstr, tmpstrlen);
            argstr[tmpstrlen-1] = '\0';
        }

        // Do the string compare
        if (oscl_strcmp(argstr, "-aviconfigfile") == 0)
        {
            iFileFound = true;
            iFileArgument = ++iFileSearch;
            break;
        }
    }

    if (iFileFound)
    {
        // Convert to UTF8 if necessary
        if (cmdline_iswchar)
        {
            oscl_wchar* cmd;
            aCommandLine->get_arg(iFileArgument, cmd);
            char tmpstr[256];
            oscl_UnicodeToUTF8(cmd, oscl_strlen(cmd), tmpstr, 256);
            tmpstr[255] = '\0';
            aFileNameAVIConfig = tmpstr;
        }
        else
        {
            char* cmdlinefilename = NULL;
            aCommandLine->get_arg(iFileArgument, cmdlinefilename);
            aFileNameAVIConfig = cmdlinefilename;
        }
        return true;
    }
    return false;
}

//This functions finds the name of audio input file from command line specified with switch -audio <filename>
//Sets the name of audio input file in reference parameter
void FindAudioSourceFile(cmd_line* aCommandLine, OSCL_HeapString<OsclMemAllocator>& aFileNameAudioInfo, FILE* aFile)
{
    int iFileArgument = 0;
    bool iFileFound = false;
    bool cmdline_iswchar = aCommandLine->is_wchar();

    int count = aCommandLine->get_count();

    // Search for the "-audio" argument
    // Go through each argument
    for (int iFileSearch = 0; iFileSearch < count; iFileSearch++)
    {
        char argstr[128];
        // Convert to UTF8 if necessary
        if (cmdline_iswchar)
        {
            oscl_wchar* argwstr = NULL;
            aCommandLine->get_arg(iFileSearch, argwstr);
            oscl_UnicodeToUTF8(argwstr, oscl_strlen(argwstr), argstr, 128);
            argstr[127] = '\0';
        }
        else
        {
            char* tmpstr = NULL;
            aCommandLine->get_arg(iFileSearch, tmpstr);
            int32 tmpstrlen = oscl_strlen(tmpstr) + 1;
            if (tmpstrlen > 128)
            {
                tmpstrlen = 128;
            }
            oscl_strncpy(argstr, tmpstr, tmpstrlen);
            argstr[tmpstrlen-1] = '\0';
        }

        // Do the string compare
        if (0 == oscl_strcmp(argstr, "-help"))
        {
            fprintf(aFile, "Source specification option for audio.:\n");
            fprintf(aFile, "  -audio sourcename\n");
            fprintf(aFile, "   Specify the source filename or URL to use for test cases which\n");
            fprintf(aFile, "   allow user-specified source name. The unit test determines the\n");
            fprintf(aFile, "   source format type using extension or URL header.\n\n");
        }
        else if (0 == oscl_strcmp(argstr, "-audio"))
        {
            iFileFound = true;
            iFileArgument = ++iFileSearch;
            break;
        }
    }

    if (iFileFound)
    {
        // Convert to UTF8 if necessary
        if (cmdline_iswchar)
        {
            oscl_wchar* cmd;
            aCommandLine->get_arg(iFileArgument, cmd);
            char tmpstr[256];
            oscl_UnicodeToUTF8(cmd, oscl_strlen(cmd), tmpstr, 256);
            tmpstr[255] = '\0';
            aFileNameAudioInfo = tmpstr;
        }
        else
        {
            char* cmdlinefilename = NULL;
            aCommandLine->get_arg(iFileArgument, cmdlinefilename);
            aFileNameAudioInfo = cmdlinefilename;
        }
    }
    else //if the -audio tag is missing from cmd line
    {
        aFileNameAudioInfo = NULL;
    }
}



//This functions finds the name of video input file from command line specified with switch -video <filename>
//Sets the name of video input file in reference parameter
void FindVideoSourceFile(cmd_line* aCommandLine, OSCL_HeapString<OsclMemAllocator>& aFileNameVideoInfo, FILE* aFile)
{
    int iFileArgument = 0;
    bool iFileFound = false;
    bool cmdline_iswchar = aCommandLine->is_wchar();

    int count = aCommandLine->get_count();

    // Search for the "-video" argument
    // Go through each argument
    for (int iFileSearch = 0; iFileSearch < count; iFileSearch++)
    {
        char argstr[128];
        // Convert to UTF8 if necessary
        if (cmdline_iswchar)
        {
            oscl_wchar* argwstr = NULL;
            aCommandLine->get_arg(iFileSearch, argwstr);
            oscl_UnicodeToUTF8(argwstr, oscl_strlen(argwstr), argstr, 128);
            argstr[127] = '\0';
        }
        else
        {
            char* tmpstr = NULL;
            aCommandLine->get_arg(iFileSearch, tmpstr);
            int32 tmpstrlen = oscl_strlen(tmpstr) + 1;
            if (tmpstrlen > 128)
            {
                tmpstrlen = 128;
            }
            oscl_strncpy(argstr, tmpstr, tmpstrlen);
            argstr[tmpstrlen-1] = '\0';
        }

        // Do the string compare
        if (0 == oscl_strcmp(argstr, "-help"))
        {
            fprintf(aFile, "Source specification option for video.:\n");
            fprintf(aFile, "  -video sourcename\n");
            fprintf(aFile, "   Specify the source filename or URL to use for test cases which\n");
            fprintf(aFile, "   allow user-specified source name. The unit test determines the\n");
            fprintf(aFile, "   source format type using extension or URL header.\n\n");
        }
        else if (0 == oscl_strcmp(argstr, "-video"))
        {
            iFileFound = true;
            iFileArgument = ++iFileSearch;
            break;
        }
    }

    if (iFileFound)
    {
        // Convert to UTF8 if necessary
        if (cmdline_iswchar)
        {
            oscl_wchar* cmd;
            aCommandLine->get_arg(iFileArgument, cmd);
            char tmpstr[256];
            oscl_UnicodeToUTF8(cmd, oscl_strlen(cmd), tmpstr, 256);
            tmpstr[255] = '\0';
            aFileNameVideoInfo = tmpstr;
        }
        else
        {
            char* cmdlinefilename = NULL;
            aCommandLine->get_arg(iFileArgument, cmdlinefilename);
            aFileNameVideoInfo = cmdlinefilename;
        }
    }
    else //if the -video tag is missing from cmd line
    {
        aFileNameVideoInfo = NULL;
    }
}

//This functions finds the name of text input file from command line specified with switch -text <filename>
//Sets the name of text input file in reference parameter
void FindTextSourceFile(cmd_line* aCommandLine, OSCL_HeapString<OsclMemAllocator>& aFileNameTextInfo, OSCL_HeapString<OsclMemAllocator>& aFileTextLogInfo, OSCL_HeapString<OsclMemAllocator>& aFileTextConfigInfo, FILE* aFile)
{
    int iFileArgument = 0;
    int iLogFileArgument = 0;
    int iTextFileArgument = 0;
    bool iFileFound = false;
    bool iLogFileFound = false;
    bool iTextFileFound = false;
    bool cmdline_iswchar = aCommandLine->is_wchar();

    int count = aCommandLine->get_count();

    // Search for the "-text" argument
    // Go through each argument
    for (int iFileSearch = 0; iFileSearch < count; iFileSearch++)
    {
        char argstr[128];
        // Convert to UTF8 if necessary
        if (cmdline_iswchar)
        {
            oscl_wchar* argwstr = NULL;
            aCommandLine->get_arg(iFileSearch, argwstr);
            oscl_UnicodeToUTF8(argwstr, oscl_strlen(argwstr), argstr, 128);
            argstr[127] = '\0';
        }
        else
        {
            char* tmpstr = NULL;
            aCommandLine->get_arg(iFileSearch, tmpstr);
            int32 tmpstrlen = oscl_strlen(tmpstr) + 1;
            if (tmpstrlen > 128)
            {
                tmpstrlen = 128;
            }
            oscl_strncpy(argstr, tmpstr, tmpstrlen);
            argstr[tmpstrlen-1] = '\0';
        }

        // Do the string compare
        if (0 == oscl_strcmp(argstr, "-help"))
        {
            fprintf(aFile, "Source specification option for text.:\n");
            fprintf(aFile, "  -text sourcename\n");
            fprintf(aFile, "   Specify the source filename or URL to use for test cases which\n");
            fprintf(aFile, "   allow user-specified source name. The unit test determines the\n");
            fprintf(aFile, "   source format type using extension or URL header.\n\n");
        }
        else if (0 == oscl_strcmp(argstr, "-text"))
        {
            iFileFound = true;
            iFileArgument = ++iFileSearch;
        }
        else if (iFileFound && (0 == oscl_strcmp(argstr, "-textlogfile")))
        {
            iLogFileFound = true;
            iLogFileArgument = ++iFileSearch;
        }
        else if (iLogFileFound && iFileFound && (0 == oscl_strcmp(argstr, "-textconfigfile")))
        {
            iTextFileFound = true;
            iTextFileArgument = ++iFileSearch;
            break;
        }
    }

    if (iFileFound)
    {
        // Convert to UTF8 if necessary
        if (cmdline_iswchar)
        {
            oscl_wchar* cmd;
            aCommandLine->get_arg(iFileArgument, cmd);
            char tmpstr[256];
            oscl_UnicodeToUTF8(cmd, oscl_strlen(cmd), tmpstr, 256);
            tmpstr[255] = '\0';
            aFileNameTextInfo = tmpstr;
        }
        else
        {
            char* cmdlinefilename = NULL;
            aCommandLine->get_arg(iFileArgument, cmdlinefilename);
            aFileNameTextInfo = cmdlinefilename;
        }
    }
    else //if the -text tag is missing from cmd line
    {
        aFileNameTextInfo = NULL;
    }

    if (iFileFound && iLogFileFound)
    {
        // Convert to UTF8 if necessary
        if (cmdline_iswchar)
        {
            oscl_wchar* cmd;
            aCommandLine->get_arg(iLogFileArgument, cmd);
            char tmpstr[256];
            oscl_UnicodeToUTF8(cmd, oscl_strlen(cmd), tmpstr, 256);
            tmpstr[255] = '\0';
            aFileTextLogInfo = tmpstr;
        }
        else
        {
            char* cmdlinefilename = NULL;
            aCommandLine->get_arg(iLogFileArgument, cmdlinefilename);
            aFileTextLogInfo = cmdlinefilename;
        }
    }
    else
    {
        aFileTextLogInfo = NULL;
    }

    if (iFileFound && iLogFileFound && iTextFileFound)
    {
        // Convert to UTF8 if necessary
        if (cmdline_iswchar)
        {
            oscl_wchar* cmd;
            aCommandLine->get_arg(iTextFileArgument, cmd);
            char tmpstr[256];
            oscl_UnicodeToUTF8(cmd, oscl_strlen(cmd), tmpstr, 256);
            tmpstr[255] = '\0';
            aFileTextConfigInfo = tmpstr;
        }
        else
        {
            char* cmdlinefilename = NULL;
            aCommandLine->get_arg(iTextFileArgument, cmdlinefilename);
            aFileTextConfigInfo = cmdlinefilename;
        }
    }
    else
    {
        aFileTextConfigInfo = NULL;
    }
}
//This functions finds the name of output input file from command line specified with switch -output <filename>
//Sets the name of output file in reference parameter
void FindOutputFile(cmd_line* aCommandLine,	OSCL_HeapString<OsclMemAllocator>& aOutputFileNameInfo, FILE* aFile)
{
    int iFileArgument = 0;
    bool iFileFound = false;
    bool cmdline_iswchar = aCommandLine->is_wchar();

    int count = aCommandLine->get_count();

    // Search for the "-output" argument
    // Go through each argument
    for (int iFileSearch = 0; iFileSearch < count; iFileSearch++)
    {
        char argstr[128];
        // Convert to UTF8 if necessary
        if (cmdline_iswchar)
        {
            oscl_wchar* argwstr = NULL;
            aCommandLine->get_arg(iFileSearch, argwstr);
            oscl_UnicodeToUTF8(argwstr, oscl_strlen(argwstr), argstr, 128);
            argstr[127] = '\0';
        }
        else
        {
            char* tmpstr = NULL;
            aCommandLine->get_arg(iFileSearch, tmpstr);
            int32 tmpstrlen = oscl_strlen(tmpstr) + 1;
            if (tmpstrlen > 128)
            {
                tmpstrlen = 128;
            }
            oscl_strncpy(argstr, tmpstr, tmpstrlen);
            argstr[tmpstrlen-1] = '\0';
        }

        // Do the string compare
        if (oscl_strcmp(argstr, "-help") == 0)
        {
            fprintf(aFile, "Output specification option.\n");
            fprintf(aFile, "  -output outputname\n");
            fprintf(aFile, "   Specify the output filename to use for test cases which\n");
            fprintf(aFile, "   allow user-specified source name. \n\n");
        }
        else if (oscl_strcmp(argstr, "-output") == 0)
        {
            iFileFound = true;
            iFileArgument = ++iFileSearch;
            break;
        }
    }

    if (iFileFound)
    {
        // Convert to UTF8 if necessary
        if (cmdline_iswchar)
        {
            oscl_wchar* cmd;
            aCommandLine->get_arg(iFileArgument, cmd);
            char tmpstr[256];
            oscl_UnicodeToUTF8(cmd, oscl_strlen(cmd), tmpstr, 256);
            tmpstr[255] = '\0';
            aOutputFileNameInfo = tmpstr;
        }
        else
        {
            char* cmdlinefilename = NULL;
            aCommandLine->get_arg(iFileArgument, cmdlinefilename);
            aOutputFileNameInfo = cmdlinefilename;
        }
    }
    else //if the -output tag is missing from cmd line
    {
        aOutputFileNameInfo = NULL;
    }
}

//Find test range args:
//To run a range of tests by enum ID:
//  -test 17 29
//if -test is not specified in the command line, it assumes running tests from 0 to CompressedNormalTestEnd-1
void FindTestRange(cmd_line *aCommandLine,	int32 &iFirstTest, int32 &iLastTest, FILE *aFile)
{
    //default is to run all tests.
    iFirstTest = 0;
    iLastTest = 999;

    int iTestArgument = 0;
    char *iTestArgStr1 = NULL;
    char *iTestArgStr2 = NULL;
    bool iTestFound = false;
    bool cmdline_iswchar = aCommandLine->is_wchar();

    int count = aCommandLine->get_count();

    // Search for the "-test" argument
    char *SourceFind = NULL;
    if (cmdline_iswchar)
    {
        SourceFind = new char[256];
    }

    // Go through each argument
    for (int iTestSearch = 0; iTestSearch < count; iTestSearch++)
    {
        // Convert to UTF8 if necessary
        if (cmdline_iswchar)
        {
            OSCL_TCHAR* cmd = NULL;
            aCommandLine->get_arg(iTestSearch, cmd);
            oscl_UnicodeToUTF8(cmd, oscl_strlen(cmd), SourceFind, 256);
        }
        else
        {
            SourceFind = NULL;
            aCommandLine->get_arg(iTestSearch, SourceFind);
        }

        // Do the string compare
        if (oscl_strcmp(SourceFind, "-help") == 0)
        {
            fprintf(aFile, "Test cases to run option. Default is ALL:\n");
            fprintf(aFile, "  -test x x\n");
            fprintf(aFile, "   Specify a range of test cases to run. To run one test case, use the\n");
            fprintf(aFile, "   same index for x and y.\n\n");

            iFirstTest = Invalid_Test;
            return;
        }
        else if (oscl_strcmp(SourceFind, "-test") == 0)
        {
            iTestFound = true;
            iTestArgument = ++iTestSearch;
            break;
        }
    }

    if (cmdline_iswchar)
    {
        delete[] SourceFind;
        SourceFind = NULL;
    }

    if (iTestFound)
    {
        // Convert to UTF8 if necessary
        if (cmdline_iswchar)
        {
            iTestArgStr1 = new char[256];
            OSCL_TCHAR* cmd;
            aCommandLine->get_arg(iTestArgument, cmd);
            if (cmd)
            {
                oscl_UnicodeToUTF8(cmd, oscl_strlen(cmd), iTestArgStr1, 256);
            }

            iTestArgStr2 = new char[256];
            aCommandLine->get_arg(iTestArgument + 1, cmd);
            if (cmd)
            {
                oscl_UnicodeToUTF8(cmd, oscl_strlen(cmd), iTestArgStr2, 256);
            }
        }
        else
        {
            aCommandLine->get_arg(iTestArgument, iTestArgStr1);
            aCommandLine->get_arg(iTestArgument + 1, iTestArgStr2);
        }

        //Pull out 2 integers...
        if (iTestArgStr1 && '0' <= iTestArgStr1[0] && iTestArgStr1[0] <= '9' && iTestArgStr2 && '0' <= iTestArgStr2[0] && iTestArgStr2[0] <= '9')
        {
            int len = oscl_strlen(iTestArgStr1);
            switch (len)
            {
                case 3:
                    iFirstTest = 0;
                    if ('0' <= iTestArgStr1[0] && iTestArgStr1[0] <= '9')
                    {
                        iFirstTest = iFirstTest + 100 * (iTestArgStr1[0] - '0');
                    }

                    if ('0' <= iTestArgStr1[1] && iTestArgStr1[1] <= '9')
                    {
                        iFirstTest = iFirstTest + 10 * (iTestArgStr1[1] - '0');
                    }

                    if ('0' <= iTestArgStr1[2] && iTestArgStr1[2] <= '9')
                    {
                        iFirstTest = iFirstTest + 1 * (iTestArgStr1[2] - '0');
                    }
                    break;

                case 2:
                    iFirstTest = 0;
                    if ('0' <= iTestArgStr1[0] && iTestArgStr1[0] <= '9')
                    {
                        iFirstTest = iFirstTest + 10 * (iTestArgStr1[0] - '0');
                    }

                    if ('0' <= iTestArgStr1[1] && iTestArgStr1[1] <= '9')
                    {
                        iFirstTest = iFirstTest + 1 * (iTestArgStr1[1] - '0');
                    }
                    break;

                case 1:
                    iFirstTest = 0;
                    if ('0' <= iTestArgStr1[0] && iTestArgStr1[0] <= '9')
                    {
                        iFirstTest = iFirstTest + 1 * (iTestArgStr1[0] - '0');
                    }
                    break;

                default:
                    break;
            }

            len = oscl_strlen(iTestArgStr2);
            switch (len)
            {
                case 3:
                    iLastTest = 0;
                    if ('0' <= iTestArgStr2[0] && iTestArgStr2[0] <= '9')
                    {
                        iLastTest = iLastTest + 100 * (iTestArgStr2[0] - '0');
                    }

                    if ('0' <= iTestArgStr2[1] && iTestArgStr2[1] <= '9')
                    {
                        iLastTest = iLastTest + 10 * (iTestArgStr2[1] - '0');
                    }

                    if ('0' <= iTestArgStr2[2] && iTestArgStr2[2] <= '9')
                    {
                        iLastTest = iLastTest + 1 * (iTestArgStr2[2] - '0');
                    }
                    break;

                case 2:
                    iLastTest = 0;
                    if ('0' <= iTestArgStr2[0] && iTestArgStr2[0] <= '9')
                    {
                        iLastTest = iLastTest + 10 * (iTestArgStr2[0] - '0');
                    }

                    if ('0' <= iTestArgStr2[1] && iTestArgStr2[1] <= '9')
                    {
                        iLastTest = iLastTest + 1 * (iTestArgStr2[1] - '0');
                    }
                    break;

                case 1:
                    iLastTest = 0;
                    if ('0' <= iTestArgStr2[0] && iTestArgStr2[0] <= '9')
                    {
                        iLastTest = iLastTest + 1 * (iTestArgStr2[0] - '0');
                    }
                    break;

                default:
                    break;
            }
        }
    }
    else //-test arg not given, assuming run tests from 0 to CompressedNormalTestEnd-1
    {
        iFirstTest = 0;
        iLastTest = CompressedNormalTestEnd - 1;
    }

    if (cmdline_iswchar)
    {
        if (iTestArgStr1)
        {
            delete[] iTestArgStr1;
            iTestArgStr1 = NULL;
        }

        if (iTestArgStr2)
        {
            delete[] iTestArgStr2;
            iTestArgStr2 = NULL;
        }

        if (SourceFind)
        {
            delete[] SourceFind;
            SourceFind = NULL;
        }
    }
}

//Look for switch to print mem leaks at the end of test. Use switch -leakinfo for finding memory leaks
void FindMemMgmtRelatedCmdLineParams(cmd_line* aCommandLine, bool& aPrintDetailedMemLeakInfo, FILE* aFile)
{
    aPrintDetailedMemLeakInfo = false;

    bool cmdline_iswchar = aCommandLine->is_wchar();

    int count = aCommandLine->get_count();

    // Search for the "-leakinfo" argument
    char *SourceFind = NULL;
    if (cmdline_iswchar)
    {
        SourceFind = new char[256];
    }

    // Go through each argument
    for (int iTestSearch = 0; iTestSearch < count; iTestSearch++)
    {
        // Convert to UTF8 if necessary
        if (cmdline_iswchar)
        {
            OSCL_TCHAR* cmd = NULL;
            aCommandLine->get_arg(iTestSearch, cmd);
            oscl_UnicodeToUTF8(cmd, oscl_strlen(cmd), SourceFind, 256);
        }
        else
        {
            SourceFind = NULL;
            aCommandLine->get_arg(iTestSearch, SourceFind);
        }

        // Do the string compare
        if (oscl_strcmp(SourceFind, "-help") == 0)
        {
            fprintf(aFile, "Printing leak info option. Default is OFF:\n");
            fprintf(aFile, "  -leakinfo\n");
            fprintf(aFile, "   If there is a memory leak, prints out the memory leak information\n");
            fprintf(aFile, "   after all specified test cases have finished running.\n\n");
        }
        else if (oscl_strcmp(SourceFind, "-leakinfo") == 0)
        {
            aPrintDetailedMemLeakInfo = true;
        }
    }

    if (cmdline_iswchar)
    {
        delete[] SourceFind;
        SourceFind = NULL;
    }
}

//This function is used to read the contents of AudioConfigFile and VideoConfigFile, one line at a time.
int fgetline(Oscl_File* aFp, char aLine[], int aMax)
{
    int nch = 0;
    char cc[2] = "\0";
    aMax = aMax - 1;

    while (aFp->Read((char*)cc, 1, 1) != 0)
    {
        if (cc[0] == '\n')
        {
            break;
        }
        if (nch < aMax)
        {
            aLine[nch] = cc[0];
            nch = nch + 1;
        }
    }

    if ((int)cc[0] == 0 && nch == 0)
    {
        return EOF;
    }
    aLine[nch] = '\0';
    return nch;
}

//This function is used to read the audioconfigfile and set the AVTConfig structure
bool LoadAudioConfiguration(OSCL_HeapString<OsclMemAllocator>& aAudioConfigFileName, AVTConfig& aAVTConfig, FILE* aFile)
{
    OSCL_UNUSED_ARG(aFile);
    char maxLine[200] = "\0";
    Oscl_FileServer iFileServer;
    Oscl_File fileConfig;
    iFileServer.Connect();
    if (fileConfig.Open(aAudioConfigFileName.get_cstr(), Oscl_File::MODE_READ | Oscl_File::MODE_TEXT, iFileServer) != 0)
    {
        return false;
    }

    while (fgetline(&fileConfig, maxLine, 200) != EOF)
    {
        // retrieve Sampling Rate
        if (oscl_CIstrncmp(maxLine, "SamplingRate", 12) == 0)
        {
            char* pcPtr = oscl_strchr(maxLine, '=');
            pcPtr++;
            PV_atoi(pcPtr, '0', (uint32&)(aAVTConfig.iSamplingRate)); // atoi syntax has to be checked once
        }

        // retrieve num of channels
        if (oscl_CIstrncmp(maxLine, "NumChannels", 11) == 0)
        {
            char* pcPtr = oscl_strchr(maxLine, '=');
            pcPtr++;
            PV_atoi(pcPtr, '0', (uint32&)aAVTConfig.iNumChannels);  // atoi syntax has to be checked once
        }
    }
    fileConfig.Close();
    iFileServer.Close();
    return true;
}


//This function is used to read the videoconfigfile and set the AVTConfig structure
bool LoadVideoConfiguration(OSCL_HeapString<OsclMemAllocator>& aVideoConfigFileName, AVTConfig& aAVTConfig, FILE* aFile)
{
    OSCL_UNUSED_ARG(aFile);
    char maxLine[200] = "\0";
    Oscl_FileServer iFileServer;
    Oscl_File fileConfig;

    iFileServer.Connect();

    if (fileConfig.Open(aVideoConfigFileName.get_cstr(), Oscl_File::MODE_READ | Oscl_File::MODE_TEXT, iFileServer) != 0)
    {
        return false;
    }

    while (fgetline(&fileConfig, maxLine, 200) != EOF)
    {
        // retrieve width
        if (oscl_CIstrncmp(maxLine, "width", 5) == 0)
        {
            char* pcPtr = oscl_strchr(maxLine, '=');
            pcPtr++;
            PV_atoi(pcPtr, '0', (uint32&)aAVTConfig.iWidth);  // atoi syntax has to be checked once

        }

        // retrieve height
        if (oscl_CIstrncmp(maxLine, "height", 6) == 0)
        {
            char* pcPtr = oscl_strchr(maxLine, '=');
            pcPtr++;
            PV_atoi(pcPtr, '0', (uint32&)aAVTConfig.iHeight);  // atoi syntax has to be checked once

        }

        // retrieve frames per second
        if (oscl_CIstrncmp(maxLine, "fps", 3) == 0)
        {
            char* pcPtr = oscl_strchr(maxLine, '=');
            pcPtr++;
            PV_atof(pcPtr, (OsclFloat&)aAVTConfig.iFps); // atoi syntax has to be checked once
        }

        // retrieve IFrame Interval
        if (oscl_CIstrncmp(maxLine, "iframeinterval", 14) == 0)
        {
            char* pcPtr = oscl_strchr(maxLine, '=');
            pcPtr++;
            PV_atoi(pcPtr, '0', (uint32&)aAVTConfig.iFrameInterval);  // atoi syntax has to be checked once
        }
    }
    fileConfig.Close();
    iFileServer.Close();
    return true;
}

//This function is used to read the AVIconfigfile and set the MediaInputTestParam structure
bool LoadAVIConfiguration(OSCL_HeapString<OsclMemAllocator>& aAVIConfigFileName, PVMediaInputAuthorEngineTestParam& aTestParam, FILE* aFile)
{
    OSCL_UNUSED_ARG(aFile);
    char maxLine[200] = "\0";
    Oscl_FileServer iFileServer;
    Oscl_File fileConfig;

    iFileServer.Connect();

    if (fileConfig.Open(aAVIConfigFileName.get_cstr(), Oscl_File::MODE_READ | Oscl_File::MODE_TEXT, iFileServer) != 0)
    {
        return false;
    }

    while (fgetline(&fileConfig, maxLine, MAXLINELENGTH) != EOF)
    {
        // retrieve video bitrate
        if (oscl_CIstrncmp(maxLine, "videobitrate", oscl_strlen("videobitrate")) == 0)
        {
            char* pcPtr = oscl_strchr(maxLine, ':');
            pcPtr++;
            PV_atoi(pcPtr, '0', (uint32&)aTestParam.iMediainputParam.iVideoBitrate);
        }
        // retrieve Audio bitrate
        if (oscl_CIstrncmp(maxLine, "audiobitrate", oscl_strlen("audiobitrate")) == 0)
        {
            char* pcPtr = oscl_strchr(maxLine, ':');
            pcPtr++;
            PV_atoi(pcPtr, '0', (uint32&)aTestParam.iMediainputParam.iAudioBitrate);
        }

        // retrieve Sampling rate
        if (oscl_CIstrncmp(maxLine, "samplingrate", oscl_strlen("samplingrate")) == 0)
        {
            char* pcPtr = oscl_strchr(maxLine, ':');
            pcPtr++;
            PV_atoi(pcPtr, '0', (uint32&)aTestParam.iMediainputParam.iSamplingRate);
        }
        // retrieve framerate
        if (oscl_CIstrncmp(maxLine, "framerate", oscl_strlen("framerate")) == 0)
        {
            char* pcPtr = oscl_strchr(maxLine, ':');
            pcPtr++;
            PV_atof(pcPtr, (OsclFloat&)aTestParam.iMediainputParam.iFrameRate);
        }


    }
    fileConfig.Close();
    iFileServer.Close();
    return true;
}

//Depending on the Test Nos, check the validity of input and output files extensions as specified in command line
//Also set the AVTConfig param by loading the contents of audio and video config files
//This function is used only for Test No 0 to CompressedNormalTestEnd-1
bool CheckSourceAndOutputFiles(cmd_line* aCommandLine, int32 firsttest, int32 lasttest, OSCL_HeapString<OsclMemAllocator> &aInputAudioFileName, OSCL_HeapString<OsclMemAllocator> &aInputVideoFileName, OSCL_HeapString<OsclMemAllocator> &aInputTextFileName, OSCL_HeapString<OsclMemAllocator> &aOutputFileName, AVTConfig &aAVTConfig, OSCL_HeapString<OsclMemAllocator> &audioconfigfilename, OSCL_HeapString<OsclMemAllocator> &videoconfigfilename, FILE* file)
{
    bool bAudioTest = false;//To track if the test is Audio
    bool bVideoTest = false;//To track if the test is Video
    bool bTextTest = false;//To track if the test is Text

    switch (firsttest)
    {
        case AMR_FOutput_Test:
        case AMR_Input_AOnly_3gpTest:
        case AMR_YUV_Input_AV_3gpTest:
        case AMR_H263_Input_AV_3gpTest:
        case AMR_YUV_Input_AV_M4V_AMR_Output_3gpTest:
        case H264_AMR_Input_AV_3gpTest:
        case AMR_FileOutput_Test_UsingExternalFileHandle:
            //Longetivity Tests
        case AMR_TEXT_Input_AT_3gp_LongetivityTest:
        case AMR_YUV_TEXT_Input_AVT_3gp_LongetivityTest:
            bAudioTest = true;
            if (!(oscl_strstr(aInputAudioFileName.get_cstr(), ".amr") != NULL || oscl_strstr(aInputAudioFileName.get_cstr(), ".AMR") != NULL))
            {
                fprintf(file, "  Input Filename incorrect!!! TestNo:%d - %d needs Input File: -audio <xxx>.amr\n\n", firsttest, lasttest);
                return false;
            }
            break;

        case AMRWB_Input_AOnly_3gpTest:
        case AMRWB_FOutput_Test:
            bAudioTest = true;
            if (!(oscl_strstr(aInputAudioFileName.get_cstr(), ".awb") != NULL || oscl_strstr(aInputAudioFileName.get_cstr(), ".AWB") != NULL))
            {
                fprintf(file, "  Input Filename incorrect!!! TestNo:%d - %d needs Input File: -audio <xxx>.awb\n\n", firsttest, lasttest);
                return false;
            }
            break;

        case AACADIF_FOutput_Test:
            bAudioTest = true;
            if (!(oscl_strstr(aInputAudioFileName.get_cstr(), ".aacadif") != NULL || oscl_strstr(aInputAudioFileName.get_cstr(), ".AACADIF") != NULL))
            {
                fprintf(file, "  Input Filename incorrect!!! TestNo:%d - %d needs Input File:-audio <xxx>.aacadif\n\n", firsttest, lasttest);
                return false;
            }
            break;
        case AACADTS_FOutput_Test:
            bAudioTest = true;
            if (!(oscl_strstr(aInputAudioFileName.get_cstr(), ".aacadts") != NULL || oscl_strstr(aInputAudioFileName.get_cstr(), ".AACADTS") != NULL))
            {
                fprintf(file, "  Input Filename incorrect!!! TestNo:%d - %d needs Input File:-audio <xxx>.aacadts\n\n", firsttest, lasttest);
                return false;
            }
            break;
            //Used for generic testcases
        default:
            if (!(aInputAudioFileName == NULL))
            {
                bAudioTest = true;
            }
            break;
    }
    if (bAudioTest) //If the test involves audio input, look for audio config file
    {
        bool bRetVal = false;
        // Load audio configuration
        if (FindAudioConfigFile(aCommandLine, audioconfigfilename, file))
        {
            bRetVal = LoadAudioConfiguration(audioconfigfilename, aAVTConfig, file);
        }

        if (!bRetVal) //Could not find Config File
        {
            fprintf(file, "  Audio Config File not available!!! Specify -audioconfigfile <filename>\n\n");
            return false;
        }
    }
    switch (firsttest)
    {
        case AMR_YUV_Input_AV_3gpTest:
        case AMR_YUV_Input_AV_M4V_AMR_Output_3gpTest:
        case YUV_TEXT_Input_VT_3gp_LongetivityTest:
        case AMR_YUV_TEXT_Input_AVT_3gp_LongetivityTest:
            bVideoTest = true;
            if (!(oscl_strstr(aInputVideoFileName.get_cstr(), ".yuv") != NULL || oscl_strstr(aInputVideoFileName.get_cstr(), ".YUV") != NULL))
            {
                fprintf(file, "  Input Filename incorrect!!! TestNo:%d - %d needs Input File:-video <xxx>.yuv\n\n", firsttest, lasttest);
                return false;
            }
            break;
        case H263_Input_VOnly_3gpTest:
        case AMR_H263_Input_AV_3gpTest:
            bVideoTest = true;
            if (!(oscl_strstr(aInputVideoFileName.get_cstr(), ".h263") != NULL || oscl_strstr(aInputVideoFileName.get_cstr(), ".H263") != NULL))
            {
                fprintf(file, "  Input Filename incorrect!!! TestNo:%d - %d needs Input File:-video <xxx>.h263\n\n", firsttest, lasttest);
                return false;
            }
            break;
        case H264_AMR_Input_AV_3gpTest:
            bVideoTest = true;
            if (!(oscl_strstr(aInputVideoFileName.get_cstr(), ".h264") != NULL || oscl_strstr(aInputVideoFileName.get_cstr(), ".H264") != NULL))
            {
                fprintf(file, "  Input Filename incorrect!!! TestNo:%d - %d needs Input File:-video <xxx>.h264\n\n", firsttest, lasttest);
                return false;
            }
            break;
            //Used for generic testcases
        default:
            if (!(aInputVideoFileName == NULL))
            {
                bVideoTest = true;
            }
            break;
    }
    if (bVideoTest) //If the test involves video input, look for audio config file
    {
        bool bRetVal = false;
        // Load video configuration
        if (FindVideoConfigFile(aCommandLine, videoconfigfilename, file))
        {
            bRetVal = LoadVideoConfiguration(videoconfigfilename, aAVTConfig, file);
        }

        if (!bRetVal) //Could not find Config File
        {
            fprintf(file, "  Video Config File not available!!! Specify -videoconfigfile <filename>\n\n");
            return false;
        }
    }
    switch (firsttest)
    {
        case TEXT_Input_TOnly_3gpTest:
        case AMR_TEXT_Input_AT_3gpTest:
        case YUV_TEXT_Input_VT_3gpTest:
        case AMR_YUV_TEXT_Input_AVT_Mp4Test:
        case TEXT_Input_TOnly_3gp_LongetivityTest:
        case AMR_TEXT_Input_AT_3gp_LongetivityTest:
        case YUV_TEXT_Input_VT_3gp_LongetivityTest:
        case AMR_YUV_TEXT_Input_AVT_3gp_LongetivityTest:
            bTextTest = true;
            if (!(oscl_strstr(aInputTextFileName.get_cstr(), ".txt") != NULL || oscl_strstr(aInputTextFileName.get_cstr(), ".TXT") != NULL))
            {
                fprintf(file, "  Input Filename incorrect!!! TestNo:%d - %d needs Input File:-text <xxx>.txt\n\n", firsttest, lasttest);
                return false;
            }
            break;
        default:
            if (!(aInputTextFileName == NULL))
            {
                bTextTest = true;
            }
            break;
    }

    if (bTextTest)
    {
        if (((aAVTConfig.iTextLogFile).get_size() == 0) && ((aAVTConfig.iTextConfigFile).get_size() == 0))
        {
            fprintf(file, "  Text Config File/LogFile not available!!! Specify -textlogfile <filename> -textconfigfile <filename>\n\n");
            return false;
        }
    }
    if ((firsttest >= 0 && lasttest < K3GP_OUTPUT_TestEnd) || (firsttest >= CompressedLongetivityTestBegin && lasttest < Compressed_LongetivityTestEnd))
    {
        if (!(oscl_strstr(aOutputFileName.get_cstr(), ".3gp") != NULL || oscl_strstr(aOutputFileName.get_cstr(), ".3GP") != NULL))
        {
            fprintf(file, "  Output Filename incorrect!!! TestNo:%d - %d needs Output File:-output <xxx>.3gp\n\n", firsttest, lasttest);
            return false;
        }
    }
    else if ((firsttest > K3GP_OUTPUT_TestEnd && lasttest < AMR_OUTPUT_TestEnd))
    {
        if (!(oscl_strstr(aOutputFileName.get_cstr(), ".amr") != NULL || oscl_strstr(aOutputFileName.get_cstr(), ".AMR") != NULL))
        {
            fprintf(file, "Output Filename incorrect!!! Output File:-output <xxx>.amr\n\n");
            return false;
        }
    }
    else if ((firsttest == AACADIF_FOutput_Test))
    {
        if (!(oscl_strstr(aOutputFileName.get_cstr(), ".aacadif") != NULL || oscl_strstr(aOutputFileName.get_cstr(), ".AACADIF") != NULL))
        {
            fprintf(file, "Output Filename incorrect!!! Output File:-output <xxx>.aacadif\n\n");
            return false;
        }
    }
    else if ((firsttest == AACADTS_FOutput_Test))
    {
        if (!(oscl_strstr(aOutputFileName.get_cstr(), ".aacadts") != NULL || oscl_strstr(aOutputFileName.get_cstr(), ".AACADTS") != NULL))
        {
            fprintf(file, "Output Filename incorrect!!! Output File:-output <xxx>.aacadts\n\n");
            return false;
        }
    }
    return true;
}


////////////////////////////////////////////////////////////////////////////
int _local_main(FILE *filehandle, cmd_line* command_line);

int local_main(FILE* filehandle, cmd_line* command_line)
{
    int retVal;
    retVal = 0;
    OsclBase::Init();
    OsclErrorTrap::Init();
    OsclMem::Init();

#if USE_OMX_ENC_NODE
    OMX_Init();
#endif

    {
        PVSDKInfo aSdkInfo;
        PVAuthorEngineInterface::GetSDKInfo(aSdkInfo);
        fprintf(filehandle, "SDK Labeled: %s built on %x\n\n",               // display SDK info
                aSdkInfo.iLabel.get_cstr(), aSdkInfo.iDate);
    }

    file = filehandle;
    fprintf(file, "PVAuthorEngine Unit Test\n\n");

    bool oPrintDetailedMemLeakInfo = false;
    FindMemMgmtRelatedCmdLineParams(command_line, oPrintDetailedMemLeakInfo, filehandle);

    //Run the test under a trap
    int32 err;

    OSCL_TRY(err, retVal = _local_main(filehandle, command_line););

    //Show any exception.
    if (err != 0)
    {
        fprintf(file, "Error!  Leave %d\n", err);
    }
#if USE_OMX_ENC_NODE
    OMX_Deinit();
#endif
    //Cleanup
#if !(OSCL_BYPASS_MEMMGT)
    {
        //Check for memory leaks before cleaning up OsclMem.
        OsclAuditCB auditCB;
        OsclMemInit(auditCB);
        if (auditCB.pAudit)
        {
            MM_Stats_t *stats = auditCB.pAudit->MM_GetStats("");
            if (stats)
            {
                fprintf(file, "\nMemory Stats:\n");
                fprintf(file, "  peakNumAllocs %d\n", stats->peakNumAllocs);
                fprintf(file, "  peakNumBytes %d\n", stats->peakNumBytes);
                fprintf(file, "  totalNumAllocs %d\n", stats->totalNumAllocs);
                fprintf(file, "  totalNumBytes %d\n", stats->totalNumBytes);
                fprintf(file, "  numAllocFails %d\n", stats->numAllocFails);
                if (stats->numAllocs)
                {
                    fprintf(file, "  ERROR: Memory Leaks! numAllocs %d, numBytes %d\n", stats->numAllocs, stats->numBytes);
                }
            }
            uint32 leaks = auditCB.pAudit->MM_GetNumAllocNodes();
            if (leaks != 0)
            {
                if (oPrintDetailedMemLeakInfo)
                {
                    fprintf(file, "ERROR: %d Memory leaks detected!\n", leaks);
                    MM_AllocQueryInfo*info = auditCB.pAudit->MM_CreateAllocNodeInfo(leaks);
                    uint32 leakinfo = auditCB.pAudit->MM_GetAllocNodeInfo(info, leaks, 0);
                    if (leakinfo != leaks)
                    {
                        fprintf(file, "ERROR: Leak info is incomplete.\n");
                    }
                    for (uint32 ii = 0;ii < leakinfo;ii++)
                    {
                        fprintf(file, "Leak Info:\n");
                        fprintf(file, "  allocNum %d\n", info[ii].allocNum);
                        fprintf(file, "  fileName %s\n", info[ii].fileName);
                        fprintf(file, "  lineNo %d\n", info[ii].lineNo);
                        fprintf(file, "  size %d\n", info[ii].size);
                        fprintf(file, "  pMemBlock 0x%x\n", (uint32)info[ii].pMemBlock);
                        fprintf(file, "  tag %s\n", info[ii].tag);
                    }
                    auditCB.pAudit->MM_ReleaseAllocNodeInfo(info);
                }
            }
        }
    }

#endif

    OsclMem::Cleanup();
    OsclErrorTrap::Cleanup();
    OsclBase::Cleanup();

    return retVal;
}


void FindAuthoringTime(cmd_line* aCmdLine, uint32& aAuthoringTime, FILE* aFile)
{
    int timeArgument = 0;
    bool timeArgFound = false;
    bool cmdline_iswchar = aCmdLine->is_wchar();

    int count = aCmdLine->get_count();

    // Search for the "-duration" argument
    // Go through each argument
    for (int argSearch = 0; argSearch < count; argSearch++)
    {
        char argstr[128];
        // Convert to UTF8 if necessary
        if (cmdline_iswchar)
        {
            oscl_wchar* argwstr = NULL;
            aCmdLine->get_arg(argSearch, argwstr);
            oscl_UnicodeToUTF8(argwstr, oscl_strlen(argwstr), argstr, 128);
            argstr[127] = '\0';
        }
        else
        {
            char* tmpstr = NULL;
            aCmdLine->get_arg(argSearch, tmpstr);
            int32 tmpstrlen = oscl_strlen(tmpstr) + 1;
            if (tmpstrlen > 128)
            {
                tmpstrlen = 128;
            }
            oscl_strncpy(argstr, tmpstr, tmpstrlen);
            argstr[tmpstrlen-1] = '\0';
        }

        // Do the string compare
        if (0 == oscl_strcmp(argstr, "-help"))
        {
            fprintf(aFile, "Authoring Time specification option for Author.:\n");
            fprintf(aFile, "  -duration time_in_seconds\n");
            fprintf(aFile, "   Specify the time argument to use for test cases which\n");
            fprintf(aFile, "   allow user-specified source name.\n\n");
        }
        else if (0 == oscl_strcmp(argstr, "-duration"))
        {
            timeArgFound = true;
            timeArgument = ++argSearch;
            break;
        }
    }

    if (timeArgFound)
    {
        // Convert to UTF8 if necessary
        if (cmdline_iswchar)
        {
            oscl_wchar* wCharTime;
            aCmdLine->get_arg(timeArgument, wCharTime);
            char tmpstr[256];
            oscl_UnicodeToUTF8(wCharTime, oscl_strlen(wCharTime), tmpstr, 256);
            tmpstr[255] = '\0';
            PV_atoi(tmpstr, '0', aAuthoringTime); // ash liz
        }
        else
        {
            char* CharTime;
            aCmdLine->get_arg(timeArgument, CharTime);
            PV_atoi(CharTime, '0', aAuthoringTime);
        }
    }
    else //if the -duration tag is missing from cmd line
    {
        aAuthoringTime = 0;
    }
}

// Pull out source file name from arguments
//	-source sometestfile.mp4
//
//
void FindSourceFile(cmd_line* command_line, OSCL_HeapString<OsclMemAllocator> &aFileNameInfo, PVMFFormatType &aInputFileFormatType, FILE *aFile)
{
    int iFileArgument = 0;
    bool iFileFound = false;
    bool cmdline_iswchar = command_line->is_wchar();

    int count = command_line->get_count();

    // Search for the "-source" argument
    // Go through each argument
    for (int iFileSearch = 0; iFileSearch < count; iFileSearch++)
    {
        char argstr[128];
        // Convert to UTF8 if necessary
        if (cmdline_iswchar)
        {
            oscl_wchar* argwstr = NULL;
            command_line->get_arg(iFileSearch, argwstr);
            oscl_UnicodeToUTF8(argwstr, oscl_strlen(argwstr), argstr, 128);
            argstr[127] = '\0';
        }
        else
        {
            char* tmpstr = NULL;
            command_line->get_arg(iFileSearch, tmpstr);
            int32 tmpstrlen = oscl_strlen(tmpstr) + 1;
            if (tmpstrlen > 128)
            {
                tmpstrlen = 128;
            }
            oscl_strncpy(argstr, tmpstr, tmpstrlen);
            argstr[tmpstrlen-1] = '\0';
        }

        // Do the string compare
        if (oscl_strcmp(argstr, "-help") == 0)
        {
            fprintf(aFile, "Source specification option.:\n");
            fprintf(aFile, "  -source sourcename\n");
            fprintf(aFile, "   Specify the source filename or URL to use for test cases which\n");
            fprintf(aFile, "   allow user-specified source name. The unit test determines the\n");
            fprintf(aFile, "   source format type using extension or URL header.\n\n");
        }
        else if (oscl_strcmp(argstr, "-source") == 0)
        {
            iFileFound = true;
            iFileArgument = ++iFileSearch;
            break;
        }
    }

    if (iFileFound)
    {
        // Convert to UTF8 if necessary
        if (cmdline_iswchar)
        {
            oscl_wchar* cmd;
            command_line->get_arg(iFileArgument, cmd);
            char tmpstr[256];
            oscl_UnicodeToUTF8(cmd, oscl_strlen(cmd), tmpstr, 256);
            tmpstr[255] = '\0';
            aFileNameInfo = tmpstr;
        }
        else
        {
            char* cmdlinefilename = NULL;
            command_line->get_arg(iFileArgument, cmdlinefilename);
            aFileNameInfo = cmdlinefilename;
        }

        // Check the file extension to determine format type
        // AVI file
        if (oscl_strstr(aFileNameInfo.get_cstr(), ".avi") != NULL || oscl_strstr(aFileNameInfo.get_cstr(), ".AAC") != NULL)
        {
            aInputFileFormatType = PVMF_MIME_AVIFF;
        }
        // WAV file
        else  if (oscl_strstr(aFileNameInfo.get_cstr(), ".wav") != NULL || oscl_strstr(aFileNameInfo.get_cstr(), ".MP3") != NULL)
        {
            aInputFileFormatType = PVMF_MIME_WAVFF;
        }
        // Unknown so set to unknown try to have the player engine recognize
        else
        {
            fprintf(file, "Source type unknown so setting to unknown and have the player engine recognize it\n");
            aInputFileFormatType = PVMF_MIME_FORMAT_UNKNOWN;
        }
    }
}

//get video encoder type from arguments
// -encV 0: M4V
//       1: H263
//       2: H264

void FindVideoEncoder(cmd_line* command_line, OSCL_HeapString<OsclMemAllocator>& aVideoEncoderInfo, FILE *aFile)
{
    aVideoEncoderInfo = SOURCENAME_PREPEND_STRING;
    aVideoEncoderInfo += KMp4EncMimeType;

    int cmdArgIndex = 0;
    bool cmdline_iswchar = command_line->is_wchar();

    int count = command_line->get_count();

    // Search for the "-encV" argument
    for (int ii = 0; ii < count; ii++)
    {
        char argstr[128];

        // Convert to UTF8 if necessary
        if (cmdline_iswchar)
        {
            oscl_wchar* argwstr = NULL;
            command_line->get_arg(ii, argwstr);
            oscl_UnicodeToUTF8(argwstr, oscl_strlen(argwstr), argstr, 128);
            argstr[127] = '\0';
        }
        else
        {
            char* tmpargstr = NULL;
            command_line->get_arg(ii, tmpargstr);
            uint32 len = oscl_strlen(tmpargstr);
            oscl_strncpy(argstr, tmpargstr, len);
            argstr[len] = '\0';
        }

        // Do the string compare
        if (oscl_strcmp(argstr, "-help") == 0)
        {
            fprintf(aFile, "Video Encoder Type option. Default is M4V:\n");
            fprintf(aFile, "  -encV encodertype\n");
            fprintf(aFile, "  specifies the encoder to be used for authoring\n  0:M4V\n  1:H263\n  2:H264\n");
            fprintf(aFile, "  e.g -encV 0 \n\n");
        }
        else if (oscl_strcmp(argstr, "-encV") == 0)
        {
            cmdArgIndex = ++ii;
            break;
        }
    }

    if (cmdArgIndex > 0)
    {
        uint32 encType = 0;

        // Convert to UTF8 if necessary
        if (cmdline_iswchar)
        {
            oscl_wchar* cmdArg;
            command_line->get_arg(cmdArgIndex, cmdArg);
            char tmpstr[3];
            oscl_UnicodeToUTF8(cmdArg, oscl_strlen(cmdArg), tmpstr, 3);
            tmpstr[2] = '\0';
            PV_atoi(tmpstr, 'd', 1, encType);
        }
        else
        {
            char* cmdArg = NULL;
            command_line->get_arg(cmdArgIndex, cmdArg);
            PV_atoi(cmdArg, 'd', 1, encType);

        }

        switch (encType)
        {
            case 0:
                aVideoEncoderInfo = KMp4EncMimeType;
                break;
            case 1:
                aVideoEncoderInfo = KH263EncMimeType;
                break;
            case 2:
                aVideoEncoderInfo = KH264EncMimeType;
                break;
            default:
                fprintf(aFile, "Encoder Type not supported\n Using M4V encoder\n");
                break;
        }

    }
}

//get audio encoder type from arguments
// -encA 0: AMR-NB
//       1: AMR-WB
//       2: AAC-ADIF
//       3: AAC-ADTS
//       4: AAC-MPEG4_AUDIO

void FindAudioEncoder(cmd_line* command_line, OSCL_HeapString<OsclMemAllocator>& aAudioEncoderInfo, FILE *aFile)
{
    aAudioEncoderInfo = SOURCENAME_PREPEND_STRING;
    aAudioEncoderInfo += KAMRNbEncMimeType;

    int cmdArgIndex = 0;
    bool cmdline_iswchar = command_line->is_wchar();

    int count = command_line->get_count();

    // Search for the "-encA" argument
    for (int ii = 0; ii < count; ii++)
    {
        char argstr[128];

        // Convert to UTF8 if necessary
        if (cmdline_iswchar)
        {
            oscl_wchar* argwstr = NULL;
            command_line->get_arg(ii, argwstr);
            oscl_UnicodeToUTF8(argwstr, oscl_strlen(argwstr), argstr, 128);
            argstr[127] = '\0';
        }
        else
        {
            char* tmpargstr = NULL;
            command_line->get_arg(ii, tmpargstr);
            uint32 len = oscl_strlen(tmpargstr);
            oscl_strncpy(argstr, tmpargstr, len);
            argstr[len] = '\0';
        }

        // Do the string compare
        if (oscl_strcmp(argstr, "-help") == 0)
        {
            fprintf(aFile, "Audio Encoder Type option. Default is AMRNb:\n");
            fprintf(aFile, "  -encA encodertype\n");
            fprintf(aFile, "  specifies the encoder to be used for authoring\n  0:AMRNb\n  1:AMRWb\n  2:AAC_ADIF\n 3:AAC_ADTS\n 4:AAC MPEG4\n");
            fprintf(aFile, "  e.g -encA 0 \n\n");
        }
        else if (oscl_strcmp(argstr, "-encA") == 0)
        {
            cmdArgIndex = ++ii;
            break;
        }
    }

    if (cmdArgIndex > 0)
    {
        uint32 encType = 0;

        // Convert to UTF8 if necessary
        if (cmdline_iswchar)
        {
            oscl_wchar* cmdArg;
            command_line->get_arg(cmdArgIndex, cmdArg);
            char tmpstr[3];
            oscl_UnicodeToUTF8(cmdArg, oscl_strlen(cmdArg), tmpstr, 3);
            tmpstr[2] = '\0';
            PV_atoi(tmpstr, 'd', 1, encType);
        }
        else
        {
            char* cmdArg = NULL;
            command_line->get_arg(cmdArgIndex, cmdArg);
            PV_atoi(cmdArg, 'd', 1, encType);

        }

        switch (encType)
        {
            case 0:
                aAudioEncoderInfo = KAMRNbEncMimeType;
                break;
            case 1:
                aAudioEncoderInfo = KAMRWbEncMimeType;
                break;
            case 2:
                aAudioEncoderInfo = KAACADIFEncMimeType;
                break;
            case 3:
                aAudioEncoderInfo = KAACADTSEncMimeType;
                break;
            case 4:
                aAudioEncoderInfo = KAACMP4EncMimeType;
                break;
            default:
                fprintf(aFile, "Encoder Type not supported\n Using AMR-NB encoder\n");
                break;
        }

    }
}

bool FindAuthoringMode(cmd_line* command_line, FILE *aFile)
{
    bool iAuthoringMode = false;
    bool cmdline_iswchar = command_line->is_wchar();

    int count = command_line->get_count();

    // Search for the "-realtime" argument
    for (int ii = 0; ii < count; ii++)
    {
        char argstr[128];

        // Convert to UTF8 if necessary
        if (cmdline_iswchar)
        {
            oscl_wchar* argwstr = NULL;
            command_line->get_arg(ii, argwstr);
            oscl_UnicodeToUTF8(argwstr, oscl_strlen(argwstr), argstr, 128);
            argstr[127] = '\0';
        }
        else
        {
            char* tmpargstr = NULL;
            command_line->get_arg(ii, tmpargstr);
            uint32 len = oscl_strlen(tmpargstr);
            oscl_strncpy(argstr, tmpargstr, len);
            argstr[len] = '\0';
        }

        // Do the string compare
        if (oscl_strcmp(argstr, "-help") == 0)
        {
            fprintf(aFile, "  -realtime \n");
            fprintf(aFile, "  specifies the authoring of output file will be done in real time mode\n");
            fprintf(aFile, "  test cases should be run one by one to author a file in realtime mode\n");
            fprintf(aFile, "  by default file will be authored in ASAP mode\n\n");
        }
        else if (0 == oscl_strcmp(argstr, "-realtime"))
        {
            iAuthoringMode = true;
            break;
        }
    }

    return iAuthoringMode;
}
OSCL_HeapString<OsclMemAllocator> FindComposerType(OSCL_HeapString<OsclMemAllocator> aFileName, FILE* aFile)
{
    OSCL_HeapString<OsclMemAllocator> compType;
    if (oscl_strstr(aFileName.get_str(), ".3gp"))
    {
        compType = K3gpComposerMimeType;
    }
    else if (oscl_strstr(aFileName.get_str(), ".amr"))
    {
        compType = KAMRNbComposerMimeType;
    }
    else if (oscl_strstr(aFileName.get_str(), ".awb"))
    {
        compType = KAMRWBComposerMimeType;
    }
    else
    {
        fprintf(aFile, "\n\nNo output file specified\n, Using default MP4 Composer\n");
        compType = K3gpComposerMimeType;
    }

    return compType;
}
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

int RunCompressedTest(cmd_line *aCommandLine, int32 &iFirstTest, int32 &iLastTest, FILE *afilehandle)
{
    int retVal = 1;
    file = afilehandle;
    int32 err;

    OSCL_HeapString<OsclMemAllocator> audiofilenameinfo = NULL;
    OSCL_HeapString<OsclMemAllocator> videofilenameinfo = NULL;
    OSCL_HeapString<OsclMemAllocator> textfilenameinfo = NULL;
    OSCL_HeapString<OsclMemAllocator> outputfilenameinfo = NULL;

    OSCL_HeapString<OsclMemAllocator> audioconfigfilename = NULL;
    OSCL_HeapString<OsclMemAllocator> videoconfigfilename = NULL;
    AVTConfig aAVTConfig;

    //Hard Coded Audio/Video values
    aAVTConfig.iWidth = KVideoFrameWidth;
    aAVTConfig.iHeight = KVideoFrameHeight;
    aAVTConfig.iFps = KVideoFrameRate;
    aAVTConfig.iFrameInterval = KVideoIFrameInterval;

    aAVTConfig.iNumChannels = KAudioNumChannels;
    aAVTConfig.iSamplingRate = KAudioTimescale;
    aAVTConfig.iLoopingEnable = false;
    // Check -audio, -video and -output tag if user wants to use command line given compressed inputs
    if (iFirstTest == iLastTest)
    {
        FindAudioSourceFile(aCommandLine, audiofilenameinfo, file);

        FindVideoSourceFile(aCommandLine, videofilenameinfo, file);

        FindTextSourceFile(aCommandLine, textfilenameinfo, aAVTConfig.iTextLogFile, aAVTConfig.iTextConfigFile,  file);

        FindOutputFile(aCommandLine, outputfilenameinfo, file);
    }

    PVAETestInputType aAudioInputType = INVALID_INPUT_TYPE;	// param1
    PVAETestInputType aVideoInputType = INVALID_INPUT_TYPE;	// param2
    PVAETestInputType aTextInputType = INVALID_INPUT_TYPE;	// param3
    OSCL_HeapString<OsclMemAllocator> aComposerMimeType = NULL;	// param3
    OSCL_HeapString<OsclMemAllocator> aAudioEncoderMimeType = NULL;	// param4
    OSCL_HeapString<OsclMemAllocator> aVideoEncoderMimeType = NULL;	// param5
    OSCL_HeapString<OsclMemAllocator> aTextEncoderMimeType = NULL;	// param6

    //If -audio , -video, -text and -output tags are not specified, we will assume hard coded input and output filenames(So no need to check validity of args)
    if (!((audiofilenameinfo == NULL) && (videofilenameinfo == NULL) && (textfilenameinfo == NULL) && (outputfilenameinfo == NULL)))
    {
        //This function will be used for only non generic tests
        if (CheckSourceAndOutputFiles(aCommandLine, iFirstTest, iLastTest, audiofilenameinfo, videofilenameinfo, textfilenameinfo, outputfilenameinfo, aAVTConfig, audioconfigfilename, videoconfigfilename, file) == false)
        {
            return 1;
        }
    }


    uint32 AuthoringTime;
    if ((iLastTest >= CompressedLongetivityTestBegin) && (iFirstTest <= Compressed_LongetivityTestEnd))

    {
        FindAuthoringTime(aCommandLine, AuthoringTime, file);
        aAVTConfig.iLoopingEnable = true;
    }

    fprintf(file, "  \nInput audio file name:%s\n  Input video filename:%s\n  Output filename:%s \n", audiofilenameinfo.get_cstr(), videofilenameinfo.get_cstr(), outputfilenameinfo.get_cstr());
    fprintf(file, "  Audio Configfile name:%s\n  Video Configfilename:%s\n", audioconfigfilename.get_cstr(), videoconfigfilename.get_cstr());
    fprintf(file, "  Test case range %d to %d\n", iFirstTest, iLastTest);

    OSCL_TRY(err,

             PVAuthorEngineTestSuite* testSuite 	= new PVAuthorEngineTestSuite(file, iFirstTest, iLastTest,
                     audiofilenameinfo.get_cstr(), videofilenameinfo.get_cstr(), textfilenameinfo.get_cstr(),
                     outputfilenameinfo.get_cstr(),	aAVTConfig,
                     aAudioInputType, aVideoInputType, aTextInputType,
                     aComposerMimeType.get_cstr(), aAudioEncoderMimeType.get_cstr(), aVideoEncoderMimeType.get_cstr(), aTextEncoderMimeType.get_cstr(), AuthoringTime);

             testSuite->run_test();
             //if (runTestErr != OSCL_ERR_NONE)
             //	fprintf(file, "ERROR: Leave Occurred! Reason %d \n", runTestErr);

             text_test_interpreter interp;
             _STRING rs = interp.interpretation(testSuite->last_result());
             fprintf(file, rs.c_str());
             const test_result the_result = testSuite->last_result();
             retVal = (int)(the_result.success_count() != the_result.total_test_count());

             delete testSuite;
             testSuite = NULL;
            );
    // end if statement if ((iFirstTest <= CompressedNormalTestEnd))
    if (err != OSCL_ERR_NONE)
    {
        fprintf(file, "ERROR: Leave Occurred! Reason %d \n", err);
        return 1;
    }
    return retVal;
}
/////////////////////////////////////////////////////////////////////////////////////////

int RunUnCompressedTest(cmd_line *aCommandLine, int32 &aFirstTest, int32 &aLastTest, FILE *afilehandle)
{
    int retVal = 1;
    file = afilehandle;
    int32 err;

    OSCL_HeapString<OsclMemAllocator> filenameinfo;
    OSCL_HeapString<OsclMemAllocator> outputfilenameinfo;
    OSCL_HeapString<OsclMemAllocator> videoencoderinfo;
    OSCL_HeapString<OsclMemAllocator> audioencoderinfo;
    OSCL_HeapString<OsclMemAllocator> configfileinfo;
    PVMFFormatType inputformattype ;
    PVMediaInputAuthorEngineTestParam testparam;


    FindSourceFile(aCommandLine, filenameinfo, inputformattype, file);
    FindOutputFile(aCommandLine, outputfilenameinfo, file);
    FindVideoEncoder(aCommandLine, videoencoderinfo, file);
    FindAudioEncoder(aCommandLine, audioencoderinfo, file);

    testparam.iFirstTest = aFirstTest;
    testparam.iLastTest = aLastTest;
    testparam.iMediainputParam.iFile = file;
    testparam.iMediainputParam.iInputFormat = inputformattype;
    testparam.iMediainputParam.iIPFileInfo = filenameinfo;
    testparam.iMediainputParam.iOPFileInfo = outputfilenameinfo;
    testparam.iMediainputParam.iVideoEncInfo = videoencoderinfo;
    testparam.iMediainputParam.iAudioEncInfo = audioencoderinfo; //KAMRNbEncMimeType;
    testparam.iMediainputParam.iComposerInfo = FindComposerType(outputfilenameinfo, file);

    //setting the default configuration info
    testparam.iMediainputParam.iAudioBitrate = 0;
    testparam.iMediainputParam.iVideoBitrate = 0;
    testparam.iMediainputParam.iFrameRate = 0.0;
    testparam.iMediainputParam.iSamplingRate = 0;

    //checks authoring mode (-realtime).By default is ASAP mode
    testparam.iMediainputParam.iRealTimeAuthoring = FindAuthoringMode(aCommandLine, file);

    // Load video configuration
    if (FindAVIConfigFile(aCommandLine, configfileinfo, file))
    {
        LoadAVIConfiguration(configfileinfo, testparam, file);
    }

    //iAsap is used when we run testcases in one go i.e running TC 0 to TC 569
    if (testparam.iMediainputParam.iRealTimeAuthoring)
    {
        testparam.iAsap = false;
    }
    else
    {
        testparam.iAsap = true;
    }

    FindAuthoringTime(aCommandLine, testparam.iMediainputParam.iLoopTime, file);
    if ((PVMediaInput_ErrorHandling_Test_WrongFormat != aFirstTest)
            && (PVMediaInput_ErrorHandling_Test_WrongIPFileName != aFirstTest))
    {
        fprintf(file, "Begin test with the following parameters:\
                \nInput File Name : %s\nOutput File Name: %s,\nVideo Encoder: %s,\nAudio Encoder: %s,\
                \nComposer: %s\n", testparam.iMediainputParam.iIPFileInfo.get_cstr(), testparam.iMediainputParam.iOPFileInfo.get_cstr(),
                testparam.iMediainputParam.iVideoEncInfo.get_cstr(), testparam.iMediainputParam.iAudioEncInfo.get_cstr(),
                testparam.iMediainputParam.iComposerInfo.get_cstr());
    }

    OSCL_TRY(err,
             PVMediaInputAuthorEngineTestSuite* test_suite =
                 new PVMediaInputAuthorEngineTestSuite(testparam);
             test_suite->run_test();
             text_test_interpreter interp;
             _STRING rs = interp.interpretation(test_suite->last_result());
             fprintf(file, rs.c_str());
             const test_result the_result = test_suite->last_result();
             retVal = (int)(the_result.success_count() != the_result.total_test_count());

             delete test_suite;
             test_suite = NULL;
            );
    if (err != OSCL_ERR_NONE)
    {
        fprintf(file, "ERROR: Leave Occurred! Reason %d \n", err);
        return 1;
    }
    return retVal;
}



int _local_main(FILE *filehandle, cmd_line *command_line)
{
    int retVal = 1;
    file = filehandle;

    // Print out the extension for help if no argument
    int32 firsttest, lasttest;
    if (command_line->get_count() == 0)
    {
        fprintf(file, "****Specify '-help' to get CommandLine arguments information options****\n\n");
        fprintf(file, "****Running all Author test cases****\n\n");
        //return 0;
        firsttest = 0;
        lasttest = KUnCompressed_Errorhandling_TestEnd;
    }
    else
    {
        FindTestRange(command_line, firsttest, lasttest, file);
        fprintf(file, "[test range from: %d to: %d]\n\n", firsttest, lasttest);
    }

    PVMFFormatType formaterr = PVMF_MIME_FORMAT_UNKNOWN;
    int32 err = 0;

    OSCL_HeapString<OsclMemAllocator> filenameinfo;

    if (Invalid_Test == firsttest)
    {
        //functions called to print command line arguments.
        fprintf(file, "CMD LINE ARGS FOR COMPRESSED TESTS [test range from: %d to: %d]\n\n", AMR_Input_AOnly_3gpTest, Compressed_LongetivityTestEnd);

        FindAudioSourceFile(command_line, filenameinfo, file);
        FindVideoSourceFile(command_line, filenameinfo, file);
        FindOutputFile(command_line, filenameinfo, file);
        FindAuthoringTime(command_line, (uint32&)err, file);

        fprintf(file, "CMD LINE ARGS FOR UNCOMPRESSED TESTS(with AVI/WAV inputs)[test range from %d to %d]\n\n", UnCompressed_NormalTestBegin, UnCompressed_LongetivityTestEnd);

        FindSourceFile(command_line, filenameinfo, (PVMFFormatType&)formaterr, file);
        FindOutputFile(command_line, filenameinfo, file);
        FindVideoEncoder(command_line, filenameinfo, file);
        FindAudioEncoder(command_line, filenameinfo, file);
        FindAuthoringTime(command_line, (uint32&)err, file);

        fprintf(file, "NO CMD LINE ARGS WERE REQUIRED TO RUN COMPRESSED ERROR HANDLING TESTS [test range from:%d to %d]\n\n", KCompressed_Errorhandling_TestBegin, KCompressed_Errorhandling_TestEnd);
        fprintf(file, "NO CMD LINE ARGS WERE REQUIRED TO RUN UNCOMPRESSED ERROR HANDLING TESTS [test range from:%d to %d]\n\n", KUnCompressed_Errorhandling_TestBegin, KUnCompressed_Errorhandling_TestEnd);

        return 0;
    }

    ///////////////////////Normal Compressed tests//////////////////////
    if (firsttest <= Compressed_LongetivityTestEnd)
    {
        retVal = RunCompressedTest(command_line, firsttest, lasttest, file);
    }
    ///////End of Normal Compressed tests///////////////////////////////////////

    ///////////////Uncompressed AVI normal and longetivity tests///////////////
    if (((firsttest >= UnCompressed_NormalTestBegin) && (firsttest <= UnCompressed_LongetivityTestEnd))
            || ((firsttest <= UnCompressed_NormalTestBegin) && (lasttest > UnCompressed_NormalTestBegin)))
    {
        retVal = RunUnCompressedTest(command_line, firsttest, lasttest, file);
    }
    //////////////////End of AVI normal and longetivity tests////////////////////

    //////////////////Compressed Errorhandling test begin/////////////////////
    if (((firsttest >= KCompressed_Errorhandling_TestBegin) && (firsttest <= KCompressed_Errorhandling_TestEnd))
            || ((firsttest <= KCompressed_Errorhandling_TestBegin) && (lasttest > KCompressed_Errorhandling_TestBegin)))
    {

        if (firsttest < KCompressed_Errorhandling_TestBegin)
        {
            firsttest = KCompressed_Errorhandling_TestBegin;
        }

        retVal = RunCompressedTest(command_line, firsttest, lasttest, file);

    }//////////////////Compressed Errorhandling test end/////////////////////

    //////////////////UnCompressed Errorhandling test begin/////////////////////
    if (((lasttest > KUnCompressed_Errorhandling_TestBegin) && (lasttest <= KUnCompressed_Errorhandling_TestEnd))
            || (((firsttest <= KUnCompressed_Errorhandling_TestBegin) || (firsttest >= KUnCompressed_Errorhandling_TestBegin))
                && (lasttest > KUnCompressed_Errorhandling_TestEnd)))
    {
        if (firsttest < KUnCompressed_Errorhandling_TestBegin)
        {
            firsttest = KUnCompressed_Errorhandling_TestBegin;
        }
        retVal = RunUnCompressedTest(command_line, firsttest, lasttest, file);

    }
    return retVal;
}

