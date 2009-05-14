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
/*  file name       : seclc.c                                           */
/*  file contents   : Close Logical Channel                             */
/*                  :                      Signalling Entity Management */
/*  draw            : '96.11.19                                         */
/*----------------------------------------------------------------------*/
/*  amendment       :                                                   */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/


/************************************************************************/
/*  Headerfile Include                                                  */
/************************************************************************/
#include    "h245inf.h"
#include    "semsgque.h"
#include	"clc.h"
#include	"lcentry.h"


/************************************************************************/
/*  function name       : Se_Clc_0400_0000                              */
/*  function outline    : Event     CLOSE.request                       */
/*                      : Status    Outgoing Idle                       */
/*  function discription: void Se_Clc_0400_0000( )                      */
/*  input data          : int32 Lcn                                       */
/*  output data         : None                                          */
/*  draw time           : '96.11.19                                     */
/*----------------------------------------------------------------------*/
/*  amendent career ()  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void CLC::_0400_0000(LCEntry *pLcbEntry)
{
    pLcbEntry->ClcStatusWrite(CLS_OUTGOING_AWTING_RPS) ;
    MsgCloseSend(pLcbEntry) ;
    T108TimerStart(pLcbEntry) ;
}


/************************************************************************/
/*  function name       : Se_Clc_0401_0001                              */
/*  function outline    : Event     RequestChannelClose                 */
/*                      : Status    Incoming Idle                       */
/*  function discription: void Se_Clc_0401_0001()  */
/*  output data         : None                                          */
/*  draw time           : '96.11.19                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void CLC::_0401_0001(LCEntry *pLcbEntry)
{
    pLcbEntry->ClcStatusWrite(CLS_INCOMING_AWTING_RPS) ;
    PtvCloseIdcSend(pLcbEntry) ;
}


/************************************************************************/
/*  function name       : Se_Clc_0401_0011                              */
/*  function outline    : Event     RequestChannelClose                 */
/*                      : Status    Incoming Awaiting Response          */
/*  function discription: void Se_Clc_0401_0011( )  */
/*  output data         : None                                          */
/*  draw time           : '96.11.19                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void CLC::_0401_0011(LCEntry *pLcbEntry)
{
    S_RccRejectCause    cause;

    cause.index = 0 ;  /* unspecified */
    PtvRjtIdcSend(pLcbEntry, Src_USER , &cause) ;
    PtvCloseIdcSend(pLcbEntry) ;
}

/************************************************************************/
/*  function name       : Se_Clc_0402_0011                              */
/*  function outline    : Event     CLOSE.response                      */
/*                      : Status    Incoming Awaiting Response          */
/*  function discription: void Se_Clc_0402_0011( void )                 */
/*  input data          : None                                          */
/*  output data         : None                                          */
/*  draw time           : '96.11.19                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void CLC::_0402_0011(LCEntry *pLcbEntry)
{
    pLcbEntry->ClcStatusWrite(CLS_INCOMING_IDLE) ;
    MsgCloseAckSend(pLcbEntry) ;
}


/************************************************************************/
/*  function name       : Se_Clc_0403_0011                              */
/*  function outline    : Event     REJECT.request                      */
/*                      : Status    Incoming Awaiting Response          */
/*  function discription: void Se_Clc_0403_0011( PS_RccRejectCause )         */
/*  input data          : PS_RccRejectCause pCause_Clc                       */
/*  output data         : None                                          */
/*  draw time           : '96.11.19                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void CLC::_0403_0011(LCEntry *pLcbEntry, PS_RccRejectCause pCause)
{
    pLcbEntry->ClcStatusWrite(CLS_INCOMING_IDLE) ;
    MsgCloseRjtSend(pLcbEntry, pCause) ;
}


/************************************************************************/
/*  function name       : Se_Clc_0404_0010                              */
/*  function outline    : Event     RequestChannelCloseAck              */
/*                      : Status    Outgoing Awaiting Response          */
/*  function discription: void Se_Clc_0404_0010( ) */
/*  output data         : None                                          */
/*  draw time           : '96.11.19                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void CLC::_0404_0010(LCEntry *pLcbEntry)
{
    T108TimerStop(pLcbEntry) ;
    pLcbEntry->ClcStatusWrite(CLS_OUTGOING_IDLE) ;
    PtvCloseCfmSend(pLcbEntry) ;
}


/************************************************************************/
/*  function name       : Se_Clc_0405_0010                              */
/*  function outline    : Event     RequestChannelCloseReject           */
/*                      : Status    Outgoing Awaiting Response          */
/*  function discription: void Se_Clc_0405_0010                         */
/*                      :              ( PS_RequestChannelCloseReject ) */
/*  input data          : PS_RequestChannelCloseReject                  */
/*                                          pRequestChannelCloseReject  */
/*  output data         : None                                          */
/*  draw time           : '96.11.19                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void CLC::_0405_0010(LCEntry *pLcbEntry, PS_RequestChannelCloseReject pRequestChannelCloseReject)
{
    T108TimerStop(pLcbEntry) ;
    pLcbEntry->ClcStatusWrite(CLS_OUTGOING_IDLE) ;
    PtvRjtIdcSend(pLcbEntry, Src_USER, &pRequestChannelCloseReject->rccRejectCause) ;
}


/************************************************************************/
/*  function name       : Se_Clc_0406_0010                              */
/*  function outline    : Event     Timer T108 Timeout                  */
/*                      : Status    Outgoing Awaiting Response          */
/*  function discription: void Se_Clc_0406_0010( int32 )                  */
/*  input data          : int32 Timer_Sequence_Number                     */
/*  output data         : None                                          */
/*  draw time           : '96.11.19                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void CLC::_0406_0010(LCEntry *pLcbEntry, int32 Timer_Sequence_Number)
{
    S_RccRejectCause    cause;

    /* Receive Timer Sequense Number Equal Timer Sequence Number */
    if (Timer_Sequence_Number == pLcbEntry->ClcTmrSqcNumberRead())
    {
        pLcbEntry->ClcStatusWrite(CLS_OUTGOING_IDLE) ;
        MsgCloseRlsSend(pLcbEntry) ;
        cause.index = 0 ;  /* unspecified */
        PtvRjtIdcSend(pLcbEntry, Src_PROTOCOL , &cause) ;
    }
}


/************************************************************************/
/*  function name       : Se_Clc_0407_0011                              */
/*  function outline    : Event     RequestChannelCloseRelease          */
/*                      : Status    Incoming Awaiting Response          */
/*  function discription: void Se_Clc_0407_0011( void )                 */
/*  input data          : None                                          */
/*  output data         : None                                          */
/*  draw time           : '96.11.19                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void CLC::_0407_0011(LCEntry *pLcbEntry)
{
    S_RccRejectCause    cause;

    cause.index = 0 ;  /* unspecified */
    pLcbEntry->ClcStatusWrite(CLS_INCOMING_IDLE) ;
    PtvRjtIdcSend(pLcbEntry, Src_PROTOCOL , &cause) ;
}


/************************************************************************/
/*  function name       : Se_ClcMsgCloseSend                            */
/*  function outline    : RequestChannelClose Send                      */
/*  function discription: void Se_ClcMsgCloseSend( void )               */
/*  input data          : None                                          */
/*  output data         : None                                          */
/*  draw time           : '96.11.19                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void CLC::MsgCloseSend(LCEntry *pLcbEntry)
{
    S_RequestChannelClose  requestChannelClose ;
    S_H245Msg              h245Msg ;

    requestChannelClose.forwardLogicalChannelNumber = (uint16) pLcbEntry->FwdLcnRead() ;
    requestChannelClose.option_of_qosCapability = false;
    requestChannelClose.option_of_rccReason = false;

    h245Msg.Type1 = H245_MSG_REQ ;
    h245Msg.Type2 = MSGTYP_CLC ;
    h245Msg.pData = (uint8*) & requestChannelClose ;

    MessageSend(&h245Msg) ;
}


/************************************************************************/
/*  function name       : Se_ClcMsgCloseAckSend                         */
/*  function outline    : RequestChannelCloseAck Send                   */
/*  function discription: void Se_ClcMsgCloseAckSend( void )            */
/*  input data          : None                                          */
/*  output data         : None                                          */
/*  draw time           : '96.11.19                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void CLC::MsgCloseAckSend(LCEntry *pLcbEntry)
{
    S_RequestChannelCloseAck   requestChannelCloseAck ;
    S_H245Msg                  h245Msg ;

    requestChannelCloseAck.forwardLogicalChannelNumber = (uint16) pLcbEntry->FwdLcnRead() ;

    h245Msg.Type1 = H245_MSG_RPS ;
    h245Msg.Type2 = MSGTYP_CLC_ACK ;
    h245Msg.pData = (uint8*) & requestChannelCloseAck ;

    MessageSend(&h245Msg) ;
}


/************************************************************************/
/*  function name       : Se_ClcMsgCloseRjtSend                         */
/*  function outline    : RequestChannelCloseReject Send                */
/*  function discription: void Se_ClcMsgCloseRjtSend( PS_RccRejectCause )  */
/*  input data          : PS_RccRejectCause pCause_clcse                   */
/*  output data         : None                                          */
/*  draw time           : '96.11.19                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void CLC::MsgCloseRjtSend(LCEntry *pLcbEntry, PS_RccRejectCause pCause)
{
    S_RequestChannelCloseReject    requestChannelCloseReject ;
    S_H245Msg                      h245Msg ;

    requestChannelCloseReject.forwardLogicalChannelNumber = (uint16) pLcbEntry->FwdLcnRead() ;
    oscl_memcpy((int8*)&requestChannelCloseReject.rccRejectCause ,
                (int8*)pCause , sizeof(S_RccRejectCause)) ;

    h245Msg.Type1 = H245_MSG_RPS ;
    h245Msg.Type2 = MSGTYP_CLC_RJT ;
    h245Msg.pData = (uint8*) & requestChannelCloseReject ;

    MessageSend(&h245Msg) ;
}


/************************************************************************/
/*  function name       : Se_ClcMsgCloseRlsSend                         */
/*  function outline    : RequestChannelCloseRelease Send               */
/*  function discription: void Se_ClcMsgCloseRlsSend( void )            */
/*  input data          : None                                          */
/*  output data         : None                                          */
/*  draw time           : '96.11.19                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void CLC::MsgCloseRlsSend(LCEntry *pLcbEntry)
{
    S_RequestChannelCloseRelease   requestChannelCloseRelease ;
    S_H245Msg                      h245Msg ;

    requestChannelCloseRelease.forwardLogicalChannelNumber = (uint16) pLcbEntry->FwdLcnRead() ;

    h245Msg.Type1 = H245_MSG_IDC ;
    h245Msg.Type2 = MSGTYP_CLC_RLS ;
    h245Msg.pData = (uint8*) & requestChannelCloseRelease ;

    MessageSend(&h245Msg) ;
}


/************************************************************************/
/*  function name       : Se_ClcPtvCloseIdcSend                         */
/*  function outline    : CLOSE.indicaton Send                          */
/*  function discription: void Se_ClcPtvCloseIdcSend( void )            */
/*  input data          : None                                          */
/*  output data         : None                                          */
/*  draw time           : '96.11.19                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void CLC::PtvCloseIdcSend(LCEntry *pLcbEntry)
{
    S_InfHeader         header ;

    header.InfType = H245_PRIMITIVE ;
    header.InfId = E_PtvId_Clc_Cls_Idc ;
    header.InfSupplement1 = pLcbEntry->FwdLcnRead() ;
    header.InfSupplement2 = pLcbEntry->DirectionalRead() ;
    /* (UNI=1, BI=2) */
    header.pParameter = NULL ;
    header.Size = 0 ;

    PrimitiveSend(&header) ;
}


/************************************************************************/
/*  function name       : Se_ClcPtvRjtIdcSend                           */
/*  function outline    : REJECT.indication Send                        */
/*  function discription: void Se_ClcPtvRjtIdcSend( int32,                */
/*                                                   PS_RccRejectCause )   */
/*  input data          : int32 Source                                    */
/*                      : PS_RccRejectCause pCause_clcse                   */
/*  output data         : None                                          */
/*  draw time           : '96.11.19                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void CLC::PtvRjtIdcSend(LCEntry *pLcbEntry,
                        int32 Source ,
                        PS_RccRejectCause pCause)
{
    S_InfHeader        header ;
    S_SourceCause_Clc  sourceCause_Clc ;

    sourceCause_Clc.Source = (ENUM_Source)Source ;
    oscl_memcpy((int8*)&sourceCause_Clc.Cause , (int8*)pCause , sizeof(S_RccRejectCause)) ;

    header.InfType = H245_PRIMITIVE ;
    header.InfId = E_PtvId_Clc_Rjt_Idc ;

    /* Test for OutgoingIncoming not needed -- RAN */
    header.InfSupplement1 = pLcbEntry->FwdLcnRead() ;
    header.InfSupplement2 = 0 ;
    header.pParameter = (uint8*) & sourceCause_Clc ;
    header.Size = sizeof(S_SourceCause_Clc) ;

    PrimitiveSend(&header) ;

}


/************************************************************************/
/*  function name       : Se_ClcPtvCloseCfmSend                         */
/*  function outline    : SEND.confirm Send                             */
/*  function discription: void Se_ClcPtvCloseCfmSend( void )            */
/*  input data          : None                                          */
/*  output data         : None                                          */
/*  draw time           : '96.11.19                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void CLC::PtvCloseCfmSend(LCEntry *pLcbEntry)
{
    S_InfHeader    header ;

    header.InfType = H245_PRIMITIVE ;
    header.InfId = E_PtvId_Clc_Cls_Cfm ;
    header.InfSupplement1 = pLcbEntry->FwdLcnRead() ;
    header.InfSupplement2 = 0 ;
    header.pParameter = NULL ;
    header.Size = 0 ;

    PrimitiveSend(&header) ;

}

/************************************************************************/
/*  function name       : Se_ClcT108TimerIdGet                          */
/*  function outline    : T108 Timer Id Get                             */
/*  function discription: Se_ClcT108TimerIdGet( PS_LCB_ENTRY )          */
/*  input data          : PS_LCB_ENTRY p_LcbEntry                       */
/*  output data         : int32 Timer Id                                  */
/*  draw time           : '96.11.11                                     */
/*----------------------------------------------------------------------*/
/*  (x)                                                                 */
/*  This routine added for multiple CLCSEs.   RAN                       */
/************************************************************************/
int32 CLC::T108TimerIdGet(LCEntry *p_LcbEntry)
{
    /* Return T108 Timer Id  */
    return(((p_LcbEntry->FwdLcnRead()) << 16) | E_TmrId_Clc_T108) ;
}

/************************************************************************/
/*  function name       : Se_ClcT108TimerStart                          */
/*  function outline    : T108 Timer Start                              */
/*  function discription: void Se_ClcT108TimerStart( void )             */
/*  input data          : None                                          */
/*  output data         : None                                          */
/*  draw time           : '96.11.26                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void CLC::T108TimerStart(LCEntry *pLcbEntry)
{
    pLcbEntry->ClcTmrSqcNumberInc() ;
    RequestTimer(T108TimerIdGet(pLcbEntry),
                 pLcbEntry->ClcTmrSqcNumberRead(),
                 TimerDuration);
}


/************************************************************************/
/*  function name       : Se_ClcT108TimerStop                           */
/*  function outline    : T108 Timer Stop                               */
/*  function discription: void Se_ClcT108TimerStop( void )              */
/*  input data          : None                                          */
/*  output data         : None                                          */
/*  draw time           : '96.11.19                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void CLC::T108TimerStop(LCEntry *pLcbEntry)
{
    CancelTimer(T108TimerIdGet(pLcbEntry)) ;
}
