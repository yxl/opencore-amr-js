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
/*  file name            : tsc_clc.h                                         */
/*  file contents        :                                                   */
/*  draw                 : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendment                                                                */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/

#ifndef TSCCLC_H_INCLUDED
#define TSCCLC_H_INCLUDED

#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif

#include "h245.h"


class TSC_clc
{
    public:

        TSC_clc() {};
        ~TSC_clc() {};

        void SetH245(H245* aH245)
        {
            iH245 = aH245;
        }

        /* Close Logical Channel */
        void ClcClsReq(uint32);
        void ClcClsRps(uint32);
        void ClcRjtReq(uint32);
    private:
        H245* iH245;
};

#endif

