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
#ifndef PFF_EOS_TEST_H_INCLUDED
#define PFF_EOS_TEST_H_INCLUDED

#include "test_base.h"


class pff_eos_test : public test_base
{
    public:
        pff_eos_test(bool aUseProxy,
                     const OSCL_wString& aFilename,
                     bool aWaitForAudioEOS,
                     bool aWaitForVideoEOS,
                     int aMaxRuns) : test_base(aUseProxy, aMaxRuns),
                iWaitForAudioEOS(aWaitForAudioEOS),
                iWaitForVideoEOS(aWaitForVideoEOS),
                iAudioEOSRecv(false),
                iVideoEOSRecv(false),
                iStopPlayLCalled(false)
        {
            iFilename = aFilename;
        };

        ~pff_eos_test()
        {
        }

        void test();

        void Run();

        void DoCancel();

        void HandleInformationalEventL(const CPVCmnAsyncInfoEvent& aEvent);

        void CommandCompletedL(const CPVCmnCmdResp& aResponse);


    private:
        bool start_async_test();

        void check_eos();

        OSCL_wHeapString<OsclMemAllocator> iFilename;

        bool iWaitForAudioEOS;
        bool iWaitForVideoEOS;

        bool iAudioEOSRecv;
        bool iVideoEOSRecv;
        bool iStopPlayLCalled;
};


#endif


