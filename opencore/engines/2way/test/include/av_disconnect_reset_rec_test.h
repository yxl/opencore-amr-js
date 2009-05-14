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
#ifndef AV_DISCONNECT_RESET_REC_TEST_H_INCLUDED
#define AV_DISCONNECT_RESET_REC_TEST_H_INCLUDED

#include "test_base.h"


class av_disconnect_reset_rec_test : public test_base
{
    public:
        av_disconnect_reset_rec_test(bool aUseProxy) : test_base(aUseProxy),
                iAudioRecStarted(false),
                iAudioStartRecId(0),
                iAudioStopRecId(0),
                iVideoRecStarted(false),
                iVideoStartRecId(0),
                iVideoStopRecId(0),
                iIsDisconnected(true)
        {};

        ~av_disconnect_reset_rec_test()
        {
        }

        void test();

        void Run();

        void DoCancel();

        void HandleInformationalEventL(const CPVCmnAsyncInfoEvent& aEvent);

        void CommandCompletedL(const CPVCmnCmdResp& aResponse);


    private:
        bool start_async_test();

        bool check_rec_started()
        {
            return (iAudioRecStarted && iVideoRecStarted);
        }
        bool check_rec_stopped()
        {
            return (!iAudioRecStarted && !iVideoRecStarted);
        }

        bool iAudioRecStarted;
        TPVCmnCommandId iAudioStartRecId;
        TPVCmnCommandId iAudioStopRecId;
        bool iVideoRecStarted;
        TPVCmnCommandId iVideoStartRecId;
        TPVCmnCommandId iVideoStopRecId;
        bool iIsDisconnected;
};


#endif


