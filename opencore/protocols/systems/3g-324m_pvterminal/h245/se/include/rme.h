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
/*  file name       : serme.h                                           */
/*  file contents   : Request Multiplex Entry Signalling Entity         */
/*                  :                                 Management Header */
/*  draw            : '96.11.26                                         */
/*----------------------------------------------------------------------*/
/*  amendment       :                                                   */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
#ifndef _SERME_H_
#define _SERME_H_


/************************************************************************/
/*  Headerfile Include                                                  */
/************************************************************************/
#include "h245pri.h"
#include "sebase.h"
#include "oscl_mem.h"

/************************************************************************/
/*  Definition                                                          */
/************************************************************************/
#define     RME_OUTGOING_IDLE           0X00    /* Outgoing Idle */
#define     RME_OUTGOING_AWTING_RPS     0X10    /* Outgoing Awaiting Response */
#define     RME_INCOMING_IDLE           0X01    /* Incoming Idle */
#define     RME_INCOMING_AWTING_RPS     0X11    /* Incoming Awaiting Response */


/************************************************************************/
/*  Function Prototype Declare                                          */
/************************************************************************/

class RMEntries
{
        enum { MAX_ENTRIES = 20 };
    public:
        RMEntries()
        {
            Clear();
        }
        ~RMEntries() {}

        int32 SizeRead()
        {
            return SizeEntries;
        }
        void EntriesCopy(uint32* pEntries)
        {
            oscl_memcpy((int8*)pEntries, (int8*)Entries, SizeEntries * sizeof(uint32)) ;
        }

        void Write(int sizeEntries, uint32* entries)
        {
            SizeEntries = sizeEntries;
            oscl_memcpy((int8*)Entries, entries, sizeEntries * sizeof(uint32));
        }

        void Clear()
        {
            SizeEntries = 0;
            oscl_memset(Entries, 0, sizeof(Entries[0]));
        }

        void Sequence()
        {
            SizeEntries = 15;
            for (int ii = 0; ii < SizeEntries; ii++)
            {
                Entries[ii] = ii + 1;
            }
        }

    private:
        int32  SizeEntries;
        uint32 Entries[ MAX_ENTRIES ];
};

class RME : public SEBase
{
    public:
        RME() : SEBase()
        {
            OutRMEntries.Sequence();
            InRMEntries.Clear();
            T107TimerStop() ;
            TmrSqcNumberClear() ;               /* H245_TWOWAY */
            StatusWrite(RME_INCOMING_IDLE) ;    /* Init incoming RMESE */
            StatusWrite(RME_OUTGOING_IDLE) ;    /* Init outgoing RMESE */
        }
        ~RME() {}

        void Reset()
        {
            OutRMEntries.Sequence();
            InRMEntries.Clear();
            T107TimerStop() ;
            TmrSqcNumberClear() ;
            StatusWrite(RME_INCOMING_IDLE) ;
            StatusWrite(RME_OUTGOING_IDLE) ;
        }

        void _0600_0000(void) ;
        void _0601_0001(PS_RequestMultiplexEntry) ;
        void _0601_0011(PS_RequestMultiplexEntry) ;
        void _0602_0011(void) ;
        void _0603_0011(PS_RmeRejectCause) ;
        void _0604_0010(PS_RequestMultiplexEntryAck) ;
        void _0605_0010(PS_RequestMultiplexEntryReject) ;
        void _0606_0010(int32) ;
        void _0607_0011(PS_RequestMultiplexEntryRelease) ;

    private:
        RME(const RME&);

        void MsgEntrySend(void) ;
        void MsgEntryAckSend(void) ;
        void MsgEntryRjtSend(PS_RmeRejectCause) ;
        void MsgEntryRlsSend(void) ;
        void PtvSendIndSend(void) ;
        void PtvRjtIdcSend(int32 , PS_RmeRejectCause) ;
        void PtvSendCfmSend(void) ;
    public:
        uint8 StatusRead(void)
        {
            return Status;
        }
        uint8 StatusRead2(void)
        {
            return Status2;
        }
    private:
        void StatusWrite(uint8 status)
        {
            if (status&0x01)  	                  /* Incoming */
            {
#ifdef PVANALYZER
                StatusShow(Status2, status);
#endif
                Status2 = status;
            }
            else  			                       /* Outgoing */
            {
#ifdef PVANALYZER
                StatusShow(Status, status);
#endif
                Status = status;
            }
        }

#ifdef PVANALYZER
        void StatusShow(uint8 oldStatus, uint8 newStatus);
        char* StateLabel(uint8 status);
#endif

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
        void T107TimerStart(void);
        void T107TimerStop(void);

        RMEntries OutRMEntries;
        RMEntries InRMEntries;
        int32       TmrSqcNumber;
        uint8     Status;
        uint8     Status2;
};

#endif /* _SERME_ */
