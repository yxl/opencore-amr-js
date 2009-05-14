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
/*  file name       : semsd.h                                           */
/*  file contents   : Master Slave Determination Signalling Entity      */
/*                  :                                 Management Header */
/*  draw            : '96.11.11                                         */
/*----------------------------------------------------------------------*/
/*  amendment       :                                                   */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
#ifndef _SEMSD_
#define _SEMSD_

#include "oscl_base.h"
#include "oscl_rand.h"
#include "oscl_tickcount.h"
#include "oscl_timer.h"
#include "sebase.h"
#include "h245def.h"
#include "h245inf.h"
#include "semsgque.h"

#define TWO_24 (1<<24)
#define TWO_23 (1<<23)
#define MSD_MAX_RETRIES 100

enum MSDStatus { MSD_INDETERMINATE = 0, MSD_MASTER = 1, MSD_SLAVE = 2 };
enum MSDErrCode
{
    MSD_ERROR_CODE_A = 0, // no response from remove MSD
    MSD_ERROR_CODE_B = 1, // remote MSD see no response from local MSD
    MSD_ERROR_CODE_C = 2, // inappropriate message
    MSD_ERROR_CODE_D = 3, // inappropriate message
    MSD_ERROR_CODE_E = 4, // inconsistent field value
    MSD_ERROR_CODE_F = 5  // max number of retries reached
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
class MSDObserver
{
    public:
        virtual ~MSDObserver() {}
        virtual void MSDDetermineConfirm(MSDStatus type) = 0;
        virtual void MSDDetermineIndication(MSDStatus type) = 0;
        virtual void MSDRejectIndication() = 0;
        virtual void MSDErrorIndication(MSDErrCode errCode) = 0;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
class MSD : public SEBase, public OsclTimerObserver
{
    private:
        enum MSDState { IDLE, OUTGOING_AWAITING_RESPONSE, INCOMING_AWAITING_RESPONSE }; // states

    public:
        MSD() :
                Observer(NULL),
                TerminalType(128),
                State(IDLE),
                Status(MSD_INDETERMINATE),
                StatusDeterminationNumber(0),
                RetryCnt(0)
        {
            // seed the random number generator
            RandGen.Seed(OsclTickCount::TickCount());
            // calcuate initial status determination number
            NewStatusDeterminationNumber();
        }

        virtual ~MSD() {}

        void Reset()
        {
            Print("Reset MSD\n");
            State = IDLE;
            Status = MSD_INDETERMINATE;
            RetryCnt = 0;
            // calcuate new status determination number
            NewStatusDeterminationNumber();
            ResetTimer();
        }

        void SetObserver(MSDObserver *observer)
        {
            Observer = observer;
        }

        void SetTerminalType(uint8 ttype)
        {
            TerminalType = ttype;
        }

        void DetermineRequest()
        {
            Print("Received MSD Determine Request\n");
            switch (GetState())
            {
                case IDLE:
                    Idle();
                    break;
                case OUTGOING_AWAITING_RESPONSE:
                case INCOMING_AWAITING_RESPONSE:
                default:
                    break;
            }
        }

        void Handler(PS_MasterSlaveDetermination msd)
        {
            Print("Received MSD\n");
            switch (GetState())
            {
                case IDLE:
                    Idle(msd);
                    break;
                case OUTGOING_AWAITING_RESPONSE:
                    OutgoingAwaitingResponse(msd);
                    break;
                case INCOMING_AWAITING_RESPONSE:
                    IncomingAwaitingResponse(msd);
                    break;
                default:
                    break;
            }
        }

        void Handler(PS_MasterSlaveDeterminationAck msda)
        {
            Print("Received MSDAck\n");
            switch (GetState())
            {
                case IDLE:
                    break;
                case OUTGOING_AWAITING_RESPONSE:
                    OutgoingAwaitingResponse(msda);
                    break;
                case INCOMING_AWAITING_RESPONSE:
                    IncomingAwaitingResponse(msda);
                    break;
                default:
                    break;
            }
        }

        void Handler(PS_MasterSlaveDeterminationReject msdr)
        {
            Print("Received MSDReject\n");
            switch (GetState())
            {
                case IDLE:
                    break;
                case OUTGOING_AWAITING_RESPONSE:
                    OutgoingAwaitingResponse(msdr);
                    break;
                case INCOMING_AWAITING_RESPONSE:
                    IncomingAwaitingResponse(msdr);
                    break;
                default:
                    break;
            }
        }

        void Handler(PS_MasterSlaveDeterminationRelease msdr)
        {
            Print("Received MSDRelease\n");
            switch (GetState())
            {
                case IDLE:
                    break;
                case OUTGOING_AWAITING_RESPONSE:
                    OutgoingAwaitingResponse(msdr);
                    break;
                case INCOMING_AWAITING_RESPONSE:
                    IncomingAwaitingResponse(msdr);
                    break;
                default:
                    break;
            }
        }

        void HandlerTimeout()
        {
            Print("Received MSD Timeout\n");
            switch (GetState())
            {
                case IDLE:
                    break;
                case OUTGOING_AWAITING_RESPONSE:
                    OutgoingAwaitingResponseTimeout();
                    break;
                case INCOMING_AWAITING_RESPONSE:
                    IncomingAwaitingResponseTimeout();
                    break;
                default:
                    break;
            }
        }

        void TimeoutOccurred(int32 timerID, int32 timeoutInfo)
        {
            Print("    MSD::TimeoutOccurred");

            OSCL_UNUSED_ARG(timerID);
            OSCL_UNUSED_ARG(timeoutInfo);
            HandlerTimeout();
        }

        MSDStatus GetStatus()
        {
            if (Status == MSD_INDETERMINATE) Print("    Status == INDETERMINATE\n");
            else if (Status == MSD_MASTER) Print("    Status == MASTER\n");
            else if (Status == MSD_SLAVE) Print("    Status == SLAVE\n");

            return Status;
        }

    private:
        MSD(const MSD&);

        void SetStatus(MSDStatus status)
        {
            Status = status;

            if (Status == MSD_INDETERMINATE) Print("    Status -> INDETERMINATE\n");
            else if (Status == MSD_MASTER) Print("    Status -> MASTER\n");
            else if (Status == MSD_SLAVE) Print("    Status -> SLAVE\n");
        }

        void SetState(MSDState state)
        {
            State = state;

            if (State == IDLE)	Print("    State -> IDLE\n");
            else if (State == OUTGOING_AWAITING_RESPONSE) Print("    State -> OUTGOING_AWAITING_RESPONSE\n");
            else if (State == INCOMING_AWAITING_RESPONSE) Print("    State -> INCOMING_AWAITING_RESPONSE\n");
        }

        MSDState GetState()
        {
            if (State == IDLE)	Print("    State == IDLE\n");
            else if (State == OUTGOING_AWAITING_RESPONSE) Print("    State == OUTGOING_AWAITING_RESPONSE\n");
            else if (State == INCOMING_AWAITING_RESPONSE) Print("    State == INCOMING_AWAITING_RESPONSE\n");

            return State;
        }

        // Handles Determine.Request when in IDLE state
        void Idle()
        {
            RetryCnt = 1;
            SetTimer();
            SetState(OUTGOING_AWAITING_RESPONSE);
            SendMSD();
        }

        void Idle(PS_MasterSlaveDetermination msd)
        {
            DetermineStatus(msd);
            if (GetStatus() == MSD_INDETERMINATE)
            {
                SendMSDReject();
            }
            else
            {
                SetTimer();
                SetState(INCOMING_AWAITING_RESPONSE);
                SendMSDAck();
                if (Observer) Observer->MSDDetermineIndication(Status);
            }
        }

        void OutgoingAwaitingResponse(PS_MasterSlaveDeterminationAck msda)
        {
            ResetTimer();
            SetStatus(((msda->decision.index == 0) ? MSD_MASTER : MSD_SLAVE));
            SetState(IDLE);
            SendMSDAck();
            if (Observer) Observer->MSDDetermineConfirm(Status);
        }

        void OutgoingAwaitingResponse(PS_MasterSlaveDetermination msd)
        {
            ResetTimer();
            DetermineStatus(msd);
            if (GetStatus() == MSD_INDETERMINATE)
            {
                if (RetryCnt >= MSD_MAX_RETRIES)
                {
                    SetState(IDLE);
                    if (Observer)
                    {
                        Observer->MSDErrorIndication(MSD_ERROR_CODE_F);
                        Observer->MSDRejectIndication();
                    }
                }
                else
                {
                    NewStatusDeterminationNumber();
                    RetryCnt++;
                    SetTimer();
                    SetState(OUTGOING_AWAITING_RESPONSE);
                    SendMSD();
                }
            }
            else
            {
                SetTimer();
                SetState(INCOMING_AWAITING_RESPONSE);
                SendMSDAck();
                if (Observer) Observer->MSDDetermineIndication(Status);
            }
        }

        void OutgoingAwaitingResponse(PS_MasterSlaveDeterminationReject msdr)
        {
            OSCL_UNUSED_ARG(msdr);
            ResetTimer();
            if (RetryCnt >= MSD_MAX_RETRIES)
            {
                SetState(IDLE);
                if (Observer)
                {
                    Observer->MSDErrorIndication(MSD_ERROR_CODE_F);
                    Observer->MSDRejectIndication();
                }
            }
            else
            {
                NewStatusDeterminationNumber();
                RetryCnt++;
                SetTimer();
                SetState(OUTGOING_AWAITING_RESPONSE);
                SendMSD();
            }
        }

        void OutgoingAwaitingResponse(PS_MasterSlaveDeterminationRelease msdr)
        {
            OSCL_UNUSED_ARG(msdr);
            ResetTimer();
            SetState(IDLE);
            if (Observer)
            {
                Observer->MSDErrorIndication(MSD_ERROR_CODE_B);
                Observer->MSDRejectIndication();
            }
        }

        void OutgoingAwaitingResponseTimeout()
        {
            SendMSDRelease();
            SetState(IDLE);
            if (Observer)
            {
                Observer->MSDErrorIndication(MSD_ERROR_CODE_A);
                Observer->MSDRejectIndication();
            }
        }

        void IncomingAwaitingResponse(PS_MasterSlaveDeterminationAck msda)
        {
            ResetTimer();
            if ((msda->decision.index == 0 && GetStatus() == MSD_MASTER) ||
                    (msda->decision.index == 1 && GetStatus() == MSD_SLAVE))
            {
                SetState(IDLE);
                if (Observer) Observer->MSDDetermineConfirm(Status);
            }
            else
            {
                SetState(IDLE);
                if (Observer)
                {
                    Observer->MSDErrorIndication(MSD_ERROR_CODE_E);
                    Observer->MSDRejectIndication();
                }
            }
        }

        void IncomingAwaitingResponse(PS_MasterSlaveDetermination msd)
        {
            OSCL_UNUSED_ARG(msd);
            ResetTimer();
            SetState(IDLE);
            if (Observer)
            {
                Observer->MSDErrorIndication(MSD_ERROR_CODE_C);
                Observer->MSDRejectIndication();
            }
        }

        void IncomingAwaitingResponse(PS_MasterSlaveDeterminationReject msdr)
        {
            OSCL_UNUSED_ARG(msdr);
            ResetTimer();
            SetState(IDLE);
            if (Observer)
            {
                Observer->MSDErrorIndication(MSD_ERROR_CODE_D);
                Observer->MSDRejectIndication();
            }
        }

        void IncomingAwaitingResponse(PS_MasterSlaveDeterminationRelease msdr)
        {
            OSCL_UNUSED_ARG(msdr);
            ResetTimer();
            SetState(IDLE);
            if (Observer)
            {
                Observer->MSDErrorIndication(MSD_ERROR_CODE_B);
                Observer->MSDRejectIndication();
            }
        }

        void IncomingAwaitingResponseTimeout()
        {
            SetState(IDLE);
            if (Observer)
            {
                Observer->MSDErrorIndication(MSD_ERROR_CODE_A);
                Observer->MSDRejectIndication();
            }
        }

        void DetermineStatus(PS_MasterSlaveDetermination msd)
        {
            Print("    My Terminal Type == %d, Incoming Terminal Type == %d\n", TerminalType, msd->terminalType);
            Print("    My Status Det Num == 0x%08x, Incoming Status Det Num == 0x%08x\n", StatusDeterminationNumber, msd->statusDeterminationNumber);

            if (TerminalType != msd->terminalType)
            {
                SetStatus((TerminalType < msd->terminalType) ? MSD_SLAVE : MSD_MASTER);
            }
            else
            {
                uint32 diff = (msd->statusDeterminationNumber - StatusDeterminationNumber) % TWO_24;
                if (diff == 0 || diff == TWO_23) SetStatus(MSD_INDETERMINATE);
                else SetStatus((diff < TWO_23) ? MSD_MASTER : MSD_SLAVE);
            }
        }

        void SendMSD()
        {
            Print("    Sending MSD - Terminal Type == %d, Status Det Num == 0x%08x\n", TerminalType, StatusDeterminationNumber);
            S_MasterSlaveDetermination masterSlaveDetermination ;

            masterSlaveDetermination.terminalType = TerminalType;
            masterSlaveDetermination.statusDeterminationNumber = StatusDeterminationNumber;

            MessageSend(H245_MSG_REQ, MSGTYP_MSD, (uint8*)&masterSlaveDetermination);
        }

        void SendMSDReject()
        {
            Print("    Sending MSDReject\n");
            S_MasterSlaveDeterminationReject masterSlaveDeterminationReject ;

            masterSlaveDeterminationReject.msdRejectCause.index = 0 ;

            MessageSend(H245_MSG_RPS, MSGTYP_MSD_RJT, (uint8*)&masterSlaveDeterminationReject) ;
        }

        void SendMSDAck()
        {
            Print("    Sending MSDAck\n");
            S_MasterSlaveDeterminationAck  masterSlaveDeterminationAck ;

            masterSlaveDeterminationAck.decision.index = (uint16)((Status == MSD_MASTER) ? 1 : 0);

            MessageSend(H245_MSG_RPS, MSGTYP_MSD_ACK, (uint8*)&masterSlaveDeterminationAck) ;
        }

        void SendMSDRelease()
        {
            Print("    Sending MSDRelease\n");
            S_MasterSlaveDeterminationRelease  masterSlaveDeterminationRelease ;

            MessageSend(H245_MSG_IDC, MSGTYP_MSD_RLS, (uint8*)&masterSlaveDeterminationRelease);
        }

        void NewStatusDeterminationNumber()
        {
            StatusDeterminationNumber = RandGen.Rand() & 0xFFFFFF; // 0xFFFFFF max allowed
            Print("    Status Det Number -> 0x%08x\n", StatusDeterminationNumber);
        }

        void SetTimer()
        {
            RequestTimer(106, 0, TimerDuration, this);
        }
        void ResetTimer()
        {
            CancelTimer(106);
        }

        MSDObserver *Observer;
        uint8 TerminalType;
        MSDState State;
        MSDStatus Status;
        uint32 StatusDeterminationNumber;
        uint8 RetryCnt;
        OsclRand RandGen;
};

#endif /* _SEMSD_ */
