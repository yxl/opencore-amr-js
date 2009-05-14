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
/*  file name       : semr.c                                            */
/*  file contents   : Mode Request Signalling Entity Management         */
/*  draw            : '96.11.15                                         */
/*----------------------------------------------------------------------*/
/*  amendment       :                                                   */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/


/************************************************************************/
/*  Headerfile Include                                                  */
/************************************************************************/
#include "h245inf.h"
#include "semsgque.h"
#include "mr.h"


/************************************************************************/
/*  function name       : Se_Mr_0700_0000                               */
/*  function outline    : Event     TRANSFER.request                    */
/*                      : Status    Outgoing Idle                       */
/*  function discription: Se_Mr_0700_0000( PS_RequestMode )        */
/*  input data          : PS_RequestMode pModeElementParam         */
/*  output data         : None                                          */
/*  draw time           : '96.11.15                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void MR::_0700_0000(PS_RequestMode pModeElementParam)
{
    OutSqcInc() ;
    StatusWrite(MR_OUTGOING_AWTING_RPS) ;
    MsgRequestSend(pModeElementParam) ;
    T109TimerStart() ;
}


/************************************************************************/
/*  function name       : Se_Mr_0700_0010                               */
/*  function outline    : Event     TRANSFER.request                    */
/*                      : Status    Outgoing Idle                       */
/*  function discription: Se_Mr_0700_0010( PS_RequestMode )        */
/*  input data          : PS_RequestMode pModeElementParam         */
/*  output data         : None                                          */
/*  draw time           : '96.11.15                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void MR::_0700_0010(PS_RequestMode pModeElementParam)
{
    T109TimerStop() ;
    OutSqcInc() ;
    StatusWrite(MR_OUTGOING_AWTING_RPS) ;
    MsgRequestSend(pModeElementParam) ;
    T109TimerStart() ;
}


/************************************************************************/
/*  function name       : Se_Mr_0701_0001                               */
/*  function outline    : Event     RequestMode                         */
/*                      : Status    Incoming Idle                       */
/*  function discription: Se_Mr_0701_0001( PS_RequestMode )             */
/*  input data          : PS_RequestMode pRequestMode                   */
/*  output data         : None                                          */
/*  draw time           : '96.11.15                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void MR::_0701_0001(PS_RequestMode pRequestMode)
{
    InSqcWrite(pRequestMode->sequenceNumber) ;
    StatusWrite(MR_INCOMING_AWTING_RPS) ;
    PtvTrnsIndSend(pRequestMode) ;
}


/************************************************************************/
/*  function name       : Se_Mr_0701_0011                               */
/*  function outline    : Event     RequestMode                         */
/*                      : Status    Incoming Awaiting Response          */
/*  function discription: Se_Mr_0701_0011( PS_RequestMode )             */
/*  input data          : PS_RequestMode pRequestMode                   */
/*  output data         : None                                          */
/*  draw time           : '96.11.15                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void MR::_0701_0011(PS_RequestMode pRequestMode)
{
    S_RmRejectCause    cause;

    InSqcWrite(pRequestMode->sequenceNumber) ;
    cause.index = 0 ;  /* modeUnavailable */
    PtvRjtIndSend(Src_USER , &cause) ;
    PtvTrnsIndSend(pRequestMode) ;
}


/************************************************************************/
/*  function name       : Se_Mr_0702_0011                               */
/*  function outline    : Event     TRANSFER.response                   */
/*                      : Status    Incoming Awaiting Response          */
/*  function discription: Se_Mr_0702_0011( PS_Response pModePref )      */
/*  input data          : PS_Response pModePref                         */
/*  output data         : None                                          */
/*  draw time           : '96.11.15                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void MR::_0702_0011(PS_Response pModePref)
{
    StatusWrite(MR_INCOMING_IDLE) ;
    MsgAckSend(pModePref) ;
}


/************************************************************************/
/*  function name       : Se_Mr_0703_0011                               */
/*  function outline    : Event     REJECT.request                      */
/*                      : Status    Incoming Awaiting Response          */
/*  function discription: Se_Mr_0703_0011( PS_RmRejectCause )                */
/*  input data          : PS_RmRejectCause pCause_Mr                         */
/*  output data         : None                                          */
/*  draw time           : '96.11.15                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void MR::_0703_0011(PS_RmRejectCause pCause)
{
    StatusWrite(MR_INCOMING_IDLE) ;
    MsgRjtSend(pCause) ;
}


/************************************************************************/
/*  function name       : Se_Mr_0704_0010                               */
/*  function outline    : Event     RequestModeAck                      */
/*                      : Status    Outgoing Awaiting Response          */
/*  function discription: Se_Mr_0704_0010( PS_RequestModeAck )          */
/*  input data          : PS_RequestModeAck pRequestModeAck             */
/*  output data         : None                                          */
/*  draw time           : '96.11.15                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void MR::_0704_0010(PS_RequestModeAck pRequestModeAck)
{
    if (pRequestModeAck->sequenceNumber == OutSqcRead())    /* Receive Sequence Number Equal Outgoing Sequence Number */
    {
        T109TimerStop() ;
        StatusWrite(MR_OUTGOING_IDLE) ;
        PtvTrnsCfmSend(pRequestModeAck) ;
    }
}


/************************************************************************/
/*  function name       : Se_Mr_0705_0010                               */
/*  function outline    : Event     RequestModeReject                   */
/*                      : Status    Outgoing Awaiting Response          */
/*  function discription: Se_Mr_0705_0010                               */
/*                      :            ( PS_RequestModeReject )           */
/*  input data          : PS_RequestModeReject pRequestModeReject       */
/*  output data         : None                                          */
/*  draw time           : '96.11.15                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void MR::_0705_0010(PS_RequestModeReject pRequestModeReject)
{
    if (pRequestModeReject->sequenceNumber == OutSqcRead())    /* Receive Sequence Number Equal Outgoing Sequence Number */
    {
        T109TimerStop() ;
        StatusWrite(MR_OUTGOING_IDLE) ;
        PtvRjtIndSend(Src_USER , &pRequestModeReject->rmRejectCause) ;
    }
}


/************************************************************************/
/*  function name       : Se_Mr_0706_0010                               */
/*  function outline    : Event     Timer T109 Timeout                  */
/*                      : Status    Outgoing Awaiting Response          */
/*  function discription: Se_Mr_0706_0010( int32 )                        */
/*  input data          : int32 Timer_Sequence_Number                     */
/*  output data         : None                                          */
/*  draw time           : '96.11.15                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void MR::_0706_0010(int32 Timer_Sequence_Number)
{
    S_RmRejectCause    cause;

    if (Timer_Sequence_Number == TmrSqcNumberRead())    /* Receive Timer Sequense Number Equal Timer Sequence Number */
    {
        StatusWrite(MR_OUTGOING_IDLE) ;
        MsgRlsSend() ;
        cause.index = 0 ; /* modeUnavailable */
        PtvRjtIndSend(Src_PROTOCOL , &cause) ;
    }
}


/************************************************************************/
/*  function name       : Se_Mr_0707_0011                               */
/*  function outline    : Event     RequestModeRelease                  */
/*                      : Status    Incoming Awaiting Response          */
/*  function discription: Se_Mr_0707_0011( void )                       */
/*  input data          : None                                          */
/*  output data         : None                                          */
/*  draw time           : '96.11.15                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void MR::_0707_0011(void)
{
    S_RmRejectCause    cause;

    cause.index = 0 ;  /* modeUnavailable */
    StatusWrite(MR_INCOMING_IDLE) ;
    PtvRjtIndSend(Src_PROTOCOL , &cause) ;
}


/************************************************************************/
/*  function name       : Se_MrMsgRequestSend                           */
/*  function outline    : RequestMode Send                              */
/*  function discription: void Se_MrMsgRequestSend(                     */
/*                                                PS_RequestMode ) */
/*  input data          : PS_RequestMode pModeElementParam         */
/*  output data         : None                                          */
/*  draw time           : '96.11.15                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void MR::MsgRequestSend(PS_RequestMode pModeElementParam)
{
    S_RequestMode    requestMode ;
    S_H245Msg        h245Msg ;

    requestMode.sequenceNumber = (uint8) OutSqcRead() ;
    requestMode.size_of_requestedModes = pModeElementParam->size_of_requestedModes ;
    requestMode.requestedModes = pModeElementParam->requestedModes ;

    h245Msg.Type1 = H245_MSG_REQ ;
    h245Msg.Type2 = MSGTYP_MR ;
    h245Msg.pData = (uint8*) & requestMode ;

    MessageSend(&h245Msg) ;
}


/************************************************************************/
/*  function name       : Se_MrMsgAckSend                               */
/*  function outline    : RequestModeAck Send                           */
/*  function discription: void Se_MrMsgAckSend( PS_Response )           */
/*  input data          : PS_Response pModePref                         */
/*  output data         : None                                          */
/*  draw time           : '96.11.15                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void MR::MsgAckSend(PS_Response pModePref)
{
    S_RequestModeAck requestModeAck ;
    S_H245Msg        h245Msg ;

    requestModeAck.sequenceNumber = (uint8) InSqcRead() ;
    oscl_memcpy(&requestModeAck.response, pModePref, sizeof(S_Response)) ;

    h245Msg.Type1 = H245_MSG_RPS ;
    h245Msg.Type2 = MSGTYP_MR_ACK ;
    h245Msg.pData = (uint8*) & requestModeAck ;

    MessageSend(&h245Msg) ;
}


/************************************************************************/
/*  function name       : Se_MrMsgRjtSend                               */
/*  function outline    : RequestModeReject Send                        */
/*  function discription: Se_MrMsgRjtSend( PS_RmRejectCause )                */
/*  input data          : PS_RmRejectCause pCause                            */
/*  output data         : None                                          */
/*  draw time           : '96.11.15                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void MR::MsgRjtSend(PS_RmRejectCause pCause)
{
    S_RequestModeReject        requestModeReject ;
    S_H245Msg                  h245Msg ;

    requestModeReject.sequenceNumber = (uint8) InSqcRead() ;
    oscl_memcpy((int8*)&requestModeReject.rmRejectCause , (int8*)pCause , sizeof(S_RmRejectCause)) ;

    h245Msg.Type1 = H245_MSG_RPS ;
    h245Msg.Type2 = MSGTYP_MR_RJT ;
    h245Msg.pData = (uint8*) & requestModeReject ;

    MessageSend(&h245Msg) ;
}


/************************************************************************/
/*  function name       : MsgRlsSend                               */
/*  function outline    : RequestModeRelease Send                       */
/*  function discription: Se_MrMsgRlsSend( void )                       */
/*  input data          : None                                          */
/*  output data         : None                                          */
/*  draw time           : '96.11.15                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void MR::MsgRlsSend(void)
{
    S_RequestModeRelease requestModeRelease ;
    S_H245Msg            h245Msg ;

    oscl_memset((int8*)&requestModeRelease  , 0 , sizeof(S_RequestModeRelease)) ;

    h245Msg.Type1 = H245_MSG_IDC ;
    h245Msg.Type2 = MSGTYP_MR_RLS ;
    h245Msg.pData = (uint8*) & requestModeRelease ;

    MessageSend(&h245Msg) ;
}


/************************************************************************/
/*  function name       : PtvTrnsIndSend                           */
/*  function outline    : TRANSFER.indicaton Send                       */
/*  function discription: Se_MrPtvTrnsIndSend( PS_RequestMode )         */
/*  input data          : PS_RequestMode pRequestMode                   */
/*  output data         : None                                          */
/*  draw time           : '96.11.15                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void MR::PtvTrnsIndSend(PS_RequestMode pRequestMode)
{
    S_InfHeader    header ;

    header.InfType = H245_PRIMITIVE ;
    header.InfId = E_PtvId_Mr_Trf_Idc ;
    header.InfSupplement1 = 0 ;
    header.InfSupplement2 = 0 ;
    header.pParameter = (uint8*)pRequestMode ;
    header.Size = sizeof(S_RequestMode) ;

    PrimitiveSend(&header) ;
}


/************************************************************************/
/*  function name       : Se_MrPtvRjtIndSend                            */
/*  function outline    : REJECT.indication Send                        */
/*  function discription: Se_MrPtvRjtIndSend( int32 , PS_RmRejectCause )     */
/*  input data          : int32 Source                                    */
/*                      : PS_RmRejectCause pCause                          */
/*  output data         : None                                          */
/*  draw time           : '96.11.15                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void MR::PtvRjtIndSend(int32 Source , PS_RmRejectCause pCause)
{
    S_InfHeader        header ;
    S_SourceCause_Mr   sourceCause_Mr ;

    sourceCause_Mr.Source = (ENUM_Source)Source ;
    oscl_memcpy((int8*)&sourceCause_Mr.Cause , (int8*)pCause , sizeof(S_RmRejectCause)) ;

    header.InfType = H245_PRIMITIVE ;
    header.InfId = E_PtvId_Mr_Rjt_Idc ;
    header.InfSupplement1 = 0 ;
    header.InfSupplement2 = 0 ;
    header.pParameter = (uint8*) & sourceCause_Mr ;
    header.Size = sizeof(S_SourceCause_Mr) ;

    PrimitiveSend(&header) ;
}


/************************************************************************/
/*  function name       : Se_MrPtvTrnsCfmSend                           */
/*  function outline    : TRANSFER.confirm Send                         */
/*  function discription: Se_MrPtvTrnsCfmSend( PS_RequestModeAck )      */
/*  input data          : PS_RequestModeAck pRequestModeAck             */
/*  output data         : None                                          */
/*  draw time           : '96.11.15                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void MR::PtvTrnsCfmSend(PS_RequestModeAck pRequestModeAck)
{
    S_InfHeader    header ;
    S_Response     modePref ;

    oscl_memcpy((int8*)&modePref , (int8*)&pRequestModeAck->response , sizeof(S_Response)) ;

    header.InfType = H245_PRIMITIVE ;
    header.InfId = E_PtvId_Mr_Trf_Cfm ;
    header.InfSupplement1 = 0 ;
    header.InfSupplement2 = 0 ;
    header.pParameter = (uint8*) & modePref ;
    header.Size = sizeof(S_Response) ;

    PrimitiveSend(&header) ;
}


#ifdef PVANALYZER /* --------SE Analyzer Tool -------- */

#define ANALYZER_SE 0x0020		// (Assume tag is fixed)
void Show245(uint16 tag, uint16 indent, char* inString);

// =========================================================
// Se_MrStatusShow()
//
// This function displays state transition information for
// the MR signaling entity.
// =========================================================
void MR::StatusShow(uint8 oldStatus, uint8 newStatus)
{
    char tempString[80];

    Show245(ANALYZER_SE, 0, "MRSE State Transition:");
    sprintf(tempString, "  from--> %s", StateLabel(oldStatus));
    Show245(ANALYZER_SE, 0, tempString);
    sprintf(tempString, "    to--> %s", StateLabel(newStatus));
    Show245(ANALYZER_SE, 0, tempString);
    Show245(ANALYZER_SE, 0, " ");
}

// ==========================================================
// Se_MrStateLabel()
//
// Returns a pointer to an approprate state label string.
// ==========================================================
char* MR::StateLabel(uint8 status)
{
    switch (status)
    {
        case MR_OUTGOING_IDLE:
            return("Outgoing IDLE");
            break;
        case MR_OUTGOING_AWTING_RPS:
            return("Outgoing AWAITING RESPONSE");
            break;
        case MR_INCOMING_IDLE:
            return("Incoming IDLE");
            break;
        case MR_INCOMING_AWTING_RPS:
            return("Incoming AWAITING RESPONSE");
            break;
        default:
            return("UNKNOWN STATE");
    }
}
#endif            /* --------------------------------- */


/************************************************************************/
/*  function name       : Se_MrT109TimerStart                           */
/*  function outline    : T109 Timer Start                              */
/*  function discription: void Se_MrT109TimerStart( void )              */
/*  input data          : None                                          */
/*  output data         : None                                          */
/*  draw time           : '96.11.26                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void MR::T109TimerStart(void)
{
    TmrSqcNumberInc() ;
    RequestTimer(E_TmrId_Mr_T109 , TmrSqcNumberRead() ,
                 TimerDuration) ;
}


/************************************************************************/
/*  function name       : Se_MrT109TimerStop                            */
/*  function outline    : T109 Timer Stop                               */
/*  function discription: void Se_MrT109TimerStop( void )               */
/*  input data          : None                                          */
/*  output data         : None                                          */
/*  draw time           : '96.11.15                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void MR::T109TimerStop(void)
{
    CancelTimer(E_TmrId_Mr_T109) ;
}
