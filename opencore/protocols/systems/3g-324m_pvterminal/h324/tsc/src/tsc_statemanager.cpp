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
// ============================================================
// FILE: TSC_statemanager.c
//
// DESCRIPTION: Terminal state management and initiation
//   of TSC events.
//
// Written by Ralph Neff, PacketVideo, 3/16/2000
// (c) 2000 PacketVideo Corp.
// ============================================================

#define TSC_statemanager_SRC
#include "tsc_statemanager.h"
#include "tsc_constants.h"

void TSC_statemanager::InitVarsSession()
{
    iLogger = PVLogger::GetLoggerObject("3g324m");
}

void TSC_statemanager::InitVarsLocal()
{
    iN100 = N100_DEFAULT;
    iSkipMsd = false;
}

TPVStatusCode TSC_statemanager::SetTerminalParam(CPVTerminalParam* params)
{
    CPVH324MParam* h324params = (CPVH324MParam*)params;
    iSkipMsd = h324params->iSkipMsd;
    return EPVT_Success;
}

void TSC_statemanager::MSDDetermineIndication(MSDStatus type)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_statemanager: MSD indication received.\n"));

    //Master slave determination(R)...Start
    if (ReadState(TSC_MSD_RECEIVE) != STARTED)
    {
        WriteState(TSC_MSD_RECEIVE, COMPLETE);
    }
    WriteState(TSC_MSD_DECISION, (type == MSD_MASTER) ? MASTER : SLAVE);
}

bool TSC_statemanager::MSDStart(TPVLoopbackMode iLoopbackMode)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_statemanager: MSD Start indication\n"));

    WriteState(TSC_MSD, STARTED);
    WriteState(TSC_MSD_RECEIVE, STARTED);
    if (iLoopbackMode != PV_LOOPBACK_NONE)
    {
        SkipMsd(); // Skip MSD for loopback
        return false;
    }
    return true;
}


// *********************************************************
// =========================================================
//                STATE AND EVENT PRIORITY I/O
// =========================================================
// *********************************************************

// =========================================================
// Tsc_StateInitialize()
//
// Set all terminal states to initial values.
// =========================================================
void TSC_statemanager::StateInitialize(void)
{
    // Exceptions
    iTerminalState[TSC_MSD_RETRYCOUNT] = iN100;

}

// =========================================================
// ReadState()
//
// Read a terminal state variable.
// =========================================================
int32 TSC_statemanager::ReadState(uint32 stateIndex)
{
    return iTerminalState[stateIndex];
}

// =========================================================
// WriteState()
//
// Write to a terminal state variable.
// =========================================================
void TSC_statemanager::WriteState(uint32 stateIndex, uint32 value)
{
    StatusShow(stateIndex, value);
    iTerminalState[stateIndex] = value;
}

// =========================================================
// Tsc_StatusShow()
//
// Display the new value of a terminal state.
// =========================================================
void TSC_statemanager::StatusShow(uint32 stateIndex, uint32 value)
{
    OSCL_UNUSED_ARG(stateIndex);
    OSCL_UNUSED_ARG(value);
    return;
}

// =========================================================
// Tsc_IncrementState()
//
// Increment a terminal state variable.
// =========================================================
void TSC_statemanager::IncrementState(uint32 stateIndex)
{
    ++iTerminalState[stateIndex];
}

// =========================================================
// Tsc_DecrementState()
//
// Decrement a terminal state variable.
// =========================================================
void TSC_statemanager::DecrementState(uint32 stateIndex)
{
    --iTerminalState[stateIndex];
}


// *********************************************************
// =========================================================
//                   PRE-REQUISITE CHECKS
// =========================================================
// *********************************************************

// =========================================================
// Tsc_PreReq_CeSend()
//
// Check pre-requisites for initiating CE Send procedure.
//
// SPECIFICS: CE Send may be started at any time.
// =========================================================
uint32 TSC_statemanager::PreReq_CeSend()
{
    if (ReadState(TSC_CE_SEND) == NOT_STARTED)
    {
        return true;
    }
    else
    {
        return false;
    }
}

// =========================================================
// Tsc_PreReq_MsdSend()
//
// Check pre-requisites for initiating the MSD procedure.
//
// SPECIFICS: MSD may be started any time after completion of
//            the CE Send procedure.
// =========================================================
uint32 TSC_statemanager::PreReq_MsdSend()
{
    if ((ReadState(TSC_MSD) == NOT_STARTED) &&
            (ReadState(TSC_CE_SEND) == COMPLETE))
    {
        return true;
    }
    else
    {
        return false;
    }
}

// =========================================================
// PreReq_MtSend()
//
// Check pre-requisites for initiating the MT Send procedure.
//
// SPECIFICS: MT Send may be started at any time after
//   the remote terminal's capabilities have been received
// =========================================================
uint32 TSC_statemanager::PreReq_MtSend()
{
    if ((ReadState(TSC_MT_SEND) == NOT_STARTED) &&
            (ReadState(TSC_CE_RECEIVE) == COMPLETE))
    {
        return true;
    }
    else
    {
        return false;
    }
}

// =========================================================
// PreReq_LcAudioSend()
//
// Check pre-requisites for initiating LC Audio Send.
//
// SPECIFICS: LC Audio Send may be started at any time after
//   the remote terminal's capabilities have been received.
//   We additionally require MSD to be complete.
// =========================================================
bool TSC_statemanager::PreReq_LcMediaSend()
{
    if ((ReadState(TSC_CE_RECEIVE) == COMPLETE)  &&
            (ReadState(TSC_MSD) == COMPLETE))
    {
        return true;
    }
    else
    {
        return false;
    }
}

void TSC_statemanager::SkipMsd()
{
    // Make us a slave so BLC can succeed in loopback mode
    WriteState(TSC_MSD_RECEIVE, COMPLETE);
    WriteState(TSC_MSD_DECISION, SLAVE);
    WriteState(TSC_MSD, COMPLETE);
}



