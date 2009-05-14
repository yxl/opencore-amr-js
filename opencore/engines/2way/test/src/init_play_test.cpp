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
#include "init_play_test.h"



void init_play_test::test()
{
    int error = 0;
    char *filename = new char[iFilename.get_size() + 1];
    if ((filename == NULL) || (oscl_UnicodeToUTF8(iFilename.get_cstr(), iFilename.get_size(), filename, iFilename.get_size() + 1) == 0))
    {
        OSCL_LEAVE(-1);
    }
    fprintf(fileoutput, "Start init play test, num runs %d, proxy %d, file %s\n", iMaxRuns, iUseProxy, filename);

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


void init_play_test::Run()
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

void init_play_test::DoCancel()
{
}


void init_play_test::HandleInformationalEventL(const CPVCmnAsyncInfoEvent& aEvent)
{
}

void init_play_test::CommandCompletedL(const CPVCmnCmdResp& aResponse)
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
                else
                {
                }
            }
            else
            {
                test_is_true(false);
                reset();
            }
            break;

        case PVT_COMMAND_RESET:
            RunIfNotReady();
            break;

        case PVT_COMMAND_INIT_PLAY_FILE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                OSCL_TRY(error, terminal->ResetPlayFileL());
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
                reset();
            }
            break;

        case PVT_COMMAND_RESET_PLAY_FILE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iCurrentRun++;
                if (iCurrentRun < iMaxRuns)
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
                    test_is_true(true);

                    OSCL_TRY(error, terminal->ResetL());
                    if (error)
                    {
                        test_is_true(false);
                        RunIfNotReady();
                    }
                    else
                    {
                    }
                }
            }
            else
            {
                test_is_true(false);
                RunIfNotReady();
            }
            break;
    }
}

bool init_play_test::start_async_test()
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





