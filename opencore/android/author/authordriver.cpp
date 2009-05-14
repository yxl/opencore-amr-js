/*
 * Copyright (C) 2008, The Android Open Source Project
 * Copyright (C) 2008 HTC Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); 
 * you may not use this file except in compliance with the License. 
 * You may obtain a copy of the License at 
 *
 *     http://www.apache.org/licenses/LICENSE-2.0 
 *
 * Unless required by applicable law or agreed to in writing, software 
 * distributed under the License is distributed on an "AS IS" BASIS, 
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
 * See the License for the specific language governing permissions and 
 * limitations under the License.
 */

//#define LOG_NDEBUG 0
#undef LOG_TAG
#define LOG_TAG "AuthorDriverWrapper"

#include <unistd.h>
#include <media/thread_init.h>
#include <ui/ISurface.h>
#include <ui/ICamera.h>
#include "authordriver.h"
#include "pv_omxcore.h"
#include <sys/prctl.h>
#include "pvmf_composer_size_and_duration.h"

using namespace android;

AuthorDriverWrapper::AuthorDriverWrapper()
{
    mAuthorDriver = new AuthorDriver();
}

void AuthorDriverWrapper::resetAndClose()
{
    LOGV("resetAndClose");
    mAuthorDriver->enqueueCommand(new author_command(AUTHOR_RESET), NULL, NULL);
    mAuthorDriver->enqueueCommand(new author_command(AUTHOR_REMOVE_VIDEO_SOURCE), NULL, NULL);
    mAuthorDriver->enqueueCommand(new author_command(AUTHOR_REMOVE_AUDIO_SOURCE), NULL, NULL);
    mAuthorDriver->enqueueCommand(new author_command(AUTHOR_CLOSE), NULL, NULL);
}

AuthorDriverWrapper::~AuthorDriverWrapper()
{
    if (mAuthorDriver) {
        // set the authoring engine to the IDLE state.
        PVAEState state = mAuthorDriver->getAuthorEngineState();
        switch (state) {
        case PVAE_STATE_IDLE:
            break;

        case PVAE_STATE_RECORDING:
            mAuthorDriver->enqueueCommand(new author_command(AUTHOR_STOP),  NULL, NULL);
            resetAndClose();
            break;

        default:
            resetAndClose();
            break;
        }

        // now it is safe to quit.
        author_command *ac = new author_command(AUTHOR_QUIT);
        enqueueCommand(ac, NULL, NULL); // will wait on mSyncSem, signaled by author thread
        delete ac; // have to delete this manually because CommandCompleted won't be called
    }
}

status_t AuthorDriverWrapper::getMaxAmplitude(int *max)
{
    if (mAuthorDriver) {
        return mAuthorDriver->getMaxAmplitude(max);
    }
    return NO_INIT;
}

status_t AuthorDriverWrapper::enqueueCommand(author_command *ac, media_completion_f comp, void *cookie)
{
    if (mAuthorDriver) {
        return mAuthorDriver->enqueueCommand(ac, comp, cookie);
    }
    return NO_INIT;
}

status_t AuthorDriverWrapper::setListener(const sp<IMediaPlayerClient>& listener) {
    if (mAuthorDriver) {
	return mAuthorDriver->setListener(listener);
    }
    return NO_INIT;
}

#undef LOG_TAG
#define LOG_TAG "AuthorDriver"
AuthorDriver::AuthorDriver()
             : OsclActiveObject(OsclActiveObject::EPriorityNominal, "AuthorDriver"),
               mAuthor(NULL),
               mVideoInputMIO(NULL),
               mVideoNode(NULL),
               mAudioNode(NULL),
               mSelectedComposer(NULL),
               mComposerConfig(NULL),
               mVideoEncoderConfig(NULL),
               mAudioEncoderConfig(NULL),
               mVideoWidth(DEFAULT_FRAME_WIDTH),
               mVideoHeight(DEFAULT_FRAME_HEIGHT),
               mVideoFrameRate((int)DEFAULT_FRAME_RATE),
               mVideoEncoder(VIDEO_ENCODER_DEFAULT),
               mOutputFormat(OUTPUT_FORMAT_DEFAULT),
               mAudioEncoder(AUDIO_ENCODER_DEFAULT)
	       ,ifpOutput(NULL)
{
    mSyncSem = new OsclSemaphore();
    mSyncSem->Create();

    createThread(AuthorDriver::startAuthorThread, this);
    // mSyncSem will be signaled when the scheduler has started
    mSyncSem->Wait();
}

AuthorDriver::~AuthorDriver()
{
}

author_command *AuthorDriver::dequeueCommand()
{
    author_command *ac;
    mQueueLock.lock();

    // XXX should we assert here?
    if (mCommandQueue.empty()) {
        PendForExec();
        mQueueLock.unlock();
        return NULL;
    }

    ac = *(--mCommandQueue.end());
    mCommandQueue.erase(--mCommandQueue.end());
    if (mCommandQueue.size() > 0) {
        RunIfNotReady();
    } else {
        PendForExec();
    }
    mQueueLock.unlock();

    return ac;
}

status_t AuthorDriver::enqueueCommand(author_command *ac, media_completion_f comp, void *cookie)
{
    // If the user didn't specify a completion callback, we
    // are running in synchronous mode.
    if (comp == NULL) {
        ac->comp = AuthorDriver::syncCompletion;
        ac->cookie = this;
    } else {
        ac->comp = comp;
        ac->cookie = cookie;
    }

    // Add the command to the queue.
    mQueueLock.lock();
    mCommandQueue.push_front(ac);

    // make a copy of this semaphore for special handling of the AUTHOR_QUIT command
    OsclSemaphore *syncsemcopy = mSyncSem;
    // make a copy of ac->which, since ac will be deleted by the standard completion function
    author_command_type whichcopy = ac->which;

    // Wake up the author thread so it can dequeue the command.
    if (mCommandQueue.size() == 1) {
        PendComplete(OSCL_REQUEST_ERR_NONE);
    }
    mQueueLock.unlock();
    // If we are in synchronous mode, wait for completion.
    if (syncsemcopy) {
        syncsemcopy->Wait();
        if (whichcopy == AUTHOR_QUIT) {
            syncsemcopy->Close();
            delete syncsemcopy;
            return 0;
        }
        return mSyncStatus;
    }

    return OK;
}

void AuthorDriver::FinishNonAsyncCommand(author_command *ac)
{
    ac->comp(0, ac->cookie); // this signals the semaphore for synchronous calls
    delete ac;
}

// The OSCL scheduler calls this when we get to run (this should happen only
// when a command has been enqueued for us).
void AuthorDriver::Run()
{
    author_command *ac;

    ac = dequeueCommand();
    if (ac == NULL) {
        // assert?
        return;
    }

    switch(ac->which) {
    case AUTHOR_INIT:
        handleInit(ac);
        break;

    case AUTHOR_SET_AUDIO_SOURCE:
        handleSetAudioSource((set_audio_source_command *)ac);
        break;

    case AUTHOR_SET_VIDEO_SOURCE:
        handleSetVideoSource((set_video_source_command *)ac);
        break;

    case AUTHOR_SET_OUTPUT_FORMAT:
        handleSetOutputFormat((set_output_format_command *)ac);
        break;

    case AUTHOR_SET_AUDIO_ENCODER:
        handleSetAudioEncoder((set_audio_encoder_command *)ac);
        break;

    case AUTHOR_SET_VIDEO_ENCODER:
        handleSetVideoEncoder((set_video_encoder_command *)ac);
        break;

    case AUTHOR_SET_VIDEO_SIZE:
        handleSetVideoSize((set_video_size_command *)ac);
        return;

    case AUTHOR_SET_VIDEO_FRAME_RATE:
        handleSetVideoFrameRate((set_video_frame_rate_command *)ac);
        return;

    case AUTHOR_SET_PREVIEW_SURFACE:
        handleSetPreviewSurface((set_preview_surface_command *)ac);
        return;

    case AUTHOR_SET_CAMERA:
        handleSetCamera((set_camera_command *)ac);
        return;

    case AUTHOR_SET_OUTPUT_FILE:
        handleSetOutputFile((set_output_file_command *)ac);
        return;

    case AUTHOR_SET_PARAMETERS:
        handleSetParameters((set_parameters_command *)ac);
        return;

    case AUTHOR_REMOVE_VIDEO_SOURCE:
        handleRemoveVideoSource(ac);
        return;

    case AUTHOR_REMOVE_AUDIO_SOURCE:
        handleRemoveAudioSource(ac);
        return;

    case AUTHOR_PREPARE: handlePrepare(ac); break;
    case AUTHOR_START: handleStart(ac); break;
    case AUTHOR_STOP: handleStop(ac); break;
    case AUTHOR_CLOSE: handleClose(ac); break;
    case AUTHOR_RESET: handleReset(ac); break;
    case AUTHOR_QUIT: handleQuit(ac); return;

    default:
        assert(0);
        break;
    }

 }

void AuthorDriver::commandFailed(author_command *ac)
{
    LOGE("Command (%d) failed", ac->which);
    ac->comp(UNKNOWN_ERROR, ac->cookie);
    delete ac;
}

void AuthorDriver::handleInit(author_command *ac)
{
    int error = 0;
    OSCL_TRY(error, mAuthor->Open(ac));
    OSCL_FIRST_CATCH_ANY(error, commandFailed(ac));
}

void AuthorDriver::handleSetAudioSource(set_audio_source_command *ac)
{
    int error = 0;

    switch(ac->as) {
    case AUDIO_SOURCE_DEFAULT:
    case AUDIO_SOURCE_MIC:
        mAudioInputMIO = new AndroidAudioInput();
        if(mAudioInputMIO != NULL){
            LOGV("create mio input audio");
            mAudioNode = PvmfMediaInputNodeFactory::Create(static_cast<PvmiMIOControl *>(mAudioInputMIO.get()));
            if(mAudioNode){
                break;
            }
            else{
            // do nothing, let it go in default case
            }
        }
        else{
        // do nothing, let it go in default case
        }
    default:
        commandFailed(ac);
        return;
    }

    OSCL_TRY(error, mAuthor->AddDataSource(*mAudioNode, ac));
    OSCL_FIRST_CATCH_ANY(error, commandFailed(ac));
}

void AuthorDriver::handleSetVideoSource(set_video_source_command *ac)
{
    int error = 0;

    switch(ac->vs) {
    case VIDEO_SOURCE_DEFAULT:
    case VIDEO_SOURCE_CAMERA: {
        AndroidCameraInput* cameraInput = new AndroidCameraInput();
        if (cameraInput) {
            LOGV("create mio input video");
            mVideoNode = PvmfMediaInputNodeFactory::Create(cameraInput);
            if (mVideoNode) {
                // pass in the application supplied camera object
                if (mCamera != 0) cameraInput->SetCamera(mCamera);
                mVideoInputMIO = cameraInput;
                break;
            }
            delete cameraInput;
        }
        commandFailed(ac);
        return;
    }
    default:
        commandFailed(ac);
        return;
    }

    OSCL_TRY(error, mAuthor->AddDataSource(*mVideoNode, ac));
    OSCL_FIRST_CATCH_ANY(error, commandFailed(ac));
}

void AuthorDriver::handleSetOutputFormat(set_output_format_command *ac)
{
    int error = 0;
    OSCL_HeapString<OsclMemAllocator> iComposerMimeType;

    if (ac->of == OUTPUT_FORMAT_DEFAULT) {
        ac->of = OUTPUT_FORMAT_THREE_GPP;
    }

    switch(ac->of) {
    case OUTPUT_FORMAT_THREE_GPP:
        iComposerMimeType = "/x-pvmf/ff-mux/3gp";
        break;

    case OUTPUT_FORMAT_MPEG_4:
        iComposerMimeType = "/x-pvmf/ff-mux/mp4";
        break;

    case OUTPUT_FORMAT_RAW_AMR:
        iComposerMimeType = "/x-pvmf/ff-mux/amr-nb"; 
        break;

    default:
        LOGE("Ln %d unsupported file format: %d", __LINE__, ac->of);
        commandFailed(ac);
        return;
    }

    mOutputFormat = ac->of;

    OSCL_TRY(error, mAuthor->SelectComposer(iComposerMimeType, mComposerConfig, ac));
    OSCL_FIRST_CATCH_ANY(error, commandFailed(ac));
}

void AuthorDriver::media_track_added(status_t status, void *cookie)
{
    AuthorDriver *ad = (AuthorDriver *)cookie;

    ad->mSyncSem->Signal();
}

void AuthorDriver::handleSetAudioEncoder(set_audio_encoder_command *ac)
{
    int error = 0;
    OSCL_HeapString<OsclMemAllocator> iAudioEncoderMimeType;

    if (ac->ae == AUDIO_ENCODER_DEFAULT)
        ac->ae = AUDIO_ENCODER_AMR_NB;

    switch(ac->ae) {
    case AUDIO_ENCODER_AMR_NB:
        iAudioEncoderMimeType = "/x-pvmf/audio/encode/amr-nb";
        break;

	default:
        commandFailed(ac);
        return;
    }

    mAudioEncoder = ac->ae;

    OSCL_TRY(error, mAuthor->AddMediaTrack(*mAudioNode, iAudioEncoderMimeType, mSelectedComposer, mAudioEncoderConfig, ac));
    OSCL_FIRST_CATCH_ANY(error, commandFailed(ac));
}

void AuthorDriver::handleSetVideoEncoder(set_video_encoder_command *ac)
{
    int error = 0;
    OSCL_HeapString<OsclMemAllocator> iVideoEncoderMimeType;

    if (ac->ve == VIDEO_ENCODER_DEFAULT)
        ac->ve = VIDEO_ENCODER_H263;

    switch(ac->ve) {
    case VIDEO_ENCODER_H263:
        iVideoEncoderMimeType = "/x-pvmf/video/encode/h263";
        break;

    case VIDEO_ENCODER_H264:
        iVideoEncoderMimeType = "/x-pvmf/video/encode/h264";
        break;

    case VIDEO_ENCODER_MPEG_4_SP:
        iVideoEncoderMimeType = "/x-pvmf/video/encode/mp4";
        break;

    default:
        commandFailed(ac);
        return;
    }

    mVideoEncoder = ac->ve;

    OSCL_TRY(error, mAuthor->AddMediaTrack(*mVideoNode, iVideoEncoderMimeType, mSelectedComposer, mVideoEncoderConfig, ac));
    OSCL_FIRST_CATCH_ANY(error, commandFailed(ac));
}

void AuthorDriver::handleSetVideoSize(set_video_size_command *ac)
{
    if (mVideoInputMIO == NULL) {
        LOGE("camera MIO is NULL");
        commandFailed(ac);
        return;
    }

    // FIXME:
    // Platform-specific and temporal workaround to prevent video size from being set too large
    if (ac->width > ANDROID_MAX_ENCODED_FRAME_WIDTH) {
        LOGW("Intended width(%d) exceeds the max allowed width(%d). Max width is used instead.", ac->width, ANDROID_MAX_ENCODED_FRAME_WIDTH);
        mVideoWidth = ANDROID_MAX_ENCODED_FRAME_WIDTH;
    } else {
        mVideoWidth = ac->width;
    }
    if (ac->height > ANDROID_MAX_ENCODED_FRAME_HEIGHT) {
        LOGW("Intended height(%d) exceeds the max allowed height(%d). Max height is used instead.", ac->height, ANDROID_MAX_ENCODED_FRAME_HEIGHT);
        mVideoHeight = ANDROID_MAX_ENCODED_FRAME_HEIGHT;
    } else {
        mVideoHeight = ac->height;
    }

    ((AndroidCameraInput *)mVideoInputMIO)->SetFrameSize(mVideoWidth, mVideoHeight);
    FinishNonAsyncCommand(ac);
}

void AuthorDriver::handleSetVideoFrameRate(set_video_frame_rate_command *ac)
{
    if (mVideoInputMIO == NULL) {
        LOGE("camera MIO is NULL");
        commandFailed(ac);
        return;
    }

    mVideoFrameRate = ac->rate;

    ((AndroidCameraInput *)mVideoInputMIO)->SetFrameRate(ac->rate);
    FinishNonAsyncCommand(ac);
}

void AuthorDriver::handleSetCamera(set_camera_command *ac)
{
    LOGV("mCamera = %p", ac->camera.get());
    mCamera = ac->camera;
    FinishNonAsyncCommand(ac);
}

void AuthorDriver::handleSetPreviewSurface(set_preview_surface_command *ac)
{
    if (mVideoInputMIO == NULL) {
        LOGE("camera MIO is NULL");
        commandFailed(ac);
        return;
    }

    ((AndroidCameraInput *)mVideoInputMIO)->SetPreviewSurface(ac->surface);
    FinishNonAsyncCommand(ac);
}

void AuthorDriver::handleSetOutputFile(set_output_file_command *ac)
{
    PVMFStatus ret = PVMFFailure;
    PvmfFileOutputNodeConfigInterface *config = NULL;

    if (!mComposerConfig) goto exit;

    config = OSCL_STATIC_CAST(PvmfFileOutputNodeConfigInterface*, mComposerConfig);
    if (!config) goto exit;

    ifpOutput = fdopen(ac->fd, "wb");
    if (NULL == ifpOutput) {
        LOGE("Ln %d fopen() error", __LINE__);
        goto exit;
    }
	
    if ( OUTPUT_FORMAT_RAW_AMR == mOutputFormat ) {
        PvmfFileOutputNodeConfigInterface *config = OSCL_DYNAMIC_CAST(PvmfFileOutputNodeConfigInterface*, mComposerConfig);
        if (!config) goto exit;
        
        ret = config->SetOutputFileDescriptor(&OsclFileHandle(ifpOutput));
    }  else if((OUTPUT_FORMAT_THREE_GPP == mOutputFormat) || (OUTPUT_FORMAT_MPEG_4 == mOutputFormat)) {
        PVMp4FFCNClipConfigInterface *config = OSCL_DYNAMIC_CAST(PVMp4FFCNClipConfigInterface*, mComposerConfig);
        if (!config) goto exit;
        
        config->SetPresentationTimescale(1000);
        ret = config->SetOutputFileDescriptor(&OsclFileHandle(ifpOutput));
    }
    

exit:
    if (ret == PVMFSuccess) {
        FinishNonAsyncCommand(ac);
    } else {
        LOGE("Ln %d SetOutputFile() error", __LINE__);
	if (ifpOutput) {
	    fclose(ifpOutput);
	    ifpOutput = NULL;
	}
        commandFailed(ac);
    }
}
PVMFStatus AuthorDriver::setMaxDurationOrFileSize(
        int64_t limit, bool limit_is_duration) {
    PVInterface *interface;
    PvmfComposerSizeAndDurationInterface *durationConfig;

    if (limit > 0xffffffff) {
        // PV API expects this to fit in a uint32.
        return PVMFErrArgument;
    }

    if (!mComposerConfig) {
        return PVMFFailure;
    }

    mComposerConfig->queryInterface(
            PvmfComposerSizeAndDurationUuid, interface);

    durationConfig =
        OSCL_DYNAMIC_CAST(PvmfComposerSizeAndDurationInterface *, interface);

    if (!durationConfig) {
        return PVMFFailure;
    }

    PVMFStatus ret;
    if (limit_is_duration) {
        // SetMaxDuration's first parameter is a boolean "enable", we enable
        // enforcement of the maximum duration if it's (strictly) positive,
        // otherwise we take it to imply disabling.
        ret = durationConfig->SetMaxDuration(
                limit > 0, static_cast<uint32>(limit));
    } else {
        // SetMaxFileSize's first parameter is a boolean "enable", we enable
        // enforcement of the maximum filesize if it's (strictly) positive,
        // otherwise we take it to imply disabling.
        ret = durationConfig->SetMaxFileSize(
                limit > 0, static_cast<uint32>(limit));
    }

    durationConfig->removeRef();
    durationConfig = NULL;

    return ret;
}

// Attempt to parse an int64 literal optionally surrounded by whitespace,
// returns true on success, false otherwise.
static bool safe_strtoi64(const char *s, int64 *val) {
    char *end;
    *val = strtoll(s, &end, 10);

    if (end == s || errno == ERANGE) {
        return false;
    }

    // Skip trailing whitespace
    while (isspace(*end)) {
        ++end;
    }

    // For a successful return, the string must contain nothing but a valid
    // int64 literal optionally surrounded by whitespace.

    return *end == '\0';
}

// Trim both leading and trailing whitespace from the given string.
static void TrimString(String8 *s) {
    size_t num_bytes = s->bytes();
    const char *data = s->string();

    size_t leading_space = 0;
    while (leading_space < num_bytes && isspace(data[leading_space])) {
        ++leading_space;
    }

    size_t i = num_bytes;
    while (i > leading_space && isspace(data[i - 1])) {
        --i;
    }

    s->setTo(String8(&data[leading_space], i - leading_space));
}

PVMFStatus AuthorDriver::setParameter(
        const String8& key, const String8& value) {
    if (key == "max-duration") {
        int64_t max_duration_ms;
        if (safe_strtoi64(value.string(), &max_duration_ms)) {
            return setMaxDurationOrFileSize(
                    max_duration_ms, true /* limit_is_duration */);
        }
    } else if (key == "max-filesize") {
        int64_t max_filesize_bytes;
        if (safe_strtoi64(value.string(), &max_filesize_bytes)) {
            return setMaxDurationOrFileSize(
                    max_filesize_bytes, false /* limit is filesize */);
        }
    }

    return PVMFErrArgument;
}

// Applies the requested parameters, completes either successfully or stops
// application of parameters upon encountering the first error, finishing the
// transaction with the failure result of that initial failure.
void AuthorDriver::handleSetParameters(set_parameters_command *ac) {
    PVMFStatus ret = PVMFSuccess;

    const char *params = ac->params().string();
    const char *key_start = params;
    for (;;) {
        const char *equal_pos = strchr(key_start, '=');
        if (equal_pos == NULL) {
            // This key is missing a value.

            ret = PVMFErrArgument;
            break;
        }

        String8 key(key_start, equal_pos - key_start);
        TrimString(&key);

        if (key.length() == 0) {
            ret = PVMFErrArgument;
            break;
        }

        const char *value_start = equal_pos + 1;
        const char *semicolon_pos = strchr(value_start, ';');
        String8 value;
        if (semicolon_pos == NULL) {
            value.setTo(value_start);
        } else {
            value.setTo(value_start, semicolon_pos - value_start);
        }

        ret = setParameter(key, value);

        if (ret != PVMFSuccess) {
            LOGE("setParameter(%s = %s) failed with result %d",
                 key.string(), value.string(), ret);
            break;
        }

        if (semicolon_pos == NULL) {
            break;
        }

        key_start = semicolon_pos + 1;
    }

    if (ret == PVMFSuccess) {
        FinishNonAsyncCommand(ac);
    } else {
        LOGE("Ln %d handleSetParameters(%s) error", __LINE__, params);
        commandFailed(ac);
    }
}

void AuthorDriver::handlePrepare(author_command *ac)
{
    int error = 0;
    OSCL_TRY(error, mAuthor->Init(ac));
    OSCL_FIRST_CATCH_ANY(error, commandFailed(ac));
}

void AuthorDriver::handleStart(author_command *ac)
{
    int error = 0;
    OSCL_TRY(error, mAuthor->Start(ac));
    OSCL_FIRST_CATCH_ANY(error, commandFailed(ac));
}

void AuthorDriver::handleStop(author_command *ac)
{
    int error = 0;
    OSCL_TRY(error, mAuthor->Stop(ac));
    OSCL_FIRST_CATCH_ANY(error, commandFailed(ac));
}

void AuthorDriver::handleClose(author_command *ac)
{
    int error = 0;
    OSCL_TRY(error, mAuthor->Close(ac));
    OSCL_FIRST_CATCH_ANY(error, commandFailed(ac));
}

void AuthorDriver::handleReset(author_command *ac)
{
    removeConfigRefs(ac);
    int error = 0;
    OSCL_TRY(error, mAuthor->Reset(ac));
    OSCL_FIRST_CATCH_ANY(error, commandFailed(ac));
}
void AuthorDriver::handleRemoveVideoSource(author_command *ac)
{
    LOGV("handleRemoveVideoSource");
    if (mVideoNode) {
        int error = 0;
        OSCL_TRY(error, mAuthor->RemoveDataSource(*mVideoNode, ac));
        OSCL_FIRST_CATCH_ANY(error, commandFailed(ac));
    } else {
       FinishNonAsyncCommand(ac); 
    }
}

void AuthorDriver::handleRemoveAudioSource(author_command *ac)
{
    LOGV("handleRemoveAudioSource");
    if (mAudioNode) {
        int error = 0;
        OSCL_TRY(error, mAuthor->RemoveDataSource(*mAudioNode, ac));
        OSCL_FIRST_CATCH_ANY(error, commandFailed(ac));
    } else {
        FinishNonAsyncCommand(ac);
    }
}

void AuthorDriver::removeConfigRefs(author_command *ac)
{
    LOGV("removeConfigRefs");

    if (mComposerConfig) {
        mComposerConfig->removeRef();
        mComposerConfig = NULL;
    }
    if (mVideoEncoderConfig) {
        mVideoEncoderConfig->removeRef();
        mVideoEncoderConfig = NULL; 
    }
    if (mAudioEncoderConfig) {
        mAudioEncoderConfig->removeRef();
        mAudioEncoderConfig = NULL;
    }
}


void AuthorDriver::handleQuit(author_command *ac)
{
    OsclExecScheduler *sched = OsclExecScheduler::Current();
    sched->StopScheduler();
}

/*static*/ int AuthorDriver::startAuthorThread(void *cookie)
{
    prctl(PR_SET_NAME, (unsigned long) "PV author", 0, 0, 0);
    AuthorDriver *ed = (AuthorDriver *)cookie;
    return ed->authorThread();
}

void AuthorDriver::doCleanUp()
{
    if (ifpOutput) {
	fclose(ifpOutput);
	ifpOutput = NULL;
    }

    if (mCamera != NULL) {
        mCamera.clear();
    }

    if (mVideoNode) {
        PvmfMediaInputNodeFactory::Delete(mVideoNode);
        mVideoNode = NULL;
        delete mVideoInputMIO;
        mVideoInputMIO = NULL;
    }

    if (mAudioNode) {
        PvmfMediaInputNodeFactory::Delete(mAudioNode);
        mAudioNode = NULL;
        mAudioInputMIO.clear();
    }
}

int AuthorDriver::authorThread()
{
    int error;

    LOGV("InitializeForThread");
    if (!InitializeForThread()) {
        LOGE("InitializeForThread failed");
        mAuthor = NULL;
        mSyncSem->Signal();
        return -1;
    }

    LOGV("OMX_Init");
    OMX_Init();

    OsclScheduler::Init("AndroidAuthorDriver");
    LOGV("Create author ...");
    OSCL_TRY(error, mAuthor = PVAuthorEngineFactory::CreateAuthor(this, this, this));
    if (error) {
        // Just crash the first time someone tries to use it for now?
        LOGE("authorThread init error");
        mAuthor = NULL;
        mSyncSem->Signal();
        return -1;
    }

    AddToScheduler();
    PendForExec();

    OsclExecScheduler *sched = OsclExecScheduler::Current();
    sched->StartScheduler(mSyncSem);
    LOGV("Delete Author");
    PVAuthorEngineFactory::DeleteAuthor(mAuthor);
    mAuthor = NULL;
 

    // Let the destructor know that we're out
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

   //moved below delete this, similar code on playerdriver.cpp caused a crash.
   //cleanup of oscl should happen at the end.
    OMX_Deinit();
    UninitializeForThread();
    return 0;
}

/*static*/ void AuthorDriver::syncCompletion(status_t s, void *cookie)
{
    AuthorDriver *ed = (AuthorDriver *)cookie;
    ed->mSyncStatus = s;
    ed->mSyncSem->Signal();
}

void AuthorDriver::CommandCompleted(const PVCmdResponse& aResponse)
{
    author_command *ac = (author_command *)aResponse.GetContext();
    status_t s = 0;

    // XXX do we want to make this illegal?  combo commands like
    // SETUP_DEFAULT_SINKS will need some changing
    if (ac == NULL) {
        // NULL command may be expected from the author engine from
        // time to time?
        // LOGE("Unexpected NULL command");
        return;
    }

    LOGV("Command (%d) completed with status(%d)", ac->which, aResponse.GetCmdStatus());
    if (ac->which == AUTHOR_SET_OUTPUT_FORMAT) {
        mSelectedComposer = aResponse.GetResponseData();
    }

    if (ac->which == AUTHOR_SET_VIDEO_ENCODER) {
        switch(mVideoEncoder) {
        case VIDEO_ENCODER_H263: {
            PVMp4H263EncExtensionInterface *config = OSCL_STATIC_CAST(PVMp4H263EncExtensionInterface*,
                                                                      mVideoEncoderConfig);
            // TODO:
            // fix the hardcoded bit rate settings.
            if (config) {
                int bitrate_setting = 192000;
                if (mVideoWidth >= 480) {
                    bitrate_setting = 420000; // unstable
                } else if (mVideoWidth >= 352) {
                    bitrate_setting = 360000;
                } else if (mVideoWidth >= 320) {
                    bitrate_setting = 320000;
                }
                config->SetNumLayers(1);
                config->SetOutputBitRate(0, bitrate_setting);
                config->SetOutputFrameSize(0, mVideoWidth, mVideoHeight);
                config->SetOutputFrameRate(0, mVideoFrameRate);
                config->SetIFrameInterval(mVideoFrameRate);
            }
        } break;
        case VIDEO_ENCODER_MPEG_4_SP: {
            PVMp4H263EncExtensionInterface *config = OSCL_STATIC_CAST(PVMp4H263EncExtensionInterface*,
                                                                      mVideoEncoderConfig);
            if (config) {
                int bitrate_setting = 192000;
                if (mVideoWidth >= 480) {
                    bitrate_setting = 420000; // unstable
                } else if (mVideoWidth >= 352) {
                    bitrate_setting = 360000;
                } else if (mVideoWidth >= 320) {
                    bitrate_setting = 320000;
                }
                config->SetNumLayers(1);
                config->SetOutputBitRate(0, bitrate_setting);
                config->SetOutputFrameSize(0, mVideoWidth, mVideoHeight);
                config->SetOutputFrameRate(0, mVideoFrameRate);
                config->SetIFrameInterval(mVideoFrameRate);
            }
        } break;

        default:
            break;
        }
    }

    if (ac->which == AUTHOR_SET_AUDIO_ENCODER) {
        switch(mAudioEncoder) {
        case AUDIO_ENCODER_AMR_NB: {
            PVAudioEncExtensionInterface *config = OSCL_STATIC_CAST(PVAudioEncExtensionInterface*,
                                                                  mAudioEncoderConfig);
                if (config) {
                    config->SetMaxNumOutputFramesPerBuffer(10);
                    config->SetOutputBitRate(GSM_AMR_5_15); // 5150 bps XXX
                }
            } break;

        default:
            break;
        }
    }

    // delete video and/or audio nodes to prevent memory leakage
    // when an authroing session is reused
    if (ac->which == AUTHOR_CLOSE) {
        LOGV("doCleanUp");
        doCleanUp();
    }

    // Translate the PVMF error codes into Android ones 
    switch(aResponse.GetCmdStatus()) {
    case PVMFSuccess: s = android::OK; break;
    case PVMFPending: *(char *)0 = 0; break; /* XXX assert */
    default: s = android::UNKNOWN_ERROR;
    }

    // Call the user's requested completion function
    ac->comp(s, ac->cookie);

    delete ac;
}

void AuthorDriver::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    LOGE("HandleErrorEvent(%d)", aEvent.GetEventType());

    if (mListener != NULL) {
	mListener->notify(
		MEDIA_RECORDER_EVENT_ERROR, MEDIA_RECORDER_ERROR_UNKNOWN,
		aEvent.GetEventType());
    }
}

static int GetMediaRecorderInfoCode(const PVAsyncInformationalEvent& aEvent) {
    switch (aEvent.GetEventType()) {
        case PVMF_COMPOSER_MAXDURATION_REACHED:
            return MEDIA_RECORDER_INFO_MAX_DURATION_REACHED;

        case PVMF_COMPOSER_MAXFILESIZE_REACHED:
            return MEDIA_RECORDER_INFO_MAX_FILESIZE_REACHED;

        default:
            return MEDIA_RECORDER_INFO_UNKNOWN;
    }
}


void AuthorDriver::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    const PVEventType event_type = aEvent.GetEventType();
    assert(!IsPVMFErrCode(event_type));
    if (IsPVMFInfoCode(event_type)) {
        LOGV("HandleInformationalEvent(%d:%s)",
             event_type, PVMFStatusToString(event_type));
    } else {
        LOGV("HandleInformationalEvent(%d)", event_type);
    }

    mListener->notify(
            MEDIA_RECORDER_EVENT_INFO,
            GetMediaRecorderInfoCode(aEvent),
            aEvent.GetEventType());
}

status_t AuthorDriver::setListener(const sp<IMediaPlayerClient>& listener) {
    mListener = listener;

    return android::OK;
}

status_t AuthorDriver::getMaxAmplitude(int *max)
{
    if (mAudioInputMIO == NULL) {
        return android::UNKNOWN_ERROR;
    }
    *max = mAudioInputMIO->maxAmplitude();
    return android::OK;
}

PVAEState AuthorDriver::getAuthorEngineState()
{
    if (mAuthor) {
        return mAuthor->GetPVAuthorState();
    }
    return PVAE_STATE_IDLE;
}

