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
/*  file name       : seannex.c                                         */
/*  file contents   : Other Management ( Annex )                        */
/*  draw            : '96.11.28                                         */
/*----------------------------------------------------------------------*/
/*  amendment       :                                                   */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/


/************************************************************************/
/*  Headerfile Include                                                  */
/************************************************************************/
#include "semsgque.h"
#include "h245def.h"
#include "h245pri.h"
#include "annex.h"


/************************************************************************/
/*  function name       : Se_Nsd_1000_XXXX                              */
/*  function outline    : Event     NonStandardMessageRequest Primitive */
/*  function discription: Se_Nsd_1000_XXXX( PS_NonStandardMessage ) */
/*  input data          : PS_NonStandardMessage                     */
/*                      :                      p_NonstandardMessage */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void NSD::_1000_XXXX(PS_NonStandardMessage p_NsmIn)
{
    MsgSend(H245_MSG_REQ , MSGTYP_NSD_REQ , (uint8*)p_NsmIn) ;
}


/************************************************************************/
/*  function name       : Se_Nsd_1001_XXXX                              */
/*  function outline    : Event     NonStandardMessageRequest           */
/*  function discription: Se_Nsd_1001_XXXX( PS_NonStandardMessage )     */
/*  input data          : PS_NonStandardMessage p_NonStandardMessage    */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void NSD::_1001_XXXX(PS_NonStandardMessage p_NonStandardMessage)
{
    PtvSend(H245_PRIMITIVE , E_PtvId_Nsd_Req_Cfm , 0 , 0 , (uint8*)p_NonStandardMessage , sizeof(S_NonStandardMessage)) ;
}


/************************************************************************/
/*  function name       : Se_Nsd_1002_XXXX                              */
/*  function outline    : Event     NonStandardMessageResponse          */
/*                      :                                     Primitive */
/*  function discription: Se_Nsd_1002_XXXX( PS_NonStandardMessage ) */
/*  input data          : PS_NonStandardMessage                     */
/*                      :                      p_NonstandardMessage */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void NSD::_1002_XXXX(PS_NonStandardMessage p_NonstandardMessage)
{
    MsgSend(H245_MSG_RPS , MSGTYP_NSD_RPS , (uint8*)p_NonstandardMessage) ;
}


/************************************************************************/
/*  function name       : Se_Nsd_1003_XXXX                              */
/*  function outline    : Event     NonStandardMessageResponse          */
/*  function discription: Se_Nsd_1003_XXXX( PS_NonStandardMessage )     */
/*  input data          : PS_NonStandardMessage p_NonStandardMessage    */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void NSD::_1003_XXXX(PS_NonStandardMessage p_NonStandardMessage)
{
    PtvSend(H245_PRIMITIVE , E_PtvId_Nsd_Rps_Cfm , 0 , 0 , (uint8*)p_NonStandardMessage , sizeof(S_NonStandardMessage)) ;
}


/************************************************************************/
/*  function name       : Se_Nsd_1004_XXXX                              */
/*  function outline    : Event     NonStandardMessageCommand Primitive */
/*  function discription: Se_Nsd_1004_XXXX( PS_NonStandardMessage ) */
/*  input data          : PS_NonStandardMessage                     */
/*                      :                      p_NonstandardMessage */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void NSD::_1004_XXXX(PS_NonStandardMessage p_NonstandardMessage)
{
    MsgSend(H245_MSG_CMD , MSGTYP_NSD_CMD , (uint8*)p_NonstandardMessage) ;
}


/************************************************************************/
/*  function name       : Se_Nsd_1005_XXXX                              */
/*  function outline    : Event     NonStandardMessageCommand           */
/*  function discription: Se_Nsd_1005_XXXX( PS_NonStandardMessage )     */
/*  input data          : PS_NonStandardMessage p_NonStandardMessage    */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void NSD::_1005_XXXX(PS_NonStandardMessage p_NonStandardMessage)
{
    PtvSend(H245_PRIMITIVE , E_PtvId_Nsd_Cmd_Cfm , 0 , 0 , (uint8*)p_NonStandardMessage , sizeof(S_NonStandardMessage)) ;
}


/************************************************************************/
/*  function name       : Se_Nsd_1006_XXXX                              */
/*  function outline    : Event     NonStandardMessageIndication        */
/*                      :                                     Primitive */
/*  function discription: Se_Nsd_1006_XXXX( PS_NonStandardMessage ) */
/*  input data          : PS_NonStandardMessage                     */
/*                      :                      p_NonstandardMessage */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void NSD::_1006_XXXX(PS_NonStandardMessage p_NonstandardMessage)
{
    MsgSend(H245_MSG_IDC , MSGTYP_NSD_IDC , (uint8*)p_NonstandardMessage) ;
}


/************************************************************************/
/*  function name       : Se_Nsd_1007_XXXX                              */
/*  function outline    : Event     NonStandardMessageIndication        */
/*  function discription: Se_Nsd_1007_XXXX( PS_NonStandardMessage )     */
/*  input data          : PS_NonStandardMessage p_NonStandardMessage    */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void NSD::_1007_XXXX(PS_NonStandardMessage p_NonStandardMessage)
{
    PtvSend(H245_PRIMITIVE , E_PtvId_Nsd_Idc_Cfm , 0 , 0 , (uint8*)p_NonStandardMessage , sizeof(S_NonStandardMessage)) ;
}


/************************************************************************/
/*  function name       : Se_Cm_1100_XXXX                               */
/*  function outline    : Event     CommunicationModeRequest Primitive  */
/*  function discription: Se_Cm_1100_XXXX                               */
/*                      :           ( PS_CommunicationModeRequest ) */
/*  input data          : PS_CommunicationModeRequest               */
/*                      :                p_CommunicationModeRequest */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void CM::_1100_XXXX(PS_CommunicationModeRequest p_CommunicationModeRequest)
{
    MsgSend(H245_MSG_REQ , MSGTYP_CM_REQ , (uint8*)p_CommunicationModeRequest) ;
}


/************************************************************************/
/*  function name       : Se_Cm_1101_XXXX                               */
/*  function outline    : Event     CommunicationModeRequest            */
/*  function discription: Se_Cm_1101_XXXX                               */
/*                      :               ( PS_CommunicationModeRequest ) */
/*  input data          : PS_CommunicationModeRequest                   */
/*                      :                    p_CommunicationModeRequest */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void CM::_1101_XXXX(PS_CommunicationModeRequest p_CommunicationModeRequest)
{
    PtvSend(H245_PRIMITIVE , E_PtvId_Cm_Req_Cfm , 0 , 0 , (uint8*)p_CommunicationModeRequest , sizeof(S_CommunicationModeRequest)) ;
}


/************************************************************************/
/*  function name       : Se_Cm_1102_XXXX                               */
/*  function outline    : Event     CommunicationModeResponse Primitive */
/*  function discription: Se_Cm_1102_XXXX                               */
/*                      :          ( PS_CommunicationModeResponse ) */
/*  input data          : PS_CommunicationModeResponse              */
/*                      :               p_CommunicationModeResponse */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void CM::_1102_XXXX(PS_CommunicationModeResponse p_CommunicationModeResponse)
{
    MsgSend(H245_MSG_RPS , MSGTYP_CM_RPS , (uint8*)p_CommunicationModeResponse) ;
}


/************************************************************************/
/*  function name       : Se_Cm_1103_XXXX                               */
/*  function outline    : Event     CommunicationModeResponse           */
/*  function discription: Se_Cm_1103_XXXX                               */
/*                      :              ( PS_CommunicationModeResponse ) */
/*  input data          : PS_CommunicationModeResponse                  */
/*                      :                   p_CommunicationModeResponse */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void CM::_1103_XXXX(PS_CommunicationModeResponse p_CommunicationModeResponse)
{
    PtvSend(H245_PRIMITIVE , E_PtvId_Cm_Rps_Cfm , 0 , 0 , (uint8*)p_CommunicationModeResponse , sizeof(S_CommunicationModeResponse)) ;
}


/************************************************************************/
/*  function name       : Se_Cm_1104_XXXX                               */
/*  function outline    : Event     CommunicationModeCommand Primitive  */
/*  function discription: Se_Cm_1104_XXXX                               */
/*                      :          ( PS_CommunicationModeCommand )  */
/*  input data          : PS_CommunicationModeCommand               */
/*                      :                p_CommunicationModeCommand*/
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void CM::_1104_XXXX(PS_CommunicationModeCommand p_CommunicationModeCommand)
{
    MsgSend(H245_MSG_CMD , MSGTYP_CM_CMD , (uint8*)p_CommunicationModeCommand) ;
}


/************************************************************************/
/*  function name       : Se_Cm_1105_XXXX                               */
/*  function outline    : Event     CommunicationModeCommand            */
/*  function discription: Se_Cm_1105_XXXX                               */
/*                      :               ( PS_CommunicationModeCommand ) */
/*  input data          : PS_CommunicationModeCommand                   */
/*                      :                    p_CommunicationModeCommand */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void CM::_1105_XXXX(PS_CommunicationModeCommand p_CommunicationModeCommand)
{
    PtvSend(H245_PRIMITIVE , E_PtvId_Cm_Cmd_Cfm , 0 , 0 , (uint8*)p_CommunicationModeCommand , sizeof(S_CommunicationModeCommand)) ;
}

/************************************************************************/
/*  function name       : Se_Cnf_1300_XXXX                              */
/*  function outline    : Event     ConferenceRequest Primitive         */
/*  function discription: void Se_Cnf_1300_XXXX                         */
/*                      :                  ( PS_ConferenceRequest ) */
/*  input data          : PS_ConferenceRequest                      */
/*                      :                       p_ConferenceRequest */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void CNF::_1300_XXXX(PS_ConferenceRequest p_ConferenceRequest)
{
    MsgSend(H245_MSG_REQ , MSGTYP_CNF_REQ , (uint8*)p_ConferenceRequest) ;
}


/************************************************************************/
/*  function name       : Se_Cnf_1301_XXXX                              */
/*  function outline    : Event     ConferenceRequest                   */
/*  function discription: Se_Cnf_1301_XXXX( PS_ConferenceRequest )      */
/*  input data          : PS_ConferenceRequest p_ConferenceRequest      */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void CNF::_1301_XXXX(PS_ConferenceRequest p_ConferenceRequest)
{
    PtvSend(H245_PRIMITIVE , E_PtvId_Cnf_Req_Cfm , 0 , 0 , (uint8*)p_ConferenceRequest , sizeof(S_ConferenceRequest)) ;
}


/************************************************************************/
/*  function name       : Se_Cnf_1302_XXXX                              */
/*  function outline    : Event     ConferenceResponse Primitive        */
/*  function discription: Se_Cnf_1302_XXXX( PS_ConferenceResponse ) */
/*  input data          : PS_ConferenceResponse                     */
/*                      :                      p_ConferenceResponse */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void CNF::_1302_XXXX(PS_ConferenceResponse p_ConferenceResponse)
{
    MsgSend(H245_MSG_RPS , MSGTYP_CNF_RPS , (uint8*)p_ConferenceResponse) ;
}


/************************************************************************/
/*  function name       : Se_Cnf_1303_XXXX                              */
/*  function outline    : Event     ConferenceResponse                  */
/*  function discription: Se_Cnf_1303_XXXX( PS_ConferenceResponse )     */
/*  input data          : PS_ConferenceResponse p_ConferenceResponse    */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void CNF::_1303_XXXX(PS_ConferenceResponse p_ConferenceResponse)
{
    PtvSend(H245_PRIMITIVE , E_PtvId_Cnf_Rps_Cfm , 0 , 0 , (uint8*)p_ConferenceResponse , sizeof(S_ConferenceResponse)) ;
}


/************************************************************************/
/*  function name       : Se_Cmd_1400_XXXX                              */
/*  function outline    : Event     SendTerminalCapabilitySet Primitive */
/*  function discription: Se_Cmd_1400_XXXX                              */
/*                      :          ( PS_SendTerminalCapabilitySet ) */
/*  input data          : PS_SendTerminalCapabilitySet              */
/*                      :               p_SendTerminalCapabilitySet */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void CMD::_1400_XXXX(PS_SendTerminalCapabilitySet p_SendTerminalCapabilitySet)
{
    MsgSend(H245_MSG_CMD , MSGTYP_CMD_STCS , (uint8*)p_SendTerminalCapabilitySet) ;
}


/************************************************************************/
/*  function name       : Se_Cmd_1401_XXXX                              */
/*  function outline    : Event     SendTerminalCapabilitySet           */
/*  function discription: Se_Cmd_1401_XXXX                              */
/*                      :              ( PS_SendTerminalCapabilitySet ) */
/*  input data          : PS_SendTerminalCapabilitySet                  */
/*                      :                   p_SendTerminalCapabilitySet */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void CMD::_1401_XXXX(PS_SendTerminalCapabilitySet p_SendTerminalCapabilitySet)
{
    PtvSend(H245_PRIMITIVE , E_PtvId_Cmd_Stcs_Cfm , 0 , 0 , (uint8*)p_SendTerminalCapabilitySet , sizeof(S_SendTerminalCapabilitySet)) ;
}


/************************************************************************/
/*  function name       : Se_Cmd_1402_XXXX                              */
/*  function outline    : Event     EncryptionCommand Primitive         */
/*  function discription: Se_Cmd_1402_XXXX( PS_EncryptionCommand )  */
/*  input data          : PS_EncryptionCommand                      */
/*                      :                       p_EncryptionCommand */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void CMD::_1402_XXXX(PS_EncryptionCommand p_EncryptionCommand)
{
    MsgSend(H245_MSG_CMD , MSGTYP_CMD_ECRPT , (uint8*)p_EncryptionCommand) ;
}


/************************************************************************/
/*  function name       : Se_Cmd_1403_XXXX                              */
/*  function outline    : Event     EncryptionCommand                   */
/*  function discription: Se_Cmd_1403_XXXX( PS_EncryptionCommand )      */
/*  input data          : PS_EncryptionCommand p_EncryptionCommand      */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void CMD::_1403_XXXX(PS_EncryptionCommand p_EncryptionCommand)
{
    PtvSend(H245_PRIMITIVE , E_PtvId_Cmd_Ecrpt_Cfm , 0 , 0 , (uint8*)p_EncryptionCommand , sizeof(S_EncryptionCommand)) ;
}


/************************************************************************/
/*  function name       : Se_Cmd_1404_XXXX                              */
/*  function outline    : Event     FlowControlCommand Primitive        */
/*  function discription: Se_Cmd_1404_XXXX( PS_FlowControlCommand ) */
/*  input data          : PS_FlowControlCommand                     */
/*                      :                      p_FlowControlCommand */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void CMD::_1404_XXXX(PS_FlowControlCommand p_FlowControlCommand)
{
    MsgSend(H245_MSG_CMD , MSGTYP_CMD_FC , (uint8*)p_FlowControlCommand) ;
}


/************************************************************************/
/*  function name       : Se_Cmd_1405_XXXX                              */
/*  function outline    : Event     FlowControlCommand                  */
/*  function discription: Se_Cmd_1405_XXXX( PS_FlowControlCommand )     */
/*  input data          : PS_FlowControlCommand p_FlowControlCommand    */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void CMD::_1405_XXXX(PS_FlowControlCommand p_FlowControlCommand, uint8* EncodedMsg, uint32 EncodedMsgSize)
{
    PtvSend(H245_PRIMITIVE , E_PtvId_Cmd_Fc_Cfm , 0 , 0 , (uint8*)p_FlowControlCommand , sizeof(S_FlowControlCommand),
            EncodedMsg, EncodedMsgSize) ;
}


/************************************************************************/
/*  function name       : Se_Cmd_1406_XXXX                              */
/*  function outline    : Event                                         */
/*  function discription: Se_Cmd_1406_XXXX( PS_EndSessionCommand )      */
/*  input data          :                                               */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void CMD::_1406_XXXX(PS_EndSessionCommand p_EndSessionCommand)
{
    MsgSend(H245_MSG_CMD , MSGTYP_CMD_ES , (uint8*)p_EndSessionCommand) ;
}


/************************************************************************/
/*  function name       : Se_Cmd_1407_XXXX                              */
/*  function outline    : Event                                         */
/*  function discription: Se_Cmd_1407_XXXX( PS_EndSessionCommand )      */
/*  input data          :                                               */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void CMD::_1407_XXXX(PS_EndSessionCommand p_EndSessionCommand)
{
    PtvSend(H245_PRIMITIVE , E_PtvId_Cmd_Es_Cfm , 0 , 0 , (uint8*)p_EndSessionCommand , sizeof(S_EndSessionCommand)) ;
}


/************************************************************************/
/*  function name       : Se_Cmd_1408_XXXX                              */
/*  function outline    : Event     MiscellaneousCommand Primitive      */
/*  function discription: Se_Cmd_1408_XXXX                              */
/*                      :               ( PS_MiscellaneousCommand ) */
/*  input data          : PS_MiscellaneousCommand                   */
/*                      :                    p_MiscellaneousCommand */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void CMD::_1408_XXXX(PS_MiscellaneousCommand p_MiscellaneousCommand)
{
    MsgSend(H245_MSG_CMD , MSGTYP_CMD_MSCL , (uint8*)p_MiscellaneousCommand) ;
}


/************************************************************************/
/*  function name       : Se_Cmd_1409_XXXX                              */
/*  function outline    : Event     MiscellaneousCommand                */
/*  function discription: Se_Cmd_1409_XXXX( PS_MiscellaneousCommand )   */
/*  input data          : PS_MiscellaneousCommand p_MiscellaneousCommand*/
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void CMD::_1409_XXXX(PS_MiscellaneousCommand p_MiscellaneousCommand, uint8* EncodedMsg, uint32 EncodedMsgSize)
{
    PtvSend(H245_PRIMITIVE , E_PtvId_Cmd_Mscl_Cfm , 0 , 0 , (uint8*)p_MiscellaneousCommand , sizeof(S_MiscellaneousCommand),
            EncodedMsg, EncodedMsgSize) ;
}


/************************************************************************/
/*  function name       : Se_Cmd_140A_XXXX                              */
/*  function outline    : Event     ConferenceCommand Primitive         */
/*  function discription: Se_Cmd_140A_XXXX( PS_ConferenceCommand )  */
/*  input data          : PS_ConferenceCommand                      */
/*                      :                       p_ConferenceCommand */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void CMD::_140A_XXXX(PS_ConferenceCommand p_ConferenceCommand)
{
    MsgSend(H245_MSG_CMD , MSGTYP_CMD_CNF , (uint8*)p_ConferenceCommand) ;
}


/************************************************************************/
/*  function name       : Se_Cmd_140B_XXXX                              */
/*  function outline    : Event     ConferenceCommand                   */
/*  function discription: Se_Cmd_140B_XXXX( PS_ConferenceCommand )      */
/*  input data          : PS_ConferenceCommand p_ConferenceCommand      */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void CMD::_140B_XXXX(PS_ConferenceCommand p_ConferenceCommand)
{
    PtvSend(H245_PRIMITIVE , E_PtvId_Cmd_Cnf_Cfm , 0 , 0 , (uint8*)p_ConferenceCommand , sizeof(S_ConferenceCommand)) ;
}

/************************************************************************/
/*  function name       : Se_Cmd_140C_XXXX                              */
/*  function outline    : Event h223MultiplexReconfiguration Primitive  */
/*  function discription: Se_Cmd_140C_XXXX( PS_h223MultiplexReconfiguration)  */
/*  input data          : PS_h223MultiplexReconfigurationPara                      */
/*                      :                       p_h223MultiplexReconfiguration */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void CMD::_140C_XXXX(PS_H223MultiplexReconfiguration p_h223MultiplexReconfiguration)
{
    MsgSend(H245_MSG_CMD , MSGTYP_CMD_HMR , (uint8*)p_h223MultiplexReconfiguration) ;
}


/************************************************************************/
/*  function name       : Se_Cmd_140D_XXXX                              */
/*  function outline    : Event     h223MultiplexReconfiguration                   */
/*  function discription: Se_Cmd_140D_XXXX( PS_h223MultiplexReconfiguration )      */
/*  input data          : PS_h223MultiplexReconfiguration p_h223MultiplexReconfiguration      */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void CMD::_140D_XXXX(PS_H223MultiplexReconfiguration p_h223MultiplexReconfiguration, uint8* EncodedMsg, uint32 EncodedMsgSize)
{
    PtvSend(H245_PRIMITIVE , E_PtvId_Cmd_Hmr_Cfm , 0 , 0 ,
            (uint8*)p_h223MultiplexReconfiguration , sizeof(S_H223MultiplexReconfiguration),
            EncodedMsg, EncodedMsgSize) ;
}

/************************************************************************/
/*  function name       : Se_Idc_1500_XXXX                              */
/*  function outline    : Event     FunctionNotUnderstood Primitive     */
/*  function discription: Se_Idc_1500_XXXX                              */
/*                      :              ( PS_FunctionNotUnderstood ) */
/*  input data          : PS_FunctionNotUnderstood                  */
/*                      :                   p_FunctionNotUnderstood */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void IDC::_1500_XXXX(PS_FunctionNotUnderstood p_FunctionNotUnderstood)
{
    MsgSend(H245_MSG_IDC , MSGTYP_IDC_FNU , (uint8*)p_FunctionNotUnderstood) ;
}


/************************************************************************/
/*  function name       : Se_Idc_1501_XXXX                              */
/*  function outline    : Event     FunctionNotUnderstood               */
/*  function discription: Se_Idc_1501_XXXX( PS_FunctionNotUnderstood )  */
/*  input data          : PS_FunctionNotUnderstood                      */
/*                      :                       p_FunctionNotUnderstood */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void IDC::_1501_XXXX(PS_FunctionNotUnderstood p_FunctionNotUnderstood)
{
    PtvSend(H245_PRIMITIVE , E_PtvId_Idc_Fnu_Cfm , 0 , 0 , (uint8*)p_FunctionNotUnderstood , sizeof(S_FunctionNotUnderstood)) ;
}


/************************************************************************/
/*  function name       : Se_Idc_1502_XXXX                              */
/*  function outline    : Event     MiscellaneousIndication Primitive   */
/*  function discription: Se_Idc_1502_XXXX                              */
/*                      :            ( PS_MiscellaneousIndication ) */
/*  input data          : PS_MiscellaneousIndication                */
/*                      :                 p_MiscellaneousIndication */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void IDC::_1502_XXXX(PS_MiscellaneousIndication p_MiscellaneousIndication)
{
    MsgSend(H245_MSG_IDC , MSGTYP_IDC_MSCL , (uint8*)p_MiscellaneousIndication) ;
}


/************************************************************************/
/*  function name       : Se_Idc_1503_XXXX                              */
/*  function outline    : Event     MiscellaneousIndication             */
/*  function discription: Se_Idc_1503_XXXX                              */
/*                      :                ( PS_MiscellaneousIndication ) */
/*  input data          : PS_MiscellaneousIndication                    */
/*                      :                     p_MiscellaneousIndication */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void IDC::_1503_XXXX(PS_MiscellaneousIndication p_MiscellaneousIndication)
{
    PtvSend(H245_PRIMITIVE , E_PtvId_Idc_Mscl_Cfm , 0 , 0 , (uint8*)p_MiscellaneousIndication , sizeof(S_MiscellaneousIndication)) ;
}


/************************************************************************/
/*  function name       : Se_Idc_1504_XXXX                              */
/*  function outline    : Event     JitterIndication Primitive          */
/*  function discription: Se_Idc_1504_XXXX( PS_JitterIndication )   */
/*  input data          : PS_JitterIndication                       */
/*                      :                        p_JitterIndication */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void IDC::_1504_XXXX(PS_JitterIndication p_JitterIndication)
{
    MsgSend(H245_MSG_IDC , MSGTYP_IDC_JTR , (uint8*)p_JitterIndication) ;
}


/************************************************************************/
/*  function name       : Se_Idc_1505_XXXX                              */
/*  function outline    : Event     JitterIndication                    */
/*  function discription: Se_Idc_1505_XXXX( PS_JitterIndication )       */
/*  input data          : PS_JitterIndication p_JitterIndication        */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void IDC::_1505_XXXX(PS_JitterIndication p_JitterIndication)
{
    PtvSend(H245_PRIMITIVE , E_PtvId_Idc_Jtr_Cfm , 0 , 0 , (uint8*)p_JitterIndication , sizeof(S_JitterIndication)) ;
}


/************************************************************************/
/*  function name       : Se_Idc_1506_XXXX                              */
/*  function outline    : Event     H223SkewIndication Primitive        */
/*  function discription: Se_Idc_1506_XXXX( PS_H223SkewIndication ) */
/*  input data          : PS_H223SkewIndication                     */
/*                      :                      p_H223SkewIndication */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void IDC::_1506_XXXX(PS_H223SkewIndication p_H223SkewIndication)
{
    MsgSend(H245_MSG_IDC , MSGTYP_IDC_H223SKW , (uint8*)p_H223SkewIndication) ;
}


/************************************************************************/
/*  function name       : Se_Idc_1507_XXXX                              */
/*  function outline    : Event     H223SkewIndication                  */
/*  function discription: Se_Idc_1507_XXXX( PS_H223SkewIndication )     */
/*  input data          : PS_H223SkewIndication p_H223SkewIndication    */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void IDC::_1507_XXXX(PS_H223SkewIndication p_H223SkewIndication)
{
    PtvSend(H245_PRIMITIVE , E_PtvId_Idc_H223skw_Cfm , 0 , 0 , (uint8*)p_H223SkewIndication , sizeof(S_H223SkewIndication)) ;
}


/************************************************************************/
/*  function name       : Se_Idc_1508_XXXX                              */
/*  function outline    : Event     NewATMVCIndication Primitive        */
/*  function discription: Se_Idc_1508_XXXX( PS_NewATMVCIndication ) */
/*  input data          : PS_NewATMVCIndication                     */
/*                      :                      p_NewATMVCIndication */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void IDC::_1508_XXXX(PS_NewATMVCIndication p_NewATMVCIndication)
{
    MsgSend(H245_MSG_IDC , MSGTYP_IDC_NWATMVC , (uint8*)p_NewATMVCIndication) ;
}


/************************************************************************/
/*  function name       : Se_Idc_1509_XXXX                              */
/*  function outline    : Event     NewATMVCIndication                  */
/*  function discription: Se_Idc_1509_XXXX( PS_NewATMVCIndication )     */
/*  input data          : PS_NewATMVCIndication p_NewATMVCIndication    */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void IDC::_1509_XXXX(PS_NewATMVCIndication p_NewATMVCIndication)
{
    PtvSend(H245_PRIMITIVE , E_PtvId_Idc_Nwatmvc_Cfm , 0 , 0 , (uint8*)p_NewATMVCIndication , sizeof(S_NewATMVCIndication)) ;
}


/************************************************************************/
/*  function name       : Se_Idc_150A_XXXX                              */
/*  function outline    : Event     UserInputIndication Primitive       */
/*  function discription: Se_Idc_150A_XXXX( PS_UserInputIndication )*/
/*  input data          : PS_UserInputIndication                    */
/*                      :                     p_UserInputIndication */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void IDC::_150A_XXXX(PS_UserInputIndication p_UserInputIndication)
{
    MsgSend(H245_MSG_IDC , MSGTYP_IDC_UI , (uint8*)p_UserInputIndication) ;
}


/************************************************************************/
/*  function name       : Se_Idc_150B_XXXX                              */
/*  function outline    : Event     UserInputIndication                 */
/*  function discription: Se_Idc_150B_XXXX( PS_UserInputIndication )    */
/*  input data          : PS_UserInputIndication p_UserInputIndication  */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void IDC::_150B_XXXX(PS_UserInputIndication p_UserInputIndication)
{
    PtvSend(H245_PRIMITIVE , E_PtvId_Idc_Ui_Cfm , 0 , 0 , (uint8*)p_UserInputIndication , sizeof(S_UserInputIndication)) ;
}


/************************************************************************/
/*  function name       : Se_Idc_150C_XXXX                              */
/*  function outline    : Event     ConferenceIndication Primitive      */
/*  function discription: Se_Idc_150C_XXXX                              */
/*                      :               ( PS_ConferenceIndication ) */
/*  input data          : PS_ConferenceIndication                   */
/*                      :                    p_ConferenceIndication */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void IDC::_150C_XXXX(PS_ConferenceIndication p_ConferenceIndication)
{
    MsgSend(H245_MSG_IDC , MSGTYP_IDC_CNF , (uint8*)p_ConferenceIndication) ;
}


/************************************************************************/
/*  function name       : Se_Idc_150C_XXXX                              */
/*  function outline    : Event     ConferenceIndication                */
/*  function discription: Se_Idc_150C_XXXX( PS_ConferenceIndication )   */
/*  input data          : PS_ConferenceIndication                       */
/*                      :                        p_ConferenceIndication */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void IDC::_150D_XXXX(PS_ConferenceIndication p_ConferenceIndication)
{
    PtvSend(H245_PRIMITIVE , E_PtvId_Idc_Cnf_Cfm , 0 , 0 , (uint8*)p_ConferenceIndication , sizeof(S_ConferenceIndication)) ;
}


/************************************************************************/
/*  function name       : Se_Idc_150E_XXXX                              */
/*  function outline    : Event     H2250MaximumSkewIndication Primitive*/
/*  function discription: Se_Idc_150E_XXXX                              */
/*                      :         ( PS_H2250MaximumSkewIndication ) */
/*  input data          : PS_H2250MaximumSkewIndication             */
/*                      :              p_H2250MaximumSkewIndication */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void IDC::_150E_XXXX(PS_H2250MaximumSkewIndication p_H2250MaximumSkewIndication)
{
    MsgSend(H245_MSG_IDC , MSGTYP_IDC_H2250MXSKW , (uint8*)p_H2250MaximumSkewIndication) ;
}


/************************************************************************/
/*  function name       : Se_Idc_150F_XXXX                              */
/*  function outline    : Event     H2250MaximumSkewIndication          */
/*  function discription: Se_Idc_150F_XXXX                              */
/*                      :             ( PS_H2250MaximumSkewIndication ) */
/*  input data          : PS_H2250MaximumSkewIndication                 */
/*                      :                  p_H2250MaximumSkewIndication */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void IDC::_150F_XXXX(PS_H2250MaximumSkewIndication p_H2250MaximumSkewIndication)
{
    PtvSend(H245_PRIMITIVE , E_PtvId_Idc_H2250mxskw_Cfm , 0 , 0 , (uint8*)p_H2250MaximumSkewIndication , sizeof(S_H2250MaximumSkewIndication)) ;
}


/************************************************************************/
/*  function name       : Se_Idc_1510_XXXX                              */
/*  function outline    : Event     MCLocationIndication Primitive      */
/*  function discription: Se_Idc_1510_XXXX                              */
/*                      :               ( PS_MCLocationIndication ) */
/*  input data          : PS_MCLocationIndication                   */
/*                      :                    p_McLocationIndication */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void IDC::_1510_XXXX(PS_MCLocationIndication p_McLocationIndication)
{
    MsgSend(H245_MSG_IDC , MSGTYP_IDC_MCLCT , (uint8*)p_McLocationIndication) ;
}


/************************************************************************/
/*  function name       : Se_Idc_1511_XXXX                              */
/*  function outline    : Event     MCLocationIndication                */
/*  function discription: Se_Idc_1511_XXXX( PS_MCLocationIndication )   */
/*  input data          : PS_MCLocationIndication                       */
/*                      :                        p_McLocationIndication */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void IDC::_1511_XXXX(PS_MCLocationIndication p_McLocationIndication)
{
    PtvSend(H245_PRIMITIVE , E_PtvId_Idc_Mclct_Cfm , 0 , 0 , (uint8*)p_McLocationIndication , sizeof(S_MCLocationIndication)) ;
}


/************************************************************************/
/*  function name       : Se_Idc_1512_XXXX                              */
/*  function outline    : Event     VendorIdentification Primitive      */
/*  function discription: Se_Idc_1512_XXXX                              */
/*                      :               ( PS_VendorIdentification ) */
/*  input data          : PS_VendorIdentification                   */
/*                      :                    p_VendorIdentification */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void IDC::_1512_XXXX(PS_VendorIdentification p_VendorIdentification)
{
    MsgSend(H245_MSG_IDC , MSGTYP_IDC_VI , (uint8*)p_VendorIdentification) ;
}


/************************************************************************/
/*  function name       : Se_Idc_1513_XXXX                              */
/*  function outline    : Event     VendorIdentification                */
/*  function discription: Se_Idc_1513_XXXX                              */
/*                      :               ( PS_VendorIdentification )     */
/*  input data          : PS_VendorIdentification                       */
/*                      :                       p_VendorIdentification */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void IDC::_1513_XXXX(PS_VendorIdentification p_VendorIdentification)
{
    PtvSend(H245_PRIMITIVE , E_PtvId_Idc_Vi_Cfm , 0 , 0 , (uint8*)p_VendorIdentification , sizeof(S_VendorIdentification)) ;
}


/************************************************************************/
/*  function name       : Se_Idc_1514_XXXX                              */
/*  function outline    : Event     FunctionNotSupported Primitive      */
/*  function discription: Se_Idc_1514_XXXX                              */
/*                      :               ( PS_FunctionNotSupported ) */
/*  input data          : PS_FunctionNotSupported                   */
/*                      :                    p_FunctionNotSupported */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void IDC::_1514_XXXX(PS_FunctionNotSupported p_FunctionNotSupported)
{
    MsgSend(H245_MSG_IDC , MSGTYP_IDC_FNS , (uint8*)p_FunctionNotSupported) ;
}


/************************************************************************/
/*  function name       : Se_Idc_1515_XXXX                              */
/*  function outline    : Event     FunctionNotSupported                */
/*  function discription: Se_Idc_1515_XXXX( PS_FunctionNotSupported )   */
/*  input data          : PS_FunctionNotSupported                       */
/*                      :                        p_FunctionNotSupported */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void IDC::_1515_XXXX(PS_FunctionNotSupported p_FunctionNotSupported)
{
    PtvSend(H245_PRIMITIVE , E_PtvId_Idc_Fns_Cfm , 0 , 0 , (uint8*)p_FunctionNotSupported , sizeof(S_FunctionNotSupported)) ;
}

/************************************************************************/
/*  function name       : Se_Idc_1516_XXXX                              */
/*  function outline    : Event     FlowControlIndication Primitive     */
/*  function discription: Se_Idc_1516_XXXX                              */
/*                      :               ( PS_FlowControlIndication )    */
/*  input data          : PS_FlowControlIndication                      */
/*                      :                    p_FlowControlIndication    */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void IDC::_1516_XXXX(PS_FlowControlIndication p_FlowControlIndication)
{
    MsgSend(H245_MSG_IDC , MSGTYP_IDC_FC , (uint8*)p_FlowControlIndication) ;
}

/************************************************************************/
/*  function name       : Se_Idc_1517_XXXX                              */
/*  function outline    : Event     FlowControlIndication               */
/*  function discription: Se_Idc_1517_XXXX( PS_FlowControlIndication)   */
/*  input data          : PS_FlowControlIndication                      */
/*                      :                        p_FlowControlIndication*/
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void IDC::_1517_XXXX(PS_FlowControlIndication p_FlowControlIndication)
{
    PtvSend(H245_PRIMITIVE , E_PtvId_Idc_Fc_Cfm , 0 , 0 , (uint8*)p_FlowControlIndication , sizeof(S_FlowControlIndication)) ;
}

/************************************************************************/
/*  function name       : Se_AnnexMsgSend                               */
/*  function outline    : Message Send Function ( Annex )               */
/*  function discription: Se_AnnexMsgSend( uint8 , uint8 , uint8* )     */
/*  input data          : uint8 MessageType                             */
/*                      : uint8 MessageNumber                           */
/*                      : uint8* RealDataPointer                        */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void Annex::MsgSend(uint8 MessageType , uint8 MessageNumber , uint8* RealDataPointer)
{
    S_H245Msg h245msg;

    h245msg.Type1 = MessageType ;
    h245msg.Type2 = MessageNumber ;
    h245msg.pData = RealDataPointer ;

    MessageSend(&h245msg) ;
}


/************************************************************************/
/*  function name       : Se_AnnexPtvSend                               */
/*  function outline    : Primitive Send Function ( Annex )             */
/*  function discription: Se_AnnexMsgSend( uint32 , uint32 , uint32 , uint32 ,  */
/*                      :                                   uint8* uint32 */
/*  input data          : uint32 InfType                                  */
/*                      : uint32 InfId                                    */
/*                      : uint32 InfSupplement1                           */
/*                      : uint32 InfSupplement2                           */
/*                      : uint8* p_Parameter                            */
/*                      : uint32 Size                                     */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void Annex::PtvSend(uint32 InfType , uint32 InfId , uint32 InfSupplement1 , uint32 InfSupplement2 , uint8* p_Parameter , uint32 Size,
                    uint8* EncodedMsg, uint32 EncodedMsgSize)
{
    S_InfHeader                infHeader ;

    infHeader.InfType = InfType ;
    infHeader.InfId = InfId ;
    infHeader.InfSupplement1 = InfSupplement1 ;
    infHeader.InfSupplement2 = InfSupplement2 ;
    infHeader.pParameter = p_Parameter ;
    infHeader.Size = Size ;
    infHeader.EncodedMsg = EncodedMsg;
    infHeader.EncodedMsgSize = EncodedMsgSize;

    PrimitiveSend(&infHeader) ;
}
