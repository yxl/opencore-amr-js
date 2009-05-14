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
/*  file name       : selc.h                                            */
/*  file contents   : Uni-Directional Logical Channel Signalling Entity */
/*                  :                                 Management Header */
/*  draw            : '96.11.11                                         */
/*----------------------------------------------------------------------*/
/*  amendment       :                                                   */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
#ifndef _SELC_H_
#define _SELC_H_


/************************************************************************/
/*  Headerfile Include                                                  */
/************************************************************************/
#include    "h245pri.h"
#include    "lcblccmn.h"

class MSD;

/************************************************************************/
/*  Function Prototype Declare                                          */
/************************************************************************/
class LC : public LCBLCCmn
{
    public:
        LC() : LCBLCCmn(), MyMSD(0) {}
        ~LC() {}

        void SetMSD(MSD *msd)
        {
            MyMSD = msd;
        }

        void Reset() {}

        void _0200_0000(LCEntry* , PS_ForwardReverseParam) ;
        void _0200_0030(LCEntry* , PS_ForwardReverseParam) ;
        void _0201_0010(LCEntry* , PS_OpenLogicalChannel) ;
        void _0203_0010(LCEntry* , PS_OlcRejectCause) ;
        void _0203_0020(LCEntry* , PS_OlcRejectCause) ;
        void _0204_0000(LCEntry* , PS_OpenLogicalChannelAck) ;
        void _0204_0010(LCEntry* , PS_OpenLogicalChannelAck) ;
        void _0206_0000(LCEntry* , PS_OpenLogicalChannelReject) ;
        void _0206_0010(LCEntry* , PS_OpenLogicalChannelReject) ;
        void _0206_0020(LCEntry* , PS_OpenLogicalChannelReject) ;
        void _0206_0030(LCEntry* , PS_OpenLogicalChannelReject) ;
        void _0207_0010(LCEntry* , int32) ;
        void _0207_0030(LCEntry* , int32) ;
        void _0209_0020(LCEntry* , PS_CloseLogicalChannelAck) ;
        void _0209_0030(LCEntry* , PS_CloseLogicalChannelAck) ;
        void _0201_0001(LCEntry* , PS_OpenLogicalChannel) ;
        void _0201_0011(LCEntry* , PS_OpenLogicalChannel) ;
        void _0202_0011(LCEntry* , PS_ReverseData) ;
        void _0203_0011(LCEntry* , PS_OlcRejectCause) ;
        void _0208_0001(LCEntry* , PS_CloseLogicalChannel) ;
        void _0208_0011(LCEntry* , PS_CloseLogicalChannel) ;

    private:
        LC(const LC&);
        MSD *MyMSD;
};

#endif /* _SELC_ */
