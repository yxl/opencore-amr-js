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
#if !defined(LOWER_LAYER_H)
#define LOWER_LAYER_H
#include "oscl_types.h"
#include "oscl_time.h"
#ifndef OSCL_TIMER_H_INCLUDED
#include "oscl_timer.h"
#endif
#include "pvt_params.h"
#include "pduparcom.h"
#include "layer.h"
#include "media_packet.h"

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
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

#ifndef OSCL_MEM_MEMPOOL_H_INCLUDED
#include "oscl_mem_mempool.h"
#endif

#ifndef PVMF_MEDIA_DATA_H_INCLUDED
#include "pvmf_media_data.h"
#endif

#ifndef PVMF_SIMPLE_MEDIA_BUFFER_H_INCLUDED
#include "pvmf_simple_media_buffer.h"
#endif

#ifndef PVMF_NODE_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
#endif

#ifndef PVMF_PORT_BASE_IMPL_H_INCLUDED
#include "pvmf_port_base_impl.h"
#endif

#ifndef PVMF_POOL_BUFFER_ALLOCATOR_H_INCLUDED
#include "pvmf_pool_buffer_allocator.h"
#endif

#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif

#include "h223types.h"
#define H223_MAX_SEND_SIZE  1024
#define DEFAULT_BITRATE 64000
#define H223_DEMUX_BUFFER_INTERVAL_MS 0

class LowerLayerObserver
{
    public:
        virtual ~LowerLayerObserver() {}
        virtual PVMFStatus GetOutgoingMuxPdus(MuxPduPacketList& packets) = 0;
        virtual void LevelSetupComplete(PVMFStatus status, TPVH223Level level) = 0;
        virtual void DataReceptionStart() = 0;
        virtual H223PduParcomObserver* GetParcomObserver()  = 0;
        virtual void LowerLayerError(TPVDirection direction, PVMFStatus error) = 0;
};


/* Lower layer which binds to Network IO */
class H223LowerLayer : public PvmfPortBaseImpl,
            public PVMFPortActivityHandler,
            public OsclTimerObject,
            public OsclTimerObserver,
            public H223PduParcomObserver
{
    public:
        H223LowerLayer(int32 aPortTag, TPVLoopbackMode aLoopbackMode);
        ~H223LowerLayer();

        void SetObserver(LowerLayerObserver* observer)
        {
            iObserver = observer;
            if (iParcom.GetRep())
                iParcom->SetObserver(iObserver->GetParcomObserver());
        }

        LowerLayerObserver* GetObserver()
        {
            return iObserver;
        }

        TPVStatusCode SetTimerRes(uint32 timer_res)
        {
            iMinTimerResolution = timer_res;
            UpdatePduSize();
            return EPVT_Success;
        }

        TPVStatusCode Open();
        TPVStatusCode Close();
        TPVStatusCode Start(H223PduParcomSharedPtr parcom);
        TPVStatusCode Stop();

        PVMFStatus Connect(PVMFPortInterface* aPort);
        void HandlePortActivity(const PVMFPortActivity &);
        //Force lower layer to stop.
        int Abort();

        TPVStatusCode SetLevel(TPVH223Level muxLevel);

        TPVH223Level GetLevel()
        {
            return iParcom->GetLevel();
        }

        TPVStatusCode SetBitrate(uint32 bitRate);

        TPVStatusCode SetTransmitProfile(bool iAnxA, bool iAnxADf, bool iAnxB, bool iAnxBOh)
        {
            iUseAnnexA = iAnxA;
            iUseAnnexADoubleFlag = iAnxADf;
            iUseAnnexB = iAnxB;
            iUseAnnexBwithHeader = iAnxBOh;
            return EPVT_Success;
        }

        TPVStatusCode SetMaxOutgoingPduSize(uint16 Size);

        int GetMinTimerRes()
        {
            return iMinTimerResolution;
        }
        /* Implement PAcketInput: process incoming packet */
        void PacketIn(PVMFSharedMediaDataPtr aMediaData);

        /* Dispatch outgoing packet */
        void DispatchPacket(OsclSharedPtr<PVMFMediaDataImpl>& pack);

        void SetLoopbackMode(TPVLoopbackMode aLoopbackMode);

        int iTimerIntervalNum;
        unsigned int iMinTimerResolution;

        void TimeoutOccurred(int32 timerID, int32 timeoutInfo);

        /* H223PduParcomObserver virtuals */
        uint32 MuxPduIndicate(uint8* pPdu, uint32 pduSz, int32 fClosing, int32 muxCode);
        void MuxPduErrIndicate(EMuxPduError err);
        void MuxSetupComplete(PVMFStatus status, TPVH223Level level);
        unsigned SetStuffingSize(TPVH223Level level);

        unsigned GetStuffingSize()
        {
            return iParcom->GetStuffingSz();
        }
        unsigned GetHeaderSize()
        {
            return iParcom->GetHeaderSz();
        }

        unsigned SetPduSize(unsigned pdu_size);
        unsigned GetPduSize()
        {
            return iPduSize;
        }
        unsigned GetSendSize();

        void ResetStats();
        void LogStats(TPVDirection dir);
        int32 GetTimestamp();
        void GetHdrFragment(OsclRefCounterMemFrag& hdr_frag);
        PVMFStatus CompletePacket(OsclSharedPtr<PVMFMediaDataImpl>& packet, int mt_entry, int pm);
        void DoStuffing(TPVH223Level level,
                        OsclSharedPtr<PVMFMediaDataImpl>& pkt,
                        uint32 stuffing_sz,
                        uint8 mux_code = 0);
        void SendStuffingMsgHeader(uint8* stuffing, uint16 num_bytes);
    protected:
        PVMFStatus PutData(PVMFSharedMediaMsgPtr aMsg);
        void Parse(uint8* buf, uint16 size);
        bool CheckLevel(uint8* pData, uint16 size , uint16* used_bytes, int* sync_detected);
        bool CheckLevelWithSync(uint8* pData, uint16 size, uint16* used_bytes) ;
        void SignalLevelSetupComplete(PVMFStatus status);

        void Run();
        void DoCancel();
    private:
        void InitParams();
        void InitRuntimeParams();
        PVMFStatus Mux();
        void UpdatePduSize();
        void CreateParcom(TPVH223Level level);
        void MuxException();

        LowerLayerObserver* iObserver;

        /* Transmit profile */
        bool iUseAnnexA;
        bool iUseAnnexADoubleFlag;
        bool iUseAnnexB;
        bool iUseAnnexBwithHeader;

        /* multiplex level */
        bool iLevelSetupComplete;

        /* Stuffing buffers for each level.  Pre-allocated and initialized with stuffing for that level */
        PVMFBufferPoolAllocator* iMemFragmentAlloc;
        OsclRefCounterMemFrag iStuffingFrag[MAX_H223_LEVELS+1];

        uint16 iStuffingSz[8];

        /* bitrate, bps */
        uint32 iBitrate;

        FragmentAllocator* iHdrFragmentAlloc;

        PVMFMediaFragGroupCombinedAlloc<OsclMemAllocator>* iDispatchPacketAlloc;
        OsclMemPoolFixedChunkAllocator* iMediaDataImplMemAlloc;

        OsclMemPoolFixedChunkAllocator* iMediaMsgPoolAlloc;

        OsclSharedPtr<PVMFMediaDataImpl> iDispatchPacket;
        MuxPduPacketList iSendPacketList;

        /* Send MUX-PDU size */
        unsigned iSendPduSz;
        unsigned iPduSize;
        unsigned iStuffingSize;

        /* Timer stuff */
        OsclTimer<OsclMemAllocator> iTimer;

        /* operational state */
        int iState;
        int iStateVal;

        /* PDU parcoms */
        H223PduParcomSharedPtr iParcom;
        TPVLoopbackMode iLoopbackMode;

        int iTimerCnt;
        int iBytesSent;
        uint32 iSendTimes;

        TimeValue iStartTimeTx;
        uint32 iNumBytesTx;
        uint32 iNumMuxSduBytesTx;
        uint32 iNumStuffingBytesTx;
        uint32 iNumPdusTx;
        uint32 iNumPmTx;

        TimeValue iStartTimeRx;
        uint32 iNumBytesRx;

        TimeValue  iLevelSetupTime;

        // Allocator for simple media data buffer
        OsclMemAllocator iMemAlloc;
        PVMFSimpleMediaBufferCombinedAlloc iMediaDataAlloc;
        TimeValue iStartMuxTime;
        TimeValue iLastMuxTime;

        PVLogger* iLogger;
        uint8* iDemuxBuffer;
        uint8* iDemuxBufferPos;
        uint32 iDemuxBufferSize;
        uint32 iSyncCheckCount;
        bool iSkipLevelCheck;
        uint8* iIdleSyncCheckBuffer;
        bool iSyncDetected;
        bool iDataReceptionStart;
};


#endif
