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
#ifndef TEST_PV_PLAYER_ENGINE_H_INCLUDED
#define TEST_PV_PLAYER_ENGINE_H_INCLUDED

#ifndef TEST_CASE_H_INCLUDED
#include "test_case.h"
#endif

#ifndef TEXT_TEST_INTERPRETER_H_INCLUDED
#include "text_test_interpreter.h"
#endif

#ifndef OSCL_SCHEDULER_AO_H_INCLUDED
#include "oscl_scheduler_ao.h"
#endif

#ifndef OSCL_EXCEPTION_H_INCLUDE
#include "oscl_exception.h"
#endif

#ifndef PV_PLAYER_FACTORY_H_INCLUDED
#include "pv_player_factory.h"
#endif

#ifndef PV_PLAYER_INTERFACE_H_INCLUDE
#include "pv_player_interface.h"
#endif

#ifndef PV_ENGINE_OBSERVER_H_INCLUDED
#include "pv_engine_observer.h"
#endif

#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif

#ifndef PVLOGGER_STDERR_APPENDER_H_INCLUDED
#include "pvlogger_stderr_appender.h"
#endif

#ifndef PVLOGGER_TIME_AND_ID_LAYOUT_H_INCLUDED
#include "pvlogger_time_and_id_layout.h"
#endif

#ifndef PVMI_MEDIA_IO_FILEOUTPUT_H_INCLUDED
#include "pvmi_media_io_fileoutput.h"
#endif

#ifndef TEST_PV_PLAYER_ENGINE_CONFIG_H_INCLUDED
#include "test_pv_player_engine_config.h"
#endif

#ifndef MEDIA_CLOCK_CONVERTER
#include "media_clock_converter.h"
#endif

template<class DestructClass>
class LogAppenderDestructDealloc : public OsclDestructDealloc
{
    public:
        virtual void destruct_and_dealloc(OsclAny *ptr)
        {
            delete((DestructClass*)ptr);
        }
};

class pvplayer_engine_test_suite : public test_case
{
    public:
        pvplayer_engine_test_suite(char *aFilename, PVMFFormatType aFiletype, int32 aFirstTest, int32 aLastTest, bool aCompV, bool aCompA, bool aFInput, bool aBCS, int32 aLogLevel, int32 aLogNode, int32 aLogText, int32 aLogMem, int32 aFileFormatType, bool  aProxyEnabled);
};


// Observer class for pvPlayer async test to notify completion of test
class pvplayer_async_test_observer
{
    public:
        // Signals completion of test. Test instance can be deleted after this callback completes.
        virtual void TestCompleted(test_case &) = 0;
};


class PVPlayerAsyncTestParam
{
    public:
        pvplayer_async_test_observer* iObserver;
        test_case* iTestCase;
        FILE* iTestMsgOutputFile;
        const char* iFileName;
        PVMFFormatType iFileType;
        bool iCompressedVideo;
        bool iCompressedAudio;
        bool iFileInput;
        bool iBCS;
        int iCurrentTestNumber;
        bool iProxyEnabled;
        //explicit copy constructor
        void Copy(const PVPlayerAsyncTestParam& aParam)
        {
            iObserver = aParam.iObserver;
            iTestCase = aParam.iTestCase;
            iTestMsgOutputFile = aParam.iTestMsgOutputFile;
            iFileName = aParam.iFileName;
            iFileType = aParam.iFileType;
            iCompressedVideo = aParam.iCompressedVideo;
            iCompressedAudio = aParam.iCompressedAudio;
            iFileInput = aParam.iFileInput;
            iBCS = aParam.iBCS;
            iCurrentTestNumber = aParam.iCurrentTestNumber;
            iProxyEnabled = aParam.iProxyEnabled;
        }
} ;

#define PVPATB_TEST_IS_TRUE(condition) (iTestCase->test_is_true_stub( (condition), (#condition), __FILE__, __LINE__ ))

typedef enum
{
    STATE_CREATE,
    STATE_QUERYINTERFACE,
    STATE_ADDDATASOURCE,
    STATE_CONFIGPARAMS,
    STATE_INIT,
    STATE_QUERYLICENSEACQIF,
    STATE_ACQUIRELICENSE,
    STATE_CANCEL_ACQUIRELICENSE,
    STATE_INIT2,
    STATE_GETMETADATAKEYLIST,
    STATE_GETMETADATAVALUELIST,
    STATE_ADDDATASINK_VIDEO,
    STATE_ADDDATASINK_AUDIO,
    STATE_PREPARE,
    STATE_WAIT_FOR_DATAREADY,
    STATE_WAIT_FOR_BUFFCOMPLETE,
    STATE_CANCELALL,
    STATE_WAIT_FOR_CANCELALL,
    STATE_START,
    STATE_SETPLAYBACKRANGE,
    STATE_PAUSE,
    STATE_RESUME,
    STATE_EOSNOTREACHED,
    STATE_STOP,
    STATE_REMOVEDATASINK_VIDEO,
    STATE_REMOVEDATASINK_AUDIO,
    STATE_RESET,
    STATE_REMOVEDATASOURCE,
    STATE_CLEANUPANDCOMPLETE,
    STATE_PROTOCOLROLLOVER,
    STATE_RECONNECT
} PVTestState;

/*!
** PVPlayerTestMioFactory: MIO Factory functions
*/
class PvmiMIOControl;
class PVPlayerTestMioFactory
{
    public:
        static PVPlayerTestMioFactory* Create();
        virtual ~PVPlayerTestMioFactory() {}

        virtual PvmiMIOControl* CreateAudioOutput(OsclAny* aParam) = 0;
        virtual PvmiMIOControl* CreateAudioOutput(OsclAny* aParam, MediaType aMediaType, bool aCompressedAudio = false)
        {
            OSCL_UNUSED_ARG(aParam);
            OSCL_UNUSED_ARG(aMediaType);
            OSCL_UNUSED_ARG(aCompressedAudio);
            return 0;
        };
        virtual PvmiMIOControl* CreateAudioOutput(OsclAny* aParam, PVRefFileOutputTestObserver* aObserver, bool aActiveTiming, uint32 aQueueLimit, bool aSimFlowControl, bool logStrings = true) = 0;
        virtual void DestroyAudioOutput(PvmiMIOControl* aMio) = 0;
        virtual PvmiMIOControl* CreateVideoOutput(OsclAny* aParam) = 0;
        virtual PvmiMIOControl* CreateVideoOutput(OsclAny* aParam, MediaType aMediaType, bool aCompressedVideo = false)
        {
            OSCL_UNUSED_ARG(aParam);
            OSCL_UNUSED_ARG(aMediaType);
            OSCL_UNUSED_ARG(aCompressedVideo);
            return 0;
        };
        virtual PvmiMIOControl* CreateVideoOutput(OsclAny* aParam, PVRefFileOutputTestObserver* aObserver, bool aActiveTiming, uint32 aQueueLimit, bool aSimFlowControl, bool logStrings = true) = 0;
        virtual void DestroyVideoOutput(PvmiMIOControl* aMio) = 0;
        virtual PvmiMIOControl* CreateTextOutput(OsclAny* aParam) = 0;
        virtual PvmiMIOControl* CreateTextOutput(OsclAny* aParam, MediaType aMediaType)
        {
            OSCL_UNUSED_ARG(aParam);
            OSCL_UNUSED_ARG(aMediaType);
            return 0;
        };
        virtual void DestroyTextOutput(PvmiMIOControl* aMio) = 0;
};

// The base class for all pvplayer engine asynchronous tests
class pvplayer_async_test_base : public OsclTimerObject,
            public PVCommandStatusObserver,
            public PVInformationalEventObserver,
            public PVErrorEventObserver
{
    public:
        pvplayer_async_test_base(PVPlayerAsyncTestParam aTestParam) :
                OsclTimerObject(OsclActiveObject::EPriorityNominal, "PVPlayerEngineAsyncTestBase")
        {
            OSCL_ASSERT(aTestParam.iObserver != NULL);
            OSCL_ASSERT(aTestParam.iTestCase != NULL);
            iObserver = aTestParam.iObserver;
            iTestCase = aTestParam.iTestCase;
            iTestMsgOutputFile = aTestParam.iTestMsgOutputFile;
            iFileName = aTestParam.iFileName;
            iFileType = aTestParam.iFileType;
            iCompressedVideo = aTestParam.iCompressedVideo;
            iCompressedAudio = aTestParam.iCompressedAudio;
            iFileInput = aTestParam.iFileInput;
            iBCS = aTestParam.iBCS;
            iTestCaseName = _STRLIT_CHAR(" ");
            iTestNumber = aTestParam.iCurrentTestNumber;
            iProxyEnabled = aTestParam.iProxyEnabled;

            // Initialize the variables to use for context data testing
            iContextObjectRefValue = 0x5C7A; // some random number
            iContextObject = iContextObjectRefValue;

            iMioFactory = PVPlayerTestMioFactory::Create();
            OSCL_ASSERT(iMioFactory);
        }

        virtual ~pvplayer_async_test_base()
        {
            if (iMioFactory)
            {
                delete iMioFactory;
                iMioFactory = NULL;
            }
        }

        virtual void StartTest() = 0;

        virtual void CommandCompleted(const PVCmdResponse& /*aResponse*/) {}
        virtual void HandleErrorEvent(const PVAsyncErrorEvent& /*aEvent*/) {}
        virtual void HandleInformationalEvent(const PVAsyncInformationalEvent& /*aEvent*/) {}

        // Utility function to retrieve the filename from string and replace ',' with '_'
        void RetrieveFilename(const oscl_wchar* aSource, OSCL_wHeapString<OsclMemAllocator>& aFilename)
        {
            if (aSource == NULL)
            {
                return;
            }

            // Find the last '\' or '/' in the string
            oscl_wchar* lastslash = (oscl_wchar*)aSource;
            bool foundlastslash = false;
            while (!foundlastslash)
            {
                const oscl_wchar* tmp1 = oscl_strstr(lastslash, _STRLIT_WCHAR("\\"));
                const oscl_wchar* tmp2 = oscl_strstr(lastslash, _STRLIT_WCHAR("/"));
                if (tmp1 != NULL)
                {
                    lastslash = (oscl_wchar*)tmp1 + 1;
                }
                else if (tmp2 != NULL)
                {
                    lastslash = (oscl_wchar*)tmp2 + 1;
                }
                else
                {
                    foundlastslash = true;
                }
            }

            // Now copy the filename
            if (lastslash)
            {
                aFilename = lastslash;
            }

            // Replace each '.' in filename with '_'
            bool finishedreplace = false;
            while (!finishedreplace)
            {
                oscl_wchar* tmp = OSCL_CONST_CAST(oscl_wchar*, oscl_strstr(aFilename.get_cstr(), _STRLIT_WCHAR(".")));
                if (tmp != NULL)
                {
                    oscl_strncpy(tmp, _STRLIT_WCHAR("_"), 1);
                }
                else
                {
                    finishedreplace = true;
                }
            }
        }

        pvplayer_async_test_observer* iObserver;
        test_case* iTestCase;
        FILE* iTestMsgOutputFile;
        const char *iFileName;
        PVMFFormatType iFileType;
        bool iCompressedVideo;
        bool iCompressedAudio;
        bool iProxyEnabled;
        bool iFileInput;
        bool iBCS;

        OSCL_HeapString<OsclMemAllocator> iTestCaseName;

        uint32 iContextObject;
        uint32 iContextObjectRefValue;

        int32 iTestNumber;

        // Media IO Factory
        PVPlayerTestMioFactory* iMioFactory;
};


// test_base-based class which will run async tests on pvPlayer engine
class pvplayer_engine_test : public test_case,
            public pvplayer_async_test_observer
{
    public:
        pvplayer_engine_test(char *aFileName, PVMFFormatType aFileType, int32 aFirstTest, int32 aLastTest,
                             bool aCompV, bool aCompA, bool aFInput, bool aBCS, int32 aLogLevel, int32 aLogNode, int32 aLogText, int32 aLogMem, int32 aFileFormatType, bool aProxyEnabled);
        ~pvplayer_engine_test();

        // Note: for command line options to work, the local tests need to be 0-99,
        // Download tests 100-199,
        // Streaming tests 200-299.
        // Interactive test 800-899
        enum PVPlayerEngineAsyncTests
        {
            NewDeleteTest = 0,
            OpenPlayStopResetTest,
            OpenPlayStopResetCPMTest,
            MetaDataTest,
            TimingTest,
            InvalidStateTest,
            PreparedStopTest,
            VideoOnlyPlay7Seconds,
            Play5StopPlay10StopReset,
            PauseResume,

            PlayPauseStop = 10,
            OutsideNodeForVideoSink,
            GetPlayerState,
            GetCurrentPosition,
            PlaySetStopPosition,
            PlaySetStopPositionVidFrameNum,
            SetStartPositionPlayStop,
            SetPlayRangePlay,
            SetPlayRangeVidFrameNumPlay,
            PlaySetPlayRangeStop,

            PlaySetPlayRangeVidFrameNumStop = 20,
            TrackLevelInfoTest,
            SetPlaybackRate2X,
            SetPlaybackRateFifth,
            CapConfigInterfaceTest,
            QueuedCommandsTest,
            LoopingTest,
            WaitForEOSTest,
            MultiplePauseResumeTest,
            MultipleRepositionTest, // Start of local tests using media IO node

            MediaIONodeOpenPlayStopTest = 30,
            MediaIONodePlayStopPlayTest,
            MediaIONodePauseResumeTest,
            MediaIONodePlaySetPlaybackRangeTest,
            MediaIONodeSetPlaybackRate3XTest,
            MediaIONodeSetPlaybackRateHalfTest,
            MediaIONodeLoopingTest,
            MediaIONodeWaitForEOSTest,
            MediaIOMultiplePauseResumeTest,
            MediaIOMultipleRepositionTest,

            MediaIORepositionConfigTest = 40,
            MediaIONodeEOSLoopingTest,
            MediaIONodeRepositionDuringPreparedTest,
            MediaIONodePlaySetPlaybackRangeStopPlayTest,
            MediaIONodePlayStopSetPlaybackRangePlayStopTest,
            MediaIONodeSetPlaybackRangeNearEndStartTest,
            MediaIONodePlayRepositionNearEndOfClipTest,
            MediaIONodeForwardStepToEOSTest,
            MediaIONodeForwardStepTest,
            MediaIONodeForwardStepActiveAudioTest,
            MediaIONodeBackwardTest,

            MP4M4VAMRFileOpenPlayStopTest = 51, // Start of testing various local files
            MP4M4VAMRFilePlayStopPlayStopTest,
            MP4H263AMRFileOpenPlayStopTest,
            MP4H263AMRFilePlayStopPlayStopTest,
            MP4AVCAMRFileOpenPlayStopTest,
            MP4AVCAMRFilePlayStopPlayStopTest,
            MP4AMRFileOpenPlayStopTest,
            MP4AMRFilePlayStopPlayStopTest,
            MP4AACFileOpenPlayStopTest,

            MP4AACFilePlayStopPlayStopTest = 60,
            MP4M4VAMRTextFileOpenPlayStopTest,
            MP4M4VAMRTextFilePlayStopPlayStopTest,
            AMRIETFFileOpenPlayStopTest,
            AMRIETFFilePlayStopPlayStopTest,
            AMRIF2FileOpenPlayStopTest,
            AMRIF2FilePlayStopPlayStopTest,
            AACADTSFileOpenPlayStopTest,
            AACADTSFilePlayStopPlayStopTest,
            AACADIFFileOpenPlayStopTest,

            AACADIFFilePlayStopPlayStopTest = 70,
            AACRawFileOpenPlayStopTest,
            AACRawFilePlayStopPlayStopTest,
            MP3CBRFileOpenPlayStopTest,
            MP3CBRFilePlayStopPlayStopTest,
            MP3VBRFileOpenPlayStopTest,
            MP3VBRFilePlayStopPlayStopTest,
            WAVFileOpenPlayStopTest,
            WAVFilePlayStopPlayStopTest,
            ASFFileOpenPlayStopTest,
            ASFFilePlayStopPlayStopTest = 80,

            //real audio test case
            RealAudioFileOpenPlayStopTest = 81,
            SetPlaybackAfterPrepare,

            MediaIONodeBackwardForwardTest = 83,
            MediaIONodePauseNearEOSBackwardResumeTest,
            MediaIONodeMultiplePauseSetPlaybackRateResumeTest,
            MediaIONodeBackwardNearEOSForwardNearStartTest,

            MultiplePauseSeekResumeTest = 87,
            MultipleSetStartPositionPlayStopTest,

            OpenPlayStopResetCPMRecognizeTest = 89, //Start of testing recognizer using DataStream input

            LastLocalTest,//placeholder

            FirstDownloadTest = 100,  //placeholder

            FTDownloadOpenPlayStopTest = 101,

            ProgDownloadPlayAsapTest, //102
            ProgDownloadDownloadThenPlayTest, //103
            ProgDownloadDownloadOnlyTest, //104
            ProgDownloadCancelDuringInitTest, //105
            ProgDownloadContentTooLarge, //106
            ProgDownloadTruncated, //107
            ProgDownloadProtocolRolloverTest, //108
            ProgDownloadSetPlayBackRangeTest, //109
            ProgDownloadPlayUtilEOSTest, //110
            FTDownloadOpenPlayUntilEOSTest, //111

            ProgDownloadDownloadThenPlayPauseTest,//112
            ProgDownloadDownloadThenPlayRepositionTest,//113
            ProgDownloadCancelDuringInitDelayTest, //114
            ProgDownloadPauseResumeAfterUnderflowTest, //115

            FTDownloadPlayStopPlayTest, //116
            ProgDownloadPlayStopPlayTest, //117

            LastDownloadTest, //placeholder

            ProgPlaybackMP4UntilEOSTest = 150,
            ProgPlaybackMP4ShortTest, //151
            ProgPlaybackMP4ShortPauseResumeTest, //152
            ProgPlaybackMP4LongPauseResumeTest, //153
            ProgPlaybackMP4StartPauseSeekResumeTwiceTest, //154
            ProgPlaybackMP4SeekStartTest, //155
            ProgPlaybackMP4StartPauseSeekResumeLoopTest, //156
            ProgPlaybackMP4SeekForwardStepLoopTest,	//157
            ProgPlaybackPlayStopPlayTest, //158
            ProgPlaybackMP4SeekToBOCAfterDownloadCompleteTest, //159
            ProgPlaybackMP4SeekInCacheAfterDownloadCompleteTest, //160
            ProgPlaybackMP4EOSStopPlayAgainTest, //161

            LastProgressivePlaybackTest, //placeholder

            ShoutcastPlayback5MinuteTest = 180,
            ShoutcastPlaybackPauseResumeTest = 181,
            ShoutcastPlaybackPlayStopPlayTest = 182,

            LastShoutcastPlaybackTest, // placeholder

            FirstStreamingTest = 200, //placeholder

            StreamingOpenPlayStopTest, //201
            StreamingOpenPlayPausePlayStopTest, //202
            StreamingOpenPlaySeekStopTest, //203
            StreamingCancelDuringPrepareTest, //204

            LastStreamingTest, //placeholder

            //Multiple Instance tests.
            MultipleInstanceOpenPlayStopTest = 300,
            MultipleThreadOpenPlayStopTest = 301,
            //this range reserved for future multiple instance tests.
            LastMultipleInstanceTest = 325,//placeholder

            FirstProjTest = 700, // placeholder
            // Project specific unit tests should have numbers 701 to 799
            LastProjTest = 799,

            FirstInteractiveTest = 800, // placeholder

            PrintMetadataTest = 801,
            PrintMemStatsTest,
            PlayUntilEOSTest,
            ReleaseMetadataTest,

            StreamingOpenPlayUntilEOSTest = 851,//851
            StreamingOpenPlayPausePlayUntilEOSTest,//852
            StreamingOpenPlaySeekPlayUntilEOSTest, //853
            StreamingJitterBufferAdjustUntilEOSTest, //854
            StreamingCloakingOpenPlayUntilEOSTest, //855
            StreamingPlayBitStreamSwitchPlayUntilEOSTest,//856
            StreamingMultiplePlayUntilEOSTest,//857
            StreamingMultipleCloakingPlayUntilEOSTest, //858
            StreamingProtocolRollOverTest, //859
            StreamingProtocolRollOverTestWithUnknownURLType, //860
            StreamingPlayListSeekTest, //861
            StreamingSeekAfterEOSTest, //862
            /*
             * TC 863: This test case checks the engine behavior when the server responds
             * with any of the following error codes - 404 (URL Not Found), 415 (Media Unsupported),
             * 457 (Invalid Range). In all cases, the test should not error out, and complete as in TC 861
             */
            StreamingPlayListErrorCodeTest, // 863
            StreamingOpenPlayMultipleSeekToEndOfClipUntilEOSTest, //864


            StreamingOpenPlayMultiplePausePlayUntilEOSTest = 875, //875

            DVBH_StreamingOpenPlayStopTest = 876, //876
            DVBH_StreamingOpenPlayUntilEOSTest, //877
            StreamingLongPauseTest, //878

            CPM_DLA_OMA1PASSTRHU_OpenFailAuthPlayStopResetTest, //879
            CPM_DLA_OMA1PASSTRHU_OpenPlayStopResetTest, //880
            CPM_DLA_OMA1PASSTRHU_UnknownContentOpenPlayStopResetTest, //881

            StreamingOpenPlayUntilEOSTestWithFileHandle,//882
            //883-888 available

            //GetLicense returns commandCompleted before CancelLic could be triggered
            CPM_DLA_OMA1PASSTRHU_CancelAcquireLicenseTooLate_CancelFails = 889, //889
            //GetLicense does not commandComplete, cancelLic is triggered
            CPM_DLA_OMA1PASSTRHU_CancelAcquireLicense_CancelSucceeds, //890
            CPM_DLA_OMA1PASSTRHU_ContentNotSupported, //891

            StreamingOpenPlayForwardPlayUntilEOSTest, //892

            //Multiple CPM Plugins
            OpenPlayStop_MultiCPMTest, //893
            StreamingLongPauseSeekTest,	//894

            GenericReset_AddDataSource = 900,
            GenericReset_Init,
            GenericReset_AddDataSinkVideo,
            GenericReset_AddDataSinkAudio,
            GenericReset_Prepare,
            GenericReset_Start,
            GenericReset_Pause,
            GenericReset_Resume,
            GenericReset_Stop,
            //GenericReset_Reset,
            GenericReset_SetPlaybackRange,

            GenericDelete_AddDataSource = 910,
            GenericDelete_Init,
            GenericDelete_AddDataSinkVideo,
            GenericDelete_AddDataSinkAudio,
            GenericDelete_Prepare,
            GenericDelete_Start,
            GenericDelete_Pause,
            GenericDelete_Resume,
            GenericDelete_Stop,
            //GenericDelete_Reset,
            GenericDelete_SetPlaybackRange,

            GenericDeleteWhileProc_AddDataSource = 920,
            GenericDeleteWhileProc_Init,
            GenericDeleteWhileProc_AddDataSinkVideo,
            GenericDeleteWhileProc_AddDataSinkAudio,
            GenericDeleteWhileProc_Prepare,
            GenericDeleteWhileProc_Start,
            GenericDeleteWhileProc_Pause,
            GenericDeleteWhileProc_Resume,
            GenericDeleteWhileProc_Stop,
            //GenericDeleteWhileProc_Reset,
            GenericDeleteWhileProc_SetPlaybackRange,

            GenericCancelAll_AddDataSource = 930,
            GenericCancelAll_Init,
            GenericCancelAll_AddDataSinkVideo,
            GenericCancelAll_AddDataSinkAudio,
            GenericCancelAll_Prepare,
            GenericCancelAll_Start,
            GenericCancelAll_Pause,
            GenericCancelAll_Resume,
            GenericCancelAll_Stop,
            //GenericCancelAll_Reset,
            GenericCancelAll_SetPlaybackRange,

            GenericCancelAllWhileProc_AddDataSource = 940,
            GenericCancelAllWhileProc_Init,
            GenericCancelAllWhileProc_AddDataSinkVideo,
            GenericCancelAllWhileProc_AddDataSinkAudio,
            GenericCancelAllWhileProc_Prepare,
            GenericCancelAllWhileProc_Start,
            GenericCancelAllWhileProc_Pause,
            GenericCancelAllWhileProc_Resume,
            GenericCancelAllWhileProc_Stop,
            //GenericCancelAllWhileProc_Reset,
            GenericCancelAllWhileProc_SetPlaybackRange,

            // ACCESS DRM plugin tests
            FirstAccessCPMTest = 960,
            QueryEngine_AccessCPMTest,			//961
            OpenPlayStop_AccessCPMTest, 		//962
            PlayStopPlayStop_AccessCPMTest,		//963
            StartupMeasurement_AccessCPMTest, 	//964
            LastAccessCPMTest,

            GenericNetworkDisconnect_AddDataSource = 1051,
            GenericNetworkDisconnect_Init,
            GenericNetworkDisconnect_AddDataSinkVideo,
            GenericNetworkDisconnect_AddDataSinkAudio,
            GenericNetworkDisconnect_Prepare,
            GenericNetworkDisconnect_Start,
            GenericNetworkDisconnect_Pause,
            GenericNetworkDisconnect_Resume,
            GenericNetworkDisconnect_Stop,
            GenericNetworkDisconnect_SetPlaybackRange,

            GenericNetworkDisconnectWhileProc_AddDataSource = 1061,
            GenericNetworkDisconnectWhileProc_Init,
            GenericNetworkDisconnectWhileProc_AddDataSinkVideo,
            GenericNetworkDisconnectWhileProc_AddDataSinkAudio,
            GenericNetworkDisconnectWhileProc_Prepare,
            GenericNetworkDisconnectWhileProc_Start,
            GenericNetworkDisconnectWhileProc_Pause,
            GenericNetworkDisconnectWhileProc_Resume,
            GenericNetworkDisconnectWhileProc_Stop,
            GenericNetworkDisconnectWhileProc_SetPlaybackRange,

            GenericNetworkDisconnectReconnect_AddDataSource = 1071,
            GenericNetworkDisconnectReconnect_Init,
            GenericNetworkDisconnectReconnect_AddDataSinkVideo,
            GenericNetworkDisconnectReconnect_AddDataSinkAudio,
            GenericNetworkDisconnectReconnect_Prepare,
            GenericNetworkDisconnectReconnect_Start,
            GenericNetworkDisconnectReconnect_Pause,
            GenericNetworkDisconnectReconnect_Resume,
            GenericNetworkDisconnectReconnect_Stop,
            GenericNetworkDisconnectReconnect_SetPlaybackRange,

            GenericNetworkDisconnectReconnectWhileProc_AddDataSource = 1081,
            GenericNetworkDisconnectReconnectWhileProc_Init,
            GenericNetworkDisconnectReconnectWhileProc_AddDataSinkVideo,
            GenericNetworkDisconnectReconnectWhileProc_AddDataSinkAudio,
            GenericNetworkDisconnectReconnectWhileProc_Prepare,
            GenericNetworkDisconnectReconnectWhileProc_Start,
            GenericNetworkDisconnectReconnectWhileProc_Pause,
            GenericNetworkDisconnectReconnectWhileProc_Resume,
            GenericNetworkDisconnectReconnectWhileProc_Stop,
            GenericNetworkDisconnectReconnectWhileProc_SetPlaybackRange,

            GenericNetworkDisconnectCancelAll_AddDataSource = 1091,
            GenericNetworkDisconnectCancelAll_Init,
            GenericNetworkDisconnectCancelAll_AddDataSinkVideo,
            GenericNetworkDisconnectCancelAll_AddDataSinkAudio,
            GenericNetworkDisconnectCancelAll_Prepare,
            GenericNetworkDisconnectCancelAll_Start,
            GenericNetworkDisconnectCancelAll_Pause,
            GenericNetworkDisconnectCancelAll_Resume,
            GenericNetworkDisconnectCancelAll_Stop,
            GenericNetworkDisconnectCancelAll_SetPlaybackRange,

            GenericNetworkDisconnectCancelAllWhileProc_AddDataSource = 1101,
            GenericNetworkDisconnectCancelAllWhileProc_Init,
            GenericNetworkDisconnectCancelAllWhileProc_AddDataSinkVideo,
            GenericNetworkDisconnectCancelAllWhileProc_AddDataSinkAudio,
            GenericNetworkDisconnectCancelAllWhileProc_Prepare,
            GenericNetworkDisconnectCancelAllWhileProc_Start,
            GenericNetworkDisconnectCancelAllWhileProc_Pause,
            GenericNetworkDisconnectCancelAllWhileProc_Resume,
            GenericNetworkDisconnectCancelAllWhileProc_Stop,
            GenericNetworkDisconnectCancelAllWhileProc_SetPlaybackRange,

            GenericNetworkDisconnectCancelAllWhileProtocolRollover,

            GenericOpenPlayMultiplePauseResumeUntilEOSTest = 1112,
            GenericOpenPlayMultipleSeekUntilEOSTest,

            GenericOpenPlayStop_SleepAddDataSource = 1114,
            GenericOpenPlayStop_SleepInit,
            GenericOpenPlayStop_SleepAddDataSinkVideo,
            GenericOpenPlayStop_SleepAddDataSinkAudio,
            GenericOpenPlayStop_SleepPrepare,
            GenericOpenPlayStop_SleepGetMetaDataValueList,
            GenericOpenPlayStop_SleepStop,

            GenericOpenPlayPauseResumeSeekStopProfiling = 1125,
            GenericOpenPlayPauseRepositionResumeUntilEOSTest,
            GenericOpenPlayPauseRepositionResumeNetworkDisconnectCancelAllTest,
            GenericOpenSetPlaybackRangeStartPlayStopTest,
            GenericOpenPlayRepositionToEndTest,
            GenericPVMFErrorCorruptReNotified,
            GenericOpenPlayPauseGetMetaDataUntilEOSTest,
            GenericOpenGetMetaDataPicTest,//1132

            //1133-1149 available.

            //BEGIN JANUS CPM TESTS
            CleanDrmData_JanusCPMTest = 1150,
            LoadLicense_JanusCPMTest,//1151
            OpenPlayStop_JanusCPMTest, //1152
            PlayStopPlayStop_JanusCPMTest,//1153
            QueryEngine_JanusCPMTest,//1154
            StartupMeasurement_JanusCPMTest, //1155
            //Janus DLA tests
            DLA_CleanDrmData_JanusCPMTest,//1156
            DLA_OpenPlayStop_JanusCPMTest,//1157
            DLA_LicenseCapture_JanusCPMTest,//1158
            DLA_CancelAcquireLicense_JanusCPMTest,//1159
            //Janus streaming tests.
            DLA_StreamingOpenPlayUntilEOST_JanusCPMTest,//1160
            DLA_StreamingOpenPlayPausePlayUntilEOS_JanusCPMTest,//1161
            DLA_StreamingOpenPlaySeekPlayUntilEOS_JanusCPMTest,//1162
            DLA_StreamingMultiplePlayUntilEOS_JanusCPMTest,//1163
            DLA_StreamingProtocolRollOverTest_JanusCPMTest,//1164
            DLA_StreamingProtocolRollOverTestWithUnknownURLType_JanusCPMTest,//1165
            DLA_StreamingCancelAcquireLicense_JanusCPMTest,//1166
            //Janus PDL tests
            DLA_PDL_OpenPlayUntilEOS_JanusCPMTest,//1167

            //this range RESERVED for future Janus tests.

            FirstDLAStreamingTest = 1200, //placeholder
            //note these are all Janus CPM tests

            DLA_StreamingCancelAll_AddDataSource = 1201,
            DLA_StreamingCancelAll_Init,
            DLA_StreamingCancelAll_LicenseAcquired,
            DLA_StreamingCancelAll_AddDataSinkVideo,
            DLA_StreamingCancelAll_AddDataSinkAudio,
            DLA_StreamingCancelAll_Prepare,
            DLA_StreamingCancelAll_Start,
            DLA_StreamingCancelAll_Pause,
            DLA_StreamingCancelAll_Resume,
            DLA_StreamingCancelAll_Stop,
            DLA_StreamingCancelAll_SetPlaybackRange,

            DLA_StreamingCancelAllWhileProc_AddDataSource = 1212,
            DLA_StreamingCancelAllWhileProc_Init,
            DLA_StreamingCancelAllWhileProc_LicenseAcquired,
            DLA_StreamingCancelAllWhileProc_AddDataSinkVideo,
            DLA_StreamingCancelAllWhileProc_AddDataSinkAudio,
            DLA_StreamingCancelAllWhileProc_Prepare,
            DLA_StreamingCancelAllWhileProc_Start,
            DLA_StreamingCancelAllWhileProc_Pause,
            DLA_StreamingCancelAllWhileProc_Resume,
            DLA_StreamingCancelAllWhileProc_Stop,
            DLA_StreamingCancelAllWhileProc_SetPlaybackRange,

            DLA_StreamingNetworkDisconnect_AddDataSource = 1223,
            DLA_StreamingNetworkDisconnect_Init,
            DLA_StreamingNetworkDisconnect_LicenseAcquired,
            DLA_StreamingNetworkDisconnect_AddDataSinkVideo,
            DLA_StreamingNetworkDisconnect_AddDataSinkAudio,
            DLA_StreamingNetworkDisconnect_Prepare,
            DLA_StreamingNetworkDisconnect_Start,
            DLA_StreamingNetworkDisconnect_Pause,
            DLA_StreamingNetworkDisconnect_Resume,
            DLA_StreamingNetworkDisconnect_Stop,
            DLA_StreamingNetworkDisconnect_SetPlaybackRange,

            DLA_StreamingNetworkDisconnectWhileProc_AddDataSource = 1234,
            DLA_StreamingNetworkDisconnectWhileProc_Init,
            DLA_StreamingNetworkDisconnectWhileProc_LicenseAcquired,
            DLA_StreamingNetworkDisconnectWhileProc_AddDataSinkVideo,
            DLA_StreamingNetworkDisconnectWhileProc_AddDataSinkAudio,
            DLA_StreamingNetworkDisconnectWhileProc_Prepare,
            DLA_StreamingNetworkDisconnectWhileProc_Start,
            DLA_StreamingNetworkDisconnectWhileProc_Pause,
            DLA_StreamingNetworkDisconnectWhileProc_Resume,
            DLA_StreamingNetworkDisconnectWhileProc_Stop,
            DLA_StreamingNetworkDisconnectWhileProc_SetPlaybackRange,

            DLA_StreamingNetworkDisconnectReconnect_AddDataSource = 1245,
            DLA_StreamingNetworkDisconnectReconnect_Init,
            DLA_StreamingNetworkDisconnectReconnect_LicenseAcquired,
            DLA_StreamingNetworkDisconnectReconnect_AddDataSinkVideo,
            DLA_StreamingNetworkDisconnectReconnect_AddDataSinkAudio,
            DLA_StreamingNetworkDisconnectReconnect_Prepare,
            DLA_StreamingNetworkDisconnectReconnect_Start,
            DLA_StreamingNetworkDisconnectReconnect_Pause,
            DLA_StreamingNetworkDisconnectReconnect_Resume,
            DLA_StreamingNetworkDisconnectReconnect_Stop,
            DLA_StreamingNetworkDisconnectReconnect_SetPlaybackRange,

            DLA_StreamingNetworkDisconnectReconnectWhileProc_AddDataSource = 1256,
            DLA_StreamingNetworkDisconnectReconnectWhileProc_Init,
            DLA_StreamingNetworkDisconnectReconnectWhileProc_LicenseAcquired,
            DLA_StreamingNetworkDisconnectReconnectWhileProc_AddDataSinkVideo,
            DLA_StreamingNetworkDisconnectReconnectWhileProc_AddDataSinkAudio,
            DLA_StreamingNetworkDisconnectReconnectWhileProc_Prepare,
            DLA_StreamingNetworkDisconnectReconnectWhileProc_Start,
            DLA_StreamingNetworkDisconnectReconnectWhileProc_Pause,
            DLA_StreamingNetworkDisconnectReconnectWhileProc_Resume,
            DLA_StreamingNetworkDisconnectReconnectWhileProc_Stop,
            DLA_StreamingNetworkDisconnectReconnectWhileProc_SetPlaybackRange,

            DLA_StreamingNetworkDisconnectCancelAll_AddDataSource = 1267,
            DLA_StreamingNetworkDisconnectCancelAll_Init,
            DLA_StreamingNetworkDisconnectCancelAll_LicenseAcquired,
            DLA_StreamingNetworkDisconnectCancelAll_AddDataSinkVideo,
            DLA_StreamingNetworkDisconnectCancelAll_AddDataSinkAudio,
            DLA_StreamingNetworkDisconnectCancelAll_Prepare,
            DLA_StreamingNetworkDisconnectCancelAll_Start,
            DLA_StreamingNetworkDisconnectCancelAll_Pause,
            DLA_StreamingNetworkDisconnectCancelAll_Resume,
            DLA_StreamingNetworkDisconnectCancelAll_Stop,
            DLA_StreamingNetworkDisconnectCancelAll_SetPlaybackRange,

            DLA_StreamingNetworkDisconnectCancelAllWhileProc_AddDataSource = 1278,
            DLA_StreamingNetworkDisconnectCancelAllWhileProc_Init,
            DLA_StreamingNetworkDisconnectCancelAllWhileProc_LicenseAcquired,
            DLA_StreamingNetworkDisconnectCancelAllWhileProc_AddDataSinkVideo,
            DLA_StreamingNetworkDisconnectCancelAllWhileProc_AddDataSinkAudio,
            DLA_StreamingNetworkDisconnectCancelAllWhileProc_Prepare,
            DLA_StreamingNetworkDisconnectCancelAllWhileProc_Start,
            DLA_StreamingNetworkDisconnectCancelAllWhileProc_Pause,
            DLA_StreamingNetworkDisconnectCancelAllWhileProc_Resume,
            DLA_StreamingNetworkDisconnectCancelAllWhileProc_Stop,
            DLA_StreamingNetworkDisconnectCancelAllWhileProc_SetPlaybackRange,

            DLA_StreamingOpenPlayMultiplePauseResumeUntilEOSTest = 1289,
            DLA_StreamingOpenPlayMultipleSeekUntilEOSTest,

            DLA_StreamingOpenPlayStop_SleepAddDataSource = 1291,
            DLA_StreamingOpenPlayStop_SleepInit,
            DLA_StreamingOpenPlayStop_SleepLicenseAcquired,
            DLA_StreamingOpenPlayStop_SleepAddDataSinkVideo,
            DLA_StreamingOpenPlayStop_SleepAddDataSinkAudio,
            DLA_StreamingOpenPlayStop_SleepPrepare,
            DLA_StreamingOpenPlayStop_SleepStop = 1301,
            //END JANUS CPM TESTS

            GenericOpenPlaySeekBeyondClipDurationTest = 1302,

            //tests various modes of track selection
            //refer to test_pv_player_engine_testset_apptrackselection.h
            //for definition of modes
            ApplicationInvolvedTrackSelectionTestDefault = 1303,
            ApplicationInvolvedTrackSelectionTestPassthru = 1304,
            ApplicationInvolvedTrackSelectionTestAudioOnly = 1305,
            ApplicationInvolvedTrackSelectionTestVideoOnly = 1306,
            ApplicationInvolvedTrackSelectionTestTextOnly = 1307,
            ApplicationInvolvedTrackSelectionTestNoTracks = 1308,

            //Metadata Query tests
            DLA_QueryEngine_JupiterCPMTest_v2_WMA = 1400,
            DLA_QueryEngine_JupiterCPMTest_v24_WMA,//1401
            DLA_QueryEngine_JupiterCPMTest_v4_Enveloped_ASF,//1402
            DLA_QueryEngine_JupiterCPMTest_v4_Enveloped_MP4,//1403
            DLA_QueryEngine_JupiterCPMTest_v4_Enveloped_Image,//1404
            //Enveloped image tests
            DLA_EnvelopeRead_JupiterCPMTest_v4_Enveloped_Image,//1405
            //Multi-media format tests
            DLA_OpenPlayStop_JupiterCPMTest_v2_WMA,//1406
            DLA_OpenPlayStop_JupiterCPMTest_v2_WMV,//1407
            DLA_OpenPlayStop_JupiterCPMTest_v24_WMA,//1408
            DLA_OpenPlayStop_JupiterCPMTest_v24_WMV,//1409
            DLA_OpenPlayStop_JupiterCPMTest_v4_WMA,//1410
            DLA_OpenPlayStop_JupiterCPMTest_v4_WMV,//1411
            DLA_OpenPlayStop_JupiterCPMTest_v4_AAC,//1412
            DLA_OpenPlayStop_JupiterCPMTest_v4_H264,//1413
            DLA_OpenPlayStop_JupiterCPMTest_v4_Enveloped_ASF,//1414
            DLA_OpenPlayStop_JupiterCPMTest_v4_Enveloped_MP4,//1415
            DLA_OpenPlayStop_JupiterCPMTest_unprotected_H264,//1416
            DLA_OpenPlayStop_JupiterCPMTest_unprotected_AAC,//1417
            //Special license protocol sequence tests
            DLA_OpenPlayStop_JupiterCPMTest_redirect,//1418
            DLA_OpenPlayStop_JupiterCPMTest_v24_WMA_fallback,//1419
            DLA_OpenPlayStop_JupiterCPMTest_v4_WMA_ringtone,//1420
            DLA_OpenPlayStop_JupiterCPMTest_v4_WMA_domain,//1421
            DLA_OpenPlayStop_JupiterCPMTest_v4_WMA_domain_renew,//1422
            DLA_OpenPlayStop_JupiterCPMTest_v4_WMA_domain_offline,//1423
            DLA_OpenPlayStop_JupiterCPMTest_v4_WMA_domain_history,//1424
            //Jupiter utility tests
            DLA_JoinDomain_JupiterCPMTest,//1425
            DLA_LeaveDomain_JupiterCPMTest,//1426
            DLA_DeleteLicense_JupiterCPMTest_v2_Content,//1427
            DLA_DeleteLicense_JupiterCPMTest_v24_Content,//1428
            DLA_MeteringByCert_JupiterCPMTest,//1429
            DLA_MeteringByMID_JupiterCPMTest,//1430
            DLA_MeteringAll_JupiterCPMTest,//1431
            DLA_LicenseUpdateAll_JupiterCPMTest,//1432
            DLA_LicenseUpdateExpired_JupiterCPMTest,//1433
            //Jupiter CancelAcquireLicense tests
            DLA_CancelAcquireLicense_JupiterCPMTest_v4_Enveloped_Image,//1434
            DLA_CancelAcquireLicense_JupiterCPMTest_v2_Content,//1435
            DLA_CancelAcquireLicense_JupiterCPMTest_v24_Content,//1436
            DLA_CancelAcquireLicense_JupiterCPMTest_v4_Enveloped_MP4,//1437
            //Jupiter streaming tests.
            DLA_StreamingOpenPlayUntilEOST_JupiterCPMTest,//1438
            DLA_StreamingOpenPlayPausePlayUntilEOS_JupiterCPMTest,//1439
            DLA_StreamingOpenPlaySeekPlayUntilEOS_JupiterCPMTest,//1440
            DLA_StreamingMultiplePlayUntilEOS_JupiterCPMTest,//1441
            DLA_StreamingCancelAcquireLicense_JupiterCPMTest,//1442
            DLA_StreamingProtocolRollOverTest_JupiterCPMTest,//1443
            DLA_StreamingProtocolRollOverTestWithUnknownURLType_JupiterCPMTest,//1444
            //RESERVED FOR FUTURE JUPITER CPM TESTS.
            LastJupiterCPMTest = 1599,//placeholder
            //End JUPITER CPM TESTS.
            /*
             * Note: Starting PVR tests from 1600 since the number of tests exceed the range
             * from 895 and 900 (the earlier location of the tests)
             */
            /*
             * =========== Basic PVR tests ===========
             */

            /*
             * 1600:        Play->pause->resume->stop test
             * Description: Self-explanatory
             */

            /**
            PVR Memory Live Buffer tests start at 1600
            */

            PVR_MLB_StreamingOpenPlayLivePausePlayStopTest = 1600,//1600
            PVR_MLB_StreamingOpenPlayLivePausePlaySeekStopTest,//1601
            PVR_MLB_StreamingOpenPlayLiveBufferBoundaryCheckTest,//1602
            /*
             * 1601:        Reposition after a play pause
             * Description: See TC 1611
             */

            /*
             * 1602:        Check live buffer boundary
             * Description: To maintain TC numbers, this test case
             *              does a play followed by a stop.
             */

            /*
             * =========== Live pause tests ===========
             */

            /*
             * 1603:        Quick successive pause/resume
             * Description: This test case will call pause/resume in a quick and
             *              repetitive sequence to test quick successive pause/resume stress condition.
             * Input:       1. repeat count
             */
            PVR_MLB_StreamingOpenPlayMultipleLivePausePlayTest,//1603
            /*
             * 1604:        Pause with random duration
             * Description: This test case will call pause/resume repeatedly.
             *              Each pause has random duration in a range.
             * Inputs:      1. repeat count.
             *              2. pause duration range
             */
            PVR_MLB_StreamingOpenPlayMultipleLivePauseRandomDurationPlayTest,//1604
            /*
             * 1605:        Long pause
             * Description: This test case will pause long period of time (> live buffer duration)
             * Input:       1. pause duration
             */
            PVR_MLB_StreamingOpenPlayLongLivePausePlayTest,//1605
            /*
             * 1606:        Stop during live pause
             * Description: This test case stop the session during live pause.
             *              Then it starts the same session again.
             */
            PVR_MLB_StreamingOpenPlayLivePauseStopTest,//1606

            /*
             * =========== Random position tests ===========
             */

            /*
             * 1607:        Jump to live after pause
             * Description: This test case jumps to live session (not buffer) after a pause.
             *				The above step is repeated several times, after pausing for a random duration.
             * Inputs:      1. Repeat count
             *              2. Pause duration range
             */
            PVR_MLB_StreamingOpenPauseJumpToLiveStopTest, //1607
            /*
             * 1608:        Jump to live after pause/resume
             * Description: This test case pauses a live session for a random duration, resumes
             *              a for random duration that is less than the pause duration, and then
             *              jumps to live. This is repeated several times.
             * Inputs:      1. Repeat count
             *              2. Pause duration range
             *              3. Resume duration range
             */
            PVR_MLB_StreamingOpenPauseResumeJumpToLiveStopTest, //1608
            /*
             * 1609:        Stop during playing back from live buffer
             * Description: This test case pauses a live session for a random duration, resumes
             *              and then stops the session while playing from the live buffer. The same
             *              session is started after the stop.
             */
            PVR_MLB_StreamingOpenPauseResumeStopTest, //1609
            /*
             * 1610:        RTSP URL test
             * Description: This test case uses an RTSP URL as the input for the streaming session.
             *              Both live and VOD cases are handled.
             */
            PVR_MLB_StreamingRTSPUrlPauseResumeStopTest, //1610

            /*
             * 1611:        PVR Repositioning test
             * Description: This test case does a sequence of repositioning requests within the live buffer.
             *              The followed sequence is: start, pause, resume, repositioning sequence, stop.
            				The repos. sequence consists of 6 repositioning requests back and forward.
             */
            PVR_MLB_StreamingPauseResumeRepositionStopTest, //1611

            /*
            * 1612:        PVR Repositioning test
            * Description: This test case does a sequence of repositioning requests within the live buffer.
            *              The followed sequence is: start, pause, repositioning, resume. This sequence is
               			done six times, followed by a stop.
            */
            PVR_MLB_StreamingPauseRepositionResumeStopTest, //1612

            /*
             * 1613:        RTSP END_OF_STREAM test for memory live buffer.
             * Description: This test is design to test the case where and RTSP END_OF_STREAM arrives
             *              during a live pause. Total playback time must equal the cumulative playlist clip
             *              duration.
             * Input:
             */
            PVR_MLB_StreamingOpenPlayLivePauseWaitForRTSPEOSResumeTest = 1613,//1613


            /*
            * 1614:        PVR Playlist switch with a full live buffer after JTL
            * Description: This test case makes a live pause it resumes once the live buffer gets full, and makes
            *              a JTL followed by a channel switch, the channel switch time is measured.

            */
            PVR_MLB_StreamingOpenPauseJmpToLiveChannelSwitchTest, //1614



            /*
            * 1615       PVR Bitrate estimation test
            * Description: Test case for mismatch between advertised and real bitrate,
            *              the root of the problem with higher-than-advertised bitrates is that it
            *              may cause overwrites in the LiveBuffer, which is then reflected in quality artifacts.
            *
            *              PLEASE USE A SDP FILE THAT ANOUNCES A SMALLER BITRATE THAN THE REAL ONE

            */
            PVR_MLB_StreamingBitrateEstimationTest = 1615,

            /**
            PVR FileSystem Live Buffer tests start at 1630
            */
            PVR_FSLB_StreamingOpenPlayLivePausePlayStopTest = 1630,//1630
            PVR_FSLB_StreamingOpenPlayLivePausePlaySeekStopTest,//1631
            PVR_FSLB_StreamingOpenPlayLiveBufferBoundaryCheckTest,//1632
            /*
             * =========== Live pause tests ===========
             */

            /*
             * 1633:        Quick successive pause/resume
             * Description: This test case will call pause/resume in a quick and
             *              repetitive sequence to test quick successive pause/resume stress condition.
             * Input:       1. repeat count
             */
            PVR_FSLB_StreamingOpenPlayMultipleLivePausePlayTest,//1633
            /*
             * 1634:        Pause with random duration
             * Description: This test case will call pause/resume repeatedly.
             *              Each pause has random duration in a range.
             * Inputs:      1. repeat count.
             *              2. pause duration range
             */
            PVR_FSLB_StreamingOpenPlayMultipleLivePauseRandomDurationPlayTest,//1634
            /*
             * 1635:        Long pause
             * Description: This test case will pause long period of time (> live buffer duration)
             * Input:       1. pause duration
             */
            PVR_FSLB_StreamingOpenPlayLongLivePausePlayTest,//1635
            /*
             * 1636:        Stop during live pause
             * Description: This test case stop the session during live pause.
             *              Then it starts the same session again.
             */
            PVR_FSLB_StreamingOpenPlayLivePauseStopTest,//1636

            /*
             * =========== Random position tests ===========
             */

            /*
             * 1637:        Jump to live after pause
             * Description: This test case jumps to live session (not buffer) after a pause.
             *				The above step is repeated several times, after pausing for a random duration.
             * Inputs:      1. Repeat count
             *              2. Pause duration range
             */
            PVR_FSLB_StreamingOpenPauseJumpToLiveStopTest, //1637
            /*
             * 1638:        Jump to live after pause/resume
             * Description: This test case pauses a live session for a random duration, resumes
             *              a for random duration that is less than the pause duration, and then
             *              jumps to live. This is repeated several times.
             * Inputs:      1. Repeat count
             *              2. Pause duration range
             *              3. Resume duration range
             */
            PVR_FSLB_StreamingOpenPauseResumeJumpToLiveStopTest, //1638
            /*
             * 1639:        Stop during playing back from live buffer
             * Description: This test case pauses a live session for a random duration, resumes
             *              and then stops the session while playing from the live buffer. The same
             *              session is started after the stop.
             */
            PVR_FSLB_StreamingOpenPauseResumeStopTest, //1639

            /*
             * 1640:        RTSP URL test
             * Description: This test case uses an RTSP URL as the input for the streaming session.
             *              Both live and VOD cases are handled.
             */
            PVR_FSLB_StreamingRTSPUrlPauseResumeStopTest, //1640

            /*
             * 1641:        PVR Repositioning test
             * Description: This test case does a sequence of repositioning requests within the live buffer.
             *              The followed sequence is: start, pause, resume, repositioning sequence, stop.
            				The repos. sequence consists of 6 repositioning requests back and forward.
             */
            PVR_FSLB_StreamingPauseResumeRepositionStopTest, //1641

            /*
            * 1642:        PVR Repositioning test
            * Description: This test case does a sequence of repositioning requests within the live buffer.
            *              The followed sequence is: start, pause, repositioning, resume. This sequence is
               			done six times, followed by a stop.
            */
            PVR_FSLB_StreamingPauseRepositionResumeStopTest, //1642

            /*
            * 1643:        RTSP END_OF_STREAM test for file system live buffer.
            * Description: Same as 1643.
            * Input:
            */

            PVR_FSLB_StreamingOpenPlayLivePauseWaitForRTSPEOSResumeTest, //1643

            /*
            * 1644:        PVR Playlist switch with a full live buffer after JTL
            * Description: This test case makes a live pause it resumes once the live buffer gets full, and makes
            *              a JTL followed by a channel switch, the channel switch time is measured.

            */
            PVR_FSLB_StreamingOpenPauseJmpToLiveChannelSwitchTest, //1644


            /**
            PVR local file playback tests start at 1660
            */

            PVR_FILEPLAYBACK_OpenPlayUntilEOFTest = 1660,// 1660

            /*
            * 1661       PVR Local Playback Open-Play-Reposition-Play-Stop
            * Description: This test case plays a local pvr file for 10 seconds, then
            *              it repositions to 50, 75, 40, 0, 60 and 95% of the local file
            *              and then it stops playback.
            *

            */

            PVR_FILEPLAYBACK_OpenPlayRepositionPlayStopTest,  // 1661


            /*
            * 1662       PVR Local Playback Open-Play-Pause-Reposition
            * Description: This test case plays a local pvr file for 10 seconds, then pauses playback
            *              it repositions to 50, 75, 40, 0, 60 and 95% of the local file
            *              and then it stops playback.
            *

            */

            PVR_FILEPLAYBACK_OpenPlayPauseRepositionTest,  // 1662

            /*
            * 1663       PVR Local Playback Retrieved duration is correct
            * Description: This test case plays a local pvr file for 3 seconds and
            *              verifies if the retrieved duration is correct
            *

            */

            PVR_FILEPLAYBACK_OpenPlayVerifyDurationTest,  // 1663




            /*===========================PVR Recorder Test cases============================*/

            /*
            * 1680       PVR_Recorder_RecordOnStartUp
            * Description: - Recording will start from NPT zero until playback is stopped.
            *              - Pass criteria will be: No playback or recording errors.
            */

            PVR_Recorder_RecordOnStartUp = 1680,

            /*
            * 1681       PVR_Recorder_RecordOnDemand
            * Description: - Recording will start from an arbitrary NPT different than zero to a point before playback is stopped.
            *              - Pass criteria will be: No playback or recording errors.
            */

            PVR_Recorder_RecordOnDemand,

            /*
            * 1682       PVR_Recorder_MultiplePause
            * Description: - Recording will start from NPT zero.
            *              - Multiple pause/resume resume will be performed
            *              - Pass criteria will be: No playback or recording errors.
            */

            PVR_Recorder_MultiplePause,

            /*
            * 1683       PVR_Recorder_MultiplePauseJTL
            * Description: - Recording will start from NPT zero.
            *              - Multiple pause/resume will be performed
            *              - After last pause JTL
            *              - Pass criteria will be: No playback or recording errors.
            */

            PVR_Recorder_MultiplePauseJTL,

            /*
            * 1684       PVR_Recorder_MultipleRepos
            * Description: - Recording will start from NPT zero.
            *              - Multiple Reposition/resume will be performed
            *              - Pass criteria will be: No playback or recording errors.
            */

            PVR_Recorder_MultipleRepos,

            /*
            * 1685       PVR_Recorder_MultipleRecord
            * Description: - Recording will start from an arbitrary NPT different than zero.
            *              - Multiple StartRecord/StopRecord will be performed
            *              - Pass criteria will be: No playback or recording errors
            */

            PVR_Recorder_MultipleRecord,

            LastInteractiveTest = 2000, // placeholder


            BeyondLastTest = 9999 //placeholder
        };

        // From test_case
        virtual void test();

        // From pvplayer_async_test_observer
        void TestCompleted(test_case&);

        //Validating file format specific player test cases
        bool ValidateTestCase(int& aCurrentTestCaseNumber);

        void SetupLoggerScheduler();

    private:
        const char *iFileName;
        PVMFFormatType iFileType;

        int iCurrentTestNumber;
        pvplayer_async_test_base* iCurrentTest;
        int32 iFirstTest;
        int32 iLastTest;
        bool iCompressedVideoOutput;
        bool iCompressedAudioOutput;
        bool iFileInput;
        bool iBCS;
        //for proxy support
        bool iProxyEnabled;

        // For test results
        int iTotalSuccess;
        int iTotalError;
        int iTotalFail;

        // For logging
        int32 iLogLevel;
        int32 iLogNode;
        int32 iLogFile;
        int32 iLogMem;

        // For memory statistics
        uint32 iTotalAlloc;
        uint32 iTotalBytes;
        uint32 iAllocFails;
        uint32 iNumAllocs;
        //Flag for checking the file type
        int32 iFileFormatType;
};

/**
 * Factory class for project specific unit test cases
 */
class PVPlayerProjTestFactory
{
    public:
        /**
         * Creates customer project test case object as specified by test case number.  Default
         * implementation returns NULL.  Customer projects that wish to extend pvPlayer unit
         * test to implement customer specific test cases should implement this function
         *
         * @param aTestCaseNumber - Unit test case number
         * @param aTestParam - Parameters to the unit test case
         * @return Newly created project specific test case object, or NULL if the test case
         *              number is not supported.
         */
        virtual pvplayer_async_test_base* GetProjTest(uint aTestCaseNumber, PVPlayerAsyncTestParam aTestParam)
        {
            OSCL_UNUSED_ARG(aTestCaseNumber);
            OSCL_UNUSED_ARG(aTestParam);
            return NULL;
        }
};

#endif


