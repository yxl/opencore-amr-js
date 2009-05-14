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
/*  file name       : seblc.h                                           */
/*  file contents   : Bi-Directional Logical Channel Signalling Entity  */
/*                  :                                 Management Header */
/*  draw            : '96.11.11                                         */
/*----------------------------------------------------------------------*/
/*  amendment       :                                                   */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
#ifndef _SEBLC_H_
#define _SEBLC_H_

/************************************************************************/
/*  Headerfile Include                                                  */
/************************************************************************/
#include "h245pri.h"
#include "lcblccmn.h"

class MSD;

/************************************************************************/
/*  Function Prototype Declare                                          */
/************************************************************************/
class BLC : public LCBLCCmn
{
    public:
        BLC() : LCBLCCmn(), MyMSD(0) {}
        ~BLC() {}

        void SetMSD(MSD *msd)
        {
            MyMSD = msd;
        }

        void Reset() {}

        void _0300_0000(LCEntry*, PS_ForwardReverseParam) ;
        void _0300_0030(LCEntry*, PS_ForwardReverseParam) ;
        void _0301_0010(LCEntry*, PS_OpenLogicalChannel) ;
        void _0303_0010(LCEntry*, PS_OlcRejectCause) ;
        void _0303_0020(LCEntry*, PS_OlcRejectCause) ;
        void _0304_0000(LCEntry*, PS_OpenLogicalChannelAck) ;
        void _0304_0010(LCEntry*, PS_OpenLogicalChannelAck) ;
        void _0306_0000(LCEntry*, PS_OpenLogicalChannelReject) ;
        void _0306_0010(LCEntry*, PS_OpenLogicalChannelReject) ;
        void _0306_0020(LCEntry*, PS_OpenLogicalChannelReject) ;
        void _0306_0030(LCEntry*, PS_OpenLogicalChannelReject) ;
        void _0307_0010(LCEntry*, int32) ;
        void _0307_0030(LCEntry*, int32) ;
        void _0309_0020(LCEntry*, PS_CloseLogicalChannelAck) ;
        void _0309_0030(LCEntry*, PS_CloseLogicalChannelAck) ;
        void _0301_0001(LCEntry*, PS_OpenLogicalChannel) ;
        void _0301_0011(LCEntry*, PS_OpenLogicalChannel) ;
        void _0301_0021(LCEntry*, PS_OpenLogicalChannel) ;
        void _0302_0011(LCEntry*, PS_ReverseData , uint16) ;
        void _0303_0011(LCEntry*, PS_OlcRejectCause) ;
        void _0305_0011(LCEntry*, PS_OpenLogicalChannelConfirm) ;
        void _0305_0021(LCEntry*, PS_OpenLogicalChannelConfirm) ;
        void _0307_0021(LCEntry*, int32) ;
        void _0308_0001(LCEntry*, PS_CloseLogicalChannel) ;
        void _0308_0011(LCEntry*, PS_CloseLogicalChannel) ;
        void _0308_0021(LCEntry*, PS_CloseLogicalChannel) ;

    private:
        BLC(const BLC&);
        MSD *MyMSD;
};

#endif /* _SEBLC_ */
