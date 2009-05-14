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
#ifndef PVMF_JITTER_BUFFER_H_INCLUDED
#define PVMF_JITTER_BUFFER_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif
#ifndef OSCL_STRING_CONTAINERS_H_INCLUDED
#include "oscl_string_containers.h"
#endif
#ifndef OSCL_MEM_MEMPOOL_H_INCLUDED
#include "oscl_mem_mempool.h"
#endif
#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif
#ifndef PVMF_TIMESTAMP_H_INCLUDED
#include "pvmf_timestamp.h"
#endif
#ifndef PVMF_MEDIA_DATA_H_INCLUDED
#include "pvmf_media_data.h"
#endif
#ifndef PVMF_MEDIA_FRAG_GROUP_H_INCLUDED
#include "pvmf_media_frag_group.h"
#endif
#ifndef PVMF_SM_TUNABLES_H_INCLUDED
#include "pvmf_sm_tunables.h"
#endif
#ifndef __MEDIA_CLOCK_CONVERTER_H
#include "media_clock_converter.h"
#endif
#ifndef PVMF_MEDIA_MSG_FORMAT_IDS_H_INCLUDED
#include "pvmf_media_msg_format_ids.h"
#endif

#ifndef PVMF_EVENT_HANDLING_H_INCLUDED
#include "pvmf_event_handling.h"
#endif

#ifndef PVMF_JITTER_BUFFER_COMMON_TYPES_H_INCLUDED
#include "pvmf_jitter_buffer_common_types.h"
#endif

#ifndef PVMF_MEDIA_CLOCK_H_INCLUDED
#include "pvmf_media_clock.h"
#endif

#ifndef PVMF_JITTER_BUFFER_COMMON_INTERNAL_H
#include "pvmf_jitter_buffer_common_internal.h"
#endif
#ifndef OSCL_DLL_H_INCLUDED
#include "oscl_dll.h"
#endif

#ifndef PVMF_JB_EVENT_NOTIFIER_H
#include "pvmf_jb_event_notifier.h"
#endif

#ifndef PVMF_FORMAT_TYPE_H_INCLUDED
#include "pvmf_format_type.h"
#endif

class PVMFSMSharedBufferAllocWithReSize;


class PVMFJitterBufferStats
{
    public:
        PVMFJitterBufferStats()
        {
            ResetStats();
        }

        void ResetStats()
        {
            totalNumPacketsReceived = 0;
            totalNumPacketsRegistered = 0;
            totalPacketsLost = 0;
            totalNumPacketsRetrieved = 0;
            //Timestamp info
            maxTimeStampRegistered = 0;
            maxTimeStampRetrieved = 0;

            //Occupancy related
            maxOccupancy = 0;
            currentOccupancy = 0;
            totalNumBytesRecvd = 0;
            packetSizeInBytesLeftInBuffer = 0;

            //Seq num info
            maxSeqNumReceived = 0;
            maxSeqNumRegistered = 0;
            lastRegisteredSeqNum = 0;
            lastRetrievedSeqNum = 0;
            seqNumBase = 0;

            maxTimeStampRetrievedWithoutRTPOffset = 0;
            ssrc = 0;
        }

        //Packet info
        uint32 totalNumPacketsReceived;
        uint32 totalNumPacketsRegistered;
        uint32 totalPacketsLost;
        uint32 totalNumPacketsRetrieved;

        //Timestamp info
        PVMFTimestamp maxTimeStampRegistered;
        PVMFTimestamp maxTimeStampRetrieved;

        //Occupancy related
        uint32 maxOccupancy;
        uint32 currentOccupancy;
        uint32 totalNumBytesRecvd;
        uint32 packetSizeInBytesLeftInBuffer;

        //Seq num info
        uint32 maxSeqNumReceived;
        uint32 maxSeqNumRegistered;
        uint32 lastRegisteredSeqNum;
        uint32 lastRetrievedSeqNum;
        uint32 seqNumBase;

        PVMFTimestamp maxTimeStampRetrievedWithoutRTPOffset;
        uint32 ssrc;
};

class MediaCommandMsgHolder
{
    public:
        MediaCommandMsgHolder()
        {
            iPreceedingMediaMsgSeqNumber = 0xFFFFFFFF;
        };

        MediaCommandMsgHolder(const MediaCommandMsgHolder& a)
        {
            iPreceedingMediaMsgSeqNumber = a.iPreceedingMediaMsgSeqNumber;
            iCmdMsg = a.iCmdMsg ;
        }

        MediaCommandMsgHolder& operator=(const MediaCommandMsgHolder& a)
        {
            if (&a != this)
            {
                iPreceedingMediaMsgSeqNumber = a.iPreceedingMediaMsgSeqNumber;
                iCmdMsg = a.iCmdMsg ;
            }
            return (*this);
        }

        uint32 iPreceedingMediaMsgSeqNumber;
        PVMFSharedMediaMsgPtr iCmdMsg;
};

typedef enum
{
    PVMF_JITTER_BUFFER_ADD_ELEM_ERROR,
    PVMF_JITTER_BUFFER_ADD_ELEM_PACKET_OVERWRITE,
    PVMF_JITTER_BUFFER_ADD_ELEM_SUCCESS
} PVMFJitterBufferAddElemStatus;

template<class Alloc>
class PVMFDynamicCircularArray
{
    public:
        PVMFDynamicCircularArray()
        {
            iNumElems = 0;
            iArraySize = 0;
            iMaxSeqNumAdded = 0;
            iLastRetrievedSeqNum = 0;
            iLastRetrievedTS = 0;
            iReadOffset = 0;
            iFirstSeqNumAdded = 0;

            ipLogger = PVLogger::GetLoggerObject("PVMFDynamicCircularArray");
            ipDataPathLoggerIn = PVLogger::GetLoggerObject("datapath.sourcenode.jitterbuffer.in");
            ipDataPathLoggerOut = PVLogger::GetLoggerObject("datapath.sourcenode.jitterbuffer.out");
        }

        PVMFDynamicCircularArray(uint32 n)
        {
            iNumElems = 0;
            iArraySize = n;
            iMaxSeqNumAdded = 0;
            iLastRetrievedSeqNum = 0;
            iLastRetrievedTS = 0;
            iReadOffset = 0;
            iFirstSeqNumAdded = 0;

            iMediaPtrVec.reserve(iArraySize);
            InitVector(iArraySize);

            ipLogger = PVLogger::GetLoggerObject("PVMFDynamicCircularArray");
            ipDataPathLoggerIn = PVLogger::GetLoggerObject("datapath.sourcenode.jitterbuffer.in");
            ipDataPathLoggerOut = PVLogger::GetLoggerObject("datapath.sourcenode.jitterbuffer.out");
        }

        virtual ~PVMFDynamicCircularArray()
        {
            if (!iMediaPtrVec.empty())
            {
                typedef typename Oscl_Vector<PVMFSharedMediaDataPtr, Alloc>::iterator iterator_type;
                iterator_type it;

                for (it = iMediaPtrVec.begin(); it != iMediaPtrVec.end(); it++)
                {
                    if (it->GetRep() != NULL)
                    {
                        it->Unbind();
                    }
                }
            }
        }

        void Clear()
        {
            PVMF_JB_LOGINFO((0, "PVMFDynamicCircularArray::Clear - Cleared Jitter Buffer"));
            typedef typename Oscl_Vector<PVMFSharedMediaDataPtr, Alloc>::iterator iterator_type;
            iterator_type it;
            for (it = iMediaPtrVec.begin(); it != iMediaPtrVec.end(); it++)
            {
                if (it->GetRep() != NULL)
                {
                    it->Unbind();
                }
            }
            iNumElems = 0;
            iMediaPtrVec.clear();
            InitVector(iArraySize);

            iMaxSeqNumAdded = 0;
            iLastRetrievedSeqNum = 0;
            iLastRetrievedTS = 0;
            iReadOffset = 0;
            iFirstSeqNumAdded = 0;
            iJitterBufferStats.currentOccupancy = 0;
            iJitterBufferStats.packetSizeInBytesLeftInBuffer = 0;
        }

        void ResetJitterBufferStats()
        {
            iJitterBufferStats.ResetStats();
        }

        uint32 getNumElements()
        {
            return iNumElems;
        }

        uint32 getArraySize()
        {
            return iArraySize;
        }

        void growCircularArray(uint32 newSize)
        {
            if (newSize > iArraySize)
            {
                /*
                 * This transfers the existing contents
                 * as well
                 */
                iMediaPtrVec.reserve(newSize);
                /* Initialize the new space */
                InitVector((newSize - iArraySize));
                iArraySize = newSize;
            }
        }

        void setFirstSeqNumAdded(uint32 aFirstSeqNumAdded)
        {
            iFirstSeqNumAdded = aFirstSeqNumAdded;
        }

        //validations on timestamp and seqnum has to be done by the user of the dynamic circular array.
        PVMFJitterBufferAddElemStatus addElement(PVMFSharedMediaDataPtr& elem, uint32 aSeqNumBase)
        {
            PVMFJitterBufferAddElemStatus oRet = PVMF_JITTER_BUFFER_ADD_ELEM_SUCCESS;

            iJitterBufferStats.totalNumPacketsReceived++;
            iJitterBufferStats.ssrc = elem->getStreamID();
            uint32 seqNum = elem->getSeqNum();
            /* Get packet size */
            uint32 size = 0;
            uint32 numFragments = elem->getNumFragments();
            for (uint32 i = 0; i < numFragments; i++)
            {
                OsclRefCounterMemFrag memFragIn;
                elem->getMediaFragment(i, memFragIn);
                size += memFragIn.getMemFrag().len;
            }
            iJitterBufferStats.totalNumBytesRecvd += size;
            iJitterBufferStats.packetSizeInBytesLeftInBuffer += size;

            //Assumptions:
            //- Validations based on ts and SeqNum are performed by the calling code,
            //- By the time the code flows at this point we had already resetted the
            //  maxSeqNumRegistered to rolled over value in the derived implementation

            if (seqNum > iJitterBufferStats.maxSeqNumRegistered)
            {
                iJitterBufferStats.maxSeqNumReceived = seqNum;
            }
            uint32 offset = (seqNum - aSeqNumBase) % iArraySize;

            PVMFSharedMediaDataPtr currElem = iMediaPtrVec[offset];
            if (currElem.GetRep() == NULL)
            {
                /* Register Packet */
                iMediaPtrVec[offset] = elem;
                iNumElems++;
                iJitterBufferStats.totalNumPacketsRegistered++;
                iJitterBufferStats.lastRegisteredSeqNum = seqNum;
                if (seqNum > iJitterBufferStats.maxSeqNumRegistered)
                {
                    iJitterBufferStats.maxSeqNumRegistered = seqNum;
                    iJitterBufferStats.maxTimeStampRegistered = elem->getTimestamp();
                }
                iJitterBufferStats.currentOccupancy = iNumElems;
                PVMF_JB_LOGDATATRAFFIC_IN((0, "AddElement seqNum %d iNumElems %d", seqNum, iNumElems));
                return (oRet);
            }
            else if (currElem->getSeqNum() != seqNum)
            {
                PVMF_JB_LOGDATATRAFFIC_IN((0, "[0x%x] JB OW: iReadOffset=%d, iNumElemsLeft=%d, iLastRetrievedSeqNum=%d, old seqNum=%d, new seqNum=%d",
                                           this,
                                           iReadOffset,
                                           iNumElems,
                                           iLastRetrievedSeqNum,
                                           seqNum,
                                           currElem->getSeqNum()));
                /* Overwrite existing data */
                currElem.Unbind();
                /* Register Packet */
                iMediaPtrVec[offset] = elem;
                iJitterBufferStats.totalNumPacketsRegistered++;
                iJitterBufferStats.lastRegisteredSeqNum = seqNum;
                if (seqNum > iJitterBufferStats.maxSeqNumRegistered)
                {
                    iJitterBufferStats.maxSeqNumRegistered = seqNum;
                    iJitterBufferStats.maxTimeStampRegistered = elem->getTimestamp();
                }
                iJitterBufferStats.currentOccupancy = iNumElems;
                oRet = PVMF_JITTER_BUFFER_ADD_ELEM_PACKET_OVERWRITE;
                return (oRet);
            }
            else
            {
                PVMF_JB_LOGDATATRAFFIC_IN((0, "[0x%x]Duplicate packet iNumElems %d", this, iNumElems));
            }
            /* Duplicate Packet - Ignore */
            return (oRet);
        }

        PVMFSharedMediaDataPtr retrieveElement()
        {
            PVMFSharedMediaDataPtr dataPkt;
            uint32 count = 0;
            while (dataPkt.GetRep() == NULL)
            {
                /* No data */
                if (count > iArraySize)
                {
                    dataPkt.Unbind();
                    return dataPkt;
                }
                /* Wrap around */
                if (iReadOffset >= iArraySize)
                {
                    iReadOffset = 0;
                }
                dataPkt = iMediaPtrVec[iReadOffset];
                if (dataPkt.GetRep() == NULL)
                {
                    PVMF_JB_LOGDATATRAFFIC_IN_E((0, "[0x%x] Hole in Jb at index %u", this, iReadOffset));
                }
                iReadOffset++;
                count++;
            }

            iNumElems--;
            /* Mark the retrieved element location as free */
            PVMFSharedMediaDataPtr retElem = iMediaPtrVec[iReadOffset-1];
            retElem.Unbind();
            iMediaPtrVec[iReadOffset-1] = retElem;
            iLastRetrievedSeqNum = (int32)(dataPkt.GetRep()->getSeqNum());
            /* Check and register packet loss */
            iJitterBufferStats.totalPacketsLost += (count - 1);
            iJitterBufferStats.maxTimeStampRetrieved = dataPkt->getTimestamp();
            iJitterBufferStats.currentOccupancy = iNumElems;
            iJitterBufferStats.totalNumPacketsRetrieved++;
            iJitterBufferStats.lastRetrievedSeqNum = iLastRetrievedSeqNum;
            /* Get packet size */
            uint32 size = 0;
            uint32 numFragments = dataPkt->getNumFragments();
            for (uint32 i = 0; i < numFragments; i++)
            {
                OsclRefCounterMemFrag memFragIn;
                dataPkt->getMediaFragment(i, memFragIn);
                size += memFragIn.getMemFrag().len;
            }
            iJitterBufferStats.packetSizeInBytesLeftInBuffer -= size;
            PVMF_JB_LOGDATATRAFFIC_OUT((0, "[0x%x]PVMFDynamicCircularArray::retrieveElement: iReadOffset=%d, iNumElemsLeft=%d, SeqNum=%d",
                                        this,
                                        iReadOffset,
                                        iNumElems,
                                        iLastRetrievedSeqNum));

            return (dataPkt);
        }

        PVMFJitterBufferStats& getStats()
        {
            return iJitterBufferStats;
        }

        PVMFJitterBufferStats* getStatsPtr()
        {
            return &iJitterBufferStats;
        }

        void peekNextElementTimeStamp(PVMFTimestamp& aTS,
                                      uint32& aSeqNum)
        {
            PVMFSharedMediaDataPtr dataPkt;

            uint32 peekOffset = iReadOffset;
            uint32 count = 0;

            while (dataPkt.GetRep() == NULL)
            {
                if (count > iNumElems)
                {
                    aTS = 0xFFFFFFFF;
                }
                if (peekOffset >= iArraySize)
                {
                    peekOffset = 0;
                }
                dataPkt = iMediaPtrVec[peekOffset];
                peekOffset++;
                count++;
            }
            aTS = dataPkt.GetRep()->getTimestamp();
            aSeqNum = dataPkt.GetRep()->getSeqNum();
            return;
        }

        void peekMaxElementTimeStamp(PVMFTimestamp& aTS,
                                     uint32& aSeqNum)
        {
            aTS = iJitterBufferStats.maxTimeStampRegistered;
            aSeqNum = iJitterBufferStats.maxSeqNumRegistered;
            return;
        }

        bool CheckCurrentReadPosition()
        {
            uint32 offset = iReadOffset;

            if (offset >= iArraySize)
            {
                offset = 0;
            }

            PVMFSharedMediaDataPtr dataPkt =
                iMediaPtrVec[offset];

            if (dataPkt.GetRep() == NULL)
            {
                return false;
            }
            return true;
        }

        bool CheckSpaceAvailability(uint32 aNumElements = 1)
        {
            if ((iArraySize - iNumElems) > aNumElements)
            {
                return true;
            }
            return false;
        }

        void setSeqNumBase(uint32 aSeqNum)
        {
            iJitterBufferStats.seqNumBase = aSeqNum;
        }

        //setRTPInfoParams replaced by setSeqNumBase

        PVMFSharedMediaDataPtr getElementAt(uint32 aIndex)
        {
            if (aIndex > iArraySize) OSCL_LEAVE(OsclErrArgument);
            return (iMediaPtrVec[aIndex]);
        }

        void AddElementAt(PVMFSharedMediaDataPtr aMediaPtr,
                          uint32 aIndex)
        {
            if (aIndex > iArraySize) OSCL_LEAVE(OsclErrArgument);
            iMediaPtrVec[aIndex] = aMediaPtr;
        }

        void PurgeElementsWithSeqNumsLessThan(uint32 aSeqNum, uint32 aPrevSeqNumBaseOut)
        {
            PVMF_JB_LOGINFO((0, "PVMFDynamicCircularArray::PurgeElementsWithSeqNumsLessThan SeqNum %d aPrevSeqNumBaseOut %d", aSeqNum, aPrevSeqNumBaseOut));
            if (!iMediaPtrVec.empty())
            {
                if (aSeqNum < iLastRetrievedSeqNum)
                {
                    typedef typename Oscl_Vector<PVMFSharedMediaDataPtr, Alloc>::iterator iterator_type;
                    iterator_type it;
                    for (it = iMediaPtrVec.begin(); it != iMediaPtrVec.end(); it++)
                    {
                        if (it->GetRep() != NULL)
                        {
                            /* Get packet size */
                            uint32 size = 0;
                            uint32 numFragments = it->GetRep()->getNumFragments();
                            for (uint32 i = 0; i < numFragments; i++)
                            {
                                OsclRefCounterMemFrag memFragIn;
                                it->GetRep()->getMediaFragment(i, memFragIn);
                                size += memFragIn.getMemFrag().len;
                            }
                            iJitterBufferStats.packetSizeInBytesLeftInBuffer -= size;
                            it->Unbind();
                        }
                    }
                    iNumElems = 0;
                    /* If after purging all elements, we want to determine the TS of the previous element
                     * (with DeterminePrevTimeStampPeek()), it will give as false information if the
                     * seqnum has wrapped around. So because of that, we set aPrevSeqNumBaseOut to be smaller
                     * than the current seqnum.
                     */
                    aPrevSeqNumBaseOut = aSeqNum - 1;

                }
                else if (aSeqNum > iLastRetrievedSeqNum)
                {
                    /*
                     * Start from last retrieved seq num and offset it by
                     * first seq number added
                     * This guarantees that we deallocate in the allocation
                     * sequence.
                     */
                    uint32 startoffset = ((iLastRetrievedSeqNum - iFirstSeqNumAdded) + 1) % iArraySize;

                    PVMF_JB_LOGDATATRAFFIC_OUT((0, "[0x%x]PVMFDynamicCircularArray::PurgeElementsWithSeqNumsLessThan:  SeqNum=%d, StartOffset=%d, iArraySize=%d",
                                                this,
                                                aSeqNum,
                                                startoffset,
                                                iArraySize));

                    for (uint32 i = 0; i < aSeqNum - (iLastRetrievedSeqNum + 1); i++)
                    {
                        uint32 offset = (startoffset + i) % iArraySize;
                        /* Mark the retrieved element location as free */
                        PVMFSharedMediaDataPtr elem = iMediaPtrVec[offset];
                        if (elem.GetRep() != NULL)
                        {
                            if (elem->getSeqNum() < aSeqNum)
                            {
                                /* Get packet size */
                                uint32 size = 0;
                                uint32 numFragments = elem->getNumFragments();
                                for (uint32 i = 0; i < numFragments; i++)
                                {
                                    OsclRefCounterMemFrag memFragIn;
                                    elem->getMediaFragment(i, memFragIn);
                                    size += memFragIn.getMemFrag().len;
                                }
                                iJitterBufferStats.packetSizeInBytesLeftInBuffer -= size;
                                elem.Unbind();
                                iMediaPtrVec[offset] = elem;
                                iNumElems--;
                            }
                        }
                    }
                }
            }
            /* To prevent us from registering any old packets */
            iLastRetrievedSeqNum = aSeqNum - 1;
            iJitterBufferStats.lastRetrievedSeqNum = iLastRetrievedSeqNum;
            iJitterBufferStats.currentOccupancy = iNumElems;
            SetReadOffset(aSeqNum);
        }

        void PurgeElementsWithTimestampLessThan(PVMFTimestamp aTS)
        {
            if (!iMediaPtrVec.empty())
            {
                while (iNumElems > 0)
                {
                    /* Wrap around */
                    if (iReadOffset >= iArraySize)
                    {
                        iReadOffset = 0;
                    }
                    PVMFSharedMediaDataPtr dataPkt = iMediaPtrVec[iReadOffset];
                    if (dataPkt.GetRep() != NULL)
                    {
                        PVMF_JB_LOGDATATRAFFIC_IN((0, "[0x%x]JB Purge:ReadOffset=%d, NumElemsLeft=%d, lastRetrievedSeqNum=%d, seqNum=%d",
                                                   this,
                                                   iReadOffset,
                                                   iNumElems,
                                                   iLastRetrievedSeqNum,
                                                   dataPkt->getSeqNum()));
                        PVMFTimestamp tmpTS = dataPkt.GetRep()->getTimestamp();
                        if (tmpTS >= aTS)
                            break;

                        /* Get packet size */
                        uint32 size = 0;
                        uint32 numFragments = dataPkt->getNumFragments();
                        for (uint32 i = 0; i < numFragments; i++)
                        {
                            OsclRefCounterMemFrag memFragIn;
                            dataPkt->getMediaFragment(i, memFragIn);
                            size += memFragIn.getMemFrag().len;
                        }
                        iJitterBufferStats.packetSizeInBytesLeftInBuffer -= size;
                        (iMediaPtrVec[iReadOffset]).Unbind();
                        iNumElems--;
                    }
                    iReadOffset++;
                }
            }
            /* To prevent us from registering any old packets */
            iLastRetrievedTS = aTS;
            iJitterBufferStats.currentOccupancy = iNumElems;
        }

        void SetReadOffset(uint32 aSeqNum)
        {
            iReadOffset = (aSeqNum - iFirstSeqNumAdded) % iArraySize;
        }

    private:
        void InitVector(uint32 size)
        {
            for (uint32 i = 0; i < size; i++)
            {
                PVMFSharedMediaDataPtr sharedMediaPtr;
                iMediaPtrVec.push_back(sharedMediaPtr);
            }
        }

        uint32 iNumElems;
        uint32 iArraySize;

        uint32 iReadOffset;
        uint32 iLastRetrievedSeqNum;
        PVMFTimestamp iLastRetrievedTS;
        uint32 iMaxSeqNumAdded;
        uint32 iFirstSeqNumAdded;

        PVMFJitterBufferStats iJitterBufferStats;
        Oscl_Vector<PVMFSharedMediaDataPtr, Alloc> iMediaPtrVec;

        PVLogger* ipLogger;
        PVLogger* ipDataPathLoggerIn;
        PVLogger* ipDataPathLoggerOut;
};

class PVMFJitterBufferObserver
{
    public:
        virtual ~PVMFJitterBufferObserver() {}
        virtual void JitterBufferFreeSpaceAvailable(OsclAny* aContext) = 0;
        virtual void ProcessJBInfoEvent(PVMFAsyncEvent& aEvent) = 0;
        virtual void PacketReadyToBeRetrieved(OsclAny* aContext) = 0;
        virtual void EndOfStreamSignalled(OsclAny* aContext) = 0;
};

class PVMFJitterBuffer
{
    public:
        virtual ~PVMFJitterBuffer() {}
        /**
        	This API will be called when Streaming will be started, i.e. when
        	we are attempting to set up the transport for communication with the server
        	(firewall packet xchange) So, the jitter buffer should be in a state to
        	start accepting the *valid* packets from the server.
        	This API will start/initialize the estimation of the server clock
        	aka estimated server clock.
        	Can Leave: No
        */
        virtual void StreamingSessionStarted() = 0;

        /**
        	This API will be called when Streaming will be paused, i.e. we expect the server to stop
        	sending the data. Because of async nature of the SDK, there may be possibility of receiving
        	some data from the server. So, Jb will continue to accept any valid data.
        	Can Leave: No
        */
        virtual void StreamingSessionPaused() = 0;

        /**
        	This API will be called when Streaming session is considered terminated (from server/client),
        	Jb will not accept any packet from the server after this call and will flush itself.
        	Can Leave: No
        */
        virtual void StreamingSessionStopped() = 0;

        /**
        	To have smooth playback, jitter buffer needs to have sufficient amount of data ahead of the
        	current playback position. As data flows out of the jitter buffer, the occupancy of the jitter
        	buffer will decrease. To order to have sufficient data in it, jitter buffer will go into
        	rebuffering once differnce between the estimated server clock and playback clock becomes
        	less than *RebufferingThreshold*. The value of this "RebufferingThreshold" is
        	initialized/modified by this API.
        	params:
        	aRebufferingThresholdInMilliSeconds:[in] uint32
        	Can Leave: No
        	Constraint:
        	This value ought to be less than "Delay/Duration" specified by "SetDurationInMilliSeconds" API
        */
        virtual void SetRebufferingThresholdInMilliSeconds(uint32 aRebufferingThresholdInMilliSeconds) = 0;

        /**
        	To have smooth playback, before letting any data out of it, jitter buffer accumulates sufficient
        	amount of data ahead of the current playback position.
        	The amoount of data that Jb should acculate is discated by this API.
        	"aDuration" specifies the amount of data(in millisec) Jb should accumuate before letting any data out of it.
        	params:
        	aDuration:[in] uint32
        	Can Leave: No
        	Constraint:
        	This value ought to be more than "RebufferingThreshold" specified by "SetRebufferingThresholdInMilliSeconds" API
        */
        virtual void SetDurationInMilliSeconds(uint32 aDuration) = 0;

        /**
        	This API will reset any EOS flag signalled by the user of the JB.
        	This will reinitialize the streaming segment.
        	Jitter buffer will go into transition state and will remian in that state unless
        	SetPlayRange API is called on it.
        	Jitter buffer will neither accept nor let out any packet out of it.
        	Can Leave: No
        	Constraint: N/A
        */
        virtual void PrepareForRepositioning() = 0;

        /**
        	Provides the observer with the mime type of the data that is persisted in JB.
        */
        virtual const char* GetMimeType() const = 0;

        /**
        	Will let the observer of JB the current state of the JB.
        	Can Leave: No
        	Constraint: N/A
        */
        virtual PVMFJitterBufferDataState GetState() const = 0;

        /**
        	This API will set the state of the Jitter buffer.
        	Param(s):[in] aState
        	Can Leave: No
        	Constraint: N/A
        */
        virtual void SetJitterBufferState(PVMFJitterBufferDataState aState) = 0;

        /**
        	Jitter buffer maintains the pointers to the chunks of the data retrieved from the server.
        	The chunks of memory that are used to persist the data retrieved from the server are allocated
        	by the allocator created in some other component (in socket node for RTSP and the HTTP streaming)
        	Jitter buffer need to know the occupancy of the memory pool for saving itself from overflowing.
        	SetJitterBufferChunkAllocator provides jitter buffer the access to the allocator that allocates
        	chunks for persisting the media.
        	Param(s):[in] aDataBufferAllocator
        	Can Leave: No
        	Constraint: N/A
        */
        virtual void SetJitterBufferChunkAllocator(OsclMemPoolResizableAllocator* aDataBufferAllocator) = 0;

        /**
        	This API provides the info about the memory pool that is allocated for persisting the media
        	from the server.
        	Param(s):[in] uint32 aSize				- Size of memory pool
                     [in] uint32 aResizeSize		- If Jb needed to be extented, this will specify the mempool
        											  chunk that will be added up to the existing mempool.
                     [in] uint32 aMaxNumResizes		- Specifies the max number of times memory chunks of size
        											  "aResizeSize" can be appended to the mempool.
                     [in] uint32 aExpectedNumberOfBlocksPerBuffer	- Specifies the estimated max number of chunks
        											  of media that are expected to be in the Jitter buffer at
        											  any instant of time.
        	Can Leave: No
        	Constraint: N/A
        */
        virtual void SetJitterBufferMemPoolInfo(uint32 aSize,
                                                uint32 aResizeSize,
                                                uint32 aMaxNumResizes,
                                                uint32 aExpectedNumberOfBlocksPerBuffer) = 0;

        /**
        	This API provides the let the observer of JB know about the memory pool stats that is allocated
        	for persisting the media received from the server.
        	Param(s):[out] uint32 aSize				- Size of memory pool
                     [out] uint32 aResizeSize		- If Jb needed to be extented, this will specify the mempool
        											  chunk that will be added up to the existing mempool.
                     [out] uint32 aMaxNumResizes		- Specifies the max number of times memory chunks of size
        											  "aResizeSize" can be appended to the mempool.
                     [out] uint32 aExpectedNumberOfBlocksPerBuffer	- Specifies the estimated max number of chunks
        											  of media that are expected to be in the Jitter buffer at
        											  any instant of time.
        	Can Leave: No
        	Constraint: N/A
        */
        virtual void GetJitterBufferMemPoolInfo(uint32& aSize,
                                                uint32& aResizeSize,
                                                uint32& aMaxNumResizes,
                                                uint32& aExpectedNumberOfBlocksPerBuffer) const  = 0;


        /**
        	Registers the packet with the jitter buffer.
        	If inplace processing is false, then each fragment in the media msg is treated as a separate packet
        	and is internally wrapped in separate media msg by this functions and persisted in the jitter buffer.
        	This method may fail with the return value PVMF_JITTER_BUFFER_ADD_PKT_INSUFFICIENT_MEMORY
        	In this case, the user of the jitter buffer is expected to make async call NotifyFreeSpaceAvailable
        	call with the jitter buffer and wait for JitterBufferFreeSpaceAvailable callback before
        	requesting to register the packet again.
        	Param(s):[in] PVMFSharedMediaMsgPtr& aMsg - Media msg to be registered with the JB
        	Can Leave: No
        	Constraint: N/A
        */
        virtual PVMFJitterBufferRegisterMediaMsgStatus RegisterMediaMsg(PVMFSharedMediaMsgPtr& aMsg) = 0;

        /**
        	Specifies the initialization of new streaming segement.
        */
        virtual bool QueueBOSCommand(uint32 aStreamId) = 0;

        /**
        	Specifies the termination of the streaming.
        	Param(s):[out] PVMFSharedMediaMsgPtr& aMsg - Cmd Msg signifying EOS
        			 [out] aCmdPacket: 	aMsg is considered to be valid if aCmdPacket is true
        	Can Leave: No
        	Constraint: N/A
        */
        virtual PVMFStatus GenerateAndSendEOSCommand(PVMFSharedMediaMsgPtr& aMediaOutMsg, bool& aCmdPacket) = 0;

        /**
        	Specifies if sufficient amount of data is available with the jitter buffer.
        	Delay is considered to be established:
        		- If estimated server clock is ahead of playback clock by duration specified by
        		"SetDurationInMilliSeconds"
        		- or, EOS is signalled by the user of the JB
        		- and JB is not in *transition* state.

        	Param(s):[out] uint32& aClockDiff - Difference between the estimated server clock and client playback clock
        	Can Leave: No
        	Constraint: N/A

        */
        virtual bool IsDelayEstablished(uint32& aClockDiff) = 0;

        /**
        	This API will provide the user of the Jitter buffer with the media msg from the Jitter buffer.
        	The packet retrieved from Jb can be
        	- media packet
        	- command packet
        	Param(s):[out] PVMFSharedMediaMsgPtr& aMediaMsgPtr - Media data retrieved from the jitter buffer
        			 [out] bool& aCmdPacket - retrieved pkt from Jb is command packet
        	Can Leave: No
        	Constraint: N/A
        	Return value:PVMFStatus [PVMFSuccess/PVMFErrNotReady]
        			PVMFSuccess: If it is possible to retieve the media msg from the JB
        			PVMFErrNotReady: If packet cannot be retrieved from the Jb as of now.
        			User is Jb is expected to requet a callback via NotifyCanRetrievePacket API to get notification
        			about the readiness of JB to send out the packet.
        */
        virtual PVMFStatus RetrievePacket(PVMFSharedMediaMsgPtr& aMediaMsgPtr, bool& aCmdPacket) = 0;

        virtual PVMFStatus SetInputPacketHeaderPreparsed(bool aPreParsed) = 0;

        /**
        	Request the JB to signal when user can retrieve the packet from it.
        */
        virtual void NotifyCanRetrievePacket() = 0;

        /**
        	Cancel the previously made request(if any) with the jitter buffer.
        */
        virtual void CancelNotifyCanRetrievePacket() = 0;


        /**
        	Returns the PVMFJitterBufferStats structure.
        */
        virtual PVMFJitterBufferStats& getJitterBufferStats() = 0;

        /**
        	Cleans up the jitter buffer. All the data in the Jb is considered invalid and
        */
        virtual void FlushJitterBuffer() = 0;

        /**
        */
        virtual void ResetJitterBuffer() = 0;


        /**
        	This function returns the mts of the next packet that is expected to be retrieved
        	from the jitter buffer.
        */
        virtual PVMFTimestamp peekNextElementTimeStamp() = 0;

        /**

        */
        virtual PVMFTimestamp peekMaxElementTimeStamp() = 0;

        /**
        	Returns true if there is no media in the JB to be retrieved.
        	else returns false
        */
        virtual bool IsEmpty() = 0;

        /**
        	Signals that EOS is received, and no media data is expected anymore.
        */
        virtual void SetEOS(bool aVal) = 0;

        /**
        	Checks if EOS is received.
        */
        virtual bool GetEOS() = 0;

        /**
        	This function is used to specify the play range, i.e. session params for the
        	current streaming segment
        */
        virtual void SetPlayRange(int32 aStartTimeInMS, bool aPlayAfterSeek, bool aStopTimeAvailable = false, int32 aStopTimeInMS = 0) = 0;


        /**
        */
        virtual void PurgeElementsWithSeqNumsLessThan(uint32 aSeqNum, uint32 aPlayerClockMS) = 0;

        /**
        */
        virtual void PurgeElementsWithTimestampLessThan(PVMFTimestamp aTS) = 0;

        /**
        	Sets the mode of processing the input packets.
        	If "aInPlaceProcessing" boolean if set to true means that media msg wrappers
        	that carry the RTP packets from upstream node can be reused.
        	If "aInPlaceProcessing" boolean if set to false, then inside "RegisterPacket"
        	We allocate a new media msg wrapper and transfer the memory fragments from "inputDataPacket"
        	to "dataPacket".
        	If there are multiple media packets in "inputDataPacket" then we have to allocate new
        	media msg wrappers for each one of these. So "aInPlaceProcessing" cannot be true if
        	downstream node packs multiple media packets in a single media msg.
        */
        virtual void SetInPlaceProcessingMode(bool aInPlaceProcessingMode) = 0;

        /**
        */
        virtual bool addMediaCommand(PVMFSharedMediaMsgPtr& aMediaCmd) = 0;

        /**
        */
        virtual void SetAdjustedTSInMS(PVMFTimestamp aAdjustedTS) = 0;

        /**
        */
        virtual void SetBroadCastSession() = 0;

        /**
        	This function is only implemented for RTSP based streaming to set the source identifier.
        */
        virtual void setSSRC(uint32 aSSRC) = 0;

        /**
        	This function is only implemented for RTSP based streaming to get the source identifier.
        */
        virtual uint32 GetSSRC() const = 0;

        /**
        	This function is specific to the RTSP based streaming.
        	This is used to set the RTP info.
        	RTP info is genreally obtained in response to the play request sent to the 3GPP server
        */
        /**
        */
        virtual void AdjustRTPTimeStamp() = 0;

        virtual void setRTPInfoParams(PVMFRTPInfoParams rtpInfoParams, bool oPlayAfterASeek) = 0;

        virtual PVMFRTPInfoParams& GetRTPInfoParams() = 0;

        /**
        	This functiosn is RTSP streaming specific and is used to detrmine the interarriavl jitter
        */
        virtual uint32 getInterArrivalJitter() = 0;

        /**
        */
        virtual bool GetRTPTimeStampOffset(uint32& aTimeStampOffset) = 0;


        /**
        */
        virtual void SetRTPTimeStampOffset(uint32 newTSBase) = 0;

        /**
        */
        virtual PVMFSharedMediaDataPtr& GetFirstDataPacket(void) = 0;

        /**
        */
        virtual bool NotifyFreeSpaceAvailable() = 0;

        virtual void SetTrackConfig(OsclRefCounterMemFrag& aConfig) = 0;
        virtual void SetMediaClockConverter(MediaClockConverter* aConverter) = 0;
        virtual void SetTimeScale(uint32 aTimeScale) = 0;
        virtual uint32 GetTimeScale() const = 0;
        virtual void SetEarlyDecodingTimeInMilliSeconds(uint32 duration) = 0;
        virtual void SetBurstThreshold(float burstThreshold) = 0;
};

///////////////////////////////////////////////////////////////////////////////
// Contains implementation of functions common to all streaming types
///////////////////////////////////////////////////////////////////////////////
typedef enum
{
    PVMF_JB_ERR_INSUFFICIENT_MEM_TO_PACKETIZE,
    PVMF_JB_ERR_INVALID_CONFIGURATION,
    PVMF_JB_ERR_CORRUPT_HDR,
    PVMF_JB_ERR_TRUNCATED_HDR,
    PVMF_JB_ERR_UNEXPECTED_PKT,
    PVMF_JB_ERR_LATE_PACKET,
    PVMF_JB_ERR_NO_PACKET,
    PVMF_JB_PACKET_PARSING_SUCCESS
} PVMFJBPacketParsingAndStatUpdationStatus;

class PVMFJitterBufferImpl : public PVMFJitterBuffer
            , public OsclMemPoolFixedChunkAllocatorObserver
            , public PVMFJBEventNotifierObserver
{
    public:
        OSCL_IMPORT_REF virtual ~PVMFJitterBufferImpl();
        OSCL_IMPORT_REF virtual void StreamingSessionStarted();
        OSCL_IMPORT_REF virtual void StreamingSessionPaused();
        OSCL_IMPORT_REF virtual void StreamingSessionStopped();
        OSCL_IMPORT_REF virtual void SetRebufferingThresholdInMilliSeconds(uint32 aRebufferingThresholdInMilliSeconds);
        OSCL_IMPORT_REF virtual void SetDurationInMilliSeconds(uint32 aDuration);
        OSCL_IMPORT_REF void PrepareForRepositioning();
        OSCL_IMPORT_REF PVMFJitterBufferDataState GetState() const;
        OSCL_IMPORT_REF virtual void SetJitterBufferState(PVMFJitterBufferDataState aState);
        OSCL_IMPORT_REF virtual void SetJitterBufferChunkAllocator(OsclMemPoolResizableAllocator* aDataBufferAllocator);
        OSCL_IMPORT_REF virtual const char* GetMimeType() const;
        OSCL_IMPORT_REF virtual PVMFJitterBufferRegisterMediaMsgStatus RegisterMediaMsg(PVMFSharedMediaMsgPtr& msg);
        OSCL_IMPORT_REF bool QueueBOSCommand(uint32 aStreamId);
        OSCL_IMPORT_REF void SetInPlaceProcessingMode(bool aInPlaceProcessingMode);
        OSCL_IMPORT_REF PVMFStatus GenerateAndSendEOSCommand(PVMFSharedMediaMsgPtr& aMediaOutMsg, bool& aCmdPacket);
        OSCL_IMPORT_REF virtual void NotifyCanRetrievePacket();
        OSCL_IMPORT_REF virtual void CancelNotifyCanRetrievePacket();
        OSCL_IMPORT_REF virtual PVMFStatus RetrievePacket(PVMFSharedMediaMsgPtr& aMediaMsgPtr, bool& aCmdPacket);
        OSCL_IMPORT_REF virtual PVMFStatus SetInputPacketHeaderPreparsed(bool aPreParsed);
        OSCL_IMPORT_REF virtual PVMFJitterBufferStats& getJitterBufferStats();
        OSCL_IMPORT_REF virtual PVMFTimestamp peekNextElementTimeStamp();
        OSCL_IMPORT_REF virtual PVMFTimestamp peekMaxElementTimeStamp();
        OSCL_IMPORT_REF bool IsEmpty();
        OSCL_IMPORT_REF void SetEOS(bool aVal);
        OSCL_IMPORT_REF bool GetEOS();
        OSCL_IMPORT_REF void SetPlayRange(int32 aStartTimeInMS, bool aPlayAfterSeek, bool aStopTimeAvailable = false, int32 aStopTimeInMS = 0);
        OSCL_IMPORT_REF bool CheckForHighWaterMark();
        OSCL_IMPORT_REF bool CheckForLowWaterMark();
        OSCL_IMPORT_REF bool CheckNumElements();
        OSCL_IMPORT_REF bool addMediaCommand(PVMFSharedMediaMsgPtr& aMediaCmd);
        OSCL_IMPORT_REF virtual bool GetPendingCommand(PVMFSharedMediaMsgPtr& aCmdMsg);
        OSCL_IMPORT_REF virtual bool HasPendingCommand();
        OSCL_IMPORT_REF void SetAdjustedTSInMS(PVMFTimestamp aAdjustedTSInMS);
        OSCL_IMPORT_REF virtual void SetBroadCastSession();
        OSCL_IMPORT_REF virtual PVMFRTPInfoParams& GetRTPInfoParams();
        OSCL_IMPORT_REF bool GetRTPTimeStampOffset(uint32& aTimeStampOffset);
        OSCL_IMPORT_REF bool NotifyFreeSpaceAvailable();
        OSCL_IMPORT_REF void freechunkavailable(OsclAny*);
        OSCL_IMPORT_REF virtual void FlushJitterBuffer();
        OSCL_IMPORT_REF virtual void ResetJitterBuffer();
        OSCL_IMPORT_REF bool CheckSpaceAvailability(PVMFSharedMediaMsgPtr& aMsg);
        OSCL_IMPORT_REF bool CheckSpaceAvailability();
        OSCL_IMPORT_REF void setSSRC(uint32 aSSRC);
        OSCL_IMPORT_REF uint32 GetSSRC() const;
        OSCL_IMPORT_REF virtual void SetJitterBufferMemPoolInfo(uint32 aSize,
                uint32 aResizeSize,
                uint32 aMaxNumResizes,
                uint32 aExpectedNumberOfBlocksPerBuffer);
        OSCL_IMPORT_REF void GetJitterBufferMemPoolInfo(uint32& aSize, uint32& aResizeSize, uint32& aMaxNumResizes, uint32& aExpectedNumberOfBlocksPerBuffer) const;
        OSCL_IMPORT_REF virtual void SetTrackConfig(OsclRefCounterMemFrag& aConfig);
        OSCL_IMPORT_REF virtual void SetMediaClockConverter(MediaClockConverter* aConverter);
        OSCL_IMPORT_REF virtual void SetTimeScale(uint32 aTimeScale);
        OSCL_IMPORT_REF virtual uint32 GetTimeScale() const ;
        OSCL_IMPORT_REF bool IsDelayEstablished(uint32& aClockDiff);
    protected:

        OSCL_IMPORT_REF void LogClientAndEstimatedServerClock(PVLogger*& aLogger);
        OSCL_IMPORT_REF bool RequestEventCallBack(JB_NOTIFY_CALLBACK aEventType, uint32 aDelay = 0, OsclAny* aContext = NULL);
        OSCL_IMPORT_REF void CancelEventCallBack(JB_NOTIFY_CALLBACK aEventType, OsclAny* aContext = NULL);
        OSCL_IMPORT_REF bool IsCallbackPending(JB_NOTIFY_CALLBACK aEventType, OsclAny* aContext);
        OSCL_IMPORT_REF void ProcessCallback(CLOCK_NOTIFICATION_INTF_TYPE aClockNotificationInterfaceType, uint32 aCallBkId, const OsclAny* aContext, PVMFStatus aStatus);

        OSCL_IMPORT_REF PVMFJitterBufferImpl(const PVMFJitterBufferConstructParams& aJBCreationData);

        typedef enum
        {
            STREAMINGSTATE_UNKNOWN,
            STREAMINGSTATE_STARTED,
            STREAMINGSTATE_PAUSED,
            STREAMINGSTATE_STOPPED
        }StreamingState;


        virtual bool IsSeqTsValidForPkt(uint32 aSeqNum, uint32 aTs, PVMFJitterBufferStats& jbStats) = 0;
        OSCL_IMPORT_REF virtual uint32 GetNumOfPackets(PVMFSharedMediaMsgPtr& aMsg) const;
        OSCL_IMPORT_REF virtual void ReportJBInfoEvent(PVMFAsyncEvent& aEvent);
        OSCL_IMPORT_REF virtual PVMFSharedMediaDataPtr RetrievePacketPayload();

        OSCL_IMPORT_REF void Construct();
        OSCL_IMPORT_REF bool Allocate(OsclSharedPtr<PVMFMediaDataImpl>& mediaDataOut);
        OSCL_IMPORT_REF bool CreateMediaData(PVMFSharedMediaDataPtr& dataPacket, OsclSharedPtr<PVMFMediaDataImpl>& mediaDataOut);

        //pure virtuals to be overridden by the derived implementation
        virtual PVMFJBPacketParsingAndStatUpdationStatus ParsePacketHeaderAndUpdateJBStats(PVMFSharedMediaDataPtr& inDataPacket,
                PVMFSharedMediaDataPtr& outDataPacket,
                uint32 aFragIndex = 0) = 0;
        virtual void EOSCmdReceived() = 0;
        virtual void DeterminePrevTimeStampPeek(uint32 aSeqNum,
                                                PVMFTimestamp& aPrevTS) = 0;

        virtual void ComputeMaxAdjustedRTPTS() = 0;
        virtual void CheckForRTPTimeAndRTPSeqNumberBase() = 0;
        virtual bool CanRetrievePacket(PVMFSharedMediaMsgPtr& aMediaOutMsg, bool& aCmdPacket) = 0;
        virtual bool CanRetrievePacket() = 0;
        virtual void DeterminePrevTimeStamp(uint32 aSeqNum) = 0;
        OSCL_IMPORT_REF virtual PVMFStatus PerformFlowControl(bool aIncomingMedia);
        uint32 iSeqNum;
        class JitterBufferMemPoolInfo
        {
            public:
                JitterBufferMemPoolInfo(): iSize(0), iResizeSize(0), iMaxNumResizes(0), iExpectedNumberOfBlocksPerBuffer(0) {}
                void Init(uint32 aSize = 0, uint32 aExpectedNumberOfBlocksPerBuffer = 0, uint32 aResizeSize = 0, uint32 aMaxNumResizes = 0)
                {
                    iSize = aSize;
                    iResizeSize = aResizeSize;
                    iMaxNumResizes = aMaxNumResizes;
                    iExpectedNumberOfBlocksPerBuffer = aExpectedNumberOfBlocksPerBuffer;
                }
                uint32 iSize;
                uint32 iResizeSize;
                uint32 iMaxNumResizes;
                uint32 iExpectedNumberOfBlocksPerBuffer;
        };
        JitterBufferMemPoolInfo iJitterBufferMemPoolInfo;
        PVMFSharedMediaDataPtr firstDataPacket;
        Oscl_Vector<PVMFSharedMediaDataPtr, OsclMemAllocator> iFirstDataPackets;

        StreamingState	  iStreamingState;
        bool	iPlayingAfterSeek;
        bool	iReportCanRetrievePacket;

        bool	iInPlaceProcessing;
        bool	iOnePacketPerFragment;
        bool	iOnePacketPerMediaMsg;

        uint32 iLastPacketOutTs;

        bool iOverflowFlag;
        /* Media Command related */
        Oscl_Vector<MediaCommandMsgHolder, OsclMemAllocator> iMediaCmdVec;
        //Allocators [Will be created only when PVMFJitterBufferImpl::iInPlaceProcessing == false]
        PVMFMediaFragGroupCombinedAlloc<OsclMemPoolFixedChunkAllocator>* iMediaDataGroupAlloc;
        OsclMemPoolFixedChunkAllocator* iMediaDataImplMemPool;
        OsclMemPoolFixedChunkAllocator* iMediaMsgMemPool;

        int32 iStartTimeInMS;
        int32 iStopTimeInMS;
        bool iPlayStopTimeAvailable;
        bool	iBroadCastSession;

        PVMFTimestamp iMaxAdjustedRTPTS;

        bool	iSessionDurationExpired;
        uint32	iDurationInMilliSeconds;
        uint32	iRebufferingThresholdInMilliSeconds;

        uint64 iMonotonicTimeStamp;
        uint32 iFirstSeqNum;
        typedef PVMFDynamicCircularArray<OsclMemAllocator> PVMFDynamicCircularArrayType;
        PVMFDynamicCircularArrayType* iJitterBuffer;
        Oscl_Vector<PVMFRTPInfoParams, OsclMemAllocator> iRTPInfoParamsVec;
        bool	iEOSSignalled;
        bool	iEOSSent;
        uint32 iStreamID;
        PVMFTimestamp iMaxAdjustedTS; //[iMaxAdjustedRTPTS]
        PVMFTimestamp iPrevAdjustedTS;//[iPrevAdjustedRTPTS]
        PVMFMediaClock& irEstimatedServerClock;
        PVMFMediaClock& irClientPlayBackClock;
        PVMFJBEventNotifier& irJBEventNotifier;
        OSCL_HeapString<OsclMemAllocator> irMimeType;
        bool&	irDelayEstablished;
        uint32&	irJitterDelayPercent;
        PVMFJitterBufferDataState&	irDataState;
        bool iInProcessingMode;
        bool iHeaderPreParsed;

        uint32 iRTPTimeScale;
        PVMFTimestamp  iPrevTSOut;
        MediaClockConverter iEstServClockMediaClockConvertor;
        PVMFJitterBufferObserver* const iObserver;
        OsclAny* const iObserverContext;

        bool   seqNumLock;

        uint32 iInterArrivalJitter;
        bool   oFirstPacket;

        OsclRefCounterMemFrag		iTrackConfig;
        uint32 iTimeScale;
        MediaClockConverter*		ipMediaClockConverter;

        uint32 SSRCLock;
        bool   oSSRCFromSetUpResponseAvailable;
        uint32 SSRCFromSetUpResponse;

        uint32 iPrevSeqNumBaseOut;
        PVMFTimestamp seqLockTimeStamp;

        PVMFTimestamp iPrevAdjustedRTPTS;
        PVMFTimestamp iPrevTSIn;
        uint32 iPrevSeqNumBaseIn;

        OsclMemPoolResizableAllocator* iBufferAlloc;
        uint32 prevMinPercentOccupancy;
        uint32 consecutiveLowBufferCount;
        uint32 iNumUnderFlow;

        bool   iMonitorReBufferingCallBkPending;
        bool   iWaitForOOOPacketCallBkPending;
        bool   iJitterBufferDurationCallBkPending;

        uint32 iWaitForOOOPacketCallBkId;
        uint32 iMonitorReBufferingCallBkId;
        uint32 iJitterBufferDurationCallBkId;

        PVLogger* ipLogger;
        PVLogger* ipClockLoggerSessionDuration;
        PVLogger* ipDataPathLogger;
        PVLogger* ipMaxRTPTsLogger;
        PVLogger* ipDataPathLoggerIn;
        PVLogger* ipDataPathLoggerOut;
        PVLogger* ipClockLogger;
        PVLogger* ipClockLoggerRebuff;
        PVLogger* ipDataPathLoggerFlowCtrl;
        PVLogger* ipJBEventsClockLogger;
        PVLogger* ipRTCPDataPathLoggerIn;
        PVLogger* ipRTCPDataPathLoggerOut;
    private:
        void CreateAllocators();
        void DestroyAllocators();
        PVMFJitterBufferRegisterMediaMsgStatus AddPacket(PVMFSharedMediaDataPtr& aDataPacket);
        OSCL_IMPORT_REF virtual PVMFJitterBufferRegisterMediaMsgStatus RegisterCmdPacket(PVMFSharedMediaMsgPtr& aMediaCmd);
        OSCL_IMPORT_REF virtual bool CanRegisterMediaMsg();
        PVMFJitterBufferRegisterMediaMsgStatus RegisterDataPacket(PVMFSharedMediaDataPtr& aDataPacket);
        void ResetParams(bool aReleaseMemory = true);
        void HandleEvent_MonitorReBuffering(const OsclAny* aContext);
        void HandleEvent_NotifyWaitForOOOPacketComplete(const OsclAny* aContext);
        void HandleEvent_JitterBufferBufferingDurationComplete();
};

#endif
