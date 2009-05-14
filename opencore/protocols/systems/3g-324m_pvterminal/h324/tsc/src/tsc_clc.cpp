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
/*  file name            : tsc_clc.c                                         */
/*  file contents        : Terminal State Control routine                    */
/*  draw                 : '96.10.04                                         */
/*---------------------------------------------------------------------------*/
/*  amendment                                                                */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
#include	"tsc_clc.h"
#include    "tsc_sub.h"     /* Sub Routine Information Header                */
#include    "h245def.h"


/*****************************************************************************/
/*  function name        : Tsc_ClcClsRps                                     */
/*  function outline     : Tsc_ClcClsRps procedure                           */
/*  function discription : Tsc_ClcClsRps( void )                             */
/*  input data           : None                                              */
/*  output data          : None                                              */
/*  draw time            : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendent career (x)   :                                                   */
/*                                                                           */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
void TSC_clc::ClcClsReq(uint32 Lcn)
{
    S_ControlMsgHeader infHeader;
    Tsc_SendDataSet(&infHeader, H245_PRIMITIVE, E_PtvId_Clc_Cls_Req, Lcn, 0, NULL, 0);
    iH245->DispatchControlMessage(&infHeader);
}


/*****************************************************************************/
/*  function name        : Tsc_ClcClsRps                                     */
/*  function outline     : Tsc_ClcClsRps procedure                           */
/*  function discription : Tsc_ClcClsRps( uint32 )                             */
/*  input data           : None                                              */
/*  output data          : None                                              */
/*  draw time            : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendent career (x)   :                                                   */
/*                                                                           */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
void TSC_clc::ClcClsRps(uint32 Lcn)
{
    S_ControlMsgHeader infHeader;
    Tsc_SendDataSet(&infHeader, H245_PRIMITIVE, E_PtvId_Clc_Cls_Rps, Lcn, 0, NULL, 0);
    iH245->DispatchControlMessage(&infHeader);
}

/*****************************************************************************/
/*  function name        : Tsc_ClcRjtReq                                     */
/*  function outline     : Tsc_ClcRjtReq procedure                           */
/*  function discription : Tsc_ClcRjtReq( void )                             */
/*  input data           : None                                              */
/*  output data          : None                                              */
/*  draw time            : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendent career (x)   :                                                   */
/*                                                                           */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
void TSC_clc::ClcRjtReq(uint32 Lcn)
{
    S_ControlMsgHeader infHeader;
    S_RccRejectCause parameter;

    // REJECT.request primitive - Parameter[CAUSE]
    parameter.index = 0;  // unspecified
    Tsc_SendDataSet(&infHeader, H245_PRIMITIVE, E_PtvId_Clc_Rjt_Req, Lcn, 0,
                    (uint8*)&parameter, sizeof(S_RccRejectCause));
    iH245->DispatchControlMessage(&infHeader);
}
