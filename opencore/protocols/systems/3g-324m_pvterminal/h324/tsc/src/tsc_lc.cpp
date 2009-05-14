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

#include    "tsc_lc.h"
#include    "tsc_sub.h"     /* Sub Routine Information Header                */
#include    "tsc_statemanager.h"
#include    "tsc_constants.h"



/*****************************************************************************/
/*  function name        : Tsc_LcEtbReq                                      */
/*  function outline     : Tsc_LcEtbReq procedure                            */
/*  function discription : Tsc_LcEtbReq( LcnIndex )                          */
/*  input data           : uint32                 Open Lcn                     */
/*  output data          : None                                              */
/*  draw time            : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendent career      :                                                   */
/*                                                                           */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
void TSC_lc::LcEtbReq(TPVChannelId Lcn,
                      PS_DataType pDataType,
                      PS_H223LogicalChannelParameters pH223Params)
{
    S_ControlMsgHeader infHeader;
    S_ForwardReverseParam parameter;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_lc: LcEtbReq lcn(%d)\n", Lcn));

    // ESTABLISH.request primitive - Parameter(FORWARD_PARAM)
    parameter.forwardLogicalChannelParameters.option_of_portNumber = false;

    PS_DataType dt = Copy_DataType(pDataType);
    oscl_memcpy(&parameter.forwardLogicalChannelParameters.dataType,
                dt,
                sizeof(S_DataType));
    OSCL_DEFAULT_FREE(dt);

    // MultiplexParameters
    parameter.forwardLogicalChannelParameters.multiplexParameters.index = 1;  // 1 = h223LogicalChannelParameters

    parameter.forwardLogicalChannelParameters.multiplexParameters.h223LogicalChannelParameters =
        Copy_H223LogicalChannelParameters(pH223Params);

    // WWU_LC: start
    parameter.forwardLogicalChannelParameters.option_of_forwardLogicalChannelDependency =
        false;
    parameter.forwardLogicalChannelParameters.option_of_replacementFor = false;
    // WWU_LC: end

    parameter.option_of_reverseLogicalChannelParameters = false;

    Tsc_SendDataSet(&infHeader,
                    H245_PRIMITIVE,
                    E_PtvId_Lc_Etb_Req,
                    Lcn,
                    0,
                    (uint8*)&parameter,
                    sizeof(S_ForwardReverseParam));

    // Primitive Send
    iH245->DispatchControlMessage(&infHeader);

    iTSCstatemanager.WriteState(TSC_LC_AUDIO_SEND, STARTED);

    Delete_DataType(&parameter.forwardLogicalChannelParameters.dataType);
    Delete_H223LogicalChannelParameters(
        parameter.forwardLogicalChannelParameters.multiplexParameters.h223LogicalChannelParameters);
    OSCL_DEFAULT_FREE(
        parameter.forwardLogicalChannelParameters.multiplexParameters.h223LogicalChannelParameters);
}


/*****************************************************************************/
/*  function name        : Tsc_LcEtbRps                                      */
/*  function outline     : Tsc_LcEtbRps procedure                            */
/*  function discription : Tsc_LcEtbRps( OpenPendingLcn )                    */
/*  input data           : uint32                                              */
/*  output data          : None                                              */
/*  draw time            : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendent career      :                                                   */
/*                                                                           */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
void TSC_lc::LcEtbRps(uint32 lcn)
{
    S_ControlMsgHeader infHeader;
    S_ReverseData      parameter;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_lc: LcEtbRps lcn(%d)\n", lcn));

    parameter.option_of_reverseLogicalChannelParameters = false;
    parameter.option_of_forwardMultiplexAckParameters = false;

    Tsc_SendDataSet(&infHeader,
                    H245_PRIMITIVE,
                    E_PtvId_Lc_Etb_Rps,
                    lcn,
                    0,
                    (uint8*)&parameter,
                    sizeof(S_ReverseData));

    // Primitive Send
    iH245->DispatchControlMessage(&infHeader);
}


/*****************************************************************************/
/*  function name        : Tsc_LcRlsReq                                      */
/*  function outline     : Tsc_LcRlsReq                                      */
/*  function discription : Tsc_LcRlsReq( ReleaseType, CloseLcnIndex )                          */
/*  input data           : int32                                               */
/*                       : uint32                                              */
/*  output data          : None                                              */
/*  draw time            : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendent career      :                                                   */
/*                                                                           */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
TPVDirection TSC_lc::LcRlsReq(int32 ReleaseType, TPVChannelId CloseLcn, uint16 Cause)
{
    S_ControlMsgHeader infHeader;
    S_OlcRejectCause parameter;
    TPVDirection dir = OUTGOING;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0,
                     "TSC_lc: LcRlsReq lcn(%d), ReleaseType(%d), Cause(%d)\n",
                     CloseLcn, ReleaseType, Cause));

    if (ReleaseType == RELEASE_CLOSE)
    {
        // Close outgoing
        dir = OUTGOING;
        Tsc_SendDataSet(&infHeader,
                        H245_PRIMITIVE,
                        E_PtvId_Lc_Rls_Req,
                        CloseLcn,
                        0, NULL, 0);
    }
    else
    {
        // Reject incoming
        dir = INCOMING;
        // REJECT.request primitive - Parameter[CAUSE]
        parameter.index = (uint16)Cause;
        Tsc_SendDataSet(&infHeader,
                        H245_PRIMITIVE,
                        E_PtvId_Lc_Rls_Req,
                        CloseLcn,
                        0,
                        (uint8*)&parameter,
                        sizeof(S_OlcRejectCause));
    }

    // Primitive Send
    iH245->DispatchControlMessage(&infHeader);

    return dir;
}
