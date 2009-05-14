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
/**
 * @file test_pv_author_engine_config.h
 * @brief Default settings and type definitions for PVAuthorEngine unit test input
 */

#ifndef TEST_PV_AUTHOR_ENGINE_CONFIG_H_INCLUDED
#define TEST_PV_AUTHOR_ENGINE_CONFIG_H_INCLUDED



// Set to 1 to use the scheduler native to the system instead of PV scheduler
#define USE_NATIVE_SCHEDULER 0

#define KLogFile _STRLIT("pvauthor.log")
#define KPVAELoggerFile _STRLIT("pvaelogger.txt")


// Input files
#define KYUVTestInput _STRLIT("yuvtestinput.yuv")
#define KH263TestInput _STRLIT("h263testinput.h263")
#define KAMRTestInput _STRLIT("amrtestinput.amr")
#define KAMRWBTestInput _STRLIT("amrwbtestinput.awb")
#define KAACADIFTestInput _STRLIT("aac_adif.aacadif")
#define KAACADTSTestInput _STRLIT("aac_adts.aacadts")
#define KH264TestInput _STRLIT("h264testinput.h264")
#define KTEXTTestInput _STRLIT("MOL004.txt")
#define KTEXTLogTestInput _STRLIT("MOL004_text0.log")
#define KTEXTTxtFileTestInput _STRLIT("MOL004_sd_txt1.txt")

// Output files
#define KAMRInputAOnly3gpTestOutput _STRLIT("amr_in_a_only_test.3gp")
#define KAMRWBInputAOnly3gpTestOutput _STRLIT("amrwb_in_a_only_test.3gp")
#define KH263InputVOnly3gpTestOutput _STRLIT("h263_in_v_only_test.3gp")
#define KAMRYUVInputAV3gpTestOutput _STRLIT("amr_yuv_in_av_test.3gp")
#define KAMRH263InputAV3gpTestOutput _STRLIT("amr_h263_in_av_test.3gp")
#define KAMRYUVInputAVMp4TestOutput _STRLIT("amr_yuv_in_av_test.3gp")
#define KAMRM4VInputAVMp4TestOutput _STRLIT("amr_m4v_in_av_test.3gp")
#define KFOAOnlyAMRTestOutput _STRLIT("amr_testoutput.amr")
#define KFOAOnlyAMRWBTestOutput _STRLIT("amrwb_testoutput.awb")
#define KFOAOnlyAACADIFTestOutput _STRLIT("aac_adif_output.aac")
#define KFOAOnlyAACADTSTestOutput _STRLIT("aac_adts_output.aac")
#define KH264AMRInputAVMp4TestTestOutput _STRLIT("h264_amr_in_av_test.3gp")
#define KYUVAMRTEXTInputAVTMp4TestOutput _STRLIT("yuv_amr_text_test.3gp")
#define KYUVTEXTInputMp4TestOutput _STRLIT("yuv_text_test.3gp")
#define KAMRTEXTInputMp4TestOutput _STRLIT("amr_text_test.3gp")
#define KTEXTInputMp4TestOutput _STRLIT("text_test.3gp")
//Error Handling TestCases
#define KTEXTTestInputWrong					_STRLIT("texttestinput_wrong.txt")

#define DEFAULTSOURCEFILENAME						"testinput.avi"
#define DEFAULTSOURCEFORMATTYPE							PVMF_MIME_AVIFF
#define DEFAULTOUTPUTFILENAME						"testoutput.3gp"
#define DEFAULTOUTPUT_3GP_FILENAME						"testoutput.3gp"

//YUV test through AVI file
#define KYUV_AVI_VIDEO_ONLY							"videoonly.avi"
#define KPCM_AVI_AUDIO_ONLY							"audioonly.wav"
#define KPCM_YUV_AVI_FILENAME						"testoutput_IYUV_8k_16_mono.avi"
#define KMediaInput_Open_RealTimeCompose_Output		"mediainput_open_realtimecompose.3gp"
//YUV & PCM test output files
#define KYUV_VOnly_3GPP_Output						"Yuv_VideoOnly_Output.3gp"
#define KPCM_AOnly_3GPP_Output						"Pcm_AudioOnly_Output.3gp"
#define KPCM_YUV_AV_3GPP_Output						"Pcm_Yuv_AV_Output.3gp"
#define KYUV_VOnly_MP4_Output						"Yuv_VideoOnly_Output.3gp"
#define KH264_VOnly_MP4_Output						"H264_VideoOnly_Output.3gp"
#define KPCM_AOnly_MP4_Output						"Pcm_AudioOnly_Output.3gp"
#define KPCM_YUV_AV_MP4_Output						"Pcm_Yuv_AV_Output.3gp"
#define KPCM16_AMR_TestOutput						"pcm_testoutput.amr"

#define KMaxFileSizeTestOutput						"maxfilesize_test.3gp"
#define KMaxFileSize_long_Output					"maxfilesize_long_output.3gp"


#define K3GPPDOWNLOADMODE_OUTPUT					"3GPPDownloadAuthoring_output.3gp"
#define K3GPPPROGRESSIVEDOWNLOADMODE_OUTPUT			"3GPPProgressiveDownloadAuthoring_output.3gp"
#define KMOVIEFRAGMENTMODE_OUTPUT					"MovieFragment_output.3gp"
#define KCAPCONFIG_OUTPUT							"CapConfig_TestOutput.3gp"
#define KAVI_Input_Long_OUTPUT						"Avi_Input_long_output.3gp"
#define K3GPPDOWNLOADMODE_Long_OUTPUT				"3GPPDownloadAuthoring_long_test.3gp"
#define K3GPPPROGRESSIVEDOWNLOADMODE_Long_OUTPUT		"3GPPProgressiveDownloadAuthoring_long_test.3gp"
#define KMOVIEFRAGMENTMODE_Long_OUTPUT				"movieFragmentFileTestlong_.3gp"
#define FILE_NAME_ERROR_HANDLING					"testinput_rgb16.avi"
#define WRONGIPFILENAME_ERRORHANDLING				"wrongdir/testinput.avi"
#define WRONGOPFILENAME_ERRORHANDLING				"pvaetest_wrong/testoutput.3gp"
#endif // TEST_PV_AUTHOR_ENGINE_CONFIG_H_INCLUDED


