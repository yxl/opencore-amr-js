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
/*  file name       : seml.h                                            */
/*  file contents   : Maintenance Loop Signalling Entity                */
/*                  :                                 Management Header */
/*  draw            : '96.11.11                                         */
/*----------------------------------------------------------------------*/
/*  amendment       :                                                   */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
#ifndef _SEML_H_
#define _SEML_H_


/************************************************************************/
/*  Headerfile Include                                                  */
/************************************************************************/
#include    <string.h>
#include    "h245pri.h"
#include	"sebase.h"

/************************************************************************/
/*  Definition                                                          */
/************************************************************************/
#define    ML_OUTGOING_NOT_LOOPED       0X00    /* Outgoing Not Looped */
#define    ML_OUTGOING_AWTING_RPS       0X10    /* Outgoing Awaiting Response */
#define    ML_OUTGOING_LOOPED           0X20    /* Outgoing Looped */
#define    ML_INCOMING_NOT_LOOPED       0X01    /* Incoming Not Looped */
#define    ML_INCOMING_AWTING_RPS       0X11    /* Incoming Awaiting Response */
#define    ML_INCOMING_LOOPED           0X21    /* Incoming Looped */

/************************************************************************/
/*  Function Prototype Declare                                          */
/************************************************************************/

class ML : public SEBase
{
    public:
        enum InOut { OUTGOING, INCOMING };

        ML(InOut inout = OUTGOING) : SEBase()
        {
            T102TimerStop() ;
            TmrSqcNumberClear() ;
            if (inout == INCOMING) StatusWrite(ML_INCOMING_NOT_LOOPED);
            else StatusWrite(ML_OUTGOING_NOT_LOOPED);
        }
        ~ML() {}

        void Reset()
        {
            T102TimerStop() ;
            TmrSqcNumberClear() ;
            if (StatusRead() & 0x01 /*INCOMING*/) StatusWrite(ML_INCOMING_NOT_LOOPED);
            else StatusWrite(ML_OUTGOING_NOT_LOOPED);
        }

        void _0900_0000(PS_MlRequestType) ;
        void _0901_0010(PS_MlRejectCause) ;
        void _0901_0011(PS_MlRejectCause) ;
        void _0901_0020(PS_MlRejectCause) ;
        void _0902_0001(PS_MaintenanceLoopRequest) ;
        void _0902_0011(PS_MaintenanceLoopRequest) ;
        void _0903_0011(void) ;
        void _0904_0010(PS_MaintenanceLoopAck) ;
        void _0905_0010(PS_MaintenanceLoopReject) ;
        void _0905_0020(PS_MaintenanceLoopReject) ;
        void _0906_0010(int32) ;
        void _0907_0011(void) ;

    private:
        ML(const ML&);

        void MsgLoopReqSend(PS_MlRequestType);
        void MsgLoopAckSend(void);
        void MsgLoopRjtSend(PS_MlRejectCause);
        void MsgLoopOffSend(void);
        void PtvLoopIdcSend(PS_MaintenanceLoopRequest);
        void PtvRlsIdcSend(int32, PS_MlRejectCause);
        void PtvLoopCfmSend(void);
        void PtvErrIdcSend(ENUM_ErrCode);
    public:
        uint8 StatusRead(void)
        {
            return Status;
        }
    private:
        void StatusWrite(uint8 status)
        {
#ifdef PVANALYZER
            StatusShow(Status, status);
#endif
            Status = status ;
        }
#ifdef PVANALYZER
        void StatusShow(uint8 oldStatus, uint8 newStatus);
        char* StateLabel(uint8 status);
#endif
        void LoopTypeRead(PS_MlRequestType pType)
        {
            oscl_memcpy((int8*)pType, (int8*)&LoopType, sizeof(S_MlRequestType));
        }
        void LoopTypeWrite(PS_MlRequestType pType)
        {
            oscl_memcpy((int8*)&LoopType, (int8*)pType, sizeof(S_MlRequestType));
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
        void T102TimerStart(void);
        void T102TimerStop(void);

        int32		TmrSqcNumber;
        uint8	Status;
        S_MlRequestType LoopType ;
};
#endif /* _SEML_ */
