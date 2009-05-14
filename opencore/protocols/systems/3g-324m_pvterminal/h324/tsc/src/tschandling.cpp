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

#include    "tscmain.h"
#include    "h245pri.h"         /* Primitive Infomation Header               */
#include    "h245_deleter.h"    // (RAN-LEAK) - Prototypes for deep delete
#include    "h245_copier.h"     // (RAN-LEAK) - Prototypes for deep copy
#include    "cpvh223multiplex.h"
#include    "logicalchannel.h"
#include    "oscl_mem.h"
#include    "tsc_constants.h"
#include    "tsc_statemanager.h"
#include    "tsc_lc.h"
#include    "tsc_clc.h"
#include    "tsc_blc.h"
#include    "tsc_mt.h"
#include    "tsc_component.h"
#include    "tsc_channelcontrol.h"
#include    "tsc_capability.h"

// =============================================================
// =============================================================
// =====   HANDLING ROUTINES FOR USER EVENTS (FROM APP)    =====
// =============================================================
// =============================================================

/*****************************************************************************/
/*  function name        : OpenSession                                   */
/*  input data           : PS_ControlMsgHeader         Dummy Pointer                */
/*  output data          : uint32                 Terminal Status              */
/*  draw time            : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  Note: This one is now called automatically when the TscMain thread      */
/*        is created.  No queue message is required. (RAN)                   */
/*                                                                           */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
uint32 TSC_324m::OpenSession(void)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m: OpenSession.\n"));

    if (iSuppInfoLen)
    {
        /* Send supp info in UII msg  and wait for TCS from remote terminal */
        Tsc_UII_Alphanumeric(iSuppInfo, (uint16)iSuppInfoLen);
    }
    else
    {
        CEStart();		// INITIATE CE-SEND
        Tsc_IdcVi();    // Send vendor information
        MSDStart();	    // INITIATE MSD
    }
    return(PhaseD_CSUP);
}

// ----------------------------------------------------------------
// NOTE: The 'PhaseF Closing Channels' routines are removed.  (RAN)
// These were:
//   Status09Event12(), Status09Event13(),
//   Status09Event17(), Status09Event18()
// ----------------------------------------------------------------

// =============================================================
// SessionClose_Comm()
//
// TSC Session Close (in Ongoing Communication) command received from user.
// =============================================================
uint32 TSC_324m::SessionClose_Comm()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m: SessionClose_Comm ... Start.\n"));
    iTerminalStatus = PhaseF_End;
    StopData();
    iTSCcomponent->CloseChannels();
    /* Primitive Send */
    EndSessionCommand();
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m: EndSession complete.\n"));
    return(iTerminalStatus);
}

// =============================================================
// SessionClose_CSUP()
//
// TSC Session Close (in Call Setup).
// =============================================================
uint32 TSC_324m::SessionClose_CSUP()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::SessionClose_CSUP"));
    iTerminalStatus = PhaseF_End;
    SignalCsupComplete(PVMFFailure);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m: EndSession complete."));
    return(iTerminalStatus);
}


// =============================================================
// =============================================================
// === HANDLING ROUTINES FOR SIGNAL ENTITY EVENTS (FROM 245) ===
// =============================================================
// =============================================================

/*****************************************************************************/
/*  function name        : LcEtbIdc           E_PtvId_Lc_Etb_Idc  */
/*  function outline     : Status04/Event09 procedure                        */
/*  function discription : Status04Event09( pReceiveInf )                */
/*  input data           : PS_ControlMsgHeader         Receive InfHeader Pointer    */
/*  output data          : uint32                 Terminal Status              */
/*  draw time            : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendment career(x)  :                                                   */
/*                                                                           */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
uint32 TSC_324m::LcEtbIdc(PS_ControlMsgHeader pReceiveInf)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m: LcEtbIdc.\n"));
    return iTSCcomponent->LcEtbIdc(pReceiveInf);
}

/*****************************************************************************/
/*  function name        : Status04Event10           E_PtvId_Lc_Etb_Cfm  */
/*  function outline     : Status04/Event10 procedure                        */
/*  function discription : Status04Event10( pReceiveInf )                */
/*  input data           : PS_ControlMsgHeader         Receive InfHeader Pointer    */
/*  output data          : uint32                 Terminal Status              */
/*  draw time            : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendment career(x)  :                                                   */
/*                                                                           */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
uint32 TSC_324m::LcEtbCfm(PS_ControlMsgHeader pReceiveInf)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m: LcEtbCfm.\n"));
    return iTSCcomponent->LcEtbCfm(pReceiveInf);
}

// =============================================================
// Status04Event11()                     E_PtvId_Lc_Rls_Idc
//
// This is LCSE RELEASE.indication.
// =============================================================
uint32 TSC_324m::LcRlsIdc(PS_ControlMsgHeader pReceiveInf)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m: LcRlsIdc.\n"));
    return iTSCcomponent->LcRlsIdc(pReceiveInf);
}

/*****************************************************************************/
/*  function name        : Status04Event13           E_PtvId_Lc_Err_Idc  */
/*  function outline     : Status04/Event13 procedure                        */
/*  function discription : Status04Event13( pReceiveInf )                */
/*  input data           : PS_ControlMsgHeader         Receive InfHeader Pointer    */
/*  output data          : uint32                 Terminal Status              */
/*  draw time            : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendment career(x)  :                                                   */
/*                                                                           */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
uint32 TSC_324m::LcErrIdc(PS_ControlMsgHeader  pReceiveInf)
{
    OSCL_UNUSED_ARG(pReceiveInf);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m: Logical channel error indication.\n"));

    /*
    ErrCode_Lc_A ,      // ( 6) inappropriate message ( OpenLogicalChannelAck )

    ErrCode_Lc_B ,      // ( 7) inappropriate message ( OpenLogicalChannelReject )

    ErrCode_Lc_C ,      // ( 8) inappropriate message ( CloseLogicalChannelAck )

    ErrCode_Lc_D ,      // ( 9) no response from Peer LCSE / B-LCSE ( Timer T103 Expiry ) */

    /*	switch(pReceiveInf->pParameter){
    	  case ErrCode_Lc_A:
    		break;
    	  case ErrCode_Lc_B:
    		break;
    	  case ErrCode_Lc_C:
    		break;
    	  case ErrCode_Lc_D:
    		break;
    	}
    */
    /* Buffer Free */
//    OSCL_DEFAULT_FREE( pReceiveInf->pParameter );
//    OSCL_DEFAULT_FREE( pReceiveInf );

    return(PhaseE_Comm);
}

/*****************************************************************************/
/*  function name        : Status04Event14          E_PtvId_Blc_Etb_Idc  */
/*  function outline     : Status04/Event14 procedure                        */
/*  function discription : Status04Event14( pReceiveInf )                */
/*  input data           : PS_ControlMsgHeader         Receive InfHeader Pointer    */
/*  output data          : uint32                 Terminal Status              */
/*  draw time            : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendment career(x)  :                                                   */
/*                                                                           */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
uint32 TSC_324m::BlcEtbIdc(PS_ControlMsgHeader pReceiveInf)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m: BlcEtbIdc.\n"));
    return iTSCcomponent->BlcEtbIdc(pReceiveInf);
}


/*****************************************************************************/
/*  function name        : Status04Event15          E_PtvId_Blc_Etb_Cfm  */
/*  function outline     : Status04/Event15 procedure                        */
/*  function discription : Status04Event15( pReceiveInf )                */
/*  input data           : PS_ControlMsgHeader         Receive InfHeader Pointer    */
/*  output data          : uint32                 Terminal Status              */
/*  draw time            : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendment career(x)  :                                                   */
/*                                                                           */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
uint32 TSC_324m::BlcEtbCfm(PS_ControlMsgHeader pReceiveInf)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::BlcEtbCfm forward(%d), reverse(%d))\n",
                     pReceiveInf->InfSupplement1,  pReceiveInf->InfSupplement2));
    if (iLoopbackMode == PV_LOOPBACK_NONE)
    {
        iTSCcomponent->BlcEtbCfm(pReceiveInf);
    }
    return PhaseE_Comm;
}

// =============================================================
// Status04Event16()                   E_PtvId_Blc_Rls_Idc
//
// This is BLCSE RELEASE.indication.  It is called when
// a Bi-Dir OLCReject is received.  It could be from an incoming or outgoing SE
// =============================================================
uint32 TSC_324m::BlcRlsIdc(PS_ControlMsgHeader pReceiveInf)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m: BlcRlsIdc.\n"));
    return iTSCcomponent->BlcRlsIdc(pReceiveInf);
}


/*****************************************************************************/
/*  function name        : Status04Event18          E_PtvId_Blc_Err_Idc  */
/*  function outline     : Status04/Event18 procedure                        */
/*  function discription : Status04Event18( pReceiveInf )                */
/*  input data           : PS_ControlMsgHeader         Receive InfHeader Pointer    */
/*  output data          : uint32                 Terminal Status              */
/*  draw time            : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendment career(x)  :                                                   */
/*                                                                           */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
uint32 TSC_324m::BlcErrIdc(PS_ControlMsgHeader  pReceiveInf)
{
    OSCL_UNUSED_ARG(pReceiveInf);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::BlcErrIdc "));
    return PhaseE_Comm;
}

/*****************************************************************************/
/*  function name        : Status04Event22           E_PtvId_Mt_Trf_Idc  */
/*  function outline     : Status04/Event22 procedure                        */
/*  function discription : Status04Event22( pReceiveInf )                */
/*  input data           : PS_ControlMsgHeader         Receive InfHeader Pointer    */
/*  output data          : uint32                 Terminal Status              */
/*  draw time            : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendment career(x)  :                                                   */
/*                                                                           */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
uint32 TSC_324m::Status04Event22(PS_ControlMsgHeader  pReceiveInf)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::Status04Event22"));
    return MuxTableTransferIndication(pReceiveInf);
}

uint32 TSC_324m::MuxTableTransferIndication(PS_ControlMsgHeader  pReceiveInf)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::MuxTableTransferIndication, wnsrp status=%d",
                     iSrp->WnsrpStatusGet()));
    PS_MuxDescriptor pMux = (PS_MuxDescriptor)pReceiveInf->pParameter;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::Status04Event22 Received mux entries sn=%d, num mux entries=%d",
                     pReceiveInf->InfSupplement1, pMux->size_of_multiplexEntryDescriptors));

    CPVMultiplexDescriptor desc(pMux);
    ((CPVH223Multiplex*)iH223)->SetIncomingMuxTableEntries(&desc);

    /* If WNSRP is enabled and Mux Entry TSC_WNSRP_MUX_ENTRY_NUMBER is received, disable WNSRP */
    if (iSrp->WnsrpStatusGet() == SRP::WNSRP_TX_SUPPORT)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_324m::Status04Event22 WNSRP is ON.  Checking for mux entry 15."));
        for (unsigned entry_num = 0; entry_num < (unsigned)pMux->size_of_multiplexEntryDescriptors; entry_num++)
        {
            if (pMux->multiplexEntryDescriptors[entry_num].multiplexTableEntryNumber ==
                    TSC_WNSRP_MUX_ENTRY_NUMBER)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "TSC_324m::Status04Event22 Received new descriptor for mux entry 15.  Disabling WNSRP."));
                iSrp->DisableWNSRPSupport();
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "TSC_324m::Status04Event22 Received new descriptor for mux entry 15.  Flushing control channel."));
                /* Flush the control logical channel */
                PVMFStatus aStatus = PVMFFailure;
                H223OutgoingChannelPtr outgoing_control_channel;
                aStatus = iH223->GetOutgoingChannel(0, outgoing_control_channel);
                if (aStatus == PVMFSuccess)
                {
                    outgoing_control_channel->Flush();
                }
                else
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                                    (0, "TSC_324m::MuxTableTransferIndication Error - Failed to lookup logical channel 0"));
                    return iTerminalStatus;
                }
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "TSC_324m::Status04Event22 Received new descriptor for mux entry 15.  Done."));
                break;
            }
        }
    }
    /* TRANSFER.response(MT) Primitive Send */
    iTSCmt.MtTrfRps(pReceiveInf->InfSupplement1, pMux); // Pass pMux on for response.
    desc.descriptor = NULL;
    return 0;
}

/*****************************************************************************/
/*  function name        : Status04Event23            E_PtvId_Mt_Trf_Cfm */
/*  function outline     : Status04/Event23 procedure                        */
/*  function discription : Status04Event23( pReceiveInf )                */
/*  input data           : PS_ControlMsgHeader         Receive InfHeader Pointer    */
/*  output data          : uint32                 Terminal Status              */
/*  draw time            : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendment career(x)  :                                                   */
/*                                                                           */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
uint32 TSC_324m::Status04Event23(PS_ControlMsgHeader  pReceiveInf)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::Status04Event23 sn=%d", pReceiveInf->InfSupplement1));
    iTSCcomponent->MuxTableSendComplete(pReceiveInf->InfSupplement1, PVMFSuccess);
    return 0;
}


OsclAny TSC_324m::TcsMsdComplete()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::TcsMsdComplete"));
    SignalCsupComplete(PVMFSuccess);
    iTSCcomponent->TcsMsdComplete();
}

OsclAny TSC_324m::SignalCsupComplete(PVMFStatus status)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::SignalCsupComplete(%d)\n", status));
    iObserver->ConnectComplete(status);
    // update the node state
    if (status == PVMFSuccess)
    {
        iTerminalStatus = PhaseE_Comm;
    }
}

/*****************************************************************************/
/*  function name        : Status04Event24            E_PtvId_Mt_Rjt_Idc */
/*  function outline     : Status04/Event24 procedure                        */
/*  function discription : Status04Event24( pReceiveInf )                */
/*  input data           : PS_ControlMsgHeader         Receive InfHeader Pointer    */
/*  output data          : uint32                 Terminal Status              */
/*  draw time            : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendment career(x)  :                                                   */
/*                                                                           */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
uint32 TSC_324m::Status04Event24(PS_ControlMsgHeader  pReceiveInf)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                    (0, "TSC_324m::Status04Event24 Mux table reject received dir(%d), sn(%d)",
                     pReceiveInf->InfSupplement1, pReceiveInf->InfSupplement2));
    if ((S_InfHeader::TDirection)pReceiveInf->InfSupplement1 == S_InfHeader::OUTGOING)
    {
        iTSCcomponent->MuxTableSendComplete(pReceiveInf->InfSupplement2,
                                            PVMFFailure);
    }
    return(PhaseD_CSUP);
}

// =============================================================
// Status04Event50()                  E_PtvId_Blc_Etb_Cfm2
//
// This is "BLCSE ESTABLISH.confirm2"
// It is called when SE receives an OLCConfirm (Bi-Dir).
// =============================================================
uint32 TSC_324m::BlcEtbCfm2(PS_ControlMsgHeader  pReceiveInf)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m: BlcEtbCfm2.\n"));
    return iTSCcomponent->BlcEtbCfm2(pReceiveInf);
}


/*****************************************************************************/
/*  function name        : Status08Event12           E_PtvId_Lc_Rls_Cfm  */
/*  function outline     : Status08/Event12 procedure                        */
/*  function discription : Status08Event12( pReceiveInf )                */
/*  input data           : PS_ControlMsgHeader         Receive InfHeader Pointer    */
/*  output data          : uint32                 Terminal Status              */
/*  draw time            : '00.4.13                                          */
/*---------------------------------------------------------------------------*/
uint32 TSC_324m::LcRlsCfm(PS_ControlMsgHeader pReceiveInf)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m: LcRlsCfm.\n"));
    iTSCcomponent->LcRlsCfm(pReceiveInf);
    return(PhaseE_Comm);
}


/*****************************************************************************/
/*  function name        : Status08Event17          E_PtvId_Blc_Rls_Cfm  */
/*  function outline     : Status08/Event17 procedure                        */
/*  function discription : Status08Event17( pReceiveInf )                */
/*  input data           : PS_ControlMsgHeader         Receive InfHeader Pointer    */
/*  output data          : uint32                 Terminal Status              */
/*  draw time            : '00.4.13                                         */
/*---------------------------------------------------------------------------*/
/* RAN - Bi-Dir OLCAck */
uint32 TSC_324m::BlcRlsCfm(PS_ControlMsgHeader pReceiveInf)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m: BlcRlsCfm.\n"));
    iTSCcomponent->BlcRlsCfm(pReceiveInf);
    return(PhaseE_Comm);
}

/*****************************************************************************/
/*  function name        : Status08Event19          E_PtvId_Clc_Cls_Idc  */
/*  function outline     : Status08/Event19 procedure                        */
/*  function discription : Status08Event19( pReceiveInf )                */
/*  input data           : PS_ControlMsgHeader         Receive InfHeader Pointer    */
/*  output data          : uint32                 Terminal Status              */
/*  draw time            : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendment career(x)  :                                                   */
/*                                                                           */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
uint32 TSC_324m::Status08Event19(PS_ControlMsgHeader pReceiveInf)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m: Status08Event19.\n"));
    return iTSCcomponent->Status08Event19(pReceiveInf);
}

/*****************************************************************************/
/*  function name        : Status08Event20          E_PtvId_Clc_Cls_Cfm  */
/*  function outline     : Status08/Event20 procedure                        */
/*  function discription : Status08Event20( pReceiveInf )                */
/*  input data           : PS_ControlMsgHeader         Receive InfHeader Pointer    */
/*  output data          : uint32                 Terminal Status              */
/*  draw time            : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendment career(x)  :                                                   */
/*                                                                           */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
uint32 TSC_324m::Status08Event20(PS_ControlMsgHeader  pReceiveInf)
{
    OSCL_UNUSED_ARG(pReceiveInf);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m: RequestChannelClose(S)...Complete\n"));
    return(PhaseE_Comm);
}

/*****************************************************************************/
/*  function name        : Status08Event21           E_PtvId_Clc_Rjt_Idc */
/*  function outline     : Status08/Event21 procedure                        */
/*  function discription : Status08Event21( pReceiveInf )                */
/*  input data           : PS_ControlMsgHeader         Receive InfHeader Pointer    */
/*  output data          : uint32                 Terminal Status              */
/*  draw time            : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendment career(x)  :                                                   */
/*                                                                           */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
uint32 TSC_324m::Status08Event21(PS_ControlMsgHeader  pReceiveInf)
{
    OSCL_UNUSED_ARG(pReceiveInf);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m: RequestChannelClose(%d)...Failed\n", pReceiveInf->InfSupplement1));
    return(PhaseE_Comm);
}

/*****************************************************************************/
/*  function name        : Status08Event22            E_PtvId_Mt_Trf_Idc */
/*  function outline     : Status08/Event22 procedure                        */
/*  function discription : Status08Event22( pReceiveInf )                */
/*  input data           : PS_ControlMsgHeader         Receive InfHeader Pointer    */
/*  output data          : uint32                 Terminal Status              */
/*  draw time            : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendment career(x)  :                                                   */
/*                                                                           */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
uint32 TSC_324m::Status08Event22(PS_ControlMsgHeader  pReceiveInf)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::Status08Event22 Received mux entries sn=%d",
                     pReceiveInf->InfSupplement1));
    return MuxTableTransferIndication(pReceiveInf);
}

/*****************************************************************************/
/*  function name        : Status08Event23            E_PtvId_Mt_Trf_Cfm */
/*  function outline     : Status08/Event23 procedure                        */
/*  function discription : Status08Event23( pReceiveInf )                */
/*  input data           : PS_ControlMsgHeader         Receive InfHeader Pointer    */
/*  output data          : uint32                 Terminal Status              */
/*  draw time            : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendment career(x)  :                                                   */
/*                                                                           */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
uint32 TSC_324m::Status08Event23(PS_ControlMsgHeader  pReceiveInf)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::Status04Event23 sn=%d",
                     pReceiveInf->InfSupplement1));
    iTSCcomponent->MuxTableSendComplete(pReceiveInf->InfSupplement1,
                                        PVMFSuccess);
    return(PhaseE_Comm);
}

/*****************************************************************************/
/*  function name        : Status08Event24            E_PtvId_Mt_Rjt_Idc */
/*  function outline     : Status08/Event24 procedure                        */
/*  function discription : Status08Event24( pReceiveInf )                */
/*  input data           : PS_ControlMsgHeader         Receive InfHeader Pointer    */
/*  output data          : uint32                 Terminal Status              */
/*  draw time            : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendment career(x)  :                                                   */
/*                                                                           */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
uint32 TSC_324m::Status08Event24(PS_ControlMsgHeader  pReceiveInf)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                    (0, "TSC_324m::Status08Event24 Mux table reject received dir(%d), sn(%d)",
                     pReceiveInf->InfSupplement1, pReceiveInf->InfSupplement2));
    if ((S_InfHeader::TDirection)pReceiveInf->InfSupplement1 ==
            S_InfHeader::OUTGOING)
    {
        iTSCcomponent->MuxTableSendComplete(pReceiveInf->InfSupplement2,
                                            PVMFFailure);
    }
    return(PhaseE_Comm);
}

/*****************************************************************************/
/*  function name        : Status08Event25          E_PtvId_Rme_Send_Idc */
/*  function outline     : Status08/Event25 procedure                        */
/*  function discription : Status08Event25( pReceiveInf )                */
/*  input data           : PS_ControlMsgHeader         Receive InfHeader Pointer    */
/*  output data          : uint32                 Terminal Status              */
/*  draw time            : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendment career(x)  :                                                   */
/*                                                                           */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
uint32 TSC_324m::Status08Event25(PS_ControlMsgHeader  pReceiveInf)
{
    OSCL_UNUSED_ARG(pReceiveInf);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::Status08Event25 RME received.  Rejecting it."));
    RmeRjtReq();
    return(PhaseE_Comm);
}

/*****************************************************************************/
/*  function name        : Status08Event26          E_PtvId_Rme_Send_Cfm */
/*  function outline     : Status08/Event26 procedure                        */
/*  function discription : Status08Event26( pReceiveInf )                */
/*  input data           : PS_ControlMsgHeader         Receive InfHeader Pointer    */
/*  output data          : uint32                 Terminal Status              */
/*  draw time            : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendment career(x)  :                                                   */
/*                                                                           */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
uint32 TSC_324m::Status08Event26(PS_ControlMsgHeader  pReceiveInf)
{
    OSCL_UNUSED_ARG(pReceiveInf);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC: RME complete.\n"));

    return(PhaseE_Comm);
}

/*****************************************************************************/
/*  function name        : Status08Event27         E_PtvId_Rme_Rjt_Idc   */
/*  function outline     : Status08/Event27 procedure                        */
/*  function discription : Status08Event27( pReceiveInf )                */
/*  input data           : PS_ControlMsgHeader         Receive InfHeader Pointer    */
/*  output data          : uint32                 Terminal Status              */
/*  draw time            : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendment career(x)  :                                                   */
/*                                                                           */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
uint32 TSC_324m::Status08Event27(PS_ControlMsgHeader  pReceiveInf)
{
    OSCL_UNUSED_ARG(pReceiveInf);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC: RME failed: reject indication.\n"));
    /*  NO ACTION FOR NOW.  MAY WANT TO ADD BETTER HANDLING LATER.
    */
    return(PhaseE_Comm);
}

/*****************************************************************************/
/*  function name        : ModeRequestIndication           E_PtvId_Mr_Trf_Idc  */
/*  function outline     : ModeRequestIndication procedure                        */
/*  function discription : ModeRequestIndication( pReceiveInf )                */
/*  input data           : PS_ControlMsgHeader         Receive InfHeader Pointer    */
/*  output data          : uint32                 Terminal Status              */
/*  draw time            : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendment career(x)  :                                                   */
/*                                                                           */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
/* WWURM: modified to handle incoming request mode message */
uint32 TSC_324m::ModeRequestIndication(PS_ControlMsgHeader  pReceiveInf)
{
    OSCL_UNUSED_ARG(pReceiveInf);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::ModeRequestIndication"));
    MrRjtReq();
    return(PhaseE_Comm);
}

// -------------------------------------------------
// getModeRequestInfo()
//
// WWURM: help function for request mode message.
// -------------------------------------------------
OsclAny TSC_324m::getModeRequestInfo(PS_ControlMsgHeader pReceiveInf,
                                     int32 *param1, int32 *param2)
{
    OSCL_UNUSED_ARG(pReceiveInf);
    OSCL_UNUSED_ARG(param2);
    OSCL_UNUSED_ARG(param1);
}

/*****************************************************************************/
/*  function name        : Status08Event29           E_PtvId_Mr_Trf_Cfm  */
/*  function outline     : Status08/Event29 procedure                        */
/*  function discription : Status08Event29( pReceiveInf )                */
/*  input data           : PS_ControlMsgHeader         Receive InfHeader Pointer    */
/*  output data          : uint32                 Terminal Status              */
/*  draw time            : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendment career(x)  :                                                   */
/*                                                                           */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
uint32 TSC_324m::Status08Event29(PS_ControlMsgHeader  pReceiveInf)
{
    OSCL_UNUSED_ARG(pReceiveInf);
    return(PhaseE_Comm);
}

/*****************************************************************************/
/*  function name        : Status08Event30            E_PtvId_Mr_Rjt_Idc */
/*  function outline     : Status08/Event30 procedure                        */
/*  function discription : Status08Event30( pReceiveInf )                */
/*  input data           : PS_ControlMsgHeader         Receive InfHeader Pointer    */
/*  output data          : uint32                 Terminal Status              */
/*  draw time            : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendment career(x)  :                                                   */
/*                                                                           */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
uint32 TSC_324m::Status08Event30(PS_ControlMsgHeader  pReceiveInf)
{
    OSCL_UNUSED_ARG(pReceiveInf);
    return(PhaseE_Comm);
}

/*****************************************************************************/
/*  function name        : Status08Event31           E_PtvId_Rtd_Trf_Cfm */
/*  function outline     : Status08/Event31 procedure                        */
/*  function discription : Status08Event31( pReceiveInf )                */
/*  input data           : PS_ControlMsgHeader         Receive InfHeader Pointer    */
/*  output data          : uint32                 Terminal Status              */
/*  draw time            : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendment career(x)  :                                                   */
/*                                                                           */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
uint32 TSC_324m::Status08Event31(PS_ControlMsgHeader  pReceiveInf)
{
    OSCL_UNUSED_ARG(pReceiveInf);
    uint32 rtd = (uint32)((PS_Delay_Rtd)pReceiveInf->pParameter)->Delay_Value;
    iNumRtdRequests++;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::Status08Event31 - iNumRtdRequests = %d, RTD = %d ms",
                     iNumRtdRequests, rtd));
    iRtdAve += rtd;
    if (rtd < iRtdMin)
    {
        iRtdMin = rtd;
    }
    if (rtd > iRtdMax)
    {
        iRtdMax = rtd;
    }

    return(PhaseE_Comm);
}

/*****************************************************************************/
/*  function name        : Status08Event32           E_PtvId_Rtd_Exp_Idc */
/*  function outline     : Status08/Event32 procedure                        */
/*  function discription : Status08Event32( pReceiveInf )                */
/*  input data           : PS_ControlMsgHeader         Receive InfHeader Pointer    */
/*  output data          : uint32                 Terminal Status              */
/*  draw time            : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendment career(x)  :                                                   */
/*                                                                           */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
uint32 TSC_324m::Status08Event32(PS_ControlMsgHeader  pReceiveInf)
{
    OSCL_UNUSED_ARG(pReceiveInf);
    return(PhaseE_Comm);
}

//===============================================================
// SendTerminalCapabilitySet()               E_PtvId_Cmd_Stcs_Cfm (RAN)
//
// Handle a received SendTerminalCapabilitySet codeword.
//===============================================================
uint32 TSC_324m::SendTerminalCapabilitySet(PS_ControlMsgHeader  pReceiveInf)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::SendTerminalCapabilitySet"));
    OSCL_UNUSED_ARG(pReceiveInf);
    CEStart();
    return(PhaseE_Comm);
}

//===============================================================
// FlowControlCommandReceived()                 E_PtvId_Cmd_Fc_Cfm (RAN)
//
// This routine handles a received FlowControl (FC) command.
// For now it does nothing but print the command received message.
// But later we should pass the received params up to application layer.
//===============================================================
uint32 TSC_324m::FlowControlCommandReceived(PS_ControlMsgHeader  pReceiveInf)
{
    OSCL_UNUSED_ARG(pReceiveInf);
    SendFunctionNotSupportedIndication(2, pReceiveInf->EncodedMsg,
                                       (uint16)pReceiveInf->EncodedMsgSize);//unknown function

    return(PhaseE_Comm);
}

//===============================================================
// MiscCmdRecv()              E_PtvId_Cmd_Mscl_Cfm (RAN)
//
// This routine handles a received Miscellaneous (MSCL) command.
//===============================================================
uint32 TSC_324m::MiscCmdRecv(PS_ControlMsgHeader  pReceiveInf)
{
    PS_MiscellaneousCommand mc;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::MiscCmdRecv - Received Miscellaneous command"));
    mc = (PS_MiscellaneousCommand) pReceiveInf->pParameter;

    // if command is a request for new I-Frame, then verify outgoing
    // channel exists and request new I-Frame from observer
    if (mc->mcType.index == 5)    // 5 = videoFastUpdatePicture
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_324m: MiscCmdRecv - Received videoFastUpdatePicture, lcn(%d)\n",
                         mc->logicalChannelNumber));
        if (iTerminalStatus != PhaseE_Comm)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "TSC_324m: MiscCmdRecv Error - invalid state(%d)",
                             iTerminalStatus));
            return iTerminalStatus;
        }
        H223OutgoingChannelPtr lcn;
        PVMFStatus aStatus = iH223->GetOutgoingChannel(mc->logicalChannelNumber,
                             lcn);
        if (aStatus != PVMFSuccess)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "TSC_324m::MiscCmdRecv Error - Request I-Frame for invalid channel ID=%d",
                             mc->logicalChannelNumber));
            return iTerminalStatus;
        }
        if (iObserver) iObserver->RequestFrameUpdate(lcn);
    }
    if (mc->mcType.index == 7)    // 7 = videoSpatialTemporalTradeoff
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_324m: MiscCmdRecv - Received videoTemporalSpatialTradeOff lcn(%d), value(%d)",
                         mc->logicalChannelNumber, mc->mcType.videoTemporalSpatialTradeOff));
        if (iTerminalStatus != PhaseE_Comm)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "TSC_324m: MiscCmdRecv Error - invalid state(%d)",
                             iTerminalStatus));
            return iTerminalStatus;
        }
        if (iTSC_324mObserver)
        {
            iTSC_324mObserver->VideoSpatialTemporalTradeoffCommandReceived(mc->logicalChannelNumber,
                    mc->mcType.videoTemporalSpatialTradeOff);
        }
    }
    else if (mc->mcType.index == 11)   // 11 = maxMuxPduSize
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_324m: MiscCmdRecv - Received maxMuxPduSize, size(%d)\n",
                         mc->mcType.maxH223MUXPDUsize));
        TPVStatusCode ret = iH223->SetMaxOutgoingPduSize(mc->mcType.maxH223MUXPDUsize);
        if (ret != EPVT_Success)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "TSC_324m: Error - Request maxMuxPduSize(%d), error(%d)\n",
                             mc->mcType.maxH223MUXPDUsize, ret));
        }
    }
    return(iTerminalStatus);
}

//===============================================================
// Status08Event55()               E_PtvId_Cmd_Hmr_Cfm (RAN)
//
// This routine handles a received H223MultiplexReconfiguration (HMR) command.
// For now it does nothing but print the command received message.
// But later we should pass the received params up to application layer.
//===============================================================
uint32 TSC_324m::Status08Event55(PS_ControlMsgHeader  pReceiveInf)
{
    OSCL_UNUSED_ARG(pReceiveInf);
    SendFunctionNotSupportedIndication(2, pReceiveInf->EncodedMsg,
                                       (uint16)pReceiveInf->EncodedMsgSize);//unknown function
    return(PhaseE_Comm);
}

/*****************************************************************************/
/*  function name        : EndSessionRecv          E_PtvId_Cmd_Es_Cfm    */
/*  function outline     : EndSessionCommand Receive procedure               */
/*  function discription : EndSessionRecv( pReceiveInf )                 */
/*  input data           : PS_ControlMsgHeader         Receive InfHeader Pointer    */
/*  output data          : uint32                 Terminal Status              */
/*  draw time            : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendment career(x)  :                                                   */
/*                                                                           */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
uint32 TSC_324m::EndSessionRecv(PS_ControlMsgHeader  pReceiveInf)
{
    OSCL_UNUSED_ARG(pReceiveInf);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::EndSessionRecv"));
    iDisconnectInitiator |= EPVT_REMOTE;
    switch (iInterfaceState)
    {
        case EPVMFNodeStarted:
        case EPVMFNodePaused:
            iObserver->DisconnectRequestReceived();
            break;
        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "EndSession...Received in invalid state"));
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "TSC_324m::EndSessionRecv Ignoring end session due to invalid state(%d)",
                             iInterfaceState));
    }
    StopData();
    iTSCcomponent->CloseChannels();
    return PhaseF_End;
}


// ===============================================================
// StopData()
//
// Sets flags in the H324 system table which stop data transmission
// for each open, outgoing logical channel.
// ================================================================
OsclAny TSC_324m::StopData()
{
    iTSCcomponent->StopData();
}


// ======================================================
// GetDecoderConfigFromOLC()
//
// Returns the decoder configuration info from an incoming
// OLC codeword.  The info is copied from either the
// Forward or Reverse LogicalChannelParameters, depending
// on the value of input argument forRev:
//
//     forRev = 0  --> info from Forward Parameters
//     forRev = 1  --> info from Reverse Parameters
//
// Note that if 'pPara' does not have genericVideo description
//   (i.e. MPEG4), then a NULL string is returned.
// =======================================================
uint8* GetDecoderConfigFromOLC(PS_ForwardReverseParam pPara, uint32 forRev, uint16 *nOctets)
{
    uint32 i = 0, n = 0;
    PS_GenericCapability pGenericCap = NULL;
    PS_GenericParameter	nonCollapsing = NULL;
    uint8* ret = NULL;

    if (forRev)  	/* reverse */
    {
        if (pPara->reverseLogicalChannelParameters.dataType.videoData->index == 5) /* genericVideoCapability */
        {
            pGenericCap = pPara->reverseLogicalChannelParameters.dataType.videoData->genericVideoCapability;
        }
    }
    else  		/* forward  */
    {
        if (pPara->forwardLogicalChannelParameters.dataType.videoData->index == 5) /* genericVideoCapability */
        {
            pGenericCap = pPara->forwardLogicalChannelParameters.dataType.videoData->genericVideoCapability;
        }
    }

    if (pGenericCap != NULL) 	/* Search for decoderConfig parameters */
    {
        n = pGenericCap->size_of_nonCollapsing;
        for (i = 0; i < n; i++)
        {
            nonCollapsing = &pGenericCap->nonCollapsing[i];
            if (nonCollapsing->parameterIdentifier.standard == 2)
            {
                *nOctets = nonCollapsing->parameterValue.octetString->size;
                ret = (uint8*) OSCL_DEFAULT_MALLOC(*nOctets);
                oscl_memcpy(ret, nonCollapsing->parameterValue.octetString->data, *nOctets);
                return ret;
            }
        }
    }
    *nOctets = 0;	/* None found; return empty string */
    return NULL;
}

// =========================================================
// ShowTsc()
//
// This function takes an output analysis line, adds the
// proper indent, and sends the result out to the display
// routine.  The 'tag' argument is simply passed on.
// =========================================================
OsclAny TSC_324m::ShowTsc(uint16 tag, uint16 indent, char* inString)
{
    OSCL_UNUSED_ARG(inString);
    OSCL_UNUSED_ARG(indent);
    OSCL_UNUSED_ARG(tag);
#ifdef PVTsc_Analyzer
    uint32 i;
    char outString[200];

    /* Construct outString with proper indent */
    for (i = 0;i < indent;++i)
    {
        outString[i] = ' ';
    }
    sprintf(outString + i, "%s", inString);

    /* Send outString to display */
    PV//Tsc_Analyzer(tag,outString);         /* The real deal */
#endif
}

// =======================================================
// //Tsc_Analyzer()
//
// Display messages using the central Tsc_Analyzer functions.
// =======================================================
/*OsclAny TSC_324m::Tsc_Analyzer(char* msg)
{
#ifdef PVTsc_Analyzer
	ShowTsc(Tsc_Analyzer_TSC,0,msg);
#endif
}*/





// =======================================================
// CheckAltCapSet()								(RAN-32K)
//
// This routine checks an AlternativeCapabilitySet structure
//   to see if a particular entry number is present.  Simple.
// =======================================================
uint32 TSC_324m::CheckAltCapSet(PS_AlternativeCapabilitySet pAltCapSet,
                                uint32 entry, uint32* preference_index)
{
    uint32 i;
    *preference_index = 999;
    for (i = 0;i < pAltCapSet->size;++i)
    {
        if (pAltCapSet->item[i] == entry)
        {
            *preference_index = i;
            return(true);
        }
    }

    return(false);
}


CPvtTerminalCapability* TSC_324m::GetRemoteCapability()
{
    return iTSCcapability.GetRemoteCapability();
}
