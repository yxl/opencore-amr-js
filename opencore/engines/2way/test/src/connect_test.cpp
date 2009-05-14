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

#include "connect_test.h"
#include "tsc_h324m_config_interface.h"
#include "pvmp4h263encextension.h"


void connect_test::test()
{
    fprintf(fileoutput, "Start %s test, num runs %d, proxy %d.\n", iRunTimerTest ? "timer configuration and encoder extension IF" : "connect", iMaxRuns, iUseProxy);
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

    this->RemoveFromScheduler();
}


void connect_test::Run()
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

void connect_test::DoCancel()
{
}


void connect_test::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{

    switch (aEvent.GetEventType())
    {
        case PVT_INDICATION_DISCONNECT:
            iAudioSourceAdded = false;
            iVideoSourceAdded = false;
            iAudioSinkAdded = false;
            iVideoSinkAdded = false;
            break;

        case PVT_INDICATION_INTERNAL_ERROR:
            break;

        case PVT_INDICATION_INCOMING_TRACK:
            printf("Incoming Indication\n");
            break;

        default:
            break;
    }
}

void connect_test::InitSucceeded()
{
    i324mConfigInterface = iH324MConfig;
    //OSCL_TRY(error, i324mIFCommandId = terminal->QueryInterface(h324mUuid, i324mConfigInterface,NULL));
    if (i324mConfigInterface == NULL)
    {
        test_is_true(false);
        reset();
        return;

    }
    // set some timers via the interface
    H324MConfigInterface * i324Interface = (H324MConfigInterface *)i324mConfigInterface;
    if (iRunTimerTest)
    {
        i324Interface->SetTimerCounter(EH324Timer, 1, 1, 10, NULL);
        i324Interface->SetTimerCounter(EH324Counter, 4, 1, 10, NULL);
    }
    i324Interface->removeRef();
    iStackIFSet = true;
    int error;
    PVUuid mp4h263EncUuid = PVMp4H263EncExtensionUUID;
    OSCL_TRY(error, iEncoderIFCommandId = terminal->QueryInterface(mp4h263EncUuid, iMP4H263EncoderInterface, NULL));
    if (error)
    {
        test_is_true(false);
        reset();
    }
}

void connect_test::InitFailed()
{
    test_is_true(false);
    test_base::InitFailed();
}

void connect_test::EncoderIFSucceeded()
{
    PVUuid mp4h263EncUuid = PVMp4H263EncExtensionUUID;
    PVMp4H263EncExtensionInterface *ptr = (PVMp4H263EncExtensionInterface *) iMP4H263EncoderInterface;
    // Default frame rate is 5
    ptr->SetOutputFrameRate(0, 4);
    iMP4H263EncoderInterface->removeRef();
    iEncoderIFSet = true;
    connect();
}

void connect_test::EncoderIFFailed()
{
//	test_is_true(false);
    reset();
}

void connect_test::ConnectFailed()
{
    test_base::ConnectFailed();
    disconnect();
}

void connect_test::ConnectCancelled()
{
    ConnectFailed();
}

void connect_test::DisCmdSucceeded()
{
    if (++iCurrentRun < iMaxRuns)
    {
        connect();
    }
    else
    {
        reset();
    }
    destroy_sink_source();
}

void connect_test::DisCmdFailed()
{
    test_is_true(false);
    reset();
    // yep- this is how the code was.
    if (++iCurrentRun < iMaxRuns)
    {
        connect();
    }
    else
    {
        reset();
    }
    destroy_sink_source();
}






