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
/*  file name       : semsgque.h                                        */
/*  file contents   : Between Encode/Decode to Control Entity           */
/*                                            Queuing Infomation Header */
/*  draw            : '96.11.11                                         */
/*----------------------------------------------------------------------*/
/*  amendment       :                                                   */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
#ifndef _SEMSGQUE_
#define _SEMSGQUE_


/************************************************************************/
/*  Headerfile Include                                                  */
/************************************************************************/
#include    "h245msg.h"


/************************************************************************/
/*  H.245 Message Type1 Definition                                      */
/************************************************************************/
#define     H245_MSG_REQ            0X00    /* H.245 Request Message */
#define     H245_MSG_RPS            0X01    /* H.245 Response Message */
#define     H245_MSG_CMD            0X02    /* H.245 Command Message */
#define     H245_MSG_IDC            0X03    /* H.245 Indication Message */


/************************************************************************/
/*  H.245 Message Type2 Definition: Request Message                     */
/************************************************************************/
#define     MSGTYP_NSD_REQ          0X00    /* NonStandardMessageRequest */
#define     MSGTYP_MSD              0X01    /* MasterSlaveDetermination */
#define     MSGTYP_CE               0X02    /* TerminalCapabilitySet */
#define     MSGTYP_LCBLC_OP         0X03    /* OpenLogicalChannel ( LC/B-LC ) */
#define     MSGTYP_LCBLC_CL         0X04    /* CloseLogicalChannel ( LC/B-LC ) */
#define     MSGTYP_CLC              0X05    /* RequestChannelClose */
#define     MSGTYP_MT               0X06    /* MultiplexEntrySend */
#define     MSGTYP_RME              0X07    /* RequestMultiplexEntry */
#define     MSGTYP_MR               0X08    /* RequestMode  */
#define     MSGTYP_RTD_REQ          0X09    /* RoundTripDelayRequest */
#define     MSGTYP_ML_REQ           0X0A    /* MaintenanceLoopRequest */
#define     MSGTYP_CM_REQ           0X0B    /* CommunicationModeRequest */
#define     MSGTYP_CNF_REQ          0X0C    /* ConferenceRequest */
#define     MSGTYP_H223AAR          0X0D    /* H223AnnexAReconfiguration */
/*  (Not a codeword - Not supported (RAN)) */


/************************************************************************/
/*  H.245 Message Type2 Definition: Response Message                    */
/************************************************************************/
#define     MSGTYP_NSD_RPS          0X00    /* NonStandardMessageResponse */
#define     MSGTYP_MSD_ACK          0X01    /* MasterSlaveDeterminationAck */
#define     MSGTYP_MSD_RJT          0X02    /* MasterSlaveDeterminationReject */
#define     MSGTYP_CE_ACK           0X03    /* TerminalCapabilitySetAck */
#define     MSGTYP_CE_RJT           0X04    /* TerminalCapabilitySetReject */
#define     MSGTYP_LCBLC_OP_ACK     0X05    /* OpenLogicalChannelAck ( LC/B-LC ) */
#define     MSGTYP_LCBLC_OP_RJT     0X06    /* OpenLogicalChannelReject ( LC/B-LC ) */
#define     MSGTYP_LCBLC_CL_ACK     0X07    /* CloseLogicalChannelAck ( LC/B-LC ) */
#define     MSGTYP_CLC_ACK          0X08    /* RequestChannelCloseAck */
#define     MSGTYP_CLC_RJT          0X09    /* RequestChannelCloseReject */
#define     MSGTYP_MT_ACK           0X0A    /* MultiplexEntrySendAck */
#define     MSGTYP_MT_RJT           0X0B    /* MultiplexEntrySendReject */
#define     MSGTYP_RME_ACK          0X0C    /* RequestMultiplexEntryAck */
#define     MSGTYP_RME_RJT          0X0D    /* RequestMultiplexEntryReject */
#define     MSGTYP_MR_ACK           0X0E    /* RequestModeAck */
#define     MSGTYP_MR_RJT           0X0F    /* RequestModeReject */
#define     MSGTYP_RTD_RPS          0X10    /* RoundTripDelayResponse */
#define     MSGTYP_ML_ACK           0X11    /* MaintenanceLoopAck */
#define     MSGTYP_ML_RJT           0X12    /* MaintenanceLoopReject */
#define     MSGTYP_CM_RPS           0X13    /* CommunicationModeResponse */
#define     MSGTYP_CNF_RPS          0X14    /* ConferenceResponse */
#define     MSGTYP_H223AAR_ACK      0X15    /* H223AnnexAReconfigurationAck */
#define     MSGTYP_H223AAR_RJT      0X16    /* H223AnnexAReconfigurationReject */
/*  (Not a codeword -- Not supported (RAN)) */


/************************************************************************/
/*  H.245 Message Type2 Definition: Command Message                     */
/************************************************************************/
#define     MSGTYP_NSD_CMD          0X00    /* NonStandardMessageCommand */
#define     MSGTYP_ML_OFF_CMD       0X01    /* MaintenanceLoopOffCommand */
#define     MSGTYP_CMD_STCS         0X02    /* SendTerminalCapabilitySet */
#define     MSGTYP_CMD_ECRPT        0X03    /* EncryptionCommand */
#define     MSGTYP_CMD_FC           0X04    /* FlowControlCommand */
#define     MSGTYP_CMD_ES           0X05    /* EndSessionCommand */
#define     MSGTYP_CMD_MSCL         0X06    /* MiscellaneousCommand */
#define     MSGTYP_CM_CMD           0X07    /* CommunicationModeCommand */
#define     MSGTYP_CMD_CNF          0X08    /* ConferenceCommand */
#define     MSGTYP_CMD_HMR          0X09    /* H223MultiplexReconfigurationCommand */


/************************************************************************/
/*  H.245 Message Type2 Definition: Indication Message                  */
/************************************************************************/
#define     MSGTYP_NSD_IDC          0X00    /* NonStandardMessage */
#define     MSGTYP_IDC_FNU          0X01    /* FunctionNotUnderstood */
#define     MSGTYP_MSD_RLS          0X02    /* MasterSlaveDeterminationRelease */
#define     MSGTYP_CE_RLS           0X03    /* TerminalCapabilitySetRelease */
#define     MSGTYP_LCBLC_OP_CFM     0X04    /* OpenLogicalChannelConfirm ( Bi-Directional Only ) */
#define     MSGTYP_CLC_RLS          0X05    /* RequestChannelCloseRelease */
#define     MSGTYP_MT_RLS           0X06    /* MultiplexEntrySendRelease */
#define     MSGTYP_RME_RLS          0X07    /* RequestMultiplexEntryRelease */
#define     MSGTYP_MR_RLS           0X08    /* RequestModeRelease */
#define     MSGTYP_IDC_MSCL         0X09    /* MiscellaneousIndication */
#define     MSGTYP_IDC_JTR          0X0A    /* JitterIndication */
#define     MSGTYP_IDC_H223SKW      0X0B    /* H223SkewIndication */
#define     MSGTYP_IDC_NWATMVC      0X0C    /* NewATMVCIndication */
#define     MSGTYP_IDC_UI           0X0D    /* UserInputIndication */
#define     MSGTYP_IDC_H2250MXSKW   0X0E    /* H2250MaximumSkewIndication */
#define     MSGTYP_IDC_MCLCT        0X0F    /* MCLocationIndication */
#define     MSGTYP_IDC_CNF          0X10    /* ConferenceIndication */
#define     MSGTYP_IDC_VI           0X11    /* VendorIdentification */
#define     MSGTYP_IDC_FNS          0X12    /* FunctionNotSupported */
#define     MSGTYP_IDC_FC           0X13    /* FlowControl */


#endif /* _SEMSGQUE_ */
