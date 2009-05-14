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
#include "audio_only_test.h"


void audio_only_test::test()
{
    fprintf(fileoutput, "Start audio only test, proxy %d,SrcFormat:", iUseProxy);

    printFormatString(iAudSrcFormatType);

    fprintf(fileoutput, " SinkFormat:");

    printFormatString(iAudSinkFormatType);

    fprintf(fileoutput, "\n");

    int error = 0;

    scheduler = OsclExecScheduler::Current();

    this->AddToScheduler();

    init_mime_strings();

    if (start_async_test())
    {
        OSCL_TRY(error, scheduler->StartScheduler());
        if (error != 0)
        {
            test_is_true(false);
            OSCL_LEAVE(error);
        }
    }

    this->RemoveFromScheduler();
    test_is_true(iTestStatus);
}


void audio_only_test::Run()
{
    if (terminal)
    {
        if (iUseProxy)
        {
            CPV2WayProxyFactory::DeleteTerminal(terminal);
        }
        else
        {
            CPV2WayEngineFactory::DeleteTerminal(terminal);
        }
        terminal = NULL;
    }

    if (timer)
    {
        delete timer;
        timer = NULL;
    }

    scheduler->StopScheduler();
}

void audio_only_test::DoCancel()
{
}


void audio_only_test::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    int error = 0;
    switch (aEvent.GetEventType())
    {
        case PVT_INDICATION_OUTGOING_TRACK:
        {
            TPVChannelId *channel_id = (TPVChannelId *)(&aEvent.GetLocalBuffer()[4]);
            printf("Indication with logical channel #%d ", *channel_id);
            if (aEvent.GetLocalBuffer()[0] == PV_AUDIO && isFirstSink)
            {
                isFirstSink = false;
                iSelAudioSource = get_audio_source(iAudSrcFormatType);
                if (iSelAudioSource != NULL)
                {
                    OSCL_TRY(error, iAudioAddSourceId = terminal->AddDataSource(*channel_id, *iSelAudioSource));
                    printf("Audio");
                }
            }
            else if (aEvent.GetLocalBuffer()[0] == PV_VIDEO)
            {
                printf("Video");
            }
            else
            {
                printf("unknown");
            }
            printf(" outgoing Track\n");
            break;
        }

        case PVT_INDICATION_INCOMING_TRACK:
        {
            TPVChannelId *channel_id = (TPVChannelId *)(&aEvent.GetLocalBuffer()[4]);
            printf("Indication with logical channel #%d ", *channel_id);
            if (aEvent.GetLocalBuffer()[0] == PV_AUDIO && isFirstSrc)
            {
                isFirstSrc = false;
                iSelAudioSink = get_audio_sink(iAudSinkFormatType);
                if (iSelAudioSink != NULL)
                {
                    OSCL_TRY(error, iAudioAddSinkId = terminal->AddDataSink(*channel_id, *iSelAudioSink));
                    printf("Audio");
                }
            }
            else if (aEvent.GetLocalBuffer()[0] == PV_VIDEO)
            {
                printf("Video");
            }
            else
            {
                printf("unknown");
            }
            printf(" incoming Track\n");
            break;
        }

        case PVT_INDICATION_DISCONNECT:
            iAudioSourceAdded = false;
            iVideoSourceAdded = false;
            iAudioSinkAdded = false;
            iVideoSinkAdded = false;
            break;

        case PVT_INDICATION_CLOSE_TRACK:
            break;

        case PVT_INDICATION_INTERNAL_ERROR:
            break;

        default:
            break;
    }
}

void audio_only_test::ConnectSucceeded()
{
}

void audio_only_test::ConnectFailed()
{
    reset();
}


void audio_only_test::TimerCallback()
{
    int error = 1;
    timer_elapsed = true;
    if (iSelAudioSource != NULL)
    {
        OSCL_TRY(error, iAudioRemoveSourceId = terminal->RemoveDataSource(*iSelAudioSource));
    }
    if (error)
    {
        iTestStatus &= false;
        disconnect();
    }
    else
    {
        error = 1;
        if (iSelAudioSink != NULL)
        {
            OSCL_TRY(error, iAudioRemoveSinkId = terminal->RemoveDataSink(*iSelAudioSink));
        }
        if (error)
        {
            iTestStatus &= false;
            disconnect();
        }
    }

}


bool audio_only_test::start_async_test()
{
    timer = new engine_timer(this);
    if (timer == NULL)
    {
        iTestStatus &= false;
        return false;
    }

    iAudioSourceAdded = false;
    iAudioSinkAdded = false;
    isFirstSink = true;
    isFirstSrc = true;

    timer->AddToScheduler();


    return test_base::start_async_test();
}


