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
/*  file name            : tscmain.h                                         */
/*  file contents        :                                                   */
/*  draw                 : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendment                                                                */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/

#ifndef TSCMAIN_H_INCLUDED
#define TSCMAIN_H_INCLUDED
#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif

#ifndef OSCLCONFIG_IO_H_INCLUDED
#include "osclconfig_io.h"
#endif

#ifndef OSCL_SCHEDULER_AO_H_INCLUDED
#include "oscl_scheduler_ao.h"
#endif

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif

#ifndef OSCL_PRIQUEUE_H_INCLUDED
#include "oscl_priqueue.h"
#endif

#ifndef PVMF_MEDIA_DATA_H_INCLUDED
#include "pvmf_media_data.h"
#endif

#ifndef PVMF_SIMPLE_MEDIA_BUFFER_H_INCLUDED
#include "pvmf_simple_media_buffer.h"
#endif

#ifndef OSCL_TIMER_H_INCLUDED
#include "oscl_timer.h"
#endif

#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif
#include "pvmf_video.h"

#include "tsc.h"
#include "cpvh223multiplex.h"
#include "srp.h"
#include "h245.h"
#include "h245def.h"
#include "h245pri.h"
#include "h223types.h"
#include "h324utils.h"
#include "tscsrpbuffer.h"
#include "tsc_node_interface.h"
#include "pv_common_types.h"
#include "tsc_statemanager.h"
#include "tsc_lc.h"
#include "tsc_blc.h"
#include "tsc_clc.h"
#include "tsc_mt.h"
#include "tsc_capability.h"
#include "tsc_componentregistry.h"

class SimpleStackElement;

#define PVUidH324Interface  PVUuid(0x2b0b54e2,0x7079,0x46c6,0xb2,0x3e,0x04,0xff,0xd3,0x0e,0x14,0x36)


/* Directional indicators */
#define     UNI             1
#define     BI              2

/* Values for MUXTABLE_CHANGE */
#define     WAIT_IDLE       0
#define     WAIT_RES1       1
#define     WAIT_RES2       2


#define PV_TSC_RTD_TIMER_ID 2
#define PV_TSC_DISCONNECT_TIMER_ID 3
#define PV_TSC_WAITING_FOR_OBLC_TIMER_ID 4
#define PV_TSC_TCS_RECEIVE_TIMER_ID 5
#define PV_TSC_LEVEL_SETUP_TIMER_ID 6
#define FIRST_OUTGOING_LCN 6
#define DEFAULT_AUDIO_AL_INDEX 4
#define DEFAULT_VIDEO_AL_INDEX 5
#define DEFAULT_AL3_CONTROL_FIELD_OCTETS 1
#define DEFAULT_END_SESSION_TIMEOUT 5
#define WAITING_FOR_OBLC_TIMEOUT_SECONDS 15
#define PV_TSC_MAX_COMMAND_ID 65535
#define TSC_SE_TIMEOUT_DEFAULT 30
#define T401_DEFAULT 8
#define N401_DEFAULT 100

#define FIRST_MUX_ENTRY_NUMBER TSC_FM_MAX_MTE+1
#define LAST_MUX_ENTRY_NUMBER 14
#define TSC_WNSRP_MUX_ENTRY_NUMBER 15
#define TSC_INCOMING_CHANNEL_MASK (1<<16)

// For requesting ports
#define PVMF_AUDIO_OUTGOING_MIMETYPE    "x-pvmf/audio;dir=outgoing"
#define PVMF_VIDEO_OUTGOING_MIMETYPE    "x-pvmf/video;dir=outgoing"
#define PVMF_INCOMING_MIMETYPE          "x-pvmf/dir=incoming"
#define PVMF_OUTGOING_MIMETYPE          "x-pvmf/dir=outgoing"

/*---------------------------------------------------------------------------*/
/*  Constant Definition                                                      */
/*---------------------------------------------------------------------------*/


// H.245 Miscellaneous Commands
typedef enum _TCmdMisc
{
    EVideoFastUpdatePicture,
    EVideoTemporalSpatialTradeOff,
    EMaxH223MUXPDUsize
}TCmdMisc;

class TSC_324mObserver
{
    public:
        virtual void IncomingVendorId(TPVH245Vendor* vendor, const uint8* pn, uint16 pn_len, const uint8* vn, uint16 vn_len) = 0;
        virtual void UserInputReceived(CPVUserInput* aUI) = 0;
        virtual void UserInputCapability(int formats) = 0;
        virtual void VideoSpatialTemporalTradeoffCommandReceived(TPVChannelId id, uint8 tradeoff) = 0;
        virtual void VideoSpatialTemporalTradeoffIndicationReceived(TPVChannelId id, uint8 tradeoff) = 0;
        virtual void SkewIndicationReceived(TPVChannelId lcn1, TPVChannelId lcn2, uint16 skew) = 0;
};

class TSC_component;


class TSC_324m : public TSC,
            public PVMFNodeInterface,
            public OsclActiveObject,
            public CPVH223MultiplexObserver,
            public H245Observer,
            public MSDObserver,
            public CEObserver,
            public SRPObserver,
            public OsclTimerObserver
{
    public:
        OSCL_IMPORT_REF TSC_324m(TPVLoopbackMode aLoopbackMode);
        ~TSC_324m();
        void SetTSC_324mObserver(TSC_324mObserver* aObserver);

        void SetInitialValues();
        void InitComponent();
        bool ProcessCommand(Tsc324mNodeCommand& aCmd);
        void ReceivedFormatSpecificInfo(TPVChannelId lcn, uint8* fsi, uint32 fsi_len);
        bool IsEstablishedLogicalChannel(TPVDirection aDir, TPVChannelId aChannelId);

        /********************************/
        /** node virtuals              **/
        /********************************/
        OSCL_IMPORT_REF PVMFStatus GetCapability(PVMFNodeCapability& aNodeCapability);

        OSCL_IMPORT_REF PVMFCommandId Init(PVMFSessionId aSession, const OsclAny* aContext = NULL);
        void DoInit(Tsc324mNodeCommand& cmd);

        PVMFCommandId Prepare(PVMFSessionId aSession
                              , const OsclAny* aContext = NULL);
        void DoPrepare(Tsc324mNodeCommand& cmd);

        OSCL_IMPORT_REF PVMFCommandId Start(PVMFSessionId aSession, const OsclAny* aContext = NULL);
        void DoStart(Tsc324mNodeCommand& cmd);

        OSCL_IMPORT_REF PVMFCommandId Stop(PVMFSessionId aSession, const OsclAny* aContext = NULL);
        void StartDisconnect(Tsc324mNodeCommand& cmd);
        void DoStop(Tsc324mNodeCommand& cmd);

        OSCL_IMPORT_REF PVMFCommandId Pause(PVMFSessionId aSession, const OsclAny* aContext = NULL);
        void DoPause(Tsc324mNodeCommand& cmd)
        {
            OSCL_UNUSED_ARG(cmd);
        }

        OSCL_IMPORT_REF PVMFCommandId Reset(PVMFSessionId aSession, const OsclAny* aContext = NULL);
        void DoReset(Tsc324mNodeCommand& cmd);

        OSCL_IMPORT_REF PVMFPortIter* GetPorts(const PVMFPortFilter* aFilter = NULL);

        OSCL_IMPORT_REF PVMFCommandId RequestPort(PVMFSessionId aSession
                , int32 aPortTag
                , const PvmfMimeString* aPortConfig = NULL
                                                      , const OsclAny* aContext = NULL);

        void DoRequestPort(Tsc324mNodeCommand& cmd);

        OSCL_IMPORT_REF PVMFCommandId ReleasePort(PVMFSessionId aSession, PVMFPortInterface& aPort, const OsclAny* aContext = NULL);
        void DoReleasePort(Tsc324mNodeCommand& cmd);

        // Port iterator virtuals
        OSCL_IMPORT_REF uint16 NumPorts()
        {
            return 1;
        }

        OSCL_IMPORT_REF PVMFPortInterface* GetNext()
        {
            return NULL;
        }

        OSCL_IMPORT_REF PVMFCommandId QueryUUID(PVMFSessionId aSession, const PvmfMimeString& aMimeType,
                                                Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids,
                                                bool aExactUuidsOnly = false,
                                                const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId QueryInterface(PVMFSessionId aSession, const PVUuid& aUuid,
                PVInterface*& aInterfacePtr,
                const OsclAny* aContext = NULL);
        void DoQueryInterface(Tsc324mNodeCommand& cmd);
        void DoQueryUuid(Tsc324mNodeCommand& cmd);

        OSCL_IMPORT_REF PVMFCommandId CancelAllCommands(PVMFSessionId aSession, const OsclAny* aContextData = NULL);
        void DoCancelAllCommands(Tsc324mNodeCommand& cmd)
        {
            OSCL_UNUSED_ARG(cmd);
        }

        OSCL_IMPORT_REF PVMFCommandId CancelCommand(PVMFSessionId aSession, PVMFCommandId aCmdId, const OsclAny* aContextData = NULL);
        void DoCancelCommand(Tsc324mNodeCommand& cmd);

        PVMFStatus ThreadLogon();
        PVMFStatus ThreadLogoff();
        void HandlePortActivity(const PVMFPortActivity& aActivity);
        PVMFCommandId Flush(PVMFSessionId aSession , const OsclAny* aContext = NULL);
        void DoFlush(Tsc324mNodeCommand& cmd)
        {
            OSCL_UNUSED_ARG(cmd);
        }

        /* PVInterface virtuals */
        void addRef() {}
        void removeRef() {}
        bool queryInterface(const PVUuid& uuid, PVInterface*& iface)
        {
            OSCL_UNUSED_ARG(uuid);
            OSCL_UNUSED_ARG(iface);
            return false;
        }

        /********************************/
        /** TSC virtuals               **/
        /********************************/
        virtual void Tsc_UII_DTMF(uint8 dtmf_value, uint16 dtmf_duration);
        virtual void Tsc_UII_Alphanumeric(const uint8* str, uint16 str_len);

        OSCL_IMPORT_REF TPVStatusCode SetVendorIdInfo(TPVH245Vendor* vendor, const uint8* pn, uint16 pn_len, const uint8* vn, uint16 vn_len);

        /* Initializes the TSC with reference to mux and controls */
        virtual TPVStatusCode InitTsc();

        TPVStatusCode SetTimerRes(uint32 timer_res);

        //virtual TPVStatusCode SetCapability(CapabilitySet* capabilities) = 0;
        virtual TPVStatusCode ResetTsc();
        virtual TPVStatusCode Connect(uint16 info_len = 0, uint8* info_buf = NULL);
        virtual TPVStatusCode Disconnect();
        //Network is already disconnected, no need to send out any messages, just shutdown (synchronous call).
        virtual TPVStatusCode Abort();

        /* Channel specific commands */
        virtual TPVStatusCode SetChannelParam(CPVParam* params, TPVChannelId channel_id = CHANNEL_ID_UNKNOWN)
        {
            OSCL_UNUSED_ARG(channel_id);
            OSCL_UNUSED_ARG(params);
            return EPVT_Success;
        }
        virtual TPVStatusCode SetTerminalParam(CPVTerminalParam* params);
        virtual CPVTerminalParam* GetTerminalParam();
        virtual TPVStatusCode SetOutgoingBitrate(int32 bitrate);


        virtual TPVStatusCode RequestFrameUpdate(PVMFPortInterface* port);
        const uint8* GetFormatSpecificInfo(PVMFPortInterface* port, uint32* len);
        TPVStatusCode RequestMaxMuxPduSize(unsigned aPduSize);
        OSCL_IMPORT_REF TPVStatusCode SetTerminalType(uint8 aTType);
        void SetDatapathLatency(TPVDirection aDir, PVMFPortInterface* aPort, uint32 aLatency);
        void SetSkewReference(PVMFPortInterface* aPort, PVMFPortInterface* aReferencePort);
        OSCL_IMPORT_REF void SetMultiplexingDelayMs(uint16 aDelay);

        /* CPVMultiplexiObserver virtuals */
        void DataReceptionStart();
        void MuxSetupComplete(PVMFStatus aStatus, TPVH223Level aLevel);
        void MuxCloseComplete();
        void MuxErrorOccurred(TPVDirection direction, TPVMuxComponent component, PVMFStatus error,
                              uint8* info = NULL, uint info_len = 0);
        void SkewDetected(TPVChannelId lcn1, TPVChannelId lcn2, uint32 skew);
        void LcnDataDetected(TPVChannelId lcn);

        /* H245Observer virtuals */
        void Handle(PS_ControlMsgHeader msg);

        // MSD User
        MSD *Msd;
        void MSDStart();
        // Observer functions
        void MSDDetermineConfirm(MSDStatus type);
        void MSDDetermineIndication(MSDStatus type);
        void MSDRejectIndication();
        void MSDErrorIndication(MSDErrCode errCode);

        // CE User
        CE *Ce;
        void CEStart();
        // Observer functions
        void CETransferIndication(OsclSharedPtr<S_TerminalCapabilitySet> tcs);
        void CETransferConfirm();
        void CERejectIndication(CESource source, CECause cause, CEDirection direction);
        void SetLoopbackMode(TPVLoopbackMode aLoopbackMode)
        {
            iLoopbackMode = aLoopbackMode;
            if (iH223) iH223->SetLoopbackMode(iLoopbackMode);
        }

        //SRP observer functions
        void TransmissionFailure()
        {
            if (iObserver)
            {
                StopData();
                iObserver->InternalError();
            }
        }

        void SetWnsrp(bool aEnableWnsrp)
        {
            iEnableWnsrp = aEnableWnsrp;
        }
        void UseWNSRP(bool aUse);

        LogicalChannelInfo* GetLogicalChannelInfo(PVMFPortInterface& port);


        /* OsclTimer observer virtuals */
        void TimeoutOccurred(int32 timerID, int32 timeoutInfo);

        void ResetStats();
        void LogStats(TPVDirection dir);

        /* Set muliplex level.  Can be called when the node is in the Initialized state */
        OSCL_IMPORT_REF void SetMultiplexLevel(TPVH223Level muxLevel);
        OSCL_IMPORT_REF void SetSduSize(TPVDirection direction, uint16 size, TPVAdaptationLayer al);
        OSCL_IMPORT_REF void SetAl2Sn(int width);
        OSCL_IMPORT_REF void SetAl3ControlFieldOctets(unsigned cfo);
        OSCL_IMPORT_REF void SetMaxOutgoingPduSize(uint16 size);
        OSCL_IMPORT_REF void SetMaxMuxPduSize(uint16 size);
        OSCL_IMPORT_REF void SetMaxCcsrlSduSize(int size);
        OSCL_IMPORT_REF void SetOutgoingMuxPduType(TPVH223MuxPduType outgoingPduType);
        /**
          Sets the list of outgoing channel types to open and the b/w associated with them.
          Leaves if the media types, number of channels or the bandwidth is not supported
          **/
        OSCL_IMPORT_REF void SetOutgoingChannelConfig(Oscl_Vector<H324ChannelParameters, PVMFTscAlloc>& out_channel_config);
        OSCL_IMPORT_REF void SetIncomingChannelConfig(Oscl_Vector<H324ChannelParameters, PVMFTscAlloc>& in_channel_config);
        OSCL_IMPORT_REF void SetEndSessionTimeout(uint32 timeout);
        OSCL_IMPORT_REF void SetAlConfig(PV2WayMediaType media_type,
                                         TPVAdaptationLayer layer,
                                         bool allow);
        OSCL_IMPORT_REF void SetTimerCounter(TPVH324TimerCounter aTimerCounter,
                                             uint8 aSeries, uint32 aSeriesOffset, uint32 aValue);
        OSCL_IMPORT_REF void SetVideoResolutions(TPVDirection dir, Oscl_Vector<PVMFVideoResolutionRange, OsclMemAllocator>& resolutions);
        OSCL_IMPORT_REF void RmeSendReq(void);
        void EndSessionCommand(void);          /* Change or End Session         */
        void Tsc_IdcVi(void);
        void SendVideoTemporalSpatialTradeoffCommand(TPVChannelId aLogicalChannel, uint8 aTradeoff);
        void SendVideoTemporalSpatialTradeoffIndication(TPVChannelId aLogicalChannel, uint8 aTradeoff);
        void SendSkewIndication(TPVChannelId aLogicalChannel1, TPVChannelId aLogicalChannel2, uint16 aSkew);
        OSCL_IMPORT_REF void  SetLogicalChannelBufferingMs(uint32 aInBufferingMs,
                uint32 aOutBufferingMs);
        void RtdTrfReq(void);
        OSCL_IMPORT_REF void GetChannelFormatAndCapabilities(TPVDirection dir,
                Oscl_Vector<FormatCapabilityInfo, OsclMemAllocator>& formats);
        OSCL_IMPORT_REF void SendTerminalCapabilitySetCommand(void);
        OSCL_IMPORT_REF void SendModeRequestCommand();

        void CmdMisc(TCmdMisc type, TPVChannelId channelId, uint32 param = 0);

        CPvtTerminalCapability* GetRemoteCapability();
        void SetClock(PVMFMediaClock* aClock);
        void SetMioLatency(int32 aLatency, bool aAudio);

    private:
        /* Function Prototypes */
        void CommandComplete(Tsc324mNodeCmdQ& aCmdQ, Tsc324mNodeCommand& aCmd, PVMFStatus aStatus, OsclAny* aEventData = NULL);

        /* Sets the dispatch table entries */
        void SetDispatchTable();
        void initVarsLocal();
        void initVarsSession();

        OsclAny TcsMsdComplete();
        OsclAny SignalCsupComplete(PVMFStatus status);

        /* TSC Handling */
        /* ------------------------------------ */
        /*        CALL SETUP Prototypes         */
        /* ------------------------------------ */
        uint32 OpenSession(void);
        uint32 LcEtbIdc(PS_ControlMsgHeader  pReceiveInf);
        uint32 LcEtbCfm(PS_ControlMsgHeader  pReceiveInf);
        uint32 LcRlsIdc(PS_ControlMsgHeader  pReceiveInf);
        uint32 LcErrIdc(PS_ControlMsgHeader  pReceiveInf);
        uint32 BlcEtbIdc(PS_ControlMsgHeader  pReceiveInf);
        uint32 BlcEtbCfm(PS_ControlMsgHeader  pReceiveInf);
        uint32 BlcRlsIdc(PS_ControlMsgHeader  pReceiveInf);
        uint32 BlcErrIdc(PS_ControlMsgHeader  pReceiveInf);
        uint32 Status04Event22(PS_ControlMsgHeader  pReceiveInf);
        uint32 Status04Event23(PS_ControlMsgHeader  pReceiveInf);
        uint32 Status04Event24(PS_ControlMsgHeader  pReceiveInf);
        uint32 BlcEtbCfm2(PS_ControlMsgHeader  pReceiveInf);      /* WWU_BLC */
        uint32 SessionClose_CSUP();
        uint32 InternalError_CSUP(PS_ControlMsgHeader  pReceiveInf);
        uint32 MuxTableTransferIndication(PS_ControlMsgHeader  pReceiveInf);

        /* ------------------------------------ */
        /*       ONGOING COMM Prototypes        */
        /* ------------------------------------ */
        uint32 Status08Event11(PS_ControlMsgHeader  pReceiveInf);
        uint32 LcRlsCfm(PS_ControlMsgHeader  pReceiveInf);       /* WWU BGFX1 */
        uint32 BlcRlsCfm(PS_ControlMsgHeader  pReceiveInf);       /* RAN BGFX1 Bi-Dir OLCAck */
        uint32 Status08Event19(PS_ControlMsgHeader  pReceiveInf);
        uint32 Status08Event20(PS_ControlMsgHeader  pReceiveInf);
        uint32 Status08Event21(PS_ControlMsgHeader  pReceiveInf);
        uint32 Status08Event22(PS_ControlMsgHeader  pReceiveInf);
        uint32 Status08Event23(PS_ControlMsgHeader  pReceiveInf);
        uint32 Status08Event24(PS_ControlMsgHeader  pReceiveInf);
        uint32 Status08Event25(PS_ControlMsgHeader  pReceiveInf);
        uint32 Status08Event26(PS_ControlMsgHeader  pReceiveInf);
        uint32 Status08Event27(PS_ControlMsgHeader  pReceiveInf);
        uint32 ModeRequestIndication(PS_ControlMsgHeader  pReceiveInf);
        uint32 Status08Event29(PS_ControlMsgHeader  pReceiveInf);
        uint32 Status08Event30(PS_ControlMsgHeader  pReceiveInf);
        uint32 Status08Event31(PS_ControlMsgHeader  pReceiveInf);
        uint32 Status08Event32(PS_ControlMsgHeader  pReceiveInf);
        uint32 Status08User03(PS_ControlMsgHeader  pReceiveInf);
        uint32 Status08User04(PS_ControlMsgHeader  pReceiveInf);
        uint32 SendTerminalCapabilitySet(PS_ControlMsgHeader  pReceiveInf);    /* RAN STCS */
        uint32 FlowControlCommandReceived(PS_ControlMsgHeader  pReceiveInf);        /* AR FC */
        uint32 FlowControlIndicationReceived(PS_ControlMsgHeader  pReceiveInf);        /* AR FC */
        void SendFlowControlIndication();
        uint32 MiscCmdRecv(PS_ControlMsgHeader  pReceiveInf);        /* RAN MSCL */
        uint32 Status08Event55(PS_ControlMsgHeader  pReceiveInf);        /* RAN HMR */
        uint32 Status08User06(PS_ControlMsgHeader  pReceiveInf);        /* RAN RME */
        uint32 Status08User07(PS_ControlMsgHeader  pReceiveInf);        /* RAN STCS */
        uint32 Status08User08(PS_ControlMsgHeader  pReceiveInf);        /* RAN HMR */
        uint32 Status08User09(PS_ControlMsgHeader  pReceiveInf);        /* RAN RCC */
        uint32 Status08User10(PS_ControlMsgHeader  pReceiveInf);	/* RAN CLC */
        uint32 SessionClose_Comm();
        uint32 InternalError_Comm(PS_ControlMsgHeader  pReceiveInf);
        uint32 EndSessionRecv(PS_ControlMsgHeader  pReceiveInf);
        uint32 VendorIdRecv(PS_ControlMsgHeader  pReceiveInf);
        uint32 UserInputIndicationRecv(PS_ControlMsgHeader  pReceiveInf);
        uint32 MiscIndicationRecv(PS_ControlMsgHeader  pReceiveInf);
        uint32 SkewIndicationRecv(PS_ControlMsgHeader  pReceiveInf);
        uint32 FunctionNotSupportedIndicationReceived(PS_ControlMsgHeader  pReceiveInf);
        void StopData();  /* RAN CLC */

        uint32 NonProcedure_Ui(PS_ControlMsgHeader  pReceiveInf);
        uint32 NonProcedure_Se(PS_ControlMsgHeader  pReceiveInf);

        /* ------------------------------------ */
        /*           MISC Prototypes            */
        /* ------------------------------------ */
        int32 GetReceivedAudioCodec(void);						// (RAN-TD)
        int32 GetReceivedVideoCodec(void);						// (RAN-TD)

        uint32 CheckAltCapSet(PS_AlternativeCapabilitySet pAltCapSet, uint32 entry, uint32* preference_index);
        void ExtractTcsParameters(PS_TerminalCapabilitySet pCap);
        void ParseTcsCapabilities(S_Capability &aCapability);

        void CmdEcrpt(void);                   /* Encryption                    */
        void CmdFc(void);                      /* Flow Control                  */
        PVMFPortInterface* FindOutgoingPort(TPVMediaType_t mediaType);
//	void CmdCnf            ( void );       /* Conference Commands           */
//	void CmdCm             ( void );       /* Communication Mode Commands   */
        void CmdHmr(int32);                    /* H223MultiplexReconfiguration  */

        /* RME */
        void RmeSendRps(void);
        void RmeRjtReq(void);

        /* Mode Request */
        void getModeRequestInfo(PS_ControlMsgHeader pReceiveInf, int *param1, int *param2);
        void MrRjtReq(void);

        // H.245 Miscellaneous Indications
        typedef enum _TIndicationMisc
        {
            EVideoTemporalSpatialTradeOffIdc,
        }TIndicationMisc;
        void IndicationMisc(TIndicationMisc type, TPVChannelId channelId, uint32 param = 0, OsclAny* param1 = NULL);
        void Tsc_IdcSkew(TPVChannelId lcn1, TPVChannelId lcn2, uint16 skew);
        void SendFunctionNotSupportedIndication(uint16 cause, uint8* function = NULL, uint16 len = 0);
        // utility functions

        void ShowTsc(uint16 tag, uint16 indent, char* inString);
        void IgnoreH245Callbacks();
        void ConfigureSrp(TPVH223Level aLevel);
        void StopSrp();

#ifdef MEM_TRACK
        void MemStats();
#endif
        /* member variables */
        /* The SRP Instance */
        SRP* iSrp;

        /* The H245 instance */
        H245* iH245;

        /* The H223 instance */
        H223* iH223;

        uint32 iTerminalStatus;		/* Terminal Status */
        uint32 iCeRetries;


        TPVH245Vendor* iVendor;
        uint8* iProductNumber;
        uint32 iProductNumberLen;
        uint8*  iVersionNumber;
        uint32 iVersionNumberLen;
        TPVH245Vendor* iVendorR;
        uint8* iProductNumberR;
        uint32 iProductNumberLenR;
        uint8*  iVersionNumberR;
        uint32 iVersionNumberLenR;

        unsigned iSendRme; /* Send RME to remote */
        unsigned iCsupSeq; /* Order of sending TCS, MSD, VID */
        unsigned iRequestMaxMuxPduSize; /* Requests maxMuxPduSize to the remote terminal if > 0.  This is done after TCS
											 if the remote terminal supports the maxMuxPduCapability */
        bool iMaxMuxPduCapabilityR;  /* Does the remote terminal suport maxMuxPduCapability ? */



        // Additional global variables.
        S_MobileOperationTransmitCapability* iTransmitCaps;


        /* Who initiated disconnect ? 0 - None, 1 - Local, 2 - Remote */
        unsigned iDisconnectInitiator;

        /* Why did the Connect fail ? */
        TPVStatusCode iConnectFailReason;

        /* Supplementary Connect info.  We shall use it for now for the case of 2-stage dialling.
            Here the supplementary info, specifies the url of the person to be dialled */
        uint8* iSuppInfo;
        uint32 iSuppInfoLen;
        // --------------------------------------
        //  Dispatch table structure definition
        // --------------------------------------
        typedef uint32(TSC_324m::*DISPATCH_PTR)(PS_ControlMsgHeader);           /* function pointer              */

        typedef struct _dispatchtable
        {
            DISPATCH_PTR Module;      /* function pointer              */
            uint32    Status;                         /* Status of terminal            */
            uint32    Event;                          /* Event                         */
        } S_DispatchTable;
        S_DispatchTable     iDispatchTable[100];

        // ----------------------------------------------------------
        // Original dispatch table for (Se<->Tsc) events
        // ----------------------------------------------------------
        H223ChannelParam* iUnusedChannels[MAX_LCNS];
        TPVLoopbackMode iLoopbackMode;

        OsclTimer<OsclMemAllocator>* iTimer;
    private:
        // OsclActiveObject virtuals
        void DoCancel();
        void Run();

        Tsc324mNodeCommand* FindPendingStartCmd();
        int32 FindPendingPortCmd(TPVMFGenericNodeCommand aCmdType,  int32 aPortTag, Tsc324mNodeCommand& cmd);
        Tsc324mNodeCommand* FindPendingStopCmd();

        int FindPendingCmd(PVMFCommandId aCommandId);
        PVMFCommandId QueueCommandL(Tsc324mNodeCommand& aCmd);

        int32 GetPortTagForLcn(TPVDirection aDir, TPVChannelId aId, PVCodecType_t aCodecType);
        uint32 GetMaxBitrateForOutgoingChannel(PVCodecType_t codecType);
        PS_DataType GetDataType(PVCodecType_t codecType, uint32 bitrate, const uint8* dci, uint16 dci_len);

        PVMFCommandId iCurrentCmdId;
        // Priority queue of commands
        Tsc324mNodeCmdQ iCmdQueue;
        Tsc324mNodeCmdQ iPendingCmdQueue;

        // SRP ports
        PVMFPortInterface* iOutgoingSrpPort;
        PVMFPortInterface* iIncomingSrpPort;
        TscSrpBuffer* iTscSrpBuffer;

        /* Mux level */
        TPVH223Level iH223Level;

        /* Outgoing pdu type - audio only, video only OR Audio+Video */
        TPVH223MuxPduType iOutgoingPduType;
        /* Configuration of incoming channels */
        Oscl_Vector<H223ChannelParam*, PVMFTscAlloc> iIncomingChannels;
        /* Pending OLCs:  This includes pending and established uni- and bi- directional, incoming and outgoing OLCs. */
        PVLogger *iLogger;
        bool iMuxTableUpdateRequired;
        uint32 iEndSessionTimeout;
        Tsc324mNodeCommand *iStopCmd;
        OsclAny* iStopContext;
        /* Timer/counter values */
        H245TimerValues iTimerValues;
        uint32 iT401; /* SRP */
        uint32 iN100; /* H245 */
        uint32 iN401; /* SRP */
        uint32 iMultiplexingDelayMs;
        uint32 iInLogicalChannelBufferingMs;
        uint32 iOutLogicalChannelBufferingMs;
        TSC_324mObserver* iTSC_324mObserver;
        Oscl_Map<PVMFPortInterface*, H223IncomingChannelPtr, OsclMemAllocator> iRequestedIncomingChannels;
        Oscl_Map<PVMFPortInterface*, H223OutgoingChannelPtr, OsclMemAllocator> iRequestedOutgoingChannels;
        uint32 iNumRtdRequests;
        uint32 iRtdMin;
        uint32 iRtdMax;
        uint32 iRtdAve;

        bool iEnableWnsrp;
        bool iInitialized;
        bool iInitializedComponent;

        TSC_statemanager iTSCstatemanager;
        TSC_component* iTSCcomponent;
        TSC_clc iTSCclc;
        TSC_blc iTSCblc;
        TSC_lc  iTSClc;
        TSC_mt  iTSCmt;
        TSC_capability iTSCcapability;
        TSCComponentRegistry iComponentRegistry;
        PVMFMediaClock* iClock;
};

#define TSC_TIMER_RES 1000
#endif

