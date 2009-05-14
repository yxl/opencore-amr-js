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
#include "avi_test.h"

#define TEST_DURATION 1000
#define AVI_FILE_SOURCE _STRLIT("C:\\data\\testoutput_IYUV_8k_16_mono.avi")

void avi_test::test()
{
    fprintf(fileoutput, "Start avitest, proxy %d,Audio SrcFormat:", iUseProxy);

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


void avi_test::Run()
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

void avi_test::DoCancel()
{
}

void avi_test::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    int error = 0;
    switch (aEvent.GetEventType())
    {
        case PVT_INDICATION_OUTGOING_TRACK:
        {
            TPVChannelId *channel_id = (TPVChannelId *)(&aEvent.GetLocalBuffer()[4]);
            printf("Indication with logical channel #%d ", *channel_id);
            if (aEvent.GetLocalBuffer()[0] == PV_AUDIO && !iSelAudioSource)
            {
                iSelAudioSource = PvmfMediaInputNodeFactory::Create(iAudioMediaInput);
                if (iSelAudioSource != NULL)
                {
                    OSCL_TRY(error, iAudioAddSourceId = terminal->AddDataSource(*channel_id, *iSelAudioSource));
                    printf("Audio");
                }
            }
            else if (aEvent.GetLocalBuffer()[0] == PV_VIDEO && !iSelVideoSource)
            {
                iSelVideoSource = PvmfMediaInputNodeFactory::Create(iVideoMediaInput);
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
            if (aEvent.GetLocalBuffer()[0] == PV_AUDIO && !iSelAudioSink)
            {
                iSelAudioSink = get_audio_sink(iAudSinkFormatType);
                if (iSelAudioSink != NULL)
                {
                    OSCL_TRY(error, iAudioAddSinkId = terminal->AddDataSink(*channel_id, *iSelAudioSink));
                    printf("Audio");
                }
            }
            else if (aEvent.GetLocalBuffer()[0] == PV_VIDEO && !iSelVideoSink)
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

void avi_test::CommandCompleted(const PVCmdResponse& aResponse)
{
    PVCommandId cmdId = aResponse.GetCmdId();

    if (cmdId == iInitCmdId)
    {
        if (aResponse.GetCmdStatus() == PVMFSuccess)
        {
            connect();
        }
        else
        {
            test_is_true(false);
            timer->Cancel();
            RunIfNotReady();
        }
    }
    else if (iConnectCmdId == cmdId)
    {
    }
    else if (iAudioAddSinkId == cmdId)
    {
        iAudioSinkAdded = true;
        if (iAudioSourceAdded && iVideoSourceAdded && iVideoSinkAdded)
            timer->RunIfNotReady(TEST_DURATION);

    }
    else if (iAudioAddSourceId == cmdId)
    {
        iAudioSourceAdded = true;
        if (iAudioSinkAdded && iVideoSourceAdded && iVideoSinkAdded)
            timer->RunIfNotReady(TEST_DURATION);
    }
    else if (iVideoAddSinkId == cmdId)
    {
        iVideoSinkAdded = true;
        if (iVideoSourceAdded && iAudioSourceAdded && iAudioSinkAdded)
            timer->RunIfNotReady(TEST_DURATION);

    }
    else if (iVideoAddSourceId == cmdId)
    {
        iVideoSourceAdded = true;
        if (iVideoSinkAdded && iAudioSourceAdded && iAudioSinkAdded)
            timer->RunIfNotReady(TEST_DURATION);
    }
    else if (iDisCmdId == cmdId)
    {
        printf("Finished disconnecting \n");
        //	destroy_sink_source();
        reset();
    }
    else if (iRstCmdId == cmdId)
    {
        test_is_true(true);
        destroy_sink_source();
        RunIfNotReady();
    }
    else
    {
        printf("Unhandled cmd here %d\n", cmdId);
    }
}

void avi_test::TimerCallback()
{
    timer_elapsed = true;
    disconnect();
}

bool avi_test::start_async_test()
{
    int error = 0;

    timer = new engine_timer(this);
    if (timer == NULL)
    {
        test_is_true(false);
        return false;
    }
    iAudioSourceAdded = false;
    iAudioSinkAdded = false;

    timer->AddToScheduler();

    if (iUseProxy)
    {
        OSCL_TRY(error, terminal = CPV2WayProxyFactory::CreateTerminal(PV_324M,
                                   (PVCommandStatusObserver *) this,
                                   (PVInformationalEventObserver *) this,
                                   (PVErrorEventObserver *) this));
    }
    else
    {
        OSCL_TRY(error, terminal = CPV2WayEngineFactory::CreateTerminal(PV_324M,
                                   (PVCommandStatusObserver *) this,
                                   (PVInformationalEventObserver *) this,
                                   (PVErrorEventObserver *) this));
    }
    if (error)
    {
        test_is_true(false);
        return false;
    }

    create_sink_source();
    HandleAvi();
    OSCL_TRY(error, iInitCmdId = terminal->Init(iSdkInitInfo));
    if (error)
    {
        test_is_true(false);
        if (iUseProxy)
        {
            CPV2WayProxyFactory::DeleteTerminal(terminal);
        }
        else
        {
            CPV2WayEngineFactory::DeleteTerminal(terminal);
        }
        terminal = NULL;
        return false;
    }
    return true;
}

bool avi_test::HandleAvi()
{
    int32 error = 0;
    iFileServer.Connect();
    {

        OSCL_TRY(error, iFileParser = PVAviFile::CreateAviFileParser(AVI_FILE_SOURCE, error, &iFileServer););

        if (!iFileParser)
        {
            return false;
        }

        uint32 numStreams = ((PVAviFile*)iFileParser)->GetNumStreams();

        for (uint32 ii = 0; ii < numStreams; ii++)
        {
            if (oscl_strstr(((PVAviFile*)iFileParser)->GetStreamMimeType(ii).get_cstr(), "audio"))
            {
                iAudioNum = ii;
                iPVAviFile = OSCL_STATIC_CAST(PVAviFile*, iFileParser);
                {
                    iAudioMediaInput = PvmiMIOAviWavFileFactory::Create(0, false, iAudioNum, (OsclAny*)iPVAviFile, FILE_FORMAT_AVI, error);
                    if (!iAudioMediaInput)
                    {
                        return PVMFFailure;
                    }
                }

            }

            if (oscl_strstr(((PVAviFile*)iFileParser)->GetStreamMimeType(ii).get_cstr(), "video"))
            {
                iVideoNum = ii;
                {
                    iPVAviFile = OSCL_STATIC_CAST(PVAviFile*, iFileParser);
                    iVideoMediaInput = PvmiMIOAviWavFileFactory::Create(0, false, iVideoNum, (OsclAny*)iPVAviFile, FILE_FORMAT_AVI, error);
                    if (!iVideoMediaInput)
                    {
                        return PVMFFailure;
                    }

                }

            }

        }
    }
    return PVMFSuccess;
}


