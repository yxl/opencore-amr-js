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
#include "av_test.h"


void av_test::test()
{
    fprintf(fileoutput, "Start avtest, proxy %d,Audio SrcFormat:", iUseProxy);

    printFormatString(iAudSrcFormatType);

    fprintf(fileoutput, " Audio SinkFormat:");

    printFormatString(iAudSinkFormatType);

    fprintf(fileoutput, " Video SourceFormat:");

    printFormatString(iVidSrcFormatType);

    fprintf(fileoutput, " Video SinkFormat:");

    printFormatString(iVidSinkFormatType);

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
            OSCL_LEAVE(error);
        }
    }

    this->RemoveFromScheduler();
}


void av_test::Run()
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

void av_test::DoCancel()
{
}

void av_test::ConnectSucceeded()
{
}

void av_test::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    int error = 0;
    switch (aEvent.GetEventType())
    {
        case PVT_INDICATION_OUTGOING_TRACK:
        {
            TPVChannelId *channel_id = (TPVChannelId *)(&aEvent.GetLocalBuffer()[4]);
            printf("Indication with logical channel #%d ", *channel_id);
            if ((aEvent.GetLocalBuffer()[0] == PV_AUDIO) && !iSelAudioSource)
            {
                iSelAudioSource = get_audio_source(iAudSrcFormatType);
                if (iSelAudioSource != NULL)
                {
                    OSCL_TRY(error, iAudioAddSourceId = terminal->AddDataSource(*channel_id, *iSelAudioSource));
                    printf("Audio");
                }
            }
            else if ((aEvent.GetLocalBuffer()[0] == PV_VIDEO) && !iSelVideoSource)
            {
                iSelVideoSource = get_video_source(iVidSrcFormatType);
                if (iSelVideoSource != NULL)
                {
                    OSCL_TRY(error, iVideoAddSourceId = terminal->AddDataSource(*channel_id, *iSelVideoSource));
                    printf("Video");
                }
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
            if ((aEvent.GetLocalBuffer()[0] == PV_AUDIO) && !iSelAudioSink)
            {
                iSelAudioSink = get_audio_sink(iAudSinkFormatType);
                if (iSelAudioSink != NULL)
                {
                    OSCL_TRY(error, iAudioAddSinkId = terminal->AddDataSink(*channel_id, *iSelAudioSink));
                    printf("Audio");
                }
            }
            else if ((aEvent.GetLocalBuffer()[0] == PV_VIDEO) && !iSelVideoSink)
            {
                iSelVideoSink = get_video_sink(iVidSinkFormatType);
                if (iSelVideoSink != NULL)
                {
                    OSCL_TRY(error, iVideoAddSinkId = terminal->AddDataSink(*channel_id, *iSelVideoSink));
                    printf("Video");
                }
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

void av_test::InitFailed()
{
    test_is_true(false);
    test_base::InitFailed();
}

void av_test::ConnectFailed()
{
}

void av_test::AudioAddSinkCompleted()
{
    iAudioSinkAdded = true;
    if (iAudioSourceAdded && iVideoSourceAdded && iVideoSinkAdded)
        timer->RunIfNotReady(TEST_DURATION);
}

void av_test::AudioAddSourceCompleted()
{
    iAudioSourceAdded = true;
    if (iAudioSinkAdded && iVideoSourceAdded && iVideoSinkAdded)
        timer->RunIfNotReady(TEST_DURATION);
}

void av_test::VideoAddSinkFailed()
{
    VideoAddSinkSucceeded();
}

void av_test::VideoAddSinkSucceeded()
{
    iVideoSinkAdded = true;
    if (iVideoSourceAdded && iAudioSourceAdded && iAudioSinkAdded)
        timer->RunIfNotReady(TEST_DURATION);
}
void av_test::VideoAddSourceSucceeded()
{
    iVideoSourceAdded = true;
    if (iVideoSinkAdded && iAudioSourceAdded && iAudioSinkAdded)
        timer->RunIfNotReady(TEST_DURATION);
}
void av_test::VideoAddSourceFailed()
{
    VideoAddSourceSucceeded();
}

void av_test::RstCmdCompleted()
{
    test_is_true(true);
    test_base::RstCmdCompleted();
}

void av_test::CheckForTimeToDisconnect()
{
    if (!iAudioSourceAdded &&
            !iAudioSinkAdded &&
            !iVideoSourceAdded &&
            !iVideoSinkAdded)
    {
        disconnect();
    }
}

void av_test::AudioRemoveSourceCompleted()
{
    iAudioSourceAdded = false;
    CheckForTimeToDisconnect();
}

void av_test::AudioRemoveSinkCompleted()
{
    iAudioSinkAdded = false;
    CheckForTimeToDisconnect();
}
void av_test::VideoRemoveSourceCompleted()
{
    iVideoSourceAdded = false;
    CheckForTimeToDisconnect();
}
void av_test::VideoRemoveSinkCompleted()
{
    iVideoSinkAdded = false;
    CheckForTimeToDisconnect();
}



void av_test::TimerCallback()
{
    int error = 0;

    if (iSelVideoSource != NULL)
    {
        OSCL_TRY(error, iVideoRemoveSourceId = terminal->RemoveDataSource(*iSelVideoSource));
    }
    if (error)
    {
        iTestStatus &= false;
        disconnect();
    }
    else
    {
        error = 1;
        if (iSelVideoSink != NULL)
        {
            OSCL_TRY(error, iVideoRemoveSinkId = terminal->RemoveDataSink(*iSelVideoSink));
        }
        if (error)
        {
            iTestStatus &= false;
            disconnect();
        }
    }

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

bool av_test::start_async_test()
{
    timer = new engine_timer(this);
    if (timer == NULL)
    {
        test_is_true(false);
        return false;
    }
    iAudioSourceAdded = false;
    iAudioSinkAdded = false;

    timer->AddToScheduler();


    return test_base::start_async_test();
}





