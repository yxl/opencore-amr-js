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
/*  file name            : tsc_statemanager.h                                 */
/*  file contents        :                                                   */
/*  draw                 : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendment                                                                */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/

#ifndef TSCSTATEMANAGER_H_INCLUDED
#define TSCSTATEMANAGER_H_INCLUDED


#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif

#include "h245.h"
#include "h324utils.h"
#include "tsc_constants.h"

class TSC_statemanager
{
    public:
        ~TSC_statemanager() {};

        void InitVarsSession();
        void InitVarsLocal();
        TPVStatusCode SetTerminalParam(CPVTerminalParam* params);
        // State Manager
        void StateInitialize(void);
        int32 ReadState(uint32 stateIndex);
        void WriteState(uint32 stateIndex, uint32 value);
        void StatusShow(uint32 stateIndex, uint32 value);
        void IncrementState(uint32 stateIndex);
        void DecrementState(uint32 stateIndex);
        uint32 PreReq_CeSend(void);
        uint32 PreReq_MsdSend(void);
        uint32 PreReq_MtSend(void);
        bool PreReq_LcMediaSend();
        void SkipMsd();
        bool MSDStart(TPVLoopbackMode iLoopbackMode);
        void MSDDetermineIndication(MSDStatus type);



        void SetN100(uint32 aN100)
        {
            iN100 = aN100;
        }

    private:
        // State Manager
        // Array of state values and priority levels
        int32 iTerminalState[MAX_TSC_STATES];

        unsigned iSkipMsd; // Skip msd
        PVLogger *iLogger;
        uint32 iN100; // H245

};

#endif

