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
/*  file name            : tsc_rme.c                                         */
/*  file contents        : Terminal State Control routine                    */
/*  draw                 : '96.10.04                                         */
/*---------------------------------------------------------------------------*/
/*  amendment                                                                */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
#include    "tsc_sub.h"
#include    "h245def.h"
#include	"tscmain.h"

/*****************************************************************************/
/*  function name        : Tsc_RmeSendReq                                    */
/*  function outline     : Tsc_RmeSendReq procedure                          */
/*  function discription : Tsc_RmeSendReq( void )                            */
/*  input data           : None                                              */
/*  output data          : None                                              */
/*  draw time            : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendent career (x)   :                                                   */
/*                                                                           */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
OSCL_EXPORT_REF void TSC_324m::RmeSendReq()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::RmeSendReq\n"));
    S_ControlMsgHeader infHeader;
    Tsc_SendDataSet(&infHeader,
                    H245_PRIMITIVE,
                    E_PtvId_Rme_Send_Req,
                    0, 0, NULL, 0);
    iH245->DispatchControlMessage(&infHeader);
}


/*****************************************************************************/
/*  function name        : Tsc_RmeSendRps                                    */
/*  function outline     : Tsc_RmeSendRps procedure                          */
/*  function discription : Tsc_RmeSendRps( void )                            */
/*  input data           : None                                              */
/*  output data          : None                                              */
/*  draw time            : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendent career (x)   :                                                   */
/*                                                                           */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
void TSC_324m::RmeSendRps()
{
    S_ControlMsgHeader infHeader;
    Tsc_SendDataSet(&infHeader,
                    H245_PRIMITIVE,
                    E_PtvId_Rme_Send_Rps,
                    0, 0, NULL, 0);
    iH245->DispatchControlMessage(&infHeader);
}


/*****************************************************************************/
/*  function name        : Tsc_RmeRjtReq                                     */
/*  function outline     : Tsc_RmeRjtReq procedure                           */
/*  function discription : Tsc_RmeRjtReq( void )                             */
/*  input data           : None                                              */
/*  output data          : None                                              */
/*  draw time            : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendent career (x)   :                                                   */
/*                                                                           */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
void TSC_324m::RmeRjtReq()
{
    S_ControlMsgHeader infHeader;
    S_RmeRejectCause parameter;

    // REJECT.request primitive - Parameter[CAUSE]
    parameter.index = 0; // unspecifiedCause

    Tsc_SendDataSet(&infHeader,
                    H245_PRIMITIVE,
                    E_PtvId_Rme_Rjt_Req,
                    0,
                    0,
                    (uint8*)&parameter,
                    sizeof(S_RmeRejectCause));
    iH245->DispatchControlMessage(&infHeader);
}
