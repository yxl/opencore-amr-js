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
/*  file name            : tsc_ce.c                                          */
/*  file contents        : Terminal State Control routine                    */
/*  draw                 : '96.10.04                                         */
/*---------------------------------------------------------------------------*/
/*  amendment                                                                */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
#include "tscmain.h"
#include "tsc_sub.h"     /* Sub Routine Information Header                */
#include "tsc_constants.h"
#include "tsc_statemanager.h"
#include "tsc_capability.h"
#include "tsc_component.h"

#define PV2WAY_DEFAULT_USER_INPUT_CAPABILITY_INDEX 2 /* Index for IA5 string */

//////////////////////////////////////////////////////////////////////////
// Start the CE process by sending this terminals capabilites to the peer terminal
//////////////////////////////////////////////////////////////////////////
void TSC_324m::CEStart(void)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "TSC_324m: CEStart\n"));
    if (iCeRetries <= 0)
    {
        return;
    }
    if (iTSCcomponent->CEStart())
    {
        iCeRetries--;
    }
}

////////////////////////////////////////////////////////////////////////
// CE User - Transfer.Indication primitive received from H.245
////////////////////////////////////////////////////////////////////////
void TSC_324m::CETransferIndication(OsclSharedPtr<S_TerminalCapabilitySet> tcs)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m: CE transfer indication received.\n"));

    // Cancel timers for TCS receive
    iTimer->Cancel(PV_TSC_TCS_RECEIVE_TIMER_ID);
    iH223->EnableStuffing(false);

    Tsc324mNodeCommand* cmd = iTSCcomponent->GetCommand();
    if (cmd)
    {
        iCmdQueue.AddL(*cmd);
        RunIfNotReady();
    }


    ////////////////////////
    // State = Call Setup
    ////////////////////////
    if (iTerminalStatus == PhaseD_CSUP)
    {
        // TRANSFER.response(CE) Primitive Send
        if (Ce) Ce->TransferResponse();

        ExtractTcsParameters(tcs);

        iTSCcomponent->CETransferIndication(tcs, iTerminalStatus);

        if (iTSCcomponent->GetVideoLayer() == 0)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "TSC_324m::CETransferIndication - Failed to negotiate video layer.\n"));
            iCeRetries = iN100;
            iConnectFailReason = EPVT_FailedToNegotiate;
            SessionClose_CSUP();
        }
        if (iTSCstatemanager.ReadState(TSC_CE_RECEIVE) != COMPLETE)
        {
            iTSCstatemanager.WriteState(TSC_CE_RECEIVE, COMPLETE);

            if ((iTSCstatemanager.ReadState(TSC_MSD) == COMPLETE))
            {
                int leave_status = 0;
                OSCL_TRY(leave_status, TcsMsdComplete());
                OSCL_FIRST_CATCH_ANY(leave_status, void());
                if (leave_status != 0)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                                    (0, "TSC_324m::CETransferIndication - Memory Allocation Failed."));
                    SignalCsupComplete(PVMFErrNoMemory);
                    return;
                }
            }
        }
    }
    //////////////////////////////////
    // State = Ongoing Communication
    //////////////////////////////////
    else if (iTerminalStatus == PhaseE_Comm)
    {
        iTSCcomponent->CETransferIndication(tcs, iTerminalStatus);
        if (Ce) Ce->TransferResponse();
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_324m::CETransferIndication Error - invalid state(%d)", iTerminalStatus));
    }
}

////////////////////////////////////////////////////////////////////////
// CE User - Transfer.Confirm primitive received from H.245
////////////////////////////////////////////////////////////////////////
void TSC_324m::CETransferConfirm()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::CETransferConfirm Capability Exchange Send...Complete."));
    ////////////////////////
    // State = Call Setup
    ////////////////////////
    if (iTerminalStatus == PhaseD_CSUP || iTerminalStatus == PhaseE_Comm)
    {
        iTSCstatemanager.WriteState(TSC_CE_SEND, COMPLETE);
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_324m::CETransferConfirm Error - Invalid state(%d)", iTerminalStatus));
    }
    iCeRetries = iN100; /* Num CE retries left */
}

////////////////////////////////////////////////////////////////////////
// CE User - Reject.Indication primitive received from H.245
////////////////////////////////////////////////////////////////////////
void TSC_324m::CERejectIndication(CESource source, CECause cause, CEDirection direction)
{
    OSCL_UNUSED_ARG(cause);

    /* WWUAPI: four scenarios can cause this function being called
    	1. INCOMING SE gets TCS again while in AWAITING RESPONSE state
    	   We send out CE RPS as soon as we receive a TCS.  It means the other temrinal
    	   is a whacky terminal sending out successive TCS's.  Maybe we just use the most
    	   recent TCS ?
    	2. OUTGOING SE gets REJECT while in AWAITING RESPONSE state
    	   End Session and reset.
    	3. TIMEOUT for OUTGOING SE
    	   End Session and reset if we are in PhaseD_CSUP
    	4. INCOMING SE gets RELEASE while in AWAITING RESPONSE
    	   We took too much time to send a response and remote SE timed out.
    	   End Session and reset
       We need a fat brain in the future to handle all these unusual cases.
       Here we just do nothing.
    */

    ////////////////////////
    // State = Call Setup
    ////////////////////////
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m: CE Reject Indication received."));

    if (iTerminalStatus == PhaseD_CSUP || iTerminalStatus == PhaseE_Comm)
    {
        if (direction == CE_OUTGOING)
        {
            if (source == CE_USER)  /* Reject */
            {
                iCeRetries = iN100;
                iConnectFailReason = EPVT_ErrorRemoteRejected;
                SessionClose_CSUP();
            }
            else  /* Caused by PROTOCOL, most likely due to timeouts */
            {
                if (iCeRetries)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                    (0, "TSC_324m: Starting CE Send.\n"));
                    CEStart();		/* INITIATE CE-SEND */
                }
                else  /* Retried enough. Call setup failed */
                {
                    iCeRetries = iN100;
                    iConnectFailReason = EPVT_Timeout;
                    SessionClose_CSUP();
                }
            }
        }
        else  /* INCOMING */
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "TSC_324m: Capability Exchange(I)...Release received.\n"));
            SessionClose_CSUP();
        }
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_324m::CERejectIndication Error - Invalid state(%d)", iTerminalStatus));
    }
}

////////////////////////////////////////////////////////////////////////////
// ExtractTcsParameters()						(RAN-32K)
//
// This routine takes the incoming TerminalCapabilitySet
//   and extracts the following useful parameters:
//      {h263_qcifMPI, h263_maxBitRate, mpeg4_maxBitRate}
// The parameters are stored in globals and may be sent
//   later to the application.
////////////////////////////////////////////////////////////////////////////
void TSC_324m::ExtractTcsParameters(PS_TerminalCapabilitySet pTcs)
{
    uint32 i;
    PS_CapabilityTableEntry pCapEntry = NULL;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::ExtractTcsParameters"));

    iTSCcapability.ResetCapability();
    Oscl_Vector<CPvtMediaCapability*, OsclMemAllocator> capabilityItems;
    int userInputCapabilities = 0;//1<<PV2WAY_DEFAULT_USER_INPUT_CAPABILITY_INDEX;
    struct _UserInputCapability *userInputCapability = NULL;

    iTSCcomponent->ExtractTcsParameters(pTcs);
    if (pTcs->option_of_multiplexCapability)
    {
        PS_MultiplexCapability muxcaps = &pTcs->multiplexCapability;
        if (muxcaps->index == 2)
        {
            PS_H223Capability h223caps = muxcaps->h223Capability;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "TSC_324m::ExtractTcsParameters maximumAl2SDUSize=%d,maximumAl3SDUSize=%d",
                             h223caps->maximumAl2SDUSize, h223caps->maximumAl3SDUSize));
            iH223->SetSduSize(OUTGOING, h223caps->maximumAl2SDUSize, E_EP_MEDIUM);
            iH223->SetSduSize(OUTGOING, h223caps->maximumAl3SDUSize, E_EP_HIGH);
            if ((h223caps->option_of_nsrpSupport == ON) && (h223caps->nsrpSupport == ON))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "TSC_324m: Remote video caps Option of NSRP support is ON\n"));

                // switch to NSRP if mux level == 0.  If level was 1 or 2, we would have switched after level setup
                if (iH223->GetMuxLevel() == 0)
                {
                    iSrp->UseNSRP(true);
                }
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "TSC_324m: Remote video caps Option of NSRP support is OFF\n"));
            }

            if (h223caps->option_of_mobileOperationTransmitCapability)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "TSC_324m: Remote video caps Option of Mobile Transmit Capability is ON\n"));
                struct _MobileOperationTransmitCapability* mobile_caps = &h223caps->mobileOperationTransmitCapability;
                if (iTransmitCaps)
                    OSCL_DEFAULT_FREE(iTransmitCaps);
                iTransmitCaps = (struct _MobileOperationTransmitCapability*)OSCL_DEFAULT_MALLOC(sizeof(struct _MobileOperationTransmitCapability));
                oscl_memcpy(iTransmitCaps, mobile_caps, sizeof(struct _MobileOperationTransmitCapability));
            }

            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "TSC_324m::ExtractTcsParameters option_of_maxMUXPDUSizeCapability=%d",
                             h223caps->option_of_maxMUXPDUSizeCapability));
            if (h223caps->option_of_maxMUXPDUSizeCapability)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "TSC_324m: Remote maxMuxPduCapability - %d\n",
                                 h223caps->maxMUXPDUSizeCapability));
                iMaxMuxPduCapabilityR = h223caps->maxMUXPDUSizeCapability ? true : false;
                unsigned size = iRequestMaxMuxPduSize;
                if (iH223->GetMuxLevel() == H223_LEVEL2)
                    size = size > H223_MAX_MUX_PDU_SIZE_LEVEL2 ? H223_MAX_MUX_PDU_SIZE_LEVEL2 : size;
                if (iMaxMuxPduCapabilityR && size)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                    (0, "TSC_324m::ExtractTcsParameters Requesting max mux pdu size (%d) from remote",
                                     size));
                    RequestMaxMuxPduSize(size);
                }
            }
        }
    }

    if (pTcs->option_of_capabilityTable)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_324m: Remote video caps Option of Capability Table is ON, size(%d)\n",
                         pTcs->size_of_capabilityTable));
        for (i = 0;i < pTcs->size_of_capabilityTable;++i)
        {
            pCapEntry = pTcs->capabilityTable + i;
            if (pCapEntry->option_of_capability)
            {
                iTSCcapability.ParseTcsCapabilities(pCapEntry->capability, capabilityItems, userInputCapabilities, userInputCapability);
            }

        }
    }
    iTSCcapability.CreateNewCapability(capabilityItems);
    userInputCapabilities = userInputCapabilities ?
                            userInputCapabilities : 1 << PV2WAY_DEFAULT_USER_INPUT_CAPABILITY_INDEX;
    if (iTSC_324mObserver)
    {
        iTSC_324mObserver->UserInputCapability(userInputCapabilities);
    }
}

