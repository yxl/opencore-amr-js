/*
**
** Copyright 2008, The Android Open Source Project
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

#ifndef _PLAYERDRIVER_H
#define _PLAYERDRIVER_H

#include <media/mediaplayer.h>
#include <media/MediaPlayerInterface.h>

using namespace android;

typedef void (*media_completion_f)(status_t status, void *cookie, bool cancelled);

// Commands that MediaPlayer sends to the PlayerDriver
class PlayerCommand
{
public:
    enum player_command_type {
        PLAYER_QUIT                     = 1,
        PLAYER_SETUP                    = 2,
        PLAYER_SET_DATA_SOURCE          = 3,
        PLAYER_SET_VIDEO_SURFACE        = 4,
        PLAYER_SET_AUDIO_SINK           = 5,
        PLAYER_INIT                     = 6,
        PLAYER_PREPARE                  = 7,
        PLAYER_START                    = 8,
        PLAYER_STOP                     = 9,
        PLAYER_PAUSE                    = 10,
        PLAYER_RESET                    = 11,
        PLAYER_SET_LOOP                 = 12,
        PLAYER_SEEK                     = 13,
        PLAYER_GET_POSITION             = 14,
        PLAYER_GET_DURATION             = 15,
        PLAYER_GET_STATUS               = 16,
        PLAYER_REMOVE_DATA_SOURCE       = 17,
        PLAYER_CANCEL_ALL_COMMANDS      = 18,
    };

    virtual             ~PlayerCommand() {}
    int                 command() { return mCommand; }
    media_completion_f  callback() { return mCallback; }
    void*               cookie() { return mCookie; }
    bool                hasCompletionHook() { return mCallback == 0; }
    void                complete(status_t status, bool cancelled) { mCallback(status, mCookie, cancelled); }
                        void set(media_completion_f cbf, void* cookie) { mCallback = cbf; mCookie = cookie; }
protected:
                        PlayerCommand(player_command_type command, media_completion_f cbf, void* cookie) :
                            mCommand(command), mCallback(cbf), mCookie(cookie) {}
private:
                        PlayerCommand();
    int                 mCommand;
    media_completion_f  mCallback;
    void*               mCookie;
};

class PlayerQuit : public PlayerCommand
{
public:
                        PlayerQuit(media_completion_f cbf, void* cookie) :
                            PlayerCommand(PLAYER_QUIT, cbf, cookie) {}
private:
                        PlayerQuit();
};

class PlayerSetup : public PlayerCommand
{
public:
                        PlayerSetup(media_completion_f cbf, void* cookie) :
                            PlayerCommand(PLAYER_SETUP, cbf, cookie) {}
private:
                        PlayerSetup();
};

class PlayerInit : public PlayerCommand
{
public:
                        PlayerInit(media_completion_f cbf, void* cookie) :
                            PlayerCommand(PLAYER_INIT, cbf, cookie) {}
private:
                        PlayerInit();
};

class PlayerPrepare: public PlayerCommand
{
public:
                        PlayerPrepare(media_completion_f cbf, void* cookie) :
                            PlayerCommand(PLAYER_PREPARE, cbf, cookie) {}
private:
                        PlayerPrepare();
};

class PlayerStart: public PlayerCommand
{
public:
                        PlayerStart(media_completion_f cbf, void* cookie) :
                            PlayerCommand(PLAYER_START, cbf, cookie) {}
private:
                        PlayerStart();
};

class PlayerStop: public PlayerCommand
{
public:
                        PlayerStop(media_completion_f cbf, void* cookie) :
                            PlayerCommand(PLAYER_STOP, cbf, cookie) {}
private:
                        PlayerStop();
};

class PlayerPause: public PlayerCommand
{
public:
                        PlayerPause(media_completion_f cbf, void* cookie) :
                            PlayerCommand(PLAYER_PAUSE, cbf, cookie) {}
private:
                        PlayerPause();
};

class PlayerReset: public PlayerCommand
{
public:
                        PlayerReset(media_completion_f cbf, void* cookie) :
                            PlayerCommand(PLAYER_RESET, cbf, cookie) {}
private:
                        PlayerReset();
};

class PlayerSetDataSource : public PlayerCommand
{
public:
                        PlayerSetDataSource(const char* url, media_completion_f cbf, void* cookie) :
                            PlayerCommand(PLAYER_SET_DATA_SOURCE, cbf, cookie), mUrl(0) {
                                if (url) mUrl = strdup(url); }
                        ~PlayerSetDataSource() { if (mUrl) free(mUrl); }
    const char*         url() const { return mUrl; }
private:
                        PlayerSetDataSource();
    char*               mUrl;
};

class PlayerSetVideoSurface : public PlayerCommand
{
public:
                        PlayerSetVideoSurface(const sp<ISurface>& surface, media_completion_f cbf, void* cookie) :
                            PlayerCommand(PLAYER_SET_VIDEO_SURFACE, cbf, cookie), mSurface(surface) {}
                        ~PlayerSetVideoSurface() { mSurface.clear(); }
    sp<ISurface>        surface() const { return mSurface; }
private:
                        PlayerSetVideoSurface();
    sp<ISurface>        mSurface;
};

class PlayerSetAudioSink : public PlayerCommand
{
public:
                        PlayerSetAudioSink(const sp<MediaPlayerInterface::AudioSink>& audioSink,
                                media_completion_f cbf, void* cookie) :
                            PlayerCommand(PLAYER_SET_AUDIO_SINK, cbf, cookie), mAudioSink(audioSink) {}
                        ~PlayerSetAudioSink() { mAudioSink.clear(); }
                        sp<MediaPlayerInterface::AudioSink> audioSink() { return mAudioSink; }
private:
                        PlayerSetAudioSink();
    sp<MediaPlayerInterface::AudioSink> mAudioSink;
};

class PlayerSetLoop: public PlayerCommand
{
public:
                        PlayerSetLoop(int loop, media_completion_f cbf, void* cookie) :
                            PlayerCommand(PLAYER_SET_LOOP, cbf, cookie), mLoop(loop) {}
                        int loop() { return mLoop; }
private:
                        PlayerSetLoop();
    int                 mLoop;
};

class PlayerSeek : public PlayerCommand
{
public:
                        PlayerSeek(int msec, media_completion_f cbf, void* cookie) :
                            PlayerCommand(PLAYER_SEEK, cbf, cookie), mMsec(msec) {}
    int                 msec() { return mMsec; }
private:
                        PlayerSeek();
    int                 mMsec;
};

class PlayerGetPosition: public PlayerCommand
{
public:
                        PlayerGetPosition(int* msec, media_completion_f cbf, void* cookie) :
                            PlayerCommand(PLAYER_GET_POSITION, cbf, cookie), mMsec(msec) {}
                        void set(int msecs) { if (mMsec) *mMsec = msecs; }
private:
                        PlayerGetPosition();
    int*                mMsec;
};

class PlayerGetDuration: public PlayerCommand
{
public:
                        PlayerGetDuration(int* msec, media_completion_f cbf, void* cookie) :
                            PlayerCommand(PLAYER_GET_DURATION, cbf, cookie), mMsec(msec) {}
                        void set(int msecs) { if (mMsec) *mMsec = msecs; }
private:
                        PlayerGetDuration();
    int*                mMsec;
};

class PlayerGetStatus: public PlayerCommand
{
public:
                        PlayerGetStatus(int *status, media_completion_f cbf, void* cookie) :
                            PlayerCommand(PLAYER_GET_STATUS, cbf, cookie), mStatus(status) {}
                        void set(int status) { *mStatus = status; }
private:
                        PlayerGetStatus();
    int*                mStatus;
};

class PlayerRemoveDataSource: public PlayerCommand
{
public:
                        PlayerRemoveDataSource(media_completion_f cbf, void* cookie) :
                            PlayerCommand(PLAYER_REMOVE_DATA_SOURCE, cbf, cookie) {}
private:
                        PlayerRemoveDataSource();
};

class PlayerCancelAllCommands: public PlayerCommand
{
public:
                        PlayerCancelAllCommands(media_completion_f cbf, void* cookie) :
                            PlayerCommand(PLAYER_CANCEL_ALL_COMMANDS, cbf, cookie) {}
private:
                        PlayerCancelAllCommands();
};

#endif // _PLAYERDRIVER_H
