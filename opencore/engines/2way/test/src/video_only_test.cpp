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
#include "video_only_test.h"

#ifndef PV_MP4_H263_ENC_EXTENSION_H_INCLUDED
#include "pvmp4h263encextension.h"
#endif

#define TRADEOFF_VALUE 5
#define TRADEOFF_VALUE_2 10


void video_only_test::test()
{
    fprintf(fileoutput, "Start video only test, proxy %d SrcFormat:", iUseProxy);

    printFormatString(iVidSrcFormatType);

    fprintf(fileoutput, " SinkFormat:");

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


void video_only_test::Run()
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

void video_only_test::DoCancel()
{
}

void video_only_test::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    int error = 0;
    switch (aEvent.GetEventType())
    {
        case PVT_INDICATION_OUTGOING_TRACK:
        {
            TPVChannelId *channel_id = (TPVChannelId *)(&aEvent.GetLocalBuffer()[4]);
            printf("Indication with logical channel #%d ", *channel_id);
            if (aEvent.GetLocalBuffer()[0] == PV_AUDIO)
            {
                printf("Audio");
            }
            else if (aEvent.GetLocalBuffer()[0] == PV_VIDEO)
            {
                iOutgoingVideo = *channel_id;
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
            if (aEvent.GetLocalBuffer()[0] == PV_AUDIO)
            {
                printf("Audio");
            }
            else if (aEvent.GetLocalBuffer()[0] == PV_VIDEO)
            {
                iIncomingVideo = *channel_id;
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

void video_only_test::H324MConfigCommandCompletedL(PVMFCmdResp& aResponse)
{
    OSCL_UNUSED_ARG(aResponse);
}

void video_only_test::H324MConfigHandleInformationalEventL(PVMFAsyncEvent& aEvent)
{
    switch (aEvent.GetEventType())
    {
        case PV_INDICATION_VIDEO_SPATIAL_TEMPORAL_TRADEOFF_COMMAND:
            if (aEvent.GetLocalBuffer()[2] == TRADEOFF_VALUE)
            {
                H324MConfigInterface * i324Interface = (H324MConfigInterface *)i324mConfigInterface;
                iTradeOffInd = i324Interface->SendVideoTemporalSpatialTradeoffIndication(iOutgoingVideo, TRADEOFF_VALUE_2);
            }
            break;
        case PV_INDICATION_VIDEO_SPATIAL_TEMPORAL_TRADEOFF_INDICATION:
            if (aEvent.GetLocalBuffer()[2] == TRADEOFF_VALUE_2)
            {
                test_is_true(true);
                timer->RunIfNotReady(TEST_DURATION);
            }
            break;
    }
}

void video_only_test::DoStuffWithH324MConfig()
{
    H324MConfigInterface * i324Interface = (H324MConfigInterface *)i324mConfigInterface;
    i324Interface->SetObserver(this);
    iTradeOffCmd = i324Interface->SendVideoTemporalSpatialTradeoffCommand(iIncomingVideo, TRADEOFF_VALUE);
    iEncIFCommandId = terminal->QueryInterface(PVMp4H263EncExtensionUUID, iVidEncIFace);
}

void video_only_test::VideoAddSinkSucceeded()
{
    iVideoSinkAdded = true;
    if (iVideoSourceAdded)
    {
        i324mConfigInterface = iH324MConfig;
//		OSCL_TRY(error, i324mIFCommandId = terminal->QueryInterface(PVH324MConfigUuid, i324mConfigInterface,NULL));
        if (i324mConfigInterface == NULL)
        {
            test_is_true(false);
            disconnect();
        }
        DoStuffWithH324MConfig();
    }
}
void video_only_test::VideoAddSourceSucceeded()
{
    iVideoSourceAdded = true;
    if (iVideoSinkAdded)
    {
        i324mConfigInterface = iH324MConfig;
//			OSCL_TRY(error, i324mIFCommandId = terminal->QueryInterface(PVH324MConfigUuid, i324mConfigInterface,NULL));
        if (i324mConfigInterface == NULL)
        {
            test_is_true(false);
            disconnect();
        }
        DoStuffWithH324MConfig();
    }
}
void video_only_test::VideoAddSourceFailed()
{
    if (iVidSrcFormatType == PVMF_MIME_M4V)
    {
        test_is_true(true);
    }
    else
    {
        test_is_true(false);
    }
    disconnect();
}

void video_only_test::EncoderIFSucceeded()
{
    PVMp4H263EncExtensionInterface *iface = (PVMp4H263EncExtensionInterface *)iVidEncIFace;
    iface->RequestIFrame();
}
void video_only_test::EncoderIFFailed()
{
    EncoderIFSucceeded();
}
void video_only_test::DisCmdFailed()
{
    DisCmdSucceeded();
}
void video_only_test::DisCmdSucceeded()
{
    printf("Finished disconnecting \n");
//	destroy_sink_source();
    if (i324mConfigInterface)
        i324mConfigInterface->removeRef();
    reset();
}

void video_only_test::InitFailed()
{
    test_is_true(false);
    test_base::InitFailed();
}

void video_only_test::ConnectSucceeded()
{
}

void video_only_test::ConnectFailed()
{
}

void video_only_test::TimerCallback()
{
    int error = 1;

    if (iSelVideoSource != NULL)
    {
        OSCL_TRY(error, iVideoRemoveSourceId = terminal->RemoveDataSource(*iSelVideoSource));
        if (error)
        {
            test_is_true(false);
            disconnect();
        }
    }
}

bool video_only_test::start_async_test()
{
    timer = new engine_timer(this);
    if (timer == NULL)
    {
        test_is_true(false);
        return false;
    }
    timer->AddToScheduler();


    return test_base::start_async_test();
}





