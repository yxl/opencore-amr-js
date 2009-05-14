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
#ifndef AUDIO_ONLY_TEST_H_INCLUDED
#define AUDIO_ONLY_TEST_H_INCLUDED

#include "test_base.h"

class audio_only_test : public test_base
{
    public:
        audio_only_test(bool aUseProxy,
                        PVMFFormatType audio_src_format = PVMF_MIME_AMR_IF2,
                        PVMFFormatType audio_sink_format = PVMF_MIME_AMR_IF2)
                : test_base(audio_src_format, audio_sink_format, PVMF_MIME_YUV420, PVMF_MIME_YUV420, aUseProxy)
        { };

        ~audio_only_test()
        {
        }


        void test();

        void Run();

        void DoCancel();

        void HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent);


        void TimerCallback();


    private:
        virtual void ConnectSucceeded();
        virtual void ConnectFailed();
        bool start_async_test();
        bool isFirstSink, isFirstSrc;
};


#endif


