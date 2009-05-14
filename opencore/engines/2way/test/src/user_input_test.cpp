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

#include "user_input_test.h"

#define DTMF_TEST_INPUT '1'
#define DTMF_TEST_UPDATE false
#define DTMF_TEST_DURATION 20

#define ALPHANUMERIC_STRING_LENGTH 16
uint8 alphanumericTestString[ALPHANUMERIC_STRING_LENGTH] =
{
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
};

void user_input_test::test()
{
    fprintf(fileoutput, "Start user input test, is dtmf %d, proxy %d.\n", iIsDTMF, iUseProxy);
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


void user_input_test::Run()
{
    if (terminal)
    {
        if (iUseProxy)
        {
            //CPV2WayProxyFactory::DeleteTerminal(terminal);
        }
        else
        {
            CPV2WayEngineFactory::DeleteTerminal(terminal);
        }
        terminal = NULL;
    }

    scheduler->StopScheduler();
}

void user_input_test::DoCancel()
{
}

void user_input_test::H324MConfigCommandCompletedL(PVMFCmdResp& aResponse)
{
    OSCL_UNUSED_ARG(aResponse);
}

void user_input_test::H324MConfigHandleInformationalEventL(PVMFAsyncEvent& aEvent)
{
    switch (aEvent.GetEventType())
    {
        case PV_INDICATION_USER_INPUT:
            break;
        case PV_INDICATION_USER_INPUT_CAPABILITY:
            break;
    }
}

void user_input_test::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    OSCL_UNUSED_ARG(aEvent);
}

void user_input_test::RstCmdCompleted()
{
    RunIfNotReady();
}

void user_input_test::DisCmdSucceeded()
{
    printf("Finished disconnecting \n");
    if (i324mConfigInterface)
        i324mConfigInterface->removeRef();
    reset();
}

void user_input_test::DisCmdFailed()
{
    printf("Finished disconnecting \n");
    if (i324mConfigInterface)
        i324mConfigInterface->removeRef();
    reset();
}

void user_input_test::ConnectSucceeeded()
{
    i324mConfigInterface = iH324MConfig;
//		OSCL_TRY(error, i324mIFCommandId = terminal->QueryInterface(PVH324MConfigUuid, i324mConfigInterface,NULL));
    if (i324mConfigInterface == NULL)
    {
        test_is_true(false);
        disconnect();
    }
    H324MConfigInterface * i324Interface = (H324MConfigInterface *)i324mConfigInterface;
    i324Interface->SetObserver(this);
    iUserInputId = i324Interface->SendUserInput(iUserInput);
}

void user_input_test::InitFailed()
{
    test_is_true(false);
    RunIfNotReady();
}

bool user_input_test::start_async_test()
{
    if (iIsDTMF)
    {
        iUserInput = new CPVUserInputDtmf(DTMF_TEST_INPUT, DTMF_TEST_UPDATE, DTMF_TEST_DURATION);
    }
    else
    {
        iUserInput = new CPVUserInputAlphanumeric(alphanumericTestString, ALPHANUMERIC_STRING_LENGTH);
    }

    if (iUserInput == NULL)
    {
        test_is_true(false);
        return false;
    }


    return test_base::start_async_test();;
}





