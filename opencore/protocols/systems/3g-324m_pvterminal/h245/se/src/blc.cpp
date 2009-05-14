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
/*  file name       : seblc.c                                           */
/*  file contents   : Bi-Directional Logical Channel Signalling Entity  */
/*                  :                                        Management */
/*  draw            : '96.11.26                                         */
/*----------------------------------------------------------------------*/
/*  amendment       :                                                   */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/


/************************************************************************/
/*  Headerfile Include                                                  */
/************************************************************************/
#include "h245inf.h"
#include "msd.h"
#include "blc.h"
#include "lcblccmn.h"

/************************************************************************/
/*  function name       : Se_Blc_0300_0000                              */
/*  function outline    : Event     ESTABLISH.request                   */
/*                      :                            ( Bi-Directional ) */
/*                      : Status    Outgoing Released                   */
/*  function discription: Se_Blc_0300_0000( PS_LCB_ENTRY ,              */
/*                      :                      PS_ForwardReverseParam ) */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*                      : PS_ForwardReverseParam p_ForwardReverseParam  */
/*  output data         : None                                          */
/*  draw time           : '96.11.25                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void BLC::_0300_0000(LCEntry *p_LcbEntry , PS_ForwardReverseParam p_ForwardReverseParam)
{
    uint32 forwardLcn = p_LcbEntry->FwdLcnRead();
    p_LcbEntry->StatusWrite(LCBLC_OUTGOING_AWTING_ETBMNT) ;
    MsgOpenSend((uint16) forwardLcn, p_ForwardReverseParam) ;
    T103TimerStart(forwardLcn, p_LcbEntry->TmrSqcNumberInc()) ;
}


/************************************************************************/
/*  function name       : Se_Blc_0300_0030                              */
/*  function outline    : Event     ESTABLISH.request                   */
/*                      :                            ( Bi-Directional ) */
/*                      : Status    Outgoing Awaiting Release           */
/*  function discription: Se_Blc_0300_0030( PS_LCB_ENTRY ,              */
/*                      :                      PS_ForwardReverseParam ) */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*                      : PS_ForwardReverseParam p_ForwardReverseParam  */
/*  output data         : None                                          */
/*  draw time           : '96.11.25                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void BLC::_0300_0030(LCEntry *p_LcbEntry , PS_ForwardReverseParam p_ForwardReverseParam)
{
    uint32 forwardLcn = p_LcbEntry->FwdLcnRead();
    T103TimerStop(forwardLcn) ;
    p_LcbEntry->StatusWrite(LCBLC_OUTGOING_AWTING_ETBMNT) ;
    MsgOpenSend((uint16) forwardLcn, p_ForwardReverseParam) ;
    T103TimerStart(forwardLcn, p_LcbEntry->TmrSqcNumberInc()) ;
}


/************************************************************************/
/*  function name       : Se_Blc_0301_0010                              */
/*  function outline    : Event     OpenLogicalChannel                  */
/*                      :                            ( Bi-Directional ) */
/*                      : Status    Outgoing Awaiting Establishment     */
/*  function discription: Se_Blc_0301_0010( PS_LCB_ENTRY ,              */
/*                      :                       PS_OpenLogicalChannel ) */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*                      : PS_OpenLogicalChannel p_OpenLogicalChannel    */
/*  output data         : None                                          */
/*  draw time           : '96.11.25                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void BLC::_0301_0010(LCEntry *p_LcbEntry , PS_OpenLogicalChannel p_OpenLogicalChannel)
{
    if (MyMSD->GetStatus() != MSD_MASTER)
    {
        uint32 forwardLcn = p_LcbEntry->FwdLcnRead();
        uint32 reverseLcn = p_LcbEntry->RvsLcnRead();
        T103TimerStop(forwardLcn) ;
        p_LcbEntry->StatusWrite(LCBLC_INCOMING_AWTING_ETBMNT) ;
        PtvEtbIdcSend(forwardLcn, p_OpenLogicalChannel, reverseLcn) ;
    }
}


/************************************************************************/
/*  function name       : Se_Blc_0303_0010                              */
/*  function outline    : Event     RELEASE.request                     */
/*                      :                            ( Bi-Directional ) */
/*                      : Status    Outgoing Awaiting Establishment     */
/*  function discription: Se_Blc_0303_0010( PS_LCB_ENTRY ,              */
/*                      :                              PS_Cause_LcBlc ) */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*                      : PS_Cause_LcBlc p_Cause_LcBlc                  */
/*  output data         : None                                          */
/*  draw time           : '96.11.25                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void BLC::_0303_0010(LCEntry *p_LcbEntry , PS_OlcRejectCause p_Cause)
{
    S_Source    source ;
    OSCL_UNUSED_ARG(p_Cause);

    uint32 forwardLcn = p_LcbEntry->FwdLcnRead();

    T103TimerStop(forwardLcn) ;

    source.index = 0 ;  /* user */

    p_LcbEntry->StatusWrite(LCBLC_OUTGOING_AWTING_RLS) ;
    MsgCloseSend((uint16) forwardLcn, &source) ;
    T103TimerStart(forwardLcn, p_LcbEntry->TmrSqcNumberInc()) ;
}


/************************************************************************/
/*  function name       : Se_Blc_0303_0020                              */
/*  function outline    : Event     RELEASE.request                     */
/*                      :                            ( Bi-Directional ) */
/*                      : Status    Outgoing Established                */
/*  function discription: Se_Blc_0303_0020( PS_LCB_ENTRY ,              */
/*                      :                              PS_Cause_LcBlc ) */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*                      : PS_Cause_LcBlc p_Cause_LcBlc                  */
/*  output data         : None                                          */
/*  draw time           : '96.11.25                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void BLC::_0303_0020(LCEntry *p_LcbEntry , PS_OlcRejectCause p_Cause)
{
    S_Source    source ;
    OSCL_UNUSED_ARG(p_Cause);

    source.index = 0 ;  /* user */

    uint32 forwardLcn = p_LcbEntry->FwdLcnRead();

    p_LcbEntry->StatusWrite(LCBLC_OUTGOING_AWTING_RLS) ;
    MsgCloseSend((uint16) forwardLcn, &source) ;
    T103TimerStart(forwardLcn, p_LcbEntry->TmrSqcNumberInc()) ;
}


/************************************************************************/
/*  function name       : Se_Blc_0304_0000                              */
/*  function outline    : Event     OpenLogicalChannelAck               */
/*                      :                            ( Bi-Directional ) */
/*                      : Status    Outgoing Released                   */
/*  function discription: Se_Blc_0304_0000( PS_LCB_ENTRY ,              */
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
void BLC::_0304_0000(LCEntry *p_LcbEntry , PS_OpenLogicalChannelAck p_OpenLogicalChannelAck)
{
    OSCL_UNUSED_ARG(p_OpenLogicalChannelAck);

    uint32 forwardLcn = p_LcbEntry->FwdLcnRead();
    uint32 reverseLcn = p_LcbEntry->RvsLcnRead();
    PtvErrIdcSend(forwardLcn, ErrCode_Blc_A, reverseLcn) ;
}


/************************************************************************/
/*  function name       : Se_Blc_0304_0010                              */
/*  function outline    : Event     OpenLogicalChannelAck               */
/*                      :                            ( Bi-Directional ) */
/*                      : Status    Outgoing Awaiting Establishment     */
/*  function discription: Se_Blc_0304_0010( PS_LCB_ENTRY ,              */
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
void BLC::_0304_0010(LCEntry *p_LcbEntry , PS_OpenLogicalChannelAck p_OpenLogicalChannelAck)
{
    uint32 forwardLcn = p_LcbEntry->FwdLcnRead();
    T103TimerStop(forwardLcn) ;
    p_LcbEntry->StatusWrite(LCBLC_OUTGOING_ETBED) ;
    p_LcbEntry->RvsLcnWrite(p_OpenLogicalChannelAck->ackReverseLogicalChannelParameters.reverseLogicalChannelNumber) ;
    uint32 reverseLcn = p_LcbEntry->RvsLcnRead();
    PtvEtbCfmSend(forwardLcn, p_OpenLogicalChannelAck, reverseLcn) ;
    MsgOpenCfmSend((uint16) forwardLcn, p_OpenLogicalChannelAck) ;
}


/************************************************************************/
/*  function name       : Se_Blc_0306_0000                              */
/*  function outline    : Event     OpenLogicalChannelReject            */
/*                      :                            ( Bi-Directional ) */
/*                      : Status    Outgoing Released                   */
/*  function discription: Se_Blc_0306_0000 ( PS_LCB_ENTRY ,             */
/*                      :                 PS_OpenLogicalChannelReject ) */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*                      : PS_OpenLogicalChannelReject                   */
/*                      :                    p_OpenLogicalChannelReject */
/*  output data         : None                                          */
/*  draw time           : '96.11.25                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void BLC::_0306_0000(LCEntry *p_LcbEntry , PS_OpenLogicalChannelReject p_OpenLogicalChannelReject)
{
    OSCL_UNUSED_ARG(p_OpenLogicalChannelReject);

    uint32 forwardLcn = p_LcbEntry->FwdLcnRead();
    uint32 reverseLcn = p_LcbEntry->RvsLcnRead();
    PtvErrIdcSend(forwardLcn, ErrCode_Blc_B, reverseLcn) ;
}


/************************************************************************/
/*  function name       : Se_Blc_0306_0010                              */
/*  function outline    : Event     OpenLogicalChannelReject            */
/*                      :                            ( Bi-Directional ) */
/*                      : Status    Outgoing Awaiting Establishment     */
/*  function discription: Se_Blc_0306_0010 ( PS_LCB_ENTRY ,             */
/*                      :                 PS_OpenLogicalChannelReject ) */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*                      : PS_OpenLogicalChannelReject                   */
/*                      :                    p_OpenLogicalChannelReject */
/*  output data         : None                                          */
/*  draw time           : '96.11.25                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void BLC::_0306_0010(LCEntry *p_LcbEntry , PS_OpenLogicalChannelReject p_OpenLogicalChannelReject)
{
    S_Source    source ;

    uint32 forwardLcn = p_LcbEntry->FwdLcnRead();
    uint32 reverseLcn = p_LcbEntry->RvsLcnRead();

    T103TimerStop(forwardLcn) ;

    source.index = 0 ;  /* user */

    p_LcbEntry->StatusWrite(LCBLC_OUTGOING_RLSED) ;
    PtvRlsIdcSend(forwardLcn, &source , &p_OpenLogicalChannelReject->olcRejectCause, S_InfHeader::OUTGOING, reverseLcn) ;
}


/************************************************************************/
/*  function name       : Se_Blc_0306_0020                              */
/*  function outline    : Event     OpenLogicalChannelReject            */
/*                      :                            ( Bi-Directional ) */
/*                      : Status    Outgoing Established                */
/*  function discription: Se_Blc_0306_0020 ( PS_LCB_ENTRY ,             */
/*                      :                 PS_OpenLogicalChannelReject ) */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*                      : PS_OpenLogicalChannelReject                   */
/*                      :                    p_OpenLogicalChannelReject */
/*  output data         : None                                          */
/*  draw time           : '96.11.25                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void BLC::_0306_0020(LCEntry *p_LcbEntry , PS_OpenLogicalChannelReject p_OpenLogicalChannelReject)
{
    S_Source    source ;

    uint32 forwardLcn = p_LcbEntry->FwdLcnRead();
    uint32 reverseLcn = p_LcbEntry->RvsLcnRead();

    p_LcbEntry->StatusWrite(LCBLC_OUTGOING_RLSED) ;

    PtvErrIdcSend(forwardLcn, ErrCode_Blc_B, reverseLcn) ;

    source.index = 1 ;  /* lcse */

    PtvRlsIdcSend(forwardLcn, &source , &p_OpenLogicalChannelReject->olcRejectCause, S_InfHeader::OUTGOING, reverseLcn) ;
}


/************************************************************************/
/*  function name       : Se_Blc_0306_0030                              */
/*  function outline    : Event     OpenLogicalChannelReject            */
/*                      :                            ( Bi-Directional ) */
/*                      : Status    Outgoing Awaiting Release           */
/*  function discription: Se_Blc_0306_0030 ( PS_LCB_ENTRY ,             */
/*                      :                 PS_OpenLogicalChannelReject ) */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*                      : PS_OpenLogicalChannelReject                   */
/*                      :                    p_OpenLogicalChannelReject */
/*  output data         : None                                          */
/*  draw time           : '96.11.25                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void BLC::_0306_0030(LCEntry *p_LcbEntry , PS_OpenLogicalChannelReject p_OpenLogicalChannelReject)
{
    OSCL_UNUSED_ARG(p_OpenLogicalChannelReject);
    uint32 forwardLcn = p_LcbEntry->FwdLcnRead();
    uint32 reverseLcn = p_LcbEntry->RvsLcnRead();
    T103TimerStop(forwardLcn) ;
    p_LcbEntry->StatusWrite(LCBLC_OUTGOING_RLSED) ;
    PtvRlsCfmSend(forwardLcn, reverseLcn) ;
}


/************************************************************************/
/*  function name       : Se_Blc_0307_0010                              */
/*  function outline    : Event     Timer T103 Timeout                  */
/*                      :                            ( Bi-Directional ) */
/*                      : Status    Outgoing Awaiting Establishment     */
/*  function discription: Se_Blc_0307_0010( PS_LCB_ENTRY , int32 )        */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*                      : int32 TmrSqcNumber                              */
/*  output data         : None                                          */
/*  draw time           : '96.11.25                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void BLC::_0307_0010(LCEntry *p_LcbEntry , int32 TmrSqcNumber)
{
    S_Source           source ;
    S_OlcRejectCause   cause ;

    if (TmrSqcNumber == p_LcbEntry->TmrSqcNumberRead())
    {
        uint32 forwardLcn = p_LcbEntry->FwdLcnRead();
        uint32 reverseLcn = p_LcbEntry->RvsLcnRead();

        p_LcbEntry->StatusWrite(LCBLC_OUTGOING_RLSED) ;

        PtvErrIdcSend(forwardLcn, ErrCode_Blc_D, reverseLcn) ;

        source.index = 1 ;  /* lcse */
        MsgCloseSend((uint16) forwardLcn, &source) ;

        oscl_memset((void*)&cause , 0 , sizeof(S_OlcRejectCause)) ;
        PtvRlsIdcSend(forwardLcn, &source , &cause, S_InfHeader::OUTGOING, reverseLcn) ;
    }
}


/************************************************************************/
/*  function name       : Se_Blc_0307_0030                              */
/*  function outline    : Event     Timer T103 Timeout                  */
/*                      :                            ( Bi-Directional ) */
/*                      : Status    Outgoing Awaiting Release           */
/*  function discription: Se_Blc_0307_0030( PS_LCB_ENTRY , int32 )        */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*                      : int32 TmrSqcNumber                              */
/*  output data         : None                                          */
/*  draw time           : '96.11.25                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void BLC::_0307_0030(LCEntry *p_LcbEntry , int32 TmrSqcNumber)
{
    if (TmrSqcNumber == p_LcbEntry->TmrSqcNumberRead())
    {
        uint32 forwardLcn = p_LcbEntry->FwdLcnRead();
        uint32 reverseLcn = p_LcbEntry->RvsLcnRead();
        p_LcbEntry->StatusWrite(LCBLC_OUTGOING_RLSED) ;
        PtvErrIdcSend(forwardLcn, ErrCode_Blc_D, reverseLcn) ;
        PtvRlsCfmSend(forwardLcn, reverseLcn) ;
    }
}


/************************************************************************/
/*  function name       : Se_Blc_0309_0020                              */
/*  function outline    : Event     CloseLogicalChannelAck              */
/*                      :                            ( Bi-Directional ) */
/*                      : Status    Outgoing Established                */
/*  function discription: Se_Blc_0309_0020( PS_LCB_ENTRY ,              */
/*                      :                   PS_CloseLogicalChannelAck ) */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*                      : PS_CloseLogicalChannelAck                     */
/*                      :                      p_CloseLogicalChannelAck */
/*  output data         : None                                          */
/*  draw time           : '96.11.25                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void BLC::_0309_0020(LCEntry *p_LcbEntry , PS_CloseLogicalChannelAck p_CloseLogicalChannelAck)
{
    S_Source           source ;
    S_OlcRejectCause   cause ;

    OSCL_UNUSED_ARG(p_CloseLogicalChannelAck);

    uint32 forwardLcn = p_LcbEntry->FwdLcnRead();
    uint32 reverseLcn = p_LcbEntry->RvsLcnRead();

    p_LcbEntry->StatusWrite(LCBLC_OUTGOING_RLSED) ;
    PtvErrIdcSend(forwardLcn, ErrCode_Blc_C, reverseLcn) ;

    source.index = 1 ;
    oscl_memset((void*)&cause , 0 , sizeof(S_OlcRejectCause)) ;
    PtvRlsIdcSend(forwardLcn, &source , &cause, S_InfHeader::OUTGOING, reverseLcn) ;
}


/************************************************************************/
/*  function name       : Se_Blc_0309_0030                              */
/*  function outline    : Event     CloseLogicalChannelAck              */
/*                      :                            ( Bi-Directional ) */
/*                      : Status    Outgoing Awaiting Release           */
/*  function discription: Se_Blc_0309_0030( PS_LCB_ENTRY ,              */
/*                      :                   PS_CloseLogicalChannelAck ) */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*                      : PS_CloseLogicalChannelAck                     */
/*                      :                      p_CloseLogicalChannelAck */
/*  output data         : None                                          */
/*  draw time           : '96.11.25                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void BLC::_0309_0030(LCEntry *p_LcbEntry , PS_CloseLogicalChannelAck p_CloseLogicalChannelAck)
{
    OSCL_UNUSED_ARG(p_CloseLogicalChannelAck);

    uint32 forwardLcn = p_LcbEntry->FwdLcnRead();
    uint32 reverseLcn = p_LcbEntry->RvsLcnRead();

    T103TimerStop(forwardLcn) ;
    p_LcbEntry->StatusWrite(LCBLC_OUTGOING_RLSED) ;
    PtvRlsCfmSend(forwardLcn, reverseLcn) ;
}


/************************************************************************/
/*  function name       : Se_Blc_0301_0001                              */
/*  function outline    : Event     OpenLogicalChannel                  */
/*                      :                            ( Bi-Directional ) */
/*                      : Status    Incoming Released                   */
/*                      : Status    Outgoing Released                   */
/*  function discription: Se_Blc_0301_0001( PS_LCB_ENTRY ,              */
/*                      :                       PS_OpenLogicalChannel ) */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*                      : PS_OpenLogicalChannel p_OpenLogicalChannel    */
/*  output data         : None                                          */
/*  draw time           : '96.11.25                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void BLC::_0301_0001(LCEntry *p_LcbEntry , PS_OpenLogicalChannel p_OpenLogicalChannel)
{
    uint32 forwardLcn = p_LcbEntry->FwdLcnRead();
    uint32 reverseLcn = p_LcbEntry->RvsLcnRead();

    p_LcbEntry->StatusWrite(LCBLC_INCOMING_AWTING_ETBMNT) ;
    PtvEtbIdcSend(forwardLcn, p_OpenLogicalChannel, reverseLcn) ;
}


/************************************************************************/
/*  function name       : Se_Blc_0301_0011                              */
/*  function outline    : Event     OpenLogicalChannel                  */
/*                      :                            ( Bi-Directional ) */
/*                      : Status    Incoming Awaiting Establishment     */
/*                      : Status    Incoming Established                */
/*  function discription: Se_Blc_0301_0011( PS_LCB_ENTRY ,              */
/*                      :                       PS_OpenLogicalChannel ) */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*                      : PS_OpenLogicalChannel p_OpenLogicalChannel    */
/*  output data         : None                                          */
/*  draw time           : '96.11.25                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void BLC::_0301_0011(LCEntry *p_LcbEntry , PS_OpenLogicalChannel p_OpenLogicalChannel)
{
    S_Source           source ;
    S_OlcRejectCause   cause ;

    source.index = 0 ;  /* user */
    cause.index = 0 ;   /* unspecified */

    uint32 forwardLcn = p_LcbEntry->FwdLcnRead();
    uint32 reverseLcn = p_LcbEntry->RvsLcnRead();

    p_LcbEntry->StatusWrite(LCBLC_INCOMING_AWTING_ETBMNT) ;
    PtvRlsIdcSend(forwardLcn, &source , &cause , S_InfHeader::INCOMING, reverseLcn) ;
    PtvEtbIdcSend(forwardLcn, p_OpenLogicalChannel, reverseLcn) ;
}


/************************************************************************/
/*  function name       : Se_Blc_0301_0021                              */
/*  function outline    : Event     OpenLogicalChannel                  */
/*                      :                            ( Bi-Directional ) */
/*                      : Status    Incoming Awaiting Confirmation      */
/*  function discription: Se_Blc_0301_0021( PS_LCB_ENTRY ,              */
/*                      :                       PS_OpenLogicalChannel ) */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*                      : PS_OpenLogicalChannel p_OpenLogicalChannel   */
/*  output data         : None                                          */
/*  draw time           : '96.11.25                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void BLC::_0301_0021(LCEntry *p_LcbEntry , PS_OpenLogicalChannel p_OpenLogicalChannel)
{
    S_Source           source ;
    S_OlcRejectCause   cause ;

    uint32 forwardLcn = p_LcbEntry->FwdLcnRead();
    uint32 reverseLcn = p_LcbEntry->RvsLcnRead();

    T103TimerStop(forwardLcn) ;

    source.index = 0 ;  /* user */
    cause.index = 0 ;   /* unspecified */

    p_LcbEntry->StatusWrite(LCBLC_INCOMING_AWTING_ETBMNT) ;
    PtvRlsIdcSend(forwardLcn, &source , &cause, S_InfHeader::INCOMING, reverseLcn) ;
    PtvEtbIdcSend(forwardLcn, p_OpenLogicalChannel, reverseLcn) ;
}


/************************************************************************/
/*  function name       : Se_Blc_0302_0011                              */
/*  function outline    : Event     ESTABLISH.response                  */
/*                      :                            ( Bi-Directional ) */
/*                      : Status    Incoming Awaiting Establishment     */
/*  function discription: Se_Blc_0302_0011( PS_LCB_ENTRY ,              */
/*                      :                       PS_ReverseData , uint32 ) */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*                      : PS_ReverseData p_ReverseData                  */
/*                      : uint32 ReverseLogicalChannelNumber              */
/*  output data         : None                                          */
/*  draw time           : '96.11.26                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void BLC::_0302_0011(LCEntry *p_LcbEntry , PS_ReverseData p_ReverseData , uint16 ReverseLogicalChannelNumber)
{
    uint32 forwardLcn = p_LcbEntry->FwdLcnRead();

    p_LcbEntry->RvsLcnWrite(ReverseLogicalChannelNumber) ;
    p_ReverseData->reverseLogicalChannelParameters.reverseLogicalChannelNumber = ReverseLogicalChannelNumber;
    p_LcbEntry->StatusWrite(LCBLC_INCOMING_AWTING_CNFMTN) ;
    MsgOpenAckSend((uint16) forwardLcn, p_ReverseData) ;
    T103TimerStart(forwardLcn, p_LcbEntry->TmrSqcNumberInc()) ;
}


/************************************************************************/
/*  function name       : Se_Blc_0303_0011                              */
/*  function outline    : Event     RELEASE.request                     */
/*                      :                            ( Bi-Directional ) */
/*                      : Status    Incoming Awaiting Establishment     */
/*  function discription: Se_Blc_0303_0011( PS_LCB_ENTRY ,              */
/*                      :                              PS_Cause_LcBlc ) */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*                      : PS_Cause_LcBlc p_Cause_LcBlc                  */
/*  output data         : None                                          */
/*  draw time           : '96.11.25                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void BLC::_0303_0011(LCEntry *p_LcbEntry , PS_OlcRejectCause p_Cause)
{
    uint32 forwardLcn = p_LcbEntry->FwdLcnRead();

    p_LcbEntry->StatusWrite(LCBLC_INCOMING_RLSED) ;
    MsgOpenRjtSend((uint16) forwardLcn, p_Cause) ;
}


/************************************************************************/
/*  function name       : Se_Blc_0305_0011                              */
/*  function outline    : Event     OpenLogicalChannelConfirm           */
/*                      :                            ( Bi-Directional ) */
/*                      : Status    Incoming Awaiting Establishment     */
/*  function discription: Se_Blc_0305_0011( PS_LCB_ENTRY ,              */
/*                      :                PS_OpenLogicalChannelConfirm ) */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*                      : PS_OpenLogicalChannelConfirm                  */
/*                      :                   p_OpenLogicalChannelConfirm */
/*  output data         : None                                          */
/*  draw time           : '96.11.25                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void BLC::_0305_0011(LCEntry *p_LcbEntry , PS_OpenLogicalChannelConfirm p_OpenLogicalChannelConfirm)
{
    S_Source           source ;
    S_OlcRejectCause   cause ;

    OSCL_UNUSED_ARG(p_OpenLogicalChannelConfirm);

    uint32 forwardLcn = p_LcbEntry->FwdLcnRead();
    uint32 reverseLcn = p_LcbEntry->RvsLcnRead();

    p_LcbEntry->StatusWrite(LCBLC_INCOMING_RLSED) ;
    PtvErrIdcSend(forwardLcn, ErrCode_Blc_E, reverseLcn) ;
    source.index = 1 ;  /* user */
    oscl_memset((void*)&cause , 0 , sizeof(S_OlcRejectCause)) ;
    PtvRlsIdcSend(forwardLcn, &source , &cause, S_InfHeader::INCOMING, reverseLcn) ;
}


/************************************************************************/
/*  function name       : Se_Blc_0305_0021                              */
/*  function outline    : Event     OpenLogicalChannelConfirm           */
/*                      :                            ( Bi-Directional ) */
/*                      : Status    Incoming Awaiting Confirmation      */
/*  function discription: Se_Blc_0305_0021( PS_LCB_ENTRY ,              */
/*                      :                PS_OpenLogicalChannelConfirm ) */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*                      : PS_OpenLogicalChannelConfirm                  */
/*                      :                   p_OpenLogicalChannelConfirm */
/*  output data         : None                                          */
/*  draw time           : '96.11.25                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void BLC::_0305_0021(LCEntry *p_LcbEntry , PS_OpenLogicalChannelConfirm p_OpenLogicalChannelConfirm)
{
    S_OpenLogicalChannelAck     OpenLogicalChannelAck ;

    OSCL_UNUSED_ARG(p_OpenLogicalChannelConfirm);

    uint32 forwardLcn = p_LcbEntry->FwdLcnRead();
    uint32 reverseLcn = p_LcbEntry->RvsLcnRead();

    T103TimerStop(forwardLcn) ;

    OpenLogicalChannelAck.forwardLogicalChannelNumber = (uint16) forwardLcn;
    OpenLogicalChannelAck.option_of_ackReverseLogicalChannelParameters = false ;
    oscl_memset((void*)&OpenLogicalChannelAck.ackReverseLogicalChannelParameters ,
                0 ,
                sizeof(S_AckReverseLogicalChannelParameters)) ;
    OpenLogicalChannelAck.option_of_separateStack = false ;
    oscl_memset((void*)&OpenLogicalChannelAck.separateStack , 0 , sizeof(S_NetworkAccessParameters)) ;
    OpenLogicalChannelAck.option_of_forwardMultiplexAckParameters = false ;
    oscl_memset((void*)&OpenLogicalChannelAck.forwardMultiplexAckParameters , 0 , sizeof(S_ForwardMultiplexAckParameters)) ;

    p_LcbEntry->StatusWrite(LCBLC_INCOMING_ETBED) ;
    PtvEtbCfmSend2(forwardLcn, &OpenLogicalChannelAck, reverseLcn) ;
}


/************************************************************************/
/*  function name       : Se_Blc_0307_0021                              */
/*  function outline    : Event     Timer T103 Timeout                  */
/*                      :                            ( Bi-Directional ) */
/*                      : Status    Incoming Awaiting Confirmation      */
/*  function discription: Se_Blc_0307_0021( PS_LCB_ENTRY , int32 )        */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*                      : int32 TmrSqcNumber                              */
/*  output data         : None                                          */
/*  draw time           : '96.11.25                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void BLC::_0307_0021(LCEntry *p_LcbEntry , int32 TmrSqcNumber)
{
    S_Source           source ;
    S_OlcRejectCause   cause ;

    if (TmrSqcNumber == p_LcbEntry->TmrSqcNumberRead())
    {
        uint32 forwardLcn = p_LcbEntry->FwdLcnRead();
        uint32 reverseLcn = p_LcbEntry->RvsLcnRead();

        p_LcbEntry->StatusWrite(LCBLC_INCOMING_RLSED) ;

        PtvErrIdcSend(forwardLcn, ErrCode_Blc_F, reverseLcn) ;

        source.index = 1 ;  /* lcse */
        oscl_memset((void*)&cause , 0 , sizeof(S_OlcRejectCause)) ;
        PtvRlsIdcSend(forwardLcn, &source , &cause, S_InfHeader::INCOMING, reverseLcn) ;
    }
}


/************************************************************************/
/*  function name       : Se_Blc_0308_0001                              */
/*  function outline    : Event     CloseLogicalChannel                 */
/*                      :                            ( Bi-Directional ) */
/*                      : Status    Incoming Released                   */
/*  function discription: Se_Blc_0308_0001( PS_LCB_ENTRY ,              */
/*                      :                      PS_CloseLogicalChannel ) */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*                      : PS_CloseLogicalChannel p_CloseLogicalChannel  */
/*  output data         : None                                          */
/*  draw time           : '96.11.25                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void BLC::_0308_0001(LCEntry *p_LcbEntry , PS_CloseLogicalChannel p_CloseLogicalChannel)
{
    OSCL_UNUSED_ARG(p_CloseLogicalChannel);
    MsgCloseAckSend((uint16) p_LcbEntry->FwdLcnRead()) ;
}


/************************************************************************/
/*  function name       : Se_Blc_0308_0011                              */
/*  function outline    : Event     CloseLogicalChannel                 */
/*                      :                            ( Bi-Directional ) */
/*                      : Status    Incoming Awaiting Establishment     */
/*                      : Status    Incoming Established                */
/*  function discription: Se_Blc_0308_0011( PS_LCB_ENTRY ,              */
/*                      :                      PS_CloseLogicalChannel ) */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*                      : PS_CloseLogicalChannel p_CloseLogicalChannel  */
/*  output data         : None                                          */
/*  draw time           : '96.11.25                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void BLC::_0308_0011(LCEntry *p_LcbEntry , PS_CloseLogicalChannel p_CloseLogicalChannel)
{
    S_OlcRejectCause   cause ;

    uint32 forwardLcn = p_LcbEntry->FwdLcnRead();
    uint32 reverseLcn = p_LcbEntry->RvsLcnRead();

    p_LcbEntry->StatusWrite(LCBLC_INCOMING_RLSED) ;

    MsgCloseAckSend((uint16) forwardLcn) ;

    cause.index = 0 ;  /* unspecified */
    PtvRlsIdcSend(forwardLcn, &p_CloseLogicalChannel->source , &cause, S_InfHeader::INCOMING, reverseLcn) ;
}


/************************************************************************/
/*  function name       : Se_Blc_0308_0021                              */
/*  function outline    : Event     CloseLogicalChannel                 */
/*                      :                            ( Bi-Directional ) */
/*                      : Status    Incoming Awaiting Confirmation      */
/*  function discription: Se_Blc_0308_0021( PS_LCB_ENTRY ,              */
/*                      :                      PS_CloseLogicalChannel ) */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*                      : PS_CloseLogicalChannel p_CloseLogicalChannel  */
/*  output data         : None                                          */
/*  draw time           : '96.11.25                                     */
/*----------------------------------------------------------------------*/
/*  amendent career ()  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void BLC::_0308_0021(LCEntry *p_LcbEntry , PS_CloseLogicalChannel p_CloseLogicalChannel)
{
    S_OlcRejectCause   cause ;

    uint32 forwardLcn = p_LcbEntry->FwdLcnRead();
    uint32 reverseLcn = p_LcbEntry->RvsLcnRead();

    T103TimerStop(forwardLcn) ;

    cause.index = 0 ;  /* unspecified */

    p_LcbEntry->StatusWrite(LCBLC_INCOMING_RLSED) ;
    MsgCloseAckSend((uint16) forwardLcn) ;
    PtvRlsIdcSend(forwardLcn, &p_CloseLogicalChannel->source , &cause, S_InfHeader::INCOMING, reverseLcn) ;
}
