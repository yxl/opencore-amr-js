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
/*  file name       : sece.h                                            */
/*  file contents   : Capability Exchange Signalling Entity             */
/*                  :                                 Management Header */
/*  draw            : '96.11.11                                         */
/*----------------------------------------------------------------------*/
/*  amendment       :                                                   */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
#ifndef _SECE_H_
#define _SECE_H_

#include "oscl_base.h"
#include "sebase.h"
#include "oscl_timer.h"
#include "h245def.h"
#include "h245inf.h"

enum CESource { CE_USER, CE_PROTOCOL };
enum CECause
{
    CE_UNSPECIFIED = 0,
    CE_UNDEFINED_TABLE_ENTRY_USED,
    CE_DESCRIPTOR_CAPACITY_EXCEEDED,
    CE_TABLE_ENTRY_CAPACITY_EXCEEDED
};
enum CEDirection { CE_OUTGOING, CE_INCOMING };

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
class CEObserver
{
    public:
        virtual ~CEObserver() {}
        virtual void CETransferIndication(OsclSharedPtr<S_TerminalCapabilitySet> tcs) = 0;
        virtual void CETransferConfirm() = 0;
        virtual void CERejectIndication(CESource source, CECause cause, CEDirection direction) = 0;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
class CE : public SEBase, public OsclTimerObserver
{
    private:
        enum CEState { IDLE, AWAITING_RESPONSE }; // states

    public:
        CE() :
                Observer(NULL),
                OutgoingState(IDLE),
                IncomingState(IDLE),
                OutSeqNum(0),
                InSeqNum(0xff)
        {}

        ~CE() {}

        void Reset()
        {
            Print("Reset CE\n");
            OutgoingState = IDLE;
            IncomingState = IDLE;
            OutSeqNum = 0;
            InSeqNum = 0xff;
            ResetTimer();
        }

        void SetObserver(CEObserver *observer)
        {
            Observer = observer;
        }

        // Primitives
        void TransferRequest(PS_TerminalCapabilitySet tcs)
        {
            Print("Received CE Transfer Request\n");
            switch (OutgoingState)
            {
                case IDLE:
                    OutgoingIdle(tcs);
                    break;
                case AWAITING_RESPONSE:
                default:
                    break;
            }
        }

        void TransferResponse()
        {
            Print("Received CE Transfer Response\n");
            switch (IncomingState)
            {
                case IDLE:
                    break;
                case AWAITING_RESPONSE:
                    IncomingAwaitingResponse();
                    break;
                default:
                    break;
            }
        }

        void RejectRequest(CECause cause, uint16 highestEntryProcessed = 0)
        {
            Print("Received CE Reject Request\n");
            switch (IncomingState)
            {
                case IDLE:
                    break;
                case AWAITING_RESPONSE:
                    IncomingAwaitingResponse(cause, highestEntryProcessed);
                    break;
                default:
                    break;
            }
        }

        // Outgoing Message Handlers
        void Handler(PS_TerminalCapabilitySetAck tcsa)
        {
            Print("Received TCSAck \n");
            switch (OutgoingState)
            {
                case IDLE:
                    break;
                case AWAITING_RESPONSE:
                    OutgoingAwaitingResponse(tcsa);
                    break;
                default:
                    break;
            }
        }

        void Handler(PS_TerminalCapabilitySetReject tcsr)
        {
            Print("Received TCSReject\n");
            switch (OutgoingState)
            {
                case IDLE:
                    break;
                case AWAITING_RESPONSE:
                    OutgoingAwaitingResponse(tcsr);
                    break;
                default:
                    break;
            }
        }

        void HandlerTimeout()
        {
            Print("Received TCS Timeout\n");
            switch (OutgoingState)
            {
                case IDLE:
                    break;
                case AWAITING_RESPONSE:
                    OutgoingAwaitingResponseTimeout();
                    break;
                default:
                    break;
            }
        }

        // Incoming Message Handlers
        void Handler(OsclSharedPtr<S_TerminalCapabilitySet> tcs)
        {
            Print("Received TCS\n");
            switch (IncomingState)
            {
                case IDLE:
                    IncomingIdle(tcs);
                    break;
                case AWAITING_RESPONSE:
                    IncomingAwaitingResponse(tcs);
                    break;
                default:
                    break;
            }
        }

        void Handler(PS_TerminalCapabilitySetRelease tcsr)
        {
            Print("Received TCSRelease\n");
            switch (IncomingState)
            {
                case IDLE:
                    break;
                case AWAITING_RESPONSE:
                    IncomingAwaitingResponse(tcsr);
                    break;
                default:
                    break;
            }
        }

        void TimeoutOccurred(int32 timerID, int32 timeoutInfo)
        {
            Print("    CE::TimeoutOccurred");
            OSCL_UNUSED_ARG(timerID);
            OSCL_UNUSED_ARG(timeoutInfo);
            HandlerTimeout();
        }

    private:
        CE(const CE&);
        CE& operator=(const CE&);

        CEObserver *Observer;
        CEState OutgoingState;
        CEState IncomingState;
        uint8 OutSeqNum;
        uint8 InSeqNum;

        void SetTimer()
        {
            RequestTimer(101, 0, TimerDuration, this);
        }
        void ResetTimer()
        {
            CancelTimer(101);
        }

        void OutgoingIdle(PS_TerminalCapabilitySet tcs)
        {
            OutSeqNum++;
            SetTimer();
            OutgoingState = AWAITING_RESPONSE;
            SendTerminalCapabilitySet(tcs);
        }

        void OutgoingAwaitingResponse(PS_TerminalCapabilitySetAck tcsa)
        {
            if (tcsa->sequenceNumber == OutSeqNum)
            {
                ResetTimer();
                OutgoingState = IDLE;
                if (Observer) Observer->CETransferConfirm();
            }
        }

        void OutgoingAwaitingResponse(PS_TerminalCapabilitySetReject tcsr)
        {
            if (tcsr->sequenceNumber == OutSeqNum)
            {
                ResetTimer();
                OutgoingState = IDLE;
                if (Observer) Observer->CERejectIndication(CE_USER, (CECause)tcsr->tcsRejectCause.index, CE_OUTGOING);
            }
        }

        void OutgoingAwaitingResponseTimeout()
        {
            OutgoingState = IDLE;
            SendTerminalCapabilitySetRelease();
            if (Observer) Observer->CERejectIndication(CE_PROTOCOL, CE_UNSPECIFIED, CE_OUTGOING);
        }

        void IncomingIdle(OsclSharedPtr<S_TerminalCapabilitySet> tcs)
        {
            InSeqNum = tcs->sequenceNumber;
            IncomingState = AWAITING_RESPONSE;
            if (Observer) Observer->CETransferIndication(tcs);
        }

        // handles TransferResponse
        void IncomingAwaitingResponse()
        {
            IncomingState = IDLE;
            SendTerminalCapabilitySetAck();
        }

        // handles RejectRequest
        void IncomingAwaitingResponse(CECause cause, uint16 highestEntryProcessed)
        {
            IncomingState = IDLE;
            SendTerminalCapabilitySetReject(cause, highestEntryProcessed);
        }

        void IncomingAwaitingResponse(PS_TerminalCapabilitySetRelease tcsr)
        {
            OSCL_UNUSED_ARG(tcsr);
            IncomingState = IDLE;
            if (Observer) Observer->CERejectIndication(CE_PROTOCOL, CE_UNSPECIFIED, CE_INCOMING);
        }

        void IncomingAwaitingResponse(OsclSharedPtr<S_TerminalCapabilitySet> tcs)
        {
            InSeqNum = tcs->sequenceNumber;
            IncomingState = AWAITING_RESPONSE;
            if (Observer) Observer->CERejectIndication(CE_USER, CE_UNSPECIFIED, CE_INCOMING);
            if (Observer) Observer->CETransferIndication(tcs);
        }

        void SendTerminalCapabilitySet(PS_TerminalCapabilitySet tcs)
        {
            Print("    Sending TCS\n");
            tcs->sequenceNumber = OutSeqNum;
            MessageSend(H245_MSG_REQ, MSGTYP_CE, (uint8*)tcs);
        }

        void SendTerminalCapabilitySetAck()
        {
            Print("    Sending TCS Ack\n");
            S_TerminalCapabilitySetAck tcsa ;
            tcsa.sequenceNumber = InSeqNum;
            MessageSend(H245_MSG_RPS, MSGTYP_CE_ACK, (uint8*)&tcsa);
        }

        void SendTerminalCapabilitySetRelease()
        {
            Print("    Sending TCS Release\n");
            S_TerminalCapabilitySetRelease tcsr;
            oscl_memset((int8*)&tcsr, 0, sizeof(S_TerminalCapabilitySetRelease)) ;
            MessageSend(H245_MSG_IDC, MSGTYP_CE_RLS, (uint8*)&tcsr);
        }

        void SendTerminalCapabilitySetReject(CECause cause, uint16 highestEntryProcessed)
        {
            Print("    Sending TCS Reject\n");
            S_TerminalCapabilitySetReject tcsr;
            S_TableEntryCapacityExceeded tece;

            tcsr.sequenceNumber = InSeqNum;
            tcsr.tcsRejectCause.index = (uint16)cause;
            if (cause == CE_TABLE_ENTRY_CAPACITY_EXCEEDED)
            {
                tcsr.tcsRejectCause.tableEntryCapacityExceeded = &tece;
                tcsr.tcsRejectCause.tableEntryCapacityExceeded->index = (uint16)(highestEntryProcessed ? 0 : 1);
                tcsr.tcsRejectCause.tableEntryCapacityExceeded->highestEntryNumberProcessed = highestEntryProcessed;
            }
            MessageSend(H245_MSG_RPS, MSGTYP_CE_RJT, (uint8*)&tcsr);
        }
};

#endif /* _SECE_ */
