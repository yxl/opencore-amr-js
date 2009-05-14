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
#if !defined(LOGICAL_CHANNEL_H)
#define LOGICAL_CHANNEL_H
#include "oscl_mem.h"
#include "adaptationlayer.h"
#include "h324utils.h"

#ifndef OSCL_MEM_MEMPOOL_H_INCLUDED
#include "oscl_mem_mempool.h"
#endif

#ifndef PVMF_MEDIA_CLOCK_H_INCLUDED
#include "pvmf_media_clock.h"
#endif

#ifndef OSCL_MAP_H_INCLUDED
#include "oscl_map.h"
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

#ifndef PVMF_MEDIA_FRAG_GROUP_H_INCLUDED
#include "pvmf_media_frag_group.h"
#endif

#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif

#ifndef PVMI_CONFIG_AND_CAPABILITY_H_INCLUDED
#include "pvmi_config_and_capability_utils.h"
#endif

#define INVALID_MUX_CODE 0xFF
#define DEF_NUM_MEDIA_DATA 100
#define SKEW_CHECK_INTERVAL 2000
#define PARSING_JITTER_DURATION 200
#define	PVDEBUG_LOG_BITSTREAM_PKT(iDebug, comp, pkt) \
{\
uint8* ptrbuf = pkt->GetMediaPtr();\
PVDEBUG_LOG_BITSTREAM(iDebug, (comp, ptrbuf, pkt->GetMediaSize()) );\
pkt->ClearMediaPtr();\
}

class LCMediaDataEntry
{
    public:
        LCMediaDataEntry() : next(NULL) {}
        ~LCMediaDataEntry()
        {
            mediaData.Unbind();
        }
        PVMFSharedMediaDataPtr mediaData;
        LCMediaDataEntry* next;
};

class LcnAlloc : public Oscl_DefAlloc
{
    public:
        void* allocate(const uint32 size)
        {
            void* tmp = (void*)OSCL_DEFAULT_MALLOC(size);
            OSCL_ASSERT(tmp != 0);
            return tmp;
        }
        void deallocate(void* p)
        {
            OSCL_DEFAULT_FREE(p);
        }
};

class LogicalChannelObserver
{
    public:
        virtual ~LogicalChannelObserver() {}
        virtual int32 GetTimestamp() = 0;
        virtual void LogicalChannelError(TPVDirection direction, TPVChannelId id, PVMFStatus error) = 0;
        virtual void SkewDetected(TPVChannelId lcn1, TPVChannelId lcn2, uint32 skew) = 0;
        virtual void ReceivedFormatSpecificInfo(TPVChannelId lcn, uint8* fsi, uint32 fsi_len) = 0;
};

class H223LogicalChannel : public PvmfPortBaseImpl,
            public PVMFPortActivityHandler,
            public PvmiCapabilityAndConfig,
            public virtual LogicalChannelInfo
{
    public:
        H223LogicalChannel(TPVChannelId num,
                           bool segmentable,
                           OsclSharedPtr<AdaptationLayer>& al,
                           PS_DataType data_type,
                           LogicalChannelObserver* observer,
                           uint32 bitrate,
                           uint32 sample_interval,
                           uint32 num_media_data);
        virtual ~H223LogicalChannel();

        /* allocate resources in this function */
        virtual void Init() = 0;

        // LogicalChannelInfo virtuals
        TPVChannelId GetLogicalChannelNumber()
        {
            return lcn;
        }

        uint32 GetSduSize()
        {
            return iAl->GetSduSize();
        }

        bool IsSegmentable()
        {
            return iSegmentable;
        }

        uint32 GetBitrate()
        {
            return iBitrate;
        }

        uint32 GetSampleInterval()
        {
            return iSampleInterval;
        }

        const uint8* GetFormatSpecificInfo(uint32* format_specific_info_len);
        PVMFTimestamp GetLastSduTimestamp()
        {
            return iLastSduTimestamp;
        }

        PVMFFormatType GetFormatType()
        {
            PVCodecType_t codec_type = GetCodecType(iDataType);
            return PVCodecTypeToPVMFFormatType(codec_type);
        }

        OsclAny SetNext(H223LogicalChannel* lcn_next)
        {
            next = lcn_next;
        }
        H223LogicalChannel* GetNext()
        {
            return next;
        }

        OsclSharedPtr<AdaptationLayer>GetAl()
        {
            return iAl;
        }

        OsclAny SetSampleInterval(uint16 sample_interval)
        {
            iSampleInterval = sample_interval;
        }

        /* Flushes the pending AL SDU data */
        virtual OsclAny Flush() = 0;

        virtual OsclAny ResetStats() = 0;
        virtual OsclAny LogStats() = 0;

        // Functions to pause and resume
        virtual void Pause();
        virtual void Resume();

        // Set format specific information
        PVMFStatus SetFormatSpecificInfo(uint8* info, uint16 info_len);

        void SetTimestampOffset(uint32 offset)
        {
            iIncomingSkew = offset;
        }
        void SetDatapathLatency(uint32 aLatency);

        void SetClock(PVMFMediaClock* aClock)
        {
            iClock = aClock;
        }

        //CapabilityAndConfig virtuals
        OSCL_IMPORT_REF void QueryInterface(const PVUuid& aUuid, OsclAny*& aPtr);

        /* PVMFPortActivityHandler virtuals */
        void HandlePortActivity(const PVMFPortActivity &aActivity)
        {
            OSCL_UNUSED_ARG(aActivity);
        }
        PVMFStatus setConfigParametersSync(PvmiKvp* selectedKvp, PvmiCapabilityAndConfig* aConfig, PVMFFormatType lcn_format_type = PVMF_MIME_FORMAT_UNKNOWN, bool aTryTwice = false);



        //Set the audio/video MIO latencies for the respective channel
        void SetAudioLatency(int32 aAudioLatency)
        {
            iAudioLatency = aAudioLatency;
        }
        void SetVideoLatency(int32 aVideoLatency)
        {
            iVideoLatency = aVideoLatency;
        }

    protected:
        TPVChannelId lcn;
        bool iSegmentable;
        H223LogicalChannel* next;
        OsclSharedPtr<AdaptationLayer> iAl;
        uint32 iBitrate;
        uint32 iSampleInterval;
        LogicalChannelObserver* iObserver;
        uint8* iFormatSpecificInfo;
        uint32 iFormatSpecificInfoLen;
        PVLogger* iLogger;
        uint32 iIncomingSkew;
        PVMFTimestamp iLastSduTimestamp;
        PS_DataType iDataType;
        OsclMemAllocator iKvpMemAlloc;
        uint32 iNumMediaData;
        uint32 iMaxSduSize;
        bool iSendFormatSpecificInfo;
        uint32 iDatapathLatency;
        PVMFFormatType iMediaType;
        bool iPaused;
        PVMFMediaClock* iClock;
        int32 iAudioLatency;
        int32 iVideoLatency;

};

/* For outgoing A/V/C ( to be muxed) */
class H223OutgoingChannel : public H223LogicalChannel
{
    public:
        H223OutgoingChannel(TPVChannelId num,
                            bool segmentable,
                            OsclSharedPtr<AdaptationLayer>& al,
                            PS_DataType data_type,
                            LogicalChannelObserver* observer,
                            uint32 bitrate,
                            uint32 sample_interval,
                            uint32 num_media_data);
        ~H223OutgoingChannel();

        void Init();

        TPVDirection GetDirection()
        {
            return OUTGOING;
        }
        bool GetNextPacket(PVMFSharedMediaDataPtr& aMediaData, PVMFStatus aStatus);

        OsclAny ReleasePacket(PVMFSharedMediaDataPtr& aMediaData);

        OsclAny Flush();

        OsclAny ResetStats();
        OsclAny LogStats();
        OsclAny SetSkewReference(LogicalChannelInfo* reference_channel)
        {
            iSkewReferenceChannel = reference_channel;
        }

        void BufferMedia(uint16 aMs);
        void SetBufferSizeMs(uint32 buffer_size_ms);

        uint32 GetNumBytesTransferred()
        {
            return iNumBytesIn;
        }
        // Functions to pause and resume the output of data from the logical channel to the mux
        void Resume();

        OSCL_IMPORT_REF PVMFStatus Connect(PVMFPortInterface* aPort);
        OSCL_IMPORT_REF virtual PVMFStatus PeerConnect(PVMFPortInterface* aPort);

        // Implement pure virtuals from PvmiCapabilityAndConfig interface
        OSCL_IMPORT_REF void setObserver(PvmiConfigAndCapabilityCmdObserver* aObserver);
        OSCL_IMPORT_REF PVMFStatus getParametersSync(PvmiMIOSession aSession, PvmiKeyType aIdentifier,
                PvmiKvp*& aParameters, int& num_parameter_elements,
                PvmiCapabilityContext aContext);
        OSCL_IMPORT_REF PVMFStatus releaseParameters(PvmiMIOSession aSession, PvmiKvp* aParameters, int num_elements);
        OSCL_IMPORT_REF void createContext(PvmiMIOSession aSession, PvmiCapabilityContext& aContext);
        OSCL_IMPORT_REF void setContextParameters(PvmiMIOSession aSession, PvmiCapabilityContext& aContext,
                PvmiKvp* aParameters, int num_parameter_elements);
        OSCL_IMPORT_REF void DeleteContext(PvmiMIOSession aSession, PvmiCapabilityContext& aContext);
        OSCL_IMPORT_REF void setParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters,
                                               int num_elements, PvmiKvp * & aRet_kvp);
        OSCL_IMPORT_REF PVMFCommandId setParametersAsync(PvmiMIOSession aSession, PvmiKvp* aParameters,
                int num_elements, PvmiKvp*& aRet_kvp, OsclAny* context = NULL);
        OSCL_IMPORT_REF uint32 getCapabilityMetric(PvmiMIOSession aSession);
        OSCL_IMPORT_REF PVMFStatus verifyParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters, int num_elements);
        /* PVMFPortActivityHandler virtuals */
        void HandlePortActivity(const PVMFPortActivity &aActivity);
    protected:
        virtual PVMFStatus PutData(PVMFSharedMediaMsgPtr media_msg);

        bool FragmentPacket(PVMFSharedMediaDataPtr& aMediaData, PVMFSharedMediaDataPtr& newpack);

        OsclSharedPtr<PVMFMediaDataImpl> StartAlPdu();

        PVMFStatus CompletePdu();

        PVMFStatus AppendOutgoingPkt(OsclSharedPtr<PVMFMediaDataImpl>& pdu, PVMFTimestamp timestamp,
                                     OsclRefCounterMemFrag* fsi = NULL);

        OsclAny ResetSkewParameters();


        PVMFStatus VerifyAndSetParameter(PvmiKvp* aKvp, bool aSetParam);

        PVMFStatus NegotiateInputSettings(PvmiCapabilityAndConfig* config);
        PVMFStatus NegotiateFSISettings(PvmiCapabilityAndConfig* config);
        PVMFStatus ReceivedFSIFromPeer(PvmiKvp* kvp);

        OsclMemPoolFixedChunkAllocator* iMediaMsgMemoryPool;
        OsclMemPoolFixedChunkAllocator* iMediaDataEntryAlloc;
        LCMediaDataEntry* lastMediaData;
        PVMFMediaFragGroupCombinedAlloc<OsclMemAllocator>* iMediaFragGroupAlloc;
        OsclMemPoolFixedChunkAllocator* iPduPktMemPool;
        OsclSharedPtr<PVMFMediaDataImpl> iCurPdu;

        TimeValue iCreateTime;
        TimeValue iStartTime;
        uint32 iNumPacketsIn;
        uint32 iNumSdusIn;
        uint32 iNumSdusDropped;
        uint32 iNumBytesIn;
        uint32 iNumSdusOut;
        uint32 iNumBytesOut;
        uint32 iMaxPacketMuxTime;
        uint32 iMaxSduMuxTime;
        uint32 iNumFlush;
        uint32 iNumBytesFlushed;
        // skew related
        LogicalChannelInfo* iSkewReferenceChannel;
        int32 iSetBufferMediaMs;
        int32 iSetBufferMediaBytes;
        int32 iBufferMediaMs;
        int32 iBufferMediaBytes;
        bool iMuxingStarted;
        PVMFTimestamp iCurPduTimestamp;
        uint32 iNumPendingPdus;
        PVLogger* iOutgoingAudioLogger;
        PVLogger* iOutgoingVideoLogger;
        bool iWaitForRandomAccessPoint;
        uint32 iBufferSizeMs;
        OsclRefCounterMemFrag iFsiFrag;
};

class H223OutgoingControlChannel : public H223OutgoingChannel
{
    public:
        H223OutgoingControlChannel(OsclSharedPtr<AdaptationLayer>& al,
                                   PS_DataType data_type,
                                   LogicalChannelObserver* observer,
                                   uint32 bitrate,
                                   uint32 sample_interval,
                                   uint32 num_media_data)
                : H223OutgoingChannel(0, SEGMENTABLE, al, data_type, observer, bitrate, sample_interval, num_media_data)
        {
        }

        PVMFStatus PutData(PVMFSharedMediaMsgPtr aMsg);
        OSCL_IMPORT_REF PVMFStatus PeerConnect(PVMFPortInterface* aPort);

};

#define NUM_INCOMING_SDU_BUFFERS 8
/* For incoming (from the remote terminal) A/V/C */
class H223IncomingChannel : public H223LogicalChannel
{
    public:
        H223IncomingChannel(TPVChannelId num,
                            bool segmentable,
                            OsclSharedPtr<AdaptationLayer>& al,
                            PS_DataType data_type,
                            LogicalChannelObserver* observer,
                            uint32 bitrate,
                            uint32 sample_interval,
                            uint32 num_media_data);
        ~H223IncomingChannel();
        void Init();

        TPVDirection GetDirection()
        {
            return INCOMING;
        }

        virtual PVMFStatus PutData(PVMFSharedMediaMsgPtr aMsg)
        {
            OSCL_UNUSED_ARG(aMsg);
            return PVMFErrNotSupported;
        }

        PVMFStatus GetData(PVMFSharedMediaMsgPtr aMsg)
        {
            OSCL_UNUSED_ARG(aMsg);
            return PVMFErrNotSupported;
        }

        /* Dispaches packets to bound PAcketInput */
        PVMFStatus AlPduData(uint8* buf, uint16 len);

        PVMFStatus AlDispatch();

        OsclAny ResetAlPdu();
        OsclAny AllocateAlPdu();
        OsclAny AppendAlPduFrag();
        uint32 CopyAlPduData(uint8* buf, uint16 len);
        uint32 CopyToCurrentFrag(uint8* buf, uint16 len);

        OsclAny Flush();

        OsclAny ResetStats();
        OsclAny LogStats();

        uint32 GetNumSdusIn()
        {
            return iNumSdusIn;
        }
        // overload Connect to send out format specific info if available
        PVMFStatus Connect(PVMFPortInterface* aPort);


        // Implement pure virtuals from PvmiCapabilityAndConfig interface
        OSCL_IMPORT_REF void setObserver(PvmiConfigAndCapabilityCmdObserver* aObserver);
        OSCL_IMPORT_REF PVMFStatus getParametersSync(PvmiMIOSession aSession, PvmiKeyType aIdentifier,
                PvmiKvp*& aParameters, int& num_parameter_elements,
                PvmiCapabilityContext aContext);
        OSCL_IMPORT_REF PVMFStatus releaseParameters(PvmiMIOSession aSession, PvmiKvp* aParameters, int num_elements);
        OSCL_IMPORT_REF void createContext(PvmiMIOSession aSession, PvmiCapabilityContext& aContext);
        OSCL_IMPORT_REF void setContextParameters(PvmiMIOSession aSession, PvmiCapabilityContext& aContext,
                PvmiKvp* aParameters, int num_parameter_elements);
        OSCL_IMPORT_REF void DeleteContext(PvmiMIOSession aSession, PvmiCapabilityContext& aContext);
        OSCL_IMPORT_REF void setParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters,
                                               int num_elements, PvmiKvp * & aRet_kvp);
        OSCL_IMPORT_REF PVMFCommandId setParametersAsync(PvmiMIOSession aSession, PvmiKvp* aParameters,
                int num_elements, PvmiKvp*& aRet_kvp, OsclAny* context = NULL);
        OSCL_IMPORT_REF uint32 getCapabilityMetric(PvmiMIOSession aSession);
        OSCL_IMPORT_REF PVMFStatus verifyParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters, int num_elements);
        /* PVMFPortActivityHandler virtuals */
        void HandlePortActivity(const PVMFPortActivity &aActivity);

    private:
        void PreAlPduData();
        PVMFStatus VerifyAndSetParameter(PvmiKvp* aKvp, bool aSetParam);
        PVMFStatus NegotiateOutputSettings(PvmiCapabilityAndConfig* config);

        OsclAny SendFormatSpecificInfo();

        PVMFStatus DispatchPendingSdus();

        PVMFStatus SendBeginOfStreamMediaCommand();

        void SetSampleTimestamps(PVMFTimestamp& aTSOffset);
        PVMFBufferPoolAllocator iMemFragmentAlloc;
        OsclMemPoolFixedChunkAllocator* iMediaMsgMemoryPool;
        PVMFMediaFragGroupCombinedAlloc<OsclMemAllocator>* iMediaFragGroupAlloc;
        OsclMemPoolFixedChunkAllocator* iPduPktMemPool;

        OsclMemAllocator iMemAlloc;
        PVMFSimpleMediaBufferCombinedAlloc iMediaDataAlloc;
        Oscl_Vector<PVMFSharedMediaMsgPtr, OsclMemAllocator> iPendingSdus;
        OsclSharedPtr<PVMFMediaDataImpl> iAlPduMediaData;
        uint8* iAlPduFragPos;
        OsclRefCounterMemFrag iAlPduFrag;
        uint32 iPduSize;
        uint32 iCurPduSize;
        TimeValue iCreateTime;
        TimeValue iStartTime;
        uint32 iNumPdusIn;
        uint32 iNumSdusIn;
        uint32 iNumBytesIn;
        uint32 iSduSizeExceededCnt;
        uint32 iNumCrcErrors;
        uint32 iNumSeqNumErrors;
        uint32 iNumAbort;
        uint32 iNumBytesFlushed;
        PVMFTimestamp iCurTimestamp;
        friend class TSC_324m;
        PVLogger* iIncomingAudioLogger;
        PVLogger* iIncomingVideoLogger;
        int32 iRenderingSkew;
};

class MuxSduData
{
    public:
        MuxSduData();
        OsclSharedPtr<H223OutgoingChannel> lcn;
        PVMFSharedMediaDataPtr sdu;
        uint16 size;
        uint16 cur_frag_num;
        uint16 cur_pos;
};
typedef Oscl_Vector<MuxSduData, OsclMemAllocator> MuxSduDataList;

#endif

