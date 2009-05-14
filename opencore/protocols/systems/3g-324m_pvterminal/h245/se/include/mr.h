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
/*  file name       : semr.h                                            */
/*  file contents   : Mode Request Signalling Entity                    */
/*                  :                                 Management Header */
/*  draw            : '96.11.11                                         */
/*----------------------------------------------------------------------*/
/*  amendment       :                                                   */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
#ifndef _SEMR_H_
#define _SEMR_H_


/************************************************************************/
/*  Headerfile Include                                                  */
/************************************************************************/
#include    "h245pri.h"
#include	"sebase.h"

/************************************************************************/
/*  Definition                                                          */
/************************************************************************/
#define     MR_OUTGOING_IDLE            0X00    /* Outgoing Idle */
#define     MR_OUTGOING_AWTING_RPS      0X10    /* Outgoing Awaiting Response */
#define     MR_INCOMING_IDLE            0X01    /* Incoming Idle */
#define     MR_INCOMING_AWTING_RPS      0X11    /* Incoming Awaiting Response */


/************************************************************************/
/*  Function Prototype Declare                                          */
/************************************************************************/
class MR : public SEBase
{
    public:
        MR() : SEBase()
        {
            Reset();
        }
        ~MR() {}

        void Reset()
        {
            OutSqcClear() ;
            InSqcClear() ;
            T109TimerStop() ;
            TmrSqcNumberClear() ;
            StatusWrite(MR_INCOMING_IDLE) ;
            StatusWrite(MR_OUTGOING_IDLE) ;
        }
        void _0700_0000(PS_RequestMode) ;
        void _0700_0010(PS_RequestMode) ;
        void _0701_0001(PS_RequestMode) ;
        void _0701_0011(PS_RequestMode) ;
        void _0702_0011(PS_Response) ;
        void _0703_0011(PS_RmRejectCause) ;
        void _0704_0010(PS_RequestModeAck) ;
        void _0705_0010(PS_RequestModeReject) ;
        void _0706_0010(int32) ;
        void _0707_0011(void) ;

    private:
        MR(const MR&);

        void MsgRequestSend(PS_RequestMode) ;
        void MsgAckSend(PS_Response) ;
        void MsgRjtSend(PS_RmRejectCause) ;
        void MsgRlsSend(void) ;
        void PtvTrnsIndSend(PS_RequestMode) ;
        void PtvRjtIndSend(int32 , PS_RmRejectCause) ;
        void PtvTrnsCfmSend(PS_RequestModeAck) ;
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
            if (status&0x01)  /* Incoming */
            {
#ifdef PVANALYZER
                StatusShow(Status2, status);
#endif
                Status2 = status ;
            }
            else  /* Outgoing */
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
        void T109TimerStart(void) ;
        void T109TimerStop(void) ;

        int32      OutSqc;
        int32      InSqc;
        int32      TmrSqcNumber;
        uint8    Status;
        uint8    Status2;
};

#endif /* _SEMR_ */
