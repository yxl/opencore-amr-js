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
/*****************************************************************************/
/*  file name            : H245Pri.h                                         */
/*  file contents        : Primitive Infomation Header                       */
/*  draw                 : '96.11.28                                         */
/*---------------------------------------------------------------------------*/
/*  amendment                                                                */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
#ifndef _H245PRI_
#define _H245PRI_

/************************************************************************/
/*  Headerfile Include                                                  */
/************************************************************************/
#include    "oscl_base.h"
#include	"oscl_mem.h"
#include    "h245def.h"


/************************************************************************/
/*  Common Value Definition: Error Code Value                           */
/************************************************************************/
typedef enum _enum_errcode
{
    ErrCode_Msd_A = 0,  /* ( 0) no response from remote MSDSE ( Local Timer T106 Expiry ) */
    ErrCode_Msd_B ,     /* ( 1) remote sees no response from local MSDSE ( Remort Timer T106 Expiry ) */
    ErrCode_Msd_C ,     /* ( 2) inappropriate message ( MasterSlaveDetermination ) */
    ErrCode_Msd_D ,     /* ( 3) inappropriate message ( MasterSlaveDeterminationReject ) */
    ErrCode_Msd_E ,     /* ( 4) inconsistent field value ( MasterSlaveDeterminationAck.decision != sv_STATUS ) */
    ErrCode_Msd_F ,     /* ( 5) maximum number of retries ( sv_NCOUNT == N100 ) */

    /* Outgoing LCSE */

    ErrCode_Lc_A ,      /* ( 6) inappropriate message ( OpenLogicalChannelAck ) */
    ErrCode_Lc_B ,      /* ( 7) inappropriate message ( OpenLogicalChannelReject ) */
    ErrCode_Lc_C ,      /* ( 8) inappropriate message ( CloseLogicalChannelAck ) */
    ErrCode_Lc_D ,      /* ( 9) no response from Peer LCSE / B-LCSE ( Timer T103 Expiry ) */

    /* Outgoing B-LCSE */

    ErrCode_Blc_A ,     /* ( 6) inappropriate message ( OpenLogicalChannelAck ) */
    ErrCode_Blc_B ,     /* ( 7) inappropriate message ( OpenLogicalChannelReject ) */
    ErrCode_Blc_C ,     /* ( 8) inappropriate message ( CloseLogicalChannelAck ) */
    ErrCode_Blc_D ,     /* ( 9) no response from Peer LCSE / B-LCSE ( Timer T103 Expiry ) */

    /* Incoming B-LCSE */

    ErrCode_Blc_E ,     /* (10) inappropriate message ( OpenLogicalChannelConfirm ) */
    ErrCode_Blc_F ,     /* (11) no response from Peer B-LCSE ( Timer T103 Expiry ) */

    /* Outgoing Mlse */

    ErrCode_Ml_A ,      /* (12) inappropriate message ( MaintenanceLoopAck ) */
    ErrCode_Ml_B        /* (13) no response from Peer MLSE ( Timer T102 Expiry ) */
} ENUM_ErrCode ;


/************************************************************************/
/*  Common Parameter Definition: Error Code Parameter                   */
/************************************************************************/
/* ERROR.indication */
typedef struct _errcode
{
    ENUM_ErrCode    ErrCode ;   /* ERRORCODE Parameter */
} S_ErrCode ;
typedef S_ErrCode   *PS_ErrCode ;


/************************************************************************/
/*  Common Value Definition: Source Value                               */
/************************************************************************/
typedef enum _esource
{
    Src_USER = 0,       /* ( 0) User        CESE LCSE B-LCSE CLCSE MTSE RMESE MRSE MLSE */
    Src_PROTOCOL ,      /* ( 1) Protocol    CESE             CLCSE MTSE RMESE MRSE      */
    Src_LCSE ,          /* ( 2) Lcse             LCSE                                   */
    Src_BLCSE ,         /* ( 3) Blcse                 B-LCSE                            */
    Src_MLSE            /* ( 4) Mlse                                               MLSE */
} ENUM_Source ;

// ==========================================================
// NOTE: The following "DoCoMo Structures" are used for
//   communication between TSC and SE.  We eliminated many
//   of the original ones when we integrated the new H.245
//   PER codec, replacing them with the actual H.245 message
//   definitions.  Here are the ones that we found advantageous
//   to keep.  (RAN)
// ==========================================================

/************************************************************************/
/*  Parameter Definition: H223 Multiplex Table                          */
/************************************************************************/
/* TRANSFER.request */
/* TRANSFER.indication */
typedef struct _muxdescriptor
{
    int32                         size_of_multiplexEntryDescriptors ;
    /* MUX_DESCRIPTOR Parameter */
    S_MultiplexEntryDescriptor  *multiplexEntryDescriptors ;
} S_MuxDescriptor ;
typedef S_MuxDescriptor     *PS_MuxDescriptor ;

/* REJECT.indication */
typedef struct _sourcecause_mt
{
    ENUM_Source                 Source ;                        /* SOURCE Parameter */
    S_MeRejectCause             Cause ;                         /* CAUSE Parameter */
} S_SourceCause_Mt ;
typedef S_SourceCause_Mt    *PS_SourceCause_Mt ;

/************************************************************************/
/*  Parameter Definition: Capability Exchange                           */
/************************************************************************/
/* REJECT.indication */
typedef struct _sourcecause_ce
{
    ENUM_Source             Source ;                            /* SOURCE Parameter */
    S_TcsRejectCause        Cause ;                             /* CAUSE Parameter */
} S_SourceCause_Ce ;
typedef S_SourceCause_Ce        *PS_SourceCause_Ce ;

/************************************************************************/
/*  Parameter Definition: Uni-directional Logical Channel               */
/*                      : Bi-directional Logical Channel                */
/************************************************************************/
/* ESTABLISH.request */
/* ESTABLISH.indication */
typedef struct _forwardreverseparam
{
    S_ForwardLogicalChannelParameters forwardLogicalChannelParameters ;
    int32 option_of_reverseLogicalChannelParameters ;  /* (Not present for unidirectional) */
    S_ReverseLogicalChannelParameters reverseLogicalChannelParameters ;
} S_ForwardReverseParam ;
typedef S_ForwardReverseParam *PS_ForwardReverseParam ;

/* ESTABLISH.response */
/* ESTABLISH.confirm */
typedef struct _reversedata
{
    int32 option_of_reverseLogicalChannelParameters ; /* (Not present for unidirectional) */
    S_AckReverseLogicalChannelParameters reverseLogicalChannelParameters ;
    // FwdMuxAckParameters added to 245 library on 9/28/00 (RAN)
    int32 option_of_forwardMultiplexAckParameters ;
    S_ForwardMultiplexAckParameters forwardMultiplexAckParameters;
} S_ReverseData ;
typedef S_ReverseData           *PS_ReverseData ;

/* RELEASE.indication */
typedef struct _sourcecause_lcblc
{
    ENUM_Source                                 Source ;        /* SOURCE Parameter */
    S_OlcRejectCause                            Cause ;         /* CAUSE Parameter */
} S_SourceCause_LcBlc ;
typedef S_SourceCause_LcBlc     *PS_SourceCause_LcBlc ;

/************************************************************************/
/*  Parameter Definition: Close Logical Channel                         */
/************************************************************************/
/* REJECT.indication */
typedef struct _sourcecause_clc
{
    ENUM_Source     Source ;   /* SOURCE Parameter */
    S_RccRejectCause   Cause ; /* CAUSE Parameter */
} S_SourceCause_Clc ;
typedef S_SourceCause_Clc   *PS_SourceCause_Clc ;

/************************************************************************/
/*  Parameter Definition: Round Trip Delay                              */
/************************************************************************/
/* TRANSFER.confirm */
typedef struct _delay_rtd
{
    int32     Delay_Value ;
} S_Delay_Rtd ;
typedef S_Delay_Rtd     *PS_Delay_Rtd ;

/************************************************************************/
/*  Parameter Definition: Maintenance Loop                              */
/************************************************************************/
/* RELEASE.indication */
/* Structure Salvaged (RAN) */
typedef struct _sourcecause_ml
{
    ENUM_Source     Source ;                                    /* SOURCE Parameter */
    S_MlRejectCause    Cause ;                                     /* CAUSE Parameter */
} S_SourceCause_Ml ;
typedef S_SourceCause_Ml    *PS_SourceCause_Ml ;

/************************************************************************/
/*  Parameter Definition: Mode Request                                  */
/************************************************************************/
/* REJECT.indication */
typedef struct _sourcecause_mr
{
    ENUM_Source         Source ;                                /* SOURCE Parameter */
    S_RmRejectCause     Cause ;                                 /* CAUSE Parameter */
} S_SourceCause_Mr ;
typedef S_SourceCause_Mr    *PS_SourceCause_Mr ;

/************************************************************************/
/*  Parameter Definition: Request Multiplex Entry                       */
/************************************************************************/
/* REJECT.indication */
typedef struct _sourcecause_rme
{
    ENUM_Source        Source ;   /* SOURCE Parameter */
    S_RmeRejectCause   Cause ;    /* CAUSE Parameter */
} S_SourceCause_Rme ;
typedef S_SourceCause_Rme   *PS_SourceCause_Rme ;


#endif /* _H245PRI_ */
