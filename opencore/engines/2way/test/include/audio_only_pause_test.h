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
#ifndef AUDIO_ONLY_PAUSE_TEST_H_INCLUDED
#define AUDIO_ONLY_PAUSE_TEST_H_INCLUDED

#include "test_base.h"


class audio_only_pause_test : public test_base
{
    public:
        audio_only_pause_test(bool aUseProxy, int aMaxRuns) : test_base(aUseProxy, aMaxRuns),
                iAudioSourceResumed(false),
                iAudioSinkResumed(false)
        {};

        ~audio_only_pause_test()
        {};

        void test();

        void Run();

        void DoCancel();

        void HandleInformationalEventL(const CPVCmnAsyncInfoEvent& aEvent);

        void CommandCompletedL(const CPVCmnCmdResp& aResponse);


    private:
        bool start_async_test();

        bool check_audio_resumed()
        {
            return (iAudioSourceResumed && iAudioSinkResumed);
        }

        bool iAudioSourceResumed;
        bool iAudioSinkResumed;
};


#endif


