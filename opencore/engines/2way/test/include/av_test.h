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
#ifndef AV_TEST_H_INCLUDED
#define AV_TEST_H_INCLUDED

#include "test_base.h"


class av_test : public test_base
{
    public:
        av_test(PVMFFormatType audio_src_format = PVMF_MIME_AMR_IF2,
                PVMFFormatType audio_sink_format = PVMF_MIME_AMR_IF2,
                PVMFFormatType video_src_format = PVMF_MIME_YUV420,
                PVMFFormatType video_sink_format = PVMF_MIME_YUV420,
                bool aUseProxy = false)
                : test_base(audio_src_format, audio_sink_format, video_src_format, video_sink_format, aUseProxy, 1)
        {}

        ~av_test()
        {
        }

        void test();

        void Run();

        void DoCancel();

        void HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent);

        void TimerCallback();

    private:
        virtual void ConnectSucceeded();
        bool start_async_test();
        virtual void InitFailed();
        virtual void ConnectFailed();
        virtual void AudioAddSinkCompleted();
        virtual void AudioAddSourceCompleted();
        virtual void VideoAddSinkFailed();
        virtual void VideoAddSinkSucceeded();
        virtual void VideoAddSourceSucceeded();
        virtual void VideoAddSourceFailed();
        virtual void RstCmdCompleted();

        virtual void AudioRemoveSourceCompleted();
        virtual void AudioRemoveSinkCompleted();

        virtual void VideoRemoveSourceCompleted();
        virtual void VideoRemoveSinkCompleted();

        void CheckForTimeToDisconnect();
};


#endif


