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
#include "use_play_file_test.h"

void use_play_file_test::test()
{
    int error = 0;
    char *filename = new char[iFilename.get_size() + 1];
    if ((filename == NULL) || (oscl_UnicodeToUTF8(iFilename.get_cstr(), iFilename.get_size(), filename, iFilename.get_size() + 1) == 0))
    {
        OSCL_LEAVE(-1);
    }

    fprintf(fileoutput, "Start use play file test, proxy %d, before add source %d, start play before use file %d, file %s\n", iUseProxy, iUsePlayFileBeforeAddSource, iStartPlayBeforeUsePlayFile, filename);

    delete filename;

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


void use_play_file_test::Run()
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

void use_play_file_test::DoCancel()
{
}

void use_play_file_test::HandleInformationalEventL(const CPVCmnAsyncInfoEvent& aEvent)
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

            if (((CPVCmnAsyncEvent&)aEvent).GetLocalBuffer()[0] == PV_AUDIO)
            {
                OSCL_TRY(error, iAudioAddSinkId = terminal->AddDataSinkL(*iAudioSink, id));
                if (error)
                {
                    test_is_true(false);
                    disconnect();
                }
            }
            break;

        case PVT_INDICATION_DISCONNECT:
            iIsConnected = false;
            iPlayStarted = false;
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
            }
            else if (((CPVCmnAsyncEvent&)aEvent).GetLocalBuffer()[0] == PV_AUDIO)
            {
                if (((CPVCmnAsyncEvent&)aEvent).GetLocalBuffer()[1] == INCOMING)
                {
                    iAudioSinkAdded = false;
                }
                else
                {
                    iAudioSourceAdded = false;
                }
            }
            break;

        case PVT_INDICATION_INTERNAL_ERROR:
            break;

        default:
            break;
    }
}

void use_play_file_test::CommandCompletedL(const CPVCmnCmdResp& aResponse)
{
    int error = 0;
    switch (aResponse.GetCmdType())
    {
        case PVT_COMMAND_INIT:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                OSCL_TRY(error, terminal->InitPlayFileL(iFilename));
                if (error)
                {
                    test_is_true(false);
                    reset();
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
            if (aResponse.GetCmdId() == iAudioAddSourceId)
            {
                if (aResponse.GetCmdStatus() == PVMFSuccess)
                {
                    iAudioSourceAdded = true;
                    check_av_started();
                }
                else
                {
                    test_is_true(false);
                    disconnect();
                }
                iAudioAddSourceId = 0;
            }
            else if (aResponse.GetCmdId() == iVideoAddSourceId)
            {
                if (aResponse.GetCmdStatus() == PVMFSuccess)
                {
                    iVideoSourceAdded = true;
                    check_av_started();
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
            if (aResponse.GetCmdId() == iAudioRemoveSourceId)
            {
                iAudioRemoveSourceId = 0;
                iAudioSourceAdded = false;
            }
            else if (aResponse.GetCmdId() == iVideoRemoveSourceId)
            {
                iVideoRemoveSourceId = 0;
                iVideoSourceAdded = false;
            }
            break;

        case PVT_COMMAND_ADD_DATA_SINK:
            if (aResponse.GetCmdId() == iAudioAddSinkId)
            {
                if (aResponse.GetCmdStatus() == PVMFSuccess)
                {
                    iAudioSinkAdded = true;
                    check_av_started();
                }
                else
                {
                    test_is_true(false);
                    disconnect();
                }
                iAudioAddSinkId = 0;
            }
            else if (aResponse.GetCmdId() == iVideoAddSinkId)
            {
                if (aResponse.GetCmdStatus() == PVMFSuccess)
                {
                    iVideoSinkAdded = true;
                    check_av_started();
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
            if (aResponse.GetCmdId() == iAudioRemoveSinkId)
            {
                iAudioRemoveSinkId = 0;
                iAudioSinkAdded = false;
            }
            else if (aResponse.GetCmdId() == iVideoRemoveSinkId)
            {
                iVideoRemoveSinkId  = 0;
                iVideoSinkAdded = false;
            }
            break;

        case PVT_COMMAND_CONNECT:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iIsConnected = true;

                OSCL_TRY(error, iAudioAddSourceId = terminal->AddDataSourceL(*iAudioSource));
                if (error)
                {
                    test_is_true(false);
                    disconnect();
                }

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
            iIsConnected = false;
            iPlayStarted = false;
            iAudioSourceAdded = false;
            iVideoSourceAdded = false;
            iAudioSinkAdded = false;
            iVideoSinkAdded = false;

            OSCL_TRY(error, terminal->ResetPlayFileL());
            if (error)
            {
                reset();
            }
            break;

        case PVT_COMMAND_INIT_PLAY_FILE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if (iUsePlayFileBeforeAddSource)
                {
                    if (iStartPlayBeforeUsePlayFile)
                    {
                        OSCL_TRY(error, terminal->StartPlayL());
                    }
                    else
                    {
                        OSCL_TRY(error, terminal->UsePlayFileAsSourceL(true));
                    }

                    if (error)
                    {
                        test_is_true(false);
                        reset();
                    }
                }
                else
                {
                    OSCL_TRY(error, terminal->ConnectL(iConnectOptions));
                    if (error)
                    {
                        test_is_true(false);
                        reset();
                    }
                }
            }
            else
            {
                test_is_true(false);
                reset();
            }
            break;

        case PVT_COMMAND_RESET_PLAY_FILE:
            reset();
            break;

        case PVT_COMMAND_USE_PLAY_FILE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iUsePlayDone = true;

                if (iStartPlayBeforeUsePlayFile)
                {
                    if (iUsePlayFileBeforeAddSource)
                    {
                        OSCL_TRY(error, terminal->ConnectL(iConnectOptions));
                        if (error)
                        {
                            test_is_true(false);
                            reset();
                        }
                    }
                    else
                    {
                        check_test_done();
                    }
                }
                else
                {
                    OSCL_TRY(error, terminal->StartPlayL());
                    if (error)
                    {
                        test_is_true(false);
                        shutdown();
                    }
                }
            }
            else
            {
                test_is_true(false);
                shutdown();
            }
            break;

        case PVT_COMMAND_START_PLAY:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iPlayStarted = true;

                if (iStartPlayBeforeUsePlayFile)
                {
                    OSCL_TRY(error, terminal->UsePlayFileAsSourceL(true));
                    if (error)
                    {
                        test_is_true(false);
                        shutdown();
                    }
                }
                else
                {
                    if (iUsePlayFileBeforeAddSource)
                    {
                        OSCL_TRY(error, terminal->ConnectL(iConnectOptions));
                        if (error)
                        {
                            test_is_true(false);
                            reset();
                        }
                    }
                    else
                    {
                        check_test_done();
                    }
                }
            }
            else
            {
                test_is_true(false);
                shutdown();
            }
            break;

        case PVT_COMMAND_STOP_PLAY:
            iPlayStarted = false;

            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                test_is_true(true);
                disconnect();
            }
            else
            {
                test_is_true(false);
                disconnect();
            }
            break;
    }
}

bool use_play_file_test::start_async_test()
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


void use_play_file_test::check_av_started()
{
    int error = 0;
    if (check_audio_started() && check_video_started())
    {
        if (iUsePlayFileBeforeAddSource)
        {
            check_test_done();
        }
        else
        {
            if (iStartPlayBeforeUsePlayFile)
            {
                OSCL_TRY(error, terminal->StartPlayL());
            }
            else
            {
                OSCL_TRY(error, terminal->UsePlayFileAsSourceL(true));
            }

            if (error)
            {
                test_is_true(false);
                disconnect();
            }
        }
    }

    return;
}

void use_play_file_test::check_test_done()
{
    int error = 0;

    if (check_audio_started() &&
            check_video_started() &&
            iUsePlayDone &&
            iPlayStarted)
    {
        OSCL_TRY(error, terminal->StopPlayL());
        if (error)
        {
            test_is_true(false);
            disconnect();
        }
    }
}


