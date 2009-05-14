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
/*  file name       : selc.c                                            */
/*  file contents   : Uni-Directional Logical Channel Signalling Entity */
/*                  :                                        Management */
/*  draw            : '96.11.25                                         */
/*----------------------------------------------------------------------*/
/*  amendment       :                                                   */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/


/************************************************************************/
/*  Headerfile Include                                                  */
/************************************************************************/
#include "h245inf.h"
#include "lcentry.h"
#include "msd.h"
#include "lc.h"

/************************************************************************/
/*  function name       : Se_Lc_0200_0000                               */
/*  function outline    : Event     ESTABLISH.request                   */
/*                      :                           ( Uni-Directional ) */
/*                      : Status    Outgoing Released                   */
/*  function discription: Se_Lc_0200_0000( PS_LCB_ENTRY ,               */
/*                      :                      PS_ForwardReverseParam ) */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*                      : PS_ForwardReverseParam p_ForwardReverseParam  */
/*  output data         : None                                          */
/*  draw time           : '96.11.01                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void LC::_0200_0000(LCEntry *p_LcbEntry , PS_ForwardReverseParam p_ForwardReverseParam)
{
    uint32 forwardLcn = p_LcbEntry->FwdLcnRead();
    p_LcbEntry->StatusWrite(LCBLC_OUTGOING_AWTING_ETBMNT) ;
    MsgOpenSend((uint16) forwardLcn, p_ForwardReverseParam) ;
    T103TimerStart(forwardLcn, p_LcbEntry->TmrSqcNumberInc()) ;
}


/************************************************************************/
/*  function name       : Se_Lc_0200_0030                               */
/*  function outline    : Event     ESTABLISH.request                   */
/*                      :                           ( Uni-Directional ) */
/*                      : Status    Outgoing Awaiting Release           */
/*  function discription: Se_Lc_0200_0030( PS_LCB_ENTRY ,               */
/*                      :                      PS_ForwardReverseParam ) */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*                      : PS_ForwardReverseParam p_ForwardReverseParam  */
/*  output data         : None                                          */
/*  draw time           : '96.11.01                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void LC::_0200_0030(LCEntry *p_LcbEntry , PS_ForwardReverseParam p_ForwardReverseParam)
{
    uint32 forwardLcn = p_LcbEntry->FwdLcnRead();
    T103TimerStop(forwardLcn) ;
    p_LcbEntry->StatusWrite(LCBLC_OUTGOING_AWTING_ETBMNT) ;
    MsgOpenSend((uint16) forwardLcn, p_ForwardReverseParam) ;
    T103TimerStart(forwardLcn, p_LcbEntry->TmrSqcNumberInc()) ;
}


/************************************************************************/
/*  function name       : Se_Lc_0201_0010                               */
/*  function outline    : Event     OpenLogicalChannel                  */
/*                      :                           ( Uni-Directional ) */
/*                      : Status    Outgoing Awaiting Establishment     */
/*  function discription: Se_Lc_0201_0010( PS_LCB_ENTRY ,               */
/*                      :                       PS_OpenLogicalChannel ) */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*                      : PS_OpenLogicalChannel p_OpenLogicalChannel    */
/*  output data         : None                                          */
/*  draw time           : '96.11.01                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void LC::_0201_0010(LCEntry *p_LcbEntry , PS_OpenLogicalChannel p_OpenLogicalChannel)
{
    if (MyMSD->GetStatus() != MSD_MASTER)
    {
        uint32 forwardLcn = p_LcbEntry->FwdLcnRead();
        T103TimerStop(forwardLcn) ;
        p_LcbEntry->StatusWrite(LCBLC_INCOMING_AWTING_ETBMNT) ;
        PtvEtbIdcSend(forwardLcn , p_OpenLogicalChannel) ;
    }
}


/************************************************************************/
/*  function name       : Se_Lc_0203_0010                               */
/*  function outline    : Event     RELEASE.request                     */
/*                      :                           ( Uni-Directional ) */
/*                      : Status    Outgoing Awaiting Establishment     */
/*  function discription: Se_Lc_0203_0010( PS_LCB_ENTRY ,               */
/*                      :                              PS_Cause_LcBlc ) */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*                      : PS_Cause_LcBlc p_Cause_LcBlc                  */
/*  output data         : None                                          */
/*  draw time           : '96.11.01                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void LC::_0203_0010(LCEntry *p_LcbEntry , PS_OlcRejectCause p_Cause_LcBlc)
{
    S_Source    source ;

    OSCL_UNUSED_ARG(p_Cause_LcBlc);

    uint32 forwardLcn = p_LcbEntry->FwdLcnRead();

    T103TimerStop(forwardLcn) ;

    /* NEW245
    source.select_of_Source = 0 ;
    source.user = true ;
    source.lcse = false ;
    */
    source.index = 0;

    p_LcbEntry->StatusWrite(LCBLC_OUTGOING_AWTING_RLS) ;
    MsgCloseSend((uint16) forwardLcn, &source) ;
    T103TimerStart(forwardLcn, p_LcbEntry->TmrSqcNumberInc()) ;
}


/************************************************************************/
/*  function name       : Se_Lc_0203_0020                               */
/*  function outline    : Event     RELEASE.request                     */
/*                      :                           ( Uni-Directional ) */
/*                      : Status    Outgoing Established                */
/*  function discription: Se_Lc_0203_0020( PS_LCB_ENTRY ,               */
/*                      :                              PS_Cause_LcBlc ) */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*                      : PS_Cause_LcBlc p_Cause_LcBlc                  */
/*  output data         : None                                          */
/*  draw time           : '96.11.25                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void LC::_0203_0020(LCEntry *p_LcbEntry , PS_OlcRejectCause p_Cause_LcBlc)
{
    S_Source    source ;

    OSCL_UNUSED_ARG(p_Cause_LcBlc);
    /* NEW245
    source.select_of_Source = 0 ;
    source.user = true ;
    source.lcse = false ;
    */
    source.index = 0;

    uint32 forwardLcn = p_LcbEntry->FwdLcnRead();

    p_LcbEntry->StatusWrite(LCBLC_OUTGOING_AWTING_RLS) ;
    MsgCloseSend((uint16) forwardLcn, &source) ;
    T103TimerStart(forwardLcn, p_LcbEntry->TmrSqcNumberInc()) ;
}


/************************************************************************/
/*  function name       : Se_Lc_0204_0000                               */
/*  function outline    : Event     OpenLogicalChannelAck               */
/*                      :                           ( Uni-Directional ) */
/*                      : Status    Outgoing Released                   */
/*  function discription: Se_Lc_0204_0000( PS_LCB_ENTRY ,               */
/*                      :                    PS_OpenLogicalChannelAck ) */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*                      : PS_OpenLogicalChannelAck                      */
/*                      :                       p_OpenLogicalChannelAck */
/*  output data         : None                                          */
/*  draw time           : '96.11.01                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void LC::_0204_0000(LCEntry *p_LcbEntry , PS_OpenLogicalChannelAck p_OpenLogicalChannelAck)
{
    OSCL_UNUSED_ARG(p_OpenLogicalChannelAck);
    PtvErrIdcSend(p_LcbEntry->FwdLcnRead() , ErrCode_Lc_A) ;
}


/************************************************************************/
/*  function name       : Se_Lc_0204_0010                               */
/*  function outline    : Event     OpenLogicalChannelAck               */
/*                      :                           ( Uni-Directional ) */
/*                      : Status    Outgoing Awaiting Establishment     */
/*  function discription: Se_Lc_0204_0010( PS_LCB_ENTRY ,               */
/*                      :                    PS_OpenLogicalChannelAck ) */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*                      : PS_OpenLogicalChannelAck                      */
/*                      :                       p_OpenLogicalChannelAck */
/*  output data         : None                                          */
/*  draw time           : '96.11.01                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void LC::_0204_0010(LCEntry *p_LcbEntry , PS_OpenLogicalChannelAck p_OpenLogicalChannelAck)
{
    uint32 forwardLcn = p_LcbEntry->FwdLcnRead();
    T103TimerStop(forwardLcn) ;
    p_LcbEntry->StatusWrite(LCBLC_OUTGOING_ETBED) ;
    PtvEtbCfmSend(forwardLcn, p_OpenLogicalChannelAck) ;
}


/************************************************************************/
/*  function name       : Se_Lc_0206_0000                               */
/*  function outline    : Event     OpenLogicalChannelReject            */
/*                      :                           ( Uni-Directional ) */
/*                      : Status    Outgoing Released                   */
/*  function discription: Se_Lc_0206_0000( PS_LCB_ENTRY ,               */
/*                      :                 PS_OpenLogicalChannelReject ) */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*                      : PS_OpenLogicalChannelReject                   */
/*                      :                    p_OpenLogicalChannelReject */
/*  output data         : None                                          */
/*  draw time           : '96.11.01                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void LC::_0206_0000(LCEntry *p_LcbEntry , PS_OpenLogicalChannelReject p_OpenLogicalChannelReject)
{
    OSCL_UNUSED_ARG(p_OpenLogicalChannelReject);
    PtvErrIdcSend(p_LcbEntry->FwdLcnRead() , ErrCode_Lc_B) ;
}


/************************************************************************/
/*  function name       : Se_Lc_0206_0010                               */
/*  function outline    : Event     OpenLogicalChannelReject            */
/*                      :                           ( Uni-Directional ) */
/*                      : Status    Outgoing Awaiting Establishment     */
/*  function discription: Se_Lc_0206_0010( PS_LCB_ENTRY ,               */
/*                      :                 PS_OpenLogicalChannelReject ) */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*                      : PS_OpenLogicalChannelReject                   */
/*                      :                    p_OpenLogicalChannelReject */
/*  output data         : None                                          */
/*  draw time           : '96.11.01                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void LC::_0206_0010(LCEntry *p_LcbEntry , PS_OpenLogicalChannelReject p_OpenLogicalChannelReject)
{
    S_Source    source ;

    uint32 forwardLcn = p_LcbEntry->FwdLcnRead();

    T103TimerStop(forwardLcn) ;

    source.index = 0;  /* user */

    p_LcbEntry->StatusWrite(LCBLC_OUTGOING_RLSED) ;
    PtvRlsIdcSend(forwardLcn, &source , &p_OpenLogicalChannelReject->olcRejectCause, S_InfHeader::OUTGOING) ;
}


/************************************************************************/
/*  function name       : Se_Lc_0206_0020                               */
/*  function outline    : Event     OpenLogicalChannelReject            */
/*                      :                           ( Uni-Directional ) */
/*                      : Status    Outgoing Established                */
/*  function discription: Se_Lc_0206_0020( PS_LCB_ENTRY ,               */
/*                      :                 PS_OpenLogicalChannelReject ) */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*                      : PS_OpenLogicalChannelReject                   */
/*                      :                    p_OpenLogicalChannelReject */
/*  output data         : None                                          */
/*  draw time           : '96.11.01                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void LC::_0206_0020(LCEntry *p_LcbEntry , PS_OpenLogicalChannelReject p_OpenLogicalChannelReject)
{
    S_Source    source ;

    uint32 forwardLcn = p_LcbEntry->FwdLcnRead();

    p_LcbEntry->StatusWrite(LCBLC_OUTGOING_RLSED) ;

    PtvErrIdcSend(forwardLcn, ErrCode_Lc_B) ;

    /* NEW245
    source.select_of_Source = 1 ;
    source.user = false ;
    source.lcse = true ;
    */
    source.index = 1;

    PtvRlsIdcSend(forwardLcn, &source , &p_OpenLogicalChannelReject->olcRejectCause, S_InfHeader::OUTGOING) ;
}


/************************************************************************/
/*  function name       : Se_Lc_0206_0030                               */
/*  function outline    : Event     OpenLogicalChannelReject            */
/*                      :                           ( Uni-Directional ) */
/*                      : Status    Outgoing Awaiting Release           */
/*  function discription: Se_Lc_0206_0030( PS_LCB_ENTRY ,               */
/*                      :                 PS_OpenLogicalChannelReject ) */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*                      : PS_OpenLogicalChannelReject                   */
/*                      :                    p_OpenLogicalChannelReject */
/*  output data         : None                                          */
/*  draw time           : '96.11.01                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void LC::_0206_0030(LCEntry *p_LcbEntry , PS_OpenLogicalChannelReject p_OpenLogicalChannelReject)
{
    OSCL_UNUSED_ARG(p_OpenLogicalChannelReject);
    uint32 forwardLcn = p_LcbEntry->FwdLcnRead();

    T103TimerStop(forwardLcn) ;
    p_LcbEntry->StatusWrite(LCBLC_OUTGOING_RLSED) ;
    PtvRlsCfmSend(forwardLcn) ;
}


/************************************************************************/
/*  function name       : Se_Lc_0207_0010                               */
/*  function outline    : Event     Timer T103 Timeout                  */
/*                      :                           ( Uni-Directional ) */
/*                      : Status    Outgoing Awaiting Establishment     */
/*  function discription: Se_Lc_0207_0010( PS_LCB_ENTRY , int32 )         */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*                      : int32 TmrSqcNumber                              */
/*  output data         : None                                          */
/*  draw time           : '96.11.11                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void LC::_0207_0010(LCEntry *p_LcbEntry , int32 TmrSqcNumber)
{
    S_Source        source ;
    S_OlcRejectCause   cause ;

    if (TmrSqcNumber == p_LcbEntry->TmrSqcNumberRead())
    {
        uint32 forwardLcn = p_LcbEntry->FwdLcnRead();

        p_LcbEntry->StatusWrite(LCBLC_OUTGOING_RLSED) ;
        PtvErrIdcSend(forwardLcn, ErrCode_Lc_D) ;

        /* NEW245
        source.select_of_Source = 1 ;
        source.user = false ;
        source.lcse = true ;
        */
        source.index = 1;

        MsgCloseSend((uint16) forwardLcn, &source) ;

        cause.index = 0 ;
        /* NEW245
        cause.unspecified = true ;
        cause.unsuitableReverseParameters = false ;
        cause.dataTypeNotSupported = false ;
        cause.dataTypeNotAvailable = false ;
        cause.unknownDataType = false ;
        cause.dataTypeALCombinationNotSupported = false ;
        cause.multicastChanneNotAllowed = false ;
        cause.insufficientBandwidth = false ;
        cause.separateStackEstablishmentFailed = false ;
        cause.invalidSessionID = false ;
        */

        PtvRlsIdcSend(forwardLcn, &source , &cause, S_InfHeader::OUTGOING) ;
    }
}


/************************************************************************/
/*  function name       : Se_Lc_0207_0030                               */
/*  function outline    : Event     Timer T103 Timeout                  */
/*                      :                           ( Uni-Directional ) */
/*                      : Status    Outgoing Awaiting Release           */
/*  function discription: Se_Lc_0207_0030( PS_LCB_ENTRY , int32 )         */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*                      : int32 TmrSqcNumber                              */
/*  output data         : None                                          */
/*  draw time           : '96.11.11                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void LC::_0207_0030(LCEntry *p_LcbEntry , int32 TmrSqcNumber)
{
    if (TmrSqcNumber == p_LcbEntry->TmrSqcNumberRead())
    {
        uint32 forwardLcn = p_LcbEntry->FwdLcnRead();
        p_LcbEntry->StatusWrite(LCBLC_OUTGOING_RLSED) ;
        PtvErrIdcSend(forwardLcn, ErrCode_Lc_D) ;
        PtvRlsCfmSend(forwardLcn) ;
    }
}


/************************************************************************/
/*  function name       : Se_Lc_0209_0020                               */
/*  function outline    : Event     CloseLogicalChannelAck              */
/*                      :                           ( Uni-Directional ) */
/*                      : Status    Outgoing Established                */
/*  function discription: Se_Lc_0209_0020( PS_LCB_ENTRY ,               */
/*                      :                   PS_CloseLogicalChannelAck ) */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*                      : PS_CloseLogicalChannelAck                     */
/*                      :                      p_CloseLogicalChannelAck */
/*  output data         : None                                          */
/*  draw time           : '96.11.01                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void LC::_0209_0020(LCEntry *p_LcbEntry , PS_CloseLogicalChannelAck p_CloseLogicalChannelAck)
{
    S_Source        source ;
    S_OlcRejectCause   cause ;

    OSCL_UNUSED_ARG(p_CloseLogicalChannelAck);

    uint32 forwardLcn = p_LcbEntry->FwdLcnRead();

    p_LcbEntry->StatusWrite(LCBLC_OUTGOING_RLSED) ;

    PtvErrIdcSend(forwardLcn, ErrCode_Lc_C) ;

    /* NEW245
    source.select_of_Source = 1 ;
    source.user = false ;
    source.lcse = true ;
    */
    source.index = 1;

    cause.index = 0 ;
    /* NEW245
    cause.unspecified = true ;
    cause.unsuitableReverseParameters = false ;
    cause.dataTypeNotSupported = false ;
    cause.dataTypeNotAvailable = false ;
    cause.unknownDataType = false ;
    cause.dataTypeALCombinationNotSupported = false ;
    cause.multicastChanneNotAllowed = false ;
    cause.insufficientBandwidth = false ;
    cause.separateStackEstablishmentFailed = false ;
    cause.invalidSessionID = false ;
    */

    PtvRlsIdcSend(forwardLcn, &source , &cause, S_InfHeader::OUTGOING) ;
}


/************************************************************************/
/*  function name       : Se_Lc_0209_0030                               */
/*  function outline    : Event     CloseLogicalChannelAck              */
/*                      :                           ( Uni-Directional ) */
/*                      : Status    Outgoing Awaiting Release           */
/*  function discription: Se_Lc_0209_0030( PS_LCB_ENTRY ,               */
/*                      :                   PS_CloseLogicalChannelAck ) */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*                      : PS_CloseLogicalChannelAck                     */
/*                      :                      p_CloseLogicalChannelAck */
/*  output data         : None                                          */
/*  draw time           : '96.11.01                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void LC::_0209_0030(LCEntry *p_LcbEntry , PS_CloseLogicalChannelAck p_CloseLogicalChannelAck)
{
    OSCL_UNUSED_ARG(p_CloseLogicalChannelAck);
    uint32 forwardLcn = p_LcbEntry->FwdLcnRead();
    T103TimerStop(forwardLcn) ;
    p_LcbEntry->StatusWrite(LCBLC_OUTGOING_RLSED) ;
    PtvRlsCfmSend(forwardLcn) ;
}


/************************************************************************/
/*  function name       : Se_Lc_0201_0001                               */
/*  function outline    : Event     OpenLogicalChannel                  */
/*                      :                           ( Uni-Directional ) */
/*                      : Status    Incoming Released                   */
/*                      : Status    Outgoing Released                   */
/*  function discription: Se_Lc_0201_0001( PS_LCB_ENTRY ,               */
/*                      :                       PS_OpenLogicalChannel ) */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*                      : PS_OpenLogicalChannel p_OpenLogicalChannel    */
/*  output data         : None                                          */
/*  draw time           : '96.11.01                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void LC::_0201_0001(LCEntry *p_LcbEntry , PS_OpenLogicalChannel p_OpenLogicalChannel)
{
    uint32 forwardLcn = p_LcbEntry->FwdLcnRead();
    p_LcbEntry->StatusWrite(LCBLC_INCOMING_AWTING_ETBMNT) ;
    PtvEtbIdcSend(forwardLcn, p_OpenLogicalChannel) ;
}


/************************************************************************/
/*  function name       : Se_Lc_0201_0011                               */
/*  function outline    : Event     OpenLogicalChannel                  */
/*                      :                           ( Uni-Directional ) */
/*                      : Status    Incoming Awaiting Establishment     */
/*                      : Status    Incoming Established                */
/*  function discription: Se_Lc_0201_0011( PS_LCB_ENTRY ,               */
/*                      :                       PS_OpenLogicalChannel ) */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*                      : PS_OpenLogicalChannel p_OpenLogicalChannel    */
/*  output data         : None                                          */
/*  draw time           : '96.11.01                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void LC::_0201_0011(LCEntry *p_LcbEntry , PS_OpenLogicalChannel p_OpenLogicalChannel)
{
    S_Source        source ;
    S_OlcRejectCause   cause ;

    source.index = 0;  /* User */

    cause.index = 999 ; /* Special cause to indicate received CLC */
    // Note that the RELEASE.indication generated here is essentially the
    //   same as if a CLC has been received for the given channel.
    //   (it's really a new OLC for an existing, established channel, so
    //    the intent seems to be to close the channel and immediately
    //    reopen it).
    // See the notes I made in Se_Lc_0208_0011() for more info.
    // (This was added 9/29/00, RAN)

    uint32 forwardLcn = p_LcbEntry->FwdLcnRead();

    p_LcbEntry->StatusWrite(LCBLC_INCOMING_AWTING_ETBMNT) ;
    PtvRlsIdcSend(forwardLcn, &source , &cause, S_InfHeader::INCOMING) ;
    PtvEtbIdcSend(forwardLcn, p_OpenLogicalChannel) ;
}


/************************************************************************/
/*  function name       : Se_Lc_0202_0011                               */
/*  function outline    : Event     ESTABLISH.response                  */
/*                      :                           ( Uni-Directional ) */
/*                      : Status    Incoming Awaiting Establishment     */
/*  function discription: Se_Lc_0202_0011( PS_LCB_ENTRY ,               */
/*                      :                              PS_ReverseData ) */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*                      : PS_ReverseData p_ReverseData                  */
/*  output data         : None                                          */
/*  draw time           : '96.11.24                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void LC::_0202_0011(LCEntry *p_LcbEntry , PS_ReverseData p_ReverseData)
{
    uint32 forwardLcn = p_LcbEntry->FwdLcnRead();
    p_LcbEntry->StatusWrite(LCBLC_INCOMING_ETBED) ;
    MsgOpenAckSend((uint16) forwardLcn, p_ReverseData) ;
}


/************************************************************************/
/*  function name       : Se_Lc_0203_0011                               */
/*  function outline    : Event     RELEASE.request                     */
/*                      :                           ( Uni-Directional ) */
/*                      : Status    Incoming Awaiting Establishment     */
/*  function discription: Se_Lc_0203_0011( PS_LCB_ENTRY ,               */
/*                      :                              PS_Cause_LcBlc ) */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*                      : PS_Cause_LcBlc p_Cause_LcBlc                  */
/*  output data         : None                                          */
/*  draw time           : '96.11.01                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void LC::_0203_0011(LCEntry *p_LcbEntry , PS_OlcRejectCause p_Cause_LcBlc)
{
    uint32 forwardLcn = p_LcbEntry->FwdLcnRead();
    p_LcbEntry->StatusWrite(LCBLC_INCOMING_RLSED) ;
    MsgOpenRjtSend((uint16) forwardLcn, p_Cause_LcBlc) ;
}


/************************************************************************/
/*  function name       : Se_Lc_0208_0001                               */
/*  function outline    : Event     CloseLogicalChannel                 */
/*                      :                           ( Uni-Directional ) */
/*                      : Status    Incoming Released                   */
/*  function discription: Se_Lc_0208_0001( PS_LCB_ENTRY ,               */
/*                      :                      PS_CloseLogicalChannel ) */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*                      : PS_CloseLogicalChannel p_CloseLogicalChannel  */
/*  output data         : None                                          */
/*  draw time           : '96.11.01                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void LC::_0208_0001(LCEntry *p_LcbEntry , PS_CloseLogicalChannel p_CloseLogicalChannel)
{
    OSCL_UNUSED_ARG(p_CloseLogicalChannel);
    MsgCloseAckSend((uint16) p_LcbEntry->FwdLcnRead()) ;
}


/************************************************************************/
/*  function name       : Se_Lc_0208_0011                               */
/*  function outline    : Event     CloseLogicalChannel                 */
/*                      :                           ( Uni-Directional ) */
/*                      : Status    Incoming Awaiting Establishment     */
/*                      : Status    Incoming Established                */
/*  function discription: Se_Lc_0208_0011( PS_LCB_ENTRY ,               */
/*                      :                      PS_CloseLogicalChannel ) */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*                      : PS_CloseLogicalChannel p_CloseLogicalChannel  */
/*  output data         : None                                          */
/*  draw time           : '96.11.01                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void LC::_0208_0011(LCEntry *p_LcbEntry , PS_CloseLogicalChannel p_CloseLogicalChannel)
{
    S_OlcRejectCause   cause ;

    uint32 forwardLcn = p_LcbEntry->FwdLcnRead();

    p_LcbEntry->StatusWrite(LCBLC_INCOMING_RLSED) ;

    MsgCloseAckSend((uint16) forwardLcn) ;

    cause.index = 999 ; /* Special cause to indicate received CLC */
    // NOTE: I'm putting this in because there's currently no way for
    //   Tsc to distinguish between the following cases:
    //   1. Outgoing LCSE: OLCReject received or Timer Expired
    //   2. Incoming LCSE: CLC received for normal channel closure
    // We'll require Tsc to intercept this special (illegal) OLCRejectCause
    //   and take action appropriate for a received CLC.
    // (This was added 9/29/00, RAN)

    PtvRlsIdcSend(forwardLcn, &p_CloseLogicalChannel->source , &cause, S_InfHeader::INCOMING) ;
}
