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
/*  file name            : tsc_mt.h                                         */
/*  file contents        :                                                   */
/*  draw                 : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendment                                                                */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/

#ifndef TSC_MT_H_INCLUDED
#define TSC_MT_H_INCLUDED

#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif


#include "cpvh223multiplex.h"
#include "h245.h"

class TSC_component;

class TSC_mt
{
    public:
        TSC_mt()
        {
            iLogger = PVLogger::GetLoggerObject("3g324m.h245user");
        };
        ~TSC_mt() {};

        void SetMembers(H245* aH245, H223* aH223, TSC_component* aTSCcomponent)
        {
            iH245 = aH245;
            iH223 = aH223;
            iTSCcomponent = aTSCcomponent;
        }
        void InitVarsSession();
        void ClearVars();
        void DeleteMuxEntry(uint32 aParam);

        /* Multiplex Table */
        bool CheckMtTrf(Oscl_Vector<CodecCapabilityInfo*, OsclMemAllocator> aOutCodecList,
                        Oscl_Vector<H324ChannelParameters, PVMFTscAlloc>* aOutgoingChannelConfig);
        void MtTrfReq(OlcList& aOlcs);
        void MtTrfRps(uint32 sequenceNumber, PS_MuxDescriptor pMux);
        void MtRjtReq();
        bool MuxTableSendComplete(uint32 sn);

        uint32 SendMuxTableForLcn(TPVChannelId id);
        void ReleaseMuxTables();
        int GetPendingMtSn()
        {
            return iPendingMtSn;
        }
        // mux descriptors
        CPVMultiplexEntryDescriptor* GenerateCombinedDescriptor(uint8 entry_num,
                TPVChannelId lcn1, unsigned size, TPVChannelId lcn2);

    private:

        Oscl_Vector<int, OsclMemAllocator> iAvailableMuxEntryNumbers;
        int iOutMtSn;
        int iPendingMtSn;
        Oscl_Vector<int, OsclMemAllocator> iToBeDeletedMuxEntryNumbers;

        //TSC_capability* iTSCcapability;
        TSC_component* iTSCcomponent;

        PVLogger* iLogger;
        /* The H245 instance */
        H245* iH245;
        /* The H223 instance */
        H223* iH223;

};

#endif

