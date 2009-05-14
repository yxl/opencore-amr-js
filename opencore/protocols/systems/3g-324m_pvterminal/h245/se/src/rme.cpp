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
/*  file name       : serme.c                                           */
/*  file contents   : Request Multiplex Entry                           */
/*                  :                      Signalling Entity Management */
/*  draw            : '96.11.26                                         */
/*----------------------------------------------------------------------*/
/*  amendment       :                                                   */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/


/************************************************************************/
/*  Headerfile Include                                                  */
/************************************************************************/
#include "h245inf.h"
#include "semsgque.h"
#include "rme.h"


/************************************************************************/
/*  function name       : Se_Rme_0600_0000                              */
/*  function outline    : Event     SEND.request                        */
/*                      : Status    Outgoing Idle                       */
/*  function discription: void Se_Rme_0600_0000( void )                 */
/*  input data          : None                                          */
/*  output data         : None                                          */
/*  draw time           : '96.11.19                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void RME::_0600_0000(void)
{
    StatusWrite(RME_OUTGOING_AWTING_RPS) ;
    MsgEntrySend() ;
    T107TimerStart() ;
}


/************************************************************************/
/*  function name       : Se_Rme_0601_0001                              */
/*  function outline    : Event     RequestMultiplexEntry               */
/*                      : Status    Incoming Idle                       */
/*  function discription: void Se_Rme_0601_0001(                        */
/*                                          PS_RequestMultiplexEntry )  */
/*  input data          : PS_RequestMultiplexEntry                      */
/*                                              pRequestMultiplexEntry  */
/*  output data         : None                                          */
/*  draw time           : '96.11.19                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void RME::_0601_0001(PS_RequestMultiplexEntry pRequestMultiplexEntry)
{
    InRMEntries.Write(pRequestMultiplexEntry->size_of_entryNumbers,
                      pRequestMultiplexEntry->entryNumbers);
    StatusWrite(RME_INCOMING_AWTING_RPS) ;
    PtvSendIndSend() ;
}


/************************************************************************/
/*  function name       : Se_Rme_0601_0011                              */
/*  function outline    : Event     RequestMultiplexEntry               */
/*                      : Status    Incoming Awaiting Response          */
/*  function discription: void Se_Rme_0601_0011(                        */
/*                                          PS_RequestMultiplexEntry )  */
/*  input data          : PS_RequestMultiplexEntry                      */
/*                                              pRequestMultiplexEntry  */
/*  output data         : None                                          */
/*  draw time           : '96.11.19                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void RME::_0601_0011(PS_RequestMultiplexEntry pRequestMultiplexEntry)
{
    S_RmeRejectCause    cause;

    cause.index = 0 ;  /* unspecifiedCause */
    PtvRjtIdcSend(Src_USER , &cause) ;
    InRMEntries.Write(pRequestMultiplexEntry->size_of_entryNumbers,
                      pRequestMultiplexEntry->entryNumbers);

    PtvSendIndSend() ;
}


/************************************************************************/
/*  function name       : Se_Rme_0602_0011                              */
/*  function outline    : Event     SEND.response                       */
/*                      : Status    Incoming Awaiting Response          */
/*  function discription: void Se_Rme_0602_0011( void )                 */
/*  input data          : None                                          */
/*  output data         : None                                          */
/*  draw time           : '96.11.19                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void RME::_0602_0011(void)
{
    StatusWrite(RME_INCOMING_IDLE) ;
    MsgEntryAckSend() ;
}


/************************************************************************/
/*  function name       : Se_Rme_0603_0011                              */
/*  function outline    : Event     REJECT.request                      */
/*                      : Status    Incoming Awaiting Response          */
/*  function discription: void Se_Rme_0603_0011( PS_RmeRejectCause )         */
/*  input data          : PS_RmeRejectCause pCause_Rme                       */
/*  output data         : None                                          */
/*  draw time           : '96.11.19                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void RME::_0603_0011(PS_RmeRejectCause pCause_Rme)
{
    StatusWrite(RME_INCOMING_IDLE) ;
    MsgEntryRjtSend(pCause_Rme) ;
}


/************************************************************************/
/*  function name       : Se_Rme_0604_0010                              */
/*  function outline    : Event     RequestMultiplexEntryAck            */
/*                      : Status    Outgoing Awaiting Response          */
/*  function discription: void Se_Rme_0604_0010(                        */
/*                                        PS_RequestMultiplexEntryAck ) */
/*  input data          : PS_RequestMultiplexEntryAck                   */
/*                                           pRequestMultiplexEntryAck  */
/*  output data         : None                                          */
/*  draw time           : '96.11.19                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void RME::_0604_0010(PS_RequestMultiplexEntryAck pRequestMultiplexEntryAck)
{
    OSCL_UNUSED_ARG(pRequestMultiplexEntryAck);
    T107TimerStop() ;
    StatusWrite(RME_OUTGOING_IDLE) ;
    PtvSendCfmSend() ;
}


/************************************************************************/
/*  function name       : Se_Rme_0605_0010                              */
/*  function outline    : Event     RequestMultiplexEntryReject         */
/*                      : Status    Outgoing Awaiting Response          */
/*  function discription: void Se_Rme_0605_0010                         */
/*                      :            ( PS_RequestMultiplexEntryReject ) */
/*  input data          : PS_RequestMultiplexEntryReject                */
/*                                        pRequestMultiplexEntryReject  */
/*  output data         : None                                          */
/*  draw time           : '96.11.19                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void RME::_0605_0010(PS_RequestMultiplexEntryReject pRequestMultiplexEntryReject)
{
    T107TimerStop() ;
    StatusWrite(RME_OUTGOING_IDLE) ;
    PtvRjtIdcSend(Src_USER , &pRequestMultiplexEntryReject->rejectionDescriptions->rmeRejectCause) ;
}


/************************************************************************/
/*  function name       : Se_Rme_0606_0010                              */
/*  function outline    : Event     Timer T107 Timeout                  */
/*                      : Status    Outgoing Awaiting Response          */
/*  function discription: void Se_Rme_0606_0010( int32 )                  */
/*  input data          : int32 Timer_Sequence_Number                     */
/*  output data         : None                                          */
/*  draw time           : '96.11.19                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void RME::_0606_0010(int32 Timer_Sequence_Number)
{
    S_RmeRejectCause    cause;

    if (Timer_Sequence_Number == TmrSqcNumberRead())    /* Receive Timer Sequense Number Equal Timer Sequence Number */
    {
        StatusWrite(RME_OUTGOING_IDLE) ;
        MsgEntryRlsSend() ;
        cause.index = 0 ;  /* unspecifiedCause */
        PtvRjtIdcSend(Src_PROTOCOL , &cause) ;
    }
}

/************************************************************************/
/*  function name       : Se_Rme_0607_0011                              */
/*  function outline    : Event     RequestMultiplexEntryRelease        */
/*                      : Status    Incoming Awaiting Response          */
/*  function discription: void Se_Rme_0607_0011( void )                 */
/*  input data          : None                                          */
/*  output data         : None                                          */
/*  draw time           : '96.11.19                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void RME::_0607_0011(PS_RequestMultiplexEntryRelease pRequestMultiplexEntryRelease)
{
    S_RmeRejectCause    cause;

    OSCL_UNUSED_ARG(pRequestMultiplexEntryRelease);

    cause.index = 0 ;  /* unspecifiedCause */
    StatusWrite(RME_INCOMING_IDLE) ;
    PtvRjtIdcSend(Src_PROTOCOL , &cause) ;
}


/************************************************************************/
/*  function name       : Se_RmeMsgEntrySend                            */
/*  function outline    : RequestMultiplexEntry Send                    */
/*  function discription: void Se_RmeMsgEntrySend( void )               */
/*  input data          : None                                          */
/*  output data         : None                                          */
/*  draw time           : '96.11.19                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void RME::MsgEntrySend(void)
{
    S_RequestMultiplexEntry		requestMultiplexEntry;
    uint32						entryNumbers[15];
    S_H245Msg					h245Msg;

    requestMultiplexEntry.size_of_entryNumbers = (uint16) OutRMEntries.SizeRead();
    OutRMEntries.EntriesCopy(entryNumbers);
    requestMultiplexEntry.entryNumbers = entryNumbers;

    h245Msg.Type1 = H245_MSG_REQ ;
    h245Msg.Type2 = MSGTYP_RME ;
    h245Msg.pData = (uint8*) & requestMultiplexEntry ;

    MessageSend(&h245Msg) ;
}


/************************************************************************/
/*  function name       : Se_RmeMsgEntryAckSend                         */
/*  function outline    : RequestMultiplexEntryAck Send                 */
/*  function discription: void Se_RmeMsgEntryAckSend( void )            */
/*  input data          : None                                          */
/*  output data         : None                                          */
/*  draw time           : '96.11.19                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void RME::MsgEntryAckSend(void)
{
    S_RequestMultiplexEntryAck	requestMultiplexEntryAck ;
    uint32						entryNumbers[15];
    S_H245Msg					h245Msg ;

    requestMultiplexEntryAck.size_of_entryNumbers = (uint16) InRMEntries.SizeRead();
    InRMEntries.EntriesCopy(entryNumbers);
    requestMultiplexEntryAck.entryNumbers = entryNumbers;

    h245Msg.Type1 = H245_MSG_RPS ;
    h245Msg.Type2 = MSGTYP_RME_ACK ;
    h245Msg.pData = (uint8*) & requestMultiplexEntryAck ;

    MessageSend(&h245Msg) ;

    return ;
}


/************************************************************************/
/*  function name       : Se_RmeMsgEntryRjtSend                         */
/*  function outline    : RequestMultiplexEntryReject Send              */
/*  function discription: void Se_RmeMsgEntryRjtSend( PS_RmeRejectCause )  */
/*  input data          : PS_RmeRejectCause pCause_rmese                   */
/*  output data         : None                                          */
/*  draw time           : '96.11.19                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void RME::MsgEntryRjtSend(PS_RmeRejectCause pCause_rmese)
{
    S_RequestMultiplexEntryReject					requestMultiplexEntryReject ;
    uint32											entryNumbers[15];
    S_RequestMultiplexEntryRejectionDescriptions	requestMultiplexEntryRejectionDescriptions ;
    S_H245Msg										h245Msg ;

    requestMultiplexEntryReject.size_of_entryNumbers = (uint16) InRMEntries.SizeRead();
    InRMEntries.EntriesCopy(entryNumbers);
    requestMultiplexEntryReject.entryNumbers = entryNumbers;

    requestMultiplexEntryReject.size_of_rejectionDescriptions = 1 ;

    requestMultiplexEntryRejectionDescriptions.multiplexTableEntryNumber = 1 ;
    oscl_memcpy((int8*)&requestMultiplexEntryRejectionDescriptions.rmeRejectCause ,
                (int8*)pCause_rmese, sizeof(S_RmeRejectCause)) ;

    requestMultiplexEntryReject.rejectionDescriptions = &requestMultiplexEntryRejectionDescriptions ;

    h245Msg.Type1 = H245_MSG_RPS ;
    h245Msg.Type2 = MSGTYP_RME_RJT ;
    h245Msg.pData = (uint8*) & requestMultiplexEntryReject ;

    MessageSend(&h245Msg) ;
}


/************************************************************************/
/*  function name       : Se_RmeMsgEntryRlsSend                         */
/*  function outline    : RequestMultiplexEntryRelease Send             */
/*  function discription: void Se_RmeMsgEntryRlsSend( void )            */
/*  input data          : None                                          */
/*  output data         : None                                          */
/*  draw time           : '96.11.19                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void RME::MsgEntryRlsSend(void)
{
    S_RequestMultiplexEntryRelease	requestMultiplexEntryRelease;
    uint32							entryNumbers[15];
    S_H245Msg						h245Msg;

    requestMultiplexEntryRelease.size_of_entryNumbers = (uint16) OutRMEntries.SizeRead();
    OutRMEntries.EntriesCopy(entryNumbers);
    requestMultiplexEntryRelease.entryNumbers = entryNumbers;

    h245Msg.Type1 = H245_MSG_IDC ;
    h245Msg.Type2 = MSGTYP_RME_RLS ;
    h245Msg.pData = (uint8*) & requestMultiplexEntryRelease ;

    MessageSend(&h245Msg) ;
}


/************************************************************************/
/*  function name       : Se_RmePtvSendIndSend                          */
/*  function outline    : SEND.indicaton Send                           */
/*  function discription: void Se_RmePtvSendIndSend( void )             */
/*  input data          : None                                          */
/*  output data         : None                                          */
/*  draw time           : '96.11.19                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void RME::PtvSendIndSend(void)
{
    S_InfHeader         header ;

    header.InfType = H245_PRIMITIVE ;
    header.InfId = E_PtvId_Rme_Send_Idc ;
    header.InfSupplement1 = 0 ;
    header.InfSupplement2 = 0 ;
    header.pParameter = NULL ;
    header.Size = 0 ;

    PrimitiveSend(&header) ;
}


/************************************************************************/
/*  function name       : PtvRjtIdcSend                           */
/*  function outline    : REJECT.indication Send                        */
/*  function discription: void PtvRjtIdcSend( int32,                */
/*                                                   PS_RmeRejectCause )   */
/*  input data          : int32 Source                                    */
/*                      : PS_RmeRejectCause pCause_rmese                   */
/*  output data         : None                                          */
/*  draw time           : '96.11.19                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void RME::PtvRjtIdcSend(int32 Source , PS_RmeRejectCause pCause)
{
    S_InfHeader        header ;
    S_SourceCause_Rme  sourceCause ;

    sourceCause.Source = (ENUM_Source)Source ;
    oscl_memcpy((int8*)&sourceCause.Cause , (int8*)pCause , sizeof(S_RmeRejectCause)) ;

    header.InfType = H245_PRIMITIVE ;
    header.InfId = E_PtvId_Rme_Rjt_Idc ;
    header.InfSupplement1 = 0 ;
    header.InfSupplement2 = 0 ;
    header.pParameter = (uint8*) & sourceCause ;
    header.Size = sizeof(S_SourceCause_Rme) ;

    PrimitiveSend(&header) ;
}


/************************************************************************/
/*  function name       : Se_RmePtvSendCfmSend                          */
/*  function outline    : SEND.confirm Send                             */
/*  function discription: void Se_RmePtvSendCfmSend( void )             */
/*  input data          : None                                          */
/*  output data         : None                                          */
/*  draw time           : '96.11.19                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void RME::PtvSendCfmSend(void)
{
    S_InfHeader    header ;

    header.InfType = H245_PRIMITIVE ;
    header.InfId = E_PtvId_Rme_Send_Cfm ;
    header.InfSupplement1 = 0 ;
    header.InfSupplement2 = 0 ;
    header.pParameter = NULL ;
    header.Size = 0 ;

    PrimitiveSend(&header) ;
}


#ifdef PVANALYZER /* --------SE Analyzer Tool -------- */

#define ANALYZER_SE 0x0020		// (Assume tag is fixed)
void Show245(uint16 tag, uint16 indent, char *inString);

// =========================================================
// Se_RmeStatusShow()
//
// This function displays state transition information for
// the RME signaling entity.
// =========================================================
void RME::StatusShow(uint8 oldStatus, uint8 newStatus)
{
    char tempString[80];

    Show245(ANALYZER_SE, 0, "RMESE State Transition:");
    sprintf(tempString, "  from--> %s", StateLabel(oldStatus));
    Show245(ANALYZER_SE, 0, tempString);
    sprintf(tempString, "    to--> %s", StateLabel(newStatus));
    Show245(ANALYZER_SE, 0, tempString);
    Show245(ANALYZER_SE, 0, " ");
}

// ==========================================================
// Se_RmeStateLabel()
//
// Returns a pointer to an approprate state label string.
// ==========================================================
char* RME::StateLabel(uint8 status)
{
    switch (status)
    {
        case RME_OUTGOING_IDLE:
            return("Outgoing IDLE");
            break;
        case RME_OUTGOING_AWTING_RPS:
            return("Outgoing AWAITING RESPONSE");
            break;
        case RME_INCOMING_IDLE:
            return("Incoming IDLE");
            break;
        case RME_INCOMING_AWTING_RPS:
            return("Incoming AWAITING RESPONSE");
            break;
        default:
            return("UNKNOWN STATE");
    }
}
#endif            /* --------------------------------- */



/************************************************************************/
/*  function name       : Se_RmeT107TimerStart                          */
/*  function outline    : T107 Timer Start                              */
/*  function discription: void Se_RmeT107TimerStart( void )             */
/*  input data          : None                                          */
/*  output data         : None                                          */
/*  draw time           : '96.11.26                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void RME::T107TimerStart(void)
{
    TmrSqcNumberInc() ;
    RequestTimer(E_TmrId_Rme_T107 , TmrSqcNumberRead() ,
                 TimerDuration) ;
}


/************************************************************************/
/*  function name       : Se_RmeT107TimerStop                           */
/*  function outline    : T107 Timer Stop                               */
/*  function discription: void Se_RmeT107TimerStop( void )              */
/*  input data          : None                                          */
/*  output data         : None                                          */
/*  draw time           : '96.11.19                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void RME::T107TimerStop(void)
{
    CancelTimer(E_TmrId_Rme_T107) ;
}
