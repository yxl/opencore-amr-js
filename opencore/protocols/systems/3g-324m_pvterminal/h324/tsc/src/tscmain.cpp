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
#include "oscl_base.h"
#include "oscl_mem.h"
#include "osclconfig_compiler_warnings.h"
#include "tscmain.h"
#include "h223types.h"
#include "h223_api.h"
#include "h223.h"
#include "cpvh223multiplex.h"
#include "layer.h"
#include "tsc_eventreceive.h"
#include "tsc_h324m_config.h"
#include "tsc_statemanager.h"
#include "tsc_lc.h"
#include "tsc_blc.h"
#include "tsc_clc.h"
#include "tsc_constants.h"
#include "tsc_mt.h"
#include "tsc_component.h"
#include "tsc_capability.h"
#include "tsc_channelcontrol.h"
#ifdef MEM_TRACK
#include "oscl_mem.h"
#include "oscl_mem_audit.h"
#endif
#define PV_2WAY_TSC_TIMER_ID "PV_2WAY_TSC_TIMER"
#define PV_2WAY_TSC_TIMER_INTERVAL 1 /* 1 second */

#define PV_2WAY_TSC_NUM_CMD_Q_ELEMENTS 10
#define PV_2WAY_TSC_NUM_PENDING_CMD_Q_ELEMENTS 10

#define TSC_MAX_CONTROL_CHANNEL_BITRATE_BPS 64000
#define TSC_BIG_UINT32 0XFFFFFFFF
#define TSC_H223_LEVEL_DEFAULT H223_LEVEL2
#define DEFAULT_TCS_RECEIVE_TIMEOUT_SECONDS 20
#define DEFAULT_TCS_LEVEL_SETUP_TIMEOUT_SECONDS 6
#define DEFAULT_RTD_TIMER_SECONDS 7


// Default command queue reserve size
#define PVMF_CMD_QUEUE_RESERVE 10

// Starting value for command IDs
#define PVMF_CMD_ID_START 10000

// the assumed minimum T401 timer interval for sizing memory pools
#define PV2WAY_TSC_MIN_T401_INCOMING 100

OSCL_EXPORT_REF TSC_324m::TSC_324m(TPVLoopbackMode aLoopbackMode)
        :   TSC(),
        OsclActiveObject(OsclActiveObject::EPriorityHigh, "TSC"),
        Msd(NULL),
        Ce(NULL),
        iSrp(NULL),
        iH245(NULL),
        iH223(NULL),
        iVendor(NULL),
        iProductNumber(NULL),
        iVersionNumber(NULL),
        iVendorR(NULL),
        iProductNumberR(NULL),
        iVersionNumberR(NULL),
        iTransmitCaps(NULL),
        iSuppInfo(NULL),
        iSuppInfoLen(0),
        iLoopbackMode(aLoopbackMode),
        iTimer(NULL),
        iOutgoingSrpPort(NULL),
        iIncomingSrpPort(NULL),
        iTscSrpBuffer(NULL),
        iLogger(NULL),
        iTSC_324mObserver(NULL),
        iTSCcomponent(NULL),
        iTSCblc(iTSCstatemanager),
        iTSClc(iTSCstatemanager),
        iTSCcapability(iTSCstatemanager),
        iComponentRegistry(iTSCstatemanager,
                           iTSCcapability,
                           iTSClc,
                           iTSCblc,
                           iTSCclc,
                           iTSCmt)
{
    iLogger = PVLogger::GetLoggerObject("3g324m.h245user");

    iTSCcomponent = NULL;

    AddToScheduler();

    SetDispatchTable();

    iDisconnectInitiator = EPVT_NONE;
    iConnectFailReason = EPVT_Failed;

    iOutgoingSrpPort = NULL;
    iIncomingSrpPort = NULL;

    iTscSrpBuffer = NULL;
    iCurrentCmdId = 1;
    iInitialized = false;
    iInitializedComponent = false;
    iEnableWnsrp = true;
    iCmdQueue.Construct(PVMF_CMD_ID_START, PVMF_CMD_QUEUE_RESERVE);
    iPendingCmdQueue.Construct(PVMF_CMD_ID_START, PVMF_CMD_QUEUE_RESERVE);
}

TSC_324m::~TSC_324m()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::~TSC_324m"));
    ResetTsc();
}

void TSC_324m::SetInitialValues()
{
    initVarsLocal();
    initVarsSession();
    iInitialized = true;
}

void TSC_324m::initVarsLocal()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::initVarsLocal"));

    if (iTransmitCaps)
    {
        OSCL_DEFAULT_FREE(iTransmitCaps);
        iTransmitCaps = NULL;
    }


    iSendRme = false;
    iCsupSeq = 0;
    iRequestMaxMuxPduSize = H223_MAX_DEMUX_PDU_SIZE;

    iPendingCmdQueue.Construct(1, 5);

    iOutgoingPduType = H223_PDU_COMBINED;

    iEndSessionTimeout = DEFAULT_END_SESSION_TIMEOUT;

    iTimerValues.iT101 = iTimerValues.iT103 = iTimerValues.iT104 =
                             iTimerValues.iT105 = iTimerValues.iT106 = iTimerValues.iT107 =
                                                      iTimerValues.iT108 = iTimerValues.iT109 = TSC_SE_TIMEOUT_DEFAULT;
    iT401 = T401_DEFAULT; /* SRP */
    iN100 = N100_DEFAULT; /* H245 */
    iN401 = N401_DEFAULT; /* SRP */
    iCeRetries = iN100; /* Num CE retries left */

    iEnableWnsrp = true;

    iMultiplexingDelayMs = 0;
    iInLogicalChannelBufferingMs = 0;
    iOutLogicalChannelBufferingMs = 0;

    iTSCstatemanager.InitVarsLocal();
    iTSCcapability.InitVarsLocal();
    if (iTSCcomponent)
    {
        iTSCcomponent->InitVarsLocal();
    }
}

void TSC_324m::initVarsSession()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::initVarsSession"));
    iTerminalStatus = Phase0_Idle;		/* Terminal Status */


    iDisconnectInitiator = EPVT_NONE;
    iConnectFailReason = EPVT_Failed;

    iMaxMuxPduCapabilityR = false;

    /* Terminal ID for vendor identification */
    /* Local terminal id */
    if (iVendorR)
    {
        OSCL_DELETE(iVendorR);
    }
    if (iProductNumberR)
    {
        OSCL_DEFAULT_FREE(iProductNumberR);
    }
    if (iVersionNumberR)
    {
        OSCL_DEFAULT_FREE(iVersionNumberR);
    }
    iVendorR = NULL;
    iProductNumberR = NULL;
    iProductNumberLenR = 0;
    iVersionNumberR = NULL;
    iVersionNumberLenR = 0;

    iH223Level = TSC_H223_LEVEL_DEFAULT;

    iMuxTableUpdateRequired = false;

    if (iSuppInfo)
    {
        OSCL_DEFAULT_FREE(iSuppInfo);
        iSuppInfo = NULL;
        iSuppInfoLen = 0;
    }


    iCeRetries = iN100; /* Num CE retries left */


    /* Initialized RTD values */
    iNumRtdRequests = 0;
    iRtdMin = TSC_BIG_UINT32;
    iRtdMax = 0;
    iRtdAve = 0;

    iTSCstatemanager.InitVarsSession();
    iTSCmt.InitVarsSession();
    iTSCcapability.InitVarsSession();
    if (iTSCcomponent)
    {
        iTSCcomponent->InitVarsSession();
    }
}

void TSC_324m::IgnoreH245Callbacks()
{
    if (!iH245)
    {
        return;
    }
    iH245->SetObserver(NULL);
    Msd->SetObserver(NULL);
    Ce->SetObserver(NULL);
}

void TSC_324m::InitComponent()
{
#ifdef MEM_TRACK
    printf("\nMemory Stats before TSC Component Init\n");
    MemStats();
#endif
    if (!iTSCcomponent)
    {
        PVInterface *componentInterface;
        // QueryInterface sets iTSCcomponent
        QueryInterface(0, PVUuidH324ComponentInterface, (PVInterface*&)componentInterface);
        if (!iTSCcomponent)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                            (0, "TSC_324m::InitComponent- unable to create component"));
            OSCL_LEAVE(PVMFFailure);
        }
        iTSCcomponent->InitVarsLocal();
        iTSCcomponent->InitVarsSession();
    }
    if (iTSCcomponent &&
            !iInitializedComponent &&
            iH245 &&
            iH223)
    {
        iTSCcomponent->SetMembers(iH245, iH223, iObserver);
        iTSCcomponent->InitTsc();
        iInitializedComponent = true;
    }
#ifdef MEM_TRACK
    printf("\nMemory Stats after TSC Component Init\n");
    MemStats();
#endif
}

TPVStatusCode TSC_324m::InitTsc()
{
#ifdef MEM_TRACK
    printf("\nMemory Stats before TSC Init\n");
    MemStats();
#endif
    int error;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC: InitTsc"));

    if (!iInitialized)
    {
        SetInitialValues();
    }

    // Allocate stack elements.
    iH245 = OSCL_NEW(H245, ());
    iH245->SetObserver(this);

    iSrp = OSCL_NEW(SRP, ());
    iSrp->SetObserver(this);

    iH223 = OSCL_NEW(CPVH223Multiplex, (iLoopbackMode));
    iH223->SetObserver(this);



    iTSClc.SetH245(iH245);
    iTSCblc.SetH245(iH245);
    iTSCclc.SetH245(iH245);

    InitComponent();


    iTSCcapability.SetMembers(iTSCcomponent);
    iTSCmt.SetMembers(iH245, iH223, iTSCcomponent);
    iH223->SetClock(iClock);
    iH223->Open();

    // set direct connection to MSD SE
    Msd = iH245->GetMSD();
    Msd->SetObserver(this);

    // set direct connection to CE SE
    Ce = iH245->GetCE();
    Ce->SetObserver(this);

    OSCL_TRY(error, iTscSrpBuffer = TscSrpBuffer::NewL());
    if (error)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "TSC: InitTsc, tscsrpbuffer allocation failed", error));
    }

    iTimer = OSCL_NEW(OsclTimer<OsclMemAllocator>,
                      (PV_2WAY_TSC_TIMER_ID, PV_2WAY_TSC_TIMER_INTERVAL));
    //OSCL_TRAP_NEW(iTimer, OsclTimer<OsclMemAllocator>, (PV_2WAY_TSC_TIMER_ID, PV_2WAY_TSC_TIMER_INTERVAL) );
    iTimer->SetObserver(this);

#ifdef MEM_TRACK
    printf("\nMemory Stats after TSC Init\n");
    MemStats();
#endif

    return EPVT_Success;
}

TPVStatusCode TSC_324m::ResetTsc()
{
    TPVStatusCode ret = EPVT_Success;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC: Reset request."));

    if (iTSCcomponent)
    {
        iTSCcomponent->ResetTsc();
        iTSCstatemanager.WriteState(TSC_CE_RECEIVE, NOT_STARTED);
        iTSCstatemanager.WriteState(TSC_CE_SEND, NOT_STARTED);
        iInitializedComponent = false; // TSC Component
    }

    if (iSrp)
    {
        if (iSrp->RequestULPort(SRP_INPUT_PORT_TAG))
            iSrp->RequestULPort(SRP_INPUT_PORT_TAG)->Disconnect();

        if (iSrp->RequestULPort(SRP_OUTPUT_PORT_TAG))
            iSrp->RequestULPort(SRP_OUTPUT_PORT_TAG)->Disconnect();

        iSrp->SrpStop();
    }

    if (iTscSrpBuffer)
    {
        iTscSrpBuffer->GetLLPort(TSCSRPBUFFER_INPUT_PORT_TAG)->Disconnect();
        iTscSrpBuffer->GetLLPort(TSCSRPBUFFER_OUTPUT_PORT_TAG)->Disconnect();
    }

    if (iH245)
    {
        iH245->UnBind();
    }

    if (iIncomingSrpPort)
    {
        iIncomingSrpPort->Disconnect();
        iIncomingSrpPort = NULL;
    }

    if (iOutgoingSrpPort)
    {
        iOutgoingSrpPort->Disconnect();
        iOutgoingSrpPort = NULL;
    }

    if (iH223)
    {
        H223IncomingChannelPtr incoming_control_channel;
        PVMFStatus aStatus = iH223->GetIncomingChannel((TPVChannelId)0,
                             incoming_control_channel);
        if (aStatus == PVMFSuccess)
        {
            incoming_control_channel->Disconnect();
        }
        else
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                            (0, "TSC_324m::ResetTsc Error - Failed to lookup incoming logical channel %d", 0));
        }

        H223OutgoingChannelPtr outgoing_control_channel;
        aStatus = iH223->GetOutgoingChannel((TPVChannelId)0, outgoing_control_channel);
        if (aStatus == PVMFSuccess)
        {
            outgoing_control_channel->Disconnect();
        }
        else
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                            (0, "TSC_324m::ResetTsc Error - Failed to lookup outgoing logical channel %d", 0));
        }
    }

    //Shutdown stack elements.

    if (iTransmitCaps)
    {
        OSCL_DEFAULT_FREE(iTransmitCaps);
        iTransmitCaps = NULL;
    }

    if (iVendor)
    {
        delete iVendor;
        iVendor = NULL;
    }

    if (iVendorR)
    {
        delete iVendorR;
        iVendorR = NULL;
    }

    if (iProductNumber)
    {
        OSCL_DEFAULT_FREE(iProductNumber);
        iProductNumber = NULL;
    }

    if (iProductNumberR)
    {
        OSCL_DEFAULT_FREE(iProductNumberR);
        iProductNumberR = NULL;
    }

    if (iVersionNumber)
    {
        OSCL_DEFAULT_FREE(iVersionNumber);
        iVersionNumber = NULL;
    }

    if (iVersionNumberR)
    {
        OSCL_DEFAULT_FREE(iVersionNumberR);
        iVersionNumberR = NULL;
    }


    if (iTimer)
    {
        iTimer->Clear();
        OSCL_DELETE(iTimer);
        iTimer = NULL;
    }


    if (iTSCcomponent)
    {
        OSCL_DELETE(iTSCcomponent);
        iTSCcomponent = NULL;
    }


    if (iH223)
    {
        iH223->Close();
        DeallocateH223Mux(iH223);
        iH223 = NULL;
    }

    if (iTscSrpBuffer)
    {
        OSCL_DELETE(iTscSrpBuffer);
        iTscSrpBuffer = NULL;
    }

    if (iSrp)
    {
        iSrp->SrpReset();
        OSCL_DELETE(iSrp);
        iSrp = NULL;
    }

    if (iH245)
    {
        OSCL_DELETE(iH245);
        iH245 = NULL;
    }

    iInitialized = false; //TSC node

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC: Reset request - done"));
#ifdef MEM_TRACK
    printf("\nMemory Stats after TSC Reset\n");
    MemStats();
#endif
    return ret;
}

TPVStatusCode TSC_324m::SetOutgoingBitrate(int32 bitrate)
{
    iH223->SetBitrate(bitrate);
    return EPVT_Success;
}

TPVStatusCode TSC_324m::SetTimerRes(uint32 timer_res)
{
    return iH223->SetTimerRes(timer_res);
}

TPVStatusCode TSC_324m::SetTerminalParam(CPVTerminalParam* params)
{
    CPVH324MParam* h324params = (CPVH324MParam*)params;

    iSendRme = h324params->iSendRme;
    iRequestMaxMuxPduSize = h324params->iRequestMaxMuxPduSize;
    iTSCcomponent->SetTerminalParam(*h324params);
    iTSCstatemanager.SetTerminalParam(params);
    return EPVT_Success;
}

CPVTerminalParam* TSC_324m::GetTerminalParam()
{
    CPVH223MuxParam h223Param;
    CPVH324MParam* h324param = new CPVH324MParam();
    iTSCcomponent->GetTerminalParam(*h324param);
    h324param->iMasterSlave = (TPVMasterSlave)iTSCstatemanager.ReadState(TSC_MSD_DECISION);
    h324param->SetH223Param(&h223Param);
    return 	h324param;
}

TPVStatusCode
TSC_324m::Connect(uint16 info_len, uint8* info_buf)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iLogger, PVLOGMSG_NOTICE,
                    (0, "TSC_324m::Connect"));
    TPVStatusCode ret = EPVT_Pending;

    iMuxTableUpdateRequired = false;
    iStopCmd = NULL;
    iTerminalStatus = Phase0_Idle;

    iTSCmt.ClearVars();

    // moved stuff to component

    if (!iTSCcomponent->Connect1LevelKnown())
    {
        iH223Level = H223_LEVEL_UNKNOWN;
    }
    ResetStats();

    iH245->Reset();
    iH245->SetTimers(iTimerValues);


    // set direct connection to MSD SE
    Msd = iH245->GetMSD();
    Msd->SetObserver(this);

    // set direct connection to CE SE
    Ce = iH245->GetCE();
    Ce->SetObserver(this);

    if (iSuppInfo)
    {
        OSCL_DEFAULT_FREE(iSuppInfo);
        iSuppInfo = NULL;
        iSuppInfoLen = 0;
    }

    if (info_len)
    {
        iSuppInfo = (uint8*)OSCL_DEFAULT_MALLOC(info_len);
        iSuppInfoLen = info_len;
        oscl_memcpy(iSuppInfo, info_buf, info_len);
    }
    iTimer->Clear();

    CPVH223MuxParam level_set;

    // Initialize the mux and wait for completion.  Performs level setup
    if (iH223Level != H223_LEVEL_UNKNOWN)
    {
        iH223->SetMultiplexLevel(iH223Level);
    }
    iH223->SetMultiplexingDelayMs((uint16)iMultiplexingDelayMs);
    iH223->SetLogicalChannelBufferingMs(iInLogicalChannelBufferingMs,
                                        iOutLogicalChannelBufferingMs);
    iH223->SetClock(iClock);

    S_H223LogicalChannelParameters lcnParams;
    lcnParams.segmentableFlag = true;
    lcnParams.adaptationLayerType.index = 1;
    H223ChannelParam in_channel_params(0,
                                       &lcnParams,
                                       TSC_MAX_CONTROL_CHANNEL_BITRATE_BPS,
                                       PV2WAY_TSC_MIN_T401_INCOMING);
    H223ChannelParam out_channel_params(0,
                                        &lcnParams,
                                        TSC_MAX_CONTROL_CHANNEL_BITRATE_BPS,
                                        iT401*100);

    iH223->OpenChannel(OUTGOING, 0, &out_channel_params);
    iH223->OpenChannel(INCOMING, 0, &in_channel_params);


    ConfigureSrp(iH223Level);

    // Reset global states (RAN-MS)
    iTSCstatemanager.StateInitialize();    // Reset call-setup states

    iTerminalStatus = PhaseD_CSUP;

    iTSCcomponent->Connect2();
    return ret;
}

void TSC_324m::SetClock(PVMFMediaClock* aClock)
{
    iClock = aClock;
}

void TSC_324m::DataReceptionStart()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::DataReceptionStart"));
    /* Start timer for Level Setup countdown */
    int32 levelSetupTimeoutInfo = 0;
    if (iTSCcomponent->IsEnabled())
    {
        levelSetupTimeoutInfo = 1; // Level setup can commence again if it fails the first time
    }
    iTimer->Request(PV_TSC_LEVEL_SETUP_TIMER_ID, levelSetupTimeoutInfo,
                    DEFAULT_TCS_LEVEL_SETUP_TIMEOUT_SECONDS, this);
}

void TSC_324m::MuxSetupComplete(PVMFStatus status, TPVH223Level level)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::MuxSetupComplete status=%d, level=%d",
                     status, level));
    iTimer->Cancel(PV_TSC_LEVEL_SETUP_TIMER_ID);

    /* Reconfigure SRP if multiplex level 0 is negotiated starting from a higher level */
    if (level == H223_LEVEL0 && level != iH223Level)
    {
        ConfigureSrp(level);
    }
    iH223Level = level;
    if (status == PVMFPending)
    {
        return;
    }
    if (status != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_324m::MuxSetupComplete Mux setup failed."));
        SignalCsupComplete(status);
        return;
    }

    iTSCcomponent->MuxSetupComplete(status, level);

    OpenSession();
    iTimer->Request(PV_TSC_RTD_TIMER_ID, PV_TSC_RTD_TIMER_ID, DEFAULT_RTD_TIMER_SECONDS, this, false);
    /* Start timer for TCS countdown */
    if (iTSCstatemanager.ReadState(TSC_CE_RECEIVE) != COMPLETE)
    {
        iTimer->Request(PV_TSC_TCS_RECEIVE_TIMER_ID, PV_TSC_TCS_RECEIVE_TIMER_ID,
                        DEFAULT_TCS_RECEIVE_TIMEOUT_SECONDS, this);
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_324m::MuxSetupComplete TCS received before MuxSetupComplete."));
    }
}

void TSC_324m::MuxCloseComplete()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m: Mux close complete - status(%d).", iTerminalStatus));

    /* All logical channels closed */
    iTerminalStatus = PhaseF_End;

    iH245->Reset();
    // set direct connection to MSD SE
    Msd = iH245->GetMSD();
    Msd->SetObserver(this);

    // set direct connection to CE SE
    Ce = iH245->GetCE();
    Ce->SetObserver(this);

    iH223->CloseChannel(OUTGOING, 0);
    iH223->CloseChannel(INCOMING, 0);
}

void TSC_324m::MuxErrorOccurred(TPVDirection direction,
                                TPVMuxComponent component,
                                PVMFStatus error,
                                uint8* info,
                                uint info_len)
{
    OSCL_UNUSED_ARG(info_len);
    OSCL_UNUSED_ARG(info);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::MuxErrorOccurred - direction(%d), component(%d), error(%d)",
                     direction, component, error));
    TPVChannelId id = CHANNEL_ID_UNKNOWN;
    if (info && component == PV_MUX_COMPONENT_LOGICAL_CHANNEL)
    {
        id = *((TPVChannelId*)info);

        if (!iTSCcomponent->HasOlc(direction, id))
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "TSC_324m::MuxErrorOccurred - Failed to lookup channel info, lcn=%d",
                             id));
            return;
        }
        if (direction == INCOMING)
        {
            H223IncomingChannelPtr incoming_lcn;
            PVMFStatus aStatus = iH223->GetIncomingChannel(id, incoming_lcn);
            if (aStatus == PVMFSuccess)
            {
                if (!incoming_lcn->IsConnected())
                {
                    RequestFrameUpdate(incoming_lcn);
                }
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "TSC_324m::MuxErrorOccurred - Failed to lookup incoming channel, lcn=%d",
                                 id));
                return;
            }
        }
    }

    PVMFAsyncEvent aEvent(PVMFInfoEvent, error, this, NULL);
    uint8* buf = aEvent.GetLocalBuffer();
    buf[0] = (uint8)PV_H324COMPONENT_H223;
    buf[1] = (uint8)component;
    buf[2] = (uint8)direction;
    if (component == PV_MUX_COMPONENT_LOGICAL_CHANNEL)
    {
        *((TPVChannelId*)(buf + 4)) = id;
    }
    //iInfoObserver->HandleNodeInformationalEvent(aEvent);
}

void TSC_324m::SkewDetected(TPVChannelId lcn1, TPVChannelId lcn2, uint32 skew)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::SkewDetected - lcn1=%d, lcn2=%d, skew=%d",
                     lcn1, lcn2, skew));
    Tsc_IdcSkew(lcn1, lcn2, (uint16)skew);
}

void TSC_324m::LcnDataDetected(TPVChannelId lcn)
{
    iTSCcomponent->LcnDataDetected(lcn);
}

void TSC_324m::ReceivedFormatSpecificInfo(TPVChannelId channel_id,
        uint8* fsi,
        uint32 fsi_len)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::ReceivedFormatSpecificInfo lcn=%d, len=%d",
                     channel_id, fsi_len));
    iTSCcomponent->ReceivedFormatSpecificInfo(channel_id, fsi, fsi_len);
}

TPVStatusCode TSC_324m::Disconnect()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC: Disconnect request."));
    //LogStats(OUTGOING);
    //LogStats(INCOMING);

    TPVStatusCode ret = EPVT_Success;

    iDisconnectInitiator |= EPVT_LOCAL;

    iTimer->Clear();

    StopSrp();
    iH223->Stop();

    MuxCloseComplete();
    while (iIncomingChannels.size())
    {
        H223ChannelParam* param = iIncomingChannels.back();
        iIncomingChannels.pop_back();
        delete param;
    }

    initVarsSession();
    return ret;
}

TPVStatusCode TSC_324m::Abort()
{
//	iMutex->Lock();
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC: Abort."));
    //LogStats(OUTGOING);
    //LogStats(INCOMING);
    TPVStatusCode ret = EPVT_Success;

    iDisconnectInitiator |= EPVT_LOCAL;

    if (iSuppInfo)
    {
        OSCL_DEFAULT_FREE(iSuppInfo);
        iSuppInfo = NULL;
        iSuppInfoLen = 0;
        iTimer->Clear();
    }

    SessionClose_Comm();

    iSrp->SrpStop();

    /* All logical channels closed */
    iTerminalStatus = PhaseF_End;

    iH245->Reset();
    // set direct connection to MSD SE
    Msd = iH245->GetMSD();
    Msd->SetObserver(this);

    // set direct connection to CE SE
    Ce = iH245->GetCE();
    Ce->SetObserver(this);

    initVarsSession();

//	iMutex->Unlock();
    return ret;
}

/* Handle control events depending on state */
/*
        Phase0_Idle = 1 ,
        PhaseA          ,
        PhaseB          ,
        PhaseC          ,
		PhaseD_CSUP		,  // Call Setup
        PhaseE_Comm     ,  // Ongoing Communication
        PhaseF_Clc      ,  // Closing all outgoing LCNs
        PhaseF_End      ,  // End of Session
        PhaseG_Dis
*/
void TSC_324m::Handle(PS_ControlMsgHeader msg)
{
    uint32            EventNo;                        /* EventNo         */
    uint32            Cnt;                            /* Counter         */
    DISPATCH_PTR func_ptr = NULL;
    bool handled = false;

//	iMutex->Lock();

    /* Event Receive; a non blocking call */
    EventNo = Tsc_EventReceive(msg);

    for (Cnt = 0; iDispatchTable[Cnt].Module != NULL; Cnt++)
    {
        if (iDispatchTable[Cnt].Status == iTerminalStatus && iDispatchTable[Cnt].Event == EventNo)
        {
            func_ptr = iDispatchTable[Cnt].Module;
            handled = true;
            (this->*func_ptr)(msg);
            break;
        }
    }
    if (!handled)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_324m: Failed to handle control message - state(%d), event no(%d).",
                         iTerminalStatus, EventNo));
    }

    return;
}

TPVStatusCode TSC_324m::RequestFrameUpdate(PVMFPortInterface* port)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::RequestFrameUpdate"));

    if (iTSCstatemanager.ReadState(TSC_CE_SEND) == NOT_STARTED)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_324m::RequestFrameUpdate ERROR  - TCS not yet requested, tcs state=NOT_STARTED"));
        return EPVT_ErrorInvalidState;
    }

    H223IncomingChannel *channel = OSCL_STATIC_CAST(H223IncomingChannel *, port);
    if (channel == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC: RequestFrameUpdate - Null port"));
        return EPVT_Failed;
    }
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::RequestFrameUpdate lcn=%d", channel->GetLogicalChannelNumber()));
    OlcParam* param = iTSCcomponent->FindOlcGivenChannel(INCOMING, channel->GetLogicalChannelNumber());
    if (param == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_324m: RequestFrameUpdate - Failed to lookup channel param"));
        return EPVT_Failed;
    }
    if (param->GetState() != OLC_ESTABLISHED)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_324m: RequestFrameUpdate - Channel not in established state.  Not sending request."));
        return EPVT_Failed;
    }

    CmdMisc(EVideoFastUpdatePicture, channel->GetLogicalChannelNumber());
    return EPVT_Success;
}

TPVStatusCode TSC_324m::RequestMaxMuxPduSize(unsigned aPduSize)
{
    if ((iTerminalStatus != PhaseD_CSUP) && (iTerminalStatus != PhaseE_Comm))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_324m: ERROR - RequestMaxMuxPduSize - invalid state(%d)",
                         iTerminalStatus));
        return EPVT_ErrorInvalidState;
    }
    CmdMisc(EMaxH223MUXPDUsize, CHANNEL_ID_UNKNOWN&0xFFFF, aPduSize);
    return EPVT_Success;
}

OSCL_EXPORT_REF TPVStatusCode TSC_324m::SetTerminalType(uint8 aTType)
{
    if (Msd)
    {
        Msd->SetTerminalType(aTType);
        return EPVT_Success;
    }
    else
    {
        return EPVT_Failed;
    }
}

OSCL_EXPORT_REF void TSC_324m::SetEndSessionTimeout(uint32 timeout)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::SetEndSessionTimeout timeout(%d)", timeout));
    iEndSessionTimeout = timeout;
    if (iStopCmd != 0)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                        (0, "TSC_324m::SetEndSessionTimeout Already disconnectiing",
                         timeout));
    }
}

OSCL_EXPORT_REF void TSC_324m::SetTimerCounter(TPVH324TimerCounter aTimerCounter,
        uint8 aSeries, uint32 aSeriesOffset, uint32 aValue)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::SetTimerCounter timer/counter(%d), series(%d), offset(%d), value(%d)",
                     aTimerCounter, aSeries, aSeriesOffset, aValue));
    if (aTimerCounter == EH324Timer)
    {
        switch (aSeries)
        {
            case 1:
            {
                switch (aSeriesOffset)
                {
                    case 1:
                        iTimerValues.iT101 = aValue;
                        break;
                    case 3:
                        iTimerValues.iT103 = aValue;
                        break;
                    case 4:
                        iTimerValues.iT104 = aValue;
                        break;
                    case 5:
                        iTimerValues.iT105 = aValue;
                        break;
                    case 6:
                        iTimerValues.iT106 = aValue;
                        break;
                    case 7:
                        iTimerValues.iT107 = aValue;
                        break;
                    case 8:
                        iTimerValues.iT108 = aValue;
                        break;
                    case 9:
                        iTimerValues.iT109 = aValue;
                        break;
                    default:
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                        (0, "TSC_324m::SetTimerCounter Error - Offset not supported for series"));
                }
            }
            break;
            case 4:
            {
                switch (aSeriesOffset)
                {
                    case 1:
                        iT401 = aValue;
                        break;
                    default:
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                        (0, "TSC_324m::SetTimerCounter Error - Offset not supported for series"));
                }
            }
            break;
            default:
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "TSC_324m::SetTimerCounter Error - Series not supported"));
        }
    }
    else if (aTimerCounter == EH324Counter)
    {
        switch (aSeries)
        {
            case 1:
            {
                switch (aSeriesOffset)
                {
                    case 0:
                        iN100 = aValue;
                        iTSCstatemanager.SetN100(iN100);
                        break;
                    default:
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                        (0, "TSC_324m::SetTimerCounter Error - Offset not supported for series"));
                }
            }
            break;
            case 4:
            {
                switch (aSeriesOffset)
                {
                    case 1:
                        iN401 = aValue;
                        break;
                    default:
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                        (0, "TSC_324m::SetTimerCounter Error - Offset not supported for series"));
                }
            }
            break;
            default:
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "TSC_324m::SetTimerCounter Error - Series not supported"));
        }
    }
}

OSCL_EXPORT_REF void TSC_324m::SetVideoResolutions(TPVDirection dir,
        Oscl_Vector<PVMFVideoResolutionRange, OsclMemAllocator>& resolutions)
{
    iTSCcapability.SetVideoResolutions(dir, resolutions);
}

void TSC_324m::SetDispatchTable()
{
    const uint16 sizeofDispatchTable = 56;
    S_DispatchTable     aDispatchTable[sizeofDispatchTable] =
    {
        /* ----------------------------------------- */
        /* ---------- CALL SETUP EVENTS ------------ */
        /* ----------------------------------------- */
        { OSCL_FUNCTION_PTR(TSC_324m::Status04Event22)   ,  PhaseD_CSUP  ,  22 }, //E_PtvId_Mt_Trf_Idc
        { OSCL_FUNCTION_PTR(TSC_324m::Status04Event23)   ,  PhaseD_CSUP  ,  23 }, //E_PtvId_Mt_Trf_Cfm
        { OSCL_FUNCTION_PTR(TSC_324m::Status04Event24)   ,  PhaseD_CSUP  ,  24 }, //E_PtvId_Mt_Rjt_Idc
        { OSCL_FUNCTION_PTR(TSC_324m::ModeRequestIndication)   ,  PhaseD_CSUP  ,  28 }, //E_PtvId_Mr_Trf_Idc
        { OSCL_FUNCTION_PTR(TSC_324m::EndSessionRecv)    ,  PhaseD_CSUP  ,  37 }, //E_PtvId_Cmd_Es_Cfm
        { OSCL_FUNCTION_PTR(TSC_324m::VendorIdRecv)      ,  PhaseD_CSUP,    56},
        { OSCL_FUNCTION_PTR(TSC_324m::UserInputIndicationRecv), PhaseD_CSUP, 57}, //E_PtvId_Idc_Ui_Cfm
        { OSCL_FUNCTION_PTR(TSC_324m::LcEtbIdc)          ,  PhaseD_CSUP  ,   9 }, //E_PtvId_Lc_Etb_Idc
        { OSCL_FUNCTION_PTR(TSC_324m::LcRlsIdc)          ,  PhaseD_CSUP  ,  11 }, //E_PtvId_Lc_Rls_Idc
        { OSCL_FUNCTION_PTR(TSC_324m::BlcEtbIdc)          ,  PhaseD_CSUP  ,   14 }, //E_PtvId_Lc_Etb_Idc
        { OSCL_FUNCTION_PTR(TSC_324m::BlcRlsIdc)          ,  PhaseD_CSUP  ,  16 }, //E_PtvId_Lc_Rls_Idc
        { OSCL_FUNCTION_PTR(TSC_324m::FunctionNotSupportedIndicationReceived)   ,  PhaseD_CSUP  ,  49}, //E_PtvId_Idc_Fns_Cfm
        { OSCL_FUNCTION_PTR(TSC_324m::FlowControlCommandReceived)   ,  PhaseD_CSUP  ,  53 }, //E_PtvId_Cmd_Fc
        { OSCL_FUNCTION_PTR(TSC_324m::SendTerminalCapabilitySet)   ,  PhaseD_CSUP  ,  52 }, //E_PtvId_Cmd_Stcs_Cfm
        { OSCL_FUNCTION_PTR(TSC_324m::MiscCmdRecv)       ,  PhaseD_CSUP  ,  54 }, //E_PtvId_Cmd_Mscl_Cfm
        { OSCL_FUNCTION_PTR(TSC_324m::MiscIndicationRecv), PhaseD_CSUP, 58}, //E_PtvId_Idc_Mscl_Cfm
        { OSCL_FUNCTION_PTR(TSC_324m::SkewIndicationRecv), PhaseD_CSUP, 59}, //E_PtvId_Idc_H223skw_Cfm
        { OSCL_FUNCTION_PTR(TSC_324m::FlowControlIndicationReceived)   ,  PhaseD_CSUP  ,  60 }, //E_PtvId_Cmd_Fc
//		{ InternalError_CSUP,  PhaseD_CSUP  ,  38 }, //H245_INTERNAL_ERROR
        /* ----------------------------------------- */
        /* --------- ONGOING COMM EVENTS ----------- */
        /* ----------------------------------------- */
        { OSCL_FUNCTION_PTR(TSC_324m::LcEtbIdc)          ,  PhaseE_Comm  ,   9 }, //E_PtvId_Lc_Etb_Idc
        { OSCL_FUNCTION_PTR(TSC_324m::LcRlsIdc)          ,  PhaseE_Comm  ,  11 }, //E_PtvId_Lc_Rls_Idc
        { OSCL_FUNCTION_PTR(TSC_324m::LcEtbCfm)          ,  PhaseE_Comm  ,  10 }, //E_PtvId_Lc_Etb_Cfm
        { OSCL_FUNCTION_PTR(TSC_324m::LcRlsCfm)          ,  PhaseE_Comm  ,  12 }, //E_PtvId_Lc_Rls_Cfm
        { OSCL_FUNCTION_PTR(TSC_324m::LcErrIdc)          ,  PhaseE_Comm  ,  13 }, //E_PtvId_Lc_Err_Idc
        { OSCL_FUNCTION_PTR(TSC_324m::BlcEtbIdc)         ,  PhaseE_Comm  ,  14 }, //E_PtvId_Blc_Etb_Idc
        { OSCL_FUNCTION_PTR(TSC_324m::BlcEtbCfm)         ,  PhaseE_Comm  ,  15 }, //E_PtvId_Blc_Etb_Cfm
        { OSCL_FUNCTION_PTR(TSC_324m::BlcEtbCfm2)        ,  PhaseE_Comm  ,  50 }, //E_PtvId_Blc_Etb_Cfm2
        { OSCL_FUNCTION_PTR(TSC_324m::BlcErrIdc)         ,  PhaseE_Comm  ,  18 }, //E_PtvId_Blc_Err_Idc
        { OSCL_FUNCTION_PTR(TSC_324m::BlcRlsIdc)         ,  PhaseE_Comm  ,  16 }, //E_PtvId_Blc_Rls_Idc
        { OSCL_FUNCTION_PTR(TSC_324m::BlcRlsCfm)         ,  PhaseE_Comm  ,  17 }, //E_PtvId_Blc_Rls_Cfm
        { OSCL_FUNCTION_PTR(TSC_324m::Status08Event19)   ,  PhaseE_Comm  ,  19 }, //E_PtvId_Clc_Cls_Idc
        { OSCL_FUNCTION_PTR(TSC_324m::Status08Event20)   ,  PhaseE_Comm  ,  20 }, //E_PtvId_Clc_Cls_Cfm
        { OSCL_FUNCTION_PTR(TSC_324m::Status08Event21)   ,  PhaseE_Comm  ,  21 }, //E_PtvId_Clc_Rjt_Idc
        { OSCL_FUNCTION_PTR(TSC_324m::Status04Event22)   ,  PhaseE_Comm  ,  22 }, //E_PtvId_Mt_Trf_Idc
        { OSCL_FUNCTION_PTR(TSC_324m::Status04Event23)   ,  PhaseE_Comm  ,  23 }, //E_PtvId_Mt_Trf_Cfm
        { OSCL_FUNCTION_PTR(TSC_324m::Status04Event24)   ,  PhaseE_Comm  ,  24 }, //E_PtvId_Mt_Rjt_Idc
        { OSCL_FUNCTION_PTR(TSC_324m::Status08Event25)   ,  PhaseE_Comm  ,  25 }, //E_PtvId_Rme_Send_Idc
        { OSCL_FUNCTION_PTR(TSC_324m::Status08Event26)   ,  PhaseE_Comm  ,  26 }, //E_PtvId_Rme_Send_Cfm
        { OSCL_FUNCTION_PTR(TSC_324m::Status08Event27)   ,  PhaseE_Comm  ,  27 }, //E_PtvId_Rme_Rjt_Idc
        { OSCL_FUNCTION_PTR(TSC_324m::ModeRequestIndication)   ,  PhaseE_Comm  ,  28 }, //E_PtvId_Mr_Trf_Idc
        { OSCL_FUNCTION_PTR(TSC_324m::Status08Event29)   ,  PhaseE_Comm  ,  29 }, //E_PtvId_Mr_Trf_Cfm
        { OSCL_FUNCTION_PTR(TSC_324m::Status08Event30)   ,  PhaseE_Comm  ,  30 }, //E_PtvId_Mr_Rjt_Idc
        { OSCL_FUNCTION_PTR(TSC_324m::Status08Event31)   ,  PhaseE_Comm  ,  31 }, //E_PtvId_Rtd_Trf_Cfm
        { OSCL_FUNCTION_PTR(TSC_324m::Status08Event32)   ,  PhaseE_Comm  ,  32 }, //E_PtvId_Rtd_Exp_Idc
        { OSCL_FUNCTION_PTR(TSC_324m::FunctionNotSupportedIndicationReceived)   ,  PhaseE_Comm ,  49}, //E_PtvId_Idc_Fns_Cfm
        { OSCL_FUNCTION_PTR(TSC_324m::SendTerminalCapabilitySet)   ,  PhaseE_Comm  ,  52 }, //E_PtvId_Cmd_Stcs_Cfm
        { OSCL_FUNCTION_PTR(TSC_324m::FlowControlCommandReceived)   ,  PhaseE_Comm  ,  53 }, //E_PtvId_Cmd_Fc_Cfm
        { OSCL_FUNCTION_PTR(TSC_324m::MiscCmdRecv)       ,  PhaseE_Comm  ,  54 }, //E_PtvId_Cmd_Mscl_Cfm
        { OSCL_FUNCTION_PTR(TSC_324m::Status08Event55)   ,  PhaseE_Comm  ,  55 }, //E_PtvId_Cmd_Hmr_Cfm
        { OSCL_FUNCTION_PTR(TSC_324m::EndSessionRecv)    ,  PhaseE_Comm  ,  37 }, //E_PtvId_Cmd_Es_Cfm
        { OSCL_FUNCTION_PTR(TSC_324m::VendorIdRecv)      ,  PhaseE_Comm  ,  56},
        { OSCL_FUNCTION_PTR(TSC_324m::UserInputIndicationRecv), PhaseE_Comm, 57}, //E_PtvId_Idc_Ui_Cfm
        { OSCL_FUNCTION_PTR(TSC_324m::MiscIndicationRecv), PhaseE_Comm, 58}, //E_PtvId_Idc_Mscl_Cfm
        { OSCL_FUNCTION_PTR(TSC_324m::SkewIndicationRecv), PhaseE_Comm, 59}, //E_PtvId_Idc_H223skw_Cfm
        { OSCL_FUNCTION_PTR(TSC_324m::FlowControlIndicationReceived)   ,  PhaseE_Comm,  60 }, //E_PtvId_Cmd_Fc
        /* ----------------------------------------- */
        /* --------- CLOSE and END EVENTS ---------- */
        /* ----------------------------------------- */
        { OSCL_FUNCTION_PTR(TSC_324m::EndSessionRecv)    ,  PhaseF_End   ,  37 }, //E_PtvId_Cmd_Es_Cfm

        { NULL                  ,  0            ,   0 }
    };

    for (int n = 0; n < sizeofDispatchTable; n++)
    {
        iDispatchTable[n] = aDispatchTable[n];
    }
}

OSCL_EXPORT_REF TPVStatusCode TSC_324m::SetVendorIdInfo(TPVH245Vendor* vendor,
        const uint8* pn,
        uint16 pn_len,
        const uint8* vn,
        uint16 vn_len)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::SetVendorIdInfo vendor(%x), pn_len(%d), vn_len(%d)", vendor, pn_len, vn_len));
    if (pn && pn_len && (pn[pn_len-1] == '\0'))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_324m::SetVendorIdInfo pn(%s)", pn));
    }
    if (vn && vn_len && (vn[vn_len-1] == '\0'))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_324m::SetVendorIdInfo vn(%s)", vn));
    }

    if (iVendor)
    {
        OSCL_DELETE(iVendor);
        iVendor = NULL;
    }
    if (iProductNumber)
    {
        OSCL_DEFAULT_FREE(iProductNumber);
        iProductNumber = NULL;
    }
    if (iVersionNumber)
    {
        OSCL_DEFAULT_FREE(iVersionNumber);
        iVersionNumber = NULL;
    }
    iProductNumberLen = pn_len;
    iVersionNumberLen = vn_len;

    if (vendor)
    {
        iVendor = vendor->Copy();
    }

    if (iProductNumberLen)
    {
        iProductNumber = (uint8*)OSCL_DEFAULT_MALLOC(iProductNumberLen);
        oscl_memcpy(iProductNumber, pn, iProductNumberLen);
    }

    if (iVersionNumberLen)
    {
        iVersionNumber = (uint8*)OSCL_DEFAULT_MALLOC(iVersionNumberLen);
        oscl_memcpy(iVersionNumber, vn, iVersionNumberLen);
    }
    return EPVT_Success;
}

OSCL_EXPORT_REF TSC* Allocate324mTSC(TPVLoopbackMode aLoopbackMode)
{
    return OSCL_NEW(TSC_324m, (aLoopbackMode));
}

OSCL_EXPORT_REF void Deallocate324mTSC(TSC* tsc)
{
    TSC_324m* tsc_324m = (TSC_324m*)tsc;
    OSCL_DELETE(tsc_324m);
}


void TSC_324m::TimeoutOccurred(int32 timerID, int32 timeoutInfo)
{
    OSCL_UNUSED_ARG(timeoutInfo);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::TimeoutOccurred"));

    if (timerID == PV_TSC_RTD_TIMER_ID)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_324m::TimeoutOccurred - Sending RTD"));
        iTscSrpBuffer->EnableBuffering(false);
        RtdTrfReq();
    }
    else if (timerID == PV_TSC_LEVEL_SETUP_TIMER_ID)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_324m::TimeoutOccurred Level setup timeout out, timeoutInfo=%d",
                         timeoutInfo));
        if (!timeoutInfo)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "TSC_324m::TimeoutOccurred Level Setup Failed."));
            SignalCsupComplete(PVMFErrTimeout);
        }
        else
        {
            /* We timed out waiting for TCS. */
            iTSCcomponent->Timeout();
            /* Start timer to receive TCS */
            iTimer->Request(PV_TSC_TCS_RECEIVE_TIMER_ID, PV_TSC_TCS_RECEIVE_TIMER_ID,
                            DEFAULT_TCS_RECEIVE_TIMEOUT_SECONDS, this);
            iH223->EnableStuffing(true);
            iH223->SetInterleavingMultiplexFlags(0, NULL);
            iH223->SetMuxLevel(TSC_H223_LEVEL_DEFAULT);
            /* Start timer to receive TCS */
            iTimer->Request(PV_TSC_LEVEL_SETUP_TIMER_ID, 0,
                            DEFAULT_TCS_LEVEL_SETUP_TIMEOUT_SECONDS, this);
        }
    }
    else if (timerID == PV_TSC_TCS_RECEIVE_TIMER_ID)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_324m::TimeoutOccurred Timed out on TCS receive.  Connect failed."));
        SignalCsupComplete(PVMFErrTimeout);
    }
    else
    {
        iTSCcomponent->TimeoutOccurred(timerID, timeoutInfo);
    }
}


void TSC_324m::ResetStats()
{
    iH223->ResetStats();
}
void TSC_324m::LogStats(TPVDirection dir)
{
    if (iH223)
        iH223->LogStats(dir);
}

// Node virtuals
// Node virtuals
// Sync requests that will NOT trigger callback events

OSCL_EXPORT_REF  PVMFStatus TSC_324m::GetCapability(PVMFNodeCapability& aNodeCapability)
{
    aNodeCapability.iCanSupportMultipleInputPorts = true;
    aNodeCapability.iCanSupportMultipleOutputPorts = true;
    aNodeCapability.iHasMaxNumberOfPorts = true;
    aNodeCapability.iMaxNumberOfPorts = 5;
    aNodeCapability.iInputFormatCapability.push_back(PVMF_MIME_H223);
    aNodeCapability.iInputFormatCapability.push_back(PVMF_MIME_M4V);
    aNodeCapability.iInputFormatCapability.push_back(PVMF_MIME_H2631998);
    aNodeCapability.iInputFormatCapability.push_back(PVMF_MIME_H2632000);
    aNodeCapability.iOutputFormatCapability.push_back(PVMF_MIME_H223);
    aNodeCapability.iOutputFormatCapability.push_back(PVMF_MIME_M4V);
    aNodeCapability.iOutputFormatCapability.push_back(PVMF_MIME_H2631998);
    aNodeCapability.iOutputFormatCapability.push_back(PVMF_MIME_H2632000);
    return PVMFSuccess;
}

OSCL_EXPORT_REF  PVMFCommandId TSC_324m::Init(PVMFSessionId aSession, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::Init(%x)", aContext));

    if (iInterfaceState != EPVMFNodeIdle)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                        (0, "TSC_324m::Init Invalid state(%d)", iInterfaceState));
        OSCL_LEAVE(PVMFErrInvalidState);
    }
    // Add command to queue
    Tsc324mNodeCommand cmd;
    cmd.Construct(aSession, PVMF_GENERIC_NODE_INIT, aContext);
    return QueueCommandL(cmd);
}

// Async requests that will trigger callback events
/**
This should establish the comm server session.  Thread specific initialization happens here */
void TSC_324m::DoInit(Tsc324mNodeCommand& cmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                    (0, "TSC_324m::DoInit(%x)", cmd.iId));

    PVMFStatus pvmfStatus = PVMFSuccess;
    // Initialize TSC
    int leave_status = 0;
    OSCL_TRY(leave_status, InitTsc());
    OSCL_FIRST_CATCH_ANY(leave_status, void());

    if (leave_status == 0)
    {
        iInterfaceState = EPVMFNodeInitialized;
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                        (0, "TSC_324m::Init - failed to initialize"));
        pvmfStatus = PVMFFailure;
        iInterfaceState = EPVMFNodeError;
    }

    CommandComplete(iCmdQueue, cmd, pvmfStatus, NULL);
}

PVMFCommandId TSC_324m::Prepare(PVMFSessionId aSession
                                , const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::Prepare aSession=%x, aContext=%x",
                     aSession, aContext));
    if (iInterfaceState != EPVMFNodeInitialized)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                        (0, "TSC_324m::Prepare Invalid state(%d)", iInterfaceState));
        OSCL_LEAVE(PVMFErrInvalidState);
    }
    // Add command to queue
    Tsc324mNodeCommand cmd;
    cmd.Construct(aSession, PVMF_GENERIC_NODE_PREPARE, aContext);
    return QueueCommandL(cmd);
}

void TSC_324m::DoPrepare(Tsc324mNodeCommand& cmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::DoPrepare"));
    iInterfaceState = EPVMFNodePrepared;
    CommandComplete(iCmdQueue, cmd, PVMFSuccess);
}

OSCL_EXPORT_REF  PVMFCommandId TSC_324m::Start(PVMFSessionId aSession, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::Start aSession=%x, aContext=%x", aSession, aContext));

    iTSCcomponent->Start();

    if (iInterfaceState != EPVMFNodePrepared || iTerminalStatus != Phase0_Idle)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                        (0, "TSC_324m::Start Init Invalid state iInterfaceState=%d, iTerminalStatus=%d",
                         iInterfaceState, iTerminalStatus));
        OSCL_LEAVE(PVMFErrInvalidState);
    }

    // Add command to queue
    Tsc324mNodeCommand cmd;
    cmd.Construct(aSession, PVMF_GENERIC_NODE_START, aContext);
    return QueueCommandL(cmd);
}

void TSC_324m::DoStart(Tsc324mNodeCommand& cmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::DoStart"));
    TPVStatusCode stat = EPVT_Failed;
    int leave_status = 0;
    PVMFStatus pvmfStatus = PVMFFailure;

    OSCL_TRY(leave_status, stat = Connect());
    OSCL_FIRST_CATCH_ANY(leave_status, Disconnect());
    if (stat == EPVT_Success || stat == EPVT_Pending)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_324m::DoStart Connect complete"));
        pvmfStatus = PVMFSuccess;
        iInterfaceState = EPVMFNodeStarted;
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_324m::DoStart Connect failed leave_status=%d",
                         leave_status));
        iInterfaceState = EPVMFNodePrepared;
    }

    CommandComplete(iCmdQueue, cmd, pvmfStatus);
    return;

}

/**
 * Causes the node to stop servicing one or all disconnected ports.
 * TBD:  Should we add an API or modify this to take an arbitrary subset of ports ?
 * @exception port_connected leaves if any of the ports are not disconnected.
 **/
OSCL_EXPORT_REF  PVMFCommandId TSC_324m::Stop(PVMFSessionId aSession,
        const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::Stop aSession=%x, aContext=%x",
                     aSession, aContext));
    switch (iInterfaceState)
    {
        case EPVMFNodeInitialized:
        case EPVMFNodePrepared:
        case EPVMFNodeStarted:
        case EPVMFNodePaused:
            // ok to call stop
            break;
        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                            (0, "TSC_324m::Stop Invalid state(%d)", iInterfaceState));
            OSCL_LEAVE(PVMFErrInvalidState);
    }

    while (!iCmdQueue.empty())
    {
        CommandComplete(iCmdQueue, iCmdQueue.front(), PVMFErrCancelled);
        iCmdQueue.Erase(&iCmdQueue.front());
    }

    iPendingCmdQueue.clear();
    // Add command to queue
    Tsc324mNodeCommand cmd;
    cmd.Construct(aSession, PVMF_GENERIC_NODE_STOP, aContext);
    return QueueCommandL(cmd);
}



void TSC_324m::DoStop(Tsc324mNodeCommand& cmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::DoStop"));

    switch (iInterfaceState)
    {
        case EPVMFNodeInitialized:
        case EPVMFNodePrepared:
        case EPVMFNodeStarted:
        case EPVMFNodePaused:
            break;
        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "TSC_324m::DoStop Ignoring stop due to invalid state(%d)",
                             iInterfaceState));
            CommandComplete(iCmdQueue, cmd, PVMFErrInvalidState);
            return;
    }

    if (iTSCcomponent)
    {
        iTSCcomponent->StartDisconnect(true);
    }

    iTerminalStatus = PhaseF_Clc;
    Disconnect();

    if (iNumRtdRequests)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_324m::DoStop iNumRtdRequests=%d, iRtdMin=%d, iRtdmax=%d, iRtdAve=%d",
                         iNumRtdRequests, iRtdMin, iRtdMax, iRtdAve / iNumRtdRequests));
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_324m::DoStop No RTD requests issued"));
    }

    iInterfaceState = EPVMFNodePrepared;
    CommandComplete(iCmdQueue, cmd, PVMFSuccess);
}

/**
 * Causes the node to pause servicing one or all connected and started ports.
 * Ports are to be resumed using the Start command ?
 * TBD:  Should we add an API or modify this to take an arbitrary subset of ports ?
 * @exception port_disconnected leaves if any of the ports are not connected.
 **/
OSCL_EXPORT_REF  PVMFCommandId TSC_324m::Pause(PVMFSessionId aSession,
        const OsclAny* aContext)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aContext);
    OSCL_LEAVE(PVMFErrNotSupported);
    return -1;
}

/**
 * Resets the node.  The node should relinquish all resources that is has acquired as part of the
 * initialization process and should be ready to be deleted when this completes.
 * @exception port_disconnected leaves if any of the ports are not connected.

 **/
OSCL_EXPORT_REF  PVMFCommandId TSC_324m::Reset(PVMFSessionId aSession,
        const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::Reset(%d)", iInterfaceState));

    switch (iInterfaceState)
    {
        case EPVMFNodeInitialized:
        case EPVMFNodePrepared:
        case EPVMFNodeError:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "TSC_324m::Reset Ok to reset"));
            break;
        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "TSC_324m::Reset Invalid state for Reset"));
            OSCL_LEAVE(PVMFErrInvalidState);
            break;
    }

    while (!iCmdQueue.empty())
    {
        CommandComplete(iCmdQueue, iCmdQueue.front(), PVMFErrCancelled);
        iCmdQueue.Erase(&iCmdQueue.front());
    }
    iPendingCmdQueue.clear();
    // Add command to queue
    Tsc324mNodeCommand cmd;
    cmd.Construct(aSession, PVMF_GENERIC_NODE_RESET, aContext);
    return QueueCommandL(cmd);
}

void TSC_324m::DoReset(Tsc324mNodeCommand& cmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::DoReset"));
    iInterfaceState = EPVMFNodeIdle;
    ResetTsc();
    CommandComplete(iCmdQueue, cmd, PVMFSuccess);
}

OSCL_EXPORT_REF PVMFCommandId TSC_324m::RequestPort(PVMFSessionId aSession
        , int32 aPortTag
        , const PvmfMimeString* aPortConfig
        , const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::RequestPort"));

    // Add command to queue
    Tsc324mNodeCommand cmd;
    cmd.Construct(aSession, PVMF_GENERIC_NODE_REQUESTPORT, aPortTag,
                  aPortConfig, aContext);
    return QueueCommandL(cmd);
}

PVMFPortInterface* TSC_324m::FindOutgoingPort(TPVMediaType_t mediaType)
{
    PVMFPortInterface* port = NULL;
    OlcParam* param = iTSCcomponent->FindOlc(OUTGOING,
                      mediaType, OLC_ESTABLISHED | OLC_PENDING);
    if (param)
    {
        uint32 channelID = param->GetChannelId();
        H223OutgoingChannelPtr outgoing_lcn;
        PVMFStatus pvmfStatus = iH223->GetOutgoingChannel(channelID,
                                outgoing_lcn);
        if (pvmfStatus == PVMFSuccess)
        {
            port = outgoing_lcn;
            iRequestedOutgoingChannels[port] = outgoing_lcn;
        }
    }
    return port;
}

void TSC_324m::DoRequestPort(Tsc324mNodeCommand& cmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::DoRequestPort"));
    PVMFStatus pvmfStatus = PVMFSuccess;
    int32 aPortTag = (int32)cmd.iParam1;

    PvmfMimeString* aPortConfig = (PvmfMimeString*)cmd.iParam2;
    PVMFPortInterface* port = NULL;
    if (aPortConfig)
    {
        PVMFFormatType formatType(aPortConfig->get_str());
        if (formatType.isFile())
        {

            // Multiplexed  PV_MULTIPLEXED
            // x-pvmf/multiplexed: to identify the lowerlayer port
            // Is the port already requested ?
            if (iH223->GetLowerLayer()->IsConnected())
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "TSC_324m::RequestPort - Port is in use"));
                pvmfStatus = PVMFErrBusy;
            }
            else
            {
                port = iH223->GetLowerLayer();
            }
        }
        else if (*aPortConfig == PVMF_AUDIO_OUTGOING_MIMETYPE)
        {
            // Audio x-pvmf/audio;dir=outgoing to identify the outgoing audio channel
            port = FindOutgoingPort(PV_AUDIO);
            if (port == NULL)
            {
                pvmfStatus = PVMFFailure;
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                                (0, "TSC_324m::DoRequestPort Error - Failed to lookup outgoing channel %d",
                                 aPortTag));
            }
        }
        else if (*aPortConfig == PVMF_VIDEO_OUTGOING_MIMETYPE)
        {
            // Video x-pvmf/video;dir=outgoing to identify the outgoing video channel
            port = FindOutgoingPort(PV_VIDEO);
            if (port == NULL)
            {
                pvmfStatus = PVMFFailure;
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                                (0, "TSC_324m::DoRequestPort Error - Failed to lookup outgoing channel %d",
                                 aPortTag));
            }
        }

        else if (*aPortConfig == PVMF_INCOMING_MIMETYPE || (aPortTag < 0))
        {
            if (aPortTag < 0)
            {
                aPortTag = -aPortTag;
            }
            // just use the channel id
            //x-pvmf/dir=incoming
            // Incoming
            H223IncomingChannelPtr incoming_lcn;
            pvmfStatus = iH223->GetIncomingChannel(aPortTag, incoming_lcn);
            if (pvmfStatus == PVMFSuccess)
            {
                port = incoming_lcn;
                iRequestedIncomingChannels[port] = incoming_lcn;
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                                (0, "TSC_324m::DoRequestPort Error - Failed to lookup incoming channel %d",
                                 aPortTag));
            }
        }

        else //if (*aPortConfig == PVMF_OUTGOING_MIMETYPE || aPortTag >=0)
        {
            //x-pvmf/dir=outgoing
            // Outgoing
            H223OutgoingChannelPtr outgoing_lcn;
            pvmfStatus = iH223->GetOutgoingChannel(aPortTag, outgoing_lcn);
            if (pvmfStatus == PVMFSuccess)
            {
                port = outgoing_lcn;
                iRequestedOutgoingChannels[port] = outgoing_lcn;
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                                (0, "TSC_324m::DoRequestPort Error - Failed to lookup outgoing channel %d",
                                 aPortTag));
            }
        }

    }

    if (pvmfStatus != PVMFPending)
    {
        CommandComplete(iCmdQueue, cmd, pvmfStatus, port);
        return;
    }
    // queue up the command in the pending queue
    iPendingCmdQueue.StoreL(cmd);
    iCmdQueue.Erase(&cmd);
}


OSCL_EXPORT_REF  PVMFCommandId TSC_324m::ReleasePort(PVMFSessionId aSession,
        PVMFPortInterface& aPort,
        const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::ReleasePort"));
    // Add command to queue
    Tsc324mNodeCommand cmd;
    cmd.Construct(aSession, PVMF_GENERIC_NODE_RELEASEPORT, aPort, aContext);
    return QueueCommandL(cmd);
}

void TSC_324m::DoReleasePort(Tsc324mNodeCommand& cmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::DoReleasePort"));

    PVMFPortInterface* port = (PVMFPortInterface*)cmd.iParam1;
    if (port->GetPortTag() == PV_MULTIPLEXED)
    {
        CommandComplete(iCmdQueue, cmd, PVMFSuccess, port);
        return;
    }

    H223LogicalChannel* lcn = OSCL_STATIC_CAST(H223LogicalChannel*, port);
    if (lcn)
    {
        TPVDirection aDirection = port->GetPortTag() < 0 ? INCOMING : OUTGOING;
        if (aDirection == INCOMING)
        {
            Oscl_Map<PVMFPortInterface*, H223IncomingChannelPtr, OsclMemAllocator>::iterator iter;
            iter = iRequestedIncomingChannels.find(port);
            if (iter != iRequestedIncomingChannels.end())
            {
                iRequestedIncomingChannels[port]->Flush();
                iH223->CloseChannel(INCOMING,
                                    iRequestedIncomingChannels[port]->GetLogicalChannelNumber());


                iRequestedIncomingChannels.erase(iter);
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "TSC_324m::DoReleasePort - Incoming channel not requested"));

            }
        }
        else
        {
            Oscl_Map<PVMFPortInterface*, H223OutgoingChannelPtr, OsclMemAllocator>::iterator iter;
            iter = iRequestedOutgoingChannels.find(port);
            if (iter != iRequestedOutgoingChannels.end())
            {
                iH223->FlushChannel(OUTGOING,
                                    iRequestedOutgoingChannels[port]->GetLogicalChannelNumber());
                iH223->CloseChannel(OUTGOING,
                                    iRequestedOutgoingChannels[port]->GetLogicalChannelNumber());
                iRequestedOutgoingChannels.erase(iter);
            }
        }
    }

    CommandComplete(iCmdQueue, cmd, PVMFSuccess, port);
    return;
}

/**
* Returns a list of ports currently available in the node that meet the filter criteria
* We can add fancier iterators and filters as needed.  For now we return all the ports.
**/
OSCL_EXPORT_REF  PVMFPortIter* TSC_324m::GetPorts(const PVMFPortFilter* aFilter)
{
    OSCL_UNUSED_ARG(aFilter);
    return NULL;
}

void TSC_324m::DoCancel()
{
}

void TSC_324m::Run()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, " TSC_324m::Run()"));
    if (!iCmdQueue.empty())
    {
        ProcessCommand(iCmdQueue.front());
    }

    if (!iCmdQueue.empty())
    {
        // Run again if there are more things to process
        RunIfNotReady();
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, " TSC_324m::Run() - done"));
}

bool TSC_324m::ProcessCommand(Tsc324mNodeCommand& aCmd)
{
    switch (aCmd.iCmd)
    {
        case PVMF_GENERIC_NODE_QUERYUUID:
            DoQueryUuid(aCmd);
            break;

        case PVMF_GENERIC_NODE_QUERYINTERFACE:
            DoQueryInterface(aCmd);
            break;

        case PVMF_GENERIC_NODE_REQUESTPORT:
            DoRequestPort(aCmd);
            break;

        case PVMF_GENERIC_NODE_RELEASEPORT:
            DoReleasePort(aCmd);
            break;

        case PVMF_GENERIC_NODE_INIT:
            DoInit(aCmd);
            break;

        case PVMF_GENERIC_NODE_PREPARE:
            DoPrepare(aCmd);
            break;

        case PVMF_GENERIC_NODE_START:
            DoStart(aCmd);
            break;

        case PVMF_GENERIC_NODE_STOP:
            DoStop(aCmd);
            break;

        case PVMF_GENERIC_NODE_FLUSH:
            DoFlush(aCmd);
            break;

        case PVMF_GENERIC_NODE_PAUSE:
            DoPause(aCmd);
            break;

        case PVMF_GENERIC_NODE_RESET:
            DoReset(aCmd);
            break;

        case PVMF_GENERIC_NODE_CANCELALLCOMMANDS:
            DoCancelAllCommands(aCmd);
            break;

        case PVMF_GENERIC_NODE_CANCELCOMMAND:
            DoCancelCommand(aCmd);
            break;

        default://unknown command type
            if (iTSCcomponent->ProcessCommand(aCmd))
            {
                CommandComplete(iCmdQueue, aCmd, PVMFSuccess);
            }
            else
            {
                CommandComplete(iCmdQueue, aCmd, PVMFFailure);
            }
            break;
    }

    return true;
}

int32 TSC_324m::FindPendingPortCmd(TPVMFGenericNodeCommand aCmdType,
                                   int32 aPortTag,
                                   Tsc324mNodeCommand& cmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::FindPendingRequestPortCmd aCmdType=%d, aPortTag=%d",
                     aCmdType, aPortTag));

    int32 cmdIndex = -1;

    for (uint32 i = 0; i < iPendingCmdQueue.size(); i++)
    {
        if ((TPVMFGenericNodeCommand)iPendingCmdQueue[i].iCmd == aCmdType)
        {
            if (aPortTag == (int32)iPendingCmdQueue[i].iParam1)
            {
                cmd = iPendingCmdQueue[i];
                cmdIndex = i;
                break;
            }
        }
    }

    if (cmdIndex < 0)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_324m::FindPendingPortCmd Error - command not found"));
        return 0;
    }

    // Destroy command object
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::FindPendingPortCmd : Deleting cmd from iPendingQueue. cmdIndex=%d",
                     cmdIndex));
    iPendingCmdQueue.Erase(&iPendingCmdQueue[cmdIndex]);
    return cmdIndex;
}

OSCL_EXPORT_REF void TSC_324m::SetMultiplexLevel(TPVH223Level muxLevel)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::SetMultiplexLevel  level(%d)\n", muxLevel));
    iH223Level = muxLevel;

    if (iH223)
    {
        iH223->SetMultiplexLevel(muxLevel);
    }
}

OSCL_EXPORT_REF void TSC_324m::SetSduSize(TPVDirection direction,
        uint16 size,
        TPVAdaptationLayer al)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::SetSduSize  direction(%d), size(%d), al(%d)\n",
                     direction, size, al));
    if (iH223)
    {
        iH223->SetSduSize(direction, size, EplForAdaptationLayer(al));
    }
}

OSCL_EXPORT_REF void TSC_324m::SetAl2Sn(int width)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::SetAl2Sn on/off(%d)\n", width));
    iTSCcomponent->SetAl2Sn(width);
}

OSCL_EXPORT_REF void TSC_324m::SetAl3ControlFieldOctets(unsigned cfo)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::SetAl3ControlFieldOctets cfo(%d)\n", cfo));
    iTSCcomponent->SetAl3ControlFieldOctets(cfo);
}

OSCL_EXPORT_REF void TSC_324m::SetMaxOutgoingPduSize(uint16 size)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::SetMaxOutgoingPduSize size(%d)\n", size));
    if (iH223)
    {
        iH223->SetMaxOutgoingPduSize(size);
    }
}

OSCL_EXPORT_REF void TSC_324m::SetMaxMuxPduSize(uint16 size)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::SetMaxMuxPduSize size(%d)\n", size));
    iRequestMaxMuxPduSize = size;
}

OSCL_EXPORT_REF void TSC_324m::SetMaxCcsrlSduSize(int size)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::SetMaxCcsrlSduSize size(%d)\n", size));
    if (iSrp)
    {
        iSrp->SetCCSRLSduSize(size);
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_324m::SetMaxCcsrlSduSize - Invalid state: SRP == NULL"));
    }
}

OSCL_EXPORT_REF void TSC_324m::SetOutgoingMuxPduType(TPVH223MuxPduType outgoingPduType)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::SetOutgoingMuxPduType type(%d)\n",
                     outgoingPduType));
    iOutgoingPduType = outgoingPduType;
}

OSCL_EXPORT_REF void TSC_324m::SetOutgoingChannelConfig(
    Oscl_Vector<H324ChannelParameters, PVMFTscAlloc>& out_channel_config)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::SetOutgoingChannelConfig size(%d)\n",
                     out_channel_config.size()));
    if (iInterfaceState != EPVMFNodeInitialized)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_324m::SetOutgoingChannelConfig Error: Invalid state."));
        OSCL_LEAVE(PVMFErrInvalidState);
    }
    iTSCcomponent->SetOutgoingChannelConfig(out_channel_config);
}

OSCL_EXPORT_REF void TSC_324m::SetIncomingChannelConfig(
    Oscl_Vector<H324ChannelParameters, PVMFTscAlloc>& in_channel_config)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::SetIncomingChannelConfig size(%d)\n",
                     in_channel_config.size()));
    iTSCcomponent->SetIncomingChannelConfig(in_channel_config);
}

OSCL_EXPORT_REF void TSC_324m::SetAlConfig(PV2WayMediaType media_type,
        TPVAdaptationLayer layer,
        bool allow)
{
    iTSCcomponent->SetAlConfig(media_type, layer, allow);
}

PVMFCommandId TSC_324m::QueryUUID(PVMFSessionId aSession,
                                  const PvmfMimeString& aMimeType,
                                  Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids,
                                  bool aExactUuidsOnly,
                                  const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::QueryUUID"));
    // create message to be completed later - depending on what it is...
    Tsc324mNodeCommand cmd;
    cmd.Construct(aSession, PVMF_GENERIC_NODE_QUERYUUID, aMimeType,
                  aUuids, aExactUuidsOnly, aContext);
    iComponentRegistry.QueryRegistry(cmd);
    //return QueueCommandL(cmd);
    return 0;
}

PVMFCommandId TSC_324m::QueryInterface(PVMFSessionId aSession,
                                       const PVUuid& aUuid,
                                       PVInterface*& aInterfacePtr,
                                       const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::QueryInterface()"));

    if (aUuid == PVH324MConfigUuid)
    {
        aInterfacePtr = OSCL_NEW(H324MConfig, (this, true));
    }
    else if (aUuid == PVUidProxiedInterface)
    {
        H324MProxiedInterface* proxied_interface_ptr = NULL;
        proxied_interface_ptr = OSCL_NEW(H324MProxiedInterface, ());
        proxied_interface_ptr->SetH324M(this);
        aInterfacePtr = proxied_interface_ptr;
    }
    else
    {
        TSC_component* backup = iTSCcomponent;

        // send onto TSCComponentRegistry::Create to let it finish
        iTSCcomponent = iComponentRegistry.Create(aSession, aUuid,
                        aInterfacePtr, aContext);
        if (iTSCcomponent)
        {
            InitComponent();
            if (backup)
            {
                OSCL_DELETE(backup);
            }
        }
    }

    // Add command to queue
    /*Tsc324mNodeCommand cmd;
    cmd.Construct(aSession, PVMF_GENERIC_NODE_QUERYINTERFACE, aUuid, aInterfacePtr, aContext) ;
    return QueueCommandL(cmd);
    */
    return 0;
}

void TSC_324m::DoQueryInterface(Tsc324mNodeCommand& cmd)
{
    OSCL_UNUSED_ARG(cmd);
    //CommandComplete(iCmdQueue, cmd, PVMFSuccess);
}

void TSC_324m::DoQueryUuid(Tsc324mNodeCommand& cmd)
{
    OSCL_UNUSED_ARG(cmd);
    //CommandComplete(iCmdQueue, cmd, PVMFSuccess);
}

PVMFCommandId TSC_324m::CancelAllCommands(PVMFSessionId aSession,
        const OsclAny* aContext)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aContext);
    OSCL_LEAVE(PVMFErrNotSupported);
    return -1;
}


OSCL_EXPORT_REF PVMFCommandId TSC_324m::CancelCommand(PVMFSessionId aSession,
        PVMFCommandId aCommandId,
        const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::CancelCommand id(%d)", aCommandId));
    // Add command to queue
    Tsc324mNodeCommand cmd;
    cmd.Construct(aSession, PVMF_GENERIC_NODE_CANCELCOMMAND, aCommandId, aContext);
    return QueueCommandL(cmd);
}

void TSC_324m::DoCancelCommand(Tsc324mNodeCommand& cmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::DoCancelCommand cmd(%x)", &cmd));
    PVMFStatus status = PVMFSuccess;
    int cmdIndex = FindPendingCmd((PVMFCommandId)cmd.iParam1);

    if (cmdIndex < 0)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_324m::DoCancelCommand Failed to lookup command(%d)",
                         (PVMFCommandId)cmd.iParam1));
        Tsc324mNodeCommand* found_cmd = iCmdQueue.FindById((PVMFCommandId)cmd.iParam1);
        if (found_cmd)
        {
            iCmdQueue.Erase(found_cmd);
        }
        else
            status = PVMFErrArgument;
    }
    else
    {
        // Destroy command object
        Tsc324mNodeCommand to_be_deleted_cmd = iPendingCmdQueue[cmdIndex];
        iPendingCmdQueue.Erase(&iPendingCmdQueue[cmdIndex]);
    }
    CommandComplete(iCmdQueue, cmd, status);
}

int TSC_324m::FindPendingCmd(PVMFCommandId aCommandId)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::FindPendingCmd id(%d)", aCommandId));
    int32 cmdIndex = -1;
    for (uint32 i = 0; i < iPendingCmdQueue.size(); i++)
    {
        if (iPendingCmdQueue[i].iId == aCommandId)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "TSC_324m::FindPendingCmd - command found in iPendingCmdQueue"));
            cmdIndex = i;
            break;
        }
    }
    return cmdIndex;
}

const uint8* TSC_324m::GetFormatSpecificInfo(PVMFPortInterface* port,
        uint32* len)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::GetFormatSpecificInfo port(%x)", port));
    return ((H223LogicalChannel*)port)->GetFormatSpecificInfo(len);
}

LogicalChannelInfo* TSC_324m::GetLogicalChannelInfo(PVMFPortInterface& port)
{
    return iTSCcomponent->GetLogicalChannelInfo(port);
}

bool TSC_324m::IsEstablishedLogicalChannel(TPVDirection aDir,
        TPVChannelId aChannelId)
{
    return iTSCcomponent->IsEstablishedLogicalChannel(aDir, aChannelId);
}

void TSC_324m::SetSkewReference(PVMFPortInterface* aPort,
                                PVMFPortInterface* aReferencePort)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::SetSkewReference"));
    H223OutgoingChannel* lcn2 = OSCL_STATIC_CAST(H223OutgoingChannel*,
                                aPort);
    H223OutgoingChannel* lcn1 = OSCL_STATIC_CAST(H223OutgoingChannel*,
                                aReferencePort);
    lcn2->SetSkewReference(lcn1);
}

void TSC_324m::SendVideoTemporalSpatialTradeoffCommand(TPVChannelId aLogicalChannel,
        uint8 aTradeoff)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::SendVideoTemporalSpatialTradeoffCommand aLogicalChannel=%d, aTradeoff=%d", aLogicalChannel, aTradeoff));
    if (iTerminalStatus != PhaseE_Comm)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_324m::SendVideoTemporalSpatialTradeoffCommand Ignoring due to invalid state."));
        return;
    }
    CmdMisc(EVideoTemporalSpatialTradeOff, aLogicalChannel, aTradeoff);
}

void TSC_324m::SendVideoTemporalSpatialTradeoffIndication(TPVChannelId aLogicalChannel,
        uint8 aTradeoff)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::SendVideoTemporalSpatialTradeoffIndication aLogicalChannel=%d, aTradeoff=%d", aLogicalChannel, aTradeoff));
    if (iTerminalStatus != PhaseE_Comm)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_324m::SendVideoTemporalSpatialTradeoffIndication Ignoring due to invalid state."));
        return;
    }
    IndicationMisc(EVideoTemporalSpatialTradeOffIdc, aLogicalChannel,
                   aTradeoff);
}

void TSC_324m::SetDatapathLatency(TPVDirection aDir, PVMFPortInterface* aPort,
                                  uint32 aLatency)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::SetDatapathLatency aDir=%d, aPort=%x, aLatency=%d", aDir, aPort, aLatency));
    OSCL_UNUSED_ARG(aDir);
    H223LogicalChannel* lcn = OSCL_STATIC_CAST(H223LogicalChannel* , aPort);
    if (lcn == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                        (0, "TSC_324m::SetDatapathLatency WARNING lcn==NULL"));
        return;
    }

    lcn->SetDatapathLatency(aLatency);
}

void TSC_324m::SendSkewIndication(TPVChannelId aLogicalChannel1,
                                  TPVChannelId aLogicalChannel2,
                                  uint16 aSkew)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::SendSkewIndication aLogicalChannel1=%d, aLogicalChannel2=%d, aSkew=%d",
                     aLogicalChannel1, aLogicalChannel2, aSkew));
    if (iTerminalStatus != PhaseE_Comm)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_324m::SendSkewIndication Ignoring due to invalid state."));
        return;
    }
    Tsc_IdcSkew(aLogicalChannel1, aLogicalChannel2, aSkew);
}

OSCL_EXPORT_REF void TSC_324m::SetMultiplexingDelayMs(uint16 aDelay)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::SetMultiplexingDelayMs,aDelay=%d", aDelay));
    iMultiplexingDelayMs = aDelay;
    if (iH223)
        iH223->SetMultiplexingDelayMs((uint16)iMultiplexingDelayMs);
}

void TSC_324m::ConfigureSrp(TPVH223Level aLevel)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::ConfigureSrp aLevel=%d", aLevel));
    iTscSrpBuffer->Stop();

    iSrp->SrpStop();
    iSrp->SrpReset();
    iSrp->SrpInitL();
    iSrp->SetSRPTimeoutValue(iT401);
    iSrp->SetNumSRPRetries(iN401);

    /* We will enable NSRP for level 0 if the remote terminal signals capability */
    if (aLevel == H223_LEVEL0)
    {
        iSrp->SetCCSRLSduSize(0);
        iSrp->UseNSRP(false);
    }
    else
    {
        iSrp->UseNSRP(true);
    }

    if (!iEnableWnsrp || aLevel == H223_LEVEL0)
    {
        iSrp->DisableWNSRPSupport();
    }

    H223IncomingChannelPtr incoming_control_channel;
    PVMFStatus aStatus = iH223->GetIncomingChannel((TPVChannelId)0,
                         incoming_control_channel);
    if (aStatus == PVMFSuccess)
    {
        incoming_control_channel->Disconnect();
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                        (0, "TSC_324m::ConfigureSrp Error - Failed to lookup incoming logical channel %d", 0));
        return;
    }

    H223OutgoingChannelPtr outgoing_control_channel;
    aStatus = iH223->GetOutgoingChannel((TPVChannelId)0,
                                        outgoing_control_channel);
    if (aStatus == PVMFSuccess)
    {
        outgoing_control_channel->Disconnect();
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                        (0, "TSC_324m::ConfigureSrp Error - Failed to lookup outgoing logical channel %d", 0));
        return;
    }

    PVMFPortInterface *port;

    port = iSrp->RequestULPort(SRP_INPUT_PORT_TAG);
    iTscSrpBuffer->GetLLPort(TSCSRPBUFFER_OUTPUT_PORT_TAG)->Connect(port);

    port = iSrp->RequestULPort(SRP_OUTPUT_PORT_TAG);
    port->Connect(iTscSrpBuffer->GetLLPort(TSCSRPBUFFER_INPUT_PORT_TAG));

    iTscSrpBuffer->GetUpperLayer()->Bind(iH245);

    iOutgoingSrpPort = iSrp->RequestLLPort(SRP_INPUT_PORT_TAG);
    iIncomingSrpPort = iSrp->RequestLLPort(SRP_OUTPUT_PORT_TAG);
    iOutgoingSrpPort->Connect(incoming_control_channel);
//	incoming_control_channel->Connect(iOutgoingSrpPort);
    iIncomingSrpPort->Connect(outgoing_control_channel);
//	outgoing_control_channel->Connect(iIncomingSrpPort);

    iTscSrpBuffer->Start();
    iSrp->SrpStart();
}

void TSC_324m::StopSrp()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::StopSrp "));
    iSrp->SrpStop();
    iTscSrpBuffer->Stop();

    H223IncomingChannelPtr incoming_control_channel;
    PVMFStatus aStatus = iH223->GetIncomingChannel((TPVChannelId)0,
                         incoming_control_channel);
    if (aStatus == PVMFSuccess)
    {
        incoming_control_channel->Disconnect();
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                        (0, "TSC_324m::ConfigureSrp Error - Failed to lookup incoming logical channel %d", 0));
    }

    H223OutgoingChannelPtr outgoing_control_channel;
    aStatus = iH223->GetOutgoingChannel((TPVChannelId)0,
                                        outgoing_control_channel);
    if (aStatus == PVMFSuccess)
    {
        outgoing_control_channel->Disconnect();
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                        (0, "TSC_324m::ConfigureSrp Error - Failed to lookup outgoing logical channel %d", 0));
    }

    PVMFPortInterface *port = NULL;

    port = iSrp->RequestULPort(SRP_INPUT_PORT_TAG);
    if (port)
        port->Disconnect();

    port = iTscSrpBuffer->GetLLPort(TSCSRPBUFFER_OUTPUT_PORT_TAG);
    if (port)
        port->Disconnect();

    port = iSrp->RequestULPort(SRP_OUTPUT_PORT_TAG);
    if (port)
        port->Disconnect();

    port = iTscSrpBuffer->GetLLPort(TSCSRPBUFFER_INPUT_PORT_TAG);
    if (port)
        port->Disconnect();

    iTscSrpBuffer->GetUpperLayer()->UnBind();

    if (iOutgoingSrpPort)
        iOutgoingSrpPort->Disconnect();
    if (iIncomingSrpPort)
        iIncomingSrpPort->Disconnect();
}


OSCL_EXPORT_REF void TSC_324m::GetChannelFormatAndCapabilities(TPVDirection dir,
        Oscl_Vector<FormatCapabilityInfo, OsclMemAllocator>& formats)
{
    iTSCcomponent->GetChannelFormatAndCapabilities(dir, formats);
}

OSCL_EXPORT_REF void TSC_324m::SetLogicalChannelBufferingMs(uint32 aInBufferingMs,
        uint32 aOutBufferingMs)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::SetLogicalChannelBufferingMs aInBufferingMs=%d, aOutBufferingMs=%d",
                     aInBufferingMs, aOutBufferingMs));
    iInLogicalChannelBufferingMs = aInBufferingMs;
    iOutLogicalChannelBufferingMs = aOutBufferingMs;
}

void TSC_324m::UseWNSRP(bool aUse)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::UseWNSRP %d", aUse));
    SetWnsrp(aUse);

    PS_MultiplexEntryDescriptor desc = iH223->GetIncomingMuxDescriptor(TSC_WNSRP_MUX_ENTRY_NUMBER);
    if (desc &&
            desc->size_of_elementList &&
            desc->elementList->muxType.index == 0 &&
            desc->elementList->muxType.logicalChannelNumber == 0)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_324m::UseWNSRP Deleting incoming descriptor for WNSRP"));
        iH223->RemoveIncomingMuxDescriptor(TSC_WNSRP_MUX_ENTRY_NUMBER);
    }
}


void TSC_324m::SetTSC_324mObserver(TSC_324mObserver* aObserver)
{
    if (aObserver == NULL)
        return;
    iTSC_324mObserver = aObserver;
}

PVMFCommandId TSC_324m::QueueCommandL(Tsc324mNodeCommand& aCmd)
{
    PVMFCommandId id;

    id = iCmdQueue.AddL(aCmd);

    //wakeup the AO
    RunIfNotReady();

    return id;
}

void TSC_324m::CommandComplete(Tsc324mNodeCmdQ& aCmdQ, Tsc324mNodeCommand& aCmd,
                               PVMFStatus aStatus, OsclAny* aEventData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m:CommandComplete Id %d Cmd %d Status %d Context %d Data %d",
                     aCmd.iId, aCmd.iCmd, aStatus, aCmd.iContext, aEventData));

    //create response
    PVMFCmdResp resp(aCmd.iId, aCmd.iContext, aStatus, aEventData);
    PVMFSessionId session = aCmd.iSession;

    //Erase the command from the queue.
    aCmdQ.Erase(&aCmd);

    //Report completion to the session observer.
    ReportCmdCompleteEvent(session, resp);
}

int32 TSC_324m::GetPortTagForLcn(TPVDirection aDir, TPVChannelId aId,
                                 PVCodecType_t aCodecType)
{
    if (aDir == INCOMING)
        return -aId;
    return GetMediaType(aCodecType);
}

PVMFStatus TSC_324m::ThreadLogon()
{
    AddToScheduler();

    SetState(EPVMFNodeIdle);
    return PVMFSuccess;
}

PVMFStatus TSC_324m::ThreadLogoff()
{
    SetState(EPVMFNodeCreated);
    RemoveFromScheduler();
    return PVMFSuccess;
}

void TSC_324m::HandlePortActivity(const PVMFPortActivity& aActivity)
{
    OSCL_UNUSED_ARG(aActivity);
}

PVMFCommandId TSC_324m::Flush(PVMFSessionId aSession,
                              const OsclAny* aContext)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aContext);

    OSCL_LEAVE(PVMFErrNotSupported);
    return 0;
}

void TSC_324m::SetMioLatency(int32 aLatency, bool aAudio)
{
    iH223->SetMioLatency(aLatency, aAudio);
}

#ifdef MEM_TRACK
void TSC_324m::MemStats()
{
#if !(OSCL_BYPASS_MEMMGT)

    OsclAuditCB auditCB;
    OsclMemInit(auditCB);
    if (auditCB.pAudit)
    {
        MM_Stats_t* stats = auditCB.pAudit->MM_GetStats("");
        if (stats)
        {
            printf("\n###################Memory Stats Start#################\n");
            printf("  numBytes %d\n", stats->numBytes);
            printf("  peakNumBytes %d\n", stats->peakNumBytes);
            printf("  numAllocs %d\n", stats->numAllocs);
            printf("  peakNumAllocs %d\n", stats->peakNumAllocs);
            printf("  numAllocFails %d\n", stats->numAllocFails);
            printf("  totalNumAllocs %d\n", stats->totalNumAllocs);
            printf("  totalNumBytes %d\n", stats->totalNumBytes);
            printf("\n###################Memory Stats End###################\n");
        }

    }
#endif
}
#endif

