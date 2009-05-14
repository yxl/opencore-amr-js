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
/*  file name       : sedisp.h                                          */
/*  file contents   : Receive Data Dispatch Header                      */
/*  draw            : '96.11.11                                         */
/*----------------------------------------------------------------------*/
/*  amendment       :                                                   */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
#ifndef _SE_H_
#define _SE_H_


/************************************************************************/
/*  Headerfile Include                                                  */
/************************************************************************/
#include	"oscl_base.h"
#include	"oscl_timer.h"
#include    "oscl_mem.h"
#include    "oscl_shared_ptr.h"
#include    "h245inf.h"
#include    "h245msg.h"
#include	"msd.h"
#include	"blc.h"
#include	"lc.h"
#include	"clc.h"
#include	"ce.h"
#include	"mt.h"
#include	"rtd.h"
#include	"ml.h"
#include	"mr.h"
#include	"rme.h"
#include	"annex.h"
#include	"lcentry.h"

class H245;
class PER;
class PVLogger;

/************************************************************************/
/*  Signalling Entity Definition                                        */
/************************************************************************/
#define     EVENT_MSDSE                 0X00000000      /* Master Slave Determination Signalling Entity */
#define     EVENT_CESE                  0X01000000      /* Capability Exchange Signalling Entity */
#define     EVENT_LCBLCSE               0X23000000      /* Uni-Directional and Bi-Directional Logical Channel Signalling Entity */
/* Note : Befor Dispatch */
#define     EVENT_LCSE                  0X02000000      /* Uni-Directional Logical Channel Signalling Entity */
/* Note : After Dispatch */
#define     EVENT_BLCSE                 0X03000000      /* Bi-Directional Logical Channel Signalling Entity */
/* Note : After Dispatch */
#define     EVENT_CLCSE                 0X04000000      /* Close Logical Channel Signalling Entity */
#define     EVENT_MTSE                  0X05000000      /* Multiplex Table Signalling Entity */
#define     EVENT_RMESE                 0X06000000      /* Request Multiplex Entry Signalling Entity */
#define     EVENT_MRSE                  0X07000000      /* Mode Request Signalling Entity */
#define     EVENT_RTDSE                 0X08000000      /* Round Trip Deray Signalling Entity */
#define     EVENT_MLSE                  0X09000000      /* Maintenance Loop Signalling Entity */

/************************************************************************/
/*  Non Standard Definition                                             */
/************************************************************************/
#define     EVENT_NSD                   0X10000000      /* Non Standard */


/************************************************************************/
/*  Communication Mode Definition                                       */
/************************************************************************/
#define     EVENT_CM                    0X11000000      /* Communication Mode */

/************************************************************************/
/*  H223 Annex A Reconfiguration Definition                             */
/************************************************************************/
#define     EVENT_H223AAR               0X12000000      /* H223 Annex A Reconfiguration */


/************************************************************************/
/*  Conference Request And Response Definition                          */
/************************************************************************/
#define     EVENT_CNF                   0X13000000      /* Conference Request And Response */


/************************************************************************/
/*  Command Definition                                                  */
/************************************************************************/
#define     EVENT_CMD                   0X14000000      /* Command */


/************************************************************************/
/*  Indication Definition                                               */
/************************************************************************/
#define     EVENT_IDC                   0X15000000      /* Indication */


/************************************************************************/
/*  Event No Definition                                                 */
/************************************************************************/
/* Stopper */
#define     STOPPER                     0XFFFFFFFF                  /* Stopper */


/* Master Slave Determination Signalling Entity ( MSDSE )  */
//#define     EVENT_PTV_MSD_DTM_REQ       EVENT_MSDSE   | 0X00000000  /* DETERMINE.request */
#define     EVENT_MSG_MSD               EVENT_MSDSE   | 0X00010000  /* MasterSlaveDetermination */
#define     EVENT_MSG_MSD_ACK           EVENT_MSDSE   | 0X00020000  /* MasterSlaveDeterminationAck */
#define     EVENT_MSG_MSD_RJT           EVENT_MSDSE   | 0X00030000  /* MasterSlaveDeterminationReject */
#define     EVENT_TIM_MSD_T106          EVENT_MSDSE   | 0X00040000  /* Timer T106 Timeout */
#define     EVENT_MSG_MSD_RLS           EVENT_MSDSE   | 0X00050000  /* MasterSlaveDeterminationRelease */


/* Capability Exchange Signalling Entity ( CESE ) */
//#define     EVENT_PTV_CE_TRF_REQ        EVENT_CESE    | 0X00000000  /* TRANSFER.request */
#define     EVENT_MSG_CE                EVENT_CESE    | 0X00010000  /* TerminalCapabilitySet */
//#define     EVENT_PTV_CE_TRF_RPS        EVENT_CESE    | 0X00020000  /* TRANSFER.response */
//#define     EVENT_PTV_CE_RJT_REQ        EVENT_CESE    | 0X00030000  /* REJECT.request */
#define     EVENT_MSG_CE_ACK            EVENT_CESE    | 0X00040000  /* TerminalCapabilitySetAck */
#define     EVENT_MSG_CE_RJT            EVENT_CESE    | 0X00050000  /* TerminalCapabilitySetReject */
#define     EVENT_TIM_CE_T101           EVENT_CESE    | 0X00060000  /* Timer T101 Timeout */
#define     EVENT_MSG_CE_RLS            EVENT_CESE    | 0X00070000  /* TerminalCapabilitySetRelease */


/* Uni-Directional Logical Channel Signalling Entity ( LCSE ) */
/* Bi-Directional Logical Channel Signalling Entity ( B-LCSE ) */
#define     EVENT_PTV_LCBLC_ETB_REQ     EVENT_LCBLCSE | 0X00000000  /* ESTABLISH.request */
#define     EVENT_MSG_LCBLC_OP          EVENT_LCBLCSE | 0X00010000  /* OpenLogicalChannel */
#define     EVENT_PTV_LCBLC_ETB_RPS     EVENT_LCBLCSE | 0X00020000  /* ESTABLISH.response */
#define     EVENT_PTV_LCBLC_RLS_REQ     EVENT_LCBLCSE | 0X00030000  /* RELEASE.request */
#define     EVENT_MSG_LCBLC_OP_ACK      EVENT_LCBLCSE | 0X00040000  /* OpenLogicalChannelAck */
#define     EVENT_MSG_LCBLC_OP_CFM      EVENT_LCBLCSE | 0X00050000  /* OpenLogicalChannelConfirm ( Bi-Directional Only ) */
#define     EVENT_MSG_LCBLC_OP_RJT      EVENT_LCBLCSE | 0X00060000  /* OpenLogicalChannelReject */
#define     EVENT_TIM_LCBLC_T103        EVENT_LCBLCSE | 0X00070000  /* Timer T103 Timeout */
#define     EVENT_MSG_LCBLC_CL          EVENT_LCBLCSE | 0X00080000  /* CloseLogicalChannel */
#define     EVENT_MSG_LCBLC_CL_ACK      EVENT_LCBLCSE | 0X00090000  /* CloseLogicalChannelAck */


/* Close Logical Channel Signalling Entity ( CLCSE ) */
#define     EVENT_PTV_CLC_CLS_REQ       EVENT_CLCSE   | 0X00000000  /* CLOSE.request  */
#define     EVENT_MSG_CLC               EVENT_CLCSE   | 0X00010000  /* RequestChannelClose */
#define     EVENT_PTV_CLC_CLS_RPS       EVENT_CLCSE   | 0X00020000  /* CLOSE.response */
#define     EVENT_PTV_CLC_RJT_REQ       EVENT_CLCSE   | 0X00030000  /* REJECT.request */
#define     EVENT_MSG_CLC_ACK           EVENT_CLCSE   | 0X00040000  /* RequestChannelCloseAck */
#define     EVENT_MSG_CLC_RJT           EVENT_CLCSE   | 0X00050000  /* RequestChannelCloseReject */
#define     EVENT_TIM_CLC_T108          EVENT_CLCSE   | 0X00060000  /* Timer T108 Timeout */
#define     EVENT_MSG_CLC_RLS           EVENT_CLCSE   | 0X00070000  /* RequestChannelCloseRelease */


/* Multiplex Table Signalling Entity ( MTSE ) */
#define     EVENT_PTV_MT_TRF_REQ        EVENT_MTSE    | 0X00000000  /* TRANSFER.request */
#define     EVENT_MSG_MT                EVENT_MTSE    | 0X00010000  /* MultiplexEntrySend */
#define     EVENT_PTV_MT_TRF_RPS        EVENT_MTSE    | 0X00020000  /* TRANSFER.response  */
#define     EVENT_PTV_MT_RJT_REQ        EVENT_MTSE    | 0X00030000  /* REJECT.request */
#define     EVENT_MSG_MT_ACK            EVENT_MTSE    | 0X00040000  /* MultiplexEntrySendAck */
#define     EVENT_MSG_MT_RJT            EVENT_MTSE    | 0X00050000  /* MultiplexEntrySendReject */
#define     EVENT_TIM_MT_T104           EVENT_MTSE    | 0X00060000  /* Timer T104 Timeout */
#define     EVENT_MSG_MT_RLS            EVENT_MTSE    | 0X00070000  /* MultiplexEntrySendRelease */


/* Request Multiplex Entry Signalling Entity ( RMESE ) */
#define     EVENT_PTV_RME_SEND_REQ      EVENT_RMESE   | 0X00000000  /* SEND.request */
#define     EVENT_MSG_RME               EVENT_RMESE   | 0X00010000  /* RequestMultiplexEntry */
#define     EVENT_PTV_RME_SEND_RPS      EVENT_RMESE   | 0X00020000  /* SEND.responsse */
#define     EVENT_PTV_RME_RJT_REQ       EVENT_RMESE   | 0X00030000  /* REJECT.request */
#define     EVENT_MSG_RME_ACK           EVENT_RMESE   | 0X00040000  /* RequestMultiplexEntryAck */
#define     EVENT_MSG_RME_RJT           EVENT_RMESE   | 0X00050000  /* RequestMultiplexEntryReject */
#define     EVENT_TIM_RME_T107          EVENT_RMESE   | 0X00060000  /* Timer T107 Timeout */
#define     EVENT_MSG_RME_RLS           EVENT_RMESE   | 0X00070000  /* RequestMultiplexEntryRelease */


/* Mode Request Signalling Entity ( MRSE ) */
#define     EVENT_PTV_MR_TRF_REQ        EVENT_MRSE    | 0X00000000  /* TRANSFER.request */
#define     EVENT_MSG_MR                EVENT_MRSE    | 0X00010000  /* RequestMode */
#define     EVENT_PTV_MR_TRF_RPS        EVENT_MRSE    | 0X00020000  /* TRANSFER.response */
#define     EVENT_PTV_MR_RJT_REQ        EVENT_MRSE    | 0X00030000  /* REJECT.request */
#define     EVENT_MSG_MR_ACK            EVENT_MRSE    | 0X00040000  /* RequestModeAck */
#define     EVENT_MSG_MR_RJT            EVENT_MRSE    | 0X00050000  /* RequestModeReject */
#define     EVENT_TIM_MR_T109           EVENT_MRSE    | 0X00060000  /* Timer T109 Timeout */
#define     EVENT_MSG_MR_RLS            EVENT_MRSE    | 0X00070000  /* RequestModeRelease */


/* Round Trip Deray Signalling Entity ( RTDSE ) */
#define     EVENT_PTV_RTD_TRF_REQ       EVENT_RTDSE   | 0X00000000  /* TRANSFER.request */
#define     EVENT_MSG_RTD_REQ           EVENT_RTDSE   | 0X00010000  /* RoundTripDelayRequest */
#define     EVENT_TIM_RTD_T105          EVENT_RTDSE   | 0X00020000  /* Timer T105 Timeout */
#define     EVENT_MSG_RTD_RPS           EVENT_RTDSE   | 0X00030000  /* RoundTripDelayResponse */


/* Maintenance Loop Signalling Entity ( MLSE ) */
#define     EVENT_PTV_ML_LOOP_REQ       EVENT_MLSE    | 0X00000000  /* LOOP.request */
#define     EVENT_MSG_ML_REQ            EVENT_MLSE    | 0X00010000  /* MaintenanceLoopRequest */
#define     EVENT_PTV_ML_LOOP_RPS       EVENT_MLSE    | 0X00020000  /* LOOP.response */
#define     EVENT_PTV_ML_RLS_REQ        EVENT_MLSE    | 0X00030000  /* RELEASE.request */
#define     EVENT_MSG_ML_ACK            EVENT_MLSE    | 0X00040000  /* MaintenanceLoopAck */
#define     EVENT_MSG_ML_RJT            EVENT_MLSE    | 0X00050000  /* MaintenanceLoopReject */
#define     EVENT_TIM_ML_T102           EVENT_MLSE    | 0X00060000  /* Timer T102 Timeout */
#define     EVENT_MSG_ML_OFF_CMD        EVENT_MLSE    | 0X00070000  /* MaintenanceLoopOffCommand */


/* Non Standard */
#define     EVENT_PTV_NSD_REQ           EVENT_NSD     | 0X00000000  /* NONSTANDARDMESSAGE_REQUEST */
#define     EVENT_MSG_NSD_REQ           EVENT_NSD     | 0X00010000  /* NonStandardMessageRequest */
#define     EVENT_PTV_NSD_RPS           EVENT_NSD     | 0X00020000  /* NONSTANDARDMESSAGE_RESPONSE */
#define     EVENT_MSG_NSD_RPS           EVENT_NSD     | 0X00030000  /* NonStandardMessageResponse */
#define     EVENT_PTV_NSD_CMD           EVENT_NSD     | 0X00040000  /* NONSTANDARDMESSAGE_COMMAND */
#define     EVENT_MSG_NSD_CMD           EVENT_NSD     | 0X00050000  /* NonStandardMessageCommand */
#define     EVENT_PTV_NSD_IDC           EVENT_NSD     | 0X00060000  /* NONSTANDARDMESSAGE_INDICATION */
#define     EVENT_MSG_NSD_IDC           EVENT_NSD     | 0X00070000  /* NonStandardMessageIndication */


/* Communication Mode */
#define     EVENT_PTV_CM_REQ            EVENT_CM      | 0X00000000  /* COMMUNICATIONMODE_REQUEST */
#define     EVENT_MSG_CM_REQ            EVENT_CM      | 0X00010000  /* CommunicationModeRequest */
#define     EVENT_PTV_CM_RPS            EVENT_CM      | 0X00020000  /* COMMUNICATIONMODE_RESPONSE */
#define     EVENT_MSG_CM_RPS            EVENT_CM      | 0X00030000  /* CommunicationModeResponse */
#define     EVENT_PTV_CM_CMD            EVENT_CM      | 0X00040000  /* COMMUNICATIONMODE_COMMAND */
#define     EVENT_MSG_CM_CMD            EVENT_CM      | 0X00050000  /* CommunicationModeCommand */


/* H223 Annex A Reconfiguration */
/* Note: This is not a 245 codeword.  */
/* These events are not supported. (RAN) */
#define     EVENT_PTV_H223AAR           EVENT_H223AAR | 0X00000000  /* H223ANNEXARECONFIGURATION */
#define     EVENT_MSG_H223AAR           EVENT_H223AAR | 0X00010000  /* H223AnnexAReconfiguration */
#define     EVENT_PTV_H223AAR_ACK       EVENT_H223AAR | 0X00020000  /* H223ANNEXARECONFIGURATION_ACK */
#define     EVENT_MSG_H223AAR_ACK       EVENT_H223AAR | 0X00030000  /* H223AnnexAReconfigurationAck */
#define     EVENT_PTV_H223AAR_RJT       EVENT_H223AAR | 0X00040000  /* H223ANNEXARECONFIGURATION_REJECT */
#define     EVENT_MSG_H223AAR_RJT       EVENT_H223AAR | 0X00050000  /* H223AnnexAReconfigurationReject */


/* Conference Request And Response */
#define     EVENT_PTV_CNF_REQ           EVENT_CNF     | 0X00000000  /* CONFERENCE_REQUEST */
#define     EVENT_MSG_CNF_REQ           EVENT_CNF     | 0X00010000  /* ConferenceRequest */
#define     EVENT_PTV_CNF_RPS           EVENT_CNF     | 0X00020000  /* CONFERENCE_RESPONSE */
#define     EVENT_MSG_CNF_RPS           EVENT_CNF     | 0X00030000  /* ConferenceResponse */


/* Command */
#define     EVENT_PTV_CMD_STCS          EVENT_CMD     | 0X00000000  /* SENDTERMINALCAPABILITYSET */
#define     EVENT_MSG_CMD_STCS          EVENT_CMD     | 0X00010000  /* SendTerminalCapabilitySet */
#define     EVENT_PTV_CMD_ECRPT         EVENT_CMD     | 0X00020000  /* ENCRYPTIONCOMMAND */
#define     EVENT_MSG_CMD_ECRPT         EVENT_CMD     | 0X00030000  /* EncryptionCommand */
#define     EVENT_PTV_CMD_FC            EVENT_CMD     | 0X00040000  /* FLOWCONTROLCOMMAND */
#define     EVENT_MSG_CMD_FC            EVENT_CMD     | 0X00050000  /* FlowControlCommand */
#define     EVENT_PTV_CMD_ES            EVENT_CMD     | 0X00060000  /* ENDSESSIONCOMMAND */
#define     EVENT_MSG_CMD_ES            EVENT_CMD     | 0X00070000  /* EndSessionCommand */
#define     EVENT_PTV_CMD_MSCL          EVENT_CMD     | 0X00080000  /* MISCELLANEOUSCOMMAND */
#define     EVENT_MSG_CMD_MSCL          EVENT_CMD     | 0X00090000  /* MiscellaneousCommand */
#define     EVENT_PTV_CMD_CNF           EVENT_CMD     | 0X000A0000  /* CONFERENCECOMMAND */
#define     EVENT_MSG_CMD_CNF           EVENT_CMD     | 0X000B0000  /* ConferenceCommand */
#define     EVENT_PTV_CMD_HMR           EVENT_CMD     | 0X000C0000  /* CONFERENCECOMMAND */
#define     EVENT_MSG_CMD_HMR           EVENT_CMD     | 0X000D0000  /* ConferenceCommand */


/* Indication */
#define     EVENT_PTV_IDC_FNU           EVENT_IDC     | 0X00000000  /* FUNCTIONNOTUNDERSTOOD */
#define     EVENT_MSG_IDC_FNU           EVENT_IDC     | 0X00010000  /* FunctionNotUnderstood */
#define     EVENT_PTV_IDC_MSCL          EVENT_IDC     | 0X00020000  /* MISCELLANEOUSINDICATION */
#define     EVENT_MSG_IDC_MSCL          EVENT_IDC     | 0X00030000  /* MiscellaneousIndication */
#define     EVENT_PTV_IDC_JTR           EVENT_IDC     | 0X00040000  /* JITTERINDICATION */
#define     EVENT_MSG_IDC_JTR           EVENT_IDC     | 0X00050000  /* JitterIndication */
#define     EVENT_PTV_IDC_H223SKW       EVENT_IDC     | 0X00060000  /* H223SKEWINDICATION */
#define     EVENT_MSG_IDC_H223SKW       EVENT_IDC     | 0X00070000  /* H223SkewIndication */
#define     EVENT_PTV_IDC_NWATMVC       EVENT_IDC     | 0X00080000  /* NEWATMVCINDICATION */
#define     EVENT_MSG_IDC_NWATMVC       EVENT_IDC     | 0X00090000  /* NewATMVCIndication */
#define     EVENT_PTV_IDC_UI            EVENT_IDC     | 0X000A0000  /* USERINPUTINDICATION */
#define     EVENT_MSG_IDC_UI            EVENT_IDC     | 0X000B0000  /* UserInputIndication */
#define     EVENT_PTV_IDC_CNF           EVENT_IDC     | 0X000C0000  /* CONFERENCEINDICATION */
#define     EVENT_MSG_IDC_CNF           EVENT_IDC     | 0X000D0000  /* ConferenceIndication */
#define     EVENT_PTV_IDC_H2250MXSKW    EVENT_IDC     | 0X000E0000  /* H2250MAXIMUMSKEWINDICATION */
#define     EVENT_MSG_IDC_H2250MXSKW    EVENT_IDC     | 0X000F0000  /* H2250MaximumSkewIndication */
#define     EVENT_PTV_IDC_MCLCT         EVENT_IDC     | 0X00100000  /* MCLOCATIONINDICATION */
#define     EVENT_MSG_IDC_MCLCT         EVENT_IDC     | 0X00110000  /* MCLocationIndication */
#define     EVENT_PTV_IDC_VI            EVENT_IDC     | 0X00120000  /* VENDORIDENTIFICATION */
#define     EVENT_MSG_IDC_VI            EVENT_IDC     | 0X00130000  /* VendorIdentification */
#define     EVENT_PTV_IDC_FNS           EVENT_IDC     | 0X00140000  /* FUNCTIONNOTSUPPORTED */
#define     EVENT_MSG_IDC_FNS           EVENT_IDC     | 0X00150000  /* FunctionNotSupported */
#define     EVENT_PTV_IDC_FC            EVENT_IDC     | 0X00160000  /* FLOWCONTROL */
#define     EVENT_MSG_IDC_FC            EVENT_IDC     | 0X00170000  /* FlowControl */

#define		ITN_ERR_CODE_0000			0X00000000  /* Information Type Error ( Information Type ) */
#define		ITN_ERR_CODE_0001			0X00000001  /* Information Id Error ( Information Id ) */
#define		ITN_ERR_CODE_0002			0X00000002  /* Status Error ( Event Number | Status ) */

/************************************************************************/
/*  Event No Table Type Definition                                      */
/************************************************************************/
typedef struct _EventNoTable
{
    uint32    EventNo ;       /* Event Number */
    uint32    InfType2 ;      /* Information Type 2 */
    const char    *Comment ;       /* Description of event (RAN) */
} S_EventNoTable ;
typedef S_EventNoTable *PS_EventNoTable ;

/************************************************************************/
/*  Function Prototype Declare                                          */
/************************************************************************/
class SE : public OsclTimerObserver
{
    public:
        OSCL_IMPORT_REF SE();
        OSCL_IMPORT_REF virtual ~SE();

        OSCL_IMPORT_REF void Reset();

        OSCL_IMPORT_REF void InformationRecv(PS_InfHeader);
        void MessageRecv(uint8 Type1, uint8 Type2, OsclSharedPtr<uint8> Data, uint8* EncodedMsg, uint32 EncodedMsgSize);

        OSCL_IMPORT_REF void SetPer(PER *per);
        OSCL_IMPORT_REF void SetH245(H245 *h245);
        OSCL_IMPORT_REF void SetTimers(H245TimerValues& timer_vals)
        {
            iTimerValues = timer_vals;
            MyCE.TimerDuration = iTimerValues.iT101;
            MyLC.TimerDuration = iTimerValues.iT103;
            MyBLC.TimerDuration = iTimerValues.iT103;
            MyMT.TimerDuration = iTimerValues.iT104;
            MyRTD.TimerDuration = iTimerValues.iT105;
            MyMSD.TimerDuration = iTimerValues.iT106;
            MyRME.TimerDuration = iTimerValues.iT107;
            MyCLC.TimerDuration = iTimerValues.iT108;
            MyMR.TimerDuration = iTimerValues.iT109;
        }

        void TimeoutOccurred(int32 timerID, int32 timeoutInfo);
        void RequestTimer(int32 timerID, int32 timeoutInfo, int32 count, OsclTimerObserver *observer = 0)
        {
            MyTimer.Request(timerID, timeoutInfo, count, observer);
        }
        void CancelTimer(int32 timerID)
        {
            MyTimer.Cancel(timerID);
        }

        void Print(const char *format, ...);

        MSD* GetMSD()
        {
            return &MyMSD;
        }
        CE*  GetCE()
        {
            return &MyCE;
        }

    private:
        SE(const SE&);
        PS_EventNoTable TablePointerGetUpperLayer(uint32) ;
        PS_EventNoTable TablePointerGetLowerLayer(uint32) ;
        uint32 EventNoDecision(PS_EventNoTable , uint32) ;
        void EventNoShow(char *comment);
        void Dispatch(uint32 , OsclSharedPtr<uint8> , uint32 , uint32, uint8* EncodedMsg = NULL, uint32 EncodedMsgSize = 0) ;
        void Nop_XXXX_XXXX(void)
        { /* no operation */
            ;
        }
        void ItnErrNotificationSend(uint32 , uint32) ;

        PER *MyPER;
        H245 *MyH245;
        OsclTimer<OsclMemAllocator> MyTimer;
        MSD MyMSD;
        BLC MyBLC;
        LC  MyLC;
        CLC MyCLC;
        NSD MyNSD;
        CM  MyCM;
        CNF MyCNF;
        CMD MyCMD;
        IDC MyIDC;
        CE	MyCE;
        MT  MyMT;
        RTD	MyRTD;
        MR	MyMR;
        RME MyRME;
        ML	MyML;  // Caution: MLSE is not implemented for Two-Way.

        LCEntryList LCBLCOutgoing;
        LCEntryList LCBLCIncoming;

        PVLogger *Logger;
        H245TimerValues iTimerValues;
};


#endif /* _SE_ */
