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
#ifndef TEST_PV_MEDIAINPUT_AUTHOR_ENGINE_H
#include "test_pv_mediainput_author_engine.h"
#endif

#ifndef TEST_PV_AUTHOR_ENGINE_TESTSET5_H_INCLUDED
#include "test_pv_author_engine_testset5.h"
#endif

#ifndef TEST_PV_AUTHOR_ENGINE_TESTSET6_H_INCLUDED
#include "test_pv_author_engine_testset6.h"
#endif

#ifndef TEST_PV_AUTHOR_ENGINE_TESTSET7_H_INCLUDED
#include "test_pv_author_engine_testset7.h"
#endif

#ifndef TEST_PV_AUTHOR_ENGINE_TESTSET8_H_INCLUDED
#include "test_pv_author_engine_testset8.h"
#endif

#define DEFAULT_LOOPTIME 60 //secs
bool PVMediaInputAuthorEngineTest::Set_Default_Params(int32 aTestnum, PVMediaInputTestParam& aMediaInputParam)
{
    aMediaInputParam.iInputFormat = DEFAULTSOURCEFORMATTYPE;

    if (iAsap)
    {
        aMediaInputParam.iRealTimeAuthoring = false;
    }

    bool retval = true;
    switch (aTestnum)
    {
        case CapConfigTest:
        {
            aMediaInputParam.iIPFileInfo = KPCM_YUV_AVI_FILENAME;
            aMediaInputParam.iOPFileInfo = KCAPCONFIG_OUTPUT;
        }
        break;

        case PVMediaInput_Open_Compose_Stop_Test:
        case PVMediaInput_Open_Compose_Stop_Test_UsingExternalFileHandle:
        {
            if ((iFirstTest != iLastTest) || (aMediaInputParam.iIPFileInfo.get_size() == 0))
            {
                aMediaInputParam.iIPFileInfo = DEFAULTSOURCEFILENAME;
            }
            if ((iFirstTest != iLastTest) || (aMediaInputParam.iOPFileInfo.get_size() == 0))
            {
                aMediaInputParam.iOPFileInfo = DEFAULTOUTPUTFILENAME;
            }
        }
        break;

        case ErrorHandling_VideoInitFailed:
        case ErrorHandling_VideoEncodeFailed:
        case ErrorHandling_VideoEncode5FramesFailed:
        case ErrorHandling_ComposerAddFragFailed:
        case ErrorHandling_ComposerAddTrackFailed:
        case ErrorHandling_MediaInputNodeStopFailed:
        case ErrorHandling_AudioInitFailed:
        case ErrorHandling_AudioEncodeFailed:
        case ErrorHandling_MediaInputNode_NoMemBuffer:
        case ErrorHandling_MediaInputNode_large_time_stamp:
        case ErrorHandling_MediaInputNode_wrong_time_stamp_after_duration:
        case ErrorHandling_MediaInputNode_zero_time_stamp:
        case ErrorHandling_MediaInputNode_StateFailure_EPause_SendMIORequest:
        case ErrorHandling_MediaInputNode_StateFailure_CancelMIORequest:
        case ErrorHandling_MediaInputNode_Corrupt_Video_InputData:
        case ErrorHandling_MediaInputNode_Corrupt_Audio_InputData:
        case ErrorHandling_MediaInputNode_Node_Cmd_Start:
        case ErrorHandling_MediaInputNode_Node_Cmd_Stop:
        case ErrorHandling_MediaInputNode_Node_Cmd_Flush:
        case ErrorHandling_MediaInputNode_Node_Cmd_Pause:
        case ErrorHandling_MediaInputNode_Node_Cmd_ReleasePort:
        case ErrorHandling_MediaInputNode_DataPath_Stall:
        case ErrorHandling_MP4Composer_AddTrack_PVMF_AMR_IETF:
        case ErrorHandling_MP4Composer_AddTrack_PVMF_3GPP_TIMEDTEXT:
        case ErrorHandling_MP4Composer_AddTrack_PVMF_M4V:
        case ErrorHandling_MP4Composer_AddTrack_PVMF_H263:
        case ErrorHandling_MP4Composer_AddTrack_PVMF_H264_MP4:
        case ErrorHandling_MP4Composer_Node_Cmd_Start:
        case ErrorHandling_MP4Composer_Node_Cmd_Stop:
        case ErrorHandling_MP4Composer_Node_Cmd_Flush:
        case ErrorHandling_MP4Composer_Node_Cmd_Pause:
        case ErrorHandling_MP4Composer_Node_Cmd_ReleasePort:
        case ErrorHandling_MP4Composer_Create_FileParser:
        case ErrorHandling_MP4Composer_RenderToFile:
        case ErrorHandling_MP4Composer_FailAfter_FileSize:
        case ErrorHandling_MP4Composer_FailAfter_Duration:
        case ErrorHandling_MP4Composer_DataPathStall:
        case ErrorHandling_VideoEncodeNode_Node_Cmd_Start:
        case ErrorHandling_VideoEncodeNode_Node_Cmd_Stop:
        case ErrorHandling_VideoEncodeNode_Node_Cmd_Flush:
        case ErrorHandling_VideoEncodeNode_Node_Cmd_Pause:
        case ErrorHandling_VideoEncodeNode_Node_Cmd_ReleasePort:
        case ErrorHandling_VideoEncodeNode_ConfigHeader:
        case ErrorHandling_VideoEncodeNode_DataPathStall_Before_ProcessingData:
        case ErrorHandling_VideoEncodeNode_DataPathStall_After_ProcessingData:
        case ErrorHandling_VideoEncodeNode_FailEncode_AfterDuration:
        case ErrorHandling_AudioEncodeNode_Node_Cmd_Start:
        case ErrorHandling_AudioEncodeNode_Node_Cmd_Stop:
        case ErrorHandling_AudioEncodeNode_Node_Cmd_Flush:
        case ErrorHandling_AudioEncodeNode_Node_Cmd_Pause:
        case ErrorHandling_AudioEncodeNode_Node_Cmd_ReleasePort:
        case ErrorHandling_AudioEncodeNode_FailEncode_AfterDuration:
        case ErrorHandling_AudioEncodeNode_DataPathStall_Before_ProcessingData:
        case ErrorHandling_AudioEncodeNode_DataPathStall_After_ProcessingData:
        case ErrorHandling_MediaInputNode_Out_Queue_busy:
        {
            aMediaInputParam.iIPFileInfo = DEFAULTSOURCEFILENAME;
            aMediaInputParam.iOPFileInfo = DEFAULTOUTPUTFILENAME;
            aMediaInputParam.iRealTimeAuthoring = false;
        }
        break;
        case ErrorHandling_AVCEncodeNode_Node_Cmd_Start:
        case ErrorHandling_AVCEncodeNode_Node_Cmd_Stop:
        case ErrorHandling_AVCEncodeNode_Node_Cmd_Flush:
        case ErrorHandling_AVCEncodeNode_Node_Cmd_Pause:
        case ErrorHandling_AVCEncodeNode_Node_Cmd_ReleasePort:
        case ErrorHandling_AVCEncodeNode_ConfigHeader:
        case ErrorHandling_AVCEncodeNode_DataPathStall_Before_ProcessingData:
        case ErrorHandling_AVCEncodeNode_DataPathStall_After_ProcessingData:
        case ErrorHandling_AVCEncodeNode_FailEncode_AfterDuration:
        case ErrorHandling_AVCVideoEncodeFailed:
        case ErrorHandling_AVCVideoEncode5FramesFailed:
        {
            aMediaInputParam.iIPFileInfo = DEFAULTSOURCEFILENAME;
            aMediaInputParam.iOPFileInfo = DEFAULTOUTPUTFILENAME;
            aMediaInputParam.iVideoEncInfo  = KH264EncMimeType;
            aMediaInputParam.iRealTimeAuthoring = false;

        }
        break;
        case PVMediaInput_Open_RealTimeCompose_Stop_Test:
        {
            aMediaInputParam.iIPFileInfo = DEFAULTSOURCEFILENAME;
            aMediaInputParam.iRealTimeAuthoring = true;
            aMediaInputParam.iOPFileInfo = KMediaInput_Open_RealTimeCompose_Output;
        }
        break;
        case PVMediaInput_Pause_Resume_Test:
        case PVMediaInput_Reset_After_Create_Test:
        case PVMediaInput_Reset_After_Open_Test:
        case PVMediaInput_Reset_After_AddDataSource_Test:
        case PVMediaInput_Reset_After_SelectComposer_Test:
        case PVMediaInput_Reset_After_AddMediaTrack_Test:
        case PVMediaInput_Reset_After_Init_Test:
        case PVMediaInput_Reset_After_Start_Test:
        case PVMediaInput_Reset_After_Pause_Test:
        case PVMediaInput_Reset_After_Recording_Test:
        case PVMediaInput_Reset_After_Stop_Test:

        case PVMediaInput_Delete_After_Create_Test:
        case PVMediaInput_Delete_After_Open_Test:
        case PVMediaInput_Delete_After_AddDataSource_Test:
        case PVMediaInput_Delete_After_SelectComposer_Test:
        case PVMediaInput_Delete_After_AddMediaTrack_Test:
        case PVMediaInput_Delete_After_Init_Test:
        case PVMediaInput_Delete_After_Start_Test:
        case PVMediaInput_Delete_After_Pause_Test:
        case PVMediaInput_Delete_After_Recording_Test:
        case PVMediaInput_Delete_After_Stop_Test:

        {
            aMediaInputParam.iIPFileInfo = DEFAULTSOURCEFILENAME;
            aMediaInputParam.iOPFileInfo = DEFAULTOUTPUTFILENAME;
        }
        break;
        case AVI_Input_Longetivity_Test:
        {
            if ((iFirstTest != iLastTest) || (aMediaInputParam.iIPFileInfo.get_size() == 0))
            {
                aMediaInputParam.iIPFileInfo = DEFAULTSOURCEFILENAME;
            }
            if ((iFirstTest != iLastTest) || (aMediaInputParam.iOPFileInfo.get_size() == 0))
            {
                aMediaInputParam.iOPFileInfo = KAVI_Input_Long_OUTPUT;
            }
            if (0 == aMediaInputParam.iLoopTime)
            {
                aMediaInputParam.iLoopTime = DEFAULT_LOOPTIME;
            }
        }
        break;

        case YUV_Input_VOnly_3gpTest:
        {
            aMediaInputParam.iIPFileInfo = KYUV_AVI_VIDEO_ONLY;
            aMediaInputParam.iOPFileInfo = KYUV_VOnly_3GPP_Output;
        }
        break;

        case H264_Input_VOnly_3gpTest:
        {
            aMediaInputParam.iIPFileInfo = KYUV_AVI_VIDEO_ONLY;
            aMediaInputParam.iOPFileInfo = KH264_VOnly_MP4_Output;
        }
        break;

        case PCM16_Input_AOnly_3gpTest:
        {
            aMediaInputParam.iIPFileInfo = KPCM_AVI_AUDIO_ONLY;
            aMediaInputParam.iInputFormat = PVMF_MIME_WAVFF;
            aMediaInputParam.iOPFileInfo = KPCM_AOnly_3GPP_Output;
        }
        break;

        case PCM16In_AMROut_Test:
        {
            aMediaInputParam.iIPFileInfo = KPCM_AVI_AUDIO_ONLY;
            aMediaInputParam.iOPFileInfo = KPCM16_AMR_TestOutput;
            aMediaInputParam.iInputFormat = PVMF_MIME_WAVFF;
        }
        break;

        case PCM16_YUV_Input_AV_3gpTest:
        {
            aMediaInputParam.iIPFileInfo = KPCM_YUV_AVI_FILENAME;
            aMediaInputParam.iOPFileInfo = KPCM_YUV_AV_3GPP_Output;
        }
        break;

        case KMaxFileSizeTest:
        {
            aMediaInputParam.iIPFileInfo = KPCM_YUV_AVI_FILENAME;
            aMediaInputParam.iOPFileInfo = KMaxFileSizeTestOutput;
        }
        break;

        case KMaxFileSizeLongetivityTest:
        {
            if ((iFirstTest != iLastTest) || (aMediaInputParam.iIPFileInfo.get_size() == 0))
            {
                aMediaInputParam.iIPFileInfo = KPCM_YUV_AVI_FILENAME;
            }
            if ((iFirstTest != iLastTest) || (aMediaInputParam.iOPFileInfo.get_size() == 0))
            {
                aMediaInputParam.iOPFileInfo = KMaxFileSize_long_Output;
            }
            if (0 == aMediaInputParam.iLoopTime)
            {
                aMediaInputParam.iLoopTime = DEFAULT_LOOPTIME;
            }
        }
        break;


        case K3GPPDownloadModeTest:
        {
            aMediaInputParam.iIPFileInfo = KPCM_YUV_AVI_FILENAME;
            aMediaInputParam.iOPFileInfo = K3GPPDOWNLOADMODE_OUTPUT;
        }
        break;

        case K3GPPDownloadModeLongetivityTest:
        {
            if ((iFirstTest != iLastTest) || (aMediaInputParam.iIPFileInfo.get_size() == 0))
            {
                aMediaInputParam.iIPFileInfo = KPCM_YUV_AVI_FILENAME;
            }
            if ((iFirstTest != iLastTest) || (aMediaInputParam.iOPFileInfo.get_size() == 0))
            {
                aMediaInputParam.iOPFileInfo = K3GPPDOWNLOADMODE_Long_OUTPUT;
            }
            if (0 == aMediaInputParam.iLoopTime)
            {
                aMediaInputParam.iLoopTime = DEFAULT_LOOPTIME;
            }
        }
        break;

        case K3GPPProgressiveDownloadModeTest:
        {
            aMediaInputParam.iIPFileInfo = KPCM_YUV_AVI_FILENAME;
            aMediaInputParam.iOPFileInfo = K3GPPPROGRESSIVEDOWNLOADMODE_OUTPUT;
        }
        break;

        case K3GPPProgressiveDownloadModeLongetivityTest:
        {
            if ((iFirstTest != iLastTest) || (aMediaInputParam.iIPFileInfo.get_size() == 0))
            {
                aMediaInputParam.iIPFileInfo = KPCM_YUV_AVI_FILENAME;
            }
            if ((iFirstTest != iLastTest) || (aMediaInputParam.iOPFileInfo.get_size() == 0))
            {
                aMediaInputParam.iOPFileInfo = K3GPPPROGRESSIVEDOWNLOADMODE_Long_OUTPUT;
            }
            if (0 == aMediaInputParam.iLoopTime)
            {
                aMediaInputParam.iLoopTime = DEFAULT_LOOPTIME;
            }
        }
        break;

        case KMovieFragmentModeTest:
        {
            aMediaInputParam.iIPFileInfo = KPCM_YUV_AVI_FILENAME;
            aMediaInputParam.iOPFileInfo = KMOVIEFRAGMENTMODE_OUTPUT;
        }
        break;

        case KMovieFragmentModeLongetivityTest:
        {
            if ((iFirstTest != iLastTest) || (aMediaInputParam.iIPFileInfo.get_size() == 0))
            {
                aMediaInputParam.iIPFileInfo = KPCM_YUV_AVI_FILENAME;
            }
            if ((iFirstTest != iLastTest) || (aMediaInputParam.iOPFileInfo.get_size() == 0))
            {
                aMediaInputParam.iOPFileInfo = KMOVIEFRAGMENTMODE_Long_OUTPUT;
            }
            if (0 == aMediaInputParam.iLoopTime)
            {
                aMediaInputParam.iLoopTime = DEFAULT_LOOPTIME;
            }
        }
        break;

        case PVMediaInput_ErrorHandling_Test_WrongFormat:
        {
            aMediaInputParam.iIPFileInfo = FILE_NAME_ERROR_HANDLING;
            aMediaInputParam.iInputFormat = DEFAULTSOURCEFORMATTYPE;
        }
        break;

        case PVMediaInput_ErrorHandling_Test_WrongIPFileName:
        {
            aMediaInputParam.iIPFileInfo = WRONGIPFILENAME_ERRORHANDLING;
            aMediaInputParam.iInputFormat = DEFAULTSOURCEFORMATTYPE;
        }
        break;

        case ErrorHandling_WrongOutputPathTest:
        {
            aMediaInputParam.iIPFileInfo = DEFAULTSOURCEFILENAME;
            aMediaInputParam.iOPFileInfo = WRONGOPFILENAME_ERRORHANDLING;
        }
        break;
        default:
            retval = false;
            break;
    }
    return retval;
}

void PVMediaInputAuthorEngineTest::test()
{
    iTotalSuccess = 0;
    iTotalFail = 0;
    iTotalError = 0;
    PVAECmdType resetState = PVAE_CMD_CREATE;

    if (iNextTestCase < UnCompressed_NormalTestBegin)
    {
        iNextTestCase = UnCompressed_NormalTestBegin;
    }

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
                    fprintf(iFile, "  Mem stats: TotalAllocs(%d), TotalBytes(%d),\n AllocFailures(%d), AllocLeak(%d)\n",
                            stats->totalNumAllocs - iTotalAlloc, stats->totalNumBytes - iTotalBytes, stats->numAllocFails - iAllocFails, stats->numAllocs - iNumAllocs);
                }
                else
                {
                    fprintf(iFile, "Retrieving memory statistics after running test case failed! Memory statistics result is not available.\n");
                }
            }
            else
            {
                fprintf(iFile, "Memory audit not available! Memory statistics result is not available.\n");
            }
#endif

        }   //iCurrentTest

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
                fprintf(iFile, "Retrieving memory statistics before running test case failed! Memory statistics result would be invalid.\n");
            }
        }
        else
        {
            fprintf(iFile, "Memory audit not available! Memory statistics result would be invalid.\n");
        }
#endif
        if (iNextTestCase > iLastTest)
        {
            iNextTestCase = Invalid_Test;
        }
        else
        {
            if ((iNextTestCase >= UnCompressed_NormalTestEnd && iNextTestCase <= UnCompressed_LongetivityTestBegin) && (iLastTest >= UnCompressed_LongetivityTestBegin))
            {
                iNextTestCase = UnCompressed_LongetivityTestBegin;
                iNextTestCase++;
            }

            if ((UnCompressed_NormalTestBegin == iNextTestCase) || (UnCompressed_LongetivityTestBegin == iNextTestCase)
                    || (KUnCompressed_Errorhandling_TestBegin == iNextTestCase)  || (UnCompressed_NormalTestEnd == iNextTestCase))
            {
                fprintf(iFile, "\nPlace Holder Not actual testcase %d: ", iNextTestCase);
                iNextTestCase++;
            }

            if ((iNextTestCase > UnCompressed_NormalTestEnd) && (iLastTest < UnCompressed_LongetivityTestBegin))
            {
                iNextTestCase = Invalid_Test;
            }//stop at last test
            else if ((UnCompressed_LongetivityTestEnd == iNextTestCase) || (KUnCompressed_Errorhandling_TestEnd == iNextTestCase))
            {
                fprintf(iFile, "\nPlace Holder Not actual testcase %d: ", iNextTestCase);
                iNextTestCase = Invalid_Test;
            }
            else
            {
                fprintf(iFile, "\nStarting Test %d: ", iNextTestCase);
                InitLoggerScheduler();
            }
        }

        PVAuthorAsyncTestParam testparam;
        testparam.iObserver = this;
        testparam.iTestCase = this;
        testparam.iTestCaseNum = iNextTestCase;
        testparam.iStdOut = iFile;
        if (!Set_Default_Params(iNextTestCase, iMediaInputParam) && (Invalid_Test != iNextTestCase))
        {
            fprintf(iFile, "\nNo Default Parameters specified for test case:%d\n", iNextTestCase);
        }

        switch (iNextTestCase)
        {
            case PVMediaInput_Open_Compose_Stop_Test:
            case PVMediaInput_Open_RealTimeCompose_Stop_Test:
            case YUV_Input_VOnly_3gpTest:
            case H264_Input_VOnly_3gpTest:
            case PCM16_Input_AOnly_3gpTest:
            case PCM16_YUV_Input_AV_3gpTest:
            case PCM16In_AMROut_Test:
            case KMaxFileSizeTest:

            case K3GPPDownloadModeTest:
            case K3GPPProgressiveDownloadModeTest:
            case CapConfigTest:
            case AVI_Input_Longetivity_Test:
            case KMaxFileSizeLongetivityTest:
            case K3GPPDownloadModeLongetivityTest:
            case K3GPPProgressiveDownloadModeLongetivityTest:
            case KMovieFragmentModeLongetivityTest:
            {
                Print_TestCase_Name(iNextTestCase);
                iCurrentTest = new pv_mediainput_async_test_opencomposestop(testparam, iMediaInputParam, false);
                // Go to next test
                ++iNextTestCase;
            }
            break;
            case PVMediaInput_Open_Compose_Stop_Test_UsingExternalFileHandle:
            {
                Print_TestCase_Name(iNextTestCase);
                iCurrentTest = new pv_mediainput_async_test_opencomposestop(testparam, iMediaInputParam, false, true);
                // Go to next test
                ++iNextTestCase;
            }
            break;
            case PVMediaInput_Pause_Resume_Test:
            {
                fprintf(iFile, "Pause Resume test with AVI/WAV MIO Comp:\n");
                iCurrentTest = new pv_mediainput_async_test_opencomposestop(testparam, iMediaInputParam, true);
                // Go to next test
                ++iNextTestCase;
            }
            break;
            case PVMediaInput_ErrorHandling_Test_WrongFormat:
            {
                fprintf(iFile, "Error Handling Wrong Format test with AVI/WAV MIO Comp\n");
                iCurrentTest = new pv_mediainput_async_test_errorhandling(testparam, iMediaInputParam, false, iNextTestCase);

                // Go to next test
                ++iNextTestCase;
            }
            break;

            case PVMediaInput_ErrorHandling_Test_WrongIPFileName:
            case ErrorHandling_WrongOutputPathTest:
            {
                Print_TestCase_Name(iNextTestCase);
                iCurrentTest = new pv_mediainput_async_test_errorhandling(testparam, iMediaInputParam, false, iNextTestCase);

                // Go to next test
                ++iNextTestCase;
            }
            break;

            case ErrorHandling_AVCVideoEncodeFailed:
            case ErrorHandling_AVCVideoEncode5FramesFailed:
            {
                iMediaInputParam.iVideoEncInfo  = KH264EncMimeType;
            }
            case ErrorHandling_VideoInitFailed:
            case ErrorHandling_VideoEncodeFailed:
            case ErrorHandling_VideoEncode5FramesFailed:
            case ErrorHandling_ComposerAddFragFailed:
            case ErrorHandling_ComposerAddTrackFailed:
            case ErrorHandling_MediaInputNodeStopFailed:
            case ErrorHandling_AudioInitFailed:
            case ErrorHandling_AudioEncodeFailed:
            case ErrorHandling_MediaInputNode_NoMemBuffer:
            case ErrorHandling_MediaInputNode_Out_Queue_busy:
            case ErrorHandling_MediaInputNode_large_time_stamp:
            case ErrorHandling_MediaInputNode_wrong_time_stamp_after_duration:
            case ErrorHandling_MediaInputNode_zero_time_stamp:
            case ErrorHandling_MediaInputNode_StateFailure_EPause_SendMIORequest:
            case ErrorHandling_MediaInputNode_StateFailure_CancelMIORequest:
            case ErrorHandling_MediaInputNode_Corrupt_Video_InputData:
            case ErrorHandling_MediaInputNode_Corrupt_Audio_InputData:
            case ErrorHandling_MediaInputNode_Node_Cmd_Start:
            case ErrorHandling_MediaInputNode_Node_Cmd_Stop:
            case ErrorHandling_MediaInputNode_Node_Cmd_Flush:
            case ErrorHandling_MediaInputNode_Node_Cmd_Pause:
            case ErrorHandling_MediaInputNode_Node_Cmd_ReleasePort:
            case ErrorHandling_MediaInputNode_DataPath_Stall:
            case ErrorHandling_MP4Composer_AddTrack_PVMF_AMR_IETF:
            case ErrorHandling_MP4Composer_AddTrack_PVMF_3GPP_TIMEDTEXT:
            case ErrorHandling_MP4Composer_AddTrack_PVMF_M4V:
            case ErrorHandling_MP4Composer_AddTrack_PVMF_H263:
            case ErrorHandling_MP4Composer_AddTrack_PVMF_H264_MP4:
            case ErrorHandling_MP4Composer_Node_Cmd_Start:
            case ErrorHandling_MP4Composer_Node_Cmd_Stop:
            case ErrorHandling_MP4Composer_Node_Cmd_Flush:
            case ErrorHandling_MP4Composer_Node_Cmd_Pause:
            case ErrorHandling_MP4Composer_Node_Cmd_ReleasePort:
            case ErrorHandling_MP4Composer_Create_FileParser:
            case ErrorHandling_MP4Composer_RenderToFile:
            case ErrorHandling_MP4Composer_FailAfter_FileSize:
            case ErrorHandling_MP4Composer_FailAfter_Duration:
            case ErrorHandling_MP4Composer_DataPathStall:
            case ErrorHandling_VideoEncodeNode_Node_Cmd_Start:
            case ErrorHandling_VideoEncodeNode_Node_Cmd_Stop:
            case ErrorHandling_VideoEncodeNode_Node_Cmd_Flush:
            case ErrorHandling_VideoEncodeNode_Node_Cmd_Pause:
            case ErrorHandling_VideoEncodeNode_Node_Cmd_ReleasePort:
            case ErrorHandling_VideoEncodeNode_ConfigHeader:
            case ErrorHandling_VideoEncodeNode_DataPathStall_Before_ProcessingData:
            case ErrorHandling_VideoEncodeNode_DataPathStall_After_ProcessingData:
            case ErrorHandling_VideoEncodeNode_FailEncode_AfterDuration:
            case ErrorHandling_AudioEncodeNode_FailEncode_AfterDuration:
            case ErrorHandling_AudioEncodeNode_DataPathStall_Before_ProcessingData:
            case ErrorHandling_AudioEncodeNode_DataPathStall_After_ProcessingData:
            case ErrorHandling_AVCEncodeNode_Node_Cmd_Start:
            case ErrorHandling_AVCEncodeNode_Node_Cmd_Stop:
            case ErrorHandling_AVCEncodeNode_Node_Cmd_Flush:
            case ErrorHandling_AVCEncodeNode_Node_Cmd_Pause:
            case ErrorHandling_AVCEncodeNode_Node_Cmd_ReleasePort:
            case ErrorHandling_AudioEncodeNode_Node_Cmd_Start:
            case ErrorHandling_AudioEncodeNode_Node_Cmd_Stop:
            case ErrorHandling_AudioEncodeNode_Node_Cmd_Flush:
            case ErrorHandling_AudioEncodeNode_Node_Cmd_Pause:
            case ErrorHandling_AudioEncodeNode_Node_Cmd_ReleasePort:
            case ErrorHandling_AVCEncodeNode_ConfigHeader:
            case ErrorHandling_AVCEncodeNode_DataPathStall_Before_ProcessingData:
            case ErrorHandling_AVCEncodeNode_DataPathStall_After_ProcessingData:
            case ErrorHandling_AVCEncodeNode_FailEncode_AfterDuration:

#ifndef _TEST_AE_ERROR_HANDLING
                fprintf(iFile, " AE Error Handling Test Not implemented\n");
                iCurrentTest = NULL;
                break;
#else
                {

                    //fprintf(iFile, "\nCurrently executing test:%d\n\n", iNextTestCase);
                    fprintf(iFile, "Begin test with the following parameters:\
				\nInput File Name : %s\nOutput File Name: %s,\nVideo Encoder: %s,\nAudio Encoder: %s,\
				\nComposer: %s\n", iMediaInputParam.iIPFileInfo.get_cstr(), iMediaInputParam.iOPFileInfo.get_cstr(),
                    iMediaInputParam.iVideoEncInfo.get_cstr(), iMediaInputParam.iAudioEncInfo.get_cstr(),
                    iMediaInputParam.iComposerInfo.get_cstr());


                    //fprintf(iFile, "\nCurrently executing test:%d\n\n", iNextTestCase);
                    Print_TestCase_Name(iNextTestCase);

                    iCurrentTest = new pv_mediainput_async_test_errorhandling(testparam, iMediaInputParam, false, iNextTestCase);

                    // Go to next test
                    ++iNextTestCase;

                }
                break;
#endif
            case PVMediaInput_Reset_After_Create_Test:
            {
                fprintf(iFile, "Reset After Create test with AVI/WAV MIO Comp\n");
                resetState = PVAE_CMD_CREATE;
                iCurrentTest = new pv_mediainput_async_test_reset(testparam, iMediaInputParam, false, resetState);
                // Go to next test
                ++iNextTestCase;
            }
            break;
            case PVMediaInput_Reset_After_Open_Test:
            {
                fprintf(iFile, "Reset After Open test with AVI/WAV MIO Comp\n");
                resetState = PVAE_CMD_OPEN;
                iCurrentTest = new pv_mediainput_async_test_reset(testparam, iMediaInputParam, false, resetState);
                // Go to next test
                ++iNextTestCase;
            }
            break;
            case PVMediaInput_Reset_After_AddDataSource_Test:
            {
                fprintf(iFile, "Reset After AddDataSource test with AVI/WAV MIO Comp\n");
                resetState = PVAE_CMD_ADD_DATA_SOURCE;
                iCurrentTest = new pv_mediainput_async_test_reset(testparam, iMediaInputParam, false, resetState);
                // Go to next test
                ++iNextTestCase;
            }
            break;
            case PVMediaInput_Reset_After_SelectComposer_Test:
            {
                fprintf(iFile, "Reset After SelectComposer test with AVI/WAV MIO Comp\n");
                resetState = PVAE_CMD_SELECT_COMPOSER;
                iCurrentTest = new pv_mediainput_async_test_reset(testparam, iMediaInputParam, false, resetState);
                // Go to next test
                ++iNextTestCase;
            }
            break;
            case PVMediaInput_Reset_After_AddMediaTrack_Test:
            {
                fprintf(iFile, "Reset After AddMediaTrack test with AVI/WAV MIO Comp\n");
                resetState = PVAE_CMD_ADD_MEDIA_TRACK;
                iCurrentTest = new pv_mediainput_async_test_reset(testparam, iMediaInputParam, false, resetState);
                // Go to next test
                ++iNextTestCase;
            }
            break;
            case PVMediaInput_Reset_After_Init_Test:
            {
                fprintf(iFile, "Reset After Init test with AVI/WAV MIO Comp\n");
                resetState = PVAE_CMD_INIT;
                iCurrentTest = new pv_mediainput_async_test_reset(testparam, iMediaInputParam, false, resetState);
                // Go to next test
                ++iNextTestCase;
            }
            break;
            case PVMediaInput_Reset_After_Start_Test:
            {
                fprintf(iFile, "Reset After Start test with AVI/WAV MIO Comp\n");
                resetState = PVAE_CMD_START;
                iCurrentTest = new pv_mediainput_async_test_reset(testparam, iMediaInputParam, false, resetState);
                // Go to next test
                ++iNextTestCase;
            }
            break;
            case PVMediaInput_Reset_After_Pause_Test:
            {
                fprintf(iFile, "Reset After Pause test with AVI/WAV MIO Comp\n");
                resetState = PVAE_CMD_PAUSE;
                iCurrentTest = new pv_mediainput_async_test_reset(testparam, iMediaInputParam, false, resetState);
                // Go to next test
                ++iNextTestCase;
            }
            break;
            case PVMediaInput_Reset_After_Recording_Test:
            {
                fprintf(iFile, "Reset After Recording test with AVI/WAV MIO Comp\n");
                resetState = PVAE_CMD_RECORDING;
                iCurrentTest = new pv_mediainput_async_test_reset(testparam, iMediaInputParam, false, resetState);
                // Go to next test
                ++iNextTestCase;
            }
            break;
            case PVMediaInput_Reset_After_Stop_Test:
            {
                fprintf(iFile, "Reset After Stop test with AVI/WAV MIO Comp\n");
                resetState = PVAE_CMD_STOP;
                iCurrentTest = new pv_mediainput_async_test_reset(testparam, iMediaInputParam, false, resetState);
                // Go to next test
                ++iNextTestCase;
            }
            break;

            case PVMediaInput_Delete_After_Create_Test:
            {
                fprintf(iFile, "Delete After Create test with AVI/WAV MIO Comp\n");
                resetState = PVAE_CMD_CREATE;
                iCurrentTest = new pv_mediainput_async_test_delete(testparam, iMediaInputParam, false, resetState);
                // Go to next test
                ++iNextTestCase;
            }
            break;
            case PVMediaInput_Delete_After_Open_Test:
            {
                fprintf(iFile, "Delete After Open test with AVI/WAV MIO Comp\n");
                resetState = PVAE_CMD_OPEN;
                iCurrentTest = new pv_mediainput_async_test_delete(testparam, iMediaInputParam, false, resetState);
                // Go to next test
                ++iNextTestCase;
            }
            break;
            case PVMediaInput_Delete_After_AddDataSource_Test:
            {
                fprintf(iFile, "Delete After AddDataSource test with AVI/WAV MIO Comp\n");
                resetState = PVAE_CMD_ADD_DATA_SOURCE;
                iCurrentTest = new pv_mediainput_async_test_delete(testparam, iMediaInputParam, false, resetState);
                // Go to next test
                ++iNextTestCase;
            }
            break;
            case PVMediaInput_Delete_After_SelectComposer_Test:
            {
                fprintf(iFile, "Delete After SelectComposer test with AVI/WAV MIO Comp\n");
                resetState = PVAE_CMD_SELECT_COMPOSER;
                iCurrentTest = new pv_mediainput_async_test_delete(testparam, iMediaInputParam, false, resetState);
                // Go to next test
                ++iNextTestCase;
            }
            break;
            case PVMediaInput_Delete_After_AddMediaTrack_Test:
            {
                fprintf(iFile, "Delete After AddMediaTrack test with AVI/WAV MIO Comp\n");
                resetState = PVAE_CMD_ADD_MEDIA_TRACK;
                iCurrentTest = new pv_mediainput_async_test_delete(testparam, iMediaInputParam, false, resetState);
                // Go to next test
                ++iNextTestCase;
            }
            break;
            case PVMediaInput_Delete_After_Init_Test:
            {
                fprintf(iFile, "Delete After Init test with AVI/WAV MIO Comp\n");
                resetState = PVAE_CMD_INIT;
                iCurrentTest = new pv_mediainput_async_test_delete(testparam, iMediaInputParam, false, resetState);
                // Go to next test
                ++iNextTestCase;
            }
            break;
            case PVMediaInput_Delete_After_Start_Test:
            {
                fprintf(iFile, "Delete After Start test with AVI/WAV MIO Comp\n");
                resetState = PVAE_CMD_START;
                iCurrentTest = new pv_mediainput_async_test_delete(testparam, iMediaInputParam, false, resetState);
                // Go to next test
                ++iNextTestCase;
            }
            break;
            case PVMediaInput_Delete_After_Pause_Test:
            {
                fprintf(iFile, "Delete After Pause test with AVI/WAV MIO Comp\n");
                resetState = PVAE_CMD_PAUSE;
                iCurrentTest = new pv_mediainput_async_test_delete(testparam, iMediaInputParam, false, resetState);
                // Go to next test
                ++iNextTestCase;
            }
            break;
            case PVMediaInput_Delete_After_Recording_Test:
            {
                fprintf(iFile, "Delete After Recording test with AVI/WAV MIO Comp\n");
                resetState = PVAE_CMD_RECORDING;
                iCurrentTest = new pv_mediainput_async_test_delete(testparam, iMediaInputParam, false, resetState);
                // Go to next test
                ++iNextTestCase;
            }
            break;
            case PVMediaInput_Delete_After_Stop_Test:
            {
                fprintf(iFile, "Delete After Stop test with AVI/WAV MIO Comp\n");
                resetState = PVAE_CMD_STOP;
                iCurrentTest = new pv_mediainput_async_test_delete(testparam, iMediaInputParam, false, resetState);
                // Go to next test
                ++iNextTestCase;
            }
            break;

            default:
            {
                iCurrentTest = NULL;
                break;
            }
        }

        if (iCurrentTest)
        {

            // Setup Scheduler
            OsclExecScheduler *sched = OsclExecScheduler::Current();
            if (sched)
            {
                int32 err;
                iCurrentTest->StartTest();

#if(USE_NATIVE_SCHEDULER)
                OSCL_TRY(err,
                         uint32 currticks  = 0;
                         currticks = OsclTickCount::TickCount();
                         uint32 starttime = OsclTickCount::TicksToMsec(currticks);

                         sched->StartNativeScheduler();

                         currticks = OsclTickCount::TickCount();
                         uint32 endtime = OsclTickCount::TicksToMsec(currticks);
                         fprintf(iFile, "  Time taken by the test:  %d\n", (endtime - starttime)););

#else
                OSCL_TRY(err,
                         uint32 currticks  = 0;
                         currticks = OsclTickCount::TickCount();
                         uint32 starttime = OsclTickCount::TicksToMsec(currticks);

                         sched->StartScheduler();

                         currticks = OsclTickCount::TickCount();
                         uint32 endtime = OsclTickCount::TicksToMsec(currticks);
                         fprintf(iFile, "  Time taken by the test:  %d\n", (endtime - starttime));
                        );
#endif

            }
            else
            {
                fprintf(iFile, "ERROR! Scheduler is not available. Test case could not run.");
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

void PVMediaInputAuthorEngineTest::CompleteTest(test_case& arTC)
{
    // Print out the result for this test case
    const test_result the_result = arTC.last_result();

    fprintf(iFile, "  Successes %d, Failures %d\n", the_result.success_count() - iTotalSuccess,
            the_result.failures().size() - iTotalFail);

    iTotalSuccess = the_result.success_count();
    iTotalFail = the_result.failures().size();
    iTotalError = the_result.errors().size();

    // Stop the scheduler
    OsclExecScheduler *sched = OsclExecScheduler::Current();
    if (sched)
    {
        sched->StopScheduler();
    }
}

void PVMediaInputAuthorEngineTest::Print_TestCase_Name(int32 aTestnum)
{
    fprintf(iFile, "\n\nTest:%d: ", aTestnum);
    switch (aTestnum)
    {
        case PVMediaInput_Open_Compose_Stop_Test:
        {
            fprintf(iFile, "Open Compose Stop test with AVI/WAV MIO Comp:\n");
        }
        break;
        case PVMediaInput_Open_RealTimeCompose_Stop_Test:
        {
            fprintf(iFile, "Open Real Time Compose Stop test with AVI/WAV MIO Comp:\n");
        }
        break;
        case YUV_Input_VOnly_3gpTest:
        {
            fprintf(iFile, "YUV to V-Only .3gp Test with AVI/WAV component\n");

        }
        break;
        case PCM16_Input_AOnly_3gpTest:
        {
            fprintf(iFile, "PCM16 to A-Only .3gp Test\n");
        }
        break;
        case PCM16_YUV_Input_AV_3gpTest:
        {
            fprintf(iFile, "PCM16 & YUV to AV .3gp Test\n");
        }
        break;
        case H264_Input_VOnly_3gpTest:
        {
            fprintf(iFile, "H264 to V-Only .3gp Test\n");
        }
        break;
        case PCM16In_AMROut_Test:
        {
            fprintf(iFile, "PCM16 to ARM Test\n");
        }
        break;
        case KMaxFileSizeTest:
        {
            fprintf(iFile, "Max FileSize test \n");
        }
        break;

        case K3GPPDownloadModeTest:
        {
            fprintf(iFile, "3GPPDownloadMode test \n");
        }
        break;
        case K3GPPProgressiveDownloadModeTest:
        {
            fprintf(iFile, "3GPPProgressiveDownloadMode test \n");
        }
        break;
        case KMovieFragmentModeTest:
        {
            fprintf(iFile, "Movie Fragment test \n");
        }
        break;
        case CapConfigTest:
        {
            fprintf(iFile, "CapConfig test \n");
        }
        break;
        case AVI_Input_Longetivity_Test:
        {
            fprintf(iFile, "AVI Input Longetivity test:\n");
        }
        break;
        case KMaxFileSizeLongetivityTest:
        {
            fprintf(iFile, "KMaxFileSizeLongetivityTest test \n");
        }
        break;
        case K3GPPDownloadModeLongetivityTest:
        {
            fprintf(iFile, "K3GPPDownloadMode Longetivity Test:\n");
        }
        break;
        case K3GPPProgressiveDownloadModeLongetivityTest:
        {
            fprintf(iFile, "K3GPPProgressiveDownloadMode Longetivity Test:\n");
        }
        break;
        case KMovieFragmentModeLongetivityTest:
        {
            fprintf(iFile, "KMovieFragmentMode Longetivity Test:\n");
        }
        break;
        case PVMediaInput_ErrorHandling_Test_WrongIPFileName:
        {
            fprintf(iFile, "Error Handling wrong IP File test with AVI/WAV MIO Comp\n");
        }
        break;

        case ErrorHandling_WrongOutputPathTest:
        {
            fprintf(iFile, "Error Handling wrong Output path for output File test with AVI/WAV MIO Comp\n");
        }
        break;
        case ErrorHandling_VideoInitFailed:
        {
            fprintf(iFile, "Error Handling test for Video Init Failed\n");
        }
        break;
        case ErrorHandling_VideoEncodeFailed:
        {
            fprintf(iFile, "Error Handiling test for Video Encode Failed\n");
        }
        break;
        case ErrorHandling_VideoEncode5FramesFailed:
        {
            fprintf(iFile, "Error Handling test for Video Encode 5Frames Failed\n");
        }
        break;
        case ErrorHandling_ComposerAddFragFailed:
        {
            fprintf(iFile, "Error Handling test for Composer Add Fragment Failed\n");
        }
        break;
        case ErrorHandling_ComposerAddTrackFailed:
        {
            fprintf(iFile, "Error Handling test for Composer Add Track Failed\n");
        }
        break;
        case ErrorHandling_AVCVideoEncodeFailed:
        {
            fprintf(iFile, "Error Handling test for AVC Video Encoder Failed\n");
        }
        break;
        case ErrorHandling_AVCVideoEncode5FramesFailed:
        {
            fprintf(iFile, "Error Handling test for AVC Video Encode 5 Frames Failed\n");
        }
        break;
        case ErrorHandling_MediaInputNodeStopFailed:
        {
            fprintf(iFile, "Error Handling test for MediaInputNode Stop Failed\n");
        }
        break;
        case ErrorHandling_AudioInitFailed:
        {
            fprintf(iFile, "Error Handiling test for Audio Init Failed \n");
        }
        break;
        case ErrorHandling_AudioEncodeFailed:
        {
            fprintf(iFile, "Error Handling test for Audio Encode Failed \n");
        }
        break;
        case ErrorHandling_MediaInputNode_NoMemBuffer:
        {
            fprintf(iFile, "Error Handling test for MediaInputNode No Memory Buffer available \n");
        }
        break;
        case ErrorHandling_MediaInputNode_Out_Queue_busy:
        {
            fprintf(iFile, "Error Handling test for MediaInputNode Out Queue Busy occur \n");
        }
        break;
        case  ErrorHandling_MediaInputNode_large_time_stamp:
        {
            fprintf(iFile, "ErrorHandling test for MediaInputNode to mess up the time stamp by giving largeer time stamp in begining. \n");
        }
        break;
        case ErrorHandling_MediaInputNode_wrong_time_stamp_after_duration:
        {
            fprintf(iFile, "ErrorHandling test for MediaInputNode to mess up the time stamp by giving wrong timestamp after some duration of time. \n");
        }
        break;
        case ErrorHandling_MediaInputNode_zero_time_stamp:
        {
            fprintf(iFile, "ErrorHandling test for MediaInputNode to mess up the time stamp by assigning timestamp value as zero. \n");
        }
        break;
        case ErrorHandling_MediaInputNode_StateFailure_EPause_SendMIORequest:
        {
            fprintf(iFile, "ErrorHandling test for MediaInputNode to fail EPause state of SendMIORequest(). \n");
        }
        break;
        case ErrorHandling_MediaInputNode_StateFailure_CancelMIORequest:
        {
            fprintf(iFile, "ErrorHandling test for MediaInputNode to fail CancelMIORequest(). \n");
        }
        break;
        case ErrorHandling_MediaInputNode_Corrupt_Video_InputData:
        {
            fprintf(iFile, "ErrorHandling test for MediaInputNode to corrupt the video input data. \n");
        }
        break;
        case ErrorHandling_MediaInputNode_Corrupt_Audio_InputData:
        {
            fprintf(iFile, "ErrorHandling test for MediaInputNode to corrupt the audio input data. \n");
        }
        break;
        case ErrorHandling_MediaInputNode_Node_Cmd_Start:
        {
            fprintf(iFile, "ErrorHandling test for MediaInputNode to fail the node command Start state. \n");
        }
        break;
        case ErrorHandling_MediaInputNode_Node_Cmd_Stop:
        {
            fprintf(iFile, "ErrorHandling test for MediaInputNode to fail the node command Stop state. \n");
        }
        break;
        case ErrorHandling_MediaInputNode_Node_Cmd_Flush:
        {
            fprintf(iFile, "ErrorHandling test for MediaInputNode to fail the node command Flush state. \n");
        }
        break;
        case ErrorHandling_MediaInputNode_Node_Cmd_Pause:
        {
            fprintf(iFile, "ErrorHandling test for MediaInputNode to fail the node command Pause state. \n");
        }
        break;
        case ErrorHandling_MediaInputNode_Node_Cmd_ReleasePort:
        {
            fprintf(iFile, "ErrorHandling test for MediaInputNode to fail the node command Release Port state. \n");
        }
        break;
        case ErrorHandling_MediaInputNode_DataPath_Stall:
        {
            fprintf(iFile, "ErrorHandling test for MediaInputNode to stall data path. \n");
        }
        break;
        case ErrorHandling_MP4Composer_AddTrack_PVMF_AMR_IETF:
        {
            fprintf(iFile, "ErrorHandling test for MP4ComposerNode to fail PVMF_AMR_IETF in AddTrack(). \n");
        }
        break;
        case ErrorHandling_MP4Composer_AddTrack_PVMF_3GPP_TIMEDTEXT:
        {
            fprintf(iFile, "ErrorHandling test for MP4ComposerNode to fail PVMF_3GPP_TIMEDTEXT in AddTrack(). \n");
        }
        break;
        case ErrorHandling_MP4Composer_AddTrack_PVMF_M4V:
        {
            fprintf(iFile, "ErrorHandling test for MP4ComposerNode to fail PVMF_M4V in AddTrack(). \n");
        }
        break;
        case ErrorHandling_MP4Composer_AddTrack_PVMF_H263:
        {
            fprintf(iFile, "ErrorHandling test for MP4ComposerNode to fail PVMF_H263 in AddTrack(). \n");
        }
        break;
        case ErrorHandling_MP4Composer_AddTrack_PVMF_H264_MP4:
        {
            fprintf(iFile, "ErrorHandling test for MP4ComposerNode to fail PVMF_H264_MP4 in AddTrack(). \n");
        }
        break;
        case ErrorHandling_MP4Composer_Node_Cmd_Start:
        {
            fprintf(iFile, "ErrorHandling test for MP4ComposerNode to fail Node Command Start. \n");
        }
        break;
        case ErrorHandling_MP4Composer_Node_Cmd_Stop:
        {
            fprintf(iFile, "ErrorHandling test for MP4ComposerNode to fail Node Command Stop. \n");
        }
        break;
        case ErrorHandling_MP4Composer_Node_Cmd_Flush:
        {
            fprintf(iFile, "ErrorHandling test for MP4ComposerNode to fail Node Command Flush. \n");
        }
        break;
        case ErrorHandling_MP4Composer_Node_Cmd_Pause:
        {
            fprintf(iFile, "ErrorHandling test for MP4ComposerNode to fail Node Command Pause. \n");
        }
        break;
        case ErrorHandling_MP4Composer_Node_Cmd_ReleasePort:
        {
            fprintf(iFile, "ErrorHandling test for MP4ComposerNode to fail Node Command Release Port. \n");
        }
        break;
        case ErrorHandling_MP4Composer_Create_FileParser:
        {
            fprintf(iFile, "ErrorHandling test for MP4ComposerNode to fail creation of mp4 file parser. \n");
        }
        break;
        case ErrorHandling_MP4Composer_RenderToFile:
        {
            fprintf(iFile, "ErrorHandling test for MP4ComposerNode to fail RenderToFile(). \n");
        }
        break;
        case ErrorHandling_MP4Composer_FailAfter_FileSize:
        {
            fprintf(iFile, "ErrorHandling test for MP4ComposerNode returns error after particular file size if reached.\n");
        }
        break;
        case ErrorHandling_MP4Composer_FailAfter_Duration:
        {
            fprintf(iFile, "ErrorHandling test for MP4ComposerNode returns error after some duration of time.\n");
        }
        break;
        case ErrorHandling_VideoEncodeNode_Node_Cmd_Start:
        {
            fprintf(iFile, "ErrorHandling test for VideoEncodeNode to fail Node Command Start. \n");
        }
        break;
        case ErrorHandling_VideoEncodeNode_Node_Cmd_Stop:
        {
            fprintf(iFile, "ErrorHandling test for VideoEncodeNode to fail Node Command Stop. \n");
        }
        break;
        case ErrorHandling_VideoEncodeNode_Node_Cmd_Flush:
        {
            fprintf(iFile, "ErrorHandling test for VideoEncodeNode to fail Node Command Flush. \n");
        }
        break;
        case ErrorHandling_VideoEncodeNode_Node_Cmd_Pause:
        {
            fprintf(iFile, "ErrorHandling test for VideoEncodeNode to fail Node Command Pause. \n");
        }
        break;
        case ErrorHandling_VideoEncodeNode_Node_Cmd_ReleasePort:
        {
            fprintf(iFile, "ErrorHandling test for VideoEncodeNode to fail Node Command ReleasePort. \n");
        }
        break;
        case ErrorHandling_VideoEncodeNode_ConfigHeader:
        {
            fprintf(iFile, "ErrorHandling test for VideoEncodeNode to fail GetVolHeader(). \n");
        }
        break;
        case ErrorHandling_VideoEncodeNode_DataPathStall_Before_ProcessingData:
        {
            fprintf(iFile, "ErrorHandling test for VideoEncodeNode to stall the data path before processing starts. \n");
        }
        break;
        case ErrorHandling_VideoEncodeNode_DataPathStall_After_ProcessingData:
        {
            fprintf(iFile, "ErrorHandling test for VideoEncodeNode to stall the data path after processing is done. \n");
        }
        break;

        case ErrorHandling_VideoEncodeNode_FailEncode_AfterDuration:
        {
            fprintf(iFile, "ErrorHandling test for VideoEncodeNode to fail encode operation after duration of time. \n");
        }
        break;
        case ErrorHandling_AudioEncodeNode_Node_Cmd_Start:
        {
            fprintf(iFile, "ErrorHandling test for AudioEncodeNode to fail Node Command Start. \n");
        }
        break;
        case ErrorHandling_AudioEncodeNode_Node_Cmd_Stop:
        {
            fprintf(iFile, "ErrorHandling test for AudioEncodeNode to fail Node Command Stop. \n");
        }
        break;
        case ErrorHandling_AudioEncodeNode_Node_Cmd_Flush:
        {
            fprintf(iFile, "ErrorHandling test for AudioEncodeNode to fail Node Command Flush. \n");
        }
        break;
        case ErrorHandling_AudioEncodeNode_Node_Cmd_Pause:
        {
            fprintf(iFile, "ErrorHandling test for AudioEncodeNode to fail Node Command Pause. \n");
        }
        break;
        case ErrorHandling_AudioEncodeNode_Node_Cmd_ReleasePort:
        {
            fprintf(iFile, "ErrorHandling test for AudioEncodeNode to fail Node Command Release Port. \n");
        }
        break;
        case ErrorHandling_AudioEncodeNode_FailEncode_AfterDuration:
        {
            fprintf(iFile, "ErrorHandling test for AudioEncodeNode to fail encode operation after duration of time. \n");
        }
        break;
        case ErrorHandling_AudioEncodeNode_DataPathStall_Before_ProcessingData:
        {
            fprintf(iFile, "ErrorHandling test for AudioEncodeNode to stall the data path before the processing starts. \n");
        }
        break;
        case ErrorHandling_AudioEncodeNode_DataPathStall_After_ProcessingData:
        {
            fprintf(iFile, "ErrorHandling test for AudioEncodeNode to stall the data path after the processing is done. \n");
        }
        break;
        case ErrorHandling_AVCEncodeNode_Node_Cmd_Start:
        {
            fprintf(iFile, "ErrorHandling test for AVCEncodeNode to fail Node Command Start. \n");
        }
        break;
        case ErrorHandling_AVCEncodeNode_Node_Cmd_Stop:
        {
            fprintf(iFile, "ErrorHandling test for AVCEncodeNode to fail Node Command Stop. \n");
        }
        break;
        case ErrorHandling_AVCEncodeNode_Node_Cmd_Flush:
        {
            fprintf(iFile, "ErrorHandling test for AVCEncodeNode to fail Node Command Flush. \n");
        }
        break;
        case ErrorHandling_AVCEncodeNode_Node_Cmd_Pause:
        {
            fprintf(iFile, "ErrorHandling test for AVCEncodeNode to fail Node Command Pause. \n");
        }
        break;
        case ErrorHandling_AVCEncodeNode_Node_Cmd_ReleasePort:
        {
            fprintf(iFile, "ErrorHandling test for AVCEncodeNode to fail Node Command ReleasePort. \n");
        }
        break;
        case ErrorHandling_AVCEncodeNode_ConfigHeader:
        {
            fprintf(iFile, "ErrorHandling test for AVCEncodeNode to fail in getting SPS and PPS value. \n");
        }
        break;
        case ErrorHandling_AVCEncodeNode_DataPathStall_Before_ProcessingData:
        {
            fprintf(iFile, "ErrorHandling test for AVCEncodeNode to stall the data path before processing starts. \n");
        }
        break;
        case ErrorHandling_AVCEncodeNode_DataPathStall_After_ProcessingData:
        {
            fprintf(iFile, "ErrorHandling test for AVCEncodeNode to stall the data path after processing is done. \n");
        }
        break;
        case ErrorHandling_AVCEncodeNode_FailEncode_AfterDuration:
        {
            fprintf(iFile, "ErrorHandling test for AVCEncodeNode to fail encode operation after duration of time. \n");
        }
        break;
        case ErrorHandling_MP4Composer_DataPathStall:
        {
            fprintf(iFile, "ErrorHandling test for MP4ComposerNode to stall data path. \n");
        }
        break;
        default:
            break;


    }
    fprintf(iFile, "\n");
}
