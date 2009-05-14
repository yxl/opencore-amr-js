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
/*  file name       : semt.h                                            */
/*  file contents   : Multiplex Table Signalling Entity                 */
/*                  :                                 Management Header */
/*  draw            : '96.11.11                                         */
/*----------------------------------------------------------------------*/
/*  amendment       :                                                   */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
#ifndef _SEMT_H_
#define _SEMT_H_

/************************************************************************/
/*  Headerfile Include                                                  */
/************************************************************************/
#include "oscl_timer.h"
#include "oscl_map.h"
#include "h245pri.h"
#include "sebase.h"
#include "oscl_mem.h"

/************************************************************************/
/*  Multuplex Table Status Definition                                   */
/************************************************************************/
#define     MT_OUTGOING_IDLE            0X00    /* Outgoing Idle */
#define     MT_OUTGOING_AWTING_RPS      0X10    /* Outgoing Awaiting Response */
#define     MT_INCOMING_IDLE            0X01    /* Incoming Idle */
#define     MT_INCOMING_AWTING_RPS      0X11    /* Incoming Awaiting Response */

#define T104_TIMER_DURATION  30 /* In seconds */

/************************************************************************/
/*  Entry Number Type Definition                                        */
/************************************************************************/
#define MAX_MUX_ENTRIES 15

class MTEntries
{
    public:
        MTEntries()
        {
            Clear();
        }
        ~MTEntries() {}

        int32 SizeRead()
        {
            return SizeMuxTableEntries;
        }
        uint32* MuxTableEntriesRead()
        {
            return MuxTableEntries;
        };

        void Clear()
        {
            SizeMuxTableEntries = 0;
            oscl_memset(MuxTableEntries, 0, sizeof(MuxTableEntries[0]));
        }

        void Write(PS_MultiplexEntryDescriptor descriptors, int32 size)
        {
            SizeMuxTableEntries = size;
            for (int ii = 0; ii < size && ii < MAX_MUX_ENTRIES; ii++)
            {
                MuxTableEntries[ii] = descriptors->multiplexTableEntryNumber;
                descriptors++;
            }
        }

    private:
        int32  SizeMuxTableEntries;
        uint32 MuxTableEntries[ MAX_MUX_ENTRIES ];
};

/* This interface provides utility functions that will be required by multiplex entry send
   state managers */
class MultiplexEntrySendUtility
{
    public:
        virtual ~MultiplexEntrySendUtility() {}
        virtual void MsgMtSend(PS_MuxDescriptor, uint8 sn) = 0;
        virtual void PtvTrfCfmSend(int32 sn) = 0;
        virtual void PtvRjtIdcSend(S_InfHeader::TDirection dir, int32 Source , PS_MeRejectCause p_Cause, int32 sn) = 0;
        virtual void MsgMtRlsSend(MTEntries& entries) = 0;
        virtual void RequestT104Timer(int32 sn) = 0;
        virtual void CancelT104Timer(int32 sn) = 0;
};

/* This class maintains state information about outgoing multiplex entry send requests */
//class MultiplexEntrySendMgr : public HeapBase
class MultiplexEntrySendMgr : public HeapBase
{
    public:
        MultiplexEntrySendMgr(int32 sn, MultiplexEntrySendUtility* util);
        void TransferRequest(PS_MuxDescriptor p_MuxDescriptor);
        void MultiplexEntrySendAck(PS_MultiplexEntrySendAck p_MultiplexEntrySendAck);
        void MultiplexEntrySendReject(PS_MultiplexEntrySendReject p_MultiplexEntrySendReject);
        void T104Timeout();
    private:
        void Write(PS_MultiplexEntryDescriptor descriptors, int32 size);
        void StatusWrite(uint32 status);
        void T104TimerStart(void) ;
        void T104TimerStop(void) ;

        MTEntries		iOutMTEntries;
        MultiplexEntrySendUtility* iUtil;
        int32            iSn;
        uint32           iStatus;
};

/************************************************************************/
/*  Function Prototype Declare                                          */
/************************************************************************/
typedef Oscl_Map<int32, MultiplexEntrySendMgr*, OsclMemAllocator> PendingMtSendMap;
class MT : public SEBase, public MultiplexEntrySendUtility, public OsclTimerObserver
{
    public:
        MT();
        ~MT()
        {
            Reset();
        }

        void Reset();

        void TransferRequest(PS_MuxDescriptor) ;
        void _0501_0001(PS_MultiplexEntrySend) ;
        void _0501_0011(PS_MultiplexEntrySend) ;
        void _0502_0011(uint32 sequenceNumber, PS_MuxDescriptor pMux) ;
        void _0503_0011(PS_MeRejectCause) ;
        void MultiplexEntrySendAck(PS_MultiplexEntrySendAck) ;
        void MultiplexEntrySendReject(PS_MultiplexEntrySendReject) ;
        void _0506_0010(int32) ;
        void _0507_0011(void) ;
        /* MultiplexEntrySendUtility virtuals */
        void MsgMtSend(PS_MuxDescriptor, uint8 sn) ;
        void PtvTrfCfmSend(int32 sn) ;
        void PtvRjtIdcSend(S_InfHeader::TDirection dir, int32 Source , PS_MeRejectCause p_Cause, int32 sn);
        void MsgMtRlsSend(MTEntries& entries) ;
        void RequestT104Timer(int32 sn);
        void CancelT104Timer(int32 sn);
        /* OsclTimer virtuals */
        void TimeoutOccurred(int32 timerID, int32 timeoutInfo);
    private:
        MT(const MT&);

        void MsgMtAckSend(uint32 sequenceNumber, PS_MuxDescriptor pMux) ;
        void MsgMtRjtSend(PS_MeRejectCause) ;

        void PtvTrfIdcSend(PS_MultiplexEntrySend) ;
    public:
        uint8 StatusRead2(void)
        {
            return Status2;
        }
    private:

        void StatusWrite(uint8 status)
        {
            if (status & 0x01)  	                  /* Incoming */
            {
#ifdef PVANALYZER /* --------SE Analyzer Tool -------- */
                StatusShow(Status2, status);
#endif            /* --------------------------------- */
                Status2 = status;
            }
        }
#ifdef PVANALYZER /* --------SE Analyzer Tool -------- */
        void StatusShow(uint8 oldStatus, uint8 newStatus);
        char* StateLabel(uint8 status);
#endif
        void OutSqcClear(void)
        {
            OutSqc = 0;
        }
        void OutSqcInc(void)
        {
            OutSqc++;
        }
        int32 OutSqcRead(void)
        {
            return OutSqc;
        }
        void InSqcClear(void)
        {
            InSqc = 0;
        }
        void InSqcWrite(int32 insqc)
        {
            InSqc = insqc;
        }
        int32 InSqcRead(void)
        {
            return InSqc;
        }
        void TmrSqcNumberClear(void)
        {
            TmrSqcNumber = 0;
        }
        int32 TmrSqcNumberRead(void)
        {
            return TmrSqcNumber;
        }
        void TmrSqcNumberInc(void)
        {
            TmrSqcNumber++;
        }

        uint8			Status2;
        int32             OutSqc;
        int32             InSqc;
        int32             TmrSqcNumber;
        MTEntries		InMTEntries;
        PendingMtSendMap iPendingMtSend;
        OsclTimer<OsclMemAllocator> iTimer;
};

#endif /* _SEMT_ */
