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
#ifndef TEST_PV_AUTHOR_ENGINE_TYPEDEFS_H
#define TEST_PV_AUTHOR_ENGINE_TYPEDEFS_H

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_ERROR_H_INCLUDED
#include "oscl_error.h"
#endif
#ifndef OSCL_ERROR_CODES_H_INCLUDED
#include "oscl_error_codes.h"
#endif
#ifndef OSCL_CONFIG_IO_H_INCLUDED
#include "osclconfig_io.h"
#endif
#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif
#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif
#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif
#ifndef OSCL_SCHEDULER_AO_H_INCLUDED
#include "oscl_scheduler_ao.h"
#endif
#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif
#ifndef PVLOGGER_STDERR_APPENDER_H_INCLUDED
#include "pvlogger_stderr_appender.h"
#endif
#ifndef PVLOGGER_FILE_APPENDER_H_INCLUDED
#include "pvlogger_file_appender.h"
#endif
#ifndef PVLOGGER_TIME_AND_ID_LAYOUT_H_INCLUDED
#include "pvlogger_time_and_id_layout.h"
#endif
#ifndef TEST_CASE_H_INCLUDED
#include "test_case.h"
#endif
#ifndef PVMF_NODE_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
#endif
#ifndef PV_ENGINE_TYPES_H_INCLUDED
#include "pv_engine_types.h"
#endif
#ifndef PV_ENGINE_OBSERVER_H_INCLUDED
#include "pv_engine_observer.h"
#endif
#ifndef PVAUTHORENGINEFACTORY_H_INCLUDED
#include "pvauthorenginefactory.h"
#endif
#ifndef PVAUTHORENGINEINTERFACE_H_INCLUDED
#include "pvauthorengineinterface.h"
#endif
#ifndef PVAETESTINPUT_H_INCLUDED
#include "pvaetestinput.h"
#endif
#ifndef OSCL_UTF8CONV_H
#include "oscl_utf8conv.h"
#endif
#ifndef OSCL_SCHEDULER_H_INCLUDED
#include "oscl_scheduler.h"
#endif
#ifndef TEXT_TEST_INTERPRETER_H_INCLUDED
#include "text_test_interpreter.h"
#endif
#ifndef PV_MP4_H263_ENC_EXTENSION_H_INCLUDED
#include "pvmp4h263encextension.h"
#endif
#ifndef PVMP4FFCN_CLIPCONFIG_H_INCLUDED
#include "pvmp4ffcn_clipconfig.h"
#endif
#ifndef PVMF_FILEOUTPUT_CONFIG_H_INCLUDED
#include "pvmf_fileoutput_config.h"
#endif
#ifndef PVMF_COMPOSER_SIZE_AND_DURATION_H_INCLUDED
#include "pvmf_composer_size_and_duration.h"
#endif
#ifndef TEST_PV_AUTHOR_ENGINE_CONFIG_H_INCLUDED
#include "test_pv_author_engine_config.h"
#endif
#ifndef UNIT_TEST_ARGS_H_INCLUDED
#include "unit_test_args.h"
#endif
#ifndef PVLOGGER_FILE_APPENDER_H_INCLUDED
#include "pvlogger_file_appender.h"
#endif
#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif
#ifndef OSCL_MEM_AUDIT_H_INCLUDED
#include "oscl_mem_audit.h"
#endif
#ifndef OSCL_UTF8CONV_H
#include "oscl_utf8conv.h"
#endif
#ifndef OSCL_STRING_UTILS_H
#include "oscl_string_utils.h"
#endif
#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_stdstring.h"
#endif
#ifndef PVMI_CONFIG_AND_CAPABILITY_H_INCLUDED
#include "pvmi_config_and_capability.h"
#endif
#ifndef PVMI_CONFIG_AND_CAPABILITY_OBSERVER_H_INCLUDED
#include "pvmi_config_and_capability_observer.h"
#endif

#ifndef OSCLCONFIG_H_INCLUDED
#include "osclconfig.h"
#endif

#ifndef OSCL_UTF8CONV_H
#include "oscl_utf8conv.h"
#endif

#ifndef PVAELOGGER_H_INCLUDED
#include "test_pv_author_engine_logger.h"
#endif

//#define _W(x) _STRLIT_WCHAR(x)

//composer mime type
#define K3gpComposerMimeType "/x-pvmf/ff-mux/3gp"
#define KAMRNbComposerMimeType		"/x-pvmf/ff-mux/amr-nb"
#define KAMRWBComposerMimeType		"/x-pvmf/ff-mux/amr-wb"
#define KAACADIFComposerMimeType	"/x-pvmf/ff-mux/adif"
#define KAACADTSComposerMimeType	"/x-pvmf/ff-mux/adts"

//encoder mime type
#define KAMRNbEncMimeType "/x-pvmf/audio/encode/amr-nb"
#define KAMRWbEncMimeType "/x-pvmf/audio/encode/amr-wb"
#define KAACMP4EncMimeType "/x-pvmf/audio/encode/X-MPEG4-AUDIO"
#define KH263EncMimeType "/x-pvmf/video/encode/h263"
#define KH264EncMimeType "/x-pvmf/video/encode/h264"
#define KMp4EncMimeType "/x-pvmf/video/encode/mp4"
#define KAACADIFEncMimeType			"/x-pvmf/audio/encode/aac/adif"
#define KAACADTSEncMimeType			"/x-pvmf/audio/encode/aac/adts"
#define KTextEncMimeType "/x-pvmf/text/encode/txt"

// Default input settings
extern const uint32 KVideoBitrate ;
extern const uint32 KVideoFrameWidth ;
extern const uint32 KVideoFrameHeight;
extern const uint32 KVideoTimescale ;
extern const uint32 KNumLayers;

extern const uint32 KVideoFrameRate;
extern const uint32 KNum20msFramesPerChunk;
extern const uint32 KAudioBitsPerSample;

extern const uint16 KVideoIFrameInterval;
extern const uint8 KH263VideoProfile;
extern const uint8 KH263VideoLevel;
extern const uint32 KAudioBitrate;
extern const uint32 KAudioBitrateWB;
extern const uint32 KAACAudioBitrate;
extern const uint32 KAudioTimescale;
extern const uint32 KAudioTimescaleWB;
extern const uint32 KAudioNumChannels;

extern const uint32 KMaxFileSize;
extern const uint32 KMaxDuration;
extern const uint32 KFileSizeProgressFreq;
extern const uint32 KDurationProgressFreq;
extern const uint32 KTestDuration;
extern const uint32 KTextTimescale;
extern const uint32 KTextFrameWidth;
extern const uint32 KTextFrameHeight;



// it's for setting Authoring Time Unit for selecting counter loop
// this time unit is used as default authoring time for longetivity tests
//const uint32 KAuthoringSessionUnit = 1800; //in seconds
extern const uint32 KAuthoringSessionUnit;
extern const uint32 KPauseDuration;

// The string to prepend to source filenames
#define SOURCENAME_PREPEND_STRING ""
#define SOURCENAME_PREPEND_WSTRING _STRLIT_WCHAR("")


#define ARRAY_SIZE	512

//enum types for test cases
typedef enum
{
    ERROR_NOSTATE = 0,
    ERROR_VIDEO_START_INIT,
    ERROR_VIDEO_START_ENCODE,
    ERROR_VIDEO_START_ENCODE_5FRAMES,
    ERROR_COMPOSER_START_ADDMEMFRAG,
    ERROR_COMPOSER_START_ADDTRACK,
    ERROR_MEDIAINPUTNODE_ADDDATASOURCE_START,
    ERROR_MEDIAINPUTNODE_ADDDATASOURCE_STOP,
    ERROR_AVC_START_ENCODE,
    ERROR_AVC_START_ENCODE_5FRAMES
}FAIL_STATE;


typedef enum
{

    /*********** Compressed Tests Begin****************************/
    // Tests that take compressed inputs
    //3gp output file
    AMR_Input_AOnly_3gpTest  = 0,						//.amr
    H263_Input_VOnly_3gpTest = 1,						//.h263
    AMR_YUV_Input_AV_3gpTest = 2,						//.amr+.yuv
    AMR_H263_Input_AV_3gpTest = 3,						//.amr+.h263

    H264_AMR_Input_AV_3gpTest = 4,						//.yuv
    AMR_YUV_Input_AV_M4V_AMR_Output_3gpTest = 5,			//.amr+.yuv
    TEXT_Input_TOnly_3gpTest = 6,
    AMR_TEXT_Input_AT_3gpTest = 7,
    YUV_TEXT_Input_VT_3gpTest = 8,
    AMR_YUV_TEXT_Input_AVT_Mp4Test = 9,

    K3GP_OUTPUT_TestEnd = 10,								//placeholder

    //amr output file
    AMR_FOutput_Test = 11,								//.amr

    AMR_OUTPUT_TestEnd = 12,								// placeholder

    //aac output file
    AACADIF_FOutput_Test = 13,							//.aacadif
    AACADTS_FOutput_Test = 14,							//.aacadts

    AAC_OUTPUT_TestEnd = 15,								// placeholder

    //ErrorHandling_WrongTextInputFileNameTest,			//Incorrect name of Text input file
    //amr output file
    AMR_FileOutput_Test_UsingExternalFileHandle = 16,		//.amr
    //3gp output file
    AMRWB_Input_AOnly_3gpTest = 17,

    //amr-wb output file
    AMRWB_FOutput_Test = 18,

    CompressedNormalTestEnd = 19,							//placeholder

    /*********** Compressed Longetivity Tests *********************/
    // tests that takes compressed inputs and run for long duration
    // The duration is configurable through command line argument
    CompressedLongetivityTestBegin = 100,			//placeholder
    //3gp output file
    TEXT_Input_TOnly_3gp_LongetivityTest = 101,			//.txt
    AMR_TEXT_Input_AT_3gp_LongetivityTest = 102,			//.amr+.txt
    YUV_TEXT_Input_VT_3gp_LongetivityTest = 103,			//.txt+.yuv
    AMR_YUV_TEXT_Input_AVT_3gp_LongetivityTest = 104,		//.txt+.yuv +.amr
    Compressed_LongetivityTestEnd = 105,					// placeholder

    /*********** Compressed Tests End******************************/

    /*********** UnCompressed Tests Begin**************************/
    // Test case that takes Uncompressed input through AVI File begin

    // AVI file must have RGB24, RGB12, YUV420 planar or PCM mono 8KHz data only
    UnCompressed_NormalTestBegin = 200,					//placeholder
    PVMediaInput_Open_Compose_Stop_Test = 201,			//Use testinput.avi
    PVMediaInput_Open_RealTimeCompose_Stop_Test = 202,	//Real Time authoring
    YUV_Input_VOnly_3gpTest = 203,						//.yuv
    PCM16_Input_AOnly_3gpTest = 204,						//.pcm
    PCM16_YUV_Input_AV_3gpTest = 205,						//.pcm+.yuv
    H264_Input_VOnly_3gpTest = 206,						//.yuv
    PCM16In_AMROut_Test = 207,							//.pcm//207

    KMaxFileSizeTest = 208,								//.amr+.yuv

    K3GPPDownloadModeTest = 210,							//.amr+.yuv
    K3GPPProgressiveDownloadModeTest = 211,				//.amr+.yuv

    KMovieFragmentModeTest = 213,
    CapConfigTest = 214,

    PVMediaInput_Pause_Resume_Test = 215,

    PVMediaInput_Reset_After_Create_Test = 216,
    PVMediaInput_Reset_After_Open_Test = 217,
    PVMediaInput_Reset_After_AddDataSource_Test = 218,
    PVMediaInput_Reset_After_SelectComposer_Test = 219,
    PVMediaInput_Reset_After_AddMediaTrack_Test = 220,
    PVMediaInput_Reset_After_Init_Test = 221,
    PVMediaInput_Reset_After_Start_Test = 222,
    PVMediaInput_Reset_After_Pause_Test = 223,
    PVMediaInput_Reset_After_Recording_Test = 224,
    PVMediaInput_Reset_After_Stop_Test = 225,

    PVMediaInput_Delete_After_Create_Test = 226,
    PVMediaInput_Delete_After_Open_Test = 227,
    PVMediaInput_Delete_After_AddDataSource_Test = 228,
    PVMediaInput_Delete_After_SelectComposer_Test = 229,
    PVMediaInput_Delete_After_AddMediaTrack_Test = 230,
    PVMediaInput_Delete_After_Init_Test = 231,
    PVMediaInput_Delete_After_Start_Test = 232,
    PVMediaInput_Delete_After_Pause_Test = 233,
    PVMediaInput_Delete_After_Recording_Test = 234,
    PVMediaInput_Delete_After_Stop_Test = 235,
    PVMediaInput_Open_Compose_Stop_Test_UsingExternalFileHandle = 236,

    UnCompressed_NormalTestEnd = 237,

    /********** Uncompressed Longetivity tests*********************/
    UnCompressed_LongetivityTestBegin = 300,			//placeholder
    AVI_Input_Longetivity_Test = 301,
    KMaxFileSizeLongetivityTest = 302,					//.amr+.yuv/302
    K3GPPDownloadModeLongetivityTest = 304,				//.amr+.yuv
    K3GPPProgressiveDownloadModeLongetivityTest = 305,  //.amr+.yuv
    KMovieFragmentModeLongetivityTest = 307,
    UnCompressed_LongetivityTestEnd = 308,

    /*********** UnCompressed Tests End****************************/

    /*********** Error Handling Tests Begin************************/

    // Error Handling tests. These are to test the error handling capability of Author Engine.
    // Some of the tests takes unsupported inputs like RGB16 data (PVMediaInput_ErrorHandling_Test_WrongFormat).
    // Other tests deliberately induces errors at various points in the data path. The error point is send through
    // KVP keys through the test app.

    // Error handling tests that takes compressed inputs
    KCompressed_Errorhandling_TestBegin = 400,
    ErrorHandling_WrongTextInputFileNameTest = 401,
    ErrorHandling_MediaInputNodeStartFailed = 402,
    KCompressed_Errorhandling_TestEnd = 403,

    //Error handling tests that takes uncompressed inputs through avi files.
    KUnCompressed_Errorhandling_TestBegin = 500,
    PVMediaInput_ErrorHandling_Test_WrongFormat = 501,	//Use testinput_rgb16.avi
    PVMediaInput_ErrorHandling_Test_WrongIPFileName = 502,
    ErrorHandling_WrongOutputPathTest = 503,
    ErrorHandling_VideoInitFailed = 504,							//Video Encoder Init Failed
    ErrorHandling_VideoEncodeFailed = 505,						//Video Encoder Encode Failed
    ErrorHandling_VideoEncode5FramesFailed = 506,					//VideoEncNode Encode5Frames Failed
    ErrorHandling_ComposerAddFragFailed = 507,					//507, Composer AddMemFrag Failed
    ErrorHandling_ComposerAddTrackFailed = 508,					//Composer AddMemTrack Failed
    ErrorHandling_AVCVideoEncodeFailed = 509,						//AVCEncNode Encode Failed
    ErrorHandling_AVCVideoEncode5FramesFailed = 510,				//AVCEncNode Encode5Frames Failed
    ErrorHandling_MediaInputNodeStopFailed = 511,
    ErrorHandling_AudioInitFailed = 512,							//Audio Encoder(AMR) Init Failed
    ErrorHandling_AudioEncodeFailed = 513,						//Audio Encoder(AMR) Encode Failed
    ErrorHandling_MediaInputNode_NoMemBuffer = 514,
    ErrorHandling_MediaInputNode_Out_Queue_busy = 515,
    ErrorHandling_MediaInputNode_large_time_stamp = 516,						//MediaInputNode Error in time stamp for large value.
    ErrorHandling_MediaInputNode_wrong_time_stamp_after_duration = 517,		//MediaInputNode Error in time stamp for wrong value after duration of time.
    ErrorHandling_MediaInputNode_zero_time_stamp = 518,						//MediaInputNode Error in time stamp for zero value.
    ErrorHandling_MediaInputNode_StateFailure_EPause_SendMIORequest = 519,	//MediaInputNode Error in SendMIOioRequest().
    ErrorHandling_MediaInputNode_StateFailure_CancelMIORequest = 520,			//MediaInputNode Error in CancelMIORequest().
    ErrorHandling_MediaInputNode_Corrupt_Video_InputData = 521,				//MediaInputNode Corrupt the video input data.
    ErrorHandling_MediaInputNode_Corrupt_Audio_InputData = 522,				//MediaInputNode Corrupt the audio input data.
    ErrorHandling_MediaInputNode_DataPath_Stall = 523,						//MediaInputNode Stall the data path.
    ErrorHandling_MP4Composer_AddTrack_PVMF_AMR_IETF = 524,					//MP4ComposerNode Error in AddTrack() for PVMF_AMR_IETF.
    ErrorHandling_MP4Composer_AddTrack_PVMF_3GPP_TIMEDTEXT = 525,				//MP4ComposerNode Error in AddTrack() for PVMF_3GPP_TIMEDTEXT.
    ErrorHandling_MP4Composer_AddTrack_PVMF_M4V = 526,						//MP4ComposerNode Error in AddTrack() for PVMF_M4V.
    ErrorHandling_MP4Composer_AddTrack_PVMF_H263 = 527,						//MP4ComposerNode Error in AddTrack() for PVMF_H263.
    ErrorHandling_MP4Composer_AddTrack_PVMF_H264_MP4 = 528,					//MP4ComposerNode Error in AddTrack() for PVMF_H264_MP4.
    ErrorHandling_MP4Composer_Create_FileParser = 529,						//MP4ComposerNode Error in the creation of mp4 file parser.
    ErrorHandling_MP4Composer_RenderToFile = 530,								//MP4ComposerNode Error in the RenderToFile().
    ErrorHandling_MP4Composer_FailAfter_FileSize = 531,						//MP4ComposerNode Error after a particular file size is reached.
    ErrorHandling_MP4Composer_FailAfter_Duration = 532,						//MP4ComposerNode Error after a duration of some time.
    ErrorHandling_MP4Composer_DataPathStall = 533,							//MP4ComposerNode Stall the data path.
    ErrorHandling_VideoEncodeNode_ConfigHeader = 534,							//VideoEncodeNode Error in GetVolHeader().
    ErrorHandling_VideoEncodeNode_DataPathStall_Before_ProcessingData = 535,	//VideoEncodeNode Stall the data path before processing starts.
    ErrorHandling_VideoEncodeNode_DataPathStall_After_ProcessingData = 536,	//VideoEncodeNode Stall the data path post processing.
    ErrorHandling_VideoEncodeNode_FailEncode_AfterDuration = 537,				//VideoEncodeNode Error in encode after duration of time.
    ErrorHandling_AudioEncodeNode_FailEncode_AfterDuration = 538,				//AudioEncodeNode Error in encode operation after duration of time.
    ErrorHandling_AudioEncodeNode_DataPathStall_Before_ProcessingData = 539,	//AudioEncodeNode Stall data path before processing starts.
    ErrorHandling_AudioEncodeNode_DataPathStall_After_ProcessingData = 540,	//AudioEncodeNode Stall data path post processing.
    ErrorHandling_AVCEncodeNode_ConfigHeader = 541,							//AVCEncodeNode Error in in getting SPS & PPS Values.
    ErrorHandling_AVCEncodeNode_DataPathStall_Before_ProcessingData = 542,	//AVCEncodeNode Stall the data path before processing starts.
    ErrorHandling_AVCEncodeNode_DataPathStall_After_ProcessingData = 543,		//AVCEncodeNode Stall the data path post processing.
    ErrorHandling_AVCEncodeNode_FailEncode_AfterDuration = 544,				//AVCEncodeNode Error in encode after duration of time.

    /***** Test for Node Commands *****/
    ErrorHandling_MediaInputNode_Node_Cmd_Start = 545,			//MediaInputNode Error in node command DoStart().
    ErrorHandling_MediaInputNode_Node_Cmd_Stop = 546,				//MediaInputNode Error in node command DoStop().
    ErrorHandling_MediaInputNode_Node_Cmd_Flush = 547,			//MediaInputNode Error in node command DoFlush().
    ErrorHandling_MediaInputNode_Node_Cmd_Pause = 548,			//MediaInputNode Error in node command DoPause().
    ErrorHandling_MediaInputNode_Node_Cmd_ReleasePort = 549,		//MediaInputNode Error in node command DoReleasePort().
    ErrorHandling_MP4Composer_Node_Cmd_Start = 550,				//MP4ComposerNode Error in the node command DoStart().
    ErrorHandling_MP4Composer_Node_Cmd_Stop = 551,				//MP4ComposerNode Error in the node command DoStop().
    ErrorHandling_MP4Composer_Node_Cmd_Flush = 552,				//MP4ComposerNode Error in the node command DoFlush().
    ErrorHandling_MP4Composer_Node_Cmd_Pause = 553,				//MP4ComposerNode Error in the node command DoPause().
    ErrorHandling_MP4Composer_Node_Cmd_ReleasePort = 554,			//MP4ComposerNode Error in the node command DoReleasePort().
    ErrorHandling_VideoEncodeNode_Node_Cmd_Start = 555,			//VideoEncodeNode Error in node command DoStart().
    ErrorHandling_VideoEncodeNode_Node_Cmd_Stop = 556,			//VideoEncodeNode Error in node command DoStop().
    ErrorHandling_VideoEncodeNode_Node_Cmd_Flush = 557,			//VideoEncodeNode Error in node command DoFlush().
    ErrorHandling_VideoEncodeNode_Node_Cmd_Pause = 558,			//VideoEncodeNode Error in node command DoPause().
    ErrorHandling_VideoEncodeNode_Node_Cmd_ReleasePort = 559,		//VideoEncodeNode Error in node command DoReleasePort().
    ErrorHandling_AudioEncodeNode_Node_Cmd_Start = 560,			//AudioEncodeNode Error in node command DoStart().
    ErrorHandling_AudioEncodeNode_Node_Cmd_Stop = 561,			//AudioEncodeNode Error in node command DoStop().
    ErrorHandling_AudioEncodeNode_Node_Cmd_Flush = 562,			//AudioEncodeNode Error in node command DoFlush().
    ErrorHandling_AudioEncodeNode_Node_Cmd_Pause = 563,			//AudioEncodeNode Error in node command DoPause().
    ErrorHandling_AudioEncodeNode_Node_Cmd_ReleasePort = 564,		//AudioEncodeNode Error in node command DoReleasePort().
    ErrorHandling_AVCEncodeNode_Node_Cmd_Start = 565,				//AVCEncodeNode Error in node command DoStart().
    ErrorHandling_AVCEncodeNode_Node_Cmd_Stop = 566,				//AVCEncodeNode Error in node command DoStop().
    ErrorHandling_AVCEncodeNode_Node_Cmd_Flush = 567,				//AVCEncodeNode Error in node command DoFlush().
    ErrorHandling_AVCEncodeNode_Node_Cmd_Pause = 568,				//AVCEncodeNode Error in node command DoPause().
    ErrorHandling_AVCEncodeNode_Node_Cmd_ReleasePort = 569,		//AVCEncodeNode Error in node command DoReleasePort().

    KUnCompressed_Errorhandling_TestEnd = 570,

    /*********** Error Handling Tests End**************************/
    LastInteractiveTest = 1000,
    Invalid_Test

} PVAETestCase;

class pvauthor_async_test_observer;

typedef struct
{
    pvauthor_async_test_observer* iObserver;
    test_case* iTestCase;
    int32 iTestCaseNum;
    FILE* iStdOut;

} PVAuthorAsyncTestParam;




/** Enumeration of types of asychronous commands that can be issued to PV Author Engine */
typedef enum
{
    PVAE_CMD_NONE = 0,
    PVAE_CMD_SET_LOG_APPENDER,
    PVAE_CMD_REMOVE_LOG_APPENDER,
    PVAE_CMD_SET_LOG_LEVEL,
    PVAE_CMD_GET_LOG_LEVEL,
    PVAE_CMD_CREATE,
    PVAE_CMD_OPEN,
    PVAE_CMD_CLOSE,
    PVAE_CMD_ADD_DATA_SOURCE,
    PVAE_CMD_ADD_DATA_SOURCE_AUDIO,
    PVAE_CMD_ADD_DATA_SOURCE_VIDEO,
    PVAE_CMD_ADD_DATA_SOURCE_TEXT,
    PVAE_CMD_REMOVE_DATA_SOURCE,
    PVAE_CMD_SELECT_COMPOSER,
    PVAE_CMD_ADD_MEDIA_TRACK,
    PVAE_CMD_ADD_AUDIO_MEDIA_TRACK,
    PVAE_CMD_ADD_VIDEO_MEDIA_TRACK,
    PVAE_CMD_ADD_TEXT_MEDIA_TRACK,
    PVAE_CMD_ADD_DATA_SINK,
    PVAE_CMD_REMOVE_DATA_SINK,
    PVAE_CMD_INIT,
    PVAE_CMD_RESET,
    PVAE_CMD_START,
    PVAE_CMD_PAUSE,
    PVAE_CMD_RESUME,
    PVAE_CMD_STOP,
    PVAE_CMD_QUERY_UUID,
    PVAE_CMD_QUERY_INTERFACE,
    PVAE_CMD_GET_SDK_INFO,
    PVAE_CMD_GET_SDK_MODULE_INFO,
    PVAE_CMD_CANCEL_ALL_COMMANDS,
    PVAE_CMD_QUERY_INTERFACE2,
    PVAE_CMD_CLEANUPANDCOMPLETE,
    PVAE_CMD_CAPCONFIG_SYNC,
    PVAE_CMD_CAPCONFIG_ASYNC,
    PVAE_CMD_RECORDING,
    PVAE_CMD_QUERY_INTERFACE1,
    PVAE_CMD_CAPCONFIG_SYNC1,
    PVAE_CMD_QUERY_INTERFACE_COMP
} PVAECmdType;


////////////////////////////////////////////////////////////////////////////
class PVLoggerSchedulerSetup
{
    public:
        PVLoggerSchedulerSetup() {};
        ~PVLoggerSchedulerSetup() {};

        void InitLoggerScheduler()
        {
            // Logging by PV Logger
            PVLogger::Init();
            //PVAELogger::ParseConfigFile(_W("uilogger.txt"));
            PVAELogger::ParseConfigFile(KPVAELoggerFile);

            // Construct and install the active scheduler
            OsclScheduler::Init("PVAuthorEngineTestScheduler");
        }

        void CleanupLoggerScheduler()
        {
            OsclScheduler::Cleanup();
            PVLogger::Cleanup();
        }

};
// Observer class for pvPlayer async test to notify completion of test
class pvauthor_async_test_observer
{
    public:
        // Signals completion of test. Test instance can be deleted after this callback completes.
        virtual void CompleteTest(test_case &) = 0;
};


////////////////////////////////////////////////////////////////////////////
class pvauthor_async_test_base : public OsclTimerObject,
            public PVCommandStatusObserver,
            public PVErrorEventObserver,
            public PVInformationalEventObserver
{
    public:
        pvauthor_async_test_base(PVAuthorAsyncTestParam aTestParam)
                : OsclTimerObject(OsclActiveObject::EPriorityNominal, "PVAuthorEngineAsyncTestBase"),
                iObserver(aTestParam.iObserver),
                iTestCase(aTestParam.iTestCase),
                iTestCaseNum(aTestParam.iTestCaseNum),
                iStdOut(aTestParam.iStdOut)
        {};

        virtual ~pvauthor_async_test_base() {}
        virtual void StartTest() = 0;
        virtual void CommandCompleted(const PVCmdResponse& /*aResponse*/) {}
        virtual void HandleErrorEvent(const PVAsyncErrorEvent& /*aEvent*/) {}
        virtual void HandleInformationalEvent(const PVAsyncInformationalEvent& /*aEvent*/) {}

        pvauthor_async_test_observer* iObserver;
        test_case* iTestCase;
        int32 iTestCaseNum;
        FILE* iStdOut;
};


#endif //#ifndef TEST_PV_AUTHOR_ENGINE_TYPEDEFS_H

