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
#ifndef TEST_PV_PLAYER_ENGINE_CONFIG_H_INCLUDED
#define TEST_PV_PLAYER_ENGINE_CONFIG_H_INCLUDED

#ifndef PVMF_FORMAT_TYPE_H_INCLUDED
#include "pvmf_format_type.h"
#endif
#define AUDIOSINK_FORMAT_TYPE PVMF_MIME_PCM16

// The default test file to use
#define DEFAULTSOURCEFILENAME "test.mp4"
#define DEFAULTSOURCEFORMATTYPE PVMF_MIME_MPEG4FF

// Set to 1 to enable MP4 file test cases, 0 to disable
#define RUN_MP4FILE_TESTCASES 1
// MP4 files to test. Needs to be defined if RUN_MP4FILE_TESTCASES is set to 1
#define LOCAL_TEST_FILE_MP4_FORMAT_TYPE PVMF_MIME_MPEG4FF
#define LOCAL_TEST_FILE_MP4_M4V_AMR "test_m4v_amr.mp4"
#define LOCAL_TEST_FILE_MP4_H263_AMR "test_h263_amr.mp4"
#define LOCAL_TEST_FILE_MP4_AVC_AMR "test_avc_amr.mp4"
#define LOCAL_TEST_FILE_MP4_AMR "test_amr.mp4"
#define LOCAL_TEST_FILE_MP4_AAC "test_aac.mp4"
#define LOCAL_TEST_FILE_MP4_M4V_AMR_TEXT "test_m4v_amr_text.mp4"

// Set to 1 to enable AMR file test cases, 0 to disable
#define RUN_AMRFILE_TESTCASES 1
// AMR files to test. Needs to be defined if RUN_AMRFILE_TESTCASES is set to 1
#define LOCAL_TEST_FILE_AMR_FORMAT_TYPE PVMF_MIME_AMRFF
#define LOCAL_TEST_FILE_AMR_IETF "test_amr_ietf.amr"
#define LOCAL_TEST_FILE_AMR_IF2 "test_amr_if2.cod"

// Set to 1 to enable AAC file test cases, 0 to disable
#define RUN_AACFILE_TESTCASES 1
// AAC files to test. Needs to be defined if RUN_AACFILE_TESTCASES is set to 1
#define LOCAL_TEST_FILE_AAC_FORMAT_TYPE PVMF_MIME_AACFF
#define LOCAL_TEST_FILE_AAC_ADTS "test_adts.aac"
#define LOCAL_TEST_FILE_AAC_ADIF "test_adif.aac"
#define LOCAL_TEST_FILE_AAC_RAW "test_raw.aac"

// Set to 1 to enable MP3 file test cases, 0 to disable
#define RUN_MP3FILE_TESTCASES 1
// MP3 files to test. Needs to be defined if RUN_MP3FILE_TESTCASES is set to 1
#define LOCAL_TEST_FILE_MP3_FORMAT_TYPE PVMF_MIME_MP3FF
#define LOCAL_TEST_FILE_MP3_CBR "test_cbr.mp3"
#define LOCAL_TEST_FILE_MP3_VBR "test_vbr.mp3"

// Set to 1 to enable 3GP file test cases, 0 to disable
#define RUN_3GPFILE_TESTCASES 1
// 3GP files to test. Needs to be defined if RUN_3GPFILE_TESTCASES is set to 1
#define LOCAL_TEST_FILE_3GP_FORMAT_TYPE PVMF_MIME_MPEG4FF
#define LOCAL_TEST_FILE_3GP "test.3gp"

// Set to 1 to enable WMA file test cases, 0 to disable
#define RUN_WMAFILE_TESTCASES 1
// WMA files to test. Needs to be defined if RUN_WMAFILE_TESTCASES is set to 1
#define LOCAL_TEST_FILE_WMA_FORMAT_TYPE PVMF_MIME_ASFFF
#define LOCAL_TEST_FILE_WMA "00000035.wma"

// Set to 1 to enable WMV file test cases, 0 to disable
#define RUN_WMVFILE_TESTCASES 1
// WMV files to test. Needs to be defined if RUN_WMVFILE_TESTCASES is set to 1
#define LOCAL_TEST_FILE_WMV_FORMAT_TYPE PVMF_MIME_ASFFF
#define LOCAL_TEST_FILE_WMV "bad_video_config.wmv"

// Set to 1 to enable WAV file test cases, 0 to disable
#define RUN_WAVFILE_TESTCASES 1
// WAV files to test. Needs to be defined if RUN_WAVFILE_TESTCASES is set to 1
#define LOCAL_TEST_FILE_WAV_FORMAT_TYPE PVMF_MIME_WAVFF
#define LOCAL_TEST_FILE_WAV "test.wav"

// Set to 1 to enable ASF file test cases, 0 to disable
#define RUN_ASFFILE_TESTCASES 0
// ASF files to test. Needs to be defined if RUN_ASFFILE_TESTCASES is set to 1
#define LOCAL_TEST_FILE_ASF_FORMAT_TYPE PVMF_MIME_ASFFF
#define LOCAL_TEST_FILE_ASF "test.asf"

// Set to 1 to enable rtsp cloaking test cases, 0 to disable
#define RUN_RTSP_CLOAKING_TESTCASES 0

// Set to 1 to enable Real audio file test cases, 0 to disable
#define RUN_REALAUDIO_FILE_TESTCASES 0

// Real audio files to test. Needs to be defined if RUN_REALAUDIO_FILE_TESTCASES is set to 1
#define LOCAL_TEST_FILE_REALAUDIO_FORMAT_TYPE PVMF_MIME_RMFF
#define LOCAL_TEST_FILE_REALAUDIO "22sec_cook10_stereo.rm"

// Set to 1 to enable FastTrack download test cases, 0 to disable
#define RUN_FASTTRACK_TESTCASES 0

// Set to 1 to enable HTTP download test cases, 0 to disable
#define RUN_HTTPDOWNLOAD_TESTCASES 1

// Set to 1 to enable streaming test cases, 0 to disable
#define RUN_STREAMING_TESTCASES 1

// Set to 1 to enable streaming test cases, 0 to disable
#define RUN_MS_HTTP_STREAMING_TESTCASES 0

// Define to enable only 3GPP streaming formats
// Used with #ifdef
#define ONLY_3GPP_STREAMING_FORMAT

#ifndef RUN_CPMJANUS_TESTCASES
// Set to 1 to enable Janus CPM test cases, 0 to disable
#define RUN_CPMJANUS_TESTCASES 0
#endif

// Set to 1 to enable OMA1 DLA Passthru CPM test cases, 0 to disable
#define RUN_CPMOMA1_DLAPASSTHRU_TESTCASES 1

// Set to 1 to enable app involved track selection test cases, 0 to disable
#define RUN_APP_TRACK_SELECTION_TESTCASES 0

// Set to 1 to use the scheduler native to the system instead of PV scheduler
#define USE_NATIVE_SCHEDULER 0

// The string to prepend to source filenames
#define SOURCENAME_PREPEND_STRING ""
#define SOURCENAME_PREPEND_WSTRING _STRLIT_WCHAR("")

// The string to prepend to output filenames
#ifdef ANDROID
#define OUTPUTNAME_PREPEND_STRING "/sdcard/"
#define OUTPUTNAME_PREPEND_WSTRING _STRLIT_WCHAR("/sdcard/")
#else
#define OUTPUTNAME_PREPEND_STRING ""
#define OUTPUTNAME_PREPEND_WSTRING _STRLIT_WCHAR("")
#endif

//Local playback range
#define FIRST_LOCAL_PLAYBACK 0
#define LAST_LOCAL_PLAYBACK 80


/*Number of Invalid test cases  */

#define NO_OF_AAC_INVALID_TESTCASES 29
const int AAC_INVALID_TEST_ARRAY[NO_OF_AAC_INVALID_TESTCASES] = { 15, 18, 20, 50, 51, 52, 53, 54, 55, 56,
        57, 58, 59, 60, 61, 62, 63, 64, 65, 66,
        73, 74, 75, 76, 77, 78, 79, 80, 81
                                                                };


#define NO_OF_AMR_INVALID_TESTCASES 28
const int AMR_INVALID_TEST_ARRAY[NO_OF_AMR_INVALID_TESTCASES] = { 50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
        60, 61, 62, 67, 68, 69, 70, 71, 72, 73,
        74, 75, 76, 77, 78, 79, 80, 81
                                                                };


#define NO_OF_3GP_OR_MP4_INVALID_TESTCASES 20
const int FILE_3GP_OR_MP4_INVALID_TEST_ARRAY[NO_OF_3GP_OR_MP4_INVALID_TESTCASES] = { 50, 63, 64, 65, 66, 67, 68, 69, 70, 71,
        72, 73, 74, 75, 76, 77, 78, 79, 80, 81
                                                                                   };


#define NO_OF_MP3_INVALID_TESTCASES 31
const int MP3_INVALID_TEST_ARRAY[NO_OF_MP3_INVALID_TESTCASES] = { 15, 18, 20, 50, 51, 52, 53, 54, 55, 56,
        57, 58, 59, 60, 61, 62, 63, 64, 65, 66,
        67, 68, 69, 70, 71, 72, 77, 78, 79, 80, 81
                                                                };



#define NO_OF_WAV_INVALID_TESTCASES 33
const int WAV_INVALID_TEST_ARRAY[NO_OF_WAV_INVALID_TESTCASES] = { 15, 18, 20, 50, 51, 52, 53, 54, 55, 56,
        57, 58, 59, 60, 61, 62, 63, 64, 65, 66,
        67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 79, 80, 81
                                                                };


#define NO_OF_ASF_INVALID_TESTCASES 29
const int ASF_INVALID_TEST_ARRAY[NO_OF_ASF_INVALID_TESTCASES] = { 51, 52, 53, 54, 55, 56, 57, 58, 59, 60,
        61, 62, 63, 64, 65, 66, 67, 68, 69, 70,
        71, 72, 73, 74, 75, 76, 77, 78, 81
                                                                };


#define NO_OF_REAL_INVALID_TESTCASES 31
const int REAL_INVALID_TEST_ARRAY[NO_OF_REAL_INVALID_TESTCASES] = {50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
        60, 61, 62, 63, 64, 65, 66, 67, 68, 69,
        70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
        80
                                                                  };


#define NO_OF_WMV_INVALID_TESTCASES 29
const int WMV_INVALID_TEST_ARRAY[NO_OF_WMV_INVALID_TESTCASES] = { 51, 52, 53, 54, 55, 56, 57, 58, 59, 60,
        61, 62, 63, 64, 65, 66, 67, 68, 69, 70,
        71, 72, 73, 74, 75, 76, 77, 78, 81
                                                                };


#define NO_OF_WMA_INVALID_TESTCASES 29
const int WMA_INVALID_TEST_ARRAY[NO_OF_WMA_INVALID_TESTCASES] = { 51, 52, 53, 54, 55, 56, 57, 58, 59, 60,
        61, 62, 63, 64, 65, 66, 67, 68, 69, 70,
        71, 72, 73, 74, 75, 76, 77, 78, 81
                                                                };


typedef enum Flag_Tag
{
    AAC_ENABLED = 0,
    AMR_ENABLED,
    MP4_ENABLED,
    MP3_ENABLED,
    WAV_ENABLED,
    ASF_ENABLED,
    RM_ENABLED,
    WMV_ENABLED,
    WMA_ENABLED,
    THREE_GP_ENABLED,
    LAST_FORMAT_ENABLED // Place holder
}Flag;

#endif

