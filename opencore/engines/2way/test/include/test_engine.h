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
#ifndef TEST_ENGINE_H_INCLUDED
#define TEST_ENGINE_H_INCLUDED

#include "oscl_error.h"
#include "oscl_timer.h"
#include "oscl_mem.h"
#include "oscl_scheduler.h"
#include "oscl_utf8conv.h"
#include "pvlogger.h"
#include "pvlogger_stderr_appender.h"
#include "pvlogger_file_appender.h"
#include "pvlogger_time_and_id_layout.h"
#include "test_case.h"
#include "text_test_interpreter.h"
#include "pv_2way_interface.h"
#include "pv_2way_engine_factory.h"
#include "pv_2way_proxy_factory.h"
#include "pvmf_media_input_node_factory.h"
#include "pv_media_output_node_factory.h"
#include "pvmi_media_io_fileoutput.h"
#include "pvmi_mio_fileinput_factory.h"
#include "pv_engine_observer.h"
#include "pv_engine_observer_message.h"
#include "tsc_h324m_config_interface.h"

#ifndef NO_2WAY_324
#include "pv_comms_io_node_factory.h"
#include "pvmi_mio_comm_loopback_factory.h"
#endif


#define RX_LOGGER_TAG _STRLIT_CHAR("pvcommionode.rx.bin")
#define TX_LOGGER_TAG _STRLIT_CHAR("pvcommionode.tx.bin")
#define PVSIP2WAY_PROFILE _STRLIT_CHAR("pvSIP2Way")
#define PVSIPDEMO_ADDRESS _STRLIT_CHAR("sip:pvSIPDemo@")
#define PVSIPDEMO2_ADDRESS _STRLIT_CHAR("sip:pvSIPDemo2@")
#define PVSIP_DEFAULT_REALM _STRLIT_CHAR("pvrealm")


#define TEST_RX_LOG_FILENAME _STRLIT("commrx.bin")
#define TEST_TX_LOG_FILENAME _STRLIT("commtx.bin")
#define TEST_LOG_FILENAME _STRLIT("test2way.log")
#define AUDIO_SOURCE_FILENAME _STRLIT("audio_in.if2")
#define AUDIO_SOURCE3_FILENAME _STRLIT("audio_in.amr")
#define AUDIO_SOURCE_RAW_FILENAME _STRLIT("pcm16testinput.pcm")
#define AUDIO_SINK_FILENAME _STRLIT("audio_if2_out.dat")
#define AUDIO_SINK_RAW_FILENAME _STRLIT("audio_pcm16_out.dat")
#define AUDIO_SINK2_FILENAME _STRLIT("audio_ietf_out.dat")
#define VIDEO_SOURCE_YUV_FILENAME _STRLIT("yuv420video.yuv")
#define VIDEO_SOURCE_H263_FILENAME _STRLIT("h263video.h263")
#define VIDEO_SOURCE_M4V_FILENAME _STRLIT("m4vvideo.m4v")
#define VIDEO_SINK_YUV_FILENAME _STRLIT("video_yuv_out.dat")
#define VIDEO_SINK_H263_FILENAME _STRLIT("video_h263_out.dat")
#define VIDEO_SINK_M4V_FILENAME _STRLIT("video_m4v_out.dat")
#define VIDEO_PREVIEW_FILENAME _STRLIT("video_preview_out.dat")
#define RECORDED_CALL_FILENAME _STRLIT("recorded_call.mp4")
#define AUDIO_ONLY_PLAY_FILENAME _STRLIT("pv-amr-122_novisual.3gp")
#define AUDIO_H263_PLAY_FILENAME _STRLIT("pv-amr-122_h263-64.3gp")
#define AUDIO_MPEG4_PLAY_FILENAME _STRLIT("pv2-amr122_mpeg4-rvlcs-64.3gp")
#define H263_ONLY_PLAY_FILENAME _STRLIT("pv-noaudio_h263-64.3gp")
#define MPEG4_ONLY_PLAY_FILENAME _STRLIT("pv2-noaudio_mpeg4-rvlcs-64.3gp")
#define SQCIF_PLAY_FILENAME _STRLIT("sqcif1.3gp")
#define QVGA_PLAY_FILENAME _STRLIT("qvga.3gp")



extern FILE *fileoutput;


template<class DestructClass>
class TwoWayLogAppenderDestructDealloc : public OsclDestructDealloc
{
    public:
        virtual void destruct_and_dealloc(OsclAny *ptr)
        {
            delete((DestructClass*)ptr);
        }
};

class engine_test_suite : public test_case
{
    public:
        engine_test_suite();

    private:
        void proxy_tests(const bool aProxy);
        void play_from_file_tests(const bool aProxy,
                                  const OSCL_wString& aFilename,
                                  const bool aHasAudio,
                                  const bool aHasVideo);
};

class engine_timer;

//test function oscl_str_is_valid_utf8
class engine_test : public test_case,
            public OsclActiveObject,
            public PVCommandStatusObserver,
            public PVInformationalEventObserver,
            public PVErrorEventObserver
{
    public:
        engine_test(bool aUseProxy = false,
                    int aMaxRuns = 1) : OsclActiveObject(OsclActiveObject::EPriorityNominal, "Test Engine"),
                iAudioSourceAdded(false),
                iAudioAddSourceId(0),
                iAudioRemoveSourceId(0),
                iAudioPauseSourceId(0),
                iAudioResumeSourceId(0),
                iAudioSinkAdded(false),
                iAudioAddSinkId(0),
                iAudioAddSink2Id(0),
                iAudioRemoveSinkId(0),
                iAudioPauseSinkId(0),
                iAudioResumeSinkId(0),
                iVideoSourceAdded(false),
                iVideoAddSourceId(0),
                iVideoRemoveSourceId(0),
                iVideoPauseSourceId(0),
                iVideoResumeSourceId(0),
                iVideoSinkAdded(false),
                iVideoAddSinkId(0),
                iVideoAddSink2Id(0),
                iVideoRemoveSinkId(0),
                iVideoPauseSinkId(0),
                iVideoResumeSinkId(0),
                iUseProxy(aUseProxy),
                iMaxRuns(aMaxRuns),
                iCurrentRun(0),
                iCommServer(NULL),
                iSelAudioSource(NULL),
                iSelAudioSink(NULL),
                iSelVideoSource(NULL),
                iSelVideoSink(NULL),
#ifndef NO_2WAY_324
                iCommServerIOControl(NULL),
#endif
                iAudioSource(NULL),
                iAudioSourceRaw(NULL),
                iAudioSource2(NULL),
                iAudioSource3(NULL),
                iAudioSourceIOControl(NULL),
                iAudioSourceRawIOControl(NULL),
                iAudioSource2IOControl(NULL),
                iAudioSource3IOControl(NULL),
                iGetSessionParamsId(0),
                iVideoSourceYUV(NULL),
                iVideoSourceH263(NULL),
                iVideoSourceM4V(NULL),
                iAudioSink(NULL),
                iAudioSinkRaw(NULL),
                iAudioSink2(NULL),
                iVideoSinkYUV(NULL),
                iVideoSinkH263(NULL),
                iVideoSinkM4V(NULL),
                iDuplicatesStarted(false),
                iVideoPreview(NULL),
                //iVideoPreviewIOControl(NULL),
                terminal(NULL),
                scheduler(NULL),
                timer(NULL),
                timer_elapsed(false),
                early_close(false),
                iTestStatus(true)
        {
            iConnectOptions.iLoopbackMode = PV_LOOPBACK_MUX;
            iRstCmdId = 0;
            iDisCmdId = 0;
            iConnectCmdId = 0;
            iInitCmdId = 0;
            iCommsAddSourceId = 0;
        }

        virtual ~engine_test()
        {
        }

        virtual void test() = 0;

        virtual void Run() = 0;

        virtual void DoCancel() = 0;

        void HandleErrorEvent(const PVAsyncErrorEvent& /*aEvent*/)
        {
        }

        virtual void HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent) = 0;

        virtual void CommandCompleted(const PVCmdResponse& aResponse) = 0;

        virtual void TimerCallback() {};

        static char iProfileName[32];
        static char iPeerAddress[64];
        static uint32 iMediaPorts[2];

    protected:


        PVCommandId iRstCmdId, iDisCmdId, iConnectCmdId, iInitCmdId;

        PVCommandId iCommsAddSourceId;

        bool iAudioSourceAdded;
        PVCommandId iAudioAddSourceId;
        PVCommandId iAudioAddSource2Id;
        PVCommandId iAudioRemoveSourceId;
        PVCommandId iAudioPauseSourceId;
        PVCommandId iAudioResumeSourceId;

        bool iAudioSinkAdded;
        PVCommandId iAudioAddSinkId;
        PVCommandId iAudioAddSink2Id;
        PVCommandId iAudioRemoveSinkId;
        PVCommandId iAudioPauseSinkId;
        PVCommandId iAudioResumeSinkId;

        bool iVideoSourceAdded;
        PVCommandId iVideoAddSourceId;
        PVCommandId iVideoAddSource2Id;
        PVCommandId iVideoRemoveSourceId;
        PVCommandId iVideoPauseSourceId;
        PVCommandId iVideoResumeSourceId;

        bool iVideoSinkAdded;
        PVCommandId iVideoAddSinkId;
        PVCommandId iVideoAddSink2Id;
        PVCommandId iVideoRemoveSinkId;
        PVCommandId iVideoPauseSinkId;
        PVCommandId iVideoResumeSinkId;

        virtual void disconnect()
        {
            int error = 0;
            OSCL_TRY(error, iDisCmdId = terminal->Disconnect());
            if (error)
            {
                reset();
            }
        }

        virtual void reset()
        {
            int error = 0;
            OSCL_TRY(error, iRstCmdId = terminal->Reset());
            if (error)
            {
                RunIfNotReady();
            }
        }

        virtual void connect()
        {
            int error = 0;
            OSCL_TRY(error, iConnectCmdId = terminal->Connect(iConnectOptions, iCommServer));
            if (error)
            {
                reset();
            }
        }

        virtual void printFormatString(PVMFFormatType aFormatType)
        {
            fprintf(fileoutput, "%s", aFormatType.getMIMEStrPtr());
        }

        bool check_audio_started()
        {
            return (iAudioSourceAdded && iAudioSinkAdded);
        }
        bool check_audio_stopped()
        {
            return (!iAudioSourceAdded && !iAudioSinkAdded);
        }
        bool check_video_started()
        {
            return (iVideoSourceAdded && iVideoSinkAdded);
        }
        bool check_video_stopped()
        {
            return (!iVideoSourceAdded && !iVideoSinkAdded);
        }


        PVMFNodeInterface *get_audio_source(PVMFFormatType format);
        PVMFNodeInterface *get_audio_sink(PVMFFormatType format);
        PVMFNodeInterface *get_video_source(PVMFFormatType format);
        PVMFNodeInterface *get_video_sink(PVMFFormatType format);

        void create_sink_source();
        void destroy_sink_source();

        void init_mime_strings();

        bool iUseProxy;
        int iMaxRuns;
        int iCurrentRun;
        PVMFNodeInterface* iCommServer;
        PVMFNodeInterface* iSelAudioSource;
        PVMFNodeInterface* iSelAudioSink;
        PVMFNodeInterface* iSelVideoSource;
        PVMFNodeInterface* iSelVideoSink;
#ifndef NO_2WAY_324
        PvmiMIOControl* iCommServerIOControl;
        PvmiMIOCommLoopbackSettings iCommSettings;
#endif
        PV2Way324ConnectOptions iConnectOptions;
        //CPV2WaySIPConnectInfo iSIPConnectOptions;
        PV2Way324InitInfo iSdkInitInfo;
        //CPV2WaySIPInitInfo iSdkSIPInitInfo;

        PvmiMIOFileInputSettings iAudioSourceRawFileSettings;
        PvmiMIOFileInputSettings iAudioSource2FileSettings;
        PvmiMIOFileInputSettings iAudioSource3FileSettings;
        PvmiMIOFileInputSettings iAudioSourceFileSettings;
        PVMFNodeInterface* iAudioSource;
        PVMFNodeInterface* iAudioSourceRaw;
        PVMFNodeInterface* iAudioSource2;
        PVMFNodeInterface* iAudioSource3;
        PvmiMIOControl* iAudioSourceIOControl;
        PvmiMIOControl* iAudioSourceRawIOControl;
        PvmiMIOControl* iAudioSource2IOControl;
        PvmiMIOControl* iAudioSource3IOControl;

        PVCommandId iGetSessionParamsId;
        PvmiMIOFileInputSettings iVideoSourceYUVFileSettings;
        PvmiMIOFileInputSettings iVideoSourceH263FileSettings;
        PvmiMIOFileInputSettings iVideoSourceM4VFileSettings;
        PVMFNodeInterface* iVideoSourceYUV;
        PVMFNodeInterface* iVideoSourceH263;
        PVMFNodeInterface* iVideoSourceM4V;
        PvmiMIOControl* iVideoSourceYUVIOControl;
        PvmiMIOControl* iVideoSourceH263IOControl;
        PvmiMIOControl* iVideoSourceM4VIOControl;


        const oscl_wchar* iAudioSinkFileName;
        const oscl_wchar* iAudioSinkRawFileName;
        const oscl_wchar* iAudioSink2FileName;
        PVMFNodeInterface* iAudioSink;
        PVMFNodeInterface* iAudioSinkRaw;
        PVMFNodeInterface* iAudioSink2;
        PVRefFileOutput* iAudioSinkIOControl;
        PVRefFileOutput* iAudioSinkRawIOControl;
        PVRefFileOutput* iAudioSink2IOControl;

        const oscl_wchar* iVideoSinkYUVFileName;
        const oscl_wchar* iVideoSinkH263FileName;
        const oscl_wchar* iVideoSinkM4VFileName;
        PVMFNodeInterface* iVideoSinkYUV;
        PVMFNodeInterface* iVideoSinkH263;
        PVMFNodeInterface* iVideoSinkM4V;
        PVRefFileOutput* iVideoSinkYUVIOControl;
        PVRefFileOutput* iVideoSinkH263IOControl;
        PVRefFileOutput* iVideoSinkM4VIOControl;
        bool iDuplicatesStarted;
        OSCL_wHeapString<OsclMemAllocator> iVideoPreviewFileName;
        PVMFNodeInterface* iVideoPreview;
        //PVRefFileOutput* iVideoPreviewIOControl;

        CPV2WayInterface *terminal;
        OsclExecScheduler *scheduler;
        engine_timer *timer;
        bool timer_elapsed;
        bool early_close;

        CPV2WayH263ConfigInfo iH263ConfigInfo;
        CPV2WayM4VConfigInfo iM4VConfigInfo;

        bool iTestStatus;
};

class engine_timer : public OsclTimerObject
{
    public:
        engine_timer(engine_test *aObserver) : OsclTimerObject(OsclActiveObject::EPriorityNominal, "Test Engine Timer"),
                iObserver(aObserver)
        {};

        ~engine_timer()
        {
            Cancel();
        }

    protected:
        void Run()
        {
            iObserver->TimerCallback();
        }
        void DoCancel()
        {
            OsclTimerObject::DoCancel();
        };

        engine_test *iObserver;

};



#endif


