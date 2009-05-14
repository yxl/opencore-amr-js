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
/*  file name       : sedisp.c                                          */
/*  file contents   : Receive Data Dispatch Management                  */
/*                  : Primitive and Timeout and Message                 */
/*  draw            : '96.11.28                                         */
/*----------------------------------------------------------------------*/
/*  amendment       :                                                   */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/


/************************************************************************/
/*  Headerfile Include                                                  */
/************************************************************************/
#include    "h245pri.h"
#include	"oscl_timer.h"
#include    "semsgque.h"
#include    "se.h"
#include    "msd.h"
#include	"blc.h"
#include    "lc.h"
#include    "clc.h"
#include    "annex.h"
#include    "ce.h"
#include    "mt.h"
#include    "rtd.h"
#include    "ml.h"
#include    "mr.h"
#include    "rme.h"
#include	"h245.h"
#include    "oscl_snprintf.h"
#include    "pvlogger.h"

/* PVANALYZER */
#define ANALYZER_SE 0x0020		// (Assume tag is fixed)
void Show245(uint16 tag, uint16 indent, char *inString);

/************************************************************************/
/*  External Variable Definition: Primitive Event No Table              */
/************************************************************************/
static const S_EventNoTable EventNoTblForPri[  ] =
{
//    { EVENT_PTV_MSD_DTM_REQ     , E_PtvId_Msd_Dtm_Req   ,"MSDSE --> DETERMINE.request" },
//    { EVENT_PTV_CE_TRF_REQ      , E_PtvId_Ce_Trf_Req    ,"CESE --> TRANSFER.request" },
//    { EVENT_PTV_CE_TRF_RPS      , E_PtvId_Ce_Trf_Rps    ,"CESE --> TRANSFER.response" },
//    { EVENT_PTV_CE_RJT_REQ      , E_PtvId_Ce_Rjt_Req    ,"CESE --> REJECT.request" },
    { EVENT_PTV_LCBLC_ETB_REQ   , E_PtvId_Lc_Etb_Req    , "LCSE --> ESTABLISH.request" },
    { EVENT_PTV_LCBLC_ETB_RPS   , E_PtvId_Lc_Etb_Rps    , "LCSE --> ESTABLISH.response" },
    { EVENT_PTV_LCBLC_RLS_REQ   , E_PtvId_Lc_Rls_Req    , "LCSE --> RELEASE.request" },
    { EVENT_PTV_LCBLC_ETB_REQ   , E_PtvId_Blc_Etb_Req   , "BLCSE --> ESTABLISH.request" },
    { EVENT_PTV_LCBLC_ETB_RPS   , E_PtvId_Blc_Etb_Rps   , "BLCSE --> ESTABLISH.response" },
    { EVENT_PTV_LCBLC_RLS_REQ   , E_PtvId_Blc_Rls_Req   , "BLCSE --> RELEASE.request" },
    { EVENT_PTV_CLC_CLS_REQ     , E_PtvId_Clc_Cls_Req   , "CLCSE --> CLOSE.request" },
    { EVENT_PTV_CLC_CLS_RPS     , E_PtvId_Clc_Cls_Rps   , "CLCSE --> CLOSE.response" },
    { EVENT_PTV_CLC_RJT_REQ     , E_PtvId_Clc_Rjt_Req   , "CLCSE --> REJECT.request" },
    { EVENT_PTV_MT_TRF_REQ      , E_PtvId_Mt_Trf_Req    , "MTSE --> TRANSFER.request" },
    { EVENT_PTV_MT_TRF_RPS      , E_PtvId_Mt_Trf_Rps    , "MTSE --> TRANSFER.response" },
    { EVENT_PTV_MT_RJT_REQ      , E_PtvId_Mt_Rjt_Req    , "MTSE --> REJECT.request" },
    { EVENT_PTV_RME_SEND_REQ    , E_PtvId_Rme_Send_Req  , "RMESE --> SEND.request" },
    { EVENT_PTV_RME_SEND_RPS    , E_PtvId_Rme_Send_Rps  , "RMESE --> SEND.responsse" },
    { EVENT_PTV_RME_RJT_REQ     , E_PtvId_Rme_Rjt_Req   , "RMESE --> REJECT.request" },
    { EVENT_PTV_MR_TRF_REQ      , E_PtvId_Mr_Trf_Req    , "MRSE --> TRANSFER.request" },
    { EVENT_PTV_MR_TRF_RPS      , E_PtvId_Mr_Trf_Rps    , "MRSE --> TRANSFER.response" },
    { EVENT_PTV_MR_RJT_REQ      , E_PtvId_Mr_Rjt_Req    , "MRSE --> REJECT.request" },
    { EVENT_PTV_RTD_TRF_REQ     , E_PtvId_Rtd_Trf_Req   , "RTDSE --> TRANSFER.request" },
    { EVENT_PTV_ML_LOOP_REQ     , E_PtvId_Ml_Loop_Req   , "MLSE --> LOOP.request" },
    { EVENT_PTV_ML_LOOP_RPS     , E_PtvId_Ml_Loop_Rps   , "MLSE --> LOOP.response" },
    { EVENT_PTV_ML_RLS_REQ      , E_PtvId_Ml_Rls_Req    , "MLSE --> RELEASE.request" },
    { EVENT_PTV_NSD_REQ         , E_PtvId_Nsd_Req       , "NSDSE --> NonstandardMessage REQUEST" },
    { EVENT_PTV_NSD_RPS         , E_PtvId_Nsd_Rps       , "NSDSE --> NonstandardMessage RESPONSE" },
    { EVENT_PTV_NSD_CMD         , E_PtvId_Nsd_Cmd       , "NSDSE --> NonstandardMessage COMMAND" },
    { EVENT_PTV_NSD_IDC         , E_PtvId_Nsd_Idc       , "NSDSE --> NonstandardMessage INDICATION" },
    { EVENT_PTV_CM_REQ          , E_PtvId_Cm_Req        , "CMSE --> CommunicationMode REQUEST" },
    { EVENT_PTV_CM_RPS          , E_PtvId_Cm_Rps        , "CMSE --> CommunicationMode RESPONSE" },
    { EVENT_PTV_CM_CMD          , E_PtvId_Cm_Cmd        , "CMSE --> CommunicationMode COMMAND" },
    { EVENT_PTV_H223AAR         , E_PtvId_H223aar       , "H223AAR --> H223AnnexA RECONFIG" },
    { EVENT_PTV_H223AAR_ACK     , E_PtvId_H223aar_Ack   , "H223AAR --> H223AnnexA RECONFIG ACK" },
    { EVENT_PTV_H223AAR_RJT     , E_PtvId_H223aar_Rjt   , "H223AAR --> H223AnnexA RECONFIG REJECT" },
    { EVENT_PTV_CNF_REQ         , E_PtvId_Cnf_Req       , "CNF --> Conference REQUEST" },
    { EVENT_PTV_CNF_RPS         , E_PtvId_Cnf_Rps       , "CNF --> Conference RESPONSE" },
    { EVENT_PTV_CMD_STCS        , E_PtvId_Cmd_Stcs      , "CMD --> SEND TerminalCapabilitySet" },
    { EVENT_PTV_CMD_ECRPT       , E_PtvId_Cmd_Ecrpt     , "CMD --> Encryption COMMAND" },
    { EVENT_PTV_CMD_FC          , E_PtvId_Cmd_Fc        , "CMD --> FlowControl COMMAND" },
    { EVENT_PTV_CMD_ES          , E_PtvId_Cmd_Es        , "CMD --> EndSession COMMAND" },
    { EVENT_PTV_CMD_MSCL        , E_PtvId_Cmd_Mscl      , "CMD --> Miscellaneous COMMAND" },
    { EVENT_PTV_CMD_CNF         , E_PtvId_Cmd_Cnf       , "CMD --> Conference COMMAND" },
    { EVENT_PTV_CMD_HMR         , E_PtvId_Cmd_Hmr       , "CMD --> H223 Multiplex RECONFIG" },
    { EVENT_PTV_IDC_FNU         , E_PtvId_Idc_Fnu       , "IDC --> FunctionNotUnderstood" },
    { EVENT_PTV_IDC_MSCL        , E_PtvId_Idc_Mscl      , "IDC --> Miscellaneous INDICATION" },
    { EVENT_PTV_IDC_JTR         , E_PtvId_Idc_Jtr       , "IDC --> Jitter INDICATION" },
    { EVENT_PTV_IDC_H223SKW     , E_PtvId_Idc_H223skw   , "IDC --> H223 Skew INDICATION" },
    { EVENT_PTV_IDC_NWATMVC     , E_PtvId_Idc_Nwatmvc   , "IDC --> NewATMVC INDICATION" },
    { EVENT_PTV_IDC_UI          , E_PtvId_Idc_Ui        , "IDC --> UserInput INDICATION" },
    { EVENT_PTV_IDC_CNF         , E_PtvId_Idc_Cnf       , "IDC --> Conference INDICATION" },
    { EVENT_PTV_IDC_H2250MXSKW  , E_PtvId_Idc_H2250mxskw, "IDC --> H2250 Maximum Skew INDICATION" },
    { EVENT_PTV_IDC_MCLCT       , E_PtvId_Idc_Mclct     , "IDC --> McLocation INDICATION" },
    { EVENT_PTV_IDC_VI          , E_PtvId_Idc_Vi        , "IDC --> Vendor ID" },
    { EVENT_PTV_IDC_FNS         , E_PtvId_Idc_Fns       , "IDC --> FunctionNotSupported" },

    /* Table End */

    { STOPPER                   , STOPPER               , "Undefined PRIMATIVE" },
} ;


/************************************************************************/
/*  External Variable Definition: Timeout Event No Table                */
/************************************************************************/
static const S_EventNoTable EventNoTblForTo[  ] =
{
//    { EVENT_TIM_MSD_T106        , E_TmrId_Msd_T106      ,"Timeout T106 (MSD)"   },
//    { EVENT_TIM_CE_T101         , E_TmrId_Ce_T101       ,"Timeout T101 (CE)"    },
    { EVENT_TIM_LCBLC_T103      , E_TmrId_LcBlc_T103    , "Timeout T103 (LC/BLC)"},
    { EVENT_TIM_CLC_T108        , E_TmrId_Clc_T108      , "Timeout T108 (CLC)"   },
    { EVENT_TIM_MT_T104         , E_TmrId_Mt_T104       , "Timeout T104 (MT)"    },
    { EVENT_TIM_RME_T107        , E_TmrId_Rme_T107      , "Timeout T107 (RME)"   },
    { EVENT_TIM_MR_T109         , E_TmrId_Mr_T109       , "Timeout T109 (MR)"    },
    { EVENT_TIM_RTD_T105        , E_TmrId_Rtd_T105      , "Timeout T105 (RTD)"   },
    { EVENT_TIM_ML_T102         , E_TmrId_Ml_T102       , "Timeout T102 (ML)"    },

    /* Table End */

    { STOPPER                   , STOPPER               , "Undefined TIMEOUT" },
} ;


/************************************************************************/
/*  External Variable Definition: Request Message Event No Table        */
/************************************************************************/
static const S_EventNoTable EventNoTblForReqMsg[  ] =
{
    { EVENT_MSG_NSD_REQ         , MSGTYP_NSD_REQ        , "NonStandardMessageRequest RECEIVED" },
    { EVENT_MSG_MSD             , MSGTYP_MSD            , "MasterSlaveDetermination RECEIVED" },
    { EVENT_MSG_CE              , MSGTYP_CE             , "TerminalCapabilitySet RECEIVED" },
    { EVENT_MSG_LCBLC_OP        , MSGTYP_LCBLC_OP       , "OpenLogicalChannel (LC/BLC) RECEIVED" },
    { EVENT_MSG_LCBLC_CL        , MSGTYP_LCBLC_CL       , "CloseLogicalChannel (LC/BLC) RECEIVED" },
    { EVENT_MSG_CLC             , MSGTYP_CLC            , "RequestChannelClose RECEIVED" },
    { EVENT_MSG_MT              , MSGTYP_MT             , "MultiplexEntrySend RECEIVED" },
    { EVENT_MSG_RME             , MSGTYP_RME            , "RequestMultiplexEntry RECEIVED" },
    { EVENT_MSG_MR              , MSGTYP_MR             , "RequestMode RECEIVED" },
    { EVENT_MSG_RTD_REQ         , MSGTYP_RTD_REQ        , "RoundTripDelayRequest RECEIVED" },
    { EVENT_MSG_ML_REQ          , MSGTYP_ML_REQ         , "MaintenanceLoopRequest RECEIVED" },
    { EVENT_MSG_CM_REQ          , MSGTYP_CM_REQ         , "CommunicationModeRequest RECEIVED" },
    { EVENT_MSG_CNF_REQ         , MSGTYP_CNF_REQ        , "ConferenceRequest RECEIVED" },
    { EVENT_MSG_H223AAR         , MSGTYP_H223AAR        , "H223AnnexAReconfiguration RECEIVED" },
    /* (Not a codeword -- Not supported (RAN)) */
    /* Table End */

    { STOPPER                   , STOPPER               , "Undefined REQUEST MESSAGE" },
} ;


/************************************************************************/
/*  External Variable Definition: Response Message Event No Table       */
/************************************************************************/
static const S_EventNoTable EventNoTblForRpsMsg[  ] =
{
    { EVENT_MSG_NSD_RPS         , MSGTYP_NSD_RPS        , "NonStandardMessageResponse RECEIVED" },
    { EVENT_MSG_MSD_ACK         , MSGTYP_MSD_ACK        , "MasterSlaveDeterminationAck RECEIVED" },
    { EVENT_MSG_MSD_RJT         , MSGTYP_MSD_RJT        , "MasterSlaveDeterminationReject RECEIVED" },
    { EVENT_MSG_CE_ACK          , MSGTYP_CE_ACK         , "TerminalCapabilitySetAck RECEIVED" },
    { EVENT_MSG_CE_RJT          , MSGTYP_CE_RJT         , "TerminalCapabilitySetReject RECEIVED" },
    { EVENT_MSG_LCBLC_OP_ACK    , MSGTYP_LCBLC_OP_ACK   , "OpenLogicalChannelAck (LC/BLC) RECEIVED" },
    { EVENT_MSG_LCBLC_OP_RJT    , MSGTYP_LCBLC_OP_RJT   , "OpenLogicalChannelReject (LC/BLC) RECEIVED" },
    { EVENT_MSG_LCBLC_CL_ACK    , MSGTYP_LCBLC_CL_ACK   , "CloseLogicalChannelAck (LC/BLC) RECEIVED" },
    { EVENT_MSG_CLC_ACK         , MSGTYP_CLC_ACK        , "RequestChannelCloseAck RECEIVED" },
    { EVENT_MSG_CLC_RJT         , MSGTYP_CLC_RJT        , "RequestChannelCloseReject RECEIVED" },
    { EVENT_MSG_MT_ACK          , MSGTYP_MT_ACK         , "MultiplexEntrySendAck RECEIVED" },
    { EVENT_MSG_MT_RJT          , MSGTYP_MT_RJT         , "MultiplexEntrySendReject RECEIVED" },
    { EVENT_MSG_RME_ACK         , MSGTYP_RME_ACK        , "RequestMultiplexEntryAck RECEIVED" },
    { EVENT_MSG_RME_RJT         , MSGTYP_RME_RJT        , "RequestMultiplexEntryReject RECEIVED" },
    { EVENT_MSG_MR_ACK          , MSGTYP_MR_ACK         , "RequestModeAck RECEIVED" },
    { EVENT_MSG_MR_RJT          , MSGTYP_MR_RJT         , "RequestModeReject RECEIVED" },
    { EVENT_MSG_RTD_RPS         , MSGTYP_RTD_RPS        , "RoundTripDelayResponse RECEIVED" },
    { EVENT_MSG_ML_ACK          , MSGTYP_ML_ACK         , "MaintenanceLoopAck RECEIVED" },
    { EVENT_MSG_ML_RJT          , MSGTYP_ML_RJT         , "MaintenanceLoopReject RECEIVED" },
    { EVENT_MSG_CM_RPS          , MSGTYP_CM_RPS         , "CommunicationModeResponse RECEIVED" },
    { EVENT_MSG_CNF_RPS         , MSGTYP_CNF_RPS        , "ConferenceResponse RECEIVED" },
    { EVENT_MSG_H223AAR_ACK     , MSGTYP_H223AAR_ACK    , "H223AnnexAReconfigurationAck RECEIVED" },
    { EVENT_MSG_H223AAR_RJT     , MSGTYP_H223AAR_RJT    , "H223AnnexAReconfigurationReject RECEIVED" },
    /* (Not a codeword -- Not supported (RAN)) */
    /* Table End */

    { STOPPER                   , STOPPER               , "Undefined RESPONSE MESSAGE" },
} ;


/************************************************************************/
/*  External Variable Definition: Command Message Event No Table        */
/************************************************************************/
static const S_EventNoTable EventNoTblForCmdMsg[  ] =
{
    { EVENT_MSG_NSD_CMD         , MSGTYP_NSD_CMD        , "NonStandardMessageCommand RECEIVED" },
    { EVENT_MSG_ML_OFF_CMD      , MSGTYP_ML_OFF_CMD     , "MaintenanceLoopOffCommand RECEIVED" },
    { EVENT_MSG_CMD_STCS        , MSGTYP_CMD_STCS       , "SendTerminalCapabilitySet RECEIVED" },
    { EVENT_MSG_CMD_ECRPT       , MSGTYP_CMD_ECRPT      , "EncryptionCommand RECEIVED" },
    { EVENT_MSG_CMD_FC          , MSGTYP_CMD_FC         , "FlowControlCommand RECEIVED" },
    { EVENT_MSG_CMD_ES          , MSGTYP_CMD_ES         , "EndSessionCommand RECEIVED" },
    { EVENT_MSG_CMD_MSCL        , MSGTYP_CMD_MSCL       , "MiscellaneousCommand RECEIVED" },
    { EVENT_MSG_CM_CMD          , MSGTYP_CM_CMD         , "CommunicationModeCommand RECEIVED" },
    { EVENT_MSG_CMD_CNF         , MSGTYP_CMD_CNF        , "ConferenceCommand RECEIVED" },
    { EVENT_MSG_CMD_HMR         , MSGTYP_CMD_HMR        , "H223MultiplexReconfiguration RECEIVED" },

    /* Table End */

    { STOPPER                   , STOPPER               , "Undefined COMMAND MESSAGE" },
} ;


/************************************************************************/
/*  External Variable Definition: Indication Message Event No Table     */
/************************************************************************/
static const S_EventNoTable EventNoTblForIdcMsg[  ] =
{
    { EVENT_MSG_NSD_IDC         , MSGTYP_NSD_IDC        , "NonStandardMessageIndication RECEIVED" },
    { EVENT_MSG_IDC_FNU         , MSGTYP_IDC_FNU        , "FunctionNotUnderstood RECEIVED" },
    { EVENT_MSG_MSD_RLS         , MSGTYP_MSD_RLS        , "MasterSlaveDeterminationRelease RECEIVED" },
    { EVENT_MSG_CE_RLS          , MSGTYP_CE_RLS         , "TerminalCapabilitySetRelease RECEIVED" },
    { EVENT_MSG_LCBLC_OP_CFM    , MSGTYP_LCBLC_OP_CFM   , "OpenLogicalChannelConfirm (BLC) RECEIVED" },
    { EVENT_MSG_CLC_RLS         , MSGTYP_CLC_RLS        , "RequestChannelCloseRelease RECEIVED" },
    { EVENT_MSG_MT_RLS          , MSGTYP_MT_RLS         , "MultiplexEntrySendRelease RECEIVED" },
    { EVENT_MSG_RME_RLS         , MSGTYP_RME_RLS        , "RequestMultiplexEntryRelease RECEIVED" },
    { EVENT_MSG_MR_RLS          , MSGTYP_MR_RLS         , "RequestModeRelease RECEIVED" },
    { EVENT_MSG_IDC_MSCL        , MSGTYP_IDC_MSCL       , "MiscellaneousIndication RECEIVED" },
    { EVENT_MSG_IDC_JTR         , MSGTYP_IDC_JTR        , "JitterIndication RECEIVED" },
    { EVENT_MSG_IDC_H223SKW     , MSGTYP_IDC_H223SKW    , "H223SkewIndication RECEIVED" },
    { EVENT_MSG_IDC_NWATMVC     , MSGTYP_IDC_NWATMVC    , "NewATMVCIndication RECEIVED" },
    { EVENT_MSG_IDC_UI          , MSGTYP_IDC_UI         , "UserInputIndication RECEIVED" },
    { EVENT_MSG_IDC_H2250MXSKW  , MSGTYP_IDC_H2250MXSKW , "H2250MaximumSkewIndication RECEIVED" },
    { EVENT_MSG_IDC_MCLCT       , MSGTYP_IDC_MCLCT      , "MCLocationIndication RECEIVED" },
    { EVENT_MSG_IDC_CNF         , MSGTYP_IDC_CNF        , "ConferenceIndication RECEIVED" },
    { EVENT_MSG_IDC_VI          , MSGTYP_IDC_VI         , "VendorIdentification RECEIVED" },
    { EVENT_MSG_IDC_FNS         , MSGTYP_IDC_FNS        , "FunctionNotSupported RECEIVED" },
    { EVENT_MSG_IDC_FC         , MSGTYP_IDC_FC          , "FlowControlIndication RECEIVED" },

    /* Table End */

    { STOPPER                   , STOPPER               , "Undefined INDICATION MESSAGE" },
} ;


/************************************************************************/
/*  function name       : SE class constructor                          */
/************************************************************************/
OSCL_EXPORT_REF SE::SE() : MyPER(0), MyH245(0), MyTimer("SETimer")
{

    MyTimer.SetObserver(this);
    MyMSD.SetSE(this);
    MyBLC.SetSE(this);
    MyLC.SetSE(this);
    MyCLC.SetSE(this);
    MyNSD.SetSE(this);
    MyCM.SetSE(this);
    MyCNF.SetSE(this);
    MyCMD.SetSE(this);
    MyIDC.SetSE(this);
    MyCE.SetSE(this);
    MyMT.SetSE(this);
    MyRTD.SetSE(this);
    MyML.SetSE(this);
    MyMR.SetSE(this);
    MyRME.SetSE(this);

    /* Signalling Entity Initialize */
    MyBLC.SetMSD(&MyMSD);
    MyLC.SetMSD(&MyMSD);

    LCBLCOutgoing.SetDirection(LC_OUTGOING);
    LCBLCIncoming.SetDirection(LC_INCOMING);

    oscl_memset(&iTimerValues, 0, sizeof(H245TimerValues));

    Logger = PVLogger::GetLoggerObject("3g324m.h245.se");
}

/************************************************************************/
/*  function name       : SE class destructor                           */
/************************************************************************/
OSCL_EXPORT_REF SE::~SE()
{
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
OSCL_EXPORT_REF void SE::Reset()
{
    // reset the states for the SEs
    MyTimer.Clear();
    MyMSD.Reset();
    MyBLC.Reset();
    MyLC.Reset();
    MyCLC.Reset();
    MyNSD.Reset();
    MyCM.Reset();
    MyCNF.Reset();
    MyCMD.Reset();
    MyIDC.Reset();
    MyCE.Reset();
    MyMT.Reset();
    MyRTD.Reset();
    MyML.Reset();
    MyMR.Reset();
    MyRME.Reset();
    LCBLCOutgoing.Reset();
    LCBLCIncoming.Reset();
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
OSCL_EXPORT_REF void SE::SetPer(PER *per)
{
    MyPER = per;
    MyMSD.SetPer(per);
    MyBLC.SetPer(per);
    MyLC.SetPer(per);
    MyCLC.SetPer(per);
    MyNSD.SetPer(per);
    MyCM.SetPer(per);
    MyCNF.SetPer(per);
    MyCMD.SetPer(per);
    MyIDC.SetPer(per);
    MyCE.SetPer(per);
    MyMT.SetPer(per);
    MyRTD.SetPer(per);
    MyML.SetPer(per);
    MyMR.SetPer(per);
    MyRME.SetPer(per);
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
OSCL_EXPORT_REF void SE::SetH245(H245 *h245)
{
    MyH245 = h245;
    MyMSD.SetH245(h245);
    MyBLC.SetH245(h245);
    MyLC.SetH245(h245);
    MyCLC.SetH245(h245);
    MyNSD.SetH245(h245);
    MyCM.SetH245(h245);
    MyCNF.SetH245(h245);
    MyCMD.SetH245(h245);
    MyIDC.SetH245(h245);
    MyCE.SetH245(h245);
    MyMT.SetH245(h245);
    MyRTD.SetH245(h245);
    MyML.SetH245(h245);
    MyMR.SetH245(h245);
    MyRME.SetH245(h245);
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
void SE::TimeoutOccurred(int32 timerID , int32 timeoutInfo)
{
    Print("    SE::TimeoutOccurred timerId, timeoutInfo=", timerID, timeoutInfo);

    S_InfHeader infHeader;

    infHeader.InfType = H245_TIMEOUT ;
    infHeader.InfId = (timerID & 0x0000FFFF) ;
    infHeader.InfSupplement1 = timeoutInfo ;
    infHeader.InfSupplement2 = ((timerID >> 16) & 0x0000FFFF) ;
    infHeader.pParameter = NULL;

    InformationRecv(&infHeader);
}


/************************************************************************/
/*  function name       : InformationRecv                               */
/*  function outline    : Receive Information From Upper Layer          */
/*                      : Primitive and Timeout                         */
/*  function discription: InformationRecv( PS_InfHeader )               */
/*  input data          : PS_InfHeader p_InfHeader                      */
/*  output data         : None                                          */
/*  draw time           : '96.11.26                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
OSCL_EXPORT_REF void SE::InformationRecv(PS_InfHeader p_InfHeader)
{
    PS_EventNoTable     p_EventNoTable ;
    uint32                Event_No ;

    p_EventNoTable = TablePointerGetUpperLayer(p_InfHeader->InfType) ;
    if (p_EventNoTable != NULL)  /* Search Table Pointer Get OK */
    {
        /* Event No Decision  */
        Event_No = EventNoDecision(p_EventNoTable , p_InfHeader->InfId) ;
        if (Event_No != STOPPER)  /* Event No Decision OK */
        {
            int32 leave_status = 0;
            OsclSharedPtr<uint8> parameter(p_InfHeader->pParameter, NULL);
            OSCL_TRY(leave_status, Dispatch(Event_No , parameter , p_InfHeader->InfSupplement1 , p_InfHeader->InfSupplement2));
            OSCL_FIRST_CATCH_ANY(leave_status, Print("SE::InformationRecv - Dispatch LEAVE"));
        }
        else /* Event No Decision NG ( Information Id Error ) */
        {
            /* Internal Error Notification ( To TSC ) */
            ItnErrNotificationSend(ITN_ERR_CODE_0000 , p_InfHeader->InfId) ;
        }
    }
    else /* Search Table Pointer Get NG ( Message Type1 Error ) */
    {
        /* Internal Error Notification ( To TSC ) */
        ItnErrNotificationSend(ITN_ERR_CODE_0000 , (uint32) p_InfHeader->InfType) ;
    }
    return ;
}

/************************************************************************/
/*  function name       : MessageRecv                                   */
/*  function outline    : Receive Information From Lower Layer          */
/*                      : Message                                       */
/*  function discription: MessageRecv( PS_H245Msg )                     */
/*  input data          : PS_H245Msg p_H245Msg                          */
/*  output data         : None                                          */
/*  draw time           : '96.11.26                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void SE::MessageRecv(uint8 Type1, uint8 Type2, OsclSharedPtr<uint8> Data, uint8* EncodedMsg, uint32 EncodedMsgSize)
{
    PS_EventNoTable     p_EventNoTable ;
    uint32                Event_No ;

    p_EventNoTable = TablePointerGetLowerLayer(Type1) ;
    if (p_EventNoTable != NULL)  /* Search Table Pointer Get OK */
    {
        /* Event No Decision  */
        Event_No = EventNoDecision(p_EventNoTable , (uint32)Type2) ;
        if (Event_No != STOPPER)  /* Event No Decision OK */
        {
            int32 leave_status = 0;
            OSCL_TRY(leave_status, Dispatch(Event_No , Data , 0 , 0, EncodedMsg, EncodedMsgSize));
            OSCL_FIRST_CATCH_ANY(leave_status, Print("SE::MessageRecv - Dispatch() LEAVE"));
        }
        else  /* Event No Decision NG ( Message Type1 Error ) */
        {
            /* Internal Error Notification ( To TSC ) */
            ItnErrNotificationSend(ITN_ERR_CODE_0001 , (uint32) Type2) ;
        }
    }
    else /* Search Table Pointer Get NG ( Message Type1 Error ) */
    {
        /* Internal Error Notification ( To TSC ) */
        ItnErrNotificationSend(ITN_ERR_CODE_0000 , (uint32) Type1) ;
    }
    return ;
}


/************************************************************************/
/*  function name       : Se_TablePointerGetUpperLayer                  */
/*  function outline    : Table Pointer Get ( From Upper Layer )        */
/*  function discription: Se_TablePointerGetUpperLayer( uint32 )          */
/*  input data          : uint32 InfType_2                                */
/*  output data         : PS_EventNoTable p_EventNoTable                */
/*  draw time           : '96.11.26                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
PS_EventNoTable SE::TablePointerGetUpperLayer(uint32 InfType_2)
{
    PS_EventNoTable p_EventNoTable ;

    switch (InfType_2)  /* Information Type 2 */
    {
        case    H245_PRIMITIVE : /* H.245 Primitive Information Receive */
            p_EventNoTable = (PS_EventNoTable)EventNoTblForPri ;
            break ;
        case    H245_TIMEOUT : /* H.245 Timeout Information Receive */
            p_EventNoTable = (PS_EventNoTable)EventNoTblForTo ;
            break ;
        default : /* Receive Information Error */
            p_EventNoTable = NULL ;
            break ;
    }
    /* Return Event No Table Pointer */
    return(p_EventNoTable) ;
}


/************************************************************************/
/*  function name       : Se_TablePointerGetLowerLayer                  */
/*  function outline    : Table Pointer Get ( From Lower Layer )        */
/*  function discription: Se_TablePointerGetLowerLayer( uint32 )          */
/*  input data          : uint32 InfType_2                                */
/*  output data         : PS_EventNoTable p_EventNoTable                */
/*  draw time           : '96.11.26                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
PS_EventNoTable SE::TablePointerGetLowerLayer(uint32 InfType_2)
{
    PS_EventNoTable p_EventNoTable ;

    switch (InfType_2)  /* Information Type 2 */
    {
        case    H245_MSG_REQ : /* Request Message Information Receive */
            p_EventNoTable = (PS_EventNoTable) EventNoTblForReqMsg ;
            break ;
        case    H245_MSG_RPS : /* Response Message Information Receive */
            p_EventNoTable = (PS_EventNoTable) EventNoTblForRpsMsg ;
            break ;
        case    H245_MSG_CMD : /* Command Message Information Receive */
            p_EventNoTable = (PS_EventNoTable) EventNoTblForCmdMsg ;
            break ;
        case    H245_MSG_IDC : /* Indication Message Information Receive */
            p_EventNoTable = (PS_EventNoTable) EventNoTblForIdcMsg ;
            break ;
        default : /* Receive Information Error */
            p_EventNoTable = NULL ;
            break ;
    }
    /* Return Event No Table Pointer */
    return(p_EventNoTable) ;
}


/************************************************************************/
/*  function name       : Se_EventNoDecision                            */
/*  function outline    : Event No Decision                             */
/*  function discription: Se_EventNoDecision( PS_EventNoTable , uint32 )  */
/*  input data          : PS_EventNoTable p_EventNoTable                */
/*                      : uint32 InfType_2                                */
/*  output data         : uint32 Event_No                                 */
/*                      :   STOPPER : Event No Decision NG              */
/*                      :   Other   : Event No Decision OK              */
/*  draw time           : '96.11.26                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
uint32 SE::EventNoDecision(PS_EventNoTable p_EventNoTable , uint32 InfType_2)
{
    uint32    Event_No ;

    /* Event No Initialize */
    Event_No = STOPPER ;

    while (p_EventNoTable->InfType2 != STOPPER)   /* Until Information Type2 Equal Stopper */
    {
        if (p_EventNoTable->InfType2 == InfType_2)  /* Information Type2 Equal */
        {
            /* Event No Set */
            Event_No = p_EventNoTable->EventNo ;
            break ;
        }
        p_EventNoTable ++ ;
    }

#ifdef PVANALYZER /* --------SE Analyzer Tool -------- */
    EventNoShow(p_EventNoTable->Comment);  /* PVANALYZER (RAN) */
#endif            /* --------------------------------- */

    /* Event No Return */
    return(Event_No) ;
}

#ifdef PVANALYZER /* --------SE Analyzer Tool -------- */
// =========================================================
// Se_EventNoShow()
//
// This function displays the SE event which is about to
// be handled by Se_Dispatch().
// =========================================================
void SE::EventNoShow(char *comment)
{
    Show245(ANALYZER_SE, 0, "Signaling Entity Event:");
    Show245(ANALYZER_SE, 2, comment);
    Show245(ANALYZER_SE, 0, " ");
}
#endif            /* --------------------------------- */

/************************************************************************/
/*  function name       : Se_Dispatch                                   */
/*  function outline    : Dispatch                                      */
/*  function discription: Se_DispatchMain( uint32 , uint8 , uint32 , uint32 ) */
/*  input data          : uint32 Event_No                                 */
/*                      : uint8* pParameter                             */
/*                      : uint32 Supplement_1                             */
/*                      : uint32 Supplement_2                             */
/*  output data         : None                                          */
/*  draw time           : '96.11.28                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void SE::Dispatch(uint32 Event_No , OsclSharedPtr<uint8> pParameter , uint32 Supplement_1 , uint32 Supplement_2,
                  uint8* EncodedMsg, uint32 EncodedMsgSize)
{
    uint8                   Se_Status ;
    uint8					Se_Status2;
    LCEntry*				pLCEntry;
    uint8                   Directional ;
    PS_ForwardReverseParam  p_ForwardReverseParam ;
    PS_OpenLogicalChannel   p_OpenLogicalChannel ;
    int32                     lcn;        /* Temp holder for FwdLCN.  RAN */

    switch (Event_No & 0XFF000000)  /* Signalling Entity Check */
    {
        case    EVENT_MSDSE : /* Master Slave Determination Signalling Entity */
            switch (Event_No)  /* Event No Check */
            {
                case    EVENT_MSG_MSD : /* MasterSlaveDetermination */
                    MyMSD.Handler((PS_MasterSlaveDetermination)((uint8*)pParameter));
                    break ;
                case    EVENT_MSG_MSD_ACK : /* MasterSlaveDeterminationAck */
                    MyMSD.Handler((PS_MasterSlaveDeterminationAck)((uint8*)pParameter));
                    break ;
                case    EVENT_MSG_MSD_RJT: /* MasterSlaveDeterminationReject */
                    MyMSD.Handler((PS_MasterSlaveDeterminationReject)((uint8*)pParameter));
                    break ;
                case    EVENT_MSG_MSD_RLS : /* MasterSlaveDeterminationRelease */
                    MyMSD.Handler((PS_MasterSlaveDeterminationRelease)((uint8*)pParameter));
                    break;
            }
            break ;
        case    EVENT_CESE : /* Capability Exchange Signalling Entity */
            switch (Event_No)  /* Event No Check */
            {
                case    EVENT_MSG_CE : /* TerminalCapabilitySet */
                {
                    OsclSharedPtr<S_TerminalCapabilitySet> param;
                    param.Bind((PS_TerminalCapabilitySet)pParameter.GetRep(), pParameter.GetRefCounter());
                    if (param.GetRefCounter())(param.GetRefCounter())->addRef();
                    MyCE.Handler(param);
                }
                break ;
                case    EVENT_MSG_CE_ACK : /* TerminalCapabilitySetAck */
                    MyCE.Handler((PS_TerminalCapabilitySetAck)((uint8*)pParameter));
                    break ;
                case    EVENT_MSG_CE_RJT : /* TerminalCapabilitySetReject */
                    MyCE.Handler((PS_TerminalCapabilitySetReject)((uint8*)pParameter));
                    break ;
                case    EVENT_MSG_CE_RLS : /* TerminalCapabilitySetRelease */
                    MyCE.Handler((PS_TerminalCapabilitySetRelease)((uint8*)pParameter));
                    break ;
            }
            break ;
        case    EVENT_LCBLCSE : /* Uni-Directional and Bi-Directional Logical Channel Signalling Entity */
            pLCEntry = NULL ;
            switch (Event_No)  /* Event No Check */
            {
                case    EVENT_PTV_LCBLC_ETB_REQ : /* ESTABLISH.request */
                    Print("Received LCBLC Establish.Request\n");
                    pLCEntry = LCBLCOutgoing.GetLCEntry(Supplement_1);
                    /* if LC doesn't exist then create it */
                    if (pLCEntry == NULL)
                    {
                        Print("    Creating New Logical Channel Entry : Outgoing %d\n", Supplement_1);
                        pLCEntry = LCBLCOutgoing.NewLCEntry(Supplement_1);
                    }
                    else
                    {
                        Print("    Using existing Logical Channel Entry : Outgoing %d %d\n", pLCEntry->FwdLcnRead(), pLCEntry->RvsLcnRead());
                    }

                    if (pLCEntry->DirectionalRead() == NO_DIRECTIONAL)  /* Unsettled Directional */
                    {                                                            /* (true for new LCB's) */
                        p_ForwardReverseParam = (PS_ForwardReverseParam)((uint8*)pParameter) ;
                        if (p_ForwardReverseParam->option_of_reverseLogicalChannelParameters == false)   /* Uni-Directional */
                        {
                            Directional = UNI_DIRECTIONAL ;
                        }
                        else /* Bi-Directional */
                        {
                            Directional = BI_DIRECTIONAL ;
                        }
                        pLCEntry->DirectionalWrite(Directional);
                    }
                    /* Uni-Directional and Bi-Directional Logical Channel Status Read ( Each Logical Channel Number ) */
                    Se_Status = pLCEntry->StatusRead();
                    if (pLCEntry->DirectionalRead() == UNI_DIRECTIONAL)  /* Uni-Directional */
                    {
                        Print("    Channel Uni-Directional\n");
                        switch (Se_Status)  /* Uni-Directional Logical Channel Status Read */
                        {
                            case    LCBLC_OUTGOING_RLSED : /* Outgoing Released  */
                                MyLC._0200_0000(pLCEntry , (PS_ForwardReverseParam)((uint8*)pParameter)) ;
                                break ;
                            case    LCBLC_OUTGOING_AWTING_ETBMNT : /* Outgoing Awaiting Establishment */
                            case    LCBLC_OUTGOING_ETBED : /* Outgoing Established */
                                /* Internal Error Notification ( To TSC ) */
                                ItnErrNotificationSend(ITN_ERR_CODE_0002 , EVENT_LCSE & Se_Status) ;
                                break ;
                            case    LCBLC_OUTGOING_AWTING_RLS : /* Outgoing Awaiting Release */
                                MyLC._0200_0030(pLCEntry , (PS_ForwardReverseParam)((uint8*)pParameter)) ;
                                break ;
                            case    LCBLC_INCOMING_RLSED : /* Incoming Released  */
                                /* (RAN) Comment: this path is not appropriate; */
                                /*       It should never be called.             */
                                MyLC._0200_0000(pLCEntry , (PS_ForwardReverseParam)((uint8*)pParameter)) ;
                                break ;
                            case    LCBLC_INCOMING_AWTING_ETBMNT : /* Incoming Awaiting Establishment */
                            case    LCBLC_INCOMING_AWTING_CNFMTN : /* Incoming Awaiting Confirmation */
                            case    LCBLC_INCOMING_ETBED : /* Incoming Established */
                                /* Internal Error Notification ( To TSC ) */
                                ItnErrNotificationSend(ITN_ERR_CODE_0002 , EVENT_LCSE & Se_Status) ;
                                break ;
                        }
                    }
                    else /* Bi-Directional */
                    {
                        Print("    Channel Bi-Directional\n");
                        switch (Se_Status)  /* Bi-Directional Logical Channel Status Read */
                        {
                            case    LCBLC_OUTGOING_RLSED : /* Outgoing Released  */
                                MyBLC._0300_0000(pLCEntry , (PS_ForwardReverseParam)((uint8*)pParameter)) ;
                                break ;
                            case    LCBLC_OUTGOING_AWTING_ETBMNT : /* Outgoing Awaiting Establishment */
                            case    LCBLC_OUTGOING_ETBED : /* Outgoing Established */
                                /* Internal Error Notification ( To TSC ) */
                                ItnErrNotificationSend(ITN_ERR_CODE_0002 , EVENT_BLCSE & Se_Status) ;
                                break ;
                            case    LCBLC_OUTGOING_AWTING_RLS : /* Outgoing Awaiting Release */
                                MyBLC._0300_0030(pLCEntry , (PS_ForwardReverseParam)((uint8*)pParameter)) ;
                                break ;
                            case    LCBLC_INCOMING_RLSED : /* Incoming Released  */
                                /* (RAN) Comment: this path is not appropriate; */
                                /*       It should never be called.             */
                                MyBLC._0300_0000(pLCEntry , (PS_ForwardReverseParam)((uint8*)pParameter)) ;
                                break ;
                            case    LCBLC_INCOMING_AWTING_ETBMNT : /* Incoming Awaiting Establishment */
                            case    LCBLC_INCOMING_AWTING_CNFMTN : /* Incoming Awaiting Confirmation */
                            case    LCBLC_INCOMING_ETBED : /* Incoming Established */
                                /* Internal Error Notification ( To TSC ) */
                                ItnErrNotificationSend(ITN_ERR_CODE_0002 , EVENT_BLCSE & Se_Status) ;
                                break ;
                        }
                    }
                    break ;
                case    EVENT_MSG_LCBLC_OP : /* OpenLogicalChannel */
                    Print("Received LCBLC OpenLogicalChannel\n");
                    pLCEntry = LCBLCIncoming.GetLCEntry(((PS_OpenLogicalChannel)((uint8*)pParameter))->forwardLogicalChannelNumber) ;
                    if (pLCEntry == NULL)
                    {
                        Print("    Creating New Logical Channel Entry : Incoming %d\n", ((PS_OpenLogicalChannel)((uint8*)pParameter))->forwardLogicalChannelNumber);
                        pLCEntry = LCBLCIncoming.NewLCEntry(((PS_OpenLogicalChannel)((uint8*)pParameter))->forwardLogicalChannelNumber);
                    }
                    else
                    {
                        Print("    Using existing Logical Channel Entry : Incoming %d %d\n", pLCEntry->FwdLcnRead(), pLCEntry->RvsLcnRead());
                    }
                    if (pLCEntry->DirectionalRead() == NO_DIRECTIONAL)  /* Unsettled Directional */
                    {
                        p_OpenLogicalChannel = (PS_OpenLogicalChannel)((uint8*)pParameter) ;
                        if (p_OpenLogicalChannel->option_of_reverseLogicalChannelParameters == false)   /* Uni-Directional */
                        {
                            Directional = UNI_DIRECTIONAL ;
                        }
                        else /* Bi-Directional */
                        {
                            Directional = BI_DIRECTIONAL ;
                        }
                        pLCEntry->DirectionalWrite(Directional);
                    }
                    /* Uni-Directional and Bi-Directional Logical Channel Status Read ( Each Logical Channel Number ) */
                    Se_Status = pLCEntry->StatusRead();
                    if (pLCEntry->DirectionalRead() == UNI_DIRECTIONAL)  /* Uni-Directional */
                    {
                        Print("    Channel Uni-Directional\n");
                        switch (Se_Status)  /* Uni-Directional Logical Channel Status Read */
                        {
                            case    LCBLC_OUTGOING_RLSED : /* Outgoing Released  */
                                /* (RAN) Comment: this path is not appropriate; */
                                /*       It should never be called.             */
                                MyLC._0201_0001(pLCEntry , (PS_OpenLogicalChannel)((uint8*)pParameter)) ;
                                break ;
                            case    LCBLC_OUTGOING_AWTING_ETBMNT : /* Outgoing Awaiting Establishment */
                                /* (RAN) Comment: this path is not appropriate; */
                                /*       It should never be called.             */
                                MyLC._0201_0010(pLCEntry , (PS_OpenLogicalChannel)((uint8*)pParameter)) ;
                                break ;
                            case    LCBLC_OUTGOING_ETBED : /* Outgoing Established */
                            case    LCBLC_OUTGOING_AWTING_RLS : /* Outgoing Awaiting Release */
                                /* Internal Error Notification ( To TSC ) */
                                ItnErrNotificationSend(ITN_ERR_CODE_0002 , EVENT_LCSE & Se_Status) ;
                                break ;
                            case    LCBLC_INCOMING_RLSED : /* Incoming Released  */
                                MyLC._0201_0001(pLCEntry , (PS_OpenLogicalChannel)((uint8*)pParameter)) ;
                                break ;
                            case    LCBLC_INCOMING_AWTING_ETBMNT : /* Incoming Awaiting Establishment */
                                MyLC._0201_0011(pLCEntry , (PS_OpenLogicalChannel)((uint8*)pParameter)) ;
                                break ;
                            case    LCBLC_INCOMING_AWTING_CNFMTN : /* Incoming Awaiting Confirmation */
                                /* Internal Error Notification ( To TSC ) */
                                ItnErrNotificationSend(ITN_ERR_CODE_0002 , EVENT_LCSE & Se_Status) ;
                                break ;
                            case    LCBLC_INCOMING_ETBED : /* Incoming Established */
                                MyLC._0201_0011(pLCEntry , (PS_OpenLogicalChannel)((uint8*)pParameter)) ;
                                break ;
                        }
                    }
                    else /* Bi-Directional */
                    {
                        Print("    Channel Bi-Directional\n");
                        switch (Se_Status)  /* Bi-Directional Logical Channel Status Read */
                        {
                            case    LCBLC_OUTGOING_RLSED : /* Outgoing Released  */
                                /* (RAN) Comment: this path is not appropriate; */
                                /*       It should never be called.             */
                                MyBLC._0301_0001(pLCEntry , (PS_OpenLogicalChannel)((uint8*)pParameter)) ;
                                break ;
                            case    LCBLC_OUTGOING_AWTING_ETBMNT : /* Outgoing Awaiting Establishment */
                                /* (RAN) Comment: this path is not appropriate; */
                                /*       It should never be called.             */
                                MyBLC._0301_0010(pLCEntry , (PS_OpenLogicalChannel)((uint8*)pParameter)) ;
                                break ;
                            case    LCBLC_OUTGOING_ETBED : /* Outgoing Established */
                            case    LCBLC_OUTGOING_AWTING_RLS : /* Outgoing Awaiting Release */
                                /* Internal Error Notification ( To TSC ) */
                                ItnErrNotificationSend(ITN_ERR_CODE_0002 , Event_No & Se_Status) ;
                                break ;
                            case    LCBLC_INCOMING_RLSED : /* Incoming Released  */
                                MyBLC._0301_0001(pLCEntry , (PS_OpenLogicalChannel)((uint8*)pParameter)) ;
                                break ;
                            case    LCBLC_INCOMING_AWTING_ETBMNT : /* Incoming Awaiting Establishment */
                                MyBLC._0301_0011(pLCEntry , (PS_OpenLogicalChannel)((uint8*)pParameter)) ;
                                break ;
                            case    LCBLC_INCOMING_AWTING_CNFMTN : /* Incoming Awaiting Confirmation */
                                MyBLC._0301_0021(pLCEntry , (PS_OpenLogicalChannel)((uint8*)pParameter)) ;
                                break ;
                            case    LCBLC_INCOMING_ETBED : /* Incoming Established */
                                MyBLC._0301_0011(pLCEntry , (PS_OpenLogicalChannel)((uint8*)pParameter)) ;
                                break ;
                        }
                    }
                    break ;
                case    EVENT_PTV_LCBLC_ETB_RPS : /* ESTABLISH.response */
                    Print("Received LCBLC Establish.Response\n");
                    pLCEntry = LCBLCIncoming.GetLCEntry(Supplement_1);
                    if (pLCEntry == NULL)
                    {
                        ItnErrNotificationSend(ITN_ERR_CODE_0002 , EVENT_LCSE) ;
                        break;
                    }
                    else
                    {
                        Print("    Using existing Logical Channel Entry : Incoming %d %d\n", pLCEntry->FwdLcnRead(), pLCEntry->RvsLcnRead());
                    }
                    /* Uni-Directional and Bi-Directional Logical Channel Status Read ( Each Logical Channel Number ) */
                    Se_Status = pLCEntry->StatusRead();
                    if (pLCEntry->DirectionalRead() == UNI_DIRECTIONAL)  /* Uni-Directional */
                    {
                        Print("    Channel Uni-Directional\n");
                        switch (Se_Status)  /* Uni-Directional Logical Channel Status Read */
                        {
                            case    LCBLC_OUTGOING_RLSED : /* Outgoing Released  */
                            case    LCBLC_OUTGOING_AWTING_ETBMNT : /* Outgoing Awaiting Establishment */
                            case    LCBLC_OUTGOING_ETBED : /* Outgoing Established */
                            case    LCBLC_OUTGOING_AWTING_RLS : /* Outgoing Awaiting Release */
                                /* Internal Error Notification ( To TSC ) */
                                ItnErrNotificationSend(ITN_ERR_CODE_0002 , EVENT_LCSE & Se_Status) ;
                                break ;
                            case    LCBLC_INCOMING_RLSED : /* Incoming Released  */
                                /* Internal Error Notification ( To TSC ) */
                                ItnErrNotificationSend(ITN_ERR_CODE_0002 , EVENT_LCSE & Se_Status) ;
                                break ;
                            case    LCBLC_INCOMING_AWTING_ETBMNT : /* Incoming Awaiting Establishment */
                                MyLC._0202_0011(pLCEntry , (PS_ReverseData)((uint8*)pParameter)) ;
                                break ;
                            case    LCBLC_INCOMING_AWTING_CNFMTN : /* Incoming Awaiting Confirmation */
                            case    LCBLC_INCOMING_ETBED : /* Incoming Established */
                                /* Internal Error Notification ( To TSC ) */
                                ItnErrNotificationSend(ITN_ERR_CODE_0002 , EVENT_LCSE & Se_Status) ;
                                break ;
                        }
                    }
                    else /* Bi-Directional */
                    {
                        Print("    Channel Bi-Directional\n");
                        switch (Se_Status)  /* Bi-Directional Logical Channel Status Read */
                        {
                            case    LCBLC_OUTGOING_RLSED : /* Outgoing Released  */
                            case    LCBLC_OUTGOING_AWTING_ETBMNT : /* Outgoing Awaiting Establishment */
                            case    LCBLC_OUTGOING_ETBED : /* Outgoing Established */
                            case    LCBLC_OUTGOING_AWTING_RLS : /* Outgoing Awaiting Release */
                                /* Internal Error Notification ( To TSC ) */
                                ItnErrNotificationSend(ITN_ERR_CODE_0002 , EVENT_BLCSE & Se_Status) ;
                                break ;
                            case    LCBLC_INCOMING_RLSED : /* Incoming Released  */
                                /* Internal Error Notification ( To TSC ) */
                                ItnErrNotificationSend(ITN_ERR_CODE_0002 , EVENT_BLCSE & Se_Status) ;
                                break ;
                            case    LCBLC_INCOMING_AWTING_ETBMNT : /* Incoming Awaiting Establishment */
                                MyBLC._0302_0011(pLCEntry , (PS_ReverseData)((uint8*)pParameter) , (uint16) Supplement_2) ;
                                break ;
                            case    LCBLC_INCOMING_AWTING_CNFMTN : /* Incoming Awaiting Confirmation */
                            case    LCBLC_INCOMING_ETBED : /* Incoming Established */
                                /* Internal Error Notification ( To TSC ) */
                                ItnErrNotificationSend(ITN_ERR_CODE_0002 , EVENT_BLCSE & Se_Status) ;
                                break ;
                        }
                    }
                    break ;
                case    EVENT_PTV_LCBLC_RLS_REQ : /* RELEASE.request */
                    Print("Received LCBLC Release.Request\n");
                    if (((uint8*)pParameter) != NULL)  	/* WWU_VOAL2: incoming SE */
                    {
                        pLCEntry = LCBLCIncoming.GetLCEntry(Supplement_1);
                        if (pLCEntry != NULL)
                        {
                            Print("    Using existing Logical Channel Entry : Incoming %d %d\n", pLCEntry->FwdLcnRead(), pLCEntry->RvsLcnRead());
                        }
                    }
                    else  				/* WWU_VOAL2: outgoing SE */
                    {
                        pLCEntry = LCBLCOutgoing.GetLCEntry(Supplement_1);
                        if (pLCEntry != NULL)
                        {
                            Print("    Using existing Logical Channel Entry : Outgoing %d %d\n", pLCEntry->FwdLcnRead(), pLCEntry->RvsLcnRead());
                        }
                    }
                    if (pLCEntry == NULL)
                    {
                        ItnErrNotificationSend(ITN_ERR_CODE_0002 , EVENT_LCSE) ;
                        break;
                    }
                    /* Uni-Directional and Bi-Directional Logical Channel Status Read ( Each Logical Channel Number ) */
                    Se_Status = pLCEntry->StatusRead();
                    if (pLCEntry->DirectionalRead() == UNI_DIRECTIONAL)  /* Uni-Directional */
                    {
                        Print("    Channel Uni-Directional\n");
                        switch (Se_Status)  /* Uni-Directional Logical Channel Status Read */
                        {
                            case    LCBLC_OUTGOING_RLSED : /* Outgoing Released  */
                                /* Internal Error Notification ( To TSC ) */
                                ItnErrNotificationSend(ITN_ERR_CODE_0002 , EVENT_LCSE & Se_Status) ;
                                break ;
                            case    LCBLC_OUTGOING_AWTING_ETBMNT : /* Outgoing Awaiting Establishment */
                                MyLC._0203_0010(pLCEntry , (PS_OlcRejectCause)((uint8*)pParameter)) ;
                                break ;
                            case    LCBLC_OUTGOING_ETBED : /* Outgoing Established */
                                MyLC._0203_0020(pLCEntry , (PS_OlcRejectCause)((uint8*)pParameter)) ;
                                break ;
                            case    LCBLC_OUTGOING_AWTING_RLS : /* Outgoing Awaiting Release */
                                /* Internal Error Notification ( To TSC ) */
                                ItnErrNotificationSend(ITN_ERR_CODE_0002 , EVENT_LCSE & Se_Status) ;
                                break ;
                            case    LCBLC_INCOMING_RLSED : /* Incoming Released  */
                                /* Internal Error Notification ( To TSC ) */
                                ItnErrNotificationSend(ITN_ERR_CODE_0002 , EVENT_LCSE & Se_Status) ;
                                break ;
                            case    LCBLC_INCOMING_AWTING_ETBMNT : /* Incoming Awaiting Establishment */
                                MyLC._0203_0011(pLCEntry , (PS_OlcRejectCause)((uint8*)pParameter)) ;
                                break ;
                            case    LCBLC_INCOMING_AWTING_CNFMTN : /* Incoming Awaiting Confirmation */
                            case    LCBLC_INCOMING_ETBED : /* Incoming Established */
                                /* Internal Error Notification ( To TSC ) */
                                ItnErrNotificationSend(ITN_ERR_CODE_0002 , EVENT_LCSE & Se_Status) ;
                                break ;
                        }
                    }
                    else /* Bi-Directional */
                    {
                        Print("    Channel Bi-Directional\n");
                        switch (Se_Status)  /* Bi-Directional Logical Channel Status Read */
                        {
                            case    LCBLC_OUTGOING_RLSED : /* Outgoing Released  */
                                /* Internal Error Notification ( To TSC ) */
                                ItnErrNotificationSend(ITN_ERR_CODE_0002 , EVENT_BLCSE & Se_Status) ;
                                break ;
                            case    LCBLC_OUTGOING_AWTING_ETBMNT : /* Outgoing Awaiting Establishment */
                                MyBLC._0303_0010(pLCEntry , (PS_OlcRejectCause)((uint8*)pParameter)) ;
                                break ;
                            case    LCBLC_OUTGOING_ETBED : /* Outgoing Established */
                                MyBLC._0303_0020(pLCEntry , (PS_OlcRejectCause)((uint8*)pParameter)) ;
                                break ;
                            case    LCBLC_OUTGOING_AWTING_RLS : /* Outgoing Awaiting Release */
                                /* Internal Error Notification ( To TSC ) */
                                ItnErrNotificationSend(ITN_ERR_CODE_0002 , EVENT_BLCSE & Se_Status) ;
                                break ;
                            case    LCBLC_INCOMING_RLSED : /* Incoming Released  */
                                /* Internal Error Notification ( To TSC ) */
                                ItnErrNotificationSend(ITN_ERR_CODE_0002 , EVENT_BLCSE & Se_Status) ;
                                break ;
                            case    LCBLC_INCOMING_AWTING_ETBMNT : /* Incoming Awaiting Establishment */
                                MyBLC._0303_0011(pLCEntry , (PS_OlcRejectCause)((uint8*)pParameter)) ;
                                break ;
                            case    LCBLC_INCOMING_AWTING_CNFMTN : /* Incoming Awaiting Confirmation */
                            case    LCBLC_INCOMING_ETBED : /* Incoming Established */
                                /* Internal Error Notification ( To TSC ) */
                                ItnErrNotificationSend(ITN_ERR_CODE_0002 , EVENT_BLCSE & Se_Status) ;
                                break ;
                        }
                    }
                    break ;
                case    EVENT_MSG_LCBLC_OP_ACK : /* OpenLogicalChannelAck */
                    Print("Received LCBLC OpenLogicalChannelAck\n");
                    pLCEntry = LCBLCOutgoing.GetLCEntry(((PS_OpenLogicalChannelAck)((uint8*)pParameter))->forwardLogicalChannelNumber) ;
                    if (pLCEntry == NULL)
                    {
                        ItnErrNotificationSend(ITN_ERR_CODE_0002 , EVENT_LCSE) ;
                        break;
                    }
                    else
                    {
                        Print("    Using existing Logical Channel Entry : Outgoing %d %d\n", pLCEntry->FwdLcnRead(), pLCEntry->RvsLcnRead());
                    }
                    /* Uni-Directional and Bi-Directional Logical Channel Status Read ( Each Logical Channel Number ) */
                    Se_Status = pLCEntry->StatusRead();
                    if (pLCEntry->DirectionalRead() == UNI_DIRECTIONAL)  /* Uni-Directional */
                    {
                        Print("    Channel Uni-Directional\n");
                        switch (Se_Status)  /* Uni-Directional Logical Channel Status Read */
                        {
                            case    LCBLC_OUTGOING_RLSED : /* Outgoing Released  */
                                MyLC._0204_0000(pLCEntry , (PS_OpenLogicalChannelAck)((uint8*)pParameter)) ;
                                break ;
                            case    LCBLC_OUTGOING_AWTING_ETBMNT : /* Outgoing Awaiting Establishment */
                                MyLC._0204_0010(pLCEntry , (PS_OpenLogicalChannelAck)((uint8*)pParameter)) ;
                                break ;
                            case    LCBLC_OUTGOING_ETBED : /* Outgoing Established */
                            case    LCBLC_OUTGOING_AWTING_RLS : /* Outgoing Awaiting Release */
                                Nop_XXXX_XXXX() ;
                                break ;
                            case    LCBLC_INCOMING_RLSED : /* Incoming Released  */
                            case    LCBLC_INCOMING_AWTING_ETBMNT : /* Incoming Awaiting Establishment */
                            case    LCBLC_INCOMING_AWTING_CNFMTN : /* Incoming Awaiting Confirmation */
                            case    LCBLC_INCOMING_ETBED : /* Incoming Established */
                                /* Internal Error Notification ( To TSC ) */
                                ItnErrNotificationSend(ITN_ERR_CODE_0002 , EVENT_LCSE & Se_Status) ;
                                break ;
                        }
                    }
                    else /* Bi-Directional */
                    {
                        Print("    Channel Bi-Directional\n");
                        switch (Se_Status)  /* Bi-Directional Logical Channel Status Read */
                        {
                            case    LCBLC_OUTGOING_RLSED : /* Outgoing Released  */
                                MyBLC._0304_0000(pLCEntry , (PS_OpenLogicalChannelAck)((uint8*)pParameter)) ;
                                break ;
                            case    LCBLC_OUTGOING_AWTING_ETBMNT : /* Outgoing Awaiting Establishment */
                                MyBLC._0304_0010(pLCEntry , (PS_OpenLogicalChannelAck)((uint8*)pParameter)) ;
                                break ;
                            case    LCBLC_OUTGOING_ETBED : /* Outgoing Established */
                            case    LCBLC_OUTGOING_AWTING_RLS : /* Outgoing Awaiting Release */
                                Nop_XXXX_XXXX() ;
                                break ;
                            case    LCBLC_INCOMING_RLSED : /* Incoming Released  */
                            case    LCBLC_INCOMING_AWTING_ETBMNT : /* Incoming Awaiting Establishment */
                            case    LCBLC_INCOMING_AWTING_CNFMTN : /* Incoming Awaiting Confirmation */
                            case    LCBLC_INCOMING_ETBED : /* Incoming Established */
                                /* Internal Error Notification ( To TSC ) */
                                ItnErrNotificationSend(ITN_ERR_CODE_0002 , EVENT_BLCSE & Se_Status) ;
                                break ;
                        }
                    }
                    break ;
                case    EVENT_MSG_LCBLC_OP_CFM : /* OpenLogicalChannelConfirm ( Bi-Directional Only ) */
                    Print("Received LCBLC OpenLogicalChannelConfirm\n");
                    pLCEntry = LCBLCIncoming.GetLCEntry(((PS_OpenLogicalChannelConfirm)((uint8*)pParameter))->forwardLogicalChannelNumber) ;
                    if (pLCEntry == NULL)
                    {
                        ItnErrNotificationSend(ITN_ERR_CODE_0002 , EVENT_LCSE) ;
                        break;
                    }
                    else
                    {
                        Print("    Using existing Logical Channel Entry : Incoming %d %d\n", pLCEntry->FwdLcnRead(), pLCEntry->RvsLcnRead());
                    }
                    /* Uni-Directional and Bi-Directional Logical Channel Status Read ( Each Logical Channel Number ) */
                    Se_Status = pLCEntry->StatusRead();
                    if (pLCEntry->DirectionalRead() == UNI_DIRECTIONAL)  /* Uni-Directional */
                    {
                        Print("    Channel Uni-Directional\n");
                        switch (Se_Status)  /* Uni-Directional Logical Channel Status Read */
                        {
                            case    LCBLC_OUTGOING_RLSED : /* Outgoing Released  */
                            case    LCBLC_OUTGOING_AWTING_ETBMNT : /* Outgoing Awaiting Establishment */
                            case    LCBLC_OUTGOING_ETBED : /* Outgoing Established */
                            case    LCBLC_OUTGOING_AWTING_RLS : /* Outgoing Awaiting Release */
                            case    LCBLC_INCOMING_RLSED : /* Incoming Released  */
                            case    LCBLC_INCOMING_AWTING_ETBMNT : /* Incoming Awaiting Establishment */
                            case    LCBLC_INCOMING_AWTING_CNFMTN : /* Incoming Awaiting Confirmation */
                            case    LCBLC_INCOMING_ETBED : /* Incoming Established */
                                /* Internal Error Notification ( To TSC ) */
                                ItnErrNotificationSend(ITN_ERR_CODE_0002 , EVENT_LCSE & Se_Status) ;
                                break ;
                        }
                    }
                    else /* Bi-Directional */
                    {
                        Print("    Channel Bi-Directional\n");
                        switch (Se_Status)  /* Bi-Directional Logical Channel Status Read */
                        {
                            case    LCBLC_OUTGOING_RLSED : /* Outgoing Released  */
                            case    LCBLC_OUTGOING_AWTING_ETBMNT : /* Outgoing Awaiting Establishment */
                            case    LCBLC_OUTGOING_ETBED : /* Outgoing Established */
                            case    LCBLC_OUTGOING_AWTING_RLS : /* Outgoing Awaiting Release */
                                /* Internal Error Notification ( To TSC ) */
                                ItnErrNotificationSend(ITN_ERR_CODE_0002 , EVENT_BLCSE & Se_Status) ;
                                break ;
                            case    LCBLC_INCOMING_RLSED : /* Incoming Released  */
                                Nop_XXXX_XXXX() ;    /* (RAN) Replaced error notn with NOP() as in SDL */
                                /* ItnErrNotificationSend( ITN_ERR_CODE_0002 , EVENT_BLCSE & Se_Status ) ;*/
                                break ;
                            case    LCBLC_INCOMING_AWTING_ETBMNT : /* Incoming Awaiting Establishment */
                                MyBLC._0305_0011(pLCEntry , (PS_OpenLogicalChannelConfirm)((uint8*)pParameter)) ;
                                break ;
                            case    LCBLC_INCOMING_AWTING_CNFMTN : /* Incoming Awaiting Confirmation */
                                MyBLC._0305_0021(pLCEntry , (PS_OpenLogicalChannelConfirm)((uint8*)pParameter)) ;
                                break ;
                            case    LCBLC_INCOMING_ETBED : /* Incoming Established */
                                Nop_XXXX_XXXX() ;
                                break ;
                        }
                    }
                    break ;
                case    EVENT_MSG_LCBLC_OP_RJT : /* OpenLogicalChannelReject */
                    Print("Received LCBLC OpenLogicalChannelReject\n");
                    pLCEntry = LCBLCOutgoing.GetLCEntry(((PS_OpenLogicalChannelReject)((uint8*)pParameter))->forwardLogicalChannelNumber) ;
                    if (pLCEntry == NULL)
                    {
                        ItnErrNotificationSend(ITN_ERR_CODE_0002 , EVENT_LCSE) ;
                        break;
                    }
                    else
                    {
                        Print("    Using existing Logical Channel Entry : Outgoing %d %d\n", pLCEntry->FwdLcnRead(), pLCEntry->RvsLcnRead());
                    }
                    /* Uni-Directional and Bi-Directional Logical Channel Status Read ( Each Logical Channel Number ) */
                    Se_Status = pLCEntry->StatusRead();
                    if (pLCEntry->DirectionalRead() == UNI_DIRECTIONAL)  /* Uni-Directional */
                    {
                        Print("    Channel Uni-Directional\n");
                        switch (Se_Status)  /* Uni-Directional Logical Channel Status Read */
                        {
                            case    LCBLC_OUTGOING_RLSED : /* Outgoing Released  */
                                MyLC._0206_0000(pLCEntry , (PS_OpenLogicalChannelReject)((uint8*)pParameter)) ;
                                break ;
                            case    LCBLC_OUTGOING_AWTING_ETBMNT : /* Outgoing Awaiting Establishment */
                                MyLC._0206_0010(pLCEntry , (PS_OpenLogicalChannelReject)((uint8*)pParameter)) ;
                                break ;
                            case    LCBLC_OUTGOING_ETBED : /* Outgoing Established */
                                MyLC._0206_0020(pLCEntry , (PS_OpenLogicalChannelReject)((uint8*)pParameter)) ;
                                break ;
                            case    LCBLC_OUTGOING_AWTING_RLS : /* Outgoing Awaiting Release */
                                MyLC._0206_0030(pLCEntry , (PS_OpenLogicalChannelReject)((uint8*)pParameter)) ;
                                break ;
                            case    LCBLC_INCOMING_RLSED : /* Incoming Released  */
                            case    LCBLC_INCOMING_AWTING_ETBMNT : /* Incoming Awaiting Establishment */
                            case    LCBLC_INCOMING_AWTING_CNFMTN : /* Incoming Awaiting Confirmation */
                            case    LCBLC_INCOMING_ETBED : /* Incoming Established */
                                /* Internal Error Notification ( To TSC ) */
                                ItnErrNotificationSend(ITN_ERR_CODE_0002 , EVENT_LCSE & Se_Status) ;
                                break ;
                        }
                    }
                    else /* Bi-Directional */
                    {
                        Print("    Channel Bi-Directional\n");
                        switch (Se_Status)  /* Bi-Directional Logical Channel Status Read */
                        {
                            case    LCBLC_OUTGOING_RLSED : /* Outgoing Released  */
                                MyBLC._0306_0000(pLCEntry , (PS_OpenLogicalChannelReject)((uint8*)pParameter)) ;
                                break ;
                            case    LCBLC_OUTGOING_AWTING_ETBMNT : /* Outgoing Awaiting Establishment */
                                MyBLC._0306_0010(pLCEntry , (PS_OpenLogicalChannelReject)((uint8*)pParameter)) ;
                                break ;
                            case    LCBLC_OUTGOING_ETBED : /* Outgoing Established */
                                MyBLC._0306_0020(pLCEntry , (PS_OpenLogicalChannelReject)((uint8*)pParameter)) ;
                                break ;
                            case    LCBLC_OUTGOING_AWTING_RLS : /* Outgoing Awaiting Release */
                                MyBLC._0306_0030(pLCEntry , (PS_OpenLogicalChannelReject)((uint8*)pParameter)) ;
                                break ;
                            case    LCBLC_INCOMING_RLSED : /* Incoming Released  */
                            case    LCBLC_INCOMING_AWTING_ETBMNT : /* Incoming Awaiting Establishment */
                            case    LCBLC_INCOMING_AWTING_CNFMTN : /* Incoming Awaiting Confirmation */
                            case    LCBLC_INCOMING_ETBED : /* Incoming Established */
                                /* Internal Error Notification ( To TSC ) */
                                ItnErrNotificationSend(ITN_ERR_CODE_0002 , EVENT_BLCSE & Se_Status) ;
                                break ;
                        }
                    }
                    break ;
                case    EVENT_TIM_LCBLC_T103 : /* Timer T103 Timeout */
                    Print("Received LCBLC T103 Timeout\n");
                    pLCEntry = LCBLCOutgoing.GetLCEntry(Supplement_2);
                    if (pLCEntry == NULL)
                    {
                        ItnErrNotificationSend(ITN_ERR_CODE_0002 , EVENT_LCSE) ;
                        break;
                    }
                    else
                    {
                        Print("    Using existing Logical Channel Entry : Outgoing %d %d\n", pLCEntry->FwdLcnRead(), pLCEntry->RvsLcnRead());
                    }
                    /* Uni-Directional and Bi-Directional Logical Channel Status Read ( Each Logical Channel Number ) */
                    Se_Status = pLCEntry->StatusRead();
                    if (pLCEntry->DirectionalRead() == UNI_DIRECTIONAL)  /* Uni-Directional */
                    {
                        Print("    Channel Uni-Directional\n");
                        switch (Se_Status)  /* Uni-Directional Logical Channel Status Read */
                        {
                            case    LCBLC_OUTGOING_RLSED : /* Outgoing Released  */
                                Nop_XXXX_XXXX() ;
                                break ;
                            case    LCBLC_OUTGOING_AWTING_ETBMNT : /* Outgoing Awaiting Establishment */
                                MyLC._0207_0010(pLCEntry , Supplement_1) ;
                                break ;
                            case    LCBLC_OUTGOING_ETBED : /* Outgoing Established */
                                Nop_XXXX_XXXX() ;
                                break ;
                            case    LCBLC_OUTGOING_AWTING_RLS : /* Outgoing Awaiting Release */
                                MyLC._0207_0030(pLCEntry , Supplement_1) ;
                                break ;
                            case    LCBLC_INCOMING_RLSED : /* Incoming Released  */
                            case    LCBLC_INCOMING_AWTING_ETBMNT : /* Incoming Awaiting Establishment */
                            case    LCBLC_INCOMING_AWTING_CNFMTN : /* Incoming Awaiting Confirmation */
                            case    LCBLC_INCOMING_ETBED : /* Incoming Established */
                                Nop_XXXX_XXXX() ;
                                break ;
                        }
                    }
                    else /* Bi-Directional */
                    {
                        Print("    Channel Bi-Directional\n");
                        switch (Se_Status)  /* Bi-Directional Logical Channel Status Read */
                        {
                            case    LCBLC_OUTGOING_RLSED : /* Outgoing Released  */
                                Nop_XXXX_XXXX() ;
                                break ;
                            case    LCBLC_OUTGOING_AWTING_ETBMNT : /* Outgoing Awaiting Establishment */
                                MyBLC._0307_0010(pLCEntry , Supplement_1) ;
                                break ;
                            case    LCBLC_OUTGOING_ETBED : /* Outgoing Established */
                                Nop_XXXX_XXXX() ;
                                break ;
                            case    LCBLC_OUTGOING_AWTING_RLS : /* Outgoing Awaiting Release */
                                MyBLC._0307_0030(pLCEntry , Supplement_1) ;
                                break ;
                            case    LCBLC_INCOMING_RLSED : /* Incoming Released  */
                            case    LCBLC_INCOMING_AWTING_ETBMNT : /* Incoming Awaiting Establishment */
                                Nop_XXXX_XXXX() ;
                                break ;
                            case    LCBLC_INCOMING_AWTING_CNFMTN : /* Incoming Awaiting Confirmation */
                                MyBLC._0307_0021(pLCEntry , Supplement_1) ;
                                break ;
                            case    LCBLC_INCOMING_ETBED : /* Incoming Established */
                                Nop_XXXX_XXXX() ;
                                break ;
                        }
                    }
                    break ;
                case    EVENT_MSG_LCBLC_CL : /* CloseLogicalChannel */
                    Print("Received LCBLC CloseLogicalChannel\n");
                    /* WWU_H245_TWOWAY */
                    pLCEntry = LCBLCIncoming.GetLCEntry(((PS_CloseLogicalChannel)((uint8*)pParameter))->forwardLogicalChannelNumber) ;
                    if (pLCEntry == NULL)
                    {
                        ItnErrNotificationSend(ITN_ERR_CODE_0002 , EVENT_LCSE) ;
                        break;
                    }
                    else
                    {
                        Print("    Using existing Logical Channel Entry : Incoming %d %d\n", pLCEntry->FwdLcnRead(), pLCEntry->RvsLcnRead());
                    }
                    /* Uni-Directional and Bi-Directional Logical Channel Status Read ( Each Logical Channel Number ) */
                    Se_Status = pLCEntry->StatusRead();
                    if (pLCEntry->DirectionalRead() == UNI_DIRECTIONAL)  /* Uni-Directional */
                    {
                        Print("    Channel Uni-Directional\n");
                        switch (Se_Status)  /* Uni-Directional Logical Channel Status Read */
                        {
                            case    LCBLC_OUTGOING_RLSED : /* Outgoing Released  */
                            case    LCBLC_OUTGOING_AWTING_ETBMNT : /* Outgoing Awaiting Establishment */
                            case    LCBLC_OUTGOING_ETBED : /* Outgoing Established */
                            case    LCBLC_OUTGOING_AWTING_RLS : /* Outgoing Awaiting Release */
                                /* Internal Error Notification ( To TSC ) */
                                ItnErrNotificationSend(ITN_ERR_CODE_0002 , EVENT_LCSE & Se_Status) ;
                                break ;
                            case    LCBLC_INCOMING_RLSED : /* Incoming Released  */
                                MyLC._0208_0001(pLCEntry , (PS_CloseLogicalChannel)((uint8*)pParameter)) ;
                                break ;
                            case    LCBLC_INCOMING_AWTING_ETBMNT : /* Incoming Awaiting Establishment */
                                MyLC._0208_0011(pLCEntry , (PS_CloseLogicalChannel)((uint8*)pParameter)) ;
                                break ;
                            case    LCBLC_INCOMING_AWTING_CNFMTN : /* Incoming Awaiting Confirmation */
                                /* Internal Error Notification ( To TSC ) */
                                ItnErrNotificationSend(ITN_ERR_CODE_0002 , EVENT_LCSE & Se_Status) ;
                                break ;
                            case    LCBLC_INCOMING_ETBED : /* Incoming Established */
                                MyLC._0208_0011(pLCEntry , (PS_CloseLogicalChannel)((uint8*)pParameter)) ;
                                break ;
                        }
                    }
                    else /* Bi-Directional */
                    {
                        Print("    Channel Bi-Directional\n");
                        switch (Se_Status)  /* Bi-Directional Logical Channel Status Read */
                        {
                            case    LCBLC_OUTGOING_RLSED : /* Outgoing Released  */
                            case    LCBLC_OUTGOING_AWTING_ETBMNT : /* Outgoing Awaiting Establishment */
                            case    LCBLC_OUTGOING_ETBED : /* Outgoing Established */
                            case    LCBLC_OUTGOING_AWTING_RLS : /* Outgoing Awaiting Release */
                                /* Internal Error Notification ( To TSC ) */
                                ItnErrNotificationSend(ITN_ERR_CODE_0002 , EVENT_BLCSE & Se_Status) ;
                                break ;
                            case    LCBLC_INCOMING_RLSED : /* Incoming Released  */
                                MyBLC._0308_0001(pLCEntry , (PS_CloseLogicalChannel)((uint8*)pParameter)) ;
                                break ;
                            case    LCBLC_INCOMING_AWTING_ETBMNT : /* Incoming Awaiting Establishment */
                                MyBLC._0308_0011(pLCEntry , (PS_CloseLogicalChannel)((uint8*)pParameter)) ;
                                break ;
                            case    LCBLC_INCOMING_AWTING_CNFMTN : /* Incoming Awaiting Confirmation */
                                MyBLC._0308_0021(pLCEntry , (PS_CloseLogicalChannel)((uint8*)pParameter)) ;
                                break ;
                            case    LCBLC_INCOMING_ETBED : /* Incoming Established */
                                MyBLC._0308_0011(pLCEntry , (PS_CloseLogicalChannel)((uint8*)pParameter)) ;
                                break ;
                        }
                    }
                    break ;
                case    EVENT_MSG_LCBLC_CL_ACK : /* CloseLogicalChannelAck */
                    Print("Received LCBLC CloseLogicalChannelAck\n");
                    pLCEntry = LCBLCOutgoing.GetLCEntry(((PS_CloseLogicalChannelAck)((uint8*)pParameter))->forwardLogicalChannelNumber) ;
                    if (pLCEntry == NULL)
                    {
                        ItnErrNotificationSend(ITN_ERR_CODE_0002 , EVENT_LCSE) ;
                        break;
                    }
                    else
                    {
                        Print("    Using existing Logical Channel Entry : Outgoing %d %d\n", pLCEntry->FwdLcnRead(), pLCEntry->RvsLcnRead());
                    }
                    /* Uni-Directional and Bi-Directional Logical Channel Status Read ( Each Logical Channel Number ) */
                    Se_Status = pLCEntry->StatusRead();
                    if (pLCEntry->DirectionalRead() == UNI_DIRECTIONAL)  /* Uni-Directional */
                    {
                        Print("    Channel Uni-Directional\n");
                        switch (Se_Status)  /* Uni-Directional Logical Channel Status Read */
                        {
                            case    LCBLC_OUTGOING_RLSED : /* Outgoing Released  */
                            case    LCBLC_OUTGOING_AWTING_ETBMNT : /* Outgoing Awaiting Establishment */
                                Nop_XXXX_XXXX() ;
                                break ;
                            case    LCBLC_OUTGOING_ETBED : /* Outgoing Established */
                                MyLC._0209_0020(pLCEntry , (PS_CloseLogicalChannelAck)((uint8*)pParameter)) ;
                                break ;
                            case    LCBLC_OUTGOING_AWTING_RLS : /* Outgoing Awaiting Release */
                                MyLC._0209_0030(pLCEntry , (PS_CloseLogicalChannelAck)((uint8*)pParameter)) ;
                                break ;
                            case    LCBLC_INCOMING_RLSED : /* Incoming Released  */
                            case    LCBLC_INCOMING_AWTING_ETBMNT : /* Incoming Awaiting Establishment */
                            case    LCBLC_INCOMING_AWTING_CNFMTN : /* Incoming Awaiting Confirmation */
                            case    LCBLC_INCOMING_ETBED : /* Incoming Established */
                                /* Internal Error Notification ( To TSC ) */
                                ItnErrNotificationSend(ITN_ERR_CODE_0002 , EVENT_LCSE & Se_Status) ;
                                break ;
                        }
                    }
                    else /* Bi-Directional */
                    {
                        Print("    Channel Bi-Directional\n");
                        switch (Se_Status)  /* Bi-Directional Logical Channel Status Read */
                        {
                            case    LCBLC_OUTGOING_RLSED : /* Outgoing Released  */
                            case    LCBLC_OUTGOING_AWTING_ETBMNT : /* Outgoing Awaiting Establishment */
                                Nop_XXXX_XXXX() ;
                                break ;
                            case    LCBLC_OUTGOING_ETBED : /* Outgoing Established */
                                MyBLC._0309_0020(pLCEntry , (PS_CloseLogicalChannelAck)((uint8*)pParameter)) ;
                                break ;
                            case    LCBLC_OUTGOING_AWTING_RLS : /* Outgoing Awaiting Release */
                                MyBLC._0309_0030(pLCEntry , (PS_CloseLogicalChannelAck)((uint8*)pParameter)) ;
                                break ;
                            case    LCBLC_INCOMING_RLSED : /* Incoming Released  */
                            case    LCBLC_INCOMING_AWTING_ETBMNT : /* Incoming Awaiting Establishment */
                            case    LCBLC_INCOMING_AWTING_CNFMTN : /* Incoming Awaiting Confirmation */
                            case    LCBLC_INCOMING_ETBED : /* Incoming Established */
                                /* Internal Error Notification ( To TSC ) */
                                ItnErrNotificationSend(ITN_ERR_CODE_0002 , EVENT_BLCSE & Se_Status) ;
                                break ;
                        }
                    }
                    break ;
            }
            break ;

        case    EVENT_CLCSE : /* Close Logical Channel Signalling Entity */
            /* Close Logical channel Status Read */
            pLCEntry = NULL;
            switch (Event_No)  /* Event No Check */
            {
                case    EVENT_PTV_CLC_CLS_REQ : /* CLOSE.request */
                    Print("Received CLC Close.Request\n");
                    lcn = Supplement_1;
                    pLCEntry = LCBLCIncoming.GetLCEntry(lcn);
                    if (pLCEntry == NULL)
                    {
                        ItnErrNotificationSend(ITN_ERR_CODE_0002 , EVENT_LCSE) ;
                        break;
                    }
                    else
                    {
                        Print("    Using existing Logical Channel Entry : Incoming %d %d\n", pLCEntry->FwdLcnRead(), pLCEntry->RvsLcnRead());
                    }
                    Se_Status = pLCEntry->ClcStatusRead();
                    switch (Se_Status)  /* Close Logical Chennel Status Check */
                    {
                        case    CLS_OUTGOING_IDLE : /* Outgoing Idle */
                            MyCLC._0400_0000(pLCEntry) ;
                            break ;
                        case    CLS_OUTGOING_AWTING_RPS : /* Outgoing Awaiting Response */
                            break ;
                        case    CLS_INCOMING_IDLE : /* Incoming Idle */
                            break ;
                        case    CLS_INCOMING_AWTING_RPS : /* Incoming Awaiting Response */
                            break ;
                    }
                    break ;
                case    EVENT_MSG_CLC : /* RequestChannelClose */
                    Print("Received CLC RequestChannelClose\n");
                    lcn = ((PS_RequestChannelClose)((uint8*)pParameter))->forwardLogicalChannelNumber;
                    pLCEntry = LCBLCOutgoing.GetLCEntry(lcn);
                    if (pLCEntry == NULL)
                    {
                        ItnErrNotificationSend(ITN_ERR_CODE_0002 , EVENT_LCSE) ;
                        break;
                    }
                    else
                    {
                        Print("    Using existing Logical Channel Entry : Outgoing %d %d\n", pLCEntry->FwdLcnRead(), pLCEntry->RvsLcnRead());
                    }
                    Se_Status = pLCEntry->ClcStatusRead();
                    switch (Se_Status)  /* Close Logical Chennel Status Check */
                    {
                        case    CLS_OUTGOING_IDLE : /* Outgoing Idle */
                            break ;
                        case    CLS_OUTGOING_AWTING_RPS : /* Outgoing Awaiting Response */
                            break ;
                        case    CLS_INCOMING_IDLE : /* Incoming Idle */
                            MyCLC._0401_0001(pLCEntry) ;
                            break ;
                        case    CLS_INCOMING_AWTING_RPS : /* Incoming Awaiting Response */
                            MyCLC._0401_0011(pLCEntry) ;
                            break ;
                    }
                    break ;
                case    EVENT_PTV_CLC_CLS_RPS : /* CLOSE.response */
                    Print("Received CLC Close.Response\n");
                    lcn = Supplement_1;
                    pLCEntry = LCBLCOutgoing.GetLCEntry(lcn);
                    if (pLCEntry == NULL)
                    {
                        ItnErrNotificationSend(ITN_ERR_CODE_0002 , EVENT_LCSE) ;
                        break;
                    }
                    else
                    {
                        Print("    Using existing Logical Channel Entry : Outgoing %d %d\n", pLCEntry->FwdLcnRead(), pLCEntry->RvsLcnRead());
                    }
                    Se_Status = pLCEntry->ClcStatusRead();
                    switch (Se_Status)  /* Close Logical Chennel Status Check */
                    {
                        case    CLS_OUTGOING_IDLE : /* Outgoing Idle */
                            break ;
                        case    CLS_OUTGOING_AWTING_RPS : /* Outgoing Awaiting Response */
                            break ;
                        case    CLS_INCOMING_IDLE : /* Incoming Idle */
                            break ;
                        case    CLS_INCOMING_AWTING_RPS : /* Incoming Awaiting Response */
                            MyCLC._0402_0011(pLCEntry) ;
                            break ;
                    }
                    break ;
                case    EVENT_PTV_CLC_RJT_REQ : /* REJECT.request */
                    Print("Received CLC Reject.Request\n");
                    lcn = Supplement_1;
                    pLCEntry = LCBLCOutgoing.GetLCEntry(lcn);
                    if (pLCEntry == NULL)
                    {
                        ItnErrNotificationSend(ITN_ERR_CODE_0002 , EVENT_LCSE) ;
                        break;
                    }
                    else
                    {
                        Print("    Using existing Logical Channel Entry : Outgoing %d %d\n", pLCEntry->FwdLcnRead(), pLCEntry->RvsLcnRead());
                    }
                    Se_Status = pLCEntry->ClcStatusRead();
                    switch (Se_Status)  /* Close Logical Chennel Status Check */
                    {
                        case    CLS_OUTGOING_IDLE : /* Outgoing Idle */
                            break ;
                        case    CLS_OUTGOING_AWTING_RPS : /* Outgoing Awaiting Response */
                            break ;
                        case    CLS_INCOMING_IDLE : /* Incoming Idle */
                            break ;
                        case    CLS_INCOMING_AWTING_RPS : /* Incoming Awaiting Response */
                            MyCLC._0403_0011(pLCEntry, (PS_RccRejectCause)((uint8*)pParameter)) ;
                            break ;
                    }
                    break ;
                case    EVENT_MSG_CLC_ACK : /* RequestChannelCloseAck */
                    Print("Received CLC RequestChannelCloseAck\n");
                    lcn = ((PS_RequestChannelCloseAck)((uint8*)pParameter))->forwardLogicalChannelNumber;
                    pLCEntry = LCBLCIncoming.GetLCEntry(lcn);
                    if (pLCEntry == NULL)
                    {
                        ItnErrNotificationSend(ITN_ERR_CODE_0002 , EVENT_LCSE) ;
                        break;
                    }
                    else
                    {
                        Print("    Using existing Logical Channel Entry : Incoming %d %d\n", pLCEntry->FwdLcnRead(), pLCEntry->RvsLcnRead());
                    }
                    Se_Status = pLCEntry->ClcStatusRead();
                    switch (Se_Status)  /* Close Logical Chennel Status Check */
                    {
                        case    CLS_OUTGOING_IDLE : /* Outgoing Idle */
                            break ;
                        case    CLS_OUTGOING_AWTING_RPS : /* Outgoing Awaiting Response */
                            MyCLC._0404_0010(pLCEntry) ;
                            break ;
                        case    CLS_INCOMING_IDLE : /* Incoming Idle */
                            break ;
                        case    CLS_INCOMING_AWTING_RPS : /* Incoming Awaiting Response */
                            break ;
                    }
                    break ;
                case    EVENT_MSG_CLC_RJT : /* RequestChannelCloseReject */
                    Print("Received CLC RequestChannelCloseReject\n");
                    lcn = ((PS_RequestChannelCloseReject)((uint8*)pParameter))->forwardLogicalChannelNumber;
                    pLCEntry = LCBLCIncoming.GetLCEntry(lcn);
                    if (pLCEntry == NULL)
                    {
                        ItnErrNotificationSend(ITN_ERR_CODE_0002 , EVENT_LCSE) ;
                        break;
                    }
                    else
                    {
                        Print("    Using existing Logical Channel Entry : Incoming %d %d\n", pLCEntry->FwdLcnRead(), pLCEntry->RvsLcnRead());
                    }
                    Se_Status = pLCEntry->ClcStatusRead();
                    switch (Se_Status)  /* Close Logical Chennel Status Check */
                    {
                        case    CLS_OUTGOING_IDLE : /* Outgoing Idle */
                            break ;
                        case    CLS_OUTGOING_AWTING_RPS : /* Outgoing Awaiting Response */
                            MyCLC._0405_0010(pLCEntry, (PS_RequestChannelCloseReject)((uint8*)pParameter)) ;
                            break ;
                        case    CLS_INCOMING_IDLE : /* Incoming Idle */
                            break ;
                        case    CLS_INCOMING_AWTING_RPS : /* Incoming Awaiting Response */
                            break ;
                    }
                    break ;
                case    EVENT_TIM_CLC_T108 : /* Timer T108 Timeout */
                    Print("Received CLC Timeout T108\n");
                    lcn = Supplement_2;
                    pLCEntry = LCBLCIncoming.GetLCEntry(lcn);
                    if (pLCEntry == NULL)
                    {
                        ItnErrNotificationSend(ITN_ERR_CODE_0002 , EVENT_LCSE) ;
                        break;
                    }
                    else
                    {
                        Print("    Using existing Logical Channel Entry : Incoming %d %d\n", pLCEntry->FwdLcnRead(), pLCEntry->RvsLcnRead());
                    }
                    Se_Status = pLCEntry->ClcStatusRead();
                    switch (Se_Status)  /* Close Logical Chennel Status Check */
                    {
                        case    CLS_OUTGOING_IDLE : /* Outgoing Idle */
                            break ;
                        case    CLS_OUTGOING_AWTING_RPS : /* Outgoing Awaiting Response */
                            MyCLC._0406_0010(pLCEntry, Supplement_1) ;
                            break ;
                        case    CLS_INCOMING_IDLE : /* Incoming Idle */
                            break ;
                        case    CLS_INCOMING_AWTING_RPS : /* Incoming Awaiting Response */
                            break ;
                    }
                    break ;
                case    EVENT_MSG_CLC_RLS : /* RequestChannelCloseRelease */
                    Print("Received CLC RequestChannelCloseRelease\n");
                    lcn = ((PS_RequestChannelCloseRelease)((uint8*)pParameter))->forwardLogicalChannelNumber;
                    pLCEntry = LCBLCOutgoing.GetLCEntry(lcn);
                    if (pLCEntry == NULL)
                    {
                        ItnErrNotificationSend(ITN_ERR_CODE_0002 , EVENT_LCSE) ;
                        break;
                    }
                    else
                    {
                        Print("    Using existing Logical Channel Entry : Outgoing %d %d\n", pLCEntry->FwdLcnRead(), pLCEntry->RvsLcnRead());
                    }
                    Se_Status = pLCEntry->ClcStatusRead();
                    switch (Se_Status)  /* Close Logical Chennel Status Check */
                    {
                        case    CLS_OUTGOING_IDLE : /* Outgoing Idle */
                            break ;
                        case    CLS_OUTGOING_AWTING_RPS : /* Outgoing Awaiting Response */
                            break ;
                        case    CLS_INCOMING_IDLE : /* Incoming Idle */
                            break ;
                        case    CLS_INCOMING_AWTING_RPS : /* Incoming Awaiting Response */
                            MyCLC._0407_0011(pLCEntry) ;
                            break ;
                    }
                    break ;
            }
            break ;
        case    EVENT_MTSE : /* Multiplex Table Signalling Entity */
            /* Multiplex Table Status Read */
            Se_Status2 = MyMT.StatusRead2() ;	/* WWU_H245_TWOWAY */
            switch (Event_No)  /* Event No Check */
            {
                case    EVENT_PTV_MT_TRF_REQ : /* TRANSFER.request */
                    Print("Received MT Transfer.Request\n");
                    MyMT.TransferRequest((PS_MuxDescriptor)((uint8*)pParameter)) ;
                    break ;
                case    EVENT_MSG_MT : /* MultiplexEntrySend */
                    Print("Received MT MultiplexEntrySend\n");
                    switch (Se_Status2)  /* Multiplex Table Status Check */
                    {
                        case    MT_INCOMING_IDLE : /* Incoming Idle */
                            MyMT._0501_0001((PS_MultiplexEntrySend)((uint8*)pParameter)) ;
                            break ;
                        case    MT_INCOMING_AWTING_RPS : /* Incoming Awaiting Response */
                            MyMT._0501_0011((PS_MultiplexEntrySend)((uint8*)pParameter)) ;
                            break ;
                    }
                    break ;
                case    EVENT_PTV_MT_TRF_RPS : /* TRANSFER.response */
                    Print("Received MT Transfer.Response\n");
                    switch (Se_Status2)  /* Multiplex Table Status Check */
                    {
                        case    MT_INCOMING_IDLE : /* Incoming Idle */
                            /* Internal Error Notification ( To TSC ) */
                            ItnErrNotificationSend(ITN_ERR_CODE_0002 , Event_No & Se_Status2) ;
                            break ;
                        case    MT_INCOMING_AWTING_RPS : /* Incoming Awaiting Response */
                            MyMT._0502_0011(Supplement_1, (PS_MuxDescriptor)((uint8*)pParameter)) ;
                            break ;
                    }
                    break ;
                case    EVENT_PTV_MT_RJT_REQ : /* REJECT.request */
                    Print("Received MT Reject.Request\n");
                    switch (Se_Status2)  /* Multiplex Table Status Check */
                    {
                        case    MT_OUTGOING_IDLE : /* Outgoing Idle */
                        case    MT_OUTGOING_AWTING_RPS : /* Outgoing Awaiting Response */
                        case    MT_INCOMING_IDLE : /* Incoming Idle */
                            /* Internal Error Notification ( To TSC ) */
                            ItnErrNotificationSend(ITN_ERR_CODE_0002 , Event_No & Se_Status2) ;
                            break ;
                        case    MT_INCOMING_AWTING_RPS : /* Incoming Awaiting Response */
                            MyMT._0503_0011((PS_MeRejectCause)((uint8*)pParameter)) ;
                            break ;
                    }
                    break ;
                case    EVENT_MSG_MT_ACK : /* MultiplexEntrySendAck */
                    Print("Received MT MultiplexEntrySendAck\n");
                    MyMT.MultiplexEntrySendAck((PS_MultiplexEntrySendAck)((uint8*)pParameter)) ;
                    break ;
                case    EVENT_MSG_MT_RJT : /* MultiplexEntrySendReject */
                    Print("Received MT MultiplexEntrySendReject\n");
                    MyMT.MultiplexEntrySendReject((PS_MultiplexEntrySendReject)((uint8*)pParameter));
                    break ;
                case    EVENT_MSG_MT_RLS : /* MultiplexEntrySendRelease */
                    Print("Received MT MultiplexEntrySendRelease\n");
                    switch (Se_Status2)  /* Multiplex Table Status Check */
                    {
                        case    MT_OUTGOING_IDLE : /* Outgoing Idle */
                        case    MT_OUTGOING_AWTING_RPS : /* Outgoing Awaiting Response */
                            /* Internal Error Notification ( To TSC ) */
                            ItnErrNotificationSend(ITN_ERR_CODE_0002 , Event_No & Se_Status2) ;
                            break ;
                        case    MT_INCOMING_IDLE : /* Incoming Idle */
                            Nop_XXXX_XXXX() ;
                            break ;
                        case    MT_INCOMING_AWTING_RPS : /* Incoming Awaiting Response */
                            MyMT._0507_0011() ;
                            break ;
                    }
                    break ;
            }
            break ;
        case    EVENT_RMESE : /* Request Multiplex Entry Signalling Entity */
            /* Request Multiplex Entry Status Read */
            Se_Status = MyRME.StatusRead() ;
            Se_Status2 = MyRME.StatusRead2() ;	/* H245_TWOWAY */
            /* Note: Se_Status is OUTGOING, Se_Status2 is INCOMING */
            switch (Event_No)  /* Event No Check */
            {
                case    EVENT_PTV_RME_SEND_REQ : /* SEND.request */
                    Print("Received RME Send.Request\n");
                    switch (Se_Status)  /* Request Multiplex Entry Status Check */
                    {
                        case    RME_OUTGOING_IDLE : /* Outgoing Idle */
                            MyRME._0600_0000() ;
                            break ;
                        case    RME_OUTGOING_AWTING_RPS : /* Outgoing Awaitimg Response */
                            break ;
                        case    RME_INCOMING_IDLE : /* Incoming Idle */
                            break ;
                        case    RME_INCOMING_AWTING_RPS : /* Incoming Awaiting Response */
                            break ;
                    }
                    break ;
                case    EVENT_MSG_RME : /* RequestMultiplexEntry */
                    Print("Received RME RequestMultiplexEntry\n");
                    switch (Se_Status2)  /* Request Multiplex Entry Status Check */
                    {
                        case    RME_OUTGOING_IDLE : /* Outgoing Idle */
                            break ;
                        case    RME_OUTGOING_AWTING_RPS : /* Outgoing Awaitimg Response */
                            break ;
                        case    RME_INCOMING_IDLE : /* Incoming Idle */
                            MyRME._0601_0001((PS_RequestMultiplexEntry)((uint8*)pParameter)) ;
                            break ;
                        case    RME_INCOMING_AWTING_RPS : /* Incoming Awaiting Response */
                            MyRME._0601_0011((PS_RequestMultiplexEntry)((uint8*)pParameter)) ;
                            break ;
                    }
                    break ;
                case    EVENT_PTV_RME_SEND_RPS : /* SEND.responsse */
                    Print("Received RME Send.Response\n");
                    switch (Se_Status2)  /* Request Multiplex Entry Status Check */
                    {
                        case    RME_OUTGOING_IDLE : /* Outgoing Idle */
                            break ;
                        case    RME_OUTGOING_AWTING_RPS : /* Outgoing Awaitimg Response */
                            break ;
                        case    RME_INCOMING_IDLE : /* Incoming Idle */
                            break ;
                        case    RME_INCOMING_AWTING_RPS : /* Incoming Awaiting Response */
                            MyRME._0602_0011() ;
                            break ;
                    }
                    break ;
                case    EVENT_PTV_RME_RJT_REQ : /* REJECT.request */
                    Print("Received RME Reject.Request\n");
                    switch (Se_Status2)  /* Request Multiplex Entry Status Check */
                    {
                        case    RME_OUTGOING_IDLE : /* Outgoing Idle */
                            break ;
                        case    RME_OUTGOING_AWTING_RPS : /* Outgoing Awaitimg Response */
                            break ;
                        case    RME_INCOMING_IDLE : /* Incoming Idle */
                            break ;
                        case    RME_INCOMING_AWTING_RPS : /* Incoming Awaiting Response */
                            MyRME._0603_0011((PS_RmeRejectCause)((uint8*)pParameter)) ;
                            break ;
                    }
                    break ;
                case    EVENT_MSG_RME_ACK : /* RequestMultiplexEntryAck */
                    Print("Received RME RequestMultiplexEntryAck\n");
                    switch (Se_Status)  /* Request Multiplex Entry Status Check */
                    {
                        case    RME_OUTGOING_IDLE : /* Outgoing Idle */
                            break ;
                        case    RME_OUTGOING_AWTING_RPS : /* Outgoing Awaitimg Response */
                            MyRME._0604_0010((PS_RequestMultiplexEntryAck)((uint8*)pParameter)) ;
                            break ;
                        case    RME_INCOMING_IDLE : /* Incoming Idle */
                            break ;
                        case    RME_INCOMING_AWTING_RPS : /* Incoming Awaiting Response */
                            break ;
                    }
                    break ;
                case    EVENT_MSG_RME_RJT : /* RequestMultiplexEntryReject */
                    Print("Received RME RequestMultiplexEntryReject\n");
                    switch (Se_Status)  /* Request Multiplex Entry Status Check */
                    {
                        case    RME_OUTGOING_IDLE : /* Outgoing Idle */
                            break ;
                        case    RME_OUTGOING_AWTING_RPS : /* Outgoing Awaitimg Response */
                            MyRME._0605_0010((PS_RequestMultiplexEntryReject)((uint8*)pParameter)) ;
                            break ;
                        case    RME_INCOMING_IDLE : /* Incoming Idle */
                            break ;
                        case    RME_INCOMING_AWTING_RPS : /* Incoming Awaiting Response */
                            break ;
                    }
                    break ;
                case    EVENT_TIM_RME_T107 : /* Timer T107 Timeout */
                    Print("Received RME Timeout T107\n");
                    switch (Se_Status)  /* Request Multiplex Entry Status Check */
                    {
                        case    RME_OUTGOING_IDLE : /* Outgoing Idle */
                            break ;
                        case    RME_OUTGOING_AWTING_RPS : /* Outgoing Awaitimg Response */
                            MyRME._0606_0010(Supplement_1) ;
                            break ;
                        case    RME_INCOMING_IDLE : /* Incoming Idle */
                            break ;
                        case    RME_INCOMING_AWTING_RPS : /* Incoming Awaiting Response */
                            break ;
                    }
                    break ;
                case    EVENT_MSG_RME_RLS : /* RequestMultiplexEntryRelease */
                    Print("Received RME RequestMultiplexEntryRelease\n");
                    switch (Se_Status2)  /* Request Multiplex Entry Status Check */
                    {
                        case    RME_OUTGOING_IDLE : /* Outgoing Idle */
                            break ;
                        case    RME_OUTGOING_AWTING_RPS : /* Outgoing Awaitimg Response */
                            break ;
                        case    RME_INCOMING_IDLE : /* Incoming Idle */
                            break ;
                        case    RME_INCOMING_AWTING_RPS : /* Incoming Awaiting Response */
                            MyRME._0607_0011((PS_RequestMultiplexEntryRelease)((uint8*)pParameter)) ;
                            break ;
                    }
                    break ;
            }
            break ;
        case    EVENT_MRSE : /* Mode Request Signalling Entry */
            /* Mode Request Status Read */
            Se_Status = MyMR.StatusRead() ;
            Se_Status2 = MyMR.StatusRead2() ;
            switch (Event_No)  /* Event No Check */
            {
                case    EVENT_PTV_MR_TRF_REQ : /* TRANSFER.request */
                    Print("Received MR Transfer.Request\n");
                    switch (Se_Status)  /* Mode Request Status Check */
                    {
                        case    MR_OUTGOING_IDLE : /* OutGoing Idle */
                            MyMR._0700_0000((PS_RequestMode)((uint8*)pParameter)) ;
                            break ;
                        case    MR_OUTGOING_AWTING_RPS : /* Outgoing Awaiting Response */
                            MyMR._0700_0010((PS_RequestMode)((uint8*)pParameter)) ;
                            break ;
                        case    MR_INCOMING_IDLE : /* Incoming Idle */
                            break ;
                        case    MR_INCOMING_AWTING_RPS : /* Incoming Awaiting Response */
                            break ;
                    }
                    break ;
                case    EVENT_MSG_MR : /* RequestMode */
                    Print("Received MR RequestMode\n");
                    switch (Se_Status2)  /* Mode Request Status Check */
                    {
                        case    MR_OUTGOING_IDLE : /* OutGoing Idle */
                            break ;
                        case    MR_OUTGOING_AWTING_RPS : /* Outgoing Awaiting Response */
                            break ;
                        case    MR_INCOMING_IDLE : /* Incoming Idle */
                            MyMR._0701_0001((PS_RequestMode)((uint8*)pParameter)) ;
                            break ;
                        case    MR_INCOMING_AWTING_RPS : /* Incoming Awaiting Response */
                            MyMR._0701_0011((PS_RequestMode)((uint8*)pParameter)) ;
                            break ;
                    }
                    break ;
                case    EVENT_PTV_MR_TRF_RPS : /* TRANSFER.response */
                    Print("Received MR Transfer.Response\n");
                    switch (Se_Status2)  /* Mode Request Status Check */
                    {
                        case    MR_OUTGOING_IDLE : /* OutGoing Idle */
                            break ;
                        case    MR_OUTGOING_AWTING_RPS : /* Outgoing Awaiting Response */
                            break ;
                        case    MR_INCOMING_IDLE : /* Incoming Idle */
                            break ;
                        case    MR_INCOMING_AWTING_RPS : /* Incoming Awaiting Response */
                            MyMR._0702_0011((PS_Response)((uint8*)pParameter)) ;
                            break ;
                    }
                    break ;
                case    EVENT_PTV_MR_RJT_REQ : /* REJECT.request */
                    Print("Received MR Reject.Request\n");
                    switch (Se_Status2)  /* Mode Request Status Check */
                    {
                        case    MR_OUTGOING_IDLE : /* OutGoing Idle */
                            break ;
                        case    MR_OUTGOING_AWTING_RPS : /* Outgoing Awaiting Response */
                            break ;
                        case    MR_INCOMING_IDLE : /* Incoming Idle */
                            break ;
                        case    MR_INCOMING_AWTING_RPS : /* Incoming Awaiting Response */
                            MyMR._0703_0011((PS_RmRejectCause)((uint8*)pParameter)) ;
                            break ;
                    }
                    break ;
                case    EVENT_MSG_MR_ACK : /* RequestModeAck */
                    Print("Received MR RequestModeAck\n");
                    switch (Se_Status)  /* Mode Request Status Check */
                    {
                        case    MR_OUTGOING_IDLE : /* OutGoing Idle */
                            break ;
                        case    MR_OUTGOING_AWTING_RPS : /* Outgoing Awaiting Response */
                            MyMR._0704_0010((PS_RequestModeAck)((uint8*)pParameter)) ;
                            break ;
                        case    MR_INCOMING_IDLE : /* Incoming Idle */
                            break ;
                        case    MR_INCOMING_AWTING_RPS : /* Incoming Awaiting Response */
                            break ;
                    }
                    break ;
                case    EVENT_MSG_MR_RJT : /* RequestModeReject */
                    Print("Received MR RequestModeReject\n");
                    switch (Se_Status)  /* Mode Request Status Check */
                    {
                        case    MR_OUTGOING_IDLE : /* OutGoing Idle */
                            break ;
                        case    MR_OUTGOING_AWTING_RPS : /* Outgoing Awaiting Response */
                            MyMR._0705_0010((PS_RequestModeReject)((uint8*)pParameter)) ;
                            break ;
                        case    MR_INCOMING_IDLE : /* Incoming Idle */
                            break ;
                        case    MR_INCOMING_AWTING_RPS : /* Incoming Awaiting Response */
                            break ;
                    }
                    break ;
                case    EVENT_TIM_MR_T109 : /* Timer T109 Timeout */
                    Print("Received MR Timeout T109\n");
                    switch (Se_Status)  /* Mode Request Status Check */
                    {
                        case    MR_OUTGOING_IDLE : /* OutGoing Idle */
                            break ;
                        case    MR_OUTGOING_AWTING_RPS : /* Outgoing Awaiting Response */
                            MyMR._0706_0010(Supplement_1) ;
                            break ;
                        case    MR_INCOMING_IDLE : /* Incoming Idle */
                            break ;
                        case    MR_INCOMING_AWTING_RPS : /* Incoming Awaiting Response */
                            break ;
                    }
                    break ;
                case    EVENT_MSG_MR_RLS : /* RequestModeReject */
                    Print("Received MR RequestModeReject\n");
                    switch (Se_Status2)  /* Mode Request Status Check */
                    {
                        case    MR_OUTGOING_IDLE : /* OutGoing Idle */
                            break ;
                        case    MR_OUTGOING_AWTING_RPS : /* Outgoing Awaiting Response */
                            break ;
                        case    MR_INCOMING_IDLE : /* Incoming Idle */
                            break ;
                        case    MR_INCOMING_AWTING_RPS : /* Incoming Awaiting Response */
                            MyMR._0707_0011() ;
                            break ;
                    }
                    break ;
            }
            break ;
        case    EVENT_RTDSE : /* Round Trip Delay Signaling Entity */
            /* Round Trip Delay Status Read */
            Se_Status = MyRTD.StatusRead() ;
            switch (Event_No)  /* Event No Check */
            {
                case    EVENT_PTV_RTD_TRF_REQ : /* TRANSFER.request */
                    Print("Received RTD Transfer.Request\n");
                    switch (Se_Status)  /* Round Trip Delay Status Check */
                    {
                        case    RTD_IDLE : /* Idle */
                            MyRTD._0800_0000() ;
                            break ;
                        case    RTD_AWTING_RPS : /* Awaiting Response */
                            MyRTD._0800_0010() ;
                            break ;
                    }
                    break ;
                case    EVENT_MSG_RTD_REQ : /* RoundTripDelayRequest */
                    Print("Received RTD RoundTripDelayRequest\n");
                    switch (Se_Status)  /* Round Trip Delay Status Check */
                    {
                        case    RTD_IDLE : /* Idle */
                        case    RTD_AWTING_RPS : /* Awaiting Response */
                            MyRTD._0801_0000((PS_RoundTripDelayRequest)((uint8*)pParameter)) ;
                            break ;
                    }
                    break ;
                case    EVENT_MSG_RTD_RPS : /* RoundTripDelayResponse */
                    Print("Received RTD RoundTripDelayResponse\n");
                    switch (Se_Status)  /* Round Trip Delay Status Check */
                    {
                        case    RTD_IDLE : /* Idle */
                            break ;
                        case    RTD_AWTING_RPS : /* Awaiting Response */
                            MyRTD._0802_0010((PS_RoundTripDelayResponse)((uint8*)pParameter)) ;
                            break ;
                    }
                    break ;
                case    EVENT_TIM_RTD_T105 : /* Timer T105 Timeout */
                    Print("Received RTD Timeout T105\n");
                    switch (Se_Status)  /* Round Trip Delay Status Check */
                    {
                        case    RTD_IDLE : /* Idle */
                            break ;
                        case    RTD_AWTING_RPS : /* Awaiting Response */
                            MyRTD._0803_0010(Supplement_1) ;
                            break ;
                    }
                    break ;
            }
            break ;
        case    EVENT_MLSE : /* Maintenance Loop Signalling Entity */
            /* Maintenance Loop Status Read */
            Se_Status = MyML.StatusRead() ;
            switch (Event_No)  /* Event No Check */
            {
                case    EVENT_PTV_ML_LOOP_REQ : /* LOOP.request */
                    switch (Se_Status)  /* Maintenance Loop Status Check */
                    {
                        case    ML_OUTGOING_NOT_LOOPED : /* Outgoing Not Looped */
                            MyML._0900_0000((PS_MlRequestType)((uint8*)pParameter)) ;
                            break ;
                        case    ML_OUTGOING_AWTING_RPS : /* Outgoing Awaiting Response */
                            break ;
                        case    ML_OUTGOING_LOOPED : /* Outgoing Looped */
                            break ;
                        case    ML_INCOMING_NOT_LOOPED : /* Incoming Not Looped */
                            break ;
                        case    ML_INCOMING_AWTING_RPS : /* Incoming Awaiting Response */
                            break ;
                        case    ML_INCOMING_LOOPED : /* Incoming Looped */
                            break ;
                    }
                    break ;
                case    EVENT_PTV_ML_RLS_REQ : /* RELEASE.request */
                    switch (Se_Status)  /* Maintenance Loop Status Check */
                    {
                        case    ML_OUTGOING_NOT_LOOPED : /* Outgoing Not Looped */
                            break ;
                        case    ML_OUTGOING_AWTING_RPS : /* Outgoing Awaiting Response */
                            MyML._0901_0010((PS_MlRejectCause)((uint8*)pParameter)) ;
                            break ;
                        case    ML_OUTGOING_LOOPED : /* Outgoing Looped */
                            MyML._0901_0020((PS_MlRejectCause)((uint8*)pParameter)) ;
                            break ;
                        case    ML_INCOMING_NOT_LOOPED : /* Incoming Not Looped */
                            break ;
                        case    ML_INCOMING_AWTING_RPS : /* Incoming Awaiting Response */
                            MyML._0901_0011((PS_MlRejectCause)((uint8*)pParameter)) ;
                            break ;
                        case    ML_INCOMING_LOOPED : /* Incoming Looped */
                            break ;
                    }
                    break ;
                case    EVENT_MSG_ML_REQ : /* MaintenanceLoopRequest */
                    switch (Se_Status)  /* Maintenance Loop Status Check */
                    {
                        case    ML_OUTGOING_NOT_LOOPED : /* Outgoing Not Looped */
                            break ;
                        case    ML_OUTGOING_AWTING_RPS : /* Outgoing Awaiting Response */
                            break ;
                        case    ML_OUTGOING_LOOPED : /* Outgoing Looped */
                            break ;
                        case    ML_INCOMING_NOT_LOOPED : /* Incoming Not Looped */
                            MyML._0902_0001((PS_MaintenanceLoopRequest)((uint8*)pParameter)) ;
                            break ;
                        case    ML_INCOMING_AWTING_RPS : /* Incoming Awaiting Response */
                        case    ML_INCOMING_LOOPED : /* Incoming Looped */
                            MyML._0902_0011((PS_MaintenanceLoopRequest)((uint8*)pParameter)) ;
                            break ;
                    }
                    break ;
                case    EVENT_PTV_ML_LOOP_RPS : /* LOOP.response */
                    switch (Se_Status)  /* Maintenance Loop Status Check */
                    {
                        case    ML_OUTGOING_NOT_LOOPED : /* Outgoing Not Looped */
                            break ;
                        case    ML_OUTGOING_AWTING_RPS : /* Outgoing Awaiting Response */
                            break ;
                        case    ML_OUTGOING_LOOPED : /* Outgoing Looped */
                            break ;
                        case    ML_INCOMING_NOT_LOOPED : /* Incoming Not Looped */
                            break ;
                        case    ML_INCOMING_AWTING_RPS : /* Incoming Awaiting Response */
                            MyML._0903_0011() ;
                            break ;
                        case    ML_INCOMING_LOOPED : /* Incoming Looped */
                            break ;
                    }
                    break ;
                case    EVENT_MSG_ML_ACK : /* MaintenanceLoopAck */
                    switch (Se_Status)  /* Maintenance Loop Status Check */
                    {
                        case    ML_OUTGOING_NOT_LOOPED : /* Outgoing Not Looped */
                            break ;
                        case    ML_OUTGOING_AWTING_RPS : /* Outgoing Awaiting Response */
                            MyML._0904_0010((PS_MaintenanceLoopAck)((uint8*)pParameter)) ;
                            break ;
                        case    ML_OUTGOING_LOOPED : /* Outgoing Looped */
                            break ;
                        case    ML_INCOMING_NOT_LOOPED : /* Incoming Not Looped */
                            break ;
                        case    ML_INCOMING_AWTING_RPS : /* Incoming Awaiting Response */
                            break ;
                        case    ML_INCOMING_LOOPED : /* Incoming Looped */
                            break ;
                    }
                    break ;
                case    EVENT_MSG_ML_RJT : /* MaintenanceLoopReject */
                    switch (Se_Status)  /* Maintenance Loop Status Check */
                    {
                        case    ML_OUTGOING_NOT_LOOPED : /* Outgoing Not Looped */
                            break ;
                        case    ML_OUTGOING_AWTING_RPS : /* Outgoing Awaiting Response */
                            MyML._0905_0010((PS_MaintenanceLoopReject)((uint8*)pParameter)) ;
                            break ;
                        case    ML_OUTGOING_LOOPED : /* Outgoing Looped */
                            MyML._0905_0020((PS_MaintenanceLoopReject)((uint8*)pParameter)) ;
                            break ;
                        case    ML_INCOMING_NOT_LOOPED : /* Incoming Not Looped */
                            break ;
                        case    ML_INCOMING_AWTING_RPS : /* Incoming Awaiting Response */
                            break ;
                        case    ML_INCOMING_LOOPED : /* Incoming Looped */
                            break ;
                    }
                    break ;
                case    EVENT_TIM_ML_T102 : /* Timer T102 Timeout */
                    switch (Se_Status)  /* Maintenance Loop Status Check */
                    {
                        case    ML_OUTGOING_NOT_LOOPED : /* Outgoing Not Looped */
                            break ;
                        case    ML_OUTGOING_AWTING_RPS : /* Outgoing Awaiting Response */
                            MyML._0906_0010(Supplement_1) ;
                            break ;
                        case    ML_OUTGOING_LOOPED : /* Outgoing Looped */
                            break ;
                        case    ML_INCOMING_NOT_LOOPED : /* Incoming Not Looped */
                            break ;
                        case    ML_INCOMING_AWTING_RPS : /* Incoming Awaiting Response */
                            break ;
                        case    ML_INCOMING_LOOPED : /* Incoming Looped */
                            break ;
                    }
                    break ;
                case    EVENT_MSG_ML_OFF_CMD : /* MaintenanceLoopOffCommand */
                    switch (Se_Status)  /* Maintenance Loop Status Check */
                    {
                        case    ML_OUTGOING_NOT_LOOPED : /* Outgoing Not Looped */
                            break ;
                        case    ML_OUTGOING_AWTING_RPS : /* Outgoing Awaiting Response */
                            break ;
                        case    ML_OUTGOING_LOOPED : /* Outgoing Looped */
                            break ;
                        case    ML_INCOMING_NOT_LOOPED : /* Incoming Not Looped */
                            break ;
                        case    ML_INCOMING_AWTING_RPS : /* Incoming Awaiting Response */
                        case    ML_INCOMING_LOOPED : /* Incoming Looped */
                            MyML._0907_0011() ;
                            break ;
                    }
                    break ;
            }
            break ;
        case    EVENT_NSD : /* Non Standard */
            switch (Event_No)  /* Event No Check */
            {
                case    EVENT_PTV_NSD_REQ : /* NonStandardMessageRequest Primitive */
                    MyNSD._1000_XXXX((PS_NonStandardMessage)((uint8*)pParameter)) ;
                    break ;
                case    EVENT_MSG_NSD_REQ : /* NonStandardMessageRequest */
                    MyNSD._1001_XXXX((PS_NonStandardMessage)((uint8*)pParameter)) ;
                    break ;
                case    EVENT_PTV_NSD_RPS : /* NonStandardMessageResponse Primitive */
                    MyNSD._1002_XXXX((PS_NonStandardMessage)((uint8*)pParameter)) ;
                    break ;
                case    EVENT_MSG_NSD_RPS : /* NonStandardMessageResponse */
                    MyNSD._1003_XXXX((PS_NonStandardMessage)((uint8*)pParameter)) ;
                    break ;
                case    EVENT_PTV_NSD_CMD : /* NonStandardMessageCommand Primitive */
                    MyNSD._1004_XXXX((PS_NonStandardMessage)((uint8*)pParameter)) ;
                    break ;
                case    EVENT_MSG_NSD_CMD : /* NonStandardMessageCommand */
                    MyNSD._1005_XXXX((PS_NonStandardMessage)((uint8*)pParameter)) ;
                    break ;
                case    EVENT_PTV_NSD_IDC : /* NonStandardMessageIndication Primitive */
                    MyNSD._1006_XXXX((PS_NonStandardMessage)((uint8*)pParameter)) ;
                    break ;
                case    EVENT_MSG_NSD_IDC : /* NonStandardMessageIndication */
                    MyNSD._1007_XXXX((PS_NonStandardMessage)((uint8*)pParameter)) ;
                    break ;
            }
            break ;
        case    EVENT_CM : /* Communication Mode */
            switch (Event_No)  /* Event No Check */
            {
                case    EVENT_PTV_CM_REQ : /* CommunicationModeRequest Primitive */
                    MyCM._1100_XXXX((PS_CommunicationModeRequest)((uint8*)pParameter)) ;
                    break ;
                case    EVENT_MSG_CM_REQ : /* CommunicationModeRequest */
                    MyCM._1101_XXXX((PS_CommunicationModeRequest)((uint8*)pParameter)) ;
                    break ;
                case    EVENT_PTV_CM_RPS : /* CommunicationModeResponse Primitive */
                    MyCM._1102_XXXX((PS_CommunicationModeResponse)((uint8*)pParameter)) ;
                    break ;
                case    EVENT_MSG_CM_RPS : /* CommunicationModeResponse */
                    MyCM._1103_XXXX((PS_CommunicationModeResponse)((uint8*)pParameter)) ;
                    break ;
                case    EVENT_PTV_CM_CMD : /* CommunicationModeCommand Primitive */
                    MyCM._1104_XXXX((PS_CommunicationModeCommand)((uint8*)pParameter)) ;
                    break ;
                case    EVENT_MSG_CM_CMD : /* CommunicationModeCommand */
                    MyCM._1105_XXXX((PS_CommunicationModeCommand)((uint8*)pParameter)) ;
                    break ;
            }
            break ;
        case    EVENT_H223AAR : /* H223 Annex A Reconfiguration */
            /* ---------------------------------- */
            /* H223AnnexAReConfiguration Removed. */
            /* (Not a 245 codeword) (RAN)         */
            /* ---------------------------------- */
            break ;
        case    EVENT_CNF : /* Conference */
            switch (Event_No)  /* Event No Check */
            {
                case    EVENT_PTV_CNF_REQ : /* ConferenceRequest Primitive */
                    MyCNF._1300_XXXX((PS_ConferenceRequest)((uint8*)pParameter)) ;
                    break ;
                case    EVENT_MSG_CNF_REQ : /* ConferenceRequest */
                    MyCNF._1301_XXXX((PS_ConferenceRequest)((uint8*)pParameter)) ;
                    break ;
                case    EVENT_PTV_CNF_RPS : /* ConferenceResponse Primitive */
                    MyCNF._1302_XXXX((PS_ConferenceResponse)((uint8*)pParameter)) ;
                    break ;
                case    EVENT_MSG_CNF_RPS : /* ConferenceResponse */
                    MyCNF._1303_XXXX((PS_ConferenceResponse)((uint8*)pParameter)) ;
                    break ;
            }
            break ;
        case    EVENT_CMD : /* Command */
            switch (Event_No)  /* Event No Check */
            {
                case    EVENT_PTV_CMD_STCS : /* SendTerminalCapabilitySet Primitive */
                    MyCMD._1400_XXXX((PS_SendTerminalCapabilitySet)((uint8*)pParameter)) ;
                    break ;
                case    EVENT_MSG_CMD_STCS : /* SendTerminalCapabilitySet */
                    MyCMD._1401_XXXX((PS_SendTerminalCapabilitySet)((uint8*)pParameter)) ;
                    break ;
                case    EVENT_PTV_CMD_ECRPT : /* EncryptionCommand Primitive */
                    MyCMD._1402_XXXX((PS_EncryptionCommand)((uint8*)pParameter)) ;
                    break ;
                case    EVENT_MSG_CMD_ECRPT : /* EncryptionCommand */
                    MyCMD._1403_XXXX((PS_EncryptionCommand)((uint8*)pParameter)) ;
                    break ;
                case    EVENT_PTV_CMD_FC : /* FlowControlCommand Primitive */
                    MyCMD._1404_XXXX((PS_FlowControlCommand)((uint8*)pParameter)) ;
                    break ;
                case    EVENT_MSG_CMD_FC : /* FlowControlCommand */
                    MyCMD._1405_XXXX((PS_FlowControlCommand)((uint8*)pParameter), EncodedMsg, EncodedMsgSize) ;
                    break ;
                case    EVENT_PTV_CMD_ES : /* ENDSESSION */
                    MyCMD._1406_XXXX((PS_EndSessionCommand)((uint8*)pParameter)) ;
                    break ;
                case    EVENT_MSG_CMD_ES : /* EndSessionCommand */
                    MyCMD._1407_XXXX((PS_EndSessionCommand)((uint8*)pParameter)) ;
                    break ;
                case    EVENT_PTV_CMD_MSCL : /* MiscellaneousCommand Primitive */
                    MyCMD._1408_XXXX((PS_MiscellaneousCommand)((uint8*)pParameter)) ;
                    break ;
                case    EVENT_MSG_CMD_MSCL : /* MiscellaneousCommand */
                    MyCMD._1409_XXXX((PS_MiscellaneousCommand)((uint8*)pParameter), EncodedMsg, EncodedMsgSize) ;
                    break ;
                case    EVENT_PTV_CMD_CNF : /* ConferenceCommand Primitive */
                    MyCMD._140A_XXXX((PS_ConferenceCommand)((uint8*)pParameter)) ;
                    break ;
                case    EVENT_MSG_CMD_CNF : /* ConferenceCommand */
                    MyCMD._140B_XXXX((PS_ConferenceCommand)((uint8*)pParameter)) ;
                    break ;
                case    EVENT_PTV_CMD_HMR : /* H223MultiplexReconfiguration Primitive */
                    MyCMD._140C_XXXX((PS_H223MultiplexReconfiguration)((uint8*)pParameter)) ;
                    break ;
                case    EVENT_MSG_CMD_HMR : /* H223MultiplexReconfiguration */
                    MyCMD._140D_XXXX((PS_H223MultiplexReconfiguration)((uint8*)pParameter), EncodedMsg, EncodedMsgSize) ;
                    break ;

            }
            break ;
        case    EVENT_IDC : /* Indication */
            switch (Event_No)  /* Event No Check */
            {
                case    EVENT_PTV_IDC_FNU : /* FunctionNotUnderstood Primitive */
                    MyIDC._1500_XXXX((PS_FunctionNotUnderstood)((uint8*)pParameter)) ;
                    break ;
                case    EVENT_MSG_IDC_FNU : /* FunctionNotUnderstood */
                    MyIDC._1501_XXXX((PS_FunctionNotUnderstood)((uint8*)pParameter)) ;
                    break ;
                case    EVENT_PTV_IDC_MSCL : /* MiscellaneousIndication Primitive */
                    MyIDC._1502_XXXX((PS_MiscellaneousIndication)((uint8*)pParameter)) ;
                    break ;
                case    EVENT_MSG_IDC_MSCL : /* MiscellaneousIndication */
                    MyIDC._1503_XXXX((PS_MiscellaneousIndication)((uint8*)pParameter)) ;
                    break ;
                case    EVENT_PTV_IDC_JTR : /* JitterIndication Primitive */
                    MyIDC._1504_XXXX((PS_JitterIndication)((uint8*)pParameter)) ;
                    break ;
                case    EVENT_MSG_IDC_JTR : /* JitterIndication */
                    MyIDC._1505_XXXX((PS_JitterIndication)((uint8*)pParameter)) ;
                    break ;
                case    EVENT_PTV_IDC_H223SKW : /* H223SkewIndication Primitive */
                    MyIDC._1506_XXXX((PS_H223SkewIndication)((uint8*)pParameter)) ;
                    break ;
                case    EVENT_MSG_IDC_H223SKW : /* H223SkewIndication */
                    MyIDC._1507_XXXX((PS_H223SkewIndication)((uint8*)pParameter)) ;
                    break ;
                case    EVENT_PTV_IDC_NWATMVC : /* NewATMVCIndication Primitive */
                    MyIDC._1508_XXXX((PS_NewATMVCIndication)((uint8*)pParameter)) ;
                    break ;
                case    EVENT_MSG_IDC_NWATMVC : /* NewATMVCIndication */
                    MyIDC._1509_XXXX((PS_NewATMVCIndication)((uint8*)pParameter)) ;
                    break ;
                case    EVENT_PTV_IDC_UI : /* UserInputIndication Primitive */
                    MyIDC._150A_XXXX((PS_UserInputIndication)((uint8*)pParameter)) ;
                    break ;
                case    EVENT_MSG_IDC_UI : /* UserInputIndication */
                    MyIDC._150B_XXXX((PS_UserInputIndication)((uint8*)pParameter)) ;
                    break ;
                case    EVENT_PTV_IDC_CNF : /* ConferenceIndication Primitive */
                    MyIDC._150C_XXXX((PS_ConferenceIndication)((uint8*)pParameter)) ;
                    break ;
                case    EVENT_MSG_IDC_CNF : /* ConferenceIndication */
                    MyIDC._150D_XXXX((PS_ConferenceIndication)((uint8*)pParameter)) ;
                    break ;
                case    EVENT_PTV_IDC_H2250MXSKW : /* H2250MaximumSkewIndication Primitive */
                    MyIDC._150E_XXXX((PS_H2250MaximumSkewIndication)((uint8*)pParameter)) ;
                    break ;
                case    EVENT_MSG_IDC_H2250MXSKW : /* H2250MaximumSkewIndication */
                    MyIDC._150F_XXXX((PS_H2250MaximumSkewIndication)((uint8*)pParameter)) ;
                    break ;
                case    EVENT_PTV_IDC_MCLCT : /* MCLocationIndication Primitive */
                    MyIDC._1510_XXXX((PS_MCLocationIndication)((uint8*)pParameter)) ;
                    break ;
                case    EVENT_MSG_IDC_MCLCT : /* MCLocationIndication */
                    MyIDC._1511_XXXX((PS_MCLocationIndication)((uint8*)pParameter)) ;
                    break ;
                case    EVENT_PTV_IDC_VI : /* VendorIdentification Primitive */
                    MyIDC._1512_XXXX((PS_VendorIdentification)((uint8*)pParameter)) ;
                    break ;
                case    EVENT_MSG_IDC_VI : /* VendorIdentification */
                    MyIDC._1513_XXXX((PS_VendorIdentification)((uint8*)pParameter)) ;
                    break ;
                case    EVENT_PTV_IDC_FNS : /* FunctionNotSupported Primitive */
                    MyIDC._1514_XXXX((PS_FunctionNotSupported)((uint8*)pParameter)) ;
                    break ;
                case    EVENT_MSG_IDC_FNS : /* FunctionNotSupported */
                    MyIDC._1515_XXXX((PS_FunctionNotSupported)((uint8*)pParameter)) ;
                    break ;
                case    EVENT_MSG_IDC_FC : /* FlowControlIndication  */
                    MyIDC._1516_XXXX((PS_FlowControlIndication)((uint8*)pParameter)) ;
                    break ;
                case    EVENT_PTV_IDC_FC : /* FlowControlIndication Primitive */
                    MyIDC._1517_XXXX((PS_FlowControlIndication)((uint8*)pParameter)) ;
                    break ;
            }
            break ;
    }

    return ;
}

/************************************************************************/
/*  function name       : ItnErrNotificationSend                     */
/*  function outline    : Internal Error Notification                   */
/*                      :                    to Terminal Status Control */
/*  function discription: ItnErrNotificationSend( uint32 , uint32 )      */
/*  input data          : uint32 Error_Inf_1                              */
/*                      : uint32 Error_Inf_2                              */
/*  output data         : None                                          */
/*  draw time           : '96.11.15                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void SE::ItnErrNotificationSend(uint32 Error_Inf_1 , uint32 Error_Inf_2)
{
    S_InfHeader    infHeader ;

    infHeader.pParameter = NULL;

    infHeader.InfType = H245_ERROR ;
    infHeader.InfId = H245_INTERNAL_ERROR_SE ;
    infHeader.InfSupplement1 = Error_Inf_1 ;
    infHeader.InfSupplement2 = Error_Inf_2 ;

    Print("Sending ItnErrNotification Msg\n");

    if (MyH245)
    {
        MyH245->ObserverMessage((unsigned char*)&infHeader, sizeof(S_InfHeader));
    }
}

void SE::Print(const char *format, ...)
{
    OSCL_UNUSED_ARG(format);

    PVLOGGER_LOG_USE_ONLY(va_list args);
    PVLOGGER_LOG_USE_ONLY(va_start(args, format));
    PVLOGGER_LOGMSG_V(PVLOGMSG_INST_LLDBG, Logger, PVLOGMSG_DEBUG, (0, format, args));
    PVLOGGER_LOG_USE_ONLY(va_end(args));
}
