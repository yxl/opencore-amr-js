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
#include "video_only_resume_close_test.h"



void video_only_resume_close_test::test()
{
    fprintf(fileoutput, "Start video only resume close test, proxy %d.\n", iUseProxy);
    int error = 0;

    scheduler = OsclExecScheduler::Current();

    this->AddToScheduler();

    if (start_async_test())
    {
        OSCL_TRY(error, scheduler->StartScheduler());
        if (error != 0)
        {
            OSCL_LEAVE(error);
        }
    }

    destroy_sink_source();

    this->RemoveFromScheduler();
}


void video_only_resume_close_test::Run()
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

    scheduler->StopScheduler();
}

void video_only_resume_close_test::DoCancel()
{
}


void video_only_resume_close_test::HandleInformationalEventL(const CPVCmnAsyncInfoEvent& aEvent)
{
    int error = 0;

    switch (aEvent.GetEventType())
    {
        case PVT_INDICATION_INCOMING_TRACK:
            TPVChannelId id;

            if (((CPVCmnAsyncEvent&)aEvent).GetLocalBuffer()[0] == PV_VIDEO)
            {
                OSCL_TRY(error, iVideoAddSinkId = terminal->AddDataSinkL(*iVideoSink, id));
                if (error)
                {
                    test_is_true(false);
                    disconnect();
                }
            }
            break;

        case PVT_INDICATION_DISCONNECT:
            iAudioSourceAdded = false;
            iVideoSourceAdded = false;
            iAudioSinkAdded = false;
            iVideoSinkAdded = false;
            break;

        case PVT_INDICATION_CLOSE_TRACK:
            if (((CPVCmnAsyncEvent&)aEvent).GetLocalBuffer()[0] == PV_VIDEO)
            {
                if (((CPVCmnAsyncEvent&)aEvent).GetLocalBuffer()[1] == INCOMING)
                {
                    iVideoSinkAdded = false;
                }
                else
                {
                    iVideoSourceAdded = false;
                }

                if (check_video_stopped())
                {
                    test_is_true(true);
                    disconnect();
                }
            }
            break;

        case PVT_INDICATION_INTERNAL_ERROR:
            break;

        default:
            break;
    }
}

void video_only_resume_close_test::CommandCompletedL(const CPVCmnCmdResp& aResponse)
{
    int error = 0;
    switch (aResponse.GetCmdType())
    {
        case PVT_COMMAND_INIT:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                OSCL_TRY(error, terminal->ConnectL(iConnectOptions));
                if (error)
                {
                    test_is_true(false);
                    reset();
                }
                else
                {
                }
            }
            else
            {
                test_is_true(false);
                RunIfNotReady();
            }
            break;

        case PVT_COMMAND_RESET:
            RunIfNotReady();
            break;

        case PVT_COMMAND_ADD_DATA_SOURCE:
            if (aResponse.GetCmdId() == iVideoAddSourceId)
            {
                if (aResponse.GetCmdStatus() == PVMFSuccess)
                {
                    iVideoSourceAdded = true;

                    OSCL_TRY(error, iVideoPauseSourceId = terminal->PauseL(*iVideoSource));
                    if (error)
                    {
                        test_is_true(false);
                        disconnect();
                    }
                }
                else
                {
                    test_is_true(false);
                    disconnect();
                }
                iVideoAddSourceId = 0;
            }
            break;

        case PVT_COMMAND_REMOVE_DATA_SOURCE:
            if (aResponse.GetCmdId() == iVideoRemoveSourceId)
            {
                iVideoRemoveSourceId = 0;
                iVideoSourceAdded = false;

                if (check_video_stopped())
                {
                    test_is_true(true);
                    disconnect();
                }
            }
            break;

        case PVT_COMMAND_ADD_DATA_SINK:
            if (aResponse.GetCmdId() == iVideoAddSinkId)
            {
                if (aResponse.GetCmdStatus() == PVMFSuccess)
                {
                    iVideoSinkAdded = true;

                    OSCL_TRY(error, iVideoPauseSinkId = terminal->PauseL(*iVideoSink));
                    if (error)
                    {
                        test_is_true(false);
                        disconnect();
                    }
                }
                else
                {
                    test_is_true(false);
                    disconnect();
                }
                iVideoAddSinkId = 0;
            }
            break;

        case PVT_COMMAND_REMOVE_DATA_SINK:
            if (aResponse.GetCmdId() == iVideoRemoveSinkId)
            {
                iVideoRemoveSinkId  = 0;
                iVideoSinkAdded = false;

                if (check_video_stopped())
                {
                    test_is_true(true);
                    disconnect();
                }
            }
            break;

        case PVT_COMMAND_CONNECT:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                OSCL_TRY(error, iVideoAddSourceId = terminal->AddDataSourceL(*iVideoSource));
                if (error)
                {
                    test_is_true(false);
                    disconnect();
                }
            }
            else
            {
                test_is_true(false);
                reset();
            }
            break;

        case PVT_COMMAND_DISCONNECT:
            iAudioSourceAdded = false;
            iVideoSourceAdded = false;
            iAudioSinkAdded = false;
            iVideoSinkAdded = false;
            reset();
            break;

        case PVT_COMMAND_PAUSE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if (aResponse.GetCmdId() == iVideoPauseSourceId)
                {
                    iVideoSourcePaused = true;
                }
                else if (aResponse.GetCmdId() == iVideoPauseSinkId)
                {
                    iVideoSinkPaused = true;
                }
                else
                {
                    test_is_true(false);
                    disconnect();
                    break;
                }

                if (check_video_paused())
                {
                    OSCL_TRY(error, iVideoResumeSourceId = terminal->ResumeL(*iVideoSource));
                    if (error)
                    {
                        test_is_true(false);
                        disconnect();
                    }
                    else
                    {
                        OSCL_TRY(error, iVideoRemoveSourceId = terminal->RemoveDataSourceL(*iVideoSource));
                        if (error)
                        {
                            test_is_true(false);
                            disconnect();
                        }
                    }

                    OSCL_TRY(error, iVideoResumeSinkId = terminal->ResumeL(*iVideoSink));
                    if (error)
                    {
                        test_is_true(false);
                        disconnect();
                    }
                    else
                    {
                        OSCL_TRY(error, iVideoRemoveSinkId = terminal->RemoveDataSinkL(*iVideoSink));
                        if (error)
                        {
                            test_is_true(false);
                            disconnect();
                        }
                    }
                }
            }
            else
            {
                test_is_true(false);
                disconnect();
            }
            break;

        case PVT_COMMAND_RESUME:
            if ((aResponse.GetCmdStatus() != PVMFErrCancelled) &&
                    (aResponse.GetCmdStatus() != PVMFSuccess))
            {
                test_is_true(false);
                disconnect();
            }
            break;

        case PVT_COMMAND_CANCEL_ALL_COMMANDS:
            break;
    }
}

bool video_only_resume_close_test::start_async_test()
{
    int error = 0;

    if (iUseProxy)
    {
        OSCL_TRY(error, terminal = CPV2WayProxyFactory::CreateTerminalL(PV_324M,
                                   (MPVCmnCmdStatusObserver *) this,
                                   (MPVCmnInfoEventObserver *) this,
                                   (MPVCmnErrorEventObserver *) this));
    }
    else
    {
        OSCL_TRY(error, terminal = CPV2WayEngineFactory::CreateTerminalL(PV_324M,
                                   (MPVCmnCmdStatusObserver *) this,
                                   (MPVCmnInfoEventObserver *) this,
                                   (MPVCmnErrorEventObserver *) this));
    }

    if (error)
    {
        test_is_true(false);
        return false;
    }

    create_sink_source();

    OSCL_TRY(error, terminal->InitL(iSdkInitInfo, iCommServer));
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





