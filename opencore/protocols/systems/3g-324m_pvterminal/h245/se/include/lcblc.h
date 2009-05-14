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
/*  file name       : selcblc.h                                           */
/*  file contents   : Common Logical Channel / Bi-directional Logical Channel */
/*                  :                                 Management Header */
/*  draw            : '96.11.11                                         */
/*----------------------------------------------------------------------*/
/*  amendment       :                                                   */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
#ifndef _SELCBLC_
#define _SELCBLC_

#include "oscl_base.h"
#include "sebase.h"
#include "h245def.h"
#include "h245inf.h"
#include "semsgque.h"

enum LCBLCErrCode
{
    LCBLC_ERROR_CODE_A = 0, // inappropriate message
    LCBLC_ERROR_CODE_B = 1, // inappropriate message
    LCBLC_ERROR_CODE_C = 2, // inappropriate message
    LCBLC_ERROR_CODE_D = 3  // no response from peer LCSE
};

enum LCBLCSource { LCBLC_USER, LCBLC_SE };
enum LCBLCCause
{
    LCBLC_UNSPECIFIED = 0,  /* (NULL) unspecified */
    LCBLC_UNSUITABLE_REVERSE_PARAMETERS, /* (NULL) unsuitableReverseParameters */
    LCBLC_DATA_TYPE_NOT_SUPPORTED, /* (NULL) dataTypeNotSupported */
    LCBLC_DATA_TYPE_NOT_AVAILABLE, /* (NULL) dataTypeNotAvailable */
    LCBLC_UNKNOWN_DATA_TYPE, /* (NULL) unknownDataType */
    LCBLC_DATA_TYPE_AL_COMBINATION_NOT_SUPPORTED, /* (NULL) dataTypeALCombinationNotSupported */
    LCBLC_MULTICAST_CHANNEL_NOT_ALLOWED, /* (NULL) multicastChannelNotAllowed */
    LCBLC_INSUFFICIENT_BANDWIDTH, /* (NULL) insufficientBandwidth */
    LCBLC_SEPERATE_STACK_ESTABLISHMENT_FAILED, /* (NULL) separateStackEstablishmentFailed */
    LCBLC_INVALID_SESSION_ID, /* (NULL) invalidSessionID */
    LCBLC_MASTER_SLAVE_CONFLICT, /* (NULL) masterSlaveConflict */
    LCBLC_WAIT_FOR_COMMUNICATION_MODE, /* (NULL) waitForCommunicationMode */
    LCBLC_INVALID_DEPENDENT_CHANNEL, /* (NULL) invalidDependentChannel */
    LCBLC_REPLACEMENT_FOR_REJECTED /* (NULL) replacementForRejected */
};
enum LCBLCDirection { LCBLC_OUTGOING, LCBLC_INCOMING };

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
class LCBLCObserver
{
    public:
        virtual void LCBLCEstablishIndication(uint32 forwardLcn, PS_ForwardLogicalChannelParameters fwdparam, uint32 reverseLcn, PS_ReverseLogicalChannelParameters revparam) = 0;
        virtual void LCBLCEstablishConfirm(uint32 forwardLcn, uint32 reverseLcn, PS_OpenLogicalChannelAck olca) = 0;
        virtual void LCBLCReleaseIndication(LCBLCSource source, LCBLCCause cause, uint32 ForwardLcn, uint32 ReverseLcn, LCBLCDirection direction) = 0;
        virtual void LCBLCReleaseConfirm(uint32 forwardLcn, uint32 reverseLcn) = 0;
        virtual void LCBLCErrorIndication(LCBLCErrCode error, uint32 forwardLcn, uint32 reverseLcn) = 0;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
class LCBLC : public SEBase, public SETimerObserver
{
    private:
        enum LCBLCState { RELEASED, AWAITING_ESTABLISHMENT, ESTABLISHED, AWAITING_RELEASE }; // states

    public:
        LCBLC(uint32 forwardLcn, LCBLCDirection direction) :
                Observer(NULL),
                State(RELEASED),
                Direction(direction),
                ForwardLcn(forwardLcn),
                ReverseLcn(0)
        {}

        ~LCBLC() {}

        void SetObserver(LCBLCObserver *observer)
        {
            Observer = observer;
        }

        void EstablishRequest(PS_ForwardReverseParam param)
        {
            Print("R e c e i v e d   L C B L C   E s t a b l i s h   R e q u e s t\n");
            if (GetDirection() == LCBLC_OUTGOING)
            {
                switch (GetState())
                {
                    case RELEASED:
                        OutgoingReleased(param);
                        break;
                    case AWAITING_RELEASE:
                        OutgoingAwaitingRelease(param);
                        break;
                    default:
                        break;
                }
            }
        }

        void EstablishResponse()
        {
            Print("R e c e i v e d   L C B L C   E s t a b l i s h   R e s p o n s e\n");
            if (GetDirection() == LCBLC_INCOMING)
            {
                switch (GetState())
                {
                    case AWAITING_ESTABLISHMENT:
                        IncomingAwaitingEstablishment();
                        break;
                    default:
                        break;
                }
            }
        }

        void ReleaseRequest(LCBLCCause cause)
        {
            Print("R e c e i v e d   L C B L C   R e l e a s e   R e q u e s t\n");
            if (GetDirection() == LCBLC_OUTGOING)
            {
                switch (GetState())
                {
                    case AWAITING_ESTABLISHMENT:
                        OutgoingAwaitingEstablishment(cause);
                        break;
                    case ESTABLISHED:
                        OutgoingEstablished(cause);
                        break;
                    default:
                        break;
                }
            }
            else
            {
                switch (GetState())
                {
                    case AWAITING_ESTABLISHMENT:
                        IncomingAwaitingEstablishment(cause);
                        break;
                    default:
                        break;
                }
            }
        }

        void Handler(PS_OpenLogicalChannel olc)
        {
            Print("R e c e i v e d   L C B L C   O p e n   L o g i c a l   C h a n n e l\n");
            if (GetDirection() == LCBLC_INCOMING)
            {
                switch (GetState())
                {
                    case RELEASED:
                        IncomingReleased(olc);
                        break;
                    case AWAITING_ESTABLISHMENT:
                        IncomingAwaitingEstablishment(olc);
                        break;
                    case ESTABLISHED:
                        IncomingEstablished(olc);
                        break;
                    default:
                        break;
                }
            }
        }

        void Handler(PS_CloseLogicalChannel clc)
        {
            Print("R e c e i v e d   L C B L C   C l o s e   L o g i c a l   C h a n n e l\n");
            if (GetDirection() == LCBLC_INCOMING)
            {
                switch (GetState())
                {
                    case RELEASED:
                        IncomingReleased(clc);
                        break;
                    case AWAITING_ESTABLISHMENT:
                        IncomingAwaitingEstablishment(clc);
                        break;
                    case ESTABLISHED:
                        IncomingEstablished(clc);
                        break;
                    default:
                        break;
                }
            }
        }

        void Handler(PS_OpenLogicalChannelAck olca)
        {
            Print("R e c e i v e d   L C B L C   O p e n   L o g i c a l   C h a n n e l   A c k\n");
            if (GetDirection() == LCBLC_OUTGOING)
            {
                switch (GetState())
                {
                    case RELEASED:
                        OutgoingReleased(olca);
                        break;
                    case AWAITING_ESTABLISHMENT:
                        OutgoingAwaitingEstablishment(olca);
                        break;
                    case ESTABLISHED:
                        OutgoingEstablished(olca);
                        break;
                    case AWAITING_RELEASE:
                        OutgoingAwaitingRelease(olca);
                        break;
                    default:
                        break;
                }
            }
        }

        void Handler(PS_OpenLogicalChannelReject olcr)
        {
            Print("R e c e i v e d   L C B L C   O p e n   L o g i c a l   C h a n n e l   R e j e c t\n");
            if (GetDirection() == LCBLC_OUTGOING)
            {
                switch (GetState())
                {
                    case RELEASED:
                        OutgoingReleased(olcr);
                        break;
                    case AWAITING_ESTABLISHMENT:
                        OutgoingAwaitingEstablishment(olcr);
                        break;
                    case ESTABLISHED:
                        OutgoingEstablished(olcr);
                        break;
                    case AWAITING_RELEASE:
                        OutgoingAwaitingRelease(olcr);
                        break;
                    default:
                        break;
                }
            }
        }

        void Handler(PS_CloseLogicalChannelAck clca)
        {
            Print("R e c e i v e d   L C B L C   C l o s e   L o g i c a l   C h a n n e l   A c k\n");
            if (GetDirection() == LCBLC_OUTGOING)
            {
                switch (GetState())
                {
                    case RELEASED:
                        OutgoingReleased(clca);
                        break;
                    case AWAITING_ESTABLISHMENT:
                        OutgoingAwaitingEstablishment(clca);
                        break;
                    case ESTABLISHED:
                        OutgoingEstablished(clca);
                        break;
                    case AWAITING_RELEASE:
                        OutgoingAwaitingRelease(clca);
                        break;
                    default:
                        break;
                }
            }
        }

        void HandlerTimeout()
        {
            Print("R e c e i v e d   L C B L C   T i m e o u t\n");
            if (GetDirection() == LCBLC_OUTGOING)
            {
                switch (GetState())
                {
                    case AWAITING_ESTABLISHMENT:
                        OutgoingAwaitingEstablishmentTimeout();
                        break;
                    case AWAITING_RELEASE:
                        OutgoingAwaitingReleaseTimeout();
                        break;
                    default:
                        break;
                }
            }
        }

        void TimeoutOccurred(int32 timerID, int32 timeoutInfo)
        {
            HandlerTimeout();
        }

    private:
        LCBLC(const LCBLC&);

        void SetState(LCBLCState state)
        {
            State = state;

            if (State == RELEASED)	Print("State -> RELEASED\n");
            else if (State == AWAITING_ESTABLISHMENT) Print("State -> AWAITING_ESTABLISHMENT\n");
            else if (State == ESTABLISHED) Print("State -> ESTABLISHED\n");
            else if (State == AWAITING_RELEASE) Print("State -> AWAITING_RELEASE\n");
        }

        LCBLCState GetState()
        {
            if (State == RELEASED)	Print("State == RELEASED\n");
            else if (State == AWAITING_ESTABLISHMENT) Print("State == AWAITING_ESTABLISHMENT\n");
            else if (State == ESTABLISHED) Print("State == ESTABLISHED\n");
            else if (State == AWAITING_RELEASE) Print("State == AWAITING_RELEASE\n");

            return State;
        }

        void OutgoingReleased(PS_ForwardReverseParam param)
        {
            SetTimer();
            SetState(AWAITING_ESTABLISHMENT);
            SendOpenLogicalChannel(param);
        }

        void OutgoingReleased(PS_ForwardReverseParam param)
        {
            ResetTimer();
            SetTimer();
            SetState(AWAITING_ESTABLISHMENT);
            SendOpenLogicalChannel(param);
        }

        void OutgoingEstablished(LCBLCCause cause)
        {
            SetTimer();
            SetState(AWAITING_RELEASE);
            SendCloseLogicalChannel(LCBLC_USER, cause);
        }

        void OutgoingAwaitingEstablishment(LCBLCCause cause)
        {
            ResetTimer();
            SetTimer();
            SetState(AWAITING_RELEASE);
            SendCloseLogicalChannel(LCBLC_USER, cause);
        }

        void IncomingAwaitingEstablishment()
        {
            SetState(ESTABLISHED);
            SendOpenLogicalChannelAck();
        }

        void IncomingAwaitingEstablishment(LCBLCCause cause)
        {
            SetState(RELEASED);
            SendOpenLogicalChannelReject(cause);
        }

        void IncomingReleased(PS_OpenLogicalChannel olc)
        {
            SetState(AWAITING_ESTABLISHMENT);
            if (Observer) Observer->LCBLCEstablishIndication(ForwardLcn, olc->forwardLogicalChannelParameters, ReverseLcn, olc->reverseLogicalChannelParameters);
        }

        void IncomingAwaitingEstablishment(PS_OpenLogicalChannel olc)
        {
            SetState(AWAITING_ESTABLISHMENT);
            if (Observer)
            {
                Observer->LCBLCReleaseIndication(LCBLC_USER, LCBLC_UNSPECIFIED, ForwardLcn, ReverseLcn, LCBLC_INCOMING);
                Observer->LCBLCEstablishIndication(ForwardLcn, olc->forwardLogicalChannelParameters, ReverseLcn, olc->reverseLogicalChannelParameters);
            }
        }

        void IncomingEstablished(PS_OpenLogicalChannel olc)
        {
            SetState(AWAITING_ESTABLISHMENT);
            if (Observer)
            {
                Observer->LCBLCReleaseIndication(LCBLC_USER, LCBLC_UNSPECIFIED, ForwardLcn, ReverseLcn, LCBLC_INCOMING);
                Observer->LCBLCEstablishIndication(ForwardLcn, olc->forwardLogicalChannelParameters, ReverseLcn, olc->reverseLogicalChannelParameters);
            }
        }

        void IncomingReleased(PS_CloseLogicalChannel clc)
        {
            SetState(RELEASED);
            SendCloseLogicalChannelAck();
        }

        void IncomingAwaitingEstablishment(PS_CloseLogicalChannel clc)
        {
            SetState(RELEASED);
            SendCloseLogicalChannelAck();
            if (Observer)
            {
                Observer->LCBLCReleaseIndication(LCBLC_NONE, LCBLC_UNSPECIFIED, ForwardLcn, ReverseLcn, LCBLC_INCOMING);
            }
        }

        void IncomingEstablished(PS_CloseLogicalChannel clc)
        {
            SetState(RELEASED);
            SendCloseLogicalChannelAck();
            if (Observer)
            {
                Observer->LCBLCReleaseIndication(LCBLC_NONE, LCBLC_UNSPECIFIED, ForwardLcn, ReverseLcn, LCBLC_INCOMING);
            }
        }

        void OutgoingReleased(PS_OpenLogicalChannelAck olca)
        {
            SetState(RELEASED);
            if (Observer) Observer->LCBLCErrorIndication(LCBLC_ERROR_CODE_A, ForwardLcn, ReverseLcn);
        }

        void OutgoingAwaitingEstablishment(PS_OpenLogicalChannelAck olca)
        {
            ResetTimer();
            SetState(ESTABLISHED);
            if (Observer) Observer->LCBLCEstablishConfirm(ForwardLcn, ReverseLcn, olca);
        }

        void OutgoingEstablished(PS_OpenLogicalChannelAck olca)
        {
            SetState(ESTABLISHED);
        }

        void OutgoingAwaitingRelease(PS_OpenLogicalChannelAck olca)
        {
            SetState(AWAITING_RELEASE);
        }

        void OutgoingReleased(PS_OpenLogicalChannelReject olcr)
        {
            SetState(RELEASED);
            if (Observer) Observer->LCBLCErrorIndication(LCBLC_ERROR_CODE_B, ForwardLcn, ReverseLcn);
        }

        void OutgoingAwaitingEstablishment(PS_OpenLogicalChannelReject olcr)
        {
            ResetTimer();
            SetState(RELEASED);
            if (Observer) Observer->LCBLCReleaseIndication(LCBLC_USER, olcr->olcRejectCause.index, ForwardLcn, ReverseLcn, LCBLC_OUTGOING);
        }

        void OutgoingEstablished(PS_OpenLogicalChannelReject olcr)
        {
            SetState(RELEASED);
            if (Observer)
            {
                Observer->LCBLCErrorIndication(LCBLC_ERROR_CODE_B, ForwardLcn, ReverseLcn);
                Observer->LCBLCReleaseIndication(LCBLC_LCSE, LCBLC_UNSPECIFIED, ForwardLcn, ReverseLcn, LCBLC_OUTGOING);
            }
        }

        void OutgoingAwaitingRelease(PS_OpenLogicalChannelReject olcr)
        {
            ResetTimer();
            SetState(RELEASED);
            if (Observer) Observer->ReleaseConfirm();
        }

        void OutgoingReleased(PS_CloseLogicalChannelAck clca)
        {
            SetState(RELEASED);
        }

        void OutgoingAwaitingEstablishment(PS_CloseLogicalChannelAck clca)
        {
            SetState(AWAITING_ESTABLISHMENT);
        }

        void OutgoingEstablished(PS_CloseLogicalChannelAck clca)
        {
            SetState(RELEASED);
            if (Observer)
            {
                Observer->LCBLCErrorIndication(LCBLC_ERROR_CODE_C, ForwardLcn, ReverseLcn);
                Observer->LCBLCReleaseIndication(LCBLC_LCSE, LCBLC_UNSPECIFIED, ForwardLcn, ReverseLcn, LCBLC_OUTGOING);
            }
        }

        void OutgoingAwaitingRelease(PS_CloseLogicalChannelAck clca)
        {
            ResetTimer();
            SetState(RELEASED);
            if (Observer) Observer->ReleaseConfirm();
        }

        void OutgoingAwaitingEstablishmentTimeout()
        {
            SetState(RELEASED);
            if (Observer) Observer->LCBLCErrorIndication(LCBLC_ERROR_CODE_D, ForwardLcn, ReverseLcn);
            SendCloseLogicalChannel(LCBLC_SE, LCBLC_UNSPECIFIED);
            if (Observer) Observer->LCBLCReleaseIndication(LCBLC_LCSE, LCBLC_UNSPECIFIED, ForwardLcn, ReverseLcn, LCBLC_OUTGOING);
        }

        void OutgoingAwaitingReleaseTimeout()
        {
            SetState(RELEASED);
            if (Observer) Observer->LCBLCErrorIndication(LCBLC_ERROR_CODE_D, ForwardLcn, ReverseLcn);
            if (Observer) Observer->LCBLCReleaseConfirm(ForwardLcn, ReverseLcn);
        }

        void SendOpenLogicalChannel(PS_ForwardReverseParam param)
        {
            Print("Sending Open Logical Channel\n");
            S_OpenLogicalChannel   openLogicalChannel ;

            openLogicalChannel.forwardLogicalChannelNumber = ForwardLcn;
            oscl_memcpy((int8*)&openLogicalChannel.forwardLogicalChannelParameters ,
                        (int8*)&param->forwardLogicalChannelParameters ,
                        sizeof(S_ForwardLogicalChannelParameters)) ;

            openLogicalChannel.option_of_reverseLogicalChannelParameters = param->option_of_reverseLogicalChannelParameters ;
            oscl_memcpy((int8*)&openLogicalChannel.reverseLogicalChannelParameters ,
                        (int8*)&param->reverseLogicalChannelParameters ,
                        sizeof(S_ReverseLogicalChannelParameters)) ;

            openLogicalChannel.option_of_separateStack = FALSE ;
            oscl_memset((int8*)&openLogicalChannel.separateStack , 0 , sizeof(S_NetworkAccessParameters)) ;

            openLogicalChannel.option_of_encryptionSync = FALSE;

            MessageSend(H245_MSG_REQ, MSGTYP_LCBLC_OP, (uint8*)&openLogicalChannel) ;
        }

        void SendOpenLogicalChannelAck(PS_ReverseData reverseData = 0)
        {
            Print("Sending Open Logical Channel Ack\n");
            S_OpenLogicalChannelAck    openLogicalChannelAck ;

            oscl_memset(&openLogicalChannelAck, 0, sizeof(S_OpenLogicalChannelAck));

            // Logical Channel Number
            openLogicalChannelAck.forwardLogicalChannelNumber = ForwardLcn;

            if (reverseData)
            {
                // ReverseLogicalChannelParameters
                openLogicalChannelAck.option_of_ackReverseLogicalChannelParameters =
                    p_ReverseData->option_of_reverseLogicalChannelParameters ;

                oscl_memcpy((int8*)&openLogicalChannelAck.ackReverseLogicalChannelParameters ,
                            (int8*)&p_ReverseData->reverseLogicalChannelParameters ,
                            sizeof(S_AckReverseLogicalChannelParameters)) ;

                openLogicalChannelAck.option_of_forwardMultiplexAckParameters =
                    p_ReverseData->option_of_forwardMultiplexAckParameters ;
                oscl_memcpy((int8*)&openLogicalChannelAck.forwardMultiplexAckParameters ,
                            (int8*)&p_ReverseData->forwardMultiplexAckParameters ,
                            sizeof(S_ForwardMultiplexAckParameters)) ;
            }

            openLogicalChannelAck.option_of_separateStack = FALSE ;
            // SeparateStack is OFF
            oscl_memset((int8*)&openLogicalChannelAck.separateStack , 0 , sizeof(S_NetworkAccessParameters)) ;

            // EncryptionSync is OFF
            openLogicalChannelAck.option_of_encryptionSync = FALSE;

            MessageSend(H245_MSG_RPS, MSGTYP_LCBLC_OP_ACK, (uint8*)&openLogicalChannelAck) ;
        }

        void SendOpenLogicalChannelConfirm(PS_OpenLogicalChannelAck openLogicalChannelAck)
        {
            Print("Sending Open Logical Channel Confirm\n", cause);
            S_OpenLogicalChannelConfirm openLogicalChannelConfirm ;

            openLogicalChannelConfirm.forwardLogicalChannelNumber = ForwardLcn;

            MessageSend(H245_MSG_IDC, MSGTYP_LCBLC_OP_CFM, (uint8*)&openLogicalChannelConfirm) ;
        }

        void SendOpenLogicalChannelReject(LCBLCCause cause)
        {
            Print("Sending Open Logical Channel Reject - Cause = %d\n", cause);
            S_OpenLogicalChannelReject openLogicalChannelReject ;

            openLogicalChannelReject.forwardLogicalChannelNumber = ForwardLcn;
            openLogicalChannelReject.olcRejectCause.index = cause;

            MessageSend(H245_MSG_RPS, MSGTYP_LCBLC_OP_RJT, (uint8*)&openLogicalChannelReject) ;
        }

        void SendCloseLogicalChannel(LCBLCSource source)
        {
            Print("Sending Close Logical Channel - Source = %d\n", source);
            S_CloseLogicalChannel  closeLogicalChannel ;

            closeLogicalChannel.forwardLogicalChannelNumber = ForwardLcn;
            closeLogicalChannel.source.index = source;
            closeLogicalChannel.option_of_reason = 1;
            closeLogicalChannel.reason.index = LCBLC_UNSPECIFIED;

            MessageSend(H245_MSG_REQ, MSGTYP_LCBLC_CL, (uint8*)&closeLogicalChannel) ;
        }

        void SendCloseLogicalChannelAck()
        {
            Print("Sending Close Logical Channel Ack\n");
            S_CloseLogicalChannelAck   closeLogicalChannelAck ;

            closeLogicalChannelAck.forwardLogicalChannelNumber = ForwardLcn;

            MessageSend(H245_MSG_RPS, MSGTYP_LCBLC_CL_ACK, (uint8*)&closeLogicalChannelAck) ;
        }

        void SetTimer()
        {
            RequestTimer(103, (int32)this, TimerDuration, this);
        }
        void ResetTimer()
        {
            CancelTimer(103, (int32)this);
        }

        LCBLCObserver *Observer;
        LCBLCState State;
        LCBLCDirection Direction;
        uint32 ForwardLcn;
        uint32 ReverseLcn;
};

#endif /* _SEMSD_ */
