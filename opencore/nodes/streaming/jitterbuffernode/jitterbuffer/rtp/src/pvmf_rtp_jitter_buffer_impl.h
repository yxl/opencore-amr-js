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
#ifndef PVMF_RTP_JITTER_BUFFER_IMPL_H_INCLUDED
#define PVMF_RTP_JITTER_BUFFER_IMPL_H_INCLUDED

#ifndef PVMF_JITTER_BUFFER_H_INCLUDED
#include "pvmf_jitter_buffer.h"
#endif

#ifndef PVMF_MEDIA_CLOCK_H_INCLUDED
#include "pvmf_media_clock.h"
#endif

#ifndef PVMF_JITTER_BUFFER_COMMON_TYPES_H_INCLUDED
#include "pvmf_jitter_buffer_common_types.h"
#endif

#define PVMF_JITTER_BUFFER_ROLL_OVER_THRESHOLD_16BIT 2000

class PVMFRTPJitterBufferImpl: public PVMFJitterBufferImpl
{
    public:
        OSCL_IMPORT_REF static PVMFJitterBuffer* New(const PVMFJitterBufferConstructParams& aCreationData);
        OSCL_IMPORT_REF virtual ~PVMFRTPJitterBufferImpl();
        OSCL_IMPORT_REF virtual void StreamingSessionStarted();
        OSCL_IMPORT_REF virtual void ResetJitterBuffer();

        OSCL_IMPORT_REF void AdjustRTPTimeStamp();
        OSCL_IMPORT_REF virtual void setRTPInfoParams(PVMFRTPInfoParams rtpInfoParams, bool oPlayAfterASeek);
        OSCL_IMPORT_REF uint32 getInterArrivalJitter();
        OSCL_IMPORT_REF virtual void PurgeElementsWithSeqNumsLessThan(uint32 aSeqNum, uint32 aPlayerClockMS);
        OSCL_IMPORT_REF virtual void PurgeElementsWithTimestampLessThan(PVMFTimestamp aTS);
        OSCL_IMPORT_REF virtual PVMFSharedMediaDataPtr& GetFirstDataPacket(void);
        OSCL_IMPORT_REF virtual bool GetRTPTimeStampOffset(uint32& aTimeStampOffset);
        OSCL_IMPORT_REF virtual void  SetRTPTimeStampOffset(uint32 newTSBase);
        OSCL_IMPORT_REF virtual bool NotifyFreeSpaceAvailable();
        OSCL_IMPORT_REF virtual void SetEarlyDecodingTimeInMilliSeconds(uint32 duration);
        OSCL_IMPORT_REF virtual void SetBurstThreshold(float burstThreshold);
        OSCL_IMPORT_REF virtual bool IsDelayEstablished(uint32& aClockDiff);
        OSCL_IMPORT_REF bool IsSeqTsValidForPkt(uint32 aSeqNum, uint32 aTs, PVMFJitterBufferStats& jbStats);

    protected:
        void Construct();
        virtual bool CanRetrievePacket();
        virtual bool CanRetrievePacket(PVMFSharedMediaMsgPtr& aMediaOutMsg, bool& aCmdPacket);
        virtual PVMFJBPacketParsingAndStatUpdationStatus ParsePacketHeaderAndUpdateJBStats(PVMFSharedMediaDataPtr& inDataPacket,
                PVMFSharedMediaDataPtr& outDataPacket,
                uint32 aFragIndex = 0);
        PVMFRTPJitterBufferImpl(const PVMFJitterBufferConstructParams& aCreationData);

        //virtual uint32 GetNumOfPackets(PVMFSharedMediaMsgPtr& aMsg) const;
        virtual void EOSCmdReceived();
        void UpdateInterArrivalJitter(PVMFTimestamp currPacketTS);
        void UpdateEstimatedServerClock(bool oFreshStart = false);

        //PVMFJitterBufferRegisterMediaMsgStatus addPacket(PVMFSharedMediaDataPtr& aDataPacket);
        virtual void CheckForRTPTimeAndRTPSeqNumberBase()
        {
            if (iRTPInfoParamsVec.size() > 0)
            {
                Oscl_Vector<PVMFRTPInfoParams, OsclMemAllocator>::iterator it;
                it = iRTPInfoParamsVec.begin();
                if (it->rtpTimeBaseSet == false)
                {
                    /* Use the value from the first packet */
                    if (seqNumLock)
                    {
                        iPrevTSOut = seqLockTimeStamp;
                        iPrevTSIn = seqLockTimeStamp;
                        iPrevAdjustedRTPTS = seqLockTimeStamp;
                    }
                }
                if (it->seqNumBaseSet == false)
                {
                    /* Use the value from the first packet */
                    if (seqNumLock)
                    {
                        iPrevSeqNumBaseOut = iFirstSeqNum;
                        iPrevSeqNumBaseIn = iFirstSeqNum;
                    }
                }
            }
        }

        PVMFRTPInfoParams *FindRTPInfoParams(uint32 aSeqNum)
        {
            if (iRTPInfoParamsVec.size() == 1)
            {
                return (iRTPInfoParamsVec.begin());
            }

            PVMFRTPInfoParams* retVal = NULL;
            Oscl_Vector<PVMFRTPInfoParams, OsclMemAllocator>::iterator it;


            for (it = iRTPInfoParamsVec.begin();
                    it < iRTPInfoParamsVec.end();
                    it++)
            {
                if (it->seqNum <= aSeqNum)
                {
                    retVal = it;
                }
            }
            return retVal;
        }
        bool IsSequenceNumEarlier(uint16 aSeqNumToComp, uint16 aBaseSeqNum, uint16& aDiff);
        void ReportJBInfoEvent(PVMFAsyncEvent& aEvent);
        void UpdatePacketArrivalStats(PVMFSharedMediaDataPtr& aArrivedPacket);
        void DeterminePrevTimeStampPeek(uint32 aSeqNum,
                                        PVMFTimestamp& aPrevTS);
        void DeterminePrevTimeStamp(uint32 aSeqNum);
        void ComputeMaxAdjustedRTPTS();
        PVMFStatus PerformFlowControl(bool aIncomingPacket);

        PVMFMediaClock* iPacketArrivalClock;
        PVMFTimebase_Tickcount iPacketArrivalTimeBase;
        PVMFTimestamp iPrevPacketTS;
        double iInterArrivalJitterD;
        uint32 iPrevPacketRecvTime;

        //Burst detection variables:
        bool			iBurstDetect;
        uint32			iBurstStartTimestamp;
        uint32			iEstServerClockBurstStartTimestamp;
        PVMFMediaClock  *iBurstClock;				//may be wallclock owned by the jitterbuffer Misc can be used instead
        PVMFTimebase_Tickcount iBurstClockTimeBase;
        bool			iRTPDataArrived;
        uint32			iEarlyDecodingTime;
        bool			iServerBurst;
        float			iBurstThreshold;
        uint32			iBurstDetectDurationInMilliSec;
        bool			iInitialBuffering;

        uint32			iPlayListRTPTimeBase;
        bool			iPlayListRTPTimeBaseSet;

        bool   isPrevRtpTimeSet;
        uint32 iPrevRtpTimeBase;
        bool   isPrevNptTimeSet;
        uint32 iPrevNptTimeInRTPTimeScale;
        MediaClockConverter iMediaClockConvertor;

};
#endif


