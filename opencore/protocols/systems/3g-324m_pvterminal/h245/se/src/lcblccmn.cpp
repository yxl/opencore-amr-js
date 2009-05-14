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
/************************************************************************/
/*  file name       : selcblccmn.c                                      */
/*  file contents   : Uni-Directional and Bi-Directional                */
/*                  :              Logical Channel Signalling Entity    */
/*                  :                                 Common Management */
/*  draw            : '96.11.23                                         */
/*----------------------------------------------------------------------*/
/*  amendment       :                                                   */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/


/************************************************************************/
/*  Headerfile Include                                                  */
/************************************************************************/
#include	"h245pri.h"
#include	"semsgque.h"
#include	"lcentry.h"
#include	"lcblccmn.h"


/************************************************************************/
/*  function name       : Se_LcBlcT103TimerIdGet                        */
/*  function outline    : T103 Timer Id Get                             */
/*  function discription: Se_LcBlcT103TimerIdGet( PS_LCB_ENTRY )        */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*  output data         : int32 Timer Id                                  */
/*  draw time           : '96.11.11                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
int32 LCBLCCmn::T103TimerIdGet(uint32 forwardLcn)
{
    /* Return T103 Timer Id  */
    return((forwardLcn << 16) | E_TmrId_LcBlc_T103) ;
}


/************************************************************************/
/*  function name       : Se_LcBlcT103TimerStart                        */
/*  function outline    : T103 Timer Start                              */
/*  function discription: Se_LcBlcT103TimerStart( PS_LCB_ENTRY )        */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*  output data         : None                                          */
/*  draw time           : '96.11.11                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void LCBLCCmn::T103TimerStart(uint32 forwardLcn, uint32 tmrSqcNumber)
{
    RequestTimer(T103TimerIdGet(forwardLcn), tmrSqcNumber, TimerDuration) ;
}


/************************************************************************/
/*  function name       : Se_LcBlcT103TimerStop                         */
/*  function outline    : T103 Timer Stop                               */
/*  function discription: Se_LcBlcT103TimerStop( PS_LCB_ENTRY )         */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*  output data         : None                                          */
/*  draw time           : '96.11.11                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void LCBLCCmn::T103TimerStop(uint32 forwardLcn)
{
    CancelTimer(T103TimerIdGet(forwardLcn)) ;
}


/************************************************************************/
/*  function name       : Se_LcBlcMsgOpenSend                           */
/*  function outline    : Open Logical Channel Send                     */
/*  function discription: Se_LcBlcMsgOpenSend( PS_LCB_ENTRY ,           */
/*                      :                      PS_ForwardReverseParam ) */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*                      : PS_ForwardReverseParam p_ForwardReverseParam  */
/*  output data         : None                                          */
/*  draw time           : '96.11.24                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void LCBLCCmn::MsgOpenSend(uint16 forwardLcn, PS_ForwardReverseParam p_ForwardReverseParam)
{
    S_OpenLogicalChannel   openLogicalChannel ;
    S_H245Msg              h245Msg ;

    openLogicalChannel.forwardLogicalChannelNumber = forwardLcn;
    oscl_memcpy((int8*)&openLogicalChannel.forwardLogicalChannelParameters ,
                (int8*)&p_ForwardReverseParam->forwardLogicalChannelParameters ,
                sizeof(S_ForwardLogicalChannelParameters)) ;

    openLogicalChannel.option_of_reverseLogicalChannelParameters = p_ForwardReverseParam->option_of_reverseLogicalChannelParameters ;
    oscl_memcpy((int8*)&openLogicalChannel.reverseLogicalChannelParameters ,
                (int8*)&p_ForwardReverseParam->reverseLogicalChannelParameters ,
                sizeof(S_ReverseLogicalChannelParameters)) ;

    openLogicalChannel.option_of_separateStack = false ;
    oscl_memset((int8*)&openLogicalChannel.separateStack , 0 , sizeof(S_NetworkAccessParameters)) ;

    openLogicalChannel.option_of_encryptionSync = false;

    h245Msg.Type1 = H245_MSG_REQ ;
    h245Msg.Type2 = MSGTYP_LCBLC_OP ;
    h245Msg.pData = (uint8*) & openLogicalChannel ;

    MessageSend(&h245Msg) ;
}


/************************************************************************/
/*  function name       : Se_LcBlcMsgOpenAckSend                        */
/*  function outline    : Open Logical Channel Ack Send                 */
/*  function discription: Se_LcBlcMsgOpenAckSend( PS_LCB_ENTRY ,        */
/*                      :                              PS_ReverseData ) */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*                      : PS_ReverseData p_ReverseData                  */
/*  output data         : None                                          */
/*  draw time           : '96.11.25                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void LCBLCCmn::MsgOpenAckSend(uint16 forwardLcn, PS_ReverseData p_ReverseData)
{
    S_OpenLogicalChannelAck    openLogicalChannelAck ;
    S_H245Msg                  h245Msg ;

    // Logical Channel Number
    openLogicalChannelAck.forwardLogicalChannelNumber = forwardLcn;

    // ReverseLogicalChannelParameters
    openLogicalChannelAck.option_of_ackReverseLogicalChannelParameters =
        p_ReverseData->option_of_reverseLogicalChannelParameters ;

    oscl_memcpy((int8*)&openLogicalChannelAck.ackReverseLogicalChannelParameters ,
                (int8*)&p_ReverseData->reverseLogicalChannelParameters ,
                sizeof(S_AckReverseLogicalChannelParameters)) ;

    openLogicalChannelAck.option_of_separateStack = false ;

    // SeparateStack is OFF
    oscl_memset((int8*)&openLogicalChannelAck.separateStack , 0 , sizeof(S_NetworkAccessParameters)) ;

    openLogicalChannelAck.option_of_forwardMultiplexAckParameters =
        p_ReverseData->option_of_forwardMultiplexAckParameters ;
    oscl_memcpy((int8*)&openLogicalChannelAck.forwardMultiplexAckParameters ,
                (int8*)&p_ReverseData->forwardMultiplexAckParameters ,
                sizeof(S_ForwardMultiplexAckParameters)) ;

    // EncryptionSync is OFF
    openLogicalChannelAck.option_of_encryptionSync = false;	/* IMP20 */

    h245Msg.Type1 = H245_MSG_RPS ;
    h245Msg.Type2 = MSGTYP_LCBLC_OP_ACK ;
    h245Msg.pData = (uint8*) & openLogicalChannelAck ;

    MessageSend(&h245Msg) ;
}


/************************************************************************/
/*  function name       : Se_LcBlcMsgOpenCfmSend                        */
/*  function outline    : Open Logical Channel Confirm Send             */
/*                      :                       ( Bi-Directional Only ) */
/*  function discription: Se_LcBlcMsgOpenCfmSend( PS_LCB_ENTRY ,        */
/*                      :                    PS_OpenLogicalChannelAck ) */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*                      : PS_OpenLogicalChannelAck                      */
/*                      :                       p_OpenLogicalChannelAck */
/*  output data         : None                                          */
/*  draw time           : '96.11.25                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void LCBLCCmn::MsgOpenCfmSend(uint16 forwardLcn, PS_OpenLogicalChannelAck p_OpenLogicalChannelAck)
{
    S_OpenLogicalChannelConfirm    openLogicalChannelConfirm ;
    S_H245Msg                      h245Msg ;

    OSCL_UNUSED_ARG(p_OpenLogicalChannelAck);

    openLogicalChannelConfirm.forwardLogicalChannelNumber = forwardLcn;

    h245Msg.Type1 = H245_MSG_IDC ;
    h245Msg.Type2 = MSGTYP_LCBLC_OP_CFM ;
    h245Msg.pData = (uint8*) & openLogicalChannelConfirm ;

    MessageSend(&h245Msg) ;
}


/************************************************************************/
/*  function name       : Se_LcBlcMsgOpenRjtSend                        */
/*  function outline    : Open Logical Channel Reject Send              */
/*  function discription: Se_LcBlcMsgOpenRjtSend( PS_LCB_ENTRY ,        */
/*                      :                              PS_Cause_LcBlc ) */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*                      : PS_Cause_LcBlc p_Cause_LcBlc                  */
/*  output data         : None                                          */
/*  draw time           : '96.11.24                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void LCBLCCmn::MsgOpenRjtSend(uint16 forwardLcn, PS_OlcRejectCause p_Cause)
{
    S_OpenLogicalChannelReject openLogicalChannelReject ;
    S_H245Msg                  h245Msg ;

    openLogicalChannelReject.forwardLogicalChannelNumber = forwardLcn;
    oscl_memcpy((int8*)&openLogicalChannelReject.olcRejectCause ,
                (int8*)p_Cause ,
                sizeof(S_OlcRejectCause)) ;

    h245Msg.Type1 = H245_MSG_RPS ;
    h245Msg.Type2 = MSGTYP_LCBLC_OP_RJT ;
    h245Msg.pData = (uint8*) & openLogicalChannelReject ;

    MessageSend(&h245Msg) ;
}


/************************************************************************/
/*  function name       : Se_LcBlcMsgCloseSend                          */
/*  function outline    : Close Logical Channel Send                    */
/*  function discription: Se_LcBlcMsgCloseSend( PS_LCB_ENTRY ,          */
/*                      :                                   PS_Source ) */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*                      : PS_Source p_Source                            */
/*  output data         : None                                          */
/*  draw time           : '96.11.24                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void LCBLCCmn::MsgCloseSend(uint16 forwardLcn, PS_Source p_Source)
{
    S_CloseLogicalChannel  closeLogicalChannel ;
    S_H245Msg              h245Msg ;

    closeLogicalChannel.forwardLogicalChannelNumber = forwardLcn;
    oscl_memcpy((int8*)&closeLogicalChannel.source , (int8*)p_Source , sizeof(S_Source)) ;
    /* IMP20_1 */
    closeLogicalChannel.option_of_reason = 1;
    closeLogicalChannel.reason.index = 0;

    h245Msg.Type1 = H245_MSG_REQ ;
    h245Msg.Type2 = MSGTYP_LCBLC_CL ;
    h245Msg.pData = (uint8*) & closeLogicalChannel ;

    MessageSend(&h245Msg) ;
}


/************************************************************************/
/*  function name       : Se_LcBlcMsgCloseAckSend                       */
/*  function outline    : Close Logical Channel Ack Send                */
/*  function discription: Se_LcBlcMsgCloseAckSend( PS_LCB_ENTRY )       */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*  output data         : None                                          */
/*  draw time           : '96.11.24                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void LCBLCCmn::MsgCloseAckSend(uint16 forwardLcn)
{
    S_CloseLogicalChannelAck   closeLogicalChannelAck ;
    S_H245Msg                  h245Msg ;

    closeLogicalChannelAck.forwardLogicalChannelNumber = forwardLcn;

    h245Msg.Type1 = H245_MSG_RPS ;
    h245Msg.Type2 = MSGTYP_LCBLC_CL_ACK ;
    h245Msg.pData = (uint8*) & closeLogicalChannelAck ;

    MessageSend(&h245Msg) ;
}


/************************************************************************/
/*  function name       : Se_LcBlcPtvEtbIdcSend                         */
/*  function outline    : ESTABLISH.indication Send                     */
/*  function discription: Se_LcBlcPtvEtbIdcSend( PS_LCB_ENTRY ,         */
/*                      :                       PS_OpenLogicalChannel ) */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*                      : PS_OpenLogicalChannel p_OpenLogicalChannel    */
/*  output data         : None                                          */
/*  draw time           : '96.11.24                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void LCBLCCmn::PtvEtbIdcSend(uint32 forwardLcn, PS_OpenLogicalChannel p_OpenLogicalChannel, int32 reverseLcn)
{
    S_InfHeader                infHeader ;
    S_ForwardReverseParam      forwardReverseParam ;

    oscl_memcpy((int8*)&forwardReverseParam.forwardLogicalChannelParameters ,
                (int8*)&p_OpenLogicalChannel->forwardLogicalChannelParameters ,
                sizeof(S_ForwardLogicalChannelParameters)) ;
    forwardReverseParam.option_of_reverseLogicalChannelParameters =
        p_OpenLogicalChannel->option_of_reverseLogicalChannelParameters ;
    oscl_memcpy((int8*)&forwardReverseParam.reverseLogicalChannelParameters ,
                (int8*)&p_OpenLogicalChannel->reverseLogicalChannelParameters ,
                sizeof(S_ReverseLogicalChannelParameters)) ;

    infHeader.InfType = H245_PRIMITIVE ;
    if (reverseLcn == -1)  /* Uni-Directional */
    {
        infHeader.InfId = E_PtvId_Lc_Etb_Idc ;
    }
    else /* Bi-Directional */
    {
        infHeader.InfId = E_PtvId_Blc_Etb_Idc ;
    }
    infHeader.InfSupplement1 = forwardLcn;
    infHeader.InfSupplement2 = reverseLcn;
    infHeader.pParameter = (uint8*) & forwardReverseParam ;
    infHeader.Size = sizeof(S_ForwardReverseParam) ;

    PrimitiveSend(&infHeader) ;

    return ;
}


/************************************************************************/
/*  function name       : Se_LcBlcPtvEtbCfmSend                         */
/*  function outline    : ESTABLISH.confirm Send                        */
/*  function discription: Se_LcBlcPtvEtbCfmSend( PS_LCB_ENTRY ,         */
/*                      :                    PS_OpenLogicalChannelAck ) */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*                      : PS_OpenLogicalChannelAck                      */
/*                      :                       p_OpenLogicalChannelAck */
/*  output data         : None                                          */
/*  draw time           : '96.11.24                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void LCBLCCmn::PtvEtbCfmSend(uint32 forwardLcn, PS_OpenLogicalChannelAck p_OpenLogicalChannelAck, int32 reverseLcn)
{
    S_InfHeader    infHeader ;
    S_ReverseData  reverseData ;

    // ReverseLogicalChannelParameters
    reverseData.option_of_reverseLogicalChannelParameters =
        p_OpenLogicalChannelAck->option_of_ackReverseLogicalChannelParameters ;
    oscl_memcpy((int8*)&reverseData.reverseLogicalChannelParameters ,
                (int8*)&p_OpenLogicalChannelAck->ackReverseLogicalChannelParameters ,
                sizeof(S_AckReverseLogicalChannelParameters)) ;

    // ForwardMultiplexAckParameters added to 245 Library on 9/28/00 (RAN)
    reverseData.option_of_forwardMultiplexAckParameters =
        p_OpenLogicalChannelAck->option_of_forwardMultiplexAckParameters;
    oscl_memcpy((int8*)&reverseData.forwardMultiplexAckParameters ,
                (int8*)&p_OpenLogicalChannelAck->forwardMultiplexAckParameters ,
                sizeof(S_ForwardMultiplexAckParameters)) ;

    infHeader.InfType = H245_PRIMITIVE ;
    if (reverseLcn == -1)  /* Uni-Directional */
    {
        infHeader.InfId = E_PtvId_Lc_Etb_Cfm ;
    }
    else /* Bi-Directional */
    {
        infHeader.InfId = E_PtvId_Blc_Etb_Cfm ;
    }
    infHeader.InfSupplement1 = forwardLcn;
    infHeader.InfSupplement2 = reverseLcn;
    infHeader.pParameter = (uint8*) & reverseData ;
    infHeader.Size = sizeof(S_ReverseData) ;

    PrimitiveSend(&infHeader) ;

    return ;
}

void LCBLCCmn::PtvEtbCfmSend2(uint32 forwardLcn, PS_OpenLogicalChannelAck p_OpenLogicalChannelAck, int32 reverseLcn)
{
    S_InfHeader    infHeader ;
    S_ReverseData  reverseData ;

    // ReverseLogicalChannelParameters
    reverseData.option_of_reverseLogicalChannelParameters =
        p_OpenLogicalChannelAck->option_of_ackReverseLogicalChannelParameters ;
    oscl_memcpy((int8*)&reverseData.reverseLogicalChannelParameters ,
                (int8*)&p_OpenLogicalChannelAck->ackReverseLogicalChannelParameters ,
                sizeof(S_AckReverseLogicalChannelParameters)) ;

    // ForwardMultiplexAckParameters added to 245 Library on 9/28/00 (RAN)
    reverseData.option_of_forwardMultiplexAckParameters =
        p_OpenLogicalChannelAck->option_of_forwardMultiplexAckParameters ;
    oscl_memcpy((int8*)&reverseData.forwardMultiplexAckParameters ,
                (int8*)&p_OpenLogicalChannelAck->forwardMultiplexAckParameters ,
                sizeof(S_ForwardMultiplexAckParameters)) ;

    infHeader.InfType = H245_PRIMITIVE ;

    infHeader.InfId = E_PtvId_Blc_Etb_Cfm2 ;

    infHeader.InfSupplement1 = forwardLcn;
    infHeader.InfSupplement2 = reverseLcn;
    infHeader.pParameter = (uint8*) & reverseData ;
    infHeader.Size = sizeof(S_ReverseData) ;

    PrimitiveSend(&infHeader) ;

    return ;
}

/************************************************************************/
/*  function name       : Se_LcBlcPtvRlsIdcSend                         */
/*  function outline    : RELEASE.indication Send                       */
/*  function discription: Se_LcBlcPtvRlsIdcSend( PS_LCB_ENTRY ,         */
/*                      :                  PS_Source , PS_Cause_lcse2 ) */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*                      : PS_Source p_Source                            */
/*                      : PS_Cause_lcse2 p_Cause_lcse2                  */
/*  output data         : None                                          */
/*  draw time           : '96.11.24                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void LCBLCCmn::PtvRlsIdcSend(uint32 forwardLcn, PS_Source p_Source , PS_OlcRejectCause p_Cause,
                             S_InfHeader::TDirection dir, int32 reverseLcn)
{
    S_InfHeader            infHeader ;
    S_SourceCause_LcBlc    sourceCause_LcBlc ;

    if (p_Source->index == 0)	/* Source == User */
    {
        sourceCause_LcBlc.Source = Src_USER ;
    }
    else						/* Source == LCSE */
    {
        if (reverseLcn == -1)  /* Uni-Directional */
        {
            sourceCause_LcBlc.Source = Src_LCSE ;
        }
        else /* Bi-Directional */
        {
            sourceCause_LcBlc.Source = Src_BLCSE ;
        }
    }
    oscl_memcpy((int8*)&sourceCause_LcBlc.Cause , (int8*)p_Cause , sizeof(S_OlcRejectCause)) ;

    infHeader.InfType = H245_PRIMITIVE ;
    if (reverseLcn == -1)  /* Uni-Directional */
    {
        infHeader.InfId = E_PtvId_Lc_Rls_Idc ;
    }
    else /* Bi-Directional */
    {
        infHeader.InfId = E_PtvId_Blc_Rls_Idc ;
    }
    infHeader.InfSupplement1 = forwardLcn;
    infHeader.InfSupplement2 = reverseLcn;
    infHeader.pParameter = (uint8*) & sourceCause_LcBlc ;
    infHeader.Size = sizeof(S_SourceCause_LcBlc) ;
    infHeader.Dir = dir;

    PrimitiveSend(&infHeader) ;

    return ;
}


/************************************************************************/
/*  function name       : Se_LcBlcPtvRlsCfmSend                         */
/*  function outline    : RELEASE.confirm Send                          */
/*  function discription: Se_LcBlcPtvRlsCfmSend( PS_LCB_ENTRY )         */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*  output data         : None                                          */
/*  draw time           : '96.11.24                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void LCBLCCmn::PtvRlsCfmSend(uint32 forwardLcn, int32 reverseLcn)
{
    S_InfHeader    infHeader ;

    infHeader.InfType = H245_PRIMITIVE ;
    if (reverseLcn == -1)  /* Uni-Directional */
    {
        infHeader.InfId = E_PtvId_Lc_Rls_Cfm ;
    }
    else /* Bi-Directional */
    {
        infHeader.InfId = E_PtvId_Blc_Rls_Cfm ;
    }
    infHeader.InfSupplement1 = forwardLcn;
    infHeader.InfSupplement2 = reverseLcn;
    infHeader.pParameter = NULL ;
    infHeader.Size = 0 ;

    PrimitiveSend(&infHeader) ;

    return ;
}


/************************************************************************/
/*  function name       : Se_LcBlcPtvErrIdcSend                         */
/*  function outline    : ERROR.indication Send                         */
/*  function discription: Se_LcBlcPtvErrIdcSend( PS_LCB_ENTRY ,         */
/*                      :                                ENUM_ErrCode ) */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*                      : ENUM_ErrCode ErrCode                          */
/*  output data         : None                                          */
/*  draw time           : '96.11.24                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void LCBLCCmn::PtvErrIdcSend(uint32 forwardLcn, ENUM_ErrCode ErrCode, int32 reverseLcn)
{
    S_InfHeader    infHeader ;
    S_ErrCode      errCode ;

    errCode.ErrCode = ErrCode ;

    infHeader.InfType = H245_PRIMITIVE ;
    if (reverseLcn == -1)  /* Uni-Directional */
    {
        infHeader.InfId = E_PtvId_Lc_Err_Idc ;
    }
    else /* Bi-Directional */
    {
        infHeader.InfId = E_PtvId_Blc_Err_Idc ;
    }
    infHeader.InfSupplement1 = forwardLcn;
    infHeader.InfSupplement2 = reverseLcn;
    infHeader.pParameter = (uint8*) & errCode ;
    infHeader.Size = sizeof(S_ErrCode) ;

    PrimitiveSend(&infHeader) ;

    return ;
}
