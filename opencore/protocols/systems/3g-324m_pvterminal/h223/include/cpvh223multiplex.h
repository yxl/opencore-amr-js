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
#if !defined(H223_MULTIPLEX_H)
#define H223_MULTIPLEX_H
#include "oscl_rand.h"

#ifndef PVMF_MEDIA_CLOCK_H_INCLUDED
#include "pvmf_media_clock.h"
#endif

#include "h245pri.h"
#include "oscl_timer.h"
#include "h223types.h"
#include "pduparcom.h"
#include "lowerlayer.h"
#include "logicalchannel.h"

#ifndef PVMF_MEDIA_DATA_IMPL_H_INCLUDED
#include "pvmf_media_data_impl.h"
#endif

#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif

class MuxTableMgr;


typedef OsclSharedPtr<H223OutgoingChannel> H223OutgoingChannelPtr;
typedef OsclSharedPtr<H223IncomingChannel> H223IncomingChannelPtr;
typedef Oscl_Map<unsigned , OsclSharedPtr<PVMFMediaDataImpl>, PoolMemAlloc_OsclMemAllocator_10> MuxSduList;
class CPVH223MultiplexObserver
{
    public:
        virtual void DataReceptionStart() = 0;
        virtual ~CPVH223MultiplexObserver() {}
        virtual void MuxSetupComplete(PVMFStatus status, TPVH223Level level) = 0;
        virtual void MuxCloseComplete() = 0;
        virtual void MuxErrorOccurred(TPVDirection direction, TPVMuxComponent component, PVMFStatus error,
                                      uint8* info = NULL, uint info_len = 0) = 0;
        virtual void SkewDetected(TPVChannelId lcn1, TPVChannelId lcn2, uint32 skew) = 0;
        virtual void LcnDataDetected(TPVChannelId lcn) = 0;
        virtual void ReceivedFormatSpecificInfo(TPVChannelId lcn, uint8* fsi, uint32 fsi_len) = 0;
};

class CPVH223Multiplex :  public LowerLayerObserver,
            public H223PduParcomObserver,
            public LogicalChannelObserver
{
    public:
        CPVH223Multiplex(TPVLoopbackMode aLoopbackMode);
        ~CPVH223Multiplex();
        void SetObserver(CPVH223MultiplexObserver* observer)
        {
            iObserver = observer;
        }
        TPVStatusCode SetTimerRes(uint32 timer_res);

        PVMFPortInterface* GetLowerLayer();

        /* LoweLayer observer virtuals */
        void DataReceptionStart();
        void LevelSetupComplete(PVMFStatus status, TPVH223Level level);
        PVMFStatus GetOutgoingMuxPdus(MuxPduPacketList& packets);
        void LowerLayerError(TPVDirection directions, PVMFStatus error);

        /* PduParcomObserver virtuals */
        uint32 MuxPduIndicate(uint8* pPdu, uint32 pduSz, int32 fClosing, int32 muxCode);
        void MuxPduErrIndicate(EMuxPduError err);
        void MuxSetupComplete(PVMFStatus status, TPVH223Level level);
        H223PduParcomObserver* GetParcomObserver()
        {
            return this;
        }

        /* LogicalChannelObserver virtuals */
        inline int32 GetTimestamp();
        void LogicalChannelError(TPVDirection direction, TPVChannelId id, PVMFStatus error);
        void SkewDetected(TPVChannelId lcn1, TPVChannelId lcn2, uint32 skew);
        void ReceivedFormatSpecificInfo(TPVChannelId lcn, uint8* fsi, uint32 fsi_len);

        void Init();

        /* allocates resources  */
        TPVStatusCode Open();

        /* de-allocates resources */
        TPVStatusCode Close();

        /* Starts active element */
        int Start(H223PduParcomSharedPtr parcom);

        /* Stops active element */
        int Stop();

        TPVH223Level GetMuxLevel();

        int GetMinTimerRes();

        TPVStatusCode SetSduSize(TPVDirection direction, uint16 size, ErrorProtectionLevel_t epl);

        uint16 GetSduSize(TPVDirection direction, ErrorProtectionLevel_t epl);

        uint16 GetMaxSduSize(TPVDirection direction, ErrorProtectionLevel_t epl);

        // Channel APIs
        unsigned GetMaxChannels(TPVDirection /*direction*/)
        {
            return MAX_LCNS;
        }
        unsigned GetNumChannels(TPVDirection direction);

        TPVStatusCode OpenChannel(TPVDirection direction, TPVChannelId channel_id, H223ChannelParam* channel_param);
        TPVStatusCode CloseChannel(TPVDirection direction, TPVChannelId channel_id);
        PVMFStatus GetOutgoingChannel(TPVChannelId id, H223OutgoingChannelPtr& channel);
        PVMFStatus GetIncomingChannel(TPVChannelId id, H223IncomingChannelPtr& channel);
        TPVStatusCode StopChannel(TPVDirection /*direction*/, TPVChannelId /*channel_id*/);
        TPVStatusCode FlushChannel(TPVDirection /*direction*/, TPVChannelId /*channel_id*/);
        bool IsValid(TPVDirection /*direction*/, TPVChannelId /*channel_id*/)
        {
            return false;
        }


        TPVStatusCode SetMaxOutgoingPduSize(uint16 Size);
        unsigned GetMaxPduSize(TPVDirection direction);

        TPVStatusCode SetIncomingMuxTableEntries(CPVMultiplexDescriptor* descriptor);
        void SetIncomingMuxDescriptors(CPVMultiplexEntryDescriptorVector& descriptors, bool replace = true);
        void SetOutgoingMuxDescriptors(CPVMultiplexEntryDescriptorVector& descriptors);
        void RemoveOutgoingMuxDescriptor(uint8 muxTblNum);
        void RemoveIncomingMuxDescriptor(uint8 muxTblNum);
        void SetLoopbackMode(TPVLoopbackMode aLoopbackMode);
        PS_MultiplexEntryDescriptor GetIncomingMuxDescriptor(uint16 muxTblNum);

        void ResetStats();
        void LogStats(TPVDirection dir);

        /* Set muliplex level.  Can be called before the multiplex is started */
        void SetMultiplexLevel(TPVH223Level muxLevel);
        void SetClock(PVMFMediaClock* aClock)
        {
            iClock = aClock;
        }
        void SetMultiplexingDelayMs(uint16 aDelay);
        void SetLogicalChannelBufferingMs(uint32 aInBufferingMs,
                                          uint32 aOutBufferingMs);
        void SetBitrate(uint32 bitrate);
        void SetInterleavingMultiplexFlags(uint16 size, uint8* flags);
        void EnableStuffing(bool enable);
        TPVStatusCode SetMuxLevel(TPVH223Level muxLevel);
        void EnableStuffOnlyFirstSend(bool enable);
        void SetMioLatency(int32 aLatency, bool aAudio);
    protected:
        TPVStatusCode AlPduData(H223IncomingChannel* aChannel,
                                uint8*& pdu,
                                PS_MultiplexElement pElement,
                                int n);
        TPVStatusCode Dispatch(H223IncomingChannel* aChannel,
                               PS_MultiplexElement pElement);
        TPVStatusCode GetAdaptationLayer(OsclSharedPtr<AdaptationLayer> &al,
                                         TPVDirection aDirection,
                                         PS_H223LogicalChannelParameters aH223lcnParams,
                                         int32 aSdu_size,
                                         uint32 aNum_media_data);
        void MakePduData(OsclSharedPtr<PVMFMediaDataImpl>& outgoingPkt,
                         int32 max_size,
                         PS_MultiplexElement pElement,
                         MuxSduData& mux_sdu_data,
                         int* pm);
        int MuxToALDispatch(uint8* pdu, int pdu_size, int fClosing, int muxCode);
        int BreakPduData(uint8*& pdu, int pdu_size, PS_MultiplexElement pElement, int ListSize);

        TPVStatusCode Mux()
        {
            return EPVT_Success;
        }
        TPVStatusCode Demux()
        {
            return EPVT_Success;
        }


        PVMFStatus FindOutgoingChannel(TPVChannelId id, OsclSharedPtr<H223OutgoingChannel>& channel);
        PVMFStatus FindIncomingChannel(TPVChannelId id, OsclSharedPtr<H223IncomingChannel>& channel);

        MuxSduData* FindMuxSduData(TPVChannelId lcn,
                                   bool segmentable,
                                   uint32* index = NULL);
        void AppendMuxSduData(MuxSduData& data);
        void SetMuxSduData(MuxSduData& data);
        void RemoveMuxSduData(TPVChannelId lcn, bool segmentable);
        /* This fn checks if there is more data in the logical channel */
        void ReleaseMuxSdu(MuxSduData& mux_sdu_data,
                           MuxSduDataList& list,
                           unsigned index);
        unsigned UpdateSduDataLists();

        OsclSharedPtr<PVMFMediaDataImpl> InitPduPacket();
        PVMFStatus CompletePduPacket(OsclSharedPtr<PVMFMediaDataImpl>& packet, int mt, int pm);
        unsigned DispatchPduPacket(MuxPduPacketList& packets,
                                   MuxSduData& mux_sdu_data,
                                   PS_MultiplexEntryDescriptor entry,
                                   uint32 max_size);
        void UpdateMuxInterval(uint16 aInterval = 0);
        void ClearSduDataLists();
        uint16 MuxLcnData(MuxSduDataList& list,
                          MuxPduPacketList& packets,
                          uint16 max_size);

        void SetStuffingMsgHeader();

        /* Lower Layer instance which binds to TSAP */
        H223LowerLayer*   iLowerLayer;

        /* Outgoing logical channels */
        Oscl_Vector<H223OutgoingChannelPtr, OsclMemAllocator> iOutgoingChannels;

        /* Incoming logical channels */
        Oscl_Vector<H223IncomingChannelPtr, OsclMemAllocator> iIncomingChannels;

        int iALIndex[MAX_LCNS];

        /* Sizes of current packets being muxed */
        MuxSduDataList iNonSegmentableSduDataList;
        MuxSduDataList iSegmentableSduDataList;
        MuxSduDataList iControlSduDataList;
        /* A '1' in the bit pos indicates data is available for this lcn */
        unsigned  iLcnsWithData;

        /* segmentable lcn ending on current pdu */
        H223IncomingChannel* iLcnEnd;

        /* bitrate, bps */
        uint32 iBitrate;

        /* operational state */
        bool iState;

        /* Mux table manager*/
        MuxTableMgr* iMuxTblMgr;

        /* SDU sizes */
        int iMaxAlSduSize[4];
        int iMaxAlSduSizeR[4];

        int iAlSduSize[4];
        int iAlSduSizeR[4];

        OSCL_wHeapString<OsclMemAllocator> iInstanceId;

        TPVLoopbackMode iLoopbackMode;

        /* Use this for aborting a segmentable mux sdu */
        DefFragmentAllocator iAlloc;
        MediaFragment* iDummyFragment;

        OsclMemPoolFixedChunkAllocator* iMediaDataImplMemAlloc;
        PVMFMediaFragGroupCombinedAlloc<OsclMemAllocator>* iMuxPduPacketAlloc;
        PVLogger *iLogger;
        PVMFMediaClock* iClock;
        OsclRand iRand;
        uint16 iMinSampleInterval;
        uint16 iNumMuxIntervalsPerTimerInterval;
        uint16 iNumBytesPerMinSampleInterval;
        uint16 iMultiplexingDelayMs;
        uint32 iInLogicalChannelBufferingMs;
        uint32 iOutLogicalChannelBufferingMs;
        uint32 iFmStuffingRepeatInterval;
        uint32 iFmStuffingMode;
        uint32 iPduNum;
        CPVH223MultiplexObserver* iObserver;
        bool iLevelSetupComplete;
        uint32 iInterleavingMultiplexFlagsSize;
        uint8* iInterleavingMultiplexFlags;
        OsclSharedPtr<PVMFMediaDataImpl> iInterleavingPacket;
        bool iEnableStuffing;
        bool iStuffOnlyFirstSend;
};

typedef CPVH223Multiplex H223;
#endif
