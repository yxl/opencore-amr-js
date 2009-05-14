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
/*  file name            : tsc_blc.h                                         */
/*  file contents        :                                                   */
/*  draw                 : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendment                                                                */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/

#ifndef TSCBLC_H_INCLUDED
#define TSCBLC_H_INCLUDED


#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif

#ifndef H245_H_INCLUDED
#include "h245.h"
#endif

#ifndef H223TYPES_H_INCLUDED
#include "h223types.h"
#endif


class TSC_statemanager;


class TSC_blc
{
    public:
        TSC_blc(TSC_statemanager& aStateManager):  iTSCstatemanager(aStateManager)
        {
            iLogger = PVLogger::GetLoggerObject("3g324m.h245user");
        };
        ~TSC_blc() {};

        void SetH245(H245* aH245)
        {
            iH245 = aH245;
        }
        /* Bi-Directional Logical Channel Establish */
        void BlcEtbReq(TPVChannelId Lcn,
                       PS_DataType pDataTypeForward,
                       PS_H223LogicalChannelParameters pH223ParamsForward,
                       PS_DataType pDataTypeReverse,
                       PS_H223LogicalChannelParameters pH223ParamsReverse);
        void BlcEtbRps(TPVChannelId, TPVChannelId);
        TPVDirection BlcRlsReq(int32, uint32, uint32);	// add cause parameter
    private:
        H245* iH245;
        TSC_statemanager& iTSCstatemanager;
        PVLogger* iLogger;
};

#endif

