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
/*  file name            : tsc_channelcontrol.h                                         */
/*  file contents        :                                                   */
/*  draw                 : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendment                                                                */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/

#ifndef TSCCHANNELCONTROL_H_INCLUDED
#define TSCCHANNELCONTROL_H_INCLUDED

#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif

#ifndef TSC_CONSTANTS_H_INCLUDED
#include "tsc_constants.h"
#endif
#include "tsc.h"
#include "cpvh223multiplex.h"

class TSC_statemanager;
class TSC_blc;
class TSC_mt;
class TSC_lc;
class TSC_component;
class TSC_capability;
class TSC_clc;

class TSC_channelcontrol
{
    public:
        TSC_channelcontrol(OlcList& aOlcs,
                           TSC_statemanager& aTSCStateManager,
                           TSC_blc& aTSCblc,
                           TSC_mt& aTSCmt,
                           TSC_lc& aTSClc,
                           TSC_capability& aTSCcapability,
                           TSC_clc& aTSCclc,
                           TSC_component& aTSCcomponent);
        ~TSC_channelcontrol() {};

        void SetMembers(H223* aH223, TSCObserver* aTSCObserver)
        {
            iObserver = aTSCObserver;
            iH223 = aH223;
        }

        LogicalChannelInfo* GetLogicalChannelInfo(PVMFPortInterface& port);
        // utility functions
        TPVStatusCode RequestIncomingChannelClose(TPVChannelId lcn);

        bool IsEstablishedLogicalChannel(TPVDirection aDir, TPVChannelId aChannelId);
        void ReceivedFormatSpecificInfo(TPVChannelId lcn, uint8* fsi, uint32 fsi_len);

    private:
        OlcList& iOlcs;
        TSCObserver* iObserver;

        // The H223 instance
        H223* iH223;

        PVLogger* iLogger;

        TSC_statemanager& iTSCstatemanager;
        TSC_blc& iTSCblc;
        TSC_mt& iTSCmt;
        TSC_lc& iTSClc;
        TSC_capability& iTSCcapability;
        TSC_clc& iTSCclc;

        TSC_component& iTSCcomponent;
};

#endif

