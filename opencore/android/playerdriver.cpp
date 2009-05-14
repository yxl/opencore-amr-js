/* playerdriver.cpp
**
** Copyright 2007, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

//#define LOG_NDEBUG 0
#define LOG_TAG "PlayerDriver"
#include <utils/Log.h>

#include <sys/prctl.h>
#include <sys/resource.h>
#include <media/thread_init.h>
#include <utils/threads.h>
#include <utils/List.h>

#include <media/MediaPlayerInterface.h>

#include "playerdriver.h"
#include <media/PVPlayer.h>

#include "media_clock_converter.h"
#include "oscl_scheduler.h"
#include "oscl_scheduler_ao.h"
#include "oscl_exception.h"

#include "pv_player_factory.h"
#include "pv_player_interface.h"
#include "pv_engine_observer.h"
#include "pvmi_media_io_fileoutput.h"
#include "pv_player_datasourceurl.h"
#include "pv_player_datasinkfilename.h"
#include "pv_player_datasinkpvmfnode.h"
#include "pvmf_errorinfomessage_extension.h"
#include "pvmf_duration_infomessage.h"
#include "oscl_mem.h"
#include "oscl_mem_audit.h"
#include "oscl_error.h"
#include "oscl_utf8conv.h"
#include "oscl_string_utils.h"
#include "android_surface_output.h"
#include "android_audio_output.h"
#include "android_audio_stream.h"
#include "pv_media_output_node_factory.h"
#include "pvmf_node_interface.h"
#include "pvmf_source_context_data.h"
#include "pvmf_download_data_source.h"
#include "OMX_Core.h"
#include "pv_omxcore.h"

// color converter
#include "cczoomrotation16.h"

//for KMJ DRM Plugin
#include "pvmf_local_data_source.h"
#include "pvmf_recognizer_registry.h"
#include "pvoma1_kmj_recognizer.h"
#include "pvmf_cpmplugin_kmj_oma1.h"


using namespace android;

# ifndef PAGESIZE
#  define PAGESIZE              4096
# endif

// library and function name to retrieve device-specific MIOs
static const char* MIO_LIBRARY_NAME = "libopencorehal";
static const char* VIDEO_MIO_FACTORY_NAME = "createVideoMio";
typedef AndroidSurfaceOutput* (*VideoMioFactory)();

class PlayerDriver :
            public OsclActiveObject,
            public PVCommandStatusObserver,
            public PVInformationalEventObserver,
            public PVErrorEventObserver
{
public:
    PlayerDriver(PVPlayer* pvPlayer);
    ~PlayerDriver();

    PlayerCommand* dequeueCommand();
    status_t enqueueCommand(PlayerCommand* command);

    // Dequeues a command from MediaPlayer and gives it to PVPlayer.
    void Run();

    // Handlers for the various commands we can accept.
    void commandFailed(PlayerCommand* ec);
    void handleSetup(PlayerSetup* ec);
    void handleSetDataSource(PlayerSetDataSource* ec);
    void handleSetVideoSurface(PlayerSetVideoSurface* ec);
    void handleSetAudioSink(PlayerSetAudioSink* ec);
    void handleInit(PlayerInit* ec);
    void handlePrepare(PlayerPrepare* ec);
    void handleStart(PlayerStart* ec);
    void handleStop(PlayerStop* ec);
    void handlePause(PlayerPause* ec);
    void handleSeek(PlayerSeek* ec);
    void handleCancelAllCommands(PlayerCancelAllCommands* ec);
    void handleRemoveDataSource(PlayerRemoveDataSource* ec);
    void handleReset(PlayerReset* ec);
    void handleQuit(PlayerQuit* ec);
    void handleGetPosition(PlayerGetPosition* ec);
    void handleGetDuration(PlayerGetDuration* ec);
    void handleGetStatus(PlayerGetStatus* ec);

    void endOfData();

    void CommandCompleted(const PVCmdResponse& aResponse);
    void HandleErrorEvent(const PVAsyncErrorEvent& aEvent);
    void HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent);

private:
    // Finish up a non-async command in such a way that
    // the event loop will keep running.
    void FinishSyncCommand(PlayerCommand* ec);

    void handleGetDurationComplete(PlayerGetDuration* cmd);

    int setupHttpStreamPre();
    int setupHttpStreamPost();



    // Starts the PV scheduler thread.
    static int startPlayerThread(void *cookie);
    int playerThread();

    // Callback for synchronous commands.
    static void syncCompletion(status_t s, void *cookie, bool cancelled);

    PVPlayer                *mPvPlayer;
    PVPlayerInterface       *mPlayer;
    PVPlayerDataSourceURL   *mDataSource;

    PVPlayerDataSink        *mAudioSink;
    PVMFNodeInterface       *mAudioNode;
    AndroidAudioMIO         *mAudioOutputMIO;

    PVPlayerDataSink        *mVideoSink;
    PVMFNodeInterface       *mVideoNode;
    PvmiMIOControl          *mVideoOutputMIO;

    PvmiCapabilityAndConfig *mPlayerCapConfig;

    OSCL_wHeapString<OsclMemAllocator> mDownloadFilename;
    OSCL_HeapString<OsclMemAllocator> mDownloadProxy;
    OSCL_wHeapString<OsclMemAllocator> mDownloadConfigFilename;
    PVMFSourceContextData   *mDownloadContextData;

    PVPMetadataList mMetaKeyList;
    Oscl_Vector<PvmiKvp,OsclMemAllocator> mMetaValueList;
    int mNumMetaValues;

    // Semaphore used for synchronous commands.
    OsclSemaphore           *mSyncSem;
    // Status cached by syncCompletion for synchronous commands.
    status_t                mSyncStatus;

    // Command queue and its lock.
    List<PlayerCommand*>    mCommandQueue;
    Mutex                   mQueueLock;

    bool                    mIsLooping;
    bool                    mDoLoop;
    bool                    mDataReadyReceived;
    bool                    mPrepareDone;
    bool                    mEndOfData;
    int                     mRecentSeek;
    bool                    mSeekComp;
    bool                    mSeekPending;
    bool                    mEmulation;
    void*                   mLibHandle;
};

PlayerDriver::PlayerDriver(PVPlayer* pvPlayer) :
        OsclActiveObject(OsclActiveObject::EPriorityNominal, "PVPlayerPlayer"),
        mPvPlayer(pvPlayer),
        mIsLooping(false),
        mDoLoop(false),
        mDataReadyReceived(false),
        mPrepareDone(false),
        mEndOfData(false),
        mRecentSeek(0),
        mSeekComp(true),
        mSeekPending(false),
        mEmulation(false)
{
    LOGV("constructor");
    mSyncSem = new OsclSemaphore();
    mSyncSem->Create();

    mDataSource = NULL;
    mAudioSink = NULL;
    mAudioNode = NULL;
    mAudioOutputMIO = NULL;
    mVideoSink = NULL;
    mVideoNode = NULL;
    mVideoOutputMIO = NULL;

    mPlayerCapConfig = NULL;
    mDownloadContextData = NULL;

    // attempt to open device-specific library
    mLibHandle = ::dlopen(MIO_LIBRARY_NAME, RTLD_NOW);

    // start player thread
    LOGV("start player thread");
    createThreadEtc(PlayerDriver::startPlayerThread, this, "PV player");

    // mSyncSem will be signaled when the scheduler has started
    mSyncSem->Wait();
}

PlayerDriver::~PlayerDriver()
{
    LOGV("destructor");
    if (mLibHandle != NULL) {
        ::dlclose(mLibHandle);
    }
}

PlayerCommand* PlayerDriver::dequeueCommand()
{
    PlayerCommand* ec;

    mQueueLock.lock();

    // XXX should we assert here?
    if (mCommandQueue.empty()) {
        PendForExec();
        mQueueLock.unlock();
        return NULL;
    }

    ec = *(--mCommandQueue.end());
    mCommandQueue.erase(--mCommandQueue.end());
    if (mCommandQueue.size() > 0 )
    {
        RunIfNotReady();
    }
    else
    {
        PendForExec();
    }
    mQueueLock.unlock();

    return ec;
}

status_t PlayerDriver::enqueueCommand(PlayerCommand* ec)
{
    if (mPlayer == NULL) {
        delete ec;
        return NO_INIT;
    }

    OsclSemaphore *syncsemcopy = NULL;

    // If the user didn't specify a completion callback, we
    // are running in synchronous mode.
    if (ec->hasCompletionHook()) {
        ec->set(PlayerDriver::syncCompletion, this);
        // make a copy of this semaphore for special handling of the PLAYER_QUIT command
        syncsemcopy = mSyncSem;
    }

    // Add the command to the queue.
    mQueueLock.lock();
    mCommandQueue.push_front(ec);

    // save command, since ec will be deleted by the standard completion function
    int command = ec->command();

    // AO needs to be scheduled only if this is the first cmd after queue was empty
    if (mCommandQueue.size() == 1)
    {
        PendComplete(OSCL_REQUEST_ERR_NONE);
    }
    mQueueLock.unlock();

    // If we are in synchronous mode, wait for completion.
    if (syncsemcopy) {
        syncsemcopy->Wait();
        if (command == PlayerCommand::PLAYER_QUIT) {
            syncsemcopy->Close();
            delete syncsemcopy;
            return 0;
        }
        return mSyncStatus;
    }

    return OK;
}

void PlayerDriver::FinishSyncCommand(PlayerCommand* ec)
{
    ec->complete(0, false);
    delete ec;
}

// The OSCL scheduler calls this when we get to run (this should happen only
// when a command has been enqueued for us).
void PlayerDriver::Run()
{
    if (mDoLoop) {
        mEndOfData = false;
        PVPPlaybackPosition begin, end;
        begin.iIndeterminate = false;
        begin.iPosUnit = PVPPBPOSUNIT_SEC;
        begin.iPosValue.sec_value = 0;
        begin.iMode = PVPPBPOS_MODE_NOW;
        end.iIndeterminate = true;
        mPlayer->SetPlaybackRange(begin, end, false, NULL);
        mPlayer->Resume();
        return;
    }

    PVPlayerState state = PVP_STATE_ERROR;
    if ((mPlayer->GetPVPlayerStateSync(state) == PVMFSuccess))
    {
        if (state == PVP_STATE_ERROR)
        {
            return;
        }
    }


    PlayerCommand* ec;

    ec = dequeueCommand();
    if (ec) {
        LOGV("Send player command: %d", ec->command());

        switch (ec->command()) {
        case PlayerCommand::PLAYER_SETUP:
            handleSetup(static_cast<PlayerSetup*>(ec));
            break;

        case PlayerCommand::PLAYER_SET_DATA_SOURCE:
            handleSetDataSource(static_cast<PlayerSetDataSource*>(ec));
            break;

        case PlayerCommand::PLAYER_SET_VIDEO_SURFACE:
            handleSetVideoSurface(static_cast<PlayerSetVideoSurface*>(ec));
            break;

        case PlayerCommand::PLAYER_SET_AUDIO_SINK:
            handleSetAudioSink(static_cast<PlayerSetAudioSink*>(ec));
            break;

        case PlayerCommand::PLAYER_INIT:
            handleInit(static_cast<PlayerInit*>(ec));
            break;

        case PlayerCommand::PLAYER_PREPARE:
            handlePrepare(static_cast<PlayerPrepare*>(ec));
            break;

        case PlayerCommand::PLAYER_START:
            handleStart(static_cast<PlayerStart*>(ec));
            break;

        case PlayerCommand::PLAYER_STOP:
            handleStop(static_cast<PlayerStop*>(ec));
            break;

        case PlayerCommand::PLAYER_PAUSE:
            handlePause(static_cast<PlayerPause*>(ec));
            break;

        case PlayerCommand::PLAYER_SEEK:
            handleSeek(static_cast<PlayerSeek*>(ec));
            break;

        case PlayerCommand::PLAYER_GET_POSITION:
            handleGetPosition(static_cast<PlayerGetPosition*>(ec));
            FinishSyncCommand(ec);
            return;

        case PlayerCommand::PLAYER_GET_STATUS:
            handleGetStatus(static_cast<PlayerGetStatus*>(ec));
            FinishSyncCommand(ec);
            return;

        case PlayerCommand::PLAYER_GET_DURATION:
            handleGetDuration(static_cast<PlayerGetDuration*>(ec));
            break;

        case PlayerCommand::PLAYER_REMOVE_DATA_SOURCE:
            handleRemoveDataSource(static_cast<PlayerRemoveDataSource*>(ec));
            break;

        case PlayerCommand::PLAYER_CANCEL_ALL_COMMANDS:
            handleCancelAllCommands(static_cast<PlayerCancelAllCommands*>(ec));
            break;

        case PlayerCommand::PLAYER_RESET:
            handleReset(static_cast<PlayerReset*>(ec));
            break;

        case PlayerCommand::PLAYER_QUIT:
            handleQuit(static_cast<PlayerQuit*>(ec));
            return;

        case PlayerCommand::PLAYER_SET_LOOP:
            mIsLooping = static_cast<PlayerSetLoop*>(ec)->loop();
            FinishSyncCommand(ec);
            return;

        default:
            LOGE("Unexpected command %d", ec->command());
            break;
        }
    }

}

void PlayerDriver::commandFailed(PlayerCommand* ec)
{
    if (ec == NULL) {
        LOGV("async command failed");
        return;
    }

    LOGV("Command failed: %d", ec->command());
    // FIXME: Ignore seek failure because it might not work when streaming
    if (mSeekPending) {
        LOGV("Ignoring failed seek");
        ec->complete(NO_ERROR, false);
        mSeekPending = false;
    } else {
        ec->complete(UNKNOWN_ERROR, false);
    }
    delete ec;
}

void PlayerDriver::handleSetup(PlayerSetup* ec)
{
    int error = 0;

    // Make sure we have the capabilities and config interface first.
    OSCL_TRY(error, mPlayer->QueryInterface(PVMI_CAPABILITY_AND_CONFIG_PVUUID,
                                            (PVInterface *&)mPlayerCapConfig, ec));
    OSCL_FIRST_CATCH_ANY(error, commandFailed(ec));
}

int PlayerDriver::setupHttpStreamPre()
{
    mDataSource->SetDataSourceFormatType((char*)PVMF_MIME_DATA_SOURCE_HTTP_URL);

    delete mDownloadContextData;
    mDownloadContextData = NULL;

    mDownloadContextData = new PVMFSourceContextData();
    mDownloadContextData->EnableCommonSourceContext();
    mDownloadContextData->EnableDownloadHTTPSourceContext();

    // FIXME:
    // This mDownloadConfigFilename at /tmp/http-stream-cfg
    // should not exist. We need to clean it up later.
    mDownloadConfigFilename = _STRLIT_WCHAR("/tmp/http-stream-cfg");
    mDownloadFilename = NULL;
    mDownloadProxy = _STRLIT_CHAR("");

    mDownloadContextData->DownloadHTTPData()->iMaxFileSize = 0xFFFFFFFF;
    mDownloadContextData->DownloadHTTPData()->iPlaybackControl = PVMFSourceContextDataDownloadHTTP::ENoSaveToFile;
    mDownloadContextData->DownloadHTTPData()->iConfigFileName = mDownloadConfigFilename;
    mDownloadContextData->DownloadHTTPData()->iDownloadFileName = mDownloadFilename;
    mDownloadContextData->DownloadHTTPData()->iProxyName = mDownloadProxy;
    mDownloadContextData->DownloadHTTPData()->iProxyPort = 0;
    mDownloadContextData->DownloadHTTPData()->bIsNewSession = true;
    mDataSource->SetDataSourceContextData(mDownloadContextData);

    return 0;
}

int PlayerDriver::setupHttpStreamPost()
{
    PvmiKvp iKVPSetAsync;
    OSCL_StackString<64> iKeyStringSetAsync;
    PvmiKvp *iErrorKVP = NULL;

    int error = 0;

    iKVPSetAsync.key = _STRLIT_CHAR("x-pvmf/net/user-agent;valtype=wchar*");
    OSCL_wHeapString<OsclMemAllocator> userAgent = _STRLIT_WCHAR("CORE/6.506.4.1 OpenCORE/2.03 (Linux;Android 1.0)(AndroidMediaPlayer 1.0)");
    iKVPSetAsync.value.pWChar_value=userAgent.get_str();
    iErrorKVP=NULL;
    OSCL_TRY(error, mPlayerCapConfig->setParametersSync(NULL, &iKVPSetAsync, 1, iErrorKVP));
    OSCL_FIRST_CATCH_ANY(error, return -1);

    iKeyStringSetAsync=_STRLIT_CHAR("x-pvmf/net/http-timeout;valtype=uint32");
    iKVPSetAsync.key=iKeyStringSetAsync.get_str();
    iKVPSetAsync.value.uint32_value=20;
    iErrorKVP=NULL;
    OSCL_TRY(error, mPlayerCapConfig->setParametersSync(NULL, &iKVPSetAsync, 1, iErrorKVP));
    OSCL_FIRST_CATCH_ANY(error, return -1);

    iKeyStringSetAsync=_STRLIT_CHAR("x-pvmf/net/num-redirect-attempts;valtype=uint32");
    iKVPSetAsync.key=iKeyStringSetAsync.get_str();
    iKVPSetAsync.value.uint32_value=4;
    iErrorKVP=NULL;
    OSCL_TRY(error, mPlayerCapConfig->setParametersSync(NULL, &iKVPSetAsync, 1, iErrorKVP));
    OSCL_FIRST_CATCH_ANY(error, return -1);

    // enable or disable HEAD request
    iKeyStringSetAsync=_STRLIT_CHAR("x-pvmf/net/http-header-request-disabled;valtype=bool");
    iKVPSetAsync.key=iKeyStringSetAsync.get_str();
    iKVPSetAsync.value.bool_value=false;
    iErrorKVP=NULL;
    OSCL_TRY(error, mPlayerCapConfig->setParametersSync(NULL, &iKVPSetAsync, 1, iErrorKVP));
    OSCL_FIRST_CATCH_ANY(error, return -1);

    iKeyStringSetAsync=_STRLIT_CHAR("x-pvmf/net/max-tcp-recv-buffer-size-download;valtype=uint32");
    iKVPSetAsync.key=iKeyStringSetAsync.get_str();
    iKVPSetAsync.value.uint32_value=64000;
    iErrorKVP=NULL;
    OSCL_TRY(error, mPlayerCapConfig->setParametersSync(NULL, &iKVPSetAsync, 1, iErrorKVP));
    OSCL_FIRST_CATCH_ANY(error, return -1);

    return 0;
}

void PlayerDriver::handleSetDataSource(PlayerSetDataSource* ec)
{
    LOGV("handleSetDataSource");
    int error = 0;
    const char* url = ec->url();
    int lengthofurl = strlen(url);
    oscl_wchar output[lengthofurl + 1];
    OSCL_wHeapString<OsclMemAllocator> wFileName;

    if (mDataSource) {
        delete mDataSource;
        mDataSource = NULL;
    }

    // Create a URL datasource to feed PVPlayer
    mDataSource = new PVPlayerDataSourceURL();
    oscl_UTF8ToUnicode(url, strlen(url), output, lengthofurl+1);
    wFileName.set(output, oscl_strlen(output));
    mDataSource->SetDataSourceURL(wFileName);
    LOGV("handleSetDataSource- scanning for extension");
    if (strncmp(url, "rtsp:", strlen("rtsp:")) == 0) {
        mDataSource->SetDataSourceFormatType((const char*)PVMF_MIME_DATA_SOURCE_RTSP_URL);
    } else if (strncmp(url, "http:", strlen("http:")) == 0) {
        if (0!=setupHttpStreamPre())
        {
            commandFailed(ec);
            return;
        }
    } else {
        mDataSource->SetDataSourceFormatType((const char*)PVMF_MIME_FORMAT_UNKNOWN); // Let PV figure it out
    }

    OSCL_TRY(error, mPlayer->AddDataSource(*mDataSource, ec));
    OSCL_FIRST_CATCH_ANY(error, commandFailed(ec));
}

void PlayerDriver::handleInit(PlayerInit* ec)
{
    int error = 0;

    if (mDownloadContextData) {
        setupHttpStreamPost();
    }

    OSCL_TRY(error, mPlayer->Init(ec));
    OSCL_FIRST_CATCH_ANY(error, commandFailed(ec));
}

void PlayerDriver::handleSetVideoSurface(PlayerSetVideoSurface* ec)
{
    int error = 0;
    AndroidSurfaceOutput* mio = NULL;

    // attempt to load device-specific video MIO
    if (mLibHandle != NULL) {
        VideoMioFactory f = (VideoMioFactory) ::dlsym(mLibHandle, VIDEO_MIO_FACTORY_NAME);
        if (f != NULL) {
            mio = f();
        }
    }

    // if no device-specific MIO was created, use the generic one
    if (mio == NULL) {
        LOGW("Using generic video MIO");
        mio = new AndroidSurfaceOutput();
    }

    // initialize the MIO parameters
    status_t ret = mio->set(mPvPlayer, ec->surface(), mEmulation);
    if (ret != NO_ERROR) {
        LOGE("Video MIO set failed");
        commandFailed(ec);
        delete mio;
        return;
    }
    mVideoOutputMIO = mio;

    mVideoNode = PVMediaOutputNodeFactory::CreateMediaOutputNode(mVideoOutputMIO);
    mVideoSink = new PVPlayerDataSinkPVMFNode;

    ((PVPlayerDataSinkPVMFNode *)mVideoSink)->SetDataSinkNode(mVideoNode);
    ((PVPlayerDataSinkPVMFNode *)mVideoSink)->SetDataSinkFormatType((char*)PVMF_MIME_YUV420);

    OSCL_TRY(error, mPlayer->AddDataSink(*mVideoSink, ec));
    OSCL_FIRST_CATCH_ANY(error, commandFailed(ec));
}

void PlayerDriver::handleSetAudioSink(PlayerSetAudioSink* ec)
{
    int error = 0;
    if (ec->audioSink()->realtime()) {
        LOGV("Create realtime output");
        mAudioOutputMIO = new AndroidAudioOutput();
    } else {
        LOGV("Create stream output");
        mAudioOutputMIO = new AndroidAudioStream();
    }
    mAudioOutputMIO->setAudioSink(ec->audioSink());

    mAudioNode = PVMediaOutputNodeFactory::CreateMediaOutputNode(mAudioOutputMIO);
    mAudioSink = new PVPlayerDataSinkPVMFNode;

    ((PVPlayerDataSinkPVMFNode *)mAudioSink)->SetDataSinkNode(mAudioNode);
    ((PVPlayerDataSinkPVMFNode *)mAudioSink)->SetDataSinkFormatType((char*)PVMF_MIME_PCM16);

    OSCL_TRY(error, mPlayer->AddDataSink(*mAudioSink, ec));
    OSCL_FIRST_CATCH_ANY(error, commandFailed(ec));
}

void PlayerDriver::handlePrepare(PlayerPrepare* ec)
{
    //Keep alive is sent during the play to prevent the firewall from closing ports while
    //streming long clip
    PvmiKvp iKVPSetAsync;
    OSCL_StackString<64> iKeyStringSetAsync;
    PvmiKvp *iErrorKVP = NULL;
    int error=0;
    iKeyStringSetAsync=_STRLIT_CHAR("x-pvmf/net/keep-alive-during-play;valtype=bool");
    iKVPSetAsync.key=iKeyStringSetAsync.get_str();
    iKVPSetAsync.value.bool_value=true;
    iErrorKVP=NULL;
    OSCL_TRY(error, mPlayerCapConfig->setParametersSync(NULL, &iKVPSetAsync, 1, iErrorKVP));
    OSCL_TRY(error, mPlayer->Prepare(ec));
    OSCL_FIRST_CATCH_ANY(error, commandFailed(ec));
}

void PlayerDriver::handleStart(PlayerStart* ec)
{
    int error = 0;

    // for video, set thread priority so we don't hog CPU
    if (mVideoOutputMIO) {
        int ret = setpriority(PRIO_PROCESS, 0, ANDROID_PRIORITY_DISPLAY);
    }
    // for audio, set thread priority so audio isn't choppy
    else {
        int ret = setpriority(PRIO_PROCESS, 0, ANDROID_PRIORITY_AUDIO);
    }

    // Signalling seek complete to continue obtaining the current position
    // from PVPlayer Engine
    mSeekComp = true;
    // if we are paused, just resume
    PVPlayerState state;
    if (mPlayer->GetPVPlayerStateSync(state) == PVMFSuccess
            && (state == PVP_STATE_PAUSED)) {
        if (mEndOfData) {
            // if we are at the end, seek to the beginning first
            mEndOfData = false;
            PVPPlaybackPosition begin, end;
            begin.iIndeterminate = false;
            begin.iPosUnit = PVPPBPOSUNIT_SEC;
            begin.iPosValue.sec_value = 0;
            begin.iMode = PVPPBPOS_MODE_NOW;
            end.iIndeterminate = true;
            mPlayer->SetPlaybackRange(begin, end, false, NULL);
        }
        OSCL_TRY(error, mPlayer->Resume(ec));
        OSCL_FIRST_CATCH_ANY(error, commandFailed(ec));
    } else {
        OSCL_TRY(error, mPlayer->Start(ec));
        OSCL_FIRST_CATCH_ANY(error, commandFailed(ec));
    }
}

void PlayerDriver::handleSeek(PlayerSeek* ec)
{
    int error = 0;

    LOGV("handleSeek");
    // Cache the most recent seek request
    mRecentSeek = ec->msec();
    // Seeking in the pause state
    PVPlayerState state;
    if (mPlayer->GetPVPlayerStateSync(state) == PVMFSuccess
            && (state == PVP_STATE_PAUSED)) {
        mSeekComp = false;
    }
    PVPPlaybackPosition begin, end;
    begin.iIndeterminate = false;
    begin.iPosUnit = PVPPBPOSUNIT_MILLISEC;
    begin.iPosValue.millisec_value = ec->msec();
    begin.iMode = PVPPBPOS_MODE_NOW;
    end.iIndeterminate = true;
    mSeekPending = true;
    OSCL_TRY(error, mPlayer->SetPlaybackRange(begin, end, false, ec));
    OSCL_FIRST_CATCH_ANY(error, commandFailed(ec));

    mEndOfData = false;
}

void PlayerDriver::handleGetPosition(PlayerGetPosition* ec)
{
    PVPPlaybackPosition pos;
    pos.iPosUnit = PVPPBPOSUNIT_MILLISEC;
    PVPlayerState state;
    //  In the pause state, get the progress bar position from the recent seek value
    // instead of GetCurrentPosition() from PVPlayer Engine.
    if (mPlayer->GetPVPlayerStateSync(state) == PVMFSuccess
            && (state == PVP_STATE_PAUSED)
            && (mSeekComp == false)) {
        ec->set(mRecentSeek);
    }
    else {
        if (mPlayer->GetCurrentPositionSync(pos) != PVMFSuccess) {
            ec->set(-1);
        } else {
            LOGV("position=%d", pos.iPosValue.millisec_value);
            ec->set((int)pos.iPosValue.millisec_value);
        }
    }
}

void PlayerDriver::handleGetStatus(PlayerGetStatus* ec)
{
    PVPlayerState stat;
    if (mPlayer->GetPVPlayerStateSync(stat) != PVMFSuccess) {
        ec->set(0);
    } else {
        ec->set(stat);
        LOGV("status=%d", stat);
    }
}

void PlayerDriver::handleGetDuration(PlayerGetDuration* ec)
{
    ec->set(-1);
    mMetaKeyList.clear();
    mMetaKeyList.push_back(OSCL_HeapString<OsclMemAllocator>("duration"));
    mMetaValueList.clear();
    mNumMetaValues=0;
    int error = 0;
    OSCL_TRY(error, mPlayer->GetMetadataValues(mMetaKeyList,0,-1,mNumMetaValues,mMetaValueList, ec));
    OSCL_FIRST_CATCH_ANY(error, commandFailed(ec));
}

void PlayerDriver::handleStop(PlayerStop* ec)
{
    int error = 0;
    // setting the looping boolean to false. MediaPlayer app takes care of setting the loop again before the start.
    mIsLooping = false;
    mDoLoop = false;
    PVPlayerState state;
    if ((mPlayer->GetPVPlayerStateSync(state) == PVMFSuccess)
            && ( (state == PVP_STATE_PAUSED) ||
                 (state == PVP_STATE_PREPARED) ||
                 (state == PVP_STATE_STARTED)
               )
       )
    {
        OSCL_TRY(error, mPlayer->Stop(ec));
        OSCL_FIRST_CATCH_ANY(error, commandFailed(ec));
    }
    else
    {
        LOGV("handleStop - Player Status = %d - Sending Reset instead of Stop\n",state);
        // TODO: Previously this called CancelAllCommands and RemoveDataSource
        handleReset(new PlayerReset(ec->callback(), ec->cookie()));
        delete ec;
    }
}

void PlayerDriver::handlePause(PlayerPause* ec)
{
    LOGV("call pause");
    int error = 0;
    OSCL_TRY(error, mPlayer->Pause(ec));
    OSCL_FIRST_CATCH_ANY(error, commandFailed(ec));
}

void PlayerDriver::handleRemoveDataSource(PlayerRemoveDataSource* ec)
{
    LOGV("handleRemoveDataSource");
    int error = 0;
    OSCL_TRY(error, mPlayer->RemoveDataSource(*mDataSource, ec));
    OSCL_FIRST_CATCH_ANY(error, commandFailed(ec));
}

void PlayerDriver::handleCancelAllCommands(PlayerCancelAllCommands* ec)
{
    LOGV("handleCancelAllCommands");
    int error = 0;
    OSCL_TRY(error, mPlayer->CancelAllCommands(ec));
    OSCL_FIRST_CATCH_ANY(error, commandFailed(ec));
}

void PlayerDriver::handleReset(PlayerReset* ec)
{
    LOGV("handleReset");
    int error = 0;

    // setting the looping boolean to false. MediaPlayer app takes care of setting the loop again before the start.
    mIsLooping = false;
    mDoLoop = false;
    mEndOfData = false;

    OSCL_TRY(error, mPlayer->Reset(ec));
    OSCL_FIRST_CATCH_ANY(error, commandFailed(ec));
}

void PlayerDriver::handleQuit(PlayerQuit* ec)
{
    OsclExecScheduler *sched = OsclExecScheduler::Current();
    sched->StopScheduler();
}

/*static*/ int PlayerDriver::startPlayerThread(void *cookie)
{
    LOGV("startPlayerThread");
    PlayerDriver *ed = (PlayerDriver *)cookie;
    return ed->playerThread();
}

int PlayerDriver::playerThread()
{
    int error;

    LOGV("InitializeForThread");
    if (!InitializeForThread())
    {
        LOGV("InitializeForThread fail");
        mPlayer = NULL;
        mSyncSem->Signal();
        return -1;
    }

    LOGV("OMX_Init");
    OMX_Init();

    LOGV("OsclScheduler::Init");
    OsclScheduler::Init("AndroidPVWrapper");

    LOGV("CreatePlayer");
    OSCL_TRY(error, mPlayer = PVPlayerFactory::CreatePlayer(this, this, this));
    if (error) {
        // Just crash the first time someone tries to use it for now?
        mPlayer = NULL;
        mSyncSem->Signal();
        return -1;
    }

    LOGV("AddToScheduler");
    AddToScheduler();
    LOGV("PendForExec");
    PendForExec();

    LOGV("OsclActiveScheduler::Current");
    OsclExecScheduler *sched = OsclExecScheduler::Current();
    LOGV("StartScheduler");
    sched->StartScheduler(mSyncSem);

    LOGV("DeletePlayer");
    PVPlayerFactory::DeletePlayer(mPlayer);

    delete mDownloadContextData;
    mDownloadContextData = NULL;

    delete mDataSource;
    mDataSource = NULL;
    delete mAudioSink;
    PVMediaOutputNodeFactory::DeleteMediaOutputNode(mAudioNode);
    delete mAudioOutputMIO;
    delete mVideoSink;
    if (mVideoNode) {
        PVMediaOutputNodeFactory::DeleteMediaOutputNode(mVideoNode);
        delete mVideoOutputMIO;
    }

    mSyncStatus = OK;
    mSyncSem->Signal();
    // note that we only signal mSyncSem. Deleting it is handled
    // in enqueueCommand(). This is done because waiting for an
    // already-deleted OsclSemaphore doesn't work (it blocks),
    // and it's entirely possible for this thread to exit before
    // enqueueCommand() gets around to waiting for the semaphore.

    // do some of destructor's work here
    // goodbye cruel world
    delete this;

    //Moved after the delete this, as Oscl cleanup should be done in the end.
    //delete this was cleaning up OsclSemaphore objects, eventually causing a crash
    OsclScheduler::Cleanup();
    LOGV("OsclScheduler::Cleanup");

    OMX_Deinit();
    UninitializeForThread();
    return 0;
}

/*static*/ void PlayerDriver::syncCompletion(status_t s, void *cookie, bool cancelled)
{
    PlayerDriver *ed = static_cast<PlayerDriver*>(cookie);
    ed->mSyncStatus = s;
    ed->mSyncSem->Signal();
}

void PlayerDriver::handleGetDurationComplete(PlayerGetDuration* cmd)
{
    cmd->set(-1);

    if (mMetaValueList.empty())
        return;

    MediaClockConverter mcc;

    for (uint32 i = 0; i < mMetaValueList.size(); ++i) {
        // Search for the duration
        const char* substr=oscl_strstr(mMetaValueList[i].key, _STRLIT_CHAR("duration;valtype=uint32;timescale="));
        if (substr!=NULL) {
            uint32 timescale=1000;
            if (PV_atoi((substr+34), 'd', timescale) == false) {
                // Retrieving timescale failed so default to 1000
                timescale=1000;
            }
            uint32 duration = mMetaValueList[i].value.uint32_value;
            if (duration > 0 && timescale > 0) {
                //set the timescale
                mcc.set_timescale(timescale);
                //set the clock to the duration as per the timescale
                mcc.set_clock(duration,0);
                //convert to millisec
                cmd->set(mcc.get_converted_ts(1000));
            }
        }
    }
}

void PlayerDriver::CommandCompleted(const PVCmdResponse& aResponse)
{
    LOGV("CommandCompleted");
    int status = aResponse.GetCmdStatus();

    if (mDoLoop) {
        mDoLoop = false;
        RunIfNotReady();
        return;
    }

    PlayerCommand* ec = static_cast<PlayerCommand*>(aResponse.GetContext());
    LOGV("Command %d status=%d", ec ? ec->command(): 0, status);
    if (ec == NULL) return;

    // FIXME: Ignore non-fatal seek errors because pvPlayerEngine returns these errors and retains it's state.
    if (mSeekPending) {
        mSeekPending = false;
        if ( ( (status == PVMFErrArgument) || (status == PVMFErrInvalidState) || (status == PVMFErrNotSupported) ) ) {
            LOGV("Ignoring error during seek");
            status = PVMFSuccess;
        }
    }

    if (status == PVMFSuccess) {
        switch (ec->command()) {
        case PlayerCommand::PLAYER_PREPARE:
            LOGV("PLAYER_PREPARE complete mDownloadContextData=%p, mDataReadyReceived=%d", mDownloadContextData, mDataReadyReceived);
            mPrepareDone = true;
            // If we are streaming from the network, we
            // have to wait until the first PVMFInfoDataReady
            // is sent to notify the user that it is okay to
            // begin playback.  If it is a local file, just
            // send it now at the completion of Prepare().
            if ((mDownloadContextData == NULL) || mDataReadyReceived) {
                mPvPlayer->sendEvent(MEDIA_PREPARED);
            }
            break;

        case PlayerCommand::PLAYER_GET_DURATION:
            handleGetDurationComplete(static_cast<PlayerGetDuration*>(ec));
            break;

        case PlayerCommand::PLAYER_PAUSE:
            LOGV("pause complete");
            break;

        case PlayerCommand::PLAYER_SEEK:
            mPvPlayer->sendEvent(MEDIA_SEEK_COMPLETE);
            break;

        default: /* shut up gcc */
            break;
        }

        // Call the user's requested completion function
        ec->complete(NO_ERROR, false);
    } else if (status == PVMFErrCancelled) {
        // Ignore cancelled command return status (PVMFErrCancelled), since it is not an error.
        LOGE("Command (%d) was cancelled", ec->command());
        status = PVMFSuccess;
        ec->complete(NO_ERROR, true);
    } else {
        // error occurred
        if (status >= 0) status = -1;
        mPvPlayer->sendEvent(MEDIA_ERROR, status);
        ec->complete(UNKNOWN_ERROR, false);
    }

    delete ec;
}

void PlayerDriver::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    int32_t eventType = aEvent.GetEventType();
    LOGE("HandleErrorEvent: type=%d\n", eventType);
    mPvPlayer->sendEvent(MEDIA_ERROR, eventType);
}

void PlayerDriver::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    switch (aEvent.GetEventType()) {
    case PVMFInfoEndOfData:
        LOGV("PVMFInfoEndOfData");
        mEndOfData = true;
        if (mIsLooping) {
            mDoLoop = true;
            Cancel();
            RunIfNotReady();
        } else {
            mPvPlayer->sendEvent(MEDIA_PLAYBACK_COMPLETE);
        }
        break;

    case PVMFInfoErrorHandlingComplete:
        LOGW("PVMFInfoErrorHandlingComplete");
        RunIfNotReady();
        break;

    case PVMFInfoBufferingStart:
        LOGV("PVMFInfoBufferingStart");
        mPvPlayer->sendEvent(MEDIA_BUFFERING_UPDATE, 0);
        break;

    case PVMFInfoBufferingStatus:
    {
        uint8 *localBuf = aEvent.GetLocalBuffer();
        if (localBuf != NULL) {
            uint32 bufPercent;
            // @TODO - Get rid of this later
            if (mDataSource->GetDataSourceFormatType() == (char*)PVMF_MIME_DATA_SOURCE_HTTP_URL) {
                oscl_memcpy(&bufPercent, localBuf, sizeof(uint32));
            }
            else {//RTSP
                oscl_memcpy(&bufPercent, &localBuf[4], sizeof(uint32));
            }
            LOGV("PVMFInfoBufferingStatus(%u)", bufPercent);
            mPvPlayer->sendEvent(MEDIA_BUFFERING_UPDATE, bufPercent);
        }
    }
    break;

    case PVMFInfoDurationAvailable:
        LOGV("PVMFInfoDurationAvailable event ....");
        {
            PVUuid infomsguuid = PVMFDurationInfoMessageInterfaceUUID;
            PVMFDurationInfoMessageInterface* eventMsg = NULL;
            PVInterface* infoExtInterface = aEvent.GetEventExtensionInterface();
            if (infoExtInterface &&
                    infoExtInterface->queryInterface(infomsguuid, (PVInterface*&)eventMsg))
            {
                PVUuid eventuuid;
                int32 infoCode;
                eventMsg->GetCodeUUID(infoCode, eventuuid);
                if (eventuuid == infomsguuid)
                {
                    uint32 SourceDurationInMS = eventMsg->GetDuration();
                    LOGV(".... with duration = %u ms",SourceDurationInMS);
                }
            }
        }
        break;

    case PVMFInfoDataReady:
        LOGV("PVMFInfoDataReady");
        if (mDataReadyReceived)
            break;
        mDataReadyReceived = true;
        // If this is a network stream, we are now ready to play.
        if (mDownloadContextData && mPrepareDone) {
            mPvPlayer->sendEvent(MEDIA_PREPARED);
        }
        break;

    case PVMFInfoVideoTrackFallingBehind:
        LOGW("Video track fell behind");
        mPvPlayer->sendEvent(MEDIA_ERROR, PVMFInfoVideoTrackFallingBehind);
        break;

    case PVMFInfoPoorlyInterleavedContent:
        LOGW("Poorly interleaved content.");
        mPvPlayer->sendEvent(MEDIA_ERROR, PVMFInfoPoorlyInterleavedContent);
        break;

    case PVMFInfoContentTruncated: //LOGI("PVMFInfoContentTruncated\n"); break;
        LOGE("Content is truncated.");
        break;

        /* Certain events we don't really care about, but don't
         * want log spewage, so just no-op them here.
         */
    case PVMFInfoPositionStatus: //LOGI("PVMFInfoPositionStatus\n"); break;
    case PVMFInfoBufferingComplete: //LOGI("PVMFInfoBufferingComplete\n"); break;
    case PVMFInfoContentLength: //LOGI("PVMFInfoContentLength: %d\n", (int)get_event_data(aEvent)); break;
    case PVMFInfoContentType: //LOGI("PVMFInfoContentType: %s\n", (char *)get_event_data(aEvent)); break;
    case PVMFInfoUnderflow: //LOGI("PVMFInfoUnderflow\n"); break;
    case PVMFInfoDataDiscarded: //LOGI("PVMFInfoDataDiscarded\n"); break;
        break;

    default:
        LOGV("HandleInformationalEvent: type=%d UNHANDLED", aEvent.GetEventType());
        break;
    }
}


namespace android {

#undef LOG_TAG
#define LOG_TAG "PVPlayer"

// ----------------------------------------------------------------------------
// implement the Packet Video player
// ----------------------------------------------------------------------------
PVPlayer::PVPlayer()
{
    LOGV("PVPlayer constructor");
    mDataSourcePath = NULL;
    LOGV("construct PlayerDriver");
    mPlayerDriver = new PlayerDriver(this);
    LOGV("send PLAYER_SETUP");
    mInit = mPlayerDriver->enqueueCommand(new PlayerSetup(0,0));
    mMemBase = 0;
    mIsDataSourceSet = false;
    mDuration = -1;
}

status_t PVPlayer::initCheck()
{
    return mInit;
}

PVPlayer::~PVPlayer()
{
    LOGV("PVPlayer destructor");
    PlayerQuit quit = PlayerQuit(0,0);
    mPlayerDriver->enqueueCommand(&quit); // will wait on mSyncSem, signaled by player thread
    free(mDataSourcePath);
    if (mMemBase) {
        munmap(mMemBase, mMemSize);
    }
}

status_t PVPlayer::setDataSource(const char *url)
{
    LOGV("setDataSource(%s)", url);
    if (mMemBase) {
        munmap(mMemBase, mMemSize);
        mMemBase = NULL;
    }
    free(mDataSourcePath);
    mDataSourcePath = NULL;

    // Don't let somebody trick us in to reading some random block of memory
    if (strncmp("mem://", url, 6) == 0)
        return android::UNKNOWN_ERROR;
    mDataSourcePath = strdup(url);
    return OK;
}

status_t PVPlayer::setDataSource(int fd, int64_t offset, int64_t length) {

    // TODO: allow PV to play from a file descriptor.
    // Eventually we'll fix PV to use a file descriptor directly instead
    // of using mmap().
    LOGV("setDataSource(%d, %lld, %lld)", fd, offset, length);
    if (mMemBase) {
        munmap(mMemBase, mMemSize);
        mMemBase = NULL;
    }
    free(mDataSourcePath);
    mDataSourcePath = NULL;

    // round offset down to page size
    long pagesize = PAGESIZE;
    long mapoffset = offset & ~(pagesize - 1);
    long mapoffsetdelta = offset - mapoffset;

    void * bar = mmap(0, length + mapoffsetdelta, PROT_READ, MAP_PRIVATE, fd, mapoffset);
    if ((int)bar == -1) {
        LOGV("error mapping file: %s\n", strerror(errno));
        return android::UNKNOWN_ERROR;
    }

    mMemBase = bar;
    mMemSize = length + mapoffsetdelta;
    char buf[80];
    sprintf(buf, "mem://%p:%lld:%lld", mMemBase,
            (int64_t) mapoffsetdelta,
            (int64_t) mMemSize - mapoffsetdelta);
    mDataSourcePath = strdup(buf);
    madvise(mMemBase, mMemSize, MADV_RANDOM);
    return OK;
}

status_t PVPlayer::setVideoSurface(const sp<ISurface>& surface)
{
    LOGV("setVideoSurface(%p)", surface.get());
    mSurface = surface;
    return OK;
}

status_t PVPlayer::prepare()
{
    status_t ret;

    // We need to differentiate the two valid use cases for prepare():
    // 1. new PVPlayer/reset()->setDataSource()->prepare()
    // 2. new PVPlayer/reset()->setDataSource()->prepare()/prepareAsync()
    //    ->start()->...->stop()->prepare()
    // If data source has already been set previously, no need to run
    // a sequence of commands and only the PLAYER_PREPARE command needs
    // to be run.
    if (!mIsDataSourceSet) {
        // set data source
        LOGV("prepare");
        LOGV("  data source = %s", mDataSourcePath);
        ret = mPlayerDriver->enqueueCommand(new PlayerSetDataSource(mDataSourcePath,0,0));
        if (ret != OK)
            return ret;

        // init
        LOGV("  init");
        ret = mPlayerDriver->enqueueCommand(new PlayerInit(0,0));
        if (ret != OK)
            return ret;

        // set video surface, if there is one
        if (mSurface != NULL) {
            LOGV("  set video surface");
            ret = mPlayerDriver->enqueueCommand(new PlayerSetVideoSurface(mSurface,0,0));
            if (ret != OK)
                return ret;
        }

        // set audio output
        // If we ever need to expose selectable audio output setup, this can be broken
        // out.  In the meantime, however, system audio routing APIs should suffice.
        LOGV("  set audio sink");
        ret = mPlayerDriver->enqueueCommand(new PlayerSetAudioSink(mAudioSink,0,0));
        if (ret != OK)
            return ret;

        // New data source has been set successfully.
        mIsDataSourceSet = true;
    }

    // prepare
    LOGV("  prepare");
    return mPlayerDriver->enqueueCommand(new PlayerPrepare(0,0));
}

void PVPlayer::run_init(status_t s, void *cookie, bool cancelled)
{
    LOGV("run_init s=%d, cancelled=%d", s, cancelled);
    if (s == NO_ERROR && !cancelled) {
        PVPlayer *p = (PVPlayer*)cookie;
        p->mPlayerDriver->enqueueCommand(new PlayerInit(run_set_video_surface, cookie));
    }
}

void PVPlayer::run_set_video_surface(status_t s, void *cookie, bool cancelled)
{
    LOGV("run_set_video_surface s=%d, cancelled=%d", s, cancelled);
    if (s == NO_ERROR && !cancelled) {
        // If we don't have a video surface, just skip to the next step.
        PVPlayer *p = (PVPlayer*)cookie;
        if (p->mSurface == NULL) {
            run_set_audio_output(s, cookie, false);
        } else {
            p->mPlayerDriver->enqueueCommand(new PlayerSetVideoSurface(p->mSurface, run_set_audio_output, cookie));
        }
    }
}

void PVPlayer::run_set_audio_output(status_t s, void *cookie, bool cancelled)
{
    LOGV("run_set_audio_output s=%d, cancelled=%d", s, cancelled);
    if (s == NO_ERROR && !cancelled) {
        PVPlayer *p = (PVPlayer*)cookie;
        p->mPlayerDriver->enqueueCommand(new PlayerSetAudioSink(p->mAudioSink, run_prepare, cookie));
    }
}

void PVPlayer::run_prepare(status_t s, void *cookie, bool cancelled)
{
    LOGV("run_prepare s=%d, cancelled=%d", s, cancelled);
    if (s == NO_ERROR && !cancelled) {
        PVPlayer *p = (PVPlayer*)cookie;
        p->mPlayerDriver->enqueueCommand(new PlayerPrepare(do_nothing,0));
    }
}

status_t PVPlayer::prepareAsync()
{
    LOGV("prepareAsync");
    status_t ret = OK;

    if (!mIsDataSourceSet) {  // If data source has NOT been set.
        // Set our data source as cached in setDataSource() above.
        LOGV("  data source = %s", mDataSourcePath);
        ret = mPlayerDriver->enqueueCommand(new PlayerSetDataSource(mDataSourcePath,run_init,this));
        mIsDataSourceSet = true;
    } else {  // If data source has been already set.
        // No need to run a sequence of commands.
        // The only command needed to run is PLAYER_PREPARE.
        ret = mPlayerDriver->enqueueCommand(new PlayerPrepare(do_nothing, NULL));
    }

    return ret;
}

status_t PVPlayer::start()
{
    LOGV("start");
    return mPlayerDriver->enqueueCommand(new PlayerStart(0,0));
}

status_t PVPlayer::stop()
{
    LOGV("stop");
    return mPlayerDriver->enqueueCommand(new PlayerStop(0,0));
}

status_t PVPlayer::pause()
{
    LOGV("pause");
    return mPlayerDriver->enqueueCommand(new PlayerPause(0,0));
}

bool PVPlayer::isPlaying()
{
    int status = 0;
    if (mPlayerDriver->enqueueCommand(new PlayerGetStatus(&status,0,0)) == NO_ERROR) {
        return (status == PVP_STATE_STARTED);
    }
    return false;
}

status_t PVPlayer::getCurrentPosition(int *msec)
{
    return mPlayerDriver->enqueueCommand(new PlayerGetPosition(msec,0,0));
}

status_t PVPlayer::getDuration(int *msec)
{
    status_t ret = mPlayerDriver->enqueueCommand(new PlayerGetDuration(msec,0,0));
    if (ret == NO_ERROR) mDuration = *msec;
    return ret;
}

status_t PVPlayer::seekTo(int msec)
{
    LOGV("seekTo(%d)", msec);
    // can't always seek to end of streams - so we fudge a little
    if ((msec == mDuration) && (mDuration > 0)) {
        msec--;
        LOGV("Seek adjusted 1 msec from end");
    }
    return mPlayerDriver->enqueueCommand(new PlayerSeek(msec,do_nothing,0));
}

status_t PVPlayer::reset()
{
    LOGV("reset");
    status_t ret = mPlayerDriver->enqueueCommand(new PlayerCancelAllCommands(0,0));
    if (ret == NO_ERROR) {
        ret = mPlayerDriver->enqueueCommand(new PlayerReset(0,0));
    }
    if (ret == NO_ERROR) {
        ret = mPlayerDriver->enqueueCommand(new PlayerRemoveDataSource(0,0));
    }
    mSurface.clear();
    LOGV("unmap file");
    if (mMemBase) {
        munmap(mMemBase, mMemSize);
        mMemBase = NULL;
    }
    mIsDataSourceSet = false;
    return ret;
}

status_t PVPlayer::setLooping(int loop)
{
    LOGV("setLooping(%d)", loop);
    return mPlayerDriver->enqueueCommand(new PlayerSetLoop(loop,0,0));
}

}; // namespace android




