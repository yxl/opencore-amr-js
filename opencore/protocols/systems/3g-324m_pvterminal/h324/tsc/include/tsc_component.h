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
/*  file name            : tsc_component.h                                   */
/*  file contents        :                                                   */
/*  draw                 : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendment                                                                */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/

#ifndef TSCCOMPONENT_H_INCLUDED
#define TSCCOMPONENT_H_INCLUDED



#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif

#ifndef TSC_H_INCLUDED
#include "tsc.h"
#endif

#ifndef H245_H_INCLUDED
#include "h245.h"
#endif

#ifndef TSC_NODE_INTERFACE_H_INCLUDED
#include "tsc_node_interface.h"
#endif

#ifndef TSC_CHANNELCONTROL_H_INCLUDED
#include "tsc_channelcontrol.h"
#endif

#ifndef TSC_H324M_CONFIG_INTERFACE_H_INCLUDED
#include "tsc_h324m_config_interface.h"
#endif

class TSC_statemanager;
class TSC_capability;
class TSC_mt;
class TSC_lc;
class TSC_blc;
class TSC_clc;



class TSC_component : public OsclTimerObserver,
            public PVMFComponentInterface
{
    public:
        TSC_component(TSC_statemanager& aTSCStateManager,
                      TSC_capability& aTSCcapability,
                      TSC_lc& aTSClc,
                      TSC_blc& aTSCblc,
                      TSC_clc& aTSCclc,
                      TSC_mt& aTSCmt);

        void SetMembers(H245* aH245, H223* aH223, TSCObserver* aTSCObserver);
        bool queryInterface(const PVUuid& aUuid, PVInterface*& aInterfacePtr);

        void Init()
        {
            InitVarsLocal();
            InitVarsSession();
        }
        bool CEStart();
        virtual void InitVarsSession();
        virtual void InitVarsLocal();
        virtual void MembersSet() {};
        void InitTsc();
        void ResetTsc();
        void Disconnect();

        void GetTerminalParam(CPVH324MParam& ah324param);
        TPVStatusCode SetTerminalParam(CPVTerminalParam& params);

        bool IsSupported(TPVDirection dir, PV2WayMediaType media, CodecCapabilityInfo& codec_info);
        bool IsSupported(TPVDirection dir, PVCodecType_t codec, FormatCapabilityInfo& capability_info);

        virtual void ExtractTcsParameters(PS_TerminalCapabilitySet pTcs);
        void CETransferIndication(OsclSharedPtr<S_TerminalCapabilitySet> tcs, uint32 aTerminalStatus);
        TPVAdaptationLayer GetVideoLayer()
        {
            return iVideoLayer;
        }

        void ClipCodecs(PS_TerminalCapabilitySet pTcs);			// (RAN-32K)

        // mux descriptors
        CPVMultiplexEntryDescriptor* GenerateSingleDescriptor(uint8 entry_num, TPVChannelId lcn1);
        void SetAlConfig(PV2WayMediaType media_type,
                         TPVAdaptationLayer layer,
                         bool allow);

        void SetAl2Al3VideoFlags(int32 userInput);
        int32 GetAl2Al3VideoFlags();
        Oscl_Vector<H324ChannelParameters, PVMFTscAlloc>* GetChannelConfig(TPVDirection dir);
        void SetAl3ControlFieldOctets(unsigned cfo);

        void SetAl2Sn(int width);
        uint32 LcEtbIdc(PS_ControlMsgHeader pReceiveInf);
        uint32 BlcEtbIdc(PS_ControlMsgHeader pReceiveInf);
        PVMFStatus ValidateOlcsWithTcs();
        OsclAny TcsMsdComplete();

        virtual void SetOutgoingChannelConfig(Oscl_Vector<H324ChannelParameters, PVMFTscAlloc>& out_channel_config);
        virtual void SetIncomingChannelConfig(Oscl_Vector<H324ChannelParameters, PVMFTscAlloc>& in_channel_config);
        virtual uint32 LcEtbCfm(PS_ControlMsgHeader pReceiveInf);
        unsigned GetOutgoingBitrate(PVCodecType_t codecType);
        void GetChannelFormatAndCapabilities(TPVDirection dir,
                                             Oscl_Vector<FormatCapabilityInfo, OsclMemAllocator>& formats);


        bool HasOlc(TPVDirection direction, TPVChannelId id, unsigned state = 0);
        OlcParam* FindOlcGivenChannel(TPVDirection direction, TPVChannelId id);
        OlcParam* FindOlc(TPVDirection direction, PV2WayMediaType media_type, unsigned state = 0);

        virtual void LcnDataDetected(TPVChannelId lcn);
        OsclAny AcceptBLCRequest(TPVChannelId OpenLcnF,
                                 TPVChannelId OpenLcnB,
                                 PS_ForwardReverseParam forRevParams);
        uint32 LcRlsIdc(PS_ControlMsgHeader pReceiveInf);
        uint32 BlcRlsIdc(PS_ControlMsgHeader pReceiveInf);
        OsclAny MuxTableSendComplete(uint32 sn, PVMFStatus status);
        OsclAny StopData();
        void Start();
        virtual void StartDisconnect(bool terminate);
        virtual bool Connect1LevelKnown();
        virtual void Connect2();
        void TimeoutOccurred(int32 timerID, int32 timeoutInfo);
        virtual bool ProcessCommand(Tsc324mNodeCommand& aCmd)
        {
            OSCL_UNUSED_ARG(aCmd);
            return false;
        }
        virtual void MuxSetupComplete(PVMFStatus status, TPVH223Level level)
        {
            OSCL_UNUSED_ARG(status);
            OSCL_UNUSED_ARG(level);
        };
        virtual Tsc324mNodeCommand* GetCommand()
        {
            return NULL;
        };
        virtual bool IsEnabled()
        {
            return true;
        };
        uint32 Status08Event19(PS_ControlMsgHeader pReceiveInf);
        uint32 BlcEtbCfm(PS_ControlMsgHeader pReceiveInf);
        uint32 BlcEtbCfm2(PS_ControlMsgHeader pReceiveInf);
        uint32 LcRlsCfm(PS_ControlMsgHeader pReceiveInf);
        uint32 BlcRlsCfm(PS_ControlMsgHeader pReceiveInf);
        void CloseChannels();
        LogicalChannelInfo* GetLogicalChannelInfo(PVMFPortInterface& port);
        virtual void Timeout() {};

        void ReceivedFormatSpecificInfo(TPVChannelId channel_id, uint8* fsi, uint32 fsi_len);
        bool IsEstablishedLogicalChannel(TPVDirection aDir, TPVChannelId aChannelId);
#ifdef MEM_TRACK
        void MemStats();
#endif
    protected:
        virtual bool Pausable()
        {
            return true;
        }
        void CheckOutgoingChannel(OlcParam* olc_param, PVMFStatus status);
        void ChannelReleased(TPVDirection dir, TPVChannelId lcn, PVMFStatus status);
        OlcParam* OpenLogicalChannel(TPVDirection dir,
                                     TPVChannelId lcn,
                                     TPVChannelId lcnRvs = CHANNEL_ID_UNKNOWN,
                                     PS_DataType dt = NULL,
                                     PS_H223LogicalChannelParameters lcp = NULL,
                                     PS_DataType dtRvs = NULL,
                                     PS_H223LogicalChannelParameters lcpRvs = NULL);
        void RemoveOlc(TPVDirection dir, TPVChannelId id);
        virtual void ReleasePendingIncomingChannel(TPVChannelId aOpenLcn)
        {
            OSCL_UNUSED_ARG(aOpenLcn);
        };
        virtual bool ReleasedPendingIncomingChannel(OlcParam* aPendingParam)
        {
            OSCL_UNUSED_ARG(aPendingParam);
            return false;
        };
        PS_AdaptationLayerType GetOutgoingLayer(PV2WayMediaType media_type, uint32 max_sample_size);
        OlcParam* OpenOutgoingChannelWithTcsCheck(PVCodecType_t codec,
                PS_AdaptationLayerType adaptation_layer,
                PVCodecType_t in_codec);

        virtual void SetCustomMultiplex(PS_ControlMsgHeader pReceiveInf,
                                        PV2WayMediaType media_type)
        {
            OSCL_UNUSED_ARG(pReceiveInf);
            OSCL_UNUSED_ARG(media_type);
        };
        virtual void RemoveMultiplex(OlcParam* olc_param)
        {
            OSCL_UNUSED_ARG(olc_param);
        };
        virtual bool AlreadyAssigned(PV2WayMediaType media_type)
        {
            OSCL_UNUSED_ARG(media_type);
            return false;
        };
        virtual void StartOlc(OlcParam* olc_param,
                              PV2WayMediaType media_type,
                              CPVMultiplexEntryDescriptorVector descriptors)
        {
            OSCL_UNUSED_ARG(olc_param);
            OSCL_UNUSED_ARG(media_type);
            OSCL_UNUSED_ARG(descriptors);
        };
        virtual bool FinishTcsMsdComplete(CPVMultiplexEntryDescriptorVector descriptors)
        {
            OSCL_UNUSED_ARG(descriptors);
            return true;
        };
        virtual void CustomGenerateTcs(PS_TerminalCapabilitySet& ret)
        {
            OSCL_UNUSED_ARG(ret);
        };
        virtual bool IsRemovable(TPVChannelId lcn)
        {
            OSCL_UNUSED_ARG(lcn);
            return true;
        };
        virtual PS_DataType GetOutgoingDataType(PVCodecType_t codecType,
                                                uint32 bitrate);

        OlcParam* OpenOutgoingChannel(PVCodecType_t codec,
                                      PS_AdaptationLayerType adaptation_layer,
                                      PS_DataType pDataTypeRvs = NULL,
                                      PS_H223LogicalChannelParameters pH223ParamsRvs = NULL);

        OlcList iOlcs;


        TSC_statemanager& iTSCstatemanager;
        TSC_capability& iTSCcapability;
        TSC_lc& iTSClc;
        TSC_blc& iTSCblc;
        TSC_clc& iTSCclc;
        TSC_mt& iTSCmt;

        H245* iH245;
        H223* iH223;

        PS_TerminalCapabilitySet iLocalTcs;
        OsclTimer<OsclMemAllocator>* iWaitingForOblcTimer;
        // Configuration of outgoing channels
        Oscl_Vector<H324ChannelParameters, PVMFTscAlloc>* iOutgoingChannelConfig;
        // Configuration of incoming channels
        Oscl_Vector<H324ChannelParameters, PVMFTscAlloc>* iIncomingChannelConfig;
        TSCObserver* iTSCObserver;
        TSC_channelcontrol iTSCchannelcontrol;


        PVLogger* iLogger;

        bool iAllowAl1Video;
        bool iAllowAl2Video;
        bool iAllowAl3Video;
        bool iAllowAl1Audio;
        bool iAllowAl2Audio;
        bool iAllowAl3Audio;		/*   (These are sent in outgoing CE) */
        bool iUseAl1Video;
        bool iUseAl2Video;
        bool iUseAl3Video;
    private:
        uint32 OpenLogicalChannel(TPVChannelId OpenLcn,
                                  PS_DataType pDataType,
                                  PS_H223LogicalChannelParameters pH223Lcp);
        PVMFStatus VerifyReverseParameters(PS_ForwardReverseParam forRevParams,
                                           TSCObserver* aObserver);
        void ReleaseOlc(OlcParam* olc, uint16 cause);
        void SignalChannelClose(TPVDirection dir, TPVChannelId lcn, TPVDirectionality directionality);
        void OpenPort(TPVDirection dir, TPVChannelId lcn, H223ChannelParam* param);

    private:


        /* AL3 control field octets */
        unsigned iAl3ControlFieldOctets;
        /* AL2 with/without sequence numbers */
        bool iAl2WithSn;

        unsigned iRemoteAl1Audio;			/* Remote terminal */
        unsigned iRemoteAl2Audio;			/* Remote terminal */
        unsigned iRemoteAl3Audio;			/*   (These are received in incoming CE) */
        unsigned iRemoteAl1Video;			/* Remote terminal */
        unsigned iRemoteAl2Video;			/* Remote terminal */
        unsigned iRemoteAl3Video;			/*   (These are received in incoming CE) */

        TPVAdaptationLayer iVideoLayer;			/* Layer to use, decided by local terminal */
        /*   (0=undetermined, 2=AL2, 3=AL3) */
        OsclSharedPtr<S_TerminalCapabilitySet> iRemoteTcs;

        Oscl_Vector<CodecCapabilityInfo*, OsclMemAllocator> iOutCodecList;
        bool iWaitingForOblc;
        PVCodecType_t iWaitingForOblcCodec;
};

#endif

