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
/*  file name       : seannex.h                                         */
/*  file contents   : Other Management Header ( Annex )                 */
/*  draw            : '96.11.28                                         */
/*----------------------------------------------------------------------*/
/*  amendment       :                                                   */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
#ifndef _SEANNEX_H_
#define _SEANNEX_H_


/************************************************************************/
/*  Headerfile Include                                                  */
/************************************************************************/
#include "h245pri.h"
#include "h245def.h"
#include "sebase.h"

/************************************************************************/
/*  Function Prototype Declare                                          */
/************************************************************************/
class Annex : public SEBase
{
    public:
        Annex() : SEBase() {}
        ~Annex() {}
        void Reset() {}
        void MsgSend(uint8 , uint8 , uint8*) ;
        void PtvSend(uint32 , uint32 , uint32 , uint32 , uint8* , uint32, uint8* EncodedMsg = NULL, uint32 EncodedMsgSize = 0) ;

    private:
        Annex(const Annex&);
};

class NSD : public Annex
{
    public:
        NSD() : Annex() {}
        ~NSD() {}
        void _1000_XXXX(PS_NonStandardMessage) ;
        void _1001_XXXX(PS_NonStandardMessage) ;
        void _1002_XXXX(PS_NonStandardMessage) ;
        void _1003_XXXX(PS_NonStandardMessage) ;
        void _1004_XXXX(PS_NonStandardMessage) ;
        void _1005_XXXX(PS_NonStandardMessage) ;
        void _1006_XXXX(PS_NonStandardMessage) ;
        void _1007_XXXX(PS_NonStandardMessage) ;
    private:
        NSD(const NSD&);
};

class CM : public Annex
{
    public:
        CM() : Annex() {}
        ~CM() {}
        void _1100_XXXX(PS_CommunicationModeRequest) ;
        void _1101_XXXX(PS_CommunicationModeRequest) ;
        void _1102_XXXX(PS_CommunicationModeResponse) ;
        void _1103_XXXX(PS_CommunicationModeResponse) ;
        void _1104_XXXX(PS_CommunicationModeCommand) ;
        void _1105_XXXX(PS_CommunicationModeCommand) ;
    private:
        CM(const NSD&);
};

class CNF : public Annex
{
    public:
        CNF() : Annex() {}
        ~CNF() {}
        void _1300_XXXX(PS_ConferenceRequest) ;
        void _1301_XXXX(PS_ConferenceRequest) ;
        void _1302_XXXX(PS_ConferenceResponse) ;
        void _1303_XXXX(PS_ConferenceResponse) ;
    private:
        CNF(const NSD&);
};

class CMD : public Annex
{
    public:
        CMD() : Annex() {}
        ~CMD() {}
        void _1400_XXXX(PS_SendTerminalCapabilitySet) ;
        void _1401_XXXX(PS_SendTerminalCapabilitySet) ;
        void _1402_XXXX(PS_EncryptionCommand) ;
        void _1403_XXXX(PS_EncryptionCommand) ;
        void _1404_XXXX(PS_FlowControlCommand) ;
        void _1405_XXXX(PS_FlowControlCommand, uint8* EncodedMsg = NULL, uint32 EncodedMsgSize = 0) ;
        void _1406_XXXX(PS_EndSessionCommand) ;
        void _1407_XXXX(PS_EndSessionCommand) ;
        void _1408_XXXX(PS_MiscellaneousCommand) ;
        void _1409_XXXX(PS_MiscellaneousCommand, uint8* EncodedMsg = NULL, uint32 EncodedMsgSize = 0) ;
        void _140A_XXXX(PS_ConferenceCommand) ;
        void _140B_XXXX(PS_ConferenceCommand) ;
        void _140C_XXXX(PS_H223MultiplexReconfiguration) ;
        void _140D_XXXX(PS_H223MultiplexReconfiguration, uint8* EncodedMsg = NULL, uint32 EncodedMsgSize = 0) ;
    private:
        CMD(const NSD&);
};

class IDC : public Annex
{
    public:
        IDC() : Annex() {}
        ~IDC() {}
        void _1500_XXXX(PS_FunctionNotUnderstood) ;
        void _1501_XXXX(PS_FunctionNotUnderstood) ;
        void _1502_XXXX(PS_MiscellaneousIndication) ;
        void _1503_XXXX(PS_MiscellaneousIndication) ;
        void _1504_XXXX(PS_JitterIndication) ;
        void _1505_XXXX(PS_JitterIndication) ;
        void _1506_XXXX(PS_H223SkewIndication) ;
        void _1507_XXXX(PS_H223SkewIndication) ;
        void _1508_XXXX(PS_NewATMVCIndication) ;
        void _1509_XXXX(PS_NewATMVCIndication) ;
        void _150A_XXXX(PS_UserInputIndication) ;
        void _150B_XXXX(PS_UserInputIndication) ;
        void _150C_XXXX(PS_ConferenceIndication) ;
        void _150D_XXXX(PS_ConferenceIndication) ;
        void _150E_XXXX(PS_H2250MaximumSkewIndication) ;
        void _150F_XXXX(PS_H2250MaximumSkewIndication) ;
        void _1510_XXXX(PS_MCLocationIndication) ;
        void _1511_XXXX(PS_MCLocationIndication) ;
        void _1512_XXXX(PS_VendorIdentification) ;
        void _1513_XXXX(PS_VendorIdentification) ;
        void _1514_XXXX(PS_FunctionNotSupported) ;
        void _1515_XXXX(PS_FunctionNotSupported) ;
        void _1516_XXXX(PS_FlowControlIndication) ;
        void _1517_XXXX(PS_FlowControlIndication) ;
    private:
        IDC(const IDC&);
};

#endif /* _SEANNEX_ */
