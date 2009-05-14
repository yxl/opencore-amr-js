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
/*  file name            : tsc_eventreceive.c                                */
/*  file contents        : Event Id Receive                                  */
/*  draw                 : '96.10.04                                         */
/*---------------------------------------------------------------------------*/
/*  amendment                                                                */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
#include "oscl_types.h"
#include "oscl_mem.h"
#include "h245inf.h"
#include "tsc_eventreceive.h"

/*---------------------------------------------------------------------------*/
/*  Variable  Definition                                                     */
/*---------------------------------------------------------------------------*/
typedef struct _eventidtable
{
    uint32      EventId;                /* EventId                   */
    uint32      EventInfType;           /* EventType                 */
    uint32      EventInfId;             /* EventId(receive)          */
} S_EventIdTable;

// -------------------------------------------------------------
// New event table for User Events (App<->Tsc).  These are now
//   handled separately from the SE<->Tsc events.  (RAN)
// -------------------------------------------------------------
static  const S_EventIdTable      UserEventIdTable[] =
{
//        {  1,  H245_USER     ,  E_User_Ce_Instruct          },
    {  2,  H245_USER     ,  E_User_Mt_Instruct          },
    {  3,  H245_USER     ,  E_User_Mr_Instruct          },
    {  4,  H245_USER     ,  E_User_Rtd_Instruct         },
    {  5,  H245_USER     ,  E_User_Ml_Instruct          },
    {  6,  H245_USER     ,  E_User_Rme_Instruct         },
    {  7,  H245_USER     ,  E_User_Stcs_Instruct        },
    {  8,  H245_USER     ,  E_User_Hmr_Instruct         },
    {  9,  H245_USER     ,  E_User_Rcc_Instruct         },
    { 10,  H245_USER     ,  E_User_Clc_Instruct         },
    { 44,  H245_USER     ,  E_User_Consent              },
    { 45,  H245_USER     ,  E_User_Reject               },
    { 50,  H245_USER     ,  E_User_Close                },
    { 51,  H245_USER     ,  E_User_NonProcedure         },
    {  0,  0             ,  0                       }
};

// -------------------------------------------------------------
// The original event table for SE<->Tsc events.  The user
//   events have been removed.  (RAN)
// -------------------------------------------------------------
static  const S_EventIdTable      EventIdTable[] =
{
//        {  2,  H245_PRIMITIVE,  E_PtvId_Msd_Dtm_Idc         },
//        {  3,  H245_PRIMITIVE,  E_PtvId_Msd_Dtm_Cfm         },
//        {  4,  H245_PRIMITIVE,  E_PtvId_Msd_Rjt_Idc         },
//        {  5,  H245_PRIMITIVE,  E_PtvId_Msd_Err_Idc         },
//        {  6,  H245_PRIMITIVE,  E_PtvId_Ce_Trf_Idc          },
//        {  7,  H245_PRIMITIVE,  E_PtvId_Ce_Trf_Cfm          },
//        {  8,  H245_PRIMITIVE,  E_PtvId_Ce_Rjt_Idc          },
    {  9,  H245_PRIMITIVE,  E_PtvId_Lc_Etb_Idc          },
    { 10,  H245_PRIMITIVE,  E_PtvId_Lc_Etb_Cfm          },
    { 11,  H245_PRIMITIVE,  E_PtvId_Lc_Rls_Idc          },
    { 12,  H245_PRIMITIVE,  E_PtvId_Lc_Rls_Cfm          },
    { 13,  H245_PRIMITIVE,  E_PtvId_Lc_Err_Idc          },
    { 14,  H245_PRIMITIVE,  E_PtvId_Blc_Etb_Idc         },
    { 15,  H245_PRIMITIVE,  E_PtvId_Blc_Etb_Cfm         },
    { 16,  H245_PRIMITIVE,  E_PtvId_Blc_Rls_Idc         },
    { 17,  H245_PRIMITIVE,  E_PtvId_Blc_Rls_Cfm         },
    { 18,  H245_PRIMITIVE,  E_PtvId_Blc_Err_Idc         },
    { 19,  H245_PRIMITIVE,  E_PtvId_Clc_Cls_Idc         },
    { 20,  H245_PRIMITIVE,  E_PtvId_Clc_Cls_Cfm         },
    { 21,  H245_PRIMITIVE,  E_PtvId_Clc_Rjt_Idc         },
    { 22,  H245_PRIMITIVE,  E_PtvId_Mt_Trf_Idc          },
    { 23,  H245_PRIMITIVE,  E_PtvId_Mt_Trf_Cfm          },
    { 24,  H245_PRIMITIVE,  E_PtvId_Mt_Rjt_Idc          },
    { 25,  H245_PRIMITIVE,  E_PtvId_Rme_Send_Idc        },
    { 26,  H245_PRIMITIVE,  E_PtvId_Rme_Send_Cfm        },
    { 27,  H245_PRIMITIVE,  E_PtvId_Rme_Rjt_Idc         },
    { 28,  H245_PRIMITIVE,  E_PtvId_Mr_Trf_Idc          },
    { 29,  H245_PRIMITIVE,  E_PtvId_Mr_Trf_Cfm          },
    { 30,  H245_PRIMITIVE,  E_PtvId_Mr_Rjt_Idc          },
    { 31,  H245_PRIMITIVE,  E_PtvId_Rtd_Trf_Cfm         },
    { 32,  H245_PRIMITIVE,  E_PtvId_Rtd_Exp_Idc         },
    { 33,  H245_PRIMITIVE,  E_PtvId_Ml_Loop_Idc         },
    { 34,  H245_PRIMITIVE,  E_PtvId_Ml_Loop_Cfm         },
    { 35,  H245_PRIMITIVE,  E_PtvId_Ml_Rls_Idc          },
    { 36,  H245_PRIMITIVE,  E_PtvId_Ml_Err_Idc          },
    { 37,  H245_PRIMITIVE,  E_PtvId_Cmd_Es_Cfm          },
    { 38,  H245_ERROR    ,  H245_INTERNAL_ERROR_TSC     },
    { 38,  H245_ERROR    ,  H245_INTERNAL_ERROR_SE      },
    { 38,  H245_ERROR    ,  H245_INTERNAL_ERROR_SRP     },
    // NOTE: Tsc currently has no handling for Event38.

    { 49,  H245_PRIMITIVE,  E_PtvId_Nsd_Req_Cfm         },
    { 49,  H245_PRIMITIVE,  E_PtvId_Nsd_Rps_Cfm         },
    { 49,  H245_PRIMITIVE,  E_PtvId_Nsd_Cmd_Cfm         },
    { 49,  H245_PRIMITIVE,  E_PtvId_Nsd_Idc_Cfm         },
    { 49,  H245_PRIMITIVE,  E_PtvId_Cm_Req_Cfm          },
    { 49,  H245_PRIMITIVE,  E_PtvId_Cm_Rps_Cfm          },
    { 49,  H245_PRIMITIVE,  E_PtvId_Cm_Cmd_Cfm          },
    { 49,  H245_PRIMITIVE,  E_PtvId_H223aar_Cfm         },  /* Not a codeword! (RAN) */
    { 49,  H245_PRIMITIVE,  E_PtvId_H223aar_Ack_Cfm     },  /* Not a codeword! (RAN) */
    { 49,  H245_PRIMITIVE,  E_PtvId_H223aar_Rjt_Cfm     },  /* Not a codeword! (RAN) */
    { 49,  H245_PRIMITIVE,  E_PtvId_Cnf_Req_Cfm         },
    { 49,  H245_PRIMITIVE,  E_PtvId_Cnf_Rps_Cfm         },
    { 49,  H245_PRIMITIVE,  E_PtvId_Cmd_Ecrpt_Cfm       },
    { 49,  H245_PRIMITIVE,  E_PtvId_Cmd_Es_Cfm          },
    { 49,  H245_PRIMITIVE,  E_PtvId_Cmd_Cnf_Cfm         },
    { 49,  H245_PRIMITIVE,  E_PtvId_Idc_Fnu_Cfm         },
    { 49,  H245_PRIMITIVE,  E_PtvId_Idc_Jtr_Cfm         },
    { 49,  H245_PRIMITIVE,  E_PtvId_Idc_Nwatmvc_Cfm     },
    { 49,  H245_PRIMITIVE,  E_PtvId_Idc_Cnf_Cfm         },
    { 49,  H245_PRIMITIVE,  E_PtvId_Idc_H2250mxskw_Cfm  },
    { 49,  H245_PRIMITIVE,  E_PtvId_Idc_Mclct_Cfm       },
    { 49,  H245_PRIMITIVE,  E_PtvId_Idc_Fns_Cfm         },
    { 49,  H245_PRIMITIVE,  E_PtvId_Idc_Fc_Cfm          },
    /* WWU_BLC: added table entries */
    { 50,  H245_PRIMITIVE,	E_PtvId_Blc_Etb_Cfm2		},
    { 52,  H245_PRIMITIVE,  E_PtvId_Cmd_Stcs_Cfm        }, /* RAN STCS */
    { 53,  H245_PRIMITIVE,  E_PtvId_Cmd_Fc_Cfm          }, /* RAN FC */
    { 54,  H245_PRIMITIVE,  E_PtvId_Cmd_Mscl_Cfm        }, /* RAN MSCL */
    { 55,  H245_PRIMITIVE,  E_PtvId_Cmd_Hmr_Cfm         }, /* RAN HMR */
    { 56,  H245_PRIMITIVE,  E_PtvId_Idc_Vi_Cfm          },
    { 57,  H245_PRIMITIVE,  E_PtvId_Idc_Ui_Cfm          }, /* AR: 2SD*/
    { 58,  H245_PRIMITIVE,  E_PtvId_Idc_Mscl_Cfm        },
    { 59,  H245_PRIMITIVE,  E_PtvId_Idc_H223skw_Cfm     },
    { 60,  H245_PRIMITIVE,  E_PtvId_Cmd_Fc          }, /* AR FC */
    {  0,  0             ,  0                       }
};


/*****************************************************************************/
/*  function name        : Tsc_EventReceive                                  */
/*  function outline     : Event Id Receive procedure                        */
/*  function discription : Tsc_EventReceive(pReceiveInf)                     */
/*  input data           : PS_InfHeader         Receive InfHeader Pointer    */
/*  output data          : uint32                 Event Id                     */
/*  draw time            : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendent career      :                                                   */
/*                                                                           */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
uint32 Tsc_EventReceive(PS_ControlMsgHeader  pReceiveInf)
{

    for (uint32 Cnt = 0; EventIdTable[Cnt].EventId != 0; ++Cnt)
    {
        if (((uint32)EventIdTable[Cnt].EventInfType == pReceiveInf->InfType) &&
                ((uint32)EventIdTable[Cnt].EventInfId == pReceiveInf->InfId))
        {
            return(EventIdTable[Cnt].EventId);
        }
    }
    return 0;
}

//===================================================================
// Tsc_UserEventLookup()                     (RAN) 6/26/00
//
// This one is added to process user events from App layer.
// It searches the UserEventIdTable to get the event ID, which
//   it then returns to the calling routine.
// NOTE: This routine differs from Tsc_EventReceive() in that
//   it doesn't pull the contents of pReceiveInf from a queue.
//   It instead assumes the contents are already there, having
//   been passed to Tsc by a direct call from the App layer.
//====================================================================
uint32 Tsc_UserEventLookup(PS_ControlMsgHeader pReceiveInf)
{
    if (pReceiveInf != NULL)
    {
        // Look up the UserEventId
        for (uint32 Cnt = 0; UserEventIdTable[Cnt].EventId != 0; ++Cnt)
        {
            if (((uint32)UserEventIdTable[Cnt].EventInfType == pReceiveInf->InfType) &&
                    ((uint32)UserEventIdTable[Cnt].EventInfId == pReceiveInf->InfId))
            {
                return(UserEventIdTable[Cnt].EventId);
            }
        }
        return 0;  // None found
    }
    return 0;
}
