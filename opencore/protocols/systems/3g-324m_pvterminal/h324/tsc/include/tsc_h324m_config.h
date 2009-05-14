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
#ifndef TSC_H324M_CONFIG_H_INCLUDED
#define TSC_H324M_CONFIG_H_INCLUDED

#ifndef TSC_H324M_CONFIG_INTERFACE_H_INCLUDED
#include "tsc_h324m_config_interface.h"
#endif

#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif

#ifndef OSCL_MAP_H_INCLUDED
#include "oscl_map.h"
#endif
#ifndef PV_PROXIED_INTERFACE_H_INCLUDED
#include "pv_proxied_interface.h"
#endif

#ifndef TSCMAIN_H_INCLUDED
#include "tscmain.h"
#endif

class PVLogger;
class CPVCmnInterfaceCmdMessage;
class TSC_324m;

class H324MConfig : public OsclActiveObject,
            public H324MConfigInterface,
            public TSC_324mObserver
{
    public:
        H324MConfig(TSC_324m* aH324M, bool aUseAO);
        virtual ~H324MConfig();

        void SetObserver(H324MConfigObserver* aObserver);

        PVMFCommandId SetMultiplexLevel(TPVH223Level aLevel, OsclAny* aContextData = NULL);
        PVMFCommandId SetMaxSduSize(TPVAdaptationLayer aLayer, int32 aSize, OsclAny* aContextData = NULL);
        PVMFCommandId SetMaxSduSizeR(TPVAdaptationLayer aLayer, int32 aSize, OsclAny* aContextData = NULL);
        PVMFCommandId SetAl2SequenceNumbers(int32 aSeqNumWidth, OsclAny* aContextData = NULL);
        PVMFCommandId SetAl3ControlFieldOctets(int32 aCfo, OsclAny* aContextData = NULL);
        PVMFCommandId SetOutoingPduType(TPVH223MuxPduType aOutgoingPduType, OsclAny* aContextData = NULL);
        PVMFCommandId SetMaxPduSize(int32 aMaxPduSize, OsclAny* aContextData = NULL);
        PVMFCommandId SetTerminalType(uint8 aTerminalType, OsclAny* aContextData = NULL);
        PVMFCommandId SetALConfiguration(TPVMediaType_t aMediaType, TPVAdaptationLayer aLayer,
                                         bool aAllow, OsclAny* aContextData = NULL);
        PVMFCommandId SendRme(OsclAny* aContextData = NULL);
        PVMFCommandId SetMaxMuxPduSize(int32 aRequestMaxMuxPduSize, OsclAny* aContextData = NULL);
        PVMFCommandId SetMaxMuxCcsrlSduSize(int32 aMaxCcsrlSduSize, OsclAny* aContextData = NULL);
        PVMFCommandId FastUpdate(PVMFNodeInterface& aTrack, OsclAny* aContextData = NULL);
        PVMFCommandId SendRtd(OsclAny* aContextData = NULL);
        PVMFCommandId SetVendor(uint8 aCc, uint8 aExt, uint32 aMc,
                                const uint8* aProduct, uint16 aProductLen,
                                const uint8* aVersion, uint16 aVersionLen,
                                OsclAny* aContextData = NULL);
        PVMFCommandId SendEndSession(OsclAny* aContextData = NULL);
        PVMFCommandId SetEndSessionTimeout(uint32 aTimeout, OsclAny* aContextData = NULL);
        PVMFCommandId SetTimerCounter(TPVH324TimerCounter aTimerCounter,
                                      uint8 aSeries, uint32 aSeriesOffset,
                                      uint32 aValue,
                                      OsclAny* aContextData = NULL);
        PVMFCommandId SetVideoResolutions(TPVDirection aDirection,
                                          Oscl_Vector<PVMFVideoResolutionRange, OsclMemAllocator>& aResolutions,
                                          OsclAny* aContextData = NULL);
        PVMFCommandId SendVendorId(OsclAny* aContextData = NULL);
        PVMFCommandId SendVideoTemporalSpatialTradeoffCommand(TPVChannelId aLogicalChannel, uint8 aTradeoff,
                OsclAny* aContextData = NULL);
        PVMFCommandId SendVideoTemporalSpatialTradeoffIndication(TPVChannelId aLogicalChannel, uint8 aTradeoff,
                OsclAny* aContextData = NULL);
        PVMFCommandId SendSkewIndication(TPVChannelId aLogicalChannel1, TPVChannelId aLogicalChannel2, uint16 aSkew,
                                         OsclAny* aContextData = NULL);
        PVMFCommandId SetLogicalChannelBufferingMs(uint32 aInBufferingMs,
                uint32 aOutBufferingMs,
                OsclAny* aContextData = NULL);
        PVMFCommandId SendUserInput(CPVUserInput* user_input,
                                    OsclAny* aContextData = NULL);
        PVMFCommandId SetWnsrp(const bool aEnableWnsrp,
                               OsclAny* aContextData = NULL);
        ////////////////////////////////////
        // PVInterface virtuals
        ////////////////////////////////////
        void addRef();
        void removeRef();
        bool queryInterface(const PVUuid& aUuid, PVInterface*& aInterfacePtr);

        // TSC_324mObserver virtuals
        void IncomingVendorId(TPVH245Vendor* vendor, const uint8* pn, uint16 pn_len, const uint8* vn, uint16 vn_len);
        void UserInputReceived(CPVUserInput* aUI);
        void UserInputCapability(int formats);
        void VideoSpatialTemporalTradeoffCommandReceived(TPVChannelId id, uint8 tradeoff);
        void VideoSpatialTemporalTradeoffIndicationReceived(TPVChannelId id, uint8 tradeoff);
        void SkewIndicationReceived(TPVChannelId lcn1, TPVChannelId lcn2, uint16 skew);
    private:
        void Run();
        void SendCmdResponse(PVMFCommandId id, OsclAny* context, PVMFStatus status);
        void SendAsyncEvent(PVMFAsyncEvent& event);

        void UseAO();

        TSC_324m* iH324M;
        int32 iReferenceCount;
        PVLogger* iLogger;
        PVMFCommandId iCommandId;
        H324MConfigObserver* iObserver;
        Oscl_Vector<PVMFCmdResp, OsclMemAllocator> iPendingResponses;
        Oscl_Vector<PVMFAsyncEvent, OsclMemAllocator> iPendingEvents;
        bool iUseAO;

        friend class H324MConfigProxied;
};


class H324MConfigProxied :
            public H324MConfigInterface,
            public PVProxiedInterfaceServer,
            public PVProxiedInterfaceClient,
            public H324MConfigObserver
{
    public:
        H324MConfigProxied(H324MConfigInterface *aH324MConfigIF, PVMainProxy *aMainProxy);
        virtual ~H324MConfigProxied();

        void SetObserver(H324MConfigObserver* aObserver);
        void H324MConfigCommandCompletedL(PVMFCmdResp& aResponse);
        void H324MConfigHandleInformationalEventL(PVMFAsyncEvent& aNotification);

        PVMFCommandId SetMultiplexLevel(TPVH223Level aLevel, OsclAny* aContextData = NULL);
        PVMFCommandId SetMaxSduSize(TPVAdaptationLayer aLayer, int32 aSize, OsclAny* aContextData = NULL);
        PVMFCommandId SetMaxSduSizeR(TPVAdaptationLayer aLayer, int32 aSize, OsclAny* aContextData = NULL);
        PVMFCommandId SetAl2SequenceNumbers(int32 aSeqNumWidth, OsclAny* aContextData = NULL);
        PVMFCommandId SetAl3ControlFieldOctets(int32 aCfo, OsclAny* aContextData = NULL);
        PVMFCommandId SetMaxPduSize(int32 aMaxPduSize, OsclAny* aContextData = NULL);
        PVMFCommandId SetTerminalType(uint8 aTerminalType, OsclAny* aContextData = NULL);
        PVMFCommandId SetALConfiguration(TPVMediaType_t aMediaType, TPVAdaptationLayer aLayer,
                                         bool aAllow, OsclAny* aContextData = NULL);
        PVMFCommandId SendRme(OsclAny* aContextData = NULL);
        PVMFCommandId SetMaxMuxPduSize(int32 aRequestMaxMuxPduSize, OsclAny* aContextData = NULL);
        PVMFCommandId SetMaxMuxCcsrlSduSize(int32 aMaxCcsrlSduSize, OsclAny* aContextData = NULL);
        PVMFCommandId FastUpdate(PVMFNodeInterface& aTrack, OsclAny* aContextData = NULL);
        PVMFCommandId SendRtd(OsclAny* aContextData = NULL);
        PVMFCommandId SetVendor(uint8 aCc, uint8 aExt, uint32 aMc,
                                const uint8* aProduct, uint16 aProductLen,
                                const uint8* aVersion, uint16 aVersionLen,
                                OsclAny* aContextData = NULL);
        PVMFCommandId SendEndSession(OsclAny* aContextData = NULL);
        PVMFCommandId SetEndSessionTimeout(uint32 aTimeout, OsclAny* aContextData = NULL);
        PVMFCommandId SetTimerCounter(TPVH324TimerCounter aTimerCounter,
                                      uint8 aSeries, uint32 aSeriesOffset,
                                      uint32 aValue,
                                      OsclAny* aContextData = NULL);
        PVMFCommandId SetVideoResolutions(TPVDirection aDirection,
                                          Oscl_Vector<PVMFVideoResolutionRange, OsclMemAllocator>& aResolutions,
                                          OsclAny* aContextData = NULL);
        PVMFCommandId SendVendorId(OsclAny* aContextData = NULL);
        PVMFCommandId SendVideoTemporalSpatialTradeoffCommand(TPVChannelId aLogicalChannel, uint8 aTradeoff,
                OsclAny* aContextData = NULL);
        PVMFCommandId SendVideoTemporalSpatialTradeoffIndication(TPVChannelId aLogicalChannel, uint8 aTradeoff,
                OsclAny* aContextData = NULL);
        PVMFCommandId SendSkewIndication(TPVChannelId aLogicalChannel1, TPVChannelId aLogicalChannel2, uint16 aSkew,
                                         OsclAny* aContextData = NULL);
        PVMFCommandId SetLogicalChannelBufferingMs(uint32 aInBufferingMs,
                uint32 aOutBufferingMs,
                OsclAny* aContextData = NULL);
        PVMFCommandId SendUserInput(CPVUserInput* user_input,
                                    OsclAny* aContextData = NULL);

        PVMFCommandId SetWnsrp(const bool aEnableWnsrp,
                               OsclAny* aContextData = NULL);
        void CleanupNotification(TPVProxyMsgId aId, OsclAny *aMsg);
        void HandleCommand(TPVProxyMsgId aMsgId, OsclAny *aMsg);
        void HandleNotification(TPVProxyMsgId aId, OsclAny *aMsg);
        void CleanupCommand(TPVProxyMsgId aId, OsclAny *aMsg);

        ////////////////////////////////////
        // PVInterface virtuals
        ////////////////////////////////////
        void addRef();
        void removeRef();
        bool queryInterface(const PVUuid& aUuid, PVInterface*& aInterfacePtr);

    private:
        H324MConfigInterface* iH324MConfigIF;
        PVMainProxy* iMainProxy;
        PVLogger* iLoggerClient;
        PVLogger* iLoggerServer;
        int32 iReferenceCount;
        TPVProxyId iProxyId;
        Oscl_Map<PVMFCommandId, CPVCmnInterfaceCmdMessage*, OsclMemAllocator> iPendingCmds;
        PVMFCommandId iCommandId;
        H324MConfigObserver* iObserver;
};

class H324MProxiedInterface : public PVProxiedInterface
{
    public:
        H324MProxiedInterface();
        virtual ~H324MProxiedInterface();

        void QueryProxiedInterface(const TPVProxyUUID& aUuid, PVInterface*& aInterfacePtr);
        void SetMainProxy(PVMainProxy* aMainProxy);

        void addRef();
        void removeRef();
        bool queryInterface(const PVUuid& aUuid, PVInterface*& aInterface);

        void SetH324M(TSC_324m* aH324M);

    private:
        TSC_324m* iH324M;
        PVMainProxy* iMainProxy;
        int32 iReferenceCount;
};

#endif

