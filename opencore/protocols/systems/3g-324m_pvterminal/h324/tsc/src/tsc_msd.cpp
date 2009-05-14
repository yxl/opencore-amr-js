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
// ----------------------------------------------------------------------
//
// This Software is an original work of authorship of PacketVideo Corporation.
// Portions of the Software were developed in collaboration with NTT  DoCoMo,
// Inc. or were derived from the public domain or materials licensed from
// third parties.  Title and ownership, including all intellectual property
// rights in and to the Software shall remain with PacketVideo Corporation
// and NTT DoCoMo, Inc.
//
// -----------------------------------------------------------------------
/*****************************************************************************/
/*  file name            : tsc_msd.c                                         */
/*  file contents        : Terminal State Control routine                    */
/*  draw                 : '96.10.04                                         */
/*---------------------------------------------------------------------------*/
/*  amendment                                                                */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
#include    "tscmain.h"
#include    "tsc_statemanager.h"
#include    "tsc_constants.h"

void TSC_324m::MSDStart()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::MSDStart()\n"));
    iTSCstatemanager.MSDStart(iLoopbackMode);
    if (iLoopbackMode == PV_LOOPBACK_NONE && Msd)
    {
        Msd->DetermineRequest();
    }
}

void TSC_324m::MSDDetermineConfirm(MSDStatus type)
{
    if (type == MSD_MASTER)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC: MSD complete. decision MASTER."));
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC: MSD complete. decision SLAVE."));
    }
    // Master/Slave Determination... Complete
    iTSCstatemanager.WriteState(TSC_MSD_DECISION, (type == MSD_MASTER) ?
                                MASTER : SLAVE);
    iTSCstatemanager.WriteState(TSC_MSD, COMPLETE);
    if (iTerminalStatus == PhaseD_CSUP)
    {
        if (iTSCstatemanager.ReadState(TSC_CE_RECEIVE) == COMPLETE)
        {
            int leave_status = 0;
            OSCL_TRY(leave_status, TcsMsdComplete());
            OSCL_FIRST_CATCH_ANY(leave_status, void());
            if (leave_status != 0)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                                (0, "TSC_324m::MSDDetermineConfirm - Memory Allocation Failed."));
                SignalCsupComplete(PVMFErrNoMemory);
                return;
            }
        }
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_324m::MSDDetermineConfirm Invalid state(%d)",
                         iTerminalStatus));
    }
}

void TSC_324m::MSDDetermineIndication(MSDStatus type)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC: MSD indication received.\n"));

    iTSCstatemanager.MSDDetermineIndication(type);
}

void TSC_324m::MSDRejectIndication()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC: MSD reject indication.\n"));

    // Retry TSC_MSD several times, then give up (RAN)
    iTSCstatemanager.DecrementState(TSC_MSD_RETRYCOUNT);
    if (iTSCstatemanager.ReadState(TSC_MSD_RETRYCOUNT) > 0)
    {
        MSDStart();
    }
    else
    {
        iConnectFailReason = EPVT_FailedToNegotiate;
        SessionClose_CSUP();

        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC: MSD failed after retries.\n"));
    }
}

void TSC_324m::MSDErrorIndication(MSDErrCode errCode)
{
    OSCL_UNUSED_ARG(errCode);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC: MSD Error %d.\n", errCode));
}

