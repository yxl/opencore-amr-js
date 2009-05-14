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

#include "tsc_blc.h"         // Bi-directional Logical Channel Information Header 
#include "tsc_sub.h"         // Sub Routine Information Header            
#include "oscl_mem.h"
#include "h245def.h"
#include "h245_copier.h"
#include "h223types.h"
#include "tsc_constants.h"
#include "tsc_statemanager.h"



/*****************************************************************************/
/*  function name        : Tsc_BlcEtbReq                                     */
/*  function outline     : Tsc_BlcEtbReq procedure                           */
/*  function discription : Tsc_BlcEtbReq( LcnIndex )                          */
/*  input data           : uint32                 Open Lcn                     */
/*  output data          : None                                              */
/*  draw time            : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendent career (x)      :                                                   */
/*                                                                           */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
void TSC_blc::BlcEtbReq(TPVChannelId Lcn,
                        PS_DataType pDataTypeForward,
                        PS_H223LogicalChannelParameters pH223ParamsForward,
                        PS_DataType pDataTypeReverse,
                        PS_H223LogicalChannelParameters pH223ParamsReverse)
{
    S_ControlMsgHeader         infHeader;
    S_ForwardReverseParam      parameter;
    S_DataType  dataTypeNull;
    dataTypeNull.index = 1;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_blc::BlcEtbReq lcn(%d), %x, %x, %x, %x", Lcn, pDataTypeForward, pH223ParamsForward, pDataTypeReverse, pH223ParamsReverse));

    if (pDataTypeReverse == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0,
                         "TSC_blc::BlcEtbReq Reverse DataType == null"));
        pDataTypeReverse = &dataTypeNull;
    }

    if (pH223ParamsReverse == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0,
                         "TSC_blc::BlcEtbReq Reverse H223Params == null"));
        pH223ParamsReverse = pH223ParamsForward;
    }

    PS_DataType dt = Copy_DataType(pDataTypeForward);
    oscl_memcpy(&parameter.forwardLogicalChannelParameters.dataType,
                dt,
                sizeof(S_DataType));
    OSCL_DEFAULT_FREE(dt);

    // Buffer Allocate
    parameter.forwardLogicalChannelParameters.option_of_portNumber = false;

    // MultiplexParameters
    parameter.forwardLogicalChannelParameters.multiplexParameters.index = 1;  // h223
    parameter.forwardLogicalChannelParameters.multiplexParameters.h223LogicalChannelParameters =
        Copy_H223LogicalChannelParameters(pH223ParamsForward);

    // ForwardLogicalChannelParameters
    parameter.forwardLogicalChannelParameters.option_of_forwardLogicalChannelDependency = false;
    parameter.forwardLogicalChannelParameters.option_of_replacementFor = false;

    // ===========================================================
    // =========== reverseLogicalChannelParameters ===============
    // ===========================================================

    parameter.option_of_reverseLogicalChannelParameters = true;

    dt = Copy_DataType(pDataTypeReverse);
    oscl_memcpy(&parameter.reverseLogicalChannelParameters.dataType,
                dt,
                sizeof(S_DataType));
    OSCL_DEFAULT_FREE(dt);

    parameter.reverseLogicalChannelParameters.option_of_rlcMultiplexParameters = true;
    // MultiplexParameters
    parameter.reverseLogicalChannelParameters.rlcMultiplexParameters.index = 0;  // h223
    parameter.reverseLogicalChannelParameters.rlcMultiplexParameters.h223LogicalChannelParameters =
        Copy_H223LogicalChannelParameters(pH223ParamsReverse);

    // rEVERSELogicalChannelParameters
    parameter.reverseLogicalChannelParameters.option_of_reverseLogicalChannelDependency = false;
    parameter.reverseLogicalChannelParameters.option_of_replacementFor = false;

    Tsc_SendDataSet(&infHeader,
                    H245_PRIMITIVE,
                    E_PtvId_Blc_Etb_Req,
                    Lcn,
                    0,
                    (uint8*)&parameter,
                    sizeof(S_ForwardReverseParam));

    // Primitive Send
    iH245->DispatchControlMessage(&infHeader);

    iTSCstatemanager.WriteState(TSC_LC_VIDEO_SEND, STARTED);

    Delete_DataType(&parameter.forwardLogicalChannelParameters.dataType);
    Delete_DataType(&parameter.reverseLogicalChannelParameters.dataType);
    Delete_H223LogicalChannelParameters(
        parameter.forwardLogicalChannelParameters.multiplexParameters.h223LogicalChannelParameters);
    OSCL_DEFAULT_FREE(
        parameter.forwardLogicalChannelParameters.multiplexParameters.h223LogicalChannelParameters);
    Delete_H223LogicalChannelParameters(
        parameter.reverseLogicalChannelParameters.rlcMultiplexParameters.h223LogicalChannelParameters);
    OSCL_DEFAULT_FREE(
        parameter.reverseLogicalChannelParameters.rlcMultiplexParameters.h223LogicalChannelParameters);
}

/*****************************************************************************/
/*  function name        : Tsc_BlcEtbRps                                     */
/*  function outline     : Tsc_BlcEtbRps procedure                           */
/*  function discription : Tsc_BlcEtbRps( OpenPendingLcn )                   */
/*  input data           : uint32                                              */
/*  output data          : None                                              */
/*  draw time            : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendent career (x)      :                                                   */
/*                                                                           */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
void TSC_blc::BlcEtbRps(TPVChannelId incoming_lcn, TPVChannelId outgoing_lcn)
{
    S_ControlMsgHeader	infHeader;
    S_ReverseData		parameter;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0,
                     "TSC_blc::BlcEtbRps incoming(%d), outgoing(%d)\n", incoming_lcn, outgoing_lcn));

    // Buffer Allocate
    oscl_memset(&parameter, 0, sizeof(S_ReverseData));

    // ESTABLISH.response primitive - Parameter(REVERSE_DATA)
    parameter.option_of_reverseLogicalChannelParameters = true;
    parameter.reverseLogicalChannelParameters.reverseLogicalChannelNumber =
        (uint16)incoming_lcn; // WWU_BLC: change 0->3
    parameter.reverseLogicalChannelParameters.option_of_portNumber = false;
    parameter.reverseLogicalChannelParameters.option_of_ackMultiplexParameters = false;
    parameter.reverseLogicalChannelParameters.option_of_replacementFor = false;

    Tsc_SendDataSet(&infHeader,
                    H245_PRIMITIVE,
                    E_PtvId_Blc_Etb_Rps,
                    incoming_lcn,
                    outgoing_lcn,
                    (uint8*)&parameter,
                    sizeof(S_ReverseData));

    // Primitive Send
    iH245->DispatchControlMessage(&infHeader);
}


/*****************************************************************************/
/*  function name        : Tsc_BlcRlsReq                                     */
/*  function outline     : Tsc_BlcRlsReq                                     */
/*  function discription : Tsc_BlcRlsReq( ReleaseType, CloseLcn )            */
/*  input data           : int32                                               */
/*                       : uint32                                              */
/*  output data          : None                                              */
/*  draw time            : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendent career (x)      :                                                   */
/*                                                                           */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
TPVDirection TSC_blc::BlcRlsReq(int32 ReleaseType,
                                uint32 CloseLcn,
                                uint32 Cause)	// WWU_VOAL2: add Cause
{
    S_ControlMsgHeader	infHeader;
    S_OlcRejectCause	parameter;
    TPVDirection dir = OUTGOING;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0,
                     "TSC_blc::BlcRlsReq lcn(%d), Release type(%d), Cause(%d)\n",
                     CloseLcn, ReleaseType, Cause));

    if (ReleaseType == RELEASE_CLOSE)
    {
        // Close outgoing
        dir = OUTGOING;
        Tsc_SendDataSet(&infHeader,
                        H245_PRIMITIVE,
                        E_PtvId_Blc_Rls_Req,
                        CloseLcn,
                        0,
                        NULL,
                        0);
    }
    else
    {
        // Reject incoming
        dir = INCOMING;
        // Buffer Allocate
        parameter.index = (uint16)Cause;
        Tsc_SendDataSet(&infHeader,
                        H245_PRIMITIVE,
                        E_PtvId_Blc_Rls_Req,
                        CloseLcn,
                        0,
                        (uint8*)&parameter,
                        sizeof(S_OlcRejectCause));
    }

    // Primitive Send
    iH245->DispatchControlMessage(&infHeader);
    return dir;
}

