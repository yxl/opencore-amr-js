/*
 * Copyright 2008, HTC Inc.
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
#define LOG_TAG "PVMediaRecorder"
#include <utils/Log.h>

#include <media/PVMediaRecorder.h>
#include <ui/ICamera.h>
#include "authordriver.h"

namespace android {

PVMediaRecorder::PVMediaRecorder()
{
    LOGV("constructor");
    mAuthorDriverWrapper = new AuthorDriverWrapper();
}

PVMediaRecorder::~PVMediaRecorder()
{
    LOGV("destructor");
    delete mAuthorDriverWrapper;
}

status_t PVMediaRecorder::init()
{
    LOGV("init");
    if (mAuthorDriverWrapper == NULL) {
        LOGE("author driver wrapper is not initialized yet");
        return UNKNOWN_ERROR;
    }

    author_command *ac = new author_command(AUTHOR_INIT);
    if (ac == NULL) {
        LOGE("failed to construct an author command");
        return UNKNOWN_ERROR;
    }
    return mAuthorDriverWrapper->enqueueCommand(ac, 0, 0);
}

status_t PVMediaRecorder::setAudioSource(audio_source as)
{
    LOGV("setAudioSource(%d)", as);
    if (mAuthorDriverWrapper == NULL) {
        LOGE("author driver wrapper is not initialized yet");
        return UNKNOWN_ERROR;
    }

    set_audio_source_command *ac = new set_audio_source_command();
    if (ac == NULL) {
        LOGE("failed to construct an author command");
        return UNKNOWN_ERROR;
    }
    ac->as = as;
    return mAuthorDriverWrapper->enqueueCommand(ac, 0, 0);
}

status_t PVMediaRecorder::setVideoSource(video_source vs)
{
    LOGV("setVideoSource(%d)", vs);
    if (mAuthorDriverWrapper == NULL) {
        LOGE("author driver wrapper is not initialized yet");
        return UNKNOWN_ERROR;
    }

    set_video_source_command *ac = new set_video_source_command();
    if (ac == NULL) {
        LOGE("failed to construct an author command");
        return UNKNOWN_ERROR;
    }
    ac->vs = vs;
    return mAuthorDriverWrapper->enqueueCommand(ac, 0, 0);
}

status_t PVMediaRecorder::setOutputFormat(output_format of)
{
    LOGV("setOutputFormat(%d)", of);
    if (mAuthorDriverWrapper == NULL) {
        LOGE("author driver wrapper is not initialized yet");
        return UNKNOWN_ERROR;
    }

    set_output_format_command *ac = new set_output_format_command();
    if (ac == NULL) {
        LOGE("failed to construct an author command");
        return UNKNOWN_ERROR;
    }
    ac->of = of;
    return mAuthorDriverWrapper->enqueueCommand(ac, 0, 0);
}

status_t PVMediaRecorder::setOutputFile(const char *path)
{
    LOGV("setOutputFile(%s)", path);
    if (mAuthorDriverWrapper == NULL) {
        LOGE("author driver wrapper is not initialized yet");
        return NO_INIT;
    }

    // use file descriptor interface
    int fd = open(path, O_RDWR | O_CREAT );
    if (-1 == fd) {
        LOGE("Ln %d open() error %d", __LINE__, fd);
        return -errno;
    }
    return setOutputFile(fd, 0, 0);
}

status_t PVMediaRecorder::setOutputFile(int fd, int64_t offset, int64_t length)
{
    LOGV("setOutputFile(%d, %lld, %lld)", fd, offset, length);
    set_output_file_command *ac = new set_output_file_command();
    if (ac == NULL) {
        LOGE("failed to construct an author command");
        return NO_MEMORY;
    }

    ac->fd = fd;
    ac->offset = offset;
    ac->length = length;
    return mAuthorDriverWrapper->enqueueCommand(ac, 0, 0);
}

status_t PVMediaRecorder::setParameters(const String8& params) {
    LOGV("setParameters(%s)", params.string());
    set_parameters_command *command = new set_parameters_command(params);
    if (command == NULL) {
        LOGE("failed to construct an author command");

        return NO_MEMORY;
    }

    return mAuthorDriverWrapper->enqueueCommand(
            command, NULL /* completion_func */, NULL /* completion_cookie */);
}

status_t PVMediaRecorder::setAudioEncoder(audio_encoder ae)
{
    LOGV("setAudioEncoder(%d)", ae);
    if (mAuthorDriverWrapper == NULL) {
        LOGE("author driver wrapper is not initialized yet");
        return UNKNOWN_ERROR;
    }

    set_audio_encoder_command *ac = new set_audio_encoder_command();
    if (ac == NULL) {
        LOGE("failed to construct an author command");
        return UNKNOWN_ERROR;
    }
    ac->ae = ae;
    return mAuthorDriverWrapper->enqueueCommand(ac, 0, 0);
}

status_t PVMediaRecorder::setVideoEncoder(video_encoder ve)
{
    LOGV("setVideoEncoder(%d)", ve);
    if (mAuthorDriverWrapper == NULL) {
        LOGE("author driver wrapper is not initialized yet");
        return UNKNOWN_ERROR;
    }

    set_video_encoder_command *ac = new set_video_encoder_command();
    if (ac == NULL) {
        LOGE("failed to construct an author command");
        return UNKNOWN_ERROR;
    }
    ac->ve = ve;
    return mAuthorDriverWrapper->enqueueCommand(ac, 0, 0);
}

status_t PVMediaRecorder::setVideoFrameRate(int frames_per_second)
{
    LOGV("setVideoFrameRate(%d)", frames_per_second);
    if (mAuthorDriverWrapper == NULL) {
        LOGE("author driver wrapper is not initialized yet");
        return UNKNOWN_ERROR;
    }

    set_video_frame_rate_command *ac = new set_video_frame_rate_command();
    if (ac == NULL) {
        LOGE("failed to construct an author command");
        return UNKNOWN_ERROR;
    }
    ac->rate = frames_per_second;
    return mAuthorDriverWrapper->enqueueCommand(ac, 0, 0);
}

status_t PVMediaRecorder::setVideoSize(int width, int height)
{
    LOGV("setVideoSize(%d, %d)", width, height);
    if (mAuthorDriverWrapper == NULL) {
        LOGE("author driver wrapper is not initialized yet");
        return UNKNOWN_ERROR;
    }

    set_video_size_command *ac = new set_video_size_command();
    if (ac == NULL) {
        LOGE("failed to construct an author command");
        return UNKNOWN_ERROR;
    }
    ac->width = width;
    ac->height = height;
    return mAuthorDriverWrapper->enqueueCommand(ac, 0, 0);
}

status_t PVMediaRecorder::setCamera(const sp<ICamera>& camera)
{
    LOGV("setCamera(%p)", camera.get());
    if (mAuthorDriverWrapper == NULL) {
        LOGE("author driver wrapper is not initialized yet");
        return UNKNOWN_ERROR;
    }

    set_camera_command *ac = new set_camera_command();
    if (ac == NULL) {
        LOGE("failed to construct an author command");
        return UNKNOWN_ERROR;
    }
    ac->camera = camera;
    return mAuthorDriverWrapper->enqueueCommand(ac, 0, 0);
}

status_t PVMediaRecorder::setPreviewSurface(const sp<ISurface>& surface)
{
    LOGV("setPreviewSurface(%p)", surface.get());
    if (mAuthorDriverWrapper == NULL) {
        LOGE("author driver wrapper is not initialized yet");
        return UNKNOWN_ERROR;
    }

    set_preview_surface_command *ac = new set_preview_surface_command();
    if (ac == NULL) {
        LOGE("failed to construct an author command");
        return UNKNOWN_ERROR;
    }
    ac->surface = surface;
    return mAuthorDriverWrapper->enqueueCommand(ac, 0, 0);
}

status_t PVMediaRecorder::getMaxAmplitude(int *max)
{
    LOGV("getMaxAmplitude");
    if (mAuthorDriverWrapper == NULL) {
        LOGE("author driver wrapper is not initialized yet");
        return UNKNOWN_ERROR;
    }

    return mAuthorDriverWrapper->getMaxAmplitude(max);
}

status_t PVMediaRecorder::prepare()
{
    LOGV("prepare");
    if (mAuthorDriverWrapper == NULL) {
        LOGE("author driver wrapper is not initialized yet");
        return UNKNOWN_ERROR;
    }

    author_command *ac = new author_command(AUTHOR_PREPARE);
    if (ac == NULL) {
        LOGE("failed to construct an author command");
        return UNKNOWN_ERROR;
    }
    return mAuthorDriverWrapper->enqueueCommand(ac, 0, 0);
}

status_t PVMediaRecorder::start()
{
    LOGV("start");
    if (mAuthorDriverWrapper == NULL) {
        LOGE("author driver wrapper is not initialized yet");
        return UNKNOWN_ERROR;
    }

    author_command *ac = new author_command(AUTHOR_START);
    if (ac == NULL) {
        LOGE("failed to construct an author command");
        return UNKNOWN_ERROR;
    }
    return mAuthorDriverWrapper->enqueueCommand(ac, 0, 0);
}

// Make sure that stop also calls PV author engine's Reset()
// and Close() so that its internal state is maintained correctly
status_t PVMediaRecorder::stop()
{
    LOGV("stop");
    status_t ret = doStop();
    if (OK != ret)
	LOGE("stop failed");
    ret = reset();
    if (OK != ret)
	LOGE("reset failed");
    ret = close();
    if (OK != ret)
	LOGE("close failed");

    return ret;
}

status_t PVMediaRecorder::doStop()
{
    LOGV("doStop");
    if (mAuthorDriverWrapper == NULL) {
        LOGE("author driver wrapper is not initialized yet");
        return UNKNOWN_ERROR;
    }

    author_command *ac = new author_command(AUTHOR_STOP);
    if (ac == NULL) {
        LOGE("failed to construct an author command");
        return UNKNOWN_ERROR;
    }
    return mAuthorDriverWrapper->enqueueCommand(ac, 0, 0);
}

status_t PVMediaRecorder::reset()
{
    LOGV("reset");
    if (mAuthorDriverWrapper == NULL) {
        LOGE("author driver wrapper is not initialized yet");
        return UNKNOWN_ERROR;
    }

    author_command *ac = new author_command(AUTHOR_RESET);
    if (ac == NULL) {
        LOGE("failed to construct an author command");
        return UNKNOWN_ERROR;
    }
    status_t ret = mAuthorDriverWrapper->enqueueCommand(ac, 0, 0);
    if (ret != OK) {
        LOGE("failed to do reset(%d)", ret);
        return UNKNOWN_ERROR;
    }
    ret = mAuthorDriverWrapper->enqueueCommand(new author_command(AUTHOR_REMOVE_VIDEO_SOURCE), 0, 0);
    if (ret != OK) {
        LOGE("failed to remove video source(%d)", ret);
        return UNKNOWN_ERROR;
    }
    ret = mAuthorDriverWrapper->enqueueCommand(new author_command(AUTHOR_REMOVE_AUDIO_SOURCE), 0, 0);
    if (ret != OK) {
        LOGE("failed to remove audio source(%d)", ret);
        return UNKNOWN_ERROR;
    }
    return ret;
}

status_t PVMediaRecorder::close()
{
    LOGV("close");
    if (mAuthorDriverWrapper == NULL) {
        LOGE("author driver wrapper is not initialized yet");
        return UNKNOWN_ERROR;
    }

    author_command *ac = new author_command(AUTHOR_CLOSE);
    if (ac == NULL) {
        LOGE("failed to construct an author command");
        return UNKNOWN_ERROR;
    }
    return mAuthorDriverWrapper->enqueueCommand(ac, 0, 0);
}

status_t PVMediaRecorder::setListener(const sp<IMediaPlayerClient>& listener) {
    LOGV("setListener");
    if (mAuthorDriverWrapper == NULL) {
        LOGE("author driver wrapper is not initialized yet");
        return UNKNOWN_ERROR;
    }
    return mAuthorDriverWrapper->setListener(listener);
}

}; // namespace android
