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
/*  file name       : semt.c                                            */
/*  file contents   : Multiplex Table Signalling Entity Management      */
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
#include "mt.h"


MultiplexEntrySendMgr::MultiplexEntrySendMgr(int32 sn, MultiplexEntrySendUtility* util)
{
    iSn = sn;
    iUtil = util;
}

void MultiplexEntrySendMgr::Write(PS_MultiplexEntryDescriptor descriptors, int32 size)
{
    iOutMTEntries.Write(descriptors, size);
}

void MultiplexEntrySendMgr::TransferRequest(PS_MuxDescriptor p_MuxDescriptor)
{
    iOutMTEntries.Clear();
    iOutMTEntries.Write(p_MuxDescriptor->multiplexEntryDescriptors ,
                        p_MuxDescriptor->size_of_multiplexEntryDescriptors);
    StatusWrite(MT_OUTGOING_AWTING_RPS) ;
    iUtil->MsgMtSend(p_MuxDescriptor, (uint8)iSn) ;
    T104TimerStart() ;
}

void MultiplexEntrySendMgr::MultiplexEntrySendAck(PS_MultiplexEntrySendAck p_MultiplexEntrySendAck)
{
    T104TimerStop() ;
    StatusWrite(MT_OUTGOING_IDLE) ;
    iUtil->PtvTrfCfmSend(p_MultiplexEntrySendAck->sequenceNumber) ;
}

void MultiplexEntrySendMgr::MultiplexEntrySendReject(PS_MultiplexEntrySendReject p_MultiplexEntrySendReject)
{
    T104TimerStop() ;
    StatusWrite(MT_OUTGOING_IDLE) ;
    iUtil->PtvRjtIdcSend(S_InfHeader::OUTGOING, Src_USER ,
                         &p_MultiplexEntrySendReject->rejectionDescriptions->meRejectCause,
                         p_MultiplexEntrySendReject->sequenceNumber) ;
}

void MultiplexEntrySendMgr::T104Timeout()
{
//	Print("MultiplexEntrySendMgr::TimeoutOccurred Error: T104 timer timed out for sn(%d)",iSn);
    StatusWrite(MT_OUTGOING_IDLE) ;
    iUtil->MsgMtRlsSend(iOutMTEntries) ;
    iUtil->PtvRjtIdcSend(S_InfHeader::OUTGOING, Src_PROTOCOL , NULL, iSn) ;
}

void MultiplexEntrySendMgr::StatusWrite(uint32 status)
{
    iStatus = status;
}

void MultiplexEntrySendMgr::T104TimerStart(void)
{
    iUtil->RequestT104Timer(iSn);
}

void MultiplexEntrySendMgr::T104TimerStop(void)
{
    iUtil->CancelT104Timer(iSn);
}

MT::MT() : SEBase(), iTimer("MultiplexTables")
{
    Reset();
    /* Initialize timer */
    iTimer .SetFrequency(1);
    iTimer.SetObserver(this);
}

void MT::Reset()
{
    OutSqcClear() ;
    InSqcClear() ;
    iTimer.Clear() ;
    InMTEntries.Clear();
    StatusWrite(MT_INCOMING_IDLE) ;

    PendingMtSendMap::iterator it = iPendingMtSend.begin();
    while (it != iPendingMtSend.end())
    {
        //OSCL_DELETE((*it++).second);
        delete(*it++).second;
    }
    iPendingMtSend.clear();
}

void MT::RequestT104Timer(int32 sn)
{
    iTimer.Request(sn, sn, T104_TIMER_DURATION, this);
}

void MT::CancelT104Timer(int32 sn)
{
    iTimer.Cancel(sn);
}

void MT::TimeoutOccurred(int32 timerID, int32 timeoutInfo)
{
    Print("MT::TimeoutOccurred Error: T104 timer timed out for timerID(%d), timeoutInfo(%d)", timerID, timeoutInfo);
    PendingMtSendMap::iterator iter = iPendingMtSend.find(timerID);
    if (iter == iPendingMtSend.end())
    {
        Print("MT::TimeoutOccurred   Error: Failed to lookup pending MultiplexEntrySend for sequence number %d", timerID);
        return;
    }
    (*iter).second->T104Timeout();
    OSCL_DELETE((*iter).second);
    iPendingMtSend.erase(iter);
}

/************************************************************************/
/*  function name       : Se_Mt_0500_0000                               */
/*  function outline    : Event     TRANSFER.request                    */
/*                      : Status    Outgoing Idle                       */
/*  function discription: Se_Mt_0500_0000( PS_MuxDescriptor )           */
/*  input data          : PS_MuxDescriptor p_MuxDescriptor              */
/*  output data         : None                                          */
/*  draw time           : '96.10.31                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void MT::TransferRequest(PS_MuxDescriptor p_MuxDescriptor)
{
    //MultiplexEntrySendMgr* new_mesend_mgr= OSCL_NEW(MultiplexEntrySendMgr , (OutSqcRead(),this));
    MultiplexEntrySendMgr* new_mesend_mgr = new MultiplexEntrySendMgr(OutSqcRead(), this);
    iPendingMtSend.insert(PendingMtSendMap::value_type(OutSqcRead(), new_mesend_mgr));
    new_mesend_mgr->TransferRequest(p_MuxDescriptor);
    OutSqcInc() ;
}


/************************************************************************/
/*  function name       : Se_Mt_0501_0001                               */
/*  function outline    : Event     MultiplexEntrySend                  */
/*                      : Status    Incoming Idle                       */
/*  function discription: Se_Mt_0501_0001( PS_MultiplexEntrySend )      */
/*  input data          : PS_MultiplexEntrySend p_MultiplexEntrySend    */
/*  output data         : None                                          */
/*  draw time           : '96.10.31                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void MT::_0501_0001(PS_MultiplexEntrySend p_MultiplexEntrySend)
{
    InSqcWrite(p_MultiplexEntrySend->sequenceNumber) ;
    // ---------------------------------------------(RAN-MT)---
    // No need to record incoming InEnum;
    // The entries will be passed to TSC and returned.
    //Se_MtInEnumClear(  ) ;
    //Se_MtInEnumWrite( p_MultiplexEntrySend->multiplexEntryDescriptors,
    //                  p_MultiplexEntrySend->size_of_multiplexEntryDescriptors ) ;
    // --------------------------------------------------------
    StatusWrite(MT_INCOMING_AWTING_RPS) ;
    PtvTrfIdcSend(p_MultiplexEntrySend) ;
    /* WWUSGW: 06/22/01 move the following line above the indication send
    Se_MtStatusWrite( MT_INCOMING_AWTING_RPS ) ;
    */
}


/************************************************************************/
/*  function name       : Se_Mt_0501_0011                               */
/*  function outline    : Event     MultiplexEntrySend                  */
/*                      : Status    Incoming Awaiting Response          */
/*  function discription: Se_Mt_0501_0011( PS_MultiplexEntrySend )      */
/*  input data          : PS_MultiplexEntrySend p_MultiplexEntrySend    */
/*  output data         : None                                          */
/*  draw time           : '96.10.31                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void MT::_0501_0011(PS_MultiplexEntrySend p_MultiplexEntrySend)
{
    InSqcWrite(p_MultiplexEntrySend->sequenceNumber) ;
    // ---------------------------------------------(RAN-MT)---
    // No need to record InEnum;
    // These will be passed to TSC and returned.
    // REJECT.indication also not sent; Has no effect in our TSC.
    //S_MeRejectCause    cause;
    //cause.index = 0 ;  /* unspecifiedCause */
    //Se_MtPtvRjtIdcSend( Src_USER , &cause ) ;
    //Se_MtInEnumClear(  ) ;
    //Se_MtInEnumWrite( p_MultiplexEntrySend->multiplexEntryDescriptors ,
    //                  p_MultiplexEntrySend->size_of_multiplexEntryDescriptors ) ;
    // ---------------------------------------------------------
    PtvTrfIdcSend(p_MultiplexEntrySend) ;
}


/************************************************************************/
/*  function name       : Se_Mt_0502_0011                               */
/*  function outline    : Event     TRANSFER.response                   */
/*                      : Status    Incoming Awaiting Response          */
/*  function discription: Se_Mt_0502_0011( void )                       */
/*  input data          : None                                          */
/*  output data         : None                                          */
/*  draw time           : '96.10.31                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void MT::_0502_0011(uint32 sequenceNumber, PS_MuxDescriptor pMux)
{
    // Return to IDLE only if we have acknowledged the last pending MES (RAN-MT)
    if (sequenceNumber == (uint32)InSqcRead())
    {
        StatusWrite(MT_INCOMING_IDLE) ;
    }

    MsgMtAckSend(sequenceNumber, pMux) ;
}


/************************************************************************/
/*  function name       : Se_Mt_0503_0011                               */
/*  function outline    : Event     REJECT.request                      */
/*                      : Status    Incoming Awaiting Response          */
/*  function discription: Se_Mt_0503_0011( PS_Cause_Mt )                */
/*  input data          : PS_Cause_Mt p_Cause_Mt                        */
/*  output data         : None                                          */
/*  draw time           : '96.10.31                                     */
/*----------------------------------------------------------------------*/
/*  amendent career ()  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void MT::_0503_0011(PS_MeRejectCause p_Cause)
{
    StatusWrite(MT_INCOMING_IDLE) ;
    MsgMtRjtSend(p_Cause) ;
}


/************************************************************************/
/*  function name       : Se_Mt_0504_0010                               */
/*  function outline    : Event     MultiplexEntrySendAck               */
/*                      : Status    Outgoing Awaiting Response          */
/*  function discription: Se_Mt_0504_0010( PS_MultiplexEntrySendAck )   */
/*  input data          : PS_MultiplexEntrySendAck                      */
/*                      :                       p_MultiplexEntrySendAck */
/*  output data         : None                                          */
/*  draw time           : '96.10.31                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void MT::MultiplexEntrySendAck(PS_MultiplexEntrySendAck p_MultiplexEntrySendAck)
{
    PendingMtSendMap::iterator iter = iPendingMtSend.find(p_MultiplexEntrySendAck->sequenceNumber);
    if (iter == iPendingMtSend.end())
    {
        Print("MT::MultiplexEntrySendAck  Error: Failed to lookup pending MultiplexEntrySend for sequence number %d", p_MultiplexEntrySendAck->sequenceNumber);
        return;
    }
    (*iter).second->MultiplexEntrySendAck(p_MultiplexEntrySendAck);
    OSCL_DELETE((*iter).second);
    iPendingMtSend.erase(iter);
}


/************************************************************************/
/*  function name       : Se_Mt_0505_0010                               */
/*  function outline    : Event     MultiplexEntrySendReject            */
/*                      : Status    Outgoing Awaiting Response          */
/*  function discription: Se_Mt_0505_0010( PS_MultiplexEntrySendReject )*/
/*  input data          : PS_MultiplexEntrySendReject                   */
/*                      :                    p_MultiplexEntrySendReject */
/*  output data         : None                                          */
/*  draw time           : '96.10.31                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void MT::MultiplexEntrySendReject(PS_MultiplexEntrySendReject p_MultiplexEntrySendReject)
{
    PendingMtSendMap::iterator iter = iPendingMtSend.find(p_MultiplexEntrySendReject->sequenceNumber);
    if (iter == iPendingMtSend.end())
    {
        Print("MT::MultiplexEntrySendReject  Error: Failed to lookup pending MultiplexEntrySend for sequence number %d", p_MultiplexEntrySendReject->sequenceNumber);
        return;
    }
    (*iter).second->MultiplexEntrySendReject(p_MultiplexEntrySendReject);
    OSCL_DELETE((*iter).second);
    iPendingMtSend.erase(iter);
}

/************************************************************************/
/*  function name       : Se_Mt_0507_0011                               */
/*  function outline    : Event     MultiplexEntrySendRelease           */
/*                      : Status    Incoming Awaiting Response          */
/*  function discription: Se_Mt_0507_0011( void )                       */
/*  input data          : None                                          */
/*  output data         : None                                          */
/*  draw time           : '96.10.31                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void MT::_0507_0011(void)
{
    StatusWrite(MT_INCOMING_IDLE) ;
    PtvRjtIdcSend(S_InfHeader::OUTGOING, Src_PROTOCOL , NULL, 0) ;
}


/************************************************************************/
/*  function name       : Se_MtMsgMtSend                                */
/*  function outline    : MultiplexEntrySend Send                       */
/*  function discription: Se_MtMsgMtSend( PS_MuxDescriptor )            */
/*  input data          : PS_MuxDescriptor p_MuxDescriptor              */
/*  output data         : None                                          */
/*  draw time           : '96.10.31                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void MT::MsgMtSend(PS_MuxDescriptor p_MuxDescriptor, uint8 sn)
{
    S_MultiplexEntrySend       multiplexEntrySend ;
    S_H245Msg                  h245Msg ;

    multiplexEntrySend.sequenceNumber = sn ;
    multiplexEntrySend.size_of_multiplexEntryDescriptors = (uint16) p_MuxDescriptor->size_of_multiplexEntryDescriptors ;
    multiplexEntrySend.multiplexEntryDescriptors = p_MuxDescriptor->multiplexEntryDescriptors ;

    h245Msg.Type1 = H245_MSG_REQ ;
    h245Msg.Type2 = MSGTYP_MT ;
    h245Msg.pData = (uint8*) & multiplexEntrySend ;

    MessageSend(&h245Msg) ;
}


/************************************************************************/
/*  function name       : Se_MtMsgMtAckSend                             */
/*  function outline    : MultiplexEntrySendAck Send                    */
/*  function discription: Se_MtMsgMtAckSend( void )                     */
/*  input data          : None                                          */
/*  output data         : None                                          */
/*  draw time           : '96.10.31                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void MT::MsgMtAckSend(uint32 sequenceNumber, PS_MuxDescriptor pMux)
{
    S_MultiplexEntrySendAck		multiplexEntrySendAck ;
    uint32						multiplexTableEntryNumber[15];
    S_H245Msg					h245Msg ;
    uint32						i, numEntries;

    numEntries = pMux->size_of_multiplexEntryDescriptors;

    multiplexEntrySendAck.sequenceNumber = (uint8) sequenceNumber;
    multiplexEntrySendAck.size_of_multiplexTableEntryNumber = (uint16) numEntries;

    // Copy the entry numbers into the MESAck codeword... (RAN-MT)
    for (i = 0;i < numEntries && i < 15;++i)
    {
        multiplexTableEntryNumber[i] = pMux->multiplexEntryDescriptors[i].multiplexTableEntryNumber;
    }
    multiplexEntrySendAck.multiplexTableEntryNumber = multiplexTableEntryNumber;

    h245Msg.Type1 = H245_MSG_RPS ;
    h245Msg.Type2 = MSGTYP_MT_ACK ;
    h245Msg.pData = (uint8*) & multiplexEntrySendAck ;

    MessageSend(&h245Msg) ;
}


/************************************************************************/
/*  function name       : Se_MtMsgMtRjtSend                             */
/*  function outline    : MultiplexEntrySendReject Send                 */
/*  function discription: Se_MtMsgMtRjtSend( PS_Cause_Mt )              */
/*  input data          : PS_Cause_Mt p_Cause_Mt                        */
/*  output data         : None                                          */
/*  draw time           : '96.10.31                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void MT::MsgMtRjtSend(PS_MeRejectCause p_Cause)
{
    // -----------------------------------------------------(RAN-MT)---
    // NOTE: This routine needs an update.
    //       Should probably add arguments for sequenceNumber, entryList
    //         and used passed-in values in the MESReject codeword.
    //       (The InEnum array is not written, and should no longer be used)
    //       See Se_MtMsgMtAckSend() for a model.
    //       No reason to update it yet, since our terminal never rejects an MT.
    // -----------------------------------------------------------------
    S_MultiplexEntrySendReject             multiplexEntrySendReject ;
    S_MultiplexEntryRejectionDescriptions  multiplexEntryRejectionDescriptions ;
    S_H245Msg                              h245Msg ;

    multiplexEntrySendReject.sequenceNumber = (uint8) InSqcRead() ;

    multiplexEntryRejectionDescriptions.multiplexTableEntryNumber = (uint8) * (InMTEntries.MuxTableEntriesRead()) ;
    oscl_memcpy((int8*)&multiplexEntryRejectionDescriptions.meRejectCause , (int8*)&p_Cause , sizeof(S_MeRejectCause)) ;
    multiplexEntrySendReject.rejectionDescriptions = &multiplexEntryRejectionDescriptions ;
    multiplexEntrySendReject.size_of_rejectionDescriptions = 1 ;

    h245Msg.Type1 = H245_MSG_RPS ;
    h245Msg.Type2 = MSGTYP_MT_RJT ;
    h245Msg.pData = (uint8*) & multiplexEntrySendReject ;

    MessageSend(&h245Msg) ;
}


/************************************************************************/
/*  function name       : Se_MtMsgMtRlsSend                             */
/*  function outline    : MultiplexEntrySendRelease Send                */
/*  function discription: Se_MtMsgMtRlsSend( void )                     */
/*  input data          : None                                          */
/*  output data         : None                                          */
/*  draw time           : '96.10.31                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void MT::MsgMtRlsSend(MTEntries& entries)
{
    S_MultiplexEntrySendRelease    multiplexEntrySendRelease;
    uint32                         multiplexTableEntryNumber[15];
    S_H245Msg                      h245Msg;

    multiplexEntrySendRelease.size_of_multiplexTableEntryNumber = (uint16) entries.SizeRead() ;

    oscl_memcpy((int8*)&multiplexTableEntryNumber ,
                (int8*)entries.MuxTableEntriesRead() ,
                sizeof(uint32) * entries.SizeRead()) ;
    multiplexEntrySendRelease.multiplexTableEntryNumber = multiplexTableEntryNumber ;

    h245Msg.Type1 = H245_MSG_IDC ;
    h245Msg.Type2 = MSGTYP_MT_RLS ;
    h245Msg.pData = (uint8*) & multiplexEntrySendRelease ;

    MessageSend(&h245Msg) ;
}


/************************************************************************/
/*  function name       : Se_MtPtvTrfIdcSend                            */
/*  function outline    : TRANSFER.indication Send                      */
/*  function discription: Se_MtPtvTrfIdcSend( PS_MultiplexEntrySend )   */
/*  input data          : PS_MultiplexEntrySend p_MultiplexEntrySend    */
/*  output data         : None                                          */
/*  draw time           : '96.10.31                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void MT::PtvTrfIdcSend(PS_MultiplexEntrySend p_MultiplexEntrySend)
{
    S_MuxDescriptor    muxDescriptor ;
    S_InfHeader        infHeader ;

    muxDescriptor.size_of_multiplexEntryDescriptors = p_MultiplexEntrySend->size_of_multiplexEntryDescriptors ;
    muxDescriptor.multiplexEntryDescriptors = p_MultiplexEntrySend->multiplexEntryDescriptors ;

    infHeader.InfType = H245_PRIMITIVE ;
    infHeader.InfId = E_PtvId_Mt_Trf_Idc ;
    infHeader.InfSupplement1 = p_MultiplexEntrySend->sequenceNumber;
    infHeader.InfSupplement2 = 0 ;
    infHeader.pParameter = (uint8*) & muxDescriptor ;
    infHeader.Size = sizeof(S_MuxDescriptor) ;

    PrimitiveSend(&infHeader) ;
}


/************************************************************************/
/*  function name       : Se_MtPtvRjtIdcSend                            */
/*  function outline    : REJECT.indication Send                        */
/*  function discription: Se_MtPtvRjtIdcSend( int32 , PS_Cause_mtse )     */
/*  input data          : int32 Source                                    */
/*                      : PS_Cause_mtse p_Cause                         */
/*  output data         : None                                          */
/*  draw time           : '96.11.16                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void MT::PtvRjtIdcSend(S_InfHeader::TDirection dir, int32 Source , PS_MeRejectCause p_Cause, int32 sn)
{
    S_SourceCause_Mt   sourceCause_Mt ;
    S_InfHeader        infHeader ;

    if (Source == Src_USER)  /* SOURCE Parameter == USER */
    {
        sourceCause_Mt.Source = (ENUM_Source)Source ;
        oscl_memcpy((int8*)&sourceCause_Mt.Cause , (int8*)p_Cause , sizeof(S_MeRejectCause)) ;
    }

    infHeader.InfType = H245_PRIMITIVE ;
    infHeader.InfId = E_PtvId_Mt_Rjt_Idc ;
    infHeader.InfSupplement1 = dir ;
    infHeader.InfSupplement2 = sn ;
    if (Source == Src_USER)  /* SOURCE Parameter Equal USER */
    {
        infHeader.pParameter = (uint8*) & sourceCause_Mt ;
        infHeader.Size = sizeof(S_SourceCause_Mt) ;
    }
    else /* SOURCE Parameter Equal PROTOCOL ( The CAUSE parameter is not present ) */
    {
        infHeader.pParameter = NULL;
        infHeader.Size = 0 ;
    }

    PrimitiveSend(&infHeader) ;
}


/************************************************************************/
/*  function name       : Se_MtPtvTrfCfmSend                            */
/*  function outline    : TRANSFER.confirm Send                         */
/*  function discription: Se_MtPtvTrfCfmSend( void )                    */
/*  input data          : None                                          */
/*  output data         : None                                          */
/*  draw time           : '96.10.31                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)  :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void MT::PtvTrfCfmSend(int32 sn)
{
    S_InfHeader    infHeader ;

    infHeader.InfType = H245_PRIMITIVE ;
    infHeader.InfId = E_PtvId_Mt_Trf_Cfm ;
    infHeader.InfSupplement1 = (uint32)sn ;
    infHeader.InfSupplement2 = 0 ;
    infHeader.pParameter = NULL ;
    infHeader.Size = 0 ;

    PrimitiveSend(&infHeader) ;
}


#ifdef PVANALYZER /* --------SE Analyzer Tool -------- */

#define ANALYZER_SE 0x0020		// (Assume tag is fixed)
void Show245(uint16 tag, uint16 indent, char *inString);

// =========================================================
// Se_MtStatusShow()
//
// This function displays state transition information for
// the MT signaling entity.
// =========================================================
void MT::StatusShow(uint8 oldStatus, uint8 newStatus)
{
    char tempString[80];

    Show245(ANALYZER_SE, 0, "MTSE State Transition:");
    sprintf(tempString, "  from--> %s", StateLabel(oldStatus));
    Show245(ANALYZER_SE, 0, tempString);
    sprintf(tempString, "    to--> %s", StateLabel(newStatus));
    Show245(ANALYZER_SE, 0, tempString);
    Show245(ANALYZER_SE, 0, " ");
}

// ==========================================================
// Se_MtStateLabel()
//
// Returns a pointer to an approprate state label string.
// ==========================================================
char* MT::StateLabel(uint8 status)
{
    switch (status)
    {
        case MT_OUTGOING_IDLE:
            return("Outgoing IDLE");
            break;
        case MT_OUTGOING_AWTING_RPS:
            return("Outgoing AWAITING RESPONSE");
            break;
        case MT_INCOMING_IDLE:
            return("Incoming IDLE");
            break;
        case MT_INCOMING_AWTING_RPS:
            return("Incoming AWAITING RESPONSE");
            break;
        default:
            return("UNKNOWN STATE");
    }
}
#endif            /* --------------------------------- */

