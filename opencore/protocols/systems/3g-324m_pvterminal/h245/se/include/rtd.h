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
/*  file name       : sertd.h                                           */
/*  file contents   : Round Trip Deray Signalling Entity                */
/*                  :                                 Management Header */
/*  draw            : '96.11.13                                         */
/*----------------------------------------------------------------------*/
/*  amendment       :                                                   */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
#ifndef _SERTD_H_
#define _SERTD_H_


/************************************************************************/
/*  Headerfile Include                                                  */
/************************************************************************/
#include	"oscl_base.h"
#include	"oscl_time.h"
#include    "h245pri.h"
#include	"sebase.h"

/************************************************************************/
/*  Definition                                                          */
/************************************************************************/
#define     RTD_IDLE                    0X00    /* Idle */
#define     RTD_AWTING_RPS              0X10    /* Awaiting Response */

/************************************************************************/
/*  Function Prototype Declare                                          */
/************************************************************************/
class RTD : public SEBase
{
    public:
        RTD() : SEBase()
        {
            Reset();
        }
        ~RTD() {}

        void Reset()
        {
            OutSqcClear() ;
            T105TimerStop() ;
            TmrSqcNumberClear() ;
            StatusWrite(RTD_IDLE) ;
        }

        void _0800_0000(void) ;
        void _0800_0010(void) ;
        void _0801_0000(PS_RoundTripDelayRequest) ;
        void _0802_0010(PS_RoundTripDelayResponse) ;
        void _0803_0010(int32) ;

    private:
        RTD(const RTD&);

        void MsgRequestSend(void) ;
        void MsgResponseSend(PS_RoundTripDelayRequest) ;
        void PtvTrnsCfmSend(int32) ;
        void PtvExpIndSend(void) ;
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

#ifdef PVANALYZER /* --------SE Analyzer Tool -------- */
        void StatusShow(uint8 oldStatus, uint8 newStatus);
        char* StateLabel(uint8 status);
#endif            /* --------------------------------- */

        void OutSqcClear(void)
        {
            OutSqc = 0;
        }
        void OutSqcInc(void)
        {
            OutSqc++;
            if (OutSqc == 256) OutSqc = 0;
        }
        int32  OutSqcRead(void)
        {
            return OutSqc;
        }
        void TmrSqcNumberClear(void)
        {
            TmrSqcNumber = 0;
        }
        int32  TmrSqcNumberRead(void)
        {
            return TmrSqcNumber;
        }
        void TmrSqcNumberInc(void)
        {
            TmrSqcNumber++;
        }
        void T105TimerStart(void);
        void T105TimerStop(void);
        void StartMeasurement(void);
        int32  StopMeasurement(void);

        int32      OutSqc;
        int32      TmrSqcNumber;
        uint8    Status;
        TimeValue StartTime;
};

#endif /* _SERTD_ */
