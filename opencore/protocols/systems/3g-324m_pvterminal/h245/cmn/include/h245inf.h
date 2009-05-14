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
/*  file name       : H245Inf.h                                            */
/*  file contents   : H.245 Interface Information Definition                  */
/*  draw            : '96.10.04                                         */
/*----------------------------------------------------------------------*/
/*  amendment       :                                                   */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
#ifndef _H245INF_
#define _H245INF_


/************************************************************************/
/*  Headerfile Include                                                  */
/************************************************************************/
#include "control_msg_hdr.h"

/************************************************************************/
/*  Interface Information Type Definition                               */
/************************************************************************/
#define     H245_USER           0       /* H.245 User Information */
#define     H245_PRIMITIVE      1       /* H.245 Primitive Information */
#define     H245_ERROR          2       /* H.245 Error Information */
#define     H245_TIMEOUT        3       /* H.245 Timeout Information */


/************************************************************************/
/*  Interface Information Id Definition: User Operation Id              */
/************************************************************************/
typedef enum _userreportid
{
//    E_User_Ce_Req_Idc ,             /* ( 0) Capability exchange Request Indication        */
//    E_User_Ce_Rjt_Idc ,             /* ( 1) Capability exchange Reject Indication         */
//    E_User_Msd_Rjt_Idc ,            /* ( 2) Master slave determination Reject Indication  */
//    E_User_Msd_Err_Idc ,            /* ( 3) Master slave determination Error Indication   */
    E_User_Lc_Req_Idc = 4 ,           /* ( 4) Uni-directional Logical Channel signalling Request Indication */
    E_User_Lc_Rls_Idc ,             /* ( 5) Uni-directional Logical Channel signalling Release Indication */
    E_User_Lc_Err_Idc ,             /* ( 6) Uni-directional Logical Channel signalling Error Indication   */
    E_User_Lc_RlsCfm_Idc ,          /* ( 7) Uni-directional Logical Channel signalling RlsCfm Indication  */
    E_User_Blc_Req_Idc ,            /* ( 8) Bi-directional Logical Channel signalling Request Indication */
    E_User_Blc_Rls_Idc ,            /* ( 9) Bi-directional Logical Channel signalling Release Indication */
    E_User_Blc_Err_Idc ,            /* (10) Bi-directional Logical Channel signalling Error Indication   */
    E_User_Blc_RlsCfm_Idc ,         /* (11) Bi-directional Logical Channel signalling RlsCfm Indication  */
    E_User_Clc_Req_Idc ,            /* (12) Close Logical Channel Request Indication      */
    E_User_Clc_Rjt_Idc ,            /* (13) Close Logical Channel Reject Indication       */
    E_User_Mt_Req_Idc ,             /* (14) H.223 Multiplex Table Request Indication      */
    E_User_Mt_Rjt_Idc ,             /* (15) H.223 Multiplex Table Reject Indication       */
    E_User_Rme_Req_Idc ,            /* (16) Request Multiplex Entry Request Indication */
    E_User_Rme_Rjt_Idc ,            /* (17) Request Multiplex Entry Reject Indication  */
    E_User_Mr_Req_Idc ,             /* (18) Mode request Request Indication */
    E_User_Mr_Rjt_Idc ,             /* (19) Mode request Reject Indication  */
    E_User_Rtd_Rjt_Idc ,            /* (20) Round trip delay Reject Indication */
    E_User_Ml_Req_Idc ,             /* (21) Maintenance Loop Request Indication */
    E_User_Ml_Rjt_Idc ,             /* (22) Maintenance Loop Reject Indication  */
    E_User_Ml_Err_Idc ,             /* (23) Maintenance Loop Error Indication  */
    E_User_CSUP_Done				/* (24) Call Setup Done Signal (WWU)		*/
} ENUM_UserReportId ;


typedef enum _useroperationid
{
//    E_User_Ce_Instruct ,            /* ( 0) Capability exchange     */
    E_User_Mt_Instruct = 1 ,          /* ( 1) H.223 Multiplex Table   */
    E_User_Rme_Instruct ,           /* ( 2) Request Multiplex Entry */
    E_User_Mr_Instruct ,            /* ( 3) Mode Request            */
    E_User_Rtd_Instruct ,           /* ( 4) Round trip delay        */
    E_User_Ml_Instruct ,            /* ( 5) Maintenance Loop        */

    E_User_Connect ,                /* ( 6) Connect to Remote       */
    E_User_Open ,                   /* ( 7) Session Open            */
    E_User_Consent ,                /* ( 8) Concent                 */
    E_User_Reject ,                 /* ( 9) Reject                  */
    E_User_Retry ,                  /* (10) Retry                   */
    E_User_Close ,                  /* (11) Session Close           */
    E_User_Disconnect ,             /* (12) Disconnect              */

    E_User_NonProcedure,            /* (13) Non Procedure           */
    E_User_TestButton,              /* (14) Test Button (RAN TB)    */

    E_User_Stcs_Instruct,           /* (15) Send Terminal Cap Set  (RAN) */
    E_User_Hmr_Instruct,            /* (16) H223Multiplex Reconfig (RAN) */
    E_User_Rcc_Instruct,            /* (17) RequestChannelClose    (RAN) */
    E_User_Clc_Instruct             /* (18) CloseLogicalChannel    (RAN) */
} ENUM_UserOperationId ;


/************************************************************************/
/*  Interface Information Id Definition: Primitive Id                   */
/************************************************************************/
typedef enum _primitiveid
{
    /* Request primitive */

//    E_PtvId_Msd_Dtm_Req ,           /* (  0) DETERMINE.request ( MSD ) */
//    E_PtvId_Ce_Trf_Req ,            /* (  1) TRANSFER.request ( CE ) */
//    E_PtvId_Ce_Rjt_Req ,            /* (  2) REJECT.request ( CE ) */
    E_PtvId_Lc_Etb_Req = 3 ,          /* (  3) ESTABLISH.request ( LC ) */
    E_PtvId_Lc_Rls_Req ,            /* (  4) RELEASE.request ( LC ) */
    E_PtvId_Blc_Etb_Req ,           /* (  5) ESTABLISH.request ( B-LC ) */
    E_PtvId_Blc_Rls_Req ,           /* (  6) RELEASE.request ( B-LC ) */
    E_PtvId_Clc_Cls_Req ,           /* (  7) CLOSE.request ( CLC ) */
    E_PtvId_Clc_Rjt_Req ,           /* (  8) REJECT.request ( CLC ) */
    E_PtvId_Mt_Trf_Req ,            /* (  9) TRANSFER.request ( MT ) */
    E_PtvId_Mt_Rjt_Req ,            /* ( 10) REJECT.request ( MT ) */
    E_PtvId_Rme_Send_Req ,          /* ( 11) SEND.request ( RME ) */
    E_PtvId_Rme_Rjt_Req ,           /* ( 12) REJECT.request ( RME ) */
    E_PtvId_Mr_Trf_Req ,            /* ( 13) TRANSFER.request ( MR ) */
    E_PtvId_Mr_Rjt_Req ,            /* ( 14) REJECT.request ( MR ) */
    E_PtvId_Rtd_Trf_Req ,           /* ( 15) TRANSFER.request ( RTD ) */
    E_PtvId_Ml_Loop_Req ,           /* ( 16) LOOP.request ( ML ) */
    E_PtvId_Ml_Rls_Req ,            /* ( 17) RELEASE.request ( ML ) */

    /* Indication primitive */

//    E_PtvId_Msd_Dtm_Idc ,           /* ( 18) DETERMINE.indication ( MSD ) */
//    E_PtvId_Msd_Rjt_Idc ,           /* ( 19) REJECT.indication ( MSD ) */
//    E_PtvId_Msd_Err_Idc ,           /* ( 20) ERROR.indication ( MSD ) */
//    E_PtvId_Ce_Trf_Idc ,            /* ( 21) TRANSFER.indication ( CE ) */
//    E_PtvId_Ce_Rjt_Idc ,            /* ( 22) REJECT.indication ( CE ) */
    E_PtvId_Lc_Etb_Idc = 23 ,          /* ( 23) ESTABLISH.indication ( LC ) */
    E_PtvId_Lc_Rls_Idc ,            /* ( 24) RELEASE.indication ( LC ) */
    E_PtvId_Lc_Err_Idc ,            /* ( 25) ERROR.indication ( LC ) */
    E_PtvId_Blc_Etb_Idc ,           /* ( 26) ESTABLISH.indication ( B-LC ) */
    E_PtvId_Blc_Rls_Idc ,           /* ( 27) RELEASE.indication ( B-LC ) */
    E_PtvId_Blc_Err_Idc ,           /* ( 28) ERROR.indication ( B-LC ) */
    E_PtvId_Clc_Cls_Idc ,           /* ( 29) CLOSE.indication ( CLC ) */
    E_PtvId_Clc_Rjt_Idc ,           /* ( 30) REJECT.indication ( CLC ) */
    E_PtvId_Mt_Trf_Idc ,            /* ( 31) TRANSFER.indication ( MT ) */
    E_PtvId_Mt_Rjt_Idc ,            /* ( 32) REJECT.indication ( MT ) */
    E_PtvId_Rme_Send_Idc ,          /* ( 33) SEND.indication ( RME ) */
    E_PtvId_Rme_Rjt_Idc ,           /* ( 34) REJECT.indication ( RME ) */
    E_PtvId_Mr_Trf_Idc ,            /* ( 35) TRANSFER.indication ( MR ) */
    E_PtvId_Mr_Rjt_Idc ,            /* ( 36) REJECT.indication ( MR ) */
    E_PtvId_Rtd_Exp_Idc ,           /* ( 37) EXPIRY.indication ( RTD ) */
    E_PtvId_Ml_Loop_Idc ,           /* ( 38) LOOP.indication ( ML ) */
    E_PtvId_Ml_Rls_Idc ,            /* ( 39) RELEASE.indication ( ML ) */
    E_PtvId_Ml_Err_Idc ,            /* ( 40) ERROR.indication ( ML ) */

    /* Response primitive */

    E_PtvId_Ce_Trf_Rps ,            /* ( 41) TRANSFER.response ( CE ) */
    E_PtvId_Lc_Etb_Rps ,            /* ( 42) ESTABLISH.response ( LC ) */
    E_PtvId_Blc_Etb_Rps ,           /* ( 43) ESTABLISH.response ( B-LC ) */
    E_PtvId_Clc_Cls_Rps ,           /* ( 44) CLOSE.response ( CLC ) */
    E_PtvId_Mt_Trf_Rps ,            /* ( 45) TRANSFER.response ( MT ) */
    E_PtvId_Rme_Send_Rps ,          /* ( 46) SEND.response ( RME ) */
    E_PtvId_Mr_Trf_Rps ,            /* ( 47) TRANSFER.response ( MR ) */
    E_PtvId_Ml_Loop_Rps ,           /* ( 48) LOOP.response ( ML ) */

    /* Confirm primitive */

    E_PtvId_Msd_Dtm_Cfm ,           /* ( 49) DETERMINE.confirm ( MSD ) */
    E_PtvId_Ce_Trf_Cfm ,            /* ( 50) TRANSFER.confirm ( CE ) */
    E_PtvId_Lc_Etb_Cfm ,            /* ( 51) ESTABLISH.confirm ( LC ) */
    E_PtvId_Lc_Rls_Cfm ,            /* ( 52) RELEASE.confirm ( LC ) */
    E_PtvId_Blc_Etb_Cfm ,           /* ( 53) ESTABLISH.confirm ( BLC ) */
    E_PtvId_Blc_Etb_Cfm2,			/* WWU_BLC */
    E_PtvId_Blc_Rls_Cfm ,           /* ( 54) RELEASE.confirm ( BLC ) */
    E_PtvId_Clc_Cls_Cfm ,           /* ( 55) CLOSE.confirm ( CLC ) */
    E_PtvId_Mt_Trf_Cfm ,            /* ( 56) TRANSFER.confirm ( MT ) */
    E_PtvId_Rme_Send_Cfm ,          /* ( 57) SEND.confirm ( RME ) */
    E_PtvId_Mr_Trf_Cfm ,            /* ( 58) TRANSFER.confirm ( MR ) */
    E_PtvId_Rtd_Trf_Cfm ,           /* ( 59) TRANSFER.confirm ( RTD ) */
    E_PtvId_Ml_Loop_Cfm ,           /* ( 60) LOOP.confirm ( ML ) */

    /* Other primitive */
    E_PtvId_Nsd_Req ,               /* ( 61) NONSTANDARDMESSAGE_REQUEST ( NSD ) */
    E_PtvId_Nsd_Req_Cfm ,           /* ( 62) NONSTANDARDMESSAGE_REQUEST.confirm ( NSD ) */
    E_PtvId_Nsd_Rps ,               /* ( 63) NONSTANDARDMESSAGE_RESPONSE ( NSD ) */
    E_PtvId_Nsd_Rps_Cfm ,           /* ( 64) NONSTANDARDMESSAGE_RESPONSE.confirm ( NSD ) */
    E_PtvId_Nsd_Cmd ,               /* ( 65) NONSTANDARDMESSAGE_COMMAND ( NSD ) */
    E_PtvId_Nsd_Cmd_Cfm ,           /* ( 66) NONSTANDARDMESSAGE_COMMAND.confirm ( NSD ) */
    E_PtvId_Nsd_Idc ,               /* ( 67) NONSTANDARDMESSAGE_INDICATION ( NSD ) */
    E_PtvId_Nsd_Idc_Cfm ,           /* ( 68) NONSTANDARDMESSAGE_INDICATION.confirm ( NSD ) */

    E_PtvId_Cm_Req ,                /* ( 69) COMMUNICATIONMODE_REQUEST ( CM ) */
    E_PtvId_Cm_Req_Cfm ,            /* ( 70) COMMUNICATIONMODE_REQUEST.confirm ( CM ) */
    E_PtvId_Cm_Rps ,                /* ( 71) COMMUNICATIONMODE_RESPONSE ( CM ) */
    E_PtvId_Cm_Rps_Cfm ,            /* ( 72) COMMUNICATIONMODE_RESPONSE.confirm ( CM ) */
    E_PtvId_Cm_Cmd ,                /* ( 73) COMMUNICATIONMODE_COMMAND ( CM ) */
    E_PtvId_Cm_Cmd_Cfm ,            /* ( 74) COMMUNICATIONMODE_COMMAND.confirm ( CM ) */

    E_PtvId_H223aar ,               /* ( 75) H223ANNEXARECONFIGURATION ( H223AAR ) */
    E_PtvId_H223aar_Cfm ,           /* ( 76) H223ANNEXARECONFIGURATION.confirm ( H223AAR ) */
    E_PtvId_H223aar_Ack ,           /* ( 77) H223ANNEXARECONFIGURATION_ACK ( H223AAR ) */
    E_PtvId_H223aar_Ack_Cfm ,       /* ( 78) H223ANNEXARECONFIGURATION_ACK.confirm ( H223AAR ) */
    E_PtvId_H223aar_Rjt ,           /* ( 79) H223ANNEXARECONFIGURATION_REJECT ( H223AAR ) */
    E_PtvId_H223aar_Rjt_Cfm ,       /* ( 80) H223ANNEXARECONFIGURATION_REJECT.confirm ( H223AAR ) */

    E_PtvId_Cnf_Req ,               /* ( 81) CONFERENCE_REQUEST ( CNF ) */
    E_PtvId_Cnf_Req_Cfm ,           /* ( 82) CONFERENCE_REQUEST.confirm ( CNF ) */
    E_PtvId_Cnf_Rps ,               /* ( 83) CONFERENCE_RESPONSE ( CNF ) */
    E_PtvId_Cnf_Rps_Cfm ,           /* ( 84) CONFERENCE_RESPONSE.confirm ( CNF ) */

    E_PtvId_Cmd_Stcs ,              /* ( 85) SENDTERMINALCAPABILITYSET ( CMD ) */
    E_PtvId_Cmd_Stcs_Cfm ,          /* ( 86) SENDTERMINALCAPABILITYSET.confirm ( CMD ) */
    E_PtvId_Cmd_Ecrpt ,             /* ( 87) ENCRYPTIONCOMMAND ( CMD ) */
    E_PtvId_Cmd_Ecrpt_Cfm ,         /* ( 88) ENCRYPTIONCOMMAND.confirm ( CMD ) */
    E_PtvId_Cmd_Fc ,                /* ( 89) FLOWCONTROLCOMMAND ( CMD ) */
    E_PtvId_Cmd_Fc_Cfm ,            /* ( 90) FLOWCONTROLCOMMAND.confirm ( CMD ) */
    E_PtvId_Cmd_Es ,                /* ( 91) ENDSESSIONCOMMAND ( CMD ) */
    E_PtvId_Cmd_Es_Cfm ,            /* ( 92) ENDSESSIONCOMMAND.confirm ( CMD ) */
    E_PtvId_Cmd_Mscl ,              /* ( 93) MISCELLANEOUSCOMMAND ( CMD ) */
    E_PtvId_Cmd_Mscl_Cfm ,          /* ( 94) MISCELLANEOUSCOMMAND.confirm ( CMD ) */
    E_PtvId_Cmd_Cnf ,               /* ( 95) CONFERENCECOMMAND ( CMD ) */
    E_PtvId_Cmd_Cnf_Cfm ,           /* ( 96) CONFERENCECOMMAND.confirm ( CMD ) */

    E_PtvId_Idc_Fnu ,               /* ( 97) FUNCTIONNOTUNDERSTOOD ( IDC ) */
    E_PtvId_Idc_Fnu_Cfm ,           /* ( 98) FUNCTIONNOTUNDERSTOOD.confirm ( IDC ) */
    E_PtvId_Idc_Mscl ,              /* ( 99) MISCELLANEOUSINDICATION ( IDC ) */
    E_PtvId_Idc_Mscl_Cfm ,          /* (100) MISCELLANEOUSINDICATION.confirm ( IDC ) */
    E_PtvId_Idc_Jtr ,               /* (101) JITTERINDICATION ( IDC ) */
    E_PtvId_Idc_Jtr_Cfm ,           /* (102) JITTERINDICATION.confirm ( IDC ) */
    E_PtvId_Idc_H223skw ,           /* (103) H223SKEWINDICATION ( IDC ) */
    E_PtvId_Idc_H223skw_Cfm ,       /* (104) H223SKEWINDICATION.confirm ( IDC ) */
    E_PtvId_Idc_Nwatmvc ,           /* (105) NEWATMVCINDICATION ( IDC ) */
    E_PtvId_Idc_Nwatmvc_Cfm ,       /* (106) NEWATMVCINDICATION.confirm ( IDC ) */
    E_PtvId_Idc_Ui ,                /* (107) USERINPUTINDICATION ( IDC ) */
    E_PtvId_Idc_Ui_Cfm ,            /* (108) USERINPUTINDICATION.confirm ( IDC ) */
    E_PtvId_Idc_Cnf ,               /* (109) CONFERENCEINDICATION ( IDC ) */
    E_PtvId_Idc_Cnf_Cfm ,           /* (110) CONFERENCEINDICATION.confirm ( IDC ) */
    E_PtvId_Idc_H2250mxskw ,        /* (111) H2250MAXIMUMSKEWINDICATION ( IDC ) */
    E_PtvId_Idc_H2250mxskw_Cfm ,    /* (112) H2250MAXIMUMSKEWINDICATION.confirm ( IDC ) */
    E_PtvId_Idc_Mclct ,             /* (113) MCLOCATIONINDICATION ( IDC ) */
    E_PtvId_Idc_Mclct_Cfm ,         /* (114) MCLOCATIONINDICATION.confirm ( IDC ) */
    E_PtvId_Idc_Vi ,                /* (115) VENDORIDENTIFICATION ( IDC ) */
    E_PtvId_Idc_Vi_Cfm ,            /* (116) VENDORIDENTIFICATION.confirm ( IDC ) */
    E_PtvId_Idc_Fns ,               /* (117) FUNCTIONNOTSUPPORTED ( IDC ) */
    E_PtvId_Idc_Fns_Cfm,            /* (118) FUNCTIONNOTSUPPORTED.confirm ( IDC ) */
    E_PtvId_Idc_Fc,                 /* (119) FLOWCONTROL( IDC ) */
    E_PtvId_Idc_Fc_Cfm,            /* (120) FLOWCONTROL.confirm ( IDC ) */

    /* Additional events (RAN) */
    /* (NOTE: above numbers are off by 1) */
    E_PtvId_Cmd_Hmr ,              /* (120) H223MultiplexReconfiguration ( CMD ) */
    E_PtvId_Cmd_Hmr_Cfm            /* (121) H223MultiplexReconfiguration.confirm ( CMD ) */
} ENUM_PrimitiveId ;

/************************************************************************/
/*  Interface Information Id Definition:                                */
/*                                     Internal Error Occurence Routine */
/************************************************************************/
#define     H245_INTERNAL_ERROR_TSC         0       /* Occur in Terminal Status Control Routine */
#define     H245_INTERNAL_ERROR_SE          1       /* Occur in Control Entity Routine ( Signalling Entity ) */
#define     H245_INTERNAL_ERROR_SRP         2       /* Occur in Simple Retransmission Protocol Routine */


/************************************************************************/
/*  Interface Information Id Definition: Timer Id                       */
/************************************************************************/
typedef enum _timerid
{
    E_TmrId_Msd_T106 ,              /* ( 0) Master Slave Determination */
    E_TmrId_Ce_T101 ,               /* ( 1) Capability Exchange */
    E_TmrId_LcBlc_T103 ,            /* ( 2) Uni-Directional and Bi-Directional Logical Channel Signalling */
    E_TmrId_Clc_T108 ,              /* ( 3) Close Logical Channel */
    E_TmrId_Mt_T104 ,               /* ( 4) Multiplex Table */
    E_TmrId_Rme_T107 ,              /* ( 5) Request Multiplex Entry */
    E_TmrId_Mr_T109 ,               /* ( 6) Mode Request */
    E_TmrId_Rtd_T105 ,              /* ( 7) Round Trip Delay */
    E_TmrId_Ml_T102                 /* ( 8) Maintenance Loop */
} ENUM_TimerId ;


/************************************************************************/
/*  Interface Information Structure Type Definitions                    */
/************************************************************************/
typedef S_ControlMsgHeader S_InfHeader;
typedef PS_ControlMsgHeader PS_InfHeader;

typedef struct
{
    uint32 iT101; /* Capability Exchange */
    uint32 iT103; /* OLC/OBLC */
    uint32 iT104; /* MT */
    uint32 iT105; /* RTD */
    uint32 iT106; /* MSD */
    uint32 iT107; /* RME */
    uint32 iT108; /* CLC */
    uint32 iT109; /* MR */
}H245TimerValues;

#endif /* _H245INF_ */

